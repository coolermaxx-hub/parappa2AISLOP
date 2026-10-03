#include "prpriv.h"

#include "animation.h"
#include "camera.h"
#include "database.h"
#include "model.h"
#include "random.h"
#include "renderstuff.h"
#include "scene.h"

#include <nalib/navector.h>

#include <eetypes.h>
#include <eestruct.h>
#include <libgraph.h>

#include <stdlib.h>

static float prFrameRate = 1.0f;
static float prInverseFrameRate = 1.0f;

static PrDebugParam debugParam[PR_DEBUG_PARAM_NUM];

static void InitializeDebugParam();

PR_EXTERN
void PrSetFrameRate(float frame_rate) {
    prFrameRate = frame_rate;
    prInverseFrameRate = 1.0f / frame_rate;
}

PR_EXTERN
float PrGetFrameRate() {
    return prFrameRate;
}

PR_EXTERN
void PrInitializeModule(sceGsZbuf zbuf) {
    InitializeDebugParam();
    PrInitializeRandomPool();

    prObjectDatabase.Initialize();
    prRenderStuff.Initialize(zbuf);
}

PR_EXTERN
void PrCleanupModule() {
    prRenderStuff.Cleanup();

    PrCleanupModel(NULL);
    PrCleanupAnimation(NULL);
    PrCleanupCamera(NULL);
    PrCleanupScene(NULL);

    prObjectDatabase.Cleanup();
}

PR_EXTERN
PrSceneObject* PrInitializeScene(sceGsDrawEnv1 *draw_env, const char *name, u_int fbp) {
    return prObjectDatabase.CreateScene(draw_env, name, fbp);
}

PR_EXTERN
PrSceneObject* PrInitializeSceneDBuff(sceGsDBuffDc *dbuff, const char *name, u_int fbp) {
    PrSceneObject *scene = prObjectDatabase.CreateScene(&dbuff->draw01, name, fbp);
    scene->m_dbuff = dbuff;
}

PR_EXTERN
void PrCleanupScene(PrSceneObject *scene) {
    if (scene == NULL) {
        scene = prObjectDatabase.m_scene_set.m_head;
        while (scene != NULL) {
            prObjectDatabase.DeleteScene(scene);
            scene = prObjectDatabase.m_scene_set.m_head;
        }
    } else {
        prObjectDatabase.DeleteScene(scene);
    }
}

PR_EXTERN
void PrSetSceneFrame() {
    /* Empty */
}

PR_EXTERN
void PrSetSceneEnv(PrSceneObject *scene, sceGsDrawEnv1 *draw_env) {
    scene->m_frame = draw_env->frame1;
    scene->m_xyoffset = draw_env->xyoffset1;
}

PR_EXTERN
void PrPreprocessSceneModel(PrSceneObject *scene) {
    scene->PreprocessModel();
}

PR_EXTERN
PrModelObject* PrInitializeModel(SpmFileHeader *spm, PrSceneObject *scene) {
    if (spm->m_magic != SPM_MAGIC) {
    #if 0 /* (poly): Only present on McDonald's Demo build */
        printf("PRLIB(FATAL): not a SPM file (illegal magic number)\n");
    #endif
        exit(0);
    }
    if (spm->m_version != SPM_VERSION) {
    #if 0 /* (poly): Only present on McDonald's Demo build */
        printf("PRLIB(FATAL): not supported SPM file version %d:%d\n", spm->version, SPM_VERSION);
    #endif
        exit(0);
    }

    PrModelObject *model = new PrModelObject(spm);
    model->Initialize();
    scene->m_model_set.Insert(model);
    model->m_linked_scene = scene;
    return model;
}

PR_EXTERN
SpaFileHeader* PrInitializeAnimation(SpaFileHeader *animation) {
    if (animation->m_magic != SPA_MAGIC) {
        exit(0);
    }
    if (animation->m_version != SPA_VERSION) {
        exit(0);
    }

    animation->Initialize();

    if (animation->m_obj_set == NULL) {
        animation->m_user_data = NULL;
        prObjectDatabase.AppendAnimation(animation);
    }
    return animation;
}

