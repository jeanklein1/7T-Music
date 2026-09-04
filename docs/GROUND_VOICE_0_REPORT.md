# GROUND_VOICE_0 — BUILD REPORT

**Branch:** `master`. **Base:** `25508ff3` (CHOIR_1 U4). Tree clean at start,
clone not shallow. **Rounds:** one. **No unit held, no STOP fired.**
**Working name; naming is Jean's gate.**

`world.wgsl` is **byte-identical**. Nothing was enrolled. No name retired.
The automaton's rule and its full-torus step are untouched.

---

## UNIT TABLE

| Unit | Subject | Status |
|---|---|---|
| U1 | the pipes and the ear | **DONE** — two source pipes, `all.field` reused, `all.current_pc` newly heard |
| U2 | the drivers' room and the seam | **DONE** — `Ground{0.8, 0.15}`, clamps in the automaton's room, a NEW per-frame seam |
| U2b | *(not in the handoff)* the tick scale reaches the step | **DONE** — without it the tick half was inert |
| U3 | gates + the record | **DONE** — every runnable gate green; probe + desk are Jean's |

---

## 1 · THE FINDING THAT MATTERS — THE TICK HALF WAS NOT PLUMBED

§0 opens: *"The plumbing already exists end to end."* For the HEIGHT half that
is true. **For the tick half it was not, and the campaign would have shipped a
knob that does nothing.**

`config.mode_gol_tick_scale` had exactly two readers in the whole tree, and
**both are inside `world.wgsl`'s `pulse_cell_target`** — the PULSE field's
per-cell target:

```wgsl
              / max(tick_period * config.mode_gol_tick_scale, 0.01);   // SPIRAL
let freq = tempo_jitter / max(tick_period * config.mode_gol_tick_scale, 0.1);  // BREATH
```

**The bank boots CONWAY**, and `automaton_surface.hpp`'s own static_assert says
so in as many words (`AUTO_TABLE.algorithm == CONWAY && rule_mask == 0x1808`).
The Conway branch of `automaton_evolve` gates on `should_tick` — and
`should_tick` is not a WGSL fact at all. It is computed **on the CPU**, in
`upload_automaton_header` (`surface/automaton.hpp`):

```cpp
const float effective_period = std::max(as.tick_period, 0.01f);
const int32_t current_tick = (int32_t)std::floor(c->time_state_.beats / effective_period);
const bool should_tick = (current_tick != as.last_tick_index);
```

No scale anywhere in it. So on the ground this program actually runs, the tick
dial reached **nothing**.

**THE FIX IS ONE LINE**, at that gate, in the same shape the two dormant WGSL
readers already use — the scale multiplies the PERIOD, so smaller is faster.
**The rule and the full-torus step are untouched**: this changes *when* a step
fires, never *what* a step does, which is exactly what §2's protect list means
by "scales modulate; the rule is untouched".

**WHY IT IS WELL-BEHAVED, AND THE CAVEAT.** The gate is a `floor` against a
stored index, so a period varying *continuously* would make the index wander
and fire steps on frames that are not tick boundaries. This scale's driver is
the room's polyphony — an **integer** count — so the period holds still between
note changes and moves once when the music does. A note change costs at most
one extra step, which is the coupling's intent rather than a glitch in it.
**A continuous driver aimed at this field would want a phase accumulator
instead of a floor**, and that is new mechanism: flagged, not built.

**Was this scope growth?** No. The commission's own sentence is "the GoL cells'
heights *and speed of update* connect to musical parameters", and §2 protects
"the automaton's rule and its full-torus step (**scales modulate**; the rule is
untouched)" — which anticipates precisely this. Delivering half a commission
and calling the other half plumbed would have been the scope failure.

---

## 2 · TWO MORE THINGS THE HANDOFF HAD WRONG

**THE ARGUMENT ORDER IS (tick, height).** §U2 writes
`set_mode_gol_scales(height_mul, tick_mul)`. The signature is:

```cpp
void set_mode_gol_scales(float tick_scale, float height_scale)
```

Both parameters are `float`, both rest at exactly `1.0`, and both are legal in
either slot. A swap **compiles, links, and passes every one of the ten text
gates** — it is visible only on the device, as a ground that gets taller when
it should get faster. This is the failure class `CLAUDE.md`'s own gate banner
names: *"a number that is legal C++, legal WGSL and wrong ACROSS the two
passes all of them."* Written `(tick_mul, height_mul)`, with the order stated
at the call site.

