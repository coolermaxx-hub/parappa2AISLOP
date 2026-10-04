"""Compare struct, union and enum definitions in the original debug info with ours.

The original C units (main, menu, os, dbug) were built with -g, and so are ours (the .s files the
build leaves next to each object carry .stabs directives). For every type name defined in both,
print the members whose name, offset or count differ. Run from the repository root after a build.

    python3 tools/dev/audit/stabs_types.py [name-substring]

Field types are not compared, only names and bit offsets. Anonymous types are skipped."""
import glob
import re
import struct
import sys


def original_stabs():
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
        strs = []
        for k in range(fd['csym']):
            p = H['cbSymOffset'] + base + (fd['isymBase'] + k) * 12
            iss, value, bits = struct.unpack_from('<iiI', f, p)
            if iss >= 0:
                strs.append(sstr(iss))
        yield fname, join_continuations(strs)


def our_stabs():
    for path in sorted(glob.glob('build/src/**/*.c.o.s', recursive=True)):
        strs = []
        for line in open(path, encoding='latin1'):
            m = re.match(r'\s*\.stabs\s+"((?:[^"\\]|\\.)*)"', line)
            if m:
                strs.append(m.group(1).replace('\\\\', '\\'))
        yield path, join_continuations(strs)


def join_continuations(strs):
    out, cur = [], ''
    for s in strs:
        if s.endswith('\\'):
            cur += s[:-1]
        else:
            out.append(cur + s)
            cur = ''
    return out


class Parser:
    def __init__(self, s):
        self.s, self.i = s, 0

    def peek(self):
        return self.s[self.i] if self.i < len(self.s) else ''

    def num(self):
        m = re.compile(r'-?\d+').match(self.s, self.i)
        self.i = m.end()
        return int(m.group())

    def expect(self, c):
        assert self.s[self.i] == c, (self.s, self.i, c)
        self.i += 1

    def type(self):
        """Returns a struct/union/enum body tuple when the type defines one, else None."""
        if self.peek() == '(':
            self.i = self.s.index(')', self.i) + 1
        else:
            self.num()
        if self.peek() != '=':
            return None
        self.i += 1
        return self.typedef()

    def typedef(self):
        c = self.peek()
        if c.isdigit() or c == '(' or c == '-':
            return self.type()
        self.i += 1
        if c in 'su':
            size = self.num()
            fields = []
            while self.peek() != ';':
                name = self.s[self.i:self.s.index(':', self.i)]
                self.i += len(name) + 1
                self.type()
                self.expect(',')
                bitpos = self.num()
                self.expect(',')
                self.num()
                self.expect(';')
                fields.append((name, bitpos))
            self.expect(';')
            return (c, size, fields)
        if c == 'e':
            vals = []
            while self.peek() != ';':
                name = self.s[self.i:self.s.index(':', self.i)]
                self.i += len(name) + 1
                vals.append((name, self.num()))
                self.expect(',')
            self.expect(';')
            return ('e', 0, vals)
        if c in '*fkB':
            self.type()
            return None
        if c == 'a':
            self.typedef()  # index range
            self.type()
            return None
        if c == 'r':
            self.type()
            self.expect(';')
            self.s.index(';', self.i)
            self.i = self.s.index(';', self.i) + 1
            self.i = self.s.index(';', self.i) + 1
            return None
        if c == 'x':
            self.i = self.s.index(':', self.i) + 1
            return None
        raise ValueError('unhandled type %r at %d in %r' % (c, self.i, self.s))


def collect(units):
    types = {}
    for unit, strs in units:
        for s in strs:
            m = re.match(r'([A-Za-z_]\w*):[tT]', s)
            if not m:
                continue
            p = Parser(s)
            p.i = m.end()
            try:
                body = p.type()
            except (ValueError, AssertionError, AttributeError):
                continue
            if body is not None:
                types.setdefault(m.group(1), (body, unit))
    return types


def main():
    want = sys.argv[1] if len(sys.argv) > 1 else ''
    orig = collect(original_stabs())
    ours = collect(our_stabs())
    for name in sorted(set(orig) & set(ours)):
        if want not in name:
            continue
        (ok, osize, ofields), ounit = orig[name]
        (nk, nsize, nfields), nunit = ours[name]
        if ofields == nfields and osize == nsize:
            continue
        print('%s (%s, ours %s)' % (name, ounit, nunit))
        if osize != nsize:
            print('  size %d, ours %d' % (osize, nsize))
        odict = {pos: n for n, pos in ofields} if ok != 'e' else None
        if ok == 'e':
            oset, nset = dict(ofields), dict(nfields)
            for n, v in ofields:
                if n not in nset:
                    print('  enum %s = %d missing' % (n, v))
            for n, v in nfields:
                if n not in oset:
                    print('  enum %s = %d not original' % (n, v))
            continue
        npos = {}
        for n, pos in nfields:
            npos.setdefault(pos, []).append(n)
        opos = {}
        for n, pos in ofields:
            opos.setdefault(pos, []).append(n)
        for pos in sorted(set(opos) | set(npos)):
            a, b = opos.get(pos, []), npos.get(pos, [])
            if a != b:
                print('  0x%x: original %s, ours %s' % (pos // 8, ', '.join(a) or '-', ', '.join(b) or '-'))


main()
