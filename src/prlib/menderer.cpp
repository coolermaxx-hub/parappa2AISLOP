#include "common.h"

#include "menderer.h"

#if defined(PRD_SYORI)
#include "dbug/syori.h"
#endif

#include "dma.h"
#include "mfifo.h"
#include "renderstuff.h"
#include "spram.h"
#include "utility.h"
#include "noodlepacket.h"
#include "gsstate.h"

#include <eeregs.h>
#include <math.h>
#include <eestruct.h>

extern int prCurrentStage;

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)EE_SCRATCHPAD_BASE;

float prMendererRatio = 0.0f;
float prMendererSyncRatio = 0.0f;
int prMendererGettingWorse = 1;
int prMendererColorModulation = 1;
float prMendererSpeed = 1.0f;
float prMendererFade = 1.0f;
float prMendererDistance = 0.0f;
float prMendererWidth = 1.0f;
float prMendererLength = 1.0f;
float prSchoolLeaderIndex = 0.0f;
int mendererInitialized = 0;
u_int noodleRandomSeed = 0;
float noodleHueOffset = 0.0f;
float noodleBrightnessPhase = 0.0f;
float noodleRotation = 0.0f;
int deceleratingMenderer = 0;
float decelerateRatio = 1.0f;
float mendererLastRatio = 0.0f;
u_int prMendererTbp = 0;
u_int prMendererWorkFbp = 0;
u_int prMendererDrawFbp = 0;



float prMendererNoodleColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

// Work-buffer copy and draw state for the noodle strips. Frame and texture
// are filled in by InitializeNoodleStripRendering.
static PrNoodleStripPacket noodleStripDmaPacket = {
    { 15, 0, PR_DMA_TAG_CNT, NULL, { 0, 0 } },
    { /*NLOOP*/14, /*EOP*/1, 0, 0, /*PRE*/0, /*PRIM*/0, /*FLG*/SCE_GIF_PACKED, /*NREG*/1, SCE_GIF_PACKED_AD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, SCE_GS_BITBLTBUF },
    { 0, SCE_GS_TRXPOS },
    { SCE_GS_SET_TRXREG(SCREEN_WIDTH, SCREEN_FIELD_HEIGHT), SCE_GS_TRXREG },
    { SCE_GS_SET_TRXDIR(2), SCE_GS_TRXDIR },
    { 0, SCE_GS_FRAME_2 },
    { SCE_GS_SET_XYOFFSET(2048 << 4, 2048 << 4), SCE_GS_XYOFFSET_2 },
    { SCE_GS_SET_SCISSOR(0, SCREEN_WIDTH - 1, 0, SCREEN_FIELD_HEIGHT - 1), SCE_GS_SCISSOR_2 },
    { 0, SCE_GS_RGBAQ },
    { 0, SCE_GS_TEX0_2 },
    { PR_TEX1_BILINEAR, SCE_GS_TEX1_2 },
    { SCE_GS_SET_COLCLAMP(1), SCE_GS_COLCLAMP },
    { PR_ALPHA_BLEND, SCE_GS_ALPHA_2 },
    { PR_TEST_ALPHA_NONZERO(SCE_GS_ZALWAYS), SCE_GS_TEST_2 },
    { 0, SCE_GS_TEXFLUSH },
};

