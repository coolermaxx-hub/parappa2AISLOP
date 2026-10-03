#include "menu/memc.h"
#include "libmc.h"

#include <sifdev.h>

#include <stdio.h>
#include <string.h>

/* Default icon.sys contents; the title, file names and line-feed offset are filled in at save time. */
static sceMcIconSys memc_iconsys = {
    {'P', 'S', '2', 'D'}, /* Head */
    0,                    /* Reserv1 */
    0,                    /* OffsLF */
    0,                    /* Reserv2 */
    0,                    /* TransRate */
    {                     /* BgColor (corners) */
        {0x80, 0, 0, 0},
        {0, 0x80, 0, 0},
        {0, 0, 0x80, 0},
        {0x80, 0x80, 0x80, 0},
    },
    {                     /* LightDir */
        {0.5f, 0.5f, 0.5f, 0.0f},
        {0.0f, -0.4f, -0.1f, 0.0f},
        {-0.5f, -0.5f, 0.5f, 0.0f},
    },
    {                     /* LightColor */
        {0.38f, 0.4f, 0.45f, 0.0f},
        {0.1f, 0.1f, 0.1f, 0.0f},
        {0.11f, 0.11f, 0.1f, 0.0f},
    },
    {0.5f, 0.5f, 0.5f, 0.0f}, /* Ambient */
};
/* MEMC_INFO.flag bit for each card type value stored in pmw->type. */
static u_int _memc_type[] = { 0x000, 0x100, 0x200, 0x400 };
static MEMC_STAT memc_stat;

static int memc_SaveFileClust(void);
static void memc_clearMEMCINFO(MEMC_INFO *info);
static int memc_mansub_ErrChk(int result);
static int memc_mansub_Open(char *name, u_int type);
static int memc_mansub_Close(void);
static int memcsub_fileChk(sceMcTblGetDir *dir, unsigned char *name, int max);
static int memc_mansub_GetInfo(int result);
static int memc_mansub_load(int result);
static int memc_manager_save(int result);
static int memc_manager_overwrite(int result);
static int memc_manager_chk(int mode);

void memc_init(void) {
    sceMcInit();
    memset(&memc_stat, 0, sizeof(memc_stat));
    memset(memc_iconsys.Reserve3, 0, sizeof(memc_iconsys.Reserve3));
}

void memc_setDirName(char *name) {
    memcpy(memc_stat.saveDir, name, 63);
    memc_stat.saveDir[63] = '\0';
}

void memc_setSaveTitle(char *name, int nLFPos) {
    int length = strlen(name);
    if (length > 64) {
        length = 64;
    }

    memcpy(memc_iconsys.TitleName, name, 64);
    memc_iconsys.TitleName[length + 1] = '\0';
    memc_iconsys.OffsLF = nLFPos;
}

void memc_setIconSysHed(void *pIhData, int IhSize) {
    int nLF;

    if (!pIhData || IhSize <= 0) {
        return;
    }

    nLF = memc_iconsys.OffsLF;
    memcpy(&memc_iconsys, pIhData, IhSize);
    memc_iconsys.OffsLF = nLF;
}

void memc_setSaveIcon(int no, void *pIconData, int nIconSize) {
    MEMC_STAT *pmw = &memc_stat;

    switch (no) {
    case MEMC_ICON_VIEW:
        pmw->iconData1 = pIconData;
        pmw->iconSize1 = nIconSize;

        strcpy(memc_iconsys.FnameView, "icon1.ico");
        break;
    case MEMC_ICON_COPY:
        if (pIconData == NULL || nIconSize == 0) {
            pmw->iconData2 = 0;
            pmw->iconSize2 = 0;

            strcpy(memc_iconsys.FnameCopy, "icon1.ico");
        } else {
            pmw->iconData2 = pIconData;
            pmw->iconSize2 = nIconSize;

            strcpy(memc_iconsys.FnameCopy, "icon2.ico");
        }
        break;
    case MEMC_ICON_DEL:
        if (pIconData == NULL || nIconSize == 0) {
            pmw->iconData3 = 0;
            pmw->iconSize3 = 0;

            strcpy(memc_iconsys.FnameDel, "icon1.ico");
        } else {
            pmw->iconData3 = pIconData;
            pmw->iconSize3 = nIconSize;

            strcpy(memc_iconsys.FnameDel, "icon3.ico");
        }
        break;
    default:
        break;
    }
}

