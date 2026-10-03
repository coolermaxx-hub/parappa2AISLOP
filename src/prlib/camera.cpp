#include "camera.h"

void SpcFileHeader::Initialize() {
    ChangePointer();
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/camera", GetCamera__C13SpcFileHeaderf);
#else
/*
 * Register allocation and stack slots differ. The original also writes
 * side.w = 1 into the cross product's temporary before copying it, which
 * hints that it used a cross-product helper that sets w itself.
 */
PrPERSPECTIVE_CAMERA* SpcFileHeader::GetCamera(float time) const {
    static PrPERSPECTIVE_CAMERA camera;

    if (m_position_track != NULL) {
        camera.position = *m_position_track->GetValue(time);
    } else {
        camera.position = m_position;
    }

    if (m_interest_track != NULL) {
        camera.interest = *m_interest_track->GetValue(time);
    } else {
        camera.interest = m_interest;
    }

    float roll;
    if (m_roll_track != NULL) {
        roll = *m_roll_track->GetValue(time);
    } else {
        roll = m_roll;
    }

    if (m_field_of_view_track != NULL) {
        camera.field_of_view = *m_field_of_view_track->GetValue(time);
    } else {
        camera.field_of_view = m_field_of_view;
    }

    PrPERSPECTIVE_CAMERA *cam = &camera;
    cam->near_clip = m_near_clip;
    cam->far_clip = m_far_clip;
    cam->aspect = m_aspect;

    /* Up is the world Y axis made perpendicular to the view direction, then rolled around it */
    NaVECTOR<float, 4> dir = cam->interest - cam->position;
    NaVECTOR<float, 4> side = dir.Cross(NaMATRIX<float, 4, 4>::IDENT[1]);
    side[3] = 1.0f;
    cam->up = side.Cross(dir).Normalize();

    if (roll != 0.0f) {
        NaMATRIX<float, 4, 4> rot = NaMATRIX<float, 4, 4>::RotateMatrix(dir, roll);
        NaMATRIX<float, 4, 4>::Apply(cam->up, rot, cam->up);
    }

    return cam;
}
#endif

/* nalib/namatrix.h: weak copies of the 9- and 16-argument Set and RotateMatrix(axis, angle) */
#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153AD0);

INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153B28);

INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153BD8);
#endif
