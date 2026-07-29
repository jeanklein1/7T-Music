# TERRAIN COUPLING LEDGER (minimal-demo workbench; consolidation, deltas verified at HEAD `950607f`)

Method: the LADDER already holds the design history (the coupling strip, the
two-wave-systems trap, DEMO-2's disposition, P-5) — NOT re-derived. Two
delta-tracers confirmed the FEED POINTS only, file:line each; one tracer
conflation corrected by hand (§1-A note). **Read-only. STOP for rulings.**

---

## §1 THE FEED POINTS AT HEAD (confirmed)

THE BOOT-PIN BLOCK — one address holds every pinned voice input
(`cartridge.hpp:406-411`): `band_motion(inactive[-1]×6, zeros)` ·
`terrain_time(0.0f)` · `mode_color_shift(0.0f)` · `mode_checker_scatter(0.0f)`.
Every setter is live code with exactly one caller — this block. The
could-be authors all exist; nothing feeds them.

### Channel A — THE WAVE VOICE (geometry)

| feed point | file:line | today's author | could-be author |
|---|---|---|---|
| `terrain_time` (config field, unit **t_beats** per its own comment) | state.hpp:376 · WGSL 1416 | **pinned 0.0f** @ cartridge.hpp:410 | `set_terrain_time` state.hpp:2157 |
| WGSL gate + phase | 2450 (`<=0 → return 0`) · 2461 (`t = terrain_time − origin`) | — | — |
| `band_blend_0..5` (per-band gate) | state.hpp:379-384 · WGSL 1419-24 | **pinned −1** (inactive) @ cartridge.hpp:409 | `set_band_motion` state.hpp:2163 |
| WGSL accessor | `get_band_blend` 529 — comment: "DRIVERLESS since gen-1 retirement" | consumers 2456/2502 gated `blend<=0 → continue` | — |
| band ACTIVITY (`band_activity_level` 391 input) | `terrain_activity_at` 398-425 | **authorless** — a static seed-hash spatial field; NO CPU feed exists | would need a NEW signal→activity wire |
| the band moving phase | 505 (`phase_moving = base + t_beats·beat_freq·temporal_freq·2π`) | every caller passes FROZEN time (bake: 0.0 @ 2398; overlay: terrain_time=0) | — |

CORRECTION (over the tracer): world.wgsl:5726's live `signal.t_beats` is the
AGENT step-trigger, not terrain. **No live-beats terrain consumer exists at
HEAD** — the world is still in every path. COHERENCE LAW for the rebuild: the
rendered heightfield BAKES at t=0; the voice must ride the OVERLAY layer
(`contrib_terrain_waves_at` + `terrain_time` + `band_blend`, applied
post-bake per-vertex) — exactly DEMO-2's disposition. Animating the baked
band path would desynchronize bake from live queries.

### Channel B — TERRAIN COLOR

| feed point | file:line | today's author | could-be author |
|---|---|---|---|
| `palette_color_smooth(…, complexity)` mix weight | pinned **0.5** at THREE sites: 5228 (gol composite) · 7186 (cell fields) · 7356 (baked fields) | none — literal | `ground_formed_with_complexity(xz).y` — still COMPUTED every bake (7062) and DISCARDED (the complexity texel was husk-swept) |
| tinted-mono strength | **0.15** literal @ 1333 (`discrete_cell_color`) | none — literal | no uniform behind it (structural tuning constant) |
| mode color coupling | `mode_color_shift` / `mode_checker_scatter` | **pinned 0.0f** @ cartridge.hpp:410-411 | setters exist (same boot block) |
| STRIP CLEANLINESS | — | the coupling strip left NO dangling tint fields (pawn-aura tint is separate + live); `band_blend` is the only written-but-inert survivor | — |

### Channel C — GoL / PULSE CLOCK

| feed point | file:line | today's author | silence AS-WIRED |
|---|---|---|---|
| Conway tick decision (CPU) | gol_zones.hpp:558 `floor(time_state_.beats / period)` → tick_mask 559-561 | **absolute BEATS** = `signal.t_beats` (DAW transport position; authored cartridge.hpp:723, U3) | beats FREEZES → mask=0 → **GoL FREEZES today** |
| CPU→GPU staging | gol_zones.hpp:566 → state.hpp:2624-26 (GoLZoneArray HEADER — not the global signal) | t_beats + dt + tick_mask | frozen values uploaded |
| WGSL Conway consume | 7499 (`tick_mask & (1<<zone)`) | mask only — kernel reads no global time | never ticks |
| WGSL springs (both algos) | 7491 (`dt = zone_config.dt`, SECONDS) | frame delta | springs settle to frozen targets, then rest |
| Pulse phase | 7524 → 5211 (`t_beats · freq …`), × `mode_gol_tick_scale` (5210 — driverless, =1.0 @ state.hpp:5433) | same header t_beats — ONE clock, two consumptions (quantized mask / continuous phase) | sin(phase) freezes |
| the silent-BPM axis (P-5) | spine_state.hpp:37 `beat_rate = 100/60` "HELD-LAST… defaults 100 BPM" | computed (cartridge.hpp:727-730) but **routed into NOTHING the tick reads** | P-5 confirmed as a code fact: the default exists, unowned |

---

## §2 THE MINIMAL COLUMN (one matrix answer)

The minimal column is ALL-FALSE (all 19 tickable rows; pinned by the golden
`none_enabled()` assert, matrix.hpp:152). The LOCKED always-on pieces —
SURFACE, SUN, POINT, PAWN BODY — are not grid rows; terrain streaming is
unconditional (no roster field exists for it).

| channel | observable in minimal AS-IS | needs ticked |
|---|---|---|
| wave voice (geometry) | **YES** — the surface is unconditional; today = static relief (the voice's REST) | nothing |
| terrain color | **YES** — unconditional | nothing |
| GoL tick | no | **Piece::gol** (two gates: family select spawn_engine.hpp:675; phase rows `ROSTER.gol` cartridge.hpp:1425-26) |
| pulses | no | the same **Piece::gol** (the PULSE_ALGORITHM_CHANCE branch inside the family) |

**THE WORKBENCH COLUMN: minimal + gol.** Waves and color need zero ticks —
plain minimal already exhibits them. One tick (gol) lights the clock
channels. Nothing else in the roster touches any terrain coupling.

---

## §3 THE ROW SHAPES (name · author · REST · composition law — the template
the first rebuild instantiates)

**ROW 1 — THE WAVE VOICE**
· author: the U-stage calls the two EXISTING setters from the beat clock —
  `set_terrain_time(t_beats)` + `set_band_motion(blend[6], rates[6])`
  (today's boot pins become the rest values).
· REST: silence ⇒ terrain_time holds 0 ⇒ **today's stillness — ALREADY TRUE.**
  The rest state ships; the voice only ever ADDS.
· composition law: ADDITIVE post-bake height voice — deformation =
  Σ per-band overlay contributions, each gated by its band_blend
  (blend ≤ 0 = band absent; all-absent = bit-identical stillness). Rides
  the OVERLAY layer only (§1-A coherence law). band ACTIVITY stays the
  static seed field unless a second wire is ruled (it is the authorless
  input — a separate decision from the time wire).

**ROW 2 — THE GoL/PULSE CLOCK**
· author: none needed — the channel is WIRED; its driver is the transport
  (absolute beats → tick_mask; pulses ride the same header t_beats).
· REST: **DECLARE THE FREEZE** — silence freezes beats, zones hold
  mid-state, springs settle; this is what the code does today. The
  declaration (comment + LADDER law) RULES the freeze correct and closes
  P-5 for this channel: the 100-BPM held-last default stays unrouted BY
  DESIGN, not by omission. (Ticking in silence would be a behavior change
  — a different ruling.)
· composition law: tick_i = floor(beats / period_i), quantized CPU-side
  into the mask; pulses = continuous phase off the same beats — ONE
  clock, two consumptions; `mode_gol_tick_scale` is the (driverless)
  tempo-warp extension point.

**ROW 3 — TERRAIN COLOR**
· author: candidate A — restore the computed complexity feed
  (`ground_formed_with_complexity(xz).y`, already computed and discarded);
  candidate B — a musical author via the existing mode setters
  (`set_mode_color_shift` / `set_mode_checker_scatter`).
· REST: **JEAN RULES.** 0.5-as-rest keeps today's pixels (bit-identical;
  the voice then MODULATES around 0.5). Computed-complexity-as-rest is the
  "true" terrain reading — but it CHANGES PIXELS everywhere (a behavior
  change, rig-gated, new-worlds class).
· composition law: complexity = MIX WEIGHT (LIGHT→CENTER per palette) — a
  multiplicative blend voice, not additive; the mode setters extend it
  (hue shift / checker scatter), all with living setters and boot pins.

(Pulses carry no separate row — same clock as ROW 2; already ruled a
non-issue for structures.)

---

STOP — the ledger is the deliverable. Rulings held for: which channel
rebuilds FIRST, and its REST (Row 1's rest is already true; Row 2's freeze
wants the declaration; Row 3's rest is a genuine fork).
