#include "renderstuff.h"

#include "dma.h"
#include "scene.h"
#include "nalib/napacket.h"
#include "noodlepacket.h"
#include "model.h"
#include "spram.h"
#include "mfifo.h"
#include "eecore.h"
#include "gsstate.h"

/* data */
// Decoded from the original 14-quadword DMA payload (13 A+D registers).
// The 14-quadword A+D payload targets drawing context 2. The three draw
// environment stores in InitializeEECore land one slot before the registers
// that follow them in this table, see docs/behavior-notes.md.
struct PrEECoreInitializationPacket {
    sceDmaTag dma;
    sceGifTag gif;
    PrGsAD frame;
    PrGsAD zbuf;
    PrGsAD test;
    PrGsAD alpha;
    PrGsAD textureFilter;
    PrGsAD frameAlpha;
    PrGsAD primitiveControl;
    PrGsAD textureFunction;
    PrGsAD colorClamp;
    PrGsAD alphaBlendControl;
    PrGsAD offset;
    PrGsAD scissor;
    PrGsAD dithering;
};
PrEECoreInitializationPacket initEECoreDmaPacket = {
    { 14, 0, 0, reinterpret_cast<sceDmaTag*>(&initEECoreDmaPacket.gif), { 0, 0 } },
    { 13, 1, 0, 0, 0, 0, 0, 1, SCE_GIF_PACKED_AD },
    { 0, SCE_GS_FRAME_2 },
    { 0, SCE_GS_ZBUF_2 },
    { PR_TEST_ALPHA_NONZERO(SCE_GS_ZGEQUAL), SCE_GS_TEST_2 },
    { PR_ALPHA_BLEND, SCE_GS_ALPHA_2 },
    { PR_TEX1_BILINEAR_LOD0, SCE_GS_TEX1_2 },
    { 0, SCE_GS_FBA_2 },
    { 1, SCE_GS_PRMODECONT },
    { SCE_GS_SET_TEXA(/*TA0*/0, /*AEM*/0, /*TA1*/0x80), SCE_GS_TEXA },
    { 1, SCE_GS_COLCLAMP },
    { 0, SCE_GS_PABE },
    { 0, SCE_GS_XYOFFSET_2 },
    { 0, SCE_GS_SCISSOR_2 },
    { 0, SCE_GS_DTHE },
};

void PrRenderStuff::InitializeEECore(PrSceneObject *scene) {
    PrEECoreInitializationPacket *packet = reinterpret_cast<PrEECoreInitializationPacket*>(PR_UNCACHED(&initEECoreDmaPacket));
    packet->zbuf.value = NaGifPacket::EncodeRegister(m_zbuf);
    packet->frame.value = NaGifPacket::EncodeRegister(scene->m_frame);
    packet->alphaBlendControl.value = NaGifPacket::EncodeRegister(scene->m_xyoffset);
    packet->offset.value = NaGifPacket::EncodeRegister(scene->m_drawEnv->scissor1);
    packet->scissor.value = NaGifPacket::EncodeRegister(scene->m_drawEnv->dthe);

    PrWaitDmaFinish(SCE_DMA_GIF);
    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0;
    sceDmaSend(chan, &initEECoreDmaPacket);
}

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)EE_SCRATCHPAD_BASE;

