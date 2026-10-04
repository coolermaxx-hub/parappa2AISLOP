#include "main/subt.h"

#include "os/cmngifpk.h"
#include "os/syssub.h"

#include "main/etc.h"

/* Shift-JIS full-width '@' (0x8197): like the ASCII '@', it starts a new subtitle line. */
#define SJIS_FULLWIDTH_AT_HI 0x81
#define SJIS_FULLWIDTH_AT_LO 0x97

/* Glyph rectangles in the font texture for ASCII 0x20-0xff (adjx/adjy are pen offsets). */
static MCODE_ASCII mcode_ascii[224] = {
    { 0, 0, 16, 25, 0, 0 }, /* 0x20 ' ' */
    { 100, 0, 7, 25, 0, 0 }, /* 0x21 '!' */
    { 200, 0, 8, 25, 0, 0 }, /* 0x22 '"' */
    { 20, 25, 20, 25, 0, 0 }, /* 0x23 '#' */
    { 240, 0, 13, 25, 0, 0 }, /* 0x24 '$' */
    { 0, 25, 20, 25, 0, 0 }, /* 0x25 '%' */
    { 40, 25, 17, 25, 0, 0 }, /* 0x26 '&' */
    { 180, 0, 5, 25, 0, 0 }, /* 0x27 ''' */
    { 140, 0, 11, 25, 0, 0 }, /* 0x28 '(' */
    { 160, 0, 10, 25, 0, 0 }, /* 0x29 ')' */
    { 60, 25, 17, 25, 0, 0 }, /* 0x2a '*' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x2b '+' */
    { 20, 0, 6, 25, 0, 0 }, /* 0x2c ',' */
    { 220, 0, 12, 25, 0, 0 }, /* 0x2d '-' */
    { 40, 0, 5, 25, 0, 0 }, /* 0x2e '.' */
    { 180, 225, 12, 25, 0, 0 }, /* 0x2f '/' */
    { 80, 25, 15, 25, 0, 0 }, /* 0x30 '0' */
    { 100, 25, 6, 25, 0, 0 }, /* 0x31 '1' */
    { 120, 25, 12, 25, 0, 0 }, /* 0x32 '2' */
    { 140, 25, 13, 25, 0, 0 }, /* 0x33 '3' */
    { 160, 25, 16, 25, 0, 0 }, /* 0x34 '4' */
    { 180, 25, 14, 25, 0, 0 }, /* 0x35 '5' */
    { 200, 25, 15, 25, 0, 0 }, /* 0x36 '6' */
    { 220, 25, 14, 25, 0, 0 }, /* 0x37 '7' */
    { 240, 25, 15, 25, 0, 0 }, /* 0x38 '8' */
    { 0, 50, 13, 25, 0, 0 }, /* 0x39 '9' */
    { 140, 225, 6, 25, 0, 0 }, /* 0x3a ':' */
    { 120, 225, 6, 25, 0, 0 }, /* 0x3b ';' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x3c '<' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x3d '=' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x3e '>' */
    { 80, 0, 13, 25, 0, 0 }, /* 0x3f '?' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x40 '@' */
    { 20, 50, 15, 25, 0, 0 }, /* 0x41 'A' */
    { 40, 50, 14, 25, 0, 0 }, /* 0x42 'B' */
    { 60, 50, 13, 25, 0, 0 }, /* 0x43 'C' */
    { 80, 50, 14, 25, 0, 0 }, /* 0x44 'D' */
    { 100, 50, 11, 25, 0, 0 }, /* 0x45 'E' */
    { 120, 50, 14, 25, 0, 0 }, /* 0x46 'F' */
    { 140, 50, 14, 25, 0, 0 }, /* 0x47 'G' */
    { 160, 50, 13, 25, 0, 0 }, /* 0x48 'H' */
    { 180, 50, 13, 25, 0, 0 }, /* 0x49 'I' */
    { 200, 50, 13, 25, 0, 0 }, /* 0x4a 'J' */
    { 220, 50, 12, 25, 0, 0 }, /* 0x4b 'K' */
    { 240, 50, 13, 25, 0, 0 }, /* 0x4c 'L' */
    { 0, 75, 17, 25, 0, 0 }, /* 0x4d 'M' */
    { 20, 75, 14, 25, 0, 0 }, /* 0x4e 'N' */
    { 40, 75, 14, 25, 0, 0 }, /* 0x4f 'O' */
    { 60, 75, 13, 25, 0, 0 }, /* 0x50 'P' */
    { 80, 75, 15, 25, 0, 0 }, /* 0x51 'Q' */
    { 100, 75, 15, 25, 0, 0 }, /* 0x52 'R' */
    { 120, 75, 11, 25, 0, 0 }, /* 0x53 'S' */
    { 140, 75, 15, 25, 0, 0 }, /* 0x54 'T' */
    { 160, 75, 14, 25, 0, 0 }, /* 0x55 'U' */
    { 180, 75, 17, 25, 0, 0 }, /* 0x56 'V' */
    { 200, 75, 20, 25, 0, 0 }, /* 0x57 'W' */
    { 220, 75, 14, 25, 0, 0 }, /* 0x58 'X' */
    { 240, 75, 15, 25, 0, 0 }, /* 0x59 'Y' */
    { 0, 100, 15, 25, 0, 0 }, /* 0x5a 'Z' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x5b '[' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x5c '\\' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x5d ']' */
    { 0, 150, 13, 25, 0, 0 }, /* 0x5e '^' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x5f '_' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x60 '`' */
    { 16, 100, 13, 25, 0, 0 }, /* 0x61 'a' */
    { 32, 100, 13, 25, 0, 0 }, /* 0x62 'b' */
    { 48, 100, 13, 25, 0, 0 }, /* 0x63 'c' */
    { 64, 100, 13, 25, 0, 0 }, /* 0x64 'd' */
    { 80, 100, 12, 25, 0, 0 }, /* 0x65 'e' */
    { 96, 100, 14, 25, 0, 0 }, /* 0x66 'f' */
    { 112, 100, 13, 25, 0, 0 }, /* 0x67 'g' */
    { 128, 100, 13, 25, 0, 0 }, /* 0x68 'h' */
    { 144, 100, 6, 25, 0, 0 }, /* 0x69 'i' */
    { 160, 100, 9, 25, 0, 0 }, /* 0x6a 'j' */
    { 176, 100, 10, 25, 0, 0 }, /* 0x6b 'k' */
    { 192, 100, 5, 25, 0, 0 }, /* 0x6c 'l' */
    { 208, 100, 15, 25, 0, 0 }, /* 0x6d 'm' */
    { 224, 100, 13, 25, 0, 0 }, /* 0x6e 'n' */
    { 240, 100, 13, 25, 0, 0 }, /* 0x6f 'o' */
    { 0, 125, 12, 25, 0, 0 }, /* 0x70 'p' */
    { 16, 125, 12, 25, 0, 0 }, /* 0x71 'q' */
    { 32, 125, 13, 25, 0, 0 }, /* 0x72 'r' */
    { 48, 125, 14, 25, 0, 0 }, /* 0x73 's' */
    { 64, 125, 14, 25, 0, 0 }, /* 0x74 't' */
    { 80, 125, 14, 25, 0, 0 }, /* 0x75 'u' */
    { 96, 125, 14, 25, 0, 0 }, /* 0x76 'v' */
    { 112, 125, 16, 25, 0, 0 }, /* 0x77 'w' */
    { 128, 125, 14, 25, 0, 0 }, /* 0x78 'x' */
    { 144, 125, 14, 25, 0, 0 }, /* 0x79 'y' */
    { 160, 125, 15, 25, 0, 0 }, /* 0x7a 'z' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x7b '{' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x7c '|' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x7d '}' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x7e '~' */
    { 0, 0, 0, 25, 0, 0 }, /* 0x7f */
    { 0, 0, 0, 25, 0, 0 }, /* 0x80 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x81 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x82 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x83 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x84 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x85 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x86 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x87 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x88 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x89 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x8a */
    { 0, 0, 0, 25, 0, 0 }, /* 0x8b */
    { 40, 175, 20, 25, 0, -6 }, /* 0x8c */
    { 0, 0, 0, 25, 0, 0 }, /* 0x8d */
    { 0, 0, 0, 25, 0, 0 }, /* 0x8e */
    { 0, 0, 0, 25, 0, 0 }, /* 0x8f */
    { 0, 0, 0, 25, 0, 0 }, /* 0x90 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x91 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x92 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x93 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x94 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x95 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x96 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x97 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x98 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x99 */
    { 0, 0, 0, 25, 0, 0 }, /* 0x9a */
    { 0, 0, 0, 25, 0, 0 }, /* 0x9b */
    { 80, 175, 20, 25, 0, -6 }, /* 0x9c */
    { 0, 0, 0, 25, 0, 0 }, /* 0x9d */
    { 0, 0, 0, 25, 0, 0 }, /* 0x9e */
    { 0, 0, 0, 25, 0, 0 }, /* 0x9f */
    { 0, 0, 0, 25, 0, 0 }, /* 0xa0 */
    { 120, 175, 7, 25, 0, -6 }, /* 0xa1 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xa2 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xa3 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xa4 */
    { 60, 0, 20, 25, 0, 0 }, /* 0xa5 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xa6 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xa7 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xa8 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xa9 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xaa */
    { 140, 175, 16, 25, 0, -6 }, /* 0xab */
    { 0, 0, 0, 25, 0, 0 }, /* 0xac */
    { 0, 0, 0, 25, 0, 0 }, /* 0xad */
    { 0, 0, 0, 25, 0, 0 }, /* 0xae */
    { 0, 0, 0, 25, 0, 0 }, /* 0xaf */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb0 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb1 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb2 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb3 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb4 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb5 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb6 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb7 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb8 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xb9 */
    { 100, 225, 9, 25, 0, -6 }, /* 0xba */
    { 160, 175, 15, 25, 0, -6 }, /* 0xbb */
    { 0, 0, 0, 25, 0, 0 }, /* 0xbc */
    { 0, 0, 0, 25, 0, 0 }, /* 0xbd */
    { 0, 0, 0, 25, 0, 0 }, /* 0xbe */
    { 100, 175, 13, 25, 0, -6 }, /* 0xbf */
    { 180, 175, 15, 25, 0, -6 }, /* 0xc0 */
    { 200, 175, 15, 25, 0, -6 }, /* 0xc1 */
    { 220, 175, 15, 25, 0, -6 }, /* 0xc2 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xc3 */
    { 240, 175, 15, 25, 0, -6 }, /* 0xc4 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xc5 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xc6 */
    { 20, 175, 15, 25, 0, 0 }, /* 0xc7 */
    { 0, 200, 12, 25, 0, -6 }, /* 0xc8 */
    { 20, 200, 12, 25, 0, -6 }, /* 0xc9 */
    { 40, 200, 12, 25, 0, -6 }, /* 0xca */
    { 60, 200, 13, 25, 0, -6 }, /* 0xcb */
    { 80, 200, 12, 25, 0, -6 }, /* 0xcc */
    { 100, 200, 12, 25, 0, -6 }, /* 0xcd */
    { 120, 200, 14, 25, 0, -6 }, /* 0xce */
    { 140, 200, 12, 25, 0, -6 }, /* 0xcf */
    { 0, 0, 0, 25, 0, 0 }, /* 0xd0 */
    { 40, 225, 14, 25, 0, -6 }, /* 0xd1 */
    { 160, 200, 14, 25, 0, -6 }, /* 0xd2 */
    { 180, 200, 14, 25, 0, -6 }, /* 0xd3 */
    { 200, 200, 15, 25, 0, -6 }, /* 0xd4 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xd5 */
    { 220, 200, 14, 25, 0, -6 }, /* 0xd6 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xd7 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xd8 */
    { 240, 200, 13, 25, 0, -6 }, /* 0xd9 */
    { 0, 225, 13, 25, 0, -6 }, /* 0xda */
    { 20, 225, 14, 25, 0, -6 }, /* 0xdb */
    { 40, 225, 13, 25, 0, -6 }, /* 0xdc */
    { 0, 0, 0, 25, 0, 0 }, /* 0xdd */
    { 0, 0, 0, 25, 0, 0 }, /* 0xde */
    { 60, 175, 16, 25, 0, -6 }, /* 0xdf */
    { 176, 125, 14, 25, 0, -6 }, /* 0xe0 */
    { 192, 125, 14, 25, 0, -6 }, /* 0xe1 */
    { 208, 125, 14, 25, 0, -6 }, /* 0xe2 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xe3 */
    { 224, 125, 13, 25, 0, -6 }, /* 0xe4 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xe5 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xe6 */
    { 0, 175, 0, 25, 0, 0 }, /* 0xe7 */
    { 240, 125, 13, 25, 0, -6 }, /* 0xe8 */
    { 0, 150, 12, 25, 0, -6 }, /* 0xe9 */
    { 16, 150, 12, 25, 0, -6 }, /* 0xea */
    { 32, 150, 14, 25, 0, -6 }, /* 0xeb */
    { 48, 150, 9, 25, 0, -6 }, /* 0xec */
    { 64, 150, 9, 25, 0, -6 }, /* 0xed */
    { 80, 150, 9, 25, 0, -6 }, /* 0xee */
    { 96, 150, 9, 25, 0, -6 }, /* 0xef */
    { 0, 0, 0, 25, 0, 0 }, /* 0xf0 */
    { 240, 150, 14, 25, 0, -6 }, /* 0xf1 */
    { 112, 150, 13, 25, 0, -6 }, /* 0xf2 */
    { 128, 150, 14, 25, 0, -6 }, /* 0xf3 */
    { 144, 150, 14, 25, 0, -6 }, /* 0xf4 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xf5 */
    { 160, 150, 13, 25, 0, -6 }, /* 0xf6 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xf7 */
    { 0, 0, 0, 25, 0, 0 }, /* 0xf8 */
    { 176, 150, 14, 25, 0, -6 }, /* 0xf9 */
    { 192, 150, 13, 25, 0, -6 }, /* 0xfa */
    { 208, 150, 13, 25, 0, -6 }, /* 0xfb */
    { 224, 150, 13, 25, 0, -6 }, /* 0xfc */
    { 0, 0, 0, 25, 0, 0 }, /* 0xfd */
    { 0, 0, 0, 25, 0, 0 }, /* 0xfe */
    { 0, 0, 0, 25, 0, 0 }, /* 0xff */
};
int SUBT_POSX = 2048;
int SUBT_POSY = 2098;
/* TEX0 for the three subtitle font textures. */
static u_long SubtGsTex0[3] = {
    SCE_GS_SET_TEX0(0x3f6d, 4, SCE_GS_PSMT4, 8, 8, 1, 0, 0x3fed, SCE_GS_PSMCT32, 0, 0, 1),
    SCE_GS_SET_TEX0(0x2300, 4, SCE_GS_PSMT4, 8, 10, 1, 0, 0x2500, SCE_GS_PSMCT16, 0, 0, 1),
    SCE_GS_SET_TEX0(0x251c, 4, SCE_GS_PSMT4, 8, 10, 1, 0, 0x27d8, SCE_GS_PSMCT16, 0, 0, 1),
};
static int subtSetNum = 0;

