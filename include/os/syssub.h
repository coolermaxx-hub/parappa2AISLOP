#ifndef SYSSUB_H
#define SYSSUB_H

#include "common.h"

#include <eetypes.h>

#include <libgraph.h>
#include <libgifpk.h>

/* Steps of the per-port pad setup state machine in GPadSysRead (PAD_SYSD::phase). */
enum {
    PAD_PHASE_DETECT = 0,        /* read the pad id and pick a setup path */
    PAD_PHASE_STD_QUERY = 40,    /* standard pad: ask for the available modes */
    PAD_PHASE_STD_SET_MODE = 41, /* standard pad: request the mode switch */
    PAD_PHASE_STD_WAIT_MODE = 42,
    PAD_PHASE_ANA_QUERY = 70,    /* analog pad: ask for the actuator count, then switch mode */
    PAD_PHASE_ANA_WAIT_MODE = 71,
    PAD_PHASE_ANA_QUERY_PRESS = 72,
    PAD_PHASE_ANA_SET_PRESS = 76, /* request pressure-sensitive mode */
    PAD_PHASE_ANA_WAIT_PRESS = 77,
    PAD_PHASE_ACT_ALIGN = 80,    /* actuator alignment request */
    PAD_PHASE_ACT_WAIT = 81,
    PAD_PHASE_READ = 99          /* setup finished: read buttons and drive actuators */
};

typedef enum {
    PAD_ENUM_NONE = 0, /* None */
    PAD_ENUM_NEG = 32, /* NeGcon controller */
    PAD_ENUM_NORMAL = 64, /* Digital controller */
    PAD_ENUM_DSHOCK = 112, /* DualShock */
    PAD_ENUM_DSHOCK2 = 121, /* DualShock 2 */
    PAD_ENUM_ANA = 80 /* Analog joystick */
} PAD_ID;

typedef enum { 
    PAD_ANA_RX = 0, 
    PAD_ANA_RY = 1, 
    PAD_ANA_LX = 2, 
    PAD_ANA_LY = 3, 
    PAD_ANA_MAX = 4 
} PAD_ANA;

typedef enum {
    PAD_PR_None = -1,
    PAD_PR_Lright = 0,
    PAD_PR_Lleft = 1,
    PAD_PR_Lup = 2,
    PAD_PR_Ldown = 3,
    PAD_PR_Rup = 4,
    PAD_PR_Rright = 5,
    PAD_PR_Rdown = 6,
    PAD_PR_Rleft = 7,
    PAD_PR_L1 = 8,
    PAD_PR_R1 = 9,
    PAD_PR_L2 = 10,
    PAD_PR_R2 = 11,
    PAD_PR_MAX = 12
} PAD_PRESS_ENUM;

typedef enum { 
    PAD_VIB_0_1 = 0, 
    PAD_VIB_0_255 = 1, 
    PAD_VIB_MAX = 2 
} PAD_VIB_ENUM;

typedef struct { // 0x24
    /* 0x00 */ PAD_ID padId; /* Pad ID */
    /* 0x04 */ u_short old; /* Button state from the previous frame. */
    /* 0x06 */ u_short shot; /* Button state from the current frame. */
    /* 0x08 */ u_short one; /* Newly pushed buttons on this frame. */
    /* 0x0a */ u_short off; /* Newly released buttons on this frame. */
    /* 0x0c */ u_char ana[4]; /* Analog data. */
    /* 0x10 */ u_char press[12]; /* Pressure sensitivity data. */
    /* 0x1c */ u_char padvib[2]; /* Vibration status to set. */
    /* 0x1e */ u_short mshot; /* Mixed button state (Dig. + Ana.) from the current frame. */
    /* 0x20 */ u_short mone; /* Newly pushed buttons (Dig. + Ana.) on this frame. */
} PADD;

typedef struct { // 0x40
    /* 0x00 */ u_char rdata[32];
    /* 0x20 */ int pad_id;
    /* 0x24 */ int phase;
    /* 0x28 */ u_char act_align[6];
    /* 0x2e */ u_char pad0[2];
    /* 0x30 */ u_char act_direct[6];
    /* 0x36 */ u_char pad1[2];
    /* 0x38 */ int pad2[2];
} PAD_SYSD;

typedef struct { // 0x8
    /* 0x0 */ u_short bit;
    /* 0x4 */ PAD_PRESS_ENUM prn;
} BIT2PR;

typedef struct { // 0x8
    /* 0x0 */ void *adrs;
    /* 0x4 */ u_int size;
} USR_MALLOC_STR;

