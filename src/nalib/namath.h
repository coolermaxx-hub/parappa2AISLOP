#ifndef NALIB_NAMATH_H
#define NALIB_NAMATH_H

/* Small scalar helpers. Plain comparisons, so no libm calls are made. */

template <typename T>
inline T NaAbs(T x) {
    return (x >= 0) ? x : -x;
}

template <typename T>
inline T NaMax(T a, T b) {
    return (a <= b) ? b : a;
}

template <typename T>
inline T NaMin(T a, T b) {
    return (a <= b) ? a : b;
}

#endif /* NALIB_NAMATH_H */
