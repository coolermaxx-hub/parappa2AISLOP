#include "prlib/animation.h"
#include "prlib/spadata.h"

// Match the documented inline serialization using distinct typed tables.
// SpaNodeAnimation already includes the first weight pointer at offset 0x30.
struct AnimationNodeRecord {
    SpaNodeAnimation node;
    SpaTrack<float> *secondWeight;
    SpaTransform *transforms[3];
    SpaTransform *extraVectors[2];
};

extern "C" int matrix_layout_test() {
    SpaFileHeader file;
    const char *base = reinterpret_cast<const char*>(&file);
    if (sizeof(file) != 0x58 ||
        reinterpret_cast<const char*>(&file.m_duration) - base != 0x14 ||
        reinterpret_cast<const char*>(&file.m_nodeCount) - base != 0x4c ||
        reinterpret_cast<const char*>(&file.m_nodes) - base != 0x50 ||
        reinterpret_cast<const char*>(file.m_inlineNodes) - base != 0x54) return 1;
    AnimationNodeRecord record;
    SpaNodeAnimation &node = record.node;
    base = reinterpret_cast<const char*>(&node);
    if (sizeof(node) != 0x34 ||
        reinterpret_cast<const char*>(&node.m_visibilityTrack) - base != 0x4 ||
        reinterpret_cast<const char*>(&node.m_transformCount) - base != 0x8 ||
        reinterpret_cast<const char*>(&node.m_transforms) - base != 0xc ||
        reinterpret_cast<const char*>(&node.m_extraVectorTransformCount) - base != 0x14 ||
        reinterpret_cast<const char*>(&node.m_extraVectorTransforms) - base != 0x18 ||
        reinterpret_cast<const char*>(&node.m_shapeWeightTrackCount) - base != 0x2c ||
        reinterpret_cast<const char*>(node.m_shapeWeightTracks) - base != 0x30) return 2;

    SpaTransform first, second, third;
    record.transforms[0] = &first;
    record.transforms[1] = &second;
    record.transforms[2] = &third;
    record.extraVectors[0] = &second;
    record.extraVectors[1] = &first;
    node.m_shapeWeightTrackCount = 2;
    node.m_transformCount = 3;
    node.m_extraVectorTransformCount = 2;
    node.BindInlineTables();
    if (node.m_transforms != record.transforms ||
        node.m_extraVectorTransforms != record.extraVectors) return 3;
    if (node.m_transforms[0] != &first || node.m_transforms[2] != &third ||
        node.m_extraVectorTransforms[0] != &second || node.m_extraVectorTransforms[1] != &first) return 4;
    // Optimization decreases the active count without moving later tables.
    node.m_transformCount = 1;
    if (node.m_extraVectorTransforms != record.extraVectors) return 5;

    SpaNodeAnimation empty;
    empty.m_shapeWeightTrackCount = 0;
    empty.m_transformCount = 0;
    empty.m_extraVectorTransformCount = 0;
    empty.BindInlineTables();
    base = reinterpret_cast<const char*>(&empty);
    if (reinterpret_cast<const char*>(empty.m_transforms) - base != 0x30 ||
        reinterpret_cast<const char*>(empty.m_extraVectorTransforms) - base != 0x30) return 6;
    return 0;
}
