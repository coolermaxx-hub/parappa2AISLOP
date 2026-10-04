#include "menu/menusub.h"

#include "main/cdctrl.h"
#include "main/etc.h"

#include "menu/menudata.h"
#include "menu/menufont.h"
#include "menu/menu_mdl.h"
#include "menu/mntm2hed.h"
#include "menu/p3mc.h"

#include "os/cmngifpk.h"
#include "os/syssub.h"
#include "os/system.h"
#include "os/tim2.h"

#include <libpad.h>

#include <prlib/prlib.h>

#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ids with this bit set index VSNDSEQ_Tbl (a scripted sequence) instead of TsVoiceTbl. */
#define TSSND_SEQ_FLAG   0x8000
/* Set on a map direction's destination number when the move is blocked once the map limit is reached. */
#define MNMAP_LIMITED    0x8000

/* ABGR colours. GS texture modulation treats 0x80 as 1.0, so this is "draw the texture unchanged". */
#define MN_COLOR_NEUTRAL 0x80808080
#define MN_COLOR_WHITE   0x80ffffff

static TSREPPAD menuPadState[2][4];
static TSSND_CHAN TsSndChan[15];
static BGMSTATE TsBGMState;
static MN_SCENE MNS_StageMap2;
static MN_SCENE MNS_CityHall;
static MN_SCENE MNS_OptCounter;
static MN_SCENE MNS_RepCounter;
static MN_SCENE MNS_StgCounter[2];
static MN_SCENE MNS_JimakuBak;
static P3MC_RANKSCORE CurRankScore;
static CURFILEINFO CurFileInfo;
static TsUSERPKT MnPkt;
static TsUSERPKT MnLPkt;
static sceGifPacket FPacket;
static MCMES_WORK MCMesWork;
static CMPMES_WORK CmpMesWork;
static RANKLIST RankLst[20];
static POPUP_MENU PopupMenu;
static SAVE_MENU SaveMenu;
static JUKE_MENU JukeMenu;
static OPTION_MENU OptionMenu;
static USERLIST_MENU UserListMenu;
static SCFADE ScFade;
static P3GAMESTATE *pP3GameState;
static int _bMapCaptureReq;
static int _MNwaitTime;
static int CurMapOldFlg;
static int CurMapNo;
static int CurMapBakFlg;
static int CurMapState;
static USER_DATA *UserWork;
static P3MC_STAGERANK *pCStageRank;
static P3MC_USRLST *UserLst;
static MN_USERLST_WORK *UserDispWork;
static int UCheckLoadError;
static int UCheckSaveError;
static int subStatus;
static int ret;
static int errorNo;
static int waitTime;
static MCRWDATA_HDL *pGameData;
static MNMAPPOS mnmapMap1[] = {
    {
        0,
        8,
        0,
        1,
        {
            { -1, -1, 0, 0 },
            { 1, 7, 10, 0 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
        },
    },
    {
        1,
        7,
        0,
        1,
        {
            { 0, 8, 10, 0 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
        },
    },
};
static MNMAPPOS mnmapMap[] = {
    {
        2,
        8,
        0,
        1,
        {
            { MNMAP_LIMITED | 5, 11, 10, 0 },
            { 1, 7, 10, 0 },
            { MNMAP_LIMITED | 3, 9, 10, 0 },
            { MNMAP_LIMITED | 7, 13, 10, 0 },
        },
    },
    {
        3,
        7,
        0,
        1,
        {
            { 0, 8, 10, 0 },
            { -1, -1, 0, 0 },
            { 2, 15, 10, 0 },
            { MNMAP_LIMITED | 8, 30, 10, 0 },
        },
    },
    {
        4,
        15,
        0,
        1,
        {
            { 3, 17, 10, 0 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
            { 1, 16, 10, 0 },
        },
    },
    {
        5,
        17,
        0,
        1,
        {
            { 4, 19, 10, 0 },
            { 2, 18, 10, 0 },
            { -1, -1, 0, 0 },
            { MNMAP_LIMITED | 0, 10, 10, 0 },
        },
    },
    {
        6,
        19,
        0,
        1,
        {
            { -1, -1, 0, 0 },
            { 3, 20, 10, 0 },
            { -1, -1, 0, 0 },
            { 5, 21, 10, 0 },
        },
    },
    {
        7,
        21,
        0,
        1,
        {
            { -1, -1, 0, 0 },
            { MNMAP_LIMITED | 0, 12, 0, 0 },
            { 4, 22, 10, 0 },
            { 6, 23, 10, 0 },
        },
    },
    {
        8,
        23,
        0,
        1,
        {
            { -1, -1, 0, 0 },
            { 7, 25, 10, 0 },
            { 5, 24, 10, 0 },
            { -1, -1, 0, 0 },
        },
    },
    {
        9,
        25,
        0,
        1,
        {
            { 6, 26, 10, 0 },
            { 8, 27, 10, 0 },
            { MNMAP_LIMITED | 0, 14, 10, 0 },
            { -1, -1, 0, 0 },
        },
    },
    {
        10,
        27,
        0,
        1,
        {
            { 7, 28, 10, 0 },
            { -1, -1, 0, 0 },
            { MNMAP_LIMITED | 1, 29, 10, 0 },
            { -1, -1, 0, 0 },
        },
    },
};
static MNMAPPOS mnmapMap2[] = {
    {
        2,
        8,
        0,
        1,
        {
            { 5, 11, 10, 0 },
            { 1, 7, 10, 0 },
            { 3, 9, 10, 0 },
            { 7, 13, 10, 0 },
        },
    },
    {
        3,
        7,
        0,
        1,
        {
            { 0, 8, 10, 0 },
            { -1, -1, 0, 0 },
            { 2, 15, 10, 0 },
            { 8, 30, 10, 0 },
        },
    },
    {
        4,
        15,
        0,
        1,
        {
            { 3, 17, 10, 0 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
            { 1, 16, 10, 0 },
        },
    },
    {
        5,
        17,
        0,
        1,
        {
            { 4, 19, 10, 0 },
            { 2, 18, 10, 0 },
            { 9, 31, 10, 0 },
            { 0, 10, 10, 0 },
        },
    },
    {
        6,
        19,
        0,
        1,
        {
            { -1, -1, 0, 0 },
            { 3, 20, 10, 0 },
            { -1, -1, 0, 0 },
            { 5, 21, 10, 0 },
        },
    },
    {
        7,
        21,
        0,
        1,
        {
            { -1, -1, 0, 0 },
            { 0, 12, 0, 0 },
            { 4, 22, 10, 0 },
            { 6, 23, 10, 0 },
        },
    },
    {
        8,
        23,
        0,
        1,
        {
            { -1, -1, 0, 0 },
            { 7, 25, 10, 0 },
            { 5, 24, 10, 0 },
            { -1, -1, 0, 0 },
        },
    },
    {
        9,
        25,
        0,
        1,
        {
            { 6, 26, 10, 0 },
            { 8, 27, 10, 0 },
            { 0, 14, 10, 0 },
            { -1, -1, 0, 0 },
        },
    },
    {
        10,
        27,
        0,
        1,
        {
            { 7, 28, 10, 0 },
            { -1, -1, 0, 0 },
            { 1, 29, 10, 0 },
            { -1, -1, 0, 0 },
        },
    },
    {
        11,
        31,
        0,
        1,
        {
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
            { 3, 32, 10, 0 },
        },
    },
};
static short RecordShopRute_Route0[20] = { 5, 4, 3, 9, -1, 0, 0, 0, 0, 3, 9, -1, 1, 2, 3, 9, -1, 0, 0, 0 };
static short *RecordShopRute[] = {
    &RecordShopRute_Route0[9], &RecordShopRute_Route0[13], &RecordShopRute_Route0[14],
    &RecordShopRute_Route0[15], &RecordShopRute_Route0[2], &RecordShopRute_Route0[1],
    RecordShopRute_Route0, &RecordShopRute_Route0[8], &RecordShopRute_Route0[12],
    &RecordShopRute_Route0[15],
};
static MNMAPPOS mnmapCityHall[] = {
    {
        32,
        2,
        -1,
        -1,
        {
            { 1, 5, 5, 1 },
            { 2, 8, 5, 2 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
        },
    },
    {
        34,
        5,
        -1,
        -1,
        {
            { -1, -1, 0, 0 },
            { 0, 4, 5, 3 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
        },
    },
    {
        33,
        8,
        -1,
        -1,
        {
            { 0, 7, 5, 4 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
            { -1, -1, 0, 0 },
        },
    },
};
static short AnmCHallPara_OptRet[] = { 14, 20, 4108, -1 };
static short AnmCHallPara_Opt[] = { 14, 21, 4108, -1 };
static short AnmCHallPara_RepRet[] = { 14, 22, 4108, -1 };
static short AnmCHallPara_Rep[] = { 14, 23, 4108, -1 };
static short AnmCHallFphs_OptRet[] = { 15, 25, 4109, -1 };
static short AnmCHallFphs_Opt[] = { 15, 26, 4109, -1 };
static short AnmCHallFphs_RepRet[] = { 15, 27, 4109, -1 };
static short AnmCHallFphs_Rep[] = { 15, 28, 4109, -1 };
static short AnmCHallChar_Log[] = { 10, 19, 11, 24, -1, 0, 0, 0 };
static short AnmCHallChar_Opt[] = { 10, 21, 11, 26, -1, 0, 0, 0 };
static short AnmCHallChar_Rep[] = { 10, 23, 11, 28, -1, 0, 0, 0 };

MN_SCENETBL Scene_StageMap       = { Mdl_StageMapH,      Cam_StageMap };
MN_SCENETBL Scene_StageMapA      = { Mdl_StageMapA,      Cam_StageMap };
MN_SCENETBL Scene_StageMapY      = { Mdl_StageMapY,      Cam_StageMap };
MN_SCENETBL Scene_CityHall       = { Mdl_CityHall,       Cam_CityHall };
MN_SCENETBL Scene_OptCounter     = { Mdl_OptCounter,     Cam_Notdef };
MN_SCENETBL Scene_RepCounter     = { Mdl_RepCounter,     Cam_Notdef };
MN_SCENETBL Scene_StgCounterLoad = { Mdl_StgCounterLoad, Cam_Notdef };
MN_SCENETBL Scene_StgCounterSave = { Mdl_StgCounterSave, Cam_Notdef };
MN_SCENETBL Scene_JimakuBak      = { Mdl_JimakuBak,      Cam_Notdef };
static u_char *UserName_InitialStr  = (u_char*)"AAAAAAAA";
static u_char *UserName_InitialStr2 = (u_char*)"        ";
static u_char UserName_AsciiSetB[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!?&1234567890 ";
static u_char UserName_AsciiSetS[] = "abcdefghijklmnopqrstuvwxyz.,-:;#$%\"'()- ";
/* A packed user-name character: character set number in the high bits, index into its table in the low 12. */
#define USERNAME_CHAR_SET_SHIFT 12
#define USERNAME_CHAR_INDEX_MASK 0xfff
#define USERNAME_CHAR(set, idx) ((idx) | ((set) << USERNAME_CHAR_SET_SHIFT))

USERNAME_CSET UserName_CharSet[] = {
    { UserName_AsciiSetB, 41 },
    { UserName_AsciiSetS, 41 },
};
static u_char *TeachersName_Tbl[] = { "TEACHER", "TEACHER", "TEACHER", "TEACHER", "TEACHER", "TEACHER", "TEACHER", "TEACHER" };
static u_char *UserName_RankingNoSave = (u_char*)"";
static char *_MONTH_STR[] = {
    "", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JLY", "AUG", "SEP", "", "", "", "", "", "",
    "OCT", "NOV", "DEC", NULL,
};
static MAPBGM MapBgmTbl[] = {
    { 4, 24, 72, 144 },
    { 5, 25, 576, 1728 },
    { 6, 26, 1152, 1728 },
    { 7, 27, 1728, 1728 },
    { 8, 28, 576, 1728 },
    { 9, 29, 288, 576 },
    { 10, 30, 1152, 1152 },
    { 11, 31, 1152, 1728 },
    { 12, 32, 1728, 1728 },
    { 13, 33, 576, 1152 },
    { 14, 23, 0, 0 },
};
static TSVOICE_TBL TsVoiceTbl[] = {
    { 0, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 2, 0, 0 },
    { 0, 3, 0, 0 },
    { 0, 4, 0, 0 },
    { 0, 5, 0, 0 },
    { 0, 6, 0, 0 },
    { 0, 7, 0, 0 },
    { 0, 8, 0, 0 },
    { 0, 9, 0, 0 },
    { 0, 10, 0, 120 },
    { 0, 11, 0, 120 },
    { 0, 10, 20, 120 },
    { 1, 12, 0, 0 },
    { 1, 13, 0, 0 },
    { 1, 14, 0, 0 },
    { 1, 15, 0, 0 },
    { 1, 16, 0, 0 },
    { 1, 17, 0, 0 },
    { 1, 18, 0, 120 },
    { 1, 19, 0, 0 },
    { 1, 20, 0, 0 },
    { 1, 21, 0, 0 },
    { 1, 22, 0, 0 },
    { 4, 23, 0, 0 },
    { 4, 24, 0, 0 },
    { 5, 25, 0, 0 },
    { 6, 26, 0, 0 },
    { 7, 27, 0, 0 },
    { 8, 28, 0, 0 },
    { 9, 29, 0, 0 },
    { 10, 30, 0, 0 },
    { 11, 31, 0, 0 },
    { 12, 32, 0, 0 },
    { 13, 33, 0, 0 },
    { 1, 34, 0, 0 },
    { 1, 35, 0, 0 },
    { 1, 36, 0, 0 },
    { 1, 37, 0, 0 },
    { 1, 38, 0, 0 },
    { 1, 39, 0, 0 },
    { 1, 40, 0, 120 },
    { 2, 41, 0, 120 },
    { 2, 42, 0, 140 },
    { 2, 43, 0, 120 },
    { 2, 44, 0, 120 },
    { 2, 45, 0, 120 },
    { 2, 46, 0, 120 },
    { 1, 47, 0, 0 },
    { 1, 48, 0, 0 },
    { 1, 49, 0, 0 },
    { 1, 50, 0, 0 },
    { 3, 51, 10, 120 },
    { 3, 52, 0, 240 },
    { 1, 53, 0, 150 },
    { 1, 22, 0, 120 },
};
static u_short VSNDSEQ_Tbl_Seq0[] = { 65535, 360, 52, 65535, 65534 };
static u_short VSNDSEQ_Tbl_Seq1[] = { 65535, 240, 52, 65535, 65534 };
static u_short VSNDSEQ_Tbl_Seq2[] = { 36, 100, 37, 100, 65533 };
static TSVSNDSEQ VSNDSEQ_Tbl[] = {
    { 1, VSNDSEQ_Tbl_Seq2 },
    { 3, VSNDSEQ_Tbl_Seq0 },
    { 3, VSNDSEQ_Tbl_Seq1 },
};
static TSTEX_TBL TexTable[] = {
    { 0, 0 },
    { 2, 0 },
    { 3, 0 },
    { 4, 0 },
    { 5, 0 },
    { 6, 0 },
    { 7, 0 },
    { 8, 0 },
    { 9, 0 },
    { 10, 0 },
    { 11, 0 },
    { 12, 0 },
    { 15, 0 },
    { 13, 0 },
    { 14, 0 },
    { 21, 0 },
    { 22, 0 },
    { 23, 0 },
    { 18, 0 },
    { 17, 0 },
    { 16, 0 },
    { 19, 0 },
    { 20, 0 },
    { 24, 0 },
    { 25, 0 },
    { 26, 0 },
    { 27, 0 },
    { 28, 0 },
    { 29, 0 },
    { 30, 0 },
    { 31, 0 },
    { 32, 0 },
    { 33, 0 },
    { 34, 0 },
    { 35, 0 },
    { 36, 0 },
    { 37, 0 },
    { 38, 0 },
    { 39, 0 },
    { 40, 0 },
    { 41, 0 },
    { 42, 0 },
    { 43, 0 },
    { 44, 0 },
    { 45, 0 },
    { 46, 0 },
    { 47, 0 },
    { 48, 0 },
    { 49, 0 },
    { 50, 0 },
    { 51, 0 },
    { 52, 0 },
    { 53, 0 },
    { 54, 0 },
    { 55, 0 },
    { 56, 0 },
    { 57, 0 },
    { 58, 0 },
    { 59, 0 },
    { 60, 0 },
    { 61, 0 },
    { 62, 0 },
    { 63, 0 },
    { 64, 0 },
    { 65, 0 },
    { 66, 0 },
    { 67, 0 },
    { 68, 0 },
    { 69, 0 },
    { 70, 0 },
    { 71, 0 },
    { 72, 0 },
    { 73, 0 },
    { 74, 0 },
    { 75, 0 },
    { 76, 0 },
    { 77, 0 },
    { 78, 0 },
    { 79, 0 },
    { 80, 0 },
    { 81, 0 },
    { 82, 0 },
    { 83, 0 },
    { 84, 0 },
    { 85, 0 },
    { 86, 0 },
    { 87, 0 },
    { 88, 0 },
    { 89, 0 },
    { 90, 0 },
    { 91, 0 },
    { 92, 0 },
    { 93, 0 },
    { 94, 0 },
    { 95, 0 },
    { 96, 0 },
    { 97, 0 },
    { 98, 0 },
    { 99, 0 },
    { 100, 0 },
    { 101, 0 },
    { 102, 0 },
    { 103, 0 },
    { 104, 0 },
};
static PATPOS PAT_ALERT_WIN_ABOVE = { 69, 0, 0, 0, 0 };
static PATPOS PAT_ALERT_WIN_CENTER = { 70, 0, 0, 0, 0 };
static PATPOS PAT_ALERT_WIN_BELOW = { 71, 0, 0, 0, 0 };
static PATPOS PAT_ALERT_WIN_FFACE[] = {
    { 72, 0, 0, 0, 0 },
    { 73, 0, 0, 0, 0 },
    { 74, 0, 0, 0, 0 },
    { 75, 0, 0, 0, 0 },
};
static PATPOS SAVE_MENU_SELPAT[] = {
    { 67, 48, 40, 0, 0 },
    { 68, 348, 40, 0, 0 },
};
static PTPOS SAVEWZoom_CXY[] = {
    { 320, 160 },
};
static PATPOS LG_SCROLL_MARK[] = {
    { 20, 309, 27, 0, 0 },
    { 20, 309, 174, 0, -1 },
};
static PATPOS RP_SCROLL_MARK[] = {
    { 18, 309, 27, 0, 0 },
    { 18, 309, 174, 0, -1 },
};
static PATPOS LLG_SCROLL_MARK[] = {
    { 22, 309, 27, 0, 0 },
    { 22, 309, 174, 0, -1 },
};
static PATPOS CSSLASH_MARK = { 15, 552, 174, 0, 0 };
static STRPOS PAGENO_StrCOD[] = {
    { 542, 182, 0x807f7f7f },
    { 572, 182, 0x807f7f7f },
};
static PTPOS CellCusPos[] = {
    { 33, 14 },
    { 33, 40 },
    { 33, 66 },
    { 33, 92 },
    { 33, 118 },
    { 33, 144 },
    { 33, 170 },
};
static STRPOS LOGS_StrCOD[] = {
    { 47, 16, 0x804d2200 },
    { 483, 12, 0x804d2200 },
    { 483, 20, 0x804d2200 },
    { 549, 16, 0x804d2200 },
    { 253, 18, 0x80660000 },
};
static STRPOS LOGL_StrCOD[] = {
    { 47, 16, 0x80003e14 },
    { 483, 12, 0x80003e14 },
    { 483, 20, 0x80003e14 },
    { 549, 16, 0x80003e14 },
    { 253, 18, 0x801e4000 },
};
static STRPOS REPLAY_StrCOD[] = {
    { 43, 16, 0x80330853 },
    { 483, 12, 0x80330853 },
    { 483, 20, 0x80330853 },
    { 549, 16, 0x80330853 },
    { 253, 14, 0x80000061 },
    { 253, 21, 0x80330853 },
};
static STRPOS VSREPLAY_StrCOD[] = {
    { 43, 16, 0x80330853 },
    { 483, 12, 0x80330853 },
    { 483, 20, 0x80330853 },
    { 549, 16, 0x80330853 },
    { 157, 14, 0x80000061 },
    { 151, 21, 0x80330853 },
    { 353, 14, 0x80000061 },
    { 355, 21, 0x80330853 },
};
static PATPOS VS_MARK = { 16, 240, 7, 0, 0 };
static PATPOS VS_WINMARK1 = { 17, 86, 14, 0, 0 };
static PATPOS VS_WINMARK2 = { 17, 291, 14, 0, 0 };
static PATPOS LG_NEWDATA_MARK = { 21, 203, 7, 0, 0 };
static PATPOS RP_NEWDATA_MARK = { 19, 203, 7, 0, 0 };
static PATPOS STGCNameBox[] = {
    { 8, 123, -6, 0, 0 },
    { 9, 159, -6, 185, 0 },
    { 10, 344, -6, 0, 0 },
    { 14, 159, 3, 0, 0 },
    { 11, 134, 6, 0, 0 },
};
static PATPOS STGCNameBoxOK = { 13, 322, 6, 0, 0 };
static PATPOS VS1PNameBox[] = {
    { 8, -1, -6, 0, 0 },
    { 9, 35, -6, 185, 0 },
    { 10, 220, -6, 0, 0 },
    { 14, 35, 3, 0, 0 },
    { 11, 10, 6, 0, 0 },
};
static PATPOS VS1PNameBoxOK = { 13, 198, 6, 0, 0 };
static PATPOS VS2PNameBox[] = {
    { 8, 269, -6, 0, 0 },
    { 9, 305, -6, 185, 0 },
    { 10, 490, -6, 0, 0 },
    { 14, 305, 3, 0, 0 },
    { 12, 278, 6, 0, 0 },
};
static PATPOS VS2PNameBoxOK = { 13, 468, 6, 0, 0 };
static MNOPT_OBJ OptionSelTbl_Sel0[] = {
    { 0, { 3, 406, 49, 0, 0 } },
    { 1, { 2, 397, 49, 0, 0 } },
};
static MNOPT_OBJ OptionSelTbl_Sel1[] = {
    { 1, { 4, 430, 75, 0, 0 } },
    { 0, { 5, 426, 75, 0, 0 } },
};
static MNOPT_OBJ OptionSelTbl_Sel2[] = {
    { 0, { 5, 426, 101, 0, 0 } },
    { 1, { 4, 430, 101, 0, 0 } },
};
static MNOPT_OBJ OptionSelTbl_Sel3[] = {
    { 0, { 5, 426, 127, 0, 0 } },
    { 1, { 4, 430, 127, 0, 0 } },
};
static PATPOS MNOptMiniFrm[] = {
    { 6, 387, 44, 0, 0 },
    { 6, 387, 70, 0, 0 },
    { 6, 387, 96, 0, 0 },
    { 6, 387, 122, 0, 0 },
};
static PATPOS MNOptLRBtn[] = {
    { 7, 360, 49, 0, 0 },
    { 7, 500, 49, 0, 0 },
    { 7, 360, 75, 0, 0 },
    { 7, 500, 75, 0, 0 },
    { 7, 360, 101, 0, 0 },
    { 7, 500, 101, 0, 0 },
    { 7, 360, 127, 0, 0 },
    { 7, 500, 127, 0, 0 },
};
static PATPOS PopMenuSel_Pat[] = {
    { 23, 56, 18, 0, 0 },
    { 24, 235, 21, 0, 0 },
    { 25, 402, 13, 0, 0 },
    { 26, 104, 42, 0, 0 },
    { 27, 466, 47, 0, 0 },
};
static PATPOS VSComMenuSel_Pat[] = {
    { 28, 402, 13, 0, 0 },
    { 30, 402, 13, 0, 0 },
    { 32, 402, 13, 0, 0 },
    { 34, 402, 13, 0, 0 },
};
static PATPOS VSComMenuSelH_Pat[] = {
    { 29, 402, 13, 0, 0 },
    { 31, 402, 13, 0, 0 },
    { 33, 402, 13, 0, 0 },
    { 35, 402, 13, 0, 0 },
};
static PATPOS SIRanking_Pat[] = {
    { 36, 0, 0, 0, 0 },
    { 48, 16, 31, 0, 0 },
};
static PATPOS Ranking_PatScroll[] = {
    { 38, 104, 21, 0, 0 },
    { 39, 104, 121, 0, 0 },
};
static PATPOS RankSISTNo_PAT[] = {
    { 37, 34, 3, 0, 0 },
    { 40, 167, 6, 0, 0 },
    { 41, 167, 6, 0, 0 },
    { 42, 167, 6, 0, 0 },
    { 43, 167, 6, 0, 0 },
    { 44, 167, 6, 0, 0 },
    { 45, 167, 6, 0, 0 },
    { 46, 167, 6, 0, 0 },
    { 47, 167, 6, 0, 0 },
};
static PATPOS VSRanking_PatTbl_Pat0[] = {
    { 49, 0, 0, 0, 0 },
    { 61, 8, 17, 0, 0 },
    { 63, 8, 17, 0, 0 },
    { 65, 8, 17, 0, 0 },
    { 60, 8, 17, 0, 0 },
};
static PATPOS VSRanking_PatTbl_Pat1[] = {
    { 49, 0, 0, 0, 0 },
    { 59, 8, 17, 0, 0 },
    { 63, 8, 17, 0, 0 },
    { 65, 8, 17, 0, 0 },
    { 62, 8, 17, 0, 0 },
};
static PATPOS VSRanking_PatTbl_Pat2[] = {
    { 49, 0, 0, 0, 0 },
    { 59, 8, 17, 0, 0 },
    { 61, 8, 17, 0, 0 },
    { 65, 8, 17, 0, 0 },
    { 64, 8, 17, 0, 0 },
};
static PATPOS VSRanking_PatTbl_Pat3[] = {
    { 49, 0, 0, 0, 0 },
    { 59, 8, 17, 0, 0 },
    { 61, 8, 17, 0, 0 },
    { 63, 8, 17, 0, 0 },
    { 66, 8, 17, 0, 0 },
};
static PATPOS *VSRanking_PatTbl[] = { VSRanking_PatTbl_Pat0, VSRanking_PatTbl_Pat1, VSRanking_PatTbl_Pat2, VSRanking_PatTbl_Pat3 };
static PATPOS RankVSSTNo_PAT[] = {
    { 50, 51, 0, 0, 0 },
    { 51, 157, 2, 0, 0 },
    { 52, 157, 2, 0, 0 },
    { 53, 157, 2, 0, 0 },
    { 54, 157, 2, 0, 0 },
    { 55, 157, 2, 0, 0 },
    { 56, 157, 2, 0, 0 },
    { 57, 157, 2, 0, 0 },
    { 58, 157, 2, 0, 0 },
};
static STRPOS SRanking_Str[] = {
    { 26, 40, 0x8056063d },
    { 204, 40, 0x8056063d },
    { 49, 40, 0x8056063d },
};
static STRPOS VRanking_Str[] = {
    { 26, 46, 0x8056063d },
    { 204, 46, 0x8056063d },
    { 49, 46, 0x8056063d },
};
static PTPOS PopRnk_pPos_Pos0[4] = {
    { 0, 0 },
    { 0, 0 },
    { 32, 50 },
    { 376, 50 },
};
static POPRNK_PPOS PopRnk_pPos[] = {
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[2] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[2] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[2] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[3] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[2] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[2] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[3] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[2] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[2] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[3] },
    { PopRnk_pPos_Pos0, &PopRnk_pPos_Pos0[3] },
};
static int PopRnkPos_No[][9] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 2, 3, 0, 0, 0, 0 },
    { 0, 4, 4, 5, 6, 6, 0, 0, 0 },
    { 0, 7, 7, 8, 9, 9, 9, 10, 7 },
};
static int PopBubblePat_No[][9] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 2, 3, 3, 0, 0, 0, 0 },
    { 0, 4, 5, 6, 7, 8, 0, 0, 0 },
    { 0, 9, 10, 11, 12, 13, 14, 15, 16 },
};
static PTPOS POPWZoom_CXY[] = {
    { 446, 132 },
    { 446, 144 },
    { 446, 97 },
    { 371, 97 },
    { 446, 156 },
    { 446, 109 },
    { 371, 109 },
    { 202, 109 },
    { 202, 156 },
    { 446, 121 },
    { 446, 85 },
    { 371, 85 },
    { 202, 85 },
    { 202, 121 },
    { 202, 156 },
    { 277, 156 },
    { 446, 156 },
    { 0, 0 },
};
static PTPOS JUKEBOX_Pos[] = {
    { 70, 73 },
    { 164, 73 },
    { 258, 73 },
    { 352, 73 },
    { 446, 73 },
    { 110, 129 },
    { 204, 129 },
    { 298, 129 },
    { 392, 129 },
    { 486, 129 },
};
static PATPOS JUKEJKT_Pat[] = {
    { 82, 0, 0, 0, 0 },
    { 83, 0, 0, 0, 0 },
    { 84, 0, 0, 0, 0 },
    { 85, 0, 0, 0, 0 },
    { 86, 0, 0, 0, 0 },
    { 87, 0, 0, 0, 0 },
    { 88, 0, 0, 0, 0 },
    { 89, 0, 0, 0, 0 },
    { 90, 0, 0, 0, 0 },
    { 91, 0, 0, 0, 0 },
};
static float JUKEWAV_INITBL[] = { 0.5f, 1.0f, -0.5f, 0.0f, 0.5f, 1.0f, -0.5f, 0.0f, 0.5f, 1.0f };
static PATPOS JUKEJKT_PatS[] = {
    { 92, 0, 0, 0, 0 },
};
static PATPOS JUKEREC_Pat[] = {
    { 93, 0, 0, 0, 0 },
    { 94, 0, 0, 0, 0 },
    { 95, 0, 0, 0, 0 },
    { 96, 0, 0, 0, 0 },
    { 97, 0, 0, 0, 0 },
    { 98, 0, 0, 0, 0 },
    { 99, 0, 0, 0, 0 },
    { 100, 0, 0, 0, 0 },
    { 101, 0, 0, 0, 0 },
    { 102, 0, 0, 0, 0 },
};
static PATPOS JUKEREC_PatS[] = {
    { 103, 0, 0, 0, 0 },
};
static u_int HosiColor[][8] = {
    { 0x80c80000, 0x7c7c68, 0x7c3400, 0x706864, 0x604040, 0x784c38, 0x643834, 0x7c3400 },
    { 0x80f8b000, 0x7c7c7c, 0x707800, 0x7c7c7c, 0x7c744c, 0x7c7c30, 0x707800, 0x786834 },
    { 0x8000d0ff, 0x7c7c7c, 20604, 0x7c7c7c, 0x447c7c, 0x447c7c, 20604, 20604 },
};
HOSI_TYPE hTypeTable[17] = {
    { 20, 5, 16, 480, 7, 50 },
    { 15, 5, 16, 720, 6, 70 },
    { 15, 5, 16, 640, 5, 80 },
    { 20, 5, 16, 480, 1, 100 },
    { 10, 4, 16, 560, 7, 70 },
    { 10, 4, 16, 400, 6, 80 },
    { 5, 4, 16, 480, 5, 90 },
    { 8, 4, 16, 320, 1, 100 },
    { 10, 3, 16, 150, 7, 60 },
    { 10, 3, 16, 140, 6, 80 },
    { 10, 3, 16, 120, 5, 100 },
    { 4, 2, 32, 280, 2, 40 },
    { 4, 2, 32, 240, 4, 100 },
    { 4, 1, 32, 300, 3, 60 },
    { 3, 1, 32, 180, 1, 100 },
    { 3, 0, 64, 320, 2, 80 },
    { 3, 0, 64, 240, 1, 100 },
};
static TSTEX_INF *tblTex = NULL;
static u_int RPPadBit[] = { SCE_PADLup, SCE_PADLdown, SCE_PADLright, SCE_PADLleft };
static MCDATA_TBL McVoiceTbl[23] = {
    { MCMES(0, 3), 22 },
    { MCMES(0, 7), 39 },
    { MCMES(0, 19), 17 },
    { MCMES(0, 17), 14 },
    { MCMES(MCMES_KIND_TIMED, 10), 41 },
    { MCMES(MCMES_KIND_TIMED, 21), 19 },
    { MCMES(MCMES_KIND_TIMED, 24), 55 },
    { MCMES(MCMES_KIND_TIMED, 23), 54 },
    { MCMES(MCMES_KIND_CANCEL, 2), 20 },
    { MCMES(MCMES_KIND_CANCEL, 11), 20 },
    { MCMES(MCMES_KIND_CANCEL, 12), 20 },
    { MCMES(MCMES_KIND_CANCEL, 13), 21 },
    { MCMES(MCMES_KIND_CANCEL, 14), 21 },
    { MCMES(MCMES_KIND_CANCEL, 4), 23 },
    { MCMES(MCMES_KIND_CONFIRM, 22), TSSND_SEQ_FLAG | 0 },
    { MCMES(MCMES_KIND_CANCEL, 5), 35 },
    { MCMES(MCMES_KIND_CANCEL, 6), 36 },
    { MCMES(MCMES_KIND_CANCEL, 8), 40 },
    { MCMES(MCMES_KIND_CANCEL, 9), 40 },
    { MCMES(MCMES_KIND_CONFIRM, 15), 16 },
    { MCMES(MCMES_KIND_CONFIRM, 16), 13 },
    { MCMES(MCMES_KIND_CANCEL, 18), 15 },
    { MCMES(MCMES_KIND_CANCEL, 20), 18 },
};
static MCDATA_TBL McFaceTbl[23] = {
    { MCMES(0, 3), 3 },
    { MCMES(0, 7), 3 },
    { MCMES(0, 19), 3 },
    { MCMES(0, 17), 3 },
    { MCMES(MCMES_KIND_TIMED, 10), 4 },
    { MCMES(MCMES_KIND_TIMED, 21), 4 },
    { MCMES(MCMES_KIND_TIMED, 24), 2 },
    { MCMES(MCMES_KIND_TIMED, 23), 2 },
    { MCMES(MCMES_KIND_CANCEL, 2), 2 },
    { MCMES(MCMES_KIND_CANCEL, 11), 2 },
    { MCMES(MCMES_KIND_CANCEL, 12), 2 },
    { MCMES(MCMES_KIND_CANCEL, 13), 2 },
    { MCMES(MCMES_KIND_CANCEL, 14), 2 },
    { MCMES(MCMES_KIND_CANCEL, 4), 2 },
    { MCMES(MCMES_KIND_CONFIRM, 22), 1 },
    { MCMES(MCMES_KIND_CANCEL, 5), 1 },
    { MCMES(MCMES_KIND_CANCEL, 6), 1 },
    { MCMES(MCMES_KIND_CANCEL, 8), 2 },
    { MCMES(MCMES_KIND_CANCEL, 9), 2 },
    { MCMES(MCMES_KIND_CONFIRM, 15), 1 },
    { MCMES(MCMES_KIND_CONFIRM, 16), 1 },
    { MCMES(MCMES_KIND_CANCEL, 18), 2 },
    { MCMES(MCMES_KIND_CANCEL, 20), 2 },
};
static int UserList_Sw = 0;
static int OptionList_Sw = 0;
static int PopMenu_Sw = 0;
static int SaveMenu_Sw = 0;
static int JukeMenu_Sw = 0;
static USERLISTTYPE_TABLE ULTypeT_CITY_STGCLR = { 2, { 0, 1 } };
static USERLISTTYPE_TABLE ULTypeT_CITY_REPLAY = { 1, { 2, 0 } };
static USERLISTTYPE_TABLE ULTypeT_SAVE_LOG = { 1, { 3, 0 } };
static USERLISTTYPE_TABLE ULTypeT_SAVE_REPLAY = { 1, { 4, 0 } };
static int POPBtn2Sel[] = { 0, 1, 2, 3, 3, 4 };
static int POPSel2Btn[] = { 0, 1, 2, 0, 2, 0 };
static int Pop_CmpMesNo[] = { 22, 25, 23, 26, 27, 0 };
static int POPSel2BtnDir[] = { 15, 3, 15, 15, 15, 0 };
static int SaveMenu_CmpMesNo[] = { 44, 45 };
typedef struct { // 0x8
    /* 0x0 */ MENU_DISKSND_ENUM bgmNo;
    /* 0x4 */ int endV;
} BGM_TABLE;

static BGM_TABLE JukeBgmTbl[] = {
    { 1, 10532 },
    { 2, 12834 },
    { 3, 14492 },
    { 4, 13410 },
    { 5, 11108 },
    { 6, 13834 },
    { 7, 12126 },
    { 8, 14254 },
    { 9, 14914 },
    { 0, 9890 },
};
static int JukeMenu_CmpMesNo[] = { 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };
static MNOPT_SELINF OptionSelTbl[] = {
    { 2, OptionSelTbl_Sel0, 40, 48 },
    { 2, OptionSelTbl_Sel1, 41, 49 },
    { 2, OptionSelTbl_Sel2, 42, 50 },
    { 2, OptionSelTbl_Sel3, 43, 51 },
};
static USERLIST_TYPE UserListTbl[] = {
    { MNS_StgCounter, 0, 1, 1, { 36, -1 } },
    { &MNS_StgCounter[1], 1, 1, 0, { 49, 47 } },
    { &MNS_RepCounter, 0, 2, 2, { 38, -1 } },
    { &MNS_StgCounter[1], 1, 1, 0, { 46, 47 } },
    { &MNS_RepCounter, 1, 2, 2, { 46, 47 } },
};
/* sdata 399820 */ extern int _TexFunc; /* static */
/* sdata 399824 */ extern HOSI_OBJ *HOSIObj; /* static */
MN_SCENE MNS_StageMap = {
    0,
    0,
    NULL,
    0,
    {
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
        {
            NULL,
            { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
            {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
            },
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0.0f,
            0.0f,
        },
    },
    0,
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    0,
    0,
};
/* sdata 399828 */ extern MAP_TIME MapTime;

static int   TsGetMenuPadIsRepeat(int no, int npad);
static void  TSSNDPLAY(int n);
static void  TSSNDSTOP(int chan);
static void  TSSNDMASK_CHAN(int chan, int mskflag);
static void  TSSND_SKIPSTOP(int n);
static void  TSSND_SKIPPLAY(int n);
static int   TSSND_CHANISSTOP(int chan);
static void  tsBGMONEPlay(int no);
static void  tsBGMONEStop(int no);
static void  tsBGMONEVol(int no, int vol);
static void  tsBGMONETop(int no, int vol);
static void  tsBGMONEflow(void);
static void  tsBGMONEPause(int flg);
/* static */ void  TsBGMInit(void);
/* static */ void  TsBGMPlay(int no, int time);
static void  TsBGMStop(int time);
static void  TsBGMMute(int time);
static int   TsBGMLoadCheck(void);
static void  TsBGMPause(int flg);
/* static */ void  TsBGMPoll(void);
static void* TsCmnPkOpen(sceGifPacket *pgifpk);
static void  TsCmnPkClose(sceGifPacket *pgifpk, void *pk, int pri);
static void  TsClearMenuPad(int no);
static void  TsGetMenuPad(int no, u_int *getpad);
static void  TsSndFlow(int flg);
static int   TSNumMov(int cn, int dn, int scale);
static int   TSLOOP(int no, int max);
static int   TSLIMIT(int no, int min, int max);
static int   TsMENU_GetMapNo(int *psize);
static void  TsMENU_GetMapTimeState(int flg);
/* static */ void  TsSetScene_Map(MN_SCENE *pScene, int mapNo, int tflg, int bFocus);
static void  TsSet_ParappaCapColor(void);
/* static */ void  TsClearSet(P3GAMESTATE *pstate);
static void  TsCheckEnding(P3GAMESTATE *pstate);
/* static */ void  TsSetRankingName(P3MC_STAGERANK *pRankTop, u_char *name);
static void  TsSetRanking2UData(USER_DATA *puser, P3MC_STAGERANK *wkRank);
/* static */ void  TsSetSaveData(MCRWDATA_HDL *pDataW, int mode, USER_DATA *puser);
/* static */ void  TsRestoreSaveData(MCRWDATA_HDL *pDataW, int mode);
/* static */ int   TsRanking_Set(void);
/* static */ int   TsCheckTimeMapChange(void);
static int   TsMemCardCheck_Flow(int flg, u_int tpad);
static int   TsMap_Flow(int flg, u_int tpad, u_int tpad2);
/* static */ void  TsMakeUserWork(int mode);
/* static */ void  TsSaveSuccessProc(void);
/* static */ int   MpSave_Flow(int flg, u_int tpad, u_int tpad2);
static int   MpCityHall_Flow(int flg, u_int tpad, u_int tpad2);
/* static */ void  MpCityHallParaStart(int pos);
static void  MpCityHallFPHSSoundMask(int flg);
/* static */ int   MpCityHallFPHSMove(int pos, int fpos);
static void  MpCityHallFPHOK(int flg);
/* static */ void  MpCityHallCharPosSet(int pos);
static int   MpPopMenu_Flow(int flg, u_int tpad);
/* static */ int   MpMapMenu_Flow(int flg, MAPPOS *mpw, u_int tpad);
static int   _MapGetMovableDir(MAPPOS *mpw);
/* static */ int   McErrorMess(int err);
static void  McInitFlow(void);
/* static */ int   McStartCheckFlow(int flg);
/* McUserCheckFlow check types: which operation the card check prepares for. */
enum {
    MCCHECK_BROWSE = 0, /* list the saved users only */
    MCCHECK_LOAD = 1,
    MCCHECK_SAVE = 2,
    MCCHECK_BOTH = 3
};

/* static */ int   McUserCheckFlow(int type, int mode, int *bError);
/* Results of McUserSaveFlow / McUserLoadFlow. */
enum {
    MCFLOW_RUNNING = -1,
    MCFLOW_DONE = 0,
    MCFLOW_BROKEN = 1,       /* the file on the card is damaged */
    MCFLOW_FAILED = 2,       /* the error was already shown to the player */
    MCFLOW_CARD_CHANGED = 4  /* card was swapped: restart the card check */
};

/* Exit states of the user save/load flows, each returning its MCFLOW_* result. */
enum {
    MCUSER_EXIT_DONE = 0xf000,
    MCUSER_EXIT_BROKEN = 0xf001,
    MCUSER_EXIT_FAILED = 0xf002,
    MCUSER_EXIT_CARD_CHANGED = 0xf004
};

/* static */ int   McUserSaveFlow(USER_DATA *puser);
/* static */ int   McUserLoadFlow(int fileNo, int mode, int bBroken);
static void  TsMCAMes_Init(void);
static int   TsMCAMes_GetSelect(void);
static int   TsMCAMes_IsON(void);
/* static */ void  TsMCAMes_Flow(u_int tpad);
/* static */ void  TsMCAMes_Draw(SPR_PKT pk, SPR_PRM *spr);
static void  TsCMPMes_Draw(SPR_PKT pk, SPR_PRM *spr);
static void  TsANIME_Init(ANIME_WK *wk);
static int   TsANIME_Poll(ANIME_WK *wk);
static void  TsANIME_Start(ANIME_WK *wk, int state, int tim);
static int   TsANIME_GetRate(ANIME_WK *wk, float *rt0, float *rt1, float *rt2);
/* static */ void  _TsSortSetRanking(P3MC_RANKSCORE **ptRank, int n, P3MC_RANKSCORE *pRank, int bNameCmp);
/* static */ RANKLIST* TsGetRankingList(int flag, int vsLev, int stageNo, int *nrank);
/* static */ int   TsPopMenu_Flow(int flg, u_int tpad);
/* static */ void  TsPopMenu_Draw(SPR_PKT pk, SPR_PRM *spr);
/* static */ int   TsSaveMenu_Flow(int flg, u_int tpad);
/* static */ void  TsSaveMenu_Draw(SPR_PKT pk, SPR_PRM *spr);
static void  TSJukeCDObj_Init(JUKECDOBJ *pw, int pno);
/* static */ void  _TsJkJacketPut(SPR_PKT pk, SPR_PRM *spr, JUKECDOBJ *pw, int px, int py, float zx, float rot, u_int abgr, u_int abgrs);
/* static */ void  _TsJkRecordPut(SPR_PKT pk, SPR_PRM *spr, JUKECDOBJ *pw, int px, int py, float zr, float rrot, u_int abgr, u_int abgrs);
/* static */ void  TSJukeCDObj_Draw(SPR_PKT pk, SPR_PRM *spr, JUKECDOBJ *pw, int px, int py, int anmtime);
static int   TsJukeIsObjAnime(int isComp);
/* static */ int   TsJukeObjAnime(int isOut);
/* static */ int   TsJukeObjAnime2(int isOut);
/* static */ int   _TsJKMoveCus(int *cx, int *cy, int mx, int my, JUKECDOBJ *cobj);
/* static */ void  _TsJKSetPadArrow(int sel, JUKECDOBJ *cobj);
/* static */ int   TsJukeMenu_Flow(int flg, u_int tpad);
static void  TsJukeMenu_Draw(SPR_PKT pk, SPR_PRM *spr);
static void  TsCmnCell_CusorSET(CELLOBJ *obj);
static void  TsCmnCell_CusorON(CELLOBJ *obj);
static void  TsCmnCell_CusorOFF(CELLOBJ *obj);
static void  TsCmnCell_CusorSEL(CELLOBJ *obj);
static void  TsCmnCell_CusorMASK(CELLOBJ *obj);
/* static */ void  TsCmnCell_CusorDraw(SPR_PKT pk, SPR_PRM *spr, int n, CELLOBJ *obj, int ox, int oy, int CurColor);
/* static */ int   TsOption_Flow(int flg, u_int tpad);
/* static */ void  TsOption_Draw(SPR_PKT pk, SPR_PRM *spr);
static int   TsUserList_GetCurFileNo(int *isBroken);
static int   TsUserList_IsGetFileSave(void);
static int   TsUserList_SortUser(void);
static void  TsUserList_SetCurUserData(USER_DATA *psrc);
static void  TsUserList_SetCurDispUserData(USER_DATA *psrc);
/* static */ void  TsUserList_SetCurFileNoCusor(int fileNo, P3MC_DATE *fDate);
static void  TsUserList_SetType(USERLISTTYPE_TABLE *ptbl, int mode, int curTag);
static int   TsUserList_TagChangeAble(USERLIST_MENU *pfw, int *pno);
/* static */ int   TsUserList_SetCurTag(USERLIST_MENU *pfw, int no);

/* static */ int   TsUserList_Flow(int flg, u_int tpad, u_int tpad2);
/* static */ void  TsUserList_Draw(SPR_PKT pk, SPR_PRM *spr);
static void  NameSpaceCut(u_char *dst, u_char *src);
/* static */ void  TsUser_PanelDraw(SPR_PKT pk, SPR_PRM *spr, USER_DATA *user, int px, int py, int pflg, int isLog);
/* static */ void  TsNAMEINBox_SetName(NAMEINW *pfw, u_char *name);
static void  TsNAMEINBox_GetName(NAMEINW *pfw, u_char *name);
/* static */ int   TsNAMEINBox_Flow(int flg, NAMEINW *pfw, u_int tpad);
/* static */ void  TsNAMEINBox_Draw(SPR_PKT pk, SPR_PRM *spr, int px, int py, int isLog, NAMEINW *pfw, int side);
static void  TsSCFADE_Flow(int flg, int prm);
/* static */ void  TsSCFADE_Draw(SPR_PKT pk, SPR_PRM *spr, int prio);
static void  TsPatTexFnc(int flg);
/* static */ void  _TsPatSetPrm(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy);
static void  TsPatPut(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy);
static void  TsPatGetSize(PATPOS *ppos, int *x, int *y, int *w, int *h);
/* static */ void  TsPatPutRZoom(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy, float zrate, float rot);
/* static */ void  TsPatPutMZoom(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy, float Zrx, float Zry, int mx, int my, float Crx, float Cry);
static void  TsPatPutSwing(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy, int mx, int my, float Crx);
static void  TsPatPutUneri(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy, int mx, int my, float Crx, float Drt);
static void  TsCELBackInit(void);
/* static */ void  _TsCELBackObjDraw(SPR_PKT pk, SPR_PRM *spr, int sw, int sh, u_int *colTbl);
/* static */ void  TsHosiPut(SPR_PKT pk, SPR_PRM *spr, TSTEX_INF *ptex, float px, float py, float zrate, float rot);

static int TsGetMenuPadIsRepeat(int no, int npad) {
    return (menuPadState[no][npad].state >= 2);
}

static void TSSNDPLAY(int n) {
    TSVOICE_TBL *ptap;
    TSSND_CHAN  *pchan;
    int          bMsk;
    TSVSNDSEQ   *pSeq;

    if (n >= 0) {
        if (n & TSSND_SEQ_FLAG) {
            pSeq  = &VSNDSEQ_Tbl[n & ~TSSND_SEQ_FLAG];
            pchan = &TsSndChan[pSeq->chanNo];

            bMsk = pchan->bMsk;

            memset(pchan, 0, sizeof(*pchan));

            pchan->isSeq = TRUE;

            pchan->pSeq = pSeq->pSeqTop;
            pchan->bMsk = bMsk;
        } else if (n < 0x38) {
            ptap  = &TsVoiceTbl[n];
            pchan = &TsSndChan[ptap->chanNo];

            bMsk = pchan->bMsk;

            memset(pchan, 0, sizeof(*pchan));

            pchan->pTap = ptap;
            pchan->bMsk = bMsk;
        }
    }
}

static void TSSNDSTOP(int chan) {
    memset(&TsSndChan[chan], 0, sizeof(TSSND_CHAN));
    MenuVoiceStop(chan);
}

static void TSSNDMASK_CHAN(int chan, int mskflag) {
    TSSND_CHAN *pchan = &TsSndChan[chan];

    pchan->bMsk = mskflag;
    if (pchan->pTap != NULL || pchan->pSeq != NULL) {
        MenuVoiceStop(chan);
    }
}

static void TSSND_SKIPSTOP(int n) {
    TSVOICE_TBL *ptap;
    TSSND_CHAN  *pchan;

    if (n >= 0x38) {
        return;
    }

    ptap  = &TsVoiceTbl[n];
    pchan = &TsSndChan[ptap->chanNo];

    if (pchan->pTap == ptap) {
        memset(pchan, 0, sizeof(*pchan));
        MenuVoiceStop(ptap->chanNo);
    }
}

static void TSSND_SKIPPLAY(int n) {
    TSVOICE_TBL *ptap;
    TSSND_CHAN  *pchan;

    if (n >= 0x38) {
        return;
    }

    ptap  = &TsVoiceTbl[n];
    pchan = &TsSndChan[ptap->chanNo];

    if (pchan->pTap == ptap && !pchan->isOn) {
        pchan->tim = ptap->ontim;
    }
}

static int TSSND_CHANISSTOP(int chan) {
    TSSND_CHAN *pchan = &TsSndChan[chan];
    int         ret   = FALSE;

    if (pchan->pTap == NULL) {
        ret = (pchan->pSeq == NULL);
    }

    return ret;
}

static void tsBGMONEPlay(int no) {
    BGMONE *wbgm  = &TsBGMState.wbgm[no];
    MAPBGM *mpbgm = &MapBgmTbl[no];

    wbgm->pbgm = mpbgm;
    wbgm->vol  = 0;
    
    if (mpbgm->lpTimeF != 0) {
        wbgm->tim = mpbgm->lpTimeF;
        MenuVoicePlayVol(mpbgm->chan, mpbgm->tapNo, 0);
    }
}

static void tsBGMONEStop(int no) {
    BGMONE *wbgm = &TsBGMState.wbgm[no];

    wbgm->pbgm = &MapBgmTbl[no];
    wbgm->vol  = 0;
    wbgm->tim  = 0;
    MenuVoiceStop(MapBgmTbl[no].chan);

    wbgm->pbgm = NULL;
}

static void tsBGMONEVol(int no, int vol) {
    BGMONE *wbgm = &TsBGMState.wbgm[no];

    if (wbgm->vol != vol) {
        wbgm->vol = vol;
        MenuVoiceSetVol(wbgm->pbgm->chan, wbgm->pbgm->tapNo, vol);
    }
}

static void tsBGMONETop(int no, int vol) {
    BGMONE *wbgm = &TsBGMState.wbgm[no];
    MAPBGM *mpbgm = &MapBgmTbl[no];

    wbgm->pbgm = mpbgm;
    wbgm->vol  = vol;
    wbgm->tim  = mpbgm->lpTimeF;
    MenuVoicePlayVol(mpbgm->chan, mpbgm->tapNo, vol);
}

static void tsBGMONEflow(void) {
    BGMONE *wbgm = TsBGMState.wbgm;
    int     i;

    for (i = 0; i < 11; i++, wbgm++) {
        if (wbgm->bPause || wbgm->pbgm == NULL) {
            continue;
        }

        if (--wbgm->tim > 0) {
            continue;
        }
        if (wbgm->pbgm->lpTime == 0) {
            continue;
        }

        wbgm->tim = wbgm->pbgm->lpTime;
        MenuVoicePlayVol(wbgm->pbgm->chan, wbgm->pbgm->tapNo, wbgm->vol);
    }
}


static void tsBGMONEPause(int flg) {
    BGMONE *wbgm = TsBGMState.wbgm;
    int     i;

    for (i = 0; i < PR_ARRAYSIZE(TsBGMState.wbgm); i++, wbgm++) {
        wbgm->bPause = (flg != 0);
    }
}

void TsBGMInit(void) {
    memset(&TsBGMState, 0, sizeof(TsBGMState));
}

static void TsBGMPlay(int no, int time) {
    BGMSTATE *pbgm = &TsBGMState;
    int       i;
    int       isCurPlay = FALSE;

    if (no >= 11) {
        return;
    }

    if (MenuVoiceBankSet(-1)) {
        pbgm->wtNo = no;
        pbgm->ctim = 0;
        pbgm->wtTim = time;
        pbgm->state = 1;
        pbgm->chgReq = 0;
        pbgm->cstate = 0;
        pbgm->wtLoad = 1;
        return;
    }

    if ((pbgm->state & 1) && pbgm->wtLoad == 0) {
        if (pbgm->sndno == no && pbgm->vol == 0x100) {
            pbgm->sndno = no;
            pbgm->vol = 0x100;
            pbgm->state = 1;
            pbgm->ttim0 = 0;
            pbgm->ttim = 0;
            tsBGMONEVol(pbgm->sndno, 0x100);
            return;
        }
        isCurPlay = TRUE;
    }

    pbgm->chgReq = 0;
    pbgm->cstate = 0;
    pbgm->ctim = 0;
    pbgm->wtLoad = 0;

    if (time > 0) {
        pbgm->ttim0 = time;
        pbgm->state = 7;
        pbgm->sndno = no;
        pbgm->vol = 0;
    } else {
        pbgm->vol = 0x100;
        pbgm->state = 1;
        pbgm->sndno = no;
        pbgm->ttim0 = 0;
    }
    pbgm->ttim = 0;

    if (!isCurPlay) {
        MNSceneMusicFitTimerClear();
        for (i = 0; i < 11; i++) {
            tsBGMONEPlay(i);
        }
    }

    tsBGMONEVol(pbgm->sndno, pbgm->vol);
}

static void TsBGMStop(int time) {
    BGMSTATE *pbgm = &TsBGMState;
    int       i;

    if (pbgm->state != 0) {
        pbgm->ttim0 = 0;
        pbgm->ttim = 0;

        if (time > 0) {
            pbgm->state = 3;
            pbgm->vol = 0x100;

            if (pbgm->ctim == 0) {
                tsBGMONEVol(pbgm->sndno, 0x100);
            }
        } else {
            pbgm->chgReq = 0;
            pbgm->cstate = 0;
            pbgm->ctim = 0;

            pbgm->state = 0;
            pbgm->vol = 0;

            for (i = 0; i < 11; i++) {
                tsBGMONEStop(i);
            }
        }
    }
}

static void TsBGMMute(int time) {
    BGMSTATE *pbgm = &TsBGMState;

    if (pbgm->state != 0) {
        pbgm->chgReq = 0;
        pbgm->cstate = 0;
        pbgm->ctim = 0;

        if (time > 0) {
            pbgm->state = 11;
            pbgm->vol = 0x100;
            pbgm->ttim0 = time;
            pbgm->ttim = 0;
        } else {
            pbgm->state = 9;
            pbgm->vol = 0;
            pbgm->ttim0 = 0;
            pbgm->ttim = 0;
        }

        tsBGMONEVol(pbgm->sndno, pbgm->vol);
    }
}

static int TsBGMLoadCheck(void) {
    return MenuVoiceBankSet(-1);
}

static void TsBGMPause(int flg) {
    BGMSTATE *pbgm = &TsBGMState;

    if (pbgm->state != 0) {
        pbgm->chgReq = 0;
        pbgm->cstate = 0;
        pbgm->ctim = 0;
        tsBGMONEPause(flg);
    }
}

void TsBGMChangePos(int no) {
    BGMSTATE *pbgm = &TsBGMState;

    if (no >= 11) {
        return;
    }

    pbgm->vol    = 256;
    pbgm->state  = 1;
    pbgm->ttim0  = 0;
    pbgm->ttim   = 0;
    pbgm->chgReq = no + 1;
}

/* static */ void TsBGMPoll(void) {
    BGMSTATE *pbgm = &TsBGMState;
    int ct;

    MNSceneMusicFitTimerFrame();
    if (pbgm->state == 0) {
        return;
    }

    if (pbgm->wtLoad) {
        MNSceneMusicFitTimerClear();
        if (MenuVoiceBankSet(-1)) {
            return;
        }
        pbgm->wtLoad = 0;
        TsBGMPlay(pbgm->wtNo, pbgm->wtTim);
    }

    tsBGMONEflow();

    if (pbgm->chgReq && (pbgm->wbgm[0].tim % 36) == 0) {
        pbgm->oldno = pbgm->sndno;
        pbgm->sndno = pbgm->chgReq - 1;
        pbgm->ctim = 1;
        pbgm->cstate = 0;
        pbgm->chgReq = 0;
    } else if (pbgm->ctim) {
        if (pbgm->ctim == 75) {
            tsBGMONEVol(0, 0);
            MenuVoiceSetVol(3, 23, 0);
            tsBGMONEVol(pbgm->oldno, 0);
            tsBGMONEVol(pbgm->sndno, pbgm->vol);
            pbgm->ctim = 0;
        } else {
            if (pbgm->ctim < 2) {
                ct = pbgm->ctim;
                tsBGMONEVol(0, ct * pbgm->vol);
            } else if (pbgm->ctim == 72) {
                tsBGMONEVol(0, 0);
                MenuVoicePlay(3, 23);
            } else if (pbgm->ctim > 72) {
                ct = 75 - pbgm->ctim;
                MenuVoiceSetVol(3, 23, (ct * pbgm->vol) / 3);
            }

            if (pbgm->ctim < 2) {
                ct = 1 - pbgm->ctim;
                tsBGMONEVol(pbgm->oldno, ct * pbgm->vol);
            } else if (pbgm->ctim == 72) {
                if (pbgm->sndno == 10) {
                    tsBGMONETop(pbgm->sndno, pbgm->vol);
                } else {
                    tsBGMONETop(pbgm->sndno, 0);
                }
            } else if (pbgm->ctim > 72) {
                if (pbgm->sndno != 10) {
                    ct = pbgm->ctim - 72;
                    tsBGMONEVol(pbgm->sndno, (ct * pbgm->vol) / 72);
                }
            }

            pbgm->ctim++;
        }
    }

    if (pbgm->ttim0) {
        pbgm->ttim++;
        if (pbgm->ttim > pbgm->ttim0) {
            pbgm->ttim0 = 0;
            pbgm->ttim = 0;
            if (pbgm->state & 4) {
                pbgm->vol = 0x100;
                pbgm->state = 1;
                pbgm->cstate = 0;
                pbgm->ctim = 0;
                tsBGMONEVol(pbgm->sndno, 0x100);
            } else {
                pbgm->vol = 0;
                pbgm->ttim0 = 0;
                pbgm->ttim = 0;
                pbgm->cstate = 0;
                pbgm->ctim = 0;
                if (!(pbgm->state & 9)) {
                    pbgm->state = 0;
                    TsBGMStop(0);
                } else {
                    pbgm->state = 9;
                    tsBGMONEVol(pbgm->sndno, 0);
                }
            }
        } else {
            pbgm->vol = (pbgm->ttim << 8) / pbgm->ttim0;
            if (!(pbgm->state & 4)) {
                pbgm->vol = 0x100 - pbgm->vol;
            }
            if (pbgm->ctim == 0) {
                tsBGMONEVol(pbgm->sndno, pbgm->vol);
            }
        }
    }
}


static void* TsCmnPkOpen(sceGifPacket *pgifpk) {
    CmnGifOpenCmnPk(pgifpk);

    sceGifPkAddGsAD(pgifpk, SCE_GS_PABE, 0);
    sceGifPkAddGsAD(pgifpk, SCE_GS_FBA_1, 0);
    sceGifPkAddGsAD(pgifpk, SCE_GS_TEST_1, SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));

    sceGifPkCloseGifTag(pgifpk);
    return pgifpk->pCurrent;
}


static void TsCmnPkClose(sceGifPacket *pgifpk, void *pk, int pri) {
    u_long giftag[2] = { SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1), 0xe };

    pgifpk->pCurrent = pk;
    sceGifPkOpenGifTag(pgifpk, *(u_long128*)giftag);
    CmnGifCloseCmnPk(pgifpk, pri);
}

int _P3DATA_SIZE(int mode) {
    u_int size;

    switch (mode) {
    case 1:
        size = 0x188;
        break;
    case 2:
        size = 0x4528;
        break;
    default:
        size = 0;
        break;
    }

    return size;
}

void TsGetTm2Tex(void *ptim2, TSTEX_INF *tex) {
    TIM2INFO info;
    int      ptw, pth;

    GetTim2Info(ptim2, &info, 1);

    tex->w = info.picturH->ImageWidth;
    tex->h = info.picturH->ImageHeight;
    tex->tex0 = info.picturH->GsTex0;

    ptw = (((int*)&tex->tex0)[0] >> 0x1a) & 0xf; /* TW */
    pth = (((u_int*)&tex->tex0)[0] >> 0x1e) | ((((u_int*)&tex->tex0)[1] & 0x3) << 0x2); /* TH */

    tex->rUsize = 1.0f / (1 << ptw);
    tex->rVsize = 1.0f / (1 << pth);

    ((u_int*)&tex->tex0)[1] |= 0x4; /* TCC = 1 */
}

void TsGetTm2HedTex(int no, TSTEX_INF *tex) {
    int           ptw, pth;
    MENU_TM2_HED *ptm2h;

    ptm2h = TsGetTM2Hed(no);

    tex->w = ptm2h->w;
    tex->h = ptm2h->h;
    tex->tex0 = ptm2h->GsTex0;

    ptw = (((int*)&ptm2h->GsTex0)[0] >> 0x1a) & 0xf; /* TW */
    pth = (((u_int*)&ptm2h->GsTex0)[0] >> 0x1e) | ((((u_int*)&ptm2h->GsTex0)[1] & 0x3) << 0x2); /* TH */

    tex->rUsize = 1.0f / (1 << ptw);
    tex->rVsize = 1.0f / (1 << pth);

    ((u_int*)&tex->tex0)[1] |= 0x4; /* TCC = 1 */
}

static void TsClearMenuPad(int no) {
    if (no >= 2) {
        return;
    }

    memset(&menuPadState[no][0], 0, sizeof(TSREPPAD) * 4);
}

static void TsGetMenuPad(int no, u_int *getpad) {
    PADD     *pPad;
    u_int     one, shot;
    TSREPPAD *pRpPad;
    u_int    *pPadBit;
    u_int     padMsk;
    u_int     i;

    pPadBit = RPPadBit;
    if (no >= 2) {
        return;
    }

    *getpad = 0;

    pRpPad = &menuPadState[no][0];
    padMsk = 0;
    pPad = &pad[no];

    one = pPad->mone;
    shot = pPad->mshot;

    for (i = 0; i < 4; i++, pRpPad++, pPadBit++) {
        padMsk |= *pPadBit;
        if (one & *pPadBit) {
            pRpPad->time = 0;
            pRpPad->state = 1;
            *getpad |= *pPadBit;
        } else if (shot & *pPadBit) {
            pRpPad->time++;
            if (pRpPad->time >= 19) {
                if (pRpPad->time % 5) {
                    pRpPad->state = 2;
                } else {
                    pRpPad->state = 3;
                    *getpad |= *pPadBit;
                }
            }
        } else {
            pRpPad->state = 0;
            pRpPad->time = 0;
        }
    }

    *getpad |= (one & ~padMsk);
}

static void TsSndFlow(int flg) {
    int          i;
    TSVOICE_TBL *ptap;
    TSSND_CHAN  *pchan;
    u_short     *pSeq, *pCur;

    if (flg == 1) {
        memset(&TsSndChan, 0, sizeof(TsSndChan));
        return;
    }

    pchan = TsSndChan;

    for (i = 0; i < 15; i++, pchan++) {
        ptap = pchan->pTap;
        if (ptap != NULL) {
            pchan->tim++;

            if (!pchan->isOn) {
                if (ptap->ontim < pchan->tim) {
                    if (!pchan->bMsk) {
                        MenuVoicePlay(i, ptap->vsetNo);
                    }

                    pchan->isOn = 1;
                    if (ptap->offtim == 0) {
                        memset(pchan, 0, sizeof(*pchan));
                    }
                }

                if (!pchan->isOn) {
                    continue;
                }
            }

            if (ptap->offtim != 0 && pchan->tim >= ptap->offtim) {
                MenuVoiceStop(i);
                memset(pchan, 0, sizeof(*pchan));
            }
        } else {
            pSeq = pchan->pSeq;
            if (pSeq != NULL) {
                pchan->tim++;
                pCur = &pSeq[pchan->sqIdx * 2];
    
                if (!pchan->isOn) {
                    pchan->isOn = 1;
                } else {
                    if (pchan->tim < pCur[1]) {
                        continue;
                    }
                    pchan->tim -= pCur[1];
                    pchan->sqIdx++;
                    pCur = &pSeq[pchan->sqIdx * 2];
                }
    
                switch (pCur[0]) {
                case 0xffff:
                    break;
                case 0xfffe:
                    pchan->sqIdx = 0;
                    pCur = pSeq;
                    break;
                case 0xfffd:
                    pchan->sqIdx = 0;
                    MenuVoiceStop(i);
                    memset(pchan, 0, sizeof(*pchan));
                    continue;
                }
                
                if (!pchan->bMsk && !(pCur[0] & 0xf000)) {
                    MenuVoicePlay(i, pCur[0]);
                }
            }
        }
    }
}

static int TSNumMov(int cn, int dn, int scale) {
    int d;
    int dv;
    int da;

    d = cn - dn;
    if (d != 0) {
        da = abs(d);

        switch (scale) {
        case 0:
            dv = (da - (da >> 3)) + 1;
            scale = 1;
            break;
        case 1:
            dv = (da - (da >> 2)) + 1;
            break;
        case 2:
            dv = (da >> 1) + 1;
            scale = 1;
            break;
        default:
            dv = (da >> 2) + 1;
            scale -= 2;
            break;
        }

        if (scale >= 2) {
            dv = ((dv * 16) - dv) / (scale * 10);
        }

        if (dv == 0) {
            dv = 1;
        }

        if (d < 0) {
            cn += dv;
            if (cn > dn) {
                cn = dn;
            }
        } else {
            cn -= dv;
            if (cn < dn) {
                cn = dn;
            }
        }
    }

    return cn;
}

float TSNumRBack(float rt, float bkrt) {
    float hrt = bkrt + 1.0f;

    rt *= (bkrt * 2.0f) + 1.0f;
    if (rt > hrt) {
        rt = (hrt * 2.0f) - rt;
    }

    return rt;
}

static int TSLOOP(int no, int max) {
    return (no + max) % max;
}

static int TSLIMIT(int no, int min, int max) {
    if (no < min) {
        return min;
    }
    if (no >= max) {
        return max - 1;
    }
    return no;
}

static int TsMENU_GetMapNo(int *psize) {
    int mn;
    int size;
    int flg;

    mn = 9;

    if (pP3GameState->pLog->nRound <= 0) {
        flg = pP3GameState->pLog->clrFlg[0];
        for (mn = 1; flg != 0; mn++) {
            flg >>= 1;
        }

        if (mn > 9) {
            mn = 9;
        }
    }

    if (mn == 0) {
        size = 0;
    } else if (mn < 2) {
        size = 1;
    } else if (mn < 4) {
        size = 2;
    } else if (mn < 5) {
        size = 3;
    } else {
        size = 4;
    }

    if (psize != NULL) {
        *psize = size;
    }

    return mn;
}

/* Inline required to match. */
static inline int PrBcdInt(u_int n) {
    return (((n / 16) * 10) + (n % 16));
}

static void TsMENU_GetMapTimeState(int flg) {
    static int nTim = 0;
    int         err;
    short       hour;
    short       state;
    MAP_TIME   *mptim;
    sceCdCLOCK  clock;

    mptim = &MapTime;

    if (flg == 1) {
        CurMapOldFlg = -1;
        CurMapBakFlg = -1;
        memset(mptim, 0, sizeof(*mptim));
        nTim = flg;
    } else if (flg == 3) {
        nTim = 30;
        return;
    }

    nTim--;
    if (nTim > 0) {
        return;
    }

    nTim = 30;

    err = sceCdReadClock(&clock);
    mptim->pad = rand() % 200;
    if (err != 0 && clock.stat == 0) {
        mptim->second = clock.second;
        mptim->minute = clock.minute;
        mptim->hour = clock.hour;
        mptim->day = clock.day;
        mptim->month = clock.month;
        mptim->year = clock.year + 0x2000;
        flg = FALSE;
    } else {
        mptim->second = 0x0;
        mptim->minute = 0x0;
        mptim->hour = 12; /* BUG: Value isn't valid BCD, */
        mptim->day = 0x1; /*      though it still works. */
        mptim->month = 0x1;
        mptim->year = 0x2000;
        flg = TRUE;
    }

    state = CurMapBakFlg;

    if (!flg) {
        hour = PrBcdInt(mptim->hour);

        /* Dumb nested ifs but required to match. */
        if (hour < 4) {
            state = 2;
        } else {
            state = 0;
            if (hour >= 7) {
                state = 1;
                if (hour >= 16) {
                    if (hour <= 18) {
                        state = 0;
                    } else {
                        state = 2;
                    }
                }
            }
        }
    } else {
        if (state <= -1) {
            state = 1;
        }
    }

    CurMapBakFlg = state;
}

/* static */ void TsSetScene_Map(MN_SCENE *pScene, int mapNo, int tflg, int bFocus) {
    int        gmn;
    P3LOG_VAL *pLog;
    int        i;
    int        nRound;
    static char map0Msk[8] = { 1, 0, 0, 0, 1, 1, 1, 1 };
    int        clrno;
    int        nCrown;
    int        cwCol[4];
    int        l;
    u_int      Cflg;
    int        cn;

    gmn = mapNo;
    pLog = pP3GameState->pLog;
    nRound = pLog->nRound;
    if (mapNo == 0) {
        gmn = TsMENU_GetMapNo(NULL);
        if (gmn == 9 && nRound >= 4) {
            gmn = 10;
        }
    }
    if (mapNo == 9 && nRound >= 4) {
        mapNo = 10;
        gmn = 10;
    }

    MNScene_End(pScene);
    switch (tflg) {
    case 1:
        MNScene_Init(pScene, &Scene_StageMap, bFocus);
        MNScene_StartAnime(pScene, -1, &StageMapAnimeBK[0]);
        break;
    case 2:
        MNScene_Init(pScene, &Scene_StageMapY, bFocus);
        MNScene_StartAnime(pScene, -1, &StageMapAnimeBK[1]);
        break;
    case 0:
    default:
        MNScene_Init(pScene, &Scene_StageMapA, bFocus);
        MNScene_StartAnime(pScene, -1, &StageMapAnimeBK[2]);
        break;
    }

    MNScene_DispSw(pScene, 1);
    MNScene_StartAnime(pScene, -1, &StageMapAnime[mapNo]);
    MNScene_StartAnime(pScene, -1, &StageMapAnime[gmn + 11]);
    MNScene_StartAnime(pScene, -1, StageMapAnimeSEA);

    for (i = 0; i < 8; i++) {
        clrno = pLog->clrCount[i];
        if (mapNo == 0 && map0Msk[i]) {
            clrno = 0;
        }

        if (clrno > 0) {
            MNScene_StartAnime(pScene, -1, &StageMapAnimeBB[i * 2]);

            if (clrno > nRound + 1) {
                clrno = nRound + 1;
            }
            if (clrno > 4) {
                clrno = 4;
            }
            if (clrno > 0) {
                clrno--;
            }
            MenuStageCl1Trans(i, clrno);

            for (l = 0; l < 4; l++) {
                cwCol[l] = 0;
            }

            nCrown = 0;
            Cflg = pLog->logCOOL[i];
            for (l = 0; l < 4 && Cflg != 0; l++, nCrown++) {
                cwCol[l] = Cflg & 0xf;
                Cflg >>= 4;
            }

            if (nCrown > 4) {
                nCrown = 4;
            }

            if (nCrown <= 0) {
                MNScene_ModelDispSw(pScene, i + 12, 0);
            } else {
                MNScene_StartAnime(pScene, -1, &StageMapCWptr[i][nCrown - 1]);
                for (l = 0; l < nCrown; l++) {
                    cn = cwCol[nCrown - 1 - l];
                    if (cn > 0) {
                        MenuCoolCl1Trans(i, l, cn - 1);
                    }
                }
            }
        } else {
            MNScene_StartAnime(pScene, -1, &StageMapAnimeBB[i * 2 + 1]);
            MNScene_ModelDispSw(pScene, i + 12, 0);
        }
    }

    if (mapNo == 0) {
        MNScene_StartAnime(pScene, -1, &StageMapAnimePA[0]);
        MNScene_StartAnime(pScene, -1, &StageMapAnimePA[6]);
    }
}

static void TsSet_ParappaCapColor(void) {
    P3LOG_VAL      *pLog = pP3GameState->pLog;
    TAP_ROUND_ENUM  n;

    switch (pLog->nRound) {
    case 0:
        n = TRND_R1;
        break;
    case 1:
        n = TRND_R2;
        break;
    case 2:
        n = TRND_R3;
        break;
    default:
        n = TRND_R4;
        break;
    }

    MenuRoundTim2Trans(n);
}

/* static */ void TsClearSet(P3GAMESTATE *pstate) {
    int        nRound;
    int        nStage;
    int        i;
    int        flg;
    int        bGoRecShop = FALSE;
    int        bRecJacket = 0;
    int        vslev;
    int        nextPos;
    u_int      clog;
    short     *pRute;
    P3LOG_VAL *pLog = pstate->pLog;

    nStage = pstate->nStage - 1;
    nRound = pLog->nRound;

    if (nStage < 0 || nStage >= 8) {
        return;
    }
    if (pstate->nMode == 1) {
        return;
    }

    if (pstate->nMode == 2) {
        vslev = pstate->vsLev;
        if (nRound >= 4 && pLog->clrVSCOM1[nStage] < 4 && vslev + 1 >= 4) {
            flg = 0;
            for (i = 0; i < 8; i++) {
                if (pLog->clrVSCOM1[i] >= 4) {
                    flg++;
                }
            }
            if (flg == 7) {
                bGoRecShop = TRUE;
                bRecJacket = 9;
            }
        }
        if (pLog->clrVSCOM1[nStage] < vslev + 1) {
            pLog->clrVSCOM1[nStage] = vslev + 1;
        }
    } else if (pstate->nMode == 0) {
        pstate->pAutoMove = NULL;
        if (nRound == 0 && pLog->clrCount[nStage] <= 0) {
            nextPos = pstate->nStage + 1;
            if (nextPos >= 9) {
                nextPos = 1;
            }
            pstate->autoMovePos[1] = -1;
            pstate->autoMovePos[0] = nextPos;
            pstate->pAutoMove = pstate->autoMovePos;
        }

        flg = (nRound < 4);
        if (flg) {
            pLog->clrFlg[nRound] |= 1 << nStage;
        }

        pLog->clrCount[nStage]++;
        if (pLog->clrCount[nStage] > nRound + 1) {
            pLog->clrCount[nStage] = nRound + 1;
        }

        if (pP3GameState->bCoolClr) {
            if (!flg && pLog->clrCOOL[nStage] < 4) {
                bGoRecShop = TRUE;
                bRecJacket = nStage;
            }
            pLog->clrCOOL[nStage] = nRound + 1;
            clog = pLog->logCOOL[nStage];
            pLog->logCOOL[nStage] = ((clog << 4) & 0xfff0) | ((nRound + 1 < 5) ? nRound + 1 : 4);
        }

        flg = TRUE;
        for (i = 0; i < 8; i++) {
            if (pLog->clrCount[i] < nRound + 1) {
                flg = FALSE;
            }
        }

        if (flg) {
            pLog->nRound++;
            if (pLog->nRound > 1000000) {
                pLog->nRound = 1000000;
            }
            printf("*** Round Up = (%d)\n", pLog->nRound);
            if (pLog->nRound == 4) {
                bGoRecShop = TRUE;
                bRecJacket = 0;
            }
        }
    }

    if (bGoRecShop) {
        pRute = RecordShopRute[nStage + 1];
        for (i = 0; i < 9; i++) {
            pstate->autoMovePos[i] = pRute[i];
            if (pRute[i] < 0) {
                break;
            }
        }
        pstate->autoMovePos[i] = -2;
        pstate->pAutoMove = pstate->autoMovePos;
        pstate->curRecJacket = bRecJacket;
    }
}

static void TsCheckEnding(P3GAMESTATE *pstate) {
    int        nRound;
    int        nStage;
    int        i;
    int        flg;
    P3LOG_VAL *pLog; /* note: not in STABS. */

    nRound = pstate->pLog->nRound;
    nStage = pstate->nStage - 1;

    if (pstate->nMode != 0) {
        pstate->endingGame = 0;
        return;
    }

    flg = 0;
    pLog = pstate->pLog;
    for (i = 0; i < PR_ARRAYSIZE(pLog->clrCount); i++) {
        if (pLog->clrCount[i] >= (nRound + 1)) {
            flg++;
        }
    }

    pstate->endingGame = 0;

    if ((flg % 2) != 0) {
        if (pLog->clrCount[nStage] < (nRound + 1)) {
            pstate->endingGame = (flg / 2) + 2;
            if (pstate->endingGame > 4) {
                pstate->endingGame = 4;
            }
        }
    }

    if (flg == 7) {
        if (pLog->clrCount[nStage] < (nRound + 1)) {
            pstate->endingGame = 1;
        }
    }
}

void TsMENU_InitSystem(void) {
    int i;

    P3MC_InitReady();
    P3MC_CheckChangeSet();

    memset(&MNS_JimakuBak, 0, sizeof(MNS_JimakuBak));
    memset(&MNS_RepCounter, 0, sizeof(MNS_RepCounter));
    memset(&MNS_OptCounter, 0, sizeof(MNS_OptCounter));
    memset(&MNS_CityHall, 0, sizeof(MNS_CityHall));
    memset(&MNS_StageMap, 0, sizeof(MNS_StageMap));
    memset(&MNS_StageMap2, 0, sizeof(MNS_StageMap2));

    for (i = 0; i < 2; i++) {
        memset(&MNS_StgCounter[i], 0, sizeof(MNS_StgCounter[i]));
    }

    tblTex = NULL;
    UserLst = NULL;
    UserDispWork = NULL;

    UserWork = (USER_DATA*)memalign(16, sizeof(USER_DATA));
    memset(UserWork, 0, sizeof(*UserWork));

    pCStageRank = (P3MC_STAGERANK*)memalign(16, sizeof(P3MC_STAGERANK[8]));
    memset(pCStageRank, 0, sizeof(P3MC_STAGERANK[8]));

    memset(&CurFileInfo, 0, sizeof(CurFileInfo));
    CurFileInfo.logFileNo = -1;
    CurFileInfo.repFileNo = -1;
}

void TsMENU_EndSystem(void) {
    if (pCStageRank != NULL) {
        free(pCStageRank);
    }

    if (UserWork != NULL) {
        free(UserWork);
    }
}

void TsMenu_RankingClear(void) {
    int             i;
    P3MC_STAGERANK *pRank = pCStageRank;

    for (i = 0; i < 8; i++, pRank++) {
        memset(pRank, 0, sizeof(*pRank));
    }
}

void TsMenu_Init(int iniflg, P3GAMESTATE *pstate) {
    int   i;
    void *ptim2;

    pP3GameState = pstate;

    PrSetStage(0);

    P3MC_SetCheckSaveSize(1, 0x8c, 0x188);
    P3MC_SetCheckSaveSize(2, 0xa0, 0x4528);

    if (!iniflg) {
        MENUSubt_PadFontSw(0);
        TsMemCardCheck_Flow(1, 0);
    } else {
        MENUSubt_PadFontSw(1);

        MNScene_Init(&MNS_CityHall, &Scene_CityHall, 1);
        MNScene_Init(&MNS_StageMap, &Scene_StageMap, 1);

        MNScene_Init(&MNS_OptCounter, &Scene_OptCounter, 0);
        MNScene_Init(&MNS_RepCounter, &Scene_RepCounter, 0);
        MNScene_Init(&MNS_JimakuBak, &Scene_JimakuBak, 0);

        MNScene_Init(&MNS_StgCounter[0], &Scene_StgCounterLoad, 0);
        MNScene_Init(&MNS_StgCounter[1], &Scene_StgCounterSave, 0);

        MNScene_DispSw(&MNS_OptCounter, 0);
        MNScene_DispSw(&MNS_RepCounter, 0);
        MNScene_DispSw(&MNS_JimakuBak, 0);

        for (i = 0; i < 2; i++) {
            MNScene_DispSw(&MNS_StgCounter[i], 0);
        }

        tblTex = (TSTEX_INF*)malloc(sizeof(TSTEX_INF) * 104);

        for (i = 0; i < 104; i++) {
            switch (TexTable[i].flg) {
            case 0:
                TsGetTm2HedTex(TexTable[i].fno, &tblTex[i]);
                break;
            case 1:
                ptim2 = GetIntAdrsCurrent(TexTable[i].fno);
                Tim2Trans(ptim2);
                TsGetTm2Tex(ptim2, &tblTex[i]);
                break;
            case 2:
                ptim2 = GetIntAdrsCurrent(TexTable[i].fno);
                TsGetTm2Tex(ptim2, &tblTex[i]);
                break;
            }
        }
    }

    UserLst = (P3MC_USRLST*)memalign(16, sizeof(P3MC_USRLST));
    memset(UserLst, 0, sizeof(*UserLst));
    P3MC_CheckChangeSet();

    UserDispWork = (MN_USERLST_WORK*)memalign(16, sizeof(MN_USERLST_WORK));
    memset(UserDispWork, 0, sizeof(*UserDispWork));

    TsInitUPacket(&MnPkt, NULL, 0x20000);
    TsInitUPacket(&MnLPkt, NULL, 0x2000);

    TsMCAMes_Init();
    TsMCAMes_SetMes(-1);
    TsCMPMes_SetMes(-1);

    TsClearMenuPad(0);
    TsClearMenuPad(1);

    TsSCFADE_Flow(1, 0);
    TsBGMInit();

    _bMapCaptureReq = FALSE;
    UserList_Sw     = FALSE;
    OptionList_Sw   = FALSE;
    PopMenu_Sw      = FALSE;
    SaveMenu_Sw     = FALSE;
    JukeMenu_Sw     = FALSE;

    TsSndFlow(1);

    TsMENU_GetMapTimeState(1);
}

void TsMenu_End(void) {
    int i;

    TsBGMStop(0);

    TsCELBackEnd();
    TsEndUPacket(&MnLPkt);
    TsEndUPacket(&MnPkt);

    MNScene_End(&MNS_JimakuBak);
    MNScene_End(&MNS_RepCounter);
    MNScene_End(&MNS_OptCounter);

    for (i = 0; i < 2; i++) {
        MNScene_End(&MNS_StgCounter[i]);
    }

    MNScene_End(&MNS_StageMap2);
    MNScene_End(&MNS_StageMap);
    MNScene_End(&MNS_CityHall);

    if (tblTex != NULL) {
        free(tblTex);
    }

    if (UserDispWork != NULL) {
        free(UserDispWork);
    }

    if (UserLst != NULL) {
        free(UserLst);
    }
}

void TsMenu_InitFlow(P3GAMESTATE *pstate) {
    pP3GameState = pstate;

    pstate->pAutoMove = NULL;
    pstate->curRecJacket = 0;

    TsSet_ParappaCapColor();

    switch (pstate->endFlg) {
    case 0:
        TsMap_Flow(1, 3, 0);
        break;
    case 1:
        TsClearSet(pP3GameState);
        TsMap_Flow(1, 2, 0);
        break;
    case 2:
        TsMap_Flow(1, 1, 0);
        break;
    }
}

int TsMenuMemcChk_Flow(void) {
    u_int tpad, tpad2;
    int   ret;

    TsGetMenuPad(0, &tpad);
    TsGetMenuPad(1, &tpad2);

    TsMCAMes_Flow(tpad);
    ret = TsMemCardCheck_Flow(0, tpad);

    TsSCFADE_Flow(0, 0);
    TsBGMPoll();

    return ret;
}

int TsMenu_Flow(void) {
    u_int tpad, tpad2;
    int   ret;

    TsGetMenuPad(0, &tpad);
    TsGetMenuPad(1, &tpad2);

    ret = TsMap_Flow(0, tpad, tpad2);

    TsMCAMes_Flow(tpad);
    TsSCFADE_Flow(0, 0);
    TsBGMPoll();
    TsSndFlow(0);

    return ret;
}

void TsMenu_Draw(void) {
    SPR_PRM    SprPrm;
    u_long128 *pkt;
    SPR_PRM   *spr;
    SPR_PKT    pk;
    int        flg;
    int        i;

    pk  = &pkt;
    spr = &SprPrm;

    MNScene_Draw(&MNS_StageMap);
    MNScene_Draw(&MNS_StageMap2);
    MNScene_Draw(&MNS_CityHall);

    flg = TsCELBackDraw(&MnPkt, spr, MNS_OptCounter.isDisp, 0);
    MNScene_Draw(&MNS_OptCounter);
    flg |= TsCELBackDraw(&MnPkt, spr, MNS_RepCounter.isDisp, 1);
    MNScene_Draw(&MNS_RepCounter);

    for (i = 0; i < PR_ARRAYSIZE(MNS_StgCounter); i++) {
        flg |= TsCELBackDraw(&MnPkt, spr, MNS_StgCounter[i].isDisp, 2);
        MNScene_Draw(&MNS_StgCounter[i]);
    }

    if (!flg) {
        TsCELBackEnd();
    }

    TsPatTexFnc(0);

    MnPkt.ptop = PR_UNCACHED(MnPkt.pkt[MnPkt.idx].PaketTop);
    pkt = (u_long128*)MnPkt.ptop;

    PkSprPkt_SetDefault(pk, spr, DrawGetDrawEnvP(DNUM_DRAW));

    if (_bMapCaptureReq) {
        TsMenu_CaptureVram(pk, spr);
        _bMapCaptureReq = FALSE;
    }
    if (UserList_Sw) {
        TsUserList_Draw(pk, spr);
    }
    if (OptionList_Sw) {
        TsOption_Draw(pk, spr);
    }

    TsSCFADE_Draw(pk, spr, 2);

    if (PopMenu_Sw) {
        TsPopMenu_Draw(pk, spr);
    }
    if (SaveMenu_Sw) {
        TsSaveMenu_Draw(pk, spr);
    }
    if (JukeMenu_Sw) {
        TsJukeMenu_Draw(pk, spr);
    }

    MnPkt.ptop = (u_int)pkt;
    sceGsSyncPath(0, 0);

    TsDrawUPacket(&MnPkt);
    sceGsSyncPath(0, 0);

    MnPkt.ptop = PR_UNCACHED(MnPkt.pkt[MnPkt.idx].PaketTop);
    pkt = (u_long128*)MnPkt.ptop;

    PkSprPkt_SetDefault(pk, spr, DrawGetDrawEnvP(DNUM_DRAW));
    TsCMPMes_SetPos(0x140, 0xba);
    TsCMPMes_Draw(pk, spr);
    TsSCFADE_Draw(pk, spr, 1);
    TsMCAMes_SetPos(0x140, 0x65);

    TsMCAMes_Draw(pk, spr);
    MnPkt.ptop = (u_int)pkt;
    sceGsSyncPath(0, 0);

    TsDrawUPacket(&MnPkt);
    sceGsSyncPath(0, 0);

    pkt = TsCmnPkOpen(&FPacket);

    PkSprPkt_SetDefault(pk, spr, DrawGetDrawEnvP(DNUM_DRAW));
    TsSCFADE_Draw(pk, spr, 0);

    TsCmnPkClose(&FPacket, pkt, 0xf);
}

typedef struct {
    u_char name[8];
} RANK_NAME;

typedef struct {
    char name[12];
} USER_NAME;

/* static */ void TsSetRankingName(P3MC_STAGERANK *pRankTop, u_char *name) {
    int             i, k, l;
    P3MC_STAGERANK *pRank = pRankTop;

    for (l = 0; l < 8; l++, pRank++) {
        for (i = 0; i < pRank->nSplay; i++) {
            if (pRank->splay[i].name[0] == '\0') {
                *(RANK_NAME*)pRank->splay[i].name = *(RANK_NAME*)name;
            }
        }

        for (k = 0; k < 4; k++) {
            for (i = 0; i < pRank->nVplay[k]; i++) {
                if (pRank->vplay[k][i].name[0] == '\0') {
                    *(RANK_NAME*)pRank->vplay[k][i].name = *(RANK_NAME*)name;
                }
            }
        }
    }
}

static void TsSetRanking2UData(USER_DATA *puser, P3MC_STAGERANK *wkRank) {
    int i;

    for (i = 0; i < PR_ARRAYSIZE(puser->stageRank); i++) {
        puser->stageRank[i] = wkRank[i];
    }
}

/* static */ void TsSetSaveData(MCRWDATA_HDL *pDataW, int mode, USER_DATA *puser) {
    P3LOG_VAL      *plog;
    P3MC_STAGERANK *pRank;
    int             nStage;
    P3MC_RANKSCORE *pScore;
    int             vsLev;

    if (pDataW->pMemTop != NULL) {
        switch (mode) {
        case 1:
            pP3GameState->pLog->game_status = *pP3GameState->pGameStatus;
            memcpy(pDataW->pData, pP3GameState->pLog, sizeof(P3LOG_VAL));

            plog = (P3LOG_VAL*)pDataW->pData;
            *(USER_NAME*)plog->name = *(USER_NAME*)puser->name;
            plog->name[11] = '\0';

            TsSetRanking2UData(&pDataW->pHead->user, pCStageRank);
            TsSetRankingName(pDataW->pHead->user.stageRank, plog->name);
            break;
        case 2:
            memcpy(pDataW->pData, pP3GameState->pReplayArea, sizeof(MC_REP_STR));

            pRank = pDataW->pHead->user.stageRank;
            nStage = puser->stageNo - 1;
            memset(pRank, 0, sizeof(pDataW->pHead->user.stageRank));

            if (nStage >= 0 && nStage < 8) {
                switch (puser->isVs) {
                case 2:
                    vsLev = puser->vsLev;
                    pRank[nStage].nVplay[vsLev] = 1;
                    pScore = pRank[nStage].vplay[vsLev];
                    *pScore = CurRankScore;
                    break;
                case 0:
                    pRank[nStage].nSplay = 1;
                    pScore = pRank[nStage].splay;
                    *pScore = CurRankScore;
                    break;
                }
            }

            TsSetRankingName(pRank, puser->name1);
            break;
        }
    }
}

/* static */ void TsRestoreSaveData(MCRWDATA_HDL *pDataW, int mode) {
    int i;

    if (pDataW->pMemTop != NULL) {
        switch (mode) {
        case 1:
            memcpy(UserWork, &pDataW->pHead->user, sizeof(USER_DATA));
            memcpy(pP3GameState->pLog, pDataW->pData, sizeof(P3LOG_VAL));
            *pP3GameState->pGameStatus = pP3GameState->pLog->game_status;
            CurFileInfo.logFileNo = pDataW->pHead->user.fileNo;

            for (i = 0; i < 8; i++) {
                memcpy(&pCStageRank[i], &UserWork->stageRank[i], sizeof(P3MC_STAGERANK));
            }
            break;
        case 2:
            memcpy(pP3GameState->pReplayArea, pDataW->pData, sizeof(MC_REP_STR));

            switch (pDataW->pHead->user.isVs) {
            case 0:
                pP3GameState->nMode = 0;
                pP3GameState->vsLev = 0;
                break;
            case 1:
                pP3GameState->nMode = 1;
                pP3GameState->vsLev = 0;
                break;
            case 2:
                pP3GameState->nMode = 2;
                pP3GameState->vsLev = pDataW->pHead->user.vsLev;
                if (pP3GameState->vsLev >= 4) {
                    pP3GameState->vsLev = 3;
                }
                break;
            }

            pP3GameState->nStage = pDataW->pHead->user.stageNo;
            CurFileInfo.repFileNo = pDataW->pHead->user.fileNo;
            break;
        }
    }
}

int DateChgInt(u_int n) {
    /* Convert BCD to decimal */
    return 
    (
        ((n & 0xf0) >> 4) * 10 +
         (n & 0xf)
    );
}

void GetRankScoreID(MAP_TIME *mptim, u_int *dat) {
    int year   = DateChgInt(mptim->year);
    int second = DateChgInt(mptim->second);
    int hour   = DateChgInt(mptim->hour);
    int day    = DateChgInt(mptim->day);
    int month  = DateChgInt(mptim->month);
    int minute = DateChgInt(mptim->minute);

    dat[0] = (year % 50) * (12 * 31 * 24 * 60 * 60) + (month % 12) * (31 * 24 * 60 * 60) +
             (day % 31) * (24 * 60 * 60) + (hour % 24) * (60 * 60) + (minute % 60) * 60 + (second % 60);
    dat[1] = ((rand() % 0x10000) << 8) + mptim->pad;
}

/* static */ int TsRanking_Set(void) {
    P3GAMESTATE    *pstate;
    P3MC_RANKSCORE *pScore;
    u_int           score;
    int             i;
    int             l;
    int             nStage;
    int            *pNRank;
    int             nRank;
    int             RankMAX;
    int             vsLev;

    pstate = pP3GameState;
    nStage = pstate->nStage;
    memset(&CurRankScore, 0, sizeof(CurRankScore));

    if (nStage < 1 || nStage > 8 || pstate->nMode == 1) {
        return -1;
    }

    if (pstate->nMode == 2) {
        vsLev   = pstate->vsLev;
        RankMAX = 10;
        score   = pstate->score;
        pNRank  = &pCStageRank[nStage - 1].nVplay[vsLev];
        pScore  = pCStageRank[nStage - 1].vplay[vsLev];
        if (pstate->winPlayer > 0) {
            return -1;
        }
    } else {
        RankMAX = 20;
        score   = pstate->score + pstate->bonusG;
        pNRank  = &pCStageRank[nStage - 1].nSplay;
        pScore  = pCStageRank[nStage - 1].splay;
    }

    GetRankScoreID(&MapTime, CurRankScore.scDate);
    CurRankScore.name[0] = '\0';
    CurRankScore.score = score;

    nRank = *pNRank;
    i = 0;
    if (nRank < RankMAX) {
        for (i = 0; i < nRank; i++) {
            if (pScore[i].score < score) {
                break;
            }
        }
        *pNRank = nRank + 1;
    } else {
        for (i = 0; i < RankMAX; i++) {
            if (pScore[i].score < score) {
                break;
            }
        }
    }

    if (i < RankMAX) {
        l = i;
        for (i = RankMAX - 1; l < i; i--) {
            pScore[i] = pScore[i - 1];
        }
        pScore[l] = CurRankScore;
        return l;
    }

    return -1;
}

void TsMENU_SetMapScreen(int mapNo) {
    CurMapNo = mapNo;
    CurMapOldFlg = CurMapBakFlg;

    TsSetScene_Map(&MNS_StageMap, mapNo, CurMapBakFlg, 1);

    CurMapState = 0;
}

static int TsCheckTimeMapChange() {
    if (CurMapState == 0 && CurMapBakFlg != CurMapOldFlg) {
        if (MNScene_isSeniAnime(&MNS_StageMap)) {
            return 0;
        }

        _bMapCaptureReq = TRUE;
        CurMapState = 1;
        return 1;
    }

    switch (CurMapState) {
    case 0:
        TsMENU_GetMapTimeState(0);
        break;
    case 1:
        TsSetScene_Map(&MNS_StageMap2, CurMapNo, CurMapBakFlg, 0);
        MNScene_CopyState(&MNS_StageMap2, &MNS_StageMap);
        TsMENU_SetMapScreen(CurMapNo);
        MNScene_CopyState(&MNS_StageMap, &MNS_StageMap2);
        MNScene_DispSw(&MNS_StageMap, 0);
        MNScene_DispSw(&MNS_StageMap2, 1);
        CurMapState = 2;
        /* fallthrough */
    case 2:
        if (TsSCFADE_Set(5, 30, 2) == 0) {
            MNScene_DispSw(&MNS_StageMap2, 0);
            MNScene_End(&MNS_StageMap2);
            MNScene_DispSw(&MNS_StageMap, 1);
            CurMapState = 0;
        }
        break;
    }

    return CurMapState;
}

int TsAnimeWait_withKeySkip(u_int tpad, MN_SCENE *scene, int ltim, u_int bnk) {
    if (bnk == -1) {
        return MNScene_isAnime(scene, ltim);
    } else {
        return MNScene_isAnimeBank(scene, ltim, bnk);
    }
}

static int TsMemCardCheck_Flow(int flg, u_int tpad) {
    static int state;
    static int mesNo;
    int ret;

    if (flg == 1) {
        McInitFlow();
        state = 0;
        mesNo = -1;
        McStartCheckFlow(1);
        TsMCAMes_SetMes(-1);
        return 0;
    }

    switch (state) {
    case 0:
        ret = McStartCheckFlow(0);
        if (ret == 0) {
            state = 0x1000;
            return 0;
        }

        if (ret >= 0) {
            if (mesNo != ret) {
                mesNo = ret;

                switch (ret) {
                case 1:
                    TsMCAMes_SetMes(MCMES(MCMES_KIND_OK, 1) | MCMES_NOPLATE | MCMES_COLOR);
                    break;
                case 2:
                    TsMCAMes_SetMes(MCMES(MCMES_KIND_OK, 0) | MCMES_NOPLATE | MCMES_COLOR);
                    break;
                }
            }

            if (TsMCAMes_GetSelect()) {
                state = 0x1000;
            }
        }

        break;
    case 0x1000:
        if (TsSCFADE_Set(2, 0xf, 0)) {
            return 0;
        }
        TsMCAMes_SetMes(-1);
        McStartCheckFlow(2);
        P3MC_CheckChangeSet();
        _MNwaitTime = 30;
        state = 0x1010;
    /* fallthrough */
    case 0x1010:
        if (--_MNwaitTime <= 0) {
            return 1;
        }
        break;
    }

    return 0;
}

static int TsMap_Flow(int flg, u_int tpad, u_int tpad2) {
    /* TODO: Fix names once made static. */
    static int state;
    static MAPPOS MapCity;
    int ret;
    int mn;

    if (flg == 1) {
        switch (tpad) {
        case 0:
            pP3GameState->nStage = 0;
            MenuVoiceBankSet(0);
            state = 0;
            break;
        case 1:
            MenuVoiceBankSet(0);
            state = 0x6500;
            break;
        case 2:
            MenuVoiceBankSet(0);
            TsBGMPlay(1, 0x14);
            state = 0x2000;
            break;
        case 3:
            MenuVoiceBankSet(0);
            state = 0;
            break;
        }

        TsMENU_GetMapTimeState(1);
        return 0;
    }

    switch (state) {
    case 0:
        CurMapOldFlg = -1;
        mn = TsMENU_GetMapNo(NULL);
        TsMENU_SetMapScreen(mn);
        MapCity.pscene = &MNS_StageMap;
        MapCity.panime = StageMapAnimePA;
        MapCity.lmtPos = mn + 1;

        if (mn < 2) {
            MapCity.mnmap = mnmapMap1;
        } else if (pP3GameState->pLog->nRound >= 4) {
            MapCity.mnmap = mnmapMap2;
        } else {
            MapCity.mnmap = mnmapMap;
        }

        MpMapMenu_Flow(1, &MapCity, 0);

        switch (pP3GameState->nStage) {
        case 1:
            mn = 1;
            break;
        case 2:
            mn = 2;
            break;
        case 3:
            mn = 3;
            break;
        case 4:
            mn = 4;
            break;
        case 5:
            mn = 5;
            break;
        case 6:
            mn = 6;
            break;
        case 7:
            mn = 7;
            break;
        case 8:
            mn = 8;
            break;
        case 9:
            mn = 9;
            break;
        case 0:
        default:
            mn = 0;
            break;
        }

        MpMapMenu_Flow(3, &MapCity, mn);
        TsCMPMes_SetMes(-1);
        TsSet_ParappaCapColor();
        if (pP3GameState->pAutoMove == NULL) {
            TsBGMPlay(MapCity.curPos + 1, 0xa);
        }
        state = 0x1000;
        /* fallthrough */
    case 0x1000:
        MNScene_DispSw(&MNS_CityHall, 0);
        MNScene_DispSw(&MNS_StageMap, 1);
        state = 0x1010;
        break;
    case 0x1010:
        if (!pP3GameState->isWipeEnd || TsBGMLoadCheck()) {
            return 0;
        }
        if (TsAnimeWait_withKeySkip(tpad, &MNS_StageMap, 0, -1)) {
            return 0;
        }
        /* fallthrough */
    case 0x1018:
        mn = 0;

        if (pP3GameState->pAutoMove != NULL) {
            switch (*pP3GameState->pAutoMove) {
            case 1:
                mn = 1;
                break;
            case 2:
                mn = 2;
                break;
            case 3:
                mn = 3;
                break;
            case 4:
                mn = 4;
                break;
            case 5:
                mn = 5;
                break;
            case 6:
                mn = 6;
                break;
            case 7:
                mn = 7;
                break;
            case 8:
                mn = 8;
                break;
            case 9:
                mn = 9;
                break;
            case 0:
                mn = 0;
                break;
            case -2:
                mn = -2;
                pP3GameState->pAutoMove = NULL;
                break;
            case -1:
            default:
                pP3GameState->pAutoMove = NULL;
                mn = 0;
                break;            
            }

            if (pP3GameState->pAutoMove != NULL) {
                MpMapMenu_Flow(4, &MapCity, mn);

                if (MapCity.sndtrg == 1) {
                    TsBGMChangePos(MapCity.curPos + 1);
                }

                pP3GameState->pAutoMove++;
                if (*pP3GameState->pAutoMove == -1) {
                    pP3GameState->pAutoMove = NULL;
                }
            }
        } 

        if (mn == -2) {
            state = 0x5010;
            break;
        }

        state = 0x1020;
        /* fallthrough */
    case 0x1020:
        if (!MapCity.bMove) {
            if (TsCheckTimeMapChange()) {
                break;
            }
        }

        ret = MpMapMenu_Flow(0, &MapCity, tpad);
        if (MapCity.anmStop != 0) {
            if (pP3GameState->pAutoMove != NULL) {
                state = 0x1018;
                break;
            }
        }

        switch (MapCity.sndtrg) {
        case 1:
            TsBGMChangePos(MapCity.curPos + 1);
            break;
        case 3:
            TSSNDPLAY(VSND_CANCEL);
            break;
        case 2:
            break;
        }

        if (ret == 0) {
            break;
        } else if (ret == 1) {
            state = 0x1100;
        } else if (ret == -1) {
            state = 0x1200;
            break;
        } else {
            break;
        }

        /* fallthrough */
    case 0x1100:
        if (MapCity.curPos == 0) {
            MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[3]);
            TSSNDPLAY(VSND_SELMODE);
            state = 0x6000;
            break;
        } else if (MapCity.curPos == 9) {
            MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[3]);
            TSSNDPLAY(VSND_SELMODE);
            state = 0x5000;
            break;
        }

        TSSNDPLAY(VSND_SELMODE);
        pP3GameState->nStage = MapCity.curPos;
        state = 0x3000;
        break;
    case 0x1200:
        TsBGMStop(0x26);
        _MNwaitTime = 40;
        TsCMPMes_SetMes(-1);
        pP3GameState->nStage = MapCity.curPos;
        state = 0x1210;
        /* fallthrough */
    case 0x1210:
        if (--_MNwaitTime <= 0) {
            return P3MRET_TOTITLE;
        }
        break;
    case 0x2000:
        if (pP3GameState->nStage < 1 || pP3GameState->nStage > 8) {
            state = 0;
            return 0;
        }
        TsRanking_Set();
        MpSave_Flow(1, 0, 0);
        state = 0x2010;
        /* fallthrough */
    case 0x2010:
        if (!pP3GameState->isWipeEnd) {
            break;
        }
        _MNwaitTime = 30;
        state = 0x2020;
        /* fallthrough */
    case 0x2020:
        if (--_MNwaitTime <= 0) {
            state = 0x2100;
        } else {
            break;
        }        
        /* fallthrough */
    case 0x2100:
        if (!MpSave_Flow(0, tpad, tpad2)) {
            return 0;
        }
        state = 0x2200;
        /* fallthrough */
    case 0x2200:
        if (TsSCFADE_Set(2, 0x1e, 0)) {
            return 0;
        }
        TsSet_ParappaCapColor();
        state = 0x2400;
        /* fallthrough */
    case 0x2400:
        TsSCFADE_Set(1, 0x1e, 0);
        state = 0;
        break;
    case 0x3000:
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[3]);
        MpPopMenu_Flow(1, 0);
        state = 0x3010;
        /* fallthrough */
    case 0x3010:
        ret = MpPopMenu_Flow(0, tpad);

        if (ret == 0) {
            break;
        } else if (ret == -1) {
            MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[0]);
            state = 0x1000;
            break;
        }

        switch (ret & 0xff) {
        case 1:
            pP3GameState->nMode = 0;
            pP3GameState->vsLev = 0;
            break;
        case 2:
            pP3GameState->nMode = 1;
            pP3GameState->vsLev = 0;
            break;
        case 3:
            pP3GameState->nMode = 2;
            pP3GameState->vsLev = ret >> 0x8;
            break;
        }

        state = 0x4000;
        break;
    case 0x4000:
        TsCheckEnding(pP3GameState);
        TsBGMStop(0x20);
        _MNwaitTime = 32;
        TsCMPMes_SetMes(-1);
        state = 0x4010;
        /* fallthrough */
    case 0x4010:
        if (--_MNwaitTime <= 0) {
            return P3MRET_PLAYGAME;
        }
        break;
    case 0x5000:
        pP3GameState->curRecJacket = 0;
        state = 0x5100;
        break;
    case 0x5010:
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[0]);
        state = 0x5018;
        _MNwaitTime = 40;
        /* fallthrough */
    case 0x5018:
        if (--_MNwaitTime <= 0) {
            TSSNDPLAY(VSND_SELMODE);
            state = 0x5100;
        }
        break;
    case 0x5100:
        pP3GameState->nStage = MapCity.curPos;
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[3]);
        TsJukeMenu_Flow(1, pP3GameState->curRecJacket);
        state = 0x5200;
        /* fallthrough */
    case 0x5200:
        if (!TsJukeMenu_Flow(0, tpad)) {
            return 0;
        }
        TsJukeMenu_Flow(2, 0);
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[0]);
        state = 0x1000;
        break;
    case 0x6000:
        MenuVoiceBankSet(0);
        state = 0x6001;
        /* fallthrough */
    case 0x6001:
        if (TsSCFADE_Set(2, 0x14, 0)) {
            return 0;
        }
        state = 0x6010;
        /* fallthrough */
    case 0x6010:
        TsBGMPlay(1, 0x14);
        TsSCFADE_Set(1, 0x14, 0);
        MpCityHall_Flow(1, 0, 0);
        state = 0x7000;
        break;
    case 0x6500:
        TsBGMPlay(1, 0x14);
        MpCityHall_Flow(1, 1, 0);
        state = 0x6510;
        /* fallthrough */
    case 0x6510:
        if (!pP3GameState->isWipeEnd) {
            break;
        }
        state = 0x7000;
        /* fallthrough */
    case 0x7000:
        ret = MpCityHall_Flow(0, tpad, tpad2);
        if (ret == 0) {
            break;
        } else if (ret == 2) {
            state = 0xff00;
        } else if (ret == 3) {
            state = 0x7020;
        } else {
            state = 0x7010;
        }
        break;
    case 0x7010:
        MenuVoiceBankSet(0);
        state = 0x7011;
        /* fallthrough */
    case 0x7011:
        if (TsSCFADE_Set(2, 0x1e, 0)) {
            return 0;
        }
        state = 0x7020;
        /* fallthrough */
    case 0x7020:
        if (TsBGMLoadCheck()) {
            return 0;
        }
        TsMap_Flow(1, 0, 0);
        TsSCFADE_Set(1, 0xf, 0);
        break;
    case 0xff00:
        return P3MRET_REPLAY;
    }

    return 0;
}

