#include "common.h"

#include "menderer.h"

#include <eekernel.h>
#include <libdma.h>
#include <libgraph.h>
#include <math.h>

#include "microprogram.h"
#include "renderstuff.h"
#include "noodlepacket.h"
#include "nalib/napacket.h"
#include "vu1/vucommon.h"

extern int prCurrentStage;

int mendererTextureCreationInitialized = 0;
int mendererTextureRequested = 0;
float mendererSyncPhase = 0.0f;
static float noodlePhase[5][3];
static PrNoodleTextureParameters noodleParameter[5];
static PrNoodleTextureCreationPacket mendererCreatePacket;
static PrNoodleTextureCopyPacket mendererTexturePacket;

void PrUpdateMendererSpeed();
void PrUpdateAwfulMenderer();
void CreateMendererTexture(float ratio);
void PrSynchronizeMendererParameter(float ratio);


/* Per-preset wave parameters: amplitude, spatial cycles, temporal frequency (x0.15), phase offset.
   Only the first five presets are read by PrSynchronizeMendererParameter. */
static const float noodleWavePresets[8][3][4] = {
    {
        { 1.0f, 2.0f, 1.6f, 0.0f },
        { 1.0f, -3.0f, 3.2f, 0.6f },
        { 1.0f, 5.0f, 4.0f, 0.2f },
    },
    {
        { 1.0f, 2.0f, 2.1f, 0.0f },
        { 1.0f, -3.0f, 0.8f, 0.6f },
        { 1.0f, 5.0f, 3.6f, 0.2f },
    },
    {
        { 1.0f, 2.0f, 0.6f, 0.0f },
        { 1.0f, -3.0f, 2.7f, 0.6f },
        { 1.0f, 5.0f, 1.9f, 0.2f },
    },
    {
        { 1.0f, 2.0f, 2.4f, 0.0f },
        { 1.0f, -3.0f, 1.4f, 0.6f },
        { 1.0f, 5.0f, 3.0f, 0.2f },
    },
    {
        { 1.0f, 2.0f, 0.9f, 0.0f },
        { 1.0f, -3.0f, 3.4f, 0.6f },
        { 1.0f, 5.0f, 2.5f, 0.2f },
    },
    {
        { 1.0f, 2.0f, 1.2f, 0.0f },
        { 1.0f, -3.0f, 2.3f, 0.6f },
        { 1.0f, 5.0f, 4.3f, 0.2f },
    },
    {
        { 1.0f, 2.0f, 0.7f, 0.0f },
        { 1.0f, -3.0f, 1.8f, 0.6f },
        { 1.0f, 5.0f, 2.9f, 0.2f },
    },
    {
        { 1.0f, 2.0f, 2.2f, 0.0f },
        { 1.0f, -3.0f, 0.5f, 0.6f },
        { 1.0f, 5.0f, 1.7f, 0.2f },
    },
};

void PrSynchronizeMendererParameter(float ratio) {
    float inv = 1.0f - ratio;

    for (u_int i = 0; i < 5; i++) {
        PrNoodleTextureParameters& param = noodleParameter[i];

        for (u_int j = 0; j < 3; j++) {
            const float *src = noodleWavePresets[i][j];

            param.amplitude[j] = src[0];
            param.spatialCycles[j] = src[1];
            param.temporalFrequency[j] = src[2] * 0.15f;
            param.phaseOffsetCycles[j] = src[3];

            if (i != 0) {
                param.amplitude[j] = inv * param.amplitude[j] + ratio * noodleParameter[0].amplitude[j];
                param.spatialCycles[j] = inv * param.spatialCycles[j] + ratio * noodleParameter[0].spatialCycles[j];
                param.temporalFrequency[j] = inv * param.temporalFrequency[j] + ratio * noodleParameter[0].temporalFrequency[j];
                param.phaseOffsetCycles[j] = inv * param.phaseOffsetCycles[j] + ratio * noodleParameter[0].phaseOffsetCycles[j];
            }
        }

        if (i != 0 && ratio == 1.0f) {
            for (u_int j = 0; j < 3; j++) {
                float half = (1.0f / param.temporalFrequency[j]) * 0.5f;
                float diff = noodlePhase[0][j] - noodlePhase[i][j];
                if (half <= fabsf(diff)) {
                    diff = -diff;
                }
                noodlePhase[i][j] += diff / 90.0f;
            }
        }
    }
}


