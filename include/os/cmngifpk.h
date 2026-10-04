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
