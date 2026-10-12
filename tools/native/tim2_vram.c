/*
 * Load TIM2 textures into the native GS memory model the way Tim2Trans does
 * (each picture to its own TEX0 base, width and format), then read a texture
 * back through a game TEX0 and write it as a PNG. With a subtitle glyph table
 * (an MCODE_STR .cod file) it also writes every glyph cut out of that texture.
 *
 * Build (from the repository root):
 *   cc -std=gnu99 -O2 -Wall -Iinclude/rtl/ee -Iinclude/rtl/common -Isrc/native \
 *      tools/native/tim2_vram.c src/native/gs_memory.c -lz -o tim2_vram
 * Usage:
 *   tim2_vram <out.png> <tbp> <tbw> <psm> <w> <h> <cbp> <cpsm> [-cod table.cod glyphs.png] file.tm0...
 * Numbers accept C prefixes (0x2300). Inputs are original game data; keep the
 * PNGs out of Git.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgraph.h>

#include "gs_memory.h"
#include "png_write.h"
#include "tim2_upload.h"

static GsMemory vram;

/* Composite on grey so transparent texels stay visible. */
static unsigned int onGrey(unsigned int c) {
    unsigned int a = c >> 24, out = 0xff000000u;
    int shift;

    for (shift = 0; shift < 24; shift += 8) {
        unsigned int v = (c >> shift) & 0xff;
        out |= ((v * a + 0x40 * (255 - a)) / 255) << shift;
    }
    return out;
}

int main(int argc, char **argv) {
    const char *out, *codPath = NULL, *glyphPath = NULL;
    int tbp, tbw, psm, w, h, cbp, cpsm, x, y, i, first = 9;
    unsigned int *image;

    if (argc < 10) {
        fprintf(stderr, "usage: %s out.png tbp tbw psm w h cbp cpsm [-cod table.cod glyphs.png] file.tm0...\n", argv[0]);
        return 1;
    }
    out = argv[1];
    tbp = strtol(argv[2], NULL, 0);
    tbw = strtol(argv[3], NULL, 0);
    psm = strtol(argv[4], NULL, 0);
    w = strtol(argv[5], NULL, 0);
    h = strtol(argv[6], NULL, 0);
    cbp = strtol(argv[7], NULL, 0);
    cpsm = strtol(argv[8], NULL, 0);
    if (strcmp(argv[first], "-cod") == 0) {
        codPath = argv[first + 1];
        glyphPath = argv[first + 2];
        first += 3;
    }
    for (i = first; i < argc; i++) {
        uploadTim2(&vram, argv[i]);
    }

    image = malloc(sizeof(unsigned int) * w * h);
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            image[y * w + x] = onGrey(GsMemoryReadTexelRGBA(&vram, psm, tbp, tbw, x, y, cbp, cpsm));
        }
    }
    writePng(out, image, w, h);
    printf("wrote %s (%dx%d)\n", out, w, h);

    if (codPath != NULL) {
        /* MCODE_STR: a count, then 12-byte MCODE_KANJI {code, u, v, w, h, adjx, adjy}. */
        long size;
        unsigned char *cod = readFile(codPath, &size);
        int count = (int)u32(cod), columns = 16, cell = 26, rows, g;
        unsigned int *sheet;

        if (4 + count * 12 != size) {
            fprintf(stderr, "%s: %d glyphs do not fill %ld bytes\n", codPath, count, size);
            return 1;
        }
        rows = (count + columns - 1) / columns;
        sheet = calloc((size_t)columns * cell * rows * cell, sizeof(unsigned int));
        for (g = 0; g < count; g++) {
            const unsigned char *k = cod + 4 + g * 12;
            int gu = u16(k + 2), gv = u16(k + 4), gw = u16(k + 6), gh = u16(k + 8);
            int ox = (g % columns) * cell + 1, oy = (g / columns) * cell + 1;
            for (y = 0; y < gh && y < cell - 1; y++) {
                for (x = 0; x < gw && x < cell - 1; x++) {
                    sheet[(oy + y) * columns * cell + ox + x] =
                        onGrey(GsMemoryReadTexelRGBA(&vram, psm, tbp, tbw, gu + x, gv + y, cbp, cpsm));
                }
            }
        }
        writePng(glyphPath, sheet, columns * cell, rows * cell);
        printf("wrote %s (%d glyphs)\n", glyphPath, count);
    }
    return 0;
}
