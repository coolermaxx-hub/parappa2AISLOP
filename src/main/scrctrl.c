#include "main/scrctrl.h"

#include "dbug/dbgmsg.h"

#include "os/cmngifpk.h"
#include "os/mtc.h"
#include "os/syssub.h"
#include "os/system.h"
#include "os/tim2.h"
#include "os/usrmem.h"

#include "main/commake.h"
#include "main/main.h"
#include "main/mbar.h"
#include "main/mcctrl.h"
#include "main/p3str.h"
#include "main/sprite.h"

#include "iop_mdl/tapctrl_rpc.h"

#include <prlib/prlib.h>

#include <libpad.h>

#include <stdio.h>
#include <stdlib.h>

static void exam_tbl_updownInit(SCORE_INDV_STR *sindv_pp);
static void exam_tbl_updownSet(SCORE_INDV_STR *sindv_pp, int now, int sikiichi, int oth);
static int  exam_tbl_updownChange(SCORE_INDV_STR *sindv_pp, TAP_CTRL_LEVEL_ENUM clv, TAP_ROUND_ENUM round, int coolf);
static void followTapInit(void);
static void followTapSave(SCORE_INDV_STR *sindv_pp);
static SCR_TAP_MEMORY* followTapLoad(int pos, int time);
static void ScrLincChangTbl(int line);
static void ScrLincChangTblRef(int line, int ck_time);
static int  tapLevelChangeSub(void);
static void tapLevelChange(SCORE_INDV_STR *sindv_pp);
static int  useIndevCodeGet(void);
static int  targetTimeGet(int line, int time, int codeAll);
static int  otehon_all_make(EXAM_CHECK *ec_pp);
static int  treateTimeChange(int time);
static int  thnum_get(int p96_num, CK_TH_ENUM ckth);
static int  MapNormalNumGet(int time);
static void on_th_make(EXAM_CHECK *ec_pp, CK_TH_ENUM ckth);
static int  exh_normal_add(EXAM_CHECK *ec_pp);
static int  exh_normal_sub(EXAM_CHECK *ec_pp);
static int  exh_nombar_sub(EXAM_CHECK *ec_pp);
static int  exh_mbar_key_out(EXAM_CHECK *ec_pp);
static int  exh_mbar_time_out(EXAM_CHECK *ec_pp);
static int  exh_mbar_num_out(EXAM_CHECK *ec_pp);
static int  exh_yaku(EXAM_CHECK *ec_pp, int hane_flag);
static int  exh_yaku_original(EXAM_CHECK *ec_pp);
static int  exh_yaku_hane(EXAM_CHECK *ec_pp);
static int  exh_allkey_out(EXAM_CHECK *ec_pp);
static int  exh_allkey_out_nh(EXAM_CHECK *ec_pp);
static int  exh_command(EXAM_CHECK *ec_pp);
static int  exh_renda_out(EXAM_CHECK *ec_pp);
static int  manemane_check_sub(EXAM_CHECK *ec_pp);
static int  manemane_check(EXAM_CHECK *ec_pp);
static int  exh_mane(EXAM_CHECK *ec_pp);
static int  exh_all_add(EXAM_CHECK *ec_pp);
static TAPSET* IndvGetTapSetAdrs(SCORE_INDV_STR *sindv_pp);
static int  nextExamTime(void);
static SCORE_INDV_STR* GetSindvPcodeLine(PLAYER_CODE pcode);
static void ExamScoreCheck(SCORE_INDV_STR *sindv_pp);
static int  ExamScoreCheckSame(SCORE_INDV_STR *sindv_pp);
static int  levelChangeCheck(RANK_LEVEL lvl0, RANK_LEVEL lvl1);
static int  levelUpRank(RANK_LEVEL lvl);
static int  levelDownRank(RANK_LEVEL lvl);
static void ScrCtrlIndvJob(void);
static void ScrTimeRenew(SCR_MAIN *scr_main_pp);
static int  otehonSetCheck(void);
static void bonusGameInit(void);
static int  bonusGameCntPls(void);
static void bonusPointSave(void);
static void bonusGameParaReq(BNG_ACT_P_ENUM actnum);
static void bonusGameKoamaReq(int kotamaNum, BNG_ACT_K_ENUM actnum);
static int  bonus_minus_point_sub(int wtime);
static int  bonus_pls_point_sub(int wtime);
static void bonusGameCtrl(int time);
static u_long hex2dec(u_long data);
static void bnNumberDisp(sceGifPacket *gif_pp, long score, short x, short y, int keta, int tate, int type);
static void bonusScoreDraw(void);
static void set_lero_gifset(sceGifPacket *gifpk_pp, LERO_TIM2_PT *let2_pp, short xp, short yp);
static void LessonRoundDisp(SCRRJ_LESSON_ROUND_ENUM type);

static int titleStartKey = FALSE;
static int fadeoutStartKey = FALSE;
static int gameEndWaitLoop = FALSE;
static int replayGuiOffFlag = FALSE;
static int jimakuWakuOff = FALSE;

int currentTblNumber = 0;
int vs_tapdat_work_cnt = 0;
int scrJimakuLine = 0;
int scrDrawLine = 0;
int scrMbarLine = 0;
int scrRefLineTime = -1;

static EXH_STR exh_str_normal[] = {
    { .score_prg = exh_normal_add,    .save_p = EXH_NORMAL_ADD,    .bairitu = 48 },
    { .score_prg = exh_normal_sub,    .save_p = EXH_NORMAL_SUB,    .bairitu = 48 },
    { .score_prg = exh_nombar_sub,    .save_p = EXH_NOMBAR_SUB,    .bairitu = 48 },
    { .score_prg = exh_mbar_key_out,  .save_p = EXH_MBAR_KEY_OUT,  .bairitu = 48 },
    { .score_prg = exh_mbar_time_out, .save_p = EXH_MBAR_TIME_OUT, .bairitu = 48 },
    { .score_prg = exh_mbar_num_out,  .save_p = EXH_MBAR_NUM_OUT,  .bairitu = 32 },
    { .score_prg = exh_allkey_out_nh, .save_p = EXH_ALLKEY_OUT,    .bairitu = 32 },
    { .score_prg = exh_renda_out,     .save_p = EXH_RENDA_OUT,     .bairitu = 32 },
    { .score_prg = exh_mane,          .save_p = EXH_MANE,          .bairitu = 8  },
    { .score_prg = exh_all_add,       .save_p = EXH_TOTAL,         .bairitu = 16 },
};

static EXH_STR exh_str_original[] = {
    { .score_prg = exh_nombar_sub,    .save_p = EXH_NOMBAR_SUB,    .bairitu = 48 },
    { .score_prg = exh_mbar_key_out,  .save_p = EXH_MBAR_KEY_OUT,  .bairitu = 16 },
    { .score_prg = exh_allkey_out,    .save_p = EXH_ALLKEY_OUT,    .bairitu = 48 },
    { .score_prg = exh_renda_out,     .save_p = EXH_RENDA_OUT,     .bairitu = 48 },
    { .score_prg = exh_yaku_original, .save_p = EXH_YAKU,          .bairitu = 16 },
    { .score_prg = exh_command,       .save_p = EXH_COMMAND,       .bairitu = 16 },
    { .score_prg = exh_all_add,       .save_p = EXH_TOTAL,         .bairitu = 16 },
};

static EXH_STR exh_str_hane[] = {
    { .score_prg = exh_nombar_sub,    .save_p = EXH_NOMBAR_SUB,    .bairitu = 48 },
    { .score_prg = exh_mbar_key_out,  .save_p = EXH_MBAR_KEY_OUT,  .bairitu = 16 },
    { .score_prg = exh_allkey_out,    .save_p = EXH_ALLKEY_OUT,    .bairitu = 16 },
    { .score_prg = exh_renda_out,     .save_p = EXH_RENDA_OUT,     .bairitu = 16 },
    { .score_prg = exh_yaku_hane,     .save_p = EXH_YAKU,          .bairitu = 24 },
    { .score_prg = exh_all_add,       .save_p = EXH_TOTAL,         .bairitu = 16 },
};

static EXH_STR exh_str_hook[] = {
    { .score_prg = manemane_check,    .save_p = EXH_MANE,          .bairitu = 16 },
    { .score_prg = exh_all_add,       .save_p = EXH_TOTAL,         .bairitu = 16 },
};

static u_int thnum_tbl[] = { 0x00e79e79, 0x00f3cf3c, 0x00c1cc1c, 0x00000000 };

TCL_CTRL tcl_ctrl[4][33] = {
    /* TRND_R1 */
    {
        /* TCL_TYPE_EZ_0  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_1  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_2  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_3  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_4  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_5  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_6  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_7  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = -1, .max = 0xf },
        /* TCL_TYPE_EZ_8  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 1, .max = 0xf },
        /* TCL_TYPE_EZ_9  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 2, .max = 0xf },
        /* TCL_TYPE_EZ_10 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 3, .max = 0xf },
        /* TCL_TYPE_EZ_11 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 4, .max = 0xf },
        /* TCL_TYPE_EZ_12 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 5, .max = 0xf },
        /* TCL_TYPE_EZ_13 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 6, .max = 0xf },
        /* TCL_TYPE_EZ_14 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 7, .max = 0xf },
        /* TCL_TYPE_EZ_15 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 8, .max = 0xf },

        /* TCL_TYPE_HD_0  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_1  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_2  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_3  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_4  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_5  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_6  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_7  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = -1, .max = 0xf },
        /* TCL_TYPE_HD_8  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 1, .max = 0xf },
        /* TCL_TYPE_HD_9  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 2, .max = 0xf },
        /* TCL_TYPE_HD_10 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 3, .max = 0xf },
        /* TCL_TYPE_HD_11 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 4, .max = 0xf },
        /* TCL_TYPE_HD_12 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 5, .max = 0xf },
        /* TCL_TYPE_HD_13 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 6, .max = 0xf },
        /* TCL_TYPE_HD_14 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 7, .max = 0xf },
        /* TCL_TYPE_HD_15 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 8, .max = 0xf },

        /* TCL_TYPE_COOL  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_COOL_OVER, .min = 0xf, .max = 0xf },
    },
    /* TRND_R2 */
    {
        /* TCL_TYPE_EZ_0  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_1  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_2  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_3  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_4  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_5  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_6  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_7  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = -1, .max = 0xf },
        /* TCL_TYPE_EZ_8  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 1, .max = 0xf },
        /* TCL_TYPE_EZ_9  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 2, .max = 0xf },
        /* TCL_TYPE_EZ_10 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 3, .max = 0xf },
        /* TCL_TYPE_EZ_11 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 4, .max = 0xf },
        /* TCL_TYPE_EZ_12 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 5, .max = 0xf },
        /* TCL_TYPE_EZ_13 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 6, .max = 0xf },
        /* TCL_TYPE_EZ_14 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 7, .max = 0xf },
        /* TCL_TYPE_EZ_15 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 8, .max = 0xf },

        /* TCL_TYPE_HD_0  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_1  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_2  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_3  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_4  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_5  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_6  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_7  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = -1, .max = 0xf },
        /* TCL_TYPE_HD_8  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 1, .max = 0xf },
        /* TCL_TYPE_HD_9  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 2, .max = 0xf },
        /* TCL_TYPE_HD_10 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 3, .max = 0xf },
        /* TCL_TYPE_HD_11 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 4, .max = 0xf },
        /* TCL_TYPE_HD_12 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 5, .max = 0xf },
        /* TCL_TYPE_HD_13 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 6, .max = 0xf },
        /* TCL_TYPE_HD_14 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 7, .max = 0xf },
        /* TCL_TYPE_HD_15 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 8, .max = 0xf },

        /* TCL_TYPE_COOL  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_COOL_OVER, .min = 0xf, .max = 0xf },
    },
    /* TRND_R3 */
    {
        /* TCL_TYPE_EZ_0  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_1  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_2  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_3  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_4  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_5  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_6  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_7  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = -1, .max = 0xf },
        /* TCL_TYPE_EZ_8  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 1, .max = 0xf },
        /* TCL_TYPE_EZ_9  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 2, .max = 0xf },
        /* TCL_TYPE_EZ_10 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 3, .max = 0xf },
        /* TCL_TYPE_EZ_11 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 4, .max = 0xf },
        /* TCL_TYPE_EZ_12 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 5, .max = 0xf },
        /* TCL_TYPE_EZ_13 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 6, .max = 0xf },
        /* TCL_TYPE_EZ_14 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 7, .max = 0xf },
        /* TCL_TYPE_EZ_15 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 8, .max = 0xf },

        /* TCL_TYPE_HD_0  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_1  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_2  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_3  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_4  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_5  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_6  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_7  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = -1, .max = 0xf },
        /* TCL_TYPE_HD_8  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 1, .max = 0xf },
        /* TCL_TYPE_HD_9  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 2, .max = 0xf },
        /* TCL_TYPE_HD_10 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 3, .max = 0xf },
        /* TCL_TYPE_HD_11 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 4, .max = 0xf },
        /* TCL_TYPE_HD_12 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 5, .max = 0xf },
        /* TCL_TYPE_HD_13 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 6, .max = 0xf },
        /* TCL_TYPE_HD_14 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 7, .max = 0xf },
        /* TCL_TYPE_HD_15 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 8, .max = 0xf },

        /* TCL_TYPE_COOL  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_COOL_OVER, .min = 0xf, .max = 0xf },
    },
    /* TRND_R4 */
    {
        /* TCL_TYPE_EZ_0  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_1  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_2  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_3  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_4  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_5  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_6  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_EZ_7  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = -1, .max = 0xf },
        /* TCL_TYPE_EZ_8  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 1, .max = 0xf },
        /* TCL_TYPE_EZ_9  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 2, .max = 0xf },
        /* TCL_TYPE_EZ_10 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 3, .max = 0xf },
        /* TCL_TYPE_EZ_11 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 4, .max = 0xf },
        /* TCL_TYPE_EZ_12 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 5, .max = 0xf },
        /* TCL_TYPE_EZ_13 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 6, .max = 0xf },
        /* TCL_TYPE_EZ_14 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 7, .max = 0xf },
        /* TCL_TYPE_EZ_15 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 8, .max = 0xf },

        /* TCL_TYPE_HD_0  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_1  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_2  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_3  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_4  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_5  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_6  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH_MORE, .min = -1, .max = -1 },
        /* TCL_TYPE_HD_7  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = -1, .max = 0xf },
        /* TCL_TYPE_HD_8  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 1, .max = 0xf },
        /* TCL_TYPE_HD_9  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 2, .max = 0xf },
        /* TCL_TYPE_HD_10 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 3, .max = 0xf },
        /* TCL_TYPE_HD_11 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 4, .max = 0xf },
        /* TCL_TYPE_HD_12 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 5, .max = 0xf },
        /* TCL_TYPE_HD_13 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 6, .max = 0xf },
        /* TCL_TYPE_HD_14 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 7, .max = 0xf },
        /* TCL_TYPE_HD_15 */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_OTH2COOL_MORE, .min = 8, .max = 0xf },

        /* TCL_TYPE_COOL  */ { .tcl_do_enum_down = TCL_DO_ZERO_UPTO, .tcl_do_enum_up = TCL_DO_COOL_OVER, .min = 0xf, .max = 0xf },
    },
};

static SCRPRGSTR scrprgstr[] = {
    { .size = 0xa, .exh_str_pp = exh_str_normal },
    { .size = 0x7, .exh_str_pp = exh_str_original },
    { .size = 0x6, .exh_str_pp = exh_str_hane },
};

static SCRPRGSTR scrprgstr_hook[] = {
    { .size = 0x2, .exh_str_pp = exh_str_hook },
};

static TIM2_DAT tim2spr_tbl[] = {
    /* BN_KANJI_TXT */
    {
        .GsTex0 = SCE_GS_SET_TEX0(0x36b0, 4, SCE_GS_PSMT4, 8, 6, 1, 0, 0x36c4, SCE_GS_PSMCT32, 0, 0, 1),
        .GsTex1 = GS_TEX1_LINEAR,
        .GsRegs = 0x0,
        .GsTexClut = 0x0,
        .w = 160,
        .h = 64,
    },
    /* BN_SUUJI_TXT */
    {
        .GsTex0 = SCE_GS_SET_TEX0(0x36c0, 2, SCE_GS_PSMT4, 7, 5, 1, 0, 0x36c5, SCE_GS_PSMCT32, 0, 0, 1),
        .GsTex1 = GS_TEX1_LINEAR,
        .GsRegs = 0x0,
        .GsTexClut = 0x0,
        .w = 120,
        .h = 24,
    },
};

static BN_NUM_TYPE bn_num_type[] = {
    /* BN_KANJI_TXT */
    {
        .tim2_dat_pp = &tim2spr_tbl[0],
        .w = 32,
        .h = 32,
        .map = {
            { 0,   0  },
            { 32,  0  },
            { 64,  0  },
            { 96,  0  },
            { 128, 0  },
            { 0,   32 },
            { 32,  32 },
            { 64,  32 },
            { 96,  32 },
            { 128, 32 },
        }
    },
    /* BN_SUUJI_TXT */
    {
        .tim2_dat_pp = &tim2spr_tbl[1],
        .w = 12,
        .h = 24,
        .map = {
            { 0,   0 },
            { 12,  0 },
            { 24,  0 },
            { 36,  0 },
            { 48,  0 },
            { 60,  0 },
            { 72,  0 },
            { 84,  0 },
            { 96,  0 },
            { 108, 0 },
        }
    },
};

static LERO_TIM2_PT lero_tim2_pt[] = {
    /* LERO_ENUM_1 */
    { .u0 = 248, .v0 = 0, .u1 = 264, .v1 = 32, .w = 32,  .h = 32 },
    /* LERO_ENUM_2 */
    { .u0 = 264, .v0 = 0, .u1 = 288, .v1 = 32, .w = 48,  .h = 32 },
    /* LERO_ENUM_3 */
    { .u0 = 288, .v0 = 0, .u1 = 312, .v1 = 32, .w = 48,  .h = 32 },
    /* LERO_ENUM_4 */
    { .u0 = 312, .v0 = 0, .u1 = 344, .v1 = 32, .w = 64,  .h = 32 },
    /* LERO_ENUM_5 */
    { .u0 = 344, .v0 = 0, .u1 = 376, .v1 = 32, .w = 64,  .h = 32 },
    /* LERO_ENUM_LESSON */
    { .u0 = 0,   .v0 = 0, .u1 = 128, .v1 = 32, .w = 256, .h = 32 },
    /* LERO_ENUM_ROUND */
    { .u0 = 128, .v0 = 0, .u1 = 248, .v1 = 32, .w = 240, .h = 32 },
};

static LERO_POS_STR lero_pos_str[][2] = {
    /* SCRRJ_LR_LESSON_1 */
    {
        { .tim2_num = LERO_ENUM_LESSON, .posx = -160, .posy = -16 },
        { .tim2_num = LERO_ENUM_1,      .posx =  112, .posy = -16 }
    },
    /* SCRRJ_LR_LESSON_2 */
    {
        { .tim2_num = LERO_ENUM_LESSON, .posx = -160, .posy = -16 },
        { .tim2_num = LERO_ENUM_2,      .posx =  104, .posy = -16 }
    },
    /* SCRRJ_LR_LESSON_3 */
    {
        { .tim2_num = LERO_ENUM_LESSON, .posx = -160, .posy = -16 },
        { .tim2_num = LERO_ENUM_3,      .posx =  104, .posy = -16 }
    },
    /* SCRRJ_LR_LESSON_4 */
    {
        { .tim2_num = LERO_ENUM_LESSON, .posx = -160, .posy = -16 },
        { .tim2_num = LERO_ENUM_4,      .posx =  96,  .posy = -16 }
    },
    /* SCRRJ_LR_LESSON_5 */
    {
        { .tim2_num = LERO_ENUM_LESSON, .posx = -160, .posy = -16 },
        { .tim2_num = LERO_ENUM_5,      .posx =  96,  .posy = -16 }
    },

    /* SCRRJ_LR_ROUND_1 */
    {
        { .tim2_num = LERO_ENUM_ROUND,  .posx = -152, .posy = -48 },
        { .tim2_num = LERO_ENUM_1,      .posx =  104, .posy = -48 }
    },
    /* SCRRJ_LR_ROUND_2 */
    {
        { .tim2_num = LERO_ENUM_ROUND,  .posx = -152, .posy = -48 },
        { .tim2_num = LERO_ENUM_2,      .posx =  96,  .posy = -48 }
    },
    /* SCRRJ_LR_ROUND_3 */
    {
        { .tim2_num = LERO_ENUM_ROUND,  .posx = -152, .posy = -48 },
        { .tim2_num = LERO_ENUM_3,      .posx =  96,  .posy = -48 }
    },
    /* SCRRJ_LR_ROUND_4 */
    {
        { .tim2_num = LERO_ENUM_ROUND,  .posx = -152, .posy = -48 },
        { .tim2_num = LERO_ENUM_4,      .posx =  88,  .posy = -48 }
    },
    /* SCRRJ_LR_ROUND_5 */
    {
        { .tim2_num = LERO_ENUM_ROUND,  .posx = -152, .posy = -48 },
        { .tim2_num = LERO_ENUM_5,      .posx =  88,  .posy = -48 }
    },
};

SCR_SND_DBUFF scr_snd_dbuff = {};
static SNDTAP *scr_sndtap_pp[4];
SCORE_STR score_str = {};
static SCORE_INDV_STR score_indv_str[5];
TAPDAT vs_tapdat_work[64] = {};
static int follow_scr_tap_memory_cnt;
static int follow_scr_tap_memory_cnt_load;
static SCR_TAP_MEMORY follow_scr_tap_memory[256];
static TAP_GROUPE_STR tap_groupe_str[5];
static COMMAKE_STR commake_str[32];
static int commake_str_cnt;
static EXAM_CHECK exam_check[3];
static u_char yaku_tmp_buf[36];
static BNG_STR bng_str;

int GetCurrentTblNumber(void) {
    return currentTblNumber;
}

DISP_LEVEL RANK_LEVEL2DISP_LEVEL(RANK_LEVEL lvl) {
    DISP_LEVEL lvl_tbl[17] = {
        DLVL_COOL,  DLVL_COOL,
        DLVL_GOOD,  DLVL_GOOD,  DLVL_GOOD,
        DLVL_BAD,   DLVL_BAD,   DLVL_BAD,
        DLVL_AWFUL, DLVL_AWFUL, DLVL_AWFUL,
        DLVL_MAX,   DLVL_MAX,   DLVL_MAX,
        DLVL_MAX,   DLVL_MAX,   DLVL_MAX,
    };

    return lvl_tbl[lvl];
}

DISP_LEVEL RANK_LEVEL2DISP_LEVEL_HK(RANK_LEVEL lvl) {
    DISP_LEVEL lvl_tbl[15] = {
        DLVL_HK_COOL,  DLVL_HK_COOL,
        DLVL_HK_GOOD,  DLVL_HK_GOOD,  DLVL_HK_GOOD,
        DLVL_HK_BAD1,  DLVL_HK_BAD1,  DLVL_HK_BAD1,
        DLVL_HK_BAD2,  DLVL_HK_BAD2,  DLVL_HK_BAD2,
        DLVL_HK_BAD3,  DLVL_HK_BAD3,  DLVL_HK_BAD3, DLVL_HK_MAX,
    };

    return lvl_tbl[lvl];
}

void ScrTapDbuffCtrlInit(void *data_top, int bk0, int bk1) {
    scr_snd_dbuff.bank[0]      = bk0;
    scr_snd_dbuff.bank[1]      = bk1;

    scr_snd_dbuff.data_top     = data_top;
    scr_snd_dbuff.next_index   = 0;

    scr_snd_dbuff.sndrec_pp[0] = NULL;
    scr_snd_dbuff.sndrec_pp[1] = NULL;
}

u_int ScrTapDbuffSet(SNDREC *sndrec_pp) {
    u_int ret;
    u_int id;

    id = scr_snd_dbuff.next_index & 1;
    if (scr_snd_dbuff.sndrec_pp[id] == sndrec_pp) {
        ret = scr_snd_dbuff.next_index;
        scr_snd_dbuff.next_index++;
        return ret;
    }

    if (scr_snd_dbuff.sndrec_pp[id ^ 1] == sndrec_pp) {
        return id ^ 1;
    }

    ScrTapDataTrans(sndrec_pp, scr_snd_dbuff.bank[id], scr_snd_dbuff.data_top);
    ret = id;

    scr_snd_dbuff.sndrec_pp[id] = sndrec_pp;
    scr_snd_dbuff.next_index++;
    return ret;
}

void ScrTapDbuffSetSp(SNDREC *sndrec_pp, int id) {
    if (id < 0) {
        return;
    }

    scr_snd_dbuff.next_index = id;
    ScrTapDataTrans(sndrec_pp, scr_snd_dbuff.bank[id & 1 ^ 1], scr_snd_dbuff.data_top);
    scr_snd_dbuff.sndrec_pp[id & 1 ^ 1] = sndrec_pp;
}

void ScrTapDbuffClear(void) {
    scr_snd_dbuff.next_index = 0;
    scr_snd_dbuff.sndrec_pp[0] = NULL;
    scr_snd_dbuff.sndrec_pp[1] = NULL;
}

void ScrTapCtrlInit(void *data_top) {
    ScrTapDbuffCtrlInit(data_top, 1, 2);
}

void ScrTapDataTrans(SNDREC *sndrec_pp, int bank, void *data_top) {
    if (sndrec_pp->bd_num >= 0) {
        TapCt(TAPCT_BDSPUTRANS | bank, (int)GetIntAdrsCurrent(sndrec_pp->bd_num), GetIntSizeCurrent(sndrec_pp->bd_num));
        TapCt(TAPCT_HDIOPTRANS | bank, (int)GetIntAdrsCurrent(sndrec_pp->hd_num), GetIntSizeCurrent(sndrec_pp->hd_num));
    }

    scr_sndtap_pp[bank] = sndrec_pp->sndtap_pp;

    TapCt(TAPCT_SETMASTERVOL, PR_CONCAT(0x3fff, 0x3fff), TAPCT_NONE);
    TapCt(TAPCT_SETVOLUME,    PR_CONCAT(0x3fff, 0x3fff), TAPCT_NONE);
}

int ScrTapDataTransCheck(void) {
    return TapCt(TAPCT_TRANSCHECK, TAPCT_NONE, TAPCT_NONE);
}

void ScrTapReq(int id, int box, int num) {
    int     use_chan;
    SNDTAP *tp_pp;

    if (id == SCR_TAP_COMMON) {
        use_chan = 0;
    } else {
        use_chan = scr_snd_dbuff.bank[id & 1];
    }

    tp_pp = &scr_sndtap_pp[use_chan][num];

    TapCt(TAPCT_TAPVOLUME | use_chan, box, tp_pp->volume);
    TapCt(TAPCT_TAPREQ    | use_chan, box, tp_pp->prg + tp_pp->key * 256);
}

void ScrTapReqStop(int box) {
    TapCt(TAPCT_TAPSTOP, box, TAPCT_NONE);
}

static void exam_tbl_updownInit(SCORE_INDV_STR *sindv_pp) {
    int i;

    if (sindv_pp->global_ply == NULL) {
        return;
    }

    for (i = 0; i < PR_ARRAYSIZE(sindv_pp->global_ply->exam_tbl_updown); i++) {
        sindv_pp->global_ply->exam_tbl_updown[i] = 0;
    }
}

static void exam_tbl_updownSet(SCORE_INDV_STR *sindv_pp, int now, int sikiichi /* Threshold */, int oth) {
    int hikaku; /* Comparison variable */

    if (sindv_pp->global_ply == NULL) {
        return;
    }

    if (oth >= sikiichi / 2) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_NONE]++;
    } else {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_NONE]--;
    }

    hikaku = sikiichi / 2;

    if (now > hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_COOLHF_OVER]++;
    }
    if (now >= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_COOLHF_MORE]++;
    }
    if (now <= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_COOLHF_UPTO]++;
    }
    if (now < hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_COOLHF_UNDER]++;
    }

    hikaku = oth;

    if (now > hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTH_OVER]++;
    }
    if (now >= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTH_MORE]++;
    }
    if (now <= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTH_UPTO]++;
    }
    if (now < hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTH_UNDER]++;
    }

    hikaku = 0;

    if (now > hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_ZERO_OVER]++;
    }
    if (now >= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_ZERO_MORE]++;
    }
    if (now <= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_ZERO_UPTO]++;
    }
    if (now < hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_ZERO_UNDER]++;
    }

    hikaku = sikiichi;

    if (now > hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_COOL_OVER]++;
    }
    if (now >= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_COOL_MORE]++;
    }
    if (now <= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_COOL_UPTO]++;
    }
    if (now < hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_COOL_UNDER]++;
    }

    hikaku = (sikiichi - oth) / 2 + oth;

    if (now > hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTH2COOL_OVER]++;
    }
    if (now >= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTH2COOL_MORE]++;
    }
    if (now <= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTH2COOL_UPTO]++;
    }
    if (now < hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTH2COOL_UNDER]++;
    }

    hikaku = oth / 2;

    if (now > hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTHHF_OVER]++;
    }
    if (now >= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTHHF_MORE]++;
    }
    if (now <= hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTHHF_UPTO]++;
    }
    if (now < hikaku) {
        sindv_pp->global_ply->exam_tbl_updown[TCL_DO_OTHHF_UNDER]++;
    }
}

