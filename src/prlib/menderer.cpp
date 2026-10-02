#include "common.h"

#if defined(PRD_SYORI)
#include "dbug/syori.h"
#endif

#include "mfifo.h"
#include "renderstuff.h"
#include "spram.h"
#include "utility.h"

#include <eeregs.h>
#include <eestruct.h>

/* sdata */
extern float prMendererRatio;
extern int prMendererGettingWorse;
extern int prMendererColorModulation;
extern float prMendererSpeed;
extern float prMendererFade;
extern int deceleratingMenderer;
extern float decelerateRatio;

extern int prCurrentStage;

struct PrNoodleStripPacket {
    sceDmaTag dmatag;
    sceGifTag giftag;
    u_long ad[14][2];
};

/* data */
extern PrNoodleStripPacket noodleStripDmaPacket;

/* sdata */
extern PrSPRAM_DATA *prSpramData_tmp_menderer;
extern u_int noodleRandomSeed;

extern u_int prMendererDrawFbp;
extern u_int prMendererWorkFbp;
extern u_int prMendererTbp;
extern float prMendererDistance;
extern float prMendererWidth;
extern float prMendererLength;
extern float noodleRotation;
extern int mendererInitialized;

/* sbss */
extern int noodleStatus;
extern int noodleChangeTimer;
extern float noodleDeltaRotation;

void PrInitializeTextureCreation(u_int tbp, u_int zbp, u_int tw, u_int th);
void PrInitializeAlphaModulation();
void PrInitializeAwfulBackground(void *tim2);
void PrInitializeNoodlePolygonPosition();
void DrawNoodleStripChunk(const NaMATRIX<float, 4, 4>& matrix);
void SetNoodleRotationMatrix(NaMATRIX<float, 4, 4>& matrix, float rot);
void StartNoodleRotation();

void InitializeNoodleStripRendering(u_int tbp, u_int fbp, u_int tw, u_int th) {
    noodleStripDmaPacket.ad[4][0] = SCE_GS_SET_FRAME(fbp, 10, 0, 0);
    noodleStripDmaPacket.ad[8][0] = SCE_GS_SET_TEX0(tbp, 4, 0, tw, th, 1, 0, 0, 0, 0, 0, 0);
}

static float GetRandom() {
    u_int seed = noodleRandomSeed * 0x19660D + 0x3C6EF35F;
    float ret = ((seed >> 8) & 0xFFFF) * (1.0f / 65536.0f);
    noodleRandomSeed = seed;
    return ret;
}

static int StageIndexForColor() {
    if (prCurrentStage == 19) {
        return 0;
    }
    return (u_int)prCurrentStage % 10;
}

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", PushNoodleColor__FPUl);

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", DrawNoodleStripChunk__FRCt8NaMATRIX3Zfi4i4);

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", SetNoodleRotationMatrix__FRt8NaMATRIX3Zfi4i4f);

