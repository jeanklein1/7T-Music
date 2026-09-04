# CHOIR_0 — BUILD REPORT

**Branch:** `master`. **Base:** `be0eb28f` (SKIRT_WELD_1). Tree clean at start
(one exception, named in §7). **Rounds:** one. **No unit held, no unit
quarantined; no STOP fired.** The name `CHOIR_0` is a working name — **naming
is Jean's gate.**

**Boot preflight ran.** The clone arrived SHALLOW; `git fetch --unshallow
origin` was the first command of the session, before any claim about history.
2376 commits, `master` and the harness branch at the same tip.

---

## UNIT TABLE

| Unit | Subject | Status |
|---|---|---|
| U0 | recon, no edits | **DONE** — 5 items, 4 confirmed exactly, 1 confirmed with a counting correction (§1) |
| U1 | the choir cap | **DONE** — `CUBE_CHOIR_N = 36`, three static_asserts, traits re-aimed |
| U2 | the ear and the envelope | **DONE** — one ear, the §1 law verbatim, the ears excised |
| U3 | the pipe and the drivers' room | **DONE** — **one divergence: base 15, not 16** (§7 D1) |
| U4 | the seam and the successor projector | **DONE** — one author, one projector home, poke-on-change |
| U5 | the excision | **DONE** — every retired name tombstoned in the diff |
| U6 | gates + ledgers + the record | **DONE** — every runnable row green; the probe is Jean's |

**Commits (3):** `f746c66e` U1+U2+U3+U4 · `1b6cc757` U5 · this one U6.
U2→U3→U4 is one dependency chain by the handoff's own §4, and splitting it
would have produced a commit whose new pipe had no reader — so the chain is
one commit and the excision is its own, which is what "the successor must
stand before the ancestor dies" asks for in commit form.

---

## 1 · U0 — EVERY ITEM

**Item 1 — `ch6.present_count` resolves. CONFIRMED.**
`canvas_1::canvas.hpp` `NAME_PRESENT_COUNT[6]` is the literal string
`"ch6.present_count"`; `publish_reading(Reading::PresentCount,
Source::channel(v), NAME_PRESENT_COUNT[v])` runs for every voice in the
per-voice loop; `shape_of(Reading::PresentCount)` returns
`{ SLOT_PRESENT_COUNT = 0, 12, StatShape::Vector }`. **Zero analysis edits
were needed and zero were made.**

**Item 2 — the `zoetrope_cell_intensity` census. CONFIRMED, with the count
corrected from five to four.** The four CALL SITES were: the reveal's
from-screen shadow seed (538), the newborn's splay in `cube_write_gpu` (771),
`project_cell_color`'s mix (956) and `zoetrope_project_slot`'s swell+splay
(1002). The handoff's fifth — "the projector, dim ~975" — is the `dim`
computation *inside* `project_cell_color`, not a separate call. **The SET is
complete either way**, which is what the excision and re-aim sets depended
on; only the tally moves. All four were re-aimed and no fifth existed.

**Item 3 — `upload_cube_color` / `upload_cube_face_variance` callers outside
the two files. CONFIRMED: none.** Both are called from exactly two lines,
both inside `cube_behaviors.hpp` (`zoetrope_project_slot`), and both
definitions live in `state.hpp`. Not even `cartridge.hpp` called them.

**Item 4 — the two sizeof-assert sites. CONFIRMED, one each, in their own
headers.** `canvas_surface.hpp:83` and `driver_surface.hpp:73`. Both moved
with their structs (Amendment D).

**Item 5 — unguarded `LATTICE_CELLS` loops. CONFIRMED: none.** Ten loops
walk it. Six guard on `activeCubes_[i].active` (the reveal's stage, the
projector flush, the seat pass, the climb, the hand-back, `clear_cubes`'s
GPU wipe walks `MAX_CUBE_INSTANCES` instead); the other four walked `cells`,
`cell_scratch` and `wdir` — arrays that are gone. Nothing reads a slot
≥ `CUBE_CHOIR_N` after the cap without an `active` test in front of it, and
`choir_light` adds a second, unconditional guard on top of that.

---

## 2 · WHAT LANDED — THE LAW, AS IT COMPILES

