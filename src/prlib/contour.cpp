#include "model.h"
#include "renderstuff.h"
#include "spram.h"

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)EE_SCRATCHPAD_BASE;
static float contourMaxDepth = 3.4028235e38f;

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

void SpmComplexNode::SaveContour(PrModelObject *model) {
    PrVuNodeHeaderDmaPacket *source = m_geometryPacket;
    const NaMATRIX<float, 4, 4> &matrix = source->m_matrix;
    const bool identity = matrix.inl0();

    // SaveContour writes history after two quadwords. RenderContour uses the
    // same mapping against the packet base; these distinct conventions come
    // from the original routines and must not be silently unified.
    const u_int historyPrefixQuadwords = 2;
    for (u_int i = 0; i < m_contourCount; i++) {
        const SpmContourIndex &mapping = m_contourIndices[i];
        const NaVECTOR<float, 4> &position = source->PositionAtQuadword(mapping.m_src);
        NaVECTOR<float, 4> &saved = m_contourPacket->PositionAtQuadword(historyPrefixQuadwords + mapping.m_dst);
        if (identity) {
            saved = position;
        } else {
            NaMATRIX<float, 4, 4>::Apply(saved, matrix, position);
        }
    }
}

void PrModelObject::ResetContour() {
    m_flags &= ~2;
}

void SpmComplexNode::RenderContour(PrModelObject *model) {
    if (!(model->m_flags & 2) ||
        (model->m_contour_blur_alpha[0] == 0.0f && model->m_contour_blur_alpha[1] == 0.0f)) {
        return;
    }

    PrVuNodeHeaderDmaPacket *source = m_geometryPacket;
    const NaMATRIX<float, 4, 4> &matrix = source->m_matrix;
    const bool identity = matrix.inl0();
    for (u_int i = 0; i < m_contourCount; i++) {
        const SpmContourIndex &mapping = m_contourIndices[i];
        const NaVECTOR<float, 4> &position = source->PositionAtQuadword(mapping.m_src);
        NaVECTOR<float, 4> &destination = m_contourPacket->PositionAtQuadword(mapping.m_dst);
        if (identity) {
            destination = position;
        } else {
            NaMATRIX<float, 4, 4>::Apply(destination, matrix, position);
        }
    }

    m_contourPacket->m_contour_blur_alpha[0] = model->m_contour_blur_alpha[0];
    m_contourPacket->m_contour_blur_alpha[1] = model->m_contour_blur_alpha[1];

    const NaMATRIX<float, 4, 4> projected = prSpramData->m_view_projection_matrix * matrix;
    NaVECTOR<float, 4> position;
    NaMATRIX<float, 4, 4>::Apply(position, projected, m_sortPosition);
    const float depth = position[3] == 0.0f ? position[2] * contourMaxDepth : position[2] / position[3];
    prRenderStuff.AppendTransmitDmaTag(&m_contourPacket->m_tag, -1, depth);
}
