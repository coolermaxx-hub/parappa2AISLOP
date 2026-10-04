#include "main/wipe.h"

#include "os/cmngifpk.h"
#include "os/mtc.h"
#include "os/syssub.h"
#include "os/system.h"
#include "os/tim2.h"

#include "main/cmnfile.h"
#include "main/drawctrl.h"
#include "main/effect.h"
#include "main/scrctrl.h"
#include "main/sprite.h"
#include "main/subt.h"

#include "iop_mdl/tapctrl_rpc.h"

#include <stdio.h>

static SNDTAP sndtap_wipe[] = {
    /* STW_TURN_IN */
    { .prg = 0, .key = 0xc,  .volume = 80 },
    /* STW_TURN_WAIT */
    { .prg = 0, .key = 0xd,  .volume = 80 },
    /* STW_TURN_OUT */
    { .prg = 0, .key = 0xe,  .volume = 80 },
    /* STW_PARAtoMINI */
    { .prg = 0, .key = 0xf,  .volume = 80 },
    /* STW_PARAtoBIG */
    { .prg = 0, .key = 0x10, .volume = 90 },
    /* STW_YESNO */
    { .prg = 0, .key = 0x11, .volume = 90 },
    /* STW_BOXY_IN */
    { .prg = 0, .key = 0x12, .volume = 90 },
    /* STW_BOXY_WAIT */
    { .prg = 0, .key = 0x13, .volume = 90 },
};

static WIPE_TYPE wipe_type = WIPE_TYPE_LOADING;
static int wipe_end_flag = TRUE;
static int loading_wipe_switch = FALSE;

/* WSHC_IN */
static WIPE_SCRATCH_TBL wipe_scratch_tbl_00[] = {
    { .frame = 0,  .data = 0    },
    { .frame = 4,  .data = 403  },
    { .frame = 11, .data = 144  },
    { .frame = 19, .data = 835  },
    { .frame = 27, .data = 547  },
    { .frame = 29, .data = 720  },
    { .frame = 35, .data = 489  },
    { .frame = 44, .data = 1267 },
    { .frame = 51, .data = 1008 },
    { .frame = 52, .data = 1152 },
    { .frame = 59, .data = 864  },
    { .frame = 67, .data = 1440 },
};

/* WSHC_LOOP */
static WIPE_SCRATCH_TBL wipe_scratch_tbl_01[] = {
    { .frame = 0,   .data = 1440 },
    { .frame = 17,  .data = 1728 },
    { .frame = 42,  .data = 1152 },
    { .frame = 58,  .data = 1584 },
    { .frame = 68,  .data = 1296 },
    { .frame = 84,  .data = 1584 },
    { .frame = 100, .data = 1296 },
    { .frame = 103, .data = 1440 },
    { .frame = 107, .data = 1296 },
    { .frame = 111, .data = 1440 },
    { .frame = 117, .data = 1296 },
    { .frame = 127, .data = 1584 },
    { .frame = 135, .data = 1440 },
};

/* WSHC_IN_MOVE */
static WIPE_SCRATCH_TBL wipe_scratch_tbl_02[] = {
    { .frame = 0,  .data = 0   },
    { .frame = 67, .data = 240 },
};

/* WSHC_LOOP_MOVE */
static WIPE_SCRATCH_TBL wipe_scratch_tbl_03[] = {
    { .frame = 0,   .data = 240 },
    { .frame = 10,  .data = 216 },
    { .frame = 26,  .data = 132 },
    { .frame = 30,  .data = 120 },
    { .frame = 39,  .data = 120 },
    { .frame = 43,  .data = 132 },
    { .frame = 55,  .data = 216 },
    { .frame = 67,  .data = 240 },
    { .frame = 72,  .data = 240 },
    { .frame = 78,  .data = 216 },
    { .frame = 94,  .data = 132 },
    { .frame = 100, .data = 120 },
    { .frame = 107, .data = 120 },
    { .frame = 115, .data = 132 },
    { .frame = 129, .data = 216 },
    { .frame = 135, .data = 240 },
};

/* WSHC_OUT */
static WIPE_SCRATCH_TBL wipe_scratch_tbl_04[] = {
    { .frame = 0,  .data = 576 },
    { .frame = 12, .data = 216 },
    { .frame = 24, .data = 72  },
    { .frame = 28, .data = 0   },
};

/* WSHC_OUT_MOVE */
static WIPE_SCRATCH_TBL wipe_scratch_tbl_05[] = {
    { .frame = 0,  .data = 240 },
    { .frame = 28, .data = 360 },
};

static WIPE_SCRATCH_CTRL wipe_scratch_ctrl[] = {
    /* WSHC_IN */
    { .frt_size = PR_ARRAYSIZE(wipe_scratch_tbl_00), .frt_pp = wipe_scratch_tbl_00 },
    /* WSHC_LOOP */
    { .frt_size = PR_ARRAYSIZE(wipe_scratch_tbl_01), .frt_pp = wipe_scratch_tbl_01 },
    /* WSHC_IN_MOVE */
    { .frt_size = PR_ARRAYSIZE(wipe_scratch_tbl_02), .frt_pp = wipe_scratch_tbl_02 },
    /* WSHC_LOOP_MOVE */
    { .frt_size = PR_ARRAYSIZE(wipe_scratch_tbl_03), .frt_pp = wipe_scratch_tbl_03 },
    /* WSHC_OUT */
    { .frt_size = PR_ARRAYSIZE(wipe_scratch_tbl_04), .frt_pp = wipe_scratch_tbl_04 },
    /* WSHC_OUT_MOVE */
    { .frt_size = PR_ARRAYSIZE(wipe_scratch_tbl_05), .frt_pp = wipe_scratch_tbl_05 },
};

