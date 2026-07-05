# RIBBON COLOR & CELLS — COUPLING DATASHEET (post-CB-1g)
The couplable surface of the ribbon's color system and cell skin, exposed in
contract-datasheet form. Classes per the standing key: L-global (body-wide,
Segment-safe), LH (through the head's history), D (discrete; selection +
state inheritance), C (identity/law; not a live target). Every pipe listed
with its idle, because rest = identity is the safety contract.

Standing rule: this file updates in the SAME COMMIT as any change to the
surface it describes.

## 1. PER-RIBBON PARAMETERS (GPURibbonState; written at commit, static per
##    life today — any per-frame coupling needs the flush seam, noted where)

| field            | meaning                              | class | idle (rest)            | coupling notes |
|------------------|--------------------------------------|-------|------------------------|----------------|
| color[3]         | dark median / uniform color          | L-global | spawn draw (pair.dark, free dark median, or SMOOTH/TINTED draw) | T2's target: gen-2 design = color_stim[3] + color_mix pipes, flush lerp(spawn, stim, mix); mix rest 0 ⇒ spawn exactly. Flush seam LIVE today: the frame conductor uploads color every frame (ribbon.inl, upload_ribbon_color) |
| color_b[3]       | light median (CONTRAST)              | L-global | pair.light + shared jitter, or free light median; median-field species: ≡ color by construction | opens a NEW musical dimension: drive the medians apart/toward ⇒ "contrast" itself as a coupled quantity. Commit-only today (rides the full-struct upload) |
| checker_scatter  | per-cell lightness texture amplitude | L-global | pair.value_var or free draw | texture-energy idiom; pipe as multiplier, rest 1. Commit-only today |
| hue_spread       | the colorful axis, radians [0, π]    | L-global | pair.hue_var(±sib)·π or free draw [0, π] | THE riot dial; pipe as additive deviation, rest 0; needs per-frame flush seam (commit-only today) — ledger item |
| seed             | GPU hash key                         | C     | spawn seed             | identity; never coupled |
| color_mode       | SMOOTH / TINTED / CONTRAST           | D (birth) | seed roll × weights  | population composition; a re-raffle-on-event coupling is possible but is a REBIRTH-class act |

## 2. PAIR-TABLE + FREE-RAFFLE PARAMETERS (CPU console; design-time, authored)
CHECKER_PAIRS rows {dark, light, value_var, hue_var, weight} — C at runtime,
the curated authoring surface.

FREE RAFFLE region spec (CB-1f) — the generative authoring surface beside
the table: FREE_DARK_LUMA/CHROMA, FREE_LIGHT_LUMA/CHROMA (disjoint luma
bands are the one kept law — the chessboard's legibility contract),
FREE_VALUE_VAR, FREE_HUE_VAR (uncapped [0,1]), and the CHROMA_D1/D2
Rodrigues basis (the CPU twin of the shader's hue machinery). The bounds
ARE the lattice; narrow to tame, widen to liberate.

FREE_PAIR_CHANCE (0 = pure curation, 1 = pure generation) is the population
dial and a spawn-time D-class coupling candidate — section-level shifts
between curated and wild.

MEDIAN-FIELD region spec (CB-1g) — the species above both pair paths:
one median raffled in (luma, chroma, hue), color_b ≡ color so the parity
term dies by algebra and the per-cell machinery carries everything —
the terrain-patch grammar on a tube. MEDIAN_LUMA is broad (no parity to
protect); MEDIAN_VALUE_VAR's floor sits higher so cells read through
texture. CELLS_MEDIAN_CHANCE is the species dial (spawn-time D-class
candidate, same idiom as FREE_PAIR_CHANCE).

Two further future D-couplings live here:
- PAIR RE-RAFFLE: a musical event re-rolls a ribbon's pair (state
  inheritance: the body keeps flying; only the skin's medians re-target
  through Segments — the CA-rule-swap grammar, applied to palette).
- WEIGHT MODULATION: mood/section-level shifts of the raffle distribution
  (spawn-time; changes the flock's composition, not existing ribbons).
- CONTRAST COLLAPSE (ledger, CB-1g): the species boundary is the distance
  between two struct fields, so a coupling driving color_b toward color
  dissolves a chessboard into a field LIVE (and back) — contrast as a
  fader, not a category. L-global; needs the color_b wire when its day
  comes.

## 3. SHADER CONSTANTS (world.wgsl, hot-reloadable — scene-level dials)
| const                | meaning                         | class | notes |
|----------------------|---------------------------------|-------|-------|
| CHECKER_CHROMA_DIR   | canonical chroma direction      | C     | basis; also the near-gray fallback direction |
| CHECKER_CHROMA_FLOOR | chroma at full spread (CB-1e, live) | L-global (scene) | "palette punch" — a scene pipe candidate; superseded and replaced CHROMA_GAIN |
Scene-level pipes over shader consts require a small uniform hop when ever
coupled; today they are the hot-tuning loop's knobs.

## 4. PER-CELL EXPRESSIONS (the skin pipeline — what the scalars modulate)
cell_id = k·4 + f (segment × face); caps outside the grid (median A).
  p        = (k + f) & 1                      — parity (the chessboard)
  hue_a    = (hash_hue(cell) − .5)·2·hue_spread — per-cell hue angle
  chroma   = reconstruct: rotate(dir(base), hue_a) · max(|chroma|, spread·FLOOR)
  value    = pole-lerp: mix(base, black|white, |hash_v|·checker_scatter)
Every expression is fixed structure; couplings act ONLY through the §1
scalars — the cells never learn what drove them (the categorical boundary,
per-cell edition).

## 5. CELL STATE (CB-2, pending the idiom fork — the D-class grid)
When the living grid lands: per-cell state buffer (binding 40 reserved),
rule id (ParamShape::Discrete; swap inherits state — Jean's CA principle),
stimulus injection at the head (EVENT-class sources: current_pc, distance),
evolution on the Wagon's beat. State modulates §1's medians per cell
(alive→hot median, age→lerp). Idiom fork open: evolve-in-place (GoL feel)
vs travel-at-P (historian feel); layer one above is idiom-independent.

## 6. RELATED SURFACES ALREADY LIVE OR TAGGED
fog.density / fog.color — the played coupling (all.field, held→table).
ribbon.amp_lateral_mult / ribbon.amp_vertical_mult — PARAM_LAYOUT rows
4–5, LIVE (coupling #2, the pitch compass): multipliers composed over the
spawn-drawn wave amps at the conductor's per-frame flush (rest 1 = the
seed's dance; the pawn mount reads the same mirror — the rider breathes
with the coupled dance for free). Source, per amendment A1 (the Wagon
ruling): "all.window_length" — the Wagon's duration-weighted chroma,
whose X₁ (resultant / center of mass) IS the compass. θ = pc·30°,
multipliers = 1 + GAIN·(cosθ, sinθ), Segment-glided over PITCH_VEC_SPAN;
double-smoothed by construction (the window drains in per-beat stairs,
the Segments glide between them); silence is a two-stage release. The
raw per-note Playhead variant ("all.present_length") stays in the back
pocket, unbuilt.
Terrain palette machinery — DRIVERLESS, scene-level future siblings
(band motion, palette drift, mode color shift) — same grammar, one scale up.
sphere/floater color — DRIVERLESS landing sites from the demolition.

## IDLE MAP SUMMARY (what silence looks like, per pipe)
Every color/cell pipe's rest reproduces the seed-drawn, pair-raffled OR
free-raffled skin exactly. A stranger reading only this table can wiggle
each row on the future panel and predict the screen — that is the
datasheet's test.
