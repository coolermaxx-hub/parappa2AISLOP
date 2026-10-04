#include "main/drawctrl.h"

#include "dbug/dbgmsg.h"
#include "dbug/vramsave.h"

#include "os/cmngifpk.h"
#include "os/mtc.h"
#include "os/syssub.h"
#include "os/system.h"
#include "os/tim2.h"

#include "main/cdctrl.h"
#include "main/mbar.h"
#include "main/sprite.h"
#include "main/p3str.h"

#include <prlib/prlib.h>

#include <eestruct.h>
#include <libgraph.h>
#include <libpad.h>

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static VCLR_PARA vclr_black = { 0, 0, 0, 0 };
/* Tap-bar layouts, mosaic stage tables and title-cut timings. */
static float bra_tap[10][2] = {
    { 0.25f, 0.1f },
    { 0.225f, 0.09f },
    { 0.2f, 0.08f },
    { 0.175f, 0.07f },
    { 0.15f, 0.06f },
    { 0.125f, 0.05f },
    { 0.1f, 0.04f },
    { 0.075f, 0.03f },
    { 0.05f, 0.02f },
    { 0.025f, 0.01f },
};

static float bra_title[16][2] = {
    { 0.2f, 0.08f },
    { 0.1875f, 0.075f },
    { 0.175f, 0.07f },
    { 0.1625f, 0.065f },
    { 0.15f, 0.06f },
    { 0.1375f, 0.055f },
    { 0.125f, 0.05f },
    { 0.1125f, 0.045f },
    { 0.1f, 0.04f },
    { 0.0875f, 0.035f },
    { 0.075f, 0.03f },
    { 0.0625f, 0.025f },
    { 0.05f, 0.02f },
    { 0.0375f, 0.015f },
    { 0.025f, 0.01f },
    { 0.0125f, 0.005f },
};

static float bra_ret[49] = {
    0.02f, 0.04f, 0.06f, 0.08f, 0.1f, 0.12f, 0.14f,
    0.16f, 0.18f, 0.2f, 0.22f, 0.24f, 0.26f, 0.28f,
    0.3f, 0.32f, 0.34f, 0.36f, 0.38f, 0.4f, 0.42f,
    0.44f, 0.46f, 0.48f, 0.5f, 0.52f, 0.54f, 0.56f,
    0.58f, 0.6f, 0.62f, 0.64f, 0.66f, 0.68f, 0.7f,
    0.72f, 0.74f, 0.76f, 0.78f, 0.8f, 0.82f, 0.84f,
    0.86f, 0.88f, 0.9f, 0.92f, 0.94f, 0.96f, 0.98f,
};

static MOZAIKU_STR mozaiku_str_poll_00[17] = {
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
};

static MOZAIKU_STR mozaiku_str_poll_01[54] = {
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
};

static MOZAIKU_STR mozaiku_str_poll_02[34] = {
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3e0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f0, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f0, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3f8, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3f8, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fc, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fc, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3fe, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
    { 0x3fe, 0x0, 0x3ff, 0x0 },
};

static MOZAIKU_POLL_STR mozaiku_poll_str[3] = {
    { 0, 17, mozaiku_str_poll_00 },
    { 0, 54, mozaiku_str_poll_01 },
    { 0, 34, mozaiku_str_poll_02 },
};

static MENTITLE_DAT mentitle_dat[] = {
    { 0, 1.5f },
    { 108, 1.5f },
    { 144, 0.0f },
    { 792, 0.0f },
    { 864, 1.0f },
    { 1216, 1.0f },
    { 1296, 0.0f },
    { 0xffffffe, 0.0f },
};

static MENTITLE_DAT mentitle_dat_dera[] = {
    { 0, 1.5f },
    { 108, 1.5f },
    { 144, 0.0f },
    { 792, 0.0f },
    { 864, 1.0f },
    { 1216, 1.0f },
    { 1296, 0.0f },
    { 2160, 0.0f },
    { 2220, 1.0f },
    { 2280, 1.0f },
    { 2340, 0.0f },
    { 2446, 0.0f },
    { 2506, 1.0f },
    { 2566, 1.0f },
    { 2626, 0.0f },
    { 2734, 0.0f },
    { 2794, 1.0f },
    { 2854, 1.0f },
    { 2914, 0.0f },
    { 3020, 0.0f },
    { 3080, 1.0f },
    { 3140, 1.0f },
    { 3200, 0.0f },
    { 3308, 0.0f },
    { 3368, 1.0f },
    { 3428, 1.0f },
    { 3488, 0.0f },
    { 3594, 0.0f },
    { 3654, 1.0f },
    { 3714, 1.0f },
    { 3774, 0.0f },
    { 3882, 0.0f },
    { 3942, 1.0f },
    { 4002, 1.0f },
    { 4062, 0.0f },
    { 4168, 0.0f },
    { 4228, 1.0f },
    { 4288, 1.0f },
    { 4348, 0.0f },
    { 4456, 0.0f },
    { 4516, 1.0f },
    { 4576, 1.0f },
    { 4636, 0.0f },
    { 4742, 0.0f },
    { 4802, 1.0f },
    { 4862, 1.0f },
    { 4922, 0.0f },
    { 5030, 0.0f },
    { 5090, 1.0f },
    { 5150, 1.0f },
    { 5210, 0.0f },
    { 5316, 0.0f },
    { 5376, 1.0f },
    { 5436, 1.0f },
    { 5496, 0.0f },
    { 5604, 0.0f },
    { 5664, 1.0f },
    { 5724, 1.0f },
    { 5784, 0.0f },
    { 5890, 0.0f },
    { 5950, 1.0f },
    { 6010, 1.0f },
    { 6070, 0.0f },
    { 6178, 0.0f },
    { 6238, 1.0f },
    { 6298, 1.0f },
    { 6358, 0.0f },
    { 6464, 0.0f },
    { 6524, 1.0f },
    { 6584, 1.0f },
    { 6644, 0.0f },
    { 6752, 0.0f },
    { 6812, 1.0f },
    { 6872, 1.0f },
    { 6932, 0.0f },
    { 7038, 0.0f },
    { 7098, 1.0f },
    { 7158, 1.0f },
    { 7218, 0.0f },
    { 7326, 0.0f },
    { 7386, 1.0f },
    { 7446, 1.0f },
    { 7506, 0.0f },
    { 7612, 0.0f },
    { 7672, 1.0f },
    { 7732, 1.0f },
    { 7792, 0.0f },
    { 7900, 0.0f },
    { 7960, 1.0f },
    { 8020, 1.0f },
    { 8080, 0.0f },
    { 8186, 0.0f },
    { 8246, 1.0f },
    { 8306, 1.0f },
    { 8366, 0.0f },
    { 8474, 0.0f },
    { 8534, 1.0f },
    { 8594, 1.0f },
    { 8654, 0.0f },
    { 8760, 0.0f },
    { 8820, 1.0f },
    { 8880, 1.0f },
    { 8940, 0.0f },
    { 9048, 0.0f },
    { 9108, 1.0f },
    { 9168, 1.0f },
    { 9228, 0.0f },
    { 9334, 0.0f },
    { 9394, 1.0f },
    { 9454, 1.0f },
    { 9514, 0.0f },
    { 9622, 0.0f },
    { 9682, 1.0f },
    { 9742, 1.0f },
    { 9802, 0.0f },
    { 9908, 0.0f },
    { 9968, 1.0f },
    { 10028, 1.0f },
    { 10088, 0.0f },
    { 10196, 0.0f },
    { 10256, 1.0f },
    { 10316, 1.0f },
    { 10376, 0.0f },
    { 10482, 0.0f },
    { 10542, 1.0f },
    { 10602, 1.0f },
    { 10662, 0.0f },
    { 10770, 0.0f },
    { 10830, 1.0f },
    { 10890, 1.0f },
    { 10950, 0.0f },
    { 11056, 0.0f },
    { 11116, 1.0f },
    { 11176, 1.0f },
    { 11236, 0.0f },
    { 11344, 0.0f },
    { 11404, 1.0f },
    { 11464, 1.0f },
    { 11524, 0.0f },
    { 11630, 0.0f },
    { 11690, 1.0f },
    { 11750, 1.0f },
    { 11810, 0.0f },
    { 11918, 0.0f },
    { 11978, 1.0f },
    { 12038, 1.0f },
    { 12098, 0.0f },
    { 12204, 0.0f },
    { 12264, 1.0f },
    { 12324, 1.0f },
    { 12384, 0.0f },
    { 12492, 0.0f },
    { 12552, 1.0f },
    { 12612, 1.0f },
    { 12672, 0.0f },
    { 12778, 0.0f },
    { 12838, 1.0f },
    { 12898, 1.0f },
    { 12958, 0.0f },
    { 13066, 0.0f },
    { 13126, 1.0f },
    { 13186, 1.0f },
    { 13246, 0.0f },
    { 13352, 0.0f },
    { 13412, 1.0f },
    { 13472, 1.0f },
    { 13532, 0.0f },
    { 0xffffffe, 0.0f },
};
static int bthrow_ctrl_time = 0;
static DR_TAP_REQ dr_tap_req[16];
static int octst_time[8];
static int octst_timeLoad[8];
static SCENECTRL scenectrl_outside[8];
static BTHROW_CTRL bthrow_ctrl[2];
static void *tmp_buf_adrs[16];
static SCENECTRL *check_scenectrl[20];
static int dr_tap_req_num;
static int scenectrl_outside_cnt;
static int scenectrl_outside_read_cnt;
static MOZAIKU_POLL_STR *mozaiku_poll_str_current_pp;
static int drawCurrentLine;
static int drawCurrentTime;
static int drawCurrentTimeOld;
static EVENTREC *drawEventrec;
static u_int useDispFlag;
static u_int drDispFlag;
static float men_ctrl_ratio;
static MEN_CTRL_ENUM men_ctrl_enum;
static u_char ddbg_pause_f;

static void  UseGsRegSet(void);
static void  UseGsSetXyOffset(int ofs);
static SCENECTRL* getOutsideCtrlScene(int time);
static void* vs06BomAdr(OBJBTHROW_TYPE thtype, int time);
static void  MozaikuPollSceneInit(void);
static void  MozaikuPollSceneReq(int id);
static int   MozaikuPollScene(void *para_pp, int frame, int first_f, int useDisp, int drDisp);
static void  DrawObjCtrlInit(OBJCTRL *oc_pp);
static void  DrawObjStrInit(OBJSTR *os_pp);
static void  DrawObjTapStrInit(OBJTAPSTR *objtapstr_pp);
static void  DrawObjStrReq(SCENE_OBJDATA *scn_pp, int num, u_int time);
static void  DrawObjStrTapReq(SCENE_OBJDATA *scn_pp, int num, u_int time, u_char *prs_adr);
static void  DrawObjTapStrTapReq(SCENE_OBJDATA *scn_pp, int req_num, u_int time, u_char *prs_adr);
static void  DrawObjStrKill(SCENE_OBJDATA *scn_pp, int num);
static u_int GetSpfTimeCtrl(OBJDAT *objdat_pp, u_int frame);
static int   DrawObjStrDisp(SCENE_OBJDATA *scn_pp, int num, u_int time, int sw);
static int   DrawObjStrDispTap(SCENE_OBJDATA *scn_pp, int num);
static void  drawUseDrDispCheckInit(void);
static int   drawDispCheckSub(u_int drD, u_int *dat_pp);
static int   drawUseDispCheck(int useD);
static int   drawDrDispCheck(int drD);
static int   DrawScenectrlReq(SCENECTRL *scenectrl_pp, u_int time);
static void  MendererCtrlInit(void);
static void  MendererCtrl(void);
static float mendRatioTitleGet(int frame, int dera_f);
static void  MendererCtrlTitleDisp(int frame, int dera_f);
static void  DrawCtrlMain(void *x);
static void  DrawSceneStrInit(SCENESTR *scstr_pp);
static void  DrawSceneStrReset(SCENESTR *scstr_pp);
/*static*/ int   ddbg_event_sel(int pad);
/*static*/ void  ddbg_event_msg(char *buf);
/*static*/ int   ddbg_scene_sel(int pad);
/*static*/ void  ddbg_scene_msg(char *buf);
/*static*/ int   ddbg_event_check(int pad);
/*static*/ int   ddbg_scene_check(int pad);
/*static*/ int   ddbg_bmp_check(int pad);
static void  ddbg_event_sub(void);
static void  ddbg_event_sub_bmp(void);
static void  ddbg_tap_check(void);
static void  ddbg_scene_sub(void);
static void  DrawCtrlMainDebug(void *x);

static void UseGsRegSet(void) {
    sceGifPacket gifpk;
  
    CmnGifADPacketMake(&gifpk, NULL);
    CmnGifADPacketMakeTrans(&gifpk);
}

void UseGsSetXyOffset(int ofs) {
    sceGifPacket gifpk;
    int          offsy;
  
    if (ofs) {
        offsy = GS_Y_COORD(0) + 8; /* half a pixel lower */
    } else {
        offsy = GS_Y_COORD(0);
    }

    CmnGifADPacketMake(&gifpk, NULL);
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET(GS_X_COORD(0), offsy));
    CmnGifADPacketMakeTrans(&gifpk);
}

void outsideDrawSceneClear(void) {
    scenectrl_outside_cnt = 0;
    scenectrl_outside_read_cnt = 0;
    WorkClear(scenectrl_outside, sizeof(scenectrl_outside));
}

int outsideDrawSceneReq(OVL_FUNC prg_pp, u_char pri, u_int useF, u_int drawF, void *param) {
    SCENECTRL *scenectrl_pp;

    if (scenectrl_outside_cnt >= PR_ARRAYSIZE(scenectrl_outside)) {
        printf("outside draw over!!\n");
        return 1;
    }

    scenectrl_pp = &scenectrl_outside[scenectrl_outside_cnt];

    scenectrl_pp->pri = pri;
    scenectrl_pp->condition = 0;

    scenectrl_pp->prg_pp = prg_pp;
    scenectrl_pp->useDisp = useF;
    scenectrl_pp->drDisp = drawF;
    scenectrl_pp->param_pp = param;

    scenectrl_outside_cnt++;
    return 0;
}

static SCENECTRL* getOutsideCtrlScene(int time) {
    SCENECTRL *scenectrl_pp;

    if (scenectrl_outside_read_cnt >= scenectrl_outside_cnt) {
        return NULL;
    }

    scenectrl_pp = &scenectrl_outside[scenectrl_outside_read_cnt];

    scenectrl_pp->start_flame = time;
    scenectrl_pp->end_flame = time + 1;

    scenectrl_outside_read_cnt++;
    return scenectrl_pp;
}

float* bra_tap_GetNext(PR_MODELHANDLE model) {
    int    i;
    float  tmp_f;
    float *ret;

    ret = NULL;
    tmp_f = PrGetContourBlurAlpha(model);

    if (tmp_f == -1.0f) {
        return NULL;
    }

    for (i = 0; i < PR_ARRAYSIZEU(bra_tap) - 1; i++) {
        if (bra_tap[i][0] == tmp_f) {
            ret = &bra_tap[i + 1][0];
            break;
        }
    }

    return ret;
}

float* bra_title_GetNext(PR_MODELHANDLE model) {
    int i;
    float tmp_f;
    float *ret;

    ret = NULL;
    tmp_f = PrGetContourBlurAlpha(model);

    if (tmp_f == -1.0f) {
        return NULL;
    }

    for (i = 0; i < PR_ARRAYSIZEU(bra_title) - 1; i++) {
        if (bra_title[i][0] == tmp_f) {
            ret = &bra_title[i + 1][0];
            break;
        }
    }

    return ret;
}

float* bra_ret_GetNext(PR_MODELHANDLE model) {
    int i;
    float tmp_f;
    float *ret;

    ret = NULL;
    tmp_f = PrGetTransactionBlendRatio(model);

    if (tmp_f == -1.0f) {
        return NULL;
    }

    for (i = 0; i < PR_ARRAYSIZEU(bra_ret) - 1; i++) {
        if (bra_ret[i] == tmp_f) {
            ret = &bra_ret[i + 1];
            break;
        }
    }

    return ret;
}