static int ldmove_rate = 0;
static int ldrecode_rate = 0;
static int ldlogo_rate = 0;

static LDMAP ldmap[] = {
    /* LDMAP_TURN */
    {
        .spmmap = 0x4,
        .spamap = 0xc,
        .spamapP = -1,

        .spmHdl = NULL,
        .spaHdl = NULL,
        .spaHdlP = NULL,

        .frame_pp = &ldmove_rate,
        .frame_ppP = NULL,
    },
    /* LDMAP_LOGO */
    {
        .spmmap = 0x2,
        .spamap = 0x9,
        .spamapP = -1,

        .spmHdl = NULL,
        .spaHdl = NULL,
        .spaHdlP = NULL,

        .frame_pp = &ldlogo_rate,
        .frame_ppP = NULL,
    },
    /* LDMAP_RECODE */
    {
        .spmmap = 0x3,
        .spamap = 0xa,
        .spamapP = 0xb,

        .spmHdl = NULL,
        .spaHdl = NULL,
        .spaHdlP = NULL,

        .frame_pp = &ldrecode_rate,
        .frame_ppP = &ldmove_rate,
    },
    /* LDMAP_RECODE_LT */
    {
        .spmmap = 0x0,
        .spamap = 0x5,
        .spamapP = 0x6,

        .spmHdl = NULL,
        .spaHdl = NULL,
        .spaHdlP = NULL,

        .frame_pp = &ldrecode_rate,
        .frame_ppP = &ldmove_rate,
    },
    /* LDMAP_LABEL */
    {
        .spmmap = 0x1,
        .spamap = 0x7,
        .spamapP = 0x8,

        .spmHdl = NULL,
        .spaHdl = NULL,
        .spaHdlP = NULL,

        .frame_pp = &ldrecode_rate,
        .frame_ppP = &ldmove_rate,
    },
};

extern PR_SCENEHANDLE ldmap_hdl;

extern WIPE_PARA_STR wipe_para_str;
extern sceGsDrawEnv1 wipe_draw_env;

void wipeParaOutReq(void);

void wipeSndReq(SNDTAP_WIPE_ENUM req) {
    SNDTAP *sndtap_pp;

    if (req >= STW_MAX) {
        printf("WIPE SND REQ OVER!![%d]\n", req);
        return;
    }

    sndtap_pp = &sndtap_wipe[req];

    TapCt(TAPCT_SETEFFECTMODE, SD_REV_MODE_OFF, 0);
    TapCt(TAPCT_TAPVOLUME | 3, 0xf, sndtap_pp->volume);
    TapCt(TAPCT_TAPREQ    | 3, 0xf, sndtap_pp->prg + sndtap_pp->key * 256);
}

void wipeSndStop(void) {
    TapCt(TAPCT_TAPSTOP, 0xf, TAPCT_NONE);
}

void wipeSndFileTrans(void) {
    TapCt(TAPCT_BDSPUTRANS | 3, (int)cmnfGetFileAdrs(74), cmnfGetFileSize(74));
    TapCt(TAPCT_HDIOPTRANS | 3, (int)cmnfGetFileAdrs(73), cmnfGetFileSize(73));

    TapCt(TAPCT_SETMASTERVOL, PR_CONCAT(0x3fff, 0x3fff), TAPCT_NONE);
    TapCt(TAPCT_SETVOLUME,    PR_CONCAT(0x3fff, 0x3fff), TAPCT_NONE);

    TapCt(TAPCT_SETEFFECTMODE, SD_REV_MODE_OFF, 0x1fff);
    TapCt(TAPCT_SETEFFECTVOL,  PR_CONCAT(0x3fff, 0x3fff), TAPCT_NONE);
}

static void LocalBufCopy(int disp) {
    sceGsMoveImage mi;
    short          dbp = 0;
    short          sbp = 0;

    switch (disp) {
    case 0:
        sbp = DrawGetTbpPos(DNUM_SHOW);
        dbp = DrawGetTbpPos(DNUM_VRAM2);
        break;
    case 1:
        sbp = DrawGetTbpPos(DNUM_DRAW);
        dbp = DrawGetTbpPos(DNUM_VRAM2);
        break;
    case -1:
        sbp = DrawGetTbpPos(DNUM_VRAM2);
        dbp = DrawGetTbpPos(DNUM_SHOW);
        break;
    case -2:
        sbp = DrawGetTbpPos(DNUM_VRAM2);
        dbp = DrawGetTbpPos(DNUM_DRAW);
        break;
    default:
        printf(" local copy error!!\n");
        break;
    }

    GGsSetLocalMoveImage(&mi, dbp, 10, 0, 0, 0, sbp, 10, 0, 0, 0, 640, 224, 0);
    FlushCache(WRITEBACK_DCACHE);

    GGsExecLocalMoveImage(&mi);
    sceGsSyncPath(0, 0);
}

