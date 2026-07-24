# AUDIT-4 — THE LEVER AUDIT

**Base:** master `cd6c0f42ffe4ef6c6728495c66fd24f04b0c339f` (the post-CONTACT_4
tree — CONTACT_1→4 closed). Read-only audit. **No mainline edits. No proposals
implemented — findings only.**

**Audit branch:** `claude/sync-handoffs-review-5vd7j6` (cut from base; carries
this report only — no behavior change).
**Probe branch:** `audit4/probes-throwaway` (cut at base, ran in an isolated
scratchpad workspace, discarded — the sketches never touched mainline; [A4-7]).

**Adapter (all runtime witnessing):** google / SwiftShader (Tint). Limits
maxStorageBuffersPerShaderStage **10**, maxUniformBuffersPerShaderStage **12**,
maxStorageTexturesPerShaderStage **4** — matches the world.wgsl FXC banner.

**Per-section verdict at a glance:**

| § | Section | Verdict |
|---|---|---|
| A4-1 | Baseline pin | **PASS** (tree identical to `_post_c4`; all machine gates green) |
| A4-2 | Lever census | **PASS** (25 mechanisms mapped; shapes assigned) |
| A4-3 | Duplication map | **FINDING** (4 flee copies drift; 3 more DRY clusters) |
| A4-4 | Sorting verdict | **PROPOSAL-SURFACE** (2 trilogy levers are fields wearing pairwise clothes) |
| A4-5 | Control-surface census | **FINDING** (6 rooms; 2 mirror-without-gate hazards; the reference-scale gap) |
| A4-6 | Contract contradictions | **FINDING** (1 confirmed declared-graceful / implemented-fatal) |
| A4-7 | Probes | **PASS** (both compile/validate; a format-portability finding falls out) |

---

## [A4-1] BASELINE PIN

Re-ran every instrument at base `cd6c0f4` and diffed against the committed
`_post_c4` artifacts.

| Instrument | Method | Result vs `_post_c4` |
|---|---|---|
| cc6 (layout budgets) | `cc6_layout_budgets.py state.hpp binding_registry.hpp` | **IDENTICAL** (byte-diff empty) |
| cc7 (WGSL binding census) | `cc7_wgsl_binding_census.py world.wgsl` | **IDENTICAL** (96 declarations) |
| cc7 mirror cross | swap `cc7_output_post_c4.json` in, run, restore | **IDENTICAL** (0 orphans both directions) |
| cc4 (static usage) | `cc4_wgsl_static_usage.py world.wgsl` | **IDENTICAL** |
| Dawn witness | `probe_dawn_witness_post_c4.mjs` (SwiftShader) | **ALL PIPELINE FAMILIES GREEN**, 0 module messages, every family `ok:true` — structural parity; only per-pipeline SwiftShader timing differs (non-structural), so the committed `probe_results_post_c4.json` was restored to keep the tree clean |

The tree has not moved since CONTACT_4 closeout. The pin holds.

### Gate outcomes (checked table; UNKNOWN where unreported)

| Gate | Owner | Outcome |
|---|---|---|
| glaw1 (`sh audit/tools/glaw1/run.sh`) | machine | **GREEN** (re-run this audit) |
| Dawn witness (all families) | machine | **GREEN** (re-run this audit) |
| cc6 / cc7 / cc7-mirror / cc4 vs `_post_c4` | machine | **IDENTICAL** (re-run this audit) |
| Windows build + boot | Jean | **UNKNOWN** (unreported) |
| THE SHELLS (flip `CONTACT_SHELL_DEBUG`, read the rings) | Jean | **UNKNOWN** |
| THE APPROACH (walk an agent down, possess) | Jean | **UNKNOWN** |
| THE CROWD (sprint a cluster, wake settles) | Jean | **UNKNOWN** |
| THE CUBES (walk beneath, swing aside; λ keeps some) | Jean | **UNKNOWN** |
| THE SPHERES (path must NOT push the player) | Jean | **UNKNOWN** |

