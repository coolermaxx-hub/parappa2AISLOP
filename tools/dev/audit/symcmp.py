import subprocess,sys,re
def syms(elf):
    d={}
    for l in subprocess.run(['nm','-S','-n',elf],capture_output=True,text=True).stdout.split('\n'):
        t=l.split()
        if len(t)==4 and 'NON_MATCHING' not in t[3]: d.setdefault(t[3],[]).append((int(t[0],16),int(t[1],16)))
    return d
def secs(elf):
    out=subprocess.run(['readelf','-S','-W',elf],capture_output=True,text=True).stdout
    r=[]
    for l in out.split('\n'):
        m=re.match(r'\s*\[\s*\d+\]\s+(\S+)\s+PROGBITS\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)',l)
        if m: r.append((m.group(1),int(m.group(2),16),int(m.group(3),16),int(m.group(4),16)))
    return r
def rd(elf,addr,size):
    for n,a,o,s in secs(elf):
        if a<=addr<a+s:
            return open(elf,'rb').read()[o+addr-a:o+addr-a+size]
new,old=sys.argv[1],sys.argv[2]
names=sys.argv[3:]
sn,so=syms(new),syms(old)
for n in names:
    if n not in sn or n not in so: print(n,'missing',n in sn,n in so); continue
    a,sa=sn[n][0]; b,sb=so[n][0]
    da=rd(new,a,sa); db=rd(old,b,sb)
    diff=[i for i in range(min(len(da),len(db))) if da[i]!=db[i]]
    print(n,'size',sa,sb,'diffbytes',len(diff),diff[:8])
