#include "common.h"

#include <eetypes.h>
#include <eestruct.h>

#include "camera.h"
#include "microprogram.h"
#include "model.h"
#include "animation.h"
#include "spadata.h"

void SpmFileHeader::ChangePointer() {
    if (m_flags & 0x1) {
        return;
    }

    this->unk64 = CalculatePointer<int>(this->unk64);
    m_nodes = CalculatePointer<SpmNode*>(m_nodes);

    for (int i = 0; i < m_node_num; i++) {
        m_nodes[i] = CalculatePointer<SpmNode>(m_nodes[i]);
    }

    /* Change node pointers starting from the root. */
    (*m_nodes)->ChangePointer(this, NULL);
    m_flags |= 0x1;
}

void SpmNode::ChangePointer(SpmFileHeader *model, SpmNode *arg1) {
    this->unk158 = model;
    this->unk164 = arg1;

    this->unk15C = model->CalculatePointer<SpmNode>(this->unk15C);
    this->unk160 = model->CalculatePointer<SpmNode>(this->unk160);

    this->unk16C[0] = model->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->unk16C[0]);
    this->unk16C[1] = model->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->unk16C[1]);

    this->unk17C = this->unk16C[0];
    if (this->unk16C[0] == NULL) {
        this->unk17C = this->unk16C[1];
    }

    for (u_int i = 0; i < 2; i++) {
        if (this->unk16C[i]) {
            this->unk16C[i]->unk194 = SCE_VIF1_SET_MSCAL(PrGetMicroProgramAddress(this->unk16C[i]->unk60), 0);
        }
    }

    if (m_flags & 0xff0) {
        this->unk198 = model->CalculatePointer<int>(this->unk198);
        if (m_flags & 0x10) {
            this->unk1B4 = model->CalculatePointer<int>(this->unk1B4);
            this->unk1B8 = model->CalculatePointer<int>(this->unk1B8);
        } else if (m_flags & 0x20) {
            this->unk1B4 = model->CalculatePointer<int>(this->unk1B4);
        }

        this->unk1A0 = model->CalculatePointer<int>(this->unk1A0);
        this->unk1A4 = model->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->unk1A4);
        PrVuNodeHeaderDmaPacket *s0 = this->unk1A4;
        if (s0 != NULL) {
            s0->unk194 = SCE_VIF1_SET_MSCAL(PrGetMicroProgramAddress(PR_MICRO_PROGRAM_CONTOUR), 0);
        }
    }

    SpmNode *s0 = this->unk15C;
    while (s0 != 0) {
        s0->ChangePointer(model, this);
        s0 = s0->unk160;
    }
}

void SpaFileHeader::ChangePointer() {
    if (m_flags & 0x1) {
        return;
    }

    unk38 = CalculatePointer<int>(unk38);
    unk50 = unk54;

    for (u_int i = 0; i < unk4C; i++) {
        SpaNodeAnimation **p = &unk50[i];
        if (*p != NULL) {
            *p = CalculatePointer<SpaNodeAnimation>(*p);
            (*p)->ChangePointer(this);
            (*p)->Optimize();
        }
    }

    m_flags |= 0x1;
}

void SpaNodeAnimation::ChangePointer(SpaFileHeader *animation) {
    this->unk4 = animation->CalculatePointer<SpaTrack<int> >(this->unk4);
    this->unk10 = animation->CalculatePointer<int>(this->unk10);
    if (this->unk4 != NULL) {
        this->unk4->ChangePointer();
    }

    this->unkC = (SpaTransform**)&this->unk30[this->unk2C];
    this->unk18 = &this->unkC[this->unk8];

    for (u_int i = 0; i < this->unk8; i++) {
        SpaTransform *transform;
        this->unkC[i] = animation->CalculatePointer<SpaTransform>(this->unkC[i]);
        if (this->unkC[i] != NULL) {
            switch (this->unkC[i]->unk0) {
            case 0:
            case 1:
            case 5:
            case 7:
                transform = this->unkC[i];
                transform->GetTrack<NaVECTOR<float, 4> >()->ChangePointer();
                if (transform->unk14 == 0) {
                    this->unkC[i] = NULL;
                }
                break;
            case 2:
            case 3:
            case 4:
                transform = this->unkC[i];
                transform->GetTrack<float>()->ChangePointer();
                if (transform->unk14 == 0) {
                    this->unkC[i] = NULL;
                }
                break;
            case 6:
                transform = this->unkC[i];
                transform->GetTrack<NaMATRIX<float, 4, 4> >()->ChangePointer();
                if (transform->unk14 == 0) {
                    this->unkC[i] = NULL;
                }
                break;
            }
        }
    }

    for (u_int i = 0; i < this->unk14; i++) {
        this->unk18[i] = animation->CalculatePointer<SpaTransform>(this->unk18[i]);
        if (this->unk18[i] != NULL) {
            this->unk18[i]->GetTrack<NaVECTOR<float, 4> >()->ChangePointer();
        }
    }

    for (u_int i = 0; i < this->unk2C; i++) {
        this->unk30[i] = animation->CalculatePointer<SpaTrack<float> >(this->unk30[i]);
        if (this->unk30[i] != NULL) {
            this->unk30[i]->ChangePointer();
        }
    }
}

void SpcFileHeader::ChangePointer() {
    if (m_flags & 0x1) {
        return;
    }

    this->unk74 = CalculatePointer<int>(this->unk74);
    
    this->m_position_track = CalculatePointer<SpaTrack<NaVECTOR<float, 4> > >(this->m_position_track);
    if (this->m_position_track != NULL) {
        this->m_position_track->ChangePointer();
    }

    this->m_interest_track = CalculatePointer<SpaTrack<NaVECTOR<float, 4> > >(this->m_interest_track);
    if (this->m_interest_track != NULL) {
        this->m_interest_track->ChangePointer();
    }

    this->m_roll_track = CalculatePointer<SpaTrack<float> >(this->m_roll_track);
    if (this->m_roll_track != NULL) {
        this->m_roll_track->ChangePointer();
    }

    this->m_field_of_view_track = CalculatePointer<SpaTrack<float> >(this->m_field_of_view_track);
    if (this->m_field_of_view_track != NULL) {
        this->m_field_of_view_track->ChangePointer();
    }

    if (m_flags & 0x8) {
        m_focal_len_track = CalculatePointer<SpaTrack<float> >(m_focal_len_track);
        if (m_focal_len_track != NULL) {
            m_focal_len_track->ChangePointer();
        }

        m_defocus_len_track = CalculatePointer<SpaTrack<float> >(m_defocus_len_track);
        if (m_defocus_len_track != NULL) {
            m_defocus_len_track->ChangePointer();
        }
    }

    m_flags |= 1;
}
