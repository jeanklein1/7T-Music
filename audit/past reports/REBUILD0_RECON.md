> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# REBUILD-0 — PHASE R: THE RECON REPORT

Read-only campaign product. Nothing moved, nothing gated, nothing repaired.
Eight censuses ran in parallel over the full cartridge (every module pair read
in full; mechanical grep sweeps cross-checked the manual censuses; zero
discrepancies). Every claim below carries a file:line citation. The report
ends in a stamp request; the movements cut only after Jean stamps.

METHOD NOTE. Censuses: R1 score map (cartridge.hpp, all 1,213 lines), R2A
deps (9 bodies/ pairs), R2B deps (surface/ + machine/ + direction/ +
render_passes + in-class graduation census), R3 channels (all 18 impl files,
10,254 lines), R4 witness, R5 sky reach, R6 packer forecast (state.hpp S4 +
world.wgsl mirrors), R7 anomaly (world.wgsl call-tree arithmetic). Exhaustiveness
of the deps censuses was verified by a mechanical sweep
(`grep -ohE '(c|self)->[A-Za-z_]+'` over all impl files) matching the manual
census exactly.

---

## 0 — FINDING ZERO: THE LENS EDITION

**Theory v3 is not in the tree.** src/docs/ holds v2 only (last docs commit
2f1ff00). The charter cites v3 §11 (the witness), F6 (the addressed-intent
socket), and A5 (the packers) — none of which exist in any committed document.
The campaign's stated purpose — every structural fact traceable to a section
of the lens — requires the lens to be committed. **Request: land v3 beside v2
before or with the stamp.** Until it lands, this report cites v2 where v2
covers the ground (§8 arrow law, §9 driver law + four strata, §10 relational
anatomy) and cites the charter's v3 section numbers on faith where it does not.

---

## 1 — R1: THE SCORE MAP

### 1.1 Today's score, decomposed

**BOOT** — initialize() (cartridge.hpp:398-419) stages GPU neutral defaults
(ROOT). init_renderer() (421-544) in order: renderer/offscreen init +
terrain-index one-shot pass (445-458, REALIZATION); init_patch_system (461) +
setup_test_rig_piers (S2); configure_orbs gated ROSTER.orbs (465);
upload_agent_registries (475); **STRAY** inline slot-0 pawn seeding (481-489);
spawn_population_for_mood gated ROSTER.wanderers (494); load_authored_textures
**UNGATED** (500-503) — **this is P2**; compile-time ROSTER report (517-541).

> RETIRED (BOOT_ONE_VOICE C, 4cc629d): setup_test_rig_piers deleted; pier
> slots 0-3 unassigned. The BOOT score above stands as written for every
> other verb; the S2 movement is now init_patch_system alone.

**UPDATE** (560-751) — frame-signal fill (563-582); SNAP-1 neutral sky words
(584-603); clock/tempo (605-613); visual_canvas tick (615, S4-driver); fog
staging (616-622); tick_pawn_couplings gated ROSTER.pawn_aura (626, S4-body);
world seed/bounds with **STRAY** finite-bounds math (628-636); the spine-owned
transition machine (638-736, SEAM[spine:transitions]) whose TEARDOWN case
(648-726) carries **STRAY** agent-mirror reseed (683-695) and **STRAY** ribbon
deactivation (705-714); set_fade/upload_signal/upload_config (737-741);
update_orb_anchor gated ROSTER.orbs (745, WITNESS); update_photographer
**UNGATED** (749, WITNESS) — **this is P1**; clear_input_deltas (750).

**RENDER** (759-1128) — two P5 readback map machines (766-841; the floater
machine carries **STRAY** sphere/cube mirror loops 813-834); portal-trigger
door gated ROSTER.transitions (843-861); respawn_evicted_agents gated
ROSTER.wanderers (866, S3); tick_cube_corral_animations **UNGATED** (869,
S4-body); stream_patches (871, S2); GoL residue + census diagnostics
(873-917); ribbon_frame_tick **UNGATED** (921, S4-body); 12-family mesh-prep
**macro fold** ROSTER_PREP_FAMILY + single Entity Mesh Gen pass (923-963);
upload_portal_array/upload_lights (964-965); resync_sky_head (967-982,
SNAP-1); dispatch_compute (984); staging copies (986-1001, P5); GoL zone
block, runtime-gated on zone_count with **STRAY** inline pass encodings
(1003-1038); pawn-aura block gated ROSTER.pawn_aura with **STRAY** config
assembly (1040-1090); orb chain gated ROSTER.orbs (1092-1101);
ground_entries→placement cascade (1103-1111); cull→shadow→main→snapshot
(1113-1118); **STRAY** gallery promotion drain (1120-1127).

**ON_INPUT** (1146-1164) — pure dispatch to direction/input; zero stray logic.

### 1.2 THE PROPOSED NEW SCORE (prose skeleton)

Per THE SCORE RULING: explicit prose in stratum order (v2 §9), movement
banners, one conductor call per piece per movement, presence constexpr-gated,
no typelist folds. Names marked † are new owner verbs created by pulling
today's strays into their owners (§1.4).

**BOOT** (init_renderer):

```
// ═══ MOVEMENT: BOOT — REALIZATION (the stage exists first) ═══
renderer init · offscreen resources · terrain-index one-shot pass
// ═══ MOVEMENT: BOOT — S2 THE SURFACE ═══
init_patch_system · setup_test_rig_piers   // RETIRED (BOOT_ONE_VOICE C, 4cc629d): setup_test_rig_piers deleted; pier slots 0-3 unassigned
// ═══ MOVEMENT: BOOT — S3 PLACEMENT ═══
seed_player_body†(agent_state_, …)                      // stray (3) dies; ungated — the pawn is unconditional
if constexpr (ROSTER.wanderers)  spawn_population_for_mood
// ═══ MOVEMENT: BOOT — PER-PIECE BOOT VERBS ═══
if constexpr (ROSTER.orbs)      configure_orbs
upload_agent_registries_to_gpu                          // ungated — pawn rides the agent buffer
if constexpr (ROSTER.gallery)   load_authored_textures   // ← P2 DIES HERE, STRUCTURALLY
// ROSTER report
```

