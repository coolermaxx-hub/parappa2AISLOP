#include "wave.h"

#include "renderstuff.h"

#include <libgifpk.h>
#include <libgraph.h>

#include <math.h>

static int WV_SCREEN_W;
static int WV_SCREEN_H;

static u_long128 cmnGifTr[904];

static void CG_WaveInit(WAVE_STR *wstr) {
    wstr->currentAng = 0.0f;
}

static void CG_WaveInitEasy(WAVE_STR *wstr, short x, short y, short w, short h, WMODE_ENUM wmode) {
    wstr->wmode = wmode;
    wstr->x = x;
    wstr->y = y;

    if (wmode == WM_WSLICE) {
        wstr->linecnt = h - 1;

        wstr->sizeW = w;
        wstr->sizeH = 1;

        wstr->addW = 0;
        wstr->addH = 1;

        wstr->u = (x - 2048) + (WV_SCREEN_W / 2);
        wstr->v = y + ((WV_SCREEN_H / 2) - 2047);

        wstr->addU = 0;
        wstr->addV = 1;
    } else {
        wstr->linecnt = w - 1;

        wstr->sizeW = 1;
        wstr->sizeH = h;

        wstr->addW = 1;
        wstr->addH = 0;

        wstr->u = x + ((WV_SCREEN_W / 2) - 2047);
        wstr->v = (y - 2048) + (WV_SCREEN_H / 2);

        wstr->addU = 1;
        wstr->addV = 0;
    }

    wstr->currentAng = 0.0f;
}

static void UG_WaveDisp(WAVE_STR *wstr, sceGsFrame *frame_pp, sceGifPacket *wavePkSpr) {
    float tmpAngle;
    int   haba_now_u, haba_now_v;
    int   tmp_u, tmp_v;
    int   tmp_x, tmp_y;
    int   tmp_w, tmp_h;

    sceGifPkAddGsAD(wavePkSpr, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_TEST_2, SCE_GS_SET_TEST(/*ATE*/false, /*ATST*/SCE_GS_ALPHA_NEVER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_KEEP,
                                                              /*DATE*/false, /*DATM*/0, /*ZTE*/true, /*ZTST*/SCE_GS_DEPTH_ALWAYS));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_ZBUF_2, SCE_GS_SET_ZBUF(/*ZBP*/prRenderStuff.m_zbuf.ZBP, /*PSM*/prRenderStuff.m_zbuf.PSM, /*ZMSK*/1));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_TEX0_2, SCE_GS_SET_TEX0(/*TBP0*/frame_pp->FBP << 5, /*TBW*/frame_pp->FBW, /*PSM*/SCE_GS_PSMCT32, /*TW*/10, /*TH*/8, /*TCC*/0, /*TFX*/SCE_GS_MODULATE,
                                                              /*CBP*/NULL, /*CPSM*/0, /*CSM*/0, /*CSA*/0, /*CLD*/0));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_TEX1_2, SCE_GS_SET_TEX1(/*LCM*/0, /*MXL*/0, /*MMAG*/SCE_GS_NEAREST, /*MMIN*/SCE_GS_NEAREST, /*MTBA*/0, /*L*/0, /*K*/0));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_PRIM, SCE_GS_SET_PRIM(/*PRIM*/SCE_GS_PRIM_SPRITE, /*IIP*/0, /*TME*/true, /*FGE*/false, /*ABE*/false, /*AA1*/true, /*FST*/1, /*CTX*/1, /*FIX*/0));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(/*AC*/1));

    tmpAngle = wstr->currentAng;
    tmp_w = wstr->sizeW;
    tmp_h = wstr->sizeH;

    if (wstr->wmode == WM_WSLICE) {
        tmp_w -= ((int)wstr->mvSize * 2);
    } else {
        tmp_h -= ((int)wstr->mvSize * 2);
    }

    for (int i = 0; i < wstr->linecnt; i++) {
        haba_now_u = (int)(((wstr->mvSize * sinf(tmpAngle)) + wstr->mvSize) * 16.0f);
        haba_now_v = haba_now_u;

        if (wstr->wmode == WM_WSLICE) {
            haba_now_v = 0;
        } else {
            haba_now_u = 0;
        }

        tmp_u = (wstr->addU * i) + wstr->u;
        tmp_v = (wstr->addV * i) + wstr->v;
        sceGifPkAddGsAD(wavePkSpr, SCE_GS_UV, SCE_GS_SET_UV((tmp_u << 4) + haba_now_u, (tmp_v << 4) + haba_now_v));

        tmp_x = wstr->x + (wstr->addW * i);
        tmp_y = wstr->y + (wstr->addH * i);
        sceGifPkAddGsAD(wavePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ(tmp_x << 4, tmp_y << 4, 1));

        tmp_u += tmp_w;
        tmp_v += tmp_h;
        sceGifPkAddGsAD(wavePkSpr, SCE_GS_UV, SCE_GS_SET_UV((tmp_u << 4) + haba_now_u, (tmp_v << 4) + haba_now_v));
        sceGifPkAddGsAD(wavePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ((tmp_x + wstr->sizeW) << 4, (tmp_y + wstr->sizeH) << 4, 1));

        tmpAngle += wstr->plsAng1line;
    }
}

static void CG_WaveDisp(WAVE_STR *wstr, sceGsFrame *frame_pp) {
    u_long       giftag[2] = { SCE_GIF_SET_TAG(/*NLOOP*/0, /*EOP*/true, /*PRE*/0, /*PRIM*/0, /*FLG*/SCE_GIF_PACKED, /*NREG*/1), SCE_GIF_PACKED_AD };
    sceGifPacket wavePkSpr;

    sceGifPkInit(&wavePkSpr, (u_long128*)cmnGifTr);
    sceGifPkReset(&wavePkSpr);
    sceGifPkCnt(&wavePkSpr, 0, 0, 0);

    sceGifPkOpenGifTag(&wavePkSpr, *(u_long128*)giftag);

    UG_WaveDisp(wstr, frame_pp, &wavePkSpr);
    sceGifPkCloseGifTag(&wavePkSpr);
    sceGifPkEnd(&wavePkSpr, 0, 0, 0);

    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    FlushCache(0);
    sceDmaSend(chan, wavePkSpr.pBase);
    sceGsSyncPath(0, 0);
}

void WaveCtrlInit(WAVE_STR *wstr, short w, short h, WMODE_ENUM wmode) {
    WV_SCREEN_W = w;
    WV_SCREEN_H = h;

    CG_WaveInit(wstr);
    CG_WaveInitEasy(wstr, 2048 - (WV_SCREEN_W / 2), 2048 - (WV_SCREEN_H / 2), WV_SCREEN_W, WV_SCREEN_H, wmode);

    wstr->mvSize = 24.0f;
    wstr->plsAng1line = 0.08f;
    wstr->plsAng1time = 0.06f;
}


void WaveCtrlDisp(WAVE_STR *wstr, sceGsFrame *frame_pp) {
    CG_WaveDisp(wstr, frame_pp);
}

void WaveCtrlUpdate(WAVE_STR *wstr, float frames) {
    wstr->currentAng += frames * wstr->plsAng1time;
    if (wstr->currentAng >= (PR_PI*2)) {
        wstr->currentAng -= (PR_PI*2);
    }
}
