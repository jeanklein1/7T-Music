# UG_FIELDS_1 — the zone meets the vocabulary field

## The sentence

A zone is a **rectangular grid of cells running a rule, expressing through a
per-cell factor that is the vocabulary field's verdict, on whichever channels
its variant declares.**

Everything in this campaign is a parameter of that sentence.

| Wanted | Is |
|---|---|
| bigger areas | the grid's dimensions |
| thin and long grids | the dimensions being unequal |
| smooth ground does not extrude | the factor |
| extrusion only in checkered zones | the factor, on the height channel |
| colour-only zone, checkered | the factor, on the colour channel |
| colour-only zone, smooth | the factor inverted, on the colour channel |
| a zone must take part of a checkered region | a threshold on the factor at birth |

Three things the tree lacks: the grid needs a second dimension, the factor
needs a polarity, the zone needs to declare its channels. Nothing else.

## The no-op law

**Every stage lands as a behavioural no-op. New behaviour arrives only when a
tier row asks for it.**

This is the campaign's gate discipline and its proof of unification. If a stage
changes what the rig looks like before any table row changes, the stage has
added a mechanism rather than generalized one, and it goes back. Per-stage
no-op conditions are stated below and are the first thing each visual gate
checks.

## Budget claim (to be confirmed by A0)

Zero new bindings. Zero new kernels. Zero new planes in `zone_life`. The struct
stays 80 bytes.

- `extent: f32` is retired — `zone_derive_params` computes it as
  `grid_size * ZONE_DERIVE_CELL_SIZE`, a cached product of a constant. Two
  sources of truth for one fact, the exact shape that produced the
  UG_REPAIR_1 B tint bug. Its slot becomes `grid_z`.
- `grid_size: u32` becomes `grid_x: u32`.
- `_zpad0` becomes `express: u32` — polarity and channel bits, packed.
- `_zpad1` stays free.
- `GOL_CELL_HEIGHT_FACTOR` is renamed `GOL_CELL_EXPRESS`. Same plane, honest
  name. **Height reads its magnitude — the Gaussian jitter still lives there.
  Colour reads its fact — a `step` on the same value.** One plane, two
  readings.
- Null zones use `transition_fraction <= 0`, already the universal skip gate
  read by the card writer, the tint, sync and evolve. No atomics, no reduction
  pass, no readback.

## Stages

| # | Commit | No-op condition | Gate |
|---|---|---|---|
| A0 | audit (read-only) | — | report only |
| S1 | FS zone match becomes geometric | a zone inside one lattice node tints identically | a zone straddling a node boundary tints across the whole zone |
| S2 | CPU extent + footprint truth | — | spawn density shifts; no visual regression |
| S3 | rectangular grid; texture to 64² | `grid_x == grid_z == old grid_size` | existing zones bit-identical; a hand-authored thin row renders |
| S4 | polarity + channels | default = discrete + both channels | existing zones bit-identical |
| S5 | colour reads express; derive refuses null zones | expressing cells already carry factor > 0 | colour-only variants appear; a zone landing wholly on smooth ground never arms |

S1 and S2 are debt collection — they are latent faults today and should land
whether or not the rest proceeds. S3 and S4 are the generalization. S5 is the
feature.

### S1 — the FS zone match becomes geometric

The tint currently pre-filters by lattice-node equality:
`floor(zp.origin / MODE_LATTICE_SPACING) == floor(world_xz / MODE_LATTICE_SPACING)`.
`MODE_LATTICE_SPACING` is 120 wu and a 32-cell zone is already 100, so a zone
can straddle its own node and fragments on the far side stop finding it. At 64
cells (200 wu) it is guaranteed. The out-of-cell exit is also `break`, which
abandons the remaining slots rather than trying the next zone.

The bounds test that follows *is* the coverage test. Drop the pre-filter, make
the exit `continue`, delete the now-unused `zone_node` and `zn`. This also
removes the FS's only dependency on `MODE_LATTICE_SPACING`, which is what
unblocks S3.

### S2 — CPU extent and footprint truth

