#include "menu/p3mc.h"

#include "main/cdctrl.h"

#include "menu/memc.h"
#include "menu/menu.h"
#include "menu/menudata.h"
#include "menu/menufont.h"

#include <libcdvd.h>

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *HedderID = "P2_USERDATA_HED";
static char *FooterID = "P2_USERDATA_FOT";
GETUSER_WORK *pUChkWork = NULL; /* static */
static P3MC_WORK P3MC_Work;
static char filePath[64];
static MEMC_INFO mcmenu_info;
static sceMcTblGetDir p3mcTblGetDir[8];
static u_char McLogFileFlg[80];
static u_char McReplayFileFlg[80];
static int FreeSizeFlg;
static int portCheckFlg;
static int NeedSize[2];
static int UChkSize[2];
static int isFileFlgCash;

static int      P3MC_GetIconSize(int mode);
static void*    P3MC_GetIconPtr(int mode, int stageNo);
static void    _P3MC_SetUserDirName(int mode, int fileNo);
static char*   _P3MC_GetFilePath(int mode, int fileNo);
static void    _P3MC_EUC2SJIS(char *des, char *src);
static void    _P3MC_ASC2SJIS(char *des, char *src);
static void    _P3MC_UserName_ASC2SJIS(char *des, char *src);
static void    _P3MC_SetBrowsInfo(int mode, int fileNo, char *name, int stageNo, int roundNo, int isVs, int ParaCol);
static int     _P3MC_mainfile_chk(int no, int data_csize, int mode, int *need);
static int     _P3MC_file_chk(char *name, int size, int *need);
static int     _P3MC_freesize_chk(void);
static int     _P3MC_GetSaveDataSize(int dsize);
static int     _P3MCStrCmpLen(char *str, char *id, int len);
static int     _P3MCStrNum(char *nstr, int len);
static int     _P3MC_MemcCheck(int mode, sceMcTblGetDir *pDirTable);
static void    _P3MC_AddUserBroken(P3MC_USRLST *pUser, int mode, int fno);
static int     _P3MC_loadCheck(P3MC_WORK *pw, int skip);
static int     _P3MC_SaveCheck(P3MC_WORK *pw);
static u_short _P3MC_proc(u_short prg);
static void    _P3MC_dataCheckFunc(P3MC_WORK *pw, P3MCDataCheckFunc funcp);
static int     _P3MC_CheckUserData(P3MC_WORK *pw);
static int     _P3MC_CheckUserDataHead(P3MC_WORK *pw);

/*
 * P3MC_WORK::prg is the state of the save/load sequencer. The top nibble selects the
 * operation (P3MC_OP_SAVE or P3MC_OP_LOAD), the next byte the stage of that operation
 * and the low byte a step or outcome inside the stage. _P3MC_proc turns the card
 * manager's result into the next state; _P3MC_SaveCheck and _P3MC_loadCheck turn
 * terminal states into the P3MC_RES_* values returned to the menu.
 */
#define P3MC_OP_MASK    0xf000
#define P3MC_OP_SAVE    0x0000
#define P3MC_OP_LOAD    0x1000
#define P3MC_STAGE_MASK 0xff00

enum {
    P3MC_STAGE_SAVE_CHECK  = 0x0200, /* looking at the card before writing */
    P3MC_STAGE_SAVE_WRITE  = 0x0400,
    P3MC_STAGE_SAVE_FORMAT = 0x0500,
    P3MC_STAGE_LOAD_CHECK  = 0x1200,
    P3MC_STAGE_LOAD_RETRY  = 0x1204,
    P3MC_STAGE_LOAD_READ   = 0x1400
};

enum {
    P3MC_SAVE_START             = 0x000, /* read the card info */
    P3MC_SAVE_CHECK_CARD        = 0x200,
    P3MC_SAVE_NO_CARD           = 0x201, /* no usable card */
    P3MC_SAVE_NO_SPACE          = 0x206,
    P3MC_SAVE_ERROR             = 0x207, /* any other card error */
    P3MC_SAVE_CARD_SWAPPED      = 0x210,
    P3MC_SAVE_CARD_LOST         = 0x211, /* card went away while writing */
    P3MC_SAVE_WRITE             = 0x400, /* file is being written */
    P3MC_SAVE_DONE              = 0x401,
    P3MC_SAVE_UNUSED_402        = 0x402, /* handled but never entered */
    P3MC_SAVE_CONFIRM_OVERWRITE = 0x410, /* file exists: ask unless P3MC_FLAG_OVERWRITE */
    P3MC_SAVE_OVERWRITE         = 0x411,
    P3MC_SAVE_NEED_FORMAT       = 0x510, /* card unformatted: ask unless P3MC_FLAG_FORMAT */
    P3MC_SAVE_FORMAT_ALLOWED    = 0x511,
    P3MC_SAVE_FORMATTING        = 0x520,
    P3MC_SAVE_FORMAT_FAILED     = 0x530
};

enum {
    P3MC_LOAD_START        = 0x1000, /* read the card info */
    P3MC_LOAD_FIRST_FILE   = 0x1001, /* first boot: load the initial file */
    P3MC_LOAD_FILE         = 0x1002,
    P3MC_LOAD_READ_CARD    = 0x1100,
    P3MC_LOAD_CHECK_CARD   = 0x1200,
    P3MC_LOAD_NO_CARD      = 0x1201,
    P3MC_LOAD_NO_SAVE_DATA = 0x1202, /* handled but never entered */
    P3MC_LOAD_UNFORMATTED  = 0x1203,
    P3MC_LOAD_UNUSED_1204  = 0x1204, /* stage only, never entered */
    P3MC_LOAD_ERROR        = 0x1207,
    P3MC_LOAD_NO_FILE      = 0x1209,
    P3MC_LOAD_CARD_SWAPPED = 0x1210,
    P3MC_LOAD_CARD_LOST    = 0x1211,
    P3MC_LOAD_BAD_DATA     = 0x1231,
    P3MC_LOAD_READ         = 0x1400, /* file is being read */
    P3MC_LOAD_DONE         = 0x1401
};

static int P3MC_GetIconSize(int mode) {
    int isize;

    switch (mode) {
    case P3MC_MODE_REPLAY:
        isize = 0x1e360;
        break;
    case P3MC_MODE_LOG:
    default:
        isize = 0x1ccb0;
        break;
    }

    return isize;
}

static void* P3MC_GetIconPtr(int mode, int stageNo) {
    int fn;

    if (mode == P3MC_MODE_LOG) {
        switch (stageNo) {
        case 1:
            fn = 0x161;
            break;
        case 2:
            fn = 0x162;
            break;
        case 3:
            fn = 0x163;
            break;
        default:
            fn = 0x164;
            break;
        }
    } else {
        switch (stageNo) {
        case 1:
            fn = 0x159;
            break;
        case 2:
            fn = 0x15a;
            break;
        case 3:
            fn = 0x15b;
            break;
        case 4:
            fn = 0x15c;
            break;
        case 5:
            fn = 0x15d;
            break;
        case 6:
            fn = 0x15e;
            break;
        case 7:
            fn = 0x15f;
            break;
        case 8:
        default:
            fn = 0x160;
            break;
        }
    }

    return GetIntAdrsCurrent(fn);
}

static void _P3MC_SetUserDirName(int mode, int fileNo) {
    memc_setDirName(_P3MC_GetFilePath(mode, fileNo));
}

