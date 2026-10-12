/*
 * Native software renderer for one SPM model in its rest pose.
 *
 * Reads an unrelocated version-5 SPM (docs/spm-geometry-layout.md,
 * spm-packet-layout.md), composes the node hierarchy, walks each node's
 * context-1 packets chunk by chunk as the EE does, takes the GS registers from
 * each chunk's GIF prefix (TEX0 selects the texture) and draws the vertex
 * strips with a z-buffer. Stage textures come from the TIM2 uploads of the
 * stage's .INT archive, stored in the native GS memory model, so 4-bit
 * textures with CLUTs are read back through the page/block/column swizzle.
 *
 * It draws what the data says; it does not run the original microprograms, so
 * it shows layout and content, not PS2 pixel equivalence. Skipped: animation,
 * skinning/deformation, contours, clipping and lighting. Reflection-mapped
 * surfaces (module 3) get approximate sphere-map coordinates from the normal;
 * the real ones need the camera (vump_refmap.vsm), so those surfaces can look
 * wrong. Blending follows each chunk's ALPHA register; opaque chunks are drawn
 * first, then blended ones without depth writes, not sorted.
 *
 * Build (repository root):
 *   cc -std=gnu99 -O2 -Wall -Iinclude/rtl/ee -Iinclude/rtl/common -Isrc/native \
 *      tools/native/spm_view.c src/native/gs_memory.c -lz -lm -o spm_view
 * Usage: spm_view out.png model.spm [-view front|side|back|top] [-size N] [-only MODULE] tex.tm0...
 * The inputs are original game data; keep the PNG out of Git.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgraph.h>

#include "gs_memory.h"
#include "png_write.h"
#include "tim2_upload.h"

#define SPM_MAGIC 0x18df540au
#define NODE_FLAG_IDENTITY 0x1u
#define PACKET_MODULE 0x60

static GsMemory vram;

typedef struct {
    float m[4][4]; /* column vectors, like NaMATRIX */
} Matrix;

typedef struct {
    float x, y, z, u, v;
    unsigned int rgba;
} Vertex;

static const unsigned char *model;
static long modelSize;

static unsigned int word(unsigned int offset) {
    unsigned int v;
    if (offset + 4 > (unsigned long)modelSize) {
        fprintf(stderr, "read outside model at %#x\n", offset);
        exit(1);
    }
    memcpy(&v, model + offset, 4);
    return v;
}

static float real(unsigned int offset) {
    unsigned int w = word(offset);
    float f;
    memcpy(&f, &w, 4);
    return f;
}

static unsigned long long qword(unsigned int offset) {
    return word(offset) | ((unsigned long long)word(offset + 4) << 32);
}

static Matrix identity(void) {
    Matrix r;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            r.m[i][j] = i == j;
        }
    }
    return r;
}

static Matrix readMatrix(unsigned int offset) {
    Matrix r;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            r.m[i][j] = real(offset + (i * 4 + j) * 4);
        }
    }
    return r;
}

/* a * b for column-vector storage: column j of the result is a * column j of b. */
static Matrix multiply(Matrix a, Matrix b) {
    Matrix r;
    int j, row;
    for (j = 0; j < 4; j++) {
        for (row = 0; row < 4; row++) {
            r.m[j][row] = ((a.m[0][row] * b.m[j][0] + a.m[1][row] * b.m[j][1]) + a.m[2][row] * b.m[j][2]) + a.m[3][row] * b.m[j][3];
        }
    }
    return r;
}

static void transform(Matrix a, const float p[3], float out[3]) {
    int row;
    for (row = 0; row < 3; row++) {
        out[row] = ((a.m[0][row] * p[0] + a.m[1][row] * p[1]) + a.m[2][row] * p[2]) + a.m[3][row];
    }
}

/* ---- Frame buffer ---- */

static int width, height;
static unsigned int *color;
static float *depth;
static float scale, centre[3];
static int viewMode; /* 0 front (+z toward the viewer), 1 side, 2 back, 3 top */

