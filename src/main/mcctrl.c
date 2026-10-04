#include "main/mcctrl.h"

#include "main/scrctrl.h"

#include "os/system.h"

#include <string.h>
#include <stdio.h>

static u_char ascii2sjiscng_tbl[] = {
    0x81, 0x40, /* "　" */
    0x81, 0x49, /* "！" */
    0x81, 0x68, /* "”" */
    0x81, 0x94, /* "＃" */
    0x82, 0x72, /* "Ｓ" */
    0x81, 0x93, /* "％" */
    0x81, 0x95, /* "＆" */
    0x81, 0x66, /* "’" */
    0x81, 0x69, /* "（" */
    0x81, 0x6A, /* "）" */
    0x81, 0x96, /* "＊" */
    0x81, 0x7B, /* "＋" */
    0x81, 0x43, /* "，" */
    0x81, 0x7C, /* "−" */
    0x81, 0x44, /* "．" */
    0x81, 0x5E, /* "／" */
    0x82, 0x4F, /* "０" */
    0x82, 0x50, /* "１" */
    0x82, 0x51, /* "２" */
    0x82, 0x52, /* "３" */
    0x82, 0x53, /* "４" */
    0x82, 0x54, /* "５" */
    0x82, 0x55, /* "６" */
    0x82, 0x56, /* "７" */
    0x82, 0x57, /* "８" */
    0x82, 0x58, /* "９" */
    0x81, 0x46, /* "：" */
    0x81, 0x47, /* "；" */
    0x81, 0x83, /* "＜" */
    0x81, 0x81, /* "＝" */
    0x81, 0x84, /* "＞" */
    0x81, 0x48, /* "？" */
    0x81, 0x97, /* "＠" */
    0x82, 0x60, /* "Ａ" */
    0x82, 0x61, /* "Ｂ" */
    0x82, 0x62, /* "Ｃ" */
    0x82, 0x63, /* "Ｄ" */
    0x82, 0x64, /* "Ｅ" */
    0x82, 0x65, /* "Ｆ" */
    0x82, 0x66, /* "Ｇ" */
    0x82, 0x67, /* "Ｈ" */
    0x82, 0x68, /* "Ｉ" */
    0x82, 0x69, /* "Ｊ" */
    0x82, 0x6A, /* "Ｋ" */
    0x82, 0x6B, /* "Ｌ" */
    0x82, 0x6C, /* "Ｍ" */
    0x82, 0x6D, /* "Ｎ" */
    0x82, 0x6E, /* "Ｏ" */
    0x82, 0x6F, /* "Ｐ" */
    0x82, 0x70, /* "Ｑ" */
    0x82, 0x71, /* "Ｒ" */
    0x82, 0x72, /* "Ｓ" */
    0x82, 0x73, /* "Ｔ" */
    0x82, 0x74, /* "Ｕ" */
    0x82, 0x75, /* "Ｖ" */
    0x82, 0x76, /* "Ｗ" */
    0x82, 0x77, /* "Ｘ" */
    0x82, 0x78, /* "Ｙ" */
    0x82, 0x79, /* "Ｚ" */
    0x81, 0x6D, /* "［" */
    0x81, 0x8F, /* "￥" */
    0x81, 0x6E, /* "］" */
    0x81, 0x4F, /* "＾" */
    0x81, 0x51, /* "＿" */
    0x81, 0x65, /* "‘" */
    0x82, 0x81, /* "ａ" */
    0x82, 0x82, /* "ｂ" */
    0x82, 0x83, /* "ｃ" */
    0x82, 0x84, /* "ｄ" */
    0x82, 0x85, /* "ｅ" */
    0x82, 0x86, /* "ｆ" */
    0x82, 0x87, /* "ｇ" */
    0x82, 0x88, /* "ｈ" */
    0x82, 0x89, /* "ｉ" */
    0x82, 0x8A, /* "ｊ" */
    0x82, 0x8B, /* "ｋ" */
    0x82, 0x8C, /* "ｌ" */
    0x82, 0x8D, /* "ｍ" */
    0x82, 0x8E, /* "ｎ" */
    0x82, 0x8F, /* "ｏ" */
    0x82, 0x90, /* "ｐ" */
    0x82, 0x91, /* "ｑ" */
    0x82, 0x92, /* "ｒ" */
    0x82, 0x93, /* "ｓ" */
    0x82, 0x94, /* "ｔ" */
    0x82, 0x95, /* "ｕ" */
    0x82, 0x96, /* "ｖ" */
    0x82, 0x97, /* "ｗ" */
    0x82, 0x98, /* "ｘ" */
    0x82, 0x99, /* "ｙ" */
    0x82, 0x9A, /* "ｚ" */
    0x81, 0x6F, /* "｛" */
    0x81, 0x62, /* "｜" */
    0x81, 0x70, /* "｝" */
    0x81, 0x60, /* "〜" */
    0x81, 0x40, /* "　" */
};