static int exam_tbl_updownChange(SCORE_INDV_STR *sindv_pp, TAP_CTRL_LEVEL_ENUM clv, TAP_ROUND_ENUM round, int coolf) {
    TCL_CTRL *tcl_ctrl_pp;
    int       ret = 0;
    int       udc;

    if (sindv_pp->global_ply == NULL) {
        return 0;
    }

    if (coolf) {
        tcl_ctrl_pp = &tcl_ctrl[round][TCL_TYPE_COOL];
    } else {
        if (sindv_pp->global_ply->exam_tbl_updown[TCL_DO_NONE] < 0) {
            tcl_ctrl_pp = &tcl_ctrl[round][clv];
        } else {
            tcl_ctrl_pp = &tcl_ctrl[round][clv + TCL_TYPE_HD_TOP];
        }
    }

    udc = 0;

    if (tcl_ctrl_pp->tcl_do_enum_up != TCL_DO_NONE) {
        udc = sindv_pp->global_ply->exam_tbl_updown[tcl_ctrl_pp->tcl_do_enum_up];
    }
    if (tcl_ctrl_pp->tcl_do_enum_down != TCL_DO_NONE) {
        udc -= sindv_pp->global_ply->exam_tbl_updown[tcl_ctrl_pp->tcl_do_enum_down];
    }

    if (udc < 0) {
        if (tcl_ctrl_pp->min < 0) {
            ret = tcl_ctrl_pp->min;
        } else {
            if (udc < -tcl_ctrl_pp->min) {
                ret = -tcl_ctrl_pp->min;
            } else {
                ret = udc;
            }
        }
    } else if (udc > 0) {
        if (tcl_ctrl_pp->max < 0) {
            ret = -tcl_ctrl_pp->max;
        } else {
            if (udc > tcl_ctrl_pp->max) {
                ret = tcl_ctrl_pp->max;
            } else {
                ret = udc;
            }
        }
    }

    return ret;
}

void vsTapdatSetMemorySave(void) {
    int        i;
    VSOTHSAVE  vsothsave_tmp;
    TAPDAT    *tapdat_pp;

    tapdat_pp = vs_tapdat_work;

    WorkClear(vsothsave_tmp, sizeof(vsothsave_tmp));

    for (i = 0; i < vs_tapdat_work_cnt; i++, tapdat_pp++) {
        int time = tapdat_pp->time / 24;
        if (time >= 32) {
            printf("OTH SAVE OVER\n");
        } else {
            vsothsave_tmp[time] = (u_char)tapdat_pp->KeyIndex;
        }
    }

    mccReqVSOTHSAVEset(&vsothsave_tmp);
}

void vsTapdatSetMemoryLoad(void) {
    VSOTHSAVE vsothsave_tmp;
    int       i;

    WorkClear(&vsothsave_tmp, sizeof(vsothsave_tmp));

    if (mccReqVSOTHSAVEget(&vsothsave_tmp)) {
        vs_tapdat_work_cnt = 0;

        for (i = 0; i < 32; i++) {
            if (vsothsave_tmp[i] != 0) {
                vs_tapdat_work[vs_tapdat_work_cnt].time = i * 24;
                vs_tapdat_work[vs_tapdat_work_cnt].tapct[0].actor = -1;
                vs_tapdat_work[vs_tapdat_work_cnt].tapct[0].sound = -1;
                vs_tapdat_work[vs_tapdat_work_cnt].KeyIndex = vsothsave_tmp[i];
                vs_tapdat_work_cnt++;
            }
        }
    }
}

void vsTapdatSet(SCORE_INDV_STR *sindv_pp) {
    int             i;
    SCR_TAP_MEMORY *stm_pp;
    int             tmptime;
    int             endlng;
    int             current_time;
    TAPSET         *tapset_pp;
    TAPDAT         *tapdat_pp;
    int             KeyCodeAll;
    int             KeyCodeAllCk;

    printf("new vs tap!!\n");

    KeyCodeAll = 0;
    KeyCodeAllCk = 0;

    tapset_pp = IndvGetTapSetAdrs(sindv_pp);
    tapdat_pp = tapset_pp->tapdat_pp;

    endlng = ((tapset_pp->taptimeEnd / 24) - (tapset_pp->taptimeStart / 24)) * 24;

    for (i = 0; i < tapset_pp->tapdat_size; i++, tapdat_pp++) {
        if (tapdat_pp->KeyIndex != KiNO) {
            if (global_data.play_typeL == PLAY_TYPE_ONE) {
                KeyCodeAll = GetIndex2KeyCode(KiTR);
                break;
            }

            KeyCodeAll |= GetIndex2KeyCode(tapdat_pp->KeyIndex);
        }
    }

    stm_pp = sindv_pp->scr_tap_memory;
    current_time = -1;

    for (i = 0; i < sindv_pp->scr_tap_memory_cnt; i++, stm_pp++) {
        if (!stm_pp->onKey) {
            continue;
        }

        tmptime = treateTimeChange(stm_pp->ofs_frame);
        if (tmptime < 0) {
            continue;
        }

        if (current_time == tmptime) {
            continue;
        }

        if (tmptime >= endlng) {
            continue;
        }

        current_time = tmptime;
        KeyCodeAllCk |= GetIndex2KeyCode(stm_pp->key);
    }

    if (KeyCodeAllCk == KeyCodeAll) {
        vs_tapdat_work_cnt = 0;
        current_time = -1;

        stm_pp = sindv_pp->scr_tap_memory;

        for (i = 0; i < sindv_pp->scr_tap_memory_cnt; i++, stm_pp++) {
            if (!stm_pp->onKey) {
                continue;
            }

            tmptime = treateTimeChange(stm_pp->ofs_frame);
            if (tmptime < 0) {
                continue;
            }

            if (current_time == tmptime) {
                continue;
            }

            if (tmptime >= endlng) {
                continue;
            }

            current_time = tmptime;

            vs_tapdat_work[vs_tapdat_work_cnt].time = current_time;
            vs_tapdat_work[vs_tapdat_work_cnt].tapct[0].actor = -1;
            vs_tapdat_work[vs_tapdat_work_cnt].tapct[0].sound = -1;
            vs_tapdat_work[vs_tapdat_work_cnt].KeyIndex = stm_pp->key;
            vs_tapdat_work_cnt++;
        }
    }
}

void vsTapdatSetMoto(SCORE_INDV_STR *sindv_pp) {
    int     i;
    TAPSET *tapset_pp;
    TAPDAT *tapdat_pp;

    vs_tapdat_work_cnt = 0;
    printf("reset moto!!\n");

    tapset_pp = IndvGetTapSetAdrs(sindv_pp);
    tapdat_pp = tapset_pp->tapdat_pp;

    for (i = 0; i < tapset_pp->tapdat_size; i++, tapdat_pp++) {
        if (tapdat_pp->KeyIndex != KiNO) {
            if (global_data.play_typeL == PLAY_TYPE_ONE) {
                vs_tapdat_work[vs_tapdat_work_cnt].KeyIndex = KiTR;
            } else {
                vs_tapdat_work[vs_tapdat_work_cnt].KeyIndex = tapdat_pp->KeyIndex;
            }

            vs_tapdat_work[vs_tapdat_work_cnt].time = tapdat_pp->time;
            vs_tapdat_work[vs_tapdat_work_cnt].tapct[0].actor = -1;
            vs_tapdat_work[vs_tapdat_work_cnt].tapct[0].sound = -1;

            vs_tapdat_work_cnt++;
        }
    }
}

static void followTapInit(void) {
    follow_scr_tap_memory_cnt = 0;
    follow_scr_tap_memory_cnt_load = 0;
    WorkClear(follow_scr_tap_memory, sizeof(follow_scr_tap_memory));
}

static void followTapSave(SCORE_INDV_STR *sindv_pp) {
    int i;

    follow_scr_tap_memory_cnt = 0;
    follow_scr_tap_memory_cnt_load = 0;

    for (i = 0; i < sindv_pp->scr_tap_memory_cnt; i++) {
        if (sindv_pp->scr_tap_memory[i].onKey) {
            follow_scr_tap_memory[follow_scr_tap_memory_cnt] = sindv_pp->scr_tap_memory[i];
            follow_scr_tap_memory_cnt++;
        }
    }
}

static SCR_TAP_MEMORY* followTapLoad(int pos, int time) {
    if (follow_scr_tap_memory_cnt <= pos) {
        return NULL;
    }
    if (follow_scr_tap_memory[pos].ofs_frame > time) {
        return NULL;
    }
    if (follow_scr_tap_memory_cnt_load != pos) {
        return NULL;
    }

    follow_scr_tap_memory_cnt_load = pos + 1;
    return &follow_scr_tap_memory[pos];
}

static void ScrLincChangTbl(int line) {
    scrJimakuLine = line;
    scrDrawLine   = line;
    scrMbarLine   = line;

    DrawCtrlTblChange(GetDrawLine(line));
}

/* Set on a line number while the line is drawn from a reference time (scrRefLineTime). */
#define SCR_LINE_REF_MODE 0x8000

static void ScrLincChangTblRef(int line, int ck_time) {
    scrDrawLine |= SCR_LINE_REF_MODE;
    scrMbarLine |= SCR_LINE_REF_MODE;
    scrRefLineTime = ck_time;

    DrawCtrlTblChange(GetDrawLine(line));
}

void ScrLineSafeRefMode(void) {
    if (scrDrawLine & SCR_LINE_REF_MODE) {
        scrDrawLine &= ~SCR_LINE_REF_MODE;
        DrawCtrlTblChange(GetDrawLine(scrDrawLine));
    }

    if (scrMbarLine & SCR_LINE_REF_MODE) {
        scrMbarLine &= ~SCR_LINE_REF_MODE;
    }
}

int ScrDrawTimeGet(int line) {
    if (line & SCR_LINE_REF_MODE) {
        return scrRefLineTime;
    }

    return score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[line].lineTime;
}

int ScrDrawTimeGetFrame(int line) {
    if (line & SCR_LINE_REF_MODE) {
        return (scrRefLineTime * 3600.0f + GetLineTempo(line) * 96.0f * 0.5f) / (GetLineTempo(line) * 96.0f);
    }

    return score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[line].lineTimeFrame;
}

void KeyCntClear(int *key_pp) {
    int i;

    for (i = 0; i < 7; i++) {
        *key_pp++ = -1;
    }
}

SCRREC* ScrCtrlCurrentSearch(SCORE_INDV_STR *sindv_pp, int index, int frame) {
    SCRREC *scrrec_pp;

    if (score_str.stdat_dat_pp->scr_pp->scr_ctrl_num <= index) {
        printf("ScrCtrlCurrentSearch index[%d] is over!!\n", index);
        return NULL;
    }

    scrrec_pp = sindv_pp->top_scr_ctrlpp[index].scrrec_pp;
    if (scrrec_pp == NULL) {
        printf("ScrCtrlCurrentSearch index[%d] is NULL line!!\n", index);
        return NULL;
    }

    while (1) {
        if (scrrec_pp->job == SCRRJ_ENDJOB || scrrec_pp->job == SCRRJ_ENDGAME) {
            break;
        }
        if (scrrec_pp->job == SCRRJ_PLY && scrrec_pp->data >= frame) {
            break;
        }

        scrrec_pp++;
    }

    return scrrec_pp;
}

void ScrCtrlIndvInit(STDAT_DAT *sdat_pp) {
    int             i, j;
    int             dare;

    SCORE_INDV_STR *sindv_pp;
    GLOBAL_PLY     *gply_pp;

    WorkClear(score_indv_str, sizeof(score_indv_str));
    sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        dare = -1;
        gply_pp = global_data.global_ply;

        for (j = 0; j < PR_ARRAYSIZE(global_data.global_ply); j++) {
            if (gply_pp[j].player_code == PR_BIT(i)) {
                dare = j;
                break;
            }
        }

        if (dare >= 0) {
            KeyCntClear(sindv_pp->keyCnt);
            tapReqGroupTapClear(Pcode2Pindex(sindv_pp->plycode));

            sindv_pp->keyCntCom = 0;
            sindv_pp->status = SCS_USE;
            sindv_pp->plycode = PR_BIT(i);
            sindv_pp->global_ply = &gply_pp[dare];
            sindv_pp->top_scr_ctrlpp = sdat_pp->scr_pp->scr_ctrl_pp;

            sindv_pp->current_scrrec_pp = ScrCtrlCurrentSearch(sindv_pp, global_data.draw_tbl_top, 0);
            sindv_pp->useLine = global_data.draw_tbl_top;

            sindv_pp->global_ply->exam_tbl_up = 0;
            sindv_pp->global_ply->exam_tbl_dw = 0;

            exam_tbl_updownInit(sindv_pp);

            for (j = 0; j < PR_ARRAYSIZE(sindv_pp->sjob); j++) {
                sindv_pp->sjob[j] = -1;
            }

            sindv_pp->tapset_pos = -1;
        }
    }
}

void ScrCtrlExamClear(SCR_EXAM_STR *sexam_pp) {
    int i;

    sexam_pp->exam_enum  = EXAM_NONE;
    sexam_pp->exam_start = -1;
    sexam_pp->exam_do    = EXAM_DO_NON;

    for (i = 0; i < PR_ARRAYSIZE(sexam_pp->scr_exam_job); i++) {
        sexam_pp->scr_exam_job[i].goto_time = -1;
        sexam_pp->scr_exam_job[i].goto_line = SCRLINE_NODATA;
        sexam_pp->scr_exam_job[i].goto_job_time = -1;
        sexam_pp->scr_exam_job[i].goto_job = -1;
    }
}

void ScrCtrlExamClearIndv(SCR_EXAM_STR *sexam_pp) {
    sexam_pp->exam_do = EXAM_DO_NON;
    sexam_pp->exam_start = -1;
}

int ScrCtrlIndvNextTime(SCORE_INDV_STR *sindv_pp, int Ncnt) {
    SCRREC *cur_pp = sindv_pp->current_scrrec_pp;

    while (1) {
        if (cur_pp->job == SCRRJ_ENDJOB) {
            return cur_pp->data;
        }
        if (cur_pp->job == SCRRJ_ENDGAME) {
            return cur_pp->data;
        }

        if (cur_pp->job == SCRRJ_PLY) {
            if (--Ncnt == 0) {
                return cur_pp->data;
            }
        }

        cur_pp++;
    }
}

int ScrCtrlIndvNextReadLine(SCORE_INDV_STR *sindv_pp, int ckf) {
    int tmp_tapstr_pos, tmp_tapset_pos;
    int cktime;

    if (sindv_pp->scrdat_pp == NULL) {
        return 0;
    }

    tmp_tapstr_pos = global_data.tapLevel;
    tmp_tapset_pos = sindv_pp->tapset_pos;

    cktime = sindv_pp->scrdat_pp->tapstr[tmp_tapstr_pos].tapset_pp[tmp_tapset_pos].taptimeStart;

    while (1) {
        tmp_tapset_pos++;

        if (tmp_tapset_pos >= sindv_pp->scrdat_pp->tapstr[tmp_tapstr_pos].tapset_size) {
            if (ckf == 0) {
                sindv_pp->tapset_pos = -1;
            }

            return 1;
        }

        if (ckf == 2) {
            if (sindv_pp->scrdat_pp->tapstr[tmp_tapstr_pos].tapset_pp[tmp_tapset_pos].taptimeStart >= cktime) {
                return 0;
            }
        } else {
            if (sindv_pp->scrdat_pp->tapstr[tmp_tapstr_pos].tapset_pp[tmp_tapset_pos].player_code == sindv_pp->plycode) {
                if (ckf == 0) {
                    sindv_pp->tapset_pos = tmp_tapset_pos;
                }

                return 0;
            }
        }
    }
}

