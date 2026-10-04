import subprocess,re,sys
def sec(obj,name):
    out=subprocess.run(['mipsel-linux-gnu-objdump','-s','-j',name,obj],capture_output=True,text=True).stdout
    b=bytearray()
    for l in out.split('\n'):
        t=l.split()
        if len(t)>=2 and re.fullmatch(r'[0-9a-f]{4,8}',t[0]):
            for x in t[1:5]:
                if re.fullmatch(r'[0-9a-f]{2,8}',x): b+=bytes.fromhex(x)
    return bytes(b)
def relocs(obj,name):
    out=subprocess.run(['mipsel-linux-gnu-objdump','-r','-j',name,obj],capture_output=True,text=True).stdout
    r={}
    for l in out.split('\n'):
        m=re.match(r'^([0-9a-f]+) (R_MIPS_\w+)\s+(\S+)',l)
        if m: r[int(m.group(1),16)]=(m.group(2),m.group(3))
    return r
new,old,secn=sys.argv[1],sys.argv[2],sys.argv[3]
a=sec(new,secn);b=sec(old,secn)
ra=relocs(new,secn);rb=relocs(old,secn)
print('sizes',len(a),len(b),'relocs',len(ra),len(rb))
n=min(len(a),len(b));bad=[]
mask=set(ra)|set(rb)
for i in range(n):
    if a[i]!=b[i] and not any(o<=i<o+4 for o in mask): bad.append(i)
print('byte diffs outside relocs',len(bad),[hex(x) for x in bad[:10]])
print('reloc offset sets equal',set(ra)==set(rb), len(set(ra)^set(rb)))
if set(ra)!=set(rb): print(sorted(set(ra)^set(rb))[:10])
