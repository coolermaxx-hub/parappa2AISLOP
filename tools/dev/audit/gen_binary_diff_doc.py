"""Regenerate docs/binary-differences.md from alldiff_out.json (run from the repo root)."""
import json
a = json.load(open('tools/dev/audit/alldiff_out.json'))
causes = {
    'nalib/navector.cpp': 'Set(NaMATRIX3) is the real template. The original image carried a brute-forced copy; that copy is not reproduced.',
    'menu/menu.c': 'MenuCtrl: register pin removed (see remaining-work.md).',
    'menu/p3mc.c': 'Typed tables and `P3MC_DATE` replace word casts and a do-while (see remaining-work.md).',
    'main/wipe.c': 'WipeParaOutDisp: loop-alignment asm removed (see remaining-work.md).',
    'main/drawctrl.c': 'DrawMozaikuDisp uses a typed masked frame (see remaining-work.md). The other two functions are not yet classified.',
    'main/scrctrl.c': 'ScrExamSetCheck: gotos replaced by structured control flow.',
    'prlib/random.cpp': 'PrFloatRandom is a plain do-while (see remaining-work.md).',
    'prlib/prlib.cpp': 'PrSetDebugParamFloat stores through the union member (see remaining-work.md).',
    'prlib/renderee.cpp': 'The two vertex kernels are inline asm blocks instead of standalone functions, and the orphan helper is gone; surrounding functions inherit the shifted register use.',
    'prlib/depthfield.cpp': 'The original carried a local copy of `NaGifPacketWrapper::AddGsAD`; ours calls the shared weak symbol. The copy is a banned brute-forced template.',
}
d = sum(len(v[0]) for v in a.values())
m = sum(len(v[1]) for v in a.values())
lines = [
    '# Binary differences', '',
    'Function-level differences between this tree and the original-compiler objects (`expected2/`), measured by `tools/dev/audit/alldiff.py`.',
    'Every difference comes from replacing compiler-steering or alias source with typed C/C++. None is a behaviour change by intent, but the original instruction order is not kept.',
    'Functions marked *not yet classified* differ and nobody has traced the cause yet; treat them as open audit items.', '',
    f'Total: {d} functions differ, {m} original symbols have no counterpart (mostly orphan `func_XXXXXXXX` helpers and brute-forced template copies that were removed on purpose).', '',
    '| Unit | Differing functions | Original symbols not reproduced | Cause |', '| --- | --- | --- | --- |',
]
for k, (bad, miss) in a.items():
    if not bad and not miss:
        continue
    lines.append(f"| `{k}` | {len(bad)} | {len(miss)} | {causes.get(k, 'Typed reconstruction without compiler steering; not yet classified.')} |")
lines += ['', 'Regenerate with `python3 tools/dev/audit/alldiff.py && python3 tools/dev/audit/gen_binary_diff_doc.py`.']
open('docs/binary-differences.md', 'w').write('\n'.join(lines) + '\n')
