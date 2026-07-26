# TER-1 — THE TERRAIN DOSSIER

the_board, branch `FINAL_TOUCH`, anchored at HEAD
`ecc850baa118e0dbb50214304934148b89802ca0`. **Read-only — no code
changed.** This executes the Program Theory's §5 L0 charter: the complete
understanding of the ground. Sections: **A** the height function's anatomy
(A1 piers, A2 the DAG mapped, A3 the coupling surface, A4 color),
**B** the CPU/GPU twins, **C** the consumers (measured demand),
**D** the frame and its known defect, **E** the measured Surface
interface with the swappability and blockization findings.

**Method.** Eight parallel read passes (one per section; the consumer
census swept twice from independent angles — by query symbol and by
module), every pass pinned to the HEAD SHA above and confirming it before
reading (the standing worktree rule). A completeness critic then
re-verified the merged draft against the tree: **10 spot-check groups
passed** (pier byte-pins, seed-mirror constants, GoL prop-for-prop
duplication, the DRIVERLESS census count, DAG divergence rows, standing-
record cites, and the two C sweeps reconciling with no contradictions);
**2 citation errors were corrected in place** (marked "critic-corrected"
where they occur) and **4 missed items folded in** (the critic addendum
after §C). Line numbers are valid at this HEAD.

**Standing records cited throughout** (this dossier supersedes none of
them; it fixes their terrain facts in one place):
`audit/INVESTIGATION_mood_seam.md` (the precision-seam record),
`audit/RADIUS_INVENTORY.md` (radii — note its `ENTITY_CULL_BASE = 350`
rows are pre-RAD-2 and now stale; the live cull base is
`VISIBILITY_CYLINDER_RADIUS − 25 = 250`, spawn_engine.inl:319/402),
`audit/REACH_GRAPH.md` (module reach), `audit/CC_AUDIT_REPORT.md` +
`audit/SWEEP_CLOSEOUT.md` (audit history).

**The one-paragraph summary.** The ground is a GPU-sovereign, six-band
seeded lattice field, scaled and biased by a CPU-authored tile-archetype
grid, with two stamp systems (piers, pyramids) max-composed on top —
baked per 50-wu patch into a 256×256 heightfield texture in absolute
world coordinates, then consumed two ways: movers re-evaluate the live
analytic policy chain per frame; placement/render/mesh-gen read the bake
and re-add the live overlay fields piecemeal. Every musical dial on the
ground is present but driverless, held neutral by one boot block. The CPU
never computes real ground height: it authors parameters, schedules
dirty-work, and keeps exactly one deliberate approximation (the ribbon's
tile-cache estimate). The known internal defect is the absolute-frame
float32 precision seam. The interface the rest of the program actually
speaks is small (§E) — and the policy vocabulary for it already exists.

---
### A. The Height Function's Anatomy

All paths relative to `/home/user/7T-Pawns/src/cartridges/the_board/`. Verified at HEAD `ecc850baa118e0dbb50214304934148b89802ca0`.

#### A.1 Composition tree, rooted at the bake's evaluator

`ground_formed_with_complexity` returns `vec2(height, complexity)` and is, by declared contract, exactly the `POLICY_BAKED_HEIGHTFIELD` contributor set plus the complexity byproduct (world.wgsl:2323-2328; mask world.wgsl:2125-2126; canonical table `POLICIES[]` modules/ground_architecture.inl:195-198).

- **`ground_formed_with_complexity`** (world.wgsl:2330) — fused root: `height = lattice.h × mods.x + mods.y + structure_height_at(xz) + contrib_pyramids_at(xz)` (world.wgsl:2333).
  - **`terrain_height_and_complexity(xz, config.world_seed, 0.0)`** (world.wgsl:589) — the base field: sum of 6 lattice bands' heights plus averaged complexity; identical lattice math to `terrain_height_at` with the complexity accumulator added (world.wgsl:2319-2321).
    - **`terrain_activity_at`** (world.wgsl:373) — bilinear 400-wu activity lattice (world.wgsl:347) producing `(activity, beat_freq)`; feeds per-band motion gating — but with `t_beats = 0` it is numerically inert on height (see time inputs below).
      - `lattice_node_seed(seed, node, ACTIVITY_SEED_BAND=50)` (world.wgsl:404, 348) — integer-lattice hash; the deterministic seed root of every node.
      - `hash_property` (world.wgsl:282) — the universal PCG-style property hash every random draw bottoms out in.
    - **`terrain_band_contribution` × 6** (world.wgsl:529) — one call per band of `TERRAIN_BANDS` (world.wgsl:322-331): the six "octaves" — spacing 200/80/30/12/5/500 wu, amp μ 8/3/1.2/0.4/0.12/15 (continental, regional, local, detail, fine, tectonic). Each band Hermite-smoothstep blends the 2×2 nearest lattice nodes (world.wgsl:546-570).
      - `band_activity_level` (world.wgsl:366) — smoothsteps raw activity against per-band `WAVE_THRESHOLD` (world.wgsl:356-363); selects how much of the *moving* phase a band would use.
      - **`evaluate_lattice_wave`** (world.wgsl:450) — the per-node leaf wave:
        - activation gate: node silent if `hash_property(seed, WAVE_PROP_ACTIVE) > band.activation` (world.wgsl:461) — spatially coherent on/off, also the source of the complexity metric;
        - `sample_gaussian` (world.wgsl:293) — Box-Muller draws (±3σ truncated) for freq/amp/damping (world.wgsl:467-473);
        - indoor amplitude ceiling: `amp = min(amp, config.terrain_amp_ceiling)` when > 0 (world.wgsl:470-471) — a mood input (see below);
        - `evaluate_directional_wave` (world.wgsl:416) — globally coherent sine ridge along a seeded direction, `exp(−damping × perpendicular distance)` envelope;
        - `evaluate_radial_wave` (world.wgsl:434) — concentric rings from a seeded offset center, `exp(−damping × radial distance)` envelope;
        - frozen vs moving phase: `mix(val_frozen, val_moving, band_act)` where `phase_moving = phase_base + t_beats·beat_freq·band.temporal_freq·2π` (world.wgsl:479-497) — identity when `t_beats = 0`.
      - complexity accumulation: `Σ weight × is_active` per node (world.wgsl:567-568), averaged over the 6 bands (world.wgsl:601).
  - **`tile_modifiers_at`** (world.wgsl:909) — Hermite-bilinear interpolation of the CPU-authored tile grid (`TileGridEntry`: amp_scale, height_bias, activation_scale, archetype; world.wgsl:881-894); `mods.x` multiplies the lattice, `mods.y` adds a bias. **`mods.z` (activation_scale) is interpolated but consumed by neither caller** (world.wgsl:2295-2296, 2332-2333) — a dead channel.
    - `tile_grid_lookup` (world.wgsl:898) — defaults to neutral "varied" outside the 17×17 window (`TILE_GRID_SIDE` = 17, state.hpp:486; also audit/RADIUS_INVENTORY.md §1).
    - CPU source — **this is where plateaus/basins live**: `generate_tile_state` (cartridge.hpp:1933) rolls a per-50-wu-tile archetype from `ARCHETYPES` (cartridge.hpp:923-928: mountainous amp 2.0 / bias +4.0; varied 1.0 / 0; basin 0.5 / −2.0; pool 0.04 / −0.5 — near-flat floor), with mood-aware pool injection (indoor weight 1.5 vs outdoor 0.05, cartridge.hpp:1959-1963), terrain-token priors (cartridge.hpp:1966-1972), neighbor-coherence multipliers (cartridge.hpp:1974-1987), and per-tile jitter (cartridge.hpp:2006-2007). `upload_tile_grid_now` (cartridge.hpp:1894-1925) packs the cache into the GPU grid, `cell_extent = PATCH_EXTENT = 50` (cartridge.hpp:1902; state.hpp:103).
  - **`structure_height_at`** (world.wgsl:1953) — CONTRIB_SOLIDS: `max` over up to 68 pier instances (`PIER_TOTAL`, world.wgsl:1890; storage binding 26, world.wgsl:1905), loop bounded by `config.pier_count` (world.wgsl:1955).
    - `evaluate_pier` (world.wgsl:1909) — rotated-box footprint with smoothstep edge mask (world.wgsl:1928-1934) and a linear near→far height ramp along local X (world.wgsl:1945-1946): the literal ramps/plateaus/blocks (test-rig + arch + column pier slots, state.hpp:96-100). CPU author: `write_pier` (modules/spawn_engine.inl:271) with the `cpuPiers_` mirror (cartridge.hpp:496).
  - **`contrib_pyramids_at`** (world.wgsl:2031) — CONTRIB_PYRAMIDS: `max` over up to 8 pyramid instances (uniform binding 30, world.wgsl:1985).
    - `evaluate_pyramid` (world.wgsl:1987) — rotated footprint, Chebyshev-distance taper with a `truncation` flat top (world.wgsl:2019-2025), smoothstep edge mask; instances authored by the pyramid spawn family, which marks covered patches for heightfield regen (audit/REACH_GRAPH.md §3-Q2, pyramid row).

**Global inputs (the true leaves).**
- **Seed** — `config.world_seed` (uniform, world.wgsl:1377), set from `world_state_.active_seed` (state.hpp:2212; cartridge.hpp:2930, 3013). Note: the bake evaluates with `config.world_seed` (world.wgsl:2331), **not** `patch_params.master_seed` — that field (world.wgsl:866; cartridge.hpp:2477) is consumed only by the cell-color/zone pass (world.wgsl:7044, 7087). Both mirror the same active seed today, but they are two plumbing routes.
- **Time** — `t_beats` is hardwired `0.0` at both call sites (world.wgsl:2294, 2331) and `make_patch_params` sets `time = 0` (cartridge.hpp:2478): the baked lattice is frozen by construction, and consequently `terrain_activity_at`'s entire output has zero effect on height (it only selects between two identical phases).
- **Mood** — enters the WGSL tree at exactly one point: the indoor per-wave amplitude clamp `terrain_amp_ceiling = 0.5` indoor / `0` outdoor (modules/mood.inl:523-524 → world.wgsl:470-471). All other mood influence is CPU-upstream: pool-archetype weights (cartridge.hpp:1959-1963) and the finite-world radius (cartridge.hpp:871).
- **Warp** — there is **no domain warp**: `grep -i warp` over world.wgsl returns nothing. The only spatial remappings in the height path are the rigid rotations into pier/pyramid local frames (world.wgsl:1913-1916, 1988-1993).

#### A.2 `terrain_height_at` (576) vs `ground_formed_with_complexity` (2330)

`terrain_height_at` (world.wgsl:576-586) is the **raw lattice sum only** — heights of the 6 bands, no modifiers, no structures — parameterized on `(xz, master_seed, t_beats)`. It is a leaf of the architecture, not a policy: its **sole caller** is `contrib_static_base_at` (world.wgsl:2293-2297), which composes `raw_h × mods.x + mods.y + structure_height_at(xz)` (the fused lattice × tile-modifiers + solids triple; fusion rationale world.wgsl:2284-2292). `ground_formed_with_complexity` adds on top of that: it swaps in `terrain_height_and_complexity` (same lattice work + the complexity accumulator, world.wgsl:589-602) and appends `contrib_pyramids_at`, i.e. it equals `contrib_static_base_at(xz) + contrib_pyramids_at(xz)` ≡ `query_ground_baked_heightfield` (world.wgsl:2627-2631), hand-fused so patch generation evaluates the expensive lattice pass once instead of twice (world.wgsl:2319-2328).

#### A.3 The complexity channel

Semantics: "local wave interference density [0,1]" (world.wgsl:3489) — the bilinear-weighted fraction of *active* lattice nodes covering the point, per band (world.wgsl:567-568), averaged over the 6 bands (world.wgsl:601). Producer chain: pass 1 stores `(height, complexity)` stride-2 in scratch (world.wgsl:6717-6720); pass 2 writes it to the heightfield texture's `.w` (world.wgsl:6811). Consumer chain: `patch_terrain_vs` copies `.w` into varying `@location(2) complexity` (world.wgsl:3572) — **and nothing reads it**: `patch_terrain_fs` (world.wgsl:3579+) contains no reference to `in.complexity` (verified by grep over the FS body), and every live call to the complexity-parameterized palette functions (`palette_color_smooth`:1183, `palette_color`:1194, `palette_target_color`:6940) passes the constant `0.5` (world.wgsl:4942, 6840, 6982, 7010, 7027). The channel is fully baked and shipped per-vertex but is presently dormant — infrastructure without a consumer.

#### A.4 How the bake vs the direct resolve paths call this tree

**Bake.** `generate_patch_batch` (cartridge.hpp:2433) dispatches, per patch: pass 1 `generate_patch_heights` (world.wgsl:6706) = one `ground_formed_with_complexity` per texel, 256×256 texels over a 50-wu patch (`make_patch_params` cartridge.hpp:2471-2482), each texel's coordinate derived **per patch in absolute world space**: `world_xz = patch_params.origin + (uv − 0.5) × extent` (world.wgsl:6712-6715) — the evaluation pattern identified as the leading float32 seam suspect in audit/INVESTIGATION_mood_seam.md (lines 53-70). Pass 2 `generate_patch_gradients` (world.wgsl:6726) performs no terrain evaluation — finite-difference gradients from shared-memory neighbors plus the `.w` store (world.wgsl:6811).

**Texture-side resolves (consume the bake).** `patch_terrain_vs` (world.wgsl:3516) samples the baked `.x` and layers the live deformation fields on top — pawn aura (3557-3558), wave overlay with analytic gradient (3561-3562), radial pulses (3565-3566) — a hand-fused "POLICY_FLYER-ish" set that deliberately excludes GoL (world.wgsl:3494-3503). `shadow_patch_terrain_vs` (world.wgsl:3684, height + waves only at 3708). `sample_terrain_y_at` (world.wgsl:7753) is the O(1) patch-grid texture lookup used by camera clamps and gallery/vegetation ground snaps (world.wgsl:7835, 7913-7993); `zone_sample_baked_terrain_y` (world.wgsl:5291) is the zone-mesh variant.

**Analytic direct resolves (re-run the tree live).** `query_ground_baked_heightfield` (world.wgsl:2627) recomputes the identical contributor set and serves as the fallback when the texture misses (world.wgsl:5306-5308). The placement queries call subsets (`query_ground_placement_pyramid`/`_vegetation` = static base only, world.wgsl:2593, 2615; `_painting` adds pyramids + GoL, world.wgsl:2602), and the flyer/walker family (world.wgsl:2645-2795) adds the deformation fields per `POLICIES[]` (modules/ground_architecture.inl:176-252) — noting that those deformation fields are currently held neutral: `contrib_terrain_waves_at` early-outs on `config.terrain_time ≤ 0` (world.wgsl:2383), and `terrain_time` is only ever written as `0.0` (state.hpp:5698; cartridge.hpp:2719) with band blends booted to −1 (state.hpp:5700-5705); radial pulses are likewise driverless (world.wgsl:2494-2497). **The CPU never evaluates the tree at all**: `estimate_terrain_height` (modules/spawn_engine.inl:1125) is an explicit non-policy tile-cache proxy, `height_bias + amp_scale × 5.0` (modules/spawn_engine.inl:1130).

### A1 — The Pier System as a Height Contributor

The pier is the codebase's unified "terrain-raising volume" — the successor of the retired `GPUSolidInstance` (state.hpp:724-727). One struct, one buffer, one evaluator; every raised footprint under a built thing (test-rig solids, arch feet, column plinths, antenna pads, portal feet) flows through it.

#### A1.1 The instance and its constraints (L0 facts)

