# Fork rules: readable reconstruction, binary validation

Reconstruct the original developer's idiomatic C++ as a maintainable foundation
for mods and a future PC port. Structural accuracy takes priority over matching
every instruction on the first attempt. Keep a clean, logical type definition
when it produces a small binary difference and explain that difference.

## What "100%" means

1. Every function originally generated from C/C++ has an understood, readable
   source implementation in its natural class or namespace.
2. Data layouts, ownership and important sections are understood and named.
3. Intentional PS2 assembly remains explained source, including VU1 microcode
   and necessary COP2/VU0 operations. Assembly fallbacks for compiler-generated
   functions are still unfinished reconstruction.
4. The build is reproducible; behavior has relevant validation; all remaining
   binary discrepancies are explicitly accounted for.

Report exact matching separately. objdiff percentages measure binary agreement,
not readability, behavioral equivalence or completion of the port. A full-ROM
match means the unchanged expected checksums pass; never alter those checksums
to make a reconstructed build appear matched.

## Reconstruction workflow

- Analyze variables and establish the struct/class layout before control flow.
  Investigate unknown layouts rather than replacing them with raw memory offsets,
  arbitrary byte arrays, `void*` or primitive type punning.
- Use proper inheritance and shared templates where the evidence supports them.
  Do not duplicate template bodies into translation units or create temporary,
  address-named or unused helpers to manipulate code generation.
- Use ordinary control flow. Pinned registers, empty assembly barriers, one-pass
  loops and other compiler tricks solely for matching are forbidden.
- Treat unconventional matching code as a draft requiring immediate refactoring.
  Document resulting scheduling, register or relocation differences honestly.
- Preserve original float operation order and game quirks. EE/VU float behavior
  differs from x86; compilation or generic MIPS tests do not prove PS2 equivalence.
- Keep intentional hardware assembly scoped to typed operations. SDK integration
  must have an understood layout; do not disguise unknown memory as SDK data.
- Record provenance: original disassembly, surviving source, symbols or SDK
  knowledge. Mark inferred names and unresolved assumptions explicitly.
- Validate layout and behavior where possible, compile affected source with the
  historical compiler, compare objects and run full checksum checks. Distinguish
  readable reconstruction from exact function matches and exact ROM matches.

Functions on timing, input, audio, scoring, RNG, file access or DMA/GS boundaries
also need notes in [behavior-notes.md](behavior-notes.md). A future portable
implementation will need original-game input/state comparisons and explicit
hardware adapters. Never commit game files; assets come from the user's disc.