void XAnimationLinkOption(PR_MODELHANDLE model, PR_ANIMATIONHANDLE animation, int first, int blumove, float time) {
    PR_ANIMATIONHANDLE  anim_tmp;
    float              *tmp_dat;

    if (blumove == BLMV_MOVE) {
        first = FALSE;
    }

    anim_tmp = PrGetLinkedAnimation(model);
    if (anim_tmp != animation) {
        if (anim_tmp != NULL) {
            first = TRUE;
        } else {
            first = FALSE;
            PrLinkAnimation(model, animation);
            PrSetTransactionBlendRatio(model, -1.0f);
            PrSetContourBlurAlpha(model, -1.0f, -1.0f);
        }
    }

    if (first) {
        switch (blumove) {
        case BLMV_NONE:
            PrResetPosture(model);
            PrSetTransactionBlendRatio(model, -1.0f);
            PrResetContour(model);
            PrSetContourBlurAlpha(model, -1.0f, -1.0f);
            break;
        case BLMV_BLUR:
            PrResetPosture(model);
            PrSetTransactionBlendRatio(model, -1.0f);
            PrSaveContour(model);
            PrSetContourBlurAlpha(model, bra_tap[0][0], bra_tap[0][1]);
            break;
        case BLMV_BLUR2:
            PrResetPosture(model);
            PrSetTransactionBlendRatio(model, -1.0f);
            PrSaveContour(model);
            PrSetContourBlurAlpha(model, bra_title[0][0], bra_title[0][1]);
            break;
        case BLMV_MOVE:
            PrResetContour(model);
            PrSetContourBlurAlpha(model, -1.0f, -1.0f);
            PrSavePosture(model);
            PrSetTransactionBlendRatio(model, bra_ret[0]);
            break;
        }

        PrLinkAnimation(model, animation);
    } else {
        switch (blumove) {
        case BLMV_NONE:
            break;
        case BLMV_BLUR:
            tmp_dat = bra_tap_GetNext(model);
            if (tmp_dat == NULL) {
                PrResetContour(model);
                PrSetContourBlurAlpha(model, -1.0f, -1.0f);
            } else {
                PrSetContourBlurAlpha(model, tmp_dat[0], tmp_dat[1]);
            }
            break;
        case BLMV_BLUR2:
            tmp_dat = bra_title_GetNext(model);
            if (tmp_dat == NULL) {
                PrResetContour(model);
                PrSetContourBlurAlpha(model, -1.0f, -1.0f);
            } else {
                PrSetContourBlurAlpha(model, tmp_dat[0], tmp_dat[1]);
            }
            break;
        case BLMV_MOVE:
            tmp_dat = bra_ret_GetNext(model);
            if (tmp_dat == NULL) {
                PrResetPosture(model);
                PrSetTransactionBlendRatio(model, -1.0f);
            } else {
                PrSetTransactionBlendRatio(model, tmp_dat[0]);
            }
            break;
        }
    }

    PrAnimateModel(model, time + 0.0f);
    PrShowModel(model, NULL);
}

void XAnimationPositionLink(PR_MODELHANDLE model, PR_ANIMATIONHANDLE animation, float time) {
    PrLinkPositionAnimation(model, animation);
    PrAnimateModelPosition(model, time + 0.0f);
}

void XAnimationPositionUnLink(PR_MODELHANDLE model, PR_ANIMATIONHANDLE animation) {
    if (PrGetLinkedPositionAnimation(model) == animation) {
        PrUnlinkPositionAnimation(model);
    }
}

void BallThrowSetFrame(int frame) {
    bthrow_ctrl_time = frame;
}

void BallThrowInit(void) {
    WorkClear(bthrow_ctrl, sizeof(bthrow_ctrl));

    bthrow_ctrl[0].targetY = 112.0f;
    bthrow_ctrl[0].targetX = 120.0f;

    bthrow_ctrl[1].targetX = 520.0f;
    bthrow_ctrl[1].targetY = 112.0f;
}

void BallThrowInitDare(int dare) {
    WorkClear(&bthrow_ctrl[dare], sizeof(BTHROW_CTRL));
}

static void* vs06BomAdr(OBJBTHROW_TYPE thtype, int time) {
    u_short bomdat_tea[12] = {
        0x178, 0x177, 0x176, 0x175,
        0x174, 0x173, 0x172, 0x171,
        0x170, 0x16f, 0x16e, 0x16d,
    };
    u_short bomdat_pa[12] = {
        0x184, 0x183, 0x182, 0x181,
        0x180, 0x17f, 0x17e, 0x17d,
        0x17c, 0x17b, 0x17a, 0x179,
    };
    u_short *bomdat;

    if (global_data.play_step != PSTEP_VS || time >= 12) {
        return NULL;
    }

    if (thtype == OBJBTHROW_PARAPPA) {
        bomdat = bomdat_pa;
    } else {
        bomdat = bomdat_tea;
    }

    return GetIntAdrsCurrent(bomdat[time]);
}

void BallThrowReq(void *mdlh, OBJBTHROW_TYPE thtype, void *texpp, void *mdlhoming) {
    BTHROW_STR *bt_pp;

    if (bthrow_ctrl[thtype].bthrow_str_cnt >= PR_ARRAYSIZE(bthrow_ctrl[thtype].bthrow_str)) {
        return;
    }

    bt_pp = &bthrow_ctrl[thtype].bthrow_str[bthrow_ctrl[thtype].bthrow_str_cnt];

    bt_pp->use = 3;
    bt_pp->mdl_adr = mdlh;
    bt_pp->tim2_dat_pp = texpp;
    bt_pp->homingpp = mdlhoming;

    if (global_data.play_step == PSTEP_VS) {
        bt_pp->endTime = 32;
        if (bt_pp->endTime > bthrow_ctrl_time) {
            bt_pp->endTime = bthrow_ctrl_time;
        }
    } else {
        bt_pp->endTime = bthrow_ctrl_time;
    }

    bthrow_ctrl[thtype].frame = bthrow_ctrl_time;
    bthrow_ctrl[thtype].bthrow_str_cnt++;
}

void BallThrowTarget(void *mdlh, OBJBTHROW_TYPE thtype, int targetframe) {
    bthrow_ctrl[thtype].targ_mdl_adr = mdlh;
}

void BallThrowPoll(void) {
    int          i, j;
    sceGifPacket gifP;
    TIM2INFO     info;
    int          w, h;
    int          px, py;

    for (i = 0; i < PR_ARRAYSIZE(bthrow_ctrl); i++) {
        if (bthrow_ctrl[i].frame == 0) {
            BallThrowInitDare(i);
        } else {
            if (bthrow_ctrl[i].targ_mdl_adr != NULL) {
                float *pos_pp = PrGetModelScreenPosition(bthrow_ctrl[i].targ_mdl_adr);

                bthrow_ctrl[i].targetX = pos_pp[0];
                bthrow_ctrl[i].targetY = pos_pp[1] + -16.0f;
                bthrow_ctrl[i].targ_mdl_adr = NULL;
            }

            if (bthrow_ctrl[i].bthrow_str_cnt != 0) {
                BTHROW_STR *bts_pp = bthrow_ctrl[i].bthrow_str;

                CmnGifADPacketMake(&gifP, DrawGetFrameP(DNUM_DRAW));
                sceGifPkAddGsAD(&gifP, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 0x80));
                sceGifPkAddGsAD(&gifP, SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0));
                sceGifPkAddGsAD(&gifP, SCE_GS_TEST_1, SCE_GS_SET_TEST_1(1, 6, 0, 0, 0, 0, 1, 1));

                for (j = 0; j < bthrow_ctrl[i].bthrow_str_cnt; j++, bts_pp++) {
                    if (bts_pp->use & 1) {
                        float *pos_pp = PrGetModelScreenPosition(bts_pp->homingpp);

                        bthrow_ctrl[i].targetX = pos_pp[0];
                        bthrow_ctrl[i].targetY = pos_pp[1] + -16.0f;

                        if (bts_pp->use & 2) {
                            float *pos_pp;

                            bts_pp->use &= ~2;

                            pos_pp = PrGetModelScreenPosition(bts_pp->mdl_adr);

                            bts_pp->xp = pos_pp[0];
                            bts_pp->yp = pos_pp[1] + -16.0f;
                            bts_pp->mdl_adr = NULL;

                            if (global_data.play_step == PSTEP_VS && bts_pp->endTime < 12) {
                                bts_pp->xp = bthrow_ctrl[i].targetX;
                                bts_pp->yp = bthrow_ctrl[i].targetY;
                            }
                        } else if (global_data.play_step == PSTEP_VS) {
                            if (bts_pp->endTime < 12) {
                                if (bts_pp->endTime == 0) {
                                    bts_pp->use = 0;
                                }
                                if (bts_pp->endTime == 11) {
                                    bts_pp->xp = bthrow_ctrl[i].targetX;
                                    bts_pp->yp = bthrow_ctrl[i].targetY;
                                }

                                bts_pp->tim2_dat_pp = vs06BomAdr(i, bts_pp->endTime);
                            } else if (bts_pp->endTime != 12) {
                                bts_pp->xp += (bthrow_ctrl[i].targetX - bts_pp->xp) / (bts_pp->endTime - 12);
                                bts_pp->yp += (bthrow_ctrl[i].targetY - bts_pp->yp) / (bts_pp->endTime - 12);
                            } else {
                                bts_pp->xp = bthrow_ctrl[i].targetX;
                                bts_pp->yp = bthrow_ctrl[i].targetY;
                            }
                        } else if (bts_pp->endTime == 0) {
                            bts_pp->use = 0;
                            bts_pp->xp = bthrow_ctrl[i].targetX;
                            bts_pp->yp = bthrow_ctrl[i].targetY;
                        } else  {
                            bts_pp->xp += (bthrow_ctrl[i].targetX - bts_pp->xp) / bts_pp->endTime;
                            bts_pp->yp += (bthrow_ctrl[i].targetY - bts_pp->yp) / bts_pp->endTime;
                        }

                        GetTim2Info(bts_pp->tim2_dat_pp, &info, 1);

                        w = info.picturH->ImageWidth * 16;
                        h = info.picturH->ImageHeight * 16;

                        sceGifPkAddGsAD(&gifP, SCE_GS_TEX0_1, info.picturH->GsTex0);
                        sceGifPkAddGsAD(&gifP, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0));

                        px = ((int)(bts_pp->xp * 16.0f) - (w) + GS_X_COORD(0));
                        py = ((int)(bts_pp->yp * 16.0f) - (h / 2) + GS_Y_COORD(0));

                        sceGifPkAddGsAD(&gifP, SCE_GS_UV, SCE_GS_SET_UV(0, 0));

                        px &= 0xffff;
                        py &= 0xffff;

                        sceGifPkAddGsAD(&gifP, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(px, py, 0));

                        px += w * 2;
                        py += h;

                        sceGifPkAddGsAD(&gifP, SCE_GS_UV, SCE_GS_SET_UV(w, h));
                        sceGifPkAddGsAD(&gifP, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(px, py, 0));

                        if (bts_pp->use != 0) {
                            bts_pp->endTime--;
                        }
                    }
                }

                CmnGifADPacketMakeTrans(&gifP);
                bthrow_ctrl[i].frame--;
            }
        }
    }
}

int BallThrowPollScene(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    ChangeDrawArea(DrawGetDrawEnvP(drDisp));
    BallThrowPoll();
    return 0;
}

static void MozaikuPollSceneInit(void) {
    mozaiku_poll_str_current_pp = NULL;
}

static void MozaikuPollSceneReq(int id) {
    if (id >= PR_ARRAYSIZEU(mozaiku_poll_str)) {
        return;
    }

    mozaiku_poll_str_current_pp = &mozaiku_poll_str[id];
    mozaiku_poll_str_current_pp->mozaiku_str_frame = 0;
}

static int MozaikuPollScene(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    if (mozaiku_poll_str_current_pp == NULL) {
        return 0;
    }

    if (mozaiku_poll_str_current_pp->mozaiku_str_frame >= mozaiku_poll_str_current_pp->mozaiku_str_cnt) {
        mozaiku_poll_str_current_pp->mozaiku_str_frame = 0;
        mozaiku_poll_str_current_pp = NULL;
        return 0;
    } else {
        ChangeDrawArea(DrawGetDrawEnvP(drDisp));

        DrawMozaikuDisp(&mozaiku_poll_str_current_pp->mozaiku_str_pp[mozaiku_poll_str_current_pp->mozaiku_str_frame], frame, first_f, useDisp, drDisp);
        mozaiku_poll_str_current_pp->mozaiku_str_frame++;
        return 0;
    }
}

void DrawTmpBuffInit(void) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(tmp_buf_adrs); i++) {
        tmp_buf_adrs[i] = NULL;
    }
}

void DrawTmpBuffQuit(void *adrs) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(tmp_buf_adrs); i++) {
        if (tmp_buf_adrs[i] != NULL) {
            if (adrs == NULL || adrs == tmp_buf_adrs[i]) {
                usrFree(tmp_buf_adrs[i]);
                tmp_buf_adrs[i] = NULL;
            }
        }
    }
}

void* DrawTmpBuffGetArea(int size) {
    int   i;
    void *ret = NULL;

    for (i = 0; i < PR_ARRAYSIZE(tmp_buf_adrs); i++) {
        if (tmp_buf_adrs[i] == NULL) {
            ret = usrMalloc(size);
            tmp_buf_adrs[i] = ret;
            break;
        }
    }

    return ret;
}

void DrawObjdatInit(int size, OBJDAT *od_pp, PR_SCENEHANDLE prf) {
    int    i;
    void  *tmp_adr;
    float  frame;

    for (i = 0; i < size; i++, od_pp++) {
        tmp_adr = GetIntAdrsCurrent(od_pp->objdat_num);

        switch (od_pp->objdat_type) {
        case ODAT_SPA:
            od_pp->handle = PrInitializeAnimation(tmp_adr);
           
            frame = PrGetAnimationEndFrame(od_pp->handle);
            od_pp->maxfr = frame + frame;
            break;
        case ODAT_SPM:
            od_pp->handle = PrInitializeModel(tmp_adr, prf);
            break;
        case ODAT_SPC:
            od_pp->handle = PrInitializeCamera(tmp_adr);

            frame = PrGetCameraEndFrame(od_pp->handle);
            od_pp->maxfr = frame + frame;
            break;
        case ODAT_SPF:
        case ODAT_TM2:
        case ODAT_CL2:
            od_pp->handle = tmp_adr;
            break;
        default:
            printf("Objdat Error!! type[%d]\n", od_pp->objdat_type);
            break;
        }
    }
}

// FIXME: Is this necessary?
const int drawctrl_rodata_padding[] = { 0, 0 };

void DrawObjdatReset(int size, OBJDAT *od_pp) {
    int   i;
    void *tmp_adr;

    for (i = 0; i < size; i++, od_pp++) {
        tmp_adr = GetIntAdrsCurrent(od_pp->objdat_num);

        switch (od_pp->objdat_type) {
        case ODAT_SPA:
            break;
        case ODAT_SPM:
            PrResetPosture(od_pp->handle);
            PrResetContour(od_pp->handle);
            PrHideModel(od_pp->handle);

            if (PrGetLinkedAnimation(od_pp->handle) != NULL) {
                PrAnimateModel(od_pp->handle, 0.0f);
                PrUnlinkAnimation(od_pp->handle);
            }
            if (PrGetLinkedPositionAnimation(od_pp->handle) != NULL) {
                PrUnlinkPositionAnimation(od_pp->handle);
                PrAnimateModelPosition(od_pp->handle, 0.0f);
            }
            break;
        case ODAT_SPC:
        case ODAT_SPF:
        case ODAT_TM2:
        case ODAT_CL2:
            break;
        default:
            printf("Objdat Error!! type[%d]\n", od_pp->objdat_type);
            break;
        }
    }
}

void DrawSceneInit(sceGsDrawEnv1 *draw_env, SCENE_OBJDATA *scene_pp, int useDisp) {
    int   i;
    u_int fbp_tmp;

    if (scene_pp->handle != NULL) {
        return;
    }

    if (useDisp != DNUM_NON) {
        fbp_tmp = DrawGetFbpPos(useDisp);
    } else {
        fbp_tmp = -1;
    }

    scene_pp->handle = PrInitializeScene(draw_env, scene_pp->usrName, fbp_tmp);
    DrawObjdatInit(scene_pp->objdat_size, scene_pp->objdat_pp, scene_pp->handle);

    for (i = 0; i < PR_ARRAYSIZE(scene_pp->objactprg_ctrl.objactprg); i++) {
        scene_pp->objactprg_ctrl.objactprg[i] = NULL;
    }

    scene_pp->objactprg_ctrl.num = 0;
    PrPreprocessSceneModel(scene_pp->handle);
}

static void DrawObjCtrlInit(OBJCTRL *oc_pp) {
    oc_pp->PRflag = 0;
    oc_pp->PRdata = 0;
    oc_pp->PRtime = 0;
}

static void DrawObjStrInit(OBJSTR *os_pp) {
    int i;

    for (i = 0; i < os_pp->size; i++) {
        DrawObjCtrlInit(&os_pp->objctrl_pp[i]);
    }

    os_pp->PRflag     = 0;
    os_pp->PRdata     = 0;
    os_pp->PRtime     = 0;
    os_pp->PRpress    = NULL;
    os_pp->PRtimeOld  = -1;

    os_pp->current_pp = NULL;
    os_pp->loop_time  = 0;
    os_pp->loop_pp    = NULL;
}

static void DrawObjTapStrInit(OBJTAPSTR *objtapstr_pp) {
    objtapstr_pp->ovl_cnt_num = 0;
}

static void DrawObjStrReq(SCENE_OBJDATA *scn_pp, int num, u_int time) {
    if (scn_pp->objstr_size <= num) {
        return;
    }

    scn_pp->objstr_pp[num].PRflag    = OBJSTR_REQ;
    scn_pp->objstr_pp[num].PRtime    = time;
    scn_pp->objstr_pp[num].PRtimeOld = -1;
}

