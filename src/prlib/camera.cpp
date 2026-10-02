#include "camera.h"

void SpcFileHeader::Initialize() {
    ChangePointer();
}

INCLUDE_ASM("asm/nonmatchings/prlib/camera", GetCamera__C13SpcFileHeaderf);

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153AD0);

INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153B28);

INCLUDE_ASM("asm/nonmatchings/prlib/camera", func_00153BD8);
