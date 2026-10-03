#!/bin/bash
# cmpf.sh myobj mysym expobj expsym
ex(){ mips-linux-gnu-objdump -d --no-show-raw-insn -M no-aliases $1 | awk -v f="<$2>:" '$2==f{p=1;next} /^[0-9a-f]+ </{p=0} p' | sed -E 's/^\s*[0-9a-f]+:\s*//;s/[0-9a-f]+ <[^>]*>//;s/\s+/ /g;s/ $//' | grep -v '^$'; }
ex $1 $2 > /tmp/c_mine.txt; ex $3 $4 > /tmp/c_exp.txt
if diff -q /tmp/c_exp.txt /tmp/c_mine.txt >/dev/null; then echo MATCH; else diff /tmp/c_exp.txt /tmp/c_mine.txt | grep -c '^[<>]'; diff -y -W 100 /tmp/c_exp.txt /tmp/c_mine.txt | head -${5:-80}; fi
