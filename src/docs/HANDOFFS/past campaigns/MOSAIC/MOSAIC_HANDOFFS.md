# MOSAIC — trencadís for the mesh-gen families

**Handoff set: CENSUS_MOSAIC → MOSAIC_0 (the probe) → MOSAIC_1 (the painter)**
Design closed in chat, 2026-07-31. Executor: CC. Every gate: Jean.

## Git law

Trunk-based development on master. CENSUS_MOSAIC commits **nothing** — findings are
reported in-session. MOSAIC_0 is **land-gated**: one branch `claude/mosaic-0`, held
until Jean's boot gate passes; on his authorization it merges to master and the branch
dies. MOSAIC_1 commits **directly to master**, one commit per lettered unit, and only
after Jean stamps the probe. CC pushes no tags (proxy 403) — Jean tags gate-passing
states from the design machine. Run `git fetch --unshallow` before any ancestry claim.
Encoding law: LF-only, no BOM, everywhere. Build gate `glaw1` is Jean's; CC never
compiles or runs the project.

## CC register

REPORT; never improvise fixes. Every FIND must match **verbatim** at its **expected
count** before any edit — on mismatch, STOP and report the live text. Blocks marked
**VERIFIED** were read from the live tree during design. Blocks marked
**CENSUS-BOUND(C-n)** must be re-anchored to what census item C-n reports before
execution — for those, the prose beside the block is the contract and the block is the
expected shape.

## The design in one breath

