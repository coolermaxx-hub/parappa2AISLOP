# Binary differences

Function-level differences between this tree and the original-compiler objects (`expected2/`), measured by `tools/dev/audit/alldiff.py`.
Every difference comes from replacing compiler-steering or alias source with typed C/C++. None is a behaviour change by intent, but the original instruction order is not kept.
Functions marked *not yet classified* differ and nobody has traced the cause yet; treat them as open audit items.

Total: 94 functions differ, 33 original symbols have no counterpart (mostly orphan `func_XXXXXXXX` helpers and brute-forced template copies that were removed on purpose).

| Unit | Differing functions | Original symbols not reproduced | Cause |
| --- | --- | --- | --- |
| `nalib/navector.cpp.o` | 1 | 0 | Set(NaMATRIX3) is the real template. The original image carried a brute-forced copy; that copy is not reproduced. |
| `menu/menu.c.o` | 1 | 0 | MenuCtrl: register pin removed (see remaining-work.md). |
| `menu/p3mc.c.o` | 3 | 0 | Typed tables and `P3MC_DATE` replace word casts and a do-while (see remaining-work.md). |
| `menu/menusub.c.o` | 16 | 0 | Twelve functions were reconstructed from asm and never matched without compiler steering; `main-rom-matching` keeps their original asm under `NON_MATCHING` (TsRestoreSaveData, TsRanking_Set, TsPopMenu_Flow, TsPopMenu_Draw, TsPopMenCus_Draw, TsSaveMenu_Draw, TSJukeCDObj_Draw, TsCmnCell_CusorDraw, TsOption_Flow, TsUserList_SetCurTag, TsUserList_Flow, TsNAMEINBox_Draw). TsBGMStop lost a one-pass do-while (tail-call layout only), TsGetTm2Tex and TsGetTm2HedTex read TEX0 fields through `sceGsTex0` (see remaining-work.md), and TsHosiPut differs only by a trailing alignment nop. |
| `main/wipe.c.o` | 1 | 0 | WipeParaOutDisp: loop-alignment asm removed (see remaining-work.md). |
| `main/drawctrl.c.o` | 3 | 0 | DrawMozaikuDisp uses a typed masked frame (see remaining-work.md). The other two functions are not yet classified. |
| `main/scrctrl.c.o` | 1 | 0 | ScrExamSetCheck: gotos replaced by structured control flow. |
| `prlib/scene.cpp.o` | 2 | 2 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/setpointer.cpp.o` | 2 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/spadata.cpp.o` | 14 | 5 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/cluster.cpp.o` | 1 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/renderee.cpp.o` | 5 | 1 | The two vertex kernels are inline asm blocks instead of standalone functions, and the orphan helper is gone; surrounding functions inherit the shifted register use. |
| `prlib/mendererawful.cpp.o` | 4 | 1 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/transition.cpp.o` | 1 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/mendereralpha.cpp.o` | 3 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/shape.cpp.o` | 2 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/menderer.cpp.o` | 6 | 5 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/spram.cpp.o` | 2 | 5 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/camera.cpp.o` | 1 | 3 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/contour.cpp.o` | 2 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/render.cpp.o` | 7 | 2 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/mendererdata.cpp.o` | 3 | 3 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/menderercreate.cpp.o` | 5 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/random.cpp.o` | 1 | 0 | PrFloatRandom is a plain do-while (see remaining-work.md). |
| `prlib/depthfield.cpp.o` | 3 | 1 | The original carried a local copy of `NaGifPacketWrapper::AddGsAD`; ours calls the shared weak symbol. The copy is a banned brute-forced template. |
| `prlib/prlib.cpp.o` | 1 | 1 | PrSetDebugParamFloat stores through the union member (see remaining-work.md). |
| `prlib/billboard.cpp.o` | 2 | 3 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/model.cpp.o` | 1 | 1 | Typed reconstruction without compiler steering; not yet classified. |

Regenerate with `python3 tools/dev/audit/alldiff.py && python3 tools/dev/audit/gen_binary_diff_doc.py`.
