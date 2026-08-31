# CHORD — uniform-seat redistricting by cadence

LOOM stratified the GROUPS by cadence (world / frame / family state /
family textures). CHORD stratifies the BYTES: seats of one cadence and
one author merge into one block, one binding, one upload per beat of
their clock. Scarcity was a symptom of granularity — the program ran
out of seats because it bound objects, not categories.

## The taxonomy
Every datum classifies by three questions: WHEN it changes (cadence),
HOW the GPU touches it (access), WHO authors it (CPU intent / GPU
truth). The cell answers the layout question mechanically. This
document is the taxonomy's record until ORGAN gives it an instrument.

## The blocks (CHORD_1..4)
- agent_room  (g2:1, uniform, C)  = portals + behaviors + tier_gains
  + occupier_amg. 2864 B. Cadence: world/mood. (It carried a second
  occupier window, occupier_cmg, for the column/antenna shafts; that
  window and those families left at PRUNE_2 U4, 6960 B -> 2864 B.)
- field_bus   (g2:9, uniform, C)  = head_poses + ribbon + authored.
  6656 B. Cadence: frame (fastest member governs).
- frame_r     (g1:1, uniform, VF) = lighting + vp + camera + sphere_pos
  (BEQ_A). 1040 B.
  Two instances: main and photographer. vp/camera arrive by
  copyBufferToBuffer from the GPU-sovereign state each frame — the
  CPU never reads them (readback law).
- scene_constants (g2:200, uniform, V) = tier_gains + figure_profiles
  + ribbon. 4336 B. Cadence: world/mood. Bound by scene and shadow.

## Rulings of record
- WINDOWS, NOT HOMES: a fact's home is its one CPU-side struct and its
  one authoring site. GPU blocks are transport windows; tier_gains and
  ribbon-state appearing in two blocks is two windows on one home, and
  the authoring site writes every window it owns.
- CHORD_5 REVERSAL (Jean, 2026-08-16): render_floating returns from
  uniform to read-only storage. The DOMESDAY demotion bought storage
  seats when storage was the famine; post-LOOM the famine moved to
  uniforms, and the 54,912 B block sat at 83.8% of the uniform binding
  ceiling — a wall on entity growth. Storage rows can afford the seat.
  The demotion record in Table C stays; this entry is the reversal's.
- SINGLE PATH, RESTATED: the Pixel offers f16, subgroups, dual-source
  blending, ASTC. Declined — one program, one path, core defaults.
  The cost is named (f16 doubles Valhall ALU) so the choice stays a
  choice.
- DYNAMIC OFFSETS STAY AT 0/8 AND 0/4: offsets solve many-instances-
  of-one-shape; the disease here was many-shapes-of-one-cadence, and a
  struct cures it with no per-draw plumbing.

## Target wallet (prediction; MANIFEST is the truth)
uniform worst 5/12 (agents C, tied by patchgen C) — was 11/12.
storage worst 5/8. All other lanes unchanged.
