# CURTAIN_REPORT — the slab walls and the shadow map

Campaign CURTAIN_1. Read at `b8708ae` (`CELL_1 rev2: the comments learn the stride-2
ring and the prefix`), which is `origin/master` at the time of this read.

**Register:** report-only. No code was edited by K1. Two of the three code handoffs
STOP; the reasons are below and the arithmetic is shown so Jean can rule without a
build.

**Disposition summary**

| handoff | state | why |
|---|---|---|
| K1 | **landed** | This document. |
| K2 | **HELD — STOP fired (K1-c)** | The addition is **+177.8 %** of the shadow pass's current terrain triangle count. The threshold is 40 %. The ruling is Jean's. |
| K3 | **HELD — STOP on mismatch** | K1-e finds a *fragment*-level discriminator but **no vertex-level one**, and the one comment that would make the fragment path safe contradicts the emission it describes. Named below; unresolved. |
| K4 | **landed** | UMBRA_3's ledger row amended; the general form filed as a PROCESS_LAWS candidate. |

---

## 0 — THE TREE MOVED, AND IT MOVED THROUGH THIS EXACT GROUND

The handoff warned that the tree had moved since PENUMBRA_3 and instructed that every
quoted fact be verified at HEAD. It had moved, and the movement is not incidental to
this campaign — it is *inside* it. Three commits landed on 2026-08-01 from another
session, hours before this handoff was read:

| commit | date | what it did |
|---|---|---|
| `21f2a66` | 2026-08-01 | `CELL_1 rev2: the ring goes stride-2 and the curtains move to the tail` |
| `bacc1a5` | 2026-08-01 | `CELL_1 rev2: the shadow pass takes the clean prefix` |
| `b8708ae` | 2026-08-01 | `CELL_1 rev2: the comments learn the stride-2 ring and the prefix` |

`bacc1a5` is an ancestor of HEAD (verified with `git merge-base --is-ancestor` in the
same command sequence as a `git fetch origin`, per P9).

**Three premises in the handoff are stale at HEAD.** None of them changes the
*artifact*; all of them change where the fix lives and what it costs.

1. **"the LOD1 IB […] emits no curtain band at all"** — false at HEAD. Since
   `21f2a66` the ring (LOD1) index buffer emits a curtain band of its own, as a
   contiguous **tail**. See K1-a.
2. **"The shadow pass draws both terrain bands through `patch_index_buffer_lod1()`"**
   — still true of the *buffer*, but the accessor named no longer exists.
   `patch_index_count_lod1()` split into `patch_index_count_ring_clean()` /
   `patch_index_count_ring_zoned()`, and since `bacc1a5` both shadow bands take the
   **clean** count. See K1-b.
3. **K2's design — "bind the LOD0 index buffer and issue one `DrawIndexed` over the
   curtain band"** — the right band is no longer in the LOD0 buffer. It is in the
   buffer the shadow pass **already has bound**, immediately past the count it
   already passes. K2's shape gets *simpler*; K2's cost gets much worse, because the
   ring's curtain band is 8 quads per cell against a cap band of only 4.

**The diagnosis itself survives verification intact.** The main pass and the shadow
pass do draw different meshes, and the difference is precisely the walls:

- Main pass, `render_passes.hpp:463-467`, plan slot C, index buffer
  `patch_index_buffer_lod1()`, index count from `reset_frustum_indirect`'s slot C =
  `patchIndexCountRingZoned_` — **19,200 indices, curtain tail included**.
- Shadow pass, `render_passes.hpp:396-409`, same buffer, count
  `patch_index_count_ring_clean()` — **6,912 indices, curtain tail excluded**.

One buffer, two counts, and the 12,288 indices between them are the walls.

---

## K1-a — THE BAND

**The curtain band is contiguous.** It is the tail of the ring (LOD1) index buffer.
`state.hpp:3419-3488`, one emission, in the order caps → skirt → curtains, with the
clean count recorded at the boundary (`:3462-3463`) and the zoned count at the end
(`:3481`):

```cpp
                    // caps + skirt is a usable prefix: the clean count stops here
                    patchIndexCountRingClean_ = (uint32_t)idx.size();
                    // curtains: the stride-2 perimeter walk, cap vert to base twin
                    for (uint32_t cell = 0; cell < Dim::UG_CELLS_PER_PATCH; cell++) {
```
```cpp
                    patchIndexCountRingZoned_ = (uint32_t)idx.size();
```

