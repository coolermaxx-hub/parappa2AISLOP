# Remaining work (audit of 2026-10-03)

Snake's definition of 100% (see [porting-rules.md](porting-rules.md)) counts matching VU asm as finished source and only asks for C where the original was compiler-generated EE code. Measured against that, this is what is left. Counts come from `INCLUDE_ASM` lines in `src/` (excluding `src/prlib/old`).

| Bucket | Functions | Meaning |
| --- | --- | --- |
| Intentional VU / hand asm | 5 | Treated as done. Not converted to C. |
| NON_MATCHING C bodies | 58 | Reconstructed in C, bytes still differ (scheduling or register allocation). Asm is used in the matching build. |
| Compiler-generated, no C yet | 37 | Still need a C body. Most are weak template copies, see below. |

## Intentional VU / hand asm (done by rule)
| File | Function | Insns |
| --- | --- | --- |
| `prlib/renderee.cpp` | `RenderVertexEECoreBothface__13PrRenderStuff` | 42 |
| `prlib/renderee.cpp` | `RenderVertexEECoreNormal__13PrRenderStuff` | 50 |
| `prlib/renderee.cpp` | `RenderNodeEECore__13PrRenderStuffP23PrVuNodeHeaderDmaPacket` | 205 |
| `prlib/renderee.cpp` | `RenderChunkEECore__13PrRenderStuffP25PrVuDataChunkPacketHeaderf` | 197 |
| `prlib/renderee.cpp` | `func_00146A08` | 2 |

## Compiler-generated, no C body yet
Weak template copies (NaMATRIX/NaVECTOR/NaGifPacket helpers instantiated per TU) cannot be written in C until the shared template body matches, because the copy is emitted wherever a TU instantiates it. Fixing `NaMATRIX<float,4,4>::RotateMatrix(int, const float&)` (30 differing lines in spadata.cpp, same code in billboard, camera and menderer) is the highest-leverage item.

| File | Function | Insns |
| --- | --- | --- |
| `prlib/billboard.cpp` | `func_0014C4E8` | 22 |
| `prlib/billboard.cpp` | `func_0014C540` | 44 |
| `prlib/billboard.cpp` | `func_0014C5F0` | 190 |
| `prlib/camera.cpp` | `func_00153AD0` | 22 |
| `prlib/camera.cpp` | `func_00153B28` | 44 |
| `prlib/camera.cpp` | `func_00153BD8` | 170 |
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
| `prlib/mendererawful.cpp` | `func_00150D10` | 22 |
| `prlib/mendererdata.cpp` | `func_00151DA0` | 22 |
| `prlib/mendererdata.cpp` | `func_00151DF8` | 10 |
| `prlib/render.cpp` | `__t8NaVECTOR2Zfi4RCfT1T1T1` | 10 |
| `prlib/render.cpp` | `func_00145E50` | 2 |
| `prlib/scene.cpp` | `func_0014B988` | 10 |
| `prlib/scene.cpp` | `func_0014B9B0` | 10 |
| `prlib/spadata.cpp` | `func_00149168` | 22 |
| `prlib/spadata.cpp` | `func_001491C0` | 44 |
| `prlib/spadata.cpp` | `func_0014ABE0` | 66 |
| `prlib/spadata.cpp` | `func_0014AFE0` | 52 |
| `prlib/spram.cpp` | `Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1` | 44 |
| `prlib/spram.cpp` | `func_00147D90` | 22 |
| `prlib/spram.cpp` | `_GLOBAL_` | 0 |
| `prlib/spram.cpp` | `func_00147E38` | 96 |
| `prlib/spram.cpp` | `func_00147FB8` | 98 |
| `prlib/spram.cpp` | `func_00148140` | 66 |
| `prlib/spram.cpp` | `func_00148248` | 66 |

Larger standalone functions:

| File | Function | Insns |
| --- | --- | --- |
| `prlib/menderercreate.cpp` | `PrInitializeTextureCreation__FUiUiUiUi` | 335 |

## NON_MATCHING C bodies by file
- `menu/menusub.c`: 13
- `prlib/menderer.cpp`: 6
- `prlib/mendererawful.cpp`: 5
- `prlib/spadata.cpp`: 5
- `prlib/mendererdata.cpp`: 4
- `prlib/render.cpp`: 4
- `prlib/mendereralpha.cpp`: 3
- `prlib/menderercreate.cpp`: 3
- `main/drawctrl.c`: 2
- `menu/p3mc.c`: 2
- `prlib/depthfield.cpp`: 2
- `prlib/scene.cpp`: 2
- `main/mbar.c`: 1
- `prlib/camera.cpp`: 1
- `prlib/cluster.cpp`: 1
- `prlib/contour.cpp`: 1
- `prlib/shape.cpp`: 1
- `prlib/spram.cpp`: 1
- `prlib/transition.cpp`: 1
