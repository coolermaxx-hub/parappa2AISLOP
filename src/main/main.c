#include "main/main.h"

#include "dbug/dbgmsg.h"
#include "dbug/dbug.h"

#include "os/cmngifpk.h"
#include "os/mtc.h"
#include "os/syssub.h"
#include "os/system.h"
#include "os/tim2.h"
#include "os/usrmem.h"

#include "main/cdctrl.h"
#include "main/cmnfile.h"
#include "main/etc.h"
#include "main/fadectrl.h"
#include "main/p3str.h"
#include "main/scrctrl.h"
#include "main/stdat.h"
#include "main/wipe.h"

#include "menu/menu.h"

#include "iop_mdl/tapctrl_rpc.h"

#include <libpad.h>

#include <math.h>
#include <stdio.h>

static int urawaza_skip_bottun = FALSE;
static int urawaza_levelsel_bottun = -1;

DBG_SELECT_STR dbg_select_str = {
    .debug_on     = FALSE,
    .use_line     = -1,
    .score_updown = FALSE,
    .non_play     = FALSE
};

static u_char *dbg_tbl_msg[] = {
    "AUTO", "BASE",
    "LV1",  "LV2",  "LV3",  "LV4",
    "LV5",  "LV6",  "LV7",  "LV8",
    "LV9",  "LV10", "LV11", "LV12",
    "LV13", "LV14", "LV15", "LV16",
};

static u_char *dbg_score_msg[] = {
    "OFF", "ON",
};

static DBG_MODE_STR dbg_mode_str[] = {
    {
        .msg_pp = "START",
        .set_pp = NULL,
        .min = 0,
        .max = 0,
        .selmsg_pp = NULL
    },
    {
        .msg_pp = "TABLE",
        .set_pp = &dbg_select_str.use_line,
        .min = -1,
        .max = 17,
        .selmsg_pp = dbg_tbl_msg
    },
    {
        .msg_pp = "SCORE DBUG",
        .set_pp = &dbg_select_str.score_updown,
        .min = 0,
        .max = 2,
        .selmsg_pp = dbg_score_msg
    },
    {
        .msg_pp = "NON PLAY",
        .set_pp = &dbg_select_str.non_play,
        .min = 0,
        .max = 2,
        .selmsg_pp = dbg_score_msg
    },
};

int overlay_loadaddr = 0x01ca0000; /* TODO: Don't hardcode */

static MENU_STR menu_str;

static void dbg_select_disp(void) {
    int           selpos;
    DBG_MODE_STR *dbg_pp;
    int           i;

    selpos = 0;
    DbgMsgInit();

    while (1) {
        MtcWait(1);

        dbg_pp = &dbg_mode_str[selpos];
        if (pad[0].one & (SCE_PADRleft | SCE_PADRright)) {
            if (dbg_pp->set_pp == NULL) {
                break;
            }

            if (pad[0].one & SCE_PADRright) {
                (*dbg_pp->set_pp)++;
            } else {
                (*dbg_pp->set_pp)--;
            }

            if (*dbg_pp->set_pp < dbg_pp->min) {
                *dbg_pp->set_pp = dbg_pp->max - 1;
            }

            if (*dbg_pp->set_pp >= dbg_pp->max) {
                *dbg_pp->set_pp = dbg_pp->min;
            }
        } else {
            if (pad[0].one & SCE_PADLup) {
                selpos--;
            } else if (pad[0].one & SCE_PADLdown) {
                selpos++;
            }
        }

        if (selpos < 0) {
            selpos = 3;
        }
        if (selpos > PR_ARRAYSIZEU(dbg_mode_str) - 1) {
            selpos = 0;
        }

        DbgMsgClear();
        DbgMsgSetSize(16, 10);
        DbgMsgSetColor(0, 128, 0);
        DbgMsgPrint("= DEBUG MENU =", 1800, 1950);

        for (i = 0; i < PR_ARRAYSIZEU(dbg_mode_str); i++) {
            if (i == selpos) {
                DbgMsgSetColor(128, 128, 0);
            } else {
                DbgMsgSetColor(128, 128, 128);
            }

            DbgMsgPrint(dbg_mode_str[i].msg_pp, 1800, (1968 + (i*12)));
            if (dbg_mode_str[i].selmsg_pp != NULL) {
                int numkun = *dbg_mode_str[i].set_pp - dbg_mode_str[i].min;

                DbgMsgPrint(dbg_mode_str[i].selmsg_pp[numkun], 2000, (1968 + (i*12)));
            }
        }

        DbgMsgFlash();
    }

    MtcWait(1);
}

/* Debug stand-in for a stage (dummyPlay): which result screen it shows and which button ended it. */
enum {
    DUMMY_MODE_TITLE = 0,
    DUMMY_MODE_REPLAY = 1,
    DUMMY_MODE_SINGLE = 2,
    DUMMY_MODE_VS_MAN = 3,
    DUMMY_MODE_VS_COM = 4
};

enum {
    DUMMY_PRESS_CIRCLE = 1,  /* single: GOOD */
    DUMMY_PRESS_CROSS = 2,   /* NG / exit */
    DUMMY_PRESS_TRIANGLE = 3 /* single: COOL */
};

