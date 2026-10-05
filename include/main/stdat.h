#ifndef STDAT_H
#define STDAT_H

#include "common.h"

#include "main/etc.h"
#include "main/subt.h"
#include "main/cdctrl.h"
#include "main/drawctrl.h"

struct SCR_MAIN;

/* Stage overlays are loaded here (overlay_loadaddr); the STDAT_DAT entries
 * point at data inside the overlay. */
#define STAGE_OVERLAY_BASE 0x01ca0000
#define STAGE_OVERLAY_PTR(type, offset) ((type *)(STAGE_OVERLAY_BASE + (offset)))

typedef struct { // 0xd0
    /* 0x00 */ PLAY_STEP play_step;
    /* 0x04 */ char *ply_name;
    /* 0x08 */ float tempo;
    /* 0x0c */ int stage;
    /* 0x10 */ EVENTREC *ev_pp;
    /* 0x14 */ struct SCR_MAIN *scr_pp;
    /* 0x18 */ JIMAKU_STR *jimaku_str_pp;
    /* 0x1c */ FILE_STR intfile;
    /* 0x48 */ FILE_STR sndfile[3];
    /* 0xcc */ TAPLVL_STR *taplvl_str_pp;
} STDAT_DAT;

/* Index into stdat_rec (global_data.play_stageL, PrSetStage). */
typedef enum {
    STDAT_STAGE_0 = 0, /* stages 0-8 follow in order */
    STDAT_STAGE_ENDING = 9, /* "STAGE 9": the ending cutscene (XTR) */
    STDAT_STAGE_BONUS = 10,
    STDAT_STAGE_VS1 = 11, /* VS 1-8 reuse the overlays of stages 1-8 */
    STDAT_STAGE_VS8 = 18,
    STDAT_STAGE_TITLE = 19,
    STDAT_STAGE_MAX = 20
} STDAT_STAGE;

typedef struct { // 0x38
    /* 0x00 */ FILE_STR ovlfile;
    /* 0x2c */ int stdat_dat_num;
    /* 0x30 */ STDAT_DAT *stdat_dat_pp;
    /* 0x34 */ char *strec_name;
} STDAT_REC;

extern FILE_STR file_str_logo_file;
extern FILE_STR file_str_menu_file;
extern FILE_STR file_str_extra_file[10];

extern STDAT_REC stdat_rec[STDAT_STAGE_MAX];

void stDatFirstFileSearch(void);

#endif /* STDAT_H */
