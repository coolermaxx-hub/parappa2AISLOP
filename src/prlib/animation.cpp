#include "animation.h"

void SpaFileHeader::Initialize() {
    ChangePointer();

    if (unk44 == 0) {
        if (unk3C != 0) {
            unk3C = 0;
        }
        if (unk40 != 0) {
            unk40 = 0;
        }
    }
}
