/*
 * Native model of GS local memory addressing. See gs_memory.h.
 *
 * Memory is a flat array of 256-byte blocks. A pixel's address is its page
 * (pages are laid out bw / page width across), the block within the page and
 * the column position within the block. The block and column tables are the
 * GS's fixed swizzles for each pixel storage format.
 */
#include "gs_memory.h"

#include <libgraph.h>

static const unsigned char blockTable32[4][8] = {
    { 0, 1, 4, 5, 16, 17, 20, 21 },
    { 2, 3, 6, 7, 18, 19, 22, 23 },
    { 8, 9, 12, 13, 24, 25, 28, 29 },
    { 10, 11, 14, 15, 26, 27, 30, 31 },
};

static const unsigned char columnTable32[8][8] = {
    { 0, 1, 4, 5, 8, 9, 12, 13 },
    { 2, 3, 6, 7, 10, 11, 14, 15 },
    { 16, 17, 20, 21, 24, 25, 28, 29 },
    { 18, 19, 22, 23, 26, 27, 30, 31 },
    { 32, 33, 36, 37, 40, 41, 44, 45 },
    { 34, 35, 38, 39, 42, 43, 46, 47 },
    { 48, 49, 52, 53, 56, 57, 60, 61 },
    { 50, 51, 54, 55, 58, 59, 62, 63 },
};

static const unsigned char blockTable16[8][4] = {
    { 0, 2, 8, 10 },
    { 1, 3, 9, 11 },
    { 4, 6, 12, 14 },
    { 5, 7, 13, 15 },
    { 16, 18, 24, 26 },
    { 17, 19, 25, 27 },
    { 20, 22, 28, 30 },
    { 21, 23, 29, 31 },
};

static const unsigned char columnTable16[8][16] = {
    { 0, 2, 8, 10, 16, 18, 24, 26, 1, 3, 9, 11, 17, 19, 25, 27 },
    { 4, 6, 12, 14, 20, 22, 28, 30, 5, 7, 13, 15, 21, 23, 29, 31 },
    { 32, 34, 40, 42, 48, 50, 56, 58, 33, 35, 41, 43, 49, 51, 57, 59 },
    { 36, 38, 44, 46, 52, 54, 60, 62, 37, 39, 45, 47, 53, 55, 61, 63 },
    { 64, 66, 72, 74, 80, 82, 88, 90, 65, 67, 73, 75, 81, 83, 89, 91 },
    { 68, 70, 76, 78, 84, 86, 92, 94, 69, 71, 77, 79, 85, 87, 93, 95 },
    { 96, 98, 104, 106, 112, 114, 120, 122, 97, 99, 105, 107, 113, 115, 121, 123 },
    { 100, 102, 108, 110, 116, 118, 124, 126, 101, 103, 109, 111, 117, 119, 125, 127 },
};

static const unsigned char blockTable4[8][4] = {
    { 0, 2, 8, 10 },
    { 1, 3, 9, 11 },
    { 4, 6, 12, 14 },
    { 5, 7, 13, 15 },
    { 16, 18, 24, 26 },
    { 17, 19, 25, 27 },
    { 20, 22, 28, 30 },
    { 21, 23, 29, 31 },
};

/* Nibble index within a 32 x 16 block; rows 8-15 repeat rows 0-7 plus 256. */
static const unsigned short columnTable4[8][32] = {
    { 0, 8, 32, 40, 64, 72, 96, 104, 2, 10, 34, 42, 66, 74, 98, 106,
      4, 12, 36, 44, 68, 76, 100, 108, 6, 14, 38, 46, 70, 78, 102, 110 },
    { 16, 24, 48, 56, 80, 88, 112, 120, 18, 26, 50, 58, 82, 90, 114, 122,
      20, 28, 52, 60, 84, 92, 116, 124, 22, 30, 54, 62, 86, 94, 118, 126 },
    { 65, 73, 97, 105, 1, 9, 33, 41, 67, 75, 99, 107, 3, 11, 35, 43,
      69, 77, 101, 109, 5, 13, 37, 45, 71, 79, 103, 111, 7, 15, 39, 47 },
    { 81, 89, 113, 121, 17, 25, 49, 57, 83, 91, 115, 123, 19, 27, 51, 59,
      85, 93, 117, 125, 21, 29, 53, 61, 87, 95, 119, 127, 23, 31, 55, 63 },
    { 192, 200, 224, 232, 128, 136, 160, 168, 194, 202, 226, 234, 130, 138, 162, 170,
      196, 204, 228, 236, 132, 140, 164, 172, 198, 206, 230, 238, 134, 142, 166, 174 },
    { 208, 216, 240, 248, 144, 152, 176, 184, 210, 218, 242, 250, 146, 154, 178, 186,
      212, 220, 244, 252, 148, 156, 180, 188, 214, 222, 246, 254, 150, 158, 182, 190 },
    { 129, 137, 161, 169, 193, 201, 225, 233, 131, 139, 163, 171, 195, 203, 227, 235,
      133, 141, 165, 173, 197, 205, 229, 237, 135, 143, 167, 175, 199, 207, 231, 239 },
    { 145, 153, 177, 185, 209, 217, 241, 249, 147, 155, 179, 187, 211, 219, 243, 251,
      149, 157, 181, 189, 213, 221, 245, 253, 151, 159, 183, 191, 215, 223, 247, 255 },
};

#define BLOCKS (GS_MEMORY_SIZE / 256)

static unsigned int block(int bp, int page, int blockInPage) {
    return (unsigned int)(bp + page * 32 + blockInPage) % BLOCKS;
}

/* Byte offset of a 32-bit pixel. */
static unsigned int address32(int bp, int bw, int x, int y) {
    int page = (y >> 5) * bw + (x >> 6);
    return block(bp, page, blockTable32[(y >> 3) & 3][(x >> 3) & 7]) * 256 + columnTable32[y & 7][x & 7] * 4;
}

