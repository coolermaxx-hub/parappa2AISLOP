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
- The WP2 position is counted in 256-sample units of a 48 kHz stream (derived
  from three formulas that agree; see the WP2 stream section below). The audio
  data itself was not inspected.

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
window, `exh_nombar_sub` penalises keys not in the teacher's key set (scaled
oddly, see below), `exh_mbar_*_out` compare first key, first
window and press count, and `exh_yaku` scores window patterns with weights
6, 9, 15 and 18. The final rank thresholds are the `TCL_*` tables near the top of
the file. A port that reproduces `{cell index per press, key per press}` and
these tables reproduces judgement.
*Inferred, not tested:* the exact meaning of the `yaku` pattern codes.

**Exam sub-scores (checked in source, 2026-10-05).** `ExamScoreCheck` grades a
line three times and `now_score` is the sum of the three `EXH_TOTAL` results
(`exam_score[0..2]`): `exh_str_normal` and `exh_str_original` on `CK_TH_NORMAL`
windows and `exh_str_hane` on `CK_TH_HANE`. Hook lines run only
`manemane_check`. Each program's raw value is multiplied by its `bairitu` and
divided by 16 (truncating), so 48 means x3, 32 x2, 24 x1.5 and 8 x0.5.
"In set" below means a key the teacher's pattern uses (`otehon_all`).

| Program | Raw value | normal | original | hane |
| --- | --- | --- | --- | --- |
| `exh_normal_add` | +1 per in-set press inside a window | 48 | | |
| `exh_normal_sub` | -1 per in-set press outside a window | 48 | | |
| `exh_nombar_sub` | -1 per press of a key not in set, then scaled (bug below) | 48 | 48 | 48 |
| `exh_mbar_key_out` | minus the teacher's press count unless the first keys match (0 if the teacher has none; always 0 in one-button play) | 48 | 16 | 16 |
| `exh_mbar_time_out` | the same for the first press's window | 48 | | |
| `exh_mbar_num_out` | minus the difference in press counts | 32 | | |
| `exh_allkey_out(_nh)` | -1 if some in-set key was never pressed (original and hane count only presses inside windows) | 32 | 48 | 16 |
| `exh_renda_out` | -1 for mashing: at least three more presses than the window has steps | 32 | 48 | 16 |
| `exh_mane` | copying: 2 x the (teacher press, player press) pairs in the same window with the same key, minus the player's presses; the better of normal and late windows | 8 | | |
| `exh_yaku_*` | window patterns, see above | | 16 | 24 |
| `exh_command` | always 0 | | 16 | |

`exh_all_add` then totals the slots, except that a missed key (`EXH_ALLKEY_OUT`
non-zero) makes the sub-score 0 and mashing makes it -100. At COOL and
COOL/GOOD rank the copy checks (missed key, press count, first key, first
window, copying) are forced to 0 and every key counts as in set, so freestyle
is not marked down for differing from the teacher. In one-button play only
triangle is in set.

`exh_nombar_sub` (BUG, matched code): the scaling was meant to follow how many
keys the teacher uses (x3 for one, x2 for two, halved for five or six), but the
loop counts only the four low key-code bits, which are L2, R2, L1 and R1. So
the penalty is x3 when the teacher uses exactly one of L1/R1, x2 when it uses
both, and unscaled otherwise; the halving never happens. A port that wants the
original scores keeps the bug.

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

**Line score adjustments (checked in source, 2026-10-05).** After `ExamScoreCheck` grades a line (`ScrExamSetCheck`, `scrctrl.c`), gameplay lines (`PSTEP_GAME`) halve `now_score`, rounding up, once for one-button play (`PLAY_TYPE_ONE`) and again for the easy table (`PLAY_TABLE_EASY`), so both together quarter it. During a replay the computed line score is then thrown away and replaced by the recorded `MC_REP_SCR` (line score and the three exam scores); a missing record gives 0. Outside replays and demos each line's result is recorded. So a replay always shows the recorded scores even if its taps would now grade differently. A positive line score counts toward `exam_tbl_up`, zero or less toward `exam_tbl_dw`, and outside versus play the line score is added to the running `score`, which is then clamped at 0.

