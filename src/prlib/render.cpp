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

/* sdata */
static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)EE_SCRATCHPAD_BASE;

static bool AwfulStatus;

// Zero-initialised A+D packet: the frame, zbuf, xyoffset and scissor values are
// filled in by InitializeVu1. The DMA tag is a "refe" transfer of the five
// quadwords of the GIF tag and its four registers that follow it.
PrVu1InitPacket initVu1DmaPacket = {
    { 5, 0, 0, reinterpret_cast<sceDmaTag*>(&initVu1DmaPacket.giftag), { 0, 0 } },
    { 4, 1, 0, 0, 0, 0, 0, 1, SCE_GIF_PACKED_AD },
    {}, SCE_GS_FRAME_1,
    {}, SCE_GS_ZBUF_1,
    {}, SCE_GS_XYOFFSET_1,
    {}, SCE_GS_SCISSOR_1,
};

PR_EXTERN float PrGetMendererRatio();
void PrDrawAwfulBackground(sceGsFrame frame);
void PrWaitMendererTexture(sceGsDrawEnv1 *env, const sceGsFrame &frame, const sceGsXyoffset &xyoffset);

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

    awful = PrGetMendererRatio() >= 1.5f;
    AwfulStatus = awful;
    PrModelObject *model = m_model_set.m_head;
    for (; model != NULL; model = model->m_list.next) {
        if ((model->m_flags & ePrModelEnabled) && (!awful || (model->m_spm_image->m_flags & eSpmFileDrawnInAwful))) {
            prSpramData->InitializeModel(model);
            model->CalculateCurrentMatrix();
        }
    }

    prRenderStuff.m_statistics.render_time1 = *T3_COUNT;
    sceGsFrame frame = m_frame;
    sceGsXyoffset xyoffset = m_xyoffset;
    PrWaitMendererTexture(m_drawEnv, frame, xyoffset);
    prRenderStuff.m_statistics.render_time2 = *T3_COUNT;

    prRenderStuff.InitializeEECore(this);
    InitializeVu1();
    prSpramData->SendDisplayHeader();
    prRenderStuff.StartRender(this);
    prRenderStuff.m_transmit_array_size = 0;
    PrStartMfifo();

    model = m_model_set.m_head;
    if (model != NULL) {
        if (model->m_spm_image->m_flags & eSpmFileBackgroundLayer) {
            prRenderStuff.AppendDmaTag(&PrGetDmaStripGifRegister(eGifRegisterMode_Background)->m_tag);
            do {
                if (model->m_flags & ePrModelEnabled) {
                    if (!awful || (model->m_spm_image->m_flags & eSpmFileDrawnInAwful)) {
                        prSpramData->InitializeModel(model);
                        model->RenderBackgroundScreenModel();
                    }
                }
                model = model->m_list.next;
            } while (model != m_preSceneModelList);
        }

        if (model != NULL && (model->m_spm_image->m_flags & eSpmFilePreSceneLayer)) {
            prRenderStuff.AppendDmaTag(&PrGetDmaStripGifRegister(eGifRegisterMode_PreScene)->m_tag);
            do {
                if (model->m_flags & ePrModelEnabled) {
                    if (!awful || (model->m_spm_image->m_flags & eSpmFileDrawnInAwful)) {
                        prSpramData->InitializeModel(model);
                        model->RenderContext1Model();
                    }
                }
                model = model->m_list.next;
            } while (model != m_normalModelList);
        }
    }

    prRenderStuff.AppendDmaTag(&PrGetDmaStripGifRegister(eGifRegisterMode_SceneModel)->m_tag);
    for (; model != m_screen_model_list; model = model->m_list.next) {
        if (model->m_flags & ePrModelEnabled) {
            if (!awful || (model->m_spm_image->m_flags & eSpmFileDrawnInAwful)) {
                prSpramData->InitializeModel(model);
                model->RenderContext1Model();
            }
        }
    }

    for (model = m_normalModelList; model != m_screen_model_list; model = model->m_list.next) {
        if (model->m_flags & ePrModelEnabled) {
            if (!awful || (model->m_spm_image->m_flags & eSpmFileDrawnInAwful)) {
                prSpramData->InitializeModel(model);
                model->RenderContext2Model();
            }
        }
    }

    prRenderStuff.m_statistics.render_time3 = *T3_COUNT;
    prRenderStuff.SortTransmitDmaArray();
    prRenderStuff.m_statistics.render_time4 = *T3_COUNT;

    if (prCurrentStage != PR_STAGE_TITLE) {
        prRenderStuff.AppendDmaTag(&PrGetDmaStripGifRegister(eGifRegisterMode_NoZWrite)->m_tag);
    }

    FlushCache(WRITEBACK_DCACHE);
    PrWaitMfifo();
    prRenderStuff.MergeRender();
    prRenderStuff.m_statistics.render_time5 = *T3_COUNT;
}

