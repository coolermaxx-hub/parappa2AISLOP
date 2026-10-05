#include "main/etc.h"

#include "main/mbar.h"

#include "os/syssub.h"
#include "os/system.h"

#include "iop_mdl/tapctrl_rpc.h"

#include <prlib/prlib.h>

#include <libpad.h>

#include <stdio.h>

void GlobalInit(void) {
    WorkClear(&game_status, sizeof(game_status));
    WorkClear(&global_data, sizeof(global_data));

    game_status.play_modeG       = PLAY_MODE_SINGLE;
    game_status.play_typeG       = PLAY_TYPE_NORMAL;
    game_status.roundG           = TRND_R1;

    game_status.play_table_modeG = PLAY_TABLE_NORMAL;
    game_status.play_stageG      = P3_STAGE_1;
    game_status.demo_flagG       = DEMOF_OFF;

    game_status.language_type    = LANG_JAPANESE;
    game_status.vibration        = VIBRATION_ON;
    game_status.subtitle         = SUBTITLE_ON;

    game_status.level_vs_enumG   = LVS_1;
}

P3_STAGE_ENUM clearStageCheck(void) {
    int           clrbit, i;
    P3_STAGE_ENUM ret;

    ret    = P3_STAGE_0;
    clrbit = game_status.stClearBit;

    for (i = 1; i < 9; i++) {
        if (!(clrbit & 1)) {
            break;
        }

        ret++;
        clrbit >>= 1;
    }

    return ret;
}

void GlobalTimeInit(GLOBAL_DATA *gl_pp) {
    gl_pp->currentTime     = 0;
    gl_pp->cdTime          = 0;
    gl_pp->vsyncTime       = 0;
    gl_pp->Snd_currentTime = 0;
    gl_pp->Snd_cdTime      = 0;
    gl_pp->Snd_vsyncTime   = 0;
}

void GlobalSetTempo(GLOBAL_DATA *gl_pp, float tempo) {
    gl_pp->tempo = tempo;
}

void GlobalTimeJobChange(TIME_GET_FLAG tfg) {
    global_data.TimeType = tfg;
    printf("time job change!!![%x]\n", tfg);
}

void GlobalTimeJob(void) {
    if (global_data.TimeType == FGF_VSYNC) {
        global_data.vsyncTime = TimeCallbackTimeGet();

        global_data.currentTime = global_data.vsyncTime;
        global_data.Snd_vsyncTime = (((global_data.currentTime * 96.0f * global_data.tempo) + 1800.0f) / 3600.0f);
        global_data.Snd_currentTime = global_data.Snd_vsyncTime;

        global_data.Snd_cdSampleCnt = CdctrlSndTime2WP2sample(global_data.tempo, global_data.Snd_vsyncTime);
    } else if (global_data.TimeType == FGF_CD) {
        CdctrlWp2GetSampleTmpBuf();

        global_data.Snd_cdTime = CdctrlWp2GetSndTimeTmp(global_data.tempo);

        global_data.Snd_currentTime = global_data.Snd_cdTime;
        global_data.cdTime = ((global_data.Snd_currentTime * 3600.0f + global_data.tempo * 96.0f * 0.5f) / (global_data.tempo * 96.0f));
        global_data.currentTime = global_data.cdTime;

        global_data.Snd_cdSampleCnt = CdctrlWp2GetSampleTmp();
    }
}

int GlobalTimeGet(void) {
    return global_data.currentTime;
}

int GlobalSndTimeGet(void) {
    return global_data.Snd_currentTime;
}

int GlobalSndSampleGet(void) {
    return global_data.Snd_cdSampleCnt;
}

TAP_ROUND_ENUM GetHatRound(void) {
    TAP_ROUND_ENUM ret = global_data.roundL;

    if (hat_change_enum != HCNG_AUTO) {
        return (TAP_ROUND_ENUM)hat_change_enum;
    }

    return ret;
}

int GlobalMendererUseCheck(void) {
    return global_data.play_step == PSTEP_GAME || global_data.play_step == PSTEP_SERIAL;
}