/* static */ void TsMakeUserWork(int mode) {
    int stage = pP3GameState->nStage;
    int round;
    int score;
    int i, no;

    if (stage < 0) {
        stage = 0;
    }
    if (stage > 8) {
        stage = 8;
    }

    memset(UserWork, 0, sizeof(USER_DATA));

    *(USER_NAME*)UserWork->name = *(USER_NAME*)pP3GameState->pLog->name;
    UserWork->name[11] = 0;

    if (mode == 2) {
        *(USER_NAME*)UserWork->name1 = *(USER_NAME*)pP3GameState->pLog->name1;
        UserWork->name1[11] = 0;

        if (pP3GameState->nMode == mode) {
            no = (stage > 8) ? 8 : stage;
            if (no > 0) {
                no--;
            }
            *(USER_NAME*)UserWork->name2 = *(USER_NAME*)TeachersName_Tbl[no];
            UserWork->name2[11] = 0;
        } else {
            *(USER_NAME*)UserWork->name2 = *(USER_NAME*)pP3GameState->pLog->name2;
            UserWork->name2[11] = 0;
        }
    }

    UserWork->mode = mode;
    UserWork->flg = 1;

    round = pP3GameState->pLog->nRound;
    for (i = 0; i < 8; i++) {
        if (round < pP3GameState->pLog->clrCount[i]) {
            break;
        }
    }
    if (i >= 8) {
        round--;
    }

    UserWork->stageNo = stage;
    score = pP3GameState->score;
    if (score < 0) {
        score = 0;
    }
    UserWork->score = score;
    if (round < 0) {
        round = 0;
    }
    if (round > 98) {
        round = 98;
    }
    UserWork->roundNo = round;
    UserWork->score2 = pP3GameState->score2P;

    if (pP3GameState->nMode == 1 || pP3GameState->nMode == 2) {
        UserWork->winner = pP3GameState->winPlayer;
    } else {
        UserWork->winner = 0;
    }

    UserWork->fileNo = 0;
    if (UserWork->mode == 1) {
        int mapNo = TsMENU_GetMapNo(NULL) - 1;
        if (mapNo <= 0) {
            mapNo = 0;
        }
        UserWork->stageNo = mapNo;
    }

    switch (pP3GameState->nMode) {
    case 0:
        UserWork->isVs = 0;
        UserWork->vsLev = 0;
        break;
    case 1:
        UserWork->isVs = 1;
        UserWork->vsLev = 0;
        break;
    case 2:
        UserWork->isVs = 2;
        UserWork->vsLev = pP3GameState->vsLev;
        if (UserWork->vsLev > 3) {
            UserWork->vsLev = 3;
        }
        break;
    }
}

