#include "main/cdctrl.h"

#include "os/mtc.h"
#include "os/syssub.h"
#include "os/tim2.h"
#include "os/usrmem.h"

#include "main/etc.h"
#include "main/p3str.h"

#include "iop_mdl/tapctrl_rpc.h"
#include "iop_mdl/wp2cd_rpc.h"

#include <eeregs.h>
#include <sifdev.h>
#include <stdio.h>

void *current_intg_adrs = NULL;

#define CD_SECTOR_SIZE 2048

#define N        4096 /* Size of ring buffer */
#define F          18 /* Upper limit */
#define THRESHOLD   2
#define LZ_FLAG_REFILL 0xff00 /* eight pending flag bits, refilled with the next control byte */
#define LZ_FLAG_EMPTY  256

/* Background decodes yield to the next frame once TIMER0 (H-blanks since the
 * last VBlank, reset in osFunc) passes this line, near the end of the field. */
#define DECODE_WAIT_HLINE 230

static unsigned char RBuff[N + F - 1]; /* Ring buffer for INT decompression */

static int cdctrlReadSub(FILE_STR *fstr_pp, int ofs, int size, int buf);
static void cdctrlReadDataOne(void *x);
static void cdctrlSndFadeOut(void *x);

u_int PackIntGetDecodeSize(u_char *fp_r) {
    return *(u_int*)fp_r;
}

int PackIntDecode(u_char *fp_r, u_char *fp_w) {
    u_int        moto_size;     /* Decode size                                  */
    int          rp    = N - F; /* Ring buffer position                         */

    unsigned int flags = 0;     /* LZSS flags                                   */
    u_char      *fp_w_end;      /* End of write buffer (buffer + decode size)   */

    int          i, c, c1, c2;
    
    asm volatile("sync.l");

    moto_size = *(u_int*)fp_r;
    fp_w = (u_char*)PR_UNCACHED(fp_w);
    fp_w_end = fp_w + moto_size;

    fp_r += 8;

    for (i = 0; i < N - F; i++) {
        RBuff[i] = 0;
    }

    while (fp_w != fp_w_end) {
        if (((flags >>= 1) & LZ_FLAG_EMPTY) == 0) {
            c = *fp_r++;
            flags = c | LZ_FLAG_REFILL;
        }

        if (flags & 1) {
            c = *fp_r++;
            *fp_w++ = c;

            RBuff[rp++] = c;
            rp &= (N - 1);
        } else {
            c1 = *fp_r++;
            c2 = *fp_r++;

            c1 |= ((c2 & 0xf0) << 4);
            c2 = (c2 & 0x0f) + THRESHOLD;

            for (i = 0; i <= c2; i++) {
                c = RBuff[(c1 + i) & (N - 1)];
                *fp_w++ = c;
                
                RBuff[rp++] = c;
                rp &= (N - 1);
            }
        }
    }

    asm volatile("sync.l");
    return 0;
}

int PackIntDecodeWait(u_char *fp_r, u_char *fp_w, int wait_hline) {
    u_int        moto_size;     /* Decode size                                  */
    u_char      *fp_w_end;      /* End of write buffer (buffer + decode size)   */

    int          rp    = N - F; /* Ring buffer position                         */
    unsigned int flags = 0;     /* LZSS flags                                   */

    int          i, c, c1, c2;
    
    printf("decode moto[%08x] saki[%08x]\n", (u_int)fp_r, (u_int)fp_w);
    asm volatile("sync.l");

    moto_size = *(u_int*)fp_r;
    fp_w = (u_char*)PR_UNCACHED(fp_w);
    fp_w_end = fp_w + moto_size;

    fp_r += 8;

    for (i = 0; i < N - F; i++) {
        RBuff[i] = 0;
    }

    while (1) {
        if (fp_w > fp_w_end) {
            printf(" over data pointer\n");
            break;
        }
        if (fp_w == fp_w_end) {
            break;
        }

        if (wait_hline < *T0_COUNT) {
            MtcWait(1);
        }

        if (((flags >>= 1) & LZ_FLAG_EMPTY) == 0) {
            c = *fp_r++;
            flags = c | LZ_FLAG_REFILL;
        }

        if (flags & 1) {
            c = *fp_r++;
            *fp_w++ = c;

            RBuff[rp++] = c;
            rp &= (N - 1);
        } else {
            c1 = *fp_r++;
            c2 = *fp_r++;

            c1 |= ((c2 & 0xf0) << 4);
            c2 = (c2 & 0x0f) + THRESHOLD;

            for (i = 0; i <= c2; i++) {
                c = RBuff[(c1 + i) & (N - 1)];
                *fp_w++ = c;
                
                RBuff[rp++] = c;
                rp &= (N - 1);
            }
        }
    }

    asm volatile("sync.l");
    return 0;
}

