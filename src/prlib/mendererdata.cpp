#include "common.h"

#include "random.h"

struct PrNoodlePositionData {
    float index;
    float position[3];
    float velocity[3];
    u_int timer;
};

extern float prMendererSpeed;
extern float prMendererRatio;
extern float prMendererSyncRatio;
extern float prSchoolLeaderIndex;

extern PrNoodlePositionData noodlePositionData[115];
extern u_int noodlePolygonIndex[116];

void SetNextTarget(PrNoodlePositionData *data);
void InitializeNoodlePositionData();

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", SetNextTarget__FP20PrNoodlePositionData);
#else /* Requires .lit4 migration */
void SetNextTarget(PrNoodlePositionData *data) {
    float x = (PrFloatRandom() - 0.5f) * 2.0f * 0.1f;
    float y = (PrFloatRandom() - 0.5f) * 2.0f * 0.04f;
    float z = (PrFloatRandom() - 0.5f) * 2.0f * 0.1f;

    float dx = x - data->position[0];
    float d;

    d = dx;
    if (d < 0.0f) {
        d = -d;
    }
    float tx = d / 0.2f * 180.0f;

    d = y - data->position[1];
    if (d < 0.0f) {
        d = -d;
    }
    float ty = d / 0.08f * 180.0f;

    d = z - data->position[2];
    if (d < 0.0f) {
        d = -d;
    }
    float tz = d / 0.2f * 180.0f;

    float txy = (ty <= tx) ? tx : ty;
    float t = 1.0f;
    if (t <= tz) {
        t = tz;
    }
    if (t <= txy) {
        t = txy;
    }

    data->timer = t;
    data->velocity[0] = dx / data->timer;
    data->velocity[1] = (y - data->position[1]) / data->timer;
    data->velocity[2] = (z - data->position[2]) / data->timer;
}
#endif

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

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", GetSynchronizeRatio__FPC20PrNoodlePositionData);
#else /* Requires .lit4 migration */
float GetSynchronizeRatio(const PrNoodlePositionData *data) {
    float diff = data->index - prSchoolLeaderIndex;
    if (diff < 0.0f) {
        diff = -diff;
    }

    float dist = 1.0f - diff;
    if (diff <= dist) {
        dist = diff;
    }

    float ratio = ((1.0f - dist) * 3.0f - 1.8f) * prMendererSyncRatio;
    if (ratio > 1.0f) {
        ratio = 1.0f;
    } else if (ratio < 0.0f) {
        ratio = 0.0f;
    }

    if (prMendererRatio >= 1.2f && prMendererRatio <= 1.8f) {
        float blend = 1.5f - prMendererRatio;
        if (blend < 0.0f) {
            blend = -blend;
        }
        blend /= 0.3f;
        ratio = ratio * blend + (1.0f - blend);
    }

    return ratio;
}
#endif

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

INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", PrGetNoodlePolygonPosition__FPt8NaVECTOR2Zfi4Ui);

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", func_00151D78);

INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", func_00151DA0);

INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", func_00151DF8);