/* static */ void TsSaveSuccessProc(void) {
    TsUserList_SetCurUserData(UserWork);

    *(USER_NAME*)pP3GameState->pLog->name = *(USER_NAME*)UserWork->name;
    pP3GameState->pLog->name[11] = 0;

    if (UserWork->mode == 2) {
        *(USER_NAME*)pP3GameState->pLog->name1 = *(USER_NAME*)UserWork->name1;
        pP3GameState->pLog->name1[11] = 0;

        if (UserWork->isVs == 1) {
            *(USER_NAME*)pP3GameState->pLog->name2 = *(USER_NAME*)UserWork->name2;
            pP3GameState->pLog->name2[11] = 0;
        }
    }

    if (UserWork->mode == 1) {
        TsSetRankingName(pCStageRank, pP3GameState->pLog->name);
    } else {
        TsSetRankingName(pCStageRank, pP3GameState->pLog->name1);
    }

    TsSetRanking2UData(UserWork, pCStageRank);

    if (UserWork->mode == 1) {
        CurFileInfo.logFileNo = UserWork->fileNo;
        CurFileInfo.logDate = UserWork->date;
    } else {
        CurFileInfo.repFileNo = UserWork->fileNo;
        CurFileInfo.repDate = UserWork->date;
    }
}

static int MpSave_Flow(int flg, u_int tpad, u_int tpad2) {
    static int state;
    static int saveSel;
    static int waitTime;
    int chkMode;
    int ret;

    if (flg == 1) {
        if (tpad == 0) {
            saveSel = 1;
        } else {
            saveSel = tpad;
        }
        CurMapOldFlg = -1;
        MNScene_DispSw(&MNS_CityHall, 0);
        MNScene_DispSw(&MNS_StageMap, 1);
        TsMENU_SetMapScreen(0);
        TsCMPMes_SetMes(-1);
        state = 0;
        return 0;
    }

    switch (state) {
    case 0:
        state = 0x1000;
    case 0x1000:
        TsSaveMenu_Flow(1, saveSel - 1);
        TsUserList_Flow(1, 0, 0);
        state = 0x1010;
    case 0x1010:
        ret = TsSaveMenu_Flow(0, tpad);
        if (ret != 0) {
            saveSel = ret;
            if (ret >= 3) {
                state = 0xf000;
            } else if (ret <= 0) {
                state = 0xf000;
            } else {
                state = 0x2000;
            }
        }
        break;
    case 0x2000:
        TsCMPMes_SetMes(-1);
        McInitFlow();
        state = 0x2010;
    case 0x2010:
        chkMode = (saveSel != 1) ? 2 : 1;
        ret = McUserCheckFlow(MCCHECK_SAVE, chkMode, NULL);
        if (ret < 0) {
            break;
        }
        if (ret == 1 || ret == 2) {
            TsMENU_GetMapTimeState(1);
            MpSave_Flow(1, saveSel, 0);
            if (UserList_Sw != 0) {
                state = 0x5000;
            } else {
                state = 0;
            }
        } else {
            state = 0x2020;
        }
        break;
    case 0x2020:
        chkMode = (saveSel != 1) ? 2 : 1;
        TsMakeUserWork(chkMode);
        state = 0x2030;
    case 0x2030:
        if (TsSCFADE_Set(2, 20, 0) != 0) {
            break;
        }
        state = 0x2040;
    case 0x2040:
        MNScene_DispSw(&MNS_CityHall, 0);
        MNScene_DispSw(&MNS_StageMap, 0);
        if (saveSel == 1) {
            TsUserList_SetType(&ULTypeT_SAVE_LOG, pP3GameState->nMode, 0);
        } else {
            TsUserList_SetType(&ULTypeT_SAVE_REPLAY, pP3GameState->nMode, 0);
        }
        UserList_Sw = 1;
        state = 0x2050;
    case 0x2050:
        if (TsSCFADE_Set(1, 20, 0) >= 9) {
            break;
        }
        state = 0x2060;
    case 0x2060:
        ret = TsUserList_Flow(0, tpad, tpad2);
        if (ret != 0) {
            McInitFlow();
            if (ret != -1) {
                if (ret < 0) {
                    if (ret != -3) {
                        return 0;
                    }
                    state = 0x2200;
                } else if (ret == 1) {
                    state = 0x3100;
                }
            } else {
                state = 0x5000;
            }
        }
        break;
    case 0x2200:
        state = 0x2210;
    case 0x2210:
        state = 0x2000;
        break;
    case 0x3100:
        UserWork->fileNo = TsUserList_GetCurFileNo(NULL);
        state = 0x3110;
    case 0x3110:
        ret = McUserSaveFlow(UserWork);
        if (ret < 0) {
            TsUserList_SetCurDispUserData(UserWork);
            break;
        }
        if (ret != 0) {
            if (ret == MCFLOW_BROKEN) {
                state = 0x2020;
            }
            if (ret == MCFLOW_FAILED) {
                state = 0x5000;
            }
            if (ret == MCFLOW_CARD_CHANGED) {
                state = 0x2200;
            }
            break;
        }
    case 0x4000:
    case 0x4010:
        if (saveSel == 1) {
            saveSel = 2;
        } else {
            saveSel = 1;
        }
        TsSaveSuccessProc();
        waitTime = 0;
        state = 0x4020;
    case 0x4020:
        TsCMPMes_SetMes(-1);
        if (++waitTime < 35) {
            break;
        }
        waitTime = 0;
        state = 0x5000;
    case 0x5000:
        if (TsSCFADE_Set(2, 20, 0) != 0) {
            break;
        }
        TsUserList_Flow(2, 0, 0);
        UserList_Sw = 0;
        TsMENU_GetMapTimeState(1);
        MpSave_Flow(1, saveSel, 0);
        state = 0x5020;
    case 0x5020:
        if (TsSCFADE_Set(1, 20, 0) < 9) {
            state = 0;
        }
        break;
    case 0xf000:
        if (pP3GameState->pAutoMove == NULL) {
            TsBGMMute(20);
        }
        state = 0xf005;
    case 0xf005:
        if (TsSCFADE_Set(2, 30, 0) >= 2) {
            break;
        }
        state = 0xf010;
    case 0xf010:
        if (pP3GameState->pLog->name[0] != '\0') {
            TsSetRankingName(pCStageRank, pP3GameState->pLog->name);
            TsSetRanking2UData(UserWork, pCStageRank);
        } else if (pP3GameState->pLog->name1[0] != '\0') {
            TsSetRankingName(pCStageRank, pP3GameState->pLog->name1);
            TsSetRanking2UData(UserWork, pCStageRank);
        }
        TsMENU_GetMapTimeState(1);
        return 1;
    }

    return 0;
}

