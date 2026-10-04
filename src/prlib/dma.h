#ifndef PRLIB_DMA_H
#define PRLIB_DMA_H

#include "common.h"

#include <eetypes.h>

/* sceDmaTag::id values: the tag ID in bits 4-6, with PCE and IRQ clear. */
enum PrDmaTagId {
    PR_DMA_TAG_REFE = 0x00, /* transfer from addr, then end */
    PR_DMA_TAG_CNT  = 0x10, /* transfer the following qwords, continue after them */
    PR_DMA_TAG_NEXT = 0x20, /* transfer the following qwords, continue at addr */
    PR_DMA_TAG_REF  = 0x30, /* transfer from addr, continue with the next tag */
    PR_DMA_TAG_REFS = 0x40, /* as REF, with stall control */
    PR_DMA_TAG_CALL = 0x50, /* transfer the following qwords, push and continue at addr */
    PR_DMA_TAG_RET  = 0x60, /* transfer the following qwords, pop and return */
    PR_DMA_TAG_END  = 0x70, /* transfer the following qwords, then end */
};

void PrWaitDmaFinish(u_int channel);

#endif /* PRLIB_DMA_H */