void PrSceneObject::InitializeVu1() {
    PrVu1InitPacket *packet = (PrVu1InitPacket*)PR_UNCACHED(&initVu1DmaPacket);
    sceGsZbuf zbuf = prRenderStuff.m_zbuf;
    packet->zbuf     = zbuf;
    packet->frame    = this->m_frame;
    packet->xyoffset = this->m_xyoffset;
    packet->scissor  = this->m_drawEnv->scissor1;

    PrWaitDmaFinish(SCE_DMA_GIF);

    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_GIF);
    chan->chcr.TTE = 0; /* Don't transfer the DMAtag */

    sceDmaSend(chan, &initVu1DmaPacket);
}

void PrSceneObject::PrepareScreenModelRender() {
    prRenderStuff.StartRender(this);
    prRenderStuff.m_transmit_array_size = 0;

    PrDmaStripForSetGifRegister *strip = PrGetDmaStripGifRegister(eGifRegisterMode_ScreenModel);
    prRenderStuff.AppendDmaTag(&strip->m_tag);

    for (PrModelObject *model = m_screen_model_list; model != NULL; model = model->m_list.next) {
        if (model->m_flags & ePrModelEnabled) {
            prSpramData->InitializeModel(model);
            model->RenderScreenModelNode();
        }
    }

    strip = PrGetDmaStripGifRegister(eGifRegisterMode_NoZWrite);
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
    this->m_scaledDisturbance = 1.0f;

    SpmFileHeader *spm = m_spm_image;

    NaMATRIX<float, 4, 4> pos;
    if (m_position_animation != NULL) {
        float time = m_position_animation_time;
        pos = *m_position_animation->m_nodes[0]->GetMatrix(time);
        mtx = &pos;
    }

    if (animation != NULL) {
        if (spm->m_flags & eSpmFileClusterModel) {
            spm->CalculateClusterMatrixAnimation(this, *mtx);

            NaVECTOR<float, 4> v;
            const NaVECTOR<float, 4> scale(1.0f, 1.0f, 1.0f, 0.0f);
            NaMATRIX<float, 4, 4>& root = spm->m_nodes[0]->m_worldMatrix;

            NaVECTOR<float, 4> tmp;
            v = NaMATRIX<float, 4, 4>::Apply(tmp, root, scale);

            float disturbance = m_disturbance;
            float len = 0.0f;
            for (int i = 0; i < 4; i++) {
                len += v[i] * v[i];
            }

            this->m_scaledDisturbance = disturbance * sqrtf(len);
        } else {
            spm->CalculateCurrentMatrixAnimation(this, *mtx);
        }
    } else if (spm->m_flags & eSpmFileClusterModel) {
        spm->CalculateClusterMatrix(this, *mtx);
    } else {
        spm->CalculateCurrentMatrix(this, *mtx);
    }
}

void SpmFileHeader::CalculateCurrentMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& parentMatrix) {
    m_nodes[0]->ComposeGlobalMatrix(model, parentMatrix);

    for (u_int i = 1; i < m_node_num; i++) {
        SpmNode *node = m_nodes[i];
        node->ComposeGlobalMatrix(model, node->m_parent->m_worldMatrix);
    }
}

