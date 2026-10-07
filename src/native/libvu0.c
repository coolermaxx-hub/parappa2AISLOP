/*
 * Native (host) versions of the libvu0 routines the engine calls. The PS2
 * build links Sony's library instead; this file is never compiled for the EE.
 *
 * Each routine follows the instruction sequence of the copy linked into
 * SCPS_150.17 (asm/sdk/libvu0.s): the same operations in the same order,
 * including which w components are cleared or kept. Host floats round to
 * nearest and keep denormals, infinities and NaNs, where the VU truncates and
 * clamps, so results can differ in the last bit and on out-of-range input.
 */
#include <float.h>
#include <math.h>
#include <string.h>

#include <libvu0.h>

void sceVu0UnitMatrix(sceVu0FMATRIX m) {
    memset(m, 0, sizeof(sceVu0FMATRIX));
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

/* m0 = m1 * m2, one column of m2 at a time; m0 may alias m2. */
void sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FMATRIX m2) {
    sceVu0FMATRIX lhs;
    int column, row;

    memcpy(lhs, m1, sizeof(lhs));
    for (column = 0; column < 4; column++) {
        sceVu0FVECTOR v;
        memcpy(v, m2[column], sizeof(v));
        for (row = 0; row < 4; row++) {
            /* VMULAX, VMADDAY, VMADDAZ, VMADDW */
            m0[column][row] = ((lhs[0][row] * v[0] + lhs[1][row] * v[1]) + lhs[2][row] * v[2]) + lhs[3][row] * v[3];
        }
    }
}

/* v0 = v1 x v2 (VOPMULA/VOPMSUB), w cleared. */
void sceVu0OuterProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2) {
    float x = v1[1] * v2[2] - v2[1] * v1[2];
    float y = v1[2] * v2[0] - v2[2] * v1[0];
    float z = v1[0] * v2[1] - v2[0] * v1[1];

    v0[0] = x;
    v0[1] = y;
    v0[2] = z;
    v0[3] = 0.0f;
}

/* v0 = v1.xyz / |v1.xyz|, w cleared. VDIV by zero gives the largest float. */
void sceVu0Normalize(sceVu0FVECTOR v0, sceVu0FVECTOR v1) {
    float length = sqrtf((v1[0] * v1[0] + v1[1] * v1[1]) + v1[2] * v1[2]);
    float q = (length != 0.0f) ? 1.0f / length : FLT_MAX;
    float x = v1[0] * q;
    float y = v1[1] * q;
    float z = v1[2] * q;

    v0[0] = x;
    v0[1] = y;
    v0[2] = z;
    v0[3] = 0.0f;
}

/* m0 = m1 with tv.xyz added to its translation column. */
void sceVu0TransMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR tv) {
    sceVu0FVECTOR translation;

    translation[0] = m1[3][0] + tv[0];
    translation[1] = m1[3][1] + tv[1];
    translation[2] = m1[3][2] + tv[2];
    translation[3] = m1[3][3];
    memmove(m0, m1, sizeof(sceVu0FVECTOR) * 3);
    memcpy(m0[3], translation, sizeof(translation));
}

/*
 * Rigid inverse: the 3x3 part is transposed (with zero w) and the translation
 * becomes -(R^T t), keeping the original translation's w. Scale and shear are
 * not undone.
 */
void sceVu0InversMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1) {
    sceVu0FMATRIX src;
    int i;

    memcpy(src, m1, sizeof(src));
    for (i = 0; i < 3; i++) {
        m0[i][0] = src[0][i];
        m0[i][1] = src[1][i];
        m0[i][2] = src[2][i];
        m0[i][3] = 0.0f;
    }
    for (i = 0; i < 3; i++) {
        /* VMULAX, VMADDAY, VMADDZ, then VSUB from zero */
        m0[3][i] = 0.0f - ((m0[0][i] * src[3][0] + m0[1][i] * src[3][1]) + m0[2][i] * src[3][2]);
    }
    m0[3][3] = src[3][3];
}

/* World-to-camera matrix for an eye at p looking along zd with up yd. */
void sceVu0CameraMatrix(sceVu0FMATRIX m, sceVu0FVECTOR p, sceVu0FVECTOR zd, sceVu0FVECTOR yd) {
    sceVu0FMATRIX m0;
    sceVu0FVECTOR xd;

    sceVu0UnitMatrix(m0);
    sceVu0OuterProduct(xd, yd, zd);
    sceVu0Normalize(m0[0], xd);
    sceVu0Normalize(m0[2], zd);
    sceVu0OuterProduct(m0[1], m0[2], m0[0]);
    sceVu0TransMatrix(m0, m0, p);
    sceVu0InversMatrix(m, m0);
}

/* Camera-to-screen projection; depth maps nearz..farz onto zmax..zmin. */
void sceVu0ViewScreenMatrix(sceVu0FMATRIX m, float scrz, float ax, float ay,
                            float cx, float cy, float zmin, float zmax, float nearz, float farz) {
    sceVu0FMATRIX mt;
    float az = (farz * nearz * (-zmin + zmax)) / (-nearz + farz);
    float cz = (-zmax * nearz + zmin * farz) / (-nearz + farz);

    sceVu0UnitMatrix(m);
    m[0][0] = scrz;
    m[1][1] = scrz;
    m[2][2] = 0.0f;
    m[3][3] = 0.0f;
    m[2][3] = 1.0f;
    m[3][2] = 1.0f;

    sceVu0UnitMatrix(mt);
    mt[0][0] = ax;
    mt[1][1] = ay;
    mt[2][2] = az;
    mt[3][0] = cx;
    mt[3][1] = cy;
    mt[3][2] = cz;

    sceVu0MulMatrix(m, mt, m);
}