// DMAcnt of two quadwords: the GIF tag and one clamp register write.
static PrNoodleStripHeader noodleStripHeaderPacket = {
    { 2, 0, PR_DMA_TAG_CNT, NULL, { 0, 0 } },
    { /*NLOOP*/1, /*EOP*/1, 0, 0, /*PRE*/0, /*PRIM*/0, /*FLG*/SCE_GIF_PACKED, /*NREG*/1, SCE_GIF_PACKED_AD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    0,
    SCE_GS_CLAMP_2,
};

// Saturation range (min, max) per stage.
static float noodleSaturationRange[10][2] = {
    { 0.3f, 0.7f },
    { 0.2f, 0.4f },
    { 0.05f, 0.2f },
    { 0.0f, 0.2f },
    { 0.1f, 0.3f },
    { 0.0f, 0.4f },
    { 0.1f, 0.3f },
    { 0.1f, 0.2f },
    { 0.1f, 0.3f },
    { 0.1f, 0.3f },
};

// Base RGB per stage.
static float noodleBaseColor[10][3] = {
    { 0.6000000238f, 0.6000000238f, 0.6000000238f },
    { 0.9284310341f, 0.5455880165f, 0.2882350087f },
    { 0.5458824039f, 0.3764706254f, 0.3764706254f },
    { 0.3764710128f, 0.4705890119f, 0.3294119835f },
    { 0.7215690017f, 0.3137260079f, 0.3450979888f },
    { 0.6807842851f, 0.5929412842f, 0.3074511886f },
    { 0.0f, 0.0f, 0.4705879986f },
    { 0.4517648816f, 0.4517648816f, 0.1694114953f },
    { 0.3764710128f, 0.3137260079f, 0.4392159879f },
    { 1.0f, 1.0f, 1.0f },
};

static int noodleStatus;
static int noodleChangeTimer;
static float noodleDeltaRotation;

void PrInitializeTextureCreation(u_int tbp, u_int zbp, u_int tw, u_int th);
void PrInitializeAlphaModulation();
void PrInitializeAwfulBackground(void *tim2);
void PrInitializeNoodlePolygonPosition();
void DrawNoodleStripChunk(const NaMATRIX<float, 4, 4>& matrix);
void SetNoodleRotationMatrix(NaMATRIX<float, 4, 4>& matrix, float rot);
void StartNoodleRotation();
void PushNoodleColor(u_long *rgbaq);

void InitializeNoodleStripRendering(u_int tbp, u_int fbp, u_int tw, u_int th) {
    noodleStripDmaPacket.frame.value = PR_FRAME_CT32(fbp);
    noodleStripDmaPacket.texture.value = SCE_GS_SET_TEX0(tbp, 4, SCE_GS_PSMCT32, tw, th, 1, 0, 0, 0, 0, 0, 0);
}

static float GetRandom() {
    u_int seed = noodleRandomSeed * 0x19660D + 0x3C6EF35F;
    float ret = ((seed >> 8) & 0xFFFF) * (1.0f / 65536.0f);
    noodleRandomSeed = seed;
    return ret;
}

static int StageIndexForColor() {
    if (prCurrentStage == PR_STAGE_TITLE) {
        return 0;
    }
    return (u_int)prCurrentStage % 10; /* VS stages 11-18 use the colours of stages 1-8 */
}

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
    if (prCurrentStage == PR_STAGE_TITLE || prMendererRatio <= 1.0f) {
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

    u_int r8 = static_cast<u_int>(r * 255.99f);
    u_int g8 = static_cast<u_int>(g * 255.99f);
    u_int b8 = static_cast<u_int>(b * 255.99f);
    *rgbaq = r8 | ((u_long)g8 << 8) | ((u_long)b8 << 16) | (0x80UL << 24);
}

/* rodata */
// DMAcnt of six quadwords: the GIF tag plus five register-list quadwords.
static const sceDmaTag noodleQuadDmaTag = { 6, 0, PR_DMA_TAG_CNT, NULL, { 0, 0 } };
// REGLIST, one loop, EOP, ten registers: PRIM, RGBAQ, then (UV, XYZ2) x4.
static const sceGifTag noodleQuadGifTag = {
    /*NLOOP*/1, /*EOP*/1, 0, 0, /*PRE*/0, /*PRIM*/0, /*FLG*/SCE_GIF_REGLIST, /*NREG*/10,
    SCE_GS_PRIM, SCE_GS_RGBAQ,
    SCE_GS_UV, SCE_GS_XYZ2, SCE_GS_UV, SCE_GS_XYZ2, SCE_GS_UV, SCE_GS_XYZ2, SCE_GS_UV, SCE_GS_XYZ2,
    0, 0, 0, 0, 0, 0
};

void PrGetNoodlePolygonPosition(NaVECTOR<float, 4> *position, u_int index);

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

    for (u_int block = 0; block < 5 && index < count; block++) {
        u_int next = block + 1;

        u_long128 *buf = prSpramData->m_noodle_buffer[0];
        prSpramData->m_noodle_buffer[0] = prSpramData->m_noodle_buffer[1];
        prSpramData->m_noodle_buffer[1] = prSpramData->m_noodle_buffer[2];
        prSpramData->m_noodle_buffer[2] = buf;

        PrNoodleStripHeader *header = reinterpret_cast<PrNoodleStripHeader*>(buf);
        *header = noodleStripHeaderPacket;
        header->clamp = SCE_GS_SET_CLAMP(/*WMS*/SCE_GS_REGION_CLAMP, /*WMT*/SCE_GS_REGION_CLAMP, 0, 255, next * 16, ((block + 2) * 16) - 1);
        PrSendMfifo(&header->dma);

        u_long v0 = (u_long)(next * 256) << 16;
        u_long v1 = (u_long)((block + 2) * 256) << 16;

        for (u_int j = 0; j < per_block && index < count; j++) {
            buf = prSpramData->m_noodle_buffer[0];
            prSpramData->m_noodle_buffer[0] = prSpramData->m_noodle_buffer[1];
            prSpramData->m_noodle_buffer[1] = prSpramData->m_noodle_buffer[2];
            prSpramData->m_noodle_buffer[2] = buf;

            PrNoodleStripQuadPacket *packet = reinterpret_cast<PrNoodleStripQuadPacket*>(buf);
            packet->dma = noodleQuadDmaTag;
            packet->gif = noodleQuadGifTag;

            NaVECTOR<float, 4> position[4];
            PrGetNoodlePolygonPosition(position, index);
            index++;

            NaVECTOR<float, 4> screen[4];
            for (int k = 0; k < 4; k++) {
                screen[k] = matrix * position[k];
            }

            packet->primitive = SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 1, 1, 0, 1, 0, 1, 1, 0);
            PushNoodleColor(&packet->color);
            for (int k = 0; k < 4; k++) {
                const u_long textureV = k < 2 ? v0 : v1;
                packet->vertices[k].uv = textureV | (k % 2 ? 0x1000 : 0);
                packet->vertices[k].xy = static_cast<u_int>(screen[k][0])
                                        | (static_cast<u_long>(static_cast<u_int>(screen[k][1])) << 16);
            }
            PrSendMfifo(&packet->dma);
        }
    }

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