```cpp
// key k = rank k/12 of RAW pitch class k%12; key k IS slot k
const float count = signal.stat(choir_ear_.channel,
                                choir_ear_.base + dressed_of_pc(k % 12));
const bool  on    = (count > (float)(k / 12));      // doubling lights the next rank
float I = choir_I_[k];
if (on) I += (1.0f - I) * rise;                     // rise = 1 - exp(-dbe/tau), tau = plateau/4
else    I  = (I > fall) ? I - fall : 0.0f;          // fall = dbe / light_release
```

**THE DRESSING PAIR IS ASSERTED, NOT ASSUMED.** The ears read raw pc
`(i + 2) % 12` from dressed index `i`. The keyboard needs that inverted, so
`dressed_of_pc(p) = (p + 10) % 12` — and the two are checked against each
other at compile time rather than by eye:

```cpp
static_assert(dressed_of_pc(2) == 0, "D is the dressed origin");
static_assert(dressed_of_pc((0 + 2) % 12) == 0
           && dressed_of_pc((7 + 2) % 12) == 7,
    "dressed_of_pc must invert the ears' (i + 2) % 12 fold");
```

**THE NUMBERS, COUNTED RATHER THAN ESTIMATED** (120 BPM / 60 fps, Δ = 1/30
beat, plateau 8, release 8):

| quantity | value |
|---|---|
| τ | `plateau / 4` = 2 beats |
| I at the plateau | `1 − e⁻⁴` = **0.98168** |
| per-frame rise coefficient | `1 − e^(−1/60)` = 0.016529 |
| attack frames that poke (ε = 1e-3) | **199 of 240** — the gate closes at I ≈ 0.939 |
| per-frame release step | `1/240` = 0.004167 — 4× the gate |
| release frames that poke | **240 of 240** |
| worst-case pokes per frame | **36** (whole choir falling at once) |
| pokes per frame in silence | **0** |

The old flush spent one ≤252-slot sweep every 0.25 beats *unconditionally*
whenever a tick ran, plus an immediate poke per struck cell. The new one is
cheaper in every state and free in silence.

---

## 3 · THE SILENT PATH IS STILL BIT-EXACT — AND WHERE IT ISN'T, ON PURPOSE

At `I = 0` the colour mix is `br + (lc − br)·0 = br`, and with `dim = 1`
(every state but the two SCREEN ones) `br` is `cube_compute_colors`'s own
output on the reconstructed spawn seed — **the seed colour, to the bit**.
The variance is `draw · (1 − 0) = draw` — **the mirror's bare tier draw, to
the bit**. Both fall out of the law rather than out of a branch, which is
why no restore pass exists.

**THE ONE DELIBERATE DIFFERENCE, AND IT IS VISIBLE.** The old variance was
`draw · ZOETROPE_FACE_REST + ZOETROPE_FACE_SPLAY · I` with `FACE_REST =
1.20`, so a cube standing dark **in a formation** wore 1.2× its spawn draw
(and in ROAM the old projector returned before the variance poke at all, so
it wore whatever birth gave it). The new law is `draw · (1 − I)` in every
state, so a dark cube wears 1.0× everywhere. This is the commission's own
law and `FACE_REST` explicitly retires with it — but it is a percept change
**on a standing screen with the music silent**, not only on a lit one, and
it is on the visual gate list. Restoring the boost would cost the
self-restoring property, so it was not quietly kept.

---

## 4 · THE TWO WIDTHS, AND WHERE THEY MEET

The canvas may not name a cartridge symbol (it lives one tier below), so the
pipe's width and the population's cap are two constants:

- `t7::CHOIR_LANES = 36` — the canvas's, the `cube.light` run's width;
- `t7::the_board::CUBE_CHOIR_N = 36` — the cartridge's, the living ceiling.

They are reconciled in the **one room that can legally see both** —
`phase_motion_drivers`:

```cpp
static_assert((int)CUBE_CHOIR_N <= CHOIR_LANES,
    "the choir has more keys than the cube.light pipe has lanes");
```

