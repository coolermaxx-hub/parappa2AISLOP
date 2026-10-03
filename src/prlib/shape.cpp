#include "common.h"

#include "model.h"
#include "animation.h"
#include "renderstuff.h"
#include "spadata.h"
#include "spram.h"

/* sdata */
extern PrSPRAM_DATA *prSpramData_tmp_shape;
extern float shape_max_depth[];

void SpmShapeNode::AddShapePosition(u_int target, float weight) {
    u_int vertex = (u_int)this->m_main_packet;
    vertex |= 0x30000000;
    u_int num = m_vertex_num;
    u_long128 *src = &m_target_offsets[target];
    u_int stride = m_target_num;
    u_int *index = m_vertex_index;

    for (u_int i = 0; i < num; i++) {
        asm volatile("lqc2 $vf17, 0x0(%0)" : : "r"(src));
        u_int n = *index++;
        src += stride;
        asm volatile("
            qmtc2.ni %0, $vf4
            vmulx.xyz $vf17, $vf17, $vf4x
        " : : "r"(weight));

        for (u_int j = 0; j < n; j++) {
            u_long128 *v = (u_long128 *)((*index++ << 4) + vertex);
            asm volatile("
                lqc2 $vf4, 0x0(%0)
                vadd.xyz $vf4, $vf4, $vf17
                sqc2 $vf4, 0x0(%0)
            " : : "r"(v));
        }
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/shape", RenderShapeNode__12SpmShapeNodeP13PrModelObject);
#else
/* Register allocation; the result also takes one extra copy through a temporary */
void SpmShapeNode::RenderShapeNode(PrModelObject *model) {
    NaVECTOR<float, 4> result;

    SpaFileHeader *animation = prSpramData_tmp_shape->m_animation;
    SpaNodeAnimation *node_anim = (animation != NULL) ? animation->unk50[this->m_index] : NULL;
    u_int track_num = (node_anim != NULL) ? node_anim->unk2C : 0;

    u_int vertex = (u_int)this->m_main_packet;
    vertex |= 0x30000000;
    u_int num = m_vertex_num;
    u_long128 *src = m_base_vertices;
    u_int *index = m_vertex_index;

    for (u_int i = 0; i < num; i++) {
        asm volatile("lqc2 $vf17, 0x0(%0)" : : "r"(src));
        u_int n = *index++;
        src++;

        for (u_int j = 0; j < n; j++) {
            u_long128 *v = (u_long128 *)((*index++ << 4) + vertex);
            asm volatile("sqc2 $vf17, 0x0(%0)" : : "r"(v));
        }
    }

    if (model->m_posture_matrices[0] != NULL) {
        for (u_int i = 0; i < track_num; i++) {
            float weight = *node_anim->unk30[i]->GetValue(prSpramData_tmp_shape->m_animation_time);
            if (prSpramData_tmp_shape->m_model_transaction_blend_ratio != 1.0f) {
                weight = BlendTransactionWeight(model, weight, i);
            }
            if (weight != 0.0f) {
                AddShapePosition(i, weight);
            }
            model->m_posture_weights[model->m_active_transition][m_weight_index + i] = weight;
        }
    } else {
        for (u_int i = 0; i < track_num; i++) {
            float weight = *node_anim->unk30[i]->GetValue(prSpramData_tmp_shape->m_animation_time);
            if (weight != 0.0f) {
                AddShapePosition(i, weight);
            }
        }
    }

    if (this->m_packets[0] != NULL) {
        PrVuNodeHeaderDmaPacket *packet = (PrVuNodeHeaderDmaPacket*)((u_int)this->m_packets[0] | 0x30000000);
        packet->m_matrix = this->m_global_matrix;
        packet->m_disturbance = prSpramData_tmp_shape->m_disturbance;
        prRenderStuff.RenderNodeEECore((PrVuNodeHeaderDmaPacket*)((u_int)packet & 0x0FFFFFFF));
    }

    if (this->m_packets[1] != NULL) {
        PrVuNodeHeaderDmaPacket *packet = (PrVuNodeHeaderDmaPacket*)((u_int)this->m_packets[1] | 0x30000000);
        packet->m_matrix = this->m_global_matrix;
        packet->m_disturbance = prSpramData_tmp_shape->m_disturbance;
        u_int arg = this->m_draw_group;

        NaMATRIX<float, 4, 4> m = prSpramData_tmp_shape->m_view_projection_matrix * this->m_global_matrix;
        result = m * this->m_center;

        float depth = result[2] / result[3];
        if (result[3] == 0.0f) {
            depth = result[2] * shape_max_depth[0];
        }

        prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)((u_int)packet & 0x0FFFFFFF), arg, depth);
    }
}
#endif
