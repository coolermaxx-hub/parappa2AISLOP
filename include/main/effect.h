#ifndef EFFECT_H
#define EFFECT_H

#include "common.h"

#include <eetypes.h>

#include <eestruct.h>
#include <libgifpk.h>

typedef enum {
    WM_WSLICE = 0,
    WM_HSLICE = 1,
    WM_MAX = 2
} WMODE_ENUM;

typedef struct { // 0x2c
    /* 0x00 */ int wmode;
    /* 0x04 */ int linecnt;
    /* 0x08 */ short x;
    /* 0x0a */ short y;
    /* 0x0c */ short sizeW;
    /* 0x0e */ short sizeH;
    /* 0x10 */ short addW;
    /* 0x12 */ short addH;
    /* 0x14 */ short u;
    /* 0x16 */ short v;
    /* 0x18 */ short addU;
    /* 0x1a */ short addV;
    /* 0x1c */ float mvSize;
    /* 0x20 */ float plsAng1line;
    /* 0x24 */ float plsAng1time;
    /* 0x28 */ float currentAng;
} WAVE_STR;

typedef struct { // 0x8
    /* 0x0 */ float ofs0;
    /* 0x4 */ float ofs1;
} PLP_OFS;

typedef struct { // 0x48
    /* 0x00 */ u_char r;
    /* 0x01 */ u_char g;
    /* 0x02 */ u_char b;
    /* 0x03 */ u_char a;
    /* 0x04 */ u_char alp;
    /* 0x08 */ PLP_OFS xyOfs[4];
    /* 0x28 */ PLP_OFS uvOfs[4];
} PLH_STR;

/* GS CLAMP region repeat: each 10-bit texel coordinate is ANDed with the mask
 * and ORed with the fix. MOZAIKU_MASK(n) snaps it down to a multiple of n (a power
 * of two), drawing the screen in n-texel mosaic blocks. */
#define MOZAIKU_MASK(n) (0x3ff & ~((n) - 1))

typedef struct { // 0x10
    /* 0x0 */ int umsk;
    /* 0x4 */ int ufix;
    /* 0x8 */ int vmsk;
    /* 0xc */ int vfix;
} MOZAIKU_STR;

typedef struct { // 0x4
    /* 0x0 */ u_char r;
    /* 0x1 */ u_char g;
    /* 0x2 */ u_char b;
    /* 0x3 */ u_char alp;
} FADE_MAKE_STR;

typedef struct { // 0x24
    /* 0x00 */ int cntW;
    /* 0x04 */ int cntH;
    /* 0x08 */ int cnterW;
    /* 0x0c */ int cnterH;
    /* 0x10 */ float time_lng;
    /* 0x14 */ float cycle_lng;
    /* 0x18 */ float cycle_w;
    /* 0x1c */ u_char r;
    /* 0x1d */ u_char g;
    /* 0x1e */ u_char b;
    /* 0x1f */ u_char a;
    /* 0x20 */ int texnum;
} NOODLES_STR;

typedef struct { // 0x10
    /* 0x0 */ int xp;
    /* 0x4 */ int yp;
    /* 0x8 */ int u;
    /* 0xc */ int v;
} NDL_PRM;

typedef struct { // 0x3
    /* 0x0 */ u_char pR;
    /* 0x1 */ u_char pG;
    /* 0x2 */ u_char pB;
} MONOCRO_STR;

void CG_WaveInit(WAVE_STR *wstr);
void CG_WaveInitEasy(WAVE_STR *wstr, short x, short y, short w, short h, WMODE_ENUM wmode);
void UG_WaveDisp(WAVE_STR *wstr, sceGsFrame *frame_pp, sceGifPacket *wavePkSpr);
void CG_WaveDisp(WAVE_STR *wstr, sceGsFrame *frame_pp, int pri);

void UG_AlpDisp(PLH_STR *plh_pp, sceGsFrame *frame_pp, sceGifPacket *alpPkSpr);
void CG_AlpDisp(PLH_STR *plh_pp, sceGsFrame *frame_pp, int pri);

void UG_MozaikuDisp(MOZAIKU_STR *moz_pp, sceGsFrame *frame_pp, sceGifPacket *mozPkSpr);
void CG_MozaikuDisp(MOZAIKU_STR *moz_pp, sceGsFrame *frame_pp, int pri);

void UG_FadeDisp(FADE_MAKE_STR *fade_pp, sceGifPacket *fadePkSpr, sceGsFrame *texFr_pp);
void UG_FadeDisp2(FADE_MAKE_STR *fade_pp, sceGifPacket *fadePkSpr, sceGsFrame *texFr_pp, float scale);
void CG_FadeDisp(FADE_MAKE_STR *fade_pp, int pri, sceGsFrame *texFr_pp);

void UG_NoodlesDisp(NOODLES_STR *ndl_pp, sceGsFrame *frame_pp, sceGifPacket *ndlPkSpr, int time);
void CG_NoodlesDisp(NOODLES_STR *ndl_pp, sceGsFrame *frame_pp, int pri, int time);

void FD_MonocroDisp(MONOCRO_STR *mono_pp, int tbp, int w, int h);

#endif /* EFFECT_H */