A pipe *wider* than the choir costs only rest-valued lanes, so **flipping
the choir to two ranks stays the single token the commission asked for**:
`CUBE_CHOIR_N = 24`, nothing else. The canvas additionally clamps its own
loop to `min(choir_target_.count, CHOIR_LANES)`, so a narrower pipe is a
narrower keyboard rather than an overrun.

---

## 5 · THE POKE GATE AND "THE SAME DARK KEY RELIGHTS"

Key = slot holds at the SLOT level by `run_spawn_preamble`'s lowest-free
reservation. It has to hold at the GPU level too, and that took two lines
the handoff did not name:

- **birth seeds the gate.** `cube_write_gpu` writes the whole slot including
  the projected colour and variance, so it records `choir_flushed[slot] =
  choir_I[slot]`. Without it a refilled slot could inherit its predecessor's
  shadow, sit inside ε of a truth it was never actually written at, and
  never poke — **the one way a dark key could fail to relight.**
- **`clear_cubes` resets the gate.** A portal wipes every GPU slot, so every
  shadow it holds becomes a lie about an empty slot. The choir's LIGHT is
  *not* reset there: it is the music's state, not the world's, and it is
  re-mirrored from the live signal every frame regardless. `clear_cubes`
  therefore loses its cells-are-not-reset banner (the ghost law's subject is
  gone) and gains a one-line gate reset.

`repaint_all` — the two formation edges that move the dim — rides through as
a **force** that outranks the gate, because it changes what a cube looks
like *without moving its light*.

---

## 6 · GATES

Run from the repo root at the final tree. **CC's environment carries Dawn's
headers, not a built Dawn, and no display**, so the probe row is Jean's.

| gate | verdict |
|---|---|
| TU gate (`console_gate`) | **PASS** — both tiers, CARTRIDGE and CONSOLE, zero diagnostics |
| G-LAW 2 | **GREEN** — 258 fn, 257 const, 55 struct, 55 binding, 34 entry points; **18 symbols retired cleanly, unchanged** |
| shell gate | **PASS** — 5 scenes, the scripted session, the export→import round trip |
| score census | **GREEN** — 7 update + 16 render rows, bijection both directions |
| WGSL gate | **PASS** — and it is a genuine no-op: `world.wgsl` is **byte-identical**. *This row did not run at baseline* — `naga` was absent from the container; `cargo install naga-cli` was run so the row could actually answer instead of failing open. |
| binding surface (`--check`) | **PASS — every relation, every witness**, S-6 included: `commit integrity: working tree clean; HEAD 17cdc81 == pushed tip`. It was red for the whole campaign and red at baseline too, for one reason S-6 states in its own verdict: the clone arrived with no upstream tracking ref, so there was no pushed tip to be equal to. It went green the moment the work was pushed, which is what that witness is for. **S-8 unchanged: 11 fixed WGSL extents checked.** |
| organ gap (`--gate`) | **PASS** — 0 surviving runtime readers across 14 graduated pairs |
| organ ledger (`--check`) | **PASS** — NO SUSPECTS |
| mirror census | **GREEN** |
| mirror offsets (`--check`) | **PASS** — 128 members, 7 structs |
| G-LAW 1 | **JEAN'S** — needs the pinned emdawnwebgpu surface build |
| **`--probe=N`** | **JEAN'S — THE DEVICE GATE. No visual sign-off before it.** |

**Every gate CC can run is green, S-6 included.** Two rows are Jean's, and one
of them is the only row that runs on a device.

**THE L33 REBUILD WITNESS WAS RUN, NOT ASSERTED.** The five `audit/` files and
`mirror_offsets.gen.inc` were deleted, the six tools re-run, and the result
diffed against the pre-delete copies: **byte-identical, all six**.

**THE RECORD RITUAL — `glaw2 --record` deliberately NOT run,** on
SKIRT_WELD_1 §6's precedent. Every name this campaign retires is **C++**,
which glaw2 does not see; no WGSL entry point, const or struct moved, and
glaw2 is GREEN without a re-record. `--record` rewrites the whole baseline
including the `declared` retirement ledger (RETRACT_0 R10: Jean-gated and
destructive). **The ritual's actual requirement is met**: every retired name
is claimed by a tombstone in the diff.

---