**Rank meter (checked in source, 2026-10-05).** `rank_level` (`RANK_LEVEL`, `etc.h`) has in-between steps: COOL, COOL/GOOD, GOOD/COOL, GOOD, GOOD/BAD, BAD/GOOD, BAD, ... AWFUL, then the END levels. `RANK_LEVEL2DISP_LEVEL` shows COOL for the first two, GOOD for the next three, and so on, so one bad line from GOOD only reaches GOOD/BAD and a second is needed to show BAD. After each exam line `ScrExamSetCheck` moves it one step with `levelUpRank` / `levelDownRank` (tables in `scrctrl.c`), depending on the exam type the stage script gives the line (`SCRRJ_EXAM`; inferred to follow the rank section being played, not checked against stage data) and its score against the COOL threshold (`exam_coolP`, see below):
- COOL exam: below the threshold steps down, otherwise up.
- GOOD exam: at or above the threshold up, zero or less down; otherwise GOOD/COOL falls back to GOOD and the other in-between levels step up, so the meter drifts back to GOOD. In one-button play or on the easy table the result is capped at GOOD, so COOL cannot be reached from a GOOD exam there.
- BAD and AWFUL exams: any positive score steps up, otherwise down.
- Hook exam (the chorus practice): the line passes after ten scoring lines (`exam_tbl_up >= 10`), otherwise it loops if the script has a loop job.
- Hook result: when the hook step ends, `selPlayDispSetPlay` (`main.c`) stores `exam_tbl_up - exam_tbl_dw / 2`, clamped to 0..10, as `HookClrCnt`. A bonus exam (`EXAM_BONUS`) repeats its line until it has been checked more than `HookClrCnt` times, so a better hook practice gives more repeats. Only the bonus game step resets that counter (`bonusGameInit`), so the bonus exam is inferred to belong to the bonus game; the stage scripts were not checked. The skip cheat (`selPlayDispSetPlayOne`) plays only the last step, so `HookClrCnt` stays at its cleared 0 and the bonus line passes on its first check.

**Versus exchanges (checked in source, 2026-10-05).** In versus play the COOL threshold of a line is just `exp`, the score for copying the opponent's pattern exactly. The opening line of an exchange (`TAPSCODE_ANSWER_F`) sets the player's score to `VS_START_SCORE` (500) plus the line score. On each answer line (`TAPSCODE_ANSWER`) the difference `now_score - exp` is applied: a negative difference comes off the player's own score, a positive one off the opponent's, both clamped at 0. The exchange is judged when the script says so or when either side reaches 0: equal scores draw, otherwise the higher wins, with the shutout and both-over-`VS_ROUND_HIGH_SCORE` variants picking the reaction (see `SCREX_AR_*` in `scrctrl.h`). The battle ends early once one side has more wins than the other could still catch up with (best of five; `SCREX_AB_*`). `SCRSUBJ_VS_RESET` puts both sides back to 500. After a versus answer line, a player who beat the copy score and has a non-zero second exam sub-score (`exam_score[1]`, the "original" program `exh_str_original`, i.e. something of their own) hands their pattern to the other side as the next one to answer (`vsTapdatSet`). The pattern keeps only presses inside a `CK_TH_NORMAL` window, snapped to their step, one per step and within the tap window; if those presses miss any key the line uses, the previous pattern stays. The pattern in force is logged with the replay (`vsTapdatSetMemorySave`), and replays load it from the log instead (`vsTapdatSetMemoryLoad`).

**Computer opponent (`commake.c`, checked in source, 2026-10-05).** In versus against the computer, the computer's answer is built once, on the first score update of its tap set (`tapEventCheck`), by `computerMaking`. It snaps two patterns to step slots (`time / TICKS_PER_STEP`, truncated, at most `CM_STEP_MAX` = 32): the line's own tap set (`cm_str_mt`) and the pattern it must answer (`cm_str_now`, from `vs_tapdat_work`: the opponent's presses that `vsTapdatSet` kept, one per step and only those inside a judgement window, or the script's question pattern after `vsTapdatSetMoto`). `maxBox` is the number of steps in the tap window minus one. The versus level (`level_vs_enumL`) picks a table and `randMakeMax` picks one strategy from it with equal odds per entry:

| Level | Strategies (entries) | Timing jitter |
| --- | --- | --- |
| `LVS_1` | 0, 0, 1, 1, 2, 2, 3, 3 | one offset in -11..10 ticks |
| `LVS_2` | 1, 2, 3, 5, 5, 6, 6 | 1-3: -11..10; 5, 6: two offsets in -5..4 |
| `LVS_3` | 7, 7, 8, 8, 9, 9 | -5..4 plus -2..1 |
| `LVS_4` | 15, 15, 16, 16 | three offsets in -2..1 |

