#ifndef PRLIB_SPRAM_H
#define PRLIB_SPRAM_H

#include "common.h"

#include "vu1/vumem.h"

#include <nalib/navector.h>
#include <nalib/namatrix.h>

#include <libdma.h>

class PrModelObject;
class SpaFileHeader;
class PrSceneObject;

class PrSPRAM_DATA {
public:
    void Initialize(PrSceneObject *scene);
    void InitializeModel(PrModelObject *model);
    void SendDisplayHeader();

public:
    NaMATRIX<float, 4, 4> unk0;
    u_long128 *m_noodle_buffer[3];
    char unk4C[0x4];
    NaVECTOR<float, 4> unk50;
    char unk60[0x40];
    NaMATRIX<float, 4, 4> m_camera_matrix;
    NaMATRIX<float, 4, 4> m_view_projection_matrix;
    NaMATRIX<float, 4, 4> unk120;
    NaMATRIX<float, 4, 4> unk160;
    char unk1A0[0x100];
    sceDmaTag m_end_dmatag;
    char unk2B0[0x100];
    PrDisplayHeader m_display_header;
    float m_animation_time;
    PrModelObject *m_current_model;
    SpaFileHeader *m_animation;
    char unk66C[0x10];
    float m_model_contour_blur_alpha[2];
    float m_model_transaction_blend_ratio;
    u_int m_disturbance_param;
    float m_disturbance;
};

#endif /* PRLIB_SPRAM_H */