static int cdSampleTmp;
static CDCTRL_STR cdctrl_str;

void CdctrlInit(void) {
    WorkClear(&cdctrl_str, sizeof(cdctrl_str));

    printf("in wp2 init\n");
    WP2Init();
    printf("out wp2 init\n");

    WP2Ctrl(WP2_CDINIT, SCECdDVD);
    WP2Ctrl(WP2_SDINIT, WP2_NONE);
    WP2Ctrl(WP2_SETMASTERVOL, SPU_VOLUME_LR(SPU_VOLUME_MAX, SPU_VOLUME_MAX));

    /*
     * WP2 init--allocate 384KB for the read
     * buffer in 768 blocks of 512 bytes each.
     */
    WP2Ctrl(WP2_INIT, 0x300);
}

void CdctrlQuit(void) {
    WorkClear(&cdctrl_str, sizeof(cdctrl_str));
    WP2Ctrl(WP2_QUIT, WP2_NONE);
}

void CdctrlMasterVolSet(u_int vol) {
    WP2Ctrl(WP2_SETMASTERVOL, SPU_VOLUME_LR(vol, vol));
}

int CdctrlSerch(FILE_STR *fstr_pp) {
    int ret = 0;

    if (fstr_pp->search) {
        return 1;
    }

    if (fstr_pp->frmode == FRMODE_CD) {
        /* Search file from CD */
        ret = sceCdSearchFile(&fstr_pp->fpCd, fstr_pp->fname);
        if (ret) {
            fstr_pp->search = 1;
        } else {
            printf("CD SEARCH ERROR!![%s]\n", fstr_pp->fname);
        }
    } else {
        /* Search file from PC */
        int rfd;
        int readsize;

        rfd = sceOpen(fstr_pp->fname, SCE_RDONLY);
        if (rfd < 0) {
            printf("Can\'t open %s\n", fstr_pp->fname);
        } else {
            readsize = sceLseek(rfd, 0, SCE_SEEK_END);
            if (readsize < 0) {
                printf("Can\'t open %s\n", fstr_pp->fname);
            } else {
                fstr_pp->fpCd.size = readsize;
                fstr_pp->search = 1;

                ret = 1;
            }
        
            sceClose(rfd);
        }
    }

    return ret;
}

static int cdctrlReadSub(FILE_STR *fstr_pp, int ofs, int size, int buf) {
    if (fstr_pp->frmode == FRMODE_CD) {
        sceCdRMode cdmode   = { .spindlctrl = SCECdSpinNom };
        int        read_lsn = fstr_pp->fpCd.lsn + (ofs / CD_SECTOR_SIZE);

        if (!sceCdSeek(read_lsn)) {
            printf("sceCdSeek Error! %s\n", fstr_pp->fname);
            return 0;
        } 

        while (sceCdSync(1)) {
            MtcWait(1);
        }

        if (!sceCdRead(read_lsn, (size + CD_SECTOR_SIZE - 1) / CD_SECTOR_SIZE, (void*)buf, &cdmode)) {
            printf("sceCdRead Error! %s\n", fstr_pp->fname);
            return 0;
        }

        while (sceCdSync(1)) {
            MtcWait(1);
        }

        if (sceCdGetError()) {
            return 0;
        }
            
        FlushCache(WRITEBACK_DCACHE);
        return 1;
    } else {
        int rfd = sceOpen(fstr_pp->fname, SCE_RDONLY);
        if (rfd < 0) {
            printf("Can\'t open %s\n", fstr_pp->fname);
            return 0;
        }

        if (sceLseek(rfd, ofs, SCE_SEEK_SET) < 0) {
            printf("Can\'t open %s\n", fstr_pp->fname);
            sceClose(rfd);
            return 0;
        }

        if (sceRead(rfd, (void*)buf, size) < 0) {
            printf("read error!! \n");
            sceClose(rfd);
            return 0;
        }

        sceClose(rfd);
        FlushCache(WRITEBACK_DCACHE);
        return 1;
    }

    return 0;
}