PR_EXTERN
SpcFileHeader* PrInitializeCamera(SpcFileHeader *camera) {
    if (camera->m_magic != SPC_MAGIC) {
        exit(0);
    }
    if (camera->m_version != SPC_VERSION) {
        exit(0);
    }

    camera->Initialize();

    if (camera->m_obj_set == NULL) {
        camera->m_user_data = NULL;
        prObjectDatabase.AppendCamera(camera);
    }
    return camera;
}

PR_EXTERN
void PrCleanupModel(PrModelObject *model) {
    if (model == NULL) {
        PrCleanupAllSceneModel(NULL);
        return;
    }
    delete model;
}

PR_EXTERN
void PrCleanupAnimation(SpaFileHeader *animation) {
    if (animation == NULL) {
        animation = prObjectDatabase.m_animation_set.m_head;
        while (animation != NULL) {
            PrCleanupAnimation(animation);
            animation = prObjectDatabase.m_animation_set.m_head;
        }
    } else if (animation->m_obj_set != NULL) {
        prObjectDatabase.DeleteAnimation(animation);
    }
}

PR_EXTERN
void PrCleanupCamera(SpcFileHeader *camera) {
    if (camera == NULL) {
        camera = prObjectDatabase.m_camera_set.m_head;
        while (camera != NULL) {
            PrCleanupCamera(camera);
            camera = prObjectDatabase.m_camera_set.m_head;
        }
    } else if (camera->m_obj_set != NULL) {
        prObjectDatabase.DeleteCamera(camera);
    }
}

PR_EXTERN
void PrCleanupAllSceneModel(PrSceneObject *scene) {
    if (scene == NULL) {
        for (scene = prObjectDatabase.m_scene_set.m_head; scene != NULL; scene = scene->m_list.next) {
            PrCleanupAllSceneModel(scene);
        }
    } else {
        PrModelObject *model = scene->m_model_set.m_head;
        while (model != NULL) {
            PrCleanupModel(model);
            model = scene->m_model_set.m_head;
        }
    }
}

PR_EXTERN
float PrGetAnimationStartFrame(SpaFileHeader *animation) {
    return 0.0f;
}

PR_EXTERN
float PrGetAnimationEndFrame(SpaFileHeader *animation) {
    return animation->m_duration * prFrameRate;
}

PR_EXTERN
float PrGetCameraStartFrame(SpcFileHeader *camera) {
    return 0.0f;
}

PR_EXTERN
float PrGetCameraEndFrame(SpcFileHeader *camera) {
    return camera->m_duration * prFrameRate;
}

PR_EXTERN
void PrSetModelUserData(PrModelObject *model, void *user_data) {
    model->m_user_data = user_data;
}

PR_EXTERN
void PrSetAnimationUserData(SpaFileHeader *animation, void *user_data) {
    animation->m_user_data = user_data;
}

PR_EXTERN
void PrSetCameraUserData(SpcFileHeader *camera, void *user_data) {
    camera->m_user_data = user_data;
}

PR_EXTERN
void* PrGetModelUserData(PrModelObject *model) {
    return model->m_user_data;
}

PR_EXTERN
void* PrGetAnimationUserData(SpaFileHeader *animation) {
    return animation->m_user_data;
}

PR_EXTERN
void* PrGetCameraUserData(SpcFileHeader *camera) {
    return camera->m_user_data;
}

PR_EXTERN
void PrLinkAnimation(PrModelObject *model, SpaFileHeader *animation) {
    model->LinkAnimation(animation);
}

PR_EXTERN
void PrUnlinkAnimation(PrModelObject *model) {
    model->LinkAnimation(NULL);
}

PR_EXTERN
SpaFileHeader* PrGetLinkedAnimation(PrModelObject *model) {
    return model->m_animation;
}

