#include "nalib/namatrix.h"

#include <math.h>

// Native checks that the axis-angle RotateMatrix is the right-handed rotation
// by +angle about the normalised axis (Rodrigues' formula), stored as columns,
// and that the single-axis overload agrees with it.

static bool near(float a, float b) {
    return fabsf(a - b) < 1e-5f;
}

static bool matchesRodrigues(const NaVECTOR<float, 4>& axis, float angle) {
    const NaMATRIX<float, 4, 4> m = NaMATRIX<float, 4, 4>::RotateMatrix(axis, angle);
    const float length = sqrtf(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    const float u[3] = { axis[0] / length, axis[1] / length, axis[2] / length };
    const float s = sinf(angle);
    const float c = cosf(angle);
    const float k[3][3] = {
        { 0.0f, -u[2], u[1] },
        { u[2], 0.0f, -u[0] },
        { -u[1], u[0], 0.0f },
    };

    for (int row = 0; row < 3; row++) {
        for (int column = 0; column < 3; column++) {
            float kk = 0.0f;
            for (int i = 0; i < 3; i++) kk += k[row][i] * k[i][column];
            const float expected = (row == column ? 1.0f : 0.0f) + s * k[row][column] + (1.0f - c) * kk;
            // m[column][row]: columns are stored as vectors.
            if (!near(m[column][row], expected)) return false;
        }
        if (m[3][row] != 0.0f || m[row][3] != 0.0f) return false;
    }
    return m[3][3] == 1.0f;
}

int main() {
    const float axes[][3] = {
        { 1.0f, 0.0f, 0.0f }, { -3.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }, { 1.0f, 2.0f, 3.0f }, { -2.0f, 0.5f, 1.0f },
        { 0.3f, -0.8f, -0.1f },
    };
    const float angles[] = { 0.3f, 1.2f, -2.0f, 3.0f };

    for (unsigned a = 0; a < sizeof(axes) / sizeof(axes[0]); a++) {
        const NaVECTOR<float, 4> axis(axes[a][0], axes[a][1], axes[a][2], 0.0f);
        for (unsigned i = 0; i < sizeof(angles) / sizeof(angles[0]); i++) {
            if (!matchesRodrigues(axis, angles[i])) return 1 + a;
        }
    }

    for (int axisIndex = 0; axisIndex < 3; axisIndex++) {
        const NaVECTOR<float, 4> axis(axisIndex == 0 ? 1.0f : 0.0f,
                                      axisIndex == 1 ? 1.0f : 0.0f,
                                      axisIndex == 2 ? 1.0f : 0.0f, 0.0f);
        const NaMATRIX<float, 4, 4> byIndex = NaMATRIX<float, 4, 4>::RotateMatrix(axisIndex, 0.7f);
        const NaMATRIX<float, 4, 4> byAxis = NaMATRIX<float, 4, 4>::RotateMatrix(axis, 0.7f);
        for (int column = 0; column < 4; column++) {
            for (int row = 0; row < 4; row++) {
                if (!near(byIndex[column][row], byAxis[column][row])) return 20 + axisIndex;
            }
        }
    }

    return 0;
}
