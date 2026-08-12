# SHADOW_0 — THE SHADOW-PASS CENSUS (READ-ONLY)

Answers the six questions SHADOW_0 asks of the shadow path, ahead of a
SHADOW_1 ruling on static-shadow caching. **Zero source edits.** No design
and no recommendation beyond the precondition matrix.

Base: `59b6d76` on `master`. Every claim carries file + symbol. Line
numbers are hints; the symbols are the authority (P2). Absences were
verified over whole files (P11).

**The measurement this answers to** (Boot 3, Vulkan, 920M):

```
shadow_pass gpu   14.17 / 14.36 / 14.49 ms   three outdoor windows
shadow_pass gpu   30.84 ms                   the indoor window, fps 13.9
main_pass   gpu   30.22 / 30.82 / 32.05 / 32.61 ms
```

**The doubling is fully explained by code, and it is not a doubling — it
is a loop.** See Q4.

---

## Q1 — Map inventory

**Two depth textures, both created in `state.hpp`, both
`Dim::SHADOW_MAP_SIZE` = 2048 square.**

| member | label | role |
|---|---|---|
| `shadowMapTexture_` / `shadowMapView_` | "Shadow Map" | the SUN map outdoors; **atlas for spot lights 0–1** indoors |
| `spotShadowMapTexture_` / `spotShadowMapView_` | "Spot Shadow Atlas" | **atlas for spot lights 2–3** |

`SHADOW_MAP_SIZE` is a TWIN — `state.hpp` `Dim::SHADOW_MAP_SIZE` and
`world.wgsl`'s `const SHADOW_MAP_SIZE: f32 = 2048.0`, mirrored by hand
under L3. The GPU-budget line at boot reports the pair at **16.0 MiB**,
the fifth-largest allocation in the program.

**How many map renders happen per frame, and what selects the count** —
`render_shadow_pass` (`render_passes.hpp`) forks on ONE predicate:

```cpp
if (c->mood_state_.spot_light_active && cpuSpotLights_.count > 0)
```

| arm | render passes per frame | target |
|---|---|---|
| outdoor (`else`) | **exactly 1** | `shadow_map_view()`, full 2048² |
| indoor | **one per spot light**, `li < count && li < MAX_SPOT_LIGHTS` | tiles |

So the count is neither the mood directly nor always-on: it is the **live
spot-light count**, capped at `MAX_SPOT_LIGHTS` (4). Boot 3's two indoor
moods announced `[Lighting] Cathedral (3 lights…)` for `indoor_flat` and
`[Lighting] Quartet (4 lights…)` for `indoor_vault` — so 3 and 4 passes
respectively.

**The tiling.** `TILE_W = SHADOW_MAP_SIZE / 2`, `TILE_H = SHADOW_MAP_SIZE`
— each light gets a 1024×2048 half. `use_sun_map = (li < 2)` routes
lights 0–1 onto the sun map (idle indoors) and 2–3 onto the spot atlas;
`within = li % 2` selects the half, and `depthLoadOp` is `Clear` for the
left half and `Load` for the right so the second tile does not wipe the
first. Viewport and scissor are both set per tile.

Per light, before its pass, one `CopyBufferToBuffer` moves that light's VP
from `spot_vp_staging()` into the VP buffer's `light_vp` slot — so the
shadow VS reads one light's matrix at a time from a shared slot.

---

## Q2 — Caster lists per map

**The list is the same for every map, and it is NOT filtered by light,
by volume, or by visibility.** `draw_shadow_all(c, pass, cast_terrain)` is
the single entry point for both arms; the only thing that varies is the
terrain flag.

**Ten drawables carry `DRAW_SHADOW`** (`drawable_table.hpp`, canonical
order == shadow order):

```
pawn, sphere, monolith, ribbon, arch, column, palm, cactus, blade, shell
```

**Plus three drawn outside the table**, in `draw_shadow_all` directly:

| pipeline | condition |
|---|---|
| `draw_shadow_patch_terrain` | **`cast_terrain` only** — band 0, plus band 1 when `render_patch_count > lod0_patch_count` |
| `draw_shadow_wall_paintings` | unconditional in the shadow path |
| `draw_shadow_gallery_frames` | unconditional in the shadow path |

That is the 13 shadow pipelines. **Terrain is the only caster any arm
excludes** — UMBRA_4's spot-caster cut, `cast_terrain=false` for every
atlas tile, on the reasoning that an indoor spot under a shell has a cone
that never reaches the horizon. The in-tree comment is explicit that this
is the whole of that edit and that no light-volume bounding mechanism
exists to decide it more finely.

**No per-light culling of anything else.** Every atlas tile redraws all
ten table drawables plus wall paintings plus gallery frames, at full
instance counts, regardless of whether that light's cone can reach them.
Each species' instance count comes from its own `*_index_count()`
high-water prefix (see ARENA_0 Q3), so a family with one live instance
still submits its whole prefix.

**Draw order is immaterial** — the pass is depth-only, stated in the
comment above `draw_shadow_all`.

---

## Q3 — The static/dynamic partition