int wipeTimeGetInWait(int time, WSHC_ENUM wshc_enum) {
    WIPE_SCRATCH_CTRL *wscc_pp;
    int maxt;
    int ret;
    int i;
    int stp, endp;
    int lngT, lngN;

    wscc_pp = &wipe_scratch_ctrl[wshc_enum];
    maxt = wscc_pp->frt_pp[wscc_pp->frt_size - 1].frame;
    maxt = time % maxt;

    endp = -1;
    stp  = 0;

    for (i = 0; i < wscc_pp->frt_size; i++) {
        if (wscc_pp->frt_pp[i].frame == maxt) {
            stp = i;
            endp = i;
            break;
        }

        if (maxt > wscc_pp->frt_pp[i].frame) {
            stp = i;
        }

        if (maxt < wscc_pp->frt_pp[i].frame) {
            endp = i;
            break;
        }
    }

    if (endp == -1) {
        endp = stp;
    }

    lngT = wscc_pp->frt_pp[endp].frame - wscc_pp->frt_pp[stp].frame;
    lngN = maxt - wscc_pp->frt_pp[stp].frame;
    if (lngT == 0) {
        ret = wscc_pp->frt_pp[stp].data;
    } else {
        ret = wscc_pp->frt_pp[endp].data - wscc_pp->frt_pp[stp].data;
        ret = ret * lngN / lngT + wscc_pp->frt_pp[stp].data;
    }

    return ret;
}

static void lddisp_init_pr(void) {
    LDMAP *ldmap_pp;
    int    i;

    PrSetFrameRate(60.0f);
    ldmap_hdl = PrInitializeScene(&DBufDc.draw01, "wipe recode\0", -1);

    ldmap_pp = ldmap;
    for (i = 0; i < PR_ARRAYSIZEU(ldmap); i++, ldmap_pp++) {
        ldmap_pp->spmHdl = PrInitializeModel(cmnfGetFileAdrs(ldmap_pp->spmmap), ldmap_hdl);

        /* Initialize animation */
        if (ldmap_pp->spamap >= 0) {
            ldmap_pp->spaHdl = PrInitializeAnimation(cmnfGetFileAdrs(ldmap_pp->spamap));

            PrLinkAnimation(ldmap_pp->spmHdl, ldmap_pp->spaHdl);
            PrAnimateModel(ldmap_pp->spmHdl, *ldmap_pp->frame_pp);
        }

        /* Initialize position animation */
        if (ldmap_pp->spamapP >= 0) {
            ldmap_pp->spaHdlP = PrInitializeAnimation(cmnfGetFileAdrs(ldmap_pp->spamapP));

            PrLinkPositionAnimation(ldmap_pp->spmHdl, ldmap_pp->spaHdlP);
            PrAnimateModelPosition(ldmap_pp->spmHdl, *ldmap_pp->frame_ppP);
        }

        PrHideModel(ldmap_pp->spmHdl);
    }

    PrPreprocessSceneModel(ldmap_hdl);
}

static void lddisp_draw_quit(int drapP) {
    LDMAP *ldmap_pp;
    float  men_tmp;
    int    i;

    men_tmp = PrGetMendererRatio();
    PrSetMendererRatio(0.0f);

    PrSetSceneFrame(ldmap_hdl, *DrawGetFrameP(drapP));
    PrSetSceneEnv(ldmap_hdl, DrawGetDrawEnvP(drapP));

    PrRender(ldmap_hdl);
    PrWaitRender();

    ldmap_pp = ldmap;
    for (i = 0; i < PR_ARRAYSIZEU(ldmap); i++, ldmap_pp++) {
        if (ldmap_pp->spamap >= 0) {
            PrUnlinkAnimation(ldmap_pp->spmHdl);
            PrCleanupAnimation(ldmap_pp->spaHdl);
        }
        if (ldmap_pp->spamapP >= 0) {
            PrUnlinkPositionAnimation(ldmap_pp->spmHdl);
            PrCleanupAnimation(ldmap_pp->spaHdlP);
        }

        PrCleanupModel(ldmap_pp->spmHdl);
    }

    PrCleanupScene(ldmap_hdl);
    PrSetMendererRatio(men_tmp);
}

static void lddisp_draw_on(LDMAP_ENUM ldmap_enum) {
    PrShowModel(ldmap[ldmap_enum].spmHdl, NULL);
}

void WipeLoadInDisp(void *x) {
    int timer;
    int firstf;
    int ttmp;

    firstf = TRUE;

    wipeSndReq(STW_TURN_IN);
    TimeCallbackTimeSetChan(TCBK_CHANNEL_WIPE, 0);

    while (1) {
        timer = TimeCallbackTimeGetChan(TCBK_CHANNEL_WIPE);

        if (timer >= 67) {
            ldmove_rate   = wipeTimeGetInWait(timer - 67, WSHC_LOOP_MOVE);
            ldrecode_rate = wipeTimeGetInWait(timer - 67, timer >= 67);
        } else {
            ldmove_rate   = wipeTimeGetInWait(timer, WSHC_IN_MOVE);
            ldrecode_rate = wipeTimeGetInWait(timer, timer >= 67);
        }

        if (!loading_wipe_switch) {
            ldmove_rate = 480 - ldmove_rate;
        }

        ldlogo_rate = timer % 360;

        lddisp_init_pr();
        lddisp_draw_on(LDMAP_TURN);
        lddisp_draw_on(LDMAP_LOGO);
        lddisp_draw_on(LDMAP_RECODE);
        lddisp_draw_on(LDMAP_RECODE_LT);
        lddisp_draw_on(LDMAP_LABEL);
        lddisp_draw_quit(DNUM_DRAW);

        wipe_end_flag = FALSE;
        timer++;

        if (timer > 67) {
            if (firstf) {
                firstf = FALSE;

                wipeSndReq(STW_TURN_WAIT);
                wipe_end_flag = TRUE;

                TimeCallbackTimeSetChan(TCBK_CHANNEL_WIPE, 67);
            } else {
                ttmp = (timer - 67) % 135;

                if (ttmp == 134 || ttmp == 0) {
                    wipe_end_flag = TRUE;
                }
            }
        }

        MtcWait(1);
    }
}