static void dummyPlay(int retTitle) {
    int     mode;
    int     ret = 0;
    long    scoreTmp[2] = {};

    u_char *msgDmy[5] = {
        "TITLE   A or O or X EXIT",
        "REPLAY  A or O or X EXIT",
        "SINGLE  A .. COOL O .. GOOD  X .. NG",
        "VS MAN  A..1p win O..2p win X..EXIT",
        "VS COM  A or O .. CLEAR  X .. NG",
    };
    u_char msg_dat[80];

    if (retTitle != 0) {
        mode = DUMMY_MODE_TITLE;
    } else if (game_status.demo_flagG == DEMOF_REPLAY) {
        mode = DUMMY_MODE_REPLAY;
    } else if (game_status.play_modeG == PLAY_MODE_SINGLE) {
        mode = DUMMY_MODE_SINGLE;
    } else if (game_status.play_modeG == PLAY_MODE_VS_MAN) {
        mode = DUMMY_MODE_VS_MAN;
    } else {
        mode = DUMMY_MODE_VS_COM;
    }

    game_status.bonusG = 0;

    DbgMsgInit();

    while (1) {
        u_char *pmd[3]   = { "SINGLE", "VS MAN", "VS COM" };
        u_char *ptype[2] = { "NORMAL", "EASY" };

        if (pad[0].one & SCE_PADRright) {
            ret = DUMMY_PRESS_CIRCLE;
        } else if (pad[0].one & SCE_PADRup) {
            ret = DUMMY_PRESS_TRIANGLE;
        } else if (pad[0].one & SCE_PADRdown) {
            ret = DUMMY_PRESS_CROSS;
        }

        if (ret != 0) {
            break;
        }

        msg_dat[0] = '\0';
        DbgMsgClear();
        DbgMsgSetSize(16, 10);
        DbgMsgSetColor(128, 128, 128);

        DbgMsgPrint(msgDmy[mode], 1780, 1968);

        switch (mode) {
        case DUMMY_MODE_SINGLE:
        case DUMMY_MODE_VS_MAN:
        case DUMMY_MODE_VS_COM:
            if (pad[0].shot & SCE_PADL1) {
                scoreTmp[0] += 10;
            }
            if (pad[0].shot & SCE_PADL2) {
                scoreTmp[0] -= 10;
            }

            if (pad[0].shot & SCE_PADR1) {
                scoreTmp[1] += 10;
            }
            if (pad[0].shot & SCE_PADR2) {
                scoreTmp[1] -= 10;
            }

            if (scoreTmp[0] < 0) {
                scoreTmp[0] = 0;
            }
            if (scoreTmp[1] < 0) {
                scoreTmp[1] = 0;
            }

            sprintf(msg_dat, "STG:%d MODE:%s TYPE:%s ROUND:%d", game_status.play_stageG, pmd[game_status.play_modeG], ptype[game_status.play_table_modeG], game_status.roundG + 1);
            DbgMsgPrint(msg_dat, 1770, 2000);

            DbgMsgSetColor(0, 255, 0);
            DbgMsgPrint("SCORE 1P  L1 .. UP   L2 .. DOWN", 1770, 2020);
            DbgMsgPrint("SCORE 2P  R1 .. UP   R2 .. DOWN", 1770, 2032);

            DbgMsgSetColor(255, 255, 0);
            sprintf(msg_dat, "1P:%5d    2P:%5d", (int)scoreTmp[0], (int)scoreTmp[1]);
            DbgMsgPrint(msg_dat, 1770, 0x7fe);
            break;
        case DUMMY_MODE_REPLAY:
            sprintf(msg_dat, "STG:%d MODE:%s TYPE:%s ROUND:%d", mc_rep_str.play_stageS, pmd[mc_rep_str.play_modeS], ptype[mc_rep_str.play_typeS], mc_rep_str.roundS + 1);
            DbgMsgPrint(msg_dat, 1770, 2000);
        }

        DbgMsgFlash();
        MtcWait(1);
    }

    switch (mode) {
    case DUMMY_MODE_SINGLE:
        if (ret == DUMMY_PRESS_CIRCLE || ret == DUMMY_PRESS_TRIANGLE) {
            u_int clrcnt;

            if (game_status.endingFlag == ENDING_MOVIE) {
                while (1) {
                    MtcWait(1);
                    if (pad[0].one & (SCE_PADRup | SCE_PADRright | SCE_PADRdown)) {
                        break;
                    }

                    DbgMsgClear();
                    DbgMsgSetSize(16, 10);
                    DbgMsgSetColor(128, 128, 128);
                    DbgMsgPrint("ENDING   A or O or X  EXIT", 1800, 1968);
                    DbgMsgFlash();
                }
            } else if (game_status.endingFlag == ENDING_BONUS_1 || game_status.endingFlag == ENDING_BONUS_2 || game_status.endingFlag == ENDING_BONUS_3) {
                while (1) {
                    MtcWait(1);
                    if (pad[0].one & (SCE_PADRup | SCE_PADRright | SCE_PADRdown)) {
                        break;
                    }

                    DbgMsgClear();
                    DbgMsgSetSize(16, 10);
                    DbgMsgSetColor(128, 128, 128);
                    DbgMsgPrint("BONUS GAME   A or O or X  EXIT", 1800, 1968);
                    DbgMsgFlash();
                }
            }

            menu_str.sel_menu_enum = SEL_MENU_SAVE;

            mc_rep_str.play_stageS = game_status.play_stageG;
            mc_rep_str.play_modeS = game_status.play_modeG;
            mc_rep_str.play_typeS = game_status.play_typeG;
            mc_rep_str.play_table_modeS = game_status.play_table_modeG;
            mc_rep_str.roundS = game_status.roundG;
            
            if (ret == DUMMY_PRESS_CIRCLE) {
                clrcnt = game_status.stClrCntGood[game_status.play_stageG];
                if (clrcnt != -1) {
                    clrcnt++;
                }
                game_status.stClrCntGood[game_status.play_stageG] = clrcnt;

                game_status.disp_level = DLVL_GOOD;
            } else {
                clrcnt = game_status.stClrCntCool[game_status.play_stageG];
                if (clrcnt != -1) {
                    clrcnt++;
                }
                game_status.stClrCntCool[game_status.play_stageG] = clrcnt;

                game_status.disp_level = DLVL_COOL;
            }

            game_status.scoreG[0] = scoreTmp[0];
            game_status.scoreG[1] = 0;
        } else {
            game_status.disp_level = DLVL_BAD;

            menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
            game_status.scoreG[0] = 0;
            game_status.scoreG[1] = 0;
        }    
        break;
    case DUMMY_MODE_VS_COM:
        if (ret == DUMMY_PRESS_CIRCLE || ret == DUMMY_PRESS_TRIANGLE) {
            u_int clrcnt;

            menu_str.sel_menu_enum = SEL_MENU_SAVE;

            mc_rep_str.play_stageS = game_status.play_stageG;
            mc_rep_str.play_modeS = game_status.play_modeG;
            mc_rep_str.play_typeS = game_status.play_typeG;
            mc_rep_str.roundS = game_status.roundG;
            mc_rep_str.play_table_modeS = game_status.play_table_modeG;

            clrcnt = game_status.stClrCntVs[game_status.play_stageG];
            if (clrcnt != -1) {
                clrcnt++;
            }
            game_status.stClrCntVs[game_status.play_stageG] = clrcnt;

            game_status.scoreG[0] = scoreTmp[0];
            game_status.scoreG[1] = scoreTmp[1];

            /* Player 1 beats the computer 3-0. */
            {
                GLOBAL_PLY *gply_pp = &global_data.global_ply[0];

                gply_pp->vsWin = 3;
                gply_pp->vsLost = 0;

                gply_pp = &global_data.global_ply[1];
                gply_pp->vsWin = 0;
                gply_pp->vsLost = 3;
            }
        } else {
            menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
            game_status.scoreG[0] = 0;
            game_status.scoreG[1] = 0;
        }
        break;
    case DUMMY_MODE_VS_MAN:
        if (ret == DUMMY_PRESS_CIRCLE || ret == DUMMY_PRESS_TRIANGLE) {
            menu_str.sel_menu_enum = SEL_MENU_SAVE;

            game_status.scoreG[0] = scoreTmp[0];
            game_status.scoreG[1] = scoreTmp[1];

            mc_rep_str.play_stageS = game_status.play_stageG;
            mc_rep_str.play_modeS = game_status.play_modeG;
            mc_rep_str.play_typeS = game_status.play_typeG;
            mc_rep_str.roundS = game_status.roundG;
            mc_rep_str.play_table_modeS = game_status.play_table_modeG;

            if (ret == DUMMY_PRESS_CIRCLE) {
                GLOBAL_PLY *gply_pp;
                gply_pp = &global_data.global_ply[0];
                gply_pp->vsWin = 0;
                gply_pp->vsLost = 3;

                gply_pp = &global_data.global_ply[1];
                gply_pp->vsWin = 3;
                gply_pp->vsLost = 0;
            } else {
                GLOBAL_PLY *gply_pp;

                gply_pp = &global_data.global_ply[0];
                gply_pp->vsWin = 3;
                gply_pp->vsLost = 0;

                gply_pp = &global_data.global_ply[1];
                gply_pp->vsWin = 0;
                gply_pp->vsLost = 3;
            }
        } else {
            menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
            game_status.scoreG[0] = 0;
            game_status.scoreG[1] = 0;
        }
        break;
    case DUMMY_MODE_REPLAY:
        menu_str.sel_menu_enum = SEL_MENU_REPLAY;
        break;
    default:
        menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
        break;
    }
}

