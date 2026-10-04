"""Regenerate docs/binary-differences.md from alldiff_out.json (run from the repo root)."""
import json
a = json.load(open('tools/dev/audit/alldiff_out.json'))
NM = 'Never matched as C: `main-rom-matching` keeps the original asm under `NON_MATCHING` for these'
WEAK = 'The missing symbols are the original\'s per-file weak copies of nalib templates, which this build does not reproduce; every file calls the one shared template'
causes = {
    'nalib/navector.cpp': 'Set(NaMATRIX3) is the real template. The original image carried a brute-forced copy; that copy is not reproduced.',
    'menu/menu.c': 'MenuCtrl: register pin removed (see remaining-work.md).',
    'menu/p3mc.c': 'Typed tables and `P3MC_DATE` replace word casts and a do-while (see remaining-work.md).',
    'main/wipe.c': 'WipeParaOutDisp: loop-alignment asm removed (see remaining-work.md).',
    'main/drawctrl.c': 'DrawMozaikuDisp uses a typed masked frame (see remaining-work.md). ' + NM + ' (DrawObjStrDisp, DrawObjPrReq: register allocation).',
    'main/scrctrl.c': 'ScrExamSetCheck: gotos replaced by structured control flow.',
    'prlib/random.cpp': 'PrFloatRandom is a plain do-while (see remaining-work.md).',
    'prlib/prlib.cpp': 'PrSetDebugParamFloat stores through the union member (see remaining-work.md). The original symbol `PrSetModelVisibillity` is spelled `PrSetModelVisibility` here, so it counts as not reproduced.',
    'prlib/scene.cpp': NM + ' (the constructor and SetAppropriateDefaultCamera). ' + WEAK + ' (NaVECTOR<float, 4>::Set and the 4-argument constructor).',
    'prlib/setpointer.cpp': 'Both functions inline `SpaTrack<T>::ChangePointer`, which finds the time table with one stride expression instead of a branch per interpolation kind; SpaNodeAnimation::ChangePointer also binds its tables through `BindInlineTables`.',
    'prlib/spadata.cpp': NM + ' (the generic GetValue/GetLinearValue/GetSprineValue instances and both RotateMatrix copies). SpaTransform::GetMatrix uses real function-local statics where the matching draft emulated them with extern guard variables. IsEverIdentical and the NaVECTOR spline are unchanged in logic and differ in register allocation. ' + WEAK + ' (the 9- and 16-argument NaMATRIX Set and two more).',
    'prlib/cluster.cpp': NM + ' (RenderClusterNode: two registers swapped).',
    'prlib/mendererawful.cpp': NM + ' (all four functions). The missing symbol is a weak nalib copy.',
    'prlib/transition.cpp': NM + ' (BlendTransitionMatrix).',
    'prlib/mendereralpha.cpp': NM + ' (all three functions).',
    'prlib/shape.cpp': NM + ' (RenderShapeNode). AddShapePosition walks typed `SpmPositionTargets` records and gives its VU asm a memory clobber, where the original did raw index arithmetic on the packet address.',
    'prlib/menderer.cpp': NM + ' (five functions). SetNoodleRotationMatrix has the same source but calls the shared NaMATRIX templates, which moves its registers. ' + WEAK + ' (Set, ScaleMatrix, TranslateMatrix, RotateMatrix).',
    'prlib/spram.cpp': NM + ' (Initialize); the static initializer differs only by a trailing alignment nop. ' + WEAK + ' (Set, Scale, Translate and their Matrix forms).',
    'prlib/camera.cpp': NM + ' (GetCamera). ' + WEAK + ' (Set and RotateMatrix).',
    'prlib/contour.cpp': NM + ' (both functions).',
    'prlib/render.cpp': NM + ' (CalculateCurrentMatrixAnimation and CalculateClusterMatrixAnimation; both inline ComposeAnimatedMatrix as the original does, but the original also inlines ComposeGlobalMatrixWithoutVisibility where this build calls it; ComposeGlobalMatrixWithoutVisibility itself; Render). CalculateClusterMatrix, CalculateCurrentMatrix and RenderContext2Node use the typed node fields and the rewritten nalib templates (*inferred*). ' + WEAK + ' (NaVECTOR::Set and a renderstuff.h helper).',
    'prlib/mendererdata.cpp': NM + ' (PrGetNoodlePolygonPosition, SetNextTarget). GetSynchronizeRatio uses the file\'s own Absolute/Minimum helpers instead of NaAbs/NaMin. ' + WEAK + ' (2x2 Set and NaVECTOR::Set).',
    'prlib/menderercreate.cpp': NM + ' (four functions). PrWaitMendererTexture writes the packet\'s register slots through the typed packet struct instead of casting into a u_long array.',
    'prlib/billboard.cpp': 'Same logic as the matching source; both functions call the shared NaMATRIX templates and the rewritten operators, which changes their scheduling (*inferred*). ' + WEAK + ' (Set and RotateMatrix(int)).',
    'prlib/model.cpp': 'GetPrimitivePosition calls the shared NaVECTOR::Set instead of a local weak copy (the missing symbol), and the inlined matrix Apply is scheduled differently.',
    'prlib/renderee.cpp': 'The two vertex kernels are inline asm blocks instead of standalone functions, and the orphan helper is gone; surrounding functions inherit the shifted register use.',
    'menu/menusub.c': 'Twelve functions were reconstructed from asm and never matched without compiler steering; `main-rom-matching` keeps their original asm under `NON_MATCHING` (TsRestoreSaveData, TsRanking_Set, TsPopMenu_Flow, TsPopMenu_Draw, TsPopMenCus_Draw, TsSaveMenu_Draw, TSJukeCDObj_Draw, TsCmnCell_CusorDraw, TsOption_Flow, TsUserList_SetCurTag, TsUserList_Flow, TsNAMEINBox_Draw). TsBGMStop lost a one-pass do-while (tail-call layout only), TsGetTm2Tex and TsGetTm2HedTex read TEX0 fields through `sceGsTex0` (see remaining-work.md), and TsHosiPut differs only by a trailing alignment nop.',
    'prlib/depthfield.cpp': 'The original carried a local copy of `NaGifPacketWrapper::AddGsAD`; ours calls the shared weak symbol. The copy is a banned brute-forced template.',
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
lines += ['', 'Regenerate with `python3 tools/dev/audit/alldiff.py && python3 tools/dev/audit/gen_binary_diff_doc.py`.']
open('docs/binary-differences.md', 'w').write('\n'.join(lines) + '\n')
