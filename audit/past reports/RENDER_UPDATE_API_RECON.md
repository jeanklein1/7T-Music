> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# RENDER / UPDATE — API SURFACE RECON (read-only; full-surface map)

Campaign: render/update API cleanup — "deeply organizational, least friction for
Jean and Claude to work with." Jean stamped **full API surface map**.

METHOD: three parallel read-only readers, one per layer (renderer dispatch/
pipeline · GPUState bind/upload · frame-loop orchestration), each file:line'd,
then synthesized here. DISPOSITION: **nothing moved, no cuts** — this is the map
that the design conversation rules on. Line numbers are HEAD at recon time
(post-`668e065`); the campaign shifts lines, so any future cut re-greps.

---

## §0 THE ONE-LINE DIAGNOSIS

**There is no single source of truth per pipeline, per binding, or per family.**
The render/update surface is a set of *hand-synced parallel arrays* spread across
four files (`renderer.hpp`, `state.hpp`, `cartridge.hpp`, `render_passes.hpp`).
The C++ compiler links almost none of them: a pipeline is stitched from an
`Entry::` string + a member + a `dispatch_`/`draw_` wrapper + a `tPipe` creation
block + a caller; a binding is an integer typed twice (layout array and group
array) hundreds of lines apart; a family (e.g. palm) is authored at ~10 sites in
4 files. Almost every friction item below is a symptom of that one missing spine,
and almost every hazard is where the missing link is **glaw1-blind at runtime**
(bind-group validation, WGSL↔C++ struct offsets, frame ordering) — failures that
compile clean and surface as a crash, corruption, or a silently-stale frame.

---

## §1 LAYER 1 — DISPATCH / PIPELINE (`renderer.hpp`, 2928 lines)

**Counts (cross-checked):** 70 `Entry::` constants (all referenced) · 64 pipelines
(30 compute + 34 render) · 30 `dispatch_*` · 34 `draw_*` (33 wrappers + 1 shared
helper `draw_shadow_indexed_mesh` @997). Compute is a clean 30↔30 1:1. 70 Entry vs
64 pipelines is expected sharing (`ENTITY_FS` @48 feeds 11 render pipelines;
`WALL_PAINTING_VS` @69, `PATCH_TERRAIN_VS/FS` @42/43 each shared).

**The quadruplication.** Each pipeline = up to 4 hand-synced sites + caller:
`Entry::<NAME>` (22–120) | `xPipeline_` member (168–265) | `dispatch_`/`draw_`
method | `tPipe(...)` creation block | the caller (rp/bodies/cartridge). Render
pipelines add a 6th name (2nd VS/FS entry). Full per-pipeline table lives in the
Layer-1 reader output; the shape is uniform.

**`tPipe` (@156) is a timer/logger, NOT a registrar.** It stamps a
`high_resolution_clock` around a `[&]`-lambda, prints the boot leaderboard entry,
pushes to `pipelineTimings_` (@154), returns the lambda's bool. The lambda
hand-builds the descriptor, sets `desc.…module = shaderModule_` (@149, one WGSL
module compiled once from `world.wgsl`), sets `entryPoint = Entry::XXX`, assigns
the member. **Entry-point strings are the sole C++→shader linkage.** The `tPipe`
`label` is a *fourth, free-text name* for the pipeline (e.g. `"gen_patch_heights"`).
There is **no `rPipe`** — render pipelines are 30–60-line inline descriptor blocks,
several mutating one shared `desc` across consecutive `tPipe` calls (arch→column→
palm→cactus→blade→pyramid @2166–2224), which couples pipelines through local state.

**Dispatch shapes (near-boilerplate, 6 groups):** A `(1,1,1)` group-0-only · B
`(1,1,1)` + group-1 texture (the 5 world updates) · C caller square 2D · D caller
1D · E `(4,4,zone_count)` + zero-guard · F `(MAX_<X>_INSTANCES,…)` mesh-gen. Bodies
differ only by the pipeline member (+ ROSTER gate). The render side already proved
the collapse: 9 shadow wrappers delegate to `draw_shadow_indexed_mesh` (@997).

