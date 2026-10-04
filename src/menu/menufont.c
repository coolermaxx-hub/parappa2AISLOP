#include "menu/menufont.h"

#include "main/etc.h"

#include "menu/menu_mdl.h"
#include "menu/mntm2hed.h"

#include <string.h>

static u_long SubtGsTex0_TmpMenuFont[3] = { 0x2007fda621413f6d, 0x2014a00661412300, 0 };
static MCODE_ASCII mcode_ascii_TmpMenuFont[] = {
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
    { 39809, 0, 0, 24, 24, 0, 0, { 0, 0, 0, 0, 0, 0 } },
    { 32385, 24, 0, 24, 24, 0, 0, { 0, 0, 0, 0, 0, 0 } },
    { 41601, 48, 0, 24, 24, 0, 0, { 0, 0, 0, 0, 0, 0 } },
    { 41089, 72, 0, 24, 24, 0, 0, { 0, 0, 0, 0, 0, 0 } },
    { 43393, 32768, 1, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { 43137, 32768, 2, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { 43649, 32768, 4, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { 43905, 32768, 8, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { 31617, 32768, 15, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { 25217, 32768, 12, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { 33153, 32768, 3, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
    { 38529, 32768, 16, 40, 32, 0, -4, { 0, 24, 40, 32, 0, -4 } },
};
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
static MCODE_STR *kanji_pp_TmpMenuFont;

static MCODE_CHAR mcode_dat_pp[512];
static MNFONT_INFO MnSubtFontInfo[3];

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
    kanji_pp_TmpMenuFont = (MCODE_STR*)kanji_data_top;
}

void MENUSubt_PadFontSw(int flg) {
    _PadFont_SW = flg;
}

void MENUSubt_PadFontArrowSet(int flg) {
    _PadArrowState = 0;

    if (flg & 1) {
        _PadArrowState = 1;
    }
    if (flg & 2) {
        _PadArrowState |= 2;
    }
    if (flg & 4) {
        _PadArrowState |= 4;
    }
    if (flg & 8) {
        _PadArrowState |= 8;
    }
}

int MENUSubtGetLine(u_char *str, int lflg) {
    SUBT_CODE subt_code[16] = {};
    int       line;

    if (lflg == LANG_JAPANESE) {
        line = _JPFont_GetSubtCode(str, subt_code);
    } else {
        line = _EGFont_GetSubtCode(str, subt_code, mcode_ascii_TmpMenuFont);
    }

    return line;
}

void MENUSubtPut(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str, int lflg) {
    SUBT_CODE subt_code[16] = {};
    int       line;
    u_long    tex0;

    MnSubtFontInfo[0].tex0 = SubtGsTex0_TmpMenuFont[0];
    MnSubtFontInfo[0].abgr = abgr;

    MnSubtFontInfo[1].tex0 = SubtGsTex0_TmpMenuFont[1];
    MnSubtFontInfo[1].abgr = abgr;

    tex0 = TsGetTM2Hed(1)->GsTex0;
    tex0 |= SCE_GS_SET_TEX0(0, 0, 0, 0, 0, 1 /* RGBA */, 0, 0, 0, 0, 0, 0);
    
    MnSubtFontInfo[2].tex0 = tex0;
    MnSubtFontInfo[2].abgr = SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);

    if (lflg == LANG_JAPANESE) {
        line = _JPFont_GetSubtCode(str, subt_code);
    } else {
        line = _EGFont_GetSubtCode(str, subt_code, mcode_ascii_TmpMenuFont);
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

    line = _EGFont_GetSubtCode(str, subt_code, mcode_ascii_TmpMenuFont);
    if (line == 0) {
        return;
    }

    MnSubtFontInfo[0].tex0 = SubtGsTex0_TmpMenuFont[0];
    MnSubtFontInfo[0].abgr = abgr;

    MnSubtFontInfo[1].tex0 = SubtGsTex0_TmpMenuFont[1];
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

    MnSubtFontInfo[1].tex0 = SubtGsTex0_TmpMenuFont[1];
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

    MnSubtFontInfo[1].tex0 = SubtGsTex0_TmpMenuFont[1];
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

    switch (pflg & 0xf00) {
    case 0x100:
        yp = (yp - ((line_num * hsize) >> 1));
        break;
    case 0x200:
        yp = (yp - (line_num * hsize) + 1);
        break;
    }

    ppMcode = mcode_dat_pp;
    for (i = 0; i < line_num; i++) {
        int x;
        int y;
        int w;

        switch (pflg & 0xf) {
        case 1:
            x = (xp - (((psubt[i].wsize * rtx) + 0.5f) * 0.5f));
            break;
        case 2:
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

            if (mflg == 2) {
                if ((pfnt + 1)->w != 0) {
                    if (_AnimeFontFlg) {
                        pfnt = pfnt + 1;
                    }
                }
            }

            adjx = (pfnt->adjx * rtx) + 0.5f;
            adjy = (pfnt->adjy * rty) + 0.5f;

            if (pfnt->u & 0x8000) {
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

                PkNSprite_Add2(pk, spr, 3);
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

    if (pfnt->v & 0x10) {
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

    PkNSprite_Add2(pk, spr, 3);
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

            PkNSprite_Add2(pk, spr, 3);
        }
    }
}

static int _JPFont_GetSubtCode(u_char *str, SUBT_CODE *subt_code) {
    int         line_num;
    MCODE_CHAR *ppMcode;
    u_char      dat0, dat1;

    if (*str == '\0') {
        return 0;
    }

    line_num = 0;
    ppMcode = mcode_dat_pp;

    while (1) {
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
            ppMcode->flg = 0;
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
            ppMcode->pmcode = codeKanjiACheck(dat0, dat1, PadSymbolFontA, 12);
            if (ppMcode->pmcode != NULL) {
                subt_code[line_num].cnt++;
                subt_code[line_num].wsize += ppMcode->pmcode->w;
                ppMcode->flg = 2;
                ppMcode++;
                continue;
            }
        }

        ppMcode->pmcode = codeKanjiCheck(dat0, dat1, kanji_pp_TmpMenuFont->mcode_kanji, kanji_pp_TmpMenuFont->mcode_max);
        if (ppMcode->pmcode != NULL) {
            subt_code[line_num].cnt++;
            subt_code[line_num].wsize += ppMcode->pmcode->w;
            ppMcode->flg = 1;
            ppMcode++;
        }
    }

    return line_num;
}

static int _EGFont_GetSubtCode(u_char *str, SUBT_CODE *subt_code, MCODE_DAT *pfnt_ascii) {
    int         line_num;
    MCODE_CHAR *ppMcode;
    u_char      c;
    u_char      dat0, dat1;

    if (*str == '\0') {
        return 0;
    }

    line_num = 0;
    ppMcode = mcode_dat_pp;

    while (1) {
        c = *str++;

        if (c == '\0') {
            line_num++;
            break;
        }
        if (c == '@') {
            line_num++;
        }

        if (c < 32) {
            continue;
        }

        if (_PadFont_SW) {
            dat0 = c;
            dat1 = *str;
            euc2sjis(&dat0, &dat1);

            ppMcode->pmcode = codeKanjiACheck(dat0, dat1, PadSymbolFontA, 12);
            if (ppMcode->pmcode != NULL) {
                str++;

                subt_code[line_num].cnt++;
                subt_code[line_num].wsize += ppMcode->pmcode->w;

                ppMcode->flg = 2;
                ppMcode++;
                continue;
            }
        }

        ppMcode->pmcode = &pfnt_ascii[c - 32];
        subt_code[line_num].wsize += ppMcode->pmcode->w;
        subt_code[line_num].cnt++;

        ppMcode->flg = 0;
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