void intReadSub(void) {
    int     read_pos;
    u_char *read_tmp_pp;
    u_char *head_read_pp;

    read_pos = 0;
    head_read_pp = usrMalloc(8192);

    printf("intg in!!!\n");

    while (1) {
        /* Read a whole sector into our header buffer */
        while (!cdctrlReadSub(cdctrl_str.fstr_pp, read_pos, CD_SECTOR_SIZE, (int)head_read_pp)) {
            MtcWait(1);
        }

        /* Check if we loaded a valid INT */
        if (PACK(head_read_pp)->id != PACKINT_MAGIC) {
            printf("INT FILE ERROR!![%s]\n", cdctrl_str.fstr_pp->fname);
            while (1) {
                MtcWait(1);
            }
        }

        FlushCache(WRITEBACK_DCACHE);

        if (PACK(head_read_pp)->head_size > CD_SECTOR_SIZE) {
            while (!cdctrlReadSub(cdctrl_str.fstr_pp, read_pos + CD_SECTOR_SIZE, ((PACK(head_read_pp)->head_size - 1) / CD_SECTOR_SIZE) * CD_SECTOR_SIZE, (int)(head_read_pp + CD_SECTOR_SIZE))) {
                MtcWait(1);
            }
        }

        read_pos += PACK(head_read_pp)->head_size + PACK(head_read_pp)->name_size;
        FlushCache(WRITEBACK_DCACHE);

        /* Check if there's data present */
        if (PACK(head_read_pp)->data_size != 0) {
            /* If we don't have a temp buffer, allocate it ourselves */
            if (cdctrl_str.tmp_area != NULL) {
                read_tmp_pp = (u_char*)(cdctrl_str.tmp_area - PACK(head_read_pp)->data_size);
                printf("buf use:%08x size:%08x\n", cdctrl_str.tmp_area - PACK(head_read_pp)->data_size, PACK(head_read_pp)->data_size);
            } else {
                read_tmp_pp = usrMalloc(PACK(head_read_pp)->data_size);
            }

            /* Read the data... */
            FlushCache(WRITEBACK_DCACHE);
            while (!cdctrlReadSub(cdctrl_str.fstr_pp, read_pos, PACK(head_read_pp)->data_size, (int)read_tmp_pp)) {
                MtcWait(1);
            }

            /* ...and decode it */
            FlushCache(WRITEBACK_DCACHE);
            PackIntDecodeWait(read_tmp_pp, (u_char*)UsrMemAllocNext(), DECODE_WAIT_HLINE);
            FlushCache(WRITEBACK_DCACHE);

            if (cdctrl_str.tmp_area == NULL) {
                usrFree(read_tmp_pp);
            }

            read_pos += PACK(head_read_pp)->data_size;
        }

        /* Handle the decoded data */
        switch (PACK(head_read_pp)->ftype) {
        /* TIM2 textures */
        case FT_VRAM: {
            int i;
            printf("int file tim2 file in\n");

            for (i = 0; i < PACK(head_read_pp)->fnum; i++) {
                Tim2Trans((void*)(PACK(head_read_pp)->adr[i] + UsrMemAllocNext()));
            }

            printf("int file tim2 file out\n");
            break;
        }
        /* Sounds */
        case FT_SND: {
            int i;
            printf("int file snd file in\n");

            while (TapCt(TAPCT_TRANSCHECK, TAPCT_NONE, TAPCT_NONE)) {
                MtcWait(1);
            }

            for (i = 0; i < (PACK(head_read_pp)->fnum / 2); i++) {
                TapCt(TAPCT_BDSPUTRANS | i, PACK(head_read_pp)->adr[i] + UsrMemAllocNext(),
                    PACK(head_read_pp)->adr[i + 1] - PACK(head_read_pp)->adr[i]);
                TapCt(TAPCT_HDIOPTRANS | i, PACK(head_read_pp)->adr[i + (PACK(head_read_pp)->fnum / 2)] + UsrMemAllocNext(),
                    PACK(head_read_pp)->adr[i + 1 + (PACK(head_read_pp)->fnum / 2)] - PACK(head_read_pp)->adr[i + (PACK(head_read_pp)->fnum / 2)]);
            }

            while (TapCt(TAPCT_TRANSCHECK, TAPCT_NONE, TAPCT_NONE)) {
                MtcWait(1);
            }

            printf("int file snd file out\n");
            break;
        }
        /* PaRappa's hat textures (TIM2) */
        case FT_R1:
        case FT_R2:
        case FT_R3:
        case FT_R4: {
            int i;

            if (GetHatRound() == (PACK(head_read_pp)->ftype - 4)) {
                printf("int file tim2 round:%d file in\n", PACK(head_read_pp)->ftype - 3);

                for (i = 0; i < PACK(head_read_pp)->fnum; i++) {
                    Tim2Trans((void*)(PACK(head_read_pp)->adr[i] + UsrMemAllocNext()));
                }

                printf("int file tim2 roud file out\n");
            }

            break;
        }
        /* Models, animations, etc. */
        case FT_ONMEM: {
            int i;
            printf("mem cnt [%d]\n", PACK(head_read_pp)->fnum);

            for (i = 0; i < PACK(head_read_pp)->fnum; i++) {
                UsrMemAlloc(PACK(head_read_pp)->adr[i + 1] - PACK(head_read_pp)->adr[i]);
            }
        }
        /* Fallthrough, stops after the ONMEM file category */
        default:
            printf("intg exit!!\n");
            usrFree(head_read_pp);
            return;
        }
    }
}

