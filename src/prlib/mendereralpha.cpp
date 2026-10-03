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

#include <eekernel.h>
#include <libdma.h>
#include <libgraph.h>

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)0x70000000;
extern int prCurrentStage;


/* data */
extern PrNoodleAlphaParameters alphaModulationPacket;
extern PrNoodleAlphaGsPacket alphaModulationGsPacket;
extern PrNoodleAlphaFramePacket alphaModulationFramePacket;
extern PrNoodleAlphaDmaPacket alphaModulationDmaPacket;

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

    alphaModulationGsPacket.frame.value = SCE_GS_SET_FRAME(zbp, 10, 0, 0);
    alphaModulationFramePacket.texture.value = SCE_GS_SET_TEX0(zbp * 32, 10, 0, tw, th, 1, 1, 0, 0, 0, 0, 0);
    alphaModulationDmaPacket.microprogram.p[0] = PrGetMendererDrawMeshAddress() | 0x14000000; /* MSCAL */
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

    u_long frame = SCE_GS_SET_FRAME(prMendererDrawFbp, 10, 0, 0);
    alphaModulationFramePacket.frame.value = frame;
    alphaModulationFramePacket.maskedFrame.value = frame | SCE_GS_SET_FRAME(0, 0, 0, 0xFFFFFF);

    FlushCache(WRITEBACK_DCACHE);
    PrWaitDmaFinish(SCE_DMA_VIF1);
    sceDmaSend(sceDmaGetChan(SCE_DMA_VIF1), &alphaModulationDmaPacket);
}

/* rodata */
// DMAcnt of thirteen quadwords: the GIF tag plus twelve A+D registers.
static const sceDmaTag alphaBlendDmaTag = { 13, 0, 0x10 /* DMAcnt */, NULL, { 0, 0 } };
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
    packet.texture.value = SCE_GS_SET_TEX0(prMendererWorkFbp * 32, 10, 0, 10, 8, 1, 1, 0, 0, 0, 0, 0);
    packet.texture.address = SCE_GS_TEX0_1;
    packet.textureFilter.value = 0;
    packet.textureFilter.address = SCE_GS_TEX1_1;
    packet.clamp.value = SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0);
    packet.clamp.address = SCE_GS_CLAMP_1;
    packet.test.value = SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);
    packet.test.address = SCE_GS_TEST_1;
    packet.alpha.value = clear ? SCE_GS_SET_ALPHA(0, 1, 2, 1, 0x40)
                               : SCE_GS_SET_ALPHA(0, 1, 1, 1, 0);
    packet.alpha.address = SCE_GS_ALPHA_1;
    packet.primitive.value = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 1, 1, 0, 1, 0, 1, 0, 0);
    packet.primitive.address = SCE_GS_PRIM;
    packet.color.value = SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0);
    packet.color.address = SCE_GS_RGBAQ;
    packet.firstUv.value = SCE_GS_SET_UV(8, 8);
    packet.firstUv.address = SCE_GS_UV;
    packet.firstPosition.value = SCE_GS_SET_XYZ(0x6C00, 0x7900, 0);
    packet.firstPosition.address = SCE_GS_XYZ2;
    packet.secondUv.value = SCE_GS_SET_UV(0x2808, 0xE08);
    packet.secondUv.address = SCE_GS_UV;
    packet.secondPosition.value = SCE_GS_SET_XYZ(0x9400, 0x8700, 0);
    packet.secondPosition.address = SCE_GS_XYZ2;

    PrSendMfifo(&packet.dma);
}
