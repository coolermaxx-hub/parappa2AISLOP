#include "common.h"

#include <eekernel.h>
#include <libdma.h>
#include <libgraph.h>
#include <math.h>

#include "microprogram.h"
#include "renderstuff.h"

extern u_int prMendererDrawFbp;
extern float prMendererSpeed;
extern float prMendererSyncRatio;
extern float prMendererRatio;
extern int prCurrentStage;

extern float mendererSyncPhase;
extern float noodlePhase[5][3];
extern float noodleParameter[5][60];
extern const float D_00396800[5][3][4];

void PrUpdateMendererSpeed();
void PrUpdateAwfulMenderer();
void CreateMendererTexture(float ratio);
void PrSynchronizeMendererParameter(float ratio);

/* data */
extern int mendererTextureRequested;
extern u_long mendererTexturePacket[];

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", PrSynchronizeMendererParameter__Ff);
#else /* Codegen differs (112 vs 128 instructions) */
void PrSynchronizeMendererParameter(float ratio) {
    float inv = 1.0f - ratio;

    for (u_int i = 0; i < 5; i++) {
        float *param = noodleParameter[i];

        for (u_int j = 0; j < 3; j++) {
            const float *src = D_00396800[i][j];

            param[j] = src[0];
            param[j + 4] = src[1];
            param[j + 8] = src[2] * 0.15f;
            param[j + 12] = src[3];

            if (i != 0) {
                param[j] = inv * param[j] + ratio * noodleParameter[0][j];
                param[j + 4] = inv * param[j + 4] + ratio * noodleParameter[0][j + 4];
                param[j + 8] = inv * param[j + 8] + ratio * noodleParameter[0][j + 8];
                param[j + 12] = inv * param[j + 12] + ratio * noodleParameter[0][j + 12];
            }
        }

        if (i != 0 && ratio == 1.0f) {
            for (u_int j = 0; j < 3; j++) {
                float half = (1.0f / param[j + 8]) * 0.5f;
                float diff = noodlePhase[0][j] - noodlePhase[i][j];
                if (half <= fabsf(diff)) {
                    diff = -diff;
                }
                noodlePhase[i][j] += diff / 90.0f;
            }
        }
    }
}
#endif

extern u_long128 mendererCreatePacket[];
extern int mendererTextureCreationInitialized; /* D_003999A0 */