int selPlayDisp(int sel_stage, int sel_disp, int firstf) {
    STDAT_DAT *stdat_dat_pp;
    int        ret = 0;

    ReportHeapUsage();
    printf("=== selPlayDisp stg:%d disp:%d ===\n", sel_stage, sel_disp);

    /* Load stage overlay */
    printf("overlay module load in\n");
    CdctrlRead(&stdat_rec[sel_stage].ovlfile, overlay_loadaddr, NULL);
    CdctrlReadWait();
    printf("overlay module load out\n");

    asm volatile("sync.l");
    FlushCache(WRITEBACK_DCACHE);

    stdat_dat_pp = &stdat_rec[sel_stage].stdat_dat_pp[sel_disp];

    CdctrlGetFileSize(&stdat_dat_pp->intfile);
    CdctrlRead(&stdat_dat_pp->intfile, UsrMemAllocNext(), UsrMemAllocEndNext());
    CdctrlReadWait();

    cmnfTim2Trans();

    global_data.play_stageL = sel_stage;
    GlobalPlySet(&global_data, stdat_dat_pp->play_step, sel_stage);
    GlobalTimeInit(&global_data);
    GlobalSetTempo(&global_data, stdat_rec[sel_stage].stdat_dat_pp[sel_disp].tempo);

    ScrCtrlInit(stdat_dat_pp, (void*)UsrMemGetAdr(0));

    while (1) {
        MtcWait(1);

        if (ScrCtrlInitCheck()) {
            break;
        }
    }

    if (!firstf) {
        while (!WipeEndCheck()) {
            MtcWait(1);
        }
    }

    if (firstf) {
        FadeCtrlReq(FMODE_BLACK_IN, 120);
    }

    ScrCtrlGoLoop();

    if (!firstf) {
        WipeOutReq();
    }

    PrSetPostureWorkArea(UsrMemAllocNext(), UsrMemAllocEndNext() - UsrMemAllocNext());
    DrawCtrlInit(stdat_dat_pp->ev_pp, global_data.draw_tbl_top, (void*)UsrMemGetAdr(0));
    PrSetPostureWorkArea(NULL, NULL);

    DrawCtrlTimeSet(0);
    MtcWait(1);

    ReportHeapUsage();

    if (firstf) {
        MtcWait(120);
    }

    while (1) {
        MtcWait(1);

        if ((pad[0].one & SCE_PADstart) && WipeEndCheck()) {
            ret = 1;
            break;
        }

        if (ScrEndCheckScore()) {
            break;
        }

        if (ScrEndCheckFadeOut() && global_data.demo_flagL == DEMOF_DEMO) {
            ret = 2;
            break;
        }
    }

    if (ret == 2) {
        CdctrlSndFadeOutWait(120);
        ret = 0;
    }

    DrawCtrlQuit();
    CdctrlWP2SetVolume(0);
    CdctrlWp2FileEnd();
    ScrCtrlQuit();
    CdctrlMasterVolSet(0x3fff);

    return ret;
}

/* Analog stick axis (0..255, centre ~0x80) beyond which the hat changes. */
#define HAT_STICK_LOW  0x40
#define HAT_STICK_HIGH 0xc0