static MCODE_STR *kanji_pp;

static sceGifPacket subtPkSpr;
static SUBT_CODE subt_code[16];
static MCODE_DAT *mcode_dat_pp[256];

void SubtInit(void) {
    subtSetNum = 0;
}

void* SubtKanjiSet(void *adrs) {
    void *ret = kanji_pp;
    kanji_pp = adrs;
    return ret;
}

void SubtClear(void) {
    CmnGifOpenCmnPk(&subtPkSpr);

    sceGifPkAddGsAD(&subtPkSpr, SCE_GS_TEXFLUSH, 0);
    sceGifPkAddGsAD(&subtPkSpr, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0));
    sceGifPkAddGsAD(&subtPkSpr, SCE_GS_TEST_1, GS_TEST_ALPHA_NONZERO);
    sceGifPkAddGsAD(&subtPkSpr, SCE_GS_PRMODECONT, 1);
    sceGifPkAddGsAD(&subtPkSpr, SCE_GS_ALPHA_1, GS_ALPHA_BLEND);

    subtSetNum = 0;
}

void SubtFlash(void) {
    if (subtSetNum != 0) {
        CmnGifCloseCmnPk(&subtPkSpr, 9);
    }
}

void SubtMcodeSet(int code) {
    sceGifPkAddGsAD(&subtPkSpr, SCE_GS_TEX0_1, SubtGsTex0[code]);
    sceGifPkAddGsAD(&subtPkSpr, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, 0, 1, 1, 0, 0));
    sceGifPkAddGsAD(&subtPkSpr, SCE_GS_TEXA, SCE_GS_SET_TEXA(0, 1, 128));
}

