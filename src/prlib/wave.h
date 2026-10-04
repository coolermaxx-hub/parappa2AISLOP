#ifndef PRLIB_WAVE_H
#define PRLIB_WAVE_H

#include "common.h"

#include <eetypes.h>
#include <eestruct.h>

/* From main/effect.c */
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

void WaveCtrlInit(WAVE_STR *wstr, short w, short h, WMODE_ENUM wmode);
void WaveCtrlDisp(WAVE_STR *wstr, sceGsFrame *frame_pp);
void WaveCtrlUpdate(WAVE_STR *wstr, float frames);

#endif /* PRLIB_WAVE_H */
