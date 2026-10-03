#include "nalib/napacket.h"
#include "prlib/noodlepacket.h"
static u_int capturedAddress;
static u_long capturedRegister;
static u_long128 capturedTag;
extern "C" void sceGifPkAddGsAD(sceGifPacket*, u_int address, u_long data) {
    capturedAddress = address;
    capturedRegister = data;
}
extern "C" void sceGifPkOpenGifTag(sceGifPacket*, u_long128 tag) { capturedTag = tag; }
int main() {
    // DMA layouts are checked with the EE compiler; host pointers differ.
    NaGifPacket packet;
    sceGsZbuf zbuf = {};
    zbuf.ZBP = 0x12;
    zbuf.ZMSK = 1;
    zbuf.pad09 = 1;
    packet.AddGsRegister(SCE_GS_ZBUF_1, zbuf);
    if (capturedAddress != SCE_GS_ZBUF_1 || capturedRegister != 0x100000212UL) return 1;
    sceGifTag tag = {};
    tag.NLOOP = 42;
    tag.EOP = 1;
    tag.NREG = 1;
    tag.REGS0 = 0xe;
    packet.OpenGifTag(tag);
    const u_long128 expected = (u_long128(0xe) << 64) | (u_long128(1) << 60) | (u_long128(1) << 15) | 42;
    if (capturedTag != expected) return 2;
    return 0;
}