**First index and index count**, derived from the emission with `Dim` at HEAD
(`UG_CELLS_PER_PATCH` 256, `UG_QUADS_PER_CELL` 4, `UG_BASE_VERTS_PER_CELL` 16,
`PATCH_MESH_N` 64, so `SKIRT_RING` 256 and the stride `s = 2`):

| band | emission | indices | running total |
|---|---|---|---|
| caps | 256 cells × 2×2 quads × 6 | 6,144 | 6,144 |
| skirt | 256/2 = 128 segments × 6 | 768 | **6,912** = `RingClean_` |
| **curtains** | 256 cells × (16/2 = 8) quads × 6 | **12,288** | **19,200** = `RingZoned_` |

- **curtain first index = 6,912**
- **curtain index count = 12,288**

Both totals are independently corroborated by `21f2a66`'s commit message, which
reports 6,912 and 19,200 from compiling the edited emission standalone.

**Accessors already exist for both**, beside each other at `state.hpp:2679-2680`:

```cpp
            uint32_t patch_index_count_ring_clean() const { return patchIndexCountRingClean_; }
            uint32_t patch_index_count_ring_zoned() const { return patchIndexCountRingZoned_; }
```

The curtain band's first index **is** `patch_index_count_ring_clean()` and its count
**is** `ring_zoned() - ring_clean()`. K2 would need no new accessor, no new buffer,
no new bind, and no shader change — it is one `DrawIndexed` per band against the
index buffer the pass has already bound. The handoff's design intent is fully
available; only its cost blocks it.

*(`patch_index_count_lod1()` no longer exists. Any handoff still naming it is
pre-`21f2a66`.)*

---

## K1-b — THE DRAWS

Both call sites verbatim, `render_passes.hpp:395-410`:

```cpp
    if (cast_terrain) {
        c->renderer_.draw_shadow_patch_terrain(
            pass,
            c->gpuState_.render_entity_group(),
            c->gpuState_.shadow_texture_group(),
            c->gpuState_.patch_index_buffer_lod1(),
            c->gpuState_.patch_index_count_ring_clean(),
            c->world_state_.lod0_patch_count
        );
        if (c->world_state_.render_patch_count > c->world_state_.lod0_patch_count) {
            // Band 1 — same IB, already bound by the band-0 helper; the
            // redundant re-bind collapsed (trivially adjacent).
            pass.DrawIndexed(c->gpuState_.patch_index_count_ring_clean(),
                c->world_state_.render_patch_count - c->world_state_.lod0_patch_count, 0, 0, c->world_state_.lod0_patch_count);
        }
    }
```

And the helper, `renderer.hpp:982-995`:

```cpp
            void draw_shadow_patch_terrain(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount,
                uint32_t instanceCount
            ) {
                pass.SetPipeline(shadowPatchTerrainPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
                pass.DrawIndexed(indexCount, instanceCount);
            }
```

**Instancing, both bands.**

| band | indexCount | instanceCount | firstIndex | baseVertex | firstInstance |
|---|---|---|---|---|---|
| 0 | `ring_clean()` = 6,912 | `lod0_patch_count` | 0 (default) | 0 (default) | 0 (default) |
| 1 | `ring_clean()` = 6,912 | `render_patch_count - lod0_patch_count` | 0 | 0 | `lod0_patch_count` |

The two bands **differ in instancing** — band 1 carries a nonzero `firstInstance`. Per
K2's own rule ("if bands 0 and 1 differ in instancing, the curtain draw is issued
**per band**, mirroring each"), K2 would be **two** `DrawIndexed` calls, not one.

**Which buffer is bound where.** `draw_shadow_patch_terrain` binds
`patch_index_buffer_lod1()` — the ring buffer — via `SetIndexBuffer`. Band 1 reuses
that binding without re-issuing it; the comment at the site says so and it is correct.

**Is the LOD0 IB bound anywhere in the shadow pass today? No.** The only binds of
`patch_index_buffer()` and `patch_index_buffer_cap_only()` in the tree are
`render_passes.hpp:456` and `:461` (main pass, plan slots A and B), and
`patch_index_buffer_lod0_live()` at `gallery.hpp:1275` (snapshot pass). The shadow
pass never binds a LOD0 index buffer. Sole terrain caster path is the one quoted
above; the drawable table's ten shadow rows bind their own mesh buffers.

