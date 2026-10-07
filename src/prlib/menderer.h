#ifndef PRLIB_MENDERER_H
#define PRLIB_MENDERER_H

#include <eetypes.h>

// Number of noodle polygons; each has its own drifting position
// (mendererdata.cpp) and the strip pass draws up to this many quads.
static const u_int PR_NOODLE_POLYGON_COUNT = 115;

// Noodle ("menderer") renderer state shared by the menderer*.cpp files.
// The definitions and their initial values live in menderer.cpp.
extern float prMendererRatio;
extern float prMendererSyncRatio;
extern int prMendererGettingWorse;
extern int prMendererColorModulation;
extern float prMendererSpeed;
extern float prMendererFade;
extern float prMendererDistance;
extern float prMendererWidth;
extern float prMendererLength;
extern float prSchoolLeaderIndex;
extern u_int prMendererTbp;
extern u_int prMendererWorkFbp;
extern u_int prMendererDrawFbp;
extern float prMendererNoodleColor[4];

#endif /* PRLIB_MENDERER_H */
