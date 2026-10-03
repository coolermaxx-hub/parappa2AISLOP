#!/bin/bash
# allnm2.sh [pattern]: diff counts (non-reloc lines) for every NON_MATCHING func in prlib/nalib
cd "$(dirname "$0")/../.."
source .venv/bin/activate
for f in src/prlib/*.cpp; do
  u=prlib/$(basename $f .cpp)
  grep -A1 "^#ifndef NON_MATCHING" $f | grep INCLUDE_ASM | sed -E 's/.*", ([^)]+)\).*/\1/' | while read fn; do
    case "$fn" in func_*|*NaGifPacket*|Set__*|__t8*) continue;; esac
    tools/dev/nmtest.sh $f $u $fn >/dev/null
    r=$(grep -v R_MIPS /tmp/nm_out.txt | grep -c '|\|<\|>')
    echo "$fn $r"
  done
done
