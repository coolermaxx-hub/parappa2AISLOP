# Noodle texture wave model

The two former `coefficients` groups are amplitudes and spatial frequencies.
This is derived from the actual setup block of
`src/prlib/vu1/vump_menderer_create_texture.vsm`, rather than from the default
parameter values. The C++ fields now describe their roles while retaining the
same 80-byte parameter prefix and 160-byte GIF template.

| CPU offset | VU register | C++ field | Meaning |
| ---: | --- | --- | --- |
| 0 | VF30 | `amplitude` | Relative amplitudes of three sine waves |
| 16 | VF29 | `spatialCycles` | Cycles across a 256-texel texture row |
| 32 | VF28 | `temporalFrequency` | Temporal cycles per scaled second |
| 48 | VF27 | `phaseOffsetCycles` | Initial phase offsets in cycles |
| 64 | VF21.xyz | `phaseTime` | Per-wave scaled time, wrapped by CPU at `1/frequency` |
| 76 | VF21.w | `bandOffset` | Band's pixel Y origin: 0, 16, 32, 48 or 64 |
| 80 | VF01..VF10 | `packet` | State and sprite GIF tags/registers |

XYZ contain the three independent waves. W is initialized to zero in each wave
parameter vector; it is not a fourth wave. `CreateMendererTexture` advances time
by `scale * prMendererSpeed / 60`, preserving the game's 60 Hz convention.

## Setup arithmetic

The paired upper/lower VU instructions read the old I value before a new `LOI`
updates it. Following those loads establishes this operation order:

- The amplitude denominator is `(a.y + a.x) + a.z`. VF22.z starts at
  `0.2734375 = 35/128`; `DIV Q` divides that by the denominator. VF30 then
  becomes `amplitude * Q`. The initial VF22.y value of 1 multiplies the final
  amplitude in the sum; it is **not an additional 1 in the denominator**.
- VF29 becomes `(spatialCycles * 6.2831855) * 0.00390625`, the phase increment
  per texel. The program emits four groups of 64 sprites, for 256 samples.
- VF28 becomes `(temporalFrequency * 6.2831855) * phaseTime`, then adds
  `phaseOffsetCycles * 6.2831855`.
- VF25.y is the band's Y origin. Each one-pixel-wide sprite is 16 pixels tall.
  VF26 starts at `(0, 0, 1, 0)`; its S increment is `1/256` and the second
  sprite corner's T increment is 1.

Ignoring VU rounding, the first corner's T coordinate at sample k is:

```
T(k) = (35/128) / (a0 + a1 + a2)
       * sum_j [ aj * sin(2*pi*(fj*tj + oj + k*cj/256)) ]
```

Here `c` is spatial cycles, `f` is temporal frequency, `t` is phase time and `o`
is the phase offset in cycles. The shader reduces phase into the interval
`[-pi/2, 3*pi/2]`, folds using `min(phase, pi - phase)`, and applies `ESIN` to
each component. The folds and three weighted sine evaluations implement the
model above; they are not a new CPU rendering implementation.

## Evidence and limits

`tests/prlib/noodle_texture_model.py` evaluates the **actual straight-line VU
setup instructions** and compares their outputs with the derived equations.
Three cases exercise independent waves, a single active wave and amplitude
normalization. Unsupported instructions fail explicitly. The evaluator respects
paired I loads but uses host arithmetic; it does not emulate VU rounding,
pipeline latency, ESIN or GS output. It intentionally covers only setup.

```sh
python -m unittest discover -s tests/prlib -p noodle_texture_model.py -v
```

The EE layout test verifies all six named offsets above and the 240-byte complete
record. The production rename does not change its arithmetic, parameter order
or upload count. Rendered equivalence, degenerate zero amplitude sums and the
hardware sine approximation remain PS2 validation work.
