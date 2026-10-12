#ifndef NATIVE_GS_MEMORY_H
#define NATIVE_GS_MEMORY_H

/*
 * Native model of the GS's 4 MB local memory. Pixels are stored at the
 * addresses the GS uses for each pixel storage format (page, block and column
 * swizzles), so data uploaded in one format and read in another (the game
 * sends 4-bit fonts as 32-bit transfers) comes back as on hardware. Never
 * built for the EE.
 */
#ifdef __cplusplus
extern "C" {
#endif

#define GS_MEMORY_SIZE (4 * 1024 * 1024)

typedef struct {
    unsigned char bytes[GS_MEMORY_SIZE];
} GsMemory;

/* Store or load one pixel's raw value: bp in 256-byte blocks, bw in 64 pixels.
 * Supported formats: PSMCT32, PSMCT24, PSMCT16, PSMT4. */
void GsMemoryWritePixel(GsMemory *mem, int psm, int bp, int bw, int x, int y, unsigned int value);
unsigned int GsMemoryReadPixel(const GsMemory *mem, int psm, int bp, int bw, int x, int y);

/* Host-to-local transfer of a w x h rectangle at (x, y), as BITBLTBUF/TRXPOS/
 * TRXREG + HWREG do. Source pixels are packed row by row in the format's own
 * size; PSMT4 packs two pixels a byte, the left one in the low nibble. */
void GsMemoryUpload(GsMemory *mem, int psm, int bp, int bw, int x, int y, int w, int h, const void *src);

/* Texel colour as 8-bit RGBA, PS2 alpha scaled so 0x80 becomes 0xff.
 * 4-bit textures look up a CSM1 CLUT at block cbp in cpsm: 8 x 2 pixels for
 * PSMCT32, 16 x 1 for PSMCT16.
 * PSMCT16 alpha follows TEXA with TA0 = 0, TA1 = 0x80 and AEM off. */
unsigned int GsMemoryReadTexelRGBA(const GsMemory *mem, int psm, int tbp, int tbw, int x, int y, int cbp, int cpsm);

#ifdef __cplusplus
}
#endif

#endif /* NATIVE_GS_MEMORY_H */
