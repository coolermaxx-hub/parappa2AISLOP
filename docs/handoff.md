# Handoff: how to pick up the PaRappa 2 decomp

This is for whoever (person or bot) continues this work. Last updated 2026-10-03.

## Ground rules from Snake (the owner)
- **Never touch upstream** `parappadev/parappa2`: no PRs, issues, comments or Discord. Work only in this fork (`coolermaxx-hub/parappa2AISLOP`).
- Push ROM-verified work straight to `main`. Every push must build `build/SCPS_150.17.rom: OK` first.
- Matching VU1 microcode (`.vsm`) and intentional inline COP2/VU asm count as done. Only compiler-generated EE C/C++ needs C.
- Keep matching code separate from the future Windows/netplay port. Don't "improve" float behaviour.
- Document timing, input, audio, scoring, VBlank and DMA behaviour in `docs/behavior-notes.md`.
- See `docs/porting-rules.md` for the full definition of 100%.
- Snake wants continuous work: after a milestone, post a short result and move on to the next item without waiting.
- **Readable C++ first (2026-10-03).** Reconstruct what the original developers wrote; the byte match is the check, not the goal. If the choice is a hack that matches or a clean version that misses, keep the clean one as `NON_MATCHING` and say in a comment why it differs. Snake accepts the lower percentage.
  - Banned: raw offsets or byte arrays standing in for structs (`*(int *)(p + 0x34)`), orphan helpers named `func_XXXXXXXX` or `*_tmp`, asm-label aliases, unused "anchor" inlines, per-TU explicit specializations of templates, `void *` or type punning.
  - Define struct layouts first, then write control flow against them.
  - A match that needs gotos, `do { } while (0)` or casts is a first draft: refactor it while keeping the match, or keep it clean and `NON_MATCHING`.
  - Check every diff for these before committing.

## State
- Progress: 1328/1429 functions (92.9%), 79.2% of code (README). It was 1346 before the readability pass moved hack-dependent matches back to asm.
- What is left, and what blocks each item, is in **`docs/remaining-work.md`**:
  - 5 intentional VU routines (done by rule).
  - 46 NON_MATCHING C bodies, which compile but don't match yet.
  - 50 weak template copies that are still asm; each template has a real generic definition in its header.
- Every function in a C/C++ file already has a C body. What remains is matching.

## Build
Needs Snake's July 12 NTSC-J prototype files in `iso/` (`SCPS_150.17`, `WAVE2PS2.IRX`, `TAPCTRL.IRX`, `MDL/*.OLM`). The originals are in the project's shared files (see `/mnt/project-files/.notes/inputs.md`).

```sh
sudo dpkg --add-architecture i386
sudo apt-get install binutils-mips-linux-gnu gcc-mipsel-linux-gnu ninja-build libc6:i386 libstdc++6:i386
python3 -m venv .venv && . .venv/bin/activate && pip install -U pip setuptools wheel && pip install -r requirements.txt
python tools/setup.py          # toolchains, dump/sym, dump/disasm
python configure.py && ninja   # must end with: build/SCPS_150.17.rom: OK
git checkout config/p3.jul12.undefined_funcs_auto.txt config/p3.jul12.undefined_syms_auto.txt
```

- Copy `build/` to `expected2/build/` once, while the build matches. `tools/dev/fdc.sh` diffs a function's object code against that copy.
- Re-run `configure.py` after any `symbol_addrs` edit or any `INCLUDE_ASM` removal, because it regenerates `asm/`.
- **ninja does not track header dependencies.** After editing a header, `rm -rf build` before checking the ROM, or a stale object can report OK. A header change in 11227df broke navector.cpp this way and was only caught by a clean build.

