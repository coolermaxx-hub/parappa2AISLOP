#include "camera.h"

void SpcFileHeader::Initialize() {
    ChangePointer();
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/camera", GetCamera__C13SpcFileHeaderf);
#else
/* Register allocation: cam and the up pointer swap s1/s2 */
PrPERSPECTIVE_CAMERA* SpcFileHeader::GetCamera(float time) const {
    static PrPERSPECTIVE_CAMERA camera;

    if (unk88 != NULL) {
        camera.position = *unk88->GetValue(time);
    } else {
        camera.position = unk40;
    }

    if (unk8C != NULL) {
        camera.interest = *unk8C->GetValue(time);
    } else {
        camera.interest = unk50;
    }

    float roll;
    if (unk90 != NULL) {
        roll = *unk90->GetValue(time);
    } else {
        roll = unk60;
    }

    if (unk94 != NULL) {
        camera.field_of_view = *unk94->GetValue(time);
    } else {
        camera.field_of_view = unk64;
    }

    PrPERSPECTIVE_CAMERA *cam = &camera;
    cam->near_clip = unk6C;
    cam->far_clip = unk70;
    cam->aspect = unk68;

    NaVECTOR<float, 4> dir;
    NaVECTOR<float, 4> tmp0;
    NaVECTOR<float, 4> tmp1;
    NaVECTOR<float, 4> tmp2;
    NaVECTOR<float, 4> tmp3;

    asm volatile(
        "lqc2         $vf4,   0x0(%0)        \n\t"
        "lqc2         $vf5,   0x0(%1)        \n\t"
        "vsub.xyzw    $vf6,   $vf4,   $vf5   \n\t"
        "sqc2         $vf6,   0x0(%2)        \n\t"
    : : "r"(&cam->interest), "r"(&cam->position), "r"(&tmp0));
    dir = tmp0;

    asm volatile(
        "lqc2         $vf4,   0x0(%0)        \n\t"
        "lqc2         $vf5,   0x0(%1)        \n\t"
        "vopmula.xyz  ACC,    $vf4,   $vf5   \n\t"
        "vopmsub.xyz  $vf6,   $vf5,   $vf4   \n\t"
        "vsub.w       $vf6,   $vf6,   $vf6   \n\t"
        "sqc2         $vf6,   0x0(%2)        \n\t"
    : : "r"(&dir), "r"(&NaMATRIX<float, 4, 4>::IDENT[1]), "r"(&tmp1));
    ((float*)&tmp1)[3] = 1.0f;
    tmp0 = tmp1;

    asm volatile(
        "lqc2         $vf4,   0x0(%0)        \n\t"
        "lqc2         $vf5,   0x0(%1)        \n\t"
        "vopmula.xyz  ACC,    $vf4,   $vf5   \n\t"
        "vopmsub.xyz  $vf6,   $vf5,   $vf4   \n\t"
        "vsub.w       $vf6,   $vf6,   $vf6   \n\t"
        "sqc2         $vf6,   0x0(%2)        \n\t"
    : : "r"(&tmp0), "r"(&dir), "r"(&tmp3));
    tmp2 = tmp3;

    asm volatile(
        "lqc2         $vf4,   0x0(%0)        \n\t"
        "vmul.xyz     $vf5,   $vf4,   $vf4   \n\t"
        "vaddy.x      $vf5,   $vf5,   $vf5y  \n\t"
        "vaddz.x      $vf5,   $vf5,   $vf5z  \n\t"
        "vsqrt        Q,      $vf5x          \n\t"
        "vwaitq                              \n\t"
        "vaddq.x      $vf5,   $vf0,   Q      \n\t"
        "vdiv         Q,      $vf0w,  $vf5x  \n\t"
        "vsub.xyzw    $vf6,   $vf0,   $vf0   \n\t"
        "vwaitq                              \n\t"
        "vmulq.xyz    $vf6,   $vf4,   Q      \n\t"
        "sqc2         $vf6,   0x0(%1)        \n\t"
    : : "r"(&tmp2), "r"(&tmp3));
    ((float*)&tmp3)[3] = 1.0f;
    tmp1 = tmp3;
    cam->up = tmp1;

    if (roll != 0.0f) {
        NaMATRIX<float, 4, 4> rot = NaMATRIX<float, 4, 4>::RotateMatrix(dir, roll);
        asm volatile(
            "lqc2         $vf4,   0x0(%0)        \n\t"
            "lqc2         $vf5,  0x10(%0)        \n\t"
            "lqc2         $vf6,  0x20(%0)        \n\t"
            "lqc2         $vf7,  0x30(%0)        \n\t"
            "lqc2         $vf8,   0x0(%1)        \n\t"
            "vmulax.xyzw  ACC,    $vf4,   $vf8x  \n\t"
            "vmadday.xyzw ACC,    $vf5,   $vf8y  \n\t"
            "vmaddaz.xyzw ACC,    $vf6,   $vf8z  \n\t"
            "vmaddw.xyzw  $vf9,   $vf7,   $vf8w  \n\t"
            "sqc2         $vf9,   0x0(%1)        \n\t"
        : : "r"(&rot), "r"(&cam->up));
    }

    return cam;
}
#endif

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153AD0);

INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153B28);

INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153BD8);
