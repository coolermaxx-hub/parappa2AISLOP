#!/bin/bash
# perm_import.sh func : import NON_MATCHING func from menusub into nonmatchings/
cd "$(dirname "$0")/../.."
source .venv/bin/activate
f=$1
cp src/menu/menusub.c /tmp/ms_bak_$f.c
python3 - "$f" <<'E'
import sys
p='src/menu/menusub.c'; f=sys.argv[1]
s=open(p).read()
a='#ifndef NON_MATCHING\nINCLUDE_ASM("'
i=s.index(f+');\n#else'); st=s.rindex(a,0,i)
k=s.index('\n',i+len(f+');\n#else'))+1; s2=s[:st]+s[k:]
e=s2.index('#endif',st); s2=s2[:e]+s2[e+len('#endif'):]
s2=s2.replace('#include <math.h>','float sinf(float);\nfloat cosf(float);')
open(p,'w').write(s2)
E
rm -rf nonmatchings/$f
python3 tools/decomp-permuter/import.py --keep src/menu/menusub.c asm/nonmatchings/menu/menusub/$f.s >/dev/null 2>&1
cp /tmp/ms_bak_$f.c src/menu/menusub.c
cd nonmatchings/$f
sed -i -E 's/(lwc1|lw) +(\$[f0-9]+), \([A-Za-z_0-9]+\) \/\* gp_rel.*/\1 \2, 0($28)/' target.s
mips-linux-gnu-as -march=r5900 -mabi=eabi -I../../include target.s -o target.o && echo imported
