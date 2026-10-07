/*
 * Load TIM2 textures into the native GS memory model the way Tim2Trans does
 * (each picture to its own TEX0 base, width and format), then read a texture
 * back through a game TEX0 and write it as a PNG. With a subtitle glyph table
 * (an MCODE_STR .cod file) it also writes every glyph cut out of that texture.
 *
 * Build (from the repository root):
 *   cc -std=c99 -O2 -Wall -Iinclude/rtl/ee -Iinclude/rtl/common -Isrc/native \
 *      tools/native/tim2_vram.c src/native/gs_memory.c -lz -o tim2_vram
 * Usage:
 *   tim2_vram <out.png> <tbp> <tbw> <psm> <w> <h> <cbp> <cpsm> [-cod table.cod glyphs.png] file.tm0...
 * Numbers accept C prefixes (0x2300). Inputs are original game data; keep the
 * PNGs out of Git.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include <libgraph.h>

#include "gs_memory.h"

static GsMemory vram;

static unsigned char *readFile(const char *path, long *size) {
    FILE *f = fopen(path, "rb");
    unsigned char *data;

    if (f == NULL) {
        perror(path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(*size);
    if (fread(data, 1, *size, f) != (size_t)*size) {
        perror(path);
        exit(1);
    }
    fclose(f);
    return data;
}

static unsigned int u32(const unsigned char *p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned int)p[3] << 24);
}

static unsigned int u16(const unsigned char *p) {
    return p[0] | (p[1] << 8);
}

/* One-picture TIM2 without a CLUT, as the stage archives carry. */
static void uploadTim2(const char *path) {
    long size;
    unsigned char *d = readFile(path, &size);
    const unsigned char *pic = d + 16;
    unsigned int clutSize, imageSize, headerSize, width, height;
    unsigned long long tex0;
    int tbp, tbw, psm;

    if (size < 64 || memcmp(d, "TIM2", 4) != 0 || u16(d + 6) != 1) {
        fprintf(stderr, "%s: not a one-picture TIM2\n", path);
        exit(1);
    }
    clutSize = u32(pic + 4);
    imageSize = u32(pic + 8);
    headerSize = u16(pic + 12);
    width = u16(pic + 0x14);
    height = u16(pic + 0x16);
    tex0 = u32(pic + 0x18) | ((unsigned long long)u32(pic + 0x1c) << 32);
    tbp = tex0 & 0x3fff;
    tbw = (tex0 >> 14) & 0x3f;
    psm = (tex0 >> 20) & 0x3f;
    if (clutSize != 0 || 16 + headerSize + imageSize > (unsigned long)size) {
        fprintf(stderr, "%s: unsupported TIM2 layout\n", path);
        exit(1);
    }
    GsMemoryUpload(&vram, psm, tbp, tbw, 0, 0, width, height, pic + headerSize);
    printf("%s: %ux%u psm %d -> tbp %#x tbw %d\n", path, width, height, psm, tbp, tbw);
    free(d);
}

static void writePng(const char *path, const unsigned int *rgba, int w, int h) {
    static const unsigned char signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n' };
    unsigned long rawSize = (unsigned long)(w * 4 + 1) * h, packedSize = compressBound(rawSize);
    unsigned char *raw = malloc(rawSize), *packed = malloc(packedSize);
    unsigned char ihdr[13];
    FILE *f;
    int x, y;

    for (y = 0; y < h; y++) {
        raw[y * (w * 4 + 1)] = 0;
        for (x = 0; x < w; x++) {
            unsigned int c = rgba[y * w + x];
            unsigned char *p = raw + y * (w * 4 + 1) + 1 + x * 4;
            p[0] = c;
            p[1] = c >> 8;
            p[2] = c >> 16;
            p[3] = c >> 24;
        }
    }
    compress2(packed, &packedSize, raw, rawSize, 9);

    f = fopen(path, "wb");
    if (f == NULL) {
        perror(path);
        exit(1);
    }
    fwrite(signature, 1, 8, f);
#define CHUNK(type, data, len)                                                   \
    do {                                                                         \
        unsigned char hdr[8] = { (len) >> 24, (len) >> 16, (len) >> 8, (len), \
                                 type[0], type[1], type[2], type[3] };          \
        unsigned long crc = crc32(crc32(0, hdr + 4, 4), (data), (len));        \
        unsigned char tail[4] = { crc >> 24, crc >> 16, crc >> 8, crc };       \
        fwrite(hdr, 1, 8, f);                                                    \
        fwrite((data), 1, (len), f);                                             \
        fwrite(tail, 1, 4, f);                                                   \
    } while (0)
    ihdr[0] = w >> 24; ihdr[1] = w >> 16; ihdr[2] = w >> 8; ihdr[3] = w;
    ihdr[4] = h >> 24; ihdr[5] = h >> 16; ihdr[6] = h >> 8; ihdr[7] = h;
    ihdr[8] = 8; ihdr[9] = 6; ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;
    CHUNK("IHDR", ihdr, 13u);
    CHUNK("IDAT", packed, (unsigned int)packedSize);
    CHUNK("IEND", ihdr, 0u);
#undef CHUNK
    fclose(f);
    free(raw);
    free(packed);
}

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
        uploadTim2(argv[i]);
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
