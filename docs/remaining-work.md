# Remaining work

The active build has no `INCLUDE_ASM` fallbacks and no `NON_MATCHING`
switches. The two handwritten vertex kernels are inline VU0 assembly in
`src/prlib/renderee.cpp`. See [source-reconstruction.md](source-reconstruction.md) for the current
implementation and validation; the earlier [matrix pass](matrix-reconstruction.md)
is retained as historical context.

This does not establish 100% decompilation. Compilation and a fallback count
cannot prove original behavior or complete understanding of the data formats.

## Required before claiming completion

1. Validate reconstructed camera/animation/geometry and noodle rendering against
   original PS2 execution. Generic MIPS scalar tests cannot exercise COP2,
   pipeline transfer delays, VU R randomness, DMA ordering or GS drawing.
2. Decode the chunk's three currently uninterpreted transport quadwords and
   remaining reserved/unknown SPM, animation and render fields. The
   [SPM hierarchy and deformation lists](spm-geometry-layout.md) now have
   evidence-backed names and typed variable-length records. The
   [SPA node tables](spa-animation-layout.md) now have typed boundary binding
   and named visibility, transform and shape-weight fields. Explain the
   distinct contour save/render index origins and test real model assets.
3. Validate the derived [texture wave model](noodle-texture-model.md) on PS2,
   including ESIN accuracy and degenerate amplitude sums. The amplitude/spatial
   frequency groups are now decoded and named; setup arithmetic has independent
   instruction-driven checks.
4. Audit and explain every remaining binary discrepancy. The main-ROM checksum
   currently fails. Do not restore compiler steering or change expected hashes.
5. Continue the readability audit of existing source outside the reconstructed
   functions and old hardware interfaces. Copy/assignment now use proper generic
   vector/matrix types, with real EE float4 MMI specializations; small-matrix
   identity checks use their own type. Basic scalar and component-wise arithmetic
   now also respects template types and dimensions while retaining the float4 VU
   backend. Matrix products and other hardware interfaces remain to audit.

The supplied OLM overlays do not contain recognizable SPM records. The retained
original executable and data templates support layout/disassembly analysis;
rendering equivalence still needs matching game model assets and PS2 execution.
No runtime rendering equivalence has been claimed.

## Current exact-match measurement

The 2026-10-03 objdiff report measured **1325 / 1429 exact functions (92.72218%)**
and **264612 / 342284 exact code bytes (77.30773%)** across 70 units. Its fuzzy
instruction score was 93.18579%. The full report is retained outside the checkout
at `/workspace/shared/parappa-env/animation-report.json`.

The clean build links both ROMs. The unchanged IOP checksum passes; the unchanged
main-ROM checksum fails. These figures include exact retained handwritten kernels
and do not imply execution equivalence for the new source.

Function matching, source reconstruction and ROM checksums are separate measures.
The saved historical badges in README are not this reconstruction's measurement.
Only verified semantic aliases are used by the mapping tool.

## Handwritten kernels retained

| Class operation | Original instructions | Role |
| --- | ---: | --- |
| `PrRenderStuff::RenderVertexEECoreBothface` | 42 | Projection, clipping and packed GS output |
| `PrRenderStuff::RenderVertexEECoreNormal` | 50 | Projection, clipping and strip face rejection |

Both are now written as inline `asm volatile` blocks that take typed pointers
to the staged input and output vertices (`PrEECoreContext`). The instruction
words are identical to the original; the compiler adds only the pointer setup
that the handwritten code did inline, so they are not exact function matches.
The one instruction the assembler cannot encode (`vclipw`) is emitted as its
raw word with a comment.

`RenderNodeEECore` and `RenderChunkEECore` were incorrectly listed as intentional
assembly in the earlier audit. Their compiler-generated orchestration is now C++
with typed hardware operations. The old two-instruction queue aliases are served
by the actual `AppendDmaTag` class member rather than address-named helpers.

