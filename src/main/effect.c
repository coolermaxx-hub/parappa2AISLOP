#include "main/effect.h"

#include "os/cmngifpk.h"
#include "os/syssub.h"

#include <libgraph.h>

#include <math.h>
#include <stdio.h>

static sceGsStoreImage gs_simage;
static sceGsLoadImage gs_loadimg;

void CG_WaveInit(WAVE_STR *wstr) {
    wstr->currentAng = 0.0f;
}

void CG_WaveInitEasy(WAVE_STR *wstr, short x, short y, short w, short h, WMODE_ENUM wmode) {
    wstr->wmode = wmode;
    wstr->x = x;
    wstr->y = y;

    if (wmode == WM_WSLICE) {
        wstr->linecnt = h - 1;
        wstr->sizeW = w;
        wstr->sizeH = 1;
        
        wstr->addW = 0;
        wstr->addH = 1;

        wstr->u = x + ((SCREEN_WIDTH / 2) - 2048);
        wstr->v = y + ((SCREEN_FIELD_HEIGHT / 2) - 2047);

        wstr->addU = 0;
        wstr->addV = 1;
    } else {
        wstr->linecnt = w - 1;
        wstr->sizeW = 1;
        wstr->sizeH = h;

        wstr->addW = 1;
        wstr->addH = 0;

        wstr->u = x + ((SCREEN_WIDTH / 2) - 2047);
        wstr->v = y + ((SCREEN_FIELD_HEIGHT / 2) - 2048);

        wstr->addU = 1;
        wstr->addV = 0;
    }

    wstr->currentAng = 0.0f;
}

void UG_WaveDisp(WAVE_STR *wstr, sceGsFrame *frame_pp, sceGifPacket *wavePkSpr) {
    int i;

    float tmpAngle;
    int haba_now_u, haba_now_v;
    int tmp_u, tmp_v;
    int tmp_x, tmp_y;

    sceGifPkAddGsAD(wavePkSpr, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_TEST_1, GS_TEST_OFF);
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_TEX0_1, GS_FRAME_TEX0(frame_pp, 0, SCE_GS_MODULATE));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_TEX1_1, 0);
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, /*IIP*/0, /*TME*/TRUE, /*FGE*/FALSE, /*ABE*/FALSE,
                                                            /*AA1*/TRUE, /*FST*/1, SCE_GS_PRIM_CTXT1, /*FIX*/FALSE));
    sceGifPkAddGsAD(wavePkSpr, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(/*AC*/1));

    tmpAngle = wstr->currentAng;

    for (i = 0; i < wstr->linecnt; i++) {
        haba_now_u = ((wstr->mvSize * sinf(tmpAngle)) + wstr->mvSize) * 16.0f;
        haba_now_v = haba_now_u;

        if (wstr->wmode == WM_WSLICE) {
            haba_now_v = 0;
        } else {
            haba_now_u = 0;
        }

        tmp_u = (wstr->addU * i) + wstr->u;
        tmp_v = (wstr->addV * i) + wstr->v;
        sceGifPkAddGsAD(wavePkSpr, SCE_GS_UV,   SCE_GS_SET_UV((tmp_u << 4) + haba_now_u, (tmp_v << 4) + haba_now_v));

        tmp_x = wstr->x + (wstr->addW * i);
        tmp_y = wstr->y + (wstr->addH * i);
        sceGifPkAddGsAD(wavePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ(tmp_x << 4, tmp_y << 4, 1));

        tmp_u += wstr->sizeW - (wstr->mvSize * 2);
        tmp_v += wstr->sizeH - (wstr->mvSize * 2);
        sceGifPkAddGsAD(wavePkSpr, SCE_GS_UV,   SCE_GS_SET_UV((tmp_u << 4) + haba_now_u, (tmp_v << 4) + haba_now_v));
        sceGifPkAddGsAD(wavePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ((tmp_x + wstr->sizeW) << 4, (tmp_y + wstr->sizeH) << 4, 1));

        tmpAngle += wstr->plsAng1line;
    }

    wstr->currentAng += wstr->plsAng1time;

    if (wstr->currentAng >= (PR_PI*2)) {
        wstr->currentAng -= (PR_PI*2);
    }
}

void CG_WaveDisp(WAVE_STR *wstr, sceGsFrame *frame_pp, int pri) {
    sceGifPacket wavePkSpr;

    CmnGifOpenCmnPk(&wavePkSpr);
    UG_WaveDisp(wstr, frame_pp, &wavePkSpr);
    CmnGifCloseCmnPk(&wavePkSpr, pri);
}

