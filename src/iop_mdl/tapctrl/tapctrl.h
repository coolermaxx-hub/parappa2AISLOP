#ifndef TAPCTRL_H
#define TAPCTRL_H

#include "common.h"

#include <sdmacro.h>

/* TapCtrl commands */
#define TAPCT_NONE (0)

/* Commands with this bit set send a reply back to the EE. */
#define TAPCT_REPLY           (0x8000)

#define TAPCT_INIT            (0x0000) /* TapInit(allocsize)                */
#define TAPCT_CHANCLOSE       (0x0050) /* TapChanClose(ch)                  */
#define TAPCT_QUIT            (0x0060) /* TapQuit()                         */
#define TAPCT_SETEFFECTVOL    (0x0080) /* TapSetEffectVolume(vol)           */
#define TAPCT_SETMASTERVOL    (0x0090) /* TapSetMasterVolume(vol)           */
#define TAPCT_SETVOLUME       (0x00a0) /* TapSetVolume(vol)                 */
#define TAPCT_SETEFFECTMODE   (0x00b0) /* TapSetEffectMode(mode, depth)     */
#define TAPCT_SDINIT          (0x00c0) /* TapSdInit()                       */
#define TAPCT_TAPREQ          (0x00d0) /* TapTapReq(chan, id, stdat)        */
#define TAPCT_TAPSTOP         (0x00e0) /* TapTapStop(id)                    */
#define TAPCT_TAPVOLUME       (0x00f0) /* TapTapVolume(id, vol)             */
#define TAPCT_ALLPAUSE        (0x0100) /* TapAllPause()                     */
#define TAPCT_ALLCONTINUE     (0x0110) /* TapAllContinue()                  */
#define TAPCT_TRANSCLEAR      (0x0120) /* TapTransClear()                   */
#define TAPCT_TAPVOLUMECHANGE (0x0130) /* TapTapVolumeChange(id, vol)       */
#define TAPCT_ALLOCSPU        (0x8010) /* TapAllocSpu(chan, spu_adr)        */
#define TAPCT_ALLOCIOP        (0x8020) /* TapAllocIop(chan, iop_size)       */
#define TAPCT_BDSPUTRANS      (0x8030) /* TapBdSpuTrans(chan, ee_snd, size) */
#define TAPCT_HDIOPTRANS      (0x8040) /* TapHdIopTrans(chan, ee_hd, size)  */
#define TAPCT_TRANSCHECK      (0x8070) /* TapTransCheck()                   */

/* TapCtrl module ID */
#define TAPCT_DEV             (0x8001)

#endif /* TAPCTRL_H */
