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

#endif /* PRLIB_SPATRACK_H */