- **48-byte pin.** `GPUPierInstance` — `origin[2]`, `half_size[2]`, `height_near`, `height_far`, `rotation`, `edge_blend`, `tier`, `is_active`, 2 pads — is `alignas(16)` and pinned by `static_assert(sizeof(GPUPierInstance) == 48)` (state.hpp:728-740). The WGSL mirror `PierInstance` (world.wgsl:1892-1903) must match exactly. The pin is an FXC-backend law, recorded in the shader's seam header: "instance structs in hot loops stay lean and byte-pinned (GPUPierInstance: 48 B … the successor of the retired 32-byte SolidInstance rule)" (world.wgsl:44-52). Audit corroboration: CC_AUDIT_REPORT.md:1427 (verification row 16, HOLDS).
- **Bounded loop.** `structure_height_at` (world.wgsl:1953-1961) iterates `i < min(config.pier_count, PIER_TOTAL)` with `PIER_TOTAL = 68u` (world.wgsl:1890) and composes by **max**, not sum (`best = max(best, h)`, :1958). The same seam header states the law: "evaluate_pier's caller bounds its loop by a uniform (config.pier_count) and dispatch is by uniform function choice, not branches" (world.wgsl:49-52). `config.pier_count` sits at byte offset 124 of `GPUDesignConfig`, enforced by `static_assert(offsetof(...) == 124)` and uploaded as a targeted 4-byte `WriteBuffer` (state.hpp:1815-1819; CC_AUDIT_REPORT.md:1421, HOLDS).
- **Slot map** (state.hpp:96-100): test rig `0-2`, **slot 3 silently skipped** (`PIER_ARCH_BASE = 4`; flagged as LAW-10, CC_AUDIT_REPORT.md:843-845), arches `4-35` (16 arches × 2 piers), columns+antennas `36-67` (`PIER_COLUMN_BASE = 36`; classical columns occupy GPU slots 0-15, antennas 16-31 via `ANTENNA_SLOT_OFFSET = 16`, state.hpp:152-156). `PIER_TOTAL = 68`.
- **Evaluator semantics.** `evaluate_pier` (world.wgsl:1909-1949): inactive → 0; rotate world Δ into the pier's local frame (:1913-1916); early-reject outside `half_size + edge_blend` (:1922-1925); smoothstep footprint mask when `edge_blend > 0.001`, hard boolean edge otherwise (:1928-1940); height lerps `height_near → height_far` along local X (:1945-1946); returns `raw_h * mask`. A zero-blend pier (e.g. the test-rig block) produces step-height walls.

#### A1.2 Authoring — `write_pier` and its five caller families

`write_pier(queue, slot, pier)` (spawn_engine.inl:271-276) is the single authoring gate: it (1) writes the CPU mirror `cpuPiers_[slot]` (`GPUPierInstance cpuPiers_[Dim::PIER_TOTAL]{}`, cartridge.hpp:496), (2) issues a 48-byte slot-targeted upload via `upload_pier_slot` (state.hpp:1965-1968), and (3) sets `world_state_.pier_count_dirty` and `world_state_.ground_entries_dirty`. `clear_pier` (spawn_engine.inl:278-284) writes a default-constructed instance (`is_active = 0`) through the same path. Callers:

| Caller | Site | Slots | Geometry notes |
|---|---|---|---|
| Test rig (`setup_test_rig_piers`) — **RETIRED (BOOT_ONE_VOICE C, 4cc629d): setup_test_rig_piers deleted; pier slots 0-3 unassigned** | `cartridge.hpp`, called once from `initialize` | 0 (ramp 0→3), 1 (plateau), 2 (block, `edge_blend = 0`) | permanent; survives `teardown_world` (clear loop starts at `PIER_ARCH_BASE`, `cartridge.hpp`) |
| Column `column_post_commit` | entity_pipeline.inl:1212-1226 (`write_pier` :1225) | `PIER_COLUMN_BASE + inst.slot` | square `solid_half` footprint (= shaft_r + max overhang + pad + blend, :1130-1137), flat `SOLID_HEIGHT`, rotation 0 |
| Antenna `antenna_post_commit` | entity_pipeline.inl:1377-1392 (`write_pier` :1391) | `PIER_COLUMN_BASE + slot + ANTENNA_SLOT_OFFSET` | shares the column pier range |
| Arch `arch_post_commit` | entity_pipeline.inl:2196-2242 (`write_pier` :2223, :2234) | `PIER_ARCH_BASE + slot*2`, `+1` | two rotated feet at ±half_span; only family that also marks regen (:2238-2241) |
| Portal `force_spawn_portal_at` | mood.inl:886-996 (`write_pier` :928, :937) | same arch slots (`PIER_ARCH_BASE + slot*2`, `+1`) | tier-mean Doorway geometry; callers at mood.inl:1102, :1131, :1237 |

Eviction clears piers symmetrically: `dispatch_evict_arch` (cartridge.hpp:1055-1056), `dispatch_evict_column` (:1070), `dispatch_evict_antenna` (:1084). REACH_GRAPH.md records this authoring web as reach edge 13 (spawn_engine → spine: "`patches_[p].phase = NEEDS_REGEN`, pier dirty flags, `cpuPiers_` mirror", REACH_GRAPH.md:72) and rates Column "MESSY — creates a pier (post_commit 1212)" and Arch "MESSIEST … two piers, heightfield regen" (REACH_GRAPH.md:185,190).

#### A1.3 Storage — CPU mirror + GPU buffer

- **CPU:** `cpuPiers_[68]` (cartridge.hpp:496). Consumers beyond write/clear: `recompute_and_upload_pier_count` scans it for the highest active slot (spawn_engine.inl:286-293), and `upload_ground_entries` reads arch-foot origins from it (render_passes.inl:50-51) — a peer-internals read the audit flags ("render_passes hand-packs GPU buffers … plus spawn_engine's `cpuPiers_` mirror", REACH_GRAPH.md:213-219).
- **GPU:** `pierBuffer_` = `makeBuffer("Pier Instances", 68 × 48 B = 3264 B, Storage | CopyDst)` (state.hpp:2931-2933), zero-initialized at boot (state.hpp:5853-5856; "cartridge uploads test rig at setup"). It is bound at `@binding(26)` (`var<storage, read>`, world.wgsl:1905) into **four** bind groups: compute-entity (state.hpp:4753-4755), patch gen (:5056-5058), ribbon compute (:5091-5093), and GoL zone compute (:5365-5367).

#### A1.4 Evaluation — where piers enter the height (precise call sites)

Piers enter through **both** the heightfield bake **and** live resolves — always via the same two functions, never anywhere else in the height chain (`terrain_height_at` at world.wgsl:576 is pier-free; the pier term is added one composition level up):

1. **Bake path.** `generate_patch_heights` (world.wgsl:6706, dispatched from `generate_patch_batch`, cartridge.hpp:2433-2468) calls `ground_formed_with_complexity` per texel (world.wgsl:6717); its height formula is `hc.x * mods.x + mods.y + structure_height_at(world_xz) + contrib_pyramids_at(world_xz)` (world.wgsl:2333). The result lands in the patch heightfield texture (via scratch + gradients pass). Downstream consumers of the bake therefore inherit piers *without touching the pier buffer*: the terrain vertex shader `patch_terrain_vs` samples the texture (world.wgsl:3542-3545) — **the rendered ground gets its piers only through the bake** — and `sample_terrain_y_at` (world.wgsl:7753) feeds the camera floor (:7835) and `compute_entity_placement`'s ground entries (:7913-7993), which "already includes pier contributions" (render_passes.inl:41-44).
2. **Live path.** `contrib_static_base_at` (world.wgsl:2293-2297) returns `raw_h * mods.x + mods.y + structure_height_at(world_xz)`. Every ground policy containing CONTRIB_SOLIDS dispatches through it: `query_ground_placement_pyramid` (:2594), `_placement_painting` (:2603), `_placement_vegetation` (:2616), `_baked_heightfield` (:2628 — analytic fallback, consumed by `zone_sample_baked_terrain_y`, :5291/:5308), `_flyer` (:2646 — consumed by sphere orbit clearance :2867, `update_camera` :6279, cube kite/corral behaviors :6570-6617), `_walker` (:2679), `_walker_tilt` (:2714), `_walker_pair` (:2745 — consumed by `pawn_ground_resolve`, :5380, :5385-5405), `_walker_agent` (:2776). The pawn's standing height therefore re-evaluates all active piers live every resolve, bounded by `pier_count`.

The contributor registry mirror is `CONTRIBUTOR_DAG`/`POLICIES` in ground_architecture.inl:146/:176, where CONTRIB_SOLIDS is one of the three fused static-base roots (world.wgsl:2284-2292).

#### A1.5 Regen triggers — who flips what, and what re-bakes

- **`pier_count_dirty`** (declared cartridge.hpp:580) — set by every `write_pier`/`clear_pier`; drained **once per frame** by `flush_pier_count` at the tail of `stream_patches` (cartridge.hpp:3901) → `recompute_and_upload_pier_count` → 4-byte upload at config offset 124. Although the flush executes after the generation dispatches are *encoded* (:3767), the `WriteBuffer` is queue-ordered before the frame's command-buffer submission (render passes and submit happen after `stream_patches` returns — comment cartridge.hpp:3485-3490), so same-frame bakes read the fresh count. Also force-set in `init_patch_system` (cartridge.hpp:2380).
- **`ground_entries_dirty`** — set by write/clear_pier and by entity commits/evictions (entity_pipeline.inl:320; cartridge.hpp:1037, :1101, :1123, :1145); additionally re-armed whenever `patch_instances_dirty` is set, because "GPU Y-correction is additive … ground entries must be re-uploaded … whenever the heightfield changes" (cartridge.hpp:3889-3892). Drained in the render path (cartridge.hpp:3412-3416) → `upload_ground_entries` (render_passes.inl:45-98) and cascades into `placement_dirty` → `dispatch_placement_correction` (:3417-3420).
- **Heightfield invalidation.** `mark_patches_for_regen(min_wx, min_wz, max_wx, max_wz, home_gx, home_gz)` (spawn_engine.inl:866-882) flips every overlapping patch in phase GENERATED to `NEEDS_REGEN` (:879), **explicitly excluding the home patch** (:876). `NEEDS_REGEN` means "heightfield stale (new pier in range)" (cartridge.hpp:605). Re-bake happens in the budgeted generation scan, which accepts `SPAWNED || NEEDS_REGEN` (cartridge.hpp:3759-3770; regens sort nearest-first, :3756-3758) → `generate_selected_patches` (:2652) → `generate_patch_batch` (:2670) → phase back to GENERATED (:2675). NEEDS_REGEN patches stay in the draw list and the patch-grid LUT meanwhile (:3794-3796, :3865-3866, :3877-3878).
- **Why the home patch is exempt:** spawn strictly precedes bake — "Spawning must complete before generation — piers from spawned entities affect heightfield baking" (cartridge.hpp:3736-3738). `spawn_selected_patches` runs select → place → `commit_entity_queue` (which does the `write_pier`s, spawn_engine.inl:1108-1112) while the patch is still SPAWNED (cartridge.hpp:2601-2613), so its own bake sees its piers.
- **Asymmetry (observed fact):** of the five pier authors, only **arch** (entity_pipeline.inl:2238-2241, AABB spanning both feet + reach) and **pyramid** (a non-pier contributor, :1590-1593) call `mark_patches_for_regen`. **Column and antenna post_commits write piers with no regen marking** (verified: entity_pipeline.inl:1212-1226, :1377-1392 contain no such call), so a column pier footprint (`solid_half` = shaft radius + overhang + pad + blend, :1130-1137) that overlapped an already-GENERATED neighbor patch would leave that patch's bake stale. **Portals** also skip regen marking (mood.inl:886-996), safely: they spawn inside the fullRegen first-frame path *before* any patch is generated (cartridge.hpp:3561-3563 precedes spawn :3593 and generate :3603), and mood transitions run `teardown_world` → `init_patch_system` (cartridge.hpp:2160-2162), which re-bakes everything anyway.

Standing-record cross-check: INVESTIGATION_mood_seam.md contains no pier claims (grep for pier/solid/structure_height: zero hits over its 174 lines); RADIUS_INVENTORY.md's only adjacent row is the entity-cull radius (:128), not a pier radius; REACH_GRAPH.md carries the three pier edges cited above (:72, :183-190, :219).

#### A1.6 What the CPU does *not* know

`estimate_terrain_height` (spawn_engine.inl:1125-1132) is a tile-cache lookup (`height_bias + amp_scale * 5.0`) with **no pier term**, and is documented as deliberately approximate: callers needing pier-accurate height "must either (a) pick up the GPU-baked heightfield via readback … or (b) defer the decision to a GPU compute pass" (spawn_engine.inl:1116-1124). Entity Y-correction takes route (b): CPU uploads `ground_y = 0` offsets, and `compute_entity_placement` replaces them from the baked heightfield (render_passes.inl:41-44), with arch feet located by the `cpuPiers_`-derived XZ in `GPUArchGroundEntry` (render_passes.inl:47-61; struct at state.hpp:759-768).

### A2 — The DAG Mapped: declaration vs. tree

> **TER-2 (rev A) ALIGNMENT APPLIED — 2026-07-11.** The divergences this
> section found have been aligned in the tree under the STATUS convention
> (REALIZED / LATENT[name] / INTENT), one behavior-identical commit:
> the WALKER_TILT masks now state the GoL suppression the body applies
> (REALIZED-DIVERGENT → REALIZED); the placement-query and gradient-fn
> rows are tagged LATENT[policy-surface]; the stub contributors, stub DAG
> edges, and CELESTIAL are tagged INTENT; the fused terrain VS is now a
> declared policy (POLICY_TERRAIN_RENDER, fused-only); the phantom-caller
> comment is truth-fixed; and ASSERT_POLICY_DAG_CLOSED now ITERATES
> CONTRIBUTOR_DAG[] — the table is load-bearing (break-tested: a bogus
> edge fails the build). Verdicts below are updated in place;
> ground_architecture.inl line numbers in citations refer to the
> pre-alignment tree (text anchors govern — the alignment inserted
> comment lines).