typedef enum {
    DNUM_NON = 0,
    DNUM_SHOW = 1,
    DNUM_DRAW = 2,
    DNUM_VRAM0 = 4,
    DNUM_VRAM1 = 8,
    DNUM_VRAM2 = 16,
    DNUM_ZBUFF = 32,
    DNUM_END = 64
} DNUM_ENUM;

/* (poly): this is weird... it's not in the SDK */
typedef struct { // 0x50
    /* 0x00 */ sceGifTag giftag;
    /* 0x10 */ sceGsBitbltbuf bitbltbuf;
    /* 0x18 */ long bitbltbufaddr;
    /* 0x20 */ sceGsTrxpos trxpos;
    /* 0x28 */ long trxposaddr;
    /* 0x30 */ sceGsTrxreg trxreg;
    /* 0x38 */ long trxregaddr;
    /* 0x40 */ sceGsTrxdir trxdir;
    /* 0x48 */ long trxdiraddr;
} sceGsMoveImage;

typedef enum {
    USE_VRAM_DRAW0 = 0,
    USE_VRAM_DRAW1 = 143360,
    USE_VRAM_ZBUF = 286720,
    USE_VRAM_DRAW2 = 430080,
    USE_VRAM_END = 655360,
    FBP_VRAM_DRAW0 = 0,
    FBP_VRAM_DRAW1 = 70,
    FBP_VRAM_ZBUF = 140,
    FBP_VRAM_DRAW2 = 210,
    FBP_VRAM_END = 320,
    TBP_VRAM_DRAW0 = 0,
    TBP_VRAM_DRAW1 = 2240,
    TBP_VRAM_ZBUF = 4480,
    TBP_VRAM_DRAW2 = 6720,
    TBP_VRAM_END = 10240
} USE_VRAM_ENUM;

typedef enum {
    USE_BUF_DRAW0 = 0,
    USE_BUF_DRAW1 = 1,
    USE_BUF_DRAW2 = 2,
    USE_BUF_ZBUFF = 3,
    USE_BUF_END = 4,
    USE_BUF_DRAW_MAX = 5
} USE_BUF_ENUM;

void WorkClear(void *clr_adrs, int size);

void GPadInit(void);
void GPadExit(void);
void GPadSysRead(void);

void padMakeData(PADD *pad_pp, u_short paddata);
void pad0Clear(PADD *pad_pp);
void padOneOffBitCLear(PADD *pad_pp);
void padNormalRead(PADD *pad_pp, u_char *rdata_pp);
void padAnaRead(PADD *pad_pp, u_char *rdata_pp);
void padAnaRead0Clear(PADD *pad_pp);
void padPrsRead(PADD *pad_pp, u_char *rdata_pp);
void padPrsRead0Clear(PADD *pad_pp);
void padPrsTreate(PADD *pad_pp);
void padActSet(PADD *pad_pp, PAD_SYSD *sysPad_pp);
void padActClear(PADD *pad_pp);
void padAnaMixPad(PADD *pad_pp);

void GPadRead(PADD *pad_pp);

PAD_PRESS_ENUM GetPadbit2PressId(u_short padbit);
u_char GetPadbit2PressPad(PADD *pad_pp, u_short padbit);

void SetBackColor(u_char R, u_char G, u_char B);
void ChangeDrawArea(sceGsDrawEnv1 *env_pp);
void ChangeDrawAreaSetGifTag(sceGsDrawEnv1 *env_pp, sceGifPacket *gifpk_pp);
void ChangeDrawArea2(sceGsDrawEnv1 *env_pp);
void ClearFrameBufferGifTag(sceGsFrame *draw_pp, sceGifPacket *gifpk_pp, u_char r, u_char g, u_char b, u_char a);
void GGsSetLocalMoveImage(
    sceGsMoveImage *lp, short dbp, short dbw, short dpsm,
    short dx, short dy, short sbp, short sbw,
    short spsm, short sx, short sy, short w, short h, short dir
);
void GGsExecLocalMoveImage(sceGsMoveImage *lp);

u_int randMakeMax(u_int max);

char* ByteString(unsigned int n);

u_int ReportHeapUsage(void);

void usrMallcInit(void);
void* usrMalloc(u_int size);
void usrFree(void *adrs);
void usrMallcReport(void);

sceGsDrawEnv1* DrawGetDrawEnvP(DNUM_ENUM dnum);
sceGsFrame* DrawGetFrameP(DNUM_ENUM dnum);
int DrawGetFbpPos(DNUM_ENUM dnum);
int DrawGetTbpPos(DNUM_ENUM dnum);

#endif /* SYSSUB_H */