static void project(const float p[3], float *sx, float *sy, float *sz) {
    float a, b, c;
    switch (viewMode) {
    case 1: a = -p[2]; b = p[1]; c = p[0]; break;
    case 2: a = -p[0]; b = p[1]; c = -p[2]; break;
    case 3: a = p[0]; b = -p[2]; c = p[1]; break;
    default: a = p[0]; b = p[1]; c = p[2]; break;
    }
    *sx = width / 2 + (a - centre[0]) * scale;
    *sy = height / 2 - (b - centre[1]) * scale;
    *sz = c;
}

/* ---- Texture sampling ---- */

typedef struct {
    int tbp, tbw, psm, tw, th, cbp, cpsm;
    int valid;
    int blend; /* PRIM.ABE of the chunk's drawing tag */
    unsigned long long alpha; /* ALPHA_1: A, B, C, D selectors and FIX */
} Texture;

static unsigned int sample(const Texture *t, float u, float v) {
    int w = 1 << t->tw, h = 1 << t->th;
    int x = (int)floorf(u * w) & (w - 1), y = (int)floorf(v * h) & (h - 1);
    return GsMemoryReadTexelRGBA(&vram, t->psm, t->tbp, t->tbw, x, y, t->cbp, t->cpsm);
}

static unsigned int modulate(unsigned int texel, unsigned int vertex) {
    unsigned int out = 0, shift;
    for (shift = 0; shift < 32; shift += 8) {
        unsigned int t = (texel >> shift) & 0xff, c = (vertex >> shift) & 0xff;
        unsigned int r = t * c / 128;
        out |= (r > 255 ? 255 : r) << shift;
    }
    return out;
}


/* GS alpha blending: Cv = ((A - B) * C >> 7) + D, with A, B, D from {Cs, Cd, 0}
 * and C from {As, Ad, FIX}. The destination alpha is taken as 0x80. */
static unsigned int blend(unsigned int src, unsigned int dst, unsigned long long reg) {
    int selA = reg & 3, selB = (reg >> 2) & 3, selC = (reg >> 4) & 3, selD = (reg >> 6) & 3;
    int fix = (reg >> 32) & 0xff, as = (int)(src >> 24) * 128 / 255, shift;
    unsigned int out = 0xff000000u;

    for (shift = 0; shift < 24; shift += 8) {
        int cs = (src >> shift) & 0xff, cd = (dst >> shift) & 0xff;
        int a = selA == 0 ? cs : selA == 1 ? cd : 0;
        int b = selB == 0 ? cs : selB == 1 ? cd : 0;
        int c = selC == 0 ? as : selC == 1 ? 0x80 : fix;
        int d = selD == 0 ? cs : selD == 1 ? cd : 0;
        int v = (((a - b) * c) >> 7) + d;
        out |= (unsigned int)(v < 0 ? 0 : v > 255 ? 255 : v) << shift;
    }
    return out;
}