void WipeLoadInDispNR(void) {
    int           timer;
    sceGifPacket  gifpk;
    FADE_MAKE_STR fade_make_str = {
        .r = 128, .g = 128, .b = 128, .alp = 128
    };
    VCLR_PARA     vclr_para = {};
    float         scl;

    timer = 0;

    while (timer != 30) {
        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);
        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_END);

        ldmove_rate = 0;
        ldrecode_rate = 0;
        if (!loading_wipe_switch) {
            ldmove_rate = 480;
        }

        lddisp_init_pr();
        lddisp_draw_on(LDMAP_TURN);
        lddisp_draw_on(LDMAP_LABEL);
        lddisp_draw_quit(DNUM_END);

        LocalBufCopy(-2);

        fade_make_str.alp = (timer * 128) / 30;
        scl = ((30 - timer) / 30.0f) + ((30 - timer) / 30.0f) + 1.0;

        CmnGifADPacketMake(&gifpk, DrawGetFrameP(DNUM_DRAW));
        UG_FadeDisp2(&fade_make_str, &gifpk, DrawGetFrameP(DNUM_END), scl);
        CmnGifADPacketMakeTrans(&gifpk);

        MtcWait(1);
        timer++;
    }
}

void WipeLoadOutDispNR(void) {
    int           timer;
    sceGifPacket  gifpk;
    FADE_MAKE_STR fade_make_str = {
        .r = 128, .g = 128, .b = 128, .alp = 128
    };
    VCLR_PARA     vclr_para = {};
    float         scl;

    timer = 0;

    while (timer != 30) {
        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);
        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_VRAM2);

        ldmove_rate = 360;
        ldrecode_rate = 0;

        if (!loading_wipe_switch) {
            ldmove_rate = 0;
        }

        lddisp_init_pr();
        lddisp_draw_on(LDMAP_TURN);
        lddisp_draw_on(LDMAP_LABEL);
        lddisp_draw_quit(DNUM_VRAM2);

        fade_make_str.alp = (3840 + (timer * -128)) / 30;
        scl = (timer / 30.0f) + (timer / 30.0f) + 1.0;

        CmnGifADPacketMake(&gifpk, DrawGetFrameP(DNUM_DRAW));
        UG_FadeDisp2(&fade_make_str, &gifpk, DrawGetFrameP(DNUM_VRAM2), scl);
        CmnGifADPacketMakeTrans(&gifpk);

        MtcWait(1);
        timer++;
    }
}

static void WipeLoadOutDisp(void *x) {
    VCLR_PARA vclr_para = {};
    int       timer;

    wipeSndReq(STW_TURN_OUT);
    TimeCallbackTimeSetChan(TCBK_CHANNEL_WIPE, 0);

    do {
        timer = TimeCallbackTimeGetChan(TCBK_CHANNEL_WIPE);

        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_VRAM2);
        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);

        ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));

        CmnGifClear();
        CmnGifFlush();
        LocalBufCopy(1);

        ldmove_rate = wipeTimeGetInWait(timer, WSHC_OUT_MOVE);
        ldrecode_rate = wipeTimeGetInWait(timer, WSHC_OUT);

        if (!loading_wipe_switch) {
            ldmove_rate = 480 - ldmove_rate;
        }

        ldlogo_rate = timer % 360;

        lddisp_init_pr();
        lddisp_draw_on(LDMAP_TURN);
        lddisp_draw_on(LDMAP_LOGO);
        lddisp_draw_on(LDMAP_RECODE);
        lddisp_draw_on(LDMAP_RECODE_LT);
        lddisp_draw_on(LDMAP_LABEL);
        lddisp_draw_quit(DNUM_DRAW);

        MtcWait(1);
        timer++;
    } while (timer < 28);

    WipeLoadOutDispNR();
    wipe_end_flag = TRUE;
    MtcExit();
}

void WipeInReq(void) {
    loading_wipe_switch ^= 1;

    wipe_end_flag = FALSE;
    wipe_type = WIPE_TYPE_LOADING;

    LocalBufCopy(0);
    MtcKill(MTC_TASK_WIPECTRL);

    WipeLoadInDispNR();
    MtcExec(WipeLoadInDisp, MTC_TASK_WIPECTRL);
}

void WipeLoadInDispSame(void *x) {
    SPR_PRIM spr_prim = {
        .x = 2048, .y = 2048,
        .scalex = 256, .scaley = 256,
        .u = 0, .v = 0,
        .w = 640, .h = 224,
    };
    SPR_DAT  spr_dat  = {};

    spr_dat.GsTex0 = SCE_GS_SET_TEX0(TBP_VRAM_DRAW2, 10, 0, 10, 8, 0, 0, 0, 0, 0, 0, 0);

    wipe_end_flag = TRUE;
    SprInit();

    while (1) {
        SprClear();
        SprPackSet(&spr_dat);

        SprDispZABnclr();

        SprDisp(&spr_prim);
        SprFlash();

        MtcWait(1);
    }
}

void WipeInReqSame(void) {
    wipe_type = WIPE_TYPE_SAME;
    wipe_end_flag = FALSE;

    MtcExec(WipeLoadInDispSame, MTC_TASK_WIPECTRL);
    LocalBufCopy(0);
}