char* memc_getfilename(int no) {
    char       *fbody;
    static char tmps0[64];

    switch (no) {
    case MEMC_FILE_ICON:
        fbody = "icon.sys";
        break;
    case MEMC_FILE_ICON1:
        fbody = "icon1.ico";
        break;
    case MEMC_FILE_ICON2:
        if (memc_stat.iconData2 == NULL) {
            return NULL;
        }
        /* fallthrough */
    case MEMC_FILE_ICON3:
        if (memc_stat.iconData3 == NULL) {
            return NULL;
        }
        /* fallthrough */
    default:
        if (no >= MEMC_FILE_MAX) {
            return NULL;
        }

        if (no == 0) {
            fbody = memc_stat.saveDir;
        } else {
            sprintf(tmps0, "SAVE%03d", no);
            fbody = tmps0;
        }

        break;
    }

    return fbody;
}

char* memc_getfilepath(int no) {
    static char tmps1[130];
    char       *fbody;

    fbody = memc_getfilename(no);
    if (fbody == NULL) {
        return NULL;
    } else {
        strcpy(tmps1, memc_stat.saveDir);
        strcat(tmps1, "/");
        strcat(tmps1, fbody);
        fbody = tmps1;
    }

    return fbody;
}

int memc_checkFormat(void) {
    return memc_stat.format;
}

int memc_getChangeState(void) {
    return memc_stat.isChange;
}

void memc_setChangeState(int flg) {
    memc_stat.isChange = flg;
}

static int memc_SaveFileClust(void) {
    MEMC_STAT *pmw = &memc_stat;
    int        size;

    if (pmw->seek != 0) {
        size = pmw->seek + pmw->size2;
        return (size + 1023) / 1024;
    } else {
        size = pmw->size;
        return (size + 1023) / 1024;
    }
}

static void memc_clearMEMCINFO(MEMC_INFO *info) {
    info->flag = 0;
    info->free = 0;
    info->allfile = 0;
    info->savefile = 0;

    if (info->dirfile != NULL) {
        if (info->dirfileMax < 1) {
            return;
        }

        memset(info->dirfile, 0, info->dirfileMax * sizeof(sceMcTblGetDir));
    }

    info->savefile = 0;
}

sceMcTblGetDir* memc_searchDirTbl(char *name, sceMcTblGetDir *dirTbl, int num, int isClose, int cmpSize, int *status) {
    int i, j;
    int flg;

    if (name == NULL) {
        return NULL;
    }

    for (i = 0; i < num && (flg = FALSE, dirTbl[i].EntryName[0] != '\0'); i++) {
        for (j = 0;; j++) {
            if (name[j] == '\0') {
                flg = TRUE;
                break;
            }
            if (name[j] != '?' && name[j] != dirTbl[i].EntryName[j]) {
                break;
            }
        }

        if (flg) {
            if (cmpSize != 0 && dirTbl[i].FileSizeByte != cmpSize) {
                if (status != NULL) {
                    *status = 2;
                }
                return NULL;
            }

            if (status != NULL) {
                *status = 0;
            }

            return &dirTbl[i];
        }
    }

    if (status != NULL) {
        *status = 1;
    }

    return NULL;
}

int memc_port_info(int port, MEMC_INFO *info) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;
 
    pmw->port = port;
    pmw->buf = (char*)info;

    pmw->cmd = sceMcFuncNoFileInfo;
    pmw->retry = 0;

    strcpy(pmw->filename, pmw->saveDir);
    strcat(pmw->filename, "/*");

    pmw->bChkSys = TRUE;
    pmw->filename[63] = '\0';

    memc_clearMEMCINFO(info);

    re = sceMcGetInfo(pmw->port, pmw->slot, &pmw->type, &pmw->free, &pmw->format);
    if (re == sceMcResSucceed) {
        pmw->func = MEMC_FUNC_GETINFO;
    }

    return re;
}

int memc_del_file(int port, int no) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;
    char      *tmpp;

    pmw->cmd = sceMcFuncNoDelete;
    pmw->retry = 0;

    pmw->port = port;
    pmw->fileNo = no;

    tmpp = memc_getfilepath(no);
    if (tmpp != NULL) {
        strcpy(pmw->filename, tmpp);
    } else {
        pmw->filename[0] = '\0';
    }

    re = sceMcDelete(pmw->port, pmw->slot, pmw->filename);
    if (re == sceMcResSucceed) {
        pmw->func = MEMC_FUNC_DELFILE;
    }

    return re;
}

