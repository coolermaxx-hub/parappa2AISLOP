#!/usr/bin/env python3
"""Pair verified original inline/template aliases with their C++ symbols.

Equal text offsets are not evidence of equal function identity once source
sizes or emission order change. Only the aliases decoded from the original
function bodies below are paired. objdiff still compares every mapped pair.
Run after configure.py --objdiff and a source build.
"""

import json
import subprocess
from pathlib import Path

CROSS = 'mips-linux-gnu-'
SET16 = 'Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1'
ROTATE_AXIS = 'RotateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4RCf'
APPEND_DMA = 'AppendDmaTag__13PrRenderStuffPC10_sceDmaTag'

# Provenance: corresponding original asm/nonmatchings/prlib/<unit> bodies.
# Set16 stores sixteen scalar arguments, XYZ factories build the corresponding
# matrices, and queue aliases tail-call PrDmaQueue::Append. Unused Set9 copies
# intentionally have no mapping when no real caller causes their emission.
VERIFIED_MAPPINGS = {
    'prlib/camera': {
        'func_00153B28': SET16,
        'func_00153BD8': ROTATE_AXIS,
    },
    'prlib/menderer': {
        'func_0014F410': SET16,
        'func_0014F4C8': 'TranslateMatrix__t8NaMATRIX3Zfi4i4RCfT1T1',
        'func_0014F5D0': 'ScaleMatrix__t8NaMATRIX3Zfi4i4RCfT1T1',
        'func_0014F6D8': 'RotateMatrix__t8NaMATRIX3Zfi4i4iRCf',
    },
    'prlib/spadata': {'func_001491C0': SET16},
    'prlib/render': {'func_00145E50': APPEND_DMA},
    'prlib/renderee': {'func_00146A08': APPEND_DMA},
    'prlib/spram': {
        '_GLOBAL_$I$Initialize__12PrSPRAM_DATAP13PrSceneObject': '_GLOBAL_$I$screenClipMatrix',
    },
}


def text_symbols(path: Path) -> set[str]:
    result = subprocess.run(
        [CROSS + 'nm', '--defined-only', str(path)],
        capture_output=True, text=True, check=True,
    )
    symbols = set()
    for line in result.stdout.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] in 'TtWw':
            symbols.add(parts[2])
    return symbols


def verified_pairs(unit_name, target_symbols, base_symbols):
    return {
        target: base
        for target, base in VERIFIED_MAPPINGS.get(unit_name, {}).items()
        if target in target_symbols and base in base_symbols
    }


def main():
    path = Path('objdiff.json')
    config = json.loads(path.read_text())
    total = 0
    for unit in config['units']:
        unit.pop('symbol_mappings', None)
        target = Path(unit.get('target_path', ''))
        base = Path(unit.get('base_path', ''))
        if not target.is_file() or not base.is_file():
            continue
        mappings = verified_pairs(unit['name'], text_symbols(target), text_symbols(base))
        if mappings:
            unit['symbol_mappings'] = mappings
            total += len(mappings)
    path.write_text(json.dumps(config, indent=2) + '\n')
    print('Wrote %d verified symbol mappings to %s' % (total, path))


if __name__ == '__main__':
    main()
