#ifndef DRAWCTRL_H
#define DRAWCTRL_H

#include "common.h"

#include <eetypes.h>

#include "main/etc.h"
#include "main/effect.h"
#include "main/sprite.h"

#include <prlib/prlib.h>

typedef int (*OVL_FUNC)(void *para_pp, int frame, int first_f, int useDisp, int drDisp);

typedef struct { // 0x8
    /* 0x0 */ TIM2_DAT *tim2_dat;
    /* 0x4 */ SPR_PRIM *spr_prim;
} TIM2DISP_STR;

typedef enum {
    MEN_CTRL_BtoG = 0,
    MEN_CTRL_AtoAB = 1,
    MEN_CTRL_ABtoB = 2,
    MEN_CTRL_AtoB = 3,
    MEN_CTRL_GtoB = 4,
    MEN_CTRL_BtoBA = 5,
    MEN_CTRL_BAtoA = 6,
    MEN_CTRL_BtoA = 7
} MEN_CTRL_ENUM;

typedef enum {
    OBJBTHROW_TEACHER = 0,
    OBJBTHROW_PARAPPA = 1,
    OBJBTHROW_MAX = 2
} OBJBTHROW_TYPE;

typedef struct { // 0x1c
    /* 0x00 */ u_char use;
    /* 0x04 */ float xp;
    /* 0x08 */ float yp;
    /* 0x0c */ void *mdl_adr;
    /* 0x10 */ void *tim2_dat_pp;
    /* 0x14 */ void *homingpp;
    /* 0x18 */ int endTime;
} BTHROW_STR;

/* BTHROW_STR::use flags. */
#define BTHROW_USE_ACTIVE 0x1 /* in flight: moved, drawn and counted down each frame */
#define BTHROW_USE_LAUNCH 0x2 /* first frame: start from the thrower model's screen position */

typedef struct { // 0xe14
    /* 0x000 */ int frame;
    /* 0x004 */ BTHROW_STR bthrow_str[128];
    /* 0xe04 */ int bthrow_str_cnt;
    /* 0xe08 */ float targetX;
    /* 0xe0c */ float targetY;
    /* 0xe10 */ void *targ_mdl_adr;
} BTHROW_CTRL;

typedef struct { // 0xc
    /* 0x0 */ int mozaiku_str_frame;
    /* 0x4 */ int mozaiku_str_cnt;
    /* 0x8 */ MOZAIKU_STR *mozaiku_str_pp;
} MOZAIKU_POLL_STR;

typedef enum {
    ODAT_SPA = 0,
    ODAT_SPM = 1,
    ODAT_SPC = 2,
    ODAT_SPF = 3,
    ODAT_TM2 = 4,
    ODAT_CL2 = 5,
    ODAT_MAX = 6
} OBJDAT_TYPE;

typedef struct { // 0x14
    /* 0x00 */ u_short objdat_type;
    /* 0x02 */ u_short objdat_num;
    /* 0x04 */ int maxfr;
    /* 0x08 */ void *handle;
    /* 0x0c */ float subdat[2];
} OBJDAT;

typedef struct { // 0x1c
    /* 0x00 */ u_char first_flag;
    /* 0x01 */ u_char job_type;
    /* 0x02 */ u_short status;
    /* 0x04 */ u_short main_num;
    /* 0x06 */ u_short sub_num;
    /* 0x08 */ u_int start_time;
    /* 0x0c */ u_int end_time;
    /* 0x10 */ u_int now_time;
    /* 0x14 */ float focal_lng;
    /* 0x18 */ float defocus_lng;
} OBJACTPRG;

enum {
    OBJACTPRG_ORG = 0,
    OBJACTPRG_NORMAL = 1,
    OBJACTPRG_TAP = 2,
    OBJACTPRG_MAX = 3
};

typedef struct { // 0x4
    /* 0x0 */ u_int maxFrame;
    /* 0x4 */ float frame[0];
} SPF_STR;

typedef enum {
    OBJSTR_REQ = 1,
    OBJSTR_ON = 2,
    OBJSTR_PRESSON = 4
} OBJSTR_ENUM;

typedef enum {
    OCTRL_NON = 0,
    OCTRL_ANI = 1,
    OCTRL_MDL = 2,
    OCTRL_CAM = 3,
    OCTRL_TM2 = 4,
    OCTRL_TCTRL = 5,
    OCTRL_SUB = 6,
    OCTRL_END = 7,
    OCTRL_LOOP_P = 8,
    OCTRL_NEXT = 9,
    OCTRL_LOOP = 10,
    OCTRL_EXIT = 11,
    OCTRL_BIBU = 12,
    OCTRL_ANIPOS = 13,
    OCTRL_CL2 = 14,
    OCTRL_BTHROW = 15,
    OCTRL_BHIT = 16,
    OCTRL_ANIPOSXX = 17,
    OCTRL_MOZAIKU = 18,
    OCTRL_MAX = 19
} OBJCTRL_TYPE;