void GlobalLobcalCopy(void) {
    switch (game_status.demo_flagG) {
    case DEMOF_OFF:
        global_data.play_modeL       = game_status.play_modeG;
        global_data.play_typeL       = game_status.play_typeG;
        global_data.roundL           = game_status.roundG;
        global_data.play_stageL      = game_status.play_stageG;
        global_data.play_table_modeL = game_status.play_table_modeG;
        
        global_data.demo_flagL       = DEMOF_OFF;
        global_data.tapLevelCtrl     = LM_AUTO;

        global_data.level_vs_enumL   = game_status.level_vs_enumG;
        break;
    case DEMOF_DEMO: {
        int demo_rnd = clearStageCheck();

        if (demo_rnd < 5) {
            demo_rnd = 5;
        }
        if (demo_rnd > 8) {
            demo_rnd = 8;
        }

        global_data.play_modeL       = PLAY_MODE_SINGLE;
        global_data.play_typeL       = game_status.play_typeG;
        global_data.roundL           = game_status.roundG;
        global_data.play_stageL      = randMakeMax(demo_rnd) + 1;
        global_data.play_table_modeL = PLAY_TABLE_NORMAL;

        global_data.demo_flagL       = DEMOF_DEMO;
        global_data.tapLevelCtrl     = LM_AUTO;
        break;
    }
    case DEMOF_REPLAY:
    default:
        global_data.play_modeL       = mc_rep_str.play_modeS;
        global_data.play_typeL       = mc_rep_str.play_typeS;
        global_data.roundL           = mc_rep_str.roundS;
        global_data.play_stageL      = mc_rep_str.play_stageS;
        
        global_data.demo_flagL       = DEMOF_REPLAY;
        global_data.play_table_modeL = mc_rep_str.play_table_modeS;
        global_data.tapLevelCtrl     = LM_AUTO;
        
        global_data.level_vs_enumL   = mc_rep_str.level_vs_enumS;
        break;
    }
}