int memc_load_file(int port, int no, char *buf, int size) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;
    char      *tmpp;

    pmw->port        = port;
    pmw->buf         = buf;
    pmw->size        = size;

    pmw->retry       = 0;
    pmw->isSyncClose = FALSE;
    pmw->fileNo      = no;

    tmpp = memc_getfilepath(no);
    if (tmpp != NULL) {
        strcpy(pmw->filename, tmpp);
    } else {
        pmw->filename[0] = '\0';
    }

    re = sceMcGetInfo(pmw->port, pmw->slot, &pmw->type, &pmw->free, &pmw->format);
    if (re == sceMcResSucceed) {
        pmw->cmd = sceMcFuncNoFileInfo;
        pmw->func = MEMC_FUNC_LOADFILE;
    }

    pmw->retry = 0;
    return re;
}

int memc_loadFirst(int port, int no, char *buf, int size) {
    MEMC_STAT *pmw = &memc_stat;
    char      *tmpp;

    pmw->port        = port;
    pmw->buf         = buf;
    pmw->size        = size;

    pmw->retry       = 0;
    pmw->isSyncClose = TRUE;
    pmw->fileNo      = no;

    tmpp = memc_getfilepath(no);
    if (tmpp != NULL) {
        strcpy(pmw->filename, tmpp);
    } else {
        pmw->filename[0] = '\0';
    }

    if (!memc_mansub_Open(pmw->filename, SCE_RDONLY)) {
        pmw->func = MEMC_FUNC_LOADFILE;
        return 0;
    }

    return 1;
}

int memc_save_file(int port, int no, char* buf, int size, int bSysRW) {
    int        re;
    int        n;
    int        isize;

    MEMC_STAT *pmw;
    char      *tmpp;

    pmw = &memc_stat;

    pmw->port    = port;
    pmw->buf     = buf;
    pmw->size    = size;
    pmw->retry   = 0;
    pmw->stat    &= ~0x1f; /* Clear all flags */

    pmw->seek    = 0;
    pmw->size2   = 0;
    pmw->bChkSys = bSysRW;
    pmw->fileNo  = no;

    tmpp = memc_getfilepath(no);
    if (tmpp != NULL) {
        strcpy(pmw->filename, tmpp);
    } else {
        pmw->filename[0] = '\0';
    }

    n = 3;
    isize = (pmw->iconSize1 + 1023) / 1024;

    if (memc_getfilename(MEMC_FILE_ICON2) != NULL) {
        isize += (pmw->iconSize2 + 1023) / 1024;
        n++;
    }
    if (memc_getfilename(MEMC_FILE_ICON3) != NULL) {
        isize += (pmw->iconSize3 + 1023) / 1024;
        n++;
    }

    pmw->sysFileSize = isize + ((n + 1) / 2) + 3;

    re = sceMcGetInfo(pmw->port, pmw->slot, &pmw->type, &pmw->free, &pmw->format);
    if (re == sceMcResSucceed) {
        pmw->cmd = sceMcFuncNoFileInfo;
        pmw->func = MEMC_FUNC_SAVEFILE;
    }

    return re;
}

int memc_seeksave_file(int port, int no, char *buf, int size, int seek, int sizef, int bSysRW) {
    MEMC_STAT *pmw = &memc_stat;
    int        re;

    re = memc_save_file(port, no, buf, size, bSysRW);

    pmw->seek = seek;
    pmw->size2 = sizef;
    return re;
}

int memc_save_overwrite(void) {
    int        re;
    int        n;
    int        isize;
    MEMC_STAT *pmw = &memc_stat;

    n = 2;
    isize = (pmw->iconSize1 + 1023) / 1024;

    if (memc_getfilename(MEMC_FILE_ICON2) != NULL) {
        isize += (pmw->iconSize2 + 1023) / 1024;
        n++;
    }
    if (memc_getfilename(MEMC_FILE_ICON3) != NULL) {
        isize += (pmw->iconSize3 + 1023) / 1024;
        n++;
    }

    pmw->sysFileSize = isize + ((n + 1) / 2) + 3;

    re = sceMcGetInfo(pmw->port, pmw->slot, &pmw->type, &pmw->free, &pmw->format);
    if (re == sceMcResSucceed) {
        pmw->cmd = sceMcFuncNoFileInfo;
        pmw->func = MEMC_FUNC_OVERWRITE;
    }

    return re;
}

int memc_port_check(int port, int *type, int *free) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;
  
    re = sceMcGetInfo(port, 0, type, free, &pmw->format);
    if (re == sceMcResSucceed) {
        pmw->cmd = sceMcFuncNoFileInfo;
        pmw->func = MEMC_FUNC_PORTCHECK;
    }

    pmw->retry = 0;
    return re;
}