static char* _P3MC_GetFilePath(int mode, int fileNo) {
    char *addName;

    strcpy(filePath, "BISCPS-15017"); /* D_003961C0 */
    addName = &filePath[12];

    switch (mode) {
    case P3MC_MODE_LOG:
        if (fileNo < 0) {
            strcpy(addName, "LOG???");
        } else {
            sprintf(addName, "LOG%03d", fileNo);
        }
        break;
    case P3MC_MODE_REPLAY:
        if (fileNo < 0) {
            strcpy(addName, "REP???");
        } else {
            sprintf(addName, "REP%03d", fileNo);
        }
        break;
    case P3MC_MODE_ALL:
    default:
        strcpy(addName, "??????");
        break;
    }

    return filePath;
}

static void _P3MC_EUC2SJIS(char *des, char *src) {
    u_char c1, c2;

    while ((c1 = *src++) != '\0') {
        c2 = *src++;
        if ((c1 % 2) == 0) {
            c2 -= 0x02;
        } else {
            c2 -= 0x61;
            if (c2 > 0x7e) {
                c2++;
            }
        }

        c1++;
        if (c1 < 0xe0) {
            c1 /= 2;
            c1 += 0x30;
        } else {
            c1 /= 2;
            c1 += 0x70;
        }

        *des++ = c1;
        *des++ = c2;
    }

    *des = '\0';
}

static void _P3MC_ASC2SJIS(char *des, char *src) {
    /* NUL-terminated EUC-JP full-width forms of the ASCII range starting at '!': ！”＃＄％＆’（）＊＋，−．／０１２３４５６７８９：；＜＝＞？＠ */
    static u_short sjisASCII0[33] = { 0xAAA1, 0xC9A1, 0xF4A1, 0xF0A1, 0xF3A1, 0xF5A1, 0xC7A1, 0xCAA1, 0xCBA1, 0xF6A1, 0xDCA1, 0xA4A1, 0xDDA1, 0xA5A1, 0xBFA1, 0xB0A3, 0xB1A3, 0xB2A3, 0xB3A3, 0xB4A3, 0xB5A3, 0xB6A3, 0xB7A3, 0xB8A3, 0xB9A3, 0xA7A1, 0xA8A1, 0xE3A1, 0xE1A1, 0xE4A1, 0xA9A1, 0xF7A1, 0 };
    /* NUL-terminated EUC-JP full-width forms of the ASCII range starting at '[': ［￥］＾＿’ */
    static u_short sjisASCII1[7] = { 0xCEA1, 0xEFA1, 0xCFA1, 0xB0A1, 0xB2A1, 0xC7A1, 0 };
    /* NUL-terminated EUC-JP full-width forms of the ASCII range starting at '{': ｛｜｝〜　 */
    static u_short sjisASCII2[6] = { 0xD0A1, 0xC3A1, 0xD1A1, 0xC1A1, 0xA1A1, 0 };

    char    *des0;
    u_char   c;
    int      n;

    des0 = des;

    for (; (c = *src) != '\0'; src++, des += 2) {
        if (c <= ' ') {
            n = 0xa1a1; /* Space */
        } else if (c <= '@') {
            c = (c - '!');
            n = sjisASCII0[c];
        } else if (c <= 'Z') {
            c = (c - 'A');
            n = (c << 8) + 0xc1a3 /* A */;
        } else if (c <= '`') {
            c = (c - '[');
            n = sjisASCII1[c];
        } else if (c <= 'z') {
            c = (c - 'a');
            n = (c << 8) + 0xe1a3 /* a */;
        } else if (c <= '~') {
            c = (c - '{');
            n = sjisASCII2[c];
        } else {
            n = 0xa1a1; /* Space */
        }

        /* Table values are little-endian pairs: the low byte is the EUC lead byte. */
        des[0] = (char)(n & 0xff);
        des[1] = (char)((n >> 8) & 0xff);
    }

    *des = '\0';
    _P3MC_EUC2SJIS(des0, des0);
}

static void _P3MC_UserName_ASC2SJIS(char *des, char *src) {
    MenuFont_ASC2EUC(des, src);
    _P3MC_EUC2SJIS(des, des);
}

static void _P3MC_SetBrowsInfo(int mode, int fileNo, char *name, int stageNo, int roundNo, int isVs, int ParaCol) {
    char  tname[256];
    int   s;
    char  tmps[30];
    int   r;
    int   size;
    void *ptr;
    int   iconNo;

    if (name == NULL) {
        return;
    }

    _P3MC_EUC2SJIS(tname, "ＰＡＲＡＰＰＡ２");

    if (mode == P3MC_MODE_LOG) {
        sprintf(tmps, "/SYS-%02d", fileNo + 1);
    } else {
        sprintf(tmps, "/REP-%02d", fileNo + 1);
    }

    s = strlen(tname);
    _P3MC_ASC2SJIS(tname + s, tmps);
    s = strlen(tname);

    switch (mode) {
    case P3MC_MODE_REPLAY:
        if (!isVs) {
            sprintf(tmps, "%s-ST%1d", name, stageNo);
        } else {
            sprintf(tmps, "%s-VS_ST%1d", name, stageNo);
        }
        break;

    default:
    case P3MC_MODE_LOG:
        if (roundNo == 0) {
            sprintf(tmps, "%s(ST%1d)", name, stageNo);
        } else {
            r = roundNo + 1;

            if (r > 99) {
                r = 99;
            }

            sprintf(tmps, "%s(C%2d)", name, r);
        }
        break;
    }

    _P3MC_UserName_ASC2SJIS(tname + s, tmps);

    memc_setSaveTitle(tname, s);

    if (mode == P3MC_MODE_LOG) {
        iconNo = ParaCol + 1;

        if (iconNo > 4) {
            iconNo = 4;
        }

        ptr = MenuDataGetIconSysHed(0, iconNo, &size);
    } else {
        iconNo = stageNo;
        ptr = MenuDataGetIconSysHed(1, iconNo, &size);
    }

    memc_setIconSysHed(ptr, size);

    memc_setSaveIcon(0, P3MC_GetIconPtr(mode, iconNo), P3MC_GetIconSize(mode));
    memc_setSaveIcon(1, NULL, 0);
    memc_setSaveIcon(2, NULL, 0);
}

/* _P3MC_file_chk: how one file on the card compares with what the game expects. */
enum {
    P3MC_FILE_DAMAGED = -2, /* present but the wrong size or still open */
    P3MC_FILE_MISSING = -1,
    P3MC_FILE_OK = 0
};

/* _P3MC_mainfile_chk: state of the whole save (system, icon and data files). */
enum {
    P3MC_MAIN_INCOMPLETE = -3, /* data file missing while the rest is intact */
    P3MC_MAIN_DAMAGED = -2,
    P3MC_MAIN_MISSING = -1,
    P3MC_MAIN_OK = 0
};

