#include "os/system.h"

#include "dbug/syori.h"

#include "main/main.h"

#include "os/cmngifpk.h"
#include "os/mtc.h"

#include <devvif0.h>
#include <devvu0.h>
#include <eekernel.h>
#include <libcdvd.h>
#include <limits.h>
#include <sifcmd.h>
#include <sifdev.h>
#include <sifrpc.h>

#include <stdio.h>
#include <stdlib.h>

/* linker: _end & _stack_size */
extern void *_end;
extern void *_stack_size;

static u_char *iop_module[] = {
    "cdrom0:\\IRX\\SIO2MAN.IRX;1",
    "cdrom0:\\IRX\\PADMAN.IRX;1",
    "cdrom0:\\IRX\\LIBSD.IRX;1",
    "cdrom0:\\IRX\\SDRDRV.IRX;1",
    "cdrom0:\\IRX\\MODMIDI.IRX;1",
    "cdrom0:\\IRX\\MODHSYN.IRX;1",
    "cdrom0:\\IRX\\MODMSIN.IRX;1",
    "cdrom0:\\IRX\\MCMAN.IRX;1",
    "cdrom0:\\IRX\\MCSERV.IRX;1",
    "cdrom0:\\IRX\\WAVE2PS2.IRX;1",
    "cdrom0:\\IRX\\TAPCTRL.IRX;1",
};

int double_tmp_on_off = 0;

static int _end_addr = (int)&_end;
static int _stack_size_addr = (int)&_stack_size;

sceGsDBuffDc DBufDc = {};
int outbuf_idx = 0;
int oddeven_idx = 0;

sceGsDrawEnv1 *drawEnvP[5] = {};

PADD pad[2] = {};

/* Global in the original; static keeps it out of the linker's common symbols. */
static u_long128 GifPkCommon[8192];

void (*OsFuncAddr)() = NULL;

GLOBAL_DATA       global_data       = {};
GAME_STATUS       game_status       = {};
MC_REP_STR        mc_rep_str        = {};
HAT_CHANGE_ENUM   hat_change_enum   = HCNG_R1;
INGAME_COMMON_STR ingame_common_str = {};

static sceGsDrawEnv1 drawEnvSp;
static sceGsDrawEnv1 drawEnvZbuff;
static sceGsDrawEnv1 drawEnvEnd;

int main(void) {
    mallocInit();

#if defined(PRD_SHIFTABLE)
    scePrintf("P3 shifted ELF - https://github.com/parappadev/parappa2\n");
#endif

    while (1) {
        MtcInit();
        initSystem();

        MtcStart(systemCtrlMain);

        MtcQuit();
        exitSystem();
    }
}

int SetIopModule(void) {
    u_int i;

    sceSifInitRpc(0);
    sceCdInit(0);

    while (!sceSifRebootIop("cdrom0:\\IRX\\IOPRP23.IMG;1"));
    while (!sceSifSyncIop());

    sceSifInitRpc(0);
    sceSifLoadFileReset();
    sceFsReset();

    sceCdInit(0);
    sceCdMmode(SCECdDVD);

    for (i = 0; i < PR_ARRAYSIZEU(iop_module); i++) {
        if (sceSifLoadModule(iop_module[i], 0, NULL) < 0) {
            printf("Can't load module [%s]\n", iop_module[i]);
            return 1;
        }
    }

    return 0;
}


static void firstClrFrameBuffer(void) {
    CLEAR_VRAM_DMA vclr_dma;

    vclr_dma.dmatag.qwc = 15;
    vclr_dma.dmatag.id = 0;
    vclr_dma.dmatag.next = (sceDmaTag*)&vclr_dma.giftag;

    vclr_dma.giftag.NLOOP = 14;
    vclr_dma.giftag.EOP = 1;
    vclr_dma.giftag.FLG = 0;
    vclr_dma.giftag.NREG = 1;
    vclr_dma.giftag.REGS0 = SCE_GIF_PACKED_AD;

    vclr_dma.clear.rgbaq.Q = 1.0f;

    sceGsSetDefDrawEnv(&vclr_dma.drEnv, SCE_GS_PSMCT32, SCREEN_WIDTH, SCREEN_HEIGHT, SCE_GS_ZNOUSE, SCE_GS_ZNOUSE);
    sceGsSetDefClear(
        &vclr_dma.clear, SCE_GS_ZNOUSE, 2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2), SCREEN_WIDTH,
        SCREEN_HEIGHT, 0, 0, 0, 0, 0
    );

    sceDmaSync(sceDmaGetChan(SCE_DMA_GIF), 0, INT_MAX);
    FlushCache(WRITEBACK_DCACHE);

    sceDmaSend(sceDmaGetChan(SCE_DMA_GIF), &vclr_dma);
    sceGsSyncPath(0, 0);
}

