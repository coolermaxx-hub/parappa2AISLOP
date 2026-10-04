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

**Yaku (pattern) score (checked in source, 2026-10-04).** `exh_yaku`
(`scrctrl.c`) grades up to 72 judgement windows, two per byte of
`yaku_tmp_buf`, as `YAKU_EMPTY`, `YAKU_HIT` or `YAKU_SPOIL`. A window is
spoiled when it lies outside the line's `top_ofs`..`end_ofs` range, when a
press lands in the gap after it (odd `th_num`), when the key is not in the
example's key set (`otehon_all`), or when it already holds a press. It then
counts the window pairs (windows 2k and 2k+1) that match one of four shapes:

| Pair | Weight |
| --- | --- |
| hit, empty | 6 |
| hit, hit | 9 |
| empty, hit | 15 |
| empty, empty | 18 |

A pair containing a spoiled window scores nothing. Empty pairs count at most as
often as the rarest of the three pressed shapes. The plain rule
(`exh_yaku_original`, counted at x1) scores zero unless at least two of the
three pressed shapes occur; the "hane" rule (`exh_yaku_hane`, counted at x1.5,
`bairitu` 24/16) scores zero unless a hit-hit or empty-hit pair occurs. *Inferred:* the weights
reward syncopation and rests over pressing on every grid point; which beat
position window 2k falls on depends on `ofs_tick` and was not traced.

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

- One gameplay read of the sticks exists: `SpHatChangeSub` (`src/main/main.c`)
  samples **pad 0 only** and, from round 4 on, picks the hat variant from
  `ana[0..1]` (below 0x40 or at/above 0xC0 on either axis, checked in the order
  Y-low, X-high, Y-high, X-low). A port with more than one local player must still
  take this from player 1, and netplay must send the two axis bytes with the mask.

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

**Save sequencer states (named 2026-10-04).** `_P3MC_proc` (`p3mc.c`) drives saves and loads through the `P3MC_SAVE_*` / `P3MC_LOAD_*` states listed at the top of the file and maps the card manager's `MEMC_ERR_*` results onto them. The menu sees only the `P3MC_RES_*` codes from `P3MC_SaveCheck` / `P3MC_LoadCheck` (`include/menu/p3mc.h`). A port that replaces the transport keeps that result contract and can drop the state machine.

**Ending and bonus unlocks (`TsCheckEnding`, `menusub.c`).** Only story mode (`nMode == 0`) unlocks anything. After a clear, the game counts the stages whose clear count has reached the current round. If the stage just played was not already cleared at this round, an odd count (1, 3, 5) sets `endingGame` to `ENDING_BONUS_1..3` (capped at 3) and a count of 7 sets `ENDING_MOVIE`. `main.c` then loads the bonus game (`STDAT_STAGE_BONUS`, `bonusType = endingFlag - ENDING_BONUS_1`) or the ending cutscene (`STDAT_STAGE_ENDING`). Starting a stage with the R1+R2 "shuriken" cheat (`PLAY_TYPE_ONE`, `main.c`) clears a pending bonus flag, so no bonus game follows that play; `ENDING_MOVIE` is kept.

## Random number generators (2026-10-03)

Provenance: direct reading of `src/os/system.c`, `src/os/syssub.c`, `src/prlib/random.cpp`, and a grep of every caller (matching C, unchanged).

There are two unrelated generators.

**1. libc `rand()` drives gameplay and menus.** `srand` is never called anywhere in the game, so the stream starts from the C library's default seed. Two things advance it:
- `osFunc()` (`system.c:228`) calls `rand()` once at the top of every frame and throws the result away. The state therefore depends on how many frames have elapsed since boot, which makes gameplay randomness effectively a function of boot-to-moment frame count.
- Every use below advances it too, in whatever order the game happens to run.

`randMakeMax(max)` (`syssub.c:573`) is `((rand() & 0x7fff) * max) >> 15`. It is the gameplay entry point. Callers:

