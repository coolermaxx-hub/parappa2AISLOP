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

**Pad input (checked in source, 2026-10-03).** `GPadSysRead()` (`src/os/syssub.c:55`)
runs the libpad state machine for each port (identify the pad, switch a standard
pad to analog mode, set vibration alignment) and copies the raw 32-byte report into
`sysPad[i].rdata`. `GPadRead()` (line 354) then fills `PADD`:

- `shot` is the held-button mask (the report is active-low, so it is inverted),
  `old` the previous frame's `shot`, `one` the buttons newly pressed this frame
  and `off` the buttons newly released (`padMakeData`).
- `ana[]` holds the analog sticks (0x80 = centre when absent) and `press[]` the
  DualShock 2 pressure bytes; on a DualShock 1 or a digital pad pressure is
  zeroed. `padPrsTreate` bumps a held button's pressure from 0 to 1 and zeroes the
  pressure of any button that is not held.
- `mshot`/`mone` are `shot`/`one` with the left stick mixed in as a D-pad
  (below 0x40 or above 0xBF counts as pressed). **Only the menu code reads
  these** (`src/menu/menusub.c:847`); the rhythm code (`tapEventCheck`) uses the
  plain digital `shot` and `one`, so the stick never taps notes.
- Vibration is requested by writing `padvib[]` during the frame; `padActSet`
  copies it to the next report and `padActClear` zeroes it afterwards, so a
  rumble lasts for the frames it is re-requested.

Because `GPadRead` runs once per frame in `osFunc` and edge detection is a
pure function of consecutive frames, a port can replace this layer with
`{held mask per frame}` and recompute `one`/`off` itself. A rollback netcode
only needs that 16-bit mask per player per frame.

**RNG (checked).** `osFunc()` calls `rand()` once every frame and discards the
result, so the RNG state depends on how many frames have elapsed. Netplay or
replays must reproduce the frame count, not just the inputs.
`src/os/syssub.c:574` builds a ranged random from `rand() & 0x7fff`.
prlib has its own generator (`src/prlib/random.cpp`, `PrFloatRandom`).

## Authoritative clock: GlobalTimeJob (2026-10-03)

Provenance: direct reading of `src/main/etc.c`, `src/main/cdctrl.c`, `src/main/scrctrl.c`, `src/iop_mdl/wp2cd/iop/bgm_play.c` (all already matching C; nothing here was changed).

**Two clock sources, one derived pair of values.** `global_data.TimeType` is `FGF_VSYNC` or `FGF_CD`. It is switched by `GlobalTimeJobChange` from the score setup (`scrctrl.c` around lines 3003, 3640, 3911, 4510) based on `GetTimeType()` of the current draw table. A stage line whose type is `GTIME_VSYNC` runs on the frame counter, every other line runs on the streamed BGM position.

`GlobalTimeJob()` runs once per main-loop iteration, immediately before `ScrTimeRenew()` (`scrctrl.c:4211`). It writes:

| Field | FGF_VSYNC | FGF_CD |
| --- | --- | --- |
| `vsyncTime` / `cdTime` | frames from `vsync_time[0]` | derived: `(Snd*3600 + tempo*48)/(tempo*96)` |
| `Snd_currentTime` (ticks, 96/beat) | `(frames*96*tempo + 1800)/3600` | `cdSampleTmp * tempo * 16 / 1875` |
| `Snd_cdSampleCnt` | derived from ticks via `CdctrlSndTime2WP2sample` | the raw stream counter |
| `currentTime` (frames) | `vsyncTime` | `cdTime` |

So the audio stream counter is the master in CD mode, and ticks are the unit the judgement code compares against. Frames are re-derived from it, not the other way round.

**Frame counter.** `vsync_time[51]` (`etc.c`) is incremented for every channel by `TimeCallback`, which is registered with `sceGsSyncVCallback` in `TimeCallbackSet`. Channel 0 is the global clock, channels 1.. are per-score-line (`TimeCallbackTimeGetChan(i)`, used for `lineTime` / `lineTimeFrame`), and `TCBK_CHANNEL_WIPE` serves the wipe effects. This runs in interrupt context, so a port must increment it exactly once per displayed frame at the point VBlank fires. `TimeCallbackTimeSetChanTempo` converts a tick position back to a frame count: `(ticks*3600 + tempo*48)/(tempo*96)`.

**Stream counter (`WP2_GETTIME`).** `CdctrlWp2GetSampleTmpBuf()` calls the IOP RPC `WP2Ctrl(WP2_GETTIME)` and stores the result in `cdSampleTmp`. It is called from `GlobalTimeJob` (CD mode) and from the logo/streamed-movie loop in `main.c:657`. It also prints `max cd time get[...]` when the RPC round trip, measured with `T0_COUNT`, sets a new maximum, which is a latency probe the original developers left in.

