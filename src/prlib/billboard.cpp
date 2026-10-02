#include "common.h"

#include "model.h"
#include "spram.h"

#include <nalib/namatrix.h>
#include <math.h>

NaMATRIX<float, 4, 4>& CreateBillboardMatrix(const NaMATRIX<float, 4, 4>& mtx);

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)0x70000000;

NaMATRIX<float, 4, 4>& CreateBillboardMatrix(const NaMATRIX<float, 4, 4>& mtx) {
    NaVECTOR<float, 4> eye;
    eye = prSpramData->m_camera.position;
    ((float*)&eye)[3] = 1.0f;

    NaVECTOR<float, 4> v;
    {
        NaMATRIX<float, 4, 4> inv = mtx.Inverse();
        NaVECTOR<float, 4> tmp;
        v = NaMATRIX<float, 4, 4>::Apply(tmp, inv, eye);
    }

    if (v[0] == 0.0f && v[2] == 0.0f) {
        return NaMATRIX<float, 4, 4>::IDENT;
    }

    float angle = atan2f(((float*)&v)[0], ((float*)&v)[2]);
    static NaMATRIX<float, 4, 4> billboard;
    billboard = NaMATRIX<float, 4, 4>::RotateMatrix(1, angle);
    return billboard;
}

void SpmNode::ApplyBillboardMatrix() {
    NaMATRIX<float, 4, 4>& m = this->unk40;
    m = m * CreateBillboardMatrix(m);
}

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C4E8);

INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C540);

INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C5F0);
