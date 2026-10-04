import re,struct,sys,os,glob
ROOT='/home/claude/parappadev/parappa2'
# ---------- C type model ----------
BASIC={'char':(1,True),'u_char':(1,False),'unsigned char':(1,False),'short':(2,True),'u_short':(2,False),'unsigned short':(2,False),
 'int':(4,True),'u_int':(4,False),'unsigned int':(4,False),'unsigned':(4,False),'long':(8,True),'u_long':(8,False),'float':(4,None),'void':(1,False),'s8':(1,True),'u8':(1,False),'s16':(2,True),'u16':(2,False),'s32':(4,True),'u32':(4,False)}
structs={}
enums=set()
alias={}
def load_headers(extra=[]):
    files=glob.glob(ROOT+'/include/**/*.h',recursive=True)+glob.glob(ROOT+'/src/**/*.h',recursive=True)+extra
    for f in files:
        try: t=open(f).read()
        except: continue
        parse_types(t)
    structs.setdefault('sceCdlFILE',[('u_int','lsn',[]),('u_int','size',[]),('char','name',[16]),('u_char','date',[8]),('u_int','flag',[])])
    for a,b in alias.items():
        if b in structs and a not in structs: structs[a]=structs[b]
def parse_types(t):
    t=re.sub(r'//[^\n]*','',t)
    for m in re.finditer(r'typedef\s+(\w+)\s+(\w+)\s*;',t): alias[m.group(2)]=m.group(1)
    for m in re.finditer(r'typedef\s+enum\s*(?:\w+\s*)?\{[^}]*\}\s*(\w+)\s*;',t): enums.add(m.group(1))
    for m in re.finditer(r'typedef\s+struct\s*(\w*)\s*\{(.*?)\}\s*(\w+)\s*;',t,re.S):
        body=m.group(2)
        mem=[]
        for line in re.split(r';',body):
            line=re.sub(r'/\*.*?\*/','',line,flags=re.S).strip()
            if not line: continue
            mm=re.match(r'^(.*?)(\**)\s*(\w+)((?:\[[^\]]*\])*)(?:\s*:\s*\d+)?$',line.replace('\n',' '))
            if not mm: mem=None;break
            ty=mm.group(1).strip()+mm.group(2)
            dims=[int(x) if x.strip().isdigit() else 0 for x in re.findall(r'\[([^\]]*)\]',mm.group(4))]
            mem.append((ty.replace(' *','*').strip(),mm.group(3),dims))
        if mem is not None: structs[m.group(3)]=mem
def tsize(ty):
    ty=ty.replace('const ','').strip()
    if ty.endswith('*'): return 4,4
    if ty in BASIC: s=BASIC[ty][0]; return s,min(s,8) if s<8 else 8
    if ty in enums: return 4,4
    if ty in structs:
        off=0;al=1
        for (t,n,d) in structs[ty]:
            s,a=tsize(t)
            cnt=1
            for x in d: cnt*=x
            off=(off+a-1)//a*a; off+=s*cnt; al=max(al,a)
        return (off+al-1)//al*al,al
    raise Exception('unknown type '+ty)
# ---------- float formatting ----------
def f32(x):
    if x!=x: raise Exception('nan')
    xb=struct.unpack('<f',struct.pack('<f',x))[0]
    for p in range(1,10):
        s='%.*g'%(p,xb)
        if struct.unpack('<f',struct.pack('<f',float(s)))[0]==xb:
            if '.' not in s and 'e' not in s: s+='.0'
            return s+'f'
# ---------- asm parsing ----------
def parse_asm(path):
    labels={};order=[];cur=None
    for l in open(path).read().split('\n'):
        m=re.match(r'dlabel (\w+)',l)
        if m: cur=m.group(1);labels[cur]={'data':bytearray(),'relocs':{}};order.append(cur);continue
        if l.startswith('enddlabel'): cur=None;continue
        if cur is None: continue
        d=labels[cur]
        m=re.search(r'\.word (.+?)\s*$',l)
        if m:
            v=m.group(1).strip()
            if re.fullmatch(r'0x[0-9A-Fa-f]+',v): d['data']+=struct.pack('<I',int(v,16))
            else:
                mm=re.fullmatch(r'(\w+)(?: \+ (0x[0-9A-Fa-f]+))?',v)
                d['relocs'][len(d['data'])]=(mm.group(1),int(mm.group(2),16) if mm.group(2) else 0)
                d['data']+=b'\0\0\0\0'
            continue
        m=re.search(r'\.short (0x[0-9A-Fa-f]+)',l)
        if m: d['data']+=struct.pack('<H',int(m.group(1),16));continue
        m=re.search(r'\.byte (0x[0-9A-Fa-f]+)',l)
        if m: d['data']+=struct.pack('<B',int(m.group(1),16));continue
        m=re.search(r'\.float (\S+)',l)
        if m: d['data']+=struct.pack('<f',float(m.group(1)));continue
        m=re.search(r'\.asciz "(.*)"\s*$',l)
        if m:
            s=m.group(1).encode('latin1').decode('unicode_escape').encode('latin1')
            d['data']+=s+b'\0';continue
        m=re.search(r'\.ascii "(.*)"\s*$',l)
        if m:
            s=m.group(1).encode('latin1').decode('unicode_escape').encode('latin1')
            d['data']+=s;continue
        m=re.search(r'\.space (0x[0-9A-Fa-f]+|\d+)',l)
        if m: d['data']+=b'\0'*int(m.group(1),0);continue
        m=re.search(r'\.double (\S+)',l)
        if m: d['data']+=struct.pack('<d',float(m.group(1)));continue
        m=re.match(r'^\s*\.align (\d+)',l)
        if m:
            a=1<<int(m.group(1))
            while len(d['data'])%a: d['data']+=b'\0'
            continue
        if re.search(r'^\s*\.(align|section)|^\s*/\*|^\s*$|nonmatching|\.include|\.set',l): continue
        if l.strip() and not l.strip().startswith('/*'): print('UNPARSED',l,file=sys.stderr)
    return labels,order

