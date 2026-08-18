# OPEN — the register of open state
One line per item: what · origin (sha or doc) · what unblocks it.
This file is the ONLY home of open/parked state. When an item closes, its line dies.

- CHORD_5 terrain-fragment suspect: possible per-pixel storage read in the terrain
  fragment chain; fix designed (move entities[0].pos into frame_r as frame-cadence
  uniform); unblocked by a DPR staircase session. Origin: CHORD campaign; law home
  docs/CHORD.md.
- 8-fit note: PARKED, awaiting Jean's design. The room-family compute pipelines carry
  9 storage-buffer layout entries visible to the compute stage, one over the WebGPU
  core default maxStorageBuffersPerShaderStage = 8. Origin: note.md +
  audit/past reports/CUT_1_LIMITS_FIT_NOTE.md (paths die this campaign; content in
  their last shas). NO HELD BRANCH EXISTS: `git ls-remote --heads origin` matched no
  branch for CUT_1 / 8-fit / C6 / LIMITS, so despite the note's "held branch" wording
  there is no branch to keep and none was deleted.
- PIPE_0: PARKED per Jean's 2026-08-07 directive. Origin sha of
  docs/HANDOFFS/WEB/PIPE_0_DECISION.md (dies this campaign).
- STREAM_0: PARKED per Jean's 2026-08-07 directive. Origin sha of
  docs/HANDOFFS/WEB/STREAM_0_DECISION.md (dies this campaign).
- Panel (JSX/Vite): resumes as sibling repo with its own git; sources and package.json recoverable at 850f896^ (CUT_1g retired them); node_modules on disk stays untouched until founding.

## Harvested at WINNOW-2 W2 (from files that died this campaign)

- HARVEST: OPTION-B WGSL PIN — world.wgsl @binding literals stay hand-mirrored
  (Option A, tools/wgsl_gate.py as the net); Option B, ONE authored source generating
  both sides, is still unbuilt. Verified still Option A at c43d44d.
  (from audit/past reports/LADDER.md P-4, eafd9ec)
- HARVEST: PREGEN-8 CONTINGENCY — rig-triggered storage weld (225→289 layers,
  TILE_GRID 17→19, MAX_ACTIVE_PATCHES) sleeps until the rig shows rim-pops under fast
  flight. (from audit/past reports/LADDER.md P-3, eafd9ec)
- HARVEST: TERRAIN-2 b3, the finite collapse — the radius-bounding choice (raise
  pregen / lower finite_radius_max / keep origin-pin degenerate) is undecided until b3
  lands; finite_radius_max is still live in gallery.hpp.
  (from audit/past reports/LADDER.md P-6, eafd9ec)
- HARVEST: the twins ruling — KEEP-THROUGH-ORGAN (the report's recommendation: the web
  is the product, the native twin is the bench, and the bench retires when the web gains
  WGSL hot reload AND the port is vendored) vs RETIRE-NOW. Awaiting Jean's word.
  (from audit/DOMESDAY_2_REPORT.md §11 Slot 1, 5ff5696)
- HARVEST: the terrain word on the pinned seed — three rounds outstanding, and the last
  open item of PROBATE_I. Wrong looks like "the whole terrain is the same hill repeated"
  or "everything outside the first patch went flat". (from GATEHOUSE_REPORT.md, a5e84bf)
- HARVEST: GF6 — whether to pay for renaming tools/gates/console_gate/ to match what it
  now is; ruled HOLD, the directory rename stays unpaid.
  (from docs/audit/PROBATE_CLOSE.md + GATEHOUSE_REPORT.md, 606924f)
- HARVEST: SPAWN_3 parked — F6 newborns seat inside render radius on move.
  (from audit/past reports/FIELD_BRIDGE.md, 4c1a804)
- HARVEST: the mood-seam crack — neighbours disagree at a shared edge; remediation
  directions recorded but NOT executed, parked per ruling.
  (from audit/past reports/INVESTIGATION_mood_seam.md, eafd9ec)
- HARVEST: TEX_C0 — parked behind the feature wallet and Jean's eye on ASTC banding
  since PROBATE_F. As originally sketched it is **impossible**, not merely expensive;
  priced and shelved with its bill attached, not refused and not scheduled. The texture
  constraint belongs beside docs/FXC_LAWS_RECORD.md.
  (from docs/audit/TEX_C0_PRICE.md, de1d5db)

## Added at CANON C2

- VOICE bus (terrain live-modulation): alias-table design — VOICE_LAYOUT[] shaped
  like PARAM_LAYOUT[], one set_voice(channel, span) door, [VOICE] boot witness
  enumerating every channel with rest + address; sketch in terrain_program_charter
  §C5 (attic since CANON, last at a8f4580); unblocked by the music-coupling campaign.
- Immediates witness: push-constant/immediate budgets have no census and no witness —
  do not spend what nothing measures; the first campaign wanting them builds the
  witness first. Origin: ESTATE_LOOM_WALLET (attic, 3b931ba).
- SALON Stage B (slot count): STILL OPEN, land-gated. PAINTING_MAX_SLOTS is still 288
  (state.hpp:271) and was last touched by LOOM_1 U3, not by a SALON re-spec; the
  re-spec §0.1/§0.2/§0.3 is unpaid. The witness the stage relies on DOES exist —
  tPipe (renderer.hpp:170). Origin: SALON_1 (attic, 3b931ba).
- SALON Stage D (weld): STILL OPEN, and sharper than the ledger recorded. The named
  slot-filter defect IS fixed — clear_wall_paintings now filters on the sentinel
  patch pair as well as form type (gallery.hpp:2481-2484). What remains: that
  function resets `gs.wall_frame_count = 0` unconditionally (gallery.hpp:2493),
  OUTSIDE the patch-filtered loop, while evict_paintings_for_patch decrements the
  same uint32_t unguarded (gallery.hpp:1423). An outdoor WALL_FRAME slot that
  survives a clear leaves the counter at 0, and the next evict on that patch
  underflows it; the value is consumed as a draw count (render_passes.hpp:490,614).
  Origin: SALON_1 (attic, 3b931ba).
- SALON Stage E (salon hang): STILL OPEN — visual gate, Jean's eye. Prerequisite
  findings §0.5 (vault-crown coupling) and §0.9 (live floor breach) stand.
  Origin: SALON_1 (attic, 3b931ba).
- SALON HELD (packing): STILL OPEN — never built: floor_margin and target_coverage
  both have 0 hits in src/. Note the ledger's cost figure assumed EXHIBITION_LAYERS
  32; the tree now reads 40 (state.hpp:294), so "32 -> 256 = +896 MiB" wants
  recomputing before it is spent. Origin: SALON_1 (attic, 3b931ba).
- ML-1 / mirror_census: the span model does not recognise strataLayoutFor()
  returning wgpu::PipelineLayout (27 sites, 57 STOPs; pre-existing at ba0e26d).
  MIRROR_LEDGER.md is frozen at its last successful regen. Unblocked by: teach
  the span model the strata accessor, or retire the pair — Jean's pick at the
  next instruments sitting.