**Both shadow draws are direct.** `DrawIndexed`, not `DrawIndexedIndirect`. The only
`DrawIndexedIndirect` in the renderer is `draw_patch_terrain_plan_slot`
(`renderer.hpp:719`), which the main pass uses and the shadow pass does not. So K2's
FXC premise holds: its draw would be direct, against an existing pipeline and existing
bind groups.

**On the FXC banner (reported because the handoff instructed a STOP on mismatch).**
The handoff says to read "the banner" and expects it to name *extra indirect draws* as
a hang risk. **The banner is not in `world.wgsl` at HEAD, and three live pointers
still address it.**

Searched untruncated per P11: `grep -n -i "fxc" world.wgsl` returns exactly 12 lines
(14, 2395, 2935, 3029, 3784, 6709, 6769, 7501, 7505, 12364, 12416, 12637). Every one
of them is either the file header's pointer to `LAWS.md` L2 (line 14) or an inline
note at its own site. **There is no banner block.** Yet the tree still speaks of one:

- `world.wgsl:2397` — *"a fn returning a constructed struct is not the runtime-indexed
  const array **the banner forbids**."*
- `world.wgsl:6769` — *"nothing inside the ground-resolve chain (**the FXC sanctum**)."*
- `PROCESS_LAWS.md`, closing section — *"FXC's **behavior** (the `world.wgsl`
  **banner's hang cliff**) is unchanged by build configuration."*

And L2, which claims to delegate the specifics to that banner — *"the operational home
of the specifics is the world.wgsl FXC banner"* — enumerates four of its own: lean
byte-pinned instance structs, no new runtime branching in the collision/ground chain,
no texture-array stamps in that chain, and 10 storage / 12 uniform buffers per stage.
**No runtime-indexed const array. No hang cliff. And no draws of any kind, indirect or
direct.** So the constraints the three pointers attribute to the banner are homeless at
HEAD: the pointers survived the block.

**This does not block K2** — the banner does not say otherwise about indirect draws, it
says nothing, and K1-b establishes independently that both shadow terrain draws are
direct `DrawIndexed` against an existing pipeline and existing bind groups. But the
handoff asked to be told, and a dangling pointer to a deleted authority on a
*hang* class is worth more than a footnote. Filed as R11-class, unchased: whether the
banner was deleted or never migrated is a question for whoever owns L2, not for this
campaign.

---

## K1-c — THE COST — **THIS IS THE STOP**

**Per patch instance:**

| | indices | triangles |
|---|---|---|
| shadow terrain today (`ring_clean`) | 6,912 | **2,304** |
| curtain tail (`ring_zoned - ring_clean`) | 12,288 | **4,096** |
| after K2 (`ring_zoned`) | 19,200 | 6,400 |

**Patches drawn per band.** Band 0 = `lod0_patch_count`; band 1 =
`render_patch_count - lod0_patch_count`. Both draw the same index count from the same
buffer, and K2 mirrors the curtain draw onto both — so the ratio is **invariant to the
instance counts**. Whatever the frame's patch counts are, the arithmetic is the same:

> **added triangles = 4,096 / 2,304 = 1.7778 → +177.8 % of the shadow pass's
> current terrain triangle count.**

**The threshold in the handoff's STOP condition is 40 %. This is 4.4× over it. K2 is
HELD.**

**Absolute, for scale.** `render_patch_count` is the patch set inside the veil ring
(`VEIL_RING_DEFAULT` = 325 wu, `PATCH_EXTENT` = 50 wu), banded at
`LOD0_RADIUS_DEFAULT` = 175 wu. Ring area gives ≈133 patches drawn (≈38 of them in
the LOD0 band); the set is patch-granular so the true count runs a little above the
disc estimate — call it ~140, and treat the absolute as an estimate while the *ratio*
above is exact. On that basis the shadow pass's terrain today is ≈323,000 triangles
per frame and K2 would add ≈573,000, taking it to ≈896,000.

**Two refinements that matter to the ruling, both in K2's favour but neither enough.**

1. **Vertex work grows less than primitive work.** The curtain tail's *top* verts are
   cap-band verts the cap draw already shades: the stride-2 perimeter walk visits
   local offsets 0,2,4,…,14, all of which lie on the 3×3 cap lattice
   `{0,2,4}×{0,2,4}` the cap quads already use. The only genuinely new vertices are
   the 8 base-band twins per cell — 2,048 per patch against 2,432 already touched.
   So **VS invocations rise ≈84 %** while **primitives rise 177.8 %**.
