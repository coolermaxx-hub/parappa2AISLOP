#include "spram.h"

#include "prpriv.h"

#include "model.h"
#include "random.h"
#include "renderstuff.h"
#include "scene.h"

#include "vu1/vucommon.h"
#include "vu1/vumem.h"

#include <eestruct.h>

extern NaMATRIX<float, 4, 4> screenClipMatrix;
extern NaMATRIX<float, 4, 4> screenPrimitiveMatrix;

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/spram", Initialize__12PrSPRAM_DATAP13PrSceneObject);
#else /* Stack slot and register allocation differ; also drop INCLUDE_RODATA D_00396790 once it matches */
extern "C" float tanf(float);

/* Out-of-line NaMATRIX helpers emitted at the end of this file */
NaMATRIX<float, 4, 4> TransMatrix_tmp_spram(const float& x, const float& y, const float& z) asm("TranslateMatrix__t8NaMATRIX3Zfi4i4RCfT1T1");
NaMATRIX<float, 4, 4> ScaleMatrix_tmp_spram(const float& x, const float& y, const float& z) asm("ScaleMatrix__t8NaMATRIX3Zfi4i4RCfT1T1");
NaMATRIX<float, 4, 4>& Scale_tmp_spram(NaMATRIX<float, 4, 4> *m, const float& x, const float& y, const float& z) asm("Scale__t8NaMATRIX3Zfi4i4RCfT1T1");
NaMATRIX<float, 4, 4>& Trans_tmp_spram(NaMATRIX<float, 4, 4> *m, const float& x, const float& y, const float& z) asm("Translate__t8NaMATRIX3Zfi4i4RCfT1T1");

void sceVu0CameraMatrix_tmp_spram(sceVu0FMATRIX m, sceVu0FVECTOR p, sceVu0FVECTOR zd, sceVu0FVECTOR yd) asm("func_00161CB0");
void sceVu0ViewScreenMatrix_tmp_spram(sceVu0FMATRIX m, float scrz, float ax, float ay,
    float cx, float cy, float zmin, float zmax, float nearz, float farz) asm("func_00161E88");

