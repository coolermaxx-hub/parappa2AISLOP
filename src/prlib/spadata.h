#ifndef PRLIB_SPADATA_H
#define PRLIB_SPADATA_H

#include "common.h"

#include <eetypes.h>
#include <math.h>

#include <nalib/namatrix.h>

class SpaFileHeader;

#include "spatrack.h"

template <> NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetSprineValue(unsigned int segment, float time) const;

template <typename T> class SpaTypedTransform;

class SpaTransform {
public:
    NaMATRIX<float, 4, 4>* GetMatrix(float time) const;
    bool IsEverIdentical();

    template <typename T> SpaTrack<T>* GetTrack();
    template <typename T> const SpaTrack<T>* GetTrack() const;

    enum Kind {
        Scale = 0, AxisAngle = 1, RotateX = 2, RotateY = 3, RotateZ = 4,
        Translate = 5, Matrix = 6, Shear = 7
    };

    u_char m_kind;
    // The remaining transform header bytes are reserved/unknown in this format.
    PR_PADDING(unk1, 0xF);
};

// The transform kind determines the actual payload type. Keeping the header
// separate avoids pretending every transform owns a matrix-sized payload.
template <typename T>
class SpaTypedTransform : public SpaTransform {
public:
    SpaTrack<T> track;
};

template <typename T>
inline SpaTrack<T>* SpaTransform::GetTrack() {
    return &static_cast<SpaTypedTransform<T>*>(this)->track;
}

template <typename T>
inline const SpaTrack<T>* SpaTransform::GetTrack() const {
    return &static_cast<const SpaTypedTransform<T>*>(this)->track;
}

class SpaNodeAnimation {
public:
    NaMATRIX<float, 4, 4>* GetMatrix(float time) const;
    bool IsVisible(float time) const;
    int Optimize();

    void ChangePointer(SpaFileHeader *animation);

    void BindInlineTables() {
        // Known serialized boundary: scalar track pointers, transform pointers,
        // then extra vector-transform pointers. Bind before Optimize compacts
        // the first transform group; its count can subsequently decrease.
        m_transforms = reinterpret_cast<SpaTransform**>(
            m_shapeWeightTracks + m_shapeWeightTrackCount);
        m_extraVectorTransforms = m_transforms + m_transformCount;
    }

public:
    PR_PADDING(unk0, 0x4);
    SpaTrack<int> *m_visibilityTrack;
    u_int m_transformCount;
    SpaTransform **m_transforms;
    // Relocated by the original code; contents and purpose remain unknown.
    int *unk10;
    u_int m_extraVectorTransformCount;
    SpaTransform **m_extraVectorTransforms;
    PR_PADDING(unk1C, 0x10);
    u_int m_shapeWeightTrackCount;
    SpaTrack<float> *m_shapeWeightTracks[1];
};

#endif /* PRLIB_SPADATA_H */
