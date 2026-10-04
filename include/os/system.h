#ifndef SYSTEM_H
#define SYSTEM_H

#include "common.h"

#include "os/syssub.h"

#include "main/etc.h"
#include "main/mcctrl.h"

#include <libgraph.h>
#include <libdma.h>

extern int outbuf_idx;
extern int oddeven_idx;

extern sceGsDBuffDc   DBufDc;
extern sceGsDrawEnv1* drawEnvP[5];
extern PADD pad[2];

extern GLOBAL_DATA global_data;
extern GAME_STATUS game_status;
extern MC_REP_STR  mc_rep_str;

/* .sdata */
extern HAT_CHANGE_ENUM hat_change_enum;

extern INGAME_COMMON_STR ingame_common_str;

typedef struct { // 0x100
    /* 0x00 */ sceDmaTag dmatag;
    /* 0x10 */ sceGifTag giftag;
    /* 0x20 */ sceGsDrawEnv1 drEnv;
    /* 0xa0 */ sceGsClear clear;
} CLEAR_VRAM_DMA;

int  SetIopModule(void);

void initSystem(void);
void exitSystem(void);

void SetOsFuncAddr(void *func_pp);
void osFunc(void);

void systemCtrlMain(void *xx);

void mallocInit(void);

#endif /* SYSTEM_H */