static int _P3MC_mainfile_chk(int no, int data_csize, int mode, int *need) {
    int   n;
    int   max;
    int   isSave;
    char *name;
    int   flg;
    int   flg1;

    flg1 = 0;
    if (need != NULL) {
        *need = 0;
    }

    if (no > 0) {
        return P3MC_MAIN_MISSING;
    }

    flg = _P3MC_file_chk(memc_getfilename(MEMC_FILE_ICON), sizeof(sceMcIconSys), need);
    if (flg == P3MC_FILE_DAMAGED) {
        flg = 0;
    }

    name = memc_getfilename(MEMC_FILE_ICON1);
    if (name != NULL) {
        flg1 = (_P3MC_file_chk(name, P3MC_GetIconSize(mode), need) != 0);
    }
    name = memc_getfilename(MEMC_FILE_ICON2);
    if (name != NULL) {
        if (_P3MC_file_chk(name, P3MC_GetIconSize(mode), need) != 0) {
            flg1 = 1;
        }
    }
    name = memc_getfilename(MEMC_FILE_ICON3);
    if (name != NULL) {
        if (_P3MC_file_chk(name, P3MC_GetIconSize(mode), need) != 0) {
            flg1 = 1;
        }
    }

    isSave = 0;
    if (flg == 0 && flg1 == 0) {
        isSave = 1;
    }

    max = no + 1;
    if (no < 0) {
        no = 0;
        max = 1;
    }

    for (n = no; n < max; n++) {
        flg = _P3MC_file_chk(memc_getfilename(n), data_csize, need);
        if (flg == P3MC_FILE_DAMAGED) {
            return P3MC_MAIN_DAMAGED;
        }
        if (flg == P3MC_FILE_MISSING && isSave) {
            return P3MC_MAIN_INCOMPLETE;
        }
        if (flg == P3MC_FILE_OK && !isSave) {
            return P3MC_MAIN_MISSING;
        }
    }

    return isSave ? P3MC_MAIN_OK : P3MC_MAIN_MISSING;
}

static int _P3MC_file_chk(char *name, int size, int *need) {
    int             i, j;
    int             flg;
    int             closeFlagSw;
    sceMcTblGetDir *pTblDir;
    int             need0;

    pTblDir = mcmenu_info.dirfile;
    flg = FALSE;

    if (name == NULL) {
        return P3MC_FILE_MISSING;
    }

    for (i = 0; pTblDir[i].EntryName[0] != '\0'; i++) {
        for (j = 0;; j++) {
            if (name[j] == '\0') {
                flg = TRUE;
                break;
            }

            if (name[j] != '?' && name[j] != pTblDir[i].EntryName[j]) {
                break;
            }
        }

        if (flg) {
            need0 = ((size + 1023) / 1024) - ((pTblDir[i].FileSizeByte + 1023) / 1024);
            if (need0 < 0) {
                need0 = 0;
            }

            if (need != NULL) {
                *need += need0;
            }

            closeFlagSw = (pTblDir[i].AttrFile >> 7) & 1;
            if (size == 0 || pTblDir[i].FileSizeByte == size) {
                if (!closeFlagSw || pTblDir[i].AttrFile & 0x80) {
                    break;
                }
            }

            return P3MC_FILE_DAMAGED;
        }
    }

    if (!flg) {
        if (need != NULL) {
            *need += (size + 1023) / 1024;
        }
        return P3MC_FILE_MISSING;
    }

    return P3MC_FILE_OK;
}

int P3MC_InitReady(void) {
    int re;

    portCheckFlg = 0;
    FreeSizeFlg = 0;

    memset(&P3MC_Work, 0, sizeof(P3MC_WORK));
    memset(&mcmenu_info, 0, sizeof(MEMC_INFO));
    memset(p3mcTblGetDir, 0, sizeof(p3mcTblGetDir));

    isFileFlgCash = 0;

    memset(McLogFileFlg, 0, sizeof(McLogFileFlg));
    memset(McReplayFileFlg, 0, sizeof(McReplayFileFlg));
    memc_init();

    mcmenu_info.dirfile = p3mcTblGetDir;
    mcmenu_info.dirfileMax = 8;
    memc_port_info(0, &mcmenu_info);

    re = memc_manager(0);
    if (re == MEMC_ERR_BUSY || re == MEMC_ERR_SWAP || re == MEMC_ERR_SWAP_UNFORMATTED) {
        P3MC_CheckChangeClear();
        return -1;
    }

    return 0;
}

int P3MC_GetSaveSize(int size, int mode) {
    int dataAsize = _P3MC_GetSaveDataSize(size);
    int icsize = P3MC_GetIconSize(mode);

    size = ((icsize + 1023) / 1024) + ((dataAsize + 1023) / 1024);
    return size + 5;
}

void P3MC_SetCheckSaveSize(int mode, int fsize, int csize) {
    int asize = _P3MC_GetSaveDataSize(csize);

    switch (mode) {
    case P3MC_MODE_LOG:
        NeedSize[0] = fsize;
        UChkSize[0] = asize;
        break;
    case P3MC_MODE_REPLAY:
        NeedSize[1] = fsize;
        UChkSize[1] = asize;
        break;
    }
}

static int _P3MC_freesize_chk(void) {
    int free;
    int flg;

    free = mcmenu_info.free;

    if (!memc_checkFormat()) {
        flg = -1;
    } else {
        flg = (free >= NeedSize[0]);
        if (free >= NeedSize[1]) {
            flg |= 2;
        }
    }

    return flg;
}

int P3MC_CheckChange(void) {
    int re, err;

    err = P3MC_RES_OK;
    re = memc_manager(1);
    if (re == MEMC_ERR_BUSY) {
        return P3MC_RES_BUSY;
    }

    switch (re) {
    case MEMC_ERR_FILE_INVALID:
        portCheckFlg = 0;
        return P3MC_RES_BUSY;
    case MEMC_ERR_INVALID:
        err = P3MC_RES_NO_CARD;
        P3MC_CheckChangeClear();
        break;
    case MEMC_ERR_SWAP:
    case MEMC_ERR_SWAP_UNFORMATTED:
        err = P3MC_RES_CARD_SWAPPED;
        break;
    case MEMC_OK:
    default:
        FreeSizeFlg = _P3MC_freesize_chk();
        break;
    }

    if (err != 0) {
        FreeSizeFlg = 0;
        portCheckFlg = 0;
        if (mcmenu_info.flag == 2) {
            return err;
        } else {
            return P3MC_RES_NO_CARD;
        }
    }

    if (portCheckFlg == 0) {
        portCheckFlg = 1;
        memc_port_check(0, &mcmenu_info.flag, &mcmenu_info.free);
        return P3MC_RES_BUSY;
    }

    portCheckFlg = 0;
    if (mcmenu_info.flag != 2) {
        FreeSizeFlg = 0;
        err = P3MC_RES_NO_CARD;
    } else {
        if (memc_getChangeState()) {
            err = P3MC_RES_CARD_SWAPPED;
        } else {
            err = P3MC_RES_OK;
        }
    }
 
    return err;
}

void P3MC_CheckChangeClear(void) {
    memc_setChangeState(0);
}

void P3MC_CheckChangeSet(void) {
    memc_setChangeState(1);
}

int P3MC_CheckIsNewSave(int mode) {
    return FreeSizeFlg & mode;
}

static int _P3MC_GetSaveDataSize(int dsize) {
    u_int dsize0 = (dsize + 0xf) >> 0x4 << 0x4;
    return sizeof(USER_HEADER) + dsize0 + sizeof(USER_FOOTER);
}

void P3MC_DeleteDataWork(MCRWDATA_HDL *phdl) {
    if (phdl == NULL) {
        return;
    }
    
    if (phdl->pMemTop != NULL) {
        free(phdl->pMemTop);
    }
    
    free(phdl);
}

MCRWDATA_HDL* P3MC_MakeDataWork(int dsize, USER_DATA *puser) {
    MCRWDATA_HDL *phdl;
    u_char       *pdata;
    int           asize, dsize0;
    u_char       *data;

    phdl = (MCRWDATA_HDL*)malloc(sizeof(MCRWDATA_HDL));
    memset(phdl, 0, sizeof(MCRWDATA_HDL));

    if (phdl != NULL) {
        dsize0 = ((dsize + 15) >> 4) << 4;
        asize = sizeof(USER_HEADER) + dsize0 + sizeof(USER_FOOTER);

        pdata = memalign(16, asize);
        memset(pdata, 0, asize);

        phdl->pMemTop = pdata;
        phdl->rwsize = asize;
        phdl->datasize = asize;
        phdl->srcsize = dsize;

        phdl->pHead = (USER_HEADER*)pdata;

        data = (u_char*)(((USER_HEADER*)pdata) + 1);
        phdl->pData = data;
        phdl->pFoot = (USER_FOOTER*)(data + dsize0);

        if (puser != NULL) {
            ((USER_HEADER*)pdata)->user = *puser;
        }

        return phdl;
    }

    P3MC_DeleteDataWork(NULL);
    return NULL;
}

