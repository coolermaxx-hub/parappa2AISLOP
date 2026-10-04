#include "main/mbar.h"

#include "main/cmnfile.h"
#include "main/sprite.h"

#include "os/cmngifpk.h"
#include "os/syssub.h"
#include "os/system.h"
#include "os/tim2.h"

#include "prlib/prlib.h"

#include <math.h>
#include <stdio.h>

/* sdata 399558 */ static NIKO_CHAN_STR *niko_chan_str_pp = NULL;
/* sdata 39955c */ static int niko_chan_str_cnt = 0;
/* sdata 399560 */ static int hook_use_flag = 0;
/* sdata 399564 */ static int exam_disp_cursor_timer = -1;
/* sdata 399568 */ static int scoreTentouFlag = 0;
/* sdata 39956c */ int otehonAniCnt = 0; /* static */
/* sdata 399570 */ int othon_frame = 0; /* static */
/* sdata 399574 */ static int vs_mouse_disp_flag = 0;
/* sdata 399578 */ static int mbar_pos_y_ofs = 0;
/* data 17c2b8 */ extern GAME_STATUS game_status; /* static */
static GLOBAL_PLY *exam_global_ply[4];
static int exam_global_ply_current_ply[4];
static int metFrameCnt[3];
static int metFrameCntLight[3];
static u_char scr_tenmetu_col_dat[4][3];
static int conditionFramCnt[4];
static int vsScoreMove[4];
static int vsScoreAni[4];
static VS_SCR_CTRL vs_scr_ctrl[4];
static MBAR_REQ_STR mbar_req_str[5];
static sceGifPacket mbar_gif;
static GLOBAL_PLY *exam_global_ply_current;
/* sdata 399584 */ enum SCR_TENMETU_ENUM {
    SCR_TENMETU_NORMAL = 0,
    SCR_TENMETU_PL = 1,
    SCR_TENMETU_MI = 2,
    SCR_TENMETU_BLACK = 3,
    SCR_TENMETU_MAX = 4
} SCR_TENMETU_ENUM = SCR_TENMETU_NORMAL;
static int mbar_ctrl_time;
static int mbar_ctrl_stage;
static int mbar_ctrl_stage_selT;
static PR_SCENEHANDLE guime_hdl;
static PR_CAMERAHANDLE guime_camera_hdl;

static void   clrColorBuffer(int id);
static void   NikoReset(void);
static void   MbarNikoDisp(sceGifPacket *gifpk_pp);
static void   MbarHookPoll(void);
static int    vsScr2Move(long scr);
static void   vsAnimationPoll(void);
static void   metColorInit(void);
static void   metColorSet(EXAM_TYPE exam_type, float per);
static void   ExamDispOn(void);
static u_long hex2dec(u_long data);
static u_long hex2decPlMi(long data);
static void   examScoreSet(sceGifPacket *ex_gif_pp);
static void   examLevelDisp(sceGifPacket *ex_gif_pp);
static void   MbarCl1CharSet(int col_num, int moto_num);
static void   MbarCharSetSub(void);
static int    MbarGetDispPosX(int tick);
static int    MbarGetDispPosY(int tick);
static int    MbarGetTimeArea(MBAR_REQ_STR *mr_pp);
static int    MbarGetTimeArea2(MBAR_REQ_STR *mr_pp);
static int    MbarGetStartTime(MBAR_REQ_STR *mr_pp);
static int    MbarGetEndTime(MBAR_REQ_STR *mr_pp);
static int    MbarGetStartTap(MBAR_REQ_STR *mr_pp);
/*static*/ void   MbarOthSet(MBAR_REQ_STR *mr_pp);
/*static*/ void   MbarCurSet(MBAR_REQ_STR *mr_pp);
static int    MbarTapSubt(MBAR_REQ_STR *mr_pp);
/*static*/ void   MbarPosOffsetSet(MBAR_REQ_STR *mr_pp);
static void   mbar_othon_frame_set(MBAR_REQ_STR *mr_pp);
static void   guidisp_init_pr(void);
static void   guidisp_draw_quit(int drapP);

