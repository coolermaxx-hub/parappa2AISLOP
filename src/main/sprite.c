#include "main/sprite.h"

#include "os/cmngifpk.h"

#include <libdma.h>
#include <libgifpk.h>
#include <libgraph.h>

#include <stdio.h>

static int sprSetNum = 0;

static u_long128 sprPacket[2048];
static sceGifPacket gifPkSpr;

static sceDmaChan *sprDmaC;

void SprInit(void) {
    sceGifPkInit(&gifPkSpr, sprPacket);
    sprDmaC = sceDmaGetChan(SCE_DMA_GIF);
    sprSetNum = 0;
}

void SprClear(void) {
    u_long giftag[2] = { SCE_GIF_SET_TAG(0, 0, 0, 0, 0, 1), SCE_GIF_PACKED_AD };

    sceGifPkReset(&gifPkSpr);
    sceGifPkCnt(&gifPkSpr, 0, 0, 0);

    sceGifPkOpenGifTag(&gifPkSpr, GIF_TAG_QWORD(giftag));
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0));
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEST_1, GS_TEST_OFF);
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(1));
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_CLAMP_1, GS_CLAMP_EDGES);

    sprSetNum = 0;
}

void SprPackSet(SPR_DAT *spr_pp) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEX0_1, spr_pp->GsTex0);
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEX1_1, spr_pp->GsTex1);

    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEXA, SCE_GS_SET_TEXA(PR_REGS(spr_pp).ta0, PR_REGS(spr_pp).aem, PR_REGS(spr_pp).ta1));

    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_PABE,  PR_REGS(spr_pp).pabe);
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_FBA_1, PR_REGS(spr_pp).fba );
}

void SprFlash(void) {
    u_long giftag[2] = { SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1), SCE_GIF_PACKED_AD };

    if (sprSetNum != 0) {
        sceGifPkCloseGifTag(&gifPkSpr);
        sceGifPkOpenGifTag(&gifPkSpr, GIF_TAG_QWORD(giftag));
        sceGifPkCloseGifTag(&gifPkSpr);

        sceGifPkEnd(&gifPkSpr, 0, 0, 0);
        sceGifPkTerminate(&gifPkSpr);

        FlushCache(WRITEBACK_DCACHE);
        sceDmaSend(sprDmaC, gifPkSpr.pBase);
        sceGsSyncPath(0, 0);
    }
}

void SprSetColor(u_char r, u_char g, u_char b, u_char a) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(r, g, b, a, GS_Q_ONE));
}

void SprDatPrint(SPR_DAT *spr_pp) {
    printf("TBP0[%x] TBW[%x] PSM[%x] TW[%x] TH[%x] TCC[%x] \nTFX[%x] CBP[%x] CPSM[%x] CSM[%x] CSA[%x] CLD[%x]\n\n",
        PR_TEX0(spr_pp).TBP0, PR_TEX0(spr_pp).TBW, PR_TEX0(spr_pp).PSM,  PR_TEX0(spr_pp).TW,  PR_TEX0(spr_pp).TH,  PR_TEX0(spr_pp).TCC,
        PR_TEX0(spr_pp).TFX,  PR_TEX0(spr_pp).CBP, PR_TEX0(spr_pp).CPSM, PR_TEX0(spr_pp).CSM, PR_TEX0(spr_pp).CSA, PR_TEX0(spr_pp).CLD);

    printf("LCM[%x] MXL[%x] MMAG[%x] MMIN[%x] MTBA[%x] L[%x] K[%x]\n\n",
        PR_TEX1(spr_pp).LCM,  PR_TEX1(spr_pp).MXL, PR_TEX1(spr_pp).MMAG, PR_TEX1(spr_pp).MMIN, 
        PR_TEX1(spr_pp).MTBA, PR_TEX1(spr_pp).L,   PR_TEX1(spr_pp).K);

    printf("TA0[%x] AEM[%x] TA1[%x] PABE[%x] FBA[%x]\n",
        PR_REGS(spr_pp).ta0,  PR_REGS(spr_pp).aem, PR_REGS(spr_pp).ta1,  PR_REGS(spr_pp).pabe, PR_REGS(spr_pp).fba);
}

