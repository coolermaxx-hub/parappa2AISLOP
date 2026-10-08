#include "spadata.h"

#include "animation.h"
#include "model.h"

#include <math.h>

// Returns the key segment containing time: (unsigned int)-1 before the first key,
// m_keyCount after the last. Tries the cached segment and its neighbours first,
// then falls back to a binary search.
unsigned int SpaTrackBase::SearchSegment(float time) const {
    if (this->m_keyCount == 1) {
        return (unsigned int)-1;
    }

    if (time <= this->m_times[0]) {
        return (unsigned int)-1;
    }

    if (time >= this->m_times[this->m_keyCount - 1]) {
        return this->m_keyCount;
    }

    if (time >= this->m_times[this->m_cachedSegment]) {
        if (time < this->m_times[this->m_cachedSegment + 1]) {
            return this->m_cachedSegment;
        }

        if ((this->m_cachedSegment + 2) < this->m_keyCount) {
            if (time < this->m_times[this->m_cachedSegment + 2]) {
                return ++this->m_cachedSegment;
            }
        }
    } else if (this->m_cachedSegment != 0) {
        if (time >= this->m_times[this->m_cachedSegment - 1]) {
            return --this->m_cachedSegment;
        }
    }

    int right = this->m_keyCount - 2;
    int left = 0;
    int mid = 0;

    while (left <= right) {
        mid = (left + right) / 2;
        if (time < this->m_times[mid]) {
            right = mid - 1;
        } else if (time >= this->m_times[mid + 1]) {
            left = mid + 1;
        } else {
            break;
        }
    }

    this->m_cachedSegment = mid;
    return this->m_cachedSegment;
}

template <typename T>
T* SpaTrack<T>::GetValue(float time) const {
    if (m_flags & Loop) {
        const float duration = m_times[m_keyCount - 1];
        if (time < 0.0f || time >= duration) {
            time = fmodf(time, duration);
        }
    }

    const unsigned int segment = SearchSegment(time);
    if (segment == (unsigned int)-1) {
        return &KeyValue(0);
    }
    if (segment == m_keyCount) {
        return &KeyValue(m_keyCount - 1);
    }
    switch (m_interpolation) {
    case Spline: return GetSprineValue(segment, time);
    case Linear: return GetLinearValue(segment, time);
    case Step: return &KeyValue(segment);
    default: return NULL;
    }
}

template <typename T>
T* SpaTrack<T>::GetLinearValue(unsigned int segment, float time) const {
    static T result;
    const float afterStart = time - m_times[segment];
    const float beforeEnd = m_times[segment + 1] - time;
    result = (KeyValue(segment) * beforeEnd + KeyValue(segment + 1) * afterStart)
             / (beforeEnd + afterStart);
    return &result;
}

template <>
int* SpaTrack<int>::GetValue(float time) const {
    if (m_flags & Loop) {
        const float duration = m_times[m_keyCount - 1];
        if (time < 0.0f || time >= duration) {
            time = fmodf(time, duration);
        }
    }
    const unsigned int segment = SearchSegment(time);
    if (segment == (unsigned int)-1) return &m_values[0];
    if (segment == m_keyCount) return &m_values[m_keyCount - 1];
    return &m_values[segment];
}

template <>
float* SpaTrack<float>::GetSprineValue(unsigned int segment, float time) const {
    static float result;
    const float duration = m_times[segment + 1] - m_times[segment];
    float& start = KeyValue(segment);
    if (duration == 0.0f) return &start;
    const float end = KeyValue(segment + 1);
    const float outgoing = OutgoingTangent(segment);
    const float incoming = IncomingTangent(segment + 1);
    const float t = (time - m_times[segment]) / duration;
    const float difference = start - end;
    result = t * (t * (t * ((outgoing + incoming) * duration + (difference + difference))
             - (outgoing + outgoing + incoming) * duration - difference * 3.0f)
             + outgoing * duration) + start;
    return &result;
}

template <>
float* SpaTrack<float>::GetLinearValue(unsigned int segment, float time) const {
    static float result;
    const float afterStart = time - m_times[segment];
    const float beforeEnd = m_times[segment + 1] - time;
    // A Linear track stores one element per key, so the key index needs no
    // interpolation stride. Only a Spline track interleaves tangents, and
    // GetValue never routes one here.
    result = (beforeEnd * m_values[segment] + afterStart * m_values[segment + 1])
             / (beforeEnd + afterStart);
    return &result;
}

