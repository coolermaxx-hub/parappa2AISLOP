"""Compare a section of the linked executable with the original, word by word.

Usage: python3 tools/dev/audit/data_words.py .data .sdata .lit4

Each differing word is classed as a data pointer moved by the section's
address shift, a code pointer to the same function and offset, or other. Run
from the repository root after a build; needs iso/SCPS_150.17.
"""
import struct,subprocess,bisect,sys
def load(elf):
    data=open(elf,'rb').read()
    out=subprocess.run(['mips-linux-gnu-readelf','-S','-W',elf],capture_output=True,text=True).stdout
    secs={}
    for l in out.split('\n'):
        if ']' not in l: continue
        p=l.split(']')[1].split()
        if len(p)>=6 and p[0].startswith('.'):
            secs[p[0]]=(int(p[2],16),int(p[3],16),int(p[4],16),p[1])
    syms=[]
    for l in subprocess.run(['mips-linux-gnu-nm','-n',elf],capture_output=True,text=True).stdout.split('\n'):
        q=l.split()
        if len(q)==3 and q[1] in 'tTW' and not q[2].startswith(('$','.L','gcc2','__gnu')): syms.append((int(q[0],16),q[2]))
    syms.sort()
    return data,secs,syms
O=load('iso/SCPS_150.17'); N=load('build/SCPS_150.17.elf')
def text_sym(img,a):
    data,secs,syms=img
    lo,_,sz,_=secs['.text']
    if not lo<=a<lo+sz: return None
    i=bisect.bisect_right([s for s,_ in syms],a)-1
    return (syms[i][1].replace('.NON_MATCHING',''), a-syms[i][0]) if i>=0 else None
def in_data(img,a):
    data,secs,syms=img
    for n in ('.data','.rodata','.sdata','.sbss','.bss','.lit4','.gcc_except_table','.vutext','.mfifo'):
        if n in secs and secs[n][0]<=a<secs[n][0]+secs[n][2]: return n
    return None
for name in sys.argv[1:]:
    oa,oo,osz,_=O[1][name]; na,no,nsz,_=N[1][name]
    ob=O[0][oo:oo+osz]; nb=N[0][no:no+nsz]
    stats={'same':0,'datashift':0,'textptr':0,'other':0}; others=[]
    for i in range(0,min(osz,nsz)//4*4,4):
        x,=struct.unpack_from('<I',ob,i); y,=struct.unpack_from('<I',nb,i)
        if x==y: stats['same']+=1; continue
        if y-x==na-oa and in_data(O,x): stats['datashift']+=1; continue
        tx,ty=text_sym(O,x),text_sym(N,y)
        if tx and ty and tx==ty: stats['textptr']+=1; continue
        stats['other']+=1; others.append((oa+i,x,y,tx,ty))
    print(name,'size',osz,nsz,stats)
    for a,x,y,tx,ty in others[:15]: print('   %#x orig %#010x ours %#010x  %s %s'%(a,x,y,tx,ty))
