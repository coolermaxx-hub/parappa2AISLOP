# SPA animation tables

Names here describe behavior established by the original July 12 executable.
They are reconstructed names, not recovered developer symbols. Layout checks
use the historical EE compiler and its 32-bit pointer ABI.

## File header

| Offset | Field | Evidence |
| ---: | --- | --- |
| 0x14 | `m_duration` | `PrGetAnimationEndFrame` multiplies it by the configured frame rate |
| 0x4c | `m_nodeCount` | Number of node pointers visited by file relocation |
| 0x50 | `m_nodes` | Runtime pointer to the inline node table |
| 0x54 | `m_inlineNodes` | Start of variable-length serialized node pointer table |

`SpaFileHeader::ChangePointer` at 0x00142308 binds `m_nodes` to the inline table,
relocates each non-null entry against the file base, relocates that node's tracks,
and optimizes the node. Header flag 1 prevents a second relocation pass. The
public time APIs divide frame inputs by the configured rate; duration is in the
same time units used by the tracks. The table at 0x38 is relocated but its contents
and purpose remain unknown.

## Animation node

| Offset | Field | Evidence |
| ---: | --- | --- |
| 0x04 | `m_visibilityTrack` | Integer track used by `IsVisible`; a missing track means visible |
| 0x08 | `m_transformCount` | Active transform count, reduced by optimization |
| 0x0c | `m_transforms` | Runtime pointer to ordered matrix transforms |
| 0x14 | `m_extraVectorTransformCount` | Count of the additional vector-transform group |
| 0x18 | `m_extraVectorTransforms` | Runtime pointer to that additional group |
| 0x2c | `m_shapeWeightTrackCount` | Scalar weight track count consumed by shape deformation |
| 0x30 | `m_shapeWeightTracks` | First group in the serialized pointer tail |

`SpaNodeAnimation::ChangePointer` at 0x001423c0 establishes the tail layout:

```
node + 0x30
  shapeWeightTrackCount * 4 bytes: SpaTrack<float>* entries
  transformCount * 4 bytes:        SpaTransform* entries
  extraVectorTransformCount * 4:   SpaTransform* entries
```

`BindInlineTables()` expresses these known boundaries using the typed pointer
arrays. It reads no payload through the wrong type. Bind the tables before
optimization: the optimizer compacts active transforms in place, reducing their
count without moving the later group. Rebinding afterward with the reduced count
would point the extra group into the inactive transform slots.

The extra group is relocated as vector tracks, independent of the transform kind.
Its eventual runtime role is not established by the routines currently examined;
the name describes only the observed storage and relocation. The pointer at node
0x10 and reserved bytes remain unexplained.

## Transform and visibility behavior

The primary transform group chooses scalar, vector or matrix tracks according to
the transform kind. Zero-key tracks are replaced with null entries. Unrecognized
kinds retain their entries. `Optimize` at 0x00148e98 removes null or identity
transforms, preserving the order of the remaining entries. It does not compact
shape weights or the extra vector group.

`SpaNodeAnimation::GetMatrix` at 0x001489f0 starts VF13..VF16 at identity and
left-multiplies each transform into that accumulator. Those VU registers stay
live across transform evaluation; the retained hardware path follows that original
convention. The result is shared static storage, not a separately owned matrix.

Node visibility first rejects an invisible parent. A node with no visibility
track inherits the parent's animated-visibility state; otherwise its own integer
track decides visibility. Without an animated source, the node's default-hidden
flag decides. These flag mutations and the original control flow are retained.

## Validation and limits

```sh
. /workspace/shared/parappa-env/activate.sh
python tools/test_matrix_layout.py --compiler /workspace/shared/parappa-env/ee/ee-gcc --source tests/prlib/animation_layout.cpp
```

The scalar test checks original field offsets and typed mixed-table placement,
including empty nodes, preserved entries and the later group's fixed address
when the active transform count decreases. It does not execute the optimizer,
visibility routines or VU composition. Existing scalar track tests cover integer
visibility-track interpolation semantics. Full camera/animation equivalence and
the unidentified fields still need original assets and PS2 execution.
