import re,subprocess,sys
def ld(f):
    out=subprocess.run(['mipsel-linux-gnu-nm','-n',f],capture_output=True,text=True).stdout
    d={}
    for l in out.split('\n'):
        p=l.split()
        if len(p)==3 and p[1] in 'dDbBsSrRgG' : d.setdefault(p[2],int(p[0],16))
    return d
old=ld('expected2/build/SCPS_150.17.elf'); new=ld('build/SCPS_150.17.elf')
lo,hi=int(sys.argv[1],16),int(sys.argv[2],16)
rows=sorted((old[n],new[n]-old[n],n) for n in old if n in new and lo<=old[n]<hi)
prev=None
for a,d,n in rows:
    if d!=prev: print('%08x delta %+x  %s'%(a,d,n)); prev=d