- 0 and 2: copy the opponent. Strategy 0 also has a key-change step that can never run (it needs the most and least used keys to differ while both are none).
- 1: play the first half of the line's own tap set twice; if that half is empty, the second half twice; if still empty, the whole tap set.
- 3: copy the opponent, then put the least used key in the middle of the first run of three empty steps; if there is none, drop one random press of the most used key (when it has at least two).
- 7: copy the opponent with its most and least used keys swapped, then fill the first three-step gap with the (pre-swap) most used key.
- 15: copy the opponent, fill the first three-step gap with the most used key, then keep filling gaps with the least used key, alternating between a gap's last and middle step, until no gap of three is left; with no gap at all, drop a press as in 3.
- 16: copy the opponent and, if it pressed fewer than `maxBox / 2` keys, repeat each press on the following step when that step is empty.
- 5, 6, 8 and 9 are 15, 3, 3 and 15 again with the smaller jitter shown.

"Most" and "least used" count the opponent's keys; ties go to the lower key for the most used and to the higher key for the least used. Offsets are added in ticks on top of the step time and stay under half a step, so presses never change order. The computer presses the result in slot order, and each press is logged like a player tap, so replays reproduce it. The strategy and jitter come from the shared `rand()` stream, so they are not reproducible from the taps alone. `computerMaking` ignores its output-size argument; the answer fits because it has at most 32 presses. The helpers loop over `maxBox` slots of 32-entry arrays, so a tap window longer than 33 steps would overrun them (inferred from the code; stage data not checked).

**Bonus game (checked in source, 2026-10-05).** `bonusGameCtrl` (`scrctrl.c`) runs once per score update while the script's `SCRSUBJ_BONUS_GAME` job is active, from tick 0 to `BONUS_GAME_END_TICKS` (18048, 188 beats). Four targets map to triangle, circle, cross and square (`KiTR`..`KiSQ`, pad 0's newly pressed buttons). Each target waits `10 + max(0, rand() % 130 - streak)` updates, then with even odds shows the item (`BNGKA_LIFTED`, which stays until pressed) or a decoy (`BNGKA_LIFT_NG`, 24 updates). Pressing a shown item adds 26, 15, 9 or 5 points for a reaction under 3, 6, 9 or more updates and extends the streak; pressing during the wait or on a decoy costs 9, 13, 18 or 24 points (waited under 8, 13, 20 or more updates), resets the streak and knocks the target back for 180 updates. A longer streak shortens the waits. `bonusPointSave` stores points won minus points lost, clamped at 0, as `BonusScore`; presses on a target that is knocked back or broken score nothing. The bonus is added to the stage score that unlocked the game for the single-play ranking (`game_status.bonusG`, `menusub.c` ranking update). The two `rand()` calls are on the shared C generator (see the RNG section), so the pattern is not reproducible from the taps alone.

**Adaptive difficulty (checked in source, 2026-10-05).** Only the player's own slot (`GPLAY_TBLCNG_REQ`, set for Parappa in `etc.c`) adapts. There are two levels: a control level `global_data.tap_ctrl_level` (`TCT_LV00`..`TCT_LV15`) and the pattern level `global_data.tapLevel` actually played.

