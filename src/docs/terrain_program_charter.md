# THE TERRAIN PROGRAM — CHARTER (Phase 0)

Founding document of the terrain reorganization campaign.
Audited against the LIVE tree at `e06fbc2` (branch `COUPLING_SAGA_SWEEP_CHECKERS`).
Sibling of `7t_program_theory_v3.md`; the model it imitates is
`contracts/ground_architecture.hpp`.

CENSUS LAW: every count in this document ships with its recipe — the exact
command that produced it, runnable from repo root — so any future auditor
reproduces every number. Function names are the stable addresses; line
anchors are secondary and dated to `e06fbc2`.

`world.wgsl` throughout = `src/cartridges/the_board/realization/world.wgsl`
(11905 lines; recipe: `wc -l src/cartridges/the_board/realization/world.wgsl`).

---

## PREFACE — WHY

The checker saga's five nights of wrong fixes were one condition:
the terrain cannot currently be STATED. Its laws live in four homes at
once — seed hashes, panel constants, live uniforms, gating braided
through the composite — with the bake/live relationship nowhere
written. Illegibility was the bug. The terrain program's goal is to
make the terrain stateable: one sentence per stratum, and the code
arranged so the sentences are checkably true. The campaign's standing
reframe: the baked texture is not a second authority — it is a CACHE
of one function at rest. pixel = same function, two moments.

---

## THE PIXEL EQUATION (the spine)

```
pixel = LIGHT( GUESTS( PIGMENT( FIELD(x, z, seed, voice),
                                voice ),
                       aura, pulse, gol ),
               sun, fog )
```

…with MOTION deforming the geometry underneath by the same voice,
and GROUND as the static truth MOTION composes over.

### The strata

- **GROUND** — the manifold: baked heightfield at t=0, contributors,
  policies. Static truth. (`ground_architecture.hpp` is the model the
  whole charter imitates.)
- **FIELD** — the distribution: seeded functions answering WHAT is at
  (x,z) — vocabulary (smooth/checker/chess/mono), palette weights,
  region anatomy, receptivity, survival. Stationary by default, WITH a
  declared live surface (coverage/spread/edge — the revived mode dials'
  true home).
- **PIGMENT** — one function CellIdentity → RGB: all color vocabularies,
  the tier cascade, the anchors. Voice enters as parameters.
- **MOTION** — the deformation voice: terrain_time, band blends, phase
  origins, overlay-wave evaluators, pulse SHAPE contributors.
- **GUESTS** — external writers onto the surface: aura (tint/height/
  normal), pulse VISUAL, GoL tint, force-field tints. The terrain owns
  the receiving seam, not the guests' internals (GoL keeps its own
  panel — standing ruling).
- **VOICE** — the single live-modulation bus: every music/live channel
  the terrain accepts, one struct, one setter, one mirror, one witness.
  Couplings write VOICE and nothing else.
- **WITNESS** — permanent instruments: numbered debug views, console
  lines at every seam, and the standing invariant: SILENCE IS
  PIXEL-IDENTICAL.
- **LIGHT** — sun/fog/shading: SHARED with the whole scene. The terrain
  CONSUMES light; it does not own it. The fog coupling stays outside
  the terrain program. This boundary is explicit.

---

## C1. THE EQUATION TABLE

Every function that touches terrain shape or color, assigned to exactly one
stage. Base recipe: `grep -n '^fn ' world.wgsl` (220 fns total; recipe:
`grep -c '^fn ' world.wgsl`), filtered to terrain-relevant; CPU side censused
via the setter/boot/flush greps quoted in §C1-recipes. Functions that resist
one stage are FINDINGS (§C1-F), not forced.

### GPU (world.wgsl)

| function | stage | one-line role |
|---|---|---|
| terrain_activity_at (L404) | GROUND | activity+beat_freq lattice gating wave frozen/moving mix |
| band_activity_level (L397) | GROUND | per-band smoothstep threshold on raw activity |
| evaluate_directional_wave (L447) | GROUND | directional sine primitive w/ perpendicular damping |
| evaluate_radial_wave (L465) | GROUND | concentric-ring sine primitive w/ radial damping |
| evaluate_lattice_wave (L481) | GROUND | per-node seeded wave draw; frozen/moving phase mix |
| terrain_band_contribution (L560) | GROUND | 2×2 Hermite blend of node waves per band |
| terrain_height_at (L607) | GROUND | total lattice height = Σ 6 band contributions |
| terrain_height_and_complexity (L620) | GROUND | fused height + complexity for the two-pass bake |
| tile_grid_lookup (L932) | GROUND | per-tile archetype/amp/bias entry lookup |
| tile_modifiers_at (L949) | GROUND | Hermite tile amp/bias (+latent activation) modifiers |
| evaluate_pier (L2237) | GROUND | one pier instance's rotated-footprint height delta |
| structure_height_at (L2281) | GROUND | max over pier instances (CONTRIB_SOLIDS) |
| evaluate_pyramid (L2315) | GROUND | one pyramid instance's tapered height |
| contrib_pyramids_at (L2359) | GROUND | max active pyramid height (CONTRIB_PYRAMIDS) |
| contrib_gol_zones_at (L2373) | GROUND | raw GoL cell extrusion height contributor |
| contrib_gol_suppression_at (L2414) | GROUND | pawn-local GoL flattening (LATENT, 0 callers) |
| contrib_static_base_at (L2661) | GROUND | fused lattice×tile_mods+bias+piers static base |
| contrib_paintings_base_at (L2673) | GROUND | stub 0.0 contributor (declared intent) |
| contrib_vegetation_base_at (L2683) | GROUND | stub 0.0 contributor (declared intent) |
| ground_formed_with_complexity (L2698) | GROUND | baked-heightfield contributor set, hand-fused |
| query_ground_placement_pyramid/painting/vegetation (L2948/2961/2979) | GROUND | placement policies (LATENT; live via manifold_height_hf) |
| query_ground_baked_heightfield (L2991) | GROUND | static+pyramids analytic form of the cached texture |
| manifold_overlay_stack (L3017) | GROUND | shared additive fold: static+pyramids+gol+waves+pulses |
| query_ground_flyer/walker/walker_tilt/walker_pair/walker_agent/celestial (L3038–3171) | GROUND | per-policy height queries |
| query_ground_flyer_gradient/walker_gradient/walker_walkable (L3184–3221) | GROUND | finite-diff gradients / cliff-clamped variant |
| manifold_height_hf (L3298) | GROUND | policy-id switch — THE FOLD's one declared home |
| manifold_position / manifold_resolve (L3323/3332) | GROUND | surface cast; cast + finite-diff normal |
| terrain_normal_at (L5933) | GROUND | walker tilt normal via manifold_resolve |
| zone_sample_baked_terrain_y (L5893) | GROUND | baked heightfield scan for zone-mesh alignment |
| sample_terrain_y_at (L8431) | GROUND | baked heightfield sample for placement/camera |
| patch_skirt_grid (L293) | GROUND | skirt vertex → perimeter grid mapping |
| generate_terrain_indices (L7321) | GROUND | compute: patch index winding |
| generate_patch_heights (L7356) | GROUND | compute bake pass 1: heights per texel |
| generate_patch_gradients (L7377) | GROUND | compute bake pass 2: gradients |
| patch_terrain_vs (L4014) | GROUND | render VS: heightfield tex + aura + waves + pulses (see F6) |
| lattice_coord / lattice_weight (L981/989) | FIELD | world→lattice cell + Hermite weights |
| lattice_node_seed / color_lattice_seed (L435/996) | FIELD | the seed primitives for every lattice |
| hash_property / sample_gaussian (L325/336) | FIELD | seeded uniform / Gaussian draw primitives |
| palette_weights_at_node / palette_field_at (L1004/1045) | FIELD | palette dominance lattice + interpolation |
| mode_tendency_at_node / mode_field_at (L1038/1057) | FIELD | smooth↔discrete tendency (quintic) |
| transition_style_at_node / transition_style_at (L1071/1082) | FIELD | blend/hybrid/scatter trimodal style |
| sparse_base_at_node / sparse_cluster_at_node / sparse_field_at (L1094/1100/1105) | FIELD | sparse envelope × cluster boost |
| coupling_strength_at_node / coupling_direction_at_node / terrain_coupling_at (L1138/1146/1156) | FIELD | terrain→mode coupling lattice (DISABLED — magnitude 0, C2-F1) |
| chess_tendency_at_node / chess_color_a/b_at_node / chess_field_at (L1173–1208) | FIELD | chess tendency (pow-25) + pair colors |
| discrete_region_at_node / discrete_region_at (L1248/1273) | FIELD | region mean+variance+receptivity anatomy |
| discrete_mono_at_node / discrete_mono_at (L1262/1301) | FIELD | mono tendency (pow-20) |
| evaluate_cell_fields (L7481) | FIELD | all fields at a cell → CellFieldState |
| checker_region_median (L1327) | PIGMENT | music median: S1 pull + S2 static per-region wander |
| discrete_cell_color (L1342) | PIGMENT | the tier cascade, music-painted |
| discrete_cell_color_at_tier (L1404) | PIGMENT | forced-tier flat switch (drift target) |
| palette_color_smooth (L1847) | PIGMENT | weighted palette blend (ROW 8 governing expression) |
| palette_target_color (L7603) | PIGMENT | palette index → drift target color |
| composite_cell_color / composite_cell_color_biased (L7536/7567) | PIGMENT | the doors: blend/scatter/sparse compositor (twins) |
| animated_cell_color (L7622) | PIGMENT | live re-composite — ZERO callers (F10) |
| animated_cell_color_lut (L7666) | PIGMENT | LUT-accelerated live re-composite (the live path) |
| pack_cell_tag / unpack_cell_tag_mode/tier/height (L1927–1940) | PIGMENT | behavior tag ↔ alpha channel |
| generate_patch_cells (L7738) | PIGMENT | compute: bake color+tag texture + mode/style/sparse LUT |
| patch_terrain_fs (L4094) | PIGMENT | terrain FS: bake → live recolor → guests → light (see F5) |
| get_band_blend / get_band_phase_origin (L535/547) | MOTION | per-band overlay accessors (DRIVERLESS) |
| overlay_band_params (L2735) | MOTION | overlay dir/freq/amp derivation (ROW 7 matrix) |
| contrib_terrain_waves_at (L2754) | MOTION | Σ 6 overlay sines, gated by terrain_time>0 |
| terrain_wave_overlay_with_gradient (L2787) | MOTION | fused overlay height + analytic gradient |
| contrib_radial_pulses_at (L2846) | MOTION | 8-slot expanding onset rings (DRIVERLESS) |
| sample_pawn_aura (L5648) | GUESTS | toroidal aura texture sample |
| contrib_pawn_aura_at_external / _self (L2890/2912) | GUESTS | aura as height contributor (see F4) |
| compute_pawn_aura (L8198) | GUESTS | compute: aura spring grid (calls gol_composite_cell_color) |
| zone_pawn_ff / zone_sphere_ff (L2189/2198) | GUESTS | force-field falloffs for tint sites |
| gol_cell_hash / gol_cell_variation (L5464/5468) | GUESTS | per-GoL-cell variation |
| reflect01 / wrap01 / apply_boundary (L5473–5483) | GUESTS | pulse-zone boundary modes |
| pulse_cell_target (L5491) | GUESTS | Pulse per-cell target (uses mode_gol_tick_scale) |
| gol_composite_cell_color (L5518) | GUESTS | terrain cell color under GoL (identity music — INTENT) |
| apply_gol_color / apply_gol_extrusion_color (L5546/5572) | GUESTS | GoL tint onto terrain / extrusion color |
| tag_cell_behavior (L7709) | GUESTS | seeded GoL eligibility tag (see F8) |
| zone_derive_params / zone_gol_sync / zone_gol_evolve (L5781/7795/7810) | GUESTS | zone config derive / life sync / Conway-Pulse tick |
| coupling_gol_next_state (L3475) | GUESTS | Conway B3/S23 rule |
| zone_emit_quad / zone_mesh_gen_cell / zone_gol_mesh_reset / zone_gol_mesh_gen (L7927–8060) | GUESTS | extrusion mesh generation |
| zone_extrusion_vs / zone_extrusion_fs / shadow_zone_extrusion_vs (L8082/8132/8167) | GUESTS | extrusion draw (suppression mirrors — F14) |
| coupling_active (L2090) | VOICE | mute_couplings bitmask gate |
| coupling_terrain_to_sphere_orbit_height (L3354) | VOICE | consumes terrain to move the sphere (see F9) |
| sample_shadow_pcf / calc_directional_light / calc_point_lights / sample_spot_shadow_pcf / calc_spot_light (L3724–3898) | LIGHT | sun/point/spot lighting |
| veil_dither_noise / shade_lit (L3933/3941) | LIGHT | dither noise; ambient+lights+fog+veil (can discard — F17) |
| coupling_pawn_to_sun_vp (L3388) | LIGHT | snapped ortho sun VP (shadow stability) |
| shadow_patch_terrain_vs (L4227) | LIGHT | terrain shadow geometry (heightfield + waves) |
| CHECKER_DEBUG_VIEW branches (const L1729; branches L4125–4132) | WITNESS | view 1 = resultant meter, 2 = receptivity map |