void GlobalPlySet(GLOBAL_DATA *gl_pp, PLAY_STEP stp, int stage_num) {
    GLOBAL_PLY *gply_pp;

    ExamDispInit();
    WorkClear(&gl_pp->global_ply, sizeof(gl_pp->global_ply));

    global_data.tap_ctrl_level = TCT_START;
    global_data.play_step = stp;

    if (global_data.demo_flagL != DEMOF_REPLAY) {
        mccReqSaveInit();
    }

    switch (stp) {
    case PSTEP_SERIAL:
        global_data.draw_tbl_top = DTBL_ENUM_SERIAL_TOP;
        
        gply_pp = &global_data.global_ply[3];
        gply_pp->player_code = PCODE_NONE;
        gply_pp->rank_level  = RLVL_GOOD;
        gply_pp->pad_type    = PAD_DEMO;
        gply_pp->now_score   = 0;
        gply_pp->score       = 0;
        gply_pp->tap_lvl_tmp = 0;
        break;
    case PSTEP_XTR:
        global_data.draw_tbl_top = DTBL_ENUM_SERIAL_TOP;
        
        gply_pp = &global_data.global_ply[3];
        gply_pp->player_code = PCODE_NONE;
        gply_pp->rank_level  = RLVL_GOOD;
        gply_pp->pad_type    = PAD_DEMO;
        gply_pp->now_score   = 0;
        gply_pp->score       = 0;
        gply_pp->tap_lvl_tmp = 0;
        break;
    case PSTEP_HOOK:
        global_data.draw_tbl_top = inCmnHookSet(stage_num);
        
        gply_pp = &global_data.global_ply[1];
        gply_pp->player_code  = PCODE_TEACHER;
        gply_pp->rank_level   = RLVL_GOOD;
        gply_pp->pad_type     = PAD_DEMO;
        gply_pp->now_score    = 0;
        gply_pp->score        = 0;
        gply_pp->tap_lvl_tmp  = 0;
        
        gply_pp = &global_data.global_ply[0];
        gply_pp->player_code  = PCODE_PARA;
        gply_pp->rank_level   = RLVL_GOOD;

        if (global_data.demo_flagL == DEMOF_DEMO) {
            gply_pp->pad_type = PAD_DEMO;
        } else if (global_data.demo_flagL == DEMOF_OFF) {
            gply_pp->pad_type = PAD_1CON;
        } else {
            gply_pp->pad_type = PAD_REPLAY;
        }

        gply_pp->now_score    = 0;
        gply_pp->score        = 0;
        gply_pp->tap_lvl_tmp  = 0;
        ExamDispPlySet(gply_pp, 0);
        break;
    case PSTEP_GAME:
        global_data.draw_tbl_top = DTBL_ENUM_GAME_TOP;
        
        gply_pp = &global_data.global_ply[2];
        gply_pp->player_code = PCODE_BOXY;
        gply_pp->rank_level  = RLVL_GOOD;
        gply_pp->pad_type    = PAD_DEMO;
        gply_pp->now_score   = 0;
        gply_pp->score       = 0;
        gply_pp->tap_lvl_tmp = 0;
        
        gply_pp = &global_data.global_ply[1];
        gply_pp->player_code = PCODE_TEACHER;
        gply_pp->rank_level  = RLVL_GOOD;
        gply_pp->pad_type    = PAD_DEMO;
        gply_pp->now_score   = 0;
        gply_pp->score       = 0;
        gply_pp->tap_lvl_tmp = 0;
        
        gply_pp = &global_data.global_ply[0];
        gply_pp->player_code  = PCODE_PARA;
        gply_pp->rank_level   = RLVL_GOOD;

        if (global_data.demo_flagL == DEMOF_DEMO) {
            gply_pp->pad_type = PAD_DEMO;
        } else if (global_data.demo_flagL == DEMOF_OFF) {
            gply_pp->pad_type = PAD_1CON;
        } else {
            gply_pp->pad_type = PAD_REPLAY;
        }

        gply_pp->now_score    = 0;
        gply_pp->score        = 0;
        gply_pp->tap_lvl_tmp  = 0;
        gply_pp->flags        = GPLAY_TBLCNG_REQ;
        ExamDispPlySet(gply_pp, 0);
        break;
    case PSTEP_BONUS:
        global_data.draw_tbl_top = DTBL_ENUM_BN_TOP;
        
        gply_pp = &global_data.global_ply[1];
        gply_pp->player_code = PCODE_TEACHER;
        gply_pp->rank_level  = RLVL_GOOD;
        gply_pp->pad_type    = PAD_UNUSE;
        gply_pp->now_score   = 0;
        gply_pp->score       = 0;
        gply_pp->tap_lvl_tmp = 0;

        gply_pp = &global_data.global_ply[0];
        gply_pp->player_code = PCODE_PARA;
        gply_pp->rank_level  = RLVL_GOOD;
        gply_pp->pad_type    = PAD_UNUSE;
        gply_pp->now_score   = 0;
        gply_pp->score       = 0;
        gply_pp->tap_lvl_tmp = 0;
        ExamDispPlySet(gply_pp, 0);
        break;
    case PSTEP_VS:
        global_data.draw_tbl_top = DTBL_ENUM_VS_TOP;
        
        gply_pp = &global_data.global_ply[2];
        gply_pp->player_code = PCODE_BOXY;
        gply_pp->rank_level  = RLVL_GOOD;
        gply_pp->pad_type    = PAD_DEMO;
        gply_pp->now_score   = 0;
        gply_pp->score       = 0;
        gply_pp->tap_lvl_tmp = 0;
        
        gply_pp->vsDraw  = 0;
        gply_pp->vsWin   = 0;
        gply_pp->vsLost  = 0;
        gply_pp->vsScore = 0;
        
        gply_pp = &global_data.global_ply[1];
        gply_pp->player_code = PCODE_TEACHER;
        gply_pp->rank_level  = RLVL_GOOD;

        if (global_data.demo_flagL == DEMOF_REPLAY) {
            gply_pp->pad_type = PAD_REPLAY;
            printf("2p replay\n");
        } else if (global_data.demo_flagL == DEMOF_DEMO) {
            gply_pp->pad_type = PAD_DEMO;
            printf("2p demo\n");
        } else if (global_data.play_modeL == PLAY_MODE_VS_COM) {
            gply_pp->pad_type = PAD_COM;
            printf("2p com\n");
            global_data.tap_ctrl_level = global_data.level_vs_enumL + 1;
        } else {
            gply_pp->pad_type = PAD_2CON;
            printf("2p 2con\n");
            global_data.tap_ctrl_level = TCT_LV00;            
        }

        gply_pp->now_score   = 0;
        gply_pp->score       = 0;
        gply_pp->tap_lvl_tmp = 0;
        
        gply_pp->vsDraw  = 0;
        gply_pp->vsWin   = 0;
        gply_pp->vsLost  = 0;
        gply_pp->vsScore = 0;
        ExamDispPlySet(gply_pp, 1);
        
        gply_pp = &global_data.global_ply[0];
        gply_pp->player_code  = PCODE_PARA;
        gply_pp->rank_level   = RLVL_GOOD;

        if (global_data.demo_flagL == DEMOF_DEMO) {
            gply_pp->pad_type = PAD_DEMO;
        } else if (global_data.demo_flagL == DEMOF_OFF) {
            gply_pp->pad_type = PAD_1CON;
        } else {
            gply_pp->pad_type = PAD_REPLAY;
        }

        gply_pp->now_score    = 0;
        gply_pp->score        = 0;
        gply_pp->tap_lvl_tmp  = 0;
        
        gply_pp->vsDraw  = 0;
        gply_pp->vsWin   = 0;
        gply_pp->vsLost  = 0;
        gply_pp->vsScore = 0;
        ExamDispPlySet(gply_pp, 0);
        break;
    default:
        printf("error GlobalPlySet PLAY_STEP [%d]\n", stp);
        break;
    }
}

