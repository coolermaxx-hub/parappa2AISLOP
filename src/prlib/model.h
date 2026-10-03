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
    float m_eeDepthBias;
    float m_eeColorScale;
    float m_contour_blur_alpha[2];
    PrMICRO_PROGRAM_MODULE m_microprogram;
    PR_PADDING(unk64, 0x4);
    float m_disturbance;
    PR_PADDING(unk6C, 0x4);
    float m_textureOffsetU;
    float m_textureOffsetV;
    PR_PADDING(unk78, 0x11C);
    int m_microprogramCall;

    NaVECTOR<float, 4>& PositionAtQuadword(u_int index) {
        // SPM geometry tables contain absolute quadword indices into the
        // variable DMA packet. The vertex portion follows its VIF/header data.
        return reinterpret_cast<NaVECTOR<float, 4>*>(this)[index];
    }
};

// Each deformed source position fans out to a variable number of quadword
// indices in the geometry DMA packet. A zero-count record occupies one word.
struct SpmPositionTargets {
    u_int count;
    u_int quadwordIndices[1];

    const SpmPositionTargets *Next() const {
        return reinterpret_cast<const SpmPositionTargets*>(quadwordIndices + count);
    }
};

// Serialized cluster streams contain a count followed by (node, weight)
// pairs for each position. Records have variable length, without padding.
struct SpmClusterInfluence {
    u_int nodeIndex;
    float weight;
};

struct SpmClusterInfluences {
    u_int count;
    SpmClusterInfluence influences[1];

    const SpmClusterInfluences *Next() const {
        return reinterpret_cast<const SpmClusterInfluences*>(influences + count);
    }
};

struct SpmClusterData {
    u_int reserved;
    SpmClusterInfluences *influences;
    NaVECTOR<float, 4> *positions;
};

struct SpmShapeData {
    u_int stride;
    NaVECTOR<float, 4> *basePositions;
    u_int postureWeightOffset;
};

struct SpmContourIndex {
    u_int m_src;
    u_int m_dst;
};

struct SpmNode {
public:
    void ChangePointer(SpmFileHeader *file, SpmNode *parent);

    void ModifySimpleDmaPacket(PrVuNodeHeaderDmaPacket *packet);

    void RenderContext1Node(PrModelObject *model);
    void RenderScreenModelNode();
    void RenderBackgroundScreenModel();
    void RenderContext2Node(PrModelObject *model);

    void ComposeGlobalMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);
    void ComposeAnimatedMatrix(PrModelObject *model, const NaMATRIX<float, 4, 4>& parentMatrix);
    void ComposeGlobalMatrixWithoutVisibility(PrModelObject *model, const NaMATRIX<float, 4, 4>& arg1);

    void BlendTransitionMatrix(PrModelObject *model, NaMATRIX<float, 4, 4>& arg1);

    void ApplyBillboardMatrix();

public:
    NaMATRIX<float, 4, 4> m_localMatrix;
    NaMATRIX<float, 4, 4> m_worldMatrix;
    // Original skinning composition: worldMatrix * bindCorrectionMatrix.
    // An inverse-bind interpretation still needs original model assets.
    NaMATRIX<float, 4, 4> m_bindCorrectionMatrix;
    NaMATRIX<float, 4, 4> m_skinningMatrix;
    PR_PADDING(unk100, 0x40);
    NaVECTOR<float, 4> m_sortPosition;
    int m_animationIndex;
    u_int m_flags;
    SpmFileHeader *m_owner;
    SpmNode *m_firstChild;
    SpmNode *m_nextSibling;
    SpmNode *m_parent;
    PR_PADDING(unk168, 0x4);
    // Opaque packet first, depth-sorted translucent packet second.
    PrVuNodeHeaderDmaPacket *m_context1Packets[2];
    PR_PADDING(unk174, 0x8);
    PrVuNodeHeaderDmaPacket *m_geometryPacket;
    float m_textureScrollU;
    float m_textureScrollV;
    u_int m_sortGroup;
    PR_PADDING(unk18C, 0x8);
    u_int m_deformPositionCount;
    SpmPositionTargets *m_positionTargets;
    u_int m_contourCount;
    SpmContourIndex *m_contourIndices;
    PrVuNodeHeaderDmaPacket *m_contourPacket;
    PR_PADDING(unk1A8, 0x8);
    // m_flags selects cluster (0x10) or shape (0x20) payloads.
    union {
        SpmClusterData m_cluster;
        SpmShapeData m_shape;
    };
    u_int m_reserved1BC;
};

enum SpmFlags {
    eSpmIsScreenModel = 0x80,
    eSpmVisible = 0x4000,
    eSpmDefaultHidden = 0x20000,
    eSpmAnimatedVisibility = 0x40000,
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

class SpmClusterGeometryNode : public SpmNode {
public:
    void RenderClusterNode(PrModelObject *model);
};

class SpmShapeNode : public SpmNode {
public:
    void AddShapePosition(u_int shapeIndex, float weight);
    void RenderShapeNode(PrModelObject *model);
    float BlendTransactionWeight(PrModelObject *model, float weight, u_int index);

    // The node's inline tail is a position-major array of shape deltas.
    NaVECTOR<float, 4> m_shapeDeltas[1];
};

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
    float *m_postureWeights[2];
    NaMATRIX<float, 4, 4> *m_postureMatrices[2];
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