### CPU

| function / block | file | stage | role |
|---|---|---|---|
| VisualCanvas::tick fog block + FOG_* tables | src/coupling/visual_canvas.hpp | VOICE (LIGHT-bound) | held field → fog density+tint, Segment-glided |
| VisualCanvas::tick checker block + PC_COLOR + CHECKER_* | src/coupling/visual_canvas.hpp | VOICE | 12-pc window lengths → resultant+amount+variance, 4-beat S&H, 2/8 envelope |
| VisualCanvas::bind | src/coupling/visual_canvas.hpp | VOICE | one-time source/target resolution |
| PARAM_LAYOUT terrain.checker_mean/var rows | src/coupling/visual_canvas.hpp | VOICE | bank slots 10–12 / 13–14 (names are DFT-era fossils — C5) |
| set_fog · set_terrain_time · set_band_motion · set_mode_color_shift · set_mode_checker_scatter · set_palette_center/light/weight · set_mode_palette_drift · set_checker_color_field · set_mode_gol_scales · set_pulse_data · set_terrain_amp_ceiling · set_aura_enabled · set_pawn_aura_height/amp_scale/height_bias · set_world_seed · set_indoor_height_cap | src/cartridges/the_board/realization/state.hpp | VOICE | dirty-flagged writers into the GPUDesignConfig mirror |
| Cartridge::initialize boot-pin block (L416–439) | src/cartridges/the_board/cartridge.hpp | VOICE | writes terrain_looks ROW 2 rests at boot |
| Cartridge::phase_motion_drivers U4 (L762–796) | src/cartridges/the_board/cartridge.hpp | VOICE | per-frame flush: canvas tick → set_fog + set_checker_color_field |
| [CHECKER] fprintf · [FLUSH] one-shot | visual_canvas.hpp / cartridge.hpp | WITNESS | decode line per read; first seam crossing |
| terrain_looks.hpp (whole file) | src/cartridges/the_board/surface/terrain_looks.hpp | constants panel | ROW 1 palette rests, ROW 2 rest pins, ROWS 3–9 pointers |
| ContributorId / PolicyId / CONTRIBUTOR_DAG / POLICIES[] | src/cartridges/the_board/contracts/ground_architecture.hpp | GROUND | the contract the WGSL mirrors (L2429–2456) |
| write_pier / clear_pier / recompute_and_upload_pier_count / setup_test_rig_piers | src/cartridges/the_board/surface/patch_system.hpp | GROUND | CPU authoring of pier instances |
| generate_patch_batch / make_patch_params / mark_patches_for_regen | src/cartridges/the_board/surface/patch_system.hpp | GROUND | bake dispatch orchestration |

### C1-F. COMPLETENESS FINDINGS (the discoveries — functions resisting one stage)

| # | item | why it resists |
|---|---|---|
| F2 | hash_property, sample_gaussian, lattice_node_seed | seed primitives used by GROUND waves, FIELD lattices, GUESTS zones, orbs — assigned FIELD as the seed vocabulary home |
| F3 | contrib_terrain_waves_at / contrib_radial_pulses_at | registered DAG contributors (GROUND) yet ARE the MOTION shapes — genuinely GROUND∧MOTION; they sit inside every policy stack |
| F4 | aura contribs + sample_pawn_aura | simultaneously a GROUND height contributor (CONTRIB_PAWN_AURA) and a GUESTS tint writer |
| F5 | patch_terrain_fs | spans PIGMENT + GUESTS + WITNESS + LIGHT in one body — the single largest illegibility site |
| F6 | patch_terrain_vs | GROUND + GUESTS(aura) + MOTION(waves,pulses) hand-fused |
| F7 | gol_composite_cell_color | PIGMENT machinery invoked only from GUESTS; music inputs deliberately identity (STATUS: INTENT, L5528–5533) |
| F8 | tag_cell_behavior | FIELD-style rolls producing a GUESTS eligibility tag baked into the PIGMENT texture alpha |
| F9 | coupling_terrain_to_sphere_orbit_height | consumes terrain, writes nothing onto it — VOICE only nominally |
| F10 | animated_cell_color | ZERO callers (recipe: `grep -n '[= ]animated_cell_color(' world.wgsl` → def only) — dead twin of _lut |
| F11 | contrib_gol_suppression_at | zero call sites; self-documented LATENT[policy-surface] |
| F12 | query_ground_placement_* | LATENT; live placement is the baked hybrid |
| F13 | tile_modifiers_at .z | computed, consumed by no caller (LATENT[tile-activation]) |
| F14 | GoL suppression law in 4 places | contrib_gol_suppression_at + walker inline + zone_extrusion_vs + shadow twin — documented sync hazard (L8102–8111) |
| F15 | DRIVERLESS trio+ | `grep -c 'DRIVERLESS since gen-1' world.wgsl` → 5 sites — VOICE channels held at rest, revive-or-delete flagged in-file |
| F16 | zone_pawn_ff / zone_sphere_ff | named zone_* but are pure GUESTS tints |
| F17 | shade_lit carries the veil and can discard | a LIGHT function holding a visibility jurisdiction |
| F18 | WITNESS has no dedicated functions | the instrument is an embedded const branch + scattered fprintf — not factored |
| F19 | consumers excluded by boundary (read terrain, author nothing) | pawn_ground_resolve, compute_entity_placement, update_camera clamp, agent behaviors — seen, not missed |

### C1-recipes