2. **Every added triangle is degenerate outside a lifted zone** (K1-d). The added
   cost is index fetch, primitive assembly and vertex shading — not fill. The shadow
   VS is not free, though: per vertex it does one heightfield `textureSampleLevel`,
   one `sample_live_card`, one `ug_cell_lift` card sample, and
   `pawn_gol_suppression`.

**The STOP is robust — it is not an artifact of the tail's density.** The cheapest
band that can seal a slab is one quad per cell side, which is what CELL_1's *original*
ring emitted (`89b63a3`: 1,024 corner-curtain quads per patch). That band is 6,144
indices = 2,048 triangles = **+88.9 %** on today's 2,304. Even the minimum sealing
geometry is more than twice the threshold. There is no cheaper shape of this fix
inside the current representation, so "make the curtain coarser" does not rescue K2 —
it only moves it from 4.4× the threshold to 2.2×.

**What this leaves for Jean.** The handoff is explicit that past this fraction the
ruling is Jean's, not the handoff's. The three live options, stated without
recommendation because the ruling is not mine to make:

- **Accept the cost.** One line reverts it (`ring_clean` → `ring_zoned` at both
  sites), and it is the smallest possible diff — it is literally the `bacc1a5` commit
  read backwards. Geometry is the frame's known bottleneck; the meter row is
  `meter_row::ShadowPass`.
- **Accept it only where it shows.** Band 0 is the near band (inside `lod0_radius`,
  ≈38 of ≈133 patches). Detachment is a near-field artifact — at distance the whole
  slab is under a texel. Curtains on band 0 alone is ≈+50 % on the *whole* pass rather
  than +177.8 %. This is a new design, not K2, and belongs to a handoff with its own
  gate.
- **Leave it.** The artifact is real and is not going away on its own.

---

## K1-d — DEGENERACY

**Confirmed: a curtain quad is exactly zero-area when its cell is unlifted.**

The quad welds cap vert `a = cap0 + lz*UG_CAP_STRIDE_C + lx` to base twin
`sa = base0 + k`, where `cell_perimeter(k)` produced `(lx, lz)`. In `ug_decode`
(`world.wgsl:300-350`) both decode to the **same** `(vx, vz)`, therefore the same
`uv`, therefore the same `height_data.x` and the same `sample_live_card`. They differ
in exactly two derived fields:

```wgsl
    d.lift_scale = 1.0 - d.wall;
```

with `d.wall = 1.0` on the base band and `0.0` on the cap band, and `d.drop` left at
`0.0` for the base band (`PATCH_SKIRT_DEPTH` is set only on the skirt-ring branch).
The single line that separates them, identical in both terrain VS:

```wgsl
    world_pos.y += lift * d.lift_scale - d.drop;
```

At `lift == 0` the cap vert and its base twin are bit-identical in world position, so
the quad has zero area and is culled at primitive assembly. The separation between the
two rows *is* the lift, which is why the band is the right geometry and not merely
adjacent to it.

**Fraction of cells lifted in a typical frame: no census measures it, and none was
added.** The censuses in the tree (`spawn_engine.hpp`, `dump_entity_census`,
`dump_agent_census`) count entities and agents, not lifted ground cells. The nearest
live instrument is `zone_rects_in_core()` (`cartridge.hpp:1640-1652`), whose witness
prints the count of zone rects reaching the LOD0 core on change — a rect count, not a
cell count.

What can be said without measuring, from constants already in the tree:

- A cell can only lift inside an active GoL zone rect; everywhere else
  `sample_live_card_gol` returns 0 and every curtain quad there is degenerate.
- `GoLZoneSpawnConfig::SPAWN_CHANCE = 0.60f` of checkerboard zones spawn
  (`gol_zones.hpp:102`), and the delivered height-enabled rate is **0.7845**
  (`gol_zones.hpp:103-110`, with the recipe shown at the constant).
- Within a height-enabled zone, the lifted fraction is whatever the GoL population is
  that tick, which nothing in the tree records.

So the lifted fraction is small and bursty, and the added triangles are overwhelmingly
degenerate — which is the strongest argument *for* K2's cost and still does not get it
under 40 %, because the threshold is stated in triangles and the triangles are real
whether or not they cover pixels.