PAD_TYPE GetPcode2PadType(PLAYER_CODE player_code) {
    int         i;
    PAD_TYPE    ret;
    GLOBAL_PLY *gl_pp;

    ret = PAD_UNUSE;

    for (i = 0; i < PR_ARRAYSIZE(global_data.global_ply); i++) {
        gl_pp = &global_data.global_ply[i];
        if (gl_pp->player_code & player_code) {
            ret = gl_pp->pad_type;
            break;
        }
    }

    return ret;
}

static u_int vsync_time[TCBK_CHANNEL_MAX];

static int TimeCallback(int x) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(vsync_time); i++) {
        vsync_time[i]++;
    }

    return 1;
}

void TimeCallbackSet(void) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(vsync_time); i++) {
        vsync_time[i] = 0;
    }

    sceGsSyncVCallback(TimeCallback);
}

u_int TimeCallbackTimeGetChan(int chan) {
    if (chan >= TCBK_CHANNEL_MAX) {
        printf("Time Callback Channel ERROR!!\n");
        return 0;
    }

    return vsync_time[chan];
}

void TimeCallbackTimeSetChan(int chan, u_int time) {
    if (chan >= TCBK_CHANNEL_MAX) {
        printf("Time Callback Channel ERROR!!\n");
        return;
    }

    vsync_time[chan] = time;
}

void TimeCallbackTimeSetChanTempo(int chan, u_int time, float tempo) {
    if (chan >= TCBK_CHANNEL_MAX) {
        printf("Time Callback Channel ERROR!!\n");
        return;
    }

    vsync_time[chan] = (int)
    (
        ((time * 3600.0f) + (tempo * 96.0f * 0.5f)) / (tempo * 96.0f)
    );
}

u_int TimeCallbackTimeGet(void) {
    return TimeCallbackTimeGetChan(0);
}

void TimeCallbackTimeSet(u_int time) {
    TimeCallbackTimeSetChan(0, time);
}

