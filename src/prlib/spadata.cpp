#include "spadata.h"

#include "animation.h"
#include "model.h"

#include <math.h>

/* placement new, used to emulate static local construction */
inline void* operator new(size_t, void *p) {
    return p;
}

/* The vector spline is hand-written VU code; see the end of the file */
template <>
const NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetSprineValue(u_int seg, float time) const;

#ifndef NON_MATCHING
/*
 * The original emits these template instances at the end of this file,
 * interleaved with copies that don't match yet. C can't place a template
 * instance after top-level asm, so the matching build keeps all of them as
 * asm at the end of the file. Built with NON_MATCHING, the generic
 * definitions below produce them.
 */
extern template const NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetValue(float time) const;
extern template const NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetLinearValue(u_int seg, float time) const;
extern template const float* SpaTrack<float>::GetValue(float time) const;
extern template const float* SpaTrack<float>::GetSprineValue(u_int seg, float time) const;
extern template const float* SpaTrack<float>::GetLinearValue(u_int seg, float time) const;
extern template const NaMATRIX<float, 4, 4>* SpaTrack<NaMATRIX<float, 4, 4> >::GetValue(float time) const;
extern template const NaMATRIX<float, 4, 4>* SpaTrack<NaMATRIX<float, 4, 4> >::GetSprineValue(u_int seg, float time) const;
extern template const NaMATRIX<float, 4, 4>* SpaTrack<NaMATRIX<float, 4, 4> >::GetLinearValue(u_int seg, float time) const;
extern template NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::RotateMatrix(const NaVECTOR<float, 4>& axis, const float& angle);
extern template NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::RotateMatrix(int axis, const float& angle);
extern template NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::TranslateMatrix(const NaVECTOR<float, 4>& v);
extern template NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::ScaleMatrix(const NaVECTOR<float, 4>& v);
extern template NaMATRIX<float, 4, 4>& NaMATRIX<float, 4, 4>::Set(
    const float& m00, const float& m01, const float& m02, const float& m03,
    const float& m10, const float& m11, const float& m12, const float& m13,
    const float& m20, const float& m21, const float& m22, const float& m23,
    const float& m30, const float& m31, const float& m32, const float& m33);
#endif

u_int SpaTrackBase::SearchSegment(float time) const {
    if (m_key_count == 1) {
        return (u_int)-1;
    }

    if (time <= m_times[0]) {
        return (u_int)-1;
    }

    if (time >= m_times[m_key_count - 1]) {
        return m_key_count;
    }

    /* Try the last segment and its neighbours before searching */
    if (time >= m_times[m_last_segment]) {
        if (time < m_times[m_last_segment + 1]) {
            return m_last_segment;
        }

        if ((m_last_segment + 2) < m_key_count) {
            if (time < m_times[m_last_segment + 2]) {
                return ++m_last_segment;
            }
        }
    } else if (m_last_segment != 0) {
        if (time >= m_times[m_last_segment - 1]) {
            return --m_last_segment;
        }
    }

    int right = m_key_count - 2;
    int left = 0;
    int mid = 0;

    while (left <= right) {
        mid = (left + right) / 2;
        if (time < m_times[mid]) {
            right = mid - 1;
        } else if (time >= m_times[mid + 1]) {
            left = mid + 1;
        } else {
            break;
        }
    }

    m_last_segment = mid;
    return m_last_segment;
}

/* Visibility tracks only step between keys */
template <>
const int* SpaTrack<int>::GetValue(float time) const {
    if (m_flags & LOOP) {
        float length = m_times[m_key_count - 1];
        if (time < 0.0f || time >= length) {
            time = fmodf(time, length);
        }
    }

    u_int seg = SearchSegment(time);
    if (seg == (u_int)-1) {
        return &m_values[0];
    }

    if (seg == m_key_count) {
        return &m_values[seg - 1];
    } else {
        return &m_values[seg];
    }
}

