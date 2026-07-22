# AUDIT-2 REPORT — post-GROUND_CARD_1 verification + Stage-5 groundwork

- **Base (the new trunk base, Jean's default designation):**
  `origin/GROUND_CARD_1` HEAD = `1240bece51f64a194b7acb820262f31ad98c92cd`
  (unchanged since the H6 push; hash-anchored per the H0 law).
- **Audit branch:** `GROUND_CARD_1_AUDIT2` (this report + the two .patch
  files; no mainline edits anywhere).
- **Probe branches (throwaway, NEVER MERGED):**
  `GROUND_CARD_1_A2_P1` (cleanup certification, commit 0c917d1),
  `GROUND_CARD_1_A2_P2` (Stage-5 retirement rehearsal, commit 8f4ab76).
- Certified diffs beside this report: `A2_P1_cleanup.patch`,
  `A2_P2_stage5_retirement.patch`.

---

## [A2-1] BASELINE PIN — VERDICT: PASS

Instruments rerun against HEAD and diffed against the batch's committed
`_post_gc1` outputs:

| Instrument | Result |
|---|---|
| cc6_layout_budgets | **BYTE-IDENTICAL** |
| cc7_wgsl_binding_census | **BYTE-IDENTICAL** |
| cc7_mirror_cross | **BYTE-IDENTICAL** |
| cc4_wgsl_static_usage | **BYTE-IDENTICAL** |
| probe_dawn_witness (post_gc1 variant) | **Structurally identical** (env, module messages = zero, every family/entry verdict identical); the only deltas are the `*_ms` timing fields, which are run-nondeterministic by nature — not a tree delta. Verdict again: ALL PIPELINE FAMILIES GREEN. |

This pin is the baseline every Stage-5 handoff cites.

**Jean's gate-list outcomes** (from the AUDIT-2 handoff header: "Jean's
Windows gates PASSED (build + boot + run)"; finer gates not individually
reported — marked UNKNOWN, no re-runs requested):

| Gate | Outcome |
|---|---|
| Build (real glaw1 + MSVC/Dawn) | **PASS** (reported) |
| Boot (FXC watch incl. write_live_card) | **PASS** (reported) |
| Run | **PASS** (reported) |
| Idle rig (pixel-identity vs CLOSURE_GPU) | UNKNOWN |
| Debug eye | UNKNOWN |
| Motion motif | UNKNOWN |
| Walkabout | UNKNOWN |
| Frustum fix check | UNKNOWN |

---

## [A2-2] CLEANUP CERTIFICATION — VERDICT: PASS (2 certified removals) + FINDINGS (stale text)

### (a) photo_patch_instances = 144 — removal surface (src/, exhaustive)

| Site | Line (verbatim) | Class |
|---|---|---|
| binding_registry.hpp:91 | `inline constexpr uint32_t photo_patch_instances      = 144;` | **the constant — removal target** |
| binding_registry.hpp:23–24 | banner prose `...patch_instances(340) / photo_patch_instances (144) / zone_patch_instances(165)...` | comment (enumeration trim) |
| world.wgsl:8458 | `// (binding 144 photo_patch_instances removed — the coordinated edit...` | H1 exit comment — CURRENT, stays |
| state.hpp:4191, 4246 | H1 removal notes | CURRENT, stay |
| world.wgsl:8506 | `// Replaces the linear scan over photo_patch_instances in sample_terrain_y_at.` | historical prose — name reference trimmed in P1 |

EXPECTED (registry line + at most comments) — **met**: zero code
references. Audit artifacts + docs mention it historically (not removal
surface). **Certified removal: see `A2_P1_cleanup.patch`** (glaw1 GREEN,
Dawn witness ALL FAMILIES GREEN on the probe). Execution belongs to the
Stage-5 session's opening commit, not this audit.

### (b) trajectoriesBuffer_ — the full retirement closure

| Component | Site(s) |
|---|---|
| Member | state.hpp:1488 (shared decl line with cameraBuffer_/floatingEntityBuffer_) |
| Creation | state.hpp:2890 `makeBuffer("Trajectories", ...)` |
| Null-check | state.hpp:2977 (shared `&&` chain) |
| CPU fill (sole write path) | state.hpp:5778–5783 (idle fill; the only WriteBuffer) |
| Feeder struct | state.hpp:565–570 `GPUTrajectory` + static_assert:1380 |
| Size constant | state.hpp:46 `Dim::MAX_TRAJECTORIES = 16` (no other user) |
| Idle constants | state.hpp:283–286 `Idle::TRAJECTORY_{VALUE,VELOCITY}` (fill-only) + **`TRAJECTORY_FIELD_{VALUE,VELOCITY}` — ZERO uses anywhere (bonus corpse)** |
| Registry | binding_registry.hpp:75 `trajectories = 101` |
| WGSL | world.wgsl:5327 `@binding(101)` declaration **survived H1** (H1 removed only the bind-entry pair per spec) + §1.2 `struct Trajectory` + `fn trajectory_release` (245–255) — **zero callers, zero entry-point references (cc4)** |

**No resisting CPU consumer found** — the closure deletes clean.
Confirmed ZERO bind entries (cc6) and ZERO shader *references* (cc4);
the surviving shader *declaration* + §1.2 primitives are part of the
corpse and certified removable. ("trajectory" prose at state.hpp:440 and
the WGSL color-trajectory notes are the polyphony color ramp — unrelated,
untouched.) **Certified: `A2_P1_cleanup.patch`**, 166 lines, glaw1 GREEN
+ witness GREEN.

### (c) Stale-claim sweep — FINDINGS (list only, no edits)

| # | Site | Claim | Why stale |
|---|---|---|---|
| 1 | binding_registry.hpp:71 | `= 60;   // aka fc_agents` | the fc_agents WGSL alias was removed in H1 [1b] |
| 2 | binding_registry.hpp:73 | `= 80;   // aka fc_camera` | fc_camera alias removed in H1 [1b] |
| 3 | binding_registry.hpp:28–29 | `(102 declarations over 97 slots — audit cc7; five documented fc_ aliases share slots)` | HEAD truth: 101 declarations; 3 fc_ aliases remain (fc_config/fc_vp/fc_patches). The parenthetical cites the pre-batch audit (nuance already logged at H2/H6); fold at the Stage-5 opening commit |
| 4 | state.hpp:3893 + state.hpp:4862 | Shadow texture layout/group banners `bindings 22-23, 28` / `(3 entries...)` | now 4 entries incl. g1:34 (H4 [4a]) |
| 5 | state.hpp:3927 + state.hpp:4887 | Render texture layout/group banners `bindings 22-23, 25-27, 28-29, 31-33` / `(10 entries...)` | now 11 entries incl. g1:34 (H4 [4a]) |
| 6 | state.hpp:4751 | `// Render entity bind group (19 entries: ...)` | array holds 18 — pre-existing drift (the render-side sibling of the CE "19" drift corrected in H1); campaign never touched this group |

Cleared (checked, NOT stale): all "225" greps (zero hits), all
`effective_ground_y` mentions (zero — [5c]'s exit comments replaced the
last), ribbon terrain-follow phrasing (all current truth), gallery
texture banners (no counts), every campaign exit comment.

---

## [A2-3] STAGE-5 RETIREMENT CENSUS — VERDICT: PASS (P2 rehearsal GREEN)

Method: cc4-style static reachability (comments/strings stripped,
identifier closure over brace-matched fn bodies) + C++ grep; rehearsed
end-to-end on probe `GROUND_CARD_1_A2_P2`.

### (a) Entry points + exclusive helpers (the kill list)

Doomed roots (all confirmed present): `zone_gol_mesh_reset` @L8090,
`zone_gol_mesh_gen` @L8100, `zone_extrusion_vs` @L8122,
`zone_extrusion_fs` @L8172, `shadow_zone_extrusion_vs` @L8233.

