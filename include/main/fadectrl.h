#ifndef FADECTRL_H
#define FADECTRL_H

#include "common.h"

/* A fade mode is a colour in the low byte (0 black, 1 white) plus FADE_OUT to
 * fade to that colour instead of in from it. */
#define FADE_COLOR_MASK 0xff
#define FADE_OUT        0x100

typedef enum {
    FMODE_BLACK_IN = 0,
    FMODE_BLACK_OUT = FADE_OUT | 0,
    FMODE_WHITE_IN = 1,
    FMODE_WHITE_OUT = FADE_OUT | 1
} FADE_MODE;

typedef struct { // 0xc
    /* 0x0 */ FADE_MODE fmode;
    /* 0x4 */ int max_time;
    /* 0x8 */ int current_time;
} FMODE_CTRL_STR;

void FadeCtrlMain(void *x);
void FadeCtrlReq(FADE_MODE fmode, int time);

#endif /* FADECTRL_H */
