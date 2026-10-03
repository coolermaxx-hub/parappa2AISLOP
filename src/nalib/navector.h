#ifndef NALIB_NAVECTOR_H
#define NALIB_NAVECTOR_H

template <typename T, int t0>
class NaVECTOR {
public:
    NaVECTOR() {}
    NaVECTOR(const T& x, const T& y) {
        v[0] = x;
        v[1] = y;
    }
    NaVECTOR(const T& x, const T& y, const T& z) {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }
    NaVECTOR(const T& x, const T& y, const T& z, const T& w);
    NaVECTOR(const NaVECTOR<float, 4>& rhs) {
        Copy(*this, rhs);
    }

public:
    T operator[](int arg0) const {
        return v[arg0];
    }

    T& operator[](int arg0) {
        return v[arg0];
    }

    bool inl0(const NaVECTOR<T, t0>& arg0) const {
        for (int i = 0; i < t0; i++) {
            if (arg0[i] != v[i]) {
                return true;
            }
        }
        
        return false;
    }

    bool inl1() const {
        for (int i = 0; i < t0; i++) {
            if (v[i] != 0.0f) {
                return true;
            }
        }
        
        return false;
    }

    NaVECTOR<T, t0>& Set(const T& x, const T& y, const T& z, const T& w);

    static NaVECTOR<float, 4>& Copy(NaVECTOR<float, 4>& lhs, const NaVECTOR<float, 4>& rhs) {
        asm volatile("
            lq $6, 0(%1)
            sq $6, 0(%0)
        " : : "r"(&lhs), "r"(&rhs)
        : "$6");
        return lhs;
    }

    NaVECTOR<float, 4>& operator=(const NaVECTOR<float, 4>& rhs) {
        return Copy(*this, rhs);
    }

    NaVECTOR<float, 4> operator*(const float& s) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            mfc1       $8, %2
            qmtc2.ni   $8, $vf5
            vmulx.xyzw $vf6, $vf4, $vf5x
            sqc2       $vf6, 0x0(%0)
        " : : "r"(&ret), "r"(this), "f"(s) : "$8");
        return ret;
    }

    NaVECTOR<float, 4> operator+(const NaVECTOR<float, 4>& rhs) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            lqc2       $vf5, 0x0(%2)
            vadd.xyzw  $vf6, $vf4, $vf5
            sqc2       $vf6, 0x0(%0)
        " : : "r"(&ret), "r"(this), "r"(&rhs));
        return ret;
    }

    NaVECTOR<float, 4> operator-(const NaVECTOR<float, 4>& rhs) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            lqc2       $vf5, 0x0(%2)
            vsub.xyzw  $vf6, $vf4, $vf5
            sqc2       $vf6, 0x0(%0)
        " : : "r"(&ret), "r"(this), "r"(&rhs));
        return ret;
    }

    NaVECTOR<float, 4> operator-() const {
        NaVECTOR<float, 4> ret;
        for (int i = 0; i < 4; i++) {
            ret[i] = 0.0f - v[i];
        }
        return ret;
    }

    /* Cross product of the xyz parts; w is cleared */
    NaVECTOR<float, 4> Cross(const NaVECTOR<float, 4>& rhs) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2         $vf4, 0x0(%1)
            lqc2         $vf5, 0x0(%2)
            vopmula.xyz  ACC, $vf4, $vf5
            vopmsub.xyz  $vf6, $vf5, $vf4
            vsub.w       $vf6, $vf6, $vf6
            sqc2         $vf6, 0x0(%0)
        " : : "r"(&ret), "r"(this), "r"(&rhs));
        return ret;
    }

    /* Unit vector along the xyz part, with w = 1 */
    NaVECTOR<float, 4> Normalize() const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            vmul.xyz   $vf5, $vf4, $vf4
            vaddy.x    $vf5, $vf5, $vf5y
            vaddz.x    $vf5, $vf5, $vf5z
            vsqrt      Q, $vf5x
            vwaitq
            vaddq.x    $vf5, $vf0, Q
            vdiv       Q, $vf0w, $vf5x
            vsub.xyzw  $vf6, $vf0, $vf0
            vwaitq
            vmulq.xyz  $vf6, $vf4, Q
            sqc2       $vf6, 0x0(%0)
        " : : "r"(&ret), "r"(this));
        ret[3] = 1.0f;
        return ret;
    }

    NaVECTOR<float, 4> operator/(const float& s) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            mfc1       $8, %2
            qmtc2.ni   $8, $vf5
            vdiv       Q, $vf0w, $vf5x
            vwaitq
            vmulq.xyzw $vf4, $vf4, Q
            sqc2       $vf4, 0x0(%0)
        " : : "r"(&ret), "r"(this), "f"(s) : "$8");
        return ret;
    }

private:
    T v[t0];

public:
    static NaVECTOR<T, t0> ZERO;
    static NaVECTOR<T, t0> ZEROH;
};

template <typename T, int t0>
inline NaVECTOR<float, 4> operator*(const float& s, const NaVECTOR<T, t0>& v) {
    return v * s;
}

/* Out of line: the original emitted weak copies of this constructor */
template <typename T, int t0>
NaVECTOR<T, t0>::NaVECTOR(const T& x, const T& y, const T& z, const T& w) {
    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;
}

template <typename T, int t0>
NaVECTOR<T, t0>& NaVECTOR<T, t0>::Set(const T& x, const T& y, const T& z, const T& w) {
    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;
    return *this;
}

#endif /* NALIB_NAVECTOR_H */