/* GS texture descriptors for the mbar sprites. */
static TIM2_DAT tim2spr_tbl[63] = {
    { SCE_GS_SET_TEX0(0x3cb6, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d1a, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cb8, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d1b, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3ceb, 2, SCE_GS_PSMT4, 4, 4, 1, 0, 0x3d3e, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 16, 16, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c72, 2, SCE_GS_PSMT4, 4, 4, 1, 0, 0x3d34, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 16, 16, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c85, 2, SCE_GS_PSMT4, 4, 4, 1, 0, 0x3d35, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 16, 16, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c94, 2, SCE_GS_PSMT4, 4, 4, 1, 0, 0x3d36, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 16, 16, {0, 0} },
    { SCE_GS_SET_TEX0(0x3ce4, 2, SCE_GS_PSMT4, 4, 4, 1, 0, 0x3d37, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 16, 16, {0, 0} },
    { SCE_GS_SET_TEX0(0x3ce5, 2, SCE_GS_PSMT4, 4, 4, 1, 0, 0x3d38, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 16, 16, {0, 0} },
    { SCE_GS_SET_TEX0(0x3ce6, 2, SCE_GS_PSMT4, 4, 4, 1, 0, 0x3d39, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 16, 16, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cba, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d1c, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cbc, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d1d, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cbe, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d1e, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cc0, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d1f, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cc6, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d23, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c6a, 2, SCE_GS_PSMT4, 3, 3, 1, 0, 0x3d32, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 8, 8, {0, 0} },
    { SCE_GS_SET_TEX0(0x3ce7, 2, SCE_GS_PSMT4, 4, 4, 1, 0, 0x3d3a, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 16, 16, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c23, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d09, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c27, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d0a, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c35, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d0b, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c37, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d0c, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c53, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d0d, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c6c, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d0e, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c6e, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d0f, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3ca0, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d10, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3ca4, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d11, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b8b, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d08, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3ccc, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d26, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c31, 4, SCE_GS_PSMT4, 8, 5, 1, 0, 0x3cf9, SCE_GS_PSMCT16, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 160, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3bf9, 2, SCE_GS_PSMT8, 7, 7, 1, 0, 0x3cf5, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 120, 88, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d3f, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d40, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d41, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d42, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d43, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d44, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d45, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d03, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d03, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d03, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d01, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d01, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d01, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d02, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d02, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d02, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d46, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d47, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d48, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d49, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 0, SCE_GS_PSMT4, 1, 1, 1, 0, 0x3d4a, SCE_GS_PSMCT32, 0, 0, 1), 0x0, 0x0, 0x0, 0, 0, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cea, 2, SCE_GS_PSMT4, 3, 3, 1, 0, 0x3d3d, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 8, 8, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c70, 2, SCE_GS_PSMT4, 3, 3, 1, 0, 0x3d33, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 8, 8, {0, 0} },
    { SCE_GS_SET_TEX0(0x3b6d, 6, SCE_GS_PSMT4, 9, 5, 1, 0, 0x3cec, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 376, 32, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c3d, 2, SCE_GS_PSMT8, 7, 6, 1, 0, 0x3cfa, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 96, 64, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cc8, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d24, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 32, 32, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cca, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d25, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 32, 32, {0, 0} },
    { SCE_GS_SET_TEX0(0x3caa, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d14, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 32, 32, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cac, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d15, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 32, 32, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cae, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d16, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 32, 32, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cb0, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d17, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 32, 32, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cb2, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d18, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3cb4, 2, SCE_GS_PSMT4, 5, 5, 1, 0, 0x3d19, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 24, 24, {0, 0} },
    { SCE_GS_SET_TEX0(0x3c96, 2, SCE_GS_PSMT4, 7, 6, 1, 0, 0x3d04, SCE_GS_PSMCT32, 0, 0, 1), GS_TEX1_LINEAR, 0x0, 0x0, 72, 64, {0, 0} },
};

static u_int tmpColor[16] = {0};

static NIKO_CHAN_STR niko_chan_str_hook[] = {
    { 146, 169, NIKO_KAGE },
    { 182, 169, NIKO_KAGE },
    { 218, 169, NIKO_KAGE },
    { 254, 169, NIKO_KAGE },
    { 290, 169, NIKO_KAGE },
    { 326, 169, NIKO_KAGE },
    { 362, 169, NIKO_KAGE },
    { 398, 169, NIKO_KAGE },
    { 434, 169, NIKO_KAGE },
    { 470, 169, NIKO_KAGE },
};

static NIKO_CHAN_STR niko_chan_str_vs[] = {
    { 24, 11, NIKO_KAGE },
    { 24, 25, NIKO_KAGE },
    { 24, 39, NIKO_KAGE },
    { 592, 11, NIKO_KAGE },
    { 592, 25, NIKO_KAGE },
    { 592, 39, NIKO_KAGE },
};

static MBHOOK_STR mbhook_str[2] = {
    { 90, 91, 0 },
    { 92, 93, 0 },
};

static u_int hook_fr_dat[16] = { 0, 38, 76, 128, 128, 128, 115, 102, 89, 76, 64, 51, 38, 25, 12, 0 };

static u_char scr_tenmetu_col[4][3] = {
    { 0xff, 0x80, 0x00 },
    { 0x80, 0x80, 0xff },
    { 0xff, 0x40, 0x40 },
    { 0xff, 0x80, 0x00 },
};

static METCOL_STR metcol_str[3] = {
    { 82, 81, 83 },
    { 85, 84, 86 },
    { 88, 87, 89 },
};

static MBA_CHAR_DATA mba_char_data[] = {
    { &tim2spr_tbl[0], 0.0f, 0.0f },
    { &tim2spr_tbl[12], 1.0f, 1.0f },
    { &tim2spr_tbl[9], 1.0f, 1.0f },
    { &tim2spr_tbl[10], 1.0f, 1.0f },
    { &tim2spr_tbl[13], 1.0f, 1.0f },
    { &tim2spr_tbl[1], 1.0f, 1.0f },
    { &tim2spr_tbl[11], 1.0f, 1.0f },
    { &tim2spr_tbl[0], 1.0f, 1.0f },
    { &tim2spr_tbl[7], 1.0f, 1.0f },
    { &tim2spr_tbl[4], 1.0f, 1.0f },
    { &tim2spr_tbl[5], 1.0f, 1.0f },
    { &tim2spr_tbl[8], 1.0f, 1.0f },
    { &tim2spr_tbl[3], 1.0f, 1.0f },
    { &tim2spr_tbl[6], 1.0f, 1.0f },
    { &tim2spr_tbl[2], 1.0f, 1.0f },
    { &tim2spr_tbl[34], 1.0f, 1.0f },
    { &tim2spr_tbl[31], 1.0f, 1.0f },
    { &tim2spr_tbl[32], 1.0f, 1.0f },
    { &tim2spr_tbl[35], 1.0f, 1.0f },
    { &tim2spr_tbl[30], 1.0f, 1.0f },
    { &tim2spr_tbl[33], 1.0f, 1.0f },
    { &tim2spr_tbl[29], 1.0f, 1.0f },
    { &tim2spr_tbl[14], 1.0f, 1.0f },
    { &tim2spr_tbl[15], 1.0f, 1.0f },
    { &tim2spr_tbl[0], 0.0f, 0.0f },
    { &tim2spr_tbl[16], 1.0f, 1.0f },
    { &tim2spr_tbl[17], 1.0f, 1.0f },
    { &tim2spr_tbl[18], 1.0f, 1.0f },
    { &tim2spr_tbl[19], 1.0f, 1.0f },
    { &tim2spr_tbl[20], 1.0f, 1.0f },
    { &tim2spr_tbl[21], 1.0f, 1.0f },
    { &tim2spr_tbl[22], 1.0f, 1.0f },
    { &tim2spr_tbl[22], 1.0f, 1.0f },
    { &tim2spr_tbl[23], 1.0f, 1.0f },
    { &tim2spr_tbl[24], 1.0f, 1.0f },
    { &tim2spr_tbl[26], 1.0f, 1.0f },
    { &tim2spr_tbl[50], 1.0f, 1.0f },
    { &tim2spr_tbl[51], 1.0f, 1.0f },
};

static u_char colp[][3] = {
    { 0, 0, 0 },
    { 25, 50, 25 },
    { 50, 25, 25 },
    { 35, 25, 50 },
    { 50, 25, 40 },
    { 50, 50, 25 },
    { 25, 25, 50 },
    { 0, 0, 0 },
};

static void (*marSetPrgTbl[])(MBAR_REQ_STR*) = { MbarPosOffsetSet, MbarBackSet, MbarOthSet, MbarCurSet };

static GUIMAP guimap[] = {
    { 22, -1, -1, 0, 0, 0, NULL, NULL },
    { 44, 63, 64, 0, 0, 0, metFrameCnt, metFrameCntLight },
    { 45, 67, 68, 0, 0, 0, &metFrameCnt[1], &metFrameCntLight[1] },
    { 46, 69, 70, 0, 0, 0, &metFrameCnt[2], &metFrameCntLight[2] },
    { 42, -1, -1, 0, 0, 0, NULL, NULL },
    { 43, -1, -1, 0, 0, 0, NULL, NULL },
    { 14, 52, -1, 0, 0, 0, &otehonAniCnt, NULL },
    { 13, 50, -1, 0, 0, 0, &othon_frame, NULL },
    { 23, 55, 56, 0, 0, 0, vsScoreAni, vsScoreMove },
    { 24, 57, 58, 0, 0, 0, &vsScoreAni[1], &vsScoreMove[1] },
};

static int guimap_single[] = { 0, 1, 4, 0 };
static int guimap_vs[] = { 1, 4, 8, 9 };
static int guimap_sr[] = { 5, 0 };
static int guimap_hk[] = { 5, 0 };

void examCharSet(EX_CHAR_DISP *ecd_pp, sceGifPacket *gifpk_pp) {
    int wl, hl;
    int xp, yp;

    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_PRMODECONT, 1);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_CLAMP_1, GS_CLAMP_EDGES);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_ALPHA_1, GS_ALPHA_BLEND);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEX0_1, ecd_pp->GsTex0);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEX1_1, ecd_pp->GsTex1);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_PABE, SCE_GS_SET_PABE(PR_REGS(ecd_pp).pabe));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_FBA_1, SCE_GS_SET_FBA(PR_REGS(ecd_pp).fba));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_COLCLAMP, 1);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(ecd_pp->kido[0], ecd_pp->kido[1], ecd_pp->kido[2], 128, GS_Q_ONE));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEST_1, GS_TEST_ALPHA_NONZERO);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 128));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(ecd_pp->alpha));

    wl = ecd_pp->w * ecd_pp->scalex * 16.0f;
    hl = ecd_pp->h * ecd_pp->scaley * 16.0f;
    xp = ecd_pp->x - (int)(ecd_pp->cx * ecd_pp->scalex);
    yp = ecd_pp->y - (int)(ecd_pp->cy * ecd_pp->scaley);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_UV, SCE_GS_SET_UV(ecd_pp->u << 4, ecd_pp->v << 4));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(xp & 0xffff, yp & 0xffff, 1));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_UV, SCE_GS_SET_UV(((ecd_pp->u + ecd_pp->w) << 4) + 8, ((ecd_pp->v + ecd_pp->h) << 4) + 8));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((xp + wl) & 0xffff, (yp + hl) & 0xffff, 1));
}

static void clrColorBuffer(int id) {
    static sceGsLoadImage tp;
    TIM2_DAT *tim2_dat_pp;
    u_char   *tr_adr;
    u_int     cpsm, cbp;

    tim2_dat_pp = &tim2spr_tbl[id];
    tr_adr = (u_char*)&tmpColor;

    cpsm = PR_TEX0(tim2_dat_pp).CPSM;
    cbp = PR_TEX0(tim2_dat_pp).CBP;

    sceGsSetDefLoadImage(&tp, cbp, 1, cpsm, 0, 0, 8, 2);
    FlushCache(WRITEBACK_DCACHE);

    sceGsExecLoadImage(&tp, (u_long128*)tr_adr);
    sceGsSyncPath(0, 0);
}

void MbarMemberClear(int stg) {
    if (stg < 6) {
        clrColorBuffer(59);
    }
    if (stg < 4) {
        clrColorBuffer(58);
    }
    if (stg < 3) {
        clrColorBuffer(57);
    }
    if (stg < 2) {
        clrColorBuffer(56);
    }
    if (stg < 1) {
        clrColorBuffer(55);
    }
}

void examCharBasic(EX_CHAR_DISP *ecd_pp, TIM2_DAT *tim2_dat_pp) {
    ecd_pp->GsTex0 = tim2_dat_pp->GsTex0;
    ecd_pp->GsTex1 = tim2_dat_pp->GsTex1;
    ecd_pp->GsRegs = tim2_dat_pp->GsRegs;

    ecd_pp->w = tim2_dat_pp->w;
    ecd_pp->h = tim2_dat_pp->h;
    
    ecd_pp->u = 0;
    ecd_pp->v = 0;
    ecd_pp->cx = 0;
    ecd_pp->cy = 0;
    
    ecd_pp->scalex = 1.0f;
    ecd_pp->scaley = 1.0f;
    ecd_pp->alpha = 0.0f;

    ecd_pp->kido[0] = 128;
    ecd_pp->kido[1] = 128;
    ecd_pp->kido[2] = 128;
}

void examCharScaleSet(EX_CHAR_DISP *ecd_pp, float scx, float scy) {
    ecd_pp->scalex = ecd_pp->scalex * scx;
    ecd_pp->scaley = ecd_pp->scaley * scy;
}

void examCharCltSet(EX_CHAR_DISP *ecd_pp, TIM2_DAT *tim2_dat_pp) {
    ((sceGsTex0*)&ecd_pp->GsTex0)->CBP = ((sceGsTex0*)&tim2_dat_pp->GsTex0)->CBP;
}

void examCharPosSet(EX_CHAR_DISP *ecd_pp, int xp, int yp) {
    ecd_pp->x = (xp << 4) - GS_X_COORD(SCREEN_WIDTH);
    ecd_pp->y = (yp << 4) - GS_Y_COORD(SCREEN_FIELD_HEIGHT);
}

void examCharUVWHSet(EX_CHAR_DISP *ecd_pp, u_short u, u_short v, u_short w, u_short h) {
    ecd_pp->u = u;
    ecd_pp->v = v;
    ecd_pp->w = w;
    ecd_pp->h = h;
}

void examCharAlphaSet(EX_CHAR_DISP *ecd_pp, u_short on_off) {
    ecd_pp->alpha = on_off;
}

void examCharKidoSet(EX_CHAR_DISP *ecd_pp, u_char rc, u_char gc, u_char bc) {
    ecd_pp->kido[0] = rc;
    ecd_pp->kido[1] = gc;
    ecd_pp->kido[2] = bc;
}

static void NikoReset(void) {
    int i;

    if (niko_chan_str_pp != NULL) {
        for (i = 0; i < niko_chan_str_cnt; i++) {
            niko_chan_str_pp[i].niko_enum = NIKO_KAGE;
        }
    }
}

