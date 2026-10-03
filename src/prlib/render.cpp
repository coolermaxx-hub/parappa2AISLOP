#include "render.h"

#include "dma.h"
#include "gifreg.h"
#include "model.h"
#include "renderstuff.h"
#include "scene.h"
#include "spram.h"

#include "animation.h"
#include "spadata.h"

#include "mfifo.h"

#include <nalib/namatrix.h>
#include <eekernel.h>
#include <eeregs.h>
#include <math.h>

#ifndef NON_MATCHING
/* This file's weak copy of the constructor is asm near the end of the file */
extern template NaVECTOR<float, 4>::NaVECTOR(const float& x, const float& y, const float& z, const float& w);
#endif

/* sdata */
static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)0x70000000;

extern bool AwfulStatus;

extern PrVu1InitPacket initVu1DmaPacket;

PR_EXTERN float PrGetMendererRatio();
void PrDrawAwfulBackground(sceGsFrame frame);
void PrWaitMendererTexture(sceGsDrawEnv1 *env, const sceGsFrame &frame, const sceGsXyoffset &xyoffset);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", Render__13PrSceneObject);
#else /* Scheduling: only register allocation of the frame/xyoffset copies (s3/s4 swapped) */
void PrSceneObject::Render() {
    FlushCache(WRITEBACK_DCACHE);

    if (PrGetMendererRatio() >= 1.5f && m_model_set.m_head != NULL) {
        prRenderStuff.InitializeEECore(this);
        PrDrawAwfulBackground(m_frame);
    }

    bool awful = true;
    prRenderStuff.ResetStatistics();
    prRenderStuff.m_statistics.render_time0 = *T3_COUNT;
    prSpramData->Initialize(this);

    PrModelObject *model = m_model_set.m_head;
    if (!(PrGetMendererRatio() >= 1.5f)) {
        awful = false;
    }
    AwfulStatus = awful;
    for (; model != NULL; model = model->m_list.next) {
        if (model->m_flags & 1) {
            if (!awful || (model->m_spm_image->m_flags & 0x100)) {
                prSpramData->InitializeModel(model);
                model->CalculateCurrentMatrix();
            }
        }
    }

    prRenderStuff.m_statistics.render_time1 = *T3_COUNT;
    sceGsFrame frame = m_frame;
    sceGsXyoffset xyoffset = m_xyoffset;
    PrWaitMendererTexture(m_draw_env, frame, xyoffset);
    prRenderStuff.m_statistics.render_time2 = *T3_COUNT;

    prRenderStuff.InitializeEECore(this);
    InitializeVu1();
    prSpramData->SendDisplayHeader();
    prRenderStuff.StartRender(this);
    prRenderStuff.m_transmit_array_size = 0;
    PrStartMfifo();

    model = m_model_set.m_head;
    if (model != NULL) {
        if (model->m_spm_image->m_flags & 0x200) {
            prRenderStuff.AppendDmaTag(&PrGetDmaStripGifRegister(eGifRegisterMode_Unk4)->m_tag);
            do {
                if (model->m_flags & 1) {
                    if (!awful || (model->m_spm_image->m_flags & 0x100)) {
                        prSpramData->InitializeModel(model);
                        model->RenderBackgroundScreenModel();
                    }
                }
                model = model->m_list.next;
            } while (model != unk98);
        }

        if (model != NULL && (model->m_spm_image->m_flags & 0x400)) {
            prRenderStuff.AppendDmaTag(&PrGetDmaStripGifRegister(eGifRegisterMode_Unk5)->m_tag);
            do {
                if (model->m_flags & 1) {
                    if (!awful || (model->m_spm_image->m_flags & 0x100)) {
                        prSpramData->InitializeModel(model);
                        model->RenderContext1Model();
                    }
                }
                model = model->m_list.next;
            } while (model != unk9C);
        }
    }

    prRenderStuff.AppendDmaTag(&PrGetDmaStripGifRegister(eGifRegisterMode_Unk0)->m_tag);
    for (; model != m_screen_model_list; model = model->m_list.next) {
        if (model->m_flags & 1) {
            if (!awful || (model->m_spm_image->m_flags & 0x100)) {
                prSpramData->InitializeModel(model);
                model->RenderContext1Model();
            }
        }
    }

    for (model = unk9C; model != m_screen_model_list; model = model->m_list.next) {
        if (model->m_flags & 1) {
            if (!awful || (model->m_spm_image->m_flags & 0x100)) {
                prSpramData->InitializeModel(model);
                model->RenderContext2Model();
            }
        }
    }

    prRenderStuff.m_statistics.render_time3 = *T3_COUNT;
    prRenderStuff.SortTransmitDmaArray();
    prRenderStuff.m_statistics.render_time4 = *T3_COUNT;

    if (prCurrentStage != 19) {
        prRenderStuff.AppendDmaTag(&PrGetDmaStripGifRegister(eGifRegisterMode_Unk1)->m_tag);
    }

    FlushCache(WRITEBACK_DCACHE);
    PrWaitMfifo();
    prRenderStuff.MergeRender();
    prRenderStuff.m_statistics.render_time5 = *T3_COUNT;
}
#endif

