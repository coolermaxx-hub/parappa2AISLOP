# Audit tools

Helpers used during the readability pass. Run them from the repository root after `rebuild.sh`.

| Script | Purpose |
| --- | --- |
| `rebuild.sh` | Clean rebuild, then print link/compile errors and the main ELF path. The main ROM checksum failing is expected. |
| `alldiff.py` | Compare every built object (calls are compared by target name; local `.text` relocations are ignored) against `expected2/` (the upstream matching tree, built with the same toolchain). Writes `alldiff_out.json`; compare with `alldiff_base.json` to spot regressions. |
| `gen_binary_diff_doc.py` | Rewrite `docs/binary-differences.md` from the last `alldiff.py` run. |
| `cmpasm.sh <file>...` | Compile HEAD and the working copy of a source file to assembly and count differing lines, also ignoring local label numbers and the numbers gcc appends to function-scope statics (both shift without changing the image). Use after pure renames and macro substitutions. |
| `symcmp.py new.elf expected.elf <symbols...>` | Compare symbol bytes between two ELFs. |
| `layout2.py <lo> <hi>` | Show symbol address deltas against `symbol_addrs`; all deltas in a section must be one constant. |
| `layout_orig.py [sections]` | Compare every data symbol's offset within `.data`, `.sdata`, `.sbss` and `.bss` with the original executable. The only way to check `.bss`, which is not in the ROM image. |
| `qwdump.py`, `tex0dec.py` | Decode GS quadwords and TEX0 values into `SCE_GS_SET_*` arguments. |
| `cmpsec.py`, `cmpdata.py`, `cmpbss.py`, `dataconv.py` | Section-level comparisons used while converting data to C. |
| `stabs.py <name>` | Dump the original executable's debug symbols for source files whose name contains `<name>`: original local names, register or stack slots and block scopes for the C units. |
| `stabs_scopes.py [name] [-o] [-s] [-v]` | Compare every function's locals and block nesting with the original debug info, using the debug info our build emits (so variables the optimizer removed are missing on both sides and do not count). `-o` also checks declaration order, `-s` blocks whose variables were all optimized away, `-v` register and stack slots. Prefer this to `stabs_locals.py`. |
| `stabs_locals.py <files>` | Older heuristic: list locals declared in the given C files that the original debug info does not have. It also flags variables the optimizer removed in both builds. |
| `stabs_types.py [name]` | Compare every struct, union and enum the original C units define with ours (from the `.s` files the build leaves): prints members whose name or offset differ, and size differences. |
| `stabs_linkage.py` | List functions and variables that are static in the original but global here (or the reverse), and functions whose parameter names differ. |

`expected2/` is untracked: build the upstream tree at the matching commit there with the same pinned toolchain.
