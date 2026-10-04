#ifndef CMNGIFPK_H
#define CMNGIFPK_H

#include "common.h"

#include <eetypes.h>
#include <libgifpk.h>

/* RGBAQ's Q field takes a float; 1.0f for untextured primitives. */
#define GS_Q_ONE 0x3f800000

/* TEX0 that samples a frame buffer as a 1024x256 CT32 texture; the 640x224 field sits in its top-left corner. */
#define GS_FRAME_TEX0(frame, tcc, tfx) \
    SCE_GS_SET_TEX0(/*TBP0*/(frame)->FBP << 5, /*TBW*/(frame)->FBW, /*PSM*/SCE_GS_PSMCT32, /*TW*/10, /*TH*/8, \
                    /*TCC*/(tcc), /*TFX*/(tfx), /*CBP*/0, /*CPSM*/0, /*CSM*/0, /*CSA*/0, /*CLD*/0)

/* Texel coordinate of the field's bottom-right corner, in 12.4 fixed point. */
#define GS_FIELD_UV_MAX SCE_GS_SET_UV(SCREEN_WIDTH << 4, SCREEN_FIELD_HEIGHT << 4)

/* TEST with no alpha test; the depth test is on but always passes. */
#define GS_TEST_OFF \
    SCE_GS_SET_TEST(/*ATE*/0, /*ATST*/SCE_GS_ALPHA_NEVER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_KEEP, \
                    /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/SCE_GS_DEPTH_ALWAYS)

/* TEST that skips pixels whose alpha is zero; the depth test always passes. */
#define GS_TEST_ALPHA_NONZERO \
    SCE_GS_SET_TEST(/*ATE*/1, /*ATST*/SCE_GS_ALPHA_GREATER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_KEEP, \
                    /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/SCE_GS_DEPTH_ALWAYS)

/* TEST where every pixel fails the alpha test and writes colour only, leaving Z untouched. */
#define GS_TEST_COLOR_ONLY \
    SCE_GS_SET_TEST(/*ATE*/1, /*ATST*/SCE_GS_ALPHA_NEVER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_FB_ONLY, \
                    /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/SCE_GS_DEPTH_ALWAYS)

/* PRIM for a textured sprite addressed with UV texel coordinates, blended when abe is set. */
#define GS_PRIM_TEX_SPRITE(abe) \
    SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, /*IIP*/0, /*TME*/1, /*FGE*/0, /*ABE*/(abe), /*AA1*/0, /*FST*/1, SCE_GS_PRIM_CTXT1, /*FIX*/0)

/* ALPHA for the usual blend, (Cs - Cd) * As + Cd. */
#define GS_ALPHA_BLEND SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_AS, SCE_GS_ALPHA_CD, 0)

/* ALPHA that blends at a fixed opacity, (Cs - Cd) * fix / 128 + Cd. */
#define GS_ALPHA_FIXED(fix) SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_FIX, SCE_GS_ALPHA_CD, (fix))

/* CLAMP that clamps both texture axes at the texture edges. */
#define GS_CLAMP_EDGES SCE_GS_SET_CLAMP(/*WMS*/SCE_GS_CLAMP, /*WMT*/SCE_GS_CLAMP, 0, 0, 0, 0)

typedef struct { // 0x8
    /* 0x0 */ u_long128 *pBase;
    /* 0x4 */ int pri;
} CMNGIF_PRI;

void CmnGifInit(void *buf_adr, int size);
void CmnGifClear(void);
void CmnGifFlush(void);

int CmnGifSetData(sceGifPacket *gifpk_pp, int pri);

int CmnGifOpenCmnPk(sceGifPacket *gifpk_pp);
int CmnGifCloseCmnPk(sceGifPacket *gifpk_pp, int pri);

u_long128* CmnGifAdrsGet(void);
int CmnGifAdrsEnd(u_long128 *adr);

void CmnGifADPacketMake(sceGifPacket *gifP_pp, sceGsFrame *gsframe_pp);
void CmnGifADPacketMake2(sceGifPacket *gifP_pp, sceGsFrame *gsframe_pp);
int CmnGifADPacketMakeTrans(sceGifPacket *gifP_pp);

#endif /* CMNGIFPK_H */
