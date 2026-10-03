#ifndef NALIB_NAPACKET_H
#define NALIB_NAPACKET_H

#include <eetypes.h>
#include <eestruct.h>
#include <libgifpk.h>
#include <string.h>

/*
 * The compiler never inlines these member functions, so every
 * file that uses one emits its own weak copy (in reverse order
 * of definition, after the rest of the file's code).
 */
class NaGifPacketWrapper : public sceGifPacket {
public:
    void Init(u_long128 *base) {
        sceGifPkInit(this, base);
    }

    void End(u_int opt1, u_int opt2, u_int flag) {
        sceGifPkEnd(this, opt1, opt2, flag);
    }

    void OpenGifTag(u_long128 tag) {
        sceGifPkOpenGifTag(this, tag);
    }

    void OpenGifTag(const sceGifTag& tag) {
        u_long128 encoded;
        memcpy(&encoded, &tag, sizeof(encoded));
        OpenGifTag(encoded);
    }

    void CloseGifTag() {
        sceGifPkCloseGifTag(this);
    }

    template <typename Register>
    static u_long EncodeRegister(const Register& value) {
        typedef char RegisterMustBe64Bits[sizeof(Register) == sizeof(u_long) ? 1 : -1];
        (void)sizeof(RegisterMustBe64Bits);
        u_long encoded;
        memcpy(&encoded, &value, sizeof(encoded));
        return encoded;
    }

    template <typename Register>
    void AddGsRegister(u_int address, const Register& value) {
        // Preserve all bits of the SDK register without scalar type punning.
        AddGsAD(address, EncodeRegister(value));
    }

    void AddGsAD(u_int addr, u_long data) {
        sceGifPkAddGsAD(this, addr, data);
    }
};

class NaGifPacket : public NaGifPacketWrapper {
public:
    void OpenGifTag(const sceGifTag& tag) {
        NaGifPacketWrapper::OpenGifTag(tag);
    }
    void OpenGifTag(u_long128 tag) {
        NaGifPacketWrapper::OpenGifTag(tag);
    }

    void AddGifPackedAD_ALPHA_1(int a, int b, int c, int d, u_char fix) {
        AddGsAD(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(a, b, c, d, fix));
    }

    void AddGifPackedAD_CLAMP_1(int wms, int wmt, u_int minu, u_int maxu, u_int minv, u_int maxv) {
        AddGsAD(SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv));
    }

    void AddGifPackedAD_FRAME_1(u_int fbp, u_int fbw, int psm, int fbmask) {
        AddGsAD(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(fbp, fbw, psm, fbmask));
    }

    void AddGifPackedAD_PRIM(int prim, int iip, bool tme, bool fge, bool abe, bool aa1, int fst, int ctxt, bool fix) {
        AddGsAD(SCE_GS_PRIM, SCE_GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix));
    }

    void AddGifPackedAD_SCISSOR_1(u_int scax0, u_int scax1, u_int scay0, u_int scay1) {
        AddGsAD(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(scax0, scax1, scay0, scay1));
    }

    void AddGifPackedAD_TEST_1(bool ate, int atst, u_char aref, int afail, bool date, int datm, bool zte, int ztst) {
        AddGsAD(SCE_GS_TEST_1, SCE_GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst));
    }

    void AddGifPackedAD_TEX0_1(u_int tbp, u_int tbw, int psm, u_int tw, u_int th, int tcc, int tfx,
                               u_int cbp, int cpsm, int csm, u_int csa, int cld) {
        AddGsAD(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld));
    }

    void AddGifPackedAD_TEX1_1(int lcm, u_int mxl, int mmag, int mmin, int mtba, u_int l, u_int k) {
        AddGsAD(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k));
    }

    void AddGifPackedAD_TEXFLUSH() {
        AddGsAD(SCE_GS_TEXFLUSH, 0);
    }

    void AddGifPackedAD_XYOFFSET_1(u_int ofx, u_int ofy) {
        AddGsAD(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET(ofx, ofy));
    }
};

#endif /* NALIB_NAPACKET_H */