**`cartridge.hpp:582` IS A BOOT PIN, NOT A SEAM.** §U2 says the composition
happens at "the same one call site". That line sits inside
`Cartridge::initialize`, in the block that also pins the terrain rows, the
pulse ring, the fly speed and the tilt tau — a **rest-pin block that runs
once**. A coupling cannot live there. The boot pin stays as the rest (exactly
the fog's and the checker's shape — both also have a boot rest AND a per-frame
seam), and the coupling is a new block in `phase_motion_drivers` beside them.

---

## 3 · GAIN 0 IS HANDS OFF, PER TERM — AND IT HAD TO BE

This is the design decision I would most want reviewed.

Every other driven config value in this tree follows one shape:

| | the driven value | its rest | its gain |
|---|---|---|---|
| fog | `config.fog_density` — **`ORGAN_PARAM_RO`**, "(driven)" | `sky_state_.fog_rest_*`, from `ATMOS_LIVE` | `DRIVERS.fog.gain` |
| checker | `config.checker_*` — **`ORGAN_PARAM_RO`**, "(driven)" | `DRIVERS.checker.rest_*` | `DRIVERS.checker.gain` |
| aura, pawn figure | `ORGAN_PARAM_RO`, "(driven)" | elsewhere | elsewhere |

**The two GoL scales break that shape**: they are `ORGAN_PARAM` — *writable*
dials — and their rest is **the field itself**, boot-pinned to 1.0. Nothing
drove them, so the dial was the only author.

A seam that wrote unconditionally would overwrite that dial every frame and
kill it silently: Jean turns "tick scale" in the panel, and the next frame puts
it back. So **at gain 0 each term passes the dial's own value straight back**.
The setter's inequality gate then sees no change and does not dirty, and the
dial is the author again. That is what `// 0 manual … 1 coupling verbatim` has
said in the drivers' room since it was written; here it is literally true.

It also means **the registry freeze holds with zero organ edits**. At
freeze-lift these two want the fog's three-row shape — an `_RO` meter for the
driven value, a rest with a home of its own, and the gain — which is a bigger
question than a row kind, because their rest is currently the field.

Paste-ready, for when the freeze lifts:

```
ORGAN_PARAM(DRIVERS, DriverSurface, ground.height_gain, F32, 0.0f, 2.0f, 0.01f, "Terrain · Ground voice", "lift gain")
ORGAN_PARAM(DRIVERS, DriverSurface, ground.tick_gain,   F32, 0.0f, 1.0f, 0.005f, "Terrain · Ground voice", "quicken gain")
```

---

## 4 · WHY THE PIPES CARRY SOURCES

Nine of the eleven pipes carry a decoded, **target-shaped** value and the seam
composes it linearly: `rest + gain·(driven − rest)`. The ground's two carry
**sources**, and they are the only two that do.

The reason is the law's own algebra. §0 puts the gain **inside** the
expression:

```
height_mul = clamp(1 + height_gain · field,   0.25, 4.0)
tick_mul   = clamp(1 / (1 + tick_gain · dens), 0.25, 4.0)
```

Splitting that canvas-side would force the linear form — `1 + g·(pipe − 1)` —
and **that changes what the gain means**. At the authored `tick_gain = 0.15`
the linear form could only ever shorten the period by 15%, however dense the
music; the ruled form reaches the clamp on a dense chord. The law and its desk
numbers were authored together, so the law stays whole at the seam.

**Rest 0 is the fog's convention, not a compromise.** The `1 +` in the law is
what makes silence a multiplier of exactly one, so no rest constant has to
promise it — and silence is today's ground **bit-exactly**, which is §0's own
headline.

**The clamps live in the automaton's room** (`contracts/automaton_surface.hpp`,
`GROUND_SCALE_MIN/MAX`), not the drivers'. They say what this *ground* will
accept, not how loudly a coupling speaks, so a different coupling aimed at the
same two fields inherits them. The floor is the load-bearing half and it is the
tick's: the period is a divisor, so a scale near zero is a ground that boils.

---

## 5 · FLAGS

