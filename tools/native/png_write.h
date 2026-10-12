#ifndef NATIVE_PNG_WRITE_H
#define NATIVE_PNG_WRITE_H

/* Minimal 8-bit RGBA PNG writer (zlib). Pixels are 0xAABBGGRR, rows top to bottom. */
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

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


#endif /* NATIVE_PNG_WRITE_H */
