#include "prlib/spatrack.h"

template <int Elements, int Keys>
class ScalarTrackFixture : public SpaTrack<float> {
public:
    ScalarTrackFixture(int interpolation) {
        m_interpolation = interpolation;
        m_flags = 0;
        m_keyCount = Keys;
        m_cachedSegment = 0;
        ChangePointer();
    }
    void SetLooping() { m_flags |= Loop; }
    float remainingValues[Elements - 1];
    float times[Keys];
};

// Visibility uses integer step sampling even when the header says Linear.
// This is the original integer GetValue specialization's behavior.
class IntegerTrackFixture : public SpaTrack<int> {
public:
    IntegerTrackFixture() {
        m_interpolation = Linear;
        m_flags = 0;
        m_keyCount = 3;
        m_cachedSegment = 0;
        ChangePointer();
    }
    void SetLooping() { m_flags |= Loop; }
    int remainingValues[2];
    float times[3];
};

extern "C" int spatrack_test() {


    ScalarTrackFixture<3, 3> linear(SpaTrackBase::Linear);
    linear.KeyValue(0) = 2.0f;
    linear.KeyValue(1) = 6.0f;
    linear.KeyValue(2) = 10.0f;
    linear.times[0] = 0.0f;
    linear.times[1] = 2.0f;
    linear.times[2] = 4.0f;
    if (linear.SearchSegment(-1.0f) != (u_int)-1) return 3;
    if (linear.SearchSegment(0.0f) != (u_int)-1) return 4;
    if (linear.SearchSegment(1.0f) != 0 || linear.SearchSegment(3.0f) != 1) return 5;
    if (linear.SearchSegment(1.0f) != 0) return 6;
    if (linear.SearchSegment(4.0f) != 3) return 7;
    if (*linear.GetValue(-1.0f) != 2.0f || *linear.GetValue(5.0f) != 10.0f) return 8;
    if (*linear.GetValue(1.0f) != 4.0f || *linear.GetValue(3.0f) != 8.0f) return 9;

    ScalarTrackFixture<6, 2> spline(SpaTrackBase::Spline);
    spline.KeyValue(0) = 2.0f;
    spline.IncomingTangent(0) = 0.0f;
    spline.OutgoingTangent(0) = 2.0f;
    spline.KeyValue(1) = 6.0f;
    spline.IncomingTangent(1) = 2.0f;
    spline.OutgoingTangent(1) = 0.0f;
    spline.times[0] = 0.0f;
    spline.times[1] = 2.0f;
    if (*spline.GetValue(1.0f) != 4.0f) return 10;
    if (*spline.GetValue(3.0f) != 6.0f) return 11;
    spline.times[1] = 0.0f;
    if (spline.GetSprineValue(0, 0.0f) != &spline.KeyValue(0)) return 12;

    ScalarTrackFixture<3, 3> step(SpaTrackBase::Step);
    step.KeyValue(0) = 3.0f;
    step.KeyValue(1) = 7.0f;
    step.KeyValue(2) = 11.0f;
    step.times[0] = 0.0f;
    step.times[1] = 2.0f;
    step.times[2] = 4.0f;
    if (*step.GetValue(1.0f) != 3.0f || *step.GetValue(3.0f) != 7.0f) return 13;
    linear.SetLooping();
    if (*linear.GetValue(5.0f) != 4.0f || *linear.GetValue(4.0f) != 2.0f) return 14;
    // fmod preserves the sign of negative inputs; the original clamps them.
    if (*linear.GetValue(-1.0f) != 2.0f) return 15;
    ScalarTrackFixture<5, 5> distant(SpaTrackBase::Step);
    for (unsigned int i = 0; i < 5; i++) {
        distant.KeyValue(i) = (float)(i * 3);
        distant.times[i] = (float)i;
    }
    if (distant.SearchSegment(3.5f) != 3 || distant.SearchSegment(0.5f) != 0) return 16;
    if (*distant.GetValue(2.0f) != 6.0f) return 17;
    IntegerTrackFixture visibility;
    visibility.KeyValue(0) = 0;
    visibility.KeyValue(1) = -7;
    visibility.KeyValue(2) = 3;
    visibility.times[0] = 0.0f;
    visibility.times[1] = 2.0f;
    visibility.times[2] = 4.0f;
    if (*visibility.GetValue(-1.0f) != 0 || *visibility.GetValue(1.0f) != 0) return 18;
    if (*visibility.GetValue(2.0f) != -7 || *visibility.GetValue(3.0f) != -7) return 19;
    if (*visibility.GetValue(4.0f) != 3 || *visibility.GetValue(5.0f) != 3) return 20;
    visibility.SetLooping();
    if (*visibility.GetValue(4.0f) != 0 || *visibility.GetValue(6.0f) != -7) return 21;
    if (*visibility.GetValue(-1.0f) != 0) return 22;
    return 0;
}

int main() {
    return spatrack_test();
}
