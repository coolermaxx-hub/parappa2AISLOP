#include "common.h"

#include "navector.h"
#include "namatrix.h"

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