static int _P3MCStrCmpLen(char *str, char *id, int len) {
    int i;

    for (i = 0; i < len; i++) {
        if (*str != *id) {
            return 1;
        }
        id++, str++;
    }

    return 0;
}

static int _P3MCStrNum(char *nstr, int len) {
    int    i;
    int    n;
    u_char c;

    n = 0;

    for (i = 0; i < len; i++, nstr++) {
        c = *nstr;
        if (c >= '0' && c <= '9') {
            n = (n * 10) + (c - '0');
        } else {
            printf("Error=%c\n", c);
        }
    }

    return n;
}

static int _P3MC_MemcCheck(int mode, sceMcTblGetDir *pDirTable) {
    int re;
    int err;
    int i;
    int fileNo;

    re = memc_manager(1);
    if (re == 0x10) {
        return -1;
    }

    err = 0;

    switch (re) {
    case 0x01:
    case 0x06:
    case 0x30:
        err = -1;
        break;
    case 0x02:
        P3MC_CheckChangeClear();
        err = 3;
        break;
    case 0x03:
        err = 2;
        break;
    case 0x00:
        FreeSizeFlg = _P3MC_freesize_chk();
        break;
    default:
        break;
    }

    if (portCheckFlg != 0 && err != 0) {
        portCheckFlg = 0;
        return err;
    }

    if (portCheckFlg == 0) {
        if (memc_port_check(0, &mcmenu_info.flag, &mcmenu_info.free) == 0) {
            portCheckFlg = 1;
        }
        FreeSizeFlg = 0;
    } else if (portCheckFlg == 1) {
        int flag = mcmenu_info.flag; /* note: variable not in STABS. */

        if (mcmenu_info.flag != 2) {
            portCheckFlg = 0;
            return 3;
        }

        if (memc_getChangeState() != 0) {
            isFileFlgCash = 0;
        }

        if (isFileFlgCash != 0) {
            portCheckFlg = 0;
            return 0;
        }

        isFileFlgCash = 0;
        memset(McLogFileFlg, 0, sizeof(McLogFileFlg));
        memset(McReplayFileFlg, 0, sizeof(McReplayFileFlg));
        memset(pDirTable, 0, sizeof(sceMcTblGetDir) * 81);

        if (memc_get_dir(0, _P3MC_GetFilePath(3, -1), pDirTable, 80) == 0) {
            portCheckFlg = flag;
        }
    } else if (portCheckFlg == 2) {
        for (i = 0; i < 80; i++) {
            char *name = &pDirTable[i].EntryName[0];
            char *type = &pDirTable[i].EntryName[12];
            char *num = &pDirTable[i].EntryName[15];

            if (name[0] == '\0') {
                break;
            }

            fileNo = _P3MCStrNum(num, 3);
            if (fileNo < 80) {
                if (_P3MCStrCmpLen(type, "LOG", 3) == 0) {
                    McLogFileFlg[fileNo] = 1;
                } else if (_P3MCStrCmpLen(type, "REP", 3) == 0) {
                    McReplayFileFlg[fileNo] = 1;
                }
            }
        }

        portCheckFlg = 0;
        return 0;
    }

    return -1;
}

int P3MC_GetUserStart(int mode, P3MC_USRLST *pUsrLst, int bFirst) {
    GETUSER_WORK *pWork;

    if (pUChkWork != NULL) {
        free(pUChkWork);
    }
    
    pWork = memalign(16, sizeof(*pWork));
    pUChkWork = pWork;

    portCheckFlg = 0;

    if (pWork != NULL) {
        pWork->bFirst = bFirst;
        pWork->curState = 0;

        if (mode & P3MC_MODE_LOG) {
            pWork->curMode = 1;
        } else if (mode & P3MC_MODE_REPLAY) {
            pWork->curMode = 2;
        } else {
            printf("P3MC_GetUser Error Mode is unknown!\n");
            pWork->curMode = 1;
        }

        pWork->curFno = 0;
        pWork->curUserMode = mode;
        pWork->pUserLst = pUsrLst;
    }

    return -1;
}

void P3MC_GetUserEnd(void) {
    if (pUChkWork != NULL) {
        free(pUChkWork);
    }

    pUChkWork = NULL;
}

int P3MC_GetUserCheck(void) {
    int           re;
    P3MC_WORK    *pw = &P3MC_Work;
    GETUSER_WORK *pcw = pUChkWork;
    P3MC_USRLST  *pUserLst;
    int           ischg = 0;
    int           i;
    int           flgl, flgr;
    int           isLoad;
    int           flg;
    int           chksize;
    int           loadPending;

    if (pcw == NULL) {
        return 1;
    }

    pUserLst = pcw->pUserLst;

    if (pcw->curState == 0) {
        re = _P3MC_MemcCheck(pcw->curUserMode, pcw->dirTable);
        if (re < 0) {
            return -2;
        }
        if (re == 3) {
            return 3;
        }

        if (memc_getChangeState()) {
            pUserLst->logPage_flg = 0;
            pUserLst->repPage_flg = 0;
            ischg = 1;
            pUserLst->nGetUser = 0;
            pUserLst->nLogGet = 0;
            pUserLst->nRepGet = 0;
            memset(pUserLst->plog_user, 0, sizeof(pUserLst->plog_user));
            memset(pUserLst->prep_user, 0, sizeof(pUserLst->prep_user));
        }

        P3MC_CheckChangeClear();
        pcw->curState = 1;
        if (ischg) {
            return -1;
        }
    }

    if (pcw->curState == 1) {
        flgl = 0;
        if (pcw->curUserMode & P3MC_MODE_LOG) {
            for (i = 0; i < 80; i++) {
                if (McLogFileFlg[i]) {
                    flgl |= 1;
                }
            }
            if (!flgl) {
                pUserLst->logPage_flg = 1;
                pUserLst->nLogGet = 0;
            }
        }

        flgr = 0;
        if (pcw->curUserMode & P3MC_MODE_REPLAY) {
            for (i = 0; i < 80; i++) {
                if (McReplayFileFlg[i]) {
                    flgr |= 1;
                }
            }
            if (!flgr) {
                pUserLst->repPage_flg = 1;
                pUserLst->nRepGet = 0;
            }
        }

        if (!(flgl | flgr)) {
            return 4;
        }

        pcw->curState = 2;
        return -2;
    }

    while (1) {
        if (pcw->curFno > 0 && pcw->curFno <= 80) {
            re = _P3MC_loadCheck(pw, 0);
            if (re < 0) {
                return -1;
            }

            if (re == 1 || re == 2 || re == 3 || re == 5) {
                if (pcw->curFno != 1) {
                    re = 1;
                }
                if (re == 2) {
                    re = 4;
                }
                return re;
            }

            if (re == 0) {
                P3MC_AddUser(pUserLst, pcw->curMode, &((USER_HEADER *)pcw->UserHeadTmp)->user);
            } else if (re == 6 || re == 4 || re == 11) {
                _P3MC_AddUserBroken(pUserLst, pcw->curMode, pcw->curFno - 1);
            }
        }

        loadPending = 0;
        while (!loadPending) {
            while (pcw->curFno < 80) {
                isLoad = 0;
                switch (pcw->curMode) {
                case P3MC_MODE_LOG:
                    isLoad = pUserLst->logPage_flg;
                    break;
                case P3MC_MODE_REPLAY:
                    isLoad = pUserLst->repPage_flg;
                    break;
                }

                if (!isLoad) {
                    flg = 0;
                    chksize = 0;
                    switch (pcw->curMode) {
                    case P3MC_MODE_LOG:
                        chksize = UChkSize[0];
                        flg = McLogFileFlg[pcw->curFno];
                        break;
                    case P3MC_MODE_REPLAY:
                        chksize = UChkSize[1];
                        flg = McReplayFileFlg[pcw->curFno];
                        break;
                    }

                    if (flg) {
                        pcw->chkData.pMemTop = pcw->UserHeadTmp;
                        pcw->chkData.rwsize = sizeof(USER_HEADER);
                        pcw->chkData.datasize = chksize;
                        P3MC_LoadUser(pcw->curMode, pcw->curFno, &pcw->chkData, 0);
                        _P3MC_dataCheckFunc(pw, _P3MC_CheckUserDataHead);
                        P3MC_Work.prg = (pcw->bFirst) ? P3MC_LOAD_FIRST_FILE : P3MC_LOAD_FILE;
                        pcw->curFno++;
                        loadPending = 1;
                        break;
                    }
                }
                pcw->curFno++;
            }

            if (loadPending) {
                break;
            }

            switch (pcw->curMode) {
            case P3MC_MODE_LOG:
                pUserLst->logPage_flg = 1;
                break;
            case P3MC_MODE_REPLAY:
                pUserLst->repPage_flg = 1;
                break;
            }

            while (1) {
                pcw->curMode <<= 1;
                if (pcw->curMode >= 3) {
                    break;
                }
                if (pcw->curMode & pcw->curUserMode) {
                    pcw->curFno = 0;
                    break;
                }
            }

            if (pcw->curFno > 0) {
                flg = 0;
                if (pUserLst->nGetUser) {
                    if (pcw->curUserMode & P3MC_MODE_LOG) {
                        flg = (pUserLst->nLogGet != 0);
                    }
                    if (pcw->curUserMode & P3MC_MODE_REPLAY) {
                        if (pUserLst->nRepGet) {
                            flg = 1;
                        }
                    }
                }
                return flg ? 0 : 4;
            }
        }
    }
}

