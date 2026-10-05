#ifndef COMMAKE_H
#define COMMAKE_H

#include "common.h"

#include "main/scrctrl.h"

#include <eetypes.h>

typedef struct { // 0x4
    /* 0x0 */ u_short keyId;
    /* 0x2 */ short timeOfs;
} CM_STR;

/* Patterns are laid out on step slots (TICKS_PER_STEP apart), eight beats' worth. */
#define CM_STEP_MAX 32

/* The computer's versus answer (see behavior-notes.md). keyCnt_* count each key
 * by KEY_INDEX_ENUM, with slot KiNO holding the total. */
typedef struct { // 0x1c4
    /* 0x000 */ CM_STR cm_str_mt[CM_STEP_MAX];   /* the line's own tap set */
    /* 0x080 */ CM_STR cm_str_now[CM_STEP_MAX];  /* the pattern to answer (vs_tapdat_work) */
    /* 0x100 */ CM_STR cm_str_make[CM_STEP_MAX]; /* the answer being built */
    /* 0x180 */ int keyKind;    /* key code mask used by cm_str_mt */
    /* 0x184 */ int keyKindNum; /* number of keys in keyKind */
    /* 0x188 */ int keyCnt_mt[KiMAX];
    /* 0x1a4 */ int keyCnt_now[KiMAX];
    /* 0x1c0 */ int maxBox;     /* steps in the tap window, minus one */
} CM_STR_CTRL;

typedef struct { // 0x8
    /* 0x0 */ int time;
    /* 0x4 */ short KeyIndex;
} COMMAKE_STR;

void    comMakeSubYure(CM_STR *cms_pp, int cnt, int min, int max);
void    comMakeSubYureReset(CM_STR *cms_pp, int cnt);
void    comMakeSubChangeKey(CM_STR *cms_pp, int cnt, int motoKey, int sakiKey, int missCnt);
void    comMakeSubSwapKey(CM_STR *cms_pp, int cnt, int swKey1, int swKey2);
void    comMakeSubSwapCntKey(CM_STR *cms_pp, int cnt, int swKey1, int swKey2, int pos);
void    comMakeSubDoubleKey(CM_STR *cms_pp, int cnt);
CM_STR* comMakeSubSpaceSearch(CM_STR *cms_pp, int cnt);
int     comMakeSubUseKeyCode(CM_STR *cms_pp, int cnt);

void comMakingNo0(CM_STR_CTRL *cmstr_pp);
void comMakingNo1(CM_STR_CTRL *cmstr_pp);
void comMakingNo2(CM_STR_CTRL *cmstr_pp);
void comMakingNo3(CM_STR_CTRL *cmstr_pp);
void comMakingNo5(CM_STR_CTRL *cmstr_pp);
void comMakingNo6(CM_STR_CTRL *cmstr_pp);
void comMakingNo7(CM_STR_CTRL *cmstr_pp);
void comMakingNo8(CM_STR_CTRL *cmstr_pp);
void comMakingNo9(CM_STR_CTRL *cmstr_pp);
void comMakingNo15(CM_STR_CTRL *cmstr_pp);
void comMakingNo16(CM_STR_CTRL *cmstr_pp);

int computerMaking(COMMAKE_STR *com_pp, int com_cnt, TAPDAT *moto_pp, int moto_cnt, TAPSET *tapset_pp, LEVEL_VS_ENUM clvl);

#endif /* COMMAKE_H */
