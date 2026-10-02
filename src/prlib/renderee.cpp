#include "renderstuff.h"

#include "dma.h"
#include "scene.h"

/* data */
extern u_long D_0038C580[];

void PrRenderStuff::InitializeEECore(PrSceneObject *scene) {
    u_long *packet = (u_long*)PR_UNCACHED(D_0038C580);

    sceGsZbuf zbuf = m_zbuf;
    packet[6] = *(u_long*)&zbuf;
    packet[4] = *(u_long*)&scene->unk50;
    packet[24] = *(u_long*)&scene->unk58;
    packet[26] = *(u_long*)&scene->unk70->scissor1;
    packet[28] = *(u_long*)&scene->unk70->dthe;

    PrWaitDmaFinish(SCE_DMA_GIF);
    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0;
    sceDmaSend(chan, D_0038C580);
}

INCLUDE_ASM("asm/nonmatchings/prlib/renderee", RenderVertexEECoreBothface__13PrRenderStuff);

INCLUDE_ASM("asm/nonmatchings/prlib/renderee", RenderVertexEECoreNormal__13PrRenderStuff);

void PrRenderStuff::RenderVertexEECoreRefmap() {
    /* Empty */
}

void PrRenderStuff::RenderVertexEECoreContour() {
    /* Empty */
}

INCLUDE_ASM("asm/nonmatchings/prlib/renderee", RenderNodeEECore__13PrRenderStuffP23PrVuNodeHeaderDmaPacket);

INCLUDE_ASM("asm/nonmatchings/prlib/renderee", RenderChunkEECore__13PrRenderStuffP25PrVuDataChunkPacketHeaderf);

/* prlib/renderstuff.h */
INCLUDE_ASM("asm/nonmatchings/prlib/renderee", func_00146A08);
