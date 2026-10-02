#include "spram.h"

#include "prpriv.h"

#include "model.h"
#include "random.h"

#include "vu1/vucommon.h"
#include "vu1/vumem.h"

#include <eestruct.h>

INCLUDE_ASM("asm/nonmatchings/prlib/spram", Initialize__12PrSPRAM_DATAP13PrSceneObject);

extern NaMATRIX<float, 4, 4> screenClipMatrix;
extern NaMATRIX<float, 4, 4> screenPrimitiveMatrix;

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
    m_display_header.inner.camera_position = this->unk50;
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

INCLUDE_ASM("asm/nonmatchings/prlib/spram", func_00147D90);

/* prlib/spram.cpp */
INCLUDE_ASM("asm/nonmatchings/prlib/spram", _GLOBAL_$I$Initialize__12PrSPRAM_DATAP13PrSceneObject);

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/spram", func_00147E38);

INCLUDE_ASM("asm/nonmatchings/prlib/spram", func_00147FB8);

INCLUDE_ASM("asm/nonmatchings/prlib/spram", func_00148140);

INCLUDE_ASM("asm/nonmatchings/prlib/spram", func_00148248);

INCLUDE_RODATA("asm/nonmatchings/prlib/spram", D_00396790);