void MbarNikoHookUse(void) {
    niko_chan_str_cnt = 10;
    niko_chan_str_pp = niko_chan_str_hook;
    NikoReset();
}

void MbarNikoVsUse(void) {
    niko_chan_str_cnt = 6;
    niko_chan_str_pp = niko_chan_str_vs;
    NikoReset();
}

void MbarNikoUnUse(void) {
    niko_chan_str_pp = NULL;
    niko_chan_str_cnt = 0;
}

void MbarNikoSet(int num, int ofs) {
    int i;

    if (niko_chan_str_pp == NULL) {
        return;
    }

    for (i = 0; i < num / 2; i++) {
        if ((i + ofs) >= niko_chan_str_cnt) {
            printf("NIKO OVER!!\n");
            return;
        }
        niko_chan_str_pp[i + ofs].niko_enum = NIKO_MARU;
    }

    if ((num % 2) != 0) {
        if (((num / 2) + ofs) >= niko_chan_str_cnt) {
            printf("NIKO OVER!!\n");
            return;
        }  
        niko_chan_str_pp[(num / 2) + ofs].niko_enum = NIKO_HALF;
    }
}

static void MbarNikoDisp(sceGifPacket *gifpk_pp) {
    int i;
    NIKO_CHAN_STR *niko_pp;
    TIM2_DAT *tim2_dat_pp;
    TIM2_DAT *tim2_dat2_pp;

    if (niko_chan_str_pp == NULL || niko_chan_str_cnt == 0) {
        return;
    }

    ChangeDrawAreaSetGifTag(DrawGetDrawEnvP(DNUM_DRAW), gifpk_pp);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEST_1, GS_TEST_ALPHA_NONZERO);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 128));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_CLAMP_1, GS_CLAMP_EDGES);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_PABE, 0);
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 128));
    sceGifPkAddGsAD(gifpk_pp, SCE_GS_ALPHA_1, GS_ALPHA_BLEND);

    niko_pp = niko_chan_str_pp;
    for (i = 0; i < niko_chan_str_cnt; i++, niko_pp++) {
        tim2_dat_pp = NULL;
        tim2_dat2_pp = NULL;

        switch (niko_chan_str_pp[i].niko_enum) {
        case NIKO_KAGE:
            tim2_dat_pp = &tim2spr_tbl[60];
            break;
        case NIKO_HALF:
            tim2_dat_pp = &tim2spr_tbl[60];
            tim2_dat2_pp = tim2_dat_pp + 1;
            break;
        case NIKO_MARU:
            tim2_dat_pp = &tim2spr_tbl[61];
            break;
        case NIKO_SKIP:
            tim2_dat_pp = &tim2spr_tbl[62];
            break;
        }

        if (tim2_dat_pp != NULL) {
            sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEX0_1, tim2_dat_pp->GsTex0);
            sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEX1_1, tim2_dat_pp->GsTex1);
            sceGifPkAddGsAD(gifpk_pp, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(TRUE));
            sceGifPkAddGsAD(gifpk_pp, SCE_GS_UV, SCE_GS_SET_UV(0, 0));
            sceGifPkAddGsAD(gifpk_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((niko_pp->xp + 0x6c0) << 4, (niko_pp->yp + 0x790) << 4, 1));
            sceGifPkAddGsAD(gifpk_pp, SCE_GS_UV, SCE_GS_SET_UV(tim2_dat_pp->w << 4, tim2_dat_pp->h << 4));
            sceGifPkAddGsAD(gifpk_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((niko_pp->xp + tim2_dat_pp->w + 0x6c0) << 4, (niko_pp->yp + (tim2_dat_pp->h / 2) + 0x790) << 4, 1));

            if (tim2_dat2_pp != NULL) {
                sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEX0_1, tim2_dat2_pp->GsTex0);
                sceGifPkAddGsAD(gifpk_pp, SCE_GS_TEX1_1, tim2_dat2_pp->GsTex1);
                sceGifPkAddGsAD(gifpk_pp, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(TRUE));
                sceGifPkAddGsAD(gifpk_pp, SCE_GS_UV, SCE_GS_SET_UV(0, 0));
                sceGifPkAddGsAD(gifpk_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((niko_pp->xp + 0x6c0) << 4, (niko_pp->yp + 0x790) << 4, 1));
                sceGifPkAddGsAD(gifpk_pp, SCE_GS_UV, SCE_GS_SET_UV(tim2_dat2_pp->w << 4, (tim2_dat2_pp->h / 2) << 4));
                sceGifPkAddGsAD(gifpk_pp, SCE_GS_XYZ2, SCE_GS_SET_XYZ2((niko_pp->xp + tim2_dat2_pp->w + 0x6c0) << 4, (niko_pp->yp + (tim2_dat2_pp->h / 4) + 0x790) << 4, 1));
            }
        }
    }
}

void MbarHookUseInit(void) {
    int i;

    hook_use_flag = TRUE;

    for (i = 0; i < PR_ARRAYSIZE(mbhook_str); i++) {
        mbhook_str[i].timer = 0;
        Tim2Trans(cmnfGetFileAdrs(mbhook_str[i].moto));
    }
}

void MbarHookUnUse(void) {
    hook_use_flag = FALSE;
}

void MbarHookUseOK(void) {
    mbhook_str[1].timer = 1;
}

void MbarHookUseNG(void) {
    mbhook_str[0].timer = 1;
}

static void MbarHookPoll(void) {
    int i;
    u_char *saki_pp;
    u_char *moto_pp;

    if (!hook_use_flag) {
        return;
    }

    for (i = 0; i < 2; i++) {
        if (mbhook_str[i].timer != 0) {
            mbhook_str[i].timer++;

            moto_pp = cmnfGetFileAdrs(mbhook_str[i].moto);
            saki_pp = cmnfGetFileAdrs(mbhook_str[i].saki);

            if (mbhook_str[i].timer < 0 || mbhook_str[i].timer > 15) {
                Tim2Trans(moto_pp);
                mbhook_str[i].timer = 0;
            } else {
                Cl2MixTrans(hook_fr_dat[mbhook_str[i].timer], 128, saki_pp, moto_pp);
            }
        }
    }
}

void vsAnimationInit(void) {
    int i;

    for (i = 0; i < 4; i++) {
        vsScoreMove[i] = 0;
        vsScoreAni[i] = 0;
    }

    WorkClear(vs_scr_ctrl, sizeof(vs_scr_ctrl));
}

void vsAnimationReq(int ply, long scrMoto, long scrSaki, VS_MV_TYPE vt) {
    VS_SCR_CTRL *vsc_pp = &vs_scr_ctrl[ply];

    vsc_pp->motoScr = scrMoto;
    vsc_pp->sakiScr = scrSaki;
    vsc_pp->animation_time = 60;
    vsc_pp->vt = vt;
}

void vsAnimationReset(int ply, long scr) {
    VS_SCR_CTRL *vsc_pp = &vs_scr_ctrl[ply];

    vsc_pp->motoScr = scr;
    vsc_pp->sakiScr = scr;
    vsc_pp->animation_time = 0;
    vsc_pp->vt = VSMT_NONE;
}

static int vsScr2Move(long scr) {
    if (scr > 500) {
        scr = 500;
    }

    return scr * 2;
}

static void vsAnimationPoll(void) {
    int i, score;

    for (i = 0; i < 4; i++) {
        if (vs_scr_ctrl[i].animation_time != 0) {
            vsScoreAni[i] = 60 - vs_scr_ctrl[i].animation_time;

            score = ((vs_scr_ctrl[i].sakiScr - vs_scr_ctrl[i].motoScr) * vsScoreAni[i]) / 60;
            score += vs_scr_ctrl[i].motoScr;

            vsScoreMove[i] = score;
            vsScoreMove[i] = vsScr2Move(vsScoreMove[i]);

            if (vs_scr_ctrl[i].vt == VSMT_UP) {
                vsScoreAni[i] += 62;
            }
            if (vs_scr_ctrl[i].vt == VSMT_DW) {
                vsScoreAni[i] += 2;
            }

            vs_scr_ctrl[i].animation_time--;
            if (vs_scr_ctrl[i].animation_time == 0) {
                vs_scr_ctrl[i].motoScr = vs_scr_ctrl[i].sakiScr;
            }
        } else {
            vsScoreAni[i] = 0;
            vsScoreMove[i] = vsScr2Move(vs_scr_ctrl[i].sakiScr);
        }
    }
}

static void metColorInit(void) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(metcol_str); i++) {
        Tim2Trans(cmnfGetFileAdrs(metcol_str[i].df_num));
    }
}

static void metColorSet(EXAM_TYPE exam_type, float per) {
    u_char *moto_pp, *saki_pp;
    int     sakiper;

    if (per == 0.0f) {
        return Tim2Trans(cmnfGetFileAdrs(metcol_str[exam_type].df_num));
    }
    if (per == -1.0f) {
        return Tim2Trans(cmnfGetFileAdrs(metcol_str[exam_type].ng_num));
    }
    if (per == 1.0f) {
        return Tim2Trans(cmnfGetFileAdrs(metcol_str[exam_type].ok_num));
    }

    sakiper = metcol_str[exam_type].df_num;
    moto_pp = cmnfGetFileAdrs(sakiper);

    if (per < 0.0f) {
        per = -per;
        if (per > 1.0f) {
            per = 1.0f;
        }
        saki_pp = cmnfGetFileAdrs(metcol_str[exam_type].ng_num);
        sakiper = per * 128;
    } else {
        if (per > 1.0f) {
            per = 1.0f;
        }
        saki_pp = cmnfGetFileAdrs(metcol_str[exam_type].ok_num);
        sakiper = per * 128;
    }

    Cl2MixTrans(sakiper, 128, saki_pp, moto_pp);
}