int getLvlTblRand(TAPLVL_DAT *taplvl_dat_pp) {
    int rand_tmp;
    int ret;
    int i;
    int check;

    ret = 0;
    rand_tmp = randMakeMax(100);
    check = 0;

    for (i = 0; i < 17; i++, ret++) {
        check += taplvl_dat_pp->per[i];
        if (check > rand_tmp) {
            break;
        }
    }

    return ret;
}

int tapLevelChangeSub(void) {
    int         add_move;
    TAPLVL_STR *taplvl_str_pp;
    int         lvl_num;

    if (global_data.tapLevelCtrl != LM_AUTO) {
        return global_data.tapLevel;
    }
    if (score_str.stdat_dat_pp->taplvl_str_pp == NULL) {
        return global_data.tapLevel;
    }

    lvl_num = global_data.roundL;
    add_move = global_data.tap_ctrl_level;

    if (global_data.play_table_modeL == PLAY_TABLE_EASY) {
        lvl_num = lvl_num + TRND_MAX;
    }

    taplvl_str_pp = &score_str.stdat_dat_pp->taplvl_str_pp[lvl_num];
    add_move = getLvlTblRand(&taplvl_str_pp->taplvl_dat[add_move]);

    global_data.tapLevel = add_move;
    return global_data.tapLevel;
}

void tapLevelChange(SCORE_INDV_STR *sindv_pp) {
    int add_move;
    int old_num;

    int tmp_lv;
    int tmp_hklv;

    if (!(sindv_pp->global_ply->flags & GPLAY_TBLCNG_REQ)) {
        return;
    }

    tmp_lv = exam_tbl_updownChange(sindv_pp, global_data.tap_ctrl_level, global_data.roundL, (RANK_LEVEL2DISP_LEVEL(sindv_pp->global_ply->rank_level) == DLVL_COOL));

    sindv_pp->global_ply->exam_tbl_up = 0;
    sindv_pp->global_ply->exam_tbl_dw = 0;

    exam_tbl_updownInit(sindv_pp);

    printf("----- LEVEL CHANGE ----\n");
    printf(" CTRL LEVEL before[%d]\n", global_data.tap_ctrl_level);

    add_move = tmp_lv + global_data.tap_ctrl_level;

    if (add_move < 0) {
        add_move = TCT_LV00;
    }
    if (add_move > 15) {
        add_move = TCT_LV15;
    }

    global_data.tap_ctrl_level = add_move;

    printf("            after [%d]\n\n", add_move);
    printf(" PLAY LEVEL before[%d]\n", global_data.tapLevel);

    old_num  = global_data.tapLevel;
    add_move = tapLevelChangeSub();

    if (global_data.demo_flagL != DEMOF_REPLAY) {
        if (global_data.tapLevelCtrl == LM_AUTO) {
            if (sindv_pp->scrdat_pp != NULL) {
                tmp_hklv = inCmnHook2GameCheck(sindv_pp->scrdat_pp->sndrec_num);
                if (tmp_hklv >= 0) {
                    add_move = tmp_hklv;
                    global_data.tapLevel = tmp_hklv;
                }
            }
        }

        /* Stage 8 specific logic */
        if (global_data.play_step == PSTEP_GAME && global_data.play_stageL == 8) {
            if (sindv_pp->scrdat_pp != NULL) {
                if (sindv_pp->scrdat_pp->sndrec_num > 23 &&
                    sindv_pp->scrdat_pp->sndrec_num < 27) {
                    add_move = old_num;
                    printf("st8 sp same num:%d sndLine:%d\n", old_num, sindv_pp->scrdat_pp->sndrec_num);
                }
            } else {
                add_move = old_num;
                printf("st8 sp same num:%d\n", old_num);
            }
        }

        mccReqLvlSet(add_move);
    } else {
        add_move = mccReqLvlGet();
    }

    global_data.tapLevel = add_move;

    if (old_num != add_move) {
        selectIndvTapResetPlay(1);
    }

    printf("            after [%d]\n\n", add_move);
}

void ScrCtrlIndvNextRead(SCORE_INDV_STR *sindv_pp, int tap_res_f) {
    int endf;
    int j;

    sindv_pp->keyCntCom = 0;
    sindv_pp->scr_tap_memory_cnt = 0;
    sindv_pp->scr_tap_vib_on = 0;
    sindv_pp->cansel_flag = FALSE;

    if (tap_res_f) {
        KeyCntClear(sindv_pp->keyCnt);
        tapReqGroupTapClear(Pcode2Pindex(sindv_pp->plycode));
    }

    ScrCtrlExamClearIndv(&sindv_pp->scr_exam_str);

    if (!(sindv_pp->status & SCS_USE)) {
        sindv_pp->scrdat_pp = NULL;
        sindv_pp->sndId = -1;

        for (j = 0; j < SCRSUBJ_MAX; j++) {
            sindv_pp->sjob[j] = -1;
        }

        ScrCtrlExamClear(&sindv_pp->scr_exam_str);
        return;
    }

    if (sindv_pp->tapset_pos != -1) {
        if (!ScrCtrlIndvNextReadLine(sindv_pp, 0)) {
            return;
        }
    }

    ScrCtrlExamClear(&sindv_pp->scr_exam_str);

    sindv_pp->scrdat_pp = 0;
    sindv_pp->sndId = -1;

    for (j = 0; j < SCRSUBJ_MAX; j++) {
        sindv_pp->sjob[j] = -1;
    }

    while (1) {
        if (sindv_pp->current_scrrec_pp->job == SCRRJ_ENDJOB) {
            sindv_pp->status |= SCS_KILL_REQ;
            sindv_pp->current_time = sindv_pp->current_scrrec_pp->data;
            return;
        }
    
        if (sindv_pp->current_scrrec_pp->job == SCRRJ_ENDGAME) {
            sindv_pp->status |= SCS_END_REQ;
            sindv_pp->current_time = sindv_pp->current_scrrec_pp->data;
            return;
        }
    
        if (sindv_pp->current_scrrec_pp->job == SCRRJ_PLY && (sindv_pp->current_scrrec_pp->sub & sindv_pp->plycode) != 0) {
            sindv_pp->current_time = sindv_pp->current_scrrec_pp->data;
            sindv_pp->cursor_num = sindv_pp->current_scrrec_pp->jobd1;
            sindv_pp->tap_follow_enum = TAP_FOLLOW_NONE;
            sindv_pp->current_scrrec_pp++;
            break;
        }

        sindv_pp->current_scrrec_pp++;
    }

    endf = FALSE;

    while (1) {
        short Rsub     = sindv_pp->current_scrrec_pp->sub;
        int   Rdata    = sindv_pp->current_scrrec_pp->data;
        int   sj_data1 = sindv_pp->current_scrrec_pp->jobd1;
        int   sj_data2 = sindv_pp->current_scrrec_pp->jobd2;

        switch (sindv_pp->current_scrrec_pp->job) {
        case SCRRJ_SCR:
            sindv_pp->tap_follow_enum = Rsub;
            sindv_pp->scrdat_pp = (SCRDAT*)Rdata;

            if (sindv_pp->scrdat_pp->sndrec_num == 0) {
                sindv_pp->sndId = -1;
                break;
            }

            sindv_pp->sndId = ScrTapDbuffSet(&score_str.stdat_dat_pp->scr_pp->sndrec_pp[sindv_pp->scrdat_pp->sndrec_num]);
            break;
        case SCRRJ_EXAM:
            sindv_pp->scr_exam_str.exam_enum = Rsub;
            sindv_pp->scr_exam_str.vsPlayer = Rdata;
            sindv_pp->scr_exam_str.exam_start = 0;
            break;
        case SCRRJ_UP_LINE:
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_UP].goto_time = Rdata;
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_UP].goto_line = Rsub;
            break;
        case SCRRJ_UP_JOB:
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_UP].goto_job_time = Rdata;
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_UP].goto_job = Rsub;
            break;
        case SCRRJ_DOWN_LINE:
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_DOWN].goto_time = Rdata;
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_DOWN].goto_line = Rsub;
            break;
        case SCRRJ_DOWN_JOB:
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_DOWN].goto_job_time = Rdata;
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_DOWN].goto_job = Rsub;
            break;
        case SCRRJ_ADD_JOB:
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_ADD].goto_line = sj_data1;
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_ADD].goto_job_time = Rdata;
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_ADD].goto_job = Rsub;
            break;
        case SCRRJ_SUB_JOB:
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_SUB].goto_line = sj_data1;
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_SUB].goto_job_time = Rdata;
            sindv_pp->scr_exam_str.scr_exam_job[SCREX_SUB].goto_job = Rsub;
            break;
        case SCRRJ_LINE:
            sindv_pp->scr_exam_str.scr_exam_job[sj_data2].goto_time = Rdata;
            sindv_pp->scr_exam_str.scr_exam_job[sj_data2].goto_line = Rsub;
            break;
        case SCRRJ_JOB:
            sindv_pp->scr_exam_str.scr_exam_job[sj_data2].goto_job_time = Rdata;
            sindv_pp->scr_exam_str.scr_exam_job[sj_data2].goto_job = Rsub;
            break;
        case SCRRJ_SUBJOB:
            if (Rsub == SCRSUBJ_SPU_ON) {
                int xx;
                int yari_ff = FALSE;

                for (xx = 0; xx < 4; xx++) {
                    if (sindv_pp->sjob[Rsub + xx] == -1) {
                        sindv_pp->sjob[Rsub + xx] = Rdata + xx;
                        sindv_pp->sjob_data[Rsub + xx][0] = sj_data1;
                        sindv_pp->sjob_data[Rsub + xx][1] = sj_data2;
                        yari_ff = TRUE;
                        break;
                    }
                }

                if (!yari_ff) {
                    printf("ERROR! SUBJOB_NO_ID OVER!\n");
                }

                break;
            }

            sindv_pp->sjob[Rsub] = Rdata;
            sindv_pp->sjob_data[Rsub][0] = sj_data1;
            sindv_pp->sjob_data[Rsub][1] = sj_data2;
            break;
        case SCRRJ_PLY:
        case SCRRJ_ENDJOB:
        case SCRRJ_ENDGAME:
            endf = TRUE;
            break;
        case SCRRJ_MSG_DISP:
            break;
        }

        if (endf) {
            break;
        }

        sindv_pp->current_scrrec_pp++;
    }

    if (sindv_pp->global_ply == NULL) {
        printf("global_play is NULL [%d]\n", sindv_pp->plycode);
    } else {
        tapLevelChange(sindv_pp);
    }

    ScrCtrlIndvNextReadLine(sindv_pp, 0);
}

void intIndvStatusSet(SCORE_INDV_STR *sindv_pp, u_int CKF, u_int STF, u_int UNF) {
    if (sindv_pp->status & CKF) {
        sindv_pp->status |= STF;
        sindv_pp->status &= ~UNF;
    }
}

void allIndvNextContinue(void) {
    int             i;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        intIndvStatusSet(sindv_pp, SCS_PAUSE, SCS_PAUSE_END, SCS_PAUSE);
    }
}

void allIndvGoContinue(void) {
    int             i;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        intIndvStatusSet(sindv_pp, SCS_PAUSE_END, 0, SCS_PAUSE_END);
    }
}

void otherIndvPause(int num) {
    int             i;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        if (i != num) {
            intIndvStatusSet(sindv_pp, SCS_USE, SCS_PAUSE, 0);
        }
    }
}

void otherIndvTapReset(int num) {
    int             i;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        if (i != num) {
            if (sindv_pp->status & SCS_USE) {
                KeyCntClear(sindv_pp->keyCnt);

                sindv_pp->keyCntCom = 0;
                sindv_pp->scr_tap_memory_cnt = 0;
                sindv_pp->scr_tap_vib_on = 0;
                sindv_pp->cansel_flag = FALSE;

                tapReqGroupTapClear(Pcode2Pindex(sindv_pp->plycode));
            }
        }
    }
}

void selectIndvTapResetPlay(int num) {
    SCORE_INDV_STR *sindv_pp = &score_indv_str[num];
    TAPSET         *tapset_pp;

    if (!(sindv_pp->status & SCS_USE)) {
        return;
    }

    tapset_pp = IndvGetTapSetAdrs(sindv_pp);
    if (tapset_pp == NULL || tapset_pp->coolup == -1) {
        return;
    }

    KeyCntClear(sindv_pp->keyCnt);

    sindv_pp->keyCntCom = 0;
    sindv_pp->scr_tap_memory_cnt = 0;
    sindv_pp->scr_tap_vib_on = 0;
    sindv_pp->cansel_flag = FALSE;

    tapReqGroupTapClear(Pcode2Pindex(sindv_pp->plycode));
}

void IndivMoveChange(SCORE_INDV_STR *sindv_pp, int goto_time, SCRLINE_ENUM goto_line) {
    int j;

    KeyCntClear(sindv_pp->keyCnt);
    tapReqGroupTapClear(Pcode2Pindex(sindv_pp->plycode));

    sindv_pp->top_scr_ctrlpp = score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp;
    sindv_pp->current_scrrec_pp = ScrCtrlCurrentSearch(sindv_pp, goto_line, goto_time);

    for (j = 0; j < PR_ARRAYSIZE(sindv_pp->sjob); j++) {
        sindv_pp->sjob[j] = -1;
    }

    sindv_pp->useLine = goto_line;
    sindv_pp->tapset_pos = -1;

    ScrCtrlIndvNextRead(sindv_pp, 1);
}

void useIndevAllMove(int goto_time, SCRLINE_ENUM goto_line) {
    int             i;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        if (sindv_pp->status & SCS_USE) {
            sindv_pp->plycode = PR_BIT(i);
            sindv_pp->status  = SCS_USE;

            IndivMoveChange(sindv_pp, goto_time, goto_line);
        }
    }
}

static int useIndevCodeGet(void) {
    int             i;
    int             ret      = 0;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        if (sindv_pp->status & SCS_USE) {
            ret |= PR_BIT(i);
        }
    }

    return ret;
}

static int targetTimeGet(int line, int time, int codeAll) {
    SCRREC *scrrec_pp;
    int     max;
    int     i;
    int     ret;
    int     pcode_tmp;

    ret       = 0;
    pcode_tmp = 0;

    max       = score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[line].scrrec_num;
    scrrec_pp = score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[line].scrrec_pp;

    for (i = 0; i < max; i++, scrrec_pp++) {
        if (scrrec_pp->job == SCRRJ_ENDJOB || scrrec_pp->job == SCRRJ_ENDGAME) {
            break;
        }

        if (scrrec_pp->job == SCRRJ_PLY) {
            pcode_tmp = scrrec_pp->sub & codeAll;
            if (scrrec_pp->data < time) {
                pcode_tmp = 0;
            }
        }

        if (scrrec_pp->job == SCRRJ_SUBJOB && scrrec_pp->sub == SCRSUBJ_DRAW_CHANGE) {
            if (pcode_tmp != 0) {
                ret = scrrec_pp->data;
                break;
            }
        }
    }

    return ret;
}

void useIndevSndKill(void) {
    int             i;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        if (sindv_pp->status & SCS_USE) {
            TapCt(TAPCT_TAPSTOP, i, TAPCT_NONE);
        }
    }
}

void useAllClearKeySnd(void) {
    int i;

    for (i = 0; i < 12; i++) {
        TapCt(TAPCT_TAPSTOP, i, TAPCT_NONE);
    }
}

int useIndevSndKillOther(int num) {
    int             i;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        if (i != num) {
            if (sindv_pp->status & SCS_USE) {
                TapCt(TAPCT_TAPSTOP, i, TAPCT_NONE);
            }
        }
    }

    return 0;
}

int TapKeyCheckNum(TAPSET *tapset_pp, int keyId, int ng_f) {
    int     i;
    int     ret;

    int     tmp_size;
    TAPDAT *tmp_tapdat_pp;

    ret = 0;

    if (ng_f) {
        tmp_size      = tapset_pp->tapdatNG_size;
        tmp_tapdat_pp = tapset_pp->tapdatNG_pp;
    } else {
        tmp_size      = tapset_pp->tapdat_size;
        tmp_tapdat_pp = tapset_pp->tapdat_pp;
    }

    for (i = 0; i < tmp_size; i++) {
        if (tmp_tapdat_pp[i].KeyIndex == keyId) {
            ret++;
        }
    }

    return ret;
}

TAPDAT* TapKeyGetDatPP(TAPSET *tapset_pp, int keyId, int keyCnt, int ng_f, u_char *keyNumSave) {
    int     i;
    int     cnt;
    int     tmp_size;
    TAPDAT *tmp_tapdat_pp;

    cnt = 0;
    *keyNumSave = 0;

    if (ng_f) {
        tmp_size      = tapset_pp->tapdatNG_size;
        tmp_tapdat_pp = tapset_pp->tapdatNG_pp;
    } else {
        tmp_size      = tapset_pp->tapdat_size;
        tmp_tapdat_pp = tapset_pp->tapdat_pp;
    }

    for (i = 0; i < tmp_size; i++) {
        if (tmp_tapdat_pp[i].KeyIndex == keyId) {
            if (cnt == keyCnt) {
                *keyNumSave = i;
                return &tmp_tapdat_pp[i];
            }

            cnt++;
        }
    }

    return NULL;
}

void tapReqGroupInit(void) {
    WorkClear(tap_groupe_str, sizeof(tap_groupe_str));
}

void tapReqGroupTapClear(PLAYER_INDEX pindex) {
    WorkClear(&tap_groupe_str[pindex], sizeof(TAP_GROUPE_STR));
}

void tapReqGroup(TAPCT *tapct_pp, PLAYER_INDEX pindex, int sndId, u_char *tappress_pp) {
    if (pindex >= PINDEX_MAX) {
        printf("tap group PINDEX over[%d]\n", pindex);
    }

    tap_groupe_str[pindex].timer       = 0;
    tap_groupe_str[pindex].sndId       = sndId;
    tap_groupe_str[pindex].tapct_pp    = tapct_pp;
    tap_groupe_str[pindex].tappress_pp = tappress_pp;
}

void tapReqGroupPoll(void) {
    int             i, j;
    int             end_frameT;
    TAP_GROUPE_STR *tgs_pp = tap_groupe_str;

    for (i = 0; i < PR_ARRAYSIZE(tap_groupe_str); i++, tgs_pp++) {
        end_frameT = 0;

        if (tgs_pp->tapct_pp == NULL) {
            continue;
        }

        for (j = 0; j < 4; j++) {
            if (tgs_pp->tapct_pp[j].frame == -1) {
                continue;
            }

            if (tgs_pp->tapct_pp[j].frame == tgs_pp->timer) {
                if (tgs_pp->tapct_pp[j].actor != -1) {
                    DrawTapReqTbl(tgs_pp->tapct_pp[j].actor, i, tgs_pp->tappress_pp);
                }
                if (tgs_pp->tapct_pp[j].sound != -1) {
                    ScrTapReq(tgs_pp->sndId, i, tgs_pp->tapct_pp[j].sound);
                }
            } else {
                end_frameT = (tgs_pp->timer < tgs_pp->tapct_pp[j].frame) ? 1 : end_frameT;
            }
        }

        if (end_frameT) {
            tgs_pp->timer++;
        } else {
            WorkClear(tgs_pp, sizeof(*tgs_pp));
        }
    }
}