## 7 · DIVERGENCE LOG

**D1 — THE PIPE'S BASE IS 15, NOT 16 (U3).** The handoff's §U3 says
`{ "cube.light", 16, 36, 0.0f }` and, in the same sentence, **"base 16, the
first free run"** and **"the bank goes 15/256 → 51/256 allocated"**. Those
three cannot all be true: the terrain run ends at slot 14, so the first free
slot is **15**, and `15 + 36 = 51` is exactly the stated total — base 16
would total 52 and leave slot 15 an unexplained hole in a register map whose
banner says slots are laid by hand so there are no collisions. Two of the
three signals say 15 and they are the two that agree with each other, so the
pipe sits at 15. **Trivially reversible** if the 16 was deliberate: one
number in `PARAM_LAYOUT`; the cartridge resolves by name.

**D2 — `slot_of_cell` EXCISED (U5), a judgment call inside U5's remit.**
The handoff's protect list keeps "`LATTICE_ROWS/COLS/CELLS`, the helix pair
and their asserts". `zoetrope_strike` was `slot_of_cell`'s **only** caller —
a lattice cell needed the map read backwards to name the slot it pokes — so
taking the strike whole takes its reader, and an inline function with no
reader is a corpse by the tree's own law. **Both CONSTANTS stay and both
static_asserts stay**, so the bijection is still proved; only the runtime
walk in the inverse direction is gone. `cell_of_slot` (the seating
direction) is untouched and still has two readers. **Restorable in three
lines** if Jean reads the protect list as covering the function too.

**D3 — `zoetrope_service`'s SIGNATURE SHED TWO PARAMETERS (U5).** With the
tick and the flush gone, `active_seed` and `t_beats` had no reader left
inside it; keeping them would have been two dead parameters at a live seam.
It takes `dt`, `point_x`, `point_z`. The function keeps its **name** as the
handoff directs — the zoetrope's *body* is the formation machine, and only
its *substrate* was the lattice.

**D4 — `<cstdio>` AND `core/instruments.hpp` LEFT `cube_behaviors.hpp`
(U5).** Their only reader in that file was the `[ZOETROPE]` strike witness.
The witness moved to the canvas with the envelope it reports, so the dial
(`INSTRUMENTS.zoetrope_witness`, **name unchanged — the rename stays
parked**) is read one tier down and the file needs neither include.

**D6 — THE `choir_light()` ACCESSOR WAS BUILT, THEN REMOVED.** U2 asks for
"`float choir_I_[36]` + `accessor const float* choir_light() const` — the
`zoetrope_rows()` accessor pattern". It was written, and an adversarial pass
over the finished diff caught it as an **orphan**: nothing calls it.
`zoetrope_rows()` had a caller because the strike took the run as an
ARGUMENT, so the cartridge had to reach the canvas directly; the choir's run
must be composed against the drivers' room before anything sees it, so it
leaves through the BANK and a second door onto the same floats can have no
caller by construction. **A one-line accessor with no reader is precisely
the corpse U5 spent itself removing**, and `slot_of_cell` went for the same
reason two units earlier — keeping one and cutting the other would have been
the inconsistency. The tombstone stands where it did. **One line to
restore** if Jean wants the door held open for a future direct reader.

**D7 — ONE COMMENT EDITED ON THE ANALYSIS SIDE, against §0(a)'s "ZERO
EDITS".** `canvas_1`'s `[ONSET]` witness justified itself as one half of a
pair: *"The board's `[ZOETROPE]` strike line can only say 'rows were
empty'… this one sits on the publish side of that line, so the two together
bound the fault."* U5 deleted the line it names. §0(a)'s rule is about the
analysis MECHANISM — "no `publish_reading`, no writer case, nothing" — and
**zero mechanism moved**: no reading, no slot, no writer, no publication,
not one executable token. What moved is a comment that had become false
about the tree, in the same class as the two documents repaired in §9. The
new text records that the coupling half of the pair is gone, that no
coupling reads `Reading::Onset` today, and why the witness still earns its
place. **Named here rather than done quietly**, because the handoff's word
was "zero".

