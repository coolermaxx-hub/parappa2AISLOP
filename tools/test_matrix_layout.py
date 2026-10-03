#!/usr/bin/env python3
"""Run scalar matrix layout checks using the historical EE compiler.

This exercises real headers and scalar stores under user-mode MIPS emulation.
The N32 ELF carrier preserves 32-bit pointers and runs the compiler's real
64-bit register loads/stores. MIPS32 would macro-expand those instructions,
including return-address loads in branch delay slots, invalidating the check.
R5900 three-operand MULT is lowered to the single-instruction MIPS MUL only
when no HI/LO reads occur. That preserves scalar results and branch delay slots;
production objects are never changed. This does not exercise COP2 instructions
or emulate PS2 floating-point behavior.
"""
import argparse
from pathlib import Path
import shutil
import re
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='tools/toolchain/ee-gcc29/bin/ee-gcc')
    parser.add_argument('--assembler', default='mips-linux-gnu-as')
    parser.add_argument('--linker', default='mips-linux-gnu-ld')
    parser.add_argument('--emulator', default='qemu-mipsn32el')
    parser.add_argument('--source', default='tests/nalib/matrix_layout.cpp',
                        help='C++ scalar test defining matrix_layout_test')
    args = parser.parse_args()
    repo = Path(__file__).resolve().parent.parent
    for name in ('compiler', 'assembler', 'linker', 'emulator'):
        program = getattr(args, name)
        if '/' in program and not Path(program).is_absolute():
            program = str(repo / program)
            setattr(args, name, program)
        if shutil.which(program) is None:
            parser.error('Required executable is unavailable: ' + program)

    with tempfile.TemporaryDirectory(prefix='parappa-matrix-') as directory:
        output = Path(directory)
        assembly = output / 'matrix.s'
        includes = ['include', 'src', 'include/rtl/common', 'include/rtl/ee',
                    'include/rtl/ee_gcc', 'include/rtl/ee_gcc/gcc-lib']
        subprocess.run([args.compiler, '-S', '-O2', '-G0', '-x', 'c++',
                        '-fno-exceptions', *['-I' + path for path in includes],
                        args.source, '-o', str(assembly)],
                       cwd=repo, check=True)
        # Sony's compiler emits R5900 MULT with a destination register even
        # under -mips3. MIPS MUL has the same low-word scalar destination, but
        # different HI/LO effects. Refuse adaptation if those effects are read.
        source_text = assembly.read_text()
        multiplication = re.compile(r'(?m)^(\s*)mult(\s+)(\$\d+,\$\d+,\$\d+)([^\n]*)$')
        if multiplication.search(source_text):
            if re.search(r'\b(?:mfhi|mflo|mfhi1|mflo1)\b', source_text):
                raise SystemExit('Scalar adapter cannot preserve observed HI/LO state')
            source_text = multiplication.sub(r'\1mul\2\3\4', source_text)
            assembly.write_text(source_text)
        start = output / 'start.s'
        start.write_text('''.set noreorder
.text
.globl _start
.ent _start
_start:
    jal matrix_layout_test
    nop
    move $a0, $v0
    li $v0, 6058
    syscall
.end _start
''')
        for source in [assembly, start]:
            subprocess.run([args.assembler, '-EL', '-mips64', '-mabi=n32', '-I' + str(repo / 'include'), str(source),
                            '-o', str(source.with_suffix('.o'))], check=True)
        executable = output / 'matrix.elf'
        subprocess.run([args.linker, '-EL', '-m', 'elf32ltsmipn32', '-e', '_start',
                        '-o', str(executable), str(start.with_suffix('.o')),
                        str(assembly.with_suffix('.o'))], check=True)
        result = subprocess.run([args.emulator, str(executable)])
        if result.returncode:
            raise SystemExit('Matrix layout check failed: ' + str(result.returncode))
    print('PASS: EE scalar checks (' + args.source + ')')


if __name__ == '__main__':
    main()
