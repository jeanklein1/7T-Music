# THE GROUND CARD — Campaign Plan v2 (post-audit)

Supersedes `ground_card_campaign_v1.md`. Baseline: **CLOSURE_GPU @ bd405d9**
(the trunk, republished under its real name; `COUPLING_SAGA_SWEEP_CHECKERS`
remains as an alias until Jean runs the delete). Evidence base:
`AUDIT_REPORT.md` + `audit/` tooling and outputs on **CLOSURE_GPU_AUDIT
(884d56d)** — cc3/cc4/cc6/cc7 machine outputs, probe_results.json (the Dawn
witness), the CC-1 hunk files. Every v1 **[CC-n]** anchor is resolved below;
claims cite their witness (probe tag or output file).

Standing instruments this audit leaves behind, adopted as permanent gates:
**glaw1 runs in CC's container** (GREEN at baseline and on every probe
commit) and **the Dawn witness** (`audit/probe_dawn_witness.mjs`) validates
every pipeline against every layout without a native boot. From this point,
no handoff returns without both.

Still on-device (Jean): run the committed CC-5 harness in Chrome on the
Windows box (FXC feature query + kernel timings — the one open probe), and
the native boot as each stage's final gate.

---

## §0 Audit disposition — the anchors resolved

| anchor | verdict | consequence |
|---|---|---|
| [CC-1] drift | 16/19 files differ from the SWEEP reference (~1,400 lines) — but the deltas are **landed history** (PREGEN-8 → 289, TILE_GRID_CAPACITY 1024/side 19, Commit C LUT retirement, cmg 190/191 rebirth, runtime-sized `patch_grid`/`zone_patch_instances`). The session snapshot matched the HEAD side. | **No design rework.** Wrong-tree findings discharged: the 169-hardcode (already fixed by runtime sizing), the uniform-stride violations (master-only; HEAD module compiles clean — `probe_results.module.messages = []`). |
| [CC-2] closure | PASS — all six evictee handles touch C++ only via creation/upload/bind-entry (`cc2_grep_log`); `bind::` greps clean; only other residence = stale `backup_board/`. | Tree-closure holds. `backup_board/` flagged for deletion (holds all 7 BOM violations). |
| [CC-3] dead bindings | **Split.** Ribbon 25+26 dead — probe A1 PASS. GoL pier **refuted** — A2 fails: `zone_gol_mesh_gen` reaches piers via the baked sampler's analytic *fallback chain*. Aura tile_grid **refuted** — A3 fails: `compute_pawn_aura → gol_composite_cell_color → evaluate_cell_fields` reads it. The comment I called a fossil was correct; the comment I trusted (CE 145/146 "required by update_camera/update_agents") is the actual fossil. **Bonus harvest: ten more dead entries** (A4–A8, all validated green). | Stage 1 grows; GoL/Aura entries stay until their kernels retire; sequencing corrected in §9. |
| [CC-4] full fan | PASS — each evictee removal fails **all five** agent kernels, `compute_vp` none (25 verbatim errors). | No partial eviction exists; only the Stage-4 three-function rewrite frees the slots. |
| [CC-5] storage textures | All four card-writer kernel shapes compile ~1 ms module / ~5 ms pipeline; `readonly_and_readwrite_storage_textures` **exposed** — on Dawn/Vulkan/SwiftShader. FXC half pending on-device. | Writer pattern validated; paint-stratum residency options stay open pending the Windows run. |
| [CC-6] budgets | Machine counts: CE 9s/7u, Placement 9s, Render Entity 8 VS / 8 FS **summed across groups** (the convention CC rightly enforced), GoL 7s, Ribbon 3s (head_poses is new), Frustum 6s, Photographer 6s. Adapter law confirmed 10s/12u. | §2/§7 regenerated; v1 hand counts held except Ribbon (2→3). |
| [CC-7] mirror | 102 WGSL declarations over 97 slots (5 documented `fc_*` aliases), registry 1:1 clean both directions; **g0:31 and g1:34 FREE**. Banner "108 literals" stale → 102. | Card bindings stand as authored. Banner count joins the sweep. |
| [CC-8] stale/bugs | (a) **LIVE BUG**: `dispatch_frustum_cull` hardcodes 4 workgroups (225-era); at 289, **slots 256–288 escape the cull pass entirely** — needs 5, authored as a derivation. (b,c) confirmed. (d) withdrawn. | Fix #1, own commit, before anything else. *Fix the tree, then count it.* |

**The headline correction the machine forced:** at HEAD,
`sample_terrain_y_at` is reached **only** by the photographer and placement
(`cc4.reverse.patch_grid`). The five agent kernels are *fully analytic* —
CE's photo_heightfield, photo_sampler, and patch_grid entries are dead
plumbing for a baked path the agents never took. So the analytic-tax
finding is *sharper* than v1 stated, and Stage 4 doesn't "keep" the baked
path in CE — it **introduces** it.

