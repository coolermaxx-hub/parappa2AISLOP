#include "menu/menu_mdl.h"

#include "os/system.h"

#include "main/cdctrl.h"

#include "menu/menusub.h"

#include <prlib/prlib.h>

#include <math.h>
#include <string.h>

static MN_MDLTBL Mdl_StageMapH[] = {
    { 47, { 104 } },
    { 76, { 152, 151, 149, 150 }, { 153, 154, 155, 156, 157, 158, 159, 160, 162, 163, 164, 165, 166, 161, 167 } },
    { 19, { 93 } },
    { 22, { 94 } },
    { 25, { 95 } },
    { 28, { 96 } },
    { 31, { 97 } },
    { 34, { 98 } },
    { 37, { 99 } },
    { 40, { 100 } },
    { 79, { 168 } },
    { 43, { 101 } },
    { 52, { 113, 114, 115, 116 } },
    { 55, { 117, 118, 119, 120 } },
    { 58, { 121, 122, 123, 124 } },
    { 61, { 125, 126, 127, 128 } },
    { 64, { 129, 130, 131, 132 } },
    { 67, { 133, 134, 135, 136 } },
    { 70, { 137, 138, 139, 140 } },
    { 73, { 141, 142, 143, 144 } },
    { 0 },
};
static MN_MDLTBL Mdl_StageMapA[] = {
    { 46, { 104 } },
    { 76, { 152, 151, 149, 150 }, { 153, 154, 155, 156, 157, 158, 159, 160, 162, 163, 164, 165, 166, 161, 167 } },
    { 20, { 93 } },
    { 23, { 94 } },
    { 26, { 95 } },
    { 29, { 96 } },
    { 32, { 97 } },
    { 35, { 98 } },
    { 38, { 99 } },
    { 41, { 100 } },
    { 80, { 168 } },
    { 44, { 102 } },
    { 52, { 113, 114, 115, 116 } },
    { 55, { 117, 118, 119, 120 } },
    { 58, { 121, 122, 123, 124 } },
    { 61, { 125, 126, 127, 128 } },
    { 64, { 129, 130, 131, 132 } },
    { 67, { 133, 134, 135, 136 } },
    { 70, { 137, 138, 139, 140 } },
    { 73, { 141, 142, 143, 144 } },
    { 0 },
};
static MN_MDLTBL Mdl_StageMapY[] = {
    { 48, { 104 } },
    { 76, { 152, 151, 149, 150 }, { 153, 154, 155, 156, 157, 158, 159, 160, 162, 163, 164, 165, 166, 161, 167 } },
    { 21, { 93 } },
    { 24, { 94 } },
    { 27, { 95 } },
    { 30, { 96 } },
    { 33, { 97 } },
    { 36, { 98 } },
    { 39, { 99 } },
    { 42, { 100 } },
    { 81, { 168 } },
    { 45, { 103 } },
    { 52, { 113, 114, 115, 116 } },
    { 55, { 117, 118, 119, 120 } },
    { 58, { 121, 122, 123, 124 } },
    { 61, { 125, 126, 127, 128 } },
    { 64, { 129, 130, 131, 132 } },
    { 67, { 133, 134, 135, 136 } },
    { 70, { 137, 138, 139, 140 } },
    { 73, { 141, 142, 143, 144 } },
    { 0 },
};
static int Cam_StageMap[] = { 180, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 0 };
MN_SCENETBL Scene_StageMap  = { Mdl_StageMapH, Cam_StageMap };
MN_SCENETBL Scene_StageMapA = { Mdl_StageMapA, Cam_StageMap };
MN_SCENETBL Scene_StageMapY = { Mdl_StageMapY, Cam_StageMap };
static PRPOS PRP_CTHAL[] = {
    { 0.0f, 0.0f, 56.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 56.0f, 0.0f, 0.0f },
};
static PRPROOT PRP_RootTbl[] = {
    { PRP_CTHAL, 2 },
};
MNANM_TBL StageMapAnime[] = {
    { MNANM_PLAY_ONCE, 0, 0, 0, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 30, 30, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 50, 50, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 70, 70, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 90, 90, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 110, 110, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 130, 130, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 150, 150, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 170, 170, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 190, 190, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 200, 200, 1, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 1, 0, 0, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 30, 30, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 50, 50, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 70, 70, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 90, 90, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 110, 110, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 130, 130, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 150, 150, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 170, 170, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 190, 190, 1, 0, { { MNANM_MOTION, 0 } } },
    { MNANM_PLAY_ONCE, 1, 200, 200, 1, 0, { { MNANM_MOTION, 0 } } },
};
MNANM_TBL StageMapAnimeBB[] = {
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 2, 0 } } },
    { MNANM_PLAY_ONCE, 3, 0, 0, 1, 0, { { MNANM_VISIBLE | 2, 0 }, { MNANM_VISIBLE | 12, 0 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 3, 0 } } },
    { MNANM_PLAY_ONCE, 3, 0, 0, 1, 0, { { MNANM_VISIBLE | 3, 0 }, { MNANM_VISIBLE | 13, 0 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 4, 0 } } },
    { MNANM_PLAY_ONCE, 3, 0, 0, 1, 0, { { MNANM_VISIBLE | 4, 0 }, { MNANM_VISIBLE | 14, 0 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 5, 0 } } },
    { MNANM_PLAY_ONCE, 3, 0, 0, 1, 0, { { MNANM_VISIBLE | 5, 0 }, { MNANM_VISIBLE | 15, 0 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 6, 0 } } },
    { MNANM_PLAY_ONCE, 3, 0, 0, 1, 0, { { MNANM_VISIBLE | 6, 0 }, { MNANM_VISIBLE | 16, 0 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 7, 0 } } },
    { MNANM_PLAY_ONCE, 3, 0, 0, 1, 0, { { MNANM_VISIBLE | 7, 0 }, { MNANM_VISIBLE | 17, 0 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 8, 0 } } },
    { MNANM_PLAY_ONCE, 3, 0, 0, 1, 0, { { MNANM_VISIBLE | 8, 0 }, { MNANM_VISIBLE | 18, 0 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 9, 0 } } },
    { MNANM_PLAY_ONCE, 3, 0, 0, 1, 0, { { MNANM_VISIBLE | 9, 0 }, { MNANM_VISIBLE | 19, 0 } } },
};
static MNANM_TBL StageMapAnimeCW1[] = {
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 12, 0 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 12, 1 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 12, 2 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 12, 3 } } },
};
static MNANM_TBL StageMapAnimeCW2[] = {
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 13, 0 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 13, 1 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 13, 2 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 13, 3 } } },
};
static MNANM_TBL StageMapAnimeCW3[] = {
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 14, 0 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 14, 1 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 14, 2 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 14, 3 } } },
};
static MNANM_TBL StageMapAnimeCW4[] = {
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 15, 0 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 15, 1 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 15, 2 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 15, 3 } } },
};
static MNANM_TBL StageMapAnimeCW5[] = {
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 16, 0 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 16, 1 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 16, 2 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 16, 3 } } },
};
static MNANM_TBL StageMapAnimeCW6[] = {
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 17, 0 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 17, 1 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 17, 2 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 17, 3 } } },
};
static MNANM_TBL StageMapAnimeCW7[] = {
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 18, 0 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 18, 1 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 18, 2 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 18, 3 } } },
};
static MNANM_TBL StageMapAnimeCW8[] = {
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 19, 0 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 19, 1 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 19, 2 } } },
    { MNANM_PLAY_MUSIC, 7, 0, 274, 1, 0, { { MNANM_MOTION | 19, 3 } } },
};
MNANM_TBL *StageMapCWptr[] = {
    StageMapAnimeCW1, StageMapAnimeCW2, StageMapAnimeCW3, StageMapAnimeCW4, StageMapAnimeCW5,
    StageMapAnimeCW6, StageMapAnimeCW7, StageMapAnimeCW8,
};
MNANM_TBL StageMapAnimeSEA[] = {
    { MNANM_PLAY_ONCE, 4, 0, 0, 1, 0, { { MNANM_VISIBLE | 10, 0 } } },
    { MNANM_PLAY_MUSIC, 4, 0, 480, 1, 0, { { MNANM_MOTION | 10, 0 } } },
};
MNANM_TBL StageMapAnimeBK[] = {
    { MNANM_PLAY_MUSIC, 8, 0, -1, 1, 0, { { MNANM_MOTION | 11, 0 } } },
    { MNANM_PLAY_MUSIC, 8, 0, -1, 1, 0, { { MNANM_MOTION | 11, 0 } } },
    { MNANM_PLAY_MUSIC, 8, 0, -1, 1, 0, { { MNANM_MOTION | 11, 0 } } },
};
MNANM_TBL StageMapAnimePA[] = {
    { MNANM_PLAY_MUSIC, 5, 0, 144, 1, 0, { { MNANM_BLEND_FAST | 1, 1 } } },
    { MNANM_PLAY_LOOP, 5, 0, -1, 1, 0, { { MNANM_BLEND_FAST | 1, 0 } } },
    { MNANM_PLAY_LOOP, 5, 0, 120, 1, 0, { { MNANM_BLEND_FAST | 1, 2 } } },
    { MNANM_PLAY_LOOP, 5, 0, 144, 1, 0, { { MNANM_BLEND_FAST | 1, 3 } } },
    { MNANM_PLAY_ONCE, 5, 0, 74, 1, 0, { { MNANM_BLEND_FAST | 1, 2 } } },
    { MNANM_PLAY_ONCE, 5, 0, 74, 1, 0, { { MNANM_VISIBLE | 1, 0 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_MOVE_MODE | 1, 1 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 1 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 1 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 2 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 2 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 3 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 3 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 4 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 4 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 5 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 5 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 6 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 6 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 7 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 7 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 8 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 8 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 9 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 9 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 10 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 10 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 11 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 11 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 12 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 12 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 13 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 13 } } },
    { MNANM_PLAY_ONCE, 6, 0, 150, 2, 0, { { MNANM_POSITION | 1, 14 } } },
    { MNANM_PLAY_ONCE, 6, 200, 350, 2, 0, { { MNANM_POSITION | 1, 14 } } },
};
MNANM_TBL StageMapBGMCamera[] = {
    { MNANM_PLAY_ONCE, 0, 0, 10532, 1, 0, { { MNANM_CAMERA, 1 } } },
    { MNANM_PLAY_ONCE, 0, 0, 12834, 1, 0, { { MNANM_CAMERA, 2 } } },
    { MNANM_PLAY_ONCE, 0, 0, 14492, 1, 0, { { MNANM_CAMERA, 3 } } },
    { MNANM_PLAY_ONCE, 0, 0, 13410, 1, 0, { { MNANM_CAMERA, 4 } } },
    { MNANM_PLAY_ONCE, 0, 0, 11108, 1, 0, { { MNANM_CAMERA, 5 } } },
    { MNANM_PLAY_ONCE, 0, 0, 13834, 1, 0, { { MNANM_CAMERA, 6 } } },
    { MNANM_PLAY_ONCE, 0, 0, 12126, 1, 0, { { MNANM_CAMERA, 7 } } },
    { MNANM_PLAY_ONCE, 0, 0, 14254, 1, 0, { { MNANM_CAMERA, 8 } } },
    { MNANM_PLAY_ONCE, 0, 0, 14914, 1, 0, { { MNANM_CAMERA, 9 } } },
    { MNANM_PLAY_ONCE, 0, 0, 9890, 1, 0, { { MNANM_CAMERA, 10 } } },
};
MN_MDLTBL Mdl_CityHall[] = {
    { 82 },
    { 83, { 169 } },
    { 76, { 151, 152 }, { 146, 147, 148 } },
    { 16, { 91, 92, 88, 90, 89 }, { 85, 86, 87 } },
    { 0 },
};
int Cam_CityHall[] = { 181, 182, 183, 184, 0, 0 };
MN_SCENETBL Scene_CityHall = { Mdl_CityHall, Cam_CityHall };
MNANM_TBL CityHallAnime[] = {
    { MNANM_PLAY_ONCE, 0, 0, 240, 2, 0, { { MNANM_CAMERA, 0 } } },
    { MNANM_PLAY_ONCE, 0, 0, 120, 2, 0, { { MNANM_CAMERA, 1 } } },
    { MNANM_PLAY_ONCE, 0, 120, 240, 2, 0, { { MNANM_CAMERA, 1 } } },
    { MNANM_PLAY_ONCE, 0, 240, 359, 1, 0, { { MNANM_CAMERA, 1 } } },
    { MNANM_PLAY_ONCE, 0, 0, 120, 2, 0, { { MNANM_CAMERA, 2 } } },
    { MNANM_PLAY_ONCE, 0, 120, 240, 2, 0, { { MNANM_CAMERA, 2 } } },
    { MNANM_PLAY_ONCE, 0, 240, 360, 1, 0, { { MNANM_CAMERA, 2 } } },
    { MNANM_PLAY_ONCE, 0, 0, 120, 2, 0, { { MNANM_CAMERA, 3 } } },
    { MNANM_PLAY_ONCE, 0, 120, 240, 2, 0, { { MNANM_CAMERA, 3 } } },
    { MNANM_PLAY_ONCE, 0, 240, 360, 1, 0, { { MNANM_CAMERA, 3 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_MOTION | 2, 0 } } },
    { MNANM_PLAY_MUSIC, 1, 0, -1, 1, 0, { { MNANM_MOTION | 3, 0 } } },
    { MNANM_PLAY_MUSIC, 2, 0, -1, 1, 0, { { MNANM_BLEND_FAST | 2, 0 } } },
    { MNANM_PLAY_MUSIC, 1, 0, -1, 1, 0, { { MNANM_BLEND_FAST | 3, 0 } } },
    { MNANM_PLAY_ONCE, 2, 0, 30, 1, 0, { { MNANM_BLEND_FAST | 2, 1 } } },
    { MNANM_PLAY_ONCE, 1, 0, 120, 1, 0, { { MNANM_BLEND_FAST | 3, 1 } } },
    { MNANM_PLAY_ONCE, 1, 0, 56, 1, 0, { { MNANM_BLEND_FAST | 3, 2 } } },
    { MNANM_PLAY_ONCE, 1, 0, 56, 1, 0, { { MNANM_BLEND_FAST | 3, 3 } } },
    { MNANM_PLAY_ONCE, 1, 0, 56, 1, 0, { { MNANM_BLEND_FAST | 3, 4 } } },
    { MNANM_PLAY_ONCE, 5, 0, 2, 1, 0, { { MNANM_POSITION | 2, 0 } } },
    { MNANM_PLAY_ONCE, 5, 0, 30, 1, 0, { { MNANM_POSITION | 2, 1 } } },
    { MNANM_PLAY_ONCE, 5, 120, 150, 1, 0, { { MNANM_POSITION | 2, 1 } } },
    { MNANM_PLAY_ONCE, 5, 0, 30, 1, 0, { { MNANM_POSITION | 2, 2 } } },
    { MNANM_PLAY_ONCE, 5, 120, 150, 1, 0, { { MNANM_POSITION | 2, 2 } } },
    { MNANM_PLAY_ONCE, 6, 0, 2, 1, 0, { { MNANM_POSITION | 3, 0 } } },
    { MNANM_PLAY_ONCE, 6, 0, 120, 1, 0, { { MNANM_POSITION | 3, 1 } } },
    { MNANM_PLAY_ONCE, 6, 120, 240, 1, 0, { { MNANM_POSITION | 3, 1 } } },
    { MNANM_PLAY_ONCE, 6, 0, 120, 1, 0, { { MNANM_POSITION | 3, 2 } } },
    { MNANM_PLAY_ONCE, 6, 120, 240, 1, 0, { { MNANM_POSITION | 3, 2 } } },
};
int Cam_Notdef[] = { 0, 0 };
MN_MDLTBL Mdl_OptCounter[] = {
    { 51 },
    { 0 },
};
MN_SCENETBL Scene_OptCounter = { Mdl_OptCounter, Cam_Notdef };
MN_MDLTBL Mdl_RepCounter[] = {
    { 77 },
    { 1 },
    { 2 },
    { 3 },
    { 4 },
    { 5 },
    { 0 },
};
MN_SCENETBL Scene_RepCounter = { Mdl_RepCounter, Cam_Notdef };
MN_MDLTBL Mdl_StgCounterLoad[] = {
    { 17 },
    { 11 },
    { 12 },
    { 13 },
    { 14 },
    { 15 },
    { 0 },
};
MN_SCENETBL Scene_StgCounterLoad = { Mdl_StgCounterLoad, Cam_Notdef };
MN_MDLTBL Mdl_StgCounterSave[] = {
    { 84 },
    { 6 },
    { 7 },
    { 8 },
    { 9 },
    { 10 },
    { 0 },
};
MN_SCENETBL Scene_StgCounterSave = { Mdl_StgCounterSave, Cam_Notdef };
MNANM_TBL CounterAnime[] = {
    {
        MNANM_PLAY_ONCE, 0, 0, 2, 1, 0,
        {
            { MNANM_VISIBLE | 1, 0 },
            { MNANM_VISIBLE | 2, 0 },
            { MNANM_VISIBLE | 3, 0 },
            { MNANM_VISIBLE | 4, 0 },
            { MNANM_VISIBLE | 5, 0 },
        },
    },
    {
        MNANM_PLAY_ONCE, 0, 0, 2, 1, 0,
        {
            { MNANM_VISIBLE | 1, 1 },
            { MNANM_VISIBLE | 2, 0 },
            { MNANM_VISIBLE | 3, 0 },
            { MNANM_VISIBLE | 4, 0 },
            { MNANM_VISIBLE | 5, 0 },
        },
    },
    {
        MNANM_PLAY_ONCE, 0, 0, 2, 1, 0,
        {
            { MNANM_VISIBLE | 1, 1 },
            { MNANM_VISIBLE | 2, 1 },
            { MNANM_VISIBLE | 3, 0 },
            { MNANM_VISIBLE | 4, 0 },
            { MNANM_VISIBLE | 5, 0 },
        },
    },
    {
        MNANM_PLAY_ONCE, 0, 0, 2, 1, 0,
        {
            { MNANM_VISIBLE | 1, 1 },
            { MNANM_VISIBLE | 2, 1 },
            { MNANM_VISIBLE | 3, 1 },
            { MNANM_VISIBLE | 4, 0 },
            { MNANM_VISIBLE | 5, 0 },
        },
    },
    {
        MNANM_PLAY_ONCE, 0, 0, 2, 1, 0,
        {
            { MNANM_VISIBLE | 1, 1 },
            { MNANM_VISIBLE | 2, 1 },
            { MNANM_VISIBLE | 3, 1 },
            { MNANM_VISIBLE | 4, 1 },
            { MNANM_VISIBLE | 5, 0 },
        },
    },
    {
        MNANM_PLAY_ONCE, 0, 0, 2, 1, 0,
        {
            { MNANM_VISIBLE | 1, 1 },
            { MNANM_VISIBLE | 2, 1 },
            { MNANM_VISIBLE | 3, 1 },
            { MNANM_VISIBLE | 4, 1 },
            { MNANM_VISIBLE | 5, 1 },
        },
    },
};
MN_MDLTBL Mdl_JimakuBak[] = {
    { 18 },
    { 0 },
};
MN_SCENETBL Scene_JimakuBak = { Mdl_JimakuBak, Cam_Notdef };

