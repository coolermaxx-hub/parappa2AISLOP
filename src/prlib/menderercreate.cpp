#include "common.h"

#include <eekernel.h>
#include <libdma.h>
#include <libgraph.h>
#include <math.h>

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
#else /* Requires .lit4 migration */
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

INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", PrInitializeTextureCreation__FUiUiUiUi);

extern u_long128 mendererCreatePacket[];

void PrWaitDmaFinish(u_int channel);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", CreateMendererTexture__Ff);
#else /* Requires .lit4 migration */
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
#else /* Requires .lit4 migration */
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

    float ratio;
    if (phase < 1.0f) {
        ratio = 0.0f;
    } else if (phase < 2.0f) {
        ratio = phase - 1.0f;
    } else if (phase < 4.0f) {
        ratio = 1.0f;
    } else {
        ratio = 5.0f - phase;
    }

    mendererSyncPhase = phase;
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
