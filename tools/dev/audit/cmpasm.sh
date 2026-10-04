#!/bin/bash
# usage: cmpasm.sh file...   compares compiled asm of HEAD vs working tree
cd "$(git rev-parse --show-toplevel)"
for f in "$@"; do
  case "$f" in *.cpp) lang=c++;; *) lang=c;; esac
  ext="${f##*.}"
  cc(){ tools/toolchain/ee-gcc29/bin/ee-gcc -S -Iinclude -Isrc -Iinclude/rtl/common -Iinclude/rtl/ee -Iinclude/rtl/ee_gcc -Iinclude/rtl/ee_gcc/gcc-lib -O2 -G8 -x $lang "$1" -o "$2" 2>&1 | grep -i " error"; }
  git show HEAD:$f > $(dirname $f)/_cmp_old.$ext
  cc $f /tmp/_cmp_new.s; cc $(dirname $f)/_cmp_old.$ext /tmp/_cmp_old.s
  rm $(dirname $f)/_cmp_old.$ext
  n=$(diff /tmp/_cmp_old.s /tmp/_cmp_new.s | grep -v '\.file' | grep -c '^[<>]')
  echo "$f asm-diff-lines: $n"
done
