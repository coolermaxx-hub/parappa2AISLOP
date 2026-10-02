#include "common.h"

#if defined(PRD_SYORI)
#include "dbug/syori.h"
#endif

#include "mfifo.h"
#include "renderstuff.h"
#include "spram.h"
#include "utility.h"

#include <eeregs.h>
#include <math.h>
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
void PushNoodleColor(u_long *rgbaq);

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

/* data */
extern float noodleSaturationRange[][2];
extern float noodleBaseColor[][3];

/* sdata */
extern float noodleHueOffset;
extern float noodleBrightnessPhase;

/* data */
extern float prMendererNoodleColor[4];

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", PushNoodleColor__FPUl);
#else /* Requires .lit4 migration */
void PushNoodleColor(u_long *rgbaq) {
    if (!prMendererColorModulation || prCurrentStage == 6) {
        *rgbaq = SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0);
        return;
    }

    int stage = StageIndexForColor();
    float *range = noodleSaturationRange[stage];
    float saturation = range[0] + (range[1] - range[0]) * GetRandom();

    float hue = GetRandom() * 3.0f + noodleHueOffset;
    if (hue > 3.0f) {
        hue -= 3.0f;
    }

    float r, g, b;

    if (hue < 1.0f) {
        r = hue;
    } else {
        r = 0.0f;
        if (hue < 2.0f) {
            r = 2.0f - hue;
        }
    }

    if (hue < 1.0f) {
        g = 0.0f;
    } else if (hue < 2.0f) {
        g = hue - 1.0f;
    } else {
        g = 3.0f - hue;
    }

    if (hue < 1.0f) {
        b = 1.0f - hue;
    } else {
        b = 0.0f;
        if (!(hue < 2.0f)) {
            b = hue - 2.0f;
        }
    }

    float *base = noodleBaseColor[stage];
    r = base[0] + r * saturation;
    g = base[1] + g * saturation;
    b = base[2] + b * saturation;

    float blend;
    if (prCurrentStage == 19 || prMendererRatio <= 1.0f) {
        blend = 0.0f;
    } else if (prMendererRatio <= 1.4f) {
        blend = (prMendererRatio - 1.0f) * 0.3f / 0.4f;
    } else if (prMendererRatio <= 1.6f) {
        blend = 0.3f;
    } else {
        blend = 1.0f;
        if (prMendererRatio <= 2.0f) {
            blend = (prMendererRatio - 1.6f) * 0.7f / 0.4f + 0.3f;
        }
    }

    float phase = noodleBrightnessPhase + GetRandom() * 2.0f;
    if (phase >= 2.0f) {
        phase -= 2.0f;
    }

    float d = 1.0f - phase;
    if (d < 0.0f) {
        d = -d;
    }

    float brightness = d * 0.29999995f + 0.6f;
    r += (brightness * prMendererNoodleColor[0] - r) * blend;
    b += (brightness * prMendererNoodleColor[2] - b) * blend;
    g += (brightness * prMendererNoodleColor[1] - g) * blend;

    if (r > 1.0f) {
        r = 1.0f;
    }
    if (g > 1.0f) {
        g = 1.0f;
    }
    if (b > 1.0f) {
        b = 1.0f;
    }

    u_int r8 = r * 255.99f;
    u_int g8 = g * 255.99f;
    u_int b8 = b * 255.99f;
    *rgbaq = r8 | ((u_long)g8 << 8) | ((u_long)b8 << 16) | (0x80UL << 24);
}
#endif

/* data */
extern u_long noodleStripHeaderPacket[6];

/* rodata */
extern const u_long D_003967E0[2]; /* DMAcnt, qwc 6 */
extern const u_long D_003967F0[2]; /* GIFtag, REGLIST PRIM RGBAQ (UV XYZ2) x4 */