**D5 — TWO ENVELOPE DEGENERATES TAKEN AT THEIR LIMITS.** A zero
`light_plateau` is τ → 0, which the law itself answers with an instant climb
to 1 (the naive arithmetic would divide by zero, or with a `> 0` guard
would freeze the key dark — the wrong limit). A zero `light_release` is an
infinite slope, so an instant fall. A frozen clock (Δ = 0) moves neither,
which is the loop seam's rule at zero length. Neither is reachable today —
the rows are parked, so no organ range exists to reach them — but the law
should not depend on that.

---

## 8 · THE THREE FLAGS THAT ARE NOT DIVERGENCES

**F1 — THE FACE VARIANCE'S REST MOVED.** §3 above. Jean's visual desk.

**F2 — TWO DIALS PARKED, NOT ENROLLED**, per §0(e) and the ORGAN_REST
freeze. `CANVAS_LIVE.light_plateau` / `.light_release` (15 → 17 floats) and
`DRIVER_LIVE.cube.light_color` / `.gain` (18 → 22 words) carry design values
and **no `organ_params.inc` row**. Four lines when the freeze lifts:

```
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, light_plateau, F32, 0.25f, 32.0f, 0.25f, "Cubes · Choir envelope", "attack plateau (beats)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, light_release, F32, 0.25f, 32.0f, 0.25f, "Cubes · Choir envelope", "release (beats)")
ORGAN_PARAM(DRIVERS, DriverSurface, cube.light_color, VEC3, 0.0f, 1.0f, 0.01f, "Cubes · Choir", "incandescence")
ORGAN_PARAM(DRIVERS, DriverSurface, cube.gain,        F32,  0.0f, 1.0f, 0.01f, "Cubes · Choir", "drive gain")
```

Both `organ_gap --gate` and `organ_ledger --check` are PASS with them
unenrolled: an unenrolled field is not a suspect, it is simply not a dial
yet. The **struct order** matters more than the rows do, and it is stated in
both sizeof asserts.

**F3 — THE SPARSE SCREEN IS THE CAP'S INTENDED CONSEQUENCE.** 36 cubes
seated through the helix across a 7×36 geometry is a far emptier screen than
252 was. The formation stations are untouched and still seat 0..N−1
correctly; the screen simply has three ranks' worth of pixels lit out of
seven rows' worth of seats. Accepted percept, on the visual gate.

---

## 8b · THE DIFF WAS REVIEWED ADVERSARIALLY, AND IT FOUND THINGS

The finished diff was put through a six-dimension review — the envelope law,
the population cap, the projector and its flush, the excision, the contracts
and tooling, the cross-tier seam — with every finding then handed to three
independent verifiers instructed to REFUTE it. **Ten findings landed and all
ten are fixed. One of them was a real behavioural regression, and it is the
single most valuable thing this pass produced:**

### A1 · THE SWELL WAS NOT RE-ASSERTED AT THE SETTLE — a real bug, now fixed

**The failure.** The climb ends by snapping `body_radius` to the BARE
`ZOETROPE_PIXEL_RADIUS` and flipping the formation to `SCREEN`. The swell
lives in the projector, and the projector is gated on the light MOVING. So a
cube arriving on the screen **under a held chord** — light steady at the
plateau, not moving — would be skipped by the gate and stand at the bare
pixel radius, unswollen, until its key next changed. **The lattice hid this
class entirely**: its flush ran unconditionally on every 0.25-beat tick, so
the swell landed within a quarter beat whatever the gate thought.
Poke-on-change has no such backstop, and that is exactly the kind of thing a
cheaper flush buys you if nobody looks.

**The fix.** The arrival is a repaint edge, so it declares itself: the settle
raises `repaint_all`, and the next frame's `choir_project` forces one full
pass — colour, variance and swell — at the new formation. Same mechanism the
dim's two edges already used, one frame of latency where the old code had up
to a quarter beat.

**The same class, one step over.** A cube BORN into a standing screen took
the bare pixel too, and its gate is seeded at birth so nothing would poke it
either. `cube_write_gpu` now swells the newborn when the screen STANDS — and
only then, because under `TO_SCREEN` the walk owns the radius and the bare
pixel is the target it is carrying every other cube toward.

### The other nine

