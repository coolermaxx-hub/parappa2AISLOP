import subprocess,sys,re,os
def funcs(o):
    out=subprocess.run(["mips-linux-gnu-objdump","-dr","--no-show-raw-insn","-M","no-aliases",o],capture_output=True,text=True).stdout
    d={};cur=None
    for l in out.split("\n"):
        m=re.match(r"^[0-9a-f]+ <(.+)>:$",l)
        if m: cur=m.group(1);d[cur]=[];continue
        if cur and l.strip():
            l=re.sub(r"^\s*[0-9a-f]+:\s*","",l)
            l=re.sub(r"[0-9a-f]+ <([^>+]+)(\+0x[0-9a-f]+)?>",r"\1\2",l)
            d[cur].append(l)
    return d
def norm(lines):
    out=[]
    for i,l in enumerate(lines):
        if l.startswith('R_MIPS'):
            # normalize previous instruction's immediate
            if out: out[-1]=re.sub(r'(-?\d+)\(([a-z0-9]+)\)$',r'IMM(\2)',out[-1]) if re.search(r'\(\w+\)$',out[-1]) else re.sub(r',(-?\d+|0x[0-9a-f]+)$',',IMM',out[-1])
            out.append('RELOC')
        else: out.append(l)
    return out
for f in sys.argv[1:]:
    b='build/src/'+f; e='expected2/'+b
    fb,fe=funcs(b),funcs(e)
    bad=[n for n in fe if n in fb and norm(fb[n])!=norm(fe[n])]
    print(f,'real diffs',len(bad),bad[:8])
