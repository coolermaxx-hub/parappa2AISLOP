#include "nalib/namatrix.h"
#include "prlib/spadata.h"
#include "prlib/spram.h"
#include "prlib/noodlepacket.h"
#include "prlib/model.h"

// This freestanding test has no CRT constructor table. Initialize the one
// identity constant used by the generic predicate explicitly inside the test.
NaMATRIX<float, 3, 3> NaMATRIX<float, 3, 3>::IDENT;

extern "C" int matrix_layout_test() {
    NaMATRIX<float, 2, 2> small(1.0f, 2.0f, 3.0f, 4.0f);
    if (sizeof(small) != 16 || small[0][1] != 2.0f || small[1][0] != 3.0f) return 1;
    NaMATRIX<float, 3, 3> medium(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
    if (sizeof(medium) != 36 || medium[1][0] != 4.0f || medium[2][2] != 9.0f) return 2;
    NaMATRIX<float, 4, 4> matrix(1.0f, 2.0f, 3.0f, 4.0f,
                              5.0f, 6.0f, 7.0f, 8.0f,
                              9.0f, 10.0f, 11.0f, 12.0f,
                              13.0f, 14.0f, 15.0f, 16.0f);
    if (sizeof(matrix) != 64 || matrix[1][0] != 5.0f || matrix[3][2] != 15.0f) return 3;
    // The nine-scalar overload writes a packed prefix even on a 4x4 matrix.
    matrix.Set(21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f);
    if (matrix[1][0] != 25.0f || matrix[2][0] != 29.0f || matrix[2][1] != 10.0f) return 4;
    NaVECTOR<float, 4> vector(2.0f, -3.0f, 4.0f, 5.0f);
    NaMATRIX<float, 4, 4> scale = NaMATRIX<float, 4, 4>::ScaleMatrix(vector);
    if (scale[0][0] != 2.0f || scale[1][1] != -3.0f ||
        scale[2][2] != 4.0f || scale[3][3] != 5.0f || scale[1][0] != 0.0f) return 5;
    NaMATRIX<float, 4, 4> translation = NaMATRIX<float, 4, 4>::TranslateMatrix(vector);
    if (translation[3][0] != 2.0f || translation[3][1] != -3.0f ||
        translation[3][2] != 4.0f || translation[3][3] != 1.0f) return 6;
    if (sizeof(SpaTrackBase) != 16 || sizeof(SpaTransform) != 16) return 7;
    if (sizeof(SpaTrack<float>) != 20 || sizeof(SpaTrack<NaVECTOR<float, 4> >) != 32 ||
        sizeof(SpaTrack<NaMATRIX<float, 4, 4> >) != 80) return 8;
    SpaTypedTransform<float> scalarTransform;
    if (&scalarTransform.track != scalarTransform.GetTrack<float>()) return 9;
    if (sizeof(PrSPRAM_DATA) != 0x4000) return 10;
    PrSPRAM_DATA scratchpad;
    const char* base = reinterpret_cast<const char*>(&scratchpad);
    if (reinterpret_cast<const char*>(scratchpad.m_packet_workspace) - base != 0x690) return 11;
    for (int i = 0; i < 3; i++) {
        if (reinterpret_cast<const char*>(scratchpad.m_packet_banks.noodle[i]) - base != (i + 1) * 0x1000) return 12;
    }
    const NaVECTOR<float, 2> point(2.0f, 3.0f);
    NaVECTOR<float, 2> transformed;
    small.ApplyTransposed(transformed, point);
    if (transformed[0] != 8.0f || transformed[1] != 18.0f) return 13;
    if (sizeof(PrNoodleTextureParameters) != 240 || sizeof(PrNoodleTextureDrawPacket) != 160 ||
        sizeof(PrNoodleTextureCreationPacket) != 96 || sizeof(PrNoodleTextureCopyPacket) != 224) return 14;
    if (sizeof(PrNoodleAlphaParameters) != 256 || sizeof(PrNoodleAlphaGsPacket) != 64 ||
        sizeof(PrNoodleAlphaFramePacket) != 208 || sizeof(PrNoodleAlphaDmaPacket) != 64 ||
        sizeof(PrNoodleBlendPacket) != 224) return 15;
    PrNoodleAlphaParameters parameters;
    const char *parameterBase = reinterpret_cast<const char*>(&parameters);
    if (reinterpret_cast<const char*>(parameters.radius) - parameterBase != 16 ||
        reinterpret_cast<const char*>(parameters.alpha) - parameterBase != 208 ||
        reinterpret_cast<const char*>(&parameters.fade) - parameterBase != 252) return 16;
    if (sizeof(SpmNode) != 0x1c0 || sizeof(SpmClusterData) != 12 || sizeof(SpmShapeData) != 12 ||
        sizeof(SpmClusterInfluence) != 8 || sizeof(PrVuNodeHeaderDmaPacket) != 0x1a0) return 17;
    SpmShapeNode shape;
    const char *shapeBase = reinterpret_cast<const char*>(&shape);
    if (reinterpret_cast<const char*>(&shape.m_shape) - shapeBase != 0x1b0 ||
        reinterpret_cast<const char*>(shape.m_shapeDeltas) - shapeBase != 0x1c0 ||
        reinterpret_cast<const char*>(&shape.m_flags) - shapeBase != 0x154) return 18;
    if (sizeof(PrEECoreContext) != 0x100 || sizeof(PrEECoreInputVertex) != 48 ||
        sizeof(PrEECoreOutputVertex) != 48) return 19;
    if (reinterpret_cast<const char*>(&scratchpad.m_eeCore) - base != 0x2b0 ||
        reinterpret_cast<const char*>(&scratchpad.m_eeCore.input) - base != 0x330 ||
        reinterpret_cast<const char*>(&scratchpad.m_eeCore.output) - base != 0x360 ||
        reinterpret_cast<const char*>(&scratchpad.m_eeCore.vertexKernel) - base != 0x3a0) return 20;
    PrVuDataChunkPacketHeader chunk;
    chunk.metadata.prefixTripletCount = 0;
    const char *chunkBase = reinterpret_cast<const char*>(&chunk);
    if (reinterpret_cast<const char*>(&chunk.gif) - chunkBase != 0x50 ||
        reinterpret_cast<const char*>(chunk.InputVertices()) - chunkBase != 0x60) return 21;
    if (sizeof(chunk) != 0x60 ||
        reinterpret_cast<const char*>(&chunk.clippedPolygonGif) - chunkBase != 0x20 ||
        reinterpret_cast<const char*>(chunk.reserved) - chunkBase != 0x30) return 33;
    chunk.metadata.prefixTripletCount = 2;
    if (reinterpret_cast<const char*>(chunk.InputVertices()) - chunkBase != 0xc0) return 34;
    PrVuContourVertexPair contourPair;
    if (sizeof(contourPair) != 64 || sizeof(PrVuContourVertex) != 32 ||
        reinterpret_cast<const char*>(&contourPair.previous.position) -
        reinterpret_cast<const char*>(&contourPair) != 32) return 35;
    PrNoodleTextureParameters texture;
    const char *textureBase = reinterpret_cast<const char*>(&texture);
    if (reinterpret_cast<const char*>(texture.amplitude) - textureBase != 0 ||
        reinterpret_cast<const char*>(texture.spatialCycles) - textureBase != 16 ||
        reinterpret_cast<const char*>(texture.temporalFrequency) - textureBase != 32 ||
        reinterpret_cast<const char*>(texture.phaseOffsetCycles) - textureBase != 48 ||
        reinterpret_cast<const char*>(texture.phaseTime) - textureBase != 64 ||
        reinterpret_cast<const char*>(&texture.packet) - textureBase != 80) return 22;
    NaMATRIX<float, 2, 2> smallCopy(small);
    if (smallCopy[0][0] != 1.0f || smallCopy[1][1] != 4.0f) return 23;
    smallCopy.Set(-1.0f, -2.0f, -3.0f, -4.0f);
    NaMATRIX<float, 2, 2>::Copy(smallCopy, small);
    if (smallCopy[0][1] != 2.0f || smallCopy[1][0] != 3.0f) return 24;
    NaMATRIX<float, 3, 3> mediumCopy(medium);
    mediumCopy = mediumCopy;
    if (mediumCopy[0][2] != 3.0f || mediumCopy[2][0] != 7.0f || mediumCopy[2][2] != 9.0f) return 25;
    NaMATRIX<float, 3, 3>::IDENT.Set(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    if (mediumCopy.IsIdentity()) return 26;
    mediumCopy = NaMATRIX<float, 3, 3>::IDENT;
    if (!mediumCopy.IsIdentity()) return 27;
    const NaVECTOR<int, 3> integerVector(7, -11, 19);
    NaVECTOR<int, 3> integerCopy(integerVector);
    NaVECTOR<int, 3>::Copy(integerCopy, integerCopy);
    if (integerCopy[0] != 7 || integerCopy[1] != -11 || integerCopy[2] != 19) return 28;
    const NaVECTOR<float, 2> pair(8.0f, -12.0f);
    NaVECTOR<float, 2> pairResult = (pair / 4.0f + pair) * 2.0f - pair;
    if (pairResult[0] != 12.0f || pairResult[1] != -18.0f) return 29;
    pairResult = -pair;
    if (pairResult[0] != -8.0f || pairResult[1] != 12.0f) return 30;
    NaMATRIX<float, 3, 3> arithmetic = (medium * 2.0f + medium) / 3.0f - medium;
    if (arithmetic.IsNonZero()) return 31;
    NaVECTOR<int, 3> doubled = integerVector * 2;
    if (doubled[0] != 14 || doubled[1] != -22 || doubled[2] != 38) return 32;
    return 0;
}
