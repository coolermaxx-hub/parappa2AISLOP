#ifndef PRLIB_CAMERA_H
#define PRLIB_CAMERA_H

#include "prpriv.h"
#include "spadata.h"
#include "linkedlist.h"
#include "objectset.h"

#include <eetypes.h>

#include <nalib/navector.h>

#define SPC_MAGIC   (0x09463AD8)
#define SPC_VERSION (1)

class SpcFileHeader {
public:
    void Initialize();
    PrPERSPECTIVE_CAMERA* GetCamera(float time) const;

    void ChangePointer();

    template <typename T>
    T* CalculatePointer(T *offset) {
        if (!offset) {
            return NULL;
        }
        return reinterpret_cast<T*>(reinterpret_cast<int>(this) + reinterpret_cast<int>(offset));
    }

public:
    u_int m_magic;
    u_short m_version;
    u_short m_flags;

    PR_PADDING(unk8, 0xC);
    float unk14;
    char m_name[32];
    PR_PADDING(unk38, 0x8);
    NaVECTOR<float, 4> m_position;
    NaVECTOR<float, 4> m_interest;
    float m_roll;
    float m_fieldOfView;
    float m_aspect;
    float m_nearClip;
    float m_farClip;
    int *unk74;
    PrLinkedList<SpcFileHeader> m_list;
    PrObjectSet<SpcFileHeader> *m_obj_set;
    void *m_user_data;
    SpaTrack<NaVECTOR<float, 4> > *m_positionTrack;
    SpaTrack<NaVECTOR<float, 4> > *m_interestTrack;
    SpaTrack<float> *m_rollTrack;
    SpaTrack<float> *m_fieldOfViewTrack;

    u_int m_depth_level;

    SpaTrack<float> *m_focal_len_track;
    SpaTrack<float> *m_defocus_len_track;
};

#endif /* PRLIB_CAMERA_H */
