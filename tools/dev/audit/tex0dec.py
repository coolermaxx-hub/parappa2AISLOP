PSM={0:'SCE_GS_PSMCT32',1:'SCE_GS_PSMCT24',2:'SCE_GS_PSMCT16',10:'SCE_GS_PSMCT16S',19:'SCE_GS_PSMT8',20:'SCE_GS_PSMT4',27:'SCE_GS_PSMT8H',36:'SCE_GS_PSMT4HL',44:'SCE_GS_PSMT4HH'}
def tex0(v):
    f=[v&0x3fff,(v>>14)&0x3f,(v>>20)&0x3f,(v>>26)&0xf,(v>>30)&0xf,(v>>34)&1,(v>>35)&3,(v>>37)&0x3fff,(v>>51)&0xf,(v>>55)&1,(v>>56)&0x1f,(v>>61)&7]
    def h(x): return '0x%x'%x if x>9 else str(x)
    psm=PSM.get(f[2],str(f[2])); cpsm=PSM.get(f[8],str(f[8]))
    s='SCE_GS_SET_TEX0(%s, %d, %s, %d, %d, %d, %d, %s, %s, %d, %d, %d)'%(h(f[0]),f[1],psm,f[3],f[4],f[5],f[6],h(f[7]),cpsm,f[9],f[10],f[11])
    # round-trip check
    r=(f[0]|f[1]<<14|f[2]<<20|f[3]<<26|f[4]<<30|f[5]<<34|f[6]<<35|f[7]<<37|f[8]<<51|f[9]<<55|f[10]<<56|f[11]<<61)
    assert r==v,(hex(v),hex(r))
    return s
