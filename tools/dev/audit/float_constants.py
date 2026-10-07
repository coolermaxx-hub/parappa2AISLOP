"""Compare each function's float constants in the original objects (obj/) with
this build's (build/src/): immediates built with lui/ori and moved with mtc1,
and lwc1 loads from .lit4/.rodata (read from the object, or from the linked
executable for constants defined elsewhere). Mutable globals are excluded by
only reading .lit4 and .rodata addresses. Run from the repository root after a
build and ./configure.py --objdiff. NaN constants never compare equal.
"""
import subprocess,glob,os,re,struct
from collections import Counter
def section_bytes(o):
    out=subprocess.run(['mips-linux-gnu-readelf','-S','-W',o],capture_output=True,text=True).stdout
    secs={}
    data=open(o,'rb').read()
    for l in out.split('\n'):
        m=re.search(r'\]\s+(\.\S+)\s+\S+\s+\S+\s+([0-9a-f]+)\s+([0-9a-f]+)',l)
        if m: secs[m.group(1)]=data[int(m.group(2),16):int(m.group(2),16)+int(m.group(3),16)]
    return secs
def symtab(o):
    out=subprocess.run(['mips-linux-gnu-objdump','-t',o],capture_output=True,text=True).stdout.split('\n')
    s={}
    for l in out:
        p=l.split()
        if len(p)>=5 and p[-1] and not p[-1].startswith('.'):
            try: s[p[-1]]=(p[-3] if len(p)>=6 else p[-2], int(p[0],16))
            except: pass
    return s
import functools
@functools.lru_cache(None)
def elfimg(elf):
    data=open(elf,'rb').read()
    phoff=struct.unpack_from('<I',data,0x1c)[0]; phentsize,phnum=struct.unpack_from('<HH',data,0x2a)
    segs=[struct.unpack_from('<IIIII',data,phoff+i*phentsize) for i in range(phnum)]
    syms={}
    for l in subprocess.run(['mips-linux-gnu-nm',elf],capture_output=True,text=True).stdout.split('\n'):
        q=l.split()
        if len(q)==3: syms.setdefault(q[2],int(q[0],16))
    return data,segs,syms
@functools.lru_cache(None)
def constranges(elf):
    out=subprocess.run(['mips-linux-gnu-readelf','-S','-W',elf],capture_output=True,text=True).stdout
    r=[]
    for l in out.split('\n'):
        m=re.search(r'\]\s+(\.lit4|\.rodata)\s+\S+\s+([0-9a-f]+)\s+\S+\s+([0-9a-f]+)',l)
        if m: r.append((int(m.group(2),16),int(m.group(2),16)+int(m.group(3),16)))
    return r
def elfread(elf,name,off):
    data,segs,syms=elfimg(elf)
    if name not in syms: return None
    a=syms[name]+off
    if not any(lo<=a<hi for lo,hi in constranges(elf)): return None
    for t,po,va,_,fs in segs:
        if t==1 and va<=a<va+fs: return data[po+a-va:po+a-va+4]
    return None
def consts(o):
    elf='iso/SCPS_150.17' if o.startswith('obj/') else 'build/SCPS_150.17.elf'
    secs=section_bytes(o); st=symtab(o)
    out=subprocess.run(['mips-linux-gnu-objdump','-dr','--no-show-raw-insn','-M','no-aliases',o],capture_output=True,text=True).stdout.split('\n')
    d={};cur=None;lui={}
    for i,l in enumerate(out):
        m=re.match(r'^[0-9a-f]+ <(.+)>:$',l)
        if m:
            if not (cur and m.group(1).startswith('.L')): cur=m.group(1); d.setdefault(cur,Counter())
            lui={}; continue
        if not cur: continue
        m=re.match(r'\s*[0-9a-f]+:\s+lui\s+(\w+),(0x[0-9a-f]+|\d+)$',l)
        if m:
            nxt=out[i+1] if i+1<len(out) else ''
            if 'R_MIPS' not in nxt: lui[m.group(1)]=int(m.group(2),0)<<16
            continue
        m=re.match(r'\s*[0-9a-f]+:\s+ori\s+(\w+),(\w+),(0x[0-9a-f]+|\d+)$',l)
        if m and m.group(2) in lui: lui[m.group(1)]=lui[m.group(2)]|int(m.group(3),0); continue
        m=re.match(r'\s*[0-9a-f]+:\s+mtc1\s+(\w+),\$f\d+',l)
        if m and m.group(1) in lui:
            v=struct.unpack('<f',struct.pack('<I',lui[m.group(1)]&0xffffffff))[0]; d[cur][round(v,6)]+=1; continue
        if re.match(r'\s*[0-9a-f]+:\s+mtc1\s+zero',l): d[cur][0.0]+=1; continue
        m=re.match(r'\s*[0-9a-f]+:\s+lwc1\s+\$f\d+,(-?\d+)\((\w+)\)',l)
        if m:
            nxt=out[i+1] if i+1<len(out) else ''
            r=re.search(r'R_MIPS_(LO16|GPREL16|LITERAL)\s+(\S+?)(\+0x([0-9a-f]+))?$',nxt.strip())
            if r:
                sym=r.group(2); off=int(r.group(4),16) if r.group(4) else 0
                off+=int(m.group(1)) if r.group(1)!='LO16' else 0
                if sym in secs: sec,base=sym,0
                elif sym in st and st[sym][0] in secs: sec,base=st[sym]
                else:
                    b=elfread(elf,sym,off)
                    if b: d[cur][round(struct.unpack('<f',b)[0],6)]+=1
                    continue
                if sec in secs and sec in ('.lit4','.rodata','.sdata','.data'):
                    b=secs[sec][base+off:base+off+4]
                    if len(b)==4 and sec in ('.lit4','.rodata'):
                        d[cur][round(struct.unpack('<f',b)[0],6)]+=1
    return d
n=0
for o in sorted(glob.glob('obj/**/*.o',recursive=True)):
    b='build/src/'+o[4:]
    if not os.path.exists(b): continue
    co,cb=consts(o),consts(b)
    for f in sorted(set(co)&set(cb)):
        a,c=set(co[f]),set(cb[f])
        if a!=c:
            n+=1; print(o[4:-2], f[:48], 'orig-only', sorted(a-c)[:6], 'ours-only', sorted(c-a)[:6])
print(n,'functions differ in float constants')