Every machine gate this audit can drive is green; every Jean-run Windows gate is
still unreported (the CONTACT_4 report's gate list has not been checked back).

---

## [A4-2] THE LEVER CENSUS

One row per influence mechanism, sorted by **shape** (Q1: field / pairwise /
spawn-time / lifecycle). "Room" = where the radius/threshold lives (WGSL const /
`GPUDesignConfig` field / tier-or-behavior table column / plain C++ const /
per-instance field). `world.wgsl` line numbers are at base `cd6c0f4`.

Legend for shape: **FIELD** = position-indexed (one/few emitters, N readers —
texture-shaped by the sorting law); **PAIRWISE** = identity-indexed (both
parties have identity — buffer-shaped); **SPAWN** = per-placement CPU;
**LIFECYCLE** = baked / state-transition.

### FIELD (position-indexed)

| Mechanism | LAW body | Eval sites | Radius/threshold · room | Consumers | Cadence |
|---|---|---|---|---|---|
| **Pawn aura** *(reference — done right)* | `compute_pawn_aura` world.wgsl:8833 | writes grid `pawn_aura_cells` :8959 + texture `pawn_aura_tex_write` :8967/8973 | `influence_radius` (default 20) · **uniform** `PawnAuraConfig` :5964 (CPU `pawn.hpp:35`, ×presence) | `sample_pawn_aura` :6003 → terrain height/color at :3012,:4177,:4404 | per-frame GPU compute → **card**; N readers sample |
| **Gradient steering** *(reference — reads a card)* | `agent_post_step` C2b block world.wgsl:6442-6463 | :6455 `sample_terrain_grad_at(ahead)` | `STEER_LOOKAHEAD_WU 4`, `STEER_GAIN 3`, `STEER_GRAD_LO/HI 0.7/1.4` · **WGSL const** :2231-2237 | own velocity (deflect along level-set) | per-frame GPU compute; reads the **baked terrain-gradient field** |
| **Point-source flee** *(trilogy)* | `update_other_agents` point block world.wgsl:7447-7484 | :7457 gate, :7474 falloff | `config.point_bubble_radius` (20) · **`GPUDesignConfig`** `state.hpp:556`; gain `flee_gain_player` · **tier column** `agents.hpp:171` | own velocity | per-frame GPU compute — **recomputed per agent against ONE point** |
| **Cube parting** *(trilogy)* | `update_cube` parting block world.wgsl:7929-7950 | :7936 gate, :7947 falloff | `CUBE_PART_RADIUS 30`, `_CAP 12`, `_GAIN 1` · **WGSL const** :2265-2267 | cube `drift_vel` | per-frame GPU compute — **recomputed per cube against ONE point** |
| **Pawn-forcefield tint** (not a force) | `zone_pawn_ff` world.wgsl:2397-2404 | :4382 (terrain FS) | `PAWN_FORCEFIELD_RADIUS_STATIONARY/_MOVING 6/2`, `_FALLOFF 2` · **WGSL const** :2170-2173 | fragment `base_color` (purple GoL-zone tint) | per-frame GPU fragment |
| **Sphere-forcefield tint** | `zone_sphere_ff` world.wgsl:2406-2413 | :4388 (terrain FS) | `SPHERE_FORCEFIELD_RADIUS 10`, `_FALLOFF 3` · **WGSL const** :2393-2394 | fragment `base_color` | per-frame GPU fragment |
| **GoL suppression** | `pawn_gol_suppression` world.wgsl:2430-2433 | :2634,:3196,:3224,:3256,:4193,:4480 | `ZONE_SUPPRESS_INNER/OUTER 4/15` · **WGSL const** :2422-2423 | ground manifold + terrain/shadow VS cell-lift | per-frame GPU compute + vertex |
| **Veil chain** | `shade_lit` veil world.wgsl:4085-4094; LOD0 gate :9461 | fade :4086,:9658; ring-kills :4230,:4724,:4848,:4881,:9561,:11108,:11442,:11678 | `veil_ring 325`, `veil_icing 40`, `lod0_radius 175`, `veil_strength` · **`GPUDesignConfig`** `state.hpp:523-526` | every draw pipeline (visibility) + frustum-cull LOD | per-frame GPU fragment/vertex/compute |
| **Zone GoL-cell tint** | `apply_gol_color` world.wgsl:5920-5942 | :4375 (terrain FS, cam-faded :4350) | `GOL_FADE_NEAR/FAR 150/300`, `GOL_TINT_STRENGTH 0.70` · **WGSL const** :5822-5839 | fragment `base_color` | per-frame GPU fragment |
| **Portal ellipse** | player/agent behavior world.wgsl:6652-6689 | :6676 ellipse `e<1`; :6682 vertical gate | per-portal `inv_span_sq`,`inv_depth_sq` · **per-instance** `PortalEntry` :5627 (b62); vertical gate `config.point_bubble_radius` | `agent.portal_trigger` (CPU harvest) | per-frame GPU compute |
| **Eviction radii** | consts world.wgsl:7169 (agent 350) / :7201 (floater 400) | agent :7506; sphere :7646; cube :7810 | **WGSL const** (agent mirrored to `agents.hpp:117`, `static_assert == Dim::EXIST_RADIUS`) | sets `is_active=0u` → CPU respawn | per-frame GPU compute |
| **Piers** | `evaluate_pier` world.wgsl:2455-2495; `structure_height_at` :2499 | :2503, :2914 (baked ground) | `PIER_TOTAL 68` · WGSL const :2436; `config.pier_count`; per-pier `PierInstance` · **per-instance** (b26) | the **baked heightfield** (all ground readers inherit) | **baked** (into static height) |

### PAIRWISE (identity-indexed)

| Mechanism | LAW body | Eval sites | Radius/threshold · room | Consumers | Cadence |
|---|---|---|---|---|---|
| **Contact spring** (the shove) | world.wgsl:7260-7280 (player) / :7367-7387 (others) | same | `CONTACT_SPRING 40`, `CONTACT_IMPULSE_CAP 6` · **WGSL const** :2210-2211; `contact_radius`,`contact_mass` · **tier column** `agents.hpp:168-169`; `PAWN_CONTACT_MASS_MULT 4` :2226 | own velocity (mass-weighted) | per-frame GPU; 32-slot loop |
| **Body-to-body flee** (servo) | world.wgsl:7291-7305 (player) / :7398-7412 (others) | same | gate `(personal+personal)×FLEE_SHELL_FRAC` → 15; `NONPLAYER_FLEE_GAIN 0.8` · **WGSL const** :2241,:2251; `personal_radius 30` · **tier column** `agents.hpp:170` | own velocity | per-frame GPU; 32-slot loop |
| **Sphere push** (agents only) | world.wgsl:7414-7436 | same | `contact_radius + fe.body_radius` (per-instance ~1.2-1.5) · **per-instance** | own velocity | per-frame GPU |
| **Cube-vs-pawn contact** | world.wgsl:7908-7922 | same | `CONTACT_CUBE_RADIUS 3 + pg.contact_radius` · **WGSL const + tier** :2223 | cube `contact_force` | per-frame GPU |
| **Flock** | `behavior_flock2d` world.wgsl:7021-7087 | gate :7056 (reads tier `personal_radius`, **NOT** `AGENT_BEHAVIORS[8].neighbor_radius` — that column is dead) | `personal_radius 30` · **tier column** | own velocity (cohesion+alignment) | per-frame GPU, beat-gated; 4-slot random sample |
| **Pursuit** | `behavior_pursuit` world.wgsl:6933-6967 | :6946 gate (2D) | `neighbor_radius 40`, `home_pull 5` · **behavior column** `agents.hpp:148` | own velocity | per-frame GPU |
| **Flee (behavior)** | `behavior_flee` world.wgsl:6977-7011 | :6991 gate (2D), :6994 falloff | `neighbor_radius 30`, `home_pull 8` · **behavior column** `agents.hpp:149` | own velocity | per-frame GPU |
| **MIN_SEPARATION / footprint registry** | `check_position` spawn_engine.hpp:520-538 | :327; gallery.hpp:837; gol_zones.hpp:439 | `MIN_SEPARATION[12][12]` · **plain C++ const** spawn_services.hpp:84; `effective_min = placing_r + footprint.r` (**per-instance** radii) | rejects a placement | **SPAWN** (per-placement CPU) |

### SPAWN-TIME (per-placement CPU)

| Mechanism | LAW body | Eval site | Radius/threshold · room | Cadence |
|---|---|---|---|---|
| **Spawn spacing / density** | `tile_apply_spawn_mult` tile_world.hpp:466-480 | spawn_engine.hpp:639 (`compose_spawn_chance`) | `entity_density`,`spatial_density[]` · **per-tile column** population_themes.hpp:266 | per-spawn (see [A4-6] — miss aborts) |
| **Proximity affinity boost** | `proximity_affinity_boost` spawn_engine.hpp:674-695 | :643 | `PROXIMITY_RADIUS/MAX_BOOST/THRESHOLD/GAP_REDUCTION[12]`, `PROXIMITY_AFFINITY[12][12]` · **plain C++ const** spawn_engine.hpp:106-127 | per-spawn |

### LIFECYCLE (baked / state-transition)

| Mechanism | LAW body | Eval site | Radius/threshold · room | Cadence |
|---|---|---|---|---|
| **Cube plasticity leak** | world.wgsl:8005-8012 | same | `lam = fe.plasticity(per-instance) × config.cube_plasticity(0.6)`; per-tier `CUBE_TIER_GAINS.plasticity` · **tier column** cube_behaviors.hpp:92 + **`GPUDesignConfig`** | per-frame drift→anchor transfer (state) |
| **Kite offsets** | `toggle_cube_kite_mode` cube_behaviors.hpp:382-424; GPU home world.wgsl:7873-7880 | F7 (input.hpp:302); GPU :7875 | per-cube `pawn_offset`,`follow_pawn` · **per-instance** | captured per-toggle; evaluated per-frame |
| **Cube corral** | `corral_cubes` cube_behaviors.hpp:282-356 | F6 (input.hpp:301); tick :358 | `CUBE_CORRAL_RADIUS 120`, `_DURATION 4` · **plain C++ const** cube_behaviors.hpp:62-63 | command one-shot; ticked per-frame CPU |
| **Possession reach** | `try_possess_nearest` agents.hpp:553 | Caps Lock (input.hpp:293) | `POSSESSION_RADIUS 20` · **plain C++ const** agents.hpp:107 | per-input event (CPU) |

**Census note — the count:** 25 mechanisms, extending the seed list with
`zone_sphere_ff` (sphere tint), the flock/pursuit split, and the piers-into-bake
path. Two seed entries collapsed on inspection: "pawn forcefield" and "zone
tints" are the **same** fragment-tint family (`zone_pawn_ff`/`zone_sphere_ff` +
`apply_gol_color`), and "gallery proximity" is **inverted** — the gallery frames
the point and paintings clamp to terrain; there is no "terrain reacts near the
art" field (world.wgsl:9247-9284, explicit no-suppression note :9268-9273).

