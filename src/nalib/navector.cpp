#include "common.h"

#include "navector.h"
#include "namatrix.h"

/*
 * The 4-argument constructor is out of line in navector.h (render.cpp and
 * scene.cpp emit weak copies of it), but this TU's static initializer has it
 * inlined, so it sees an inline definition.
 */
template <>
inline NaVECTOR<float, 4>::NaVECTOR(const float& x, const float& y, const float& z, const float& w) {
    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;
}

NaVECTOR<float, 4> NaVECTOR<float, 4>::ZERO(0.0f, 0.0f, 0.0f, 0.0f);
NaVECTOR<float, 4> NaVECTOR<float, 4>::ZEROH(0.0f, 0.0f, 0.0f, 1.0f);

NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::ZERO(0.0f, 0.0f, 0.0f, 0.0f,
                                                  0.0f, 0.0f, 0.0f, 0.0f,
                                                  0.0f, 0.0f, 0.0f, 0.0f,
                                                  0.0f, 0.0f, 0.0f, 0.0f);
NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::IDENT(1.0f, 0.0f, 0.0f, 0.0f,
                                                   0.0f, 1.0f, 0.0f, 0.0f,
                                                   0.0f, 0.0f, 1.0f, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 1.0f);

NaVECTOR<float, 2> NaVECTOR<float, 2>::ZERO(0.0f, 0.0f);

NaVECTOR<float, 3> NaVECTOR<float, 3>::ZERO(0.0f, 0.0f, 0.0f);
NaVECTOR<float, 3> NaVECTOR<float, 3>::ZEROH(0.0f, 0.0f, 1.0f);

NaMATRIX<float, 2, 2> NaMATRIX<float, 2, 2>::ZERO(0.0f, 0.0f,
                                                  0.0f, 0.0f);
NaMATRIX<float, 3, 3> NaMATRIX<float, 3, 3>::ZERO(0.0f, 0.0f, 0.0f,
                                                  0.0f, 0.0f, 0.0f,
                                                  0.0f, 0.0f, 0.0f);
NaMATRIX<float, 2, 2> NaMATRIX<float, 2, 2>::IDENT(1.0f, 0.0f,
                                                   0.0f, 1.0f);
NaMATRIX<float, 3, 3> NaMATRIX<float, 3, 3>::IDENT(1.0f, 0.0f, 0.0f,
                                                   0.0f, 1.0f, 0.0f,
                                                   0.0f, 0.0f, 1.0f);

/* the next object (sdk/graphdev) starts on a 16-byte boundary */
asm(".section .data\n.align 4\n.text");