void PrInitializeTextureCreation(u_int tbp, u_int zbp, u_int tw, u_int th) {
    sceGsZbuf zbuf = prRenderStuff.m_zbuf;
    u_int zbpBlocks = zbp << 5;

    PrSynchronizeMendererParameter(0.0f);

    /* TEX0: TBP0 = tbp, TBW = 4, PSM = CT32, TW/TH, TCC = 1 */
    u_long tex0 = SCE_GS_SET_TEX0(tbp, 4, SCE_GS_PSMCT32, tw, th, 1, 0, 0, 0, 0, 0, 0);
    /* FRAME: FBP = zbp, FBW = 10 */
    u_long frame = SCE_GS_SET_FRAME(zbp, 10, SCE_GS_PSMCT32, 0);

    for (u_int i = 0; i < 5; i++) {
        PrNoodleTextureParameters& param = noodleParameter[i];
        PrNoodleTextureDrawPacket& packet = param.packet;

        param.amplitude[3] = 0.0f;
        param.spatialCycles[3] = 0.0f;
        param.temporalFrequency[3] = 0.0f;
        param.phaseOffsetCycles[3] = 0.0f;
        param.phaseTime[0] = 0.0f;
        param.phaseTime[1] = 0.0f;
        param.phaseTime[2] = 0.0f;
        param.bandOffset = (float)(i * 16);

        /* 8 A+D registers */
        packet.stateTag.NLOOP = 8;
        packet.stateTag.EOP = 0;
        packet.stateTag.FLG = 0;
        packet.stateTag.NREG = 1;
        packet.stateTag.REGS0 = 0xE;

        packet.state[0].value = frame;
        packet.state[0].address = SCE_GS_FRAME_1;
        packet.state[1].value = 0;
        packet.state[1].address = SCE_GS_XYOFFSET_1;
        packet.state[2].value = SCE_GS_SET_SCISSOR(0, 0x27F, i * 16, i * 16 + 15);
        packet.state[2].address = SCE_GS_SCISSOR_1;
        packet.state[3].value = SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3FE00000);
        packet.state[3].address = SCE_GS_RGBAQ;
        packet.state[4].value = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 1, 1, 0, 0, 0, 0, 0, 0);
        packet.state[4].address = SCE_GS_PRIM;
        packet.state[5].value = SCE_GS_SET_CLAMP(2, 2, 0, 255, 0, 15); /* region clamp 256x16 */
        packet.state[5].address = SCE_GS_CLAMP_1;
        packet.state[6].value = tex0;
        packet.state[6].address = SCE_GS_TEX0_1;
        packet.state[7].value = SCE_GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0); /* linear mag/min */
        packet.state[7].address = SCE_GS_TEX1_1;

        /* 64 sprites: ST, RGBAQ, XYZ2 x 2 */
        packet.spriteTag.NLOOP = 0x40;
        packet.spriteTag.EOP = 1;
        packet.spriteTag.FLG = 0;
        packet.spriteTag.NREG = 6;
        packet.spriteTag.REGS0 = 2;
        packet.spriteTag.REGS1 = 1;
        packet.spriteTag.REGS2 = 5;
        packet.spriteTag.REGS3 = 2;
        packet.spriteTag.REGS4 = 1;
        packet.spriteTag.REGS5 = 5;
    }

    /* Packet that uploads the noodle parameters to VU1 and runs the microprogram */
    sceDmaTag *dmaTag = &mendererCreatePacket.stateDma;
    dmaTag->qwc = 3;
    dmaTag->id = 0x10;                        /* cnt */
    dmaTag->next = NULL;
    dmaTag->p[0] = SCE_VIF1_SET_FLUSH(0);
    dmaTag->p[1] = SCE_VIF1_SET_DIRECT(3, 0);

    sceGifTag *gifTag = &mendererCreatePacket.stateGif;
    gifTag->NLOOP = 2;
    gifTag->EOP = 1;
    gifTag->FLG = 0;
    gifTag->NREG = 1;
    gifTag->REGS0 = 0xE;

    mendererCreatePacket.state[0].value = 0x30000; /* TEST_1: ZTE, ZTST ALWAYS */
    mendererCreatePacket.state[0].address = SCE_GS_TEST_1;
    mendererCreatePacket.state[1].value = NaGifPacketWrapper::EncodeRegister(zbuf) | (u_long(1) << 32);
    mendererCreatePacket.state[1].address = SCE_GS_ZBUF_1;

    dmaTag = &mendererCreatePacket.parameterDma;
    dmaTag->qwc = 0x4B;                       /* 5 * 15 qwords of noodleParameter */
    dmaTag->id = 0x30;                        /* ref */
    dmaTag->next = (sceDmaTag*)noodleParameter;
    dmaTag->p[0] = 0;
    dmaTag->p[1] = SCE_VIF1_SET_UNPACK(0, 0x4B, PR_VIF_UNPACK_V4_32(0), 0);

    dmaTag = &mendererCreatePacket.endDma;
    dmaTag->qwc = 0;
    dmaTag->id = 0x70;                        /* end */
    dmaTag->next = NULL;
    dmaTag->p[0] = SCE_VIF1_SET_FLUSH(0);
    dmaTag->p[1] = SCE_VIF1_SET_MSCAL(PrGetMendererCreateTextureAddress(), 0);

    /* Packet that copies the finished texture from the z buffer area to tbp */
    sceGifTag *texTag = &mendererTexturePacket.tag;
    texTag->NLOOP = 13;
    texTag->EOP = 1;
    texTag->FLG = 0;
    texTag->NREG = 1;
    texTag->REGS0 = 0xE;

    mendererTexturePacket.state[0].value = SCE_GS_SET_BITBLTBUF(zbpBlocks, 10, SCE_GS_PSMCT32, tbp, 4, SCE_GS_PSMCT32);
    mendererTexturePacket.state[0].address = SCE_GS_BITBLTBUF;
    mendererTexturePacket.state[1].value = SCE_GS_SET_TRXPOS(0, 0, 0, 16, 0);
    mendererTexturePacket.state[1].address = SCE_GS_TRXPOS;
    mendererTexturePacket.state[2].value = SCE_GS_SET_TRXREG(256, 80);
    mendererTexturePacket.state[2].address = SCE_GS_TRXREG;
    mendererTexturePacket.state[3].value = 2; /* local to local */
    mendererTexturePacket.state[3].address = SCE_GS_TRXDIR;
    mendererTexturePacket.state[4].address = SCE_GS_SCISSOR_1; /* data filled at draw time */
    mendererTexturePacket.state[5].value = 0;
    mendererTexturePacket.state[5].address = SCE_GS_TEXFLUSH;
    mendererTexturePacket.state[6].value = NaGifPacketWrapper::EncodeRegister(zbuf);
    mendererTexturePacket.state[6].address = SCE_GS_ZBUF_1;
    mendererTexturePacket.state[7].value = SCE_GS_SET_TEST(1, 0, 0, 2, 0, 0, 1, 1);
    mendererTexturePacket.state[7].address = SCE_GS_TEST_1;
    mendererTexturePacket.state[8].value = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 1, 0, 0);
    mendererTexturePacket.state[8].address = SCE_GS_PRIM;
    mendererTexturePacket.state[9].value = 0;
    mendererTexturePacket.state[9].address = SCE_GS_XYZ2;
    mendererTexturePacket.state[10].value = SCE_GS_SET_XYZ(0x1000, 0x600, 0);
    mendererTexturePacket.state[10].address = SCE_GS_XYZ2;
    mendererTexturePacket.state[11].address = SCE_GS_FRAME_1; /* data filled at draw time */
    mendererTexturePacket.state[12].address = SCE_GS_XYOFFSET_1; /* data filled at draw time */

    mendererTextureCreationInitialized = 1;
}


