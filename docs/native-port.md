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
| GS local memory | GS | `src/native/gs_memory.c`: page/block/column addressing for PSMCT32/24/16 and PSMT4, CSM1 CLUTs | Started: stage subtitle fonts decode correctly |
| Model viewer | EE, VU1 and GS | `tools/native/spm_view.c`: rest pose, textured and blended, from stage data | Started: PaRappa and the stage props draw correctly |
| GS drawing, frame buffer | GIF/DMA packets | Software rasterizer, later a GPU backend | Planned |
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

### GS memory

`src/native/gs_memory.c` stores pixels at the addresses the GS uses for each
format, so data uploaded in one format and read in another comes back as on
hardware. The game relies on this: its subtitle fonts are 4-bit textures sent
as 32-bit transfers. `tools/native/tim2_vram.c` loads TIM2 files the way
`Tim2Trans` does and reads a texture back through a game TEX0; on the stage
archives the user supplied it yields the legible kanji and ASCII subtitle
fonts ([stage-archives.md](stage-archives.md)). PSMT8 and PSMCT16S addressing
are not written yet.

### SPM viewer

`tools/native/spm_view.c` reads an unrelocated SPM straight from the layouts in
[spm-geometry-layout.md](spm-geometry-layout.md) and
[spm-packet-layout.md](spm-packet-layout.md): it composes the node hierarchy,
walks each node's context-1 packets chunk by chunk, takes TEX0, ALPHA and PRIM
from the chunk's GIF prefix, and draws the vertex strips with a z-buffer,
reading textures through the GS memory model. On the Stage 1 archive it draws
PaRappa (`para_para.spm`: hat, headband with its emblem, eyes, ears, hands), the
burger-shop robot and the stage props, textured.

What it confirmed from real data:

- Vertices are three quadwords (position, colour where 128 is neutral, ST) or,
  for the reflection module 3, four (position, normal, colour, ST).
- Each chunk's vertices form one triangle strip.
- A chunk's prefix is a PACKED A+D GIF packet of five registers (TEX0, TEX1,
  CLAMP, ALPHA, FBA) followed by the drawing tag whose PRIM gives TME and ABE.
- Textures are 4-bit with 16- or 32-bit palettes, and sit inside the stage's
  32-bit uploads at block addresses that are not page aligned, so the memory
  model's swizzles have to be exact (a wrong 16-bit palette layout produced
  static, a wrong 4-bit table produced streaks).

Limits: no animation, skinning, contours, lighting or clipping; reflection
surfaces use approximate sphere-map coordinates (the real ones need the camera,
`vump_refmap.vsm`), so some bodies still render dark; blended chunks are not
depth sorted. It shows layout and content, not PS2 pixel equivalence.

```sh
cc -std=gnu99 -O2 -Wall -Iinclude/rtl/ee -Iinclude/rtl/common -Isrc/native tools/native/spm_view.c src/native/gs_memory.c -lz -lm -o /tmp/spm_view
/tmp/spm_view out.png model.spm -size 640 stage-textures/*.tm0   # -view side, -only 3
```

## Tests

```sh
gcc -std=c99 -O2 -Wall -Wextra -Iinclude/rtl/ee -Iinclude/rtl/common -c src/native/libvu0.c -o /tmp/libvu0.o
g++ -std=c++98 -O2 -Wall -Wextra -Isrc -Iinclude/rtl/ee -Iinclude/rtl/common tests/native/vu0_math.cpp /tmp/libvu0.o -o /tmp/vu0_math
/tmp/vu0_math
```

The existing `tests/nalib/*.cpp` native tests also build and pass with the host
paths.

```sh
cc -std=gnu99 -O2 -Wall -Iinclude/rtl/ee -Iinclude/rtl/common -Isrc/native tests/native/gs_memory.c src/native/gs_memory.c -o /tmp/gs_memory
/tmp/gs_memory
```

