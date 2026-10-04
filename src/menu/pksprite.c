#include "menu/pksprite.h"

#include "os/cmngifpk.h"

#include <eestruct.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GIF register descriptors for the packed REGS word; four bits each, first register in the low nibble. */
#define PK_REG_PRIM(slot)  ((u_long)0 << ((slot) * 4))
#define PK_REG_RGBAQ(slot) ((u_long)1 << ((slot) * 4))
#define PK_REG_UV(slot)    ((u_long)3 << ((slot) * 4))
#define PK_REG_XYZF2(slot) ((u_long)4 << ((slot) * 4))
#define PK_REG_AD(slot)    ((u_long)SCE_GIF_PACKED_AD << ((slot) * 4))

/*
 * The register helpers below emit a PACKED GIF tag followed by A+D
 * quadwords, each holding one register value and its address.
 */
typedef struct {
    PKGIFTAG     gif;
    sceGifPackAd ad[3];
} PK_AD_PACKET;

/*
 * SCISSOR keeps each coordinate in its own 16-bit lane, of which the GS
 * uses the low 11 bits. PkSCISSOR_Add stores whole lanes, so out-of-range
 * bits land in the unused lane bits instead of being masked off.
 */
typedef struct {
    short x0, x1, y0, y1;
} PK_SCISSOR_LANES;

static sceGsScissor _PkDefSCISSOR PR_ALIGNED(16) = {
    .SCAX0 = 0, .SCAX1 = SCREEN_WIDTH - 1,
    .SCAY0 = 0, .SCAY1 = SCREEN_FIELD_HEIGHT - 1,
};

static sceGsZbuf _PkDefZBUFFER PR_ALIGNED(16) = {
    .ZBP = 0, .PSM = 0, .ZMSK = 0,
};

static int _PkScrW;
static int _PkScrH;

static void   _tsWorkEnd(TS_WORKMEM *emem);
static u_int* _tsWorkInit(TS_WORKMEM *emem, u_int *buf, u_int size);
static void   PkDefReg_Add(SPR_PKT pkt);
static void   rotcossin(float rot);

static void _tsWorkEnd(TS_WORKMEM *emem) {
    if (emem->isAlloc && emem->top != NULL) {
        free(emem->top);

        emem->top = NULL;
        emem->size = 0;
    }
}

static u_int* _tsWorkInit(TS_WORKMEM *emem, u_int *buf, u_int size) {
    _tsWorkEnd(emem);

    if (buf == NULL) {
        size = (size / 16) * 16;
        if (size != 0) {
            buf = memalign(128, size);
        }

        emem->top = buf;
        emem->size = size;
        emem->isAlloc = TRUE;
    } else {
        emem->top = buf;
        emem->size = size;
        emem->isAlloc = FALSE;
    }

    return emem->top;
}

u_long128* TsInitUPacket(TsUSERPKT *pk, u_long128 *buf, u_int size) {
    u_int top;

    memset(pk, 0, sizeof(*pk));

    top = (u_int)_tsWorkInit(&pk->mem, (u_int*)buf, size);
    if (top == 0) {
        return NULL;
    }

    pk->size = size / 16;

    pk->pkt[0].PaketTop = top;
    pk->pkt[1].PaketTop = top;

    pk->btop = pk->ptop = PR_UNCACHED(pk->pkt[pk->idx].PaketTop);
    return (u_long128*)top;
}

void TsEndUPacket(TsUSERPKT *pk) {
    _tsWorkEnd(&pk->mem);

    pk->pkt[0].PaketTop = NULL;
    pk->pkt[1].PaketTop = NULL;

    pk->size = 0;
    pk->btop = NULL;
    pk->ptop = NULL;
}

void TsDrawUPacket(TsUSERPKT *up) {
    TSPAKET    *pk;
    u_int       qwc;
    sceDmaChan *PktChan;
    sceDmaTag  *tp;
    u_int       top;

    pk  = &up->pkt[up->idx];
    qwc = ((up->ptop & ~PR_UC_ADDR) - pk->PaketTop) / 16;

    if (qwc == 0) {
        return;
    }

    if (qwc > up->size) {
        printf("★パケットSizeOver!!(User)(%x/%x)\n", qwc, up->size);
    }

    tp  = &pk->tag[0];
    top = pk->PaketTop;

    while (1) {
        if (qwc < 0xfff0) {
            sceDmaAddRef(&tp, qwc, (void*)top);
            break;
        }

        sceDmaAddRef(&tp, 0xfff0, (void*)top);
        qwc -= 0xfff0;
        top += (0xfff0 * 16);
    }

    sceDmaAddEnd(&tp, 0, NULL);

    up->idx  = 1 - up->idx;
    up->btop = up->ptop = PR_UNCACHED(up->pkt[up->idx].PaketTop);

    PktChan = sceDmaGetChan(SCE_DMA_GIF);
    FlushCache(WRITEBACK_DCACHE);

    sceDmaSync(PktChan, 0, 0);
    sceDmaSend(PktChan, pk);
}

void PkTEX0_Add(SPR_PKT pkt, u_long texreg) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = texreg;
    pk->ad[0].ADDR = SCE_GS_TEX0_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkTEX1_Add(SPR_PKT pkt, u_long texreg) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = texreg;
    pk->ad[0].ADDR = SCE_GS_TEX1_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkCLAMP_Add(SPR_PKT pkt, u_long texrp) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = texrp;
    pk->ad[0].ADDR = SCE_GS_CLAMP_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkALPHA_Add(SPR_PKT pkt, u_long alpreg) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = alpreg;
    pk->ad[0].ADDR = SCE_GS_ALPHA_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkTEST_Add(SPR_PKT pkt, u_long testsw) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = testsw;
    pk->ad[0].ADDR = SCE_GS_TEST_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkSCISSOR_Add(SPR_PKT pkt, short x, short y, short w, short h) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);

    GS_REG_VIEW(PK_SCISSOR_LANES, pk->ad[0].DATA).x0 = x;
    GS_REG_VIEW(PK_SCISSOR_LANES, pk->ad[0].DATA).x1 = x + w - 1;
    GS_REG_VIEW(PK_SCISSOR_LANES, pk->ad[0].DATA).y0 = y;
    GS_REG_VIEW(PK_SCISSOR_LANES, pk->ad[0].DATA).y1 = y + h - 1;

    pk->ad[0].ADDR = SCE_GS_SCISSOR_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkDefSCISSOR_Add(SPR_PKT pkt) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = GS_REG_WORD(_PkDefSCISSOR);
    pk->ad[0].ADDR = SCE_GS_SCISSOR_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkOFFSET_Add(SPR_PKT pkt, int x, int y) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);

    GS_REG_VIEW(sceGsXyoffset, pk->ad[0].DATA).OFX = x;
    GS_REG_VIEW(sceGsXyoffset, pk->ad[0].DATA).OFY = y;

    pk->ad[0].ADDR = SCE_GS_XYOFFSET_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkPABE_Add(SPR_PKT pkt, u_int flg) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = flg;
    pk->ad[0].ADDR = SCE_GS_PABE;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkFBA_Add(SPR_PKT pkt, u_int flg) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = flg;
    pk->ad[0].ADDR = SCE_GS_FBA_1;

    *pkt = (u_long128*)&pk->ad[1];
}

void PkCCLAMP_Add(SPR_PKT pkt, u_int flg) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = flg;
    pk->ad[0].ADDR = SCE_GS_COLCLAMP;

    *pkt = (u_long128*)&pk->ad[1];
}

static void PkDefReg_Add(SPR_PKT pkt) {
    PK_AD_PACKET *pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 2);
    pk->gif.regs = PK_REG_AD(0) | PK_REG_AD(1);

    pk->ad[0].ADDR = SCE_GS_TEXFLUSH;

    pk->ad[1].DATA = GS_TEXA_STP;
    pk->ad[1].ADDR = SCE_GS_TEXA;

    *pkt = (u_long128*)&pk->ad[2];
}

