#!/usr/bin/env python3
"""
Add `symbol_mappings` to objdiff.json for functions that sit at the same
.text offset in the target and base objects but carry different names.

The usual case is an out-of-line template or inline helper (e.g. a
NaGifPacket member) that splat named `func_XXXXXXXX` in the target, while
the compiled C++ object emits it under its mangled name. Without a mapping
objdiff can't pair the two and reports the function as unmatched.

objdiff still diffs every mapped pair, so a mapping never turns a
non-matching function into a matching one.

usage (after `python configure.py --objdiff` and `ninja`):
    python3 tools/objdiff_symbol_mappings.py
"""

import json
import subprocess
from pathlib import Path

CROSS = "mips-linux-gnu-"


def text_symbols(path: Path) -> dict[int, list[str]]:
    out = subprocess.run(
        [f"{CROSS}nm", "-n", "--defined-only", str(path)],
        capture_output=True, text=True, check=True,
    ).stdout
    syms: dict[int, list[str]] = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) != 3 or parts[1] not in "TtWw":
            continue
        name = parts[2]
        if name.endswith(".NON_MATCHING") or name in ("gcc2_compiled.", "__gnu_compiled_cplusplus", "__gnu_compiled_c"):
            continue
        syms.setdefault(int(parts[0], 16), []).append(name)
    return syms


def main():
    conf_path = Path("objdiff.json")
    conf = json.loads(conf_path.read_text())

    total = 0
    for unit in conf["units"]:
        target = Path(unit.get("target_path", ""))
        base = Path(unit.get("base_path", ""))
        if not target.exists() or not base.exists():
            continue

        tsyms = text_symbols(target)
        bsyms = text_symbols(base)
        mappings = {}
        for addr, tnames in tsyms.items():
            bnames = bsyms.get(addr, [])
            if not bnames:
                continue
            for tname in tnames:
                if tname in bnames:
                    continue
                cands = [b for b in bnames if b not in tsyms.get(addr, [])]
                if len(cands) == 1:
                    mappings[tname] = cands[0]

        if mappings:
            unit["symbol_mappings"] = mappings
            total += len(mappings)
        else:
            unit.pop("symbol_mappings", None)

    conf_path.write_text(json.dumps(conf, indent=2))
    print(f"Wrote {total} symbol mappings to {conf_path}")


if __name__ == "__main__":
    main()