MCODE_DAT* codeKanjiCheck(u_char dat0, u_char dat1) {
    u_short      code     = dat0 | (dat1 << 8);
    MCODE_KANJI *kcode_pp = kanji_pp->mcode_kanji;
    int          i        = 0;

    for (i = 0; i < kanji_pp->mcode_max; i++, kcode_pp++) {
        if (kcode_pp->code == code) {
            return (MCODE_DAT*)(&kcode_pp->u);
        }
    }

    return NULL;
}

/* Same function, matching lines too */
/* https://github.com/JunkBox-Library/JunkBox_Lib/blob/255f34bccfd0fab817d71730f6b45cab52062ec8/Lib/tools.c#L2579 */
static void euc2sjis(unsigned char *c1, unsigned char *c2) {
    if (*c1 % 2 == 0) {
        *c2 -= 0x02;
    } else {
        *c2 -= 0x61;
        if (*c2 > 0x7e) {
            (*c2)++;
        }
    }

    if (*c1 < 0xdf) {
        (*c1)++;
        *c1 /= 2;
        *c1 += 0x30;
    } else {
        (*c1)++;
        *c1 /= 2;
        *c1 += 0x70;
    }
}

void SubtMsgPrint(u_char* msg_pp, int xp, int yp, int jap_flag, int mline) {
    u_char *tmp_pp;
    int     line_num;
    int     i, j, k;
    int     hsize;
    int     cnt_all;

    cnt_all = 0;    
    SubtMcodeSet(jap_flag);

    if (*msg_pp == '\0') {
        return;
    }

    WorkClear(&subt_code, sizeof(subt_code));

    line_num = 0;
    hsize    = 13;
    tmp_pp   = msg_pp;

    while (1) {
        if (hsize == 13) {
            hsize = 13;
        }

        if (*tmp_pp == '\0') {
            line_num++;
            break;
        }

        if (jap_flag) {
            u_char dat0 = tmp_pp[0];
            u_char dat1 = tmp_pp[1];

            if (dat0 == '@') {
                line_num++;
            } else if (dat0 != ' ') {
                euc2sjis(&dat0, &dat1);
                if (dat0 == SJIS_FULLWIDTH_AT_HI && dat1 == SJIS_FULLWIDTH_AT_LO) {
                    line_num++;
                } else {
                    mcode_dat_pp[cnt_all] = codeKanjiCheck(dat0, dat1);
                    if (mcode_dat_pp[cnt_all] != NULL) {
                        subt_code[line_num].cnt++;
                        subt_code[line_num].wsize += mcode_dat_pp[cnt_all]->w;
                        cnt_all++;
                    }
                }

                tmp_pp++;
            }
        } else {
            if (*tmp_pp == '@') {
                line_num++;
            } else {
                mcode_dat_pp[cnt_all] = &mcode_ascii[*tmp_pp - 32];
                subt_code[line_num].cnt++;
                subt_code[line_num].wsize += mcode_dat_pp[cnt_all]->w;
                cnt_all++;
            }
        }

        tmp_pp++;
    }

    if (mline != 0 && mline < line_num) {
        line_num = mline;
    }

    for (i = 0, k = 0; i < line_num; i++) {
        int posx = xp - (subt_code[i].wsize / 2);
        int posy = yp + (hsize * i);

        for (j = 0; j < subt_code[i].cnt; j++) {
            MCODE_DAT *mcode_pp = mcode_dat_pp[k++];

            sceGifPkAddGsAD(&subtPkSpr, SCE_GS_PRIM, GS_PRIM_TEX_SPRITE(TRUE));

            sceGifPkAddGsAD(&subtPkSpr, SCE_GS_UV,
                SCE_GS_SET_UV(mcode_pp->u << 4, mcode_pp->v << 4));
            sceGifPkAddGsAD(&subtPkSpr, SCE_GS_XYZ2,
                SCE_GS_SET_XYZ2((posx + mcode_pp->adjx) << 4, (posy + (mcode_pp->adjy / 2)) << 4, 1));
            sceGifPkAddGsAD(&subtPkSpr, SCE_GS_UV,
                SCE_GS_SET_UV((mcode_pp->u + mcode_pp->w) << 4, (mcode_pp->v + mcode_pp->h) << 4));
            sceGifPkAddGsAD(&subtPkSpr, SCE_GS_XYZ2,
                SCE_GS_SET_XYZ2((posx + mcode_pp->w + mcode_pp->adjx) << 4, (posy + ((mcode_pp->h + mcode_pp->adjy) / 2)) << 4, 1));

            posx += mcode_pp->w;
            subtSetNum = 1;
        }
    }
}

