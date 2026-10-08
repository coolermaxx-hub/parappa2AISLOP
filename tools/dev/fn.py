#!/usr/bin/env python3

"""Print one original function's disassembly straight out of the dump.

Usage: fn.py SYMBOL [SYMBOL...]
"""

import re
import sys
from pathlib import Path

DUMP = Path("dump/disasm/SCPS_150.text.s")
LABEL = re.compile(r"^(?:glabel|endlabel|dlabel) (\S+)")
START = re.compile(r"^(?:glabel|nonmatching) (\S+)")


def extract(symbol, lines):
    """Return the label's body: from its glabel to the next endlabel."""
    begin = end = None
    for i, line in enumerate(lines):
        m = START.match(line)
        if m and m.group(1) == symbol and begin is None:
            begin = i
            continue
        if begin is not None and line.startswith("endlabel " + symbol):
            end = i
            break
    if begin is None:
        return None
    if end is None:
        for j in range(begin + 1, len(lines)):
            if START.match(lines[j]) or LABEL.match(lines[j]):
                end = j
                break
        else:
            end = len(lines)
    return lines[begin:end]


def main():
    lines = DUMP.read_text(encoding="utf-8", errors="replace").splitlines()
    for symbol in sys.argv[1:]:
        body = extract(symbol, lines)
        print(f"===== {symbol} =====")
        if body is None:
            print("(not found)")
            continue
        for line in body:
            if line.startswith("endlabel"):
                break
            print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())