void SetNoodleRotationMatrix(NaMATRIX<float, 4, 4>& matrix, float rot) {
    rot = (rot - floorf(rot)) * 2.0f * PR_PI;
    matrix = NaMATRIX<float, 4, 4>::RotateMatrix(2, rot);
    matrix = NaMATRIX<float, 4, 4>::TranslateMatrix(0.5f, 0.5f, 0.0f) * matrix;
    matrix = NaMATRIX<float, 4, 4>::ScaleMatrix(10240.0f, 3584.0f, 0.0f) * matrix;
    matrix = NaMATRIX<float, 4, 4>::TranslateMatrix(32768.0f, 32768.0f, 0.0f) * matrix;
}

static void PreDrawNoodleStrip() {
    u_long128 *buf = prSpramData->m_noodle_buffer[0];
    prSpramData->m_noodle_buffer[0] = prSpramData->m_noodle_buffer[1];
    prSpramData->m_noodle_buffer[1] = prSpramData->m_noodle_buffer[2];
    prSpramData->m_noodle_buffer[2] = buf;

    PrNoodleStripPacket *packet = (PrNoodleStripPacket*)buf;
    *packet = noodleStripDmaPacket;
    packet->bitbltbuf.value = SCE_GS_SET_BITBLTBUF(prMendererDrawFbp * 32, 10, 0, prMendererWorkFbp * 32, 10, 0);

    PrSendMfifo(&packet->dma);
}

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
        distance = t * (-0.0586f - 0.32f) + 0.32f;
        width = t * (1.0f - 0.7f) + 0.7f;
    } else {
        float t = ratio - 2.0f;
        length = t * (0.9376f - 1.6016f) + 1.6016f;
        distance = t * (0.45f - -0.0586f) + -0.0586f;
        width = t * (0.7f - 1.0f) + 1.0f;
    }

    prMendererWidth = width;
    prMendererDistance = distance;
    prMendererLength = length;

    DrawNoodleStripChunk(matrix);
}

void StartNoodleRotation() {
    noodleChangeTimer = 900;
    noodleDeltaRotation = 0.001f;
    noodleStatus = 0;
    noodleRotation = 0.0f;
}

void UpdateNoodleRotation() {
    if (prCurrentStage == 6) {
        noodleRotation += prMendererSpeed * 0.001f;
        return;
    }

    if (noodleStatus == 0 || noodleStatus == 2) {
        u_int timer = noodleChangeTimer - 1;
        if (timer == 0) {
            timer = 900;
            noodleStatus = (noodleStatus + 1) % 4;
        }
        noodleChangeTimer = timer;
    }

    float delta = noodleDeltaRotation;
    if (noodleStatus == 1) {
        delta -= 1.6666667e-05f;
        if (delta < -0.001f) {
            delta = -0.001f;
            noodleStatus = 2;
        }
        noodleDeltaRotation = delta;
    } else if (noodleStatus == 3) {
        delta += 1.6666667e-05f;
        if (delta > 0.001f) {
            delta = 0.001f;
            noodleStatus = 0;
        }
        noodleDeltaRotation = delta;
    }

    noodleRotation += prMendererSpeed * delta;
}

void PrUpdateMendererSpeed() {
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

PR_EXTERN
void PrDecelerateMenderer(u_int frames) {
    if (prCurrentStage == 6 || prCurrentStage == 16) {
        frames = 120;
    }

    float r;
    if (prCurrentStage == 6 || prCurrentStage == 16) {
        r = prMendererFade / frames;
    } else {
        r = prMendererSpeed / frames;
    }
    decelerateRatio = r;
    deceleratingMenderer = 1;
}

PR_EXTERN
void PrRestartMenderer() {
    deceleratingMenderer = 0;
    prMendererSpeed = 1.0f;
    prMendererFade = 1.0f;
}

void UpdateNoodleRotation();
void DrawNoodleStrip(float ratio, float rot);
void PrWaitDmaFinish(u_int channel);
void PrFadeFrameImage(float fade);
void PrCreateAlphaModulation(float alpha);
void PrStartAwfulRotation();
void PrBlendNoodleImage(bool clear);

void DrawMenderer();

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
    if (prCurrentStage == PR_STAGE_TITLE || prCurrentStage == 6) {
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
    SyoriUpdateStats(&prRenderStuff.m_statistics);
#endif
}

// Retained CRT registration: the original initializer is an empty routine.
// It is called by the SDK constructor table, not a template-emission helper.
PR_EXTERN
void _GLOBAL_$I$prMendererRatio(void) {
}

/* prlib/menderer.cpp */
