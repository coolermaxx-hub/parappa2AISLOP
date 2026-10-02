#include "render.h"

#include "dma.h"
#include "model.h"
#include "renderstuff.h"
#include "scene.h"
#include "spram.h"

#include <nalib/namatrix.h>

/* sdata */
extern PrSPRAM_DATA *prSpramData_tmp_render;

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

void SpmFileHeader::CalculateCurrentMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    m_nodes[0]->ComposeGlobalMatrix(model, arg1);

    for (u_int i = 1; i < m_node_num; i++) {
        SpmNode *node = m_nodes[i];
        node->ComposeGlobalMatrix(model, node->unk164->unk40);
    }
}

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

    if (this->m_flags & 0x2000) {
        return;
    }

    if ((this->m_flags & 0x4000) && (!AwfulStatus || (this->m_flags & 0x400000))) {
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
            : : "r"(sp0), "r"(&prSpramData_tmp_render->m_view_projection_matrix));

            float f12 = sp0[2] / sp0[3];
            if (sp0[3] == 0.0f) {
                f12 = sp0[2] * 3.40282347e+38f;
            }

            prRenderStuff.AppendTransmitDmaTag(&packet->m_tag, this->unk188, f12);
        }

        if (this->m_flags & 0x40) {
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

void SpmNode::RenderScreenModelNode() {
    prRenderStuff.m_statistics.node_num++;

    if (!(m_flags & 0x4000)) {
        return;
    }

    PrVuNodeHeaderDmaPacket *packet = this->unk16C[0];
    if (packet != NULL) {
        prRenderStuff.m_statistics.opaque_context1_node_num++;
        PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
        uc->m_matrix = this->unk40;
        prRenderStuff.AppendDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF));
    }

    PrVuNodeHeaderDmaPacket *packet2 = this->unk16C[1];
    if (packet2 != NULL) {
        prRenderStuff.m_statistics.transmit_context1_node_num++;
        PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet2);
        uc->m_matrix = this->unk40;

        NaVECTOR<float, 4> pos;
        NaVECTOR<float, 4> tmp;
        asm volatile(
            "lqc2     $vf4,  0x0(%1)        \n\t"
            "lqc2     $vf5,  0x10(%1)       \n\t"
            "lqc2     $vf6,  0x20(%1)       \n\t"
            "lqc2     $vf7,  0x30(%1)       \n\t"
            "lqc2     $vf8,  0x0(%2)        \n\t"
            "vmulax   ACC,   $vf4,   $vf8   \n\t"
            "vmadday  ACC,   $vf5,   $vf8   \n\t"
            "vmaddaz  ACC,   $vf6,   $vf8   \n\t"
            "vmaddw   $vf9,  $vf7,   $vf8   \n\t"
            "sqc2     $vf9,  0x0(%0)        \n\t"
        : : "r"(&tmp), "r"(&this->unk40), "r"(&this->unk140) : "memory");

        pos = tmp;

        prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF), this->unk188, -pos[2]);
    }
}

void PrModelObject::RenderBackgroundScreenModel() {
    m_spm_image->RenderBackgroundScreenModel();
    m_rendered_once = 1;
}

void SpmFileHeader::RenderBackgroundScreenModel() {
    for (u_int i = 0; i < m_node_num; i++) {
        m_nodes[i]->RenderBackgroundScreenModel();
    }
}

void SpmNode::RenderBackgroundScreenModel() {
    prRenderStuff.m_statistics.node_num++;

    if ((m_flags & 0x4000) && (!AwfulStatus || (m_flags & 0x400000))) {
        for (u_int i = 0; i < 2; i++) {
            PrVuNodeHeaderDmaPacket *packet = this->unk16C[i];
            if (packet != NULL) {
                packet = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
                packet->m_matrix = this->unk40;
                prRenderStuff.AppendDmaTag((sceDmaTag*)((u_int)packet & 0x0FFFFFFF));

                if (i == 0) {
                    prRenderStuff.m_statistics.opaque_context1_node_num++;
                } else {
                    prRenderStuff.m_statistics.transmit_context1_node_num++;
                }
            }
        }
    }
}

void PrModelObject::RenderContext2Model() {
    SpmFileHeader *spm = m_spm_image;
    prSpramData_tmp_render->m_animation = m_animation;
    prSpramData_tmp_render->m_animation_time = m_animation_time;
    spm->RenderContext2Model(this);
    m_rendered_once = 1;
}

