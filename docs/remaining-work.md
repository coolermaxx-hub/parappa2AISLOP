# Remaining work (audit of 2026-10-03, updated after RotateMatrix(int))

Snake's definition of 100% (see [porting-rules.md](porting-rules.md)) counts matching VU asm as finished source and only asks for C where the original was compiler-generated EE code. Measured against that, this is what is left. Counts come from `INCLUDE_ASM` lines in `src/` (excluding `src/prlib/old`).

| Bucket | Functions | Meaning |
| --- | --- | --- |
| Intentional VU / hand asm | 5 | Treated as done. Not converted to C. |
| NON_MATCHING C bodies | 49 | Reconstructed in C, bytes still differ (scheduling or register allocation). Asm is used in the matching build. |
| Compiler-generated, no C yet | 29 | All are weak template copies, see below. |

## Intentional VU / hand asm (done by rule)
| File | Function | Insns |
| --- | --- | --- |
| `prlib/renderee.cpp` | `RenderVertexEECoreBothface__13PrRenderStuff` | 42 |
| `prlib/renderee.cpp` | `RenderVertexEECoreNormal__13PrRenderStuff` | 50 |
| `prlib/renderee.cpp` | `RenderNodeEECore__13PrRenderStuffP23PrVuNodeHeaderDmaPacket` | 205 |
| `prlib/renderee.cpp` | `RenderChunkEECore__13PrRenderStuffP25PrVuDataChunkPacketHeaderf` | 197 |
| `prlib/renderee.cpp` | `func_00146A08` | 2 |

## Compiler-generated, no C body yet
These are all weak template copies (NaMATRIX/NaVECTOR/NaGifPacket helpers instantiated per TU). A TU emits its copies at the end, in the order the templates were first used, so a copy can only come from C once the TU's own code (or an unused inline standing in for it, see `scene.cpp`) marks the same templates in the same order, and every helper body matches.

Done so far: `RotateMatrix(int)` matches and has a generic definition in `nalib/namatrix.h`; `billboard.cpp`, `scene.cpp`, `mendererdata.cpp` and `mendererawful.cpp` emit their copies from C. What blocks the rest:
- `menderer.cpp`, `spram.cpp`: `ScaleMatrix(const float&, const float&, const float&)` is still 44 lines off. `TranslateMatrix` with the same signature matches as an in-class `return NaMATRIX<float, 4, 4>(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1);`. `spram.cpp` also needs `Initialize`.
- `camera.cpp`: needs `RotateMatrix(const NaVECTOR<float, 4>&, const float&)` to match.
- `spadata.cpp`: its template functions are explicit specializations that are emitted in place, while the original emitted every one as a weak copy at the end of the TU.
- `depthfield.cpp`: needs `ApplyDepthOfField`.
- `render.cpp`: the NaVECTOR constructor copy sits in the middle of the TU, which is not understood yet.

The original ELF (`iso/SCPS_150.17`) keeps the symbol of the first copy of each instance, so `mips-linux-gnu-nm -n iso/SCPS_150.17` names them.

| File | Function | Insns |
| --- | --- | --- |
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
| `prlib/render.cpp` | `__t8NaVECTOR2Zfi4RCfT1T1T1` | 10 |
| `prlib/render.cpp` | `func_00145E50` | 2 |
| `prlib/spadata.cpp` | `func_00149168` | 22 |
| `prlib/spadata.cpp` | `func_001491C0` | 44 |
| `prlib/spadata.cpp` | `TranslateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4` | 66 |
| `prlib/spadata.cpp` | `ScaleMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4` | 52 |
| `prlib/spram.cpp` | `Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1` | 44 |
| `prlib/spram.cpp` | `Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1` | 22 |
| `prlib/spram.cpp` | `_GLOBAL_$I$Initialize__12PrSPRAM_DATAP13PrSceneObject` | 20 |
| `prlib/spram.cpp` | `Translate__t8NaMATRIX3Zfi4i4RCfT1T1` | 96 |
| `prlib/spram.cpp` | `Scale__t8NaMATRIX3Zfi4i4RCfT1T1` | 98 |
| `prlib/spram.cpp` | `ScaleMatrix__t8NaMATRIX3Zfi4i4RCfT1T1` | 66 |
| `prlib/spram.cpp` | `TranslateMatrix__t8NaMATRIX3Zfi4i4RCfT1T1` | 66 |

## NON_MATCHING C bodies by file
- `menu/menusub.c`: 11
- `prlib/menderer.cpp`: 5
- `prlib/mendererawful.cpp`: 4
- `prlib/menderercreate.cpp`: 4
- `prlib/render.cpp`: 4
- `prlib/spadata.cpp`: 4
- `prlib/mendereralpha.cpp`: 3
- `main/drawctrl.c`: 2
- `prlib/mendererdata.cpp`: 2
- `prlib/scene.cpp`: 2
- `menu/p3mc.c`: 1
- `prlib/camera.cpp`: 1
- `prlib/cluster.cpp`: 1
- `prlib/contour.cpp`: 1
- `prlib/depthfield.cpp`: 1
- `prlib/shape.cpp`: 1
- `prlib/spram.cpp`: 1
- `prlib/transition.cpp`: 1
