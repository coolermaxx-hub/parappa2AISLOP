#ifndef PRLIB_SPRAM_H
#define PRLIB_SPRAM_H

#include "common.h"

#include "prpriv.h"
#include "eecore.h"

#include "vu1/vumem.h"

#include <nalib/navector.h>
#include <nalib/namatrix.h>

#include <libdma.h>

class PrModelObject;
class SpaFileHeader;
class PrSceneObject;

struct PrScratchpadPacketBanks {
    u_long128 prefix[(0x1000 - 0x690) / sizeof(u_long128)];
    u_long128 noodle[3][0x1000 / sizeof(u_long128)];
};

class PrSPRAM_DATA {
public:
    void Initialize(PrSceneObject *scene);
    void InitializeModel(PrModelObject *model);
    void SendDisplayHeader();

public:
    NaMATRIX<float, 4, 4> unk0;
    u_long128 *m_noodle_buffer[3];
    char unk4C[0x4];
    PrPERSPECTIVE_CAMERA m_camera;
    NaVECTOR<float, 4> m_camera_direction;
    NaMATRIX<float, 4, 4> m_camera_matrix;
    NaMATRIX<float, 4, 4> m_view_projection_matrix;
    NaMATRIX<float, 4, 4> unk120;
    NaMATRIX<float, 4, 4> unk160;
    NaMATRIX<float, 4, 4> unk1A0;
    NaMATRIX<float, 4, 4> unk1E0;
    NaMATRIX<float, 4, 4> unk220;
    char unk260[0x40];
    sceDmaTag m_end_dmatag;
    PrEECoreContext m_eeCore;
    PrDisplayHeader m_display_header;
    float m_animation_time;
    PrModelObject *m_current_model;
    SpaFileHeader *m_animation;
    u_int m_unk66C;
    u_int m_unk670;
    u_int m_unk674;
    u_int m_unk678;
    float m_model_contour_blur_alpha[2];
    float m_model_transaction_blend_ratio;
    u_int m_disturbance_param;
    float m_disturbance;

    // GIF packets can use the whole remaining scratchpad. Noodle rendering
    // uses three 4 KiB banks within that same storage; these are shared views,
    // not independent buffers. Metadata ends at byte 0x690.
    union {
        u_long128 m_packet_workspace[(0x4000 - 0x690) / sizeof(u_long128)];
        PrScratchpadPacketBanks m_packet_banks;
    };
};

#endif /* PRLIB_SPRAM_H */