**FRICTION (Layer 1):**
- **[L1-a] Four ORPHAN render pipelines — built every boot, drawn nowhere.**
  `draw_pyramid` (911), `draw_shadow_pyramid` (1157), `draw_shadow_gallery_frames`
  (1201), `draw_shadow_wall_paintings` (1187) have no caller anywhere in `src/`
  (only echoes in `backup_board/`). Their pipelines (194/207/218/223), Entry
  constants (58/59/66/72), and creation blocks (2216/2751/2442/2546) are dead
  weight behind `ROSTER.pyramid`/`ROSTER.gallery`. **Pyramid is the sharp one:**
  its *compute* half is live (`dispatch_pyramid_mesh_gen` @cartridge.hpp:320
  generates a mesh) but the mesh is **never drawn** — pyramid is absent from the
  main entity list AND `draw_shadow_all`. RECONCILE with earlier terrain work
  (b2b ruled pyramids **CAST — buried occupiers the terrain drapes over via
  `contrib_pyramids_at`, not surface-standers**): the render orphan is very likely
  **by-design** (pyramids ARE terrain, not drawn geometry) — in which case the
  pyramid *mesh-gen* compute + its buffers may ALSO be excavatable. gallery-frame
  & wall-painting shadows: the entities render in the main pass (rp:601/584) but
  cast no shadow — intentional (flat/wall-mounted) or a gap. **Flag for ruling:
  delete-as-dead vs wire-the-missing-call.** This is the RAYMARCH-shaped, pixel-
  identical-gated cut in this subsystem.
- **[L1-b] Add/remove one kernel = 6–8 hand-synced sites across 4 files** (Entry +
  member + dispatch + creation + caller + `pipelines_skipped()` tally @1217–1236 +
  for mesh-gen the `FAMILY_DISPATCH` row @cartridge.hpp:1300–1334 + the wrapper
  pair @cartridge.hpp:316–359). Compiler links none.
- **[L1-c] `pipelines_skipped()` (1217–1235) is hand-totaled arithmetic** (`n +=
  3/5/6/7/…` per family) that must equal the real gated count; the orphan pyramid
  pair is counted in but never draws.
- **[L1-d] Naming drift across the 4–5 names per pipeline:** `COMPUTE_RIBBON_RINGS`
  →`ribbonRingPipeline_`; `compute_`/`_patches` prefixes dropped in members/methods;
  **cube (compute/ROSTER) ↔ monolith (render)** for one entity; **blade** identifiers
  drop "cluster" but the WGSL string values keep it (`"blade_cluster_vs"`).
- **[L1-e] Double ROSTER-gating:** `if constexpr (!ROSTER.x) return;` guards both
  the method AND the creation block (indoor_shell nests it twice @2269/2797).
- **[L1-f] No render-pipeline builder** → copy-pasted descriptor scaffolding +
  shared-`desc`-mutation coupling (reorder hazard).

---

## §2 LAYER 2 — BIND / BUFFER / UPLOAD (`state.hpp`, 5671 lines)

**24 bind-group layout/group pairs** built in `createBindGroups()` (3527–5441; one
extra group rebuilt @1955). Each entry is hand-indexed `entries[k].binding = <int>`.
The two 19-entry groups (Compute Entity @3531/4516; Render Entity @3633/4614, reused
by Photographer @4940) are the worst. Binding NUMBERS are a sparse, semantically
banded, hand-assigned convention (0–2 core; 20–39 terrain; 60 agent; 80 camera;
100/101 floating/traj; 120–122 ribbon; 140–169 photog/placement/zone; 170–172 aura;
180–198 mesh-gen; **render = compute + 200**, comment @4612; 500/501 cull outputs).

**The core hazard [L2-a]: the binding integer is typed twice with no shared
constant** — once in the `BindGroupLayoutEntry` block (~3531+) and once in the
`BindGroupEntry` block (~4516+), hundreds of lines apart, kept in sync only by the
same `k` index carrying the same literal. WebGPU validates the binding *set* at
`CreateBindGroup` (indifferent to array order — proven by Photographer Compute
filling `entries[9]`=binding 80 between `entries[1]` and `entries[2]`, @4049/5026),
so a wrong binding or wrong `std::array<…,N>` size **compiles clean and fails at
runtime**. This is exactly the glaw1-blind re-index that made the RAYMARCH husk a
deferred storage-weld. Adding/removing a binding = edit both `entries[k]` blocks +
bump both `N`s.

