# decomp-permuter setup (matching line only)

`setup.sh src/<file>.c <Function>` builds a permuter job for one function that is
still `INCLUDE_ASM` and has a `NON_MATCHING` draft, then prints its starting score.
Run the job with `python3 $PERMUTER/permuter.py <dir> -j4 --stop-on-zero`. Accept a
result only at score 0, then move it into the source and confirm with
`ninja && sha1sum -c config/p3.jul12.checksum.sha1`. C++ functions are not set up:
the permuter parses C only.

`queue.tsv` is the starting score for each of the 16 C targets (2026-10-12), lowest first.

`setup_unit.py` is the better harness: it compiles the whole translation unit for
every candidate, so static helpers inline exactly as in the real build, and
scores against the unit object built with the function still in asm.
`splice.py` puts a candidate's version of the function back into the unit.