void cdctrlReadData(void *x) {
    while (!CdctrlSerch(cdctrl_str.fstr_pp)) {
        MtcWait(1);
    }

    switch (cdctrl_str.fstr_pp->ftmode) {
    case FTMODE_INTG:
        intReadSub();
        current_intg_adrs = (void*)cdctrl_str.read_area;
        break;
    case FTMODE_ETC:
        while (!cdctrlReadSub(cdctrl_str.fstr_pp, 0, cdctrl_str.fstr_pp->fpCd.size, cdctrl_str.read_area)) {
            MtcWait(1);
        }
        break;
    }

    FlushCache(WRITEBACK_DCACHE);
    MtcWait(1);

    cdctrl_str.status = 0;
    MtcExit();
}

static void cdctrlReadDataOne(void *x) {
    while (!CdctrlSerch(cdctrl_str.fstr_pp)) {
        MtcWait(1);
    }

    while (!cdctrlReadSub(cdctrl_str.fstr_pp, 0, cdctrl_str.fstr_pp->fpCd.size, cdctrl_str.read_area)) {
        MtcWait(1);
    }

    MtcWait(1);
    cdctrl_str.status = 0;
    MtcExit();
}

void CdctrlRead(FILE_STR *fstr_pp, u_int buf, int tmpbuf) {
    cdctrl_str.status = 1;
    cdctrl_str.error_status = 0;
    cdctrl_str.read_area = buf;
    cdctrl_str.tmp_area = tmpbuf;
    cdctrl_str.fstr_pp = fstr_pp;
    MtcExec(cdctrlReadData, MTC_TASK_CDCTRL);
}

void CdctrlReadOne(FILE_STR *fstr_pp, u_int buf, int tmpbuf) {
    cdctrl_str.status = 1;
    cdctrl_str.error_status = 0;
    cdctrl_str.read_area = buf;
    cdctrl_str.tmp_area = tmpbuf;
    cdctrl_str.fstr_pp = fstr_pp;
    MtcExec(cdctrlReadDataOne, MTC_TASK_CDCTRL);
}

/* さき(saki) -> Destination / もと(moto) -> Source */
void usrMemcpy(void *sakip, void *motop, int size) {
    int     i = size / sizeof(int);
    int *m_pp = motop;
    int *s_pp = sakip;

    while (--i != -1) {
        *s_pp++ = *m_pp++;
    }
}