Helpers whose ONLY roots are the doomed set (machine closure; die with it):

    apply_gol_extrusion_color   @L5626   (expected)
    zone_mesh_gen_cell          @L7988   (expected)
    zone_sample_baked_terrain_y @L5949   (expected)
    zone_emit_quad              @L7967   (mesh-gen helper)
    query_ground_baked_heightfield @L2960  (the predicted cascade head)
    contrib_static_base_at      @L2626   (see cascade — stronger than predicted)
    contrib_terrain_waves_at    @L2719   (BEYOND SPEC — the two SKIP-DOOMED
                                          wave calls were its LAST callers)
    terrain_height_at           @L621    (BEYOND SPEC — sole caller is
                                          contrib_static_base_at; the bake
                                          uses terrain_height_and_complexity)

Shared helpers reached by doomed AND survivors (MUST STAY) were computed
and honored — notably `contrib_pyramids_at`, `structure_height_at`,
`tile_modifiers_at`, `gol_composite_cell_color`, `sample_pawn_aura`
(full list in the closure output, reproducible via the method above).

### (b) THE CASCADE (explicit)

    query_ground_baked_heightfield: pre-callers = [zone_sample_baked_terrain_y] → post = []  (dies)
    contrib_static_base_at:  pre = [query_ground_baked_heightfield] → post = []  (dies ENTIRELY —
        the bake never called it; ground_formed_with_complexity hand-fuses the composition)
    contrib_pyramids_at:     pre = [ground_formed_with_complexity, query_ground_baked_heightfield]
                             → post = [ground_formed_with_complexity]  → BAKE-ONLY ✓
    contrib_terrain_waves_at: pre = [zone_extrusion_vs, shadow_zone_extrusion_vs] → post = []  (dies)
    contrib_gol_zones_at:    post = [write_live_card, contrib_gol_suppression_at(LATENT, 0 callers)]  (stays — the writer's)

BAKE-ONLY verdict: `contrib_static_base_at` and `contrib_pyramids_at`
have ZERO post-retirement callers outside the bake closure. No surviving
caller findings.

### (c) Bindings + buffers freed

Exclusive to the doomed closure (machine-verified):
`zone_heightfield` g0:163, `zone_hf_sampler` g0:164,
`zone_patch_instances` g0:165, `zone_mesh_vertices` g0:167,
`zone_mesh_indices` g0:168, `zone_mesh_indirect` g0:169 — declarations +
GoL-layout entries + buffers (`zoneMeshVertexBuffer_/IndexBuffer_/
IndirectBuffer_`, state.hpp:1586–1588/3457–3469) +
`Dim::MAX_ZONE_MESH_VERTICES/INDICES` (state.hpp:246–247).
GoL layout also drops its `tile_grid`(25) / `pier_instances`(26) /
`pyramid_instances`(30) entries — their only GoL user was
zone_gol_mesh_gen (the pier drop is the probe-A pre-certified one; the
shared declarations stay for the bake).
**The indirect slot:** dt_zone draws `DrawIndexedIndirect` in BOTH the
MAIN and SHADOW passes (drawable row `DRAW_SHADOW | DRAW_MAIN`) — the
retirement frees one indexed-indirect budget line in EACH of those two
passes (the FXC banner's "one DrawIndexedIndirect per render pass" law).

### (d) C++ side (exact rows/calls)

| Component | Site |
|---|---|
| dt_zone thunk | drawable_table.hpp:59–63 |
| DRAWABLES row | drawable_table.hpp:111 `{ "zone", DRAW_SHADOW \| DRAW_MAIN, dt_zone }` |
| zone_active plumbing | drawable_table.hpp:49 (field) + render_passes.hpp:334, 405 (fills) + **bodies/gallery.hpp:1189 (`/*zone_active=*/false` — the 5th DrawBind site, found BY the P2 glaw1 gate)** |
| Draw verbs | renderer.hpp:725–758 (`draw_zone_extrusion` + shadow) |
| Render pipelines | renderer.hpp:285–286 members; main creation ~1668–1728 ("Zone Cell Extrusion"); shadow creation 2238–2241 |
| Mesh compute pipelines | renderer.hpp members `zoneGolMeshResetPipeline_/GenPipeline_`; creation 1497–1506 — **the same block creates zone_derive_params: KEEP derive, re-pin its layout to the shared GoL layout** (P2 shows the shape) |
| Mesh dispatch half | gol_zones.hpp:293 (proto) + 674–685 (`dispatch_zone_mesh`) + cartridge.hpp:1373 (the call inside `phase_gol_zone_compute`). **The sync/evolve/derive halves STAY**: `dispatch_zone_sync`:656, `dispatch_zone_evolve`:665, `flush_zone_derive_requests`:567–580, and the `GolZoneCompute` row (cartridge.hpp:1503) itself stays |
| Mesh dispatch methods | renderer.hpp:633–654 (`dispatch_zone_mesh_reset/gen`) |
| zone_mesh accessors | state.hpp:2774–2776 + the `zone_mesh_gen_group()/layout()` aliases |

### (e) Probe P2 — the rehearsal (throwaway)

Whole retirement (a–d) performed mechanically on `GROUND_CARD_1_A2_P2`
(commit 8f4ab76; **`A2_P2_stage5_retirement.patch`**, 999 lines):

    8 files changed, 48 insertions(+), 635 deletions(-)

Gates: **glaw1 GREEN** (after catching the gallery.hpp DrawBind site —
a real Stage-5 spec item this rehearsal surfaced); **Dawn witness ALL
PIPELINE FAMILIES GREEN** (P2 family roster: gol_zone = sync/evolve/
derive only; zone render pipelines retired; layouts re-parsed from the
P2 tree; module compiles with zero messages). One rehearsal lesson for
the Stage-5 spec: removing a WGSL entry point must take its
`@compute/@vertex/@fragment` attribute lines with it (the first P2 cut
left five orphaned attribute lines → Tint "unexpected attributes").

Post-retirement cc6 GoL row (expected ≈ 2 storage / 2 uniform):

    GoL Zone Compute Layout: 5 entries — 2 storage (zone_config, zone_life),
    2 uniform (config, zone_derive_requests), 1 storage-texture
    (zone_life_tex_write 162). cc6 flags: EMPTY.

## [A2-4] STAGE-5 TOPOLOGY FACTS — verbatim extraction (no edits)

All quotes taken at base 1240bece51f64a194b7acb820262f31ad98c92cd.


### (a) terrain_index_gen — the kernel, its constants, its dispatch

`src/cartridges/the_board/realization/world.wgsl:7360-7393` — section banner + the whole kernel

```
        signal.aspect_ratio
    );

    // Sun VP: kite coupling — the sun orbits THE POINT at fixed
    // offset (was the pawn; the 300-unit shadow box must cover
    // what the eye sees, so it follows the point's host — identical
    // when the pawn hosts, tracks the camera in free-fly).
    if (coupling_active(COUPLING_PAWN_TO_SUN_VP)) {
        vp_data.light_vp = coupling_pawn_to_sun_vp(
            point_pos(),
            config.sun_direction
        );
    }
}

// Fills the terrain index buffer with the standard quad triangulation pattern.
@compute @workgroup_size(8, 8)
fn generate_terrain_indices(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= TERRAIN_MESH_N || id.y >= TERRAIN_MESH_N) { return; }

    let base = (id.y * TERRAIN_MESH_N + id.x) * 6u;
    let i00 = id.y * TERRAIN_MESH_STRIDE + id.x;
    let i10 = i00 + 1u;
    let i01 = i00 + TERRAIN_MESH_STRIDE;
    let i11 = i01 + 1u;

    // Same winding as the original CPU builder: (i00,i01,i10), (i10,i01,i11)
    terrain_mesh_indices[base + 0u] = i00;
    terrain_mesh_indices[base + 1u] = i01;
    terrain_mesh_indices[base + 2u] = i10;
    terrain_mesh_indices[base + 3u] = i10;
    terrain_mesh_indices[base + 4u] = i01;
    terrain_mesh_indices[base + 5u] = i11;
}
```

`src/cartridges/the_board/realization/world.wgsl:262-263` — WGSL mesh consts

```
const TERRAIN_MESH_N: u32 = 256u;
const TERRAIN_MESH_STRIDE: u32 = TERRAIN_MESH_N + 1u;  // vertices per row (fence posts)
```

`src/cartridges/the_board/realization/state.hpp:49-51` — C++ twins (Dim::)

```
            constexpr uint32_t TERRAIN_MESH_N = 256;
            constexpr uint32_t TERRAIN_MESH_VERTS = (TERRAIN_MESH_N + 1) * (TERRAIN_MESH_N + 1);
            constexpr uint32_t TERRAIN_INDEX_COUNT = TERRAIN_MESH_N * TERRAIN_MESH_N * 6;
```

`src/cartridges/the_board/realization/state.hpp:2835-2835` — workgroup count authorship

```
            static constexpr uint32_t terrain_mesh_workgroups() { return Dim::TERRAIN_MESH_N / 8; }
```

`src/cartridges/the_board/realization/renderer.hpp:443-456` — dispatch site

```
            void dispatch_generate_terrain_indices(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup terrainIndexGenBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(generateTerrainIndicesPipeline_);
                pass.SetBindGroup(0, terrainIndexGenBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }

            // Pass 1: evaluate ground_formed() per texel, store height only.
            void dispatch_generate_patch_heights(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup patchGenBindGroup,
```


### (b) the patch VS decode — vertex_index → (x,z,uv), both VS

`src/cartridges/the_board/realization/world.wgsl:276-312` — the authored decode constants (mesh + skirt cluster)

```
    let raw = vec2(config.lod_point_x, config.lod_point_z)
            - vec2(LIVE_CARD_EXTENT * 0.5);
    return floor(raw / cs) * cs;                          // cell snap
}
const PATCH_MESH_N: u32 = 64u;          // mesh subdivisions per patch (VS bilinear-samples 256-texel heightfield)
const PATCH_MESH_STRIDE: u32 = PATCH_MESH_N + 1u;

// THE ONE-ADDRESS LAW (SEAMLESSNESS corollary — charter C8). A cell
// has exactly ONE address: its world cell index. Every consumer —
// hash, roll, noise, color/tag texel, bake write — derives from it. A
// texel is COMPUTED FROM the address; patch_uv never addresses
// anything by itself again.
fn cell_address(world_xz: vec2<f32>) -> vec2<i32> {
    let cs = PATCH_EXTENT / f32(PATCH_CELL_N);
    return vec2<i32>(floor(world_xz / cs));
}

// ─── Patch skirts (weld #2, SKIRTS) ─────────────────────────────────
// Each patch skirts its FULL perimeter to hide inter-patch cracks
// (precision + LOD/T-junction) with one mechanism: duplicate the edge
// ring, drop the copies below the composited surface, quad-strip
// ring→copy. Skirt verts have vertex_index >= PATCH_GRID_VERT_COUNT; the
// index geometry is appended by the C++ patch-IB gen (state.hpp).
const PATCH_GRID_VERT_COUNT: u32 = PATCH_MESH_STRIDE * PATCH_MESH_STRIDE;  // 65*65 = 4225
const PATCH_SKIRT_RING: u32 = 4u * PATCH_MESH_N;                           // 256 perimeter verts
// Curtain depth (world units below the composited edge). For a heightfield
// the curtain only ever shows at the crack it fills or, in a finite world,
// the outer rim — so start generous; rig-tuned.
const PATCH_SKIRT_DEPTH: f32 = 8.0;

// Skirt ring index k in [0, PATCH_SKIRT_RING) -> its perimeter grid vertex
// (vx, vz), each in [0, PATCH_MESH_N]. CW walk: bottom, right, top, left.
// MIRROR of the C++ skirt_grid_index (state.hpp patch IB) — the two MUST
// agree so each skirt quad's top edge reads the right composited height.
fn patch_skirt_grid(k: u32) -> vec2<u32> {
    let n = PATCH_MESH_N;
    if (k < n)         { return vec2<u32>(k, 0u); }
```

`src/cartridges/the_board/realization/world.wgsl:3987-4032` — patch_terrain_vs: signature through heightfield sample

```
fn patch_terrain_vs(
    @builtin(vertex_index) vi: u32,
    @builtin(instance_index) patch_id: u32
) -> PatchTerrainVarying {
    // Direct or indirect patch lookup (override-controlled per pipeline variant)
    var actual_id = patch_id;
    if (USE_PATCH_INDIRECTION) { actual_id = visible_patch_indices[patch_id]; }
    let pi = patch_instances[actual_id];

    // Decode grid position from vertex index. Skirt verts (index >=
    // PATCH_GRID_VERT_COUNT) map to a perimeter grid vertex; they inherit the
    // full composited surface below and get dropped by PATCH_SKIRT_DEPTH after.
    var vx: u32;
    var vz: u32;
    var is_skirt = false;
    if (vi >= PATCH_GRID_VERT_COUNT) {
        let g = patch_skirt_grid(vi - PATCH_GRID_VERT_COUNT);
        vx = g.x;
        vz = g.y;
        is_skirt = true;
    } else {
        vx = vi % PATCH_MESH_STRIDE;
        vz = vi / PATCH_MESH_STRIDE;
    }

    // UV within the patch [0, 1]
    let uv = vec2(
        f32(vx) / f32(PATCH_MESH_N),
        f32(vz) / f32(PATCH_MESH_N)
    );

    // Remap UV to align with texel centers in the heightfield.
    let res = f32(PATCH_HEIGHTFIELD_N);
    let sample_uv = (uv * (res - 1.0) + 0.5) / res;

    // Sample heightfield from this patch's array layer
    // .x = height, .yz = gradients, .w = unused (was complexity — swept)
    let height_data = textureSampleLevel(
        patch_heightfield_array_read, bilinear_sampler,
        sample_uv, i32(pi.layer), 0.0
    );

    // World position: patch origin + local offset
    let half = pi.extent * 0.5;
    var world_pos = vec3(
        pi.origin.x + (uv.x - 0.5) * pi.extent,
```

`src/cartridges/the_board/realization/world.wgsl:4283-4318` — shadow_patch_terrain_vs: signature through world_pos

```
fn shadow_patch_terrain_vs(
    @builtin(vertex_index) vi: u32,
    @builtin(instance_index) patch_id: u32
) -> ShadowVarying {
    let pi = patch_instances[patch_id];

    // Same skirt decode as patch_terrain_vs — the shadow pass shares the
    // patch index buffers (which now carry skirt indices), so it must map
    // and drop skirt verts too, else vi >= grid count reads garbage.
    var vx: u32;
    var vz: u32;
    var is_skirt = false;
    if (vi >= PATCH_GRID_VERT_COUNT) {
        let g = patch_skirt_grid(vi - PATCH_GRID_VERT_COUNT);
        vx = g.x;
        vz = g.y;
        is_skirt = true;
    } else {
        vx = vi % PATCH_MESH_STRIDE;
        vz = vi / PATCH_MESH_STRIDE;
    }

    let uv = vec2(
        f32(vx) / f32(PATCH_MESH_N),
        f32(vz) / f32(PATCH_MESH_N)
    );

    let res = f32(PATCH_HEIGHTFIELD_N);
    let sample_uv = (uv * (res - 1.0) + 0.5) / res;

    let height_data = textureSampleLevel(
        patch_heightfield_array_read, bilinear_sampler,
        sample_uv, i32(pi.layer), 0.0
    );

    let wx = pi.origin.x + (uv.x - 0.5) * pi.extent;
```

`src/cartridges/the_board/realization/state.hpp:142-145` — C++ index-count twins (Dim::)

```
            constexpr uint32_t PATCH_MESH_N = 64;      // mesh subdivisions per patch (LOD-0)
            constexpr uint32_t PATCH_INDEX_COUNT = PATCH_MESH_N * PATCH_MESH_N * 6;
            constexpr uint32_t PATCH_MESH_N_LOD1 = 32;  // LOD-1: half resolution
            constexpr uint32_t PATCH_INDEX_COUNT_LOD1 = PATCH_MESH_N_LOD1 * PATCH_MESH_N_LOD1 * 6;
```


### (c) the skirt — emission, decode, depth, instrument note, perimeter

`src/cartridges/the_board/realization/world.wgsl:293-316` — skirt constants + patch_skirt_grid decode (perimeter walk)

```
// ─── Patch skirts (weld #2, SKIRTS) ─────────────────────────────────
// Each patch skirts its FULL perimeter to hide inter-patch cracks
// (precision + LOD/T-junction) with one mechanism: duplicate the edge
// ring, drop the copies below the composited surface, quad-strip
// ring→copy. Skirt verts have vertex_index >= PATCH_GRID_VERT_COUNT; the
// index geometry is appended by the C++ patch-IB gen (state.hpp).
const PATCH_GRID_VERT_COUNT: u32 = PATCH_MESH_STRIDE * PATCH_MESH_STRIDE;  // 65*65 = 4225
const PATCH_SKIRT_RING: u32 = 4u * PATCH_MESH_N;                           // 256 perimeter verts
// Curtain depth (world units below the composited edge). For a heightfield
// the curtain only ever shows at the crack it fills or, in a finite world,
// the outer rim — so start generous; rig-tuned.
const PATCH_SKIRT_DEPTH: f32 = 8.0;

// Skirt ring index k in [0, PATCH_SKIRT_RING) -> its perimeter grid vertex
// (vx, vz), each in [0, PATCH_MESH_N]. CW walk: bottom, right, top, left.
// MIRROR of the C++ skirt_grid_index (state.hpp patch IB) — the two MUST
// agree so each skirt quad's top edge reads the right composited height.
fn patch_skirt_grid(k: u32) -> vec2<u32> {
    let n = PATCH_MESH_N;
    if (k < n)         { return vec2<u32>(k, 0u); }
    else if (k < 2u*n) { return vec2<u32>(n, k - n); }
    else if (k < 3u*n) { return vec2<u32>(n - (k - 2u*n), n); }
    else               { return vec2<u32>(0u, n - (k - 3u*n)); }
}
```

`src/cartridges/the_board/realization/state.hpp:3010-3085` — CPU index emission: LOD0 grid + skirt ring + LOD1 (createBuffers block)

```
                    else if (k < 2 * N) { vx = N;             vz = k - N; }
                    else if (k < 3 * N) { vx = N - (k - 2 * N); vz = N; }
                    else                { vx = 0;             vz = N - (k - 3 * N); }
                    return vz * S + vx;
                };

                // LOD-0: full 64×64 mesh (24576 indices)
                {
                    std::vector<uint32_t> idx;
                    idx.reserve(Dim::PATCH_INDEX_COUNT);
                    for (uint32_t z = 0; z < Dim::PATCH_MESH_N; z++) {
                        for (uint32_t x = 0; x < Dim::PATCH_MESH_N; x++) {
                            uint32_t stride = Dim::PATCH_MESH_N + 1;
                            uint32_t i00 = z * stride + x;
                            uint32_t i10 = i00 + 1;
                            uint32_t i01 = i00 + stride;
                            uint32_t i11 = i01 + 1;
                            idx.push_back(i00); idx.push_back(i01); idx.push_back(i10);
                            idx.push_back(i10); idx.push_back(i01); idx.push_back(i11);
                        }
                    }
                    // Skirt: every ring segment (LOD-0 full density).
                    for (uint32_t k = 0; k < SKIRT_RING; k++) {
                        uint32_t k1 = (k + 1) % SKIRT_RING;
                        uint32_t a  = skirt_grid_index(k);
                        uint32_t b  = skirt_grid_index(k1);
                        uint32_t sa = SKIRT_GRID_VERTS + k;
                        uint32_t sb = SKIRT_GRID_VERTS + k1;
                        idx.push_back(a); idx.push_back(b); idx.push_back(sa);
                        idx.push_back(b); idx.push_back(sb); idx.push_back(sa);
                    }
                    patchIndexCount_ = (uint32_t)idx.size();
                    patchIndexBuffer_ = makeBuffer("Patch IB",
                        patchIndexCount_ * 4,
                        wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst);
                    if (!patchIndexBuffer_) return false;
                    auto q = device_.GetQueue();
                    q.WriteBuffer(patchIndexBuffer_, 0, idx.data(), idx.size() * 4);
                }

                {
                    constexpr uint32_t step = Dim::PATCH_MESH_N / Dim::PATCH_MESH_N_LOD1;  // = 2
                    constexpr uint32_t stride = Dim::PATCH_MESH_N + 1;  // 65 verts per row
                    std::vector<uint32_t> idx;
                    idx.reserve(Dim::PATCH_INDEX_COUNT_LOD1);
                    for (uint32_t z = 0; z < Dim::PATCH_MESH_N_LOD1; z++) {
                        for (uint32_t x = 0; x < Dim::PATCH_MESH_N_LOD1; x++) {
                            uint32_t i00 = (z * step) * stride + (x * step);
                            uint32_t i10 = i00 + step;
                            uint32_t i01 = i00 + step * stride;
                            uint32_t i11 = i01 + step;
                            idx.push_back(i00); idx.push_back(i01); idx.push_back(i10);
                            idx.push_back(i10); idx.push_back(i01); idx.push_back(i11);
                        }
                    }
                    // Skirt: coarse ring — LOD-1 uses every `step`th perimeter vert
                    // so the skirt top matches the LOD-1 interior edge exactly.
                    for (uint32_t k = 0; k < SKIRT_RING; k += step) {
                        uint32_t k1 = (k + step) % SKIRT_RING;
                        uint32_t a  = skirt_grid_index(k);
                        uint32_t b  = skirt_grid_index(k1);
                        uint32_t sa = SKIRT_GRID_VERTS + k;
                        uint32_t sb = SKIRT_GRID_VERTS + k1;
                        idx.push_back(a); idx.push_back(b); idx.push_back(sa);
                        idx.push_back(b); idx.push_back(sb); idx.push_back(sa);
                    }
                    patchIndexCountLOD1_ = (uint32_t)idx.size();
                    patchIndexBufferLOD1_ = makeBuffer("Patch IB LOD1",
                        patchIndexCountLOD1_ * 4,
                        wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst);
                    if (!patchIndexBufferLOD1_) return false;
                    auto q = device_.GetQueue();
                    q.WriteBuffer(patchIndexBufferLOD1_, 0, idx.data(), idx.size() * 4);
                }

                return createSphereMesh() && createMonolithMesh() && createArchMesh() && createColumnMesh() && createPalmMesh() && createCactusMesh() && createBladeMesh() && createPyramidMesh() && createShellMesh() && createGoLZoneBuffers();
```

`src/cartridges/the_board/realization/world.wgsl:3956-3962` — INCIDENT #2 I3 instrument (varying comment)

```
    @location(2) patch_uv: vec2<f32>,    // UV within the patch [0,1] for cell sampling
    @location(3) @interpolate(flat) layer: u32,  // heightfield/cell array layer
    // TEMPORARY (INCIDENT #2, I3): 1.0 on skirt ring-copy verts, 0.0 on
    // the surface — wall fragments interpolate toward 1. Remove with
    // the instruments after conviction.
    @location(4) skirt: f32,
}
```

`src/cartridges/the_board/realization/world.wgsl:4061-4061` — INCIDENT #2 I3 instrument (the varying write)

```
    out.skirt = select(0.0, 1.0, is_skirt);   // TEMPORARY (INCIDENT #2, I3)
```


### (d) LOD1 — mesh/indices path, draw calls, counts

`src/cartridges/the_board/realization/state.hpp:143-145` — authored counts

```
            constexpr uint32_t PATCH_INDEX_COUNT = PATCH_MESH_N * PATCH_MESH_N * 6;
            constexpr uint32_t PATCH_MESH_N_LOD1 = 32;  // LOD-1: half resolution
            constexpr uint32_t PATCH_INDEX_COUNT_LOD1 = PATCH_MESH_N_LOD1 * PATCH_MESH_N_LOD1 * 6;
```

`src/cartridges/the_board/realization/state.hpp:3049-3080` — LOD1 index generation (step=2 over the same grid)

```

                {
                    constexpr uint32_t step = Dim::PATCH_MESH_N / Dim::PATCH_MESH_N_LOD1;  // = 2
                    constexpr uint32_t stride = Dim::PATCH_MESH_N + 1;  // 65 verts per row
                    std::vector<uint32_t> idx;
                    idx.reserve(Dim::PATCH_INDEX_COUNT_LOD1);
                    for (uint32_t z = 0; z < Dim::PATCH_MESH_N_LOD1; z++) {
                        for (uint32_t x = 0; x < Dim::PATCH_MESH_N_LOD1; x++) {
                            uint32_t i00 = (z * step) * stride + (x * step);
                            uint32_t i10 = i00 + step;
                            uint32_t i01 = i00 + step * stride;
                            uint32_t i11 = i01 + step;
                            idx.push_back(i00); idx.push_back(i01); idx.push_back(i10);
                            idx.push_back(i10); idx.push_back(i01); idx.push_back(i11);
                        }
                    }
                    // Skirt: coarse ring — LOD-1 uses every `step`th perimeter vert
                    // so the skirt top matches the LOD-1 interior edge exactly.
                    for (uint32_t k = 0; k < SKIRT_RING; k += step) {
                        uint32_t k1 = (k + step) % SKIRT_RING;
                        uint32_t a  = skirt_grid_index(k);
                        uint32_t b  = skirt_grid_index(k1);
                        uint32_t sa = SKIRT_GRID_VERTS + k;
                        uint32_t sb = SKIRT_GRID_VERTS + k1;
                        idx.push_back(a); idx.push_back(b); idx.push_back(sa);
                        idx.push_back(b); idx.push_back(sb); idx.push_back(sa);
                    }
                    patchIndexCountLOD1_ = (uint32_t)idx.size();
                    patchIndexBufferLOD1_ = makeBuffer("Patch IB LOD1",
                        patchIndexCountLOD1_ * 4,
                        wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst);
                    if (!patchIndexBufferLOD1_) return false;
```

`src/cartridges/the_board/realization/state.hpp:1681-1684` — the LOD1 draw law

```

            // GPU frustum culling — LOD0 only (Dawn D3D12 limit: one indirect draw per pass).
            // LOD1 always uses direct DrawIndexed; CPU computes its count.
            wgpu::Buffer frustumIndirectLOD0_;            // Indirect|CopyDst — DrawIndexedIndirect target
```

`src/cartridges/the_board/realization/render_passes.hpp:320-330` — main-pass LOD1 draw (direct DrawIndexed, instance-offset)

```
        c->gpuState_.patch_index_buffer(),
        c->gpuState_.patch_index_count(),
        c->world_state_.lod0_patch_count
    );
    if (c->world_state_.render_patch_count > c->world_state_.lod0_patch_count) {
        pass.SetIndexBuffer(c->gpuState_.patch_index_buffer_lod1(), wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(c->gpuState_.patch_index_count_lod1(),
            c->world_state_.render_patch_count - c->world_state_.lod0_patch_count, 0, 0, c->world_state_.lod0_patch_count);
    }

    // The drawable table — shadow members, canonical order.
```


### (e) the patch draw path — every authored number

`src/cartridges/the_board/realization/render_passes.hpp:360-392` — main-pass terrain FORK: LOD0 indirect-vs-direct + LOD1

```
    desc.depthStencilAttachment = &depthAttachment;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);

    // Terrain LOD0
    if (c->renderer_.use_indirect_terrain()) {
        // Outdoor: GPU-frustum-culled LOD0 via DrawIndexedIndirect
        c->renderer_.draw_patch_terrain_lod0_indirect(
            pass,
            c->gpuState_.render_entity_group(),
            c->gpuState_.render_texture_group(),
            c->gpuState_.patch_index_buffer(),
            c->gpuState_.frustum_indirect_lod0()
        );
    } else {
        // Indoor: direct draw with CPU count
        c->renderer_.draw_patch_terrain_direct(
            pass,
            c->gpuState_.render_entity_group(),
            c->gpuState_.render_texture_group(),
            c->gpuState_.patch_index_buffer(),
            c->gpuState_.patch_index_count(),
            c->world_state_.lod0_patch_count
        );
    }

    // Terrain LOD1 — always direct (Dawn D3D12 limit: only one indirect per pass)
    if (c->world_state_.render_patch_count > c->world_state_.lod0_patch_count) {
        c->renderer_.draw_patch_terrain_direct(
            pass,
            c->gpuState_.render_entity_group(),
            c->gpuState_.render_texture_group(),
            c->gpuState_.patch_index_buffer_lod1(),
```

`src/cartridges/the_board/realization/render_passes.hpp:313-330` — shadow-pass terrain FORK

```
// A shadow pass is DEPTH-ONLY, so draw order is doubly immaterial here.
inline void draw_shadow_all(MachineCtx* c, wgpu::RenderPassEncoder& pass) {
    // FORK — terrain: LOD0 direct + a manual LOD1 DrawIndexed (per-pass shape).
    c->renderer_.draw_shadow_patch_terrain(
        pass,
        c->gpuState_.render_entity_group(),
        c->gpuState_.shadow_texture_group(),
        c->gpuState_.patch_index_buffer(),
        c->gpuState_.patch_index_count(),
        c->world_state_.lod0_patch_count
    );
    if (c->world_state_.render_patch_count > c->world_state_.lod0_patch_count) {
        pass.SetIndexBuffer(c->gpuState_.patch_index_buffer_lod1(), wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(c->gpuState_.patch_index_count_lod1(),
            c->world_state_.render_patch_count - c->world_state_.lod0_patch_count, 0, 0, c->world_state_.lod0_patch_count);
    }

    // The drawable table — shadow members, canonical order.
```

`src/cartridges/the_board/realization/renderer.hpp:760-806` — draw_patch_terrain_lod0_indirect + draw_patch_terrain_direct

```
            void draw_patch_terrain_lod0_indirect(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                wgpu::Buffer indexBufferLOD0,
                wgpu::Buffer indirectLOD0
            ) {
                pass.SetPipeline(patchTerrainIndirectPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.SetIndexBuffer(indexBufferLOD0, wgpu::IndexFormat::Uint32);
                pass.DrawIndexedIndirect(indirectLOD0, 0);
            }

            // Direct terrain draw — uses non-indirect pipeline (outdoor or indoor variant).
            // For LOD1 outdoor, LOD0+LOD1 indoor, snapshot pass, etc.
            void draw_patch_terrain_direct(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount,
                uint32_t instanceCount,
                uint32_t firstInstance = 0
            ) {
                pass.SetPipeline(patchTerrainPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
                pass.DrawIndexed(indexCount, instanceCount, 0, 0, firstInstance);
            }

            // Frustum cull activation — typically driven by world type (walled vs open).
            // Walled worlds (small, finite) benefit less from culling; open worlds do.
            void set_frustum_cull_active(bool active) { useIndirectTerrainPipeline_ = active; }
            bool use_indirect_terrain() const { return useIndirectTerrainPipeline_; }

            void draw_pawn(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                uint32_t vertexCount
            ) {
                pass.SetPipeline(pawnPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                // One instance per agent slot. Inactive slots collapse via a
```

`src/cartridges/the_board/realization/state.hpp:2481-2492` — reset_frustum_indirect (the authored indirect args)

```
            void reset_frustum_indirect(wgpu::Queue& queue) {
                uint32_t args[5] = { patchIndexCount_, 0, 0, 0, 0 };
                queue.WriteBuffer(frustumComputeBuffer_, 0, args, sizeof(args));
            }

            // (legacy cell mesh accessors removed — bindings 43-45 reserved)
            static constexpr uint32_t pawn_vertex_count() { return Dim::PAWN_VERTEX_COUNT; }
            wgpu::Buffer sphere_vertex_buffer() const { return sphereVertexBuffer_; }
            wgpu::Buffer sphere_index_buffer() const { return sphereIndexBuffer_; }
            uint32_t sphere_index_count() const { return sphereIndexCount_; }
            wgpu::Buffer monolith_vertex_buffer() const { return monolithVertexBuffer_; }
            wgpu::Buffer monolith_index_buffer() const { return monolithIndexBuffer_; }
```

`src/cartridges/the_board/realization/state.hpp:3042-3047` — index buffer creation sizes (LOD0)

```
                    patchIndexBuffer_ = makeBuffer("Patch IB",
                        patchIndexCount_ * 4,
                        wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst);
                    if (!patchIndexBuffer_) return false;
                    auto q = device_.GetQueue();
                    q.WriteBuffer(patchIndexBuffer_, 0, idx.data(), idx.size() * 4);
```

`src/cartridges/the_board/realization/state.hpp:3077-3081` — index buffer creation sizes (LOD1)

```
                    patchIndexBufferLOD1_ = makeBuffer("Patch IB LOD1",
                        patchIndexCountLOD1_ * 4,
                        wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst);
                    if (!patchIndexBufferLOD1_) return false;
                    auto q = device_.GetQueue();
```


### (f) GoL mask anchors — the height_factor plane, derive kernel, corner snap

**FINDING (writer identity):** no WGSL kernel writes the height_factor plane. The sole writer is the CPU seed path in gol_zones.hpp (below), uploaded once at zone commit; zone_derive_params (GPU) derives zone_config parameters, not zone_life planes.

`src/cartridges/the_board/realization/world.wgsl:7825-7834` — the zone_life plane map (GOL_CELL_* slot constants)

```
const GOL_ZONE_STRIDE: u32 = 7168u;     // floats per zone (7 slots × 1024 cells)
const GOL_CELL_VISUAL: u32 = 0u;        // slot 0: height spring visual [0,1]
const GOL_CELL_VELOCITY: u32 = 1024u;   // slot 1: height spring velocity
const GOL_CELL_TARGET: u32 = 2048u;     // slot 2: current target (binary, Conway reads)
const GOL_CELL_NEXT: u32 = 3072u;       // slot 3: next target (binary, Conway writes)
const GOL_CELL_HEIGHT_FACTOR: u32 = 4096u;  // slot 4: per-cell height multiplier (persistent)
const GOL_CELL_COLOR_VISUAL: u32 = 5120u;   // slot 5: color spring visual [0,1]
const GOL_CELL_COLOR_VELOCITY: u32 = 6144u; // slot 6: color spring velocity

@compute @workgroup_size(8, 8, 1)
```

`src/cartridges/the_board/bodies/gol_zones.hpp:95-111` — GoLZoneProp hash ids + spawn-config height-factor stats

```
    static constexpr uint32_t TARGET_B = 937u;
    // Per-cell seeding
    static constexpr uint32_t HEIGHT_FACTOR = 938u;
};

// ── Spawn Configuration ──────────────────────────────────────────
struct GoLZoneSpawnConfig {
    static constexpr float SPAWN_CHANCE = 0.15f;  // fraction of checkerboard zones
    static constexpr float HEIGHT_CHANCE = 0.30f;  // fraction of zones that get extrusion
    static constexpr float ZONE_EXTENT = 100.0f; // 32 × 3.125 = cell-aligned
    static constexpr float MODE_THRESHOLD = 0.50f;  // min interpolated mode for eligibility
    // Per-cell height factor seeding (Gaussian draw per cell)
    static constexpr float HEIGHT_FACTOR_MEAN = 1.0f;
    static constexpr float HEIGHT_FACTOR_SIGMA = 0.15f;
    static constexpr float HEIGHT_FACTOR_CLAMP_LO = 0.6f;
    static constexpr float HEIGHT_FACTOR_CLAMP_HI = 1.4f;
    // Lens target color range: color = hash * RANGE + LO
```

`src/cartridges/the_board/bodies/gol_zones.hpp:528-545` — the CPU height_factor seed (writer of the plane)

```
        }
    }

    // Generate per-cell height factors: Gaussian draw, clamped
    std::vector<float> height_factors(Dim::GOL_ZONE_CELLS);
    for (uint32_t i = 0; i < Dim::GOL_ZONE_CELLS; i++) {
        float hf = cpu_sample_gaussian(seed + i, GoLZoneProp::HEIGHT_FACTOR,
            GoLZoneSpawnConfig::HEIGHT_FACTOR_MEAN, GoLZoneSpawnConfig::HEIGHT_FACTOR_SIGMA);
        height_factors[i] = std::max(GoLZoneSpawnConfig::HEIGHT_FACTOR_CLAMP_LO,
            std::min(GoLZoneSpawnConfig::HEIGHT_FACTOR_CLAMP_HI, hf));
    }

    // Upload all 7 slots
    c->gpuState_.upload_zone_life(queue, slot, life.data(), height_factors.data(), Dim::GOL_ZONE_CELLS);
}

// ═══ PER-FRAME UPLOAD ════════════════════════════════════════════

```

`src/cartridges/the_board/bodies/gol_zones.hpp:362-370` — host corner snap (cell-grid-snapped zone corner)

```
            gs.zones[slot].active = true;

            // Zone corner (cell-grid-snapped)
            float corner_x = std::floor(
                (raw_cx - GoLZoneSpawnConfig::ZONE_EXTENT * 0.5f) / PATCH_CELL_SIZE) * PATCH_CELL_SIZE;
            float corner_z = std::floor(
                (raw_cz - GoLZoneSpawnConfig::ZONE_EXTENT * 0.5f) / PATCH_CELL_SIZE) * PATCH_CELL_SIZE;

            // Algorithm selection
```

`src/cartridges/the_board/realization/world.wgsl:2342-2350` — WGSL corner math mirror (contrib_gol_zones_at)

```
        if (zp.alive_height < 0.01) { continue; }

        let zone_corner = zp.origin - zp.extent * 0.5;
        let cell_size = zp.extent / f32(zp.grid_size);
        let rel = world_xz - zone_corner;
        let cx = i32(floor(rel.x / cell_size));
        let cy = i32(floor(rel.y / cell_size));

        if (cx < 0 || cx >= i32(zp.grid_size) || cy < 0 || cy >= i32(zp.grid_size)) { continue; }
```

`src/cartridges/the_board/realization/world.wgsl:5837-5945` — zone_derive_params — full kernel body

```
fn zone_derive_params(@builtin(global_invocation_id) gid: vec3<u32>) {
    let req_idx = gid.x;
    if (req_idx >= zone_derive_requests.count) { return; }
    let req = zone_derive_requests.requests[req_idx];

    let seed = lattice_node_seed(req.world_seed, vec2(req.nx, req.nz), GOL_ZONE_SEED_BAND);

    // Zone origin: snap corner to cell grid, then center
    let raw_cx = (f32(req.nx) + 0.5) * MODE_LATTICE_SPACING;
    let raw_cz = (f32(req.nz) + 0.5) * MODE_LATTICE_SPACING;
    let corner_x = floor((raw_cx - ZONE_DERIVE_EXTENT * 0.5) / ZONE_DERIVE_CELL_SIZE) * ZONE_DERIVE_CELL_SIZE;
    let corner_z = floor((raw_cz - ZONE_DERIVE_EXTENT * 0.5) / ZONE_DERIVE_CELL_SIZE) * ZONE_DERIVE_CELL_SIZE;

    var zc: GoLZoneConfig;
    zc.origin = vec2(corner_x + ZONE_DERIVE_EXTENT * 0.5, corner_z + ZONE_DERIVE_EXTENT * 0.5);
    zc.extent = ZONE_DERIVE_EXTENT;
    zc.grid_size = 32u;
    zc.algorithm = req.algorithm;

    // Target colors (shared by both algorithms)
    zc.target_r = hash_property(seed, ZONE_PROP_TARGET_R) * ZONE_DERIVE_LENS_RANGE + ZONE_DERIVE_LENS_LO;
    zc.target_g = hash_property(seed, ZONE_PROP_TARGET_G) * ZONE_DERIVE_LENS_RANGE + ZONE_DERIVE_LENS_LO;
    zc.target_b = hash_property(seed, ZONE_PROP_TARGET_B) * ZONE_DERIVE_LENS_RANGE + ZONE_DERIVE_LENS_LO;

    let height_enabled = req.height_enabled != 0u;

    if (req.algorithm == GOL_ALGORITHM_CONWAY) {
        // Conway tier selection (cumulative weight)
        let tier_roll = hash_property(seed, ZONE_PROP_TIER);
        var tier_idx: u32 = GOL_TIER_COUNT - 1u;
        var cumul: f32 = 0.0;
        for (var t: u32 = 0u; t < GOL_TIER_COUNT; t++) {
            cumul += GOL_TIERS[t].weight;
            if (tier_roll < cumul) { tier_idx = t; break; }
        }
        let tp = GOL_TIERS[tier_idx];

        let actual_height = height_enabled && (tp.force_no_height == 0u);

        zc.tick_period = max(0.1,
            sample_gaussian(seed, ZONE_PROP_TICK_PERIOD, tp.tick_period_mean, tp.tick_period_sigma));
        zc.spring_stiffness = max(0.1,
            sample_gaussian(seed, ZONE_PROP_SPRING, tp.spring_stiffness_mean, tp.spring_stiffness_sigma));
        zc.transition_fraction = clamp(
            sample_gaussian(seed, ZONE_PROP_TRANSITION, tp.transition_fraction_mean, tp.transition_fraction_sigma),
            0.01, 0.5);
        zc.alive_height = select(0.0,
            max(0.5, sample_gaussian(seed, ZONE_PROP_HEIGHT, tp.alive_height_mean, tp.alive_height_sigma)),
            actual_height);
        zc.spring_variance = tp.spring_variance;

        // Pulse fields: zeroed for Conway
        zc.wander_radius = 0.0;
        zc.phase_randomness = 0.0;
        zc.boundary_mode = GOL_BOUNDARY_REFLECT;
        zc.tempo_randomness = 0.0;

        // Color mode selection (weighted by height state)
        let color_roll = hash_property(seed, ZONE_PROP_COLOR_ROLL);
        zc.color_mode = 2u; // fallback: blackish
        var ccum: f32 = 0.0;
        for (var c: u32 = 0u; c < 3u; c++) {
            if (actual_height) {
                ccum += GOL_COLOR_WEIGHTS_HEIGHT[c];
            } else {
                ccum += GOL_COLOR_WEIGHTS_NO_HEIGHT[c];
            }
            if (color_roll < ccum) { zc.color_mode = c; break; }
        }
    } else {
        // Pulse tier selection
        let tier_roll = hash_property(seed, ZONE_PROP_PULSE_TIER);
        var tier_idx: u32 = GOL_PULSE_TIER_COUNT - 1u;
        var cumul: f32 = 0.0;
        for (var t: u32 = 0u; t < GOL_PULSE_TIER_COUNT; t++) {
            cumul += GOL_PULSE_TIERS[t].weight;
            if (tier_roll < cumul) { tier_idx = t; break; }
        }
        let pp = GOL_PULSE_TIERS[tier_idx];

        let actual_height = height_enabled && (pp.force_no_height == 0u);

        zc.tick_period = max(0.1,
            sample_gaussian(seed, ZONE_PROP_TICK_PERIOD, pp.tick_period_mean, pp.tick_period_sigma));
        zc.spring_stiffness = max(0.1,
            sample_gaussian(seed, ZONE_PROP_SPRING, pp.spring_stiffness_mean, pp.spring_stiffness_sigma));
        zc.transition_fraction = clamp(
            sample_gaussian(seed, ZONE_PROP_TRANSITION, pp.transition_fraction_mean, pp.transition_fraction_sigma),
            0.01, 0.5);
        zc.alive_height = select(0.0,
            max(0.5, sample_gaussian(seed, ZONE_PROP_HEIGHT, pp.alive_height_mean, pp.alive_height_sigma)),
            actual_height);
        zc.phase_randomness = clamp(
            sample_gaussian(seed, ZONE_PROP_PHASE_RANDOM, pp.phase_randomness_mean, pp.phase_randomness_sigma),
            0.0, 1.0);
        zc.wander_radius = max(0.0,
            sample_gaussian(seed, ZONE_PROP_WANDER, pp.wander_radius_mean, pp.wander_radius_sigma));
        zc.boundary_mode = pp.boundary_mode;
        zc.tempo_randomness = clamp(
            sample_gaussian(seed, ZONE_PROP_TEMPO_RANDOM, pp.tempo_randomness_mean, pp.tempo_randomness_sigma),
            0.0, 1.0);
        zc.spring_variance = pp.spring_variance;

        // Pulse zones always use LENS color mode
        zc.color_mode = GOL_COLOR_LENS;
    }

    zone_config.zones[req.slot] = zc;
}
```


### (g) the suppression pair post-batch — the mirrored sites

`src/cartridges/the_board/realization/world.wgsl:2176-2186` — the shared radii

```
const ZONE_SPHERE_TINT_STRENGTH: f32 = 0.5;

// --- Pawn GoL suppression radii (shared between height_at + extrusion VS)
const ZONE_SUPPRESS_INNER: f32 = 4.0;   // full suppression inside this radius
const ZONE_SUPPRESS_OUTER: f32 = 15.0;  // zero suppression beyond this radius

// --- [COUPLING:cells→terrain:height]
const PIER_TOTAL: u32 = 68u;

struct PierInstance {
    origin:      vec2<f32>,   // world XZ center of footprint
```

`src/cartridges/the_board/realization/world.wgsl:2374-2385` — compute side: contrib_gol_suppression_at (LATENT reference form)

```
// should do the same.
// STATUS: LATENT[policy-surface] — the standalone form has zero callers
// today (the contributor is realized inline in walker/tilt/pair per the
// FXC fusion above); kept as the reference form for any consumer that
// wants suppression as a separate value.
fn contrib_gol_suppression_at(world_xz: vec2<f32>, consumer_pos: vec3<f32>) -> f32 {
    let h = contrib_gol_zones_at(world_xz);
    let d = distance(world_xz, consumer_pos.xz);
    let factor = 1.0 - smoothstep(ZONE_SUPPRESS_INNER, ZONE_SUPPRESS_OUTER, d);
    return h * factor;
}

```

`src/cartridges/the_board/realization/world.wgsl:3044-3050` — walker inline (the live compute-side smoothstep)

```
    let gol = sample_live_card_gol(xz);
    let d = distance(xz, qi.consumer_pos.xz);
    let supp_factor = 1.0 - smoothstep(ZONE_SUPPRESS_INNER, ZONE_SUPPRESS_OUTER, d);
    // Shared world stack + the mover-anchored self-aura (added after so the
    // pawn stands on its own aura peak without reading the grid).
    return manifold_overlay_stack(xz, qi, gol * (1.0 - supp_factor))
         + contrib_pawn_aura_at_self();
```

`src/cartridges/the_board/realization/world.wgsl:3104-3110` — pair inline (same smoothstep by construction)

```
    // on. Walker is that same tilt plus the pawn-self aura peak; they now
    // differ only by the aura.
    let d = distance(xz, qi.consumer_pos.xz);
    let supp_factor = 1.0 - smoothstep(ZONE_SUPPRESS_INNER, ZONE_SUPPRESS_OUTER, d);
    let gol_supp = gol * (1.0 - supp_factor);
    let tilt   = base + gol_supp + live_h;
    let walker = tilt + contrib_pawn_aura_at_self();
```

`src/cartridges/the_board/realization/world.wgsl:8135-8158` — render side: zone_extrusion_vs suppression (the VS mirror)

```
    let pawn_xz = render_pawn_pos().xz;
    let aura = sample_pawn_aura(pos.xz, pawn_xz);
    let ground_target = terrain_y + aura.r * config.pawn_aura_height + wave_y;

    // Wave overlay: lift entire extrusion mesh with animated terrain
    world_pos.y += wave_y;

    // ── Pawn proximity suppression — render-side mirror of
    // contrib_gol_suppression_at ────────────────────────────────────
    // Must stay in sync with the contributor's smoothstep (same
    // ZONE_SUPPRESS_INNER / ZONE_SUPPRESS_OUTER radii, same
    // 1 - smoothstep(inner, outer, dist) shape). The two cannot easily
    // share a function because this VS reads the render-stage
    // render_agents binding while contrib_gol_suppression_at reads
    // the compute-stage agent_state binding. If either changes radii
    // or shape, update the other. The shadow zone extrusion VS below
    // also mirrors this; keep all three in sync.
    let pawn_dist = distance(pos.xz, pawn_xz);
    let suppression = 1.0 - smoothstep(ZONE_SUPPRESS_INNER, ZONE_SUPPRESS_OUTER, pawn_dist);
    if (suppression > 0.001) {
        world_pos.y = mix(pos.y, ground_target, suppression);
    }

    var out: ZoneExtrusionVarying;
```

`src/cartridges/the_board/realization/world.wgsl:8233-8256` — render side: shadow_zone_extrusion_vs (the third mirror)

```
fn shadow_zone_extrusion_vs(
    @location(0) pos: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
    @location(3) color: vec3<f32>
) -> ShadowVarying {
    var world_pos = pos;
    let terrain_y = uv.x;
    let wave_y = contrib_terrain_waves_at(pos.xz);
    world_pos.y += wave_y;
    // Render-side mirror of contrib_gol_suppression_at — kept in sync
    // with the contributor and with zone_extrusion_vs's suppression
    // block (above). See that block's annotation for rationale on why
    // the function isn't shared across stages.
    let pawn_dist = distance(pos.xz, render_pawn_pos().xz);
    let suppression = 1.0 - smoothstep(ZONE_SUPPRESS_INNER, ZONE_SUPPRESS_OUTER, pawn_dist);
    if (suppression > 0.001) {
        // Shadow doesn't have aura texture — use terrain_y + wave only
        world_pos.y = mix(pos.y + wave_y, terrain_y + wave_y, suppression);
    }
    var out: ShadowVarying;
    out.clip_pos = render_vp.light_vp * vec4(world_pos, 1.0);
    return out;
}
```

**The pair Stage 5 collapses:** the render-side mirrors (zone_extrusion_vs + shadow_zone_extrusion_vs) die with the extrusion stack (A2-3); the compute-side smoothstep (walker/tilt/pair inline + the LATENT reference form) becomes the ONLY home of the suppression shape — the "keep all three in sync" burden dissolves.

---

## [A2-5] STAGE-5 FXC-SHAPE PROBES — VERDICT: PASS (timings logged)

Two throwaway WGSL prototypes compiled + validated through the witness
(headless Chromium = real Dawn; **Tint/Vulkan on SwiftShader — the FXC
word stays with Jean's machine**):

| Probe | Shape | module (create+info) | pipeline (async create) | Messages |
|---|---|---|---|---|
| (i) unified decode | patch-VS-shaped `proto_unified_decode_vs`: vertex_index → cell/band split (16-quad cap tiles vs 16 curtain quads per cell), **arithmetic + select, no loops, no tables**; validated as a full render pipeline (VS + trivial FS) | 2.4 ms | 13.7 ms | none |
| (ii) index-gen | `proto_index_gen` @8x8: per-cell emission of the 16-quad-cap + 16-curtain index pattern (192 indices/cell, fixed inner loop, fixed offsets, atomic-free) | 1.1 ms | 12.7 ms | none |

FLAG check (>10 s FXC-hang family): **nothing flagged** — both shapes
are 3–4 orders of magnitude under the line on Tint. The probes buy
Stage 5 the compiler's word on its two new code shapes; the FXC
confirmation rides Jean's next Windows build.

---

## [A2-6] THE FEATURE LINE — VERDICT: PENDING

Jean has not supplied the Chrome `wgslLanguageFeatures` line from the
design machine's witness run. **PENDING** — it blocks nothing in
Stages 5–7 (it decides paint-card v2 residency only: single-texture
`read_write` feedback vs a ping-pong pair).

Container observation (recorded for contrast, NOT the design machine's
word — this is Tint/Vulkan on SwiftShader): the audit container's
Chromium reports

    ['packed_4x8_integer_dot_product', 'unrestricted_pointer_parameters',
     'pointer_composite_access', 'readonly_and_readwrite_storage_textures']

i.e. `readonly_and_readwrite_storage_textures` IS exposed here; whether
Jean's released-Chrome/D3D12 exposes it is the line that matters.

---

## Verdict summary

| Section | Verdict |
|---|---|
| A2-1 baseline pin | **PASS** — all instruments byte-identical; witness structurally identical, ALL GREEN |
| A2-2a 144 constant | **PASS** — certified removable (`A2_P1_cleanup.patch`) |
| A2-2b trajectories | **PASS** — closure deletes clean; bonus corpse (TRAJECTORY_FIELD_*); certified |
| A2-2c stale text | **FINDINGS** — 6 items listed for the Stage-5 opening commit |
| A2-3 retirement census | **PASS** — kill list + cascade machine-verified; P2 rehearsal GREEN (glaw1 + Dawn + cc6 row 2s/2u); cascade STRONGER than spec (waves evaluator + terrain_height_at also die; static base dies entirely) |
| A2-4 topology facts | **PASS** — verbatim + one FINDING (height_factor plane is CPU-seeded; no kernel writes it) |
| A2-5 shape probes | **PASS** — both shapes validate in milliseconds on Tint |
| A2-6 feature line | **PENDING** (blocks nothing) |

Probe branches `GROUND_CARD_1_A2_P1` / `GROUND_CARD_1_A2_P2` are
throwaway rehearsals — never merged; the certified diffs beside this
report are the Stage-5 session's raw material.
