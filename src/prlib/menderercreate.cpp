#include "common.h"

#include <eekernel.h>
#include <libdma.h>
#include <libgraph.h>

extern u_int prMendererDrawFbp;

/* data */
extern int mendererTextureRequested;
extern u_long mendererTexturePacket[];

INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", PrSynchronizeMendererParameter__Ff);

INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", PrInitializeTextureCreation__FUiUiUiUi);

INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", CreateMendererTexture__Ff);

INCLUDE_ASM("asm/nonmatchings/prlib/menderercreate", PrCreateMendererTexture);

void PrWaitMendererTexture(sceGsDrawEnv1 *env, const sceGsFrame &frame, const sceGsXyoffset &xyoffset) {
    if (mendererTextureRequested != 0) {
        mendererTextureRequested = 0;
        prMendererDrawFbp = frame.FBP;
        *(sceGsFrame*)&mendererTexturePacket[24] = frame;
        *(sceGsXyoffset*)&mendererTexturePacket[26] = xyoffset;
        *(sceGsScissor*)&mendererTexturePacket[10] = env->scissor1;

        FlushCache(WRITEBACK_DCACHE);
        sceGsSyncPath(0, 0);

        sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
        chan->chcr.TTE = 0;
        sceDmaSendN(chan, mendererTexturePacket, 14);
        sceGsSyncPath(0, 0);
    }
}

INCLUDE_RODATA("asm/nonmatchings/prlib/menderercreate", D_00396800);