PLAYER_INDEX Pcode2Pindex(PLAYER_CODE pc) {
    int i;

    if (pc == 0) {
        return 0;
    }

    for (i = 0; i < 32; i++) {
        if (((int)pc >> i) & 0x1) {
            return i;
        }
    }

    return 0;
}

int GetKeyCode2Index(int code) {
    if (code & KcTR) {
        return KiTR;
    }
    if (code & KcCI) {
        return KiCI;
    }
    if (code & KcXX) {
        return KiXX;
    }
    if (code & KcSQ) {
        return KiSQ;
    }

    if (code & KcL1) {
        return KiL1;
    }
    if (code & KcR1) {
        return KiR1;
    }

    if (code & KcL2) {
        return KiL1;
    }
    if (code & KcR2) {
        return KiR1;
    }

    return KiNO;
}

int GetIndex2KeyCode(int index) {
    static int KeyCode[7] = {
        0, KcTR, KcCI, KcXX, KcSQ, KcL1, KcR1,
    };

    if (index > 0 && index < PR_ARRAYSIZEU(KeyCode)) {
        return KeyCode[index];
    }

    return 0;
}

int GetIndex2PressId(int index) {
    static int KeyPressNum[7] = {
        PAD_PR_None, PAD_PR_Rup, PAD_PR_Rright, PAD_PR_Rdown, PAD_PR_Rleft, PAD_PR_L1, PAD_PR_R1,
    };

    if (index > PR_ARRAYSIZEU(KeyPressNum)) {
        return PAD_PR_None;
    }

    return KeyPressNum[index];
}

int GetKeyCode2PressId(int code) {
    if (code & KcTR) {
        return PAD_PR_Rup;
    }
    if (code & KcCI) {
        return PAD_PR_Rright;
    }
    if (code & KcXX) {
        return PAD_PR_Rdown;
    }
    if (code & KcSQ) {
        return PAD_PR_Rleft;
    }

    if (code & KcL1) {
        return PAD_PR_L1;
    }
    if (code & KcR1) {
        return PAD_PR_R1;
    }

    return PAD_PR_None;
}

TAP_LINE_LEVEL_ENUM ChangeTapLevel(TAP_LINE_LEVEL_ENUM now_lvl) {
    return now_lvl;
}

static PR_SCENEHANDLE usrSceneHandle = NULL;

void UsrPrInitScene(void) {
    /* Empty */
}

void UsrPrQuitScene(void) {
    /* Empty */
}

void UsrPrSetScene(void) {
    usrSceneHandle = PrInitializeScene(&DBufDc.draw01, "safe scene", FBP_VRAM_DRAW2);

    PrPreprocessSceneModel(usrSceneHandle);
    PrSetSceneEnv(usrSceneHandle, DrawGetDrawEnvP(DNUM_DRAW));

    PrRender(usrSceneHandle);
    PrWaitRender();

    PrCleanupScene(usrSceneHandle);
    usrSceneHandle = NULL;
}

static SCR_SND_AREA scr_snd_area[] = {
    { .spu_size = 0x120000, .spu_adrs = 0x5010,   .iop_size = 0x3000 },
    { .spu_size = 0x50000,  .spu_adrs = 0x125010, .iop_size = 0x3000 },
    { .spu_size = 0x50000,  .spu_adrs = 0x175010, .iop_size = 0x3000 },
    { .spu_size = 0x36700,  .spu_adrs = 0x1C5010, .iop_size = 0x3000 },
};

void SpuBankSet(void) {
    int i;

    for (i = 0; i < PR_ARRAYSIZEU(scr_snd_area) - 1; i++) {
        TapCt(TAPCT_ALLOCSPU | i, scr_snd_area[i].spu_adrs, TAPCT_NONE);
        TapCt(TAPCT_ALLOCIOP | i, scr_snd_area[i].iop_size, TAPCT_NONE);
    }
}

void SpuBankSetAll(void) {
    int i;

    for (i = 0; i < PR_ARRAYSIZEU(scr_snd_area); i++) {
        TapCt(TAPCT_ALLOCSPU | i, scr_snd_area[i].spu_adrs, TAPCT_NONE);
        TapCt(TAPCT_ALLOCIOP | i, scr_snd_area[i].iop_size, TAPCT_NONE);
    }
}

