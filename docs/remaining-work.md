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
2. Explain the purpose of the chunk's two reserved transport quadwords and
   remaining reserved/unknown SPM, animation and render fields. The
   [SPM hierarchy and deformation lists](spm-geometry-layout.md) now have
   evidence-backed names and typed variable-length records. The
   [SPA node tables](spa-animation-layout.md) now have typed boundary binding
   and named visibility, transform and shape-weight fields. The
   [packet audit](spm-packet-layout.md) identifies the clipping GIF template and
   current/previous contour pairs in 43 built-in models; broader stage asset
   coverage remains open. The VIF follow-up verifies that the two zero slots
   are uploaded but skipped by the traced header consumers; it does not assign
   them invented semantics. Standalone model audits now validate VIF boundaries
   and contour source positions, with synthetic reflection/antiline coverage.
3. Validate the derived [texture wave model](noodle-texture-model.md) on PS2,
   including ESIN accuracy and degenerate amplitude sums. The amplitude/spatial
   frequency groups are now decoded and named; setup arithmetic has independent
   instruction-driven checks.
4. Audit and explain every remaining binary discrepancy. Every differing unit
   in [binary-differences.md](binary-differences.md) now has a stated cause;
   most are functions that never matched as C, and a few causes are inferred
   from the source change rather than traced. The main-ROM checksum currently
   fails. Do not restore compiler steering or change expected hashes.
