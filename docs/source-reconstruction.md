# Readable source reconstruction

The compiler-generated assembly fallback removal is complete for the active
`src/` tree. Only two handwritten EE/VU vertex kernels remain in
`src/prlib/renderee.cpp`; the obsolete `src/prlib/old` directory is not built.
This is a source milestone, **not 100% validated decompilation or an exact ROM
match**. Unknown format fields and PS2 behavior still require investigation.

## Reconstructed systems

- Animation tracks share a real `SpaTrack<T>` implementation. The serialized
  keys are inline typed values followed by key times, rather than a fabricated
  pointer. Spline keys contain value/incoming/outgoing tangent records. Transform
  tags select typed payloads and matrix scratch results have real static lifetime.
- Camera sampling, animation composition, posture blending and scene rendering
  compile from C++. Classes own their operations; artificial matrix helpers,
  copied template bodies and unused template-emission helpers were removed.
- Matrix setters address typed column storage. Scale/translation and rotation
  share their implementations. The historical packed-prefix setter and vector
  scale's homogeneous component are preserved. Hardware math declares its real
  memory effects; matrix multiplication no longer falsely clobbers CPU registers.
- Scratchpad RAM has overlapping typed views for GIF workspace and three noodle
  banks. Noodle drawing, texture creation/copying, alpha modulation, awful
  backgrounds and depth of field use reconstructed source. Known DMA/GIF records
  have named fields. GS register serialization copies the SDK representation
  without reading register structs through incompatible scalar pointers.
- Cluster influences are count-prefixed `(nodeIndex, weight)` records. Shape
  and contour nodes inherit the shared node layout. The flags select cluster or
  shape payloads in a typed union. Shape position/weight fields have named uses.
- The EE node and chunk renderers now have C++ control flow. Their context defines
  screen/clip matrices and input/output vertex records. They preserve the
  handwritten kernels' persistent VU register convention, strip winding, VU R
  disturbance sequence and three-buffer rotation. The original transfer delay
  remains an explicitly documented hardware operation.
- Existing readable menu, save-data, memory-card and scene draw-control drafts
  are enabled. Option button timers use their declared array; memory-card retry
  flow uses ordinary loops instead of jumping across nested loops. Menu math
  includes its proper declarations.

These layouts come from original instruction addressing, SDK definitions and
retained data templates. Names describing use are inferred where original debug
names are unavailable. Serialized boundary casts describe known variable records
or PS2 cache aliases; they are not a substitute for inventing unknown layouts.

## Validation

The rootless cloud toolchain is described by the saved environment configuration.
In a build shell, activate it first:

```sh
cd /workspace/parappa2AISLOP
. /workspace/shared/parappa-env/activate.sh
```

A clean configuration/build compiled and linked both ROMs with the historical
EE/IOP toolchains. Header dependencies are incomplete in the existing Ninja
configuration, so clean before validating shared-header changes:

```sh
ninja -t clean
python /workspace/shared/parappa-env/configure.py
ninja -j4 build/SCPS_150.17.rom build/WAVE2PS2.IRX.rom
```

Meaningful checks passed:

```sh
python tools/test_matrix_layout.py --compiler /workspace/shared/parappa-env/ee/ee-gcc
g++ -std=c++98 -O2 -Wall -Wextra -Werror -Isrc tests/prlib/spatrack.cpp -o /tmp/spatrack-test
/tmp/spatrack-test
g++ -std=c++98 -O2 -Wall -Wextra -Werror -Isrc -Iinclude/rtl/ee -Iinclude/rtl/common tests/nalib/gifpacket.cpp -o /tmp/gifpacket-test
/tmp/gifpacket-test
python -m unittest discover -s tests/prlib -p noodle_texture_model.py -v
```

