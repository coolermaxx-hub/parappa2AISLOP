"""Compare each function's local debug symbols with the original's: names, storage and block nesting.

Both the original C units and ours are built with -g, so this compares like with like: gcc only
describes locals that survive optimization, so a variable the optimizer folded away (a copy, a loop
counter replaced by a pointer) is missing from both sides and does not show up as a difference.
That makes this more reliable than reading declarations from the source (stabs_locals.py).

    python3 tools/dev/audit/stabs_scopes.py [function-or-file-substring] [-o] [-s] [-v]

For every function present in both, the locals are printed as a nested list, e.g.
    nStage:r16 { n:r2 } { r:r6 i:r8 }
where r<N> is a register, s<N> a stack offset, p a parameter and V/S a function-scope static. A
function is reported when the names in a scope or the nesting differ. Scopes without any described
variable (all of theirs were optimized away) only count with -s; the original has some that cannot
be traced back to source, such as a block covering a whole function. With -o, the declaration
order within a scope must match too; with -v, register and stack slots must match as well (they
differ in functions whose code does not match the original). Run from the repository root after a
build."""
import glob
import re
import struct
import sys

N_FUN, N_STSYM, N_LCSYM, N_RSYM, N_PSYM, N_LSYM, N_LBRAC, N_RBRAC = 0x24, 0x26, 0x28, 0x40, 0xa0, 0x80, 0xc0, 0xe0


def original_entries():
    """Yields (file, [(stab code, string, value)]) for the original C units."""
    f = open('iso/SCPS_150.17', 'rb').read()
    e_shoff = struct.unpack_from('<I', f, 0x20)[0]
    e_shentsize, e_shnum, e_shstrndx = struct.unpack_from('<HHH', f, 0x2e)
    secs = [struct.unpack_from('<IIIIIIIIII', f, e_shoff + i * e_shentsize) for i in range(e_shnum)]
    shstr = secs[e_shstrndx][4]

    def nm(o):
        return f[shstr + o:f.index(b'\0', shstr + o)].decode()

    md = [s for s in secs if nm(s[0]) == '.mdebug'][0]
    off = md[4]
    h = struct.unpack_from('<hh' + 'i' * 23, f, off)
    names = ['magic', 'vstamp', 'ilineMax', 'cbLine', 'cbLineOffset', 'idnMax', 'cbDnOffset', 'ipdMax',
             'cbPdOffset', 'isymMax', 'cbSymOffset', 'ioptMax', 'cbOptOffset', 'iauxMax', 'cbAuxOffset',
             'issMax', 'cbSsOffset', 'issExtMax', 'cbSsExtOffset', 'ifdMax', 'cbFdOffset', 'crfd',
             'cbRfdOffset', 'iextMax', 'cbExtOffset']
    H = dict(zip(names, h))
    base = off + 0x60 - H['cbLineOffset']
    if H['cbFdOffset'] < off:
        base = 0
    for i in range(H['ifdMax']):
        o = H['cbFdOffset'] + base + i * 72
        fd = dict(zip(['adr', 'rss', 'issBase', 'cbSs', 'isymBase', 'csym'],
                      struct.unpack_from('<Iiiiii', f, o)))

        def sstr(iss):
            p = H['cbSsOffset'] + base + fd['issBase'] + iss
            return f[p:f.index(b'\0', p)].decode('latin1')

        fname = sstr(fd['rss']) if fd['rss'] >= 0 else ''
        if not fname.endswith('.c'):
            continue
        entries = []
        for k in range(fd['csym']):
            p = H['cbSymOffset'] + base + (fd['isymBase'] + k) * 12
            iss, value, bits = struct.unpack_from('<iiI', f, p)
            index = bits >> 12
            # Embedded STABS: the auxiliary index is 0x8f300 plus the stab code.
            if index & ~0xff == 0x8f300:
                entries.append((index & 0xff, sstr(iss) if iss >= 0 else '', value))
        yield fname, entries


