# THE COMPOSITION RECON (read-only; the spawn-probability stack, mapped)

What composes the scenery: for one (family, tile, moment), EVERY factor that
touches spawn probability, in actual application order, each with its algebra,
its authoring table, its consumer, its units. Worked example: a COLUMN.
METHOD: three parallel scope-tracers (gate core / tile-theme layer / outer
gates + funnels) + hand-verified numbers and an independent `adj_mod`
completeness grep at HEAD (`7786895`). **Nothing moved. STOP for the panel
design.**

---

## §0 HEADLINE

The stack is SHORT and MULTIPLICATIVE — six probability factors, one clamp,
one deterministic roll, then three post-roll vetoes. But it is authored
**three times** (the generic preamble + two bespoke funnels, with real
semantic divergences: clamp presence, veto style, proximity presence), and
its realized probability is NOT its authored probability (the roll happens at
SELECT; placement can still veto at PLACE — and the PopFamily enum order is
silently the placement PRIORITY). Four knobs are identity/dead today, one of
them a **newly found dead write** (`pop.theme_idx`) and one a **per-gate dead
computation** (the preamble's archetype). The theme ENVELOPE — the most
elaborate machinery in the layer — turns out to touch spawn probability NOT
AT ALL: it is the tier axis (which KIND of column), a category boundary the
panel design must respect.

---

## §1 THE STACK — every factor, in actual application order (COLUMN)

The cadence that triggers an attempt (not probability, but the sample rate):
`RPhase::StreamPatches` (per frame) → `stream_patches` → two patch pickers
(priority 7×7 window, patch_system.hpp:782; nearest-first capped at
`SPAWN_BUDGET_PER_FRAME=4`, :903) → `spawn_selected_patches` (:480) → per
patch: `evaluate_theme_envelope` then `select_entities_for_patch`, then the
patch flips ALLOCATED→SPAWNED (:488) — **one attempt per patch lifetime, one
candidate point per patch** (no sub-patch cells in the generic path), and the
roll is `tile_seed(active_seed,gx,gz)`-deterministic: a re-allocated patch
would roll identically. A patch IS the tile (PATCH_EXTENT=50).

| # | factor | algebra | authoring table | consumer | units | COLUMN value |
|---|---|---|---|---|---|---|
| V0 | ROSTER.column | VETO (compile-time fold) | demos/matrix.hpp → roster.hpp:92 | spawn_engine.hpp:655 | bool | enabled |
| V1 | idempotency (family already on patch) | VETO | — (active-array scan) | spawn_engine.hpp:196-202 | — | one column per patch |
| 1 | MOOD_MULTIPLIER[active] | multiply (0 ⇒ de-facto veto) | ColumnConfig entities.hpp:157 `{1,1,1,1,1,0}` | spawn_engine.hpp:205 (adj_mod init) | × | 1.0 @ open; 0.0 @ fin_ref |
| 2 | GLOBAL_ENTITY_DENSITY | multiply | spawn_services.hpp:64 | spawn_engine.hpp:206 | × | **1.0 — IDENTITY** |
| 3 | tile entity_density | multiply | TilePopulation ← DENSITY band × Σ theme density_mult (population_themes.hpp:263,300) | tile_world.hpp:469 (F3 face) | × | **≡1.0 today — both halves dead (§5)** |
| 4 | tile spatial_density[COLUMN] | multiply | Σ over 4 lattice nodes: THEMES[node].spawn_weight[2] × bilinear w (population_themes.hpp:291) | tile_world.hpp:470 (F3 face) | × (1.0 neutral) | 0.5 (BARREN) … 4.0 (COLONNADE) |
| 5 | proximity_affinity_boost | capped-add applied as multiply: min(1+Σaff, MAX_BOOST) after THRESHOLD | PROXIMITY_* spawn_engine.hpp:108-129 | spawn_engine.hpp:214, impl :628-649 | × ≥1 | 1.0 until ≥2 columns within 60 wu; then min(1+0.4n, 2.0) |
| 6 | base SPAWN_CHANCE × adj_mod, clamp | capped multiply: min(·, 1.0) | ColumnConfig::SPAWN_CHANCE entities.hpp:156 | spawn_engine.hpp:615 | probability | base 0.030 |
| R | THE ROLL | VETO (the gate) | — | spawn_engine.hpp:616 `cpu_hash_f(tile_seed(seed,gx,gz), SPAWN_ROLL=700u) < chance` | uniform [0,1] | deterministic per (gx,gz,seed) |
| V2 | slot exhaustion | VETO (post-roll) | Dim::MAX_COLUMN_ONLY=16 state.hpp:144 | spawn_engine.hpp:224-227 | count | — |
| V3 | placement (PLACE phase) | VETO (post-roll) | MIN_SEPARATION row {5,10,8,6,5,5,0…} spawn_services + GAP_REDUCTION 0.3 + MAX_FOOTPRINTS=128 | negotiate_position → check_position spawn_engine.hpp:498-516 | wu | slot freed on failure (entity_pipeline.hpp:557) |
| V4 | host patch gone (COMMIT) | VETO (post-roll) | — | entity_pipeline.hpp:560-562 | — | — |

NOT in the stack (category boundaries, flagged): the theme ENVELOPE
(spike/sustain/decay/cooldown → `temporal_flavor` → TIER weights,
entity_pipeline.hpp:134-136 — which KIND, not WHETHER); indoor rescale
(params) and the finite wall-clamp (position); `allow_*` mood flags (none
gate COLUMN); streaming radius under finite (candidate-set size, not
per-patch probability).

## §2 THE WORKED EXAMPLE — a COLUMN under MOOD_OPEN_DEFAULT

Chance per patch = `min(0.030 × mood × global × ent_d × spat_d × prox, 1.0)`,
then roll `cpu_hash_f(tile_seed, 700u) < chance`.

| scenario | mood | global | ent_d | spat_d[COL] | prox | **chance** |
|---|---|---|---|---|---|---|
| neutral tile, empty world | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 | **0.030** |
| tile ON a COLONNADE node (all 4 nodes COLONNADE) | 1.0 | 1.0 | 1.0 | **4.0** | 1.0 | **0.120** |
| COLONNADE node at w=0.5, other nodes neutral: 4.0·0.5+1.0·0.5 | 1.0 | 1.0 | 1.0 | 2.5 | 1.0 | **0.075** |
| COLONNADE heart + established cluster (2 columns ≤60 wu: 1+0.4·2=1.8) | 1.0 | 1.0 | 1.0 | 4.0 | 1.8 | **0.216** |
| same, boost saturated (cap 2.0) | 1.0 | 1.0 | 1.0 | 4.0 | 2.0 | **0.240** |
| BARREN tile | 1.0 | 1.0 | 1.0 | 0.5 | 1.0 | **0.015** |
| any tile @ MOOD_FINITE_OUTDOOR_REF | **0.0** | — | — | — | — | **0** |

The spatial term is the big authored lever (0.5–4.0, an 8× dynamic range);
proximity adds up to 2× on top but only *after* the cluster exists — the
composition law is "themes seed, clusters compound." THEMES[2] (COLONNADE)
sourcing: spawn_weight[COLUMN]=4.0, density_mult=1.0, envelope
{spike 150, sustain 15, decay 6, cooldown 6}, lattice weight 0.31 — the
envelope numbers shape which TIERS the spawned columns draw (temporal roll
P(COLONNADE)=150/190≈0.79 at full spike vs 10/50=0.20 at base), never how
many columns.

REALIZED vs AUTHORED: chance above is the SELECT-phase roll only. Realized
P(column) = chance × P(slot free: <16 active) × P(placement survives
MIN_SEPARATION vs the footprints registered *before it*) — the last two are
world-state-dependent and unauthored. A rolled-successful column that fails
placement burns nothing permanent (slot freed) but the patch is SPAWNED —
the attempt never repeats.

## §3 THE RE-AUTHORING MAP (the adj_mod sites — exhaustive, grep-verified)

ONE canonical stack + TWO bespoke funnels + ONE shared face; `adj_mod` exists
in exactly 4 files.

- **CANONICAL** — `run_spawn_preamble` spawn_engine.hpp:205-219 + clamp :615.
  TEN families ride it: COLUMN, ANTENNA, PYRAMID, ARCH, BLADE, PALM, CACTUS,
  SPHERE, CUBE, **and RIBBON** (ribbon.hpp:1115 — bespoke payload, generic
  gate; its 0.900 base rides the same chain).
- **SHARED FACE** — `tile_apply_spawn_mult` tile_world.hpp:469-470 (F3): the
  one stanza all three authors call.
- **GoL funnel** — gol_zones.hpp:311-355: mood init + **explicit `<=0`
  early-return** (vs generic multiply-to-zero); ×GLOBAL; F3; NO proximity;
  rolls **per lattice node** (`cpu_lattice_node_seed`, 120-wu spacing — a
  different seed domain than tile_seed); base 0.15; clamp **[0,1] both ends**.
- **Gallery funnel** — gallery.hpp:713-744: content gate (snapshot pool ≥3 —
  unique); mood init + explicit veto; ×GLOBAL; F3; NO proximity; base is
  **archetype-indexed** `GALLERY_CHANCE_BY_ARCHETYPE[4]={0.03,0.06,0.30,0.40}`
  (not a scalar); **NO upper clamp** on base×adj (divergence — a hot tile
  could push chance past 1.0 = certainty); own idempotency + 150-wu
  gallery-distance check + MAX_GALLERIES=48.

The divergence inventory a collapse must reconcile: clamp (min(·,1) generic /
[0,1] GoL / NONE gallery), mood veto style (multiply-through vs
early-return — same outcome, different mechanism), proximity (generic-only;
zero rows for GoL/gallery make omission behavior-neutral TODAY but
structurally divergent), seed domain (tile_seed vs lattice-node seed),
base authority (scalar vs archetype-indexed).

## §4 ORDER MATTERS — AND ISN'T STATED

1. **Roll-before-placement.** The probability roll and slot claim happen at
   SELECT; MIN_SEPARATION vetoes at PLACE. Authored SPAWN_CHANCE is an upper
   bound, not the realized rate. Nowhere stated.
2. **PopFamily order is placement PRIORITY.** `select_entities_for_patch`
   loops f=0..11 and the queue places in push order — within a patch,
   PYRAMID's footprint is registered before ARCH's check, ARCH's before
   COLUMN's… The enum order silently allocates ground. (The F-1 pin now
   freezes this too — a semantic the pin's charter doesn't yet name.)