void initSystem(void) {
    SetIopModule();

    sceDevVif0Reset();
    sceDevVu0Reset();
    sceGsResetPath();
    sceDmaReset(SCE_DMA_VIF1);

    GPadInit();

    firstClrFrameBuffer();
    sceGsSyncV(0);
    sceGsResetGraph(0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME);

    sceGsSetDefDBuffDc(&DBufDc, SCE_GS_PSMCT32, SCREEN_WIDTH, SCREEN_HEIGHT / 2, SCE_GS_ZGEQUAL, SCE_GS_PSMZ32, SCE_GS_CLEAR);
    SetBackColor(0, 0, 0);

    /* Start TIMER0 on H-BLANK mode. */
    *T0_MODE = T_MODE_CLKS_M | T_MODE_CUE_M;
#if defined(PRD_SYORI)
    /*
     * TIMER2 and TIMER3 are reserved by the
     * EE kernel, so modifying them will cause
     * a TLB modification exception in HW.
     *
     * prlib's rendering statictics use TIMER3
     * regardless.
     *
     * Testing confirms it's already set at
     * the mode we want, so we can still use them.
     */
     if ((*T3_MODE & (T_MODE_CLKS_M|T_MODE_CUE_M)) !=
         (T_MODE_CLKS_M|T_MODE_CUE_M)) {
        printf("Unexpected TIMER3 mode\n");
        PR_BREAK();
     }
#if 0
    *T3_MODE = T_MODE_CLKS_M | T_MODE_CUE_M;
#endif
#endif

    outbuf_idx = 0;

    FlushCache(WRITEBACK_DCACHE);
    sceGsSyncPath(0, 0);
    sceGsSwapDBuffDc(&DBufDc, outbuf_idx);

    drawEnvP[USE_BUF_DRAW0] = &DBufDc.draw11;
    drawEnvP[USE_BUF_DRAW1] = &DBufDc.draw01;

    drawEnvSp = DBufDc.draw01;
    drawEnvSp.frame1.FBP = FBP_VRAM_DRAW2;
    drawEnvP[USE_BUF_DRAW2] = &drawEnvSp;
    sceGsSetHalfOffset(&drawEnvSp, 2048, 2048, 0);

    drawEnvZbuff = DBufDc.draw01;
    drawEnvZbuff.frame1.FBP = FBP_VRAM_ZBUF;
    drawEnvP[USE_BUF_ZBUFF] = &drawEnvZbuff;
    sceGsSetHalfOffset(&drawEnvZbuff, 2048, 2048, 0);

    drawEnvEnd = DBufDc.draw01;
    drawEnvEnd.frame1.FBP = FBP_VRAM_END;
    drawEnvP[USE_BUF_END] = &drawEnvEnd;
    sceGsSetHalfOffset(&drawEnvEnd, 2048, 2048, 0);

    CmnGifInit(GifPkCommon, PR_ARRAYSIZE(GifPkCommon));
    CmnGifClear();

    SyoriLineInit(256);
    while (!sceGsSyncV(0));

    usrMallcInit();
}

void exitSystem(void) {
    sceGsSyncPath(0, 0);
    GPadExit();
    sceCdInit(SCECdEXIT);
    sceSifExitCmd();
}

void SetOsFuncAddr(void* func_pp) {
    OsFuncAddr = func_pp;
}

void osFunc(void) {
    int total_h_cnt;

    rand();

    SyoriLineCnt(16);

    CmnGifFlush();
    sceGsSyncPath(0, 0);

    SyoriLineCnt(16);
    SyoriLineDisp(132, 2);

    total_h_cnt = *T0_COUNT;
    oddeven_idx = sceGsSyncV(0);
    *T0_COUNT = 0;

    SyoriLineReset();
    SyoriLineCnt(0);

    GPadSysRead();
    GPadRead(pad);

    outbuf_idx ^= 1;
    CmnGifClear();

    if (outbuf_idx) {
        sceGsSetHalfOffset(&DBufDc.draw11, 2048, 2048, oddeven_idx ^ 1);
        sceGsSetHalfOffset2(&DBufDc.draw12, 2048, 2048, oddeven_idx ^ 1);
    } else {
        sceGsSetHalfOffset(&DBufDc.draw01, 2048, 2048, oddeven_idx ^ 1);
        sceGsSetHalfOffset2(&DBufDc.draw02, 2048, 2048, oddeven_idx ^ 1);
    }

    FlushCache(WRITEBACK_DCACHE);

    /*
     * Wait for the GIF channel to become available
     * if it isn't.
     */
    while ( *D2_CHCR & D_CHCR_STR_M );

    if (sceGsSyncPath(0, 0)) {
        printf("SyncPath timeout \n");
    }
    if (sceGsSwapDBuffDc(&DBufDc, outbuf_idx)) {
        printf("swap dma error\n");
    }
}

void systemCtrlMain(void *xx) {
    MtcExec(mainStart, 1);
    SetOsFuncAddr(osFunc);

    while (1) {
        OsFuncAddr();
        MtcWait(1);
    }
}

#define MEM_SIZE    (0x02000000) /* PS2 memory size */
#define DUMMY_PAGE  (0x1000)     /* 4KB dummy page  */
#define HEADER_SIZE (0x10)       /* malloc header   */

/* See the malloc() technical note */
static int FullAllocAndFree(void) {
    int stack_sizeX = _stack_size_addr;
    int endX = _end_addr;
    int heap_size;
    
    heap_size = (MEM_SIZE - (HEADER_SIZE*2) - DUMMY_PAGE) -
                (endX + DUMMY_PAGE + stack_sizeX);

    free(malloc(heap_size));
    return heap_size;
}

void mallocInit(void) {
    int size = FullAllocAndFree();
    scePrintf("HEAP SIZE[%08x]\n", size);
}
