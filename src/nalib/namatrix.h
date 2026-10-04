#ifndef NALIB_NAMATRIX_H
#define NALIB_NAMATRIX_H

#include "navector.h"

#include <libvu0.h>
#include <math.h>

template <typename T, int t0, int t1>
class NaMATRIX {
public:
    NaMATRIX() {}
    NaMATRIX(const T& m00, const T& m01, const T& m10, const T& m11) {
        Set(m00, m01, m10, m11);
    }
    NaMATRIX(const T& m00, const T& m01, const T& m02, const T& m10, const T& m11, const T& m12, const T& m20, const T& m21, const T& m22) {
        Set(m00, m01, m02, m10, m11, m12, m20, m21, m22);
    }
    NaMATRIX(const T& m00, const T& m01, const T& m02, const T& m03, const T& m10, const T& m11, const T& m12, const T& m13, const T& m20, const T& m21, const T& m22, const T& m23, const T& m30, const T& m31, const T& m32, const T& m33) {
        Set(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
    }
    NaMATRIX(const NaMATRIX<T, t0, t1>& rhs) {
        Copy(*this, rhs);
    }

    const NaVECTOR<T, t0>& operator[](int index) const {
        return m[index];
    }

    bool IsIdentity() const {
        const NaMATRIX<T, t0, t1>& ident = IDENT;
        for (int i = 0; i < t1; i++) {
            if (ident[i].Differs(this->m[i])) {
                return false;
            }
        }

        return true;
    }

    bool IsNonZero() const {
        for (int i = 0; i < t1; i++) {
            if (this->m[i].IsNonZero()) {
                return true;
            }
        }
        
        return false;
    }

    static NaMATRIX<T, t0, t1>& Copy(NaMATRIX<T, t0, t1>& lhs, const NaMATRIX<T, t0, t1>& rhs);

    NaMATRIX<T, t0, t1>& Set(const T& m00, const T& m01, const T& m10, const T& m11);
    NaMATRIX<T, t0, t1>& Set(const T& m00, const T& m01, const T& m02, const T& m10, const T& m11, const T& m12, const T& m20, const T& m21, const T& m22);
    NaMATRIX<T, t0, t1>& Set(const T& m00, const T& m01, const T& m02, const T& m03, const T& m10, const T& m11, const T& m12, const T& m13, const T& m20, const T& m21, const T& m22, const T& m23, const T& m30, const T& m31, const T& m32, const T& m33);

    static NaMATRIX<float, 4, 4> RotateMatrix(const NaVECTOR<float, 4>& axis, const float& angle);
    static NaMATRIX<float, 4, 4> RotateMatrix(int axis, const float& angle);

    static NaMATRIX<float, 4, 4> ScaleMatrix(const float& x, const float& y, const float& z);

    static NaMATRIX<float, 4, 4> TranslateMatrix(const float& x, const float& y, const float& z);
    static NaMATRIX<float, 4, 4> TranslateMatrix(const NaVECTOR<float, 4>& translation);
    static NaMATRIX<float, 4, 4> ScaleMatrix(const NaVECTOR<float, 4>& scale);

    NaMATRIX<float, 4, 4> Inverse() const {
        NaMATRIX<float, 4, 4> ret;
        sceVu0InversMatrix((sceVu0FVECTOR*)&ret, (sceVu0FVECTOR*)this);
        return ret;
    }

    NaMATRIX<T, t0, t1>& operator=(const NaMATRIX<T, t0, t1>& rhs) {
        return Copy(*this, rhs);
    }

    static NaVECTOR<T, t0>& Apply(NaVECTOR<T, t0>& out, const NaMATRIX<T, t0, t1>& lhs, const NaVECTOR<T, t1>& rhs);

    // Scalar callers use the transpose of the VU column-vector convention.
    // Keep the original left-to-right accumulation, including the initial zero.
    NaVECTOR<T, t1>& ApplyTransposed(NaVECTOR<T, t1>& result, const NaVECTOR<T, t0>& value) const {
        for (int i = 0; i < t1; i++) {
            T sum = 0;
            for (int j = 0; j < t0; j++) sum += m[i][j] * value[j];
            result[i] = sum;
        }
        return result;
    }

    NaVECTOR<T, t1> ApplyTransposed(const NaVECTOR<T, t0>& value) const {
        NaVECTOR<T, t1> result;
        ApplyTransposed(result, value);
        return result;
    }

    NaVECTOR<T, t0> operator*(const NaVECTOR<T, t1>& rhs) const {
        NaVECTOR<T, t0> ret;
        Apply(ret, *this, rhs);
        return ret;
    }

    static NaMATRIX<T, t0, t1>& Multiply(NaMATRIX<T, t0, t1>& out, const NaMATRIX<T, t0, t1>& lhs, const NaMATRIX<T, t1, t1>& rhs);

    NaMATRIX<float, 4, 4>& Translate(const float& x, const float& y, const float& z);
    NaMATRIX<float, 4, 4>& Scale(const float& x, const float& y, const float& z);

    NaMATRIX<T, t0, t1> operator*(const NaMATRIX<T, t1, t1>& rhs) const {
        NaMATRIX<T, t0, t1> result;
        Multiply(result, *this, rhs);
        return result;
    }

    NaMATRIX<T, t0, t1> operator*(const T& s) const {
        NaMATRIX<T, t0, t1> ret;
        for (int i = 0; i < t1; i++) {
            ret.m[i] = m[i] * s;
        }
        return ret;
    }

    NaMATRIX<T, t0, t1> operator+(const NaMATRIX<T, t0, t1>& rhs) const {
        NaMATRIX<T, t0, t1> ret;
        for (int i = 0; i < t1; i++) {
            ret.m[i] = m[i] + rhs.m[i];
        }
        return ret;
    }

    NaMATRIX<T, t0, t1> operator-(const NaMATRIX<T, t0, t1>& rhs) const {
        NaMATRIX<T, t0, t1> ret;
        for (int i = 0; i < t1; i++) {
            ret.m[i] = m[i] - rhs.m[i];
        }
        return ret;
    }

    NaMATRIX<T, t0, t1> operator/(const T& s) const {
        NaMATRIX<T, t0, t1> ret;
        for (int i = 0; i < t1; i++) {
            ret.m[i] = m[i] / s;
        }
        return ret;
    }

private:
    // VU matrices store consecutive column vectors. The Set overloads write
    // a packed prefix, including the historical nine-scalar Set on a 4x4.
    T& Element(int index) {
        return m[index / t0][index % t0];
    }

    NaVECTOR<T, t0> m[t1];

public:
    static NaMATRIX<T, t0, t1> ZERO;
    static NaMATRIX<T, t0, t1> IDENT;
};

template <typename T, int t0, int t1>
inline NaMATRIX<T, t0, t1>& NaMATRIX<T, t0, t1>::Copy(NaMATRIX<T, t0, t1>& lhs, const NaMATRIX<T, t0, t1>& rhs) {
    for (int column = 0; column < t1; column++) lhs.m[column] = rhs.m[column];
    return lhs;
}

// Keep the original four-quadword MMI transfer for the actual EE 4x4 type.
template <>
inline NaMATRIX<float, 4, 4>& NaMATRIX<float, 4, 4>::Copy(NaMATRIX<float, 4, 4>& lhs, const NaMATRIX<float, 4, 4>& rhs) {
    asm volatile("\n\
        lq $6, 0(%1)\n\
        lq $7, 0x10(%1)\n\
        lq $8, 0x20(%1)\n\
        lq $9, 0x30(%1)\n\
        sq $6, 0(%0)\n\
        sq $7, 0x10(%0)\n\
        sq $8, 0x20(%0)\n\
        sq $9, 0x30(%0)\n\
    " : : "r"(&lhs), "r"(&rhs)
    : "$6", "$7", "$8", "$9", "memory");
    return lhs;
}

// Column-vector products: t0 rows, t1 columns. Stage the complete result
// before writing out so that the scalar path permits the same in-place use
// as the VU path. Seed with the first product, as VMULA does.
template <typename T, int t0, int t1>
inline NaVECTOR<T, t0>& NaMATRIX<T, t0, t1>::Apply(NaVECTOR<T, t0>& out, const NaMATRIX<T, t0, t1>& lhs, const NaVECTOR<T, t1>& rhs) {
    NaVECTOR<T, t0> result;
    for (int row = 0; row < t0; row++) {
        T sum = lhs.m[0][row] * rhs[0];
        for (int column = 1; column < t1; column++) sum += lhs.m[column][row] * rhs[column];
        result[row] = sum;
    }
    return out = result;
}

// The right operand is square so the product retains this matrix's shape.
template <typename T, int t0, int t1>
inline NaMATRIX<T, t0, t1>& NaMATRIX<T, t0, t1>::Multiply(NaMATRIX<T, t0, t1>& out, const NaMATRIX<T, t0, t1>& lhs, const NaMATRIX<T, t1, t1>& rhs) {
    NaMATRIX<T, t0, t1> result;
    for (int column = 0; column < t1; column++) Apply(result.m[column], lhs, rhs[column]);
    return Copy(out, result);
}

// Preserve the original EE float4 instruction sequence and accumulation order.
template <>
inline NaVECTOR<float, 4>& NaMATRIX<float, 4, 4>::Apply(NaVECTOR<float, 4>& out, const NaMATRIX<float, 4, 4>& lhs, const NaVECTOR<float, 4>& rhs) {
    asm volatile("\n\
        lqc2         $vf4, 0x0(%1)\n\
        lqc2         $vf5, 0x10(%1)\n\
        lqc2         $vf6, 0x20(%1)\n\
        lqc2         $vf7, 0x30(%1)\n\
        lqc2         $vf8, 0x0(%2)\n\
        vmulax.xyzw  ACC, $vf4, $vf8x\n\
        vmadday.xyzw ACC, $vf5, $vf8y\n\
        vmaddaz.xyzw ACC, $vf6, $vf8z\n\
        vmaddw.xyzw  $vf9, $vf7, $vf8w\n\
        sqc2         $vf9, 0x0(%0)\n\
    " : : "r"(&out), "r"(&lhs), "r"(&rhs) : "memory");
    return out;
}

template <>
inline NaMATRIX<float, 4, 4>& NaMATRIX<float, 4, 4>::Multiply(NaMATRIX<float, 4, 4>& out, const NaMATRIX<float, 4, 4>& lhs, const NaMATRIX<float, 4, 4>& rhs) {
    asm volatile("\n\
        lqc2         $vf4, 0x0(%1)\n\
        lqc2         $vf5, 0x10(%1)\n\
        lqc2         $vf6, 0x20(%1)\n\
        lqc2         $vf7, 0x30(%1)\n\
        lqc2         $vf8, 0x0(%2)\n\
        vmulax.xyzw  ACC, $vf4, $vf8x\n\
        vmadday.xyzw ACC, $vf5, $vf8y\n\
        vmaddaz.xyzw ACC, $vf6, $vf8z\n\
        vmaddw.xyzw  $vf9, $vf7, $vf8w\n\
        sqc2         $vf9, 0x0(%0)\n\
        lqc2         $vf8, 0x10(%2)\n\
        vmulax.xyzw  ACC, $vf4, $vf8x\n\
        vmadday.xyzw ACC, $vf5, $vf8y\n\
        vmaddaz.xyzw ACC, $vf6, $vf8z\n\
        vmaddw.xyzw  $vf9, $vf7, $vf8w\n\
        sqc2         $vf9, 0x10(%0)\n\
        lqc2         $vf8, 0x20(%2)\n\
        vmulax.xyzw  ACC, $vf4, $vf8x\n\
        vmadday.xyzw ACC, $vf5, $vf8y\n\
        vmaddaz.xyzw ACC, $vf6, $vf8z\n\
        vmaddw.xyzw  $vf9, $vf7, $vf8w\n\
        sqc2         $vf9, 0x20(%0)\n\
        lqc2         $vf8, 0x30(%2)\n\
        vmulax.xyzw  ACC, $vf4, $vf8x\n\
        vmadday.xyzw ACC, $vf5, $vf8y\n\
        vmaddaz.xyzw ACC, $vf6, $vf8z\n\
        vmaddw.xyzw  $vf9, $vf7, $vf8w\n\
        sqc2         $vf9, 0x30(%0)\n\
    " : : "r"(&out), "r"(&lhs), "r"(&rhs) : "memory");
    return out;
}

template <typename T, int t0, int t1>
NaMATRIX<T, t0, t1>& NaMATRIX<T, t0, t1>::Set(const T& m00, const T& m01, const T& m10, const T& m11) {
    Element(0) = m00;
    Element(1) = m01;
    Element(2) = m10;
    Element(3) = m11;
    return *this;
}

template <typename T, int t0, int t1>
NaMATRIX<T, t0, t1>& NaMATRIX<T, t0, t1>::Set(const T& m00, const T& m01, const T& m02, const T& m10, const T& m11, const T& m12, const T& m20, const T& m21, const T& m22) {
    Element(0) = m00;
    Element(1) = m01;
    Element(2) = m02;
    Element(3) = m10;
    Element(4) = m11;
    Element(5) = m12;
    Element(6) = m20;
    Element(7) = m21;
    Element(8) = m22;
    return *this;
}

template <typename T, int t0, int t1>
NaMATRIX<T, t0, t1>& NaMATRIX<T, t0, t1>::Set(const T& m00, const T& m01, const T& m02, const T& m03, const T& m10, const T& m11, const T& m12, const T& m13, const T& m20, const T& m21, const T& m22, const T& m23, const T& m30, const T& m31, const T& m32, const T& m33) {
    Element(0) = m00;
    Element(1) = m01;
    Element(2) = m02;
    Element(3) = m03;
    Element(4) = m10;
    Element(5) = m11;
    Element(6) = m12;
    Element(7) = m13;
    Element(8) = m20;
    Element(9) = m21;
    Element(10) = m22;
    Element(11) = m23;
    Element(12) = m30;
    Element(13) = m31;
    Element(14) = m32;
    Element(15) = m33;
    return *this;
}

template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::RotateMatrix(const NaVECTOR<float, 4>& axis, const float& angle) {
    const float yz = axis[1] * axis[1] + axis[2] * axis[2];
    const float length = sqrtf(axis[0] * axis[0] + yz);
    const float radius = sqrtf(yz);
    float p, q;

    if (radius < 1.1920929e-07f) {
        p = 0.0f;
        q = 1.0f;
    } else {
        p = axis[1] / radius;
        q = axis[2] / radius;
    }

    const float a = radius / length;
    const float b = -axis[0] / length;
    const float s = sinf(angle);
    const float c = cosf(angle);
    const float k = a * (c - 1.0f);

    return NaMATRIX<float, 4, 4>(a * k + 1.0f, p * b * k + q * a * s, q * b * k - p * a * s, 0.0f,
                                  p * b * k - q * a * s, -p * p * a * k + c, -p * q * a * k - b * s, 0.0f,
                                  q * b * k + p * a * s, -p * q * a * k + b * s, -q * q * a * k + c, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f);
}

// These are column-major transforms; translation occupies the last column.
template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::RotateMatrix(int axis, const float& angle) {
    const float c = cosf(angle);
    const float s = sinf(angle);

    if (axis == 0) {
        return NaMATRIX<float, 4, 4>(1.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, c, s, 0.0f,
                                   0.0f, -s, c, 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f);
    }
    if (axis == 1) {
        return NaMATRIX<float, 4, 4>(c, 0.0f, -s, 0.0f,
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   s, 0.0f, c, 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f);
    }
    return NaMATRIX<float, 4, 4>(c, s, 0.0f, 0.0f,
                               -s, c, 0.0f, 0.0f,
                               0.0f, 0.0f, 1.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 1.0f);
}

template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::ScaleMatrix(const float& x, const float& y, const float& z) {
    return NaMATRIX<float, 4, 4>(x, 0.0f, 0.0f, 0.0f,
                               0.0f, y, 0.0f, 0.0f,
                               0.0f, 0.0f, z, 0.0f,
                               0.0f, 0.0f, 0.0f, 1.0f);
}

template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::TranslateMatrix(const float& x, const float& y, const float& z) {
    return NaMATRIX<float, 4, 4>(1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 1.0f, 0.0f,
                               x, y, z, 1.0f);
}

template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::TranslateMatrix(const NaVECTOR<float, 4>& translation) {
    return TranslateMatrix(translation[0], translation[1], translation[2]);
}

template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::ScaleMatrix(const NaVECTOR<float, 4>& scale) {
    // Unlike the three-scalar overload, this overload preserves scale.w.
    return NaMATRIX<float, 4, 4>(scale[0], 0.0f, 0.0f, 0.0f,
                               0.0f, scale[1], 0.0f, 0.0f,
                               0.0f, 0.0f, scale[2], 0.0f,
                               0.0f, 0.0f, 0.0f, scale[3]);
}

template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4>& NaMATRIX<T, t0, t1>::Translate(const float& x, const float& y, const float& z) {
    const NaMATRIX<float, 4, 4> translation = TranslateMatrix(x, y, z);
    return Multiply(*this, translation, *this);
}

template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4>& NaMATRIX<T, t0, t1>::Scale(const float& x, const float& y, const float& z) {
    const NaMATRIX<float, 4, 4> scale = ScaleMatrix(x, y, z);
    return Multiply(*this, scale, *this);
}

#endif /* NALIB_NAMATRIX_H */
