#include "common.h"

#include "model.h"

#include <nalib/namatrix.h>

NaMATRIX<float, 4, 4>& CreateBillboardMatrix(const NaMATRIX<float, 4, 4>& mtx);

INCLUDE_ASM("asm/nonmatchings/prlib/billboard", CreateBillboardMatrix__FRCt8NaMATRIX3Zfi4i4);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/billboard", ApplyBillboardMatrix__7SpmNode);
#else
/* Register allocation: a0/a1 swapped for the product temporary */
void SpmNode::ApplyBillboardMatrix() {
    NaMATRIX<float, 4, 4>& m = this->unk40;
    m = m * CreateBillboardMatrix(m);
}
#endif

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C4E8);

INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C540);

INCLUDE_ASM("asm/nonmatchings/prlib/billboard", func_0014C5F0);