/* Byte offset of a 16-bit pixel. */
static unsigned int address16(int bp, int bw, int x, int y) {
    int page = (y >> 6) * bw + (x >> 6);
    return block(bp, page, blockTable16[(y >> 3) & 7][(x >> 4) & 3]) * 256 + columnTable16[y & 7][x & 15] * 2;
}

/* Nibble offset of a 4-bit pixel; pages are 128 wide, so bw / 2 across. */
static unsigned int address4(int bp, int bw, int x, int y) {
    int page = (y >> 7) * (bw >> 1) + (x >> 7);
    return block(bp, page, blockTable4[(y >> 4) & 7][(x >> 5) & 3]) * 512 + columnTable4[y & 7][x & 31] + ((y & 8) ? 256 : 0);
}

void GsMemoryWritePixel(GsMemory *mem, int psm, int bp, int bw, int x, int y, unsigned int value) {
    unsigned char *m = mem->bytes;
    unsigned int a;

    switch (psm) {
    case SCE_GS_PSMCT32:
        a = address32(bp, bw, x, y);
        m[a] = value;
        m[a + 1] = value >> 8;
        m[a + 2] = value >> 16;
        m[a + 3] = value >> 24;
        break;
    case SCE_GS_PSMCT24:
        a = address32(bp, bw, x, y);
        m[a] = value;
        m[a + 1] = value >> 8;
        m[a + 2] = value >> 16;
        break;
    case SCE_GS_PSMCT16:
        a = address16(bp, bw, x, y);
        m[a] = value;
        m[a + 1] = value >> 8;
        break;
    case SCE_GS_PSMT4:
        a = address4(bp, bw, x, y);
        if (a & 1) {
            m[a >> 1] = (m[a >> 1] & 0x0f) | ((value & 0xf) << 4);
        } else {
            m[a >> 1] = (m[a >> 1] & 0xf0) | (value & 0xf);
        }
        break;
    default:
        break;
    }
}

unsigned int GsMemoryReadPixel(const GsMemory *mem, int psm, int bp, int bw, int x, int y) {
    const unsigned char *m = mem->bytes;
    unsigned int a;

    switch (psm) {
    case SCE_GS_PSMCT32:
        a = address32(bp, bw, x, y);
        return m[a] | (m[a + 1] << 8) | (m[a + 2] << 16) | ((unsigned int)m[a + 3] << 24);
    case SCE_GS_PSMCT24:
        a = address32(bp, bw, x, y);
        return m[a] | (m[a + 1] << 8) | (m[a + 2] << 16);
    case SCE_GS_PSMCT16:
        a = address16(bp, bw, x, y);
        return m[a] | (m[a + 1] << 8);
    case SCE_GS_PSMT4:
        a = address4(bp, bw, x, y);
        return (a & 1) ? m[a >> 1] >> 4 : m[a >> 1] & 0xf;
    default:
        return 0;
    }
}

void GsMemoryUpload(GsMemory *mem, int psm, int bp, int bw, int x, int y, int w, int h, const void *src) {
    const unsigned char *s = (const unsigned char *)src;
    int i, j;

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            unsigned int value;
            int n = j * w + i;

            switch (psm) {
            case SCE_GS_PSMCT32:
                value = s[n * 4] | (s[n * 4 + 1] << 8) | (s[n * 4 + 2] << 16) | ((unsigned int)s[n * 4 + 3] << 24);
                break;
            case SCE_GS_PSMCT24:
                value = s[n * 3] | (s[n * 3 + 1] << 8) | (s[n * 3 + 2] << 16);
                break;
            case SCE_GS_PSMCT16:
                value = s[n * 2] | (s[n * 2 + 1] << 8);
                break;
            case SCE_GS_PSMT4:
                value = (n & 1) ? s[n >> 1] >> 4 : s[n >> 1] & 0xf;
                break;
            default:
                return;
            }
            GsMemoryWritePixel(mem, psm, bp, bw, x + i, y + j, value);
        }
    }
}

static unsigned int scaleAlpha(unsigned int a) {
    return a >= 0x80 ? 0xff : a * 2;
}

static unsigned int colour(unsigned int raw, int psm) {
    if (psm == SCE_GS_PSMCT16) {
        unsigned int r = (raw & 0x1f) << 3, g = ((raw >> 5) & 0x1f) << 3, b = ((raw >> 10) & 0x1f) << 3;
        unsigned int a = (raw & 0x8000) ? 0x80 : 0;
        return r | (g << 8) | (b << 16) | (scaleAlpha(a) << 24);
    }
    if (psm == SCE_GS_PSMCT24) {
        return (raw & 0xffffff) | 0xff000000u;
    }
    return (raw & 0xffffff) | (scaleAlpha(raw >> 24) << 24);
}

unsigned int GsMemoryReadTexelRGBA(const GsMemory *mem, int psm, int tbp, int tbw, int x, int y, int cbp, int cpsm) {
    unsigned int raw = GsMemoryReadPixel(mem, psm, tbp, tbw, x, y);

    if (psm == SCE_GS_PSMT4) {
        /* CSM1: 16 entries sit at the CLUT base as an 8 x 2 block of 32-bit
         * pixels, or a 16 x 1 row of 16-bit pixels. */
        if (cpsm == SCE_GS_PSMCT16) {
            raw = GsMemoryReadPixel(mem, cpsm, cbp, 1, raw, 0);
        } else {
            raw = GsMemoryReadPixel(mem, cpsm, cbp, 1, raw & 7, raw >> 3);
        }
        return colour(raw, cpsm);
    }
    return colour(raw, psm);
}
