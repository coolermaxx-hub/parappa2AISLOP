"""List float loads from a stack slot that come before a later 16-byte store
(sq or sqc2) to the same slot, with no other store to those bytes in between.

Usage: python3 tools/dev/audit/hoisted_vector_reads.py <object-or-elf> [lo hi]

This is the shape of the sort-depth bug in the original executable: the
compiler read a vector's z and w before the inline VU/MMI copy that writes the
vector, because that assembly declared no "memory" clobber. Unlike
stale_stack_reads.py it also reports slots written earlier in the function, so
the early read returns an older value instead of leftover memory. Address order
is used as a stand-in for control flow; check every hit by hand. See
docs/behavior-notes.md (stale sort depth). Run from the repository root.
"""
import re
import subprocess
import sys

STORE_SIZES = {'sw': 4, 'swc1': 4, 'sd': 8, 'sq': 16, 'sqc2': 16}


def functions(path, lo=None, hi=None):
    args = ['mips-linux-gnu-objdump', '-d', '--no-show-raw-insn', '-M', 'no-aliases', path]
    if lo is not None:
        args += ['--start-address=%#x' % lo, '--stop-address=%#x' % hi]
    out = subprocess.run(args, capture_output=True, text=True).stdout
    funcs, cur = {}, None
    for line in out.split('\n'):
        m = re.match(r'^([0-9a-f]+) <(.+)>:$', line)
        if m:
            name = m.group(2)
            # Local labels and compiler markers do not start a new function.
            if not (cur and (name.startswith('.L') or name.startswith('gcc2_compiled')
                             or name.startswith('__gnu_compiled'))):
                cur = name
                funcs.setdefault(cur, [])
            continue
        m = re.match(r'^\s*([0-9a-f]+):\s*(\S+)\s*(.*)$', line)
        if cur and m:
            funcs[cur].append((int(m.group(1), 16), m.group(2), m.group(3)))
    return funcs


def stack_offset(args):
    m = re.search(r',(-?\d+)\(sp\)$', args)
    return int(m.group(1)) if m else None


def scan(ins):
    for i, (addr, op, args) in enumerate(ins):
        off = stack_offset(args) if op == 'lwc1' else None
        if off is None:
            continue
        for later_addr, later_op, later_args in ins[i + 1:]:
            store = stack_offset(later_args) if later_op in STORE_SIZES else None
            if store is None or not store <= off < store + STORE_SIZES[later_op]:
                continue
            if later_op in ('sq', 'sqc2'):
                yield addr, args, later_addr, '%s %s' % (later_op, later_args)
            break


def main():
    path = sys.argv[1]
    lo = int(sys.argv[2], 0) if len(sys.argv) > 2 else None
    hi = int(sys.argv[3], 0) if len(sys.argv) > 3 else None
    for name, ins in functions(path, lo, hi).items():
        for hit in scan(ins):
            print('%s %x lwc1 %s ... %x %s' % ((name,) + hit))


if __name__ == '__main__':
    main()