void metFrameInit(void) {
    int i;

    for (i = 0; i < 3; i++) {
        metFrameCnt[i] = 140;
        metFrameCntLight[i] = 0;
    }

    scoreTentouFlag = 0;

    for (i = 0; i < 4; i++) {
        scr_tenmetu_col_dat[i][0] = scr_tenmetu_col[0][0];
        scr_tenmetu_col_dat[i][1] = scr_tenmetu_col[0][1];
        scr_tenmetu_col_dat[i][2] = scr_tenmetu_col[0][2];
        exam_global_ply_current_ply[i] = 0;
    }
}

void conditionFrameInit(void) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(conditionFramCnt); i++) {
        conditionFramCnt[i] = 60;
    }
}

void ExamDispInit(void) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(exam_global_ply); i++) {
        exam_global_ply[i] = NULL;
    }

    exam_disp_cursor_timer = -1;

    metFrameInit();

    metColorInit();

    conditionFrameInit();

    otehonAniCnt = 0;
    othon_frame = 0;

    ExamDispReset();

    vsAnimationInit();
}

void ExamDispPlySet(GLOBAL_PLY *ply, int pos) {
    exam_global_ply[pos] = ply;
}

void ExamDispReq(int ply, int plmi) {
    exam_disp_cursor_timer = 0;
    exam_global_ply_current = exam_global_ply[ply];

    if (plmi) {
        exam_global_ply_current_ply[ply] = 1;
    } else {
        exam_global_ply_current_ply[ply] = 0;
    }
}

void ExamDispReset(void) {
    exam_disp_cursor_timer = -1;
    metFrameInit();
    metColorInit();
}

float examScore2Level(long score) {
    float ret_lvl = score / 140.0f;

    if (ret_lvl > 0) {
        ret_lvl += 0.2;
    } else {
        ret_lvl -= 0.2;
    }

    if (ret_lvl > 1.0) {
        ret_lvl = 1.0f;
    }

    if (ret_lvl < -1.0) {
        ret_lvl = -1.0f;
    }

    return ret_lvl;
}

static void ExamDispOn(void) {
    int met_time;
    int i;
    float maxfr;
    float lev_tmp;
    u_int perd;
    u_char *moto_p;

    if (exam_disp_cursor_timer < 0) {
        return metFrameInit();
    }
    exam_disp_cursor_timer += 1;
    if (exam_disp_cursor_timer >= 91) {
        exam_disp_cursor_timer = -1;
        metColorInit();
        for (i = 0; i < 3; i++) {
            exam_global_ply_current_ply[i] = 0;
        }
        met_time = 0;
    } else if (exam_disp_cursor_timer >= 61) {
        met_time = 90 - exam_disp_cursor_timer;
    } else {
        met_time = 30;
        if (met_time >= exam_disp_cursor_timer) {
            met_time = exam_disp_cursor_timer;
        }
    }
    maxfr = met_time / 30.0f;
    for (i = 0; i < 3; i++) {
        lev_tmp = examScore2Level(exam_global_ply_current->exam_score[i]);
        if (lev_tmp < 0.0f && lev_tmp < -maxfr) {
            lev_tmp = -maxfr;
        }
        if (0.0f < lev_tmp && maxfr < lev_tmp) {
            lev_tmp = maxfr;
        }
        if (exam_global_ply_current->exam_score[i] > 0) {
            metColorSet(i, maxfr);
        } else {
            metColorSet(i, -maxfr);
        }
        lev_tmp -= -1.0;
        lev_tmp = (lev_tmp / 2.0) * 280.0;
        metFrameCnt[i] = lev_tmp;
        if (exam_disp_cursor_timer < 30) {
            if (exam_global_ply_current->exam_score[i] > 0) {
                metFrameCntLight[i] = exam_disp_cursor_timer;
            } else {
                metFrameCntLight[i] = exam_disp_cursor_timer + 60;
            }
        } else {
            metFrameCntLight[i] = 0;
        }
    }
    for (i = 0; i < 3; i++) {
        if (exam_global_ply_current_ply[i] != 0) {
            if (exam_global_ply[i]->now_score > 0) {
                moto_p = scr_tenmetu_col[1];
            } else {
                moto_p = scr_tenmetu_col[2];
            }
            perd = exam_disp_cursor_timer;
            perd *= 16;
            /* fold the 9-bit ramp into a 0..255..0 triangle wave for the blink */
            if (perd & 0x100) {
                perd ^= 0xff;
            }
            perd &= 0xff;
            scr_tenmetu_col_dat[i][0] = ((moto_p[0] * perd) >> 9) + (moto_p[0] >> 1);
            scr_tenmetu_col_dat[i][1] = ((moto_p[1] * perd) >> 9) + (moto_p[1] >> 1);
            scr_tenmetu_col_dat[i][2] = ((moto_p[2] * perd) >> 9) + (moto_p[2] >> 1);
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

static u_long hex2decPlMi(long data) {
    u_long ret = 0;
    u_int  i;
    long   plmichar;

    if (data == 0) {
        return 0;
    }

    plmichar = 0;

    if (data > 0) {
        plmichar = 10;
    }

    if (data < 0) {
        plmichar = 11; 
        data = data * -1;
    }

    for (i = 0; i < 16; i++) {
        if (data == 0) {
            ret |= (plmichar << (i * 4));
            break;
        }

        ret |= (data % 10) << (i * 4);
        data /= 10;
    }

    return ret;
}

void examNumDisp(sceGifPacket *ex_gif_pp, long score, short x, short y, int keta, u_char *coldat_pp, int plmi) {
    int          i;
    u_char       num;
    int          first_f;
    EX_CHAR_DISP ex_ecd;

    first_f = FALSE;
    examCharBasic(&ex_ecd, &tim2spr_tbl[27]);
    examCharKidoSet(&ex_ecd, coldat_pp[0], coldat_pp[1], coldat_pp[2]);
    
    if (plmi) {
        score = hex2decPlMi(score);
    } else {
        score = hex2dec(score);
    }

    for (i = 0; i < keta; i++) {
        plmi = i + 1;
        num = (score >> ((keta - plmi) << 2)) & 0xf;

        if (num != 0 || first_f || i == (keta - 1)) {
            first_f = TRUE;
            examCharUVWHSet(&ex_ecd, num * 13, 0, 13, 24);
            examCharPosSet(&ex_ecd, x + (i * 15), y);
            examCharSet(&ex_ecd, ex_gif_pp);
        }
    }
}

static void examScoreSet(sceGifPacket *ex_gif_pp) {
    int i;
    int pos_dat[2][2] = {
        { 530, 4  },
        { 530, 36 },
    };

    for (i = 0; i < 2; i++) {
        if (exam_global_ply[i] != NULL) {
            if (exam_global_ply_current_ply[i]) {
                examNumDisp(ex_gif_pp, exam_global_ply[i]->now_score, pos_dat[i][0], pos_dat[i][1], 5, scr_tenmetu_col_dat[i], 1);
            } else {
                examNumDisp(ex_gif_pp, exam_global_ply[i]->score, pos_dat[i][0], pos_dat[i][1], 5, scr_tenmetu_col_dat[i], 0);
            }
        }
    }
}

static void examLevelDisp(sceGifPacket *ex_gif_pp) {
    GLOBAL_PLY   *exg_p;
    int           old_fr, targ_fr;
    EX_CHAR_DISP  ex_ecd;
    int           plevel, i;

    for (i = 0; i < 4; i++) {
        exg_p = exam_global_ply[i];
        if (exg_p != NULL) {
            old_fr  = conditionFramCnt[i];
            targ_fr = exg_p->rank_level * 20;

            if (old_fr < targ_fr) {
                old_fr += 2;
                if (targ_fr < old_fr) {
                    old_fr = targ_fr;
                }
            } else {
                old_fr -= 2;
                if (old_fr < targ_fr) {
                    old_fr = targ_fr;
                }
            }

            if (old_fr > 240) {
                old_fr = 240;
            }

            conditionFramCnt[i] = old_fr;
        }
    }

    plevel = conditionFramCnt[0] * 96 / 240;

    examCharBasic(&ex_ecd, &tim2spr_tbl[28]);
    examCharUVWHSet(&ex_ecd, plevel, 0, 24, 88);
    examCharPosSet(&ex_ecd, 616, 136);
    examCharSet(&ex_ecd, ex_gif_pp);
}

void ExamDispSet(void) {
    sceGifPacket ex_gif;

    ExamDispOn();
    CmnGifOpenCmnPk(&ex_gif);
    examScoreSet(&ex_gif);
    examLevelDisp(&ex_gif);
    vsAnimationPoll();
    CmnGifCloseCmnPk(&ex_gif, 6);
}

void ExamDispSubt(void) {
    /* Empty */
}

void MbarInit(int stg) {
    mbar_ctrl_stage = stg;

    if (stg == 6) {
        mbar_ctrl_stage_selT = 1;
    } else {
        mbar_ctrl_stage_selT = stg;
    }
    
    MbarCharSetSub();
}

void MbarReset(void) {
    WorkClear(mbar_req_str, sizeof(mbar_req_str));
}

void MbarReq(MBAR_REQ_ENUM mm_req, TAPSET *ts_pp, int curr_time, SCR_TAP_MEMORY *tm_pp, int tm_cnt, 
             int lang, int tapdat_size, TAPDAT *tapdat_pp, GUI_CURSOR_ENUM guic) {
    PLAYER_INDEX pidx;

    if (ts_pp == NULL) {
        printf("MbarReq   TAPSET adrs is NULL\n");
        return;
    }

    pidx = Pcode2Pindex(ts_pp->player_code);
    mbar_req_str[pidx].mbar_req_enum = mm_req;
    mbar_req_str[pidx].tapset_pp = ts_pp;
    mbar_req_str[pidx].current_time = curr_time;
    mbar_req_str[pidx].scr_tap_memory_pp = tm_pp;
    mbar_req_str[pidx].scr_tap_memory_cnt = tm_cnt;
    mbar_req_str[pidx].lang = lang;

    mbar_req_str[pidx].tapdat_size = tapdat_size;
    mbar_req_str[pidx].tapdat_pp = tapdat_pp;
    mbar_req_str[pidx].gui_cursor_enum = guic;
}

void MbarSetCtrlTime(int mctime) {
    mbar_ctrl_time = mctime;
}

static void MbarCl1CharSet(int col_num, int moto_num) {
    MBA_CHAR_DATA *mbcd_col = &mba_char_data[col_num];
    MBA_CHAR_DATA *mbcd_mot = &mba_char_data[moto_num];
    sceGsTex0      ColGsTex0;
    sceGsTex0      MotGsTex0;

    ColGsTex0 = *(sceGsTex0*)&mbcd_col->tim2_dat_pp->GsTex0;
    MotGsTex0 = *(sceGsTex0*)&mbcd_mot->tim2_dat_pp->GsTex0;

    *mbcd_col->tim2_dat_pp = *mbcd_mot->tim2_dat_pp;

    MotGsTex0.CBP = ColGsTex0.CBP;
    mbcd_col->tim2_dat_pp->GsTex0 = GS_REG_WORD(MotGsTex0);
}

static void MbarCharSetSub(void) {
    int i;

    for (i = 0; i < 7; i++) {
        MbarCl1CharSet(i + 15, i + 1);
    }
}

void MbarGifInit(void) {
    CmnGifADPacketMake(&mbar_gif, 0);
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_TEST_1, GS_TEST_ALPHA_NONZERO);
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 128));
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_CLAMP_1, GS_CLAMP_EDGES);
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_PABE, 0);    
}