---

## K1-e — THE WALL NORMAL

### The P1-B finding is confirmed verbatim at HEAD

`world.wgsl:4552`, in `patch_terrain_fs`:

```wgsl
    var normal = normalize(vec3(-in.gradients.x, 1.0, -in.gradients.y));
```

`world.wgsl:4508`, in `patch_terrain_vs`:

```wgsl
    out.gradients = height_data.yz + live.yz;
```

`gradients` carries **no lift term**, and `lift` moves only `world_pos.y`. So a lifted
cell's cap *and* its vertical curtain walls carry the unlifted terrain's normal, which
points up. On a vertical face that is wrong by ~90°, and it is what makes the slab
sides read at nearly the brightness of their tops in Screenshots 94 and 95. P1-B
stands.

### Is there a discriminator? — **at the fragment, yes; at the vertex, no**

**No vertex-level discriminator exists, and this is structural rather than an
oversight.** `d.wall` is 1 only on the curtain-**bottom** twin. The *top* two verts of
every curtain quad are cap-band verts — not copies of cap verts, the **same indices**
the cap quads use:

```cpp
                            uint32_t a  = cap0 + lz * Dim::UG_CAP_STRIDE_C + lx;
                            uint32_t b  = cap0 + lz1 * Dim::UG_CAP_STRIDE_C + lx1;
                            uint32_t sa = base0 + k;
                            uint32_t sb = base0 + k1;
```

They are welded — that weld is what makes the curtain seal instead of gap. A
per-vertex normal cannot be given to a wall top without giving the identical normal to
the cap surface that shares the vertex. **K3's stated edit — "give curtain-wall
vertices the outward horizontal normal" — is therefore not expressible without
splitting those verts**, and the handoff itself rules vertex-splitting out: *"a
geometry change, not a normal fix, and it belongs to a campaign with its own gate."*

**A fragment-level discriminator does exist, and it is already plumbed.**
`PatchTerrainVarying` carries `d.wall` to the FS, smoothly interpolated:

```wgsl
    @location(4) skirt: f32,
```
```wgsl
    out.skirt = d.wall;   // the INCIDENT-#2 instrument, generalized — 1 on
                          // curtain-bottom + skirt copies (wall fragments
                          // interpolate toward 1)
```

It is exactly `0.0` across every cap fragment (all four verts of a cap quad are
cap-band) and ramps `0 → 1` top-to-bottom across every curtain-wall and skirt
fragment. The FS already reads it with a threshold at `world.wgsl:4623`:
`if (in.skirt > 0.01)` (DEBUG_VIEW 3, the skirt paint). By the handoff's own wording —
*"any vertex attribute **or derivable quantity**"* — one exists, so K3 is not dead on
K1-e's face. It is dead on what follows.

### Why K3 still STOPS — a mismatch at the site K3 must read

The outward normal cannot be recovered from "this is a wall" alone. The wall face for
cell *(cx,cz)*'s top edge and the wall face for cell *(cx,cz+1)*'s bottom edge are the
**same plane**, and both cells emit a curtain there; the outward direction is opposite
for the two, so the fix must know **which cell owns the quad**. The only carrier of
that fact is:

```wgsl
    @location(5) @interpolate(flat) cell_local: vec3<u32>,
```
```wgsl
    out.cell_local = vec3<u32>(d.cellx, d.cellz,
                               select(0u, 1u, vi >= UG_CAP_BASE));
```

For a **curtain** quad this is sound: all four verts come from one cell (`cap0` and
`base0` are both derived from the same `cell`), so `cell_local.xy` is that cell and
`.z` is 1 whichever vertex is provoking. Given it, the arithmetic is pure and cheap —
`f = in.patch_uv * PATCH_CELL_N - vec2<f32>(in.cell_local.xy)` lands in `[0,1]²`, a
wall fragment pins one component to 0 or 1, and the outward normal is `(∓1,0,0)` or
`(0,0,∓1)` accordingly. No new binding, no new varying, no branch in the sampled
chain. That is K3, and it would work.

**The mismatch is on the other user of the same test: the patch skirt.** Skirt
fragments also carry `skirt > 0`, so the fix must exclude them, and `cell_local.z` is
the intended exclusion. Its declaration says so:

```wgsl
    // cell, patch-local, decoded in the VS. z = 1 on the cap and base
    // bands, where every vertex of a primitive shares one cell so flat is
    // exact; 0 on legacy and skirt, whose quads straddle cells and whose
    // fragments keep the world floor.
```

**The emission contradicts that comment.** A skirt triangle's first vertex is
`a = skirt_cap_index(k)`, and `skirt_cap_index` returns
`Dim::UG_CAP_BASE + …` — a **cap-band** index, so `vi >= UG_CAP_BASE` and the VS
writes `z = 1`, not 0. Under flat interpolation from the first vertex, every skirt
fragment would report `cell_local.z == 1u` — the opposite of what the comment promises
and of what K3 needs.

I did not chase which side is wrong. Resolving it needs either a verified reading of
Dawn's provoking-vertex convention for `@interpolate(flat)` against this emission, or
a new discriminator — and a new varying is exactly the "new binding" K3 forbids. Two
things follow:

- **K3 STOPS**, under the campaign's own rule: *verify every named anchor verbatim
  before editing; STOP on mismatch.* Building the normal fix on a contested test would
  risk silently re-normalling the entire patch skirt ring — a change no gate row in
  this campaign names, on geometry (WALL_1's rim curtain) that is not this campaign's
  business.
- **The finding carries to HORIZON**, as K3's text provides for, with the arithmetic
  above already worked out so the next handoff starts from a design rather than a
  question.

**The one experiment that unblocks K3**, for the record and costing one frame: HEAD
already ships the instrument. DEBUG_VIEW 3 paints `in.skirt > 0.01` magenta. A
sibling read of `cell_local.z` on those same fragments — or simply noting whether the
existing DEBUG_VIEW 4 / colour-ownership path shows skirt fragments re-homing per
`owned_texel` (`world.wgsl:4577-4578`, the only consumer of `cell_local.z` today) —
settles it without adding anything permanent.

### Adjacent, seen and not chased

`shadow_patch_terrain_vs` omits the pawn aura that `patch_terrain_vs` applies
(`world.wgsl:4486-4487`, `aura.r * config.pawn_aura_height`). Caster and receiver
surfaces therefore differ under the pawn's footprint. This is a second main/shadow
mesh divergence, unrelated to curtains, not diagnosed here, and no recon is authorized
for it by this campaign.

---

## K1-f — PROVENANCE — **the artifact is newly CAUSED, and it is hours old**

**When ECONOMY_1 E2 pinned the shadow terrain to LOD1:** `c5e95db`, 2026-07-29,
*"ECONOMY_1 E2: shadow pass draws terrain at LOD1 density (HELD — Class II argued,
gate binding)"*, merged at `8804f0c`. (Note for future archaeology: `c5e95db` is **not**
an ancestor of HEAD — the repository was re-imported at the orphan root `2a66822`, so
E2's *content* is in HEAD while its SHA is not reachable. Verified with `git fetch` +
`git merge-base --is-ancestor` in one sequence, per P9. Any `git log -S` run against
this tree will return `2a66822` for essentially every string; it is the import, not a
change.)

**Has the GoL lift height or the zone activation rate changed since? No.**
`git diff c5e95db HEAD -- bodies/gol_zones.hpp` is 11 insertions and 1 deletion, and
every one of them is ECONOMY_1 E1 rev2's zone world-footprint persistence
(`corner_x/corner_z/extent_x/extent_z` on `GoLZoneState`, written in `commit_gol`)
plus one comment. `SPAWN_CHANCE`, `HEIGHT_CHANCE`, the height-factor constants,
`alive_height_mean/sigma` and `mode_gol_height_scale` are all untouched.

**So the lift did not change. The caster did — three times, and only the last one
produces the artifact.**

| era | shadow terrain caster | a lifted cell casts | detached? |
|---|---|---|---|
| ECONOMY_1 E2 → CELL_1 (07-29 → 07-31) | legacy interior grid at `step = 2` + coarse skirt ring, 6,912 indices | a flat top **flanked by ramps** — boundary quads straddle two cells, so lift interpolates across the seam | **no** — the caster is connected to the ground |
| CELL_1 `89b63a3` (07-31) | cell slabs, 8,064 indices = 256 cap-corner quads + **1,024 corner curtains** + 64 skirt | a slab **with walls** | **no** — the walls seal it |
| CELL_1 rev2 `bacc1a5` (**08-01**) | the clean prefix, 6,912 indices, **curtain tail excluded** | a slab **cap alone**, floating at lift height over unlifted ground | **YES** |