## Menu and main-loop readability pass (2026-10-03)

Inherited upstream code outside the reconstructed engine had its own compiler
steering. Removed, with source kept as the active build: the `do { } while (0)`
in `_P3MC_mainfile_chk` and `TsBGMStop`, the gotos in `ScrExamSetCheck`, the
word-casts over save timestamps (now `P3MC_DATE`, whose two words are compared
through `P3MC_DATE_WORD`), the `FILE_DATE` cast struct and the byte-offset cast
in `TsOption_Flow`. The whole `main/mbar` data section is now typed C in `mbar.c` (texture
descriptor table with decoded TEX0 fields, GUI maps, niko/hook layouts), with
byte-identical contents. The remaining
`*(u_long *)&sceGs...` reads in `wipe.c` follow the SDK idiom of submitting a
register struct as one 64-bit GIF A+D value.

## Naming pass over magic numbers (2026-10-04)

Raw constants were replaced by names with no change to the build (the function
diff list stayed at 92 differing / 33 missing after every step, and edits inside
already-differing functions were checked for identical assembly). Covered:

- GS/GIF/VIF register values through the SDK `SCE_GS_SET_*`, `SCE_GIF_*` and
  `SCE_VIF1_*` macros; the draw field size (`SCREEN_WIDTH`, `SCREEN_FIELD_HEIGHT`)
  and the primitive origin (`GS_X_COORD`, `GS_Y_COORD`).
- Recurring GS register values as named macros: game side in `os/cmngifpk.h`
  (`GS_TEST_OFF`, `GS_TEST_ALPHA_NONZERO`, `GS_TEST_COLOR_ONLY`, `GS_ALPHA_BLEND`,
  `GS_ALPHA_FIXED`, `GS_CLAMP_EDGES`, `GS_PRIM_TEX_SPRITE`, `GS_FRAME_TEX0`),
  `GS_TEX1_LINEAR` in `os/tim2.h`, and prlib side in `prlib/gsstate.h`
  (`PR_TEST_*`, `PR_ALPHA_*`, `PR_TEX1_*`, `PR_FRAME_CT32`). TEX0 pixel formats use
  `SCE_GS_PSM*`, DMA tag ids use `PrDmaTagId`. Every read of a libgraph register
  struct as its 64-bit word goes through `GS_REG_WORD` in `common.h`, the reverse
  view through `GS_REG_VIEW`, and two-word GIF tags through `GIF_TAG_QWORD`: the
  places a port has to replace.
- prlib flag words (`SpmFlags`, `SpmFileFlags`, `PrModelFlags`, SPA/SPC file
  flags), GIF register pass modes (`PrSetGifRegisterMode`), the title stage check
  (`PR_STAGE_TITLE`) and object magic values.
- Pad setup phases and terminal types (`syssub`), button masks (`SCE_PAD*`).
- The memory card layer: `P3MC_SAVE_*` / `P3MC_LOAD_*` sequencer states,
  `P3MC_RES_*` results, `P3MC_FLAG_*`, save kinds (`P3MC_MODE_LOG/REPLAY`),
  file check results, `MEMC_ERR_*` codes at every call site, and the menu-side
  flow states and results (`MCUSER_*`, `MCUCHK_*`, `MCFLOW_*`, `MCCHECK_*`).
- Menu message numbers (`MCMES`, kinds), subtitle alignment, scene animation
  commands (`MNANM_*`), user-name character packing, stage indices
  (`STDAT_STAGE_*`) and ending/bonus flags (`ENDING_*`).