void SubtCtrlInit(void *adrs, int ser_f) {
    SubtInit();
    SubtKanjiSet(adrs);

    if (ser_f) {
        SUBT_POSX = 2048;
        SUBT_POSY = 2122;
    } else {
        SUBT_POSX = 2048;
        SUBT_POSY = 2104;
    }
}

void SubtCtrlPrint(JIMAKU_STR *jstr_pp, int line, int time, int lang) {
    JIMAKU_STR *jstr_tmp_pp = &jstr_pp[line];
    int         i;

    for (i = 0; i < jstr_tmp_pp->size; i++) {
        if ((time >= jstr_tmp_pp->jimaku_dat_pp[i].starTime) &&
            (time <  jstr_tmp_pp->jimaku_dat_pp[i].endTime)) {
            SubtClear();
            SubtMsgPrint(jstr_tmp_pp->jimaku_dat_pp[i].txtData[lang], SUBT_POSX, SUBT_POSY, lang == LANG_JAPANESE, 0);
            SubtFlash();
            return;
        }
    }
}

void SubtTapPrint(u_char *tap_msg_pp, int lang) {
    /* BUG: should be logical OR instead of bitwise? */
    if (tap_msg_pp == NULL | *tap_msg_pp == '\0') {
        return;
    }

    SubtClear();
    SubtMsgPrint(tap_msg_pp, SUBT_POSX, SUBT_POSY, lang == LANG_JAPANESE, 2);
    SubtFlash();
}