static void SpHatChangeSub(void) {
    PADD *pad_pp;

    hat_change_enum = HCNG_AUTO;

    if (game_status.roundG < TRND_R4) {
        return;
    }

    pad_pp = &pad[0];

    if (pad_pp->ana[1] < HAT_STICK_LOW) {
        hat_change_enum = HCNG_R1;
    } else if (pad_pp->ana[0] >= HAT_STICK_HIGH) {
        hat_change_enum = HCNG_R2;
    } else if (pad_pp->ana[1] >= HAT_STICK_HIGH) {
        hat_change_enum = HCNG_R3;
    } else if (pad_pp->ana[0] < HAT_STICK_LOW) {
        hat_change_enum = HCNG_R4;
    }

    {
        int           rt2t_r1[3] = { 0xb6, 0xb7, 0xb8 };
        int           rt2t_r2[3] = { 0xb9, 0xba, 0xbb };
        int           rt2t_r3[3] = { 0xbc, 0xbd, 0xbe };
        int           rt2t_r4[3] = { 0xbf, 0xc0, 0xc1 };
        RT2TRANS_STR  rt2trans_str[4] = {
            { .num = 3, .data_pp = rt2t_r1 },
            { .num = 3, .data_pp = rt2t_r2 },
            { .num = 3, .data_pp = rt2t_r3 },
            { .num = 3, .data_pp = rt2t_r4 },
        };
    
        RT2TRANS_STR *rt2trans_str_pp = &rt2trans_str[GetHatRound()];
        int           i;

        for (i = 0; i < rt2trans_str_pp->num; i++) {
            Tim2Trans(GetIntAdrsCurrent(rt2trans_str_pp->data_pp[i]));
        }
    }
}

int selPlayDispTitleDisp(int sel_stage, int sel_disp, int ovl_load) {
    STDAT_DAT *stdat_dat_pp;
    int        ret = 0;

    ReportHeapUsage();
    printf("=== selPlayDisp stg:%d disp:%d ===\n", sel_stage, sel_disp);

    if (ovl_load) {
        printf("overlay module load in\n");
        CdctrlRead(&stdat_rec[sel_stage].ovlfile, overlay_loadaddr, NULL);
        CdctrlReadWait();
        printf("overlay module load out\n");
    }

    asm volatile("sync.l");
    FlushCache(WRITEBACK_DCACHE);

    stdat_dat_pp = &stdat_rec[sel_stage].stdat_dat_pp[sel_disp];

    global_data.play_stageL = sel_stage;
    GlobalPlySet(&global_data, stdat_dat_pp->play_step, sel_stage);
    GlobalTimeInit(&global_data);
    GlobalSetTempo(&global_data, stdat_rec[sel_stage].stdat_dat_pp[sel_disp].tempo);

    ScrCtrlInit(stdat_dat_pp, (void*)UsrMemGetAdr(0));

    while (1) {
        MtcWait(1);

        if (ScrCtrlInitCheck()) {
            break;
        }
    }

    ScrCtrlGoLoop();
    WipeOutReq();

    PrSetPostureWorkArea(UsrMemAllocNext(), UsrMemAllocEndNext() - UsrMemAllocNext());
    DrawCtrlInit(stdat_dat_pp->ev_pp, global_data.draw_tbl_top, (void*)UsrMemGetAdr(0));
    PrSetPostureWorkArea(NULL, NULL);

    DrawCtrlTimeSet(0);
    MtcWait(1);

    ReportHeapUsage();

    while (1) {
        MtcWait(1);

        if (ScrEndCheckTitle()) {
            ret = 1;
            break;
        }
        
        if (ScrEndCheckScore()) {
            break;
        }

        SpHatChangeSub();

        if (pad[0].one & SCE_PADLdown) {
            dbg_select_str.debug_on ^= 1;
        }

        if (dbg_select_str.debug_on) {
            sceGifPacket dbgPk;

            DbgMsgInit();
            DbgMsgClear();

            CmnGifOpenCmnPk(&dbgPk);
            DbgMsgClearUserPkt(&dbgPk);
            DbgMsgPrintUserPkt("DEBUG", 1730, 1948, &dbgPk);
            CmnGifCloseCmnPk(&dbgPk, 7);
        }
    }

    if (ret != 0) {
        MtcWait(60);
    }

    DrawCtrlQuit();
    CdctrlWP2SetVolume(0);
    CdctrlWp2FileEnd();
    ScrCtrlQuit();
    return ret;
}

void xtrView(FILE_STR *file_str_pp) {
    int timer;
    int seek_top;

    timer = 0;

    CdctrlXTRset(file_str_pp, UsrMemAllocNext());
    seek_top = getTopSeekPos();

    CdctrlWP2Play();
    CdctrlWP2SetVolume(0x3fff);

    while (1) {
        MtcWait(1);

        if (pad[0].one & SCE_PADstart || CdctrlWP2PlayEndCheck() || timer >= 6540) {
            break;
        }

        CdctrlWp2GetSampleTmpBuf();

        timer = CdctrlWp2CdSample2Frame(CdctrlWp2GetSampleTmp() - seek_top);
        p3StrPoll(timer);

        timer++;
    }

    CdctrlWp2FileEnd();
    p3StrQuitSd();
}

void logoDispOne(SPR_PRIM *sprm_pp, TIM2_DAT *tmd_pp) {
    int timer;

    sprm_pp->w = tmd_pp->w;
    sprm_pp->h = tmd_pp->h;
    FadeCtrlReq(FMODE_BLACK_IN, 30);

    timer = 29;
    while (timer != -1) {
        timer--;

        SprClear();
        SprPackSet((SPR_DAT*)tmd_pp);
        SprDisp(sprm_pp);
        SprFlash();

        MtcWait(1);
    }

    timer = 119;
    while (timer != -1) {
        timer--;

        SprClear();
        SprPackSet((SPR_DAT*)tmd_pp);
        SprDisp(sprm_pp);
        SprFlash();

        MtcWait(1);
    }

    FadeCtrlReq(FMODE_BLACK_OUT, 30);

    timer = 29;
    while (timer != -1) {
        timer--;

        SprClear();
        SprPackSet((SPR_DAT*)tmd_pp);
        SprDisp(sprm_pp);
        SprFlash();

        MtcWait(1);
    }
}

static int uramen_end_flag = FALSE;

static void uramenFileSearchTask(void *x) {
    printf("file search in\n");
    stDatFirstFileSearch();
    printf("file search out\n");

    uramen_end_flag = FALSE;
    MtcExit();
}

static void uramenFileSearchSet(void) {
    uramen_end_flag = TRUE;
    MtcExec(uramenFileSearchTask, MTC_TASK_03);
}

static void uramenFileSearchEnd(void) {
    if (uramen_end_flag) {
        while (uramen_end_flag) {
            MtcWait(1);
        }
    }
}