void CdctrlMemIntgDecode(u_int rbuf, u_int setbuf) {
    u_int   next_rp;
    u_char *head_read_pp;

    current_intg_adrs = (void*)setbuf;
    next_rp           = rbuf;
    head_read_pp      = usrMalloc(8192);

    while (1) {
        /* Copy the INT to our buffer */
        usrMemcpy(head_read_pp, (void*)next_rp, PACK(next_rp)->head_size);
        FlushCache(WRITEBACK_DCACHE);
        
        /* Check if we loaded a valid INT */
        if (PACK(head_read_pp)->id != PACKINT_MAGIC) {
            printf("INT FILE ERROR!![%s]\n", cdctrl_str.fstr_pp->fname);
            while (1) {
                MtcWait(1);
            }
        }

        next_rp += PACK(head_read_pp)->head_size + PACK(head_read_pp)->name_size;
        FlushCache(WRITEBACK_DCACHE);

        /* Check if there's data present */
        if (PACK(head_read_pp)->data_size != 0) {
            /* Decode the data */
            PackIntDecodeWait((u_char*)next_rp, (u_char*)UsrMemAllocNext(), DECODE_WAIT_HLINE);
            next_rp += PACK(head_read_pp)->data_size;
        }

        FlushCache(WRITEBACK_DCACHE);

        /* Handle the decoded data */
        switch (PACK(head_read_pp)->ftype) {
        /* TIM2 textures */
        case FT_VRAM: {
            int i;
            printf("tim trans in\n");

            for (i = 0; i < PACK(head_read_pp)->fnum; i++) {
                Tim2Trans((void*)(PACK(head_read_pp)->adr[i] + UsrMemAllocNext()));
            }

            printf("tim trans out\n");
            break;
        }
        /* Sounds */
        case FT_SND: {
            int i;

            for (i = 0; i < (PACK(head_read_pp)->fnum / 2); i++) {
                TapCt(TAPCT_BDSPUTRANS | i, PACK(head_read_pp)->adr[i] + UsrMemAllocNext(),
                    PACK(head_read_pp)->adr[i + 1] - PACK(head_read_pp)->adr[i]);
                TapCt(TAPCT_HDIOPTRANS | i, PACK(head_read_pp)->adr[i + (PACK(head_read_pp)->fnum / 2)] + UsrMemAllocNext(),
                    PACK(head_read_pp)->adr[i + 1 + (PACK(head_read_pp)->fnum / 2)] - PACK(head_read_pp)->adr[i + (PACK(head_read_pp)->fnum / 2)]);
            }

            while (TapCt(TAPCT_TRANSCHECK, TAPCT_NONE, TAPCT_NONE)) {
                MtcWait(1);
            }
            break;
        }
        /* PaRappa's hat textures (TIM2) */
        case FT_R1:
        case FT_R2:
        case FT_R3:
        case FT_R4: {
            int i;

            if (GetHatRound() == PACK(head_read_pp)->ftype - 4) {
                printf("int file tim2 round:%d file in\n", PACK(head_read_pp)->ftype - 3);

                for (i = 0; i < PACK(head_read_pp)->fnum; i++) {
                    Tim2Trans((void*)(PACK(head_read_pp)->adr[i] + UsrMemAllocNext()));
                }

                printf("int file tim2 roud file out\n");
            }

            break;
        }
        /* Models, animations, etc. */
        case FT_ONMEM: {
            int i;
            printf("mem cnt [%d]\n", PACK(head_read_pp)->fnum);

            for (i = 0; i < PACK(head_read_pp)->fnum; i++) {
                UsrMemAlloc(PACK(head_read_pp)->adr[i+1] - PACK(head_read_pp)->adr[i]);
            }

            break;
        }
        default:
            printf("intg exit!!\n");
            usrFree(head_read_pp);
            return;
        }
    }
}

int CdctrlStatus(void) {
    return cdctrl_str.status;
}

void CdctrlReadWait(void) {
    do {
        MtcWait(1);
    } while (CdctrlStatus());  
}

u_int CdctrlGetFileSize(FILE_STR *fstr_pp) {
    while (!CdctrlSerch(fstr_pp)) {
        MtcWait(1);
    }

    return fstr_pp->fpCd.size;
}

