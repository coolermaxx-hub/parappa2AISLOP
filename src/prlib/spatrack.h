#ifndef PRLIB_SPATRACK_H
#define PRLIB_SPATRACK_H

#include <math.h>
#include <stddef.h>


class SpaTrackBase {
public:
    enum Interpolation { Spline = 0, Linear = 1, Step = 2 };
    enum Flags { Loop = 1 };

    unsigned int SearchSegment(float time) const;
    unsigned int GetKeyCount() const { return m_keyCount; }

protected:
    unsigned char m_interpolation;
    unsigned char m_reserved;
    unsigned short m_flags;
    unsigned int m_keyCount;
    mutable unsigned int m_cachedSegment;
    float *m_times;
};

// Serialized variable-length track: a 16-byte header, typed key data, then
// m_keyCount floats containing key times. Spline keys have three T elements
// (value, incoming tangent, outgoing tangent); other keys have one.
template <typename T>
class SpaTrack : public SpaTrackBase {
public:
    T* GetValue(float time) const;
    T* GetSprineValue(unsigned int segment, float time) const;
    T* GetLinearValue(unsigned int segment, float time) const;

    void ChangePointer() {
        const unsigned int elementsPerKey = m_interpolation == Spline ? 3 : 1;
        // This crosses the known file-format boundary between the key array
        // and time array. No key value is read through a different scalar type.
        m_times = reinterpret_cast<float*>(m_values + m_keyCount * elementsPerKey);
    }

    T& KeyValue(unsigned int key) const {
        const unsigned int stride = m_interpolation == Spline ? 3 : 1;
        return m_values[key * stride];
    }
    T& IncomingTangent(unsigned int key) const { return m_values[key * 3 + 1]; }
    T& OutgoingTangent(unsigned int key) const { return m_values[key * 3 + 2]; }

private:
    // One-element tail follows the original variable-length asset convention.
    mutable T m_values[1];
};

// Scalar arithmetic and the fused VU spline have distinct implementations.
template <> int* SpaTrack<int>::GetValue(float time) const;
template <> float* SpaTrack<float>::GetSprineValue(unsigned int segment, float time) const;
template <> float* SpaTrack<float>::GetLinearValue(unsigned int segment, float time) const;

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

inline unsigned int SpaTrackBase::SearchSegment(float arg0) const {
    if (this->m_keyCount == 1) {
        return (unsigned int)-1;
    }

    if (arg0 <= this->m_times[0]) {
        return (unsigned int)-1;
    }

    if (arg0 >= this->m_times[this->m_keyCount - 1]) {
        return this->m_keyCount;
    }

    if (arg0 >= this->m_times[this->m_cachedSegment]) {
        if (arg0 < this->m_times[this->m_cachedSegment + 1]) {
            return this->m_cachedSegment;
        }

        if ((this->m_cachedSegment + 2) < this->m_keyCount) {
            if (arg0 < this->m_times[this->m_cachedSegment + 2]) {
                return ++this->m_cachedSegment;
            }
        }
    } else if (this->m_cachedSegment != 0) {
        if (arg0 >= this->m_times[this->m_cachedSegment - 1]) {
            return --this->m_cachedSegment;
        }
    }

    int right = this->m_keyCount - 2;
    int left = 0;
    int mid = 0;

    while (left <= right) {
        mid = (left + right) / 2;
        if (arg0 < this->m_times[mid]) {
            right = mid - 1;
        } else if (arg0 >= this->m_times[mid + 1]) {
            left = mid + 1;
        } else {
            break;
        }
    }

    this->m_cachedSegment = mid;
    return this->m_cachedSegment;
}

template <>
inline int* SpaTrack<int>::GetValue(float time) const {
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
inline float* SpaTrack<float>::GetSprineValue(unsigned int segment, float time) const {
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
inline float* SpaTrack<float>::GetLinearValue(unsigned int segment, float time) const {
    static float result;
    const float afterStart = time - m_times[segment];
    const float beforeEnd = m_times[segment + 1] - time;
    result = (beforeEnd * KeyValue(segment) + afterStart * KeyValue(segment + 1))
             / (beforeEnd + afterStart);
    return &result;
}


#endif /* PRLIB_SPATRACK_H */