void tapEventCheck(SCORE_INDV_STR *sindv_pp, int Ttime, int Ctime, int num) {
    int padType;

    int onKeyTime;
    int onKeyOut;

    TAPSET      *tapset_pp;
    u_char      *tappress_pp;
    TAPDAT      *tapdat_pp;

    PLAYER_ENUM  player_enum_tmp;
    u_char       othNumTmp;

    onKeyOut = 0;

    tappress_pp = NULL;
    tapdat_pp   = NULL;

    othNumTmp = 0;

    if (sindv_pp->scrdat_pp == NULL) {
        return;
    }

    if (global_data.play_step == PSTEP_HOOK) {
        if (sindv_pp->global_ply != NULL) {
            if (sindv_pp->global_ply->pad_type == PAD_1CON) {
                if (pad[0].one & SCE_PADstart) {
                    sindv_pp->cansel_flag = TRUE;
                    sindv_pp->scr_exam_str.exam_enum = EXAM_CANCEL;
                    return;
                }
            }
        }
    }

    if (sindv_pp->scr_exam_str.exam_enum == EXAM_CANCEL) {
        int paddata_tmp = 0;

        if (sindv_pp->global_ply != NULL) {
            padType = sindv_pp->global_ply->pad_type;

            if (global_data.play_step == PSTEP_BONUS) {
                padType = PAD_1CON;
            }
            if (global_data.play_step == PSTEP_VS && global_data.demo_flagL == DEMOF_REPLAY) {
                padType = PAD_1CON;
            }

            if (padType == PAD_1CON || padType == PAD_2CON) {
                paddata_tmp = pad[padType].one;
            }

            if (GetKeyCode2Index(paddata_tmp) != KiNO && !sindv_pp->cansel_flag) {
                if (global_data.play_step == PSTEP_BONUS) {
                    ScrTapReq(SCR_TAP_COMMON, 3, 0);
                } else {
                    ScrTapReq(SCR_TAP_COMMON, 3, 2);
                }

                sindv_pp->cansel_flag = TRUE;
            }
        }

        return;
    }

    padType = PAD_DEMO;

    if (sindv_pp->global_ply != NULL) {
        padType = sindv_pp->global_ply->pad_type;
    }

    {
        int tapset_pos_tmp = sindv_pp->tapset_pos;
        if (tapset_pos_tmp == -1) {
            ScrCtrlIndvNextReadLine(sindv_pp, 0);
            if (tapset_pos_tmp == sindv_pp->tapset_pos) {
                return;
            }
        }
    }

    if (sindv_pp->tapset_pos >= sindv_pp->scrdat_pp->tapstr[global_data.tapLevel].tapset_size) {
        printf("TAP !! tap set pos is OVER!!\n");
        return;
    }

    tapset_pp = &sindv_pp->scrdat_pp->tapstr[global_data.tapLevel].tapset_pp[sindv_pp->tapset_pos];

    if (Ctime < (tapset_pp->taptimeStart - 24)) {
        return;
    }

    player_enum_tmp = NON_PLAYER_NUM;
    if (sindv_pp->plycode == PCODE_PARA) {
        player_enum_tmp = PARA_PLAYER_NUM;
    }
    if (sindv_pp->plycode == PCODE_TEACHER) {
        player_enum_tmp = TEACHER_PLAYER_NUM;
    }
    if (sindv_pp->plycode == PCODE_BOXY) {
        player_enum_tmp = BOXY_PLAYER_NUM;
    }

    if (sindv_pp->scr_tap_vib_on == 0) {
        if (tapset_pp->tapscode == TAPSCODE_QUESTION) {
            vsTapdatSetMoto(sindv_pp);
        } else {
            if (tapset_pp->tapscode != TAPSCODE_NORMAL && padType == PAD_COM) {
                commake_str_cnt = computerMaking(commake_str, PR_ARRAYSIZE(commake_str), vs_tapdat_work, vs_tapdat_work_cnt, tapset_pp, global_data.level_vs_enumL);
            }
        }

        sindv_pp->scr_tap_vib_on++;
    }

    onKeyTime = -1;

    switch (padType) {
    case PAD_DEMO:
        if (sindv_pp->tap_follow_enum == TAP_FOLLOW_LOAD) {
            SCR_TAP_MEMORY *flt_mem_pp = followTapLoad(sindv_pp->keyCntCom, Ctime - tapset_pp->taptimeStart);
            if (flt_mem_pp != NULL) {
                tapdat_pp = &tapset_pp->tapdat_pp[flt_mem_pp->othNum];
                othNumTmp = flt_mem_pp->othNum;
                sindv_pp->keyCntCom++;
                onKeyOut = TRUE;                
                onKeyTime = flt_mem_pp->ofs_frame + tapset_pp->taptimeStart + sindv_pp->current_time;
            }

            break;
        }

        if (tapset_pp->tapdat_size > sindv_pp->keyCntCom) {
            tapdat_pp = &tapset_pp->tapdat_pp[sindv_pp->keyCntCom];

            if (Ctime < (tapdat_pp->time + tapset_pp->taptimeStart)) {
                break;
            }

            othNumTmp = sindv_pp->keyCntCom;

            sindv_pp->keyCntCom++;            
            onKeyOut = TRUE;
            onKeyTime = tapdat_pp->time + tapset_pp->taptimeStart + sindv_pp->current_time;
        }

        break;
    case PAD_REPLAY: {
        int keyId;
        int keyId_x;

        short paddata;

        short key_num;
        short key_max;

        int time_tmp;

        paddata = mccReqTapGet(Ttime, sindv_pp->useLine, &time_tmp, player_enum_tmp);

        if (paddata == 0) {
            break;
        }

        if (paddata & SCE_PADLleft) {
            KeyCntClear(sindv_pp->keyCnt);
        }

        keyId = GetKeyCode2Index(paddata);
        keyId_x = keyId;

        if (keyId == KiNO) {
            break;
        }

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            keyId_x = KiTR;
        }

        if (!(paddata & SCE_PADLright)) {
            sindv_pp->keyCnt[keyId_x]++;
        }

        key_num = sindv_pp->keyCnt[keyId_x];
        if (key_num < 0) {
            key_num = 0;
        }

        key_max = TapKeyCheckNum(tapset_pp, keyId_x, FALSE);

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            key_max = tapset_pp->tapdat_size;
        }

        if (key_max == 0) {
            key_max = TapKeyCheckNum(tapset_pp, keyId_x, TRUE);

            if (key_max == 0) {
                break;
            }

            onKeyOut = FALSE;
        } else {
            onKeyOut = TRUE;
        }

        onKeyTime = time_tmp;
        key_num %= key_max;

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            othNumTmp = key_num;
            tapdat_pp = &tapset_pp->tapdat_pp[key_num];
        } else {
            tapdat_pp = TapKeyGetDatPP(tapset_pp, keyId_x, key_num, onKeyOut ^ 1, &othNumTmp);
        }

        break;
    }
    case PAD_COM: {
        COMMAKE_STR *com_pp;
        int          keyId_x;
        short        key_num;
        short        key_max;

        if (commake_str_cnt <= sindv_pp->keyCntCom) {
            break;
        }

        com_pp = &commake_str[sindv_pp->keyCntCom];

        if (Ctime < (com_pp->time + tapset_pp->taptimeStart)) {
            break;
        }

        onKeyTime = com_pp->time + tapset_pp->taptimeStart + sindv_pp->current_time;

        sindv_pp->keyCntCom++;

        keyId_x = com_pp->KeyIndex;

        if (keyId_x == KiNO) {
            break;
        }

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            keyId_x = KiTR;
        }

        sindv_pp->keyCnt[keyId_x]++;

        mccReqTapSet(Ttime, sindv_pp->useLine, keyId_x, player_enum_tmp);

        key_num = sindv_pp->keyCnt[keyId_x];
        if (key_num < 0) {
            key_num = 0;
        }

        key_max = TapKeyCheckNum(tapset_pp, keyId_x, FALSE);

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            key_max = tapset_pp->tapdat_size;
        }

        if (key_max == 0) {
            key_max = TapKeyCheckNum(tapset_pp, keyId_x, TRUE);

            if (key_max == 0) {
                break;
            }

            onKeyOut = FALSE;
        } else {
            onKeyOut = TRUE;
        }

        key_num %= key_max;

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            othNumTmp = key_num;
            tapdat_pp = &tapset_pp->tapdat_pp[key_num];
        } else {
            tapdat_pp = TapKeyGetDatPP(tapset_pp, keyId_x, key_num, onKeyOut ^ 1, &othNumTmp);
        }

        break;
    }
    case PAD_1CON:
    case PAD_2CON: {
        int keyId;
        int keyId_x;

        u_short paddata;
        u_short paddata_one;

        short key_num;
        short key_max;

        if (padType != PAD_1CON && padType != PAD_2CON) {
            onKeyTime = -1;
            break;
        }

        paddata = pad[padType].shot;
        paddata_one = pad[padType].one;

        if (sindv_pp->scr_tap_vib_on < 3) {
            if (game_status.vibration == VIBRATION_ON) {
                pad[padType].padvib[0] = 1;
            }

            sindv_pp->scr_tap_vib_on++;
        }

        if (paddata & SCE_PADLleft) {
            KeyCntClear(sindv_pp->keyCnt);
            mccReqTapResetSet(player_enum_tmp);
        }

        keyId = GetKeyCode2Index(paddata_one);
        if (keyId == KiNO) {
            break;
        }

        keyId_x = keyId;

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            keyId_x = KiTR;
        }

        if (paddata & SCE_PADLright) {
            mccReqTapHoldSet(player_enum_tmp);
        } else {
            sindv_pp->keyCnt[keyId_x]++;
        }

        mccReqTapSet(Ttime, sindv_pp->useLine, keyId_x, player_enum_tmp);

        key_num = sindv_pp->keyCnt[keyId_x];
        if (key_num < 0) {
            key_num = 0;
        }

        key_max = TapKeyCheckNum(tapset_pp, keyId_x, FALSE);

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            key_max = tapset_pp->tapdat_size;
        }

        if (key_max == 0) {
            key_max = TapKeyCheckNum(tapset_pp, keyId_x, TRUE);

            if (key_max == 0) {
                break;
            }

            onKeyOut = FALSE;
        } else {
            onKeyOut = TRUE;
        }

        onKeyTime = Ttime;
        key_num %= key_max;

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            othNumTmp = key_num;
            tapdat_pp = &tapset_pp->tapdat_pp[key_num];
        } else {
            tapdat_pp = TapKeyGetDatPP(tapset_pp, keyId_x, key_num, onKeyOut ^ 1, &othNumTmp);
        }

        if (GetIndex2PressId(keyId) >= 0) {
            tappress_pp = &pad[padType].press[GetIndex2PressId(keyId)];
        }

        break;
    }
    case PAD_UNUSE:
        onKeyTime = -1;
        break;
    }

    if (onKeyTime < 0) {
        return;
    }

    tapReqGroup(tapdat_pp->tapct, Pcode2Pindex(sindv_pp->plycode), (onKeyOut) ? sindv_pp->sndId : -1, tappress_pp);

    if (tapdat_pp->KeyIndex != KiNO) {
        SCR_TAP_MEMORY *mkey_pp = &sindv_pp->scr_tap_memory[sindv_pp->scr_tap_memory_cnt];

        mkey_pp->key       = tapdat_pp->KeyIndex;
        mkey_pp->othNum    = othNumTmp;
        mkey_pp->ofs_frame = onKeyTime - tapset_pp->taptimeStart - sindv_pp->current_time;
        mkey_pp->onKey     = onKeyOut;

        sindv_pp->scr_tap_memory_cnt++;
        if (sindv_pp->scr_tap_memory_cnt > 255) {
            sindv_pp->scr_tap_memory_cnt = 255;
            printf(" KEY STACK OVER!!\n");
        }

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            mkey_pp->key = KiTR;
        }

        {
            int local_map;
            int xx;

            local_map = MapNormalNumGet(mkey_pp->ofs_frame + 96);

            mkey_pp->othOn = FALSE;

            for (xx = 0; xx < tapset_pp->tapdat_size; xx++) {
                int map = MapNormalNumGet(tapset_pp->tapdat_pp[xx].time + 96);
                if (local_map == map) {
                    if (global_data.play_typeL == PLAY_TYPE_ONE) {
                        mkey_pp->othOn = TRUE;
                        break;
                    }

                    if (tapset_pp->tapdat_pp[xx].KeyIndex == mkey_pp->key) {
                        mkey_pp->othOn = TRUE;
                        break;
                    }
                }
            }

            if (mkey_pp->othOn) {
                MbarHookUseOK();
            } else {
                MbarHookUseNG();
            }
    
        }
    }
}

static int otehon_all_make(EXAM_CHECK *ec_pp) {
    int i;
    int ret = 0;

    if (ec_pp->tapset_pp == NULL) {
        return 0;
    }

    for (i = 0; i < ec_pp->tapset_pp->tapdat_size; i++) {
        ret |= GetIndex2KeyCode(ec_pp->tapset_pp->tapdat_pp[i].KeyIndex);
    }

    return ret;
}

static int treateTimeChange(int time) {
    int thnum_ofs = thnum_get(24, CK_TH_NORMAL);
    int thnum_now = thnum_get((time + 96) / 4, CK_TH_NORMAL);

    if ((thnum_now % 2) != 0) {
        return -1;
    }
    if (thnum_now < thnum_ofs) {
        return -1;
    }

    thnum_now -= thnum_ofs;
    thnum_now /= 2;
    return thnum_now * 24;
}

static int thnum_get(int p96_num, CK_TH_ENUM ckth) {
    u_int thnum_data;
    int   ck_bit, ck_dat;
    int   ret_cnt;
    int   i;

    ck_bit = -1;
    ret_cnt = 0;
    thnum_data = thnum_tbl[ckth];

    for (i = 0; i <= p96_num; i++) {
        ck_dat = (thnum_data >> (23 - (i % 24))) & 1;

        if (ck_bit < 0) {
            if (ck_dat == 0) {
                ret_cnt = 1;
            }
        } else if (ck_dat != ck_bit) {
            ret_cnt++;
        }

        ck_bit = ck_dat;
    }

    return ret_cnt;
}

static int MapNormalNumGet(int time) {
    return thnum_get(time / 4, CK_TH_NORMAL);
}

static void on_th_make(EXAM_CHECK *ec_pp, CK_TH_ENUM ckth) {
    int     i;
    int     frame;
    int     ofsT, ofsTend;
    int     p96_num;
    TAPDAT *tapdat_pp;
    int     tapdat_cnt;

    if (ec_pp->ckth == ckth) {
        return;
    }

    ec_pp->ckth = ckth;

    ofsT = ec_pp->ofs_tick % 96;
    ofsT += 96;

    ofsTend = ofsT + ec_pp->tapset_pp->taptimeEnd - ec_pp->tapset_pp->taptimeStart;

    for (i = 0; i < ec_pp->ted_num; i++) {
        frame   = ec_pp->stm_pp[i].ofs_frame;
        p96_num = (frame + ofsT) / 4;

        ec_pp->ted[i].p96_num = p96_num;
        ec_pp->ted[i].th_num = thnum_get(p96_num, ckth);
        ec_pp->ted[i].key = ec_pp->stm_pp[i].key;
    }

    ec_pp->top_ofs = thnum_get((ofsT - 24) / 4, ckth);
    ec_pp->end_ofs = thnum_get(ofsTend / 4, ckth);

    if (ec_pp->tapset_pp == NULL) {
        printf("score line over!!\n");
        return;
    }

    ec_pp->oth_num = 0;

    tapdat_pp = ec_pp->tapset_pp->tapdat_pp;
    tapdat_cnt = ec_pp->tapset_pp->tapdat_size;

    if (ec_pp->vs_use) {
        tapdat_pp = ec_pp->vs_tapdat_pp;
        tapdat_cnt = ec_pp->vs_tapdat_cnt;
    }

    for (i = 0; i < tapdat_cnt; i++, tapdat_pp++) {
        if (tapdat_pp->KeyIndex != KiNO) {
            ec_pp->oth_num++;

            frame   = tapdat_pp->time;
            p96_num = (frame + ofsT) / 4;

            if (ckth == CK_TH_LATE) {
                p96_num += 2;
            }

            ec_pp->oth[i].p96_num = p96_num;
            ec_pp->oth[i].th_num = thnum_get(p96_num, ckth);
            ec_pp->oth[i].key = tapdat_pp->KeyIndex;
        }
    }
}

static int exh_normal_add(EXAM_CHECK *ec_pp) {
    int i;
    int ret = 0;

    for (i = 0; i < ec_pp->ted_num; i++) {
        if (!(ec_pp->ted[i].th_num & 1)) {
            if (GetIndex2KeyCode(ec_pp->ted[i].key) & ec_pp->otehon_all) {
                ret++;
            }
        }
    }

    return ret;
}

static int exh_normal_sub(EXAM_CHECK *ec_pp) {
    int i;
    int ret = 0;

    for (i = 0; i < ec_pp->ted_num; i++) {
        if (GetIndex2KeyCode(ec_pp->ted[i].key) & ec_pp->otehon_all) {
            ret -= ec_pp->ted[i].th_num & 1;
        }
    }

    return ret;
}

static int exh_nombar_sub(EXAM_CHECK *ec_pp) {
    int i;
    int ret;
    int bai, otehon;

    ret = 0;

    for (i = 0; i < ec_pp->ted_num; i++) {
        int keycode = GetIndex2KeyCode(ec_pp->ted[i].key);
        if ((keycode & ec_pp->otehon_all) == 0) {
            ret -= 1;
        }
    }

    bai    = 0;
    otehon = ec_pp->otehon_all;

    for (i = 0; i < 4u; i++) {
        bai += (otehon >> i) & 1;
    }

    if (bai == 1) {
        ret *= 3;
    }
    if (bai == 2) {
        ret *= 2;
    }

    if (bai == 5) {
        ret /= 2;
    }
    if (bai == 6) {
        ret /= 2;
    }

    return ret;
}

static int exh_mbar_key_out(EXAM_CHECK *ec_pp) {
    int ret;

    if (global_data.play_typeL == PLAY_TYPE_ONE) {
        return 0;
    }

    ret = -ec_pp->oth_num;
    if (ec_pp->ted_num != 0) {
        if (ec_pp->oth_num == 0) {
            ret = 0;
        } else if (ec_pp->oth[0].key == ec_pp->ted[0].key) {
            ret = 0;
        }
    }

    return ret;
}

static int exh_mbar_time_out(EXAM_CHECK *ec_pp) {
    int ret;

    ret = -ec_pp->oth_num;
    if (ec_pp->ted_num != 0) {
        if (ec_pp->oth_num == 0) {
            ret = 0;
        } else if (ec_pp->oth[0].th_num == ec_pp->ted[0].th_num) {
            ret = 0;
        }
    }

    return ret;
}

static int exh_mbar_num_out(EXAM_CHECK *ec_pp) {
    int ret = ec_pp->ted_num - ec_pp->oth_num;
    return (ret >= 0) ? -ret : ret;
}

static int exh_yaku(EXAM_CHECK *ec_pp, int hane_flag) {
    int i, j;
    int ofsT, ofsE;
    int ret;

    WorkClear(yaku_tmp_buf, sizeof(yaku_tmp_buf));

    ofsT = ec_pp->top_ofs / 2;
    ofsE = ec_pp->end_ofs / 2;

    for (i = 0; i < (PR_ARRAYSIZE(yaku_tmp_buf) * 2); i++) {
        if (i < ofsT || i > ofsE) {
            if ((i % 2) != 0) {
                yaku_tmp_buf[i / 2] &= 0xf0;
                yaku_tmp_buf[i / 2] |= 0x02;
            } else {
                yaku_tmp_buf[i / 2] &= 0x0f;
                yaku_tmp_buf[i / 2] |= 0x20;
            }
        }
    }

    for (i = 0; i < ec_pp->ted_num; i++) {
        int bufID = ec_pp->ted[i].th_num / 2;

        if ((ec_pp->ted[i].th_num % 2) == 0 && (GetIndex2KeyCode(ec_pp->ted[i].key) & ec_pp->otehon_all) != 0) {
            u_char setD;
            if ((bufID % 2) != 0) {
                setD = 0x01;

                if ((yaku_tmp_buf[bufID / 2] & 0x0f) != 0) {
                    setD = 0x02;
                }

                yaku_tmp_buf[bufID / 2] &= 0xf0;
                yaku_tmp_buf[bufID / 2] |= setD;
            } else {
                setD = 0x10;

                if ((yaku_tmp_buf[bufID / 2] & 0xf0) != 0) {
                    setD = 0x20;
                }

                yaku_tmp_buf[bufID / 2] &= 0x0f;
                yaku_tmp_buf[bufID / 2] |= setD;
            }
        } else {
            if ((bufID % 2) != 0) {
                yaku_tmp_buf[bufID / 2] &= 0xf0;
                yaku_tmp_buf[bufID / 2] |= 0x02;
            } else {
                yaku_tmp_buf[bufID / 2] &= 0x0f;
                yaku_tmp_buf[bufID / 2] |= 0x20;
            }
        }
    }

    {
        u_char yaku_map[4] = { 16, 17, 1,  0  };
        u_char yaku_scr[4] = { 6,  9,  15, 18 };
        u_char yaku_cnt[4] = {};
        u_char ymin, ymax;

        for (i = 0; i < PR_ARRAYSIZE(yaku_tmp_buf); i++) {
            for (j = 0; j < 4; j++) {
                if (yaku_tmp_buf[i] == yaku_map[j]) {
                    yaku_cnt[j]++;
                }
            }
        }

        if (!hane_flag) {
            ymin = 0;

            for (i = 0; i < 3; i++) {
                if (yaku_cnt[i] != 0) {
                    ymin++;
                }
            }

            if (ymin == 0 || ymin == 1) {
                yaku_cnt[3] = 0;
                yaku_cnt[2] = 0;
                yaku_cnt[1] = 0;
                yaku_cnt[0] = 0;
            }
        }

        ymax = min(min(yaku_cnt[0], yaku_cnt[1]), yaku_cnt[2]);

        if (hane_flag) {
            if (yaku_cnt[1] == 0 && yaku_cnt[2] == 0) {
                yaku_cnt[3] = 0;
                yaku_cnt[2] = 0;
                yaku_cnt[1] = 0;
                yaku_cnt[0] = 0;
            }
        }

        if (yaku_cnt[3] > ymax) {
            yaku_cnt[3] = ymax;
        }

        ret = 0;

        for (i = 0; i < 4; i++) {
            ret += yaku_cnt[i] * yaku_scr[i];
        }

        return ret;
    }
}

static int exh_yaku_original(EXAM_CHECK *ec_pp) {
    return exh_yaku(ec_pp, FALSE);
}

static int exh_yaku_hane(EXAM_CHECK *ec_pp) {
    return exh_yaku(ec_pp, TRUE);
}

static int exh_allkey_out(EXAM_CHECK *ec_pp) {
    int use_bit;
    int i;

    for (i = 0, use_bit = 0; i < ec_pp->ted_num; i++) {
        if (!(ec_pp->ted[i].th_num & 1)) {
            use_bit |= GetIndex2KeyCode(ec_pp->ted[i].key);
        }
    }

    use_bit &= ec_pp->otehon_all;

    if (use_bit == ec_pp->otehon_all) {
        return 0;
    } else {
        return -1;
    }
}

static int exh_allkey_out_nh(EXAM_CHECK *ec_pp) {
    int use_bit;
    int i;

    for (i = 0, use_bit = 0; i < ec_pp->ted_num; i++) {
        use_bit |= GetIndex2KeyCode(ec_pp->ted[i].key);
    }

    use_bit &= ec_pp->otehon_all;

    if (use_bit == ec_pp->otehon_all) {
        return 0;
    } else {
        return -1;
    }
}

static int exh_command(EXAM_CHECK *ec_pp) {
    return 0;
}

static int exh_renda_out(EXAM_CHECK *ec_pp) {
    int renda_ck;

    renda_ck = ec_pp->tapset_pp->taptimeEnd - ec_pp->tapset_pp->taptimeStart;
    renda_ck = ((renda_ck + 23) / 24);
    renda_ck += 3;

    if (ec_pp->ted_num >= renda_ck) {
        return -1;
    }

    return 0;
}