int memc_format(int port) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;

    pmw->port = port;

    re = sceMcGetInfo(port, pmw->slot, &pmw->type, &pmw->free, &pmw->format);
    if (re == sceMcResSucceed) {
        pmw->cmd = sceMcFuncNoFileInfo;
        pmw->func = MEMC_FUNC_FORMAT;
    }

    pmw->retry = 0;
    return re;
}

int memc_chg_dir(int port, char *name) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;

    pmw->retry = 0;
    strcpy(pmw->filename, name);

    re = sceMcChdir(port, 0, name, NULL);
    if (re == sceMcResSucceed) {
        pmw->func = MEMC_FUNC_CHGDIR;
        pmw->cmd = sceMcFuncNoChDir;
    }

    return re;
}

int memc_get_dir(int port, char *name, sceMcTblGetDir *dir, int max) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;

    memset(dir, 0, max * sizeof(sceMcTblGetDir));

    pmw->port = port;

    strcpy(pmw->filename, name);

    pmw->size = max;
    pmw->buf = (char*)dir;

    re = sceMcGetDir(pmw->port, pmw->slot, pmw->filename, 0, max, dir);
    if (re == sceMcResSucceed) {
        pmw->func = MEMC_FUNC_GETDIR;
        pmw->cmd = sceMcFuncNoGetDir;
        pmw->retry = 0;
    }

    return re;
}

int memc_get_dir_continue(sceMcTblGetDir *dir, int max) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;

    memset(dir, 0, max * sizeof(sceMcTblGetDir));

    pmw->size = max;
    pmw->buf = (char*)dir;

    re = sceMcGetDir(pmw->port, pmw->slot, pmw->filename, 1, max, dir);
    if (re == sceMcResSucceed) {
        pmw->func = MEMC_FUNC_GETDIR;
        pmw->cmd = sceMcFuncNoGetDir;
        pmw->retry = 0;
    }

    return re;
}

static int memc_mansub_ErrChk(int result) {
    MEMC_STAT *pmw = &memc_stat;

    if (pmw->func == MEMC_FUNC_GETDIR && result >= pmw->size) {
        return MEMC_ERR_DIR_TOO_MANY;
    }

    if (result >= 0) {
        pmw->func = MEMC_FUNC_IDLE;
        return MEMC_OK;
    }

    switch (result) {
    case sceMcResChangedCard:
        pmw->func = MEMC_FUNC_IDLE;
        pmw->isChange = TRUE;
        return MEMC_ERR_SWAP;
    case sceMcResNoFormat:
        pmw->func = MEMC_FUNC_IDLE;
        return MEMC_ERR_UNFORMATTED;
    /* Switched to unformatted MC (sceMcGetInfo) */
    case -2000: /* sceMcResNoFormat */
        pmw->func = MEMC_FUNC_IDLE;
        pmw->isChange = TRUE;
        return MEMC_ERR_SWAP_UNFORMATTED;
    case sceMcResFullDevice:
        pmw->func = MEMC_FUNC_IDLE;
        return MEMC_ERR_FULL;
    case sceMcResNoEntry:
        pmw->func = MEMC_FUNC_IDLE;
        switch (pmw->cmd) {
        case sceMcFuncNoMkdir:
            return MEMC_OK; /* Dir. already exists */
        case sceMcFuncNoGetDir:
            return MEMC_ERR_DIR_NOT_FOUND;
        case sceMcFuncNoOpen:
        case sceMcFuncNoDelete:
            return MEMC_ERR_FILE_NOT_FOUND;
        }
        return MEMC_ERR_FILE_INVALID;
    default:
        pmw->func = MEMC_FUNC_IDLE;
        return MEMC_ERR_INVALID;
    }
}

static int memc_mansub_Open(char *name, u_int type) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;

    re = sceMcOpen(pmw->port, pmw->slot, name, type);
    if (re == sceMcResSucceed) {
        pmw->cmd = sceMcFuncNoOpen;
    }

    return re;
}

static int memc_mansub_Close(void) {
    int        re;
    MEMC_STAT *pmw = &memc_stat;

    re = sceMcClose(pmw->fd);
    if (re == sceMcResSucceed) {
        pmw->cmd = sceMcFuncNoClose;
    }

    return re;
}

static int memcsub_fileChk(sceMcTblGetDir *dir, unsigned char *name, int max) {
    int i, j, s;

    if (name == NULL) {
        return -1;
    }

    s = 0;

    for (i = 0; i < max; i++) {
        for (j = 0; j < 64; j++) {
            if (name[j] == '\0') {
                s++;
                break;
            }
            if (dir[i].EntryName[j] != name[j]) {
                break;
            }
        }
    }

    return s;
}