static u_int AMusicFitTime;

static void SetDrawEnv12(sceGsDrawEnv1 *pdenv);
static void MnMoveMode_InitRoot(int movNo);
static void _myVu0Length(float *v0, float *v1);
static void _MnParMovRoot_GetPos(PRPROOT *prt, float rate, float *pos);
static void MnMoveModelPosition(void *spm, int movNo, int ttim, int ttim0);

void MNScene_ExecAnime(MN_SCENE *pshdl);

void MNSceneMusicFitTimerClear(void) {
    AMusicFitTime = 0;
}

void MNSceneMusicFitTimerFrame(void) {
    AMusicFitTime++;
}

int MNSceneGetMusicFitTimer(void) {
    return AMusicFitTime;
}

void MNScene_Init(MN_SCENE *pshdl, MN_SCENETBL *tbl, int bFocus) {
    MN_HMDL   *mdl;
    MN_MDLTBL *mtbl;
    int       *ctbl;
    int        mn;

    mdl  = pshdl->mdl;
    ctbl = tbl->pctbl;
    mtbl = tbl->pmtbl;

    memset(pshdl, 0, sizeof(*pshdl));

    pshdl->scene = PrInitializeScene(&DBufDc.draw01, NULL, (bFocus) ? FBP_VRAM_DRAW2 : -1);

    for (mn = 0; mn < PR_ARRAYSIZE(pshdl->mdl) && mtbl->fn_mdl != 0; mn++, mdl++, mtbl++) {
        int i;

        mdl->spm = PrInitializeModel(GetIntAdrsCurrent(mtbl->fn_mdl), pshdl->scene);
        mdl->ablend_rate = mdl->ablend_speed = 0.0f;
        mdl->bABlend = 0;

        for (i = 0; i < PR_ARRAYSIZE(mtbl->fn_anmm) && mtbl->fn_anmm[i] != 0; i++) {
            mdl->spa_m[i] = PrInitializeAnimation(GetIntAdrsCurrent(mtbl->fn_anmm[i]));
        }
        for (i = 0; i < PR_ARRAYSIZE(mtbl->fn_anmp) && mtbl->fn_anmp[i] != 0; i++) {
            mdl->spa_p[i] = PrInitializeAnimation(GetIntAdrsCurrent(mtbl->fn_anmp[i]));
        }

        if (mdl->spa_m[0] != NULL) {
            PrLinkAnimation(mdl->spm, mdl->spa_m[0]);
            mdl->MSpaNo = 1;
        }
        if (mdl->spa_p[0] != NULL) {
            PrLinkPositionAnimation(mdl->spm, mdl->spa_p[0]);
            mdl->PSpaNo = 1;
        }

        PrShowModel(mdl->spm, NULL);
        mdl->dspSw = 1;
    }

    pshdl->nmdl = mn;

    for (mn = 0; mn < 14 && ctbl[mn] != 0; mn++) {
        pshdl->spc[mn] = PrInitializeCamera(GetIntAdrsCurrent(ctbl[mn]));
    }

    pshdl->ncam = mn;

    if (pshdl->spc[0] != NULL) {
        PrSelectCamera(pshdl->spc[0], pshdl->scene);
        pshdl->CAniNo = 1;
    }

    PrPreprocessSceneModel(pshdl->scene);
}