| finding | disposition |
|---|---|
| the poke gate compares against the last FLUSHED value, not the last frame — so it THINS rather than stops, and the comment implied stopping | **fixed** — the comment states the accumulate-against-the-shadow behaviour and the counted tail (≈17 more pokes over the next ten plateaus) |
| the traffic tally omitted the swell's `body_radius` write, which fires on every poke in the one state the instrument is played in | **fixed** — 3 writes, 720 B/frame worst case, stated |
| `choir_light` advertises itself as I's one door, and `choir_project` read `choir_I` directly past it | **fixed** — it reads through the door |
| `state.hpp`'s FIELD_2 subscriber comment still named 252 as the cube living ceiling | **fixed** — it names the capacity as capacity, and `CUBE_CHOIR_N` as what moved |
| the inversion `static_assert` proved one point twice (`dressed_of_pc(2)` and `dressed_of_pc((0+2)%12)` are the same claim) and left ten lanes unchecked | **fixed** — a `constexpr` walk over all twelve |
| `VisualCanvas::choir_light()` had no caller anywhere | **fixed** — removed, D6 |
| "logarithmic" is wrong for `1 − e^(−t/τ)`, in three comments | **fixed** — the commission's word is kept and the actual curve named beside it, with the asymptote that is the real difference |
| `INSTRUMENTS.zoetrope_witness`'s comment described the deleted `[ZOETROPE]` line | **fixed** |
| `canvas_1`'s `[ONSET]` witness cited that same deleted line as its partner | **fixed**, D7 |

Every counted claim in §2 was re-derived numerically rather than estimated —
the first draft of the poke-gate comment said "about six sevenths of the way
up, roughly two thirds through the plateau", and the count says **I ≈ 0.939
at frame 199 of 240**.

---

## 9 · WHAT MADE ME HESITATE

**The `[Zoetrope] ears bound: 7 of 7` line is quoted as EVIDENCE in two
documents.** `COUPLING_ATLAS.md` §0(a) uses it to prove there are no
`SignalLayout` misses, and `OPEN.md`'s LIGATURE_1 entry records it as the
post-splice state. Deleting the line without touching them would have left
two documents citing a boot log the program can no longer print. Both are
updated in place — the *argument* survives (the misses count is still 0 and
its silence still means the same thing), only the witness's name changed.

**The atlas's §1 counts are load-bearing and appear in four places.** "55
published, 12 heard, 43 unheard" is quoted in the atlas twice, in its §5
orphan table, and in `OPEN.md`'s handover index. The choir hands seven names
back and takes one, so it is **6 heard, 49 unheard** now, and all four sites
are corrected together. A count that is right in one paragraph and wrong in
the next is worse than a count nobody wrote down.

**I nearly wrote that the excision "removed 15 automaton dials" in the
atlas.** It would have collided with §4/§6's `AUTO_LIVE` — the **ground's**
automaton, a different machine that is still standing. Caught and rewritten
before the commit; the two automata are now named apart wherever the atlas
mentions either.

**The commit split.** The handoff's §4 makes U2→U3→U4 one chain, so the
"one commit per logical unit" contract and "the successor stands before the
ancestor dies" pull against each other: a U2+U3 commit would ship a pipe
with no reader. The chain is one commit; the excision is its own.

---

## 10 · FILES TOUCHED