The question is whether a caster's *transform* moves between mood
transitions. For most of this program the answer has an unusual shape:
**there is no per-instance transform at all.** The mesh-gen families bake
world-space vertices, and the shadow draws are `DrawIndexed(indexCount)`
with no instancing and no model matrix (ARENA_0 Q2/Q3). Their geometry is
therefore static in the strongest sense — the vertex buffer itself is the
transform, and it changes only when a mesh-gen kernel runs.

| family | transform lives in | written | verdict |
|---|---|---|---|
| **arch** | `archVertexBuffer_` (world-space verts) via `arch_mesh_gen` | on `arch_mesh_gen_pending` only | **STATIC** |
| **column / antenna** | `columnVertexBuffer_` via `column_mesh_gen` | on `column_mesh_gen_pending` | **STATIC** |
| **palm** | `palmVertexBuffer_` via `palm_mesh_gen` | on `palm_mesh_gen_pending` | **STATIC** |
| **cactus** | `cactusVertexBuffer_` via `cactus_mesh_gen` | on `cactus_mesh_gen_pending` | **STATIC** |
| **blade** | `bladeVertexBuffer_` via `blade_cluster_mesh_gen` | on `blade_mesh_gen_pending` | **STATIC** |
| **shell** | `shellVertexBuffer_` | regenerated at mood application (`[Shell] Generated FLAT/GROIN VAULT` at each transition) | **STATIC between transitions** |
| **gallery_frame** | gallery slot placements | on gallery slot placement | **STATIC between placements** |
| **wall_painting** | painting placements | at mood application (`[WallPainting] Placed 20 …`) | **STATIC between transitions** |
| **monolith** | `floatingEntityBuffer_` (cube slots) | **`update_cube`, every frame** | **DYNAMIC** |
| **sphere** | `floatingEntityBuffer_` (sphere slots) | **`update_sphere`, every frame** | **DYNAMIC** |
| **pawn / agents** | `agentStateBuffer_` | **`update_player_agent` + `update_other_agents`, every frame** | **DYNAMIC** |
| **ribbon** | `ringTransformsBuffer_` | **`compute_ribbon_rings`, every frame** | **DYNAMIC** |
| **terrain** | patch instance buffers + heightfield | streamed; per-frame cull | **DYNAMIC (set), STATIC (density)** |

**The cadence is dirty-driven and already exists.**
`phase_entity_mesh_gen` (RPhase::EntityMeshGen, `Driver::Algo`) builds a
`dirty[PopFamily::COUNT]` array from twelve constexpr-gated `prepare_mesh`
calls and dispatches only the dirty families — it "branches on
dirty-ness, not the enable bit". Each `prepare_*_mesh_gen` in
`bodies/grounded.hpp` returns false immediately unless its
`*_mesh_gen_pending` flag is set, and clears the flag when it fires.

**So the static set is already identified by an existing flag, per
family, and is already the thing that gates GPU work once per change
rather than once per frame.** A static-shadow cache would key on exactly
those flags.

**The partition is clean: nine static families, four dynamic.** No family
straddles — nothing is "static except when X".

---

## Q4 — The indoor doubling, derived from code

**It is not a doubling of one pass. It is N passes where outdoor runs
one, and the meter sums them into one row.**

The accumulation is explicit (`cartridge.hpp`, the meter's mapped
callback):

```cpp
frame_ms[self->meter_.snap_pairs[p].row] += ms;   // += , across pairs
```

and `meter_arm_render(row)` allocates a **fresh timestamp pair per call**.
Every atlas tile calls it. So `shadow_pass` indoor is the SUM over tiles,
not one tile's cost.

**The arithmetic.** Boot 3's indoor window is `t=120.3`, covering
t≈90.2–120.3 — after the `indoor_flat` transition at t=91.6, which
announced **Cathedral, 3 lights**. So 3 tiles.

| | outdoor | indoor |
|---|---|---|
| passes | 1 | 3 |
| target per pass | 2048 × 2048 | 1024 × 2048 (**half** the pixels) |
| terrain caster | **yes** | **no** (UMBRA_4) |
| other casters | all 12 | all 12 |
| measured (sum) | 14.17–14.49 ms | 30.84 ms |
| **per pass** | 14.2 ms | **10.3 ms** |

**This accounts for the 14 → 31 ms, and the accounting is not a wash.**
The code predicts 3 passes and the meter reports ~3× a smaller number, so
the *shape* is confirmed. What the derivation also surfaces is that a spot
tile costs **72% of the full sun pass** while rendering half the pixels
and skipping the single largest caster in the program. Removing terrain
and halving the raster area bought only 28%.

That residue is the finding. It says the indoor shadow cost is dominated
by something that does **not** scale with raster area or with terrain —
i.e. by the twelve remaining caster draws and their per-pass fixed costs,
repeated in full for every light. Each tile re-submits the entire caster
set with no per-light filtering (Q2), and the per-species draw counts are
high-water prefixes rather than live counts.

**Predicted scaling, untested:** `indoor_vault` (Quartet, 4 lights) should
read ~41 ms on the same reasoning. Boot 3 transitioned into it at t=123.5
but the log ends before the next window, so this is a prediction the next
meter boot can settle for free.

---

