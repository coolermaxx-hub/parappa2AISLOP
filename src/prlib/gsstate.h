#ifndef PRLIB_GSSTATE_H
#define PRLIB_GSSTATE_H

#include "common.h"

#include <libgraph.h>

/* GS register values shared by the prlib renderers. ZTE is always on: the
 * depth test is disabled by passing SCE_GS_ZALWAYS rather than clearing ZTE. */

/* No alpha test, with the given depth test. */
#define PR_TEST_NO_ALPHA(ztst) \
    SCE_GS_SET_TEST(/*ATE*/0, /*ATST*/SCE_GS_ALPHA_NEVER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_KEEP, \
                    /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/(ztst))

/* Draw only pixels with alpha above zero, with the given depth test. */
#define PR_TEST_ALPHA_NONZERO(ztst) \
    SCE_GS_SET_TEST(/*ATE*/1, /*ATST*/SCE_GS_ALPHA_GREATER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_KEEP, \
                    /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/(ztst))

/* Every pixel fails the alpha test and writes Z only. */
#define PR_TEST_Z_ONLY \
    SCE_GS_SET_TEST(/*ATE*/1, /*ATST*/SCE_GS_ALPHA_NEVER, /*AREF*/0, /*AFAIL*/SCE_GS_AFAIL_ZB_ONLY, \
                    /*DATE*/0, /*DATM*/0, /*ZTE*/1, /*ZTST*/SCE_GS_ZALWAYS)

/* (Cs - Cd) * As + Cd. FIX is unused but written as 128. */
#define PR_ALPHA_BLEND SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_AS, SCE_GS_ALPHA_CD, 128)

/* (Cs - Cd) * fix / 128 + Cd: blend at a fixed opacity. */
#define PR_ALPHA_FIXED(fix) SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_FIX, SCE_GS_ALPHA_CD, (fix))

/* Bilinear magnify and minify. */
#define PR_TEX1_BILINEAR \
    SCE_GS_SET_TEX1(/*LCM*/0, /*MXL*/0, /*MMAG*/SCE_GS_LINEAR, /*MMIN*/SCE_GS_LINEAR, /*MTBA*/0, /*L*/0, /*K*/0)

/* Bilinear magnify and minify with the LOD fixed at K = 0. */
#define PR_TEX1_BILINEAR_LOD0 \
    SCE_GS_SET_TEX1(/*LCM*/1, /*MXL*/0, /*MMAG*/SCE_GS_LINEAR, /*MMIN*/SCE_GS_LINEAR, /*MTBA*/0, /*L*/0, /*K*/0)

/* FRAME for a screen-wide CT32 buffer at fbp. */
#define PR_FRAME_CT32(fbp) SCE_GS_SET_FRAME((fbp), SCREEN_WIDTH / 64, SCE_GS_PSMCT32, 0)

#endif /* PRLIB_GSSTATE_H */
