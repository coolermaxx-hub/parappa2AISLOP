#include "common.h"

#include <nalib/namatrix.h>

#include "model.h"
#include "spram.h"

#include <math.h>

NaMATRIX<float, 4, 4>& CreateBillboardMatrix(const NaMATRIX<float, 4, 4>& mtx);

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)0x70000000;

NaMATRIX<float, 4, 4>& CreateBillboardMatrix(const NaMATRIX<float, 4, 4>& mtx) {
    NaVECTOR<float, 4> eye;
    eye = prSpramData->m_camera.position;
    eye[3] = 1.0f;

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
    NaMATRIX<float, 4, 4>& m = this->m_worldMatrix;
    m = m * CreateBillboardMatrix(m);
}
