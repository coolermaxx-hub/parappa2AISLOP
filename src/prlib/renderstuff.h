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

struct PrTransmitEntry {
    float depth;
    u_int sortGroup;
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
    void AppendTransmitDmaTag(const sceDmaTag *tag, u_int sortGroup, float depth);

    static int CompareFunction(const void *arg0, const void *arg1);
    void SortTransmitDmaArray();

    void MergeRender();

    void InitializeEECore(PrSceneObject *scene);

    static void RenderVertexEECoreBothface();
    static void RenderVertexEECoreNormal();
    static void RenderVertexEECoreRefmap();
    static void RenderVertexEECoreContour();

    void RenderNodeEECore(PrVuNodeHeaderDmaPacket *arg0);
    void RenderChunkEECore(PrVuDataChunkPacketHeader *arg0, float arg1);

public:
    void AppendDmaTag(const sceDmaTag *tag) {
        // The SDK queue takes an untyped DMA tag address; ownership stays here.
        m_dma_queue.Append(const_cast<sceDmaTag*>(tag));
    }

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