`bacc1a5` names the trade in its own message: *"Lifted cells no longer cast their
walls; that is the expected trade, watched at the gate."* It was watched at the gate,
and this is what the gate saw.

**`bacc1a5` is the first moment in the repository's history at which the shadow
terrain caster contains lifted slab caps and no wall geometry whatsoever.** Before it,
detachment was geometrically impossible: pre-CELL_1 the caster ramped (wrong
silhouette, but attached), and CELL_1-original carried curtains. Detachment requires a
disconnected caster, and a disconnected caster first exists at `bacc1a5`.

**The answer to K1-f's question is therefore: newly caused, not newly noticed** — and
caused today, roughly six hours before this handoff was read, by a commit whose gate
row correctly predicted it. This does not make CURTAIN_1's diagnosis wrong; it makes it
right and *younger* than the handoff assumed. It also means the revert is a one-line
revert of a one-line change, which is what makes the K1-c ruling worth putting in front
of Jean rather than around.

**The one caveat, stated plainly.** This dating assumes Screenshots 94 and 95 postdate
`bacc1a5` (2026-08-01 04:14 UTC). If they predate it, the floating-cap mechanism
cannot be their cause and K1 must widen before anything lands — because no earlier tree
state can produce a detached caster. Jean's own discriminator settles it either way,
and is worth running first regardless.

### Jean's discriminator, with the arithmetic verified at HEAD

The handoff predicts detachment distance `= lift / tan(sun elevation)`.
`MOOD_OPEN_SUNSET`'s sun direction is authored at `spine_state.hpp:200` as
`{0.96f, -0.26f, -0.13f}`, normalized at `mood.hpp:545-550`. That gives |horizontal| =
0.9658 and |vertical| = 0.2592 — matching the table already in `world.wgsl:3289`
(`MOOD_OPEN_SUNSET  0.966  0.259`) — so

> detachment = lift × 0.9658 / 0.2592 = **3.726 × lift**

The handoff's ≈3.7× is exact at HEAD. **The prediction is on record before any edit
lands:** in a high-sun mood the quads should sit close under their slabs; at sunset
they should fly far out, at 3.73× the lift height. If detachment does *not* track sun
elevation, the floating-cap diagnosis is wrong and K1 must widen.

---

## R11 — A STALE LABEL, REPORTED NOT EDITED

K2 was to correct this as part of its commit. K2 is held, so the correction is held
with it and is recorded here instead. `cartridge.hpp:1624-1628`:

```cpp
                // SCOPE: curtains exist ONLY in the LOD0 index buffer. The
                // cap-only choice is correct on a clean LOD0 patch because no
                // cell there lifts — not because a lift without a curtain is
                // harmless. In the LOD1 ring cells lift and own no curtain;
                // what seals those seams is the rim curtain (WALL_1 — skirt
                // ring copies stand on unlifted ground).
```

**Two clauses are false at HEAD**, both since `21f2a66`: curtains do *not* exist only
in the LOD0 index buffer, and cells in the LOD1 ring *do* own a curtain. The true
statement is that the ring's curtain exists and the **shadow pass** does not draw it.
This is a behaviour claim, so per P5 it is corrected, not annotated around. Suggested
replacement, ready to land with whichever commit next touches this site:

```cpp
                // SCOPE: the LOD0 index buffer's curtain band is what this
                // flag switches. The ring (LOD1) buffer has a curtain band
                // too, as its tail (CELL_1 rev2) — the main pass draws it via
                // plan slot C, and the shadow pass does not, taking the clean
                // prefix instead. The cap-only choice is correct on a clean
                // LOD0 patch because no cell there lifts — not because a lift
                // without a curtain is harmless.
```

The already-corrected note at `world.wgsl:4771-4775` is **accurate** at HEAD and needs
nothing:

```wgsl
    // buffer. The shadow pass binds the ring IB's clean prefix only
    // (ECONOMY_1 E2, CELL_1 rev2): stride-2 cap lattice + skirt copies.
    // The curtain tail is not drawn here, so curtains again do not
    // cast — caps cast as slabs, walls do not.
```

---

## SECOND OBSERVATION — recorded, not chased