void PkTEX0_SetAdd(SPR_PKT pkt, int vram, int w, int h, int isLinear) {
    PK_AD_PACKET *pk;
    int           n;
    int           tw, th;

    pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 3);
    pk->gif.regs = PK_REG_AD(0) | PK_REG_AD(1) | PK_REG_AD(2);

    pk->ad[0].ADDR = SCE_GS_TEXFLUSH;

    th = 0;
    tw = 0;

    for (n = 1; n < w; n <<= 1) {
        tw++;
    }
    for (n = 1; n < h; n <<= 1) {
        th++;
    }

    if (tw == 0) {
        tw = 1;
    }
    if (th == 0) {
        th = 1;
    }

    if (isLinear) {
        pk->ad[1].DATA = SCE_GS_SET_TEX1_1(0, 0, 1, 1, 0, 0, 0);
    } else {
        pk->ad[1].DATA = SCE_GS_SET_TEX1_1(0, 0, 0, 0, 0, 0, 0);
    }

    pk->ad[1].ADDR = SCE_GS_TEX1_1;

    pk->ad[2].DATA = SCE_GS_SET_TEX0(vram, (w + 63) / 64, SCE_GS_PSMCT32, tw, th, 1, 0, 0, 0, 0, 0, 0);
    pk->ad[2].ADDR = SCE_GS_TEX0_1;

    *pkt = (u_long128*)&pk->ad[3];
}

u_int GetDToneColor(u_int sbgr, u_int dbgr, int ton) {
    u_char *pd, *ps;
    int     sc, dc;
    int     r, g, b, a;
    u_int   ret;

    ps = (u_char*)&sbgr;
    pd = (u_char*)&dbgr;

    dc = pd[0]; sc = ps[0];
    r = sc + (((dc - sc) * ton) >> 8);

    dc = pd[1]; sc = ps[1];
    g = sc + (((dc - sc) * ton) >> 8);

    dc = pd[2]; sc = ps[2];
    b = sc + (((dc - sc) * ton) >> 8);

    dc = pd[3]; sc = ps[3];
    a = sc + (((dc - sc) * ton) >> 8);

    #if 0
    r = min(max(r, 0), 255);
    g = min(max(g, 0), 255);
    b = min(max(b, 0), 255);
    a = min(max(a, 0), 255);
    ret = (u_int)SCE_GS_SET_RGBAQ(r, g, b, a, 0x0 /* 0.0f */);
    #else
    /*
     * MMI version of the clamp-and-pack above. Like the original, it uses
     * $2, $4 (or $9) and $8 as scratch without declaring them as clobbers;
     * declaring them changes the surrounding register allocation. The
     * three tone colour helpers share this block.
     */
    asm(
        "pcpyld  $2, %4, %3       \n\t"
        "pcpyld  $4, %2, %1       \n\t"
        "ppacw   $2, $2, $4       \n\t"
        "li      $8, 0xff000000ff \n\t"
        "pcpyld  $8, $8, $8       \n\t"
        "pmaxw   $2, $2, $0       \n\t"
        "pminw   $2, $2, $8       \n\t"
        "ppach   $2, $0, $2       \n\t"
        "ppacb   %0, $0, $2       \n\t"
    : "=r"(ret) : "r"(r), "r"(g), "r"(b), "r"(a)
    );
    #endif

    return ret;
}

u_int GetToneColorA(u_int abgr, int tona, int tonb, int tong, int tonr) {
    u_int r, g, b, a;
    u_int ret;

    r = abgr_get_r(abgr);
    g = abgr_get_g(abgr);
    b = abgr_get_b(abgr);
    a = abgr_get_a(abgr);

    r = (r * tonr) >> 4;
    g = (g * tong) >> 4;
    b = (b * tonb) >> 4;
    a = (a * tona) >> 4;

    #if 0
    r = min(max(r, 0), 255);
    g = min(max(g, 0), 255);
    b = min(max(b, 0), 255);
    a = min(max(a, 0), 255);
    ret = (u_int)SCE_GS_SET_RGBAQ(r, g, b, a, 0x0 /* 0.0f */);
    #else
    asm(
        "pcpyld  $2, %4, %3       \n\t"
        "pcpyld  $9, %2, %1       \n\t"
        "ppacw   $2, $2, $9       \n\t"
        "li      $8, 0xff000000ff \n\t"
        "pcpyld  $8, $8, $8       \n\t"
        "pmaxw   $2, $2, $0       \n\t"
        "pminw   $2, $2, $8       \n\t"
        "ppach   $2, $0, $2       \n\t"
        "ppacb   %0, $0, $2       \n\t"
    : "=r"(ret) : "r"(r), "r"(g), "r"(b), "r"(a)
    );
    #endif

    return ret;
}

u_int GetToneColorH(u_int abgr, int tona, int tonb, int tong, int tonr) {
    u_int r, g, b, a;
    u_int ret;

    r = abgr_get_r(abgr);
    g = abgr_get_g(abgr);
    b = abgr_get_b(abgr);
    a = abgr_get_a(abgr);

    r = (r * tonr) >> 8;
    g = (g * tong) >> 8;
    b = (b * tonb) >> 8;
    a = (a * tona) >> 8;

    #if 0
    r = min(max(r, 0), 255);
    g = min(max(g, 0), 255);
    b = min(max(b, 0), 255);
    a = min(max(a, 0), 255);
    ret = (u_int)SCE_GS_SET_RGBAQ(r, g, b, a, 0x0 /* 0.0f */);
    #else
    asm(
        "pcpyld  $2, %4, %3       \n\t"
        "pcpyld  $9, %2, %1       \n\t"
        "ppacw   $2, $2, $9       \n\t"
        "li      $8, 0xff000000ff \n\t"
        "pcpyld  $8, $8, $8       \n\t"
        "pmaxw   $2, $2, $0       \n\t"
        "pminw   $2, $2, $8       \n\t"
        "ppach   $2, $0, $2       \n\t"
        "ppacb   %0, $0, $2       \n\t"
    : "=r"(ret) : "r"(r), "r"(g), "r"(b), "r"(a)
    );
    #endif

    return ret;
}

void SetSprDefOfsXY(SPR_PRM *spr) {
    spr->ofsx = 2048.0f - (_PkScrW * 0.5f);
    spr->ofsy = 2048.0f - (_PkScrH * 0.5f);
}

void SetSprScreenXYWH(SPR_PRM *spr) {
    spr->sw = _PkScrW;
    spr->sh = _PkScrH;
    spr->px = spr->py = 0;
}

void PkSprPkt_SetDrawEnv(SPR_PKT pkt, SPR_PRM *spr, sceGsDrawEnv1 *pdenv) {
    PK_AD_PACKET *pk;

    if (pdenv == NULL) {
        return;
    }

    pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 3);
    pk->gif.regs = PK_REG_AD(0) | PK_REG_AD(1) | PK_REG_AD(2);

    pk->ad[0].ADDR = SCE_GS_TEXFLUSH;

    pk->ad[1].DATA = GS_REG_WORD(pdenv->frame1);
    pk->ad[1].ADDR = SCE_GS_FRAME_1;

    pk->ad[2].DATA = GS_REG_WORD(pdenv->zbuf1);
    pk->ad[2].ADDR = SCE_GS_ZBUF_1;

    *pkt = (u_long128*)&pk->ad[3];

    _PkDefSCISSOR = pdenv->scissor1;
    _PkDefZBUFFER = pdenv->zbuf1;

    _PkScrW = _PkDefSCISSOR.SCAX1 - _PkDefSCISSOR.SCAX0 + 1;
    _PkScrH = _PkDefSCISSOR.SCAY1 - _PkDefSCISSOR.SCAY0 + 1;

    SetSprDefOfsXY(spr);
    PkDefSCISSOR_Add(pkt);
}