1. *Per exam line* (`ScrExamSetCheck`): `exp` is the score the line would give for copying the example exactly (`ExamScoreCheckSame`, which re-grades a copy of the player state; 150 when the rank is COOL or COOL_GOOD). The COOL threshold is `coolup + exp` (just `exp` in versus). `exam_tbl_updownSet` then bumps counters comparing the line score `now` with 0, `exp`, `exp/2`, the threshold, half of it and the midpoint of `exp` and the threshold (`TCL_DO_*_OVER/MORE/UPTO/UNDER` for >, >=, <=, <). Slot `TCL_DO_NONE` is a balance: +1 when `exp` is at least half the threshold, -1 otherwise.
2. *Per score line* (`tapLevelChange`, called from `ScrCtrlIndvNextRead`): `exam_tbl_updownChange` picks a rule from `tcl_ctrl[round][type]`: `TCL_TYPE_COOL` when the displayed rank is COOL, otherwise the control level's "EZ" rule when the balance is negative or its "HD" rule (`+ TCL_TYPE_HD_TOP`). The move is the rule's up counter minus its down counter, clamped to `max` / `-min`; a negative `max` or `min` means "move by exactly that many" whatever the count. The control level moves by that and is clamped to 0..15, and all counters reset.
3. `tapLevelChangeSub` then draws the pattern level from the stage's `TAPLVL_DAT` for that control level (one table set per round, easy mode after the normal ones): `per[0..16]` are percentages walked against `randMakeMax(100)` (see the RNG section). The hook step picks one hook line at random (`inCmnHookSet`) and stores the level it was played at (`inCmnHook2GameSave`); when that sound line comes up in the game, `inCmnHook2GameCheck` reuses the stored level. Stage 8 keeps the previous level on sound lines 24-26. Replays read the recorded level instead (`mccReqLvlGet`). A changed level restarts the line's tap pattern (`selectIndvTapResetPlay`).

With `tapLevelCtrl != LM_AUTO` the pattern level stays fixed. A port must keep the counter comparisons and rule tables exactly, since the pattern the player sees next depends on them.

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
- Pressure has one gameplay effect: a tap passes `&press[key]` to the draw
  side (`tapEventCheck` → `DrawTapReqTbl`), and while the tap's animation runs
  `DrawObjStrDispTap` (`drawctrl.c`, `PAD_PRESS_HELD` 100 / `PAD_PRESS_HARD`
  180) advances its clock by 2 per frame above 100 and 3 above 180, and marks it
  held so a hold animation keeps going. On a pad without pressure the bytes are
  0 or 1, so those animations always run at 1x. Scoring does not read pressure.
  (`GetPadbit2PressPad` in `syssub.c` would index `press[]` with the button mask;
  it is never called.)
- Vibration is requested by writing `padvib[]` during the frame; `padActSet`
  copies it to the next report and `padActClear` zeroes it afterwards, so a
  rumble lasts for the frames it is re-requested.
- Scripted players (`PAD_DEMO`: the teacher, Boxy, and Parappa in the attract
  demo) press each key of their tap set at its exact time, except in "follow"
  lines. A score line whose script sets `TAP_FOLLOW_SAVE` copies the player's
  key presses at the end of their tap set (`followTapSave`); a later line set to
  `TAP_FOLLOW_LOAD` makes its scripted player play, in order and at the
  player's offsets, the tap-set entries those presses were matched to
  (`followTapLoad`), so that character echoes the player's timing. Which stages use it was not checked against stage data.
- The only gameplay rumble is a turn cue (`tapEventCheck`, `scrctrl.c`,
  checked 2026-10-05). From one step before a human player's tap set opens,
  `scr_tap_vib_on` counts the score updates spent in it: the first one does the
  tap set's setup (versus copy or computer pattern), and the second and third
  request `padvib[0]` if vibration is on, so the small motor runs for two
  updates. The counter is cleared when the next tap set is read or the pattern
  is reset. The computer player and replays never rumble.

- One gameplay read of the sticks exists: `SpHatChangeSub` (`src/main/main.c`)
  samples **pad 0 only** and, from round 4 on, picks the hat variant from
  `ana[0..1]`, the right stick (`PAD_ANA_RX/RY`: below `PAD_ANA_LOW` 0x40 or at
  or above `PAD_ANA_HIGH` 0xC0 on either axis, checked in the order Y-low, X-high,
  Y-high, X-low). A port with more than one local player must still take this
  from player 1, and netplay must send the two axis bytes with the mask.
- `urawazaKeyCheck` (main.c) is a level-select cheat: with R3 held, the right
  stick's direction picks one of 17 tap levels (`TLL_*`), or `randMakeMax(17)`
  picks one if the stick is centred (so it consumes RNG). It is inert in this
  build: only the debug loop `ura_check` calls it, nothing ever sets
  `urawaza_levelsel_bottun` to a level or `urawaza_skip_bottun` to TRUE, so the
  `LM_FIX` branch in the stage start and `selPlayDispSetPlayOne` never run from it.

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

