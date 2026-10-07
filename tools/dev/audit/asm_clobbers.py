"""List inline asm blocks that write a hard-coded register without declaring it
clobbered, or that write an operand declared only as an input.

Usage: python3 tools/dev/audit/asm_clobbers.py <source files>

Both are undefined for GCC: the compiler may keep a live value in the register
or reuse the input after the asm. Such blocks only work while the allocation
happens to avoid them (the tone colour helpers in menu/pksprite.c did). The
first operand of each instruction is taken as its destination (the second for
mtc1/ctc1), and stores, branches and VU instructions are skipped, so check
every hit by hand. Run from the repository root."""
import re, sys
from pathlib import Path
NAMES = {'zero':0,'at':1,'v0':2,'v1':3,'a0':4,'a1':5,'a2':6,'a3':7,'t0':8,'t1':9,'t2':10,'t3':11,'t4':12,'t5':13,'t6':14,'t7':15,
         's0':16,'s1':17,'s2':18,'s3':19,'s4':20,'s5':21,'s6':22,'s7':23,'t8':24,'t9':25,'k0':26,'k1':27,'gp':28,'sp':29,'fp':30,'s8':30,'ra':31}
NOWRITE = re.compile(r'^(s[bhwdq]|swc1|sqc2|sdl|sdr|swl|swr|sync(\.\w)?|j|jr|jal|b\w*|qmtc2(\.\w+)?|ctc\d|vsqi|vsqd|lqc2|cache|nop|mt(hi|lo)\d?|c\.\w+\.s|di|ei|v(?!.*\$\d)\w*)$')
def reg(tok):
    tok = tok.strip()
    m = re.match(r'^\$f(\d+)$', tok)
    if m: return 'f' + m.group(1)
    m = re.match(r'^\$(\d+)$', tok)
    if m: return int(m.group(1))
    m = re.match(r'^\$([a-z]\w*)$', tok)
    if m and m.group(1) in NAMES: return NAMES[m.group(1)]
    return None
STR = re.compile(r'"((?:[^"\\]|\\.)*)"')
for path in sys.argv[1:]:
    src = Path(path).read_text(errors='replace')
    for m in re.finditer(r'\b(?:asm|__asm__)\s*(?:volatile|__volatile__)?\s*\(', src):
        depth, i = 1, m.end()
        while depth and i < len(src):
            if src[i] == '(': depth += 1
            elif src[i] == ')': depth -= 1
            elif src[i] == '"':
                i += 1
                while src[i] != '"':
                    i += 2 if src[i] == '\\' else 1
            i += 1
        body = src[m.end():i - 1]
        # Replace string literals by placeholders so ':' inside them does not split sections.
        lits = []
        masked = STR.sub(lambda x: (lits.append(x.group(1)), '"@%d"' % (len(lits) - 1))[1], body)
        masked = re.sub(r'/\*.*?\*/', '', masked, flags=re.S)
        sections = masked.split(':')
        template = ''.join(lits[int(n)] for n in re.findall(r'"@(\d+)"', sections[0]))
        outputs = re.findall(r'"@(\d+)"\s*\(', sections[1]) if len(sections) > 1 else []
        inputs = re.findall(r'"@(\d+)"\s*\(', sections[2]) if len(sections) > 2 else []
        n_out = len(outputs)
        clob = sections[3] if len(sections) > 3 else ''
        declared = {reg(lits[int(n)]) for n in re.findall(r'"@(\d+)"', clob)}
        written, inwrites = set(), set()
        for ins in re.split(r'\\n|;', template):
            ins = ins.replace('\\t', ' ').strip()
            if not ins or ins.startswith('.') or ins.endswith(':'): continue
            op, _, args = ins.partition(' ')
            if NOWRITE.match(op): continue
            ops = [a.strip() for a in args.split(',')]
            first = ops[0]
            if op in ('mtc1', 'ctc1') and len(ops) > 1:
                first = ops[1]
            r = reg(first)
            if r not in (None, 0): written.add(r)
            mo = re.match(r'^%(\d+)$', first)
            if mo and int(mo.group(1)) >= n_out: inwrites.add(int(mo.group(1)))
        line = src.count('\n', 0, m.start()) + 1
        if written - declared:
            print('%s:%d writes %s, undeclared' % (path, line, sorted(map(str, written - declared))))
        if inwrites:
            print('%s:%d writes input operand(s) %s' % (path, line, sorted(inwrites)))
