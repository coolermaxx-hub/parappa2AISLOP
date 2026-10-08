# Functions the build no longer emits

The original executable contains 26 functions that our build does not emit.
They are not 26 gaps in the reconstruction. Triage with
`tools/dev/triage_missing.py` splits them into three groups, and only the last
needed new source.

| Group | Count | Nature |
| --- | ---: | --- |
| Unreferenced in the original | 24 | Compiler-emitted copies the linker kept; nothing calls them |
| Inline-emission difference | 1 | Real function, our build inlines it instead of emitting a weak copy |
| Absent body | 1 | Now matched exactly; see below |

Reproduce the table with:

```
python tools/dev/dump_disasm_win.py       # disassembly of iso/SCPS_150.17
python tools/dev/triage_missing.py
python tools/dev/classify_missing.py SYMBOL...
```

`tools/dev/dump_disasm_win.py` exists because `tools/setup.py` downloads the
Linux-only `ccc` binary before dumping asm. The `spimdisasm` call itself is pure
Python, so it runs here unchanged.

## Group 1: unreferenced copies (24)

`tools/dev/classify_missing.py` groups these by exact instruction words, which
shows most are byte-identical clones of one nalib inline emitted into several
translation units:

| Body | Copies | Identity |
| --- | ---: | --- |
| 22 words | `func_0014C4E8`, `func_00153AD0`, `func_0014F3B8`, `func_00149168`, `func_00151DA0` | `NaMATRIX<float,4,4>::Set`, 9 scalars |
| 43 words | `func_0014C540`, `func_00153B28`, `func_0014F410`, `func_001491C0` | `NaMATRIX<float,4,4>::Set`, 16 scalars |
| 10 words | `func_0014B988`, `func_0014B9B0`, `func_00151D78`, `func_00151DF8` | `NaVECTOR<float,4>::Set(x,y,z,w)` |
| 190 words | `func_0014C5F0`, `func_0014F6D8` | `RotateMatrix(NaVECTOR<float,4>, float)` |
| 2 words | `func_00145E50`, `func_00146A08` | `sceGifPkAddGsAD` tail thunk |

Nine of these already carry a verified pairing in
`tools/objdiff_symbol_mappings.py` (`SET16`, `ROTATE_AXIS`, `APPEND_DMA`); the
rest are the same template bodies under addresses, which that file deliberately
leaves unmapped because equal text offsets are not evidence of identity.

The remaining copies: `Set__t8NaMATRIX3Zfi4i4RCfT1T1` and
`Set__t8NaMATRIX3Zfi2i2RCfT1T1T1T1T1T1T1T1T1` (spram, mendererawful) are the
9-scalar `Set` with mangled names, `func_0014D748` is
`NaGifPacketWrapper::AddGsAD(u_int, u_long)` building a packed 64-bit register
word, `func_0014D8D8` is another `sceGifPkAddGsAD` thunk, and `func_00153BD8` is
a 170-word `RotateMatrix(NaVECTOR<float,4>, float)` that differs from the
190-word copies in register allocation only.

No `jal` reaches any of them in the original. `AGENTS.md` forbids creating
address-named or unused helpers to manipulate emitted code, so the correct
resolution is to document them as dead code, not to reproduce them. The honest
end state is that these stay counted as unmatched in `report.json` and are
explained here.

## Group 2: `OpenGifTag__11NaGifPacketUI80` (prlib/depthfield, 8 bytes)

Called once, from `0x0014CAA4`. It is the weak out-of-line copy of the inline
`NaGifPacket::OpenGifTag(u_long128)` in `src/nalib/napacket.h`, which is a tail
call to `sceGifPkOpenGifTag`. Our build inlines all four bytes at the call site
and emits no separate symbol.

Nothing is missing: the behaviour is present. This is an inlining difference, and
it is only recorded because objdiff reports the original's weak copy as absent.
Whether to note it as a known difference or to accept the inlining is a
measurement decision, not a reconstruction one.

## Group 3: `ScaleMatrix__t8NaMATRIX3Zfi4i4RCt8NaVECTOR2Zfi4` (prlib/spadata, 208 bytes)

The one genuine gap, and it is called once, from `GetMatrix__C12SpaTransformf`
at `0x00148824`. It is now reconstructed and matches exactly.

It is not a new overload. The declaration already exists as the static
`NaMATRIX<T,t0,t1>::ScaleMatrix(const NaVECTOR<float,4>&)` in
`src/nalib/namatrix.h`, which returns 64 bytes through a hidden pointer in `$a0`
and takes the vector in `$a1`. The caller passes `$a0 = sp+0x50` and
`$a1 = &v`, and the callee returns `$v0 = $a0`, so the ABI agrees.

The difference is the body. The original builds the matrix in a stack temporary
with two nested loops, writing `scale[i]` only where `row == column` and zero
elsewhere, then transfers it with four `lq`/`sq` pairs (the float4 `Copy`
specialization):

```
.L0014B018   outer loop over 4 rows
.L0014B028     inner loop over 4 columns
                 bnel $t0, $a3, .L0014B038     // skip unless row == column
                 sw   $zero, 0x0($v1)           // off-diagonal
                 lwc1 $f0, 0x0($a2)            // scale[column]
                 swc1 $f0, 0x0($v1)
...
lq/sq x4        Copy into the return slot
```

A preceding 4-iteration loop at `.L0014AFF0` containing only `nop`s is the
compiler's own artifact and has no source-level meaning; do not reproduce it by
hand.

Our `GetMatrix` called the vector overload from the start
(`src/prlib/spadata.cpp:188`), but `ScaleMatrix` built the result by invoking the
16-scalar `NaMATRIX` constructor with sixteen spelled-out constants. That is
small enough to inline at the single call site, so no symbol was emitted.

`ScaleMatrix(const NaVECTOR<float,4>&)` now fills the diagonal in place with the
two nested loops above and returns the result, which is the shape that keeps the
call out of line. `NaMATRIX` also gained the non-const `operator[]` overload it
was missing (`NaVECTOR` has had both all along), because filling the matrix in
place needs a mutable column accessor.

**Verified.** The change compiles, ee-gcc 2.95 keeps the function out of line on
its own, and objdiff reports an exact match at 208 bytes. No pragma or `asm`
barrier was needed to hold it there. The main-ROM checksum is unchanged by this
fix and still fails, as it does on this branch generally.

## What this does not establish

An exact function match is not proof of behavioural equivalence, and neither is a
compilation. The two groups above stay unmatched and documented rather than
reproduced; that is a measurement decision, not a claim that they are resolved.

The counts in this file come from `tools/dev/triage_missing.py`, which reads the
original disassembly directly and so sees all 26 addresses. `progress/report.json`
lists only 17, because objdiff can pair a function by name alone and the 24
address-named clones have no counterpart symbol to pair against. Neither number
contradicts the other; they are different questions.