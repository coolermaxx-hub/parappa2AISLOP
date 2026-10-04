import re,sys
f,label=sys.argv[1],sys.argv[2]
t=open(f).read()
m=re.search(r'dlabel %s\n(.*?)enddlabel %s'%(label,label),t,re.S)
w=[int(x,16) for x in re.findall(r'\.word (0x[0-9A-Fa-f]+)',m.group(1))]
R={0:'PRIM',1:'RGBAQ',2:'ST',3:'UV',5:'XYZ2',6:'TEX0_1',8:'CLAMP_1',0x14:'TEX1_1',0x18:'XYOFFSET_1',0x3f:'TEXFLUSH',0x40:'SCISSOR_1',0x42:'ALPHA_1',0x44:'DIMX',0x45:'DTHE',0x46:'COLCLAMP',0x47:'TEST_1',0x49:'PABE',0x4a:'FBA_1',0x4c:'FRAME_1',0x4e:'ZBUF_1',0x34:'MIPTBP1_1',0x3b:'TEXA',0x3d:'FOGCOL'}
for i in range(0,len(w),4):
    q=w[i:i+4]
    if len(q)<4: print(i//4,q);continue
    print(i//4,'val=0x%x'%(q[0]|q[1]<<32),'addr=0x%x %s'%(q[2],R.get(q[2],'?')),'| words',[hex(x) for x in q])