static void PreDrawNoodleStrip() {
    u_long128 *buf = prSpramData_tmp_menderer->m_noodle_buffer[0];
    prSpramData_tmp_menderer->m_noodle_buffer[0] = prSpramData_tmp_menderer->m_noodle_buffer[1];
    prSpramData_tmp_menderer->m_noodle_buffer[1] = prSpramData_tmp_menderer->m_noodle_buffer[2];
    prSpramData_tmp_menderer->m_noodle_buffer[2] = buf;

    PrNoodleStripPacket *packet = (PrNoodleStripPacket*)buf;
    *packet = noodleStripDmaPacket;
    packet->ad[0][0] = SCE_GS_SET_BITBLTBUF(prMendererDrawFbp * 32, 10, 0, prMendererWorkFbp * 32, 10, 0);

    PrSendMfifo(&packet->dmatag);
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", DrawNoodleStrip__Fff);
#else /* Requires .lit4 migration */
void DrawNoodleStrip(float ratio, float rot) {
    PreDrawNoodleStrip();

    NaMATRIX<float, 4, 4> matrix;
    SetNoodleRotationMatrix(matrix, rot);

    float width, length, distance;
    if (ratio <= 1.0f) {
        distance = ratio * -0.2854f + 0.6054f;
        length = 0.9375f;
        width = 0.7f;
    } else if (ratio <= 2.0f) {
        float t = ratio - 1.0f;
        length = t * 0.6641f + 0.9375f;
        width = t * 0.3f + 0.7f;
        distance = t * -0.3786f + 0.32f;
    } else {
        float t = ratio - 2.0f;
        length = t * -0.664f + 1.6016f;
        width = t * -0.3f + 1.0f;
        distance = t * 0.5086f + -0.0586f;
    }

    prMendererWidth = width;
    prMendererDistance = distance;
    prMendererLength = length;

    DrawNoodleStripChunk(matrix);
}
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", StartNoodleRotation__Fv);
#else /* Requires .lit4 migration */
void StartNoodleRotation() {
    noodleChangeTimer = 900;
    noodleDeltaRotation = 0.001f;
    noodleStatus = 0;
    noodleRotation = 0.0f;
}
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", UpdateNoodleRotation__Fv);
#else /* Requires .lit4 migration */
void UpdateNoodleRotation() {
    if (prCurrentStage == 6) {
        noodleRotation += prMendererSpeed * 0.001f;
        return;
    }

    if (noodleStatus == 0 || noodleStatus == 2) {
        if (--noodleChangeTimer == 0) {
            noodleChangeTimer = 900;
            noodleStatus = (noodleStatus + 1) % 4;
        }
    }

    if (noodleStatus == 1) {
        noodleDeltaRotation -= 1.6666667e-05f;
        if (noodleDeltaRotation < -0.001f) {
            noodleDeltaRotation = -0.001f;
            noodleStatus = 2;
        }
    } else if (noodleStatus == 3) {
        noodleDeltaRotation += 1.6666667e-05f;
        if (noodleDeltaRotation > 0.001f) {
            noodleDeltaRotation = 0.001f;
            noodleStatus = 0;
        }
    }

    noodleRotation += prMendererSpeed * noodleDeltaRotation;
}
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", PrUpdateMendererSpeed__Fv);
#else
static void PrUpdateMendererSpeed() {
    if (deceleratingMenderer == 0) {
        return;
    }

    if (prCurrentStage == 6 || prCurrentStage == 16) {
        float fade = prMendererFade - decelerateRatio;
        if (fade < 0.0f) {
            prMendererFade = 0.0f;
        } else {
            prMendererFade = fade;
        }
    } else {
        float speed = prMendererSpeed - decelerateRatio;
        if (speed < 0.0f) {
            prMendererSpeed = 0.0f;
        } else {
            prMendererSpeed = speed;
        }
    }
}
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", PrDecelerateMenderer);
#else /* Scheduling: li v0,1 should come before the swc1 */
PR_EXTERN
void PrDecelerateMenderer(u_int frames) {
    if (prCurrentStage == 6 || prCurrentStage == 16) {
        frames = 120;
    }

    if (prCurrentStage == 6 || prCurrentStage == 16) {
        decelerateRatio = prMendererFade / frames;
    } else {
        decelerateRatio = prMendererSpeed / frames;
    }

    deceleratingMenderer = 1;
}
#endif

PR_EXTERN
void PrRestartMenderer() {
    deceleratingMenderer = 0;
    prMendererSpeed = 1.0f;
    prMendererFade = 1.0f;
}

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", DrawMenderer__Fv);
void DrawMenderer();

PR_EXTERN
void PrSetMendererRatio(float ratio) {
    prMendererRatio = ratio;
}

PR_EXTERN
float PrGetMendererRatio() {
    return prMendererRatio;
}

PR_EXTERN
void PrSetMendererDirection(int direction) {
    prMendererGettingWorse = direction;
}

PR_EXTERN
int PrGetMendererDirection() {
    return prMendererGettingWorse;
}

PR_EXTERN
void PrSetMendererColorModulation(int modulation) {
    prMendererColorModulation = modulation;
}

PR_EXTERN
int PrIsMendererColorModulation() {
    return prMendererColorModulation;
}

PR_EXTERN
void PrInitializeMenderer(u_int tbp, void *noodlePicture, u_int fbp) {
    sceGsZbuf zbuf = prRenderStuff.m_zbuf;
    prMendererColorModulation = 1;
    prMendererTbp = tbp;
    prMendererWorkFbp = fbp;

    u_int tw = PrGetBitSize(256);
    u_int th = PrGetBitSize(16);

    PrInitializeTextureCreation(tbp, zbuf.ZBP, tw, th);
    InitializeNoodleStripRendering(tbp, fbp, tw, th);
    PrInitializeAlphaModulation();
    PrInitializeAwfulBackground(noodlePicture);
    PrInitializeNoodlePolygonPosition();
    StartNoodleRotation();
    mendererInitialized = 1;
}

PR_EXTERN
void PrRenderMenderer() {
    DrawMenderer();
    prRenderStuff.m_statistics.render_time8 = *T3_COUNT;
#if defined(PRD_SYORI)
    SyoriUpdateStats(&prRenderStuff.mStatistics);
#endif
}

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", func_0014F3B8);

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", func_0014F410);

/* prlib/menderer.cpp */
PR_EXTERN
void _GLOBAL_$I$prMendererRatio(void) {
}

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", func_0014F4C8);

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", func_0014F5D0);

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", func_0014F6D8);

/* prlib/menderer.cpp */
INCLUDE_RODATA("asm/nonmatchings/prlib/menderer", D_003967E0);

INCLUDE_RODATA("asm/nonmatchings/prlib/menderer", D_003967F0);