static int MpCityHall_Flow(int flg, u_int tpad, u_int tpad2) {
    /* TODO: Fix names once made static. */
    static int state;
    static int waitTime;
    static MAPPOS MapCHall;
    static int scstate;
    static int scstPos;
    static int curTag;
    static int fphs_pos;
    static u_int AnmBit;
    int chkType, chkMode;
    int bBroken;
    int ret;
    int anmno;
    int cmpmesNo;
    int isError;
    int ntag;

    if (flg == 1) {
        switch (tpad) {
        case 0:
            MNScene_DispSw(&MNS_StageMap, 0);
            MNScene_DispSw(&MNS_CityHall, 1);
            AnmBit = MNScene_StartAnime(&MNS_CityHall, -1, CityHallAnime);
            TsCMPMes_SetMes(-1);
            MapCHall.pscene = &MNS_CityHall;
            MapCHall.panime = CityHallAnime;
            MapCHall.mnmap = mnmapCityHall;
            MpMapMenu_Flow(1, &MapCHall, 0);
            MapCHall.curPos = 0;
            waitTime = 0;
            state = 0;
            scstate = 0;
            scstPos = 0;
            MpCityHallCharPosSet(0);
            fphs_pos = 0;
            MpCityHallFPHSSoundMask(0);
            TSSNDPLAY(TSSND_SEQ_FLAG | 1);
            return 0;
        case 1:
            MNScene_DispSw(&MNS_StageMap, 0);
            MNScene_DispSw(&MNS_CityHall, 1);
            MapCHall.pscene = &MNS_CityHall;
            MapCHall.panime = CityHallAnime;
            MapCHall.mnmap = mnmapCityHall;
            MpMapMenu_Flow(1, &MapCHall, 0);
            MpMapMenu_Flow(3, &MapCHall, 2);
            TsCMPMes_SetMes(-1);
            scstate = 0;
            state = 0x100;
            scstPos = 0;
            MpCityHallCharPosSet(2);
            fphs_pos = 2;
            return 0;
        }
    }

    switch (scstate) {
    case 0:
        if (scstPos != 0) {
            scstate = 0x2100;
        }
        break;
    case 0x100:
        if (TsSCFADE_Set(2, 0xa, 1)) {
            break;
        }
        TsUserList_Flow(2, 0, 0);
        UserList_Sw = 0;
        MNScene_DispSw(&MNS_OptCounter, 0);
        OptionList_Sw = 0;
        scstate = 0x110;
        /* fallthrough */
    case 0x110:
        MNScene_DispSw(&MNS_CityHall, 1);
        MpMapMenu_Flow(3, &MapCHall, MapCHall.curPos);
        MpCityHallCharPosSet(MapCHall.curPos);
        fphs_pos = MapCHall.curPos;
        MpCityHallFPHSSoundMask(0);
        scstate = 0x120;
        /* fallthrough */
    case 0x120:
        if (!TsSCFADE_Set(1, 0x14, 1)) {
            scstate = 0;
        }
        break;
    case 0x2000:
        if (scstPos == 0) {
            scstate = 0x100;
        }
        break;
    case 0x2100:
        anmno = -1;
        cmpmesNo = -1;

        switch (MapCHall.curPos) {
        case 0:
            anmno = 3;
            cmpmesNo = MENU_LOGLOAD_CAM;
            MpCityHallFPHOK(0);
            break;
        case 2:
            anmno = 9;
            cmpmesNo = MENU_REPLOAD_CAM;
            MpCityHallFPHOK(1);
            break;
        case 1:
            anmno = 6;
            cmpmesNo = MENU_OPT_CAM;
            MpCityHallFPHOK(2);
            break;
        }

        AnmBit = 0x80000000;
        if (anmno >= 0) {
            AnmBit |= MNScene_StartAnime(&MNS_CityHall, -1, &CityHallAnime[anmno]);
        }

        MpCityHallFPHSSoundMask(1);
        TsCMPMes_SetMes(cmpmesNo);
        scstate = 0x2200;
        /* fallthrough */
    case 0x2200:
        if (scstPos == 0) {
            scstate = 0x100;
        }
        if (!TsAnimeWait_withKeySkip(tpad, &MNS_CityHall, 0xa, AnmBit)) {
            scstate = 0x2000;
        }
        break;
    }

    fphs_pos = MpCityHallFPHSMove(MapCHall.curPos, fphs_pos);

    switch (state) {
    case 0:
        if (++waitTime == 70) {
            TsCMPMes_SetMes(MENU_HALL_INSIDE);
            TSSNDPLAY(VSND_MENU1);
        }
        if (TsAnimeWait_withKeySkip(tpad, &MNS_CityHall, 0, -1)) {
            return 0;
        }
        TSSND_SKIPPLAY(VSND_MENU1);
        TsCMPMes_SetMes(MENU_HALL_INSIDE);
        MpMapMenu_Flow(3, &MapCHall, 0);
        state = 0x100;
        /* fallthrough */
    case 0x100:
        ret = MpMapMenu_Flow(0, &MapCHall, tpad);
        if (MapCHall.anmStop != 0) {
            TSSND_SKIPSTOP(2);
        }
        if (MapCHall.anmtrg != 0) {
            MpCityHallParaStart(MapCHall.anmtrg);
        }

        switch (MapCHall.sndtrg) {
        case 1:
            TSSNDPLAY(VSND_MVCUS_LR);
            break;
        case 2:
            TSSNDPLAY(VSND_SELPOPUP);
            break;
        case 3:
            TSSNDPLAY(VSND_CANCEL);
            break;
        }

        if (ret != 0) {
            if (ret == 1) {
                switch (MapCHall.curPos) {
                case 0:
                    TSSNDSTOP(3);
                    TSSNDPLAY(VSND_MENU2);
                    break;
                case 2:
                    TSSNDSTOP(3);
                    TSSNDPLAY(VSND_MENU3);
                    break;
                case 1:
                    TSSNDSTOP(3);
                    TSSNDPLAY(VSND_MENU4);
                    break;
                }

                state = 0x1000;
            }

            if (ret == -1) {
                state = 0xf000;
            }

            return 0;
        }

        break;
    case 0x1000:
        if (MapCHall.curPos == 1) {
            state = 0x1500;
            break;
        }
        McInitFlow();
        state = 0x1010;
        /* fallthrough */
    case 0x1010:
        if (MapCHall.curPos == 0) {
            curTag = 0;
            chkMode = 1;
            chkType = MCCHECK_BOTH;
        } else {
            curTag = 0;
            chkType = MCCHECK_LOAD;
            chkMode = 2;
        }

        ret = McUserCheckFlow(chkType, chkMode, 0);
        if (ret == -2) {
            break;
        }
        if (ret > 0 && ret < 3) {
            state = 0x5000;
            break;
        }
        state = 0x1500;
        /* fallthrough */
    case 0x1500:
        scstPos = 1;
    
        switch (MapCHall.curPos) {
        case 0:
        case 2:
            state = 0x2000;
            break;
        case 1:
            state = 0x3000;
            break;
        }
    
        break;
    case 0x2000:
        if (MapCHall.curPos == 0) {
            chkMode = 1;
            chkType = MCCHECK_BOTH;
        } else {
            chkMode = 2;
            chkType = MCCHECK_LOAD;
        }

        isError = 0;
        ret = McUserCheckFlow(chkType, chkMode, &isError);
        if (ret < 0) {
            if (isError == 1) {
                scstPos = 0;
            }
            if (isError == 2) {
                scstPos = 1;
                break;
            }
            return 0;
        }
        if (ret > 0 && ret < 3) {
            state = 0x5000;
            break;
        }
        TsMCAMes_SetMes(-1);
        state = 0x2020;
        break;
    case 0x2020:
        if (scstate & 0xfff) {
            return 0;
        }
        state = 0x2021;
        /* fallthrough */
    case 0x2021:
        if (UserList_Sw) {
            if (TsSCFADE_Set(2, 0x14, 0)) {
                return 0;
            }
        }
        TsMakeUserWork(1);
        state = 0x2022;
        /* fallthrough */
    case 0x2022:
        if (MapCHall.curPos == 0) {
            if (UCheckLoadError != 0) {
                ntag = 1;
                curTag = 1;
            } else {
                ntag = curTag;
            }
            TsUserList_SetType(&ULTypeT_CITY_STGCLR, pP3GameState->nMode, ntag);
        } else {
            TsUserList_SetType(&ULTypeT_CITY_REPLAY, pP3GameState->nMode, 0);
        }

        MNScene_End(&MNS_StageMap2);
        MNScene_Init(&MNS_StageMap2, &Scene_CityHall, 0);
        MNScene_CopyState(&MNS_StageMap2, &MNS_CityHall);
        MNScene_DispSw(&MNS_CityHall, 0);
        MNScene_DispSw(&MNS_StageMap2, 2);
        state = 0x2024;
        /* fallthrough */
    case 0x2024:
        if (!UserList_Sw) {
            chkType = 5;
            TsSCFADE_Set(chkType, 0x14, 2);
        } else {
            chkType = 1;
            TsSCFADE_Set(chkType, 0x14, 0);
        }
        UserList_Sw = 1;
        state = 0x2026;
        /* fallthrough */
    case 0x2026:
        if (TsSCFADE_Set(0, 0, 0) >= 9) {
            break;
        }
        MNScene_DispSw(&MNS_CityHall, 0);
        MNScene_DispSw(&MNS_StageMap2, 0);
        MNScene_End(&MNS_StageMap2);
        state = 0x2028;
        /* fallthrough */
    case 0x2028:
        ret = TsUserList_Flow(0, tpad, tpad2);

        if (ret != 0) {
            switch (ret) {
            case -1:
                state = 0x5000;
                break;
            case 1:
                if (TsUserList_IsGetFileSave()) {
                    state = 0x2830;
                } else {
                    state = 0x2030;
                }
                break;
            case -3:
                if (TsUserList_IsGetFileSave()) {
                    curTag = 1;
                } else {
                    curTag = 0;
                }

                McInitFlow();
                state = 0x2000;
                break;
            }

            McInitFlow();
            return 0;
        }

        break;
    case 0x2030:
        if (MapCHall.curPos == 0) {
            chkMode = 1;
        } else {
            chkMode = 2;
        }

        ret = McUserLoadFlow(TsUserList_GetCurFileNo(&bBroken), chkMode, bBroken);
        if (ret < 0) {
            return 0;
        }

        waitTime = 0;
        if (ret != 0) {
            if (ret == MCFLOW_BROKEN) {
                state = 0x2028;
            }
            if (ret == MCFLOW_FAILED) {
                state = 0x2050;
            }
            if (ret != MCFLOW_CARD_CHANGED) {
                return 0;
            }
            McInitFlow();
            state = 0x2000;
            break;
        }

        TsCMPMes_SetMes(-1);
        state = 0x2040;
        /* fallthrough */
    case 0x2040:
        if (++waitTime >= 35) {
            waitTime = 0;
            TsSet_ParappaCapColor();
            if (MapCHall.curPos == 2) {
                state = 0xf010;
            } else {
                state = 0xef00;
            }
        }
        break;
    case 0x2830:
        UserWork->fileNo = TsUserList_GetCurFileNo(NULL);
        state = 0x2840;
        /* fallthrough */
    case 0x2840:
        ret = McUserSaveFlow(UserWork);
        if (ret < 0) {
            TsUserList_SetCurDispUserData(UserWork);
            return 0;
        }
        if (ret != 0) {
            if (ret == MCFLOW_BROKEN) {
                state = 0x2020;
            }
            if (ret == MCFLOW_FAILED) {
                state = 0x2900;
            }
            if (ret == MCFLOW_CARD_CHANGED) {
                McInitFlow();
                state = 0x2000;
                break;
            }
            return 0;
        }
        /* fallthrough */
    case 0x2850:
        TsSaveSuccessProc();
        waitTime = 0;
        state = 0x2860;
        /* fallthrough */
    case 0x2860:
        TsCMPMes_SetMes(-1);
        if (++waitTime >= 35) {
            waitTime = 0;
            state = 0x2900;
        } else {
            break;
        }
        /* fallthrough */
    case 0x2050:
    case 0x2900:
        state = 0x5000;
        break;
    case 0x3000:
        if (scstate & 0xfff) {
            return 0;
        }
        OptionList_Sw = 1;
        TsOption_Flow(1, tpad);
        MNScene_End(&MNS_StageMap2);
        MNScene_Init(&MNS_StageMap2, &Scene_CityHall, 0);
        MNScene_CopyState(&MNS_StageMap2, &MNS_CityHall);
        MNScene_DispSw(&MNS_CityHall, 0);
        MNScene_DispSw(&MNS_StageMap2, 2);
        MNScene_DispSw(&MNS_OptCounter, 1);
        state = 0x3005;
        /* fallthrough */
    case 0x3005:
        if (TsSCFADE_Set(5, 0x14, 2)) {
            return 0;
        }
        MNScene_DispSw(&MNS_CityHall, 0);
        MNScene_DispSw(&MNS_StageMap2, 0);
        MNScene_End(&MNS_StageMap2);
        state = 0x3010;
        /* fallthrough */
    case 0x3010:
        ret = TsOption_Flow(0, tpad);
        if (ret != 0) {
            if (ret == -1) {
                state = 0x3f00;
            }
            if (ret == 1) {
                state = 0x3f00;
            }
            return 0;
        }
        break;
    case 0x3f00:
        state = 0x5000;
        /* fallthrough */
    case 0x5000:
        scstPos = 0;
        state = 0x5008;
        break;
    case 0x5008:
        if (scstate & 0xfff) {
            return 0;
        }
        state = 0x100;
        break;
    case 0xef00:
        TSSNDSTOP(3);
        MenuVoiceBankSet(0);
        state = 0xef10;
        /* fallthrough */
    case 0xef10:
        if (TsSCFADE_Set(2, 0x14, 1)) {
            return 0;
        }
        TsUserList_Flow(2, 0, 0);
        UserList_Sw = 0;
        MNScene_DispSw(&MNS_OptCounter, 0);
        OptionList_Sw = 0;
        TSSNDSTOP(3);
        return 3;
    case 0xf000:
        TSSNDSTOP(3);
        return 1;
    case 0xf010:
        TSSNDSTOP(3);
        scstPos = 0;
        state = 0xf014;
        break;
    case 0xf014:
        TsCMPMes_SetMes(-1);
        if (scstate & 0xfff) {
            return 0;
        }
        TsBGMStop(12);
        _MNwaitTime = 10;
        state = 0xf018;
        /* fallthrough */
    case 0xf018:
        if (--_MNwaitTime <= 0) {
            TSSNDSTOP(3);
            return 2;
        }
        break;
    }

    return 0;
}

/* static */ void MpCityHallParaStart(int pos) {
    short *ptr = NULL;
    int    n;

    switch (pos) {
    case 1:
        ptr = AnmCHallPara_Opt;
        break;
    case 2:
        ptr = AnmCHallPara_Rep;
        break;
    case 3:
        ptr = AnmCHallPara_OptRet;
        break;
    case 4:
        ptr = AnmCHallPara_RepRet;
        break;
    }

    if (ptr == NULL) {
        return;
    }

    while ((n = *ptr) != -1) {
        if (n & 0x1000) {
            MNScene_ContinueAnime(&MNS_CityHall, -1, &CityHallAnime[n & ~0x1000]);
        } else {
            MNScene_StartAnime(&MNS_CityHall, -1, &CityHallAnime[n]);
        }
        ptr++;
    }
}

static void MpCityHallFPHSSoundMask(int flg) {
    TSSNDMASK_CHAN(3, flg);
}

/* static */ int MpCityHallFPHSMove(int pos, int fpos) {
    short *ptr = NULL;
    int    n;

    if (pos == fpos) {
        if (!TsAnimeWait_withKeySkip(0, &MNS_CityHall, 0, 6)) {
            if (TSSND_CHANISSTOP(3)) {
                TSSNDPLAY(TSSND_SEQ_FLAG | 2);
            }
        }
        return fpos;
    }

    if (TsAnimeWait_withKeySkip(0, &MNS_CityHall, 2, 6)) {
        return fpos;
    }

    if (pos > 0 && fpos > 0) {
        pos = 0;
    }

    switch (fpos) {
    case 0:
        switch (pos) {
        case 1:
            ptr = AnmCHallFphs_Opt;
            break;
        case 2:
            ptr = AnmCHallFphs_Rep;
            break;
        }
        break;
    case 1:
        if (pos == 0) {
            ptr = AnmCHallFphs_OptRet;
        }
        break;
    case 2:
        if (pos == 0) {
            ptr = AnmCHallFphs_RepRet;
        }
        break;
    }

    if (ptr == NULL) {
        return fpos;
    }

    TSSNDPLAY(0x34);
    while ((n = *ptr) != -1) {
        if (n & 0x1000) {
            MNScene_ContinueAnime(&MNS_CityHall, -1, &CityHallAnime[n & ~0x1000]);
        } else {
            MNScene_StartAnime(&MNS_CityHall, -1, &CityHallAnime[n]);
        }
        ptr++;
    }

    return pos;
}

static void MpCityHallFPHOK(int flg) {
    int n;

    TSSNDMASK_CHAN(3, flg);

    switch (flg) {
    case 0:
        n = 0x10;
        break;
    case 1:
        n = 0x11;
        break;
    case 2:
    default:
        n = 0x12;
        break;
    }

    if (!TsAnimeWait_withKeySkip(0, &MNS_CityHall, 1, 6)) {
        MNScene_ContinueAnime(&MNS_CityHall, 1, NULL);
        MNScene_StartAnime(&MNS_CityHall, -1, &CityHallAnime[n]);
    } else {
        MNScene_ContinueAnime(&MNS_CityHall, -1, &CityHallAnime[n]);
    }
}

/* static */ void MpCityHallCharPosSet(int pos) {
    short *ptr = NULL;
    u_int  AnmBit;
    int    n;

    switch (pos) {
    case 0:
        ptr = AnmCHallChar_Log;
        break;
    case 1:
        ptr = AnmCHallChar_Opt;
        break;
    case 2:
        ptr = AnmCHallChar_Rep;
        break;
    }

    AnmBit = 0x80000000;
    while ((n = *ptr) != -1) {
        AnmBit |= MNScene_StartAnime(&MNS_CityHall, -1, &CityHallAnime[n & ~0x1000]);
        ptr++;
    }

    MNScene_SetAnimeBankEnd(&MNS_CityHall, AnmBit);
    TSSNDPLAY(TSSND_SEQ_FLAG | 2);
}

static int MpPopMenu_Flow(int flg, u_int tpad) {
    static int state;
    int ret;
    int mpsize;

    if (flg == 1) {
        state = 0;
        return 0;
    }

    switch (state) {
    case 0:
        state = 0x1000;
        break;
    case 0x1000:
        TsMENU_GetMapNo(&mpsize);
        TsPopMenu_Flow(1, mpsize);
        state = 0x1010;
    /* fallthrough */
    case 0x1010:
        ret = TsPopMenu_Flow(0, tpad);
        if (ret == 0) {
            break;
        }

        TsPopMenu_Flow(2, 0);

        if (ret == -1) {
            state = 0xf000;
            break;
        }

        if (ret < 0) {
            state = 0xf000;
            break;
        }

        return ret;
    case 0xf000:
        return -1;
    }

    return 0;
}

static int MpMapMenu_Flow(int flg, MAPPOS *mpw, u_int tpad) {
    int       state;
    int       bkNo;
    int       posNo;
    int       idx;
    MNMAPPOS *mpos;

    if (flg == 1) {
        mpw->state = 0;
        mpw->anmStop = 0;
        mpw->bMove = 0;
        mpw->curPos = 0;
        mpw->mvFlag = 0;
        MENUSubt_PadFontArrowSet(0);
        return 0;
    }

    if (flg == 3) {
        mpw->curPos = tpad;
        if (mpw->mnmap != NULL) {
            mpos = &mpw->mnmap[tpad];
            mpw->mvFlag = _MapGetMovableDir(mpw);
            MENUSubt_PadFontArrowSet(mpw->mvFlag);
            TsCMPMes_SetMes(mpos->cmpmes);

            if (mpw->pscene != NULL && mpw->panime != NULL) {
                int anmNo;

                anmNo = mpos->posanm0;
                mpw->anmBit = 0x80000000;
                if (anmNo != -1) {
                    mpw->anmBit |= MNScene_StartAnime(mpw->pscene, -1, &mpw->panime[anmNo]);
                }
                anmNo = mpos->posanm1;
                if (anmNo != -1) {
                    mpw->anmBit |= MNScene_StartAnime(mpw->pscene, -1, &mpw->panime[anmNo]);
                }
                MNScene_SetAnimeEnd(mpw->pscene);
            }
        }
        mpw->anmtrg = 0;
        return 0;
    }

    if (flg == 4) {
        bkNo = mpw->curPos;
        if (mpw->mnmap != NULL) {
            mpos = &mpw->mnmap[bkNo];
            for (idx = 0; idx < 4; idx++) {
                if (tpad == mpos->mapdir[idx].mapNo) {
                    break;
                }
            }

            if (idx < 4) {
                posNo = mpos->mapdir[idx].mapNo & ~MNMAP_LIMITED;
                if (mpw->lmtPos != 0) {
                    if (posNo >= mpw->lmtPos) {
                        return 0;
                    }
                    if (mpos->mapdir[idx].mapNo & MNMAP_LIMITED) {
                        if (bkNo == mpw->lmtPos - 1) {
                            return 0;
                        }
                        if (posNo == mpw->lmtPos - 1) {
                            return 0;
                        }
                    }
                }

                mpw->curPos = posNo;
                mpw->anmLtim = mpos->mapdir[idx].anmLtim;
                if (mpw->pscene != NULL && mpw->panime != NULL) {
                    int anmNo;

                    anmNo = mpos->movanm1;
                    mpw->bMove = 1;
                    mpw->anmBit = 0x80000000;
                    if (anmNo != -1) {
                        mpw->anmBit |= MNScene_StartAnime(mpw->pscene, -1, &mpw->panime[anmNo]);
                    }
                    anmNo = mpos->mapdir[idx].anmNo;
                    if (anmNo != -1) {
                        mpw->anmBit |= MNScene_StartAnime(mpw->pscene, -1, &mpw->panime[anmNo]);
                    }
                }
                mpw->anmtrg = mpos->mapdir[idx].exflg;
                mpw->sndtrg = 1;
                mpw->state = 0x1100;
                return 0;
            }
        }
        return 0;
    }

    if (mpw->mnmap == NULL) {
        return 0;
    }

    state = mpw->state;
    posNo = mpw->curPos;
    mpw->anmtrg = 0;
    mpw->sndtrg = 0;
    mpw->anmStop = 0;

    switch (state) {
    case 0:
        mpw->bMove = 0;
        state = 0x1000;
    case 0x1000:
        mpos = &mpw->mnmap[posNo];
        if (mpw->bMove != 0 && !TsAnimeWait_withKeySkip(tpad, mpw->pscene, 0, mpw->anmBit)) {
            mpw->bMove = 0;
            if (mpw->pscene != NULL && mpw->panime != NULL) {
                int anmNo;

                anmNo = mpos->posanm0;
                mpw->anmBit = 0x80000000;
                if (anmNo != -1) {
                    mpw->anmBit |= MNScene_StartAnime(mpw->pscene, -1, &mpw->panime[anmNo]);
                }
                anmNo = mpos->posanm1;
                if (anmNo != -1) {
                    mpw->anmBit |= MNScene_StartAnime(mpw->pscene, -1, &mpw->panime[anmNo]);
                }
                MNScene_SetAnimeBankEnd(mpw->pscene, mpw->anmBit);
            }
        }

        idx = -1;
        mpw->mvFlag = _MapGetMovableDir(mpw);
        MENUSubt_PadFontArrowSet(mpw->mvFlag);
        TsCMPMes_SetMes(mpos->cmpmes);

        if (tpad & SCE_PADRright) {
            state = 0x1200;
            mpw->sndtrg = 2;
        } else if (tpad & SCE_PADRdown) {
            state = 0x1f00;
            mpw->sndtrg = 3;
        }

        if (tpad & SCE_PADLleft) {
            idx = 0;
        } else if (tpad & SCE_PADLright) {
            idx = 1;
        } else if (tpad & SCE_PADLup) {
            idx = 2;
        } else if (tpad & SCE_PADLdown) {
            idx = 3;
        }

        if (idx >= 0 && ((mpw->mvFlag >> idx) & 1)) {
            posNo = mpos->mapdir[idx].mapNo;
            if (posNo != -1) {
                posNo &= ~MNMAP_LIMITED;
                mpw->curPos = posNo;
                mpw->anmLtim = mpos->mapdir[idx].anmLtim;
                if (mpw->pscene != NULL && mpw->panime != NULL) {
                    int anmNo;

                    anmNo = mpos->movanm1;
                    mpw->bMove = 1;
                    mpw->anmBit = 0x80000000;
                    if (anmNo != -1) {
                        mpw->anmBit |= MNScene_StartAnime(mpw->pscene, -1, &mpw->panime[anmNo]);
                    }
                    anmNo = mpos->mapdir[idx].anmNo;
                    if (anmNo != -1) {
                        mpw->anmBit |= MNScene_StartAnime(mpw->pscene, -1, &mpw->panime[anmNo]);
                    }
                }
                state = 0x1100;
                mpw->anmtrg = mpos->mapdir[idx].exflg;
                mpw->sndtrg = 1;
            }
        }
        break;
    case 0x1100:
        if (!TsAnimeWait_withKeySkip(tpad, mpw->pscene, mpw->anmLtim, mpw->anmBit)) {
            mpw->anmStop = 1;
            state = 0x1000;
        }
        break;
    case 0x1200:
        state = 0x2000;
        break;
    case 0x2000:
    case 0xf000:
        mpw->state = 0;
        MENUSubt_PadFontArrowSet(0);
        return 1;
    case 0x1f00:
    case 0xf010:
        mpw->state = 0;
        MENUSubt_PadFontArrowSet(0);
        return -1;
    }

    mpw->state = state;
    return 0;
}

static int _MapGetMovableDir(MAPPOS *mpw) {
    int       posNo, ret, DirMask;
    MNMAPPOS *mpos;
    int       flg, i;

    DirMask = 1;
    mpos = &mpw->mnmap[mpw->curPos];

    ret = 0;

    for (i = 0; i < PR_ARRAYSIZE(mpos->mapdir); i++, DirMask <<= 1) {
        flg = TRUE;
        posNo = mpos->mapdir[i].mapNo;

        if (posNo == -1) {
            flg = FALSE;
        } else {
            posNo &= ~MNMAP_LIMITED;
            if (mpw->lmtPos != 0) {
                if (posNo >= mpw->lmtPos) {
                    flg = FALSE;
                } else {
                    if (mpos->mapdir[i].mapNo & 0x8000) {
                        if (mpw->curPos == (mpw->lmtPos - 1) || posNo == (mpw->lmtPos - 1)) {
                            flg = FALSE;
                        }
                    }
                }
            }
        }

        if (flg) {
            ret |= DirMask;
        }
    }

    return ret;
}

/* static */ int McErrorMess(int err) {
    int mes;

    if (!TsMCAMes_IsON()) {
        switch (err) {
        case 2:
            mes = 0x2000008;
            break;
        case 3:
            mes = 0x2000002;
            break;
        case 4:
            mes = 0x2000005;
            break;
        case 1:
        case 5:
            mes = 0x2000014;
            break;
        case 6:
            mes = 0x2000009;
            break;
        case 7:
            mes = 0x200000d;
            break;
        case 15:
            mes = 0x200000e;
            break;
        case 10:
            mes = 0x2000012;
            break;
        case 12:
            mes = 0x2000004;
            break;
        case 40:
            mes = 0x2000006;
            break;
        case 50:
            mes = 0x200000b;
            break;
        case 60:
            mes = 0x200000c;
            break;
        case 70:
            mes = 0xc000018;
            break;
        case 80:
            mes = 0xc000017;
            break;
        case 100:
            mes = 0xc00000a;
            break;
        case 200:
            mes = 0xc000015;
            break;
        default:
            return 0;
        }

        TsMCAMes_SetMes(mes);
    }

    if (TsMCAMes_GetSelect() > 0) {
        TsMCAMes_SetMes(-1);
        return 0;
    }
    return -1;
}

static void McInitFlow(void) {
    subStatus = 0;
    ret       = 0;
    errorNo   = 0;
    waitTime  = 0;
}

static int McStartCheckFlow(/* a0 4 */ int flg) {
    /* v1 3 */ int ret;

    if (flg == 1) {
        McInitFlow();
        P3MC_OpeningCheckStart();
        return 0;
    }

    if (flg == 2) {
        P3MC_OpeningCheckEnd();
        return 0;
    }

    switch (subStatus) {
    case 0:
        subStatus = 1;
        break;
    case 1:
        ret = P3MC_OpeningCheck();
        if (ret >= 0) {
            if (ret & 1) {
                ret = 0;
            } else {
                ret = 1;
            }
        }

        switch (ret) {
        case -2:
            subStatus = 0x100;
            break;
        case 0:
            subStatus = 0x100;
            break;
        case 1:
            subStatus = 0x10;
            break;
        case -3:
            subStatus = 0x20;
            break;
        }
    
        break;
    case 0x10:
        ret = P3MC_OpeningCheck();
        if (ret >= 0) {
            ret = (ret & 0x1) ^ 0x1;
        }

        switch (ret) {
        case -3:
            subStatus = 0x20;
            break;
        case -2:
        case 0:
            subStatus = 0x100;
            break;
        case -1:
        case 1:
        default:
            break;
        }

        break;
    case 0x20:
        ret = P3MC_OpeningCheck();
        if (ret >= 0) {
            ret = (ret & 0x1) ^ 0x1;
        }

        switch (ret) {
        case -2:
        case 0:
            subStatus = 0x100;
            break;
        case 1:
            subStatus = 0x10;
            break;
        case -3:
        case -1:
        default:
            break;
        }

        break;
    case 0x100:
        return 0;
    }

    switch (subStatus) {
    case 0x10:
        return 1;
    case 0x20:
        return 2;
    }

    return -1;
}

/* static */ int McUserCheckFlow(int type, int mode, int *bError) {
    static int isRun = -2;
    int flg;

    switch (subStatus) {
    case 0:
        UCheckSaveError = 0;
        isRun = -2;
        UCheckLoadError = 0;
        if (type == MCCHECK_BROWSE) {
            flg = P3MC_GetUserStart(mode, UserLst, 0);
        } else {
            flg = P3MC_GetUserStart(mode, UserLst, 1);
        }
        if (flg != 0) {
            subStatus = 0x100;
        } else {
            subStatus = MCUSER_EXIT_DONE;
        }
        break;
    case 0x100:
        isRun = -2;
        ret = P3MC_GetUserCheck();
        errorNo = 0;
        if (ret < 0) {
            if (ret == -1) {
                isRun = -1;
                subStatus = 0x102;
            }
        } else {
            errorNo = ret;
            subStatus = 0x150;
        }
        break;
    case 0x102:
        isRun = -1;
        waitTime = 90;
        TsMCAMes_SetMes(MCMES(0, 3));
        if (errorNo == 0) {
            subStatus = 0x103;
        } else {
            subStatus = 0x104;
        }
        break;
    case 0x103:
        isRun = -1;
        waitTime--;
        errorNo = P3MC_GetUserCheck();
        if (errorNo >= 0) {
            subStatus = 0x104;
        }
        break;
    case 0x104:
        isRun = -1;
        if (--waitTime > 0) {
            break;
        }
        if (type == MCCHECK_BROWSE && errorNo == 0) {
            if (P3MC_CheckBrokenUser(UserLst, mode) != 0) {
                errorNo = 80;
            }
        }
        subStatus = 0x150;
        break;
    case 0x150:
        UCheckSaveError = 0;
        UCheckLoadError = 0;
        if ((type & MCCHECK_SAVE) || type == MCCHECK_BROWSE) {
            if (errorNo == 2) {
                errorNo = 0;
                UCheckLoadError = 4;
            }
            if (type == MCCHECK_BROWSE && (errorNo == 3 || errorNo == 4 || errorNo == 5)) {
                memset(UserLst, 0, sizeof(*UserLst));
                errorNo = 0;
            }
        }
        if ((type & MCCHECK_SAVE) && errorNo == 4) {
            errorNo = 0;
            UCheckLoadError = 4;
            if (P3MC_CheckIsNewSave(mode) == 0) {
                if (mode == 2) {
                    UCheckSaveError = 15;
                } else {
                    UCheckSaveError = 7;
                }
            }
        }
        if (errorNo == 0) {
            if (type == MCCHECK_SAVE) {
                errorNo = UCheckSaveError;
            }
            if (type == MCCHECK_LOAD) {
                errorNo = UCheckLoadError;
            }
            if (type == MCCHECK_BOTH) {
                UCheckSaveError = 0;
            }
            if (errorNo == 0) {
                subStatus = MCUSER_EXIT_DONE;
                break;
            }
        }
        if (errorNo == 1) {
            if (type == MCCHECK_BROWSE) {
                errorNo = 70;
            } else {
                errorNo = 12;
            }
        }
        if (errorNo == 3) {
            subStatus = 0xe000;
        } else {
            subStatus = 0x160;
        }
        if (errorNo == 4 && mode == 2) {
            errorNo = 40;
        }
        break;
    case 0x160:
        TsMCAMes_SetMes(-1);
        if (isRun == -1 && bError != NULL) {
            *bError = 1;
        }
        subStatus = 0x165;
    case 0x165:
        ret = P3MC_CheckChange();
        if (ret == 3 || ret == 5) {
            subStatus = 0xe000;
            break;
        }
        if (McErrorMess(errorNo) < 0) {
            break;
        }
        subStatus = 0x166;
    case 0x166:
        if (P3MC_CheckChange() >= 0) {
            subStatus = 0xf0f0;
        }
        break;
    case 0xe000:
        ret = P3MC_CheckChange();
        if (ret == 0 || ret == 5) {
            if (isRun == -1 && bError != NULL) {
                *bError = 2;
            }
            subStatus = 0;
        } else {
            if (errorNo == 3 && type == MCCHECK_SAVE) {
                if (mode == 2) {
                    errorNo = 60;
                } else {
                    errorNo = 50;
                }
            }
            if (McErrorMess(errorNo) >= 0) {
                subStatus = 0xee10;
            }
        }
        break;
    case 0xee10:
        subStatus = 0xf0f0;
        break;
    case 0xf0f0:
        if (type == MCCHECK_BROWSE && (errorNo == 70 || errorNo == 80)) {
            if (errorNo == 70) {
                memset(UserLst, 0, sizeof(*UserLst));
            }
            subStatus = MCUSER_EXIT_DONE;
        } else {
            subStatus = MCUSER_EXIT_FAILED;
        }
        break;
    case MCUSER_EXIT_DONE:
        P3MC_GetUserEnd();
        TsMCAMes_SetMes(-1);
        return MCFLOW_DONE;
    case MCUSER_EXIT_BROKEN:
        P3MC_GetUserEnd();
        TsMCAMes_SetMes(-1);
        return MCFLOW_BROKEN;
    case MCUSER_EXIT_FAILED:
        P3MC_GetUserEnd();
        TsMCAMes_SetMes(-1);
        return MCFLOW_FAILED;
    }

    return isRun;
}

