/* Native checks of the GS memory model's addressing (src/native/gs_memory.c).
 * Build: cc -std=gnu99 -O2 -Wall -Iinclude/rtl/ee -Iinclude/rtl/common -Isrc/native
 *        tests/native/gs_memory.c src/native/gs_memory.c
 * The stage subtitle fonts give the end-to-end check: their 4-bit glyphs,
 * uploaded as 32-bit transfers, read back legibly (docs/native-port.md). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgraph.h>

#include "gs_memory.h"

static GsMemory mem;
static int failures = 0;

static void check(int ok, const char *what) {
    if (!ok) {
        printf("FAIL: %s\n", what);
        failures++;
    }
}

/* Each pixel of one page must land in a distinct cell of the page's 8 KB. */
static void pageIsBijective(int psm, int width, int height, int bits, const char *name) {
    int cells = 8192 * 8 / bits, x, y, unique = 0;
    unsigned char *seen = calloc(cells, 1);

    memset(&mem, 0, sizeof(mem));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* Mark the cell by writing all ones, then find which cell changed. */
            int before = unique, i;
            GsMemoryWritePixel(&mem, psm, 0, width / 64 > 0 ? width / 64 : 1, x, y, 0xffffffffu);
            for (i = 0; i < cells; i++) {
                int bit = i * bits;
                unsigned int v = (mem.bytes[bit >> 3] >> (bit & 7)) & 1;
                if (v && !seen[i]) {
                    seen[i] = 1;
                    unique++;
                }
            }
            if (unique != before + 1) {
                printf("FAIL: %s pixel (%d,%d) reused or missed a cell\n", name, x, y);
                failures++;
                free(seen);
                return;
            }
        }
    }
    check(unique == cells, name);
    free(seen);
}

int main(void) {
    int x, y;

    pageIsBijective(SCE_GS_PSMCT32, 64, 32, 32, "PSMCT32 page covers 8 KB once");
    pageIsBijective(SCE_GS_PSMCT16, 64, 64, 16, "PSMCT16 page covers 8 KB once");
    pageIsBijective(SCE_GS_PSMT4, 128, 128, 4, "PSMT4 page covers 8 KB once");

    /* Pages tile by buffer width: the second page of a 128-wide CT32 buffer
     * starts 32 blocks after the first. */
    memset(&mem, 0, sizeof(mem));
    GsMemoryWritePixel(&mem, SCE_GS_PSMCT32, 0, 2, 64, 0, 0x11223344u);
    check(mem.bytes[32 * 256] == 0x44 && mem.bytes[32 * 256 + 3] == 0x11, "PSMCT32 second page starts at block 32");

    /* Round trips in each format. */
    for (y = 0; y < 40; y++) {
        for (x = 0; x < 70; x++) {
            GsMemoryWritePixel(&mem, SCE_GS_PSMT4, 0x100, 4, x, y, (x * 7 + y) & 0xf);
        }
    }
    for (y = 0; y < 40; y++) {
        for (x = 0; x < 70; x++) {
            if (GsMemoryReadPixel(&mem, SCE_GS_PSMT4, 0x100, 4, x, y) != (unsigned int)((x * 7 + y) & 0xf)) {
                check(0, "PSMT4 round trip");
                y = 40;
                break;
            }
        }
    }

    /* CSM1 palettes: entry i of a 32-bit CLUT is pixel (i & 7, i >> 3), of a
     * 16-bit CLUT pixel (i, 0). Alpha 0x80 reads back as 0xff. */
    memset(&mem, 0, sizeof(mem));
    for (x = 0; x < 16; x++) {
        GsMemoryWritePixel(&mem, SCE_GS_PSMCT32, 0x40, 1, x & 7, x >> 3, 0x80000000u | (x * 16) | ((15 - x) * 16) << 8);
        GsMemoryWritePixel(&mem, SCE_GS_PSMCT16, 0x60, 1, x, 0, 0x8000 | x);
    }
    GsMemoryWritePixel(&mem, SCE_GS_PSMT4, 0, 2, 5, 3, 11);
    check(GsMemoryReadTexelRGBA(&mem, SCE_GS_PSMT4, 0, 2, 5, 3, 0x40, SCE_GS_PSMCT32) == (0xff000000u | (11 * 16) | ((15 - 11) * 16) << 8), "PSMT4 texel through a 32-bit CLUT");
    check(GsMemoryReadTexelRGBA(&mem, SCE_GS_PSMT4, 0, 2, 5, 3, 0x60, SCE_GS_PSMCT16) == (0xff000000u | (11 << 3)), "PSMT4 texel through a 16-bit CLUT");

    if (failures == 0) {
        printf("PASS: native GS memory addressing\n");
    }
    return failures != 0;
}
