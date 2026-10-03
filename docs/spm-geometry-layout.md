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

The three chunk transport quadwords, reserved node/header fields and the
contour history's distinct index origins remain unresolved. Naming the fields
above does not resolve those separate questions.

The follow-up objdiff report retains 1325/1429 exact functions and 264612/342284
exact code bytes. Only the already-unmatched cluster and shape unit scores change
(from 80.92% to 74.925%, and 85.73163% to 75.0703%, respectively). Explicit record
stepping and indexed target access replace the old incremented integer cursor.
The report does not establish runtime equivalence; VU validation remains open.