**Frame counter.** `vsync_time[51]` (`etc.c`) is incremented for every channel by `TimeCallback`, which is registered with `sceGsSyncVCallback` in `TimeCallbackSet`. Channel 0 is the global clock, channels 1.. are per-score-line (`TimeCallbackTimeGetChan(i)`, used for `lineTime` / `lineTimeFrame`), and `TCBK_CHANNEL_WIPE` serves the wipe effects. This runs in interrupt context, so a port must increment it exactly once per displayed frame at the point VBlank fires. `TimeCallbackTimeSetChanTempo` converts a tick position back to a frame count: `(ticks*3600 + tempo*48)/(tempo*96)`. The source writes these conversions as `TICKS_TO_FRAMES` / `FRAMES_TO_TICKS` (`include/main/etc.h`).

**Stream counter (`WP2_GETTIME`).** `CdctrlWp2GetSampleTmpBuf()` calls the IOP RPC `WP2Ctrl(WP2_GETTIME)` and stores the result in `cdSampleTmp`. It is called from `GlobalTimeJob` (CD mode) and from the logo/streamed-movie loop in `main.c:657`. It also prints `max cd time get[...]` when the RPC round trip, measured with `T0_COUNT`, sets a new maximum, which is a latency probe the original developers left in.

The IOP side (`BgmGetTime`) builds the value from `ReadOutCnt` (advanced by `TrackSize/2` for every SPU block-transfer interrupt, `gBgmIntr`) plus the position inside the current SPU transfer buffer (`sceSdBlockTransStatus`, `/1024`). It retries until no interrupt fired during the read (`gBgmIntrTime`). The result is therefore the SPU playback position and not a decode or read position.

**Unit.** One WP2 time unit is 256 samples at 48 kHz, i.e. 187.5 units per second. This is derived, not documented in the source, but three places agree: `ticks = units*tempo*16/1875` against `ticks/sec = 96*tempo/60`, `frames = units*24/75` against 60 frames/sec, and `ofsCdtime*48/256` in `scrctrl.c` converts a line offset in milliseconds to units (48 samples per millisecond; the same offset is turned into ticks as `tempo*96*ms/60000`).

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
- Capacity limits (256 scores, 128 levels, 2560 taps, 100 versus snapshots) print `... save over!!` and drop the entry silently. They are real limits of the original format. One exception: `mccReqLvlSet` checks the level count against 256 although `levelN` holds 128 (`MC_REP_LEVEL_MAX`), so a 129th difficulty move would overwrite `mc_rep_dat_cnt` and the start of the tap log. A level entry is recorded each time `ScrCtrlIndvNextRead` (`scrctrl.c`) finishes reading a score line, so a play with more than 128 lines would hit it; whether any stage gets that far has not been checked. A port should clamp at 128 and keep the original layout.

**Memory card I/O (`memc.c`, `p3mc.c`).** Plain `sceMcOpen/Read/Write` sequences driven by a small state machine (`pmw->...`), one call per frame, polled with `sceMcSync`. It also writes the icon and `sceMcIconSys` header. Names are converted to Shift-JIS by `setAscii2SjisCode` (`mcctrl.c`), which maps ASCII to full-width codes through `ascii2sjiscng_tbl`. A port should replace the transport (files in a save directory) but keep the `MC_REP_STR` layout, so original saves stay readable.

**Save sequencer states (named 2026-10-04).** `_P3MC_proc` (`p3mc.c`) drives saves and loads through the `P3MC_SAVE_*` / `P3MC_LOAD_*` states listed at the top of the file and maps the card manager's `MEMC_ERR_*` results onto them. The menu sees only the `P3MC_RES_*` codes from `P3MC_SaveCheck` / `P3MC_LoadCheck` (`include/menu/p3mc.h`). A port that replaces the transport keeps that result contract and can drop the state machine.

**Damaged-file check (`_P3MC_file_chk`, `p3mc.c`).** A save file counts as damaged only when its size differs from the expected one. The check also reads the card's "closed" attribute (`sceMcFileAttrClosed`), but tests it as `!closed || closed`, so a file left unclosed by an interrupted write passes. A port that wants to detect torn saves needs its own check; matching the original means ignoring that state.

**COOL crown history (`P3LOG_VAL.logCOOL`, `include/menu/menu.h`).** Each stage keeps the rounds of its last four COOL clears as nibbles, newest in the low nibble. `TsClearSet` (`menusub.c`) shifts the word left by 4, masks it to 16 bits and ORs in the round (1..4). The stage map draws one crown per non-zero nibble, coloured by that round. A port keeps the 16-bit packing so old logs still show the same crowns.