void MNScene_End(MN_SCENE *pshdl) {
    MN_HMDL *mdl;
    int      mn;

    mdl = pshdl->mdl;
    for (mn = 0; mn < pshdl->nmdl; mn++, mdl++) {
        int i;

        if (mdl->spm != NULL) {
            PrCleanupModel(mdl->spm);
        }

        for (i = 0; i < PR_ARRAYSIZE(mdl->spa_m); i++) {
            if (mdl->spa_m[i] != NULL) {
                PrCleanupAnimation(mdl->spa_m[i]);
            }
        }

        for (i = 0; i < PR_ARRAYSIZE(mdl->spa_p); i++) {
            if (mdl->spa_p[i] != NULL) {
                PrCleanupAnimation(mdl->spa_p[i]);
            }
        }
    }

    for (mn = 0; mn < pshdl->ncam; mn++) {
        if (pshdl->spc[mn] != NULL) {
            PrCleanupCamera(pshdl->spc[mn]);
        }
    }

    if (pshdl->scene != NULL) {
        PrCleanupScene(pshdl->scene);
    }

    memset(pshdl, 0, sizeof(*pshdl));
}

static void SetDrawEnv12(sceGsDrawEnv1 *pdenv) {
    DRAWENV_TAG12 denvTag;

    if (pdenv == NULL) {
        return;
    }

    ((u_long*)&denvTag.giftag)[0] = SCE_GIF_SET_TAG(8, 1, 0, 0, 0, 1);
    ((u_long*)&denvTag.giftag)[1] = SCE_GIF_PACKED_AD;
    denvTag.denv1 = *pdenv;

    sceGsSyncPath(0, 0);
    FlushCache(WRITEBACK_DCACHE);

    sceGsPutDrawEnv(&denvTag.giftag);
    sceGsSyncPath(0, 0);

    denvTag.denv1.frame1addr    = SCE_GS_FRAME_2;
    denvTag.denv1.zbuf1addr     = SCE_GS_ZBUF_2;
    denvTag.denv1.xyoffset1addr = SCE_GS_XYOFFSET_2;
    denvTag.denv1.scissor1addr  = SCE_GS_SCISSOR_2;
    denvTag.denv1.test1addr     = SCE_GS_TEST_2;
    FlushCache(WRITEBACK_DCACHE);

    sceGsPutDrawEnv(&denvTag.giftag);
    sceGsSyncPath(0, 0);
}