/* static */ int McUserSaveFlow(USER_DATA *puser) {
    switch (subStatus) {
    case 0:
        ret = P3MC_CheckChange();
        if (ret < 0) {
            break;
        }

        pGameData = P3MC_MakeDataWork(_P3DATA_SIZE(puser->mode), puser);
        TsSetSaveData(pGameData, puser->mode, puser);
        if (ret != 0) {
            subStatus = 0x2010;
            break;
        }
        subStatus = 0x100;
    case 0x100:
        P3MC_SaveUser(pGameData, P3MC_FLAG_OVERWRITE | P3MC_FLAG_WRITE_SYSTEM);
        subStatus = 0x2000;
        break;
    case 0x1000:
        TsMCAMes_SetMes(MCMES(MCMES_KIND_CONFIRM, 16));
        subStatus = 0x1010;
    case 0x1010:
        ret = P3MC_CheckChange();
        if (ret == 3 || ret == 5) {
            TsMCAMes_SetMes(-1);
            subStatus = 0xe000;
            break;
        }
        ret = TsMCAMes_GetSelect();
        if (ret != 0) {
            if (ret == 1) {
                subStatus = 0x1020;
            } else {
                subStatus = MCUSER_EXIT_FAILED;
            }
        }
        break;
    case 0x1020:
        if (P3MC_CheckChange() < 0) {
            break;
        }
        TsMCAMes_SetMes(-1);
        subStatus = 0x1030;
    case 0x1030:
        P3MC_SaveUser(pGameData, P3MC_FLAG_OVERWRITE | P3MC_FLAG_FORMAT);
        subStatus = 0x2000;
        break;
    case 0x2000:
        ret = P3MC_SaveCheck();
        if (ret == P3MC_RES_ACCESSING) {
            TsMCAMes_SetMes(MCMES(0, 19));
        }
        if (ret == P3MC_RES_FORMATTING) {
            TsMCAMes_SetMes(MCMES(0, 17));
        }
        if (ret < 0) {
            break;
        }
        subStatus = 0x2010;
    case 0x2010:
        if (ret != 0) {
            errorNo = 0;
            switch (ret) {
            case P3MC_RES_NEED_FORMAT:
                subStatus = 0x1000;
                break;
            case P3MC_RES_NO_CARD:
                if (puser->mode == 2) {
                    errorNo = 60;
                } else {
                    errorNo = 50;
                }
                break;
            case P3MC_RES_FILE_ERROR:
            case P3MC_RES_CONFIRM_OVERWRITE:
                errorNo = 1;
                break;
            case P3MC_RES_NO_SPACE:
                if (puser->mode == 2) {
                    errorNo = 15;
                } else {
                    errorNo = 7;
                }
                break;
            default:
                errorNo = ret;
                break;
            }
            if (errorNo != 0) {
                subStatus = 0x2200;
            }
            break;
        }
        subStatus = 0x2100;
    case 0x2100:
        *puser = pGameData->pHead->user;
        subStatus = 0x21f0;
        TsMCAMes_SetMes(-1);
    case 0x21f0:
        if (McErrorMess(200) >= 0) {
            subStatus = MCUSER_EXIT_DONE;
        }
        break;
    case 0x2200:
        TsMCAMes_SetMes(-1);
        subStatus = 0x2201;
    case 0x2201:
        ret = P3MC_CheckChange();
        if (ret == 3 || ret == 5) {
            subStatus = 0xe000;
            break;
        }
        if (McErrorMess(errorNo) < 0) {
            break;
        }
        subStatus = 0x2202;
    case 0x2202:
        if (P3MC_CheckChange() >= 0) {
            subStatus = MCUSER_EXIT_FAILED;
        }
        break;
    case 0xe000:
        ret = P3MC_CheckChange();
        switch (ret) {
        case 0:
        case 5:
            subStatus = MCUSER_EXIT_CARD_CHANGED;
            break;
        default:
            if (puser->mode == 2) {
                errorNo = 60;
            } else {
                errorNo = 50;
            }
            if (McErrorMess(errorNo) < 0) {
                break;
            }
            subStatus = 0xee10;
            break;
        }
        break;
    case 0xee10:
        if (P3MC_CheckChange() >= 0) {
            subStatus = MCUSER_EXIT_FAILED;
        }
        break;
    case MCUSER_EXIT_DONE:
        P3MC_DeleteDataWork(pGameData);
        TsMCAMes_SetMes(-1);
        return MCFLOW_DONE;
    case MCUSER_EXIT_BROKEN:
        P3MC_DeleteDataWork(pGameData);
        TsMCAMes_SetMes(-1);
        return MCFLOW_BROKEN;
    case MCUSER_EXIT_FAILED:
        P3MC_DeleteDataWork(pGameData);
        TsMCAMes_SetMes(-1);
        return MCFLOW_FAILED;
    case MCUSER_EXIT_CARD_CHANGED:
        P3MC_DeleteDataWork(pGameData);
        return MCFLOW_CARD_CHANGED;
    }

    return MCFLOW_RUNNING;
}

/* static */ int McUserLoadFlow(int fileNo, int mode, int bBroken) {
    switch (subStatus) {
    case 0:
        ret = P3MC_CheckChange();
        if (ret < 0) {
            break;
        }

        pGameData = P3MC_MakeDataWork(_P3DATA_SIZE(mode), NULL);
        if (ret != 0) {
            subStatus = 0x2010;
            break;
        }
        if (bBroken) {
            ret = 6;
            subStatus = 0x2010;
            break;
        }
        subStatus = 0x100;
    case 0x100:
        waitTime = 90;
        TsMCAMes_SetMes(MCMES(0, 7));
        P3MC_LoadUser(mode, fileNo, pGameData, 0);
        subStatus = 0x2000;
        break;
    case 0x2000:
        waitTime--;
        ret = P3MC_LoadCheck();
        if (ret >= 0) {
            subStatus = 0x2002;
        }
        break;
    case 0x2002:
        if (--waitTime > 0) {
            break;
        }
        subStatus = 0x2010;
    case 0x2010:
        if (ret != 0) {
            errorNo = 0;
            switch (ret) {
            case P3MC_RES_FILE_ERROR:
            case P3MC_RES_UNFORMATTED:
            case P3MC_RES_NO_SAVE_DATA:
                errorNo = 2;
                break;
            default:
                errorNo = ret;
                break;
            }
            if (errorNo != 0) {
                subStatus = 0x2200;
            }
            break;
        }
        subStatus = 0x2100;
    case 0x2100:
        TsRestoreSaveData(pGameData, mode);
        subStatus = 0x2f00;
        TsMCAMes_SetMes(-1);
    case 0x2f00:
        if (McErrorMess(100) >= 0) {
            subStatus = MCUSER_EXIT_DONE;
        }
        break;
    case 0x2200:
        TsMCAMes_SetMes(-1);
        subStatus = 0x2201;
    case 0x2201:
        ret = P3MC_CheckChange();
        if (ret == 3 || ret == 5) {
            subStatus = 0xe000;
            break;
        }
        if (McErrorMess(errorNo) < 0) {
            break;
        }
        subStatus = 0x2202;
    case 0x2202:
        if (P3MC_CheckChange() < 0) {
            break;
        }
        if (errorNo == 6 && bBroken) {
            subStatus = MCUSER_EXIT_BROKEN;
        } else {
            subStatus = MCUSER_EXIT_FAILED;
        }
        break;
    case 0xe000:
        ret = P3MC_CheckChange();
        if (ret == 0 || ret == 5) {
            subStatus = MCUSER_EXIT_CARD_CHANGED;
        } else if (McErrorMess(errorNo) >= 0) {
            subStatus = 0xee10;
        }
        break;
    case 0xee10:
        if (P3MC_CheckChange() < 0) {
            break;
        }
        TsMCAMes_SetMes(-1);
        if (errorNo == 6 && bBroken) {
            subStatus = MCUSER_EXIT_BROKEN;
        } else {
            subStatus = MCUSER_EXIT_FAILED;
        }
        break;
    case MCUSER_EXIT_DONE:
        P3MC_DeleteDataWork(pGameData);
        TsMCAMes_SetMes(-1);
        return MCFLOW_DONE;
    case MCUSER_EXIT_BROKEN:
        P3MC_DeleteDataWork(pGameData);
        TsMCAMes_SetMes(-1);
        return MCFLOW_BROKEN;
    case MCUSER_EXIT_FAILED:
        P3MC_DeleteDataWork(pGameData);
        TsMCAMes_SetMes(-1);
        return MCFLOW_FAILED;
    case MCUSER_EXIT_CARD_CHANGED:
        P3MC_DeleteDataWork(pGameData);
        return MCFLOW_CARD_CHANGED;
    }

    return MCFLOW_RUNNING;
}

static void TsMCAMes_Init(void) {
    memset(&MCMesWork, 0, sizeof(MCMesWork));
    MCMesWork.mesflg = -1;
}

static int TsMCAMes_GetSelect(void) {
    MCMES_WORK *pmesw = &MCMesWork;

    if (pmesw->mesflg < 0) {
        return 1;
    } else if (pmesw->mesflg & MCMES_KIND_MASK) {
        return pmesw->selflg;
    }

    return 1;
}

static int TsMCAMes_IsON(void) {
    MCMES_WORK *pmesw = &MCMesWork;
    return (u_int)~pmesw->mesflg >> 0x1f;
}

void TsMCAMes_SetPos(int x, int y) {
    MCMES_WORK *pmesw = &MCMesWork;

    pmesw->px = x;
    pmesw->py = y;
}

void TsMCAMes_SetMes(int no) {
    MCMES_WORK *pmesw;
    int         i;

    pmesw = &MCMesWork;

    if (no < 0) {
        pmesw->mesflg = -1;
        pmesw->selflg = 0;
        pmesw->seltim = 0;
        pmesw->line = 0;
        pmesw->btflg = 0;
        pmesw->backSw = 0;
        pmesw->faceNo = 0;
        return;
    }

    if (pmesw->mesflg != no) {
        if (TSSND_CHANISSTOP(2)) {
            for (i = 0; i < PR_ARRAYSIZEU(McVoiceTbl); i++) {
                if (McVoiceTbl[i].mesNo == no) {
                    TSSNDPLAY(McVoiceTbl[i].dataNo);
                    break;
                }
            }
        }
    }

    pmesw->mesflg = no;
    pmesw->faceNo = 0;

    for (i = 0; i < PR_ARRAYSIZEU(McFaceTbl); i++) {
        if (McFaceTbl[i].mesNo == no) {
            pmesw->faceNo = McFaceTbl[i].dataNo;
            break;
        }
    }

    if (no & MCMES_NOPLATE) {
        pmesw->backSw = 0;
        pmesw->btflg = 0;
    } else {
        pmesw->backSw = 1;
        pmesw->btflg = 1;
    }

    if (no & MCMES_COLOR) {
        pmesw->color = 1;
    } else {
        pmesw->color = 0;
    }

    pmesw->selflg = 0;
    pmesw->seltim = 0;
    pmesw->line = _PkMCMsgGetLine(pmesw->mesflg & 0xffff);
}

static void TsMCAMes_Flow(u_int tpad) {
    MCMES_WORK *pmesw;
    int         cLine;
    float       fRate;
    int         isOK, isCAN;

    pmesw = &MCMesWork;
    if (pmesw->backSw && pmesw->mesflg >= 0) {
        cLine = pmesw->line;
        if (cLine == 1) {
            cLine = 2;
        }

        cLine *= 4096;
        if (pmesw->D0line == 0) {
            pmesw->Dline = pmesw->D0line = cLine;
        } else {
            pmesw->Dline = pmesw->D0line = TSNumMov(pmesw->D0line, cLine, 4);
        }

        fRate = (MNSceneGetMusicFitTimer() % 72) / 72.0f;
        fRate *= 6.2831855f;
        fRate = cosf(fRate);
        fRate = ((fRate * fRate) * 1228.8f);
        pmesw->Dline += (int)fRate;
    } else {
        pmesw->D0line = 0;
        pmesw->Dline = 0;
    }

    if (pmesw->btflg != 0) {
        if (pmesw->btton < 0x40) {
            pmesw->btton += 7;
            if (pmesw->btton > 0x40) {
                pmesw->btton = 0x40;
            }
        }
    } else {
        if (pmesw->btton > 0) {
            pmesw->btton -= 7;
            if (pmesw->btton < 0) {
                pmesw->btton = 0;
            }
        }
    }

    if (pmesw->mesflg >= 0 && (pmesw->mesflg & MCMES_KIND_MASK)) {
        if (pmesw->mesflg & ((MCMES_BIT_TIMED | MCMES_BIT_CANCEL_OK) << 24)) {
            if (!TSSND_CHANISSTOP(1)) {
                pmesw->seltim++;
                return;
            }
        }

        if (pmesw->mesflg & (MCMES_BIT_CANCEL << 24)) {
            isCAN = TRUE;
        } else {
            isCAN = FALSE;
        }
    
        if (pmesw->mesflg & (MCMES_BIT_OK << 24)) {
            isOK = TRUE;
        } else {
            isOK = FALSE;
        }
        
        if (pmesw->mesflg & ((MCMES_BIT_CONFIRM | MCMES_BIT_CANCEL_OK) << 24)) {
            isOK = TRUE;
            isCAN = TRUE;
        }
    
        if (isOK && (tpad & SCE_PADRright)) {
            pmesw->selflg = 1;
            TSSNDPLAY(6);
        }
    
        if (isCAN && (tpad & SCE_PADRdown)) {
            pmesw->selflg = 2;
            if (pmesw->mesflg & (MCMES_BIT_CANCEL_OK << 24)) {
                TSSNDPLAY(6);
            } else {
                TSSNDPLAY(9);
            }
        }
    
        if (pmesw->mesflg & ((MCMES_BIT_TIMED | MCMES_BIT_CANCEL_OK) << 24)) {
            pmesw->seltim++;
            if (pmesw->seltim >= 0x79) {
                pmesw->selflg = 1;
            }
        }
    }
}

/* static */ void TsMCAMes_Draw(SPR_PKT pk, SPR_PRM *spr) {
    MCMES_WORK *pmesw = &MCMesWork;
    int         px, py, x, y;
    float       fRate, fLine;
    float       ofsy;
    u_int       col;

    if (pmesw->btton) {
        spr->rgba0 = pmesw->btton << 24;
        spr->zy = 1.0f;
        spr->zx = 1.0f;
        SetSprScreenXYWH(spr);
        PkCRect_Add(pk, spr, 0);
    }

    if (pmesw->mesflg < 0) {
        return;
    }

    py = pmesw->py;
    px = pmesw->px;
    if (pmesw->Dline <= 0x1000) {
        fLine = 0.0f;
        fRate = 0.0f;
    } else {
        fRate = (pmesw->Dline - 0x1000) / 4096.0f;
        fLine = fRate * 12.0f - 2.0f;
    }

    if (pmesw->backSw) {
        x = px - 0x124;
        ofsy = spr->ofsy;
        y = py + 5;
        spr->zy = 0.5f;
        spr->zx = 1.0f;
        spr->rgba0 = MN_COLOR_NEUTRAL;
        spr->ofsy = ofsy - (fLine * 0.5f + 36.0f);
        TsPatPut(pk, spr, &PAT_ALERT_WIN_ABOVE, x, y);

        spr->ofsy += 35.5f;
        if (fLine > 0.0f) {
            spr->zy = fRate * 0.5f;
            TsPatPut(pk, spr, &PAT_ALERT_WIN_CENTER, x, y);
            spr->ofsy += fLine - 1.0f;
        }

        spr->zy = 0.5f;
        TsPatPut(pk, spr, &PAT_ALERT_WIN_BELOW, x, py + 1);
        if (pmesw->faceNo > 0) {
            TsPatPut(pk, spr, &PAT_ALERT_WIN_FFACE[pmesw->faceNo - 1], px + 0xcc, y);
        }
        spr->ofsy = ofsy;
    }

    if (pmesw->mesflg >= 0) {
        y = py - ((pmesw->line * 12) >> 1);
        col = 0x80220061;
        if (pmesw->color != 0) {
            if (pmesw->color == 1) {
                col = 0x807f7f7f;
            }
        }
        _PkMCMsgPut(pk, spr, pmesw->mesflg & 0xffff, px, y, col);
    }
}

void TsCMPMes_SetPos(int x, int y) {
    CMPMES_WORK *pmesw = &CmpMesWork;
    pmesw->px = x;
    pmesw->py = y;
}

void TsCMPMes_SetMes(int no) {
    CMPMES_WORK *pmesw = &CmpMesWork;

    if (no < 0) {
        pmesw->mesflg = -1;
        pmesw->backSw = 0;

        pmesw->wh = 0;
        pmesw->ww = 0;
    } else {
        pmesw->mesflg = no;
        pmesw->backSw = 1;

        pmesw->ww = 600;
        pmesw->wh = 26;
    }
}

static void TsCMPMes_Draw(SPR_PKT pk, SPR_PRM *spr) {
    CMPMES_WORK *pmesw = &CmpMesWork;

    if (pmesw->backSw != 0) {
        MNScene_DispSw(&MNS_JimakuBak, 1);
        MNScene_Draw(&MNS_JimakuBak);
    } else {
        MNScene_DispSw(&MNS_JimakuBak, 0);
    }

    if (pmesw->mesflg < 0x1000u) {
        _PkSubMsgPut(pk, spr, pmesw->mesflg & 0xffff, pmesw->px, pmesw->py, 0x807f7f7f);
    }
}

void TsANIME_Init(ANIME_WK *wk) {
    memset(wk, 0, sizeof(*wk));
}

static int TsANIME_Poll(ANIME_WK *wk) {
    if (wk->atim != 0) {
        wk->atim--;
    }

    if (wk->aflg == 0) {
        return wk->atim;
    }

    if (!(wk->aflg & 0x1)) {
        wk->atrn = TSNumMov(wk->atrn, 0, 3);
        wk->atrn2 = TSNumMov(wk->atrn2, 0, 4);
        wk->atrn3 = TSNumMov(wk->atrn3, 0, 6);
    } else {
        wk->atrn = TSNumMov(wk->atrn, 0, 4);
        wk->atrn2 = TSNumMov(wk->atrn2, 0, 3);
        wk->atrn3 = TSNumMov(wk->atrn3, 0, 6);
    }

    if (wk->atrn == 0 && wk->atrn2 == 0 && wk->atrn3 == 0) {
        wk->aflg = 0;
    }

    return wk->atim;
}

static void TsANIME_Start(ANIME_WK *wk, int state, int tim) {
    wk->atrn2 = 0x200;
    wk->atrn  = 0x200;
    wk->atrn3 = 0x200;

    wk->atim = tim;
    wk->aflg = state;
}

static int TsANIME_GetRate(ANIME_WK *wk, float *rt0, float *rt1, float *rt2) {
    float p0, p1, p2;

    if (wk->aflg) {
        p0 = wk->atrn  * 0.001953125f;
        p1 = wk->atrn2 * 0.001953125f;
        p2 = wk->atrn3 * 0.001953125f;

        if (!(wk->aflg & 1)) {
            p0 = 1.0f - p0;
            p1 = 1.0f - p1;
        } else {
            p2 = 1.0f - p2;
        }

        if (rt0 != NULL) {
            *rt0 = p0;
        }
        if (rt1 != NULL) {
            *rt1 = p1;
        }
        if (rt2 != NULL) {
            *rt2 = p2;
        }

        return 1;
    } else {
        if (rt0 != NULL) {
            *rt0 = 1.0f;
        }
        if (rt1 != NULL) {
            *rt1 = 1.0f;
        }
        if (rt2 != NULL) {
            *rt2 = 1.0f;
        }

        return 0;
    }
}

/* static */ void _TsSortSetRanking(P3MC_RANKSCORE **ptRank, int n, P3MC_RANKSCORE *pRank, int bNameCmp) {
    int l, k, m;
    int isSame;

    for (l = 0; l < n; l++, pRank++) {
        for (k = 0; k < 20; k++) {
            if (ptRank[k] == NULL || ptRank[k]->score < pRank->score) {
                for (m = 19; k < m; m--) {
                    ptRank[m] = ptRank[m - 1];
                }
                ptRank[k] = pRank;
                break;
            }

            if (ptRank[k]->score == pRank->score && ptRank[k]->scDate[0] == pRank->scDate[0] && ptRank[k]->scDate[1] == pRank->scDate[1]) {
                isSame = TRUE;
                if (bNameCmp) {
                    int n;

                    for (n = 0; n < 8; n++) {
                        if (ptRank[k]->name[n] != pRank->name[n]) {
                            isSame = FALSE;
                            break;
                        }
                        if (ptRank[k]->name[n] == '\0') {
                            break;
                        }
                    }
                }

                if (isSame) {
                    break;
                }
            }
        }
    }
}

/* static */ RANKLIST* TsGetRankingList(int flag, int vsLev, int stageNo, int *nrank) {
    int             i;
    int             maxn;
    int             rnkMax;
    P3MC_RANKSCORE *ptRank[20];

    for (i = 0; i < 20; i++) {
        ptRank[i] = NULL;
    }

    {
        int             n;
        P3MC_RANKSCORE *pRank;

        if (flag == 0) {
            n     = pCStageRank[stageNo].nSplay;
            pRank = pCStageRank[stageNo].splay;
        } else {
            n     = pCStageRank[stageNo].nVplay[vsLev];
            pRank = pCStageRank[stageNo].vplay[vsLev];
        }
        _TsSortSetRanking(ptRank, n, pRank, 1);
    }

    maxn = P3MC_SortUser(UserLst, 1, 0);
    for (i = 0; i < maxn; i++) {
        int             n;
        P3MC_RANKSCORE *pRank;
        USER_DATA      *pUser = UserLst->pUserTbl[i];

        if (pUser->flg == 1) {
            if (flag == 0) {
                n     = pUser->stageRank[stageNo].nSplay;
                pRank = pUser->stageRank[stageNo].splay;
            } else {
                n     = pUser->stageRank[stageNo].nVplay[vsLev];
                pRank = pUser->stageRank[stageNo].vplay[vsLev];
            }
            _TsSortSetRanking(ptRank, n, pRank, 1);
        }
    }

    maxn = P3MC_SortUser(UserLst, 2, 0);
    for (i = 0; i < maxn; i++) {
        int             n;
        P3MC_RANKSCORE *pRank;
        USER_DATA      *pUser = UserLst->pUserTbl[i];

        if (pUser->flg == 1) {
            if (flag == 0) {
                if (pUser->isVs != 0) {
                    continue;
                }
                n     = pUser->stageRank[stageNo].nSplay;
                pRank = pUser->stageRank[stageNo].splay;
            } else {
                if (pUser->isVs != 2) {
                    continue;
                }
                n     = pUser->stageRank[stageNo].nVplay[vsLev];
                pRank = pUser->stageRank[stageNo].vplay[vsLev];
            }
            _TsSortSetRanking(ptRank, n, pRank, 0);
        }
    }

    rnkMax = 0;
    for (i = 0; i < 20 && ptRank[i] != NULL; i++) {
        RankLst[i].score = ptRank[i]->score;
        *(RANK_NAME*)RankLst[i].name = *(RANK_NAME*)ptRank[i]->name;
        RankLst[i].name[8] = '\0';
        if (RankLst[i].name[0] == '\0') {
            strcpy(RankLst[i].name, UserName_RankingNoSave);
        }
        rnkMax++;
    }

    *nrank = rnkMax;
    return RankLst;
}

void TsPopCusAOff(POPCTIM *pfw) {
    POPCOFF *poff;
    int      i;

    poff = pfw->offinf;

    for (i = 0; i < PR_ARRAYSIZE(pfw->offinf); i++, poff++) {
        if (i == pfw->onTNo) {
            poff->cltm = 0xf;
        }

        if (poff->bCur != 0) {
            poff->bCur = 0;
            poff->time = 0xf;
        } else {
            poff->time = 0;
            poff->bCur = 0;
            poff->cltm = 0;
        }
    }
}

void TsPopCusDim(POPCTIM *pfw, int n, int flg) {
    if (n >= PR_ARRAYSIZEU(pfw->bDim)) {
        return;
    }

    pfw->bDim[n] = flg;
}

void TsPopCusInit(POPCTIM *pfw,  int curIdx) {
    memset(pfw, 0, sizeof(*pfw));
    pfw->onTNo = curIdx;
}

void TsPopCusFlow(POPCTIM *pfw) {
    int i;

    if (pfw->okTim != 0) {
        pfw->okTim--;
    }
    if (pfw->onTim != 0) {
        pfw->onTim--;
    }
    if (pfw->srTim != 0) {
        pfw->srTim--;
    }

    for (i = 0; i < PR_ARRAYSIZE(pfw->offinf); i++) {
        if (pfw->offinf[i].time) {
            pfw->offinf[i].time--;
        }
        if (pfw->offinf[i].cltm) {
            pfw->offinf[i].cltm--;
        }
    }
}

/* static */ void TsPopCusPut(SPR_PKT pk, SPR_PRM *spr, int flg, POPCTIM *pfw, int bPut, int i, PATPOS *ppos, int px, int py) {
    float rt3;
    float rt = 0.0f;
    u_int mode;

    mode = bPut;
    if (pfw->bDim[i]) {
        mode = 4;
    }

    switch (mode) {
    case 0:
        if (!(flg & 1)) {
            return;
        }
        if (pfw->offinf[i].time) {
            rt   = pfw->offinf[i].time * 0.06666667f;
            mode = 1;
        }
        break;
    case 1:
        if (!(flg & 1)) {
            return;
        }
        rt = 1.0f;
        if (pfw->srTNo == i && pfw->srTim) {
            rt = 1.0f - pfw->srTim * 0.125f;
        }
        break;
    case 2:
        if (!(flg & 2)) {
            return;
        }
        pfw->srTNo = i;
        pfw->srTim = 8;
        break;
    case 3:
        if (!(flg & 1)) {
            return;
        }
        pfw->srTNo = i;
        pfw->srTim = 8;
        break;
    case 4:
        if (flg) {
            return;
        }
        spr->rgba0 = 0x40808080;
        break;
    case 5:
        if (!(flg & 4)) {
            return;
        }
        spr->rgba0 = 0x80707070;
        break;
    case 6:
        if (!(flg & 4)) {
            return;
        }
        break;
    }

    if (mode < 2 || mode == 5) {
        if (pfw->offinf[i].cltm) {
            float ct = pfw->offinf[i].cltm * 0.06666667f;
            spr->rgba0 = GetDToneColor(pfw->nabgr, pfw->habgr, ct * 240.0f * ct);
        }
    }

    switch (mode) {
    case 2:
    {
        float drt = pfw->okTim / 25.0f;
        float zrt = sinf(drt * 9.424778f);
        rt3 = sinf(drt * 6.2831855f);
        zrt *= drt * 0.9f * drt + 0.1f;
        spr->rgba0 = GetDToneColor(0xffffff, MN_COLOR_WHITE, rt3 * 256.0f * rt3);

        TsPatTexFnc(2);
        TsPatPutMZoom(pk, spr, ppos, px, py, 1.0 - zrt * 0.2, zrt * 0.6 + 1.0, 8, 4, zrt * -0.2, zrt * 0.6);
        TsPatTexFnc(0);

        pfw->srTim = 8;
        pfw->srTNo = i;
        pfw->offinf[i].bCur = 1;
    }
        break;
    case 1:
        TsPatPutSwing(pk, spr, ppos, px, py, 2, 8, pfw->fswing * rt);
        if (pfw->offinf[i].time == 0) {
            pfw->offinf[i].bCur = 1;
        }
        break;
    case 3:
    {
        float zrt = sinf(pfw->onTim * 2.1991148f * 0.1f);
        pfw->srTNo = i;
        pfw->srTim = 8;
        TsPatPutMZoom(pk, spr, ppos, px, py, zrt * 0.05f + 1.0f, zrt * 0.14f + 1.0f, 8, 4, zrt * -0.05f, zrt * -0.3f);
        pfw->offinf[i].bCur = 1;
    }
        break;
    case 5:
    case 6:
    {
        float zrat = sinf((MNSceneGetMusicFitTimer() % 45) * 3.1415927f / 45.0f);
        pfw->srTNo = i;
        pfw->srTim = 8;
        TsPatPutMZoom(pk, spr, ppos, px, py, zrat * 0.1f + 0.95f, zrat * 0.1f + 0.95f, 4, 4, 0.0f, 0.0f);
        pfw->offinf[i].bCur = 1;
    }
        break;
    default:
        TsPatPut(pk, spr, ppos, px, py);
        pfw->offinf[i].bCur = 0;
        break;
    }
}

int TsPUPCheckMove(int nbtn, int bank, POPCTIM *pfw) {
    return (pfw->bDim[POPBtn2Sel[(bank != 0) ? (nbtn + 3) : (nbtn + 0)]] == FALSE);
}

/* static */ int TsPopMenu_Flow(int flg, u_int tpad) {
    POPUP_MENU *pfw = &PopupMenu;
    int state;
    int aflg;
    int sel;
    int osel;
    int bkSel;
    int i;

    if (flg == 1) {
        TsANIME_Init(&pfw->awork);
        pfw->isRnkWAnime = 0;
        TsPopCusInit(&pfw->cani, 0);

        pfw->state = 0;
        pfw->selno = 0;
        pfw->btnNo = 0;
        pfw->bSelRank = 0;
        pfw->selLev = 0;
        pfw->rVsLev = 0;
        pfw->urTim = 0;

        if (pP3GameState->pLog->nRound == 0 && !(((int)pP3GameState->pLog->clrFlg[0] >> (pP3GameState->nStage - 1)) & 1)) {
            for (i = 0; i < 5; i++) {
                if (i != 0) {
                    pfw->cani.bDim[i] = 1;
                }
            }
        }

        if (pP3GameState->pLog->clrVSCOM1[pP3GameState->nStage - 1] == 0) {
            pfw->levMax = 1;
        } else {
            pfw->levMax = 4;
        }

        if (tpad != 0) {
            tpad--;
        }

        pfw->nPPosSet = PopRnkPos_No[tpad][pP3GameState->nStage];
        pfw->nPBubPat = PopBubblePat_No[tpad][pP3GameState->nStage];
        PopMenu_Sw = 1;
        pfw->isRankOn = 0;
        pfw->isSelLev = 0;
        return 0;
    }

    if (flg == 2) {
        PopMenu_Sw = 0;
        pfw->isRankOn = 0;
        pfw->isSelLev = 0;
        return 0;
    }

    aflg = TsANIME_Poll(&pfw->awork);
    TsPopCusFlow(&pfw->cani);

    state = pfw->state;
    switch (state) {
    case 0:
        state = 0x800;
        TsANIME_Start(&pfw->awork, 2, 15);
        aflg = TsANIME_Poll(&pfw->awork);
        pfw->isRnkWAnime = 0;
        /* fallthrough */
    case 0x800:
        if (aflg) {
            break;
        }
        /* fallthrough */
    case 0x1000:
        pfw->isSelLev = 0;
        state = 0x1010;
        pfw->selLev = 0;
        pfw->exitflg = 0;
        break;
    case 0x1010:
        if (pP3GameState->pLog->nRound == 0 && !(((int)pP3GameState->pLog->clrFlg[0] >> (pP3GameState->nStage - 1)) & 1)) {
            MENUSubt_PadFontArrowSet(0);
        } else {
            MENUSubt_PadFontArrowSet(POPSel2BtnDir[pfw->selno]);
        }
        TsCMPMes_SetMes(Pop_CmpMesNo[pfw->selno]);

        if (TsCheckTimeMapChange()) {
            break;
        }

        i = pfw->btnNo;
        bkSel = POPBtn2Sel[pfw->bSelRank ? i + 3 : i];
        sel = i;

        if (tpad & SCE_PADLleft) {
            sel--;
        }
        if (tpad & SCE_PADLright) {
            sel++;
        }
        if (i != sel) {
            sel = TSLOOP(sel, 3);
            if (TsPUPCheckMove(sel, pfw->bSelRank, &pfw->cani)) {
                pfw->btnNo = sel;
                pfw->bSelRank = 0;
                TSSNDPLAY(VSND_MVCUS_LR);
            }
        }

        if (pfw->btnNo == 1) {
            pfw->bSelRank = 0;
        } else {
            sel = pfw->bSelRank;
            if (tpad & SCE_PADLup) {
                sel--;
            }
            if (tpad & SCE_PADLdown) {
                sel++;
            }
            if (pfw->bSelRank != sel) {
                sel = TSLOOP(sel, 2);
                if (TsPUPCheckMove(pfw->btnNo, sel, &pfw->cani)) {
                    pfw->bSelRank = sel;
                    TSSNDPLAY(VSND_MVCUS_UD);
                }
            }
        }

        pfw->selno = POPBtn2Sel[pfw->bSelRank ? pfw->btnNo + 3 : pfw->btnNo];
        if (bkSel != pfw->selno) {
            TsPopCusAOff(&pfw->cani);
            pfw->cani.onTim = 10;
            pfw->cani.onTNo = pfw->selno;
        }
        TsCMPMes_SetMes(Pop_CmpMesNo[pfw->selno]);

        if (tpad & SCE_PADRdown) {
            state = 0xf020;
            pfw->exitflg = 1;
            TSSNDPLAY(VSND_CANCEL);
        }
        if (tpad & SCE_PADRright) {
            pfw->exitflg = 0;
            switch (pfw->selno) {
            case 0:
            case 1:
                TSSNDPLAY(VSND_GO_GAME);
                break;
            case 2:
                if (pfw->levMax < 2) {
                    TSSNDPLAY(VSND_GO_GAME);
                } else {
                    TSSNDPLAY(VSND_SELPOPUP);
                }
                break;
            case 3:
            case 4:
                TSSNDPLAY(VSND_SELPOPUP);
                break;
            }
            state = 0x2000;
        }
        break;
    case 0x2000:
        switch (pfw->selno) {
        case 0:
        case 1:
            MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[2]);
            state = 0xf000;
            break;
        case 2:
            if (pfw->levMax < 2) {
                state = 0xf000;
                MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[2]);
                pfw->selLev = 0;
            } else {
                state = 0x3000;
            }
            break;
        case 3:
        case 4:
            state = 0x5000;
            break;
        }
        break;
    case 0x3000:
        pfw->cani.okTim = 25;
        state = 0x3005;
        pfw->cani.onTNo = pfw->selno;
        /* fallthrough */
    case 0x3005:
        if (pfw->cani.okTim) {
            break;
        }
        pfw->selLev = 0;
        pfw->isSelLev = 1;
        TsCMPMes_SetMes(24);
        state = 0x3010;
        /* fallthrough */
    case 0x3010:
        if (TsCheckTimeMapChange()) {
            break;
        }

        osel = sel = pfw->selLev;
        if (tpad & SCE_PADLleft) {
            sel--;
        }
        if (tpad & SCE_PADLright) {
            sel++;
        }
        sel = TSLOOP(sel, pfw->levMax);
        if (osel != sel) {
            pfw->selLev = sel;
            TSSNDPLAY(VSND_MVCUS_LR);
        }

        if (tpad & SCE_PADRdown) {
            state = 0x1000;
            TSSNDPLAY(VSND_CANCEL);
        }
        if (tpad & SCE_PADRright) {
            state = 0xf000;
            TSSNDPLAY(VSND_GO_GAME);
            MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[2]);
            pfw->isSelLev = 0;
        }
        break;
    case 0x5000:
        pfw->cani.okTim = 25;
        pfw->cani.onTNo = pfw->selno;
        if (pfw->selno == 3) {
            pfw->nTagMax = 1;
            pfw->nRankMax = 20;
        } else {
            pfw->nRankMax = 10;
            pfw->nTagMax = pfw->levMax;
        }
        pfw->rVsLev = 0;
        state = 0x5010;
        McInitFlow();
        break;
    case 0x5010:
        ret = McUserCheckFlow(MCCHECK_BROWSE, 3, NULL);
        if (ret < 0) {
            break;
        }
        if (ret == 1 || ret == 2) {
            pfw->isRankOn = 0;
            state = 0x1010;
        } else {
            state = 0x6000;
        }
        break;
    case 0x6000:
        TsANIME_Start(&pfw->awork, 4, 15);
        pfw->isRnkWAnime = 1;
        aflg = TsANIME_Poll(&pfw->awork);
        /* fallthrough */
    case 0x6002:
        pfw->selrnkpg = 0;
        pfw->isRankOn = 1;
        pfw->rsline = 0;
        pfw->rStageNo = pP3GameState->nStage - 1;
        if (pfw->selno == 3) {
            pfw->rankFlg = 0;
        } else {
            pfw->rankFlg = 1;
        }
        pfw->pRanking = TsGetRankingList(pfw->rankFlg, pfw->rVsLev, pfw->rStageNo, &pfw->nRanking);
        if (pfw->nRanking > pfw->nRankMax) {
            pfw->nRanking = pfw->nRankMax;
        }
        pfw->nPageMax = (pfw->nRanking + 9) / 10;
        if (pfw->nPageMax == 0) {
            pfw->nPageMax = 1;
        }
        state = 0x6005;
        /* fallthrough */
    case 0x6005:
        if (aflg) {
            break;
        }
        if (pfw->nPageMax < 2) {
            if (pfw->rankFlg) {
                TsCMPMes_SetMes(30);
            } else {
                TsCMPMes_SetMes(29);
            }
        } else {
            TsCMPMes_SetMes(28);
        }
        state = 0x6010;
        break;
    case 0x6010:
        if (pfw->rsline) {
            pfw->rsline = TSNumMov(pfw->rsline, 0, 5);
        } else {
            if (TsCheckTimeMapChange()) {
                break;
            }

            sel = pfw->selrnkpg;
            bkSel = sel;
            osel = sel;
            if (tpad & SCE_PADLup) {
                sel--;
            }
            if (tpad & SCE_PADLdown) {
                sel++;
            }
            sel = TSLIMIT(sel, 0, pfw->nPageMax);
            if (bkSel != sel) {
                if (sel < bkSel) {
                    pfw->rsline = 160;
                } else {
                    pfw->rsline = -160;
                }
                pfw->selrnkpg = sel;
                if (sel < osel) {
                    TSSNDPLAY(VSND_MVCUS_U);
                } else {
                    TSSNDPLAY(VSND_MVCUS_D);
                }
            }

            sel = pfw->rVsLev;
            bkSel = sel;
            if (tpad & SCE_PADLleft) {
                sel--;
            }
            if (tpad & SCE_PADLright) {
                sel++;
            }
            sel = TSLOOP(sel, pfw->nTagMax);
            if (bkSel != sel) {
                pfw->rVsLev = sel;
                TSSNDPLAY(VSND_MVCUS_LR);
                state = 0x6002;
                break;
            }
        }

        if (tpad & SCE_PADRdown) {
            pfw->exitflg = 0;
            state = 0x6020;
            TSSNDPLAY(VSND_CANCEL);
        }
        break;
    case 0x6020:
        state = 0x6030;
        TsANIME_Start(&pfw->awork, 5, 15);
        aflg = TsANIME_Poll(&pfw->awork);
        pfw->isRnkWAnime = 1;
        /* fallthrough */
    case 0x6030:
        if (aflg) {
            break;
        }
        pfw->isRankOn = 0;
        state = 0x1010;
        break;
    case 0xf000:
        pfw->cani.okTim = 25;
        state = 0xf010;
        pfw->cani.onTNo = pfw->selno;
        /* fallthrough */
    case 0xf010:
        if (pfw->cani.okTim) {
            break;
        }
        /* fallthrough */
    case 0xf020:
        state = 0xf080;
        TsANIME_Start(&pfw->awork, 3, 15);
        aflg = TsANIME_Poll(&pfw->awork);
        pfw->isRnkWAnime = 0;
        /* fallthrough */
    case 0xf080:
        if (aflg) {
            break;
        }
        /* fallthrough */
    case 0xf100:
        PopMenu_Sw = 0;
        pfw->isRankOn = 0;
        if (pfw->exitflg) {
            state = 0xff20;
            break;
        }
        switch (pfw->selno) {
        case 0:
            ret = 1;
            break;
        case 1:
            ret = 2;
            break;
        case 2:
            ret = (pfw->selLev << 8) | 3;
            break;
        }
        return ret;
    case 0xff20:
        return -1;
    }

    pfw->state = state;
    return 0;
}

