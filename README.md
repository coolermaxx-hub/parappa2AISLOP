# PaRappa the Rapper 2 Decompilation
![progress](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/coolermaxx-hub/parappa2AISLOP/main/progress/total_progress.json)

<img src=".github/resources/transparent.png" style="margin:7px" align="right" width="20%" alt="PaRappa icon by pips">

A work-in-progress decompilation of [*PaRappa the Rapper 2*](https://en.wikipedia.org/wiki/PaRappa_the_Rapper_2) (パラッパラッパー2) for the PlayStation 2.<br>
We are currently targeting the July 12th NTSC-J prototype, but we aim to target the final NTSC-J/NTSC/PAL builds in the foreseeable future.<br><br>

> **Unofficial AI-assisted fork.** This is not the official project and is not affiliated with parappadev. The official decompilation lives at [parappadev/parappa2](https://github.com/parappadev/parappa2). Please don't take questions about this fork to the upstream maintainers or their Discord servers.

### Fork progress compared to upstream
Measured with objdiff on the July 12th NTSC-J prototype, fork `main` against upstream `main` (45694de). Every number counts only code that builds byte-for-byte identical to the original; functions that still compile from asm, or only have a `NON_MATCHING` C version, do not count. Compiler-emitted helper copies that splat names `func_XXXXXXXX` are paired with their C++ names by `tools/objdiff_symbol_mappings.py` (objdiff still diffs each pair).

| Folder | Upstream functions | Fork functions | Upstream code bytes | Fork code bytes
|--------|-------------------:|---------------:|--------------------:|----------------:
| `dbug` | 21 / 21 (100%) | 21 / 21 (100.0%) | 100% | 100.0%
| `os` | 100 / 100 (100%) | 100 / 100 (100.0%) | 100% | 100.0%
| `iop_mdl` | 4 / 4 (100%) | 4 / 4 (100.0%) | 100% | 100.0%
| `main` | 564 / 570 (98.9%) | 567 / 570 (99.5%) | 95.0% | 96.4%
| `menu` | 308 / 374 (82.4%) | 359 / 374 (96.0%) | 52.0% | 80.9%
| `prlib` | 143 / 360 (39.7%) | 278 / 360 (77.2%) | 21.8% | 43.0%
| **Total** | **1140 / 1429 (79.8%)** | **1329 / 1429 (93.0%)** | **63.3%** | **78.6%**

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

Fork rules and what "100%" means here: [porting-rules](docs/porting-rules.md). Notes on timing, input, audio and RNG for a future port: [behavior-notes](docs/behavior-notes.md).