const NaMATRIX<float, 4, 4>* SpaTransform::GetMatrix(float time) const {
    /* FIXME: static locals; see the note in GetSprineValue */
    extern NaVECTOR<float, 4> vector_tmp_spadata_transform;
    extern int tmp_0_transform_vector;
    if (tmp_0_transform_vector == 0) {
        tmp_0_transform_vector = 1;
    }

    extern NaMATRIX<float, 4, 4> matrix_tmp_spadata_transform;
    extern int tmp_0_transform_matrix;
    if (tmp_0_transform_matrix == 0) {
        new (&matrix_tmp_spadata_transform) NaMATRIX<float, 4, 4>;
        tmp_0_transform_matrix = 1;
    }

    switch (m_type) {
    case SCALE:
        vector_tmp_spadata_transform = *GetTrack<NaVECTOR<float, 4> >()->GetValue(time);
        vector_tmp_spadata_transform[3] = 1.0f;
        matrix_tmp_spadata_transform = NaMATRIX<float, 4, 4>::ScaleMatrix(vector_tmp_spadata_transform);
        return &matrix_tmp_spadata_transform;
    case ROTATE_AXIS: {
        vector_tmp_spadata_transform = *GetTrack<NaVECTOR<float, 4> >()->GetValue(time);
        float angle = vector_tmp_spadata_transform[3];
        vector_tmp_spadata_transform[3] = 1.0f;
        matrix_tmp_spadata_transform = NaMATRIX<float, 4, 4>::RotateMatrix(vector_tmp_spadata_transform, angle);
        return &matrix_tmp_spadata_transform;
    }
    case ROTATE_X:
    case ROTATE_Y:
    case ROTATE_Z: {
        float angle = *GetTrack<float>()->GetValue(time);
        matrix_tmp_spadata_transform = NaMATRIX<float, 4, 4>::RotateMatrix(m_type - ROTATE_X, angle);
        return &matrix_tmp_spadata_transform;
    }
    case TRANSLATE:
        vector_tmp_spadata_transform = *GetTrack<NaVECTOR<float, 4> >()->GetValue(time);
        matrix_tmp_spadata_transform = NaMATRIX<float, 4, 4>::TranslateMatrix(vector_tmp_spadata_transform);
        return &matrix_tmp_spadata_transform;
    case MATRIX:
        return GetTrack<NaMATRIX<float, 4, 4> >()->GetValue(time);
    case SHEAR:
        vector_tmp_spadata_transform = *GetTrack<NaVECTOR<float, 4> >()->GetValue(time);
        matrix_tmp_spadata_transform.Set(1.0f, 0.0f, 0.0f, 0.0f,
                                         vector_tmp_spadata_transform[0], 1.0f, 0.0f, 0.0f,
                                         vector_tmp_spadata_transform[1], vector_tmp_spadata_transform[2], 1.0f, 0.0f,
                                         0.0f, 0.0f, 0.0f, 1.0f);
        return &matrix_tmp_spadata_transform;
    default:
        return &NaMATRIX<float, 4, 4>::IDENT;
    }
}

