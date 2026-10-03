#include "model.h"
#include "spram.h"

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)0x70000000;

void PrModelObject::SavePosture() {
    if (!(m_spm_image->m_flags & 0x40)) {
        return;
    }

    if (m_rendered_once) {
        m_active_transition = 1 - m_active_transition;
        m_flags |= 4;
    }
}

void PrModelObject::ResetPosture() {
    m_flags &= ~4;
}

void SpmNode::BlendTransitionMatrix(PrModelObject *model, NaMATRIX<float, 4, 4>& mtx) {
    float ratio = prSpramData->m_model_transaction_blend_ratio;

    if (!(model->m_flags & 0x4)) {
        return;
    }

    NaMATRIX<float, 4, 4>& posture = model->m_postureMatrices[1 - model->m_active_transition][this->m_animationIndex];
    mtx = posture * (1.0f - ratio) + mtx * ratio;
}

float SpmShapeNode::BlendTransactionWeight(PrModelObject *model, float weight, u_int index) {
    float ratio = prSpramData->m_model_transaction_blend_ratio;

    if (!(model->m_flags & 0x4)) {
        return weight;
    }

    float *posture = model->m_postureWeights[1 - model->m_active_transition];
    return (1.0f - ratio) * posture[m_shape.postureWeightOffset + index] + ratio * weight;
}
