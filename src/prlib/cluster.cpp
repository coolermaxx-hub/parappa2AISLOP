#include "common.h"

#include "model.h"
#include "renderstuff.h"
#include "spram.h"

static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)0x70000000;

void SpmClusterGeometryNode::RenderClusterNode(PrModelObject *model) {
    PrVuNodeHeaderDmaPacket *vertices = reinterpret_cast<PrVuNodeHeaderDmaPacket*>(PR_UNCACHEDACCEL(m_geometryPacket));
    SpmNode **nodes = this->m_owner->m_nodes;
    u_int num = this->m_deformPositionCount;
    const SpmClusterInfluences *weights = m_cluster.influences;
    const SpmPositionTargets *targets = m_positionTargets;
    NaVECTOR<float, 4> *position = m_cluster.positions;

    for (u_int i = 0; i < num; i++) {
        u_int weight_num = weights->count;

        asm volatile("vsub.xyzw $vf17, $vf0, $vf0");
        NaVECTOR<float, 4> *p = position;
        position++;
        asm volatile("lqc2 $vf18, 0x0(%0)" : : "r"(p) : "memory");

        for (u_int j = 0; j < weight_num; j++) {
            const SpmClusterInfluence &influence = weights->influences[j];
            u_int node = influence.nodeIndex;
            float w = influence.weight;
            asm volatile(
                "lqc2         $vf13,  0x0(%0)            \n\t"
                "lqc2         $vf14,  0x10(%0)           \n\t"
                "lqc2         $vf15,  0x20(%0)           \n\t"
                "lqc2         $vf16,  0x30(%0)           \n\t"
            : : "r"(&nodes[node]->m_skinningMatrix) : "memory");
            asm volatile(
                "qmtc2.ni     %0,     $vf4               \n\t"
                "vmulx.xyzw   $vf4,   $vf18,  $vf4x      \n\t"
                "vmulax.xyzw  ACC,    $vf13,  $vf4x      \n\t"
                "vmadday.xyzw ACC,    $vf14,  $vf4y      \n\t"
                "vmaddaz.xyzw ACC,    $vf15,  $vf4z      \n\t"
                "vmaddaw.xyzw ACC,    $vf16,  $vf4w      \n\t"
                "vmaddw.xyzw  $vf17,  $vf17,  $vf0w      \n\t"
            : : "r"(w));
        }

        weights = weights->Next();
        const u_int targetCount = targets->count;
        for (u_int j = 0; j < targetCount; j++) {
            const u_int idx = targets->quadwordIndices[j];
            asm volatile("sqc2 $vf17, 0x0(%0)" : : "r"(&vertices->PositionAtQuadword(idx)) : "memory");
        }
        targets = targets->Next();
    }

    PrVuNodeHeaderDmaPacket *packet = this->m_context1Packets[0];
    if (packet != NULL) {
        PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
        uc->m_matrix = NaMATRIX<float, 4, 4>::IDENT;
        uc->m_disturbance = prSpramData->m_disturbance * model->unkA4;
        prRenderStuff.RenderNodeEECore((PrVuNodeHeaderDmaPacket*)((u_int)uc & 0x0FFFFFFF));
    }

    PrVuNodeHeaderDmaPacket *uc = this->m_context1Packets[1];
    if (uc != NULL) {
        uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(uc);
        weights = m_cluster.influences;
        u_int weight_num = weights->count;

        asm volatile("vsub.xyzw $vf17, $vf0, $vf0");
        asm volatile("lqc2 $vf18, 0x0(%0)" : : "r"(&this->m_sortPosition) : "memory");

        for (u_int j = 0; j < weight_num; j++) {
            const SpmClusterInfluence &influence = weights->influences[j];
            u_int node = influence.nodeIndex;
            float w = influence.weight;
            asm volatile(
                "lqc2         $vf13,  0x0(%0)            \n\t"
                "lqc2         $vf14,  0x10(%0)           \n\t"
                "lqc2         $vf15,  0x20(%0)           \n\t"
                "lqc2         $vf16,  0x30(%0)           \n\t"
            : : "r"(&nodes[node]->m_skinningMatrix) : "memory");
            asm volatile(
                "qmtc2.ni     %0,     $vf4               \n\t"
                "vmulx.xyzw   $vf4,   $vf18,  $vf4x      \n\t"
                "vmulax.xyzw  ACC,    $vf13,  $vf4x      \n\t"
                "vmadday.xyzw ACC,    $vf14,  $vf4y      \n\t"
                "vmaddaz.xyzw ACC,    $vf15,  $vf4z      \n\t"
                "vmaddaw.xyzw ACC,    $vf16,  $vf4w      \n\t"
                "vmaddw.xyzw  $vf17,  $vf17,  $vf0w      \n\t"
            : : "r"(w));
        }

        uc->m_matrix = NaMATRIX<float, 4, 4>::IDENT;
        uc->m_disturbance = prSpramData->m_disturbance * model->unkA4;

        NaVECTOR<float, 4> pos;
        asm volatile(
            "lqc2         $vf13,  0x0(%1)            \n\t"
            "lqc2         $vf14,  0x10(%1)           \n\t"
            "lqc2         $vf15,  0x20(%1)           \n\t"
            "lqc2         $vf16,  0x30(%1)           \n\t"
            "vmulax.xyzw  ACC,    $vf13,  $vf17x     \n\t"
            "vmadday.xyzw ACC,    $vf14,  $vf17y     \n\t"
            "vmaddaz.xyzw ACC,    $vf15,  $vf17z     \n\t"
            "vmaddw.xyzw  $vf17,  $vf16,  $vf17w     \n\t"
            "sqc2         $vf17,  0x0(%0)            \n\t"
        : : "r"(&pos), "r"(&prSpramData->m_view_projection_matrix) : "memory");

        float z;
        if (pos[3] == 0.0f) {
            z = pos[2] * 3.40282347e+38f;
        } else {
            z = pos[2] / pos[3];
        }

        prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF), this->m_sortGroup, z);
    }
}
