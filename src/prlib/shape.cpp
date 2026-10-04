#include "common.h"

#include "model.h"
#include "animation.h"
#include "renderstuff.h"
#include "spadata.h"
#include "spram.h"

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)EE_SCRATCHPAD_BASE;
static float shapeMaxDepth = 3.4028235e38f;

void SpmShapeNode::AddShapePosition(u_int shapeIndex, float weight) {
    PrVuNodeHeaderDmaPacket *vertices = reinterpret_cast<PrVuNodeHeaderDmaPacket*>(PR_UNCACHEDACCEL(m_geometryPacket));
    u_int num = this->m_deformPositionCount;
    NaVECTOR<float, 4> *src = &m_shapeDeltas[shapeIndex];
    u_int stride = m_shape.stride;
    const SpmPositionTargets *targets = m_positionTargets;

    for (u_int i = 0; i < num; i++) {
        asm volatile("lqc2 $vf17, 0x0(%0)" : : "r"(src) : "memory");
        const u_int targetCount = targets->count;
        src += stride;
        asm volatile("
            qmtc2.ni %0, $vf4
            vmulx.xyz $vf17, $vf17, $vf4x
        " : : "r"(weight));

        for (u_int j = 0; j < targetCount; j++) {
            NaVECTOR<float, 4> *v = &vertices->PositionAtQuadword(targets->quadwordIndices[j]);
            asm volatile("
                lqc2 $vf4, 0x0(%0)
                vadd.xyz $vf4, $vf4, $vf17
                sqc2 $vf4, 0x0(%0)
            " : : "r"(v) : "memory");
        }
        targets = targets->Next();
    }
}

void SpmShapeNode::RenderShapeNode(PrModelObject *model) {
    NaVECTOR<float, 4> result;

    SpaFileHeader *animation = prSpramData->m_animation;
    SpaNodeAnimation *node_anim = (animation != NULL) ? animation->m_nodes[this->m_animationIndex] : NULL;
    u_int track_num = (node_anim != NULL) ? node_anim->m_shapeWeightTrackCount : 0;

    PrVuNodeHeaderDmaPacket *vertices = reinterpret_cast<PrVuNodeHeaderDmaPacket*>(PR_UNCACHEDACCEL(m_geometryPacket));
    u_int num = this->m_deformPositionCount;
    NaVECTOR<float, 4> *src = m_shape.basePositions;
    const SpmPositionTargets *targets = m_positionTargets;

    for (u_int i = 0; i < num; i++) {
        asm volatile("lqc2 $vf17, 0x0(%0)" : : "r"(src) : "memory");
        const u_int targetCount = targets->count;
        src++;

        for (u_int j = 0; j < targetCount; j++) {
            NaVECTOR<float, 4> *v = &vertices->PositionAtQuadword(targets->quadwordIndices[j]);
            asm volatile("sqc2 $vf17, 0x0(%0)" : : "r"(v) : "memory");
        }
        targets = targets->Next();
    }

    if (model->m_postureMatrices[0] != NULL) {
        for (u_int i = 0; i < track_num; i++) {
            float weight = *node_anim->m_shapeWeightTracks[i]->GetValue(prSpramData->m_animation_time);
            if (prSpramData->m_model_transaction_blend_ratio != 1.0f) {
                weight = BlendTransactionWeight(model, weight, i);
            }
            if (weight != 0.0f) {
                AddShapePosition(i, weight);
            }
            model->m_postureWeights[model->m_active_transition][m_shape.postureWeightOffset + i] = weight;
        }
    } else {
        for (u_int i = 0; i < track_num; i++) {
            float weight = *node_anim->m_shapeWeightTracks[i]->GetValue(prSpramData->m_animation_time);
            if (weight != 0.0f) {
                AddShapePosition(i, weight);
            }
        }
    }

    if (this->m_context1Packets[0] != NULL) {
        PrVuNodeHeaderDmaPacket *packet = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(this->m_context1Packets[0]);
        packet->m_matrix = this->m_worldMatrix;
        packet->m_disturbance = prSpramData->m_disturbance;
        prRenderStuff.RenderNodeEECore((PrVuNodeHeaderDmaPacket*)PR_DECACHE(packet));
    }

    if (this->m_context1Packets[1] != NULL) {
        PrVuNodeHeaderDmaPacket *packet = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(this->m_context1Packets[1]);
        packet->m_matrix = this->m_worldMatrix;
        packet->m_disturbance = prSpramData->m_disturbance;
        u_int arg = this->m_sortGroup;

        NaMATRIX<float, 4, 4> m = prSpramData->m_view_projection_matrix * this->m_worldMatrix;
        result = m * this->m_sortPosition;

        float depth = result[3] == 0.0f ? result[2] * shapeMaxDepth : result[2] / result[3];

        prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)PR_DECACHE(packet), arg, depth);
    }
}
