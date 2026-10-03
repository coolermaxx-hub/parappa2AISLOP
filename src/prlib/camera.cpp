#include "camera.h"

void SpcFileHeader::Initialize() {
    ChangePointer();
}

PrPERSPECTIVE_CAMERA* SpcFileHeader::GetCamera(float time) const {
    static PrPERSPECTIVE_CAMERA camera;
    camera.position = m_positionTrack ? *m_positionTrack->GetValue(time) : m_position;
    camera.interest = m_interestTrack ? *m_interestTrack->GetValue(time) : m_interest;
    const float roll = m_rollTrack ? *m_rollTrack->GetValue(time) : m_roll;
    camera.field_of_view = m_fieldOfViewTrack ? *m_fieldOfViewTrack->GetValue(time) : m_fieldOfView;
    camera.near_clip = m_nearClip;
    camera.far_clip = m_farClip;
    camera.aspect = m_aspect;

    typedef NaVECTOR<float, 4> Vector;
    const Vector direction = camera.interest - camera.position;
    Vector right = Vector::Cross3(direction, NaMATRIX<float, 4, 4>::IDENT[1]);
    right[3] = 1.0f;
    const Vector vertical = Vector::Cross3(right, direction);
    camera.up = Vector::Normalize3(vertical);
    camera.up[3] = 1.0f;

    if (roll != 0.0f) {
        const NaMATRIX<float, 4, 4> rotation = NaMATRIX<float, 4, 4>::RotateMatrix(direction, roll);
        NaMATRIX<float, 4, 4>::Apply(camera.up, rotation, camera.up);
    }
    return &camera;
}