void WipeOutReq(void) {
    if (wipe_type == WIPE_TYPE_SAME) {
        MtcKill(MTC_TASK_WIPECTRL);
        wipe_end_flag = TRUE;
    } else if (wipe_type == WIPE_TYPE_YES_NO) {
        MtcKill(MTC_TASK_WIPECTRL);
        wipe_end_flag = TRUE;
    } else if (wipe_type == WIPE_TYPE_PARA) {
        wipeParaOutReq();
    } else if (wipe_type == WIPE_TYPE_BOXY) {
        MtcKill(MTC_TASK_WIPECTRL);
        wipe_end_flag = TRUE;
    } else if (wipe_type == WIPE_TYPE_BOXY_WAIT) {
        wipeSndStop();
        MtcKill(MTC_TASK_WIPECTRL);
        wipe_end_flag = TRUE;
    } else {
        wipe_end_flag = FALSE;
        MtcExec(WipeLoadOutDisp, MTC_TASK_WIPECTRL);        
    }
}

int WipeEndCheck(void) {
    return wipe_end_flag;
}

void WipeYesNoDispTask(void *x) {
    void *scn_hdl;
    void *spm_hdl;
    float men_tmp;

    SPR_PRIM spr_prim = {
        .x = 2048, .y = 2048,
        .scalex = 256, .scaley = 256,
        .u = 0, .v = 0,
        .w = 640, .h = 224,
    };

    SPR_DAT spr_dat = {};

    spr_dat.GsTex0 = SCE_GS_SET_TEX0(TBP_VRAM_DRAW2, 10, 0, 10, 8, 0, 0, 0, 0, 0, 0, 0);;

    SprInit();

    while (1) {
        SprClear();
        SprPackSet(&spr_dat);

        SprDispZBnclr();
        SprSetColor(64, 64, 64, 128);

        SprDisp(&spr_prim);
        SprFlash();

        men_tmp = PrGetMendererRatio();

        PrSetMendererRatio(0.0f);
        PrSetFrameRate(60.0f);

        scn_hdl = PrInitializeScene(&DBufDc.draw01, "wipe", -1);
        spm_hdl = PrInitializeModel(cmnfGetFileAdrs(47), scn_hdl);
        PrShowModel(spm_hdl, NULL);
        PrSetSceneEnv(scn_hdl, DrawGetDrawEnvP(DNUM_DRAW));

        PrPreprocessSceneModel(scn_hdl);
        PrRender(scn_hdl);
        PrWaitRender();

        PrCleanupModel(spm_hdl);
        PrCleanupScene(scn_hdl);
        PrSetMendererRatio(men_tmp);

        MtcWait(1);
    }
}

void wipeYesNoDispReq(void) {
    printf("wipe yes/no req\n");

    wipe_type = WIPE_TYPE_YES_NO;
    wipe_end_flag = TRUE;
    wipeSndReq(STW_YESNO);

    MtcExec(WipeYesNoDispTask, MTC_TASK_WIPECTRL);
    LocalBufCopy(0);
}

void wipeYesNoDispEnd(void) {
    MtcKill(MTC_TASK_WIPECTRL);
}

static int wipe_para_spa_type = 0;
static VCLR_PARA vclr_para_disp = { 255, 255, 255, 0 };

static void WipeInitPrDataPara(sceGsFrame *fr_pp) {
    int fbp;

    PrSetFrameRate(60.0f);
    
    fbp = DrawGetFbpPos(DNUM_VRAM2);
    wipe_para_str.scene_hdl = PrInitializeScene(&DBufDc.draw01, "wipe para", fbp);
    wipe_para_str.spm_hdl   = PrInitializeModel(cmnfGetFileAdrs(75), wipe_para_str.scene_hdl);

    if (wipe_para_spa_type) { /* PaRappa "bye-bye" waving animation */
        wipe_para_str.spa_hdl = PrInitializeAnimation(cmnfGetFileAdrs(76));
    } else {
        wipe_para_str.spa_hdl = PrInitializeAnimation(cmnfGetFileAdrs(77));
    }

    PrLinkAnimation(wipe_para_str.spm_hdl, wipe_para_str.spa_hdl);

    wipe_para_str.spc_hdl = PrInitializeCamera(cmnfGetFileAdrs(78));

    PrSelectCamera(wipe_para_str.spc_hdl, wipe_para_str.scene_hdl);
    PrSetSceneFrame(wipe_para_str.scene_hdl, *fr_pp);
}

static void WipeQuitPrDataPara(void) {
    PrUnlinkAnimation(wipe_para_str.spm_hdl);
    PrCleanupAnimation(wipe_para_str.spa_hdl);

    PrCleanupModel(wipe_para_str.spm_hdl);
    PrCleanupCamera(wipe_para_str.spc_hdl);
    PrCleanupScene(wipe_para_str.scene_hdl);
}

static void WipeDispPrDataPara(int frame, sceGsDrawEnv1 *envPP) {
    float men_tmp;
    
    men_tmp = PrGetMendererRatio();
    PrSetMendererRatio(0.0f);

    PrSetSceneEnv(wipe_para_str.scene_hdl, envPP);
    PrPreprocessSceneModel(wipe_para_str.scene_hdl);

    PrAnimateSceneCamera(wipe_para_str.scene_hdl, frame);
    PrAnimateModel(wipe_para_str.spm_hdl, frame);
    PrShowModel(wipe_para_str.spm_hdl, NULL);

    PrRender(wipe_para_str.scene_hdl);
    PrWaitRender();

    PrSetMendererRatio(men_tmp);
}

void WipeParaColorSet(u_char r, u_char g, u_char b) {
    vclr_para_disp.r = r;
    vclr_para_disp.g = g;
    vclr_para_disp.b = b;
}

