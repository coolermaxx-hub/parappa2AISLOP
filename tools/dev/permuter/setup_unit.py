"""setup_unit.py src/file.c ASMNAME CNAME : permuter job that compiles the whole unit (so inlining and
unit context match the real build) and compares the whole .text against the baseline object.
Usage: PERMUTER=... PERM_OUT=... python3 tools/dev/permuter/setup_unit.py src/menu/menusub.c TsOption_Flow TsOption_Flow
Then: python3 $PERMUTER/permuter.py $PERM_OUT/<name> -j4 --stop-on-zero
C units only (the permuter parses C). Review every result: the permuter happily reads
uninitialised variables or drops statements to lower the score."""
import sys,subprocess,os,re,shutil
HERE=os.path.dirname(os.path.abspath(__file__))
ROOT=os.path.abspath(os.path.join(HERE,'../../..'))
PERMUTER=os.environ.get('PERMUTER',os.path.join(ROOT,'tools/decomp-permuter'))
OUT=os.environ.get('PERM_OUT','/tmp/perm')
BASEOBJ=os.environ.get('BASE_OBJ',os.path.join(ROOT,'build'))
src,asmname,cname=sys.argv[1:4]
lang='c++' if src.endswith('.cpp') else 'c'
D=f'{OUT}/{asmname}'
shutil.rmtree(D,ignore_errors=True); os.makedirs(D)
orig=open(f'{ROOT}/{src}').read()
unit=src[4:].rsplit('.',1)[0]
key=f'INCLUDE_ASM("asm/nonmatchings/{unit}", {asmname});'
a=orig.index(key); a=orig.rindex('#ifndef NON_MATCHING',0,a)
mod=orig[:a]+'#if 0\n'+orig[a+len('#ifndef NON_MATCHING\n'):]
ext='.cpp' if lang=='c++' else '.c'
open(f'{D}/unit{ext}','w').write(mod)
CC=f'{ROOT}/tools/toolchain/ee-gcc29/bin/ee-gcc'
INC=f'-I{ROOT}/include -I{ROOT}/src -I{ROOT}/include/rtl/common -I{ROOT}/include/rtl/ee -I{ROOT}/include/rtl/ee_gcc -I{ROOT}/include/rtl/ee_gcc/gcc-lib'
# preprocessed full unit (keeps line structure), used for splicing
subprocess.run(f'{CC} -E -P {INC} -x {lang} {D}/unit{ext} > {D}/full.i',shell=True,check=True,cwd=f'{ROOT}/src/'+os.path.dirname(src[4:]))
# parseable base: C only -> strip other functions
if lang=='c':
    shutil.copy(f'{D}/full.i',f'{D}/base.c')
    subprocess.run(['python3',os.path.join(PERMUTER,'strip_other_fns.py'),f'{D}/base.c',cname],check=True)
flags='-O2 -G8 -g -Wa,-Iinclude' if lang=='c' else '-O2 -G8 -x c++ -fno-exceptions -fno-strict-aliasing -Wa,-Iinclude'
open(f'{D}/compile.sh','w').write(f'''#!/bin/bash
in=$1; out=$3; t=$(mktemp -d)
python3 {HERE}/splice.py {D}/full.i "$in" {cname} > $t/u.i || {{ rm -rf $t; exit 1; }}
cd {ROOT}
{CC} -S {INC} {flags} -x {lang} $t/u.i -o $t/u.s 2>/dev/null || {{ rm -rf $t; exit 1; }}
cp {D}/unit{ext} $t/src{ext}; python3 tools/buildtools/lit4fix.py {src} $t/u.s >/dev/null 2>&1
{CC} -c -O2 -G8 -Wa,-Iinclude -x assembler $t/u.s -o "$out" 2>/dev/null; r=$?
mips-linux-gnu-strip "$out" -N dummy-symbol-name 2>/dev/null
rm -rf $t; exit $r
''')
os.chmod(f'{D}/compile.sh',0o755)
# The target is the unit as built with the function still INCLUDE_ASM: build once before
# editing, or point BASE_OBJ at a saved copy of build/.
shutil.copy(f'{BASEOBJ}/{src}.o',f'{D}/target.o')
open(f'{D}/settings.toml','w').write(f'func_name = "{cname}"\ncompiler_type = "gcc"\n')
