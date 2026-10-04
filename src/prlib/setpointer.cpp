#include "common.h"

#include <eetypes.h>
#include <eestruct.h>

#include "camera.h"
#include "microprogram.h"
#include "model.h"
#include "animation.h"
#include "spadata.h"

void SpmFileHeader::ChangePointer() {
    if (m_flags & eSpmFileRelocated) {
        return;
    }

    this->unk64 = CalculatePointer<int>(this->unk64);
    m_nodes = CalculatePointer<SpmNode*>(m_nodes);

    for (int i = 0; i < m_node_num; i++) {
        m_nodes[i] = CalculatePointer<SpmNode>(m_nodes[i]);
    }

    /* Change node pointers starting from the root. */
    (*m_nodes)->ChangePointer(this, NULL);
    m_flags |= eSpmFileRelocated;
}

void SpmNode::ChangePointer(SpmFileHeader *file, SpmNode *parent) {
    this->m_owner = file;
    this->m_parent = parent;

    this->m_firstChild = file->CalculatePointer<SpmNode>(this->m_firstChild);
    this->m_nextSibling = file->CalculatePointer<SpmNode>(this->m_nextSibling);

    this->m_context1Packets[0] = file->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->m_context1Packets[0]);
    this->m_context1Packets[1] = file->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->m_context1Packets[1]);

    this->m_geometryPacket = this->m_context1Packets[0];
    if (this->m_context1Packets[0] == NULL) {
        this->m_geometryPacket = this->m_context1Packets[1];
    }

    for (u_int i = 0; i < 2; i++) {
        if (this->m_context1Packets[i]) {
            this->m_context1Packets[i]->m_microprogramCall = SCE_VIF1_SET_MSCAL(PrGetMicroProgramAddress(this->m_context1Packets[i]->m_microprogram), 0);
        }
    }

    if (m_flags & eSpmDeformPayloadMask) {
        this->m_positionTargets = file->CalculatePointer<SpmPositionTargets>(this->m_positionTargets);
        if (m_flags & eSpmClusterPayload) {
            m_cluster.influences = file->CalculatePointer<SpmClusterInfluences>(m_cluster.influences);
            m_cluster.positions = file->CalculatePointer<NaVECTOR<float, 4> >(m_cluster.positions);
        } else if (m_flags & eSpmShapePayload) {
            m_shape.basePositions = file->CalculatePointer<NaVECTOR<float, 4> >(m_shape.basePositions);
        }

        m_contourIndices = file->CalculatePointer<SpmContourIndex>(m_contourIndices);
        this->m_contourPacket = file->CalculatePointer<PrVuNodeHeaderDmaPacket>(this->m_contourPacket);
        PrVuNodeHeaderDmaPacket *contour = m_contourPacket;
        if (contour != NULL) {
            contour->m_microprogramCall = SCE_VIF1_SET_MSCAL(PrGetMicroProgramAddress(PR_MICRO_PROGRAM_CONTOUR), 0);
        }
    }

    SpmNode *child = m_firstChild;
    while (child != NULL) {
        child->ChangePointer(file, this);
        child = child->m_nextSibling;
    }
}

void SpaFileHeader::ChangePointer() {
    if (m_flags & eSpaFileRelocated) {
        return;
    }

    unk38 = CalculatePointer<int>(unk38);
    m_nodes = m_inlineNodes;

    for (u_int i = 0; i < m_nodeCount; i++) {
        SpaNodeAnimation **p = &m_nodes[i];
        if (*p != NULL) {
            *p = CalculatePointer<SpaNodeAnimation>(*p);
            (*p)->ChangePointer(this);
            (*p)->Optimize();
        }
    }

    m_flags |= eSpaFileRelocated;
}