void UG_AlpDisp(PLH_STR *plh_pp, sceGsFrame *frame_pp, sceGifPacket *alpPkSpr) {
    int i;

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_TEXFLUSH, 0);

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(/*AC*/1));

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_TEST_1, GS_TEST_OFF);
    sceGifPkAddGsAD(alpPkSpr, SCE_GS_ALPHA_1, GS_ALPHA_FIXED(plh_pp->alp));

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP(/*WMS*/SCE_GS_REGION_CLAMP, /*WMT*/SCE_GS_REGION_CLAMP,
                                                               /*MINU*/0, /*MAXU*/SCREEN_WIDTH - 1, /*MINV*/0, /*MAXV*/SCREEN_FIELD_HEIGHT - 1));
    sceGifPkAddGsAD(alpPkSpr, SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1));

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_PABE, SCE_GS_SET_PABE(FALSE));

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_TEX0_1, GS_FRAME_TEX0(frame_pp, 0, SCE_GS_MODULATE));

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(/*LCM*/0, /*MXL*/0, /*MMAG*/SCE_GS_LINEAR, /*MMIN*/SCE_GS_LINEAR,
                                                             /*MTBA*/0, /*L*/0, /*K*/0));

    /* note: TEXA set is useless as all framebuffers are RGBA32. */
    sceGifPkAddGsAD(alpPkSpr, SCE_GS_TEXA, SCE_GS_SET_TEXA(/*TA0*/128, /*AEM*/0, /*TA1*/128));

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(plh_pp->r, plh_pp->g, plh_pp->b, 0, GS_Q_ONE));

    sceGifPkAddGsAD(alpPkSpr, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, /*IIP*/1, /*TME*/TRUE, /*FGE*/FALSE, /*ABE*/TRUE,
                                                           /*AA1*/FALSE, /*FST*/1, SCE_GS_PRIM_CTXT1, /*FIX*/FALSE));

    for (i = 0; i < 4; i++) {
        short ofs_tbl[4][2] = {
            { 0,   0   },
            { SCREEN_WIDTH, 0 },
            { 0, SCREEN_FIELD_HEIGHT },
            { SCREEN_WIDTH, SCREEN_FIELD_HEIGHT }
        };
        int   tmp0, tmp1;

        tmp0 = plh_pp->uvOfs[i].ofs0 * 16.0f;
        tmp0 += ofs_tbl[i][0] << 4;
        tmp1 = plh_pp->uvOfs[i].ofs1 * 16.0f;
        tmp1 += ofs_tbl[i][1] << 4;

        sceGifPkAddGsAD(alpPkSpr, SCE_GS_UV, SCE_GS_SET_UV(tmp0 & 0xffff, tmp1 & 0xffff));

        tmp0 = plh_pp->xyOfs[i].ofs0 * 16.0f;
        tmp0 += (ofs_tbl[i][0] + (GS_X_COORD(0) >> 4)) << 4;
        tmp1 = plh_pp->xyOfs[i].ofs1 * 16.0f;
        tmp1 += (ofs_tbl[i][1] + (GS_Y_COORD(0) >> 4)) << 4;

        sceGifPkAddGsAD(alpPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ(tmp0 & 0xffff, tmp1 & 0xffff, 1));
    }
}

void CG_AlpDisp(PLH_STR *plh_pp, sceGsFrame *frame_pp, int pri) {
    sceGifPacket alpPkSpr;
    
    CmnGifOpenCmnPk(&alpPkSpr);
    UG_AlpDisp(plh_pp, frame_pp, &alpPkSpr);
    CmnGifCloseCmnPk(&alpPkSpr, pri);
}