/* note: has inline from 'data/logo/logo_tm2.h' */
void startUpDisp(void) {
    /* Splash screens TIM2 data */
    static TIM2_DAT tim2spr_tbl[2] = {
        /* NanaOn-Sha */
        {
            .GsTex0 = SCE_GS_SET_TEX0(10240, 6, SCE_GS_PSMT8, 9, 8, 0, 0, 10320, SCE_GS_PSMCT32, 0, 0, 1),
            .GsTex1 = GS_TEX1_LINEAR,
            .w = 320,
            .h = 224
        },
        /* SCEI presents */
        {
            .GsTex0 = SCE_GS_SET_TEX0(10600, 8, SCE_GS_PSMT4, 9, 6, 0, 0, 10324, SCE_GS_PSMCT16, 0, 0, 1),
            .GsTex1 = GS_TEX1_LINEAR,
            .w = 440,
            .h = 52
        },
    };

    /* Splash screens sprite data */
    SPR_PRIM spr_prim[2] = {
        { .x = 2048, .y = 2048, .scalex = 256, .scaley = 128 },
        { .x = 2048, .y = 2048, .scalex = 512, .scaley = 256 }
    };

    UsrMemClear();
    SpuBankSet();

    CdctrlRead(&file_str_logo_file, UsrMemAllocNext(), NULL);
    CdctrlReadWait();

    SprInit();
    MenuMemCardCheck();

    uramenFileSearchSet();
    logoDispOne(&spr_prim[0], &tim2spr_tbl[1]);
    logoDispOne(&spr_prim[1], &tim2spr_tbl[0]);
    uramenFileSearchEnd();

    UsrMemClear();
    SpuBankSet();
}

int selPlayDispType(int sel_stage, int sel_disp, CANCEL_TYPE_ENUM canseltype) {
    STDAT_DAT  *stdat_dat_pp;
    int         fsize;
    int         ret;
    int         tmp_area;
    int         yn_disp_on;
    GLOBAL_PLY *gply_pp;

    stdat_dat_pp = &stdat_rec[sel_stage].stdat_dat_pp[sel_disp];

    if (stdat_dat_pp->play_step == PSTEP_XTR) {
        fsize = CdctrlGetFileSize(&stdat_dat_pp->intfile);
        fsize = ((fsize + 2047) / 2048) * 2048;

        tmp_area = UsrMemEndAlloc(fsize);
        UsrMemEndFree();

        CdctrlRead(&stdat_dat_pp->intfile, UsrMemAllocNext(), tmp_area);
        CdctrlReadWait();
    }

    while (1) {
        ret = FALSE;

        global_data.play_stageL = sel_stage;
        GlobalPlySet(&global_data, stdat_dat_pp->play_step, sel_stage);
        GlobalTimeInit(&global_data);
        GlobalSetTempo(&global_data, stdat_rec[sel_stage].stdat_dat_pp[sel_disp].tempo);

        ScrCtrlInit(stdat_dat_pp, (void*)UsrMemGetAdr(0));

        while (1) {
            MtcWait(1);

            if (ScrCtrlInitCheck()) {
                break;
            }
        }

        while (!WipeEndCheck()) {
            MtcWait(1);
        }

        ScrCtrlGoLoop();
        WipeOutReq();

        PrSetPostureWorkArea(UsrMemAllocNext(), UsrMemAllocEndNext() - UsrMemAllocNext());
        DrawCtrlInit(stdat_dat_pp->ev_pp, global_data.draw_tbl_top, (void*)UsrMemGetAdr(0));
        PrSetPostureWorkArea(NULL, 0);
        DrawCtrlTimeSet(0);

        MtcWait(1);

        while (1) {
            MtcWait(1);

            if (canseltype != CBE_HOOK) {
                int btn = pad[0].one;
                if (canseltype == CBE_VS_MAN) {
                    btn |= pad[1].one;
                }

                if ((btn & SCE_PADstart) && WipeEndCheck() && !ScrEndWaitLoop()) {
                    ret = TRUE;
                    break;
                }
            }

            if (ScrEndCheckScore()) {
                break;
            }
        }

        DrawCtrlQuit();

        CdctrlWP2SetVolume(0);
        CdctrlWp2FileEnd();

        ScrCtrlQuit();

        if (canseltype == CBE_NORMAL) {
            break;
        }
        if (canseltype == CBE_HOOK) {
            break;
        }

        if (ret) {
            gply_pp = &global_data.global_ply[0];
        } else {
            if (canseltype == CBE_SINGLE) {
                DISP_LEVEL disp_level;

                gply_pp = &global_data.global_ply[0];

                disp_level = RANK_LEVEL2DISP_LEVEL(gply_pp->rank_level);
                if (disp_level == DLVL_COOL || disp_level == DLVL_GOOD) {
                    break;
                }
            } else if (canseltype == CBE_VS_MAN) {
                gply_pp = &global_data.global_ply[0];
            } else if (canseltype == CBE_VS_COM) {
                gply_pp = &global_data.global_ply[0];
                if (gply_pp->vsWin > gply_pp->vsLost) {
                    break;
                }
            }
        }

        wipeYesNoDispReq();

        while (1) {
            int btn;

            MtcWait(1);

            btn = pad[0].one;
            if (canseltype == CBE_VS_MAN) {
                btn |= pad[1].one;
            }

            if (btn & SCE_PADRright) {
                yn_disp_on = FALSE;
                break;
            }
            if (btn & SCE_PADRdown) {
                yn_disp_on = TRUE;
                break;
            }
        }

        if (!yn_disp_on) {
            wipeParaInReq();
        } else {
            break;
        }
    }

    return ret;
}

