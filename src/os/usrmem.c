#include "os/usrmem.h"

#include <stdio.h>

/* I'm not sure if it actually was 128bit aligned (minimum is 32bit),
 * but 128bit makes more sense in this case. */
static char usrMemoryData[0x1880000] PR_ALIGNED(128); /* ~25 MB */

/*
 * A two-ended stack allocator over usrMemoryData. usrMemPos holds block
 * boundaries: front blocks grow up from usrMemPos[0], end blocks grow down
 * from the last slot. usrMemPos[n] is the start of front block n, and
 * USRMEM_END_POS(n) the bottom of end block n (end block 0 is the top of
 * the arena). Blocks are rounded up to 16 bytes.
 */
#define USRMEM_POS_MAX    2048
#define USRMEM_END_POS(n) usrMemPos[USRMEM_POS_MAX - 1 - (n)]

static u_int usrMemPos[USRMEM_POS_MAX];

static int usrMemPosCnt;
static int usrMemPosEndCnt;

void UsrMemClear(void) {
    usrMemPos[0] = (u_int)usrMemoryData;
    USRMEM_END_POS(0) = (u_int)&usrMemoryData[sizeof(usrMemoryData)];

    usrMemPosCnt = 0;
    usrMemPosEndCnt = 0;
}

void UsrMemClearTop(void) {
    usrMemPosCnt = 0;
    usrMemPos[0] = (u_int)usrMemoryData;
}

void UsrMemClearEnd(void) {
    usrMemPosEndCnt = 0;
    USRMEM_END_POS(0) = (u_int)&usrMemoryData[sizeof(usrMemoryData)];
}

u_int UsrMemGetAdr(int id) {
    if (id >= usrMemPosCnt) {
        printf("UsrMemGetAdr ID over[%d]\n", id);
        return 0;
    }

    return usrMemPos[id];
}

u_int UsrMemGetSize(int id) {
    if (id >= usrMemPosCnt) {
        printf("UsrMemGetSize ID over[%d]\n", id);
        return 0;
    }

    return usrMemPos[id + 1] - usrMemPos[id];
}

/* Unlike UsrMemGetAdr, this returns the top of end block id + 1, not its
 * start. Nothing calls the end-block getters. */
u_int UsrMemGetEndAdr(int id) {
    if (id >= usrMemPosEndCnt) {
        printf("UsrMemGetEndAdr ID over[%d]\n", id);
        return 0;
    }

    return USRMEM_END_POS(id);
}

u_int UsrMemGetEndSize(int id) {
    if (id >= usrMemPosEndCnt) {
        printf("UsrMemGetEndSize ID over[%d]\n", id);
        return 0;
    }

    return USRMEM_END_POS(id) - USRMEM_END_POS(id + 1);
}

u_int UsrMemAllocNext(void) {
    return usrMemPos[usrMemPosCnt];
}

u_int UsrMemAllocEndNext(void) {
    return USRMEM_END_POS(usrMemPosEndCnt);
}

u_int UsrMemAlloc(int size) {
    u_int ret = UsrMemAllocNext();

    usrMemPosCnt++;
    usrMemPos[usrMemPosCnt] = ret + ((size + 15) / 16) * 16;

    if (usrMemPos[usrMemPosCnt] > UsrMemAllocEndNext()) {
        printf("UsrMemAlloc size over [0x%08x]\n", usrMemPos[usrMemPosCnt] - UsrMemAllocEndNext());
        usrMemPosCnt--;
        return NULL;
    }

    return ret;
}

u_int UsrMemEndAlloc(int size) {
    u_int ret = UsrMemAllocEndNext() - ((size + 15) / 16) * 16;

    if (ret < UsrMemAllocNext()) {
        printf("UsrMemEndAlloc size over [0x%08x]\n", UsrMemAllocNext() - ret);
        return NULL;
    }

    usrMemPosEndCnt++;
    USRMEM_END_POS(usrMemPosEndCnt) = ret;
    return ret;
}

void UsrMemFree(void) {
    if (usrMemPosCnt != 0) {
        usrMemPosCnt--;
    }
}

void UsrMemEndFree(void) {
    if (usrMemPosEndCnt != 0) {
        usrMemPosEndCnt--;
    }
}
