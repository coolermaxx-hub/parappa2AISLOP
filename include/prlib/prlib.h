/*
 * note: This header is only to be used by the C side of the codebase.
 *       Code from prlib itself may use the 'prpriv.h' header.
 */

#ifndef PRLIB_C_H
#define PRLIB_C_H

#include <eetypes.h>
#include <libgraph.h>
#include <libvu0.h>

typedef void* PR_MODELHANDLE;
typedef void* PR_ANIMATIONHANDLE;
typedef void* PR_CAMERAHANDLE;
typedef void* PR_SCENEHANDLE;
typedef float PR_FRAME;

/* model.cpp */
void PrSetPostureWorkArea(u_int area_top, int area_size);

/* prlib.cpp */
void  PrSetFrameRate(float frame_rate);
float PrGetFrameRate(void);

void PrInitializeModule(sceGsZbuf zbuf);
void PrCleanupModule(void);

PR_SCENEHANDLE PrInitializeScene(sceGsDrawEnv1 *draw_env, const char *name, u_int fbp);
void PrInitializeSceneDBuff(sceGsDBuff *dbuff, const char *name, u_int fbp);

void PrCleanupScene(PR_SCENEHANDLE scene);

void PrSetSceneFrame(PR_SCENEHANDLE scene, sceGsFrame frame);
void PrSetSceneEnv(PR_SCENEHANDLE scene, sceGsDrawEnv1 *draw_env);

void PrPreprocessSceneModel(PR_SCENEHANDLE scene);

PR_MODELHANDLE PrInitializeModel(void* spm, PR_SCENEHANDLE scene);
PR_ANIMATIONHANDLE PrInitializeAnimation(PR_ANIMATIONHANDLE spa);
PR_CAMERAHANDLE PrInitializeCamera(PR_CAMERAHANDLE spc);

void PrCleanupModel(PR_MODELHANDLE model);
void PrCleanupAnimation(PR_ANIMATIONHANDLE animation);
void PrCleanupCamera(PR_CAMERAHANDLE camera);
void PrCleanupAllSceneModel(PR_SCENEHANDLE scene);

float PrGetAnimationStartFrame(PR_ANIMATIONHANDLE animation);
float PrGetAnimationEndFrame(PR_ANIMATIONHANDLE animation);

float PrGetCameraStartFrame(PR_CAMERAHANDLE camera);
float PrGetCameraEndFrame(PR_CAMERAHANDLE camera);

void PrSetModelUserData(PR_MODELHANDLE model, void *user_data);
void PrSetAnimationUserData(PR_ANIMATIONHANDLE animation, void *user_data);
void PrSetCameraUserData(PR_CAMERAHANDLE camera, void *user_data);

void* PrGetModelUserData(PR_MODELHANDLE model);
void* PrGetAnimationUserData(PR_ANIMATIONHANDLE animation);
void* PrGetCameraUserData(PR_CAMERAHANDLE camera);

void PrLinkAnimation(PR_MODELHANDLE model, PR_ANIMATIONHANDLE animation);
void PrUnlinkAnimation(PR_MODELHANDLE model);

PR_ANIMATIONHANDLE PrGetLinkedAnimation(PR_MODELHANDLE model);

void PrLinkPositionAnimation(PR_MODELHANDLE model, PR_ANIMATIONHANDLE animation);
void PrUnlinkPositionAnimation(PR_MODELHANDLE model);

PR_ANIMATIONHANDLE PrGetLinkedPositionAnimation(PR_MODELHANDLE model);

void PrSelectCamera(PR_CAMERAHANDLE camera, PR_SCENEHANDLE scene);
PR_CAMERAHANDLE PrGetSelectedCamera(PR_SCENEHANDLE scene);

void* PrGetCurrentCamera(PR_SCENEHANDLE scene);

void PrSetDefaultCamera(PR_SCENEHANDLE scene, PR_CAMERAHANDLE camera);
void PrSetAppropriateDefaultCamera(PR_SCENEHANDLE scene);

void PrShowModel(PR_MODELHANDLE model, sceVu0FMATRIX *position);

float* PrGetModelMatrix(PR_MODELHANDLE model);

void PrHideModel(PR_MODELHANDLE model);

float* PrGetModelPrimitivePosition(PR_MODELHANDLE model);
float* PrGetModelScreenPosition(PR_MODELHANDLE model);

void PrAnimateModel(PR_MODELHANDLE model, float time);
void PrAnimateModelPosition(PR_MODELHANDLE model, float time);
void PrAnimateSceneCamera(PR_SCENEHANDLE scene, float time);

void PrRender(PR_SCENEHANDLE scene);
void PrWaitRender(void);

void PrSetStage(int stage);

void PrSetDepthOfField(PR_MODELHANDLE scene, float focal_lng, float defocus_lng);
void PrSetDepthOfFieldLevel(PR_SCENEHANDLE scene, u_int level);

float PrGetFocalLength(PR_SCENEHANDLE scene);
float PrGetDefocusLength(PR_SCENEHANDLE scene);

u_int PrGetDepthOfFieldLevel(PR_SCENEHANDLE scene);

void PrSaveContour(PR_MODELHANDLE model);
void PrResetContour(PR_MODELHANDLE model);

void PrSavePosture(PR_MODELHANDLE model);
void PrResetPosture(PR_MODELHANDLE model);

void PrSetContourBlurAlpha(PR_MODELHANDLE model, float alpha, float alpha2);

void PrSetTransactionBlendRatio(PR_MODELHANDLE model, float ratio);

float PrGetContourBlurAlpha(PR_MODELHANDLE model);
float PrGetContourBlurAlpha2(PR_MODELHANDLE model);

float PrGetTransactionBlendRatio(PR_MODELHANDLE model);

void  PrSetModelDisturbance(PR_MODELHANDLE model, float disturbance);
float PrGetModelDisturbance(PR_MODELHANDLE model);

u_int PrGetVertexNum(PR_MODELHANDLE model);

char* PrGetModelName(PR_MODELHANDLE model);
char* PrGetAnimationName(PR_ANIMATIONHANDLE animation);
char* PrGetCameraName(PR_CAMERAHANDLE camera);
char* PrGetSceneName(PR_SCENEHANDLE scene);

void* PrGetRenderingStatistics(void);

void PrSetModelVisibility(PR_MODELHANDLE model, u_int node_idx, u_int visible);

PR_MODELHANDLE     PrGetModelImage(PR_MODELHANDLE model);
PR_ANIMATIONHANDLE PrGetAnimationImage(PR_ANIMATIONHANDLE animation);
PR_CAMERAHANDLE    PrGetCameraImage(PR_CAMERAHANDLE camera);

void  PrSetDebugParam(int param, int value);
void  PrSetDebugParamFloat(int param, float value);
int   PrGetDebugParam(int param);
float PrGetDebugParamFloat(int param);

/* menderer.cpp */
void  PrDecelerateMenderer(int speed);
void  PrRestartMenderer(void);

void  PrSetMendererRatio(float ratio);
float PrGetMendererRatio(void);

void  PrSetMendererDirection(int direction);
int   PrGetMendererDirection(void);

void  PrSetMendererColorModulation(int color);
int   PrIsMendererColorModulation(void);

void  PrInitializeMenderer(u_int tbp, void *noodlePicture, u_int fbp);
void  PrRenderMenderer(void);

/* menderercreate.cpp */
void PrCreateMendererTexture(void);

#endif /* PRLIB_C_H */