| file | what |
|---|---|
| `src/cartridges/the_board/bodies/cube_behaviors.hpp` | the choir band + cap; the traits re-aim; `choir_I` / `choir_flushed`; the successor projector (4 functions); the four re-aims; the excision + every tombstone |
| `src/coupling/visual_canvas.hpp` | `CHOIR_VOICE` / `CHOIR_LANES` / `dressed_of_pc` + its twelve-term round trip; the `cube.light` pipe row; the one ear; the envelope law; the ears excised |
| `src/coupling/canvas_surface.hpp` | `light_plateau`, `light_release`; sizeof 15 → 17 |
| `src/cartridges/the_board/contracts/driver_surface.hpp` | the `Cube` block + its `DRIVER_TABLE` row; sizeof 18 → 22 |
| `src/cartridges/the_board/cartridge.hpp` | the `cube.light` binding + boot witness; the seam (mirror + `choir_project`); the strike call retired; `zoetrope_service`'s call re-shaped |
| `docs/COUPLING_ATLAS.md` | §0(a) witness, §1 counts + the source table, §2 the ninth pipe, **§3 EARS → THE CHOIR**, §5, §6 |
| `docs/OPEN.md` | the CHOIR_0 entry; the campaign ledger row; three stale citations repaired |
| `docs/CHOIR_0_REPORT.md` | this |
| `src/core/instruments.hpp` | one comment: the dial gates `[CHOIR]` + `[ONSET]` now, not the deleted `[ZOETROPE]` line |
| `src/cartridges/the_board/realization/state.hpp` | one comment: FIELD_2's subscriber cap names the capacity as capacity (§8b) |
| `src/analysis/canvas_1/canvas.hpp` | **one comment only — see D7.** Zero mechanism |
| `audit/BINDING_LEDGER.md`, `COMMAND_LEDGER.md`, `MIRROR_LEDGER.md` | regenerated (provenance stamps only — **no structural row moved**) |

**`world.wgsl` is byte-identical. `MANIFEST.md`, `binding_surface.gen.inc`,
`binding_registry.hpp`, `limits_floor.gen.inc`, `features_wallet.gen.inc`,
`mirror_offsets.gen.inc` and `audit/ORGAN.md` are byte-identical** — the
generators were re-run and wrote no change, which is the ledgers proving
that no shader, no binding and no organ row moved.

**Two ledgers were STALE before this campaign began**, and regenerating them
fixed pre-existing drift rather than recording mine: `BINDING_LEDGER.md` and
`MIRROR_LEDGER.md` still carried `HEM_1`/`c8b856dc` provenance at
`be0eb28f`. Noted so the delta is not read as this campaign's.

**AND HERE IS WHY THEY GO STALE — a property of the tooling, not an
accident.** Each of these ledgers stamps *"the last commit touching any file
I scan"*. `BINDING_LEDGER` and `MIRROR_LEDGER` both scan `state.hpp`. So
regenerating them **in the same commit that edits a scanned file** stamps
them ONE COMMIT BEHIND by construction — the commit they belong to does not
exist yet when the tool reads the log. The only stamp that can be right is
one written by a FOLLOW-UP commit. This campaign hit it twice (U6 and U6b
each stamped their predecessor) and closed it with U6d; the two ledgers that
arrived stale at `be0eb28f` are the same mechanism, unclosed.

`mirror_census.py` makes it easy to hit, because **it is a gate and a
generator in one file**: running it to check a verdict rewrites its artifact
as a side effect. Running the battery after a commit therefore dirties the
tree. Both facts are worth a successor's attention — **the fix is to
regenerate last and commit the ledgers alone**, which is what U6d is.

The witness that this is only a stamp and not a structural drift: the second
full regeneration produced the identical file list, so nothing oscillates,
and the **L33 rebuild witness was re-run at the settled state and is
byte-identical across all six artifacts.**

---

## 11 · FOR JEAN — THE VISUAL GATE

1. **`the-board --probe=120` first.** No visual sign-off before PROBE GREEN.
2. **The light colour triple** — `DRIVER_LIVE.cube.light_color` is
   `{1.0, 0.92, 0.72}`, warm incandescent against seed-cool bodies. It is
   your desk's; one line in `driver_surface.hpp`.
3. **The attack's shape at τ = 2.** Fast at switch-on by construction, and
   ≈ 98% at 8 beats. If it reads too eager, `light_plateau` is the knob —
   raising it lengthens the climb *and* softens the first frames, because
   τ is derived from it.
4. **The variance convergence** — a lit cube converging to one flat face,
   and a dark cube at 1.0× its draw rather than the old 1.2× in formation
   (§3). This is the one place the silent picture moved.
5. **The sparse screen** at 36 of 252 seats (§8 F3). If it reads too thin,
   the answer is *not* to raise the cap past a whole number of pianos —
   it is `CUBE_CHOIR_N = 24` in the other direction plus a screen re-shape,
   or a new ruling on the seating.
6. **The name.** `CHOIR_0` is a working name.