---

## [A4-3] THE DUPLICATION MAP

Four DRY clusters. For each: the copies, the DRIFT across them, and the ONE body
they should collapse to.

### Cluster (a) — the flee servo, **4 expressions**

| # | Site | Gate | Falloff | Gain (room) | Cap | dt | Structure |
|---|---|---|---|---|---|---|---|
| **A** | `behavior_flee` :6991-6998 | **2D** (`neighbor_radius` 30) | **linear** `1−d/r` :6994 | `home_pull×persist_gain` (behavior+tier) | none (post-step cap) | **×dt** | force-push |
| **B** | player kernel :7291-7305 | **3D** (`(pers+pers)×FRAC`→15) | **none** | `NONPLAYER_FLEE_GAIN 0.8` (WGSL const) | none | **none** | deficit servo + matador |
| **C** | others kernel :7398-7412 | **3D** (same) | **none** | `NONPLAYER_FLEE_GAIN 0.8` | none | **none** | deficit servo + matador (**byte-identical to B**) |
| **D** | point-source :7447-7484 | **3D** (`point_bubble_radius` 20) | **linear** `1−pd/ppr` :7474 | `flee_gain_player` (tier, <1) | none | **none** | deficit servo + matador |

**DRIFT.** Same intent ("give way to an approacher"), four implementations. Gate
dimensionality splits 2D (A) vs 3D (B/C/D). Falloff splits linear (A, D) vs
**absent** (B, C) — the S2a comment at :7468-7472 states D was *given* the falloff
"the servo never had," i.e. B and C still lack it. Gain lives in three different
rooms. Only A is dt-scaled; B/C/D are velocity-target servos added dt-free
(frame-rate coupled by construction). B and C are a literal copy-paste (same 15
lines twice, one per kernel). A fifth, easily-conflated term sits inline with
B/C — the **contact spring** (`push = min((r−d)·SPRING·dt, CAP)·mass`) — which
*is* dt-scaled and capped, unlike its flee neighbors.

