#include "microprogram.h"

#include "vu1/vumem.h"

#include "vu1/vump_common.h"

#include "vu1/vump_normal.h"
#include "vu1/vump_bothface.h"
#include "vu1/vump_contour.h"
#include "vu1/vump_refmap.h"
#include "vu1/vump_screen.h"
#include "vu1/vump_antiline.h"
#include "vu1/vump_menderer.h"

#include <libdma.h>

static u_int MicroProgramEntryPoint[PR_MICRO_PROGRAM_MAX];

static u_int MendererCreateTextureEntryPoint;
static u_int MendererDrawMeshEntryPoint;

static void CalculateEntryPoint() {
    MicroProgramEntryPoint[PR_MICRO_PROGRAM_NORMAL]   = VU_ADDR(vump_normal,   vump_top);
    MicroProgramEntryPoint[PR_MICRO_PROGRAM_BOTHFACE] = VU_ADDR(vump_bothface, vump_top);
    MicroProgramEntryPoint[PR_MICRO_PROGRAM_CONTOUR]  = VU_ADDR(vump_contour,  vump_top);
    MicroProgramEntryPoint[PR_MICRO_PROGRAM_REFMAP]   = VU_ADDR(vump_refmap,   vump_top);
    MicroProgramEntryPoint[PR_MICRO_PROGRAM_SCREEN]   = VU_ADDR(vump_screen,   vump_top);
    MicroProgramEntryPoint[PR_MICRO_PROGRAM_ANTILINE] = VU_ADDR(vump_antiline, vump_top);

    MendererCreateTextureEntryPoint = VU_ADDR(vump_menderer_create_texture, vump_top);
    MendererDrawMeshEntryPoint      = VU_ADDR(vump_menderer_draw_mesh,      vump_top);
}

void PrLoadMicroPrograms() {
    sceDmaChan *chan = sceDmaGetChan(SCE_DMA_VIF1);

    /*
     * TTE is required to transfer
     * the first MPG VIFcodes inside
     * of the DMAtag's upper 64 bits:
     *
     * NOP,
     * MPG 0x0, 0x100 (2048 bytes)
     */
    chan->chcr.TTE = 1;

    sceDmaSend(chan, vump_dmatag);
    CalculateEntryPoint();
}

u_int PrGetMicroProgramAddress(PrMICRO_PROGRAM_MODULE module) {
    return MicroProgramEntryPoint[module];
}

u_int PrGetInputVertexParameterNum(PrMICRO_PROGRAM_MODULE module) {
    static u_int ParameterTable[] = {
        3, /* NORMAL   */
        3, /* BOTHFACE */
        2, /* CONTOUR  */
        4, /* REFMAP   */
        3, /* SCREEN   */
        3, /* ANTILINE */
    };

    return ParameterTable[module];
}

// Apparently a vertex attribute mask: its bit count is the parameter number
// above (13, 5 and 15 have 3, 2 and 4 bits), and contour vertices carry
// position and colour only. Inferred bits: 1 position, 2 normal (reflection
// map only), 4 colour, 8 texture coordinates. No game code calls this; SPM
// packets store the same value (PrVuNodeHeaderDmaPacket::m_inputVertexParameterSet).
u_int PrGetInputVertexParameterSet(PrMICRO_PROGRAM_MODULE module) {
    static u_int ParameterTable[] = {
        13, /* NORMAL   */
        13, /* BOTHFACE */
        5,  /* CONTOUR  */
        15, /* REFMAP   */
        13, /* SCREEN   */
        13, /* ANTILINE */
    };

    return ParameterTable[module];
}

u_int PrGetOutputVertexParameterNum(PrMICRO_PROGRAM_MODULE module) {
    static u_int ParameterTable[] = {
        3, /* NORMAL   */
        3, /* BOTHFACE */
        2, /* CONTOUR  */
        3, /* REFMAP   */
        3, /* SCREEN   */
        3, /* ANTILINE */
    };

    return ParameterTable[module];
}

u_int PrGetOutputVertexParameterSet(PrMICRO_PROGRAM_MODULE module) {
    static u_int ParameterTable[] = {
        13, /* NORMAL   */
        13, /* BOTHFACE */
        5,  /* CONTOUR  */
        13, /* REFMAP   */
        13, /* SCREEN   */
        13, /* ANTILINE */
    };

    return ParameterTable[module];
}

u_int PrGetMendererCreateTextureAddress() {
    return MendererCreateTextureEntryPoint;
}

u_int PrGetMendererDrawMeshAddress() {
    return MendererDrawMeshEntryPoint;
}