/* static */ void TsPopMenu_Draw(SPR_PKT pk, SPR_PRM *spr) {
    POPUP_MENU *pfw = &PopupMenu;
    char        buf[32];
    float       rt0, rt1, rt2;
    int         ton;
    float       uneri;
    float       rt;
    PTPOS      *pos;
    PATPOS     *pat;
    PATPOS     *stno;
    STRPOS     *str;
    RANKLIST   *pRanking;
    int         nRanking;
    u_int       hicol, nmcol, col;
    int         px, py;
    int         ox, oy;
    int         x, y, sy;
    int         i, n;

    pfw->cani.fswing = -cosf((MNSceneGetMusicFitTimer() % 72) / 72.0f * 6.2831855f);
    uneri = (MNSceneGetMusicFitTimer() % 180) / 180.0f;

    spr->zx = 1.0f;
    spr->zy = 0.5f;
    PkALPHA_Add(pk, 0x44);

    if (pfw->isRnkWAnime == 0 && TsANIME_GetRate(&pfw->awork, &rt0, &rt1, &rt2)) {
        pos = &POPWZoom_CXY[pfw->nPBubPat];
        ton = rt0 * 256.0f;

        spr->zoom.centerX = pos->x;
        spr->zoom.centerY = pos->y;
        spr->zoom.zoomX   = rt0;
        spr->zoom.zoomY   = rt1;
        spr->zoom.isOn    = 1;
    } else {
        spr->zoom.isOn = 0;
        ton = 0x100;
    }

    pos = PopRnk_pPos[pfw->nPPosSet].menu;
    py  = pos->y;
    px  = pos->x;
    spr->rgba0 = GetDToneColor(0x404040, MN_COLOR_NEUTRAL, ton);

    if (pfw->urTim) {
        pfw->urTim--;
    }

    hicol = GetDToneColor(0x404040, MN_COLOR_WHITE, ton);
    nmcol = GetDToneColor(0x404040, MN_COLOR_NEUTRAL, ton);
    TsPopMenCus_Draw(pk, spr, pfw, px, py, hicol, nmcol, 0);
    TsPopMenCus_Draw(pk, spr, pfw, px, py, hicol, nmcol, 1);
    TsPopMenCus_Draw(pk, spr, pfw, px, py, hicol, nmcol, 6);
    spr->zoom.isOn = 0;

    if (pfw->isRankOn == 0) {
        return;
    }

    ox  = px;
    oy  = py;
    pos = PopRnk_pPos[pfw->nPPosSet].rank;
    py  = pos->y;
    px  = pos->x;

    if (pfw->isRnkWAnime) {
        if (TsANIME_GetRate(&pfw->awork, &rt0, &rt1, &rt2)) {
        if (pfw->rankFlg == 0) {
            pat = &PopMenuSel_Pat[3];
        } else {
            pat = &PopMenuSel_Pat[4];
        }

        x = pat->x + ox;
        spr->zoom.zoomY   = rt1 * 0.9f + 0.1f;
        spr->zoom.isOn    = 1;
        spr->zoom.centerX = (x + 32) + (x - px) / 1.8f;
        spr->zoom.zoomX   = rt0 * 0.65f + 0.35f;
        spr->zoom.centerY = pat->y + oy + 6;
        ton = (int)(rt0 * 156.0f) + 100;
        rt  = rt2 * 0.6f;
        } else {
            spr->zoom.isOn = 0;
            ton = 0x100;
            rt  = 0.0f;
        }
    } else {
        ton = 0x100;
        rt  = 0.0f;
    }

    spr->rgba0 = GetDToneColor(0x20ffffff, MN_COLOR_NEUTRAL, ton);

    if (pfw->rankFlg == 0) {
        pat  = SIRanking_Pat;
        stno = RankSISTNo_PAT;
        n    = 2;
    } else {
        pat  = VSRanking_PatTbl[pfw->rVsLev];
        stno = RankVSSTNo_PAT;
        n    = 5;
    }

    for (i = 0; i < n; i++) {
        if (rt != 0.0f) {
            float f = sinf(rt * 6.2831855f);

            TsPatPutMZoom(pk, spr, &pat[i], px, py, f * 0.2f + 1.0f, f * 0.2f + 1.0f, 8, 8, f * 0.3f, f * 0.3f);
            pfw->urTim = 20;
        } else if (i == 0) {
            TsPatPutUneri(pk, spr, &pat[i], px, py, 8, 10, uneri, (pfw->urTim != 0) ? 1.0f - pfw->urTim * 0.05f : 1.0f);
        } else {
            TsPatPut(pk, spr, &pat[i], px, py);
        }
    }

    if (rt < 0.15f) {
        ton = (0.15f - rt) * 256.0f * 6.6666665f;
    } else {
        ton = 0;
    }

    spr->rgba0 = GetDToneColor(0x808080, MN_COLOR_NEUTRAL, ton);
    TsPatPut(pk, spr, &stno[0], px, py);
    TsPatPut(pk, spr, &stno[pfw->rStageNo + 1], px, py);

    if (pfw->selrnkpg > 0) {
        TsPatPut(pk, spr, &Ranking_PatScroll[0], px, py);
    }
    if (pfw->selrnkpg < pfw->nPageMax - 1) {
        TsPatPut(pk, spr, &Ranking_PatScroll[1], px, py);
    }

    pRanking = pfw->pRanking;
    nRanking = pfw->nRanking;

    if (pfw->rankFlg == 0) {
        str = SRanking_Str;
    } else {
        str = VRanking_Str;
    }

    y  = py + str->y;
    sy = y - 5;
    SetSprScreenXYWH(spr);
    PkSCISSOR_Add(pk, spr->px, sy, spr->sw, 80);

    py -= pfw->selrnkpg * 80 + (pfw->rsline >> 1);

    for (i = 0; i < nRanking; i++, py += 8) {
        x = py + str->y;
        if (y - 13 < x && x < sy + 88) {
            col = GetDToneColor(str->abgr & 0xffffff, str->abgr, ton);

            sprintf(buf, "%d", i + 1);
            MENUFontPutR(pk, spr, px + str[0].x, x, col, 0x100, buf, 1.0f);

            sprintf(buf, "%d", pRanking[i].score);
            MENUFontPutR(pk, spr, px + str[1].x, py + str[1].y, col, 0x102, buf, 1.0f);

            sprintf(buf, "%s", pRanking[i].name);
            MENUFontPutR(pk, spr, px + str[2].x, py + str[2].y, col, 0x100, buf, 1.1f);
        }
    }

    PkDefSCISSOR_Add(pk);
    spr->zoom.isOn = 0;
}

void TsPopMenCus_Draw(SPR_PKT pk, SPR_PRM *spr, POPUP_MENU *pfw, int px, int py, u_int hicol, u_int nmcol, int dflg) {
    int   i;
    float bofsy;
    int   bPut;
    int   bHiLgt;

    bofsy = spr->ofsy;

    for (i = 0; i < 5; i++) {
        bHiLgt = 0;
        pfw->cani.habgr = hicol;
        pfw->cani.nabgr = nmcol;

        spr->ofsy = bofsy + sinf((MNSceneGetMusicFitTimer() % 480) * 6.2831855f * 0.0020833334f + POPSel2Btn[i] * 2.5132742f) * 3.3f;

        if (i == pfw->selno && pfw->cani.okTim) {
            bPut = 2;
            bHiLgt = 2;
        } else {
            bPut = 0;
            if (POPSel2Btn[i] == pfw->btnNo) {
                bPut = 1;
            }
            if (i == pfw->selno) {
                bHiLgt = 1;
                if (pfw->cani.onTim) {
                    bPut = 3;
                }
            }
        }

        if (pfw->isSelLev) {
            bPut = (i == 2) ? 5 : 0;
        }

        switch (bHiLgt) {
        case 0:
            spr->rgba0 = pfw->cani.nabgr;
            break;
        case 1:
            spr->rgba0 = pfw->cani.habgr;
            break;
        }

        TsPopCusPut(pk, spr, dflg, &pfw->cani, bPut, i, &PopMenuSel_Pat[i], px, py);

        if (i == 2) {
            int     i;
            int     bPut0;
            PATPOS *pt;

            pfw->cani.habgr = nmcol;
            pfw->cani.nabgr = nmcol;

            for (i = 0; i < pfw->levMax; i++) {
                spr->rgba0 = nmcol;
                bPut0 = bPut;
                pt = &VSComMenuSel_Pat[i];

                if (pfw->isSelLev) {
                    bPut0 = 6;
                    if (i == pfw->selLev) {
                        spr->rgba0 = GetDToneColor(MN_COLOR_WHITE, 0x80606060, sinf((MNSceneGetMusicFitTimer() % 30) * 3.1415927f / 30.0f) * 256.0f);
                        pt = &VSComMenuSelH_Pat[i];
                    } else {
                        spr->rgba0 = 0x80707070;
                    }
                }

                TsPopCusPut(pk, spr, dflg, &pfw->cani, bPut0, 2, pt, px, py);
            }
        }
    }

    spr->ofsy = bofsy;
}

/* static */ int TsSaveMenu_Flow(int flg, u_int tpad) {
    SAVE_MENU *pfw = &SaveMenu;
    int        state;
    int        aret;
    int        sel;

    if (flg == 1) {
        if (tpad < 2) {
            pfw->selno = tpad;
        } else {
            pfw->selno = 0;
        }
        pfw->state = 0;
        TsANIME_Init(&pfw->awork);
        TsPopCusInit(&pfw->cani, 0);
        SaveMenu_Sw = TRUE;
        return 0;
    }

    if (flg == 2) {
        SaveMenu_Sw = FALSE;
        return 0;
    }

    state = pfw->state;
    aret = TsANIME_Poll(&pfw->awork);
    TsPopCusFlow(&pfw->cani);

    switch (state) {
    case 0:
        state = 0x100;
        TsANIME_Start(&pfw->awork, 2, 15);
        aret = TsANIME_Poll(&pfw->awork);
    case 0x100:
        if (aret) {
            break;
        }
    case 0x1000:
        pfw->exitflg = 0;
        state = 0x1010;
        break;
    case 0x1010:
        TsCMPMes_SetMes(SaveMenu_CmpMesNo[pfw->selno]);
        if (TsCheckTimeMapChange()) {
            break;
        }

        sel = pfw->selno;
        if (tpad & SCE_PADLleft) {
            sel--;
        }
        if (tpad & SCE_PADLright) {
            sel++;
        }

        if (pfw->selno != sel) {
            sel = TSLOOP(sel, 2);
            pfw->selno = sel;
            TsPopCusAOff(&pfw->cani);
            pfw->cani.onTNo = sel;
            pfw->cani.onTim = 10;
            TSSNDPLAY(2);
            TsCMPMes_SetMes(SaveMenu_CmpMesNo[pfw->selno]);
        }

        if (tpad & SCE_PADRdown) {
            pfw->exitflg = 1;
            TSSNDPLAY(9);
            state = 0xf020;
        } else if (tpad & SCE_PADRright) {
            pfw->exitflg = 0;
            MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[3]);
            state = 0xf000;
            TSSNDPLAY(6);
        }
        break;
    case 0xf000:
        pfw->cani.okTim = 25;
        state = 0xf010;
        pfw->cani.onTNo = pfw->selno;
    case 0xf010:
        if (pfw->cani.okTim) {
            break;
        }
    case 0xf020:
        state = 0xf080;
        TsANIME_Start(&pfw->awork, 3, 20);
        aret = TsANIME_Poll(&pfw->awork);
    case 0xf080:
        if (aret) {
            break;
        }
    case 0xf100:
        SaveMenu_Sw = FALSE;
        if (pfw->exitflg) {
            state = 0xff20;
            break;
        }

        switch (pfw->selno) {
        case 0:
            ret = 1;
            break;
        case 1:
            ret = 2;
            break;
        default:
            ret = 3;
            break;
        }
        return ret;
    case 0xff20:
        return -1;
    }

    pfw->state = state;
    return 0;
}

static void TsSaveMenu_Draw(SPR_PKT pk, SPR_PRM *spr) {
    SAVE_MENU *pfw = &SaveMenu;
    PATPOS    *ppat;
    int        i;
    float      fswing;
    int        arate;
    float      rt0, rt1, rt2;
    int        bHiLgt;
    int        bPut;

    fswing = (MNSceneGetMusicFitTimer() % 72) / 72.0f;
    fswing = -cosf(fswing * 6.2831855f);
    arate = MNSceneGetMusicFitTimer() % 180;
    pfw->cani.fswing = fswing;

    if (TsANIME_GetRate(&pfw->awork, &rt0, &rt1, &rt2)) {
        spr->zoom.centerX = SAVEWZoom_CXY[0].x;
        spr->zoom.centerY = SAVEWZoom_CXY[0].y;
        spr->zoom.zoomX = rt1;
        spr->zoom.zoomY = rt0;
        spr->zoom.isOn = 1;
        arate = rt0 * 256.0f;
    } else {
        spr->zoom.isOn = 0;
        arate = 256;
    }

    spr->zx = 1.0f;
    spr->zy = 0.5f;
    PkALPHA_Add(pk, 0x44);

    spr->rgba0 = GetDToneColor(0x404040, MN_COLOR_NEUTRAL, arate);
    ppat = SAVE_MENU_SELPAT;
    pfw->cani.habgr = GetDToneColor(0x404040, MN_COLOR_WHITE, arate);
    pfw->cani.nabgr = GetDToneColor(0x404040, MN_COLOR_NEUTRAL, arate);

    for (i = 0; i < 2; i++, ppat++) {
        bHiLgt = 0;
        bPut = 0;

        if (i == pfw->selno) {
            bHiLgt = 1;
            if (pfw->cani.okTim) {
                bPut = 2;
                bHiLgt = 2;
            } else {
                bPut = (pfw->cani.onTim) ? 3 : 1;
            }
        }

        switch (bHiLgt) {
        case 0:
            spr->rgba0 = pfw->cani.nabgr;
            break;
        case 1:
            spr->rgba0 = pfw->cani.habgr;
            break;
        }

        TsPopCusPut(pk, spr, 3, &pfw->cani, bPut, i, ppat, 0, 0);
    }

    spr->zoom.isOn = 0;
}

static void TSJukeCDObj_Init(JUKECDOBJ *pw, int pno) {
    memset(pw, 0, sizeof(*pw));
    pw->patPos = &JUKEBOX_Pos[pno];
    pw->patNo = pno;
}

/* static */ void _TsJkJacketPut(SPR_PKT pk, SPR_PRM *spr, JUKECDOBJ *pw, int px, int py, float zx, float rot, u_int abgr, u_int abgrs) {
    if (pw->vrate == 0.0f) {
        spr->rgba0 = abgrs;
        TsPatPutRZoom(pk, spr, JUKEJKT_PatS, px + 10, py + 5, zx, rot);
        spr->rgba0 = abgr;
        TsPatPutRZoom(pk, spr, &JUKEJKT_Pat[pw->patNo], px, py, zx, rot);
    } else {
        spr->rgba0 = abgrs;
        TsPatPutMZoom(pk, spr, JUKEJKT_PatS, px + 10, py + 5, zx + pw->vrate, zx + pw->vrate, 8, 8, pw->vrate, pw->vrate);
        spr->rgba0 = abgr;
        TsPatPutMZoom(pk, spr, &JUKEJKT_Pat[pw->patNo], px, py, zx + pw->vrate, zx + pw->vrate, 8, 8, pw->vrate, pw->vrate);
    }
}

/* static */ void _TsJkRecordPut(SPR_PKT pk, SPR_PRM *spr, JUKECDOBJ *pw, int px, int py, float zr, float rrot, u_int abgr, u_int abgrs) {
    float box, boy;

    if (pw->rox == 0.0f && pw->roy == 0.0f) {
        return;
    }

    box = spr->ofsx;
    boy = spr->ofsy;

    spr->rgba0 = abgrs;
    spr->ofsx = box + pw->rox;
    spr->ofsy = boy + pw->roy;
    TsPatPutRZoom(pk, spr, JUKEREC_PatS, px + 10, py + 5, zr, rrot);

    spr->rgba0 = abgr;
    TsPatPutRZoom(pk, spr, &JUKEREC_Pat[pw->patNo], px, py, zr, rrot);

    spr->ofsx = box;
    spr->ofsy = boy;
}

/* static */ void TSJukeCDObj_Draw(SPR_PKT pk, SPR_PRM *spr, JUKECDOBJ *pw, int px, int py, int anmtime) {
    PATPOS *ppat;
    float   box, boy;
    float   jrot, rrot;
    float   addx, addy;
    float   rt, zm;
    float   jzr, rzr;
    float   jz, rz;
    float   swing;
    float   fx, fy;
    float   rpx, rpy;
    u_int   abgr, abgrs;
    u_int   rabgr, rabgrs;
    int     ton;
    int     i;
    int     sx, sy;

    addy = 0.0f;
    px += pw->patPos->x;
    py += pw->patPos->y;
    ppat = &JUKEJKT_Pat[pw->patNo];

    jzr = 1.0f;
    rzr = 1.0f;
    zm = 1.0f;
    swing = 1.0f;

    box = spr->ofsx;
    boy = spr->ofsy;

    PkALPHA_Add(pk, 0x44);

    jrot = 0.0f;
    addx = 0.0f;
    rrot = 0.0f;

    switch (pw->state) {
    case TSJKCUS_MSK:
        return;
    case TSJKCUS_DEF:
        abgr = 0x80707070;
        abgrs = 0x30808080;
        pw->vrate = 0.0f;
        break;
    case TSJKCUS_CUR:
    case TSJKCUS_SELOK:
        zm = 1.25f;
        abgr = 0x80c0c0c0;
        abgrs = 0x30808080;
        pw->vrate = 0.0f;
        break;
    case TSJKCUS_ON:
        pw->time++;
        rt = pw->time * (1.0f / 15.0f);
        if (pw->time >= 15) {
            pw->time = 0;
            pw->state = TSJKCUS_CUR;
        }

        fx = rt * 3.1415927f;
        abgrs = 0x30808080;
        zm = (sinf(fx) * 0.2f + 1.0f) * (rt * 0.25f + 1.0f);
        pw->vrate = sinf(fx) * 0.1f;
        abgr = GetDToneColor(0x80707070, 0x80c0c0c0, rt * 256.0f);
        break;
    case TSJKCUS_OFF:
        pw->time++;
        rt = pw->time * (1.0f / 15.0f);
        if (pw->time >= 15) {
            pw->time = 0;
            pw->state = TSJKCUS_DEF;
        }

        fx = rt * 3.1415927f;
        abgrs = 0x30808080;
        zm = (1.0 - sinf(fx) * 0.25) * ((1.0f - rt) * 0.25f + 1.0f);
        pw->vrate = sinf(fx) * -0.08f;
        abgr = GetDToneColor(0x80c0c0c0, 0x80707070, rt * 256.0f);
        break;
    default:
        abgr = 0x80c0c0c0;
        abgrs = 0x30808080;
        pw->vrate = jrot;
        break;
    }

    if (pw->bMsk) {
        abgr = 0x30808080;
        abgrs = 0x18808080;
        pw->vrate = 0.0f;
    }

    rabgr = abgr;
    rabgrs = abgrs;

    switch (pw->anime) {
    case TSJKANM_IN:
        if (pw->atime) {
            pw->innm = 0x200;
            pw->atime--;
            return;
        }

        pw->innm = TSNumMov(pw->innm, 0, 5);
        rt = 1.0f - pw->innm / 512.0f;

        jrot = (1.0f - rt) * 6.2831855f;
        addx = (1.0f - rt) * (pw->dir ? 200.0f : -200.0f);
        if (pw->dir2) {
            addy = (1.0f - rt) * 20.0f;
        } else {
            addy = (1.0f - rt) * -20.0f;
        }

        ton = rt * 256.0f;
        abgr = GetDToneColor(0xffffff, abgr, ton);
        abgrs = GetDToneColor(0x808080, abgrs, ton);
        jzr = (1.0f - rt) * 1.5f + 1.0f;

        if (pw->innm == 0) {
            pw->anime = TSJKANM_OFF;
            pw->time = 0;
            pw->atime = 0;
        }
        break;
    case TSJKANM_OUT:
        if (pw->atime) {
            pw->atime--;
            pw->innm = 0x100;
            break;
        }

        pw->innm = TSNumMov(pw->innm, 0, 5);
        rt = 1.0f - pw->innm / 256.0f;

        jrot = rt * -9.424778f;
        addx = rt * (pw->dir ? -200.0f : 200.0f);
        addy = rt * (pw->dir2 ? 20.0f : -20.0f);

        ton = rt * 256.0f;
        abgr = GetDToneColor(abgr, 0xffffff, ton);
        abgrs = GetDToneColor(abgr, 0x808080, ton);
        jzr = rt * 1.2f + 0.5f;

        if (pw->innm == 0) {
            pw->anime = TSJKANM_OFF;
            pw->state = TSJKCUS_MSK;
            pw->time = 0;
            pw->atime = 0;
        }
        break;
    case TSJKANM_PLAY2:
    case TSJKANM_RETURN2:
        if (pw->atime) {
            pw->atime--;
            pw->innm = 0x80;
            pw->oy = pw->rot = pw->ox = 0.0f;
            if (pw->anime == TSJKANM_RETURN2) {
                return;
            }
            break;
        }

        pw->innm = TSNumMov(pw->innm, 0, 12);
        rt = 1.0f - pw->innm / 128.0f;
        TsPatGetSize(ppat, &sx, &sy, NULL, NULL);
        fx = sx + px - 318;
        fy = sy + py - 161;

        if (pw->anime == TSJKANM_RETURN2) {
            rt = 1.0f - rt;
        }

        ton = rt * 256.0f;
        addx = fx * 0.35f * rt;
        addy = fy * 0.5f * rt;
        abgr = GetDToneColor(abgr, 0x404040, ton);
        abgrs = GetDToneColor(abgrs, 0x808080, ton);
        jzr = 1.0f - rt * 0.3f;

        if (pw->innm == 0) {
            if (pw->anime == TSJKANM_RETURN2) {
                pw->state = TSJKCUS_DEF;
            } else {
                pw->state = TSJKCUS_MSK;
            }
            pw->anime = TSJKANM_OFF;
            pw->time = 0;
            pw->atime = 0;
        }
        break;
    case TSJKANM_PLAY1:
    case TSJKANM_RETURN1:
        if (pw->atime) {
            pw->atime--;
            pw->innm = 0x200;
            pw->rox = pw->rrot = pw->ox = pw->oy = pw->rot = 0.0f;

            if (pw->anime == TSJKANM_RETURN1) {
                abgr = 0x80c0c0c0;
                abgrs = 0x30808080;
                rabgr = 0x80c0c0c0;
                rabgrs = 0x30808080;
                pw->rox = zm * 0.0f;
                pw->roy = zm * -43.0f;
                swing = 0.0f;
            }
            break;
        }

        pw->rox = pw->roy = pw->ox = rpx = pw->oy = 0.0f;
        rpy = -43.0f;

        pw->innm = TSNumMov(pw->innm, 0, 7);
        rzr = 1.0f;
        rt = 1.0f - pw->innm / 512.0f;

        if (pw->anime == TSJKANM_RETURN1) {
            rt = 1.0f - rt;
        }

        swing = 1.0f - rt;
        rabgr = abgr;
        rabgrs = abgrs;

        fx = rt * 0.5f * 3.1415927f;
        pw->rox = (1.0f - cosf(fx)) * rpx * zm * jzr;
        pw->roy = sinf(fx) * rpy * zm * jzr;

        if (pw->innm == 0) {
            pw->time = 0;
            pw->atime = 0;
            if (pw->anime == TSJKANM_RETURN1) {
                pw->state = TSJKCUS_CUR;
                pw->anime = TSJKANM_OFF;
            } else {
                pw->state = TSJKCUS_CUR;
                pw->anime = TSJKANM_ROTATE;
            }
        }
        break;
    case TSJKANM_ROTATE:
        swing = 0.0f;

        if (pw->atime == 0) {
            pw->innm2 = 0x400;
            pw->atime = 1;
            pw->rrot = swing;
            pw->innm = 0x400;
        } else {
            pw->atime++;
            pw->rrot += ((pw->atime > 110) ? 110 : pw->atime) * 0.0023f;
            if (pw->rrot > 6.2831855f) {
                pw->rrot -= 6.2831855f;
            }
        }

        fx = zm * 0.0f;
        fy = zm * -43.0f;
        rrot = pw->rrot;
        rzr = 1.0f;
        pw->rox = fx;
        pw->roy = fy;

        if (pw->atime > 70) {
            pw->innm2 = TSNumMov(pw->innm2, 0, 11);
            rt = 1.0f - pw->innm2 / 1024.0f;
            ton = rt * 256.0f;
            abgr = GetDToneColor(abgr, 0xffffff, ton);
            abgrs = GetDToneColor(abgrs, 0x808080, ton);
            jzr = 1.0f - rt * 0.5f;
        }

        if (pw->atime > 100) {
            pw->innm = TSNumMov(pw->innm, 0, 14);
            rt = 1.0f - pw->innm / 1024.0f;
            ton = rt * 256.0f;
            rabgr = GetDToneColor(rabgr, 0xffffff, ton);
            rabgrs = GetDToneColor(rabgrs, 0x808080, ton);
            rzr = rt * 0.8f + rzr;

            TsPatGetSize(ppat, &sx, &sy, NULL, NULL);
            rpx = sx + px - 318;
            rpy = sy + py - 181;
            pw->rox = fx - rpx * rt;
            pw->roy = fy - rpy * rt;
        }

        if (pw->innm == 0 && pw->innm2 == 0) {
            pw->state = TSJKCUS_MSK;
            pw->anime = TSJKANM_OFF;
            pw->time = 0;
            pw->atime = 0;
        }
        break;
    case TSJKANM_ROTSTOP:
        swing = 0.0f;

        if (pw->atime == 0) {
            pw->atime = 80;
            pw->rrot = swing;
            for (i = 0; i < 60; i++) {
                pw->rrot -= i;
            }
            pw->innm = 0x400;

            TsPatGetSize(ppat, &sx, &sy, NULL, NULL);
            abgr = 0xffffff;
            abgrs = 0x808080;
            rpx = sx + px - 318;
            rpy = sy + py - 181;
            pw->roy = zm * -43.0f - rpy;
            pw->rox = zm * 0.0f - rpx;
            break;
        }

        pw->atime--;
        i = pw->atime - 20;
        if (i > 0) {
            pw->rrot += i;
            rrot = pw->rrot * 0.005f;
        } else {
            jrot = swing;
        }

        pw->innm = TSNumMov(pw->innm, 0, 6);
        rt = pw->innm / 1024.0f;
        rzr = rt * 0.8f + 1.0f;
        jzr = 1.0f - rt * 0.5f;

        ton = rt * 256.0f;
        abgr = GetDToneColor(abgr, 0xffffff, ton);
        abgrs = GetDToneColor(abgrs, 0x808080, ton);

        TsPatGetSize(ppat, &sx, &sy, NULL, NULL);
        rpx = sx + px - 318;
        rpy = sy + py - 181;
        fx = zm * 0.0f;
        fy = zm * -43.0f;
        pw->roy = fy - rpy * rt;
        pw->rox = fx - rpx * rt;

        ton = rt * 256.0f * 0.5f;
        rabgr = GetDToneColor(rabgr, 0xffffff, ton);
        rabgrs = GetDToneColor(rabgrs, 0x808080, ton);

        if (pw->atime == 0) {
            pw->state = TSJKCUS_CUR;
            pw->atime = 1;
            pw->anime = TSJKANM_RETURN1;
            pw->rox = fx;
            pw->roy = fy;
            pw->time = 0;
        }
        break;
    default:
        pw->oy = pw->rot = pw->ox = 0.0f;
        break;
    }

    jz = jzr * zm;
    rz = rzr * zm;

    spr->ofsx += addx + pw->ox;
    spr->ofsy += addy + pw->oy;

    rt = (anmtime % 240) * 2.0f / 240.0f;
    spr->ofsy += sinf((rt + JUKEWAV_INITBL[pw->patNo]) * 3.1415927f) * 8.0f * swing;

    if (pw->anime == TSJKANM_ROTATE || pw->anime == TSJKANM_ROTSTOP) {
        _TsJkJacketPut(pk, spr, pw, px, py, jz, jrot, abgr, abgrs);
        _TsJkRecordPut(pk, spr, pw, px, py, rz, rrot, rabgr, rabgrs);
    } else {
        _TsJkRecordPut(pk, spr, pw, px, py, rz, rrot, rabgr, rabgrs);
        _TsJkJacketPut(pk, spr, pw, px, py, jz, jrot, abgr, abgrs);
    }

    spr->ofsy = boy;
    spr->ofsx = box;
}

static int TsJukeIsObjAnime(int isComp) {
    int        i;
    JUKE_MENU *pfw;

    pfw = &JukeMenu;

    for (i = 0; i < PR_ARRAYSIZE(pfw->cusObj); i++) {
        if (pfw->cusObj[i].bMsk) {
            continue;
        }

        if (!isComp) {
            if (pfw->cusObj[i].anime == TSJKANM_ROTATE) {
                if (pfw->cusObj[i].atime > 120) {
                    continue;
                }
            }

            if (pfw->cusObj[i].anime == TSJKANM_ROTSTOP) {
                if (pfw->cusObj[i].atime > 20) {
                    continue;
                }
            }
        }

        if (pfw->cusObj[i].anime != TSJKANM_OFF) {
            return TRUE;
        }
    }

    return FALSE;
}

static int TsJukeObjAnime(int isOut) {
    int        i;
    JUKE_MENU *pfw = &JukeMenu;

    for (i = 0; i < 10; i++) {
        if (i < 5) {
            pfw->cusObj[i].dir2 = i & 1;
            pfw->cusObj[i].dir = 1;
            if (!isOut) {
                pfw->cusObj[i].state = 1;
                pfw->cusObj[i].anime = TSJKANM_IN;
                pfw->cusObj[i].atime = (5 - i) * 6 + 1;
            } else {
                pfw->cusObj[i].anime = TSJKANM_OUT;
                pfw->cusObj[i].atime = (5 - i) * 6 + 1;
            }
        } else {
            pfw->cusObj[i].dir2 = (i + 1) & 1;
            pfw->cusObj[i].dir = 0;
            if (!isOut) {
                pfw->cusObj[i].state = 1;
                pfw->cusObj[i].anime = TSJKANM_IN;
                pfw->cusObj[i].atime = (i - 5) * 6 + 19;
            } else {
                pfw->cusObj[i].anime = TSJKANM_OUT;
                pfw->cusObj[i].atime = (i - 5) * 6 + 19;
            }
        }
    }

    return 0;
}

static int TsJukeObjAnime2(int isOut) {
    int        i;
    JUKE_MENU *pfw = &JukeMenu;

    for (i = 0; i < 10; i++) {
        if (i == pfw->selno) {
            switch (isOut) {
            case 0:
                pfw->cusObj[i].atime = 1;
                pfw->cusObj[i].state = 2;
                pfw->cusObj[i].anime = TSJKANM_PLAY1;
                break;
            case 2:
                pfw->cusObj[i].atime = 0;
                pfw->cusObj[i].state = isOut;
                pfw->cusObj[i].anime = TSJKANM_ROTSTOP;
                break;
            }
        } else {
            pfw->cusObj[i].atime = (rand() % 3) * 3 + 1;
            switch (isOut) {
            case 0:
                pfw->cusObj[i].anime = TSJKANM_PLAY2;
                break;
            case 1:
                pfw->cusObj[i].state = isOut;
                pfw->cusObj[i].anime = TSJKANM_RETURN2;
                break;
            }
        }
    }

    return 0;
}

/* static */ int _TsJKMoveCus(int *cx, int *cy, int mx, int my, JUKECDOBJ *cobj) {
    int ox = *cx;
    int oy = *cy;
    int x, y, pos;
    int i;

    x   = ox;
    y   = TSLOOP(oy + my, 2);
    pos = y * 5 + x;

    if (mx != 0) {
        pos = TSLOOP(pos + mx, 10);
        x   = pos % 5;
        y   = pos / 5;
    }

    if (!cobj[pos].bMsk) {
        *cx = x;
        *cy = y;
        return 1;
    }

    if (mx != 0) {
        if (mx > 0) {
            mx = 1;
        } else {
            mx = -1;
        }
        for (i = 0; i < 10; i++) {
            pos = TSLOOP(pos + mx, 10);
            if (!cobj[pos].bMsk) {
                x = pos % 5;
                y = pos / 5;
                *cx = x;
                *cy = y;
                return (ox != x || oy != y);
            }
        }
        return 0;
    }

    for (i = x; i < 5; i++) {
        pos = y * 5 + i;
        if (!cobj[pos].bMsk) {
            x = pos % 5;
            y = pos / 5;
            *cx = x;
            *cy = y;
            return 1;
        }
    }

    for (i = x; i >= 0; i--) {
        pos = y * 5 + i;
        if (!cobj[pos].bMsk) {
            x = pos % 5;
            y = pos / 5;
            *cx = x;
            *cy = y;
            return (ox != x || oy != y);
        }
    }

    return 0;
}

/* static */ void _TsJKSetPadArrow(int sel, JUKECDOBJ *cobj) {
    int bx, by;
    int flg;
    int x = sel % 5;
    int y = sel / 5;

    flg = 0;

    bx = x;
    by = y;
    if (_TsJKMoveCus(&bx, &by, -1, 0, cobj)) {
        flg |= 1;
    }

    bx = x;
    by = y;
    if (_TsJKMoveCus(&bx, &by, 1, 0, cobj)) {
        flg |= 2;
    }

    bx = x;
    by = y;
    if (_TsJKMoveCus(&bx, &by, 0, -1, cobj)) {
        flg |= 4;
    }

    bx = x;
    by = y;
    if (_TsJKMoveCus(&bx, &by, 0, 1, cobj)) {
        flg |= 8;
    }

    MENUSubt_PadFontArrowSet(flg);
}