void SprDisp(SPR_PRIM *prm_pp) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(FALSE));

    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_UV, SCE_GS_SET_UV((prm_pp->u + 1) << 4, (prm_pp->v + 1) << 4));
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((prm_pp->x << 4) - (((prm_pp->w * prm_pp->scalex) / 2) >> 4),
                                                            (prm_pp->y << 4) - (((prm_pp->h * prm_pp->scaley) / 2) >> 4), 1));

    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_UV, SCE_GS_SET_UV((prm_pp->u + prm_pp->w - 1) << 4, (prm_pp->v + prm_pp->h - 1) << 4));
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((prm_pp->x << 4) + (((prm_pp->w * prm_pp->scalex) / 2) >> 4),
                                                            (prm_pp->y << 4) + (((prm_pp->h * prm_pp->scaley) / 2) >> 4), 1));

    sprSetNum++;
}

void SprDispAlp(SPR_PRIM *prm_pp) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(TRUE));

    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_UV, SCE_GS_SET_UV((prm_pp->u + 1) << 4, (prm_pp->v + 1) << 4));
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((prm_pp->x << 4) - (((prm_pp->w * prm_pp->scalex) / 2) >> 4),
                                                            (prm_pp->y << 4) - (((prm_pp->h * prm_pp->scaley) / 2) >> 4), 1));

    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_UV, SCE_GS_SET_UV((prm_pp->u + prm_pp->w - 1) << 4, (prm_pp->v + prm_pp->h - 1) << 4));
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((prm_pp->x << 4) + (((prm_pp->w * prm_pp->scalex) / 2) >> 4),
                                                            (prm_pp->y << 4) + (((prm_pp->h * prm_pp->scaley) / 2) >> 4), 1));

    sprSetNum++;
}

/* Following sprites write RGB only, keeping the frame buffer's alpha and Z. */
void SprDispZABnclr(void) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEST_1, SCE_GS_SET_TEST_1(/*ATE*/1, /*ATST*/SCE_GS_ALPHA_NEVER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_RGB_ONLY,
                                                               /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/SCE_GS_ZALWAYS));
}

/* Following sprites write colour only, keeping Z. */
void SprDispZBnclr(void) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEST_1, GS_TEST_COLOR_ONLY);
}

/* Following sprites are depth tested (Z >= buffer) with no alpha test. */
void SprDispZcheck(void) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEST_1, SCE_GS_SET_TEST_1(/*ATE*/0, /*ATST*/SCE_GS_ALPHA_NEVER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_KEEP,
                                                               /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/SCE_GS_ZGEQUAL));
}

/* flg set: draw only texels with alpha above zero. Clear: draw only texels with zero alpha. */
void SprDispAcheck(int flg) {
    if (flg) {
        sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEST_1, GS_TEST_ALPHA_NONZERO);
    } else {
        sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEST_1, SCE_GS_SET_TEST_1(/*ATE*/1, /*ATST*/SCE_GS_ALPHA_LEQUAL, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_KEEP,
                                                                   /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/SCE_GS_ZALWAYS));
    }
}

void SprDispAlphaSet(void) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_ALPHA_1, GS_ALPHA_BLEND);
}

void SprBox(SPR_PRIM *prm_pp) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_TEST_1, SCE_GS_SET_TEST(/*ATE*/1, /*ATST*/SCE_GS_ALPHA_ALWAYS, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_KEEP,
                                                             /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/SCE_GS_DEPTH_ALWAYS));
    /* Cd * As + Cs: the frame is scaled by the box alpha, then the box colour is added. */
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CD, SCE_GS_ALPHA_ZERO, SCE_GS_ALPHA_AS, SCE_GS_ALPHA_CS, 0));
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, /*IIP*/0, /*TME*/0, /*FGE*/0, /*ABE*/1,
                                                            /*AA1*/0, /*FST*/0, SCE_GS_PRIM_CTXT1, /*FIX*/0));

    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((prm_pp->x << 4) - (((prm_pp->w * prm_pp->scalex) / 2) >> 4),
                                                            (prm_pp->y << 4) - (((prm_pp->h * prm_pp->scaley) / 2) >> 4), 1));

    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((prm_pp->x << 4) + (((prm_pp->w * prm_pp->scalex) / 2) >> 4),
                                                            (prm_pp->y << 4) + (((prm_pp->h * prm_pp->scaley) / 2) >> 4), 1));

    sprSetNum++;
}

void SprWindow(u_int x, u_int y, u_int w, u_int h) {
    sceGifPkAddGsAD(&gifPkSpr, SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR_1(x, (x + w) - 1, y, (y + h) - 1));
}

void SprWindowDf(void) {
    SprWindow(0, 0, SCREEN_WIDTH, SCREEN_FIELD_HEIGHT);
}
