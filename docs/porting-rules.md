# Fork rules: matching first, port later

This fork has one job: rebuild the PS2 binary byte for byte. A Windows port is the
long-term goal, but it will live in a separate portable core. Nothing in this
repository is changed toward portability.

```
PS2-MATCH         exact historical reconstruction (this repo)
    |
behavior notes + regression tests from the original game
    |
PORTABLE CORE     original game logic, hardware independent
    |
PLATFORM          Windows input / audio / graphics / files / network
```

## What "100%" means here

1. Every EE function that the compiler generated from C/C++ is reconstructed as
   source that matches byte for byte.
2. Hand-written PS2 code counts as finished source when it reproduces the
   original. That includes VU1 microcode (`.vsm`/`.dsm`) and intentional inline
   COP2/VU0 macro assembly. We do not rewrite it in C to raise a percentage.
3. Important data and sections are understood and named.
4. The build is reproducible, and no executable blob is left unexplained.

objdiff's function percentage is a guide, not the definition. It counts the
hand-written VU routines in `prlib/renderee.cpp` as "unmatched" even though their
assembly is the real source.

## Rules while matching

- Hacks are fine if they match: odd casts, inline COP2, compiler tricks, globals.
- Never "improve" float behaviour or operation order. EE/VU float rules (no
  denormals, rounding, NaN handling) differ from x86.
- Keep original quirks. Fixes belong to a later "enhanced mode", not here.
- A function is done when objdiff shows an exact match and `ninja` ends with
  `build/SCPS_150.17.rom: OK`. Plausible-looking C is not done.
- Every commit says what changed, why it is believed correct, the ROM result,
  and any struct or type changes.
- Provenance: say where a function's shape came from (direct reverse
  engineering, `src/prlib/old/`, SDK knowledge, or decomp-permuter).
- Functions on a behavioural boundary get a note in
  [behavior-notes.md](behavior-notes.md), not just a "matched" mark. Boundaries
  are pad reads, VBlank, timers, song position, scoring and judgement, SPU and
  audio streaming, RNG, frame advance, CD and file access, memory card, and
  DMA/GS.

## Later, outside this repo

- Pin the toolchain (compiler, assembler, linker scripts, Python tools) in a
  container.
- Add `static_assert` size and offset checks once struct layouts are certain.
- Record input sequences on the original game and compare state at fixed
  timestamps (song position, score, rank, judgement, RNG).
- Load assets from the user's own disc. Never commit game data.