void MNScene_Draw(MN_SCENE *pshdl) {
    if (pshdl->isDisp) {
        MNScene_ExecAnime(pshdl);

        if (pshdl->isDisp & 1) {
            PrSetSceneEnv(pshdl->scene, DrawGetDrawEnvP(DNUM_DRAW));
            PrRender(pshdl->scene);
            PrWaitRender();
        }

        if (pshdl->isDisp & 2) {
            TsMenu_CleanVram(DNUM_VRAM2);

            SetDrawEnv12(DrawGetDrawEnvP(DNUM_VRAM2));

            PrSetSceneEnv(pshdl->scene, DrawGetDrawEnvP(DNUM_VRAM2));
            PrRender(pshdl->scene);
            PrWaitRender();

            SetDrawEnv12(DrawGetDrawEnvP(DNUM_DRAW));
        }
    }
}

void MNScene_DispSw(MN_SCENE *pshdl, int sw) {
    pshdl->isDisp = sw;
}

void MNScene_SetAnimete(MN_SCENE *pshdl) {
    MN_HMDL *mdl;
    int      mn; 

    mdl = pshdl->mdl;
    for (mn = 0; mn < pshdl->nmdl; mn++, mdl++) {
        if (!mdl->dspSw) {
            continue;
        }

        PrAnimateModel(mdl->spm, pshdl->time[mdl->MAniNo]);
        PrAnimateModelPosition(mdl->spm, pshdl->time[mdl->PAniNo]);

        if (mdl->MvTblNo != 0) {
            MNANM_TBL *anm = pshdl->anime[mdl->PMovNo];
            if (anm != NULL) {
                MnMoveModelPosition(mdl->spm, mdl->MvTblNo - 1,
                                    pshdl->time[mdl->PMovNo], anm->etime);
            }
        }

        if (mdl->bABlend) {
            PrSetTransactionBlendRatio(mdl->spm, mdl->ablend_rate);
        }
    }

    PrAnimateSceneCamera(pshdl->scene, pshdl->time[pshdl->CAniNo]);
}

