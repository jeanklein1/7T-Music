# STAGE_0 — BUILD REPORT

**Branch:** `master`. **Base:** `85b602bb`. Tree clean at start, clone not
shallow. **No unit held, no STOP fired.** **Working name; naming is Jean's gate.**

One law: *everything computed is visible.*

---

## UNIT TABLE

| Unit | Subject | Status |
|---|---|---|
| U0 | recon | **DONE** — six dimensions, exhaustive; five findings below came out of it |
| U1 | the pin | **DONE** — `WORLD_RADIUS_PIN = 2u`, seated rather than substituted (§1) |
| U2 | the veil excision | **DONE** — both rooms; the spine is 15 render rows |
| U5 | the push cut | **DONE** — run BEFORE U4, deliberately (§3) |
| U4 | the kite cut | **DONE** — stride 208 → 192 |
| U3 | the permanent choir | **DONE** — and it would have been monochrome (§2) |
| U6 | gates + the record | **DONE** — every runnable gate green; `--record` PREPARED AND HELD |

Order run: U1 → U2 → U5 → U4 → U3 → U6. §3's chain, with U5 and U4 swapped for
the reason in §3 below.

---

## 1 · U1 — WHY THE PIN IS SEATED, NOT SUBSTITUTED

§U1 retires `derive_finite_radius`. Doing that **turns a gate red**, and the
handoff's §0(d) treatment does not save it:

1. It is the **declared reader** of `WORLD_LIVE.radius_min` / `.radius_max` in
   `tools/organ_readers.py`. Retire it and two ENROLLED rows have no reader, so
   `organ_ledger --check` fails — under a freeze that forbids retiring the rows.
2. It is the **last fence** before the radius becomes a buffer index. Its own
   comment says so, and three `static_asserts` bind the capacity it clamps.

So the pin is seated where the tree already built a door: `WORLD_LIVE`'s range is
pinned shut at boot and the draw's own `if (lo >= hi) return lo;` returns it
without reaching the hash. That arm exists and explains itself — *"a pinned range
is still a legal range, and it is how a hand asks for one size without a second
mechanism."* This campaign is that hand.

**The ground is provably unperturbed.** Every seeded draw in this tree is an
independent address-keyed hash — `cpu_hash(seed, prop)`, `tile_seed(master, gx,
gz)` — never a sequential stream (grep: zero hits for `mt19937`, `std::rand`,
`default_random_engine`, any `rng_state`). The radius drew at property `77u`,
used by nothing else. Retiring the draw cannot shift a draw anywhere.

**The dials stay live on purpose**: widen the range at the panel and the radius
is drawn again, which is how the pin is *inspected* rather than trusted. The boot
witness says which it got — `[PINNED]` or `[drawn]`.

---

## 2 · U3 — THE CHOIR WOULD HAVE BEEN MONOCHROME

The single most valuable thing recon produced.

`choir_slot_seed` recomputes each cube's base colour as
`tile_seed(world_seed, ac.patch_gx, ac.patch_gz)` — from the **mirror's** patch
coordinates, on every poke. A boot-born cube belongs to no patch, so those sit at
their `0,0` default and **all twenty-four keys hash the same seed**. The whole
instrument comes out one flat colour. Nothing asserts it. It would simply look
wrong, on a device, after the campaign shipped.

`CHOIR_PATCH_ROW = -30000` is the fix: one distinct synthetic pair per key, on a
row no real patch grid reaches. **Counted, not assumed:**

| | distinct `tile_seed` over 24 keys |
|---|---|
| naive birth (0,0 default) | **1** |
| with `CHOIR_PATCH_ROW`, world seed 12345 | **24** |
| with `CHOIR_PATCH_ROW`, world seed 999 | **24** |