void PkZBUFMask_Add(SPR_PKT pkt, int bMsk) {
    PK_AD_PACKET *pk;

    if (_PkDefZBUFFER.ZBP == 0) {
        return;
    }

    pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    pk->gif.regs = PK_REG_AD(0);
    pk->ad[0].DATA = GS_REG_WORD(_PkDefZBUFFER);
    pk->ad[0].ADDR = SCE_GS_ZBUF_1;

    *pkt = (u_long128*)&pk->ad[1];

    GS_REG_VIEW(sceGsZbuf, pk->ad[0].DATA).ZMSK = (bMsk != 0);
}

void PkSprPkt_SetTexVram(SPR_PKT pkt, SPR_PRM *spr, sceGsDrawEnv1 *pdenv) {
    PK_AD_PACKET *pk;
    int           x, y, w, h;
    int           tbp, tbw, psm;

    if (pdenv == NULL) {
        return;
    }

    /* The frame buffer, as a texture (TBP counts 64-word blocks, FBP 2048-word pages). */
    tbp = pdenv->frame1.FBP << 5;
    tbw = pdenv->frame1.FBW;
    psm = pdenv->frame1.PSM;

    x = pdenv->scissor1.SCAX0;
    y = pdenv->scissor1.SCAY0;
    w = (pdenv->scissor1.SCAX1 - x) + 1;
    h = (pdenv->scissor1.SCAY1 - y) + 1;

    spr->ux = x;
    spr->uy = y;
    spr->uw = w;
    spr->uh = h;

    pk = (PK_AD_PACKET*)*pkt;

    pk->gif.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 2);
    pk->gif.regs = PK_REG_AD(0) | PK_REG_AD(1);

    /* texflush doesn't use the data. */
    pk->ad[0].ADDR = SCE_GS_TEXFLUSH;

    pk->ad[1].DATA = SCE_GS_SET_TEX0(tbp, tbw, psm, 10/*1024*/, 10/*1024*/, 1, 0, 0, 0, 0, 0, 0);
    pk->ad[1].ADDR = SCE_GS_TEX0_1;

    *pkt = (u_long128*)&pk->ad[2];
}

void PkSprPkt_SetDefault(SPR_PKT pk, SPR_PRM *spr, sceGsDrawEnv1 *pdenv) {
    if (pdenv != NULL) {
        _PkDefSCISSOR = pdenv->scissor1;
        _PkDefZBUFFER = pdenv->zbuf1;
    }

    _PkScrW = _PkDefSCISSOR.SCAX1 - _PkDefSCISSOR.SCAX0 + 1;
    _PkScrH = _PkDefSCISSOR.SCAY1 - _PkDefSCISSOR.SCAY0 + 1;

    SetSprDefOfsXY(spr);

    spr->zoom.isOn = FALSE;
    spr->rgba0 = (u_int)SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0x0 /* 0.0f */);
    spr->zdepth = 0;
    spr->zx = spr->zy = 1.0f;

    PkDefReg_Add(pk);
    PkPABE_Add(pk, 0);
    PkFBA_Add(pk, 0);
    PkALPHA_Add(pk, GS_ALPHA_BLEND);
    PkTEST_Add(pk, GS_TEST_OFF);
    PkCLAMP_Add(pk, GS_CLAMP_REPEAT);
    PkCCLAMP_Add(pk, SCE_GS_SET_COLCLAMP(1));
    PkDefSCISSOR_Add(pk);
    /* note: undocumented bit on TEX1. useless? */
    PkTEX1_Add(pk, SCE_GS_SET_TEX1(0, 0, 1, 0, 0, 0, 0) | (1<<13));
}