u_int MNScene_StartAnime(MN_SCENE *pshdl, int no, MNANM_TBL *anime) {
    MNANM_COBJ *acobj;
    int         i;
    u_int       anmBit;

    if (anime == NULL) {
        pshdl->anime[no] = NULL;
        return PR_BIT(31);
    }
    if (no < 0) {
        no = anime->aTimNo;
    }
    if (no >= 10) {
        no = 0;
    }

    pshdl->speed[no] = anime->aSpeed;
    pshdl->time[no] = anime->stime;
    pshdl->anime[no] = anime;
    
    acobj  = anime->anmCobj;
    anmBit = PR_BIT(no);

    for (i = 0; i < 6 && acobj->cflg != 0; i++, acobj++) {
        int      cflg = acobj->cflg;
        int      ano;
        MN_HMDL *mdl;

        ano = cflg & MNANM_TARGET_MASK;
        switch (cflg & MNANM_KIND_MASK) {
        case MNANM_CAMERA:
            PrSelectCamera(pshdl->spc[acobj->no], pshdl->scene);
            pshdl->CAniNo = no;
            pshdl->CSpcNo = acobj->no + 1;
            break;
        case MNANM_MOTION:
            mdl = &pshdl->mdl[ano];
            if (mdl->spm != NULL) {
                PrShowModel(mdl->spm, NULL);
                PrLinkAnimation(mdl->spm, mdl->spa_m[acobj->no]);

                mdl->MAniNo = no;
                mdl->MSpaNo = acobj->no + 1;
                mdl->dspSw  = 1;
            }
            break;
        case MNANM_BLEND_FAST:
        case MNANM_BLEND_SLOW:
            mdl = &pshdl->mdl[ano];
            if (mdl->spm != NULL) {
                if (mdl->bABlend != 0) {
                    PrResetPosture(mdl->spm);
                }

                PrSavePosture(mdl->spm);
                PrShowModel(mdl->spm, NULL);
                PrLinkAnimation(mdl->spm, mdl->spa_m[acobj->no]);

                mdl->bABlend = 1;
                mdl->ablend_rate = 0.0f;

                if ((acobj->cflg & MNANM_KIND_MASK) == MNANM_BLEND_SLOW) {
                    mdl->ablend_speed = 0.033333335f;
                } else {
                    mdl->ablend_speed = 0.1f;
                }

                mdl->MAniNo = no;
                mdl->MSpaNo = acobj->no + 1;
                mdl->dspSw  = 1;
            }
            break;
        case MNANM_POSITION:
            mdl = &pshdl->mdl[ano];
            if (mdl->spm != NULL) {
                PrShowModel(mdl->spm, NULL);
                PrLinkPositionAnimation(mdl->spm, mdl->spa_p[acobj->no]);

                mdl->PAniNo = no;
                mdl->PSpaNo = acobj->no + 1;
                mdl->dspSw  = 1;
            }
            break;
        case MNANM_MOVE_MODE:
            mdl = &pshdl->mdl[ano];
            if (mdl->spm != NULL) {
                MnMoveMode_InitRoot(acobj->no - 1);
                PrShowModel(mdl->spm, NULL);
                PrUnlinkPositionAnimation(mdl->spm);

                mdl->PMovNo  = no;
                mdl->MvTblNo = acobj->no;
                mdl->dspSw   = 1;
                mdl->PSpaNo  = 0;
            }
            break;
        case MNANM_VISIBLE:
            mdl = &pshdl->mdl[ano];
            if (mdl->spm != NULL) {
                if (acobj->no != 0) {
                    mdl->dspSw = 1;
                    PrShowModel(mdl->spm, NULL);
                } else {
                    mdl->dspSw = 0;
                    PrHideModel(mdl->spm);
                }
            }
            break;
        }
    }

    MNScene_SetAnimete(pshdl);
    return anmBit;
}