void UG_MozaikuDisp(MOZAIKU_STR *moz_pp, sceGsFrame *frame_pp, sceGifPacket *mozPkSpr) {
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(/*AC*/1));
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_TEST_1, GS_TEST_OFF);
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 128, GS_Q_ONE));

    /* Region repeat masks and fixes the texel coordinates, which snaps them into mosaic blocks. */
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP(/*WMS*/SCE_GS_REGION_REPEAT, /*WMT*/SCE_GS_REGION_REPEAT,
                                                               /*UMSK*/moz_pp->umsk & 0x3ff, /*UFIX*/moz_pp->ufix & 0x3ff,
                                                               /*VMSK*/moz_pp->vmsk & 0x3ff, /*VFIX*/moz_pp->vfix & 0x3ff));

    sceGifPkAddGsAD(mozPkSpr, SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(0));
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_PABE, SCE_GS_SET_PABE(FALSE));

    sceGifPkAddGsAD(mozPkSpr, SCE_GS_TEX0_1, GS_FRAME_TEX0(frame_pp, 0, SCE_GS_MODULATE));
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(/*LCM*/0, /*MXL*/0, /*MMAG*/SCE_GS_NEAREST, /*MMIN*/SCE_GS_NEAREST,
                                                             /*MTBA*/0, /*L*/0, /*K*/0));
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(FALSE));

    sceGifPkAddGsAD(mozPkSpr, SCE_GS_UV, GS_FIELD_UV_MAX);
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(SCREEN_WIDTH), GS_Y_COORD(SCREEN_FIELD_HEIGHT), 0));

    sceGifPkAddGsAD(mozPkSpr, SCE_GS_UV, SCE_GS_SET_UV(0, 0));
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(0), GS_Y_COORD(0), 1));

    sceGifPkAddGsAD(mozPkSpr, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(mozPkSpr, SCE_GS_CLAMP_1, 0);
}

void CG_MozaikuDisp(MOZAIKU_STR *moz_pp, sceGsFrame *frame_pp, int pri) {
    sceGifPacket mozPkSpr;
    
    CmnGifOpenCmnPk(&mozPkSpr);
    UG_MozaikuDisp(moz_pp, frame_pp, &mozPkSpr);
    CmnGifCloseCmnPk(&mozPkSpr, pri);
}

void UG_FadeDisp(FADE_MAKE_STR *fade_pp, sceGifPacket *fadePkSpr, sceGsFrame *texFr_pp) {
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(/*AC*/1));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_ALPHA_1, GS_ALPHA_BLEND);
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP(/*WMS*/SCE_GS_CLAMP, /*WMT*/SCE_GS_CLAMP, 0, 0, 0, 0));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_PABE, SCE_GS_SET_PABE(FALSE));

    if (texFr_pp == NULL) {
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_TEST_1, GS_TEST_COLOR_ONLY);
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(fade_pp->r, fade_pp->g, fade_pp->b, fade_pp->alp, 0));
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, /*IIP*/0, /*TME*/FALSE, /*FGE*/FALSE, /*ABE*/TRUE,
                                                                /*AA1*/FALSE, /*FST*/1, SCE_GS_PRIM_CTXT1, /*FIX*/FALSE));
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(0), GS_Y_COORD(0), 1));
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(SCREEN_WIDTH), GS_Y_COORD(SCREEN_FIELD_HEIGHT), 1));
        return;
    }

    sceGifPkAddGsAD(fadePkSpr, SCE_GS_TEST_1, GS_TEST_ALPHA_NONZERO);
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, fade_pp->alp, 0));

    sceGifPkAddGsAD(fadePkSpr, SCE_GS_TEX0_1, GS_FRAME_TEX0(texFr_pp, 0, SCE_GS_DECAL));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(TRUE));

    sceGifPkAddGsAD(fadePkSpr, SCE_GS_UV, SCE_GS_SET_UV(0, 0));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(0), GS_Y_COORD(0), 1));

    sceGifPkAddGsAD(fadePkSpr, SCE_GS_UV, GS_FIELD_UV_MAX);
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(SCREEN_WIDTH), GS_Y_COORD(SCREEN_FIELD_HEIGHT), 1));
}