void PkNSprite_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagTF *sp = (SprTagTF*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 6);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_UV(2) | PK_REG_XYZF2(3) | PK_REG_UV(4) | PK_REG_XYZF2(5);

    sp->prim = GS_PRIM_TEX_SPRITE(TRUE);
    sp->rgba = ppspr->rgba0;

    if (flg & PKSPR_UV_RECT) {
        asm volatile("lq $9, 0x20(%0)" : : "r"(ppspr) : "$9", "memory");
    } else {
        asm volatile("lq $9, 0x00(%0)" : : "r"(ppspr) : "$9", "memory");
    }

    asm volatile(
        "lq     $8,  0x20(%1)  \n\t"
        "li     $10, -6        \n\t"
        "daddu  $9,  $0,  $8   \n\t"
        "pcpyld $8,  $8,  $0   \n\t"
        "paddw  $9,  $8,  $9   \n\t"
        "psllw  $9,  $9,  0x4  \n\t"
        "li     $8,  0x8       \n\t"
        "pextlw $8,  $10, $8   \n\t"
        "pextlw $8,  $8,  $8   \n\t"
        "paddw  $8,  $9,  $8   \n\t"
        "ppach  $9,  $0,  $8   \n\t"
        "sd     $9,  0x20(%0)  \n\t"
        "prot3w $10, $9        \n\t"
        "sd     $10, 0x30(%0)  \n\t"
    : : "r"(sp), "r"(ppspr) : "$8", "$9", "$10", "memory");

    asm volatile(
        "lqc2    $vf01, 0x00(%0)     \n\t"
        "lqc2    $vf02, 0x10(%0)     \n\t"
        "vitof0  $vf01, $vf01        \n\t"
        "vmul.zw $vf01, $vf01, $vf02 \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf03, 0x00(%0)     \n\t"
            "vmr32   $vf04, $vf03        \n\t"
            "vsub.xy $vf01, $vf01, $vf03 \n\t"
            "vmr32   $vf04, $vf04        \n\t"
            "vmul.zw $vf01, $vf01, $vf03 \n\t"
            "vmul.xy $vf01, $vf01, $vf04 \n\t"
            "vadd.xy $vf01, $vf01, $vf03 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd.xy $vf01, $vf01, $vf02 \n\t"
        "lw      $9,    0x00(%1)     \n\t"
        "vmr32   $vf03, $vf01        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.zw $vf01, $vf01, $vf03 \n\t"
        "vftoi4  $vf01, $vf01        \n\t"
        "qmfc2   $8,    $vf01        \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x28(%0)     \n\t"
        "dsrl    $8,    $8,    32    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x38(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkNSprite_Add2(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagTF *sp = (SprTagTF*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 6);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_UV(2) | PK_REG_XYZF2(3) | PK_REG_UV(4) | PK_REG_XYZF2(5);

    sp->prim = GS_PRIM_TEX_SPRITE(TRUE);
    sp->rgba = ppspr->rgba0;

    if (flg & PKSPR_UV_RECT) {
        asm volatile("lq $9, 0x20(%0)" : : "r"(ppspr) : "$9", "memory");
    } else {
        asm volatile("lq $9, 0x00(%0)" : : "r"(ppspr) : "$9", "memory");
    }

    asm volatile(
        "lq     $8,  0x20(%1)  \n\t"
        "li     $10, 0x4       \n\t"
        "daddu  $9,  $0,  $8   \n\t"
        "pcpyld $8,  $8,  $0   \n\t"
        "paddw  $9,  $8,  $9   \n\t"
        "psllw  $9,  $9,  0x4  \n\t"
        "li     $8,  0x8       \n\t"
        "pextlw $8,  $10, $8   \n\t"
        "pextlw $8,  $8,  $8   \n\t"
        "paddw  $8,  $9,  $8   \n\t"
        "ppach  $9,  $0,  $8   \n\t"
        "sd     $9,  0x20(%0)  \n\t"
        "prot3w $10, $9        \n\t"
        "sd     $10, 0x30(%0)  \n\t"
    : : "r"(sp), "r"(ppspr) : "$8", "$9", "$10", "memory");

    asm volatile(
        "lqc2    $vf01, 0x00(%0)     \n\t"
        "lqc2    $vf02, 0x10(%0)     \n\t"
        "vitof0  $vf01, $vf01        \n\t"
        "vmul.zw $vf01, $vf01, $vf02 \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf03, 0x00(%0)     \n\t"
            "vmr32   $vf04, $vf03        \n\t"
            "vsub.xy $vf01, $vf01, $vf03 \n\t"
            "vmr32   $vf04, $vf04        \n\t"
            "vmul.zw $vf01, $vf01, $vf03 \n\t"
            "vmul.xy $vf01, $vf01, $vf04 \n\t"
            "vadd.xy $vf01, $vf01, $vf03 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd.xy $vf01, $vf01, $vf02 \n\t"
        "lw      $9,    0x00(%1)     \n\t"
        "vmr32   $vf03, $vf01        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.zw $vf01, $vf01, $vf03 \n\t"
        "vftoi4  $vf01, $vf01        \n\t"
        "qmfc2   $8,    $vf01        \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x28(%0)     \n\t"
        "dsrl    $8,    $8,    32    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x38(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkNSprite_AddAdj(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagTF *sp = (SprTagTF*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 6);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_UV(2) | PK_REG_XYZF2(3) | PK_REG_UV(4) | PK_REG_XYZF2(5);

    sp->prim = GS_PRIM_TEX_SPRITE(TRUE);
    sp->rgba = ppspr->rgba0;

    if (flg & PKSPR_UV_RECT) {
        asm volatile("lq $9, 0x20(%0)" : : "r"(ppspr) : "$9", "memory");
    } else {
        asm volatile("lq $9, 0x00(%0)" : : "r"(ppspr) : "$9", "memory");
    }

    asm volatile(
        "lq     $8,  0x20(%1)  \n\t"
        "li     $10, 0x7       \n\t"
        "daddu  $9,  $0,  $8   \n\t"
        "pcpyld $8,  $8,  $0   \n\t"
        "paddw  $9,  $8,  $9   \n\t"
        "psllw  $9,  $9,  0x4  \n\t"
        "li     $8,  0x8       \n\t"
        "pextlw $8,  $10, $8   \n\t"
        "pextlw $8,  $8,  $8   \n\t"
        "paddw  $8,  $9,  $8   \n\t"
        "ppach  $9,  $0,  $8   \n\t"
        "sd     $9,  0x20(%0)  \n\t"
        "prot3w $10, $9        \n\t"
        "sd     $10, 0x30(%0)  \n\t"
    : : "r"(sp), "r"(ppspr) : "$8", "$9", "$10", "memory");

    asm volatile(
        "lqc2    $vf01, 0x00(%0)     \n\t"
        "lqc2    $vf02, 0x10(%0)     \n\t"
        "vitof0  $vf01, $vf01        \n\t"
        "vmul.zw $vf01, $vf01, $vf02 \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf03, 0x00(%0)     \n\t"
            "vmr32   $vf04, $vf03        \n\t"
            "vsub.xy $vf01, $vf01, $vf03 \n\t"
            "vmr32   $vf04, $vf04        \n\t"
            "vmul.zw $vf01, $vf01, $vf03 \n\t"
            "vmul.xy $vf01, $vf01, $vf04 \n\t"
            "vadd.xy $vf01, $vf01, $vf03 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd.xy $vf01, $vf01, $vf02 \n\t"
        "lw      $9,    0x00(%1)     \n\t"
        "vmr32   $vf03, $vf01        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.zw $vf01, $vf01, $vf03 \n\t"
        "vftoi4  $vf01, $vf01        \n\t"
        "qmfc2   $8,    $vf01        \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x28(%0)     \n\t"
        "dsrl    $8,    $8,    32    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x38(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkCRect_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagCF *sp = (SprTagCF*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 4);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_XYZF2(2) | PK_REG_XYZF2(3);

    sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0);
    sp->rgba = ppspr->rgba0;

    asm volatile(
        "lqc2    $vf01, 0x00(%0)     \n\t"
        "lqc2    $vf02, 0x10(%0)     \n\t"
        "vitof0  $vf01, $vf01        \n\t"
        "vmul.zw $vf01, $vf01, $vf02 \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf03, 0x00(%0)     \n\t"
            "vmr32   $vf04, $vf03        \n\t"
            "vsub.xy $vf01, $vf01, $vf03 \n\t"
            "vmr32   $vf04, $vf04        \n\t"
            "vmul.zw $vf01, $vf01, $vf03 \n\t"
            "vmul.xy $vf01, $vf01, $vf04 \n\t"
            "vadd.xy $vf01, $vf01, $vf03 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd.xy $vf01, $vf01, $vf02 \n\t"
        "lw      $9,    0x00(%1)     \n\t"
        "vmr32   $vf03, $vf01        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.zw $vf01, $vf01, $vf03 \n\t"
        "vftoi4  $vf01, $vf01        \n\t"
        "qmfc2   $8,    $vf01        \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x20(%0)     \n\t"
        "dsrl    $8,    $8,    32    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x28(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkCGRect_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagCG *sp = (SprTagCG*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 9);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_XYZF2(2) | PK_REG_RGBAQ(3) | PK_REG_XYZF2(4) | PK_REG_RGBAQ(5) | PK_REG_XYZF2(6) | PK_REG_RGBAQ(7) | PK_REG_XYZF2(8);

    sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 1, 0, 0, 1, 0, 1, 0, 0);
    sp->rgba0 = ppspr->rgba0;
    sp->rgba1 = ppspr->rgba1;
    sp->rgba2 = ppspr->rgba2;
    sp->rgba3 = ppspr->rgba3;

    asm volatile(
        "lqc2    $vf01, 0x00(%0)     \n\t"
        "lqc2    $vf02, 0x10(%0)     \n\t"
        "vitof0  $vf01, $vf01        \n\t"
        "vmul.zw $vf01, $vf01, $vf02 \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf03, 0x00(%0)     \n\t"
            "vmr32   $vf04, $vf03        \n\t"
            "vsub.xy $vf01, $vf01, $vf03 \n\t"
            "vmr32   $vf04, $vf04        \n\t"
            "vmul.zw $vf01, $vf01, $vf03 \n\t"
            "vmul.xy $vf01, $vf01, $vf04 \n\t"
            "vadd.xy $vf01, $vf01, $vf03 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd.xy $vf01, $vf01, $vf02 \n\t"
        "lw      $9,    0x00(%1)     \n\t"
        "vmr32   $vf03, $vf01        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.zw $vf01, $vf01, $vf03 \n\t"
        "vftoi4  $vf01, $vf01        \n\t"
        "qmfc2   $8,    $vf01        \n\t"
        "pexew   $11,   $8           \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "ppach   $11,   $0,    $11   \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x20(%0)     \n\t"
        "pextlw  $10,   $9,    $11   \n\t"
        "sd      $10,   0x30(%0)     \n\t"
        "dsrl    $10,   $11,   32    \n\t"
        "pextlw  $10,   $9,    $10   \n\t"
        "sd      $10,   0x40(%0)     \n\t"
        "dsrl    $10,   $8,    32    \n\t"
        "pextlw  $10,   $9,    $10   \n\t"
        "sd      $10,   0x50(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "$11", "memory");

    *pk = (u_long128*)(sp + 1);
}

/* Sine series coefficients for rotcossin, loaded into VF01 as one quadword:
   x^5, x^4, x^3 and x^2 terms are scaled from these by the VU0 code. */
float rotCosSinCoefficients[4] __attribute__((aligned(16))) = {
    2.60188699e-06f, -1.98074136e-04f, 8.33302550e-03f, -1.66666567e-01f
};

static void rotcossin(float rot) {
    asm volatile(
        "mtc1      $0,    $f0                      \n\t"
        "c.olt.s   %0,    $f0                      \n\t"
        "li.s      $f0,   1.57079637050628662109e0 \n\t"
        "bc1f      _RotCosSin_01                   \n\t"
        "add.s     %0,    $f0,   %0                \n\t" /* rx += (pi/2) */
        "li        $7,    1                        \n\t"
        "j         _RotCosSin_02                   \n\t"
"_RotCosSin_01:                                    \n\t"
        "sub.s     %0,    $f0,   %0                \n\t" /* rx = (pi/2)-rx */
        "move      $7,    $0                       \n\t"
"_RotCosSin_02:                                    \n\t"
        "mfc1      $8,    %0                       \n\t"
        "qmtc2     $8,    $vf03                    \n\t"
        "la        $8,    rotCosSinCoefficients    \n\t" /* Transfer coefficients of S5-S2 to VF01 */
        "lqc2      $vf01, 0($8)                    \n\t"
        "vmr32.w   $vf03, $vf03                    \n\t" /* Copy VF03.x(v) to VF03.w */
        "vaddx.x   $vf04, $vf00, $vf03             \n\t" /* Copy VF03.x(v) to VF04.x */
        "vmul.x    $vf03, $vf03, $vf03             \n\t" /* Square VF03.x to v^2 */
        "vmulx.yzw $vf04, $vf04, $vf00             \n\t" /* VF04.yzw = 0 */
        "vmulw     $vf02, $vf01, $vf03             \n\t" /* Apply VF03.w(v) to S2-S5 */
        "vmulx     $vf02, $vf02, $vf03             \n\t" /* Multiply by VF03.x(v^2) */
        "vmulx.xyz $vf02, $vf02, $vf03             \n\t" /* Multiply by VF03.x(v^2) */
        "vaddw.x   $vf04, $vf04, $vf02             \n\t" /* s += k2 */
        "vmulx.xy  $vf02, $vf02, $vf03             \n\t" /* Multiply by VF03.x(v^2) */
        "vaddz.x   $vf04, $vf04, $vf02             \n\t" /* s += z */
        "vmulx.x   $vf02, $vf02, $vf03             \n\t" /* Multiply by VF03.x(v^2) */
        "vaddy.x   $vf04, $vf04, $vf02             \n\t" /* s += y */
        "vaddx.x   $vf04, $vf04, $vf02             \n\t" /* s += x (sin is over) */
        "vaddx.xy  $vf04, $vf19, $vf04             \n\t" /* .xy = s (append) */
        "vmul.x    $vf05, $vf04, $vf04             \n\t" /* VF05.x = s*s */
        "vsubx.w   $vf05, $vf00, $vf05             \n\t" /* VF05.w = 1-(s*s) */
        "vsqrt     Q,     $vf05w                   \n\t" /* Q = sqrt(1-s*s) (cos is over) */
        "vwaitq                                    \n\t"
        "cfc2      $8,    $vi22                    \n\t"
        "qmtc2     $8,    $vf05                    \n\t"
        "bne       $7,    $0,    _rcossin_01       \n\t"
        "vaddx.x   $vf04, $vf19, $vf05             \n\t" /* VF04.x = s */
        "b         _rcossin_02                     \n\t"
"_rcossin_01:                                      \n\t"
        "vsubx.x   $vf04, $vf19, $vf05             \n\t" /* VF04.x = s */
"_rcossin_02:                                      \n\t"
    : : "f"(rot) : "$7", "$8", "$f0", "memory");
}

void _pkVU0RotMatrixZ(float rz) {
    asm volatile(
        "vsub     $vf19, $vf00, $vf00 \n\t"
    );

    rotcossin(rz);

    asm volatile(
        "vmove    $vf09, $vf00        \n\t"
        "vmove.zw $vf06, $vf19        \n\t"
        "vmove.zw $vf07, $vf19        \n\t"
        "vsub.zw  $vf04, $vf04, $vf04 \n\t"
        "vmr32    $vf08, $vf09        \n\t"
        "vaddx.y  $vf06, $vf19, $vf04 \n\t"
        "vaddy.x  $vf06, $vf19, $vf04 \n\t"
        "vsubx.x  $vf07, $vf19, $vf04 \n\t"
        "vaddy.y  $vf07, $vf19, $vf04 \n\t"
    );
}

void PkRSprite_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagTFR *sp = (SprTagTFR*)*pk;

    _pkVU0RotMatrixZ(ppspr->rot);

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 10);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_UV(2) | PK_REG_XYZF2(3) | PK_REG_UV(4) | PK_REG_XYZF2(5) | PK_REG_UV(6) | PK_REG_XYZF2(7) | PK_REG_UV(8) | PK_REG_XYZF2(9);

    sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0, 1, 0, 1, 0, 1, 0, 0);
    sp->rgba = ppspr->rgba0;

    if (flg & PKSPR_UV_RECT) {
        asm volatile("lq $9, 0x20(%0)" : : "r"(ppspr) : "$9", "memory");
    } else {
        asm volatile("lq $9, 0x00(%0)" : : "r"(ppspr) : "$9", "memory");
    }

    asm volatile(
        "lq     $8,  0x20(%1) \n\t"
        "li     $10, -8       \n\t"
        "daddu  $9,  $0,  $8  \n\t"
        "pcpyld $8,  $8,  $0  \n\t"
        "paddw  $9,  $8,  $9  \n\t"
        "psllw  $9,  $9,  0x4 \n\t"
        "li     $8,  8        \n\t"
        "pextlw $8,  $10, $8  \n\t"
        "pextlw $8,  $8,  $8  \n\t"
        "paddw  $8,  $9,  $8  \n\t"
        "ppach  $9,  $0,  $8  \n\t"
        "sd     $9,  0x20(%0) \n\t"
        "pexew  $10, $8       \n\t"
        "ppach  $8,  $0,  $10 \n\t"
        "sd     $8,  0x30(%0) \n\t"
        "prot3w $10, $8       \n\t"
        "sd     $10, 0x40(%0) \n\t"
        "prot3w $10, $9       \n\t"
        "sd     $10, 0x50(%0) \n\t"
    : : "r"(sp), "r"(ppspr) : "$8", "$9", "$10", "memory");

    asm volatile(
        "lqc2    $vf01, 0x00(%0)     \n\t"
        "lq      $8,    0x10(%0)     \n\t"
        "vitof0  $vf01, $vf01        \n\t"
        "qmtc2   $8,    $vf02        \n\t"
        "pcpyud  $8,    $8,    $8    \n\t"
        "qmtc2   $8,    $vf03        \n\t"
        "lq      $8,    0x30(%0)     \n\t"
        "pcpyld  $8,    $8,    $8    \n\t"
        "qmtc2   $8,    $vf04        \n\t"
        "vmul.xy $vf04, $vf04, $vf03 \n\t"
    : : "r"(ppspr) : "$8", "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lq       $8,    0x00(%0)     \n\t"
            "qmtc2    $8,    $vf05        \n\t"
            "pcpyud   $8,    $8,    $8    \n\t"
            "qmtc2    $8,    $vf08        \n\t"
            "vadda.xy ACC,   $vf01, $vf00 \n\t"
            "vmadd.xy $vf01, $vf04, $vf08 \n\t"
            "vmul.zw  $vf01, $vf01, $vf05 \n\t"
            "vmul.zw  $vf04, $vf04, $vf08 \n\t"
            "vsub.xy  $vf01, $vf01, $vf05 \n\t"
            "vadda.xy ACC,   $vf05, $vf00 \n\t"
            "vmadd.xy $vf01, $vf01, $vf08 \n\t"
        : : "r"(&ppspr->zoom) : "$8", "memory");
    } else {
        asm volatile(
            "vadd.xy $vf01,$vf01,$vf04"
        );
    }

    asm volatile(
        "vadd.xy $vf08, $vf01, $vf02 \n\t"
        "vmr32   $vf02, $vf02        \n\t"
        "vmr32   $vf04, $vf04        \n\t"
        "vmr32   $vf05, $vf01        \n\t"
        "vmr32   $vf02, $vf02        \n\t"
        "vmr32   $vf04, $vf04        \n\t"
        "vmr32   $vf05, $vf05        \n\t"
        "lw      $12,   0x00(%1)     \n\t"
        "vmul.xy $vf06, $vf06, $vf02 \n\t"
        "vmul.xy $vf07, $vf07, $vf02 \n\t"
        "vsub.xy $vf09, $vf00, $vf04 \n\t"
        "vsub.y  $vf10, $vf00, $vf04 \n\t"
        "vsub.x  $vf10, $vf05, $vf04 \n\t"
        "vsub.x  $vf11, $vf00, $vf04 \n\t"
        "vsub.y  $vf11, $vf05, $vf04 \n\t"
        "vsub.xy $vf12, $vf05, $vf04 \n\t"
        "vmulax  ACC,   $vf06, $vf09 \n\t"
        "vmadday ACC,   $vf07, $vf09 \n\t"
        "vmaddw  $vf09, $vf08, $vf00 \n\t"
        "vmulax  ACC,   $vf06, $vf10 \n\t"
        "vmadday ACC,   $vf07, $vf10 \n\t"
        "vmaddw  $vf10, $vf08, $vf00 \n\t"
        "vftoi4  $vf09, $vf09        \n\t"
        "vmulax  ACC,   $vf06, $vf11 \n\t"
        "vmadday ACC,   $vf07, $vf11 \n\t"
        "vmaddw  $vf11, $vf08, $vf00 \n\t"
        "vftoi4  $vf10, $vf10        \n\t"
        "qmfc2   $8,    $vf09        \n\t"
        "vmulax  ACC,   $vf06, $vf12 \n\t"
        "vmadday ACC,   $vf07, $vf12 \n\t"
        "vmaddw  $vf12, $vf08, $vf00 \n\t"
        "vftoi4  $vf11, $vf11        \n\t"
        "qmfc2   $9,    $vf10        \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "pextlw  $8,    $12,   $8    \n\t"
        "sd      $8,    0x28(%0)     \n\t"
        "vftoi4  $vf12, $vf12        \n\t"
        "ppach   $8,    $0,    $9    \n\t"
        "pextlw  $8,    $12,   $8    \n\t"
        "sd      $8,    0x38(%0)     \n\t"
        "qmfc2   $10,   $vf11        \n\t"
        "ppach   $8,    $0,    $10   \n\t"
        "pextlw  $8,    $12,   $8    \n\t"
        "sd      $8,    0x48(%0)     \n\t"
        "qmfc2   $11,   $vf12        \n\t"
        "ppach   $8,    $0,    $11   \n\t"
        "pextlw  $8,    $12,   $8    \n\t"
        "sd      $8,    0x58(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "$11", "$12", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkCLine2_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagLF *sp = (SprTagLF*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 4);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_XYZF2(2) | PK_REG_XYZF2(3);

    if (flg & PKSPR_ANTIALIAS) {
        sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 0, 0, 0, 1, 1, 1, 0, 0);
    } else {
        sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 0, 0, 0, 1, 0, 1, 0, 0);
    }

    sp->rgba = ppspr->rgba0;

    asm volatile(
        "lqc2   $vf01, 0x00(%0) \n\t"
        "lqc2   $vf02, 0x10(%0) \n\t"
        "vitof0 $vf01, $vf01    \n\t"
    : : "r"(ppspr) : "memory");

    if (flg & PKSPR_UV_RECT) {
        asm volatile(
            "vmul.zw $vf01, $vf01, $vf02 \n\t"
        );
    } else {
        asm volatile(
            "vsubx.z $vf01, $vf01, $vf01 \n\t"
            "vsuby.w $vf01, $vf01, $vf01 \n\t"
        );
    }

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf03, 0x00(%0)     \n\t"
            "vmr32   $vf04, $vf03        \n\t"
            "vsub.xy $vf01, $vf01, $vf03 \n\t"
            "vmr32   $vf04, $vf04        \n\t"
            "vmul.zw $vf01, $vf01, $vf03 \n\t"
            "vmul.xy $vf01, $vf01, $vf04 \n\t"
            "vadd.xy $vf01, $vf01, $vf03 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd.xy $vf01, $vf01, $vf02 \n\t"
        "lw      $9,    0x00(%1)     \n\t"
        "vmr32   $vf03, $vf01        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.zw $vf01, $vf01, $vf03 \n\t"
        "vftoi4  $vf01, $vf01        \n\t"
        "qmfc2   $8,    $vf01        \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x20(%0)     \n\t"
        "dsrl    $8,    $8,    32    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x28(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkCLineS_AddStart(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagLSF *sp = (SprTagLSF*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 0, 0, 0, SCE_GIF_REGLIST, 2);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1);

    sp->GifCord2.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
    sp->GifCord2.regs = PK_REG_AD(0);

    if (flg & PKSPR_ANTIALIAS) {
        sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_LINESTRIP, 1, 0, 0, 1, 1, 1, 0, 0);
    } else {
        sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_LINESTRIP, 1, 0, 0, 1, 0, 1, 0, 0);
    }

    sp->rgba = ppspr->rgba0;

    asm volatile(
        "lqc2   $vf01, 0x00(%0) \n\t"
        "lqc2   $vf02, 0x10(%0) \n\t"
        "vitof0 $vf01, $vf01    \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf03, 0x00(%0)     \n\t"
            "vmr32   $vf04, $vf03        \n\t"
            "vsub.xy $vf01, $vf01, $vf03 \n\t"
            "vmr32   $vf04, $vf04        \n\t"
            "vmul.xy $vf01, $vf01, $vf04 \n\t"
            "vadd.xy $vf01, $vf01, $vf03 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd.xy $vf01, $vf01, $vf02 \n\t"
        "lw      $9,    0x00(%1)     \n\t"
        "vftoi4  $vf01, $vf01        \n\t"
        "qmfc2   $8,    $vf01        \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "li      $8,    0xd          \n\t"
        "pcpyld  $10,   $8,    $10   \n\t"
        "sq      $10,   0x30(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkCLineS_AddNext(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagLSFN *sp = (SprTagLSFN*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 2);
    sp->GifCord.regs = PK_REG_RGBAQ(0) | PK_REG_XYZF2(1);

    sp->rgba = ppspr->rgba0;

    asm volatile(
        "lqc2   $vf01, 0x00(%0) \n\t"
        "lqc2   $vf02, 0x10(%0) \n\t"
        "vitof0 $vf01, $vf01    \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf03, 0x00(%0)     \n\t"
            "vmr32   $vf04, $vf03        \n\t"
            "vsub.xy $vf01, $vf01, $vf03 \n\t"
            "vmr32   $vf04, $vf04        \n\t"
            "vmul.xy $vf01, $vf01, $vf04 \n\t"
            "vadd.xy $vf01, $vf01, $vf03 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd.xy $vf01, $vf01, $vf02 \n\t"
        "lw      $9,    0x00(%1)     \n\t"
        "vftoi4  $vf01, $vf01        \n\t"
        "qmfc2   $8,    $vf01        \n\t"
        "ppach   $8,    $0,    $8    \n\t"
        "pextlw  $10,   $9,    $8    \n\t"
        "sd      $10,   0x18(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkPolyF3_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    TriTagCFR *sp = (TriTagCFR*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 5);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_XYZF2(2) | PK_REG_XYZF2(3) | PK_REG_XYZF2(4);

    sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_TRI, 0, 0, 0, 1, 0, 1, 0, 0);
    sp->rgba = ppspr->rgba0;

    asm volatile(
        "lqc2    $vf01, 0x50(%0)     \n\t"
        "lqc2    $vf02, 0x60(%0)     \n\t"
        "lqc2    $vf04, 0x10(%0)     \n\t"
        "vmr32   $vf03, $vf04        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.xy $vf03, $vf00, $vf04 \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf04, 0x00(%0)     \n\t"
            "vmr32   $vf05, $vf04        \n\t"
            "lqc2    $vf06, 0x00(%0)     \n\t"
            "vmr32   $vf05, $vf05        \n\t"
            "vadd.xy $vf04, $vf00, $vf05 \n\t"
            "vadd.xy $vf05, $vf00, $vf06 \n\t"
            "vsub    $vf01, $vf01, $vf05 \n\t"
            "vsub    $vf02, $vf02, $vf05 \n\t"
            "vmul    $vf01, $vf01, $vf04 \n\t"
            "vmul    $vf02, $vf02, $vf04 \n\t"
            "vadd    $vf01, $vf01, $vf05 \n\t"
            "vadd    $vf02, $vf02, $vf05 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd   $vf01, $vf01, $vf03 \n\t"
        "lw     $9,    0x00(%1)     \n\t"
        "vftoi4 $vf01, $vf01        \n\t"
        "vadd   $vf02, $vf02, $vf03 \n\t"
        "qmfc2  $8,    $vf01        \n\t"
        "ppach  $8,    $0,    $8    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x20(%0)     \n\t"
        "dsrl   $8,    $8,    32    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x28(%0)     \n\t"
        "vftoi4 $vf02, $vf02        \n\t"
        "qmfc2  $8,    $vf02        \n\t"
        "ppach  $8,    $0,    $8    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x30(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkPolyF4_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagCFR *sp = (SprTagCFR*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_REGLIST, 6);
    sp->GifCord.regs = PK_REG_PRIM(0) | PK_REG_RGBAQ(1) | PK_REG_XYZF2(2) | PK_REG_XYZF2(3) | PK_REG_XYZF2(4) | PK_REG_XYZF2(5);

    sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0, 0, 0, 1, 0, 1, 0, 0);
    sp->rgba = ppspr->rgba0;

    asm volatile(
        "lqc2    $vf01, 0x50(%0)     \n\t"
        "lqc2    $vf02, 0x60(%0)     \n\t"
        "lqc2    $vf04, 0x10(%0)     \n\t"
        "vmr32   $vf03, $vf04        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.xy $vf03, $vf00, $vf04 \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf04, 0x00(%0)     \n\t"
            "vmr32   $vf05, $vf04        \n\t"
            "lqc2    $vf06, 0x00(%0)     \n\t"
            "vmr32   $vf05, $vf05        \n\t"
            "vadd.xy $vf04, $vf00, $vf05 \n\t"
            "vadd.xy $vf05, $vf00, $vf06 \n\t"
            "vsub    $vf01, $vf01, $vf05 \n\t"
            "vsub    $vf02, $vf02, $vf05 \n\t"
            "vmul    $vf01, $vf01, $vf04 \n\t"
            "vmul    $vf02, $vf02, $vf04 \n\t"
            "vadd    $vf01, $vf01, $vf05 \n\t"
            "vadd    $vf02, $vf02, $vf05 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd   $vf01, $vf01, $vf03 \n\t"
        "lw     $9,    0x00(%1)     \n\t"
        "vftoi4 $vf01, $vf01        \n\t"
        "vadd   $vf02, $vf02, $vf03 \n\t"
        "qmfc2  $8,    $vf01        \n\t"
        "ppach  $8,    $0,    $8    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x20(%0)     \n\t"
        "dsrl   $8,    $8,    32    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x28(%0)     \n\t"
        "vftoi4 $vf02, $vf02        \n\t"
        "qmfc2  $8,    $vf02        \n\t"
        "ppach  $8,    $0,    $8    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x30(%0)     \n\t"
        "dsrl   $8,    $8,    32    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x38(%0)     \n\t"
    : : "r"(sp), "r"(&ppspr->zdepth) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

void PkPolyFT4_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg) {
    SprTagTFR *sp = (SprTagTFR*)*pk;

    sp->GifCord.tag = SCE_GIF_SET_TAG(1, SCE_GS_TRUE, 0, 0, SCE_GIF_REGLIST, 10);
    sp->GifCord.regs =
        (u_long)(SCE_GS_PRIM ) << (0 * 4) |
        (u_long)(SCE_GS_RGBAQ) << (1 * 4) |
        (u_long)(SCE_GS_UV   ) << (2 * 4) |
        (u_long)(SCE_GS_XYZF2) << (3 * 4) |
        (u_long)(SCE_GS_UV   ) << (4 * 4) |
        (u_long)(SCE_GS_XYZF2) << (5 * 4) |
        (u_long)(SCE_GS_UV   ) << (6 * 4) |
        (u_long)(SCE_GS_XYZF2) << (7 * 4) |
        (u_long)(SCE_GS_UV   ) << (8 * 4) |
        (u_long)(SCE_GS_XYZF2) << (9 * 4);

    sp->prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0, SCE_GS_TRUE, SCE_GS_FALSE, SCE_GS_TRUE,
                               SCE_GS_FALSE, 1, 0, 0);
    sp->rgba = ppspr->rgba0;

    asm volatile(
        "lq     $9,  0x20(%0) \n\t"
    : : "r"(ppspr) : "$9", "memory");

    #if 0
    sp->uv0 = SCE_GS_SET_UV((ppspr->ux << 4) + 8, (ppspr->uy << 4) + 8);
    sp->uv1 = SCE_GS_SET_UV((ppspr->ux << 4) - 8, (ppspr->uy << 4) + 8);
    sp->uv2 = SCE_GS_SET_UV((ppspr->ux << 4) + 8, (ppspr->uy << 4) - 8);
    sp->uv3 = SCE_GS_SET_UV((ppspr->ux << 4) - 8, (ppspr->uy << 4) - 8);
    #else
    asm volatile(
        "lq     $8,  0x20(%0) \n\t"
        "li     $10, -8       \n\t"
        "daddu  $9,  $0,  $8  \n\t"
        "pcpyld $8,  $8,  $0  \n\t"
        "paddw  $9,  $8,  $9  \n\t"
        "psllw  $9,  $9,  0x4 \n\t"
        "li     $8,  0x8      \n\t"
        "pextlw $8,  $10, $8  \n\t"
        "pextlw $8,  $8,  $8  \n\t"
        "paddw  $8,  $9,  $8  \n\t"
        "ppach  $9,  $0,  $8  \n\t"
        "sd     $9,  0x20(%1) \n\t"
        "pexew  $10, $8       \n\t"
        "ppach  $8,  $0,  $10 \n\t"
        "sd     $8,  0x30(%1) \n\t"
        "prot3w $10, $8       \n\t"
        "sd     $10, 0x40(%1) \n\t"
        "prot3w $10, $9       \n\t"
        "sd     $10, 0x50(%1) \n\t"
    : : "r"(ppspr), "r"(sp) : "$8", "$9", "$10", "memory");
    #endif

    asm volatile(
        "lqc2    $vf01, 0x50(%0)     \n\t"
        "lqc2    $vf02, 0x60(%0)     \n\t"
        "lqc2    $vf04, 0x10(%0)     \n\t"
        "vmr32   $vf03, $vf04        \n\t"
        "vmr32   $vf03, $vf03        \n\t"
        "vadd.xy $vf03, $vf00, $vf04 \n\t"
    : : "r"(ppspr) : "memory");

    if ((flg & PKSPR_ZOOM) && ppspr->zoom.isOn) {
        asm volatile(
            "lqc2    $vf04, 0x0(%0)      \n\t"
            "vmr32   $vf05, $vf04        \n\t"
            "lqc2    $vf06, 0x0(%0)      \n\t"
            "vmr32   $vf05, $vf05        \n\t"
            "vadd.xy $vf04, $vf00, $vf05 \n\t"
            "vadd.xy $vf05, $vf00, $vf06 \n\t"
            "vsub    $vf01, $vf01, $vf05 \n\t"
            "vsub    $vf02, $vf02, $vf05 \n\t"
            "vmul    $vf01, $vf01, $vf04 \n\t"
            "vmul    $vf02, $vf02, $vf04 \n\t"
            "vadd    $vf01, $vf01, $vf05 \n\t"
            "vadd    $vf02, $vf02, $vf05 \n\t"
        : : "r"(&ppspr->zoom) : "memory");
    }

    asm volatile(
        "vadd   $vf01, $vf01, $vf03 \n\t"
        "lw     $9,    0x0(%0)      \n\t"
        "vftoi4 $vf01, $vf01        \n\t"
        "vadd   $vf02, $vf02, $vf03 \n\t"
        "qmfc2  $8,    $vf01        \n\t"
        "ppach  $8,    $0,    $8    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x28(%1)     \n\t"
        "dsrl   $8,    $8,    32    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x38(%1)     \n\t"
        "vftoi4 $vf02, $vf02        \n\t"
        "qmfc2  $8,    $vf02        \n\t"
        "ppach  $8,    $0,    $8    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x48(%1)     \n\t"
        "dsrl   $8,    $8,    32    \n\t"
        "pextlw $10,   $9,    $8    \n\t"
        "sd     $10,   0x58(%1)     \n\t"
    : : "r"(&ppspr->zdepth), "r"(sp) : "$8", "$9", "$10", "memory");

    *pk = (u_long128*)(sp + 1);
}