void PrGetNoodlePolygonPosition(NaVECTOR<float, 4> *position, u_int index);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", DrawNoodleStripChunk__FRCt8NaMATRIX3Zfi4i4);
#else /* Requires .lit4 migration */
void DrawNoodleStripChunk(const NaMATRIX<float, 4, 4>& matrix) {
    u_int count = 115;

    if (prCurrentStage == 6 || prCurrentStage == 16) {
        float ratio = prMendererRatio;
        float t;

        if (ratio <= 1.05f) {
            t = 0.0f;
        } else if (ratio <= 1.4f) {
            t = (ratio - 1.0f) / 0.4f;
        } else if (ratio <= 1.6f) {
            t = 1.0f;
        } else if (ratio < 1.95f) {
            t = (2.0f - ratio) / 0.4f;
        } else {
            t = 0.0f;
        }

        count = (u_int)(t * (t * t) * 114.0f + 0.5f) + 1;
        if (count > 115) {
            count = 115;
        } else if (count == 0) {
            count = 1;
        }
    }

    u_int per_block = (115 + 4) / 5;
    u_int index = 0;
    noodleRandomSeed = 0;

    for (u_int block = 0; block < 5; ) {
        u_int next = block + 1;

        u_long128 *buf = prSpramData_tmp_menderer->m_noodle_buffer[0];
        prSpramData_tmp_menderer->m_noodle_buffer[0] = prSpramData_tmp_menderer->m_noodle_buffer[1];
        prSpramData_tmp_menderer->m_noodle_buffer[1] = prSpramData_tmp_menderer->m_noodle_buffer[2];
        prSpramData_tmp_menderer->m_noodle_buffer[2] = buf;

        u_long *header = (u_long*)buf;
        for (int i = 0; i < 6; i++) {
            header[i] = noodleStripHeaderPacket[i];
        }
        header[4] = ((u_long)(((block + 2) * 16) - 1) << 34) | 0x3FC00A | ((u_long)(next * 16) << 24);
        PrSendMfifo((sceDmaTag*)header);

        u_long v0 = (u_long)(next * 256) << 16;
        u_long v1 = (u_long)((block + 2) * 256) << 16;

        for (u_int j = 0; j < per_block; j++) {
            buf = prSpramData_tmp_menderer->m_noodle_buffer[0];
            prSpramData_tmp_menderer->m_noodle_buffer[0] = prSpramData_tmp_menderer->m_noodle_buffer[1];
            prSpramData_tmp_menderer->m_noodle_buffer[1] = prSpramData_tmp_menderer->m_noodle_buffer[2];
            prSpramData_tmp_menderer->m_noodle_buffer[2] = buf;

            u_long *packet = (u_long*)buf;
            packet[0] = D_003967E0[0];
            packet[1] = D_003967E0[1];
            packet[2] = D_003967F0[0];
            packet[3] = D_003967F0[1];

            NaVECTOR<float, 4> position[4];
            PrGetNoodlePolygonPosition(position, index);
            index++;

            NaVECTOR<float, 4> screen[4];
            for (int k = 0; k < 4; k++) {
                screen[k] = matrix * position[k];
            }

            u_long *ad = &packet[4];
            *ad++ = 0x35C;
            PushNoodleColor(ad++);
            *ad++ = v0;
            *ad++ = (u_int)screen[0][0] | ((u_long)(u_int)screen[0][1] << 16);
            *ad++ = v0 | 0x1000;
            *ad++ = (u_int)screen[1][0] | ((u_long)(u_int)screen[1][1] << 16);
            *ad++ = v1;
            *ad++ = (u_int)screen[2][0] | ((u_long)(u_int)screen[2][1] << 16);
            *ad++ = v1 | 0x1000;
            *ad = (u_int)screen[3][0] | ((u_long)(u_int)screen[3][1] << 16);
            PrSendMfifo((sceDmaTag*)packet);

            if (index == count) {
                goto done;
            }
        }

        if (index == count) {
            break;
        }
        block = next;
    }

done:
    float hue = noodleHueOffset + prMendererSpeed * 0.07f;
    if (hue >= 3.0f) {
        hue -= 3.0f;
    }

    float brightness = noodleBrightnessPhase + prMendererSpeed * 0.0528f;
    if (brightness >= 2.0f) {
        brightness -= 2.0f;
    }

    noodleBrightnessPhase = brightness;
    noodleHueOffset = hue;
}
#endif

/* Template instances emitted in spram.cpp */
NaMATRIX<float, 4, 4> TransMatrix_tmp_menderer(const float& x, const float& y, const float& z) asm("func_00148248");
NaMATRIX<float, 4, 4> ScaleMatrix_tmp_menderer(const float& x, const float& y, const float& z) asm("func_00148140");

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", SetNoodleRotationMatrix__FRt8NaMATRIX3Zfi4i4f);
#else /* Requires .lit4 migration */
void SetNoodleRotationMatrix(NaMATRIX<float, 4, 4>& matrix, float rot) {
    rot = (rot - floorf(rot)) * 2.0f * 3.1415927f;
    matrix = NaMATRIX<float, 4, 4>::RotateMatrix(2, rot);
    matrix = TransMatrix_tmp_menderer(0.5f, 0.5f, 0.0f) * matrix;
    matrix = ScaleMatrix_tmp_menderer(10240.0f, 3584.0f, 0.0f) * matrix;
    matrix = TransMatrix_tmp_menderer(32768.0f, 32768.0f, 0.0f) * matrix;
}
#endif

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
        distance = ratio * (0.32f - 0.6054f) + 0.6054f;
        length = 0.9375f;
        width = 0.7f;
    } else if (ratio <= 2.0f) {
        float t = ratio - 1.0f;
        length = t * (1.6016f - 0.9375f) + 0.9375f;
        width = t * (1.0f - 0.7f) + 0.7f;
        distance = t * (-0.0586f - 0.32f) + 0.32f;
    } else {
        float t = ratio - 2.0f;
        length = t * (0.9376f - 1.6016f) + 1.6016f;
        width = t * (0.7f - 1.0f) + 1.0f;
        distance = t * (0.45f - -0.0586f) + -0.0586f;
    }

    prMendererWidth = width;
    prMendererDistance = distance;
    prMendererLength = length;

    DrawNoodleStripChunk(matrix);
}
#endif

