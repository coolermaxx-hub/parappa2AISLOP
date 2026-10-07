#ifndef PRLIB_NOODLEPACKET_H
#define PRLIB_NOODLEPACKET_H

#include <eetypes.h>
#include <eestruct.h>
#include <libdma.h>

// Wire layouts decoded from the original DMA/GIF templates. REGLIST packets
// carry PRIM, RGBAQ, then four UV/XY pairs; addresses are in the GIF tag.
struct PrNoodleStripHeader {
    sceDmaTag dma;
    sceGifTag gif;
    u_long clamp;
    u_long clampAddress;
};

struct PrNoodleStripVertex {
    u_long uv;
    u_long xy;
};

struct PrNoodleStripQuadPacket {
    sceDmaTag dma;
    sceGifTag gif;
    u_long primitive;
    u_long color;
    PrNoodleStripVertex vertices[4];
};

struct PrGsAD {
    u_long value;
    u_long address;
};

struct PrNoodleTextureDrawPacket {
    sceGifTag stateTag;
    PrGsAD state[8];
    sceGifTag spriteTag;
};

// VU texture input: three sine waves in XYZ, with W reserved. Amplitudes are
// normalized by their sum; spatialCycles counts cycles across 256 texels.
// See docs/noodle-texture-model.md for the microprogram derivation.
struct PrNoodleTextureParameters {
    float amplitude[4];
    float spatialCycles[4];
    float temporalFrequency[4];
    float phaseOffsetCycles[4];
    float phaseTime[3];
    float bandOffset;
    PrNoodleTextureDrawPacket packet;
};

struct PrNoodleTextureCreationPacket {
    sceDmaTag stateDma;
    sceGifTag stateGif;
    PrGsAD state[2];
    sceDmaTag parameterDma;
    sceDmaTag endDma;
};

struct PrNoodleTextureCopyPacket {
    sceGifTag tag;
    PrGsAD state[13];
};

// Modulation input is a REGLIST GIF header followed by seven eight-strip
// parameter groups and a final control vector. The last lane controls fade.
static const u_int PR_NOODLE_ALPHA_STRIPS = 8;

struct PrNoodleAlphaParameters {
    sceGifTag tag;
    float radius[PR_NOODLE_ALPHA_STRIPS];
    float phase[PR_NOODLE_ALPHA_STRIPS];
    float horizontalJitter[PR_NOODLE_ALPHA_STRIPS];
    float verticalJitter[PR_NOODLE_ALPHA_STRIPS];
    float weight[PR_NOODLE_ALPHA_STRIPS];
    float signedWeight[PR_NOODLE_ALPHA_STRIPS];
    float alpha[PR_NOODLE_ALPHA_STRIPS];
    float control[3];
    float fade;
};

struct PrNoodleAlphaGsPacket {
    sceGifTag tag;
    PrGsAD frame;
    PrGsAD offset;
    PrGsAD test;
};

struct PrNoodleAlphaFramePacket {
    sceGifTag tag;
    PrGsAD maskedFrame;
    PrGsAD texture;
    PrGsAD textureFilter;
    PrGsAD clamp;
    PrGsAD textureFunction;
    PrGsAD primitive;
    PrGsAD firstUv;
    PrGsAD firstPosition;
    PrGsAD secondUv;
    PrGsAD secondPosition;
    PrGsAD frame;
    PrGsAD offset;
};

struct PrNoodleAlphaDmaPacket {
    sceDmaTag state;
    sceDmaTag parameters;
    sceDmaTag microprogram;
    sceDmaTag frame;
};

struct PrNoodleBlendPacket {
    sceDmaTag dma;
    sceGifTag gif;
    PrGsAD flush;
    PrGsAD texture;
    PrGsAD textureFilter;
    PrGsAD clamp;
    PrGsAD test;
    PrGsAD alpha;
    PrGsAD primitive;
    PrGsAD color;
    PrGsAD firstUv;
    PrGsAD firstPosition;
    PrGsAD secondUv;
    PrGsAD secondPosition;
};

// Full-screen colour fade: a translucent sprite blended over the frame.
struct PrFadeFramePacket {
    sceGifTag tag;
    PrGsAD test;
    PrGsAD alpha;
    PrGsAD color;
    PrGsAD primitive;
    PrGsAD firstPosition;
    PrGsAD secondPosition;
};

struct PrAwfulBackgroundVertex {
    PrGsAD uv;
    PrGsAD position;
};

// Context 2 draw of the rotating "awful" background texture: a textured
// triangle strip quad, then a second sprite pass under a different depth test.
struct PrAwfulBackgroundPacket {
    sceGifTag tag;
    PrGsAD flush;
    PrGsAD setupTest;
    PrGsAD maskedZbuf;
    PrGsAD alpha;
    PrGsAD texture;
    PrGsAD textureFilter;
    PrGsAD clamp;
    PrGsAD color;
    PrGsAD primitive;
    PrAwfulBackgroundVertex vertices[4];
    PrGsAD overlayTest;
    PrGsAD zbuf;
    PrGsAD overlayPrimitive;
    PrGsAD overlayFirstPosition;
    PrGsAD overlaySecondPosition;
    PrGsAD finalTest;
};

// Local-to-local copy of the work buffer, then a context 2 draw state setup.
// The packet is copied to the GIF each frame with only frame and texture patched.
struct PrNoodleStripPacket {
    sceDmaTag dma;
    sceGifTag gif;
    PrGsAD bitbltbuf;
    PrGsAD trxpos;
    PrGsAD trxreg;
    PrGsAD trxdir;
    PrGsAD frame;
    PrGsAD offset;
    PrGsAD scissor;
    PrGsAD color;
    PrGsAD texture;
    PrGsAD textureFilter;
    PrGsAD colorClamp;
    PrGsAD alpha;
    PrGsAD test;
    PrGsAD flush;
};

#endif /* PRLIB_NOODLEPACKET_H */