NaMATRIX<float, 4, 4>* SpaNodeAnimation::GetMatrix(float time) const {
    /* FIXME: static local; see the note in GetSprineValue */
    extern NaMATRIX<float, 4, 4> matrix_tmp_spadata_node;
    extern int tmp_0_node_matrix;
    if (tmp_0_node_matrix == 0) {
        new (&matrix_tmp_spadata_node) NaMATRIX<float, 4, 4>;
        tmp_0_node_matrix = 1;
    }

    asm volatile("
        vmove.xyzw $vf16, $vf0
        vmr32.xyzw $vf15, $vf0
        vmr32.xyzw $vf14, $vf15
        vmr32.xyzw $vf13, $vf14
    ");

    for (u_int i = 0; i < this->m_transform_count; i++) {
        const NaMATRIX<float, 4, 4> *m = this->m_transforms[i]->GetMatrix(time);
        asm volatile("
            lqc2         $vf4, 0x0(%0)
            lqc2         $vf5, 0x10(%0)
            lqc2         $vf6, 0x20(%0)
            lqc2         $vf7, 0x30(%0)
            vmulax.xyzw  ACC, $vf4, $vf13x
            vmadday.xyzw ACC, $vf5, $vf13y
            vmaddaz.xyzw ACC, $vf6, $vf13z
            vmaddw.xyzw  $vf13, $vf7, $vf13w
            vmulax.xyzw  ACC, $vf4, $vf14x
            vmadday.xyzw ACC, $vf5, $vf14y
            vmaddaz.xyzw ACC, $vf6, $vf14z
            vmaddw.xyzw  $vf14, $vf7, $vf14w
            vmulax.xyzw  ACC, $vf4, $vf15x
            vmadday.xyzw ACC, $vf5, $vf15y
            vmaddaz.xyzw ACC, $vf6, $vf15z
            vmaddw.xyzw  $vf15, $vf7, $vf15w
            vmulax.xyzw  ACC, $vf4, $vf16x
            vmadday.xyzw ACC, $vf5, $vf16y
            vmaddaz.xyzw ACC, $vf6, $vf16z
            vmaddw.xyzw  $vf16, $vf7, $vf16w
        " : : "r"(m));
    }

    asm volatile("
        sqc2 $vf13, 0x0(%0)
        sqc2 $vf14, 0x10(%0)
        sqc2 $vf15, 0x20(%0)
        sqc2 $vf16, 0x30(%0)
    " : : "r"(&matrix_tmp_spadata_node));

    return &matrix_tmp_spadata_node;
}

bool SpaNodeAnimation::IsVisible(float time) const {
    if (m_visibility == NULL) {
        return true;
    }

    return *m_visibility->GetValue(time);
}

/*
 * A node with a visibility track takes its visibility from it; 0x40000 in
 * m_flags records that. Without one it inherits from its parent node,
 * if that node is animated, or else uses its own static flags.
 */
bool SpaFileHeader::IsNodeVisible(SpmNode *node, float time) const {
    SpaNodeAnimation *animation = this->m_nodes[node->m_index];
    SpmNode *other = node->m_parent;

    if (animation != NULL && animation->m_visibility == NULL) {
        animation = NULL;
    }

    if (other != NULL) {
        if (!(other->m_flags & 0x4000)) {
            return false;
        }
        if (other->m_flags & 0x40000) {
            if (animation == NULL) {
                node->m_flags |= 0x40000;
                if (other->m_flags & 0x4000) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    }

    if (animation != NULL) {
        node->m_flags |= 0x40000;
        return animation->IsVisible(time);
    }

    node->m_flags &= ~0x40000;
    return (node->m_flags & 0x20000) ? false : true;
}

/* True if the transform has one key and that key is the identity */
bool SpaTransform::IsEverIdentical() {
    switch (m_type) {
    case SCALE:
        if (GetKeyCount() == 1) {
            const NaVECTOR<float, 4>& scale = GetTrack<NaVECTOR<float, 4> >()->GetKeyValue(0);
            bool identical = false;
            if (scale[0] == 1.0f) {
                if (scale[1] != 1.0f) {
                    return false;
                }
                if (scale[2] == 1.0f) {
                    identical = true;
                }
            }
            return identical;
        }

        return false;

    case ROTATE_AXIS:
        if (GetKeyCount() == 1) {
            if (GetTrack<NaVECTOR<float, 4> >()->GetKeyValue(0)[3] == 0.0f) {
                return true;
            }
        }

        return false;

    case ROTATE_X:
    case ROTATE_Y:
    case ROTATE_Z:
        if (GetKeyCount() == 1) {
            if (GetTrack<float>()->GetKeyValue(0) == 0.0f) {
                return true;
            }
        }

        return false;

    case TRANSLATE:
        if (GetKeyCount() == 1) {
            const NaVECTOR<float, 4>& offset = GetTrack<NaVECTOR<float, 4> >()->GetKeyValue(0);
            bool identical = false;
            if (offset[0] == 0.0f) {
                if (offset[1] != 0.0f) {
                    return false;
                }
                if (offset[2] == 0.0f) {
                    identical = true;
                }
            }
            return identical;
        }

        return false;

    case MATRIX:
        if (GetKeyCount() == 1) {
            return GetTrack<NaMATRIX<float, 4, 4> >()->GetKeyValue(0).inl0();
        }

        return false;
        
    case SHEAR:
        if (GetKeyCount() == 1) {
            const NaVECTOR<float, 4>& shear = GetTrack<NaVECTOR<float, 4> >()->GetKeyValue(0);
            bool identical = false;
            if (shear[0] == 0.0f) {
                if (shear[1] != 0.0f) {
                    return false;
                }
                if (shear[2] == 0.0f) {
                    identical = true;
                }
            }
            return identical;
        }

        return false;

    default:
        NaMATRIX<float, 4, 4>& ident = NaMATRIX<float, 4, 4>::IDENT;
        return ident.inl1();
    }
}

/* Drops transforms that never change anything; returns how many */
int SpaNodeAnimation::Optimize() {
    int remove_count = 0;

    for (int i = 0; i < m_transform_count; i++) {
        SpaTransform *transform = m_transforms[i];
        if (transform == NULL || transform->IsEverIdentical()) {
            remove_count++;
            continue;
        }
        m_transforms[i - remove_count] = m_transforms[i];
    }

    m_transform_count -= remove_count;
    return remove_count;
}

template <typename T>
const T* SpaTrack<T>::GetValue(float time) const {
    if (m_flags & LOOP) {
        float length = m_times[m_key_count - 1];
        if (time < 0.0f || time >= length) {
            time = fmodf(time, length);
        }
    }

    u_int seg = SearchSegment(time);

    /* Before the first key or past the last one, hold the end value */
    if (seg == (u_int)-1) {
        return &m_values[0];
    }

    if (seg == m_key_count) {
        if (m_interpolation == SPLINE) {
            return &m_values[(seg - 1) * 3];
        } else {
            return &m_values[seg - 1];
        }
    }

    switch (m_interpolation) {
    case SPLINE:
        return GetSprineValue(seg, time);
    case LINEAR:
        return GetLinearValue(seg, time);
    case STEP:
        return &m_values[seg];
    default:
        break;
    }

    return NULL;
}

template <typename T>
const T* SpaTrack<T>::GetLinearValue(u_int seg, float time) const {
    static T value;

    float d0 = time - m_times[seg];
    float d1 = m_times[seg + 1] - time;

    value = (d1 * m_values[seg] + d0 * m_values[seg + 1]) / (d1 + d0);
    return &value;
}

/*
 * Cubic Hermite spline. Each key stores its value and two tangents, so
 * the segment runs from p0 with out-tangent m0 to p1 with in-tangent m1.
 */
template <typename T>
const T* SpaTrack<T>::GetSprineValue(u_int seg, float time) const {
    static T value;

    float dt = m_times[seg + 1] - m_times[seg];
    const T& p0 = m_values[seg * 3];
    const T& m0 = m_values[seg * 3 + 2];
    const T& m1 = m_values[seg * 3 + 4];
    const T& p1 = m_values[seg * 3 + 3];
    if (dt == 0.0f) {
        return &p0;
    }

    float t = (time - m_times[seg]) / dt;
    T d = p0 - p1;

    value = t * (t * (t * ((m0 + m1) * dt + d * 2.0f) - (m0 * 2.0f + m1) * dt - d * 3.0f) + m0 * dt) + p0;
    return &value;
}

#ifndef NON_MATCHING
/* nalib/namatrix.h and the SpaTrack templates above, in the original's order */
INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetLinearValue__Ct8SpaTrack1Zt8NaVECTOR2Zfi4Uif);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetValue__Ct8SpaTrack1Zt8NaVECTOR2Zfi4f);

/* Weak copies of the 9- and 16-argument NaMATRIX<float, 4, 4>::Set */
INCLUDE_ASM("asm/nonmatchings/prlib/spadata", func_00149168);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", func_001491C0);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", RotateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4RCf);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetSprineValue__Ct8SpaTrack1ZfUif);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetLinearValue__Ct8SpaTrack1ZfUif);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetValue__Ct8SpaTrack1Zff);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetSprineValue__Ct8SpaTrack1Zt8NaMATRIX3Zfi4i4Uif);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetLinearValue__Ct8SpaTrack1Zt8NaMATRIX3Zfi4i4Uif);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetValue__Ct8SpaTrack1Zt8NaMATRIX3Zfi4i4f);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", TranslateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", RotateMatrix__t8NaMATRIX3Zfi4i4iRCf);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", ScaleMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4);
#endif