**UPDATE**:

```
// ═══ MOVEMENT: THE CLOCK AND THE SIGNAL (root) ═══
frame-signal fill (O-5a) · sky words NEUTRAL (SNAP-1/O-1) · clock/tempo
// ═══ MOVEMENT: S4 MOTION — DRIVERS ═══
visual_canvas_.tick(signal) · fog staging
// ═══ MOVEMENT: S4 MOTION — BODIES ═══
if constexpr (ROSTER.pawn_aura) tick_pawn_couplings
// ═══ MOVEMENT: THE TRANSITION MACHINE (spine; TEARDOWN is its own movement, below) ═══
// ═══ MOVEMENT: REALIZATION STAGING ═══
set_fade · upload_signal · upload_config (O-5b/c)
// ═══ MOVEMENT: WITNESS ═══
if constexpr (ROSTER.orbs)      update_orb_anchor        (O-5d: last-frame readback, declared lag)
if constexpr (ROSTER.gallery)   update_photographer      // ← P1 DIES HERE, STRUCTURALLY
// ═══ MOVEMENT: DRIVER BOOKKEEPING ═══
clear_input_deltas (O-5e: dead-last)
```

**TEARDOWN** (the machine's middle phase becomes a movement with per-owner
verbs — the owner-verb pattern already coexists inside today's teardown_world:
clear_spheres patch_system.inl:323, clear_cubes :324, teardown_orbs :361):

```
// ═══ MOVEMENT: TEARDOWN (fixed sequence O-3) ═══
world_gen++                                              // T-01, the P5 fence, FIRST
return-state capture BEFORE world_state_ overwrite       // T-02/T-03
teardown_surface (patch/tile/theme/footprint core of today's teardown_world)
per-owner teardown verbs, one per piece, constexpr-gated:
  teardown_entities† · teardown_gol† · teardown_ribbon† · teardown_gallery†
  · clear_spheres · clear_cubes · teardown_orbs · teardown_pawn_aura†
GPU player reset · reseed_player_body† (agents; stray (3)'s twin dies)
set_world_seed BEFORE apply_mood                          // T-08/T-09
if constexpr (ROSTER.wanderers) spawn_population_for_mood
release_finite_ribbons†                                   // stray (4) dies; AFTER apply_mood (O-3 step 6)
back_portal_pending scheduled LAST                        // T-13
```

**RENDER**:

```
// ═══ MOVEMENT: WITNESS — HARVEST (P5 maps; consumes LAST frame's capture; leads the score — O-2) ═══
pawn map machine (spine)
floater map machine (spine), mirrors reconciled by owner verbs:
  if constexpr (ROSTER.sphere)  reconcile_sphere_mirror†   // stray (1) dies
  if constexpr (ROSTER.cube)    reconcile_cube_mirror†
if constexpr (ROSTER.transitions) portal-trigger door (entry door #2)
// ═══ MOVEMENT: S2 SURFACE LIFECYCLE ═══
stream_patches            (carries the declared S3-trigger seam inside — SEAM[patch:spawn-trigger])
// ═══ MOVEMENT: S3 PLACEMENT ═══
if constexpr (ROSTER.wanderers) respawn_evicted_agents     [REORDER-CLAIM RC-1]
// ═══ MOVEMENT: S4 MOTION — BODIES ═══
if constexpr (ROSTER.cube)    tick_cube_corral_animations  // ← gate closes ungated site (869)  [RC-2]
if constexpr (ROSTER.ribbon)  ribbon_frame_tick            // ← gate closes ungated site (921); O-1 upstream
// ═══ MOVEMENT: REALIZATION ═══
twelve explicit prepare_mesh lines, each constexpr-gated   // macro fold DISSOLVED (no typelist folds)
single Entity Mesh Gen pass over dirty families
upload_portal_array · upload_lights
resync_sky_head            (O-1: AFTER ribbon tick, BEFORE dispatch)
dispatch_compute
// ═══ MOVEMENT: WITNESS — CAPTURE (O-2: staging copies AFTER compute; feeds next frame's harvest) ═══
// ═══ MOVEMENT: REALIZATION, CONTINUED ═══
if constexpr (ROSTER.gol)       gol zone passes via owner verbs†  // stray (6) dies; O-6a barriers  [D7]
if constexpr (ROSTER.pawn_aura) aura block, config via pawn verb† // stray (2) dies; O-4 flags
if constexpr (ROSTER.orbs)      orb chain (O-6b: copy_prev before dynamics)
ground_entries → placement cascade (O-4)
frustum cull → shadow → main → snapshot (O-7 tail)
if constexpr (ROSTER.gallery)   drain_gallery_promotions†  // stray (5) dies; AFTER snapshot
```