**Attract demo (`titleDisp`, `GlobalLobcalCopy`, checked in source, 2026-10-05).** Left on the title, the game alternates the title movie with a demo play. The demo is single play of the stage's last step (the song itself) at the current circuit, on stage `1 + randMakeMax(n)`, where `n` is the number of stages cleared in a row from stage 1 in circuit 1 (`clrFlg[0]`, via `clearStageCheck`), clamped to 5..8. Parappa uses the `PAD_DEMO` pad type like the teacher and Boxy, so he presses every key of the tap set at its exact time. Start stops it at once; otherwise it ends at the script's fade-out point (`ScrEndCheckFadeOut`) with a 120-frame music fade. Nothing is recorded.

**Circuit progression (`TsClearSet`, `menusub.c`, checked in source, 2026-10-05).** It runs when a play reaches the save screen (see "What a finished play records"), only for stages 1-8 and never for two-player versus. `P3LOG_VAL.nRound` is the circuit, 0-based (the menu shows `CIRCUIT n+1`).

- Single play: in circuit 0 a stage's first clear makes Parappa walk on to the next stage on the map (8 wraps to 1). In circuits 0-3 the clear sets the stage's bit in `clrFlg[nRound]`. `clrCount[stage]` counts clears, capped at the circuit number + 1. A COOL clear records the circuit in `clrCOOL` and the crown history; from circuit 4 on, a COOL clear on a stage whose best COOL circuit is below 4 sends Parappa to the record shop for that stage's record. When every stage's `clrCount` has reached the circuit number + 1, the circuit goes up (capped at 1,000,000); reaching circuit 4 also sends him to the record shop.
- Versus the computer: `clrVSCOM1[stage]` keeps the highest level won (1-4). From circuit 4 on, a level-4 win that completes level 4 on all eight stages sends him to the record shop for record 9.
- A record shop visit replaces the walk with a fixed route from the stage (`RecordShopRute`).

**Ending and bonus unlocks (`TsCheckEnding`, `menusub.c`).** Only story mode (`nMode == 0`) unlocks anything. After a clear, the game counts the stages whose clear count has reached the current round. If the stage just played was not already cleared at this round, an odd count (1, 3, 5) sets `endingGame` to `ENDING_BONUS_1..3` (capped at 3) and a count of 7 sets `ENDING_MOVIE`. `main.c` then loads the bonus game (`STDAT_STAGE_BONUS`, `bonusType = endingFlag - ENDING_BONUS_1`) or the ending cutscene (`STDAT_STAGE_ENDING`). Starting a stage with the R1+R2 "shuriken" cheat (`PLAY_TYPE_ONE`, `main.c`) clears a pending bonus flag, so no bonus game follows that play; `ENDING_MOVIE` is kept.

