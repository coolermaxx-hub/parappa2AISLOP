#include "main/pack.h"

#include <stdio.h>

#define N        4096 /* Size of ring buffer */
#define F          18 /* Upper limit */
#define THRESHOLD   2

static unsigned char RBuff[N + F - 1];

u_int PackGetDecodeSize(u_char *fp_r) {
    return *(u_int*)fp_r;
}

int PackDecode(u_char *fp_r, u_char *fp_w) {
    int           i, c1, c2, c;
    u_int         moto_size;  /* Decode size */
    int           rp = N - F; /* Ring buffer position */
    unsigned int  flags = 0;  /* LZSS flags */
    u_char       *fp_w_end;   /* End of write buffer (buffer + decode size) */
    PACK_HEADER  *pack_header_pp;

    moto_size = *(u_int*)fp_r;
    pack_header_pp = (PACK_HEADER*)fp_w;

    fp_w = (u_char*)PR_UNCACHED(fp_w);
    fp_w_end = fp_w + moto_size;
    fp_r += 8;

    for (i = 0; i < (N - F); i++) {
        RBuff[i] = 0;
    }

    while (fp_w != fp_w_end) {
        if (((flags >>= 1) & 256) == 0) {
            c = *fp_r++;
            flags = c | 0xff00;
        }
        
        if (flags & 0x1) {
            c = *fp_r++;
            *fp_w++ = c;
            RBuff[rp++] = c;
            rp &= (N - 1);
        } else {
            c1 = *fp_r++;
            c2 = *fp_r++;

            c1 |= (c2 & 0xf0) << 0x4;
            c2 = (c2 & 0xf) + THRESHOLD;

            for (i = 0; i <= c2; i++) {
                c = RBuff[(c1 + i) & (N - 1)];
                *fp_w++ = c;
                RBuff[rp++] = c;
                rp &= (N - 1);
            }
        }
    }

    return (pack_header_pp->id != 0x12345678);
}

int PackGetAdrs(u_int adrs, int num) {
    PACK_STR *pack_str_pp = (PACK_STR*)adrs;

    if (num >= pack_str_pp->pack_header.fcnt) {
        return -1;
    }

    return (u_int)pack_str_pp + pack_str_pp->pack_list[num].ofs;
}

void PackDbgList(u_int adrs) {
    PACK_STR *pack_str_pp = (PACK_STR*)adrs;
    int       i;

    for (i = 0; i < pack_str_pp->pack_header.fcnt; i++) {
        printf(" #%d file[%s] size[%d]\n", i, pack_str_pp->pack_list[i].fname, pack_str_pp->pack_list[i].size);
    }
}