PR_EXTERN
void PrLinkPositionAnimation(PrModelObject *model, SpaFileHeader *animation) {
    model->LinkPositionAnimation(animation);
}

PR_EXTERN
void PrUnlinkPositionAnimation(PrModelObject *model) {
    model->LinkPositionAnimation(NULL);
}

PR_EXTERN
SpaFileHeader* PrGetLinkedPositionAnimation(PrModelObject *model) {
    return model->m_position_animation;
}

PR_EXTERN
void PrSelectCamera(SpcFileHeader *camera, PrSceneObject *scene) {
    scene->SelectCamera(camera);
}

PR_EXTERN
SpcFileHeader* PrGetSelectedCamera(PrSceneObject *scene) {
    return scene->m_camera;
}

PR_EXTERN
PrPERSPECTIVE_CAMERA* PrGetCurrentCamera(PrSceneObject *scene) {
    return scene->GetCurrentCamera();
}

PR_EXTERN
void PrSetDefaultCamera(PrPERSPECTIVE_CAMERA *camera, PrSceneObject *scene) {
    scene->m_default_camera = *camera;
}

PR_EXTERN
void PrSetAppropriateDefaultCamera(PrSceneObject *scene) {
    scene->SetAppropriateDefaultCamera();
}

PR_EXTERN
void PrShowModel(PrModelObject *model, NaMATRIX<float, 4, 4> *position) {
    model->m_flags |= 1;
    if (position != NULL) {
        model->m_matrix = *position;
    } else {
        model->m_matrix = NaMATRIX<float, 4, 4>::IDENT;
    }
}

PR_EXTERN
NaMATRIX<float, 4, 4>* PrGetModelMatrix(PrModelObject *model) {
    if ((model->m_flags & 1) == 0) {
        return NULL;
    }
    return &model->m_matrix;
}

PR_EXTERN
void PrHideModel(PrModelObject *model) {
    model->m_flags &= ~1;
}

PR_EXTERN
NaVECTOR<float, 4>* PrGetModelPrimitivePosition(PrModelObject *model) {
    static NaVECTOR<float, 4> vector;

    model->GetPrimitivePosition(&vector);
    return &vector;
}

PR_EXTERN
NaVECTOR<float, 4>* PrGetModelScreenPosition(PrModelObject *model) {
    static NaVECTOR<float, 4> vector;

    model->GetScreenPosition(&vector);
    return &vector;
}

int prCurrentStage = 0;

PR_EXTERN
void PrAnimateModel(PrModelObject *model, float time) {
    model->m_animation_time = time * prInverseFrameRate;
}

PR_EXTERN
void PrAnimateModelPosition(PrModelObject *model, float time) {
    model->m_position_animation_time = time * prInverseFrameRate;
}

PR_EXTERN
void PrAnimateSceneCamera(PrSceneObject *scene, float time) {
    scene->m_camera_time = time * prInverseFrameRate;
}

PR_EXTERN
void PrRender(PrSceneObject *scene) {
    scene->Render();
}

PR_EXTERN
void PrWaitRender() {
    prRenderStuff.WaitRender();
}

PR_EXTERN
void PrSetStage(int stage) {
    prCurrentStage = stage;
}

PR_EXTERN
void PrSetDepthOfField(PrSceneObject *scene, float focal_lng, float defocus_lng) {
    if (focal_lng != 0.0f) {
        if (focal_lng < 0.0f || defocus_lng <= focal_lng) {
            defocus_lng = 0.0f;
            return;
        }
    } else {
        defocus_lng = 0.0f;
    }

    scene->m_default_focal_len = focal_lng;
    scene->m_default_defocus_len = defocus_lng;
}

PR_EXTERN
void PrSetDepthOfFieldLevel(PrSceneObject *scene, u_int level) {
    scene->m_default_depth_level = level;
}

PR_EXTERN
float PrGetFocalLength(PrSceneObject *scene) {
    return scene->m_default_focal_len;
}

PR_EXTERN
float PrGetDefocusLength(PrSceneObject *scene) {
    return scene->m_default_defocus_len;
}