- Menu flows in `menusub.c`: the shared `MNFLOW_RUN/INIT/END` argument, and
  named states, entrances and results for every flow: stage map (`TSMAP_*`),
  city hall (`CHALL_*`, camera `CHCAM_*`), map cursor (`MAPMENU_*`, `MAPSND_*`,
  `MNMAP_DIR_*`), save (`MPSAVE_*`, `SAVEMENU_*`), pop-up menus (`POPMENU_*`,
  `POPUP_*`, `POPSEL_*`), options (`OPTMENU_*`), juke box (`JUKE_*`, camera
  `JKCAM_*`), boot card check (`MCSTART_*`, `MCCARD_*`), user list (`ULST_*`,
  results `ULIST_*`) and name entry (`NAMEIN_*`). Play modes use `PLAY_MODE_*`,
  the menu entry reason `SEL_MENU_*`, map positions and scripted cursor paths
  `MAP_POS_*` / `AUTO_MOVE_*`, screen fades `SCFADE_*`, window animations
  `ANIME_*`. The card error selectors passed to `McErrorMess` are `MCERR_*`,
  every message id is an `MCA_*` name and every voice a `VSND_*` name.

Port note: `TsNAMEINBox_Flow` receives the edited name's `char*` through its
`u_int tpad` argument on `MNFLOW_INIT` (original API overloading). A 64-bit port
has to split that into a separate parameter.

## Known byte differences from readability changes

The full per-unit table is in [binary-differences.md](binary-differences.md).

- `PrFloatRandom` (src/prlib/random.cpp) is a plain rejection-sampling
  do-while. The original needed a backward `goto` to match. Behaviour is
  identical (reroll while `value / RAND_MAX >= 1.0f`); only the block layout
  differs.
- `PrSetDebugParamFloat` (src/prlib/prlib.cpp) stores through the `float`
  member of the `PrDebugParam` union instead of reinterpreting the value
  through an `int*`. Same bits; the original code generation is not kept.
- `WipeParaOutDisp` (src/main/wipe.c) no longer carries an `asm(".align 2")`
  inside its loop; the original had one extra nop for loop alignment.
- `MenuCtrl` (src/menu/menu.c) no longer pins a local to `a1` with a
  register variable; the original used different temporaries for the same
  stores.
- `DrawMozaikuDisp` (src/main/drawctrl.c) builds its unused masked frame copy
  as a typed struct instead of a volatile `u_long` store.
- `_P3MC_ASC2SJIS` (src/menu/p3mc.c) uses typed `u_short` lookup tables
  (the `menu/p3mc` data section is now C) and writes the two output bytes
  separately instead of one `u_short` store through a `char*`. Same bytes on
  little-endian; the code is a few instructions longer than the original.

## Data sections still in asm

Everything the project's own source defines is now typed C: `.data`, `.sdata`,
`.sbss`, `.bss` and `.rodata` of every `main`, `menu`, `os`, `dbug`, `prlib`
and `nalib` object. Each conversion was checked against the original layout
(symbol order and addresses, and byte contents apart from relocation words).
The asm objects that remain are not project source:

- `sdk/*`: the vendor library data and `libgcc` runtime tables.
- `common_ipk`: a binary archive kept as a data asset.
- `gcc_except_table.rodata`: compiler-generated C++ exception tables.
- `299300.sdata`: a pointer to `__main` from the C++ runtime start-up.
- `prlib` `lit4`: float literals, handled by `tools/buildtools/lit4fix.py`.

Padding that no source construct explains is an explicit slinky `pad` entry
rather than a placeholder variable: 8 bytes after the `drawctrl` `.sdata`,
16 bytes in `.data` where the `napacket` object used to be, and 0xAA0 bytes of
`.bss` after `os/tim2` (the original `tim2` bss is 2720 bytes larger than its
five statics; the contents are unknown).

The `spadata` template layout is deliberate: `SpaTrack` accessors are defined
once in `spadata.cpp` so that only that unit owns their function-local statics,
as in the original image. Matrix spline interpolation is the generic template
and the vector and float versions are specializations, which reproduces the
original `.bss` order.

`stdat_dat_*` overlay pointers (event, scene and subtitle records inside the
stage overlays) are written as fixed addresses because those objects live in
the separately loaded `.OLM` overlays and have no symbols in this executable.