void inCmnInit(int stg) {
    WorkClear(&ingame_common_str, sizeof(ingame_common_str));
    ingame_common_str.BonusStage = stg;
}

static u_char hkl_pknum_01[] = {
    HKLV_SNDREC_12, HKLV_SNDREC_13, HKLV_SNDREC_14,
};

static u_char hkl_pknum_02[] = {
    HKLV_SNDREC_14, HKLV_SNDREC_15, HKLV_SNDREC_16,
};

static u_char hkl_pknum_03[] = {
    HKLV_SNDREC_06, HKLV_SNDREC_07, HKLV_SNDREC_08,
};

static u_char hkl_pknum_04[] = {
    HKLV_SNDREC_11, HKLV_SNDREC_12, HKLV_SNDREC_13,
};

static u_char hkl_pknum_05[] = {
    HKLV_SNDREC_16, HKLV_SNDREC_17,
};

static u_char hkl_pknum_06[] = {
    HKLV_SNDREC_20,
};

static u_char hkl_pknum_07[] = {
    HKLV_SNDREC_12,
};

static HKL_PKSTR hkl_pkstr[] = {
    { .cnt = 0,                          .hknum_pp = NULL         },
    { .cnt = PR_ARRAYSIZE(hkl_pknum_01), .hknum_pp = hkl_pknum_01 },
    { .cnt = PR_ARRAYSIZE(hkl_pknum_02), .hknum_pp = hkl_pknum_02 },
    { .cnt = PR_ARRAYSIZE(hkl_pknum_03), .hknum_pp = hkl_pknum_03 },
    { .cnt = PR_ARRAYSIZE(hkl_pknum_04), .hknum_pp = hkl_pknum_04 },
    { .cnt = PR_ARRAYSIZE(hkl_pknum_05), .hknum_pp = hkl_pknum_05 },
    { .cnt = PR_ARRAYSIZE(hkl_pknum_06), .hknum_pp = hkl_pknum_06 },
    { .cnt = PR_ARRAYSIZE(hkl_pknum_07), .hknum_pp = hkl_pknum_07 },
    { .cnt = 0,                          .hknum_pp = NULL         },
};

int inCmnHookMaxLineCnt(int stg) {
    if (stg >= PR_ARRAYSIZEU(hkl_pkstr)) {
        return 0;
    }
    return hkl_pkstr[stg].cnt;
}

HKLV_SNDREC_ENUM inCmnHookMaxLinePknum(int stg, int line) {
    if (stg >= PR_ARRAYSIZEU(hkl_pkstr) || line >= hkl_pkstr[stg].cnt) {
        return 0;
    }

    return hkl_pkstr[stg].hknum_pp[line];
}

int inCmnHookSet(int stg) {
    int cnt, ret;

    ret = DTBL_ENUM_HOOK_TOP;
    cnt = inCmnHookMaxLineCnt(stg);

    if (cnt == 0) {
        ingame_common_str.HookLine = HKLV_SNDREC_NOUSE;
    } else {
        ret = randMakeMax(cnt);
        ingame_common_str.HookLine = inCmnHookMaxLinePknum(stg, ret);
    }

    printf("HOOK pack:%d line:%d\n", ingame_common_str.HookLine, ret);
    return ret;
}

int inCmnHook2GameCheck(int pack_id) {
    int ret = -1;

    if (ingame_common_str.HookLine == HKLV_SNDREC_NOUSE) {
        return -1;
    }

    if (ingame_common_str.HookLine == pack_id) {
        ret = ingame_common_str.HookLevel;
        printf("HOOK level use:%d  pack:%d\n", ret, pack_id);
    }

    return ret;
}

void inCmnHook2GameSave(int level) {
    ingame_common_str.HookLevel = level;
    printf("HOOK pack:%d level:%d\n", ingame_common_str.HookLine, level);
}