**Same buffer bound at different binding numbers across layouts** (no central
registry): `patchInstancesBuffer_` → 340/144/165/340; `orbStateBuffer_` →
400/410/413; `cameraBuffer_` → 80/280; `zoneConfigBuffer_` → 160/32;
`visiblePatchIndicesBuffer_` → 391/500.

**~90 buffer members** (1350–1563), creation split over three regions (2657–2720
main+staging, 2782–3221 index+mesh). Readback set = four `CopySrc→MapRead` staging
pairs (agent @2698, floating @2705, camera @2712, ribbon-rings @2719).

**~60 `upload_*` methods** (1591–2642), each a thin `queue.WriteBuffer(buf, off,
&v, size)`. ~40 differ only by handle/type/stride (candidate: templated
`upload<T>` / `upload_slot<T>`). Partial/offset writers are the risk sub-class:
config sub-writers hardcode literal offsets **124/144/384** duplicated in a
`static_assert(offsetof==literal)` AND the `WriteBuffer` (1645–1666); the ~12 orb
offset-writers (2485–2566) auto-track C++ `offsetof` but have **no assert binding
the field position to WGSL** — a `GPUOrbConfig` reorder silently writes wrong fields.

**Struct mirrors:** ~50 `GPU…` mirrors, ~48 with `sizeof` static_asserts.
**[L2-b] `ArchVertex` (@1195) has NO size assert** — the sole GPU-written / GPU-read
vertex mirror lacking a layout guard, spread across **12 `sizeof(ArchVertex)`
sites** (6 VB allocs, 6 bind-group entry sizes). WGSL drift → silent offset
corruption, no diagnostic. `ShellVertex` (@1204) same, smaller blast. (Contrast:
`MeshVertex` @1188 IS asserted @1280.) **Adding these two asserts is near-free and
closes a live latent hazard — and may itself flag a pre-existing mismatch.**

---

## §3 LAYER 3 — FRAME LOOP (`cartridge.hpp` render/update + `render_passes.hpp`)

