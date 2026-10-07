# Native PC build

The goal is to run the reconstructed game natively on a PC. The PS2 build stays
the reference: every native path sits beside the EE code it replaces, selected
by `__R5900__` (defined by `ee-gcc`, never by a host compiler), and the PS2
build must stay byte-identical when native code is added.

## What native execution can and cannot show

Running natively executes this tree's C/C++, not the original executable. It
can show that the reconstructed types, file layouts, animation and transform
code produce sensible results on real game data (a garbled model means a wrong
layout), and it catches crashes and out-of-range reads. It cannot show that the
result matches what a PS2 draws: the VU's truncating float arithmetic, the GS
and the original microprograms are replaced, not emulated. Equivalence with the
original still needs either PS2 execution or a reference that runs the original
code (for example a static recompilation of `SCPS_150.17`).

## Layers

| Layer | PS2 | Native | Status |
| --- | --- | --- | --- |
| nalib float4 vector/matrix | VU0 inline asm | Scalar C++ in the same headers, same operation order | Done (2026-10-07) |
| libvu0 subset | Sony library | `src/native/libvu0.c` | Done: the eight routines the engine reaches |
| prlib model, animation, camera | VU0 asm blocks in `render.cpp`, `spadata.cpp`, `shape.cpp`, `cluster.cpp`, `model.cpp` | Scalar paths | Next |
| Geometry packets (VU1 microprograms) | `vump_*.vsm` | CPU transform + rasterizer or GPU | Planned |
| GS state, textures, frame buffer | GIF/DMA packets | Software rasterizer, later a GPU backend | Planned |
| IOP sound (WP2 stream, tap sounds) | `WAVE2PS2.IRX`, `TAPCTRL.IRX` | Host audio | Planned |
| Pads, CD file access, threads (MTC), memory card | EE kernel and SDK | Host equivalents | Planned |

### nalib float4 paths

`NaVECTOR<float, 4>` and `NaMATRIX<float, 4, 4>` use the generic templates on
the host; those already accumulate in the VU order (x, y, z, then w). Two
operations need their own host forms because the VU does not do the obvious
thing: division multiplies by the VDIV reciprocal (`v * (1 / s)`, not
`v / s`), and `Normalize3` and division by zero use the largest float, as VDIV
does, so a zero vector normalizes to zero.

### libvu0

`src/native/libvu0.c` implements `sceVu0UnitMatrix`, `MulMatrix`,
`OuterProduct`, `Normalize`, `TransMatrix`, `InversMatrix`, `CameraMatrix` and
`ViewScreenMatrix` from the copies linked into `SCPS_150.17`
(`asm/sdk/libvu0.s`): the same operations, operand order and treatment of each
w component.

## Tests

```sh
gcc -std=c99 -O2 -Wall -Wextra -Iinclude/rtl/ee -Iinclude/rtl/common -c src/native/libvu0.c -o /tmp/libvu0.o
g++ -std=c++98 -O2 -Wall -Wextra -Isrc -Iinclude/rtl/ee -Iinclude/rtl/common tests/native/vu0_math.cpp /tmp/libvu0.o -o /tmp/vu0_math
/tmp/vu0_math
```

The existing `tests/nalib/*.cpp` native tests also build and pass with the host
paths.