// The two vertex kernels below are handwritten VU0 macro-mode code. VF1-VF4
// hold the clip matrix, VF5-VF8 the screen matrix and VF10 the depth bias, all
// loaded by RenderNodeEECore; VF11 (winding) and VF17/VF19 (previous strip
// positions) persist between vertices. Each kernel reads the staged input
// vertex and writes the output vertex in GIF order (texture, color, position).
// The vertex is flagged "kicked off" (W = 0x8000 via VFTOI15) when it is
// outside the clip volume, in the flagged clip window, or - for the normal
// kernel - when it faces away from the camera.
void PrRenderStuff::RenderVertexEECoreBothface() {
    PrEECoreContext &context = prSpramData->m_eeCore;
    asm volatile(
        ".set push\n\t"
        ".set noreorder\n\t"
        "lqc2         $vf12, 0x00(%0)\n\t"          /* position */
        "lqc2         $vf13, 0x10(%0)\n\t"          /* color */
        "lqc2         $vf14, 0x20(%0)\n\t"          /* texture */
        "lui          $3, 0x4\n\t"
        "vmulax.xyzw  ACC, $vf5, $vf12x\n\t"        /* screen = position * screenMatrix */
        "vmadday.xyzw ACC, $vf6, $vf12y\n\t"
        "vmaddaz.xyzw ACC, $vf7, $vf12z\n\t"
        "vmaddw.xyzw  $vf16, $vf8, $vf12w\n\t"
        "vmulax.xyzw  ACC, $vf1, $vf12x\n\t"        /* clip = position * clipMatrix */
        "vmadday.xyzw ACC, $vf2, $vf12y\n\t"
        "vmaddaz.xyzw ACC, $vf3, $vf12z\n\t"
        "vdiv         Q, $vf0w, $vf16w\n\t"
        "vmaddw.xyzw  $vf24, $vf4, $vf12w\n\t"
        "vmuly.xyz    $vf13, $vf13, $vf10y\n\t"
        "addi         $3, $3, -0x1\n\t"
        "vsuba.xyzw   ACC, $vf0, $vf0\n\t"
        ".word        0x4BF8C1FF\n\t"               /* vclipw.xyzw $vf24, $vf24w */
        "vaddax.z     ACC, $vf0, $vf10x\n\t"
        "vmaddq.xyzw  $vf16, $vf16, Q\n\t"
        "vmulq.xyzw   $vf14, $vf14, Q\n\t"
        "vmtir        $vi6, $vf14w\n\t"
        "cfc2.ni      $2, $vi18\n\t"
        "vftoi4.xyzw  $vf16, $vf16\n\t"
        "and          $2, $2, $3\n\t"
        "vftoi0.xyzw  $vf13, $vf13\n\t"
        "bnez         $2, .LEECoreBothfaceKick\n\t"
        "vnop\n\t"
        "cfc2.ni      $2, $vi6\n\t"
        "beqz         $2, .LEECoreBothfaceStore\n\t"
        "vnop\n"
".LEECoreBothfaceKick:\n\t"
        "vftoi15.w    $vf16, $vf0\n\t"
        "vnop\n"
".LEECoreBothfaceStore:\n\t"
        "sqc2         $vf14, 0x00(%1)\n\t"
        "sqc2         $vf13, 0x10(%1)\n\t"
        "sqc2         $vf16, 0x20(%1)\n\t"
        ".set pop"
        : : "r"(&context.input), "r"(&context.output) : "$2", "$3", "memory");
}

void PrRenderStuff::RenderVertexEECoreNormal() {
    PrEECoreContext &context = prSpramData->m_eeCore;
    asm volatile(
        ".set push\n\t"
        ".set noreorder\n\t"
        "lqc2         $vf12, 0x00(%0)\n\t"
        "lqc2         $vf13, 0x10(%0)\n\t"
        "lqc2         $vf14, 0x20(%0)\n\t"
        "lui          $3, 0x4\n\t"
        "vmulax.xyzw  ACC, $vf5, $vf12x\n\t"
        "vmadday.xyzw ACC, $vf6, $vf12y\n\t"
        "vmaddaz.xyzw ACC, $vf7, $vf12z\n\t"
        "vmaddw.xyzw  $vf16, $vf8, $vf12w\n\t"
        "vmulax.xyzw  ACC, $vf1, $vf12x\n\t"
        "vmadday.xyzw ACC, $vf2, $vf12y\n\t"
        "vmaddaz.xyzw ACC, $vf3, $vf12z\n\t"
        "vdiv         Q, $vf0w, $vf16w\n\t"
        "vmaddw.xyzw  $vf24, $vf4, $vf12w\n\t"
        "vmuly.xyz    $vf13, $vf13, $vf10y\n\t"
        "addi         $3, $3, -0x1\n\t"
        "vsuba.xyzw   ACC, $vf0, $vf0\n\t"
        ".word        0x4BF8C1FF\n\t"               /* vclipw.xyzw $vf24, $vf24w */
        "vaddax.z     ACC, $vf0, $vf10x\n\t"
        "vmaddq.xyzw  $vf16, $vf16, Q\n\t"
        "vmulq.xyzw   $vf14, $vf14, Q\n\t"
        "vmtir        $vi6, $vf14w\n\t"
        "cfc2.ni      $2, $vi18\n\t"
        "vsub.xyzw    $vf18, $vf16, $vf17\n\t"      /* edge from the previous strip position */
        "vftoi4.xyzw  $vf24, $vf16\n\t"
        "and          $2, $2, $3\n\t"
        "vftoi0.xyzw  $vf13, $vf13\n\t"
        "vopmula.xyz  ACC, $vf18, $vf19\n\t"        /* face normal z from the two strip edges */
        "vopmsub.xyz  $vf0, $vf19, $vf18\n\t"
        "bnez         $2, .LEECoreNormalKick\n\t"
        "cfc2.ni      $2, $vi6\n\t"
        "bnez         $2, .LEECoreNormalKick\n\t"
        "vnop\n\t"
        "vnop\n\t"
        "cfc2.ni      $2, $vi17\n\t"                /* sign of the face normal (status flag) */
        "andi         $2, $2, 0x20\n\t"
        "beqz         $2, .LEECoreNormalStore\n\t"
        "vnop\n"
".LEECoreNormalKick:\n\t"
        "vftoi15.w    $vf24, $vf0\n"
".LEECoreNormalStore:\n\t"
        "vmulz.xyzw   $vf19, $vf18, $vf11z\n\t"
        "vsub.z       $vf11, $vf0, $vf11\n\t"
        "sqc2         $vf14, 0x00(%1)\n\t"
        "sqc2         $vf13, 0x10(%1)\n\t"
        "sqc2         $vf24, 0x20(%1)\n\t"
        "vmove.xyzw   $vf17, $vf16\n\t"
        ".set pop"
        : : "r"(&context.input), "r"(&context.output) : "$2", "$3", "memory");
}

