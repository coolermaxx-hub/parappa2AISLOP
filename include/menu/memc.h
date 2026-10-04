#ifndef MEMC_H
#define MEMC_H

#include "common.h"

#include <eetypes.h>
#include <libmc.h>

#define MEMC_ICON_VIEW (0)
#define MEMC_ICON_COPY (1)
#define MEMC_ICON_DEL  (2)

#define MEMC_FILE_ICON    (-1)
#define MEMC_FILE_ICON1   (-2)
#define MEMC_FILE_ICON2   (-3)
#define MEMC_FILE_ICON3   (-4)
#define MEMC_FILE_SAVEDIR (0)
#define MEMC_FILE_SAVE    (1)
#define MEMC_FILE_MAX     (2)

#define MEMC_FLAG_DIR   (1<<0) /* valid dir */
#define MEMC_FLAG_ICON  (1<<1)
#define MEMC_FLAG_ICON1 (1<<2)
#define MEMC_FLAG_ICON2 (1<<3)
#define MEMC_FLAG_ICON3 (1<<4)
#define MEMC_FLAG_UNK5  (1<<5)
#define MEMC_FLAG_UNK6  (1<<6)
#define MEMC_FLAG_UNK7  (1<<7)
#define MEMC_FLAG_PS1   (1<<8)
#define MCMC_FLAG_PS2   (1<<9)
#define MEMC_FLAG_PDA   (1<<10)

/* stat write flags */
#define MEMC_STAT_SYS   (1<<0)
#define MEMC_STAT_ICON  (1<<1)
#define MEMC_STAT_ICON1 (1<<2)
#define MEMC_STAT_ICON2 (1<<3)
#define MEMC_STAT_ICON3 (1<<4)
#define MEMC_STAT_ALL   (MEMC_STAT_SYS | MEMC_STAT_ICON | MEMC_STAT_ICON1 | MEMC_STAT_ICON2 | MEMC_STAT_ICON3)

#define MEMC_MODE_SYNC  (0)
#define MEMC_MODE_ASYNC (1)

#define MEMC_FUNC_IDLE            (0)
#define MEMC_FUNC_PORTCHECK       (1)
#define MEMC_FUNC_UNK2            (2) /* unused */
#define MEMC_FUNC_LOADFILE        (3)
#define MEMC_FUNC_SAVEFILE        (4)
#define MEMC_FUNC_UNK5            (5) /* unused */
#define MEMC_FUNC_SKIP            (6)
#define MEMC_FUNC_FORMAT          (7)
#define MEMC_FUNC_UNK8            (8) /* unused */
#define MEMC_FUNC_GETDIR          (9)
#define MEMC_FUNC_CHGDIR          (10)
#define MEMC_FUNC_GETINFO         (11)
#define MEMC_FUNC_DELFILE         (12)
#define MEMC_FUNC_OVERWRITE_CHECK (13)
#define MEMC_FUNC_OVERWRITE       (14)

#define MEMC_OK                   (0)
#define MEMC_ERR_FILE_INVALID     (1)
#define MEMC_ERR_INVALID          (2) /* unknown err. / invalid card */
#define MEMC_ERR_UNFORMATTED      (3)
#define MEMC_ERR_FULL             (4)
#define MEMC_ERR_FILE_NOT_FOUND   (5)
#define MEMC_ERR_SWAP             (6)
#define MEMC_ERR_BUSY             (16)
#define MEMC_ERR_DIR_NOT_FOUND    (17)
#define MEMC_ERR_FILE_EXISTS      (18) /* overwrite check */
#define MEMC_ERR_DIR_TOO_MANY     (19) /* too many files */
#define MEMC_ERR_SWAP_UNFORMATTED (48)

/* Private cmd values for the split write of a save file with a seek offset (not SDK sceMcFunc*). */
#define MEMC_CMD_WRITE_SEEK  (0x1000e) /* seek to pmw->seek before the second write */
#define MEMC_CMD_WRITE_TAIL  (0x1000f) /* write the remaining pmw->size2 bytes */

typedef struct { // 0x340
    /* 0x000 */ u_int flag;
    /* 0x004 */ u_int stat;
    /* 0x008 */ int port;
    /* 0x00c */ int slot;
    /* 0x010 */ int type;
    /* 0x014 */ int free;
    /* 0x018 */ int cmd;
    /* 0x01c */ int func;
    /* 0x020 */ int fd;
    /* 0x024 */ int retry;
    /* 0x028 */ int format;
    /* 0x02c */ char *buf;
    /* 0x030 */ int size;
    /* 0x034 */ int isChange;
    /* 0x038 */ int seek;
    /* 0x03c */ int size2;
    /* 0x040 */ u_int oldOWClust;
    /* 0x044 */ u_int sizeOW;
    /* 0x048 */ int bChkSys;
    /* 0x04c */ int isSyncClose;
    /* 0x050 */ char filename[64];
    /* 0x090 */ int fileNo;
    /* 0x0c0 */ sceMcTblGetDir curDir[8];
    /* 0x2c0 */ char saveDir[64];
    /* 0x300 */ int iconSize1;
    /* 0x304 */ u_char *iconData1;
    /* 0x308 */ int iconSize2;
    /* 0x30c */ u_char *iconData2;
    /* 0x310 */ int iconSize3;
    /* 0x314 */ u_char *iconData3;
    /* 0x318 */ int sysFileSize;
} MEMC_STAT;

typedef struct { // 0x18
    /* 0x00 */ u_int flag;
    /* 0x04 */ int free;
    /* 0x08 */ int allfile;
    /* 0x0c */ int savefile;
    /* 0x10 */ int dirfileMax;
    /* 0x14 */ sceMcTblGetDir *dirfile;
} MEMC_INFO;

void memc_init(void);

void memc_setDirName(char *name);
void memc_setSaveTitle(char *name, int nLFPos);
void memc_setIconSysHed(void *pIhData, int IhSize);
void memc_setSaveIcon(int no, void *pIconData, int nIconSize);

char* memc_getfilename(int no);
char* memc_getfilepath(int no);

int memc_checkFormat(void);
int memc_getChangeState(void);
void memc_setChangeState(int flg);
sceMcTblGetDir* memc_searchDirTbl(char *name, sceMcTblGetDir *dirTbl, int num, int isClose, int cmpSize, int *status);

int memc_port_info(int port, MEMC_INFO *info);
int memc_del_file(int port, int no);
int memc_load_file(int port, int no, char *buf, int size);
int memc_loadFirst(int port, int no, char *buf, int size);
int memc_save_file(int port, int no, char *buf, int size, int bSysRW);
int memc_seeksave_file(int port, int no, char *buf, int size, int seek, int sizef, int bSysRW);
int memc_save_overwrite(void);
int memc_port_check(int port, int *type, int *free);
int memc_format(int port);
int memc_chg_dir(int port, char *name);
int memc_get_dir(int port, char *name, sceMcTblGetDir *dir, int max);
int memc_get_dir_continue(sceMcTblGetDir *dir, int max);

int memc_manager(int mode);

#endif /* MEMC_H */