void StartNoodleRotation() {
    noodleChangeTimer = 900;
    noodleDeltaRotation = 0.001f;
    noodleStatus = 0;
    noodleRotation = 0.0f;
}

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

/* sdata */
extern float prSchoolLeaderIndex;
extern float mendererLastRatio;

void UpdateNoodleRotation();
void DrawNoodleStrip(float ratio, float rot);
void PrWaitDmaFinish(u_int channel);
void PrFadeFrameImage(float arg0);
void PrCreateAlphaModulation(float alpha);
void PrStartAwfulRotation();
void PrBlendNoodleImage(bool clear);

void DrawMenderer();

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/menderer", DrawMenderer__Fv);
#else /* Requires .lit4 migration */
void DrawMenderer() {
    float ratio = prMendererRatio;

    if (ratio == 0.0f) {
        StartNoodleRotation();
        return;
    }

    float leader = prSchoolLeaderIndex + prMendererSpeed * 0.0032f;
    if (leader > 1.0f) {
        leader -= 1.0f;
    }

    if (prCurrentStage == 8 || prCurrentStage == 18) {
        prSchoolLeaderIndex = leader;
        return;
    }

    if (ratio != 1.0f && ratio != 2.0f) {
        noodleChangeTimer = 900;
    }

    prSchoolLeaderIndex = leader;
    UpdateNoodleRotation();

    float delta = -0.01f;
    if (noodleDeltaRotation >= 0.0f) {
        delta = 0.01f;
    }
    delta *= prMendererSpeed;

    if (ratio <= 1.0f) {
        ratio = sqrtf(ratio) * 2.0f - 1.0f;
        noodleRotation += delta * (1.0f - ratio);
    } else {
        float d = ratio - 1.5f;
        if (d < 0.0f) {
            d = -d;
        }
        noodleRotation += delta * (0.5f - d);
        PrFadeFrameImage(1.0f - (d + d));
        PrWaitDmaFinish(2);
    }

    if (noodleRotation > 1.0f) {
        noodleRotation -= 1.0f;
    }
    if (noodleRotation < 0.0f) {
        noodleRotation += 1.0f;
    }

    float alpha = 1.0f;
    if (ratio > 1.5f) {
        alpha = (ratio - 1.5f) * 0.5f;
    } else if (ratio > 1.0f) {
        alpha = (1.5f - ratio) * 2.0f;
    }

    if (ratio != 0.0f) {
        PrCreateAlphaModulation(alpha);
    }

    PrStartMfifo();

    if (mendererLastRatio <= 1.6f && ratio > 1.0f) {
        PrStartAwfulRotation();
    }

    float strip;
    if (prCurrentStage == 19 || prCurrentStage == 6) {
        if (ratio <= 1.0f) {
            strip = ratio;
        } else if (ratio <= 1.4f) {
            strip = (ratio - 1.0f) / 0.4f + 1.0f;
        } else if (ratio <= 1.6f) {
            strip = 2.0f;
        } else {
            strip = (ratio - 1.6f) * 0.6f / 0.4f + 2.0f;
        }
    } else if (prMendererGettingWorse) {
        if (ratio <= 0.5f) {
            strip = ratio + ratio;
        } else if (ratio <= 0.6f) {
            strip = 1.0f;
        } else if (ratio <= 1.0f) {
            strip = 1.0f - (ratio - 0.6f) * 0.5f / 0.4f;
        } else if (ratio <= 1.13f) {
            strip = (ratio - 1.0f) * 0.5f / 0.13f + 0.5f;
        } else if (ratio <= 1.4f) {
            strip = (ratio - 1.13f) / 0.26f + 1.0f;
        } else if (ratio <= 1.6f) {
            strip = 2.0f;
        } else {
            strip = (ratio - 1.6f) / 0.4f + 2.0f;
        }
    } else {
        if (ratio <= 1.0f) {
            strip = ratio * 0.5f;
        } else if (ratio <= 1.13f) {
            strip = (ratio - 1.0f) * 0.5f / 0.13f + 0.5f;
        } else if (ratio <= 1.4f) {
            strip = (ratio - 1.13f) / 0.26f + 1.0f;
        } else if (ratio <= 1.6f) {
            strip = 2.0f;
        } else {
            strip = (ratio - 1.6f) / 0.4f + 2.0f;
        }
    }

    mendererLastRatio = ratio;
    DrawNoodleStrip(strip, noodleRotation);

    PrWaitDmaFinish(1);
    while (*VIF1_STAT & 0x3) {
        /* Wait for VIF1 to go idle */
    }

    PrBlendNoodleImage(ratio == 0.0f);
    PrStopMfifo();
}
#endif

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
