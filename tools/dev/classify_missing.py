#!/usr/bin/env python3

"""Group the outstanding original functions by identical instruction word sequence.

objdiff reports a function as having no counterpart when our build stops
emitting that copy. Grouping by exact word sequence shows which of those are
byte-identical clones of one another (the same inline/template body emitted
into several translation units) and which are genuinely distinct.
"""

import re
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from fn import DUMP, extract  # noqa: E402

WORD = re.compile(r"/\* [0-9A-F ]+ ([0-9A-F]{8}) \*/")


def words(symbol, lines):
    body = extract(symbol, lines)
    if body is None:
        return None
    return tuple(WORD.findall("\n".join(body)))


def main(symbols):
    lines = DUMP.read_text(encoding="utf-8", errors="replace").splitlines()
    groups = defaultdict(list)
    for symbol in symbols:
        seq = words(symbol, lines)
        if seq is None:
            print(f"{symbol}: NOT FOUND")
            continue
        groups[seq].append(symbol)
    for seq, members in sorted(groups.items(), key=lambda kv: -len(kv[1])):
        print(f"[{len(seq):3d} instrs] {' == '.join(members)}")


if __name__ == "__main__":
    main(sys.argv[1:])