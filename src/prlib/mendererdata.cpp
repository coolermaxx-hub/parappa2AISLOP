#include "common.h"

#include "random.h"

#include "nalib/namath.h"
#include "nalib/namatrix.h"

#include <math.h>

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
extern float prMendererDistance;
extern float prMendererWidth;
extern float prMendererLength;

extern PrNoodlePositionData noodlePositionData[115];
extern u_int noodlePolygonIndex[116];

void SetNextTarget(PrNoodlePositionData *data);
void UpdateNoodlePositionData(PrNoodlePositionData *data);
float GetSynchronizeRatio(const PrNoodlePositionData *data);
void InitializeNoodlePositionData();

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", SetNextTarget__FP20PrNoodlePositionData);
#else /* Regalloc: the first abs copies dx before comparing */
void SetNextTarget(PrNoodlePositionData *data) {
    float x = (PrFloatRandom() - 0.5f) * 2.0f * 0.1f;
    float y = (PrFloatRandom() - 0.5f) * 2.0f * 0.04f;
    float z = (PrFloatRandom() - 0.5f) * 2.0f * 0.1f;

    float dx = x - data->position[0];

    float tx = NaAbs(dx) / 0.2f * 180.0f;
    float ty = NaAbs(y - data->position[1]) / 0.08f * 180.0f;
    float tz = NaAbs(z - data->position[2]) / 0.2f * 180.0f;

    float txy = NaMax(ty, tx);
    float t = NaMax(NaMax(1.0f, tz), txy);

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

float GetSynchronizeRatio(const PrNoodlePositionData *data) {
    float diff = NaAbs(data->index - prSchoolLeaderIndex);
    float dist = NaMin(diff, 1.0f - diff);

    diff = 1.0f - dist;
    float ratio = diff * 3.0f - 1.8f;
    ratio *= prMendererSyncRatio;
    if (ratio > 1.0f) {
        ratio = 1.0f;
    } else if (ratio < 0.0f) {
        ratio = 0.0f;
    }

    if (prMendererRatio >= 1.2f && prMendererRatio <= 1.8f) {
        float blend = NaAbs(1.5f - prMendererRatio) / 0.3f;
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

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", PrGetNoodlePolygonPosition__FPt8NaVECTOR2Zfi4Ui);
#else /* First draft: the 2x2 rotate loops and Set argument setup still differ */
void PrGetNoodlePolygonPosition(NaVECTOR<float, 4> *pos, u_int index) {
    u_int n = noodlePolygonIndex[index];
    PrNoodlePositionData *data = &noodlePositionData[n];

    UpdateNoodlePositionData(data);
    float sync = 1.0f - GetSynchronizeRatio(data);

    float ratio = prMendererRatio - 0.4f;
    float y = data->position[1] * sync;
    float x = data->position[0] * sync * NaMax(ratio, 1.0f);
    float z = data->position[2] * sync;

    float angle = (float)n * 6.2831855f / 115.0f + z;
    float c = cosf(angle);
    float s = sinf(angle);
    NaMATRIX<float, 2, 2> rot(c, s, -s, c);

    float distance = prMendererDistance + x;
    float width = prMendererWidth;
    float length = prMendererLength;

    NaVECTOR<float, 2> p0 = rot.Transform(NaVECTOR<float, 2>(distance, y + width * 0.5f * 0.093756f));
    NaVECTOR<float, 2> p1 = rot.Transform(NaVECTOR<float, 2>(distance + length * 0.5f, y + width * 0.5f * 0.093756f));
    NaVECTOR<float, 2> p2 = rot.Transform(NaVECTOR<float, 2>(distance, y + -width * 0.5f * 0.093756f));
    NaVECTOR<float, 2> p3 = rot.Transform(NaVECTOR<float, 2>(distance + length * 0.5f, y + -width * 0.5f * 0.093756f));

    pos[0].Set(p0[0], p0[1], 0.0f, 1.0f);
    pos[1].Set(p1[0], p1[1], 0.0f, 1.0f);
    pos[2].Set(p2[0], p2[1], 0.0f, 1.0f);
    pos[3].Set(p3[0], p3[1], 0.0f, 1.0f);
}
#endif

/* nalib/namatrix.h, nalib/navector.h: weak copies of the 4- and 9-argument 2x2 Set and of NaVECTOR::Set */
#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", func_00151D78);

INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", func_00151DA0);

INCLUDE_ASM("asm/nonmatchings/prlib/mendererdata", func_00151DF8);
#endif