static int memc_mansub_GetInfo(int result) {
    MEMC_INFO *info;
    MEMC_STAT *pmw = &memc_stat;
    int        re;

    switch (pmw->cmd) {
    case sceMcFuncNoFileInfo:
        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        } else {
            if (result < 0) {
                pmw->func = MEMC_FUNC_IDLE;

                if (result != sceMcResNoFormat) {
                    re = result;
                } else {
                    re = -2000;
                }

                return memc_mansub_ErrChk(re);
            } else {
                if (!pmw->format) {
                    pmw->func = MEMC_FUNC_IDLE;
                    return MEMC_ERR_UNFORMATTED;
                } else {
                    info = (MEMC_INFO*)pmw->buf;

                    info->free = pmw->free;
                    info->flag |= _memc_type[pmw->type];

                    if (sceMcGetDir(pmw->port, pmw->slot, pmw->filename, 0, info->dirfileMax, info->dirfile) == sceMcResSucceed) {
                        pmw->cmd = sceMcFuncNoGetDir;
                        return MEMC_ERR_BUSY;
                    }

                    return MEMC_ERR_FILE_INVALID;
                }
            }
        }
        break;
    case sceMcFuncNoGetDir:
        if (result < 0) {
            pmw->func = MEMC_FUNC_IDLE;
            return memc_mansub_ErrChk(result);
        } else {
            info = (MEMC_INFO*)pmw->buf;

            info->allfile = result;
            info->flag |= MEMC_FLAG_DIR;

            if (pmw->bChkSys) {
                if (memcsub_fileChk(info->dirfile, memc_getfilename(MEMC_FILE_ICON), result)) {
                    info->flag |= MEMC_FLAG_ICON;
                }
                if (memcsub_fileChk(info->dirfile, memc_getfilename(MEMC_FILE_ICON1), result)) {
                    info->flag |= MEMC_FLAG_ICON1;
                }
                if (memcsub_fileChk(info->dirfile, memc_getfilename(MEMC_FILE_ICON2), result)) {
                    info->flag |= MEMC_FLAG_ICON2;
                }
                if (memcsub_fileChk(info->dirfile, memc_getfilename(MEMC_FILE_ICON3), result)) {
                    info->flag |= MEMC_FLAG_ICON3;
                }

                info->savefile  = memcsub_fileChk(info->dirfile, pmw->saveDir, result);
                info->savefile += memcsub_fileChk(info->dirfile, "SAVE", result);
            }

            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_OK;
        }
        break;
    default:
        return MEMC_OK;
    }
}

static int memc_mansub_load(int result) {
    MEMC_STAT *pmw = &memc_stat;

    switch (pmw->cmd) {
    case sceMcFuncNoFileInfo:
        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        }

        if (!memc_mansub_Open(pmw->filename, SCE_RDONLY)) {
            pmw->func = MEMC_FUNC_LOADFILE;
            break;
        }

        return MEMC_ERR_FILE_INVALID;
    case sceMcFuncNoOpen:
        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        pmw->fd = result;
        if (!sceMcRead(pmw->fd, pmw->buf, pmw->size)) {
            pmw->cmd = sceMcFuncNoRead;
            return MEMC_ERR_BUSY;
        }

        return MEMC_ERR_BUSY;
    case sceMcFuncNoRead:
        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (memc_mansub_Close()) {
            return MEMC_ERR_FILE_INVALID;
        }

        pmw->func = MEMC_FUNC_SKIP;
        if (!pmw->isSyncClose) {
            return MEMC_ERR_BUSY;
        }

        pmw->isSyncClose = FALSE;
        if (sceMcSync(0, NULL, &result) != sceMcExecFinish) {
            return MEMC_ERR_BUSY;
        }

        return memc_mansub_ErrChk(result);
    }

    return MEMC_ERR_BUSY;
}