**Collapse to:** one **`proximity_response(self, other, gain, cap, mass_ratio,
…)`** that carries 3D gate + overlap spring + approach-gated flee (linear falloff
+ matador split, escape-capped). B and C become one call site each; the contact
spring folds into the same body (`other_r=0` ⇒ flee-only). Proven compilable in
[A4-7] probe (i). A (the policy force) is a different animal — it stays a
behavior, but should call the shared **falloff helper** (cluster c).

### Cluster (b) — radius composition, **no single rule**

| Site | Composition |
|---|---|
| agent–agent contact :7271 | `contact_radius + contact_radius` (sum of two body radii) |
| agent–sphere contact :7427 | `contact_radius + fe.body_radius` (body + per-instance body) |
| cube–pawn contact :7916 | `CONTACT_CUBE_RADIUS(3) + contact_radius` (const + body) |
| body-to-body flee :7291 | `(personal_radius + personal_radius) × FLEE_SHELL_FRAC` (sum × fraction) |
| point-source flee :7457 | `config.point_bubble_radius` (single field, no composition) |
| cube parting :7936 | `CUBE_PART_RADIUS` (single const) |
| spawn spacing :526 | `placing_radius + footprint.radius` (sum of two footprint radii) |

**DRIFT.** Four different arithmetic patterns for "what distance gates this
pair." Sum-of-two-bodies, const+body, single-field, and sum×fraction all coexist
with no shared helper — a reader cannot tell from a call site which convention is
in force.

**Collapse to:** one **`pair_gate(a_radius, b_radius)`** returning the summed
shell, with the fraction/const cases expressed as explicit inputs to it (so the
"×FLEE_SHELL_FRAC" and "single-field" cases read as *deliberate* deviations, not
silent ones).

### Cluster (c) — falloff shapes, **linear vs smoothstep vs absent**

| Shape | Sites |
|---|---|
| linear-in-distance `1−d/r` | point-source flee :7474, behavior_flee :6994, cube parting :7947 |
| linear-in-overlap `(r−d)`, capped, ×dt | contact spring :7275,:7382; cube contact :7920 |
| **absent** (flat across shell) | body-to-body flee B :7296, C :7403 |
| smoothstep | pawn/sphere tint :2400/:2411, GoL suppression :2431, veil :4086, aura stimulus :8858 |

**DRIFT.** The reflex family (the flees) is split three ways — two use linear,
two use none. The *visual* fields consistently use smoothstep; the *physics*
reflexes are inconsistent.

**Collapse to:** one **`falloff(d, r) = clamp(1 − d/r, 0, 1)`** (the reflex
shape — what behavior_flee and the point-source already spell out inline);
smoothstep stays reserved for the tint/visibility fields (a deliberate,
documented split: reflexes are linear, veils are smooth). Wiring B/C through it
is the same one-line fix S2a applied to D.

