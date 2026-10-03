#include "common.h"

#include "model.h"
#include "spram.h"

#include <nalib/namatrix.h>

#include <math.h>

#ifndef NON_MATCHING
/* This file's weak copies of these are asm at the end of the file */
extern template NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::RotateMatrix(int axis, const float& angle);
extern template NaMATRIX<float, 4, 4>& NaMATRIX<float, 4, 4>::Set(
    const float& m00, const float& m01, const float& m02, const float& m03,
    const float& m10, const float& m11, const float& m12, const float& m13,
    const float& m20, const float& m21, const float& m22, const float& m23,
    const float& m30, const float& m31, const float& m32, const float& m33);
#endif

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)0x70000000;

/* Rotation about Y that turns the model's local Z axis toward the camera */
static NaMATRIX<float, 4, 4>& CreateBillboardMatrix(const NaMATRIX<float, 4, 4>& mtx) {
    NaVECTOR<float, 4> eye;
    eye = prSpramData->m_camera.position;
    eye[3] = 1.0f;

    /* The camera position in the model's local space */
    NaVECTOR<float, 4> v;
    {
        NaMATRIX<float, 4, 4> inv = mtx.Inverse();
        NaVECTOR<float, 4> tmp;
        v = NaMATRIX<float, 4, 4>::Apply(tmp, inv, eye);
    }

    if (v[0] == 0.0f && v[2] == 0.0f) {
        return NaMATRIX<float, 4, 4>::IDENT;
    }

    float angle = atan2f(v[0], v[2]);
    static NaMATRIX<float, 4, 4> billboard;
    billboard = NaMATRIX<float, 4, 4>::RotateMatrix(1, angle);
    return billboard;
}

void SpmNode::ApplyBillboardMatrix() {
    NaMATRIX<float, 4, 4>& m = this->unk40;
    m = m * CreateBillboardMatrix(m);
}

/*
 * nalib/namatrix.h: weak copies of the 9- and 16-argument Set and of
 * RotateMatrix(int). The generic RotateMatrix(int) compiles to the same
 * instructions, but the original aligns its last case to 8 bytes, which
 * plain C here doesn't reproduce; the Set copies come before it, so they
 * stay asm with it.
 */
#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C4E8);

INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C540);

INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C5F0);
#endif
