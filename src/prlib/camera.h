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
    NaVECTOR<float, 4> unk40;
    NaVECTOR<float, 4> unk50;
    float unk60;
    float unk64;
    float unk68;
    float unk6C;
    float unk70;
    int *unk74;
    PrLinkedList<SpcFileHeader> m_list;
    PrObjectSet<SpcFileHeader> *m_obj_set;
    void *m_user_data;
    SpaTrack<NaVECTOR<float, 4> > *unk88;
    SpaTrack<NaVECTOR<float, 4> > *unk8C;
    SpaTrack<float> *unk90;
    SpaTrack<float> *unk94;

    u_int m_depth_level;

    SpaTrack<float> *m_focal_len_track;
    SpaTrack<float> *m_defocus_len_track;
};

#endif /* PRLIB_CAMERA_H */