static void DrawObjStrTapReq(SCENE_OBJDATA *scn_pp, int num, u_int time, u_char *prs_adr) {
    if (scn_pp->tapstr_size <= num) {
        return;
    }

    scn_pp->tapstr_pp[num].PRflag    = OBJSTR_REQ;
    scn_pp->tapstr_pp[num].PRtime    = 0;
    scn_pp->tapstr_pp[num].PRpress   = prs_adr;
    scn_pp->tapstr_pp[num].PRtimeOld = -1;
}

static void DrawObjTapStrTapReq(SCENE_OBJDATA *scn_pp, int req_num, u_int time, u_char *prs_adr) {
    int     max;
    OBJTAP *objtap_pp, *objtap_tmp_pp, *objtap_end_pp_ck;
    int     loop_time_cnt, loop_ari_flag;
    int     i;

    loop_time_cnt = 0;
    loop_ari_flag = FALSE;

    max = scn_pp->objtapstr_pp[req_num].objtap_size;
    if (max == 0) {
        return;
    }

    time = ++scn_pp->objtapstr_pp[req_num].ovl_cnt_num;

    objtap_pp = scn_pp->objtapstr_pp[req_num].objtap_pp;
    objtap_end_pp_ck = objtap_pp + max;
    objtap_tmp_pp = objtap_pp;

    for (i = 0; i < max; i++) {
        if (objtap_pp[i].channel == OTE_LOOP_P || objtap_pp[i].channel == OTE_LOOP || objtap_pp[i].channel == OTE_NEXT) {
            loop_ari_flag = TRUE;
        }
    }

    if (loop_ari_flag) {
        while (1) {
            if (time == loop_time_cnt) {
                objtap_pp = objtap_tmp_pp;
                break;
            }
            if (objtap_tmp_pp >= objtap_end_pp_ck) {
                break;
            }

            switch (objtap_tmp_pp->channel) {
            case OTE_LOOP_P:
                objtap_pp = objtap_tmp_pp;
                break;
            case OTE_NEXT:
                loop_time_cnt++;
                break;
            case OTE_LOOP:
                loop_time_cnt++;
                objtap_tmp_pp = objtap_pp;
                break;
            }

            objtap_tmp_pp++;
        }
    }

    for (; objtap_pp < objtap_end_pp_ck; objtap_pp++) {
        if (objtap_pp->channel == OTE_LOOP) {
            break;
        }
        if (objtap_pp->channel == OTE_NEXT) {
            break;
        }
        if (objtap_pp->channel == OTE_LOOP_P) {
            break;
        }

        if (scn_pp->dr_tap_req[objtap_pp->channel].req_no >= 0) {
            if (scn_pp->dr_tap_req[objtap_pp->channel].req_no != objtap_pp->obj_num) {
                DrawObjStrTapQuit(scn_pp, scn_pp->dr_tap_req[objtap_pp->channel].tap_id, 0);
                scn_pp->dr_tap_req[objtap_pp->channel].req_no = -1;
            }
        }

        DrawObjStrTapReq(scn_pp, objtap_pp->obj_num, 0, prs_adr);

        scn_pp->dr_tap_req[objtap_pp->channel].tap_id = objtap_pp->obj_num;
        scn_pp->dr_tap_req[objtap_pp->channel].req_no = objtap_pp->obj_num;
        scn_pp->dr_tap_req[objtap_pp->channel].player_index = objtap_pp->channel;
        scn_pp->dr_tap_req[objtap_pp->channel].pad_prs_pp = prs_adr;
    }
}

void DrawObjStrTapQuit(SCENE_OBJDATA *scn_pp, int num, u_int time) {
    if (num < 0 || scn_pp->tapstr_size <= num) {
        return;
    }

    scn_pp->tapstr_pp[num].PRflag    = 0;
    scn_pp->tapstr_pp[num].PRtime    = 0;
    scn_pp->tapstr_pp[num].PRpress   = NULL;
    scn_pp->tapstr_pp[num].PRtimeOld = -1;
}

void DrawObjTapStrTapQuit(SCENE_OBJDATA *scn_pp, int req_num) {
    int     i;

    int     max       = scn_pp->objtapstr_pp[req_num].objtap_size;
    OBJTAP *objtap_pp = scn_pp->objtapstr_pp[req_num].objtap_pp;

    for (i = 0; i < max; i++, objtap_pp++) {
        if (objtap_pp->channel != OTE_LOOP_P && objtap_pp->channel != OTE_NEXT && objtap_pp->channel != OTE_LOOP) {
            DrawObjStrTapQuit(scn_pp, scn_pp->dr_tap_req[objtap_pp->channel].req_no, 0);
            scn_pp->dr_tap_req[objtap_pp->channel].req_no = -1;
        }
    }
}

static void DrawObjStrKill(SCENE_OBJDATA *scn_pp, int num) {
    if (scn_pp->objstr_size <= num) {
        return;
    }

    scn_pp->objstr_pp[num].PRflag    = 0;
    scn_pp->objstr_pp[num].PRtime    = 0;
    scn_pp->objstr_pp[num].PRpress   = NULL;
    scn_pp->objstr_pp[num].PRtimeOld = -1;
}

static u_int GetSpfTimeCtrl(OBJDAT *objdat_pp, u_int frame) {
    SPF_STR *spf_str_pp;
    u_int    max_cnt;
    u_int    ret;

    spf_str_pp = objdat_pp->handle;
    max_cnt = (spf_str_pp->maxFrame * 2) - 2;
    if (frame > max_cnt) {
        frame = max_cnt;
    }

    if (frame & 0x1) {
        ret = (spf_str_pp->frame[frame / 2] + spf_str_pp->frame[(frame / 2) + 1]) / 2;
    } else {
        ret = spf_str_pp->frame[frame / 2];
    }

    return ret;
}

void camOtherKill(OBJACTPRG *objactprg_pp, int objactprg_num, int oya_num) {
    int i;

    for (i = 0; i < objactprg_num; i++, objactprg_pp++) {
        if (i != oya_num) {
            if (objactprg_pp->job_type == OCTRL_CAM) {
                objactprg_pp->job_type = OCTRL_NON;
            }
        }
    }
}

void posAniOtherKill(OBJACTPRG *objactprg_pp, int objactprg_num, int ani_num, int mod_num) {
    int i;

    for (i = 0; i < objactprg_num; i++, objactprg_pp++) {
        if (i != ani_num) {
            if (objactprg_pp->job_type == OCTRL_ANIPOS && objactprg_pp->sub_num == mod_num) {
                objactprg_pp->job_type = OCTRL_NON;
            }
        }
    }
}

static int DrawObjStrDisp(SCENE_OBJDATA *scn_pp, int num, u_int time, int sw) {
    OBJSTR    *objstr_pp;
    OBJCTRL   *objctrl_pp, *objctrl_end_pp;
    OBJACTPRG *objactprg_pp;
    OBJACTPRG *objactprg_tmp_pp;
    int        i, first_f, tmp_time, endflag, ret;

    first_f = FALSE;
    endflag = FALSE;
    ret = FALSE;

    if (sw) {
        objstr_pp = &scn_pp->tapstr_pp[num];
        objactprg_pp = scn_pp->objactprg_ctrl.objactprg[OBJACTPRG_TAP];
    } else {
        objstr_pp = &scn_pp->objstr_pp[num];
        objactprg_pp = scn_pp->objactprg_ctrl.objactprg[OBJACTPRG_NORMAL];
    }

    if (objstr_pp->PRflag == 0) {
        return FALSE;
    }

    if (objstr_pp->PRflag & OBJSTR_REQ) {
        for (i = 0; i < objstr_pp->size; i++) {
            DrawObjCtrlInit(&objstr_pp->objctrl_pp[i]);
        }

        objstr_pp->PRflag &= ~OBJSTR_REQ;
        objstr_pp->PRflag |= OBJSTR_ON;

        first_f = TRUE;

        objstr_pp->PRdata = 0;
        objstr_pp->current_pp = objstr_pp->objctrl_pp;
        objstr_pp->loop_time = 0;
        objstr_pp->loop_pp = NULL;
    }

    if (time <= objstr_pp->PRtime) {
        time = 0;
    } else {
        time -= objstr_pp->PRtime;
    }

    tmp_time = time;

    objctrl_pp = objstr_pp->objctrl_pp;
    for (i = 0; i < objstr_pp->size; i++, objctrl_pp++) {
        if (objctrl_pp->frame <= time && objctrl_pp->objctrl_type == OCTRL_TCTRL) {
            tmp_time = GetSpfTimeCtrl(&scn_pp->objdat_pp[objctrl_pp->dat[0]], time - objctrl_pp->frame);
        }
    }

    objctrl_end_pp = objstr_pp->objctrl_pp + objstr_pp->size;
    objctrl_pp = objstr_pp->current_pp;

    if (num == 0) {
        for (i = 0; i < scn_pp->objactprg_ctrl.num; i++) {
            objactprg_pp[i].job_type = OCTRL_NON;
        }
    }

    while (objctrl_pp < objctrl_end_pp) {
        if (objctrl_pp->frame <= tmp_time) {
            switch (objctrl_pp->objctrl_type) {
            case OCTRL_ANI:
            case OCTRL_MDL:
            case OCTRL_CAM:
            case OCTRL_TM2:
                objactprg_tmp_pp = &objactprg_pp[objctrl_pp->dat[0]];
                objactprg_tmp_pp->main_num = objctrl_pp->dat[0];
                objactprg_tmp_pp->sub_num = objctrl_pp->dat[1];

                if (objctrl_pp->status & OCTRL_STAT_DISABLED) {
                    objactprg_tmp_pp->job_type = OCTRL_NON;
                    break;
                }

                objactprg_tmp_pp->now_time = tmp_time - objctrl_pp->frame;

                objactprg_tmp_pp->first_flag = 0;
                if (objactprg_tmp_pp->now_time == 0) {
                    objactprg_tmp_pp->first_flag = 1;
                }

                if (objctrl_pp->status & OCTRL_STAT_SAVE_TIME) {
                    octst_time[objctrl_pp->dat[4]] = objactprg_tmp_pp->now_time;
                }
                if (objctrl_pp->status & OCTRL_STAT_LOAD_TIME) {
                    if (first_f) {
                        octst_timeLoad[objctrl_pp->dat[4]] = octst_time[objctrl_pp->dat[4]];
                    }
                    objactprg_tmp_pp->now_time = octst_timeLoad[objctrl_pp->dat[4]];
                }

                objactprg_tmp_pp->job_type = objctrl_pp->objctrl_type;
                objactprg_tmp_pp->status = objctrl_pp->status;

                objactprg_tmp_pp->focal_lng = scn_pp->objdat_pp[objctrl_pp->dat[0]].subdat[0];
                objactprg_tmp_pp->defocus_lng = scn_pp->objdat_pp[objctrl_pp->dat[0]].subdat[1];

                if (objctrl_pp->objctrl_type == OCTRL_ANI) {
                    objactprg_tmp_pp->focal_lng = objctrl_pp->subDat;
                }

                objactprg_tmp_pp->start_time = 0;
                objactprg_tmp_pp->end_time = 0;

                if (objctrl_pp->objctrl_type != OCTRL_ANI && objctrl_pp->objctrl_type != OCTRL_CAM) {
                    break;
                }

                if (objctrl_pp->objctrl_type == OCTRL_CAM) {
                    objactprg_tmp_pp->end_time = scn_pp->objdat_pp[objctrl_pp->dat[0]].maxfr;
                } else {
                    objactprg_tmp_pp->end_time = scn_pp->objdat_pp[objctrl_pp->dat[1]].maxfr;
                }

                if (objctrl_pp->status & OCTRL_STAT_START_OVERRIDE) {
                    objactprg_tmp_pp->start_time = objctrl_pp->dat[2];
                }
                if (objctrl_pp->status & OCTRL_STAT_END_OVERRIDE) {
                    objactprg_tmp_pp->end_time = objctrl_pp->dat[3];
                }
                if (objctrl_pp->status & OCTRL_STAT_START_OVERRIDE) {
                    objactprg_tmp_pp->start_time = objctrl_pp->dat[2];
                }
                if (objctrl_pp->status & OCTRL_STAT_END_OVERRIDE) {
                    objactprg_tmp_pp->end_time = objctrl_pp->dat[3];
                }

                if (objctrl_pp->status & OCTRL_STAT_LOOP) {
                    objactprg_tmp_pp->now_time %= objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time;
                    if (objctrl_pp->status & OCTRL_STAT_SAVE_TIME) {
                        octst_time[objctrl_pp->dat[4]] = objactprg_tmp_pp->now_time;
                    }
                } else {
                    if (objactprg_tmp_pp->now_time > objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time) {
                        objactprg_tmp_pp->job_type = OCTRL_NON;
                    }
                }

                if (objctrl_pp->objctrl_type == OCTRL_CAM && objactprg_tmp_pp->job_type != OCTRL_NON) {
                    camOtherKill(objactprg_pp, scn_pp->objactprg_ctrl.num, objctrl_pp->dat[0]);
                }
                break;
            case OCTRL_ANIPOS:
                objactprg_tmp_pp = &objactprg_pp[objctrl_pp->dat[1]];
                objactprg_tmp_pp->main_num = objctrl_pp->dat[1];
                objactprg_tmp_pp->sub_num = objctrl_pp->dat[0];

                if (objctrl_pp->status & OCTRL_STAT_DISABLED) {
                    objactprg_tmp_pp->job_type = OCTRL_NON;
                    break;
                }

                objactprg_tmp_pp->now_time = tmp_time - objctrl_pp->frame;

                if (objctrl_pp->status & OCTRL_STAT_SAVE_TIME) {
                    octst_time[objctrl_pp->dat[4]] = objactprg_tmp_pp->now_time;
                }
                if (objctrl_pp->status & OCTRL_STAT_LOAD_TIME) {
                    if (first_f) {
                        octst_timeLoad[objctrl_pp->dat[4]] = octst_time[objctrl_pp->dat[4]];
                    }
                    objactprg_tmp_pp->now_time = octst_timeLoad[objctrl_pp->dat[4]];
                }

                objactprg_tmp_pp->job_type = objctrl_pp->objctrl_type;
                objactprg_tmp_pp->status = objctrl_pp->status;

                objactprg_tmp_pp->start_time = 0;
                objactprg_tmp_pp->end_time = 0;

                objactprg_tmp_pp->end_time = scn_pp->objdat_pp[objctrl_pp->dat[1]].maxfr;

                if (objctrl_pp->status & OCTRL_STAT_START_OVERRIDE) {
                    objactprg_tmp_pp->start_time = objctrl_pp->dat[2];
                }
                if (objctrl_pp->status & OCTRL_STAT_END_OVERRIDE) {
                    objactprg_tmp_pp->end_time = objctrl_pp->dat[3];
                }

                if (objctrl_pp->status & OCTRL_STAT_LOOP) {
                    objactprg_tmp_pp->now_time %= objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time;
                    if (objctrl_pp->status & OCTRL_STAT_SAVE_TIME) {
                        octst_time[objctrl_pp->dat[4]] = objactprg_tmp_pp->now_time;
                    }
                } else {
                    if (objactprg_tmp_pp->now_time > objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time) {
                        objactprg_tmp_pp->job_type = OCTRL_NON;
                    }
                }

                if (objactprg_tmp_pp->job_type != OCTRL_NON) {
                    posAniOtherKill(objactprg_pp, scn_pp->objactprg_ctrl.num, objctrl_pp->dat[1], objctrl_pp->dat[0]);
                }
                break;
            case OCTRL_CL2:
                objactprg_tmp_pp = &objactprg_pp[objctrl_pp->dat[0]];
                objactprg_tmp_pp->main_num = objctrl_pp->dat[0];
                objactprg_tmp_pp->sub_num = objctrl_pp->dat[1];

                if (objctrl_pp->status & OCTRL_STAT_DISABLED) {
                    objactprg_tmp_pp->job_type = OCTRL_NON;
                    break;
                }

                objactprg_tmp_pp->now_time = tmp_time - objctrl_pp->frame;
                objactprg_tmp_pp->job_type = objctrl_pp->objctrl_type;
                objactprg_tmp_pp->status = objctrl_pp->status;
                objactprg_tmp_pp->start_time = 0;
                objactprg_tmp_pp->end_time = objctrl_pp->dat[2];

                if (objactprg_tmp_pp->now_time > objactprg_tmp_pp->end_time) {
                    objactprg_tmp_pp->job_type = OCTRL_NON;
                }
                break;
            case OCTRL_BIBU:
                if (objctrl_pp->frame <= tmp_time) {
                    scn_pp->objactprg_ctrl.objactprg[OBJACTPRG_NORMAL][objctrl_pp->dat[0]].focal_lng = objctrl_pp->subDat;
                }
                break;
            case OCTRL_SUB:
                if (objctrl_pp->PRdata <= objstr_pp->PRdata) {
                    if (objctrl_pp->status & OCTRL_STAT_DISABLED) {
                        DrawObjStrKill(scn_pp, objctrl_pp->dat[0]);
                    } else {
                        DrawObjStrReq(scn_pp, objctrl_pp->dat[0], objctrl_pp->frame);
                    }
                    objctrl_pp->PRdata = objstr_pp->PRdata + 1;
                }
                break;
            case OCTRL_END:
                ret = TRUE;
                endflag = TRUE;
                break;
            case OCTRL_LOOP_P:
                objstr_pp->loop_pp = objctrl_pp;
                break;
            case OCTRL_NEXT:
                objstr_pp->PRdata++;
                break;
            case OCTRL_LOOP:
                tmp_time -= objctrl_pp->frame;
                if (tmp_time < 0) {
                    endflag = TRUE;
                }
                objstr_pp->PRdata++;
                objctrl_pp = objstr_pp->loop_pp;
                break;
            case OCTRL_EXIT:
                objstr_pp->PRflag = 0;
                endflag = TRUE;
                ret = TRUE;
                break;
            case OCTRL_TCTRL:
                break;
            }

            if (endflag) {
                break;
            }
        }

        objctrl_pp++;
    }

    objstr_pp->PRtimeOld = tmp_time;
    return ret;
}