static int memc_manager_save(int result) {
    int             re;
    MEMC_STAT      *pmw;
    char            name[64];
    char           *fname;
    int             i;
    int             iscls;
    int             isfn;
    sceMcTblGetDir *owDir;
    int             isSysOWrite;

    pmw = &memc_stat;

    switch (pmw->cmd) {
    case sceMcFuncNoFileInfo:
        if (result == sceMcResSucceed) {
            if (!pmw->format) {
                result = sceMcResNoFormat;
            }
        }

        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        }

        strcpy(name, pmw->saveDir);
        strcpy(name + strlen(pmw->saveDir), "/*");

        if (!sceMcGetDir(pmw->port, pmw->slot, name, 0, PR_ARRAYSIZE(pmw->curDir), pmw->curDir)) {
            pmw->cmd = sceMcFuncNoGetDir;
        } else {
            pmw->func = MEMC_FUNC_IDLE;
            pmw->cmd = 0;
        }

        break;
    case sceMcFuncNoGetDir:
        if (result == 0) {
            if (!pmw->format) {
                result = sceMcResNoFormat;
            }
        }

        if (result == sceMcResNoEntry) {
            result = 0;
        }

        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        }

        pmw->oldOWClust = 0;

        if (result >= 2) {
            isSysOWrite = FALSE;
            iscls = 0;
            isfn = 0;

            for (i = 0; i < result; i++) {
                if (pmw->curDir[i].EntryName[0] != '.') {
                    iscls += ((pmw->curDir[i].FileSizeByte + 1023) / 1024);
                    isfn++;
                }
            }

            pmw->oldOWClust = (iscls + (isfn / 2)) + 2;

            if (pmw->bChkSys ||
                memc_searchDirTbl(memc_getfilename(MEMC_FILE_ICON),  pmw->curDir, result, TRUE, sizeof(sceMcIconSys), NULL) == NULL ||
                memc_searchDirTbl(memc_getfilename(MEMC_FILE_ICON1), pmw->curDir, result, TRUE, pmw->iconSize1,       NULL) == NULL ||
                ((fname = memc_getfilename(MEMC_FILE_ICON2)) != NULL &&
                    memc_searchDirTbl(fname, pmw->curDir, result, TRUE, pmw->iconSize2, NULL) == NULL) ||
                ((fname = memc_getfilename(MEMC_FILE_ICON3)) != NULL &&
                    memc_searchDirTbl(fname, pmw->curDir, result, TRUE, pmw->iconSize3, NULL) == NULL)) {
                isSysOWrite = TRUE;
            }

            pmw->sizeOW = 0;

            if ((owDir = memc_searchDirTbl(memc_getfilename(pmw->fileNo), pmw->curDir, result, FALSE, 0, NULL)) != NULL) {
                pmw->sizeOW = owDir->FileSizeByte;
                pmw->func = MEMC_FUNC_OVERWRITE_CHECK;
                return MEMC_ERR_FILE_EXISTS;
            }

            if (isSysOWrite) {
                pmw->func = MEMC_FUNC_SAVEFILE;
                pmw->stat |= (MEMC_STAT_SYS | MEMC_STAT_ICON);
                memc_mansub_Open(memc_getfilepath(MEMC_FILE_ICON), SCE_CREAT | SCE_WRONLY);
                break;
            }

            re = memc_SaveFileClust();
            if (pmw->free < re) {
                pmw->func = MEMC_FUNC_IDLE;
                return MEMC_ERR_FULL;
            } else {
                if (memc_mansub_Open(pmw->filename, SCE_CREAT | SCE_WRONLY) == 1) {
                    pmw->func = MEMC_FUNC_IDLE;
                    return MEMC_ERR_FULL;
                }
            }
        } else {
            re = memc_SaveFileClust();

            if (pmw->free < (re + pmw->sysFileSize)) {
                pmw->func = MEMC_FUNC_IDLE;
                return MEMC_ERR_FULL;
            } else {
                if (!sceMcMkdir(pmw->port, pmw->slot, pmw->saveDir)) {
                    pmw->cmd = sceMcFuncNoMkdir;
                    pmw->stat |= MEMC_STAT_SYS;
                    break;
                } else {
                    pmw->func = MEMC_FUNC_IDLE;
                    return MEMC_ERR_FILE_INVALID;
                }
            }
        }

        break;
    case sceMcFuncNoMkdir:
        if (result == sceMcResSucceed) {
            if (!pmw->format) {
                result = sceMcResNoFormat;
            }
        }

        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        }

        if ((re = memc_mansub_ErrChk(result)) != 0) {
            return re;
        }

        pmw->func = MEMC_FUNC_SAVEFILE;
        pmw->stat |= MEMC_STAT_ICON;
        memc_mansub_Open(memc_getfilepath(MEMC_FILE_ICON), SCE_CREAT | SCE_WRONLY);
        break;
    case sceMcFuncNoOpen:
        if (result == sceMcResSucceed) {
            if (!pmw->format) {
                result = sceMcResNoFormat;
            }
        }

        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        }

        pmw->fd = result;

        if (pmw->stat & MEMC_STAT_ICON3) {
            if (!sceMcWrite(result, pmw->iconData3, pmw->iconSize3)) {
                pmw->cmd = sceMcFuncNoWrite;
            }

            break;
        }

        if (pmw->stat & MEMC_STAT_ICON2) {
            if (!sceMcWrite(result, pmw->iconData2, pmw->iconSize2)) {
                pmw->cmd = sceMcFuncNoWrite;
            }

            break;
        }

        if (pmw->stat & MEMC_STAT_ICON1) {
            if (!sceMcWrite(result, pmw->iconData1, pmw->iconSize1)) {
                pmw->cmd = sceMcFuncNoWrite;
            }

            break;
        }

        if (pmw->stat & MEMC_STAT_ICON) {
            if (!sceMcWrite(result, &memc_iconsys, sizeof(sceMcIconSys))) {
                pmw->cmd = sceMcFuncNoWrite;
            }

            break;
        }

        if (!sceMcWrite(result, pmw->buf, pmw->size)) {
            if (pmw->seek > 0) {
                pmw->cmd = 0x1000e;
            } else {
                pmw->cmd = sceMcFuncNoWrite;
            }
        }

        break;
    case 0x1000e:
        if (result == sceMcResSucceed) {
            if (!pmw->format) {
                result = sceMcResNoFormat;
            }
        }

        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        }

        sceMcSeek(pmw->fd, pmw->seek, 0);
        pmw->cmd = 0x1000f;
        break;
    case 0x1000f:
        if (result == sceMcResSucceed) {
            if (!pmw->format) {
                result = sceMcResNoFormat;
            }
        }

        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        }

        if (!sceMcWrite(pmw->fd, pmw->buf + pmw->size, pmw->size2)) {
            pmw->cmd = sceMcFuncNoWrite;
        }

        break;
    case sceMcFuncNoWrite:
        if (result == sceMcResSucceed) {
            if (!pmw->format) {
                result = sceMcResNoFormat;
            }
        }

        if (result < 0) {
            return memc_mansub_ErrChk(result);
        }

        if (pmw->type != sceMcTypePS2) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_INVALID;
        }

        sceMcFlush(pmw->fd);
        pmw->cmd = sceMcFuncNoFlush;
        break;
    case sceMcFuncNoFlush:
        if (!memc_mansub_Close()) {
            if (!(pmw->stat & MEMC_STAT_SYS)) {
                pmw->func = MEMC_FUNC_SKIP;
            }
        } else {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_FILE_INVALID;
        }

        break;
    case sceMcFuncNoClose:
        if (result == sceMcResSucceed) {
            char *fname;

            if (!(pmw->stat & MEMC_STAT_ICON1)) {
                pmw->stat |= MEMC_STAT_ICON1;
                memc_mansub_Open(memc_getfilepath(MEMC_FILE_ICON1), SCE_CREAT | SCE_WRONLY);
                break;
            }

            if (!(pmw->stat & MEMC_STAT_ICON2)) {
                if ((fname = memc_getfilepath(MEMC_FILE_ICON2)) != NULL) {
                    pmw->stat |= MEMC_STAT_ICON2;
                    memc_mansub_Open(fname, SCE_CREAT | SCE_WRONLY);
                    break;
                }
            }

            if (!(pmw->stat & MEMC_STAT_ICON3)) {
                if ((fname = memc_getfilepath(MEMC_FILE_ICON3)) != NULL) {
                    pmw->stat |= MEMC_STAT_ICON3;
                    memc_mansub_Open(fname, SCE_CREAT | SCE_WRONLY);
                    break;
                }
            }

            pmw->stat &= ~MEMC_STAT_ALL;
            memc_mansub_Open(pmw->filename, SCE_CREAT | SCE_WRONLY);
        } else {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_FILE_INVALID;
        }

        break;
    }

    return MEMC_ERR_BUSY;
}

