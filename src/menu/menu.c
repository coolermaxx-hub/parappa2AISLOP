#include "menu/menu.h"

#include "os/mtc.h"
#include "os/system.h"

#include "main/wipe.h"

#include "menu/menudata.h"
#include "menu/menusub.h"

#include <prlib/prlib.h>

#include <string.h>

static P3LOG_VAL P3Log_Val;

static int bFirst;
static int VsLev;

static void menuDraw(void *x);
static void menuDrawReq(void);
static void menuDrawQuit(void);
static void P3LogInit(P3LOG_VAL *plog);

static void menuDraw(void *x) {
    while (1) {
        TsMenu_Draw();
        MtcWait(1);
    }
}

static void menuDrawReq(void) {
    PrSetFrameRate(60.0f);
    MtcExec(menuDraw, MTC_TASK_07);
}

static void menuDrawQuit(void) {
    MtcKill(MTC_TASK_07);
}

int MenuMemCardCheck(void) {
    bFirst = TRUE;
    VsLev = 0;

    P3LogInit(&P3Log_Val);

    TsMENU_InitSystem();
    TsMenu_RankingClear();
    TsMenu_Init(0, NULL);

    menuDrawReq();
    MenuMsgInit();

    while (!TsMenuMemcChk_Flow()) {
        MtcWait(1);
    }

    TsMenu_End();
    menuDrawQuit();
    return 0;
}

#if 1
/*
 * (poly):
 * Hopefully this can be matched without resorting to
 * these hacks.
 * Leaving register info in case it's necessary.
 */
