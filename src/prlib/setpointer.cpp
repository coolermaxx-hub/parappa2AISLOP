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
    this->m_file = model;
    this->m_parent = arg1;

    this->m_child = model->CalculatePointer<SpmNode>(this->m_child);
    this->m_sibling = model->CalculatePointer<SpmNode>(this->m_sibling);

    this->m_packets[0] = model->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->m_packets[0]);
    this->m_packets[1] = model->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->m_packets[1]);

    this->m_main_packet = this->m_packets[0];
    if (this->m_packets[0] == NULL) {
        this->m_main_packet = this->m_packets[1];
    }

    for (u_int i = 0; i < 2; i++) {
        if (this->m_packets[i]) {
            this->m_packets[i]->unk194 = SCE_VIF1_SET_MSCAL(PrGetMicroProgramAddress(this->m_packets[i]->unk60), 0);
        }
    }

    if (m_flags & 0xff0) {
        this->m_vertex_index = model->CalculatePointer<u_int>(this->m_vertex_index);
        if (m_flags & SPM_NODE_CLUSTER) {
            SpmClusterGeometryNode *cluster = static_cast<SpmClusterGeometryNode*>(this);
            cluster->m_weights = model->CalculatePointer<SpmClusterWeight>(cluster->m_weights);
            cluster->m_positions = model->CalculatePointer<NaVECTOR<float, 4> >(cluster->m_positions);
        } else if (m_flags & SPM_NODE_SHAPE) {
            SpmShapeNode *shape = static_cast<SpmShapeNode*>(this);
            shape->m_base_vertices = model->CalculatePointer<u_long128>(shape->m_base_vertices);
        }

        this->m_contour_index = model->CalculatePointer<SpmContourIndex>(this->m_contour_index);
        this->m_contour_packet = model->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->m_contour_packet);
        PrVuNodeHeaderDmaPacket *s0 = this->m_contour_packet;
        if (s0 != NULL) {
            s0->unk194 = SCE_VIF1_SET_MSCAL(PrGetMicroProgramAddress(PR_MICRO_PROGRAM_CONTOUR), 0);
        }
    }

    SpmNode *s0 = this->m_child;
    while (s0 != 0) {
        s0->ChangePointer(model, this);
        s0 = s0->m_sibling;
    }
}

void SpaFileHeader::ChangePointer() {
    if (m_flags & 0x1) {
        return;
    }

    unk38 = CalculatePointer<int>(unk38);
    m_nodes = m_node_table;

    for (u_int i = 0; i < m_node_num; i++) {
        SpaNodeAnimation **p = &m_nodes[i];
        if (*p != NULL) {
            *p = CalculatePointer<SpaNodeAnimation>(*p);
            (*p)->ChangePointer(this);
            (*p)->Optimize();
        }
    }

    m_flags |= 0x1;
}

void SpaNodeAnimation::ChangePointer(SpaFileHeader *animation) {
    this->m_visibility = animation->CalculatePointer<SpaTrack<int> >(this->m_visibility);
    this->unk10 = animation->CalculatePointer<int>(this->unk10);
    if (this->m_visibility != NULL) {
        this->m_visibility->ChangePointer();
    }

    /* The pointer tables follow each other: weight tracks, transforms, then unk18 */
    this->m_transforms = (SpaTransform**)&this->m_weight_tracks[this->m_weight_track_num];
    this->unk18 = &this->m_transforms[this->m_transform_count];

    for (u_int i = 0; i < this->m_transform_count; i++) {
        SpaTransform *transform;
        this->m_transforms[i] = animation->CalculatePointer<SpaTransform>(this->m_transforms[i]);
        if (this->m_transforms[i] != NULL) {
            switch (this->m_transforms[i]->m_type) {
            case SpaTransform::SCALE:
            case SpaTransform::ROTATE_AXIS:
            case SpaTransform::TRANSLATE:
            case SpaTransform::SHEAR:
                transform = this->m_transforms[i];
                transform->GetTrack<NaVECTOR<float, 4> >()->ChangePointer();
                if (transform->GetKeyCount() == 0) {
                    this->m_transforms[i] = NULL;
                }
                break;
            case SpaTransform::ROTATE_X:
            case SpaTransform::ROTATE_Y:
            case SpaTransform::ROTATE_Z:
                transform = this->m_transforms[i];
                transform->GetTrack<float>()->ChangePointer();
                if (transform->GetKeyCount() == 0) {
                    this->m_transforms[i] = NULL;
                }
                break;
            case SpaTransform::MATRIX:
                transform = this->m_transforms[i];
                transform->GetTrack<NaMATRIX<float, 4, 4> >()->ChangePointer();
                if (transform->GetKeyCount() == 0) {
                    this->m_transforms[i] = NULL;
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

    for (u_int i = 0; i < this->m_weight_track_num; i++) {
        this->m_weight_tracks[i] = animation->CalculatePointer<SpaTrack<float> >(this->m_weight_tracks[i]);
        if (this->m_weight_tracks[i] != NULL) {
            this->m_weight_tracks[i]->ChangePointer();
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
