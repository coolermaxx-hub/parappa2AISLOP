"""Regenerate docs/binary-differences.md from alldiff_out.json (run from the repo root)."""
import json
a = json.load(open('tools/dev/audit/alldiff_out.json'))
NM = 'Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these'
WEAK = 'The missing symbols are the original\'s per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template'
causes = {
    'nalib/navector.cpp': 'Set(NaMATRIX3) is the real template. The original image carried a brute-forced copy; that copy is not reproduced.',
    'menu/menu.c': 'MenuCtrl: register pin removed, and the matching draft\'s self-assignment and repeated `pLog` store dropped (see remaining-work.md).',
    'menu/p3mc.c': 'Typed tables and `P3MC_DATE` replace word casts and a do-while (see remaining-work.md). _P3MC_CheckUserDataHead tests the header fields itself instead of through an inline helper that existed only to match (the original debug info has no trace of it); only the layout of its `flg` branch differs.',
    'main/wipe.c': 'WipeParaOutDisp: loop-alignment asm removed (see remaining-work.md). WipeParaInDisp lost the same `asm(".align 2")` at the top of its loop (2026-10-12); it no longer matches, and the extra alignment is not restored because it only steers the compiler.',
    'main/drawctrl.c': 'DrawMozaikuDisp uses a typed masked frame and drawDispCheckSub lost a temporary named after a register (see remaining-work.md). ' + NM + ' (DrawObjStrDisp, DrawObjPrReq: register allocation).',
    'main/scrctrl.c': 'ScrExamSetCheck: gotos replaced by structured control flow; the replay score buffer is declared in its own block, as the original debug info shows, which changes the stack frame layout.',
    'main/mbar.c': 'MbarGetDispPosY picks the row offset with a plain conditional instead of a register-named temporary and an off-by-one constant (see remaining-work.md).',
    'prlib/random.cpp': 'PrFloatRandom is a plain do-while (see remaining-work.md).',
    'prlib/prlib.cpp': 'PrSetDebugParamFloat stores through the union member (see remaining-work.md).',
    'prlib/scene.cpp': NM + ' (the constructor and SetAppropriateDefaultCamera). ' + WEAK + ' (NaVECTOR<float, 4>::Set and the 4-argument constructor).',
    'prlib/setpointer.cpp': 'Both functions inline `SpaTrack<T>::ChangePointer`, which finds the time table with one stride expression instead of a branch per interpolation kind; SpaNodeAnimation::ChangePointer also binds its tables through `BindInlineTables`.',
    'prlib/spadata.cpp': NM + ' (the generic GetValue/GetLinearValue/GetSprineValue instances and both RotateMatrix copies). SpaTransform::GetMatrix now differs only in calling the shared NaMATRIX::Set instead of the original\'s weak copy. IsEverIdentical handles the scale, translate and shear cases in one branch where the original tests them separately; the NaVECTOR spline differs in register allocation. ' + WEAK + ' (the 9- and 16-argument NaMATRIX Set). The vector forms of ScaleMatrix and TranslateMatrix are emitted here as the shared template and differ from the original\'s copies.',
    'prlib/cluster.cpp': NM + ' (RenderClusterNode: two registers swapped).',
    'prlib/mendererawful.cpp': NM + ' (all four functions). The missing symbol is a weak nalib copy.',
    'prlib/transition.cpp': NM + ' (BlendTransitionMatrix).',
    'prlib/mendereralpha.cpp': NM + ' (all three functions).',
    'prlib/shape.cpp': NM + ' (RenderShapeNode). AddShapePosition walks typed `SpmPositionTargets` records and gives its VU asm a memory clobber, where the original did raw index arithmetic on the packet address. As in RenderContext2Node, the original reads the sort position\'s z and w before storing them; this build uses the stored values ([notes](behavior-notes.md#stale-sort-depth-in-context-2-and-shape-nodes-2026-10-07)).',
    'prlib/menderer.cpp': NM + ' (five functions). SetNoodleRotationMatrix has the same source but calls the shared NaMATRIX templates, which moves its registers. ' + WEAK + ' (Set, ScaleMatrix, TranslateMatrix, RotateMatrix).',
    'prlib/spram.cpp': NM + ' (Initialize); the static initializer differs only by a trailing alignment nop. ' + WEAK + ' (the 9-argument Set). Scale, Translate, ScaleMatrix and TranslateMatrix are emitted here as the shared template and differ from the original\'s copies.',
    'prlib/camera.cpp': NM + ' (GetCamera). ' + WEAK + ' (Set and RotateMatrix).',
    'prlib/contour.cpp': NM + ' (both functions). RenderContour also differs in behaviour: the original reads its sort depth\'s z and w before storing the projected position ([notes](behavior-notes.md#stale-sort-depth-in-context-2-and-shape-nodes-2026-10-07)).',
    'prlib/render.cpp': NM + ' (CalculateCurrentMatrixAnimation and CalculateClusterMatrixAnimation; both inline ComposeAnimatedMatrix as the original does, but the original also inlines ComposeGlobalMatrixWithoutVisibility where this build calls it; ComposeGlobalMatrixWithoutVisibility itself; Render). Traced against the original objects (2026-10-07): CalculateClusterMatrix differs only in register allocation and scheduling (one extra register copy, otherwise the same operations); CalculateCurrentMatrix declares the position-animation matrix `pos` outside its `if` block, so `mtx` never points at an ended object, which gives `pos` its own stack slot (a 208-byte frame instead of 160) and moves its empty constructor loop ahead of the branch; RenderContext2Node computes its sort depth from the stored position, where the original reads `pos[2]` and `pos[3]` from the stack before storing them (reproduced by removing the nalib asm `"memory"` clobbers, which the matching reconstruction lacks). That last one is a behaviour difference ([notes](behavior-notes.md#stale-sort-depth-in-context-2-and-shape-nodes-2026-10-07)). ' + WEAK + ' (a renderstuff.h helper).',
    'prlib/mendererdata.cpp': NM + ' (PrGetNoodlePolygonPosition, SetNextTarget). SetNextTarget is 98.75%: the original copies `dx` into a second float register before taking its absolute value, and seven plausible shapes of `Absolute` (conditional both ways, if/negate, a temporary, a const reference, a static function, `0.0f - x`) did not reproduce it (2026-10-12). ' + WEAK + ' (2x2 Set and NaVECTOR::Set).',
    'prlib/menderercreate.cpp': NM + ' (four functions). PrWaitMendererTexture writes the packet\'s register slots through the typed packet struct instead of casting into a u_long array.',
    'prlib/billboard.cpp': 'Traced against the original objects (2026-10-07): ApplyBillboardMatrix differs only in register allocation (`a0` and `a1` swap roles); CreateBillboardMatrix has the same operations plus one register copy and one nop, with one load scheduled later. ' + WEAK + ' (Set and RotateMatrix(int)).',
    'prlib/model.cpp': 'GetPrimitivePosition calls the shared NaVECTOR::Set instead of a local weak copy, and the inlined matrix Apply is scheduled differently.',
    'prlib/renderee.cpp': 'The two vertex kernels are inline asm blocks instead of standalone functions, and the orphan helper is gone; surrounding functions inherit the shifted register use.',
    'menu/menusub.c': 'Eleven functions were reconstructed from asm and never matched without compiler steering; `main-rom-matching` keeps their original asm under `NON_MATCHING` (TsRestoreSaveData, TsRanking_Set, TsPopMenu_Flow, TsPopMenu_Draw, TsPopMenCus_Draw, TsSaveMenu_Draw, TSJukeCDObj_Draw, TsCmnCell_CusorDraw, TsUserList_SetCurTag, TsUserList_Flow, TsNAMEINBox_Draw). TsBGMStop lost a one-pass do-while (tail-call layout only), TsGetTm2Tex and TsGetTm2HedTex read TEX0 fields through `sceGsTex0` (see remaining-work.md).',
    'menu/pksprite.c': 'PkTEX0_SetAdd builds TEX0 with `SCE_GS_SET_TEX0` as one 64-bit value; the original wrote it as two 32-bit halves (see remaining-work.md). GetDToneColor, GetToneColorA and GetToneColorH declare the scratch registers their MMI clamp-and-pack asm overwrites ($2, $8 and $4 or $9); the original left them undeclared, which only worked because of where the compiler put the inputs, so the inputs and result now sit in other registers.',
    'prlib/depthfield.cpp': 'The original carried a local copy of `NaGifPacketWrapper::AddGsAD`; ours calls the shared weak symbol. The copy is a banned brute-forced template. ApplyDepthOfField also has a 16-byte larger frame and swaps `s0` and `s1`; `main-rom-matching` never matched it either ("zbuf and the packet pointer swap s0/s1"). Passing the GIF tag without the wrapper\'s `memcpy` temporary changes nothing, so the frame difference is register allocation, not a missing type (2026-10-12).',
}
d = sum(len(v[0]) for v in a.values())
m = sum(len(v[1]) for v in a.values())
lines = [
    '# Binary differences', '',
    'Function-level differences between this tree and the original-compiler objects (`expected2/`), measured by `tools/dev/audit/alldiff.py`.',
    'Each difference comes from a function that never matched as C (the `main-rom-matching` branch keeps its original asm), or from replacing compiler steering, alias casts and weak template copies with typed C/C++. None is a behaviour change by intent, but the original instruction order is not kept.',
    'Causes marked *inferred* were read from the source change, not traced instruction by instruction. A unit marked *not yet classified* is an open audit item.', '',
    f'Total: {d} functions differ, {m} original symbols have no counterpart (mostly orphan `func_XXXXXXXX` helpers and brute-forced template copies that were removed on purpose).', '',
    '| Unit | Differing functions | Original symbols not reproduced | Cause |', '| --- | --- | --- | --- |',
]
for k, (bad, miss) in a.items():
    if not bad and not miss:
        continue
    lines.append(f"| `{k}` | {len(bad)} | {len(miss)} | {causes.get(k[:-2] if k.endswith('.o') else k, 'Typed reconstruction without compiler steering; not yet classified.')} |")
REGENERATE = 'Regenerate with `python3 tools/dev/audit/alldiff.py && python3 tools/dev/audit/gen_binary_diff_doc.py`.'
lines += ['', REGENERATE]
# Keep the hand-written sections that follow the generated table.
old = open('docs/binary-differences.md').read()
tail = old.split(REGENERATE, 1)[1] if REGENERATE in old else '\n'
open('docs/binary-differences.md', 'w').write('\n'.join(lines) + tail)