static int memc_manager_overwrite(int result) {
    MEMC_STAT *pmw = &memc_stat;
    int        size, need;
    char      *fname;
    int        func;

    if (pmw->cmd != sceMcFuncNoFileInfo) {
        return MEMC_ERR_BUSY;
    }

    if ((result = memc_mansub_ErrChk(result)) == 0) {
        func = MEMC_FUNC_SAVEFILE;
        size = memc_SaveFileClust() + pmw->sysFileSize;
        need = size - pmw->oldOWClust;

        if (pmw->free < need) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_FULL;
        }

        if (pmw->bChkSys ||
            memc_searchDirTbl(memc_getfilename(MEMC_FILE_ICON),  pmw->curDir, 0, TRUE, sizeof(sceMcIconSys), NULL) == NULL ||
            memc_searchDirTbl(memc_getfilename(MEMC_FILE_ICON1), pmw->curDir, 0, TRUE, pmw->iconSize1,       NULL) == NULL ||
            ((fname = memc_getfilename(MEMC_FILE_ICON2)) != NULL &&
                memc_searchDirTbl(fname, pmw->curDir, 0, TRUE, pmw->iconSize2, NULL) == NULL) ||
            ((fname = memc_getfilename(MEMC_FILE_ICON3)) != NULL &&
                memc_searchDirTbl(fname, pmw->curDir, 0, TRUE, pmw->iconSize3, NULL) == NULL)) {
            pmw->func = MEMC_FUNC_SAVEFILE;
            pmw->stat |= (MEMC_STAT_SYS | MEMC_STAT_ICON);
            memc_mansub_Open(memc_getfilepath(MEMC_FILE_ICON), SCE_CREAT | SCE_WRONLY);
            return MEMC_ERR_BUSY;
        }

        if (memc_mansub_Open(pmw->filename, SCE_CREAT | SCE_WRONLY) == 1) {
            pmw->func = MEMC_FUNC_IDLE;
            return MEMC_ERR_FULL;
        } else {
            pmw->func = func;
            return MEMC_ERR_BUSY;
        }
    }

    return result;
}

