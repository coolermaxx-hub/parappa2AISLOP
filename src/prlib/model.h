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
    PR_PADDING(unk50, 0x10);
    PrMICRO_PROGRAM_MODULE unk60;
    PR_PADDING(unk64, 0x4);
    float unk68;
    PR_PADDING(unk6C, 0x128);
    int unk194;
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

    void BlendTransitionMatrix(PrModelObject *model, NaMATRIX<float, 4, 4>& arg1);

    void ApplyBillboardMatrix();

public:
    PR_PADDING(unk0, 0x40);
    NaMATRIX<float, 4, 4> unk40;
    PR_PADDING(unk80, 0xc0);
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
    PR_PADDING(unk180, 0x8);
    u_int unk188;
    PR_PADDING(unk18C, 0xc);
    int *unk198;
    PR_PADDING(unk19C, 0x4);
    int *unk1A0;
    PrVuNodeHeaderDmaPacket *unk1A4;
    PR_PADDING(unk1A8, 0xc);
    int *unk1B4;
    int *unk1B8;
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
    PR_PADDING(unk50, 0xC);
    int m_vertex_num;
    PR_PADDING(unk60, 0x4);
    int *unk64;
    u_int m_node_num;
    PR_PADDING(unk6C, 0x4);
    int unk70;
    SpmNode **m_nodes;
    int unk78;
};

class SpmClusterGeometryNode {
public:
    void RenderClusterNode(PrModelObject *model);
};

class SpmShapeNode {
public:
    void RenderShapeNode(PrModelObject *model);
};

class SpmComplexNode {
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
    PR_PADDING(unk50, 0x4);
    void *m_user_data;
    SpmFileHeader *m_spm_image;
    u_int m_flags;
    float m_animation_time;
    float m_position_animation_time;
    SpaFileHeader *m_animation;
    SpaFileHeader *m_position_animation;
    int m_active_transition;
    PR_PADDING(unk74, 0x10);
    int m_rendered_once;
    PR_PADDING(unk88, 0xC);
    float m_contour_blur_alpha[2];
    float m_transaction_blend_ratio;
    float m_disturbance;
    PR_PADDING(unkA4, 0xC);
};

#endif /* PRLIB_MODEL_H */