static void drawTriangle(const Vertex *v0, const Vertex *v1, const Vertex *v2, const Texture *tex) {
    float sx[3], sy[3], sz[3];
    const Vertex *vs[3];
    float p[3], area;
    int minx, maxx, miny, maxy, x, y;

    vs[0] = v0; vs[1] = v1; vs[2] = v2;
    for (x = 0; x < 3; x++) {
        p[0] = vs[x]->x; p[1] = vs[x]->y; p[2] = vs[x]->z;
        project(p, &sx[x], &sy[x], &sz[x]);
    }
    area = (sx[1] - sx[0]) * (sy[2] - sy[0]) - (sx[2] - sx[0]) * (sy[1] - sy[0]);
    if (fabsf(area) < 1e-6f) {
        return;
    }
    minx = (int)floorf(fminf(sx[0], fminf(sx[1], sx[2])));
    maxx = (int)ceilf(fmaxf(sx[0], fmaxf(sx[1], sx[2])));
    miny = (int)floorf(fminf(sy[0], fminf(sy[1], sy[2])));
    maxy = (int)ceilf(fmaxf(sy[0], fmaxf(sy[1], sy[2])));
    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx >= width) maxx = width - 1;
    if (maxy >= height) maxy = height - 1;

    for (y = miny; y <= maxy; y++) {
        for (x = minx; x <= maxx; x++) {
            float px = x + 0.5f, py = y + 0.5f;
            float w0 = ((sx[1] - px) * (sy[2] - py) - (sx[2] - px) * (sy[1] - py)) / area;
            float w1 = ((sx[2] - px) * (sy[0] - py) - (sx[0] - px) * (sy[2] - py)) / area;
            float w2 = 1.0f - w0 - w1;
            float z, u, v;
            unsigned int c;

            if (w0 < -1e-4f || w1 < -1e-4f || w2 < -1e-4f) {
                continue;
            }
            z = w0 * sz[0] + w1 * sz[1] + w2 * sz[2];
            if (z <= depth[y * width + x]) {
                continue;
            }
            u = w0 * v0->u + w1 * v1->u + w2 * v2->u;
            v = w0 * v0->v + w1 * v1->v + w2 * v2->v;
            if (tex->valid) {
                c = modulate(sample(tex, u, v), v0->rgba);
            } else {
                c = v0->rgba;
            }
            if (tex->blend) {
                color[y * width + x] = blend(c, color[y * width + x], tex->alpha);
            } else {
                depth[y * width + x] = z;
                color[y * width + x] = c | 0xff000000u;
            }
        }
    }
}

/* ---- Model traversal ---- */

static int stats[4]; /* nodes, chunks, triangles, textured chunks */
static int pass; /* 0: opaque chunks, 1: blended chunks */
static int onlyModule = -1; /* -only N: draw one microprogram module, for debugging */

