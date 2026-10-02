#ifndef NALIB_NAPACKET_H
#define NALIB_NAPACKET_H

#include <eetypes.h>
#include <eestruct.h>
#include <libgifpk.h>

/*
 * The member functions are never inlined by the compiler,
 * so every user emits its own weak copy of the ones it needs
 * (in reverse declaration order, after the rest of the file).
 */
class NaGifPacket {
public:
    void Init(u_long128 *base) {
        sceGifPkInit(&m_packet, base);
    }

    void End(u_int opt1, u_int opt2, u_int flag) {
        sceGifPkEnd(&m_packet, opt1, opt2, flag);
    }

    void OpenGifTag(u_long128 tag) {
        sceGifPkOpenGifTag(&m_packet, tag);
    }

    void CloseGifTag() {
        sceGifPkCloseGifTag(&m_packet);
    }

    void AddGsAD(u_int addr, u_long data) {
        sceGifPkAddGsAD(&m_packet, addr, data);
    }

    void OpenGifTag(const sceGifTag& tag) {
        OpenGifTag(*(u_long128*)&tag);
    }

    void AddAlpha1(int a, int b, int c, int d, u_char fix) {
        AddGsAD(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(a, b, c, d, fix));
    }

    void AddClamp1(int wms, int wmt, u_int minu, u_int maxu, u_int minv, u_int maxv) {
        AddGsAD(SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv));
    }

    void AddFrame1(u_int fbp, u_int fbw, int psm, u_int fbmask) {
        AddGsAD(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(fbp, fbw, psm, fbmask));
    }

    void AddPrim(int prim, int iip, u_int tme, u_int fge, u_int abe, u_int aa1, int fst, int ctxt, u_int fix) {
        AddGsAD(SCE_GS_PRIM, SCE_GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix));
    }

    void AddScissor1(u_int scax0, u_int scax1, u_int scay0, u_int scay1) {
        AddGsAD(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(scax0, scax1, scay0, scay1));
    }

    void AddTest1(u_int ate, int atst, u_char aref, int afail, u_int date, int datm, u_int zte, int ztst) {
        AddGsAD(SCE_GS_TEST_1, SCE_GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst));
    }

    void AddTex0_1(u_int tbp, u_int tbw, int psm, u_int tw, u_int th, u_int tcc, u_int tfx,
                   u_int cbp, int cpsm, int csm, u_int csa, int cld) {
        AddGsAD(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld));
    }

    void AddTex1_1(int lcm, u_int mxl, int mmag, int mmin, int mtba, u_int l, u_int k) {
        AddGsAD(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k));
    }

    void AddTexflush() {
        AddGsAD(SCE_GS_TEXFLUSH, 0);
    }

    void AddXyoffset1(u_int ofx, u_int ofy) {
        AddGsAD(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET(ofx, ofy));
    }

private:
    sceGifPacket m_packet;
};

#endif /* NALIB_NAPACKET_H */