`GoLZoneSpawnConfig::ZONE_EXTENT = 100.0f` and `FOOTPRINT_RADIUS = 50.0f`
survive from before U5 made extent tier-derived. A 25 wu zone reserves a 50 wu
radius — sixteen times its area — so the spawn engine rejects neighbours it
shouldn't. With rectangles a circle cannot describe the zone at all.

The CPU already runs `select_tier`, so the true dimensions are in hand.
Footprint becomes a half-extent pair derived from the tier's cells.

### S3 — rectangular grid, 64² texture

`grid_size` splits. Every `y * grid_size + x` becomes `y * grid_x + x`; every
bounds test takes its own axis; the dispatch takes both. `extent` retires and
its readers derive from the grid.

The texture goes to 64², `GOL_ZONE_TEX_N` to 64.0, `GOL_ZONE_CELLS` to 4096,
`GOL_ZONE_STRIDE` to 28672. Buffer ≈ 917 KB, texture ≈ 262 KB at 8 zones.
Trivial in memory; the point of A0 is to confirm it is trivial in *layout*.

`GOL_ZONE_TEX_N` was introduced in UG_REPAIR_1 B precisely so this constant
had one home. It is now load-bearing.

### S4 — polarity and channels

`zone_seed_mask` already multiplies the per-cell factor by
`step(0.5, discrete_visibility_rest(...))`. Smooth ground already refuses to
extrude; the feature is half-shipped and has been invisible because until
UG_REPAIR_2 the extrusions were either absent or a quarter cell off.

Polarity is one `select` on that step. Channels are two bits read by S5. The
tier tables gain an express column, and the three zone types become **authored
rows, not code**.

### S5 — the colour channel and the null-zone guard

Colour is gated at the sim's texture store: the colour spring is multiplied by
`step(0.001, express)`. Height needs no change — `contrib_gol_zones_at`
already multiplies by the plane.

`zone_derive_params` samples the field across the zone's rectangle at birth and
leaves `transition_fraction` at zero if the zone would express nothing.

## Explicitly out of scope

**Interconnection.** Ruled out. Zones stay toroidal and sealed; the evolve
kernel's `(cell + d + gs) % gs` is untouched. The honest reason it is awkward
is that life is stored per zone rather than per world cell, and that is a
Layer E question, not this campaign's.

**Layer E.** The masks here are frozen at birth. Under E they would become live
reads and a zone would re-shape as the field warps. The interface is the same
either way — a per-cell scalar the zone consults — so E converts frozen to live
without touching this framework. Running this campaign first also gives E its
first real customer, which is better than starting it against a blank canvas.

**Cut C, the tile card.** Orthogonal. `tile_grid` feeds the archetype into the
tier cascade, but making it a texture changes no answer the mask gives. It is
the right thing to run *while* this campaign sits in gate limbo.

## Open rulings

1. **Texture size.** 64² fixed, or size-classed layers? 64² costs 262 KB.
   Recommend fixed — a size class is a second source of truth.
2. **Aspect ratio floor.** Is 8 × 64 (25 × 200 wu) allowed, or is there a
   minimum short side? Affects tier authoring, not code.
3. **`extent` retirement.** Recommend yes. The alternative is a second extent
   field, which reinstates the redundancy.
4. **Express word layout.** Proposed: bit 0 polarity (0 = smooth, 1 = discrete),
   bit 1 height channel, bit 2 colour channel. Room for five more.
5. **Arming threshold.** At least one expressing cell, or a fraction? A
   fraction needs a count; "at least one" needs only a short-circuit.
6. **Colour-only on smooth ground** imposes a visible 3.125 wu cell grid where
   none exists. That is a real aesthetic difference — a grid on smoothness
   rather than a pattern inside an existing grid. Confirm that is the intent.
7. **Tier tables:** new rows, or express columns on existing rows?

## What A0 gates

If a render- or compute-side layout is at its cap such that any part of this
forces an eviction, D-full stops being optional and comes first — the migration
would be paid for anyway. Otherwise S1 proceeds immediately.