void SpmFileHeader::RenderContext2Model(PrModelObject *model) {
    if (m_flags & 0x8) {
        return;
    }

    for (u_int i = 0; i < m_node_num; i++) {
        m_nodes[i]->RenderContext2Node(model);
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", RenderContext2Node__7SpmNodeP13PrModelObject);
#else /* Need to match .sdata, stack order */
void SpmNode::RenderContext2Node(PrModelObject *model) {
    if (!(this->m_flags & 0x2000)) {
        return;
    }

    if ((this->m_flags & 0x4000) && (!AwfulStatus || (this->m_flags & 0x400000))) {
        prRenderStuff.m_statistics.opaque_context2_node_num++;

        if (this->m_flags & 0x10) {
            SpmClusterGeometryNode *cluster = reinterpret_cast<SpmClusterGeometryNode*>(this);
            cluster->RenderClusterNode(model);
        } else if (this->m_flags & 0x20) {
            SpmShapeNode *shape = reinterpret_cast<SpmShapeNode*>(this);
            shape->RenderShapeNode(model);
        } else {
            PrVuNodeHeaderDmaPacket *packet = this->unk16C[0];
            if (packet != NULL) {
                PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
                uc->m_matrix = this->unk40;
                uc->unk68 = prSpramData_tmp_render->m_disturbance;
                prRenderStuff.RenderNodeEECore((PrVuNodeHeaderDmaPacket*)((u_int)uc & 0x0FFFFFFF));
            }

            PrVuNodeHeaderDmaPacket *packet2 = this->unk16C[1];
            if (packet2 != NULL) {
                PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet2);
                uc->m_matrix = this->unk40;
                uc->unk68 = prSpramData_tmp_render->m_disturbance;

                NaVECTOR<float, 4> pos;
                {
                    NaMATRIX<float, 4, 4> tmp;
                    asm volatile(
                        "lqc2     $vf4,  0x0(%1)        \n\t"
                        "lqc2     $vf5,  0x10(%1)       \n\t"
                        "lqc2     $vf6,  0x20(%1)       \n\t"
                        "lqc2     $vf7,  0x30(%1)       \n\t"
                        "lqc2     $vf8,  0x0(%2)        \n\t"
                        "vmulax   ACC,   $vf4,   $vf8   \n\t"
                        "vmadday  ACC,   $vf5,   $vf8   \n\t"
                        "vmaddaz  ACC,   $vf6,   $vf8   \n\t"
                        "vmaddw   $vf9,  $vf7,   $vf8   \n\t"
                        "sqc2     $vf9,  0x0(%0)        \n\t"
                        "lqc2     $vf8,  0x10(%2)       \n\t"
                        "vmulax   ACC,   $vf4,   $vf8   \n\t"
                        "vmadday  ACC,   $vf5,   $vf8   \n\t"
                        "vmaddaz  ACC,   $vf6,   $vf8   \n\t"
                        "vmaddw   $vf9,  $vf7,   $vf8   \n\t"
                        "sqc2     $vf9,  0x10(%0)       \n\t"
                        "lqc2     $vf8,  0x20(%2)       \n\t"
                        "vmulax   ACC,   $vf4,   $vf8   \n\t"
                        "vmadday  ACC,   $vf5,   $vf8   \n\t"
                        "vmaddaz  ACC,   $vf6,   $vf8   \n\t"
                        "vmaddw   $vf9,  $vf7,   $vf8   \n\t"
                        "sqc2     $vf9,  0x20(%0)       \n\t"
                        "lqc2     $vf8,  0x30(%2)       \n\t"
                        "vmulax   ACC,   $vf4,   $vf8   \n\t"
                        "vmadday  ACC,   $vf5,   $vf8   \n\t"
                        "vmaddaz  ACC,   $vf6,   $vf8   \n\t"
                        "vmaddw   $vf9,  $vf7,   $vf8   \n\t"
                        "sqc2     $vf9,  0x30(%0)       \n\t"
                    : : "r"(&tmp), "r"(&prSpramData_tmp_render->m_view_projection_matrix), "r"(&this->unk40));

                    NaMATRIX<float, 4, 4> mtx;
                    mtx = tmp;

                    NaVECTOR<float, 4> *v = (NaVECTOR<float, 4>*)&tmp;
                    asm volatile(
                        "lqc2     $vf4,  0x0(%1)        \n\t"
                        "lqc2     $vf5,  0x10(%1)       \n\t"
                        "lqc2     $vf6,  0x20(%1)       \n\t"
                        "lqc2     $vf7,  0x30(%1)       \n\t"
                        "lqc2     $vf8,  0x0(%2)        \n\t"
                        "vmulax   ACC,   $vf4,   $vf8   \n\t"
                        "vmadday  ACC,   $vf5,   $vf8   \n\t"
                        "vmaddaz  ACC,   $vf6,   $vf8   \n\t"
                        "vmaddw   $vf9,  $vf7,   $vf8   \n\t"
                        "sqc2     $vf9,  0x0(%0)        \n\t"
                    : : "r"(v), "r"(&mtx), "r"(&this->unk140));
                    pos = *v;
                }

                float z = pos[2] / pos[3];
                if (pos[3] == 0.0f) {
                    z = pos[2] * 3.40282347e+38f;
                }

                prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF), this->unk188, z);
            }
        }

        if (this->m_flags & 0x40) {
            SpmComplexNode *complex = reinterpret_cast<SpmComplexNode*>(this);
            complex->RenderContour(model);
        }
    }
}
#endif

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/render", func_00145DB0);

/* prlib/render.cpp */
void SpmNode::ComposeGlobalMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    SpmNode *parent = this->unk164;

    if (parent != NULL && !(parent->m_flags & 0x4000)) {
        m_flags &= ~0x4000;
    } else {
        bool visible = !(m_flags & 0x20000);
        if (visible) {
            m_flags |= 0x4000;
        } else {
            m_flags &= ~0x4000;
        }
    }

    if (m_flags & 0x4000) {
        ComposeGlobalMatrixWithoutVisibility(model, arg1);
    }
}

/* prlib/renderstuff.h */
INCLUDE_ASM("asm/nonmatchings/prlib/render", func_00145E50);

/* prlib/render.cpp */
INCLUDE_ASM("asm/nonmatchings/prlib/render", ComposeGlobalMatrixWithoutVisibility__7SpmNodeP13PrModelObjectRCt8NaMATRIX3Zfi4i4);
