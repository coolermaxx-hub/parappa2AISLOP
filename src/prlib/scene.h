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

class PrSceneObject {
public:
    PrSceneObject(sceGsDrawEnv1 *draw_env, const char *name, u_int work_fbp);
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
    u_int m_magic;
    PrPERSPECTIVE_CAMERA m_default_camera;
    sceGsFrame m_frame;
    sceGsXyoffset m_xyoffset;
    PrObjectSet<PrModelObject> m_model_set;
    SpcFileHeader *m_camera;
    sceGsDrawEnv1 *m_draw_env;
    u_int m_width;
    u_int m_height;
    float m_camera_time;
    char *m_name;
    float m_default_focal_len;
    float m_default_defocus_len;
    u_int m_default_depth_level;
    sceGsDBuffDc *m_dbuff;
    /* Frame buffer page for the depth of field pass; 0xFFFFFFFF disables it */
    u_int m_work_fbp;
    /*
     * The sorted model list runs: background models (by m_sort_order), then
     * models with SpmFileHeader flag 0x400, then the rest, then screen
     * models. These point at the start of each later group.
     */
    PrModelObject *m_flag400_model_list;
    PrModelObject *m_normal_model_list;
    PrModelObject *m_screen_model_list;
    PR_PADDING(unkA4, 0x8);
};

#endif /* PRLIB_SCENE_H */
