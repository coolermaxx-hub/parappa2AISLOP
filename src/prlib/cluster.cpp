#include "common.h"

#include "model.h"
#include "renderstuff.h"
#include "spram.h"

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/cluster", RenderClusterNode__22SpmClusterGeometryNodeP13PrModelObject);
#else
/* Register allocation: t2/t3 swapped for the second packet and its matrix */
static PrSPRAM_DATA *prSpramData = (PrSPRAM_DATA*)0x70000000;

void SpmClusterGeometryNode::RenderClusterNode(PrModelObject *model) {
    u_int vertex = (u_int)this->unk17C;
    vertex |= 0x30000000;
    SpmNode **nodes = this->unk158->m_nodes;
    u_int num = this->unk194;
    const SpmClusterWeight *weight = this->m_cluster_weights;
    u_int *index = (u_int*)this->unk198;
    NaVECTOR<float, 4> *position = (NaVECTOR<float, 4>*)this->unk1B8;

    for (u_int i = 0; i < num; i++) {
        u_int weight_num = (weight++)->count;

        asm volatile("vsub.xyzw $vf17, $vf0, $vf0");
        NaVECTOR<float, 4> *p = position;
        position++;
        asm volatile("lqc2 $vf18, 0x0(%0)" : : "r"(p));

        for (u_int j = 0; j < weight_num; j++) {
            u_int node = (weight++)->node;
            float w = (weight++)->weight;
            asm volatile(
                "lqc2         $vf13,  0x0(%0)            \n\t"
                "lqc2         $vf14,  0x10(%0)           \n\t"
                "lqc2         $vf15,  0x20(%0)           \n\t"
                "lqc2         $vf16,  0x30(%0)           \n\t"
            : : "r"(&nodes[node]->unkC0));
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

        u_int index_num = *index++;
        for (u_int j = 0; j < index_num; j++) {
            u_int idx = *index++;
            asm volatile("sqc2 $vf17, 0x0(%0)" : : "r"((idx << 4) + vertex));
        }
    }

    PrVuNodeHeaderDmaPacket *packet = this->unk16C[0];
    if (packet != NULL) {
        PrVuNodeHeaderDmaPacket *uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(packet);
        uc->m_matrix = NaMATRIX<float, 4, 4>::IDENT;
        uc->unk68 = prSpramData->m_disturbance * model->unkA4;
        prRenderStuff.RenderNodeEECore((PrVuNodeHeaderDmaPacket*)((u_int)uc & 0x0FFFFFFF));
    }

    PrVuNodeHeaderDmaPacket *uc = this->unk16C[1];
    if (uc != NULL) {
        uc = (PrVuNodeHeaderDmaPacket*)PR_UNCACHEDACCEL(uc);
        weight = this->m_cluster_weights;
        u_int weight_num = (weight++)->count;

        asm volatile("vsub.xyzw $vf17, $vf0, $vf0");
        asm volatile("lqc2 $vf18, 0x0(%0)" : : "r"(&this->unk140));

        for (u_int j = 0; j < weight_num; j++) {
            u_int node = (weight++)->node;
            float w = (weight++)->weight;
            asm volatile(
                "lqc2         $vf13,  0x0(%0)            \n\t"
                "lqc2         $vf14,  0x10(%0)           \n\t"
                "lqc2         $vf15,  0x20(%0)           \n\t"
                "lqc2         $vf16,  0x30(%0)           \n\t"
            : : "r"(&nodes[node]->unkC0));
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
        uc->unk68 = prSpramData->m_disturbance * model->unkA4;

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
        : : "r"(&pos), "r"(&prSpramData->m_view_projection_matrix));

        float z;
        if (pos[3] == 0.0f) {
            z = pos[2] * 3.40282347e+38f;
        } else {
            z = pos[2] / pos[3];
        }

        prRenderStuff.AppendTransmitDmaTag((sceDmaTag*)((u_int)uc & 0x0FFFFFFF), this->unk188, z);
    }
}
#endif