void MNScene_ContinueAnime(MN_SCENE *pshdl, int no, MNANM_TBL *anime) {
    if (anime == NULL) {
        if (no < 11u) {
            pshdl->cntani[no] = NULL;
        }
    } else {
        if (no < 0) {
            no = anime->aTimNo;
        }
        if (no > 9) {
            no = 0;
        }

        pshdl->speed[no]  = anime->aSpeed;
        pshdl->cntani[no] = anime;
    }
}

void MNScene_StopAnime(MN_SCENE *pshdl,int no) {
    MNScene_StartAnime(pshdl, no, NULL);
    MNScene_ContinueAnime(pshdl, no, NULL);
}

void MNScene_ExecAnime(MN_SCENE *pshdl) {
    MN_HMDL   *mdl;
    int        mn;
    int        i;
    int        time;

    for (i = 0; i < 10; i++) {
        MNANM_TBL *panm = pshdl->anime[i];

        if (panm == NULL) {
            continue;
        }

        if (panm->kind == MNANM_PLAY_MUSIC) {
            time = AMusicFitTime * pshdl->speed[i];
            if (panm->etime > 0) {
                time = (time % panm->etime);
            }
        } else {
            time = pshdl->time[i] + pshdl->speed[i];

            if (panm->etime >= 0 && panm->etime < time) {
                if (panm->kind == MNANM_PLAY_LOOP) {
                    time = panm->stime;
                } else {
                    time = panm->etime;
                }
            }

            if (time == panm->etime) {
                if (pshdl->cntani[i] != NULL) {
                    MNScene_StartAnime(pshdl, i, pshdl->cntani[i]);
                    MNScene_ContinueAnime(pshdl, i, NULL);
                }
            }
        }

        pshdl->time[i] = time;
    }

    mdl = pshdl->mdl;
    for (mn = 0; mn < pshdl->nmdl; mn++, mdl++) {
        if (!mdl->bABlend) {
            continue;
        }

        if (mdl->ablend_rate >= 1.0f) {
            mdl->ablend_rate = mdl->ablend_speed = 0.0f;
            PrResetPosture(mdl->spm);
            mdl->bABlend = 0;
        } else {
            mdl->ablend_rate += mdl->ablend_speed;
            if (mdl->ablend_rate > 1.0f) {
                mdl->ablend_rate = 1.0f;
            }
        }
    }

    MNScene_SetAnimete(pshdl);
}

void MNScene_CopyState(MN_SCENE *pdhdl, MN_SCENE *pshdl) {
    MN_HMDL *mdl;
    MN_HMDL *smdl;
    int      i;

    memcpy(pdhdl->time, pshdl->time, sizeof(pdhdl->time));
    memcpy(pdhdl->anime, pshdl->anime, sizeof(pdhdl->anime));
    memcpy(pdhdl->cntani, pshdl->cntani, sizeof(pdhdl->cntani));
    memcpy(pdhdl->speed, pshdl->speed, sizeof(pdhdl->speed));

    pdhdl->CSpcNo = pshdl->CSpcNo;
    pdhdl->CAniNo = pshdl->CAniNo;

    if (pdhdl->CSpcNo != 0) {
        if (pdhdl->scene != NULL) {
            PrSelectCamera(pdhdl->spc[pdhdl->CSpcNo - 1], pdhdl->scene);
        }
    }

    mdl  = pdhdl->mdl;
    smdl = pshdl->mdl;

    pdhdl->nmdl = pshdl->nmdl;

    for (i = 0; i < pshdl->nmdl; i++, mdl++, smdl++) {
        mdl->dspSw = smdl->dspSw;
        mdl->ablend_rate = mdl->ablend_speed = 0.0f;

        mdl->MAniNo  = smdl->MAniNo;
        mdl->PAniNo  = smdl->PAniNo;

        mdl->PMovNo  = smdl->PMovNo;
        mdl->MvTblNo = smdl->MvTblNo;

        mdl->MSpaNo  = smdl->MSpaNo;
        mdl->PSpaNo  = smdl->PSpaNo;

        mdl->bABlend = 0;

        if (mdl->spm != NULL) {
            if (mdl->dspSw != 0) {
                PrShowModel(mdl->spm, NULL);
            } else {
                PrHideModel(mdl->spm);
            }
        }

        if (mdl->MSpaNo != 0) {
            if (mdl->spm != NULL) {
                PrLinkAnimation(mdl->spm, mdl->spa_m[mdl->MSpaNo - 1]);
            }
        }

        if (mdl->PSpaNo != 0) {
            if (mdl->spm != NULL) {
                PrLinkPositionAnimation(mdl->spm, mdl->spa_p[mdl->PSpaNo - 1]);
            }
        }

        if (mdl->MvTblNo != 0) {
            PrUnlinkPositionAnimation(mdl->spm);
            MnMoveMode_InitRoot(mdl->MvTblNo - 1);
        }
    }
}