int MenuCtrl(/* s0 16 */ MENU_STR *menu_str_ptr) {
    /* s0 16 */ int ret;
    /* 0x0(sp) */ P3GAMESTATE P3GameState;
    /* a3 7 */ GAME_STATUS *pGStatus;
    /* a0 4 */ int win;
    /* a1 5 */ int lost;

    MenuDataSndInit();

    P3GameState.pLog = &P3Log_Val;

    P3GameState.vsLev = VsLev;
    P3GameState.pGameStatus = menu_str_ptr->game_status_p;
    P3GameState.pReplayArea = menu_str_ptr->mc_rep_str_p;
    P3GameState.endFlg = menu_str_ptr->sel_menu_enum;
    
    pGStatus = menu_str_ptr->game_status_p;

    P3GameState.nStage = pGStatus->play_stageG;

    P3GameState.nMode = pGStatus->play_modeG;
    P3GameState.pLog->nRound = P3GameState.pLog->nRound;
    P3GameState.isState = 0;
    P3GameState.pLog = &P3Log_Val;

    if (bFirst) {
        P3GameState.pLog->nRound = 0;
        P3GameState.pLog->clrFlg[0] = 0;

        P3GameState.nStage = 0;
        P3GameState.endFlg = 0;
    } else {
        P3GameState.nStage = pGStatus->play_stageG;
    }

    bFirst = FALSE;

    P3GameState.nMode = pGStatus->play_modeG;

    P3GameState.bCoolClr = (pGStatus->disp_level == DLVL_COOL);

    P3GameState.bonusG = pGStatus->bonusG;

    P3GameState.score = pGStatus->scoreG[0];
    P3GameState.score2P = pGStatus->scoreG[1];

    win = global_data.global_ply->vsWin;
    lost = global_data.global_ply->vsLost;

    if (win == lost) {
        P3GameState.winPlayer = 2;
    } else {
        if (win > lost) {
            P3GameState.winPlayer = 0;
        } else {
            P3GameState.winPlayer = 1;
        }
    }

    P3GameState.isState = 0;

    TsMenu_Init(1, &P3GameState);
    menuDrawReq();
    MenuMsgInit();

    TsMenu_InitFlow(&P3GameState);

    while (1) {
        P3GameState.isWipeEnd = WipeEndCheck();

        ret = TsMenu_Flow();
        if (ret != 0) {
            break;
        }

        MtcWait(1);
    }

    MenuDataSndQuit();
    menuDrawQuit();
    TsMenu_End();

    switch (ret) {
    case P3MRET_SELECT:
        break;
    case P3MRET_PLAYGAME:
        P3GameState.endFlg = 1;
        break;
    case P3MRET_REPLAY:
        P3GameState.endFlg = 2;
        break;
    case P3MRET_TOTITLE:
        P3GameState.endFlg = 0;
        break;
    }

    pGStatus = P3GameState.pGameStatus;

    pGStatus->play_stageG = P3GameState.nStage;
    pGStatus->play_modeG = P3GameState.nMode;

    if (pGStatus->play_modeG != PLAY_MODE_SINGLE) {
        switch (P3GameState.vsLev) {
        case 0:
            pGStatus->level_vs_enumG = LVS_1;
            break;
        case 1:
            pGStatus->level_vs_enumG = LVS_2;
            break;
        case 2:
            pGStatus->level_vs_enumG = LVS_3;
            break;
        default:
            pGStatus->level_vs_enumG = LVS_4;
            break;
        }

        VsLev = P3GameState.vsLev;
    } else {
        pGStatus->level_vs_enumG = LVS_1;
        VsLev = 0;
    }

    switch (P3GameState.pLog->nRound) {
    case 0:
        pGStatus->roundG = TRND_R1;
        break;
    case 1:
        pGStatus->roundG = TRND_R2;
        break;
    case 2:
        pGStatus->roundG = TRND_R3;
        break;
    default:
        pGStatus->roundG = TRND_R4;
        break;
    }

    if (ret == P3MRET_PLAYGAME) {
        pGStatus->endingFlag = P3GameState.endingGame;
    } else {
        pGStatus->endingFlag = 0;
    }

    pGStatus->stClearBit = P3GameState.pLog->clrFlg[0];

    if (ret == P3MRET_REPLAY) {
        pGStatus->demo_flagG = DEMOF_REPLAY;
    } else {
        pGStatus->demo_flagG = DEMOF_OFF;
    }

    return (ret == P3MRET_TOTITLE);
}
#else /* In an ideal world: */
/* https://decomp.me/scratch/gAbue */
int MenuCtrl(MENU_STR *menu_str_ptr) {
    int          ret;
    P3GAMESTATE  P3GameState;
    GAME_STATUS *pGStatus;
    int          win, lost;

    MenuDataSndInit();

    P3GameState.vsLev = VsLev;
    P3GameState.pGameStatus = menu_str_ptr->game_status_p;
    P3GameState.pReplayArea = menu_str_ptr->mc_rep_str_p;
    P3GameState.endFlg = menu_str_ptr->sel_menu_enum;

    pGStatus = menu_str_ptr->game_status_p;

    P3GameState.nStage = pGStatus->play_stageG;
    P3GameState.nMode = pGStatus->play_modeG;

    P3GameState.isState = 0;
    P3GameState.pLog = &P3Log_Val;

    if (bFirst) {
        P3Log_Val.nRound = 0;
        P3Log_Val.clrFlg[0] = 0;
        P3GameState.nStage = 0;
        P3GameState.endFlg = 0;
    } else {
        P3GameState.nStage = pGStatus->play_stageG;
    }

    bFirst = FALSE;

    P3GameState.nMode = pGStatus->play_modeG;
    P3GameState.bCoolClr = (pGStatus->disp_level == DLVL_COOL);
    P3GameState.bonusG = pGStatus->bonusG;

    P3GameState.score = pGStatus->scoreG[0];
    P3GameState.score2P = pGStatus->scoreG[1];

    win = global_data.global_ply->vsWin;
    lost = global_data.global_ply->vsLost;

    if (win == lost) {
        P3GameState.winPlayer = 2;
    } else if (win > lost) {
        P3GameState.winPlayer = 0;
    } else {
        P3GameState.winPlayer = 1;
    }

    P3GameState.isState = 0;

    TsMenu_Init(1, &P3GameState);
    menuDrawReq();
    MenuMsgInit();

    TsMenu_InitFlow(&P3GameState);

    while (1) {
        P3GameState.isWipeEnd = WipeEndCheck();

        ret = TsMenu_Flow();
        if (ret != 0) {
            break;
        }

        MtcWait(1);
    }

    MenuDataSndQuit();
    menuDrawQuit();
    TsMenu_End();

    switch (ret) {
    case P3MRET_SELECT:
        break;
    case P3MRET_PLAYGAME:
        P3GameState.endFlg = 1;
        break;
    case P3MRET_REPLAY:
        P3GameState.endFlg = 2;
        break;
    case P3MRET_TOTITLE:
        P3GameState.endFlg = 0;
        break;
    }

    pGStatus = P3GameState.pGameStatus;
    pGStatus->play_stageG = P3GameState.nStage;
    pGStatus->play_modeG = P3GameState.nMode;
    if (pGStatus->play_modeG != PLAY_MODE_SINGLE) {
        switch (P3GameState.vsLev) {
        case 0:
            pGStatus->level_vs_enumG = LVS_1;
            break;
        case 1:
            pGStatus->level_vs_enumG = LVS_2;
            break;
        case 2:
            pGStatus->level_vs_enumG = LVS_3;
            break;
        default:
            pGStatus->level_vs_enumG = LVS_4;
            break;
        }

        VsLev = P3GameState.vsLev;
    } else {
        pGStatus->level_vs_enumG = LVS_1;
        VsLev = 0;
    }

    switch (P3GameState.pLog->nRound) {
    case 0:
        pGStatus->roundG = TRND_R1;
        break;
    case 1:
        pGStatus->roundG = TRND_R2;
        break;
    case 2:
        pGStatus->roundG = TRND_R3;
        break;
    default:
        pGStatus->roundG = TRND_R4;
        break;
    }

    if (ret == P3MRET_PLAYGAME) {
        pGStatus->endingFlag = P3GameState.endingGame;
    } else {
        pGStatus->endingFlag = 0;
    }

    pGStatus->stClearBit = P3GameState.pLog->clrFlg[0];
    if (ret == P3MRET_REPLAY) {
        pGStatus->demo_flagG = DEMOF_REPLAY;
    } else {
        pGStatus->demo_flagG = DEMOF_OFF;
    }

    return (ret == P3MRET_TOTITLE);
}
#endif

static void P3LogInit(P3LOG_VAL *plog) {
    memset(plog, 0, sizeof(*plog));
}