/* OBJCTRL::status, copied into OBJACTPRG::status when the control entry becomes active. */
enum {
    OCTRL_STAT_DISABLED = 0x1,       /* entry is skipped */
    OCTRL_STAT_LOOP = 0x2,           /* animation time wraps at the end */
    OCTRL_STAT_START_OVERRIDE = 0x4, /* start time comes from dat[2] */
    OCTRL_STAT_END_OVERRIDE = 0x8,   /* end time comes from dat[3] */
    OCTRL_STAT_BLUR = 0x10,          /* BLMV_BLUR link option */
    OCTRL_STAT_MOVE = 0x20,          /* BLMV_MOVE link option */
    OCTRL_STAT_CONTINUE = 0x40,      /* carry on from the previous program's time */
    OCTRL_STAT_SAVE_TIME = 0x80,     /* store the current time in slot dat[4] */
    OCTRL_STAT_LOAD_TIME = 0x100,    /* resume from the time stored in slot dat[4] */
    OCTRL_STAT_BLUR2 = 0x200         /* BLMV_BLUR2 link option */
};

typedef struct { // 0x20
    /* 0x00 */ u_int frame;
    /* 0x04 */ u_short objctrl_type;
    /* 0x06 */ u_short status;
    /* 0x08 */ u_short dat[5];
    /* 0x14 */ float subDat;
    /* 0x18 */ u_char PRflag;
    /* 0x1a */ u_short PRdata;
    /* 0x1c */ u_int PRtime;
} OBJCTRL;

typedef struct { // 0x24
    /* 0x00 */ int size;
    /* 0x04 */ OBJCTRL *objctrl_pp;
    /* 0x08 */ u_char PRflag;
    /* 0x0a */ u_short PRdata;
    /* 0x0c */ int PRtime;
    /* 0x10 */ int PRtimeOld;
    /* 0x14 */ u_char *PRpress;
    /* 0x18 */ OBJCTRL *current_pp;
    /* 0x1c */ u_int loop_time;
    /* 0x20 */ OBJCTRL *loop_pp;
} OBJSTR;

typedef enum {
    OTE_LOOP_P = 65520,
    OTE_NEXT = 65521,
    OTE_LOOP = 65522
} OBJTAP_LOOP_ENUM;

typedef struct { // 0x4
    /* 0x0 */ u_short obj_num;
    /* 0x2 */ u_short channel;
} OBJTAP;

typedef struct { // 0xc
    /* 0x0 */ int objtap_size;
    /* 0x4 */ OBJTAP *objtap_pp;
    /* 0x8 */ int ovl_cnt_num;
} OBJTAPSTR;

typedef struct { // 0x10
    /* 0x0 */ int num;
    /* 0x4 */ OBJACTPRG *objactprg[3];
} OBJACTPRG_CTRL;

/* tap_id of a request that applies to every scene object */
#define DR_TAP_ID_ALL 0xfe

/* DrawTapReqTbl request word: tap id in the high byte, request number in the low byte. */
#define DR_TAP_REQ(id, no) (((id) << 8) | (no))

/* Request numbers the game sends to every object (DR_TAP_ID_ALL). */
#define DR_TAP_REQ_RANK_DOWN   2 /* the rank got worse at a line change */
#define DR_TAP_REQ_RANK_UP     3 /* the rank improved (stage 6 also sends it when unchanged) */
#define DR_TAP_REQ_TITLE_START 4 /* START pressed on the title */

typedef struct { // 0xc
    /* 0x0 */ short tap_id;
    /* 0x2 */ short req_no;
    /* 0x4 */ PLAYER_INDEX player_index;
    /* 0x8 */ u_char *pad_prs_pp;
} DR_TAP_REQ;

typedef struct { // 0x21c
    /* 0x000 */ int objdat_size;
    /* 0x004 */ OBJDAT *objdat_pp;
    /* 0x008 */ int objstr_size;
    /* 0x00c */ OBJSTR *objstr_pp;
    /* 0x010 */ int tapstr_size;
    /* 0x014 */ OBJSTR *tapstr_pp;
    /* 0x018 */ int objtapstr_size;
    /* 0x01c */ OBJTAPSTR *objtapstr_pp;
    /* 0x020 */ char *usrName;
    /* 0x024 */ int tap_id;
    /* 0x028 */ void *handle;
    /* 0x02c */ OBJACTPRG_CTRL objactprg_ctrl;
    /* 0x03c */ DR_TAP_REQ dr_tap_req[40];
} SCENE_OBJDATA;