### Cluster (d) — the personal-shell family, **one number in three rooms**

The value **30** is the social shell, but it lives in:

| Room | Symbol | Read by |
|---|---|---|
| tier column `agents.hpp:170` | `personal_radius = 30` | body-to-body flee gate (×0.25→15) **and** flock gate (raw, world.wgsl:7056) |
| behavior column `agents.hpp:149,150` | `neighbor_radius = 30` (flee), `30` (flock) | behavior_flee gate; **flock's is DEAD** — `behavior_flock2d` reads the tier `personal_radius`, not this |
| WGSL const :2251 | `FLEE_SHELL_FRAC = 0.25` | scales the personal-shell sum |

**DRIFT.** Three "30"s that happen to agree today. The flock's
`AGENT_BEHAVIORS[8].neighbor_radius` is a **dead knob** — a Jean edit there
changes nothing, because the flock body reads the tier `personal_radius`
(world.wgsl:7056). A reader tuning "flock sensing" would edit the wrong 30.

**Collapse to:** one social-radius source. Retire the dead flock
`neighbor_radius` (or wire the flock gate to read it), and make the flee shell a
named product of the single `personal_radius` so the three rooms become one.

---

## [A4-4] THE SORTING VERDICT — the section Jean will rule on

**The law:** position-indexed is texture-shaped (a FIELD / card); identity-indexed
is buffer-shaped (pairwise). For each pairwise-looking row: is it genuinely
pairwise, or a field wearing pairwise clothes?

**The two that already did it right** (the framework Jean is asking about):

