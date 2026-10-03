# Matrix reconstruction and validation

This records the earlier matrix-only pass. Subsequent activation of camera,
animation, SPRAM and rendering source is documented in
[source-reconstruction.md](source-reconstruction.md). Build/measurement statements
below are historical, not current status.

This pass replaces compiler steering with readable shared template code. It is
an incomplete reconstruction, not a claim of 100% completion or a matched ROM.

## Layout evidence

`NaVECTOR<T, N>` contains `T v[N]`. `NaMATRIX<T, rows, columns>` contains consecutive
column vectors. Original VU multiplication loads each column and multiplies it
by the corresponding vector component. Translation is in the fourth column.
Original scalar setters write consecutive elements, including the nine-scalar
setter's packed prefix when instantiated for a 4x4 matrix. This behavior is
preserved rather than reinterpreted as a padded 3x3 submatrix.

The new mutable vector accessor and private matrix `Element` accessor express
that storage through typed elements. No casts of vectors or matrices to scalar
buffers are needed for setters or homogeneous-component updates.

Original disassembly establishes an additional distinction:

- The three-scalar scale factory puts 1 in the homogeneous diagonal element.
- The vector scale factory copies all four diagonal components, including `w`.
- The vector translation factory reads only `x`, `y`, `z` and sets the corner to 1.

The generic implementations preserve these distinctions. Both transform methods
pre-multiply the existing matrix. Matrix multiplication remains the existing
hardware VU operation, shared by the operators and in-place transforms; it is
not replaced by CPU arithmetic with potentially different rounding.

## Source changes

Axis rotation now uses ordinary conditionals and constructors. The artificial
one-pass loops and temporary matrix construction helpers are removed from both
the generic header and `spadata.cpp`. Scalar and vector transform factories share
the class template; menderer and animation code call that API directly.

SPRAM's reconstructed initializer calls the typed class methods under
`NON_MATCHING`. Its default build still uses the original initializer and helper
assembly. A missing emitted nine-scalar setter is not counted as a C++ match.
Menderer and animation translation units now emit their used template bodies
naturally; duplicated explicit setter bodies and unused emission helpers are
not used to force their placement.

The four-component vector constructor is one inline generic implementation,
including initialization of static vector constants. No float-specific copy of
its body is needed.

Earlier candidates that forced rendering, cluster and camera matches using
pinned registers, an empty assembly barrier or artificial data aliases were
withdrawn. Their existing reconstruction remains unfinished. Animation track
payload types and static-local storage in `spadata.cpp` also still need analysis;
this pass does not establish those layouts or certify that function as finished.

## Validation

Run the scalar layout and transform checks with the historical EE compiler:

```sh
. /workspace/shared/parappa-env/activate.sh
python tools/test_matrix_layout.py --compiler /workspace/shared/parappa-env/ee/ee-gcc
```

The test compiles the actual headers, links a freestanding scalar MIPS test and
runs it under QEMU. It checks matrix sizes, column storage, preservation of the
packed-prefix setter tail, vector scaling with non-unit `w`, and translation
with a non-unit input `w`. It does not run COP2 or validate PS2 float edge cases.

A clean build compiles and links both `SCPS_150.17.rom` and `WAVE2PS2.IRX.rom`.
The IOP ROM checksum passes. The main ROM checksum fails after the readable
refactor; the expected checksum remains unchanged. Plain `ninja` therefore
fails its main checksum validation, as it should.

The final object report measures 1,336 / 1,429 exact functions (93.49%) and
272,484 / 342,284 matched code bytes (79.61%). The pre-refactor baseline had
1,346 exact functions. This decrease is recorded rather than hidden by changing
published badges or claiming readable source is an exact match.

Object comparison confirms that typed four-by-four scalar stores and the
three-scalar translation factory can still match exactly. Scale construction,
rotation and in-place transform copies have remaining instruction, scheduling
and register differences. Removing artificial helper emission also changes
function order and downstream relocations. These differences are unresolved;
compilation and the scalar tests alone do not prove game behavior equivalence.

The next work is to establish animation track payload types and actual
static-local lifetimes, then reconstruct the remaining camera and rendering
functions against those types. Keep exact matching reports separate from this
source reconstruction work.