/* Per-noodle record in noodleParameter: 20 floats of parameters, then a GIF packet */
typedef struct {
    sceGifTag adTag;
    u_long frame, frameAddr;
    u_long xyoffset, xyoffsetAddr;
    u_long scissor, scissorAddr;
    u_long rgbaq, rgbaqAddr;
    u_long prim, primAddr;
    u_long clamp, clampAddr;
    u_long tex0, tex0Addr;
    u_long tex1, tex1Addr;
    sceGifTag spriteTag;
} MendererNoodlePacket;

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", PrInitializeTextureCreation__FUiUiUiUi);
#else /* Reconstructed from the asm; GS register values decoded by hand */
void PrInitializeTextureCreation(u_int tbp, u_int zbp, u_int tw, u_int th) {
    sceGsZbuf zbuf = prRenderStuff.m_zbuf;
    u_int zbpBlocks = zbp << 5;
    u_long dbp = (u_long)tbp << 32;

    PrSynchronizeMendererParameter(0.0f);

    /* TEX0: TBP0 = tbp, TBW = 4, PSM = CT32, TW/TH, TCC = 1 */
    u_long tex0 = (u_long)((tbp | 0x10000)) | ((u_long)1 << 34) | ((u_long)tw << 26) | ((u_long)th << 30);
    /* FRAME: FBP = zbp, FBW = 10 */
    u_long frame = (u_long)(zbp | 0xA0000);

    for (u_int i = 0; i < 5; i++) {
        float *param = noodleParameter[i];
        MendererNoodlePacket *pkt = (MendererNoodlePacket*)&param[20];

        param[3] = 0.0f;
        param[7] = 0.0f;
        param[11] = 0.0f;
        param[15] = 0.0f;
        param[16] = 0.0f;
        param[17] = 0.0f;
        param[18] = 0.0f;
        param[19] = (float)(i * 16);

        /* 8 A+D registers */
        pkt->adTag.NLOOP = 8;
        pkt->adTag.EOP = 0;
        pkt->adTag.FLG = 0;
        pkt->adTag.NREG = 1;
        pkt->adTag.REGS0 = 0xE;

        pkt->frame = frame;
        pkt->frameAddr = 0x4C;                /* FRAME_1 */
        pkt->xyoffset = 0;
        pkt->xyoffsetAddr = 0x18;             /* XYOFFSET_1 */
        pkt->scissor = ((u_long)(i * 16 + 15) << 48) | ((u_long)(i * 16) << 32) | 0x27F0000;
        pkt->scissorAddr = 0x40;              /* SCISSOR_1 */
        pkt->rgbaq = 0x3FE0000080808080;
        pkt->rgbaqAddr = 1;                   /* RGBAQ */
        pkt->prim = 0x1E;                     /* sprite, IIP, TME */
        pkt->primAddr = 0;                    /* PRIM */
        pkt->clamp = 0x3C003FC00A;            /* region clamp 256x16 */
        pkt->clampAddr = 8;                   /* CLAMP_1 */
        pkt->tex0 = tex0;
        pkt->tex0Addr = 6;                    /* TEX0_1 */
        pkt->tex1 = 0x60;                     /* linear mag/min */
        pkt->tex1Addr = 0x14;                 /* TEX1_1 */

        /* 64 sprites: ST, RGBAQ, XYZ2 x 2 */
        pkt->spriteTag.NLOOP = 0x40;
        pkt->spriteTag.EOP = 1;
        pkt->spriteTag.FLG = 0;
        pkt->spriteTag.NREG = 6;
        pkt->spriteTag.REGS0 = 2;
        pkt->spriteTag.REGS1 = 1;
        pkt->spriteTag.REGS2 = 5;
        pkt->spriteTag.REGS3 = 2;
        pkt->spriteTag.REGS4 = 1;
        pkt->spriteTag.REGS5 = 5;
    }

    /* Packet that uploads the noodle parameters to VU1 and runs the microprogram */
    sceDmaTag *dmaTag = (sceDmaTag*)&mendererCreatePacket[0];
    dmaTag->qwc = 3;
    dmaTag->id = 0x10;                        /* cnt */
    dmaTag->next = NULL;
    dmaTag->p[0] = 0x11000000;                /* FLUSH */
    dmaTag->p[1] = 0x50000003;                /* DIRECT, 3 qwords */

    sceGifTag *gifTag = (sceGifTag*)&mendererCreatePacket[1];
    gifTag->NLOOP = 2;
    gifTag->EOP = 1;
    gifTag->FLG = 0;
    gifTag->NREG = 1;
    gifTag->REGS0 = 0xE;

    u_long *ad = (u_long*)&mendererCreatePacket[2];
    ad[0] = 0x30000;                          /* TEST_1: ZTE, ZTST ALWAYS */
    ad[1] = 0x47;
    ad[2] = *(u_long*)&zbuf | ((u_long)1 << 32); /* ZBUF_1 with ZMSK */
    ad[3] = 0x4E;

    dmaTag = (sceDmaTag*)&mendererCreatePacket[4];
    dmaTag->qwc = 0x4B;                       /* 5 * 15 qwords of noodleParameter */
    dmaTag->id = 0x30;                        /* ref */
    dmaTag->next = (sceDmaTag*)noodleParameter;
    dmaTag->p[0] = 0;
    dmaTag->p[1] = 0x6C4B0000;                /* UNPACK V4-32, NUM = 0x4B, ADDR 0 */

    dmaTag = (sceDmaTag*)&mendererCreatePacket[5];
    dmaTag->qwc = 0;
    dmaTag->id = 0x70;                        /* end */
    dmaTag->next = NULL;
    dmaTag->p[0] = 0x11000000;                /* FLUSH */
    dmaTag->p[1] = 0x14000000 | PrGetMendererCreateTextureAddress(); /* MSCAL */

    /* Packet that copies the finished texture from the z buffer area to tbp */
    sceGifTag *texTag = (sceGifTag*)&mendererTexturePacket[0];
    texTag->NLOOP = 13;
    texTag->EOP = 1;
    texTag->FLG = 0;
    texTag->NREG = 1;
    texTag->REGS0 = 0xE;

    mendererTexturePacket[2] = (u_long)(zbpBlocks | 0xA0000) | dbp | ((u_long)4 << 48); /* BITBLTBUF */
    mendererTexturePacket[3] = 0x50;
    mendererTexturePacket[4] = (u_long)16 << 48;                                        /* TRXPOS */
    mendererTexturePacket[5] = 0x51;
    mendererTexturePacket[6] = ((u_long)0x50 << 32) | 0x100;                            /* TRXREG: 256x80 */
    mendererTexturePacket[7] = 0x52;
    mendererTexturePacket[8] = 2;                                                       /* TRXDIR */
    mendererTexturePacket[9] = 0x53;
    mendererTexturePacket[11] = 0x40;                                                   /* SCISSOR_1, data filled at draw time */
    mendererTexturePacket[12] = 0;                                                      /* TEXFLUSH */
    mendererTexturePacket[13] = 0x3F;
    mendererTexturePacket[14] = *(u_long*)&zbuf;                                        /* ZBUF_1 */
    mendererTexturePacket[15] = 0x4E;
    mendererTexturePacket[16] = 0x32001;                                                /* TEST_1 */
    mendererTexturePacket[17] = 0x47;
    mendererTexturePacket[18] = 0x106;                                                  /* PRIM: sprite, FST */
    mendererTexturePacket[19] = 0;
    mendererTexturePacket[20] = 0;                                                      /* XYZ2 */
    mendererTexturePacket[21] = 5;
    mendererTexturePacket[22] = 0x6001000;                                              /* XYZ2 */
    mendererTexturePacket[23] = 5;
    mendererTexturePacket[25] = 0x4C;                                                   /* FRAME_1, data filled at draw time */
    mendererTexturePacket[27] = 0x18;                                                   /* XYOFFSET_1, data filled at draw time */

    mendererTextureCreationInitialized = 1;
}
#endif