def our_entries():
    for path in sorted(glob.glob('build/src/**/*.c.o.s', recursive=True)):
        entries = []
        for line in open(path, encoding='latin1'):
            m = re.match(r'\s*\.stabs\s+"((?:[^"\\]|\\.)*)",(\d+),\d+,-?\d+,(\S+)', line)
            if m:
                entries.append((int(m.group(2)), m.group(1), m.group(3)))
                continue
            m = re.match(r'\s*\.stabn\s+(\d+),\d+,-?\d+,(\S+)', line)
            if m:
                entries.append((int(m.group(1)), '', m.group(2)))
        yield path[len('build/src/'):-len('.o.s')], entries


def describe(code, string, value):
    name, _, desc = string.partition(':')
    letter = desc[:1]
    if code == N_RSYM:
        return '%s:r%s' % (name, value)
    if code == N_PSYM or letter in 'Pp':
        return '%s:p' % name
    if code in (N_STSYM, N_LCSYM):
        return '%s:%s' % (name, letter)
    if code == N_LSYM and (letter.isdigit() or letter == '('):
        return '%s:s%s' % (name, value)
    return None


def functions(entries):
    """Maps function name to its list of local tokens ('{', '}' or a description)."""
    result = {}
    current = None
    for code, string, value in entries:
        if code == N_FUN:
            m = re.match(r'(\w+):[Ff]', string)
            current = result.setdefault(m.group(1), []) if m else None
            continue
        if current is None:
            continue
        if code == N_LBRAC:
            current.append('{')
        elif code == N_RBRAC:
            current.append('}')
        elif re.search(r':[tT]', string):
            continue
        else:
            token = describe(code, string, value)
            if token:
                current.append(token)
    return result


def tree(tokens):
    """Nests the tokens into (variables, child scopes).

    In STABS a block's variables come just before its N_LBRAC. The function body is the outermost
    block, so the parameters and the body's locals end up in the returned scope. A block whose
    variables were all optimized away still gets its brace pair, so an empty scope means the source
    declared something there."""
    root = ([], [])
    stack = [root]
    pending = []
    for t in tokens:
        if t == '{':
            scope = (pending, [])
            pending = []
            stack[-1][1].append(scope)
            stack.append(scope)
        elif t == '}':
            stack[-1][0].extend(pending)
            pending = []
            if len(stack) > 1:
                stack.pop()
        else:
            pending.append(t)
    root[0].extend(pending)

    # Fold the function body block into the function.
    if len(root[1]) == 1 and not root[0]:
        body = root[1][0]
        root = (body[0], body[1])
    return root


def render(scope, slots=True):
    parts = [t if slots else t.split(':')[0] for t in scope[0]]
    parts += ['{ %s }' % render(child, slots) for child in scope[1]]
    return ' '.join(parts)


def shape(scope, ordered, strict):
    names = [t.split(':')[0] for t in scope[0]]
    if not ordered:
        names = sorted(names)
    children = []
    for child in scope[1]:
        sub = shape(child, ordered, strict)
        if not strict and not sub[0]:
            children.extend(sub[1])  # a block that describes nothing: keep only what it holds
        else:
            children.append(sub)
    return (tuple(names), tuple(children))


def main():
    flags = {a for a in sys.argv[1:] if a.startswith('-')}
    args = [a for a in sys.argv[1:] if not a.startswith('-')]
    verbose, ordered, strict = '-v' in flags, '-o' in flags, '-s' in flags
    want = args[0] if args else ''
    original = {}
    for fname, entries in original_entries():
        short = '/'.join(fname.split('/')[-2:])
        for func, tokens in functions(entries).items():
            original[(short, func)] = tree(tokens)
    differ = 0
    for unit, entries in our_entries():
        short = '/'.join(unit.split('/')[-2:])
        for func, tokens in functions(entries).items():
            orig = original.get((short, func))
            if orig is None or (want and want not in func and want not in unit):
                continue
            ours = tree(tokens)
            if shape(orig, ordered, strict) != shape(ours, ordered, strict) or (verbose and orig != ours):
                differ += 1
                print('%s %s' % (unit, func))
                print('  original: ' + render(orig))
                print('  ours:     ' + render(ours))
    print('%d function(s) differ' % differ)


if __name__ == '__main__':
    main()