/* static */ int TsJukeMenu_Flow(int flg, u_int tpad) {
    JUKE_MENU *pfw = &JukeMenu;
    int i;
    int state;
    int sel;
    int osel;
    int selx, sely;
    static int scstate;
    static int scstPos;

    if (flg == 1) {
        memset(pfw, 0, sizeof(*pfw));

        pfw->selno = tpad;
        if (tpad >= 10) {
            pfw->selno = 0;
        }

        pfw->state = 0;
        JukeMenu_Sw = flg;
        pfw->anmTime = 0;
        scstate = 0;
        scstPos = 0;

        for (i = 0; i < 10; i++) {
            TSJukeCDObj_Init(&pfw->cusObj[i], i);
        }

        pfw->cusObj[8].bMsk = 0;

        for (i = 0; i < 8; i++) {
            int no;

            switch (i) {
            case 0:
                no = 0;
                break;
            case 1:
                no = 1;
                break;
            case 2:
                no = 2;
                break;
            case 3:
                no = 3;
                break;
            case 4:
                no = 4;
                break;
            case 5:
                no = 5;
                break;
            case 6:
                no = 6;
                break;
            case 7:
            default:
                no = 7;
                break;
            }

            if (pP3GameState->pLog->clrCOOL[i] < 4) {
                pfw->cusObj[no].bMsk = 1;
            }
        }

        for (i = 0; i < 8; i++) {
            if (pP3GameState->pLog->clrVSCOM1[i] < 4) {
                break;
            }
        }
        if (i < 8) {
            pfw->cusObj[9].bMsk = 1;
        }

        if (pfw->cusObj[pfw->selno].bMsk) {
            for (i = 0; i < 10; i++) {
                if (!pfw->cusObj[i].bMsk) {
                    pfw->selno = i;
                    break;
                }
            }
        }
        return 0;
    }

    if (flg == 2) {
        JukeMenu_Sw = 0;
        scstate = 0;
        scstPos = 0;
        return 0;
    }

    switch (scstate) {
    case 0:
        if (scstPos) {
            scstate = 0x2100;
        }
        break;
    case 0x100:
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[1]);
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[34]);
        scstate = 0x110;
        /* fallthrough */
    case 0x110:
        if (!TsAnimeWait_withKeySkip(tpad, &MNS_StageMap, 0, -1)) {
            MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[3]);
            scstate = 0;
        }
        break;
    case 0x2000:
        if (!scstPos) {
            scstate = 0x100;
        }
        break;
    case 0x2100:
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[4]);
        scstate = 0x2180;
        break;
    case 0x2180:
        if (TsAnimeWait_withKeySkip(tpad, &MNS_StageMap, 0, -1)) {
            break;
        }
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[1]);
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[33]);
        scstate = 0x2200;
        /* fallthrough */
    case 0x2200:
        if (!TsAnimeWait_withKeySkip(tpad, &MNS_StageMap, 0, -1)) {
            MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimePA[5]);
            scstate = 0x2000;
        }
        break;
    }

    state = pfw->state;
    pfw->anmTime++;

    switch (state) {
    case 0:
        TsJukeObjAnime(0);
        state = 0x100;
        pfw->cusObj[pfw->selno].time = 0;
        pfw->cusObj[pfw->selno].state = TSJKCUS_CUR;
        /* fallthrough */
    case 0x100:
        if (TsJukeIsObjAnime(1)) {
            break;
        }
        /* fallthrough */
    case 0x1000:
        pfw->exitflg = 0;
        state = 0x1100;
        break;
    case 0x1100:
        _TsJKSetPadArrow(pfw->selno, pfw->cusObj);
        TsCMPMes_SetMes(JukeMenu_CmpMesNo[pfw->selno]);

        if (TsCheckTimeMapChange()) {
            break;
        }

        osel = sel = pfw->selno;
        selx = osel % 5;
        sely = osel / 5;

        if (tpad & SCE_PADLup) {
            _TsJKMoveCus(&selx, &sely, 0, -1, pfw->cusObj);
        } else if (tpad & SCE_PADLdown) {
            _TsJKMoveCus(&selx, &sely, 0, 1, pfw->cusObj);
        } else if (tpad & SCE_PADLleft) {
            _TsJKMoveCus(&selx, &sely, -1, 0, pfw->cusObj);
        } else if (tpad & SCE_PADLright) {
            _TsJKMoveCus(&selx, &sely, 1, 0, pfw->cusObj);
        }

        sel = sely * 5 + selx;
        if (osel != sel) {
            sel = TSLIMIT(sel, 0, 10);
            pfw->selno = sel;
            (pfw->cusObj + osel)->state = TSJKCUS_OFF;
            (pfw->cusObj + osel)->time = 0;
            (pfw->cusObj + sel)->state = TSJKCUS_ON;
            (pfw->cusObj + sel)->time = 0;
            TSSNDPLAY(VSND_MVCUS_LR);
        }

        if (tpad & SCE_PADRdown) {
            pfw->exitflg = 1;
            TSSNDPLAY(VSND_CANCEL);
            state = 0xf020;
        } else if (tpad & SCE_PADRright) {
            pfw->exitflg = 0;
            (pfw->cusObj + sel)->state = TSJKCUS_SELOK;
            (pfw->cusObj + sel)->time = 0;
            TSSNDPLAY(VSND_SELMODE);
            state = 0x3000;
        }
        break;
    case 0x3000:
        scstPos = 1;
        state = 0x3010;
        TsCMPMes_SetMes(-1);
        TsJukeObjAnime2(0);
        MenuDataDiskSndReq(JukeBgmTbl[pfw->selno].bgmNo);
        /* fallthrough */
    case 0x3010:
        if (TsJukeIsObjAnime(0)) {
            break;
        }
        TsBGMMute(40);
        state = 0x3020;
        /* fallthrough */
    case 0x3020:
        if (!TsJukeIsObjAnime(1) && (tpad & 0x840)) {
            pfw->exitflg = 1;
            TSSNDPLAY(VSND_CANCEL);
            state = 0x6000;
        }
        if (MenuDataDiskSndReady()) {
            break;
        }
        /* fallthrough */
    case 0x3f00:
        TsBGMPause(1);
        state = 0x4000;
        MenuDataDiskSndPlay();
        MenuDataDiskVolume(128);
        memset(&pfw->MNS_StageMapW, 0, sizeof(pfw->MNS_StageMapW));
        MNScene_CopyState(&pfw->MNS_StageMapW, &MNS_StageMap);
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapAnimeSEA[1]);
        MNScene_StartAnime(&MNS_StageMap, -1, &StageMapBGMCamera[pfw->selno]);
        pfw->timeV = 0;
        break;
    case 0x4000:
        if (!TsJukeIsObjAnime(1)) {
            pfw->timeV++;
            if (pfw->timeV >= JukeBgmTbl[pfw->selno].endV) {
                state = 0x6000;
            } else if (pfw->timeV + 60 < JukeBgmTbl[pfw->selno].endV && (tpad & 0x840)) {
                pfw->exitflg = 1;
                TSSNDPLAY(VSND_CANCEL);
                state = 0x5000;
            }
        }
        break;
    case 0x5000:
        state = 0x5008;
        _bMapCaptureReq = 1;
        break;
    case 0x5008:
    {
        int mn;
        u_int AnmBit = 0x80000000;

        mn = TsMENU_GetMapNo(NULL);
        state = 0x5010;
        TsSetScene_Map(&MNS_StageMap2, CurMapNo, CurMapOldFlg, 0);
        MNScene_CopyState(&MNS_StageMap, &pfw->MNS_StageMapW);
        TsMENU_SetMapScreen(mn);
        MNScene_CopyState(&MNS_StageMap, &pfw->MNS_StageMapW);
        MNScene_DispSw(&MNS_StageMap2, 1);
        MNScene_SetAnimeBankEnd(&MNS_StageMap2, MNScene_StartAnime(&MNS_StageMap2, -1, &StageMapAnimePA[33]) | AnmBit);
        MNScene_DispSw(&MNS_StageMap, 0);
        pfw->bgmFadeVol = 128;
        TsJukeObjAnime2(2);
    }
        /* fallthrough */
    case 0x5010:
        if (pfw->bgmFadeVol > 0) {
            pfw->bgmFadeVol -= 3;
            if (pfw->bgmFadeVol < 0) {
                pfw->bgmFadeVol = 0;
            }
            MenuDataDiskVolume(pfw->bgmFadeVol);
        }
        if (TsSCFADE_Set(5, 60, 2)) {
            break;
        }
        state = 0x5020;
        MNScene_DispSw(&MNS_StageMap2, 0);
        MNScene_End(&MNS_StageMap2);
        MNScene_StartAnime(&MNS_StageMap, -1, StageMapAnimeSEA);
        MNScene_DispSw(&MNS_StageMap, 1);
        MenuDataDiskSndEnd();
        TsJukeObjAnime2(1);
        scstPos = 0;
        TsBGMPause(0);
        TsBGMPlay(pP3GameState->nStage + 1, 20);
        /* fallthrough */
    case 0x5020:
        if (!TsJukeIsObjAnime(1)) {
            state = 0x1000;
        }
        break;
    case 0x6000:
    {
        int mn;

        TsJukeObjAnime2(2);
        state = 0x6010;
        MenuDataDiskSndEnd();
        mn = TsMENU_GetMapNo(NULL);
        MNScene_CopyStateMdl(&pfw->MNS_StageMapW, &MNS_StageMap);
        TsMENU_SetMapScreen(mn);
        MNScene_CopyState(&MNS_StageMap, &pfw->MNS_StageMapW);
        MNScene_StartAnime(&MNS_StageMap, -1, StageMapAnimeSEA);
        scstPos = 0;
        TsBGMPause(0);
        TsBGMPlay(pP3GameState->nStage + 1, 60);
        break;
    }
    case 0x6010:
        if (TsJukeIsObjAnime(1) && !TsJukeIsObjAnime(0)) {
            break;
        }
        TsJukeObjAnime2(1);
        state = 0x6020;
        /* fallthrough */
    case 0x6020:
        if (!TsJukeIsObjAnime(1)) {
            state = 0x1000;
        }
        break;
    case 0xf000:
    case 0xf010:
    case 0xf020:
        TsJukeObjAnime(1);
        state = 0xf080;
        /* fallthrough */
    case 0xf080:
        if (TsJukeIsObjAnime(1)) {
            break;
        }
        /* fallthrough */
    case 0xf100:
        JukeMenu_Sw = 0;
        if (pfw->exitflg) {
            state = 0xff20;
            break;
        }
        return -1;
    case 0xff20:
        return -1;
    }

    pfw->state = state;
    return 0;
#undef scstate
#undef scstPos
}

static void TsJukeMenu_Draw(SPR_PKT pk, SPR_PRM *spr) {
    JUKE_MENU *pfw;
    int        i;

    pfw = &JukeMenu;

    spr->zoom.isOn = FALSE;
    spr->zx = 1.0f;
    spr->zy = 0.5f;

    PkALPHA_Add(pk, 0x44);

    for (i = 0; i < PR_ARRAYSIZE(pfw->cusObj); i++) {
        if (i != pfw->selno) {
            TSJukeCDObj_Draw(pk, spr, &pfw->cusObj[i], 0, 0, pfw->anmTime);
        }
    }

    if (pfw->selno < PR_ARRAYSIZEU(pfw->cusObj)) {
        TSJukeCDObj_Draw(pk, spr, &pfw->cusObj[pfw->selno], 0, 0, pfw->anmTime);
    }

    spr->zoom.isOn = FALSE;
}

static void TsCmnCell_CusorSET(CELLOBJ *obj) {
    obj->state = 0;
    obj->flg = 1;
    obj->ton = 0x80;
}

static void TsCmnCell_CusorON(CELLOBJ *obj) {
    obj->state = 1;
    obj->flg = 0;
    obj->tim = 0x10;
}

static void TsCmnCell_CusorOFF(CELLOBJ *obj) {
    obj->state = 2;
    obj->flg = 1;
    obj->tim = 0x10;
}

static void TsCmnCell_CusorSEL(CELLOBJ *obj) {
    obj->state = 3;
    obj->flg = 1;
    obj->tim = 0x18;
}

static void TsCmnCell_CusorMASK(CELLOBJ *obj) {
    obj->state = 0;
    obj->flg = 0xffff;
}

/* static */ void TsCmnCell_CusorDraw(SPR_PKT pk, SPR_PRM *spr, int n, CELLOBJ *obj, int ox, int oy, int CurColor) {
    TSTEX_INF *ptex;
    int flg;
    int ton;
    int t;
    float ft;

    if ((u_int)(n + 1) >= 9) {
        return;
    }

    if ((u_int)n < 7 && obj != NULL) {
        switch (obj->state) {
        case 1:
            if (--obj->tim == 0) {
                obj->flg = 1;
                obj->state = 0;
            } else {
                obj->ton = obj->tim * 8 + 0x100;
            }
            obj->flg = 1;
            break;
        case 2:
            if (--obj->tim == 0) {
                obj->state = 0;
                obj->flg = 0;
            } else {
                t = obj->tim * 16;
                if (t > 90) {
                    obj->flg = 1;
                    obj->ton = ((t - 90) << 8) / 166;
                } else {
                    obj->flg = 0;
                    obj->ton = ((90 - t) << 8) / 90;
                }
            }
            break;
        case 3:
            if (--obj->tim == 0) {
                obj->state = 0;
                obj->flg = 1;
            } else {
                ft = sinf((obj->tim % 8) * 0.125f * 3.1415927f);
                obj->flg = 1;
                obj->ton = (int)(ft * 256.0f) + 0x100;
            }
            break;
        }

        if (obj->state == 0) {
            obj->ton = 0x100;
        }
    }

    if ((u_int)n < 7) {
        ton = obj->ton;
        flg = obj->flg;
    } else {
        ton = 0x100;
        flg = 0;
    }

    if (flg == 1) {
        PkALPHA_Add(pk, 0x48);
        spr->rgba0 = GetDToneColor(0, CurColor, ton);
    } else if (flg == -1) {
        return;
    } else {
        PkALPHA_Add(pk, 0x42);
        spr->rgba0 = GetDToneColor(0x808080, 0x10808080, ton);
    }

    ptex = &tblTex[1];
    spr->zx = 1.0f;
    spr->zy = 0.47f;
    PkTEX0_Add(pk, ptex->tex0);

    spr->ux = 0;
    spr->uy = 0;
    spr->uw = ptex->w;
    spr->uh = ptex->h;
    spr->px = CellCusPos[n + 1].x + ox;
    spr->py = CellCusPos[n + 1].y + oy;
    spr->sw = 0x234;
    spr->sh = ptex->h * 2;
    PkNSprite_Add(pk, spr, PKSPR_UV_RECT);
    PkALPHA_Add(pk, 0x44);
}

/* static */ int TsOption_Flow(int flg, u_int tpad) {
    OPTION_MENU  *pfw = &OptionMenu;
    int           state;
    int           sel;
    int           i;
    int           l;
    int          *psw;
    MNOPT_SELINF *pselw;
    static int opt_lang;
    static int opt_subt;
    static int opt_vibr;
    static int opt_oneb;

    if (flg == 1) {
        pfw->state = 0;
        pfw->selno = 0;
        opt_lang = pP3GameState->pGameStatus->language_type;
        opt_subt = pP3GameState->pGameStatus->subtitle;
        opt_vibr = pP3GameState->pGameStatus->vibration;
        opt_oneb = pP3GameState->pGameStatus->play_table_modeG;
        memset(pfw->cellcs, 0, sizeof(pfw->cellcs));
        memset(pfw->btnlr, 0, sizeof(pfw->btnlr));
        TsCmnCell_CusorSET(&pfw->cellcs[pfw->selno]);

        pselw = OptionSelTbl;
        for (i = 0; i < PR_ARRAYSIZEU(pfw->sw); i++, pselw++) {
            switch (i) {
            case 0:
                psw = &opt_lang;
                break;
            case 1:
                psw = &opt_subt;
                break;
            case 2:
                psw = &opt_vibr;
                break;
            default:
                psw = &opt_oneb;
                break;
            }

            for (l = 0; l < pselw->nObj; l++) {
                if (pselw->pObjTbl[l].workVol == *psw) {
                    break;
                }
            }

            if (l < pselw->nObj) {
                pfw->sw[i] = l;
            } else {
                pfw->sw[i] = 0;
            }
        }
        return 0;
    }

    state = pfw->state;
    if (flg == 2) {
        return 0;
    }

    switch (state) {
    case 0:
        state = 0x1000;
    case 0x1000:
    {
        int osel;
        int old;

        osel = sel = pfw->selno;
        if (tpad & SCE_PADLup) {
            sel--;
        }
        if (tpad & SCE_PADLdown) {
            sel++;
        }
        if (osel != sel) {
            sel = TSLOOP(sel, 4);
            TsCmnCell_CusorOFF(&pfw->cellcs[osel]);
            TsCmnCell_CusorON(&pfw->cellcs[sel]);
            pfw->selno = sel;
            TSSNDPLAY(OptionSelTbl[sel].voiceNo);
            TSSNDPLAY(5);
        }

        osel = pfw->selno;
        old = sel = pfw->sw[osel];
        if (tpad & SCE_PADLleft) {
            sel--;
        }
        if (tpad & SCE_PADLright) {
            sel++;
        }
        if (old != sel) {
            int max;

            /* Start the press timer of the left or right arrow */
            pfw->btnlr[osel].tim[(sel < old) ? 0 : 1] = 6;
            max = OptionSelTbl[osel].nObj;
            pfw->sw[pfw->selno] = TSLOOP(sel, max);
            TSSNDPLAY(2);
        }

        if (tpad & SCE_PADRright) {
            pfw->exitflg = 0;
            state = 0xf000;
            TSSNDPLAY(6);
        }
        if (tpad & SCE_PADRdown) {
            state = 0xf000;
            pfw->exitflg = 1;
            TSSNDPLAY(9);
        }
        TsCMPMes_SetMes(OptionSelTbl[pfw->selno].cmpMesNo);
    }
        break;
    case 0xf000:
        if (pfw->exitflg == 0) {
            pselw = OptionSelTbl;
            for (i = 0; i < PR_ARRAYSIZEU(pfw->sw); i++, pselw++) {
                switch (i) {
                case 0:
                    psw = &opt_lang;
                    break;
                case 1:
                    psw = &opt_subt;
                    break;
                case 2:
                    psw = &opt_vibr;
                    break;
                default:
                    psw = &opt_oneb;
                    break;
                }
                *psw = pselw->pObjTbl[pfw->sw[i]].workVol;
            }

            pP3GameState->pGameStatus->language_type = opt_lang;
            pP3GameState->pGameStatus->subtitle = opt_subt;
            pP3GameState->pGameStatus->vibration = opt_vibr;
            pP3GameState->pGameStatus->play_table_modeG = opt_oneb;
            TsCMPMes_SetMes(-1);
        }
    case 0xf100:
        if (pfw->exitflg == 0) {
            state = 0xff10;
        } else {
            state = 0xff20;
        }
        break;
    case 0xff10:
        return 1;
    case 0xff20:
        return -1;
    }

    pfw->state = state;
    return 0;
}

static void TsOption_Draw(SPR_PKT pk, SPR_PRM *spr) {
    int           i;
    OPTION_MENU  *pfw = &OptionMenu;
    MNOPT_SELINF *pselw;
    int           l;
    PATPOS       *ppat;
    float         zr;

    for (i = 0; i < PR_ARRAYSIZEU(pfw->cellcs); i++) {
        TsCmnCell_CusorDraw(pk, spr, i, &pfw->cellcs[i], 0, 0, 0x1a808080);
    }

    for (i = 0; i < PR_ARRAYSIZEU(pfw->btnlr); i++) {
        spr->rgba0 = 0x800062ff;
        TsPatPut(pk, spr, &MNOptMiniFrm[i], 0, 0);

        for (l = 0; l < 2; l++) {
            ppat = &MNOptLRBtn[i * 2 + l];

            if (pfw->btnlr[i].tim[l] > 0) {
                pfw->btnlr[i].tim[l]--;
                spr->rgba0 = MN_COLOR_WHITE;
                zr = (float)pfw->btnlr[i].tim[l] * 0.5 * (1.0f / 6.0f) + 1.0;
            } else {
                spr->rgba0 = MN_COLOR_NEUTRAL;
                zr = 1.0f;
            }

            TsPatPutRZoom(pk, spr, ppat, 0, 0, zr, (l == 0) ? -1.5707964f : 1.5707964f);
        }
    }

    PkALPHA_Add(pk, 0x44);
    spr->zx = 1.0f;
    spr->zy = 0.5f;
    spr->rgba0 = MN_COLOR_NEUTRAL;

    pselw = OptionSelTbl;
    for (i = 0; i < PR_ARRAYSIZEU(pfw->sw); i++, pselw++) {
        TsPatPut(pk, spr, &pselw->pObjTbl[pfw->sw[i]].ppat, 0, 0);
    }
}

static int TsUserList_GetCurFileNo(int *isBroken) {
    USERLIST_MENU *pfw   = &UserListMenu;
    USER_DATA     *puser = pfw->pusrlst->pUserTbl[pfw->curuser + pfw->curPageTop];

    if (isBroken != NULL) {
        *isBroken = (puser->flg == 2);
    }

    return puser->fileNo;
}

static int TsUserList_IsGetFileSave(void) {
    USERLIST_MENU *pfw = &UserListMenu;
    return pfw->isSave;
}

static int TsUserList_SortUser(void) {
    USERLIST_MENU *pfw;
    int            i, maxn;

    pfw = &UserListMenu;
    pfw->pusrdspWk = UserDispWork;
    memset(pfw->pusrdspWk, 0, sizeof(*pfw->pusrdspWk));

    if (pfw->pusrlst == NULL) {
        return 0;
    }

    maxn = P3MC_SortUser(pfw->pusrlst, pfw->dataMode, pfw->isSave);
    for (i = 0; i < maxn; i++) {
        *(pfw->pusrdspWk->pUserDisp + i) = *pfw->pusrlst->pUserTbl[i];
    }

    return maxn;
}

static void TsUserList_SetCurUserData(USER_DATA *psrc) {
    USERLIST_MENU *pfw = &UserListMenu;
    USER_DATA     *puser;

    if (pfw->pusrlst == NULL) {
        return;
    }

    puser = pfw->pusrlst->pUserTbl[pfw->curuser + pfw->curPageTop];
    if (puser->flg == 0) {
        P3MC_AddUser(pfw->pusrlst, pfw->dataMode, psrc);
    } else {
        *puser = *psrc;
    }
}

static void TsUserList_SetCurDispUserData(USER_DATA *psrc) {
    USERLIST_MENU *pfw = &UserListMenu;

    *(pfw->pusrdspWk->pUserDisp + (pfw->curuser + pfw->curPageTop)) = *psrc;
}

/* static */ void TsUserList_SetCurFileNoCusor(int fileNo, P3MC_DATE *fDate) {
    USERLIST_MENU *pfw = &UserListMenu;
    USER_DATA     *puser;
    int            i;

    if (fileNo < 0 || pfw->pusrlst == NULL) {
        return;
    }

    for (i = 0; i < pfw->userMax; i++) {
        puser = pfw->pusrlst->pUserTbl[i];
        if (puser->fileNo == fileNo && P3MC_DATE_WORD(&puser->date, 0) == P3MC_DATE_WORD(fDate, 0) && P3MC_DATE_WORD(&puser->date, 1) == P3MC_DATE_WORD(fDate, 1)) {
            break;
        }
    }

    if (i >= pfw->userMax) {
        return;
    }

    pfw->curuser = 2;
    pfw->curPageTop = i - 2;
    if (i + 3 >= pfw->userMax) {
        pfw->curPageTop = pfw->userMax - 5;
        pfw->curuser = i - pfw->curPageTop;
    }
    if (pfw->curPageTop < 0) {
        pfw->curuser += pfw->curPageTop;
        pfw->curPageTop = 0;
    }
}

static void TsUserList_SetType(USERLISTTYPE_TABLE *ptbl, int mode, int curTag) {
    USERLIST_MENU *pfw = &UserListMenu;

    TsUserList_Flow(1, 0, 0);

    pfw->ptypttbl = ptbl;
    pfw->gameMode = mode;

    TsUserList_Flow(3, curTag, 0);
}

static int TsUserList_TagChangeAble(USERLIST_MENU *pfw, int *pno) {
    int flg, no;

    no = *pno;
    if (no < 0) {
        no = 0;
    }
    if (no >= pfw->ptypttbl->nType) {
        no = (pfw->ptypttbl->nType - 1);
    }

    if (UserListTbl[pfw->ptypttbl->typeNo[no]].isSave) {
        flg = UCheckSaveError;
    } else {
        flg = UCheckLoadError;
    }

    if (flg != 0) {
        *pno = no;
    }

    return flg;
}

/* static */ int TsUserList_SetCurTag(USERLIST_MENU *pfw, int no) {
    USERLIST_TYPE *ptbl;
    int            fileNo;

    TsUserList_TagChangeAble(pfw, &no);
    TsUserList_Flow(2, 0, 0);

    fileNo = -1;
    ptbl = &UserListTbl[pfw->ptypttbl->typeNo[no]];

    pfw->isSave = ptbl->isSave;
    pfw->scene = ptbl->pScene;
    pfw->dataMode = ptbl->dataMode;
    pfw->cmpMesTbl = ptbl->cmpMesTbl;
    pfw->dispColor = ptbl->dispColor;
    pfw->nTag = no;

    if (pfw->dataMode == 1) {
        pfw->curFileDate = CurFileInfo.logDate;
        if (!pfw->isSave) {
            fileNo = CurFileInfo.logFileNo;
        } else {
            fileNo = CurFileInfo.logFileNo;
        }
    } else {
        pfw->curFileDate = CurFileInfo.repDate;
        if (!pfw->isSave) {
            fileNo = CurFileInfo.repFileNo;
        }
    }

    pfw->curFileNo = fileNo;
    return 0;
}

/* static */ int TsUserList_Flow(int flg, u_int tpad, u_int tpad2) {
    USERLIST_MENU *pfw = &UserListMenu;
    USER_DATA     *puser;
    int            state;
    int            sely;
    int            osely;
    int            optop;
    int            ret;
    int            ret2;
    int            i;
    int            bScrollEnd;
    int            nCell;
    int            err;
    int            dumy;
    int            sflg;
    int            errNo;

    if (flg == 1) {
        pfw->curFileNo = -1;
        pfw->state = 0;
        pfw->wuser = UserWork;
        pfw->ptypttbl = NULL;
        pfw->scene = NULL;
        pfw->nTag = 0;
        pfw->dataMode = 0;
        pfw->dispColor = 0;
        pfw->isSave = 0;
        pfw->cmpMesTbl = NULL;
        pfw->gameMode = 0;
        pfw->curuser = 0;
        pfw->curPageTop = 0;
        pfw->isNameIn = 0;
        pfw->sline = 0.0f;
        pfw->pusrlst = NULL;
        pfw->exitflg = 0;
        pfw->userMax = TsUserList_SortUser();
        pfw->mcerrNo = 0;
        pfw->mcRetTag = 0;

        TsNAMEINBox_Flow(2, &pfw->nameinw[0], 0);
        TsNAMEINBox_Flow(2, &pfw->nameinw[1], 0);

        memset(pfw->cellcs, 0, sizeof(pfw->cellcs));
        for (i = 0; i < 5; i++) {
            TsCmnCell_CusorMASK(&pfw->cellcs[i]);
        }
        return 0;
    }

    if (flg == 3) {
        if (pfw->ptypttbl != NULL && TsUserList_SetCurTag(pfw, tpad)) {
            return 0;
        }

        pfw->isNameIn = 0;
        pfw->pusrlst = UserLst;
        pfw->sline = 0.0f;
        pfw->exitflg = 0;
        pfw->userMax = TsUserList_SortUser();

        TsNAMEINBox_Flow(2, &pfw->nameinw[0], 0);
        TsNAMEINBox_Flow(2, &pfw->nameinw[1], 0);

        pfw->curuser = 0;
        pfw->curPageTop = 0;

        nCell = pfw->userMax;
        if (nCell > 5) {
            nCell = 5;
        }

        if (pfw->curFileNo >= 0) {
            TsUserList_SetCurFileNoCusor(pfw->curFileNo, &pfw->curFileDate);
        }
        if (pfw->scene != NULL) {
            MNScene_StartAnime(pfw->scene, -1, &CounterAnime[nCell]);
            MNScene_DispSw(pfw->scene, 1);
        }

        memset(pfw->cellcs, 0, sizeof(pfw->cellcs));
        for (i = nCell; i < 5; i++) {
            TsCmnCell_CusorMASK(&pfw->cellcs[i]);
        }

        if (pfw->userMax > 0) {
            TsCmnCell_CusorSET(&pfw->cellcs[pfw->curuser]);
        }
        if (pfw->cmpMesTbl != NULL) {
            TsCMPMes_SetMes(pfw->cmpMesTbl[0]);
        }
        return 0;
    }

    state = pfw->state;

    if (flg == 2) {
        pfw->isNameIn = 0;
        pfw->pusrlst = NULL;
        pfw->exitflg = 0;
        if (pfw->ptypttbl != NULL) {
            for (i = 0; i < pfw->ptypttbl->nType; i++) {
                MNScene_DispSw(UserListTbl[pfw->ptypttbl->typeNo[i]].pScene, 0);
            }
        }
        pfw->scene = NULL;
        return 0;
    }

    if (state < 0xe000) {
        ret = P3MC_CheckChange();
        if (ret == 3 || ret == 5) {
            state = 0xe000;
            TsCMPMes_SetMes(-1);
            TsMCAMes_SetMes(-1);
            pfw->isNameIn = 0;
            TsNAMEINBox_Flow(2, &pfw->nameinw[0], 0);
            TsNAMEINBox_Flow(2, &pfw->nameinw[1], 0);
        }
    }

    switch (state) {
    case 0x1000:
        if (pfw->mcerrNo != 0 && McErrorMess(pfw->mcerrNo) < 0) {
            break;
        }
        pfw->nTag = pfw->mcRetTag;
        TsUserList_Flow(3, pfw->nTag, 0);
        state = 0x3000;
        break;
    case 0:
    case 0x3000:
        pfw->isNameIn = 0;
        state = 0x3100;
        pfw->exitflg = 0;
        /* fallthrough */
    case 0x3100:
        if (pfw->cmpMesTbl != NULL) {
            TsCMPMes_SetMes(pfw->cmpMesTbl[0]);
        }

        bScrollEnd = 0;
        if (pfw->sline != 0.0f) {
            if (pfw->sline > 0.0f) {
                pfw->sline -= 3.25f;
                if (pfw->sline < 0.0f) {
                    pfw->sline = 0.0f;
                }
            } else {
                pfw->sline += 3.25f;
                if (pfw->sline > 0.0f) {
                    pfw->sline = 0.0f;
                }
            }
            bScrollEnd = 1;
            if (pfw->sline != 0.0f) {
                break;
            }
        }

        sely = pfw->nTag;
        ret = sely;
        if (tpad & SCE_PADLleft) {
            sely--;
        }
        if (tpad & SCE_PADLright) {
            sely++;
        }
        sely = TSLIMIT(sely, 0, pfw->ptypttbl->nType);

        if (ret != sely) {
            if (tpad & SCE_PADLleft) {
                TSSNDPLAY(VSND_MVCUS_L);
            } else {
                TSSNDPLAY(VSND_MVCUS_R);
            }

            dumy = sely;
            err = TsUserList_TagChangeAble(pfw, &dumy);
            if (err) {
                state = 0x1000;
                pfw->mcerrNo = err;
                pfw->mcRetTag = pfw->nTag;
            }
            pfw->nTag = sely;
            TsUserList_Flow(3, sely, 0);
            break;
        }

        osely = sely = pfw->curuser;
        optop = pfw->curPageTop;
        if (pfw->userMax > 0) {
            if (tpad & SCE_PADLup) {
                sely--;
            }
            if (tpad & SCE_PADLdown) {
                sely++;
            }
            if (bScrollEnd && sely == osely) {
                if (TsGetMenuPadIsRepeat(0, 0)) {
                    sely--;
                }
                if (TsGetMenuPadIsRepeat(0, 1)) {
                    sely++;
                }
            }
        }

        if (osely != sely) {
            sflg = 1;
            if (sely < 0) {
                sflg = 2;
                pfw->curPageTop += sely;
                sely = 0;
            }
            if (sely >= 5) {
                sflg = 3;
                pfw->curPageTop += sely - 4;
                sely = 4;
            }
            if (pfw->curPageTop < 0) {
                pfw->curPageTop = 0;
                sflg = 0;
            }
            if (pfw->curPageTop + sely >= pfw->userMax) {
                sflg = 0;
                if (pfw->curPageTop == 0) {
                    sely = pfw->userMax - 1;
                } else {
                    pfw->curPageTop = (pfw->userMax - 1) - sely;
                }
            }
            pfw->curuser = sely;

            if (sflg) {
                if (sely < osely) {
                    TSSNDPLAY(VSND_MVCUS_U);
                } else {
                    TSSNDPLAY(VSND_MVCUS_D);
                }
            }

            if (optop != pfw->curPageTop) {
                if (pfw->curPageTop < optop) {
                    pfw->sline = -26.0f;
                } else {
                    pfw->sline = 26.0f;
                }
            }

            if (osely != sely) {
                TsCmnCell_CusorOFF(&pfw->cellcs[osely]);
                TsCmnCell_CusorON(&pfw->cellcs[sely]);
            }

            if (sflg == 2) {
                TsCmnCell_CusorOFF(&pfw->cellcs[sely + 1]);
                TsCmnCell_CusorON(&pfw->cellcs[sely]);
            } else if (sflg == 3) {
                TsCmnCell_CusorOFF(&pfw->cellcs[sely - 1]);
                TsCmnCell_CusorON(&pfw->cellcs[sely]);
            }
        }

        if (tpad & SCE_PADRright) {
            state = 0x3f00;
            TsCmnCell_CusorSEL(&pfw->cellcs[pfw->curuser]);
            pfw->wtim = 28;
            TSSNDPLAY(VSND_SELPOPUP);
        }
        if (tpad & SCE_PADRdown) {
            state = 0x3f10;
            TSSNDPLAY(VSND_CANCEL);
        }
        break;
    case 0x3f00:
        if (--pfw->wtim <= 0) {
            state = 0x3f08;
        }
        break;
    case 0x3f08:
        state = pfw->isSave ? 0x4000 : 0x5000;
        break;
    case 0x3f10:
        pfw->exitflg = 1;
        state = 0xff20;
        break;
    case 0x5000:
        state = 0x5010;
        TsMCAMes_SetMes(MCMES(MCMES_KIND_CONFIRM, 22));
        /* fallthrough */
    case 0x5010:
        ret = TsMCAMes_GetSelect();
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            state = 0xff10;
            break;
        }
        state = 0x3000;
        TsMCAMes_SetMes(-1);
        break;
    case 0x4000:
        puser = pfw->pusrlst->pUserTbl[pfw->curuser + pfw->curPageTop];
        state = 0x4010;
        if (puser->fileNo != 0xffff) {
            break;
        }
        state = 0x4005;
        /* fallthrough */
    case 0x4005:
        errNo = (pfw->dataMode == 2) ? 15 : 7;
        if (McErrorMess(errNo) >= 0) {
            state = 0x3000;
        }
        break;
    case 0x4010:
        puser = pfw->pusrlst->pUserTbl[pfw->curuser + pfw->curPageTop];
        if (!puser->flg) {
            state = 0x4020;
            break;
        }
        state = 0x4015;
        TsMCAMes_SetMes(MCMES(MCMES_KIND_CONFIRM, 15));
        /* fallthrough */
    case 0x4015:
        ret = TsMCAMes_GetSelect();
        if (ret == 0) {
            break;
        }
        state = (ret == 1) ? 0x4020 : 0x3000;
        TsMCAMes_SetMes(-1);
        break;
    case 0x4020:
        pfw->isNameIn = 1;
        pfw->wuser->fileNo = TsUserList_GetCurFileNo(NULL);

        if (pfw->dataMode == 1) {
            TsNAMEINBox_Flow(1, &pfw->nameinw[0], (u_int)pfw->wuser->name);
        } else {
            TsNAMEINBox_Flow(1, &pfw->nameinw[0], (u_int)pfw->wuser->name1);
        }

        if (pfw->dataMode == 1) {
            pfw->nameinw[0].dispType = 0;
        } else if (pfw->gameMode == 0) {
            pfw->nameinw[0].dispType = 0;
        } else {
            pfw->nameinw[0].dispType = 1;
            if (pfw->gameMode == 1) {
                TsNAMEINBox_Flow(1, &pfw->nameinw[1], (u_int)pfw->wuser->name2);
                pfw->nameinw[1].dispType = 2;
            }
        }

        state = 0x4030;
        if (pfw->cmpMesTbl != NULL) {
            TsCMPMes_SetMes(pfw->cmpMesTbl[1]);
        }
        /* fallthrough */
    case 0x4030:
        pfw->nameinw[0].isCan = 1;
        ret2 = 1;
        ret = TsNAMEINBox_Flow(0, &pfw->nameinw[0], tpad);
        if (pfw->gameMode == 1 && pfw->dataMode != 1) {
            pfw->nameinw[1].isCan = 1;
            ret2 = TsNAMEINBox_Flow(0, &pfw->nameinw[1], tpad2);
        }

        if (ret == -2 || ret2 == -2) {
            TsNAMEINBox_Flow(3, &pfw->nameinw[0], 0);
            TsNAMEINBox_Flow(3, &pfw->nameinw[1], 0);
        }
        if (ret == -1 || ret2 == -1) {
            state = 0x4f10;
        }
        if (ret == 1 && ret2 == 1) {
            state = 0x4f00;
        }
        break;
    case 0x4f00:
        state = 0xf000;
        break;
    case 0x4f10:
        state = 0x3000;
        TsNAMEINBox_Flow(2, &pfw->nameinw[0], 0);
        TsNAMEINBox_Flow(2, &pfw->nameinw[1], 0);
        pfw->exitflg = 1;
        break;
    case 0xe000:
        ret = P3MC_CheckChange();
        if (ret == 0 || ret == 5) {
            state = 0xff40;
            break;
        }
        err = 3;
        if (pfw->isSave) {
            err = (pfw->dataMode == 2) ? 60 : 50;
        }
        if (McErrorMess(err) >= 0) {
            state = 0xee10;
        }
        break;
    case 0xee10:
        state = 0xff20;
        break;
    case 0xf000:
        TsUserList_SetCurDispUserData(pfw->wuser);
        pfw->state = 0xf100;
        /* fallthrough */
    case 0xf100:
        TsMCAMes_SetMes(-1);
        /* fallthrough */
    case 0xff10:
        if (P3MC_CheckChange() < 0) {
            break;
        }
        pfw->exitflg = 0;
        pfw->state = 0x3000;
        pfw->isNameIn = 0;
        TsNAMEINBox_Flow(2, &pfw->nameinw[0], 0);
        TsNAMEINBox_Flow(2, &pfw->nameinw[1], 0);
        return 1;
    case 0xff20:
        if (P3MC_CheckChange() < 0) {
            break;
        }
        pfw->isNameIn = 0;
        pfw->exitflg = 1;
        TsMCAMes_SetMes(-1);
        TsNAMEINBox_Flow(2, &pfw->nameinw[0], 0);
        TsNAMEINBox_Flow(2, &pfw->nameinw[1], 0);
        return -1;
    case 0xff40:
        pfw->isNameIn = 0;
        pfw->exitflg = 1;
        TsNAMEINBox_Flow(2, &pfw->nameinw[0], 0);
        TsNAMEINBox_Flow(2, &pfw->nameinw[1], 0);
        return -3;
    }

    pfw->state = state;
    return 0;
}

/* static */ void TsUserList_Draw(SPR_PKT pk, SPR_PRM *spr) {
    USERLIST_MENU *pfw = &UserListMenu;
    u_char         buf[16];
    int            isScroll;
    float          ofsy;
    PATPOS        *scr;
    STRPOS        *ps;
    USER_DATA     *user;
    int            dispColor;
    int            i, j, k, n;
    int            px, py;
    int            y;
    int            pflg;

    ofsy = spr->ofsy;
    spr->zx = 1.0f;
    spr->zy = 0.5f;
    PkALPHA_Add(pk, 0x44);
    spr->rgba0 = MN_COLOR_NEUTRAL;

    dispColor = pfw->dispColor;
    switch (dispColor) {
    case 0:
        scr = LG_SCROLL_MARK;
        break;
    case 1:
        scr = LLG_SCROLL_MARK;
        break;
    case 2:
    default:
        scr = RP_SCROLL_MARK;
        break;
    }

    if (pfw->curPageTop > 0) {
        TsPatPut(pk, spr, &scr[0], 0, 0);
    }
    if (pfw->curPageTop + 5 < pfw->userMax) {
        TsPatPut(pk, spr, &scr[1], 0, 0);
    }
    TsPatPut(pk, spr, &CSSLASH_MARK, 0, 0);

    isScroll = 1;

    sprintf(buf, "%d", pfw->curuser + pfw->curPageTop + 1);
    ps = PAGENO_StrCOD;
    MENUFontPutS(pk, spr, ps->x, ps->y, ps->abgr, 0x201, buf);
    ps++;
    sprintf(buf, "%d", pfw->userMax);
    MENUFontPutS(pk, spr, ps->x, ps->y, ps->abgr, 0x201, buf);

    if (pfw->sline == 0.0f) {
        isScroll = 0;
    }

    if (isScroll) {
        TsMenu_CaptureVram(pk, spr);

        y = 0x29;
        PkSCISSOR_Add(pk, 0x26, 0x2a, 0x230, 0x80);

        spr->zy = 1.0f;
        spr->zx = 1.0f;
        spr->ofsy += pfw->sline;
        if (1.0f < pfw->sline) {
            y = 0xf;
        }

        PkALPHA_Add(pk, SCE_GS_SET_ALPHA(0, 1, 2, 1, 0x80));
        spr->rgba0 = MN_COLOR_NEUTRAL;
        spr->ux = 0x26;
        spr->uy = 0x43;
        spr->uw = 0x230;
        spr->uh = 0x1a;

        for (i = 0; i < 6; i++, y += 0x1a) {
            spr->px = 0x26;
            spr->py = y;
            spr->sw = 0x230;
            spr->sh = 0x1a;
            PkNSprite_AddAdj(pk, spr, PKSPR_UV_RECT);
        }

        spr->ofsy = ofsy;
        spr->zx = 1.0f;
        spr->zy = 0.5f;
        PkALPHA_Add(pk, 0x44);
    }

    spr->rgba0 = MN_COLOR_NEUTRAL;

    n = 5;
    if (isScroll) {
        n = 6;
        spr->ofsy += pfw->sline;
    }

    for (i = 0; i < n; i++) {
        if (0.0f < pfw->sline) {
            j = i - 1;
        } else {
            j = i;
        }
        if (j < 0) {
            TsCmnCell_CusorDraw(pk, spr, j, NULL, 0, 0, 0x20808080);
        } else {
            TsCmnCell_CusorDraw(pk, spr, j, &pfw->cellcs[j], 0, 0, 0x20808080);
        }
    }

    spr->ofsy = ofsy;

    n = 5;
    if (isScroll) {
        n = 6;
        spr->ofsy = ofsy + pfw->sline;
    }

    for (i = 0; i < n; i++) {
        int jj;
        if (0.0f < pfw->sline) {
            jj = i - 1;
        } else {
            jj = i;
        }
        k  = jj + pfw->curPageTop;
        px = CellCusPos[jj + 1].x;
        py = CellCusPos[jj + 1].y;

        if (k >= 0 && k < pfw->userMax) {
            user = &pfw->pusrdspWk->pUserDisp[k];

            if (pfw->curuser == i) {
                pflg = (pfw->nameinw[0].nameMsk == 1) ? 2 : 0;
                if (pfw->nameinw[1].nameMsk == 1) {
                    pflg |= 4;
                }
                if (pfw->nameinw[0].nameMsk || pfw->nameinw[1].nameMsk) {
                    user = pfw->wuser;
                }
            } else {
                pflg = 0;
            }

            TsUser_PanelDraw(pk, spr, user, px, py, pflg, dispColor);
        }
    }

    spr->ofsy = ofsy;
    PkDefSCISSOR_Add(pk);

    px = CellCusPos[pfw->curuser + 1].x;
    py = CellCusPos[pfw->curuser + 1].y;

    if (pfw->nameinw[0].isOn) {
        TsNAMEINBox_Draw(pk, spr, px, py, dispColor, &pfw->nameinw[0], 0);
    }
    if (pfw->nameinw[1].isOn) {
        TsNAMEINBox_Draw(pk, spr, px, py, dispColor, &pfw->nameinw[1], 1);
    }
}