**Two seeds, and the second is not ours to choose.** The BODY seed
(`cpu_hash(CHOIR_SEED, k)`) draws radius, bob, aspects, face variance —
world-independent, as ruled. The COLOUR seed must be the projector's, because
`choir_slot_seed` is on the choir light's protect list and will recompute it
regardless; so the birth dresses `inst.colors` **through that same seed**, or the
first poke would repaint every cube.

**What this leaves, named:** the BODIES are world-independent and the PALETTE is
not — the same instrument wears a different finish in each world. That reads
well. If Jean wants the finish fixed too it is one line (`CHOIR_SEED` for the
world seed at `choir_slot_seed`'s call site), but the call site is protected and
the choice is his.

---

## 3 · WHY U5 RAN BEFORE U4

The plasticity leak was gated `if (fe.follow_pawn == 0u)` — anchor mode only,
because in kite mode the anchor was dormant. **That gate is where U4 and U5
collide.** Cutting the kite first makes the leak unconditional for the first
time, live over push-produced drift, in a mode it was written never to run in.
Running U5 first retired the leak while its gate still meant what it said.

**And the leak's retirement is what makes WHEEL_0 sound.** That block WROTE
`fe.target_x/target_z` every frame. WHEEL_0 serves stations through the
glide-target door *"gated on the station having moved past epsilon"* — a CPU
memory compared against the target the GPU holds — and this was the kernel
quietly moving that target underneath it. With the leak gone the target is the
CPU's alone. Recorded at the site, because a later campaign would have found it
only on a device.

---

## 4 · U2 — THE PREMISE WAS WRONG AND THE CONCLUSION SURVIVED IT

§0(b): *"At a pinned radius 2 the world spans 250 wu per side — every point sees
every other."*

| radius | box span | corner-to-corner | vs ring 342 |
|---|---|---|---|
| 1 | 150 wu | 212.1 | never fires |
| **2 (the pin)** | **250 wu** | **353.6** | **FIRES — by 11.6 wu** |
| 3 | 350 wu | 495.0 | fires |
| 4 | 450 wu | 636.4 | fires |

The ring was **not** dead at the pin. It fired in an 11.6 wu corner sliver — and
under the stage law that sliver should draw. The excision is right for a reason
the handoff did not give: not *"it never fires"* but *"what it still cut, it
should not have."* The record says so where the block used to be.

Two things that cut the same way: **the shadow pass never had a ring gate**, so
at r ≥ 2 the world already carried shadows with no visible caster — the excision
fixes that inconsistency. And **the CPU half was already dead**:
`update_entity_draw_visibility` has had an empty body and no caller since
ONE_SURFACE-I U6, while remaining the sole reader of `draw_ring()`.

---

## 5 · TWO DEFECTS ALREADY IN THE TREE

**A DECLARATION/DEFINITION MISMATCH NO GATE COULD SEE.** `agents.hpp` declared
`respawn_evicted_agents` with **four** parameters and defined it with **six**.
Different overloads — so the declaration named a function never defined and never
called. Legal C++; invisible to the TU gate (type-checks, does not link), the
shell gate (never links this path) and the probe (never reaches it). Anyone
calling the four-argument form would have got a link error at the end of a build.

**THE CHAIN'S OWN PROSE WAS ITS LEAST ACCURATE TEXT.** The `Dim` veil block said
RING was **325** (the constant beside it read **342**) and that floaters evicted
at **400** (the WGSL const has been **800** since `309ab754`, and three comments
in two rooms still said 400). It also described the icing as live; the icing has
been 100% epitaph prose since ONE_SURFACE-I U4 — no `icing` identifier exists in
either room.

---

## 6 · THE FOLD §U2 ASKED FOR, AND WHY IT IS NOT DONE

*"`render_patch_count` folds into `active_patch_count` (one set now — every
resident patch draws)."* True of the RING. **False of the PHASE:**
`band_patches` skips any patch whose phase is not `GENERATED`, so the two counts
differ while a world is still building.