3. **The clamp position.** min(·,1.0) after ALL multiplies — boosts compound
   before capping; and gallery has no clamp at all (§3).
4. **The tile-cache dependency.** `tile_apply_spawn_mult` is a SILENT no-op
   on cache miss (tile_world.hpp:467-8). Live sequence guarantees
   `ensure_tile` ran at allocation (patch_system.hpp:746/873) before any
   spawn — but nothing states or asserts that ordering; a reorder would
   silently drop the whole theme layer from the stack.
5. **The envelope call SEQUENCE is biography.** `evaluate_theme_envelope`
   mutates global state once per spawned patch (patch_system.hpp:485) —
   which patches spawn first (nearest-first ordering, budget 4/frame,
   player-position-dependent!) determines every tile's temporal_flavor.
   This is the one place the player's PATH writes the (tier) biography.
6. **Mood is read at gate time.** A patch rolled during a transition gets
   that frame's `mood_state_.active` — spawn outcomes are coupled to WHEN
   the patch streamed in, not just where it is.
7. **Float order = bit-identity.** mood × global × ent_d × spat_d × prox, in
   exactly that sequence, then ×base, then clamp. Multiplication is not
   associative in float — any collapse must preserve this exact order (§6).

## §5 DEAD / IDENTITY KNOBS TODAY