**What a finished play records (`gamePlayDisp`, `main.c`, checked in source, 2026-10-05).** Pressing Start during play (either pad in two-player versus; not during the stage's hook section, `CBE_HOOK`) ends it as cancelled, which records nothing. Otherwise, single play with a final rank of COOL or GOOD goes to the save screen and adds one to that stage's COOL or GOOD clear count; a lower rank goes back to stage select. A versus match against a person always goes to the save screen and counts nothing. Against the computer only a win (`vsWin > vsLost`) is saved and counted in `stClrCntVs`. Clear counts stop at 0xffffffff. Replays return to the replay menu and demos to stage select, with no save.

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

## Boundary inventory

Every boundary found by the original keyword search now has a section above.

| Boundary | Files | Where it is covered |
| --- | --- | --- |
| Pad reads | `src/os/syssub.c` (`scePadRead`), `src/os/system.c` | Pad input, in the rhythm clock section |
| VBlank / frame | `src/os/system.c`, `src/os/mtc.c`, `src/main/etc.c` | Authoritative clock; frame loop |
| Hardware timers (`T0..T3_COUNT`) | `src/os/system.c`, `src/main/cdctrl.c`, `src/prlib/*` | Below |
| Audio stream (WP2) and SE (TapCt) | `src/main/cdctrl.c`, `src/iop_mdl/*`, `src/main/scrctrl.c` | CD, file loading and the WP2 stream container |
| Score, judgement, rank | `src/main/scrctrl.c`, `src/main/etc.c`, `src/main/mbar.c`, `src/main/main.c` | Rhythm clock section (yaku, line score, rank meter, versus, bonus game, adaptive difficulty) |
| CD / files | `src/main/cdctrl.c`, `src/main/p3str.c`, `src/os/system.c` | CD, file loading and the WP2 stream container |
| Memory card | `src/menu/memc.c`, `src/menu/p3mc.c`, `src/main/mcctrl.c` | Replay log and memory card |
| RNG | `src/os/system.c`, `src/os/syssub.c`, `src/prlib/random.cpp` | Random number generators |
| GS / DMA / VU | `src/os/system.c`, `src/os/cmngifpk.c`, `src/prlib/*` | Frame loop; renderer reconstruction |

**Hardware timers (checked in source, 2026-10-05).** No timer feeds game state.
Both timers count H-blanks. `osFunc` zeroes `T0_COUNT` right after
`sceGsSyncV`, so it reads as "lines since the last VBlank" (the value read just
before the reset, `total_h_cnt`, is unused). `cdctrl.c` uses it twice: the
LZSS decode loop (`PackIntDecodeWait`) yields with `MtcWait(1)` whenever the
count is past `DECODE_WAIT_HLINE` (230, near the end of the field), so a large
decode is spread over several frames instead of dropping one, and the
`WP2_GETTIME` round trip is timed for the `max cd time get` debug print. The
prlib renderer reads `T3_COUNT` only for its debug render-time statistics, and
the `SyoriLine*` CPU meter it would feed is stubbed out in this build
(`src/dbug/syori.c`). A port can drop the statistics and replace the decode
throttle with any per-frame time budget, as long as loads still report busy for
at least one frame (see the CD section).


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
- Contour mapping destinations select a current/previous vertex pair. Save
  writes `previous.position` at pair + 0x20; render writes `current.position`
  at pair + 0x00. The [built-in model audit](spm-packet-layout.md) verifies all
  201 mapped destinations. The offset is a pair member, not a packet prefix;
  save timing remains controlled by the original caller.
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

## nalib matrix products (2026-10-04)

`NaMATRIX<T, rows, columns>` stores column vectors. The original VU sequence
visible in `PrModelObject::GetPrimitivePosition` (0x140d20 onwards) loads the four
columns, seeds ACC with column 0 times x, then accumulates y, z and w. The
float4 `Apply` and `Multiply` now have explicit specializations containing the
same hardware instructions. Their argument and result types remain unchanged.
Game transforms still use VU arithmetic, including its accumulation order.

The generic products previously accepted only float4 operands regardless of
the enclosing template. They now use `T` and the matrix dimensions. `Apply`
maps a `columns`-component vector to a `rows`-component vector; `Multiply`
accepts a square right operand of size `columns`, retaining the left operand's
shape. These scalar implementations are inferred from the evidenced column
convention, not recovered instruction matches for unused template instances.
They seed each sum with the first product and accumulate in column order.
Their complete result is staged before writing the output, permitting output
to alias either input, including squaring a matrix in place. `ApplyTransposed`
retains its separate scalar convention and initial-zero accumulation.

Validation:

- `tests/nalib/matrix_products.cpp` passes as a native C++98 executable. It
  checks noncommuting matrix products, vector products, output references,
  left/right/both-input aliasing, float3 column permutations, integer4 storage,
  and rectangular row/column products. It also compiles to a PS2 object with
  the historical EE compiler at `-O2`.
- The existing `tools/test_matrix_layout.py` EE scalar checks pass. The new
  product test is not run through that generic MIPS adapter: its EE assembly
  uses R5900 integer multiply-accumulate instructions the adapter does not
  support. Native execution does not validate PS2 floating-point edge cases.
- A fresh historical build links both ROMs. Both ROM images are byte-identical
  to the pre-change build. Objdiff's per-unit measures are unchanged:
  1316/1429 exact functions and 259636/342284 exact code bytes. The unchanged
  IOP checksum passes and the unchanged main-ROM checksum still fails.

Native check command:

```sh
g++ -std=c++98 -O2 -Wall -Wextra -Werror -Isrc -Iinclude/rtl/ee -Iinclude/rtl/common tests/nalib/matrix_products.cpp -o /tmp/matrix-products
/tmp/matrix-products
```

The restored cloud workspace has inode numbers too large for the historical
32-bit compiler's `stat` interface. This pass copied build inputs to
`/tmp/parappa-matrix-products` and used the existing rootless toolchain there.
The build log, original-output hashes and objdiff report are retained outside
Git as `/workspace/shared/parappa-env/matrix-products-{build.log,before.json,report.json}`.
No game assets or generated outputs are committed.

## Matrix inverse is a rigid inverse (2026-10-05)

`NaMATRIX::Inverse` (`src/nalib/namatrix.h`) calls `sceVu0InversMatrix`. The SDK routine (`asm/sdk/libvu0.s`) transposes the upper 3x3, zeroes the w of the first three columns and writes the translation as `-(R^T t)`; it keeps the source's `w` of the last column. That is the exact inverse only for rotation plus translation. Its one caller, `CreateBillboardMatrix` (`src/prlib/billboard.cpp`), moves the camera into the node's space and keeps only `atan2(x, z)`, so a uniform scale on the node does not change the result, but a non-uniform one does. A port must reproduce the rigid inverse, not substitute a general 4x4 inverse, or billboards on non-uniformly scaled nodes will face a different way.

## Axis-angle rotation (2026-10-05)

`NaMATRIX::RotateMatrix(axis, angle)` (`src/nalib/namatrix.h`) builds the
rotation from the axis's direction in the yz plane (`p`, `q`) and its angle to
the x axis (`a`, `b`); the result is the standard right-handed rotation by `+angle` about the
normalised axis, column-stored like the single-axis overload. The axis length
does not matter and `w` is ignored; an axis along x takes the `radius <
FLT_EPSILON` branch and still gives the right matrix, while a zero axis divides
by zero. The camera roll (`camera.cpp`) and SPA axis-angle keys
(`spadata.cpp`) use it, so a port can use any standard axis-angle routine,
accepting float rounding differences.

Validation: `tests/nalib/rotate_matrix.cpp` compares it with Rodrigues' formula
for seven axes (including both x directions) and four angles, and checks that
`RotateMatrix(0..2, angle)` agrees with the unit-axis form. It passes as a
native C++98 executable and fails when the expected rotation is reversed.

```sh
g++ -std=c++98 -O2 -Wall -Wextra -Werror -Isrc -Iinclude/rtl/ee -Iinclude/rtl/common tests/nalib/rotate_matrix.cpp -o /tmp/rotate-matrix
/tmp/rotate-matrix
```

**Vector helpers (read from the VU code, 2026-10-05).** `NaVECTOR::Cross3`
(`src/nalib/navector.h`) is the ordinary cross product `lhs x rhs` with `w` set
to 0. `Normalize3` sums `x*x + y*y` then adds `z*z`, takes the square root and
multiplies `xyz` by `1 / length`, with `w` set to 0. A zero vector does not give
NaN on the VU: by the VU manual, division by zero returns the largest float
rather than infinity, and `0 * max` is 0, so the result is the zero vector
(inferred from the manual, not run on hardware). The camera code relies on it
in one corner case: `SpcFileHeader::GetCamera` (`camera.cpp`) builds `up` as
`normalize((direction x Y) x direction)`, which is zero when the camera looks
straight up or down, and `sceVu0CameraMatrix` then gets a zero up vector. A port
should make `Normalize3` return zero for a zero input instead of dividing.

## Subtitles (2026-10-05)

`subt.c` draws subtitles only while the subtitle option is on; they never feed
back into play. Scene subtitles (`SubtCtrlPrint`) are a list per subtitle line
of `[starTime, endTime)` entries in frames, converted from the draw line's tick
position with `TICKS_TO_FRAMES`, and the first entry containing the time is
shown. Lyrics under the rhythm bar (`SubtTapPrintWake`, from `MbarTapSubt`)
come from the tap set: text of three or more lines is split into two-line pages
spread evenly over the tap window. `@` (or the full-width Shift-JIS `@`) starts
a new line. Japanese text is stored as EUC and converted to Shift-JIS with the
usual `euc2sjis` arithmetic before the glyph lookup; characters without a glyph
are skipped. Lines are centred on screen, starting at field line 168, or 186
for story-type steps (`PSTEP_SERIAL`, bonus, hook and XTR scenes) and Boxy's
wipe. Fonts are chosen by `SUBT_FONT` (`include/main/subt.h`).
