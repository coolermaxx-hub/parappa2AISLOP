#include "render.h"

#include "dma.h"
#include "model.h"
#include "renderstuff.h"
#include "scene.h"
#include "spram.h"

#include <nalib/namatrix.h>

/* float D_FLT_003998E4; */

extern PrSPRAM_DATA *prSpramData;

extern bool AwfulStatus;

extern PrVu1InitPacket initVu1DmaPacket;

INCLUDE_ASM("asm/nonmatchings/prlib/render", Render__13PrSceneObject);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", InitializeVu1__13PrSceneObject);
#else /* Uses tail call? */
void PrSceneObject::InitializeVu1() {
    PrVu1InitPacket *packet = (PrVu1InitPacket*)PR_UNCACHED(&initVu1DmaPacket);
    packet->zbuf     = prRenderStuff.m_zbuf;
    packet->frame    = this->unk50;
    packet->xyoffset = this->unk58;
    packet->scissor  = this->unk70->scissor1;

    PrWaitDmaFinish(SCE_DMA_GIF);

    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0; /* Don't transfer the DMAtag */

    sceDmaSend(chan, &initVu1DmaPacket);
}
#endif

INCLUDE_ASM("asm/nonmatchings/prlib/render", PrepareScreenModelRender__13PrSceneObject);

INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateCurrentMatrix__13PrModelObject);

INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateCurrentMatrix__13SpmFileHeaderP13PrModelObjectRCt8NaMATRIX3Zfi4i4);

INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateCurrentMatrixAnimation__13SpmFileHeaderP13PrModelObjectRCt8NaMATRIX3Zfi4i4);

INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateClusterMatrix__13SpmFileHeaderP13PrModelObjectRCt8NaMATRIX3Zfi4i4);

INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateClusterMatrixAnimation__13SpmFileHeaderP13PrModelObjectRCt8NaMATRIX3Zfi4i4);

void PrModelObject::RenderContext1Model() {
    m_spm_image->RenderContext1Model(this);
    m_rendered_once = 1;
}

void SpmFileHeader::RenderContext1Model(PrModelObject *model) {
    if (m_flags & 0x10) {
        return;
    }

    for (u_int i = 0; i < m_node_num; i++) {
        m_nodes[i]->RenderContext1Node(model);
    }
}

INCLUDE_ASM("asm/nonmatchings/prlib/render", ModifySimpleDmaPacket__7SpmNodeP23PrVuNodeHeaderDmaPacket);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", RenderContext1Node__7SpmNodeP13PrModelObject);
#else /* Need to match .sdata */
void SpmNode::RenderContext1Node(PrModelObject *model) {
    prRenderStuff.m_statistics.node_num++;

    if (this->unk154 & 0x2000) {
        return;
    }

    if ((this->unk154 & 0x4000) && (!AwfulStatus || (this->unk154 & 0x400000))) {
        PrVuNodeHeaderDmaPacket *packet = this->unk16C[0];
        if (packet != NULL) {
            prRenderStuff.m_statistics.opaque_context1_node_num++;
            ModifySimpleDmaPacket(packet);
            prRenderStuff.AppendDmaTag(&packet->m_tag);
        }

        packet = this->unk16C[1];
        if (packet != NULL) {
            prRenderStuff.m_statistics.transmit_context1_node_num++;
            ModifySimpleDmaPacket(packet);

            float sp0[4];

            asm volatile(
                "lqc2     vf13,   0x0(%0)       \n\t"
                "lqc2     vf14,  0x10(%0)       \n\t"
                "lqc2     vf15,  0x20(%0)       \n\t"
                "lqc2     vf16,  0x30(%0)       \n\t"
            : : "r"(&this->unk40));

            asm volatile(
                "lqc2     vf04,  0x0(%0)        \n\t"
                "vmulax   ACC,   vf13,    vf04  \n\t"
                "vmadday  ACC,   vf14,    vf04  \n\t"
                "vmaddaz  ACC,   vf15,    vf04  \n\t"
                "vmaddw   vf17,  vf16,    vf04  \n\t"
            : : "r"(&this->unk140));

            asm volatile(
                "lqc2     vf13,     0(%1)       \n\t"
                "lqc2     vf14,  0x10(%1)       \n\t"
                "lqc2     vf15,  0x20(%1)       \n\t"
                "lqc2     vf16,  0x30(%1)       \n\t"
                "vmulax   ACC,   vf13,    vf17  \n\t"
                "vmadday  ACC,   vf14,    vf17  \n\t"
                "vmaddaz  ACC,   vf15,    vf17  \n\t"
                "vmaddw   vf17,  vf16,    vf17  \n\t"
                "sqc2     vf17,   0x0(%0)       \n\t"
            : : "r"(sp0), "r"(&prSpramData->unkE0));

            float f12 = sp0[2] / sp0[3];
            if (sp0[3] == 0.0f) {
                f12 = sp0[2] * D_FLT_003998E4;
            }

            prRenderStuff.AppendTransmitDmaTag(&packet->m_tag, this->unk188, f12);
        }

        if (this->unk154 & 0x40) {
            SpmComplexNode *complex = reinterpret_cast<SpmComplexNode*>(this);
            complex->RenderContour(model);
        }
    }
}
#endif

void PrModelObject::RenderScreenModelNode() {
    m_spm_image->RenderScreenModelNode();
    m_rendered_once = 1;
}

void SpmFileHeader::RenderScreenModelNode() {
    for (u_int i = 0; i < m_node_num; i++) {
        m_nodes[i]->RenderScreenModelNode();
    }
}

INCLUDE_ASM("asm/nonmatchings/prlib/render", RenderScreenModelNode__7SpmNode);

void PrModelObject::RenderBackgroundScreenModel() {
    m_spm_image->RenderBackgroundScreenModel();
    m_rendered_once = 1;
}

void SpmFileHeader::RenderBackgroundScreenModel() {
    for (u_int i = 0; i < m_node_num; i++) {
        m_nodes[i]->RenderBackgroundScreenModel();
    }
}

INCLUDE_ASM("asm/nonmatchings/prlib/render", RenderBackgroundScreenModel__7SpmNode);

INCLUDE_ASM("asm/nonmatchings/prlib/render", RenderContext2Model__13PrModelObject);

void SpmFileHeader::RenderContext2Model(PrModelObject *model) {
    if (m_flags & 0x8) {
        return;
    }

    for (u_int i = 0; i < m_node_num; i++) {
        m_nodes[i]->RenderContext2Node(model);
    }
}

INCLUDE_ASM("asm/nonmatchings/prlib/render", RenderContext2Node__7SpmNodeP13PrModelObject);

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/render", func_00145DB0);

/* prlib/render.cpp */
INCLUDE_ASM("asm/nonmatchings/prlib/render", ComposeGlobalMatrix__7SpmNodeP13PrModelObjectRCt8NaMATRIX3Zfi4i4);

/* prlib/renderstuff.h */
INCLUDE_ASM("asm/nonmatchings/prlib/render", func_00145E50);

/* prlib/render.cpp */
INCLUDE_ASM("asm/nonmatchings/prlib/render", ComposeGlobalMatrixWithoutVisibility__7SpmNodeP13PrModelObjectRCt8NaMATRIX3Zfi4i4);