typedef enum {
    SC_UF_NOUSE = 0,
    SC_UF_USENOW = 1
} SCENECTRL_USE_FLAG_ENUM;

typedef enum {
    SBI_R1 = 0,
    SBI_R2 = 1,
    SBI_R3 = 2,
    SBI_R4 = 3,
    SBI_TBL0 = 4,
    SBI_TBL1 = 5,
    SBI_TBL2 = 6,
    SBI_TBL3 = 7,
    SBI_TBL4 = 8,
    SBI_TBL5 = 9,
    SBI_TBL6 = 10,
    SBI_TBL7 = 11,
    SBI_CLR0 = 12,
    SBI_CLR1 = 13,
    SBI_CLR2 = 14,
    SBI_CLR3 = 15,
    SBI_CLR4 = 16,
    SBI_CLR5 = 17,
    SBI_CLR6 = 18,
    SBI_CLR7 = 19,
    SBI_CLR8 = 20,
    SBI_MAX = 21,
    SBI_ANDBIT = 31
} SCEN_BRA_INDEX;

typedef enum {
    SBE_ALL = 0,
    SBE_R1 = 1,
    SBE_R2 = 2,
    SBE_R3 = 4,
    SBE_R4 = 8,
    SBE_TBL0 = 16,
    SBE_TBL1 = 32,
    SBE_TBL2 = 64,
    SBE_TBL3 = 128,
    SBE_TBL4 = 256,
    SBE_TBL5 = 512,
    SBE_TBL6 = 1024,
    SBE_TBL7 = 2048,
    SBE_CLR0 = 4096,
    SBE_CLR1 = 8192,
    SBE_CLR2 = 16384,
    SBE_CLR3 = 32768,
    SBE_CLR4 = 65536,
    SBE_CLR5 = 131072,
    SBE_CLR6 = 262144,
    SBE_CLR7 = 524288,
    SBE_CLR8 = 1048576,
    SBE_ANDBIT = -2147483648
} SCEN_BRA_ENUM;

typedef struct { // 0x20
    /* 0x00 */ u_int start_flame;
    /* 0x04 */ u_int end_flame;
    /* 0x08 */ u_int useDisp;
    /* 0x0c */ u_int drDisp;
    /* 0x10 */ u_char pri; /* draw order, lowest first; see DRAW_PRI_* */
    /* 0x11 */ u_char use_flag;
    /* 0x14 */ OVL_FUNC prg_pp;
    /* 0x18 */ void *param_pp;
    /* 0x1c */ u_int condition;
} SCENECTRL;

typedef enum {
    DRPRGF_INIT = -2,
    DRPRGF_RESET = -1,
    DRPRGF_FIRST = 0,
    DRPRGF_TIMES = 1
} DRAWPRG_FLAG;

typedef struct { // 0x8
    /* 0x0 */ int scenectrl_num;
    /* 0x4 */ SCENECTRL *scenectrl_pp;
} SCENESTR;

typedef struct { // 0x8
    /* 0x0 */ int scenestr_size;
    /* 0x4 */ SCENESTR *scenestr_pp;
} EVENTREC;

typedef struct { // 0x3
    /* 0x0:00 */ u_char r : 5;
    /* 0x1:00 */ u_char g : 5;
    /* 0x2:00 */ u_char b : 5;
    /* 0x2:05 */ u_char a : 1;
} RGB15TR;

typedef struct { // 0x8
    /* 0x0 */ int frame;
    /* 0x4 */ float ratio;
} MENTITLE_DAT;

typedef struct { // 0xc
    /* 0x0 */ char *dbgmsg;
    /* 0x4 */ int  (*prg_pp)(int pad);
    /* 0x8 */ void (*msg_pp)(char *buf);
} DRAW_DBG_STR;

typedef struct { // 0xc
    /* 0x0 */ int frame;
    /* 0x4 */ int xp;
    /* 0x8 */ int yp;
} MOVEIN_DATA;

typedef struct { // 0x10
    /* 0x0 */ short u;
    /* 0x2 */ short v;
    /* 0x4 */ short w;
    /* 0x6 */ short h;
    /* 0x8 */ int movein_data_size;
    /* 0xc */ MOVEIN_DATA *movein_data_pp;
} MOVEIN_PARA;

typedef enum {
    DDSP_HMOVE_L = 0,
    DDSP_HMOVE_R = 1,
    DDSP_VMOVE_U = 2,
    DDSP_VMOVE_D = 3
} DDSP_ENUM;

typedef struct { // 0x18
    /* 0x00 */ DDSP_ENUM type;
    /* 0x04 */ float pos_start;
    /* 0x08 */ float pos_add;
    /* 0x0c */ int move_size;
    /* 0x10 */ float next_line_ang;
    /* 0x14 */ float next_time_ang;
} DOUBLE_PARA;