void PrSceneObject::InitializeVu1() {
    PrVu1InitPacket *packet = (PrVu1InitPacket*)PR_UNCACHED(&initVu1DmaPacket);
    sceGsZbuf zbuf = prRenderStuff.m_zbuf;
    packet->zbuf     = zbuf;
    packet->frame    = this->m_frame;
    packet->xyoffset = this->m_xyoffset;
    packet->scissor  = this->m_draw_env->scissor1;

    PrWaitDmaFinish(SCE_DMA_GIF);

    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0; /* Don't transfer the DMAtag */

    sceDmaSend(chan, &initVu1DmaPacket);
}

void PrSceneObject::PrepareScreenModelRender() {
    prRenderStuff.StartRender(this);
    prRenderStuff.m_transmit_array_size = 0;

    PrDmaStripForSetGifRegister *strip = PrGetDmaStripGifRegister(eGifRegisterMode_Unk3);
    prRenderStuff.AppendDmaTag(&strip->m_tag);

    for (PrModelObject *model = m_screen_model_list; model != NULL; model = model->m_list.next) {
        if (model->m_flags & 1) {
            prSpramData->InitializeModel(model);
            model->RenderScreenModelNode();
        }
    }

    strip = PrGetDmaStripGifRegister(eGifRegisterMode_Unk1);
    prRenderStuff.AppendDmaTag(&strip->m_tag);

    prRenderStuff.SortTransmitDmaArray();
    prRenderStuff.MergeRender();
}

void PrModelObject::CalculateCurrentMatrix() {
    PrSPRAM_DATA *spram = prSpramData;
    const NaMATRIX<float, 4, 4> *mtx = &this->m_matrix;

    spram->m_animation_time = m_animation_time;
    spram->m_current_model = this;
    SpaFileHeader *animation = m_animation;
    spram->m_animation = animation;
    this->m_scaled_disturbance = 1.0f;

    SpmFileHeader *spm = m_spm_image;

    if (m_position_animation != NULL) {
        float time = m_position_animation_time;
        NaMATRIX<float, 4, 4> pos;
        pos = *m_position_animation->unk50[0]->GetMatrix(time);
        mtx = &pos;
    }

    if (animation != NULL) {
        if (spm->m_flags & 0x20) {
            spm->CalculateClusterMatrixAnimation(this, *mtx);

            NaVECTOR<float, 4> v;
            NaMATRIX<float, 4, 4>& root = spm->m_nodes[0]->m_global_matrix;
            NaVECTOR<float, 4> scale(1.0f, 1.0f, 1.0f, 0.0f);

            NaVECTOR<float, 4> tmp;
            v = NaMATRIX<float, 4, 4>::Apply(tmp, root, scale);

            float disturbance = m_disturbance;
            float len = 0.0f;
            for (int i = 0; i < 4; i++) {
                len += v[i] * v[i];
            }

            this->m_scaled_disturbance = disturbance * sqrtf(len);
        } else {
            spm->CalculateCurrentMatrixAnimation(this, *mtx);
        }
    } else if (spm->m_flags & 0x20) {
        spm->CalculateClusterMatrix(this, *mtx);
    } else {
        spm->CalculateCurrentMatrix(this, *mtx);
    }
}

