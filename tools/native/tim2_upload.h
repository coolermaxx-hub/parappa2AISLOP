#ifndef NATIVE_TIM2_UPLOAD_H
#define NATIVE_TIM2_UPLOAD_H

/* File reading and one-picture TIM2 upload into the native GS memory model,
 * as Tim2Trans does (each picture goes to its own TEX0 base, width and format). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgraph.h>

#include "gs_memory.h"

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
static void uploadTim2(GsMemory *vram, const char *path) {
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
    GsMemoryUpload(vram, psm, tbp, tbw, 0, 0, width, height, pic + headerSize);
    printf("%s: %ux%u psm %d -> tbp %#x tbw %d\n", path, width, height, psm, tbp, tbw);
    free(d);
}


#endif /* NATIVE_TIM2_UPLOAD_H */