static int memc_manager_chk(int mode) {
    int        re;
    int        result;
    MEMC_STAT *pmw;

    pmw = &memc_stat;

    re = sceMcSync(mode, NULL, &result);
    if (re == sceMcExecFinish) {
        switch (pmw->func) {
        case MEMC_FUNC_LOADFILE:
            return memc_mansub_load(result);
        case MEMC_FUNC_SAVEFILE:
            return memc_manager_save(result);
        case MEMC_FUNC_PORTCHECK:
            if (result == sceMcResNoFormat) {
                result = -2000;
            }
            return memc_mansub_ErrChk(result);
        case MEMC_FUNC_FORMAT:
            switch (pmw->cmd) {
            case sceMcFuncNoFileInfo:
                if (result >= 0) {
                    if (pmw->type != sceMcTypePS2) {
                        pmw->func = MEMC_FUNC_IDLE;
                        return MEMC_ERR_INVALID;
                    }

                    re = sceMcFormat(pmw->port, pmw->slot);
                    if (re == sceMcResSucceed) {
                        pmw->cmd = sceMcFuncNoFormat;
                        break;
                    }

                    pmw->func = MEMC_FUNC_IDLE;
                    pmw->cmd = 0;
                    return MEMC_ERR_FILE_INVALID;
                }

                return memc_mansub_ErrChk(result);
            case sceMcFuncNoFormat:
                if (result >= 0) {
                    pmw->func = MEMC_FUNC_IDLE;
                    return MEMC_OK;
                }

                pmw->retry++;
                if (pmw->retry >= 30) {
                    pmw->func = MEMC_FUNC_IDLE;
                    return MEMC_ERR_FILE_INVALID;
                }

                sceMcFormat(pmw->port, pmw->slot);
                break;
            }
            break;
        case MEMC_FUNC_UNK2:
        case MEMC_FUNC_SKIP:
        case MEMC_FUNC_UNK8:
        case MEMC_FUNC_GETDIR:
        case MEMC_FUNC_CHGDIR:
        case MEMC_FUNC_DELFILE:
            return memc_mansub_ErrChk(result);
        case MEMC_FUNC_GETINFO:
            return memc_mansub_GetInfo(result);
        case MEMC_FUNC_OVERWRITE:
            return memc_manager_overwrite(result);
        case MEMC_FUNC_IDLE:
        case MEMC_FUNC_UNK5:
        default:
            pmw->func = MEMC_FUNC_IDLE;
            break;
        case MEMC_FUNC_OVERWRITE_CHECK:
            return MEMC_ERR_BUSY;
        }
    }

    if (re == sceMcExecIdle) {
        pmw->func = MEMC_FUNC_IDLE;
        return MEMC_OK;
    }

    return MEMC_ERR_BUSY;
}

int memc_manager(int mode) {
    int re;

    if (mode == MEMC_MODE_ASYNC) {
        re = memc_manager_chk(mode);
    } else {
        do {
            re = memc_manager_chk(mode);
        } while (re == MEMC_ERR_BUSY);
    }

    return re;
}
