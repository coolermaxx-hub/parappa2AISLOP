#include "spadata.h"

#include "animation.h"
#include "model.h"

#include <math.h>

/* placement new, used to emulate static local construction */
inline void* operator new(size_t, void *p) {
    return p;
}

template <>
NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetSprineValue(u_int seg, float arg1) const;

u_int SpaTrackBase::SearchSegment(float arg0) const {
    if (this->unk4 == 1) {
        return (u_int)-1;
    }

    if (arg0 <= this->unkC[0]) {
        return (u_int)-1;
    }

    if (arg0 >= this->unkC[this->unk4 - 1]) {
        return this->unk4;
    }

    if (arg0 >= this->unkC[this->unk8]) {
        if (arg0 < this->unkC[this->unk8 + 1]) {
            return this->unk8;
        }

        if ((this->unk8 + 2) < this->unk4) {
            if (arg0 < this->unkC[this->unk8 + 2]) {
                return ++this->unk8;
            }
        }
    } else if (this->unk8 != 0) {
        if (arg0 >= this->unkC[this->unk8 - 1]) {
            return --this->unk8;
        }
    }

    int right = this->unk4 - 2;
    int left = 0;
    int mid = 0;

    while (left <= right) {
        mid = (left + right) / 2;
        if (arg0 < this->unkC[mid]) {
            right = mid - 1;
        } else if (arg0 >= this->unkC[mid + 1]) {
            left = mid + 1;
        } else {
            break;
        }
    }

    this->unk8 = mid;
    return this->unk8;
}

template <>
int* SpaTrack<int>::GetValue(float arg0) const {
    if (this->unk2 & 0x1) {
        float f13 = this->unkC[this->unk4 - 1];
        if (arg0 < 0.0f || arg0 >= f13) {
            arg0 = fmodf(arg0, f13);
        }
    }

    u_int seg = this->SearchSegment(arg0);
    if (seg == (u_int)-1) {
        return (int*)&this->unk10;
    }

    if (seg == this->unk4) {
        return (int*)&this->unkC + seg;
    } else {
        return (int*)&this->unk10 + seg;
    }
}

template <> NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetValue(float arg0) const;
template <> float* SpaTrack<float>::GetValue(float arg0) const;
template <> NaMATRIX<float, 4, 4>* SpaTrack<NaMATRIX<float, 4, 4> >::GetValue(float arg0) const;