int selPlayDispSetPlay(int sel_stage) {
    int        i;
    STDAT_DAT *stdat_dat_pp;
    int        fsize;
    int        ret;

    ret = 0;

    printf("overlay module load out\n");
    CdctrlRead(&stdat_rec[sel_stage].ovlfile, overlay_loadaddr, NULL);
    printf("overlay module load in\n");
    CdctrlReadWait();

    for (i = stdat_rec[sel_stage].stdat_dat_num - 1; i >= 0; i--) {
        stdat_dat_pp = &stdat_rec[sel_stage].stdat_dat_pp[i];

        if (stdat_dat_pp->play_step != PSTEP_XTR) {
            fsize = CdctrlGetFileSize(&stdat_dat_pp->intfile);
            fsize = ((fsize + 2047) / 2048) * 2048;

            CdctrlReadOne(&stdat_dat_pp->intfile, UsrMemEndAlloc(fsize), NULL);
            CdctrlReadWait();
        }
    }

    for (i = 0; i < stdat_rec[sel_stage].stdat_dat_num; i++) {
        stdat_dat_pp = &stdat_rec[sel_stage].stdat_dat_pp[i];

        if (stdat_dat_pp->play_step == PSTEP_XTR) {
            ret = selPlayDispType(sel_stage, i, CBE_NORMAL);
        } else {
            int decp = UsrMemAllocEndNext();

            UsrMemEndFree();

            CdctrlMemIntgDecode(decp, UsrMemAllocNext());

            cmnfTim2Trans();

            if (stdat_dat_pp->play_step == PSTEP_HOOK) {
                GLOBAL_PLY *gply_pp = &global_data.global_ply[0];

                ret = selPlayDispType(sel_stage, i, CBE_HOOK);

                ingame_common_str.HookClrCnt = gply_pp->exam_tbl_up - (gply_pp->exam_tbl_dw / 2);
                if (ingame_common_str.HookClrCnt < 0) {
                    ingame_common_str.HookClrCnt = 0;
                }
                if (ingame_common_str.HookClrCnt > 10) {
                    ingame_common_str.HookClrCnt = 10;
                }

                printf("HOOK cnt:%d\n", ingame_common_str.HookClrCnt);
            } else {
                if (global_data.play_modeL == PLAY_MODE_SINGLE) {
                    ret = selPlayDispType(sel_stage, i, CBE_SINGLE);
                } else if (global_data.play_modeL == PLAY_MODE_VS_MAN) {
                    ret = selPlayDispType(sel_stage, i, CBE_VS_MAN);
                } else if (global_data.play_modeL == PLAY_MODE_VS_COM) {
                    ret = selPlayDispType(sel_stage, i, CBE_VS_COM);
                }
            }
        }

        UsrMemClearTop();

        if (i != (stdat_rec[sel_stage].stdat_dat_num - 1)) {
            PLAY_STEP play_step_tmp = stdat_rec[sel_stage].stdat_dat_pp[i + 1].play_step;

            if (play_step_tmp == PSTEP_GAME) {
                wipeParaInReq();
            } else if (play_step_tmp == PSTEP_HOOK) {
                wipeBoxyInReq();
            } else {
                wipeBoxyWaitReq();
            }

            MtcWait(2);
        }
    }

    return ret;
}

int selPlayDispSetPlayOne(int sel_stage) {
    int        i;
    STDAT_DAT *stdat_dat_pp;
    int        fsize;
    int        ret;

    ret = 0;

    printf("overlay module load out\n");
    CdctrlRead(&stdat_rec[sel_stage].ovlfile, overlay_loadaddr, NULL);
    printf("overlay module load in\n");
    CdctrlReadWait();

    i = stdat_rec[sel_stage].stdat_dat_num - 1;
    stdat_dat_pp = &stdat_rec[sel_stage].stdat_dat_pp[i];

    fsize = CdctrlGetFileSize(&stdat_dat_pp->intfile);
    fsize = ((fsize + 2047) / 2048) * 2048;

    CdctrlReadOne(&stdat_dat_pp->intfile, UsrMemEndAlloc(fsize), NULL);
    CdctrlReadWait();

    {
        int decp = UsrMemAllocEndNext();

        UsrMemEndFree();
        CdctrlMemIntgDecode(decp, UsrMemAllocNext());
    }

    cmnfTim2Trans();

    if (stdat_dat_pp->play_step == PSTEP_HOOK) {
        ret = selPlayDispType(sel_stage, i, CBE_HOOK);
    } else if (global_data.play_modeL == PLAY_MODE_SINGLE) {
        ret = selPlayDispType(sel_stage, i, CBE_SINGLE);
    } else if (global_data.play_modeL == PLAY_MODE_VS_MAN) {
        ret = selPlayDispType(sel_stage, i, CBE_VS_MAN);
    } else if (global_data.play_modeL == PLAY_MODE_VS_COM) {
        ret = selPlayDispType(sel_stage, i, CBE_VS_COM);
    }

    UsrMemClearTop();
    return ret;
}

