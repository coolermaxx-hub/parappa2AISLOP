#include "renderstuff.h"

#include "dma.h"
#include "scene.h"
#include "nalib/napacket.h"
#include "noodlepacket.h"
#include "model.h"
#include "spram.h"
#include "mfifo.h"
#include "eecore.h"

/* data */
// Decoded from the original 14-quadword DMA payload (13 A+D registers).
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
    PrGsAD offset;
    PrGsAD scissor;
    PrGsAD dithering;
    PrGsAD ditherMatrix;
};
extern PrEECoreInitializationPacket initEECoreDmaPacket;

void PrRenderStuff::InitializeEECore(PrSceneObject *scene) {
    PrEECoreInitializationPacket *packet = reinterpret_cast<PrEECoreInitializationPacket*>(PR_UNCACHED(&initEECoreDmaPacket));
    packet->zbuf.value = NaGifPacket::EncodeRegister(m_zbuf);
    packet->frame.value = NaGifPacket::EncodeRegister(scene->m_frame);
    packet->offset.value = NaGifPacket::EncodeRegister(scene->m_xyoffset);
    packet->scissor.value = NaGifPacket::EncodeRegister(scene->m_drawEnv->scissor1);
    packet->dithering.value = NaGifPacket::EncodeRegister(scene->m_drawEnv->dthe);

    PrWaitDmaFinish(SCE_DMA_GIF);
    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0;
    sceDmaSend(chan, &initEECoreDmaPacket);
}

INCLUDE_ASM("asm/nonmatchings/prlib/renderee", RenderVertexEECoreBothface__13PrRenderStuff);

INCLUDE_ASM("asm/nonmatchings/prlib/renderee", RenderVertexEECoreNormal__13PrRenderStuff);

void PrRenderStuff::RenderVertexEECoreRefmap() {
    /* Empty */
}

void PrRenderStuff::RenderVertexEECoreContour() {
    /* Empty */
}

// These handwritten kernels use a persistent VU register convention. They
// consume the context input and produce its output without a C++ this pointer.
extern PrEECoreVertexKernel eeCoreVertexKernels[4] asm("D_0038C670");
extern const sceDmaTag eeCoreChunkDmaTag asm("D_0038C690");
extern const NaVECTOR<float, 4> eeCoreRandomCenter asm("D_0038C6A0");
struct PrEECoreDisturbance {
    u_int seed;
    u_int reserved;
    float amplitude;
    u_int reservedTail;
};
extern PrEECoreDisturbance eeCoreDisturbance asm("D_01C831B0");
extern PrSPRAM_DATA *eeCoreScratchpad asm("D_003998EC");

void PrRenderStuff::RenderNodeEECore(PrVuNodeHeaderDmaPacket *packet) {
    PrVuDataChunkPacketHeader *first = reinterpret_cast<PrVuDataChunkPacketHeader*>(packet + 1);
    if (PrMfifoUnsentDataSize() != 0 || first->dma.id == 0x10 || static_cast<u_int>(packet->m_microprogram) >= PR_MICRO_PROGRAM_CONTOUR) {
        AppendDmaTag(&packet->m_tag);
        return;
    }

    packet = reinterpret_cast<PrVuNodeHeaderDmaPacket*>(PR_UNCACHEDACCEL(packet));
    PrEECoreContext &context = eeCoreScratchpad->m_eeCore;
    context.WaitForVuTransfers();
    NaMATRIX<float, 4, 4>::Multiply(context.screenMatrix, eeCoreScratchpad->m_view_projection_matrix, packet->m_matrix);
    context.WaitForVuTransfers();
    NaMATRIX<float, 4, 4>::Multiply(context.clipMatrix, eeCoreScratchpad->m_worldClipMatrix, packet->m_matrix);
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

    packet = reinterpret_cast<PrVuNodeHeaderDmaPacket*>(reinterpret_cast<u_int>(packet) & 0x0fffffff);
    PrVuDataChunkPacketHeader *chunk = reinterpret_cast<PrVuDataChunkPacketHeader*>(packet + 1);
    while (chunk != NULL) {
        RenderChunkEECore(chunk, packet->m_disturbance);
        chunk = chunk->Next();
    }
}

void PrRenderStuff::RenderChunkEECore(PrVuDataChunkPacketHeader *chunk, float disturbance) {
    PrSPRAM_DATA *scratchpad = eeCoreScratchpad;
    PrEECoreContext &context = scratchpad->m_eeCore;
    u_long128 *buffer = scratchpad->m_noodle_buffer[2];
    sceDmaTag *dma = reinterpret_cast<sceDmaTag*>(buffer);
    *dma = eeCoreChunkDmaTag;
    const u_int prefixQuadwords = chunk->metadata.prefixVertexCount * 3 + 1;
    dma->qwc = prefixQuadwords + chunk->metadata.vertexCount * 3;

    // The GIF prefix can contain already transformed vertices. Preserve these
    // quadwords exactly before appending the CPU-rendered vertex records.
    const u_long128 *prefix = reinterpret_cast<const u_long128*>(&chunk->gif);
    u_long128 *destination = buffer + 1;
    for (u_int i = 0; i < prefixQuadwords; i++) destination[i] = prefix[i];
    PrEECoreOutputVertex *output = reinterpret_cast<PrEECoreOutputVertex*>(destination + prefixQuadwords);
    const PrEECoreInputVertex *input = chunk->vertices + chunk->metadata.prefixVertexCount;

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
                : : "r"(&eeCoreRandomCenter), "r"(&eeCoreDisturbance), "r"(&context.input.position)
                : "memory");
        }
        context.vertexKernel();
        output[i] = context.output;
    }
    PrSendMfifo(dma);
    scratchpad = eeCoreScratchpad;
    buffer = scratchpad->m_noodle_buffer[0];
    scratchpad->m_noodle_buffer[0] = scratchpad->m_noodle_buffer[1];
    scratchpad->m_noodle_buffer[1] = scratchpad->m_noodle_buffer[2];
    scratchpad->m_noodle_buffer[2] = buffer;
}

// The original trailing queue-forwarding stub is emitted from AppendDmaTag
// in renderstuff.h when used; it is not a separate engine function.