void MNScene_CopyStateMdl(MN_SCENE *pdhdl, MN_SCENE *pshdl) {
    MN_HMDL *mdl;
    MN_HMDL *smdl;
    int      i;

    memcpy(pdhdl->time, pshdl->time, sizeof(pdhdl->time));
    memcpy(pdhdl->anime, pshdl->anime, sizeof(pdhdl->anime));
    memcpy(pdhdl->cntani, pshdl->cntani, sizeof(pdhdl->cntani));
    memcpy(pdhdl->speed, pshdl->speed, sizeof(pdhdl->speed));

    mdl  = pdhdl->mdl;
    smdl = pshdl->mdl;
    for (i = 0; i < pshdl->nmdl; i++, mdl++, smdl++) {
        mdl->dspSw = smdl->dspSw;
        mdl->ablend_rate = mdl->ablend_speed = 0.0f;

        mdl->MAniNo  = smdl->MAniNo;
        mdl->PAniNo  = smdl->PAniNo;

        mdl->PMovNo  = smdl->PMovNo;
        mdl->MvTblNo = smdl->MvTblNo;

        mdl->MSpaNo  = smdl->MSpaNo;
        mdl->PSpaNo  = smdl->PSpaNo;

        mdl->bABlend = 0;

        if (mdl->spm != NULL) {
            if (mdl->dspSw != 0) {
                PrShowModel(mdl->spm, NULL);
            } else {
                PrHideModel(mdl->spm);
            }
        }

        if (mdl->MSpaNo != 0) {
            if (mdl->spm != NULL) {
                PrLinkAnimation(mdl->spm, mdl->spa_m[mdl->MSpaNo - 1]);
            }
        }

        if (mdl->PSpaNo != 0) {
            if (mdl->spm != NULL) {
                PrLinkPositionAnimation(mdl->spm, mdl->spa_p[mdl->PSpaNo - 1]);
            }
        }

        if (mdl->MvTblNo != 0) {
            PrUnlinkPositionAnimation(mdl->spm);
            MnMoveMode_InitRoot(mdl->MvTblNo - 1);
        }
    }
}

void MNScene_SetAnimeSpeed(MN_SCENE *pshdl, int nAnime, int speed) {
    if (nAnime >= 10) {
        return;
    }

    pshdl->speed[nAnime] = speed;
}

void MNScene_SetAnimeEnd(MN_SCENE *pshdl) {
    int i;

    for (i = 0; i < 10; i++) {
        MNANM_TBL *panm = pshdl->anime[i];

        if (panm != NULL) {
            if (panm->kind != MNANM_PLAY_LOOP && panm->kind != MNANM_PLAY_MUSIC) {
                pshdl->time[i] = panm->etime;
            }
        }
    }

    MNScene_SetAnimete(pshdl);
}

void MNScene_SetAnimeBankEnd(MN_SCENE *pshdl, u_int bnk) {
    MNANM_TBL *panm;

    if ((int)bnk >= 0) {
        panm = pshdl->anime[bnk];

        if (panm != NULL) {
            if (panm->kind != MNANM_PLAY_LOOP && panm->kind != MNANM_PLAY_MUSIC) {
                pshdl->time[bnk] = panm->etime;
            }
        }
    } else {
        int i;

        for (i = 0; i < 10; i++, bnk >>= 1) {
            if (!(bnk & 1)) {
                continue;
            }

            panm = pshdl->anime[i];
            if (panm != NULL) {
                if (panm->kind != MNANM_PLAY_LOOP && panm->kind != MNANM_PLAY_MUSIC) {
                    pshdl->time[i] = panm->etime;
                }
            }
        }
    }

    MNScene_SetAnimete(pshdl);
}

