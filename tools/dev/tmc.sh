#!/bin/bash
# tmc.sh file.cpp -> /tmp/tm.o and disasm per function
cd "$(dirname "$0")/../.."
G=tools/toolchain/ee-gcc29/bin/ee-gcc
$G -S -Iinclude -Isrc -Iinclude/rtl/common -Iinclude/rtl/ee -Iinclude/rtl/ee_gcc -Iinclude/rtl/ee_gcc/gcc-lib -O2 -G8 -x c++ -fno-exceptions -fno-strict-aliasing $1 -o /tmp/tm.s 2>&1 | grep -v warning | head -5
$G -c -O2 -G8 -x assembler -Wa,-Iinclude /tmp/tm.s -o /tmp/tm.o
mips-linux-gnu-nm -n /tmp/tm.o | grep -i " [tw] "
