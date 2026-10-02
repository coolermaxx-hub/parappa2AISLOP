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

/* render.cpp's own out-of-line copy of the NaVECTOR<float, 4> constructor */
NaVECTOR<float, 4>* CtorVector_tmp_render(NaVECTOR<float, 4> *v, const float& x, const float& y, const float& z, const float& w) asm("__t8NaVECTOR2Zfi4RCfT1T1T1");

/* sdata */
PrSPRAM_DATA *prSpramData_tmp_render = (PrSPRAM_DATA*)0x70000000;

extern bool AwfulStatus;

extern PrVu1InitPacket initVu1DmaPacket;

PR_EXTERN float PrGetMendererRatio();
void PrDrawAwfulBackground(sceGsFrame frame);
void PrWaitMendererTexture(sceGsDrawEnv1 *env, const sceGsFrame &frame, const sceGsXyoffset &xyoffset);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", Render__13PrSceneObject);
#else /* Scheduling: the AwfulStatus store and the frame/xyoffset copies */
void PrSceneObject::Render() {
    FlushCache(WRITEBACK_DCACHE);

    if (PrGetMendererRatio() >= 1.5f && m_model_set.m_head != NULL) {
        prRenderStuff.InitializeEECore(this);
        PrDrawAwfulBackground(unk50);
    }

    bool awful = true;
    prRenderStuff.ResetStatistics();
    prRenderStuff.m_statistics.render_time0 = *T3_COUNT;
    prSpramData_tmp_render->Initialize(this);

    PrModelObject *model = m_model_set.m_head;
    if (!(PrGetMendererRatio() >= 1.5f)) {
        awful = false;
    }
    AwfulStatus = awful;

    for (; model != NULL; model = model->m_list.next) {
        if (model->m_flags & 1) {
            if (!awful || (model->m_spm_image->m_flags & 0x100)) {
                prSpramData_tmp_render->InitializeModel(model);
                model->CalculateCurrentMatrix();
            }
        }
    }

    prRenderStuff.m_statistics.render_time1 = *T3_COUNT;
    sceGsFrame frame = unk50;
    sceGsXyoffset xyoffset = unk58;
    PrWaitMendererTexture(unk70, frame, xyoffset);
    prRenderStuff.m_statistics.render_time2 = *T3_COUNT;

    prRenderStuff.InitializeEECore(this);
    InitializeVu1();
    prSpramData_tmp_render->SendDisplayHeader();
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
                        prSpramData_tmp_render->InitializeModel(model);
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
                        prSpramData_tmp_render->InitializeModel(model);
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
                prSpramData_tmp_render->InitializeModel(model);
                model->RenderContext1Model();
            }
        }
    }

    for (model = unk9C; model != m_screen_model_list; model = model->m_list.next) {
        if (model->m_flags & 1) {
            if (!awful || (model->m_spm_image->m_flags & 0x100)) {
                prSpramData_tmp_render->InitializeModel(model);
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
    packet->frame    = this->unk50;
    packet->xyoffset = this->unk58;
    packet->scissor  = this->unk70->scissor1;

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
            prSpramData_tmp_render->InitializeModel(model);
            model->RenderScreenModelNode();
        }
    }

    strip = PrGetDmaStripGifRegister(eGifRegisterMode_Unk1);
    prRenderStuff.AppendDmaTag(&strip->m_tag);

    prRenderStuff.SortTransmitDmaArray();
    prRenderStuff.MergeRender();
}