**[L3-a] The frame is split across THREE call sites, not one:** `update()`
(`cartridge.hpp:570-799`, CPU + queue-writes, **no encoder**), `render()`
(`807-1198`, GPU encoding into the host's single encoder + more queue writes), and
the host `Finish()`+`Submit()` (`incubator.cpp:254-255`). A maintainer reading only
`render()` **misses half the per-frame sequence** — signal/clock fill, config
staging, the transition machine, and the photographer update all live in `update()`.

**The fragile ordering core** — each guarded ONLY by hand call-order + a comment,
each fails **silently** (no compile/runtime error):
- **[L3-b] O-5b/c staging→upload (`cartridge.hpp:778-786`) — highest.** Every config
  setter must precede `upload_config`; every signal field must precede
  `upload_signal`. A setter placed after the upload is silently dropped for that
  frame. The split `upload_signal` vs `upload_config` doubles the trap.
- O-5a `dt_beats` reads `prev_beats` before the clock advances (573/596/626) —
  reorder = 1-frame beat skew.
- O-5e `clear_input_deltas` last (798) — reorder = input loss.
- Pre-machine seed/bounds (648–662) — teardown re-stages seed; reorder = wrong
  bounds on teardown frames.
- O-7 frustum cull before indirect draws (1184–1187 → consumed rp:461) — data dep
  through the indirect buffer, comment-guarded.
- RC-1/RC-2 (950–966): "respawn touches slots 1+ only" / "corral + eviction touch
  disjoint cube fields" — **disjointness invariants guarded by prose**; a future
  overlap corrupts silently.

  *Enforced-by-mechanism (good, for contrast):* O-6a zone sync→evolve→mesh barrier
  IS the pass boundaries; P5 `world_gen` guard is a real closure compare
  (688/824/866/902); camera-host re-check is a real callback guard.

**[L3-c] Hidden second queue submit.** `flush_zone_derive_requests`
(`gol_zones.hpp:577-587`) creates its **own encoder and Submits** mid-`render()`
(call site `cartridge.hpp:1148`) — invisible at the call site, its barrier
semantics depend on running before the host submit. "One encoder, one submit"
reasoning from `render()` is wrong on any frame with a new GoL derive.

**[L3-d] Three hand-synced draw lists.** `draw_shadow_all` (rp:315-430),
`render_main_pass` (rp:434-618), `render_snapshot_pass` (gallery.hpp:1162-1207) each
enumerate the family list **separately**, each with a *different* bind group
(`render_entity_group` / `shadow_texture_group` / `photographer_render_entity_group`).
A new drawable = 3 coordinated edits; miss one and it silently drops from shadows/
snapshots. (This is the same root as the L1-a shadow orphans.)

**[L3-e] Four-site family addition:** a new family needs a `prepare_mesh` stanza
(cartridge.hpp:1037-1084), a `FAMILY_DISPATCH` row (1299-1336), a wrapper pair
(316-360), and a ROSTER bit — near-identical, hand-written, unlinked.

**[L3-f] ROSTER gated in two idioms across two files:** compile-time
`if constexpr (ROSTER.x)` scattered through `update()`+`render()` vs
`render_passes.hpp` gating the *same* families on **runtime GPU counts**
(`zone_count>0`, `rendered_slot!=UINT32_MAX`, `wall_frame_count`). The residue
check (cartridge.hpp:984-995) exists *because* this split is error-prone.

**render_passes.hpp** = free functions in `t7::the_board` (owns no state), state via
`MachineCtx* c` (9 organ reaches) + **3 out-of-face reaches threaded through the
call site** (the "B law", rp:14-23): `cpuSpotLights_`, `clearColor_`, the
`orbs_state_`/`orbs_deps_` pair.

**Per-frame GPU cost (full ROSTER):** ~14 compute passes + 1 on the hidden derive
encoder; 2 render passes typical (shadow+main), up to ~6 (indoor atlas + snapshot);
normally 1 submit, 2 on a derive frame.

---

## §4 THE CROSS-CUTTING PATTERN — one "family" is authored at ~10 sites in 4 files

Worked example, **palm** (a mid-complexity grounded family), every site that must
agree:

| site | file:line | what |
|---|---|---|
| mesh-gen Entry | renderer.hpp:99 | `PALM_MESH_GEN` |
| mesh-gen member+dispatch+create | renderer.hpp:263/662/1848 | compute pipeline |
| render Entry (VS) | renderer.hpp:100 | `PALM_VS` (+ shared `ENTITY_FS`) |
| render member+draw+create | renderer.hpp:191/860/2183 | render pipeline |
| shadow Entry+member+draw+create | renderer.hpp:101/204/1115/2718 | shadow pipeline |
| mesh-gen bind group | state.hpp:4452 (Palm Mesh Gen, bindings 180-182) | params/VB/IB |
| buffers | state.hpp:~3000+ | palm params/VB/IB members + creation |
| upload | state.hpp:2303 | `upload_palm_mesh_params_slot` |
| prepare-mesh stanza | cartridge.hpp:1037-1084 | dirty-set entry |
| FAMILY_DISPATCH row + wrapper pair | cartridge.hpp:1300-1334 / 316-359 | |
| ROSTER bit | roster.hpp | `ROSTER.palm` |
| main draw call | render_passes.hpp:547 | `draw_palm` |
| shadow draw call | render_passes.hpp:395 | `draw_shadow_palm` |

~13 edit sites, 4 files, **zero compiler-enforced links between them.** Multiply by
~10 grounded families. This is the friction Jean feels; it is one shape repeated.

---

## §5 FRICTION LEDGER (ranked, cross-layer)

**A. glaw1-blind runtime-failure risk (crash / corruption / silent-stale):**
1. **[L2-a]** hand-typed binding integers duplicated across 24 layout/group pairs,
   no shared constant. (The RAYMARCH-husk pain, systemic.)
2. **[L3-b]** O-5b/c staging-setter→upload ordering: setter-after-upload silently
   dropped; the highest-frequency silent trap.
3. **[L2-b]** `ArchVertex`/`ShellVertex` no size assert (GPU-written/read vertex
   format; WGSL drift → offset corruption).
4. **[L2]** offset-writers keyed to layout position (config literals 124/144/384;
   orb `offsetof` cluster with no WGSL-tie assert).
5. **[L3-a/c]** frame split across 3 call sites + hidden 2nd submit → ordering
   reasoning is non-local.

**B. maintenance friction (edit-site multiplication):**
6. **[§4 / L1-b / L3-e]** ~10–13 hand-synced sites per family across 4 files.
7. **[L1-d/e / L3-f]** naming drift + double ROSTER-gating in two idioms.
8. **[L2]** ~40 near-identical `upload_*` wrappers; ~90 buffers with no registry.
9. **[L1-f]** no render-pipeline builder; shared-`desc`-mutation coupling.
10. **[L1-c]** hand-totaled `pipelines_skipped()`.

**C. latent dead / correctness:**
11. **[L1-a]** 4 orphan render pipelines (pyramid render + 3 shadows) built, never
    drawn; pyramid mesh-gen generates an undrawn mesh (likely by-design → also
    excavatable). The pixel-identical-gated cut of this subsystem.
12. **[L3-d]** 3 hand-synced draw lists (shadow/main/snapshot).

---

## §6 ORGANIZATIONAL CANDIDATES (menu — NOT cuts; Jean stamps)

Each tagged with risk class + gate. Ordered value-first within risk tier. Nothing
here is executed; this is the board for the design conversation.

**Tier 0 — near-free safety adds (glaw1-gated, may bug-find):**
- **C1. Add `static_assert(sizeof(ArchVertex)==…)` + `ShellVertex`.** Closes L2-b.
  If it fails, it just found a live latent bug. ~2 lines. Gate: glaw1 (the assert
  IS the check).

**Tier 1 — pixel-identical dead-code cut (RAYMARCH-shaped):**
- **C2. The orphan sweep (L1-a / L3-d).** Delete the 4 orphan draw wrappers +
  their pipelines/entries/creation blocks — pending your ruling on **pyramid**
  (delete-as-CAST vs wire-to-draw) and gallery/wall shadows (intended vs gap). If
  pyramid is CAST-by-design, the pyramid *mesh-gen* compute + buffers likely join
  the cut. Gate: glaw1 + **pixel-identical** rig (these draw nothing today).

**Tier 2 — behavior-identical C++ refactor (glaw1-gated, mechanical breadth):**
- **C3. Per-pipeline single-source-of-truth (L1-b/c/d/e/f).** One table/X-macro row
  per pipeline → derive Entry + member + dispatch + create + label + skip-count.
  Collapses the quadruplication, kills naming drift + double-gating + hand-totaled
  skip. Same pipelines built → glaw1-gated, behavior-identical. Widest C++ churn.
- **C4. Upload helper (L2 #5).** `upload<T>(buf,v)` / `upload_slot<T>(buf,slot,v)`
  collapsing ~40 wrappers. glaw1-gated, behavior-identical.
- **C5. Unified family/draw table (§4 / L3-d/e).** One family enumeration feeding
  shadow/main/snapshot draws + mesh-gen + ROSTER → a new family is one row. Larger;
  glaw1 + rig (pixel-identical).

**Tier 3 — storage/layout weld (glaw1-BLIND runtime → crash-aware gate):**
- **C6. Binding registry (L2-a).** Named binding constants / a table so layout and
  group reference one symbol; optionally a helper that emits layout+group entries
  together. Directly removes the re-index hazard (and unblocks the deferred
  RAYMARCH-husk removal). **Same risk class as the parked storage-weld follow-on —
  gate: app launches + bind-group validation passes, THEN pixel-identical.** The
  TerrainState husk + complexity texel could ride this once the registry exists.

**Tier 4 — ordering made structural (behavior-sensitive → rig):**
- **C7. Staging→upload barrier (L3-b).** A "commit staging" step or a dirty-set the
  uploader drains, so a late setter can't silently drop. Removes the highest-
  frequency silent trap. Gate: rig (behavior-sensitive).
- **C8. Make O-1..O-7 ordering explicit** (types/asserts/structure over comments).
  Lower priority; largest design surface.

**Recommended opening move:** C1 (free safety) + C2 (the orphan sweep, your
RAYMARCH rhythm) as the first cuts, then C3/C4 (behavior-identical ergonomics) as
the backbone, with C6 (binding registry) as the deliberate storage-weld chapter
that also retires the parked husk. C5/C7 are the higher-design items to sequence
after. But this is your board — stamp the order.

---

## §7 DISCIPLINE

Read-only. Nothing moved, no pipelines/bindings/kernels cut, no struct touched.
`git status` clean but for this file. Full stop for the design conversation before
any render/update change.
