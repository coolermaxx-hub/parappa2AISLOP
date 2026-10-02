#ifndef NALIB_NAVECTOR_H
#define NALIB_NAVECTOR_H

template <typename T, int t0>
class NaVECTOR {
public:
    NaVECTOR() {}

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

private:
    T v[t0];
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
