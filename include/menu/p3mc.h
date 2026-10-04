#ifndef P3MC_H
#define P3MC_H

#include "common.h"

#include <eetypes.h>
#include <libmc.h>

/* File numbers per save kind: LOG000-LOG079 and REP000-REP079. */
#define P3MC_FILE_MAX      80
/* Entries read from the card directory in one listing. */
#define P3MC_DIR_ENTRY_MAX 80

typedef struct { // 0x14
    /* 0x00 */ u_int scDate[2];
    /* 0x08 */ u_char name[8];
    /* 0x10 */ u_int score;
} P3MC_RANKSCORE;

typedef struct { // 0x4c4
    /* 0x000 */ int nSplay;
    /* 0x004 */ P3MC_RANKSCORE splay[20];
    /* 0x194 */ int nVplay[4];
    /* 0x1a4 */ P3MC_RANKSCORE vplay[4][10];
} P3MC_STAGERANK;

/*
 * Save timestamp: BCD fields from sceCdReadClock, plus a random tiebreak in
 * pad. Read as two little-endian words the fields give year:month:day and
 * hour:minute:second:pad, so the game orders and matches saves by comparing
 * those words (see P3MC_SortUser, TsUserList_SetCurFileNoCusor).
 */
typedef struct { // 0x8
    /* 0x0 */ u_char day;
    /* 0x1 */ u_char month;
    /* 0x2 */ u_short year;
    /* 0x4 */ u_char pad;
    /* 0x5 */ u_char second;
    /* 0x6 */ u_char minute;
    /* 0x7 */ u_char hour;
} P3MC_DATE;

/* Word 0 (year:month:day) or 1 (hour:minute:second:pad) of a timestamp */
#define P3MC_DATE_WORD(date, i) (((const u_int *)(date))[i])

/* Kinds of save file (USER_DATA::mode). Used as a bit mask where several kinds are listed. */
enum {
    P3MC_MODE_LOG = 1,    /* the player's save ("LOGnnn" directories) */
    P3MC_MODE_REPLAY = 2, /* a stage replay ("REPnnn" directories) */
    P3MC_MODE_ALL = 3
};

typedef struct { // 0x2664
    /* 0x0000 */ char name[12];
    /* 0x000c */ char name1[12];
    /* 0x0018 */ char name2[12];
    /* 0x0024 */ u_short stageNo;
    /* 0x0026 */ u_short fileNo;
    /* 0x0028 */ u_char flg;
    /* 0x0029 */ u_char mode;
    /* 0x002a */ u_char isVs; /* PLAY_MODE of the stage that was played */
    /* 0x002b */ u_char vsLev;
    /* 0x002c */ u_int score;
    /* 0x0030 */ u_int score2;
    /* 0x0034 */ u_short roundNo;
    /* 0x0036 */ u_char winner;
    /* 0x0037 */ u_char pads;
    /* 0x0038 */ u_int pad[1];
    /* 0x003c */ P3MC_DATE date;
    /* 0x0044 */ P3MC_STAGERANK stageRank[8];
} USER_DATA;

typedef struct { // 0x2684
    /* 0x0000 */ char header[16];
    /* 0x0010 */ USER_DATA user;
    /* 0x2674 */ char footer[16];
} USER_HEADER;

typedef struct { // 0x10
    /* 0x0 */ char footer[16];
} USER_FOOTER;

typedef struct { // 0x1c
    /* 0x00 */ u_char *pMemTop;
    /* 0x04 */ u_int rwsize;
    /* 0x08 */ u_int datasize;
    /* 0x0c */ u_int srcsize;
    /* 0x10 */ USER_HEADER *pHead;
    /* 0x14 */ void *pData;
    /* 0x18 */ USER_FOOTER *pFoot;
} MCRWDATA_HDL;

typedef struct { // 0xc2980
    /* 0x00000 */ int nGetUser;
    /* 0x00004 */ USER_DATA getUser[P3MC_FILE_MAX + 1]; /* + 1: room for the user being saved */
    /* 0xc25a8 */ int logPage_flg;
    /* 0xc25ac */ int repPage_flg;
    /* 0xc25b0 */ int nLogGet;
    /* 0xc25b4 */ int nRepGet;
    /* 0xc25b8 */ USER_DATA *plog_user[P3MC_FILE_MAX];
    /* 0xc26f8 */ USER_DATA *prep_user[P3MC_FILE_MAX];
    /* 0xc2838 */ int nUserMax;
    /* 0xc283c */ USER_DATA *pUserTbl[P3MC_FILE_MAX + 1];
} P3MC_USRLST;