```
wc -l world.wgsl                                      # 11905
grep -c '^fn ' world.wgsl                             # 220
grep -n '^fn ' world.wgsl                             # inventory base
grep -n '[= ]animated_cell_color(' world.wgsl         # F10 (0 call sites)
grep -n 'contrib_gol_suppression_at(' world.wgsl      # F11
grep -n 'DRIVERLESS since gen-1' world.wgsl           # F15 (5 sites)
grep -n 'set_[a-z_]*' src/cartridges/the_board/realization/state.hpp
sed -n '416,439p;762,796p' src/cartridges/the_board/cartridge.hpp
```

---

## C2. FIELD CENSUS

All lattices share `lattice_node_seed` (L435); color lattices route through
`color_lattice_seed(node, band)` = `lattice_node_seed(world_seed, node, band+100)`.
Interpolation: bilinear Hermite (`lattice_coord`/`lattice_weight`), EXCEPT
`discrete_region_at`, `discrete_mono_at`, `terrain_activity_at`, which
hand-inline the identical math (duplication finding C2-F2).

| # | lattice | node fn | spacing | seed band | props | shaping | consuming door/function |
|---|---|---|---|---|---|---|---|
| 1 | palette | palette_weights_at_node | PALETTE_LATTICE_SPACING = 300 | 0 | 500 | dominant .85 / minor .05 select | smooth_color via palette_color_smooth |
| 2 | mode | mode_tendency_at_node | MODE_LATTICE_SPACING = 120 | 1 | 501 | raw^5 (MODE_BIAS_EXPONENT) | blend + scatter doors; GoL eligibility; zone node reuse |
| 3 | style | transition_style_at_node | TRANSITION_LATTICE_SPACING = 200 | 2 | 510 | trimodal 0 / 0.5 / 1 | blend↔scatter mix |
| 4 | sparse base | sparse_base_at_node | SPARSE_BASE_SPACING = 160 | 3 | 520 | raw^3 | sparse door |
| 5 | sparse cluster | sparse_cluster_at_node | SPARSE_CLUSTER_SPACING = 40 | 4 | 521 | raw; base×(1+2·cluster) | sparse door |
| 6 | chess | chess_tendency/color_a/color_b_at_node | CHESS_LATTICE_SPACING = 55 | 12/13/14 | 850; 860–862; 870–872 | tendency raw^25; colors raw RGB | chess tier of the cascade |
| 7 | region | discrete_region_at_node | DISCRETE_COLOR_LATTICE_SPACING = 80 | 10 (wander id: 20) | 800–804; wander 601–603 | var = .02+raw·.23; receptivity raw | tinted + full-color tiers |
| 8 | mono | discrete_mono_at_node | DISCRETE_MONO_LATTICE_SPACING = 250 | 11 | 810 | raw^20 | BW/tint cuts |
| 9 | terrain-mode coupling | coupling_strength/direction_at_node | COUPLING_LATTICE_SPACING = 250 | 15/16 | 530/531 | strength raw^3; direction trimodal ±1/0 | RETIRED whole (P1-C4, ruling 6); bands/props stay reserved |
| 10 | activity (non-color) | inline in terrain_activity_at | ACTIVITY_LATTICE_SPACING = 400 | 50 (direct) | 220/221 | beat_freq log-interp | wave gating |
| 11 | vocab warp X (ZONE GEOMETRY) | vocab_warp_channel band 17 | MODE_WARP_SCALE = 240 | 17 | 540 | raw·2−1 → [−1,1] × MODE_WARP_AMP | domain warp of mode/style/sparse (AMP = 0 = identity) |
| 12 | vocab warp Y (ZONE GEOMETRY) | vocab_warp_channel band 18 | MODE_WARP_SCALE = 240 | 18 | 540 | raw·2−1 → [−1,1] × MODE_WARP_AMP | domain warp of mode/style/sparse (AMP = 0 = identity) |

### The doors (composite_cell_color L7536 / _biased L7567) — CONTINUITY CLASSIFICATION

Edge constants (all anchored on MODE_DISCRETE_THRESHOLD = 0.70, ROW 4 L1673):
BLEND_EDGE 0.55→0.75 · SCATTER 0.35→0.75 · SPARSE_SURVIVAL 0.22 window 0.35.

| door | decision | verdict | the quoted comparison |
|---|---|---|---|
| BLEND | blend_t = smoothstep(.55,.75,mode); mix | **GLIDE-SAFE** | L7573 `let blend_t = smoothstep(blend_edge_lo, blend_edge_hi, biased_mode);` |
| SCATTER | survival smoothstep, then binary roll | **FLIP** | L7580 `let cell_visible_scatter = s.cell_roll < survival;` |
| STYLE MIX | mix(blend, scatter, style) | glide-safe itself; transmits the scatter flip at weight `style` | L7584 |
| SPARSE | survival roll + hard mode-zone boolean | **FLIP ×2** | L7589 `s.sparse_roll < sparse_survival;` · L7591 `biased_mode > scatter_edge;` |

Tier-cascade cuts in discrete_cell_color — all **FLIP** under any future
moving bias (jitter-dithered chess ±0.015 and mono ±0.075 stagger the pop
front spatially but each cell still pops): chess L1347, colorful L1351
(hard, no jitter — flips coherent areas at once), BW L1373, tint L1379.
The drift tier select `u32(round(config.mode_discrete_tier))` is a FLIP at
every .5 boundary. **Status:** no runtime bias currently reaches these cuts
(tendency and mono are seed-static) — the classification is the design
constraint for the collapse: doors must be glide-safe BEFORE their biases
couple (Treaty: CONTINUITY).

GLIDE-SAFE today and surviving the collapse unchanged: the checker S1 pull,
S2 static wander, S3 variance widening — all continuous in CPU-enveloped
drivers (L1337/1335/1367).

### The glide law (ruling tee-up — the flip fix, choose one)

- **(i) Stateless per-cell fade band (RECOMMENDED):** replace each binary
  roll comparison with a narrow smoothstep around the roll:
  `visible = smoothstep(roll − W, roll + W, survival)` and mix by it. Every
  cell fades in/out over a W-wide band as the bias moves; zero state, pure
  function of (roll, survival); the pop front becomes a dissolve. The
  mode-zone boolean gets the same treatment (smoothstep around scatter_edge).
- **(ii) Hysteresis:** two thresholds (enter/exit) so cells latch — requires
  per-cell memory (a state texture) the pipeline does not have; adds a
  write path and an ordering question per frame. More faithful to "cells
  hold their decision," but stateful and bake-incompatible.
  Recommendation: (i) — it preserves the pure-function law (bake = live at
  rest is provable by substitution) and costs two smoothsteps.

### CellIdentity (draft — ratify shape before Phase 1)

Basis: `struct CellFieldState` (L7467–7477) + everything the doors/cascade
compute internally. Collapsing the doors means identity carries vocabulary +
rolls + continuous weights; color realization moves to one pure resolver.

```wgsl
struct CellIdentity {
    tier: u32,              // 0 full · 1 tint · 2 BW · 3 chess-BW · 4 chess-color
    parity: u32,            // (gx+gz)&1
    cell_roll: f32,         // prop 900 — scatter survival test
    sparse_roll: f32,       // prop 910 — sparse survival test
    bw_roll: f32,           // prop 830 — black/white pick
    color_noise: vec3<f32>, // props 840–842 — spread around region median
    chess_jitter: f32,      // prop 815 (±.015)
    mono_jitter: f32,       // prop 820 (±.075)
    blend_t: f32,           // smoothstep(.55,.75,mode)
    scatter_survival: f32,  // smoothstep(.35,.75,mode)
    sparse_survival: f32,   // smoothstep(thr,thr+.35,sparse)
    style: f32,
    in_mode_zone: f32,      // carry as smoothstep, not bool — the FLIP fix
    region_mean: vec3<f32>, region_variance: f32, region_receptivity: f32,
    region_wander: vec3<f32>,        // band-20 static per-region offset
    chess_color_a: vec3<f32>, chess_color_b: vec3<f32>,
    smooth_color: vec3<f32>, archetype: u32, mode: f32, sparse: f32,
}
```

### C2 findings

- **C2-F1** coupling lattice is live machinery, dead effect: `MODE_COUPLING_MAGNITUDE = 0.0 // DISABLED` (L1755).
- **C2-F2** three hand-inlined Hermite interpolators duplicate lattice_coord/weight — divergence risk on collapse.
- **C2-F4** MODE_SCATTER_FLOOR_EDGE is double-duty (scatter floor AND sparse exclusion floor) — one const, two doors; splitting changes behavior.
- **C2-F5** the composite twins (biased/unbiased) are duplicated bodies — deliberate, flagged at ROW 8 (L1834–1842); the collapse resolves them.

Recipes: `awk 'NR>=7536 && NR<=7599' world.wgsl` · `grep -n '_at_node\|_SPACING' world.wgsl` · `grep -n 'chess_jitter\|mono_jitter\|round(config.mode_discrete_tier' world.wgsl`.

---

## C3. PIGMENT CENSUS

### Vocabularies