PKMESH* PkMesh_Create(int w, int h) {
    PKMESH *pmesh = malloc(sizeof(PKMESH));

    if (pmesh == NULL) {
        return NULL;
    }

    memset(pmesh, 0, sizeof(*pmesh));
    pmesh->pmspt = malloc(sizeof(PKMSPT) * (w + 1) * (h + 1));

    if (pmesh->pmspt == NULL) {
        free(NULL);
        return NULL;
    } else {
        pmesh->mw = w;
        pmesh->mh = h;
    }

    return pmesh;
}

void PkMesh_Delete(PKMESH *mesh) {
    if (mesh != NULL) {
        if (mesh->pmspt != NULL) {
            free(mesh->pmspt);
        }

        free(mesh);
    }
}

void PkMesh_SetXYWH(PKMESH *mesh, float px0, float py0, float sw, float sh) {
    PKMSPT *pt;
    int     x, y;
    float   fmw, fmh;

    pt = mesh->pmspt;

    mesh->px = px0;
    mesh->py = py0;

    mesh->sw = sw;
    mesh->sh = sh;

    fmw = 1.0f / mesh->mw;
    fmh = 1.0f / mesh->mh;

    for (y = 0; y < (mesh->mh + 1); y++) {
        float py = py0 + (y * sh * fmh);

        for (x = 0; x < (mesh->mw + 1); x++, pt++) {
            pt->x = px0 + (x * sw * fmw);
            pt->y = py;

            pt->ofsx = pt->ofsy = 0.0f;
        }
    }
}