static MC_REP_STR mc_rep_str_local;

static u_char mc_holdTmp[4];
static u_char mc_resetTmp[4];

static MC_REP_CTRL mc_rep_ctrl;

int setAscii2SjisCode(u_char *saki_pp, u_char *moto_pp) {
    int    i;
    u_char dat_tmp;

    *saki_pp = '\0';

    for (i = 0; i < strlen(moto_pp); i++) {
        dat_tmp = moto_pp[i];

        if (dat_tmp < 0x20 || dat_tmp > 0x7f) {
            printf("dat over!!\n");
            return 1;
        }

        *saki_pp++ = ascii2sjiscng_tbl[(dat_tmp - 32) * 2];
        *saki_pp++ = ascii2sjiscng_tbl[(dat_tmp - 32) * 2 + 1];
    }

    *saki_pp = '\0';
    return 0;
}

void mccReqInit(void) {
    WorkClear(&mc_rep_str_local, sizeof(mc_rep_str_local));
    WorkClear(mc_holdTmp, sizeof(mc_holdTmp));
    WorkClear(mc_resetTmp, sizeof(mc_resetTmp));
}

void mccReqSaveInit(void) {
    mc_rep_str_local.scoreN_cnt = 0;
    mc_rep_str_local.levelN_cnt = 0;
    mc_rep_str_local.mc_rep_dat_cnt = 0;
    mc_rep_str_local.mc_vsoth_cnt = 0;
}

void mccReqCtrlClr(void) {
    WorkClear(&mc_rep_ctrl, sizeof(mc_rep_ctrl));
}

void mccReqScrSet(MC_REP_SCR *mcr_scr_pp) {
    if (mc_rep_str_local.scoreN_cnt >= 256) {
        printf("score save over!!\n");
        return;
    }

    mc_rep_str_local.mc_rep_scr[mc_rep_str_local.scoreN_cnt] = *mcr_scr_pp;
    mc_rep_str_local.scoreN_cnt++;
}

MC_REP_SCR* mccReqScrGet(void) {
    MC_REP_SCR *ret;

    if (mc_rep_ctrl.cl_scoreN_cnt >= 256 ||
        mc_rep_ctrl.cl_scoreN_cnt >= mc_rep_str_local.scoreN_cnt) {
        printf("score load over!!\n");
        return NULL;
    }

    ret = &mc_rep_str_local.mc_rep_scr[mc_rep_ctrl.cl_scoreN_cnt];
    mc_rep_ctrl.cl_scoreN_cnt++;
    return ret;
}

void mccReqLvlSet(u_int lvl) {
    if (mc_rep_str_local.levelN_cnt >= 256) {
        printf("level save over!!\n");
        return;
    }

    mc_rep_str_local.levelN[mc_rep_str_local.levelN_cnt] = lvl;
    mc_rep_str_local.levelN_cnt++;
}

u_int mccReqLvlGet(void) {
    u_int ret;

    if (mc_rep_ctrl.cl_levelN_cnt >= 256 ||
        mc_rep_ctrl.cl_levelN_cnt >= mc_rep_str_local.levelN_cnt) {
        printf("level load over!!\n");
        return 0;
    }

    ret = mc_rep_str_local.levelN[mc_rep_ctrl.cl_levelN_cnt];
    mc_rep_ctrl.cl_levelN_cnt++;
    return ret;
}

void mccReqTapSet(u_int time, u_int useLine, u_int id, PLAYER_ENUM ply) {
    MC_REP_DAT *mcrd_pp;

    if (mc_rep_str_local.mc_rep_dat_cnt >= 2560) {
        printf("tap save over!!\n");
        return;
    }

    mcrd_pp = &mc_rep_str_local.mc_rep_dat[mc_rep_str_local.mc_rep_dat_cnt];

    mcrd_pp->timeP = time;
    mcrd_pp->padId = id;
    mcrd_pp->useL = useLine;
    mcrd_pp->ply = ply;

    mcrd_pp->resT = 0;
    mcrd_pp->holdT = 0;

    if (mc_resetTmp[ply] != 0) {
        mcrd_pp->resT = 1;
    }
    if (mc_holdTmp[ply] != 0) {
        mcrd_pp->holdT = 1;
    }

    mc_resetTmp[ply] = 0;
    mc_holdTmp[ply] = 0;

    mc_rep_str_local.mc_rep_dat_cnt++;
}

void mccReqTapResetSet(PLAYER_ENUM ply) {
    mc_resetTmp[ply] = 1;
}