| Caller | Decides |
| --- | --- |
| `commake.c` (`comMakeSubYure`, key swaps, `comMakeingTbl_tmp[randMakeMax(...)]`) | how the teacher's command pattern is varied |
| `scrctrl.c:1149` `getLvlTblRand` | difficulty level move, picked from a percent table |
| `scrctrl.c:4757, 4772` (`rand() % 130`, `% 2`) | bonus-game kotama timing |
| `etc.c:157, 669` | demo stage choice, hook line choice |
| `main.c:1304` | attract-mode pick |
| `menu/menusub.c`, `menu/p3mc.c` | menu animation jitter and the save-file date pad |

The `%` forms in `menusub.c` and `scrctrl.c` use the low bits of `rand()` directly, so they differ in distribution from `randMakeMax`. A port that swaps the generator for a different one changes these distributions: keep the exact libc algorithm if bit-exact replays of original behaviour are wanted.

**2. `PrRandom()` (prlib, `random.cpp`) is cosmetic.** It is a 97-entry lagged pool seeded from a linear generator (`seed = seed*0x5d588b65 + 1`, initial seed 1, two warm-up draws in `PrInitializeRandomPool`). `PrFloatRandom` returns `PrRandom()/RAND_MAX` and re-draws if it reaches 1.0. Callers are all renderer effects: the noodle/menderer parameters (`mendererdata.cpp`, `mendereralpha.cpp`), the awful-mode change timer (`mendererawful.cpp:41`) and the SPRAM disturbance parameter (`spram.cpp:157`). None of them touches score, judgement or the pattern shown to the player. A third, separate generator lives in `menderer.cpp`: `GetRandom()` advances `noodleRandomSeed` with the Numerical Recipes LCG (`seed*0x19660D + 0x3C6EF35F`) and returns bits 8..23 as a float in [0, 1). It only picks the noodle colours (saturation, hue) and brightness phase, and the seed is reset to 0 at the start of every noodle draw (`menderer.cpp:271`), so the sequence is the same each frame and needs no syncing.

**Netplay consequences.**
- The gameplay stream must be seeded explicitly and advanced identically on all peers. The original's "one `rand()` per frame, never seeded" makes every session differ unless boot timing is identical, so a port needs its own deterministic stream and must pick the seed from the host.
- Menu and visual randomness can use any local generator; it never feeds back into the rhythm game. `PrRandom` can stay local to each peer.
- Replays (see the replay log section) record the tap log and difficulty moves (`mccReqLvlSet`) but not the command-pattern randomness, so the command patterns are re-rolled on playback. That only works because the replay is played back against the same per-frame `rand()` history, i.e. it is not guaranteed to reproduce identical teacher patterns. Worth testing against a real recorded replay before relying on it.

## CD, file loading and the WP2 stream container (2026-10-03)

Provenance: direct reading of `src/main/cdctrl.c`, `src/main/p3str.c`, `src/main/main.c`, `src/main/scrctrl.c`, `src/iop_mdl/wp2cd/iop/bgm_play.c` and `src/iop_mdl/wp2cd/wp2cd.h` (all matching C; nothing here was changed).

**Loads run as a cooperative task.** `CdctrlRead` / `CdctrlReadOne` set `cdctrl_str.status = 1` and start `cdctrlReadData` on the `MTC_TASK_CDCTRL` task. Every wait inside it (`sceCdSync`, a failed read, the TapCt transfer check) is `MtcWait(1)`, i.e. "come back next frame". Callers poll `CdctrlStatus()` or spin in `CdctrlReadWait()`. Failed reads are retried forever, and a bad INT header (`PACKINT_MAGIC` mismatch) hangs in an `MtcWait` loop on purpose. A port can do loads synchronously, as long as the code that polls `CdctrlStatus()` still sees at least one frame of "busy" where it expects it.

**Two file back ends.** `FILE_STR.frmode` picks `FRMODE_CD` (`sceCdSearchFile` + `sceCdRead` of whole 2048-byte sectors, rounded up) or `FRMODE_PC` (host0 `sceOpen`/`sceLseek`/`sceRead`, the development path). Both end with `FlushCache(WRITEBACK_DCACHE)`. For a port, FRMODE_PC is the model to follow: plain file reads by name.

