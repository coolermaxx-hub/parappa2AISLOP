#ifndef PRLIB_CAMERA_H
#define PRLIB_CAMERA_H

#include "prpriv.h"
#include "spadata.h"

#include <eetypes.h>

#include <nalib/navector.h>

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
    PR_PADDING(unk38, 0x3c);
    int *unk74;
    PR_PADDING(unk78, 0xC);
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
