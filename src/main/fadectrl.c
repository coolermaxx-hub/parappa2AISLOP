#include "main/fadectrl.h"

#include "os/mtc.h"

#include "main/effect.h"

#include <eeregs.h>

static tGS_BGCOLOR bgcolor_tmp[2] = {{}, {255, 255, 255, 0, 0}};

static FMODE_CTRL_STR fmode_ctrl_str;

void FadeCtrlMain(void *x) {
    int tmp_time;
    FADE_MAKE_STR fade_make_str;

    fade_make_str.r = bgcolor_tmp[fmode_ctrl_str.fmode & FADE_COLOR_MASK].R;
    fade_make_str.g = bgcolor_tmp[fmode_ctrl_str.fmode & FADE_COLOR_MASK].G;
    fade_make_str.b = bgcolor_tmp[fmode_ctrl_str.fmode & FADE_COLOR_MASK].B;

    if (fmode_ctrl_str.current_time < fmode_ctrl_str.max_time) {
        while (fmode_ctrl_str.current_time < fmode_ctrl_str.max_time) {
            tmp_time = fmode_ctrl_str.current_time;

            if ((fmode_ctrl_str.fmode & FADE_OUT) == 0) {
                tmp_time = fmode_ctrl_str.max_time - fmode_ctrl_str.current_time;
            }

            /* 128 is opaque: a fade in starts there, a fade out stops one step
             * short of it, and nothing is drawn once the task exits. */
            fade_make_str.alp = (tmp_time * 128) / fmode_ctrl_str.max_time;
            /* GIF packet priority 100 is above every other user's (the highest elsewhere
             * is 15), so CmnGifFlush draws the fade over all other queued packets. */
            CG_FadeDisp(&fade_make_str, 100, NULL);

            fmode_ctrl_str.current_time++;
            MtcWait(1);
        }
    }

    MtcExit();
}

void FadeCtrlReq(FADE_MODE fmode, int time) {
    fmode_ctrl_str.current_time = 0;
    fmode_ctrl_str.fmode        = fmode;
    fmode_ctrl_str.max_time     = time;

    MtcExec(FadeCtrlMain, MTC_TASK_FACECTRL);
}
