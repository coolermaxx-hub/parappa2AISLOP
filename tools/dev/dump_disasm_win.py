#!/usr/bin/env python3

"""Windows-friendly wrapper around the same spimdisasm call tools/setup.py makes.

setup.py shells out to the downloaded Linux-only `ccc` binary before dumping
asm, so the disassembly step cannot run on this host. The spimdisasm package
itself is pure Python and works here, so call it directly with the same
arguments the project uses.
"""

import sys

import spimdisasm.elfObjDisasm

P3_ELF_PATH = "iso/SCPS_150.17"
P3_GP_VALUE = 0x3A0EF0
DISASM_DIR = "dump/disasm"


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else DISASM_DIR
    sys.argv = [
        "spimdisasm.elfObjDisasm",
        "--instr-category", "r5900",
        "--compiler", "EEGCC",
        "--endian", "little",
        "--gp", f"{P3_GP_VALUE}",
        P3_ELF_PATH,
        out,
    ]
    spimdisasm.elfObjDisasm.elfObjDisasmMain()


if __name__ == "__main__":
    main()