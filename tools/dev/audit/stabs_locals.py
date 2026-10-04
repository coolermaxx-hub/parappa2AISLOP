"""List locals declared in the given C sources that the original debug info does not have for that function.

Usage: python3 tools/dev/audit/stabs_locals.py src/main/*.c
The declaration parser is a heuristic; check each hit with stabs.py before acting on it. Hits are often
readable aids worth keeping (typed casts of a void* parameter); remove or rename the ones that only steered
code generation."""
import re,subprocess,sys,os
S=os.path.dirname(os.path.abspath(__file__))
# parse STABS for all files
out=subprocess.run(['python3',S+'/stabs.py',''],capture_output=True,text=True).stdout.split('\n')
stabs={}  # (file, func) -> set(names)
cur=None; curfile=None
for l in out:
    if l.startswith('FILE'):
        curfile=l.split()[1]; cur=None; continue
    parts=l.split(' ',5)
    if len(parts)<6: continue
    name=parts[5]
    m=re.match(r'^(\w+):([Ff])\d',name)
    if m:
        cur=(curfile,m.group(1)); stabs.setdefault(cur,set()); continue
    if parts[1]=='2' and parts[2]=='5': cur=None; continue
    if cur:
        m=re.match(r'^(\w+):[A-Za-z]?\d',name) or re.match(r'^(\w+):[a-zA-Z]',name)
        if m and not name.startswith('$'):
            stabs[cur].add(m.group(1))
# source parse
DECL=re.compile(r'^\s+(?:static\s+|const\s+|register\s+|volatile\s+|unsigned\s+|signed\s+|struct\s+)*([A-Za-z_]\w*)((?:\s*\*)*)\s+(.+?);\s*(?:/[*/].*)?$')
KW={'return','goto','case','else','if','while','for','do','switch','break','continue','sizeof','printf','delete','new','typedef'}
def locals_of(body):
    names=[]
    depth=0
    for line in body.split('\n'):
        m=DECL.match(line)
        if m and m.group(1) not in KW and '(' not in m.group(3).split('=')[0]:
            rest=m.group(3)
            # split top-level commas
            parts=[];d=0;curp=''
            for ch in rest:
                if ch in '([{': d+=1
                if ch in ')]}': d-=1
                if ch==',' and d==0: parts.append(curp); curp=''
                else: curp+=ch
            parts.append(curp)
            for p in parts:
                p=p.split('=')[0].strip().lstrip('*').strip()
                p=re.sub(r'\[.*','',p).strip()
                if re.match(r'^[A-Za-z_]\w*$',p): names.append(p)
    return names
FUNC=re.compile(r'^(?:static\s+|/\* static \*/\s*|inline\s+)*[A-Za-z_][\w\s\*]*?\b([A-Za-z_]\w*)\s*\(([^;{]*)\)\s*\{\s*$', re.M)
for path in sys.argv[1:]:
    src=open(path,encoding='latin1').read()
    base=os.path.basename(path)
    for m in FUNC.finditer(src):
        fn=m.group(1)
        if fn in KW: continue
        # find body by brace matching
        i=m.end()-1; d=0
        for j in range(i,len(src)):
            if src[j]=='{': d+=1
            elif src[j]=='}':
                d-=1
                if d==0: break
        body=src[i+1:j]
        key=None
        for k in stabs:
            if k[1]==fn and (k[0]==base or k[0].endswith('/'+base) or os.path.basename(k[0])==base): key=k
        if key is None: continue
        params=set(re.findall(r'(\w+)\s*(?:\[[^\]]*\])?\s*(?:,|$)',m.group(2)))
        extra=[n for n in locals_of(body) if n not in stabs[key] and n not in params]
        if extra:
            ln=src[:m.start()].count('\n')+1
            print(f'{path}:{ln} {fn}: {sorted(set(extra))}')
