# PC port roadmap

Target: a native PC build of *PaRappa the Rapper 2*, ultimately with netplay.

This document is a plan, not a report. Where it states facts they come from the
reconstructed source or from [behavior-notes.md](behavior-notes.md), and the
provenance is given. Where it is inference or design judgement it says so.
Nothing above Phase 0 has been built or run; see
[Verification status](#verification-status).

Related: [remaining-work.md](remaining-work.md) covers finishing the
decompilation, [porting-rules.md](porting-rules.md) covers how to write the
source, [unemitted-copies.md](unemitted-copies.md) covers the dead functions we
now know we can drop rather than port.

## What this branch already gives us

The reason this fork exists in readable form rather than ROM-matching form is
that a port cannot use assembly fallbacks. `main-rom-matching` keeps
`INCLUDE_ASM` bodies the original compiler generated; those would have to be
hand-translated with no types, no names and no structure. This branch has no
assembly fallbacks, so the game's logic is compilable C++.

Concretely, for porting purposes:

- Game logic in `src/main`, `src/menu`, `src/os`, `src/dbug` is plain C and is
  the part that carries over most directly.
- The engine in `src/prlib` and the math in `src/nalib` are typed C++ templates
  with documented layouts.
- Data formats are understood and named: SPM geometry and deformation lists
  ([spm-geometry-layout.md](spm-geometry-layout.md)), SPA animation node tables
  ([spa-animation-layout.md](spa-animation-layout.md)), the packet layout
  ([spm-packet-layout.md](spm-packet-layout.md)), and the noodle texture model
  ([noodle-texture-model.md](noodle-texture-model.md)).
- The hardware touchpoints have been catalogued rather than left implicit. See
  "Boundary inventory" in [behavior-notes.md](behavior-notes.md).

Two things follow that are easy to get wrong when estimating this project:

- **Byte-match percentage is worth nothing to a port.** The current 92.65%
  figure measures decompilation fidelity, not portability. Work on the remaining
  105 mismatching functions has been deprioritised for exactly this reason: the
  ones still reachable need either compiler steering that
  [AGENTS.md](../AGENTS.md) forbids or a source shape not yet derived, and none
  of it is a portability blocker.
- **Much of what objdiff still reports as absent is dead code we do not need.**
  Of the 26 functions the original contains and our build does not, 24 are
  unreferenced in the original and are deleted rather than ported; see
  [unemitted-copies.md](unemitted-copies.md).

## What the decompilation does not give us

Estimated at 20-30% of a finished port. The remainder:

| Area | Why it is new work |
| --- | --- |
| Renderer | GS, VU0, VU1, DMA, GIF and VIF are replaced by a modern graphics API, not translated |
| Audio | `wave2ps2.irx` and `tapctrl.irx` are separate R3000 binaries; BGM streaming and voice/SFX need an audio engine written from scratch |
| Pointer width | ~150 sites in C and ~110 in prlib C++ assume 32-bit pointers |
| Timing | VBlank-driven 60 Hz NTSC-J with HBlank timers |

The pointer inventory is already grouped by kind in
[behavior-notes.md](behavior-notes.md#32-bit-pointer-assumptions-2026-10-04), and
`CalculatePointer` in prlib is called out as the single choke point for
SPM/SPA/SPC file records. That grouping is the work list for phase 2.

The in-game models are not the hard part. The models are geometry, animation
data and packets, all of which are documented; what is hard is that the *shading*
leans on VU1 microprograms and the GS's fixed-function pipeline, and reproducing
the look means reimplementing that in a modern shader rather than emulating the
hardware.

## Phases

Each phase has an exit test. Do not start a phase before the previous exit test
passes; the failures compound badly.

### Phase 0 - verified build (complete)

Get the historical toolchain running and confirm a baseline.

- The toolchain is Linux-only: `tools/toolchain/ee-gcc29/bin/ee-gcc` is a 32-bit
  Linux i386 binary, `tools/objdiff/objdiff-cli` is Linux x86-64, and
  `tools/ccc` (downloaded by `tools/setup.py`) is Linux-only.
- Install per [build.md](build.md): `gcc-mipsel-linux-gnu`,
  `binutils-mips-linux-gnu`, `ninja-build`, plus the Python requirements, on a
  host with i386 support.
- Establish the baseline measurement and confirm the pending `namatrix.h` change
  in [unemitted-copies.md](unemitted-copies.md) neither helps nor regresses.

Exit test: a clean build, an objdiff report, and a decision on the
`ScaleMatrix` change based on measured evidence rather than expectation.

**Passed 2026-10-07.** All three conditions are met:

- Clean build, both ROMs linked, on `codex-work` at `1b32dba`.
- objdiff report regenerated: 1324 / 1429 exact functions, 261683 / 342284
  exact code bytes, fuzzy 95.15883%.
- The `ScaleMatrix` change was decided on measurement, not expectation: writing
  the diagonal with a nested loop keeps the function out of line without any
  pragma or asm barrier, and it matches the original exactly at 208 bytes and
  52 instructions. It needed a non-const `NaMATRIX::operator[]`, which the
  class was missing.

Two environment findings worth carrying forward, both in
[build.md](../build.md)'s territory:

- The repository's committed blobs contain CRLF, so every `./tools/*.py`
  shebang fails on a clean checkout. Strip CR from text files only; doing it
  across all tracked files corrupts the binary tools.
- `ninja` records no header dependencies, so a `.h` edit does not trigger a
  rebuild. Every measurement above comes from a clean rebuild.

Phase 1 is now unblocked.

### Phase 1 - host-compile the engine

Make the reconstructed sources build for a 64-bit host with the hardware
interfaces stubbed. No graphics yet. The goal is to prove the *logic* is
genuinely portable and to surface every 32-bit assumption at once.

- `docs/remaining-work.md` already records that every EE source parses with a
  64-bit host gcc/g++ using `-D__IEEE_LITTLE_ENDIAN -D__R5900__` and
  `-fpermissive`. Extend that from "parses" to "links and runs".
- Introduce the hardware seam as a real interface rather than `#ifdef`-ing the
  PS2 SDK away. `GS_REG_WORD`, `GS_REG_VIEW` and `GIF_TAG_QWORD` in
  `include/common.h` are documented as the places a port replaces; they are the
  natural boundary.
- Float behaviour differs between the EE/VU and x86. Per
  [porting-rules.md](porting-rules.md), compilation does not prove equivalence.
  Expect this to be a recurring source of subtle mismatch and validate
  deliberately rather than assuming.

Exit test: the menu system runs on a host binary and can be driven with a
keyboard.

### Phase 2 - the three hardware seams

The bulk of the real work. These are independent and can be staffed separately.

**Graphics.** Replace GS packet emission with a modern API. The typed register
structures and `PKGIFTAG`/`PK_AD_PACKET` builders in the source describe intent
well enough to translate into draw calls, and prlib's GS state is documented in
`src/prlib/gsstate.h`. Expect to replace the two handwritten VU0 vertex kernels
in `src/prlib/renderee.cpp` with vertex shaders that perform the same projection
and clipping.

**Audio.** Write a BGM player for the WP2 stream container plus a voice/SFX path
replacing TapCtrl. The container layout is documented in the CD section of
[behavior-notes.md](behavior-notes.md); the playback semantics are yours to
define.

**Timing and input.** Replace `sceGsSyncV`/VBlank with a fixed-step loop, and
`scePadRead` with a real input backend. Timing is the piece most likely to
determine whether the port is playable, so treat it as first-class rather than
as plumbing.

Exit test: a stage renders and plays with correct-looking audio, at the right
speed.

### Phase 3 - single-player correctness

This is the phase that decides whether the port is worth continuing, and it is
the one most likely to be underestimated.

Grading does not use millisecond windows. Per
[behavior-notes.md](behavior-notes.md#judgement-and-line-scores), the song clock
produces ticks, ticks are quantised into 4-tick cells (24 per beat), and each
tap is looked up in one of three 24-bit tables (`thnum_tbl`,
`src/main/scrctrl.c:142`). Reproducing the original's judgement therefore means
reproducing its clock accurately, not approximating it.

Adaptive difficulty compounds this. Per the same document, the pattern level is
drawn from `TAPLVL_DAT` using percentages walked against `randMakeMax(100)`, and
the control level moves according to the player's own recent scores. So the chart
a player sees depends on their own performance history *and* on RNG. Get the
clock wrong and judgement is wrong regardless of everything else.

Exit test: the game is genuinely playable and fair to a human, and documented
test replays grade identically to the original.

### Phase 4 - netplay

See the hazards below. This phase is gated on phase 3.

## Netplay hazards

These are the things most likely to sink netplay, in rough order of severity.
The first two are findings from the existing analysis rather than speculation,
and both are more serious than "add rollback netcode".

### The chart itself is not guaranteed identical between peers

This is the one that changes the design. Adaptive difficulty selects the pattern
level with `randMakeMax(100)`, which reads libc `rand()`. And the original never
calls `srand`, while `osFunc()` calls `rand()` once at the top of every frame
and discards the result, so the stream position depends on how many frames have
elapsed since boot.

Two players whose RNG streams differ will be *shown different patterns*. They
would be graded against different charts, and neither player's score would mean
anything. So this is not a cosmetic determinism problem; the simulation is not
even the same problem on the two peers.

Resolution: replace the gameplay RNG with an explicitly seeded, portable stream
advanced identically on every peer, with the seed chosen by the host and agreed
at match start. Per [behavior-notes.md](behavior-notes.md#random-number-generators-2026-10-03),
of the three generators only libc `rand()` reaches gameplay - `PrRandom` and
`GetRandom` are cosmetic, and `GetRandom`'s seed is reset every noodle draw so
its sequence is identical each frame. That narrows the work to one stream.

Open question: whether to keep the exact libc algorithm for fidelity to original
behaviour, or to adopt a modern PRNG and accept that replays of original
recordings will not reproduce. The document argues for keeping libc if
bit-exact replays matter. This is a decision, not a fact.

### Rhythm games need latency calibration, not just determinism

Even with identical state, two players with different audio output latencies
will be judged differently. Deterministic simulation does not solve this; it
only removes one source of divergence and leaves the dominant one.

The original's clock has two sources and the choice is per score line
(`GTIME_CD` for streamed audio, `GTIME_VSYNC` for frame-counted lines). In the
`FGF_CD` case the audio stream position read back from the IOP is the authority,
and the developers were visibly worried about that query's latency - they timed
it with `T0_COUNT` and print the worst case. That is a warning: a clock derived
from a stream position is as good as the readback latency.

Practical options, none decided yet:

- **Input delay calibration per player**, simplest, costs responsiveness.
- **Audio-clock-driven rollback**, correct but considerably more complex and
  fights the fixed-step model.
- **Rewriting judgement onto a local audio clock**, which departs from the
  original's behaviour and must be validated against phase 3 first.

Anything that changes judgement must be validated against the original before
netplay is layered on top, or bugs become indistinguishable between the two.

### Determinism hazards in the renderer

The engine reads hardware state that a PC does not have. VU R-register
randomness, DMA ordering and GS pipeline transfer delays are called out in
[remaining-work.md](remaining-work.md) as reasons that generic MIPS scalar tests
cannot establish equivalence. Any port decision that depends on them is
unvalidated.

The three cosmetic generators are safe to leave local per peer. The gameplay
stream is not.

### Tick quantisation is an opportunity

Because judgement is quantised to 4-tick cells, and the notes conclude that a
port only needs to reproduce `{cell index per press, key per press}`, the network
payload can be quantised input rather than raw frame-by-frame pad state. That is
a meaningfully smaller and more robust wire format than input streams, and it is
worth designing the netcode around deliberately rather than discovering later.

The corollary is that a peer can only be held to judgements it could actually
have made at its own latency. That is the same problem as above, and it needs an
explicit policy rather than an accident of implementation.

## Verification status

Phase 0 is complete and verified; see that section for the measurement and the
two environment findings. Everything from Phase 1 onward is still a plan and
has not been built or run.

No claim of functional equivalence to the original has been made or should be
inferred from the decompilation matching percentage.

The original executable is retained locally for disassembly and comparison, and
its assets (SCPS_150.17, the IRX modules, the OLM stage overlays) are needed to
run and compare against. Those must not be committed.

## Scale

This is a multi-year project for a small team, and the target is unusually hard:
no commercial PC port of this game exists. Its identity leans on PS2 hardware -
VU-driven model morphing and the GS's fixed-function pipeline - so "portable" here
means reimplementing a rendering approach, not translating code.

Treat phase 3 passing as the real go/no-go decision. Phases 0 through 2 are
substantial and well-bounded; if phase 3 cannot make the game genuinely playable
and fair, netplay is not worth building.