void PrModelObject::CalculateCurrentMatrix() {
    PrSPRAM_DATA *spram = prSpramData_tmp_render;
    const NaMATRIX<float, 4, 4> *mtx = &this->unk10;

    spram->m_animation_time = m_animation_time;
    spram->m_current_model = this;
    SpaFileHeader *animation = m_animation;
    spram->m_animation = animation;
    this->unkA4 = 1.0f;

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
            NaVECTOR<float, 4> scale;
            NaMATRIX<float, 4, 4>& root = spm->m_nodes[0]->unk40;
            CtorVector_tmp_render(&scale, 1.0f, 1.0f, 1.0f, 0.0f);

            NaVECTOR<float, 4> tmp;
            v = NaMATRIX<float, 4, 4>::Apply(tmp, root, scale);

            float disturbance = m_disturbance;
            float len = 0.0f;
            for (int i = 0; i < 4; i++) {
                len += v[i] * v[i];
            }

            this->unkA4 = disturbance * sqrtf(len);
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
        node->ComposeGlobalMatrix(model, node->unk164->unk40);
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateCurrentMatrixAnimation__13SpmFileHeaderP13PrModelObjectRCt8NaMATRIX3Zfi4i4);
#else
// NON_MATCHING: 11 opcode hunks left (gcc hoists the 1.0f blend constant in the loop copy)
static inline void ComposeNoVis_tmp(SpmNode *node, PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    if (model->unk7C[0] != NULL) {
        PrSPRAM_DATA *spram;
        if (node->m_flags & 0x1) {
            spram = prSpramData_tmp_render;
            spram->unk0 = NaMATRIX<float, 4, 4>::IDENT;
        } else {
            spram = prSpramData_tmp_render;
            spram->unk0 = node->unk0;
        }

        if (spram->m_model_transaction_blend_ratio != 1.0f) {
            node->BlendTransitionMatrix(model, spram->unk0);
            spram = prSpramData_tmp_render;
        }

        node->unk40 = arg1 * spram->unk0;
        int idx = node->unk150;
        model->unk7C[model->m_active_transition][idx] = spram->unk0;
    } else if (node->m_flags & 0x1) {
        node->unk40 = arg1;
    } else {
        node->unk40 = arg1 * node->unk0;
    }

    if (node->m_flags & 0x8000) {
        node->ApplyBillboardMatrix();
    }
}

static inline void ComposeAnim_tmp(SpmNode *node, PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    float time = prSpramData_tmp_render->m_animation_time;
    SpaFileHeader *animation = prSpramData_tmp_render->m_animation;
    SpmNode *parent = node->unk164;

    if (parent == NULL || (parent->m_flags & 0x4000)) {
        if (animation->IsNodeVisible(node, time)) {
            node->m_flags |= 0x4000;
            goto done;
        }
    }
    node->m_flags &= ~0x4000;
    done:

    if (node->m_flags & 0x4000) {
        SpaNodeAnimation *na = animation->unk50[node->unk150];
        if (na == NULL) {
            ComposeNoVis_tmp(node, model, arg1);
        } else {
            int e = 0;
            if (model->unk7C[0] != NULL) {
                PrSPRAM_DATA *spram;
                e = 0; if (na->unk8 == 0) e = 1;
                if (e & 1) {
                    spram = prSpramData_tmp_render;
                    spram->unk0 = NaMATRIX<float, 4, 4>::IDENT;
                } else {
                    const NaMATRIX<float, 4, 4> *m = na->GetMatrix(time);
                    spram = prSpramData_tmp_render;
                    spram->unk0 = *m;
                }

                if (spram->m_model_transaction_blend_ratio != 1.0f) {
                    node->BlendTransitionMatrix(model, spram->unk0);
                    spram = prSpramData_tmp_render;
                }

                node->unk40 = arg1 * spram->unk0;
                int idx = node->unk150;
                model->unk7C[model->m_active_transition][idx] = spram->unk0;
            } else if ((e = 0, (na->unk8 == 0 ? (e = 1) : 0), e & 1)) {
                node->unk40 = arg1;
            } else {
                NaMATRIX<float, 4, 4> tmp = *na->GetMatrix(time);
                node->unk40 = arg1 * tmp;
            }

            if (node->m_flags & 0x8000) {
                node->ApplyBillboardMatrix();
            }
        }
    }
}

void SpmFileHeader::CalculateCurrentMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    ComposeAnim_tmp(m_nodes[0], model, arg1);

    for (u_int i = 1; i < m_node_num; i++) {
        SpmNode *node = m_nodes[i];
        ComposeAnim_tmp(node, model, node->unk164->unk40);
    }
}
#endif

