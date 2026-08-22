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
  struct: `OrbConfig`'s driverless gen-1 block, `Instruments.watcher_ticks` (driver
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
- FIREFOX STAGING RATCHET (bounded; Firefox-side; the cheap cure landed, its
  witness owed): Firefox on Windows compiles the module with no diagnostics,
  boots, runs 30–56 fps on the Kepler, and loses the device with
  `[Device] LOST reason=1 : Out of memory` after several world transitions —
  rapid keyed transitions reach it in under a minute, a calm session in many
  minutes or never. about:memory, GPU process, across ONE keyed transition:
  gpu-committed 1003 → 1308 MiB; private write-combined commit 858 → 1089 MiB
  in 37 → 43 segments; a second transition +15 MiB, 46 segments; flat for
  minutes on either side. The program's own GPU budget is 251 MiB throughout
  and creates nothing after boot. Reading: Firefox's wgpu serves
  WriteBuffer/WriteTexture from upload-heap blocks it sub-allocates; a
  transition's upload burst — the paintings re-staged at teardown, 1 MiB
  WriteTextures landing several to a frame — opens blocks the per-frame trickle
  then keeps from emptying. Chrome's Dawn and WebKit recycle differently: an
  iPad (Safari 26) ran 10+ min with transitions. ORGAN_8 P3 paced the paintings
  to one lane (AUTHORED_FETCH_INFLIGHT_CAP = 1); its witness is a before/after
  about:memory pair across one keyed transition, Jean's. If the step survives:
  the structural cure is that the program owns its staging — a fixed ring of
  MAP_WRITE buffers made at boot, mapped, filled, unmapped and copied from, so
  the browser allocates no staging at all — priced at one round over
  GPUState's upload doors, not built. Origin: COMPAT_1's witness runs,
  2026-08-21.
- In-place same-shape atmosphere transition (sunset → night without a
  teardown, the sky re-drawn over a standing world): priced at ATMOS_1,
  not built — the spawns' population row is per mood, so a "same shape"
  transition is not yet a no-op below the sky. Origin: ATMOS_1 §7 — the
  campaign's handoff, held by Jean rather than filed in the tree; the
  campaign opens at 71619c0. Unblocked by a visual need (the recording's
  long takes) or the indoor atmospheres' rework.
- The regime's second subscriber owes the flag (REGIME_1): the four weight
  rows raise the MOOD definition flag only, so a weight edit re-rolls and
  re-speaks the sky and nothing else. The first family to carry a regime
  column (the orb mood bank is eligible today) must make the weight rows
  raise its definition flag too — a per-field mask or a "regime law"
  hook at the boundary — or a regime change leaves it behind. Origin:
  REGIME_1 §0.5.
- A per-parameter "mood-wide" flag for the regimes (ATMOS_2 §0.5): today a
  parameter wanted the same in every regime is set equal in every regime,
  four dials that agree. The flag would make it one dial and restore
  "one fact, one home" at the tuning level; it costs a selector row per
  parameter and a draw that reads regime 0 when the flag is set. Also
  priced, not built: a second independent roll (axes), should the
  combinatorics of independent light and fog be wanted back; and moving
  the sun's bearing into the regime row. LENS_1's ALL position writes
  every regime at a stroke, which is the gesture form of the same relief;
  the flag remains the one-home form. Unblocked by Jean's tuning asking
  for any of them. Origin: ATMOS_2 §0.5.
- naga is installed per session in CC's container with `cargo install
  naga-cli` (minutes), and `tools/wgsl_gate.py` then runs in-container on the
  raw module. Origin: ATMOS_1 report FLAG 12, answered at COMPAT_1.

## RIBBON_1 — the witnesses Jean owes the campaign

No gate can see any of these; they are the eye's, and the campaign is not
closed until they are looked at. Origin: RIBBON_1 (three commits on
`claude/ribbon-1`, base `fd53316`).

- THE FLOWN BODY FOLLOWS ITS TRACK. The spine became a SPACE law: one
  sample per chord of flight, so the body is drawn where the head has been.
  Tier 0 at full throttle and every parked ribbon are unchanged; tiers 1
  and 2 and every wanderer are visibly different from `fd53316` — where the
  body used to whip, laying rings at `cube_size` regardless of how far the
  head actually travelled, it now trails the path. This is the ruling to
  overturn if the old whip was the beauty.
- THE SKY RULE, both readers. The head should bank away from a shaft and
  climb a roofline before it reaches one (COLOSSAL antennas are the test:
  125 m of post with drums 20 m wide at the top). The body should bulge
  around what it meets and flow through the bulge — the bulge stays where
  the thing is — and lift over a pyramid rather than enter it.
- THE SEAT. `R` boards and lands on an ease, never a teleport. The saddle
  sits on ring 0's top face through roll and pitch; the nose faces where it
  swims and the tube does not shear at the neck.
- THE DIALS MOVE THE FLIGHT LIVE. Twelve `Ribbon · Head` / `Ribbon · Sky
  Rule` rows on the organ reach the kernels through `config.ribbon_*`;
  turning one mid-flight should change the flight without a respawn.
- FIREFOX. Three `WriteBuffer`s a frame per ribbon, where fifteen stood,
  and 6.4 KB of ring poses no longer uploaded twice per frame. The staging
  ratchet above reads against this.

### Priced at RIBBON_1, not built

- THE GESTURE RING. `[SEAM:ribbon-displacement]` (world.wgsl §6.5) now names
  its own shape: to let music drive the head's displacement, record the
  head's lateral/vertical into a GESTURE ring beside the spine —
  time-cadenced where the spine is chord-cadenced — and read the delayed
  samples in `ribbon_displacement_at`. Unblocked by the music-coupling
  campaign.
- THE SKY RULE'S COST. `sky_push` walks 344 emitters (32 shafts, 16 arches,
  32 walkers, 264 floaters) per reader, and `ribbon_body` runs one reader
  per ring — up to 400 — plus `ribbon_ground`'s analytic terrain per ring.
  No measurement exists; `the-board-web-meter`'s `ribbon_body` row is the
  first one to read. The cures if it bites, in order of cheapness: an
  EMIT_STRIDE-style stride on the body's rule reads, a broad-phase cull by
  the ribbon's own bounding box, or a shared per-frame shortlist the head
  builds once.

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
