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

## State
- Progress: 1339/1429 functions (93.7%), 80.3% of code, per the README at commit d9d4a1d. Seven more template copies moved to C after that (11227df), and progress has not been regenerated since.
- What is left, and what blocks each item, is in **`docs/remaining-work.md`**:
  - 5 intentional VU routines (done by rule).
  - 49 NON_MATCHING C bodies, which compile but don't match yet.
  - 29 weak template copies that are still asm.
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

## Helper scripts (`tools/dev/`)
| Script | Use |
| --- | --- |
| `fdc.sh unit func` | Build one object and side-by-side diff `func` against `expected2/` (e.g. `fdc.sh prlib/render Render__13PrSceneObject`). |
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
  - gcc emits them at the end of a TU, in the order the templates were first *used*. An unused `static inline` that calls a template still emits the copy.
  - The linker binds every call to the first copy in link order. So a C-emitted copy must not come before the currently named first copy.
  - See `scene.cpp`, `mendererdata.cpp`, `mendererawful.cpp` and `billboard.cpp` for the pattern.
- **One-pass `do { } while (0)` loops** change gcc 2.95's reorg delay-slot prediction, its CSE path following and its label alignment. They are what matched `RotateMatrix(int)`; see the comment in `src/nalib/namatrix.h`.
- **Conditional moves (`movz`/`movn`)** come from jump.c turning `x = a; if (cond) x = b;` into a cmov. The comparison operands are copied first, which explains a stray `daddu vN, sX, zero` before an `sltu`.
- **Variable reuse:** an extra `mov.s` or copy in the target often means the original reused one variable for two things (this matched `GetSynchronizeRatio`).
- **GCC 2.95.3 sources** (`reorg.c`, `cse.c`, `loop.c`, `final.c`, `jump.c`) can be fetched from `raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-2.95.3/gcc/`. Reading the relevant pass beats guessing.

## Suggested next steps
1. **Regenerate progress** and push, since the last 7 template copies aren't counted yet.
2. **`ScaleMatrix(const float&, const float&, const float&)`** is 44 lines off; the loop counter is in v0/v1 swapped and the temps are scheduled differently.
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