static int manemane_check_sub(EXAM_CHECK *ec_pp) {
    int mane_cnt;
    int i, j;

    mane_cnt = 0;

    for (i = 0; i < ec_pp->oth_num; i++) {
        for (j = 0; j < ec_pp->ted_num; j++) {
            if (global_data.play_typeL == PLAY_TYPE_ONE) {
                if (ec_pp->oth[i].th_num == ec_pp->ted[j].th_num) {
                    mane_cnt++;
                }
            } else {
                if (ec_pp->oth[i].th_num == ec_pp->ted[j].th_num) {
                    if (ec_pp->oth[i].key == ec_pp->ted[j].key) {
                        mane_cnt++;
                    }
                }
            }
        }
    }

    return (mane_cnt * 2) - ec_pp->ted_num;
}

static int manemane_check(EXAM_CHECK *ec_pp) {
    int mane_cnt;
    int i, j;

    mane_cnt = 0;

    for (i = 0; i < ec_pp->oth_num; i++) {
        for (j = 0; j < ec_pp->ted_num; j++) {
            if (global_data.play_typeL == PLAY_TYPE_ONE) {
                if (ec_pp->oth[i].th_num == ec_pp->ted[j].th_num) {
                    mane_cnt++;
                }
            } else {
                if (ec_pp->oth[i].th_num == ec_pp->ted[j].th_num) {
                    if (ec_pp->oth[i].key == ec_pp->ted[j].key) {
                        mane_cnt++;
                    }
                }
            }
        }
    }

    return (mane_cnt * 2) - ec_pp->ted_num - abs(ec_pp->oth_num - ec_pp->ted_num);
}

static int exh_mane(EXAM_CHECK *ec_pp) {
    int normal_point;
    int late_point;

    normal_point = manemane_check_sub(ec_pp);

    on_th_make(ec_pp, CK_TH_LATE);

    late_point = manemane_check_sub(ec_pp);

    if (late_point < normal_point) {
        late_point = normal_point;
    }

    return late_point;
}

static int exh_all_add(EXAM_CHECK *ec_pp) {
    int i;
    int total;

    total = 0;

    for (i = 0; i < 12; i++) {
        if (i != EXH_TOTAL) {
            total += ec_pp->each_point[i];
        }
    }

    if (ec_pp->each_point[EXH_ALLKEY_OUT] != 0) {
        total = 0;
    }
    if (ec_pp->each_point[EXH_RENDA_OUT] != 0) {
        total = -100;
    }

    return total;
}

static TAPSET* IndvGetTapSetAdrs(SCORE_INDV_STR *sindv_pp) {
    int id = global_data.tapLevel;
    int ln = sindv_pp->tapset_pos;

    if (ln < 0 || sindv_pp->scrdat_pp == NULL) {
        return NULL;
    }

    if (ln >= sindv_pp->scrdat_pp->tapstr[id].tapset_size) {
        return NULL;
    }

    return &sindv_pp->scrdat_pp->tapstr[id].tapset_pp[ln];
}

static int nextExamTime(void) {
    int             i;
    SCORE_INDV_STR *sindv_pp;
    TAPSET         *tapset_pp;
    int             ret;

    sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZEU(score_indv_str); i++, sindv_pp++) {
        if (!(sindv_pp->status & SCS_USE)) {
            continue;
        }
        if (sindv_pp->status & SCS_END ||
            sindv_pp->status & SCS_END_REQ ||
            sindv_pp->status & SCS_KILL_REQ ||
            sindv_pp->status & SCS_WAIT ||
            sindv_pp->status & SCS_PAUSE) {
            continue;
        }

        tapset_pp = IndvGetTapSetAdrs(sindv_pp);
        if (tapset_pp != NULL) {
            if (sindv_pp->scr_exam_str.exam_enum != EXAM_NONE) {
                ret = (tapset_pp->taptimeEnd + sindv_pp->current_time) - sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].lineTime;
                return ret;
            }
        }
    }

    ret = -1;
    return ret;
}

static SCORE_INDV_STR* GetSindvPcodeLine(PLAYER_CODE pcode) {
    SCORE_INDV_STR *sindv_pp;
    int             i;

    sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZEU(score_indv_str); i++, sindv_pp++) {
        if (sindv_pp->status & SCS_USE) {
            if (sindv_pp->plycode == pcode) {
                return sindv_pp;
            }
        }
    }

    return NULL;
}

static void ExamScoreCheck(SCORE_INDV_STR *sindv_pp) {
    int  i, j;
    long sum;

    WorkClear(exam_check, sizeof(exam_check));

    for (i = 0; i < 3; i++) {
        exam_check[i].tapstr_level = global_data.tapLevel;
        exam_check[i].tapset_level = sindv_pp->tapset_pos;
        exam_check[i].scrdat_pp = sindv_pp->scrdat_pp;
        exam_check[i].ckth = CK_TH_NOCK;
        exam_check[i].ted_num = sindv_pp->scr_tap_memory_cnt;
        exam_check[i].stm_pp = sindv_pp->scr_tap_memory;

        exam_check[i].tapset_pp = IndvGetTapSetAdrs(sindv_pp);

        if (exam_check[i].tapset_pp == NULL) {
            exam_check[i].ofs_tick = sindv_pp->current_time;
        } else {
            exam_check[i].ofs_tick = exam_check[i].tapset_pp->taptimeStart + sindv_pp->current_time;
        }

        if (sindv_pp->global_ply->rank_level == RLVL_COOL || sindv_pp->global_ply->rank_level == RLVL_COOL_GOOD) {
            exam_check[i].otehon_all = KcAll;
        } else {
            exam_check[i].otehon_all = otehon_all_make(&exam_check[i]);
        }

        if (exam_check[i].tapset_pp->tapscode == TAPSCODE_ANSWER) {
            exam_check[i].vs_use = TRUE;
            exam_check[i].vs_tapdat_pp = vs_tapdat_work;
            exam_check[i].vs_tapdat_cnt = vs_tapdat_work_cnt;
        }

        if (global_data.play_typeL == PLAY_TYPE_ONE) {
            exam_check[i].otehon_all = KcTR;
        }
    }

    if (global_data.play_step == PSTEP_HOOK) {
        EXAM_CHECK *exam_check_pp;
        SCRPRGSTR  *scrprgstr_pp;
        int         ret;

        exam_check_pp = exam_check;
        scrprgstr_pp  = scrprgstr_hook;

        on_th_make(exam_check_pp, CK_TH_NORMAL);

        for (j = 0; j < scrprgstr_pp->size; j++) {
            ret = scrprgstr_pp->exh_str_pp[j].score_prg(exam_check_pp);

            if (sindv_pp->global_ply->rank_level == RLVL_HK_COOL || sindv_pp->global_ply->rank_level == RLVL_HK_COOL_GOOD) {
                if (scrprgstr_pp->exh_str_pp[j].save_p == EXH_ALLKEY_OUT ||
                    scrprgstr_pp->exh_str_pp[j].save_p == EXH_MBAR_NUM_OUT ||
                    scrprgstr_pp->exh_str_pp[j].save_p == EXH_MBAR_KEY_OUT ||
                    scrprgstr_pp->exh_str_pp[j].save_p == EXH_MANE ||
                    scrprgstr_pp->exh_str_pp[j].save_p == EXH_MBAR_TIME_OUT) {
                    ret = 0;
                }
            }

            exam_check_pp->each_point[scrprgstr_pp->exh_str_pp[j].save_p] = (ret * scrprgstr_pp->exh_str_pp[j].bairitu) / 16;
        }
    } else {
        CK_TH_ENUM  ck_th_enum_tbl[3] = { 
            CK_TH_NORMAL, 
            CK_TH_NORMAL, 
            CK_TH_HANE
        };
        EXAM_CHECK *exam_check_pp;
        SCRPRGSTR  *scrprgstr_pp;
        int         ret;

        for (i = 0; i < 3; i++) {
            exam_check_pp = &exam_check[i];
            scrprgstr_pp  = &scrprgstr[i];

            on_th_make(exam_check_pp, ck_th_enum_tbl[i]);

            for (j = 0; j < scrprgstr_pp->size; j++) {
                ret = scrprgstr_pp->exh_str_pp[j].score_prg(exam_check_pp);

                if (sindv_pp->global_ply->rank_level == RLVL_COOL || sindv_pp->global_ply->rank_level == RLVL_COOL_GOOD) {
                    if (scrprgstr_pp->exh_str_pp[j].save_p == EXH_ALLKEY_OUT ||
                        scrprgstr_pp->exh_str_pp[j].save_p == EXH_MBAR_NUM_OUT ||
                        scrprgstr_pp->exh_str_pp[j].save_p == EXH_MBAR_KEY_OUT ||
                        scrprgstr_pp->exh_str_pp[j].save_p == EXH_MANE ||
                        scrprgstr_pp->exh_str_pp[j].save_p == EXH_MBAR_TIME_OUT) {
                        ret = 0;
                    }
                }

                exam_check_pp->each_point[scrprgstr_pp->exh_str_pp[j].save_p] = (ret * scrprgstr_pp->exh_str_pp[j].bairitu) / 16;
            }
        }
    }

    sum = 0;

    for (i = 0; i < 3; i++) {
        sindv_pp->global_ply->exam_score[i] = exam_check[i].each_point[EXH_TOTAL];
        sum += exam_check[i].each_point[EXH_TOTAL];
    }

    sindv_pp->global_ply->now_score = sum;
}

static int ExamScoreCheckSame(SCORE_INDV_STR *sindv_pp) {
    SCORE_INDV_STR  sindv_tmp;
    GLOBAL_PLY      global_ply_copy;
    int             i;
    TAPSET         *tapset_pp;
    SCR_TAP_MEMORY *scr_tap_memory_pp;
    TAPDAT         *tapdat_pp;
    int             tapdat_cnt;

    if (sindv_pp->global_ply->rank_level == RLVL_COOL ||
        sindv_pp->global_ply->rank_level == RLVL_COOL_GOOD) {
        return 150;
    }

    global_ply_copy = *sindv_pp->global_ply;

    sindv_tmp      = *sindv_pp;
    sindv_tmp.global_ply = &global_ply_copy;

    tapset_pp = IndvGetTapSetAdrs(sindv_pp);

    sindv_tmp.scr_tap_memory_cnt = 0;
    scr_tap_memory_pp = sindv_tmp.scr_tap_memory;

    tapdat_pp = tapset_pp->tapdat_pp;    
    tapdat_cnt = tapset_pp->tapdat_size;

    if (tapset_pp->tapscode == TAPSCODE_ANSWER) {
        tapdat_pp = vs_tapdat_work;
        tapdat_cnt = vs_tapdat_work_cnt;
    }

    for (i = 0; i < tapdat_cnt; i++, tapdat_pp++) {
        if (tapdat_pp->KeyIndex != KiNO) {
            if (global_data.play_typeL == PLAY_TYPE_ONE) {
                scr_tap_memory_pp->key = KiTR;
            } else {
                scr_tap_memory_pp->key = tapdat_pp->KeyIndex;
            }

            scr_tap_memory_pp->onKey = TRUE;
            scr_tap_memory_pp->ofs_frame = tapdat_pp->time;

            sindv_tmp.scr_tap_memory_cnt++;
            scr_tap_memory_pp++;
        }
    }

    ExamScoreCheck(&sindv_tmp);
    return global_ply_copy.now_score;
}

static int levelChangeCheck(RANK_LEVEL lvl0, RANK_LEVEL lvl1) {
    int lvl0_tmp = RANK_LEVEL2DISP_LEVEL_HK(lvl0);
    int lvl1_tmp = RANK_LEVEL2DISP_LEVEL_HK(lvl1);

    if (lvl0_tmp != lvl1_tmp) {
        return (lvl0_tmp > lvl1_tmp);
    }

    return -1;
}

static int levelUpRank(RANK_LEVEL lvl) {
    RANK_LEVEL up_tbl[17] = {
        /* COOL      -> COOL      */ RLVL_COOL,
        /* COOL/GOOD -> COOL      */ RLVL_COOL,
        /* GOOD/COOL -> COOL      */ RLVL_COOL,
        /* GOOD      -> GOOD/COOL */ RLVL_GOOD_COOL,
        /* GOOD/BAD  -> GOOD      */ RLVL_GOOD,
        /* BAD/GOOD  -> GOOD      */ RLVL_GOOD,
        /* BAD       -> BAD/GOOD  */ RLVL_BAD_GOOD,
        /* BAD/AWFUL -> BAD       */ RLVL_BAD,
        /* AWFUL/BAD -> BAD       */ RLVL_BAD,
        /* AWFUL     -> AWFUL/BAD */ RLVL_AWFUL_BAD,
        /* AWFUL/END -> AWFUL     */ RLVL_AWFUL,
        /* END0      -> AWFUL     */ RLVL_AWFUL,
        /* END1      -> END0      */ RLVL_END0,
        /* END2      -> END1      */ RLVL_END1,
        /* HK_END0   -> END1      */ RLVL_END1,
        /* HK_END1   -> END1      */ RLVL_END1,
        /* HK_END2   -> END1      */ RLVL_END1,
    };

    return up_tbl[lvl];
}

static int levelDownRank(RANK_LEVEL lvl) {
    RANK_LEVEL down_tbl[17] = {
        /* COOL      -> COOL/GOOD */ RLVL_COOL_GOOD,
        /* COOL/GOOD -> GOOD      */ RLVL_GOOD,
        /* GOOD/COOL -> GOOD      */ RLVL_GOOD,
        /* GOOD      -> GOOD/BAD  */ RLVL_GOOD_BAD,
        /* GOOD/BAD  -> BAD       */ RLVL_BAD,
        /* BAD/GOOD  -> BAD       */ RLVL_BAD,
        /* BAD       -> BAD/AWFUL */ RLVL_BAD_AWFUL,
        /* BAD/AWFUL -> AWFUL     */ RLVL_AWFUL,
        /* AWFUL/BAD -> AWFUL     */ RLVL_AWFUL,
        /* AWFUL     -> AWFUL/END */ RLVL_AWFUL_END,
        /* AWFUL/END -> END1      */ RLVL_END1,
        /* END0      -> END1      */ RLVL_END1,
        /* END1      -> END2      */ RLVL_END2,
        /* END2      -> HK_END1   */ RLVL_HK_END1,
        /* HK_END0   -> HK_END1   */ RLVL_HK_END1,
        /* HK_END1   -> HK_END1   */ RLVL_HK_END1,
        /* HK_END2   -> HK_END1   */ RLVL_HK_END1,
    };

    return down_tbl[lvl];
}

void ScrMoveSetSub(SCORE_INDV_STR *sindv_pp, int Pnum, int sub_job, int sub_time, int goto_job, int goto_time, int start_move_line, int start_move_time) {
    int             ttype;
    int             tmp_cdsample;
    int             target_move_time;
    SCORE_INDV_STR *sub_in_pp;

    target_move_time = targetTimeGet(goto_job, goto_time, useIndevCodeGet());

    useIndevAllMove(goto_time, goto_job);

    useIndevSndKill();

    sindv_pp->wakeUpTime = sub_time;
    sindv_pp->wakeUpGoTime = goto_time;
    sindv_pp->wakeUpWaitLine = sub_job;

    sindv_pp->status |= SCS_WAIT;
    otherIndvPause(Pnum);

    sub_in_pp = &score_indv_str[4]; /* MOVE */
    sub_in_pp->status = SCS_USE;
    sub_in_pp->plycode = PCODE_MOVE;
    sub_in_pp->global_ply = 0;

    IndivMoveChange(sub_in_pp, 0, sub_job);

    sub_in_pp->retStartLine = start_move_line;
    sub_in_pp->refStartTime = start_move_time;

    sub_in_pp->refTartegLine = goto_job;
    sub_in_pp->refTargetTime = target_move_time;

    if (GetTimeType(sub_job) != GTIME_VSYNC) {
        GlobalTimeJobChange(FGF_CD);
    } else {
        GlobalTimeJobChange(FGF_VSYNC);
    }

    TimeCallbackTimeSetChanTempo(sub_job, 0, GetLineTempo(sub_job));

    sub_in_pp->top_scr_ctrlpp[sub_job].lineTime = 0;
    sub_in_pp->top_scr_ctrlpp[sub_job].lineTimeFrame = 0;

    ScrLincChangTbl(sub_job);

    tapEventCheck(sub_in_pp, 0, 0, 4);

    ttype = GetTimeType(goto_job);
    if (ttype == GTIME_VSYNC) {
        if (GetTimeType(start_move_line) != ttype) {
            CdctrlWp2Stop();
        }
    } else {
        u_char chantmp[2];
        printf("file seek\n");

        tmp_cdsample = CdctrlSndTime2WP2sample(GetLineTempo(sub_job), goto_time);
        tmp_cdsample -= (GetTimeOfset(goto_job) * 48) / 256;
        if (tmp_cdsample < 0) {
            tmp_cdsample = 0;
        }

        CheckIndvCdChannel(sindv_pp, chantmp);
        CdctrlWP2SetFileSeekChan(&score_str.stdat_dat_pp->sndfile[ttype], tmp_cdsample, chantmp[0], chantmp[1]);
    }
}

