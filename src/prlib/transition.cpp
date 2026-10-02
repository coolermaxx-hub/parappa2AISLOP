#include "model.h"
#include "spram.h"

/* sdata */
extern PrSPRAM_DATA *prSpramData_tmp_transition;

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

INCLUDE_ASM("asm/nonmatchings/prlib/transition", BlendTransitionMatrix__7SpmNodeP13PrModelObjectRt8NaMATRIX3Zfi4i4);

float SpmShapeNode::BlendTransactionWeight(PrModelObject *model, float weight, u_int index) {
    float ratio = prSpramData_tmp_transition->m_model_transaction_blend_ratio;

    if (!(model->m_flags & 0x4)) {
        return weight;
    }

    float *posture = model->unk74[1 - model->m_active_transition];
    return (1.0f - ratio) * posture[this->unk1B8 + index] + ratio * weight;
}
