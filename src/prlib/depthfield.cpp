#include "common.h"

#include "renderstuff.h"
#include "scene.h"
#include "spram.h"
#include "vram.h"

#include <eeregs.h>
#include <libdma.h>
#include <libgraph.h>

/* sdata */
extern PrSPRAM_DATA *prSpramData_tmp_depthfield;

/* bss */
extern sceGifTag gifTag_tmp_depthfield;

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", ApplyDepthOfField__13PrSceneObject);
#else
/* Register allocation: zbuf and the packet pointer swap s0/s1 */
void PrSceneObject::ApplyDepthOfField() {
    u_int level = GetDepthLevel();
    if (level == 0) {
        return;
    }

    float focal = GetFocalLength();
    if (focal <= 0.0f) {
        return;
    }

    float defocus = GetDefocusLength();
    if (defocus <= focal) {
        return;
    }

    if (this->unk94 == 0xffffffff) {
        return;
    }

    u_int width = this->unk74;
    u_int height = this->unk78;

    PrVRAM_RECT src;
    src.x = 0;
    src.buffer_width = width;
    src.y = ((u_long)this->unk50.FBP << 11) / width;
    src.w = width;
    src.h = height;

    PrVRAM_RECT work;
    work.buffer_width = width;
    work.x = 0;
    work.y = (this->unk94 << 11) / width;
    work.w = width >> 1;
    work.h = height;

    sceGsZbuf zbuf = prRenderStuff.m_zbuf;
    NaGifPacket packet;
    packet.Init((u_long128*)((char*)prSpramData_tmp_depthfield + 0x690));
    packet.End(0, 0, 0);

    sceGifTag& tag = gifTag_tmp_depthfield;
    tag.NLOOP = 0;
    tag.EOP = 1;
    tag.PRE = 0;
    tag.FLG = 0;
    tag.NREG = 1;
    tag.REGS0 = 0xe;
    packet.BeginGifTag(*(u_long128*)&tag);

    zbuf.ZMSK = 1;
    packet.AddGsAD(SCE_GS_ZBUF_1, *(u_long*)&zbuf);

    bool first = true;
    for (u_int i = 1; i <= level; i++) {
        packet.AddTexflush();
        PrAddDrawAreaDefinition(&packet, work, first, true, 0);
        PrAddTextureAreaDefinition(&packet, src, first);
        PrAddSpriteDefinitionSuperSampled(&packet, work, src);

        float depth = focal + (defocus - focal) * i * 0.25f;
        const NaMATRIX<float, 4, 4>& m = prSpramData_tmp_depthfield->unk1A0;
        PrSetSpriteDefinitionZ((m[2][2] * depth + m[3][2]) / (m[2][3] * depth + m[3][3]));

        packet.AddTexflush();
        packet.AddTest1(0, 0, 0, 0, 0, 0, 1, 2);
        PrAddDrawAreaDefinition(&packet, src, false, false, 0);
        PrAddTextureAreaDefinition(&packet, work, false);
        packet.AddGsAD(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0, 1, 2, 1, 0x60));
        PrAddSpriteDefinition(&packet, src, work, true);

        if (first) {
            work.w = width >> 2;
            work.h = height >> 1;
        }
        first = false;
    }

    zbuf.ZMSK = 0;
    packet.AddGsAD(SCE_GS_ZBUF_1, *(u_long*)&zbuf);

    sceGsDrawEnv1 *env = this->unk70;
    packet.AddGsAD(SCE_GS_SCISSOR_1, *(u_long*)&env->scissor1);
    packet.AddGsAD(SCE_GS_XYOFFSET_1, *(u_long*)&this->unk58);
    packet.AddGsAD(SCE_GS_TEST_1, *(u_long*)&env->test1);
    packet.AddGsAD(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x80));
    packet.CloseGifTag();
    sceGifPkTerminate(&packet.m_packet);

    sceDmaChan *dma = sceDmaGetChan(SCE_DMA_GIF);
    dma->chcr.TTE = 0;
    FlushCache(0);
    sceDmaSend(dma, (u_long128*)(((u_int)packet.m_packet.pBase & 0x3fff) | 0x80000000));
}
#endif


/* nalib/napacket.h */
INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", func_0014CD38);

INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", AddTest1__11NaGifPacketUiiUciUiiUii);

INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", func_0014CDA0);

INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", AddGsAD__11NaGifPacketUiUl);

INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", func_0014CDB0);

INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", func_0014CDB8);

INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", func_0014CDC0);

INCLUDE_ASM("asm/nonmatchings/prlib/depthfield", func_0014CDC8);
