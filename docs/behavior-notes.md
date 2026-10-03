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

**Tap timestamp (checked in source, 2026-10-02).** `ScrCtrlIndvJob()` computes
`ctime_next` from the line's `lineTime` (the clock value for this frame) and
`indvTime = ctime_next - current_time`. It calls `tapEventCheck(sindv, ctime_next, indvTime, i)`
(`src/main/scrctrl.c`, around line 3979). For a human pad, `tapEventCheck` reads
`pad[].shot` / `pad[].one` (this frame's edge-detected buttons, produced by
`GPadRead` in `osFunc`) and records the press with `mccReqTapSet(Ttime, ...)`, so
**a press is stamped with the frame's clock value, not its real press time.**
Every press within one frame gets the same timestamp. Replays store these
frame-quantised times (`mccReqTapGet`), and the computer player is driven from
a precomputed list compared against the same clock. So a faithful port can
reproduce judgement from `{clock per frame, per-frame pad edges}`.
**How `lineTime` advances (checked in source, 2026-10-03).** `ScrTimeRenew()`
(`src/main/scrctrl.c:3984`) runs once per frame (called around line 4212) and
rewrites `lineTime` (ticks, 96 per beat) and `lineTimeFrame` (60 Hz frames) for
every score line:

- *VSYNC lines:* `lineTime = (frames * 96 * tempo + 1800) / 3600`, plus
  `tempo * 96 * ofsCdtime / 60000` (`ofsCdtime` is a per-line offset in
  milliseconds), clamped at 0. `lineTimeFrame` is the raw VBlank counter for that
  line (`TimeCallbackTimeGetChan(i)`).
- *Streamed lines:* `samplecnt = GlobalSndSampleGet() + ofsCdtime * 48 / 256`
  (during `PSTEP_XTR` it is `CdctrlWp2GetSampleTmp() - getTopSeekPos()`), clamped
  at 0, then `CdctrlWp2CdSample2SndTime` and `CdctrlWp2CdSample2Frame` turn it
  into ticks and frames for the line's tempo.
- `allTimeCallbackTimeSetChanTempo()` and the goto-line code at
  `scrctrl.c:3578` set `lineTime` directly and derive `lineTimeFrame` as
  `(ticks * 3600 + tempo * 96 / 2) / (tempo * 96)`, i.e. rounded to the nearest
  frame.

So a tap's timestamp (`Ttime`) is a tick value taken from the audio sample
counter (streamed lines) or the VBlank counter (VSYNC lines), sampled once per
frame.

**Judgement windows (checked in source; reading of the bit tables is mine,
2026-10-03).** Grading does not use millisecond windows. It quantises ticks into
4-tick cells (24 cells per beat) and looks each tap up in one of three 24-bit
tables, `thnum_tbl` in `scrctrl.c:136`:

| Table | Bits (cell 0 first) | Windows (1 = inside) |
| --- | --- | --- |
| `CK_TH_NORMAL` `0x00e79e79` | `111001111001111001111001` | 16 ticks wide, every 24 ticks, from 4 ticks before each 16th-note grid point to 12 after it |
| `CK_TH_LATE` `0x00f3cf3c` | `111100111100111100111100` | 16 ticks wide starting on each grid point |
| `CK_TH_HANE` `0x00c1cc1c` | `110000011100110000011100` | irregular, used for the "hane" (syncopated) pattern |

`thnum_get(cell, table)` counts the 0/1 transitions up to that cell, so an even
result means the cell is inside a window and an odd one means it is in a gap;
`result / 2` is the window index. `on_th_make()` converts each recorded press
(`ofs_frame`, which is really in ticks despite the name) to a cell with
`(ofs + ofsT) / 4`, where `ofsT = ofs_tick % 96 + 96`, and does the same for the
teacher's pattern (LATE adds 2 cells). The `exh_*` functions then score the
comparison: `exh_normal_add`/`exh_normal_sub` count presses inside or outside a
window, `exh_nombar_sub` penalises keys not in the teacher's key set (scaled by
how many keys the teacher uses), `exh_mbar_*_out` compare first key, first
window and press count, and `exh_yaku` scores window patterns with weights
6, 9, 15 and 18. The final rank thresholds are the `TCL_*` tables near the top of
the file. A port that reproduces `{cell index per press, key per press}` and
these tables reproduces judgement.
*Inferred, not tested:* the exact meaning of the `yaku` pattern codes.

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