void SpmFileHeader::CalculateCurrentMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    m_nodes[0]->ComposeGlobalMatrix(model, arg1);

    for (u_int i = 1; i < m_node_num; i++) {
        SpmNode *node = m_nodes[i];
        node->ComposeGlobalMatrix(model, node->m_parent->m_global_matrix);
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateCurrentMatrixAnimation__13SpmFileHeaderP13PrModelObjectRCt8NaMATRIX3Zfi4i4);
#else
/*
 * The world matrix is the parent's times this node's local matrix. With
 * transitions active, the local matrix is also stored (blended) per model.
 * Inline in the original: inlined into CalculateCurrentMatrixAnimation,
 * with a weak copy (called by ComposeGlobalMatrix) at the end of this file.
 * Built here, gcc also inlines it into ComposeGlobalMatrix, and the copy
 * schedules the m_flags load differently.
 */
inline void SpmNode::ComposeGlobalMatrixWithoutVisibility(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    if (model->m_posture_matrices[0] != NULL) {
        /* Keep the local matrix for transitions, blended toward the previous one */
        if (m_flags & 0x1) {
            prSpramData->unk0 = NaMATRIX<float, 4, 4>::IDENT;
        } else {
            prSpramData->unk0 = this->m_local_matrix;
        }

        if (prSpramData->m_model_transaction_blend_ratio != 1.0f) {
            BlendTransitionMatrix(model, prSpramData->unk0);
        }

        this->m_global_matrix = arg1 * prSpramData->unk0;
        model->m_posture_matrices[model->m_active_transition][this->m_index] = prSpramData->unk0;
    } else if (m_flags & 0x1) {
        this->m_global_matrix = arg1;
    } else {
        this->m_global_matrix = arg1 * this->m_local_matrix;
    }

    if (m_flags & 0x8000) {
        ApplyBillboardMatrix();
    }
}

/*
 * Like ComposeGlobalMatrix, but visibility and the local matrix come from
 * the scene's current animation when the node has one. Always inlined in
 * the original, so it has no symbol.
 */
inline void SpmNode::ComposeGlobalMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    float time = prSpramData->m_animation_time;
    SpaFileHeader *animation = prSpramData->m_animation;
    SpmNode *parent = this->m_parent;

    if ((parent == NULL || (parent->m_flags & 0x4000)) && animation->IsNodeVisible(this, time)) {
        m_flags |= 0x4000;
    } else {
        m_flags &= ~0x4000;
    }

    if (!(m_flags & 0x4000)) {
        return;
    }

    SpaNodeAnimation *node_animation = animation->unk50[this->m_index];
    if (node_animation == NULL) {
        ComposeGlobalMatrixWithoutVisibility(model, arg1);
        return;
    }

    bool identity = node_animation->m_transform_count == 0;
    if (model->m_posture_matrices[0] != NULL) {
        /* Keep the local matrix for transitions, blended toward the previous one */
        if (identity) {
            prSpramData->unk0 = NaMATRIX<float, 4, 4>::IDENT;
        } else {
            prSpramData->unk0 = *node_animation->GetMatrix(time);
        }

        if (prSpramData->m_model_transaction_blend_ratio != 1.0f) {
            BlendTransitionMatrix(model, prSpramData->unk0);
        }

        this->m_global_matrix = arg1 * prSpramData->unk0;
        model->m_posture_matrices[model->m_active_transition][this->m_index] = prSpramData->unk0;
    } else if (identity) {
        this->m_global_matrix = arg1;
    } else {
        this->m_global_matrix = arg1 * *node_animation->GetMatrix(time);
    }

    if (m_flags & 0x8000) {
        ApplyBillboardMatrix();
    }
}

/* NON_MATCHING: register allocation and scheduling of the inlined node code */
void SpmFileHeader::CalculateCurrentMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    m_nodes[0]->ComposeGlobalMatrixAnimation(model, arg1);

    for (u_int i = 1; i < m_node_num; i++) {
        SpmNode *node = m_nodes[i];
        node->ComposeGlobalMatrixAnimation(model, node->m_parent->m_global_matrix);
    }
}
#endif