void SpmFileHeader::CalculateClusterMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    SpmNode *node = m_nodes[0];
    node->ComposeGlobalMatrix(model, arg1);
    if (node->m_flags & 0x1000) {
        const NaMATRIX<float, 4, 4>& b = node->unk80;
        node->unkC0 = node->unk40 * b;
    }

    for (u_int i = 1; i < m_node_num; i++) {
        node = m_nodes[i];
        node->ComposeGlobalMatrix(model, node->unk164->unk40);
        if (node->m_flags & 0x1000) {
            const NaMATRIX<float, 4, 4>& b = node->unk80;
            node->unkC0 = node->unk40 * b;
        }
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", CalculateClusterMatrixAnimation__13SpmFileHeaderP13PrModelObjectRCt8NaMATRIX3Zfi4i4);
#else
// NON_MATCHING: same per-node composition as CalculateCurrentMatrixAnimation plus the 0x1000 unkC0 step
void SpmFileHeader::CalculateClusterMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    SpmNode *node = m_nodes[0];
    ComposeAnim_tmp(node, model, arg1);
    if (node->m_flags & 0x1000) {
        const NaMATRIX<float, 4, 4>& b = node->unk80;
        node->unkC0 = node->unk40 * b;
    }

    for (u_int i = 1; i < m_node_num; i++) {
        node = m_nodes[i];
        ComposeAnim_tmp(node, model, node->unk164->unk40);
        if (node->m_flags & 0x1000) {
            const NaMATRIX<float, 4, 4>& b = node->unk80;
            node->unkC0 = node->unk40 * b;
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
    uc->m_matrix = this->unk40;
    uc->unk68 = prSpramData_tmp_render->m_disturbance;

    float du = this->unk180;
    float dv = this->unk184;

    if (du != 0.0f) {
        uc->unk70 += du;
        if (uc->unk70 > 1.0f) {
            uc->unk70 -= 1.0f;
        } else if (uc->unk70 < 0.0f) {
            uc->unk70 += 1.0f;
        }
    }

    if (dv != 0.0f) {
        uc->unk74 += dv;
        if (uc->unk74 > 1.0f) {
            uc->unk74 -= 1.0f;
        } else if (uc->unk74 < 0.0f) {
            uc->unk74 += 1.0f;
        }
    }
}

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

            PrVuNodeHeaderDmaPacket *uc = this->unk16C[1];
            if (uc != NULL) {
                uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(uc);
                const NaMATRIX<float, 4, 4>& m = this->unk40;
                uc->m_matrix = m;
                uc->unk68 = prSpramData_tmp_render->m_disturbance;

                NaVECTOR<float, 4> pos;
                {
                    NaMATRIX<float, 4, 4> mtx = prSpramData_tmp_render->m_view_projection_matrix * m;
                    NaVECTOR<float, 4> tmp;
                    pos = NaMATRIX<float, 4, 4>::Apply(tmp, mtx, this->unk140);
                }

                float z = pos[2] / pos[3];
                u_int key = this->unk188;
                if (pos[3] == 0.0f) {
                    z = pos[2] * 3.40282347e+38f;
                }

                prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF), key, z);
            }
        }

        if (this->m_flags & 0x40) {
            SpmComplexNode *complex = reinterpret_cast<SpmComplexNode*>(this);
            complex->RenderContour(model);
        }
    }
}

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/render", __t8NaVECTOR2Zfi4RCfT1T1T1);

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
#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/render", ComposeGlobalMatrixWithoutVisibility__7SpmNodeP13PrModelObjectRCt8NaMATRIX3Zfi4i4);
#else
/* Scheduling: m_flags is loaded before prSpramData is reloaded after BlendTransitionMatrix */
void SpmNode::ComposeGlobalMatrixWithoutVisibility(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1) {
    if (model->unk7C[0] != NULL) {
        PrSPRAM_DATA *spram;
        if (m_flags & 0x1) {
            spram = prSpramData_tmp_render;
            spram->unk0 = NaMATRIX<float, 4, 4>::IDENT;
        } else {
            spram = prSpramData_tmp_render;
            spram->unk0 = this->unk0;
        }

        if (spram->m_model_transaction_blend_ratio != 1.0f) {
            BlendTransitionMatrix(model, spram->unk0);
            spram = prSpramData_tmp_render;
        }

        this->unk40 = arg1 * spram->unk0;
        int idx = this->unk150;
        model->unk7C[model->m_active_transition][idx] = spram->unk0;
    } else if (m_flags & 0x1) {
        this->unk40 = arg1;
    } else {
        this->unk40 = arg1 * this->unk0;
    }

    if (m_flags & 0x8000) {
        ApplyBillboardMatrix();
    }
}
#endif
