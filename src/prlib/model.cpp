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

    unk7C[0] = NULL;
    unk7C[1] = NULL;
    unk74[0] = NULL;
    unk74[1] = NULL;
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

    if ((m_flags & 0x8) && unk74[0] != NULL) {
        delete[] unk74[0];
    }

    if ((m_flags & 0x10) && unk7C[0] != NULL) {
        delete[] unk7C[0];
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
                unk7C[0] = new NaMATRIX<float, 4, 4>[node_num * 2];
                m_flags |= 0x10;
            } else {
                unk7C[0] = matrix;
            }
            unk7C[1] = unk7C[0] + node_num;

            u_int weight_num = spm->unk6C;
            if (weight_num != 0) {
                float *weight = (float*)AllocateFromWorkArea(weight_num * sizeof(float) * 2);
                if (weight == NULL) {
                    unk74[0] = new float[weight_num * 2];
                    m_flags |= 0x8;
                } else {
                    unk74[0] = weight;
                }
                unk74[1] = unk74[0] + weight_num;
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

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/model", GetPrimitivePosition__13PrModelObjectPt8NaVECTOR2Zfi4);
#else
/* Register allocation: this/position swapped (s0/s1) */
void PrModelObject::GetPrimitivePosition(NaVECTOR<float, 4> *position) {
    position->Set(0.0f, 0.0f, 0.0f, 1.0f);

    if (m_position_animation != NULL) {
        NaMATRIX<float, 4, 4> *matrix = m_position_animation->unk50[0]->GetMatrix(m_position_animation_time);
        asm volatile(
            "lqc2       $vf4,   0x0(%0)        \n\t"
            "lqc2       $vf5,  0x10(%0)        \n\t"
            "lqc2       $vf6,  0x20(%0)        \n\t"
            "lqc2       $vf7,  0x30(%0)        \n\t"
            "lqc2       $vf8,   0x0(%1)        \n\t"
            "vmulax     ACC,    $vf4,   $vf8   \n\t"
            "vmadday    ACC,    $vf5,   $vf8   \n\t"
            "vmaddaz    ACC,    $vf6,   $vf8   \n\t"
            "vmaddw     $vf9,   $vf7,   $vf8   \n\t"
            "sqc2       $vf9,   0x0(%1)        \n\t"
        : : "r"(matrix), "r"(position));
    } else {
        asm volatile(
            "lqc2       $vf4,   0x0(%0)        \n\t"
            "lqc2       $vf5,  0x10(%0)        \n\t"
            "lqc2       $vf6,  0x20(%0)        \n\t"
            "lqc2       $vf7,  0x30(%0)        \n\t"
            "lqc2       $vf8,   0x0(%1)        \n\t"
            "vmulax     ACC,    $vf4,   $vf8   \n\t"
            "vmadday    ACC,    $vf5,   $vf8   \n\t"
            "vmaddaz    ACC,    $vf6,   $vf8   \n\t"
            "vmaddw     $vf9,   $vf7,   $vf8   \n\t"
            "sqc2       $vf9,   0x0(%1)        \n\t"
        : : "r"(&unk10), "r"(position));
    }

    asm volatile(
        "lqc2       $vf4,   0x0(%0)        \n\t"
        "lqc2       $vf5,  0x10(%0)        \n\t"
        "lqc2       $vf6,  0x20(%0)        \n\t"
        "lqc2       $vf7,  0x30(%0)        \n\t"
        "lqc2       $vf8,   0x0(%1)        \n\t"
        "vmulax     ACC,    $vf4,   $vf8   \n\t"
        "vmadday    ACC,    $vf5,   $vf8   \n\t"
        "vmaddaz    ACC,    $vf6,   $vf8   \n\t"
        "vmaddw     $vf9,   $vf7,   $vf8   \n\t"
        "sqc2       $vf9,   0x0(%1)        \n\t"
    : : "r"(&prSpramData_tmp_model->m_view_projection_matrix), "r"(position));
}
#endif

void PrModelObject::GetScreenPosition(NaVECTOR<float, 4> *position) {
    GetPrimitivePosition(position);

    float *p = reinterpret_cast<float*>(position);
    float w = p[3];
    p[0] = p[0] / w - 1728.0f;
    p[1] = p[1] / w - 1936.0f;
    p[3] = 1.0f;
}

#ifndef NON_MATCHING
/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/model", func_00140E38);
#endif