template <>
NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetSprineValue(u_int segment, float time) const {
    float duration = this->m_times[segment + 1] - this->m_times[segment];
    NaVECTOR<float, 4> *start = &KeyValue(segment);
    if (duration == 0.0f) {
        return start;
    }

    float t = (time - this->m_times[segment]) / duration;

    static NaVECTOR<float, 4> result;

    // Preserve the original fused VU evaluation. start points to a spline key:
    // value, incoming tangent, outgoing tangent, then the next key.
    asm volatile(
        "lqc2     $vf04,  0x00(%1)       \n\t"
        "lqc2     $vf05,  0x30(%1)       \n\t"
        "vsub     $vf01,  $vf04,  $vf05  \n\t"
        
        "lqc2     $vf06,  0x20(%1)       \n\t"
        "lqc2     $vf07,  0x40(%1)       \n\t"
        "vadd     $vf02,  $vf06,  $vf07  \n\t"
        
        "qmtc2    %2,     $vf08          \n\t"
        "vadda    ACC,    $vf01,  $vf01  \n\t"
        "vmaddx   $vf03,  $vf02,  $vf08  \n\t"
        
        "qmtc2    %3,     $vf09          \n\t"
        "vadda    ACC,    $vf06,  $vf06  \n\t"
        "vmaddw   $vf02,  $vf07,  $vf00  \n\t"
        "vadda.w  ACC,    $vf00,  $vf00  \n\t"
        "vmadd.w  $vf08,  $vf00,  $vf00  \n\t"
        "vmulax   ACC,    $vf03,  $vf09  \n\t"
        "vmsubax  ACC,    $vf02,  $vf08  \n\t"
        "vmul.x   $vf10,  $vf09,  $vf08  \n\t"
        "vmul.x   $vf02,  $vf09,  $vf09  \n\t"
        "vmsubw   $vf03,  $vf01,  $vf08  \n\t"
        "vaddax   ACC,    $vf04,  $vf00  \n\t"
        "vmaddax  ACC,    $vf06,  $vf10  \n\t"
        "vmaddx   $vf01,  $vf03,  $vf02  \n\t"
        "sqc2     $vf01,  0(%0)          \n\t"
    : : "r"(&result), "r"(start), "r"(duration), "r"(t) : "memory");

    return &result;
}

