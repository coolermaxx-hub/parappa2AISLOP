#ifndef PRLIB_MODEL_H
#define PRLIB_MODEL_H

#include "common.h"

#include "linkedlist.h"
#include "microprogram.h"
#include "objectset.h"

#include <eetypes.h>
#include <libdma.h>

#include <nalib/navector.h>
#include <nalib/namatrix.h>

#define SPM_MAGIC   (0x18df540a)
#define SPM_VERSION (5)

class PrModelObject;
class PrSceneObject;

class SpmFileHeader;
class SpaFileHeader;

struct PrVuNodeHeaderDmaPacket {
    sceDmaTag m_tag;
    NaMATRIX<float, 4, 4> m_matrix;
    PR_PADDING(unk50, 0x8);
    float m_contour_blur_alpha[2];
    PrMICRO_PROGRAM_MODULE unk60;
    PR_PADDING(unk64, 0x4);
    float unk68;
    PR_PADDING(unk6C, 0x4);
    float unk70;
    float unk74;
    PR_PADDING(unk78, 0x11C);
    int unk194;
};

/*
 * Skinning weights of a cluster node: for each vertex, a count followed by
 * that many (node index, weight) pairs.
 */
union SpmClusterWeight {
    u_int count;
    u_int node;
    float weight;
};

/* Which source vertex feeds which slot of the contour packet */
struct SpmContourIndex {
    u_int m_src;
    u_int m_dst;
};

struct SpmNode {
public:
    void ChangePointer(SpmFileHeader *arg0, SpmNode *arg1);

    void ModifySimpleDmaPacket(PrVuNodeHeaderDmaPacket *packet);

    void RenderContext1Node(PrModelObject *model);
    void RenderScreenModelNode();
    void RenderBackgroundScreenModel();
    void RenderContext2Node(PrModelObject *model);

    void ComposeGlobalMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);
    void ComposeGlobalMatrixWithoutVisibility(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);
    void ComposeGlobalMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);

    void BlendTransitionMatrix(PrModelObject *model, NaMATRIX<float, 4, 4>& arg1);

    void ApplyBillboardMatrix();

public:
    NaMATRIX<float, 4, 4> unk0;
    NaMATRIX<float, 4, 4> unk40;
    NaMATRIX<float, 4, 4> unk80;
    NaMATRIX<float, 4, 4> unkC0;
    PR_PADDING(unk100, 0x40);
    NaVECTOR<float, 4> unk140;
    int unk150;
    u_int m_flags;
    SpmFileHeader *unk158;
    SpmNode *unk15C;
    SpmNode *unk160;
    SpmNode *unk164;
    PR_PADDING(unk168, 0x4);
    PrVuNodeHeaderDmaPacket *unk16C[2];
    PR_PADDING(unk174, 0x8);
    PrVuNodeHeaderDmaPacket *unk17C;
    float unk180;
    float unk184;
    u_int unk188;
    PR_PADDING(unk18C, 0x8);
    /* Cluster and shape nodes: vertex count, and for each vertex the number of
       VU vertex slots it feeds followed by those slot indices */
    u_int m_vertex_num;
    u_int *m_vertex_index;
    u_int m_contour_index_num;
    SpmContourIndex *m_contour_index;
    PrVuNodeHeaderDmaPacket *m_contour_packet;
};

/* SpmNode::m_flags: which subclass a node is */
enum SpmNodeType {
    SPM_NODE_CLUSTER = 0x10,
    SPM_NODE_SHAPE = 0x20,
    SPM_NODE_CONTOUR = 0x40,
};

enum SpmFlags {
    eSpmIsScreenModel = 0x80,
};

class SpmFileHeader {
public:
    void ChangePointer();