void mccReqTapHoldSet(PLAYER_ENUM ply) {
    mc_holdTmp[ply] = 1;
}

void mccReqVSOTHSAVEset(VSOTHSAVE *sv) {
    int i;

    if (mc_rep_str_local.mc_vsoth_cnt >= 100) {
        printf("vs oth save over!!\n");
        return;
    }

    for (i = 0; i < PR_SIZEOF(VSOTHSAVE); i++) {
        mc_rep_str_local.vsothsave[mc_rep_str_local.mc_vsoth_cnt][i] = (*sv)[i];
    }

    mc_rep_str_local.mc_vsoth_cnt++;
}

int mccReqVSOTHSAVEget(VSOTHSAVE *sv) {
    int i;

    if (mc_rep_ctrl.cl_vsoth_cnt >= 100 ||
        mc_rep_ctrl.cl_vsoth_cnt >= mc_rep_str_local.mc_vsoth_cnt) {
        printf("vs oth load over!!\n");
        return 0;
    }
    
    for (i = 0; i < PR_SIZEOF(VSOTHSAVE); i++) {
        (*sv)[i] = mc_rep_str_local.vsothsave[mc_rep_ctrl.cl_vsoth_cnt][i];
    }

    mc_rep_ctrl.cl_vsoth_cnt++;
    return 1;
}

u_short mccReqTapGet(u_int time, u_int useLine, u_int *time_pp, PLAYER_ENUM ply) {
    u_short     ret;
    MC_REP_DAT *mcrd_pp;
    u_int      *rep_cnt;

    rep_cnt = &mc_rep_ctrl.cl_mc_rep_dat_cnt[ply];

    while (1) {
        if (*rep_cnt >= 2560 || *rep_cnt >= mc_rep_str_local.mc_rep_dat_cnt) {
            return 0;
        }

        mcrd_pp = &mc_rep_str_local.mc_rep_dat[*rep_cnt];

        if (mcrd_pp->ply == ply) {
            break;
        }

        (*rep_cnt)++;
    }

    if (mcrd_pp->timeP > time) {
        return 0;
    }
    if (mcrd_pp->useL != useLine) {
        return 0;
    }

    ret = GetIndex2KeyCode(mcrd_pp->padId);

    if (mcrd_pp->holdT) {
        ret |= KC_FLAG_HOLD;
    }
    if (mcrd_pp->resT) {
        ret |= KC_FLAG_RESET;
    }

    *time_pp = mcrd_pp->timeP;
    (*rep_cnt)++;
        
    return ret;
}

void mccReqTapForward(u_int time, u_int useLine) {
    MC_REP_DAT *mcrd_pp;
    int         i;
    u_int      *rep_cnt;

    rep_cnt = mc_rep_ctrl.cl_mc_rep_dat_cnt;

    for (i = 0; i < 4; i++, rep_cnt++) {
        while (1) {
            if (*rep_cnt >= 2560 || *rep_cnt >= mc_rep_str_local.mc_rep_dat_cnt) {
                return;
            }

            mcrd_pp = &mc_rep_str_local.mc_rep_dat[*rep_cnt];

            /* note: Single line conditions needed to match! */
            if (mcrd_pp->timeP > time) break;
            if (mcrd_pp->useL != useLine) break;
            if (mcrd_pp->ply != i) break;

            (*rep_cnt)++;
            printf("TAP forward!!\n");
        }
    }
}

void mccReqTapForwardOwn(u_int time, u_int useLine, int ply) {
    MC_REP_DAT *mcrd_pp;
    u_int      *rep_cnt;

    rep_cnt = &mc_rep_ctrl.cl_mc_rep_dat_cnt[ply];

    while (1) {
        if (*rep_cnt >= 2560 || *rep_cnt >= mc_rep_str_local.mc_rep_dat_cnt) {
            break;
        }

        mcrd_pp = &mc_rep_str_local.mc_rep_dat[*rep_cnt];

        /* note: Single line conditions needed to match! */
        if (mcrd_pp->timeP > time) break;
        if (mcrd_pp->useL != useLine) break;
        if (mcrd_pp->ply != ply) break;

        (*rep_cnt)++;
        printf("TAP forward!!\n");
    }
}

void mccLocalGlobalCopy(void) {
    mc_rep_str = mc_rep_str_local;

    mc_rep_str.play_modeS       = game_status.play_modeG;
    mc_rep_str.play_typeS       = game_status.play_typeG;
    mc_rep_str.roundS           = game_status.roundG;
    mc_rep_str.play_stageS      = game_status.play_stageG;
    mc_rep_str.play_table_modeS = game_status.play_table_modeG;
}

void mccGlobalLocalCopy(void) {
    mc_rep_str_local = mc_rep_str;
}
