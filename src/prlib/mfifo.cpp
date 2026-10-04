#include "mfifo.h"

#include "dma.h"
#include "spram.h"

#include <eeregs.h>
#include <libdma.h>

#define PR_MFIFOSIZE (0x10000)
// Ring buffer for the GIF MFIFO; lives in its own section at the end of the image.
char mfifoBase[PR_MFIFOSIZE] __attribute__((section(".mfifo")));

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)EE_SCRATCHPAD_BASE;
static bool mfifoProcessing = false;

#define PR_ALIGNSPR(addr) ((u_int)(addr) & 0xfff0)

void PrInitializeMfifo() {
    *D_RBOR = (u_int)mfifoBase;
    *D_RBSR = PR_MFIFOSIZE - 0x10;
}

void PrCleanupMfifo() {
    PrStopMfifo();
}

void PrStartMfifo() {
    if (!mfifoProcessing) {
        mfifoProcessing = true;
        PrWaitDmaFinish(SCE_DMA_fromSPR);
        PrWaitDmaFinish(SCE_DMA_GIF);

        /*
         * Enable MFIFO and set the GIF
         * as the drain channel.
         */
        ((tD_CTRL*)D_CTRL)->MFD = 3;
        ((tD_CTRL*)D_CTRL)->RELE = 0;
        ((tD_CTRL*)D_CTRL)->RCYC = 0;
        
        /*
         * Initialize the SPRfrom and GIF
         * addresses to the ring buffer.
         *
         * Setting them to the same value
         * will also stall the transfer
         * (D2_TADR==D8_MADR).
         */
        *D8_MADR = (u_int)mfifoBase;
        *D2_TADR = (u_int)mfifoBase;

        /*
         * Start GIF transfer.
         *
         * MOD=Chain
         * ASP,TTE,TIE=0
         * STR=1
         */
        *(int*)D2_CHCR = 0x104;
    }
}

void PrStopMfifo() {
    if (mfifoProcessing) {
        PrSendMfifo(&prSpramData->m_end_dmatag);
        PrWaitDmaFinish(SCE_DMA_GIF);

        /*
         * Disable MFIFO drain and cycle stealing.
         */
        ((tD_CTRL*)D_CTRL)->MFD = 0;
        ((tD_CTRL*)D_CTRL)->RELE = 0;

        mfifoProcessing = false;
    }
}

void PrWaitMfifo() {
    if (mfifoProcessing) {
        while (*D8_CHCR & D_CHCR_STR_M);
        while (*D2_TADR != *D8_MADR); /* Wait until the GIF is done */
    }
}

void PrSendMfifo(const sceDmaTag *tag) {
    u_int qwc = tag->qwc + 1;
    u_int size = qwc * 16;

    while (*D8_CHCR & D_CHCR_STR_M);

    *D8_SADR = PR_ALIGNSPR(tag);
    *D8_QWC = qwc;

    while (1) {
        u_int gif_tadr = *D2_TADR;
        u_int spr_madr = *D8_MADR;
        if (gif_tadr == spr_madr /* MFIFO is empty */ ||
            /* Ring buffer has enough size */
            size < PR_ALIGNSPR(gif_tadr - spr_madr)) {
            break;
        }
    }

    *(int*)D8_CHCR |= D_CHCR_STR_M;
}

u_int PrMfifoUnsentDataSize() {
    u_int gif_tadr = *D2_TADR;
    u_int spr_madr = *D8_MADR;
    return PR_ALIGNSPR(spr_madr - gif_tadr);
}