static int DrawObjStrDispTap(SCENE_OBJDATA *scn_pp, int num) {
    OBJSTR    *objstr_pp;
    OBJCTRL   *objctrl_pp, *objctrl_end_pp;
    OBJACTPRG *objactprg_pp, *objactprg_org_pp;
    OBJACTPRG *objactprg_tmp_pp, *objactprg_tmp_org_pp;
    int        i, first_f, tmp_time, endflag, ret, check_pos;

    first_f = FALSE;
    endflag = FALSE;
    ret = FALSE;

    objstr_pp = &scn_pp->tapstr_pp[num];
    objactprg_pp = scn_pp->objactprg_ctrl.objactprg[OBJACTPRG_TAP];
    objactprg_org_pp = scn_pp->objactprg_ctrl.objactprg[OBJACTPRG_ORG];

    if (objstr_pp->PRflag == 0) {
        return FALSE;
    }

    if (objstr_pp->PRflag & OBJSTR_REQ) {
        if (objstr_pp->current_pp == NULL) {
            objstr_pp->current_pp = objstr_pp->objctrl_pp;
            objstr_pp->PRdata = 0;
            objstr_pp->loop_time = 0;
            objstr_pp->loop_pp = objstr_pp->objctrl_pp;
        }

        for (i = 0; i < objstr_pp->size; i++) {
            DrawObjCtrlInit(&objstr_pp->objctrl_pp[i]);
        }

        objstr_pp->PRdata++;

        objstr_pp->PRflag &= ~OBJSTR_REQ;
        objstr_pp->PRflag |= OBJSTR_ON;

        first_f = TRUE;

        objctrl_pp = objstr_pp->objctrl_pp;
        objctrl_end_pp = objctrl_pp + objstr_pp->size;

        check_pos = 1;
        while (1) {
            if (check_pos == objstr_pp->PRdata) {
                break;
            }
            if (objctrl_pp >= objctrl_end_pp) {
                objctrl_pp = objstr_pp->objctrl_pp;
                break;
            }

            switch (objctrl_pp->objctrl_type) {
            case OCTRL_LOOP_P:
                objstr_pp->loop_pp = objctrl_pp;
                break;
            case OCTRL_NEXT:
                check_pos++;
                break;
            case OCTRL_LOOP:
                check_pos++;
                objctrl_pp = objstr_pp->loop_pp;
                break;
            }

            objctrl_pp++;
        }

        objstr_pp->current_pp = objctrl_pp;
    } else {
        if (objstr_pp->PRpress != NULL) {
            if (*objstr_pp->PRpress > 100) {
                objstr_pp->PRflag |= OBJSTR_PRESSON;
                objstr_pp->PRtime++;
            }
            if (*objstr_pp->PRpress > 180) {
                objstr_pp->PRtime++;
            }
        }

        objstr_pp->PRtime++;
    }

    objctrl_pp = objstr_pp->current_pp;
    objctrl_end_pp = objstr_pp->objctrl_pp + objstr_pp->size;

    tmp_time = objstr_pp->PRtime;

    while (objctrl_pp < objctrl_end_pp) {
        switch (objctrl_pp->objctrl_type) {
        case OCTRL_ANI:
        case OCTRL_MDL:
        case OCTRL_CAM:
        case OCTRL_TM2:
            if (tmp_time < objctrl_pp->frame) {
                break;
            }

            objactprg_tmp_pp = &objactprg_pp[objctrl_pp->dat[0]];
            objactprg_tmp_org_pp = &objactprg_org_pp[objctrl_pp->dat[0]];
            
            objactprg_tmp_pp->main_num = objctrl_pp->dat[0];
            objactprg_tmp_pp->sub_num = objctrl_pp->dat[1];

            if (objctrl_pp->status & OCTRL_STAT_DISABLED) {
                objactprg_tmp_pp->job_type = OCTRL_NON;
                break;
            }

            objactprg_tmp_pp->now_time = tmp_time - objctrl_pp->frame;

            if (objctrl_pp->status & OCTRL_STAT_SAVE_TIME) {
                octst_time[objctrl_pp->dat[4]] = objactprg_tmp_pp->now_time;
            }
            if (objctrl_pp->status & OCTRL_STAT_LOAD_TIME) {
                if (first_f) {
                    octst_timeLoad[objctrl_pp->dat[4]] = octst_time[objctrl_pp->dat[4]];
                }
                objactprg_tmp_pp->now_time = octst_timeLoad[objctrl_pp->dat[4]];
            }

            objactprg_tmp_pp->job_type = objctrl_pp->objctrl_type;
            objactprg_tmp_pp->status = objctrl_pp->status;

            objactprg_tmp_pp->start_time = 0;
            objactprg_tmp_pp->end_time = 0;

            objactprg_tmp_pp->first_flag = first_f;

            objactprg_tmp_pp->focal_lng = scn_pp->objdat_pp[objctrl_pp->dat[0]].subdat[0];
            objactprg_tmp_pp->defocus_lng = scn_pp->objdat_pp[objctrl_pp->dat[0]].subdat[1];

            if (objctrl_pp->objctrl_type == OCTRL_ANI) {
                objactprg_tmp_pp->focal_lng = objctrl_pp->subDat;
                objactprg_tmp_pp->end_time = scn_pp->objdat_pp[objctrl_pp->dat[1]].maxfr;
            } else if (objctrl_pp->objctrl_type == OCTRL_CAM) {
                objactprg_tmp_pp->end_time = scn_pp->objdat_pp[objctrl_pp->dat[0]].maxfr;
            } else {
                break;
            }

            if (objctrl_pp->status & OCTRL_STAT_CONTINUE) {
                if (objactprg_tmp_pp->job_type == objactprg_tmp_org_pp->job_type) {
                    objactprg_tmp_pp->now_time = objactprg_tmp_org_pp->now_time + 1;
                    objctrl_pp->PRdata = 1;
                }
                if (objactprg_tmp_org_pp->job_type == OCTRL_NON && objctrl_pp->PRdata != 0) {
                    objctrl_pp->PRdata = 2;
                    objactprg_tmp_pp->job_type = OCTRL_NON;
                    break;
                }
            }

            if (objctrl_pp->status & OCTRL_STAT_START_OVERRIDE) {
                objactprg_tmp_pp->start_time = objctrl_pp->dat[2];
            }
            if (objctrl_pp->status & OCTRL_STAT_END_OVERRIDE) {
                objactprg_tmp_pp->end_time = objctrl_pp->dat[3];
            }

            if (objctrl_pp->status & OCTRL_STAT_LOOP) {
                int haba = objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time;
                objactprg_tmp_pp->now_time %= haba;
            } else {
                int haba = objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time;
                if (objactprg_tmp_pp->now_time > haba) {
                    objactprg_tmp_pp->job_type = OCTRL_NON;
                }
            }

            if (objctrl_pp->objctrl_type == OCTRL_CAM && objactprg_tmp_pp->job_type != OCTRL_NON) {
                camOtherKill(objactprg_pp, scn_pp->objactprg_ctrl.num, objctrl_pp->dat[0]);
            }

            if (objctrl_pp->objctrl_type == OCTRL_ANI) {
                if (objctrl_pp->dat[4] == 0) {
                    objstr_pp->PRpress = NULL;
                }

                if (objctrl_pp->dat[4] == 2) {
                    if (objstr_pp->PRpress != NULL) {
                        if (*objstr_pp->PRpress == 0) {
                            objstr_pp->PRpress = NULL;
                        }
                    }

                    if (!(objstr_pp->PRflag & OBJSTR_PRESSON)) {
                        objstr_pp->PRpress = NULL;
                    }

                    if (objstr_pp->PRpress == NULL) {
                        int haba = (objctrl_pp->frame + objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time);
                        if (tmp_time < haba) {
                            objstr_pp->PRtime = haba;
                            tmp_time = objstr_pp->PRtime;
                        }

                        objactprg_tmp_pp->job_type = OCTRL_NON;
                    }
                }
            }
            break;
        case OCTRL_ANIPOS:
            objactprg_tmp_pp = &objactprg_pp[objctrl_pp->dat[1]];
            objactprg_tmp_pp->main_num = objctrl_pp->dat[1];
            objactprg_tmp_pp->sub_num = objctrl_pp->dat[0];

            if (objctrl_pp->status & OCTRL_STAT_DISABLED) {
                objactprg_tmp_pp->job_type = OCTRL_NON;
                break;
            }

            objactprg_tmp_pp->now_time = tmp_time - objctrl_pp->frame;

            if (objctrl_pp->status & OCTRL_STAT_SAVE_TIME) {
                octst_time[objctrl_pp->dat[4]] = objactprg_tmp_pp->now_time;
            }
            if (objctrl_pp->status & OCTRL_STAT_LOAD_TIME) {
                if (first_f) {
                    octst_timeLoad[objctrl_pp->dat[4]] = octst_time[objctrl_pp->dat[4]];
                }
                objactprg_tmp_pp->now_time = octst_timeLoad[objctrl_pp->dat[4]];
            }

            objactprg_tmp_pp->job_type = objctrl_pp->objctrl_type;
            objactprg_tmp_pp->status = objctrl_pp->status;

            objactprg_tmp_pp->start_time = 0;
            objactprg_tmp_pp->end_time = 0;

            objactprg_tmp_pp->end_time = scn_pp->objdat_pp[objctrl_pp->dat[1]].maxfr;

            if (objctrl_pp->status & OCTRL_STAT_START_OVERRIDE) {
                objactprg_tmp_pp->start_time = objctrl_pp->dat[2];
            }
            if (objctrl_pp->status & OCTRL_STAT_END_OVERRIDE) {
                objactprg_tmp_pp->end_time = objctrl_pp->dat[3];
            }
            if (objctrl_pp->status & OCTRL_STAT_LOOP) {
                int haba = objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time;
                objactprg_tmp_pp->now_time %= haba;
            } else {
                int haba = objactprg_tmp_pp->end_time - objactprg_tmp_pp->start_time;
                if (objactprg_tmp_pp->now_time > haba) {
                    objactprg_tmp_pp->job_type = OCTRL_NON;
                }
            }

            if (objactprg_tmp_pp->job_type != OCTRL_NON) {
                posAniOtherKill(objactprg_pp, scn_pp->objactprg_ctrl.num, objctrl_pp->dat[1], objctrl_pp->dat[0]);
            }
            break;
        case OCTRL_CL2: {
            int tmpX;
            objactprg_tmp_pp = &objactprg_pp[objctrl_pp->dat[0]];
            objactprg_tmp_pp->main_num = objctrl_pp->dat[0];
            objactprg_tmp_pp->sub_num = objctrl_pp->dat[1];

            if (objctrl_pp->status & OCTRL_STAT_DISABLED) {
                objactprg_tmp_pp->job_type = OCTRL_NON;
                break;
            }

            objactprg_tmp_pp->now_time = tmp_time - objctrl_pp->frame;


            objactprg_tmp_pp->job_type = objctrl_pp->objctrl_type;
            objactprg_tmp_pp->status = objctrl_pp->status;
            
            objactprg_tmp_pp->start_time = 0;
            tmpX = objctrl_pp->dat[2];
            objactprg_tmp_pp->end_time = tmpX;
            if (objactprg_tmp_pp->now_time > tmpX) {
                objactprg_tmp_pp->job_type = OCTRL_NON;
            }
            break;
        }
        case OCTRL_NEXT:
        case OCTRL_LOOP:
            endflag = TRUE;
            break;
        case OCTRL_END:
        case OCTRL_EXIT:
            if (objctrl_pp->frame <= tmp_time) {
                objstr_pp->PRflag = 0;
                endflag = TRUE;
                ret = TRUE;
            }
            break;
        case OCTRL_BIBU:
            if (objctrl_pp->frame <= tmp_time) {
                scn_pp->objactprg_ctrl.objactprg[OBJACTPRG_NORMAL][objctrl_pp->dat[0]].focal_lng = objctrl_pp->subDat;
            }
            break;
        case OCTRL_BTHROW:
            if (first_f) {
                BallThrowReq(
                    scn_pp->objdat_pp[objctrl_pp->dat[0]].handle,
                    objctrl_pp->status,
                    scn_pp->objdat_pp[objctrl_pp->dat[4]].handle,
                    scn_pp->objdat_pp[objctrl_pp->dat[1]].handle
                );
            }
            break;
        case OCTRL_MOZAIKU:
            if (first_f) {
                MozaikuPollSceneReq(objctrl_pp->dat[0]);
            }
            break;
        case OCTRL_TCTRL:
        case OCTRL_SUB:
        case OCTRL_LOOP_P:
        case OCTRL_BHIT:
        case OCTRL_ANIPOSXX:
            break;
        }

        if (endflag) {
            break;
        }

        objctrl_pp++;
    }

    objstr_pp->PRtimeOld = tmp_time;
    return ret;
}


void DrawSceneReset(SCENE_OBJDATA *scene_pp) {
    int i;

    DrawObjdatReset(scene_pp->objdat_size, scene_pp->objdat_pp);

    for (i = 0; i < scene_pp->objstr_size; i++) {
        DrawObjStrInit(&scene_pp->objstr_pp[i]);
    }

    for (i = 0; i < scene_pp->tapstr_size; i++) {
        DrawObjStrInit(&scene_pp->tapstr_pp[i]);
    }

    for (i = 0; i < scene_pp->objtapstr_size; i++) {
        DrawObjTapStrInit(&scene_pp->objtapstr_pp[i]);
    }

    for (i = 0; i < PR_ARRAYSIZE(scene_pp->objactprg_ctrl.objactprg); i++) {
        if (scene_pp->objactprg_ctrl.objactprg[i] != NULL) {
            DrawTmpBuffQuit(scene_pp->objactprg_ctrl.objactprg[i]);
            scene_pp->objactprg_ctrl.objactprg[i] = NULL;
        }
    }

    for (i = 0; i < PR_ARRAYSIZE(scene_pp->dr_tap_req); i++) {
        scene_pp->dr_tap_req[i].req_no = -1;
    }
}

void DrawSceneFirstSet(SCENE_OBJDATA *scene_pp) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(scene_pp->objactprg_ctrl.objactprg); i++) {
        if (scene_pp->objactprg_ctrl.objactprg[i] == NULL) {
            scene_pp->objactprg_ctrl.objactprg[i] = DrawTmpBuffGetArea(scene_pp->objdat_size * sizeof(OBJACTPRG));
        }

        WorkClear(scene_pp->objactprg_ctrl.objactprg[i], scene_pp->objdat_size * sizeof(OBJACTPRG));
    }

    scene_pp->objactprg_ctrl.num = scene_pp->objdat_size;

    for (i = 0; i < PR_ARRAYSIZE(scene_pp->dr_tap_req); i++) {
        scene_pp->dr_tap_req[i].req_no = -1;
    }
}