**INT archives.** An `.INT` file is a chain of packs, each with a header (`PACK`: id, head_size, name_size, data_size, ftype, fnum, adr[]) followed by LZSS data (`PackIntDecode`: 4 KiB ring buffer, F = 18, THRESHOLD = 2, ring initialised to 0, write position starts at N - F, decoded size in the first word, data at +8). The pack `ftype` decides what happens after decoding:

| ftype | Action |
| --- | --- |
| `FT_VRAM` | every entry is a TIM2, sent to GS memory with `Tim2Trans` |
| `FT_SND` | first half of `adr[]` is SPU sample data (`TAPCT_BDSPUTRANS`), second half is the matching header (`TAPCT_HDIOPTRANS`); waits for `TAPCT_TRANSCHECK` |
| `FT_R1..FT_R4` | hat textures; only the pack matching `GetHatRound()` is uploaded |
| `FT_ONMEM` | each entry becomes a `UsrMemAlloc` block (models, animations, etc.), addressed later by index via `GetIntAdrsCurrent` |
| anything else | end of chain |

`PackIntDecodeWait` is the variant used for every load: it yields (`MtcWait(1)`) whenever `T0_COUNT` passes 230 scanlines, so decompression is spread across frames instead of dropping one. This only affects loading time, not gameplay.

**The WP2 stream is a multiplexed container, not just audio.** The IOP reads the file in blocks of `Tr1Size = maxChan * 512` bytes: one 512-byte slot per channel. Two slots, `ReqChan[0]` and `ReqChan[1]`, are copied to the SPU as the left and right ADPCM data. Every other slot that starts with a non-zero `P3STR_TRH.trSize` is a data packet, and the IOP DMAs `trSize` bytes of it straight into EE memory at `TransEEAdrs + trAdr` (`BgmTrans` in `bgm_play.c`). `TransEEAdrs` is set by `WP2_SETTRPOINT`: 0 for normal songs, the XTR buffer for streamed cutscenes.

A port has to demultiplex the same way: pick two channels for audio and apply the data packets to the cutscene buffer as the stream position passes them.

