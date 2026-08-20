# OPEN — the register of open state
One line per item: what · origin (sha or doc) · what unblocks it.
This file is the ONLY home of open/parked state. When an item closes, its line dies.

- PIPE_0: PARKED per Jean's 2026-08-07 directive. Origin sha of
  docs/HANDOFFS/WEB/PIPE_0_DECISION.md (path retired to git; content at its last sha).
- STREAM_0: PARKED per Jean's 2026-08-07 directive. Origin sha of
  docs/HANDOFFS/WEB/STREAM_0_DECISION.md (path retired to git; content at its last sha).
- DAWN_REFERENCE web-era rewrite: docs/reference/DAWN_REFERENCE.md is
  archival-with-named-drift (its own CANON stamp); the rewrite awaits Jean's
  reference round. Origin: the stamp itself.
- GUARD DEBT in realization/renderer.hpp: four __EMSCRIPTEN__ guards survive
  SUNSET_1 (census `git grep -n "__EMSCRIPTEN__" -- src`). Coverage is
  SPLIT, not blind: console_gate defines __EMSCRIPTEN__ (fidelity with emcc)
  and so type-checks the shipping arms; glaw1 does not, and so type-checks
  the dead ones. The sitting decides two things: collapse the guards
  (SUNSET_1's stated intent; may cost stub declarations), and whether
  glaw1's macro set should match the build's. Origin: RECENSION_1 FLAG-7,
  corrected by RECENSION_4 when the gates were first read whole. Unblocked
  by the gates sitting.
- L26 docket — fields marked dead, awaiting the sitting that next opens their
  struct: `RibbonState.is_roaming`, `RibbonRingTransform._pad0`,
  `OrbConfig`'s driverless gen-1 block, `Instruments.watcher_ticks` (driver
  went with the FileWatcher at SUNSET_1; `pawn.cpp:60` still names the dial),
  `OrbMoodConfig.base_hue` / `.hue_variance` (dead BY CONSTRUCTION — every
  ORB_PALETTES row carries count ≥ 1, so `pack_palette_` never leaves
  `palette_count` at 0 and the kernel's legacy single-hue arm is
  unreachable; ORGAN_4 P1b), and `OrbMoodConfig.motion_rule` (dead TREE-WIDE
  — `configure_orbs` writes the player's `os.current_motion_rule` and never
  reads the config field; ORGAN_4 C2). The three orb fields stay put rather
  than dying now because `ORB_MOOD_TABLE` is positionally brace-initialised
  (D3: the braces outrank the broom), and the twin rooms — table and struct
  — must move in one commit. Each dies in the commit that reopens its struct
  (L26, L3: twin rooms, one commit). Origin: L26, moved here by RECENSION_2;
  grown by ORGAN_4 P1b. Unblocked by any campaign that relayouts one of the
  three structs named.
- Dead boot write, awaiting the boot block's next sitting: state.hpp's
  `config_.aura_enabled = 1.0f` boot seed is overwritten within one frame by
  tick_pawn_couplings (the driven window's only runtime author since
  ORGAN_2a). Origin: ORGAN_2a D3. Dies in the commit that next opens the
  boot-config block.
- CENSUS_1b, the exhaustive walk of the four realization giants (state.hpp,
  world.wgsl, renderer.hpp, cartridge.hpp) and the ten bodies/** files, line
  by line. CENSUS_1 covered them by sweep and declared that edge; the refuter
  breached it immediately (gol_zones.hpp's "Upload all 7 slots" against a
  5-slot stride). Origin: PROCESS_LAWS SCHEDULING RECORD, moved by
  RECENSION_3 when its dated owner (the control-panel campaign) ran without
  collecting it. Unblocked by any campaign that enumerates those files.
- pawn.cpp IS UNGATED: no gate compiles the TU that owns main(), the rAF
  driver and the boot sequence — glaw1 compiles tu.cpp, console_gate compiles
  cartridge.hpp and console.hpp. One missing declaration is the whole reason:
  tools/gates/console_gate/stubs/emscripten.h stubs
  emscripten_set_main_loop_arg but not emscripten_set_main_loop, the form
  pawn.cpp calls. CC verified everything else in the TU compiles clean under
  the existing gate flags. Cure is one stub line; adding to the pinned stub
  set is a gates decision. Origin: RECENSION_2 FLAG-U5c. Unblocked by the
  gates sitting.
- FIREFOX REGAIN, HELD: naga 30 lacks immediate_address_space, so the module
  does not compile there; the audience card is the boundary. Priced: a
  generated no-immediates module (the wgsl_gate transform as generator) plus,
  on that path only, the per-patch staging machinery PROBATE_I retired — two
  boot paths, doubled witnesses. Origin: world.wgsl COMPILER FLOOR block,
  PROBATE_E3. Unblocked by naga shipping the extension — watched
  automatically by tools/wgsl_gate.py ARM 4, which prints when it happens —
  or by an exhibition that pays for the spend.
- In-place same-shape atmosphere transition (sunset → night without a
  teardown, the sky re-drawn over a standing world): priced at ATMOS_1,
  not built — the spawns' population row is per mood, so a "same shape"
  transition is not yet a no-op below the sky. Origin: ATMOS_1 §7 — the
  campaign's handoff, held by Jean rather than filed in the tree; the
  campaign opens at 71619c0. Unblocked by a visual need (the recording's
  long takes) or the indoor atmospheres' rework.
- wgsl_gate.py TypeError in the directive-removed branch: the two-line note at
  "DIRECTIVE not in src" splits a %s from its operand, so the branch raises
  instead of printing. Unreachable while world.wgsl carries requires
  immediate_address_space; fires the day it does not — i.e. exactly when the
  gate should announce its own retirement. Reproduced 2026-08-18. Origin:
  RECENSION rider FLAG-R1. Cure is one line; unblocked by the gates sitting.

## Harvested at WINNOW-2 W2 (from files that died this campaign)

- HARVEST: PREGEN-8 CONTINGENCY — rig-triggered storage weld (225→289 layers,
  TILE_GRID 17→19, MAX_ACTIVE_PATCHES) sleeps until the rig shows rim-pops under fast
  flight. (from audit/past reports/LADDER.md P-3, eafd9ec)
- HARVEST: TERRAIN-2 b3, the finite collapse — the radius-bounding choice (raise
  pregen / lower finite_radius_max / keep origin-pin degenerate) is undecided until b3
  lands; finite_radius_max is still live in gallery.hpp.
  (from audit/past reports/LADDER.md P-6, eafd9ec)
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