typedef struct { // 0x20
    /* 0x00 */ int prg;
    /* 0x04 */ int dstat;
    /* 0x08 */ int data_no;
    /* 0x0c */ int data_mode;
    /* 0x10 */ int data_stage;
    /* 0x14 */ int prgflag;
    /* 0x18 */ MCRWDATA_HDL *dhdl;
    /* 0x1c */ void *data_cfunc;
} P3MC_WORK;

/* P3MC_SaveUser / P3MC_LoadUser flags (P3MC_WORK::prgflag). */
#define P3MC_FLAG_OVERWRITE    0x1 /* replace an existing file without asking */
#define P3MC_FLAG_FORMAT       0x2 /* format an unformatted card without asking */
#define P3MC_FLAG_WRITE_SYSTEM 0x4 /* also write the system and icon files (memc_save_file bSysRW) */
#define P3MC_FLAG_RETRY_SWAP   0x8 /* re-read the card instead of failing when it was swapped */

/* P3MC_SaveCheck / P3MC_LoadCheck results: negative while the operation is still running. */
enum {
    P3MC_RES_FORMATTING = -3, /* save only: the card is being formatted */
    P3MC_RES_ACCESSING = -2,  /* the card is being read or written */
    P3MC_RES_BUSY = -1,
    P3MC_RES_OK = 0,
    P3MC_RES_FILE_ERROR = 1,
    P3MC_RES_UNFORMATTED = 2, /* load only */
    P3MC_RES_NO_CARD = 3,
    P3MC_RES_NO_SAVE_DATA = 4, /* load only */
    P3MC_RES_CARD_SWAPPED = 5,
    P3MC_RES_BAD_DATA = 6,     /* load only */
    P3MC_RES_NO_SPACE = 7,     /* save only */
    P3MC_RES_CONFIRM_OVERWRITE = 8, /* save only */
    P3MC_RES_NEED_FORMAT = 9,  /* save only */
    P3MC_RES_FORMAT_FAILED = 10, /* save only */
    P3MC_RES_NO_FILE = 11      /* load only, reported to the menu as NO_SAVE_DATA */
};

typedef int (*P3MCDataCheckFunc)(P3MC_WORK *pw);

typedef struct { // 0x3b00
    /* 0x0000 */ int curState;
    /* 0x0004 */ int curMode;
    /* 0x0008 */ int curFno;
    /* 0x000c */ int curUserMode;
    /* 0x0010 */ int bFirst;
    /* 0x0014 */ MCRWDATA_HDL chkData;
    /* 0x0030 */ P3MC_USRLST *pUserLst;
    /* 0x0034 */ u_char UserHeadTmp[9860];
    /* 0x26c0 */ sceMcTblGetDir dirTable[P3MC_DIR_ENTRY_MAX + 1];
} GETUSER_WORK;

int P3MC_InitReady(void);
int P3MC_GetSaveSize(int size, int mode);
void P3MC_SetCheckSaveSize(int mode, int fsize, int csize);
/* Polls the card slot: P3MC_RES_BUSY while checking, then P3MC_RES_OK, P3MC_RES_NO_CARD or P3MC_RES_CARD_SWAPPED. */
int P3MC_CheckChange(void);
void P3MC_CheckChangeClear(void);
void P3MC_CheckChangeSet(void);
int P3MC_CheckIsNewSave(int mode);
void P3MC_DeleteDataWork(MCRWDATA_HDL *phdl);
MCRWDATA_HDL* P3MC_MakeDataWork(int dsize, USER_DATA *puser);
int P3MC_GetUserStart(int mode, P3MC_USRLST *pUsrLst, int bFirst);
void P3MC_GetUserEnd(void);
int P3MC_GetUserCheck(void);
void P3MC_AddUser(P3MC_USRLST *pUser, int mode, USER_DATA *puser);
int P3MC_SortUser(P3MC_USRLST *pUser, int mode, int isSave);
int P3MC_CheckBrokenUser(P3MC_USRLST *pUser, int mode);
void P3MC_OpeningCheckStart(void);
void P3MC_OpeningCheckEnd(void);
/* P3MC_OpeningCheck result: P3MC_RES_BUSY while working, -P3MC_RES_UNFORMATTED or
 * -P3MC_RES_NO_CARD, otherwise a mask of the saves that will fit on the card (free
 * space, or an existing file that can be overwritten). */
#define P3MC_OPEN_LOG_FITS    1
#define P3MC_OPEN_REPLAY_FITS 2
int P3MC_OpeningCheck(void);
int P3MC_LoadUser(int mode, int fileNo, MCRWDATA_HDL *pdhdl, int flg);
int P3MC_LoadCheck(void);
void P3MC_SetUserWorkTime(USER_DATA *puser);
int P3MC_SaveUser(MCRWDATA_HDL *pdhdl, int flg);
int P3MC_SaveCheck(void);

#endif /* P3MC_H */
