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
python -m unittest discover -s tests/prlib -p spm_packets.py -v
python tools/test_matrix_layout.py --compiler /workspace/shared/parappa-env/ee/ee-gcc
```

The audit checks archive and DMA bounds, prefix/vertex extents, clipping tags,
reserved quadwords and contour destinations. It supports the unrelocated
version-5 primary packet modules observed above; an unsupported module or a
nonzero reserved field is an investigation failure, not proof that an asset is
invalid. Extended contour fields are read only when the contour flag is set:
simple serialized nodes can be shorter than the full C++ node type.

Five synthetic tests exercise valid records and rejection of truncated chunks,
wrong contour destinations, unexplained reserved data and truncated archives.
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