int ScrExamSetCheck(SCORE_INDV_STR *sindv_pp, int Pnum, int ctime_next, int indvTime) {
    TAPSET       *tapset_pp;
    int           yaruyaru;
    SCR_EXAM_STR *scex_pp;

    tapset_pp = IndvGetTapSetAdrs(sindv_pp);
    if (tapset_pp == NULL) {
        return 0;
    }

    yaruyaru = FALSE;

    if (indvTime >= tapset_pp->taptimeEnd) {
        yaruyaru = TRUE;
    }

    if (sindv_pp->scr_exam_str.exam_enum == EXAM_CANCEL) {
        yaruyaru = TRUE;
    }

    if (yaruyaru) {
        if (sindv_pp->tap_follow_enum == TAP_FOLLOW_SAVE) {
            followTapSave(sindv_pp);
        }

        scex_pp = &sindv_pp->scr_exam_str;

        /* Cancelled, bonus and empty exams skip the scoring step */
        if (scex_pp->exam_enum != EXAM_CANCEL && scex_pp->exam_enum != EXAM_BONUS && scex_pp->exam_enum != EXAM_NONE &&
            scex_pp->exam_do == EXAM_DO_NON) {
            ExamScoreCheck(sindv_pp);

            if (global_data.play_step == PSTEP_GAME) {
                if (global_data.play_typeL == PLAY_TYPE_ONE) {
                    sindv_pp->global_ply->now_score = (sindv_pp->global_ply->now_score + 1) / 2;
                }
                if (global_data.play_table_modeL == PLAY_TABLE_EASY) {
                    sindv_pp->global_ply->now_score = (sindv_pp->global_ply->now_score + 1) / 2;
                }
            }

            if (dbg_select_str.debug_on && dbg_select_str.score_updown) {
                if (sindv_pp->global_ply != NULL) {
                    if (pad[0].shot & SCE_PADLdown) {
                        sindv_pp->global_ply->now_score = 1;
                    }
                    if (pad[0].shot & SCE_PADLup) {
                        sindv_pp->global_ply->now_score = 500;
                    }
                }
            }

            if (global_data.demo_flagL == DEMOF_REPLAY) {
                MC_REP_SCR *mcr_scr_pp;
                int         i;

                mcr_scr_pp = mccReqScrGet();

                if (mcr_scr_pp != NULL) {
                    sindv_pp->global_ply->now_score = mcr_scr_pp->now_score;

                    for (i = 0; i < 3; i++) {
                        sindv_pp->global_ply->exam_score[i] = mcr_scr_pp->exam_score[i];
                    }
                } else {
                    printf("!!!mccReqScrGet over!!!\n");

                    sindv_pp->global_ply->now_score = 0;

                    for (i = 0; i < 3; i++) {
                        sindv_pp->global_ply->exam_score[i] = 0;
                    }
                }
            } else {
                if (global_data.demo_flagL == DEMOF_OFF) {
                    MC_REP_SCR mcr_scr;
                    int        i;

                    mcr_scr.now_score = sindv_pp->global_ply->now_score;

                    for (i = 0; i < 3; i++) {
                        mcr_scr.exam_score[i] = sindv_pp->global_ply->exam_score[i];
                    }

                    mccReqScrSet(&mcr_scr);
                }
            }

            if (sindv_pp->global_ply->now_score > 0) {
                sindv_pp->global_ply->exam_tbl_up++;
            } else {
                sindv_pp->global_ply->exam_tbl_dw++;
            }

            if (global_data.play_step != PSTEP_VS) {
                sindv_pp->global_ply->score += sindv_pp->global_ply->now_score;
            }

            if (sindv_pp->global_ply->score < 0) {
                sindv_pp->global_ply->score = 0;
            }

            if (sindv_pp->plycode == PCODE_PARA) {
                ExamDispReq(0, 1);
            } else {
                ExamDispReq(1, 1);
            }
        }

        if (scex_pp->exam_enum != EXAM_NONE) {
            int rank_saki = RLVL_COOL;
            int rank_moto = RLVL_COOL;

            if (global_data.demo_flagL == DEMOF_REPLAY) {
                PLAYER_ENUM player_enum_tmp = NON_PLAYER_NUM;

                if (sindv_pp->plycode == PCODE_PARA) {
                    player_enum_tmp = PARA_PLAYER_NUM;
                }
                if (sindv_pp->plycode == PCODE_TEACHER) {
                    player_enum_tmp = TEACHER_PLAYER_NUM;
                }
                if (sindv_pp->plycode == PCODE_BOXY) {
                    player_enum_tmp = BOXY_PLAYER_NUM;
                }

                mccReqTapForwardOwn(ctime_next, sindv_pp->useLine, player_enum_tmp);
            }

            if (scex_pp->exam_enum == EXAM_CANCEL) {
                if (sindv_pp->cansel_flag ||
                    (ScrCtrlIndvNextReadLine(sindv_pp, 2) && indvTime >= tapset_pp->taptimeEnd)
                ) {
                    scex_pp->exam_do = EXAM_DO_END_GO;
                    scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[0];
                } else {
                    if (indvTime < tapset_pp->taptimeEnd) {
                        return 0;
                    }
                }
            } else if (scex_pp->exam_enum == EXAM_BONUS) {
                int bline = ingame_common_str.HookClrCnt;

                if (bline > 10) {
                    bline = 10;
                }

                if (bline < bonusGameCntPls()) {
                    scex_pp->exam_do = EXAM_DO_END_GO;
                    scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[0];
                }
            } else {
                if (scex_pp->exam_do == EXAM_DO_NON) {
                    int exp = 0;

                    scex_pp->exam_point = sindv_pp->global_ply->now_score;

                    if (tapset_pp == NULL) {
                        scex_pp->exam_coolP = 11111;
                    } else {
                        exp = ExamScoreCheckSame(sindv_pp);
                        if (global_data.play_step != PSTEP_VS) {
                            scex_pp->exam_coolP = tapset_pp->coolup + exp;
                        } else {
                            scex_pp->exam_coolP = exp;
                        }
                    }

                    exam_tbl_updownSet(sindv_pp, sindv_pp->global_ply->now_score, scex_pp->exam_coolP, exp);

                    scex_pp->exam_do = EXAM_DO_END;

                    rank_moto = sindv_pp->global_ply->rank_level;
                    rank_saki = sindv_pp->global_ply->rank_level;

                    switch (scex_pp->exam_enum) {
                    case EXAM_COOL:
                        if (scex_pp->exam_point < scex_pp->exam_coolP) {
                            rank_saki = levelDownRank(rank_moto);
                        } else {
                            rank_saki = levelUpRank(rank_moto);
                        }
                        break;
                    case EXAM_GOOD:
                        if (scex_pp->exam_point >= scex_pp->exam_coolP) {
                            rank_saki = levelUpRank(rank_moto);
                        } else if (scex_pp->exam_point <= 0) {
                            rank_saki = levelDownRank(rank_moto);
                        } else {
                            if (rank_moto == RLVL_GOOD_COOL) {
                                rank_saki = levelDownRank(rank_moto);
                            } else if (rank_moto != RLVL_GOOD) {
                                rank_saki = levelUpRank(rank_moto);
                            }
                        }

                        if (rank_saki < RLVL_GOOD) {
                            if (global_data.play_typeL == PLAY_TYPE_ONE || global_data.play_table_modeL == PLAY_TABLE_EASY) {
                                rank_saki = RLVL_GOOD;
                            }
                        }

                        break;
                    case EXAM_BAD:
                    case EXAM_AWFUL:
                        if (scex_pp->exam_point > 0) {
                            rank_saki = levelUpRank(rank_moto);
                        } else {
                            rank_saki = levelDownRank(rank_moto);
                        }
                        break;
                    case EXAM_HOOK: {
                        int nexton = ScrCtrlIndvNextReadLine(sindv_pp, 1);
                        printf("hook exam!!\n");

                        MbarNikoSet(sindv_pp->global_ply->exam_tbl_up * 2, 0);

                        if (!nexton) {
                            if (sindv_pp->global_ply->exam_tbl_up >= 10) {
                                scex_pp->exam_do = EXAM_DO_END_GO;
                                scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[0];

                                printf("hook end job\n");
                            }
                        } else {
                            if (sindv_pp->global_ply->exam_tbl_up >= 10) {
                                scex_pp->exam_do = EXAM_DO_END_GO;
                                scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[0];

                                printf("hook end job\n");
                            } else {
                                if (scex_pp->scr_exam_job[1].goto_line != -1) {
                                    scex_pp->exam_do = EXAM_DO_END_GO;
                                    scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[1];

                                    printf("hook loop job\n");
                                }
                            }
                        }

                        if (scex_pp->exam_point > 0) {
                            ScrTapReq(SCR_TAP_COMMON, 0, 0);
                        } else {
                            ScrTapReq(SCR_TAP_COMMON, 0, 1);
                        }

                        break;
                    }
                    case EXAM_VS: {
                        int         hantei_flag;
                        GLOBAL_PLY *gplay_my, *gplay_enemy;
                        int         my_ply,    ene_ply;
                        int         tapscode; /* not in the original debug info */

                        hantei_flag = ScrCtrlIndvNextReadLine(sindv_pp, 2) != 0;

                        if (sindv_pp->plycode == PCODE_PARA) {
                            my_ply  = 0;
                            ene_ply = 1;
                        } else {
                            my_ply  = 1;
                            ene_ply = 0;
                        }

                        gplay_my    = sindv_pp->global_ply;
                        gplay_enemy = score_indv_str[Pcode2Pindex(scex_pp->vsPlayer)].global_ply;

                        printf("exam vs [%d] index[%d]\n", gplay_enemy, scex_pp->vsPlayer);

                        tapscode = IndvGetTapSetAdrs(sindv_pp)->tapscode;
                        if (tapscode == TAPSCODE_ANSWER_F) {
                            int pointx;

                            printf("first !!hantei[%d]\n", hantei_flag);

                            if (sindv_pp->global_ply->now_score > 0) {
                                pointx = sindv_pp->global_ply->now_score + 500;
                                ScrTapReq(SCR_TAP_COMMON, 0, 0);
                                vsAnimationReq(my_ply, 500, pointx, VSMT_UP);
                            } else {
                                pointx = sindv_pp->global_ply->now_score + 500;
                                ScrTapReq(SCR_TAP_COMMON, 0, 1);
                                vsAnimationReq(my_ply, 500, pointx, VSMT_DW);
                            }

                            sindv_pp->global_ply->score = pointx;
                        } else if (tapscode == TAPSCODE_ANSWER) {
                            int pointx = sindv_pp->global_ply->now_score - scex_pp->exam_coolP;

                            if (pointx < 0) {
                                vsAnimationReq(my_ply, gplay_my->score, gplay_my->score + pointx, VSMT_DW);

                                gplay_my->score += pointx;
                                if (gplay_my->score < 0) {
                                    gplay_my->score = 0;
                                }

                                ScrTapReq(SCR_TAP_COMMON, 0, 1);

                                sindv_pp->global_ply->now_score = pointx;

                                ExamDispReq(my_ply, 1);
                            } else {
                                vsAnimationReq(my_ply,  gplay_my->score,    gplay_my->score,             VSMT_UP);
                                vsAnimationReq(ene_ply, gplay_enemy->score, gplay_enemy->score - pointx, VSMT_DW);

                                gplay_enemy->score -= pointx;
                                if (gplay_enemy->score < 0) {
                                    gplay_enemy->score = 0;
                                }

                                ScrTapReq(SCR_TAP_COMMON, 0, 0);

                                gplay_enemy->now_score = -pointx;

                                ExamDispReq(ene_ply, 1);
                                ExamDispReq(my_ply,  0);
                            }

                            if (gplay_my->score == 0 || gplay_enemy->score == 0) {
                                hantei_flag = TRUE;
                            }
                        }

                        if (hantei_flag) {
                            int jobnum;
                            int endbatle;
                            int battle_cnt;

                            mccReqTapForward(ctime_next, sindv_pp->useLine);

                            if (gplay_my->score == gplay_enemy->score) {
                                jobnum = SCREX_AR_DRAW;

                                gplay_my->vsDraw++;
                                gplay_enemy->vsDraw++;
                            } else {
                                if (gplay_enemy->score < gplay_my->score) {
                                    jobnum = SCREX_AR_WIN0;

                                    gplay_my->vsWin++;
                                    gplay_enemy->vsLost++;

                                    if (gplay_my->score > 250 && gplay_enemy->score == 0) {
                                        jobnum = SCREX_AR_WIN1;
                                    }

                                    if (gplay_my->score > 250 && gplay_enemy->score > 250) {
                                        jobnum = SCREX_AR_WIN2;
                                    }
                                } else {
                                    jobnum = SCREX_AR_LOSE0;

                                    gplay_my->vsLost++;
                                    gplay_enemy->vsWin++;

                                    if (gplay_enemy->score > 250 && gplay_my->score == 0) {
                                        jobnum = SCREX_AR_LOSE1;
                                    }

                                    if (gplay_my->score > 250 && gplay_enemy->score > 250) {
                                        jobnum = SCREX_AR_LOSE2;
                                    }
                                }
                            }

                            battle_cnt = gplay_my->vsWin + gplay_my->vsLost + gplay_my->vsDraw;
                            endbatle = sindv_pp->scr_exam_str.scr_exam_job[SCREX_AR_DRAW].goto_job == SCRLINE_NODATA;

                            if (gplay_my->vsLost < gplay_my->vsWin) {
                                if (gplay_my->vsWin > (((!endbatle ? 5 : 4) - battle_cnt) + gplay_my->vsLost)) {
                                    endbatle = TRUE;
                                }
                            } else {
                                if (gplay_my->vsLost > (((!endbatle ? 5 : 4) - battle_cnt) + gplay_my->vsWin)) {
                                    endbatle = TRUE;
                                }
                            }

                            if (endbatle) {
                                if (gplay_my->vsLost == gplay_my->vsWin) {
                                    jobnum = SCREX_AB_DRAW;
                                } else if (gplay_my->vsLost < gplay_my->vsWin) {
                                    jobnum = SCREX_AB_WIN0;

                                    if (gplay_my->vsWin >= 3 && gplay_my->vsLost <= 1) {
                                        jobnum = SCREX_AB_WIN1;
                                    }

                                    if (gplay_my->vsWin >= 1 && gplay_my->vsWin <= 2) {
                                        jobnum = SCREX_AB_WIN2;
                                    }
                                } else {
                                    jobnum = SCREX_AB_LOSE0;

                                    if (gplay_my->vsLost >= 3 && gplay_my->vsWin <= 1) {
                                        jobnum = SCREX_AB_LOSE1;
                                    }

                                    if (gplay_my->vsLost >= 1 && gplay_my->vsLost <= 2) {
                                        jobnum = SCREX_AB_LOSE2;
                                    }
                                }
                            }

                            gplay_my->vsScore    += gplay_my->score;
                            gplay_enemy->vsScore += gplay_enemy->score;

                            scex_pp->exam_do = EXAM_DO_END_GO;
                            scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[jobnum];

                            if (sindv_pp->plycode == PCODE_PARA) {
                                MbarNikoSet((gplay_my->vsWin * 2) + gplay_my->vsDraw, 0);
                                MbarNikoSet((gplay_enemy->vsWin * 2) + gplay_enemy->vsDraw, 3);
                            } else {
                                MbarNikoSet((gplay_my->vsWin * 2) + gplay_my->vsDraw, 3);
                                MbarNikoSet((gplay_enemy->vsWin * 2) + gplay_enemy->vsDraw, 0);
                            }
                        }

                        printf("vs cool point:%d  now point:%d\n", scex_pp->exam_coolP, scex_pp->exam_point);

                        if (scex_pp->exam_point > scex_pp->exam_coolP) {
                            if (gplay_my->exam_score[1] != 0) {
                                vsTapdatSet(sindv_pp);
                            }
                        }

                        if (global_data.demo_flagL == DEMOF_REPLAY) {
                            vsTapdatSetMemoryLoad();
                        } else {
                            vsTapdatSetMemorySave();
                        }

                        break;
                    }
                    }

                    if (scex_pp->exam_enum != EXAM_HOOK && scex_pp->exam_enum != EXAM_VS) {
                        if (rank_saki != rank_moto) {
                            int line_change;

                            sindv_pp->global_ply->rank_level = rank_saki;

                            line_change = levelChangeCheck(rank_saki, rank_moto);
                            
                            if (line_change >= 0) {
                                int men_ctrl_enum = -1;

                                scex_pp->exam_do = EXAM_DO_END_GO;
                                scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[line_change];

                                if (rank_moto < rank_saki) {
                                    ScrTapReq(SCR_TAP_COMMON, 0, 1);

                                    if (RANK_LEVEL2DISP_LEVEL_HK(rank_saki) == DLVL_BAD) {
                                        men_ctrl_enum = MEN_CTRL_GtoB;
                                    }
                                    if (RANK_LEVEL2DISP_LEVEL_HK(rank_saki) == DLVL_AWFUL) {
                                        men_ctrl_enum = MEN_CTRL_BtoA;
                                    }
                                } else {
                                    ScrTapReq(SCR_TAP_COMMON, 0, 0);

                                    if (RANK_LEVEL2DISP_LEVEL_HK(rank_saki) == DLVL_GOOD) {
                                        men_ctrl_enum = MEN_CTRL_BtoG;
                                    }
                                    if (RANK_LEVEL2DISP_LEVEL_HK(rank_saki) == DLVL_BAD) {
                                        men_ctrl_enum = MEN_CTRL_AtoB;
                                    }
                                }

                                if (men_ctrl_enum >= 0) {
                                    MendererReq(men_ctrl_enum);
                                }
                            } else {
                                if (rank_moto < rank_saki) {
                                    if (scex_pp->scr_exam_job[3].goto_job != -1) {
                                        if (scex_pp->scr_exam_job[3].goto_line == 0 || !ScrCtrlIndvNextReadLine(sindv_pp, 1)) {
                                            scex_pp->exam_do = EXAM_DO_END_GO_RET;
                                            scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[3];
                                        }
                                    }

                                    ScrTapReq(SCR_TAP_COMMON, 0, 1);
                                } else {
                                    if (scex_pp->scr_exam_job[2].goto_job != -1) {
                                        if (scex_pp->scr_exam_job[2].goto_line == 0 || !ScrCtrlIndvNextReadLine(sindv_pp, 1)) {
                                            scex_pp->exam_do = EXAM_DO_END_GO_RET;
                                            scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[2];
                                        }
                                    }
                                    ScrTapReq(SCR_TAP_COMMON, 0, 0);
                                }
                            }
                        } else {
                            if (scex_pp->scr_exam_job[2].goto_job != -1) {
                                if (scex_pp->scr_exam_job[2].goto_line == 0 || !ScrCtrlIndvNextReadLine(sindv_pp, 1)) {
                                    scex_pp->exam_do = EXAM_DO_END_GO_RET;
                                    scex_pp->scr_exam_job_pp = &scex_pp->scr_exam_job[2];
                                }
                            }

                            ScrTapReq(SCR_TAP_COMMON, 0, 0);
                        }
                    }
                }
            }

            if (scex_pp->exam_do == EXAM_DO_END_GO) {
                int goto_time, goto_line;

                tapReqGroupInit();

                scex_pp->exam_start = sindv_pp->current_time + tapset_pp->taptimeEnd;

                if (scex_pp->scr_exam_job_pp->goto_job_time > 0) {
                    ScrMoveSetSub(sindv_pp, Pnum, scex_pp->scr_exam_job_pp->goto_job, scex_pp->scr_exam_job_pp->goto_job_time,
                                  scex_pp->scr_exam_job_pp->goto_line, scex_pp->scr_exam_job_pp->goto_time, sindv_pp->useLine, scex_pp->exam_start);
                    return -1;
                }

                goto_time = scex_pp->scr_exam_job_pp->goto_time;
                goto_line = scex_pp->scr_exam_job_pp->goto_line;

                useIndevAllMove(goto_time, goto_line);

                TimeCallbackTimeSetChanTempo(goto_line, goto_time, GetLineTempo(goto_line));

                sindv_pp->top_scr_ctrlpp[goto_line].lineTime = goto_time;
                sindv_pp->top_scr_ctrlpp[goto_line].lineTimeFrame = ((goto_time * 3600.0f) + (GetLineTempo(goto_line) * 96.0f * 0.5f)) / (GetLineTempo(goto_line) * 96.0f);

                ScrLincChangTbl(goto_line);
                return -1;
            }

            if (global_data.play_step == PSTEP_GAME) {
                if (rank_moto < rank_saki) {
                    DrawTapReqTbl(DR_TAP_REQ(DR_TAP_ID_ALL, DR_TAP_REQ_RANK_DOWN), PINDEX_PARA, NULL);
                } else if (rank_moto > rank_saki) {
                    DrawTapReqTbl(DR_TAP_REQ(DR_TAP_ID_ALL, DR_TAP_REQ_RANK_UP), PINDEX_PARA, NULL);
                } else {
                    /* Stage 6 specific logic */
                    if (global_data.play_stageL == 6) {
                        DrawTapReqTbl(DR_TAP_REQ(DR_TAP_ID_ALL, DR_TAP_REQ_RANK_UP), PINDEX_PARA, NULL);
                    }
                }
            }

            if (scex_pp->exam_do == EXAM_DO_END_GO_RET) {
                int             sub_job, sub_time;
                int             ttype;
                SCR_EXAM_JOB   *sej_pp;
                SCORE_INDV_STR *sub_in_pp;
                int             exam_start; /* note: variable not in STABS. */

                tapReqGroupInit();

                exam_start = sindv_pp->current_time + tapset_pp->taptimeEnd;
                scex_pp->exam_start = exam_start;

                sej_pp = scex_pp->scr_exam_job_pp;

                sub_job = sej_pp->goto_job;
                sub_time = sej_pp->goto_job_time;

                ScrCtrlIndvNextRead(sindv_pp, 1);
                sindv_pp->sjob[0] = exam_start;
                sindv_pp->sjob_data[0][0] = 0;
                sindv_pp->sjob_data[0][1] = 0;
                sindv_pp->sjob[2] = exam_start;
                sindv_pp->sjob_data[2][0] = 0;
                sindv_pp->sjob_data[2][1] = 0;

                sindv_pp->wakeUpTime = sub_time;
                sindv_pp->wakeUpGoTime = exam_start;
                sindv_pp->wakeUpWaitLine = sub_job;

                sindv_pp->status |= SCS_WAIT;

                otherIndvTapReset(Pnum);
                otherIndvPause(Pnum);

                sub_in_pp = &score_indv_str[4];
                sub_in_pp->status = SCS_USE;
                sub_in_pp->plycode = PCODE_MOVE;
                sub_in_pp->global_ply = NULL;

                IndivMoveChange(sub_in_pp, 0, sub_job);

                ttype = GetTimeType(sub_job);
                if (ttype != GTIME_VSYNC) {
                    GlobalTimeJobChange(FGF_CD);
                } else {
                    GlobalTimeJobChange(FGF_VSYNC);
                }

                TimeCallbackTimeSetChanTempo(sub_job, 0, GetLineTempo(sub_job));

                sub_in_pp->top_scr_ctrlpp[sub_job].lineTime = 0;
                sub_in_pp->top_scr_ctrlpp[sub_job].lineTimeFrame = 0;

                ScrLincChangTbl(sub_job);

                ttype = GetTimeType(sindv_pp->useLine);
                if (ttype != GTIME_VSYNC) {
                    int    tmp_cdsample;
                    u_char chantmp[2];

                    tmp_cdsample  = CdctrlSndTime2WP2sample(GetLineTempo(sub_job), exam_start);
                    tmp_cdsample -= (GetTimeOfset(sindv_pp->useLine) * 48) / 256;
                    if (tmp_cdsample < 0) {
                        tmp_cdsample = 0;
                    }

                    CheckIndvCdChannel(sindv_pp, chantmp);
                    CdctrlWP2SetFileSeekChan(&score_str.stdat_dat_pp->sndfile[ttype], tmp_cdsample, chantmp[0], chantmp[1]);
                }

                return -1;
            }

            if (sindv_pp->scr_exam_str.exam_do == EXAM_DO_END && global_data.play_step == PSTEP_GAME && sindv_pp->global_ply != NULL) {
                if (sindv_pp->global_ply->rank_level == RLVL_COOL || sindv_pp->global_ply->rank_level == RLVL_COOL_GOOD) {
                    ScrCtrlIndvNextRead(sindv_pp, 0);
                    ScrLincChangTbl(sindv_pp->useLine);
                    return 1;
                }
            }

        }

        if (sindv_pp->scr_exam_str.exam_do != EXAM_DO_END_GO && sindv_pp->scr_exam_str.exam_do != EXAM_DO_END_GO_RET) {
            ScrCtrlIndvNextRead(sindv_pp, 1);
            ScrLincChangTbl(sindv_pp->useLine);
            return 1;
        }
    }

    return 0;
}

