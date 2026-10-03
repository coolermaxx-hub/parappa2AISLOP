#ifndef PRLIB_MENDERER_H
#define PRLIB_MENDERER_H

#include <eetypes.h>

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

#endif /* PRLIB_MENDERER_H */