5. Continue the readability audit of existing source outside the reconstructed
   functions and old hardware interfaces. Copy/assignment now use proper generic
   vector/matrix types, with real EE float4 MMI specializations; small-matrix
   identity checks use their own type. Basic scalar and component-wise arithmetic
   now also respects template types and dimensions while retaining the float4 VU
   backend. Matrix/vector products now use the template's dimensions and element
   type, with the existing float4 VU code isolated in explicit specializations;
   see [product validation](behavior-notes.md#nalib-matrix-products-2026-10-04).
   `Inverse` is now documented as the SDK's rigid inverse
   ([notes](behavior-notes.md#matrix-inverse-is-a-rigid-inverse-2026-10-05));
   transform-only APIs and other hardware interfaces remain to audit.

The supplied OLM overlays do not contain recognizable SPM records. The retained
original executable and its built-in `common.ipk` support layout/disassembly
analysis and real model packet checks. Rendering equivalence still needs
original PS2 execution, and the packet audit does not cover all stage models.
No runtime rendering equivalence has been claimed.

## Current exact-match measurement

The 2026-10-05 objdiff report (`progress/report.json`, regenerated with
`./configure.py --objdiff` and `objdiff-cli report generate`) measures
**1311 / 1429 exact functions (91.74248%)** and **259104 / 342284 exact code
bytes (75.69854%)** across 70 units, with a fuzzy instruction score of
94.13218%. By folder: dbug 21/21, os 100/100, iop_mdl 4/4, main 563/570,
menu 355/374, prlib 268/360. README and the badge files in `progress/` show the
same report.

The clean build links both ROMs. The unchanged IOP checksum passes; the unchanged
main-ROM checksum fails. These figures include the accumulated readability
changes and do not imply execution equivalence for the new source. Function
matching, source reconstruction and ROM checksums are separate measures. Only
verified semantic aliases are used by the mapping tool.

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
in `TsOption_Flow` (which matches again without it: the arrow timer is written through
`(pfw->btnlr + osel)->tim[...]` after the option count is read). The whole `main/mbar` data section is now typed C in `mbar.c` (texture
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
  `GS_ALPHA_FIXED`, `GS_ALPHA_ADD`, `GS_ALPHA_SUBTRACT`, `GS_CLAMP_EDGES`,
  `GS_PRIM_TEX_SPRITE`, `GS_FRAME_TEX0`),
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
- Every file of the ELF's built-in `common.ipk` (`CMNF_FILE_ENUM` in
  `main/cmnfile.h`), named from the model and animation names the files carry,
  or from their use (the wipe's sound bank and subtitle glyphs, meter palettes,
  hook marks); SPU volumes (`SPU_VOLUME_MAX`, `SPU_VOLUME_LR`) and the wipe's
  TapCtrl bank and voice.
- The memory card layer: `P3MC_SAVE_*` / `P3MC_LOAD_*` sequencer states,
  `P3MC_RES_*` results, `P3MC_FLAG_*`, save kinds (`P3MC_MODE_LOG/REPLAY`),
  file check results, `MEMC_ERR_*` codes at every call site, and the menu-side
  flow states and results (`MCUSER_*`, `MCUCHK_*`, `MCFLOW_*`, `MCCHECK_*`).
- Menu message numbers (`MCMES`, kinds), text placement flags (`MNFONT_*`, shared
  in `menu/menufont.h`), scene animation commands and timer banks (`MNANM_*`,
  `MN_SCENE_BANKS`, bank sets `MNANM_BANK_SET`), user-name character packing, stage indices
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

Modern compilers: every EE C and C++ source now parses with a 64-bit host
gcc/g++ (`-D__IEEE_LITTLE_ENDIAN -D__R5900__`, plus `-fpermissive` for C++
pointer casts). The old multi-line asm strings in `nalib/navector.h`,
`nalib/namatrix.h`, `prlib/spadata.cpp` and `prlib/shape.cpp` became standard
literals, a cast used as an lvalue in `TsCELBackDraw` became two statements, and
the nalib static constants use `template <>`. In inline header functions the asm
text stays one string literal continued with backslash-newline: ee-gcc 2.95
garbles concatenated asm strings there (`spram.cpp` failed to assemble). The
pointer-size casts that remain are listed in
[behavior-notes.md](behavior-notes.md#32-bit-pointer-assumptions-2026-10-04).

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
- `TsGetTm2Tex` and `TsGetTm2HedTex` (src/menu/menusub.c) read TW and TH and
  set TCC through the `sceGsTex0` fields instead of shifting the two 32-bit
  halves of the 64-bit TEX0 word. Same bits; the compiler now works on the
  whole word.
- `PkTEX0_SetAdd` (src/menu/pksprite.c) builds its TEX0 value with
  `SCE_GS_SET_TEX0` instead of writing the low and high 32-bit halves of the
  register word separately. Same bits for valid (non-negative) inputs; the
  compiler now does the work in 64-bit registers. The rest of the sprite
  packet builder is typed (`PKGIFTAG`, `PK_AD_PACKET`) with unchanged code.
- Two small functions lost temporaries that existed only to steer register
  allocation, neither of which the original debug info lists:
  `MbarGetDispPosY` (src/main/mbar.c; a `v0` local and a `0x1df - 1` that was
  incremented again before use) and `drawDispCheckSub` (src/main/drawctrl.c; a
  `v0` local). Same results; registers and one conditional move differ.
  (`subjobEvent` had the same kind of temporary; it matches again since its
  reverse-playback job computes `time_tmp` in two steps and its sound-transfer
  job reads a typed `SCRDAT`, as the original debug info shows.)
- `ScrExamSetCheck` (src/main/scrctrl.c) declares the replay score buffer
  `mcr_scr` inside the block that fills it, where the original debug info
  places it. The old function-scope declaration only reproduced the original
  stack frame; the frame is now 16 bytes smaller and the spill slots move.

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
rather than a placeholder variable: 16 bytes in `.data` where the `napacket`
object used to be, 0xAA0 bytes of `.bss` after `os/tim2` (the original `tim2`
bss is 2720 bytes larger than its five statics) and 0x1E28 bytes of `.bss`
after `main/mbar` (up to `pack`'s buffer at 0x1c72000). Neither bss range has a
symbol, a debug-info entry or a code reference in the original; the contents
are unknown.

The 8 bytes that used to be padded after the `drawctrl` `.sdata` are the
variable `ANI_BLUMOVE_ENUM`: the original declares its animation-blend enum
as `enum { ... } ANI_BLUMOVE_ENUM;` without `typedef`, which defines an unused
global that its linker put after the file's small data. `mbar.c`'s
`SCR_TENMETU_ENUM` is the same slip. Both are defined last in their files.

`tools/dev/audit/layout_orig.py` compares every data symbol's offset with the
original executable. `.data`, `.sdata`, `.sbss` and `.bss` agree, except that
`common_ipk_end` sits 4 bytes late: the extracted asset includes the 4 zero
bytes of alignment that follow the archive. Nothing references that label.
Two `.bss` drifts were found and fixed this way (bss is not in the ROM image,
so image comparisons cannot see it): the `mbar` gap above was missing, and
`screenClipMatrix` / `screenPrimitiveMatrix` (`prlib/spram.cpp`) had lost their
16-byte alignment, which their `lq`/`sq` copies need.

The `spadata` template layout is deliberate: `SpaTrack` accessors are defined
once in `spadata.cpp` so that only that unit owns their function-local statics,
as in the original image. Matrix spline interpolation is the generic template
and the vector and float versions are specializations, which reproduces the
original `.bss` order.

`stdat_dat_*` overlay pointers (event, scene and subtitle records inside the
stage overlays) are written as fixed addresses because those objects live in
the separately loaded `.OLM` overlays and have no symbols in this executable.
