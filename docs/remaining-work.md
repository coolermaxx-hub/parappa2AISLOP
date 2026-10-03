# Remaining work (audit of 2026-10-03, after the readability pass)

Snake's definition of 100% (see [porting-rules.md](porting-rules.md)) counts matching VU asm as finished source and only asks for C where the original was compiler-generated EE code. Since 2026-10-03 the code also has to read like the original developers' C++ (see [handoff.md](handoff.md#ground-rules-from-snake-the-owner)): a clean body that misses the match is kept as `NON_MATCHING`, and asm stays in the matching build. Counts come from `INCLUDE_ASM` lines in `src/` (excluding `src/prlib/old`).

| Bucket | Functions | Meaning |
| --- | --- | --- |
| Intentional VU / hand asm | 5 | Treated as done. Not converted to C. |
| NON_MATCHING C bodies | 47 | Reconstructed in clean C/C++, bytes still differ. Asm is used in the matching build. |
| Weak template copies still asm | 50 | The template has a real generic definition in a header; this TU's copy of it is still asm. |

## Intentional VU / hand asm (done by rule)
| File | Function | Insns |
| --- | --- | --- |
| `prlib/renderee.cpp` | `RenderVertexEECoreBothface__13PrRenderStuff` | 42 |
| `prlib/renderee.cpp` | `RenderVertexEECoreNormal__13PrRenderStuff` | 50 |
| `prlib/renderee.cpp` | `RenderNodeEECore__13PrRenderStuffP23PrVuNodeHeaderDmaPacket` | 205 |
| `prlib/renderee.cpp` | `RenderChunkEECore__13PrRenderStuffP25PrVuDataChunkPacketHeaderf` | 197 |
| `prlib/renderee.cpp` | `func_00146A08` | 2 |

## Weak template copies still asm
Each of these is a copy of a template member (NaMATRIX, NaVECTOR, SpaTrack, NaGifPacket) that has a real `template <...>` definition in its header. The copies stay asm for one of these reasons:

- **Emission order.** gcc emits implicit template instances at the end of the TU, after everything else, while `INCLUDE_ASM` lands in place. So once one copy in a TU's tail has to be asm, every copy after it has to be asm too. `extern template` declarations (under `#ifndef NON_MATCHING`) stop the C instantiation, so calls go to the asm copy by its mangled name. Each file's tail is wrapped in `#ifndef NON_MATCHING` with a comment saying which copy blocks it.
- **`RotateMatrix(int)`** (billboard, camera, menderer, spadata): the generic definition in `namatrix.h` compiles to the same instructions, but the original pads its last case to 8 bytes. That padding is the only difference; it was previously forced with `do { } while (0)` tricks, which were removed.
- **`ScaleMatrix(x, y, z)` / `Scale` / `TranslateMatrix(x, y, z)`** (menderer, spram): `ScaleMatrix` is 38 lines off at best; the target computes the literal temporaries' addresses after setting the loop counter.
- **Vector `RotateMatrix`** (camera, spadata): 74 lines off; the target evaluates the constant temps early.
- **SpaTrack members** (spadata): the generic `GetValue`, `GetLinearValue` and `GetSprineValue` for `float`, `NaVECTOR` and `NaMATRIX` are byte-identical with `NON_MATCHING` defined, but they come after `RotateMatrix(int)` in the tail.
- **render.cpp**: the tail is the 4-argument NaVECTOR constructor, `ComposeGlobalMatrix`, `PrRenderStuff::AppendDmaTag` (`func_00145E50`) and `ComposeGlobalMatrixWithoutVisibility`, which is 6 lines off.
- **depthfield.cpp**: needs `ApplyDepthOfField`.

The `func_XXXXXXXX` names are copies whose first instance lives in another TU (the linker binds every call to the first copy, so the original ELF has no symbol for later ones). They are only labels on asm, never C helpers. `mips-linux-gnu-nm -n iso/SCPS_150.17` names the first copies.

| File | Function | Insns |
| --- | --- | --- |
| `prlib/billboard.cpp` | `func_0014C4E8` | 22 |
| `prlib/billboard.cpp` | `func_0014C540` | 44 |
| `prlib/billboard.cpp` | `func_0014C5F0` | 190 |
| `prlib/camera.cpp` | `func_00153AD0` | 22 |
| `prlib/camera.cpp` | `func_00153B28` | 44 |
| `prlib/camera.cpp` | `func_00153BD8` | 170 |
| `prlib/depthfield.cpp` | `AddGifPackedAD_TEXFLUSH__11NaGifPacket` | 4 |
| `prlib/depthfield.cpp` | `AddGifPackedAD_TEST_1__11NaGifPacketbiUcibibi` | 22 |
| `prlib/depthfield.cpp` | `OpenGifTag__11NaGifPacketUI80` | 2 |
| `prlib/depthfield.cpp` | `AddGsAD__18NaGifPacketWrapperUiUl` | 2 |
| `prlib/depthfield.cpp` | `CloseGifTag__18NaGifPacketWrapper` | 2 |
| `prlib/depthfield.cpp` | `OpenGifTag__18NaGifPacketWrapperUI80` | 2 |
| `prlib/depthfield.cpp` | `End__18NaGifPacketWrapperUiUiUi` | 2 |
| `prlib/depthfield.cpp` | `Init__18NaGifPacketWrapperPUI80` | 2 |
| `prlib/menderer.cpp` | `func_0014F3B8` | 22 |
| `prlib/menderer.cpp` | `func_0014F410` | 44 |
| `prlib/menderer.cpp` | `func_0014F4C8` | 66 |
| `prlib/menderer.cpp` | `func_0014F5D0` | 66 |
| `prlib/menderer.cpp` | `func_0014F6D8` | 190 |
| `prlib/mendererawful.cpp` | `Set__t8NaMATRIX3Zfi2i2RCfT1T1T1` | 10 |
| `prlib/mendererawful.cpp` | `Set__t8NaMATRIX3Zfi2i2RCfT1T1T1T1T1T1T1T1` | 22 |
| `prlib/mendererdata.cpp` | `func_00151D78` | 10 |
| `prlib/mendererdata.cpp` | `func_00151DA0` | 22 |
| `prlib/mendererdata.cpp` | `func_00151DF8` | 10 |
| `prlib/render.cpp` | `__t8NaVECTOR2Zfi4RCfT1T1T1` | 10 |
| `prlib/render.cpp` | `func_00145E50` | 2 |
| `prlib/render.cpp` | `ComposeGlobalMatrixWithoutVisibility__7SpmNodeP13PrModelObjectRCt8NaMATRIX3Zfi4i4` | 222 |
| `prlib/scene.cpp` | `func_0014B988` | 10 |
| `prlib/scene.cpp` | `func_0014B9B0` | 10 |
| `prlib/spadata.cpp` | `GetLinearValue__Ct8SpaTrack1Zt8NaVECTOR2Zfi4Uif` | 58 |
| `prlib/spadata.cpp` | `GetValue__Ct8SpaTrack1Zt8NaVECTOR2Zfi4f` | 76 |
| `prlib/spadata.cpp` | `func_00149168` | 22 |
| `prlib/spadata.cpp` | `func_001491C0` | 44 |
| `prlib/spadata.cpp` | `RotateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4RCf` | 170 |
| `prlib/spadata.cpp` | `GetSprineValue__Ct8SpaTrack1ZfUif` | 52 |
| `prlib/spadata.cpp` | `GetLinearValue__Ct8SpaTrack1ZfUif` | 24 |
| `prlib/spadata.cpp` | `GetValue__Ct8SpaTrack1Zff` | 76 |
| `prlib/spadata.cpp` | `GetSprineValue__Ct8SpaTrack1Zt8NaMATRIX3Zfi4i4Uif` | 956 |
| `prlib/spadata.cpp` | `GetLinearValue__Ct8SpaTrack1Zt8NaMATRIX3Zfi4i4Uif` | 274 |
| `prlib/spadata.cpp` | `GetValue__Ct8SpaTrack1Zt8NaMATRIX3Zfi4i4f` | 76 |
| `prlib/spadata.cpp` | `TranslateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4` | 66 |
| `prlib/spadata.cpp` | `RotateMatrix__t8NaMATRIX3Zfi4i4iRCf` | 190 |
| `prlib/spadata.cpp` | `ScaleMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4` | 52 |
| `prlib/spram.cpp` | `Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1` | 44 |
| `prlib/spram.cpp` | `Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1` | 22 |
| `prlib/spram.cpp` | `_GLOBAL_$I$Initialize__12PrSPRAM_DATAP13PrSceneObject` | 20 |
| `prlib/spram.cpp` | `Translate__t8NaMATRIX3Zfi4i4RCfT1T1` | 96 |
| `prlib/spram.cpp` | `Scale__t8NaMATRIX3Zfi4i4RCfT1T1` | 98 |
| `prlib/spram.cpp` | `ScaleMatrix__t8NaMATRIX3Zfi4i4RCfT1T1` | 66 |
| `prlib/spram.cpp` | `TranslateMatrix__t8NaMATRIX3Zfi4i4RCfT1T1` | 66 |

## NON_MATCHING C bodies by file
- `menu/menusub.c`: 12
- `prlib/menderer.cpp`: 5
- `prlib/mendererawful.cpp`: 4
- `prlib/menderercreate.cpp`: 4
- `prlib/mendereralpha.cpp`: 3
- `prlib/render.cpp`: 3
- `main/drawctrl.c`: 2
- `menu/p3mc.c`: 2
- `prlib/contour.cpp`: 2
- `prlib/mendererdata.cpp`: 2
- `prlib/scene.cpp`: 2
- `prlib/camera.cpp`: 1
- `prlib/cluster.cpp`: 1
- `prlib/depthfield.cpp`: 1
- `prlib/shape.cpp`: 1
- `prlib/spram.cpp`: 1
- `prlib/transition.cpp`: 1

### Moved to NON_MATCHING by the readability pass
These matched before but only through hacks that the new rules ban:
- `menu/p3mc.c` `_P3MC_mainfile_chk`: the match needed a `do { } while (0)` around the icon checks (register allocation).
- `prlib/contour.cpp` `SpmComplexNode::SaveContour`: indexing the vertex array puts the base register first in two `addu`s; the original added the shifted index to the packet address as integers.
- `menu/menusub.c` `TsUserList_SetCurTag`: the original copies the timestamp through a pointer; a plain struct copy of `P3MC_DATE` folds the address into the loads.
- prlib: 16 functions, mostly the template copies listed above, which were previously emitted from C through asm-label aliases, unused anchor inlines and per-TU explicit specializations.

### Known leftovers
- `prSpramData_tmp_contour`, `_shape` and `_menderer` are still externs to asm data. Each file's `.sdata` holds more than the pointer: contour and shape also hold an unnamed `FLT_MAX` literal that only their (NON_MATCHING) render functions emit, and menderer holds the noodle state, including `lastRatio`, a static local of the asm `DrawMenderer`. They can become file statics once those functions match.
- `*(u_long *)&gs_register_struct` is left as is: it is how the SCE libraries and samples turn `sceGs*` register structs into 64-bit GIF data, and the SDK has no union for it.
- Code inherited from upstream still has a few of its own matching tricks (for example the `do {} while (0)` in `TsBGMStop`, which blocks a sibling call, and the `volatile` frame in `DrawMozaikuDisp`). They were left alone.