void CdctrlWP2SetChannel(u_char Lchan, u_char Rchan) {
    if (cdctrl_str.wp2chan[0] == Lchan &&
        cdctrl_str.wp2chan[1] == Rchan) {
        return;
    }

    WP2Ctrl(WP2_SETCHANNEL, PR_CONCAT(Lchan, Rchan));
    cdctrl_str.wp2chan[0] = Lchan;
    cdctrl_str.wp2chan[1] = Rchan;

    printf("channel change L[%d] R[%d]\n", Lchan, Rchan);
}

void CdctrlWP2Set(FILE_STR *fstr_pp) {
    cdctrl_str.status = 1;
    cdctrl_str.error_status = 0;
    cdctrl_str.fstr_pp = fstr_pp;
    
    WP2Ctrl(WP2_SETMODE, fstr_pp->mchan);

    while (!CdctrlSerch(cdctrl_str.fstr_pp));

    WP2Ctrl(WP2_SETTRPOINT, WP2_NONE);
    if (fstr_pp->frmode == FRMODE_PC) {
        WP2Ctrl(WP2_OPENFLOC, (int)cdctrl_str.fstr_pp->fname);
    } else {
        WP2Ctrl(WP2_OPENFLOC, (int)&cdctrl_str.fstr_pp->fpCd);
    }

    CdctrlWP2SetVolume(0);
    
    cdctrl_str.read_area = WP2Ctrl(WP2_GETTIMESAMPLE, 0);
    cdctrl_str.wp2chan[0] = 0;
    cdctrl_str.wp2chan[1] = 1;

    WP2Ctrl(WP2_SETCHANNEL, PR_CONCAT(cdctrl_str.wp2chan[0], cdctrl_str.wp2chan[1]));
    WP2Ctrl(WP2_PRELOADBACK, WP2_NONE);
}

void CdctrlWP2SetFileSeek(FILE_STR *fstr_pp, int seek_pos) {
    /* Stop the current WP2 playing */
    CdctrlWP2SetVolume(0);
    WP2Ctrl(WP2_STOP, WP2_NONE);

    if (cdctrl_str.fstr_pp != fstr_pp) {
        cdctrl_str.fstr_pp = fstr_pp;
        printf("!!max chan [%d]\n", fstr_pp->mchan);

        WP2Ctrl(WP2_SETMODE, cdctrl_str.fstr_pp->mchan);
        if (fstr_pp->frmode == FRMODE_PC) {
            WP2Ctrl(WP2_SEEKFLOC, (int)cdctrl_str.fstr_pp->fname);
        } else {
            WP2Ctrl(WP2_SEEKFLOC, (int)&cdctrl_str.fstr_pp->fpCd);
        }

        cdctrl_str.wp2chan[1] = 0;
        cdctrl_str.wp2chan[0] = 0;
    }

    /* Seek and preload */
    WP2Ctrl(WP2_SEEK, seek_pos);
    WP2Ctrl(WP2_PRELOADBACK, WP2_NONE);
}

void CdctrlWP2SetFileSeekChan(FILE_STR *fstr_pp, int seek_pos, u_char Lch, u_char Rch) {
    /* Stop the current WP2 playing */
    CdctrlWP2SetVolume(0);
    WP2Ctrl(WP2_STOP, WP2_NONE);

    cdctrl_str.fstr_pp = fstr_pp;
    printf("!!max chan [%d]\n", fstr_pp->mchan);

    WP2Ctrl(WP2_SETMODE, cdctrl_str.fstr_pp->mchan);
    if (fstr_pp->frmode == FRMODE_PC) {
        WP2Ctrl(WP2_SEEKFLOC, (int)cdctrl_str.fstr_pp->fname);
    } else {
        WP2Ctrl(WP2_SEEKFLOC, (int)&cdctrl_str.fstr_pp->fpCd);
    }

    cdctrl_str.wp2chan[1] = 0;
    cdctrl_str.wp2chan[0] = 0;

    /* Seek, set the channels, and preload */
    WP2Ctrl(WP2_SEEK, seek_pos);
    CdctrlWP2SetChannel(Lch, Rch);
    WP2Ctrl(WP2_PRELOADBACK, WP2_NONE);
}

void CdctrlWP2Seek(int seek_pos) {
    WP2Ctrl(WP2_STOP, WP2_NONE);
    WP2Ctrl(WP2_SEEK, seek_pos);
    WP2Ctrl(WP2_PRELOAD, WP2_NONE);
}

