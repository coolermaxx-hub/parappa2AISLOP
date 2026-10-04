# Binary differences

Function-level differences between this tree and the original-compiler objects (`expected2/`), measured by `tools/dev/audit/alldiff.py`.
Every difference comes from replacing compiler-steering or alias source with typed C/C++. None is a behaviour change by intent, but the original instruction order is not kept.
Functions marked *not yet classified* differ and nobody has traced the cause yet; treat them as open audit items.

Total: 92 functions differ, 33 original symbols have no counterpart (mostly orphan `func_XXXXXXXX` helpers and brute-forced template copies that were removed on purpose).

| Unit | Differing functions | Original symbols not reproduced | Cause |
| --- | --- | --- | --- |
| `nalib/navector.cpp.o` | 1 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `menu/menu.c.o` | 1 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `menu/p3mc.c.o` | 3 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `menu/menusub.c.o` | 14 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `main/wipe.c.o` | 1 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `main/drawctrl.c.o` | 3 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `main/scrctrl.c.o` | 1 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/scene.cpp.o` | 2 | 2 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/setpointer.cpp.o` | 2 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/spadata.cpp.o` | 14 | 5 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/cluster.cpp.o` | 1 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/renderee.cpp.o` | 5 | 1 | Typed reconstruction without compiler steering; not yet classified. |
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
| `prlib/random.cpp.o` | 1 | 0 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/depthfield.cpp.o` | 3 | 1 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/prlib.cpp.o` | 1 | 1 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/billboard.cpp.o` | 2 | 3 | Typed reconstruction without compiler steering; not yet classified. |
| `prlib/model.cpp.o` | 1 | 1 | Typed reconstruction without compiler steering; not yet classified. |

Regenerate with `python3 tools/dev/audit/alldiff.py && python3 tools/dev/audit/gen_binary_diff_doc.py`.