void SpmFileHeader::CalculateClusterMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    SpmNode *node = m_nodes[0];
    node->ComposeGlobalMatrix(model, arg1);
    if (node->m_flags & 0x1000) {
        const NaMATRIX<float, 4, 4>& b = node->m_bind_matrix;
        node->m_cluster_matrix = node->m_global_matrix * b;
    }

    for (u_int i = 1; i < m_node_num; i++) {
        node = m_nodes[i];
        node->ComposeGlobalMatrix(model, node->m_parent->m_global_matrix);
        if (node->m_flags & 0x1000) {
            const NaMATRIX<float, 4, 4>& b = node->m_bind_matrix;
            node->m_cluster_matrix = node->m_global_matrix * b;
        }
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateClusterMatrixAnimation__13SpmFileHeaderP13PrModelObjectRCt8NaMATRIX3Zfi4i4);
#else
/* NON_MATCHING: as CalculateCurrentMatrixAnimation */
void SpmFileHeader::CalculateClusterMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    SpmNode *node = m_nodes[0];
    node->ComposeGlobalMatrixAnimation(model, arg1);
    if (node->m_flags & 0x1000) {
        const NaMATRIX<float, 4, 4>& b = node->m_bind_matrix;
        node->m_cluster_matrix = node->m_global_matrix * b;
    }

    for (u_int i = 1; i < m_node_num; i++) {
        node = m_nodes[i];
        node->ComposeGlobalMatrixAnimation(model, node->m_parent->m_global_matrix);
        if (node->m_flags & 0x1000) {
            const NaMATRIX<float, 4, 4>& b = node->m_bind_matrix;
            node->m_cluster_matrix = node->m_global_matrix * b;
        }
    }
}
#endif

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

void SpmNode::ModifySimpleDmaPacket(PrVuNodeHeaderDmaPacket *packet) {
    PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
    uc->m_matrix = this->m_global_matrix;
    uc->m_disturbance = prSpramData->m_disturbance;

    float du = this->m_u_scroll;
    float dv = this->m_v_scroll;

    if (du != 0.0f) {
        uc->m_u_offset += du;
        if (uc->m_u_offset > 1.0f) {
            uc->m_u_offset -= 1.0f;
        } else if (uc->m_u_offset < 0.0f) {
            uc->m_u_offset += 1.0f;
        }
    }

    if (dv != 0.0f) {
        uc->m_v_offset += dv;
        if (uc->m_v_offset > 1.0f) {
            uc->m_v_offset -= 1.0f;
        } else if (uc->m_v_offset < 0.0f) {
            uc->m_v_offset += 1.0f;
        }
    }
}

