import subprocess,re,sys
def syms(obj,sec):
    out=subprocess.run(['mipsel-linux-gnu-objdump','-t',obj],capture_output=True,text=True).stdout
    r=[]
    for l in out.split('\n'):
        m=re.match(r'^([0-9a-f]{8}) (\S+)\s+(?:\S+\s+)?(\.\w+)\s+([0-9a-f]{8})\s+(\S+)$',l)
        if m and m.group(3)==sec and m.group(5)!=sec and 'NON_MATCHING' not in m.group(5) and not m.group(5).startswith('D_'): r.append((int(m.group(1),16),m.group(5)))
    return sorted(r)
def size(obj,sec):
    out=subprocess.run(['mipsel-linux-gnu-objdump','-h',obj],capture_output=True,text=True).stdout
    for l in out.split('\n'):
        p=l.split()
        if len(p)>2 and p[1]==sec: return int(p[2],16)
new,old,sec=sys.argv[1:4]
a=syms(new,sec); b=syms(old,sec)
bad=[(hex(x[0]),x[1],hex(y[0]),y[1]) for x,y in zip(a,b) if x[0]!=y[0]]
print(new.split('/')[-1],sec,'syms',len(a),len(b),'size',size(new,sec),size(old,sec),'MISMATCH' if bad or len(a)!=len(b) else 'ok',bad[:4])
