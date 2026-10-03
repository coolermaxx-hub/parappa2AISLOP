# Branches

`codex-work` is the primary line (the owner, Snake, decided this on 2026-10-03).
Work and push here. The older ROM-matching line is preserved, untouched, as
`main-rom-matching` (and the original `main`); it keeps hack-dependent functions
as guarded `INCLUDE_ASM`. Do not force-push over either. Never touch upstream
parappadev/parappa2 (no pushes, PRs, issues or comments); work only in this fork.
Readability on `codex-work` is the objective; the main ROM checksum is expected
to mismatch and is reported candidly, never by restoring hacks.

# Decompilation constraints

Reconstruct idiomatic, maintainable C++ for a future open-source PC port.
Structural accuracy takes priority over a perfect binary match. Binary comparison
validates the reconstruction; it is not the sole objective.

- Establish the types, ownership and struct/class layout before implementing a
  function. If a layout is unknown, investigate it first. Record uncertainty;
  do not hide it behind `void*`, byte buffers, raw offsets or type punning.
- Put logic in its natural class or namespace. Do not create address-named,
  temporary or unused helpers to manipulate emitted code or template order.
- Reconstruct shared template implementations. Do not copy template bodies into
  translation units to force weak symbols, register choices or instruction order.
- Do not add pinned registers, empty assembly barriers, one-pass loops, nested
  gotos or other compiler steering solely to improve a match.
- Refactor an unconventional matching draft immediately. Keep the readable source
  and document any remaining binary differences rather than restoring a hack.
- Preserve evidenced game behavior, float operation order and original quirks.
  Intentional PS2/VU assembly and explicit SDK interfaces remain valid where the
  hardware requires them; they must operate on understood typed objects.
- Distinguish source reconstruction, exact function matches and full-ROM matches.
  Never count an assembly fallback as reconstructed C++ or change expected hashes
  to conceal a mismatch. Do not claim functional equivalence from compilation.
- Validate changed layouts and behavior with relevant tests, compile with the
  historical toolchain, inspect objdiff, and report checksum failures candidly.
- Keep uploaded original game files and generated output out of Git.

See `docs/porting-rules.md` for the workflow and `docs/remaining-work.md` for
remaining work. The user's current instructions supersede older matching-first
advice elsewhere in the repository.
