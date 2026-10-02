#include "common.h"

#include "dma.h"
#include "random.h"
#include "renderstuff.h"
#include "tim2.h"
#include "utility.h"
#include "wave.h"

#include "nalib/namatrix.h"

#include <eekernel.h>
#include <eetypes.h>
#include <libdma.h>
#include <libgraph.h>

/* data */
extern u_long mendererFadeData[7][2];
extern u_long awfulBackgroundPacket[24][2] asm("D_0038C9B0");

extern float prMendererNoodleColor[];

/* sdata */
extern float awfulAngle;
extern float prMendererSpeed;
extern float prMendererFade;

/* sbss */
extern TIM2_PICTUREHEADER *awfulPicture;
extern u_int mendererAwfulColor;
extern int awfulStatus;
extern u_int awfulChangeTimer;
extern float awfulRotation;

/* bss */
extern WAVE_STR awfulWave;

float GetAwfulRotation();

void SetNextSwitchRotationTimer() {
    awfulChangeTimer = (u_int)((PrFloatRandom() * 4.0f + 3.0f) * 60.0f);
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", GetAwfulRotation__Fv);
#else /* Requires .lit4 migration */
float GetAwfulRotation() {
    if (awfulStatus == 0 || awfulStatus == 2) {
        if (--awfulChangeTimer == 0) {
            awfulStatus = (awfulStatus + 1) % 4;
            SetNextSwitchRotationTimer();
        }
    }

    if (awfulStatus == 1) {
        awfulRotation -= 8.888889e-05f;
        if (awfulRotation < -0.016f) {
            awfulRotation = -0.016f;
            awfulStatus = 2;
        }
    } else if (awfulStatus == 3) {
        awfulRotation += 8.888889e-05f;
        if (awfulRotation > 0.016f) {
            awfulRotation = 0.016f;
            awfulStatus = 0;
        }
    }

    return awfulRotation;
}
#endif

void PrStartAwfulRotation() {
    awfulStatus = 0;
    awfulRotation = 0.016f;
    SetNextSwitchRotationTimer();
}

void PrFadeFrameImage(float arg0) {
    if (arg0 == 0.0f) {
        return;
    }

    u_int alp = (u_int)(arg0 * 128.0f + 0.5f);
    mendererFadeData[3][0] =
        SCE_GS_SET_RGBAQ(
            (mendererAwfulColor >> 0 ) & 255,
            (mendererAwfulColor >> 8 ) & 255,
            (mendererAwfulColor >> 16) & 255,
            alp & 255,
            0
        );

    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0;

    FlushCache(WRITEBACK_DCACHE);
    PrWaitDmaFinish(SCE_DMA_GIF);
    sceDmaSendN(chan, &mendererFadeData, 7);
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", PrInitializeAwfulBackground__FPv);
#else
/* Scheduling: the clut colour is copied through two registers and stored late */
void PrInitializeAwfulBackground(void *tim2) {
    if (tim2 != NULL) {
        awfulPicture = Tim2GetPictureHeader(tim2, 0);
        u_int color = Tim2GetClutColor(awfulPicture, 0, 0);
        mendererAwfulColor = color;
        prMendererNoodleColor[0] = (u_int)((color >> 0 ) & 255) / 255.0f;
        prMendererNoodleColor[1] = (u_int)((color >> 8 ) & 255) / 255.0f;
        prMendererNoodleColor[2] = (u_int)((color >> 16) & 255) / 255.0f;
        prMendererNoodleColor[3] = 1.0f;
    } else {
        awfulPicture = NULL;
    }

    WaveCtrlInit(&awfulWave, 640, 224, WM_WSLICE);
}
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", PrDrawAwfulBackground__FG10sceGsFrame);
#else /* First draft: TEX0 build order, stack layout and the 2x2 rotate loops still differ */
static inline NaVECTOR<float, 2> RotateVector_tmp(const NaMATRIX<float, 2, 2>& m, const NaVECTOR<float, 2>& v) {
    float r[2];
    for (int i = 0; i < 2; i++) {
        float sum = 0.0f;
        for (int j = 0; j < 2; j++) {
            sum += m[i][j] * v[j];
        }
        r[i] = sum;
    }
    return NaVECTOR<float, 2>(r[0], r[1]);
}

void PrDrawAwfulBackground(sceGsFrame frame) {
    TIM2_PICTUREHEADER *pic = awfulPicture;
    if (pic == NULL) {
        return;
    }

    sceGsZbuf zbuf = prRenderStuff.m_zbuf;
    u_int zbp = zbuf.ZBP;
    u_int tbp = (zbp + 4) * 32;
    u_int cbp = tbp + 0x80;

    awfulBackgroundPacket[3][0] = SCE_GS_SET_ZBUF(zbuf.ZBP, zbuf.PSM, 1);
    awfulBackgroundPacket[19][0] = SCE_GS_SET_ZBUF(zbuf.ZBP, zbuf.PSM, 0);

    pic->GsTex0 = SCE_GS_SET_TEX0(tbp, 4, SCE_GS_PSMT4, (u_int)PrGetBitSize(256), (u_int)PrGetBitSize(256),
                                  1, 1, cbp, SCE_GS_PSMCT16, 0, 0, 1);
    pic->GsTex1 = SCE_GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0);
    pic->GsRegs = 0x800000;
    pic->GsTexClut = 0;
    Tim2LoadPicture(pic);

    awfulBackgroundPacket[5][0] = pic->GsTex0;
    awfulBackgroundPacket[4][0] = SCE_GS_SET_ALPHA(0, 1, 2, 1, (u_int)(prMendererFade * 128.0f));

    float angle = -awfulAngle;
    float c = cosf(angle);
    float s = sinf(angle);
    NaMATRIX<float, 2, 2> rot(c, s, -s, c);

    NaVECTOR<float, 2> p0 = RotateVector_tmp(rot, NaVECTOR<float, 2>(2730.0f, 2048.0f));
    NaVECTOR<float, 2> p1 = RotateVector_tmp(rot, NaVECTOR<float, 2>(-2730.0f, 2048.0f));

    awfulBackgroundPacket[10][0] = SCE_GS_SET_UV((u_int)(p0[0] + 8192.0f), (u_int)(p0[1] + 8192.0f));
    awfulBackgroundPacket[12][0] = SCE_GS_SET_UV((u_int)(p1[0] + 8192.0f), (u_int)(p1[1] + 8192.0f));
    awfulBackgroundPacket[14][0] = SCE_GS_SET_UV((u_int)(8192.0f - p1[0]), (u_int)(8192.0f - p1[1]));
    awfulBackgroundPacket[16][0] = SCE_GS_SET_UV((u_int)(8192.0f - p0[0]), (u_int)(8192.0f - p0[1]));

    PrWaitDmaFinish(SCE_DMA_GIF);
    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0;
    FlushCache(WRITEBACK_DCACHE);
    sceDmaSendN(chan, awfulBackgroundPacket, 24);

    WaveCtrlDisp(&awfulWave, &frame);
}
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", PrUpdateAwfulMenderer__Fv);
#else /* Requires .lit4 migration */
void PrUpdateAwfulMenderer() {
    float angle = awfulAngle + prMendererSpeed * GetAwfulRotation();
    if (angle >= 6.2831855f) {
        angle -= 6.2831855f;
    }
    if (angle < 0.0f) {
        angle += 6.2831855f;
    }
    awfulAngle = angle;

    WaveCtrlUpdate(&awfulWave, prMendererSpeed);
}
#endif

/* nalib/navector.h */
#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", Set__t8NaMATRIX3Zfi2i2RCfT1T1T1);

INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", func_00150D10);
#endif
