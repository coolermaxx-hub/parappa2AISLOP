#!/usr/bin/env python3

"""Triage the functions objdiff reports as having no counterpart in our build.

Splits them by whether the original executable actually calls them. An
unreferenced copy in the original is dead weight the linker kept; readable
source has no reason to emit it, so it is a documentation problem rather than
a reconstruction problem.
"""

import json
import re
import sys
from collections import defaultdict
from pathlib import Path

DUMP = Path("dump/disasm/SCPS_150.text.s")
REPORT = Path("progress/report.json")
CALL = re.compile(r"\b(?:jal|j)\s+(\S+)\s*$")

# objdiff symbol pairs already verified by tools/objdiff_symbol_mappings.py.
KNOWN_ALIASES = {
    "func_00153B28": "Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1",
    "func_00153BD8": "RotateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4RCf",
    "func_0014F410": "Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1",
    "func_0014F4C8": "TranslateMatrix__t8NaMATRIX3Zfi4i4RCfT1T1",
    "func_0014F5D0": "ScaleMatrix__t8NaMATRIX3Zfi4i4RCfT1T1",
    "func_0014F6D8": "RotateMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4RCf",
    "func_001491C0": "Set__t8NaMATRIX3Zfi4i4RCfT1T1T1T1T1T1T1T1T1T1T1T1T1T1T1",
    "func_00145E50": "AppendDmaTag__13PrRenderStuffPC10_sceDmaTag",
    "func_00146A08": "AppendDmaTag__13PrRenderStuffPC10_sceDmaTag",
}

# Bodies are byte-identical across units, so one identification covers its group.
IDENTITIES = {
    "func_0014C4E8": "NaMATRIX<float,4,4>::Set (9 scalars)",
    "func_0014C540": "NaMATRIX<float,4,4>::Set (16 scalars)",
    "func_0014C5F0": "NaMATRIX<float,4,4>::RotateMatrix(NaVECTOR<float,4>, float)",
    "func_0014F3B8": "NaMATRIX<float,4,4>::Set (9 scalars)",
    "func_00151DA0": "NaMATRIX<float,4,4>::Set (9 scalars)",
    "func_0014D748": "NaGifPacketWrapper::AddGsAD(u_int, u_long)",
    "func_0014D8D8": "sceGifPkAddGsAD tail thunk",
    "func_00149168": "NaMATRIX<float,4,4>::Set (9 scalars)",
    "func_0014B988": "NaVECTOR<float,4>::Set(x,y,z,w)",
    "func_0014B9B0": "NaVECTOR<float,4>::Set(x,y,z,w)",
    "func_00151D78": "NaVECTOR<float,4>::Set(x,y,z,w)",
    "func_00151DF8": "NaVECTOR<float,4>::Set(x,y,z,w)",
    "Set__t8NaMATRIX3Zfi4i4RCfT1T1": "NaMATRIX<float,4,4>::Set (9 scalars)",
    "Set__t8NaMATRIX3Zfi2i2RCfT1T1T1T1T1T1T1T1T1": "NaMATRIX<float,2,2>::Set (9 scalars)",
    "func_00153AD0": "NaMATRIX<float,4,4>::Set (9 scalars)",
    "OpenGifTag__11NaGifPacketUI80": "NaGifPacket::OpenGifTag(u_long128)",
    "ScaleMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4": "UNIDENTIFIED (only genuinely absent body)",
}


def missing():
    report = json.loads(REPORT.read_text(encoding="utf-8"))
    out = []
    for unit in report["units"]:
        for func in unit.get("functions", []):
            if func.get("fuzzy_match_percent") is None:
                out.append((unit["name"], func["name"], int(func.get("size", 0))))
    return sorted(out)


def callers(lines):
    found = defaultdict(list)
    for line in lines:
        m = CALL.search(line.rstrip())
        if m:
            found[m.group(1)].append(line)
    return found


def main():
    lines = DUMP.read_text(encoding="utf-8", errors="replace").splitlines()
    called = callers(lines)
    rows = missing()
    print(f"{len(rows)} functions have no counterpart in our build\n")
    dead = live = 0
    for unit, name, size in rows:
        refs = called.get(name, [])
        live += bool(refs)
        dead += not refs
        ident = IDENTITIES.get(name) or KNOWN_ALIASES.get(name) or "-"
        state = "called" if refs else "UNCALLED"
        print(f"{state:9s} {unit:24s} {size:5d}  {name}\n{'':9s} {'':24s} {'':5s}  -> {ident}")
    print(f"\n{live} have callers, {dead} are unreferenced in the original")


if __name__ == "__main__":
    sys.exit(main())