/* Draw order (SCENECTRL::pri) of the scenes the game requests each frame
 * besides the score's own. Score scenes above DRAW_PRI_MENDERER draw without
 * the noodle effect. */
#define DRAW_PRI_BALL_THROW 200
#define DRAW_PRI_MENDERER   210
#define DRAW_PRI_MOZAIKU    215
#define DRAW_PRI_MBAR       220
#define DRAW_PRI_VRAM_CLEAR 229
#define DRAW_PRI_GUI        240

void outsideDrawSceneClear(void);
int outsideDrawSceneReq(int (*prg_pp)(void *para_pp, int frame, int first_f, int useDisp, int drDisp), u_char pri, u_int useF, u_int drawF, void *param);

float* bra_tap_GetNext(PR_MODELHANDLE model);
float* bra_title_GetNext(PR_MODELHANDLE model);
float* bra_ret_GetNext(PR_MODELHANDLE model);

void XAnimationLinkOption(PR_MODELHANDLE model, PR_ANIMATIONHANDLE animation, int first, int blumove, float time);
void XAnimationPositionLink(PR_MODELHANDLE model, PR_ANIMATIONHANDLE animation, float time);

void BallThrowSetFrame(int frame);
void BallThrowInit(void);
void BallThrowInitDare(int dare);
void BallThrowReq(void *mdlh, OBJBTHROW_TYPE thtype, void *texpp, void *mdlhoming);
void BallThrowTarget(void *mdlh, OBJBTHROW_TYPE thtype, int targetframe);
void BallThrowPoll(void);
int BallThrowPollScene(void *para_pp, int frame, int first_f, int useDisp, int drDisp);

void DrawTmpBuffInit(void);
void DrawTmpBuffQuit(void *adrs);
void* DrawTmpBuffGetArea(int size);

void DrawObjdatInit(int size, OBJDAT *od_pp, PR_SCENEHANDLE prf);
void DrawObjdatReset(int size, OBJDAT *od_pp);

void DrawSceneInit(sceGsDrawEnv1 *draw_env, SCENE_OBJDATA *scene_pp, int useDisp);

void DrawObjStrTapQuit(SCENE_OBJDATA *scn_pp, int num, u_int time);
void DrawObjTapStrTapQuit(SCENE_OBJDATA *scn_pp, int req_num);

void camOtherKill(OBJACTPRG *objactprg_pp, int objactprg_num, int oya_num);
void posAniOtherKill(OBJACTPRG *objactprg_pp, int objactprg_num, int ani_num, int mod_num);

void DrawSceneReset(SCENE_OBJDATA *scene_pp);
void DrawSceneFirstSet(SCENE_OBJDATA *scene_pp);

void Cl2MixTrans(int now_T, int max_T, u_char *cl2_0_pp, u_char *cl2_1_pp);

void DrawObjPrReq(SCENE_OBJDATA *scene_pp);

void DrawObjStrTapTimeNext(SCENE_OBJDATA *sod_pp);

void DrawObjTapCtrl(SCENE_OBJDATA *sod_pp, DR_TAP_REQ *tap_pp, int tap_num);

/* Overlay functions */
int DrawSceneObjData(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawDoubleDispIn(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawVramClear(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawMoveDispIn(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawAlphaBlendDisp(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawMozaikuDisp(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawFadeDisp(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawPlphaIndex8Disp(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawTim2DIsp(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
int DrawNoodlesDisp(void *para_pp,  int frame, int first_f, int useDisp, int drDisp);
int DrawVramLocalCopy(void *para_pp, int frame, int first_f,  int useDisp, int drDisp);
int DrawVramLocalCopy2(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
/* Overlay functions end */

void MendererCtrlTitle(void);
void MendererCtrlTitleDera(void);
void MendererReq(MEN_CTRL_ENUM menum);
MEN_CTRL_ENUM GetMendererEnum(void);

/* Overlay function */
int MendererCtrlScene(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
/* Overlay function end */

int sceneConditionCheck(u_int cond_flag);
void resetDrawSceneObjData(SCENESTR *scstr_pp);

void DrawCtrlInit(EVENTREC *ev_pp, int ctrlTbl, void *dat_top);
void DrawCtrlQuit(void);

void DrawCtrlTimeSet(int time);
void DrawCtrlTblChange(int ctrlTbl);

int DrawTapReqTbl(int atap, PLAYER_INDEX pindx, u_char *prs_pp);

void ddbg_tctrl_sub(void);

void DrawCtrlInitDebug(EVENTREC *ev_pp, int ctrlTbl, void *dat_top);

#endif /* DRAWCTRL_H */
