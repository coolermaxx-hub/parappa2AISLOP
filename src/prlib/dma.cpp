#include "dma.h"

#include <eeregs.h>

/* Volatile channel control type. */
#define PR_VCHCR volatile tD_CHCR

void PrWaitDmaFinish(u_int channel) {
    static PR_VCHCR *chcr_table[] = {
        (tD_CHCR*)D0_CHCR, /* VIF0    */
        (tD_CHCR*)D1_CHCR, /* VIF1    */
        (tD_CHCR*)D2_CHCR, /* GIF     */
        (tD_CHCR*)D3_CHCR, /* fromIPU */
        (tD_CHCR*)D4_CHCR, /* toIPU   */
        (tD_CHCR*)D5_CHCR, /* SIF0    */
        (tD_CHCR*)D6_CHCR, /* SIF1    */
        (tD_CHCR*)D7_CHCR, /* SIF2    */
        (tD_CHCR*)D8_CHCR, /* fromSPR */
        (tD_CHCR*)D9_CHCR, /* toSPR   */

        /*
         * note: These NULL elements are only
         * needed because of the next symbol being
         * 16-byte aligned (NaGifPacket::s_work).
         */
        NULL,
        NULL,
    };

    PR_VCHCR *chcr = chcr_table[channel];

    /*
     * No COP0 polling for the
     * scratchpad channels (but why?).
     */
    if (channel >= 8) {
        while (chcr->STR & 1);
        return;
    }

    /*
     * This method of waiting for the DMA transfer to finish
     * is more efficient than the regular channel STR field polling
     * as it does not use the main bus.
     *
     * It consists of the following:
     *   1. Clear the DMA channel interrupt status (D_STAT.CISn).
     *   2. Save the priority control register (D_PCR).
     *   3. Set the appropriate COP control fields (D_PCR.CPCn).
     *       * This will start outputting the DMA channel status to CPCOND0.
     *   4. Run an empty loop with the BC0F instruction.
     *       * The BC0xx family of instructions check the CPCOND0 signal.
     *       * When the DMA transfer finishes, the loop will break.
     *   5. Restore the previously saved PCR register and
     *      clear the DMA channel interrupt status.
     */
    *D_STAT = PR_BIT(channel);
    if (chcr->STR & 1) {
        u_int pcr = *D_PCR;
        u_int pcr_old = pcr;
        
        pcr &= ~0x3ff; /* clear CPC0-CPC9: only this channel drives CPCOND0 */
        pcr |= PR_BIT(channel);

        *D_PCR = pcr;
        asm("sync.l");

        asm volatile(
        "cpcond0_wait_loop:         \n\t"
        "    bc0f cpcond0_wait_loop \n\t"
        "    nop                    \n\t"
        );

        *D_STAT = PR_BIT(channel);
        *D_PCR = pcr_old;
    }
}
