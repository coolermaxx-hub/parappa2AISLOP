#include "common.h"

#include "dma.h"
#include "microprogram.h"
#include "random.h"
#include "mfifo.h"
#include "renderstuff.h"
#include "spram.h"
#include "utility.h"

#include <eekernel.h>
#include <libdma.h>
#include <libgraph.h>

/* sdata */
extern float prMendererSpeed;
extern float prMendererFade;
extern u_int prMendererDrawFbp;
extern int prCurrentStage;

extern u_int prMendererWorkFbp;
extern PrSPRAM_DATA *prSpramData_tmp_mendereralpha;

/* data */
extern float alphaModulationPacket[];   /* VIF unpack of the per-strip modulation parameters */
extern u_long alphaModulationGsPacket[];
extern u_long alphaModulationFramePacket[];
extern u_int alphaModulationDmaPacket[];

/* bss */
extern float mendererDeltaRotation[8];
extern float alphaWeight[8];

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendereralpha", PrInitializeAlphaModulation__Fv);
#else /* Codegen differs (162 vs 164 instructions) */
void PrInitializeAlphaModulation() {
    float *param = &alphaModulationPacket[4];

    for (u_int i = 0; i < 8; i++) {
        param[i] = PrFloatRandom() * 7.0f + 14.0f;
        param[i + 8] = 0.0f;
        param[i + 16] = PrFloatRandom() * 0.8f - 0.4f;
        param[i + 24] = PrFloatRandom() * 0.8f - 0.4f;

        float weight = PrFloatRandom() * 0.5f + 0.5f;
        param[i + 32] = weight;
        param[i + 40] = weight;

        alphaWeight[i] = (param[i] * 0.5f - 3.5f) * 24.0f + 32.0f;

        mendererDeltaRotation[i] = PrFloatRandom() * 0.01f + 0.006f;
        if (PrRandom() & 1) {
            mendererDeltaRotation[i] = -mendererDeltaRotation[i];
        }
    }

    u_long tw = PrGetBitSize(640);
    u_long th = PrGetBitSize(224);
    u_long zbp = prRenderStuff.m_zbuf.ZBP;

    alphaModulationGsPacket[2] = SCE_GS_SET_FRAME(zbp, 10, 0, 0);
    alphaModulationFramePacket[4] = SCE_GS_SET_TEX0(zbp * 32, 10, 0, tw, th, 1, 1, 0, 0, 0, 0, 0);
    alphaModulationDmaPacket[10] = PrGetMendererDrawMeshAddress() | 0x14000000; /* MSCAL */
}
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendereralpha", PrCreateAlphaModulation__Ff);
#else /* Codegen differs (110 vs 108 instructions) */
void PrCreateAlphaModulation(float alpha) {
    if (prCurrentStage == 6 || prCurrentStage == 16) {
        alpha = 0.0f;
    }

    float *param = &alphaModulationPacket[4];

    for (u_int i = 0; i < 8; i++) {
        float phase = param[i + 8] + prMendererSpeed * mendererDeltaRotation[i];
        param[i + 8] = phase;

        if (phase > 1.5707964f) {
            param[i + 8] = 3.1415927f - phase;
            mendererDeltaRotation[i] = -mendererDeltaRotation[i];
            param[i + 40] = -param[i + 40];
        } else if (phase < -1.5707964f) {
            param[i + 8] = -3.1415927f - phase;
            mendererDeltaRotation[i] = -mendererDeltaRotation[i];
            param[i + 40] = -param[i + 40];
        }

        param[i + 48] = alphaWeight[i] * alpha;
    }

    alphaModulationPacket[63] = prMendererFade * 128.0f;

    u_long frame = SCE_GS_SET_FRAME(prMendererDrawFbp, 10, 0, 0);
    alphaModulationFramePacket[22] = frame;
    alphaModulationFramePacket[2] = frame | SCE_GS_SET_FRAME(0, 0, 0, 0xFFFFFF);

    FlushCache(WRITEBACK_DCACHE);
    PrWaitDmaFinish(SCE_DMA_VIF1);
    sceDmaSend(sceDmaGetChan(SCE_DMA_VIF1), alphaModulationDmaPacket);
}
#endif

/* rodata */
extern const sceDmaTag D_00396980;   /* DMAcnt */
extern const u_long D_00396990[2];   /* GIFtag, A+D */

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendereralpha", PrBlendNoodleImage__Fb);
#else /* Scheduling; the A+D tail is written through a pointer that is bumped per store */
void PrBlendNoodleImage(bool clear) {
    u_long128 *buf = prSpramData_tmp_mendereralpha->GetNextNoodleBuffer();

    u_long *p = (u_long*)buf;
    *(sceDmaTag*)p = D_00396980;
    p[2] = D_00396990[0];
    p[3] = D_00396990[1];

    p[4] = 0;
    p[5] = SCE_GS_TEXFLUSH;
    p[6] = SCE_GS_SET_TEX0(prMendererWorkFbp * 32, 10, 0, 10, 8, 1, 1, 0, 0, 0, 0, 0);
    p[7] = SCE_GS_TEX0_1;
    p[8] = 0;
    p[9] = SCE_GS_TEX1_1;
    p[10] = SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0);
    p[11] = SCE_GS_CLAMP_1;
    p[12] = SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);
    p[13] = SCE_GS_TEST_1;

    if (clear) {
        p[14] = SCE_GS_SET_ALPHA(0, 1, 2, 1, 0x40);
    } else {
        p[14] = SCE_GS_SET_ALPHA(0, 1, 1, 1, 0);
    }

    u_long *ad = &p[15];
    *ad++ = SCE_GS_ALPHA_1;
    *ad++ = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 1, 1, 0, 1, 0, 1, 0, 0);
    *ad++ = SCE_GS_PRIM;
    *ad++ = SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0);
    *ad++ = SCE_GS_RGBAQ;
    *ad++ = SCE_GS_SET_UV(8, 8);
    *ad++ = SCE_GS_UV;
    *ad++ = SCE_GS_SET_XYZ(0x6C00, 0x7900, 0);
    *ad++ = SCE_GS_XYZ2;
    *ad++ = SCE_GS_SET_UV(0x2808, 0xE08);
    *ad++ = SCE_GS_UV;
    *ad++ = SCE_GS_SET_XYZ(0x9400, 0x8700, 0);
    *ad = SCE_GS_XYZ2;

    PrSendMfifo((sceDmaTag*)buf);
}
#endif

INCLUDE_RODATA("asm/nonmatchings/prlib/mendereralpha", D_00396980);

INCLUDE_RODATA("asm/nonmatchings/prlib/mendereralpha", D_00396990);