void P3MC_AddUser(P3MC_USRLST *pUser, int mode, USER_DATA *puser) {
    USER_DATA *newUser = &pUser->getUser[pUser->nGetUser];
    *newUser = *puser;

    pUser->nGetUser++;
    if (pUser->nGetUser > 0x4f) {
        printf(" AddUserWork Over All File Count...\n");
        pUser->nGetUser = 0x4f;
    }

    switch (mode) {
    case P3MC_MODE_LOG:
        pUser->plog_user[pUser->nLogGet] = newUser;
        pUser->nLogGet++;
        if (pUser->nLogGet > 0x4f) {
            printf(" AddUserWork Over Log File Count...\n");
            pUser->nLogGet = 0x4f;
        }
        break;
    case P3MC_MODE_REPLAY:
        pUser->prep_user[pUser->nRepGet] = newUser;
        pUser->nRepGet++;
        if (pUser->nRepGet > 0x4f) {
            printf(" AddUserWork Over Replay File Count...\n");
            pUser->nRepGet = 0x4f;
        }
        break;
    }
}

static void _P3MC_AddUserBroken(P3MC_USRLST *pUser, int mode, int fno) {
    USER_DATA *newUser = &pUser->getUser[pUser->nGetUser];

    memset(newUser, 0, sizeof(USER_DATA));
    strcpy(newUser->name, "(BROKEN)");
    strcpy(newUser->name1, "(BROKEN)");
    newUser->stageNo = 0;
    newUser->fileNo = fno;
    newUser->mode = mode;
    newUser->flg = 2;

    newUser->date.year = -0xfb0 - fno;

    pUser->nGetUser++;
    if (pUser->nGetUser > 0x4f) {
        printf(" AddUserWork Over All File Count...\n");
        pUser->nGetUser = 0x4f;
    }

    switch (mode) {
    case P3MC_MODE_LOG:
        pUser->plog_user[pUser->nLogGet] = newUser;
        pUser->nLogGet++;
        if (pUser->nLogGet > 0x4f) {
            printf(" AddUserWork Over Log File Count...\n");
            pUser->nLogGet = 0x4f;
        }
        break;
    case P3MC_MODE_REPLAY:
        pUser->prep_user[pUser->nRepGet] = newUser;
        pUser->nRepGet++;
        if (pUser->nRepGet > 0x4f) {
            printf(" AddUserWork Over Replay File Count...\n");
            pUser->nRepGet = 0x4f;
        }
        break;
    }
}

int P3MC_SortUser(P3MC_USRLST *pUser, int mode, int isSave) {
    USER_DATA  *newUser;
    USER_DATA **pSort;
    int         nSort;
    int         i, l;
    int         isNew;
    int         nmuser;
    USER_DATA **pmuser;
    u_char      map[80];

    if (mode == P3MC_MODE_LOG) {
        pmuser = pUser->plog_user;
        nmuser = pUser->nLogGet;
    } else {
        pmuser = pUser->prep_user;
        nmuser = pUser->nRepGet;
    }

    isNew = P3MC_CheckIsNewSave(mode);

    if (isSave) {
        newUser = &pUser->getUser[pUser->nGetUser];
        memset(newUser, 0, sizeof(USER_DATA));

        if (isNew) {
            memset(map, 0, PR_ARRAYSIZE(map));
            for (i = 0; i < nmuser; i++, pmuser++) {
                map[(*pmuser)->fileNo] = 1;
            }

            for (i = 0; i < PR_ARRAYSIZE(map); i++) {
                if (map[i] == 0) {
                    break;
                }
            }

            if (i < PR_ARRAYSIZE(map)) {
                newUser->fileNo = i;
            }
        } else {
            newUser->fileNo = -1;
        }

        pUser->nUserMax = 1;
        pUser->pUserTbl[0] = newUser;
        pSort = &pUser->pUserTbl[1];
    } else {
        pUser->nUserMax = 0;
        pSort = &pUser->pUserTbl[0];
    }

    if (mode == P3MC_MODE_LOG) {
        nSort = pUser->nLogGet;
        memcpy(pSort, pUser->plog_user, nSort * sizeof(USER_DATA*));
    } else {
        nSort = pUser->nRepGet;
        memcpy(pSort, pUser->prep_user, nSort * sizeof(USER_DATA*));
    }

    for (i = 0; i < (nSort - 1); i++) {
        USER_DATA **pSrc = &pSort[i];

        for (l = i + 1; l < nSort; l++) {
            u_int s = P3MC_DATE_WORD(&pSrc[0]->date, 0);
            u_int d = P3MC_DATE_WORD(&pSort[l]->date, 0);

            if (s <= d) {
                if (s != d || P3MC_DATE_WORD(&pSrc[0]->date, 1) <= P3MC_DATE_WORD(&pSort[l]->date, 1)) {
                    USER_DATA *tmp = pSrc[0];
                    pSrc[0] = pSort[l]; pSort[l] = tmp;
                }
            }
        }
    }

    pUser->nUserMax += nSort;
    return pUser->nUserMax;
}

