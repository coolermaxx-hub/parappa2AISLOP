#include "vram.h"

int PrAddDrawAreaDefinition(NaGifPacket *packet, const PrVRAM_RECT& rect, bool offset, bool test, u_long color) {
    int num = 2;
    u_int x0 = rect.x;
    u_int y = rect.y;
    u_int fbw = rect.buffer_width >> 6;
    u_int y0 = y & 0x1f;
    u_int x1 = x0 + rect.w;
    u_int y1 = y0 + rect.h;

    packet->AddGifPackedAD_FRAME_1((y >> 5) * fbw, fbw, 0, 0);
    packet->AddGifPackedAD_SCISSOR_1(x0, x1, y0, y1);

    if (offset) {
        packet->AddGifPackedAD_XYOFFSET_1(0, 0);
        num = 3;
    }

    if (test) {
        packet->AddGifPackedAD_TEST_1(0, 0, 0, 0, 0, 0, 0, 0);
        num++;
    }

    if (color) {
        packet->AddGifPackedAD_PRIM(6 /* SPRITE */, 0, 0, 0, 0, 0, 0, 0, 0);
        num += 4;
        packet->AddGsAD(SCE_GS_RGBAQ, color);
        packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ(x0 << 4, y0 << 4, 0));
        packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ(x1 << 4, y1 << 4, 0));
    }

    return num;
}

int PrAddTextureAreaDefinition(NaGifPacket *packet, const PrVRAM_RECT& rect, bool bilinear) {
    u_int w = rect.w;
    u_int y = rect.y;
    u_int tbw = rect.buffer_width >> 6;
    u_int x = rect.x;
    u_int h = rect.h;
    u_int v = y & 0x1f;

    packet->AddGifPackedAD_TEX0_1(((y >> 5) * tbw) << 5, tbw, 0, 10, 10, 1, 1, 0, 0, 0, 0, 0);
    packet->AddGifPackedAD_CLAMP_1(2, 2, x, x + w - 1, v, v + h - 1);

    if (!bilinear) {
        return 2;
    }

    packet->AddGifPackedAD_TEX1_1(0, 0, 1, 1, 0, 0, 0);
    return 3;
}

static u_int prSpriteDefinitionZ = 0;

void PrSetSpriteDefinitionZ(u_int z) {
    prSpriteDefinitionZ = z;
}

static inline int Max(int a, int b) { return a > b ? a : b; }

int PrAddSpriteDefinition(NaGifPacket *packet, const PrVRAM_RECT& dst, const PrVRAM_RECT& src, bool blend) {
    u_int dx = dst.x;
    u_int dy = dst.y & 0x1f;
    u_int dw = dst.w;
    u_int dh = dst.h;
    u_int sx = src.x;
    u_int sy = src.y & 0x1f;
    u_int sw = src.w;
    u_int sh = src.h;

    packet->AddGifPackedAD_PRIM(6, 0, 1, 0, blend, 0, 1, 0, 0);
    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max(sx * 16 + 8, 0), Max(sy * 16 + 8, 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ(dx << 4, dy << 4, prSpriteDefinitionZ << 4));
    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max((sx + sw) * 16 + 8, 0), Max((sy + sh) * 16 + 8, 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ((dx + dw) << 4, (dy + dh) << 4, prSpriteDefinitionZ << 4));

    return 5;
}

int PrAddSpriteDefinitionSuperSampled(NaGifPacket *packet, const PrVRAM_RECT& dst, const PrVRAM_RECT& src) {
    u_int dx = dst.x;
    u_int dy = dst.y & 0x1f;
    u_int dw = dst.w;
    u_int dh = dst.h;
    u_int sx = src.x;
    u_int sy = src.y & 0x1f;
    u_int sw = src.w;
    u_int sh = src.h;

    u_int du = (sw * 4) / dw;
    u_int dv = (sh * 2) / dh;
    int u0 = (du >> 2) + 8;
    int v0 = dv + 8;

    packet->AddGifPackedAD_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0);
    packet->AddGifPackedAD_ALPHA_1(0, 2, 2, 2, 0x21);

    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max((sx << 4) + (u0 - du), 0), Max((sy << 4) + (v0 - dv), 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ(dx << 4, dy << 4, prSpriteDefinitionZ << 4));
    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max(((sx + sw) << 4) + (u0 - du), 0), Max(((sy + sh) << 4) + (v0 - dv), 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ((dx + dw) << 4, (dy + dh) << 4, prSpriteDefinitionZ << 4));

    packet->AddGifPackedAD_ALPHA_1(0, 2, 2, 1, 0x21);

    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max((sx << 4) + (u0 + du), 0), Max((sy << 4) + (v0 - dv), 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ(dx << 4, dy << 4, prSpriteDefinitionZ << 4));
    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max(((sx + sw) << 4) + (u0 + du), 0), Max(((sy + sh) << 4) + (v0 - dv), 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ((dx + dw) << 4, (dy + dh) << 4, prSpriteDefinitionZ << 4));

    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max((sx << 4) + (u0 - du), 0), Max((sy << 4) + (v0 + dv), 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ(dx << 4, dy << 4, prSpriteDefinitionZ << 4));
    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max(((sx + sw) << 4) + (u0 - du), 0), Max(((sy + sh) << 4) + (v0 + dv), 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ((dx + dw) << 4, (dy + dh) << 4, prSpriteDefinitionZ << 4));

    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max((sx << 4) + (u0 + du), 0), Max((sy << 4) + (v0 + dv), 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ(dx << 4, dy << 4, prSpriteDefinitionZ << 4));
    packet->AddGsAD(SCE_GS_UV, SCE_GS_SET_UV(Max(((sx + sw) << 4) + (u0 + du), 0), Max(((sy + sh) << 4) + (v0 + dv), 0)));
    packet->AddGsAD(SCE_GS_XYZ2, SCE_GS_SET_XYZ((dx + dw) << 4, (dy + dh) << 4, prSpriteDefinitionZ << 4));

    return 19;
}