static void drawChunk(Matrix world, unsigned int chunk, unsigned int moduleId) {
    static const int quadwords[6] = { 3, 3, 0, 4, 3, 3 };
    unsigned int prefix = word(chunk + 0x10), count = word(chunk + 0x1c);
    unsigned int stride, first, i, q;
    Texture tex;
    Vertex *vs;

    if (moduleId > 5 || quadwords[moduleId] == 0 || (onlyModule >= 0 && (int)moduleId != onlyModule)) {
        return;
    }
    stride = quadwords[moduleId] * 16;
    first = chunk + 0x60 + prefix * 48;

    memset(&tex, 0, sizeof(tex));
    /* The prefix is a PACKED A+D GIF packet: a tag, then (data, address) pairs. */
    for (q = 1; q < 1 + prefix * 3; q++) {
        unsigned long long data = qword(chunk + 0x50 + q * 16);
        unsigned long long address = qword(chunk + 0x50 + q * 16 + 8);
        if (address == 0x42) { /* ALPHA_1 */
            tex.alpha = data;
        }
        if (address == 0x06) { /* TEX0_1 */
            tex.tbp = data & 0x3fff;
            tex.tbw = (data >> 14) & 0x3f;
            tex.psm = (data >> 20) & 0x3f;
            tex.tw = (data >> 26) & 15;
            tex.th = (data >> 30) & 15;
            tex.cbp = (data >> 37) & 0x3fff;
            tex.cpsm = (data >> 51) & 15;
            tex.valid = 1;
        }
    }
    {
        /* The last prefix quadword is the drawing tag; PRIM sits at bits 47-57. */
        unsigned long long drawTag = qword(chunk + 0x50 + prefix * 3 * 16);
        unsigned int prim = (drawTag >> 47) & 0x7ff;
        if (!((prim >> 4) & 1)) {
            tex.valid = 0; /* TME off: untextured */
        }
        tex.blend = (prim >> 6) & 1;
    }
    if (tex.blend != pass) {
        return;
    }
    stats[1]++;
    stats[3] += tex.valid;

    vs = malloc(sizeof(Vertex) * (count ? count : 1));
    for (i = 0; i < count; i++) {
        float p[3], o[3];
        unsigned int base = first + i * stride;
        float r, g, b, a;
        p[0] = real(base); p[1] = real(base + 4); p[2] = real(base + 8);
        transform(world, p, o);
        vs[i].x = o[0]; vs[i].y = o[1]; vs[i].z = o[2];
        /* Vertex colour is 128 for "unmodulated", as the GS's 8-bit colour sees it. */
        unsigned int colourAt = quadwords[moduleId] == 4 ? 32 : 16, stAt = quadwords[moduleId] == 4 ? 48 : 32;
        r = real(base + colourAt); g = real(base + colourAt + 4); b = real(base + colourAt + 8); a = real(base + colourAt + 12);
        vs[i].rgba = (unsigned int)fminf(r, 255) | ((unsigned int)fminf(g, 255) << 8) |
                     ((unsigned int)fminf(b, 255) << 16) | ((unsigned int)fminf(a, 255) << 24);
        vs[i].u = real(base + stAt);
        vs[i].v = real(base + stAt + 4);
        if (moduleId == 3) {
            /* Reflection module: the microprogram derives the coordinates from the
             * normal. Approximation: a sphere map from the world normal's x and y. */
            float n[3], nw[3];
            n[0] = real(base + 16); n[1] = real(base + 20); n[2] = real(base + 24);
            nw[0] = world.m[0][0] * n[0] + world.m[1][0] * n[1] + world.m[2][0] * n[2];
            nw[1] = world.m[0][1] * n[0] + world.m[1][1] * n[1] + world.m[2][1] * n[2];
            vs[i].u = 0.5f + 0.5f * nw[0];
            vs[i].v = 0.5f - 0.5f * nw[1];
        }
    }
    for (i = 0; i + 2 < count; i++) {
        if (i & 1) {
            drawTriangle(&vs[i + 1], &vs[i], &vs[i + 2], &tex);
        } else {
            drawTriangle(&vs[i], &vs[i + 1], &vs[i + 2], &tex);
        }
        stats[2]++;
    }
    free(vs);
}

static void drawNode(unsigned int node, Matrix parent) {
    unsigned int flags = word(node + 0x154), packet, k, child;
    Matrix world = (flags & NODE_FLAG_IDENTITY) ? parent : multiply(parent, readMatrix(node));

    stats[0] += pass == 0;
    for (k = 0; k < 2; k++) {
        packet = word(node + 0x16c + k * 4);
        if (packet) {
            unsigned int chunk = packet + 0x1a0;
            for (;;) {
                unsigned int tag = word(chunk), qwc = tag & 0xffff, id = (tag >> 28) & 7;
                drawChunk(world, chunk, word(packet + PACKET_MODULE));
                if (id == 6) {
                    break;
                }
                chunk += (qwc + 1) * 16;
            }
        }
    }
    for (child = word(node + 0x15c); child; child = word(child + 0x160)) {
        drawNode(child, world);
    }
}

/* Bounds of the posed vertices, to frame the picture. */
static float lo[3] = { 1e30f, 1e30f, 1e30f }, hi[3] = { -1e30f, -1e30f, -1e30f };

static void boundsChunk(Matrix world, unsigned int chunk, unsigned int moduleId) {
    static const int quadwords[6] = { 3, 3, 0, 4, 3, 3 };
    unsigned int prefix = word(chunk + 0x10), count = word(chunk + 0x1c), i;
    unsigned int first = chunk + 0x60 + prefix * 48;

    if (moduleId > 5 || quadwords[moduleId] == 0) {
        return;
    }
    for (i = 0; i < count; i++) {
        float p[3], o[3];
        int k;
        p[0] = real(first + i * quadwords[moduleId] * 16);
        p[1] = real(first + i * quadwords[moduleId] * 16 + 4);
        p[2] = real(first + i * quadwords[moduleId] * 16 + 8);
        transform(world, p, o);
        for (k = 0; k < 3; k++) {
            if (o[k] < lo[k]) lo[k] = o[k];
            if (o[k] > hi[k]) hi[k] = o[k];
        }
    }
}