void PkMesh_SetUVWH(PKMESH *mesh, float ux0, float uy0, float uw, float uh) {
    PKMSPT *pt;
    int     x, y;
    float   fmw, fmh;

    pt = mesh->pmspt;

    fmw = 1.0f / mesh->mw;
    fmh = 1.0f / mesh->mh;

    for (y = 0; y < (mesh->mh + 1); y++) {
        int uy = uy0 + (y * uh * fmh);

        for (x = 0; x < (mesh->mw + 1); x++, pt++) {
            pt->u = ux0 + (x * uw * fmw);
            pt->v = uy;
        }
    }
}

void PkCMesh_Add(SPR_PKT pk, SPR_PRM *spr, PKMESH *mesh) {
    int     cidx, lidx;
    int     x, y;

    cidx = 0;
    lidx = mesh->mw + 1;

    for (y = 0; y < mesh->mh; y++) {
        PKMSPT *pt = &mesh->pmspt[cidx];


        for (x = 0; x < mesh->mw; x++, pt++) {
            spr->px0 = pt[0].x + pt[0].ofsx;
            spr->py0 = pt[0].y + pt[0].ofsy;

            spr->px1 = pt[1].x + pt[1].ofsx;
            spr->py1 = pt[1].y + pt[1].ofsy;

            spr->px2 = pt[lidx].x + pt[lidx].ofsx;
            spr->py2 = pt[lidx].y + pt[lidx].ofsy;

            spr->px3 = pt[lidx + 1].x + pt[lidx + 1].ofsx;
            spr->py3 = pt[lidx + 1].y + pt[lidx + 1].ofsy;

            PkPolyF4_Add(pk, spr, 0);
        }

        cidx += lidx;
    }
}