void WipeEnd(void) {
    wipe_end_flag = TRUE;
  
    MtcKill(MTC_TASK_WIPECTRL);
    wipeSndStop();
}

static void WipeParaInDisp(void *x) {
    sceGifPacket gifP;
    VCLR_PARA    vclr_para = {};
    
    SPR_PRIM     spr_prim  = {
        .x = 2048, .y = 2048,
        .scalex = 256, .scaley = 256,
        .u = 0, .v = 0,
        .w = 640, .h = 224,
    };
    SPR_DAT      spr_dat   = {};
    int          timer;

    spr_dat.GsTex0 = SCE_GS_SET_TEX0(TBP_VRAM_DRAW2, 10, 0, 10, 8, 1, 1, 0, 0, 0, 0, 0);

    SprInit();
    TimeCallbackTimeSetChan(TCBK_CHANNEL_WIPE, 0);

    while (1) {
        timer = TimeCallbackTimeGetChan(TCBK_CHANNEL_WIPE);
        if (timer > 60) {
            timer = (timer - 60) % 180 + 60;
        }

        CmnGifFlush();
        CmnGifClear();

        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);
        DrawVramClear(&vclr_para_disp, 0, 0, DNUM_NON, DNUM_DRAW);

        wipe_draw_env = *DrawGetDrawEnvP(DNUM_VRAM2);
        wipe_draw_env.frame1.FBMSK = 0x00ffffff; /* Mask alpha */

        CmnGifADPacketMake(&gifP, &wipe_draw_env.frame1);
        sceGifPkAddGsAD(&gifP, SCE_GS_FRAME_1, *(u_long*)&wipe_draw_env.frame1);
        sceGifPkAddGsAD(&gifP, SCE_GS_XYOFFSET_1, *(u_long*)&wipe_draw_env.xyoffset1);
        sceGifPkAddGsAD(&gifP, SCE_GS_SCISSOR_1, *(u_long*)&wipe_draw_env.scissor1);
        sceGifPkAddGsAD(&gifP, SCE_GS_TEST_1, SCE_GS_SET_TEST(1, 0, 0, 1, 0, 0, 1, 1));
        sceGifPkAddGsAD(&gifP, SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0));
        sceGifPkAddGsAD(&gifP, SCE_GS_RGBAQ, 0);
        sceGifPkAddGsAD(&gifP, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(0), GS_Y_COORD(0), 1));
        sceGifPkAddGsAD(&gifP, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(37888, 34560, 1));
        CmnGifADPacketMakeTrans(&gifP);

        WipeInitPrDataPara(&wipe_draw_env.frame1);
        WipeDispPrDataPara(timer, &wipe_draw_env);
        WipeQuitPrDataPara();

        ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));

        SprClear();
        SprPackSet(&spr_dat);
        SprDispAcheck(TRUE);
        SprDisp(&spr_prim);
        SprFlash();

        if (timer > 60) {
            wipe_end_flag = TRUE;
        }
        if (timer > 238) {
            wipe_end_flag = 2;
        }

        MtcWait(1);
    }
}

static void WipeParaInDispMove(void *x) {
    VCLR_PARA vclr_para = {};

    SPR_PRIM  spr_prim  = {
        .x = 2048, .y = 2048,
        .scalex = 256, .scaley = 256,
        .u = 0, .v = 0,
        .w = 640, .h = 224,
    };
    SPR_DAT   spr_dat   = {};
    int       timer;

    spr_dat.GsTex0 = SCE_GS_SET_TEX0(TBP_VRAM_DRAW2, 10, 0, 10, 8, 1, 1, 0, 0, 0, 0, 0);

    SprInit();
    TimeCallbackTimeSetChan(TCBK_CHANNEL_WIPE, 0);

    while (1) {
        timer = TimeCallbackTimeGetChan(TCBK_CHANNEL_WIPE);
        if (timer > 60) {
            timer = (timer - 60) % 180 + 60;
        }

        CmnGifFlush();
        CmnGifClear();

        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);
        DrawVramClear(&vclr_para_disp, 0, 0, DNUM_NON, DNUM_VRAM2);

        wipe_draw_env = *DrawGetDrawEnvP(DNUM_VRAM2);
        wipe_draw_env.frame1.FBMSK = 0x00ffffff; /* Mask alpha */

        WipeInitPrDataPara(&wipe_draw_env.frame1);
        WipeDispPrDataPara(timer, &wipe_draw_env);
        WipeQuitPrDataPara();

        ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));

        SprClear();
        SprPackSet(&spr_dat);
        SprDispAcheck(FALSE);
        SprDisp(&spr_prim);
        SprFlash();

        if (timer > 60) {
            wipe_end_flag = TRUE;
        }
        if (timer > 238) {
            wipe_end_flag = 2;
        }

        MtcWait(1);
    }
}

