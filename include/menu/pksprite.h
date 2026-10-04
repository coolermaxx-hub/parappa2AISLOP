#ifndef PKSPRITE_H
#define PKSPRITE_H

#include "common.h"

#include <eetypes.h>
#include <eestruct.h>
#include <libdma.h>
#include <libgraph.h>
#include <libvu0.h>

#define abgr_get_r(abgr) (((u_char*)&abgr)[0])
#define abgr_get_g(abgr) (((u_char*)&abgr)[1])
#define abgr_get_b(abgr) (((u_char*)&abgr)[2])
#define abgr_get_a(abgr) (((u_char*)&abgr)[3])

typedef struct { // 0x40
    /* 0x00 */ sceDmaTag tag[3];
    /* 0x30 */ u_int PaketTop;
} TSPAKET;

typedef struct { // 0xc
    /* 0x0 */ u_int isAlloc;
    /* 0x4 */ u_int *top;
    /* 0x8 */ u_int size;
} TS_WORKMEM;

typedef struct { // 0xa0
    /* 0x00 */ TSPAKET pkt[2];
    /* 0x80 */ u_int idx;
    /* 0x84 */ u_int size;
    /* 0x88 */ u_int ptop;
    /* 0x8c */ u_int btop;
    /* 0x90 */ u_int next;
    /* 0x94 */ TS_WORKMEM mem;
} TsUSERPKT;

typedef u_long128 **SPR_PKT;

/* A GIF tag as the two 64-bit words the sprite packets write. */
typedef struct {
    u_long tag;  /* SCE_GIF_SET_TAG(...) */
    u_long regs; /* REGS: four bits per register, first register in the low nibble */
} PR_ALIGNED(16) PKGIFTAG;

typedef struct { // 0x18
    /* 0x00 */ float centerX;
    /* 0x04 */ float centerY;
    /* 0x08 */ float zoomX;
    /* 0x0c */ float zoomY;
    /* 0x10 */ int isOn;
    /* 0x14 */ int pad;
} SPR_ZOOM;

typedef struct { // 0x90
    /* 0x00 */ int px;
    /* 0x04 */ int py;
    /* 0x08 */ int sw;
    /* 0x0c */ int sh;
    /* 0x10 */ float ofsx;
    /* 0x14 */ float ofsy;
    /* 0x18 */ float zx;
    /* 0x1c */ float zy;
    /* 0x20 */ int ux;
    /* 0x24 */ int uy;
    /* 0x28 */ int uw;
    /* 0x2c */ int uh;
    /* 0x30 */ float cx;
    /* 0x34 */ float cy;
    /* 0x38 */ float rot;
    /* 0x3c */ u_int zdepth;
    /* 0x40 */ u_int rgba0;
    /* 0x44 */ u_int rgba1;
    /* 0x48 */ u_int rgba2;
    /* 0x4c */ u_int rgba3;
    /* 0x50 */ float px0;
    /* 0x54 */ float py0;
    /* 0x58 */ float px1;
    /* 0x5c */ float py1;
    /* 0x60 */ float px2;
    /* 0x64 */ float py2;
    /* 0x68 */ float px3;
    /* 0x6c */ float py3;
    /* 0x70 */ SPR_ZOOM zoom;
} PR_ALIGNED(16) SPR_PRM; /* 16-byte aligned in the original debug info (sizeof 0x90) */

/* Flags for the Pk*Sprite/Line _Add functions. */
#define PKSPR_UV_RECT   0x1 /* texture rectangle comes from ux/uy/uw/uh instead of px/py/sw/sh */
#define PKSPR_ZOOM      0x2 /* apply SPR_PRM::zoom when it is switched on */
#define PKSPR_ANTIALIAS 0x4 /* antialiased line */

typedef struct { // 0x18
    /* 0x00 */ float u;
    /* 0x04 */ float v;
    /* 0x08 */ float x;
    /* 0x0c */ float y;
    /* 0x10 */ float ofsx;
    /* 0x14 */ float ofsy;
} PKMSPT;

typedef struct { // 0x1c
    /* 0x00 */ int mw;
    /* 0x04 */ int mh;
    /* 0x08 */ int px;
    /* 0x0c */ int py;
    /* 0x10 */ int sw;
    /* 0x14 */ int sh;
    /* 0x18 */ PKMSPT *pmspt;
} PKMESH;

typedef struct { // 0x40
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long prim;
    /* 0x18 */ u_long rgba;
    /* 0x20 */ u_long uv0;
    /* 0x28 */ u_long xyz2_0;
    /* 0x30 */ u_long uv1;
    /* 0x38 */ u_long xyz2_1;
} SprTagTF;

typedef struct { // 0x30
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long prim;
    /* 0x18 */ u_long rgba;
    /* 0x20 */ u_long xyz2_0;
    /* 0x28 */ u_long xyz2_1;
} SprTagCF;

typedef struct { // 0x60
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long prim;
    /* 0x18 */ u_long rgba;
    /* 0x20 */ u_long uv0;
    /* 0x28 */ u_long xyz2_0;
    /* 0x30 */ u_long uv1;
    /* 0x38 */ u_long xyz2_1;
    /* 0x40 */ u_long uv2;
    /* 0x48 */ u_long xyz2_2;
    /* 0x50 */ u_long uv3;
    /* 0x58 */ u_long xyz2_3;
} SprTagTFR;

typedef struct { // 0x40
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long prim;
    /* 0x18 */ u_long rgba;
    /* 0x20 */ u_long xyz2_0;
    /* 0x28 */ u_long xyz2_1;
    /* 0x30 */ u_long xyz2_2;
    /* 0x38 */ u_long xyz2_3;
} SprTagCFR;