Screenshot 95's dark speckled arc across the mid-distance terrain, and Screenshot 94's
band of high-frequency multicolour checkering at the left, are not explained by this
campaign and no recon was authorized for them. Recorded so a future reader knows they
were seen and not diagnosed. If they persist, they get their own question.

One fact from this read is worth leaving beside them without claiming it explains
anything: the colour-ownership resolution at `world.wgsl:4577-4578` is the **only**
consumer of `cell_local.z`, and K1-e above shows that field's value on skirt fragments
is contested. That is a colour path, and both observations are colour artifacts.
Stated as adjacency, not as diagnosis.

---

## K4 — CLOSE

### Campaign ledger

| Handoff | Commit | State | Notes |
|---|---|---|---|
| K1 | `curtain: K1 recon — the band, the cost, the provenance` | **landed** | This document. Three of the handoff's premises stale at HEAD; the diagnosis survives all three. Two STOPs fired — one on cost, one on a mismatch. |
| K2 | `curtain: slab walls enter the shadow map` | **HELD — STOP fired** | K1-c: +177.8 % against a 40 % threshold, 4.4× over. Invariant to instance counts, and not rescued by a coarser band (the cheapest sealing geometry is still +88.9 %). Design is otherwise *simpler* than the handoff supposed — the band is in the buffer the pass already binds, and two accessors for it already exist. Held for Jean, with three options and no recommendation. |
| K3 | `curtain: geometric normal on curtain walls` | **HELD — STOP on mismatch** | K1-e: no vertex discriminator exists or can (the wall's top verts *are* the cap's verts, welded by index). A fragment discriminator exists and is plumbed. But the fix needs `cell_local` to exclude skirt fragments, and that field's declared value on skirt quads contradicts the emission that writes it. Carries to HORIZON with the arithmetic worked out. |
| K4 | `curtain: campaign close` | **landed** | This section, UMBRA_3's ledger amendment, and the PROCESS_LAWS candidate. |

### What this campaign edited

Nothing outside `src/docs/`. Two commits, both records:

- `src/docs/CURTAIN_REPORT.md` — this document.
- `src/docs/UMBRA_REPORT.md` — UMBRA_3's ledger row amended, with the reasoning
  beneath the table.
- `src/docs/HANDOFFS/PROCESS_LAWS.md` — one unnumbered candidate beside P10.

Three code corrections are **reported and not made**, each because its owning handoff
STOPPED: the `ring_clean` → `ring_zoned` switch (K2), the curtain-wall normal (K3),
and `cartridge.hpp`'s two false SCOPE clauses (K2's label correction, travelling with
K2). Replacement text for the last is in the R11 section above, ready to paste.

### The one that is worth saying plainly

This campaign was handed a defect, a cause, and a fix, and the fix is a one-line
revert of a one-line change made six hours earlier by another session — whose own
commit message predicted this exact outcome and accepted it as a trade. Both sessions
reasoned correctly. What neither had was a record connecting "curtains leave the
caster set" to "lifted cells' shadows detach and float," because UMBRA_3 filed the
first as a saving and gated it against an artifact that was not this one.

The cost arithmetic is why K2 does not simply land: the walls are twice the triangles
of the caps that need them, and the frame's bottleneck is geometry. That is a real
tension between two correct positions — CELL_1 rev2's and CURTAIN_1's — and it is
Jean's to resolve, which is what the 40 % threshold in the handoff was for.

### Gate rows still standing

K2 and K3 did not land, so their gate rows are **predictions on record, not
observations owed**. Two of them can be run now, before any edit, and both are worth
running:

1. **The detachment discriminator** (K1-f). Detachment should scale as
   `lift / tan(sun elevation)` — 3.73× the lift at `MOOD_OPEN_SUNSET`, close under
   the slab in a high-sun mood. If it does not track sun elevation, the floating-cap
   diagnosis is wrong and K1 must widen before anything lands.
2. **The dating check** (K1-f). If Screenshots 94 and 95 predate `bacc1a5`
   (2026-08-01 04:14 UTC), the floating-cap mechanism cannot be their cause, because
   no earlier tree state can produce a disconnected caster.

The third — K2's *"slab walls self-shadow at grazing sun, walls are vertical so they
sit at the bias ceiling C2 just tightened"* — cannot be observed until K2 lands. It is
recorded here so that whoever lands K2 inherits it: **that is C2's ladder, not a K2
revert.**