void PrWaitDmaFinish(u_int channel);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", CreateMendererTexture__Ff);
#else /* Codegen differs (153 vs 161 instructions) */
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
        float *param = noodleParameter[i];

        for (u_int j = 0; j < 3; j++) {
            float phase = noodlePhase[i][j] + step;

            if (phase < 0.0f) {
                do {
                    phase += 1.0f / param[j + 8];
                } while (phase < 0.0f);
            } else if (1.0f / param[j + 8] <= phase) {
                do {
                    phase -= 1.0f / param[j + 8];
                } while (1.0f / param[j + 8] <= phase);
            }

            noodlePhase[i][j] = phase;
            param[j + 16] = phase;
        }
    }

    FlushCache(WRITEBACK_DCACHE);
    PrWaitDmaFinish(SCE_DMA_VIF1);

    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_VIF1);
    chan->chcr.TTE = 1;
    sceDmaSend(chan, mendererCreatePacket);
}
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", PrCreateMendererTexture);
#else /* Float register allocation */
/* Sync ratio over the 5-unit sync cycle: off, ramp up, hold, ramp down */
static inline float GetSyncRatio(float phase) {
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
    float ratio = GetSyncRatio(phase);

    prMendererSyncRatio = ratio;
    CreateMendererTexture(ratio);
}
#endif

void PrWaitMendererTexture(sceGsDrawEnv1 *env, const sceGsFrame &frame, const sceGsXyoffset &xyoffset) {
    if (mendererTextureRequested != 0) {
        mendererTextureRequested = 0;
        prMendererDrawFbp = frame.FBP;
        *(sceGsFrame*)&mendererTexturePacket[24] = frame;
        *(sceGsXyoffset*)&mendererTexturePacket[26] = xyoffset;
        *(sceGsScissor*)&mendererTexturePacket[10] = env->scissor1;

        FlushCache(WRITEBACK_DCACHE);
        sceGsSyncPath(0, 0);

        sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
        chan->chcr.TTE = 0;
        sceDmaSendN(chan, mendererTexturePacket, 14);
        sceGsSyncPath(0, 0);
    }
}

INCLUDE_RODATA("asm/nonmatchings/prlib/menderercreate", D_00396800);
