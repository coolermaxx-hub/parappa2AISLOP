#include "prlib/model.h"
#include "prlib/renderstuff.h"

// Golden offsets come from the original ChangePointer, matrix composition and
// deformation routines, not from the reconstructed field declarations.
extern "C" int matrix_layout_test() {
    SpmNode node;
    const char *base = reinterpret_cast<const char*>(&node);
    if (sizeof(node) != 0x1c0 ||
        reinterpret_cast<const char*>(&node.m_localMatrix) - base != 0x0 ||
        reinterpret_cast<const char*>(&node.m_worldMatrix) - base != 0x40 ||
        reinterpret_cast<const char*>(&node.m_bindCorrectionMatrix) - base != 0x80 ||
        reinterpret_cast<const char*>(&node.m_skinningMatrix) - base != 0xc0) return 1;
    if (reinterpret_cast<const char*>(&node.m_sortPosition) - base != 0x140 ||
        reinterpret_cast<const char*>(&node.m_animationIndex) - base != 0x150 ||
        reinterpret_cast<const char*>(&node.m_owner) - base != 0x158 ||
        reinterpret_cast<const char*>(&node.m_firstChild) - base != 0x15c ||
        reinterpret_cast<const char*>(&node.m_nextSibling) - base != 0x160 ||
        reinterpret_cast<const char*>(&node.m_parent) - base != 0x164) return 2;
    if (reinterpret_cast<const char*>(node.m_context1Packets) - base != 0x16c ||
        reinterpret_cast<const char*>(&node.m_geometryPacket) - base != 0x17c ||
        reinterpret_cast<const char*>(&node.m_textureScrollU) - base != 0x180 ||
        reinterpret_cast<const char*>(&node.m_textureScrollV) - base != 0x184 ||
        reinterpret_cast<const char*>(&node.m_sortGroup) - base != 0x188) return 3;
    if (reinterpret_cast<const char*>(&node.m_deformPositionCount) - base != 0x194 ||
        reinterpret_cast<const char*>(&node.m_positionTargets) - base != 0x198 ||
        reinterpret_cast<const char*>(&node.m_contourPacket) - base != 0x1a4) return 4;
    PrVuNodeHeaderDmaPacket packet;
    base = reinterpret_cast<const char*>(&packet);
    if (sizeof(packet) != 0x1a0 ||
        reinterpret_cast<const char*>(&packet.m_microprogram) - base != 0x60 ||
        reinterpret_cast<const char*>(&packet.m_disturbance) - base != 0x68 ||
        reinterpret_cast<const char*>(&packet.m_textureOffsetU) - base != 0x70 ||
        reinterpret_cast<const char*>(&packet.m_textureOffsetV) - base != 0x74 ||
        reinterpret_cast<const char*>(&packet.m_microprogramCall) - base != 0x194) return 5;

    PrTransmitEntry entry;
    base = reinterpret_cast<const char*>(&entry);
    if (sizeof(entry) != 12 ||
        reinterpret_cast<const char*>(&entry.depth) - base != 0 ||
        reinterpret_cast<const char*>(&entry.sortGroup) - base != 4 ||
        reinterpret_cast<const char*>(&entry.tag) - base != 8) return 11;

    // Four serialized records: multiple targets (including a duplicate), an
    // empty record, one target, and another empty record. No alignment padding.
    const u_int serializedTargets[] = {3, 0x20, 0x26, 0x20, 0, 1, 0x35, 0};
    const SpmPositionTargets *targets =
        reinterpret_cast<const SpmPositionTargets*>(serializedTargets);
    if (targets->count != 3 || targets->quadwordIndices[0] != 0x20 ||
        targets->quadwordIndices[1] != 0x26 || targets->quadwordIndices[2] != 0x20) return 6;
    targets = targets->Next();
    if (targets->count != 0) return 7;
    targets = targets->Next();
    if (targets->count != 1 || targets->quadwordIndices[0] != 0x35) return 8;
    targets = targets->Next();
    if (targets->count != 0) return 9;
    if (reinterpret_cast<const u_int*>(targets->Next()) != serializedTargets + 8) return 10;
    return 0;
}