void MbarGifTrans(int pri) {
    CmnGifCloseCmnPk(&mbar_gif, pri);
}

void MbarCharSet(MBARR_CHR *mb_pp) {
    MBA_CHAR_DATA *mbcd_pp;
    float          w, h;
    int            x1, y1, x2, y2;

    mbcd_pp = &mba_char_data[mb_pp->mbc_enum];
    if (mbcd_pp->tim2_dat_pp == NULL) {
        return;
    }

    sceGifPkAddGsAD(&mbar_gif, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(mb_pp->r, mb_pp->g, mb_pp->b, mb_pp->a, GS_Q_ONE));
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_TEX0_1, mbcd_pp->tim2_dat_pp->GsTex0);
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_TEX1_1, mbcd_pp->tim2_dat_pp->GsTex1);
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(FALSE));

    w = mbcd_pp->sclx * mb_pp->sclx * mbcd_pp->tim2_dat_pp->w;
    h = mbcd_pp->scly * mb_pp->scly * mbcd_pp->tim2_dat_pp->h;
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_UV, SCE_GS_SET_UV(0, 0));

    w *= 0.5f;
    h *= 0.5f;
    x1 = (mb_pp->xp - w) * 16.0f;
    y1 = (mb_pp->yp - h) * 16.0f;
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(0) + x1, GS_Y_COORD(0) + y1, 1));

    sceGifPkAddGsAD(&mbar_gif, SCE_GS_UV, SCE_GS_SET_UV(mbcd_pp->tim2_dat_pp->w << 4, mbcd_pp->tim2_dat_pp->h << 4));

    x2 = (mb_pp->xp + w) * 16.0f;
    y2 = (mb_pp->yp + h) * 16.0f;
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(0) + x2, GS_Y_COORD(0) + y2, 1));
}

void MbarCharSet2(MBARR_CHR2 *mb_pp) {
    MBA_CHAR_DATA *mbcd_pp;
    int            x1, y1, x2, y2;
    
    mbcd_pp = &mba_char_data[mb_pp->mbc_enum];
    if (mbcd_pp->tim2_dat_pp == NULL) {
        return;
    }

    sceGifPkAddGsAD(&mbar_gif, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(mb_pp->r, mb_pp->g, mb_pp->b, mb_pp->a, GS_Q_ONE));
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_TEX0_1, mbcd_pp->tim2_dat_pp->GsTex0);
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_TEX1_1, mbcd_pp->tim2_dat_pp->GsTex1);
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(FALSE));

    sceGifPkAddGsAD(&mbar_gif, SCE_GS_UV, SCE_GS_SET_UV(0, 0));

    x1 = mb_pp->xp + mb_pp->ofsx;
    y1 = mb_pp->yp + mb_pp->ofsy;
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(x1), GS_Y_COORD(y1), 1));

    sceGifPkAddGsAD(&mbar_gif, SCE_GS_UV, SCE_GS_SET_UV(mbcd_pp->tim2_dat_pp->w << 4, mbcd_pp->tim2_dat_pp->h << 4));

    x2 = mb_pp->xp2 + mb_pp->ofsx2;
    y2 = mb_pp->yp2 + mb_pp->ofsy2;
    sceGifPkAddGsAD(&mbar_gif, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(x2), GS_Y_COORD(y2), 1));
}

/* The up/down windows split the field in half and stop short of the right edge. */
#define MBWINDOW_SPLIT_X1 525
#define MBWINDOW_SPLIT_Y  (SCREEN_FIELD_HEIGHT / 2)

void MbarWindowSet(MBWINDOW_ENUM wenum) {
    switch (wenum) {
    case MBWINDOW_NORMAL:
        sceGifPkAddGsAD(&mbar_gif, SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, SCREEN_WIDTH - 1, 0, SCREEN_FIELD_HEIGHT - 1));
        break;
    case MBWINDOW_UP:
        sceGifPkAddGsAD(&mbar_gif, SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, MBWINDOW_SPLIT_X1, 0, MBWINDOW_SPLIT_Y - 1));
        break;
    case MBWINDOW_DOWN:
        sceGifPkAddGsAD(&mbar_gif, SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, MBWINDOW_SPLIT_X1, MBWINDOW_SPLIT_Y, SCREEN_FIELD_HEIGHT - 1));
        break;
    }
}

static int MbarGetDispPosX(int tick) {
    int pos;

    if (tick < 0) {
        return 13;
    }

    if (tick < 480) {
        pos = (tick * 25 / 24);
    } else {
        pos = ((tick - 384) * 25 / 24);
    }

    return pos + 13;
}

static int MbarGetDispPosY(int tick) {
    int v0, pos;

    pos = 0x1df - 1;

    if (tick < 0) {
        return mbar_pos_y_ofs + 0x17;
    }
    
    pos = (pos + 1) < tick;
    pos *= 0x19;
    v0 = mbar_pos_y_ofs + 0x17;
    return pos + v0;
}

static int MbarGetTimeArea(MBAR_REQ_STR *mr_pp) {
    int ret = 0;

    if (mr_pp->mbar_req_enum == MBAR_NONE) {
        return ret;
    }

    if (mbar_ctrl_time < (mr_pp->current_time + mr_pp->tapset_pp->taptimeStart)) {
        return ret;
    }
    if (mbar_ctrl_time >= (mr_pp->current_time + mr_pp->tapset_pp->taptimeEnd)) {
        return ret;
    }

    if (mr_pp->mbar_req_enum & MBAR_MASK_ELAPSED) {
        ret = mbar_ctrl_time - mr_pp->current_time - mr_pp->tapset_pp->taptimeStart;
    }
    if (mr_pp->mbar_req_enum & MBAR_MASK_FUTURE) {
        ret = mr_pp->tapset_pp->taptimeEnd - mr_pp->tapset_pp->taptimeStart;
    }

    return ret;
}

static int MbarGetTimeArea2(MBAR_REQ_STR *mr_pp) {
    int ret = 0;

    if (mr_pp->mbar_req_enum == MBAR_NONE) {
        return ret;
    }

    if (mbar_ctrl_time >= (mr_pp->current_time + mr_pp->tapset_pp->taptimeEnd)) {
        return ret;
    }

    if (mr_pp->mbar_req_enum & MBAR_MASK_ELAPSED) {
        ret = mbar_ctrl_time - mr_pp->current_time - mr_pp->tapset_pp->taptimeStart;
    }
    if (mr_pp->mbar_req_enum & MBAR_MASK_FUTURE) {
        ret = mr_pp->tapset_pp->taptimeEnd - mr_pp->tapset_pp->taptimeStart;
    }

    return ret;
}

int MbarGetStartTime(MBAR_REQ_STR *mr_pp) {
    return ((mr_pp->current_time + mr_pp->tapset_pp->taptimeStart - 24) / 96) * 96;
}

int MbarGetEndTime(MBAR_REQ_STR *mr_pp) {
    return (mr_pp->current_time + mr_pp->tapset_pp->taptimeEnd);
}

static int MbarGetStartTap(MBAR_REQ_STR *mr_pp) {
    int ret;

    ret = mr_pp->current_time + mr_pp->tapset_pp->taptimeStart;
    ret = (ret / 24 - 1) * 24;
    return -1 < ret ? ret : 0;
}

