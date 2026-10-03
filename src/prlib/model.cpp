#include "model.h"
#include "animation.h"
#include "spadata.h"
#include "spram.h"
#include "scene.h"

#include <nalib/navector.h>

/* sdata */
extern PrSPRAM_DATA *prSpramData_tmp_model;
extern u_char *workAreaTopAddress;
extern u_int workAreaSize;

PR_EXTERN
void PrSetPostureWorkArea(void *addr, u_int size) {
    u_char *p = (u_char*)addr;

    while ((u_int)p & 0xF) {
        p++;
        size--;
    }

    workAreaTopAddress = p;
    workAreaSize = size;
}

/* static */ void* AllocateFromWorkArea(u_int size) {
    if (workAreaTopAddress == NULL) {
        return NULL;
    }

    size = (size + 15) / 16 * 16;
    if (workAreaSize < size) {
        return NULL;
    }

    void *ret = workAreaTopAddress;
    workAreaSize -= size;
    workAreaTopAddress += size;
    return ret;
}

PrModelObject::PrModelObject(SpmFileHeader *spm) {
    m_list.next = NULL;
    m_list.prev = NULL;
    m_obj_set = NULL;
    m_linked_scene = NULL;

    unk10 = NaMATRIX<float, 4, 4>::IDENT;

    unk50 = 0x55668899;
    m_user_data = NULL;
    m_spm_image = spm;
    m_flags = 0;

    m_animation_time = 0.0f;
    m_position_animation_time = 0.0f;
    m_animation = NULL;
    m_position_animation = NULL;

    m_postureMatrices[0] = NULL;
    m_postureMatrices[1] = NULL;
    m_postureWeights[0] = NULL;
    m_postureWeights[1] = NULL;
    m_rendered_once = 0;
    m_active_transition = 0;

    unk88 = NULL;
    unk8C = NULL;
    unk90 = 0;
    m_contour_blur_alpha[0] = 0.0f;
    m_contour_blur_alpha[1] = 0.0f;
    m_transaction_blend_ratio = 1.0f;
    m_disturbance = 0.0f;
    unkA4 = 1.0f;

    spm->unk50 = this;
}

PrModelObject::~PrModelObject() {
    m_linked_scene->m_model_set.Remove(this);
    m_linked_scene = NULL;

    CleanupAnimation();
    CleanupPositionAnimation();

    m_spm_image->unk50 = NULL;

    delete unk88;
    delete unk8C;

    if ((m_flags & 0x8) && m_postureWeights[0] != NULL) {
        delete[] m_postureWeights[0];
    }

    if ((m_flags & 0x10) && m_postureMatrices[0] != NULL) {
        delete[] m_postureMatrices[0];
    }
}

void PrModelObject::Initialize() {
    SpmFileHeader *spm = m_spm_image;
    spm->ChangePointer();

    if (spm->m_flags & 0x40) {
        u_int node_num = spm->m_node_num;
        if (node_num != 0) {
            NaMATRIX<float, 4, 4> *matrix = (NaMATRIX<float, 4, 4>*)AllocateFromWorkArea(node_num * sizeof(NaMATRIX<float, 4, 4>) * 2);
            if (matrix == NULL) {
                m_postureMatrices[0] = new NaMATRIX<float, 4, 4>[node_num * 2];
                m_flags |= 0x10;
            } else {
                m_postureMatrices[0] = matrix;
            }
            m_postureMatrices[1] = m_postureMatrices[0] + node_num;

            u_int weight_num = spm->unk6C;
            if (weight_num != 0) {
                float *weight = (float*)AllocateFromWorkArea(weight_num * sizeof(float) * 2);
                if (weight == NULL) {
                    m_postureWeights[0] = new float[weight_num * 2];
                    m_flags |= 0x8;
                } else {
                    m_postureWeights[0] = weight;
                }
                m_postureWeights[1] = m_postureWeights[0] + weight_num;
            }
        }
    }

    m_flags &= ~0x4;
}

void PrModelObject::LinkAnimation(SpaFileHeader *animation) {
    if (animation != NULL) {
        m_animation = animation;
        m_animation_time = 0.0f;
    } else {
        CleanupAnimation();
    }
}

void PrModelObject::CleanupAnimation() {
    m_animation = NULL;
}

void PrModelObject::LinkPositionAnimation(SpaFileHeader *animation) {
    if (animation != NULL) {
        m_position_animation = animation;
        m_position_animation_time = 0.0f;
    } else {
        CleanupPositionAnimation();
    }
}

void PrModelObject::CleanupPositionAnimation() {
    m_position_animation = NULL;
}

void PrModelObject::UnionBoundaryBox(NaVECTOR<float, 4> *arg0, NaVECTOR<float, 4> *arg1) {
    asm volatile(
        "lqc2       $vf13,   0x0(%0)       \n\t"
        "lqc2       $vf14,  0x10(%0)       \n\t"
        "lqc2       $vf15,  0x20(%0)       \n\t"
        "lqc2       $vf16,  0x30(%0)       \n\t"
    : : "r"(&this->unk10));

    asm volatile(
        "lqc2       $vf17,  0x0(%0)        \n\t"
        "vmulax     ACC,    $vf13,  $vf17  \n\t"
        "vmadday    ACC,    $vf14,  $vf17  \n\t"
        "vmaddaz    ACC,    $vf15,  $vf17  \n\t"
        "vmaddw     $vf17,  $vf16,  $vf17  \n\t"
    : : "r"(&m_spm_image->unk30));

    asm volatile(
        "lqc2       $vf04,  0x0(%0)        \n\t"
        "vmini.xyz  $vf04,  $vf04,  $vf17  \n\t"
        "sqc2       $vf04,  0x0(%0)        \n\t"
    : : "r"(arg0));

    asm volatile(
        "lqc2       $vf17,  0x0(%0)        \n\t"
        "vmulax     ACC,    $vf13,  $vf17  \n\t"
        "vmadday    ACC,    $vf14,  $vf17  \n\t"
        "vmaddaz    ACC,    $vf15,  $vf17  \n\t"
        "vmaddw     $vf17,  $vf16,  $vf17  \n\t"
    : : "r"(&m_spm_image->unk40));

    asm volatile(
        "lqc2       $vf04,  0x0(%0)        \n\t"
        "vmax.xyz   $vf04,  $vf04,  $vf17  \n\t"
        "sqc2       $vf04,  0x0(%0)        \n\t"
    : : "r"(arg1));
}

void PrModelObject::GetPrimitivePosition(NaVECTOR<float, 4> *position) {
    position->Set(0.0f, 0.0f, 0.0f, 1.0f);

    if (m_position_animation != NULL) {
        NaMATRIX<float, 4, 4>::Apply(*position, *m_position_animation->m_nodes[0]->GetMatrix(m_position_animation_time), *position);
    } else {
        NaMATRIX<float, 4, 4>::Apply(*position, unk10, *position);
    }

    NaMATRIX<float, 4, 4>::Apply(*position, prSpramData_tmp_model->m_view_projection_matrix, *position);
}

void PrModelObject::GetScreenPosition(NaVECTOR<float, 4> *position) {
    GetPrimitivePosition(position);

    float w = (*position)[3];
    (*position)[0] = (*position)[0] / w - 1728.0f;
    (*position)[1] = (*position)[1] / w - 1936.0f;
    (*position)[3] = 1.0f;
}