void SubtMenuCtrlInit(void *adrs) {
    SubtInit();
    SubtKanjiSet(adrs);
}

void SubtMenuCtrlPrint(u_char *msg_pp, int xp, int yp, int lang) {
    xp += GS_X_COORD(0) >> 4;
    yp += GS_Y_COORD(0) >> 4;

    SubtClear();
    SubtMsgPrint(msg_pp, xp, yp, lang == LANG_JAPANESE, 0);
    SubtFlash();
}

int SubtMsgDataKaijyouCnt(u_char *msg_pp, int jap_flag) {
    u_char *tmp_pp;
    u_char  dat0, dat1;
    int     ret = 1;

    if (*msg_pp == '\0') {
        return NULL;
    }

    tmp_pp = msg_pp;

    while (*tmp_pp != '\0') {
        dat0 = tmp_pp[0];
        dat1 = tmp_pp[1];

        tmp_pp++;

        /* New line */
        if (dat0 == '@') {
            ret++;
        } else if (jap_flag) {
            /* Japanese subt. new line */ 
            euc2sjis(&dat0, &dat1);
            tmp_pp++;

            /* '@' in SJIS - new line */
            if (dat0 == SJIS_FULLWIDTH_AT_HI && dat1 == SJIS_FULLWIDTH_AT_LO) {
                ret++;
            }
        }
    }

    return ret;
}

