#ifndef MENU_H
#define MENU_H

#include "common.h"

#include <eetypes.h>

#include "main/etc.h"
#include "main/mcctrl.h"

typedef enum {
    SEL_MENU_STAGESEL = 0,
    SEL_MENU_SAVE = 1,
    SEL_MENU_REPLAY = 2
} SEL_MENU_ENUM;

typedef struct { // 0xc
    /* 0x0 */ GAME_STATUS *game_status_p;
    /* 0x4 */ MC_REP_STR *mc_rep_str_p;
    /* 0x8 */ SEL_MENU_ENUM sel_menu_enum;
} MENU_STR;

/* mess_mc messages */
enum {
    MCA_NO_CARD = 0,
    MCA_NO_SPACE = 1,
    MCA_NO_CARD2 = 2,
    MCA_CHECK = 3,
    MCA_CHECK_ERR = 4,
    MCA_NO_DATA_S = 5,
    MCA_NO_DATA_R = 6,
    MCA_LOAD = 7,
    MCA_LOAD_ERR = 8,
    MCA_LOAD_ERR2 = 9,
    MCA_LOAD_COMP = 10,
    MCA_NO_CARD3 = 11,
    MCA_NO_CARD4 = 12,
    MCA_SAVE_ERR = 13,
    MCA_SAVE_ERR2 = 14,
    MCA_REWRITE_CONF = 15,
    MCA_INITIAL_CONF = 16,
    MCA_INITIAL = 17,
    MCA_INITIAL_ERR = 18,
    MCA_SAVE = 19,
    MCA_SAVE_ERR3 = 20,
    MCA_SAVE_COMP = 21,
    MCA_LOAD_CONF = 22,
    MCA_RANK_ERR = 23,
    MCA_RANK_ERR2 = 24,
    MCA_DUMY_NUM = 25
};

/* mess_menu messages */
enum {
    MENU_MAP1_HALL = 0,
    MENU_MAP1_ST1 = 1,
    MENU_HALL = 2,
    MENU_ST1 = 3,
    MENU_ST2 = 4,
    MENU_ST3 = 5,
    MENU_ST4 = 6,
    MENU_ST5 = 7,
    MENU_ST6 = 8,
    MENU_ST7 = 9,
    MENU_ST8 = 10,
    MENU_RECORD = 11,
    MENU_REC1 = 12,
    MENU_REC2 = 13,
    MENU_REC3 = 14,
    MENU_REC4 = 15,
    MENU_REC5 = 16,
    MENU_REC6 = 17,
    MENU_REC7 = 18,
    MENU_REC8 = 19,
    MENU_REC9 = 20,
    MENU_REC10 = 21,
    MENU_POPUP_S = 22,
    MENU_POPUP_VCOMP = 23,
    MENU_POPUP_LEVEL = 24,
    MENU_POPUP_VP2 = 25,
    MENU_POPUP_SRANK = 26,
    MENU_POPUP_VRANK = 27,
    MENU_LIST = 28,
    MENU_LIST_1PAGE = 29,
    MENU_LIST_VS = 30,
    MENU_HALL_INSIDE = 31,
    MENU_LOGLOAD_CT = 32,
    MENU_REPLOAD_CT = 33,
    MENU_OPT_CT = 34,
    MENU_LOGLOAD_CAM = 35,
    MENU_LOGLOAD = 36,
    MENU_REPLOAD_CAM = 37,
    MENU_REPLOAD_CEL = 38,
    MENU_OPT_CAM = 39,
    MENU_OPT_LANG = 40,
    MENU_OPT_SUBTITLE = 41,
    MENU_OPT_VIB = 42,
    MENU_OPT_BUTTON = 43,
    MENU_SAVE_STAGE = 44,
    MENU_SAVE_REPLAY = 45,
    MENU_SAVE_CEL = 46,
    MENU_SAVE_NAME = 47,
    MENU_DUMY_CHAR = 48,
    MENU_SAVE_CEL2 = 49
};

typedef struct { // 0x188
    /* 0x000 */ int nRound;
    /* 0x004 */ char name[12];
    /* 0x010 */ char name1[12];
    /* 0x01c */ char name2[12];
    /* 0x028 */ u_int clrFlg[4];
    /* 0x038 */ int clrCount[8];
    /* 0x058 */ int clrVSCOM1[8];
    /* 0x078 */ int clrCOOL[8];
    /* 0x098 */ u_int logCOOL[8]; /* per stage: the last four COOL clears, one nibble each
                                   * (the round, 1-4), newest in the low nibble; the map
                                   * shows one crown per entry */
    /* 0x0b8 */ GAME_STATUS game_status;
} P3LOG_VAL;

typedef struct { // 0x58
    /* 0x00 */ int nStage;
    /* 0x04 */ int nMode; /* PLAY_MODE */
    /* 0x08 */ int vsLev;
    /* 0x0c */ P3LOG_VAL *pLog;
    /* 0x10 */ GAME_STATUS *pGameStatus;
    /* 0x14 */ MC_REP_STR *pReplayArea;
    /* 0x18 */ int endFlg; /* SEL_MENU_ENUM: why the menu was entered, and so where it starts */
    /* 0x1c */ u_int score;
    /* 0x20 */ u_int score2P;
    /* 0x24 */ u_int bonusG;
    /* 0x28 */ int bCoolClr;
    /* 0x2c */ int winPlayer;
    /* 0x30 */ int endingGame;
    /* 0x34 */ int isWipeEnd;
    /* 0x38 */ int isState;
    /* 0x3c */ short *pAutoMove;
    /* 0x40 */ short autoMovePos[10];
    /* 0x54 */ int curRecJacket;
} P3GAMESTATE;

enum {
    P3MRET_SELECT = 0,
    P3MRET_ABORT = 1,
    P3MRET_PLAYGAME = 2,
    P3MRET_REPLAY = 3,
    P3MRET_TOTITLE = 4
};

int MenuMemCardCheck(void);
int MenuCtrl(MENU_STR *menu_str_ptr);

#endif /* MENU_H */
