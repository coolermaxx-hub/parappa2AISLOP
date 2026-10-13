# Matching notes (claude/rom-matching-100)

Findings from converting the remaining `INCLUDE_ASM` functions to C on this
branch. "Off by N" counts differing instructions after normalising branch
targets and relocations, with every `NON_MATCHING` block of the unit in C.

## Matched

| Function | Key to the match |
| --- | --- |
| `TsUserList_SetCurTag` (menusub.c) | The date copies go through a pointer to the source field |
| `PrUpdateMendererSpeed` (menderer.cpp) | Both clamps `goto` their store |
| `SpmComplexNode::SaveContour` (contour.cpp) | Source vectors addressed as `(m_src << 4) + (u_int)src` |

## spadata.cpp: emission order

All 14 asm functions in this unit are blocked mainly by the order gcc 2.95
emits them, and the original order is fully explained by two rules:

1. Pending template instances are emitted at the end of the unit, grouped by
   first reference, each function's callees before it.
2. Deferred inline functions that were not inlined are emitted after all
   template instances, in reverse order of when they were deferred.

The original order (`GetLinearValue<VEC>`, `GetValue<VEC>`, `Set` 9-arg,
`Set` 16-arg, `RotateMatrix(vec)`, the `float` and `NaMATRIX` track
instances, then `TranslateMatrix(vec)`, `RotateMatrix(int)`,
`ScaleMatrix(vec)`, and last `GetSprineValue<VEC>`) follows if:

- `TranslateMatrix(const NaVECTOR&)`, `RotateMatrix(int, const float&)` and
  `ScaleMatrix(const NaVECTOR&)` are `inline` in namatrix.h (tested: the
  three then land last, in the original order);
- the `GetSprineValue<NaVECTOR>` specialisation is `inline`, defined before
  `SpaTransform::GetMatrix`, and has a real `static NaVECTOR<float, 4>`
  local. A function-scope static with a constructor stops gcc 2.95 from
  inlining it, and the guard it generates is `_$tmp_0.N`, which is the
  `tmp_0_1744` word at the start of spadata's .sdata. The other `tmp_0_*`
  guards and `*_tmp_spadata_*` buffers are likewise function statics.

So the unit needs its .sdata/.sbss/.bss owned by C (real statics instead of
the `extern` placeholders) as well as body fixes: `RotateMatrix(vec)` (96
off), the `NaMATRIX` spline/linear instances (186/58), `GetLinearValue<VEC>`
(28), `TranslateMatrix(vec)` (20), `RotateMatrix(int)` (44). The `float`
instances and `ScaleMatrix(vec)` are within 2 instructions.

## Near misses

| Function | Off by | Notes |
| --- | --- | --- |
| `SetNextTarget` | 6 | The original compares and negates a copy of `dx`; 20 abs-helper shapes leave it unchanged |
| `Render` | 10 | Declaring `xyoffset` before `frame` gets from 14 to 10; the `AwfulStatus` store is duplicated into both loop-entry paths in the original |
| `GetAwfulRotation` | 10 | The original shares the rotation store between the status 1 path and the status 3 clamp |
| `PrUpdateAwfulMenderer`, `PrCreateMendererTexture` | 14, 36 | The original keeps a second copy of the wrapped angle/phase; same idiom as `GetSynchronizeRatio`'s `NaAbs` |
| `RenderClusterNode` | 24 | `t2`/`t3` swap between the second packet and its matrix address |
| `ApplyDepthOfField` | 24 | Loading `zbuf` after `Init` gets from 28 to 24. The original loads `zbuf` early and computes the packet address for `Init` separately from the one it keeps in `s1`. Its 8 weak helper copies already land exactly when it is C |
| `UpdateNoodleRotation` | 43 (mini harness) | Same block layout as `GetAwfulRotation`; the stage 6 path must not cross-jump into the final add |

## Tools

See `tools/dev/permuter/README.md`. Permuter results need review: it lowers
the score by reading uninitialised variables or adding dead branches.
