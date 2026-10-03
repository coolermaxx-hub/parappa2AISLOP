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
    NaVECTOR(const NaVECTOR<T, t0>& rhs) {
        Copy(*this, rhs);
    }

public:
    T& operator[](int index) {
        return v[index];
    }

    T operator[](int arg0) const {
        return v[arg0];
    }

    T* Data() { return v; }
    const T* Data() const { return v; }

    NaVECTOR<T, t0> operator-() const {
        NaVECTOR<T, t0> result;
        for (int i = 0; i < t0; i++) result[i] = T(0) - v[i];
        return result;
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

    static NaVECTOR<T, t0>& Copy(NaVECTOR<T, t0>& lhs, const NaVECTOR<T, t0>& rhs);

    NaVECTOR<T, t0>& operator=(const NaVECTOR<T, t0>& rhs) {
        return Copy(*this, rhs);
    }

    static NaVECTOR<float, 4> Cross3(const NaVECTOR<float, 4>& lhs, const NaVECTOR<float, 4>& rhs) {
        NaVECTOR<float, 4> result;
        asm volatile("
            lqc2         $vf4, 0(%0)
            lqc2         $vf5, 0(%1)
            vopmula.xyz  ACC, $vf4, $vf5
            vopmsub.xyz  $vf6, $vf5, $vf4
            vsub.w       $vf6, $vf6, $vf6
            sqc2         $vf6, 0(%2)
        " : : "r"(&lhs), "r"(&rhs), "r"(&result) : "memory");
        return result;
    }

    static NaVECTOR<float, 4> Normalize3(const NaVECTOR<float, 4>& value) {
        NaVECTOR<float, 4> result;
        // Keep the VU sum and reciprocal order, including singular inputs.
        asm volatile("
            lqc2         $vf4, 0(%0)
            vmul.xyz     $vf5, $vf4, $vf4
            vaddy.x      $vf5, $vf5, $vf5y
            vaddz.x      $vf5, $vf5, $vf5z
            vsqrt        Q, $vf5x
            vwaitq
            vaddq.x      $vf5, $vf0, Q
            vdiv         Q, $vf0w, $vf5x
            vsub.xyzw    $vf6, $vf0, $vf0
            vwaitq
            vmulq.xyz    $vf6, $vf4, Q
            sqc2         $vf6, 0(%1)
        " : : "r"(&value), "r"(&result) : "memory");
        return result;
    }

    NaVECTOR<T, t0> operator*(const T& s) const;

    NaVECTOR<T, t0> operator+(const NaVECTOR<T, t0>& rhs) const;

    NaVECTOR<T, t0> operator-(const NaVECTOR<T, t0>& rhs) const;

    NaVECTOR<T, t0> operator/(const T& s) const;

private:
    T v[t0];

public:
    static NaVECTOR<T, t0> ZERO;
    static NaVECTOR<T, t0> ZEROH;
};

template <typename T, int t0>
inline NaVECTOR<T, t0>& NaVECTOR<T, t0>::Copy(NaVECTOR<T, t0>& lhs, const NaVECTOR<T, t0>& rhs) {
    for (int i = 0; i < t0; i++) lhs.v[i] = rhs.v[i];
    return lhs;
}

// The 16-byte EE vector has a genuine MMI transfer specialization. Other
// dimensions use the shared typed implementation, without widening their copy.
template <>
inline NaVECTOR<float, 4>& NaVECTOR<float, 4>::Copy(NaVECTOR<float, 4>& lhs, const NaVECTOR<float, 4>& rhs) {
        asm volatile("
            lq $6, 0(%1)
            sq $6, 0(%0)
        " : : "r"(&lhs), "r"(&rhs)
        : "$6", "memory");
        return lhs;
    }

// Float4 arithmetic keeps the original VU operations and rounding. Other
// types and dimensions share ordinary typed arithmetic.
template <typename T, int t0>
inline NaVECTOR<T, t0> NaVECTOR<T, t0>::operator*(const T& s) const {
    NaVECTOR<T, t0> result;
    for (int i = 0; i < t0; i++) result.v[i] = v[i] * s;
    return result;
}

template <>
inline NaVECTOR<float, 4> NaVECTOR<float, 4>::operator*(const float& s) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            mfc1       $8, %2
            qmtc2.ni   $8, $vf5
            vmulx.xyzw $vf6, $vf4, $vf5x
            sqc2       $vf6, 0x0(%0)
        " : : "r"(&ret), "r"(this), "f"(s) : "$8", "memory");
        return ret;
    }

template <typename T, int t0>
inline NaVECTOR<T, t0> NaVECTOR<T, t0>::operator+(const NaVECTOR<T, t0>& rhs) const {
    NaVECTOR<T, t0> result;
    for (int i = 0; i < t0; i++) result.v[i] = v[i] + rhs.v[i];
    return result;
}

template <>
inline NaVECTOR<float, 4> NaVECTOR<float, 4>::operator+(const NaVECTOR<float, 4>& rhs) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            lqc2       $vf5, 0x0(%2)
            vadd.xyzw  $vf6, $vf4, $vf5
            sqc2       $vf6, 0x0(%0)
        " : : "r"(&ret), "r"(this), "r"(&rhs) : "memory");
        return ret;
    }

template <typename T, int t0>
inline NaVECTOR<T, t0> NaVECTOR<T, t0>::operator-(const NaVECTOR<T, t0>& rhs) const {
    NaVECTOR<T, t0> result;
    for (int i = 0; i < t0; i++) result.v[i] = v[i] - rhs.v[i];
    return result;
}

template <>
inline NaVECTOR<float, 4> NaVECTOR<float, 4>::operator-(const NaVECTOR<float, 4>& rhs) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            lqc2       $vf5, 0x0(%2)
            vsub.xyzw  $vf6, $vf4, $vf5
            sqc2       $vf6, 0x0(%0)
        " : : "r"(&ret), "r"(this), "r"(&rhs) : "memory");
        return ret;
    }

template <typename T, int t0>
inline NaVECTOR<T, t0> NaVECTOR<T, t0>::operator/(const T& s) const {
    NaVECTOR<T, t0> result;
    for (int i = 0; i < t0; i++) result.v[i] = v[i] / s;
    return result;
}

template <>
inline NaVECTOR<float, 4> NaVECTOR<float, 4>::operator/(const float& s) const {
        NaVECTOR<float, 4> ret;
        asm volatile("
            lqc2       $vf4, 0x0(%1)
            mfc1       $8, %2
            qmtc2.ni   $8, $vf5
            vdiv       Q, $vf0w, $vf5x
            vwaitq
            vmulq.xyzw $vf4, $vf4, Q
            sqc2       $vf4, 0x0(%0)
        " : : "r"(&ret), "r"(this), "f"(s) : "$8", "memory");
        return ret;
    }

/* Shared by vector constants and all four-component instances. */
template <typename T, int t0>
inline NaVECTOR<T, t0>::NaVECTOR(const T& x, const T& y, const T& z, const T& w) {
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
