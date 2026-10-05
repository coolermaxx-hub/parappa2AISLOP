#include "spram.h"

#include "dma.h"
#include "prpriv.h"

#include "model.h"
#include "random.h"
#include "renderstuff.h"
#include "scene.h"

#include "vu1/vucommon.h"
#include "vu1/vumem.h"

#include <eestruct.h>

// Copied with lq/sq, which ignore the low four address bits, so these must be
// quadword aligned (the original has them at 16-byte boundaries).
static NaMATRIX<float, 4, 4> screenClipMatrix PR_ALIGNED(16);
static NaMATRIX<float, 4, 4> screenPrimitiveMatrix PR_ALIGNED(16);

extern "C" float tanf(float);

void PrSPRAM_DATA::Initialize(PrSceneObject *scene) {
    static const sceDmaTag endDmaTag = { 0, 0, 0x70, NULL, { 0, 0 } };

    m_noodle_buffer[0] = m_packet_banks.noodle[0];
    m_noodle_buffer[1] = m_packet_banks.noodle[1];
    m_noodle_buffer[2] = m_packet_banks.noodle[2];
    m_end_dmatag = endDmaTag;

    m_camera = *scene->GetCurrentCamera();

    m_camera_direction = NaVECTOR<float, 4>::Normalize3(m_camera.interest - m_camera.position);
    m_camera_direction[3] = 1.0f;

    NaVECTOR<float, 4> yd = -m_camera.up;
    sceVu0CameraMatrix((sceVu0FVECTOR*)&m_camera_matrix, m_camera.position.Data(), m_camera_direction.Data(), yd.Data());

    u_int width = scene->m_width;
    u_int height = scene->m_height;
    float zmax = (float)(0xFFFFFFFF >> (37 - prRenderStuff.GetZbufBits()));
    // Keep depth inside the buffer's range: zmin = 2e-6 * zmax, zmax *= 1 - 2e-6.
    float zmin = zmax * 1.99999999e-06f;
    zmax *= 0.999997973f;
    float aspect = (float)width / (float)height * 3.0f / 4.0f;
    float scrz = 1.0f / tanf(m_camera.field_of_view * 0.5f);

    sceVu0ViewScreenMatrix((sceVu0FVECTOR*)&m_viewScreenMatrix, scrz, height * 0.5f * aspect, height * 0.5f,
        2048.0f, 2048.0f, zmin, zmax, m_camera.near_clip, m_camera.far_clip);
    zmax -= zmin;
    m_view_projection_matrix = m_viewScreenMatrix * m_camera_matrix;

    sceVu0ViewScreenMatrix((sceVu0FVECTOR*)&m_viewClipMatrix, scrz,
        height * 1.00999999f * 0.5f * aspect / 2048.0f, height * 1.00999999f * 0.5f / 2048.0f,
        0.0f, 0.0f, -1.0f, 1.0f, m_camera.near_clip, m_camera.far_clip);
    m_worldClipMatrix = m_viewClipMatrix * m_camera_matrix;

    sceVu0ViewScreenMatrix((sceVu0FVECTOR*)&m_viewGuardMatrix, scrz,
        4096.0f / (width + 2) * 0.99000001f * height * 0.5f * aspect / 2048.0f,
        4096.0f / (height + 2) * 0.99000001f * height * 0.5f / 2048.0f,
        0.0f, 0.0f, -1.0f, 1.0f, m_camera.near_clip, m_camera.far_clip);
    m_worldGuardMatrix = m_viewGuardMatrix * m_camera_matrix;

    screenClipMatrix = NaMATRIX<float, 4, 4>::TranslateMatrix(width * 0.5f, -(float)height * 0.5f, 5010.0f);
    // 1.01 / 2048 on x and y (the clip matrix's 1% margin), 1 / 5010 on z.
    screenClipMatrix.Scale(0.000493164058f, 0.000493164058f, 0.000199600792f);

    screenPrimitiveMatrix = NaMATRIX<float, 4, 4>::ScaleMatrix(1.0f, -0.5f, zmax / 10020.0f);
    screenPrimitiveMatrix.Translate(2048.0f - width * 0.5f, 2048.0f - height * 0.5f, zmin - zmax * -10010.0f / 10020.0f);

    m_unk66C = 0;
    m_unk670 = 0;
    m_unk674 = 0;
    m_unk678 = 0;
}

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
        /* .id   */ PR_DMA_TAG_END,
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
    m_display_header.inner.worldClipMatrix = this->m_worldClipMatrix;
    m_display_header.inner.camera_matrix = m_camera_matrix;
    m_display_header.inner.camera_position = m_camera.position;
    m_display_header.inner.worldGuardMatrix = this->m_worldGuardMatrix;
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
