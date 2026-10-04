# SPM chunk and contour packet layout

The built-in `common.ipk` supplies real model records for the remaining packet
questions. The 2026-10-04 audit of `assets/common_ipk.databin.bin` found 43 SPM
models, 334 nodes, 229 primary packets and 2807 primary chunks. Primary packets
use the normal (32), both-face (11) and screen (186) microprograms. These counts
cover this archive, not all stage assets.

## Chunk header and variable GIF prefix

Offsets below are relative to the chunk's DMA tag, after the 0x1a0-byte node
upload. `PrVuDataChunkPacketHeader` now describes the fixed 0x60-byte header;
it no longer pretends that variable GS state is an input vertex array.

| Offset | Record | Evidence |
| ---: | --- | --- |
| 0x00 | DMA tag with VIF commands | CNT chunks followed by RET |
| 0x10 | Prefix triplet count, reserved word, winding, vertex count | EE renderer and VU chunk setup |
| 0x20 | Clipped-polygon GIF template | `scissor_initialize` loads VU TOP+1 |
| 0x30, 0x40 | Two reserved quadwords | Zero in all 2807 primary chunks; purpose unresolved |
| 0x50 | Start of variable GIF prefix | Copied before transformed vertex output |
| 0x60 + prefixTripletCount * 0x30 | Input vertices | Three quadwords per primary vertex |

In `src/prlib/vu1/vump_menderer_draw_mesh.vsm`, `scissor_initialize` obtains
TOP, advances to TOP+4, and loads the template from three quadwords earlier.
It copies the prefix in groups of three, appends the template, then fills the
clipped polygon's loop count and end bit before kicking its output packet.
All audited templates specify packed ST/RGBAQ/XYZ2 (`REGS = 0x512`, NREG = 3),
PRE enabled, zero initial loops and triangle-fan PRIM. Their other PRIM flags
vary by material; the ordinary draw tag can specify a strip instead.

The prefix is `1 + prefixTripletCount * 3` quadwords long. For example, a
`hirecord` chunk has a seven-quadword prefix: an A+D tag, five GS register
writes and a drawing tag. Calling these groups "prefix vertices" was wrong.
`InputVertices()` now steps over the actual serialized prefix at the typed
packet boundary. Rendering still copies those quadwords unchanged.

## Contour pairs

The ONMEM file 75 (`para_para.spm`) has three contour nodes and 201 mapped
pairs (129, 36 and 36). Every destination index points to the beginning of a
pair in the contour vertex stream:

| Pair offset | Typed member |
| ---: | --- |
| 0x00 | `current.position` |
| 0x10 | `current.color` |
| 0x20 | `previous.position` |
| 0x30 | `previous.color` |

`vump_contour.vsm` consumes alternating position/color records and alternates
the two alpha values. `SaveContour` writes the saved position two quadwords
after the mapping destination; `RenderContour` writes the current position at
the mapping destination. These are members of one pair, not different index
origins or a separate packet prefix. `PrVuContourVertexPair` expresses this
layout without changing transforms, color data or draw order. "Previous"
means the position captured by the caller's save operation.

## Reproduce and limits

With the user's extracted asset available:

```sh
python tools/dev/audit/spm_packets.py assets/common_ipk.databin.bin
python tools/dev/audit/spm_packets.py path/to/model.spm --format spm
python -m unittest discover -s tests/prlib -p spm_packets.py -v
python tools/test_matrix_layout.py --compiler /workspace/shared/parappa-env/ee/ee-gcc
```

The audit checks archive, DMA and VIF bounds, prefix/vertex extents, clipping
tags, reserved quadwords and contour source/destination records. It supports
unrelocated version-5 normal, both-face, reflection, screen and antiline primary
packets, plus contour packets. Reflection and antiline currently have synthetic
test coverage, not real-asset coverage. An unsupported module or a
nonzero reserved field is an investigation failure, not proof that an asset is
invalid. Extended contour fields are read only when the contour flag is set:
simple serialized nodes can be shorter than the full C++ node type.