int P3MC_CheckBrokenUser(P3MC_USRLST *pUser, int mode) {
    int         nmuser;
    USER_DATA **pmuser;

    int i;
    int nBrk = 0;

    if (mode & P3MC_MODE_LOG) {
        pmuser = pUser->plog_user;
        nmuser = pUser->nLogGet;

        for (i = 0; i < nmuser; i++, pmuser++) {
            if ((*pmuser)->flg == 2) {
                nBrk++;
            }
        }
    }

    if (mode & P3MC_MODE_REPLAY) {
        pmuser = pUser->prep_user;
        nmuser = pUser->nRepGet;

        for (i = 0; i < nmuser; i++, pmuser++) {
            if ((*pmuser)->flg == 2) {
                nBrk++;
            }
        }
    }

    return nBrk;
}

void P3MC_OpeningCheckStart(void) {
    GETUSER_WORK *pWork;

    if (pUChkWork != NULL) {
        free(pUChkWork);
    }

    pWork = memalign(16, sizeof(*pWork));
    pUChkWork = pWork;
    
    P3MC_CheckChangeSet();
    portCheckFlg = 0;

    if (pWork != NULL) {
        pWork->curState = 0;
    }
}

void P3MC_OpeningCheckEnd(void) {
    if (pUChkWork != NULL) {
        free(pUChkWork);
    }

    pUChkWork = NULL;
}

int P3MC_OpeningCheck(void) {
    int           re;
    int           flg;
    int           chk;
    int           i;
    GETUSER_WORK *pcw;

    pcw = pUChkWork;

    if (pcw == NULL) {
        return 1;
    }

    if (pcw->curState == 0) {
        re = _P3MC_MemcCheck(3, pcw->dirTable);
        if (re < 0) {
            return -1;
        }
        if (re == 2 || re == 3) {
            return -re;
        }

        P3MC_CheckChangeClear();
        pcw->curState = 1;
        return -1;
    }

    if (pcw->curState == 1) {
        chk = _P3MC_freesize_chk();

        for (flg = 0, i = 0; i < 80; i++) {
            if (McReplayFileFlg[i] != 0) {
                flg++;
            }
        }

        if (flg != 0) {
            chk |= 0x2;
        }

        if (chk & 0x1) {
            pcw->curState = 0;
            return chk;
        }

        for (flg = 0, i = 0; i < 80; i++) {
            if (McLogFileFlg[i] != 0) {
                flg++;
            }
        }

        if (flg == 0 || flg >= 4) {
            if (flg != 0) {
                chk |= 0x1;
            }

            pcw->curState = 0;
            return chk;
        }

        pcw->curState = 2;
        pcw->curFno = 0;
    }

    if (pcw->curState == 2) {
        int fno, n;

        n = pcw->curFno;

        for (fno = 0; fno < 80; fno++) {
            if (McLogFileFlg[fno] != 0) {
                if (n == 0) {
                    break;
                }

                n--;
            }
        }

        if (fno >= 80) {
            pcw->curState = 0;
            return 0;
        }

        _P3MC_SetUserDirName(1, fno);

        re = memc_port_info(0, &mcmenu_info);
        if (re != 0) {
            memc_manager(1);
            return -1;
        }

        pcw->curState = 3;
    }

    if (pcw->curState == 3) {
        int isErr;

        re = memc_manager(1);

        if (re == 0x10) {
            return -1;
        }

        switch (re) {
        case 0:
            isErr = FALSE;
            break;
        case 5:
        case 17:
            isErr = TRUE;
            break;
        case 16: /* note: random case to trigger use of jumptable */
        case 48:
        default:
            pcw->curState = 0;
            return -1;
        }

        if (!isErr) {
            int need = 0;

            if (_P3MC_mainfile_chk(-1, UChkSize[0], 1, &need) == P3MC_MAIN_INCOMPLETE) {
                isErr = TRUE;
            } else {
                isErr = (mcmenu_info.free < need);
            }

            if (!isErr) {
                pcw->curState = 0;
                return 1;
            }
        }

        pcw->curFno++;
        if (pcw->curFno >= 3) {
            pcw->curState = 0;
            return 0;
        }

        pcw->curState = 2;
    }

    return -1;
}

int P3MC_LoadUser(int mode, int fileNo, MCRWDATA_HDL *pdhdl, int flg) {
    P3MC_WORK *pw = &P3MC_Work;

    memset(pdhdl->pMemTop, 0, pdhdl->rwsize);

    _P3MC_SetUserDirName(mode, fileNo);
    _P3MC_dataCheckFunc(pw, _P3MC_CheckUserData);

    pw->prg = P3MC_LOAD_START;
    pw->dstat = 0;

    pw->data_mode = mode;
    pw->data_no = fileNo;
    pw->data_stage = 0;

    pw->dhdl = pdhdl;
    pw->prgflag = flg;
    return 0;
}

int P3MC_LoadCheck(void) {
    int        re;
    P3MC_WORK *pw = &P3MC_Work;

    re = _P3MC_loadCheck(pw, 0);
    if (re < 0) {
        if (pw->dstat == 0) {
            return P3MC_RES_BUSY;
        } else {
            return P3MC_RES_ACCESSING;
        }
    }

    if (re != 0) {
        if (re == P3MC_RES_NO_FILE) {
            re = P3MC_RES_NO_SAVE_DATA;
        }

        memset(pw->dhdl->pMemTop, 0, pw->dhdl->rwsize);
    }

    return re;
}

static int _P3MC_loadCheck(P3MC_WORK *pw, int skip) {
    int ret;
    int re;

    ret = P3MC_RES_BUSY;

    switch (pw->prg) {
    case P3MC_LOAD_FIRST_FILE:
        re = memc_loadFirst(0, 0, pw->dhdl->pMemTop, pw->dhdl->rwsize);
        if (re == 0) {
            pw->prg = P3MC_LOAD_READ;
        } else {
            _P3MC_proc(pw->prg);
        }
        break;
    case P3MC_LOAD_FILE:
        re = memc_load_file(0, 0, pw->dhdl->pMemTop, pw->dhdl->rwsize);
        if (re == 0) {
            pw->prg = P3MC_LOAD_READ;
        } else {
            _P3MC_proc(pw->prg);
        }
        break;
    case P3MC_LOAD_START:
        pw->prg = P3MC_LOAD_READ_CARD;
        /* fallthrough */
    case P3MC_LOAD_READ_CARD:
        if (skip == 0) {
            re = memc_port_info(0, &mcmenu_info);
        } else {
            re = memc_port_check(0, &mcmenu_info.flag, NULL);
        }

        if (re == 0) {
            pw->prg = P3MC_LOAD_CHECK_CARD;
        } else {
            _P3MC_proc(pw->prg);
        }
        break;
    case P3MC_LOAD_CHECK_CARD:
        pw->prg = _P3MC_proc(pw->prg);
        if (pw->prg == P3MC_LOAD_READ) {
            memc_load_file(0, 0, pw->dhdl->pMemTop, pw->dhdl->rwsize);
        }
        break;
    case P3MC_LOAD_NO_CARD:
        ret = P3MC_RES_NO_CARD;
        break;
    case P3MC_LOAD_NO_SAVE_DATA:
        ret = P3MC_RES_NO_SAVE_DATA;
        break;
    case P3MC_LOAD_NO_FILE:
        ret = P3MC_RES_NO_FILE;
        break;
    case P3MC_LOAD_UNFORMATTED:
        ret = P3MC_RES_UNFORMATTED;
        break;
    case P3MC_LOAD_ERROR:
    case P3MC_LOAD_CARD_LOST:
        ret = P3MC_RES_FILE_ERROR;
        break;
    case P3MC_LOAD_CARD_SWAPPED:
        ret = P3MC_RES_CARD_SWAPPED;
        break;
    case P3MC_LOAD_BAD_DATA:
        ret = P3MC_RES_BAD_DATA;
        break;
    case P3MC_LOAD_READ:
        pw->dstat = 1;

        pw->prg = _P3MC_proc(pw->prg);
        if (pw->prg != P3MC_LOAD_DONE) {
            break;
        }

        if (pw->data_cfunc != NULL) {
            if (((P3MCDataCheckFunc)pw->data_cfunc)(pw) != 0) {
                pw->prg = P3MC_LOAD_BAD_DATA;
                ret = P3MC_RES_BAD_DATA;
                break;
            }
        }

        /* fallthrough */
    case P3MC_LOAD_DONE:
        ret = P3MC_RES_OK;
        break;
    default:
        break;
    }

    return ret;
}

