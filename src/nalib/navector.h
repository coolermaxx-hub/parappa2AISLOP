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
    NaVECTOR(const T& x, const T& y, const T& z, const T& w) {
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3] = w;
    }
    NaVECTOR(const NaVECTOR<float, 4>& rhs) {
        Copy(*this, rhs);
    }

public:
    T operator[](int arg0) const {
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
NaVECTOR<T, t0>& NaVECTOR<T, t0>::Set(const T& x, const T& y, const T& z, const T& w) {
    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;
    return *this;
}

#endif /* NALIB_NAVECTOR_H */