void MbarSclRotMake(MBARR_CHR *mbarr_pp, int mbtime) {
    float tmp_rate;

    mbarr_pp->sclx = 1.0f;
    mbarr_pp->scly = 1.0f;

    if (mbtime >= (u_int)96) {
        return;
    }

    if (mbtime < 24) {
        tmp_rate = (24 - mbtime) / 24.0f + 1.0f;
        mbarr_pp->sclx = tmp_rate;
        mbarr_pp->scly = tmp_rate;
    }

    if (mbtime < 96) {
        tmp_rate = cosf(mbtime * 6.2831855f / 96.0f);
        mbarr_pp->sclx *= tmp_rate;
    }
}

void MbarGuideLightMake(MBARR_CHR *mbarr_pp, int mbtime) {
    u_char col = 128;

    if (mbtime >= 0 && mbtime < 144) {
        col = (144 - mbtime) * 128 / 144 + 128u;
    }

    mbarr_pp->r = mbarr_pp->g = mbarr_pp->b = col;
}

int MbarFlashMake(MBARR_CHR *mbarr_pp, MBARR_CHR *mbarr_moto_pp, int mbtime, int fltype) {
    float   fsize;
    u_char *colpp;
    u_char  scale;

    if (mbtime < 0) {
        return 0;
    }
    if (mbtime >= 24) {
        return 0;
    }

    mbarr_pp->xp = mbarr_moto_pp->xp;
    mbarr_pp->yp = mbarr_moto_pp->yp + 100;
    
    fsize = 24 - mbtime;
    fsize += fsize;
    fsize /= 24.0f;
    fsize += 1.0f;
    mbarr_pp->scly = fsize;
    mbarr_pp->sclx = fsize;
    
    colpp = colp[fltype];
    scale = (mbtime * 128) / 24;
    mbarr_pp->r = (colpp[0] * scale) / 128;
    mbarr_pp->g = (colpp[1] * scale) / 128;
    mbarr_pp->b = (colpp[2] * scale) / 128;

    return 1;    
}

void MbarBackSet(MBAR_REQ_STR *mr_pp) {
    int        i, stt, endt, sttap, curtime;
    MBARR_CHR  mbarr;
    MBARR_CHR2 mbarr_chr2;
    int        sttime, endtime;

    mbarr = (MBARR_CHR) {
        .mbc_enum = MBC_BALL,
        .xp = 13, .yp = 23,
        .sclx = 1.0f, .scly = 1.0f,
        .r = 128, .g = 128, .b = 128, .a = 128
    };

    MbarWindowSet(MBWINDOW_UP);

    if (MbarGetTimeArea2(mr_pp) == 0) {
        return;
    }

    curtime = mbar_ctrl_time;
    stt = MbarGetStartTime(mr_pp);
    endt = MbarGetEndTime(mr_pp);
    sttap = MbarGetStartTap(mr_pp);

    if (mr_pp->mbar_req_enum & MBAR_BIT_PARAPPA) {
        mbarr_chr2.mbc_enum = MBC_GLINE_P;
    } else {
        mbarr_chr2.mbc_enum = MBC_GLINE_T;
    }

    endtime = endt - stt - 1;
    if (curtime < stt) {
        sttime = 0;
    } else {
        sttime = curtime - stt;
    }

    mbarr_chr2.b = 128;
    mbarr_chr2.g = 128;
    mbarr_chr2.r = 128;
    mbarr_chr2.a = 32;
    mbarr_chr2.ofsx2 = 0;
    mbarr_chr2.ofsx = 0;
    mbarr_chr2.ofsy = -14;
    mbarr_chr2.ofsy2 = 14;
    
    if (sttime < endtime) {
        mbarr_chr2.xp = MbarGetDispPosX(sttime);
        mbarr_chr2.yp = MbarGetDispPosY(sttime);
        mbarr_chr2.xp2 = MbarGetDispPosX(endtime);
        mbarr_chr2.yp2 = MbarGetDispPosY(endtime);

        if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_LOW) {
            mbarr_chr2.yp2 += 50;
            mbarr_chr2.yp += 50;
        } else if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_MID) {
            mbarr_chr2.yp2 += 25;
            mbarr_chr2.yp += 25;
        }

        if (mbarr_chr2.yp != mbarr_chr2.yp2) {
            mbarr_chr2.xp2 = MbarGetDispPosX(479);
            mbarr_chr2.yp2 = MbarGetDispPosY(479);

            if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_LOW) {
                mbarr_chr2.yp2 += 50;
            } else if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_MID) {
                mbarr_chr2.yp2 += 25;
            }

            MbarCharSet2(&mbarr_chr2);

            mbarr_chr2.xp = MbarGetDispPosX(480);
            mbarr_chr2.yp = MbarGetDispPosY(480);
            mbarr_chr2.xp2 = MbarGetDispPosX(endtime);
            mbarr_chr2.yp2 = MbarGetDispPosY(endtime);

            if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_LOW) {
                mbarr_chr2.yp2 += 50;
                mbarr_chr2.yp += 50;
            } else if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_MID) {
                mbarr_chr2.yp2 += 25;
                mbarr_chr2.yp += 25;
            }
        }

        MbarCharSet2(&mbarr_chr2);
    }

    for (endtime = stt + 24; endtime < endt; endtime += 24) {
        if ((endtime - 24) >= sttap) {
            sttime = endtime - stt;
            mbarr.xp = MbarGetDispPosX(sttime);
            mbarr.yp = MbarGetDispPosY(sttime);

            mbarr.mbc_enum = MBC_BALL;
            if (((endtime / 24) % 4) == 0) {
                mbarr.mbc_enum = MBC_STAR;
            }

            if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_LOW) {
                mbarr.yp += 50;
            } else if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_MID) {
                mbarr.yp += 25;
            }

            if (mr_pp->mbar_req_enum & MBAR_BIT_GUIDE_LIGHT) {
                MbarGuideLightMake(&mbarr, curtime - endtime);
            }

            MbarCharSet(&mbarr);
        }
    }
}

/* static */ void MbarOthSet(MBAR_REQ_STR *mr_pp) {
    int       i;
    MBARR_CHR mbarr = {
        .mbc_enum = MBC_NONE,
        .xp = 0, .yp = 0,
        .sclx = 1.0f, .scly = 1.0f,
        .r = 128, .g = 128, .b = 128, .a = 128
    };
    MBARR_CHR mbarrFlash = {
        .mbc_enum = MBC_FLASH,
        .xp = 0, .yp = 0,
        .sclx = 1.0f, .scly = 1.0f,
        .r = 128, .g = 128, .b = 128, .a = 128
    };
    int       curtime, gbalTapTime, stt;
    int       fl_type;

    MbarWindowSet(MBWINDOW_UP);

    if (MbarGetTimeArea2(mr_pp) == 0) {
        return;
    }

    stt = MbarGetStartTime(mr_pp);
    gbalTapTime = mr_pp->current_time + mr_pp->tapset_pp->taptimeStart;
    curtime = mbar_ctrl_time - mr_pp->current_time - mr_pp->tapset_pp->taptimeStart;

    if (mr_pp->mbar_req_enum & MBAR_MASK_SCRIPT) {
        TAPDAT *tapdat_pp = mr_pp->tapdat_pp;
        for (i = 0; i < mr_pp->tapdat_size; i++, tapdat_pp++) {
            if (tapdat_pp->KeyIndex == KiNO) {
                continue;
            }

            if (global_data.play_typeL == PLAY_TYPE_ONE) {
                mbarr.mbc_enum = MBC_SP;
                if (mr_pp->mbar_req_enum & MBAR_MASK_ALT) {
                    mbarr.mbc_enum = MBC_M_SP;
                }
            } else {
                mbarr.mbc_enum = tapdat_pp->KeyIndex;
                if (mr_pp->mbar_req_enum & MBAR_MASK_ALT) {
                    mbarr.mbc_enum += MBC_SP;
                }
            }

            if (tapdat_pp->time < curtime) {
                if (!(mr_pp->mbar_req_enum & MBAR_MASK_PAST)) {
                    continue;
                }
                mbarr.a = 64;
            } else {
                if (!(mr_pp->mbar_req_enum & MBAR_MASK_FUTURE)) {
                    continue;
                }
            }

            mbarr.xp = MbarGetDispPosX(gbalTapTime + tapdat_pp->time - stt);
            mbarr.yp = MbarGetDispPosY(gbalTapTime + tapdat_pp->time - stt);

            if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_LOW) {
                mbarr.yp += 50;
            } else if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_MID) {
                mbarr.yp += 25;
            }

            mbarr.sclx = 1.0f;
            mbarr.scly = 1.0f;
            MbarCharSet(&mbarr);

            mbarr.a = 128;
        }
    }

    if (mr_pp->mbar_req_enum & MBAR_MASK_MEMORY) {
        SCR_TAP_MEMORY *stm_pp = mr_pp->scr_tap_memory_pp;
        for (i = 0; i < mr_pp->scr_tap_memory_cnt; i++, stm_pp++) {
            if (curtime < stm_pp->ofs_frame) {
                break;
            }

            if (stm_pp->key == 0) {
                continue;
            }

            if (global_data.play_typeL == PLAY_TYPE_ONE) {
                if (mr_pp->mbar_req_enum & MBAR_BIT_MEMORY) {
                    mbarr.mbc_enum = MBC_SP;
                } else {
                    mbarr.mbc_enum = MBC_M_SP;
                }

                if (mr_pp->mbar_req_enum & MBAR_BIT_OTHER_DIM) {
                    if (!stm_pp->othOn) {
                        mbarr.mbc_enum = MBC_BW_SP;
                    }
                }

                fl_type = 0;
            } else {
                if (mr_pp->mbar_req_enum & MBAR_BIT_MEMORY) {
                    mbarr.mbc_enum = stm_pp->key;
                } else {
                    mbarr.mbc_enum = stm_pp->key + MBC_SP;
                }

                if (mr_pp->mbar_req_enum & MBAR_BIT_OTHER_DIM) {
                    if (!stm_pp->othOn) {
                        mbarr.mbc_enum = stm_pp->key + MBC_M_SP;
                    }
                }

                fl_type = stm_pp->key;
            }

            mbarr.xp = MbarGetDispPosX(gbalTapTime + stm_pp->ofs_frame - stt);
            mbarr.yp = MbarGetDispPosY(gbalTapTime + stm_pp->ofs_frame - stt);
            if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_LOW) {
                mbarr.yp += 50;
            } else if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_MID) {
                mbarr.yp += 25;
            }

            MbarSclRotMake(&mbarr, curtime - stm_pp->ofs_frame);

            if (mr_pp->mbar_req_enum & MBAR_BIT_GUIDE_LIGHT) {
                MbarGuideLightMake(&mbarr, curtime - stm_pp->ofs_frame);
            }

            MbarCharSet(&mbarr);

            if (MbarFlashMake(&mbarrFlash, &mbarr, curtime - stm_pp->ofs_frame, fl_type)) {
                MbarWindowSet(MBWINDOW_DOWN);
                MbarCharSet(&mbarrFlash);
                MbarWindowSet(MBWINDOW_UP);
            }
        }
    }
}

