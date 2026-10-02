# Behaviour notes for a future port

These notes come from reading the decompiled source. Each says whether it was
checked or only inferred. Confirm anything marked *inferred* on hardware or in
an emulator before relying on it.

## Rhythm clock (first pass, 2026-10-02)

**The song clock has two sources, chosen per score line.** Each line has a time
type: `GTIME_CD0..2` for streamed audio or `GTIME_VSYNC` for frame-counted
lines (`include/main/scrctrl.h`). When the active line changes,
`GlobalTimeJobChange()` switches `global_data.TimeType` between `FGF_CD` and
`FGF_VSYNC` (`src/main/scrctrl.c`, around lines 3002 and 3638).

`GlobalTimeJob()` (`src/main/etc.c`) runs once per game-loop frame and
recomputes the clock:

- **FGF_CD (streamed song):** asks the IOP for the WP2 stream position
  (`WP2Ctrl(WP2_GETTIME)` in `CdctrlWp2GetSampleTmpBuf`, `src/main/cdctrl.c`).
  It converts that position to song ticks (`sample * tempo * 16 / 1875`),
  then to frames. **The audio stream position is the authority here.** The
  code also times the IOP query with `T0_COUNT` and prints the worst case,
  which suggests the developers worried about its latency.
- **FGF_VSYNC:** reads `vsync_time[0]`. A `sceGsSyncVCallback` handler
  (`TimeCallback` in `src/main/etc.c`) increments all 51 `vsync_time`
  channels on every VBlank. The game converts frames to ticks as
  `frames * 96 * tempo / 3600`, so the units are 60 Hz frames and 96 ticks
  per beat (checked from the formulas).
- For seeks, the game converts a tick time back to a stream position with
  `CdctrlSndTime2WP2sample` and subtracts a per-line offset
  (`GetTimeOfset(line) * 48 / 256`).
- *Inferred:* the 16/1875 factor equals 256/30000, so the WP2 position is
  probably counted in 256-sample units of a 48 kHz stream. This is not checked.

**Input sampling (inferred).** `osFunc()` (`src/os/system.c`) runs once per
frame from the system thread. It waits for VBlank (`sceGsSyncV`), resets
`T0_COUNT`, then reads the pads (`GPadSysRead`, `GPadRead`). The score loop
(`src/main/scrctrl.c`, around line 4211) later calls `GlobalTimeJob()` and then
`ScrCtrlIndvJob()`, which handles taps. So input appears to be sampled once per
frame just after VBlank, while the song time is sampled later in the same frame
from the audio stream. Judgement granularity is therefore one frame at best.
The next step is to trace how a tap gets its timestamp (`onKeyTime` near
`scrctrl.c:1879`).

**RNG (checked).** `osFunc()` calls `rand()` once every frame and discards the
result, so the RNG state depends on how many frames have elapsed. Netplay or
replays must reproduce the frame count, not just the inputs.
`src/os/syssub.c:574` builds a ranged random from `rand() & 0x7fff`.
prlib has its own generator (`src/prlib/random.cpp`, `PrFloatRandom`).

## Boundary inventory (keyword search, not yet analysed)

| Boundary | Files |
| --- | --- |
| Pad reads | `src/os/syssub.c` (`scePadRead`), `src/os/system.c` |
| VBlank / frame | `src/os/system.c`, `src/os/mtc.c`, `src/main/etc.c` |
| Hardware timers (`T0..T3_COUNT`) | `src/os/system.c`, `src/main/cdctrl.c`, `src/prlib/render.cpp`, `src/prlib/renderstuff.cpp`, `src/prlib/menderer.cpp` |
| Audio stream (WP2) and SE (TapCt) | `src/main/cdctrl.c`, `src/iop_mdl/wp2cd_rpc.c`, `src/iop_mdl/tapctrl_rpc.c`, `src/iop_mdl/wp2cd/iop/*`, `src/main/scrctrl.c` |
| Score, judgement, rank | `src/main/scrctrl.c`, `src/main/etc.c`, `src/main/mbar.c`, `src/main/main.c` |
| CD / files | `src/main/cdctrl.c`, `src/os/system.c` |
| Memory card | `src/menu/memc.c`, `src/menu/p3mc.c`, `src/main/mcctrl.c` |
| RNG | `src/os/system.c`, `src/os/syssub.c`, `src/prlib/random.cpp` |
| GS / DMA / VU | `src/os/system.c`, `src/prlib/*` |