int gamePlayDisp(void) {
    int         sel_stage;
    GLOBAL_PLY *gply_pp;
    u_int       clrcnt;
    int         cancel_flag;
    int         ret;

    cancel_flag = 0;

    GlobalLobcalCopy();

    game_status.bonusG = 0;

    if (global_data.demo_flagL == DEMOF_REPLAY) {
        mccGlobalLocalCopy();
    } else {
        if (dbg_select_str.debug_on) {
            if (dbg_select_str.use_line < 0) {
                global_data.tapLevelCtrl = LM_AUTO;
            } else {
                global_data.tapLevel = dbg_select_str.use_line;
                global_data.tapLevelCtrl = LM_FIX;
            }
        } else {
            if (urawaza_levelsel_bottun >= 0) {
                global_data.tapLevelCtrl = LM_FIX;
                global_data.tapLevel = urawaza_levelsel_bottun;
            }
        }
    }

    sel_stage = global_data.play_stageL;

    inCmnInit(sel_stage);

    if (global_data.play_modeL != PLAY_MODE_SINGLE) {
        sel_stage += 10;
    }

    if (global_data.demo_flagL != DEMOF_OFF) {
        SpuBankSet();
        UsrMemClear();

        ret = selPlayDisp(sel_stage, stdat_rec[sel_stage].stdat_dat_num - 1, FALSE);
    } else {
        if (urawaza_skip_bottun) {
            cancel_flag = selPlayDispSetPlayOne(sel_stage);
        } else {
            cancel_flag = selPlayDispSetPlay(sel_stage);
        }

        ret = cancel_flag;
    }

    if (game_status.demo_flagG == DEMOF_REPLAY) {
        menu_str.sel_menu_enum = SEL_MENU_REPLAY;
    } else if (game_status.demo_flagG == DEMOF_DEMO) {
        menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
    } else {
        if (game_status.play_modeG == PLAY_MODE_SINGLE) {
            int dsip_level;

            gply_pp = &global_data.global_ply[0];

            dsip_level = RANK_LEVEL2DISP_LEVEL(gply_pp->rank_level);
            game_status.disp_level = dsip_level;

            if ((dsip_level == DLVL_COOL || dsip_level == DLVL_GOOD) && !cancel_flag) {
                mccLocalGlobalCopy();

                game_status.scoreG[0] = gply_pp->score;
                game_status.scoreG[1] = 0;

                menu_str.sel_menu_enum = SEL_MENU_SAVE;

                if (dsip_level == DLVL_GOOD) {
                    clrcnt = game_status.stClrCntGood[game_status.play_stageG];
                    if (clrcnt != -1) {
                        clrcnt++;
                    }
                    game_status.stClrCntGood[game_status.play_stageG] = clrcnt;
                } else {
                    clrcnt = game_status.stClrCntCool[game_status.play_stageG];
                    if (clrcnt != -1) {
                        clrcnt++;
                    }
                    game_status.stClrCntCool[game_status.play_stageG] = clrcnt;
                }

                if (game_status.endingFlag == ENDING_MOVIE) {
                    SpuBankSet();
                    UsrMemClear();
                    WipeInReq();
                    MtcWait(2);

                    selPlayDisp(STDAT_STAGE_ENDING, 0, FALSE);
                } else if (
                    game_status.endingFlag == ENDING_BONUS_1 ||
                    game_status.endingFlag == ENDING_BONUS_2 ||
                    game_status.endingFlag == ENDING_BONUS_3
                ) {
                    ingame_common_str.SingleScore = gply_pp->score;
                    ingame_common_str.bonusType   = (game_status.endingFlag - ENDING_BONUS_1);

                    SpuBankSet();
                    UsrMemClear();
                    WipeInReq();
                    MtcWait(2);

                    selPlayDisp(STDAT_STAGE_BONUS, 0, FALSE);

                    game_status.bonusG = ingame_common_str.BonusScore;
                }
            } else {
                menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
                game_status.scoreG[0] = 0;
                game_status.scoreG[1] = 0;
            }
        } else if (game_status.play_modeG == PLAY_MODE_VS_MAN) {
            if (!cancel_flag) {
                mccLocalGlobalCopy();

                gply_pp = &global_data.global_ply[0];

                game_status.scoreG[0] = gply_pp->vsScore;
                game_status.scoreG[1] = (gply_pp + 1)->vsScore;

                menu_str.sel_menu_enum = SEL_MENU_SAVE;
            } else {
                menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
                game_status.scoreG[0] = 0;
                game_status.scoreG[1] = 0;
            }
        } else if (game_status.play_modeG == PLAY_MODE_VS_COM) {
            gply_pp = &global_data.global_ply[0];

            if ((gply_pp->vsWin > gply_pp->vsLost) && !cancel_flag) {
                mccLocalGlobalCopy();

                game_status.scoreG[0] = gply_pp->vsScore;
                game_status.scoreG[1] = (gply_pp + 1)->vsScore;

                menu_str.sel_menu_enum = SEL_MENU_SAVE;

                clrcnt = game_status.stClrCntVs[game_status.play_stageG];
                if (clrcnt != -1) {
                    clrcnt++;
                }
                game_status.stClrCntVs[game_status.play_stageG] = clrcnt;
            } else {
                menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
                game_status.scoreG[0] = 0;
                game_status.scoreG[1] = 0;
            }
        }
    }

    return ret;
}

void titleDisp(int firstf) {
    STDAT_DAT *stdat_dat_pp;
    int        fsize, decp;
    int        loop;
    int        deramode;

    deramode = 0;
    loop = 0;

    while (1) {
        UsrMemClear();
        SpuBankSet();

        game_status.demo_flagG = DEMOF_OFF;
        GlobalLobcalCopy();

        stdat_dat_pp = stdat_rec[19].stdat_dat_pp;

        fsize = CdctrlGetFileSize(&stdat_dat_pp->intfile);
        fsize = ((fsize + 2047) / 2048) * 2048;

        CdctrlReadOne(&stdat_dat_pp->intfile, UsrMemEndAlloc(fsize), NULL);
        CdctrlReadWait();

        if (loop == 0) {
            selPlayDisp(0, 0, firstf);
            WipeInReqSame();
            SetBackColor(0xff, 0xff, 0xff);
        }
        MtcWait(2);

        decp = UsrMemAllocEndNext();
        UsrMemClearTop();
        UsrMemEndFree();
        CdctrlMemIntgDecode(decp, UsrMemAllocNext());

        game_status.demo_flagG = DEMOF_OFF;
        GlobalLobcalCopy();

        if (selPlayDispTitleDisp(19, deramode, loop)) {
            SetBackColor(0, 0, 0);
            break;
        }

        firstf = FALSE;
        SetBackColor(0, 0, 0);
        deramode ^= 1;

        WipeInReq();
        MtcWait(2);

        SpuBankSet();
        UsrMemClear();

        game_status.demo_flagG = DEMOF_DEMO;

        loop = gamePlayDisp();
        WipeInReq();
        MtcWait(2);
    }
}

int urawazaKeyCheck(void) {
    PADD *pad_pp;
    int   ret;
    int   change_tbl[17] = {
        TLL_LV01,   TLL_LV03, TLL_LV05, TLL_LV07,
        TLL_NORMAL, TLL_LV10, TLL_LV12, TLL_LV14,
        TLL_LV16,   TLL_LV15, TLL_LV13, TLL_LV11,
        TLL_LV09,   TLL_LV08, TLL_LV06, TLL_LV04,
        TLL_LV02,
    };

    pad_pp = &pad[0];

    /* R3 button */
    if (!(pad_pp->shot & SCE_PADj)) {
        ret = -1;
    } else {
        int   ud_d;
        float pos;

        /* With the right stick tilted, its direction picks one of 17 sectors; otherwise pick at random. */
        if (pad_pp->ana[PAD_ANA_RY] <  (128 - 64) || pad_pp->ana[PAD_ANA_RX] >= (64 + 128) ||
            pad_pp->ana[PAD_ANA_RY] >= (64 + 128) || pad_pp->ana[PAD_ANA_RX] <  (128 - 64)) {
            short rx = pad_pp->ana[PAD_ANA_RX] - 128;
            short ry = pad_pp->ana[PAD_ANA_RY] - 128;

            pos = atan2(-rx, ry);
            pos = (pos + PR_PI);
            pos = (pos * 17.0f) / (PR_PI*2);
            ud_d = pos;
        } else {
            ud_d = randMakeMax(17);
        }

        if (ud_d < 0) {
            ud_d = 0;
        }
        if (ud_d >= 17) {
            ud_d = 0;
        }

        ud_d = change_tbl[ud_d];
        printf("level fix:%d\n", ud_d);

        ret = ud_d;
    }

    return ret;
}