## Helper scripts (`tools/dev/`)
| Script | Use |
| --- | --- |
| `fdc.sh unit func` | Build one object and side-by-side diff `func` against `expected2/` (e.g. `fdc.sh prlib/render Render__13PrSceneObject`). |
| `nmtest2.sh src/file unit func` | Like `nmtest.sh`, but defines `NON_MATCHING` for the whole file (needed where a file's tail of template copies is guarded). `FUNC=NONE` just compiles. |
| `fdc2.sh unit mine expected` | `fdc.sh` for when the names differ, e.g. a C copy against an asm `func_` copy. |
| `nmtest.sh src/file unit func` | Temporarily switch `func` to its NON_MATCHING C body, diff it, restore the file. Prints the diff line count; output goes to `/tmp/nm_out.txt`. |
| `allnm2.sh` | Run `nmtest.sh` over every prlib NON_MATCHING function (diff counts; a 0 or a compile error is bogus, check `/tmp/nm_out.txt`). |
| `tmc.sh file.cpp` | Compile a scratch TU with the game flags to `/tmp/tm.o` and list its symbols. |
| `cmpf.sh myobj mysym expobj expsym` | Compare one function in two objects. |
| `perm_import.sh func` / `mkperm.sh` | Make decomp-permuter dirs (C only; the permuter can't parse C++). |
| `update_progress.py` | After `objdiff-cli report generate -o /tmp/report_fork.json`, rewrite `progress/*.json` and the README table. |
| `gen_remaining.py` | Regenerate the tables in `docs/remaining-work.md`. |

### Progress regeneration
```sh
python configure.py --objdiff && ninja && python3 tools/objdiff_symbol_mappings.py
tools/objdiff/objdiff-cli report generate -o /tmp/report_fork.json
python3 tools/dev/update_progress.py
python configure.py && ninja   # back to the normal build
```

## Knowledge that saves time
- **The original ELF has symbols.** `mips-linux-gnu-nm -n iso/SCPS_150.17` names every global function, including the first copy of each weak template instance. 578 SDK/libc names were imported from it in bf8228b. Static data and locals there are still worth mining.
- **`dump/sym/p3_functions.cpp`** lists each C function's locals with their registers. Writing C with exactly those locals fixes most register-allocation diffs.
- **Weak template copies**:
  - gcc emits implicit template instances at the end of a TU, in the order the templates were first *used*; `INCLUDE_ASM` lands in place. So C copies can't follow an asm copy in a TU's tail.
  - The linker binds every call to the first copy in link order.
  - **`extern template` works in ee-gcc 2.95.** `extern template NaMATRIX<float, 4, 4> NaMATRIX<float, 4, 4>::RotateMatrix(int, const float&);` stops the instantiation, so calls go to the mangled name the asm copy defines. Use it under `#ifndef NON_MATCHING` and guard the asm tail the same way (see `billboard.cpp`, `spadata.cpp`).
  - A file-local static that asm still references (e.g. `prSpramData` in render.cpp) is given its address in `config/p3.jul12.undefined_syms.txt` under the asm's name.
- **`objdump` prints `...` for runs of zero words**, which hides alignment differences. Compare raw `.text` bytes when only the size differs (that is how the 8-byte padding in `RotateMatrix(int)` was found).
- **Unions change scheduling.** Accessing a field through a union made gcc treat the store as aliasing and reorder `_P3MC_AddUserBroken`; a plain nested struct didn't. An inline accessor also compiled differently from the same expression written out, so `P3MC_DATE_WORD` is a macro.
- **Conditional moves (`movz`/`movn`)** come from jump.c turning `x = a; if (cond) x = b;` into a cmov. The comparison operands are copied first, which explains a stray `daddu vN, sX, zero` before an `sltu`.
- **Variable reuse:** an extra `mov.s` or copy in the target often means the original reused one variable for two things (this matched `GetSynchronizeRatio`).
- **GCC 2.95.3 sources** (`reorg.c`, `cse.c`, `loop.c`, `final.c`, `jump.c`) can be fetched from `raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-2.95.3/gcc/`. Reading the relevant pass beats guessing.

## Suggested next steps
0. Keep auditing for the banned patterns: `prSpramData_tmp_menderer` / `_mendereralpha` externs (asm-data aliases), the `template <> inline` constructor in `navector.cpp`, and the many `unkXX` struct fields that can be named from how they are used.
1. Match `ComposeGlobalMatrixWithoutVisibility` (6 lines: `m_flags` is reloaded before `prSpramData` after `BlendTransitionMatrix`). Then render.cpp's tail of weak copies can come from C; see remaining-work.md.
2. **`ScaleMatrix(const float&, const float&, const float&)`** is 38 lines off at best (see remaining-work.md for the by-value finding).
   - `TranslateMatrix` with the same signature already matches as an in-class `return NaMATRIX<float, 4, 4>(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1);`.
   - Fixing ScaleMatrix unlocks menderer's 5 copies and most of spram's.
   - Test harness: take `&NaMATRIX<float, 4, 4>::ScaleMatrix` in a scratch TU, then compare against `expected2/build/src/prlib/spram.cpp.o` `func_00148140`.
3. **Vector `RotateMatrix`** (spadata) is 74 lines off. It is built like `TranslateMatrix` (constructor form, temps before the loop), but the target evaluates the constant temps early.
   - It unlocks camera's 3 copies.
4. **Near misses by diff lines:**
   - `ComposeGlobalMatrixWithoutVisibility`: 6
   - `PrUpdateMendererSpeed`: 8; the block order of the two `jr ra` tails differs.
   - `PrUpdateAwfulMenderer`: 11; two FPRs with surviving `mov.s` copies, and the permuter failed.
   - `DrawObjStrDisp`: 15; the target uses `movz` for the `PRtime` subtraction. The `{ u_int t = time - PR; if (time <= PR) t = 0; time = t; }` shape gets a cmov, but as `movn` and with s6/s7 swapped (32 lines).
   - `TsPopMenCus_Draw`: 22
   - `TsOption_Flow`: 32
5. Keep `docs/remaining-work.md` current (`tools/dev/gen_remaining.py`) and add behaviour notes as functions are understood.