**F1 — `field` IS NOT EMA'd, AND THE GROUND WILL JUMP.** §0 dismisses the
smoothing question because "`field` is already EMA'd analysis-side". It is
not: `canvas_1` publishes `Reading::Field` as `field_index(p.field)`, a
**discrete held election**. The fog reads the same source and answers exactly
that — it carries the table lookup on a `Segment` over `fog_span`, "so density
and color drift across a modulation instead of snapping", in its own band's
words. Without one, **the whole ground changes height in a single frame when
the field elects**. Built as ruled and flagged rather than fixed: a ground that
jumps on a modulation may be the percept, and that is a desk question, not a
correctness one. The one-idiom fix is one line per pipe and is written out at
the site.

**F2 — THE HEIGHT GAIN SATURATES OVER HALF ITS RANGE.**

| field | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|---|
| `1 + 0.8·field` | 1.0 | 1.8 | 2.6 | 3.4 | 4.2 | 5.0 | 5.8 |
| after the 4.0 ceiling | 1.0 | 1.8 | 2.6 | 3.4 | **4.0** | **4.0** | **4.0** |

Fields 4, 5 and 6 are one height. That may be right — a loud room is a loud
room — but it is a choice. **`0.5` is the gain that maps the full field range
onto the full clamp**, field 6 landing on 4.0 exactly.

`tick_gain 0.15` wants no note: voices 0..12 give period × 1.0 / 0.87 / 0.77 /
0.69 / 0.63 / 0.53 / 0.45 / 0.36 — monotone and well clear of the floor.

**F3 — CONFIG'S "GPU-SIDE" SCOPE REASON IS NOW INCOMPLETE.**
`tools/organ_readers.py` excludes the CONFIG family with the reason *"GPU-side:
`config_` is uploaded whole and read in world.wgsl"*. Since U2b,
`mode_gol_tick_scale` is *also* read CPU-side, at the step gate. The exclusion
still stands and no gate moves; the reason no longer covers every field it
names.

**F4 — THE TWO PULSE FLOORS ARE NOT THE SAME NUMBER.** `pulse_cell_target`'s
SPIRAL branch guards `max(tick_period * scale, 0.01)` and its BREATH branch
three lines below guards `max(…, 0.1)` — a factor of ten apart, in one
function, on one field. Both are dormant on a Conway ground. Named because a
future PULSE campaign will trip over it, not because it is this campaign's.

---

## 6 · GATES

| gate | verdict |
|---|---|
| TU gate (`console_gate`) | **PASS** — both tiers, zero diagnostics |
| G-LAW 2 | **GREEN** — 18 symbols retired cleanly, unchanged. C++ values only; no record |
| shell gate | **PASS** |
| score census | **GREEN** |
| WGSL gate | **PASS**, and a genuine no-op: `world.wgsl` is **byte-identical** — the tick fix is CPU-side |
| binding surface (`--check`) | **PASS** — every relation and witness, S-6 included once pushed |
| organ gap (`--gate`) | **PASS** — nothing enrolled; the freeze holds |
| organ ledger (`--check`) | **PASS** — NO SUSPECTS |
| mirror census / offsets | **GREEN** / **PASS** |
| G-LAW 1 · `--probe=N` | **JEAN'S.** No visual sign-off before PROBE GREEN |

`tools/organ_readers.py` gains `bind_signal_layout` as a DRIVERS reader — the
boot witness prints the two gains, and **the table's own census flagged it as
a `?` line the moment it appeared**, which is that mechanism working. The eight
remaining `?` lines are byte-identical to baseline.

---

## 7 · FOR JEAN — THE DESK

1. **`the-board --probe=120` first.**
2. **The lift at a loud passage** (`height_gain 0.8`) — and F2: fields 4–6 are
   currently one height. `0.5` is the gain that spreads them.
3. **The tick under dense chords** (`tick_gain 0.15`). It works at all only
   because of U2b; before it the knob was inert.
4. **Both clamps** — `GROUND_SCALE_MIN/MAX` = 0.25 / 4.0, in the automaton's
   room.
5. **Should the ground JUMP or DRIFT on a modulation?** (F1.) It jumps today.
   One line per pipe makes it drift, the fog's own idiom.
6. **Turn the two `mode_gol_*_scale` panel dials with the gains at 0** — they
   should work exactly as they did before this campaign. That is §3's whole
   claim and it is the cheapest thing on this list to check.
7. **The name.** `GROUND_VOICE_0` is a working name, and so is the GoL-colour
   ruling's placement in `OPEN.md`.
