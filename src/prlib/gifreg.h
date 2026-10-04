#ifndef PRLIB_GIFREG_H
#define PRLIB_GIFREG_H

#include "common.h"

#include <eetypes.h>
#include <eestruct.h>
#include <libdma.h>

struct PrGifPackedAd {
    u_long data;
    u_long addr;
};

class PrDmaStripForSetGifRegister {
public:
    PrDmaStripForSetGifRegister() {
        m_strip_len = 0;
        m_frozen = false;
    }
    ~PrDmaStripForSetGifRegister() {
        /* Empty */
    }

    void Initialize() {
        m_strip_len = 0;
        m_frozen = false;
    }

    void Append(u_int addr, const u_long& data);
    void Freeze(u_char id, const void *addr);

public:
    sceDmaTag m_tag;
    sceGifTag m_giftag;
    PrGifPackedAd m_strip[16];
    int m_strip_len;
    bool m_frozen;
    PR_PADDING(unk128, 0x8);
};

/* Register setups sent between the render passes (built in PrInitializeDmaStripGifRegister). */
enum PrSetGifRegisterMode {
    eGifRegisterMode_SceneModel = 0,  /* normal models: depth test GEQUAL with Z writes */
    eGifRegisterMode_NoZWrite = 1,    /* Z writes masked on both contexts (translucent entries, end of frame) */
    eGifRegisterMode_DebugQuad = 2,   /* a four-colour test quad, never selected */
    eGifRegisterMode_ScreenModel = 3, /* clears Z to 0 with a full-screen Z-only sprite before screen models */
    eGifRegisterMode_Background = 4,  /* background layer: depth test ALWAYS, Z writes masked */
    eGifRegisterMode_PreScene = 5,    /* pre-scene layer: depth only (alpha test NEVER, AFAIL=ZB_ONLY), test ALWAYS */
};

void PrInitializeDmaStripGifRegister(sceGsZbuf zbuf);
PrDmaStripForSetGifRegister* PrGetDmaStripGifRegister(PrSetGifRegisterMode mode);

#endif /* PRLIB_GIFREG_H */