**Channel switching is how the music reacts to play.** The stream holds several stereo pairs, interleaved (inferred from the channel tables as alternative mixes of the same song; the audio itself was not inspected). `CdctrlWP2SetChannel(L, R)` changes which pair the IOP sends to the SPU, without seeking. `SetLineChannel` picks `scr_ctrl.cdChan` per score line. Each tap set can also carry its own pair (`tapset.chan`, checked in `scrctrl.c` around line 4360 while the current time is inside that tap set's window): -2 keeps the current pair, -1 picks from the line's "auto" table by the width of the tap window (`taptimeEnd - taptimeStart` against `scr_chan_auto_pp[i].time`), and anything else is used directly. `CdctrlWP2SetFileSeekChan` does a stop, seek and preload when play jumps to another line (`goto_job`, `scrctrl.c:3033`) or when an exam starts (`scrctrl.c:3664`); the seek target is the line's tick position converted to stream units minus the line's time offset (`ofs * 48 / 256`). Only lines with `gtime_type != GTIME_VSYNC` touch the stream. A port must switch at the same block boundary to stay in sync, since the stream position is the rhythm clock (see the clock section above).

**XTR streamed cutscenes.** `CdctrlXTRset` reads the XTR header (`read_size`, `channel`, `seek`, `trbox_tr[]`), decodes each `trbox_tr` preload (LZSS) to `usebuf + trpos`, calls `p3StrInit`, and points `WP2_SETTRPOINT` at `usebuf`, so the rest of the scene data arrives inside the audio stream. A missing `channel` field defaults to 6 channels. Playback (`main.c` around line 650) polls `WP2_GETTIME` each frame, converts it to frames (`units * 24 / 75`) relative to the header's seek position, and calls `p3StrPoll(frame)`. `p3StrPoll` refuses to go backwards (`back time`), starts or stops each object (`OD_SCENE` models, animations, cameras, fades, sprites) by its start and end frame, and draws them sorted. The movie ends on Start, at the end of the stream, or after 6540 frames.

## Frame loop, task scheduler and GS/DMA submission (2026-10-03)

Provenance: direct reading of `src/os/system.c`, `src/os/mtc.c`, `include/os/mtc.h`, `src/os/cmngifpk.c`, `src/prlib/dmaqueue.cpp`, `src/prlib/renderstuff.cpp` (matching C). Nothing here was changed.

**One scheduler cycle is one frame.** `MtcChangeThCtrl` (`mtc.c`) is a round-robin over 16 task slots. Each slot holds one thread, started by `MtcExec(fn, slot)`. A task gives up the CPU with `MtcWait(n)` (resume after n cycles) or `MtcExit()`. Slot 0 (`MTC_TASK_CTRL`) is `systemCtrlMain`, which calls `osFunc()` and then `MtcWait(1)`; `osFunc` blocks in `sceGsSyncV`, so the cycle is locked to VBlank. Within a frame, tasks always run in slot order:

| Slot | Task |
| --- | --- |
| 0 | system: `osFunc` (VBlank wait, pads, buffer swap) |
| 1 | `mainStart` (game flow) |
| 3 | `uramenFileSearchTask` |
| 5 | `ScrCtrlMainLoop` (score, timing, judgement) |
| 7 | `DrawCtrlMain` / `menuDraw` |
| 10 | CD loads and the sound fade-out |
| 13 | `FadeCtrlMain` |
| 15 | wipe effects |

So in every frame: VBlank, pad read, game flow, score/judgement, then drawing. A port that replaces threads with plain function calls must keep this order, and must keep `MtcWait(n)` semantics: a task started with `MtcExec` in a higher slot than the caller runs later in the same frame, while one in a lower slot first runs next frame. `MtcExec` on a busy slot kills the old task first. The scheduler checks a stack canary (`0x572a8b4c`) on every wait and hangs on overflow.

**`osFunc` order (`system.c:225`).** `rand()`; `CmnGifFlush()` (send last frame's 2D packets on PATH3); `sceGsSyncPath`; `sceGsSyncV` (wait for VBlank, returns the field: odd/even); `T0_COUNT = 0` (the hblank counter used for load throttling and debug meters); `GPadSysRead` + `GPadRead`; flip `outbuf_idx`; `CmnGifClear()`; set the half-pixel offset for the next field; wait for GIF DMA; `sceGsSwapDBuffDc`.

**Video mode.** `sceGsResetGraph(0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME)` with 640x224 draw buffers (`SCREEN_HEIGHT / 2`), 32-bit colour and 32-bit Z (`ZGEQUAL`). Each frame is rendered at half height and offset by half a line according to the field (`sceGsSetHalfOffset(..., oddeven_idx ^ 1)`), so the output is 60 fields per second. A port can render at full height and ignore the field offset; it does not feed back into game logic. Extra GS buffers (`drawEnvSp`, `drawEnvZbuff`, `drawEnvEnd`) share the draw env and differ only in FBP.

**2D path: the common GIF packet.** 2D code opens a packet with `CmnGifOpenCmnPk` (which resets TEXFLUSH, TEX1, TEST (Z always), PRMODECONT, CLAMP, RGBAQ) and closes it with a priority (`CmnGifCloseCmnPk(pk, pri)`). Up to 64 packets per frame. `CmnGifFlush` sorts them by `pri` with an exchange sort that is **not stable**: packets with equal priority can be reordered depending on what sits between them. A port that wants identical layering has to copy that exact sort, not use a stable sort.

**3D path: prlib DMA queue.** prlib collects VIF1 DMA chains per model and per transparent chunk with `AppendTransmitDmaTag(tag, group, depth)`, sorts them with libc `qsort` by group, then depth (`PrRenderStuff::CompareFunction`), and feeds them to a ring of DMA lists (`PrDmaQueue`) that the VIF1 channel walks under stall control (`D_CTRL` STS, `D_STADR` advanced on every `Append`). `qsort` is also not stable, so equal (group, depth) entries keep whatever order the PS2 newlib `qsort` produces. Transform and lighting run in VU1 microcode (with an EE fallback whose vertex kernels remain intentional assembly). Stage 19 appends one extra GIF register strip at the end.

**What a port must keep.** The slot order and one-cycle-per-VBlank rule are what tie input, clock, judgement and drawing together. The two unstable sorts only affect what is drawn on top, never game state. Nothing in the GS/DMA path writes back into game logic, except `T0_COUNT` thresholds that decide when loads yield.

## Boundary inventory (keyword search, not yet analysed)

| Boundary | Files |
| --- | --- |
| Pad reads | `src/os/syssub.c` (`scePadRead`), `src/os/system.c` |
| VBlank / frame | `src/os/system.c`, `src/os/mtc.c`, `src/main/etc.c` (analysed above) |
| Hardware timers (`T0..T3_COUNT`) | `src/os/system.c`, `src/main/cdctrl.c`, `src/prlib/render.cpp`, `src/prlib/renderstuff.cpp`, `src/prlib/menderer.cpp` |
| Audio stream (WP2) and SE (TapCt) | `src/main/cdctrl.c`, `src/iop_mdl/wp2cd_rpc.c`, `src/iop_mdl/tapctrl_rpc.c`, `src/iop_mdl/wp2cd/iop/*`, `src/main/scrctrl.c` |
| Score, judgement, rank | `src/main/scrctrl.c`, `src/main/etc.c`, `src/main/mbar.c`, `src/main/main.c` |
| CD / files | `src/main/cdctrl.c`, `src/main/p3str.c`, `src/os/system.c` (analysed above) |
| Memory card | `src/menu/memc.c`, `src/menu/p3mc.c`, `src/main/mcctrl.c` |
| RNG | `src/os/system.c`, `src/os/syssub.c`, `src/prlib/random.cpp` |
| GS / DMA / VU | `src/os/system.c`, `src/os/cmngifpk.c`, `src/prlib/*` (analysed above) |


## Readable renderer reconstruction (source/disassembly, 2026-10-03)

The reconstructed renderer preserves the following observed rules; these have
not been validated through a PS2 game run.

- Scalar animation searches clamp before the first and at/after the last key.
  Looping uses `fmodf`, including its negative-input sign; it does not turn
  negative times into positive wrapped times. Spline tangents are scaled by the
  segment duration. Static sampled results are shared per type as in the draft.
- Noodle color sampling retains its original LCG update order. Alpha setup makes
  four floating random calls and one rotation-speed random call plus its sign
  choice per strip. Modulation is suppressed in stages 6 and 16; phase bounces
  at plus/minus pi/2, reversing angular velocity and signed weight together.
- The EE chunk path preserves GIF prefix records, transforms the remaining
  vertices, sends the resulting DMA chain and advances the three scratchpad
  buffers. The handwritten kernels share VF1..VF11 and preserve strip state in
  VF17/VF19. Disturbance uses the VU R register and mixes seed/position components
  in the original order; a host RNG substitute would change behavior.
- Contour save destinations include a two-quadword prefix while the render
  mapping is relative to the packet base. This difference is explicit in the
  source. Its relationship to real contour history assets still needs testing.
- Memory-card user scanning retries the asynchronous load immediately after
  submitting it. Refactoring the nested jump preserves that retry and avoids
  marking a scan page complete while its load is pending.

DMA/GIF sizes, cache aliases and scalar tests establish only the represented
layouts and CPU-side rules. They do not prove transfer timing, frame output or
EE/VU float equivalence. Remaining work is tracked in `remaining-work.md`.


**Texture waves (decoded from VU source).** The texture curve is the normalized
sum of three sine waves. Amplitude, spatial cycles and temporal frequency have
separate packet vectors. The shader normalizes by the sum of the three amplitudes,
without an extra constant in the denominator. CPU phase time advances in scaled
1/60-second steps. See `noodle-texture-model.md` for instruction provenance,
operation order and the remaining ESIN/rounding validation limits.

### SPM deformation targets

Shape and cluster paths now use `SpmPositionTargets`: a count followed by
quadword indices relative to the first non-null context-1 geometry packet. Empty
records consume one word; repeated indices are preserved. This changes source
structure without changing target order, VU accumulation or the cache alias used
for writes. See [spm-geometry-layout.md](spm-geometry-layout.md) for provenance,
packet fields, sorting conventions and unverified asset/runtime details.

### SPA visibility and table binding

The animation node's pointer tail contains scalar shape-weight tracks, matrix
transforms, then an extra vector-transform group. Bind the tables before the
optimizer compacts the active transform entries; later groups retain their
original addresses. Integer visibility tracks use step sampling even with a
Linear interpolation header. A nonzero integer, including a negative value,
means visible. Original loop wrapping and parent/default visibility flags remain
in effect. See [spa-animation-layout.md](spa-animation-layout.md) for evidence
and unverified fields.

## EE-core init packet register slots (2026-10-03)

**The original `PrRenderStuff::InitializeEECore` stores three draw-environment
registers one slot ahead of the register address that follows them.** The
static A+D packet (`initEECoreDmaPacket`, `src/prlib/renderee.cpp`) lists, for
drawing context 2: FRAME_2, ZBUF_2, TEST_2, ALPHA_2, TEX1_2, FBA_2, PRMODECONT,
TEXA, COLCLAMP, PABE, XYOFFSET_2, SCISSOR_2, DTHE. The function writes the
scene's `xyoffset` into the PABE slot, `scissor1` into the XYOFFSET_2 slot and
`dthe` into the SCISSOR_2 slot (checked from the instruction offsets +176,
+192 and +208 against the data addresses). The DTHE slot is never written. The
source reproduces this exactly because the stores are part of the matching
code; whether it is a latent bug in the shipped game or is harmless (the
EE-core path may only run with defaults that make the shifted values inert)
is *not known*. A port should verify this on hardware or in an emulator
before copying the behaviour or fixing it.

## Stage map music (2026-10-04)

Provenance: direct reading of the `TsBGM*` / `tsBGMONE*` functions in
`src/menu/menusub.c` and `MenuVoice*` in `src/menu/menudata.c` (all matching
C; only names were added).

**All eleven map tracks play at once, and the current one is the only one
unmuted.** `MapBgmTbl` holds one voice per track: track 0 is a two-beat loop
heard while the cursor moves (`BGM_TRACK_MOVE`), tracks 1-10 belong to map
positions 0-9 (`BGM_TRACK_MAP(pos)`; 10 is the record shop). `TsBGMPlay`
starts every track together (`tsBGMONEPlay`), and `tsBGMONEflow` restarts each
voice from a frame counter: first after `lpTimeF` frames, then every `lpTime`
frames (0: never, the record shop's voice plays once). All loop lengths are
multiples of 144 frames, so the tracks stay on a common beat of 36 frames
(`BGM_BEAT_FRAMES`, 100 BPM at 60 fps). The clock is the frame count, not the
SPU, so a port has to advance it once per game frame.

**A place change waits for the beat, then follows a fixed 75-frame script.**
`TsBGMChangePos` only records the request; `TsBGMPoll` acts when track 0's
counter is a multiple of 36. On the next frame (`BGMCHG_CUT`) the old track is
silenced and the move loop plays at the current volume. At frame 72
(`BGMCHG_ARRIVE`) the move loop is silenced, the sting (voice set 23, the
record shop's voice, on channel 3) plays, and the new track restarts from its
top at volume 0, except the record shop's, which starts at full volume. On the
two following frames the sting drops to 2/3 and 1/3 and the new track rises
to 1/72 and 2/72 (the divisor is 72, not 3, so the rise is barely audible);
at frame 75 both jump to their final volumes.

**Fades:** `TsBGMPlay(no, time)` fades in linearly over `time` frames,
`TsBGMMute(time)` fades out over `time` frames and leaves the tracks running
silenced (`BGMST_MUTE`). Volumes are on a 0-256 scale and scale each voice's
own volume (`MenuVoiceSetVol`: `volume * vol >> 8`).

Original quirks, kept as they are:

- `TsBGMStop(time)` with `time > 0` does not fade or stop anything. It sets
  the state to `BGMST_ON | BGMST_FADE` and restores full volume; `time` is
  never stored. Callers still wait as if it faded (`TsBGMStop(38)` then a
  40-frame wait before the title, `TsBGMStop(32)` before a stage). The music
  is cut later by whatever stops the sound system.
- When a fade down ends, `TsBGMPoll` stops the music only if the state has
  neither `BGMST_ON` nor `BGMST_MUTE`. Only `TsBGMMute` starts a fade down,
  and its state has both, so the stop branch never runs.
- The voice bank mechanism is inert in this build: `MenuVoiceBankSet` only
  records the request and always returns 0, and every `VoiceSet` entry is in
  bank 0 (`menudata.c`), so the bank checks in `MenuVoicePlayVol` never skip a
  voice. `TsBGMLoadCheck` is therefore always false and the deferred start in
  `TsBGMPlay` (`wtLoad`) never runs.

## Screen noodle warp grid (2026-10-04)

`UG_NoodlesDisp` (`src/main/effect.c`, matching C) builds a grid of
(cntW + 1) x (cntH + 1) vertices stored column by column, cntH + 1 per column,
then draws one triangle strip per column pair. The strip loop steps through the
grid by `cntW + 1` per column instead of `cntH + 1`. With a square grid this is
the same thing; with cntW != cntH the strips pick the wrong vertices, and with
cntW > cntH the last strip reads past the allocation. The counts come from the
scene data (`NOODLES_STR` passed through `drawctrl.c`); whether any shipped
scene uses a non-square grid has not been checked. A port should keep the
square-grid behaviour and guard the other case rather than copy the overrun.

## 32-bit pointer assumptions (2026-10-04)

Found by compiling the EE sources with a 64-bit host gcc/g++
(`-Wint-to-pointer-cast`, `-Wpointer-to-int-cast`, and the `loses precision`
diagnostics under `-fpermissive`): about 150 sites in C and 110 in prlib C++.
Every one is fine on the EE, where pointers and `int` are both 32 bits. A
64-bit port has to deal with each group below; the matching source keeps the
original types.

- **File formats relocated in place.** Data loaded from disc stores 32-bit
  offsets that the game adds to the file base and writes back into the same
  word. `p3StrInitSd` (`usrD`, `dataD`, `adrD`, `ADRD::adrs`),
  `PackGetAdrs` (`pack.c`), `PACKINT_FILE_STR::adr` (`cdctrl.c`), the TIM2
  walkers in `src/os/tim2.c`, and prlib's `CalculatePointer` helpers
  (`model.h`, `animation.h`, `camera.h`) all do this. A port either keeps the
  game heap inside a 4 GB arena and treats these words as arena offsets, or
  converts each file into native structs at load time. `CalculatePointer` is
  the single point to change for SPM/SPA/SPC.
- **Handles kept in `u_int` fields.** `ADRD::handle` holds model, animation,
  camera, TIM2 and CL2 pointers; `P3SRT_OD::pad2` (original name) holds the
  scene handle from `PrInitializeScene`; `SD_FADE::subDadr` and
  `SD_DISPIN::subDadr` hold pointers into their own records. These need a
  pointer-sized field, which changes the on-disc layout, so they belong with
  the load-time conversion above.
- **EE memory map bits.** `PR_UNCACHED`/`PR_UNCACHEDACCEL` (`common.h`) OR
  0x20000000/0x30000000 into addresses, `PR_DECACHE` masks them off,
  `PR_DMA_SPR_ADDR` (`prpriv.h`) makes scratchpad DMA addresses, and
  `usrmem.c` tracks allocations as `u_int` positions. On a PC these become
  identity operations and plain pointers.
- **Addresses passed as `int` arguments.** `WP2Ctrl(WP2_OPENFLOC, (int)name)`
  and `WP2_SEEKFLOC` (`cdctrl.c`), `cdctrlReadSub`'s buffer, `TapCt` bank
  transfers (`scrctrl.c`, `wipe.c`, `menudata.c`), `TsNAMEINBox_Flow`'s
  argument (`menusub.c`) and `UsrMemGetAdr`. These are RPC-style interfaces to
  the IOP or generic message arguments; the port's replacements should take
  real pointers.
- **Packet pointers stored as `u_int`.** `TsUSERPKT::ptop`/`btop`
  (`pksprite.h`) and the DMA chain builders in `pksprite.c` and prlib hold the
  current packet position as an integer. They only exist to build GS/DMA
  packets, which a port's renderer replaces.
- **Link-time symbols.** `system.c` stores `(int)&_end` and
  `(int)&_stack_size` in static initialisers, which is only a constant
  expression on a 32-bit target.
