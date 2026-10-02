#include "common.h"

#include "dma.h"
#include "random.h"
#include "tim2.h"
#include "wave.h"

#include <eekernel.h>
#include <eetypes.h>
#include <libdma.h>
#include <libgraph.h>

/* data */
extern u_long mendererFadeData[7][2];

extern float prMendererNoodleColor[];

/* sdata */
extern float awfulAngle;
extern float prMendererSpeed;

/* sbss */
extern TIM2_PICTUREHEADER *awfulPicture;
extern u_int mendererAwfulColor;
extern int awfulStatus;
extern u_int awfulChangeTimer;
extern float awfulRotation;

/* bss */
extern WAVE_STR awfulWave;

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

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", PrStartAwfulRotation__Fv);
#else /* Requires .lit4 migration */
void PrStartAwfulRotation() {
    awfulStatus = 0;
    awfulRotation = 0.016f;
    SetNextSwitchRotationTimer();
}
#endif

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

INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", PrDrawAwfulBackground__FG10sceGsFrame);

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
INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", Set__t8NaMATRIX3Zfi2i2RCfT1T1T1);

INCLUDE_ASM("asm/nonmatchings/prlib/mendererawful", func_00150D10);
