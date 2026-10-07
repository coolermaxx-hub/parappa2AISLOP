# Stage archives (.INT)

Each stage step loads one `.INT` archive from `\DATA` or `\DATAS`
(`src/main/stdat.c`). `CdctrlMemIntgDecode` (`src/main/cdctrl.c`) reads it as a
run of sections:

| Offset | Field | Meaning |
| --- | --- | --- |
| 0x00 | `id` | `0x44332211` (`PACKINT_MAGIC`) |
| 0x04 | `fnum` | Files in the section |
| 0x08 | `ftype` | `FILE_TYPE_ENUM`: 1 textures to VRAM, 2 sounds, 3 data kept in memory, 4-7 PaRappa's hat textures for rounds 1-4; anything else ends the archive |
| 0x0c | `head_size` | This header plus `fnum + 1` offsets into the decoded data, padded |
| 0x10 | `name_size` | Name block size |
| 0x14 | `data_size` | Compressed data size, padded to the CD sector |
| 0x20 | `adr[]` | Start of each file in the decoded data (16-byte aligned), then the end |

The name block holds an `(name offset, exact size)` pair per file followed by
the NUL-terminated names. The data is one LZSS stream (`PackIntDecode`: a
4096-byte ring, 3-18 byte matches, a u32 decoded size and one skipped word
first).

`tools/dev/audit/int_archive.py <file.int> [out-dir]` lists or extracts the
files.

## The \DATAS cutscene archives (checked 2026-10-07)

`ST00SR0`, `ST01SR0` and `ST01SR1` each hold the subtitle fonts for one
cutscene step and nothing else:

- a 4-bit kanji font split into 32-bit TIM2 pieces whose names give their VRAM
  block (`img08960.tm0` is TBP 0x2300, the `SUBT_FONT_KANJI` base in
  `subt.c`), with its 16-bit CLUT at 0x2500 (`img09472.tm0`);
- the ASCII font at 0x3f6d with its 32-bit CLUT at 0x3fed (`img16237`,
  `img16365`), shared with `menufont.c`;
- one `.cod` file, the `MCODE_STR` glyph table that `codeKanjiCheck` searches:
  a count, then 12-byte `MCODE_KANJI` entries `{code, u, v, w, h, adjx,
  adjy}`, the code being the Shift-JIS bytes in text order. The tables hold 98,
  72 and 130 glyphs and fill their files exactly.

Loading the TIM2 pieces into the native GS memory model and reading the fonts
back through `SubtGsTex0` (`tools/native/tim2_vram.c`) gives legible fonts,
and cutting each `MCODE_KANJI` rectangle out gives the expected characters
(the first two codes, 0x8141 and 0x8149, are 、 and ！). No models are in
these archives; stage models would be in the `\DATA\ST0xGM0` (game) and
`ST0xHK0` (hook practice) archives.