## Q5 — Invalidation hooks

The exact sites a static-shadow cache would refresh on. All already exist
and all already gate real work.

**1. Per-family mesh dirty flags** — `bodies/grounded.hpp`, the
`EntitiesState` fields:

```
arch_mesh_gen_pending, column_mesh_gen_pending (column + antenna),
palm_mesh_gen_pending, cactus_mesh_gen_pending, blade_mesh_gen_pending
```

Set on spawn/evict; consumed and cleared by the matching
`prepare_*_mesh_gen`, which `phase_entity_mesh_gen` calls once per frame.
**This is the finest-grained hook available and it is per family.**

**2. Mood transition** — the whole static set changes. Observable in the
log as `[Mood] Applied: …`, and it is where the shell is regenerated
(`[Shell] Generated FLAT` / `GROIN VAULT`) and wall paintings are placed
(`[WallPainting] Placed …`). `mood_state_.lights_dirty` is set at
`direction/mood.hpp:584` and consumed by `upload_lights`.

**3. Portal traversal** — `[Portal] GPU trigger: arch N -> seed=…` leads
to a world teardown and re-spawn; every family's pending flag is set by
the respawn path. Same class as (2) but entered from GPU state rather
than a keypress.

**4. Spawn / respawn / eviction** — the spawn engine sets the per-family
pending flags; agent respawn (`[Agents] Respawn 1 around …`) touches only
the DYNAMIC set and so needs no static invalidation at all.

**5. `lights_dirty`** — a second, independent axis: `mood.hpp:584` and
`surface/patch_system.hpp:137`. A cached spot map is invalid when the
light's VP moves, not only when casters move. Both writers must be hooks.

---

## Q6 — SHADOW_1 PRECONDITION MATRIX

**GO / NO-GO per row. No design, no recommendation.**

| # | precondition | verdict | evidence |
|---|---|---|---|
| 1 | The static/dynamic partition is clean — no family straddles | **GO** | Q3: nine static (arch, column/antenna, palm, cactus, blade, shell, gallery_frame, wall_painting, + terrain density), four dynamic (pawn/agents, sphere, monolith/cube, ribbon). The static families have no per-instance transform at all — their world-space vertex buffers ARE the transform |
| 2 | Invalidation hooks are sufficient and already exist | **GO** | Q5: five per-family `*_mesh_gen_pending` flags already gate mesh-gen once per change; mood/portal/spawn all set them; `lights_dirty` covers the VP axis |
| 3 | Per-map caster lists are separable | **NO-GO as it stands** | Q2: `draw_shadow_all` is ONE list for every map. Terrain is the only thing any arm excludes, via a single bool. There is no per-light caster set, no light-volume bounding, and the in-tree comment states plainly that no such mechanism exists. Separating them is work SHADOW_1 must do, not a property it can assume |
| 4 | The measurement justifies the work | **GO** | Q4: 14.2 ms outdoor, 30.8 ms indoor at 3 lights, ~41 ms predicted at 4. Against a 16.6 ms budget and a frame whose CPU rows total under 4 ms, `shadow_pass` is the second-largest GPU line and the largest that is structurally redundant |
| 5 | Storage-texture budget admits a copy/blit or two-map compose | **GO** | The ledger's gate reports storage textures at **2 of 4** program-wide, and the tightest single row is `storagetex 2/2/1`. Two seats are free. Depth formats and the copy path are NOT verified here — see the caveat below |
| 6 | A cached static map has somewhere to live | **GO, with a sizing note** | Both existing maps are 2048² and the pair costs 16.0 MiB of a 267.1 MiB boot budget. A third map of the same size is ~8 MiB — under 3% of current usage, and the two-arm structure already proves the code can address more than one depth target |
| 7 | The dynamic set is small enough that redrawing it per frame stays cheap | **UNMEASURED** | Q2/Q3: the four dynamic families are pawn, sphere, monolith, ribbon. Nothing in this census separates their cost from the static families' inside the 14.2 ms — the meter times the whole pass. **SHADOW_1's saving is bounded by a number nobody has yet measured**, and the cheapest way to get it is a temporary per-family timestamp or a build with the static families' draws commented out |

**Two rows short of clean, and they are different in kind.** Row 3 is
**mechanism SHADOW_1 must build** — a per-light caster list where today
there is one global one. Row 7 is **a measurement nobody has taken**: the
static/dynamic cost split inside the pass. Row 7 is the cheaper of the
two and it bounds the value of the whole campaign, so it is the one worth
taking first.

**One caveat carried out of row 5, unverified here:** whether the depth
format these textures use is copyable/blittable on the WebGPU core
feature set, and whether the web twin's core-defaults device grants it.
That is a Dawn- and spec-side fact, and per standing order 3 it is a
hypothesis until read from the source. It is named rather than assumed.

**One thing this census did not find, stated because an absence is a
witness only if stated:** there is no existing shadow-map cache, no
per-light caster culling, no light-volume bounding, and no frame-to-frame
depth reuse anywhere in the shadow path. The pass is fully recomputed
every frame, for every light, from the full caster set. SHADOW_1 starts
from zero, not from a partial mechanism.