// Inline: the original image has no out-of-line copy, only the bodies inlined
// into the two *MatrixAnimation functions below.
inline void SpmNode::ComposeAnimatedMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& parentMatrix) {
    PrSPRAM_DATA *spram = prSpramData;
    const float time = spram->m_animation_time;
    SpaFileHeader *animation = spram->m_animation;
    const bool parentVisible = m_parent == NULL || (m_parent->m_flags & eSpmVisible);
    if (!parentVisible || !animation->IsNodeVisible(this, time)) {
        m_flags &= ~eSpmVisible;
        return;
    }
    m_flags |= eSpmVisible;

    SpaNodeAnimation *nodeAnimation = animation->m_nodes[m_animationIndex];
    if (nodeAnimation == NULL) {
        ComposeGlobalMatrixWithoutVisibility(model, parentMatrix);
        return;
    }

    const bool identity = nodeAnimation->m_transformCount == 0;
    if (model->m_postureMatrices[0] != NULL) {
        if (identity) {
            spram->m_nodeMatrix = NaMATRIX<float, 4, 4>::IDENT;
        } else {
            const NaMATRIX<float, 4, 4> *local = nodeAnimation->GetMatrix(time);
            spram = prSpramData;
            spram->m_nodeMatrix = *local;
        }
        if (spram->m_model_transaction_blend_ratio != 1.0f) {
            BlendTransitionMatrix(model, spram->m_nodeMatrix);
            spram = prSpramData;
        }
        m_worldMatrix = parentMatrix * spram->m_nodeMatrix;
        model->m_postureMatrices[model->m_active_transition][m_animationIndex] = spram->m_nodeMatrix;
    } else if (identity) {
        m_worldMatrix = parentMatrix;
    } else {
        const NaMATRIX<float, 4, 4> local = *nodeAnimation->GetMatrix(time);
        m_worldMatrix = parentMatrix * local;
    }
    if (m_flags & eSpmBillboard) ApplyBillboardMatrix();
}

void SpmFileHeader::CalculateCurrentMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& parentMatrix) {
    m_nodes[0]->ComposeAnimatedMatrix(model, parentMatrix);
    for (u_int i = 1; i < m_node_num; i++) {
        SpmNode *node = m_nodes[i];
        node->ComposeAnimatedMatrix(model, node->m_parent->m_worldMatrix);
    }
}

void SpmFileHeader::CalculateClusterMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& parentMatrix) {
    SpmNode *node = m_nodes[0];
    node->ComposeGlobalMatrix(model, parentMatrix);
    if (node->m_flags & eSpmSkinned) {
        const NaMATRIX<float, 4, 4>& b = node->m_bindCorrectionMatrix;
        node->m_skinningMatrix = node->m_worldMatrix * b;
    }

    for (u_int i = 1; i < m_node_num; i++) {
        node = m_nodes[i];
        node->ComposeGlobalMatrix(model, node->m_parent->m_worldMatrix);
        if (node->m_flags & eSpmSkinned) {
            const NaMATRIX<float, 4, 4>& b = node->m_bindCorrectionMatrix;
            node->m_skinningMatrix = node->m_worldMatrix * b;
        }
    }
}

void SpmFileHeader::CalculateClusterMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& parentMatrix) {
    SpmNode *node = m_nodes[0];
    node->ComposeAnimatedMatrix(model, parentMatrix);
    if (node->m_flags & eSpmSkinned) {
        const NaMATRIX<float, 4, 4>& b = node->m_bindCorrectionMatrix;
        node->m_skinningMatrix = node->m_worldMatrix * b;
    }

    for (u_int i = 1; i < m_node_num; i++) {
        node = m_nodes[i];
        node->ComposeAnimatedMatrix(model, node->m_parent->m_worldMatrix);
        if (node->m_flags & eSpmSkinned) {
            const NaMATRIX<float, 4, 4>& b = node->m_bindCorrectionMatrix;
            node->m_skinningMatrix = node->m_worldMatrix * b;
        }
    }
}

void PrModelObject::RenderContext1Model() {
    m_spm_image->RenderContext1Model(this);
    m_rendered_once = 1;
}

void SpmFileHeader::RenderContext1Model(PrModelObject *model) {
    if (m_flags & eSpmFileNoContext1Nodes) {
        return;
    }

    for (u_int i = 0; i < m_node_num; i++) {
        m_nodes[i]->RenderContext1Node(model);
    }
}

