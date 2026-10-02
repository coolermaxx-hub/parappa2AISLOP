#ifndef PRLIB_VRAM_H
#define PRLIB_VRAM_H

#include "common.h"

#include <eetypes.h>

#include "nalib/napacket.h"

struct PrVRAM_RECT {
    u_int x;
    u_int y;
    u_int w;
    u_int h;
    u_int buffer_width;
};

int PrAddDrawAreaDefinition(NaGifPacket *packet, const PrVRAM_RECT& rect, bool offset, bool test, u_long color);
int PrAddTextureAreaDefinition(NaGifPacket *packet, const PrVRAM_RECT& rect, bool bilinear);
void PrSetSpriteDefinitionZ(u_int z);
int PrAddSpriteDefinition(NaGifPacket *packet, const PrVRAM_RECT& dst, const PrVRAM_RECT& src, bool blend);
int PrAddSpriteDefinitionSuperSampled(NaGifPacket *packet, const PrVRAM_RECT& dst, const PrVRAM_RECT& src);

#endif /* PRLIB_VRAM_H */
