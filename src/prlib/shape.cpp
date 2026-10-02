#include "common.h"

#include "model.h"

void SpmShapeNode::AddShapePosition(u_int arg0, float arg1) {
    u_int vertex = (u_int)this->unk17C;
    vertex |= 0x30000000;
    u_int num = this->unk194;
    u_long128 *src = &this->unk1C0[arg0];
    u_int stride = this->unk1B0;
    u_int *index = this->unk198;

    for (u_int i = 0; i < num; i++) {
        asm volatile("lqc2 $vf17, 0x0(%0)" : : "r"(src));
        u_int n = *index++;
        src += stride;
        asm volatile("
            qmtc2.ni %0, $vf4
            vmulx.xyz $vf17, $vf17, $vf4x
        " : : "r"(arg1));

        for (u_int j = 0; j < n; j++) {
            u_long128 *v = (u_long128 *)((*index++ << 4) + vertex);
            asm volatile("
                lqc2 $vf4, 0x0(%0)
                vadd.xyz $vf4, $vf4, $vf17
                sqc2 $vf4, 0x0(%0)
            " : : "r"(v));
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/prlib/shape", RenderShapeNode__12SpmShapeNodeP13PrModelObject);
