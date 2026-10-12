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
    /*
     * FIXME(poly):
     *   Was a goto actually used here?
     *   I can't get it to match without one. 
     */
generate_rand:
    float ret = PrRandom();

    if ((ret / RAND_MAX) >= 1.0f) {
        goto generate_rand;
    }

    return ret / RAND_MAX;
}