void SpmNode::RenderContext1Node(PrModelObject *model) {
    prRenderStuff.m_statistics.node_num++;

    if (this->m_flags & 0x2000) {
        return;
    }

    if ((this->m_flags & 0x4000) && (!AwfulStatus || (this->m_flags & 0x400000))) {
        PrVuNodeHeaderDmaPacket *packet = this->m_packets[0];
        if (packet != NULL) {
            prRenderStuff.m_statistics.opaque_context1_node_num++;
            ModifySimpleDmaPacket(packet);
            prRenderStuff.AppendDmaTag(&packet->m_tag);
        }

        packet = this->m_packets[1];
        if (packet != NULL) {
            prRenderStuff.m_statistics.transmit_context1_node_num++;
            ModifySimpleDmaPacket(packet);

            float sp0[4];

            asm volatile(
                "lqc2     vf13,   0x0(%0)       \n\t"
                "lqc2     vf14,  0x10(%0)       \n\t"
                "lqc2     vf15,  0x20(%0)       \n\t"
                "lqc2     vf16,  0x30(%0)       \n\t"
            : : "r"(&this->m_global_matrix));

            asm volatile(
                "lqc2     vf04,  0x0(%0)        \n\t"
                "vmulax   ACC,   vf13,    vf04  \n\t"
                "vmadday  ACC,   vf14,    vf04  \n\t"
                "vmaddaz  ACC,   vf15,    vf04  \n\t"
                "vmaddw   vf17,  vf16,    vf04  \n\t"
            : : "r"(&this->m_center));

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
            : : "r"(sp0), "r"(&prSpramData->m_view_projection_matrix));

            float f12 = sp0[2] / sp0[3];
            if (sp0[3] == 0.0f) {
                f12 = sp0[2] * 3.40282347e+38f;
            }

            prRenderStuff.AppendTransmitDmaTag(&packet->m_tag, this->m_draw_group, f12);
        }

        if (this->m_flags & SPM_NODE_CONTOUR) {
            SpmComplexNode *complex = static_cast<SpmComplexNode*>(this);
            complex->RenderContour(model);
        }
    }
}

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

    PrVuNodeHeaderDmaPacket *packet = this->m_packets[0];
    if (packet != NULL) {
        prRenderStuff.m_statistics.opaque_context1_node_num++;
        PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
        uc->m_matrix = this->m_global_matrix;
        prRenderStuff.AppendDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF));
    }

    PrVuNodeHeaderDmaPacket *packet2 = this->m_packets[1];
    if (packet2 != NULL) {
        prRenderStuff.m_statistics.transmit_context1_node_num++;
        PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet2);
        uc->m_matrix = this->m_global_matrix;

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
        : : "r"(&tmp), "r"(&this->m_global_matrix), "r"(&this->m_center) : "memory");

        pos = tmp;

        prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF), this->m_draw_group, -pos[2]);
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
            PrVuNodeHeaderDmaPacket *packet = this->m_packets[i];
            if (packet != NULL) {
                packet = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
                packet->m_matrix = this->m_global_matrix;
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
    prSpramData->m_animation = m_animation;
    prSpramData->m_animation_time = m_animation_time;
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

void SpmNode::RenderContext2Node(PrModelObject *model) {
    if (!(this->m_flags & 0x2000)) {
        return;
    }

    if ((this->m_flags & 0x4000) && (!AwfulStatus || (this->m_flags & 0x400000))) {
        prRenderStuff.m_statistics.opaque_context2_node_num++;

        if (this->m_flags & SPM_NODE_CLUSTER) {
            SpmClusterGeometryNode *cluster = static_cast<SpmClusterGeometryNode*>(this);
            cluster->RenderClusterNode(model);
        } else if (this->m_flags & SPM_NODE_SHAPE) {
            SpmShapeNode *shape = static_cast<SpmShapeNode*>(this);
            shape->RenderShapeNode(model);
        } else {
            PrVuNodeHeaderDmaPacket *packet = this->m_packets[0];
            if (packet != NULL) {
                PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
                uc->m_matrix = this->m_global_matrix;
                uc->m_disturbance = prSpramData->m_disturbance;
                prRenderStuff.RenderNodeEECore((PrVuNodeHeaderDmaPacket*)((u_int)uc & 0x0FFFFFFF));
            }

            PrVuNodeHeaderDmaPacket *uc = this->m_packets[1];
            if (uc != NULL) {
                uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(uc);
                const NaMATRIX<float, 4, 4>& m = this->m_global_matrix;
                uc->m_matrix = m;
                uc->m_disturbance = prSpramData->m_disturbance;

                NaVECTOR<float, 4> pos;
                {
                    NaMATRIX<float, 4, 4> mtx = prSpramData->m_view_projection_matrix * m;
                    NaVECTOR<float, 4> tmp;
                    pos = NaMATRIX<float, 4, 4>::Apply(tmp, mtx, this->m_center);
                }

                float z = pos[2] / pos[3];
                u_int key = this->m_draw_group;
                if (pos[3] == 0.0f) {
                    z = pos[2] * 3.40282347e+38f;
                }

                prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF), key, z);
            }
        }

        if (this->m_flags & SPM_NODE_CONTOUR) {
            SpmComplexNode *complex = static_cast<SpmComplexNode*>(this);
            complex->RenderContour(model);
        }
    }
}

/* nalib/navector.h: weak copy of the 4-argument NaVECTOR constructor */
#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", __t8NaVECTOR2Zfi4RCfT1T1T1);
#endif

/* prlib/render.cpp */
void SpmNode::ComposeGlobalMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    SpmNode *parent = this->m_parent;

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
/* Weak copy of the inline ComposeGlobalMatrixWithoutVisibility; its source is above CalculateCurrentMatrixAnimation */
#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", ComposeGlobalMatrixWithoutVisibility__7SpmNodeP13PrModelObjectRCt8NaMATRIX3Zfi4i4);
#endif
