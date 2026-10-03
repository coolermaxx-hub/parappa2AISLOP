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
    NaMATRIX(const NaMATRIX<float, 4, 4>& rhs) {
        NaMATRIX<float, 4, 4> *dst = this;
        const NaMATRIX<float, 4, 4> *src = &rhs;
        Copy(*dst, *src);
    }

    const NaVECTOR<T, t0>& operator[](int arg0) const {
        return m[arg0];
    }

    bool inl0() const {
        const NaMATRIX<float, 4, 4>& a0 = NaMATRIX<float, 4, 4>::IDENT;
        for (int i = 0; i < t1; i++) {
            if (a0[i].inl0(this->m[i])) {
                return false;
            }
        }

        return true;
    }

    bool inl1() const {
        for (int i = 0; i < t1; i++) {
            if (this->m[i].inl1()) {
                return true;
            }
        }
        
        return false;
    }

    static NaMATRIX<float, 4, 4>& Copy(NaMATRIX<float, 4, 4>& lhs, const NaMATRIX<float, 4, 4>& rhs) {
        asm volatile("
            lq $6, 0(%1)
            lq $7, 0x10(%1)
            lq $8, 0x20(%1)
            lq $9, 0x30(%1)
            sq $6, 0(%0)
            sq $7, 0x10(%0)
            sq $8, 0x20(%0)
            sq $9, 0x30(%0)
        " : : "r"(&lhs), "r"(&rhs)
        : "$6", "$7", "$8", "$9");
        return lhs;
    }

    NaMATRIX<T, t0, t1>& Set(const T& m00, const T& m01, const T& m10, const T& m11);
    NaMATRIX<T, t0, t1>& Set(const T& m00, const T& m01, const T& m02, const T& m10, const T& m11, const T& m12, const T& m20, const T& m21, const T& m22);
    NaMATRIX<T, t0, t1>& Set(const T& m00, const T& m01, const T& m02, const T& m03, const T& m10, const T& m11, const T& m12, const T& m13, const T& m20, const T& m21, const T& m22, const T& m23, const T& m30, const T& m31, const T& m32, const T& m33);

    static NaMATRIX<float, 4, 4> RotateMatrix(const NaVECTOR<float, 4>& axis, const float& angle);
    static NaMATRIX<float, 4, 4> RotateMatrix(int axis, const float& angle);
    static NaMATRIX<float, 4, 4> TranslateMatrix(const float& x, const float& y, const float& z);
    static NaMATRIX<float, 4, 4> TranslateMatrix(const NaVECTOR<float, 4>& v);
    static NaMATRIX<float, 4, 4> ScaleMatrix(const float& x, const float& y, const float& z);
    static NaMATRIX<float, 4, 4> ScaleMatrix(const NaVECTOR<float, 4>& v);

    NaMATRIX<float, 4, 4>& Translate(const float& x, const float& y, const float& z);
    NaMATRIX<float, 4, 4>& Scale(const float& x, const float& y, const float& z);

    NaMATRIX<float, 4, 4> Inverse() const {
        NaMATRIX<float, 4, 4> ret;
        sceVu0InversMatrix((sceVu0FVECTOR*)&ret, (sceVu0FVECTOR*)this);
        return ret;
    }

    NaMATRIX<float, 4, 4>& operator=(const NaMATRIX<float, 4, 4>& rhs) {
        return Copy(*this, rhs);
    }

    static NaVECTOR<float, 4>& Apply(NaVECTOR<float, 4>& out, const NaMATRIX<float, 4, 4>& lhs, const NaVECTOR<float, 4>& rhs) {
        asm volatile("
            lqc2         $vf4, 0x0(%1)
            lqc2         $vf5, 0x10(%1)
            lqc2         $vf6, 0x20(%1)
            lqc2         $vf7, 0x30(%1)
            lqc2         $vf8, 0x0(%2)
            vmulax.xyzw  ACC, $vf4, $vf8x
            vmadday.xyzw ACC, $vf5, $vf8y
            vmaddaz.xyzw ACC, $vf6, $vf8z
            vmaddw.xyzw  $vf9, $vf7, $vf8w
            sqc2         $vf9, 0x0(%0)
        " : : "r"(&out), "r"(&lhs), "r"(&rhs));
        return out;
    }

    NaVECTOR<float, 4> operator*(const NaVECTOR<float, 4>& rhs) const {
        NaVECTOR<float, 4> ret;
        Apply(ret, *this, rhs);
        return ret;
    }

    NaMATRIX<float, 4, 4> operator*(const NaMATRIX<float, 4, 4>& rhs) const {
        NaMATRIX<float, 4, 4> ret;
        NaMATRIX<float, 4, 4> *pret = &ret;
        const NaMATRIX<float, 4, 4> *pthis = this;
        asm volatile("
            lqc2         $vf4, 0x0(%1)
            lqc2         $vf5, 0x10(%1)
            lqc2         $vf6, 0x20(%1)
            lqc2         $vf7, 0x30(%1)
            lqc2         $vf8, 0x0(%2)
            vmulax.xyzw  ACC, $vf4, $vf8x
            vmadday.xyzw ACC, $vf5, $vf8y
            vmaddaz.xyzw ACC, $vf6, $vf8z
            vmaddw.xyzw  $vf9, $vf7, $vf8w
            sqc2         $vf9, 0x0(%0)
            lqc2         $vf8, 0x10(%2)
            vmulax.xyzw  ACC, $vf4, $vf8x
            vmadday.xyzw ACC, $vf5, $vf8y
            vmaddaz.xyzw ACC, $vf6, $vf8z
            vmaddw.xyzw  $vf9, $vf7, $vf8w
            sqc2         $vf9, 0x10(%0)
            lqc2         $vf8, 0x20(%2)
            vmulax.xyzw  ACC, $vf4, $vf8x
            vmadday.xyzw ACC, $vf5, $vf8y
            vmaddaz.xyzw ACC, $vf6, $vf8z
            vmaddw.xyzw  $vf9, $vf7, $vf8w
            sqc2         $vf9, 0x20(%0)
            lqc2         $vf8, 0x30(%2)
            vmulax.xyzw  ACC, $vf4, $vf8x
            vmadday.xyzw ACC, $vf5, $vf8y
            vmaddaz.xyzw ACC, $vf6, $vf8z
            vmaddw.xyzw  $vf9, $vf7, $vf8w
            sqc2         $vf9, 0x30(%0)
        " : : "r"(pret), "r"(pthis), "r"(&rhs) : "$6", "$7", "$8", "$9");
        return ret;
    }

    NaMATRIX<float, 4, 4> operator*(const float& s) const {
        NaMATRIX<float, 4, 4> ret;
        for (int i = 0; i < 4; i++) {
            ret.m[i] = m[i] * s;
        }
        return ret;
    }

    NaMATRIX<float, 4, 4> operator+(const NaMATRIX<float, 4, 4>& rhs) const {
        NaMATRIX<float, 4, 4> ret;
        for (int i = 0; i < 4; i++) {
            ret.m[i] = m[i] + rhs.m[i];
        }
        return ret;
    }

    NaMATRIX<float, 4, 4> operator-(const NaMATRIX<float, 4, 4>& rhs) const {
        NaMATRIX<float, 4, 4> ret;
        for (int i = 0; i < 4; i++) {
            ret.m[i] = m[i] - rhs.m[i];
        }
        return ret;
    }

    NaMATRIX<float, 4, 4> operator/(const float& s) const {
        NaMATRIX<float, 4, 4> ret;
        for (int i = 0; i < 4; i++) {
            ret.m[i] = m[i] / s;
        }
        return ret;
    }

private:
    NaVECTOR<T, t0> m[t1];

public:
    static NaMATRIX<T, t0, t1> ZERO;
    static NaMATRIX<T, t0, t1> IDENT;
};

template <typename T, int t0, int t1>
inline NaMATRIX<float, 4, 4> operator*(const float& s, const NaMATRIX<T, t0, t1>& m) {
    return m * s;
}

template <typename T, int t0, int t1>
NaMATRIX<T, t0, t1>& NaMATRIX<T, t0, t1>::Set(const T& m00, const T& m01, const T& m10, const T& m11) {
    ((T*)m)[0] = m00;
    ((T*)m)[1] = m01;
    ((T*)m)[2] = m10;
    ((T*)m)[3] = m11;
    return *this;
}

template <typename T, int t0, int t1>
NaMATRIX<T, t0, t1>& NaMATRIX<T, t0, t1>::Set(const T& m00, const T& m01, const T& m02, const T& m10, const T& m11, const T& m12, const T& m20, const T& m21, const T& m22) {
    ((T*)m)[0] = m00;
    ((T*)m)[1] = m01;
    ((T*)m)[2] = m02;
    ((T*)m)[3] = m10;
    ((T*)m)[4] = m11;
    ((T*)m)[5] = m12;
    ((T*)m)[6] = m20;
    ((T*)m)[7] = m21;
    ((T*)m)[8] = m22;
    return *this;
}

template <typename T, int t0, int t1>
NaMATRIX<T, t0, t1>& NaMATRIX<T, t0, t1>::Set(const T& m00, const T& m01, const T& m02, const T& m03, const T& m10, const T& m11, const T& m12, const T& m13, const T& m20, const T& m21, const T& m22, const T& m23, const T& m30, const T& m31, const T& m32, const T& m33) {
    ((T*)m)[0] = m00;
    ((T*)m)[1] = m01;
    ((T*)m)[2] = m02;
    ((T*)m)[3] = m03;
    ((T*)m)[4] = m10;
    ((T*)m)[5] = m11;
    ((T*)m)[6] = m12;
    ((T*)m)[7] = m13;
    ((T*)m)[8] = m20;
    ((T*)m)[9] = m21;
    ((T*)m)[10] = m22;
    ((T*)m)[11] = m23;
    ((T*)m)[12] = m30;
    ((T*)m)[13] = m31;
    ((T*)m)[14] = m32;
    ((T*)m)[15] = m33;
    return *this;
}

/*
 * Matrices are column-major like libvu0's: m[3] is the translation
 * column and a vector is transformed as M * v (see Apply).
 */
template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::TranslateMatrix(const float& x, const float& y, const float& z) {
    return NaMATRIX<float, 4, 4>(1.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f, 0.0f,
                                 x,    y,    z,    1.0f);
}

/* Identity with the translation column replaced by v (w included) */
template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::TranslateMatrix(const NaVECTOR<float, 4>& v) {
    NaMATRIX<float, 4, 4> ret;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            ret.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    ret.m[3] = v;
    return ret;
}

template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::ScaleMatrix(const float& x, const float& y, const float& z) {
    return NaMATRIX<float, 4, 4>(x,    0.0f, 0.0f, 0.0f,
                                 0.0f, y,    0.0f, 0.0f,
                                 0.0f, 0.0f, z,    0.0f,
                                 0.0f, 0.0f, 0.0f, 1.0f);
}

/* Diagonal matrix of all four components of v */
template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::ScaleMatrix(const NaVECTOR<float, 4>& v) {
    NaMATRIX<float, 4, 4> ret;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ret.m[i][j] = (i == j) ? v[j] : 0.0f;
        }
    }
    return ret;
}