void subjobEvent(SCORE_INDV_STR *sindv_pp, int ctime_next) {
    int j;
    int cont_job;

    for (j = 0; j < PR_ARRAYSIZE(sindv_pp->sjob); j++) {
        cont_job = FALSE;

        if (sindv_pp->sjob[j] == -1 || sindv_pp->sjob[j] > ctime_next) {
            continue;
        }

        switch (j) {
        case SCRSUBJ_CDSND_ON:
            CdctrlWP2SetVolume(120);
            break;
        case SCRSUBJ_CDSND_OFF:
            CdctrlWP2SetVolume(0);
            break;
        case SCRSUBJ_DRAW_CHANGE:
            ScrLincChangTbl(sindv_pp->useLine);
            break;
        case SCRSUBJ_TAP_RESET:
            break;
        case SCRSUBJ_EFFECT:
            TapCt(TAPCT_SETEFFECTMODE, sindv_pp->sjob_data[j][0], sindv_pp->sjob_data[j][1]);
            break;
        case SCRSUBJ_REVERS: {
            int drline = sindv_pp->retStartLine;
            int time_tmp;
            int temp2 = (sindv_pp->sjob_data[j][0] / 2) < ctime_next;

            if (temp2) {
                drline = sindv_pp->refTartegLine;
            }

            temp2 = sindv_pp->refTargetTime - sindv_pp->refStartTime;
            time_tmp = temp2 * ctime_next;
            time_tmp /= sindv_pp->sjob_data[j][0];
            time_tmp = time_tmp != 0 ? time_tmp : 1;
            ScrLincChangTblRef(drline, time_tmp + sindv_pp->refStartTime);

            cont_job = TRUE;
            break;
        }
        case SCRSUBJ_SPU_ON:
        case SCRSUBJ_SPU_ON2:
        case SCRSUBJ_SPU_ON3:
        case SCRSUBJ_SPU_ON4:
            ScrTapReq(SCR_TAP_COMMON, sindv_pp->sjob_data[j][0], sindv_pp->sjob_data[j][1]);
            break;
        case SCRSUBJ_TITLE:
            if (sindv_pp->sjob_data[j][0] == 0) {
                if (sindv_pp->sjob_data[j][1] == 0) {
                    MendererCtrlTitle();
                    sindv_pp->sjob_data[j][1] = 1;
                }
                if (pad[0].one & SCE_PADstart) {
                    int next_time = ScrCtrlIndvNextTime(sindv_pp, 2);
                    TimeCallbackTimeSetChanTempo(sindv_pp->useLine, next_time, GetLineTempo(sindv_pp->useLine));
                }
            } else if (sindv_pp->sjob_data[j][0] == 2) {
                if (sindv_pp->sjob_data[j][1] == 0) {
                    MendererCtrlTitleDera();
                    sindv_pp->sjob_data[j][1] = 1;
                }
                if (pad[0].one & SCE_PADstart) {
                    int next_time = ScrCtrlIndvNextTime(sindv_pp, 2);
                    TimeCallbackTimeSetChanTempo(sindv_pp->useLine, next_time, GetLineTempo(sindv_pp->useLine));
                }
            } else if (pad[0].one & SCE_PADstart) {
                if (!titleStartKey) {
                    titleStartKey = TRUE;
                    ScrTapReq(SCR_TAP_COMMON, 0, 2);
                    DrawTapReqTbl(DR_TAP_REQ(DR_TAP_ID_ALL, DR_TAP_REQ_TITLE_START), PINDEX_NONE, NULL);
                }
            }

            cont_job = TRUE;
            break;
        case SCRSUBJ_LOOP: {
            int next_time = sindv_pp->sjob_data[j][0];

            TimeCallbackTimeSetChanTempo(sindv_pp->useLine, next_time, GetLineTempo(sindv_pp->useLine));
            cont_job = TRUE;
            break;
        }
        case SCRSUBJ_FADEOUT:
            fadeoutStartKey = TRUE;
            cont_job = TRUE;
            break;
        case SCRSUBJ_ENDLOOP:
            gameEndWaitLoop = TRUE;
            break;
        case SCRSUBJ_SPUTRANS: {
            int *data = (int*)sindv_pp->sjob_data[j][0];
            if (*data != 0) {
                ScrTapDbuffSetSp(&score_str.stdat_dat_pp->scr_pp->sndrec_pp[*data], sindv_pp->sndId);
            }
            break;
        }
        case SCRSUBJ_STOP_MENDERER: {
            int next_time = ScrCtrlIndvNextTime(sindv_pp, 1) - sindv_pp->current_time;

            PrDecelerateMenderer(((next_time * 3600.0f) + (GetLineTempo(sindv_pp->useLine) * 96.0f * 0.5f)) / (GetLineTempo(sindv_pp->useLine) * 96.0f));
            printf("SCRSUBJ_STOP_MENDERER req\n");
            break;
        }
        case SCRSUBJ_BONUS_GAME:
            bonusGameCtrl(ctime_next);
            cont_job = TRUE;
            break;
        case SCRSUBJ_BONUS_GAME_END:
            bonusPointSave();
            bonusScoreDraw();
            break;
        case SCRSUBJ_LESSON: {
            int time_tmp;
            LessonRoundDisp(sindv_pp->sjob_data[j][0]);

            time_tmp = sindv_pp->sjob_data[j][1] + 1;
            sindv_pp->sjob_data[j][1] = time_tmp;

            cont_job = (time_tmp < 180);
            break;
        }
        case SCRSUBJ_VS_RESET: {
            SCORE_INDV_STR *cngSindv_pp;

            cngSindv_pp = GetSindvPcodeLine(PCODE_TEACHER);
            if (cngSindv_pp != NULL) {
                cngSindv_pp->global_ply->score = 500;
            }
            vsAnimationReset(1, 500);

            cngSindv_pp = GetSindvPcodeLine(PCODE_PARA);
            if (cngSindv_pp != NULL) {
                cngSindv_pp->global_ply->score = 500;
            }
            vsAnimationReset(0, 500);
            break;
        }
        case SCRSUBJ_CDSND_READY:
            CdctrlWP2Set(&score_str.stdat_dat_pp->sndfile[sindv_pp->sjob_data[j][0]]);
            break;
        case SCRSUBJ_CDSND_REQ:
            CdctrlWP2Play();
            CdctrlWP2SetVolume(sindv_pp->sjob_data[j][0]);
            break;
        case SCRSUBJ_SPU_OFF:
            ScrTapReqStop(sindv_pp->sjob_data[j][0]);
            break;
        case SCRSUBJ_JIMAKU_OFF:
            jimakuWakuOff = TRUE;
            break;
        }

        if (!cont_job) {
            sindv_pp->sjob[j] = -1;
        }
    }
}

static void ScrCtrlIndvJob(void) {
    int             i;
    int             ctime_next;
    SCORE_INDV_STR *sindv_pp;
    int             next_time;

    sindv_pp = score_indv_str;

    for (i = 0; i < 5; i++, sindv_pp++) {
        if (!(sindv_pp->status & SCS_USE)) {
            continue;
        }
        if (sindv_pp->status & SCS_END) {
            continue;
        }
        if (sindv_pp->status & SCS_PAUSE) {
            continue;
        }
        if (sindv_pp->status & SCS_PAUSE_END) {
            continue;
        }

        ctime_next = sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].lineTime;

        if (!(sindv_pp->status & SCS_WAIT)) {
            if (sindv_pp->status & SCS_END_REQ) {
                if (sindv_pp->current_time <= ctime_next) {
                    sindv_pp->status &= ~SCS_END_REQ;
                    sindv_pp->status |= SCS_END;
                }
            } else if (sindv_pp->status & SCS_KILL_REQ) {
                if (sindv_pp->current_time <= ctime_next) {
                    sindv_pp->status = 0;
                }
            }
        } else {
            int check_time = sindv_pp->top_scr_ctrlpp[sindv_pp->wakeUpWaitLine].lineTime;

            if (check_time >= sindv_pp->wakeUpTime) {
                int ttype; /* note: variable not in STABS (not required, but matches line numbers). */

                printf("wake up now!! ckline[%d] time[%d] cktime[%d]\n", sindv_pp->wakeUpWaitLine, check_time, sindv_pp->wakeUpTime);

                TimeCallbackTimeSetChanTempo(sindv_pp->useLine, sindv_pp->wakeUpGoTime, GetLineTempo(sindv_pp->useLine));

                ctime_next = sindv_pp->wakeUpGoTime;
                sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].lineTime = ctime_next;
                sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].lineTimeFrame = (((ctime_next * 3600.0f) + ((GetLineTempo(sindv_pp->useLine) * 96.0f) * 0.5f)) / (GetLineTempo(sindv_pp->useLine) * 96.0f));
                sindv_pp->status &= ~SCS_WAIT;

                allIndvNextContinue();

                ttype = GetTimeType(sindv_pp->useLine);
                if (ttype != GTIME_VSYNC) {
                    printf("cd job now!!\n");
                    SetLineChannel(sindv_pp->useLine);
                    CdctrlWP2Play();
                    GlobalTimeJobChange(FGF_CD);
                } else {
                    GlobalTimeJobChange(FGF_VSYNC);
                }

                subjobEvent(sindv_pp, ctime_next);
            }
        }
    }

    allIndvGoContinue();

    sindv_pp = score_indv_str;

    for (i = 0; i < 5; i++, sindv_pp++) {
        int indvTime;

        if (!(sindv_pp->status & SCS_USE)) {
            continue;
        }

        if (sindv_pp->status & SCS_END ||
            sindv_pp->status & SCS_END_REQ ||
            sindv_pp->status & SCS_KILL_REQ ||
            sindv_pp->status & SCS_WAIT ||
            sindv_pp->status & SCS_PAUSE) {
            continue;
        }

        ctime_next = sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].lineTime;
        indvTime   = ctime_next - sindv_pp->current_time;

        if (sindv_pp->current_time <= ctime_next) {
            subjobEvent(sindv_pp, ctime_next);

            next_time = ScrCtrlIndvNextTime(sindv_pp, 1);

            if (sindv_pp->scrdat_pp == NULL) {
                if (ctime_next >= next_time) {
                    ScrCtrlIndvNextRead(sindv_pp, 1);
                }
                continue;
            } else if (sindv_pp->scr_exam_str.exam_enum == EXAM_NONE) {
                TAPSET *tapset_pp = IndvGetTapSetAdrs(sindv_pp);

                if (sindv_pp->tap_follow_enum == TAP_FOLLOW_SAVE) {
                    followTapSave(sindv_pp);
                }

                if (tapset_pp == NULL) {
                    if (ctime_next >= next_time) {
                        ScrCtrlIndvNextRead(sindv_pp, 1);
                        continue;
                    }
                } else {
                    if (indvTime >= tapset_pp->taptimeEnd) {
                        ScrCtrlIndvNextRead(sindv_pp, 1);
                        continue;
                    }
                }
            } else {
                int ret = ScrExamSetCheck(sindv_pp, i, ctime_next, indvTime);

                if (ret < 0) {
                    return;
                } else if (ret > 0) {
                    continue;
                }
            }

            tapEventCheck(sindv_pp, ctime_next, indvTime, i);
        }
    }
}

static void ScrTimeRenew(SCR_MAIN *scr_main_pp) {
    int   i;
    int   samplecnt;
    float tempo;

    for (i = 0; i < scr_main_pp->scr_ctrl_num; i++) {
        if (scr_main_pp->scr_ctrl_pp[i].gtime_type == GTIME_VSYNC) {
            scr_main_pp->scr_ctrl_pp[i].lineTime =  ((TimeCallbackTimeGetChan(i) * 96.0f * GetLineTempo(i) + 1800.0f)  / 3600.0f);
            scr_main_pp->scr_ctrl_pp[i].lineTime += ((GetLineTempo(i) * 96.0f * scr_main_pp->scr_ctrl_pp[i].ofsCdtime) / 60000.0f);

            if (scr_main_pp->scr_ctrl_pp[i].lineTime < 0) {
                scr_main_pp->scr_ctrl_pp[i].lineTime = 0;
            }

            scr_main_pp->scr_ctrl_pp[i].lineTimeFrame = TimeCallbackTimeGetChan(i);
        } else {
            samplecnt = GlobalSndSampleGet() + ((scr_main_pp->scr_ctrl_pp[i].ofsCdtime * 48) / 256);

            if (global_data.play_step == PSTEP_XTR) {
                samplecnt = CdctrlWp2GetSampleTmp() - getTopSeekPos();
            }
            if (samplecnt < 0) {
                samplecnt = 0;
            }

            tempo = GetLineTempo(i);

            scr_main_pp->scr_ctrl_pp[i].lineTime = CdctrlWp2CdSample2SndTime(samplecnt, tempo);
            scr_main_pp->scr_ctrl_pp[i].lineTimeFrame = CdctrlWp2CdSample2Frame(samplecnt);
        }
    }
}

void ScrMbarReq(int mbarTime) {
    int             i;
    SCORE_INDV_STR *sindv_pp;
    TAPSET         *tapset_pp;
    int             dare;
    int             tapdat_size;
    TAPDAT         *tapdat_pp;

    MbarSetCtrlTime(mbarTime);
    sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZEU(score_indv_str); i++, sindv_pp++) {
        if (!(sindv_pp->status & SCS_USE)) {
            continue;
        }

        if (sindv_pp->status & SCS_END ||
            sindv_pp->status & SCS_END_REQ ||
            sindv_pp->status & SCS_KILL_REQ ||
            sindv_pp->status & SCS_WAIT ||
            sindv_pp->status & SCS_PAUSE) {
            continue;
        }

        tapset_pp = IndvGetTapSetAdrs(sindv_pp);
        if (tapset_pp == NULL) {
            continue;
        }

        tapdat_size = tapset_pp->tapdat_size;
        tapdat_pp   = tapset_pp->tapdat_pp;

        if (global_data.play_step == PSTEP_VS) {
            dare = MBAR_PARAPPA_VS;

            if (sindv_pp->plycode == PCODE_TEACHER) {
                dare = MBAR_TEACHER_VS;
            }

            if (sindv_pp->plycode == PCODE_BOXY) {
                dare = MBAR_BOXY_VS;
                if (mbarTime < (sindv_pp->current_time + tapset_pp->taptimeEnd)) {
                    MbarReset();
                }
            }
        } else {
            if (global_data.play_step == PSTEP_HOOK) {
                dare = MBAR_PARAPPA_HOOK;
                dare = (sindv_pp->plycode != PCODE_PARA) ? MBAR_TEACHER_HOOK : dare;
            } else {
                dare = MBAR_PARAPPA;
                dare = (sindv_pp->plycode != PCODE_PARA) ? MBAR_TEACHER_HOOK : dare;
            }
        }

        if (tapset_pp->tapscode == TAPSCODE_ANSWER) {
            tapdat_size = vs_tapdat_work_cnt;
            tapdat_pp   = vs_tapdat_work;
        }

        if ((dare == MBAR_PARAPPA || dare == MBAR_PARAPPA_HOOK) && mbarTime < sindv_pp->current_time) {
            dare = MBAR_NONE;
        }

        MbarReq(dare, tapset_pp, sindv_pp->current_time, sindv_pp->scr_tap_memory, sindv_pp->scr_tap_memory_cnt, game_status.language_type, tapdat_size, tapdat_pp, sindv_pp->cursor_num);
    }
}

void allTimeCallbackTimeSetChanTempo(int time) {
    int       i;
    SCR_MAIN *scr_main_pp = score_str.stdat_dat_pp->scr_pp;

    for (i = 0; i < scr_main_pp->scr_ctrl_num; i++) {
        if (scr_main_pp->scr_ctrl_pp[i].gtime_type == GTIME_VSYNC) {
            scr_main_pp->scr_ctrl_pp[i].lineTime = time;
            TimeCallbackTimeSetChanTempo(i, time, GetLineTempo(i));

            scr_main_pp->scr_ctrl_pp[i].lineTimeFrame = (time * 3600.0f + GetLineTempo(i) * 96.0f * 0.5f) / (GetLineTempo(i) * 96.0f);
        }
    }
}

int SetIndvDrawTblLine(SCORE_INDV_STR *sindv_pp) {
    TAPSET *tapset_pp;
    int     ctime;

    if (!(sindv_pp->status & SCS_USE)) {
        return 0;
    }

    if (sindv_pp->status & SCS_END ||
        sindv_pp->status & SCS_END_REQ ||
        sindv_pp->status & SCS_KILL_REQ ||
        sindv_pp->status & SCS_WAIT ||
        sindv_pp->status & SCS_PAUSE) {
        return 0;
    }

    if (sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].gtime_type == GTIME_VSYNC) {
        return 0;
    }

    tapset_pp = IndvGetTapSetAdrs(sindv_pp);
    ctime     = sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].lineTime;
    if (tapset_pp == NULL) {
        return 0;
    }

    if (global_data.play_step == PSTEP_VS) {
        if (ctime >= (sindv_pp->current_time + tapset_pp->taptimeEnd)) {
            return 0;
        }
    } else {
        if (ctime < (sindv_pp->current_time + tapset_pp->taptimeStart) || ctime >= (sindv_pp->current_time + tapset_pp->taptimeEnd)) {
            return 0;
        }
    }

    currentTblNumber = sindv_pp->scrdat_pp->drawofs[global_data.tapLevel];
    return 0;
}

static int otehonSetCheck(void) {
    SCORE_INDV_STR *sindv_pp;
    TAPSET         *tapset_pp;
    int             i;

    sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        if (!(sindv_pp->status & SCS_USE)) {
            continue;
        }
        if (sindv_pp->status & SCS_END ||
            sindv_pp->status & SCS_END_REQ ||
            sindv_pp->status & SCS_KILL_REQ ||
            sindv_pp->status & SCS_WAIT ||
            sindv_pp->status & SCS_PAUSE_END ||
            sindv_pp->status & SCS_PAUSE) {
            continue;
        }

        tapset_pp = IndvGetTapSetAdrs(sindv_pp);
        if (tapset_pp != NULL && tapset_pp->coolup != -1) {
            if (sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].lineTime >= sindv_pp->current_time) {
                return 1;
            }
        }
    }

    return 0;
}

void ScrCtrlMainLoop(void *x) {
    int tmp_time;
    int rtime;
    int subtline;
    int i;

    if (score_str.stdat_dat_pp->play_step == PSTEP_BONUS) {
        bonusGameInit();
    }

    while (TapCt(TAPCT_TRANSCHECK, TAPCT_NONE, TAPCT_NONE)) {
        MtcWait(1);
    }

    if (GetTimeType(global_data.draw_tbl_top) != GTIME_VSYNC) {
        while (CdctrlWP2CheckBuffer()) {
            printf("cd buffer wait\n");
            MtcWait(1);
        }
    }
    MtcWait(3);

    score_str.ready_flag = TRUE;
    do {
        MtcWait(1);
    } while (!score_str.go_loop_flag);

    if (GetTimeType(global_data.draw_tbl_top) != GTIME_VSYNC) {
        CdctrlWP2Play();
        CdctrlWP2SetVolume(120);
    } else {
        TimeCallbackTimeSet(0);
        allTimeCallbackTimeSetChanTempo(0);
    }

    while (1) {
        MbarReset();
        MbarDispSceneVsDrawInit();

        outsideDrawSceneClear();

        GlobalTimeJob();
        ScrTimeRenew(score_str.stdat_dat_pp->scr_pp);

        if (global_data.demo_flagL == DEMOF_REPLAY) {
            if (pad[0].one & SCE_PADselect) {
                replayGuiOffFlag ^= 1;
            }
        }

        ScrLineSafeRefMode();

        ScrCtrlIndvJob();
        tapReqGroupPoll();

        for (i = 0; i < PR_ARRAYSIZEU(score_indv_str); i++) {
            SetIndvCdChannel(&score_indv_str[i]);
            SetIndvDrawTblLine(&score_indv_str[i]);
        }

        DrawCtrlTimeSet(ScrDrawTimeGetFrame(scrDrawLine));

        rtime = nextExamTime();
        if (rtime >= 0) {
            BallThrowSetFrame(((rtime * 3600.0f) + (score_str.stdat_dat_pp->tempo * 96.0f * 0.5f)) / (score_str.stdat_dat_pp->tempo * 96.0f));
        }

        SprClear();

        if (score_str.mbar_flag) {
            ScrMbarReq(ScrDrawTimeGet(scrMbarLine));
            outsideDrawSceneReq(MbarDispScene, 0xdc, DNUM_NON, DNUM_VRAM2, NULL);
            if (!replayGuiOffFlag) {
                if (otehonSetCheck()) {
                    outsideDrawSceneReq(MbarDispGuiScene, 0xf0, DNUM_DRAW, DNUM_DRAW, NULL);
                } else {
                    outsideDrawSceneReq(MbarDispGuiScene, 0xf0, DNUM_NON, DNUM_DRAW, NULL);
                }
            } else {
                outsideDrawSceneReq(MbarDispGuiSceneMbarArea, 0xf0, DNUM_NON, DNUM_DRAW, NULL);
            }
        } else if (!jimakuWakuOff) {
            outsideDrawSceneReq(MbarDispGuiScene, 0xf0, DNUM_NON, DNUM_DRAW, NULL);
            if (game_status.subtitle == SUBTITLE_ON) {
                ExamDispSubt();
            }
        }

        tmp_time = ((ScrDrawTimeGet(scrJimakuLine) * 3600.0f) + (score_str.stdat_dat_pp->tempo * 96.0f * 0.5f)) / (score_str.stdat_dat_pp->tempo * 96.0f);
        subtline = GetSubtLine(scrJimakuLine);
        if (subtline != -1) {
            if (game_status.subtitle == SUBTITLE_ON) {
                SubtCtrlPrint(score_str.stdat_dat_pp->jimaku_str_pp, subtline, tmp_time, game_status.language_type);
            }
        }

        if (global_data.play_step == PSTEP_GAME ||
            global_data.play_step == PSTEP_VS   ||
            global_data.play_step == PSTEP_HOOK ||
            global_data.play_step == PSTEP_BONUS) {
            sceGifPacket dbgPk;
            u_char *dbg_tbl_msg[17] = {
                "BASE",
                "LV1",  "LV2",  "LV3",  "LV4",
                "LV5",  "LV6",  "LV7",  "LV8",
                "LV9",  "LV10", "LV11", "LV12",
                "LV13", "LV14", "LV15", "LV16",
            };
            int    drtime;
            u_char timemsg[16];
            static int dbgmsg_on_off = TRUE;

            if (pad[0].one & SCE_PADi) {
                dbgmsg_on_off ^= 1;
            }

            if (dbgmsg_on_off) {
                DbgMsgInit();
                DbgMsgClear();
                CmnGifOpenCmnPk(&dbgPk);
                DbgMsgClearUserPkt(&dbgPk);
                DbgMsgPrintUserPkt(dbg_tbl_msg[global_data.tapLevel], 0x6c2, 0x79c, &dbgPk);
                drtime = ScrDrawTimeGet(scrMbarLine);
                sprintf(timemsg, "%2d.%d.%2d", (drtime / 384) + 1, ((drtime / 96) % 4) + 1, (drtime % 96) + 1);
                DbgMsgPrintUserPkt(timemsg, 0x6c2, 0x792, &dbgPk);
                CmnGifCloseCmnPk(&dbgPk, 0xf);
            }
        }

        MtcWait(1);
    }
}

GET_TIME_TYPE GetTimeType(int scr_line) {
    return score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[scr_line].gtime_type;
}

int GetTimeOfset(int scr_line) {
    return score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[scr_line].ofsCdtime;
}

int GetSubtLine(int scr_line) {
    return score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[scr_line].subtLine;
}

int GetDrawLine(int scr_line) {
    return score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[scr_line].drawLine;
}

float GetLineTempo(int scr_line) {
    return score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[scr_line].tempo;
}

void SetLineChannel(int scr_line) {
    SCR_CTRL *scr_ctrl_pp = &score_str.stdat_dat_pp->scr_pp->scr_ctrl_pp[scr_line];

    if (scr_ctrl_pp->gtime_type != GTIME_VSYNC) {
        CdctrlWP2SetChannel(scr_ctrl_pp->cdChan[0], scr_ctrl_pp->cdChan[1]);
    }
}