void PrRenderStuff::RenderVertexEECoreRefmap() {
    /* Empty */
}

void PrRenderStuff::RenderVertexEECoreContour() {
    /* Empty */
}

// These handwritten kernels use a persistent VU register convention. They
// consume the context input and produce its output without a C++ this pointer.
// Slots 4-7 are zero in the original data.
static PrEECoreVertexKernel eeCoreVertexKernels[8] = {
    PrRenderStuff::RenderVertexEECoreNormal,
    PrRenderStuff::RenderVertexEECoreBothface,
    PrRenderStuff::RenderVertexEECoreContour,
    PrRenderStuff::RenderVertexEECoreRefmap,
};
static sceDmaTag eeCoreChunkDmaTag = { 0, 0, PR_DMA_TAG_CNT, NULL, { 0, 0 } };
static float eeCoreRandomCenter[4] __attribute__((aligned(16))) = { 1.5f, 1.5f, 1.5f, 1.5f };
struct PrEECoreDisturbance {
    u_int seed;
    u_int reserved;
    float amplitude;
    u_int reservedTail;
};
static PrEECoreDisturbance eeCoreDisturbance;


void PrRenderStuff::RenderNodeEECore(PrVuNodeHeaderDmaPacket *packet) {
    PrVuDataChunkPacketHeader *first = reinterpret_cast<PrVuDataChunkPacketHeader*>(packet + 1);
    if (PrMfifoUnsentDataSize() != 0 || first->dma.id == 0x10 || static_cast<u_int>(packet->m_microprogram) >= PR_MICRO_PROGRAM_CONTOUR) {
        AppendDmaTag(&packet->m_tag);
        return;
    }

    packet = reinterpret_cast<PrVuNodeHeaderDmaPacket*>(PR_UNCACHEDACCEL(packet));
    PrEECoreContext &context = prSpramData->m_eeCore;
    context.WaitForVuTransfers();
    NaMATRIX<float, 4, 4>::Multiply(context.screenMatrix, prSpramData->m_view_projection_matrix, packet->m_matrix);
    context.WaitForVuTransfers();
    NaMATRIX<float, 4, 4>::Multiply(context.clipMatrix, prSpramData->m_worldClipMatrix, packet->m_matrix);
    context.WaitForVuTransfers();
    context.vertexKernel = eeCoreVertexKernels[packet->m_microprogram];

    // VF1..VF11 are intentionally shared with the handwritten vertex kernels.
    // Subsequent CPU packet copies use MMI, not VU math, to preserve this state.
    asm volatile(
        "lqc2 $vf5, 0x0(%0)\n\t"
        "lqc2 $vf6, 0x10(%0)\n\t"
        "lqc2 $vf7, 0x20(%0)\n\t"
        "lqc2 $vf8, 0x30(%0)\n\t"
        "lqc2 $vf1, 0x0(%1)\n\t"
        "lqc2 $vf2, 0x10(%1)\n\t"
        "lqc2 $vf3, 0x20(%1)\n\t"
        "lqc2 $vf4, 0x30(%1)\n\t"
        "lqc2 $vf10, 0x0(%2)\n\t"
        : : "r"(&context.screenMatrix), "r"(&context.clipMatrix), "r"(&packet->m_eeDepthBias)
        : "memory");

    packet = static_cast<PrVuNodeHeaderDmaPacket*>(PR_DECACHE(packet));
    PrVuDataChunkPacketHeader *chunk = reinterpret_cast<PrVuDataChunkPacketHeader*>(packet + 1);
    while (chunk != NULL) {
        RenderChunkEECore(chunk, packet->m_disturbance);
        chunk = chunk->Next();
    }
}