## §1 Purpose

Unchanged in substance: re-found the resource substrate; fields migrate to
textures; the per-frame deformation is evaluated **once**, into a card
everything samples. Motif-preserving; rest frames bit-identical. Sorting
law: *position-indexed → texture; identity-indexed → buffer.*
Coupling-agnosticism holds — the conversation since touched §9/§10 only.

Convergent in-tree evidence (`state.hpp:3377`, `LATENT[gate-a-shared]`):
the tree already noted the zone-mesh stack is droppable but the zone pair
is "exclusive-in-Compute-Entity + Entity-Placement. Retire = re-section
both groups." This campaign is that note, executed with a mechanism.

## §2 Budget law and pressure (machine-counted, cc6)

Device law: 10 storage / 12 uniform per stage (banner + adapter probe
agree). Counting convention: per stage, summed across all bound groups.

| layout | storage | uniform | dead inside (probe) |
|---|---|---|---|
| Compute Entity (+CT) | **9** | 7 | 101, 145, 146, 152 (A4); CT nearest 23 (A5) |
| Entity Placement | **9** | 1 | 60, 144 (A7) |
| Render Entity (+RT) | **8 VS / 8 FS** | 6 VS | — |
| GoL Zone | 7 | 4 | none removable pre-UG (A2) |
| Frustum Cull | 6 | 1 | 60, 80 (A8) |
| Photographer | 6 | 2 | 144 (A6) |
| Ribbon | 3 | 2 | 25, 26 (A1) |

A machine-given lens: **pipeline-creation time as complexity proxy**
(SwiftShader/Tint baselines): `update_other_agents` **7.08 s**,
`update_player_agent` 3.91 s, `update_cube` 1.72 s, `zone_gol_mesh_gen`
1.17 s — the analytic ground chain is where shader complexity lives. Not
FXC numbers, but measurable: Stage 4 predicts these collapse, and the
committed harness can measure it.

## §3 The census sort (delta from v1)