typedef struct { // 0x40
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long prim;
    /* 0x18 */ u_long rgba;
    /* 0x20 */ u_long xyz2_0;
    /* 0x28 */ u_long xyz2_1;
    /* 0x30 */ u_long xyz2_2;
    /* 0x38 */ u_long pad;
} TriTagCFR;

typedef struct { // 0x30
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long prim;
    /* 0x18 */ u_long rgba;
    /* 0x20 */ u_long xyz2_0;
    /* 0x28 */ u_long xyz2_1;
} SprTagLF;

typedef struct { // 0x40
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long prim;
    /* 0x18 */ u_long rgba;
    /* 0x20 */ PKGIFTAG GifCord2;
    /* 0x30 */ u_long xyz3;
    /* 0x38 */ u_long reg;
} SprTagLSF;

typedef struct { // 0x20
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long rgba;
    /* 0x18 */ u_long xyz2_0;
} SprTagLSFN;

typedef struct { // 0x60
    /* 0x00 */ PKGIFTAG GifCord;
    /* 0x10 */ u_long prim;
    /* 0x18 */ u_long rgba0;
    /* 0x20 */ u_long xyz2_0;
    /* 0x28 */ u_long rgba1;
    /* 0x30 */ u_long xyz2_1;
    /* 0x38 */ u_long rgba2;
    /* 0x40 */ u_long xyz2_2;
    /* 0x48 */ u_long rgba3;
    /* 0x50 */ u_long xyz2_3;
    /* 0x58 */ u_long pad;
} SprTagCG;

u_long128* TsInitUPacket(TsUSERPKT *pk, u_long128 *buf, u_int size);
void TsEndUPacket(TsUSERPKT *pk);
void TsDrawUPacket(TsUSERPKT *up);

void PkTEX0_Add(SPR_PKT pkt, u_long texreg);
void PkTEX1_Add(SPR_PKT pkt, u_long texreg);
void PkCLAMP_Add(SPR_PKT pkt, u_long texrp);
void PkALPHA_Add(SPR_PKT pkt, u_long alpreg);
void PkTEST_Add(SPR_PKT pkt, u_long testsw);
void PkSCISSOR_Add(SPR_PKT pkt, short x, short y, short w, short h);
void PkDefSCISSOR_Add(SPR_PKT pkt);
void PkOFFSET_Add(SPR_PKT pkt, int x, int y);
void PkPABE_Add(SPR_PKT pkt, u_int flg);
void PkFBA_Add(SPR_PKT pkt, u_int flg);
void PkCCLAMP_Add(SPR_PKT pkt, u_int flg);

void PkTEX0_SetAdd(SPR_PKT pkt, int vram, int w, int h, int isLinear);

u_int GetDToneColor(u_int sbgr, u_int dbgr, int ton);
u_int GetToneColorA(u_int abgr, int tona, int tonb, int tong, int tonr);
u_int GetToneColorH(u_int abgr, int tona, int tonb, int tong, int tonr);

void SetSprDefOfsXY(SPR_PRM *spr);
void SetSprScreenXYWH(SPR_PRM *spr);

void PkSprPkt_SetDrawEnv(SPR_PKT pkt, SPR_PRM *spr, sceGsDrawEnv1 *pdenv);

void PkZBUFMask_Add(SPR_PKT pkt, int bMsk);

void PkSprPkt_SetTexVram(SPR_PKT pkt, SPR_PRM *spr, sceGsDrawEnv1 *pdenv);
void PkSprPkt_SetDefault(SPR_PKT pk, SPR_PRM *spr, sceGsDrawEnv1 *pdenv);

void PkNSprite_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg);
void PkNSprite_Add2(SPR_PKT pk, SPR_PRM *ppspr, int flg);
void PkNSprite_AddAdj(SPR_PKT pk, SPR_PRM *ppspr, int flg);

void PkCRect_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg);
void PkCGRect_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg);

void _pkVU0RotMatrixZ(float rz);

void PkRSprite_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg);

void PkCLine2_Add( SPR_PKT pk, SPR_PRM *ppspr, int flg);
void PkCLineS_AddStart(SPR_PKT pk, SPR_PRM *ppspr, int flg);
void PkCLineS_AddNext(SPR_PKT pk, SPR_PRM *ppspr, int flg);

void PkPolyF3_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg);
void PkPolyF4_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg);
void PkPolyFT4_Add(SPR_PKT pk, SPR_PRM *ppspr, int flg);

PKMESH* PkMesh_Create(int w, int h);
void PkMesh_Delete(PKMESH *mesh);

void PkMesh_SetXYWH(PKMESH *mesh, float px0, float py0, float sw, float sh);
void PkMesh_SetUVWH(PKMESH *mesh, float ux0, float uy0, float uw, float uh);

void PkCMesh_Add(SPR_PKT pk, SPR_PRM *spr, PKMESH *mesh);
void PkFTMesh_Add(SPR_PKT pk, SPR_PRM *spr, PKMESH *mesh);

void PkMesh_SetHLinOfs(PKMESH *mesh, int no, float x, float y);
void PkMesh_SetVLinOfs(PKMESH *mesh, int no, float x, float y);
void PkMesh_SetHLinOfsLRX(PKMESH *mesh, int no, float lofsx, float rofsx);
void PkMesh_SetVLinOfsUDY(PKMESH *mesh, int no, float uofsy, float dofsy);

#endif /* PKSPRITE_H */