A mosaic body is cut out of a painted volume. The volume has two scales — the
terrain's own structure at the body's size: **passages** (a coarse lattice, ~12 wu,
each raffling a small palette that always seats a near-white binder) and **shards**
(F1 Voronoi, ~0.3 wu, each raffling one palette member and jittering it by the
passage's own variance). Every shard leans its shading normal a little, like a plate
pressed into mortar — that lean is why trencadís glitters. Past ~45 units from the eye
the mosaic dissolves into the body's flat base color: anti-shimmer and the walk's cost
cap in one smoothstep. Per entity, a 16-bit seed rolled at spawn (a sub-roll of the
existing COLOR_OVER hit — plain sandstone never mosaics) rides the existing per-vertex
index channel as `enc = mosaic_seed·64 + slot`. Seed 0 = plain — and every legacy mesh
byte already decodes as plain, so the transition needs no regeneration and no flag day.

**THE PAINT ANCHOR LAW** (candidate L11 — Jean's stamp, not CC's): *evaluate a thing
in the frame that owns it.* Physics is owned by the world → it reads the live position
(the anchor law as already written). Pigment is owned by the body → it reads the mesh
frame: `paint_pos = vec3(world_pos.x, in.pos.y, world_pos.z)` — mesh-authored XZ (the
grounded mesh-gen lift is Y-only), body-relative Y, immune to `ground_y` and the live
card. `world_pos` remains the coordinate of light, fog, and veil. Two coordinates, two
jobs; neither borrows the other's.

What this campaign does **not** touch: no new bindings, no bind-group growth, no
vertex-format growth, no new pipelines, no indirect draws, no shadow-pass shading. The
flagged FXC chain (`pawn_ground_resolve`) is untouched. Flora, shell, pawn, sphere,
monolith, and ribbon VSes are untouched — zero-init opts them out. The ribbon FS fork
stays byte-identical.

---

## PHASE 0 — CENSUS_MOSAIC (read-only; REPORT, then STOP)

Scope: `src/cartridges/the_board/**` and `src/incubator_dual.cpp` only (the JSX
designers are a sketch program — out of census scope). Zero edits, zero builds, zero
commits. Produce the report table, then stop for Jean.

**C-1 — EntityVarying producers.** `grep -n -- "-> EntityVarying" realization/world.wgsl`.
List every producer. Confirm each declares `var out: EntityVarying;` (zero-init is
what lets non-painting families opt out of the new fields with zero edits). Expected:
arch_vs, column_vs, palm_vs, cactus_vs, blade_vs, shell_vs, pawn_vs, sphere_vs,
monolith_vs, the ribbon VS, possibly an orb VS. **STOP** if any producer constructs
the struct without `var out` zero-init.

**C-2 — The FXC banner.** Report the current world.wgsl banner FXC block verbatim
(the live law; the pier-era list is retired). Flag any line naming: fragment-shader
loops or unrolling, const-array dynamic indexing, growth of ArchMeshParams /
ColumnMeshParams, or interstage/varying counts. **STOP** at any conflict and report —
Contingencies §C3 covers the ArchMeshParams case.

**C-3 — GPUDesignConfig C++ tail.** Report the last ~10 fields of `GPUDesignConfig`
(state.hpp) and its sizeof static_assert verbatim. Expected witness: **592**; expected
tail `… float point_bubble_radius; float cube_plasticity; float pawn_tilt_tau; };`.

**C-4 — Config boot pins.** Locate the site where `config_.veil_ring` /
`lod0_radius` / fog defaults are first written (the boot-pin block). Report file +
block verbatim. This anchors M0-a.4.

**C-5 — hash_property.** Report the WGSL definition verbatim and its position
relative to `shade_lit` and `entity_fs`. Expected: `fn hash_property(seed: u32,
prop: u32) -> f32`, defined above shade_lit. The MOSAIC block (M0-b.1) must sit
**after** hash_property and **before** entity_fs (declaration-before-use is the
tree's own noted law — see the pawn-figure helper comment). The default anchor in
M0-b.1 assumes hash_property precedes shade_lit; if it does not, report the earliest
legal insertion span and re-anchor M0-b.1 there.

**C-6 — CMG vertex writes.** In the column mesh-gen kernel: report the vertex-write
helper (name + signature) or the inline index-float write sites (`… i + 9u …`),
whichever exists, with the **count** of sites whose index argument is the slot. Report
the kernel-top lines where `slot` and the params struct bind (anchors M1-c.9).

**C-7 — AMG call sites.** Report every `amg_write_vertex(` call site's final argument
and the count (expected: all pass the slot / gid.x). Report each sub-mesh function's
top where slot + params bind (anchors M1-c.9).

**C-8 — Arch color path.** Report `arch_compute_colors` verbatim and the full
`ArchProp` registry (600-series) verbatim. Confirm which `inst.colors[]` indices the
arch writes (the mesh params read `[3..5]`), and the highest 600-series number in use
(anchors M1-c.4's ArchProp additions — expected free at 650/651).

**C-9 — Active structs + the portal path.** Report `ActiveArch` and `ActiveColumn`
verbatim (grounded.hpp). Report whether the portal force-spawn path writes ActiveArch
field-by-field (→ M1-c.6 adds an explicit `mosaic_seed = 0` there) or resets the
struct wholesale (→ default-init covers it). Note: the GPU side is stale-slot-proof
either way — see the `is_portal` guard in M1-c.7.

**C-10 — The one producer.** Report `build_column_mesh_params_from` verbatim (the Q3
one-producer both the commit and reupload/cull paths share). Anchors M1-c.8.

**C-11 — Sizeof-derived uploads.** Confirm `upload_arch_mesh_params_slot` and the
arch/column params **buffer creations** derive sizes from `sizeof` (the skew-beacon
pattern). Report the creation lines. **STOP** if any size is a literal — that literal
must join the M1-c.7 growth edit.

**C-12 — Ceilings.** Report MAX_COLOR_CHANNELS, the arch/column slot maxima, and
ANTENNA_SLOT_OFFSET. Confirm every slot value that can reach the index channel is
**< 64** (the encode's slot field). If any family's slot space reaches 64, apply
Contingencies §C4 (7-bit slot field) instead of stopping — but report it loudly.

**C-13 — GPUColumnMeshParams C++ tail.** Report the struct's tail (the `_pad128_*`
lines) and its sizeof static_assert verbatim (expected witness 128). Anchors M1-c.7's
pad repurpose.

Report as a table — item / expected / found / match — then STOP for Jean.

---

## PHASE 1 — MOSAIC_0, the probe (branch `claude/mosaic-0`, held)

**Purpose.** The one measurement this campaign's life depends on: FXC's compile cost
for the two riskiest shapes MOSAIC_1 adds to a nine-pipeline fragment shader — the
27-cell Voronoi walk and a runtime-indexed const array — behind a **runtime uniform**
gate the compiler cannot fold. The probe ships dark: at `mosaic_enable = 0` the frame
is visually identical; the compiled cost is the artifact. An override constant would
measure nothing (the branch would fold away) — the gate must be the uniform. The walk
and the medians table land here in their **final** form; MOSAIC_1b replaces only the
consumption site, so nothing measured is later rewritten.

**Jean's gate.** Machine-clean boot, FXC time before/after (the boot is the only FXC
timer; the Chromium Dawn witness validates Tint only). Then one live flip of
`mosaic_enable → 1` to eyeball the debug voronoi on every entity and confirm the walk
executes; flip back. Stamp → merge, branch dies. Kill → branch dies unmerged; see
Contingencies §C1/§C2.

### Commit M0-a — `MOSAIC_0a: config gains the mosaic dials (592→624; dark)`

**M0-a.1 — WGSL DesignConfig tail** (realization/world.wgsl). VERIFIED, expect 1.

FIND:
```
    pawn_tilt_tau: f32,
}
```

REPLACE:
```
    pawn_tilt_tau: f32,
    // ─── THE MOSAIC (MOSAIC_0/1) — trencadís dials ───────────────────
    // Mirror of GPUDesignConfig tail (state.hpp) — GROWTH LAW, same
    // commit, same order; sizeof witness 592 → 624. Rests: Dim::MOSAIC_*
    // (boot pins). enable = master gate (0 = dark; the probe's runtime
    // witness — a uniform, so FXC cannot fold the walk away).
    // shard_size: wu per shard cell (batch-jittered ±30% per entity).
    // passage_scale: the coarse palette lattice (the bench's passages).
    // radius/icing: eye-anchored dissolve band [radius−icing, radius] —
    //   the fog's metric (texel density is a view fact); anti-shimmer
    //   and the walk's cost cap in one smoothstep.
    // facet: per-shard plate lean on the shading normal (the glitter).
    mosaic_enable: f32,
    mosaic_shard_size: f32,
    mosaic_passage_scale: f32,
    mosaic_radius: f32,
    mosaic_icing: f32,
    mosaic_facet: f32,
    _pad624_0: f32,
    _pad624_1: f32,
}
```

**M0-a.2 — C++ twin** (realization/state.hpp). CENSUS-BOUND(C-3): bind the FIND to
the reported tail. The REPLACE appends after `pawn_tilt_tau`, same order as the WGSL
room:
```
            float pawn_tilt_tau;
            // ─── THE MOSAIC (MOSAIC_0/1) — trencadís dials ───────────
            // Mirror of world.wgsl DesignConfig tail — GROWTH LAW (same
            // commit, same order). Rests: Dim::MOSAIC_* via the boot pins.
            float mosaic_enable;
            float mosaic_shard_size;
            float mosaic_passage_scale;
            float mosaic_radius;
            float mosaic_icing;
            float mosaic_facet;
            float _pad624_0;
            float _pad624_1;
```
In the same edit, update the sizeof static_assert **592 → 624** and extend its message
with `(MOSAIC_0: +6 dials +2 pads — Jean OK'd at handoff)`.

**M0-a.3 — The rests** (state.hpp, Dim — beside the veil defaults). VERIFIED, expect 1.

FIND:
```
            static_assert(VEIL_RING_DEFAULT - VEIL_ICING_DEFAULT > LOD0_RADIUS_DEFAULT,
                "VEIL CHAIN: the icing band sits wholly outside the LOD0 core");
```

REPLACE:
```
            static_assert(VEIL_RING_DEFAULT - VEIL_ICING_DEFAULT > LOD0_RADIUS_DEFAULT,
                "VEIL CHAIN: the icing band sits wholly outside the LOD0 core");

            // ── THE MOSAIC (MOSAIC_0/1) — trencadís rests ──
            // SHARD: wu per cell (~10× under the terrain cell 3.125; a
            //   per-entity batch jitters it ±30%). PASSAGE: the coarse
            //   palette lattice — slightly larger than a body. RADIUS/
            //   ICING: eye-anchored dissolve. FACET: plate-lean strength.
            // All live-tunable via config; these are the rests.
            constexpr float MOSAIC_SHARD_SIZE_DEFAULT = 0.30f;
            constexpr float MOSAIC_PASSAGE_DEFAULT    = 12.0f;
            constexpr float MOSAIC_RADIUS_DEFAULT     = 45.0f;
            constexpr float MOSAIC_ICING_DEFAULT      = 15.0f;
            constexpr float MOSAIC_FACET_DEFAULT      = 0.25f;
            static_assert(MOSAIC_RADIUS_DEFAULT < LOD0_RADIUS_DEFAULT,
                "MOSAIC CHAIN: the mosaic band sits wholly inside the full-mesh core");
```

**M0-a.4 — Boot pins.** CENSUS-BOUND(C-4): at the reported boot-pin site, beside the
veil pins, add:
```
            config_.mosaic_enable        = 0.0f;   // dark until Jean's gate
            config_.mosaic_shard_size    = Dim::MOSAIC_SHARD_SIZE_DEFAULT;
            config_.mosaic_passage_scale = Dim::MOSAIC_PASSAGE_DEFAULT;
            config_.mosaic_radius        = Dim::MOSAIC_RADIUS_DEFAULT;
            config_.mosaic_icing         = Dim::MOSAIC_ICING_DEFAULT;
            config_.mosaic_facet         = Dim::MOSAIC_FACET_DEFAULT;
```

### Commit M0-b — `MOSAIC_0b: the 27-cell walk + probe consumption (runtime-gated, dark)`

**M0-b.1 — The MOSAIC block** (world.wgsl). Insert after shade_lit's closing brace —
subject to C-5: the block must sit after `hash_property` and before `entity_fs`.
VERIFIED default anchor, expect 1.

FIND:
```
    return mix(fogged, config.fog_color, veil);
}
```

REPLACE:
```
    return mix(fogged, config.fog_color, veil);
}

// ═══ THE MOSAIC (MOSAIC_0/1) — trencadís for the mesh-gen families ═══
//
// THE PAINT ANCHOR LAW: pigment evaluates in the frame that owns it.
// Physics is the world's → the live position. Paint is the body's →
// paint_pos = (world_pos.x, in.pos.y, world_pos.z): mesh-authored XZ
// (the grounded lift is Y-only), body-relative Y — immune to ground_y
// and the live card. world_pos remains light/fog/veil's coordinate.
//
// Two scales — the terrain's own structure at the body's size:
// PASSAGES (~12 wu) raffle a small palette, always seating the
// near-white binder; SHARDS (~0.3 wu, F1 Voronoi) raffle one member
// and jitter it by the passage's variance. The per-shard normal lean
// is the pressed-plate glitter. The eye-anchored dissolve past
// mosaic_radius (the fog's metric — texel density is a view fact) is
// anti-shimmer and the walk's cost cap in one smoothstep.
//
// Property run 900–916 (hash_property): 900-902 site jitter (cell
// seed), 903 passage K, 904 raffle (shard), 905-907 members (passage),
// 908 variance (passage), 910-912 color jitter (shard), 913 batch
// (entity), 914-916 facet (shard). Cell-folded seeds — disjoint from
// the CPU entity registries by construction.

const MOSAIC_MEDIANS = array(
    vec3(0.16, 0.32, 0.62),   // cobalt
    vec3(0.20, 0.55, 0.58),   // teal
    vec3(0.85, 0.63, 0.25),   // ochre
    vec3(0.42, 0.56, 0.30),   // moss
    vec3(0.72, 0.30, 0.22),   // rust
    vec3(0.88, 0.78, 0.40),   // sun
    vec3(0.35, 0.25, 0.50),   // violet
    vec3(0.60, 0.75, 0.80),   // sky
);
const MOSAIC_WHITE: vec3<f32> = vec3(0.90, 0.88, 0.84);   // the binder
const MOSAIC_VAR_BASE: f32 = 0.03;   // per-shard jitter floor
const MOSAIC_VAR_SPAN: f32 = 0.10;   // + passage-hashed span

// Fold a 3D lattice cell into the hash_property seed space. Spatial-
// hash primes; bitcast keeps negative cells well-mixed. salt
// decorrelates the shard lattice from the passage lattice.
fn mosaic_cell_seed(c: vec3<i32>, salt: u32) -> u32 {
    return (bitcast<u32>(c.x) * 73856093u)
         ^ (bitcast<u32>(c.y) * 19349663u)
         ^ (bitcast<u32>(c.z) * 83492791u)
         ^ (salt * 2654435761u);
}

// F1-only 3×3×3 Voronoi: returns the nearest jittered site's cell seed
// — the shard's identity. No F2: the grout died at design (Güell's
// binder is pale; what separates shards is the shards), and F2 would
// double the walk's register pressure for a line we don't draw.
fn mosaic_shard(p: vec3<f32>) -> u32 {
    let base = vec3<i32>(floor(p));
    var best_d = 1e9;
    var best = 0u;
    for (var dz = -1; dz <= 1; dz++) {
        for (var dy = -1; dy <= 1; dy++) {
            for (var dx = -1; dx <= 1; dx++) {
                let cell = base + vec3<i32>(dx, dy, dz);
                let cs = mosaic_cell_seed(cell, 0u);
                let site = vec3<f32>(cell) + vec3(hash_property(cs, 900u),
                                                  hash_property(cs, 901u),
                                                  hash_property(cs, 902u));
                let dv = site - p;
                let d = dot(dv, dv);
                if (d < best_d) { best_d = d; best = cs; }
            }
        }
    }
    return best;
}
```

**M0-b.2 — entity_fs, probe form** (world.wgsl). VERIFIED, expect 1.

FIND:
```
@fragment
fn entity_fs(in: EntityVarying) -> @location(0) vec4<f32> {
    return vec4(shade_lit(in.world_pos, normalize(in.normal), normalize(in.normal), in.entity_color, 1.0), 1.0);
}
```

REPLACE:
```
@fragment
fn entity_fs(in: EntityVarying) -> @location(0) vec4<f32> {
    var albedo = in.entity_color;
    // MOSAIC_0 PROBE — the FXC witness for the campaign's two compile
    // risks in this nine-pipeline FS: the 27-cell walk and a runtime-
    // indexed const array. Runtime-gated on a uniform (unfoldable), so
    // the COMPILED cost is present while the frame rests byte-identical
    // (mosaic_enable = 0). MOSAIC_1b replaces this consumption with the
    // painter; the walk and the table stay as landed here.
    if (config.mosaic_enable > 0.5) {
        let s = mosaic_shard(in.world_pos / max(config.mosaic_shard_size, 1e-4));
        albedo = mix(albedo, MOSAIC_MEDIANS[u32(hash_property(s, 910u) * 7.999)], 0.7);
    }
    return vec4(shade_lit(in.world_pos, normalize(in.normal), normalize(in.normal), albedo, 1.0), 1.0);
}
```

**Held here.** Push `claude/mosaic-0`, report the two commit hashes, STOP. Jean boots,
times, flips, stamps or kills.

---

## PHASE 2 — MOSAIC_1, the painter (master, only after Jean stamps the probe)

Three commits, each dark until the last, each bisectable. M1-a and M1-b are
world.wgsl only; M1-c is the lockstep C++/WGSL biography commit.

### Commit M1-a — `MOSAIC_1a: the paint anchor channel (dark)`

After this commit every seed is still 0 — nothing changes on screen. Legacy meshes in
flight decode as plain by construction (`enc & 63 = slot`, `enc >> 6 = 0` for any bare
slot < 64), so no regeneration is forced.

**M1-a.1 — EntityVarying** (world.wgsl). VERIFIED, expect 1.

FIND:
```
struct EntityVarying {
    @builtin(position) clip_pos: vec4<f32>,
    @location(0) world_pos: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) entity_color: vec3<f32>,
}
```

REPLACE:
```
struct EntityVarying {
    @builtin(position) clip_pos: vec4<f32>,
    @location(0) world_pos: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) entity_color: vec3<f32>,
    // THE PAINT ANCHOR (MOSAIC_1): pigment coordinates. paint_y is the
    // mesh-authored body Y (in.pos.y) — immune to ground_y + the live
    // card; XZ reuses world_pos (the grounded mesh-gen lift is Y-only).
    // The FS assembles paint_pos = (world_pos.x, paint_y, world_pos.z).
    // mosaic_seed 0 = unpainted — every zero-init VS opts out for free;
    // only arch_vs / column_vs write these today.
    @location(3) paint_y: f32,
    @location(4) @interpolate(flat) mosaic_seed: u32,
}
```

**M1-a.2 — The channel law + decode** (world.wgsl). VERIFIED, expect 1.

FIND:
```
// --- Catenary Arch
struct ArchVertexInput {
    @location(0) pos: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) color: vec3<f32>,
    @location(3) arch_index: f32,   // slot index as float (avoids GPU denorm flush on bitcast<f32>(u32))
};
```

REPLACE:
```
// --- Catenary Arch
struct ArchVertexInput {
    @location(0) pos: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) color: vec3<f32>,
    // THE INDEX CHANNEL (MOSAIC_1): enc = mosaic_seed·64 + slot, as a
    // float (small-int exact; avoids GPU denorm flush on
    // bitcast<f32>(u32)). slot < 64 (census C-12); seed < 65536 →
    // enc < 2^22, f32-exact. Families that never paint write seed 0 —
    // their bytes are unchanged and their VSes keep the plain u32()
    // read (identity on a bare slot: palm/cactus/blade untouched).
    // Painted families (arch, column) and their shadow twins decode
    // via entity_index_decode below.
    @location(3) arch_index: f32,
};

fn entity_index_decode(v: f32) -> vec2<u32> {
    let enc = u32(v);
    return vec2<u32>(enc & 63u, enc >> 6u);   // (slot, mosaic_seed)
}
```

**M1-a.3 — arch_vs.** VERIFIED, expect 1.

FIND:
```
@vertex
fn arch_vs(in: ArchVertexInput) -> EntityVarying {
    let idx = u32(in.arch_index);
    let ground_y = textureLoad(entity_ground_atlas, vec2<i32>(i32(idx) + GROUND_ATLAS_ARCH, 0), 0).r;
    var world_pos = in.pos + vec3(0.0, ground_y, 0.0);
    world_pos.y += sample_live_card(world_pos.xz).x;

    var out: EntityVarying;
    out.clip_pos = render_vp.m * vec4(world_pos, 1.0);
    out.world_pos = world_pos;
    out.normal = in.normal;
    out.entity_color = in.color;
    return out;
}
```

REPLACE:
```
@vertex
fn arch_vs(in: ArchVertexInput) -> EntityVarying {
    let dec = entity_index_decode(in.arch_index);
    let ground_y = textureLoad(entity_ground_atlas, vec2<i32>(i32(dec.x) + GROUND_ATLAS_ARCH, 0), 0).r;
    var world_pos = in.pos + vec3(0.0, ground_y, 0.0);
    world_pos.y += sample_live_card(world_pos.xz).x;

    var out: EntityVarying;
    out.clip_pos = render_vp.m * vec4(world_pos, 1.0);
    out.world_pos = world_pos;
    out.normal = in.normal;
    out.entity_color = in.color;
    out.paint_y = in.pos.y;
    out.mosaic_seed = dec.y;
    return out;
}
```

**M1-a.4 — shadow_arch_vs** (the shadow twin needs the slot for the atlas; it takes
no paint). VERIFIED, expect 1.

FIND:
```
@vertex
fn shadow_arch_vs(in: ArchVertexInput) -> ShadowVarying {
    let idx = u32(in.arch_index);
    let ground_y = textureLoad(entity_ground_atlas, vec2<i32>(i32(idx) + GROUND_ATLAS_ARCH, 0), 0).r;
    var world_pos = in.pos + vec3(0.0, ground_y, 0.0);
    world_pos.y += sample_live_card(world_pos.xz).x;

    var out: ShadowVarying;
    out.clip_pos = render_vp.light_vp * vec4(world_pos, 1.0);
    return out;
}
```

REPLACE:
```
@vertex
fn shadow_arch_vs(in: ArchVertexInput) -> ShadowVarying {
    let dec = entity_index_decode(in.arch_index);
    let ground_y = textureLoad(entity_ground_atlas, vec2<i32>(i32(dec.x) + GROUND_ATLAS_ARCH, 0), 0).r;
    var world_pos = in.pos + vec3(0.0, ground_y, 0.0);
    world_pos.y += sample_live_card(world_pos.xz).x;

    var out: ShadowVarying;
    out.clip_pos = render_vp.light_vp * vec4(world_pos, 1.0);
    return out;
}
```

**M1-a.5 — column_vs.** VERIFIED, expect 1. Identical transformation to M1-a.3 with
`GROUND_ATLAS_COLUMN`: FIND the current column_vs body (the arch_vs shape with
GROUND_ATLAS_COLUMN), REPLACE with the decode + `out.paint_y = in.pos.y;` +
`out.mosaic_seed = dec.y;` additions, exactly as arch_vs.

**M1-a.6 — shadow_column_vs.** VERIFIED, expect 1. Identical transformation to
M1-a.4 with `GROUND_ATLAS_COLUMN`.

### Commit M1-b — `MOSAIC_1b: the painter (dark)`

Still dark: every seed is 0, the gate never opens.

**M1-b.1 — MosaicSample + mosaic_sample** (world.wgsl). Insert after mosaic_shard.
Anchor = mosaic_shard's tail from M0-b.1, expect 1.

FIND:
```
                if (d < best_d) { best_d = d; best = cs; }
            }
        }
    }
    return best;
}
```

REPLACE:
```
                if (d < best_d) { best_d = d; best = cs; }
            }
        }
    }
    return best;
}

struct MosaicSample {
    color: vec3<f32>,
    facet: vec3<f32>,   // per-shard plate lean, unscaled — the FS scales it
}

fn mosaic_sample(paint_pos: vec3<f32>, entity_seed: u32) -> MosaicSample {
    // Per-entity batch: shards from one workshop vary ±30% in size.
    let batch = 0.85 + 0.30 * hash_property(entity_seed, 913u);
    let shard = mosaic_shard(paint_pos / max(config.mosaic_shard_size * batch, 1e-4));

    // THE PASSAGE — the coarse lattice choosing this region's small
    // palette (the bench's green/yellow/white runs). One scale up.
    let pcell = vec3<i32>(floor(paint_pos / max(config.mosaic_passage_scale, 1e-3)));
    let ps = mosaic_cell_seed(pcell, 7u);

    // K ∈ {2,3,4} members: the near-white binder + (K−1) decorrelated
    // picks from the median table (the antenna-drum pattern).
    let k = 2u + u32(hash_property(ps, 903u) * 2.999);
    // Raffle with the binder at double weight (Güell's white share):
    // roll 0..k; 0 and 1 are both the binder.
    let roll = u32(hash_property(shard, 904u) * f32(k + 1u));
    var med = MOSAIC_WHITE;
    if (roll >= 2u) {
        let pick = hash_property(ps, 903u + roll);   // bands 905..907
        med = MOSAIC_MEDIANS[u32(pick * 7.999)];
    }

    // Per-shard jitter scaled by the passage's own variance — the
    // terrain's variance-law shape at the mosaic's numbers.
    let vari = MOSAIC_VAR_BASE + hash_property(ps, 908u) * MOSAIC_VAR_SPAN;
    let jit = (vec3(hash_property(shard, 910u),
                    hash_property(shard, 911u),
                    hash_property(shard, 912u)) - 0.5) * 2.0;

    var s: MosaicSample;
    s.color = clamp(med + jit * vari, vec3(0.0), vec3(1.0));
    s.facet = (vec3(hash_property(shard, 914u),
                    hash_property(shard, 915u),
                    hash_property(shard, 916u)) - 0.5) * 2.0;
    return s;
}
```

**M1-b.2 — entity_fs, final form.** FIND = the probe body from M0-b.2 **verbatim**
(the self-consistent chain), expect 1.

FIND:
```
@fragment
fn entity_fs(in: EntityVarying) -> @location(0) vec4<f32> {
    var albedo = in.entity_color;
    // MOSAIC_0 PROBE — the FXC witness for the campaign's two compile
    // risks in this nine-pipeline FS: the 27-cell walk and a runtime-
    // indexed const array. Runtime-gated on a uniform (unfoldable), so
    // the COMPILED cost is present while the frame rests byte-identical
    // (mosaic_enable = 0). MOSAIC_1b replaces this consumption with the
    // painter; the walk and the table stay as landed here.
    if (config.mosaic_enable > 0.5) {
        let s = mosaic_shard(in.world_pos / max(config.mosaic_shard_size, 1e-4));
        albedo = mix(albedo, MOSAIC_MEDIANS[u32(hash_property(s, 910u) * 7.999)], 0.7);
    }
    return vec4(shade_lit(in.world_pos, normalize(in.normal), normalize(in.normal), albedo, 1.0), 1.0);
}
```

REPLACE:
```
@fragment
fn entity_fs(in: EntityVarying) -> @location(0) vec4<f32> {
    var albedo = in.entity_color;
    let geo_n = normalize(in.normal);
    var n = geo_n;
    // THE MOSAIC (MOSAIC_1) — trencadís cladding, gated per-entity by
    // the seed and globally by the master dial. Eye-anchored dissolve
    // first: past the band the body IS its base color and the walk
    // never runs (the fade is the cost cap). Inside it, the painter;
    // the plate lean rides the SHADING normal only — geo_n stays
    // geometric (the terrain's pre-aura pattern; shadow and backface
    // math unmoved).
    if (in.mosaic_seed != 0u && config.mosaic_enable > 0.5) {
        let fade = 1.0 - smoothstep(config.mosaic_radius - config.mosaic_icing,
                                    config.mosaic_radius,
                                    distance(in.world_pos, render_camera.pos));
        if (fade > 0.001) {
            let paint_pos = vec3(in.world_pos.x, in.paint_y, in.world_pos.z);
            let s = mosaic_sample(paint_pos, in.mosaic_seed);
            albedo = mix(albedo, s.color, fade);
            n = normalize(geo_n + s.facet * (config.mosaic_facet * fade));
        }
    }
    return vec4(shade_lit(in.world_pos, n, geo_n, albedo, 1.0), 1.0);
}
```

`ribbon_fs` stays byte-identical — the ribbon's seed rests 0 until a ruling grants it
a body-local anchor (Horizon).

### Commit M1-c — `MOSAIC_1c: the biography (LIVE)`

The lockstep commit: spawn rolls, param plumbing, mesh-gen encodes. C++ and WGSL
param-struct edits land together (GROWTH LAW). After this commit, newly spawned
arches/columns carry seeds; resident meshes stay plain until natural regeneration —
both decode correctly, no flag day.

**M1-c.1 — EntityInstance** (contracts/entity_types.hpp). VERIFIED, expect 1.

FIND:
```
    float    burial = 0.0f;
    float    colors[MAX_COLOR_CHANNELS]{};
};
```

REPLACE:
```
    float    burial = 0.0f;
    float    colors[MAX_COLOR_CHANNELS]{};
    uint32_t mosaic_seed = 0;   // MOSAIC_1: 0 = plain; 1..65535 rides the index channel
};
```

**M1-c.2 — ColumnProp** (bodies/grounded.hpp). VERIFIED, expect 1.

FIND:
```
    static constexpr uint32_t COLOR_OVER = 740u;
    static constexpr uint32_t COLOR_VAR_R = 741u;
    static constexpr uint32_t COLOR_VAR_G = 742u;
    static constexpr uint32_t COLOR_VAR_B = 743u;
};
```

REPLACE:
```
    static constexpr uint32_t COLOR_OVER = 740u;
    static constexpr uint32_t COLOR_VAR_R = 741u;
    static constexpr uint32_t COLOR_VAR_G = 742u;
    static constexpr uint32_t COLOR_VAR_B = 743u;
    static constexpr uint32_t MOSAIC_ROLL = 750u;   // MOSAIC_1: sub-roll of the COLOR_OVER hit
    static constexpr uint32_t MOSAIC_SEED = 751u;   // MOSAIC_1: 16-bit paint identity
};
```

**M1-c.3 — ArchProp** (grounded.hpp). CENSUS-BOUND(C-8): append at the registry's
tail, next clear decade (expected 650/651 — bind to the reported ceiling):
```
    static constexpr uint32_t MOSAIC_ROLL = 650u;   // MOSAIC_1: sub-roll of the COLOR_OVER hit
    static constexpr uint32_t MOSAIC_SEED = 651u;   // MOSAIC_1: 16-bit paint identity
```

**M1-c.4 — The fractions** (grounded.hpp). Both VERIFIED, expect 1 each.

ColumnConfig — FIND:
```
struct ColumnConfig {
    static constexpr float SPAWN_CHANCE = 0.030f;
    static constexpr float POSITION_JITTER = 0.35f;
};
```
REPLACE:
```
struct ColumnConfig {
    static constexpr float SPAWN_CHANCE = 0.030f;
    static constexpr float POSITION_JITTER = 0.35f;
    static constexpr float MOSAIC_FRACTION = 0.35f;   // MOSAIC_1: of COLOR_OVER hits
};
```

ArchConfig — FIND:
```
struct ArchConfig {
    static constexpr float SPAWN_CHANCE = 0.030f;
    // Per-mood spawn multiplier (Bayesian: prior × mood_factor × adjacency_factor)
    // Position jitter within patch (fraction of Dim::PATCH_EXTENT)
    static constexpr float POSITION_JITTER = 0.35f;
};
```
REPLACE:
```
struct ArchConfig {
    static constexpr float SPAWN_CHANCE = 0.030f;
    // Per-mood spawn multiplier (Bayesian: prior × mood_factor × adjacency_factor)
    // Position jitter within patch (fraction of Dim::PATCH_EXTENT)
    static constexpr float POSITION_JITTER = 0.35f;
    static constexpr float MOSAIC_FRACTION = 0.55f;   // MOSAIC_1: of COLOR_OVER hits
};
```

**M1-c.5 — column_compute_colors roll** (machine/entity_pipeline.hpp). VERIFIED,
expect 1.

FIND:
```
        inst.colors[2] = COLUMN_PALETTE[pal_idx][2] + (cpu_hash_f(inst.seed, ColumnProp::COLOR_VAR_B) - 0.5f) * 0.06f;
    } else {
```

REPLACE:
```
        inst.colors[2] = COLUMN_PALETTE[pal_idx][2] + (cpu_hash_f(inst.seed, ColumnProp::COLOR_VAR_B) - 0.5f) * 0.06f;
        // THE MOSAIC ROLL (MOSAIC_1): a decorated column may be trencadís
        // — a sub-roll of the COLOR_OVER hit; plain sandstone never
        // mosaics. The flat palette color above stays as the dissolve
        // target.
        if (cpu_hash_f(inst.seed, ColumnProp::MOSAIC_ROLL) < ColumnConfig::MOSAIC_FRACTION) {
            inst.mosaic_seed = 1u + (uint32_t)(cpu_hash_f(inst.seed, ColumnProp::MOSAIC_SEED) * 65534.0f);
        }
    } else {
```

**M1-c.6 — arch_compute_colors roll + Active structs + write_actives.**

arch_compute_colors: CENSUS-BOUND(C-8) — inside its COLOR_OVER hit branch, mirror
M1-c.5 exactly with `ArchProp::MOSAIC_ROLL / MOSAIC_SEED / ArchConfig::MOSAIC_FRACTION`,
writing `inst.mosaic_seed`. The sandstone/else path stays untouched.

ActiveArch + ActiveColumn: CENSUS-BOUND(C-9) — each gains
```
    uint32_t mosaic_seed = 0;   // MOSAIC_1 — frozen at spawn; 0 = plain
```

column_write_active — VERIFIED, expect 1 (unique via COLUMN_TIERS):
FIND:
```
    ac.segs_around = COLUMN_TIERS[inst.tier_idx].segs_around;
    ac.shaft_rings = COLUMN_TIERS[inst.tier_idx].shaft_rings;
    ac.tier_idx = inst.tier_idx;
    ac.cached_ground_y = inst.cached_ground_y;
```
REPLACE:
```
    ac.segs_around = COLUMN_TIERS[inst.tier_idx].segs_around;
    ac.shaft_rings = COLUMN_TIERS[inst.tier_idx].shaft_rings;
    ac.tier_idx = inst.tier_idx;
    ac.cached_ground_y = inst.cached_ground_y;
    ac.mosaic_seed = inst.mosaic_seed;
```

antenna_write_active — VERIFIED, expect 1 (unique via ANTENNA_TIERS):
FIND:
```
    ac.segs_around = ANTENNA_TIERS[inst.tier_idx - COLUMN_TIER_COUNT].segs_around;
    ac.shaft_rings = ANTENNA_TIERS[inst.tier_idx - COLUMN_TIER_COUNT].shaft_rings;
    ac.tier_idx = inst.tier_idx;
    ac.cached_ground_y = inst.cached_ground_y;
```
REPLACE:
```
    ac.segs_around = ANTENNA_TIERS[inst.tier_idx - COLUMN_TIER_COUNT].segs_around;
    ac.shaft_rings = ANTENNA_TIERS[inst.tier_idx - COLUMN_TIER_COUNT].shaft_rings;
    ac.tier_idx = inst.tier_idx;
    ac.cached_ground_y = inst.cached_ground_y;
    ac.mosaic_seed = 0u;   // antennas never mosaic (v1) — a recycled slot must not inherit
```

arch_write_active: CENSUS-BOUND(C-8/C-9) — add `ac.mosaic_seed = inst.mosaic_seed;`
beside its cached_ground_y line. Portal force-spawn path: if C-9 reported
field-by-field ActiveArch writes, add `es.arches[slot].mosaic_seed = 0;` there
(one line, CPU-state hygiene; the GPU side is guarded regardless — M1-c.8).

**M1-c.7 — Param structs, both rooms** (state.hpp + world.wgsl, one edit set).

GPUArchMeshParams (C++) — VERIFIED, expect 1: append after `uint32_t is_active;`:
```
            // MOSAIC_1: 0 = plain; 1..65535 rides the index channel as
            // enc = mosaic_seed·64 + slot. GROWTH LAW: 64 → 80 with the
            // WGSL twin, same commit. Zero-init {} keeps every direct-
            // build path (portals) plain by construction.
            uint32_t mosaic_seed;
            uint32_t _pad80_0;
            uint32_t _pad80_1;
            uint32_t _pad80_2;
```
and update the static_assert **64 → 80**, message
`"GPUArchMeshParams must be 80 bytes — keep in sync with world.wgsl::ArchMeshParams (MOSAIC_1: 64 → 80)"`.

ArchMeshParams (WGSL) — VERIFIED, expect 1: append after `is_active: u32,`:
```
    // MOSAIC_1 (GROWTH 64 → 80 with the C++ twin): 0 = plain;
    // enc = mosaic_seed·64 + slot rides the vertex index channel.
    mosaic_seed: u32,
    _pad80_0: u32,
    _pad80_1: u32,
    _pad80_2: u32,
```
and update the header comment `(size: 64 bytes)` → `(size: 80 bytes)`.

GPUColumnMeshParams (C++) — CENSUS-BOUND(C-13): repurpose the first pad, same
offset, sizeof 128 unchanged:
```
            uint32_t mosaic_seed;   // MOSAIC_1 — _pad128_0 repurposed (the indoor_height_cap precedent); 0 = plain
```
(replacing `float _pad128_0;`).

ColumnMeshParams (WGSL) — VERIFIED, expect 1:
FIND:
```
    _pad128_0: f32, _pad128_1: f32, _pad128_2: f32,
}
```
REPLACE:
```
    // MOSAIC_1: _pad128_0 repurposed (the indoor_height_cap precedent) —
    // 0 = plain; enc = mosaic_seed·64 + slot rides the index channel.
    mosaic_seed: u32, _pad128_1: f32, _pad128_2: f32,
}
```

**M1-c.8 — The producers** (spawn_engine.hpp + entity_pipeline.hpp).

build_arch_mesh_params — VERIFIED, expect 1:
FIND:
```
    else {
        p.color_r = a.col_r; p.color_g = a.col_g; p.color_b = a.col_b;
    }
    p.is_active = 1;
    return p;
```
REPLACE:
```
    else {
        p.color_r = a.col_r; p.color_g = a.col_g; p.color_b = a.col_b;
    }
    // Portals are functional markers — always plain (stale-slot-proof).
    p.mosaic_seed = a.is_portal ? 0u : a.mosaic_seed;
    p.is_active = 1;
    return p;
```

arch_write_gpu — VERIFIED, expect 1:
FIND:
```
    mp.color_r    = inst.colors[3]; mp.color_g = inst.colors[4]; mp.color_b = inst.colors[5];
    mp.is_active  = 1;
```
REPLACE:
```
    mp.color_r    = inst.colors[3]; mp.color_g = inst.colors[4]; mp.color_b = inst.colors[5];
    mp.mosaic_seed = inst.mosaic_seed;
    mp.is_active  = 1;
```

build_column_mesh_params_from — CENSUS-BOUND(C-10): add
`p.mosaic_seed = ac.mosaic_seed;` beside its color copies. This is the Q3
one-producer — the commit path (column_write_gpu) and the reupload/cull path both
ride it, and antennas arrive plain through their zeroed ActiveColumn.

**M1-c.9 — The mesh-gen encoders** (world.wgsl). CENSUS-BOUND(C-6, C-7).

CMG kernel: at the reported kernel top, after the params bind, add
```
    // THE INDEX CHANNEL (MOSAIC_1): every vertex of this body carries
    // enc — legacy plain-slot meshes decode identically (seed 0).
    let enc = p.mosaic_seed * 64u + slot;
```
and at every reported index-write site, replace the slot argument with `enc`
(expected count = C-6's report; verify the count matches before editing).

AMG sub-mesh functions: same shape — one `enc` per function after its params bind,
every `amg_write_vertex(…)` final argument `slot`/gid.x → `enc` (expected count =
C-7's report).

**Commit M1-c and report.** After this commit the world is live: new spawns paint,
residents stay plain until natural regeneration, both decode correctly.

---

## THE VISUAL GATE (Jean, machine-clean)

Flip `mosaic_enable → 1` and fly.

- **Unchanged:** flora, pawns, spheres, monoliths, shell, ribbon, portals, antennas,
  terrain, shadows.
- **Painted:** ~55% of decorated arches, ~30% of all columns (0.85 × 0.35). Near
  one: shards ~0.3 wu in batches that differ ±30% between neighbors; palette
  passages changing every ~12 wu (monumental arches carry several up the vault;
  doorways usually one); near-white binder threaded through everything; glitter as
  the camera moves (the facet). Walking away: dissolve to the flat palette color
  across [30, 45] — no pop, no shimmer at range.
- **Dials, live:** shard_size (0.05–1.0), passage_scale (6–30), facet (0–0.5),
  radius/icing, the two MOSAIC_FRACTIONs (rebuild), enable as the A/B switch.
- **METER_1:** main_pass GPU row, enable on vs off, camera parked near a monumental
  arch — the fade band should confine the cost; report the delta.
- On pass: Jean tags from the design machine; stale-comment corrections fold into
  the next handoff as usual.

## CONTINGENCIES

**§C1 — Probe fails outright (FXC hang / unacceptable boot delta).** Branch dies
unmerged. The ruled-fork fallback: a `mosaic_fs` bound to the arch + column color
pipelines only (two FS compiles carry the walk; `entity_fs` reverts verbatim; the
ribbon precedent for a ruled fork). That is its own mini-handoff — do not improvise
it from this document.

**§C2 — Timing suggests the const table, not the walk.** Replace the runtime-indexed
`MOSAIC_MEDIANS` with a select-chain over the 8 entries (mechanical; ~7 selects).
Only on Jean's ruling after a manual bisect of the two probe shapes.

**§C3 — C-2 banner names ArchMeshParams growth.** STOP before M1-c.7. Options for
Jean's ruling: columns-only v1 (arch seed deferred), or packing the seed into
`segs_v`'s high bits (ugly; needs its own decode law). Neither is CC's call.

**§C4 — C-12 finds a slot space ≥ 64.** Widen the slot field to 7 bits: encode
`enc = mosaic_seed·128 + slot`, decode `(enc & 127u, enc >> 7u)` — still f32-exact
(seed ≤ 65535 → enc < 2^24). One constant in the encoder comment, the decode helper,
and both mesh-gen `enc` lines, same commit.

## HORIZON (not this campaign)

- **Ground-median inheritance = the scale column:** passage medians ←
  `discrete_region_at` at the body's foot behind a config flag — the same painting
  law at the ground's spacing. "Independent vs inherited" becomes a spacing, not a
  branch.
- **The music turn:** pass passage medians through `checker_region_median` with
  `checker_resultant` / `checker_music_amount` — architecture joins the music field
  with the terrain's own function; the fields already sit in config.
- **Moving bodies (pawn, ribbon):** the same painter with a body-local `paint_pos`;
  needs an anchor ruling per family.
- **ARCH_PALETTE is still one stone** — growing it is a one-table edit, orthogonal
  to the mosaic.
- **LAWS.md L11** — the paint anchor law, both halves, on Jean's stamp.
- **The panel:** six dials + two fractions + the medians table are the mosaic's
  control-panel column — one home each, the panel a view, not a copy.
