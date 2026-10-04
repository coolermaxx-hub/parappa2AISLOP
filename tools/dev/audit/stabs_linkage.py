"""List functions and variables whose linkage or parameter names differ from the original debug info.

STABS mark each definition as global (F function, G variable) or file-local (f, S). For the C units,
print every name defined in both builds with a different mark, then every function whose parameter
names differ. Run from the repository root after a
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


def collect_params(units, unit_name):
    params = {}
    for unit, strs in units:
        func = None
        for s in strs:
            m = re.match(r'([A-Za-z_]\w*):[Ff]', s)
            if m:
                func = (unit_name(unit), m.group(1))
                params[func] = []
                continue
            m = re.match(r'([A-Za-z_]\w*):[pPR]', s)
            if m and func and m.group(1) not in params[func]:
                params[func].append(m.group(1))
    return params


def main():
    orig = collect(original_stabs(), lambda u: u)
    ours = collect(our_stabs(), lambda u: u.replace('build/src/', '').replace('.o.s', ''))
    kind = {'F': 'global', 'f': 'static', 'G': 'global', 'S': 'static'}
    for unit in sorted(set(orig) & set(ours)):
        for name, mark in sorted(orig[unit].items()):
            our_mark = ours[unit].get(name)
            if our_mark is not None and our_mark != mark:
                print('%s %s: original %s, ours %s' % (unit, name, kind[mark], kind[our_mark]))

    orig_params = collect_params(original_stabs(), lambda u: u)
    our_params = collect_params(our_stabs(), lambda u: u.replace('build/src/', '').replace('.o.s', ''))
    for func in sorted(set(orig_params) & set(our_params)):
        if orig_params[func] != our_params[func]:
            print('%s %s(%s): ours (%s)' % (func[0], func[1], ', '.join(orig_params[func]), ', '.join(our_params[func])))


if __name__ == '__main__':
    main()