static void boundsNode(unsigned int node, Matrix parent) {
    unsigned int flags = word(node + 0x154), packet, k, child;
    Matrix world = (flags & NODE_FLAG_IDENTITY) ? parent : multiply(parent, readMatrix(node));

    for (k = 0; k < 2; k++) {
        packet = word(node + 0x16c + k * 4);
        if (packet) {
            unsigned int chunk = packet + 0x1a0;
            for (;;) {
                unsigned int tag = word(chunk), qwc = tag & 0xffff, id = (tag >> 28) & 7;
                boundsChunk(world, chunk, word(packet + PACKET_MODULE));
                if (id == 6) {
                    break;
                }
                chunk += (qwc + 1) * 16;
            }
        }
    }
    for (child = word(node + 0x15c); child; child = word(child + 0x160)) {
        boundsNode(child, world);
    }
}

int main(int argc, char **argv) {
    long size;
    unsigned char *file;
    const char *out;
    int i, first = 3, nodes;
    unsigned int table;
    float span[3];

    width = height = 640;
    if (argc < 3) {
        fprintf(stderr, "usage: %s out.png model.spm [-view front|side|back|top] [-size N] tex.tm0...\n", argv[0]);
        return 1;
    }
    out = argv[1];
    file = readFile(argv[2], &size);
    model = file;
    modelSize = size;
    for (; first + 1 < argc && argv[first][0] == '-'; first += 2) {
        if (strcmp(argv[first], "-view") == 0) {
            viewMode = !strcmp(argv[first + 1], "side") ? 1 : !strcmp(argv[first + 1], "back") ? 2 : !strcmp(argv[first + 1], "top") ? 3 : 0;
        } else if (strcmp(argv[first], "-only") == 0) {
            onlyModule = atoi(argv[first + 1]);
        } else if (strcmp(argv[first], "-size") == 0) {
            width = height = atoi(argv[first + 1]);
        }
    }
    if (word(0) != SPM_MAGIC || (word(4) & 0xffff) != 5 || (word(4) >> 16) & 1) {
        fprintf(stderr, "%s: not an unrelocated version-5 SPM\n", argv[2]);
        return 1;
    }
    for (i = first; i < argc; i++) {
        uploadTim2(&vram, argv[i]);
    }

    nodes = word(0x68);
    table = word(0x74);
    if (nodes == 0) {
        return 1;
    }
    boundsNode(word(table), identity());
    for (i = 0; i < 3; i++) {
        centre[i] = (lo[i] + hi[i]) / 2;
        span[i] = hi[i] - lo[i];
    }
    {
        float a = viewMode == 1 ? span[2] : span[0], b = viewMode == 3 ? span[2] : span[1];
        float m = a > b ? a : b;
        float ca = viewMode == 1 ? centre[2] : centre[0], cb = viewMode == 3 ? centre[2] : centre[1];
        scale = (width - 24) / (m > 1e-6f ? m : 1.0f);
        if (viewMode == 1) { ca = -centre[2]; }
        if (viewMode == 2) { ca = -centre[0]; }
        if (viewMode == 3) { cb = -centre[2]; }
        centre[0] = ca;
        centre[1] = cb;
    }
    color = calloc((size_t)width * height, sizeof(unsigned int));
    depth = malloc(sizeof(float) * width * height);
    for (i = 0; i < width * height; i++) {
        depth[i] = -1e30f;
        color[i] = 0xff241c1cu;
    }
    for (pass = 0; pass < 2; pass++) {
        drawNode(word(table), identity());
    }
    writePng(out, color, width, height);
    printf("%s: %d nodes, %d chunks (%d textured), %d triangles -> %s\n", argv[2], stats[0], stats[1], stats[3], stats[2], out);
    return 0;
}