void PrWaitDmaFinish(u_int channel);

void CreateMendererTexture(float ratio) {
    PrSynchronizeMendererParameter(ratio);

    float scale;
    float r = prMendererRatio;
    if (r <= 1.0f) {
        scale = 1.0f;
    } else if (r <= 1.4f) {
        scale = (r - 1.0f) * 5.6666665f / 0.4f + 1.0f;
    } else if (r <= 1.6f) {
        scale = 6.6666665f;
    } else {
        scale = (2.0f - r) * 5.6666665f / 0.4f + 1.0f;
    }

    mendererTextureRequested = 1;
    float step = scale * prMendererSpeed / 60.0f;

    for (u_int i = 0; i < 5; i++) {
        PrNoodleTextureParameters& param = noodleParameter[i];

        for (u_int j = 0; j < 3; j++) {
            float phase = noodlePhase[i][j] + step;

            if (phase < 0.0f) {
                do {
                    phase += 1.0f / param.temporalFrequency[j];
                } while (phase < 0.0f);
            } else if (1.0f / param.temporalFrequency[j] <= phase) {
                do {
                    phase -= 1.0f / param.temporalFrequency[j];
                } while (1.0f / param.temporalFrequency[j] <= phase);
            }

            noodlePhase[i][j] = phase;
            param.phaseTime[j] = phase;
        }
    }

    FlushCache(WRITEBACK_DCACHE);
    PrWaitDmaFinish(SCE_DMA_VIF1);

    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_VIF1);
    chan->chcr.TTE = 1;
    sceDmaSend(chan, &mendererCreatePacket);
}