Folding them would either overwrite the allocation count with a partial
generation count, or hand the draw patches with no layer yet. **Two counts, two
facts, both kept** — and the field's comment, which blamed the ring and was
already false before this campaign, now says what actually separates them.

---

## 7 · THE STRIDE MOVED — THE ONE THING ONLY THE PROBE CAN ANSWER

`GPUFloatingEntityState`: **208 → 192**. The two kite fields are exactly 16
bytes, so the struct lands on 12×16 with no padding added or removed in either
room, and every field below drops by exactly 16.

**§U4's justification was wrong** — there is no "16-aligned hole at 176";
`pawn_offset` fully occupied 176..187 and the cut opens no hole either. The real
fact is better. **The real hazard is worse:** this struct carries **no
BYTE-FOR-BYTE marker in `world.wgsl`**, so `mirror_offsets.py`'s per-field
witness — 128 members across 7 structs — **does not cover it**. Three
`static_asserts` in `state.hpp` are the whole static proof, and one was ADDED
here to pin `behavior_phase` into the slot the kite vacated.

This is the campaign that most needs `--probe=N`.

---

## 8 · THE RECORD RITUAL — PREPARED AND HELD

`glaw2` is **GREEN without a re-record**: 258 fn, 253 const, **22 symbols retired
cleanly** (18 before). `--record` was run against a copy and **reverted
byte-for-byte**. It would change:

| | |
|---|---|
| **my four** | `AGENT_EVICTION_RADIUS(_SQ)`, `FLOATER_EVICTION_RADIUS(_SQ)` |
| **seventeen the baseline never dropped** | the whole `MOSAIC_*` set, `veil_t`, `seg_closest`, `signal_active`, `render_point_pos` |
| **six the baseline never gained** | `AUTO_CELL_RETRACT`, `AUTO_MODE_THRESHOLD`, `agent_ground_resolve`, `gol_carve_fade`, `sample_cell_retract`, `ug_cell_center` |

Re-recording now would silently bless **two campaigns' worth of unrecorded
drift** as baseline. That is exactly why RETRACT_0 R10 calls it Jean-gated and
destructive. **The re-record is Jean's word.**

---

## 9 · GATES

| gate | verdict |
|---|---|
| TU gate | **PASS** — both tiers, zero diagnostics |
| G-LAW 2 | **GREEN** — no re-record; the diff is prepared and held (§8) |
| WGSL gate | **PASS** — naga parses, scopes and validates every edit |
| score census | **GREEN** — 7 update + **15** render rows |
| shell gate | **PASS** |
| binding surface | **PASS** — every relation and witness, S-6 once pushed |
| organ gap / ledger | **PASS** — the freeze holds; two rows stand dark |
| mirror census / offsets | **GREEN** / **PASS** |
| G-LAW 1 · `--probe=N` | **JEAN'S** |

`meter_row`'s pins caught the enum shift the instant `RespawnAgents` left index
2 — the third time that shape has run (PortalTrigger, GolDeriveFlush), and
exactly what those `static_asserts` are for.

---

## 10 · FOR JEAN — THE DESK

1. **`the-board --probe=120` first**, and this campaign more than any: a stride
   change on the shared floater buffer is what only a device refuses (§7).
2. **The boot log tells you the stage law held**: `[World] Born FINITE radius=2
   (5x5 patches) [PINNED]` and `[CHOIR] born: 24 keys, 2 rank(s), seed 0x7c0119e5`.
3. **Look at the edge.** The veil is gone; the board's wall is the picture now
   (ruling 4). If it reads wrong, that is the ruling to revisit, not the code.
4. **The Monolith is at key 18** — rank 1's tritone, the far side of the wheel.
   One index to move.
5. **The choir's palette follows the world seed; its bodies do not** (§2). One
   line if you want the finish fixed too.
6. **The kite and the push are gone.** Nothing follows you any more. That is the
   commission, and it is the biggest felt change in the session.
7. **The held `--record`** (§8) — yours.