void PrRenderStuff::RenderChunkEECore(PrVuDataChunkPacketHeader *chunk, float disturbance) {
    PrSPRAM_DATA *scratchpad = prSpramData;
    PrEECoreContext &context = scratchpad->m_eeCore;
    u_long128 *buffer = scratchpad->m_noodle_buffer[2];
    sceDmaTag *dma = reinterpret_cast<sceDmaTag*>(buffer);
    *dma = eeCoreChunkDmaTag;
    const u_int prefixQuadwords = chunk->metadata.prefixTripletCount * 3 + 1;
    dma->qwc = prefixQuadwords + chunk->metadata.vertexCount * 3;

    // Preserve the complete variable GIF prefix (GS state and drawing tags)
    // before appending the CPU-rendered vertex records.
    const u_long128 *prefix = reinterpret_cast<const u_long128*>(&chunk->gif);
    u_long128 *destination = buffer + 1;
    for (u_int i = 0; i < prefixQuadwords; i++) destination[i] = prefix[i];
    PrEECoreOutputVertex *output = reinterpret_cast<PrEECoreOutputVertex*>(destination + prefixQuadwords);
    const PrEECoreInputVertex *input = chunk->InputVertices();

    // Reset clip flags once per strip; VF11.z alternates its winding sign in
    // the normal kernel. VF17/VF19 retain the previous strip positions.
    asm volatile("ctc2.ni $0, $vi18\n\tlqc2 $vf11, 0x0(%0)"
                 : : "r"(&chunk->metadata) : "memory");
    if (disturbance != 0.0f) {
        eeCoreDisturbance.seed = scratchpad->m_disturbance_param;
        eeCoreDisturbance.amplitude = disturbance;
    }
    for (u_int i = 0; i < chunk->metadata.vertexCount; i++) {
        context.input = input[i];
        if (disturbance != 0.0f) {
            // VU R randomness mixes the original position with the model seed.
            // Keep the original fused multiply/subtract order and R updates.
            asm volatile(
                "lqc2 $vf23, 0x0(%0)\n\t"
                "lqc2 $vf24, 0x0(%1)\n\t"
                "lqc2 $vf25, 0x0(%2)\n\t"
                "vrinit R, $vf24x\n\t"
                "vaddax.xyz ACC, $vf25, $vf0x\n\t"
                "vrxor R, $vf25x\n\t"
                "vrnext.x $vf26, R\n\t"
                "vrxor R, $vf25y\n\t"
                "vrnext.y $vf26, R\n\t"
                "vrxor R, $vf25z\n\t"
                "vrnext.z $vf26, R\n\t"
                "vmsubaz.xyz ACC, $vf23, $vf24z\n\t"
                "vmaddz.xyz $vf25, $vf26, $vf24z\n\t"
                "sqc2 $vf25, 0x0(%2)\n\t"
                : : "r"(eeCoreRandomCenter), "r"(&eeCoreDisturbance), "r"(&context.input.position)
                : "memory");
        }
        context.vertexKernel();
        output[i] = context.output;
    }
    PrSendMfifo(dma);
    scratchpad = prSpramData;
    buffer = scratchpad->m_noodle_buffer[0];
    scratchpad->m_noodle_buffer[0] = scratchpad->m_noodle_buffer[1];
    scratchpad->m_noodle_buffer[1] = scratchpad->m_noodle_buffer[2];
    scratchpad->m_noodle_buffer[2] = buffer;
}

// The original trailing queue-forwarding stub is emitted from AppendDmaTag
// in renderstuff.h when used; it is not a separate engine function.
