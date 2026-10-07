# Binary differences

Function-level differences between this tree and the original-compiler objects (`expected2/`), measured by `tools/dev/audit/alldiff.py`.
Each difference comes from a function that never matched as C (the `main-rom-matching` branch keeps its original asm), or from replacing compiler steering, alias casts and weak template copies with typed C/C++. None is a behaviour change by intent, but the original instruction order is not kept.
Causes marked *inferred* were read from the source change, not traced instruction by instruction. A unit marked *not yet classified* is an open audit item.

Total: 102 functions differ, 23 original symbols have no counterpart (mostly orphan `func_XXXXXXXX` helpers and brute-forced template copies that were removed on purpose).

| Unit | Differing functions | Original symbols not reproduced | Cause |
| --- | --- | --- | --- |
| `nalib/navector.cpp.o` | 1 | 0 | Set(NaMATRIX3) is the real template. The original image carried a brute-forced copy; that copy is not reproduced. |
| `menu/pksprite.c.o` | 4 | 0 | PkTEX0_SetAdd builds TEX0 with `SCE_GS_SET_TEX0` as one 64-bit value; the original wrote it as two 32-bit halves (see remaining-work.md). GetDToneColor, GetToneColorA and GetToneColorH declare the scratch registers their MMI clamp-and-pack asm overwrites ($2, $8 and $4 or $9); the original left them undeclared, which only worked because of where the compiler put the inputs, so the inputs and result now sit in other registers. |
| `menu/menu.c.o` | 1 | 0 | MenuCtrl: register pin removed, and the matching draft's self-assignment and repeated `pLog` store dropped (see remaining-work.md). |
| `menu/p3mc.c.o` | 3 | 0 | Typed tables and `P3MC_DATE` replace word casts and a do-while (see remaining-work.md). |
| `menu/menusub.c.o` | 14 | 0 | Eleven functions were reconstructed from asm and never matched without compiler steering; `main-rom-matching` keeps their original asm under `NON_MATCHING` (TsRestoreSaveData, TsRanking_Set, TsPopMenu_Flow, TsPopMenu_Draw, TsPopMenCus_Draw, TsSaveMenu_Draw, TSJukeCDObj_Draw, TsCmnCell_CusorDraw, TsUserList_SetCurTag, TsUserList_Flow, TsNAMEINBox_Draw). TsBGMStop lost a one-pass do-while (tail-call layout only), TsGetTm2Tex and TsGetTm2HedTex read TEX0 fields through `sceGsTex0` (see remaining-work.md). |
| `main/wipe.c.o` | 1 | 0 | WipeParaOutDisp: loop-alignment asm removed (see remaining-work.md). |
| `main/drawctrl.c.o` | 4 | 0 | DrawMozaikuDisp uses a typed masked frame and drawDispCheckSub lost a temporary named after a register (see remaining-work.md). Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (DrawObjStrDisp, DrawObjPrReq: register allocation). |
| `main/scrctrl.c.o` | 1 | 0 | ScrExamSetCheck: gotos replaced by structured control flow; the replay score buffer is declared in its own block, as the original debug info shows, which changes the stack frame layout. |
| `main/mbar.c.o` | 1 | 0 | MbarGetDispPosY picks the row offset with a plain conditional instead of a register-named temporary and an off-by-one constant (see remaining-work.md). |
| `prlib/scene.cpp.o` | 2 | 2 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (the constructor and SetAppropriateDefaultCamera). The missing symbols are the original's per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template (NaVECTOR<float, 4>::Set and the 4-argument constructor). |
| `prlib/setpointer.cpp.o` | 2 | 0 | Both functions inline `SpaTrack<T>::ChangePointer`, which finds the time table with one stride expression instead of a branch per interpolation kind; SpaNodeAnimation::ChangePointer also binds its tables through `BindInlineTables`. |
| `prlib/spadata.cpp.o` | 15 | 2 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (the generic GetValue/GetLinearValue/GetSprineValue instances and both RotateMatrix copies). SpaTransform::GetMatrix now differs only in calling the shared NaMATRIX::Set instead of the original's weak copy. IsEverIdentical handles the scale, translate and shear cases in one branch where the original tests them separately; the NaVECTOR spline differs in register allocation. The missing symbols are the original's per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template (the 9- and 16-argument NaMATRIX Set). The vector forms of ScaleMatrix and TranslateMatrix are emitted here as the shared template and differ from the original's copies. |
| `prlib/cluster.cpp.o` | 1 | 0 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (RenderClusterNode: two registers swapped). |
| `prlib/renderee.cpp.o` | 5 | 1 | The two vertex kernels are inline asm blocks instead of standalone functions, and the orphan helper is gone; surrounding functions inherit the shifted register use. |
| `prlib/mendererawful.cpp.o` | 4 | 1 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (all four functions). The missing symbol is a weak nalib copy. |
| `prlib/transition.cpp.o` | 1 | 0 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (BlendTransitionMatrix). |
| `prlib/mendereralpha.cpp.o` | 3 | 0 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (all three functions). |
| `prlib/shape.cpp.o` | 2 | 0 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (RenderShapeNode). AddShapePosition walks typed `SpmPositionTargets` records and gives its VU asm a memory clobber, where the original did raw index arithmetic on the packet address. As in RenderContext2Node, the original reads the sort position's z and w before storing them; this build uses the stored values ([notes](behavior-notes.md#stale-sort-depth-in-context-2-and-shape-nodes-2026-10-07)). |
| `prlib/menderer.cpp.o` | 6 | 5 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (five functions). SetNoodleRotationMatrix has the same source but calls the shared NaMATRIX templates, which moves its registers. The missing symbols are the original's per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template (Set, ScaleMatrix, TranslateMatrix, RotateMatrix). |
| `prlib/spram.cpp.o` | 6 | 1 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (Initialize); the static initializer differs only by a trailing alignment nop. The missing symbols are the original's per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template (the 9-argument Set). Scale, Translate, ScaleMatrix and TranslateMatrix are emitted here as the shared template and differ from the original's copies. |
| `prlib/camera.cpp.o` | 1 | 3 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (GetCamera). The missing symbols are the original's per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template (Set and RotateMatrix). |
| `prlib/contour.cpp.o` | 2 | 0 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (both functions). RenderContour also differs in behaviour: the original reads its sort depth's z and w before storing the projected position ([notes](behavior-notes.md#stale-sort-depth-in-context-2-and-shape-nodes-2026-10-07)). |
| `prlib/render.cpp.o` | 7 | 1 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (CalculateCurrentMatrixAnimation and CalculateClusterMatrixAnimation; both inline ComposeAnimatedMatrix as the original does, but the original also inlines ComposeGlobalMatrixWithoutVisibility where this build calls it; ComposeGlobalMatrixWithoutVisibility itself; Render). Traced against the original objects (2026-10-07): CalculateClusterMatrix differs only in register allocation and scheduling (one extra register copy, otherwise the same operations); CalculateCurrentMatrix declares the position-animation matrix `pos` outside its `if` block, so `mtx` never points at an ended object, which gives `pos` its own stack slot (a 208-byte frame instead of 160) and moves its empty constructor loop ahead of the branch; RenderContext2Node computes its sort depth from the stored position, where the original reads `pos[2]` and `pos[3]` from the stack before storing them (reproduced by removing the nalib asm `"memory"` clobbers, which the matching reconstruction lacks). That last one is a behaviour difference ([notes](behavior-notes.md#stale-sort-depth-in-context-2-and-shape-nodes-2026-10-07)). The missing symbols are the original's per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template (a renderstuff.h helper). |
| `prlib/mendererdata.cpp.o` | 2 | 3 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (PrGetNoodlePolygonPosition, SetNextTarget). The missing symbols are the original's per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template (2x2 Set and NaVECTOR::Set). |
| `prlib/menderercreate.cpp.o` | 5 | 0 | Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these (four functions). PrWaitMendererTexture writes the packet's register slots through the typed packet struct instead of casting into a u_long array. |
| `prlib/random.cpp.o` | 1 | 0 | PrFloatRandom is a plain do-while (see remaining-work.md). |
| `prlib/depthfield.cpp.o` | 3 | 1 | The original carried a local copy of `NaGifPacketWrapper::AddGsAD`; ours calls the shared weak symbol. The copy is a banned brute-forced template. |
| `prlib/prlib.cpp.o` | 1 | 0 | PrSetDebugParamFloat stores through the union member (see remaining-work.md). |
| `prlib/billboard.cpp.o` | 2 | 3 | Traced against the original objects (2026-10-07): ApplyBillboardMatrix differs only in register allocation (`a0` and `a1` swap roles); CreateBillboardMatrix has the same operations plus one register copy and one nop, with one load scheduled later. The missing symbols are the original's per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template (Set and RotateMatrix(int)). |
| `prlib/model.cpp.o` | 1 | 0 | GetPrimitivePosition calls the shared NaVECTOR::Set instead of a local weak copy, and the inlined matrix Apply is scheduled differently. |

Regenerate with `python3 tools/dev/audit/alldiff.py && python3 tools/dev/audit/gen_binary_diff_doc.py`.

## Packet layout follow-up (2026-10-04)

The [SPM packet reconstruction](spm-packet-layout.md) replaces the contour
`destination + 2` convention with `pair.previous.position` and removes a false
vertex-array member from the chunk header. Comparison against `3edc2a2` with
the historical compiler shows changed address arithmetic and scheduling in
`SaveContour` and changed base formation/register allocation in
`RenderChunkEECore`. Both functions already differed from the original; this
pass preserves the typed expressions rather than steering the compiler back.
The table above was regenerated on 2026-10-07 against a fresh build of
`main-rom-matching` (`af59110`, main checksum OK) as `expected2/`. That build
names template copies the earlier baseline left as `func_XXXXXXXX`, so some
symbols moved from "not reproduced" to "differing" without a source change.

## Where the main ROM's 4,864 bytes go (2026-10-07)

The built main ROM is 4,864 bytes shorter than the original because `.text`
shrinks and every later section moves down with it. Measured against
`iso/SCPS_150.17`:

| Section | Original | This build | Difference |
| --- | --- | --- | --- |
| `.text` | 0x78034 (491,572) | 0x76d58 (486,744) | -4,828 |
| `.data` start | 0x17a900 | 0x179600 | -0x1300 (-4,864) |
| `.data`, `.sdata` | | | same size |
| `.rodata` | 0x7e74 | 0x7e80 | +12 |
| `.gcc_except_table` | 0x148 | 0x180 | +56 |

`.data` is 64-byte aligned, so the 4,828-byte code shrink becomes the 4,864-byte
ROM difference.

Per object, `.text` is 5,076 bytes smaller in total. Alignment between
objects at link time absorbs the rest, leaving 4,828. That 5,076 breaks down as:

| Cause | Bytes |
| --- | --- |
| `CalculateCurrentMatrixAnimation` and `CalculateClusterMatrixAnimation` call `ComposeGlobalMatrixWithoutVisibility` instead of inlining it (render.cpp) | -3,240 |
| Per-file nalib template copies in the original (4,224 bytes in 9 prlib units) replaced by shared instances (3,816) | -408 |
| Other functions that compile to a different size (87 functions; for example `DrawNoodleStripChunk` -256, spram's `Scale`/`Translate` -232/-224, `RenderChunkEECore` -184, the SPA track templates +40 to +160) | about -1,420 |

45 of those 87 functions differ by 8 bytes or less (alignment `nop`s), 40 bytes
in all. No code was dropped. Each original function missing here is a per-file
template copy now served by a shared instance, or a helper whose code is now
inline (the renderee vertex kernels). Reproduce the per-object figures by comparing each
`obj/` object with its `build/src/` counterpart, using symbol gaps for sizes.

## Floating-point precision check (2026-10-07)

On the EE, `double` arithmetic goes through the C library's software helpers
(`dpadd`, `dpsub`, `dpmul`, `dpcmp`, `fptodp`, `dptofp`, `litodp`, `fptoui`). A
`float` expression written with a `double` literal, or the reverse, therefore
shows up as a different set of helper calls. For every function in both
builds, the helper calls in the original objects (`obj/`) match this build's,
once jump-table labels are merged back into their functions. The original
makes 81 such calls in all.

The one difference is `DrawNoodleStripChunk`: the original has 9 `fptoui` call
sites and this build has 3. The original unrolls four vertices times two
coordinates, plus the polygon count; this build loops over the vertices. The
number of conversions at run time is the same. So the reconstruction keeps the
original's float/double choices, which a port should keep too: for example
`TSJukeCDObj_Draw` and `TsPopCusPut` compute in `double` exactly where the
original does.

## Callee check (2026-10-07)

`tools/dev/audit/callee_sets.py` compares every function's callees with the
original's, ignoring nalib template instances and per-file helper copies. Only
four functions differ, all explained:

| Function | Difference | Why |
| --- | --- | --- |
| `CalculateCurrentMatrixAnimation`, `CalculateClusterMatrixAnimation` | This build calls `ComposeGlobalMatrixWithoutVisibility` | The original inlines it (see the ROM shortfall above) |
| `RenderNodeEECore` | The original calls `PrRenderStuff::AppendDmaTag` | The original's per-file copy of the `renderstuff.h` helper; this build inlines it |
| `TsBGMStop` | The original calls `tsBGMONEVol` | This build tail-calls it with a jump, which the script does not count |

No function in this build calls something the original does not, or skips a
call the original makes.

## Float constant check (2026-10-07)

`tools/dev/audit/float_constants.py` compares every function's float constants
(immediates and `.lit4`/`.rodata` loads) with the original's. Five functions
differ, none in game logic:

- **`ApplyDepthOfField`:** both builds load the same NaN bit pattern, which
  never compares equal to itself.
- **Matrix template instances (`spram.cpp`'s `Scale`/`Translate`, `spadata.cpp`'s
  `ScaleMatrix`/`TranslateMatrix`):** the original's per-file copies build the
  matrix inline, while this build calls the shared constructor.

The check found one naming slip, fixed in `TsNAMEINBox_Draw`: the original keeps
the blink period as the integer `ltim = 12` and divides by `(float)ltim`.

## Data sections against the original (2026-10-07)

objdiff reports 99.60% of data bytes matched. That gap comes from how data
symbols and their relocations pair between the builds; it is not different
bytes. The linked executable's data, compared word by word with
`SCPS_150.17`:

| Section | Result |
| --- | --- |
| `.data` (2,188,752 bytes) | 546,032 words identical; 975 are data pointers moved by exactly -0x1300 with the code shift; 181 are code pointers to the same function and offset |
| `.sdata` (1,752) | identical apart from 25 data pointers (-0x1300) and 1 code pointer |
| `.lit4` (972) | identical: every float literal |
| `.vutext` (10,320) | identical: every VU1 microprogram |
| `.rodata` | 12 bytes longer from alignment padding; the same 448 strings and the same 1,490 jump-table entries per function |

The code-pointer comparison resolves both values to a function and offset. Of
the 181, 12 point at SDK or C-library routines that this build names
`func_<original address>`. Those names encode the original's pointer value, so
they are the same function.

