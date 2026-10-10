#include "menu/menufont.h"

#include "main/etc.h"

#include "menu/menu_mdl.h"
#include "menu/mntm2hed.h"

#include <string.h>

static u_long SubtGsTex0[3] = {
    SCE_GS_SET_TEX0(0x3f6d, 4, SCE_GS_PSMT4, 8, 8, 1, 0, 0x3fed, SCE_GS_PSMCT32, 0, 0, 1),
    SCE_GS_SET_TEX0(0x2300, 4, SCE_GS_PSMT4, 8, 9, 1, 0, 0x2500, SCE_GS_PSMCT16, 0, 0, 1),
    0,
};
static MCODE_ASCII mcode_ascii[] = {
    { 0, 0, 16, 25, 0, 0 },
    { 100, 0, 7, 25, 0, 0 },
    { 200, 0, 8, 25, 0, 0 },
    { 20, 25, 20, 25, 0, 0 },
    { 240, 0, 13, 25, 0, 0 },
    { 0, 25, 20, 25, 0, 0 },
    { 40, 25, 17, 25, 0, 0 },
    { 180, 0, 5, 25, 0, 0 },
    { 140, 0, 11, 25, 0, 0 },
    { 160, 0, 10, 25, 0, 0 },
    { 60, 25, 17, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 20, 0, 6, 25, 0, 0 },
    { 220, 0, 12, 25, 0, 0 },
    { 40, 0, 5, 25, 0, 0 },
    { 180, 225, 12, 25, 0, 0 },
    { 80, 25, 15, 25, 0, 0 },
    { 100, 25, 6, 25, 0, 0 },
    { 120, 25, 12, 25, 0, 0 },
    { 140, 25, 13, 25, 0, 0 },
    { 160, 25, 16, 25, 0, 0 },
    { 180, 25, 14, 25, 0, 0 },
    { 200, 25, 15, 25, 0, 0 },
    { 220, 25, 14, 25, 0, 0 },
    { 240, 25, 15, 25, 0, 0 },
    { 0, 50, 13, 25, 0, 0 },
    { 140, 225, 6, 25, 0, 0 },
    { 120, 225, 6, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 80, 0, 13, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 20, 50, 15, 25, 0, 0 },
    { 40, 50, 14, 25, 0, 0 },
    { 60, 50, 13, 25, 0, 0 },
    { 80, 50, 14, 25, 0, 0 },
    { 100, 50, 11, 25, 0, 0 },
    { 120, 50, 14, 25, 0, 0 },
    { 140, 50, 14, 25, 0, 0 },
    { 160, 50, 13, 25, 0, 0 },
    { 180, 50, 13, 25, 0, 0 },
    { 200, 50, 13, 25, 0, 0 },
    { 220, 50, 12, 25, 0, 0 },
    { 240, 50, 13, 25, 0, 0 },
    { 0, 75, 17, 25, 0, 0 },
    { 20, 75, 14, 25, 0, 0 },
    { 40, 75, 14, 25, 0, 0 },
    { 60, 75, 13, 25, 0, 0 },
    { 80, 75, 15, 25, 0, 0 },
    { 100, 75, 15, 25, 0, 0 },
    { 120, 75, 11, 25, 0, 0 },
    { 140, 75, 15, 25, 0, 0 },
    { 160, 75, 14, 25, 0, 0 },
    { 180, 75, 17, 25, 0, 0 },
    { 200, 75, 20, 25, 0, 0 },
    { 220, 75, 14, 25, 0, 0 },
    { 240, 75, 15, 25, 0, 0 },
    { 0, 100, 15, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 150, 13, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 16, 100, 13, 25, 0, 0 },
    { 32, 100, 13, 25, 0, 0 },
    { 48, 100, 13, 25, 0, 0 },
    { 64, 100, 13, 25, 0, 0 },
    { 80, 100, 12, 25, 0, 0 },
    { 96, 100, 14, 25, 0, 0 },
    { 112, 100, 13, 25, 0, 0 },
    { 128, 100, 13, 25, 0, 0 },
    { 144, 100, 6, 25, 0, 0 },
    { 160, 100, 9, 25, 0, 0 },
    { 176, 100, 10, 25, 0, 0 },
    { 192, 100, 5, 25, 0, 0 },
    { 208, 100, 15, 25, 0, 0 },
    { 224, 100, 13, 25, 0, 0 },
    { 240, 100, 13, 25, 0, 0 },
    { 0, 125, 12, 25, 0, 0 },
    { 16, 125, 12, 25, 0, 0 },
    { 32, 125, 13, 25, 0, 0 },
    { 48, 125, 14, 25, 0, 0 },
    { 64, 125, 14, 25, 0, 0 },
    { 80, 125, 14, 25, 0, 0 },
    { 96, 125, 14, 25, 0, 0 },
    { 112, 125, 16, 25, 0, 0 },
    { 128, 125, 14, 25, 0, 0 },
    { 144, 125, 14, 25, 0, 0 },
    { 160, 125, 15, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 40, 175, 20, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 80, 175, 20, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 120, 175, 7, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 60, 0, 20, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 140, 175, 16, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 100, 225, 9, 25, 0, -6 },
    { 160, 175, 15, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 100, 175, 13, 25, 0, -6 },
    { 180, 175, 15, 25, 0, -6 },
    { 200, 175, 15, 25, 0, -6 },
    { 220, 175, 15, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 240, 175, 15, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 20, 175, 15, 25, 0, 0 },
    { 0, 200, 12, 25, 0, -6 },
    { 20, 200, 12, 25, 0, -6 },
    { 40, 200, 12, 25, 0, -6 },
    { 60, 200, 13, 25, 0, -6 },
    { 80, 200, 12, 25, 0, -6 },
    { 100, 200, 12, 25, 0, -6 },
    { 120, 200, 14, 25, 0, -6 },
    { 140, 200, 12, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 40, 225, 14, 25, 0, -6 },
    { 160, 200, 14, 25, 0, -6 },
    { 180, 200, 14, 25, 0, -6 },
    { 200, 200, 15, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 220, 200, 14, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 240, 200, 13, 25, 0, -6 },
    { 0, 225, 13, 25, 0, -6 },
    { 20, 225, 14, 25, 0, -6 },
    { 40, 225, 13, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 60, 175, 16, 25, 0, -6 },
    { 176, 125, 14, 25, 0, -6 },
    { 192, 125, 14, 25, 0, -6 },
    { 208, 125, 14, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 224, 125, 13, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 175, 0, 25, 0, 0 },
    { 240, 125, 13, 25, 0, -6 },
    { 0, 150, 12, 25, 0, -6 },
    { 16, 150, 12, 25, 0, -6 },
    { 32, 150, 14, 25, 0, -6 },
    { 48, 150, 9, 25, 0, -6 },
    { 64, 150, 9, 25, 0, -6 },
    { 80, 150, 9, 25, 0, -6 },
    { 96, 150, 9, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 240, 150, 14, 25, 0, -6 },
    { 112, 150, 13, 25, 0, -6 },
    { 128, 150, 14, 25, 0, -6 },
    { 144, 150, 14, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 160, 150, 13, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 176, 150, 14, 25, 0, -6 },
    { 192, 150, 13, 25, 0, -6 },
    { 208, 150, 13, 25, 0, -6 },
    { 224, 150, 13, 25, 0, -6 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
    { 0, 0, 0, 25, 0, 0 },
};
static MCODE_ASCII mcode_HalfSpace = { 0, 0, 10, 25, 0, 0 };
static MCODE_KANJI_ANIME PadSymbolFontA[] = {
    { MCODE_SJIS(0x81, 0x9b) /* ○ */, 0, 0, 24, 24, 0, 0, { 0, 0, 0, 0, 0, 0 } },
    { MCODE_SJIS(0x81, 0x7e) /* × */, 24, 0, 24, 24, 0, 0, { 0, 0, 0, 0, 0, 0 } },
    { MCODE_SJIS(0x81, 0xa2) /* △ */, 48, 0, 24, 24, 0, 0, { 0, 0, 0, 0, 0, 0 } },
    { MCODE_SJIS(0x81, 0xa0) /* □ */, 72, 0, 24, 24, 0, 0, { 0, 0, 0, 0, 0, 0 } },
    { MCODE_SJIS(0x81, 0xa9) /* ← */, MCODE_PAD_ARROW, MCODE_ARROW_LEFT, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { MCODE_SJIS(0x81, 0xa8) /* → */, MCODE_PAD_ARROW, MCODE_ARROW_RIGHT, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { MCODE_SJIS(0x81, 0xaa) /* ↑ */, MCODE_PAD_ARROW, MCODE_ARROW_UP, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { MCODE_SJIS(0x81, 0xab) /* ↓ */, MCODE_PAD_ARROW, MCODE_ARROW_DOWN, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { MCODE_SJIS(0x81, 0x7b) /* ＋ */, MCODE_PAD_ARROW, MCODE_ARROW_ALL, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { MCODE_SJIS(0x81, 0x62) /* ｜ */, MCODE_PAD_ARROW, MCODE_ARROW_UP | MCODE_ARROW_DOWN, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { MCODE_SJIS(0x81, 0x81) /* ＝ */, MCODE_PAD_ARROW, MCODE_ARROW_LEFT | MCODE_ARROW_RIGHT, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { MCODE_SJIS(0x81, 0x96) /* ＊ */, MCODE_PAD_ARROW, MCODE_ARROW_LIVE, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
};
/* D-pad glyph: the base pad, then the lit left, right, up and down arrows. */
MCODE_DAT ArowPat[] = {
    { 0, 24, 40, 32, 0, -4 },
    { 40, 24, 40, 32, 0, -4 },
    { 80, 24, 40, 32, 0, -4 },
    { 0, 56, 40, 32, 0, -4 },
    { 40, 56, 40, 32, 0, -4 },
};
static MCODE_DAT TsFont[] = {
    { 0, 1, 17, 17, 0, 0 },
    { 18, 1, 12, 17, 0, 0 },
    { 36, 1, 13, 17, 0, 0 },
    { 54, 1, 18, 17, 0, 0 },
    { 72, 1, 17, 17, 0, 0 },
    { 90, 1, 18, 17, 0, 0 },
    { 108, 1, 18, 17, 0, 0 },
    { 126, 1, 13, 17, 0, 0 },
    { 144, 1, 16, 17, 0, 0 },
    { 162, 1, 16, 17, 0, 0 },
    { 180, 1, 18, 17, 0, 0 },
    { 198, 1, 18, 17, 0, 0 },
    { 216, 1, 9, 17, 0, 0 },
    { 234, 1, 17, 17, 0, 0 },
    { 0, 19, 9, 17, 0, 0 },
    { 18, 19, 17, 17, 0, 0 },
    { 36, 19, 17, 17, 0, 0 },
    { 54, 19, 12, 17, 3, 0 },
    { 72, 19, 14, 17, 1, 0 },
    { 90, 19, 16, 17, 0, 0 },
    { 108, 19, 16, 17, 0, 0 },
    { 126, 19, 17, 17, 0, 0 },
    { 144, 19, 17, 17, 1, 0 },
    { 162, 19, 17, 17, 0, 0 },
    { 180, 19, 17, 17, 0, 0 },
    { 198, 19, 17, 17, 1, 0 },
    { 216, 19, 12, 17, 0, 0 },
    { 234, 19, 12, 17, 0, 0 },
    { 0, 37, 17, 17, 0, 0 },
    { 18, 37, 17, 17, 0, 0 },
    { 36, 37, 17, 17, 0, 0 },
    { 54, 37, 17, 17, 0, 0 },
    { 72, 37, 17, 17, 0, 0 },
    { 90, 37, 18, 17, 0, 0 },
    { 108, 37, 17, 17, 0, 0 },
    { 126, 37, 16, 17, 0, 0 },
    { 144, 37, 17, 17, 0, 0 },
    { 162, 37, 15, 17, 0, 0 },
    { 180, 37, 16, 17, 0, 0 },
    { 198, 37, 17, 17, 0, 0 },
    { 216, 37, 17, 17, 0, 0 },
    { 234, 37, 14, 17, 0, 0 },
    { 0, 55, 15, 17, 0, 0 },
    { 18, 55, 16, 17, 0, 0 },
    { 36, 55, 16, 17, 0, 0 },
    { 54, 55, 18, 17, 0, 0 },
    { 72, 55, 17, 17, 0, 0 },
    { 90, 55, 18, 17, 0, 0 },
    { 108, 55, 17, 17, 0, 0 },
    { 126, 55, 18, 17, 0, 0 },
    { 144, 55, 16, 17, 0, 0 },
    { 162, 55, 14, 17, 0, 0 },
    { 180, 55, 17, 17, 0, 0 },
    { 198, 55, 17, 17, 0, 0 },
    { 216, 55, 18, 17, 0, 0 },
    { 234, 55, 18, 17, 0, 0 },
    { 0, 73, 17, 17, 0, 0 },
    { 18, 73, 17, 17, 0, 0 },
    { 36, 73, 17, 17, 0, 0 },
    { 54, 73, 14, 17, 0, 0 },
    { 72, 73, 19, 17, 0, 0 },
    { 90, 73, 14, 17, 0, 0 },
    { 108, 73, 17, 17, 0, 0 },
    { 126, 73, 17, 17, 0, 0 },
    { 144, 73, 13, 17, 0, 0 },
    { 162, 73, 17, 17, 0, 0 },
    { 180, 73, 17, 17, 0, 0 },
    { 198, 73, 17, 17, 0, 0 },
    { 216, 73, 17, 17, 0, 0 },
    { 234, 73, 16, 17, 0, 0 },
    { 0, 91, 17, 17, 0, 0 },
    { 18, 91, 16, 17, 0, 0 },
    { 36, 91, 17, 17, 0, 0 },
    { 54, 91, 13, 17, 1, 0 },
    { 72, 91, 15, 17, 0, 0 },
    { 90, 91, 15, 17, 0, 0 },
    { 108, 91, 13, 17, 1, 0 },
    { 126, 91, 19, 17, 0, 0 },
    { 144, 91, 18, 17, 0, 0 },
    { 162, 91, 17, 17, 0, 0 },
    { 180, 91, 17, 17, 0, 0 },
    { 198, 91, 18, 17, 0, 0 },
    { 216, 91, 17, 17, 0, 0 },
    { 234, 91, 17, 17, 0, 0 },
    { 0, 109, 17, 17, 0, 0 },
    { 18, 109, 18, 17, 0, 0 },
    { 36, 109, 18, 17, 0, 0 },
    { 54, 109, 18, 17, 0, 0 },
    { 72, 109, 18, 17, 0, 0 },
    { 90, 109, 17, 17, 0, 0 },
    { 108, 109, 17, 17, 0, 0 },
    { 126, 109, 14, 17, 0, 0 },
    { 144, 109, 13, 17, 0, 0 },
    { 162, 109, 14, 17, 0, 0 },
    { 180, 109, 17, 17, 0, 0 },
};
static char Tbl_ASC2EUC[193] = {
    -95, -95, -95, -86, -95, -55, -95, -12, -95, -16, -95, -13, -95, -11, -95, -57, -95, -54, -95,
    -53, -95, -10, -95, -36, -95, -92, -95, -35, -95, -91, -95, -65, -93, -80, -93, -79, -93, -78,
    -93, -77, -93, -76, -93, -75, -93, -74, -93, -73, -93, -72, -93, -71, -95, -89, -95, -88, -95,
    -29, -95, -31, -95, -28, -95, -87, -95, -9, -93, -63, -93, -62, -93, -61, -93, -60, -93, -59,
    -93, -58, -93, -57, -93, -56, -93, -55, -93, -54, -93, -53, -93, -52, -93, -51, -93, -50, -93,
    -49, -93, -48, -93, -47, -93, -46, -93, -45, -93, -44, -93, -43, -93, -42, -93, -41, -93, -40,
    -93, -39, -93, -38, -95, -50, -95, -17, -95, -49, -95, -80, -95, -95, -95, -58, -93, -31, -93,
    -30, -93, -29, -93, -28, -93, -27, -93, -26, -93, -25, -93, -24, -93, -23, -93, -22, -93, -21,
    -93, -20, -93, -19, -93, -18, -93, -17, -93, -16, -93, -15, -93, -14, -93, -13, -93, -12, -93,
    -11, -93, -10, -93, -9, -93, -8, -93, -7, -93, -6, -95, -48, -95, -61, -95, -47, -95, -63, -95,
    -95, 0,
};
static int _PadFont_SW = 0;
static int _PadArrowState = 0;
static int _AnimeFontFlg;
static MCODE_STR *kanji_pp;

static MCODE_CHAR mcode_dat_pp[512];
static MNFONT_INFO MnSubtFontInfo[SUBTN_END]; /* texture and colour per glyph kind, indexed by MCODE_CHAR::flg */

static void _PKFontPut(SPR_PKT pk, SPR_PRM *spr, SUBT_CODE *psubt, int line_num, int xp, int yp, int pflg, int hsize, float rtx, float rty);
static void _PADArrow_Put(SPR_PKT pk, SPR_PRM *spr, MCODE_DAT *pfnt, int x, int y);
static int _JPFont_GetSubtCode(u_char *str, SUBT_CODE *subt_code);
static int _EGFont_GetSubtCode(u_char *str, SUBT_CODE *subt_code, MCODE_DAT *pfnt_ascii);
static MCODE_DAT* codeKanjiCheck(u_char dat0, u_char dat1, MCODE_KANJI *kcode_pp, int kcode_max);
static MCODE_DAT* codeKanjiACheck(u_char dat0, u_char dat1, MCODE_KANJI_ANIME *kcode_pp, int kcode_max);
static void euc2sjis(unsigned char *c1, unsigned char *c2);

void MenuFont_ASC2EUC(char *des, char *src) {
    u_int len = strlen(Tbl_ASC2EUC) / 2;
    u_char c;

    for (; (c = *src) != '\0'; src++) {
        if (c >= ' ' && (c - ' ') < len) {
            c = (c - ' ') * 2;
            *des++ = Tbl_ASC2EUC[c + 0];
            *des++ = Tbl_ASC2EUC[c + 1];
        } else {
            *des++ = 0xa1; /* Full-width space */
            *des++ = 0xa1;
        }
    }

    *des = '\0';
}

void MENUSubtSetKanji(void *kanji_data_top) {
    kanji_pp = (MCODE_STR*)kanji_data_top;
}

void MENUSubt_PadFontSw(int flg) {
    _PadFont_SW = flg;
}

void MENUSubt_PadFontArrowSet(int flg) {
    _PadArrowState = 0;

    if (flg & MCODE_ARROW_LEFT) {
        _PadArrowState = MCODE_ARROW_LEFT;
    }
    if (flg & MCODE_ARROW_RIGHT) {
        _PadArrowState |= MCODE_ARROW_RIGHT;
    }
    if (flg & MCODE_ARROW_UP) {
        _PadArrowState |= MCODE_ARROW_UP;
    }
    if (flg & MCODE_ARROW_DOWN) {
        _PadArrowState |= MCODE_ARROW_DOWN;
    }
}

int MENUSubtGetLine(u_char *str, int lflg) {
    SUBT_CODE subt_code[16] = {};
    int       line;

    if (lflg == LANG_JAPANESE) {
        line = _JPFont_GetSubtCode(str, subt_code);
    } else {
        line = _EGFont_GetSubtCode(str, subt_code, mcode_ascii);
    }

    return line;
}

void MENUSubtPut(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str, int lflg) {
    SUBT_CODE subt_code[16] = {};
    int       line;
    u_long    tex0;

    MnSubtFontInfo[0].tex0 = SubtGsTex0[0];
    MnSubtFontInfo[0].abgr = abgr;

    MnSubtFontInfo[1].tex0 = SubtGsTex0[1];
    MnSubtFontInfo[1].abgr = abgr;

    tex0 = TsGetTM2Hed(1)->GsTex0;
    tex0 |= SCE_GS_SET_TEX0(0, 0, 0, 0, 0, 1 /* RGBA */, 0, 0, 0, 0, 0, 0);
    
    MnSubtFontInfo[2].tex0 = tex0;
    MnSubtFontInfo[2].abgr = SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);

    if (lflg == LANG_JAPANESE) {
        line = _JPFont_GetSubtCode(str, subt_code);
    } else {
        line = _EGFont_GetSubtCode(str, subt_code, mcode_ascii);
    }

    if (line != 0) {
        spr->rgba0 = abgr;

        _AnimeFontFlg = ((MNSceneGetMusicFitTimer() % 40) > 20);

        spr->zx = 1.0f;
        spr->zy = 0.47f;

        _PKFontPut(pk, spr, subt_code, line, x, y, flg, 0x1a, spr->zx, spr->zy);
    }
}

void MENUFontPutL(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str) {
    SUBT_CODE subt_code[16] = {};
    int       line;
    u_long    tex0;

    line = _EGFont_GetSubtCode(str, subt_code, mcode_ascii);
    if (line == 0) {
        return;
    }

    MnSubtFontInfo[0].tex0 = SubtGsTex0[0];
    MnSubtFontInfo[0].abgr = abgr;

    MnSubtFontInfo[1].tex0 = SubtGsTex0[1];
    MnSubtFontInfo[1].abgr = abgr;

    tex0 = TsGetTM2Hed(1)->GsTex0;
    tex0 |= SCE_GS_SET_TEX0(0, 0, 0, 0, 0, 1 /* RGBA */, 0, 0, 0, 0, 0, 0);

    MnSubtFontInfo[2].tex0 = tex0;
    MnSubtFontInfo[2].abgr = SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);

    _AnimeFontFlg = ((MNSceneGetMusicFitTimer() % 40) > 20);

    spr->zx = 1.0f;
    spr->zy = 0.5f;

    _PKFontPut(pk, spr, subt_code, line, x, y, flg, 24, spr->zx, spr->zy);
}

void MENUFontPutS(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str) {
    SUBT_CODE subt_code[16] = {};
    int       line;
    u_long    tex0;

    line = _EGFont_GetSubtCode(str, subt_code, TsFont);
    if (line == 0) {
        return;
    }

    tex0 = TsGetTM2Hed(0)->GsTex0;
    tex0 |= SCE_GS_SET_TEX0(0, 0, 0, 0, 0, 1 /* RGBA */, 0, 0, 0, 0, 0, 0);

    MnSubtFontInfo[0].tex0 = tex0;
    MnSubtFontInfo[0].abgr = abgr;

    MnSubtFontInfo[1].tex0 = SubtGsTex0[1];
    MnSubtFontInfo[1].abgr = abgr;

    tex0 = TsGetTM2Hed(1)->GsTex0;
    tex0 |= SCE_GS_SET_TEX0(0, 0, 0, 0, 0, 1 /* RGBA */, 0, 0, 0, 0, 0, 0);

    MnSubtFontInfo[2].tex0 = tex0;
    MnSubtFontInfo[2].abgr = SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);

    _AnimeFontFlg = ((MNSceneGetMusicFitTimer() % 40) > 20);

    spr->zx = 0.6f;
    spr->zy = 0.44999999f;

    _PKFontPut(pk, spr, subt_code, line, x, y, flg, 16, spr->zx, spr->zy);
}

void MENUFontPutR(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str, float dw) {
    SUBT_CODE subt_code[16] = {};
    int       line;
    u_long    tex0;

    line = _EGFont_GetSubtCode(str, subt_code, TsFont);
    if (line == 0) {
        return;
    }

    tex0 = TsGetTM2Hed(0)->GsTex0;
    tex0 |= SCE_GS_SET_TEX0(0, 0, 0, 0, 0, 1 /* RGBA */, 0, 0, 0, 0, 0, 0);

    MnSubtFontInfo[0].tex0 = tex0;
    MnSubtFontInfo[0].abgr = abgr;

    MnSubtFontInfo[1].tex0 = SubtGsTex0[1];
    MnSubtFontInfo[1].abgr = abgr;

    tex0 = TsGetTM2Hed(1)->GsTex0;
    tex0 |= SCE_GS_SET_TEX0(0, 0, 0, 0, 0, 1 /* RGBA */, 0, 0, 0, 0, 0, 0);

    MnSubtFontInfo[2].tex0 = tex0;
    MnSubtFontInfo[2].abgr = SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);

    _AnimeFontFlg = ((MNSceneGetMusicFitTimer() % 40) > 20);

    spr->zx = 0.6f;
    spr->zy = 0.38f;

    _PKFontPut(pk, spr, subt_code, line, x, y, flg, 16, spr->zx * dw, spr->zy);
}

static void _PKFontPut(SPR_PKT pk, SPR_PRM *spr, SUBT_CODE *psubt, int line_num, int xp, int yp, int pflg, int hsize, float rtx, float rty) {
    int         i, j;
    MCODE_CHAR *ppMcode;
    int         oflg = -1;

    hsize = ((hsize * rty) + 0.5f);

    switch (pflg & MNFONT_VALIGN_MASK) {
    case MNFONT_VALIGN_MIDDLE:
        yp = (yp - ((line_num * hsize) >> 1));
        break;
    case MNFONT_VALIGN_BOTTOM:
        yp = (yp - (line_num * hsize) + 1);
        break;
    }

    ppMcode = mcode_dat_pp;
    for (i = 0; i < line_num; i++) {
        int x;
        int y;
        int w;

        switch (pflg & MNFONT_HALIGN_MASK) {
        case MNFONT_HALIGN_CENTER:
            x = (xp - (((psubt[i].wsize * rtx) + 0.5f) * 0.5f));
            break;
        case MNFONT_HALIGN_RIGHT:
            x = (xp - ((psubt[i].wsize * rtx) + 0.5f)) + 1.0f;
            break;
        default:
            x = xp;
            break;
        }

        y = yp + (hsize * i);

        for (j = 0; j < psubt[i].cnt; j++) {
            int        adjx, adjy;
            MCODE_DAT *pfnt = ppMcode->pmcode;
            int        mflg = ppMcode->flg;
            ppMcode++;

            if (oflg != mflg) {
                PkTEX0_Add(pk, MnSubtFontInfo[mflg].tex0);
                spr->rgba0 = MnSubtFontInfo[mflg].abgr;
                oflg = mflg;
            }

            /* Pad symbols are MCODE_KANJI_ANIME entries, so pfnt + 1 is the glyph's
             * alternate frame (apat). Only the d-pad glyphs have one (the pad with no
             * arrow lit); they flash to it on frames 21-39 of every 40 counted by the
             * menu music timer (_AnimeFontFlg). */
            if (mflg == SUBTN_PADSYM_CODE) {
                if ((pfnt + 1)->w != 0) {
                    if (_AnimeFontFlg) {
                        pfnt = pfnt + 1;
                    }
                }
            }

            adjx = (pfnt->adjx * rtx) + 0.5f;
            adjy = (pfnt->adjy * rty) + 0.5f;

            if (pfnt->u & MCODE_PAD_ARROW) {
                _PADArrow_Put(pk, spr, pfnt, x + adjx, y + adjy);
            } else {
                spr->ux = pfnt->u;
                spr->uy = pfnt->v;
                spr->uw = pfnt->w;
                spr->uh = pfnt->h;

                spr->px = x + adjx;
                spr->py = y + adjy;
                spr->sw = pfnt->w;
                spr->sh = pfnt->h;

                PkNSprite_Add2(pk, spr, PKSPR_UV_RECT | PKSPR_ZOOM);
            }

            w = pfnt->w * rtx + 0.5f;
            x += w;
        }
    }
}

static void _PADArrow_Put(SPR_PKT pk, SPR_PRM *spr, MCODE_DAT *pfnt, int x, int y) {
    int        i;
    int        aflg;
    MCODE_DAT *pat;

    if (pfnt->v & MCODE_ARROW_LIVE) {
        aflg = _PadArrowState;
    } else {
        aflg = pfnt->v;
    }

    pat = ArowPat;

    spr->px = x;
    spr->py = y;
    
    spr->ux = pat->u;
    spr->uy = pat->v;

    spr->uw = pat->w;
    spr->uh = pat->h;

    spr->sw = pat->w;
    spr->sh = pat->h;

    PkNSprite_Add2(pk, spr, PKSPR_UV_RECT | PKSPR_ZOOM);
    pat++;

    for (i = 0; i < 4; i++, pat++, aflg >>= 1) {
        if (aflg & 1) {
            spr->px = x;
            spr->py = y;

            spr->ux = pat->u;
            spr->uy = pat->v;

            spr->uw = pat->w;
            spr->uh = pat->h;

            spr->sw = pat->w;
            spr->sh = pat->h;

            PkNSprite_Add2(pk, spr, PKSPR_UV_RECT | PKSPR_ZOOM);
        }
    }
}

static int _JPFont_GetSubtCode(u_char *str, SUBT_CODE *subt_code) {
    int         line_num;
    MCODE_CHAR *ppMcode;

    if (*str == '\0') {
        return 0;
    }

    line_num = 0;
    ppMcode = mcode_dat_pp;

    while (1) {
        u_char dat0, dat1;

        dat0 = *str++;

        if (dat0 == '\0') {
            line_num++;
            break;
        }

        /* '@' - new line */
        if (dat0 == '@') {
            line_num++;
            continue;
        }

        if (dat0 == ' ') {
            ppMcode->pmcode = &mcode_HalfSpace;
            subt_code[line_num].wsize += ppMcode->pmcode->w;
            subt_code[line_num].cnt++;
            ppMcode->flg = SUBTN_ETC_CODE;
            ppMcode++;
            continue;
        }

        dat1 = *str++;
        euc2sjis(&dat0, &dat1);

        /* '@' in SJIS - new line */
        if (dat0 == 0x81 && dat1 == 0x97) {
            line_num++;
            continue;
        }

        if (_PadFont_SW) {
            ppMcode->pmcode = codeKanjiACheck(dat0, dat1, PadSymbolFontA, PR_ARRAYSIZE(PadSymbolFontA));
            if (ppMcode->pmcode != NULL) {
                subt_code[line_num].cnt++;
                subt_code[line_num].wsize += ppMcode->pmcode->w;
                ppMcode->flg = SUBTN_PADSYM_CODE;
                ppMcode++;
                continue;
            }
        }

        ppMcode->pmcode = codeKanjiCheck(dat0, dat1, kanji_pp->mcode_kanji, kanji_pp->mcode_max);
        if (ppMcode->pmcode != NULL) {
            subt_code[line_num].cnt++;
            subt_code[line_num].wsize += ppMcode->pmcode->w;
            ppMcode->flg = SUBTN_KANJI_CODE;
            ppMcode++;
        }
    }

    return line_num;
}

static int _EGFont_GetSubtCode(u_char *str, SUBT_CODE *subt_code, MCODE_DAT *pfnt_ascii) {
    int         line_num;
    MCODE_CHAR *ppMcode;

    if (*str == '\0') {
        return 0;
    }

    line_num = 0;
    ppMcode = mcode_dat_pp;

    while (1) {
        u_char c = *str++;


        if (c == '\0') {
            line_num++;
            break;
        }
        if (c == '@') {
            line_num++;
        }

        if (c < ' ') {
            continue;
        }

        if (_PadFont_SW) {
            u_char dat0, dat1;

            dat0 = c;
            dat1 = *str;
            euc2sjis(&dat0, &dat1);

            ppMcode->pmcode = codeKanjiACheck(dat0, dat1, PadSymbolFontA, PR_ARRAYSIZE(PadSymbolFontA));
            if (ppMcode->pmcode != NULL) {
                str++;

                subt_code[line_num].cnt++;
                subt_code[line_num].wsize += ppMcode->pmcode->w;

                ppMcode->flg = SUBTN_PADSYM_CODE;
                ppMcode++;
                continue;
            }
        }

        /* The ASCII glyph tables (mcode_ascii, TsFont) start at the space character. */
        ppMcode->pmcode = &pfnt_ascii[c - ' '];
        subt_code[line_num].wsize += ppMcode->pmcode->w;
        subt_code[line_num].cnt++;

        ppMcode->flg = SUBTN_ETC_CODE;
        ppMcode++;
    }

    return line_num;
}

static MCODE_DAT* codeKanjiCheck(u_char dat0, u_char dat1, MCODE_KANJI *kcode_pp, int kcode_max) {
    u_short code = dat0 | (dat1 << 8);
    int     i    = 0;

    for (i = 0; i < kcode_max; i++, kcode_pp++) {
        if (kcode_pp->code == code) {
            return (MCODE_DAT*)(&kcode_pp->u);
        }
    }

    return NULL;
}

static MCODE_DAT* codeKanjiACheck(u_char dat0, u_char dat1, MCODE_KANJI_ANIME *kcode_pp, int kcode_max) {
    u_short code = dat0 | (dat1 << 8);
    int     i    = 0;

    for (i = 0; i < kcode_max; i++, kcode_pp++) {
        if (kcode_pp->code == code) {
            return (MCODE_DAT*)(&kcode_pp->u);
        }
    }

    return NULL;
}

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
