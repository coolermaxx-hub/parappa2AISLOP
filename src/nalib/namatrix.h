#ifndef NALIB_NAMATRIX_H
#define NALIB_NAMATRIX_H

#include "navector.h"

#include <libvu0.h>

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
        " : : "r"(pret), "r"(this), "r"(&rhs) : "$6", "$7", "$8", "$9");
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

private:
    NaVECTOR<T, t0> m[t1];

public:
    static NaMATRIX<T, t0, t1> ZERO;
    static NaMATRIX<T, t0, t1> IDENT;
};

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

#endif /* NALIB_NAMATRIX_H */