void P3MC_SetUserWorkTime(USER_DATA *puser) {
    int        err;
    sceCdCLOCK clock;

    err = sceCdReadClock(&clock);
    puser->date.pad = rand() % 200;

    if (err != 0 && clock.stat == 0) {
        puser->date.second = clock.second;
        puser->date.minute = clock.minute;
        puser->date.hour   = clock.hour;

        puser->date.day    = clock.day;
        puser->date.month  = clock.month;
        puser->date.year   = clock.year + 0x2000;
    } else {
        puser->date.second = 0;
        puser->date.minute = 0;
        puser->date.hour   = 12;

        puser->date.day    = 1;
        puser->date.month  = 1;
        puser->date.year   = 0x2000;
    }
}

typedef struct {
    char id[16];
} P3MC_FILEID;

int P3MC_SaveUser(MCRWDATA_HDL *pdhdl, int flg) {
    P3MC_WORK  *pw = &P3MC_Work;
    u_char     *pData;
    u_char     *name;
    int         mode;
    int         stageNo;
    int         roundNo;
    int         fileNo;
    int         isVs;
    int         ParaCol;
    P3LOG_VAL  *pLog;

    ParaCol = 0;
    pData = pdhdl->pMemTop;

    mode = ((USER_HEADER *)pData)->user.mode;
    stageNo = ((USER_HEADER *)pData)->user.stageNo;
    roundNo = ((USER_HEADER *)pData)->user.roundNo;
    fileNo = ((USER_HEADER *)pData)->user.fileNo;
    isVs = ((USER_HEADER *)pData)->user.isVs;
    name = (mode == P3MC_MODE_LOG) ? ((USER_HEADER *)pData)->user.name : ((USER_HEADER *)pData)->user.name1;

    if (mode == P3MC_MODE_LOG) {
        pLog = pdhdl->pData;
        ParaCol = pLog->nRound;
        if (ParaCol < 0) {
            ParaCol = 0;
        }
        if (ParaCol >= 5) {
            ParaCol = 4;
        }
    }

    _P3MC_SetUserDirName(mode, fileNo);
    _P3MC_SetBrowsInfo(mode, fileNo, name, stageNo, roundNo, isVs, ParaCol);

    isFileFlgCash = 0;
    P3MC_SetUserWorkTime(&pdhdl->pHead->user);

    *(P3MC_FILEID *)pdhdl->pHead->header = *(P3MC_FILEID *)HedderID;
    *(P3MC_FILEID *)pdhdl->pHead->footer = *(P3MC_FILEID *)FooterID;
    *(P3MC_FILEID *)pdhdl->pFoot->footer = *(P3MC_FILEID *)FooterID;

    P3MC_Work.prg = 0;
    pw->data_no = fileNo;
    pw->data_mode = mode;
    pw->data_stage = (mode == P3MC_MODE_LOG) ? 0 : stageNo;
    pw->prgflag = flg;
    pw->dhdl = pdhdl;
    pw->dstat = 0;

    _P3MC_CheckUserDataHead(pw);
    return 0;
}

int P3MC_SaveCheck(void) {
    int        re;
    P3MC_WORK *pw = &P3MC_Work;

    re = _P3MC_SaveCheck(pw);
    if (re >= 0) {
        return re;
    }

    if (pw->dstat == 1) {
        return P3MC_RES_ACCESSING;
    }
    if (pw->dstat != 2) {
        return P3MC_RES_BUSY;
    }
    return P3MC_RES_FORMATTING;
}

static int _P3MC_SaveCheck(P3MC_WORK *pw) {
    int ret;
    int re;

    ret = P3MC_RES_BUSY;

    switch (pw->prg) {
    case P3MC_SAVE_START:
        re = memc_port_info(0, &mcmenu_info);
        if (re == 0) {
            pw->prg = P3MC_SAVE_CHECK_CARD;
        } else {
            _P3MC_proc(pw->prg);
        }
        break;
    case P3MC_SAVE_CHECK_CARD:
        pw->prg = _P3MC_proc(pw->prg);
        if (pw->prg == P3MC_SAVE_WRITE) {
            pw->dstat = 1;
            if (pw->dhdl->pMemTop != NULL) {
                memc_save_file(0, 0, pw->dhdl->pMemTop, pw->dhdl->rwsize, pw->prgflag & P3MC_FLAG_WRITE_SYSTEM);
            }
        }
        break;
    case P3MC_SAVE_NO_CARD:
        ret = P3MC_RES_NO_CARD;
        break;
    case P3MC_SAVE_NO_SPACE:
        ret = P3MC_RES_NO_SPACE;
        break;
    case P3MC_SAVE_ERROR:
    case P3MC_SAVE_CARD_LOST:
        ret = P3MC_RES_FILE_ERROR;
        break;
    case P3MC_SAVE_CARD_SWAPPED:
        ret = P3MC_RES_CARD_SWAPPED;
        break;
    case P3MC_SAVE_DONE:
        ret = P3MC_RES_OK;
        break;
    case P3MC_SAVE_WRITE:
        pw->prg = _P3MC_proc(pw->prg);

        if (pw->prg == P3MC_SAVE_DONE) {
            break;
        }
        if (pw->prg == P3MC_SAVE_WRITE) {
            break;
        }

        if (pw->prg == P3MC_SAVE_NO_SPACE) {
            ret = P3MC_RES_NO_SPACE;
        } else {
            ret = P3MC_RES_FILE_ERROR;
        }

        break;
    case P3MC_SAVE_UNUSED_402:
        pw->prg = _P3MC_proc(pw->prg);
        break;
    case P3MC_SAVE_CONFIRM_OVERWRITE:
        if (pw->prgflag & P3MC_FLAG_OVERWRITE) {
            memc_port_info(0, &mcmenu_info);
            pw->prg = P3MC_SAVE_OVERWRITE;
        } else {
            ret = P3MC_RES_CONFIRM_OVERWRITE;
        }
        break;
    case P3MC_SAVE_OVERWRITE:
        pw->prg = _P3MC_proc(pw->prg);
        if (pw->prg == P3MC_SAVE_WRITE) {
            pw->dstat = 1;
            if (pw->dhdl->pMemTop != NULL) {
                memc_save_file(0, 0, pw->dhdl->pMemTop, pw->dhdl->rwsize, pw->prgflag & P3MC_FLAG_WRITE_SYSTEM);
            }
        }
        break;
    case P3MC_SAVE_NEED_FORMAT:
        if (pw->prgflag & P3MC_FLAG_FORMAT) {
            memc_port_info(0, &mcmenu_info);
            pw->prg = P3MC_SAVE_FORMAT_ALLOWED;
        } else {
            ret = P3MC_RES_NEED_FORMAT;
        }
        break;
    case P3MC_SAVE_FORMAT_ALLOWED:
        pw->prg = _P3MC_proc(pw->prg);
        break;
    case P3MC_SAVE_FORMATTING:
        pw->dstat = 2;
        pw->prg = _P3MC_proc(pw->prg);
        break;
    case P3MC_SAVE_FORMAT_FAILED:
        ret = P3MC_RES_FORMAT_FAILED;
        break;
    default:
        break;
    }

    return ret;
}