- **Pawn aura** (`compute_pawn_aura`, world.wgsl:8833). One emitter (the pawn), a
  radius, a falloff. It does **not** loop agents — it splats the pawn's influence
  into a 64×64 **card** (`pawn_aura_tex_write`, rgba16float, `aura_cs = 3.125`)
  and N terrain consumers **sample** it (`sample_pawn_aura`, bilinear, "a
  continuous influence field," :5995-6002). Position-indexed → texture-shaped.
  Correct.
- **Gradient steering** (C2b, world.wgsl:6455). It does not recompute the
  terrain — it **reads the pre-baked terrain-gradient field**
  (`sample_terrain_grad_at`). A card read, not a per-pair computation. Correct.

**The genuinely-pairwise rows (keep buffer-shaped):**

| Row | Verdict | Why |
|---|---|---|
| Contact spring | **genuinely pairwise** | both bodies have identity; the mass ratio `m_o/(m_s+m_o)` needs *both* masses. Not a field. |
| Body-to-body flee (B/C) | **genuinely pairwise** | responds to a *specific other's* velocity along a *specific* axis; identity-indexed. |
| Sphere push, cube-vs-pawn contact | **genuinely pairwise** | body-vs-body overlap. |
| Flock | **genuinely pairwise** | samples specific neighbor slots (identity). |
| MIN_SEPARATION / footprints | **genuinely pairwise** (spawn-time) | placing-vs-each-existing; identity + spawn cadence. |

**The fields wearing pairwise clothes (the finding):**

| Row | Current shape | True shape | Evidence |
|---|---|---|---|
| **Point-source flee (D)** | per-agent loop, each recomputes its response to **one point** | **FIELD** — one emitter (the pawn/camera), N readers | :7447-7484 reads `point_pos()` + one velocity; the math is identical to the aura's stimulus, just evaluated N times instead of splatted once |
| **Cube parting** | per-cube, each recomputes its response to **one point** | **FIELD** — one emitter, N cube readers | :7929-7950 reads `point_pos()`; same shape as D |

Both are **position-indexed against a single emitter** — texture-shaped by the
law — yet implemented as per-body buffer loops. They are the aura's twin written
the aura's-predecessor way.

### The costed presence-card sketch (channel budget)

A **presence card**: one compute pass per frame splats the point's presence +
approach signal into a card; the walkers and cubes replace their per-body point
math with a **sample**. Probe (ii) in [A4-7] compiled and validated the writer.

- **Card:** 256² **rgba16float** (see the format finding below), 2 channels used
  (`r` = presence/falloff, `g` = approach), = **512 KiB**. The aura's own card is
  the precedent.
- **Writer stage bindings:** 1 storage buffer (emitter list) + 1 uniform
  (params) + 1 storage texture. Against the banner (10 / 12 / 4): **trivially
  within** — this is the aura writer's shape exactly.
- **Reader stages:** +1 sampled texture + 1 sampler each (like `sample_pawn_aura`).
- **Format finding (from probe ii):** `rg16float` — the natural 2-channel choice
  named in the handoff — **is not a core write-only storage format** and Dawn
  rejects the bind-group layout ("RG16Float does not support storage texture
  access WriteOnly"). The portable format is **rgba16float**, which is exactly
  what the pawn aura already uses. So the presence card inherits the aura's format
  by necessity, not just by convenience.

The card also **closes the camera-host velocity gap**: today D and the cube
parting fall back to `BUBBLE_PART_SPEED` because there is no camera velocity
field (the deferred `config.point_vel_x/z`). A central writer holds the point's
velocity once, so "a still camera parts no one" becomes true for free.

**Verdict: PROPOSAL-SURFACE.** The point-source flee and the cube parting are the
two levers the sorting law says belong on the field/card side — beside the aura,
not beside the pairwise gather.

---

## [A4-5] THE CONTROL-SURFACE CENSUS

Every knob that tunes "how bodies react," its value, room, whether a change is
live (uniform re-upload) or a rebuild (WGSL const → recompile), whether it has a
stated reference scale, and whether it is mirrored across the CPU/GPU seam.

| Knob | Value | Room | Live / rebuild | Reference scale? | Mirrored? |
|---|---|---|---|---|---|
| `CONTACT_SPRING` | 40 | WGSL const | rebuild | units (Δv/wu·s) — yes | no |
| `CONTACT_IMPULSE_CAP` | 6 | WGSL const | rebuild | units (Δv) — yes | no |
| `NONPLAYER_FLEE_GAIN` | 0.8 | WGSL const | rebuild | dimensionless — yes | no |
| `FLEE_SHELL_FRAC` | 0.25 | WGSL const | rebuild | bubble 20 → 15 — yes | no |
| `CONTACT_CUBE_RADIUS` | 3 | WGSL const | rebuild | cube alt (flagged [UNREACHABLE]) | no |
| `CUBE_PART_RADIUS/_CAP/_GAIN` | 30/12/1 | WGSL const | rebuild | indoor cap 18.75 — yes (outdoor caveat) | no |
| `PAWN_CONTACT_MASS_MULT` | 4 | WGSL const | rebuild | dimensionless — yes | no |
| `STEER_LOOKAHEAD/_GAIN/_GRAD_LO/_HI` | 4/3/0.7/1.4 | WGSL const | rebuild | cell 3.125 (lookahead only) — partial | no |
| `BUBBLE_PART_SPEED` | 4 | WGSL const | rebuild | wu/s — yes | no |
| `PAWN_FORCEFIELD_RADIUS_*` | 6/2 | WGSL const | rebuild | **none stated** | no |
| `contact_radius` (×4) | 1.6/1.4/2.0/1.8 | tier column agents.hpp | rebuild (baked to GPU tier def) | body — yes | **yes** (→ `GPUAgentTierDef`) |
| `contact_mass` (×4) | 1.0/0.8/1.5/1.2 | tier column | rebuild | yield authority — yes | **yes** |
| `personal_radius` (×4) | 30 | tier column | rebuild | flock 30 — yes | **yes** |
| `flee_gain_player` (×4) | 0.70/0.85/0.50/0.60 | tier column | rebuild | catchability (<1) — yes | **yes** |
| `neighbor_radius` (flee/pursuit/flock) | 30/40/30 | behavior column | rebuild | **none stated**; flock's is **dead** | **yes** |
| `home_pull` (flee/pursuit) | 8/5 | behavior column | rebuild | **none stated** | **yes** |
| `config.point_bubble_radius` | 20 | `GPUDesignConfig` | **live** (setter + machine gate) | `contracts/point.hpp` — yes | gated (setter) |
| `config.cube_plasticity` | 0.6 | `GPUDesignConfig` | **live** | dimensionless — yes | gated (setter) |
| `CUBE_TIER_GAINS.plasticity` (×4) | 1.0/0.8/1.2/0.5 | tier column cube_behaviors.hpp | rebuild | per-tier λ — yes | **yes** |
| `AGENT_EVICTION_RADIUS` | 350 | C++ const + WGSL const | rebuild | `Dim::EXIST_RADIUS` — yes | **yes (hand-mirrored)** |
| `FLOATER_EVICTION_RADIUS` | 400 | WGSL const | rebuild | comment only | no C++ mirror |
| `POSSESSION_RADIUS` | 20 | C++ const | rebuild | bubble 20 — yes | no (CPU only) |

**Answer 1 — how many rooms to tune "how bodies react": SIX.**
(1) WGSL module consts (the contact/flee/steer/cube physics); (2) the
`AGENT_TIER_GAINS` columns (contact_radius, contact_mass, personal_radius,
flee_gain_player); (3) the `AGENT_BEHAVIORS` columns (home_pull, neighbor_radius,
drag, speed_cap); (4) `GPUDesignConfig` fields (point_bubble_radius,
cube_plasticity — the only **live** knobs); (5) `CUBE_TIER_GAINS` (per-tier
plasticity); (6) per-instance fields (`fe.body_radius`, `fe.plasticity`,
`fe.spring_stiffness`, `fe.drag`). To answer "why does the crowd feel wrong" you
must hold all six in your head at once.

**Answer 2 — mirrored without a machine gate (drift hazards):**
- **`AGENT_EVICTION_RADIUS`** — the WGSL const (world.wgsl:7169) and the C++
  const (agents.hpp:117) are hand-copied twins. The `static_assert` only pins the
  C++ side to `Dim::EXIST_RADIUS`; **nothing catches WGSL/C++ drift** — the
  header itself says so ("The compiler cannot catch drift; the prose below is the
  contract," agents.hpp:111-113).
- **`FLOATER_EVICTION_RADIUS`** — WGSL const with **no C++ mirror at all** (named
  only in comments); a CPU consumer that assumed 400 would silently disagree.
- The **tier/behavior columns** cross the seam via `GPUAgentTierDef` (a baked
  upload), which *is* size-checked (`static_assert(sizeof … == 48)`) — those
  mirror *with* a gate. The two eviction radii are the ungated ones.

**Answer 3 — knobs with NO reference scale (the CONTACT_4 misses):**
- `PAWN_FORCEFIELD_RADIUS_STATIONARY/_MOVING` (6/2) — the scale ledger names them
  as "distinct from the personal-shell family" but gives **no reference** for why
  6 and 2. (It is a *tint* radius, not a physics radius — so arguably it belongs
  to the visual ledger, not the influence ledger; either way it is unreferenced.)
- `AGENT_BEHAVIORS.neighbor_radius` (flee 30 / pursuit 40 / flock 30) and
  `home_pull` (8 / 5) — the behavior-table knobs never got a reference column;
  CONTACT_4's ledger covered the WGSL consts and the tier columns but **stopped
  at the behavior table**. `neighbor_radius`'s flock row is worse than
  unreferenced — it is dead ([A4-3] cluster d).

---

## [A4-6] CONTRACT CONTRADICTIONS

Swept the four named files (`surface_services.hpp`, `spawn_services.hpp`,
`tile_world.hpp`, `entity_types.hpp`) for decl/def pairs where the declared
contract and the implementation disagree. **NO FIXES — findings only.**

**Fatal-site sweep:** the only `std::abort` / `std::terminate` / `assert(` /
`throw` in the entire `the_board` tree that is a *runtime* fatal (not a
`static_assert`) is **one**: `tile_world.hpp:476`. Every other decl/def pair in
the four files is consistent (e.g. `spawn_services.hpp:215` `indoor_bounds_clamp`
declares "Returns false … the caller skips the spawn (the loud line prints)" and
the impl matches — graceful with a print, no abort).

### CONFIRMED — `tile_apply_spawn_mult` (F3): declared graceful, implemented fatal

Two sibling tile-cache accessors in the **same file** handle a cache miss in
**opposite** ways, and F3's own declaration describes the graceful behavior its
implementation does *not* have.

**Room 1 — the declaration** (`tile_world.hpp:197-201`), verbatim:

```
// F3: applies the tile's spawn modifiers ONTO the accumulator —
// density then per-family theme multiplier, in that order (the two
// multiplies stay separate: bit-identity under FP non-associativity);
// no memory of the tile = no change.
void tile_apply_spawn_mult(const TileWorldState& tw, int32_t gx, int32_t gz,
                           uint32_t family, float& adj_mod);
```

"**no memory of the tile = no change**" — a cold tile leaves `adj_mod` untouched
(graceful no-op).

**Room 2 — the definition** (`tile_world.hpp:466-477`), verbatim:

```
inline void tile_apply_spawn_mult(const TileWorldState& tw, int32_t gx, int32_t gz,
                                  uint32_t family, float& adj_mod) {
    auto it = tw.tileCache_.find({ gx, gz });
    if (it == tw.tileCache_.end()) {
        …
        std::cerr << "[SPAWN] tile_apply_spawn_mult MISS at (" << gx << "," << gz
                  << ") family " << family << " — ensure_tile did not precede the gate\n";
        std::abort();
    }
```

On a cold tile the implementation prints and **`std::abort()`s** — the opposite
of "no change."

**The sibling that proves the contradiction is not a house style** — F4
`tile_archetype` (`tile_world.hpp:203-206` decl, `:484-487` def) declares
"returns false (out untouched) where the tile is cold — callers keep their own
miss defaults" **and its impl does exactly that** (`return false;`). So within one
file, a cold-tile miss is fatal for F3 and graceful for F4 — and F3's decl
comment ("no memory of the tile = no change") reads like F4's contract, not F3's
behavior.

The def's own comment reframes the intent ("A miss here means the
allocation→spawn ordering broke — fail at the seam, not by quietly dropping the
theme layer"), which is a defensible engineering choice — but it **contradicts
the declaration's stated contract**. The audit records the contradiction; it does
not rule on which side is right (that is Jean's call, and NO FIX is applied).

`surface_services.hpp` and `entity_types.hpp` reference the tile functions only
by name in prose (no competing decl); no other declared-graceful / implemented-
fatal pair exists in the four files.

---

## [A4-7] PROBES

Both probes ran on the throwaway branch (scratchpad workspace, base `cd6c0f4`,
never mainline), compiled and validated under real Dawn (SwiftShader / Tint).
**>10 s FLAG: none.**

### Probe (i) — unified `proximity_response()` + 33-slot caller

A single body absorbing the trilogy's pairwise copies: 3D gate + overlap spring
(planar, capped, dt-scaled) + approach-gated flee (linear falloff + matador
tangential split, escape-capped), `other_r=0` ⇒ point-source (flee-only). Caller
kernel loops 32 identity-indexed pairs + 1 point source (the 33 slots).

- **Compile:** GREEN, 0 messages, 6.6 ms.
- **Pipeline create:** GREEN, 89.0 ms (well under the 10 s flag).
- **Bindings:** 1 storage + 1 uniform + 0 storage-texture — under the banner.
- **Reading:** the four flee copies + the two contact copies collapse into one
  reviewable body; the drift of [A4-3] cluster (a) is closed *by construction*
  (there is one gate, one falloff, one cap to read).

### Probe (ii) — presence-card writer (splat N emitters → card)

Splats up to 16 bounded emitters (falloff-weighted presence in `r`, approach
signal in `g`) into a 256² card, `cell = 3.125` (the aura/patch cell).

- **`rg16float` (as named in the handoff):** **FAILS validation** — Dawn:
  "Texture format TextureFormat::RG16Float does not support storage texture
  access StorageTextureAccess::WriteOnly." (Not a core write-only storage format.)
- **`rgba16float` (the aura's format):** **GREEN**, 0 messages, compile 1.0 ms +
  pipeline 14.2 ms.
- **Bindings:** 1 storage + 1 uniform + 1 storage-texture — under the banner
  (10 / 12 / 4).
- **Cost vs banner:** the writer is the aura writer's shape; 256² rgba16float =
  512 KiB; readers add +1 sampled texture +1 sampler each. Comfortable.

**Probe verdict: PASS** — the two consolidations the report proposes both
compile and validate cheaply; the only surprise is the format-portability point
(rg16float → rgba16float), which the presence card inherits from the aura anyway.

---

## TAIL — the question Jean asked

> *Which of the levers we built during the contact trilogy should have been a
> lever on the existing unified framework instead of a new system, and what would
> it cost to move it there now?*

**Two levers, and they split by the sorting law.**

**1. The point-source flee (the bubble presence) — and its twin, the cube
parting — should have been a lever on the pawn-aura field framework, not new
per-body code.** The aura already is the unified framework the trilogy was
reaching for: one emitter (the pawn), a radius, a falloff, splatted into a card
that N consumers sample (`compute_pawn_aura` → `sample_pawn_aura`). The
point-source flee (world.wgsl:7447-7484) and the cube parting (:7929-7950) are
*position-indexed against that same single emitter* — texture-shaped by the law —
but the trilogy wrote them as per-agent and per-cube loops that recompute the
point's field N times each. They are the aura written the aura's-predecessor way.
Had they been a lever on the aura framework, the bubble presence would be *one*
authored radius+falloff feeding a "presence/approach" channel, and the walkers,
the cubes, and (for free) the camera-host path would all read the *same* field —
instead of the point math being copy-drifted across two kernels with a
`BUBBLE_PART_SPEED` fallback papering over the missing camera velocity.

**Cost to move now:** small, and de-risked by probe (ii). Add one presence-card
writer compute pass (the aura writer's twin: 1 storage + 1 uniform + 1 storage
texture, a 256² rgba16float card = 512 KiB — all under the 10/12/4 banner;
pipeline compiles in ~15 ms). Replace the per-agent point block (:7447-7484) and
the per-cube parting block (:7929-7950) with a `sample_presence()` read (+1
sampled texture +1 sampler per reader, exactly as `sample_pawn_aura`). The
framework, the format (rgba16float — forced, since rg16float is not storage-
writable), the cell size (3.125), and the bilinear-sample idiom all already exist
in the aura. The move also retires the deferred `config.point_vel_x/z` gap: a
central writer holds the point's velocity once, so a still camera parts no one.

**2. The contact spring and the body-to-body flee should NOT be a field — they
are genuinely pairwise — but they should have been ONE body, not four.** The
trilogy correctly kept them identity-indexed (the mass ratio needs both bodies),
but it grew four flee expressions (two of them byte-identical B/C) that drifted on
gate dimensionality, falloff presence, gain room, and dt. The "existing unified
framework" for *these* is not the aura — it is a single `proximity_response()`
(probe i: GREEN, ~96 ms, 2 bindings). **Cost to move now:** replace the two
copied flee blocks + the two contact blocks with calls to one body; the drift
closes by construction, and there is nothing new to allocate (same buffers, same
loop).

**In one line:** the point-source flee + cube parting were a *field* built as
pairwise code (move them onto the aura card — cheap, framework exists); the
contact spring + body flee were *pairwise* built as four copies (collapse to one
`proximity_response` — cheaper still). The trilogy's mechanisms are correct; what
it lacked was the discipline to ask, per lever, "one emitter or two identities?"
— the sorting law — before writing the code.

---

*Read-only audit. Base `cd6c0f4` unchanged. No mainline edits; no fixes applied.
Probe sketches captured in the session scratchpad and discarded with the throwaway
branch.*