    void CalculateCurrentMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);
    void CalculateCurrentMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);

    void CalculateClusterMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);
    void CalculateClusterMatrixAnimation(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);

    void RenderContext1Model(PrModelObject *model);
    void RenderScreenModelNode();
    void RenderBackgroundScreenModel();
    void RenderContext2Model(PrModelObject *model);

    template <typename T>
    T* CalculatePointer(T *offset) {
        if (!offset) {
            return NULL;
        }
        return reinterpret_cast<T*>(reinterpret_cast<int>(this) + reinterpret_cast<int>(offset));
    }

public:
    u_int m_magic;
    u_short m_version;
    u_short m_flags;
    PR_PADDING(unk8, 0x8);
    char m_name[32];
    NaVECTOR<float, 4> unk30;
    NaVECTOR<float, 4> unk40;
    PrModelObject *unk50;
    PR_PADDING(unk54, 0x8);
    int m_vertex_num;
    PR_PADDING(unk60, 0x4);
    int *unk64;
    u_int m_node_num;
    u_int unk6C;
    int unk70;
    SpmNode **m_nodes;
    int unk78;
};

/* A skinned node: each vertex is a weighted blend of other nodes' matrices */
class SpmClusterGeometryNode : public SpmNode {
public:
    void RenderClusterNode(PrModelObject *model);

public:
    PR_PADDING(unk1A8, 0xC);
    SpmClusterWeight *m_weights;
    NaVECTOR<float, 4> *m_positions;
};

/* A morph-target node: base vertices plus weighted per-target offsets */
class SpmShapeNode : public SpmNode {
public:
    void AddShapePosition(u_int target, float weight);
    void RenderShapeNode(PrModelObject *model);

    float BlendTransactionWeight(PrModelObject *model, float weight, u_int index);

public:
    PR_PADDING(unk1A8, 0x8);
    /* Offsets are stored per vertex, one for each target */
    u_int m_target_num;
    u_long128 *m_base_vertices;
    /* Where this node's target weights start in PrModelObject's posture weights */
    u_int m_weight_index;
    PR_PADDING(unk1BC, 0x4);
    u_long128 m_target_offsets[1];
};


/* A node that draws an outline (SPM_NODE_CONTOUR) */
class SpmComplexNode : public SpmNode {
public:
    void SaveContour(PrModelObject *model);
    void RenderContour(PrModelObject *model);
};

class PrModelObject {
public:
    PrModelObject(SpmFileHeader *spm);
    ~PrModelObject();

    void Initialize();

    void LinkAnimation(SpaFileHeader *animation);
    void CleanupAnimation();

    void LinkPositionAnimation(SpaFileHeader *animation);
    void CleanupPositionAnimation();

    void UnionBoundaryBox(NaVECTOR<float, 4> *arg0, NaVECTOR<float, 4> *arg1);

    void GetPrimitivePosition(NaVECTOR<float, 4> *position);
    void GetScreenPosition(NaVECTOR<float, 4> *position);

    void CalculateCurrentMatrix();

    void RenderContext1Model();
    void RenderScreenModelNode();
    void RenderBackgroundScreenModel();
    void RenderContext2Model();

    void SavePosture();
    void ResetPosture();

    void SaveContour();
    void ResetContour();

public:
    PrLinkedList<PrModelObject> m_list;
    PrObjectSet<PrModelObject> *m_obj_set;
    PrSceneObject *m_linked_scene;
    NaMATRIX<float, 4, 4> unk10;
    u_int unk50;
    void *m_user_data;
    SpmFileHeader *m_spm_image;
    u_int m_flags;
    float m_animation_time;
    float m_position_animation_time;
    SpaFileHeader *m_animation;
    SpaFileHeader *m_position_animation;
    int m_active_transition;
    float *unk74[2];
    NaMATRIX<float, 4, 4> *unk7C[2];
    int m_rendered_once;
    int *unk88;
    int *unk8C;
    int unk90;
    float m_contour_blur_alpha[2];
    float m_transaction_blend_ratio;
    float m_disturbance;
    float unkA4;
    PR_PADDING(unkA8, 0x8);
};

#endif /* PRLIB_MODEL_H */
