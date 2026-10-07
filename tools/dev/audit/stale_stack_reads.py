"""List loads from a function's own stack frame that come before any store to
those bytes, for every object under a directory (default: obj/, the original
units split by `./configure.py --objdiff`).

A hit means the function may read stale stack memory. The scan is linear in
address order, swaps each call with its delay slot, follows registers set to
sp+N, and treats a stack address passed to a call or used by VU code as
written. It does not follow unaligned (sdl/sdr) or indexed stores, or stack
addresses kept in saved registers across calls, so check every hit by hand;
see docs/behavior-notes.md (stale sort depth) for the genuine cases found so
far. Run from the repository root."""
import glob
import re
import subprocess
import sys

LOADS = {'lw': 4, 'lwc1': 4, 'lq': 16, 'lqc2': 16, 'lh': 2, 'lhu': 2, 'lb': 1, 'lbu': 1, 'lwu': 4}
STORES = {'sw': 4, 'swc1': 4, 'sq': 16, 'sqc2': 16, 'sd': 8, 'sh': 2, 'sb': 1}


def functions(path):
    out = subprocess.run(["mips-linux-gnu-objdump", "-d", "--no-show-raw-insn", "-M", "no-aliases", path],
                         capture_output=True, text=True).stdout
    funcs, cur = {}, None
    for line in out.split("\n"):
        m = re.match(r"^([0-9a-f]+) <(.+)>:$", line)
        if m:
            # Jump-table labels split a function; keep them in the current one.
            if not (m.group(2).startswith('.L') and cur):
                cur = m.group(2)
                funcs[cur] = []
            continue
        m = re.match(r"^\s*([0-9a-f]+):\s*(\S+)\s*(.*)$", line)
        if cur and m:
            funcs[cur].append((int(m.group(1), 16), m.group(2), m.group(3)))
    return funcs


def scan(name, ins):
    frame = 0
    for _, op, args in ins[:4]:
        m = re.match(r'sp,sp,-(\d+)$', args.replace(' ', ''))
        if op == 'addiu' and m:
            frame = int(m.group(1))
    ins = list(ins)
    k = 0
    while k < len(ins) - 1:
        if ins[k][1] in ('jal', 'jalr'):
            ins[k], ins[k + 1] = ins[k + 1], ins[k]
            k += 2
        else:
            k += 1

    written, escaped, base = set(), set(), {'sp': 0}
    for addr, op, args in ins:
        a = [x.strip() for x in args.split(',')]
        if a and a[0] == 'sp':
            continue
        if op in ('addiu', 'daddiu') and len(a) == 3 and a[1] in base:
            try:
                base[a[0]] = base[a[1]] + int(a[2], 0)
                continue
            except ValueError:
                pass
        if op in ('daddu', 'addu', 'or') and len(a) == 3 and a[2] == 'zero' and a[1] in base:
            base[a[0]] = base[a[1]]
            continue
        m = re.match(r'(-?\d+)\((\w+)\)$', a[-1]) if a else None
        if m and m.group(2) in base and (op in LOADS or op in STORES):
            off = base[m.group(2)] + int(m.group(1))
            span = set(range(off, off + (LOADS.get(op) or STORES.get(op))))
            if op in STORES:
                written |= span
            elif not span <= written and not span & escaped and off < frame:
                yield addr - ins[0][0], op, args, off
            continue
        if op in ('jal', 'jalr') or op.startswith('v'):
            for reg, value in base.items():
                if reg != 'sp':
                    escaped |= set(range(value, value + 64))
        if a and a[0] in base and a[0] != 'sp':
            base.pop(a[0], None)


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else 'obj'
    for path in sorted(glob.glob(root + '/**/*.o', recursive=True)):
        for name, ins in functions(path).items():
            for offset, op, args, slot in scan(name, ins):
                print(f"{path}: {name} +{offset:#x} {op} {args}  (sp+{slot})")


if __name__ == '__main__':
    main()
