import re,glob,os,collections
vu=[];nm=[];noc=[]
for f in sorted(glob.glob('src/*/*.c')+glob.glob('src/*/*.cpp')):
    if '/old/' in f: continue
    s=open(f).read()
    lines=s.split('\n')
    for i,l in enumerate(lines):
        m=re.match(r'INCLUDE_ASM\("([^"]+)", ([^)]+)\);',l)
        if not m: continue
        d,fn=m.groups()
        asm=f'{d}/{fn}.s'
        n=sum(1 for x in open(asm) if '*/  ' in x) if os.path.exists(asm) else 0
        unit=f[4:]
        # NON_MATCHING with a C body: next non-empty line is #else
        j=i+1
        while j<len(lines) and lines[j].strip()=='' : j+=1
        prev=lines[i-1].strip() if i>0 else ''
        if 'renderee' in f: vu.append((unit,fn,n))
        elif prev=='#ifndef NON_MATCHING' and lines[j].startswith('#else'): nm.append((unit,fn,n))
        else: noc.append((unit,fn,n))
print(len(vu),len(nm),len(noc))
out=[]
out.append('| File | Function | Insns |\n| --- | --- | --- |')
for u,fn,n in noc: out.append(f'| `{u}` | `{fn}` | {n} |')
open('/tmp/noc.md','w').write('\n'.join(out)+'\n')
c=collections.Counter(u for u,_,_ in nm)
open('/tmp/nm.md','w').write('\n'.join(f'- `{u}`: {k}' for u,k in c.most_common())+'\n')
