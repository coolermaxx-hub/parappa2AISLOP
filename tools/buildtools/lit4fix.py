#!/usr/bin/env python3
"""
Point C float literals at the .lit4 entries that still live in asm.

gcc emits non-trivial float constants as `li.s $fN,<value>` and leaves it to
the assembler to put the value in .lit4. Until every function of a file is
decompiled, that file's .lit4 data comes from asm/data/<unit>.lit4.s, so a C
function that uses such a constant would end up with a second copy of it.

This rewrites each `li.s` that would need a .lit4 entry into a gp-relative
load of the matching symbol in the asm .lit4 file. The assembler emits
literal entries in source order without merging duplicates, so the asm file
is walked in order: functions pulled in with INCLUDE_ASM advance the cursor
past the symbols they reference, and decompiled functions take the next
entries. The value is checked before a symbol is used.

usage: lit4fix.py <source file> <compiler asm output, rewritten in place>
"""

import re
import struct
import sys
from pathlib import Path

LIT4_RE = re.compile(r"dlabel (D_\w+)\n\s*/\* \w+ \w+ ([0-9A-Fa-f]{8}) \*/\s*\.float")
LI_S_RE = re.compile(r"^(\s*)li\.s\s+(\$f\d+),\s*(\S+)\s*$")
INCLUDE_RE = re.compile(r'\.include\s+\\?"(asm/nonmatchings/[^"\\]+)\\?"')
SYM_RE = re.compile(r"\((D_[0-9A-Fa-f]{8})\)")


def float_bits(text: str) -> int:
    return struct.unpack(">I", struct.pack(">f", float(text)))[0]


def main() -> int:
    src, asm_path = sys.argv[1], sys.argv[2]

    unit = re.sub(r"^src/", "", re.sub(r"\.(c|cpp)$", "", src))
    lit4_path = Path(f"asm/data/{unit}.lit4.s")
    if not lit4_path.exists():
        return 0

    entries = []
    for m in LIT4_RE.finditer(lit4_path.read_text()):
        value = struct.unpack("<I", bytes.fromhex(m.group(2)))[0]
        entries.append((m.group(1), value))
    index = {sym: i for i, (sym, _) in enumerate(entries)}

    lines = Path(asm_path).read_text().split("\n")
    cursor = 0
    out = []
    for line in lines:
        inc = INCLUDE_RE.search(line)
        if inc:
            path = Path(inc.group(1))
            if path.exists():
                for sym in SYM_RE.findall(path.read_text()):
                    if sym in index:
                        cursor = max(cursor, index[sym] + 1)
            out.append(line)
            continue

        m = LI_S_RE.match(line)
        if m:
            bits = float_bits(m.group(3))
            # Values the assembler builds with lui/mtc1 never touch .lit4.
            if bits & 0xFFFF:
                if cursor < len(entries) and entries[cursor][1] == bits:
                    sym = entries[cursor][0]
                    cursor += 1
                    # .extern with a size marks the symbol as small data, so the
                    # assembler turns the load into a single gp-relative lwc1.
                    out.append(f"{m.group(1)}.extern\t{sym},4")
                    out.append(f"{m.group(1)}lwc1\t{m.group(2)},{sym}")
                    continue
                print(
                    f"lit4fix: {src}: no .lit4 entry for {m.group(3)} "
                    f"(next is {entries[cursor][0] if cursor < len(entries) else 'none'})",
                    file=sys.stderr,
                )
        out.append(line)

    Path(asm_path).write_text("\n".join(out))
    return 0


if __name__ == "__main__":
    sys.exit(main())
