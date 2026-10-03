# Remaining work

The active build has no compiler-generated `INCLUDE_ASM` fallbacks and no
`NON_MATCHING` switches. The two remaining includes are handwritten vertex
kernels. See [source-reconstruction.md](source-reconstruction.md) for the current
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
in `TsOption_Flow`. Still open: the `*_tmp_NNN` externs in `menusub.c` and
`mbar.c`, which stand for file-static variables that still live in splat data
sections, and the `*(u_long *)&sceGs...` register-struct reads used to feed GIF
packets.