Two corrections: `patch_grid` is **dead in CE today** (rejoins as the baked
card's index at Stage 4); `trajectories` (@101) has **zero shader readers
anywhere** (`cc4`) — full-retirement candidate pending a C++ writer check
(§9 Stage 6).

## §4 Findings (machine-backed)

1. **The analytic tax, total** — no baked component in any agent kernel;
   the dead CE texture pair is the fossil of a flip that never happened;
   the tax's size is visible in the compile-time proxy.
2. **The funnel theorem, machine form** (`cc4.reverse`): piers →
   `structure_height_at` alone; pyramids → `contrib_pyramids_at` alone;
   zone pair → `contrib_gol_zones_at` (+ GoL's own kernels); tile_grid →
   `tile_modifiers_at` / `tile_grid_lookup` / `evaluate_cell_fields` (the
   color path pinning it render-side). Three functions carry everything.
3. **Full fan** [CC-4]: every evictee reaches all five agents through
   policy dispatch — eviction is all-or-nothing per the rewrite.
4. **Triple-shadow & live re-derivation** — unchanged, now with exact
   per-entry-point binding maps in `cc4_output`.
5. **The dead-entry harvest** — twelve validated removals across six
   layouts (A1, A4–A8), enumerated in §2.
6. **The frustum-cull bug** [CC-8a] — the one live correctness defect;
   Stage 1 leads with it.
7. **Statically-live fallbacks** — `zone_gol_mesh_gen` reaches
   tile/pier/pyramid only through the baked sampler's analytic fallback:
   dynamically near-dead, statically binding. UNIFIED GROUND retires the
   kernel and the question together.

## §5 The Live Card — specification (confirmed)

As v1 §5, with anchors resolved: **g0:31 write / g1:34 read confirmed
free** [CC-7]; writer kernel shape probe-validated fast [CC-5/C];
`pier_instances = array<PierInstance, 68>` at HEAD (the 48-byte/68-slot law
holds). Channels R=waves+pulses Δh / G,B=∂x,∂z / A=raw GoL; the
writer-calls-existing-evaluators law; 512² over 800 wu (Nyquist-verified);
`config.lod_point` centering **snapped to the cell grid** (the UNIFIED
GROUND prerequisite); spine row between R9 and R10 with two witnesses.

## §6 Consumer rewiring (corrected)

The per-policy composition table stands (v1 §6). Mechanics corrections:

- **Stage 4 introduces the baked path to the agents**: the three contrib
  call sites become `baked_sample + card` compositions; CE *gains*
  photo_heightfield + photo_sampler (no storage cost) and re-livens
  patch_grid (RO storage), while piers, zone_config, zone_life leave
  storage and tile_grid, pyramids leave uniforms.
- **GoL's evictions ride UNIFIED GROUND, not Stage 4** — its
  pier/tile/pyramid reach is the mesh kernel's fallback chain, and that
  kernel retires whole.
- Render-side: `patch_terrain_fs` reaches tile_grid (live-recolor
  archetype, confirmed by cc4) — pinned until Layer C/E.

## §7 Binding deltas (three horizons, from cc6 baselines)

| layout | HEAD | post-sweep (S1) | post-Stage-4 | post-UG (S5) |
|---|---|---|---|---|
| Compute Entity | 9s / 7u | 7s / 7u | **5s / 5u** (+2 sampled, +1 sampler) | 5s / 5u |
| Entity Placement | 9s / 1u | 7s / 1u | **5s / 1u** (+card sampled) | 5s |
| GoL Zone | 7s / 4u | 7s / 4u | 7s / 4u | **2s / 2u** (sync/evolve/derive) |
| Frustum Cull | 6s | 4s | 4s | 4s |
| Photographer | 6s | 5s | 5s | 5s |
| Ribbon | 3s / 2u | 2s / 1u | 2s / 1u | 2s / 1u |
| Render Entity (+RT) | 8/8 | 8/8 | 8/8 (+card in g1) | 8/8 — relief waits for Layer C/E, honestly |

CE storage at campaign's rest: `vp_data, agent_state, camera_state,
floating_entities, patch_grid` — **five under a ten law**.

## §8 Behavior deltas — the disclosure gate

v1's four stand (pulse-shaded normals; walker onto the sampled surface;
single evaluation moment; rest identity) plus the window covenant. Added:
**5.** the frustum fix is a behavior change — correctness restoration
(slots 256–288 become cullable); expected visual delta none, gated by
native boot. **6.** the dead-entry sweep is behavior-neutral *by validator
proof* — every removal already witnessed green, so Stage 1's risk is
clerical, not semantic.

## §9 The staged plan v2

**Stage 0 — DONE.** Audit complete; instruments adopted. Open on-device:
the CC-5 FXC harness run; native boots at stage gates.

**Stage 1 — PRE-CAMPAIGN REPAIR & SWEEP.**
1. The frustum dispatch fix — workgroups **derived** from
   `Dim::MAX_ACTIVE_PATCHES` (never re-authored as 5), own commit, first.
2. The twelve dead-entry removals, each commit citing its probe tag
   (A1/A4/A5/A6/A7/A8); layout + group edited as pairs; registry constants
   remain (numbers are addresses; most still serve other layouts).
3. Fossil/stale text: the CE 145/146 rationale (the true fossil), banner
   108→102, ribbon layout comments.
4. Jean-stamp flags, executed here if stamped: delete `backup_board/`
   (kills all 7 BOMs); `trajectories` writer-side retirement check.
Gates: glaw1 + Dawn witness green per commit; one native boot.

**Stage 2** — card plumbing, instrument first (v1; writer probe-validated).
**Stage 3** — render-side rewire (v1; rest-identity is the net).
**Stage 4** — compute-side rewire per §6; gates: glaw1 + witness +
walkabout + cc6 recount showing §7's column + the compile-time-collapse
measurement. **Stage 5 — THE UNIFIED GROUND** (ratified): cap tiles +
cell-granular curtains absorbing the patch skirt; card.a nearest-sampled on
the snapped window; zone-mesh stack retirement (GoL → 2s); suppression
triple → pair; POLICY_TERRAIN_RENDER gains CONTRIB_GOL_ZONES in both
rooms. **Stage 6** — disposition pointers to the cleanup campaign: LATENT
query variants, `trajectories`, `backup_board/`, `LATENT[gate-a-shared]`
(closed by this campaign; the note retires with a pointer here).

## §10 Deferred, and the coupling seat

Named destinations unchanged (Layer C; Layer E — confirmed aimed-at; the
paint stratum, residency informed by the pending FXC half of CC-5; the 3D
atmosphere card). Ruling register: UNIFIED GROUND **decided**; mode-field
couplings (drift / morph-with-hint / warp-breath) awaiting; the Profile
Law's three rulings awaiting; Layer C/E timing awaiting; card
resolution/extent awaiting. §10 grows the coupling surface when the thread
resumes; nothing above moves when it lands.

## §11 Risk register (delta)

Drift closed at bd405d9; branch naming resolved by adoption (**CLOSURE_GPU
is the trunk**; alias deletion is one command, Jean's call). FXC: one probe
open (on-device); everything else pattern-proven and now *measurable*.
Hidden consumers: closed by grep + validator, permanently gated by the Dawn
witness. Pixel regression: rest-identity + probe-tagged sweep + staged
boots. Process: no handoff returns without glaw1 + witness green.

---
*Evidence: CLOSURE_GPU_AUDIT @ 884d56d (`AUDIT_REPORT.md`, `audit/`).
Seed docs for the implementation session: this file + the audit branch +
`session_bridge_terrain_era.md` + `7t_program_theory_v3.md`.*