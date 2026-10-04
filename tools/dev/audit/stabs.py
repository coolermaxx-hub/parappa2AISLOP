"""Dump the .mdebug symbols of the original executable (iso/SCPS_150.17) for source files whose name contains argv[1].

The C units were built with -g, so their STABS give original local variable names, types (st 0 entries,
'name:r1' = register, 'name:123' = stack) and block scopes: a block's variables are listed just before its
$LBB entry. prlib units carry only procedure symbols. Run from the repository root."""
import struct,sys
f=open('iso/SCPS_150.17','rb').read()
# find .mdebug section offset via section headers
e_shoff=struct.unpack_from('<I',f,0x20)[0]; e_shentsize,e_shnum,e_shstrndx=struct.unpack_from('<HHH',f,0x2e)
secs=[struct.unpack_from('<IIIIIIIIII',f,e_shoff+i*e_shentsize) for i in range(e_shnum)]
shstr=secs[e_shstrndx][4]
def nm(o): return f[shstr+o:f.index(b'\0',shstr+o)].decode()
md=[s for s in secs if nm(s[0])=='.mdebug'][0]
off=md[4]
h=struct.unpack_from('<hh'+'i'*23,f,off)
names=['magic','vstamp','ilineMax','cbLine','cbLineOffset','idnMax','cbDnOffset','ipdMax','cbPdOffset','isymMax','cbSymOffset','ioptMax','cbOptOffset','iauxMax','cbAuxOffset','issMax','cbSsOffset','issExtMax','cbSsExtOffset','ifdMax','cbFdOffset','crfd','cbRfdOffset','iextMax','cbExtOffset']
H=dict(zip(names,h))
# offsets in HDRR are relative to file start (or to section in some toolchains); detect
base=off+0x60-H['cbLineOffset']
if H['cbFdOffset']<off: base=0
def fdr(i):
    o=H['cbFdOffset']+base+i*72
    v=struct.unpack_from('<IiiiiiiiiihhiiiiIii',f,o)
    return dict(zip(['adr','rss','issBase','cbSs','isymBase','csym','ilineBase','cline','ioptBase','copt','ipdFirst','cpd','iauxBase','caux','rfdBase','crfd','bits','cbLineOffset','cbLine'],v))
def sstr(fd,iss):
    o=H['cbSsOffset']+base+fd['issBase']+iss
    return f[o:f.index(b'\0',o)].decode('latin1')
want=sys.argv[1]
for i in range(H['ifdMax']):
    fd=fdr(i)
    fname=sstr(fd,fd['rss']) if fd['rss']>=0 else ''
    if want not in fname: continue
    print('FILE',fname,fd['csym'])
    for k in range(fd['csym']):
        o=H['cbSymOffset']+base+(fd['isymBase']+k)*12
        iss,value,bits=struct.unpack_from('<iiI',f,o)
        st=bits&0x3f; sc=(bits>>6)&0x1f; idx=bits>>12
        s=sstr(fd,iss) if iss>=0 else ''
        print(k,st,sc,hex(idx),value,s)
