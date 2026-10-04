#ifndef PRLIB_SCENE_H
#define PRLIB_SCENE_H

#include "common.h"

#include "prpriv.h"
#include "objectset.h"

#include <eetypes.h>
#include <eestruct.h>
#include <libgraph.h>

class PrModelObject;
class SpcFileHeader;

#define PR_SCENE_SIGNATURE (0x19832B1A) /* PrSceneObject::m_signature, set by the constructor */

class PrSceneObject {
public:
    PrSceneObject(sceGsDrawEnv1 *draw_env, const char *name, u_int workFbp);
    ~PrSceneObject();

    void SelectCamera(SpcFileHeader *camera);
    PrPERSPECTIVE_CAMERA* GetCurrentCamera();
    void SetAppropriateDefaultCamera();

    float GetFocalLength() const;
    float GetDefocusLength() const;
    u_int GetDepthLevel() const;
    void ApplyDepthOfField();

    void PreprocessModel();

    void Render();
    void InitializeVu1();
    void PrepareScreenModelRender();

public:
    PrLinkedList<PrSceneObject> m_list;
    PrObjectSet<PrSceneObject> *m_obj_set;
    u_int m_signature;
    PrPERSPECTIVE_CAMERA m_default_camera;
    sceGsFrame m_frame;
    sceGsXyoffset m_xyoffset;
    PrObjectSet<PrModelObject> m_model_set;
    SpcFileHeader *m_camera;
    sceGsDrawEnv1 *m_drawEnv;
    u_int m_width;
    u_int m_height;
    float m_camera_time;
    char *m_name;
    float m_default_focal_len;
    float m_default_defocus_len;
    u_int m_default_depth_level;
    sceGsDBuffDc *m_dbuff;
    u_int m_workFbp;
    PrModelObject *m_flag400ModelList;
    PrModelObject *m_normalModelList;
    PrModelObject *m_screen_model_list;
    PR_PADDING(unkA4, 0x8);
};

#endif /* PRLIB_SCENE_H */
