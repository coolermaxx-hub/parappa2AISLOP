> Current reconstruction priorities: readable, typed C++ takes precedence over
> exact instruction matching. See [the rules](docs/porting-rules.md) and
> [source reconstruction](docs/source-reconstruction.md). Compiler-generated
> assembly fallbacks are removed from the active build; validation and matching
> remain incomplete. The progress below measures exact matches on that readable
> line; the older ROM-matching line (1339 / 1429 functions) is kept on the
> `main-rom-matching` branch.

# PaRappa the Rapper 2 Decompilation
![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/total_progress.json)

<img src=".github/resources/transparent.png" style="margin:7px" align="right" width="20%" alt="PaRappa icon by pips">

A work-in-progress decompilation of [*PaRappa the Rapper 2*](https://en.wikipedia.org/wiki/PaRappa_the_Rapper_2) (パラッパラッパー2) for the PlayStation 2.<br>
We are currently targeting the July 12th NTSC-J prototype, but we aim to target the final NTSC-J/NTSC/PAL builds in the foreseeable future.<br><br>

> **Unofficial AI-assisted fork.** This is not the official project and is not affiliated with parappadev. The official decompilation lives at [parappadev/parappa2](https://github.com/parappadev/parappa2). Please don't take questions about this fork to the upstream maintainers or their Discord servers.

### Fork progress compared to upstream
Measured with objdiff on the July 12th NTSC-J prototype (2026-10-05), fork `main` against upstream `main` (45694de). The fork trades some exact matches for readable source, so a few functions that matched in assembly-shaped C now build slightly differently; each one is listed with its cause in [binary-differences](docs/binary-differences.md). Every number counts only code that builds byte-for-byte identical to the original; functions that still compile from asm, or only have a `NON_MATCHING` C version, do not count. Compiler-emitted helper copies that splat names `func_XXXXXXXX` are paired with their C++ names by `tools/objdiff_symbol_mappings.py` (objdiff still diffs each pair).

| Folder | Upstream functions | Fork functions | Upstream code bytes | Fork code bytes
|--------|-------------------:|---------------:|--------------------:|----------------:
| `dbug` | 21 / 21 (100%) | 21 / 21 (100.0%) | 100% | 100.0%
| `os` | 100 / 100 (100%) | 100 / 100 (100.0%) | 100% | 100.0%
| `iop_mdl` | 4 / 4 (100%) | 4 / 4 (100.0%) | 100% | 100.0%
| `main` | 564 / 570 (98.9%) | 563 / 570 (98.8%) | 95.0% | 93.0%
| `menu` | 308 / 374 (82.4%) | 355 / 374 (94.9%) | 52.0% | 82.1%
| `prlib` | 143 / 360 (39.7%) | 268 / 360 (74.4%) | 21.8% | 34.5%
| **Total** | **1140 / 1429 (79.8%)** | **1311 / 1429 (91.7%)** | **63.3%** | **75.7%**

### Progress
*Badges below show this fork's matched-function percentage. The upstream project's own numbers are on its [decomp(dot)dev page](https://decomp.dev/parappadev/parappa2).*

#### EE Core
| Folder | Progress | Description
|--------|----------|------------
| `dbug` | ![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/dbug_progress.json) | Debug utilities (VRAM save, debug menus, etc.)
| `os` | ![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/os_progress.json) | OS utilities (threading, pad, memory, etc.)
| `iop_mdl` | ![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/iop_mdl_progress.json) | IOP module control routines
| `main` | ![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/main_progress.json) | Gameplay code (score logic, loading screen, etc.)
| `menu` | ![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/menu_progress.json) | Menu code (UI flow, Memory Card saving, etc.)
| `prlib` | ![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/prlib_progress.json) | Game engine (rendering, models/animations, etc.)
| `src` | ![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/total_progress.json) | Total percentage

#### IOP modules
| Folder | Progress | Description
|--------|----------|------------
| `wavep2` | ![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/wp2cd.total_progress.json) | BGM and asset streaming
| `tapctrl` | N/A | Voice and sound effect playback

### Contributing

Please see our [build](docs/build.md) and [decompilation](docs/decompilation.md) guides.

Fork rules and what "100%" means here: [porting-rules](docs/porting-rules.md). Notes on timing, input, audio and RNG for a future port: [behavior-notes](docs/behavior-notes.md). What is left against that definition: [remaining-work](docs/remaining-work.md).

