#!/bin/bash
# nmtest.sh src/file unit func : test NON_MATCHING body of func
cd "$(dirname "$0")/../.."
cp $1 /tmp/nmtest_bak
python3 - "$1" "$3" <<'E'
import sys,re
p,f=sys.argv[1],sys.argv[2]; s=open(p).read()
a='#ifndef NON_MATCHING\nINCLUDE_ASM("'
i=s.index(f+');\n#else'); st=s.rindex(a,0,i)
k=s.index('\n',i+len(f+');\n#else'))+1; s2=s[:st]+s[k:]
e=s2.index('#endif',st); s2=s2[:e]+s2[e+len('#endif'):]
open(p,'w').write(s2)
E
source .venv/bin/activate
tools/dev/fdc.sh $2 $3 > /tmp/nm_out.txt 2>&1
grep -c '|\|<\|>' /tmp/nm_out.txt
cp /tmp/nmtest_bak $1
