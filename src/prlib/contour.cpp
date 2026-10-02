#include "model.h"

void PrModelObject::SaveContour() {
    SpmFileHeader *spm = m_spm_image;
    if (spm->unk70 == 0 || !m_rendered_once) {
        return;
    }

    u_int node_num = spm->m_node_num;
    for (int i = 0; i < node_num; i++) {
        SpmNode *node = spm->m_nodes[i];
        if (node->m_flags & 0x40) {
            SpmComplexNode *complex = reinterpret_cast<SpmComplexNode*>(node);
            complex->SaveContour(this);
        }
    }

    m_flags |= 2;
}

void SpmComplexNode::SaveContour(PrModelObject *model) {
    PrVuNodeHeaderDmaPacket *packet = this->unk17C;
    NaMATRIX<float, 4, 4> *matrix = &packet->m_matrix;
    NaVECTOR<float, 4> *src = reinterpret_cast<NaVECTOR<float, 4>*>(packet);

    bool ident = matrix->inl0();
    NaVECTOR<float, 4> *dst = reinterpret_cast<NaVECTOR<float, 4>*>((u_int)this->unk1A4 + 0x20);
    u_int num = this->unk19C;
    if (ident) {
        for (u_int i = 0; i < num; i++) {
            NaVECTOR<float, 4> *s = (NaVECTOR<float, 4> *)((this->unk1A0[i].m_src << 4) + (u_int)src);
            dst[this->unk1A0[i].m_dst] = *s;
        }
    } else {
        for (u_int i = 0; i < num; i++) {
            NaVECTOR<float, 4> result;
            NaVECTOR<float, 4> tmp;
            NaVECTOR<float, 4> *v = (NaVECTOR<float, 4> *)((this->unk1A0[i].m_src << 4) + (u_int)src);
            u_int d = this->unk1A0[i].m_dst;

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

void PrModelObject::ResetContour() {
    m_flags &= ~2;
}

INCLUDE_ASM("asm/nonmatchings/prlib/contour", RenderContour__14SpmComplexNodeP13PrModelObject);
