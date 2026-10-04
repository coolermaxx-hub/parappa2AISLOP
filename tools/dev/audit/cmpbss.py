import subprocess,sys,re
def syms(obj,sec):
    out=subprocess.run(['mipsel-linux-gnu-objdump','-t',obj],capture_output=True,text=True).stdout
    r=[]
    for l in out.split('\n'):
        m=re.match(r'^([0-9a-f]{8}) \S+\s+\S*\s*(\.\w+)\s+([0-9a-f]{8})\s+(\S+)$',l)
        if m and m.group(2)==sec: r.append((int(m.group(1),16),m.group(4)))
    return sorted(r)
new,old,sec=sys.argv[1:4]
oldsec=sys.argv[4] if len(sys.argv)>4 else sec
a=syms(new,sec); b=syms(old,oldsec)
b=[x for x in b if not x[1].startswith('D_')]
print(len(a),len(b))
for x,y in zip(a,b):
    flag='' if x[0]==y[0] else '   <<<<'
    print('%x %-22s | %x %s%s'%(x[0],x[1],y[0],y[1],flag))