void Cl2MixTrans(int now_T, int max_T, u_char *cl2_0_pp, u_char *cl2_1_pp) {
    TIM2INFO        tim2info0, tim2info1;
    u_char         *dat_pp;
    sceGsLoadImage  l_image;

    int             trSize0, trSize1;
    int             trType0, trType1;
    int             trW, trH;
    void           *trAdrs0, *trAdrs1;
    int             trTbp0, trTbp1;
    int             trTbw;

    if (!GetTim2Info(cl2_0_pp, &tim2info0, 1)) {
        printf("This data isn't TIM2 file\n");
        return;
    }

    if (!GetTim2Info(cl2_1_pp, &tim2info1, 1)) {
        printf("This data isn't TIM2 file\n");
        return;
    }

    if (now_T > max_T) {
        now_T = max_T;
    }

    if (tim2info0.fileH->FileId[0] != tim2info1.fileH->FileId[0]) {
        printf("file type is difference\n");
        return;
    }

    if (tim2info0.fileH->FileId[0] == 'T') {
        trSize0 = tim2info0.picturH->ImageSize;
        trSize1 = tim2info1.picturH->ImageSize;

        trType0 = tim2info0.picturH->ImageType;
        trType1 = tim2info1.picturH->ImageType;

        trW = tim2info1.picturH->ImageWidth;
        trH = tim2info1.picturH->ImageHeight;

        trAdrs0 = tim2info0.image_pp;
        trAdrs1 = tim2info1.image_pp;

        trTbp0 = ((sceGsTex0*)&tim2info0.picturH->GsTex0)->TBP0;
        trTbp1 = ((sceGsTex0*)&tim2info1.picturH->GsTex0)->TBP0;

        trTbw = ((sceGsTex0*)&tim2info0.picturH->GsTex0)->TBW;
    } else {
        trSize0 = tim2info0.picturH->ClutSize;
        trSize1 = tim2info1.picturH->ClutSize;

        trType0 = tim2info0.picturH->ClutType & 0x1f;
        trType1 = tim2info1.picturH->ClutType & 0x1f;

        if (tim2info1.picturH->ClutColors > 16) {
            trW = 16;
            trH = 16;
        } else {
            trW = 8;
            trH = 2;
        }
    
        trAdrs0 = tim2info0.clut_pp;
        trAdrs1 = tim2info1.clut_pp;

        trTbp0 = ((sceGsTex0*)&tim2info0.picturH->GsTex0)->CBP;
        trTbp1 = ((sceGsTex0*)&tim2info1.picturH->GsTex0)->CBP;

        trTbw = 1; /* 64px */
    }

    if (trSize0 != trSize1 || trType0 != trType1 || trTbp0 != trTbp1) {
        printf("Clut (Image) type is difference\n");
        return;
    }

    dat_pp = usrMalloc(trSize0);

    if (trType0 == TIM2_RGB16) {
        int      i;
        int      maxx, maxy;
        RGB15TR *rgb15tr0, *rgb15tr1;
        RGB15TR *tr_pp;

        maxx = (now_T * 256) / max_T;
        maxy = 256 - maxx;

        rgb15tr0 = trAdrs0;
        rgb15tr1 = trAdrs1;
        tr_pp = (RGB15TR*)dat_pp;

        trType0 = TIM2_RGB16;

        for (i = 0; i < (trSize0 / 2); i++) {
            tr_pp->r = ((maxx * rgb15tr0->r) + (maxy * rgb15tr1->r)) / 256;
            tr_pp->g = ((maxx * rgb15tr0->g) + (maxy * rgb15tr1->g)) / 256;
            tr_pp->b = ((maxx * rgb15tr0->b) + (maxy * rgb15tr1->b)) / 256;
            tr_pp->a = rgb15tr0->a;

            rgb15tr0++;
            rgb15tr1++;
            tr_pp++;
        }
    } else {
        int     i;
        u_char *tr_pp;
        u_char *col0, *col1;
        int     maxx, maxy;

        maxx = (now_T * 256) / max_T;
        maxy = 256 - maxx;

        col0 = trAdrs0;
        col1 = trAdrs1;

        tr_pp = dat_pp;

        for (i = 0; i < trSize0; i++) {
            *tr_pp = ((maxx * (*col0)) + (maxy * (*col1))) / 256;

            col0++, col1++;
            tr_pp++;
        }
    }

    PR_SCOPE()
    u_short typemode[6] = {
        SCE_GS_PSMCT32, /* TIM2_NONE   */ /* No CLUT (ClutType only) */
        SCE_GS_PSMCT16, /* TIM2_RGB16  */
        SCE_GS_PSMCT24, /* TIM2_RGB24  */
        SCE_GS_PSMCT32, /* TIM2_RGB32  */
        SCE_GS_PSMCT32, /* TIM2_IDTEX4 */
        SCE_GS_PSMCT32  /* TIM2_IDTEX8 */
    };

    sceGsSetDefLoadImage(&l_image, trTbp0, trTbw, typemode[trType0], 0, 0, trW, trH);
    FlushCache(WRITEBACK_DCACHE);

    sceGsExecLoadImage(&l_image, (u_long128*)dat_pp);
    sceGsSyncPath(0, 0);
    PR_SCOPEEND()

    usrFree(dat_pp);
}

void DrawObjPrReq(SCENE_OBJDATA *scene_pp) {
    OBJACTPRG      *cam_pp[OBJACTPRG_MAX];
    OBJACTPRG      *org_pp;
    OBJACTPRG      *prg_pp;
    int             i, j, set_f, chg_f;
    int             first, blumove;
    OBJACTPRG_CTRL *ctrl_pp = &scene_pp->objactprg_ctrl;

    for (i = OBJACTPRG_MAX - 1; i >= 0; i--) {
        cam_pp[i] = NULL;
    }

    org_pp = ctrl_pp->objactprg[OBJACTPRG_ORG];

    for (i = 0; i < ctrl_pp->num; i++) {
        set_f = FALSE;

        for (j = OBJACTPRG_TAP; j > OBJACTPRG_ORG; j--) {
            prg_pp = ctrl_pp->objactprg[j];

            if (prg_pp[i].job_type == OCTRL_NON) {
                continue;
            }
            if (prg_pp[i].status & OCTRL_STAT_DISABLED) {
                continue;
            }

            switch (prg_pp[i].job_type) {
            case OCTRL_ANI:
                blumove = 0;
                if (prg_pp[i].status & OCTRL_STAT_MOVE) {
                    blumove = 2;
                }
                if (prg_pp[i].status & OCTRL_STAT_BLUR) {
                    blumove = 1;
                }
                if (prg_pp[i].status & OCTRL_STAT_BLUR2) {
                    blumove = 3;
                }

                first = prg_pp[i].first_flag;
                if (org_pp[i].job_type == OCTRL_NON) {
                    first = 1;
                }

                PrSetModelDisturbance(scene_pp->objdat_pp[i].handle, prg_pp[i].focal_lng);
                XAnimationLinkOption(scene_pp->objdat_pp[i].handle, scene_pp->objdat_pp[prg_pp[i].sub_num].handle,
                                     first, blumove, prg_pp[i].now_time + prg_pp[i].start_time);
                set_f = TRUE;
                org_pp[i] = prg_pp[i];
                break;
            case OCTRL_ANIPOS:
                XAnimationPositionLink(scene_pp->objdat_pp[prg_pp[i].sub_num].handle, scene_pp->objdat_pp[i].handle,
                                       prg_pp[i].now_time + prg_pp[i].start_time);
                set_f = TRUE;
                break;
            case OCTRL_MDL:
                set_f = TRUE;
                if (org_pp[i].job_type == OCTRL_NON) {
                    PrShowModel(scene_pp->objdat_pp[i].handle, NULL);
                    org_pp[i] = prg_pp[i];
                }
                break;
            case OCTRL_CAM:
                cam_pp[j] = &prg_pp[i];
                break;
            case OCTRL_TM2:
                chg_f = TRUE;
                if (org_pp[i].job_type != OCTRL_NON) {
                    chg_f = FALSE;
                }
                if (org_pp[i].sub_num != prg_pp[i].sub_num) {
                    chg_f = TRUE;
                }
                if (chg_f) {
                    set_f = TRUE;
                    Tim2TransX(scene_pp->objdat_pp[i].handle, prg_pp[i].sub_num);
                    org_pp[i] = prg_pp[i];
                }
                break;
            case OCTRL_CL2:
                Cl2MixTrans(prg_pp[i].now_time, prg_pp[i].end_time, scene_pp->objdat_pp[i].handle,
                            scene_pp->objdat_pp[prg_pp[i].sub_num].handle);
                org_pp[i] = prg_pp[i];
                break;
            }

            if (set_f) {
                break;
            }
        }

        if (!set_f) {
            switch (org_pp[i].job_type) {
            case OCTRL_ANI:
                PrResetPosture(scene_pp->objdat_pp[i].handle);
                PrSetTransactionBlendRatio(scene_pp->objdat_pp[i].handle, -1.0f);
                PrResetContour(scene_pp->objdat_pp[i].handle);
                PrSetContourBlurAlpha(scene_pp->objdat_pp[i].handle, -1.0f, -1.0f);
                PrHideModel(scene_pp->objdat_pp[i].handle);
                break;
            case OCTRL_MDL:
                PrHideModel(scene_pp->objdat_pp[i].handle);
                break;
            case OCTRL_ANIPOS:
                XAnimationPositionUnLink(scene_pp->objdat_pp[org_pp[i].sub_num].handle, scene_pp->objdat_pp[i].handle);
                break;
            }

            org_pp[i].job_type = OCTRL_NON;
        }
    }

    for (j = OBJACTPRG_MAX - 1; j >= 0; j--) {
        if (cam_pp[j] != NULL) {
            PrSelectCamera(scene_pp->objdat_pp[cam_pp[j]->main_num].handle, scene_pp->handle);
            PrAnimateSceneCamera(scene_pp->handle, (float)(cam_pp[j]->now_time + cam_pp[j]->start_time) + 0.0f);
            PrSetDepthOfField(scene_pp->handle, cam_pp[j]->focal_lng, cam_pp[j]->defocus_lng);
            camOtherKill(org_pp, ctrl_pp->num, cam_pp[j]->main_num);
            org_pp[cam_pp[j]->main_num] = *cam_pp[j];
            break;
        }
    }
}

void DrawObjStrTapTimeNext(SCENE_OBJDATA *sod_pp) {
    int        i, max_num;
    OBJACTPRG *objactprg_pp;
    u_int      time_tmp;

    max_num      = sod_pp->objactprg_ctrl.num;
    objactprg_pp = sod_pp->objactprg_ctrl.objactprg[OBJACTPRG_TAP];

    for (i = 0; i < max_num; i++, objactprg_pp++) {
        objactprg_pp->first_flag = 0;

        if (objactprg_pp->job_type != OCTRL_NON) {
            objactprg_pp->now_time++;

            if (objactprg_pp->job_type == OCTRL_ANI || objactprg_pp->job_type == OCTRL_CAM) {
                time_tmp = (objactprg_pp->end_time - objactprg_pp->start_time) + 1;

                if (objactprg_pp->status & OCTRL_STAT_LOOP) {
                    objactprg_pp->now_time %= time_tmp;
                }

                if (objactprg_pp->now_time >= time_tmp) {
                    objactprg_pp->job_type = OCTRL_NON;
                }
            }
        }
    }
}

void DrawObjTapCtrl(SCENE_OBJDATA *sod_pp, DR_TAP_REQ *tap_pp, int tap_num) {
    int i;
    int max_num;

    max_num = sod_pp->objactprg_ctrl.num;

    WorkClear(sod_pp->objactprg_ctrl.objactprg[OBJACTPRG_TAP], max_num * sizeof(OBJACTPRG));

    for (i = 0; i < tap_num; i++) {
        if (tap_pp[i].tap_id == DR_TAP_ID_ALL || sod_pp->tap_id == tap_pp[i].tap_id) {
            if (tap_pp[i].req_no >= sod_pp->objtapstr_size) {
                printf(" TAP REQ Table over!! ID:%d num:%d\n", tap_pp[i].tap_id, tap_pp[i].req_no);
                continue;
            }

            DrawObjTapStrTapReq(sod_pp, tap_pp[i].req_no, 0, tap_pp[i].pad_prs_pp);
        }
    }

    for (i = 0; i < sod_pp->tapstr_size; i++) {
        DrawObjStrDispTap(sod_pp, i);
    }
}

int DrawSceneObjData(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    int            i;
    SCENE_OBJDATA *scn_pp = (SCENE_OBJDATA*)para_pp;

    if (first_f == DRPRGF_INIT) {
        DrawSceneInit(&DBufDc.draw01, scn_pp, useDisp);
        return 0;
    }

    if (first_f == DRPRGF_RESET) {
        DrawSceneReset(scn_pp);
        return 0;
    }

    if (first_f == DRPRGF_FIRST) {
        DrawSceneFirstSet(scn_pp);

        for (i = 0; i < scn_pp->objstr_size; i++) {
            DrawObjStrInit(&scn_pp->objstr_pp[i]);
        }

        for (i = 0; i < scn_pp->tapstr_size; i++) {
            DrawObjStrInit(&scn_pp->tapstr_pp[i]);
        }

        DrawObjStrReq(scn_pp, 0, 0);

        for (i = 0; i < scn_pp->objtapstr_size; i++) {
            DrawObjTapStrInit(&scn_pp->objtapstr_pp[i]);
        }
    }

    PrSetSceneEnv(scn_pp->handle, DrawGetDrawEnvP(drDisp));

    for (i = 0; i < scn_pp->objstr_size; i++) {
        DrawObjStrDisp(scn_pp, i, frame, FALSE);
    }

    DrawObjTapCtrl(scn_pp, dr_tap_req, dr_tap_req_num);
    DrawObjPrReq(scn_pp);

    PrPreprocessSceneModel(scn_pp->handle);

    /* Disable menderer if scene is named "nome_" */
    if (strncmp(scn_pp->usrName, "nome_", 5) != 0) {
        PrRender(scn_pp->handle);
        PrWaitRender();
    } else {
        float men_tmp = PrGetMendererRatio();
        PrSetMendererRatio(0.0f);
        PrRender(scn_pp->handle);
        PrWaitRender();
        PrSetMendererRatio(men_tmp);
    }

    return 0;
}

int DrawDoubleDispIn(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGifPacket gifpk;
    int          i;
    sceGsFrame  *use_pp;
    int          req_req;
    int          ng_f;
    float        current_ang;
    float        ck_pos;
    int          cnt_size;
    float        adj_pos;
    float        treat_pos;
    int          tmp_x, tmp_y, tmp_w, tmp_h;
    DOUBLE_PARA *dpara_pp = (DOUBLE_PARA*)para_pp; /* note: not in STABS. */

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    use_pp = DrawGetFrameP(useDisp);

    CmnGifADPacketMake(&gifpk, DrawGetFrameP(drDisp));
    req_req = 0;

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEX0_1, GS_FRAME_TEX0(use_pp, 0, SCE_GS_MODULATE));
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0));
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEST_1, SCE_GS_SET_TEST(SCE_GS_FALSE, SCE_GS_ALPHA_NEVER, 0, SCE_GS_AFAIL_KEEP, SCE_GS_FALSE, 0, SCE_GS_TRUE, SCE_GS_DEPTH_ALWAYS));

    sceGifPkAddGsAD(&gifpk, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0));

    current_ang = dpara_pp->next_time_ang * frame;
    ck_pos = dpara_pp->pos_start + dpara_pp->pos_add * frame;

    if (dpara_pp->type == DDSP_VMOVE_U || dpara_pp->type == DDSP_VMOVE_D) {
        cnt_size = SCREEN_WIDTH;
    } else {
        cnt_size = SCREEN_FIELD_HEIGHT;
    }

    for (i = 0; i < cnt_size; i++) {
        tmp_x = 0;
        treat_pos = ck_pos + sinf(current_ang) * dpara_pp->move_size;
        tmp_y = 0;
        tmp_w = 0;
        tmp_h = 0;
        ng_f = 0;

        switch (dpara_pp->type) {
        case DDSP_HMOVE_L:
            if (treat_pos < 1.0f) {
                ng_f = 1;
            } else {
                if (treat_pos > SCREEN_WIDTH) {
                    treat_pos = SCREEN_WIDTH;
                }
                tmp_x = 0;
                tmp_y = i;
                tmp_w = treat_pos;
                tmp_h = 1;
            }
            break;
        case DDSP_HMOVE_R:
            if (treat_pos >= SCREEN_WIDTH - 1) {
                ng_f = 1;
            } else {
                if (treat_pos < 0.0f) {
                    treat_pos = 0.0f;
                }
                adj_pos = SCREEN_WIDTH - treat_pos;
                tmp_y = i;
                tmp_x = treat_pos;
                tmp_w = adj_pos;
                tmp_h = 1;
            }
            break;
        case DDSP_VMOVE_U:
            if (treat_pos < 1.0f) {
                ng_f = 1;
            } else {
                if (treat_pos > SCREEN_FIELD_HEIGHT) {
                    treat_pos = SCREEN_FIELD_HEIGHT;
                }
                tmp_x = i;
                tmp_h = treat_pos;
                tmp_w = 1;
            }
            break;
        case DDSP_VMOVE_D:
            if (treat_pos > SCREEN_FIELD_HEIGHT - 1) {
                ng_f = 1;
            } else {
                if (treat_pos < 0.0f) {
                    treat_pos = 0.0f;
                }
                adj_pos = SCREEN_FIELD_HEIGHT - treat_pos;
                tmp_x = i;
                tmp_y = treat_pos;
                tmp_w = 1;
                tmp_h = adj_pos;
            }
            break;
        }

        if (!ng_f) {
            sceGifPkAddGsAD(&gifpk, SCE_GS_UV, SCE_GS_SET_UV(tmp_x << 4, tmp_y << 4));
            sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ(GS_X_COORD(tmp_x), GS_Y_COORD(tmp_y), 1));
            sceGifPkAddGsAD(&gifpk, SCE_GS_UV, SCE_GS_SET_UV((tmp_x + tmp_w) << 4, (tmp_y + tmp_h) << 4));
            sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ(GS_X_COORD((tmp_x + tmp_w)), GS_Y_COORD((tmp_y + tmp_h)), 1));
            req_req = 1;
        }

        current_ang += dpara_pp->next_line_ang;
    }

    if (req_req) {
        CmnGifADPacketMakeTrans(&gifpk);
    }

    return 0;
}

