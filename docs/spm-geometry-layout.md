# SPM hierarchy and deformation records

These names describe uses established by the original executable and retained
VU microcode. They are reconstructed names, not recovered debug symbols. All
listed offsets refer to the 32-bit EE ABI; `SpmNode` remains 0x1c0 bytes.

| Node offset | Field | Evidence |
| ---: | --- | --- |
| 0x000 | `m_localMatrix` | Non-animated composition multiplies the parent matrix by this matrix, unless the identity flag is set |
| 0x040 | `m_worldMatrix` | Composition result; used as each child's parent matrix and uploaded for drawing |
| 0x080 | `m_bindCorrectionMatrix` | Cluster composition multiplies world by this stored matrix |
| 0x0c0 | `m_skinningMatrix` | Cluster composition result, consumed by weighted position transforms |
| 0x140 | `m_sortPosition` | Projected to derive translucent packet depth; cluster rendering weights this position too |
| 0x150 | `m_animationIndex` | Index into node animation and posture matrix arrays |
| 0x158 | `m_owner` | File pointer assigned during relocation, used to resolve cluster node indices |
| 0x15c | `m_firstChild` | Start of recursive child relocation |
| 0x160 | `m_nextSibling` | Link followed after relocating each child |
| 0x164 | `m_parent` | Assigned by relocation; supplies parent visibility and matrix |
| 0x16c | `m_context1Packets[2]` | Opaque packet followed by depth-sorted translucent packet |
| 0x17c | `m_geometryPacket` | Alias of the first non-null context-1 packet; deformation indices use this base |
| 0x180 / 0x184 | `m_textureScrollU/V` | Per-update increments of packet texture offsets |
| 0x188 | `m_sortGroup` | Primary unsigned key of the translucent queue |
| 0x194 | `m_deformPositionCount` | Number of source positions and corresponding target-list records |
| 0x198 | `m_positionTargets` | Variable-length target list for each source position |
| 0x1a4 | `m_contourPacket` | Relocated contour packet, patched with the contour microprogram call |

The hierarchy comes directly from `SpmNode::ChangePointer` at 0x00142168.
Matrix uses are visible in `ComposeGlobalMatrixWithoutVisibility`,
`CalculateClusterMatrix[Animation]`, and `RenderClusterNode`. The correction
matrix likely serves an inverse-bind role, but its precise asset-space convention
has not been verified. Its name deliberately describes the observed composition.

## Position target lists

`AddShapePosition` at 0x0014bd18 reads the pointer at node + 0x198, then a 32-bit
count followed by that many 32-bit indices. Each index is shifted left by four
and added to the geometry packet base. It repeats for the count at node + 0x194.
`RenderShapeNode` and `RenderClusterNode` consume the same layout.

`SpmPositionTargets` now expresses this record. `Next()` advances over its actual
count, not `sizeof(SpmPositionTargets)`. Empty records occupy four bytes; there
is no inter-record alignment padding. Multiple or duplicate targets remain in
original order. Shape accumulation preserves XYZ-only changes and retains W;
cluster deformation still writes the original full vector. No VU arithmetic or
weight accumulation order has changed.

## Node packet and sorting fields

The node header remains 0x1a0 bytes. Relocation reads the microprogram enum at
0x60 and writes `MSCAL(address, 0)` at 0x194. The EE fallback consumes disturbance
at 0x68. The normal/bothface/refmap VU programs read it from node quadword 45.z.
Offsets 0x70 and 0x74 hold texture offsets: `ModifySimpleDmaPacket` advances and
wraps them, and `vump_refmap.vsm` adds quadword 46.xy to reflection coordinates.
The original single-step wrapping is retained, including values exactly 0 or 1.

`PrTransmitEntry` contains `depth`, unsigned `sortGroup`, and a DMA tag pointer
at offsets 0, 4 and 8. Sorting compares group first, then depth. Contours use group
0xffffffff; the renderer changes GIF state when reaching that group. Equal keys
retain the original unstable `qsort` behavior.

## Validation and open fields

Run the historical-compiler scalar checks with the existing environment active:

```sh
python tools/test_matrix_layout.py --compiler /workspace/shared/parappa-env/ee/ee-gcc --source tests/prlib/geometry_layout.cpp
```

The test verifies original offsets and walks serialized multiple-target,
duplicate-target, empty and single-target records. It does not execute COP2,
validate actual model assets or establish rendered equivalence.