void ura_check(void) {
    u_char msg_tmp[32];
    int    ret;

    DbgMsgInit();

    while (1) {
        MtcWait(1);

        ret = urawazaKeyCheck();
        if (ret < 0) {
            sprintf(msg_tmp, "ura:NOUSE");
        } else {
            sprintf(msg_tmp, "ura:%d", ret);
        }

        DbgMsgClear();

        DbgMsgSetSize(22, 10);
        DbgMsgSetColor(128, 128, 128);
        DbgMsgPrint(msg_tmp, 1820, 1948);

        DbgMsgFlash();
    }
}

void mainStart(void *xx) {
    static int first_f = TRUE;

    mccReqInit();
    CdctrlInit();

    PrInitializeModule(DBufDc.draw01.zbuf1);
    UsrPrInitScene();

    hat_change_enum = HCNG_AUTO;

    TapCtInit();
    TapCt(TAPCT_INIT, 0, TAPCT_NONE);

    TimeCallbackSet();
    GlobalInit();
    UsrMemClear();
    SpuBankSetAll();

    menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
    menu_str.mc_rep_str_p = &mc_rep_str;
    menu_str.game_status_p = &game_status;

    cmnfTim2Trans();
    wipeSndFileTrans();

    CdctrlReadWait();
    printf("int read end\n");

    startUpDisp();

    while (1) {
        urawaza_levelsel_bottun = -1;
        titleDisp(first_f);

        WipeInReq();
        MtcWait(2);

        first_f = FALSE;

        menu_str.sel_menu_enum = SEL_MENU_STAGESEL;
        game_status.demo_flagG = DEMOF_OFF;

        while (1) {
            int retTitle;

            UsrMemClear();
            SpuBankSet();

            urawaza_levelsel_bottun = -1;
            urawaza_skip_bottun = FALSE;

            CdctrlRead(&file_str_menu_file, UsrMemAllocNext(), UsrMemAllocEndNext());
            CdctrlReadWait();

            while (!WipeEndCheck()) {
                MtcWait(1);
            }
            WipeOutReq();

            PrSetStage(0);
            game_status.play_typeG = PLAY_TYPE_NORMAL;

            retTitle = MenuCtrl(&menu_str);

            /*
             * Cheat code: L1 + L2
             *
             * Skip cutscenes + Boxy Boy practice.
             */
            if (pad[0].shot & SCE_PADL1 &&
                pad[0].shot & SCE_PADL2) {
                urawaza_skip_bottun = TRUE;
            } else {
                urawaza_skip_bottun = FALSE;
            }

            /*
             * Cheat code: R1 + R2
             *
             * Enable 'shuriken' mode.
             */
            if (pad[0].shot & SCE_PADR2 &&
                pad[0].shot & SCE_PADR1 &&
                game_status.play_modeG == PLAY_MODE_SINGLE) {
                game_status.play_typeG = PLAY_TYPE_ONE;
            } else {
                game_status.play_typeG = PLAY_TYPE_NORMAL;
            }

            /*
             * Cheat code: R3 + Right analog stick
             *
             * Change stage difficulty, can only be
             * used given the following conditions:
             *    - Singleplayer mode.
             *    - 'Shuriken' mode is disabled.
             *    - Max round/circuit (Yellow hat).
             *    - Current stage is not a demo.
             *    - Not in easy mode.
             */
            if (game_status.play_modeG == PLAY_MODE_SINGLE &&
                game_status.play_typeG == PLAY_TYPE_NORMAL &&
                game_status.roundG >= TRND_R4 &&
                game_status.demo_flagG == DEMOF_OFF &&
                game_status.play_table_modeG != PLAY_TABLE_EASY) {
                urawaza_levelsel_bottun = urawazaKeyCheck();
            } else {
                urawaza_levelsel_bottun = -1;
            }

            if (game_status.play_typeG == PLAY_TYPE_ONE) {
                if (game_status.endingFlag == ENDING_BONUS_1) {
                    game_status.endingFlag = ENDING_NONE;
                }
            
                if (game_status.endingFlag == ENDING_BONUS_2) {
                    game_status.endingFlag = ENDING_NONE;
                }
            
                if (game_status.endingFlag == ENDING_BONUS_3) {
                    game_status.endingFlag = ENDING_NONE;
                }
            }

            if (game_status.play_table_modeG == PLAY_TABLE_EASY) {
                if (game_status.endingFlag == ENDING_BONUS_1) {
                    game_status.endingFlag = ENDING_NONE;
                }
            
                if (game_status.endingFlag == ENDING_BONUS_2) {
                    game_status.endingFlag = ENDING_NONE;
                }
            
                if (game_status.endingFlag == ENDING_BONUS_3) {
                    game_status.endingFlag = ENDING_NONE;
                }
            }

            if (dbg_select_str.debug_on && !retTitle) {
                dbg_select_disp();
            }

            WipeInReq();
            UsrMemClear();
            SpuBankSet();

            if (retTitle) {
                break;
            }

            if (dbg_select_str.debug_on && dbg_select_str.non_play) {
                while (!WipeEndCheck()) {
                    MtcWait(1);
                }
                WipeOutReq();

                dummyPlay(retTitle);
            } else {
                gamePlayDisp();
            }

            WipeInReq();
            MtcWait(2);

            urawaza_levelsel_bottun = -1;
        }
    }
}