/* Same Hermite spline as the generic version, written as one VU0 block */
template <>
const NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetSprineValue(u_int seg, float time) const {
    float dt = m_times[seg + 1] - m_times[seg];
    const NaVECTOR<float, 4> *key = &m_values[seg * 3];
    if (dt == 0.0f) {
        return key;
    }

    float t = (time - m_times[seg]) / dt;

    /*
     * FIXME(poly): These `tmp_X` symbols aren't real
     * and are product of the static NaVECTOR instances.
     *
     * The solution would be to make these static
     * (when the TU's .bss matches, of course), and
     * the compiler should generate these temp. symbols.
     *
     * The symbols themselves are meant to call the
     * constructor once, though in this case there's
     * no actual call because it is inlined (empty constructor).
     */
    extern NaVECTOR<float, 4> return_buffer;
    extern int tmp_0_1744;
    if (tmp_0_1744 == 0) {
        tmp_0_1744 = 1;
    }

    #if 0
        vf04.xyzw = key[0];
        vf05.xyzw = key[3];
        vf01.xyzw = vf04.xyzw - vf05.xyzw;
        
        vf06.xyzw = key[2];
        vf07.xyzw = key[4];
        vf02.xyzw = vf06.xyzw + vf07.xyzw;
        
        vf08.x    = dt;
        ACC.xyzw  = vf01.xyzw + vf01.xyzw;
        vf03.xyzw = ACC.xyzw + (vf02.xyzw * vf08.x);
        
        vf09.x    = t;
        ACC.xyzw  = vf06.xyzw + vf06.xyzw;
        vf02.xyzw = ACC.xyzw + (vf07.xyzw * vf00.w);
        ACC.w     = vf00.w + vf00.w;
        vf08.w    = ACC.w + (vf00.w * vf00.w);
        ACC.xyzw  = vf03.xyzw * vf09.x;
        ACC.xyzw  = ACC.xyzw - (vf02.xyzw * vf08.x);
        vf10.x    = vf09.x * vf08.x;
        vf02.x    = vf09.x * vf09.x;
        vf03.xyzw = ACC.xyzw - (vf01.xyzw * vf08.w);
        ACC.xyzw  = vf04.xyzw + vf00.x;
        ACC.xyzw  = ACC.xyzw + (vf06.xyzw * vf10.x);
        vf01.xyzw = ACC.xyzw + (vf03.xyzw * vf02.x);
        return_buffer = vf01.xyzw;
    #else
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
    : : "r"(&return_buffer), "r"(key), "r"(dt), "r"(t));
    #endif

    return &return_buffer;
}