static u_short _P3MC_proc(u_short prg) {
    u_short    re;
    P3MC_WORK *pw = &P3MC_Work;
    int        need;

    re = memc_manager(1);

    switch (re) {
    case MEMC_ERR_SWAP:
    case MEMC_ERR_SWAP_UNFORMATTED:
        switch (prg & P3MC_STAGE_MASK) {
        case P3MC_STAGE_SAVE_CHECK:
        case P3MC_STAGE_LOAD_CHECK:
            if (pw->prgflag & P3MC_FLAG_RETRY_SWAP) {
                memc_port_info(0, &mcmenu_info);
                re = prg;
                break;
            }
            /* fallthrough */
        default:
            if (prg & P3MC_OP_MASK) {
                re = P3MC_LOAD_CARD_SWAPPED;
            } else {
                re = P3MC_SAVE_CARD_SWAPPED;
            }
            break;
        }
        break;
    case MEMC_OK:
        switch (prg & P3MC_STAGE_MASK) {
        case P3MC_STAGE_SAVE_CHECK:
            if (mcmenu_info.flag & MCMC_FLAG_PS2) {
                if (!_P3MC_mainfile_chk(-1, pw->dhdl->datasize, pw->data_mode, &need)) {
                    re = P3MC_SAVE_CONFIRM_OVERWRITE;
                    break;
                }
                if (mcmenu_info.free < need) {
                    re = P3MC_SAVE_NO_SPACE;
                } else {
                    re = P3MC_SAVE_WRITE;
                }
                break;
            }
            re = P3MC_SAVE_NO_CARD;
            break;
        case P3MC_STAGE_SAVE_WRITE:
            if (prg == P3MC_SAVE_OVERWRITE) {
                re = P3MC_SAVE_WRITE;
            } else {
                re = P3MC_SAVE_DONE;
            }
            break;
        case P3MC_STAGE_SAVE_FORMAT:
            memc_port_info(0, &mcmenu_info);
            re = P3MC_SAVE_CHECK_CARD;
            break;
        case P3MC_STAGE_LOAD_CHECK:
            if (mcmenu_info.flag & MCMC_FLAG_PS2) {
                /* note: variable not in STABS info. */
                int chk = _P3MC_mainfile_chk(-1, pw->dhdl->datasize, pw->data_mode, NULL);
                if (chk >= P3MC_MAIN_MISSING && chk <= P3MC_MAIN_OK) {
                    re = P3MC_LOAD_READ;
                } else {
                    re = P3MC_LOAD_BAD_DATA;
                }
                break;
            }
            re = P3MC_LOAD_NO_CARD;
            break;
        case P3MC_STAGE_LOAD_RETRY:
            re = P3MC_LOAD_READ_CARD;
            break;
        case P3MC_STAGE_LOAD_READ:
            re = P3MC_LOAD_DONE;
            break;
        default:
            re = prg & P3MC_OP_MASK;
            break;
        }
        break;
    case MEMC_ERR_BUSY:
        re = prg;
        break;
    case MEMC_ERR_FILE_EXISTS:
        re = P3MC_SAVE_WRITE;
        memc_save_overwrite();
        break;
    case MEMC_ERR_INVALID:
        switch (prg & P3MC_OP_MASK) {
        case P3MC_OP_SAVE:
            if (prg == P3MC_SAVE_WRITE) {
                re = P3MC_SAVE_CARD_LOST;
            } else {
                re = P3MC_SAVE_NO_CARD;
            }
            break;
        case P3MC_OP_LOAD:
            if ((prg & P3MC_STAGE_MASK) == P3MC_STAGE_LOAD_READ) {
                re = P3MC_LOAD_CARD_LOST;
            } else {
                re = P3MC_LOAD_NO_CARD;
            }
            break;
        }
        break;
    case MEMC_ERR_FILE_NOT_FOUND:
    case MEMC_ERR_DIR_NOT_FOUND:
        switch (prg & P3MC_OP_MASK) {
        case P3MC_OP_SAVE:
            if (_P3MC_freesize_chk() & pw->data_mode) {
                re = P3MC_SAVE_WRITE;
            } else {
                re = P3MC_SAVE_NO_SPACE;
            }
            break;
        case P3MC_OP_LOAD:
            re = P3MC_LOAD_NO_FILE;
            break;
        }
        break;
    case MEMC_ERR_FULL:
        if ((prg & P3MC_OP_MASK) == P3MC_OP_SAVE) {
            re = P3MC_SAVE_NO_SPACE;
        } else {
            re = P3MC_LOAD_BAD_DATA;
        }
        break;
    case MEMC_ERR_UNFORMATTED:
        switch (prg & P3MC_OP_MASK) {
        case P3MC_OP_SAVE:
            if ((prg & P3MC_STAGE_MASK) == P3MC_STAGE_SAVE_CHECK) {
                re = P3MC_SAVE_NEED_FORMAT;
            } else if (prg == P3MC_SAVE_FORMAT_ALLOWED) {
                re = P3MC_SAVE_FORMATTING;
                memc_format(0);
            } else {
                re = P3MC_SAVE_CARD_SWAPPED;
            }
            break;
        case P3MC_OP_LOAD:
            re = P3MC_LOAD_UNFORMATTED;
            break;
        }
        break;
    default:
        if ((prg & P3MC_STAGE_MASK) == P3MC_STAGE_SAVE_FORMAT) {
            re = P3MC_SAVE_FORMAT_FAILED;
        } else if (prg & P3MC_OP_LOAD) {
            re = P3MC_LOAD_ERROR;
        } else {
            re = P3MC_SAVE_ERROR;
        }
        break;
    }

    return re;
}

void _P3MC_dataCheckFunc(P3MC_WORK *pw, P3MCDataCheckFunc funcp) {
    pw->data_cfunc = funcp;
}

static int _P3MC_CheckUserData(P3MC_WORK *pw) {
    USER_FOOTER *pfoot = pw->dhdl->pFoot;

    if (_P3MC_CheckUserDataHead(pw)) {
        return 1;
    } else {
        return (strcmp(FooterID, pfoot->footer) != 0);
    }
}

/* There are no traces of such use of inlines on the
 * symbols, yet the function only matches this way. */
static inline int _P3MC_CheckHead(USER_HEADER *hed, P3MC_WORK *pw) {
    /* Checks done on a single line according to symbols */
    if (hed->user.fileNo != pw->data_no ||
        hed->user.mode != pw->data_mode || 
        hed->user.mode != pw->data_mode ||
        (pw->data_stage > 0 && hed->user.stageNo != pw->data_stage) ||
        (hed->user.name[0] == 0 && hed->user.name1[0] == 0)) {
        return 1;
    }
    return 0;
}

/* Without use of inlines you can have a decent
 * match with a minor difference on a likely branch:
 * https://decomp.me/scratch/QD9p8 */
static int _P3MC_CheckUserDataHead(P3MC_WORK *pw) {
    USER_HEADER *hed = (USER_HEADER*)pw->dhdl->pMemTop;

    if (strcmp(hed->header, HedderID) != 0 || strcmp(hed->footer, FooterID) != 0) {
        return 1;
    }

    if (hed->user.flg == 0) {
        return 0;
    }

    return _P3MC_CheckHead(hed, pw);
}