void SpmNode::ModifySimpleDmaPacket(PrVuNodeHeaderDmaPacket *packet) {
    PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
    uc->m_matrix = this->m_worldMatrix;
    uc->m_disturbance = prSpramData->m_disturbance;

    float du = this->m_textureScrollU;
    float dv = this->m_textureScrollV;

    if (du != 0.0f) {
        uc->m_textureOffsetU += du;
        if (uc->m_textureOffsetU > 1.0f) {
            uc->m_textureOffsetU -= 1.0f;
        } else if (uc->m_textureOffsetU < 0.0f) {
            uc->m_textureOffsetU += 1.0f;
        }
    }

    if (dv != 0.0f) {
        uc->m_textureOffsetV += dv;
        if (uc->m_textureOffsetV > 1.0f) {
            uc->m_textureOffsetV -= 1.0f;
        } else if (uc->m_textureOffsetV < 0.0f) {
            uc->m_textureOffsetV += 1.0f;
        }
    }
}

void SpmNode::RenderContext1Node(PrModelObject *model) {
    prRenderStuff.m_statistics.node_num++;

    if (this->m_flags & eSpmContext2) {
        return;
    }

    if ((this->m_flags & eSpmVisible) && (!AwfulStatus || (this->m_flags & eSpmDrawnInAwful))) {
        PrVuNodeHeaderDmaPacket *packet = this->m_context1Packets[0];
        if (packet != NULL) {
            prRenderStuff.m_statistics.opaque_context1_node_num++;
            ModifySimpleDmaPacket(packet);
            prRenderStuff.AppendDmaTag(&packet->m_tag);
        }

        packet = this->m_context1Packets[1];
        if (packet != NULL) {
            prRenderStuff.m_statistics.transmit_context1_node_num++;
            ModifySimpleDmaPacket(packet);

            float sp0[4];

            asm volatile(
                "lqc2     vf13,   0x0(%0)       \n\t"
                "lqc2     vf14,  0x10(%0)       \n\t"
                "lqc2     vf15,  0x20(%0)       \n\t"
                "lqc2     vf16,  0x30(%0)       \n\t"
            : : "r"(&this->m_worldMatrix));

            asm volatile(
                "lqc2     vf04,  0x0(%0)        \n\t"
                "vmulax   ACC,   vf13,    vf04  \n\t"
                "vmadday  ACC,   vf14,    vf04  \n\t"
                "vmaddaz  ACC,   vf15,    vf04  \n\t"
                "vmaddw   vf17,  vf16,    vf04  \n\t"
            : : "r"(&this->m_sortPosition));

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

            prRenderStuff.AppendTransmitDmaTag(&packet->m_tag, this->m_sortGroup, f12);
        }

        if (this->m_flags & eSpmContourNode) {
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

    if (!(m_flags & eSpmVisible)) {
        return;
    }

    PrVuNodeHeaderDmaPacket *packet = this->m_context1Packets[0];
    if (packet != NULL) {
        prRenderStuff.m_statistics.opaque_context1_node_num++;
        PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
        uc->m_matrix = this->m_worldMatrix;
        prRenderStuff.AppendDmaTag((sceDmaTag*)PR_DECACHE(uc));
    }

    PrVuNodeHeaderDmaPacket *packet2 = this->m_context1Packets[1];
    if (packet2 != NULL) {
        prRenderStuff.m_statistics.transmit_context1_node_num++;
        PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet2);
        uc->m_matrix = this->m_worldMatrix;

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
        : : "r"(&tmp), "r"(&this->m_worldMatrix), "r"(&this->m_sortPosition) : "memory");

        pos = tmp;

        prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)PR_DECACHE(uc), this->m_sortGroup, -pos[2]);
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

    if ((m_flags & eSpmVisible) && (!AwfulStatus || (m_flags & eSpmDrawnInAwful))) {
        for (u_int i = 0; i < 2; i++) {
            PrVuNodeHeaderDmaPacket *packet = this->m_context1Packets[i];
            if (packet != NULL) {
                packet = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
                packet->m_matrix = this->m_worldMatrix;
                prRenderStuff.AppendDmaTag((sceDmaTag*)PR_DECACHE(packet));

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
    if (m_flags & eSpmFileNoContext2Nodes) {
        return;
    }

    for (u_int i = 0; i < m_node_num; i++) {
        m_nodes[i]->RenderContext2Node(model);
    }
}

void SpmNode::RenderContext2Node(PrModelObject *model) {
    if (!(this->m_flags & eSpmContext2)) {
        return;
    }

    if ((this->m_flags & eSpmVisible) && (!AwfulStatus || (this->m_flags & eSpmDrawnInAwful))) {
        prRenderStuff.m_statistics.opaque_context2_node_num++;

        if (this->m_flags & eSpmClusterPayload) {
            SpmClusterGeometryNode *cluster = reinterpret_cast<SpmClusterGeometryNode*>(this);
            cluster->RenderClusterNode(model);
        } else if (this->m_flags & eSpmShapePayload) {
            SpmShapeNode *shape = static_cast<SpmShapeNode*>(this);
            shape->RenderShapeNode(model);
        } else {
            PrVuNodeHeaderDmaPacket *packet = this->m_context1Packets[0];
            if (packet != NULL) {
                PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
                uc->m_matrix = this->m_worldMatrix;
                uc->m_disturbance = prSpramData->m_disturbance;
                prRenderStuff.RenderNodeEECore((PrVuNodeHeaderDmaPacket*)PR_DECACHE(uc));
            }

            PrVuNodeHeaderDmaPacket *uc = this->m_context1Packets[1];
            if (uc != NULL) {
                uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(uc);
                const NaMATRIX<float, 4, 4>& m = this->m_worldMatrix;
                uc->m_matrix = m;
                uc->m_disturbance = prSpramData->m_disturbance;

                NaVECTOR<float, 4> pos;
                {
                    NaMATRIX<float, 4, 4> mtx = prSpramData->m_view_projection_matrix * m;
                    NaVECTOR<float, 4> tmp;
                    pos = NaMATRIX<float, 4, 4>::Apply(tmp, mtx, this->m_sortPosition);
                }

                float z = pos[2] / pos[3];
                u_int key = this->m_sortGroup;
                if (pos[3] == 0.0f) {
                    z = pos[2] * 3.40282347e+38f;
                }

                prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)PR_DECACHE(uc), key, z);
            }
        }

        if (this->m_flags & eSpmContourNode) {
            SpmComplexNode *complex = static_cast<SpmComplexNode*>(this);
            complex->RenderContour(model);
        }
    }
}

