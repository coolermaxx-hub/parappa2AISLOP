#include "scene.h"

#include "camera.h"
#include "model.h"

#include <float.h>

/* data */
extern char D_0038C720[]; /* "(noname)" */

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/scene", __13PrSceneObjectP13sceGsDrawEnv1PCcUi);
#else /* Codegen differs (102 vs 106 instructions) */
PrSceneObject::PrSceneObject(sceGsDrawEnv1 *draw_env, const char *name, u_int work_fbp) {
    m_list.next = NULL;
    m_list.prev = NULL;
    m_obj_set = NULL;
    m_magic = 0x19832B1A;

    m_camera = NULL;
    m_camera_time = 0.0f;
    m_name = D_0038C720;
    m_default_focal_len = 0.0f;
    m_default_defocus_len = 0.0f;
    m_default_depth_level = 3;
    m_dbuff = NULL;
    m_work_fbp = work_fbp;

    m_draw_env = draw_env;
    m_frame = draw_env->frame1;
    m_width = draw_env->frame1.FBW * 64;
    m_xyoffset = draw_env->xyoffset1;
    m_height = (2048 - (draw_env->xyoffset1.OFY >> 4)) * 2;

    m_default_camera.position.Set(m_camera_time, m_camera_time, 1000.0f, 1.0f);
    m_default_camera.interest.Set(m_camera_time, m_camera_time, m_camera_time, 1.0f);
    m_default_camera.up.Set(m_camera_time, 1.0f, m_camera_time, 1.0f);
    m_default_camera.aspect = 1.0f;
    m_default_camera.field_of_view = 1.0471976f;
    m_default_camera.near_clip = 100.0f;
    m_default_camera.far_clip = 1000000.0f;

    m_normal_model_list = NULL;
    m_screen_model_list = NULL;
    m_flag400_model_list = NULL;
}
#endif

PrSceneObject::~PrSceneObject() {
    /* Empty */
}

void PrSceneObject::SelectCamera(SpcFileHeader *camera) {
    m_camera = camera;
    m_camera_time = 0.0f;
}