static void WipeParaOutDisp(void *x) {
    VCLR_PARA vclr_para = {};

    SPR_PRIM  spr_prim  = {
        .x = 2048, .y = 2048,
        .scalex = 256, .scaley = 256,
        .u = 0, .v = 0,
        .w = 640, .h = 224,
    };
    SPR_DAT   spr_dat   = {};
    int       timer     = 0;

    spr_dat.GsTex0 = SCE_GS_SET_TEX0(TBP_VRAM_DRAW2, 10, 0, 10, 8, 1, 1, 0, 0, 0, 0, 0);

    SprInit();

    while (1) {
        CmnGifFlush();
        CmnGifClear();

        LocalBufCopy(1);

        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_ZBUFF);
        DrawVramClear(&vclr_para_disp, 0, 0, DNUM_NON, DNUM_VRAM2);

        wipe_draw_env = *DrawGetDrawEnvP(DNUM_VRAM2);
        wipe_draw_env.frame1.FBMSK = 0x00ffffff; /* Mask alpha */

        WipeInitPrDataPara(&wipe_draw_env.frame1);
        WipeDispPrDataPara(60 - timer, &wipe_draw_env);
        WipeQuitPrDataPara();

        ChangeDrawArea(DrawGetDrawEnvP(DNUM_DRAW));

        SprClear();
        SprPackSet(&spr_dat);

        SprDispAcheck(FALSE);

        SprDisp(&spr_prim);
        SprFlash();

        DrawVramClear(&vclr_para, 0, 0, DNUM_NON, DNUM_VRAM2);

        timer++;
        if (timer > 60) {
            timer = 60;
            wipe_end_flag = TRUE;
            MtcExit();
        }

        MtcWait(1);
    }
}

void wipeParaInReq(void) {
    wipe_end_flag = FALSE;
    wipe_type = WIPE_TYPE_PARA;
    wipeSndReq(STW_PARAtoMINI);

    wipe_para_spa_type = 0;
    MtcExec(WipeParaInDisp, MTC_TASK_WIPECTRL);

    LocalBufCopy(0);
}

/* Used in the demos - unused on the final version */
void wipeParaInReqByeBye(void) {
    wipe_end_flag = FALSE;
    wipe_type = WIPE_TYPE_PARA;
    wipeSndReq(STW_PARAtoMINI);

    wipe_para_spa_type = 1;
    MtcExec(WipeParaInDisp, MTC_TASK_WIPECTRL);

    LocalBufCopy(0);
}

void wipeParaInReqMove(void) {
    wipe_end_flag = FALSE;
    wipe_type = WIPE_TYPE_PARA;
    wipeSndReq(STW_PARAtoMINI);

    wipe_para_spa_type = 0;
    MtcExec(WipeParaInDispMove, MTC_TASK_WIPECTRL);
}

void wipeParaOutReq(void) {
    wipe_end_flag = FALSE;
    wipeSndReq(STW_PARAtoBIG);

    MtcExec(WipeParaOutDisp, MTC_TASK_WIPECTRL);
}

static JIMAKU_DAT jimaku_dat0[] = {
    {
        .starTime = 114,
        .endTime = 230,
        .txtData = {
            "Come on Parappa,@relax,",
            "さあ、パラッパ　リラックスして",
        }
    },
    {
        .starTime = 230,
        .endTime = 318,
        .txtData = {
            "it's practice time",
            "練習タイムだよ",
        }
    },
};

JIMAKU_STR jimaku_str[] = {
    { .size = PR_ARRAYSIZE(jimaku_dat0), .jimaku_dat_pp = jimaku_dat0 },
};

static PR_SCENEHANDLE ldmap_hdl;

WIPE_PARA_STR wipe_para_str = {};
sceGsDrawEnv1 wipe_draw_env = {};

static void WipeBoxyInDisp(void *x) {
    void *scn_hdl;
    void *scn2_hdl;
    void *spm_hdl;
    void *spa_hdl;
    void *spm2_hdl;
    void *spa2_hdl;
    void *spm_subt_hdl;
    void *spc_hdl;

    float men_tmp;
    int   animate_frame = 0;

    SPR_PRIM spr_prim = {
        .x = 2048, .y = 2048,
        .scalex = 256, .scaley = 256,
        .u = 0, .v = 0,
        .w = 640, .h = 224,
    };
    SPR_DAT  spr_dat  = {};

    spr_dat.GsTex0 = SCE_GS_SET_TEX0(TBP_VRAM_DRAW2, 10, 0, 10, 8, 0, 0, 0, 0, 0, 0, 0);

    Tim2Trans(cmnfGetFileAdrs(79));
    SprInit();

    PrSetFrameRate(60.0f);

    while (1) {
        men_tmp = PrGetMendererRatio();

        PrSetMendererRatio(0.0f);
        PrSetFrameRate(60.0f);

        scn2_hdl = PrInitializeScene(&DBufDc.draw01, "wipe boxy 2", -1);
        spm2_hdl = PrInitializeModel(cmnfGetFileAdrs(40), scn2_hdl);
        spa2_hdl = PrInitializeAnimation(cmnfGetFileAdrs(60));

        PrLinkAnimation(spm2_hdl, spa2_hdl);
        PrAnimateModel(spm2_hdl, animate_frame);
        PrShowModel(spm2_hdl, NULL);

        PrSelectCamera(PrInitializeCamera(cmnfGetFileAdrs(71)), scn2_hdl);

        PrSetSceneFrame(scn2_hdl, *DrawGetFrameP(DNUM_DRAW));
        PrSetSceneEnv(scn2_hdl, DrawGetDrawEnvP(DNUM_DRAW));

        PrPreprocessSceneModel(scn2_hdl);
        PrRender(scn2_hdl);
        PrWaitRender();

        PrUnlinkAnimation(spm2_hdl);
        PrCleanupAnimation(spa2_hdl);
        PrCleanupModel(spm2_hdl);
        PrCleanupScene(scn2_hdl);

        if (animate_frame < 120) {
            u_char alp = ((animate_frame * 128) / 120);
            alp = 128 - alp;

            SprClear();
            SprDispAlphaSet();
            SprPackSet(&spr_dat);

            SprDispZABnclr();
            SprSetColor(128, 128, 128, alp);

            SprDispAlp(&spr_prim);
            SprFlash();
        }

        scn_hdl = PrInitializeScene(&DBufDc.draw01, "wipe boxy", -1);
        spm_hdl = PrInitializeModel(cmnfGetFileAdrs(15), scn_hdl);
        spa_hdl = PrInitializeAnimation(cmnfGetFileAdrs(54));

        PrLinkAnimation(spm_hdl, spa_hdl);
        PrAnimateModel(spm_hdl, animate_frame);
        PrShowModel(spm_hdl, NULL);

        spc_hdl = PrInitializeCamera(cmnfGetFileAdrs(71));
        PrSelectCamera(spc_hdl, scn_hdl);

        PrSetSceneFrame(scn_hdl, *DrawGetFrameP(DNUM_DRAW));

        spm_subt_hdl = PrInitializeModel(cmnfGetFileAdrs(43), scn_hdl);
        if (game_status.subtitle == SUBTITLE_ON) {
            PrShowModel(spm_subt_hdl, NULL);
        } else {
            PrHideModel(spm_subt_hdl);
        }

        PrSetSceneEnv(scn_hdl, DrawGetDrawEnvP(DNUM_DRAW));

        PrPreprocessSceneModel(scn_hdl);
        PrRender(scn_hdl);
        PrWaitRender();

        PrUnlinkAnimation(spm_hdl);
        PrCleanupAnimation(spa_hdl);
        PrCleanupModel(spm_hdl);
        PrCleanupModel(spm_subt_hdl);
        PrCleanupCamera(spc_hdl);
        PrCleanupScene(scn_hdl);
        PrSetMendererRatio(men_tmp);

        if (game_status.subtitle == SUBTITLE_ON) {
            /* Print initial practice mode subtitles */
            SubtCtrlPrintBoxyWipe(jimaku_str, 0, animate_frame, game_status.language_type, cmnfGetFileAdrs(80));
        }

        animate_frame++;

        if (animate_frame > 400) {
            animate_frame = 400;
            wipe_end_flag = TRUE;
        }

        MtcWait(1);
    }
}

