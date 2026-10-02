#include "common.h"

#if defined(PRD_SYORI)
#include "dbug/syori.h"
#endif

#include "mfifo.h"
#include "renderstuff.h"
#include "spram.h"

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

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", DrawNoodleStrip__Fff);

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", StartNoodleRotation__Fv);

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", UpdateNoodleRotation__Fv);

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

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", PrDecelerateMenderer);

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

INCLUDE_ASM("asm/nonmatchings/prlib/menderer", PrInitializeMenderer);

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