static inline NaVECTOR<float, 4> Normalize_tmp_spram(const NaVECTOR<float, 4>& v) {
    NaVECTOR<float, 4> ret;
    asm volatile("
        lqc2       $vf4, 0x0(%1)
        vmul.xyz   $vf5, $vf4, $vf4
        vaddy.x    $vf5, $vf5, $vf5y
        vaddz.x    $vf5, $vf5, $vf5z
        vsqrt      Q, $vf5x
        vwaitq
        vaddq.x    $vf5, $vf0, Q
        vdiv       Q, $vf0w, $vf5x
        vsub.xyzw  $vf6, $vf0, $vf0
        vwaitq
        vmulq.xyz  $vf6, $vf4, Q
        sqc2       $vf6, 0x0(%0)
    " : : "r"(&ret), "r"(&v));
    ((float*)&ret)[3] = 1.0f;
    return ret;
}

static inline NaVECTOR<float, 4> Negate_tmp_spram(const NaVECTOR<float, 4>& v) {
    NaVECTOR<float, 4> ret;
    for (int i = 0; i < 4; i++) {
        ((float*)&ret)[i] = 0.0f - v[i];
    }
    return ret;
}

void PrSPRAM_DATA::Initialize(PrSceneObject *scene) {
    static const sceDmaTag endDmaTag = { 0, 0, 0x70, NULL, { 0, 0 } };

    m_noodle_buffer[0] = (u_long128*)((u_int)this + 0x1000);
    m_noodle_buffer[1] = (u_long128*)((u_int)this + 0x2000);
    m_noodle_buffer[2] = (u_long128*)((u_int)this + 0x3000);
    m_end_dmatag = endDmaTag;

    m_camera = *scene->GetCurrentCamera();

    m_camera_direction = Normalize_tmp_spram(m_camera.interest - m_camera.position);

    NaVECTOR<float, 4> yd = Negate_tmp_spram(m_camera.up);
    sceVu0CameraMatrix_tmp_spram((sceVu0FVECTOR*)&m_camera_matrix, (float*)&m_camera.position, (float*)&m_camera_direction, (float*)&yd);

    u_int width = scene->unk74;
    u_int height = scene->unk78;
    float zmax = (float)(0xFFFFFFFF >> (37 - prRenderStuff.GetZbufBits()));
    float zmin = zmax * 1.99999999e-06f;
    zmax *= 0.999997973f;
    float aspect = (float)width / (float)height * 3.0f / 4.0f;
    float scrz = 1.0f / tanf(m_camera.field_of_view * 0.5f);

    sceVu0ViewScreenMatrix_tmp_spram((sceVu0FVECTOR*)&unk1A0, scrz, height * 0.5f * aspect, height * 0.5f,
        2048.0f, 2048.0f, zmin, zmax, m_camera.near_clip, m_camera.far_clip);
    zmax -= zmin;
    m_view_projection_matrix = unk1A0 * m_camera_matrix;

    sceVu0ViewScreenMatrix_tmp_spram((sceVu0FVECTOR*)&unk1E0, scrz,
        height * 1.00999999f * 0.5f * aspect / 2048.0f, height * 1.00999999f * 0.5f / 2048.0f,
        0.0f, 0.0f, -1.0f, 1.0f, m_camera.near_clip, m_camera.far_clip);
    unk120 = unk1E0 * m_camera_matrix;

    sceVu0ViewScreenMatrix_tmp_spram((sceVu0FVECTOR*)&unk220, scrz,
        4096.0f / (width + 2) * 0.99000001f * height * 0.5f * aspect / 2048.0f,
        4096.0f / (height + 2) * 0.99000001f * height * 0.5f / 2048.0f,
        0.0f, 0.0f, -1.0f, 1.0f, m_camera.near_clip, m_camera.far_clip);
    unk160 = unk220 * m_camera_matrix;

    screenClipMatrix = TransMatrix_tmp_spram(width * 0.5f, -(float)height * 0.5f, 5010.0f);
    Scale_tmp_spram(&screenClipMatrix, 0.000493164058f, 0.000493164058f, 0.000199600792f);

    screenPrimitiveMatrix = ScaleMatrix_tmp_spram(1.0f, -0.5f, zmax / 10020.0f);
    Trans_tmp_spram(&screenPrimitiveMatrix, 2048.0f - width * 0.5f, 2048.0f - height * 0.5f, zmin - zmax * -10010.0f / 10020.0f);

    m_unk66C = 0;
    m_unk670 = 0;
    m_unk674 = 0;
    m_unk678 = 0;
}
#endif

void PrSPRAM_DATA::InitializeModel(PrModelObject *model) {
    m_model_contour_blur_alpha[0] = model->m_contour_blur_alpha[0];
    m_model_contour_blur_alpha[1] = model->m_contour_blur_alpha[1];

    m_model_transaction_blend_ratio = model->m_transaction_blend_ratio;

    float disturbance = model->m_disturbance;
    float debug_disturbance = PrGetDebugParamFloat(PR_FLOAT_PARAM_DISTURBANCE);
    m_disturbance = disturbance * debug_disturbance;
}

void PrSPRAM_DATA::SendDisplayHeader() {
    static sceDmaTag dmaTagTemplate = {
        /* .qwc  */ (sizeof(PrDisplayHeader) / 16) - 1,
        /* .mark */ 0,
        /* .id   */ 0x70, /* DMAend */
        /* .next */ NULL,
        /* .p    */ {
            SCE_VIF1_SET_STCYCL(/*WL*/4, /*CL*/4, 0),
            SCE_VIF1_SET_STMOD(0, 0)
        }
    };

    m_display_header.tag = dmaTagTemplate;

    m_display_header.stmask[0] = SCE_VIF1_SET_STMASK(0);
    m_display_header.stmask[1] = 0;

    m_display_header.base      = SCE_VIF1_SET_BASE(PR_VU1_CHUNK1_START, 0);
    m_display_header.offset    = SCE_VIF1_SET_OFFSET(PR_VU1_CHUNK2_START, 0);
    m_display_header.mark      = SCE_VIF1_SET_MARK(0, 0);
    m_display_header.mskpath3  = SCE_VIF1_SET_MSKPATH3(0, 0);

    m_display_header.unpack[0] = SCE_VIF1_SET_NOP(0);
    m_display_header.unpack[1] = SCE_VIF1_SET_UNPACK(PR_VU1_DISPLAYHDR_ADDR, sizeof(PrInnerDisplayHeader) / 16, PR_VIF_UNPACK_V4_32(0), 0);

    m_display_header.inner.view_projection_matrix = m_view_projection_matrix;
    m_display_header.inner.unk70 = this->unk120;
    m_display_header.inner.camera_matrix = m_camera_matrix;
    m_display_header.inner.camera_position = m_camera.position;
    m_display_header.inner.unk100 = this->unk160;
    m_display_header.inner.screen_clip_matrix = screenClipMatrix;
    m_display_header.inner.screen_primitive_matrix = screenPrimitiveMatrix;

    u_int disturbance_param = PrRandom();
    m_disturbance_param = disturbance_param;
    m_display_header.inner.disturbance_param = disturbance_param;

    void       *tag  = PR_DMA_SPR_ADDR(&m_display_header);
    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_VIF1);
    chan->chcr.TTE = 1;
    sceDmaSend(chan, tag);
}

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/spram", Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1);

INCLUDE_ASM("asm/nonmatchings/prlib/spram", Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1);

/* prlib/spram.cpp */
INCLUDE_ASM("asm/nonmatchings/prlib/spram", _GLOBAL_$I$Initialize__12PrSPRAM_DATAP13PrSceneObject);

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/spram", Translate__t8NaMATRIX3Zfi4i4RCfT1T1);

INCLUDE_ASM("asm/nonmatchings/prlib/spram", Scale__t8NaMATRIX3Zfi4i4RCfT1T1);

INCLUDE_ASM("asm/nonmatchings/prlib/spram", ScaleMatrix__t8NaMATRIX3Zfi4i4RCfT1T1);

INCLUDE_ASM("asm/nonmatchings/prlib/spram", TranslateMatrix__t8NaMATRIX3Zfi4i4RCfT1T1);

INCLUDE_RODATA("asm/nonmatchings/prlib/spram", D_00396790);