| knob | where | state |
|---|---|---|
| GLOBAL_ENTITY_DENSITY = 1.0 | spawn_services.hpp:64; ×3 sites | IDENTITY (wired, live knob) |
| DENSITY noise band (MIN==MAX==1.0) | population_themes.hpp:28-32, block :245-264 | **DEAD** — the whole lattice-hash + pow(·,0.6) + bilinear block is computed and discarded; EXPONENT/SPACING/SEED_BAND affect nothing |
| density_mult (all 5 themes = 1.0) | THEMES rows | IDENTITY ⇒ entity_density ≡ 1.0 for every tile; tile_world.hpp:469 and population_themes.hpp:300 are no-ops today |
| **pop.theme_idx** | population_themes.hpp:299 | **DEAD WRITE (new)** — the spatial dominant-theme is authored and read nowhere; the live theme_idx is the TEMPORAL one (envelope) |
| **SpawnPreamble.archetype** | spawn_engine.hpp:611-612 | **dead computation (new)** — `tile_archetype` runs per generic gate; run_spawn_preamble reads only .passed/.seed; the sole archetype consumer (gallery) calls tile_archetype itself |
| ArchConfig::MOOD_MULTIPLIER {1,1,1,1,1,1} | entities.hpp:66 | all-ones IDENTITY row (every other family vetoes somewhere) |
| COLUMN proximity | tables | DORMANT, not dead (identity until 2 neighbors; then live) |

## §6 THE §12 FRAME + THE GUARD

**The tables stay plural.** MIN_SEPARATION, PROXIMITY_*, THEMES, the mood
rows, the per-family SPAWN_CHANCE constants — the authoring surface is
correct as-is (annotated + F-pinned); nothing here proposes touching a value.

**The STACK is what wants to become one.** The composition law — mood →
global → tile(F3) → [proximity] → base × adj → clamp → roll — is authored
three times with the §3 divergences. The collapse shape (for the panel
design, NOT executed here): one composition function every consumer calls,
taking per-family inputs (base-chance authority, mood row, proximity
on/off, seed domain) as DATA — so gallery's archetype base and GoL's
lattice roll stay authored facts, not re-implementations.

**BIT-IDENTITY GUARD on any eventual cut:** identical float multiplication
ORDER (§4.7), identical clamp placement per consumer (including gallery's
absent clamp — collapsing it means either preserving no-clamp behavior or
ruling the clamp in as a BEHAVIOR CHANGE, gated accordingly), identical
seed domains and hash-call sequence, and the envelope call cadence
untouched (its call sequence is biography). The frozen worlds must not
notice the refactor.

---

STOP — the map is the deliverable. The panel design (which of these knobs
become live authoring surfaces — spatial_density's 8× range and the
proximity caps being the obvious candidates; the dead DENSITY band being a
ready-made density panel waiting for MIN≠MAX) is yours to rule.