PR_EXTERN
u_int PrGetDepthOfFieldLevel(PrSceneObject *scene) {
    return scene->m_default_depth_level;
}

PR_EXTERN
void PrSaveContour(PrModelObject *model) {
    model->SaveContour();
}

PR_EXTERN
void PrResetContour(PrModelObject *model) {
    model->ResetContour();
}

PR_EXTERN
void PrSavePosture(PrModelObject *model) {
    model->SavePosture();
}

PR_EXTERN
void PrResetPosture(PrModelObject *model) {
    model->ResetPosture();
}

PR_EXTERN
void PrSetContourBlurAlpha(PrModelObject *model, float alpha, float alpha2) {
    model->m_contour_blur_alpha[0] = alpha;
    model->m_contour_blur_alpha[1] = alpha2;
}

PR_EXTERN
void PrSetTransactionBlendRatio(PrModelObject *model, float ratio) {
    model->m_transaction_blend_ratio = ratio;
}

PR_EXTERN
float PrGetContourBlurAlpha(PrModelObject *model) {
    return model->m_contour_blur_alpha[0];
}

PR_EXTERN
float PrGetContourBlurAlpha2(PrModelObject *model) {
    return model->m_contour_blur_alpha[1];
}

PR_EXTERN
float PrGetTransactionBlendRatio(PrModelObject *model) {
    return model->m_transaction_blend_ratio;
}

PR_EXTERN
void PrSetModelDisturbance(PrModelObject *model, float disturbance) {
    model->m_disturbance = disturbance;
}

PR_EXTERN
float PrGetModelDisturbance(PrModelObject *model) {
    return model->m_disturbance;
}

PR_EXTERN
int PrGetVertexNum(PrModelObject *model) {
    return model->m_spm_image->m_vertex_num;
}

PR_EXTERN
char* PrGetModelName(PrModelObject *model) {
    return model->m_spm_image->m_name;
}

PR_EXTERN
char* PrGetAnimationName(SpaFileHeader *animation) {
    return animation->m_name;
}

PR_EXTERN
char* PrGetCameraName(SpcFileHeader *camera) {
    return camera->m_name;
}

PR_EXTERN
char* PrGetSceneName(PrSceneObject *scene) {
    return scene->m_name;
}

PR_EXTERN
PrRENDERING_STATISTICS* PrGetRenderingStatistics() {
    PrRenderStuff *renderStuff = &prRenderStuff;
    return &renderStuff->m_statistics;
}

PR_EXTERN
void PrSetModelVisibility(PrModelObject *model, u_int node_idx, bool visible) {
    if (node_idx >= model->m_spm_image->m_node_num) {
        return;
    }

    SpmNode *node = model->m_spm_image->m_nodes[node_idx];
    if (visible) {
        node->m_flags &= ~0x20000;
    } else {
        node->m_flags |= 0x20000;
    }
}

PR_EXTERN
SpmFileHeader* PrGetModelImage(PrModelObject *model) {
    return model->m_spm_image;
}

PR_EXTERN
SpaFileHeader* PrGetAnimationImage(SpaFileHeader *animation) {
    return animation;
}

PR_EXTERN
SpcFileHeader* PrGetCameraImage(SpcFileHeader *camera) {
    return camera;
}

PR_EXTERN
void PrSetDebugParam(PrDEBUG_PARAM param, int value) {
    debugParam[param].d = value;
}

PR_EXTERN
void PrSetDebugParamFloat(PrDEBUG_PARAM param, float value) {
    debugParam[param].f = value;
}

PR_EXTERN
int PrGetDebugParam(PrDEBUG_PARAM param) {
    return debugParam[param].d;
}

PR_EXTERN
float PrGetDebugParamFloat(PrDEBUG_PARAM param) {
    return debugParam[param].f;
}

static void InitializeDebugParam() {
    PrSetDebugParamFloat(PR_FLOAT_PARAM_DISTURBANCE, 1.0f);
}