int SetIndvCdChannel(SCORE_INDV_STR *sindv_pp) {
    TAPSET   *tapset_pp;
    SCR_CTRL *scr_ctrl_pp;
    int       ctime;
    int       chantmp[2];

    if (!(sindv_pp->status & SCS_USE)) {
        return 0;
    }
    if (sindv_pp->status & SCS_END ||
        sindv_pp->status & SCS_END_REQ ||
        sindv_pp->status & SCS_KILL_REQ ||
        sindv_pp->status & SCS_WAIT ||
        sindv_pp->status & SCS_WAIT ||
        sindv_pp->status & SCS_PAUSE) {
        return 0;
    }

    if (sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].gtime_type == GTIME_VSYNC) {
        return 0;
    }

    tapset_pp   = IndvGetTapSetAdrs(sindv_pp);
    ctime       = sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].lineTime;
    scr_ctrl_pp = &sindv_pp->top_scr_ctrlpp[sindv_pp->useLine];

    if (tapset_pp == NULL) {
        return 0;
    }

    if (ctime < (sindv_pp->current_time + tapset_pp->taptimeStart) || ctime >= (sindv_pp->current_time + tapset_pp->taptimeEnd)) {
        return 0;
    }

    chantmp[0] = tapset_pp->chan[0];
    chantmp[1] = tapset_pp->chan[1];

    if (tapset_pp->chan[0] == -2) {
        return 1;
    }

    if (tapset_pp->chan[0] == -1) {
        if (scr_ctrl_pp->scr_chan_auto_size != 0) {
            int i;
            int haba = tapset_pp->taptimeEnd - tapset_pp->taptimeStart;

            for (i = 0; i < scr_ctrl_pp->scr_chan_auto_size; i++) {
                chantmp[0] = scr_ctrl_pp->scr_chan_auto_pp[i].chan[0];
                chantmp[1] = scr_ctrl_pp->scr_chan_auto_pp[i].chan[1];
                if (haba < scr_ctrl_pp->scr_chan_auto_pp[i].time) {
                    break;
                }
            }
        } else {
            chantmp[0] = scr_ctrl_pp->cdChan[0];
            chantmp[1] = scr_ctrl_pp->cdChan[1];
        }
    }

    CdctrlWP2SetChannel(chantmp[0], chantmp[1]);
    return 1;
}

int CheckIndvCdChannel(SCORE_INDV_STR *sindv_pp, u_char *chantmp) {
    TAPSET *tapset_pp;

    chantmp[0] = 0;
    chantmp[1] = 1;

    if (!(sindv_pp->status & SCS_USE)) {
        return 0;
    }

    if (sindv_pp->top_scr_ctrlpp[sindv_pp->useLine].gtime_type == GTIME_VSYNC) {
        return 0;
    }

    tapset_pp = IndvGetTapSetAdrs(sindv_pp);
    if (tapset_pp == NULL) {
        return 0;
    }

    chantmp[0] = tapset_pp->chan[0];
    chantmp[1] = tapset_pp->chan[1];

    if (tapset_pp->chan[0] == -2) {
        return 0;
    }

    if (tapset_pp->chan[0] == -1) {
        SCR_CTRL *scr_ctrl_pp = &sindv_pp->top_scr_ctrlpp[sindv_pp->useLine];

        if (scr_ctrl_pp->scr_chan_auto_size != 0) {
            int i;
            int haba = tapset_pp->taptimeEnd - tapset_pp->taptimeStart;

            for (i = 0; i < scr_ctrl_pp->scr_chan_auto_size; i++) {
                chantmp[0] = scr_ctrl_pp->scr_chan_auto_pp[i].chan[0];
                chantmp[1] = scr_ctrl_pp->scr_chan_auto_pp[i].chan[1];
                if (haba < scr_ctrl_pp->scr_chan_auto_pp[i].time) {
                    break;
                }
            }
        } else {
            chantmp[0] = scr_ctrl_pp->cdChan[0];
            chantmp[1] = scr_ctrl_pp->cdChan[1];
        }
    }

    return 1;
}

void ScrCtrlInit(STDAT_DAT *sdat_pp, void *data_top) {
    int           i, j;
    int           add_move;
    GET_TIME_TYPE ttype;

    score_str.int_top      = data_top;
    score_str.stdat_dat_pp = sdat_pp;
    ScrTapCtrlInit(data_top);

    PrSetStage(global_data.play_stageL);
    PrRestartMenderer();

    if (global_data.play_step == PSTEP_HOOK) {
        inCmnHook2GameSave(tapLevelChangeSub());
        MbarHookUseInit();
        MbarNikoHookUse();
    } else {
        MbarHookUnUse();
        if (global_data.play_step == PSTEP_VS) {
            MbarNikoVsUse();
        } else {
            MbarNikoUnUse();
        }
    }

    SprInit();
    mccReqCtrlClr();
    tapReqGroupInit();

    if (global_data.play_step == PSTEP_VS) {
        add_move = tapLevelChangeSub();
        if (global_data.demo_flagL == DEMOF_REPLAY) {
            add_move = mccReqLvlGet();
        } else {
            mccReqLvlSet(add_move);
        }
        global_data.tapLevel = add_move;
    }

    MbarMemberClear(clearStageCheck());

    currentTblNumber = 0;
    outsideDrawSceneClear();

    if (score_str.stdat_dat_pp->scr_pp->sndrec_num != 0) {
        ScrTapDataTrans(score_str.stdat_dat_pp->scr_pp->sndrec_pp, 0, score_str.int_top);
    }

    TapCt(TAPCT_SETEFFECTMODE, SD_REV_MODE_OFF, 0x1fff);
    TapCt(TAPCT_SETEFFECTVOL,  PR_CONCAT(0x3fff, 0x3fff), TAPCT_NONE);

    for (i = 0; i < PR_ARRAYSIZE(score_str.stdat_dat_pp->sndfile); i++) {
        if (score_str.stdat_dat_pp->sndfile[i].fname != NULL) {
            while (!CdctrlSerch(&score_str.stdat_dat_pp->sndfile[i])) {
                printf("file serch error![%s]\n", score_str.stdat_dat_pp->sndfile[i].fname);
            }
        }
    }
    FlushCache(WRITEBACK_DCACHE);

    ScrCtrlIndvInit(sdat_pp);
    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++) {
        ScrCtrlIndvNextRead(&score_indv_str[i], TRUE);
    }

    ttype = GetTimeType(global_data.draw_tbl_top);
    if (ttype == GTIME_VSYNC) {
        GlobalTimeJobChange(FGF_VSYNC);
    } else {
        GlobalTimeJobChange(FGF_CD);
        if (score_str.stdat_dat_pp->play_step != PSTEP_XTR) {
            CdctrlWP2Set(&score_str.stdat_dat_pp->sndfile[ttype]);
        } else {
            CdctrlXTRset(&score_str.stdat_dat_pp->sndfile[ttype], UsrMemAllocNext());
        }
    }

    SetLineChannel(global_data.draw_tbl_top);

    score_str.ready_flag = FALSE;

    scrJimakuLine = global_data.draw_tbl_top;
    scrMbarLine   = global_data.draw_tbl_top;
    scrDrawLine   = global_data.draw_tbl_top;

    score_str.go_loop_flag = FALSE;
    MtcExec(ScrCtrlMainLoop, MTC_TASK_SCORECTRL);

    score_str.mbar_flag = FALSE;

    if (GlobalMendererUseCheck()) {
        PrInitializeMenderer(0x3ded, GetIntAdrsCurrent(INTNUM_NOODLE_TEX), DrawGetFbpPos(DNUM_VRAM2));
    }

    if (score_str.stdat_dat_pp->play_step != PSTEP_SERIAL &&
        score_str.stdat_dat_pp->play_step != PSTEP_BONUS &&
        score_str.stdat_dat_pp->play_step != PSTEP_XTR) {
        for (j = 0; j < 4; j++) {
            /* Empty */
        }

        score_str.mbar_flag = TRUE;
        MbarInit(score_str.stdat_dat_pp->stage);
        ExamDispReset();
    }

    SubtCtrlInit(GetIntAdrsCurrent(INTNUM_SUBT_CODE),
        /* Serial types (Cutscene, bonus stage, Boxy Boy practice) */
        (score_str.stdat_dat_pp->play_step == PSTEP_SERIAL) |
        (score_str.stdat_dat_pp->play_step == PSTEP_BONUS) |
        (score_str.stdat_dat_pp->play_step == PSTEP_HOOK) |
        (score_str.stdat_dat_pp->play_step == PSTEP_XTR)
    );

    titleStartKey = FALSE;
    fadeoutStartKey = FALSE;
    gameEndWaitLoop = FALSE;
    replayGuiOffFlag = FALSE;
    followTapInit();

    jimakuWakuOff = FALSE;
}

void ScrCtrlQuit(void) {
    useAllClearKeySnd();
    TapCt(TAPCT_TRANSCLEAR, TAPCT_NONE, TAPCT_NONE);

    MtcKill(MTC_TASK_SCORECTRL);
}

int ScrCtrlInitCheck(void) {
    return score_str.ready_flag;
}

void ScrCtrlGoLoop(void) {
    score_str.go_loop_flag = TRUE;
}

int ScrEndCheckScore(void) {
    int             i;
    SCORE_INDV_STR *sindv_pp = score_indv_str;

    for (i = 0; i < PR_ARRAYSIZE(score_indv_str); i++, sindv_pp++) {
        if (!(sindv_pp->status & SCS_USE)) {
            continue;
        }
        if (sindv_pp->status & SCS_END) {
            printf("end end end[%d] line time[%d]\n", i, sindv_pp->top_scr_ctrlpp[i].lineTime);
            return 1;
        }
    }

    return 0;
}

int ScrEndCheckTitle(void) {
    return titleStartKey;
}

int ScrEndCheckFadeOut(void) {
    return fadeoutStartKey;
}

int ScrEndWaitLoop(void) {
    return gameEndWaitLoop;
}

static void bonusGameInit(void) {
    WorkClear(&bng_str, sizeof(bng_str));

    bng_str.st_time   = 0;
    bng_str.end_time  = 18048;
    bng_str.bonus_cnt = 0;
}

static int bonusGameCntPls(void) {
    bng_str.bonus_cnt++;
    return bng_str.bonus_cnt;
}

static void bonusPointSave(void) {
    ingame_common_str.BonusScore = bng_str.ok_cnt - bng_str.ng_cnt;
    if (ingame_common_str.BonusScore < 0) {
        ingame_common_str.BonusScore = 0;
    }
}

void bngTapEventCheck(SCORE_INDV_STR *sindv_pp, int num, int id) {
    TAPSET *tapset_pp;
    TAPDAT *tapdat_pp;

    if (sindv_pp->scrdat_pp == NULL) {
        return;
    }

    tapset_pp = &sindv_pp->scrdat_pp->tapstr[global_data.tapLevel].tapset_pp[sindv_pp->tapset_pos];
    tapdat_pp = &tapset_pp->tapdat_pp[num];

    if (tapdat_pp->tapct[0].actor != -1) {
        DrawTapReqTbl(tapdat_pp->tapct[0].actor, Pcode2Pindex(sindv_pp->plycode), NULL);
    }

    if (tapdat_pp->tapct[0].sound != -1) {
        ScrTapReq(sindv_pp->sndId, id, tapdat_pp->tapct[0].sound);
    }
}

static void bonusGameParaReq(BNG_ACT_P_ENUM actnum) {
    bngTapEventCheck(&score_indv_str[2], actnum, 0);
}

static void bonusGameKoamaReq(int kotamaNum, BNG_ACT_K_ENUM actnum) {
    bngTapEventCheck(&score_indv_str[1], actnum + kotamaNum, kotamaNum + 1);
}

static int bonus_minus_point_sub(int wtime) {
    if (wtime < 8) {
        return 9;
    } else if (wtime < 13) {
        return 13;
    } else if (wtime < 20) {
        return 18;
    } else {
        return 24;
    }
}

static int bonus_pls_point_sub(int wtime) {
    if (wtime < 3) {
        return 26;
    } else if (wtime < 6) {
        return 15;
    } else if (wtime < 9) {
        return 9;
    } else {
        return 5;
    }
}

static void bonusGameCtrl(int time) {
    int         actnum;
    int         mochimono_ofs;
    BNG_KOTAMA *bng_kotama_pp;

    mochimono_ofs = ingame_common_str.bonusType * 4;

    bng_str.now_time = time;

    if (time < bng_str.st_time || time >= bng_str.end_time) {
        return;
    }

    actnum = GetKeyCode2Index(pad[0].one);
    switch (actnum) {
    case KiTR:
    case KiCI:
    case KiXX:
    case KiSQ:
        actnum -= 1;
        bng_kotama_pp = &bng_str.bng_kotama[actnum];

        switch (bng_kotama_pp->bng_kotama_act_enum) {
        case BNGKA_LIFT:
        case BNGKA_LIFT_NG: {
            int kotamatime = bng_kotama_pp->wait_time;

            bonusGameKoamaReq(actnum, BNGAKE_4_TOP);

            bng_kotama_pp->wait_time = 0;
            bng_kotama_pp->bng_kotama_act_enum = BNGKA_BLOW;

            bonusGameParaReq(actnum + BNGAPE_A_NG);

            bng_str.ng_cnt += bonus_minus_point_sub(kotamatime);
            bng_str.renzoku_cnt = 0;            
            break;
        }
        case BNGKA_LIFTED: {
            int kotamatime = bng_kotama_pp->wait_time;

            bonusGameKoamaReq(actnum, mochimono_ofs + BNGAKE_3_TOP);

            bng_kotama_pp->wait_time = 0;
            bng_kotama_pp->bng_kotama_act_enum = BNGKA_BREAK;

            bonusGameParaReq(actnum);

            bng_str.ok_cnt += bonus_pls_point_sub(kotamatime);
            bng_str.renzoku_cnt++;
            break;
        }
        default:
            bonusGameParaReq(actnum + BNGAPE_A_NG);
            break;
        }
    }

    {
        /* A second bng_kotama_pp, as in the original debug info. */
        int         i;
        BNG_KOTAMA *bng_kotama_pp = bng_str.bng_kotama;

        for (i = 0; i < 4; i++, bng_kotama_pp++) {
            bng_kotama_pp->wait_time++;

            switch (bng_kotama_pp->bng_kotama_act_enum) {
            case BNGKA_NOTHING:
                bonusGameKoamaReq(i, BNGAKE_5_TOP);
                bng_kotama_pp->wait_time = 0;
                bng_kotama_pp->bng_kotama_act_enum = BNGKA_LIFT;
                break;
            case BNGKA_LIFT: {
                int randam_num;

                if (bng_kotama_pp->wait_time == 1) {
                    randam_num = (rand() % 130);
                    randam_num -= bng_str.renzoku_cnt;

                    if (randam_num < 0) {
                        randam_num = 0;
                    }

                    randam_num += 10;

                    bng_kotama_pp->wait_next_time = randam_num;
                } else {
                    randam_num = bng_kotama_pp->wait_next_time;
                }

                if (bng_kotama_pp->wait_time > randam_num) {
                    if ((rand() % 2) != 0) {
                        bonusGameKoamaReq(i, mochimono_ofs);
                        bng_kotama_pp->bng_kotama_act_enum = BNGKA_LIFTED;
                    } else {
                        bonusGameKoamaReq(i, BNGAKE_2_TOP);
                        bng_kotama_pp->bng_kotama_act_enum = BNGKA_LIFT_NG;
                    }

                    bng_kotama_pp->wait_time = 0;
                }

                break;
            }
            case BNGKA_LIFT_NG:
                if (bng_kotama_pp->wait_time >= 24) {
                    bng_kotama_pp->bng_kotama_act_enum = BNGKA_NOTHING;
                    bng_kotama_pp->wait_time = 0;
                }
                break;
            case BNGKA_BLOW:
                if (bng_kotama_pp->wait_time >= 180) {
                    bng_kotama_pp->bng_kotama_act_enum = BNGKA_NOTHING;
                    bng_kotama_pp->wait_time = 0;
                }
                break;
            case BNGKA_BREAK:
                if (bng_kotama_pp->wait_time >= 36) {
                    bng_kotama_pp->bng_kotama_act_enum = BNGKA_NOTHING;
                    bng_kotama_pp->wait_time = 0;
                }
                break;
            case BNGKA_LIFTED:
                break;
            }
        }
    }
}

static u_long hex2dec(u_long data) {
    u_long ret = 0;
    u_int  i;

    for (i = 0; i < 16; i++) {
        if (data == 0) {
            break;
        }

        ret |= (data % 10) << (i * 4);
        data /= 10;
    }

    return ret;
}

static void bnNumberDisp(sceGifPacket *gif_pp, long score, short x, short y, int keta, int tate, int type) {
    int          i;
    u_char       num;
    int          first_f = FALSE;
    BN_NUM_TYPE *bn_num_type_pp;

    bn_num_type_pp = &bn_num_type[type];

    score = hex2dec(score);

    sceGifPkAddGsAD(gif_pp, SCE_GS_TEX0_1, bn_num_type_pp->tim2_dat_pp->GsTex0);
    sceGifPkAddGsAD(gif_pp, SCE_GS_TEX1_1, bn_num_type_pp->tim2_dat_pp->GsTex1);
    sceGifPkAddGsAD(gif_pp, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(FALSE));

    for (i = 0; i < keta; i++) {
        /* score is BCD after hex2dec: one digit per nibble, most significant first. */
        num = (score >> ((keta - 1 - i) * 4)) & 0xf;

        if (num != 0 || first_f || i == (keta - 1)) {
            sceGifPkAddGsAD(gif_pp, SCE_GS_UV, SCE_GS_SET_UV(bn_num_type_pp->map[num][0] << 4, bn_num_type_pp->map[num][1] << 4));
            sceGifPkAddGsAD(gif_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ((x << 4) + GS_X_COORD(0), (y << 4) + GS_Y_COORD(0), 1));

            sceGifPkAddGsAD(gif_pp, SCE_GS_UV, SCE_GS_SET_UV((bn_num_type_pp->map[num][0] + bn_num_type_pp->w) << 4, (bn_num_type_pp->map[num][1] + bn_num_type_pp->h) << 4));
            sceGifPkAddGsAD(gif_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ(((x + (GS_X_COORD(0)>>4)) + bn_num_type_pp->w) << 4, ((y + (GS_Y_COORD(0)>>4)) + bn_num_type_pp->h) << 4, 1));

            first_f = TRUE;
        }

        if (tate) {
            y += bn_num_type_pp->h;
        } else {
            x += bn_num_type_pp->w;
        }
    }
}

static void bonusScoreDraw(void) {
    long         scr_stg;
    long         scr_bn;
    long         scr_add;

    sceGifPacket bn_gif;
    VCLR_PARA    vclr_para = {};

    DrawVramClear(&vclr_para, 0, FALSE, DNUM_NON, DNUM_VRAM2);
    ChangeDrawArea(DrawGetDrawEnvP(DNUM_VRAM2));

    CmnGifADPacketMake(&bn_gif, NULL);

    sceGifPkAddGsAD(&bn_gif, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(&bn_gif, SCE_GS_TEST_1, GS_TEST_OFF);
    sceGifPkAddGsAD(&bn_gif, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 0x80));
    sceGifPkAddGsAD(&bn_gif, SCE_GS_CLAMP_1, GS_CLAMP_EDGES);
    sceGifPkAddGsAD(&bn_gif, SCE_GS_PABE, 0);
    sceGifPkAddGsAD(&bn_gif, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 0x80));

    scr_bn  = ingame_common_str.BonusScore;
    scr_stg = ingame_common_str.SingleScore;
    scr_add = scr_stg + scr_bn;

    bnNumberDisp(&bn_gif, scr_stg,  0,  0, 5, TRUE, BN_KANJI_TXT);
    bnNumberDisp(&bn_gif, scr_stg, 96, 32, 5, FALSE, BN_SUUJI_TXT);

    bnNumberDisp(&bn_gif, scr_bn,  32,  0, 5, TRUE, BN_KANJI_TXT);
    bnNumberDisp(&bn_gif, scr_bn,  96, 56, 5, FALSE, BN_SUUJI_TXT);

    bnNumberDisp(&bn_gif, scr_add, 64,  0, 5, TRUE, BN_KANJI_TXT);
    bnNumberDisp(&bn_gif, scr_add, 96, 80, 5, FALSE, BN_SUUJI_TXT);

    bnNumberDisp(&bn_gif, ingame_common_str.BonusStage, 96,  0, 1, FALSE, BN_KANJI_TXT);
    bnNumberDisp(&bn_gif, ingame_common_str.BonusStage, 128, 0, 1, FALSE, BN_SUUJI_TXT);

    CmnGifADPacketMakeTrans(&bn_gif);
}

static void set_lero_gifset(sceGifPacket *gifpk_pp, LERO_TIM2_PT *let2_pp, short xp, short yp) {
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_UV,   SCE_GS_SET_UV(let2_pp->u0 << 4, let2_pp->v0 << 4));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((xp + 2048) << 4,
                                                           (yp + 2048) << 4, 1));

    sceGifPkAddGsAD(gifpk_pp, SCE_GS_UV,   SCE_GS_SET_UV(let2_pp->u1 << 4, let2_pp->v1 << 4));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((xp + 2048 + let2_pp->w) << 4, 
                                                           (yp + 2048 + let2_pp->h) << 4, 1));
}

static void LessonRoundDisp(SCRRJ_LESSON_ROUND_ENUM type) {
    sceGifPacket gifpk;
    TIM2_DAT    *tim2_dat_pp;
    int          i;

    if (type >= SCRRJ_LR_MAX) {
        return;
    }

    tim2_dat_pp = lessonTim2InfoGet();
    GS_REG_VIEW(sceGsTex0, tim2_dat_pp->GsTex0).CBP = GS_REG_VIEW(sceGsTex0, lessonCl2InfoGet(type)->GsTex0).CBP;

    CmnGifOpenCmnPk(&gifpk);
    ChangeDrawAreaSetGifTag(DrawGetDrawEnvP(DNUM_DRAW), &gifpk);

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEST_1, GS_TEST_ALPHA_NONZERO);
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 0x80));
    sceGifPkAddGsAD(&gifpk, SCE_GS_CLAMP_1, GS_CLAMP_EDGES);
    sceGifPkAddGsAD(&gifpk, SCE_GS_PABE, 0);
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 0x80));
    sceGifPkAddGsAD(&gifpk, SCE_GS_ALPHA_1, GS_ALPHA_BLEND);
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEX0_1, tim2_dat_pp->GsTex0);
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEX1_1, tim2_dat_pp->GsTex1);
    sceGifPkAddGsAD(&gifpk, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(TRUE));

    for (i = 0; i < 2; i++) {
        set_lero_gifset(&gifpk, &lero_tim2_pt[lero_pos_str[type][i].tim2_num], lero_pos_str[type][i].posx, lero_pos_str[type][i].posy);
    }

    CmnGifCloseCmnPk(&gifpk, 2);
}
