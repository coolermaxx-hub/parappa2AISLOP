# Audit tools

Helpers used during the readability pass. Run them from the repository root after `rebuild.sh`.

| Script | Purpose |
| --- | --- |
| `rebuild.sh` | Clean rebuild, then print link/compile errors and the main ELF path. The main ROM checksum failing is expected. |
| `alldiff.py` | Compare every built object against `expected2/` (the upstream matching tree, built with the same toolchain). Writes `alldiff_out.json`; compare with `alldiff_base.json` to spot regressions. |
| `cmpasm.sh <file>...` | Compile HEAD and the working copy of a source file to assembly and count differing lines. Use after pure renames and macro substitutions. |
| `symcmp.py new.elf expected.elf <symbols...>` | Compare symbol bytes between two ELFs. |
| `layout2.py <lo> <hi>` | Show symbol address deltas against `symbol_addrs`; all deltas in a section must be one constant. |
| `qwdump.py`, `tex0dec.py` | Decode GS quadwords and TEX0 values into `SCE_GS_SET_*` arguments. |
| `cmpsec.py`, `cmpdata.py`, `cmpbss.py`, `dataconv.py` | Section-level comparisons used while converting data to C. |

`expected2/` is untracked: build the upstream tree at the matching commit there with the same pinned toolchain.
