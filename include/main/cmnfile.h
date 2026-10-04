#ifndef CMNFILE_H
#define CMNFILE_H

#include "common.h"

typedef enum {
    CMN_NONE,  /* None */
    CMN_VRAM,  /* TIM2 textures */
    CMN_SND,   /* Sounds */
    CMN_ONMEM, /* Models, animations, etc. to store in the memory pool */
    CMN_MAX    /* N/A */
} CMN_FILE_TYPE_ENUM;

typedef struct { // 0x10
    /* 0x0 */ int fnum;
    /* 0x4 */ int ftype;
    /* 0x8 */ int f_size;
    /* 0xc */ int pad;
    /* 0x10 */ int adr[0];
} CMN_FILE_STR;

/*
 * Files of the ONMEM category, for cmnfGetFileAdrs/cmnfGetFileSize. The names
 * come from the files themselves: an SPM carries its model name, an SPA or SPC
 * "<model>__<animation>" (a trailing _p marks a position animation).
 */
typedef enum {
    CMNF_SPM_HIRECORD,             /*  0: hirecord_SPXm.spm */
    CMNF_SPM_LABEL,                /*  1: label_SPXm.spm */
    CMNF_SPM_LOADING,              /*  2: loading_SPXm.spm */
    CMNF_SPM_RECORD,               /*  3: record_SPXm.spm */
    CMNF_SPM_TURN,                 /*  4: turn_SPXm.spm */
    CMNF_SPA_HIRECORD_LOAD0,       /*  5: hirecord_SPXm__load0_ */
    CMNF_SPA_HIRECORD_LOAD0_P,     /*  6: hirecord_SPXm__load0__p */
    CMNF_SPA_LABEL_LOAD0,          /*  7: label_SPXm__load0_ */
    CMNF_SPA_LABEL_LOAD0_P,        /*  8: label_SPXm__load0__p */
    CMNF_SPA_LOADING_LOAD0,        /*  9: loading_SPXm__load0_ */
    CMNF_SPA_RECORD_LOAD0,         /* 10: record_SPXm__load0_ */
    CMNF_SPA_RECORD_LOAD0_P,       /* 11: record_SPXm__load0__p */
    CMNF_SPA_TURN_LOAD0,           /* 12: turn_SPXm__load0_ */
    CMNF_SPM_BAR_BAR,              /* 13: bar_bar.spm */
    CMNF_SPM_BXY_BAR,              /* 14: bxy_bar.spm */
    CMNF_SPM_BXY_BXY,              /* 15: bxy_bxy_SPXt.spm */
    CMNF_SPM_CAP_LL2,              /* 16: cap_ll2_SPXm.spm */
    CMNF_SPM_CAP_LR2,              /* 17: cap_lr2_SPXm.spm */
    CMNF_SPM_CAP_ML2,              /* 18: cap_ml2_SPXm.spm */
    CMNF_SPM_CAP_MR2,              /* 19: cap_mr2_SPXm.spm */
    CMNF_SPM_CAP_RL2,              /* 20: cap_rl2_SPXm.spm */
    CMNF_SPM_CAP_RR2,              /* 21: cap_rr2_SPXm.spm */
    CMNF_SPM_COUNTER,              /* 22: counter_SPXm.spm */
    CMNF_SPM_CUNT1P,               /* 23: cunt1p.spm */
    CMNF_SPM_CUNT2P,               /* 24: cunt2p.spm */
    CMNF_SPM_GRA_L1,               /* 25: gra_L1_SPXm.spm */
    CMNF_SPM_GUI_ALL2,             /* 26: gui_all2_SPXm.spm */
    CMNF_SPM_GUI_ALL,              /* 27: gui_all_SPXm.spm */
    CMNF_SPM_HARI_L1,              /* 28: hari_l1_SPXm.spm */
    CMNF_SPM_HARI_L,               /* 29: hari_l_SPXae10m.spm */
    CMNF_SPM_HARI_M1,              /* 30: hari_m1_SPXm.spm */
    CMNF_SPM_HARI_M,               /* 31: hari_m_SPXae10m.spm */
    CMNF_SPM_HARI_R1,              /* 32: hari_r1_SPXm.spm */
    CMNF_SPM_HARI_R,               /* 33: hari_r_SPXae10m.spm */
    CMNF_SPM_HI_L,                 /* 34: hi_l_SPXae10m.spm */
    CMNF_SPM_HI_LRED,              /* 35: hi_lred_SPXae10m.spm */
    CMNF_SPM_HI_M,                 /* 36: hi_m_SPXae10m.spm */
    CMNF_SPM_HI_MRED,              /* 37: hi_mred_SPXae10m.spm */
    CMNF_SPM_HI_R,                 /* 38: hi_r_SPXae10m.spm */
    CMNF_SPM_HI_RRED,              /* 39: hi_rred_SPXae10m.spm */
    CMNF_SPM_HKBACK,               /* 40: hkback.spm */
    CMNF_SPM_HKCOUNTER,            /* 41: hkcounter.spm */
    CMNF_SPM_JIMAKU1,              /* 42: jimaku1_SPXm.spm */
    CMNF_SPM_LS_JIMAKU1,           /* 43: ls_jimaku1_SPXm.spm */
    CMNF_SPM_METER1,               /* 44: meter1_SPXm.spm */
    CMNF_SPM_METER2,               /* 45: meter2_SPXm.spm */
    CMNF_SPM_METER3,               /* 46: meter3_SPXm.spm */
    CMNF_SPM_TRYAGAIN,             /* 47: tryagain_SPXm.spm */
    CMNF_SPM_VS,                   /* 48: vs_SPXm.spm */
    CMNF_SPM_VSBG,                 /* 49: vsbg.spm */
    CMNF_SPA_BAR_BAR_OTE_BAR,      /* 50: bar_bar__ote_bar_ */
    CMNF_SPA_BXY_BAR_CUNT,         /* 51: bxy_bar__cunt_ */
    CMNF_SPA_BXY_BAR_OTE_BAR,      /* 52: bxy_bar__ote_bar_ */
    CMNF_SPA_BXY_BXY_HK_AKEWIPE,   /* 53: bxy_bxy_SPXt__hk_akewipe_ */
    CMNF_SPA_BXY_BXY_HK_WIPE_COOL, /* 54: bxy_bxy_SPXt__hk_wipe_cool */
    CMNF_SPA_CUNT1P_CUNT,          /* 55: cunt1p__cunt_ */
    CMNF_SPA_CUNT1P_CUNT_P,        /* 56: cunt1p__cunt__p */
    CMNF_SPA_CUNT2P_CUNT,          /* 57: cunt2p__cunt_ */
    CMNF_SPA_CUNT2P_CUNT_P,        /* 58: cunt2p__cunt__p */
    CMNF_SPA_HKBACK_HK_AKEWIPE,    /* 59: hkback__hk_akewipe_ */
    CMNF_SPA_HKBACK_HK_WIPE_COOL,  /* 60: hkback__hk_wipe_cool */
    CMNF_SPA_METER1_METER2,        /* 61: meter1_SPXm__meter2_ */
    CMNF_SPA_METER1_METER2_P,      /* 62: meter1_SPXm__meter2__p */
    CMNF_SPA_METER1_METER,         /* 63: meter1_SPXm__meter_ */
    CMNF_SPA_METER1_METER_P,       /* 64: meter1_SPXm__meter__p */
    CMNF_SPA_METER2_METER2,        /* 65: meter2_SPXm__meter2_ */
    CMNF_SPA_METER2_METER2_P,      /* 66: meter2_SPXm__meter2__p */
    CMNF_SPA_METER2_METER,         /* 67: meter2_SPXm__meter_ */
    CMNF_SPA_METER2_METER_P,       /* 68: meter2_SPXm__meter__p */
    CMNF_SPA_METER3_METER,         /* 69: meter3_SPXm__meter_ */
    CMNF_SPA_METER3_METER_P,       /* 70: meter3_SPXm__meter__p */
    CMNF_SPC_HK_WIPE,              /* 71: cameraShape1__hk_wipe */
    CMNF_SPC_OTE_BAR,              /* 72: cameraShape1__ote_bar */
    CMNF_WIPE_SND_HD,              /* 73: sound header (the wipe's TapCtrl bank) */
    CMNF_WIPE_SND_BD,              /* 74: sound body (the wipe's TapCtrl bank) */
    CMNF_SPM_PARA_PARA,            /* 75: para_para.spm */
    CMNF_SPA_PARA_PARA_WP_END,     /* 76: para_para__wp_end_ */
    CMNF_SPA_PARA_PARA_WP_R,       /* 77: para_para__wp_r_ */
    CMNF_SPC_WP_R,                 /* 78: cameraShape1__wp_r */
    CMNF_WIPE_SUBT_FONT,           /* 79: TIM2, 256x48 sheet of 24x24 glyphs */
    CMNF_WIPE_SUBT_CODE,           /* 80: MCODE_STR, the 19 glyphs of the font above */
    CMNF_METCOL_NORMAL_NG,         /* 81: CLT2, meter palette (EXAMTYPE_NORMAL) at full NG */
    CMNF_METCOL_NORMAL,            /* 82: CLT2, meter palette (EXAMTYPE_NORMAL) */
    CMNF_METCOL_NORMAL_OK,         /* 83: CLT2, meter palette (EXAMTYPE_NORMAL) at full OK */
    CMNF_METCOL_ORIGINAL_NG,       /* 84: CLT2, the same for EXAMTYPE_ORIGINAL */
    CMNF_METCOL_ORIGINAL,          /* 85: CLT2 */
    CMNF_METCOL_ORIGINAL_OK,       /* 86: CLT2 */
    CMNF_METCOL_HANE_NG,           /* 87: CLT2, the same for EXAMTYPE_HANE */
    CMNF_METCOL_HANE,              /* 88: CLT2 */
    CMNF_METCOL_HANE_OK,           /* 89: CLT2 */
    CMNF_HOOK_NG,                  /* 90: TIM2, hook mark (MbarHookUseNG flashes it) */
    CMNF_HOOK_NG_FLASH,            /* 91: TIM2, its flash colours */
    CMNF_HOOK_OK,                  /* 92: TIM2, hook mark (MbarHookUseOK flashes it) */
    CMNF_HOOK_OK_FLASH,            /* 93: TIM2, its flash colours */
    CMNF_MAX
} CMNF_FILE_ENUM;

int cmnfTim2Trans(void);
void* cmnfGetFileAdrs(int num);
int cmnfGetFileSize(int num);

#endif /* CMNFILE_H */