static void NameSpaceCut(u_char *dst, u_char *src) {
    int     i, l;
    u_char *ps;
    u_char  c;

    ps = src;

    while (*ps == ' ') {
        ps++;
    }

    l = 0;

    while ((c = *ps++) != '\0') {
        dst[l] = c;
        l++;
    }

    dst[l] = '\0';

    for (i = l - 1; i > 0; i--) {
        if (dst[i] != ' ') {
            break;
        }

        dst[i] = '\0';
    }
}

/* static */ void TsUser_PanelDraw(SPR_PKT pk, SPR_PRM *spr, USER_DATA *user, int px, int py, int pflg, int isLog) {
    u_char  buf[32];
    STRPOS *strpos;
    STRPOS *ps;
    u_int   m;

    spr->zx = 1.0f;
    spr->zy = 0.5f;
    PkALPHA_Add(pk, 0x44);

    if (user == NULL || user->flg == 0) {
        spr->rgba0 = MN_COLOR_NEUTRAL;
        TsPatPut(pk, spr, (isLog >= 0) ? ((isLog < 2) ? &LG_NEWDATA_MARK : &RP_NEWDATA_MARK) : &RP_NEWDATA_MARK, px, py);
        return;
    }

    if (user->mode == 2) {
        if (user->isVs) {
            spr->rgba0 = MN_COLOR_NEUTRAL;
            TsPatPut(pk, spr, &VS_MARK, px, py);

            switch (user->winner) {
            case 0:
                TsPatPut(pk, spr, &VS_WINMARK1, px, py);
                break;
            case 1:
                TsPatPut(pk, spr, &VS_WINMARK2, px, py);
                break;
            case 2:
                break;
            }

            strpos = VSREPLAY_StrCOD;
        } else {
            strpos = REPLAY_StrCOD;
        }

        sprintf(buf, "STAGE%d", user->stageNo);
    } else {
        if (isLog) {
            strpos = LOGL_StrCOD;
        } else {
            strpos = LOGS_StrCOD;
        }

        if (user->roundNo) {
            int round = (user->roundNo + 1 > 99) ? 99 : user->roundNo + 1;
            sprintf(buf, "CIRCUIT%d", round);
        } else {
            sprintf(buf, "STAGE%d", user->stageNo);
        }
    }

    if (user->flg == 2) {
        strcpy(buf, " STAGE?");
    }
    ps = strpos;
    MENUFontPutS(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, buf);

    if (user->flg != 2 && user->date.year != 0) {
        m = user->date.month;
        if (m >= 19) {
            m = 18;
        }
        sprintf(buf, "%02x.%s.%04x", user->date.day, _MONTH_STR[m], user->date.year);
        ps = &strpos[1];
        MENUFontPutS(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, buf);

        sprintf(buf, "%02x:%02x", user->date.hour, user->date.minute);
        ps = &strpos[2];
        MENUFontPutS(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, buf);
    } else {
        ps = &strpos[1];
        MENUFontPutS(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, "--.---.----");
        ps = &strpos[2];
        MENUFontPutS(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, "--:--");
    }

    sprintf(buf, "%02d", user->fileNo + 1);
    ps = &strpos[3];
    MENUFontPutS(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, buf);

    ps = &strpos[4];
    if (!(pflg & 2)) {
        if (user->mode == 1) {
            NameSpaceCut(buf, user->name);
        } else {
            NameSpaceCut(buf, user->name1);
        }
        MENUFontPutL(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, buf);
    }

    if (user->mode == 2) {
        sprintf(buf, "%06d", user->score);
        ps = &strpos[5];
        MENUFontPutS(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, buf);

        if (user->isVs) {
            ps = &strpos[6];
            if (!(pflg & 4)) {
                NameSpaceCut(buf, user->name2);
                MENUFontPutL(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, buf);
            }

            sprintf(buf, "%06d", user->score2);
            ps = &strpos[7];
            MENUFontPutS(pk, spr, ps->x + px, ps->y + py, ps->abgr, 0x201, buf);
        }
    }
}

/* .sdata after TsUser_PanelDraw's string literals */
int _TexFunc = 0;
HOSI_OBJ *HOSIObj = NULL;
MAP_TIME MapTime = { 0 };

/* static */ void TsNAMEINBox_SetName(NAMEINW *pfw, u_char *name) {
    u_short       *pcode = pfw->curnchr;
    int            i, l, j;
    USERNAME_CSET *cset;
    u_char        *pchrlst;

    if (name == NULL || *name == '\0') {
        name = UserName_InitialStr;
    }

    for (i = 0; i < PR_ARRAYSIZE(pfw->curnchr); i++, pcode++) {
        if (*name == '\0') {
            *pcode = 0x27;
            continue;
        }

        cset = UserName_CharSet;
        for (j = 0; j < 2; j++, cset++) {
            pchrlst = cset->ptbl;
            for (l = 0; l < cset->len; l++, pchrlst++) {
                if (*name == *pchrlst) {
                    break;
                }
            }

            if (l < cset->len) {
                break;
            }
        }

        if (j >= 2) {
            *pcode = 0x27;
        } else {
            *pcode = USERNAME_CHAR(j, l);
        }

        name++;
    }
}

static void TsNAMEINBox_GetName(NAMEINW *pfw, u_char *name) {
    u_short *pcode = pfw->curnchr;
    int      i;

    for (i = 0; i < PR_ARRAYSIZE(pfw->curnchr); i++, pcode++) {
        *name++ = UserName_CharSet[(*pcode / (1 << USERNAME_CHAR_SET_SHIFT))].ptbl[*pcode & USERNAME_CHAR_INDEX_MASK];
    }

    *name = '\0';
}

/* static */ int TsNAMEINBox_Flow(int flg, NAMEINW *pfw, u_int tpad) {
    int i;
    int state;
    int aflg;

    if (flg == 1) {
        TsANIME_Init(&pfw->awork);
        pfw->isOn = 1;
        pfw->isCan = 1;
        pfw->nameMsk = 0;
        pfw->onTime = 0;
        pfw->desname = (char *)tpad;
        pfw->state = 0;
        pfw->dispType = 0;
        if (*(char *)tpad != '\0') {
            pfw->curnpos = 8;
        } else {
            pfw->curnpos = 0;
        }
        pfw->curchrmode = 0;
        return 0;
    }

    if (flg == 2) {
        pfw->isOn = 0;
        pfw->state = 0;
        pfw->nameMsk = 0;
        pfw->onTime = 0;
        TsANIME_Init(&pfw->awork);
        return 0;
    }

    if (flg == 3) {
        if (pfw->isOn && pfw->state < 0xff30) {
            TsANIME_Init(&pfw->awork);
            pfw->nameMsk = 0;
            pfw->state = 0xff30;
        }
        return 0;
    }

    aflg = TsANIME_Poll(&pfw->awork);
    state = pfw->state;

    switch (state) {
    case 0:
        state = 0x80;
        TsNAMEINBox_SetName(pfw, (u_char *)pfw->desname);
        pfw->isOn = 1;
        pfw->nameMsk = 1;
        TsANIME_Start(&pfw->awork, 2, 15);
        aflg = TsANIME_Poll(&pfw->awork);
    case 0x80:
        if (aflg != 0) {
            break;
        }
    case 0x1000:
        state = 0x4100;
    case 0x4100:
    {
        int sel;
        int osel;

        sel = osel = pfw->curnpos;
        if (tpad & SCE_PADLleft) {
            sel--;
        }
        if (tpad & SCE_PADLright) {
            sel++;
        }
        if (sel < 8 && (tpad & SCE_PADRright)) {
            sel++;
            tpad &= ~0x20;
        }
        if (osel != sel) {
            sel = TSLOOP(sel, 9);
            pfw->curnpos = sel;
            pfw->curchrmode = pfw->curnchr[sel] >> USERNAME_CHAR_SET_SHIFT;
            TSSNDPLAY(2);
        }

        if (sel < 8) {
            sel = pfw->curchrmode;
            if (tpad & 0x100) {
                sel++;
            }
            if (pfw->curchrmode != sel) {
                pfw->curchrmode = TSLOOP(sel, 2);
                pfw->curnchr[pfw->curnpos] = USERNAME_CHAR(pfw->curchrmode, pfw->curnchr[pfw->curnpos] & USERNAME_CHAR_INDEX_MASK);
                TSSNDPLAY(5);
            }

            osel = sel = pfw->curnchr[pfw->curnpos] & USERNAME_CHAR_INDEX_MASK;
            if (tpad & SCE_PADLup) {
                sel++;
            }
            if (tpad & SCE_PADLdown) {
                sel--;
            }
            if (osel != sel) {
                pfw->curnchr[pfw->curnpos] = USERNAME_CHAR(pfw->curchrmode, TSLOOP(sel, UserName_CharSet[pfw->curchrmode].len - 1));
                TSSNDPLAY(5);
            }
        }
    }

        if ((tpad & SCE_PADRright) && pfw->curnpos == 8) {
            pfw->onTime = 30;
            TsNAMEINBox_GetName(pfw, (u_char *)pfw->desname);
            state = 0xff00;
            TSSNDPLAY(6);
        }
        if (pfw->isCan && (tpad & SCE_PADRdown)) {
            state = 0xff20;
            TSSNDPLAY(9);
        }
        if (tpad & 0x80) {
            for (i = 0; i < 8; i++) {
                TsNAMEINBox_SetName(pfw, UserName_InitialStr2);
                pfw->curnpos = 0;
            }
            TSSNDPLAY(9);
        }
        break;
    case 0xff00:
        if (pfw->onTime != 0) {
            pfw->onTime--;
            break;
        }
    case 0xff10:
        pfw->nameMsk = 2;
        state = 0xff18;
        TsANIME_Start(&pfw->awork, 3, 15);
        aflg = TsANIME_Poll(&pfw->awork);
    case 0xff18:
        if (aflg == 0) {
            pfw->isOn = 0;
            return 1;
        }
        break;
    case 0xff20:
        TsNAMEINBox_Flow(3, pfw, 0);
        return -2;
    case 0xff30:
        pfw->onTime = 0;
        pfw->nameMsk = 0;
        state = 0xff38;
        TsANIME_Start(&pfw->awork, 3, 15);
        aflg = TsANIME_Poll(&pfw->awork);
    case 0xff38:
        if (aflg == 0) {
            pfw->isOn = 0;
            return -1;
        }
        break;
    }

    pfw->state = state;
    return 0;
}

/* static */ void TsNAMEINBox_Draw(SPR_PKT pk, SPR_PRM *spr, int px, int py, int isLog, NAMEINW *pfw, int side) {
    float   ofsx = spr->ofsx;
    float   ofsy = spr->ofsy;
    u_char  str[2];
    float   rt0, rt1;
    float   rt;
    PATPOS *pbox;
    PATPOS *pok;
    int     ton;
    u_int   col, curcol;
    int     x, y;
    int     i;
    u_short code;

    rt0 = (MNSceneGetMusicFitTimer() % 360) / 360.0f;
    if (side) {
        rt0 = sinf((rt0 + rt0) * 3.1415927f - 1.5707964f) * 4.0f;
    } else {
        rt0 = sinf((rt0 + rt0) * 3.1415927f) * 4.0f;
    }

    spr->ofsy += rt0;
    spr->zx = 1.0f;
    spr->zy = 0.5f;

    switch (pfw->dispType) {
    case 1:
        pbox = VS1PNameBox;
        pok  = &VS1PNameBoxOK;
        break;
    case 2:
        pbox = VS2PNameBox;
        pok  = &VS2PNameBoxOK;
        break;
    case 0:
    default:
        pbox = STGCNameBox;
        pok  = &STGCNameBoxOK;
        break;
    }

    if (TsANIME_GetRate(&pfw->awork, &rt0, &rt1, NULL)) {
        x = px + pbox->x + 0x72;
        y = py + pbox->y + 10;
        spr->zoom.centerX = x;
        spr->zoom.centerY = y;

        rt = TSNumRBack(rt1, 0.3f);
        if (pfw->awork.aflg & 1) {
            spr->zoom.zoomX = rt;
        } else {
            spr->zoom.zoomX = (1.0f - rt) * 1.2f + 1.0f;
        }

        rt = TSNumRBack(rt0, 0.15f);
        spr->zoom.zoomY = rt * rt;
        spr->zoom.isOn  = 1;
        ton = rt0 * rt0 * 256.0f;
    } else {
        spr->zoom.isOn = 0;
        ton = 0x100;
    }

    col = GetDToneColor(0x808080, MN_COLOR_NEUTRAL, ton);
    spr->rgba0 = col;
    PkALPHA_Add(pk, 0x44);

    for (i = 0; i < 5; i++) {
        TsPatPut(pk, spr, &pbox[i], px, py);
    }

    rt = sinf((MNSceneGetMusicFitTimer() % 12) / 12.0f * 3.1415927f) * 256.0f;
    if (pfw->onTime) {
        curcol = GetDToneColor(0x800a6ec8, MN_COLOR_WHITE, rt);
    } else {
        curcol = GetDToneColor(0x803ca0ff, 0x80b4ffff, rt);
    }

    x = px + pbox->x + 0x25;
    y = py + pbox->y + 9;
    if (pfw->onTime) {
        spr->px = x;
        spr->py = y;
        spr->sw = 0xb8;
        spr->sh = 0x19;
    } else {
        x += pfw->curnpos * 20;
        spr->px = x;
        spr->py = y;
        spr->sh = 0x19;
        spr->sw = (pfw->curnpos == 8) ? 0x18 : 0x15;
    }

    spr->rgba0 = GetDToneColor(curcol & 0xffffff, curcol, ton);
    PkCRect_Add(pk, spr, PKSPR_ZOOM);

    spr->rgba0 = col;
    TsPatPut(pk, spr, pok, px, py);

    x = px + pbox->x + 0x30;
    y = py + pbox->y + 9;

    switch (isLog) {
    case 0:
        col = 0x80660000;
        break;
    case 1:
        col = 0x801e4000;
        break;
    case 2:
    default:
        col = 0x80000061;
        break;
    }
    col = GetDToneColor(0x808080, col, ton);

    for (i = 0; i < 8; i++) {
        code   = pfw->curnchr[i];
        str[0] = UserName_CharSet[code >> USERNAME_CHAR_SET_SHIFT].ptbl[code & USERNAME_CHAR_INDEX_MASK];
        str[1] = 0;
        MENUFontPutL(pk, spr, x, y, col, 1, str);
        x += 20;
    }

    spr->ofsx = ofsx;
    spr->ofsy = ofsy;
}

int TsSCFADE_Set(int flg, int num, int prio) {
    SCFADE *pfw = &ScFade;
    int     state = 0;
    int     t;

    switch (flg) {
    case 1:
    case 5:
        if (pfw->state != flg) {
            TsSCFADE_Flow(1, 0);
            pfw->ton = 256;
        } else {
            state = 1;
        }
        break;
    case 2:
    case 6:
        if (pfw->state != flg) {
            TsSCFADE_Flow(1, 0);
            pfw->ton = 0;
        } else {
            state = 1;
        }
        break;
    default:
        state = 1;
        break;
    }

    if (state) {
        t = pfw->ttim0 - 1;
        t -= pfw->ttim;
        if (t < 1) {
            return 0;
        }
        return t;
    }

    if (num != 0) {
        pfw->prio = prio;
        pfw->state = flg;
        pfw->ttim0 = num;
        pfw->ttim = 0;
    }

    return num;
}

static void TsSCFADE_Flow(int flg, int prm) {
    SCFADE *pfw = &ScFade;

    if (flg == 1 || flg == 3) {
        memset(pfw, 0, sizeof(*pfw));
        if (flg == 3) {
            pfw->ton = prm;
        }
        return;
    }

    if (pfw->ttim0 != 0) {
        pfw->ttim++;
    }

    switch (pfw->state) {
    case 1:
    case 5:
        pfw->ton = 256 - ((pfw->ttim * 256) / pfw->ttim0);
        if (pfw->ton <= 0) {
            TsSCFADE_Flow(1, 0);
        }
        break;
    case 2:
    case 6:
        pfw->ton = (pfw->ttim * 256) / pfw->ttim0;
        if (pfw->ton >= 256) {
            TsSCFADE_Flow(3, 256);
        }
        break;
    }
}

/* static */ void TsSCFADE_Draw(SPR_PKT pk, SPR_PRM *spr, int prio) {
    SCFADE *pfw = &ScFade;
    u_int   abgr;

    if (pfw->ton == 0 || prio != pfw->prio) {
        return;
    }

    switch (pfw->state) {
    case 5:
    case 6:
        spr->rgba0 = MN_COLOR_NEUTRAL;
        spr->zx = spr->zy = 1.0f;
        PkALPHA_Add(pk, SCE_GS_SET_ALPHA(0, 1, 2, 1, (pfw->ton * 128) >> 8));
        PkSprPkt_SetTexVram(pk, spr, DrawGetDrawEnvP(DNUM_VRAM2));
        SetSprScreenXYWH(spr);
        PkNSprite_AddAdj(pk, spr, PKSPR_UV_RECT);
        PkALPHA_Add(pk, 0x44);
        break;
    case 1:
    case 2:
    default:
        spr->zx = spr->zy = 1.0f;
        abgr = GetDToneColor(0, 0x80000000, pfw->ton);
        spr->rgba0 = abgr;
        SetSprScreenXYWH(spr);
        PkALPHA_Add(pk, 0x44);
        PkCRect_Add(pk, spr, 0);
        break;
    }
}

void _PkMCMsgPut(SPR_PKT pk, SPR_PRM *spr, int id, int x, int y, u_int abgr) {
    int flg;

    if (pP3GameState != NULL) {
        flg = pP3GameState->pGameStatus->language_type;
    } else {
        flg = LANG_JAPANESE;
    }

    MENUSubtPut(pk, spr, x, y, abgr, 1, MenuMsgGetMessageMc(id, flg), flg);
}

int _PkMCMsgGetLine(int id) {
    int flg;

    if (pP3GameState != NULL) {
        flg = pP3GameState->pGameStatus->language_type;
    } else {
        flg = LANG_JAPANESE;
    }

    return MENUSubtGetLine(MenuMsgGetMessageMc(id, flg), flg);
}

void _PkSubMsgPut(SPR_PKT pk, SPR_PRM *spr, int id, int x, int y, u_int abgr) {
    int flg;

    if (pP3GameState != NULL) {
        flg = pP3GameState->pGameStatus->language_type;
    } else {
        flg = LANG_JAPANESE;
    }

    MENUSubtPut(pk, spr, x, y, abgr, 1, MenuMsgGetMessageSub(id, flg), flg);
}

void TsMenu_CleanVram(int nFrm) {
    SPR_PRM        SprPrm;
    u_long128     *pkt;
    SPR_PKT        pk;
    sceGsDrawEnv1 *pdenv;

    pk = &pkt;

    pdenv = DrawGetDrawEnvP(nFrm);
    if (pdenv != NULL) {
        MnLPkt.ptop = PR_UNCACHED(MnLPkt.pkt[MnLPkt.idx].PaketTop);
        pkt = (u_long128*)MnLPkt.ptop;

        PkSprPkt_SetDefault(pk, &SprPrm, pdenv);
        PkSprPkt_SetDrawEnv(pk, &SprPrm, pdenv);
        PkZBUFMask_Add(pk, FALSE);

        SprPrm.zdepth = 0;
        SprPrm.rgba0 = 0x80000000;
    
        SetSprScreenXYWH(&SprPrm);
        PkCRect_Add(pk, &SprPrm, 0);

        PkSprPkt_SetDrawEnv(pk, &SprPrm, DrawGetDrawEnvP(DNUM_DRAW));
        PkZBUFMask_Add(pk, TRUE);

        MnLPkt.ptop = (u_int)pkt;
        sceGsSyncPath(0, 0);

        TsDrawUPacket(&MnLPkt);
        sceGsSyncPath(0, 0);
    }
}

void TsMenu_CaptureVram(SPR_PKT pk, SPR_PRM *spr) {
    PkSprPkt_SetDrawEnv(pk, spr, DrawGetDrawEnvP(DNUM_VRAM2));
    PkSprPkt_SetTexVram(pk, spr, DrawGetDrawEnvP(DNUM_DRAW));

    PkTEX1_Add(pk, SCE_GS_SET_TEX1(0, 0, 0, 0, 0, 0, 0));
    PkALPHA_Add(pk, SCE_GS_SET_ALPHA(2, 2, 0, 0, 0));

    spr->zy = 1.0f;
    spr->zx = 1.0f;
    spr->zdepth = 0;
    spr->rgba0 = SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);

    SetSprScreenXYWH(spr);
    spr->ux = spr->px;
    spr->uy = spr->py;
    spr->uw = spr->sw;
    spr->uh = spr->sh;

    PkNSprite_AddAdj(pk, spr, PKSPR_UV_RECT);
    PkTEX1_Add(pk, 0x2020);

    PkSprPkt_SetDrawEnv(pk, spr, DrawGetDrawEnvP(DNUM_DRAW));
    PkSprPkt_SetTexVram(pk, spr, DrawGetDrawEnvP(DNUM_VRAM2));

    PkALPHA_Add(pk, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0));
}

void TsSetCTransSpr(SPR_PKT pk, SPR_PRM *spr, int mx, int my, float zx, float zy) {
    PKMESH *mesh;
    PKMSPT *pt;
    int     x, y;
    int     cx, cy;
    float   rw, rh;
    float   rx, ry;
    float   sx;

    mesh = PkMesh_Create(mx, my);
    PkMesh_SetXYWH(mesh, spr->px, spr->py, spr->sw * spr->zx, spr->sh * spr->zy);
    PkMesh_SetUVWH(mesh, spr->ux, spr->uy, spr->uw, spr->uh);

    cx = (mesh->mw + 1) >> 1;
    cy = (mesh->mh + 1) >> 1;
    rw = mesh->sw * 0.5f * zx;
    rh = mesh->sh * 0.5f * zy;

    for (y = 0; y < mesh->mh + 1; y++) {
        for (x = 0; x < mesh->mw + 1; x++) {
            pt = &mesh->pmspt[y * (mesh->mw + 1) + x];

            rx = (float)(cx - x) / cx;
            ry = (float)(cy - y) / cy;

            sx = cosf(rx * 1.5707964f);
            pt->ofsx += rw * (1.0f - cosf(ry * 1.5707964f)) * rx;
            pt->ofsy += rh * (1.0f - sx) * ry;
        }
    }

    PkFTMesh_Add(pk, spr, mesh);
    PkMesh_Delete(mesh);
}

void TsSetSLTransSpr(SPR_PKT pk, SPR_PRM *spr, int mx, int my, float zx) {
    PKMESH *mesh;
    int     y;
    float   rw;
    float   zy;
    float   boy;

    boy = spr->ofsy;

    zy = (1.08f - sinf((zx + 1.0f) * 3.1415927f * 0.5f) * 0.08f) * spr->zy;
    spr->ofsy += spr->sh * spr->zy - spr->sh * zy;
    rw = spr->sh * spr->zy * cosf(zx * 0.31415927f + 1.5707964f);

    mesh = PkMesh_Create(mx, my);
    PkMesh_SetXYWH(mesh, spr->px, spr->py, spr->sw * spr->zx, spr->sh * zy);
    PkMesh_SetUVWH(mesh, spr->ux, spr->uy, spr->uw, spr->uh);

    for (y = 0; y < mesh->mh + 1; y++) {
        PkMesh_SetHLinOfs(mesh, y, rw * cosf(((float)y / mesh->mh) * 1.5707964f), 0.0f);
    }

    PkFTMesh_Add(pk, spr, mesh);
    PkMesh_Delete(mesh);

    spr->ofsy = boy;
}

void TsSetPNTransSpr(SPR_PKT pk, SPR_PRM *spr, int mx, int my, float wr, float dr) {
    PKMESH *mesh;
    int     x, y;
    float   fdy;
    float   flx, frx;
    float   lx, rx;
    float   uy, dy;

    flx = wr * 6.2831855f;
    mesh = PkMesh_Create(mx, my);
    frx = flx - 3.1415927f;
    fdy = flx + 3.1415927f;
    PkMesh_SetXYWH(mesh, spr->px, spr->py, spr->sw * spr->zx, spr->sh * spr->zy);
    PkMesh_SetUVWH(mesh, spr->ux, spr->uy, spr->uw, spr->uh);

    for (y = 0; y < mesh->mh + 1; y++) {
        lx = sinf(flx + ((float)y / mesh->mh) * 3.1415927f) * 1.5f + 0.75f;
        rx = -sinf(frx - ((float)y / mesh->mh) * 4.712389f) * 1.5f + 0.75f;
        PkMesh_SetHLinOfsLRX(mesh, y, lx * dr, rx * dr);
    }

    for (x = 0; x < mesh->mw + 1; x++) {
        uy = cosf(flx + ((float)x / mesh->mw) * 3.1415927f) * 0.7f;
        dy = -cosf(fdy - ((float)x / mesh->mw) * 4.712389f) * 0.7f;
        PkMesh_SetVLinOfsUDY(mesh, x, uy * dr, dy * dr);
    }

    PkFTMesh_Add(pk, spr, mesh);
    PkMesh_Delete(mesh);
}

static void TsPatTexFnc(int flg) {
    _TexFunc = flg;
}

/* static */ void _TsPatSetPrm(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy) {
    TSTEX_INF *ptex = &tblTex[ppos->texNo];
    int        x, y, w, h;
    u_int      tw;

    switch (_TexFunc) {
    case 0:
        PkTEX0_Add(pk, ptex->tex0);
        break;
    case 1:
        PkTEX0_Add(pk, ptex->tex0 | ((u_long)2 << 35));
        break;
    case 2:
        PkTEX0_Add(pk, ptex->tex0 | ((u_long)3 << 35));
        break;
    }

    spr->ux = 0;
    spr->uy = 0;
    spr->uw = ptex->w;
    spr->uh = ptex->h;

    x = ox + ppos->x;
    y = oy + ppos->y;
    w = ppos->w;
    if (w == 0) {
        w = ptex->w;
    } else if (w == -1) {
        w = ptex->w;
        spr->uy = 0;
        spr->ux = w - 1;
        spr->uw = 2 - w;
        spr->uh = ptex->h;
    }

    h = ppos->h;
    if (h == 0) {
        h = ptex->h;
    } else if (h == -1) {
        h = ptex->h;
        spr->ux = 0;
        spr->uy = h - 1;
        tw = ptex->w;
        spr->uh = 2 - h;
        spr->uw = tw;
    }

    spr->px = x;
    spr->py = y;
    spr->sw = w;
    spr->sh = h;
}

static void TsPatPut(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy) {
    _TsPatSetPrm(pk, spr, ppos, ox, oy);
    PkNSprite_Add(pk, spr, PKSPR_UV_RECT | PKSPR_ZOOM);
}

static void TsPatGetSize(PATPOS *ppos, int *x, int *y, int *w, int *h) {
    TSTEX_INF *ptex = &tblTex[ppos->texNo];

    if (w != NULL) {
        *w = ptex->w;
    }
    if (h != NULL) {
        *h = ptex->h;
    }
    if (x != NULL) {
        *x = ppos->x + (ptex->w / 2);
    }
    if (y != NULL) {
        *y = ppos->y + (ptex->h / 2);
    }
}

/* static */ void TsPatPutRZoom(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy, float zrate, float rot) {
    float zx  = spr->zx;
    float zy  = spr->zy;
    float ofx = spr->ofsx;
    float ofy = spr->ofsy;

    spr->zx = zx * zrate;
    spr->zy = zy * zrate;
    _TsPatSetPrm(pk, spr, ppos, ox, oy);

    spr->ofsx -= spr->sw * (spr->zx - zx) * 0.5f;
    spr->ofsy -= spr->sh * (spr->zy - zy) * 0.5f;

    while (rot > 3.1415927f) {
        rot -= 6.2831855f;
    }
    while (rot < -3.1415927f) {
        rot += 6.2831855f;
    }

    spr->rot = rot;
    spr->cx = spr->sw * 0.5f;
    spr->cy = spr->sh * 0.5f;
    PkRSprite_Add(pk, spr, PKSPR_UV_RECT | PKSPR_ZOOM);

    spr->zx = zx;
    spr->zy = zy;
    spr->ofsx = ofx;
    spr->ofsy = ofy;
}

/* static */ void TsPatPutMZoom(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy, float Zrx, float Zry, int mx, int my, float Crx, float Cry) {
    float zx   = spr->zx;
    float zy   = spr->zy;
    float ofsx = spr->ofsx;
    float ofsy = spr->ofsy;

    spr->zx = zx * Zrx;
    spr->zy = zy * Zry;
    _TsPatSetPrm(pk, spr, ppos, ox, oy);

    spr->ofsx -= spr->sw * (spr->zx - zx) * 0.5f;
    spr->ofsy -= spr->sh * (spr->zy - zy) * 0.5f;
    TsSetCTransSpr(pk, spr, mx, my, Crx, Cry);

    spr->zx = zx;
    spr->zy = zy;
    spr->ofsx = ofsx;
    spr->ofsy = ofsy;
}

static void TsPatPutSwing(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy, int mx, int my, float Crx) {
    _TsPatSetPrm(pk, spr, ppos, ox, oy);
    TsSetSLTransSpr(pk, spr, mx,my, Crx);
}

static void TsPatPutUneri(SPR_PKT pk, SPR_PRM *spr, PATPOS *ppos, int ox, int oy, int mx, int my, float Crx, float Drt) {
    _TsPatSetPrm(pk, spr, ppos, ox, oy);
    TsSetPNTransSpr(pk, spr, mx, my, Crx, Drt);
}

static void TsCELBackInit(void) {
    int        i, l;
    HOSI_TYPE *type;
    int        num;

    type = hTypeTable;
    num  = 0;
    for (l = 0; l < PR_ARRAYSIZEU(hTypeTable); l++, type++) {
        num += type->num;
    }

    if (HOSIObj != NULL) {
        free(HOSIObj);
        HOSIObj = NULL;
    }

    if (num != 0) {
        HOSIObj = malloc(num * sizeof(HOSI_TYPE));
    } else {
        HOSIObj = NULL;
    }

    if (HOSIObj != NULL) {
        HOSI_OBJ *obj;
        memset(HOSIObj, 0, num * sizeof(HOSI_TYPE));

        type = hTypeTable;
        obj  = HOSIObj;

        for (l = 0; l < PR_ARRAYSIZEU(hTypeTable); l++, type++) {
            for (i = 0; i < type->num; i++, obj++) {
                obj->wtim = (rand() % (type->dispTime >> 3)) * 2;
            }
        }
    }
}

void TsCELBackEnd(void) {
    if (HOSIObj != NULL) {
        free(HOSIObj);
        HOSIObj = NULL;
    }
}

int TsCELBackDraw(TsUSERPKT *UPacket, SPR_PRM *spr, int dispSw, int colNo) {
    u_int      bkabgr;
    u_int     *colTbl;
    u_long128 *pkt;
    SPR_PKT    pk;
    int        bDrawVram;

    pk = &pkt;

    if (dispSw != 0 && (colNo >= 0 && colNo <= 2)) {
        if (HOSIObj == NULL) {
            TsCELBackInit();
            if (HOSIObj == NULL) {
                return 0;
            }
        }
    } else {
        return 0;
    }

    bDrawVram = (dispSw ^ 0x1);
    bDrawVram &= 1;

    (u_int)pkt = UPacket->ptop = PR_UNCACHED(UPacket->pkt[UPacket->idx].PaketTop);
    PkSprPkt_SetDefault(pk, spr, DrawGetDrawEnvP((bDrawVram) ? DNUM_VRAM2 : DNUM_DRAW));

    if (bDrawVram) {
        PkSprPkt_SetDrawEnv(pk, spr, DrawGetDrawEnvP(DNUM_VRAM2));
    }

    colTbl = HosiColor[colNo];
    bkabgr = colTbl[0];
    PkALPHA_Add(pk, 0x44);

    spr->zx = spr->zy = 1.0f;
    SetSprScreenXYWH(spr);
    spr->rgba0 = bkabgr;

    PkCRect_Add(pk, spr, 0);
    _TsCELBackObjDraw(pk, spr, spr->sw, spr->sh, colTbl);

    if (bDrawVram) {
        PkSprPkt_SetDrawEnv(pk, spr, DrawGetDrawEnvP(DNUM_DRAW));
    }

    UPacket->ptop = (u_int)pkt;
    sceGsSyncPath(0, 0);
    TsDrawUPacket(UPacket);
    sceGsSyncPath(0, 0);

    return 1;
}

/* static */ void _TsCELBackObjDraw(SPR_PKT pk, SPR_PRM *spr, int sw, int sh, u_int *colTbl) {
    int        i;
    int        l;
    HOSI_OBJ  *obj;
    HOSI_TYPE *type;
    TSTEX_INF *ptex;
    u_int      abgr;
    int        ton;
    int        t;
    int        t0;
    float      zf;
    float      rt;
    int        x;
    int        y;
    int        w;
    int        h;

    obj  = HOSIObj;
    type = hTypeTable;

    for (l = 0; l < PR_ARRAYSIZEU(hTypeTable); l++, type++) {
        ptex = &tblTex[type->patNo + 76];
        PkTEX0_Add(pk, ptex->tex0);

        abgr = colTbl[type->colIdx];
        zf = type->rate * 0.01f;
        spr->zy = zf;
        spr->zx = zf * 2.0f;

        for (i = 0; i < type->num; i++, obj++) {
            if (obj->wtim > 0) {
                if (--obj->wtim == 0) {
                    obj->tim = type->dispTime;
                    obj->dir = rand() & 1;

                    switch (rand() % 4) {
                    case 0:
                        w = sw;
                        h = sh >> 2;
                        x = 0;
                        y = 0;
                        break;
                    case 1:
                        w = sw >> 3;
                        h = sh;
                        x = 0;
                        y = 0;
                        break;
                    case 2:
                        w = sw >> 3;
                        h = sh;
                        y = 0;
                        x = sw - w;
                        break;
                    default:
                        h = sh >> 2;
                        x = 0;
                        w = sw;
                        y = sh - h;
                        break;
                    }

                    x += (rand() % (w / (type->patW >> 1))) * (type->patW >> 1) - 20;
                    y += (rand() % (h / (type->patW >> 2))) * (type->patW >> 2) - 10;

                    obj->px = x;
                    obj->vx = ((sw >> 1) - x) * 0.001f;
                    obj->py = y;
                    obj->vy = ((sh >> 1) - y) * 0.001f;
                }
            } else {
                if (--obj->tim <= 0) {
                    obj->wtim = (rand() % (type->dispTime >> 3)) * 3;
                } else {
                    t   = (type->dispTime * 3) >> 2;
                    t0  = type->dispTime >> 2;
                    ton = 0x100;

                    if (t < obj->tim) {
                        ton = 0x100 - (((obj->tim - t) << 8) / t0);
                    } else if (obj->tim < t0) {
                        ton = (obj->tim << 8) / t0;
                    }

                    spr->rgba0 = GetDToneColor(abgr, abgr | 0x80000000, ton);

                    zf = (float)(obj->tim % t) / t;
                    zf = cosf(zf * 6.2831855f) * 0.1 + 0.9;

                    t   = type->dispTime >> 1;
                    rt  = (float)(obj->tim % t) / t;
                    if (obj->dir) {
                        rt = -rt;
                    }

                    obj->px -= obj->vx;
                    obj->py -= obj->vy;
                    TsHosiPut(pk, spr, ptex, obj->px, obj->py, zf, rt * 6.2831855f);
                }
            }
        }
    }
}

/* static */ void TsHosiPut(SPR_PKT pk, SPR_PRM *spr, TSTEX_INF *ptex, float px, float py, float zrate, float rot) {
    float zx  = spr->zx;
    float zy  = spr->zy;
    float ofx = spr->ofsx;
    float ofy = spr->ofsy;

    spr->ux = 0;
    spr->uy = 0;
    spr->uw = ptex->w;
    spr->uh = ptex->h;

    spr->zx = zx * zrate;
    spr->zy = zy * zrate;

    spr->px = 0;
    spr->py = 0;
    spr->sw = ptex->w;
    spr->sh = ptex->h;

    spr->ofsx = ofx - spr->sw * (spr->zx - zx) * 0.5f + px;
    spr->ofsy = ofy - spr->sh * (spr->zy - zy) * 0.5f + py;

    while (rot > 3.1415927f) {
        rot -= 6.2831855f;
    }
    while (rot < -3.1415927f) {
        rot += 6.2831855f;
    }

    spr->rot = rot;
    spr->cx = spr->sw * 0.5f;
    spr->cy = spr->sh * 0.5f;
    PkRSprite_Add(pk, spr, PKSPR_UV_RECT | PKSPR_ZOOM);

    spr->zx = zx;
    spr->zy = zy;
    spr->ofsx = ofx;
    spr->ofsy = ofy;
}

