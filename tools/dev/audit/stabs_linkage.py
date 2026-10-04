"""List functions and variables whose linkage differs from the original debug info.

STABS mark each definition as global (F function, G variable) or file-local (f, S). For the C units,
print every name defined in both builds with a different mark. Run from the repository root after a
build. A file-local original that is global here can usually just become static; an original global
that is static here is often deliberate (an uninitialized global would become a common symbol and
move in .bss)."""
import re

from stabs_types import original_stabs, our_stabs


def collect(units, unit_name):
    defs = {}
    for unit, strs in units:
        for s in strs:
            m = re.match(r'([A-Za-z_]\w*):([FfGS])', s)
            if m:
                defs.setdefault(unit_name(unit), {})[m.group(1)] = m.group(2)
    return defs


def main():
    orig = collect(original_stabs(), lambda u: u)
    ours = collect(our_stabs(), lambda u: u.replace('build/src/', '').replace('.o.s', ''))
    kind = {'F': 'global', 'f': 'static', 'G': 'global', 'S': 'static'}
    for unit in sorted(set(orig) & set(ours)):
        for name, mark in sorted(orig[unit].items()):
            our_mark = ours[unit].get(name)
            if our_mark is not None and our_mark != mark:
                print('%s %s: original %s, ours %s' % (unit, name, kind[mark], kind[our_mark]))


if __name__ == '__main__':
    main()
