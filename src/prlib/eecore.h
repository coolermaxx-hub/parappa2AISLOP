#ifndef PRLIB_EECORE_H
#define PRLIB_EECORE_H

#include <eetypes.h>
#include <eestruct.h>
#include <libdma.h>
#include <nalib/namatrix.h>

// The EE fallback consumes three quadwords per vertex and emits the same
// number in GIF order. Integer color/texture bits are copied without conversion.
struct PrEECoreInputVertex {
    NaVECTOR<float, 4> position;
    u_long128 color;
    u_long128 texture;
};
struct PrEECoreOutputVertex {
    u_long128 texture;
    u_long128 color;
    u_long128 position;
};

typedef void (*PrEECoreVertexKernel)();

struct PrEECoreContext {
    NaMATRIX<float, 4, 4> screenMatrix;
    NaMATRIX<float, 4, 4> clipMatrix;
    PrEECoreInputVertex input;
    PrEECoreOutputVertex output;
    u_long128 reserved;
    PrEECoreVertexKernel vertexKernel;
    u_int reservedTail[3];

    static void WaitForVuTransfers() {
        // The original EE path uses four iterations of four CPU NOPs around
        // VU matrix transfers. This is a hardware pipeline delay, not compiler
        // steering; keep it explicit until PS2 execution validates an adapter.
        for (int cycleGroup = 0; cycleGroup < 4; cycleGroup++) {
            asm volatile("nop\n\tnop\n\tnop\n\tnop" : : : "memory");
        }
    }
};

struct PrVuChunkMetadata {
    u_int prefixVertexCount;
    u_int reserved;
    float winding;
    u_int vertexCount;
};

struct PrVuDataChunkPacketHeader {
    sceDmaTag dma;
    PrVuChunkMetadata metadata;
    // These three transport quadwords are not interpreted by the EE path.
    // Their VIF-side meanings remain to be decoded; do not invent fields.
    u_long128 transportRecords[3];
    sceGifTag gif;
    PrEECoreInputVertex vertices[1];

    PrVuDataChunkPacketHeader *Next() {
        if (dma.id != 0x10) return NULL;
        return reinterpret_cast<PrVuDataChunkPacketHeader*>(
            reinterpret_cast<u_long128*>(this) + dma.qwc + 1);
    }
};

#endif /* PRLIB_EECORE_H */