# ---------- declarations ----------
def find_decls(srcfile, names):
    """map name -> (type, dims, is_static_comment, line_index) from extern lines in srcfile"""
    lines=open(srcfile).read().split('\n')
    out={}
    for i,l in enumerate(lines):
        m=re.match(r'^(?:/\*[^*]*\*/\s*)?(?://\s*)?extern\s+(const\s+)?([A-Za-z_][\w ]*?)\s*(\**)\s*(\w+)((?:\[[^\]]*\])*)\s*;(.*)$',l)
        if not m: continue
        name=m.group(4)
        if name not in names: continue
        ty=(m.group(2).strip()+m.group(3)).replace(' *','*')
        dims=[int(x) if x.strip().isdigit() else 0 for x in re.findall(r'\[([^\]]*)\]',m.group(5))]
        out[name]={'type':ty,'dims':dims,'const':bool(m.group(1)),'line':i,'static':'/* static */' in m.group(6),'text':l}
    return out
class Ctx: pass
def fmt_int(v,signed,size):
    if signed and v>=1<<(size*8-1): v-=1<<(size*8)
    if (not signed) and v>=0x10000: return '0x%x'%v
    if signed and v<0: return str(v)
    return str(v)
def scalar(ty,buf,off,relocs,ctx):
    base=ty.replace('const ','').strip()
    if base.endswith('*'):
        if off in relocs: return ctx.ptr(relocs[off],base)
        v=struct.unpack_from('<I',buf,off)[0]
        if v==0: return 'NULL'
        return '(%s)0x%X'%(base,v)
    if base=='float':
        if off in relocs: raise Exception('float reloc')
        return f32(struct.unpack_from('<f',buf,off)[0])
    if base in enums:
        return str(struct.unpack_from('<i',buf,off)[0])
    sz,sg=BASIC[base]
    fmt={1:'B',2:'H',4:'I',8:'Q'}[sz]
    v=struct.unpack_from('<'+fmt,buf,off)[0]
    if base in('char','u_char') :
        pass
    return fmt_int(v,sg,sz)
def emit(ty,dims,buf,off,relocs,ctx,indent=0):
    """returns (text,size)"""
    base=ty.replace('const ','').strip()
    if dims:
        n=dims[0]; sub=dims[1:]
        es,ea=tsize(base) ; 
        for d in sub: es*=d
        if n==0: n=(len(buf)-off)//es
        items=[]
        for i in range(n):
            items.append(emit(base,sub,buf,off+i*es,relocs,ctx,indent+4)[0])
        simple=all('\n' not in x and not x.startswith('{') for x in items)
        if simple:
            lines=[];cur=''
            for x in items:
                piece=x+', '
                if len(cur)+len(piece)>96: lines.append(cur.rstrip());cur=''
                cur+=piece
            if cur: lines.append(cur.rstrip())
            if len(lines)==1: return '{ '+lines[0].rstrip(',')+' }',es*n
            return '{\n'+'\n'.join(' '*(indent+4)+l for l in lines)+'\n'+' '*indent+'}',es*n
        return '{\n'+'\n'.join(' '*(indent+4)+x+',' for x in items)+'\n'+' '*indent+'}',es*n
    if base in structs:
        size,al=tsize(base)
        parts=[];o=0
        for (t,n,d) in structs[base]:
            s,a=tsize(t)
            o=(o+a-1)//a*a
            cnt=1
            for x in d: cnt*=x
            txt,_=emit(t,d,buf,off+o,relocs,ctx,indent+4)
            parts.append(txt)
            o+=s*cnt
        one='{ '+', '.join(parts)+' }'
        if '\n' not in one and len(one)<110: return one,size
        return '{\n'+'\n'.join(' '*(indent+4)+p+',' for p in parts)+'\n'+' '*indent+'}',size
    return scalar(base,buf,off,relocs,ctx),tsize(base)[0]

def parse_symaddrs():
    d={}
    for l in open(ROOT+'/config/p3.jul12.symbol_addrs.txt'):
        m=re.match(r'^(\w+)\s*=\s*(0x[0-9a-fA-F]+);\s*(.*)$',l.strip())
        if m:
            sz=re.search(r'size:(0x[0-9a-fA-F]+)',m.group(3))
            d.setdefault(m.group(1),[]).append((int(m.group(2),16),int(sz.group(1),16) if sz else None))
    return d
def parse_asm_addrs(path):
    """label -> vram of first item"""
    out={};cur=None
    for l in open(path).read().split('\n'):
        m=re.match(r'dlabel (\w+)',l)
        if m: cur=m.group(1);continue
        if cur and cur not in out:
            m=re.search(r'/\* [0-9A-F]+ ([0-9A-F]{8})',l)
            if m: out[cur]=int(m.group(1),16)
    return out