/* static */ void MbarCurSet(MBAR_REQ_STR *mr_pp) {
    MBARR_CHR mbarr;
    int       curtime, gbalTapTime, sttime;

    mbarr = (MBARR_CHR) {
        .mbc_enum = MBC_NONE,
        .xp = 0, .yp = 0,
        .sclx = 1.0f, .scly = 1.0f,
        .r = 128, .g = 128, .b = 128, .a = 128
    }; 
    
    gbalTapTime = mr_pp->current_time + mr_pp->tapset_pp->taptimeStart;
    sttime = MbarGetStartTime(mr_pp);
    
    curtime = mbar_ctrl_time;
    curtime -= mr_pp->current_time;
    curtime -= mr_pp->tapset_pp->taptimeStart;

    gbalTapTime += curtime;
    gbalTapTime -= sttime;

    mbarr.mbc_enum = mr_pp->gui_cursor_enum + MBC_NONECUR;
    mbarr.xp = MbarGetDispPosX(gbalTapTime);
    mbarr.yp = MbarGetDispPosY(gbalTapTime);

    if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_LOW) {
        mbarr.yp += 50;
    } else if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_MID) {
        mbarr.yp += 25;
    }

    MbarWindowSet(MBWINDOW_UP);
    MbarCharSet(&mbarr);

    mbarr.yp += 100;
    mbarr.a = 1; mbarr.b = 1; mbarr.g = 1; mbarr.r = 1;

    MbarWindowSet(MBWINDOW_DOWN);
    MbarCharSet(&mbarr);
}

static int MbarTapSubt(MBAR_REQ_STR *mr_pp) {
    int lng, nowp;

    if (MbarGetTimeArea(mr_pp) == 0) {
        return 0;
    }

    if (game_status.subtitle == SUBTITLE_ON) {
        lng = mr_pp->tapset_pp->taptimeEnd - mr_pp->tapset_pp->taptimeStart;
        nowp = mbar_ctrl_time - mr_pp->current_time - mr_pp->tapset_pp->taptimeStart;
        SubtTapPrintWake(mr_pp->tapset_pp->tapsubt[mr_pp->lang], mr_pp->lang, lng, nowp);
    }

    return 1;
}

/*static*/ void MbarPosOffsetSet(MBAR_REQ_STR *mr_pp) {
    mbar_pos_y_ofs = 0;
    if (MbarGetTimeArea2(mr_pp) == 0) {
        return;
    }
    if ((MbarGetEndTime(mr_pp) - MbarGetStartTime(mr_pp)) <= 480) {
        mbar_pos_y_ofs = 12;
    }
}

void mbar_othon_frame_set(MBAR_REQ_STR* mr_pp) {
    int curtime;
    int mp_st, mp_end, mp_fix;

    curtime = mbar_ctrl_time - mr_pp->current_time - mr_pp->tapset_pp->taptimeStart;
    curtime += 48;

    if (curtime < 0) {
        return;
    }

    if (curtime >= 73) {
        return;
    }

    if (mr_pp->mbar_req_enum & MBAR_BIT_ROW_LOW) {
        mp_st = 50;
        mp_end = 90;
        mp_fix = 90;
    } else {
        mp_st = 90;
        mp_end = 130;
        mp_fix = 50;
    }

    if (curtime < 24) {
        mp_fix = (((mp_end - mp_st) * curtime) / 24) + mp_st;
    }

    othon_frame = mp_fix;
}

void MbarDisp(void) {
    int i, j;

    MbarGifInit();

    for (i = 0; i < 4u; i++) {
        for (j = 0; j < PR_ARRAYSIZEU(mbar_req_str); j++) {
            if (mbar_req_str[j].mbar_req_enum == MBAR_NONE) {
                continue;
            }
            if (mbar_req_str[j].tapset_pp == NULL || mbar_req_str[j].tapset_pp->coolup == -1) {
                continue;
            }
            (*marSetPrgTbl[i])(&mbar_req_str[j]);
        }
    }

    MbarGifTrans(7);

    if (game_status.subtitle == SUBTITLE_ON) {
        for (i = 0; i < PR_ARRAYSIZEU(mbar_req_str); i++) {
            if (mbar_req_str[i].mbar_req_enum == MBAR_NONE) {
                continue;
            }
            if (mbar_req_str[i].tapset_pp == NULL) {
                continue;
            }
            if (MbarTapSubt(&mbar_req_str[i])) {
                break;
            }
        }
    }
}

int MbarDispScene(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    int   i, j;
    float saved_ratio;

    saved_ratio = PrGetMendererRatio();
    PrSetMendererRatio(0.0f);

    PR_SCOPE()
    VCLR_PARA vclr_para = {};
    DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_VRAM2);
    PR_SCOPEEND()

    ChangeDrawArea(DrawGetDrawEnvP(drDisp));
    MbarGifInit();

    for (i = 0; i < PR_ARRAYSIZEU(mbar_req_str); i++) {
        if (mbar_req_str[i].mbar_req_enum == MBAR_NONE) {
            continue;
        }
        if (mbar_req_str[i].tapset_pp == NULL || mbar_req_str[i].tapset_pp->coolup == -1) {
            continue;
        }
        for (j = 0; j < 4u; j++) {
            (marSetPrgTbl[j])(&mbar_req_str[i]);
        }
    }

    MbarWindowSet(MBWINDOW_NORMAL);
    for (i = 0; i < PR_ARRAYSIZEU(mbar_req_str); i++) {
        if (mbar_req_str[i].mbar_req_enum == MBAR_NONE) {
            continue;
        }
        if (mbar_req_str[i].tapset_pp == NULL || mbar_req_str[i].tapset_pp->coolup == -1) {
            continue;
        }
        mbar_othon_frame_set(&mbar_req_str[i]);
    }

    ExamDispOn();
    examScoreSet(&mbar_gif);
    examLevelDisp(&mbar_gif);
    vsAnimationPoll();
    MbarHookPoll();
    CmnGifADPacketMakeTrans(&mbar_gif);
    PrSetMendererRatio(saved_ratio);

    if (game_status.subtitle == SUBTITLE_ON) {
        for (i = 0; i < PR_ARRAYSIZEU(mbar_req_str); i++) {
            if (mbar_req_str[i].mbar_req_enum == MBAR_NONE) {
                continue;
            }
            if (mbar_req_str[i].tapset_pp == NULL) {
                continue;
            }
            if (MbarTapSubt(&mbar_req_str[i])) {
                break;
            }
        }
    }

    return 0;
}

int MbarDispSceneDraw(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    int   i, j;
    float saved_ratio;

    if (first_f == -2) {
        return 0;
    }
    if (first_f == -1) {
        return 0;
    }

    saved_ratio = PrGetMendererRatio();
    PrSetMendererRatio(0.0f);

    PR_SCOPE()
    VCLR_PARA vclr_para = {};
    DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_VRAM2);
    PR_SCOPEEND()

    ChangeDrawArea(DrawGetDrawEnvP(drDisp));
    MbarGifInit();

    for (i = 0; i < PR_ARRAYSIZEU(mbar_req_str); i++) {
        if (mbar_req_str[i].mbar_req_enum == MBAR_NONE) {
            continue;
        }
        if (mbar_req_str[i].tapset_pp == NULL || mbar_req_str[i].tapset_pp->coolup == -1) {
            continue;
        }
        if (global_data.play_step != PSTEP_VS || i == PINDEX_BOXY) {
            for (j = 0; j < 4u; j++) {
                (marSetPrgTbl[j])(&mbar_req_str[i]);
            }
        }
    }

    MbarWindowSet(MBWINDOW_NORMAL);
    CmnGifADPacketMakeTrans(&mbar_gif);
    PrSetMendererRatio(saved_ratio);
    return 0;
}

int MbarDispSceneVsDraw(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    if (first_f == -2) {
        return 0;
    }
    if (first_f == -1) {
        return 0;
    }

    vs_mouse_disp_flag = 1;
    return 0;
}

void MbarDispSceneVsDrawInit(void) {
    vs_mouse_disp_flag = 0;
}