void CdctrlWP2Play(void) {
    WP2Ctrl(WP2_START, WP2_NONE);
}

int CdctrlWP2PlayEndCheck(void) {
    return WP2Ctrl(WP2_GETMODE, WP2_NONE) & WP2_MODE_TERMINATE;
}

int CdctrlWP2CheckBuffer(void) {
    return WP2Ctrl(WP2_READBUFF, WP2_NONE);
}

void CdctrlWP2SetVolume(u_short vol) {
    cdctrl_str.volume = vol;

    /* Scale to the 16-bit volume WP2 takes, clamped to 0x7fff, for both channels. */
    vol *= 256;
    if ((short)vol < 0) {
        vol = 0x7fff;
    }

    WP2Ctrl(WP2_SETVOLDIRECT, SPU_VOLUME_LR(vol, vol));
}

u_short CdctrlWP2GetVolume(void) {
    return cdctrl_str.volume;
}

void CdctrlWp2Stop(void) {
    WP2Ctrl(WP2_STOP, WP2_NONE);
}

void CdctrlWp2FileEnd(void) {
    WP2Ctrl(WP2_STOP, WP2_NONE);
    WP2Ctrl(WP2_CLOSE, WP2_NONE);
}

/* WP2 time is counted in 256-sample blocks of the 48 kHz stream, 187.5 a
 * second: frames = units * 24 / 75, ticks = units * tempo * 16 / 1875. */
int CdctrlWp2GetSample(void) {
    return WP2Ctrl(WP2_GETTIME, WP2_NONE);
}

int CdctrlWp2GetFrame(void) {
    return (WP2Ctrl(WP2_GETTIME, WP2_NONE) * 24) / 75;
}

int CdctrlFrame2WP2sample(int frame) {
    return (frame * 75) / 24;
}

long CdctrlWp2GetSndTime(float tempo) {
    float beat = WP2Ctrl(WP2_GETTIME, WP2_NONE);

    beat = (tempo * 16.0f * beat) / 1875.0f;
    return (int)beat;
}

long CdctrlSndTime2WP2sample(float tempo, long beat) {
    return (beat * 1875.0f) / (tempo * 16.0f);
}

void CdctrlWp2GetSampleTmpBuf(void) {
    static int time_tmp_max = 0;
    int time_tmp = *T0_COUNT;

    cdSampleTmp = WP2Ctrl(WP2_GETTIME, WP2_NONE);
    time_tmp = *T0_COUNT - time_tmp;

    if (time_tmp > time_tmp_max) {
        printf("max cd time get[%d]\n", time_tmp);
        time_tmp_max = time_tmp;
    }
}

int CdctrlWp2GetSampleTmp(void) {
    return cdSampleTmp;
}

int CdctrlWp2GetFrameTmp(void) {
    int frame = cdSampleTmp;
    return (frame * 24) / 75;
}

long CdctrlWp2GetSndTimeTmp(float tempo) {
    float beat = cdSampleTmp;

    beat = (beat * tempo * 16.0f) / 1875.0f;
    return (int)beat;
}

long CdctrlWp2CdSample2SndTime(long samplecnt, float tempo) {
    float beat = samplecnt;

    beat = (beat * tempo * 16.0f) / 1875.0f;
    return (int)beat;
}

long CdctrlWp2CdSample2Frame(long samplecnt) {
    return (samplecnt * 24) / 75;
}

