// Native checks of the host float4 paths in nalib and of src/native/libvu0.c.
// Build: g++ -std=c++98 -O2 -Wall -Wextra -Isrc -Iinclude/rtl/ee -Iinclude/rtl/common
//        -x c src/native/libvu0.c -x c++ tests/native/vu0_math.cpp
#include "nalib/namatrix.h"

#include <float.h>
#include <math.h>
#include <stdio.h>

template <>
NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::IDENT(1.0f, 0.0f, 0.0f, 0.0f,
                                                   0.0f, 1.0f, 0.0f, 0.0f,
                                                   0.0f, 0.0f, 1.0f, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 1.0f);

static int failures = 0;

static void check(bool ok, const char *what) {
    if (!ok) {
        printf("FAIL: %s\n", what);
        failures++;
    }
}

static bool near(float a, float b, float eps = 1e-4f) {
    return fabsf(a - b) <= eps;
}

static void apply(float out[4], sceVu0FMATRIX m, const float v[4]) {
    for (int row = 0; row < 4; row++) {
        out[row] = m[0][row] * v[0] + m[1][row] * v[1] + m[2][row] * v[2] + m[3][row] * v[3];
    }
}

static void vectors() {
    NaVECTOR<float, 4> a(1.0f, 2.0f, 3.0f, 4.0f);
    NaVECTOR<float, 4> b(-2.0f, 0.5f, 4.0f, 1.0f);

    NaVECTOR<float, 4> sum = a + b;
    check(sum[0] == -1.0f && sum[3] == 5.0f, "float4 add");
    NaVECTOR<float, 4> scaled = a * 2.0f;
    check(scaled[2] == 6.0f, "float4 scale");

    // Division multiplies by the reciprocal, as VDIV + VMULQ do.
    NaVECTOR<float, 4> third = a / 3.0f;
    float q = 1.0f / 3.0f;
    check(third[0] == 1.0f * q && third[2] == 3.0f * q, "float4 divide by reciprocal");

    NaVECTOR<float, 4> cross = NaVECTOR<float, 4>::Cross3(a, b);
    check(cross[0] == 2.0f * 4.0f - 0.5f * 3.0f && cross[1] == 3.0f * -2.0f - 4.0f * 1.0f &&
          cross[2] == 1.0f * 0.5f - -2.0f * 2.0f && cross[3] == 0.0f, "Cross3");

    NaVECTOR<float, 4> unit = NaVECTOR<float, 4>::Normalize3(NaVECTOR<float, 4>(3.0f, 0.0f, 4.0f, 9.0f));
    check(near(unit[0], 0.6f) && near(unit[2], 0.8f) && unit[3] == 0.0f, "Normalize3");
    NaVECTOR<float, 4> zero = NaVECTOR<float, 4>::Normalize3(NaVECTOR<float, 4>(0.0f, 0.0f, 0.0f, 1.0f));
    check(zero[0] == 0.0f && zero[1] == 0.0f && zero[2] == 0.0f && zero[3] == 0.0f, "Normalize3 of zero is zero");
}

static void matrices() {
    NaMATRIX<float, 4, 4> m(1.0f, 2.0f, 3.0f, 0.0f,
                            0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 2.0f, 0.0f,
                            5.0f, 6.0f, 7.0f, 1.0f);
    NaVECTOR<float, 4> p(1.0f, 1.0f, 1.0f, 1.0f);
    NaVECTOR<float, 4> r;
    NaMATRIX<float, 4, 4>::Apply(r, m, p);
    check(r[0] == 6.0f && r[1] == 9.0f && r[2] == 12.0f && r[3] == 1.0f, "float4 Apply");

    // The VU product writes column by column, so the output may be the right operand.
    NaMATRIX<float, 4, 4> product = m;
    NaMATRIX<float, 4, 4>::Multiply(product, m, product);
    NaMATRIX<float, 4, 4> expected;
    NaMATRIX<float, 4, 4> copy = m;
    NaMATRIX<float, 4, 4>::Multiply(expected, m, copy);
    bool same = true;
    for (int c = 0; c < 4; c++) for (int row = 0; row < 4; row++) same = same && product[c][row] == expected[c][row];
    check(same, "float4 Multiply into its right operand");
}

static void sdk() {
    // Rigid transform: rotate 90 degrees about z, then translate.
    sceVu0FMATRIX m = { { 0, 1, 0, 0 }, { -1, 0, 0, 0 }, { 0, 0, 1, 0 }, { 3, 4, 5, 1 } };
    sceVu0FMATRIX inv, ident;
    sceVu0InversMatrix(inv, m);
    sceVu0MulMatrix(ident, inv, m);
    bool unit = true;
    for (int c = 0; c < 4; c++) for (int row = 0; row < 4; row++) unit = unit && near(ident[c][row], c == row ? 1.0f : 0.0f);
    check(unit, "InversMatrix undoes a rigid transform");

    sceVu0FVECTOR eye = { 10, 0, 0, 1 }, zd = { -1, 0, 0, 0 }, yd = { 0, 1, 0, 0 };
    sceVu0FMATRIX camera;
    sceVu0CameraMatrix(camera, eye, zd, yd);
    float origin[4], ahead[4];
    float target[4] = { 0, 0, 0, 1 };
    apply(origin, camera, eye);
    apply(ahead, camera, target);
    check(near(origin[0], 0) && near(origin[1], 0) && near(origin[2], 0), "CameraMatrix puts the eye at the origin");
    check(near(ahead[0], 0) && near(ahead[1], 0) && near(ahead[2], 10), "CameraMatrix looks along zd");

    sceVu0FMATRIX screen;
    sceVu0ViewScreenMatrix(screen, 512.0f, 1.0f, 0.47f, 2048.0f, 2048.0f, 1.0f, 16777215.0f, 1.0f, 65536.0f);
    float nearPoint[4] = { 1, 1, 1, 1 }, farPoint[4] = { 0, 0, 65536, 1 }, out[4];
    apply(out, screen, nearPoint);
    check(near(out[0] / out[3], 2048.0f + 512.0f) && near(out[1] / out[3], 2048.0f + 0.47f * 512.0f), "ViewScreenMatrix x/y");
    check(near(out[2] / out[3], 16777215.0f, 2.0f), "ViewScreenMatrix maps nearz to zmax");
    apply(out, screen, farPoint);
    check(near(out[2] / out[3], 1.0f, 1e-2f), "ViewScreenMatrix maps farz to zmin");
}

int main() {
    vectors();
    matrices();
    sdk();
    if (failures == 0) {
        printf("PASS: native VU0 math\n");
    }
    return failures != 0;
}
