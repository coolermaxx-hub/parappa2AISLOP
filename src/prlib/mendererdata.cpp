#include "common.h"

#include "menderer.h"

#include "random.h"

#include "nalib/namatrix.h"

#include <math.h>

struct PrNoodlePositionData {
    float index;
    float position[3];
    float velocity[3];
    u_int timer;
};


extern PrNoodlePositionData noodlePositionData[115];
extern u_int noodlePolygonIndex[116];

void SetNextTarget(PrNoodlePositionData *data);
void UpdateNoodlePositionData(PrNoodlePositionData *data);
float GetSynchronizeRatio(const PrNoodlePositionData *data);
void InitializeNoodlePositionData();

namespace {
float Absolute(float x) {
    return (x >= 0.0f) ? x : -x;
}

float Maximum(float a, float b) {
    return (a <= b) ? b : a;
}
}

void SetNextTarget(PrNoodlePositionData *data) {
    float x = (PrFloatRandom() - 0.5f) * 2.0f * 0.1f;
    float y = (PrFloatRandom() - 0.5f) * 2.0f * 0.04f;
    float z = (PrFloatRandom() - 0.5f) * 2.0f * 0.1f;

    float dx = x - data->position[0];

    float tx = Absolute(dx) / 0.2f * 180.0f;
    float ty = Absolute(y - data->position[1]) / 0.08f * 180.0f;
    float tz = Absolute(z - data->position[2]) / 0.2f * 180.0f;

    float txy = Maximum(ty, tx);
    float t = Maximum(Maximum(1.0f, tz), txy);

    data->timer = static_cast<u_int>(t);
    data->velocity[0] = dx / data->timer;
    data->velocity[1] = (y - data->position[1]) / data->timer;
    data->velocity[2] = (z - data->position[2]) / data->timer;
}

void InitializeNoodlePositionData() {
    PrNoodlePositionData *data = noodlePositionData;

    for (u_int i = 0; i < 115; i++, data++) {
        data->position[0] = (PrFloatRandom() - 0.5f) * 2.0f * 0.1f;
        data->position[1] = (PrFloatRandom() - 0.5f) * 2.0f * 0.04f;
        data->position[2] = (PrFloatRandom() - 0.5f) * 2.0f * 0.1f;
        data->index = ((i * 2) % 115) / 115.0f;
        SetNextTarget(data);
    }
}

void UpdateNoodlePositionData(PrNoodlePositionData *data) {
    if (--data->timer == (u_int)-1) {
        SetNextTarget(data);
    }

    data->position[0] += prMendererSpeed * data->velocity[0];
    data->position[1] += prMendererSpeed * data->velocity[1];
    data->position[2] += prMendererSpeed * data->velocity[2];
}

namespace {
float Minimum(float a, float b) {
    return (a <= b) ? a : b;
}
}

float GetSynchronizeRatio(const PrNoodlePositionData *data) {
    float diff = Absolute(data->index - prSchoolLeaderIndex);
    float dist = Minimum(diff, 1.0f - diff);

    diff = 1.0f - dist;
    float ratio = diff * 3.0f - 1.8f;
    ratio *= prMendererSyncRatio;
    if (ratio > 1.0f) {
        ratio = 1.0f;
    } else if (ratio < 0.0f) {
        ratio = 0.0f;
    }

    if (prMendererRatio >= 1.2f && prMendererRatio <= 1.8f) {
        float blend = Absolute(1.5f - prMendererRatio) / 0.3f;
        ratio = ratio * blend + (1.0f - blend);
    }

    return ratio;
}

void PrInitializeNoodlePolygonPosition() {
    for (u_int k = 0; k < 115; k++) {
        noodlePolygonIndex[k] = k;
    }

    for (u_int i = 0; i < 114; i++) {
        u_int j = PrRandom() % (115 - i);
        if (j != i) {
            u_int tmp = noodlePolygonIndex[i];
            noodlePolygonIndex[i] = noodlePolygonIndex[j];
            noodlePolygonIndex[j] = tmp;
        }
    }

    InitializeNoodlePositionData();
}

void PrGetNoodlePolygonPosition(NaVECTOR<float, 4> *pos, u_int index) {
    u_int n = noodlePolygonIndex[index];
    PrNoodlePositionData *data = &noodlePositionData[n];

    UpdateNoodlePositionData(data);
    float sync = 1.0f - GetSynchronizeRatio(data);

    float ratio = prMendererRatio - 0.4f;
    float y = data->position[1] * sync;
    float x = data->position[0] * sync * Maximum(ratio, 1.0f);
    float z = data->position[2] * sync;

    float angle = (float)n * 6.2831855f / 115.0f + z;
    float c = cosf(angle);
    float s = sinf(angle);
    NaMATRIX<float, 2, 2> rot(c, s, -s, c);

    float distance = prMendererDistance + x;
    float width = prMendererWidth;
    float length = prMendererLength;

    NaVECTOR<float, 2> p0 = rot.ApplyTransposed(NaVECTOR<float, 2>(distance, y + width * 0.5f * 0.093756f));
    NaVECTOR<float, 2> p1 = rot.ApplyTransposed(NaVECTOR<float, 2>(distance + length * 0.5f, y + width * 0.5f * 0.093756f));
    NaVECTOR<float, 2> p2 = rot.ApplyTransposed(NaVECTOR<float, 2>(distance, y + -width * 0.5f * 0.093756f));
    NaVECTOR<float, 2> p3 = rot.ApplyTransposed(NaVECTOR<float, 2>(distance + length * 0.5f, y + -width * 0.5f * 0.093756f));

    pos[0].Set(p0[0], p0[1], 0.0f, 1.0f);
    pos[1].Set(p1[0], p1[1], 0.0f, 1.0f);
    pos[2].Set(p2[0], p2[1], 0.0f, 1.0f);
    pos[3].Set(p3[0], p3[1], 0.0f, 1.0f);
}
