#include "common.h"

#include "menderer.h"

#include "dma.h"
#include "random.h"
#include "renderstuff.h"
#include "tim2.h"
#include "utility.h"
#include "wave.h"

#include "noodlepacket.h"

#include "nalib/namatrix.h"

#include <eekernel.h>
#include <eetypes.h>
#include <libdma.h>
#include <libgraph.h>



/* data */
static PrFadeFramePacket mendererFadeData = {
    { 6, 1, 0, 0, 0, 0, 0, 1, SCE_GIF_PACKED_AD },
    { SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1), SCE_GS_TEST_1 },
    { SCE_GS_SET_ALPHA(0, 1, 0, 1, 0), SCE_GS_ALPHA_1 },
    { 0, SCE_GS_RGBAQ },
    { SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 0, 0, 0), SCE_GS_PRIM },
    { SCE_GS_SET_XYZ(0x6C00, 0x7900, 0), SCE_GS_XYZ2 },
    { SCE_GS_SET_XYZ(0x9400, 0x8700, 0), SCE_GS_XYZ2 },
};

static PrAwfulBackgroundPacket awfulBackgroundPacket = {
    { 23, 1, 0, 0, 0, 0, 0, 1, SCE_GIF_PACKED_AD },
    { 0, SCE_GS_TEXFLUSH },
    { SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1), SCE_GS_TEST_2 },
    { 0, SCE_GS_ZBUF_2 },
    { 0, SCE_GS_ALPHA_2 },
    { 0, SCE_GS_TEX0_2 },
    { SCE_GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0), SCE_GS_TEX1_2 },
    { 0, SCE_GS_CLAMP_2 },
    { SCE_GS_SET_RGBAQ(0xFF, 0xFF, 0xFF, 0x80, 0), SCE_GS_RGBAQ },
    { SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0, 1, 0, 1, 0, 1, 1, 0), SCE_GS_PRIM },
    {
        { { 0, SCE_GS_UV }, { SCE_GS_SET_XYZ(0x6C00, 0x7900, 0), SCE_GS_XYZ2 } },
        { { 0, SCE_GS_UV }, { SCE_GS_SET_XYZ(0x9400, 0x7900, 0), SCE_GS_XYZ2 } },
        { { 0, SCE_GS_UV }, { SCE_GS_SET_XYZ(0x6C00, 0x8700, 0), SCE_GS_XYZ2 } },
        { { 0, SCE_GS_UV }, { SCE_GS_SET_XYZ(0x9400, 0x8700, 0), SCE_GS_XYZ2 } },
    },
    { SCE_GS_SET_TEST(1, 0, 0, 2, 0, 0, 1, 1), SCE_GS_TEST_2 },
    { 0, SCE_GS_ZBUF_2 },
    { SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 1, 1, 0), SCE_GS_PRIM },
    { SCE_GS_SET_XYZ(0x7C00, 0x7900, 0), SCE_GS_XYZ2 },
    { SCE_GS_SET_XYZ(0x8D00, 0x7B00, 0), SCE_GS_XYZ2 },
    { SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2), SCE_GS_TEST_2 },
};

extern float prMendererNoodleColor[];

float awfulAngle = 0.0f;

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

float GetAwfulRotation() {
    if (awfulStatus == 0 || awfulStatus == 2) {
        u_int timer = awfulChangeTimer - 1;
        if (timer == 0) {
            awfulChangeTimer = 0;
            awfulStatus = (awfulStatus + 1) % 4;
            SetNextSwitchRotationTimer();
        } else {
            awfulChangeTimer = timer;
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
    mendererFadeData.color.value =
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

void PrDrawAwfulBackground(sceGsFrame frame) {
    TIM2_PICTUREHEADER *pic = awfulPicture;
    if (pic == NULL) {
        return;
    }

    sceGsZbuf zbuf = prRenderStuff.m_zbuf;
    u_int zbp = zbuf.ZBP;
    u_int tbp = (zbp + 4) * 32;
    u_int cbp = tbp + 0x80;

    awfulBackgroundPacket.maskedZbuf.value = SCE_GS_SET_ZBUF(zbuf.ZBP, zbuf.PSM, 1);
    awfulBackgroundPacket.zbuf.value = SCE_GS_SET_ZBUF(zbuf.ZBP, zbuf.PSM, 0);

    pic->GsTex0 = SCE_GS_SET_TEX0(tbp, 4, SCE_GS_PSMT4, (u_int)PrGetBitSize(256), (u_int)PrGetBitSize(256),
                                  1, 1, cbp, SCE_GS_PSMCT16, 0, 0, 1);
    pic->GsTex1 = SCE_GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0);
    pic->GsRegs = 0x800000;
    pic->GsTexClut = 0;
    Tim2LoadPicture(pic);

    awfulBackgroundPacket.texture.value = pic->GsTex0;
    awfulBackgroundPacket.alpha.value = SCE_GS_SET_ALPHA(0, 1, 2, 1, (u_int)(prMendererFade * 128.0f));

    float angle = -awfulAngle;
    float c = cosf(angle);
    float s = sinf(angle);
    NaMATRIX<float, 2, 2> rot(c, s, -s, c);

    NaVECTOR<float, 2> p0 = rot.ApplyTransposed(NaVECTOR<float, 2>(2730.0f, 2048.0f));
    NaVECTOR<float, 2> p1 = rot.ApplyTransposed(NaVECTOR<float, 2>(-2730.0f, 2048.0f));

    awfulBackgroundPacket.vertices[0].uv.value = SCE_GS_SET_UV((u_int)(p0[0] + 8192.0f), (u_int)(p0[1] + 8192.0f));
    awfulBackgroundPacket.vertices[1].uv.value = SCE_GS_SET_UV((u_int)(p1[0] + 8192.0f), (u_int)(p1[1] + 8192.0f));
    awfulBackgroundPacket.vertices[2].uv.value = SCE_GS_SET_UV((u_int)(8192.0f - p1[0]), (u_int)(8192.0f - p1[1]));
    awfulBackgroundPacket.vertices[3].uv.value = SCE_GS_SET_UV((u_int)(8192.0f - p0[0]), (u_int)(8192.0f - p0[1]));

    PrWaitDmaFinish(SCE_DMA_GIF);
    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0;
    FlushCache(WRITEBACK_DCACHE);
    sceDmaSendN(chan, &awfulBackgroundPacket, 24);

    WaveCtrlDisp(&awfulWave, &frame);
}

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