/* Template instances emitted later in this TU */
NaMATRIX<float, 4, 4> ScaleMatrix_tmp_spadata(const NaVECTOR<float, 4>& v) asm("func_0014AFE0");
NaMATRIX<float, 4, 4> TransMatrix_tmp_spadata(const NaVECTOR<float, 4>& v) asm("func_0014ABE0");
NaMATRIX<float, 4, 4>& SetMatrix_tmp_spadata(NaMATRIX<float, 4, 4> *m,
    const float& m00, const float& m01, const float& m02, const float& m03,
    const float& m10, const float& m11, const float& m12, const float& m13,
    const float& m20, const float& m21, const float& m22, const float& m23,
    const float& m30, const float& m31, const float& m32, const float& m33) asm("Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1");

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetMatrix__C12SpaTransformf);
#else /* Scheduling: two instructions swapped in case 7 */
NaMATRIX<float, 4, 4>* SpaTransform::GetMatrix(float arg0) const {
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

    switch (this->unk0) {
    case 0:
        vector_tmp_spadata_transform = *((SpaTrack<NaVECTOR<float, 4> >*)&this->unk10)->GetValue(arg0);
        ((float*)&vector_tmp_spadata_transform)[3] = 1.0f;
        matrix_tmp_spadata_transform = ScaleMatrix_tmp_spadata(vector_tmp_spadata_transform);
        return &matrix_tmp_spadata_transform;
    case 1: {
        vector_tmp_spadata_transform = *((SpaTrack<NaVECTOR<float, 4> >*)&this->unk10)->GetValue(arg0);
        float angle = vector_tmp_spadata_transform[3];
        ((float*)&vector_tmp_spadata_transform)[3] = 1.0f;
        matrix_tmp_spadata_transform = NaMATRIX<float, 4, 4>::RotateMatrix(vector_tmp_spadata_transform, angle);
        return &matrix_tmp_spadata_transform;
    }
    case 2:
    case 3:
    case 4: {
        float angle = *((SpaTrack<float>*)&this->unk10)->GetValue(arg0);
        matrix_tmp_spadata_transform = NaMATRIX<float, 4, 4>::RotateMatrix(this->unk0 - 2, angle);
        return &matrix_tmp_spadata_transform;
    }
    case 5:
        vector_tmp_spadata_transform = *((SpaTrack<NaVECTOR<float, 4> >*)&this->unk10)->GetValue(arg0);
        matrix_tmp_spadata_transform = TransMatrix_tmp_spadata(vector_tmp_spadata_transform);
        return &matrix_tmp_spadata_transform;
    case 6:
        return ((SpaTrack<NaMATRIX<float, 4, 4> >*)&this->unk10)->GetValue(arg0);
    case 7: {
        vector_tmp_spadata_transform = *((SpaTrack<NaVECTOR<float, 4> >*)&this->unk10)->GetValue(arg0);
        NaMATRIX<float, 4, 4> *ret = &matrix_tmp_spadata_transform;
        SetMatrix_tmp_spadata(ret,
            1.0f, 0.0f, 0.0f, 0.0f,
            ((float*)&vector_tmp_spadata_transform)[0], 1.0f, 0.0f, 0.0f,
            ((float*)&vector_tmp_spadata_transform)[1], ((float*)&vector_tmp_spadata_transform)[2], 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
        return ret;
    }
    default:
        return &NaMATRIX<float, 4, 4>::IDENT;
    }
}
#endif

NaMATRIX<float, 4, 4>* SpaNodeAnimation::GetMatrix(float arg0) const {
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

    for (u_int i = 0; i < this->unk8; i++) {
        NaMATRIX<float, 4, 4> *m = this->unkC[i]->GetMatrix(arg0);
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

bool SpaNodeAnimation::IsVisible(float arg0) const {
    if (this->unk4 == NULL) {
        return true;
    }

    return *this->unk4->GetValue(arg0);
}

bool SpaFileHeader::IsNodeVisible(SpmNode *arg0, float arg1) const {
    SpaNodeAnimation *a0 = this->unk50[arg0->unk150];
    SpmNode *a2 = arg0->unk164;

    if (a0 != NULL && a0->unk4 == NULL) {
        a0 = NULL;
    }

    if (a2 != NULL) {
        if (!(a2->m_flags & 0x4000)) {
            return false;
        }
        if (a2->m_flags & 0x40000) {
            if (a0 == NULL) {
                arg0->m_flags |= 0x40000;
                if (a2->m_flags & 0x4000) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    }

    if (a0 != NULL) {
        arg0->m_flags |= 0x40000;
        return a0->IsVisible(arg1);
    }

    arg0->m_flags &= ~0x40000;
    return (arg0->m_flags & 0x20000) ? false : true;
}

bool SpaTransform::IsEverIdentical() {
    switch (this->unk0) {
    case 0:
        if (this->unk14 == 1) {
            bool v0 = false;
            if (this->unk20[0][0] == 1.0f) {
                if (this->unk20[0][1] != 1.0f) {
                    return false;
                }
                if (this->unk20[0][2] == 1.0f) {
                    v0 = true;
                }
            }
            return v0;
        }

        return false;

    case 1:
        if (this->unk14 == 1) {
            if (this->unk20[0][3] == 0.0f) {
                return true;
            }
        }

        return false;

    case 2:
    case 3:
    case 4:
        if (this->unk14 == 1) {
            if (this->unk20[0][0] == 0.0f) {
                return true;
            }
        }

        return false;

    case 5:
        if (this->unk14 == 1) {
            bool v0 = false;
            if (this->unk20[0][0] == 0.0f) {
                if (this->unk20[0][1] != 0.0f) {
                    return false;
                }
                if (this->unk20[0][2] == 0.0f) {
                    v0 = true;
                }
            }
            return v0;
        }

        return false;

    case 6:
        if (this->unk14 == 1) {
            return this->unk20.inl0();
        }

        return false;
        
    case 7:
        if (this->unk14 == 1) {
            bool v0 = false;
            if (this->unk20[0][0] == 0.0f) {
                if (this->unk20[0][1] != 0.0f) {
                    return false;
                }
                if (this->unk20[0][2] == 0.0f) {
                    v0 = true;
                }
            }
            return v0;
        }

        return false;

    default:
        NaMATRIX<float, 4, 4>& a2 = NaMATRIX<float, 4, 4>::IDENT;
        return a2.inl1();
    }
}

int SpaNodeAnimation::Optimize() {
    int remove_count = 0;

    for (int i = 0; i < this->unk8; i++) {
        SpaTransform *transform = this->unkC[i];
        if (transform == NULL || transform->IsEverIdentical()) {
            remove_count++;
            continue;
        }
        this->unkC[i - remove_count] = this->unkC[i];
    }

    this->unk8 -= remove_count;
    return remove_count;
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetLinearValue__Ct8SpaTrack1Zt8NaVECTOR2Zfi4Uif);
#else /* Regalloc */
template <>
NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetLinearValue(u_int seg, float arg1) const {
    /* FIXME: static local; see the note in GetSprineValue below */
    extern NaVECTOR<float, 4> value_tmp_spadata_linear_vector;
    extern int tmp_0_linear_vector;
    if (tmp_0_linear_vector == 0) {
        tmp_0_linear_vector = 1;
    }

    float *keys = this->unkC;
    float d0 = arg1 - keys[seg];
    float d1 = keys[seg + 1] - arg1;
    NaVECTOR<float, 4> *values = (NaVECTOR<float, 4>*)this;

    value_tmp_spadata_linear_vector = (values[seg + 1] * d1 + values[seg + 2] * d0) / (d1 + d0);
    return &value_tmp_spadata_linear_vector;
}
#endif

template <>
NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetValue(float arg0) const {
    if (this->unk2 & 0x1) {
        float f13 = this->unkC[this->unk4 - 1];
        if (arg0 < 0.0f || arg0 >= f13) {
            arg0 = fmodf(arg0, f13);
        }
    }

    u_int seg = this->SearchSegment(arg0);

    if (seg == (u_int)-1) {
        return (NaVECTOR<float, 4>*)&this->unk10;
    }

    if (seg == this->unk4) {
        if (this->unk0 == 0) {
            return (NaVECTOR<float, 4>*)&this->unk10 + ((seg - 1) * 3);
        } else {
            return (NaVECTOR<float, 4>*)this + seg;
        }
    }

    switch (this->unk0) {
    case 0:
        return this->GetSprineValue(seg, arg0);
    case 1:
        return this->GetLinearValue(seg, arg0);
    case 2:
        return (NaVECTOR<float, 4>*)&this->unk10 + seg;
    default:
        break;
    }

    return NULL;
}

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/spadata", func_00149168);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", func_001491C0);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/spadata", RotateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4RCf);
#else
static inline NaMATRIX<float, 4, 4> MakeMatrix_tmp_spadata(
    const float& m00, const float& m01, const float& m02, const float& m03,
    const float& m10, const float& m11, const float& m12, const float& m13,
    const float& m20, const float& m21, const float& m22, const float& m23,
    const float& m30, const float& m31, const float& m32, const float& m33) return ret {
    SetMatrix_tmp_spadata(&ret, m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

template <>
NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::RotateMatrix(const NaVECTOR<float, 4>& axis, const float& angle) {
    float yz = axis[1] * axis[1] + axis[2] * axis[2];
    float len = sqrtf(axis[0] * axis[0] + yz);
    float r = sqrtf(yz);
    float p, q;

    if (r < 1.1920929e-07f) {
        p = 0.0f;
        q = 1.0f;
    } else {
        p = axis[1] / r;
        q = axis[2] / r;
    }

    float a = r / len;
    float b = -axis[0] / len;
    float s = sinf(angle);
    float c = cosf(angle);
    float k = a * (c - 1.0f);

    return MakeMatrix_tmp_spadata(a * k + 1.0f, p * b * k + q * a * s, q * b * k - p * a * s, 0.0f,
                                  p * b * k - q * a * s, -p * p * a * k + c, -p * q * a * k - b * s, 0.0f,
                                  q * b * k + p * a * s, -p * q * a * k + b * s, -q * q * a * k + c, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f);
}
#endif

/* prlib/spadata.cpp */
template <>
float* SpaTrack<float>::GetSprineValue(u_int seg, float arg1) const {
    extern float value_tmp_spadata_sprine_float;

    float *keys = this->unkC;
    float dt = keys[seg + 1] - keys[seg];
    float *p0 = &((float*)this)[seg * 3 + 4];
    float *m0 = &((float*)this)[seg * 3 + 6];
    float *m1 = &((float*)this)[seg * 3 + 8];
    float *p1 = &((float*)this)[seg * 3 + 7];
    if (dt == 0.0f) {
        return p0;
    }

    float t = (arg1 - keys[seg]) / dt;
    float d = *p0 - *p1;

    value_tmp_spadata_sprine_float = t * (t * (t * ((*m0 + *m1) * dt + (d + d)) - (*m0 + *m0 + *m1) * dt - d * 3.0f) + *m0 * dt) + *p0;
    return &value_tmp_spadata_sprine_float;
}

template <>
float* SpaTrack<float>::GetLinearValue(u_int seg, float arg1) const {
    extern float value_tmp_spadata_linear_float;

    float *keys = this->unkC;
    u_int next = seg + 1;
    float *values = (float*)&this->unk10;
    float *v0 = &values[seg];
    float *v1 = &values[next];
    float d0 = arg1 - keys[seg];
    float d1 = keys[seg + 1] - arg1;

    value_tmp_spadata_linear_float = (d1 * *v0 + d0 * *v1) / (d1 + d0);
    return &value_tmp_spadata_linear_float;
}

template <>
float* SpaTrack<float>::GetValue(float arg0) const {
    if (this->unk2 & 0x1) {
        float f13 = this->unkC[this->unk4 - 1];
        if (arg0 < 0.0f || arg0 >= f13) {
            arg0 = fmodf(arg0, f13);
        }
    }

    u_int seg = this->SearchSegment(arg0);

    if (seg == (u_int)-1) {
        return (float*)&this->unk10;
    }

    if (seg == this->unk4) {
        if (this->unk0 == 0) {
            return (float*)&this->unk10 + ((seg - 1) * 3);
        } else {
            return (float*)&this->unkC + seg;
        }
    }

    switch (this->unk0) {
    case 0:
        return this->GetSprineValue(seg, arg0);
    case 1:
        return this->GetLinearValue(seg, arg0);
    case 2:
        return (float*)&this->unk10 + seg;
    default:
        break;
    }

    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetSprineValue__Ct8SpaTrack1Zt8NaMATRIX3Zfi4i4Uif);

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", GetLinearValue__Ct8SpaTrack1Zt8NaMATRIX3Zfi4i4Uif);

template <>
NaMATRIX<float, 4, 4>* SpaTrack<NaMATRIX<float, 4, 4> >::GetValue(float arg0) const {
    if (this->unk2 & 0x1) {
        float f13 = this->unkC[this->unk4 - 1];
        if (arg0 < 0.0f || arg0 >= f13) {
            arg0 = fmodf(arg0, f13);
        }
    }

    u_int seg = this->SearchSegment(arg0);

    if (seg == (u_int)-1) {
        return (NaMATRIX<float, 4, 4>*)&this->unk10;
    }

    if (seg == this->unk4) {
        if (this->unk0 == 0) {
            return (NaMATRIX<float, 4, 4>*)&this->unk10 + ((seg - 1) * 3);
        } else {
            return (NaMATRIX<float, 4, 4>*)&this->unk10 + (seg - 1);
        }
    }

    switch (this->unk0) {
    case 0:
        return this->GetSprineValue(seg, arg0);
    case 1:
        return this->GetLinearValue(seg, arg0);
    case 2:
        return (NaMATRIX<float, 4, 4>*)&this->unk10 + seg;
    default:
        break;
    }

    return NULL;
}

/* nalib/navector.h */
INCLUDE_ASM("asm/nonmatchings/prlib/spadata", func_0014ABE0);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/prlib/spadata", RotateMatrix__t8NaMATRIX3Zfi4i4iRCf);
#else
template <>
NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::RotateMatrix(int axis, const float& angle) {
    float c = cosf(angle);
    float s = sinf(angle);

    if (axis == 0) {
        return MakeMatrix_tmp_spadata(1.0f, 0.0f, 0.0f, 0.0f,
                                      0.0f, c, s, 0.0f,
                                      0.0f, -s, c, 0.0f,
                                      0.0f, 0.0f, 0.0f, 1.0f);
    } else if (axis == 1) {
        return MakeMatrix_tmp_spadata(c, 0.0f, -s, 0.0f,
                                      0.0f, 1.0f, 0.0f, 0.0f,
                                      s, 0.0f, c, 0.0f,
                                      0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        return MakeMatrix_tmp_spadata(c, s, 0.0f, 0.0f,
                                      -s, c, 0.0f, 0.0f,
                                      0.0f, 0.0f, 1.0f, 0.0f,
                                      0.0f, 0.0f, 0.0f, 1.0f);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/prlib/spadata", func_0014AFE0);

template <>
NaVECTOR<float, 4>* SpaTrack<NaVECTOR<float, 4> >::GetSprineValue(u_int seg, float arg1) const {
    float f2 = this->unkC[seg + 1] - this->unkC[seg];
    NaVECTOR<float, 4> *a0 = (NaVECTOR<float, 4>*)&this->unk10 + (seg * 3);
    if (f2 == 0.0f) {
        return a0;
    }

    float f0 = (arg1 - this->unkC[seg]) / f2;

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
        vf04.xyzw = a0[0];
        vf05.xyzw = a0[3];
        vf01.xyzw = vf04.xyzw - vf05.xyzw;
        
        vf06.xyzw = a0[2];
        vf07.xyzw = a0[4];
        vf02.xyzw = vf06.xyzw + vf07.xyzw;
        
        vf08.x    = f2;
        ACC.xyzw  = vf01.xyzw + vf01.xyzw;
        vf03.xyzw = ACC.xyzw + (vf02.xyzw * vf08.x);
        
        vf09.x    = f0;
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
    : : "r"(&return_buffer), "r"(a0), "r"(f2), "r"(f0));
    #endif

    return &return_buffer;
}
