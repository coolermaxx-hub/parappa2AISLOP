"""Compare each function's callees in the original objects (obj/) with this
build's (build/src/). Jump-table label pieces are merged into their function;
nalib template instances and per-file helper copies are ignored, since those are
known to differ. Plain jumps (tail calls) are not counted, so a tail call shows
up as a missing callee: check each hit by hand. Run from the repository root
after a build and ./configure.py --objdiff.
"""
import subprocess,glob,os,re
from collections import Counter
SKIP=re.compile(r'^(func_|Set__t8NaMATRIX|Set__t8NaVECTOR|RotateMatrix__t8|ScaleMatrix__t8|TranslateMatrix__t8|Scale__t8|Translate__t8|__t8NaVECTOR|__t8NaMATRIX|_\$_|__builtin_|AddGsAD|OpenGifTag|CloseGifTag|Copy__|Apply__|Multiply__)')
def calls(o):
    out=subprocess.run(['mips-linux-gnu-objdump','-dr','--no-show-raw-insn',o],capture_output=True,text=True).stdout.split('\n')
    d={};cur=None
    for i,l in enumerate(out):
        m=re.match(r'^[0-9a-f]+ <(.+)>:$',l)
        if m:
            if not (cur and m.group(1).startswith('.L')): cur=m.group(1); d.setdefault(cur,Counter())
            continue
        m=re.search(r'\bjal\s+[0-9a-f]+ <([^>+]+)',l)
        if m and cur:
            nxt=out[i+1] if i+1<len(out) else ''
            r=re.search(r'R_MIPS_26\s+(\S+)',nxt)
            name=m.group(1) if (not r or r.group(1).startswith('.')) else r.group(1)
            if not SKIP.match(name): d[cur][name]+=1
    return d
n=0
for o in sorted(glob.glob('obj/**/*.o',recursive=True)):
    b='build/src/'+o[4:]
    if not os.path.exists(b): continue
    co,cb=calls(o),calls(b)
    for f in sorted(set(co)&set(cb)):
        a,c=set(co[f]),set(cb[f])
        if a!=c:
            n+=1; print(o[4:-2], f[:50], 'orig-only', sorted(a-c), 'ours-only', sorted(c-a))
print(n,'functions with different callee sets')