/* prlib/render.cpp */
void SpmNode::ComposeGlobalMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& parentMatrix) {
    SpmNode *parent = this->m_parent;

    if (parent != NULL && !(parent->m_flags & eSpmVisible)) {
        m_flags &= ~eSpmVisible;
    } else {
        bool visible = !(m_flags & eSpmDefaultHidden);
        if (visible) {
            m_flags |= eSpmVisible;
        } else {
            m_flags &= ~eSpmVisible;
        }
    }

    if (m_flags & eSpmVisible) {
        ComposeGlobalMatrixWithoutVisibility(model, parentMatrix);
    }
}

/* prlib/render.cpp */
void SpmNode::ComposeGlobalMatrixWithoutVisibility(PrModelObject *model, const NaMATRIX<float, 4, 4>& parentMatrix) {
    if (model->m_postureMatrices[0] != NULL) {
        PrSPRAM_DATA *spram;
        if (m_flags & eSpmIdentityLocalMatrix) {
            spram = prSpramData;
            spram->m_nodeMatrix = NaMATRIX<float, 4, 4>::IDENT;
        } else {
            spram = prSpramData;
            spram->m_nodeMatrix = this->m_localMatrix;
        }

        if (spram->m_model_transaction_blend_ratio != 1.0f) {
            BlendTransitionMatrix(model, spram->m_nodeMatrix);
            spram = prSpramData;
        }

        this->m_worldMatrix = parentMatrix * spram->m_nodeMatrix;
        int idx = this->m_animationIndex;
        model->m_postureMatrices[model->m_active_transition][idx] = spram->m_nodeMatrix;
    } else if (m_flags & eSpmIdentityLocalMatrix) {
        this->m_worldMatrix = parentMatrix;
    } else {
        this->m_worldMatrix = parentMatrix * this->m_localMatrix;
    }

    if (m_flags & eSpmBillboard) {
        ApplyBillboardMatrix();
    }
}