The EE-compiled scalar test checks matrix/track layouts and stores, homogeneous
transform conventions, the complete 16 KiB scratchpad, node inheritance/payload
positions, DMA/GIF record sizes and EE chunk/context offsets. Native tests cover
track cache searches, interpolation, endpoint/loop behavior and bit-preserving
GS/GIF encoding. Native pointers differ from PS2 pointers, so DMA layout asserts
run with the EE compiler, not the host compiler. Generic MIPS emulation does not
execute PS2 COP2 or validate PS2 floating-point behavior. The runner now uses a MIPS64 N32 ELF carrier, preserving 32-bit pointers while
executing actual 64-bit register saves. Its earlier MIPS32 assembler expanded
return-address loads in branch delay slots, so those earlier runs were inadequate
validation. The corrected runner also maps scalar R5900 three-operand MULT to
MIPS MUL only when HI/LO is never read, preserving the single delay-slot
instruction. Production objects are never adapted. The corrected checks pass
without the assembler warning.

Checksum validation keeps the original expected hashes:

```sh
sha1sum -c config/p3.jul12.checksum.sha1
sha1sum -c config/irx.wave2ps2.jul12.checksum.sha1
```

`WAVE2PS2.IRX.rom` passes. `SCPS_150.17.rom` fails after readable reconstruction.
The expected hashes have not been changed.

## Exact matching

Current measured totals are recorded in `remaining-work.md`. Measure separately
from source coverage:

```sh
python /workspace/shared/parappa-env/configure.py --objdiff
python tools/objdiff_symbol_mappings.py
tools/objdiff/objdiff-cli report generate -p . -o /workspace/shared/parappa-env/current-report.json -f json-pretty
```

The mapping tool now pairs only aliases verified from original function bodies.
Text offsets alone previously paired unrelated functions after source sizes
changed, including `SearchSegment` with `GetMatrix` and noodle rotation with
speed updates. Those pairings are removed. A missing unused template instance
remains unmatched; no unused call is added to manufacture its emission.

The objdiff configuration path can rewrite original assembly encoding. Perform
another clean normal configuration/build afterward before checking ROM hashes.


## Follow-up findings

The [texture model](noodle-texture-model.md) is now derived and tested against
its setup instructions. Named amplitudes, spatial cycles, temporal frequency and
cycle phase offsets replace neutral coefficient fields without changing the
serialized layout.

Vector/matrix copying previously accepted only float4 types despite being class
templates. It now uses shared type/dimension-aware implementations plus actual
float4 MMI specializations. A 3x3 identity test previously referred to the 4x4
identity constant and failed to instantiate; it now uses its own type. The scalar
tests cover these APIs, self-assignment, integer vector copies, small-vector
arithmetic and 3x3 matrix arithmetic. Basic scalar and component-wise arithmetic
now respects template types and dimensions; explicit float4 specializations retain
the original VU instructions and reciprocal rounding. Matrix products and actual
PS2 execution remain separate work. A deliberately incorrect arithmetic assertion
was rejected with test exit 32, verifying the corrected runner reports failures.

The follow-up report loses one exact match in `SpmNode::ApplyBillboardMatrix`
(300 original code bytes; 99.26667% fuzzy instruction agreement). The shared
math implementation is retained; no compiler-steering workaround is added.
Billboard vector component accesses now use typed indexing instead of float-pointer
casts. Matching percentages alone do not establish the cause or behavior of a
difference.

## SPM hierarchy and deformation follow-up

See [spm-geometry-layout.md](spm-geometry-layout.md) for field provenance and
validation. Node hierarchy, matrix roles, packet selection and depth-sort fields
now use descriptive names. Shape and cluster deformation share the serialized
`SpmPositionTargets` record rather than walking anonymous integer streams.
Reserved fields and the bind-correction asset-space convention remain explicit
unknowns. The EE scalar geometry test checks layouts and variable record stepping;
it does not validate VU execution.

## SPA animation table follow-up

See [spa-animation-layout.md](spa-animation-layout.md) for original addresses,
field uses and unknowns. File/node tables, visibility tracks and shape-weight
tracks now have descriptive names. `BindInlineTables()` owns the known serialized
boundaries and runs before optimization changes the active transform count.
EE tests validate mixed and empty table layouts; native tests now cover integer
visibility-track step sampling, nonzero negative values, endpoints and wrapping.
These checks do not execute hierarchy visibility or VU matrix composition.
