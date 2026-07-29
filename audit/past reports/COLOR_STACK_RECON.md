# RECON B — THE COLOR STACK (how a patch gets its color, end to end)

Application-ordered pipeline + parameter classification + the couplable
surface, judged against the law: **a voice modulates generator parameters,
never decorates output.** Read-only, verified at HEAD `66f582c`. **STOP.**

---

## §0 HEADLINE

Fourteen stages split across a GEN-TIME BAKE (compute, writes the cell-color
texture) and a RENDER-TIME FS (samples it, layers tints, hands to shade_lit).
The couplable surface has TWO tiers: **three `mode_*` uniforms already
plumbed to live upstream consumers but boot-pinned to neutral** (a driver
away — no graduation needed) and **the palette arrays (CENTER/LIGHT/WEIGHT/
VARIANCE): WGSL compile-time consts with no C++ mirror** — the FORK tier.
Premise correction: **archetype currently has ZERO effect on terrain color**
— it never touches palette selection (that's a pure seed roll), and its only
color consumer is the mode-coupling shift, zeroed by
`MODE_COUPLING_MAGNITUDE = 0.0`. Today's wiring is LAWFUL — every driverless
uniform sits upstream of the compositor; the standing output-decoration
layers (aura, zone tints) are structural, not voice-driven — they are where
a careless couple would break the law, not where it breaks today.

## §1 THE STAGE CHAIN (application order; world.wgsl unless noted)

GEN-TIME BAKE — `generate_patch_cells` @7412 → `patch_cell_color` texture:
1. archetype lookup — `evaluate_cell_fields` @7204-05 (`tile_grid_lookup`)
2. palette node selection — `palette_weights_at_node` @986 (cumulative over
   PALETTE_WEIGHT @993-95; dominant → 0.85/0.05 branch @1000-08; **pure
   seed roll** `hash_property(seed, 500u)` @988 — archetype-free)
3. palette field Hermite blend — `palette_field_at` @1022
4. smooth base color — `palette_color_smooth(w, 0.5)` @1206, called @7186
5. discrete color (checker + tiers) — `discrete_cell_color` @1299
   (chess @1301-12; mono/tinted/full @1315-42)
6. mode / style / sparse fields — @1034/@1059/@1082, consumed @7192-94
7. terrain→mode coupling shift — `terrain_coupling_at` @1133 → @7216-20
   (**MODE_COUPLING_MAGNITUDE = 0 → stage is a no-op**)
8. composite smooth↔discrete↔sparse — `composite_cell_color` @7226 →
   texel @7446

RENDER-TIME — `patch_terrain_fs` @3811:
9.  sample baked base_color @3825-30
10. mode-bias re-derive — `animated_cell_color_lut` @7345, called @3843,
    gated `has_mode_bias` @3834-36 (**DRIVERLESS — pins hold it off**)
11. GoL zone tint + GOL_FADE — @3849-3901, `apply_gol_color` @5241
12. pawn/sphere zone tint — @3885-94
13. pawn aura tint — `base_color + aura.gba` @3906-18
14. `shade_lit` @3658 (called @3920) — lighting + fog; **veil icing
    @3684-93 EXCLUDED (the veil's jurisdiction; color ends at base_color
    entering shade_lit)**

## §2 THE PARAMETERS (distribution-shaped vs literal)

DISTRIBUTION-SHAPED (the voice-shiftable class):

| name | value | home | consumer | class |
|---|---|---|---|---|
| PALETTE_CENTER[4] | 4× rgb | :1492 | palette_color_smooth :1210; palette_target_color :7291 | **center/median** |
| PALETTE_LIGHT[4] | 4× rgb | :1498 | :1210 (complexity=0 endpoint) | center (light-end) |
| PALETTE_VARIANCE[4] | .08/.14/.20/.12 | :1504 | `palette_color` :1228 — **DEAD consumer** (only _smooth is live) | spread |
| PALETTE_WEIGHT[4] | .42/.28/.04/.26 | :1510 | palette_weights_at_node :994 | mix-weight (selection prior) |
| discrete region variance | 0.02 + roll·0.23 | :1244 | :1341/:1379 | spread (seed-rolled) |
| complexity | **0.5 literal** ×3 | :7186/:7328/:7356 | palette_color_smooth | mix-weight (LIGHT↔CENTER) |
| mode μ shift | config.mode_color_shift = 0 | state.hpp:395 | composite_cell_color_biased :7256 | **center-shift** |
| sparse scatter | config.mode_checker_scatter = 0 | state.hpp:396 | :7275 | spread/threshold-shift |
| palette drift | config.mode_palette_{target,intensity,tier} = 0 | state.hpp:398 | :7286/:7370-75 | mix-weight (drift) |

LITERAL/STRUCTURAL: MODE_DISCRETE_THRESHOLD 0.70 (:1520); dominant 0.85/0.05
(:1001-07); blend edges −0.15/+0.05, scatter_edge −0.35 (:7228-35); sparse
0.22 (:7244); chess cuts 0.45/0.65, mono cuts 0.35/0.20, tint_strength 0.15
(:1303-33); exponents 5.0/3.0/3.0 (:1521/:1525/:1553);
MODE_COUPLING_MAGNITUDE 0.0 (:1554); lattice spacings (:1518-28);
GOL_FADE 150/300 (:5149-50); ZONE_*_TINT (:1907-10).

## §3 THE ARCHETYPE INPUT (premise corrected)

CPU author: `generate_tile_state` tile_world.hpp:279 — cumulative roll over
`ARCHETYPES[].base_weight` (:43-48) + neighbor influence (:317-330) +
terrain-token priors (:310-315); uploaded @ :266. Color consumers: EXACTLY
ONE — the stage-7 coupling shift (@7216-18), which MODE_COUPLING_MAGNITUDE=0
disables. Palette selection never sees it. **Archetype → color is a wired
channel at zero gain, not a live input.**

## §4 THE COUPLABLE SURFACE (where "a spectrum moves the median" writes)

| parameter | today's home | couplable via |
|---|---|---|
| palette medians (CENTER) | WGSL const, no mirror | **graduation** (FORK tier → uniform) — the ideal couple point: writes stage-4's input, pure generator modulation |
| PALETTE_LIGHT / WEIGHT | WGSL const, no mirror | graduation |
| PALETTE_VARIANCE | WGSL const, no mirror, dead consumer | graduation + revive `palette_color` |
| complexity 0.5 | literal ×3 | graduation (or the Row-3 restore — TERRAIN_COUPLING_LEDGER) |
| mode μ | `set_mode_color_shift` state.hpp:2188 — LIVE setter, live consumer :7256 | **existing uniform — needs only a driver** (boot pin cartridge.hpp:411 + the has_mode_bias gate @3834) |
| sparse spread | `set_mode_checker_scatter` :2191 → :7275 | existing uniform — needs a driver |
| palette drift | `set_mode_palette_drift` :2194 → :7286/:7370 | existing uniform — needs a driver |

## §5 THE LAW CHECK

LAWFUL (generator-parameter modulation, upstream of the compositor):
all three mode_* uniforms (μ-shift @7256; scatter threshold @7275; drift —
which re-runs the FULL compositor and mixes two complete generator passes,
@7336/@7377 — borderline in form, generator-space in substance); and the
graduation targets (palette medians/weights/variance, complexity).

OUTPUT DECORATION (the anti-pattern inventory — structural today, NOT
voice-driven, flagged as the surface a careless couple would break):
- **pawn aura** @3911: `base_color + aura.gba` — additive RGB post-composite;
  the clearest decoration path in the stack. A lawful aura-voice would shift
  a LOCAL palette center instead.
- GoL zone tint (@5241 + GOL_FADE @3853) and pawn/sphere zone tints
  (@3887/93) — post-composite mixes; legitimately those subsystems' own
  visualization, but decoration in form.
- fog/veil (@3676/@3684-93) — view-space, excluded per boundary.

No voice-driven post-multiply exists today. The stack is clean upstream;
the rule for any rebuild: write at §4's points, never at stages 11-13.

---

STOP — rulings held for: which couple lands first (a driver for the three
live mode_* uniforms is the no-graduation path; the palette-median
graduation is the FORK-tier path), and whether the aura/zone decoration
layers get re-homed under the law or stand as subsystem visualization.
