#include "renderstuff.h"

#if defined(PRD_SYORI)
#include "dbug/syori.h"
#endif

#include "gifreg.h"
#include "mfifo.h"
#include "microprogram.h"
#include "scene.h"

#include <eeregs.h>
#include <libgraph.h>

#include <stdlib.h>

PrRenderStuff prRenderStuff;

PrRenderStuff::PrRenderStuff() : m_dma_queue(1200) {
    m_transmit_array_size = 0;
    m_transmit_array_max = 0;
    m_transmit_array = NULL;
    AllocateTransmitDmaArray(600);

    m_scene = NULL;
}

PrRenderStuff::~PrRenderStuff() {
    Cleanup();
}

void PrRenderStuff::Initialize(sceGsZbuf zbuf) {
    m_zbuf = zbuf;
    m_scene = NULL;
    this->unk28 = 0;

    PrLoadMicroPrograms();

    PrInitializeDmaStripGifRegister(zbuf);
    PrInitializeMfifo();

    /*
     * Disable VIF DMAtag mismatch errors to
     * avoid HW bug where these are triggered
     * with valid packets, causing stalls.
     *
     * Though these are already guaranteed
     * to be set from the sceDevVif0Reset()
     * and sceGsResetPath() calls on
     * InitSystem() (os/system.c).
     */
    *VIF0_ERR |= (1<<1);
    *VIF1_ERR |= (1<<1);
}

void PrRenderStuff::Cleanup() {
    PrCleanupMfifo();

    delete m_transmit_array;
    m_transmit_array_size = 0;
    m_transmit_array_max = 0;
    m_transmit_array = NULL;

    m_scene = NULL;
}

u_int PrRenderStuff::GetZbufBits(void) const {
    switch (m_zbuf.PSM) {
    case 0:  /* PSMZ32 */
        return 32;
    case 1:  /* PSMZ24 */
        return 24;
    case 2:  /* PSMZ16 */
        return 16;
    case 10: /* PSMZ16S */
        return 16;
    default:
        break;
    }

    return 0;
}

void PrRenderStuff::ResetStatistics() {
    m_statistics.node_num = 0;

    m_statistics.opaque_context1_node_num = 0;
    m_statistics.transmit_context1_node_num = 0;
    m_statistics.opaque_context2_node_num = 0;
    m_statistics.transmit_context2_node_num = 0;
    
    m_statistics.render_time0 = 0;
    m_statistics.render_time1 = 0;
    m_statistics.render_time2 = 0;
    m_statistics.render_time3 = 0;
    m_statistics.render_time4 = 0;
    m_statistics.render_time5 = 0;
    m_statistics.render_time6 = 0;
    m_statistics.render_time7 = 0;
    m_statistics.render_time8 = 0;

    m_statistics.dynamic_append_transmit_node = false;
}

void PrRenderStuff::StartRender(PrSceneObject *scene) {
    m_dma_queue.Start();
    m_scene = scene;
}

void PrRenderStuff::WaitRender() {
    bool noSync = false;

    m_dma_queue.Wait();
    PrStopMfifo();

    m_statistics.render_time6 = *T3_COUNT;

    if (m_scene->GetFocalLength() != 0.0f) {
        sceGsSyncPath(0, 0);
        m_scene->ApplyDepthOfField();
        sceGsSyncPath(0, 0);
        noSync = true;
    }

    if (m_scene->m_screen_model_list != NULL) {
        m_scene->PrepareScreenModelRender();
        m_dma_queue.Wait();
        noSync = false;
    }

    if (!noSync) {
        sceGsSyncPath(0, 0);
    }

    m_statistics.render_time7 = *T3_COUNT;

    m_scene = NULL;
}

void PrRenderStuff::AllocateTransmitDmaArray(u_int size) {
    if (m_transmit_array_max >= size) {
        return;
    }

    int elems = 600;
    while (elems < size) {
        elems *= 2;
    }

    delete m_transmit_array;
    m_transmit_array = new PrTransmitEntry[elems];
    m_transmit_array_max = elems;
}

void PrRenderStuff::AppendTransmitDmaTag(const sceDmaTag *tag, u_int sortGroup, float depth) {
    static bool warned = false;

    if (m_transmit_array_size >= m_transmit_array_max) {
        if (!warned) {
            warned = true;
            return;
        }
    } else {
        m_transmit_array[m_transmit_array_size].depth = depth;
        m_transmit_array[m_transmit_array_size].sortGroup = sortGroup;
        m_transmit_array[m_transmit_array_size].tag = tag;
        m_transmit_array_size++;
    }
}

int PrRenderStuff::CompareFunction(const void *lhs, const void *rhs) {
    PrTransmitEntry *entryA = (PrTransmitEntry*)lhs;
    PrTransmitEntry *entryB = (PrTransmitEntry*)rhs;

    if (entryA->sortGroup != entryB->sortGroup) {
        return (entryA->sortGroup >= entryB->sortGroup) ? 1 : -1;
    }

    if (entryA->depth == entryB->depth) {
        return 0;
    } else if (entryB->depth < entryA->depth) {
        return 1;
    }

    return -1;
}

void PrRenderStuff::SortTransmitDmaArray() {
    if (m_transmit_array_size > 1) {
        qsort(m_transmit_array, m_transmit_array_size, sizeof(PrTransmitEntry), CompareFunction);
    }
}

void PrRenderStuff::MergeRender() {
    bool first = false;

    for (int i = 0; i < m_transmit_array_size; i++) {
        if (!first && m_transmit_array[i].sortGroup == -1) {
            PrDmaStripForSetGifRegister *strip = PrGetDmaStripGifRegister(eGifRegisterMode_NoZWrite);
            AppendDmaTag(&strip->m_tag);
            first = true;
        }

        AppendDmaTag(m_transmit_array[i].tag);
    }

    if (prCurrentStage == 19) {
        PrDmaStripForSetGifRegister *strip = PrGetDmaStripGifRegister(eGifRegisterMode_NoZWrite);
        AppendDmaTag(&strip->m_tag);
    }
}
