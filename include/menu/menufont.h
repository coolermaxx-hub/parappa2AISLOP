#ifndef MENUFONT_H
#define MENUFONT_H

#include "common.h"

#include <eetypes.h>

#include "main/subt.h"
#include "menu/pksprite.h"

enum {
    SUBTN_ETC_CODE,
    SUBTN_KANJI_CODE,
    SUBTN_PADSYM_CODE,
    SUBTN_END
};

typedef struct { // 0x14
    /* 0x00 */ u_short u;
    /* 0x02 */ u_short v;
    /* 0x04 */ u_short w;
    /* 0x06 */ u_short h;
    /* 0x08 */ char adjx;
    /* 0x09 */ char adjy;
    /* 0x0a */ MCODE_DAT apat;
} MCODE_DAT_ANIME;

/* Glyph code of a two-byte Shift-JIS character, first byte in the low half. */
#define MCODE_SJIS(b0, b1) ((b0) | ((b1) << 8))

/* MCODE_DAT::u flag marking a d-pad glyph; its v then holds MCODE_ARROW_* bits. */
#define MCODE_PAD_ARROW 0x8000

/* D-pad arrows lit on a pad glyph, also the MENUSubt_PadFontArrowSet bits. */
#define MCODE_ARROW_LEFT  1
#define MCODE_ARROW_RIGHT 2
#define MCODE_ARROW_UP    4
#define MCODE_ARROW_DOWN  8
#define MCODE_ARROW_ALL   (MCODE_ARROW_LEFT | MCODE_ARROW_RIGHT | MCODE_ARROW_UP | MCODE_ARROW_DOWN)
/* Pad glyph v: light the arrows last set with MENUSubt_PadFontArrowSet. */
#define MCODE_ARROW_LIVE  0x10

typedef struct { // 0x16
    /* 0x00 */ u_short code;
    /* 0x02 */ u_short u;
    /* 0x04 */ u_short v;
    /* 0x06 */ u_short w;
    /* 0x08 */ u_short h;
    /* 0x0a */ char adjx;
    /* 0x0b */ char adjy;
    /* 0x0c */ MCODE_DAT apat;
} MCODE_KANJI_ANIME;

typedef struct { // 0x8
    /* 0x0 */ int flg;
    /* 0x4 */ MCODE_DAT *pmcode;
} MCODE_CHAR;

typedef struct { // 0x10
    /* 0x0 */ u_long tex0;
    /* 0x8 */ u_int abgr;
    /* 0xc */ u_int pad;
} MNFONT_INFO;

void MenuFont_ASC2EUC(char *des, char *src);

void MENUSubtSetKanji(void *kanji_data_top);
void MENUSubt_PadFontSw(int flg);
void MENUSubt_PadFontArrowSet(int flg);

int MENUSubtGetLine(u_char *str, int lflg);

void MENUSubtPut(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str, int lflg);
void MENUFontPutL(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str);
void MENUFontPutS(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str);
void MENUFontPutR(SPR_PKT pk, SPR_PRM *spr, int x, int y, u_int abgr, int flg, u_char *str, float dw);

#endif /* MENUFONT_H */
