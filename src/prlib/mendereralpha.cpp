#include "common.h"

#include "menderer.h"

#include "dma.h"
#include "microprogram.h"
#include "random.h"
#include "mfifo.h"
#include "renderstuff.h"
#include "spram.h"
#include "utility.h"
#include "noodlepacket.h"
#include "vu1/vucommon.h"
#include "gsstate.h"

#include <eekernel.h>
#include <libdma.h>
#include <libgraph.h>

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)EE_SCRATCHPAD_BASE;
extern int prCurrentStage;


/* data */
// Parameter block uploaded to VU1 memory; only the GIF-style header is static.
// PACKED, 24 loops of RGBAQ then XYZ2, with the PRIM field enabled.
static PrNoodleAlphaParameters alphaModulationPacket = {
    { 24, 1, 0, 0, 1, 0, 0, 2, 1 /* RGBAQ */, 5 /* XYZ2 */, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

// Frame and test setup for the modulation pass. The frame value is patched
// in PrInitializeAlphaModulation.
static PrNoodleAlphaGsPacket alphaModulationGsPacket = {
    { 3, 1, 0, 0, 0, 0, 0, 1, 0xe /* A+D */, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, SCE_GS_FRAME_1 },
    { 0, SCE_GS_XYOFFSET_1 },
    { PR_TEST_NO_ALPHA(SCE_GS_ZALWAYS), SCE_GS_TEST_1 },
};

// Draws the modulated buffer back to the frame as a region-clamped sprite.
// Frame and texture values are patched at runtime.
static PrNoodleAlphaFramePacket alphaModulationFramePacket = {
    { 12, 1, 0, 0, 0, 0, 0, 1, 0xe /* A+D */, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, SCE_GS_FRAME_1 },
    { 0, SCE_GS_TEX0_1 },
    { PR_TEX1_BILINEAR, SCE_GS_TEX1_1 },
    { SCE_GS_SET_CLAMP(/*WMS*/SCE_GS_REGION_CLAMP, /*WMT*/SCE_GS_REGION_CLAMP, 0, 23, 0, 15), SCE_GS_CLAMP_1 },
    { 0, SCE_GS_TEXFLUSH },
    { SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 1, 1, 0, 0, 0, 1, 0, 0), SCE_GS_PRIM },
    { SCE_GS_SET_UV(0, 0), SCE_GS_UV },
    { SCE_GS_SET_XYZ(0, 0, 0), SCE_GS_XYZ2 },
    { SCE_GS_SET_UV(0x180, 0x100), SCE_GS_UV },
    { SCE_GS_SET_XYZ(0x2800, 0xE00, 0), SCE_GS_XYZ2 },
    { 0, SCE_GS_FRAME_1 },
    { SCE_GS_SET_XYOFFSET(GS_X_COORD(0), GS_Y_COORD(0)), SCE_GS_XYOFFSET_1 },
};

// VIF1 chain: GS state (REF), parameters (REF, UNPACK), microprogram (MSCAL,
// patched at runtime) and the frame draw (REFE).
static PrNoodleAlphaDmaPacket alphaModulationDmaPacket = {
    { 4, 0, PR_DMA_TAG_REF, (sceDmaTag*)&alphaModulationGsPacket, { SCE_VIF1_SET_FLUSHE(0), SCE_VIF1_SET_DIRECT(4, 0) } },
    { 16, 0, PR_DMA_TAG_REF, (sceDmaTag*)&alphaModulationPacket, { SCE_VIF1_SET_FLUSH(0), SCE_VIF1_SET_UNPACK(0, 16, PR_VIF_UNPACK_V4_32(0), 0) } },
    { 0, 0, PR_DMA_TAG_CNT, NULL, { SCE_VIF1_SET_MSCAL(0, 0), 0 } },
    { 13, 0, PR_DMA_TAG_REFE, (sceDmaTag*)&alphaModulationFramePacket, { SCE_VIF1_SET_FLUSHE(0), SCE_VIF1_SET_DIRECT(13, 0) } },
};

static float mendererDeltaRotation[8];
static float alphaWeight[8];

void PrInitializeAlphaModulation() {
    PrNoodleAlphaParameters &parameters = alphaModulationPacket;

    for (u_int i = 0; i < 8; i++) {
        parameters.radius[i] = PrFloatRandom() * 7.0f + 14.0f;
        parameters.phase[i] = 0.0f;
        parameters.horizontalJitter[i] = PrFloatRandom() * 0.8f - 0.4f;
        parameters.verticalJitter[i] = PrFloatRandom() * 0.8f - 0.4f;

        float weight = PrFloatRandom() * 0.5f + 0.5f;
        parameters.weight[i] = weight;
        parameters.signedWeight[i] = weight;

        alphaWeight[i] = (parameters.radius[i] * 0.5f - 3.5f) * 24.0f + 32.0f;

        mendererDeltaRotation[i] = PrFloatRandom() * 0.01f + 0.006f;
        if (PrRandom() & 1) {
            mendererDeltaRotation[i] = -mendererDeltaRotation[i];
        }
    }

    u_long tw = PrGetBitSize(640);
    u_long th = PrGetBitSize(224);
    u_long zbp = prRenderStuff.m_zbuf.ZBP;

    alphaModulationGsPacket.frame.value = PR_FRAME_CT32(zbp);
    alphaModulationFramePacket.texture.value = SCE_GS_SET_TEX0(zbp * 32, 10, SCE_GS_PSMCT32, tw, th, 1, 1, 0, 0, 0, 0, 0);
    alphaModulationDmaPacket.microprogram.p[0] = SCE_VIF1_SET_MSCAL(PrGetMendererDrawMeshAddress(), 0);
}

void PrCreateAlphaModulation(float alpha) {
    if (prCurrentStage == 6 || prCurrentStage == 16) {
        alpha = 0.0f;
    }

    PrNoodleAlphaParameters &parameters = alphaModulationPacket;

    for (u_int i = 0; i < 8; i++) {
        float phase = parameters.phase[i] + prMendererSpeed * mendererDeltaRotation[i];
        parameters.phase[i] = phase;

        if (phase > 1.5707964f) {
            parameters.phase[i] = 3.1415927f - phase;
            mendererDeltaRotation[i] = -mendererDeltaRotation[i];
            parameters.signedWeight[i] = -parameters.signedWeight[i];
        } else if (phase < -1.5707964f) {
            parameters.phase[i] = -3.1415927f - phase;
            mendererDeltaRotation[i] = -mendererDeltaRotation[i];
            parameters.signedWeight[i] = -parameters.signedWeight[i];
        }

        parameters.alpha[i] = alphaWeight[i] * alpha;
    }

    alphaModulationPacket.fade = prMendererFade * 128.0f;

    u_long frame = PR_FRAME_CT32(prMendererDrawFbp);
    alphaModulationFramePacket.frame.value = frame;
    /* FBMSK masks the RGB bits, so the masked frame writes alpha only. */
    alphaModulationFramePacket.maskedFrame.value = frame | SCE_GS_SET_FRAME(0, 0, 0, 0xFFFFFF);

    FlushCache(WRITEBACK_DCACHE);
    PrWaitDmaFinish(SCE_DMA_VIF1);
    sceDmaSend(sceDmaGetChan(SCE_DMA_VIF1), &alphaModulationDmaPacket);
}

/* rodata */
// DMAcnt of thirteen quadwords: the GIF tag plus twelve A+D registers.
static const sceDmaTag alphaBlendDmaTag = { 13, 0, PR_DMA_TAG_CNT, NULL, { 0, 0 } };
// PACKED, twelve loops of one A+D register, EOP.
static const sceGifTag alphaBlendGifTag = {
    12, 1, 0, 0, 0, 0, 0, 1,
    0xe /* A+D */, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void PrBlendNoodleImage(bool clear) {
    u_long128 *buf = prSpramData->m_noodle_buffer[0];
    prSpramData->m_noodle_buffer[0] = prSpramData->m_noodle_buffer[1];
    prSpramData->m_noodle_buffer[1] = prSpramData->m_noodle_buffer[2];
    prSpramData->m_noodle_buffer[2] = buf;

    // This scratchpad bank owns a DMA/GIF wire packet during the blend pass.
    PrNoodleBlendPacket &packet = *reinterpret_cast<PrNoodleBlendPacket*>(buf);
    packet.dma = alphaBlendDmaTag;
    packet.gif = alphaBlendGifTag;

    packet.flush.value = 0;
    packet.flush.address = SCE_GS_TEXFLUSH;
    packet.texture.value = SCE_GS_SET_TEX0(prMendererWorkFbp * 32, 10, SCE_GS_PSMCT32, 10, 8, 1, 1, 0, 0, 0, 0, 0);
    packet.texture.address = SCE_GS_TEX0_1;
    packet.textureFilter.value = 0;
    packet.textureFilter.address = SCE_GS_TEX1_1;
    packet.clamp.value = SCE_GS_SET_CLAMP(/*WMS*/SCE_GS_CLAMP, /*WMT*/SCE_GS_CLAMP, 0, 0, 0, 0);
    packet.clamp.address = SCE_GS_CLAMP_1;
    packet.test.value = PR_TEST_NO_ALPHA(SCE_GS_ZALWAYS);
    packet.test.address = SCE_GS_TEST_1;
    /* Clearing mixes the work buffer in at a fixed 50%; otherwise it is blended by the destination alpha. */
    packet.alpha.value = clear ? PR_ALPHA_FIXED(0x40)
                               : SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_AD, SCE_GS_ALPHA_CD, 0);
    packet.alpha.address = SCE_GS_ALPHA_1;
    packet.primitive.value = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 1, 1, 0, 1, 0, 1, 0, 0);
    packet.primitive.address = SCE_GS_PRIM;
    packet.color.value = SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0);
    packet.color.address = SCE_GS_RGBAQ;
    packet.firstUv.value = SCE_GS_SET_UV(8, 8);
    packet.firstUv.address = SCE_GS_UV;
    packet.firstPosition.value = SCE_GS_SET_XYZ(GS_X_COORD(0), GS_Y_COORD(0), 0);
    packet.firstPosition.address = SCE_GS_XYZ2;
    packet.secondUv.value = SCE_GS_SET_UV(0x2808, 0xE08);
    packet.secondUv.address = SCE_GS_UV;
    packet.secondPosition.value = SCE_GS_SET_XYZ(0x9400, 0x8700, 0);
    packet.secondPosition.address = SCE_GS_XYZ2;

    PrSendMfifo(&packet.dma);
}