The follow-up [real-asset packet audit](spm-packet-layout.md) identifies the
clipped-polygon GIF template and the current/previous contour vertex pairs.
Two zero chunk quadwords remain unresolved; the node and header fields the
engine never reads are listed below.

The historical geometry-pass objdiff report retained 1325/1429 exact functions and 264612/342284
exact code bytes. Only the already-unmatched cluster and shape unit scores change
(from 80.92% to 74.925%, and 85.73163% to 75.0703%, respectively). Explicit record
stepping and indexed target access replace the old incremented integer cursor.
The report does not establish runtime equivalence; VU validation remains open.

## Unnamed node fields in real models (2026-10-07)

A node record is 0x190 bytes unless the node has a deform payload
(`eSpmDeformPayloadMask`, 0xff0). Without one, the next node or the node's own
packet starts at +0x190. That holds for 310 of the 312 such nodes in the
built-in `common.ipk`; the other two are the last node of their file
(`bxy_bxy_SPXt.spm`, file 15, and `para_para.spm`, file 75), so the file
ends there. `SpmNode` declares the full 0x1c0 bytes. The code reaches the
fields past 0x190 only for deform nodes: `ChangePointer` behind
`eSpmDeformPayloadMask`, and the cluster, shape and contour routines behind
their own flags. A port that loads records into a fixed-size struct must
still copy only 0x190 bytes from a node without a payload.

The remaining node fields hold converter data that no code in the original
executable reads, so a port can skip them. Values come from the 43 SPM models
(334 nodes, 22 of them deform nodes) in the executable's built-in `common.ipk`,
using `tools/dev/audit/spm_reserved_fields.py`, which also checks each
relation below:

| Field | Offset | Contents in common.ipk |
| --- | --- | --- |
| `unk100` | 0x100, 64 bytes | Identity matrix in all 334 nodes |
| `m_vertexCount` | 0x168 | Vertex count of the node's context-1 packet, 0 without one (334 of 334). No node has both packets. |
| `m_context1PacketQwords` | 0x174, 2 words | Size of `m_context1Packets[k]` in quadwords, the 0x1a0-byte node upload included (229 of 229 packets); 0 for a null packet (439 of 439) |
| `m_reserved18C` | 0x18C | Always 0 |
| `unk190` | 0x190 | Deform nodes only: `m_vertexCount + m_deformPositionCount` (22 of 22) |
| `m_contourPacketQwords` | 0x1A8 | Contour nodes: size of `m_contourPacket` in quadwords (3 of 3). Other deform nodes: 0, 4, or words such as 0x081860a0 and 0xbfffa938 that look like stale converter memory |
| `m_reserved1AC` | 0x1AC | Deform nodes only; always 0 |
| `m_reserved1BC` | 0x1BC | Deform nodes only; always 0 |

The earlier count of these fields (before this section was revised) read
0x190 and up in every node, so it mixed in the next record: `1.0f` was the
next node's `m_localMatrix[0][0]` and 0x10000019 was the DMA `cnt` tag
(25 quadwords) that opens a packet's node upload.

`unk190` keeps its placeholder name: the sum is exact, but nothing says what
the converter used it for.

Code check: no load or store at any of these offsets appears in any prlib
function of `SCPS_150.17` except unrelated structs (GIF register strips, the
AWFUL background packet). `SpmNode::ChangePointer` does not touch them. An
access through a base already offset into the node would be missed by this
search.

The header's reserved words are unused too. `unk8` and `unk64` are always 0,
`unk54` holds two small words (0 to 4), and `unk60` is 0 in 37 models and
0xc to 0x1d6 in the rest. Two searches find no read of offsets 0x8, 0xc, 0x54,
0x58 or 0x60 on a header:

- In the header's own methods (`SpmFileHeader::*`), the only loads at offset 8
  are on `SpaNodeAnimation` pointers (a non-zero word there leads to
  `SpaNodeAnimation::GetMatrix`).
- Following every header pointer loaded from `PrModelObject::m_spm_image` or
  `SpmNode::m_owner` finds reads of offsets 0x0, 0x6, 0x50, 0x5c, 0x68, 0x70,
  0x74 and 0x78, and none of the reserved ones.

`unk64` is still relocated by `ChangePointer`; with a stored 0 it becomes the
file's base address, which nothing reads.