| # | vocabulary | authored | consumed | values / source |
|---|---|---|---|---|
| V1–V3 | smooth palette quartet (center/light/weight) | terrain_looks ROW 1 `PALETTE_*_REST` → boot copy → config | palette_color_smooth; palette_weights_at_node; palette_target_color | sand/salmon/green(rare)/warm; weights .42/.28/.04/.26 |
| V4 | region seed colors | discrete_region_at_node props 800–804 | tinted+full tiers; receptivity → DEBUG_VIEW 2 ONLY (C3-F1) | raw hash³ per 80-unit node |
| V5 | chess pairs | chess_color_a/b_at_node (bands 13/14) | colorful-chess tier | raw hash³ — not palette-derived |
| V6 | monos | inline literals at the tier sites | same lines (author = consumer) | 0.03/0.95 · 0.02/0.95 · greys 0.12/0.85 — UNNAMED (C3-F2) |
| V7 | drift targets | REST_MODE_PALETTE_DRIFT_* → config | palette_target_color + at_tier via animated paths | all 0 — DRIVERLESS |
| V8 | checker music resultant | PC_COLOR[12] (visual_canvas.hpp:181–193, Jean's hues, pc 0 = D = red) | checker_region_median → tinted+full tiers; DEBUG_VIEW 1 | 2/8 envelope; rest 0 → seed |
| V9 | GoL zone tint (adjacent) | GoLZoneConfig target_r/g/b — own panel §7.0b by ruling | apply_gol_color (GOL_TINT_STRENGTH .70) | per-zone uniform |

### Tier cascade (discrete_cell_color, first gate wins)

1. chess (`tendency + jitter > .45`) → 1a colorful (`> .65`) pair-by-parity; 1b B&W 0.03/0.95
2. pure-BW (`mono_eff > .35`) 0.02/0.95 by bw_roll
3. tinted (`mono_eff > .20`) mix(grey base, MUSIC-TURNED mean, 0.15)
4. full color: MUSIC-TURNED mean + noise × (seed var + music var)

Note: cascade order is the REVERSE of `_at_tier` id order (0=full … 4=chess-color).

### The seed-pure anchors (the protected set)

chess pair colors (L1352/1414) · chess-BW (L1355/1417) · pure-BW
(L1376/1420) · tinted-tier grey bases (L1385/1424) · smooth palette color
(palette_color_smooth reads only config.palette_*) · the whole bake
(generate_patch_cells passes identity, L7764–65) · the GoL path
(gol_composite_cell_color identity, RULED, L5528–32).

### Color constants and homes

PALETTE_DOMINANT/MINOR_WEIGHT .85/.05 (ROW 3) · PALETTE_COMPLEXITY .5
(pinned; 5 call sites) · door edges (C2) · cascade cuts .45/.65/.35/.20 ·
DISCRETE_TINT_STRENGTH .15 (PINNED couplable literal — GRADUATE candidate) ·
CHECKER_WANDER .12 · CHECKER_VAR_PER_NOTE .025 / VAR_MAX .30 ·
CHECKER_DEBUG_VIEW 0 · GoL: GOL_TINT_STRENGTH .70 (own panel).

### C3 findings

- **C3-F1 (receptivity orphan):** `DiscreteRegion.receptivity` (prop 804) is
  computed and interpolated but has NO live consumer except DEBUG_VIEW 2.
  The struct comment promises "the floor is a ROW 5 dial, applied at the
  mix" — but RECEPTIVITY_FLOOR retired with the wheel and
  checker_region_median never reads receptivity. Recipe:
  `grep -n 'receptivity' world.wgsl` → 1245–1296 + 4132 only.
  Ruling needed: re-wire into the pc-color mix, or retire prop 804.
- **C3-F2 (unnamed monos):** the grey/B&W literals are the only ROW 5-
  jurisdiction cuts NOT promoted to named constants.

### The composite's fate (Phase 1/2 work items)

| function | Phase 1 (doors → FIELD outputs) | Phase 2 (pigment → flat switch) |
|---|---|---|
| evaluate_cell_fields | gains door outputs: blend_t, survivals, tier id | ships tier id instead of eager discrete_color |
| composite_cell_color | shrinks to arithmetic over precomputed doors | collapses to mix(smooth, switch(tier), door); twins merge |
| composite_cell_color_biased | bias moves upstream into field evaluation | deletes as a separate body |
| discrete_cell_color | cascade exits into the tier resolver | deletes; _at_tier becomes sole authority |
| discrete_cell_color_at_tier | unchanged — already the flat switch | THE pigment function (anchors live inside its cases) |
| animated_cell_color | reads baked door outputs | converges with _lut (or deletes now — F10, zero callers) |
| animated_cell_color_lut | door outputs bake into LUT free .w channel (L7779) | one fn; LUT vs live = data-source flag |
| gol_composite_cell_color | its manual field-duplicate collapses to a call | unchanged semantics (identity by ruling) |
| generate_patch_cells | bakes tier id in free .w | bakes switch output; identity law unaffected |

---

## C4. MOTION CENSUS

### Graduated channels

| channel | config field | rest | setter | driver | consumers |
|---|---|---|---|---|---|
| terrain_time | terrain_time | 0 (REST_TERRAIN_TIME — "frozen clock") | set_terrain_time | **DRIVERLESS** (boot pin only) | both overlay evaluators' master gate |
| band_blend 0–5 | band_blend_* | −1 ×6 (inactive sentinel) | set_band_motion | **DRIVERLESS** | get_band_blend → both evaluators (`blend <= 0 → continue`) |
| band_phase_origin 0–5 | band_phase_origin_* | 0 ×6 | set_band_motion | **DRIVERLESS** | `t = terrain_time − origin` |
| pulse ring | pulse_count + pulse_data[8×vec4] | count 0, zeros — NOT paneled (C4-F1) | set_pulse_data | **DRIVERLESS** | contrib_radial_pulses_at |

Historical driver shape (evidence): `backup_board/modules/musical.inl:298–300`
wrote band_motion + terrain_time per frame — the gen-1 coupling.

### Two wave systems, not one (C4-F2)

The `evaluate_*_wave` / `terrain_band_contribution` chain is the LATTICE/BAKE
system — every live call passes literal `t_beats = 0.0` (L2662/2699), so it
is static in practice. The OVERLAY system (contrib_terrain_waves_at /
terrain_wave_overlay_with_gradient) is a separate inline-sine chain over
OVERLAY_WAVES. The charter's MOTION stratum is the overlay system; the
lattice system is GROUND with a dormant time parameter.

### Coherence blessing (CONFIRMED)

FLYER/WALKER physics (pawn_ground_resolve → query_ground_walker_pair;
manifold_height_hf dispatch) and RENDER (patch_terrain_vs + 13 entity/veg
VS sites) all ride the same wave+pulse deformation; the bake
(query_ground_baked_heightfield, generate_patch_heights, sample_terrain_y_at)
stays static truth. Recorded seams if revived: structures don't ride
(L8580–88, RULED); the 13 entity VS sites ride waves only, not pulses (L2508).

### THE BALLAST — sentenced

Recipe: `grep -rn 'wave_enable_mask\|wave_freeze_mask\|wave_frozen\|wave_time_scale' src/ | grep -v backup_board`

| field | reads in WGSL | reads in C++ | verdict |
|---|---|---|---|
| wave_time_scale | 0 | 0 (setter removed; comment state.hpp:2142) | **declaration-only → DELETE** |
| wave_enable_mask | 0 | 0 | **declaration-only → DELETE** |
| wave_freeze_mask | 0 | 0 | **declaration-only → DELETE** |
| wave_frozen_t0/1/2 | 0 | 0 | **declaration-only → DELETE** |

24 dead bytes early in GPUDesignConfig (bytes 16, 40–56). Deleting shifts
every later offset in a 592-byte struct with hard witnesses
(`sizeof == 592` state.hpp:1342; offsetof pins at 124/144/384, recipe:
`grep -n 'offsetof(GPUDesignConfig\|sizeof(GPUDesignConfig) ==' state.hpp`)
plus the raw-offset WriteBuffer calls at state.hpp:1787/1796/1805. A paired
two-room re-lay with witness re-pin — **Phase 3, beside the VOICE re-lay**,
never a casual delete.

### THE CLOCK — ruling teed up for Jean

Today: `set_terrain_time` has one caller (the boot pin, writing 0). Both
overlay evaluators return zero outright at `terrain_time <= 0` (L2755/2788).
Second gate: all blends rest at −1 and the loops `continue` on `blend <= 0`
— motion needs BOTH keys.

- **(a) Run the clock continuously; stillness = blends at rest (RECOMMENDED).**
  Zero visual change at rest (the blend gate already yields exactly 0).
  Activation becomes single-key (a coupling raises a blend). Removes the
  authoring foot-gun where a blend-only driver silently does nothing.
  Cost/architecture: a per-frame terrain_time through the dirty-config
  setter would re-upload 592 B/frame — so (a) implies moving the clock read
  to `signal.t_beats` (FrameSignal already carries it) and retiring
  config.terrain_time's clock role; the ≤0 guard becomes a debug mute.
  The evaluators run their 6-continue loop at rest instead of one compare
  (small, hot-path — measure at Phase 3).
- **(b) Keep time-zero stillness.** Identical visuals today; preserves the
  cheap early-out; retains the clock+enable conflation and the two-key
  foot-gun; any future driver inherits the per-frame-flush problem anyway.

Either way the bake stays static (its literal 0.0 is independent).

---

## C5. VOICE — THE BUS

### Channel table (every live channel the terrain accepts today)

| bus address (proposed) | config field | rest | stratum | driver | identity-at-rest |
|---|---|---|---|---|---|
| voice.color.resultant | checker_resultant[3] | {0,0,0} | PIGMENT | checker coupling (U4) | **YES** — L1337 `mix(seed_mean, music_median, music_amount)` → amount 0 = seed |
| voice.presence.amount | checker_music_amount | 0 | PIGMENT (+FS gate) | checker coupling (U4) | **YES** — mix weight 0 + FS gate skips at ≤.001 (L4124) |
| voice.field.variance | checker_music_variance | 0 | PIGMENT | checker coupling (U4) | **YES** — +0 at 0 (L1367) |
| voice.field.mode_shift | mode_color_shift | 0 | FIELD | **DRIVERLESS** | **YES** — clamp(mode+0)=mode (L7568) |
| voice.field.scatter | mode_checker_scatter | 0 | FIELD | **DRIVERLESS** | **YES** — max(thr−0,0)=thr (L7587) |
| voice.color.drift_target | mode_palette_target | 0 | PIGMENT | **DRIVERLESS** | YES (vacuous — unread while intensity 0) |
| voice.color.drift_intensity | mode_palette_intensity | 0 | PIGMENT | **DRIVERLESS** | **YES** — `drift > 0.001` false → base (L7643) |
| voice.color.drift_tier | mode_discrete_tier | 0 | PIGMENT | **DRIVERLESS** | YES (vacuous — same gate) |
| voice.motion.gol_tick / gol_height | mode_gol_tick_scale / _height_scale | 1 / 1 | GUESTS (GoL jurisdiction — pointer row only) | **DRIVERLESS** | **YES** — pure ×1.0 multipliers |
| voice.motion.band_blend[6] | band_blend_* | −1 ×6 | MOTION | **DRIVERLESS** | **YES** — `blend <= 0 → continue` (L2762/2797) |
| voice.motion.band_origin[6] | band_phase_origin_* | 0 ×6 | MOTION | **DRIVERLESS** | YES (vacuous — behind the blend gate) |
| voice.motion.time | terrain_time | 0 | MOTION | **DRIVERLESS** | **YES** — `<= 0 → return 0` (L2755/2788) |
| voice.event.pulse | pulse_count + pulse_data | 0 / zeros | MOTION | **DRIVERLESS** | **YES** — `count == 0 → 0` (L2847) |
| voice.color.palette[c/l/w] | palette_center/light/weight | ROW 1 rests | PIGMENT | **DRIVERLESS** (setters have ZERO callers; boot writes config directly) | **YES** — rest ≡ pre-graduation literals "bit-identical by construction" |
| (LIGHT — outside the program) | fog_density + fog_color | boot ≡ field-0 entry | LIGHT | fog coupling (U4) | N/A by design — held source, no idle |
| voice.presence.aura_enabled | aura_enabled | boot 1.0 | GUESTS | tick_pawn_couplings (U5) | YES for OFF; boot is ON (presence-ramped, body jurisdiction) |
| voice.presence.aura_height | pawn_aura_height | 0 | GUESTS/GROUND | tick_pawn_couplings (U5) | **YES** — pure ×0 |
| (DEAD — no address) | pawn_amp_scale, pawn_height_bias | 1 / 0 | — | **NONE** (zero setter callers) | **C5-F1: dead BOTH directions** — zero WGSL readers too; padding wearing channel names |

Driverless proof recipe (yields ONLY the boot-pin block):
```
grep -rn 'set_terrain_time\|set_band_motion\|set_mode_color_shift\|set_mode_checker_scatter\|set_mode_palette_drift\|set_mode_gol_scales\|set_pulse_data\|set_palette_center\|set_palette_light\|set_palette_weight\|set_pawn_amp_scale\|set_pawn_height_bias' src --include='*.hpp' --include='*.inl' --include='*.cpp' | grep -v backup_board | grep -v 'src/docs' | grep -v 'realization/state.hpp' | grep -v terrain_looks.hpp | grep -v visual_canvas.hpp
```

### Honest-rename flags (Phase 3)

Config/setter names are honest post-rebuild. The fossils are the BANK PIPES:
`"terrain.checker_mean"` carries a resultant COLOR, not a mean;
`"terrain.checker_var"`[0] carries music_amount — a presence riding a pipe
named var. Only comments hold the meaning. Rename beside the bus re-lay.

### Proposed bus (design sketch — NOT an edit)

Initially an ALIAS TABLE (fields stay physically where they sit; moving
mid-struct breaks the mirror): a `VOICE_LAYOUT[]` table shaped exactly like
`PARAM_LAYOUT[]` (name · config offset · width · rest, rests sourced from
terrain_looks ROWS 1–2) making "boot pin = reset(rests)" one loop instead of
seven calls; one `set_voice(channel, span)` door; the WGSL DesignConfig
block IS the mirror and gains a ROW 2 sub-index naming each field's bus
address; one `[VOICE]` boot witness enumerating every channel with rest +
DRIVEN(coupling)/DRIVERLESS(pinned) status; `pawn_amp_scale`/`pawn_height_bias`
retire or become the first free slots. The physical re-lay (ballast delete +
grouping) happens once, at Phase 3, with the witness re-pinned.

---

## C6. GUESTS

| guest | writes | reads | guard |
|---|---|---|---|
| GoL tint | patch_terrain_fs L4174–78 → apply_gol_color | cell tag alpha, zone_params, zone_life_read, §7.0b consts | 4-deep: tag>.001 → GOL bit → fade>.01 → zone match + color_val>.01 |
| Pawn FF tint | L4181–84 mix toward ZONE_PAWN_TINT | zone_pawn_ff, render_pawn state | **nested inside GoL** and ×color_val (C6-F1) |
| Sphere FF tint | L4187–90 mix toward ZONE_SPHERE_TINT | zone_sphere_ff, floating entity 0 | same nesting (C6-F1) |
| Pawn aura | L4202–14: +aura.gba, +r×0.15 brighten, normal perturb | pawn_aura_read texture, aura_enabled | enabled ≥.5; footprint bounds; active>.01. FS magic 0.15/0.3 unpaneled |
| Radial pulse VISUAL | **NONE** — pulses are HEIGHT-only (patch_terrain_vs L4074) | — | count==0 → 0; DRIVERLESS |

**C6-F1:** the FF tints are not independent guests — they are subordinate
tenants of GoL, tinting only living GoL cells (×color_val). The extrusion FS
applies the same tints UNSCALED (L8139/8145) — an inconsistency to rule on.

### Composition order (as the code stands)

```
0 rim discard → 1 baked color → 2 live LUT recolor (REPLACES)
→ 3 GoL tint → 4 pawn FF (nested) → 5 sphere FF (nested)
→ 6 aura color delta → 7 aura brighten → 8 aura normal perturb
→ 9 shade_lit (fog/veil last)
```

**Verdict: EMERGENT — a ruling for Jean.** The FS has section banners, not
an order declaration. The repo DOES declare order for HEIGHT (THE FOLD,
L3286–88; manifold_overlay_stack ORDER note L3010–16) — the color side has
no equivalent, and color guests do NOT commute (mix vs additive vs replace),
so the order is semantically load-bearing. The charter proposes adopting the
current order as declared law verbatim, written above the FS as the fold
comment is written above manifold_height_hf.

*(RESOLVED Phase 2, D3: ruling 4 declared the order law — THE
COMPOSITION ORDER comment now stands above patch_terrain_fs, verbatim
from the table above. The FF-tint scaling inconsistency (C6-F1) was
harmonized the same commit: the extrusion FF tints scale by the color
spring, gol-enabled verification flagged for Jean.)*

GoL-keeps-its-own-panel ruling recorded at L5528–32, L1859–60, L137–38.

---

## C7. WITNESS

### Existing instruments (selected; full census recipe below)

| instrument | where | witnesses | class |
|---|---|---|---|
| CHECKER_DEBUG_VIEW 0/1/2 | world.wgsl L1729 + L4125–32 | art / resultant meter / receptivity map | permanent, hot-reload |
| [CHECKER] | visual_canvas.hpp:453 | one line per 4-beat decode read | permanent |
| [FLUSH] | cartridge.hpp:786 | FIRST live seam crossing (one-shot — gap 7) | permanent, once |
| [the_board] bind echo | cartridge.hpp:593 | resolved pipes at boot | boot |
| [GPUState] skew beacon | state.hpp:2837 | sizeof(GPUDesignConfig) — stale-binary detector | boot |
| [SPINE] | cartridge.hpp:1561+ | row order + face law (abort on fail) | boot, fail-loud |
| [Hot Reload] / [FileWatcher] / [Renderer] timings | renderer.hpp:1209 / incubator_dual.cpp:209 / renderer.hpp:348 | reload success / change detect / compile ms | permanent |
| [port] | canvas.hpp (was :165) | raw MIDI at the port's mouth | **RETIRED Phase 1 (ruling 8) — re-paste recipe below** |
| [canvas], [SignalLayout], [ParamLayout], [ROSTER], [ROSTER residue], body tags | various | boot + event witnesses | permanent |

Census recipe: `grep -rhoE '"\[[A-Za-z: _-]+\]' src/ --include='*.hpp' --include='*.cpp' --include='*.inl' | sort | uniq -c | sort -rn`

### Retired instruments — re-paste recipes

**[port]** — chord forensics at the port's mouth (retired Phase 1,
ruling 8; verdict served: the parser is innocent). To re-arm, paste
into `Canvas::update` (analysis/canvas_1/canvas.hpp) immediately after
`const int n = port_.poll(beat, ev, 256);`:

