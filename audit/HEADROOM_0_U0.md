# HEADROOM_0 — U0 RECON

U1 is landed (`69bcaac`). **U2 and U3 are not built**, and both for
reasons U0 was written to find. This is the report the handoff asks for
before either is built.

HEAD at recon: `c10dea8`. Read-only except U1's own commit.

## U0.1 — the envelope's site: found, and it needed no plumbing

`cartridge.hpp`, the mapped callback, the loop that folds
`frame_ms[self->meter_.snap_pairs[p].row] += ms`. Both extremes are one
comparison each inside a loop that already reads every timestamp, so the
envelope cost one `if` per pair and no new state beyond a `RowStat`.

Landed in U1. Nothing further owed here.

## U0.2 — the DPR parameter path: **there is none**

The handoff says report the mechanism before building it, and the stop
condition says build the smallest one if none exists. The report first,
because it is worse than "none":

```
src/console/console.hpp:90    inline constexpr float MAX_DEVICE_PIXEL_RATIO = 1.5f;
src/console/console.hpp:1075  if (ratio <= MAX_DEVICE_PIXEL_RATIO) return;
src/console/console.hpp:1076  fbWidth  = (int)((float)winW * MAX_DEVICE_PIXEL_RATIO);
src/console/console.hpp:1077  fbHeight = (int)((float)winH * MAX_DEVICE_PIXEL_RATIO);
```

- **No URL-parameter mechanism exists anywhere in the tree.** A grep for
  `URLSearchParams`, `location.search`, `emscripten_run_script` and
  `getenv` across `src/` returns nothing. There is no preset system
  reading runtime input; the "presets" are CMake/compile-time
  (`THE_BOARD_DEMO`, `INSTRUMENTS`).
- **The cap is `inline constexpr`.** U2 cannot plumb a value into it; it
  must first become a runtime `inline float` (or a function returning
  one), which changes three consumer sites from constant-folded to
  loaded.

So U2 is two changes, not one: **de-constexpr the cap**, then **build the
smallest possible reader for it**. The smallest reader on the web twin is
one `EM_ASM_DOUBLE` (or `emscripten_run_script_int`) parsing
`location.search` once at init, clamped to a sane range, with the native
twin keeping the compile-time default. That is roughly fifteen lines in
`console.hpp` and touches no consumer's arithmetic.

**Not built, because the handoff asked for the mechanism to be reported
first and this is the report.** Say the word and it is one commit; the
shape above is what I would write.

## U0.3 — the census repair: **a static_assert blocks the handoff's first option**

The handoff offers two routes and asks which is smaller. Neither is
available as written, and the reason is a guard the tree already carries.

### The dial the handoff wants already exists — and it is load-bearing

`phase_census_dumps` is already gated:

```
if constexpr (!INSTRUMENTS.periodic_census) return;
```

But `core/instruments.hpp` carries this:

```
static_assert(!INSTRUMENTS.frame_meter || INSTRUMENTS.periodic_census,
    "INSTRUMENTS: frame_meter on with periodic_census off — the [METER] "
    "table prints on the census cadence, so the meter would accumulate "
    "every frame and report nothing");
```

**The [METER] table prints inside `phase_census_dumps`** (the
`"[METER] window %uf …"` block sits below the gate, in the same
function). So "gate the periodic dump off by default under the meter
preset" would turn off **the meter itself** — the instrument the soak
exists to read. The assert is right, and it is what makes option one
impossible rather than merely unwise.

### The offender is narrower than "the dump"

`census_dumps max 1178.39 / 957.51 / 713.54` is the whole phase, which
now contains two different things sharing one cadence and one dial:

| rider | what it costs | does the soak need it? |
|---|---|---|
| the entity `[CENSUS]` text | ~50 blocking `std::cout` lines | **no** |
| the `[METER]` window table | the numbers the soak reads | **yes** |

So the smaller edit is **neither of the two offered**. It is to split the
dial: keep one cadence, and let the entity dump be silenced
independently of the meter table. Concretely, one new `Instruments`
bool — `census_entity_dump` — gating only the entity block, with the
existing `periodic_census` continuing to gate the cadence and the meter
table, and the `static_assert` left exactly as it is.

Why this over buffering: buffering the text and flushing "outside the
frame" moves the same blocking write to the same thread a few
microseconds later. It reduces N writes to one, which is real but is a
constant factor on a cost the soak does not need to pay at all. Silence
is cheaper than any amount of buffering, and it is a smaller edit.

**Not built**, because it adds a field to the `Instruments` contract and
the handoff's stop condition says report which route and why before
choosing. This is that report; the route above is the recommendation.

## STATE

| unit | status |
|---|---|
| U1 — the envelope | **landed**, `69bcaac`, master |
| U2 — the DPR knob | **not built** — no parameter path exists and the cap is `constexpr`; shape reported above |
| U3 — census gating | **not built** — the handoff's first option is blocked by a `static_assert`; a third route is recommended above |

Nothing was compiled. `glaw1` is the witness for U1, and the soak cannot
begin until it builds.
