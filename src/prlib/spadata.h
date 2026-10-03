#ifndef PRLIB_SPADATA_H
#define PRLIB_SPADATA_H

#include "common.h"

#include <eetypes.h>

#include <nalib/namatrix.h>

class SpaFileHeader;

/*
 * An animation track as stored in a .spa file. The header is followed in
 * place by the key values (for spline tracks, three per key: the value and
 * its two tangents) and then by the key times. ChangePointer points
 * m_times at them once the file is loaded.
 */
class SpaTrackBase {
public:
    enum Interpolation {
        SPLINE = 0,
        LINEAR = 1,
        STEP   = 2,
    };

    enum Flags {
        LOOP = 0x1,
    };

    u_int SearchSegment(float time) const;

    u_int GetKeyCount() const {
        return m_key_count;
    }

public:
    void ChangePointer(u_int spline_size, u_int size) {
        int ptr;
        if (m_interpolation == SPLINE) {
            ptr = (int)(this + 1) + (m_key_count * spline_size);
            m_times = (float*)ptr;
        } else {
            ptr = (int)(this + 1) + (m_key_count * size);
            m_times = (float*)ptr;
        }
    }

protected:
    friend class SpaTransform;

    u_char m_interpolation;
    u_char unk1;
    u_short m_flags;
    u_int m_key_count;
    mutable u_int m_last_segment;
    float *m_times;
};

template <typename T>
class SpaTrack : public SpaTrackBase {
public:
    const T* GetValue(float time) const;

    const T* GetSprineValue(u_int seg, float time) const;
    const T* GetLinearValue(u_int seg, float time) const;

    const T& GetKeyValue(u_int i) const {
        return m_values[i];
    }

public:
    void ChangePointer() {
        int ptr;
        if (m_interpolation == SPLINE) {
            ptr = (int)((SpaTrackBase*)this + 1) + (m_key_count * (sizeof(T) * 3));
            m_times = (float*)ptr;
        } else {
            ptr = (int)((SpaTrackBase*)this + 1) + (m_key_count * sizeof(T));
            m_times = (float*)ptr;
        }
    }

private:
    /* m_key_count values, or 3 per key for spline tracks */
    T m_values[1];
};

/* One step of a node's transform: a track whose value type depends on m_type */
class SpaTransform {
public:
    enum Type {
        SCALE        = 0, /* NaVECTOR: x, y, z (and w) scale */
        ROTATE_AXIS  = 1, /* NaVECTOR: axis in xyz, angle in w */
        ROTATE_X     = 2, /* float: angle */
        ROTATE_Y     = 3, /* float: angle */
        ROTATE_Z     = 4, /* float: angle */
        TRANSLATE    = 5, /* NaVECTOR */
        MATRIX       = 6, /* NaMATRIX */
        SHEAR        = 7, /* NaVECTOR: xy, xz, yz */
    };

    const NaMATRIX<float, 4, 4>* GetMatrix(float time) const;
    bool IsEverIdentical();

    u_int GetKeyCount() const {
        return m_track.m_key_count;
    }

    template <typename T>
    SpaTrack<T>* GetTrack() {
        return static_cast<SpaTrack<T>*>(&m_track);
    }

    template <typename T>
    const SpaTrack<T>* GetTrack() const {
        return static_cast<const SpaTrack<T>*>(&m_track);
    }

public:
    u_char m_type;
    PR_PADDING(unk1, 0xF);
    /* A SpaTrack of the type given by m_type; its values follow in place */
    SpaTrackBase m_track;
};
    
class SpaNodeAnimation {
public:
    NaMATRIX<float, 4, 4>* GetMatrix(float) const;
    bool IsVisible(float arg0) const;
    int Optimize();

    void ChangePointer(SpaFileHeader *animation);

public:
    PR_PADDING(unk0, 0x4);
    SpaTrack<int> *m_visibility;
    u_int m_transform_count;
    SpaTransform **m_transforms;
    int *unk10;
    u_int unk14;
    SpaTransform **unk18;
    PR_PADDING(unk1C, 0x10);
    /* Shape node morph target weights, one track per target */
    u_int m_weight_track_num;
    SpaTrack<float> *m_weight_tracks[1];
};

#endif /* PRLIB_SPADATA_H */
