#!/bin/bash
# fdc2.sh unit(prlib/billboard) my_symbol expected_symbol -- compare a function whose names differ (e.g. weak copies named func_XXXXXXXX in the expected object)
cd "$(dirname "$0")/../.."
if [ -f src/$1.cpp ]; then o=src/$1.cpp.o; else o=src/$1.c.o; fi
ninja build/$o >/tmp/fd.log 2>&1 || { echo "COMPILE ERROR"; grep -v warning /tmp/fd.log | grep -E "rror|undeclared|: " | head -5; exit 1; }
MINE=$2; EXP=$3
ex(){ mips-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases $1 | awk -v f="<$2>:" '$2==f{p=1;next} /^[0-9a-f]+ </{p=0} p' | sed -E 's/^\s*[0-9a-f]+:\s*//; s/[0-9a-f]+ <([^>+]+)\+(0x[0-9a-f]+)>/\1+\2/' | sed "s/$MINE/FUNC/g; s/$EXP/FUNC/g"; }
ex expected2/build/$o $3 > /tmp/exp.fd; ex build/$o $2 > /tmp/mine.fd
if diff -q /tmp/exp.fd /tmp/mine.fd >/dev/null; then echo "MATCH $2"; else diff -y -W 110 /tmp/exp.fd /tmp/mine.fd; fi