void PkFTMesh_Add(SPR_PKT pk, SPR_PRM *spr, PKMESH *mesh) {
    int     cidx, lidx;
    int     x, y;

    cidx = 0;
    lidx = mesh->mw + 1;

    for (y = 0; y < mesh->mh; y++) {
        PKMSPT *pt = &mesh->pmspt[cidx];


        for (x = 0; x < mesh->mw; x++, pt++) {
            spr->ux  = pt[0].u;
            spr->uy  = pt[0].v;

            spr->uw  = pt[1].u    - pt[0].u;
            spr->uh  = pt[lidx].v - pt[0].v;

            spr->px0 = pt[0].x + pt[0].ofsx;
            spr->py0 = pt[0].y + pt[0].ofsy;

            spr->px1 = pt[1].x + pt[1].ofsx;
            spr->py1 = pt[1].y + pt[1].ofsy;

            spr->px2 = pt[lidx].x + pt[lidx].ofsx;
            spr->py2 = pt[lidx].y + pt[lidx].ofsy;

            spr->px3 = pt[lidx + 1].x + pt[lidx + 1].ofsx;
            spr->py3 = pt[lidx + 1].y + pt[lidx + 1].ofsy;

            PkPolyFT4_Add(pk, spr, 2);
        }

        cidx += lidx;
    }
}

