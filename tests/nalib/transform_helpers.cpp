#include "nalib/namatrix.h"

#include <math.h>

// Native checks for the transform constructors, ApplyTransposed and the
// identity/zero predicates. The float 4x4 products are VU assembly and cannot
// run here, so points are transformed with a local column-vector product, and
// 4x4 matrices are never copied (Copy is an MMI quadword transfer).

template <>
NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::IDENT(1.0f, 0.0f, 0.0f, 0.0f,
                                                   0.0f, 1.0f, 0.0f, 0.0f,
                                                   0.0f, 0.0f, 1.0f, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 1.0f);
template <>
NaMATRIX<float, 2, 2> NaMATRIX<float, 2, 2>::IDENT(1.0f, 0.0f,
                                                   0.0f, 1.0f);

static bool near(float a, float b) {
    return fabsf(a - b) < 1e-5f;
}

// result = m * v with m stored as column vectors.
static NaVECTOR<float, 4> transform(const NaMATRIX<float, 4, 4>& m, const NaVECTOR<float, 4>& v) {
    NaVECTOR<float, 4> result;
    for (int row = 0; row < 4; row++) {
        result[row] = m[0][row] * v[0] + m[1][row] * v[1] + m[2][row] * v[2] + m[3][row] * v[3];
    }
    return result;
}

static bool equals(const NaVECTOR<float, 4>& v, float x, float y, float z, float w) {
    return v[0] == x && v[1] == y && v[2] == z && v[3] == w;
}

int main() {
    typedef NaMATRIX<float, 4, 4> Matrix;
    typedef NaVECTOR<float, 4> Vector;

    // Translation sits in the last column: points move, directions do not.
    const Matrix translate = Matrix::TranslateMatrix(2.0f, -3.0f, 5.0f);
    if (!equals(translate[3], 2.0f, -3.0f, 5.0f, 1.0f)) return 1;
    if (!equals(transform(translate, Vector(1.0f, 1.0f, 1.0f, 1.0f)), 3.0f, -2.0f, 6.0f, 1.0f)) return 2;
    if (!equals(transform(translate, Vector(1.0f, 1.0f, 1.0f, 0.0f)), 1.0f, 1.0f, 1.0f, 0.0f)) return 3;

    // The vector overload ignores w.
    const Matrix translateVector = Matrix::TranslateMatrix(Vector(2.0f, -3.0f, 5.0f, 9.0f));
    for (int column = 0; column < 4; column++) {
        if (translateVector[column].Differs(translate[column])) return 4;
    }

    // The three-scalar scale keeps w at 1; the vector overload copies scale.w.
    const Matrix scale = Matrix::ScaleMatrix(2.0f, 3.0f, 4.0f);
    if (!equals(transform(scale, Vector(1.0f, 1.0f, 1.0f, 1.0f)), 2.0f, 3.0f, 4.0f, 1.0f)) return 5;
    const Matrix scaleVector = Matrix::ScaleMatrix(Vector(2.0f, 3.0f, 4.0f, 0.5f));
    if (!equals(transform(scaleVector, Vector(1.0f, 1.0f, 1.0f, 1.0f)), 2.0f, 3.0f, 4.0f, 0.5f)) return 6;

    // ApplyTransposed dots each stored column with the input. The noodle code
    // builds rot(c, s, -s, c), a +angle rotation as a column matrix, so
    // ApplyTransposed rotates by -angle.
    const float angle = 0.6f;
    const float c = cosf(angle);
    const float s = sinf(angle);
    const NaMATRIX<float, 2, 2> rot(c, s, -s, c);
    const NaVECTOR<float, 2> x = rot.ApplyTransposed(NaVECTOR<float, 2>(1.0f, 0.0f));
    const NaVECTOR<float, 2> y = rot.ApplyTransposed(NaVECTOR<float, 2>(0.0f, 1.0f));
    if (!near(x[0], cosf(-angle)) || !near(x[1], sinf(-angle))) return 7;
    if (!near(y[0], -sinf(-angle)) || !near(y[1], cosf(-angle))) return 8;
    NaVECTOR<float, 2> out;
    if (&rot.ApplyTransposed(out, NaVECTOR<float, 2>(2.0f, 1.0f)) != &out) return 9;
    if (!near(out[0], 2.0f * c + s) || !near(out[1], -2.0f * s + c)) return 10;

    // IsIdentity compares exactly against IDENT (so -0.0f still passes).
    if (!Matrix::IDENT.IsIdentity()) return 11;
    const Matrix negativeZeroOffset = Matrix::TranslateMatrix(-0.0f, 0.0f, -0.0f);
    if (!negativeZeroOffset.IsIdentity()) return 12;
    const Matrix tinyOffset = Matrix::TranslateMatrix(1e-7f, 0.0f, 0.0f);
    if (tinyOffset.IsIdentity()) return 13;
    if (!NaMATRIX<float, 2, 2>::IDENT.IsIdentity() || rot.IsIdentity()) return 14;

    // IsNonZero treats -0.0f as zero and NaN as non-zero.
    const Matrix negativeZero(-0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f,
                              -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f);
    if (negativeZero.IsNonZero()) return 15;
    const float nan = sqrtf(-1.0f);
    const Matrix withNan(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, nan);
    if (!withNan.IsNonZero()) return 16;
    if (!Matrix::IDENT.IsNonZero()) return 17;

    return 0;
}
