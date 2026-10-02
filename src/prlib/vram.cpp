#include "common.h"

#include <eetypes.h>

INCLUDE_ASM("asm/nonmatchings/prlib/vram", PrAddDrawAreaDefinition__FP11NaGifPacketRC11PrVRAM_RECTbbUl);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", PrAddTextureAreaDefinition__FP11NaGifPacketRC11PrVRAM_RECTb);

/* sdata */
extern u_int prSpriteDefinitionZ;

void PrSetSpriteDefinitionZ(u_int z) {
    prSpriteDefinitionZ = z;
}

INCLUDE_ASM("asm/nonmatchings/prlib/vram", PrAddSpriteDefinition__FP11NaGifPacketRC11PrVRAM_RECTT1b);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", PrAddSpriteDefinitionSuperSampled__FP11NaGifPacketRC11PrVRAM_RECTT1);

/* nalib/napacket.h */
INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D668);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D680);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D6C0);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D748);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D7A0);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D7D0);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D838);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D868);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D8A8);

INCLUDE_ASM("asm/nonmatchings/prlib/vram", func_0014D8D8);
