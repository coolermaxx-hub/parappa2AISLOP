#include "scene.h"

#include "camera.h"
#include "model.h"

#include <float.h>

/* data */
static char defaultSceneName[] = "(noname)";

PrSceneObject::PrSceneObject(sceGsDrawEnv1 *draw_env, const char *name, u_int workFbp) {
    m_list.next = NULL;
    m_list.prev = NULL;
    m_obj_set = NULL;
    m_signature = PR_SCENE_SIGNATURE;

    m_camera = NULL;
    m_camera_time = 0.0f;
    m_name = defaultSceneName;
    m_default_focal_len = 0.0f;
    m_default_defocus_len = 0.0f;
    m_default_depth_level = 3;
    m_dbuff = NULL;
    m_workFbp = workFbp;

    m_drawEnv = draw_env;
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

    m_normalModelList = NULL;
    m_screen_model_list = NULL;
    m_flag400ModelList = NULL;
}

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

void PrSceneObject::SetAppropriateDefaultCamera() {
    static float boundsMinInit = 3.4028235e38f;
    static float boundsMaxInit = -3.4028235e38f;

    NaVECTOR<float, 4> min;
    NaVECTOR<float, 4> max;
    float init;

    init = boundsMinInit;
    for (int i = 0; i < 4; i++) {
        min[i] = init;
    }
    init = boundsMaxInit;
    for (int i = 0; i < 4; i++) {
        max[i] = init;
    }

    for (PrModelObject *model = m_model_set.m_head; model != NULL; model = model->m_list.next) {
        if (!(model->m_flags & ePrModelEnabled)) {
            continue;
        }

        u_short flags = model->m_spm_image->m_flags;
        if (flags & eSpmFileScreenModel) {
            continue;
        }
        if (flags & eSpmFileBackgroundLayer) {
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

float PrSceneObject::GetFocalLength() const {
    SpcFileHeader *camera = m_camera;
    if (camera == NULL || !(camera->m_flags & eSpcFileHasFocusTracks) ) {
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
    if (camera == NULL || !(camera->m_flags & eSpcFileHasFocusTracks)) {
        return m_default_defocus_len;
    } else {
        return *camera->m_defocus_len_track->GetValue(m_camera_time);
    }
}

u_int PrSceneObject::GetDepthLevel() const {
    SpcFileHeader *camera = m_camera;
    if (camera == NULL || !(camera->m_flags & eSpcFileHasFocusTracks)) {
        return m_default_depth_level;
    } else {
        return camera->m_depth_level;
    }
}

void PrSceneObject::PreprocessModel() {
    PrModelObject *background_list = NULL; /* sorted by m_sortOrder, ascending */
    PrModelObject *model = m_model_set.m_head;

    PrModelObject *model_list = NULL;
    PrModelObject *screen_list = NULL;
    PrModelObject *prescene_list = NULL;

    while (model != NULL) {
        SpmFileHeader *spm = model->m_spm_image;
        PrModelObject *next = model->m_list.next;
        if (spm->m_flags & eSpmFileScreenModel) {
            model->m_list.next = screen_list;
            screen_list = model;
        } else if (spm->m_flags & eSpmFileBackgroundLayer) {
            PrModelObject *cursor = background_list;
            PrModelObject **link = &background_list;
            u_int sortOrder = spm->m_sortOrder;
            while (cursor != NULL && cursor->m_spm_image->m_sortOrder < sortOrder) {
                link = &cursor->m_list.next;
                cursor = *link;
            }
            model->m_list.next = cursor;
            *link = model;
        } else if (spm->m_flags & eSpmFilePreSceneLayer) {
            model->m_list.next = prescene_list;
            prescene_list = model;
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
        this->m_normalModelList = model_list;
    } else {
        this->m_normalModelList = screen_list;
    }

    if (prescene_list != NULL) {
        this->m_flag400ModelList = prescene_list;
    } else {
        this->m_flag400ModelList = this->m_normalModelList;
    }

    PrModelObject *node = background_list;
    if (node != NULL) {
        head = node;
        while (background_list != NULL) {
            PrModelObject *next;
            background_list = node->m_list.next;
            node->m_list.prev = tail;
            tail = node;
            next = background_list;
            node = next;
        }
    }

    if (prescene_list != NULL) {
        if (head == NULL) {
            head = prescene_list;
        } else {
            tail->m_list.next = prescene_list;
        }

        do {
            PrModelObject *model = prescene_list;
            prescene_list = prescene_list->m_list.next;
            model->m_list.prev = tail;
            tail = model;
        } while (prescene_list != NULL);
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