`modules/ground_architecture.inl` declares 11 contributors (enum `ContributorId`, ground_architecture.inl:102–115), 6 DAG edges (`CONTRIBUTOR_DAG`, :146–153), and — pre-alignment — 9 policies (`POLICIES[]`; **10 after TER-2** added POLICY_TERRAIN_RENDER). Its own header states the contract: the file is "pure declarations + compile-time validation, zero runtime logic" (:87–93), and world.wgsl must mirror ids and masks by hand (:94–99). The audit's REACH_GRAPH already classified the file as VOCABULARY — "pure constexpr DAG/POLICIES tables + static_asserts; zero reaches either way" (audit/REACH_GRAPH.md:96) — and this pass confirms it: outside the `#include` at cartridge.hpp:515, **no C++ code references `POLICIES`, `CONTRIBUTOR_DAG`, `ContributorId`, or `PolicyId`** (repo-wide grep; only comments and the sibling cartridges' WGSL mirrors match). The WGSL mirror consts (world.wgsl:2097–2151) are likewise referenced by no expression — each `query_ground_*` body hand-restates its contributor list, a three-place agreement with no validator (corroborated by audit/CC_AUDIT_REPORT.md:691–693, LAW-5 #103).

#### Contributor rows → realization

| Declaration row (.inl) | Realization site(s) | Verdict |
|---|---|---|
| `CONTRIB_TERRAIN_LATTICE` (:103) | fused into `contrib_static_base_at` world.wgsl:2293–2297 via `terrain_height_at`:576; mirror const :2097 | REALIZED (fused) |
| `CONTRIB_TILE_MODIFIERS` (:104) | `tile_modifiers_at`:909, fused at :2295–2296 | REALIZED (fused) |
| `CONTRIB_SOLIDS` (:105) | `evaluate_pier`:1909 + `structure_height_at`:1953, fused at :2296; CPU authoring `write_pier` spawn_engine.inl:271, `cpuPiers_` cartridge.hpp:496, boot piers cartridge.hpp:2398/2409/2420 | REALIZED (fused) |
| `CONTRIB_PYRAMIDS` (:106) | `contrib_pyramids_at`:2031; CPU authoring `upload_pyramids` state.hpp:2544, entity_pipeline.inl:1550–1566 | REALIZED |
| `CONTRIB_PAINTINGS_BASES` (:107) | `contrib_paintings_base_at`:2305 — returns 0.0, **zero call sites, present in no policy mask** (neither C++ :176–252 nor WGSL :2120–2151) | **INTENT** (TER-2: status-tagged at the enum row; stub fn kept) |
| `CONTRIB_VEGETATION_BASES` (:108) | `contrib_vegetation_base_at`:2315 — same: 0.0 stub, zero call sites, in no mask | **INTENT** (TER-2: status-tagged; stub fn kept) |
| `CONTRIB_GOL_ZONES` (:109) | `contrib_gol_zones_at`:2045; CPU `upload_zone_config`/`_header` state.hpp:2778/2784, `upload_gol_zone_config` gol_zones.inl:583 | REALIZED |
| `CONTRIB_TERRAIN_WAVES` (:110) | `contrib_terrain_waves_at`:2382; also ~15 inline render-VS call sites (e.g. :4083–4154, 7412, 8525, 9958) | REALIZED |
| `CONTRIB_RADIAL_PULSES` (:111) | `contrib_radial_pulses_at`:2496 — self-annotated **DRIVERLESS** (:2494–2495); sole CPU setter call is `set_pulse_data(0, zero_pulses)` cartridge.hpp:2725 → `pulse_count`=0 → early-exit :2497 | REALIZED, driver parked (evaluates 0 in current tree) |
| `CONTRIB_PAWN_AURA` (:112) | **two** realizations for one row: `contrib_pawn_aura_at_external`:2540 (grid sample) and `contrib_pawn_aura_at_self`:2562 (scalar peak); backing `sample_pawn_aura`:5057, compute pipeline renderer.hpp:77/581, dispatch cartridge.hpp:3395. The form split is documented only in WGSL comments (:2191–2199); the registry row has no field expressing it | REALIZED (dual-form, undeclared split) |
| `CONTRIB_GOL_SUPPRESSION` (:113) | named fn `contrib_gol_suppression_at`:2082 has **zero callers**; the contributor lives as inlined math in `query_ground_walker`:2687–2690, `_walker_tilt`:2718–2721, `_walker_pair`:2755–2757, plus a render-side mirror (:7492–7496) | REALIZED (inline-only; named fn dead) |

#### DAG edge rows → realization

| Edge (.inl:146–153) | Realization | Verdict |
|---|---|---|
| LATTICE → PYRAMIDS, TILE_MODIFIERS → PYRAMIDS, SOLIDS → PYRAMIDS (:147–149) | composition order in every pyramid-bearing query (`contrib_static_base_at` then `+ contrib_pyramids_at`: world.wgsl:2628–2629, 2646–2647, 2679–2680, 2714–2715, 2745–2746, 2776–2777) and the fused bake :2333 | REALIZED |
| PYRAMIDS → PAINTINGS_BASES (:150), SOLIDS → PAINTINGS_BASES (:151) | `to` endpoint is a 0.0 stub in no policy mask — closure over them is well-defined and currently vacuous | **INTENT** (TER-2: status-tagged at the table rows; kept) |
| SOLIDS → VEGETATION_BASES (:152) | same — vacuous | **INTENT** (TER-2: tagged; kept) |

**SUPERSEDED BY TER-2 2d — the table now has a reader.** `ASSERT_POLICY_DAG_CLOSED` is rewritten as an immediately-invoked constexpr lambda that ITERATES `CONTRIBUTOR_DAG[]` over `CONTRIBUTOR_DAG_EDGE_COUNT`: adding an edge re-validates every policy with no further edits, and world.wgsl's extension checklist now says so. The declaration is load-bearing — break-tested by adding a bogus edge ({GOL_ZONES → PYRAMIDS}) to an isolated copy: the build fails on exactly the policies containing PYRAMIDS without GOL (BAKED_HEIGHTFIELD, TERRAIN_RENDER). *(Pre-TER-2 this paragraph read: the asserts hardcoded six literal pairs and the table was referenced nowhere.)*

#### Policy rows → realization

| Declaration row (.inl) | WGSL mask | Query fn | Live consumers | Verdict |
|---|---|---|---|---|
| `POLICY_PLACEMENT_PYRAMID` (:179–181) | :2120 | `query_ground_placement_pyramid`:2593 — **zero call sites** | live pyramid Y-correction is `compute_entity_placement` 5-point min over the baked heightfield (:7977–7997), whose set **includes CONTRIB_PYRAMIDS** — the declared mask excludes it ("pyramids don't see themselves", :2592) | **LATENT[policy-surface]** (TER-2: fn tagged; the declared-intent exclusion is recorded at the row as a possible future aesthetic ruling — Jean's; changing behavior is a BEHAVIOR stage) |
| `POLICY_PLACEMENT_PAINTING` (:183–187) | :2121–2123 | `query_ground_placement_painting`:2602 — zero call sites | realized as a documented hybrid: `sample_terrain_y_at + contrib_gol_zones_at` (:7913–7914, equivalence argued :7899–7912) in `compute_entity_placement`:7887 | REALIZED (via the documented hybrid; the named fn is tagged **LATENT[policy-surface]**) |
| `POLICY_PLACEMENT_VEGETATION` (:189–191) | :2124 | `query_ground_placement_vegetation`:2615 — zero call sites | live palm/cactus/blade/column Y path samples the baked heightfield (:7930, 7939, 7949, 7959) = static_base **+ pyramids**, contradicting the declared "trees don't stand on pyramids" mask (:190) | **LATENT[policy-surface]** (TER-2: fn tagged; declared-intent exclusion recorded as a possible future aesthetic ruling) |
| `POLICY_BAKED_HEIGHTFIELD` (:195–198) | :2125–2126 | `query_ground_baked_heightfield`:2627 (called as fallback :5308); fused twin `ground_formed_with_complexity`:2330 → `generate_patch_heights`:6706/6717 | CPU orchestration `generate_patch_batch` cartridge.hpp:2433 (set named at :2427); texture consumers `sample_terrain_y_at`:7753 and the photographer camera clamp :7835 (the main camera's :6279 clamp is analytic POLICY_FLYER, not a baked consumer — critic-corrected) | REALIZED |
| `POLICY_FLYER` (:202–209, gradient=true) | :2127–2132 | `query_ground_flyer`:2645 | sphere `coupling_terrain_to_sphere_orbit_height`:2867, `update_camera`:6279, `update_cube`:6570/6575/6617 | REALIZED — `query_ground_flyer_gradient` has zero callers, now tagged **LATENT[policy-surface]**; the row's gradient flag is annotated "(intent)" |
| `POLICY_WALKER` (:214–222, gradient=true) | :2133–2139 | `query_ground_walker`:2678; fused `query_ground_walker_pair`:2743 | `pawn_ground_resolve` :5385–5405, `behavior_player_controlled` :5590–5593 | REALIZED — `query_ground_walker_gradient` and `query_ground_walker_walkable` have zero callers, now tagged **LATENT[policy-surface]**; gradient flag annotated "(intent)" |
| `POLICY_WALKER_TILT` (:230–236, gradient=true) | :2140–2144 | `query_ground_walker_tilt`:2713 | `terrain_normal_at`:5348–5352 (hand-rolled 3-sample gradient; no `_gradient` fn per the :2267 convention), `query_ground_walker_pair`:2758 | **REALIZED** (TER-2 truth-fix 1b: both masks now carry `CONTRIB_GOL_SUPPRESSION` and all four prose sites state the suppression the body applies — the declaration matches the shipped body; behavior untouched) |
| `POLICY_WALKER_AGENT` (:239–246, gradient=true) | :2145–2150 | `query_ground_walker_agent`:2775 | `agent_post_step`:5486 (scalar only) | REALIZED — `gradient_supported=true` remains unrealized (no gradient fn, no multi-sample consumer); flag kept and annotated "(intent; gradient path unrealized — see status)" per TER-2 2a |
| `POLICY_CELESTIAL` (:249–251) | :2151 | `query_ground_celestial`:2791 — zero callers, "kept for symmetry" (:2790) | none ("future celestial entity placement (none today)", :2789) | **INTENT** (TER-2: row + query fn status-tagged; kept for symmetry) |

Note — **truth-fixed by TER-2 1a**: the phantom-caller comment at world.wgsl:7877–7879 (which claimed the placement queries were "used by the real spawn decision code") now states the truth: the queries have no live caller (LATENT[policy-surface]), CPU spawn decisions stay on `estimate_terrain_height`, and the GPU placement-correction pass is the live Y path via the baked heightfield.

#### Code without a declaration row

| Code site | What it is | Assessment |
|---|---|---|
| `query_ground_walker_pair` world.wgsl:2743 | fused two-policy query (walker + tilt in one pass) | CODE-WITHOUT-ROW, benign — semantics proven ≡ separate calls (audit/CC_AUDIT_REPORT.md:1430, invariant #19 HOLDS) |
| `contrib_static_base_at` :2293 | fused eval of rows 0–2 | acknowledged in declaration (:137–139) — not drift |
| `ground_formed_with_complexity` :2330 | baked policy + complexity byproduct, bypasses query API | acknowledged fusion (:12–13 of .inl; world.wgsl:2325–2329); also load-bearing in the mood-seam crack mechanism (audit/INVESTIGATION_mood_seam.md:53–56) |
| `patch_terrain_vs` :3494–3510 | hand-fused render path: heightfield + aura(:3557) + pulses(:3565) + waves — **explicitly no GoL** (:3502) | **ROW ADDED by TER-2 1d** — now declared as `POLICY_TERRAIN_RENDER` (.inl row + WGSL mask block, fused-only, STATUS: REALIZED); the VS header names the policy and its keep-consistent clause points at the mask block |
| `zone_extrusion_vs` :7402 / `shadow_zone_extrusion_vs` :7482 | inline waves + aura + suppression mirror (:7412–7416, 7490–7493) | acknowledged fused path; suppression mirror is prose-synced only (audit/CC_AUDIT_REPORT.md:700, #105) |
| ~15 render VS sites adding `contrib_terrain_waves_at` alone (:4083–4154, 8525, 8577, 9958, 9972, 10288, 10302, 10520, 10534) | single-contributor inline consumption | sanctioned by TER-2 1d — the POLICY_TERRAIN_RENDER declaration carries a covering sentence naming these as single-contributor consumptions of the render set |
| `estimate_terrain_height` spawn_engine.inl:1125 | CPU tile-cache approximation | deliberately outside the policy system, self-documented (:1116–1124) |

#### Verdict summary

Post-alignment (TER-2 rev A): of 11 contributor rows — 8 REALIZED (one driver-parked LATENT-analogue, one dual-form now noted at the enum row, SUPPRESSION's standalone fn tagged LATENT[policy-surface]) and 2 INTENT stubs, every one status-marked. Of 6 DAG edges: 3 REALIZED, 3 INTENT — and the edge table is now READ by the closure assert (load-bearing, break-tested). Of 10 policy rows: 6 REALIZED (walker_tilt realigned by truth-fix 1b; terrain_render newly declared), 3 LATENT[policy-surface] placement rows (declared-intent exclusions recorded for Jean's future ruling), 1 INTENT (celestial). Every POLICIES[] row carries exactly one STATUS marker. The remaining unchecked mirror is the WGSL mask block (prose-synced only — the C++ asserts cannot see WGSL); that is the standing §B.7 twin, unchanged by this pass.

### A3 — The coupling surface of L0: the driverless dials

The gen-1 demolition did not remove the ground's musical capabilities — it removed their *drivers* and left every capability tagged in place, held at a neutral value written once at boot. The census is exact at this HEAD: `grep DRIVERLESS` over `src/cartridges/the_board` returns **11 shader tags in world.wgsl** (lines 502, 616, 1784, 2494, 4922, 5326, 6337, 6639, 6966, 6975, 10611) **plus 1 CPU landing site** (modules/orbs.inl:439), matching the constitution's ledger entry verbatim (src/docs/cartridge_constitution.md:99-103). The companion BOOT-NEUTRAL class (constitution:104-106) is the single block at cartridge.hpp:2711-2726 — `initialize()` writes band blends to −1, terrain time to 0, all five mode-color uniforms to 0, both GoL scales to 1.0, and zeroes the pulse ring buffer; `initializeState()` lands the same defaults GPU-side plus `amplitude_scale = Idle::AMPLITUDE_SCALE = 1.0` (state.hpp:5687-5711, 5714-5718, 238). Boot also arms the gates: `mute_couplings = Coupling::NONE` and `mute_signal = 0` (state.hpp:5652-5653), so the coupling landing sites *run every frame* — the silence comes from substituted-zero inputs and zeroed uniforms, not from the mute mask. Each entry dies by revive-or-delete when its region is next worked (constitution:102-103).

Six of the twelve are ground dials — plus the complexity channel, added to this list by TER-2 2b. This is L0's future musical surface:

#### The ground dials (six driverless + one latent channel)

| Capability | Tag site | Dial | Modulates | Neutral / boot write |
|---|---|---|---|---|
| Terrain amplitude | world.wgsl:1784-1787 (coupling fn) + 5326-5328 (landing site) | `terrain_state.amplitude_scale` via `trajectories[0]`, driver input `poly` hardwired 0.0 | attack/release envelope on terrain amplitude (attack 10, release 5 — world.wgsl:1455-1456) | rests at `IDLE_AMPLITUDE_SCALE = 1.0` (world.wgsl:1454); boot `Idle::AMPLITUDE_SCALE = 1.0` (state.hpp:238, 5715) |
| Band motion | world.wgsl:502-503 (accessors) | `config.band_blend_0..5`, `band_phase_origin_0..5` (world.wgsl:1396-1408) | per-band mix of the 6-wave CONTRIB_TERRAIN_WAVES overlay; phase origin prevents teleport on activation | all blends −1, origins 0 (cartridge.hpp:2716-2718; state.hpp:5700-5711) |
| Terrain time | no shader tag of its own — covered by the tagged boot block (cartridge.hpp:2711-2714) | `config.terrain_time` — "t_beats for terrain evaluation (0 = frozen)" (world.wgsl:1393) | temporal phase of the wave overlay: `t = terrain_time − origin` (world.wgsl:2394, 2440); ≤0 early-outs the whole contributor (2383, 2427) | 0.0 (cartridge.hpp:2719; state.hpp:5698) |
| Radial pulses | world.wgsl:2494-2495 | `config.pulse_count` + `pulse_data[8]` vec4s `(origin_x, origin_z, onset, amplitude)` (world.wgsl:1419, 1426) | CONTRIB_RADIAL_PULSES — expanding gaussian ring wavefronts from note onsets, 30 wu/s, ~8 s life (world.wgsl:2482-2524) | count 0, 32 zeros (cartridge.hpp:2724-2725; state.hpp:5689-5690) |
| Mode color quintet | world.wgsl:6966-6967 + 6975-6976 | `mode_color_shift` [0,~0.6], `mode_checker_scatter` [0,~0.5], `mode_palette_target` [0,3], `mode_palette_intensity` [0,1], `mode_discrete_tier` [0,4] (world.wgsl:1410-1414) | ground *color*: smooth→discrete bias, checker survival, palette drift toward a target tier per domain (`animated_cell_color` 6959-6994; LUT variant 6999-7028) | all 0.0 (cartridge.hpp:2720-2722) |
| GoL scales | world.wgsl:4922-4923 (tick); height reads at 2063 and 7279 | `mode_gol_tick_scale` (tempo divisor of Pulse-archetype cells, 4924), `mode_gol_height_scale` (multiplier on GoL cell extrusion, 2063 and 7279) | tempo and height of the living-zone extrusions (CONTRIB_GOL_ZONES) | both 1.0 (cartridge.hpp:2723; state.hpp:5687-5688) |
| Complexity channel (TER-2 2b) | LATENT[complexity] tags at the `.w` bake store and the terrain varying | the heightfield `.w` (wave-interference density [0,1]) — baked and shipped per-vertex, read by nothing (palette calls hardcode 0.5) | a future coupling target: interference density → material/color response | already neutral by absence of a consumer; wiring is one FS read |

**Terrain amplitude is the deepest cut — severed at both ends.** The landing site in `update_terrain_config` still executes per frame when `signal_active() && coupling_active(COUPLING_POLYPHONY_TO_AMPLITUDE)` (world.wgsl:5325, bit 0 defined at 1734), but the raw `signal.stats[0]` read was substituted with a literal `0.0` (5326-5328), so the trajectory perpetually releases to 1.0. More important for revival: the output `terrain_state.amplitude_scale` currently feeds **only** `dynamics_terrain_gradient_max` → `terrain_state.lipschitz_factor` (5337-5338), and `lipschitz_factor` has **no reader anywhere in the tree** (grep: declaration world.wgsl:647/state.hpp:508, init state.hpp:5718, write 5338 — nothing consumes it). Nothing multiplies terrain height by `amplitude_scale`. A gen-2 "amplitude" coupling therefore needs both a driver *and* a re-plumbed displacement consumer; the tag at 1784-1787 additionally parks the sovereignty question — direct shader reads of the signal bypass canvas and bank, so revival must be "a deliberately designed gen-2 idiom," not a reconnection.

**Band motion + terrain time are the cheap, fully-wired live path.** The wave overlay (`contrib_terrain_waves_at`, world.wgsl:2382-2416; fused-gradient twin 2426+) is *not baked*: it is added per-frame in every ground policy (walker/flyer/tilt sums at 2649-2650, 2692-2693, 2722-2723, 2748-2749, 2779-2780), in the terrain VS (3561), and in every entity vertex shader (4083-4154, 8525, 8577, 9958-9972, 10288-10302, 10520-10534) — cost 6 sin() per evaluation point (2344). Setting any `band_blend_i > 0` and running `terrain_time` makes the whole world breathe with zero re-bake, one uniform write per frame (`set_terrain_time`/`set_band_motion`, state.hpp:2299-2319, dirty-flag batched). The blend semantics are already banded for polyphony (fine ripples first, continental swells last — 2337-2342; OVERLAY_WAVES table 2364-2374). Note the deeper, expensive sibling: the *lattice's own* beat-time. `terrain_height_at(xz, seed, t_beats)` (576) and the frozen/moving wave mix inside `evaluate_lattice_wave` (485-497) accept live t_beats, but both production call sites pass literal `0.0` (`contrib_static_base_at`:2294, `ground_formed_with_complexity`:2331), and that frozen result is what `generate_patch_heights` bakes into the per-patch heightfield (6706-6717). Driving lattice time means re-baking patches every frame — the overlay exists precisely so L0 can move without paying that.

**Radial pulses carry a recorded unit ambiguity a driver must settle.** The shader consumes `age = t_seconds − p.z` (world.wgsl:2503-2505) while the CPU twin names the field `onset_beats`; with the writer demolished there is no live code to adjudicate, and the audit ruled the park itself is the finding (CC_AUDIT_REPORT.md:982-985; SWEEP_CLOSEOUT.md:97). The first gen-2 note-onset driver must pick the unit and fix both sides.

**The mode color quintet is zero-cost at rest by construction.** `patch_terrain_fs` gates the animated path on `has_mode_bias` — any of shift/scatter/palette-intensity > 0.001 wakes the LUT-accelerated recolor (`animated_cell_color_lut`), otherwise the FS samples the baked cell-color texture untouched (world.wgsl:3593-3603). This is the ground's *chromatic* channel: it can drift the whole surface toward a named palette tier (sand/salmon/green/warm — 1461-1466) and re-bias the smooth/discrete/checker composition without touching geometry.

**GoL scales have a census gap.** The tick-scale read is tagged (4922-4924) but the height-scale's second read in `zone_mesh_gen_cell` (7279) carries no tag — its sibling at 2063 is inside tagged CONTRIB_GOL_ZONES territory, and the audit records the undercount as L3 #73 (CC_AUDIT_REPORT.md:566-569). Any revive-or-delete pass on this capability must visit both sites.

#### Census members outside the ground surface (for completeness)

- **Sphere/floater color** — tags world.wgsl:6337-6338 and 6639-6640: `coupling_signal_polyphony_to_sphere_color` (1806-1822, a PGA hue spiral) runs each frame with `poly = 0.0`, so colors decay to `base_color`. Gate bit 12 (1746). Entity-layer, not L0.
- **Orb inputs** — tag world.wgsl:10611-10613 on `OrbConfig`: `force_radial`, `color_pulse/converge/surge`, `noise_amp`, `speed_mult`. Held neutral not by the boot block but by `configure_orbs`' config-site zeros on every mood entry (orbs.inl:727, 741-743; `noise_amp` rests at `ORB_NOISE_FLOOR`, orbs.inl:722). The four per-frame upload helpers exist with zero call sites (state.hpp:2734-2748; CC_AUDIT_REPORT.md:425-429).
- **The stats bus** — tag world.wgsl:616-619: `FrameSignal.stats[64]`, the 256-byte analysis array that would carry any raw-signal drive, is still uploaded every frame (cartridge.hpp:2868-2870) with zero shader consumers; whether GPU-side direct coupling exists *at all* is the parked gen-2 sovereignty decision. The audit flags it as the one park bounded by nothing (CC_AUDIT_REPORT.md:559-561).
- **The CPU landing site** — orbs.inl:437-442: `speed_mult_current` rests at 1.0 (identity), resets with the mood (orbs.inl:778), survives mood transitions in-flight (orbs.inl:641-644); the designed gen-2 "orb.speed" pipe lands here. GPU-side it scales each motion rule's dominant speed (world.wgsl:11118, 11164, 11270).

#### Standing conditions on revival

1. **The cited design doc is missing.** Every revive comment points at `coupling_layer_migration_map.md`, which exists nowhere in the tree (grep; recorded at CC_AUDIT_REPORT.md:602). Gen-2 coupling design starts by re-deriving or recreating those retirement decisions.
2. **Placement is already hardened for a moving ground.** Placement policies deliberately exclude all deformation fields — "placement should be stable against animated terrain" (modules/ground_architecture.inl:176-191) — and CPU spawn height is only the tile-cache approximation (`estimate_terrain_height`, modules/spawn_engine.inl:1116-1132). Waking waves, pulses, or amplitude cannot destabilize spawn Y.
3. **Two neutral-writing sites serve one census class.** The tagged block (cartridge.hpp:2711-2726) and `initializeState`'s defaults (state.hpp:5687-5711) overlap; the constitution counts one block and says it "dies with the last driverless entry it serves" (constitution:104-106) — a revive pass should collapse the redundancy it leaves behind.
4. **Precision caveat for far-field couplings.** The baked ground evaluates absolute world coordinates per patch (world.wgsl:6712-6715), the mechanism behind the distance-correlated seam (audit/INVESTIGATION_mood_seam.md:49-70); a coupling that raises amplitude or gradient far from origin will amplify that ULP crack (crack size scales with local gradient — INVESTIGATION_mood_seam.md:66-67). Likewise any reveal-band work sits at the 275 wu visible ring, not the 175 wu LOD ring (audit/RADIUS_INVENTORY.md:26-29, 186-188) — a breathing ground changes what those edges look like, not where they are.

### A4 — Terrain Color Anatomy (site map)

Terrain color is a two-stage grammar (the §2.2 tuning surface, directory at `world.wgsl:60-109`) living entirely in `/home/user/7T-Pawns/src/cartridges/the_board/world.wgsl`. Stage 1 evaluates a stack of independent spatial fields, each built the same way: a deterministic per-node raffle (`hash_property`:282 keyed by `lattice_node_seed`:404 through `color_lattice_seed`:956, band+100) on an integer lattice, then Hermite-smoothstep bilinear interpolation between the 2×2 surrounding nodes (`lattice_coord`:941, weights `frac²(3−2·frac)` at :945, `lattice_weight`:949). The **palette raffle** picks one dominant of four palettes per 300-wu node (dominant 0.85 / others 0.05); the **mode field** decides smooth-vs-discrete tendency; the **discrete-region raffle** rolls a *free* RGB mean + variance (no palette table) for mosaic cells. Stage 2 (`composite_cell_color`:6880) gates everything on `MODE_DISCRETE_THRESHOLD = 0.70` (:1489) — a blend window, a probabilistic scatter-survival window, and a sparse-scatter exception — with per-cell rolls seeded from the world cell grid (band 200u). Colors are baked once per patch by `generate_patch_cells`:7066 into a 16×16 cell texture (:7100) plus a mode/style/sparse LUT (:7104); the terrain FS samples the bake (:3584) and only re-composites live (`animated_cell_color_lut`:6999 at :3602) when a musical mode bias is nonzero (:3593-3596).

| Mechanism | Site | One sentence |
|---|---|---|
| Palette tables (CENTER/LIGHT/VARIANCE/WEIGHT) | world.wgsl:1461-1484 | Four palettes — sand/salmon/green/warm — with light variants, per-cell noise amplitudes, and raffle weights (.42/.28/.04/.26). |
| Palette raffle per node | world.wgsl:963-987 | Cumulative-weight roll over `PALETTE_WEIGHT` picks a dominant palette per node; weights hard-set 0.85/0.05×3. |
| Hermite lattice blend (shared idiom) | world.wgsl:941-953 | `lattice_coord`/`lattice_weight` give C1-continuous bilinear blending used by every field below. |
| Palette field at position | world.wgsl:999-1008 | 2×2 Hermite blend of node raffles at `PALETTE_LATTICE_SPACING` = 300 wu (:1487). |
| Smooth palette color | world.wgsl:1183-1190 | Weighted LIGHT→CENTER mix by complexity, no per-cell noise; the only palette-color function actually called (:4942, :6840, :7010). |
| Discrete palette color (dormant) | world.wgsl:1194-1208 | Adds per-cell noise (props 600-602) scaled by `PALETTE_VARIANCE` — **defined but never called**; the variance table is dead in the live path. |
| Mode field + bias | world.wgsl:992-996, 1011-1020 | Per-node roll raised to `MODE_BIAS_EXPONENT` = 5 (:1490) so ~83% of world is smooth; 120-wu lattice (:1488). |
| Mode gates | world.wgsl:6882-6904 | Blend window [T−0.15, T+0.05], scatter survival [T−0.35, T+0.05], sparse threshold 0.22, mode-zone cutoff at T−0.35 — all off `MODE_DISCRETE_THRESHOLD` 0.70 (:1489); duplicated verbatim in the biased variant :6909-6936. |
| Transition style (blend vs scatter) | world.wgsl:1025-1045 | Trimodal per-node commit (25% blend / 30% hybrid / 45% scatter) on a 200-wu lattice, mixed at :6895. |
| Sparse scatter field | world.wgsl:1048-1081 | Cubic-biased base envelope (160 wu) × cluster hot-spots (40 wu, boost 1+2c at :1079) puts isolated discrete cells on smooth ground. |
| Discrete-region free raffle | world.wgsl:1215-1223, 1238-1254 | Per 80-wu node: free RGB mean (props 800-802) + variance 0.02-0.25 (prop 803), Hermite-blended. |
| Mono tendency | world.wgsl:1227-1235, 1257-1273 | pow-20 roll on a 250-wu lattice makes B&W cell zones nearly extinct. |
| Chess field | world.wgsl:1127-1179 | pow-25 tendency plus two free-rolled region colors on a 55-wu lattice. |
| Per-cell tier cascade | world.wgsl:1276-1320 | Chess gate (tendency+jitter > 0.45; colorful only > 0.65) → pure B&W (> 0.35) → tinted mono (> 0.20) → full color = region mean + cell noise (props 840-842) × variance. |
| Tier bypass (musical drift) | world.wgsl:1329-1359 | `discrete_cell_color_at_tier` re-renders a cell as any of the 5 tiers, skipping the cascade. |
| Field evaluation + coupling | world.wgsl:6829-6877 | `evaluate_cell_fields` fills `CellFieldState` (:6815); per-cell rolls props 900/910 (:6851-6852); terrain-mode coupling (:1092-1122) applied at :6870-6874 but neutralized — `MODE_COUPLING_MAGNITUDE` 0.0 DISABLED (:89). |
| Bake kernel | world.wgsl:7066-7105 | `generate_patch_cells` seeds each 3.125-wu cell (band 200u, :7087), stores composited RGB + GoL behavior tag (:7100) and the mode/style/sparse LUT (:7104); `PATCH_CELL_N` 16 / `PATCH_EXTENT` 50 (:253-254). |
| Runtime re-composite | world.wgsl:6959-7035, 3591-3603 | `animated_cell_color(_lut)` re-derives with `mode_color_shift`/`mode_checker_scatter`/`mode_palette_intensity` biases (DesignConfig :1410-1414) — all flagged DRIVERLESS since gen-1 retirement (:6966-6977); FS guard at :3593-3596. |
| GoL adoption | world.wgsl:4933-4952, 5154-5155, 5242-5244 | `gol_composite_cell_color` re-runs the exact field stack at matching cell size (:4931) so extruded GoL cells match the ground (:7563); zone color mode is its own cumulative raffle over `GOL_COLOR_WEIGHTS`. |

**Checker/ribbon skin adoption (note only).** The sky-ribbon CONTRAST skin re-uses the grammar's idioms rather than its fields: per-cell parity between two authored medians (`ribbon_vs`, world.wgsl:4523-4538 — same two-color parity shape as the chess tier :1281-1288), per-cell `hash_property` draws for hue (:4536) and value jitter (:4557), the CB-1e chroma reconstruction constants `CHECKER_CHROMA_DIR`/`FLOOR` (:4404-4406) with `hue_rotate` (:4411), gated on `color_mode == 2` (:4562-4563); state lives in `RibbonState.checker_scatter`/`color_b` (:834, :849).

**Standing records.** The color grammar has no dedicated standing record: `audit/INVESTIGATION_mood_seam.md` greps empty for palette/checker/discrete, and `audit/RADIUS_INVENTORY.md`/`audit/REACH_GRAPH.md` touch color only tangentially (blend state at RADIUS_INVENTORY.md:167; agent body palette at REACH_GRAPH.md:228). The relevant history is in `audit/SWEEP_CLOSEOUT.md:38` and `audit/CC_AUDIT_REPORT.md:312` — the palette slot-3 identity fix ("3=grey" → "3=warm", authority world.wgsl:1465).

### Section B — The Twins: every CPU/GPU duplication of ground math

**Doctrine and precedent.** The board's constitution is explicit: "GPU is sovereign. CPU dead-reckoning exists only for placement, picking, and step decisions; the visual reality is here" (world.wgsl:20-26). The frame-law precedent for how drift announces itself is the ribbon mount mirror: "Drift is SELF-ANNOUNCING: the rider visibly leans differently than the face beneath it" (ribbon.inl:133-137, the MOUNT_TANGENT_ALIGN/BANK_GAIN/BANK_MAX lockstep mirrors of world.wgsl's ring motor). Every pair below is graded against that standard: what does the player *see* when the two sides disagree?

| # | Pair | GPU authority | CPU twin | Byte-law | Drift severity |
|---|---|---|---|---|---|
| B.1 | Pawn ground resolve | world.wgsl:5380 | **none (verified absence)** | n/a — temporal lag only | low (already patched once) |
| B.2 | Ribbon altitude vs. bake | world.wgsl:2330, 6706 | spawn_engine.inl:1125 | declared approximation | medium, budgeted 25 wu |
| B.3 | Pier buffer mirror | world.wgsl:1905-1961 | cartridge.hpp:496 | exact bit-copy, single writer | near-zero |
| B.4 | Pyramid mirror + regen AABB | world.wgsl:1985-2039 | entities.inl:651; entity_pipeline.inl:1581, 2236 | copy exact; AABB dead-reckoned | low; arch AABB PLAUSIBLE gap |
| B.5 | seed_utils FXC mirrors | world.wgsl:282-412 | seed_utils.inl:32-77 | bit-exact ints, ULP floats | latent, catastrophic if broken |
| B.6 | GoL zone dual authorship | world.wgsl:5179, 7038 | gol_zones.inl:317-449, 550-566 | bit-exact rolls + twin tables | high maintenance burden |
| B.7 | Ground policy registry | world.wgsl:2097-2151 | ground_architecture.inl:102-303 | manual mask mirror, CPU-only asserts | the constitutional seam |
| B.8 | Patch origin / addressing | world.wgsl:6711, 7766 | cartridge.hpp:2471 | algebraic (not bit) equality | the mood-seam crack |
| B.9 | Adjacent hardware mirrors | various | various | manual consts | inventory |

#### B.1 — Pawn ground resolve: GPU kernel with NO CPU twin (a verified absence)

**GPU authority.** `pawn_ground_resolve` (world.wgsl:5380-5419) — snap/step-climb/axis-slide over `query_ground_walker_pair` (world.wgsl:2743-2762), called from `behavior_player_controlled` (world.wgsl:5593) inside the dedicated 1-thread kernel `update_player_agent` (world.wgsl:6159-6173). This is the FXC-fragile inlined region: the agent kernel was split in two because FXC inlines every switch branch and the unified kernel's compile hit 48 s (world.wgsl:6088-6103); the walker queries hand-fuse the GoL + suppression loop because a second pass "compounds significantly under FXC loop unrolling" (world.wgsl:2671-2677, 2727-2733).

**CPU twin: none — and that is the finding.** The CPU never computes pawn ground. `PlayerState` holds only `readback_x/z` (cartridge.hpp:241-242); the readback callback copies exactly `pos_x`, `pos_z`, `portal_trigger` (cartridge.hpp:3122-3125). CPU writes of pawn Y are constants at spawn/teardown only (`agents.inl:581` writes `pos_y = 0.0f` and lets the GPU snap it; cartridge.hpp:2795, 3002). Grep for any CPU walker-height evaluation comes back empty.

**Byte-law.** Not arithmetic but *temporal*: every CPU consumer of the pawn (entity cull spawn_engine.inl:408, streaming center cartridge.hpp:3505, gallery cadence gallery.inl:626, ribbon nearest-slot ribbon.inl:1038) runs 1-2 frames behind the rendered pawn (state.hpp:461).

**Drift risk & visible symptom.** The lag already produced one shipped symptom: CPU LOD banding (from lagged readback) vs. a GPU cull gate reading the live pawn disagreed in the ~175 wu annulus, making patches "flicker in/out and z-fight" — fixed by pushing the CPU's yardstick to the GPU as `lod_pawn_x/z` so "both sides partition with the same yardstick by construction" (state.hpp:459-472). That fix is the house pattern for this pair: don't grow a CPU twin; ship the CPU's number to the GPU. Residual symptom if lag grows: entity show/hide and streaming decisions trailing a fast pawn by a stride.

#### B.2 — CPU tile cache + `estimate_terrain_height` vs. the GPU heightfield bake

This candidate splits into an **author channel** (not a twin) and a **true dead-reckoner** (the twin).

**Author channel.** `generate_tile_state` (cartridge.hpp:1933-2100) is the CPU *authority* for per-tile modifiers — archetype roll, `amp_scale`, `height_bias` via `tile_seed` + `cpu_hash_f` (cartridge.hpp:1994-2014) — uploaded through `upload_tile_grid_now` (cartridge.hpp:1894-1925) into the byte-pinned `GPUTileGrid` (state.hpp:478-495, static_asserts at :484, :495). The GPU only *consumes*: `tile_grid_lookup` / `tile_modifiers_at` (world.wgsl:898-933) feed `contrib_static_base_at` (world.wgsl:2293-2297) and the bake. No duplicated math — but two sync contracts ride on it: (a) the out-of-grid default `TileGridEntry(1.0, 0.0, 1.0, 1u)` (world.wgsl:903) must equal the CPU's missing-cache fill (cartridge.hpp:1917-1920) — today it does; (b) the grid re-uploads with a new origin on recenter (cartridge.hpp:1899-1901), which is precisely the "modifier / tile-grid reference that shifts on recenter" suspect the mood-seam record holds open for the finite-world fork (audit/INVESTIGATION_mood_seam.md, "Important caveat" section).

**The dead-reckoner.** `estimate_terrain_height` (spawn_engine.inl:1125-1132): `height_bias + amp_scale * 5.0f` from `tileCache_` — a constant-5-wu stand-in for the entire lattice evaluation. GPU truth for the same point is `ground_formed_with_complexity` = `raw_lattice × amp + bias + piers + pyramids` (world.wgsl:2330-2335), baked per texel by `generate_patch_heights` (world.wgsl:6706-6721). The header is honest: "rough CPU-side approximation… NOT a ground policy query… deliberately kept as the CPU fast path per the ground policy" (spawn_engine.inl:1116-1124). The old full CPU terrain mirror was *deleted*: "GPU is single source of truth for entity ground_y… Only estimate_terrain_height (tileCache_ lookup) survives for ribbon" (cartridge.hpp:501-504).

**Why it exists.** The ribbon head is CPU-side (flight integrator, ribbon.inl:659+) and needs a ground number *now*, without readback latency. Sole consumers: the ribbon altitude birthright + floor (ribbon.inl:1023-1024, 1054-1055). The birthright bakes once, latching on the first *warm* tile via `terrain_tile_warm` (spawn_engine.inl:1138-1142; ribbon.inl:683-697 — cold-cache 0 is "indistinguishable from flat ground by value"), then the flight floor is `ground_y + RIBBON_FLOOR_MARGIN` (25 wu) low-passed over 180 wu of travel (ribbon.inl:735-750, :126-127).

**Byte-law.** Dead-reckoning by declaration — no bit or ULP contract. Error vs. truth is the raw lattice height minus 5.0, i.e. up to roughly a band amplitude (band means: tectonic 15±6, continental 8±3; world.wgsl:322-331), plus every pier/pyramid the estimate cannot see.

**Drift risk & visible symptom.** *The rider is literally the drift test here*: in sky mode the pawn is bolted to the ribbon head (world.wgsl:5522-5548). If the estimate under-reads a crest — or the tile is answered while an overlapping pier/pyramid isn't in the cache's vocabulary at all — the flown ribbon and its mounted rider visibly clip through terrain crests or pier tops; over-read, and a parked ribbon hovers absurdly high over its birthplace forever ("parked ribbons hold this number forever", ribbon.inl:688). The 25 wu floor margin plus the landscape low-pass is the explicit tolerance budget absorbing the twin's error.

#### B.3 — Pier system: `cpuPiers_` vs. `pier_instances`

**GPU authority.** `evaluate_pier` / `structure_height_at` (world.wgsl:1909-1961) over storage binding 26 (world.wgsl:1905); the loop is bounded by uniform `config.pier_count` "to keep FXC happy" (world.wgsl:1952).

**CPU twin.** `GPUPierInstance cpuPiers_[Dim::PIER_TOTAL]` (cartridge.hpp:496; PIER_TOTAL=68, deterministic slot map state.hpp:96-100).

**Why.** The CPU needs pier contents without readback for: `recompute_and_upload_pier_count` (spawn_engine.inl:286-293) and packing arch pier-feet positions into ground entries (render_passes.inl:50-55). Note: the declaration comment claims "dead-reckoning step-height checks" (cartridge.hpp:494-495) — **no such CPU check exists anymore** (step-height lives in the GPU walker policy); comment drift, flagged below.

**Byte-law.** The strongest in the tree: exact bit-copy with single-writer discipline — `write_pier` / `clear_pier` write the *same struct* to mirror and GPU in one call (spawn_engine.inl:271-284, "written through here so cpu/gpu mirrors stay in sync" :254-256), 48-byte static_assert (state.hpp:740). Even mood's hand-rolled portal spawn goes through `write_pier` (mood.inl:928, 937) — no bypass writer found.

**Drift risk & symptom.** Near-zero while the single-writer holds. If a bypass writer ever appears: `pier_count` under-counts → the GPU loop stops early → a pier that *renders* (its arch/column mesh is separate) but contributes no ground → entities and the pawn sink through a visible pier top.

#### B.4 — Pyramid instances: `cpu_pyramids` + the regen-AABB dead-reckoning

**GPU authority.** `evaluate_pyramid` / `contrib_pyramids_at` (world.wgsl:1987-2039) over uniform binding 30 (:1985); baked into the heightfield via POLICY_BAKED_HEIGHTFIELD (world.wgsl:2125-2126, 2333).

**CPU twin.** `entities_state_.cpu_pyramids` — "CPU mirror for heightfield baking" (entities.inl:651, diagrammed at :51). Written and uploaded whole at commit (entity_pipeline.inl:1565-1566); read by ground-entry packing (render_passes.inl:88-95). Byte-law: exact copy (asserts state.hpp:795-802). Same single-writer strength as B.3.

**The real twin math is the regen AABB.** When a pyramid/arch lands, the CPU must predict *which already-baked patches the GPU footprint touches* and mark them `NEEDS_REGEN` (`mark_patches_for_regen`, spawn_engine.inl:863-882). The pyramid path recomputes the GPU's reach exactly — rotated-AABB `(half+blend)·|cos| + (half+blend)·|sin|` (entity_pipeline.inl:1583-1592) matching `evaluate_pyramid`'s reject bound (world.wgsl:2003). The arch-pier path is cruder: `reach = max(pier_half_x, pier_half_z) + edge_blend` around the two feet (entity_pipeline.inl:2236-2241), while the GPU pier's true world-axis extent is `(half_x+blend)|cos| + (half_z+blend)|sin|` (world.wgsl:1916-1923) — for a diagonally rotated pier with comparable half-extents that exceeds the CPU bound by up to ~41 %.

**Drift risk & symptom (PLAUSIBLE, geometry-dependent).** If a rotated pier's blend skirt crosses into a neighboring GENERATED patch inside that shortfall band, the neighbor never re-bakes: a hairline ledge along the patch boundary where the pier ramp is guillotined — visually a cousin of the mood-seam crack but *entity-anchored* (appears next to arches, doesn't move with distance-from-origin). Patch granularity (50 wu, state.hpp:103) makes the window narrow but nonzero.

#### B.5 — seed_utils FXC mirrors, where they feed terrain

**Contract.** "cpu_lattice_node_seed and cpu_sample_gaussian are FXC mirrors — must produce identical bit-for-bit results to the WGSL counterparts" (seed_utils.inl:24-28); world.wgsl:10-12 states the same from the other shore.

**Verified pairs.** `cpu_hash`/`cpu_hash_f` (seed_utils.inl:32-42) ≡ `hash_property` (world.wgsl:282-288): identical constants 747796405 / 2891336453 / 2654435769, identical shifts, identical `/0xFFFFFFFF` normalization. `cpu_lattice_node_seed` (seed_utils.inl:54-62) ≡ `lattice_node_seed` (world.wgsl:404-412): identical 73856093 / 19349663 / 83492791 multipliers and finalizer; the i32→u32 casts are two's-complement on both sides, so negative lattice nodes hash bit-identically. `cpu_sample_gaussian` (seed_utils.inl:71-77) ≡ `sample_gaussian` (world.wgsl:293-302): same Box-Muller, same `GAUSSIAN_PAIR_OFFSET = 1000` (matched by the C++ comment at seed_utils.inl:73), same 1e-6 clamp and ±3σ truncation. `tile_seed` (seed_utils.inl:44-51) has **no WGSL counterpart at all** — tile decisions are CPU-only and travel to the GPU as data (B.2), never re-derived.

**Byte-law, precisely stated.** Bit-exact on the u32 integer path (hash, seed derivation, comparisons of hashes against constants). Only *ULP-identical* through the Gaussian: WGSL `log`/`cos`/`sqrt` carry implementation tolerances that C++ libm does not share, so a CPU and GPU Gaussian from the same (seed, prop) can differ in low mantissa bits. Safe for parameter draws; unsafe if either side ever branches on a Gaussian crossing a razor-thin threshold the other side also tests.

**Where they feed TERRAIN specifically.** (i) The GoL zone rolls, band 250 — the load-bearing case, B.6. (ii) The entity density/theme lattices (cpu_lattice_node_seed with bands 160/170, cartridge.hpp:2029, 2058) are *CPU-only* — the seed-band registry deliberately decorrelates them from GPU bands (cartridge.hpp:955) and no shader evaluates them, so no drift is possible there. (iii) Everything else (ribbon geometry ribbon.inl:1084-1103, entity params entity_pipeline.inl:226, indoor lights mood.inl:337+) shapes geometry, not ground.

**Drift symptom if the hash mirrors broke.** Not subtle: every decision seeded on both sides diverges at once — the B.6 split-brain fires world-wide in a single frame. This pair's drift test is "everything, everywhere," which is why it is the one contract stated as bit-for-bit.

#### B.6 — GoL zones: one seed, three evaluators, two authoritative tables (the deepest live twin)

Ground relevance: `CONTRIB_GOL_ZONES` is real terrain height — `visual × alive_height × height_factor` extrusion (world.wgsl:2045-2065) included in the walker/flyer policies the pawn stands on (world.wgsl:2133-2139).

**The same lattice seed is derived independently at three sites:** CPU spawn selection `cpu_lattice_node_seed(active_seed, nx, nz, 250)` (gol_zones.inl:366); GPU `zone_derive_params` `lattice_node_seed(req.world_seed, node, GOL_ZONE_SEED_BAND=250)` (world.wgsl:5184); and the terrain bake's `tag_cell_behavior` re-rolling the *same* zone decisions per cell (world.wgsl:7044).

**Duplicated decisions, prop-for-prop:** spawn roll 920 (CPU gol_zones.inl:367-370 vs GPU world.wgsl:7047-7048), tier roll 921 (CPU `select_tier` :405/:419 vs GPU cumulative loops world.wgsl:5207-5213 and 7051-7057 — three copies of one bucket walk), height roll 922 (:394-395 vs :7060-7061), tick-period Gaussian 931 — CPU derives it "for tick mask (matches GPU)" (gol_zones.inl:282, :408-410) while the GPU independently re-derives the same draw for animation (world.wgsl:5218-5219, 5261-5262). The per-cell `height_factor` is CPU-seeded via `cpu_sample_gaussian` (gol_zones.inl:566) into a buffer the GPU extrusion reads.

**Duplicated tables and constants, named in-source:** GOL_TIERS/PULSE_TIERS exist whole on both sides — "the GPU renders from these, the CPU seeds/ticks from the twin, so both are authoritative and a tuner must edit both" (world.wgsl:1666-1669; gol_zones.inl:62-64). `MODE_LATTICE_SPACING = 120` (gol_zones.inl:67-72, "hardware mirror… change both sides"). Zone corner snap: CPU `PATCH_CELL_SIZE = 3.125` (gol_zones.inl:73, :383-386) vs GPU `ZONE_DERIVE_CELL_SIZE = 3.125` (world.wgsl:5149, :5189-5190). SPAWN_CHANCE 0.15 / HEIGHT_CHANCE 0.30 (gol_zones.inl:121-122 vs world.wgsl:1618-1619).

**Byte-law.** Bit-exact required on every u32 roll (guaranteed only by B.5's hash mirror); ULP-tolerant on the Gaussians (tick-mask granularity absorbs it); and **two intentional asymmetries** that make the twin loose at the membership edge: the CPU multiplies spawn chance by `adj_mod` (mood × entity_density × theme, gol_zones.inl:320-330, 368) which the GPU cell-tag ignores (raw 0.15, world.wgsl:7048), and the GPU gates on the mode field ≥ 0.5 (world.wgsl:7040) which the CPU **never evaluates** — `MODE_THRESHOLD` is declared (gol_zones.inl:124) but referenced by no CPU code (grep-verified). So cell-tags and live zones are already only *statistically* aligned by design.

**Drift risk & visible symptom.** Split-brain zones: a checkerboard area whose cells are tagged GoL-animated with no live simulation behind them (static haunted tiles), or a live zone whose cells never tag; a tier mismatch gives a zone whose extrusion *height* belongs to one tier and whose tick *rhythm* to another; a tick-period mismatch makes the pawn — standing on the extrusion via POLICY_WALKER — ride height steps that pop out of sync with the visible spring animation. The rider-on-the-cells is this pair's drift test.

#### B.7 — Ground architecture registry: `POLICIES[]` vs. `POLICY_*_MASK` (the constitutional mirror)

**CPU side.** ground_architecture.inl: `ContributorId` (:102-115), `PolicyId` (:117-128), `CONTRIBUTOR_DAG` (:146-153), `POLICIES[]` (:176-252), with compile-time DAG-closure static_asserts per policy (:272-303). Zero runtime reads either way (REACH_GRAPH kinds it VOCABULARY — audit/REACH_GRAPH.md §2).

**GPU side.** The mask consts (world.wgsl:2097-2151), the `query_ground_<policy>` dispatchers (:2678, 2713, 2743, 2775…), and — the sharpest edge — **two hand-fused copies** that bypass the query API for per-texel/per-vertex cost: `ground_formed_with_complexity` ≡ POLICY_BAKED_HEIGHTFIELD (world.wgsl:2319-2335) and `patch_terrain_vs` ≡ walker-minus-suppression (:2270-2282), each carrying an "if the policy's contributor set ever changes, update this function to match" clause.

**Byte-law.** Manual numeric mirror ("must mirror POLICIES[].contributors bitmasks exactly", world.wgsl:2117-2119; ground_architecture.inl:94-99). Machine-checked on the CPU side only — the DAG-closure asserts validate the C++ masks against the C++ DAG, but **nothing compares the WGSL consts to the C++ table**; the WGSL side is enforced by comment and extension checklist (world.wgsl:2247-2268).

**Drift risk & visible symptom.** The contract names it: "Drift would mean query_ground_<policy> evaluates a different contributor set on GPU than what CPU placement believed" (ground_architecture.inl:96-98). Concretely: the rendered terrain is the *baked* set + live VS deformations, while the pawn's Y is the *analytic walker* chain — if either fused copy or a mask gains/loses a contributor unilaterally, the pawn's feet detach from the rendered surface (hover or sink) exactly where that contributor is nonzero. The pawn-against-the-ground is this registry's rider test.

#### B.8 — Patch origin & heightfield addressing: CPU authors, GPU re-derives (the mood-seam twin)

**CPU author.** `make_patch_params`: `origin = ((g + 0.5) · PATCH_EXTENT)`, `resolution = 256`, seed, layer (cartridge.hpp:2471-2482; PATCH_EXTENT=50 state.hpp:103, PATCH_HEIGHTFIELD_N=256 state.hpp:104, byte contract GPUPatchParams=32 B state.hpp:1427 vs the WGSL PatchParams struct world.wgsl:862-871 (critic-corrected cite)).

**GPU consumers — two different reconstructions.** The bake maps texels via `origin + (uv − 0.5)·extent` (world.wgsl:6711-6715). The sampler `sample_terrain_y_at` *independently re-derives* the origin from the grid index — `(f32(gx)+0.5)·cell_extent` (world.wgsl:7766) — and remaps UV to texel centers (:7770-7772), a formula mirrored again in the patch VS (:3537, :3698). One 65×65 vertex heightfield per patch serves both LODs; LOD-1 is only an index buffer stepping by 2 (state.hpp:3016-3041) — confirming the standing record's "one heightfield per patch" claim.

**Byte-law.** Algebraic equality only — never bit equality at shared edges. This is precisely the crack mechanism audit/INVESTIGATION_mood_seam.md pinned: `(origin_A + 0.5·extent)` vs `((origin_A + extent) − 0.5·extent)` round differently at ~1 ULP of the origin's magnitude, so adjacent patches sample the terrain function at different world points along a shared edge.

**Drift symptom.** Already on the record: intermittent hairline patch-boundary cracks, worse with distance from origin, resetting after mood transitions (which reset the pawn to origin). New contribution from this pass: every `sample_terrain_y_at` consumer — the entity Y-correction pass (world.wgsl:7887+) and the photographer/camera terrain clamps (:7835) — inherits the same ULP-scale disagreement between the sampler's re-derived origin and the bake's uploaded one, so far-from-origin entities can sit a hair off their rendered ground even without a visible crack.

#### B.9 — Remaining hardware mirrors, the anti-twin, and comment drift

- **`AGENT_EVICTION_RADIUS = 360`** — CPU const (agents.inl:181-186) manually mirrored as a WGSL const because "the WGSL needs it as a const for FXC inlining" (world.wgsl:6118-6123). Drift symptom: agents evicted at a different ring than the CPU respawn logic expects — population flicker at the world's horizon.
- **The anti-twin, worth imitating:** `FLOATER_EVICTION_RADIUS = 400` is deliberately GPU-only — "no C++ constant mirrors this today. The CPU learns of kernel evictions through the is_active readback, not a duplicated radius" (world.wgsl:6149-6152; readback sync cartridge.hpp:3134-3149). The pair was *eliminated* instead of maintained.
- **Struct byte contracts** (the substrate every pair rides on): world.wgsl §2.1 "must mirror their C++ counterparts in state.hpp byte-for-byte" (world.wgsl:8, 27-33), enforced by CPU static_asserts — GPUTileGrid (state.hpp:484, 495), GPUPierInstance 48 B (:740), GPUPyramidArray (:795-802), GPUAgentState 96 B (:1409-1410), GPUPatchParams 32 B (:1427). Enforcement is one-sided (C++ asserts; WGSL is prose).
- **Cube anchor mirror (adjacent, position not ground):** `activeCubes_[i].cx/cz` is a CPU dead-reckoned estimate of GPU-animated cube anchors, updated "best-effort" (cube_behaviors.inl:406-424, 507-515); the kite-release path explicitly *refuses* to dead-reckon and hands the write to the kernel via a `follow_pawn = 2u` sentinel (:497-516) — the same eliminate-the-twin instinct as the floater radius.
- **Comment drift found (docs lying about the twins):** (1) spawn_engine.inl:55 lists `terrain_cpu.inl` as a dependency — the file does not exist (deleted per cartridge.hpp:501-504; module dir listing confirms). (2) world.wgsl:7885 claims "Blade: excluded (no compute binding — uses CPU terrain mirror)" — stale twice over: blades *are* Y-corrected in `compute_entity_placement` (world.wgsl:7954-7961) and the CPU terrain mirror is gone. (3) cartridge.hpp:494-495 says `cpuPiers_` exists "for dead-reckoning step-height checks" — no such check survives; live readers are pier-count recompute and ground-entry packing (B.3).

#### Synthesis

The tree has exactly **one live arithmetic dead-reckoner of ground** (`estimate_terrain_height`, budgeted by a 25 wu margin and consumed only by the ribbon), two **bit-copy staging mirrors** (piers, pyramids) whose risk sits not in the mirror but in the CPU's *footprint-reach* predictions feeding regen, one **bit-exact hash contract** underwriting everything seeded (seed_utils), one **dual-authority subsystem** that re-rolls the same seed on both processors by design (GoL), one **constitutional mask mirror** checked by machine on only one side (ground architecture), and one **precision twin** already carrying the tree's known visible defect (patch origins / the mood seam). The two healthiest patterns in the codebase are both twin-*eliminations*: push the CPU's number to the GPU (`lod_pawn`), or let the GPU's number flow back by readback instead of duplicating it (floater eviction, cube release sentinel).

### C — The consumers, by query symbol (sweep 1 of 2)

Anchored at `ecc850baa118e0dbb50214304934148b89802ca0`; every line number below re-verified against this tree. Method: function-index of `world.wgsl` (fn/entry-point table), then exhaustive caller grep per query symbol, each call site mapped to its enclosing function and its dispatch/bind path traced to the CPU spine. Organizing key: the policy table `POLICIES` (modules/ground_architecture.inl:176) — every live consumer resolves to one policy, either **analytically** (contributor sum at query time) or via the **baked texture** (POLICY_BAKED_HEIGHTFIELD cache).

#### C.1 — The two roots

All height truth originates in one analytic stack, evaluated GPU-side only:

| Layer | Symbol | Site | Called by |
|---|---|---|---|
| lattice | `terrain_height_at` | world.wgsl:576 | **only** `contrib_static_base_at`:2294 |
| lattice+complexity | `terrain_height_and_complexity` | world.wgsl:589 | **only** `ground_formed_with_complexity`:2331 |
| tile modifiers | `tile_modifiers_at` | world.wgsl:909 | contrib_static_base_at:2295, ground_formed:2332 (sole GPU readers of the CPU tile themes, via `upload_tile_grid_now` cartridge.hpp:1894/1911-1912) |
| piers | `evaluate_pier`:1909 via `structure_height_at`:1953 (loop at 1957) | world.wgsl | contrib_static_base_at:2296, ground_formed:2333 |
| pyramids | `evaluate_pyramid`:1987 via `contrib_pyramids_at`:2031 (loop at 2035) | world.wgsl | ground_formed:2333 + every placement/flyer/walker dispatcher (2604, 2629, 2647, 2680, 2715, 2746, 2777) |
| fused static base | `contrib_static_base_at` | world.wgsl:2293 | all 9 `query_ground_*` dispatchers (2594, 2603, 2616, 2628, 2646, 2679, 2714, 2745, 2776) |
| fused bake source | `ground_formed_with_complexity` | world.wgsl:2330 | **only** `generate_patch_heights`:6717 (the bake) |

The **bake** turns root 1 into root 2: `generate_patch_heights` (world.wgsl:6706, 256×256 texels/patch, 1 analytic eval each) → scratch → `generate_patch_gradients` (6727, finite-difference grad_x/grad_z + complexity) → `textureStore(patch_heightfield_array_write, …, vec4(height, grad_x, grad_z, complexity))` at 6811. CPU driver: `generate_patch_batch` cartridge.hpp:2434 (dispatch 2453), patch frame from `make_patch_params` cartridge.hpp:2471-2483; cadence **per-bake** — distance-sorted, budgeted streaming (cartridge.hpp:3750-3760) plus re-bakes via `mark_patches_for_regen` (spawn_engine.inl:866) triggered by pyramid commit (entity_pipeline.inl:1590) and arch/pier commit (entity_pipeline.inl:2238).

#### C.2 — Analytic policy queries: live consumers (all GPU, all in 0D compute)

| Consumer | Asks | Policy / query symbol | Side | Cadence | Site |
|---|---|---|---|---|---|
| pawn ground resolve (`pawn_ground_resolve` ← `behavior_player_controlled` ← `update_player_agent`) | height + tilt-height pair (step-climb + slide) | `query_ground_walker_pair` ×2-4 | GPU | per-frame | world.wgsl:5385, 5386, 5401, 5405 (resolve); 5593 (caller); 6172 (kernel) |
| pawn tilt/orientation (`terrain_normal_at` ← `behavior_player_controlled`) | normal (3-tap forward difference) | `query_ground_walker_tilt` ×3 | GPU | per-frame | world.wgsl:5350-5352 (taps); 5605 (caller) |
| NPC agent ground snap (`agent_post_step` ← 9 `behavior_*` ← `update_other_agents`) | height | `query_ground_walker_agent` ×1/agent (≤31) | GPU | per-frame | world.wgsl:5486 (query); returns at 5679, 5753, 5798, 5836, 5873, 5917, 5961, 6037, 6070; kernel 6185/6195-6210 |
| camera terrain clamp (`update_camera`) | height (live contributors, incl. aura) | `query_ground_flyer` ×1 | GPU | per-frame | world.wgsl:6279 (rationale 6270-6276) |
| sphere orbit height (`coupling_terrain_to_sphere_orbit_height` ← `compose_sphere_from_orbit_pga` ← `update_sphere`) | height | `query_ground_flyer` ×1/sphere | GPU | per-frame | world.wgsl:2867 (query); 3109 (orbit); 6304 (kernel) |
| cube home altitude (`update_cube`, kite or anchor mode) | height | `query_ground_flyer` ×1/cube | GPU | per-frame | world.wgsl:6570 (kite) / 6575 (anchor) |
| cube terrain-clearance clamp (`update_cube`, at actual xz) | height | `query_ground_flyer` ×1/cube | GPU | per-frame | world.wgsl:6617 |
| GoL zone terrain fallback (`zone_sample_baked_terrain_y` when no covering patch) | height | `query_ground_baked_heightfield` | GPU | per-frame (fallback branch only) | world.wgsl:5308 |

Consumers of the deformation contributors *inside* these queries (contrib_gol_zones_at:2045, contrib_gol_suppression_at:2082, contrib_terrain_waves_at:2382, contrib_radial_pulses_at:2496, contrib_pawn_aura_at_external:2540 / _self:2562) ride along per the policy masks (ground_architecture.inl:176-246) and are not independent consumers — except the direct per-vertex uses in C.5.

#### C.3 — Baked heightfield texture consumers (POLICY_BAKED_HEIGHTFIELD, texture variant)

Texture: `patchHeightfieldArrayTexture_` (state.hpp:1543-1545), rgba16float = (height, grad_x, grad_z, complexity). One heightfield per patch, shared by both LOD meshes (LOD differs only in index buffers, state.hpp:113/~3020).

| Consumer | Asks | Access path (binding) | Side | Cadence | Site |
|---|---|---|---|---|---|
| `patch_terrain_vs` | height + gradient + complexity | `patch_heightfield_array_read` @28, Render Texture BindGroup state.hpp:4969 | GPU | per-frame per-vertex | world.wgsl:3542-3551, 3571 (gradients), 3572 (complexity) |
| `patch_terrain_fs` | normal (from interpolated gradients) + complexity→color | varyings from VS | GPU | per-frame per-fragment | world.wgsl:3580 (normal), 3584+ |
| `shadow_patch_terrain_vs` | height only | @28, Shadow Texture BindGroup state.hpp:4938 | GPU | per-frame (shadow pass) | world.wgsl:3701-3708 |
| `compute_photographer_vp` — snapshot camera above-terrain clamp | height | `sample_terrain_y_at`:7753 via `photo_heightfield` @145 + `patch_grid` @152, Photographer Compute BindGroup state.hpp:5245 | GPU | per-snapshot frame (gallery.inl:1209; renderer.hpp:23) | world.wgsl:7835 |
| `compute_entity_placement` — paintings Y (hybrid: texture + analytic GoL) | height (+`contrib_gol_zones_at`) | `sample_terrain_y_at` ×≤32 | GPU | on-dirty (ground_entries_dirty→placement_dirty, cartridge.hpp:3412-3420) | world.wgsl:7913-7914 |
| `compute_entity_placement` — columns+antennas | height ×≤32 | same, Entity Placement Compute BindGroup state.hpp:5277 | GPU | on-dirty | world.wgsl:7930 |
| `compute_entity_placement` — palms / cacti / blades | height ×≤24 / ≤20 / ≤32 | same | GPU | on-dirty | world.wgsl:7939, 7949, 7959 |
| `compute_entity_placement` — arches (2-point pier-feet min) | height ×2/arch | same | GPU | on-dirty | world.wgsl:7970-7971 |
| `compute_entity_placement` — pyramids (5-point corner min) | height ×5/pyramid | same | GPU | on-dirty | world.wgsl:7989-7993 |
| `zone_mesh_gen_cell` ← `zone_gol_mesh_gen` — GoL extrusion mesh | height ×5/alive cell (4 corners + center) | `zone_sample_baked_terrain_y`:5291 via `zone_heightfield` @163 + `zone_patch_instances`, GoL Zone Compute BindGroup state.hpp:5395 | GPU | per-frame while `gol_state_.zone_count > 0` (cartridge.hpp:3318, 3340-3350) | world.wgsl:7292-7295, 7303 |
| 0D compute passes (update_camera, update_agents, …) | — (capability only) | @145/146/152 bound in Compute Entity BindGroup state.hpp:4775-4788 (layout 3759-3772) | GPU | **currently unread** — no 0D pass calls `sample_terrain_y_at` | comment state.hpp:4775 |

Spatial index producer: CPU rebuilds `patch_grid` (`GPUPatchGrid`, O(1) layer lookup) each stream pass — cartridge.hpp:3849+; `sample_terrain_y_at` returns 0.0 outside the window (world.wgsl:7759-7762). `photo_patch_instances` @144 is retained-but-unused (TODO world.wgsl:7677-7682).

#### C.4 — Second-order ground caches (heightfield → buffers/atlas → entity VS)

`compute_entity_placement` writes its answers twice: into per-family ground buffers (`arch_ground` @147:7697, `column_ground` @148:7709, `pyramid_ground` @149:7721, `plant_ground` @150:7731) and into `entity_ground_atlas` (r32float 256×1, write @151:7734, read @390:4758). Consumers of the atlas — all per-frame per-vertex, all also adding `contrib_terrain_waves_at` live:

| Consumer | Asks | Cadence | Sites (atlas read, wave add) |
|---|---|---|---|
| `arch_vs` / `shadow_arch_vs` | height (+waves) | per-frame | world.wgsl:4081/4083, 4096/4098 |
| `column_vs` / `shadow_column_vs` (columns + antennas) | height (+waves) | per-frame | 4109/4111, 4124/4126 |
| `pyramid_vs` / `shadow_pyramid_vs` | height (+waves) | per-frame | 4137/4139, 4152/4154 |
| `palm_vs` / `shadow_palm_vs` | height (+waves) | per-frame | 9956/9958, 9970/9972 |
| `cactus_vs` / `shadow_cactus_vs` | height (+waves) | per-frame | 10286/10288, 10300/10302 |
| `blade_cluster_vs` / `shadow_blade_cluster_vs` | height (+waves) | per-frame | 10518/10520, 10532/10534 |

CPU feeder (the *upstream* half of this loop): `upload_ground_entries` (render_passes.inl:45-135) packs each family's XZ + `cached_ground_y` (pier-top offset, terrain excluded) — arches from `cpuPiers_` feet (50-55) + cached_ground_y (57), columns/antennas (70, 79), pyramids from `cpu_pyramids` (88-96), plants (115, 122, 129). Cadence: on `ground_entries_dirty` (set by entity commits entity_pipeline.inl:320, pier writes spawn_engine.inl:275/283, spine wrappers cartridge.hpp:1037-1145, and ORed with patch_instances_dirty at cartridge.hpp:3892), consumed at cartridge.hpp:3412-3420.

Drift flag: world.wgsl:7885 says "Blade: excluded (no compute binding — uses CPU terrain mirror)" — stale; blades are Y-corrected at 7954-7962 and read the atlas at 10518.

#### C.5 — Live deformation overlays queried directly per-vertex

| Consumer | Asks | Symbol | Cadence | Site |
|---|---|---|---|---|
| `patch_terrain_vs` | wave height + analytic wave gradient (fused) | `terrain_wave_overlay_with_gradient`:2426 | per-frame per-vertex | world.wgsl:3561, gradients merged at 3571 |
| `patch_terrain_vs` | pulse height | `contrib_radial_pulses_at` | per-frame per-vertex | world.wgsl:3565 |
| `patch_terrain_vs` | aura lift | `sample_pawn_aura`:5057 | per-frame per-vertex | world.wgsl:3557-3558 |
| `zone_extrusion_vs` / `shadow_zone_extrusion_vs` | wave height (+aura for suppression target in main VS) | `contrib_terrain_waves_at`, `sample_pawn_aura` | per-frame per-vertex | world.wgsl:7412/7416, 7490 |
| `zone_extrusion_fs` | aura (ring tint) | `sample_pawn_aura` | per-frame per-fragment | world.wgsl:7465 |
| `wall_painting_vs` / `shadow_wall_painting_vs` | wave height | `contrib_terrain_waves_at` | per-frame per-vertex | world.wgsl:8525, 8577 |
| 12 entity VSes | wave height | `contrib_terrain_waves_at` | per-frame per-vertex | (sites in C.4 table) |

#### C.6 — CPU-side consumers

| Consumer | Asks | Symbol / field | Side | Cadence | Site |
|---|---|---|---|---|---|
| ribbon head mover — altitude birthright latch + floor clamp | height (approx.) | `estimate_terrain_height` (spawn_engine.inl:1125: tileCache height_bias + amp_scale·5) | CPU | per-frame (rendered slot) | ribbon.inl:1023, 1054 (queries); latch 691, floor 735 |
| ribbon head mover — "is the sample truth yet" | warm-validity | `terrain_tile_warm` (spawn_engine.inl:1138) | CPU | per-frame | ribbon.inl:1024, 1055; `alt_baked` 477/678 |
| GPU tile-modifier bridge | height fields (height_bias, amp_scale, activation) | `tileCache_` → `upload_tile_grid_now` | CPU→GPU | on recenter/stream (cartridge.hpp:2659, 3900 on tileGridDirty) | cartridge.hpp:1894, 1911-1913 |
| ground-entry packer | pier-top offset (`cached_ground_y`), pier feet XZ (`cpuPiers_`), pyramid frames (`cpu_pyramids`) | `upload_ground_entries` | CPU | on-dirty | render_passes.inl:50-129 (fields: entities.inl:174/340/410/476/538/601, entity_types.inl:175; writers entity_pipeline.inl:294/462/696/881/1179/1343/1540/2148, mood.inl:961) |
| pier mirror maintenance | pier instance (height/origin) | `write_pier`/`clear_pier` → `cpuPiers_` (cartridge.hpp:496) + GPU pier buffer | CPU | per-spawn/despawn | spawn_engine.inl:271-284 |
| pyramid mirror | pyramid instance (for bake + ground pack) | `cpu_pyramids` (entities.inl:651) | CPU | per-spawn/evict | entity_pipeline.inl:1559-1566; cartridge.hpp:1034-1046, 2265 |
| pawn Y on CPU | — | **none** — readback copies only pos_x/pos_z/portal_trigger into `player_` | CPU | per-frame readback | cartridge.hpp:3124-3127 (full `agent_state_.slots` memcpy at 3121 carries GPU pos_y, but the only CPU `.pos_y` touches are boot/teardown writes: cartridge.hpp:2795, 3002; agents.inl:581 spawns at 0) |
| non-height tile reads (for completeness) | spawn gates (density/theme/archetype), not height | `tileCache_` | CPU | per-spawn attempt / per-capture | spawn_engine.inl:124, 673; gol_zones.inl:326; gallery.inl:662, 772 |

The deleted-mirror marker: cartridge.hpp:501-504 — "GPU is single source of truth for entity ground_y… Only estimate_terrain_height (tileCache_ lookup) survives for ribbon." Confirmed by exhaustive grep: ribbon.inl:1023/1054 are its only callers, and estimate_terrain_height's own doc (spawn_engine.inl:1116-1124) declares it "NOT a ground policy query."

#### C.7 — Frame queries (pawn position as the reference frame)

Height's sibling question — "where is the pawn" — feeds every radius decision. Consumers verified at this HEAD (fuller catalogs in audit/REACH_GRAPH.md §3-Q1 and audit/RADIUS_INVENTORY.md §1):

| Consumer | Asks | Side | Cadence | Site |
|---|---|---|---|---|
| patch stream center | frame (grid cell) | CPU | per-frame | cartridge.hpp:3505-3506 |
| visibility/LOD band pack | frame (wu, patch-AABB metric) | CPU | per-frame | cartridge.hpp:3790-3826 |
| `lod_pawn` sync to GPU | frame | CPU→GPU | per-frame | cartridge.hpp:3845-3847; GPU consumer `frustum_cull_patches` LOD0 gate world.wgsl:8107-8111 |
| entity draw cull | frame (point Euclidean) | CPU | per-frame | spawn_engine.inl:408-410; **RAD-2 has landed** — cull_base = VISIBILITY_CYLINDER_RADIUS − ENTITY_CULL_EDGE_MARGIN = 250 wu (spawn_engine.inl:319, 402); RADIUS_INVENTORY's ENTITY_CULL_BASE:316 is stale |
| floater eviction (spheres/cubes) | frame | GPU | per-frame | world.wgsl:6320, 6505 (radius 6152) |
| NPC agent eviction | frame | GPU | per-frame | world.wgsl:6217-6222 |
| gallery photographer pacing | frame (walk distance) | CPU | per-frame | gallery.inl:626-639 |
| ribbon nearest-slot pick | frame | CPU | per-frame | ribbon.inl:1038-1041 |
| camera/FPV anchor, sun VP | frame | GPU | per-frame | world.wgsl:6256-6266, 6296+ (`compute_pawn_pos`:4719 / `render_pawn_pos`:4741) |

#### C.8 — Dead / reserved query symbols (declared, zero live callers)

| Symbol | Site | Status |
|---|---|---|
| `query_ground_placement_pyramid` / `_painting` / `_vegetation` | world.wgsl:2593 / 2602 / 2615 | no callers; the placement pass uses the cheaper texture+analytic hybrid instead (rationale comment world.wgsl:7876-7879, hybrid at 7913-7914) |
| `query_ground_baked_heightfield` | world.wgsl:2627 | one caller only — the zone fallback (5308) |
| `query_ground_celestial` | world.wgsl:2791 | returns 0.0; symmetry slot, no callers |
| `query_ground_flyer_gradient` | world.wgsl:2802 | no callers |
| `query_ground_walker_gradient` | world.wgsl:2816 | no callers |
| `query_ground_walker_walkable` | world.wgsl:2833 | no callers (referenced only in the doc block 2242) |
| `query_ground_walker` | world.wgsl:2678 | **effectively dead** — its only callers are the two unused wrappers above (2817-2821, 2834-2839); the live pawn path reads `query_ground_walker_pair`/`_tilt`. The comment at world.wgsl:5586 ("single query_ground_walker call") is drift. |
| `contrib_paintings_base_at` / `contrib_vegetation_base_at` | world.wgsl:2305 / 2315 | 0.0 stubs — registry placeholders |

Net shape for the dossier: **all real height/gradient/normal evaluation is GPU-resident**; the CPU participates only as (a) parameter producer (tile themes, piers, pyramids, patch frames), (b) dirty-flag scheduler, and (c) one approximate consumer (ribbon). Gradients have exactly one producer (the bake, pass 2) and one live analytic consumer chain (pawn tilt); normals are reconstructed at exactly two points (terrain FS from texture, pawn orientation from 3-tap tilt).

### C. The Consumers, by module — measured demand on the Surface interface (sweep 2 of 2)

Anchored read-only at `ecc850b`. This sweep walks every subsystem that asks the ground anything and records what it asks, which side of the ground bus answers, and how often. The bus has four sides, all verified at the sites below:

- **A — GPU analytical (live)**: the `query_ground_<policy>` family re-evaluates the contributor chain per call (world.wgsl:2645-2793); the policy vocabulary is declared in modules/ground_architecture.inl:176-252 (POLICIES) over the contributor DAG at :146-153. Includes animated fields (GoL, waves, pulses, aura).
- **B — GPU baked texture**: the per-patch heightfield array baked by `generate_patch_heights` (world.wgsl:6706, evaluating `ground_formed_with_complexity`:2330 = static base + pyramids only, world.wgsl:2621-2631). Sampled via the terrain VS (:3542), `sample_terrain_y_at` O(1) patch_grid lookup (:7753, grid uploaded cartridge.hpp:3856-3887), and `zone_sample_baked_terrain_y` (:5291, analytical fallback `query_ground_baked_heightfield` :5308).
- **B\* — baked relay**: consumers that read a *derived* baked value — the `entity_ground_atlas` r32float texture written by the placement-correction pass (world.wgsl:7734, read at :4758) or a terrain_y carried in a vertex attribute (zone extrusion uv.x, :7411).
- **C — CPU coarse**: `estimate_terrain_height` tile-cache lookup (spawn_engine.inl:1125-1132), explicitly "NOT a ground policy query" (:1116-1124); warmth probe `terrain_tile_warm` (:1138-1142). By design the *only* CPU height path (cartridge.hpp:501-504: "Terrain CPU mirror deleted… Only estimate_terrain_height survives for ribbon").
- **— (XZ-only)**: the consumer asks *nothing* of height.

#### C.1 Consumer table

| # | Consumer | What it asks the ground | Policy / source | Bus | Cadence | Site |
|---|---|---|---|---|---|---|
| 1 | **Pawn resolve** (player kernel) | Standing Y (aura-lifted) + step-climb-safe Y at new/prev XZ; axis-slide candidates when blocked | WALKER + WALKER_TILT via `query_ground_walker_pair` (world.wgsl:2743) | A | Every frame, 1 thread (`update_player_agent` :6159); 2 paired queries happy path, 4 when step-blocked | `pawn_ground_resolve` world.wgsl:5380-5419, called :5593 |
| 2 | **Pawn tilt normal** | 3-point forward-difference (eps 0.5) for body tilt quaternion | WALKER_TILT (excludes self-aura/suppression) | A | Every frame | `terrain_normal_at` world.wgsl:5348-5356, called :5605 |
| 3 | **Pawn aura height** (the walker's aura term) | Nothing sampled — the pawn "knows" it sits at its aura peak: constant `config.pawn_aura_height` | `contrib_pawn_aura_at_self` world.wgsl:2562-2563 | A (scalar) | CPU authors the scalar every frame (presence × profile height) | pawn.inl:141-167 → state.hpp:2380-2381 |
| 4 | **Agents' walking** (GPU agent kernel) | Ground snap after velocity integration; full GoL lift + external aura, no self-suppression | WALKER_AGENT (world.wgsl:2775-2783) | A | Every frame × ≤31 active slots, 1 query each, all ten behaviors funnel through `agent_post_step` | world.wgsl:5461, snap :5484-5486; kernel :6184-6225. CPU spawns at `pos_y = 0` (agents.inl:581) |
| 5 | **Primary camera** | Never-underground clamp: ground + 1.5 clearance at camera XZ | FLYER (live, so animated ridges don't clip) | A | Every frame, 1 query | world.wgsl:6276-6281 in `update_camera` :6227 |
| 6 | **Sphere orbit** | Minimum terrain clearance under orbital Y | FLYER via `coupling_terrain_to_sphere_orbit_height` | A | Every frame × active sphere, 1 query | world.wgsl:2856-2873, applied :3102-3115 from `update_sphere` :6303 |
| 7 | **Cube hover** | (a) home altitude: ground at anchor-or-kite XZ + orbit_height; (b) clearance clamp at *actual* drifted XZ | FLYER ×2 | A | Every frame × active cube, 2 queries | world.wgsl:6562-6577 (home), 6615-6623 (clamp) in `update_cube` :6475 |
| 8 | Floater/agent eviction | Nothing — pawn-XZ radius only | — | — | Every frame | world.wgsl:6320, 6505 (400 wu, :6152); agents :6217-6222 |
| 9 | **Terrain render VS** (+ aura extrusion) | Baked height+gradients per vertex, then live aura extrusion `aura.r × config.pawn_aura_height`, waves, pulses (hand-fused "POLICY_FLYER-ish") | B + live fused (:3494-3513) | B(+A-fused) | Per vertex (65×65 grid × drawn patches), every frame | `patch_terrain_vs` world.wgsl:3516-3576; aura :3555-3558 |
| 10 | **Shadow terrain VS** | Baked height + waves ONLY — omits aura and radial pulses | B + waves | B | Per vertex, every shadow frame | world.wgsl:3684-3713 (sample :3701, sum :3708) |
| 11 | **Entity render + shadow VS** (arch, column/antenna, pyramid, palm, cactus, blade) | Corrected `ground_y` per instance | `entity_ground_atlas` (written by #15) | B\* | Per vertex, every frame incl. shadow variants | reads world.wgsl:4081/4096, 4109/4124, 4137/4152, 9956/9970, 10286/10300, 10518/10532; binding :4758 |
| 12 | **Placement laws** (`negotiate_position` + per-family gates) | **Nothing** — jitter, indoor wall clamp, separation/footprint, registration are all XZ | — | — | Spawn events | spawn_engine.inl:174-238; callers entity_pipeline.inl:277 (7 generic families), ribbon.inl:1259, GoL bypass gol_zones.inl:461-474 |
| 13 | Spawn commits (all families + mood portal) | Nothing — upload `cached_ground_y = 0`, Y deferred to GPU; `ground_y_offset` carries pier/solid offset only | deferred to #15 | — | Spawn/commit events | entity_pipeline.inl:294, 1540, 2148; mood.inl:960-961; offsets entity_types.inl:176, entity_pipeline.inl:1138/1283/2099 |
| 14 | **Ground-entry packing** | Nothing sampled — packs XZ (+ pier-foot XZ from `cpuPiers_` mirror) with offset-only ground_y | CPU pack feeding #15 | — | Dirty-gated: `ground_entries_dirty` set by commits (entity_pipeline.inl:320), pier writes (spawn_engine.inl:275,283), evictions (cartridge.hpp:1037-1145), teardown (:2381), OR patch changes (:3892); consumed :3412-3416 | `upload_ground_entries` render_passes.inl:45-135 |
| 15 | **Placement-correction compute** | Baked height per entity: paintings hybrid (baked + analytical `contrib_gol_zones_at`, NO suppression), column/palm/cactus/blade 1-point, arch 2-point pier-feet min, pyramid 5-point min; writes atlas | B (via `sample_terrain_y_at`) | B | Dirty-gated (`placement_dirty`, cartridge.hpp:3417-3420) | `compute_entity_placement` world.wgsl:7886-7999; painting policy note :7899-7912 |
| 16 | **Ribbon** | (a) coarse ground + tile-warmth at head/anchor XZ; (b) altitude *birthright* baked once at first warm sample (`ground_y + ribbon.height`, latched); (c) flight terrain FLOOR (`ground_y + RIBBON_FLOOR_MARGIN`, low-passed, critically damped pen) | `estimate_terrain_height` + `terrain_tile_warm` | **C** (sole CPU consumer) | Every frame for the rendered slot, 1 estimate + 1 warmth probe | ribbon.inl:1015-1030, 1046-1059 (in `ribbon_frame_tick` :903); bake :683-697; floor :734-751. Spawn anchor Y = 0 (:1308) |
| 17 | **GoL zone siting** | Nothing — lattice-determined XZ, footprint only | — | — | Spawn events | gol_zones.inl:451-474 |
| 18 | **GoL zone mesh gen** | 5 baked samples per cell (4 corners + center; center carried to VS in uv.x) — "exact terrain as rendered" | `zone_sample_baked_terrain_y` (zone_heightfield view, analytical fallback) | B | Every frame while `zone_count > 0` (cartridge.hpp:3318, 3339-3350), per cell × zone | world.wgsl:7271-7311; sampler :5289-5309; bindings :5111-5112 |
| 19 | **Zone extrusion VS** (main + shadow) | Suppression target = carried terrain_y + aura height + wave; shadow variant terrain_y + wave only ("Shadow doesn't have aura texture") | B\* + live aura/waves | B\*(+A) | Per vertex, every frame | world.wgsl:7401-7444 (target :7414-7417); shadow :7482-7505 (:7499-7500) |
| 20 | **Gallery siting** | Nothing — painting `position[1] = 0`, Y from #15; site pacing reads tile *archetype* (theme, not height) | — | — | Gallery events | gallery.inl:1110-1112; archetype read :660-665 |
| 21 | **Photographer (CPU)** | Nothing — capture cadence = cumulative pawn walk distance from `readback_x/z` | — | — | Every frame | `update_photographer` gallery.inl:625-675; capture stamps XZ only :706-707 |
| 22 | **Photographer VP compute** | Camera-above-terrain clamp (+0.1) at eye XZ; bind group has no live contributors, so baked-only by construction | B via `sample_terrain_y_at` | B | Snapshot frames only (world.wgsl:7809-7810; dispatched from `render_snapshot_pass` gallery.inl:1205-1213) | world.wgsl:7828-7836 |
| 23 | **Streaming (allocate/spawn/generate)** | Nothing — center = `floor(readback/PATCH_EXTENT)`, budgets sorted by XZ distance | — | — | Every frame | cartridge.hpp:3505-3506, 3741-3747, 3760-3769 |
| 24 | **LOD banding (CPU)** | Nothing — XZ point-to-patch-AABB distance vs VISIBILITY/LOD0 cylinders | — | — | Every frame | cartridge.hpp:3787-3821 |
| 25 | **GPU frustum cull / LOD0 gate** | Nothing of the live ground — fixed Y slab −50..200 in the patch AABB; LOD0 gate is XZ nearest-edge vs CPU-banded `lod_pawn` | — (fixed band) | — | Every frame × patch | world.wgsl:8021-8023, 8076-8077, 8107-8115 |
| 26 | **Entity distance cull** | Nothing — XZ Euclidean from readback vs `cull_base = VISIBILITY_CYLINDER_RADIUS − 25 = 250` with capped size-*inset* (taller culls earlier, never past the edge) | — | — | Every frame (cartridge.hpp:3897) | spawn_engine.inl:319-322, 395-419 |
| 27 | Pawn aura compute (contributor authoring) | Nothing of height — pawn XZ + heading; samples `gol_composite_cell_color` for tint delta only | — | — | Every frame while `aura_presence > 0` (cartridge.hpp:3355) | world.wgsl:7513-7568 |
| 28 | Orbs / cube_behaviors / input / mood | No ground asks. Orbs are sky-anchored (checked: zero `query_ground_*`/`sample_terrain_*` hits in their kernels); cube corral/kite anchors on agent XZ; mood defers portal Y to #15 and *authors* piers (`write_pier`) rather than querying | — | — | — | mood.inl:960-961; REACH_GRAPH.md §3-Q1 cross-checked |

#### C.2 Load-bearing observations

1. **The CPU is height-blind by design.** The pawn readback is XZ-only — `PlayerState` has `readback_x/z` and no Y (cartridge.hpp:241-242); no `readback_y` exists anywhere in the tree (grep: zero hits). Every CPU consumer of "where is the pawn" (streaming, culls, photographer, ribbon nearest-slot, orb anchor) operates in XZ. The single CPU height query is the ribbon's coarse tile-cache estimate (#16), preserved deliberately (spawn_engine.inl:1116-1124; cartridge.hpp:501-504).
2. **Entity Y is a one-way GPU pipeline**: CPU places XZ (#12) and uploads zero/offset-only ground_y (#13, #14) → `compute_entity_placement` samples the baked heightfield (#15) → `entity_ground_atlas` → per-vertex reads in six families' render *and shadow* VSs (#11). Dirty-gated, not per-frame; correctness after terrain changes relies on `ground_entries_dirty`/`placement_dirty` being ORed with `patch_instances_dirty` (cartridge.hpp:3892-3894).
3. **Two answers to "how high is the ground"**: movers (pawn, agents, camera, sphere, cube) read the *live analytical* policies including animated fields; placement/render/mesh-gen read the *baked* static_base+pyramids texture and re-add animated fields piecemeal per consumer (terrain VS re-adds aura+waves+pulses :3555-3566; painting correction re-adds GoL analytically :7913-7914; zone VS re-adds aura+waves :7414-7420). A Surface interface must preserve that split or unify it knowingly.
4. **Shadows sample height** via three routes — baked heightfield directly (#10), the ground atlas (#11 shadow variants), and carried terrain_y (#19 shadow) — but shadow terrain omits aura + radial pulses (world.wgsl:3708), so shadowed terrain diverges from lit terrain wherever aura/pulses deform it.
5. **Streaming/LOD/cull never consults height** (#23-#26): XZ distances plus a fixed −50..200 Y slab (world.wgsl:8021-8022). No consumer couples visibility to ground shape.
6. **Standing-record deltas found while sweeping** (cite-and-supersede): RADIUS_INVENTORY.md §1's `ENTITY_CULL_BASE = 350` rows are stale — the RAD-2 rebase it prescribed (§2 "Fix classification") is now IN the tree as `cull_base = VISIBILITY_CYLINDER_RADIUS − ENTITY_CULL_EDGE_MARGIN = 250` with re-signed size-inset (spawn_engine.inl:302-322). Two comment drifts: gallery.inl:1202-1203 claims placement correction is "unconditional every frame" (it is dirty-gated, cartridge.hpp:3417-3420); cartridge.hpp:494-495 claims `cpuPiers_` serves "dead-reckoning step-height checks," but its only reads are pier-foot XZ packing (render_passes.inl:50-51) and the count scan (spawn_engine.inl:289).
7. **Per-frame analytical query budget** (the hot demand a Surface must sustain): pawn ≈ 7-11 policy evaluations (2-4 paired + 3 tilt + camera's 1 flyer), + 1 per active agent (≤31), + 1 per sphere, + 2 per cube — each evaluation walking the full live contributor chain (world.wgsl:2645-2783). Everything else is baked-texture bandwidth or event-driven.
### Critic addendum — items recovered by the completeness pass

1. **A ground-deformation consumer both C sweeps missed:**
   `patch_terrain_fs` samples the pawn aura **per fragment** — color
   delta, height-boost brightening, and a **normal perturbation**
   (`aura.r × 0.3` upward) at world.wgsl:3666-3672. Add to the demand
   table as: terrain FS / asks aura field (deformation + normal bias) /
   GPU / per-fragment.
2. **A named twin constant B.6 missed:** `PULSE_ALGORITHM_CHANCE`
   (CPU `0.35f`, gol_zones.inl:79, used :389-390) vs the WGSL const
   `0.35` (world.wgsl:1714-1715, carrying a "must match CPU" comment) —
   and the WGSL side has **zero shader consumers**: a twin whose GPU half
   is dead weight.
3. **Unlisted manual mirrors in the addressing path (B.9):** the WGSL
   patch-geometry consts `PATCH_EXTENT = 50.0`, `PATCH_HEIGHTFIELD_N =
   256`, `PATCH_CELL_N` (world.wgsl:252-255) hand-mirror
   `Dim::PATCH_EXTENT`/`PATCH_HEIGHTFIELD_N` etc. (state.hpp:103-105,
   113) with no cross-check — the substrate constants of every origin/UV
   formula in B.8.
4. **Comment drift:** state.hpp:4778-4782 claims the cached heightfield
   is "`sample_terrain_y_at` consumed by update_camera, update_agents" —
   false at this HEAD; no 0D compute pass calls `sample_terrain_y_at`
   (the bindings are capability-only, as §C.3 records).

---

## D — THE FRAME (the coordinate contract and its known defect)

**The contract as it is.** One absolute world frame, no floating origin,
no rebase, anywhere:

- Patch identity is an unbounded integer grid coordinate `(gx, gz)`;
  a patch's world origin is `((gx + 0.5) · PATCH_EXTENT, (gz + 0.5) ·
  PATCH_EXTENT)` with `PATCH_EXTENT = 50` (make_patch_params
  cartridge.hpp:2474-2476; state.hpp:103).
- The bake evaluates every texel at
  `world_xz = patch_params.origin + (uv − 0.5) × extent`
  (world.wgsl:6711-6715) — absolute coordinates, per patch,
  independently.
- The texture sampler **re-derives** the same origin from the grid index
  — `(f32(gx) + 0.5) · cell_extent` (world.wgsl:7766) — as does the
  terrain VS (:3537) and its shadow twin (:3698): the origin formula
  exists in at least four places, equal algebraically, never bit-equal
  at shared edges (§B.8).
- The streaming reference is the **recenter cursor**:
  `centerX/Z = floor(player_.readback_x/z ÷ PATCH_EXTENT)`
  (cartridge.hpp:3505-3506), cached as `last_center_x/z`
  (cartridge.hpp:570-571, `INT32_MAX` = force full regen). The tile-
  modifier grid re-uploads with a new origin on recenter
  (cartridge.hpp:1899-1901) — the one frame-dependent input to the
  otherwise pure height function.
- The finite-mode world is bounded to `[−fr·50, (fr+1)·50]` wu
  (mood.inl:536-537 et al.); open mode is unbounded, so coordinates grow
  with travel.

**The known internal defect** (standing record:
`audit/INVESTIGATION_mood_seam.md`, mechanism at its lines 49-70).
Adjacent patches compute their shared edge as `origin_A + 0.5·extent` vs
`origin_B − 0.5·extent` — algebraically equal, float32-divergent by
~1 ULP *at the magnitude of the origin*. Far from (0,0), neighbors sample
the height function at measurably different world points along a shared
edge → intermittent hairline cracks that worsen with distance from
origin and reset after mood transitions (which reset the pawn to origin
and reseed the world). §B.8 adds this dossier's new observation: every
`sample_terrain_y_at` consumer — entity Y-correction, the photographer
clamp — inherits the same ULP-scale disagreement between the sampler's
re-derived origin and the bake's uploaded one, so far-from-origin
entities can sit a hair off their rendered ground even where no crack is
visible.

**The theory's framing, stated as given.** Under the Program Theory, the
floating-origin fix is an **L0-internal change**: §C measures that every
consumer speaks queries — `query_ground_*(world_xz)`,
`sample_terrain_y_at(world_xz)`, atlas reads — and none does origin
arithmetic of its own. A rebase (or the narrower canonical-sample-lattice
fix from the investigation's remediation menu) changes the evaluation
frame *inside* those functions; the query surface and its consumers are
untouched. The empirical gate remains **Jean's settling test** (the
refined three-step in INVESTIGATION_mood_seam.md): origin-relative
worsening ⇒ precision confirmed; the finite-world fork redirects to the
tile-grid/modifier-origin path. Nothing is designed here; the defect,
its mechanism, and its L0-internal fix classification are recorded.

---

## E — THE MEASURED SURFACE INTERFACE (the dossier's synthesis)

Derived strictly from §C's demand table. Each query is justified by named
consumers; nothing is included on speculation. The striking measured
fact: the policy dimension is not optional — consumers demonstrably need
*different contributor sets* (a mover must feel the GoL extrusion it
stands on; placement must NOT feel animated fields; the bake must exclude
what it cannot cache) — and that vocabulary already exists as
`POLICIES[]` (ground_architecture.inl:176).

### The minimal query set

| Query | Shape | Justified by (consumers, from §C) |
|---|---|---|
| **height_at(world_xz, policy)** — live | full contributor chain per policy | pawn resolve (walker_pair ×2-4/frame), pawn tilt taps, agents (walker_agent ×≤31), camera clamp (flyer), sphere orbit (flyer), cube hover+clamp (flyer ×2) — ≈7-11 pawn-adjacent + per-entity evaluations per frame, all GPU |
| **height_at(world_xz) — baked** | static base + pyramids, cached per patch | terrain VS/FS (per-vertex/fragment), shadow terrain VS, placement correction (≤32/family, dirty-gated), GoL mesh gen (5/cell/frame), photographer clamp, zone fallback |
| **gradient_at / normal_at** | baked: texture .yz; live: 3-tap analytic | terrain FS normals (baked), pawn tilt quaternion (analytic 3-tap `query_ground_walker_tilt`); no other consumer asks for gradients — the four `*_gradient` query fns are dead (§C.8) |
| **frame_at (the reference body)** | pawn XZ in world coords, CPU readback + GPU live | streaming center, LOD banding, entity cull, photographer pacing, ribbon nearest-slot, orb anchor, GPU evictions — note the CPU is **height-blind by design** (no readback_y exists; §C.2-1) |
| **valid_at / warm_at(world_xz)** | "is the sample truth yet" | exactly one consumer — the ribbon altitude birthright latch (`terrain_tile_warm`, ribbon.inl:1024/1055) — but load-bearing: it gates a write-once bake |
| **coarse_height_at(world_xz)** — CPU | declared approximation (tile bias + amp×5) | exactly one consumer — the ribbon head mover (floor clamp + birthright); preserved deliberately as the only CPU height path (spawn_engine.inl:1116-1124) |
| **Lifecycle notifications** | regen marking + dirty chain | pier/pyramid authors call `mark_patches_for_regen`; entity Y correctness rides `ground_entries_dirty → placement_dirty` (ORed with patch changes, cartridge.hpp:3892); consumers: the whole one-way entity-Y pipeline (§C.2-2) |
| **The authoring surface (inverse direction)** | `write_pier` / pyramid upload / tile-grid upload | L0 is not read-only: columns, antennas, arches, portals stamp piers; pyramids bake in; the CPU tile author feeds the modifier grid. Any Surface interface must carry the stamp channel, not only queries |

Explicitly **not** in the measured set (zero live demand): walkable-region
queries (`query_ground_walker_walkable` — zero callers), celestial
placement, flyer/walker gradient variants, complexity as a consumer-facing
channel (baked and shipped, read by nothing — §A.3), and any CPU query of
the pawn's ground height.

### E1 — Swappability distance (finding, not design)

**Already speaking only the query set (geodesic-ready):** every GPU mover
(pawn, agents, camera, sphere, cube — pure `query_ground_*` calls);
the entity render/shadow VSs (atlas reads); GoL mesh gen and zone VS
(sampler + carried terrain_y); the photographer VP; placement laws
(XZ-only, Y deferred); streaming/cull/LOD *as consumers* (pure XZ
distances — no height coupling, §C.2-5).

**The gap list — reaches into plane-specific internals, ranked by
conversion difficulty (hardest first):**

1. **The addressing substrate itself** — patch grid keyed on global
   integer tiling of a plane, origin formulas in four places, the tile
   grid's recenter-relative origin, the `patch_grid` O(1) LUT. This is
   L0's own internals rather than a consumer reach, but it is where a
   non-planar (geodesic) L0 rewrites everything below the query line.
2. **`upload_ground_entries`** (render_passes.inl:45-135) — hand-packs
   GPU buffers from `cpuPiers_` and `cpu_pyramids` internals and every
   family's field layout; it speaks storage layout, not queries (already
   flagged in REACH_GRAPH §3-Q3).
3. **The ribbon's CPU path** — `estimate_terrain_height` encodes the
   tile-archetype formula (`bias + amp×5`), i.e. a plane-tile concept, and
   `terrain_tile_warm` encodes cache-tile residency; converting means
   giving the Surface a coarse/validity query with the same latency
   contract.
4. **`patch_terrain_vs`'s hand-fused overlay set** — "POLICY_FLYER-ish"
   matching no declared policy (world.wgsl:3494-3513); it must become a
   declared policy for the interface to be honest.
5. **The fused evaluators** (`contrib_static_base_at`,
   `ground_formed_with_complexity`, the walker fusions) — correct today,
   but each is a hand-kept copy of a policy that only comments enforce
   (§B.7); the swap line runs exactly through them.

### E2 — Blockization readiness (finding only — PARKED per Jean's ruling)

The anatomy already exposes natural contributor seams: the six lattice
bands are a **data table** (TERRAIN_BANDS, world.wgsl:322-331); the tile
modifier layer is **CPU-authored data** with one multiply-add semantics;
piers and pyramids are **instance-list stamps** with self-contained
evaluators and max composition; the overlay fields (waves, pulses, aura)
are **additive time-fields** behind uniforms; GoL suppression is a
**clamp** — each already has one function, one input struct, one
composition operator, which is exactly the shape a composable generator
would formalize. The declaration layer for it exists (CONTRIBUTOR_DAG /
POLICIES) but is presently documentation-shaped: nothing reads the
tables (§A2), and the fused evaluators are the anti-seam — three
hand-inlined copies of policy sums that blockization would either have to
preserve (codegen/fusion) or pay for (per-texel dispatch through a
contributor list under FXC). The seams are recorded; none are cut.

---

## Charter status

Executed. The ground is fully understood at this HEAD: anatomy (A),
stamps (A1), declaration-vs-tree (A2), the musical surface (A3), color
(A4), the twins and their drift tests (B), the measured demand (C), the
frame and its defect (D), and the minimal interface with the
swappability and blockization findings (E). Every future terrain
decision — the rebase, the skirts, the blockization, the geodesic dream,
the bass casting — can be made against this dossier instead of against
memory.
