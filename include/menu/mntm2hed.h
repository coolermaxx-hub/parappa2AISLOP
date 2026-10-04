#ifndef MNTM2HED_H
#define MNTM2HED_H

#include "common.h"
#include <eetypes.h>

/* Pre-uploaded menu texture: size, TIM2 image type and the GS registers that select it in VRAM. */
typedef struct { // 0x30
    /* 0x00 */ int w;
    /* 0x04 */ int h;
    /* 0x08 */ int type; /* TIM2_IDTEX4 or TIM2_IDTEX8 */
    /* 0x0c */ int pad;
    /* 0x10 */ u_long GsTex0;
    /* 0x18 */ u_long GsTex1;
    /* 0x20 */ u_long GsReg;  /* always 0 */
    /* 0x28 */ u_long GsClut; /* not read by the game; contents unknown */
} MENU_TM2_HED;

MENU_TM2_HED* TsGetTM2Hed(int no);

#endif /* MNTM2HED_H */
