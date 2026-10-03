#include "model.h"
#include "renderstuff.h"
#include "spram.h"

/* sdata */
extern PrSPRAM_DATA *prSpramData_tmp_contour;
extern float contour_max_depth[];

void PrModelObject::SaveContour() {
    SpmFileHeader *spm = m_spm_image;
    if (spm->unk70 == 0 || !m_rendered_once) {
        return;
    }

    u_int node_num = spm->m_node_num;
    for (int i = 0; i < node_num; i++) {
        SpmNode *node = spm->m_nodes[i];
        if (node->m_flags & 0x40) {
            SpmComplexNode *complex = static_cast<SpmComplexNode*>(node);
            complex->SaveContour(this);
        }
    }

    m_flags |= 2;
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/contour", SaveContour__14SpmComplexNodeP13PrModelObject);
#else
/*
 * Two addu operands are swapped: the original adds the scaled index to the
 * packet address as integers ((m_src << 4) + (u_int)src), while indexing the
 * vector array puts the base first.
 */
void SpmComplexNode::SaveContour(PrModelObject *model) {
    PrVuNodeHeaderDmaPacket *packet = this->unk17C;
    NaMATRIX<float, 4, 4> *matrix = &packet->m_matrix;
    NaVECTOR<float, 4> *src = reinterpret_cast<NaVECTOR<float, 4>*>(packet);

    bool ident = matrix->inl0();
    NaVECTOR<float, 4> *dst = reinterpret_cast<NaVECTOR<float, 4>*>(m_contour_packet) + 2;
    u_int num = m_contour_index_num;
    if (ident) {
        for (u_int i = 0; i < num; i++) {
            NaVECTOR<float, 4> *s = &src[m_contour_index[i].m_src];
            dst[m_contour_index[i].m_dst] = *s;
        }
    } else {
        for (u_int i = 0; i < num; i++) {
            NaVECTOR<float, 4> result;
            NaVECTOR<float, 4> tmp;
            NaVECTOR<float, 4> *v = &src[m_contour_index[i].m_src];
            u_int d = m_contour_index[i].m_dst;

            asm volatile(
                "lqc2         $vf4,   0x0(%0)            \n\t"
                "lqc2         $vf5,  0x10(%0)            \n\t"
                "lqc2         $vf6,  0x20(%0)            \n\t"
                "lqc2         $vf7,  0x30(%0)            \n\t"
                "lqc2         $vf8,   0x0(%1)            \n\t"
                "vmulax.xyzw  ACC,    $vf4,   $vf8x      \n\t"
                "vmadday.xyzw ACC,    $vf5,   $vf8y      \n\t"
                "vmaddaz.xyzw ACC,    $vf6,   $vf8z      \n\t"
                "vmaddw.xyzw  $vf9,   $vf7,   $vf8w      \n\t"
                "sqc2         $vf9,   0x0(%2)            \n\t"
            : : "r"(matrix), "r"(v), "r"(&tmp));

            result = tmp;
            dst[d] = result;
        }
    }
}
#endif

void PrModelObject::ResetContour() {
    m_flags &= ~2;
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/contour", RenderContour__14SpmComplexNodeP13PrModelObject);
#else
/* Register allocation: the product/copy temporaries land in t3/t4 instead of t8/t9 */
void SpmComplexNode::RenderContour(PrModelObject *model) {
    NaVECTOR<float, 4> result;
    NaVECTOR<float, 4> tmp;

    if (!(model->m_flags & 2)) {
        return;
    }

    if (model->m_contour_blur_alpha[0] == 0.0f && model->m_contour_blur_alpha[1] == 0.0f) {
        return;
    }

    PrVuNodeHeaderDmaPacket *packet = this->unk17C;
    NaMATRIX<float, 4, 4> *matrix = &packet->m_matrix;
    NaVECTOR<float, 4> *src = reinterpret_cast<NaVECTOR<float, 4>*>(packet);

    bool ident = matrix->inl0();
    NaVECTOR<float, 4> *dst = reinterpret_cast<NaVECTOR<float, 4>*>(m_contour_packet);
    u_int num = m_contour_index_num;
    if (ident) {
        for (u_int i = 0; i < num; i++) {
            NaVECTOR<float, 4> *s = &src[m_contour_index[i].m_src];
            dst[m_contour_index[i].m_dst] = *s;
        }
    } else {
        for (u_int i = 0; i < num; i++) {
            NaVECTOR<float, 4> *v = &src[m_contour_index[i].m_src];
            u_int d = m_contour_index[i].m_dst;

            asm volatile(
                "lqc2         $vf4,   0x0(%0)            \n\t"
                "lqc2         $vf5,  0x10(%0)            \n\t"
                "lqc2         $vf6,  0x20(%0)            \n\t"
                "lqc2         $vf7,  0x30(%0)            \n\t"
                "lqc2         $vf8,   0x0(%1)            \n\t"
                "vmulax.xyzw  ACC,    $vf4,   $vf8x      \n\t"
                "vmadday.xyzw ACC,    $vf5,   $vf8y      \n\t"
                "vmaddaz.xyzw ACC,    $vf6,   $vf8z      \n\t"
                "vmaddw.xyzw  $vf9,   $vf7,   $vf8w      \n\t"
                "sqc2         $vf9,   0x0(%2)            \n\t"
            : : "r"(matrix), "r"(v), "r"(&tmp));

            result = tmp;
            dst[d] = result;
        }
    }

    m_contour_packet->m_contour_blur_alpha[0] = model->m_contour_blur_alpha[0];
    m_contour_packet->m_contour_blur_alpha[1] = model->m_contour_blur_alpha[1];

    NaMATRIX<float, 4, 4> m = prSpramData_tmp_contour->m_view_projection_matrix * *matrix;
    result = NaMATRIX<float, 4, 4>::Apply(tmp, m, this->unk140);

    float depth = result[2] / result[3];
    if (result[3] == 0.0f) {
        depth = result[2] * contour_max_depth[0];
    }

    prRenderStuff.AppendTransmitDmaTag(&m_contour_packet->m_tag, -1, depth);
}
#endif