/* Needs .sdata match */
static void guidisp_init_pr(void) {
    GUIMAP *guim_pp;
    int     i;

    PrSetFrameRate(60.0f);

    guime_hdl = PrInitializeScene(&DBufDc.draw01, "gui", -1);
    guime_camera_hdl = PrInitializeCamera(cmnfGetFileAdrs(72));
    PrSelectCamera(guime_camera_hdl, guime_hdl);
    PrAnimateSceneCamera(guime_hdl, 0.0f);

    for (i = 0; i < 10u; i++) {
        guim_pp = &guimap[i];
        guim_pp->spmHdl = PrInitializeModel(cmnfGetFileAdrs(guim_pp->spmmap), guime_hdl);

        if (guim_pp->spamap >= 0) {
            guim_pp->spaHdl = PrInitializeAnimation(cmnfGetFileAdrs(guim_pp->spamap));
            PrLinkAnimation(guim_pp->spmHdl, guim_pp->spaHdl);
            PrAnimateModel(guim_pp->spmHdl, *guim_pp->frame_pp);
        }

        if (guim_pp->spamapP >= 0) {
            guim_pp->spaHdlP = PrInitializeAnimation(cmnfGetFileAdrs(guim_pp->spamapP));
            PrLinkPositionAnimation(guim_pp->spmHdl, guim_pp->spaHdlP);
            PrAnimateModelPosition(guim_pp->spmHdl, *guim_pp->frame_ppP);
        }
    }

    PrPreprocessSceneModel(guime_hdl);
}

static void guidisp_draw_quit(int drapP) {
    GUIMAP *guim_pp;
    float   saved_ratio;
    int     i;

    saved_ratio = PrGetMendererRatio();
    PrSetMendererRatio(0.0f);

    PrSetSceneFrame(guime_hdl, *DrawGetFrameP(drapP));
    PrSetSceneEnv(guime_hdl, DrawGetDrawEnvP(drapP));

    PrRender(guime_hdl);
    PrWaitRender();

    for (i = 0; i < 10u; i++) {
        guim_pp = &guimap[i];

        if (guim_pp->spamap >= 0) {
            PrUnlinkAnimation(guim_pp->spmHdl);
            PrCleanupAnimation(guim_pp->spaHdl);
        }

        if (guim_pp->spamapP >= 0) {
            PrUnlinkPositionAnimation(guim_pp->spmHdl);
            PrCleanupAnimation(guim_pp->spaHdlP);
        }
        
        PrCleanupModel(guim_pp->spmHdl);
    }

    PrCleanupCamera(guime_camera_hdl);
    PrCleanupScene(guime_hdl);

    PrSetMendererRatio(saved_ratio);
}

int MbarDispGuiScene(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    int   *use_mappp;
    int    use_mappp_cnt;
    int    i;
    float  saved_ratio;

    use_mappp = NULL;
    use_mappp_cnt = 0;

    saved_ratio = PrGetMendererRatio();
    PrSetMendererRatio(0.0f);

    if (global_data.play_step == PSTEP_GAME || global_data.play_step == PSTEP_VS) {
        switch (clearStageCheck()) {
        case P3_STAGE_0:
            guimap[GUIME_HARI_L].spamapP = 0x44;
            guimap[GUIME_HARI_M].spamapP = 0x40;
            guimap[GUIME_HARI_R].spamapP = 0x46;
            break;
        case P3_STAGE_1:
            guimap[GUIME_HARI_L].spamapP = 0x3e;
            guimap[GUIME_HARI_M].spamapP = 0x42;
            guimap[GUIME_HARI_R].spamapP = 0x46;
            break;
        default:
            guimap[GUIME_HARI_L].spamapP = 0x40;
            guimap[GUIME_HARI_M].spamapP = 0x44;
            guimap[GUIME_HARI_R].spamapP = 0x46;
            break;
        }
    }

    ChangeDrawArea(DrawGetDrawEnvP(drDisp));

    PR_SCOPE()
    VCLR_PARA vclr_para = {};
    DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);
    PR_SCOPEEND()

    otehonAniCnt++;
    otehonAniCnt %= 140;

    guidisp_init_pr();

    switch (global_data.play_step) {
    case PSTEP_GAME: {
        int clrstg;

        use_mappp = guimap_single;
        use_mappp_cnt = 3;

        if (useDisp != DNUM_NON) {
            PrShowModel(guimap[GUIME_BXY].spmHdl, NULL);
            PrShowModel(guimap[GUIME_NEW_OTEHON].spmHdl, NULL);
        }

        clrstg = clearStageCheck();
        if (clrstg > P3_STAGE_1) {
            PrShowModel(guimap[GUIME_HARI_R].spmHdl, NULL);
        }
        if (clrstg > P3_STAGE_0) {
            PrShowModel(guimap[GUIME_HARI_M].spmHdl, NULL);
        }

        break;
    }
    case PSTEP_VS: {
        int curnum, clrstg;

        use_mappp = guimap_vs;
        use_mappp_cnt = 4;

        if (useDisp != DNUM_NON && vs_mouse_disp_flag) {
            PrShowModel(guimap[GUIME_BXY].spmHdl, NULL);
            PrShowModel(guimap[GUIME_NEW_OTEHON].spmHdl, NULL);
        }

        curnum = mbar_req_str[PINDEX_TEACHER].gui_cursor_enum - 3;
        if (curnum < 0) {
            curnum = mbar_ctrl_stage_selT;
        }
        mbar_ctrl_stage_selT = curnum;

        clrstg = clearStageCheck();
        if (clrstg > P3_STAGE_1) {
            PrShowModel(guimap[GUIME_HARI_R].spmHdl, NULL);
        }
        if (clrstg > P3_STAGE_0) {
            PrShowModel(guimap[GUIME_HARI_M].spmHdl, NULL);
        }

        break;
    }
    case PSTEP_XTR:
        use_mappp = guimap_sr;
        use_mappp_cnt = 1;
        break;
    case PSTEP_HOOK:
    case PSTEP_BONUS:
        use_mappp = guimap_hk;
        use_mappp_cnt = 1;
        break;
    case PSTEP_SERIAL:
        break;
    }

    if (use_mappp != NULL) {
        for (i = 0; i < use_mappp_cnt; i++) {
            if (game_status.subtitle == SUBTITLE_OFF) {
                if (use_mappp[i] == GUIME_JIMAKU) {
                    continue;
                }
                if (use_mappp[i] == GUIME_JIMAKU_SER) {
                    continue;
                }
            }

            PrShowModel(guimap[use_mappp[i]].spmHdl, NULL);
        }
    }

    guidisp_draw_quit(drDisp);

    PrSetMendererRatio(saved_ratio);

    PR_SCOPE()
    static sceGifPacket mbarNiko_gif;
    CmnGifADPacketMake(&mbarNiko_gif, NULL);
    MbarNikoDisp(&mbarNiko_gif);
    CmnGifADPacketMakeTrans(&mbarNiko_gif);
    PR_SCOPEEND()

    return 0;
}

int MbarDispGuiSceneMbarArea(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    float saved_ratio;

    saved_ratio = PrGetMendererRatio();
    PrSetMendererRatio(0.0f);

    ChangeDrawArea(DrawGetDrawEnvP(drDisp));

    PR_SCOPE()
    VCLR_PARA vclr_para = {};
    DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);
    PR_SCOPEEND()

    otehonAniCnt++;
    otehonAniCnt %= 140;

    guidisp_init_pr();

    switch (global_data.play_step) {
    case PSTEP_SERIAL:
        break;
    case PSTEP_HOOK:
    case PSTEP_GAME:
    case PSTEP_BONUS:
    case PSTEP_VS:
        if (game_status.subtitle == SUBTITLE_ON) {
            PrShowModel(guimap[GUIME_JIMAKU].spmHdl, NULL);
        }
        break;
    case PSTEP_XTR:
        if (game_status.subtitle == SUBTITLE_ON) {
            PrShowModel(guimap[GUIME_JIMAKU_SER].spmHdl, NULL);
        }
        break;
    }

    guidisp_draw_quit(drDisp);

    PrSetMendererRatio(saved_ratio);
    return 0;
}

TIM2_DAT* lessonTim2InfoGet(void) {
    return &tim2spr_tbl[52];
}

TIM2_DAT* lessonCl2InfoGet(SCRRJ_LESSON_ROUND_ENUM type) {
    u_short le_num[10] = {
        /* SCRRJ_LR_LESSON_1 */ 0x2d,
        /* SCRRJ_LR_LESSON_2 */ 0x2e,
        /* SCRRJ_LR_LESSON_3 */ 0x2f,
        /* SCRRJ_LR_LESSON_4 */ 0x30,
        /* SCRRJ_LR_LESSON_5 */ 0x31,

        /* SCRRJ_LR_ROUND_1  */ 0x2d,
        /* SCRRJ_LR_ROUND_2  */ 0x2e,
        /* SCRRJ_LR_ROUND_3  */ 0x2f,
        /* SCRRJ_LR_ROUND_4  */ 0x30,
        /* SCRRJ_LR_ROUND_5  */ 0x31,
    };

    return &tim2spr_tbl[le_num[type]];
}

void MbarDemoCharDisp(void) {
    TIM2_DAT *tim2_dat_pp;
    SPR_PRIM  spr_prim = {
        .x = 2298,
        .y = 2128,
        .scalex = 256,
        .scaley = 128,
        .u = 0,
        .v = 0,
        .w = 0,
        .h = 0,
    };

    SprInit();
    ChangeDrawArea(DrawGetDrawEnvP(2));
    tim2_dat_pp = &tim2spr_tbl[53];
    spr_prim.w = tim2_dat_pp->w;
    spr_prim.h = tim2_dat_pp->h;
    SprClear();
    SprDispAlphaSet();
    SprPackSet((SPR_DAT*)tim2_dat_pp);
    SprDispZABnclr();
    SprSetColor(128, 128, 128, 128);
    SprDispAlp(&spr_prim);
    SprFlash();
}