int DrawVramClear(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    VCLR_PARA    *vclr_para_pp = (VCLR_PARA*)para_pp; /* note: not in STABS. */
    sceGifPacket  gifpk;

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    UsrPrSetScene();

    CmnGifADPacketMake2(&gifpk, DrawGetFrameP(drDisp));

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEST_2, SCE_GS_SET_TEST(1, 0, 0, 1, 0, 0, 1, 1));
    sceGifPkAddGsAD(&gifpk, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(vclr_para_pp->r, vclr_para_pp->g, vclr_para_pp->b, vclr_para_pp->a, 0));
    sceGifPkAddGsAD(&gifpk, SCE_GS_SCISSOR_2, SCE_GS_SET_SCISSOR(0, SCREEN_WIDTH, 0, SCREEN_FIELD_HEIGHT));
    sceGifPkAddGsAD(&gifpk, SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(1));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYOFFSET_2, SCE_GS_SET_XYOFFSET(GS_X_COORD(0), GS_Y_COORD(0)));

    sceGifPkAddGsAD(&gifpk, SCE_GS_PRIM, SCE_GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 1, 0));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(0), GS_Y_COORD(0), 1));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(SCREEN_WIDTH), GS_Y_COORD(SCREEN_FIELD_HEIGHT), 1));

    CmnGifADPacketMakeTrans(&gifpk);

    ChangeDrawArea2(DrawGetDrawEnvP(DNUM_DRAW));

    return 0;
}

int DrawMoveDispIn(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGifPacket gifpk;
    sceGsFrame  *use_pp, *draw_pp; /* note: not in STABS. */
    int          xp, yp;
    int          i;
    int          stp, endp;
    int          lngT, lngN;
    MOVEIN_PARA *movein_pp = (MOVEIN_PARA*)para_pp; /* note: not in STABS. */

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    use_pp = DrawGetFrameP(useDisp);
    draw_pp = DrawGetFrameP(drDisp);

    CmnGifADPacketMake(&gifpk, draw_pp);

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEX0_1, GS_FRAME_TEX0(use_pp, 0, SCE_GS_MODULATE));
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0));
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEST_1, SCE_GS_SET_TEST(SCE_GS_FALSE, SCE_GS_ALPHA_NEVER, 0, SCE_GS_AFAIL_KEEP, SCE_GS_FALSE, 0, SCE_GS_TRUE, SCE_GS_DEPTH_ALWAYS));

    sceGifPkAddGsAD(&gifpk, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0));

    stp = 0;
    endp = -1;

    for (i = 0; i < movein_pp->movein_data_size; i++) {
        if (movein_pp->movein_data_pp[i].frame == frame) {
            stp = i;
            endp = i;
            break;
        }

        if (movein_pp->movein_data_pp[i].frame < frame) {
            stp = i;
        }

        if (movein_pp->movein_data_pp[i].frame > frame) {
            endp = i;
            break;
        }
    }

    if (endp == -1) {
        endp = stp;
    }

    lngT = movein_pp->movein_data_pp[endp].frame - movein_pp->movein_data_pp[stp].frame;
    lngN = frame - movein_pp->movein_data_pp[stp].frame;

    if (lngT == 0) {
        xp = movein_pp->movein_data_pp[stp].xp << 4;
        yp = movein_pp->movein_data_pp[stp].yp << 4;
    } else {
        xp = movein_pp->movein_data_pp[endp].xp - movein_pp->movein_data_pp[stp].xp;
        xp = (((xp * lngN) << 4) / lngT) + (movein_pp->movein_data_pp[stp].xp << 4);

        yp = movein_pp->movein_data_pp[endp].yp - movein_pp->movein_data_pp[stp].yp;
        yp = (((yp * lngN) << 4) / lngT) + (movein_pp->movein_data_pp[stp].yp << 4);
    }

    xp = (xp + GS_X_COORD(0));
    yp = (yp + GS_Y_COORD(0));
    xp &= 0xffff;
    yp &= 0xffff;

    sceGifPkAddGsAD(&gifpk, SCE_GS_UV, SCE_GS_SET_UV(movein_pp->u << 4, movein_pp->v << 4));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ(xp, yp, 1));

    sceGifPkAddGsAD(&gifpk, SCE_GS_UV, SCE_GS_SET_UV((movein_pp->u + movein_pp->w) << 4, (movein_pp->v + movein_pp->h) << 4));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ(xp + (movein_pp->w << 4), yp + (movein_pp->h << 4), 1));

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEXFLUSH, 0);

    CmnGifADPacketMakeTrans(&gifpk);
    return 0;
}

int DrawAlphaBlendDisp(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGsFrame   *use_pp;
    sceGifPacket  gifpk;

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    use_pp = DrawGetFrameP(useDisp);

    CmnGifADPacketMake(&gifpk, DrawGetFrameP(drDisp));
    UG_AlpDisp(para_pp, use_pp, &gifpk);
    CmnGifADPacketMakeTrans(&gifpk);

    return 0;
}

int DrawMozaikuDisp(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGsFrame          *use_pp;
    sceGsFrame          *draw_pp;
    sceGifPacket         gifpk;
    sceGsFrame           maskedFrame;

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    use_pp = DrawGetFrameP(useDisp);
    draw_pp = DrawGetFrameP(drDisp);

    CmnGifADPacketMake(&gifpk, draw_pp);

    /* The original builds a masked copy of the draw frame that is never submitted. */
    maskedFrame = *draw_pp;
    maskedFrame.FBMSK = 0x1f1f1f1f;

    UG_MozaikuDisp(para_pp, use_pp, &gifpk);
    CmnGifADPacketMakeTrans(&gifpk);
    return 0;
}

int DrawFadeDisp(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGsFrame    *draw_pp, *use_pp;
    sceGifPacket   gifpk;
    int            alp;
    int            i;
    int            stp, endp;
    int            lngT, lngN;
    FADE_MAKE_STR  fade_make_str;
    FADE_STR      *fade_pp = (FADE_STR*)para_pp; /* note: not in STABS. */

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    draw_pp = DrawGetFrameP(drDisp);
    use_pp = DrawGetFrameP(useDisp);
    CmnGifADPacketMake(&gifpk, draw_pp);

    if (drDisp == DNUM_DRAW && useDisp == DNUM_VRAM2) {
        sceGifPkAddGsAD(&gifpk, SCE_GS_XYOFFSET_1, *(u_long*)&DrawGetDrawEnvP(DNUM_VRAM2)->xyoffset1);
    }

    stp = 0;
    endp = -1;

    for (i = 0; i < fade_pp->fade_data_size; i++) {
        if (fade_pp->fade_data_pp[i].frame == frame) {
            stp = i;
            endp = i;
            break;
        }

        if (fade_pp->fade_data_pp[i].frame < frame) {
            stp = i;
        }

        if (fade_pp->fade_data_pp[i].frame > frame) {
            endp = i;
            break;
        }
    }

    if (endp == -1) {
        endp = stp;
    }

    lngT = fade_pp->fade_data_pp[endp].frame - fade_pp->fade_data_pp[stp].frame;
    lngN = frame - fade_pp->fade_data_pp[stp].frame;

    if (lngT == 0) {
        alp = fade_pp->fade_data_pp[stp].alp;
    } else {
        alp = fade_pp->fade_data_pp[endp].alp - fade_pp->fade_data_pp[stp].alp;
        alp = ((alp * lngN) / lngT) + fade_pp->fade_data_pp[stp].alp;
    }

    fade_make_str.r = fade_pp->r;
    fade_make_str.g = fade_pp->g;
    fade_make_str.b = fade_pp->b;
    fade_make_str.alp = alp;
    UG_FadeDisp(&fade_make_str, &gifpk, use_pp);

    CmnGifADPacketMakeTrans(&gifpk);
    return 0;
}

int DrawPlphaIndex8Disp(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGifPacket    gifpk;
    sceGsFrame     *use_pp, *draw_pp;
    int             r_tmp, g_tmp, b_tmp, a_tmp;
    int             i, stp, endp;
    int             lngT, lngN;
    int             useTbp;
    ALP_INDEX8_STR *alp_pp = (ALP_INDEX8_STR*)para_pp; /* note: not in STABS. */

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    use_pp = DrawGetFrameP(useDisp);
    draw_pp = DrawGetFrameP(drDisp);

    stp = 0;
    endp = -1;

    for (i = 0; i < alp_pp->alp8_size; i++) {
        if (alp_pp->alp8_pp[i].frame == frame) {
            stp = i;
            endp = i;
            break;
        }

        if (alp_pp->alp8_pp[i].frame < frame) {
            stp = i;
        }

        if (alp_pp->alp8_pp[i].frame > frame) {
            endp = i;
            break;
        }
    }

    if (endp == -1) {
        endp = stp;
    }

    lngT = alp_pp->alp8_pp[endp].frame - alp_pp->alp8_pp[stp].frame;
    lngN = frame - alp_pp->alp8_pp[stp].frame;

    if (lngT == 0) {
        r_tmp = alp_pp->alp8_pp[stp].r;
        g_tmp = alp_pp->alp8_pp[stp].g;
        b_tmp = alp_pp->alp8_pp[stp].b;
        a_tmp = alp_pp->alp8_pp[stp].alp;
    } else {
        r_tmp = alp_pp->alp8_pp[endp].r - alp_pp->alp8_pp[stp].r;
        r_tmp = ((r_tmp * lngN) / lngT) + alp_pp->alp8_pp[stp].r;

        g_tmp = alp_pp->alp8_pp[endp].g - alp_pp->alp8_pp[stp].g;
        g_tmp = ((g_tmp * lngN) / lngT) + alp_pp->alp8_pp[stp].g;

        b_tmp = alp_pp->alp8_pp[endp].b - alp_pp->alp8_pp[stp].b;
        b_tmp = ((b_tmp * lngN) / lngT) + alp_pp->alp8_pp[stp].b;

        a_tmp = alp_pp->alp8_pp[endp].alp - alp_pp->alp8_pp[stp].alp;
        a_tmp = ((a_tmp * lngN) / lngT) + alp_pp->alp8_pp[stp].alp;
    }

    if (use_pp != NULL) {
        useTbp = DrawGetFbpPos(useDisp) << 5;
    } else {
        useTbp = DrawGetFbpPos(drDisp) << 5;
    }

    Tim2Trans_TBP_MODE(GetIntAdrsCurrent(alp_pp->alp8_pp[stp].tim2num), useTbp, SCE_GS_PSMT8H);

    CmnGifADPacketMake(&gifpk, draw_pp);

    if (use_pp != NULL) {
        sceGifPkAddGsAD(&gifpk, SCE_GS_TEX0_1, GS_FRAME_TEX0(use_pp, 1, SCE_GS_MODULATE));
        sceGifPkAddGsAD(&gifpk, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0));
    }

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEST_1, SCE_GS_SET_TEST(SCE_GS_TRUE, SCE_GS_ALPHA_NEVER, 0, SCE_GS_AFAIL_RGB_ONLY, SCE_GS_FALSE, 0, SCE_GS_TRUE, SCE_GS_DEPTH_ALWAYS));

    sceGifPkAddGsAD(&gifpk, SCE_GS_ALPHA_1,
        (use_pp == NULL)
        ? SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_AD, SCE_GS_ALPHA_CD, 0)
        : SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_AS, SCE_GS_ALPHA_CD, 0)
    );

    sceGifPkAddGsAD(&gifpk, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(r_tmp, g_tmp, b_tmp, a_tmp, 0x00000001));

    sceGifPkAddGsAD(&gifpk, SCE_GS_PRIM,
        (use_pp != NULL)
        ? SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0)
        : SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0)
    );

    sceGifPkAddGsAD(&gifpk, SCE_GS_UV, SCE_GS_SET_UV((0 << 4) | (1 << 3), (0 << 4) | (1 << 3)));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ(GS_X_COORD(0), GS_Y_COORD(0), 1));

    sceGifPkAddGsAD(&gifpk, SCE_GS_UV, SCE_GS_SET_UV((SCREEN_WIDTH << 4) | (1 << 3), (SCREEN_FIELD_HEIGHT << 4) | (1 << 3)));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ(GS_X_COORD(SCREEN_WIDTH), GS_Y_COORD(SCREEN_FIELD_HEIGHT), 1));

    CmnGifADPacketMakeTrans(&gifpk);
    return 0;
}

int DrawTim2DIsp(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    TIM2DISP_STR* tim2disp_pp = (TIM2DISP_STR*)para_pp; /* note: not in STABS. */

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    ChangeDrawArea(DrawGetDrawEnvP(drDisp));

    SprInit();
    SprClear();
    SprPackSet((SPR_DAT*)tim2disp_pp->tim2_dat);
    SprDispAcheck(TRUE);
    SprDisp(tim2disp_pp->spr_prim);
    SprFlash();

    return 0;
}

int DrawNoodlesDisp(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGsFrame   *draw_pp, *use_pp, *col_pp;
    sceGifPacket  gifpk;
    NOODLES_STR  *ndl_pp = (NOODLES_STR*)para_pp; /* note: not in STABS. */

    if (first_f == DRPRGF_INIT) {
        return 0;
    }
    if (first_f == DRPRGF_RESET) {
        return 0;
    }

    use_pp = DrawGetFrameP(useDisp);
    draw_pp = DrawGetFrameP(drDisp);
    col_pp = DrawGetFrameP(DNUM_ZBUFF);

    CmnGifADPacketMake(&gifpk, draw_pp);

    Tim2Trans_TBP_MODE(GetIntAdrsCurrent(ndl_pp->texnum), use_pp->FBP << 5, SCE_GS_PSMT8H);
    Tim2TransColor_TBP(GetIntAdrsCurrent(ndl_pp->texnum), col_pp->FBP << 5);

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEX0_1, SCE_GS_SET_TEX0(use_pp->FBP << 5, use_pp->FBW, SCE_GS_PSMT8H, 10, 8, 1, SCE_GS_MODULATE, col_pp->FBP << 5, SCE_GS_PSMCT16, 0, 0, 1));
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0));

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEST_1, SCE_GS_SET_TEST(SCE_GS_TRUE, SCE_GS_ALPHA_NEVER, 0, SCE_GS_AFAIL_RGB_ONLY, SCE_GS_FALSE, 0, SCE_GS_TRUE, 1));
    sceGifPkAddGsAD(&gifpk, SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_AS, SCE_GS_ALPHA_CD, 0));
    sceGifPkAddGsAD(&gifpk, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 64));

    sceGifPkAddGsAD(&gifpk, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, SCE_GS_TRUE, SCE_GS_FALSE, SCE_GS_TRUE, SCE_GS_FALSE, 1, 0, 0));
    sceGifPkAddGsAD(&gifpk, SCE_GS_FRAME_1, *(u_long*)use_pp);

    sceGifPkAddGsAD(&gifpk, SCE_GS_UV, SCE_GS_SET_UV(0, 0));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ(GS_X_COORD(0), GS_Y_COORD(0), 1));

    sceGifPkAddGsAD(&gifpk, SCE_GS_UV, SCE_GS_SET_UV(SCREEN_WIDTH << 4, SCREEN_FIELD_HEIGHT << 4));
    sceGifPkAddGsAD(&gifpk, SCE_GS_XYZ2, SCE_GS_SET_XYZ(GS_X_COORD(SCREEN_WIDTH), GS_Y_COORD(SCREEN_FIELD_HEIGHT), 1));

    sceGifPkAddGsAD(&gifpk, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(&gifpk, SCE_GS_FRAME_1, *(u_long*)draw_pp);

    UG_NoodlesDisp(ndl_pp, use_pp, &gifpk, frame);

    CmnGifADPacketMakeTrans(&gifpk);
    return 0;
}

int DrawVramLocalCopy(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGsMoveImage mi;
    short          sbp, dbp;

    if (first_f != DRPRGF_FIRST) {
        return 0;
    }

    printf("local vram copy\n");

    sbp = DrawGetTbpPos(useDisp);
    dbp = DrawGetTbpPos(drDisp);

    GGsSetLocalMoveImage(&mi, dbp, 10, SCE_GS_PSMCT32, 0, 0, sbp, 10, SCE_GS_PSMCT32, 0, 0, SCREEN_WIDTH, SCREEN_FIELD_HEIGHT, 0);
    FlushCache(WRITEBACK_DCACHE);

    GGsExecLocalMoveImage(&mi);
    sceGsSyncPath(0, 0);

    return 0;
}

int DrawVramLocalCopy2(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    sceGsMoveImage mi;
    short          sbp, dbp;

    if (first_f == DRPRGF_RESET) {
        return 0;
    }
    if (first_f == DRPRGF_INIT) {
        return 0;
    }

    printf("local vram copy\n");

    sbp = DrawGetTbpPos(useDisp);
    dbp = DrawGetTbpPos(drDisp);

    GGsSetLocalMoveImage(&mi, dbp, 10, SCE_GS_PSMCT32, 0, 0, sbp, 10, SCE_GS_PSMCT32, 0, 0, SCREEN_WIDTH, SCREEN_FIELD_HEIGHT, 0);
    FlushCache(WRITEBACK_DCACHE);

    GGsExecLocalMoveImage(&mi);
    sceGsSyncPath(0, 0);

    return 0;
}

static void drawUseDrDispCheckInit(void) {
    useDispFlag = DNUM_NON;
    drDispFlag = DNUM_NON;
}

static int drawDispCheckSub(u_int drD, u_int *dat_pp) {
    int ret = drD & 0xff;

    if (((drD >> 0x8) & 0xff) != 0) {
        int v0 = *dat_pp & 0xff;
        if (v0 & drD) {
            ret = (drD >> 0x8) & 0xff;
        }
    }

    *dat_pp |= ret;
    return ret;
}

