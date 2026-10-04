#include "gifreg.h"

#include "dma.h"

#include "gsstate.h"

static bool gifRegisterModeInitialized = false;
static PrDmaStripForSetGifRegister setGifRegisterMode[6];

void PrDmaStripForSetGifRegister::Append(u_int addr, const u_long& data) {
    m_strip[m_strip_len].addr = addr;
    m_strip[m_strip_len].data = data;
    m_strip_len++;
}

void PrDmaStripForSetGifRegister::Freeze(u_char id, const void *addr) {
    int len = m_strip_len;

    m_frozen = true;

    m_tag.qwc = m_strip_len + 1;
    m_tag.mark = 0;
    m_tag.id = id;
    m_tag.next = (sceDmaTag*)addr;
    m_tag.p[0] = SCE_VIF1_SET_FLUSH(0);
    m_tag.p[1] = SCE_VIF1_SET_DIRECT(m_strip_len + 1, 0);

    m_giftag.NLOOP = len;
    m_giftag.EOP = true;
    m_giftag.pad16 = 0;
    m_giftag.id = 0;
    m_giftag.PRE = false;
    m_giftag.PRIM = 0;
    m_giftag.FLG = SCE_GIF_PACKED;
    m_giftag.NREG = 1;
    m_giftag.REGS0 = SCE_GIF_PACKED_AD;
}

void PrInitializeDmaStripGifRegister(sceGsZbuf zbuf) {
    if (gifRegisterModeInitialized) {
        return;
    }

    for (int i = 0; i < PR_ARRAYSIZEU(setGifRegisterMode); i++) {
        PrDmaStripForSetGifRegister& strip = setGifRegisterMode[i];
        strip.Initialize();

        switch (i) {
        case eGifRegisterMode_Background:
            zbuf.ZMSK = 1;

            strip.Append(SCE_GS_TEST_1, PR_TEST_ALPHA_NONZERO(SCE_GS_ZALWAYS));
            strip.Append(SCE_GS_ALPHA_1, PR_ALPHA_BLEND);
            strip.Append(SCE_GS_TEX1_1, PR_TEX1_BILINEAR_LOD0);
            strip.Append(SCE_GS_ZBUF_1, GS_REG_WORD(zbuf));
            strip.Append(SCE_GS_FBA_1, SCE_GS_SET_FBA_1(0));
            break;
        case eGifRegisterMode_SceneModel:
            zbuf.ZMSK = 0;

            strip.Append(SCE_GS_TEST_1, PR_TEST_ALPHA_NONZERO(SCE_GS_ZGEQUAL));
            strip.Append(SCE_GS_ALPHA_1, PR_ALPHA_BLEND);
            strip.Append(SCE_GS_TEX1_1, PR_TEX1_BILINEAR_LOD0);
            strip.Append(SCE_GS_ZBUF_1, GS_REG_WORD(zbuf));
            strip.Append(SCE_GS_FBA_1, SCE_GS_SET_FBA_1(0));
            break;
        case eGifRegisterMode_NoZWrite:
            zbuf.ZMSK = 1;

            strip.Append(SCE_GS_ZBUF_1, GS_REG_WORD(zbuf));
            strip.Append(SCE_GS_ZBUF_2, GS_REG_WORD(zbuf));
            strip.Append(SCE_GS_ALPHA_1, PR_ALPHA_BLEND);
            strip.Append(SCE_GS_ALPHA_2, PR_ALPHA_BLEND);
            break;
        case eGifRegisterMode_DebugQuad:
            strip.Append(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, /*IIP*/1, /*TME*/0, /*FGE*/0, /*ABE*/0,
                                                      /*AA1*/0, /*FST*/1, SCE_GS_PRIM_CTXT1, /*FIX*/0));

            strip.Append(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 0, 0, 128, 0x00000001));
            strip.Append(SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(270), GS_Y_COORD(62), -1));

            strip.Append(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 0, 128, 0x00000001));
            strip.Append(SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(370), GS_Y_COORD(62), -1));

            strip.Append(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 0, 255, 128, 0x00000001));
            strip.Append(SCE_GS_XYZ2, GS_X_COORD(270) | (GS_Y_COORD(162) << 16));

            strip.Append(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(0, 255, 255, 128, 0x00000001));
            strip.Append(SCE_GS_XYZ2, GS_X_COORD(370) | (GS_Y_COORD(162) << 16));
            break;
        case eGifRegisterMode_ScreenModel:
            zbuf.ZMSK = 0;

            strip.Append(SCE_GS_ZBUF_1, GS_REG_WORD(zbuf));
            strip.Append(SCE_GS_FBA_1, SCE_GS_SET_FBA_1(0));
            /* Clear the depth buffer to Z = 0 with a full-field sprite before the screen models. */
            strip.Append(SCE_GS_TEST_1, PR_TEST_Z_ONLY);
            strip.Append(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, /*IIP*/0, /*TME*/0, /*FGE*/0, /*ABE*/0,
                                                      /*AA1*/0, /*FST*/0, SCE_GS_PRIM_CTXT1, /*FIX*/0));
            strip.Append(SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(0), GS_Y_COORD(0), 0));
            strip.Append(SCE_GS_XYZ2, SCE_GS_SET_XYZ2(GS_X_COORD(SCREEN_WIDTH), GS_Y_COORD(SCREEN_FIELD_HEIGHT), 0));
            strip.Append(SCE_GS_TEST_1, PR_TEST_ALPHA_NONZERO(SCE_GS_ZGEQUAL));
            strip.Append(SCE_GS_ALPHA_1, PR_ALPHA_BLEND);
            strip.Append(SCE_GS_TEX1_1, PR_TEX1_BILINEAR_LOD0);
            break;
        case eGifRegisterMode_PreScene:
            zbuf.ZMSK = 0;

            /* Pre-scene models only lay down depth. */
            strip.Append(SCE_GS_TEST_1, PR_TEST_Z_ONLY);
            strip.Append(SCE_GS_ZBUF_1, GS_REG_WORD(zbuf));
            strip.Append(SCE_GS_FBA_1, SCE_GS_SET_FBA_1(0));
            break;
        }

        strip.Freeze(PR_DMA_TAG_RET, NULL);
    }

    gifRegisterModeInitialized = true;
}

PrDmaStripForSetGifRegister* PrGetDmaStripGifRegister(PrSetGifRegisterMode mode) {
    return &setGifRegisterMode[mode];
}