NaMATRIX<float, 4, 4>* SpaTransform::GetMatrix(float time) const {
    static NaVECTOR<float, 4> vector;
    static NaMATRIX<float, 4, 4> matrix;

    switch (m_kind) {
    case Scale:
        vector = *GetTrack<NaVECTOR<float, 4> >()->GetValue(time);
        vector[3] = 1.0f;
        matrix = NaMATRIX<float, 4, 4>::ScaleMatrix(vector);
        return &matrix;
    case AxisAngle: {
        vector = *GetTrack<NaVECTOR<float, 4> >()->GetValue(time);
        float angle = vector[3];
        vector[3] = 1.0f;
        matrix = NaMATRIX<float, 4, 4>::RotateMatrix(vector, angle);
        return &matrix;
    }
    case RotateX:
    case RotateY:
    case RotateZ: {
        float angle = *GetTrack<float>()->GetValue(time);
        matrix = NaMATRIX<float, 4, 4>::RotateMatrix(m_kind - RotateX, angle);
        return &matrix;
    }
    case Translate:
        vector = *GetTrack<NaVECTOR<float, 4> >()->GetValue(time);
        matrix = NaMATRIX<float, 4, 4>::TranslateMatrix(vector);
        return &matrix;
    case Matrix:
        return GetTrack<NaMATRIX<float, 4, 4> >()->GetValue(time);
    case Shear: {
        vector = *GetTrack<NaVECTOR<float, 4> >()->GetValue(time);
        NaMATRIX<float, 4, 4> *ret = &matrix;
        ret->Set(
            1.0f, 0.0f, 0.0f, 0.0f,
            vector[0], 1.0f, 0.0f, 0.0f,
            vector[1], vector[2], 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
        return ret;
    }
    default:
        return &NaMATRIX<float, 4, 4>::IDENT;
    }
}

NaMATRIX<float, 4, 4>* SpaNodeAnimation::GetMatrix(float time) const {
    static NaMATRIX<float, 4, 4> matrix;

    asm volatile(
        "vmove.xyzw $vf16, $vf0  \n\t"
        "vmr32.xyzw $vf15, $vf0  \n\t"
        "vmr32.xyzw $vf14, $vf15 \n\t"
        "vmr32.xyzw $vf13, $vf14 \n\t"
    );

    for (u_int i = 0; i < this->m_transformCount; i++) {
        NaMATRIX<float, 4, 4> *m = this->m_transforms[i]->GetMatrix(time);
        asm volatile(
            "lqc2         $vf4, 0x0(%0)       \n\t"
            "lqc2         $vf5, 0x10(%0)      \n\t"
            "lqc2         $vf6, 0x20(%0)      \n\t"
            "lqc2         $vf7, 0x30(%0)      \n\t"
            "vmulax.xyzw  ACC, $vf4, $vf13x   \n\t"
            "vmadday.xyzw ACC, $vf5, $vf13y   \n\t"
            "vmaddaz.xyzw ACC, $vf6, $vf13z   \n\t"
            "vmaddw.xyzw  $vf13, $vf7, $vf13w \n\t"
            "vmulax.xyzw  ACC, $vf4, $vf14x   \n\t"
            "vmadday.xyzw ACC, $vf5, $vf14y   \n\t"
            "vmaddaz.xyzw ACC, $vf6, $vf14z   \n\t"
            "vmaddw.xyzw  $vf14, $vf7, $vf14w \n\t"
            "vmulax.xyzw  ACC, $vf4, $vf15x   \n\t"
            "vmadday.xyzw ACC, $vf5, $vf15y   \n\t"
            "vmaddaz.xyzw ACC, $vf6, $vf15z   \n\t"
            "vmaddw.xyzw  $vf15, $vf7, $vf15w \n\t"
            "vmulax.xyzw  ACC, $vf4, $vf16x   \n\t"
            "vmadday.xyzw ACC, $vf5, $vf16y   \n\t"
            "vmaddaz.xyzw ACC, $vf6, $vf16z   \n\t"
            "vmaddw.xyzw  $vf16, $vf7, $vf16w \n\t"
        : : "r"(m) : "memory");
    }

    asm volatile(
        "sqc2 $vf13, 0x0(%0)  \n\t"
        "sqc2 $vf14, 0x10(%0) \n\t"
        "sqc2 $vf15, 0x20(%0) \n\t"
        "sqc2 $vf16, 0x30(%0) \n\t"
    : : "r"(&matrix) : "memory");

    return &matrix;
}

bool SpaNodeAnimation::IsVisible(float time) const {
    if (this->m_visibilityTrack == NULL) {
        return true;
    }

    return *this->m_visibilityTrack->GetValue(time);
}

bool SpaFileHeader::IsNodeVisible(SpmNode *node, float time) const {
    SpaNodeAnimation *visibilityAnimation = this->m_nodes[node->m_animationIndex];
    SpmNode *parent = node->m_parent;

    if (visibilityAnimation != NULL && visibilityAnimation->m_visibilityTrack == NULL) {
        visibilityAnimation = NULL;
    }

    if (parent != NULL) {
        if (!(parent->m_flags & eSpmVisible)) {
            return false;
        }
        if (parent->m_flags & eSpmAnimatedVisibility) {
            if (visibilityAnimation == NULL) {
                node->m_flags |= eSpmAnimatedVisibility;
                if (parent->m_flags & eSpmVisible) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    }

    if (visibilityAnimation != NULL) {
        node->m_flags |= eSpmAnimatedVisibility;
        return visibilityAnimation->IsVisible(time);
    }

    node->m_flags &= ~eSpmAnimatedVisibility;
    return (node->m_flags & eSpmDefaultHidden) ? false : true;
}

bool SpaTransform::IsEverIdentical() {
    switch (m_kind) {
    case Scale:
    case Translate:
    case Shear: {
        const SpaTrack<NaVECTOR<float, 4> >* track = GetTrack<NaVECTOR<float, 4> >();
        if (track->GetKeyCount() != 1) return false;
        const NaVECTOR<float, 4>& value = track->KeyValue(0);
        const float expected = m_kind == Scale ? 1.0f : 0.0f;
        return value[0] == expected && value[1] == expected && value[2] == expected;
    }
    case AxisAngle: {
        const SpaTrack<NaVECTOR<float, 4> >* track = GetTrack<NaVECTOR<float, 4> >();
        return track->GetKeyCount() == 1 && track->KeyValue(0)[3] == 0.0f;
    }
    case RotateX:
    case RotateY:
    case RotateZ: {
        const SpaTrack<float>* track = GetTrack<float>();
        return track->GetKeyCount() == 1 && track->KeyValue(0) == 0.0f;
    }
    case Matrix: {
        const SpaTrack<NaMATRIX<float, 4, 4> >* track = GetTrack<NaMATRIX<float, 4, 4> >();
        return track->GetKeyCount() == 1 && track->KeyValue(0).IsIdentity();
    }
    default:
        // Preserve the original fallback, including its unusual predicate.
        return NaMATRIX<float, 4, 4>::IDENT.IsNonZero();
    }
}

int SpaNodeAnimation::Optimize() {
    int removedCount = 0;

    for (int i = 0; i < this->m_transformCount; i++) {
        SpaTransform *transform = this->m_transforms[i];
        if (transform == NULL || transform->IsEverIdentical()) {
            removedCount++;
            continue;
        }
        this->m_transforms[i - removedCount] = this->m_transforms[i];
    }

    this->m_transformCount -= removedCount;
    return removedCount;
}

template <typename T>
T* SpaTrack<T>::GetSprineValue(unsigned int segment, float time) const {
    static T result;
    const float duration = m_times[segment + 1] - m_times[segment];
    T& start = KeyValue(segment);
    if (duration == 0.0f) return &start;
    const T& end = KeyValue(segment + 1);
    const T& outgoing = OutgoingTangent(segment);
    const T& incoming = IncomingTangent(segment + 1);
    const float t = (time - m_times[segment]) / duration;
    T difference = start - end;
    result = ((((outgoing + incoming) * duration + difference * 2.0f) * t
              - (outgoing * 2.0f + incoming) * duration - difference * 3.0f) * t
              + outgoing * duration) * t + start;
    return &result;
}