void wipeBoxyInReq(void) {
    wipe_end_flag = FALSE;
    wipe_type = WIPE_TYPE_BOXY;
    wipeSndReq(STW_BOXY_IN);

    LocalBufCopy(0);
    MtcExec(WipeBoxyInDisp, MTC_TASK_WIPECTRL);
}

static void WipeBoxyWaitDisp(void *x) {
    void *scn_hdl;
    void *spm_hdl;
    void *spa_hdl;
    void *spm2_hdl;
    void *spa2_hdl;
    void *spm_subt_hdl;
    void *spc_hdl;

    float men_tmp;
    int   animate_frame;

    animate_frame = 0;
    PrSetFrameRate(60.0f);

    while (1) {
        men_tmp = PrGetMendererRatio();
        PrSetMendererRatio(0.0f);
        PrSetFrameRate(60.0f);

        scn_hdl = PrInitializeScene(&DBufDc.draw01, "wipe boxy wait", -1);
        spm_hdl = PrInitializeModel(cmnfGetFileAdrs(15), scn_hdl);
        spa_hdl = PrInitializeAnimation(cmnfGetFileAdrs(53));

        PrLinkAnimation(spm_hdl, spa_hdl);
        PrAnimateModel(spm_hdl, animate_frame);
        PrShowModel(spm_hdl, NULL);

        spm2_hdl = PrInitializeModel(cmnfGetFileAdrs(40), scn_hdl);
        spa2_hdl = PrInitializeAnimation(cmnfGetFileAdrs(59));

        PrLinkAnimation(spm2_hdl, spa2_hdl);
        PrAnimateModel(spm2_hdl, animate_frame);
        PrShowModel(spm2_hdl, NULL);

        spc_hdl = PrInitializeCamera(cmnfGetFileAdrs(71));

        PrSelectCamera(spc_hdl, scn_hdl);
        PrSetSceneFrame(scn_hdl, *DrawGetFrameP(DNUM_DRAW));

        spm_subt_hdl = PrInitializeModel(cmnfGetFileAdrs(43), scn_hdl);
        if (game_status.subtitle == SUBTITLE_ON) {
            PrShowModel(spm_subt_hdl, NULL);
        } else {
            PrHideModel(spm_subt_hdl);
        }

        PrSetSceneEnv(scn_hdl, DrawGetDrawEnvP(DNUM_DRAW));
        PrPreprocessSceneModel(scn_hdl);

        PrRender(scn_hdl);
        PrWaitRender();

        PrUnlinkAnimation(spm_hdl);
        PrCleanupAnimation(spa_hdl);

        PrCleanupModel(spm_hdl);

        PrUnlinkAnimation(spm2_hdl);
        PrCleanupAnimation(spa2_hdl);

        PrCleanupModel(spm2_hdl);
        PrCleanupModel(spm_subt_hdl);
        PrCleanupCamera(spc_hdl);
        PrCleanupScene(scn_hdl);

        PrSetMendererRatio(men_tmp);

        animate_frame++;
        animate_frame %= 96;

        if (animate_frame < 2) {
            wipe_end_flag = TRUE;
        } else {
            wipe_end_flag = FALSE;
        }

        MtcWait(1);
    }
}

void wipeBoxyWaitReq(void) {
    wipe_end_flag = FALSE;
    wipe_type = WIPE_TYPE_BOXY_WAIT;
    wipeSndReq(STW_BOXY_WAIT);

    MtcExec(WipeBoxyWaitDisp, MTC_TASK_WIPECTRL);
}
