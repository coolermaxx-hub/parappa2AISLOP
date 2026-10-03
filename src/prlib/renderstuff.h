#ifndef PRLIB_RENDERSTUFF_H
#define PRLIB_RENDERSTUFF_H

#include "common.h"

#include "prpriv.h"
#include "dmaqueue.h"

#include <eetypes.h>
#include <eestruct.h>
#include <libdma.h>

class PrSceneObject;

struct PrVuNodeHeaderDmaPacket;
struct PrVuDataChunkPacketHeader;

/* A DMA chain queued for this frame; sorted by group, then by depth */
struct PrTransmitEntry {
    float depth;
    u_int group;
    const sceDmaTag *tag;
};

class PrRenderStuff {
public:
    PrRenderStuff();
    ~PrRenderStuff();

    void Initialize(sceGsZbuf zbuf);
    void Cleanup();

    u_int GetZbufBits(void) const;
    void ResetStatistics();

    void StartRender(PrSceneObject *scene);
    void WaitRender();

    void AllocateTransmitDmaArray(u_int size);
    void AppendTransmitDmaTag(const sceDmaTag *tag, u_int group, float depth);

    static int CompareFunction(const void *arg0, const void *arg1);
    void SortTransmitDmaArray();

    void MergeRender();

    void InitializeEECore(PrSceneObject *scene);

    void RenderVertexEECoreBothface();
    void RenderVertexEECoreNormal();
    void RenderVertexEECoreRefmap();
    void RenderVertexEECoreContour();

    void RenderNodeEECore(PrVuNodeHeaderDmaPacket *arg0);
    void RenderChunkEECore(PrVuDataChunkPacketHeader *arg0, float arg1);

public:
    void AppendDmaTag(const sceDmaTag *tag);

public:
    PrDmaQueue m_dma_queue;
    u_int m_transmit_array_size;
    int m_transmit_array_max;
    PrTransmitEntry *m_transmit_array;
    PrSceneObject *m_scene;
    sceGsZbuf m_zbuf;
    int unk28;
    PrRENDERING_STATISTICS m_statistics;
};

extern PrRenderStuff prRenderStuff;

#endif /* PRLIB_RENDERSTUFF_H */