void PkMesh_SetHLinOfs(PKMESH *mesh, int no, float x, float y) {
    PKMSPT *pt;
    int     i;

    if (mesh->mh < no) {
        return;
    }
    
    pt = &mesh->pmspt[(mesh->mw + 1) * no];
    for (i = 0; i < mesh->mw + 1; i++, pt++) {
        pt->ofsx = x;
        pt->ofsy = y;
    }
}

void PkMesh_SetVLinOfs(PKMESH *mesh, int no, float x, float y) {
    PKMSPT *pt;
    int     i;

    if (mesh->mw < no) {
        return;
    }

    pt = &mesh->pmspt[no];
    for (i = 0; i < mesh->mh + 1; i++, pt += (mesh->mw + 1)) {
        pt->ofsx = x;
        pt->ofsy = y;
    }
}

void PkMesh_SetHLinOfsLRX(PKMESH *mesh, int no, float lofsx, float rofsx) {
    PKMSPT *pt;
    int     i;
    float   x, dx;

    x  = lofsx;
    dx = (rofsx - lofsx) / mesh->mw;

    if (mesh->mh < no) {
        return;
    }

    pt = &mesh->pmspt[(mesh->mw + 1) * no];
    for (i = 0; i < mesh->mw + 1; i++, pt++) {
        pt->ofsx = x;
        x += dx;
    }
}

void PkMesh_SetVLinOfsUDY(PKMESH *mesh, int no, float uofsy, float dofsy) {
    PKMSPT *pt;
    int     i;
    float   y, dy;

    y  = uofsy;
    dy = (uofsy - dofsy) / mesh->mh;

    if (mesh->mw < no) {
        return;
    }

    pt = &mesh->pmspt[no];
    for (i = 0; i < mesh->mh + 1; i++, pt += (mesh->mw + 1)) {
        pt->ofsy = y;
        y += dy;
    }
}
