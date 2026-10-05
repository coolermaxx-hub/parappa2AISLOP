#ifndef WP2CD_H
#define WP2CD_H

#include "common.h"

#include <libcdvd.h>

/* WP2 commands */
#define WP2_NONE                 (0)

#define WP2_INIT                 (0x8000) /* Arg -> Block size        */
#define WP2_QUIT                 (0x0001) /* Arg -> Status (unused)   */
#define WP2_OPEN                 (0x8002) /* Arg -> File name         */
#define WP2_CLOSE                (0x0003) /* Arg -> Status (unused)   */
#define WP2_PRELOAD              (0x0004) /*         No args          */
#define WP2_START                (0x8005) /*         No args          */
#define WP2_STOP                 (0x0006) /* Arg -> Volume (unused)   */
#define WP2_SEEK                 (0x8007) /* Arg -> Seek offset/pos.  */
#define WP2_SETVOLUME            (0x0008) /* Arg -> Volume            */
#define WP2_SETVOLDIRECT         (0x0009) /* Arg -> Direct volume     */
#define WP2_SETMASTERVOL         (0x000a) /* Arg -> Master volume     */
#define WP2_SETMODE              (0x800c) /* Arg -> Mode              */
#define WP2_GETMODE              (0x800b) /*         No args          */
#define WP2_SDINIT               (0x000d) /* Arg -> Status (unused)   */
#define WP2_SETCHANNEL           (0x000f) /* Arg -> Channel           */
#define WP2_CDINIT               (0x000e) /* Arg -> Disc media mode   */
#define WP2_GETTIME              (0x8010) /*         No args          */
#define WP2_GETTIMESAMPLE        (0x8011) /*         No args          */
#define WP2_GETCDERRCODE         (0x8012) /*         No args          */
#define WP2_OPENFLOC             (0x8013) /* Arg -> File name/CD str. */
#define WP2_SEEKFLOC             (0x8014) /* Arg -> File name/CD str. */
#define WP2_PRELOADBACK          (0x0015) /*         No args          */
#define WP2_SETTRPOINT           (0x0016) /* Arg -> Transfer address  */
#define WP2_READBUFF             (0x8017) /*         No args          */

/* WP2 modes */
/* Set */
#define WP2_MODE_REPEAT_OFF      (0x0000)
#define WP2_MODE_REPEAT_DEFAULT  (0x0001)
#define WP2_MODE_REPEAT_FORCED   (0x0002)

#define WP2_MODE_STEREO          (0x0000)
#define WP2_MODE_MONO            (0x0010)

/* Get */
#define WP2_MODE_IDLE            (0x0000)
#define WP2_MODE_RUNNING         (0x1000)
#define WP2_MODE_PAUSE           (0x2000)
#define WP2_MODE_FADE            (0x4000)
#define WP2_MODE_TERMINATE       (0x8000)

/* IOP side only: the low 12 bits keep the flags set by WP2_SETMODE, and a
 * file is open for streaming while WP2_MODE_STREAMING is set. */
#define WP2_MODE_SET_MASK        (0x0fff)
#define WP2_MODE_STREAMING       (0x0800)

/* WaveP2 module ID */
#define WP2CD_DEV                (0x8800)

#if !defined(__R5900__)

typedef struct { // 0x30
    /* 0x00:0 */ unsigned int size : 32; /* File size (in sectors) */
    /* 0x04:0 */ unsigned int pos : 32;
    /* 0x08:0 */ unsigned int ofs : 32;
    /* 0x0c:0 */ unsigned int Channel : 32; /* Number of channels */
    /* 0x10 */ u_short ReqChan[2]; /* Channels reserved for audio [L&R channels] */
    /* 0x14 */ int TransPos;
    /* 0x18 */ int TransMax;
    /* 0x1c */ int Tr1Size; /* Total size of channels */
    /* 0x20 */ int StartTrPos; /* (Unused) */
    /* 0x24 */ int TransEEAdrs; /* Base addr. for EE transfers */
    /* 0x28 */ int TransId; /* DMA queuing identifier. */
    /* 0x2c */ int readBackFlag;
} WAVEP2;

typedef struct { // 0x10
    /* 0x0 */ int buf_pos[2];
    /* 0x8 */ int TrackSize;
    /* 0xc */ int dbuf_flg;
} SBUF;

typedef struct { // 0x10
    /* 0x0 */ u_int trSize;
    /* 0x4 */ u_int trAdr;
    /* 0x8 */ u_int pad1;
    /* 0xc */ u_int pad2;
    /* 0x10 */ u_char dat[0];
} P3STR_TRH;

/* Read mode types */
#define RDMODE_CD (0)
#define RDMODE_PC (1)

/* bgm_com.c */
extern int          sce_bgm_loop(void);

/* bgm_play.c */
extern void         BgmSetVolumeDirect(unsigned int vol);
extern void         BgmSetMasterVolume(unsigned int vol);
extern void         BgmCdInit(int mode);
extern void         BgmSdInit(int status);
extern int          BgmInit(int block_size);
extern void         BgmQuit(int status);
extern int          BgmOpen(char *filename);
extern int          BgmOpenFLoc(sceCdlFILE *fpLoc);
extern void         BgmClose(int status);
extern int          BgmPreLoad(void);
extern void         BgmPreLoadBack(void);
extern int          BgmReadBuffFull(void);
extern int          BgmStart(void);
extern void         BgmStop(unsigned int vol);
extern void         BgmSetVolume(unsigned int vol);
extern void         BgmSetMode(u_int maxChan);
extern unsigned int BgmGetMode(void);
extern int          BgmSeek(unsigned int ofs);
extern int          BgmSeekFLoc(sceCdlFILE *fpLoc);
extern void         BgmSetChannel(u_int chan);
extern void         BgmSetTrPoint(u_int trpos);
extern int          BgmGetTime(void);
extern int          BgmGetTSample(void);
extern int          BgmGetCdErrCode(void);

#endif

#endif /* WP2CD_H */