PrPERSPECTIVE_CAMERA* PrSceneObject::GetCurrentCamera() {    
    if (m_camera != NULL) {
        return m_camera->GetCamera(m_camera_time);  
    } else {
        return &m_default_camera;
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/scene", SetAppropriateDefaultCamera__13PrSceneObject);
#else /* Regalloc and scheduling */
void PrSceneObject::SetAppropriateDefaultCamera() {
    /* static const float in .sdata */
    extern float scene_bbox_max_init[];
    extern float scene_bbox_min_init[];

    NaVECTOR<float, 4> min;
    NaVECTOR<float, 4> max;
    float init;

    init = scene_bbox_max_init[0];
    for (int i = 0; i < 4; i++) {
        ((float*)&min)[i] = init;
    }
    init = scene_bbox_min_init[0];
    for (int i = 0; i < 4; i++) {
        ((float*)&max)[i] = init;
    }

    for (PrModelObject *model = m_model_set.m_head; model != NULL; model = model->m_list.next) {
        if (!(model->m_flags & 0x1)) {
            continue;
        }

        u_short flags = model->m_spm_image->m_flags;
        if (flags & 0x80) {
            continue;
        }
        if (flags & 0x200) {
            continue;
        }

        model->UnionBoundaryBox(&min, &max);
    }

    NaVECTOR<float, 4> size = (max - min) / 2.0f;
    if (size[0] < 0.0f || size[1] < 0.0f || size[2] < 0.0f) {
        return;
    }

    NaVECTOR<float, 4> center = (min + max) / 2.0f;
    NaVECTOR<float, 4> dir;
    dir.Set(0.0f, 0.0f, -1.0f, 1.0f);

    float r = (size[2] <= size[1]) ? size[1] : size[2];
    NaVECTOR<float, 4> eye = center - dir * (size[2] + r * 1.7320508f * 1.3f);

    m_default_camera.position.Set(eye[0], eye[1], eye[2], 1.0f);
    m_default_camera.interest.Set(center[0], center[1], center[2], 1.0f);

    float depth = eye[2] - center[2];
    m_default_camera.up.Set(0.0f, 1.0f, 0.0f, 1.0f);

    m_default_camera.aspect = 1.0f;
    m_default_camera.field_of_view = 1.0471976f;
    m_default_camera.near_clip = depth / 30000.0f;
    m_default_camera.far_clip = depth * 30.0f;
}
#endif

float PrSceneObject::GetFocalLength() const {
    SpcFileHeader *camera = m_camera;
    if (camera == NULL || !(camera->m_flags & 0x8) ) {
        return m_default_focal_len;
    } else {
        float focal_length = *camera->m_focal_len_track->GetValue(m_camera_time);
        if (focal_length <= 0.0f) {
            focal_length = FLT_EPSILON;
        }
        return focal_length;
    }
}

float PrSceneObject::GetDefocusLength() const {
    SpcFileHeader *camera = m_camera;
    if (camera == NULL || !(camera->m_flags & 0x8)) {
        return m_default_defocus_len;
    } else {
        return *camera->m_defocus_len_track->GetValue(m_camera_time);
    }
}

u_int PrSceneObject::GetDepthLevel() const {
    SpcFileHeader *camera = m_camera;
    if (camera == NULL || !(camera->m_flags & 0x8)) {
        return m_default_depth_level;
    } else {
        return camera->m_depth_level;
    }
}

void PrSceneObject::PreprocessModel() {
    PrModelObject *sp = NULL;
    PrModelObject *model = m_model_set.m_head;

    PrModelObject *model_list = NULL;
    PrModelObject *screen_list = NULL;
    PrModelObject *t1 = NULL;

    while (model != NULL) {
        SpmFileHeader *spm = model->m_spm_image;
        PrModelObject *next = model->m_list.next;
        if (spm->m_flags & eSpmIsScreenModel) {
            model->m_list.next = screen_list;
            screen_list = model;
        } else if (spm->m_flags & 0x200) {
            PrModelObject *a1 = sp;
            PrModelObject **a3 = &sp;
            u_int t0_1 = spm->m_sort_order;
            while (a1 != NULL && a1->m_spm_image->m_sort_order < t0_1) {
                a3 = (PrModelObject**)a1;
                a1 = *a3;
            }
            model->m_list.next = a1;
            *a3 = model;
        } else if (spm->m_flags & 0x400) {
            model->m_list.next = t1;
            t1 = model;
        } else {
            model->m_list.next = model_list;
            model_list = model;
        }
        model = next;
    }

    PrModelObject *head = NULL;
    PrModelObject *tail = NULL;

    m_screen_model_list = screen_list;
    if (model_list != NULL) {
        this->m_normal_model_list = model_list;
    } else {
        this->m_normal_model_list = screen_list;
    }

    if (t1 != NULL) {
        this->m_flag400_model_list = t1;
    } else {
        this->m_flag400_model_list = this->m_normal_model_list;
    }

    PrModelObject *v1 = sp;
    if (v1 != NULL) {
        head = v1;
        while (sp != NULL) {
            PrModelObject *v0;
            sp = v1->m_list.next;
            v1->m_list.prev = tail;
            tail = v1;
            v0 = sp;
            v1 = v0;
        }
    }

    if (t1 != NULL) {
        if (head == NULL) {
            head = t1;
        } else {
            tail->m_list.next = t1;
        }

        do {
            PrModelObject *model = t1;
            t1 = t1->m_list.next;
            model->m_list.prev = tail;
            tail = model;
        } while (t1 != NULL);
    }

    if (model_list != NULL) {
        if (head == NULL) {
            head = model_list;
        } else {
            tail->m_list.next = model_list;
        }

        while (model_list != NULL) {
            PrModelObject *model = model_list;
            model_list = model_list->m_list.next;
            model->m_list.prev = tail;
            tail = model;
        }
    }

    if (screen_list != NULL) {
        if (head == NULL) {
            head = screen_list;
        } else {
            tail->m_list.next = screen_list;
        }

        do {
            PrModelObject *model = screen_list;
            screen_list = screen_list->m_list.next;
            model->m_list.prev = tail;
            tail = model;
        } while (screen_list != NULL);
    }

    m_model_set.m_head = head;
    m_model_set.m_tail = tail;
}

/* nalib/navector.h: weak copies of NaVECTOR<float, 4>::Set and the 4-argument constructor */
#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/scene", func_0014B988);

INCLUDE_ASM("asm/nonmatchings/prlib/scene", func_0014B9B0);
#endif