**THE WITNESS SPLIT (constraint-driven deviation, for the stamp).** The
charter's movement order places WITNESS between S4 and REALIZATION. In
update() it sits exactly there. In render() the P5 protocol (O-2) forces a
split: HARVEST must lead the score (it maps last frame's copy and every
downstream consumer — stream center, portal door, corral — eats its output),
and CAPTURE must sit after dispatch_compute (it copies this frame's results).
The proposed prose names both movements explicitly rather than pretending the
stratum order holds where the protocol forbids it.

**THE REORDER POLICY (for the stamp).** The proposed prose reorders exactly
two things relative to today: RC-1 (respawn moves after stream_patches — S3
after S2; safety: respawn touches slots 1+ only, slot 0 is never evicted
[world.wgsl:6248], and stream's bubble center reads player_.readback_x/z
refreshed at harvest [patch_system.inl:711-712] — no data edge between them)
and RC-2 (cube corral moves after stream — S4 after S2; the corral tick and
patch eviction both touch cube slots, so identity is plausible but not
proven). Rule proposed: **a reorder cuts only with a written safety argument
plus the pixel-stable gate; any reorder that fails the gate reverts, and the
movement banner records today's order as a cited constraint instead.** The
PRIME INVARIANT outranks banner aesthetics.

### 1.3 The ordering constraints (the laws the new score must cite in place)

- **O-1 / SNAP-1** — resync_sky_head is the authoritative sky author, AFTER
  ribbon_frame_tick, BEFORE dispatch_compute; update() ships neutral zeros so
  a lost resync fails LOUD; correctness rides queue-write submission order
  (cartridge.hpp:584-603, 919-921, 967-984).
- **O-2 / P5** — two 3-phase readback machines: IDLE →(copy after compute)→
  COPIED →(MapAsync next frame)→ MAPPING →(callback)→ IDLE; world_gen captured
  in the closure; Unmap unconditional; results one frame late by design
  (288-291, 766-841, 986-1001).
- **O-3 / TEARDOWN** — world_gen++ first; capture before overwrite;
  teardown_world; GPU reset + mirror reseed; set_world_seed before apply_mood;
  apply_mood before wanderer respawn AND before the finite-ribbon release
  (which reads MOOD_TABLE[mood_state_.active]); back_portal_pending last
  (648-726).
- **O-4 / DEFERRED FLAGS** — portals_dirty boots true; aura_cfg_dirty full-vs-
  frame upload; aura_needs_clear one-frame 999 protocol;
  ground_entries_dirty→placement_dirty cascade; per-family mesh dirty[]
  (209, 1046, 1088-1089, 1103-1111).
- **O-5 / UPDATE MICRO-ORDER** — dt_beats before prev_beats advance; set_fade
  after the machine; upload_signal/config after all staging setters; orb
  anchor tolerates one-frame lag; clear_input_deltas dead-last (582, 605-613,
  737-750).
- **O-6 / GPU BARRIERS** — GoL derive→sync→evolve→mesh as separate passes for
  barriers; orb copy_prev strictly before dynamics (1003, 1092-1101).
- **O-7 / RENDER MACRO-ORDER** — mesh prep (CPU uploads) before the single
  mesh-gen pass; cull before the draw passes; snapshot before promotions
  (759-1128).

### 1.4 The strays (nine, ranked) and the ungated three

Relocation candidates — each becomes an owner verb in the m2 cut (the score
cannot be one-call-per-piece while these sit inline):

1. Floater mirror reconciliation loops (813-834) → spheres/cube_behaviors
   verbs (SPAWN_PROTECTION_S moves home with them).
2. Pawn-aura GPUPawnAuraConfig assembly (1046-1073) → pawn verb.
3. Agent slot-0 seeding, boot twin + teardown twin (481-489, 683-695) →
   agents verb (one function, two callers).
4. TEARDOWN ribbon deactivation loop (705-714) → ribbon verb.
5. Gallery promotion drain (1120-1127) → gallery verb.
6. GoL inline pass encodings (1009-1037) → gol_zones verbs (the sibling orbs
   family already owns its dispatches module-side — the asymmetry is the tell).
7. Finite-bounds math (629-636) — PATCH_EXTENT vocabulary in the score; minor.
8. '[Player] pos' stdout (902-908) — DIAG-unwrapped; minor.
9. Boot terrain-index one-shot encoder block (445-458) — boot-only; minor.

Ungated call sites the new score gates: tick_cube_corral_animations (869, no
ROSTER.cube), ribbon_frame_tick (921, no ROSTER.ribbon), and the GoL compute
block (1004, runtime zone_count only — see D7). The ROSTER gate census
(465, 494, 517, 626, 699, 745, 847, 866, 887, 932-936, 1044, 1096) is
otherwise complete.

ROOT-declared legitimacy (NOT strays, per standing seam rulings): the
FAMILY_DISPATCH hub + six wrapper pairs (SEAM[spine:K2-related]), the P5
machines + world_gen (SEAM[spine:P5]), the transition machine + mood_state_ +
pendingDestination_ residency (SEAM[spine:transitions], K4), the portal
trigger hooks (SEAM[spine:portal-system]) (cartridge.hpp:12-43, 250-267,
280-291, 753-758).

---

## 2 — R2: THE DEPS TABLE

The m3 input: what each module's XDeps struct must carry when the keyhole
dissolves. "Root R/W" = spine-resident organs; "services" = file-scope
functions consumed (homes verified: spawn_engine.hpp:92-222,
entity_pipeline.{hpp:55-123,inl:45-148}, patch_system.{hpp:67-187,inl:25-384},
tile_world.inl:286-295, population_themes.hpp:56, seed_utils.hpp:26-107).

| module | root reads | root writes | sibling reads | sibling writes | services | GPU face |
|---|---|---|---|---|---|---|
| **population_themes** | — | — | — | — | own hpp + cpu_hash_f; DIAG census call | **zero** |
| **family_dispatch** | — | — | — (addresses only) | — | — | zero |
| **tile_world** | world seed/radius; mood_state_.active | — | — | — | seed_utils, themes vocab, MOOD_TABLE, PATCH_EXTENT | 1 method |
| **pawn** | player_.aura_presence, time.dt | player_.aura_presence (**sanctioned P8**) | — | — | — | 2 methods |
| **orbs** | world seed; readback_x/z; time | — | — | — | — | 14 methods + 5 renderer (**cleanest body**) |
| **spheres** | time.seconds | — | — | — | preamble, generic S3, find_patch, THEMES, hashes | 1 method (+GPUState& bypass: clear_spheres) |
| **gol_zones** | mood.active, world seed, time | — | tile_world tileCache_ | — | preamble-family, footprints, find_patch, seed_utils | 5 methods + 1 renderer + **device_ self-submit** (gol_zones.inl:302-312) |
| **agents** | possessed_slot, transitionPhase_, world seed, time | player_.possessed_slot (:276) | COLUMN_PALETTE (vocab) | — | seed_utils | 7 methods |
| **cube_behaviors** | possessed_slot, time, mood.active | gpuState_.config() raw field (:84) | **agent_state_.slots** (:104-105, :201-202) | — | preamble, generic S3, find_patch, THEMES, hashes | 6 methods (+GPUState& bypass: clear_cubes) |
| **entities** | ROSTER.portal | world.ground_entries_dirty ×4; mood.portals_dirty (:251,315,328,341,273) | ARCH_TIERS (vocab) | — | pier writers, find_patch/record_entity, preamble, generic S3, seed_utils, THEMES | 13 methods |
| **ribbon** | time, visual_canvas + 4 TargetBindings, sky trio, readback, inputState_.move | player_.sky_mode_prev, sky_yaw_eased (:399,418,421) | tile_world samplers | — | preamble, negotiate, THEMES, seed_utils, find_patch/record_entity | 5 methods (+GPUState& bypasses: ribbon_advance_head, ribbon_rebuild_body_upload) |
| **gallery** | readback, sunDirection_, clearColor_, world seed/counts, mood.active | — | tile_world tileCache_; **ribbon_state_.rendered_slot** (:711) | — | footprints, seed_utils, find_patch/record_entity, stb/filesystem | **widest body face**: ~20 methods + 8 renderer |
| **spawn_engine** | world finite/seed, mood.active, time, readback | — | entities (r) | entities draw_visible + mesh_gen_pending (**SEAM: culling service**) | seed_utils, mood/patch vocab | 2 methods |
| **entity_pipeline** | mood.active | mood.portals_dirty (:972); world.ground_entries_dirty (:164) | entities (r) | entities welded-four blocks (**SEAM: dispatch recipes**) | preamble, negotiate, pier writers, regen, seed_utils, themes/mood/entities vocab | 4 methods |
| **input** | keys_/mouse_/inputState_ (owner), player_ fpv/sky, world radius, device_ | player_ toggles; world radius + recenter poke (:235-262) | pawn_state_ aura toggles (:129-135) | — | command fans → mood/orbs/agents/cubes; patch vocab | 2 methods |
| **mood** | world seed/finite (r-only) | **dominant spine writer**: mood_state_, transitionPhase_, pendingDestination_, backPortalPosition_, cpuSpotLights_, cpuPortalArray_, sun/clear colors (K4-declared) | entities arches, ribbon mood_offset/gpu[0] | gol.mood_allowed, pawn.aura_enabled, entities.lights_dirty, ribbon.rendered_slot (:507,508,374,492) | force_spawn_portal_arch (THE CHANNEL), commit_ribbon, gallery walls, configure_orbs, render_passes VP math, seed_utils | 12 methods + 1 renderer |
| **render_passes** | world counts, mood fade/spot, clearColor_ | **ZERO CPU writes** | entities/piers/gol/ribbon/gallery/orbs (r-only) | — | render_orbs; own VP math | **largest face**: 46 members + 33 renderer |
| **patch_system** | **fattest root client**: world_state_ 94 reaches, readback ×10, inputState_ budget, mood back_portal flag | world_state_ (declared co-owner, R-a); mood.portals_dirty | tile/themes/ribbon/entities (r) | **teardown hub: 10 sibling organs** (§3.1); tile cache fills; theme idx; ribbon tips | S3 dispatch seam, tile_world gens, themes envelope, mood back-portal, seed_utils | 30 methods + 3 renderer (+raw config() pokes) |

**Exemplar deps-struct shapes** (m3 derives the rest from the table):

```
PawnDeps       { PlayerState& player; const TimeState& time; GPU }           // P8 write stays declared
OrbsDeps       { const WorldState& world; const PlayerState& player; const TimeState& time; GPU; Renderer }
GolZonesDeps   { const MoodState& mood; const WorldState& world; const TimeState& time;
                 const TileWorldState& tiles; GPU; Renderer }                 // device_ reach dissolves (§2.3)
PatchSystemDeps{ WorldState& world (co-owner); const PlayerState& player; MoodState& mood(flag);
                 TileWorldState& tiles; ThemesState& themes; …teardown verbs replace organ writes }
```

### 2.1 THE GRADUATION LIST (in-class types that must reach file scope)

| type | today | reached by | verdict |
|---|---|---|---|
| **struct TimeState** | cartridge.hpp:163-172 | 8 modules (spawn_engine, pawn, orbs, ribbon, spheres, gol, cubes, agents) | **GRADUATES** |
| **struct MoodState** | cartridge.hpp:193-219 | 9 modules; semantically mood-owned, spine-resident (K4) | **GRADUATES** |
| **struct PlayerState** | cartridge.hpp:228-245 | 9 modules | **GRADUATES** |
| **enum TransitionPhase** | cartridge.hpp:269 | mood.inl:1001,1008 + agents.inl:227 pay the Cartridge:: qualification tax | **GRADUATES** |
| readback enums | cartridge.hpp:288-291 | **zero modules** — spine-only P5 machinery | **STAYS** (evidence-based) |
| six dispatch wrapper pairs | cartridge.hpp:328-372 | family_dispatch.inl rows only; SEAM[spine:K2-related] | **STAYS** (stamped seam) |

Proposed homes (D3): TimeState + PlayerState + TransitionPhase →
**contracts/spine_state.hpp** (root-owned organs, many readers — contract
room); MoodState → **direction/mood.hpp** per the WorldState precedent
("struct lives with patch_system; instance stays at root", R-a stamp,
cartridge.hpp:146-150). Alternative: all four to contracts/spine_state.hpp.
Jean picks.

Contrast set already graduated (the pattern to follow): WorldState @
patch_system.hpp:37, InputState/KeyState/MouseState @ input.hpp:27-51,
PortalDestination @ mood_constants.hpp:28, GPUPortalArray/GPUSpotLightArray @
state.hpp, all 14 module states.

### 2.2 Keyhole bypasses + the device reach (m3 cleanup set)

Four functions take GPUState& instead of the keyhole: clear_spheres
(spheres.hpp:34), clear_cubes (cube_behaviors.inl:68), ribbon_advance_head
(ribbon.inl:147), ribbon_rebuild_body_upload (ribbon.inl:117). One module
reaches device_ directly and self-submits a command buffer:
flush_zone_derive_requests (gol_zones.inl:302-312). Two raw
gpuState_.config() field-write sites bypass named setters:
cube_behaviors.inl:84 (floater_coordination) and patch_system.inl:397, 438,
996, 999-1000 (pier_count / placement_patch_count / lod_pawn — poke-then-flush
idiom). All become deps-struct-conformant at m3 (setter additions where the
house style demands: set_floater_coordination).

---

## 3 — R3: THE CHANNEL CENSUS

### 3.1 True module→module writes (seven clusters) + proposed shapes

1. **input → pawn_state_** aura toggles (input.inl:129-135). Shape today:
   direct write + dirty flag. Proposed: pawn-owned command verbs
   (toggle_aura / toggle_aura_height) matching the orbs/agents/cube command
   pattern on the very next lines of the same switch.
2. **mood → gol_state_.mood_allowed** (mood.inl:507). Already channel-shaped
   (policy flag, consumer gates at its own tick, gol_zones.inl:321). Keep.
3. **mood → pawn_state_.aura_enabled** force-off (mood.inl:508). Proposed:
   a pawn-side mood_allowed flag consumed by tick_pawn_couplings — same gate
   pattern as (2), removes the reach into the player-preference bit.
4. **mood ↔ entities_state_.lights_dirty** (mood.inl:374, 969-970) — mood is
   BOTH producer and consumer; the flag merely lives in the wrong organ
   (entities.hpp:478, never touched by entities.inl). Proposed: **re-home to
   mood_state_** — the peer-write disappears entirely (patch_system.inl:367
   retargets with it).
5. **mood → ribbon_state_.rendered_slot** = 0 after uploading gpu[0]
   (mood.inl:488-492). Proposed: fold into the owner's commit path
   (commit_ribbon or a ribbon-owned promote_to_rendered verb).
6. **patch_system → tile_world / themes / ribbon organs** (tile cache fills
   patch_system.inl:439,750,885,891; token reset 190-192; theme idx 620 +
   reset 198; ribbon two-tip registration 632-648). Proposed: owner verbs —
   ensure_tile / reset_terrain_memory (tile_world);
   evaluate_theme_envelope stores its own result + reset verb (themes);
   ribbon_register_tip_if_here (ribbon — its inverse, the evict ref_count
   decrement, already lives owner-side at ribbon.inl:904-905).
7. **patch_system teardown_world bulk sweep over SEVEN foreign organs**
   (patch_system.inl:183-371: entities, gol, ribbon, gallery, spawn_engine
   queues/footprints, pawn flags, mood flag). Proposed: per-owner teardown
   verbs — the pattern already coexists in the same function (clear_spheres
   :323, clear_cubes :324, teardown_orbs :361, rotate_authored_staging :345).
   **Census re-rank: this cluster rides m2**, because the TEARDOWN movement's
   one-call-per-piece prose needs the verbs anyway (§8).

### 3.2 Module→root writes (channel or relocation)

- ribbon → player_.sky_mode_prev / sky_yaw_eased (ribbon.inl:399, 418, 421):
  both exist only for ribbon's law — **relocation into RibbonState removes the
  writes without any channel** (feeds R5/P3).
- pawn → player_.aura_presence: sanctioned SEAM[spine:P8]; writer is the
  semantic owner. Stays declared.
- agents → player_.possessed_slot (:276): possession transfer is inherently a
  player-state change; the synchronous command is coherent. Keep, declare.
- input → player_/world_state_ (input.inl:235-262): the toggles are
  conventional spine-state commands; the INT32_MAX force-regen poke (261-262)
  is a hidden regen request into the streaming conductor — proposed: a
  patch_system-owned **request_recenter** verb names it.
- entities/entity_pipeline → world.ground_entries_dirty; entities/pipeline →
  mood.portals_dirty: already flag channels with spine/owner-tick consumers.
  Keep, inventory below.
- cube_behaviors.inl:84 + patch_system config() pokes: named setters (§2.2).

### 3.3 The flag-channel inventory (eight, already intent-shaped)

writer → flag → consumer: (1) mood_state_.portals_dirty [mood:714,
entities:273, entity_pipeline:972, patch_system:210 → mood upload_portal_array
:942]; (2) world.ground_entries_dirty [entities ×4, pipeline:164,
patch_system ×4 → render() :1103]; (3) entities.lights_dirty [mood:374,
patch_system:367 → mood:969 — mis-homed, see §3.1.4]; (4)
mood.back_portal_pending [spine:716 → cleared mood:721, timed by
patch_system:756]; (5) gol.mood_allowed [mood:507 → gol:321]; (6)
pawn.aura_cfg_dirty [input, pawn, patch_system → render() :1046]; (7)
*_mesh_gen_pending [owners + machine + patch_system → prepare_*_mesh_gen];
(8) world.pier_count_dirty [pier writers → flush_pier_count :401].

### 3.4 The direction module + the F6 socket

direction/ already hosts the two driver casts (input, mood — v2 §9's INPUT
and ALGORITHM/policy authors). The channel stage adds **no new residents**:
every conversion in §3.1/§3.2 lands owner-side (verbs on bodies) or stays a
root flag. The command-door families (input→orbs/agents/cubes/pawn,
request_mood_transition) and the eight flag channels constitute the founding
CHANNEL REGISTRY. **The F6 addressed-intent socket is RESERVED, not built**,
per the charter: when a driver must address a body it does not own by
synchronous command, the socket is where that intent routes. Nothing today
requires it; naming it prevents inventing it twice.

---

## 4 — R4: THE WITNESS MAP

**The CPU witness record is PlayerState** (cartridge.hpp:228-246):
possessed_slot (possession) · fpv_mode (camera) · sky trio (sky-flight — R5's
subject) · readback_x/z + readback_portal_trigger (P5 harvest; **no readback_y
exists anywhere** — witness altitude is GPU-only) · aura_presence (P8).

**Writers**: the P5 pawn-map callback is the sole live author of readback_*
(786-789, world_gen-guarded); TEARDOWN resets them (670-695, but NOT
sky/fpv/aura_presence); input toggles fpv/sky (235, 247); ribbon writes the
sky pair (399, 418-421 — the one module writing witness state); pawn ramps
aura_presence (P8); agents moves possessed_slot (:276).

**The camera has NO CPU mirror.** GPUCameraState is written once at boot
(state.hpp:5458-5472) and integrated purely GPU-side by update_camera
(world.wgsl:6285-6359: damped aim_point τ=0.30, FPV eye, POLICY_FLYER terrain
clamp, indoor clamps), ordered player → others → camera → VP
(render_passes.inl:128-182). The camera feeds BACK into the pawn (heading
basis, wgsl:5613-5636) — the witness is a mutually-coupled pawn+camera pair,
GPU-resident. The camera is never reset on transitions.

**The anchor is GPU-only.** compute_pawn_aura re-derives cell ownership from
compute_pawn_pos() every dispatch over a toroidal 64×64 grid
(wgsl:7573-7649); CPU holds config + presence only.

**The bubble seed** (readback_x/z consumers): patch stream center
(patch_system.inl:711-712), five distance sorts (783, 793, 808, 916, 929),
allocation window (832-858), LOD banding re-exported as config lod_pawn_x/z
so the GPU cull partitions with the identical lagged yardstick (950-1001,
wgsl:8163-8171), entity draw-culling (spawn_engine.inl:178, 210, 241), ribbon
nearest-slot + away-angle (ribbon.inl:478, 672), photographer walk/capture
(gallery.inl:108-158), orb dome anchor (cartridge.hpp:745), portal door
(:847-861). Mirror-based consumers read agent_state_.slots[possessed] off the
same memcpy: agent respawn disk, possession scan, cube corral/kite.
GPU-side witness feeds that bypass CPU entirely (all keyed on
config.possessed_slot): GoL force-field, floater eviction radius, aura
compute, photographer VP, sun-shadow VP. Negative findings: spot lights are
mood-authored, NOT readback consumers (mood.inl:968-991); the pawn-following
shadow is the GPU sun VP (wgsl:6720-6725).

**PROPOSED EXTRACTION BOUNDARY (m5).** The witness contract is: (a)
PlayerState graduates at m1 and becomes the named witness record; (b) the P5
harvest remains the SOLE author of readback_* (spine, SEAM[spine:P5]); (c)
possession transfer stays an agents-owned door writing possessed_slot +
set_possessed_slot as one act; (d) fpv/sky toggles stay input commands; (e)
the sky trio leaves PlayerState per the P3 ruling (either R5 option removes
it); (f) consumers take the witness by const reference (or the two floats)
through their deps structs — never via Cartridge*; (g) the GPU-side witness
stays GPU-resident, selector = config.possessed_slot; no CPU camera mirror is
created; no readback_y is invented. Under this boundary the witness movement
in the score is exactly: HARVEST (maps + mirror verbs + portal door) and
CAPTURE (staging copies), plus update()'s two witness consumers.

---

## 5 — R5: THE SKY REACH (P3 decision material)

**Full extent.** CPU rider state: player_ sky trio (cartridge.hpp:233-235).
Entry: F8 → toggle_sky_mode (input.inl:167, 246-250). Signal: update() ships
sky_mode + seven NEUTRAL zeros (584-603); authoritative author is the render()
resync block (967-982) → GPUState::resync_sky_head (state.hpp:1595-1607,
asserted 32-byte sub-range write). Ribbon side: saddle mount computed
unconditionally in ribbon_advance_head (ribbon.inl:248-298; MOUNT_* constants
ribbon.hpp:94-101); release edge + prev latch (386-399); flight-input author +
yaw ease (406-423); wander preemption shares the same input words (424-434);
THE EVICTOR PIN (897-900) above the two-tip ref-count law. Realization: the
eight sky signal words feed ONLY the pawn kernel's mount branch
(wgsl:628-638, 5580-5607); the flown geometry reaches the ring pipeline
through head_poses uploads, not sky words.

**HAZARD (today, both options close it).** Every sky site is roster-ungated.
In a ribbon-less demo (minimal), F8 sets sky_mode=1, resync ships the default
mount (0,0,0), and the wgsl branch snaps the pawn to origin — the designed
fail-LOUD zeros become player-facing behavior (input.inl:167 vs the gated
sites at cartridge.hpp:626/745/847/866).

**OPTION A — ribbon-owned fixture (rides ROSTER.ribbon).** The mount is
already ribbon-owned (RibbonHead.mount); A completes ownership: sky trio →
a SkyFlight sub-struct in RibbonState; the resync block → the tail of
ribbon_frame_tick (the O-1 AFTER-tick constraint is then satisfied **by
construction**); F8 gains if constexpr (ROSTER.ribbon) and retargets.
**Cost: ~14 edits / 4 files, 0 new files, 0 roster/demo edits, wgsl+state.hpp
untouched.** Consequence: sky-flight ceases to exist exactly when ribbon is
off; PLAYER sheds all SEAM[ribbon:sky-mode] tenants; single CPU owner.
Residual: the pawn kernel's mount branch remains a declared cross-piece seam
(ribbon authors the possessed agent's pose).

**OPTION B — own piece, roster bit sky_flight.** Minimal tier (gates only,
nothing moves): ~8 edits / 6 files — new bit + all_enabled term, +1
initializer in EVERY demo sentence, a new FIRST-EDGE assert
(!sky_flight || ribbon — the hazard combination made illegal), F8 gate,
signal-author + resync gates; wgsl untouched-but-runtime-dead; ribbon.inl
behavior-identical at constant-false. Full own-piece tier: ~20 edits / 8
files (2 new: a direction/sky_flight pair owning the trio + resync).
**Buys the one demo sentence A cannot express: grounded-pawn-with-ribbons**
(ribbons wander, nobody flies).

**Comparison for the ruling**: A is cheaper, ends with one owner, and closes
the hazard with an existing bit; B costs a bit + a legality edge + demo-
sentence arity everywhere, and buys expressiveness. Both leave realization as
dumb wires. Jean rules at m6.

---

## 6 — R6: THE PACKER FORECAST (A5; forecast only)

**Census**: state.hpp §S4 (269-1334) holds 52 GPU-facing structs — 47
PINNED-MIRRORED (sizeof assert + named WGSL twin), 2 PINNED-MIRRORED-INDIRECT
(GPUCactusGroundEntry, GPUBladeClusterGroundEntry — no WGSL twin; byte-locked
to PalmGroundEntry via the shared 76-slot plant buffer, wgsl:7792; the buffer
is even allocated with sizeof(GPUPalmGroundEntry), state.hpp:3073-3076), 1
PINNED-LOCAL (MeshVertex — stride contract), **2 UNPINNED: ArchVertex (40 B,
the stride for ALL SIX mesh-gen families' vertex buffers — the biggest
unguarded layout in the file) and ShellVertex (36 B)**.

**Order pins beyond sizeof** — eight sub-range upload paths pin field ORDER;
five carry their own asserts (sky block, pier_count==124,
placement_patch_count==144, lod_pawn_x==384, orb palette slice==72); **three
are unguarded landmines**: stage_spot_vps view_proj slices (1677-1682), orb
dome_center (2520-2526), GoL 16-byte header (2553-2560).

**Packer-resistant shared structs** (descending): GPUFrameSignal +
GPUDesignConfig (global) · GPUAgentState (pawn+wanderers+portal) ·
GPUFloatingEntityState (sphere+cube, one 208-byte layout with a documented
alignment landmine at offset 176) · GPUPierInstance (arch+column+antenna
tiers) · GPUColumnMeshParams (antenna embedded — tier 3 + drum colors;
**antenna owns zero structs anywhere**) · PalmGroundEntry layout
(palm+cactus+blade) · ArchVertex (6 families, unpinned) · GPUPaintingSlot
(gallery+indoor via form_type) · GPUPyramidArray and arch/column mesh params
(5 consumer files each). Also: GPUPatchGrid deliberately has NO alignas(16)
(1220-1223) — a packer that "normalizes" it breaks the binding size.

**Single-module, packer-friendly**: tile grid pair, GoL quartet, aura pair,
orb pair (heavy slice pins noted), patch triplet, photographer config, light
structs, ribbon pair (minus one patch_system reach).

**Proposed m7 policy (if m7 runs at all)**: first act is pure additions — the
three missing order asserts + sizeof asserts for ArchVertex/ShellVertex;
packers then touch ONLY the single-module set; the resistant list stays
untouched; every move re-runs the full byte-pin suite. m7 remains optional.

---

## 7 — R7: THE ANOMALY DIAGNOSIS (P4 — diagnosis only, world.wgsl frozen)

**The 34 s is compile time, front-loaded at pipeline creation** (tPipe wraps
CreateComputePipeline; runtime is one thread — DispatchWorkgroups(1,1,1),
renderer.hpp:358).

**Dominant driver = ground-chain multiplicity × FXC const-loop unrolling of
the procedural terrain lattice.** behavior_player_controlled makes 2 ground
calls; pawn_ground_resolve inlines query_ground_walker_pair at 4 sites and
terrain_normal_at inlines the tilt query at 3 → **7 full contributor chains**,
each carrying terrain_height_at, whose 6 bands × 2×2 lattice nodes unroll to
24 evaluate_lattice_wave bodies (~190 scalar ops each: 13 hashes, 3
Box-Muller gaussians, both radial+directional wave arms compiled).
**Arithmetic: 7 × 24 = 168 lattice bodies ≈ 32,000 straight-line
instructions**, plus 42 unrolled overlay-wave bodies, 56 activity/tile nodes,
and 7 rolled copies each of the pier(≤68)/pyramid(≤8)/GoL(≤8)/pulse(≤8)
loops → a ~35-40k-instruction single function. FXC's optimizer is superlinear
in straight-line size → 34 s.

**The repo already knows**: the unified kernel compiled at 48 s and was split
(wgsl:6146-6153); query_ground_walker_pair exists to halve 14→7 chains
(5428-5432); GoL suppression is hand-fused to avoid double zone loops
(2716-2721); the pier loop is runtime-bounded expressly "to keep FXC happy"
(:1958). **Dead gate found**: PAWN_GOL_GROUND_ENABLED (:1727-1729) is
advertised as a compile gate for exactly this kernel and has ZERO references —
documentation-level anomaly, reported not repaired.

**Prediction**: update_other_agents runs the SAME chain from all 9 behaviors
via agent_post_step (9 inline sites vs the player's 7; only the aura form
differs) — same compile class expected; its "compiled once per kernel"
comment (5512-5513) does not hold under FXC full inlining. Contrast:
update_camera = 1 chain; compute_vp = 0 (ms class). Roster note: wanderers
gates the other-agents pipeline, but update_player_agent has no gate — the
34 s lands in **every** demo sentence.

**Levers (named, not cut)**: the 7× chain multiplier (fewer query sites; a
baked static-base for the player resolve) and the 24× lattice unroll (bake to
texture). Any cut is bus-side-aware, pull-based, and Jean-ordered — the FXC
laws are absolute.

---

## 8 — PROPOSED STAGE ORDER (census-adjusted)

- **m1 — SERVICES GRADUATION.** TimeState, MoodState, PlayerState,
  TransitionPhase to file scope (homes per D3); readback enums + dispatch
  wrappers stay by evidence. Pure type moves, zero behavior. Must precede m3
  (deps structs need the types complete at file scope).
- **m2 — THE SCORE REWRITE + THE SCORE CENSUS TOOL.** The §1.2 prose lands:
  movement banners; macro fold → twelve explicit gated lines; the nine strays
  become owner verbs; the ungated three gain gates (D7 for GoL); **P1 and P2
  die structurally**; TEARDOWN becomes per-owner verbs (census re-rank: pulled
  forward from m4 — the movement's prose needs the verbs, and the owner-verb
  pattern already coexists in today's teardown_world). Beside glaw1, the
  census tool lands: machine-checked bijection roster↔score, both directions,
  standing gate for everything after. Reorders under the RC policy (§1.2).
- **m3 — KEYHOLE DISSOLUTION + PAIR MERGES.** Deps structs per §2, in reach
  order (fewest first): population_themes → tile_world → pawn → spheres →
  orbs → gol_zones → agents → cube_behaviors → entities → gallery → ribbon →
  spawn_engine → entity_pipeline → input → mood → render_passes →
  patch_system. The four GPUState& bypasses, the gol device_ self-submit, and
  the raw config() pokes dissolve here.
- **m4 — THE CHANNELS.** The §3 conversion list minus what m2 already took:
  pawn command verbs, mood→pawn mood_allowed, lights_dirty re-home,
  rendered_slot promotion fold, tile/theme owner verbs, ribbon tip verb,
  request_recenter. Channel registry documented; F6 socket reserved.
- **m5 — THE WITNESS.** The §4 boundary: sole-author law, consumers through
  deps, possession door declared.
- **m6 — THE SKY.** Per Jean's P3 ruling (§5, Option A or B). Closes the F8
  hazard.
- **m7 — THE PACKERS (optional).** Per §6 policy: asserts first, single-module
  set only, byte-pin gates.

Order rationale: m1 unblocks m3's types; m2 early because the census tool
becomes the standing gate and P1/P2 are charter obligations; m3 before m4
because channel shapes live in deps structs; m5 after m4 so intents feed the
witness cleanly; m6 needs the witness home settled if Option A; m7 last and
skippable. PRIME INVARIANT at every stage: demo=full behavior-identical,
minimal as second witness, bisection-ready single-intent commits.

---

## 9 — THE STAMP REQUEST

- **D1 — The lens.** Land theory v3 beside v2 (finding zero). Without it the
  traceability requirement cannot be met for §11/F6/A5 citations.
- **D2 — The score.** Stamp the §1.2 prose skeleton: the movement banners,
  the witness HARVEST/CAPTURE split in render() (O-2-forced), and the RC
  reorder policy (argument + pixel gate, else banner records today's order).
- **D3 — Graduation homes.** TimeState/PlayerState/TransitionPhase →
  contracts/spine_state.hpp; MoodState → direction/mood.hpp (WorldState
  precedent). Or all four to contracts/. Pick one.
- **D4 — Teardown verbs ride m2.** Approve the census re-rank (§3.1.7, §8) or
  keep them in m4.
- **D5 — The channel conversions.** Approve the §3.1/§3.2 shapes, notably:
  lights_dirty re-homes to mood_state_; rendered_slot write folds into the
  ribbon owner path; the INT32_MAX poke becomes request_recenter; possession
  and P8 stay declared doors.
- **D6 — P3, the sky.** Option A (ribbon fixture, ~14 edits/4 files, one
  owner) or Option B (own bit, ~8-20 edits, buys grounded-pawn-with-ribbons).
  §5 is the decision material; ruling executes at m6.
- **D7 — The GoL gate form.** Wrap the render() GoL compute block in
  if constexpr (ROSTER.gol) (structural gate above today's runtime
  zone_count gate; behavior-identical — the residue proof shows zone_count
  stays 0 when disabled) — or keep runtime-only and let the census tool
  carry an exception note.
- **D8 — m7 go/no-go.** If go: the assert-first rider (§6) is a precondition.
- **D9 — Interim hazard acknowledgment.** The F8 pawn-to-origin hazard (§5)
  stays open until m6 by design; m2 does not gate the sky path (it is not a
  conductor-call site). Acknowledge or order an early F8 gate as a rider.

Full stop. The movements cut only after the stamp.