/* static */ int drawUseDispCheck(int useD) {
    return drawDispCheckSub(useD, &useDispFlag);
}

/* static */ int drawDrDispCheck(int drD) {
    return drawDispCheckSub(drD, &drDispFlag);
}

static int DrawScenectrlReq(SCENECTRL *scenectrl_pp, u_int time) {
    int ontime_flag;
    int ret;

    ret = 0;

    ontime_flag = (time >= scenectrl_pp->start_flame);
    if (time >= scenectrl_pp->end_flame) {
        ontime_flag = FALSE;
    }

    if (scenectrl_pp->prg_pp == NULL) {
        return -1;
    }

    if (ontime_flag) {
        int use_num = drawUseDispCheck(scenectrl_pp->useDisp);
        int dr_num = drawDrDispCheck(scenectrl_pp->drDisp);

        switch (dr_num) {
        case DNUM_VRAM2:
            UseGsSetXyOffset(FALSE);
            break;
        case DNUM_DRAW:
            if (oddeven_idx & 1) {
                UseGsSetXyOffset(FALSE);
            } else {
                UseGsSetXyOffset(TRUE);
            }
            break;
        }

        if (!scenectrl_pp->use_flag) {
            ret = scenectrl_pp->prg_pp(scenectrl_pp->param_pp, time - scenectrl_pp->start_flame, DRPRGF_FIRST, use_num, dr_num);
            scenectrl_pp->use_flag = TRUE;
        } else {
            ret = scenectrl_pp->prg_pp(scenectrl_pp->param_pp, time - scenectrl_pp->start_flame, DRPRGF_TIMES, use_num, dr_num);
        }
    } else {
        if (scenectrl_pp->use_flag) {
            ret = scenectrl_pp->prg_pp(scenectrl_pp->param_pp, 0, DRPRGF_RESET, DNUM_NON, DNUM_NON);
            scenectrl_pp->use_flag = FALSE;
        }
    }

    return ret;
}

static int mend_title_req = 0;

void MendererCtrlInit(void) {
    men_ctrl_ratio = 0.0f;
    men_ctrl_enum = MEN_CTRL_BtoG;
    mend_title_req = 0;
}

void MendererCtrlTitle(void) {
    men_ctrl_ratio = 2.0f;
    men_ctrl_enum = MEN_CTRL_BtoA;
    mend_title_req = 1;
}

void MendererCtrlTitleDera(void) {
    men_ctrl_ratio = 2.0f;
    men_ctrl_enum = MEN_CTRL_BtoA;
    mend_title_req = 2;
}

static void MendererCtrl(void) {
    float Mmax = 0.0f;
    float Mmin = 0.0f;

    switch (men_ctrl_enum) {
    case MEN_CTRL_BtoG:
        men_ctrl_ratio -= 0.0083333338f;
        Mmax = 1.0f, Mmin = 0.0f;
        PrSetMendererDirection(FALSE);
        break;
    case MEN_CTRL_AtoAB:
        men_ctrl_ratio -= 0.0083333338f;
        Mmax = 2.0f, Mmin = 1.5f;
        PrSetMendererDirection(FALSE);
        break;
    case MEN_CTRL_ABtoB:
        men_ctrl_ratio -= 0.0083333338f;
        Mmax = 1.5f, Mmin = 1.0f;
        PrSetMendererDirection(FALSE);
        break;
    case MEN_CTRL_AtoB:
        men_ctrl_ratio -= 0.0083333338f;
        Mmax = 2.0f, Mmin = 1.0f;
        PrSetMendererDirection(FALSE);
        break;
    case MEN_CTRL_GtoB:
        men_ctrl_ratio += 0.0083333338f;
        Mmax = 1.0f, Mmin = 0.0f;
        PrSetMendererDirection(TRUE);
        break;
    case MEN_CTRL_BtoBA:
        men_ctrl_ratio += 0.0083333338f;
        Mmax = 1.5f, Mmin = 1.0f;
        PrSetMendererDirection(TRUE);
        break;
    case MEN_CTRL_BAtoA:
        men_ctrl_ratio += 0.0083333338f;
        Mmax = 2.0f, Mmin = 1.5f;
        PrSetMendererDirection(TRUE);
        break;
    case MEN_CTRL_BtoA:
        men_ctrl_ratio += 0.0083333338f;
        Mmax = 2.0f, Mmin = 1.0f;
        PrSetMendererDirection(TRUE);
        break;
    }

    if (men_ctrl_ratio > Mmax) {
        men_ctrl_ratio = Mmax;
    }
    if (men_ctrl_ratio < Mmin) {
        men_ctrl_ratio = Mmin;
    }

    if (men_ctrl_ratio != 0.0f) {
        PrSetMendererRatio(men_ctrl_ratio);
        PrRenderMenderer();
    }
}

static float mendRatioTitleGet(int frame, int dera_f) {
    int           i;
    int           stp, endp;
    MENTITLE_DAT *men_pp, *men_mot_pp;
    int           sizeMen;
    int           lntT, nowT;
    float         per, retT;

    stp = 0;
    endp = -1;

    if (dera_f) {
        sizeMen = 168;
        men_pp = mentitle_dat_dera;
    } else {
        sizeMen = 8;
        men_pp = mentitle_dat;
    }

    men_mot_pp = men_pp;

    for (i = 0; i < sizeMen; i++, men_pp++) {
        if (men_pp->frame == frame) {
            stp = i;
            endp = i;
            break;
        }

        if (men_pp->frame < frame) {
            stp = i;
        }

        if (men_pp->frame > frame) {
            endp = i;
            break;
        }
    }

    if (endp == -1) {
        endp = stp;
    }

    if (endp == stp) {
        retT = men_mot_pp[stp].ratio;
        return retT;
    } else {
        lntT = men_mot_pp[endp].frame;
        nowT = frame - men_mot_pp[stp].frame;
        retT = men_mot_pp[endp].ratio - men_mot_pp[stp].ratio;

        per = (float)nowT / (float)(lntT - men_mot_pp[stp].frame);
        retT = (retT * per) + men_mot_pp[stp].ratio;
        return retT;
    }
}

static void MendererCtrlTitleDisp(int frame, int dera_f) {
    PrSetMendererRatio(mendRatioTitleGet(frame, dera_f));
    PrRenderMenderer();

    PrSetMendererRatio(2.0f);
}

void MendererReq(MEN_CTRL_ENUM menum) {
    men_ctrl_enum = menum;
}

MEN_CTRL_ENUM GetMendererEnum(void) {
    return men_ctrl_enum;
}

int MendererCtrlScene(void *para_pp, int frame, int first_f, int useDisp, int drDisp) {
    int scene_req_flag = *(int*)para_pp; /* note: not in STABS. */

    if (!scene_req_flag) {
        UsrPrSetScene();
    }

    ChangeDrawArea(DrawGetDrawEnvP(drDisp));

    if (mend_title_req == 1) {
        MendererCtrlTitleDisp(drawCurrentTime, FALSE);
    } else if (mend_title_req == 2) {
        MendererCtrlTitleDisp(drawCurrentTime, TRUE);
    } else {
        MendererCtrl();
    }

    return 0;
}

/*
 * -- Stage condition flag definition --
 * 
 * (andor)  (unused)  (stg clr)  (tbl)   (rnd)
 *    0    0000000000 000000000 00000000 0000
 * 
 * Bit(s) 0-3: Round/circuit/hat
 *   Each bit represents a round from 1 to 4.
 * 
 * Bit(s) 4-11: Table number (difficulty/level)
 *   Each bit represents a table number (difficulty/level) from 0 to 7.
 * 
 * Bit(s) 12-20: Stage clear
 *   Each bit represents a stage from 0 to 8.
 * 
 * Bit(s) 31: ANDOR (AND/OR mode)
 *   0: Conditions use OR logic.
 *      cond = RND | TBL | STGCLR;
 *   1: Conditions use AND logic.
 *      cond = RND & TBL & STGCLR;
 */
int sceneConditionCheck(u_int cond_flag) {
    int andor;
    int i;

    andor = cond_flag & SBE_ANDBIT;
    cond_flag &= 0x7fffffff;

    if (cond_flag == 0) {
        return 1;
    }

    for (i = 0; i < SBI_MAX; i++) {
        if (((int)cond_flag >> i) & 1) {
            switch (i) {
            case SBI_R1:
            case SBI_R2:
            case SBI_R3:
            case SBI_R4:
                if (i != global_data.roundL) {
                    if (andor != 0) {
                        return 0;
                    }
                } else {
                    if (andor == 0) {
                        return 1;
                    }
                }
                break;
            case SBI_TBL0:
            case SBI_TBL1:
            case SBI_TBL2:
            case SBI_TBL3:
            case SBI_TBL4:
            case SBI_TBL5:
            case SBI_TBL6:
            case SBI_TBL7:
                if (GetCurrentTblNumber() != (i - 4)) {
                    if (andor != 0) {
                        return 0;
                    }
                } else {
                    if (andor == 0) {
                        return 1;
                    }
                }
                break;
            case SBI_CLR0:
            case SBI_CLR1:
            case SBI_CLR2:
            case SBI_CLR3:
            case SBI_CLR4:
            case SBI_CLR5:
            case SBI_CLR6:
            case SBI_CLR7:
            case SBI_CLR8:
                if (clearStageCheck() != (i - 12)) {
                    if (andor != 0) {
                        return 0;
                    }
                } else {
                    if (andor == 0) {
                        return 1;
                    }
                }
                break;
            default:
                return 0;
            }
        }
    }

    return (andor != 0);
}

static void DrawCtrlMain(void *x) {
    int        i, j, lsjkl;
    int        check_cnt;
    SCENESTR  *scenestr_pp;
    SCENECTRL *scenectrl_pp;
    int        scene_req_flag;
    SCENECTRL *scenectrl_tmp;
    
    BallThrowInit();
    MozaikuPollSceneInit();

    while (1) {
        scene_req_flag = 0;

        if (GlobalMendererUseCheck()) {
            PrCreateMendererTexture();
        }

        if (drawCurrentLine >= drawEventrec->scenestr_size) {
            scenestr_pp  = NULL;
            scenectrl_pp = NULL;
        } else {
            scenestr_pp  = &drawEventrec->scenestr_pp[drawCurrentLine];
            scenectrl_pp = scenestr_pp->scenectrl_pp;
        }

        drawUseDrDispCheckInit();
        check_cnt = 0;

        if (global_data.play_step == PSTEP_XTR) {
            scenestr_pp  = NULL;
            scenectrl_pp = NULL;
            p3StrPoll(drawCurrentTime);
        }

        if (scenectrl_pp != NULL) {
            for (i = 0; i < PR_ARRAYSIZE(check_scenectrl); i++) {
                check_scenectrl[i] = NULL;
            }

            for (i = 0; i < scenestr_pp->scenectrl_num; i++, scenectrl_pp++) {
                if (scenectrl_pp->start_flame <= drawCurrentTime &&
                    scenectrl_pp->end_flame    > drawCurrentTime) {
                    if (sceneConditionCheck(scenectrl_pp->condition)) {
                        check_scenectrl[check_cnt] = scenectrl_pp;
                        check_cnt++;
                        continue;
                    }
                }

                DrawScenectrlReq(scenectrl_pp, INT_MAX);
            }
        }

        outsideDrawSceneReq(BallThrowPollScene, 200, DNUM_NON, DNUM_DRAW, NULL);
        outsideDrawSceneReq(MozaikuPollScene, 215, DNUM_DRAW, DNUM_DRAW, NULL);

        if (GlobalMendererUseCheck()) {
            outsideDrawSceneReq(MendererCtrlScene, 210, DNUM_NON, DNUM_DRAW, &scene_req_flag);
        }

        if (global_data.play_step == PSTEP_VS || global_data.play_step == PSTEP_HOOK) {
            outsideDrawSceneReq(DrawVramClear, 229, DNUM_NON, DNUM_ZBUFF, &vclr_black);
        }

        while ((scenectrl_tmp = getOutsideCtrlScene(drawCurrentTime)) != NULL) {
            check_scenectrl[check_cnt] = scenectrl_tmp;
            check_cnt++;
        }

        for (i = 0; i < check_cnt - 1; i++) {
            for (j = i + 1; j < check_cnt; j++) {
                if (check_scenectrl[i]->pri > check_scenectrl[j]->pri) {
                    SCENECTRL *tmp = check_scenectrl[i];
                    check_scenectrl[i] = check_scenectrl[j];
                    check_scenectrl[j] = tmp;
                }
            }
        }

        for (lsjkl = 0; lsjkl < check_cnt; lsjkl++) {
            float men_tmp = PrGetMendererRatio();

            if (check_scenectrl[lsjkl]->prg_pp == DrawSceneObjData) {
                if (check_scenectrl[lsjkl]->pri > 210) {
                    PrSetMendererRatio(0.0f);
                }
                
                scene_req_flag = TRUE;
                DrawScenectrlReq(check_scenectrl[lsjkl], drawCurrentTime);
                PrSetMendererRatio(men_tmp);
            } else {
                if (!scene_req_flag) {
                    UsrPrSetScene();
                    scene_req_flag = TRUE;
                }

                DrawScenectrlReq(check_scenectrl[lsjkl], drawCurrentTime);
            }
        }

        dr_tap_req_num = 0;

        ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));
        UseGsRegSet();

        if (!scene_req_flag) {
            UsrPrSetScene();
        }

        outsideDrawSceneClear();
        MtcWait(1);
    }
}

static void DrawSceneStrInit(SCENESTR *scstr_pp) {
    int i;

    for (i = 0; i < scstr_pp->scenectrl_num; i++) {
        scstr_pp->scenectrl_pp[i].use_flag = SC_UF_NOUSE;

        if (scstr_pp->scenectrl_pp[i].prg_pp != NULL) {
        #if defined(PRD_SHIFTABLE)
            /*
             * Overlays have pointers to scene draw functions.
             * Until overlays are linked properly, we need
             * to fix the pointers at runtime.
             *
             * note: Addresses are hardcoded for the Jul. 12th
             *       prototype overlays.
             */
            u_int prg = (u_int)scstr_pp->scenectrl_pp[i].prg_pp;
            if (prg == 0x00113bd8) {
                scstr_pp->scenectrl_pp[i].prg_pp = DrawSceneObjData;
            } else if (prg == 0x00114200) {
                scstr_pp->scenectrl_pp[i].prg_pp = DrawVramClear;
            } else if (prg == 0x00114358) {
                scstr_pp->scenectrl_pp[i].prg_pp = DrawMoveDispIn;
            } else if (prg == 0x00114750) {
                scstr_pp->scenectrl_pp[i].prg_pp = DrawFadeDisp;
            } else if (prg == 0x00115000) {
                scstr_pp->scenectrl_pp[i].prg_pp = DrawVramLocalCopy2;
            } else if (prg == 0x0011b368) {
                scstr_pp->scenectrl_pp[i].prg_pp = MbarDispSceneDraw;
            } else if (prg == 0x0011b520) {
                scstr_pp->scenectrl_pp[i].prg_pp = MbarDispSceneVsDraw;
            } else {
                /*
                 * Can't hang here, DrawSceneStrInit is called
                 * when retrying a stage. Also, the printf call
                 * doesn't seem to work?
                 */
            #if 0
                scePrintf("Unknown program [0x%08x]", prg);
                PR_BREAK();
            #endif
            }
        #endif
            scstr_pp->scenectrl_pp[i].prg_pp(scstr_pp->scenectrl_pp[i].param_pp, 0, DRPRGF_INIT, scstr_pp->scenectrl_pp[i].useDisp, scstr_pp->scenectrl_pp[i].drDisp);
        }
    }
}

static void DrawSceneStrReset(SCENESTR *scstr_pp) {
    int i;

    if (scstr_pp == NULL || scstr_pp->scenectrl_pp == NULL) {
        return;
    }

    for (i = 0; i < scstr_pp->scenectrl_num; i++) {
        if (scstr_pp->scenectrl_pp[i].use_flag != SC_UF_NOUSE) {
            scstr_pp->scenectrl_pp[i].use_flag = SC_UF_NOUSE;

            if (scstr_pp->scenectrl_pp[i].prg_pp != NULL) {
                scstr_pp->scenectrl_pp[i].prg_pp(scstr_pp->scenectrl_pp[i].param_pp, 0, DRPRGF_RESET, 0, 0);
            }
        }
    }
}

void resetDrawSceneObjData(SCENESTR *scstr_pp) {
    int            i;
    SCENE_OBJDATA *sod_pp;

    for (i = 0; i < scstr_pp->scenectrl_num; i++) {
        if (scstr_pp->scenectrl_pp[i].prg_pp != NULL &&
            scstr_pp->scenectrl_pp[i].prg_pp == DrawSceneObjData) {
            sod_pp = scstr_pp->scenectrl_pp[i].param_pp;
            sod_pp->handle = NULL;
        }
    }
}

