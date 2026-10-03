#!/bin/bash
# mkperm.sh NAME cfile asmfile(gp_rel-converted) -- make a permuter dir for a standalone C proxy
set -e
cd "$(dirname "$0")/../.."
D=nonmatchings/$1; rm -rf $D; mkdir -p $D
cp $2 $D/base.c
cat tools/dev/perm_hdr.s $3 > $D/target.s
mips-linux-gnu-as -march=r5900 -mabi=eabi -Iinclude $D/target.s -o $D/target.o
cat > $D/compile.sh <<EOS
#!/usr/bin/env bash
set -euo pipefail
INPUT="\$(realpath "\$1")"
OUTPUT="\$(realpath "\$3")"
cd "$(dirname "$0")/../.."
tools/toolchain/ee-gcc29/bin/ee-gcc -c -O2 -G8 -g "\$INPUT" -o "\$OUTPUT"
EOS
chmod +x $D/compile.sh
F=$(grep -o 'glabel [A-Za-z_0-9]*' $3 | head -1 | cut -d' ' -f2)
printf 'func_name = "%s"\ncompiler_type = "gcc"\n' $F > $D/settings.toml