void SpaNodeAnimation::ChangePointer(SpaFileHeader *animation) {
    this->m_visibilityTrack = animation->CalculatePointer<SpaTrack<int> >(this->m_visibilityTrack);
    this->unk10 = animation->CalculatePointer<int>(this->unk10);
    if (this->m_visibilityTrack != NULL) {
        this->m_visibilityTrack->ChangePointer();
    }

    BindInlineTables();

    for (u_int i = 0; i < this->m_transformCount; i++) {
        SpaTransform *transform;
        this->m_transforms[i] = animation->CalculatePointer<SpaTransform>(this->m_transforms[i]);
        if (this->m_transforms[i] != NULL) {
            switch (this->m_transforms[i]->m_kind) {
            case SpaTransform::Scale:
            case SpaTransform::AxisAngle:
            case SpaTransform::Translate:
            case SpaTransform::Shear:
                transform = this->m_transforms[i];
                transform->GetTrack<NaVECTOR<float, 4> >()->ChangePointer();
                if (transform->GetTrack<NaVECTOR<float, 4> >()->GetKeyCount() == 0) {
                    this->m_transforms[i] = NULL;
                }
                break;
            case SpaTransform::RotateX:
            case SpaTransform::RotateY:
            case SpaTransform::RotateZ:
                transform = this->m_transforms[i];
                transform->GetTrack<float>()->ChangePointer();
                if (transform->GetTrack<float >()->GetKeyCount() == 0) {
                    this->m_transforms[i] = NULL;
                }
                break;
            case SpaTransform::Matrix:
                transform = this->m_transforms[i];
                transform->GetTrack<NaMATRIX<float, 4, 4> >()->ChangePointer();
                if (transform->GetTrack<NaMATRIX<float, 4, 4> >()->GetKeyCount() == 0) {
                    this->m_transforms[i] = NULL;
                }
                break;
            }
        }
    }

    for (u_int i = 0; i < this->m_extraVectorTransformCount; i++) {
        this->m_extraVectorTransforms[i] = animation->CalculatePointer<SpaTransform>(this->m_extraVectorTransforms[i]);
        if (this->m_extraVectorTransforms[i] != NULL) {
            this->m_extraVectorTransforms[i]->GetTrack<NaVECTOR<float, 4> >()->ChangePointer();
        }
    }

    for (u_int i = 0; i < this->m_shapeWeightTrackCount; i++) {
        this->m_shapeWeightTracks[i] = animation->CalculatePointer<SpaTrack<float> >(this->m_shapeWeightTracks[i]);
        if (this->m_shapeWeightTracks[i] != NULL) {
            this->m_shapeWeightTracks[i]->ChangePointer();
        }
    }
}

void SpcFileHeader::ChangePointer() {
    if (m_flags & eSpcFileRelocated) {
        return;
    }

    this->unk74 = CalculatePointer<int>(this->unk74);
    
    this->m_positionTrack = CalculatePointer<SpaTrack<NaVECTOR<float, 4> > >(this->m_positionTrack);
    if (this->m_positionTrack != NULL) {
        this->m_positionTrack->ChangePointer();
    }

    this->m_interestTrack = CalculatePointer<SpaTrack<NaVECTOR<float, 4> > >(this->m_interestTrack);
    if (this->m_interestTrack != NULL) {
        this->m_interestTrack->ChangePointer();
    }

    this->m_rollTrack = CalculatePointer<SpaTrack<float> >(this->m_rollTrack);
    if (this->m_rollTrack != NULL) {
        this->m_rollTrack->ChangePointer();
    }

    this->m_fieldOfViewTrack = CalculatePointer<SpaTrack<float> >(this->m_fieldOfViewTrack);
    if (this->m_fieldOfViewTrack != NULL) {
        this->m_fieldOfViewTrack->ChangePointer();
    }

    if (m_flags & eSpcFileHasFocusTracks) {
        m_focal_len_track = CalculatePointer<SpaTrack<float> >(m_focal_len_track);
        if (m_focal_len_track != NULL) {
            m_focal_len_track->ChangePointer();
        }

        m_defocus_len_track = CalculatePointer<SpaTrack<float> >(m_defocus_len_track);
        if (m_defocus_len_track != NULL) {
            m_defocus_len_track->ChangePointer();
        }
    }

    m_flags |= eSpcFileRelocated;
}