Fourteen synthetic tests exercise valid records, the supported vertex strides,
VIF boundaries and commands, source/destination record alignment, reserved data
and archive truncation.
The EE scalar test checks the header, prefix stepping and contour pair layout.
None of these checks executes the GS/VU rendering pipeline. The two zero
quadwords and other reserved node fields remain open; original PS2 execution
is still needed for rendering equivalence. No original asset bytes are included
in the tests or committed by this change.

## Binary comparison

Historical-compiler assembly was compared with `3edc2a2` using its original
headers as well as sources. After accounting for one-to-one local-label
renumbering, the changes are confined to the two access expressions:
`SaveContour` adds 32 bytes after scaling the pair index instead of adding two
quadwords before scaling, and `RenderChunkEECore` forms the vertex base before
adding the prefix displacement, with different temporary registers and scheduling.
The typed expressions are retained. These functions already differed from the
original binary; exact-function statistics alone do not describe these changes.

The full-tree objdiff measurement is recorded in [remaining-work.md](remaining-work.md).
It includes the earlier readability work fetched from `codex-work`, not only
this packet-layout change. Both ROMs build, the IOP checksum passes, and the
main-ROM checksum remains different from the original.

## VIF and reserved-slot trace (2026-10-04)

The follow-up checked all 2807 primary chunks and 10 contour chunks. Each DMA
tag carries NOP followed by unmasked V4-32 UNPACK at TOPS-relative address zero
(`0x6cNN8000`). `SendDisplayHeader` sets STCYCL(4,4) and STMOD(0), so each
uploaded quadword occupies one VU quadword without cycling gaps or addition.
The uploaded range ends one quadword before the DMA payload ends. That final
quadword contains MSCNT (`0x17000000`) and three NOPs; it is command data, not
vertex data. The audit now validates both boundaries independently and treats
UNPACK NUM=0 as 256 quadwords, as required by VIF.

Tracing the retained microprograms establishes the following accesses:

| Path | Header and prefix accesses |
| --- | --- |
| `start_normal`, `start_bothface`, `start_contour`, `start_refmap`, `start_screen`, `start_antiline` | Read metadata at TOP; initialize input cursor to TOP+4; copy one prefix quadword followed by `prefixTripletCount` groups of three |
| `scissor_initialize` | Read prefix count at TOP, then load clipping tag at TOP+1; copy prefix from TOP+4 |
| Ordinary output | Begin output at TOP+0xd2, separate from the chunk header |

Thus TOP+2 and TOP+3 are **uploaded but skipped by these header consumers**.
They are zero in all 2817 audited chunks. No semantic field name is justified
by these accesses. They remain reserved; this trace does not explain why the
asset producer allocated them or prove that every possible path leaves them
unused. In particular, it is not a PS2 execution trace.

The normal/both-face/screen/antiline input strides are three quadwords;
reflection adds a normal and uses four; contours use two. These values agree
with `PrGetInputVertexParameterNum` and the VU load sequences. Contour source
indices now have to select a position in the first non-null primary packet,
not merely an in-bounds address. All 201 built-in mappings pass this stronger
check. The source audit does not execute vertex arithmetic or DMA scheduling.

## Stage and execution coverage limits

All 11 supplied `stg*.olm` files were scanned for the unrelocated SPM magic
(`0a 54 df 18`); none contain it. This does not rule out compressed or externally
loaded models. The standalone input mode is ready for extracted stage SPMs;
no additional real stage model coverage is claimed.

The uploaded `SCPS-18002.zip` could not be transferred into this executor:
the download tool reports that it exceeds its 32 MiB transfer limit. No PCSX2
executable or captured original-game replay is available in this workspace.
Consequently animation, contour and noodle rendering comparisons against PS2
execution remain unperformed. They require accessible extracted stage assets
and an original/reconstructed execution comparison with the same inputs.

This follow-up changes the audit and documentation only; it changes no runtime
source or ROM output. The preceding build and objdiff results remain historical
validation of the runtime source, not evidence of PS2 equivalence.