```cpp
if (n > 0) {
    std::fprintf(stderr, "[port] beat=%.2f n=%d :", beat, n);
    for (int i = 0; i < n && i < 8; ++i)
        std::fprintf(stderr, " %s ch%d p%d v%.2f",
            ev[i].type == MidiEvent::NOTE_ON ? "on" : "off",
            (int)ev[i].channel, (int)ev[i].pitch, ev[i].velocity);
    std::fprintf(stderr, "\n");
}
```

### Standing verification laws (Phase 2, D5.3)

- **VOICE-COHERENCE** — held chord, camera straddling a patch border:
  the border must be invisible. Patches are windows, music on or off.
  This class is INVISIBLE to silence-identity by definition — the live
  path only runs under music — which is the reason the canon needed a
  second eye. Instrument: the (temporary) tier view, then the art
  itself.
- **PAWN-WALK** — a fixed dune walk in silence as the PHYSICS face of
  silence-identity: the framebuffer watches pixels, the pawn watches
  the manifold. Prediction stated per phase; any deviation is
  diagnostic (see OPEN INCIDENTS #1).
- **glaw2** — the WGSL parse gate, CC-side, every world.wgsl handback:
  Tint CLI or naga (current tool: naga-cli 30.0.0) runs before the
  handback reaches Jean. glaw1 is C++-blind to WGSL and Jean is not
  the parser; the `id` incident is the reason this law exists.

### Gaps (seams with no witness)

1. The bake dispatch — no count/echo at dispatch_generate_patch_cells; a
   patch baking with stale config is invisible.
2. The LUT contents — mode/style/sparse never witnessed post-bake.
3. The door decisions — which cells the gating admits is invisible except as art.
4. The S2 wander/S3 spread actually applied per region — no witness.
5. VOICE at the GPU end — only view 1; no numeric readback.
6. Receptivity — view-only (and orphaned: C3-F1).
7. [FLUSH] is one-shot — the seam runs unwitnessed after the first note.
8. Bake-vs-live identity — asserted in comments, never tested.

### TERRAIN_DEBUG_VIEW registry (proposal)

*(W3.4 status — the registry as it stands after the zone-geometry
work: 0 art · 1 wheel meter (CHECKER 1) · 2 coverage (CHECKER 2) ·
3 skirt paint (TERRAIN 3, permanent) · 4 zone-geometry sculpting room
(TERRAIN 4, permanent: live post-warp mode field + patch border lines
+ red coastline isoline). INCIDENT #2's I1/I2 audits and the Phase-2
tier view retired after their shots. The full single-registry
migration — one const, the layout below — is still Phase 4.)*

| slot | name | shows |
|---|---|---|
| 0 | ART | fold-out (existing) |
| 1 | VOICE METER | resultant × presence (existing) |
| 2 | FIELD COVERAGE | cells taking the live LUT path vs baked (existing) |
| 3 | SKIRT PAINT | perimeter curtain fragments (existing) |
| 4 | ZONE GEOMETRY | live mode field + borders + coastline (existing) |
| 5 | SPARSE SURVIVAL | LUT .b vs survival window |
| 6 | MOTION PHASE | band blend/phase state |
| 7 | GUEST MASKS | wander offset + spread magnitude |

### The two standing invariants as named tests

**SILENCE-IDENTITY** — silence ⇒ pixel-identical to the bake. Today
comment-only ("seam-proof by law", L1655/7761–63; "RESTS are law",
terrain_looks 116–23) plus the structural half: the has_mode_bias door skips
the live path at rest. The UNTESTED half: that the live path at amount→0+
converges to the baked color (the door is a step at 0.001, not a proven
limit). Named test: boot rig, silence, framebuffer hash equals bake hash.

**PROVENANCE** — the magenta probe, kept as a documented one-liner recipe,
never improvised again. Paste immediately above the final return of
patch_terrain_fs (L4216):

```wgsl
return vec4(1.0, 0.0, 1.0, 1.0);  // PROVENANCE PROBE — remove after reading
```

All ground magenta → this FS owns the pixels; some not → another pipeline
draws them; none → the reload chain is cut ([Hot Reload] line absent).
Scoped variant: paste inside the `else if (has_mode_bias)` branch → magenta
marks exactly the live-path cells (doubles as registry slot 3).

---

## C8. THE TREATIES

**BAKE/LIVE** — Per quantity: frozen-at-spawn vs live, declared (the C5
table is the declaration). The cache law: bake = PIGMENT(FIELD, VOICE=rest);
live = the SAME function, VOICE=now; silence ⇒ cache equals recompute, bit
for bit. The baked texture is never a second authority.

**CONTINUITY** — All time-variation enters through CPU envelopes (Segments);
the GPU receives only glided values and static seeds. A stepped GPU-side
hash is a teleport by construction and is banned in this program (the law
already stands in checker_region_median's comment, L1331–36). Doors must be
glide-safe before their biases couple (C2's classification is the work list).

**THE FACE** — Three crossings only: ground queries (manifold_*), the VOICE
setter(s), and the draw. Nothing else crosses the terrain's boundary.

**SOVEREIGNTY** — No shader reads of the analysis signal for color or field
decisions (the §3.1 ruling, restated as this program's article). The
analysis signal reaches the terrain only through VOICE. (Status: holds
today — the checker path's render_signal read was removed at CHECKER-TUNE
A1; recipe: `grep -n 'render_signal' world.wgsl` → height/pulse clock and
FS radial-pulse t_seconds only, no color/field reads.)

**SEAMLESSNESS (the window treaty)** — An effect is ONE continuous
field over (world, time). Frames sample it in time; patches sample it
in space; NEITHER sampling grid may appear in its value. Patch
identity and bake moment are banned inputs to any live quantity. The
geometry precedent: the patch meshes read one heightfield and share
perimeter evaluation — continuity by shared function. The color/field
side achieves the same by pure world-space functions (lattices +
Hermite, global voice values, world-grid cells); the bake side's
guarantee is identity-at-rest — patches baked at different moments
agree because both bake the SAME function at VOICE = rest.
Enforcement: every new effect declares its world-field; per-patch
state, per-patch envelopes, or per-patch random targets are seams by
construction. Pairs with CONTINUITY as the two axes of one law: no
steps in time, no seams in space.
*(Appended Phase 1, per the Phase 1 handoff D6.1.)*

**THE ONE-ADDRESS COROLLARY** (appended Phase 2, D5.2 — the corollary
as it stands in code, §1.3 world.wgsl, verbatim):

```wgsl
// THE ONE-ADDRESS LAW (SEAMLESSNESS corollary — charter C8). A cell
// has exactly ONE address: its world cell index. Every consumer —
// hash, roll, noise, LUT texel, bake write — derives from it. A
// texel is COMPUTED FROM the address; patch_uv never addresses
// anything by itself again.
fn cell_address(world_xz: vec2<f32>) -> vec2<i32> {
    let cs = PATCH_EXTENT / f32(PATCH_CELL_N);
    return vec2<i32>(floor(world_xz / cs));
}
```

---

## C9. DISPOSITIONS + PHASE PLAN

### Dispositions (the TERRAIN-1 M6 pattern)

| tag | items |
|---|---|
| **KEEP** | ground_architecture contract + THE FOLD; all lattices and their seeds; the tier anchors (protected set, C3); the checker coupling (decode, envelope, S1–S3); the boot-pin law; the skew beacon; GoL's own panel (ruling) |
| **MOVE(→FIELD)** | door decisions out of the composite into evaluate_cell_fields outputs (Phase 1); tier resolution out of discrete_cell_color's cascade (Phase 1) |
| **MOVE(→WITNESS)** | CHECKER_DEBUG_VIEW → TERRAIN_DEBUG_VIEW registry (Phase 4); the embedded debug branches → numbered slots |
| **COLLAPSE** | composite twins (biased/unbiased) → one body (Phase 2); animated_cell_color + _lut → one fn (Phase 2); three hand-inlined Hermite interpolators → lattice_coord/weight (Phase 1); gol_composite_cell_color's manual field duplicate → a call (Phase 1) |
| **GRADUATE(literal→dial)** | DISCRETE_TINT_STRENGTH (flagged couplable); the unnamed mono/grey literals → named consts (C3-F2); aura FS magic 0.15/0.3 → panel consts |
| **DELETE** | THE BALLAST wave_* (24 B, Phase 3 re-lay); animated_cell_color if not converged (F10, zero callers); pawn_amp_scale + pawn_height_bias (C5-F1, dead both directions); [port] probe (C7-F1, verdict served); stale header comment `MODE_DISCRETE_THRESHOLD 0.05` at world.wgsl:81 + web mirror (arch-F1) |
| **RULE(Jean)** | the clock law (C4); the flip-door glide mechanism (C2); CellIdentity shape (C2); guest composition order + FF-tint scaling inconsistency (C6); receptivity orphan (C3-F1); MODE_COUPLING_MAGNITUDE=0 lattice (revive or retire); pulse rest not paneled (C4-F1) |

### Phases (each BEHAVIOR-IDENTICAL, verified by SILENCE-IDENTITY + PROVENANCE)

1. **FIELD extraction + door continuity.** CellIdentity lands; doors emit
   continuous outputs; the glide law (as ruled) replaces the flip
   comparisons; Hermite dedupe; gol_composite dedupe. Bit-identical at rest
   by substitution.
2. **PIGMENT unification.** The cascade exits into the tier resolver;
   _at_tier becomes the sole pigment authority; composite twins merge;
   animated pair converges. The bake and the live path provably call one
   function at two moments.
3. **VOICE bus + ballast delete + honest renames.** VOICE_LAYOUT table +
   one setter + [VOICE] witness; the 592-byte re-lay deletes the 24 ballast
   bytes and regroups; witnesses re-pinned; pipe fossils renamed; dead
   channels retired. One commit, two rooms, one witness bump.
4. **WITNESS registry + invariants.** TERRAIN_DEBUG_VIEW slots 0–7;
   [FLUSH] grows a per-N-seconds heartbeat variant; SILENCE-IDENTITY becomes
   a boot-rig test; PROVENANCE stays a documented recipe.

---

## THE SUBSTRATE — RATIFIED HORIZON (STATUS: INTENT — Phase 2, D5.1)

**RULING: stack, not volume.** Truth stays analytic (functions +
caches); DEVIATION gains one body: a single world-windowed, scrolling,
layered resource — height-delta · color-delta · pattern-bias ·
presence — one scroll window, one decay pass, one deposit discipline,
one debug-view family. The aura is recorded as the substrate's first
citizen avant la lettre, not an exception.

**THE WINDOW LAW.** A finite window onto a conceptually infinite
rest-field — scroll-in initializes to rest; effects fade inside the
window radius; toroidal wrap (the aura's proven mechanics,
generalized).

**THE DECAY LAW.** Every layer decays to EXACT zero (snap at epsilon,
never asymptotic); the bake ignores the substrate entirely; silence
therefore still equals the bake, bit for bit.

**ADMISSION DISCIPLINE.** A layer enters like a VOICE channel —
declared address, rest = zero, decay law stated — or not at all.

**STAGING.** Color/pattern/presence layers first (fragment-stage only,
zero FXC exposure); height-delta second, GATED on the feasibility
errand below. Build after Phase 2+3 give it clean insertion points.

**FEASIBILITY ERRAND — receipts (run Phase 2).** Question: does the
aura's HEIGHT contribution texture-sample INSIDE the walker/ground
chain (the FXC-sensitive family), or is it analytic there? Answer:
**YES — samples and compiles.** The chain:

```wgsl
// world.wgsl:5595 (inside sample_pawn_aura)
return textureSampleLevel(pawn_aura_read, bilinear_sampler, aura_uv, 0.0);

// world.wgsl:2784-2786
fn contrib_pawn_aura_at_external(world_xz: vec2<f32>) -> f32 {
    return sample_pawn_aura(world_xz, compute_pawn_pos().xz).r * config.pawn_aura_height;
}
```

`contrib_pawn_aura_at_external` is a declared contributor of
POLICY_FLYER (`world.wgsl:2936`) and POLICY_WALKER_AGENT
(`world.wgsl:3055`) — ground-query policies evaluated in compute
stages — and every build to date is green. This IS the precedent the
substrate height layer rides. One caveat for the height layer's own
ruling: the pawn's OWN standing Y deliberately avoids the grid —
POLICY_WALKER uses the analytic scalar peak `contrib_pawn_aura_at_self()
= config.pawn_aura_height` (world.wgsl:2806-2808) because sampling the
directionally-biased grid at the pawn's own XZ produces locomotion
bobbing (documented at the fn). That is a FEEL ruling, not a compiler
limit — a substrate height layer under the pawn's feet needs the same
self-treatment decision, not a routing workaround.

**FIRST CITIZEN: the out-of-phase checkers.** Bounded cast NOW-able
(VOICE.event): ship events (target, t0); each cell evaluates an
analytic envelope at its own offset — phase from a static world-cell
hash (random mode) or from distance to a pulse origin (traveling-wave
mode). Unbounded cast LATER (substrate): deposits + per-cell decay —
stains that outlive the ring.

---

## THE PIGMENT CONTROL MAP (W3.1 — the decision tree, every knob tagged)

Tags: **S** = STRUCTURAL (redraws geography; regen to see; a ruling to
move) · **C** = COUPLABLE (voice-drivable, glided, rest = identity) ·
**D** = DERIVED (no knobs by law).

**VOCABULARY SELECT**
- zone geometry: lattice spacing **S** (`MODE_LATTICE_SPACING`, ROW 4)
  · warp amp **S** (`MODE_WARP_AMP`, ROW 4 ZONE-GEOMETRY — new, landed
  0) · warp scale **S** (`MODE_WARP_SCALE`, same group) · node bias
  **S** (`MODE_BIAS_EXPONENT`, ROW 4)
- coverage: threshold **S** (`MODE_DISCRETE_THRESHOLD`, ROW 4) ·
  threshold bias **C** (voice.field → `mode_color_shift` /
  `mode_checker_scatter` — driverless today; fade-band prerequisite
  already LANDED at W = 0, ROW 5 DOOR_FADE_W_*)
- transition: blend width **S** (`MODE_BLEND_EDGE_LO/HI`, ROW 5 —
  **RULING PENDING**: Jean's open taste call from INCIDENT #2) ·
  character doors **S** (`MODE_SCATTER_*`, `SPARSE_SURVIVAL_*`, ROW 5)
  · door fade widths **C-adjacent** (`DOOR_FADE_W_*`, ROW 5)

**SMOOTH**
- palette centers/lights/weights **C** (graduated to config, ROW 1 —
  driverless today) · complexity **S** (`PALETTE_COMPLEXITY`, ROW 3)

**CHECKER**
- tier cuts **S** (`CHESS_TENDENCY_CUT` / `CHESS_COLORFUL_CUT` /
  `MONO_BW_CUT` / `MONO_TINT_CUT`, ROW 5) · tint strength **S**
  (`DISCRETE_TINT_STRENGTH`, ROW 5, pinned) · per-tier vocabularies
  **S** (the anchors in discrete_cell_color_at_tier — the protected
  set) · the music wheel **C** (voice.color — LIVE: `checker_resultant`
  + `checker_music_amount` + `checker_music_variance`, ROW 5 dials
  CHECKER_WANDER / VAR_PER_NOTE / VAR_MAX)

**DERIVED** — blend_t, scatter/sparse survivals, the composite:
no knobs by law (door_values + composite_cell_color are arithmetic
over FIELD + the dials above).

---

## ARCHAEOLOGY — the zone-geometry dig (W0, Route verdict: B)

The memory ("this exact gridded-zone look was solved months ago") was
dug before building. **Verdict: ROUTE B — no ancestor mechanism ever
existed; the memory maps to constant-taming.** So the next person
doesn't re-dig:

- **Recipes run**: `git log --oneline --all --follow -- …world.wgsl`;
  pickaxe `-S` over all branches for `warp` / `rotate` / `jitter` /
  `skew` / `mode_field` / `mode_tendency` / `MODE_LATTICE_SPACING` /
  `MODE_DISCRETE_THRESHOLD: f32 = 0.05` / `flooded stage`; full-body
  reads of the mode chain at `1d954e7` (2026-04-04), `bb11a1d`,
  `ca2b93c^`, `ca2b93c`, HEAD.
- **Finding 1 — the field never had geometry machinery.** At every
  point in history the mode field is the same square lattice +
  separable Hermite (lattice_coord/lattice_weight) + quintic node bias
  (`MODE_BIAS_EXPONENT = 5.0`), spacing 120, threshold 0.70. The
  pickaxe `warp`/`skew` hits are orbs/lab code; `rotate`/`jitter` hits
  in world.wgsl are quaternions, per-cell tier jitters (props 815/820,
  still present), and wave seed jitter — none touch the vocabulary
  sampling domain.
- **Finding 2 — the overwrite event was real but lost only a tuning
  excursion.** `297e670`'s own message convicts CHECKER-2's
  `ca2b93c` ("canvas and world") of replacing world.wgsl from an older
  tree. Its mode-region hunk, quoted:

  ```
  -const MODE_DISCRETE_THRESHOLD: f32 = 0.70;       // above → discrete cells
  +const MODE_DISCRETE_THRESHOLD: f32 = 0.25;       // SCOPE (temp): flooded stage for calibration — RESTORE 0.70
  ```

  plus the header stamp `0.70 → 0.05` (the arch-F1 fossil, fixed
  P1-C4). `a5f1aa2` ("CHECKER-2 S1: instruments retired") restored
  0.70. The stride-fix commits themselves (`951faf4`/`3665ed6`/
  `297e670`) are 13–14-line uniform-struct diffs — not a loss event.
- **Conclusion**: the solved look lived in the dials (high threshold +
  quintic bias keeping zones rare and small); the coupling era's
  stronger contrasts un-hid the lattice's native axis-aligned
  anisotropy. The new SYSTEM (the zone-geometry warp, lattices 11/12,
  seed bands 17/18) is the right answer — landed at identity, Jean
  sculpts in view 4.

---

## ARCHAEOLOGY — the per-beat flicker (VOICE.event's ancestor)

**FOUND (historical, removed).** The remembered effect is the CHECKER-1
calibration SCOPE instrument: a LINEAR triangle, period 2 beats, range
[−1,1], amplitude SCOPE_AMP 0.25, added achromatically (same scalar, all
channels, all cells in unison) to the checker mean offset — injected in
`animated_cell_color_lut` (the render-only caller, to dodge the bake
pipeline's binding-200 validation), with companion MUSIC_GAIN and a flooded
threshold. Born `ca2b93c` ("canvas and world"), retired ~3h later `a5f1aa2`
("instruments retired — the calibration protocol served its purpose").
Recipes: `git log --all --oneline -S 'scope_tri'` → exactly those two
commits. Nothing per-beat touches terrain color in the live tree; the live
envelope is smooth 2/8. Adjacent-not-terrain: orb `color_pulse` (dormant,
hardcoded 0). If VOICE.event revives the idea, the ancestor's shape is:
beat-locked additive swing on checker color, injected at the render-only
caller. Note arch-F3: the SCOPE was achromatic and unison — if the memory
is of per-cell or hue flicker, no such effect exists in history.

Residue to clean (DELETE row): the stale `0.05` header comment at
world.wgsl:81 (and its web mirror) — a ca2b93c fossil the retirement missed.

---

## OPEN RULINGS (gathered for Jean)

1. **The clock law** — (a) continuous clock, stillness = blends at rest
   (recommended; implies the clock reads signal.t_beats) vs (b) time-zero
   stillness stays.
2. **The flip-door glide mechanism** — (i) stateless per-cell fade band
   (recommended) vs (ii) hysteresis with per-cell memory.
3. **CellIdentity's shape** — ratify/amend the C2 draft before Phase 1.
4. **Guest composition order** — currently EMERGENT; adopt the current
   order as declared law, or re-order deliberately. Include the FF-tint
   scaling inconsistency (terrain FS ×color_val vs extrusion FS unscaled).
5. **C1 orphans** — the dual-stage residents (aura GROUND∧GUESTS,
   waves/pulses GROUND∧MOTION, patch_terrain_fs four-stage, tag_cell_behavior,
   coupling_terrain_to_sphere_orbit_height) — accept as declared seams or
   split in Phase 1/2.
6. **The receptivity orphan** (C3-F1) — re-wire prop 804 into the pc-color
   mix, or retire it.
7. **MODE_COUPLING_MAGNITUDE = 0** — revive the terrain→mode coupling
   lattice or retire lattice 9.
8. **Housekeeping rulings** — [port] probe removal; pulse rest panel row;
   the state.hpp:5621 "-1 = use activity field" fossil comment.

### RULING LEDGER — all eight CLOSED (Phase 1)

| # | ruling | verdict |
|---|---|---|
| 1 | the clock law | **(a)** — continuous clock; stillness = blends at rest |
| 2 | the flip-door glide | **(i)** — stateless per-cell fade band (door_fade; W dials at ROW 5, landed 0) |
| 3 | CellIdentity's shape | **ratified + amended** — chess_eff/mono_eff ride the struct; receptivity OUT |
| 4 | guest composition order | **declared law** — current order adopted; FF-tint scaling harmonized Phase 2 |
| 5 | C1 orphans | **declared seams** — accepted as-is; F9 re-classed consumer |
| 6 | the receptivity orphan | **retired** — prop 804 out (P1-C4; ID stays reserved) |
| 7 | MODE_COUPLING_MAGNITUDE = 0 | **retired** — lattice 9 out whole (P1-C4; seeds 15/16, props 530/531 stay reserved) |
| 8 | housekeeping | **all approved** — [port] out (re-paste recipe in C7), pulse rest paneled (ROW 2), fossils fixed |

Dated 2026-07-19, stamped by Jean (the Phase 1 handoff).

---

## PHASE 2 — GATE RECORD (S0)

- **Baseline**: commit `b94c99c` (the `id`-redeclaration fix, build
  confirmed green by Jean); world.wgsl sha256
  `21f88f8d94771077b12884012fc211b9986852c6678e1a7186c2b8a8a295f29f`.
- **glaw2 adopted as standing law**: every handback touching
  world.wgsl runs a WGSL parse gate CC-side before it reaches Jean.
  Tool: `naga-cli` 30.0.0 (`naga world.wgsl` — parse + validation).
  Baseline run: green. The `id` incident is the reason; glaw1 is
  WGSL-blind and Jean is not the parser. (Standing-law text lands in
  C7 WITNESS with the Phase 2 amendments.)
- **Drift note (handoff vs tree)**: the Phase 2 handoff describes the
  landed fix as "local → `ci`"; the live tree's fix (`b94c99c`) named
  the local `cell_id`. Same selective rename, same scope; spelling
  differs. Recorded, not blocking — the gate's substance (fix landed,
  module parses) is confirmed by the green build.

---

## OPEN INCIDENTS (Phase 2, D5.4)

### #1 — pawn-climb (dune climbing compromised)

- **Symptoms**: dune climbing compromised — live AND silence;
  suspected walkability threshold. (Jean's ongoing observation.)
- **Suspects**: the step_h/eps literals at the walker_walkable call
  sites; the web-port "stride fix" era. NOT the Phase-1 activity
  dedupe — the pre-P1 body was already Hermite-identical (verified by
  reading at P1-C3).
- **Parked recipes**: the no-rebuild hot-swap bisection vs `d2a045c`;
  `git log -S "step_h"` / `git log -S "walker_walkable"`.
- **Observation protocol**: record WHERE (dune vs pier), music or
  silence, and the approach angle each time it reproduces.
- **STATUS: open — deferred post-terrain by Jean's ruling.** Phase 2
  is grounding-inert by its scope fence (zero edits in the ground
  chain: walker/manifold queries, heights, activity, waves, piers,
  step_h/eps all untouched), so any change in climbing behavior
  across Phase 2 is itself evidence.

### #2 — the border strip (music-only washed band at patch borders)

- **Opened**: with the INCIDENT #2 handoff (post-Phase-2 tree).
- **Evidence** (Jean's canonical pair + walk test): same camera, music
  vs silence — silence SEAMLESS; music shows a full-width horizontal
  band of washed/desaturated cells, ~one cell tall, soft edges. The
  band does NOT follow the pawn — world-anchored, sits on a patch
  border. Aura-delta hypothesis dead (pawn-anchored mechanisms
  excluded). Silence-clean ⇒ the bake agrees with itself; the LIVE
  path disagrees with the bake exactly in a border strip.
- **Suspects**: C1 texel derivation at the edge (OOB → robustness
  zeros → mode 0 → smooth wash) · C2 bake edge-row write · C3
  center-vs-corner registration · C4 the skirt wall · C5 LOD ring
  density (EXCLUDED by R1: rings share extent and CELL_N).
- **Instruments**: TERRAIN_DEBUG_VIEW 1/2/3 (I1 texel audit, I2 LUT
  field audit, I3 skirt paint) — temporary, hot-reload, removed after
  conviction.
- **STATUS: CLOSED (W3.2).** The three instruments exonerated all
  five mechanical suspects: I1 parity clean and continuous across
  borders (C1 cleared), I2 mode continuous and organic at the sampled
  spots (C2/C3 cleared), I3 skirts clean away from the band (C4
  cleared), C5 excluded by R1 (rings share extent and CELL_N). Cause
  RECLASSIFIED — no defect: the band is the blend zone's honest face
  under voice divergence (the live recolor mixes off-wheel smooth
  with pulled discrete inside the blend band, which near-parallels a
  border at the canonical spot). Smooth stays off-wheel by Jean's
  ruling; transition width remains an open STRUCTURAL item in the
  PIGMENT CONTROL MAP (RULING PENDING).
- **Canon caveat (added at INCIDENT #3)**: I1's clean shot was true
  at its camera but the C1 strip is fractions of a cell wide —
  SUB-PIXEL at that distance. Instruments have viewing conditions;
  a clean instrument shot clears a mechanism only at the scale the
  camera can resolve.

### #3 — the border chimera (music-only slivers at patch borders)

- **Opened**: with Jean's pixel-analyzed screenshot — two straight
  world-axis seams crossing at the pawn (a patch corner); border
  cells sliced into thin slivers painted with colors belonging to
  NEITHER neighbor. Music-only; silence clean.
- **Mechanism (convicted)**: a fragment rendered by patch A whose
  world position floors into patch B's first cell — its derived texel
  clamps to A's edge row, so FIELDS (mode/style/sparse/tier) come
  from A's edge cell while HASHES (rolls, noise, chess parity) came
  from B's cell via cell_address(world). Two addresses, one fragment
  — the split brain reborn on a strip only the LIVE path can produce
  (the bake is world-pure, hence silence clean). Sub-pixel at I1's
  camera — see the canon caveat above. Cross-reference: the
  border-strip hunt's C1 fix-shape, now applied.
- **Fix — OWNERSHIP RESOLUTION**: `addr_used = patch_origin_address +
  clamped_texel` in patch_terrain_fs; animated_cell_color_lut takes
  addr_used and derives cell_gx/gz/cell_seed (hence every hash, roll,
  and noise, including inside discrete_cell_color_at_tier) from it.
  Continuous interpolations (palette/chess/region/median) keep the
  fragment's world_xz — continuous fields cannot express a chimera,
  and in-domain fragments stay bit-identical (raw == clamped ⇒
  addr_used == the world floor). Live color path only; bake, ground
  chain, doors, and the parked warp untouched.
- **STATUS: fix landed — closes on the re-shot pair** (Jean's exact
  spot, music playing: the crossing seams gone, border cells whole;
  silence unchanged).
