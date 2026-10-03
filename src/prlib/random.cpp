#include "random.h"

#include <stdlib.h>

static u_int randomSeed = 1;
static u_int randomPool[97];

static u_int RawRandom() {
    randomSeed = (randomSeed * 0x5d588b65 + 1);
    return randomSeed;
}

u_int PrRandom() {
    static u_int poolIndex = 0;
    int ret;

    poolIndex = randomPool[poolIndex] % 97;
    ret = randomPool[poolIndex] / 2;

    randomPool[poolIndex] = RawRandom();
    return ret;
}

void PrInitializeRandomPool() {
    for (u_int i = 0; i < PR_ARRAYSIZE(randomPool); i++) {
        randomPool[i] = RawRandom();
    }

    PrRandom();
    PrRandom();
}

float PrFloatRandom() {
    // Rejection sampling: reroll until the scaled value is strictly below 1.0.
    float scaled;
    do {
        scaled = static_cast<float>(PrRandom()) / RAND_MAX;
    } while (scaled >= 1.0f);

    return scaled;
}