The IOP side (`BgmGetTime`) builds the value from `ReadOutCnt` (advanced by `TrackSize/2` for every SPU block-transfer interrupt, `gBgmIntr`) plus the position inside the current SPU transfer buffer (`sceSdBlockTransStatus`, `/1024`). It retries until no interrupt fired during the read (`gBgmIntrTime`). The result is therefore the SPU playback position and not a decode or read position.

**Unit.** One WP2 time unit is 256 samples at 48 kHz, i.e. 187.5 units per second. This is derived, not documented in the source, but three places agree: `ticks = units*tempo*16/1875` against `ticks/sec = 96*tempo/60`, `frames = units*24/75` against 60 frames/sec, and `ofsCdtime*48/256` in `scrctrl.c:4000` converts a line offset in 1/48000 s to units.

**What a port must keep.**
- Input sampling is against `Snd_currentTime`, which only advances when `GlobalTimeJob` runs. All taps within one frame see the same tick value (already noted for pads).
- In CD mode the tick value comes from SPU playback position, so audio latency is built in: whatever the stream position says is what the player heard.
- In VSYNC mode there is no audio dependency, which is what makes lines of that type deterministic and a natural netplay base.
- Rounding: the `+1800` and `+tempo*48` terms round to nearest. Do not change them.

## Replay log and memory card (2026-10-03)

Provenance: direct reading of `src/main/mcctrl.c`, `include/main/mcctrl.h`, `src/main/scrctrl.c`, `src/menu/memc.c` (matching C, unchanged).

**The replay is an input log, not a state dump.** `MC_REP_STR` (0x4528 bytes) is what the memory card stores for a replay. It holds the play mode/type/round/stage, up to 256 per-line score records, 128 level (difficulty move) bytes, up to 2560 tap records and up to 100 "versus other" snapshots of 32 bytes. Playback re-runs the game and feeds the recorded taps back in through the `PAD_REPLAY` pad type, so the game logic itself must be deterministic given the same taps, tempo and RNG stream.

**Tap record (`MC_REP_DAT`, 4 bytes).**

| Field | Bits | Meaning |
| --- | --- | --- |
| `timeP` | 17 | the tick value (`Ttime`) at which the tap was accepted |
| `padId` | 3 | key index (the logical button, after `KiTR` remapping in one-button play) |
| `resT` | 1 | a "reset" request was pending for this player |
| `holdT` | 1 | a "hold" request was pending for this player |
| `ply` | 2 | player slot, 0..3 |
| `useL` | 8 | score line the tap belongs to |

`mccReqTapSet` is called from `tapEventCheck` (`scrctrl.c:2003`, `:2088`) once per accepted tap, after the pad-type switch has decided which key it was. `mccReqTapGet` is its mirror: per player it walks the log with a private cursor, and returns a tap only when its recorded tick is `<=` the current tick and the line matches. So during replay a tap is delivered on the first frame whose tick has reached the recorded one, which is the same frame it was originally accepted on.

**Why this matters for the port.**
- A replay or a netplay input packet only needs `(tick, line, key, player, reset/hold)`, which fits in 4 bytes. The 17-bit tick limits one recording to 131071 ticks.
- `mccReqTapForward` / `mccReqTapForwardOwn` skip log entries that are already in the past when a line starts (printing `TAP forward!!`). That is the resynchronisation path, and is the model for late-input handling.
- `mccReqLvlSet/Get` (`scrctrl.c:1251`, `:4477`) store the difficulty moves, so the replay does not re-decide them. Anything else in the game that varies at runtime (RNG, the `vsothsave` snapshots in versus mode, `vsTapdat*`) must be recorded the same way or be derived from the log.
- Capacity limits (256 scores, 128 levels, 2560 taps, 100 versus snapshots) print `... save over!!` and drop the entry silently. They are real limits of the original format.

**Memory card I/O (`memc.c`, `p3mc.c`).** Plain `sceMcOpen/Read/Write` sequences driven by a small state machine (`pmw->...`), one call per frame, polled with `sceMcSync`. It also writes the icon and `sceMcIconSys` header. Names are converted to Shift-JIS by `setAscii2SjisCode` (`mcctrl.c`), which maps ASCII to full-width codes through `ascii2sjiscng_tbl`. A port should replace the transport (files in a save directory) but keep the `MC_REP_STR` layout, so original saves stay readable.

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
