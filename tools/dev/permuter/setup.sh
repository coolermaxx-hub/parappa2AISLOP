#!/bin/bash
# setup.sh src/path.c FuncName   (FuncName = the INCLUDE_ASM symbol; C files only)
# Builds a decomp-permuter directory for one function from its NON_MATCHING draft and
# prints the starting score (0 = exact). Needs decomp-permuter checked out at $PERMUTER
# (default tools/decomp-permuter) and the original asm in asm/nonmatchings/.
# Output: $PERM_OUT/FuncName (default /tmp/perm). Run:  python3 $PERMUTER/permuter.py <dir> -j4 --stop-on-zero
cd "$(git rev-parse --show-toplevel)"
ROOT=$PWD; src=$1; fn=$2; unit=${src#src/}; unit=${unit%.c}
PERMUTER=${PERMUTER:-$PWD/tools/decomp-permuter}; D=${PERM_OUT:-/tmp/perm}/$fn; rm -rf $D; mkdir -p $D
CC=tools/toolchain/ee-gcc29/bin/ee-gcc
INC="-Iinclude -Isrc -Iinclude/rtl/common -Iinclude/rtl/ee -Iinclude/rtl/ee_gcc -Iinclude/rtl/ee_gcc/gcc-lib"
$CC -E -P -DNON_MATCHING $INC -x c $src > $D/base.c 2>/dev/null
python3 $PERMUTER/strip_other_fns.py $D/base.c $fn >/dev/null 2>&1
cat > $D/compile.sh <<EOF
#!/bin/bash
cd $ROOT
in=\$1; out=\$3; s=\$(mktemp /tmp/permXXXXXX.s)
CC=$CC
\$CC -S $INC -O2 -G8 -g -Wa,-Iinclude -x c "\$in" -o \$s 2>/dev/null || { rm -f \$s; exit 1; }
python3 tools/buildtools/lit4fix.py "\$in" \$s >/dev/null 2>&1
\$CC -c -O2 -G8 -g -Wa,-Iinclude -x assembler \$s -o "\$out" 2>/dev/null; r=\$?
rm -f \$s; exit \$r
EOF
chmod +x $D/compile.sh
(echo '.include "include/labels.inc"'; echo '.section .text'; echo '.set noat'; echo '.set noreorder'; cat asm/nonmatchings/$unit/$fn.s) > $D/target.s
$CC -c -O2 -G8 -g -Wa,-Iinclude -x assembler $D/target.s -o $D/target.o 2>/dev/null
printf 'func_name = "%s"\ncompiler_type = "gcc"\n' $fn > $D/settings.toml
cd $PERMUTER && python3 permuter.py $D --debug 2>&1 | grep "base score"