void UG_FadeDisp2(FADE_MAKE_STR *fade_pp, sceGifPacket *fadePkSpr, sceGsFrame *texFr_pp, float scale) {
    int xp, yp;

    sceGifPkAddGsAD(fadePkSpr, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(/*AC*/1));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_ALPHA_1, GS_ALPHA_BLEND);
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP(/*WMS*/SCE_GS_REPEAT, /*WMT*/SCE_GS_REPEAT,
                                                                0, SCREEN_WIDTH - 1, 0, SCREEN_FIELD_HEIGHT - 1));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_PABE, SCE_GS_SET_PABE(FALSE));

    if (texFr_pp == NULL) {
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_TEST_1, GS_TEST_COLOR_ONLY);
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(fade_pp->r, fade_pp->g, fade_pp->b, fade_pp->alp, 0));
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, /*IIP*/0, /*TME*/FALSE, /*FGE*/FALSE, /*ABE*/TRUE,
                                                                /*AA1*/FALSE, /*FST*/1, SCE_GS_PRIM_CTXT1, /*FIX*/FALSE));
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ(GS_X_COORD(0), GS_Y_COORD(0), 1));
        sceGifPkAddGsAD(fadePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ(GS_X_COORD(SCREEN_WIDTH), GS_Y_COORD(SCREEN_FIELD_HEIGHT), 1));
        return;
    }

    sceGifPkAddGsAD(fadePkSpr, SCE_GS_TEST_1, GS_TEST_ALPHA_NONZERO);
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, fade_pp->alp, 0));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_TEX0_1, GS_FRAME_TEX0(texFr_pp, 1, SCE_GS_MODULATE));
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(TRUE));

    sceGifPkAddGsAD(fadePkSpr, SCE_GS_UV, SCE_GS_SET_UV(0, 0));

    xp = (scale * SCREEN_WIDTH);
    yp = (scale * SCREEN_FIELD_HEIGHT);

    xp /= 2;
    yp /= 2;
    
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ((2048 - xp) << 4, (2048 - yp) << 4, 1));

    sceGifPkAddGsAD(fadePkSpr, SCE_GS_UV, GS_FIELD_UV_MAX);
    sceGifPkAddGsAD(fadePkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ((xp + 2048) << 4, (yp + 2048) << 4, 1));
}

void CG_FadeDisp(FADE_MAKE_STR *fade_pp, int pri, sceGsFrame *texFr_pp) {
    sceGifPacket fadePkSpr;
  
    CmnGifOpenCmnPk(&fadePkSpr);
    UG_FadeDisp(fade_pp, &fadePkSpr, texFr_pp);
    CmnGifCloseCmnPk(&fadePkSpr,pri);
}

void UG_NoodlesDisp(NOODLES_STR *ndl_pp, sceGsFrame *frame_pp, sceGifPacket *ndlPkSpr, int time) {
    int      i, j;
    int      xx, yy;
    NDL_PRM *ndl_prm_pp;
    NDL_PRM *tmp_pp;
    u_int    tbp, tbw;

    ndl_prm_pp = usrMalloc((ndl_pp->cntW + 1) * (ndl_pp->cntH + 1) * sizeof(NDL_PRM));
    tmp_pp     = ndl_prm_pp;

    for (i = 0; i <= ndl_pp->cntW; i++) {
        for (j = 0; j <= ndl_pp->cntH; j++, tmp_pp++) {
            tmp_pp->u = (i * SCREEN_WIDTH) / ndl_pp->cntW;
            tmp_pp->v = (j * SCREEN_FIELD_HEIGHT) / ndl_pp->cntH;
            tmp_pp->u = tmp_pp->u << 4;
            tmp_pp->v = tmp_pp->v << 4;

            tmp_pp->xp = tmp_pp->u + GS_X_COORD(0);
            tmp_pp->yp = tmp_pp->v + GS_Y_COORD(0);

            xx = sinf((time + j) * ((PR_PI*2) / 49)) * ndl_pp->cycle_lng * 10.0f;
            yy = sinf((time + i) * ((PR_PI*2) / 49)) * ndl_pp->cycle_lng * 10.0f;

            tmp_pp->xp += xx;
            tmp_pp->yp += yy;
        }
    }

    sceGifPkAddGsAD(ndlPkSpr, SCE_GS_TEX0_1, GS_FRAME_TEX0(frame_pp, 1, SCE_GS_MODULATE));

    sceGifPkAddGsAD(ndlPkSpr, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, SCE_GS_FALSE/*MTBA*/, 0, 0));

    sceGifPkAddGsAD(ndlPkSpr, SCE_GS_TEST_1, SCE_GS_SET_TEST(SCE_GS_TRUE/*ATE*/, SCE_GS_ALPHA_NEVER, 0, SCE_GS_AFAIL_RGB_ONLY, SCE_GS_FALSE/*DATE*/,
                                                             SCE_GS_FALSE/*DATM*/, SCE_GS_TRUE/*ZTE*/, SCE_GS_DEPTH_ALWAYS));

    sceGifPkAddGsAD(ndlPkSpr, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(1));

    sceGifPkAddGsAD(ndlPkSpr, SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, 0));

    sceGifPkAddGsAD(ndlPkSpr, SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP(SCE_GS_REPEAT, SCE_GS_REPEAT, 0, 0, 0, 0));

    sceGifPkAddGsAD(ndlPkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(ndl_pp->r, ndl_pp->g, ndl_pp->b, ndl_pp->a, 0x00000001));

    sceGifPkAddGsAD(ndlPkSpr, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0/*IIP*/, SCE_GS_TRUE/*TME*/, SCE_GS_FALSE/*FGE*/,
                                                           SCE_GS_TRUE/*ABE*/, SCE_GS_FALSE/*AA1*/, 1/*FST*/, SCE_GS_PRIM_CTXT1, 0/*FIX*/));

    {
        int      i, j; 
        NDL_PRM *tmp1_pp, *tmp2_pp;

        for (i = 0; i < ndl_pp->cntW; i++) {
            tmp1_pp = &ndl_prm_pp[(i + 0) * (ndl_pp->cntW + 1)];
            tmp2_pp = &ndl_prm_pp[(i + 1) * (ndl_pp->cntW + 1)];

            for (j = 0; j <= ndl_pp->cntH; j++) {
                sceGifPkAddGsAD(ndlPkSpr, SCE_GS_UV, SCE_GS_SET_UV(tmp1_pp->u + 8, tmp1_pp->v + 8));
                sceGifPkAddGsAD(ndlPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ(tmp1_pp->xp, tmp1_pp->yp, 1));

                sceGifPkAddGsAD(ndlPkSpr, SCE_GS_UV, SCE_GS_SET_UV(tmp2_pp->u + 8, tmp2_pp->v + 8));
                sceGifPkAddGsAD(ndlPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ(tmp2_pp->xp, tmp2_pp->yp, 1));

                tmp1_pp++;
                tmp2_pp++;
            }

            sceGifPkAddGsAD(ndlPkSpr, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0/*IIP*/, SCE_GS_TRUE/*TME*/, SCE_GS_FALSE/*FGE*/,
                                                                   SCE_GS_TRUE/*ABE*/, SCE_GS_FALSE/*AA1*/, 1/*FST*/, SCE_GS_PRIM_CTXT1, 0/*FIX*/));
        }
    }

    usrFree(ndl_prm_pp);
}