/* Applies a translation after this transform */
template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4>& NaMATRIX<T, t0, t1>::Translate(const float& x, const float& y, const float& z) {
    *this = TranslateMatrix(x, y, z) * *this;
    return *this;
}

/* Applies a scale after this transform */
template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4>& NaMATRIX<T, t0, t1>::Scale(const float& x, const float& y, const float& z) {
    *this = ScaleMatrix(x, y, z) * *this;
    return *this;
}

/* Rotation by angle around axis (which needn't be unit length) */
template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::RotateMatrix(const NaVECTOR<float, 4>& axis, const float& angle) {
    float yz = axis[1] * axis[1] + axis[2] * axis[2];
    float len = sqrtf(axis[0] * axis[0] + yz);
    float r = sqrtf(yz);
    float p, q;

    if (r < 1.1920929e-07f) {
        p = 0.0f;
        q = 1.0f;
    } else {
        p = axis[1] / r;
        q = axis[2] / r;
    }

    float a = r / len;
    float b = -axis[0] / len;
    float s = sinf(angle);
    float c = cosf(angle);
    float k = a * (c - 1.0f);

    return NaMATRIX<float, 4, 4>(a * k + 1.0f,          p * b * k + q * a * s,  q * b * k - p * a * s,  0.0f,
                                 p * b * k - q * a * s, -p * p * a * k + c,      -p * q * a * k - b * s, 0.0f,
                                 q * b * k + p * a * s, -p * q * a * k + b * s,  -q * q * a * k + c,     0.0f,
                                 0.0f,                  0.0f,                    0.0f,                   1.0f);
}

/* Rotation by angle around the X (0), Y (1) or Z (2) axis */
template <typename T, int t0, int t1>
NaMATRIX<float, 4, 4> NaMATRIX<T, t0, t1>::RotateMatrix(int axis, const float& angle) {
    float c = cosf(angle);
    float s = sinf(angle);

    if (axis == 0) {
        return NaMATRIX<float, 4, 4>(1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, c,    s,    0.0f,
                                     0.0f, -s,   c,    0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f);
    }

    if (axis == 1) {
        return NaMATRIX<float, 4, 4>(c,    0.0f, -s,   0.0f,
                                     0.0f, 1.0f, 0.0f, 0.0f,
                                     s,    0.0f, c,    0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f);
    }

    return NaMATRIX<float, 4, 4>(c,    s,    0.0f, 0.0f,
                                 -s,   c,    0.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 1.0f);
}

#endif /* NALIB_NAMATRIX_H */