namespace {
float SynchronizationRatio(float phase) {
    if (phase < 1.0f) {
        return 0.0f;
    } else if (phase < 2.0f) {
        return phase - 1.0f;
    } else if (phase < 4.0f) {
        return 1.0f;
    } else {
        return 5.0f - phase;
    }
}
}

PR_EXTERN
void PrCreateMendererTexture() {
    PrUpdateMendererSpeed();
    PrUpdateAwfulMenderer();

    if (prCurrentStage == 19 || prCurrentStage == 6 || prCurrentStage == 8 || prCurrentStage == 18) {
        CreateMendererTexture(0.0f);
        return;
    }

    float phase = mendererSyncPhase + prMendererSpeed * 0.003f;
    if (phase >= 5.0f) {
        phase -= 5.0f;
    }

    mendererSyncPhase = phase;
    float ratio = SynchronizationRatio(phase);

    prMendererSyncRatio = ratio;
    CreateMendererTexture(ratio);
}

void PrWaitMendererTexture(sceGsDrawEnv1 *env, const sceGsFrame &frame, const sceGsXyoffset &xyoffset) {
    if (mendererTextureRequested != 0) {
        mendererTextureRequested = 0;
        prMendererDrawFbp = frame.FBP;
        mendererTexturePacket.state[11].value = NaGifPacketWrapper::EncodeRegister(frame);
        mendererTexturePacket.state[12].value = NaGifPacketWrapper::EncodeRegister(xyoffset);
        mendererTexturePacket.state[4].value = NaGifPacketWrapper::EncodeRegister(env->scissor1);

        FlushCache(WRITEBACK_DCACHE);
        sceGsSyncPath(0, 0);

        sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
        chan->chcr.TTE = 0;
        sceDmaSendN(chan, &mendererTexturePacket, 14);
        sceGsSyncPath(0, 0);
    }
}