int MNScene_isAnime(MN_SCENE *pshdl, int ltim) {
    int i;

    for (i = 0; i < 10; i++) {
        MNANM_TBL *panm = pshdl->anime[i];

        if (panm != NULL) {
            if (panm->kind != MNANM_PLAY_LOOP && panm->kind != MNANM_PLAY_MUSIC) {
                if (pshdl->time[i] < (panm->etime - ltim)) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

int MNScene_isAnimeBank(MN_SCENE *pshdl, int ltim, u_int bnk) {
    MNANM_TBL *panm;

    if ((int)bnk >= 0) {
        panm = pshdl->anime[bnk];
        if (panm != NULL) {
            if (panm->kind != MNANM_PLAY_LOOP && panm->kind != MNANM_PLAY_MUSIC) {
                if (pshdl->time[bnk] < (panm->etime - ltim)) {
                    return 1;
                }
            }
        }
    } else {
        int i;

        for (i = 0; i < 10; i++, bnk >>= 1) {
            if (!(bnk & 1)) {
                continue;
            }
            panm = pshdl->anime[i];
            if (panm != NULL) {
                if (panm->kind != MNANM_PLAY_LOOP && panm->kind != MNANM_PLAY_MUSIC) {
                    if (pshdl->time[i] < (panm->etime - ltim)) {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

int MNScene_isSeniAnime(MN_SCENE *pshdl) {
    int      mn;
    MN_HMDL *mdl = pshdl->mdl;

    for (mn = 0; mn < pshdl->nmdl; mn++, mdl++) {
        if (mdl->bABlend) {
            return 1;
        }
    }

    return 0;
}

int MNScene_ModelDispSw(MN_SCENE *pshdl, int nmdl, int bsw) {
    int      ret;
    MN_HMDL *mdl;

    if (pshdl->nmdl <= nmdl) {
        return 0;
    }
    
    mdl = &pshdl->mdl[nmdl];
    if (mdl->spm == NULL) {
        return 0;
    }

    ret = mdl->dspSw;
    mdl->dspSw = bsw;

    if (bsw != 0) {
        PrShowModel(mdl->spm, NULL);
        return ret;
    } else {
        PrHideModel(mdl->spm);
        return ret;
    }
}

/*
 * Calculates the length of a vector:
 *  - v0 -> Output vector
 *  - v1 -> Input vector
 */
static void _myVu0Length(float *v0, float *v1) {
    asm(
        "lqc2     $vf04, 0x0(%1)       \n\t" /* Load v1(vf04) */
                                             /* vf04 = v1     */

        "vmul.xyz $vf05, $vf04, $vf04  \n\t" /* vf05.xyz = (vf04.xyz)^2 */
        "vaddy.x  $vf05, $vf05, $vf05  \n\t" /* vf05.x  += vf05.y       */
        "vaddz.x  $vf05, $vf05, $vf05  \n\t" /* vf05.x  += vf05.z       */

        "vsqrt    Q,     $vf05x        \n\t" /* sqrt(vf05.x)  */
        "vwaitq                        \n\t" /* wait for sqrt */

        "vaddq.x  $vf05, $vf00, Q      \n\t" /* vf05 = Q   */
        "qmfc2    $8,    $vf05         \n\t" /* $t0 = vf05 */

        "sw       $8,    0x0(%0)       \n\t" /* save v0  */
                                             /* v0 = $t0 */
    : : "r"(v0), "r"(v1) : "$8", "memory");
}

static void MnMoveMode_InitRoot(int movNo) {
    int      i;

    PRPROOT *prt;
    PRPOS   *ppos;
    float    flen;

    prt  = &PRP_RootTbl[movNo & ~128];
    ppos = prt->ppos;
    flen = 0.0f;

    for (i = 1; i < prt->npos; i++) {
        float flen0;
        sceVu0FVECTOR v0;
        sceVu0FVECTOR v1;

        v0[0] = ppos[i - 1].x;
        v0[1] = ppos[i - 1].y;
        v0[2] = ppos[i - 1].z;
        v0[3] = 0.0f;

        v1[0] = ppos[i].x;
        v1[1] = ppos[i].y;
        v1[2] = ppos[i].z;
        v1[3] = 0.0f;

        sceVu0SubVector(v0, v1, v0);
        _myVu0Length(&flen0, v0);
        flen += flen0;

        ppos[i].dist = flen;
        ppos[i].rly = atan2f(v0[0], v0[2]);
    }

    ppos->dist = 0.0f;

    if (prt->npos >= 2) {
        ppos->rly = ppos[1].rly;
    } else {
        ppos->rly = 0.0f;
    }

    if (flen == 0.0f) {
        flen = 1.0f;
    }

    flen = (1.0f / flen);
    for (i = 1; i < prt->npos; i++) {
        ppos[i].dist *= flen;
    }
}

static void _MnParMovRoot_GetPos(PRPROOT *prt, float rate, float *pos) {
    PRPOS *ppos;
    int    i;

    ppos = prt->ppos;
    i    = 0;

    if (rate < 1.0f) {
        for (i = 1; i < prt->npos; i++) {
            if (rate < ppos[i].dist) break;
        }
    }

    if (rate >= 1.0f || i > prt->npos) {
        /* Past the end of the route: stay on its last point. */
        pos[0] = ppos[prt->npos - 1].x;
        pos[1] = ppos[prt->npos - 1].y;
        pos[2] = ppos[prt->npos - 1].z;
        pos[3] = ppos[prt->npos - 1].rly;
    } else {
        PRPOS *psrc;
        PRPOS *pdst;

        pdst = &ppos[i];
        psrc = &ppos[i - 1];

        if (pdst->dist == psrc->dist) {
            rate = 0.0f;
        } else {
            rate = (rate - psrc->dist) / (pdst->dist - psrc->dist);
        }

        pos[0] = ((pdst->x - psrc->x) * rate) + psrc->x;
        pos[1] = ((pdst->y - psrc->y) * rate) + psrc->y;
        pos[2] = ((pdst->z - psrc->z) * rate) + psrc->z;
        pos[3] = pdst->rly;
    }
}

static void MnMoveModelPosition(void *spm, int movNo, int ttim, int ttim0) {
    float rate;
    float WRATE;
    float fry;

    WRATE = 0.17f;

    if (ttim >= ttim0) {
        fry = 1.0f;
    } else {
        fry = (float)ttim / (float)ttim0;
    }

    if (spm != NULL) {
        sceVu0FVECTOR cpos;
        sceVu0FMATRIX mt;

        /* Ease along the route; bit 7 of movNo runs it backwards. */
        rate = sinf(fry * 0.5f * (float)M_PI);

        if (movNo & 0x80) {
            rate = 1.0f - rate;
        }

        _MnParMovRoot_GetPos(&PRP_RootTbl[movNo & ~0x80], rate, cpos);

        /* Face along the route while moving; at either end, face the default way. */
        if (rate == 0.0f || rate == 1.0f) {
            fry = 0.0f;
        } else {
            fry = cpos[3];

            if (movNo & 0x80) {
                fry += (float)M_PI;
            }
            if (fry > (float)M_PI) {
                fry -= ((float)M_PI * 2.0f);
            }
        }

        sceVu0UnitMatrix(mt);
        sceVu0RotMatrixY(mt, mt, fry);

        mt[3][0] = cpos[0] * WRATE;
        mt[3][1] = cpos[1] * WRATE;
        mt[3][2] = cpos[2] * WRATE;
        mt[3][3] = WRATE;

        PrShowModel(spm, &mt);
    }
}