void CdctrlXTRset(FILE_STR *fstr_pp, u_int usebuf) {
    int seek_pos;

    cdctrl_str.status = 1;
    cdctrl_str.error_status = 0;
    cdctrl_str.fstr_pp = fstr_pp;

    /* Search the XTR file */
    while (!CdctrlSerch(cdctrl_str.fstr_pp));

    /* Read the XTR's header (1 sector) */
    while (!cdctrlReadSub(fstr_pp, 0, CD_SECTOR_SIZE, usebuf)) {
        MtcWait(1);
    }
    
    FlushCache(WRITEBACK_DCACHE);

    if (STR(usebuf)->read_size > CD_SECTOR_SIZE) {
        while (!cdctrlReadSub(fstr_pp, CD_SECTOR_SIZE, STR(usebuf)->read_size - CD_SECTOR_SIZE, (int)(usebuf + CD_SECTOR_SIZE))) {
            MtcWait(1);
        }
    }

    FlushCache(WRITEBACK_DCACHE);

    if (STR(usebuf)->channel != 0) {
        cdctrl_str.fstr_pp->mchan = STR(usebuf)->channel;
    } else {
        cdctrl_str.fstr_pp->mchan = 6;
    }

    WP2Ctrl(WP2_SETMODE, cdctrl_str.fstr_pp->mchan);
    FlushCache(WRITEBACK_DCACHE);

    /* Read and decode each compressed transfer box into place. */
    {
        int       i;
        TRBOX_TR *tb_pp;
        u_char   *pr_pp;

        tb_pp = STR(usebuf)->trbox_tr;
        for (i = 0; i < PR_ARRAYSIZE(STR(usebuf)->trbox_tr); i++, tb_pp++) {
            if (tb_pp->press_size == 0) {
                continue;
            }

            pr_pp = (u_char*)UsrMemEndAlloc(tb_pp->press_size);
            UsrMemEndFree();
            FlushCache(WRITEBACK_DCACHE);

            while (!cdctrlReadSub(fstr_pp, tb_pp->read_pos, tb_pp->press_size, (int)pr_pp)) {
                MtcWait(1);
            }

            FlushCache(WRITEBACK_DCACHE);

            printf("dec size[%08x]\n", PackIntGetDecodeSize(pr_pp));
            printf("dec info trpos[%08x] read_pos[%08x] press_size[%08x]\n", tb_pp->trpos, tb_pp->read_pos, tb_pp->press_size);

            PackIntDecodeWait(pr_pp, (u_char*)(tb_pp->trpos + usebuf), DECODE_WAIT_HLINE);
        }
    }

    FlushCache(WRITEBACK_DCACHE);
    
    seek_pos = STR(usebuf)->seek;

    printf("P3STR INIT REQ IN\n");
    p3StrInit(usebuf);
    printf("P3STR INIT REQ OUT\n");

    WP2Ctrl(WP2_SETTRPOINT, usebuf);
    printf("WP2_BGMSETTRPOINT exit\n");

    if (fstr_pp->frmode == FRMODE_PC) {
        WP2Ctrl(WP2_OPENFLOC, (int)cdctrl_str.fstr_pp->fname);
    } else {
        WP2Ctrl(WP2_OPENFLOC, (int)&cdctrl_str.fstr_pp->fpCd);
    }

    WP2Ctrl(WP2_SEEK, seek_pos);
    printf("WP2_SEEK exit\n");

    WP2Ctrl(WP2_SETCHANNEL, PR_CONCAT(0, 1));
    WP2Ctrl(WP2_PRELOADBACK, WP2_NONE);
    printf("WP2_PRELOAD exit\n");

    CdctrlWP2SetVolume(0);

    cdctrl_str.read_area  = WP2Ctrl(WP2_GETTIMESAMPLE, WP2_NONE);
    cdctrl_str.wp2chan[0] = 0;
    cdctrl_str.wp2chan[1] = 1;
}

void* GetIntAdrsCurrent(u_short num) {
    return (void*)UsrMemGetAdr(num);
}

int GetIntSizeCurrent(u_short num) {
    return UsrMemGetSize(num);
}

static int sndFadeTime = 0;

static void cdctrlSndFadeOut(void *x) {
    int timer;

    for (timer = 0; timer < sndFadeTime; timer++) {
        CdctrlMasterVolSet(((sndFadeTime - timer) * SPU_VOLUME_MAX) / sndFadeTime);
        MtcWait(1);
    }

    sndFadeTime = 0;
    MtcExit();
}

void CdctrlSndFadeOut(int time) {
    sndFadeTime = time;
    MtcExec(cdctrlSndFadeOut, MTC_TASK_CDCTRL);
}

int CdctrlSndFadeOutCheck(void) {
    return (sndFadeTime == 0);
}

void CdctrlSndFadeOutWait(int time) {
    CdctrlSndFadeOut(time);
    
    while (!CdctrlSndFadeOutCheck()) {
        MtcWait(1);
    }
}
