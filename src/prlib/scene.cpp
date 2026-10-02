#include "scene.h"

#include "camera.h"
#include "model.h"

#include <float.h>

INCLUDE_ASM("asm/nonmatchings/prlib/scene", __13PrSceneObjectP13sceGsDrawEnv1PCcUi);

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

INCLUDE_ASM("asm/nonmatchings/prlib/scene", SetAppropriateDefaultCamera__13PrSceneObject);

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
            u_int t0_1 = spm->unk78;
            while (a1 != NULL && a1->m_spm_image->unk78 < t0_1) {
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
        this->unk9C = model_list;
    } else {
        this->unk9C = screen_list;
    }

    if (t1 != NULL) {
        this->unk98 = t1;
    } else {
        this->unk98 = this->unk9C;
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

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/scene", func_0014B988);

INCLUDE_ASM("asm/nonmatchings/prlib/scene", func_0014B9B0);