void CG_NoodlesDisp(NOODLES_STR *ndl_pp, sceGsFrame *frame_pp, int pri, int time) {
    sceGifPacket noodlesPkSpr;
  
    CmnGifOpenCmnPk(&noodlesPkSpr);
    UG_NoodlesDisp(ndl_pp, frame_pp, &noodlesPkSpr, time);
    CmnGifCloseCmnPk(&noodlesPkSpr, pri);
}

void FD_MonocroDisp(MONOCRO_STR *mono_pp, int tbp, int w, int h) {
    u_char *dat_pp;
    int     i, j, k;
    short   sizew, sizeh;
    u_short ctmp;
    
    for (i = 0; i < h; i += 32) {
        sizeh = 32;
        if (sizeh > h - i) {
            sizeh = h - i;
        }
        
        for (j = 0; j < w; j += 128) {
            sizew = 128;
            if (sizeh > w - j) {
                sizew = w - j;
            }

            dat_pp = (u_char*)EE_SCRATCHPAD_BASE;
            sceGsSetDefStoreImage(&gs_simage, tbp, w / 64, 0, j, i, sizew, sizeh);
            FlushCache(WRITEBACK_DCACHE);

            if (sceGsExecStoreImage(&gs_simage, (u_long128*)EE_SCRATCHPAD_BASE) < 0) {
                printf("vramsave Timeout error!!\n");
                return;
            }

            sceGsSyncPath(0, 0);

            for (k = 0; k < 4096; k++) {
                u_short mono = (dat_pp[0] + dat_pp[1] + dat_pp[2]);
                mono /= 3;

                ctmp = (mono_pp->pR * mono) / 128;
                if (ctmp > 256) {
                    ctmp = 255;
                }
                dat_pp[0] = ctmp;
                
                ctmp = (mono_pp->pG * mono) / 128;
                if (ctmp > 256) {
                    ctmp = 255;
                }
                dat_pp[1] = ctmp;
                
                ctmp = (mono_pp->pB * mono) / 128;
                if (ctmp > 256) {
                    ctmp = 255;
                }
                dat_pp[2] = ctmp;

                dat_pp += 4;
            }

            sceGsSyncPath(0, 0);
            FlushCache(WRITEBACK_DCACHE);

            sceGsSetDefLoadImage(&gs_loadimg, tbp, w / 64, 0, j, i, sizew, sizeh);
            FlushCache(WRITEBACK_DCACHE);

            sceGsExecLoadImage(&gs_loadimg, (u_long128*)EE_SCRATCHPAD_BASE);
            sceGsSyncPath(0, 0);
        }
    }
}
