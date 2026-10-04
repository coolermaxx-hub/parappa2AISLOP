#include "dmaqueue.h"

#include "prpriv.h"
#include "dma.h"

#include <eekernel.h>

#include <malloc.h>

PrDmaQueue::PrDmaQueue(u_int size) {
    m_size  = size;
    m_queue = (PrDmaList*)memalign(128, (size + 1) * sizeof(PrDmaList));

    Initialize();
    FlushCache(WRITEBACK_DCACHE);

    m_started = false;
    m_pos = 0;

    m_queue = (PrDmaList*)PR_UNCACHEDACCEL(m_queue);
}

PrDmaQueue::~PrDmaQueue() {
    if (m_queue != NULL) {
        free(PR_DECACHE(m_queue));
    }
}

void PrDmaQueue::Initialize() {
    for (int i = 0; i <= m_size; i++) {
        PrDmaList *queue = &m_queue[i];

        /*
         * Set up a REFS DMAtag that will
         * stall the queue until there is
         * data appended to the CALL DMAtag.
         */
        queue->stall_qw[0] = 0;
        queue->stall_qw[1] = 0;
        queue->stall_qw[2] = 0;
        queue->stall_qw[3] = 0;

        queue->stall_tag.qwc = 1;
        queue->stall_tag.mark = 0;
        queue->stall_tag.id = PR_DMA_TAG_REFS;
        queue->stall_tag.next = (sceDmaTag*)&queue->stall_qw;
        queue->stall_tag.p[0] = 0;
        queue->stall_tag.p[1] = 0;

        /*
         * Set up a CALL DMAtag to
         * jump to the appended tag.
         *
         * For the last tag, use a RET DMAtag
         * to return to the queue and continue.
         */
        queue->call_tag.qwc = 0;
        queue->call_tag.mark = 0;
        queue->call_tag.id = PR_DMA_TAG_CALL;
        queue->call_tag.p[0] = 0;
        queue->call_tag.p[1] = 0;

        /*
         * Jump to the next list in the queue.
         */
        queue->next_tag.qwc = 0;
        queue->next_tag.mark = 0;
        queue->next_tag.id = PR_DMA_TAG_NEXT;
        queue->next_tag.next = &((queue + 1)->stall_tag);
        queue->next_tag.p[0] = 0;
        queue->next_tag.p[1] = 0;
    }
}

void PrDmaQueue::Start() {
    m_pos = 0;
    PrWaitDmaFinish(SCE_DMA_VIF1);

    /*
     * Enable stall control on VIF1 ch.
     * and set the stall address to the
     * start of the queue.
     */
    *D_CTRL |= 0x40;
    *D_STADR = (u_int)PR_DECACHE(&m_queue[0].stall_qw);

    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_VIF1);
    chan->chcr.TTE = 1;

    sceDmaSend(chan, PR_DECACHE(&m_queue[0].stall_tag));
    m_started = true;
}

void PrDmaQueue::Append(void *tag) {
    static bool warned = false;

    if (m_pos == m_size) {
        if (!warned) {
        #if 0 /* (poly): Only present on McDonald's Demo build */
            printf("PRLIB(WARN): DMA queue ovewrflow\n");
            printf("PRLIB(WARN): you must initialize the queue with more size\n");
        #endif
            warned = true;
        }

        Wait();
        Start();
    }

    /*
     * Append the DMAtag to
     * the queue's call tag.
     */
    m_queue[m_pos].call_tag.next = (sceDmaTag*)tag;
    asm("sync.l");

    /*
     * Update the stall address to
     * the next list in the queue.
     *
     * This will resume the VIF1 transfer
     * and continue until it reaches the
     * next stall address.
     */
    m_pos++;
    PrDmaList *queue = &m_queue[m_pos - 1];
    *D_STADR = (u_int)PR_DECACHE(queue + 1);
}

void PrDmaQueue::Wait() {
    m_queue[m_pos].call_tag.id = PR_DMA_TAG_END;
    m_queue[m_pos].call_tag.next = NULL;
    asm("sync.l");

    PrDmaList *queue = &m_queue[m_pos - 1];
    *D_STADR = (u_int)PR_DECACHE(queue + 2);

    PrWaitDmaFinish(SCE_DMA_VIF1);
    *D_CTRL &= ~0xc0; /* Disable stall control. */

    /*
     * Restore the original tag type
     * after stopping the queue.
     */
    m_queue[m_pos].call_tag.id = PR_DMA_TAG_CALL;
    m_started = false;
}