void DrawCtrlInit(EVENTREC *ev_pp, int ctrlTbl, void *dat_top) {
    int i;

    PrSetFrameRate(60.0f);
    DrawTmpBuffInit();

    for (i = 0; i < ev_pp->scenestr_size; i++) {
        resetDrawSceneObjData(&ev_pp->scenestr_pp[i]);
    }

    for (i = 0; i < ev_pp->scenestr_size; i++) {
        DrawSceneStrInit(&ev_pp->scenestr_pp[i]);
    }

    drawEventrec    = ev_pp;
    drawCurrentLine = ctrlTbl;

    drawCurrentTime    = 0;
    drawCurrentTimeOld = -1;
    dr_tap_req_num     = 0;

    outsideDrawSceneClear();
    MendererCtrlInit();
    
    MtcExec(DrawCtrlMain, MTC_TASK_DRAWCTRL);
}

void DrawCtrlQuit(void) {
    int       i;
    EVENTREC *ev_pp = drawEventrec;

    DrawTmpBuffQuit(NULL);

    for (i = 0; i < ev_pp->scenestr_size; i++) {
        resetDrawSceneObjData(&ev_pp->scenestr_pp[i]);
    }

    if (global_data.play_step == PSTEP_XTR) {
        p3StrQuitSd();
    }

    MendererCtrlInit();

    PrSetMendererRatio(0.0f);
    PrCleanupModel(NULL);
    PrCleanupAnimation(NULL);
    PrCleanupCamera(NULL);
    PrCleanupScene(NULL);

    MtcKill(MTC_TASK_DRAWCTRL);
}

void DrawCtrlTimeSet(int time) {
    if (drawCurrentTime == 0) {
        drawCurrentTime = time;
        return;
    }

    if (drawCurrentTime == time) {
        drawCurrentTime = time + 1;
        return;
    }

    drawCurrentTime = time;
}

void DrawCtrlTblChange(int ctrlTbl) {
    if (ctrlTbl == drawCurrentLine) {
        return;
    }

    if (drawCurrentLine >= drawEventrec->scenestr_size) {
        drawCurrentLine = ctrlTbl;
        return;
    }

    DrawSceneStrReset(&drawEventrec->scenestr_pp[drawCurrentLine]);
    drawCurrentLine = ctrlTbl;
}

int DrawTapReqTbl(int atap, PLAYER_INDEX pindx, u_char *prs_pp) {
    if (dr_tap_req_num >= 16) {
        printf("TAP REQ OVER ERROR!!\n");
        return 1;
    }

    dr_tap_req[dr_tap_req_num].tap_id = (u_int)(atap & 0xff00) >> 8;
    dr_tap_req[dr_tap_req_num].req_no = atap & 0x00ff;
    dr_tap_req[dr_tap_req_num].player_index = pindx;
    dr_tap_req[dr_tap_req_num].pad_prs_pp = prs_pp;
    dr_tap_req_num++;
    return 0;
}

static int ddbg_event_num = 0;
static int ddbg_scene_num = 0;
static int ddbg_go_event_scene = 0;
static int ddbg_tap_num = 0;
static int ddbg_bmp_frame = 0;

/* static */ int ddbg_event_sel(int pad) {
    ddbg_event_num += pad;

    if (ddbg_event_num < 0) {
        ddbg_event_num = drawEventrec->scenestr_size - 1;
    }
    
    if (ddbg_event_num >= drawEventrec->scenestr_size) {
        ddbg_event_num = 0;
    }

    drawCurrentLine = ddbg_event_num;
    
    ddbg_scene_sel(0);
    return 0;
}

/* static */ void ddbg_event_msg(char *buf) {
    sprintf(buf, "#%02X", ddbg_event_num);
}

/* static */ int ddbg_scene_sel(int pad) {
    SCENESTR *scenestr_pp = &drawEventrec->scenestr_pp[ddbg_event_num];

    ddbg_scene_num += pad;

    if (ddbg_scene_num < 0) {
        ddbg_scene_num = scenestr_pp->scenectrl_num - 1;
    }
    
    if (ddbg_scene_num >= scenestr_pp->scenectrl_num) {
        ddbg_scene_num = 0;
    }

    return 0;
}

/* static */ void ddbg_scene_msg(char *buf) {
    sprintf(buf, "#%02X", ddbg_scene_num);
}

/* static */ int ddbg_event_check(int pad) {
    ddbg_go_event_scene = 0;
    return 1;
}

/* static */ int ddbg_scene_check(int pad) {
    ddbg_go_event_scene = 1;
    return 1;
}

/* static */ int ddbg_bmp_check(int pad) {
    ddbg_go_event_scene = 2;
    return 1;
}

static void ddbg_event_sub(void) {
    int        i, j, lsjkl;
    int        check_cnt;
    SCENECTRL *scenectrl_pp;
    SCENESTR  *scenestr_pp;

    if (drawEventrec->scenestr_pp[drawCurrentLine].scenectrl_pp == NULL) {
        return;
    }

    scenectrl_pp = drawEventrec->scenestr_pp[drawCurrentLine].scenectrl_pp;
    scenestr_pp = &drawEventrec->scenestr_pp[drawCurrentLine];

    check_cnt = 0;
    drawUseDrDispCheckInit();

    for (i = 0; i < PR_ARRAYSIZE(check_scenectrl); i++) {
        check_scenectrl[i] = NULL;
    }

    for (i = 0; i < scenestr_pp->scenectrl_num; i++, scenectrl_pp++) {
        if (scenectrl_pp->start_flame <= drawCurrentTime &&
            scenectrl_pp->end_flame    > drawCurrentTime) {
            check_scenectrl[check_cnt] = scenectrl_pp;
            check_cnt++;
        } else {
            DrawScenectrlReq(scenectrl_pp, drawCurrentTime);
        }
    }

    for (i = 0; i < check_cnt - 1; i++) {
        for (j = i + 1; j < check_cnt; j++) {
            if (check_scenectrl[i]->pri > check_scenectrl[j]->pri) {
                SCENECTRL *tmp = check_scenectrl[i];
                check_scenectrl[i] = check_scenectrl[j];
                check_scenectrl[j] = tmp;
            }
        }
    }

    for (lsjkl = 0; lsjkl < check_cnt; lsjkl++) {
        DrawScenectrlReq(check_scenectrl[lsjkl], drawCurrentTime);
    }

    dr_tap_req_num = 0;

    ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));
    UseGsRegSet();
}

/* Debug menu entries: label, selection handler, optional value printer. */
static DRAW_DBG_STR draw_dbg_str[5] = {
    { "EVENT LINE",  ddbg_event_sel,   ddbg_event_msg },
    { "SCENE LINE",  ddbg_scene_sel,   ddbg_scene_msg },
    { "EVENT CHECK", ddbg_event_check, NULL },
    { "SCENE CHECK", ddbg_scene_check, NULL },
    { "BMP OUT",     ddbg_bmp_check,   NULL },
};

static void ddbg_event_sub_bmp(void) {
    int        i, j, lsjkl;
    int        check_cnt;
    SCENECTRL *scenectrl_pp;
    SCENESTR  *scenestr_pp;
    u_char    *dat1_pp, *dat2_pp;
    
    if (drawEventrec->scenestr_pp[drawCurrentLine].scenectrl_pp == NULL) {
        return;
    }

    scenectrl_pp = drawEventrec->scenestr_pp[drawCurrentLine].scenectrl_pp;
    scenestr_pp = &drawEventrec->scenestr_pp[drawCurrentLine];

    check_cnt = 0;
    drawUseDrDispCheckInit();

    drawCurrentTime = ddbg_bmp_frame * 2;

    for (i = 0; i < PR_ARRAYSIZE(check_scenectrl); i++) {
        check_scenectrl[i] = NULL;
    }

    for (i = 0; i < scenestr_pp->scenectrl_num; i++, scenectrl_pp++) {
        if (scenectrl_pp->start_flame <= drawCurrentTime &&
            scenectrl_pp->end_flame    > drawCurrentTime) {
            check_scenectrl[check_cnt] = scenectrl_pp;
            check_cnt++;
        } else {
            DrawScenectrlReq(scenectrl_pp, drawCurrentTime);
        }
    }

    for (i = 0; i < check_cnt - 1; i++) {
        for (j = i + 1; j < check_cnt; j++) {
            if (check_scenectrl[i]->pri > check_scenectrl[j]->pri) {
                SCENECTRL *tmp = check_scenectrl[i];
                check_scenectrl[i] = check_scenectrl[j];
                check_scenectrl[j] = tmp;
            }
        }
    }

    /* BUG: should use usrMalloc */
    dat1_pp = malloc(0x8C000); /* ~573.4 KB */
    dat2_pp = malloc(0x8C000);

    sceGsSetHalfOffset(&DBufDc.draw11,  2048, 2048, 0);
    sceGsSetHalfOffset2(&DBufDc.draw12, 2048, 2048, 0);
    sceGsSetHalfOffset(&DBufDc.draw01,  2048, 2048, 0);
    sceGsSetHalfOffset2(&DBufDc.draw02, 2048, 2048, 0);
    FlushCache(WRITEBACK_DCACHE);

    ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));

    for (lsjkl = 0; lsjkl < check_cnt; lsjkl++) {
        DrawScenectrlReq(check_scenectrl[lsjkl], drawCurrentTime);
    }

    VramTmpSave(dat1_pp, SCREEN_WIDTH, SCREEN_FIELD_HEIGHT, outbuf_idx ^ 1);

    PR_SCOPE()
    VCLR_PARA vclr_para = {};
        
    DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_DRAW);
    DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_VRAM2);
    DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);
    PR_SCOPEEND()

    sceGsSetHalfOffset(&DBufDc.draw11,  2048, 2048, 1);
    sceGsSetHalfOffset2(&DBufDc.draw12, 2048, 2048, 1);
    sceGsSetHalfOffset(&DBufDc.draw01,  2048, 2048, 1);
    sceGsSetHalfOffset2(&DBufDc.draw02, 2048, 2048, 1);
    FlushCache(WRITEBACK_DCACHE);

    ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));

    for (lsjkl = 0; lsjkl < check_cnt; lsjkl++) {
        DrawScenectrlReq(check_scenectrl[lsjkl], drawCurrentTime);
    }

    ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));

    VramTmpSave(dat2_pp, SCREEN_WIDTH, SCREEN_FIELD_HEIGHT, outbuf_idx ^ 1);

    PR_SCOPE()
    char save_name[20];

    sprintf(save_name, "scr%04d.bmp", ddbg_bmp_frame);
    VramTmpSaveOutBMP(save_name, SCREEN_WIDTH, SCREEN_FIELD_HEIGHT, outbuf_idx ^ 1, dat1_pp, dat2_pp);
    PR_SCOPEEND()

    usrFree(dat2_pp);
    usrFree(dat1_pp);
    
    dr_tap_req_num = 0;
    ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));
    UseGsRegSet();

    ddbg_bmp_frame++;
}

static void ddbg_tap_check(void) {
    SCENECTRL     *scenectrl_pp;
    SCENE_OBJDATA *scene_objdata_pp;
    int            tid;
    int            req_num;
    u_short        paddata;
    u_char        *prs_adr;

    req_num = -1;
    prs_adr = NULL;

    if (drawEventrec->scenestr_pp[drawCurrentLine].scenectrl_pp == NULL) {
        return;
    }

    scenectrl_pp = &drawEventrec->scenestr_pp[drawCurrentLine].scenectrl_pp[ddbg_scene_num];
    if (scenectrl_pp->prg_pp == DrawSceneObjData) {
        scene_objdata_pp = scenectrl_pp->param_pp;
        if (scene_objdata_pp->tapstr_size != 0) {
            paddata = pad[0].one;
            tid = scene_objdata_pp->tap_id;

            if (paddata & SCE_PADLup) {
                req_num = ddbg_tap_num;
                prs_adr = &pad[0].press[2];
            } else if (paddata & SCE_PADLdown) {
                req_num = ddbg_tap_num;
                prs_adr = &pad[0].press[3];
            } else if (paddata & SCE_PADLright) {
                req_num = ddbg_tap_num;
                prs_adr = &pad[0].press[0];
                ddbg_tap_num++;
            } else if (paddata & SCE_PADLleft) {
                req_num = ddbg_tap_num;
                prs_adr = &pad[0].press[1];
                ddbg_tap_num--; 
            }

            if (ddbg_tap_num >= scene_objdata_pp->objtapstr_size) {
                ddbg_tap_num = 0;
            }

            if (ddbg_tap_num < 0) {
                ddbg_tap_num = scene_objdata_pp->objtapstr_size - 1;
            }

            if (req_num >= 0) {
                DrawTapReqTbl(req_num + (tid << 8), PINDEX_NONE, prs_adr);
            }
        }
    }
}

static void ddbg_scene_sub(void) {
    SCENECTRL *scenectrl_pp;

    if (drawEventrec->scenestr_pp[drawCurrentLine].scenectrl_pp == NULL) {
        return;
    }

    scenectrl_pp = &drawEventrec->scenestr_pp[drawCurrentLine].scenectrl_pp[ddbg_scene_num];

    drawUseDrDispCheckInit();

    if (drawCurrentTime < scenectrl_pp->start_flame) {
        drawCurrentTime = scenectrl_pp->start_flame;
    }
    if (drawCurrentTime >= scenectrl_pp->end_flame) {
        drawCurrentTime = scenectrl_pp->end_flame - 1;
    }

    DrawScenectrlReq(scenectrl_pp, drawCurrentTime);

    dr_tap_req_num = 0;
    ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));
    UseGsRegSet();
}

void ddbg_tctrl_sub(void) {
    u_short paddata = pad[0].shot;
    u_short paddata_one = pad[0].one;

    if (paddata_one & SCE_PADRdown) {
        ddbg_pause_f ^= 1;
    }

    if (paddata_one & SCE_PADRup) {
        drawCurrentTime = 0;
    }

    if (!ddbg_pause_f) {
        drawCurrentTime += 1;
    } else {
        if (paddata & SCE_PADR1) {
            drawCurrentTime++;
        }
        if (paddata_one & SCE_PADR2) {
            drawCurrentTime += 600;
        }

        if (paddata & SCE_PADL1) {
            drawCurrentTime--;
        }
        if (paddata_one & SCE_PADL2) {
            drawCurrentTime -= 600;
        }

        if (paddata_one & SCE_PADRright) {
            drawCurrentTime++;
        }
        if (paddata_one & SCE_PADRleft) {
            drawCurrentTime--;
        }
    }

    if (drawCurrentTime < 0) {
        drawCurrentTime = 0;
    }
}

static void DrawCtrlMainDebug(void *x) {
    u_short paddata;
    int     sel_pos;
    int     i;
    char    msg_buff[32];

    sel_pos = 0;

    while (1) {
        DbgMsgInit();

        while (1) {
            MtcWait(1);

            paddata = pad[0].one;
            if (paddata & SCE_PADLup) {
                sel_pos--;
            }
            if (paddata & SCE_PADLdown) {
                sel_pos++;
            }

            if (sel_pos < 0) {
                sel_pos = 4;
            }
            if (sel_pos > 4u) {
                sel_pos = 0;
            }

            if (paddata & SCE_PADRright) {
                if (draw_dbg_str[sel_pos].prg_pp(1)) {
                    ddbg_pause_f = 0;
                    break;
                }
            }
            if (paddata & SCE_PADRleft) {
                if (draw_dbg_str[sel_pos].prg_pp(-1)) {
                    ddbg_pause_f = 0;
                    break;
                }
            }

            DbgMsgClear();
            DbgMsgSetSize(0x20, 0x10);

            for (i = 0; i < PR_ARRAYSIZEU(draw_dbg_str); i++) {
                if (i == sel_pos) {
                    DbgMsgSetColor(128, 128, 0);
                } else {
                    DbgMsgSetColor(128, 128, 128);
                }

                DbgMsgPrint(draw_dbg_str[i].dbgmsg, 0x6ea, 0x79c + (i*0x14));
                if (draw_dbg_str[i].msg_pp != NULL) {
                    draw_dbg_str[i].msg_pp(msg_buff);
                    DbgMsgPrint(msg_buff, 0x866, 0x79c + (i*0x14));
                }
            }

            DbgMsgFlash();
        }

        drawCurrentTime = 0;
        ddbg_tap_num = 0;
        ddbg_bmp_frame = 0;

        while (1) {
            MtcWait(1);

            if (pad[0].one & SCE_PADselect) {
                break;
            }
            
            ddbg_tctrl_sub();
            if (ddbg_go_event_scene == 1) {
                ddbg_tap_check();
                ddbg_scene_sub();
            } else if (ddbg_go_event_scene == 0) {
                ddbg_event_sub();
            } else {
                ddbg_event_sub_bmp();
            }
        }

        DrawSceneStrReset(&drawEventrec->scenestr_pp[drawCurrentLine]);
    }
}

void DrawCtrlInitDebug(EVENTREC *ev_pp, int ctrlTbl, void *dat_top) {
    int i;

    PrSetFrameRate(60.0f);
    DrawTmpBuffInit();

    for (i = 0; i < ev_pp->scenestr_size; i++) {
        DrawSceneStrInit(&ev_pp->scenestr_pp[i]);
    }

    drawEventrec       = ev_pp;
    drawCurrentLine    = 0;
    drawCurrentTime    = 0;
    drawCurrentTimeOld = -1;

    dr_tap_req_num = 0;

    MtcExec(DrawCtrlMainDebug, MTC_TASK_DRAWCTRL);
}