u_char* SubtMsgDataPos(u_char *msg_pp, int jap_flag, int pos) {
    u_char *tmp_pp;
    u_char  dat0, dat1;
    int     ret = 0;

    if (*msg_pp == '\0') {
        return NULL;
    }

    tmp_pp = msg_pp;

    while (1) {
        if (ret == pos) {
            return tmp_pp;
        }
        if (*tmp_pp == '\0') {
            break;
        }

        dat0 = tmp_pp[0];
        dat1 = tmp_pp[1];
        tmp_pp++;

        /* New line */
        if (dat0 == '@') {
            ret++;
        } else if (jap_flag) {
            euc2sjis(&dat0, &dat1);

            /* '@' in SJIS - new line */
            if (dat0 == SJIS_FULLWIDTH_AT_HI && dat1 == SJIS_FULLWIDTH_AT_LO) {
                ret++;
            }

            tmp_pp++;
        }
    }

    return NULL;
}

void SubtTapPrintWake(u_char *tap_msg_pp, int lang, int lng, int nowp) {
    int cntmax;
    int selpos;

    if (nowp < 0) {
        return;
    }

    if (nowp >= lng) {
        nowp = lng - 1;
    }

    if (tap_msg_pp == NULL) {
        return;
    }

    cntmax = SubtMsgDataKaijyouCnt(tap_msg_pp, lang == LANG_JAPANESE);
    if (cntmax >= 3) {
        cntmax = ((cntmax + 1) / 2 * nowp) / lng * 2;
        tap_msg_pp = SubtMsgDataPos(tap_msg_pp, lang == LANG_JAPANESE, cntmax);
    }

    SubtTapPrint(tap_msg_pp, lang);
}

void SubtCtrlPrintBoxyWipe(JIMAKU_STR *jstr_pp, int line, int time, int lang, void *code_pp) {
    int         i;
    int         lang_f;

    void       *kanjiset_tmp_pp;
    JIMAKU_STR *jstr_tmp_pp = &jstr_pp[line];

    for (i = 0; i < jstr_tmp_pp->size; i++) {
        if ((time >= jstr_tmp_pp->jimaku_dat_pp[i].starTime) &&
            (time <  jstr_tmp_pp->jimaku_dat_pp[i].endTime)) {
            SubtInit();

            kanjiset_tmp_pp = SubtKanjiSet(code_pp);

            lang_f = (lang == LANG_JAPANESE);
            if (lang_f) {
                lang_f = 2;
            }

            SubtClear();
            SubtMsgPrint(jstr_tmp_pp->jimaku_dat_pp[i].txtData[lang], 2048, 2122, lang_f, 0);

            SubtFlash();
            SubtKanjiSet(kanjiset_tmp_pp);
            return;
        }
    }
}
