// ═══════════════════════════════════════════════════════════════════════
// ORGAN — THE ENROLLMENT LIST
//
// ADDING A DIAL IS ONE LINE IN THIS FILE. Nothing else, anywhere.
//
//   ORGAN_PARAM(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL)
//   ORGAN_PARAM_DEF(... , DEFKIND, DEFSTRUCT, DEFFIELD)
//                                       — the same, plus a DEFINITION
//   ORGAN_PARAM_DEFONLY(TYPE, MIN, MAX, STEP, GROUP, LABEL,
//                       DEFKIND, DEFSTRUCT, DEFFIELD)
//                                       — a DEFINITION WITH NO INSTANCE
//   ORGAN_PARAM_RO(BLOCK, STRUCT, FIELD, TYPE, GROUP, LABEL)
//                                       — a WITNESS: the panel meters it
//                                         and organ_set refuses to write
//                                         it, so it carries no range
//
// BLOCK  — one of ORGAN_BLOCK_* (organ_registry.hpp). The three CPU homes
//          GPUState exposes to the panel, plus DRIVERS — the drivers' room
//          (ORGAN_2a), a contracts-tier CPU surface built FOR the panel and
//          read by the seams, which is why it needs no accessor. There is
//          no way to name anything else, which is the sovereignty boundary.
// STRUCT — the C++ home type the offset is taken in.
// FIELD  — the member, spelled as offsetof takes it. Array elements and
//          nested members are legal: `tier_gains[2].color_r`.
// TYPE   — ORGAN_F32 / U32 / BOOL / VEC3 / VEC4. VEC3/VEC4 read as many
//          contiguous floats; a colour is a VEC3 over 0..1 and the panel
//          renders it as a colour input.
//
// DEFKIND / DEFSTRUCT / DEFFIELD (O1b, generalized by ORGAN_2b) — the
//          member this dial DEFINES, the struct it lives in, and the FAMILY
//          that struct belongs to. ORGAN_PARAM enrolls a dial whose home is
//          the only truth there is; ORGAN_PARAM_DEF enrolls one whose home
//          is an INSTANCE some author re-authors, and names the fact that
//          author reads. A definition must be a run of floats with the same
//          lane count as TYPE.
//
//          MOOD  — the_board::MoodProfile. Answers "what does this mood
//                  mean"; the write's target selects WHICH mood. Eligible
//                  only if the mood apply is the field's one runtime reader
//                  — the rule lives beside MOOD_LIVE in
//                  contracts/spine_state.hpp.
//          TIER  — the_board::AgentTierBank (TIER_LIVE). Answers "what does
//                  this WORLD mean by its tiers"; there is one bank, so the
//                  target is ignored by design. Eligible only if
//                  upload_agent_registries_to_gpu is the bank's one runtime
//                  reader — the rule lives beside TIER_LIVE in
//                  contracts/agent_tiers.hpp.
//
//          ORGAN_PARAM_DEFONLY enrolls a definition with NO INSTANCE at
//          all: the fact has a definition the panel may write and no home
//          the panel may address. Its block is the sentinel ORGAN_BLOCK_NONE,
//          it takes no BLOCK/STRUCT/FIELD, and preview on it is refused.
//
// The compiler takes the offset. Rename a field and the build fails here,
// which is the whole point of a COMPILED registry: it cannot drift into
// pointing at the wrong bytes, because it does not point at bytes — it
// points at a member and lets the compiler resolve it. Both offsets work
// this way, so the definition side cannot drift from the instance side.
//
// The proving set below was census-derived from the structs themselves;
// anything whose meaning was not obvious from its name and its comment was
// left out on purpose. A wrong range on a dial is worse than a missing
// dial: the missing one is silent, the wrong one lies.
// ═══════════════════════════════════════════════════════════════════════

// ─── THE SUN ─────────────────────────────────────────────────────────
// GPULighting::sun is what the FRAGMENT shader reads (world.wgsl reads
// frame_r.lighting.sun for direction, colour, intensity and ambient), so
// colour, intensity and ambient move the picture immediately and
// unambiguously — that home is their only window. Direction is the
// exception ORGAN_2c ruled on; see the paragraph above its line below.
//
// AND THEY ARE THE FOUR WITH A DEFINITION. The mood apply authors this
// home on every mood change — apply_mood_lighting reads MoodProfile's
// sun_* into the deps, upload_lights composes GPULighting from them, and
// the home records what shipped. So a write here is an INSTANCE that the
// next mood change takes back, and the durable edit is the definition
// beside it. The panel writes the definition by default and lets the same
// apply produce the instance; nothing is duplicated to make that work.
// ORGAN_2c — sun DIRECTION is a MULTI-WINDOW fact: the author's fan fills
// config (the compute face's shadow VP — frame_r is unreachable there by
// the A8a split), frame_r.lighting (fragment shading) and the
// photographer's uniform. A preview write could move only one of them —
// light without shadow, the half-knob this campaign abolishes — so the
// dial is DEFINITION-ONLY: every write goes through apply_mood_lighting
// and moves every window. The witness beside it meters the instance the
// author produces; definition and witness disagreeing is the fan caught
// mid-stride or broken, which is exactly what a meter is for. (colour /
// intensity / ambient keep their single window and their preview.)
ORGAN_PARAM_DEFONLY(VEC3, -1.0f, 1.0f, 0.01f, "Sky & Light · Sun", "direction", MOOD, MoodProfile, sun_direction)
ORGAN_PARAM_RO(LIGHTING, GPULighting, sun.direction, VEC3, "Sky & Light · Sun", "direction (instance)")
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.color,     VEC3,  0.0f, 1.0f, 0.01f, "Sky & Light · Sun", "colour",    MOOD, MoodProfile, sun_color)
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.intensity, F32,   0.0f, 4.0f, 0.01f, "Sky & Light · Sun", "intensity", MOOD, MoodProfile, sun_intensity)
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.ambient,   F32,   0.0f, 1.0f, 0.005f, "Sky & Light · Sun", "ambient",  MOOD, MoodProfile, sun_ambient)

// ORGAN_2b — CLEAR COLOUR, THE DEFINITION-ONLY DIAL. It passes the
// eligibility rule (apply_mood_lighting is clear_color's only runtime
// reader) but its instance, the cartridge's clearColor_, is not one of the
// exposed homes — so there is nothing to enroll a value dial on. Enrolled
// as a definition with no instance: the write is always the mood's
// meaning, the re-apply is the same apply_mood_lighting the sun dials
// already trigger, and preview is refused because there is nothing a
// preview could show.
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f, 0.01f, "Sky & Light · Sun", "clear colour", MOOD, MoodProfile, clear_color)

// ─── FOG (the drivers' room + its witnesses) ─────────────────────────
// ORGAN_2a. Fog is DRIVEN — phase_motion_drivers flushes the field
// coupling through set_fog every frame — so the dials sit on the
// driver's seam (contracts/driver_surface.hpp) and the driven values
// convert to read-only witnesses below them. out = rest + gain·(driven
// − rest): gain 1 is the coupling verbatim, gain 0 is the curator's
// hand.
ORGAN_PARAM(DRIVERS, DriverSurface, fog.rest_density, F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Fog", "rest density")
ORGAN_PARAM(DRIVERS, DriverSurface, fog.rest_color,   VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Fog", "rest colour")
ORGAN_PARAM(DRIVERS, DriverSurface, fog.gain,         F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Fog", "drive gain")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fog_density,  F32,  "Atmosphere · Fog", "density (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fog_color,    VEC3, "Atmosphere · Fog", "colour (driven)")

// ─── Atmosphere · Veil ───────────────────────────────────────────────
// ORGAN_3 w1 (C1). Boot-pinned from Dim::*_DEFAULT and authored by
// nothing else — the tree's own word beside them is "LIVE values ride
// config (veil_ring/veil_icing/lod0_radius, tunable — config-tune live)".
//
// THE VEIL CHAIN BOUNDS THESE THREE, AND THE RANGES CARRY IT. state.hpp
// proves the chain at compile time:
//     EXIST_RADIUS(350) > VEIL_RING > LOD0_RADIUS
//     VEIL_RING - VEIL_ICING > LOD0_RADIUS
// A static_assert cannot guard a runtime dial, so the boxes below are cut
// so that EVERY combination inside them still satisfies the chain:
// worst case ring 265 - icing 60 = 205 > 175 = max lod0, and max ring 349
// < 350 = EXIST. Widen one of these three and the chain needs re-proving
// against the other two — that is the whole reason the numbers are odd.
ORGAN_PARAM(CONFIG, GPUDesignConfig, veil_ring,   F32, 265.0f, 349.0f, 0.5f,  "Atmosphere · Veil", "ring (draw authority)")
ORGAN_PARAM(CONFIG, GPUDesignConfig, veil_icing,  F32, 0.0f,   60.0f,  0.25f, "Atmosphere · Veil", "icing band")
ORGAN_PARAM(CONFIG, GPUDesignConfig, lod0_radius, F32, 0.0f,   175.0f, 1.0f,  "Atmosphere · Veil", "LOD0 core")
ORGAN_PARAM(CONFIG, GPUDesignConfig, veil_dither, F32, 0.0f,   1.0f,   1.0f,  "Atmosphere · Veil", "rim dither (>0.5)")

// ─── Terrain · Palette N ────────────────────────────────────────
// ORGAN_3 w1 (C1). THE TERRAIN_LOOKS PANEL ROW 1 (surface/
// terrain_looks.hpp) authors these; GPUState::initializeState is the only
// writer, at boot. Ranges are the panel's own word: "center/light = rgb
// 0-1; weight = selection probability". The weights' "sums 1.0" is a
// convention the WGSL normalises, NOT a clamp — four independent lanes.
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_center[0][0], VEC3, 0.0f, 1.0f, 0.01f, "Terrain · Palette 0 sand",   "centre")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_light[0][0],  VEC3, 0.0f, 1.0f, 0.01f, "Terrain · Palette 0 sand",   "light")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_center[1][0], VEC3, 0.0f, 1.0f, 0.01f, "Terrain · Palette 1 salmon", "centre")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_light[1][0],  VEC3, 0.0f, 1.0f, 0.01f, "Terrain · Palette 1 salmon", "light")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_center[2][0], VEC3, 0.0f, 1.0f, 0.01f, "Terrain · Palette 2 green",  "centre")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_light[2][0],  VEC3, 0.0f, 1.0f, 0.01f, "Terrain · Palette 2 green",  "light")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_center[3][0], VEC3, 0.0f, 1.0f, 0.01f, "Terrain · Palette 3 warm",   "centre")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_light[3][0],  VEC3, 0.0f, 1.0f, 0.01f, "Terrain · Palette 3 warm",   "light")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_weight[0], F32, 0.0f, 1.0f, 0.005f, "Terrain · Palette mix", "sand")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_weight[1], F32, 0.0f, 1.0f, 0.005f, "Terrain · Palette mix", "salmon")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_weight[2], F32, 0.0f, 1.0f, 0.005f, "Terrain · Palette mix", "green")
ORGAN_PARAM(CONFIG, GPUDesignConfig, palette_weight[3], F32, 0.0f, 1.0f, 0.005f, "Terrain · Palette mix", "warm")

// ─── Terrain · Motion / Bands / Modes / GoL ─────────────────────
// ORGAN_3 w1 (C1). THE TERRAIN_LOOKS PANEL ROW 2 — the surface voice's
// SILENCE. The cartridge boot-pin block writes exactly these through the
// setters and, per the panel's own note, "the mode trio is DRIVERLESS
// since gen-1 retirement — driver-ready, held at rest by this row".
// Enrolling them is what driver-ready was waiting for. Ranges are ROW 2's
// own units, quoted at each line.
ORGAN_PARAM(CONFIG, GPUDesignConfig, terrain_time, F32, 0.0f, 64.0f, 0.25f, "Terrain · Motion", "clock (beats)")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_blend_0, F32, -1.0f, 1.0f, 0.01f, "Terrain · Bands", "0 continental")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_blend_1, F32, -1.0f, 1.0f, 0.01f, "Terrain · Bands", "1 regional")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_blend_2, F32, -1.0f, 1.0f, 0.01f, "Terrain · Bands", "2 local")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_blend_3, F32, -1.0f, 1.0f, 0.01f, "Terrain · Bands", "3 detail")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_blend_4, F32, -1.0f, 1.0f, 0.01f, "Terrain · Bands", "4 fine")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_blend_5, F32, -1.0f, 1.0f, 0.01f, "Terrain · Bands", "5 tectonic")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_phase_origin_0, F32, 0.0f, 64.0f, 0.25f, "Terrain · Band phase", "0 continental")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_phase_origin_1, F32, 0.0f, 64.0f, 0.25f, "Terrain · Band phase", "1 regional")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_phase_origin_2, F32, 0.0f, 64.0f, 0.25f, "Terrain · Band phase", "2 local")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_phase_origin_3, F32, 0.0f, 64.0f, 0.25f, "Terrain · Band phase", "3 detail")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_phase_origin_4, F32, 0.0f, 64.0f, 0.25f, "Terrain · Band phase", "4 fine")
ORGAN_PARAM(CONFIG, GPUDesignConfig, band_phase_origin_5, F32, 0.0f, 64.0f, 0.25f, "Terrain · Band phase", "5 tectonic")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mode_color_shift,       F32, -1.0f, 1.0f, 0.01f, "Terrain · Modes", "colour shift")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mode_checker_scatter,   F32, -1.0f, 1.0f, 0.01f, "Terrain · Modes", "checker scatter")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mode_palette_target,    F32, 0.0f, 3.0f, 1.0f,   "Terrain · Modes", "palette target (idx)")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mode_palette_intensity, F32, 0.0f, 1.0f, 0.005f, "Terrain · Modes", "palette intensity")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mode_discrete_tier,     F32, 0.0f, 4.0f, 1.0f,   "Terrain · Modes", "discrete tier (idx)")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mode_gol_tick_scale,   F32, 0.1f, 4.0f, 0.02f, "Terrain · GoL", "tick scale")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mode_gol_height_scale, F32, 0.0f, 4.0f, 0.02f, "Terrain · GoL", "height scale")

// ─── Terrain · Mosaic ──────────────────────────────────────────
// ORGAN_3 w1 (C1). Rests are Dim::MOSAIC_*; state.hpp's own note says
// "All live-tunable via config; these are the rests."
ORGAN_PARAM(CONFIG, GPUDesignConfig, mosaic_enable,        F32, 0.0f, 1.0f,  1.0f,   "Terrain · Mosaic", "enable")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mosaic_shard_size,    F32, 0.0f, 1.2f,  0.005f, "Terrain · Mosaic", "shard size")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mosaic_passage_scale, F32, 0.0f, 48.0f, 0.25f,  "Terrain · Mosaic", "passage scale")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mosaic_blend,         F32, 0.0f, 1.0f,  0.005f, "Terrain · Mosaic", "boundary blend")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mosaic_facet,         F32, 0.0f, 1.0f,  0.005f, "Terrain · Mosaic", "facet lean")

// ─── AURA (the drivers' room + its witnesses) ────────────────────────
// The intent is the presence ramp's rest target — moved home from
// PawnState.aura_enabled (U5); key 3 and the mood policy door write it
// here now. attack/release carry the authored AURA_PRESENCE_* values;
// height_gain multiplies profile.height_scale at the tick. The two
// driven outputs witness below.
ORGAN_PARAM(DRIVERS, DriverSurface, aura.intent,      BOOL, 0.0f, 1.0f,  1.0f,    "Pawn · Aura", "intent")
ORGAN_PARAM(DRIVERS, DriverSurface, aura.attack,      F32,  0.05f, 8.0f, 0.05f,   "Pawn · Aura", "attack (1/s)")
ORGAN_PARAM(DRIVERS, DriverSurface, aura.release,     F32,  0.05f, 8.0f, 0.05f,   "Pawn · Aura", "release (1/s)")
ORGAN_PARAM(DRIVERS, DriverSurface, aura.height_gain, F32,  0.0f, 2.0f,  0.01f,   "Pawn · Aura", "height gain")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, aura_enabled,     F32, "Pawn · Aura", "enabled (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, pawn_aura_height, F32, "Pawn · Aura", "height (driven)")

// ─── THE TIERS ───────────────────────────────────────────────────────
// AgentTierParams::color_r/g/b are three contiguous floats, so one VEC3
// addresses the triple; speed_gain is the tier's motion scale. The home is
// agentRoomStage_.tier_gains — ONE home, and organ_flush writes BOTH of the
// windows CHORD opened onto it (agent_room's and scene_constants').
//
// THE DEFINITIONS HERE BELONG TO THE WORLD (ORGAN_2b). TIER_LIVE
// (contracts/agent_tiers.hpp) is the bank the author reads — what this
// world means by its tiers — seeded at load from the authored
// AGENT_TIER_GAINS. A write to the bank raises the tier flag, and the
// frame boundary re-speaks upload_agent_registries_to_gpu, so the edit
// lands within a frame and survives the next world init. That answers
// O1a's question for these eight: they were contested because their
// author took the instance back, and the durable word is the bank.
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Agents · Tier 0", "colour",     TIER, AgentTierBank, t[0].color_r)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Agents · Tier 0", "speed gain", TIER, AgentTierBank, t[0].speed_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Agents · Tier 1", "colour",     TIER, AgentTierBank, t[1].color_r)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Agents · Tier 1", "speed gain", TIER, AgentTierBank, t[1].speed_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Agents · Tier 2", "colour",     TIER, AgentTierBank, t[2].color_r)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Agents · Tier 2", "speed gain", TIER, AgentTierBank, t[2].speed_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Agents · Tier 3", "colour",     TIER, AgentTierBank, t[3].color_r)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Agents · Tier 3", "speed gain", TIER, AgentTierBank, t[3].speed_gain)

// ─── Interaction · Field ───────────────────────────────────────
// ORGAN_3 w1 (C1). THE PANEL (contracts/control_panel.hpp) authors these;
// GPUState::initializeState boot-pins them into config and nothing else
// writes them. Both dialects read them — the GPU field and the ribbon
// head's CPU sum — so one dial moves both rooms, which is the panel's
// whole argument.
//
// FIELD_K IS HALF OF A COMPILE-TIME PROOF. control_panel.hpp asserts
// FIELD_BEACON_S < FIELD_K so the beacon's pull loses to field repulsion
// at the ring. This dial only ever RAISES the winning side, so the ruling
// survives every value it can reach; the beacon's own four stay unenrolled
// for exactly that reason (the ledger names it).
ORGAN_PARAM(CONFIG, GPUDesignConfig, field_slack,         F32, 0.0f, 12.0f,   0.05f, "Interaction · Field", "slack")
ORGAN_PARAM(CONFIG, GPUDesignConfig, field_k,             F32, 0.0f, 1200.0f, 5.0f,  "Interaction · Field", "k (accel)")
ORGAN_PARAM(CONFIG, GPUDesignConfig, field_fmax,          F32, 0.0f, 2400.0f, 10.0f, "Interaction · Field", "f max")
ORGAN_PARAM(CONFIG, GPUDesignConfig, field_occupier_gain, F32, 0.0f, 4.0f,    0.02f, "Interaction · Field", "occupier gain")
ORGAN_PARAM(CONFIG, GPUDesignConfig, field_authored_gain, F32, 0.0f, 4.0f,    0.02f, "Interaction · Field", "authored gain")
ORGAN_PARAM(CONFIG, GPUDesignConfig, field_gain_cube,     F32, 0.0f, 16.0f,   0.08f, "Interaction · Field", "cube gain")
ORGAN_PARAM(CONFIG, GPUDesignConfig, field_gain_sphere,   F32, 0.0f, 4.0f,    0.02f, "Interaction · Field", "sphere gain")
ORGAN_PARAM(CONFIG, GPUDesignConfig, field_gain_agent,    F32, 0.0f, 16.0f,   0.08f, "Interaction · Field", "agent gain")

// ─── Interaction · the bodies' own dials ───────────────────────
// ORGAN_3 w1 (C1). Each is boot-pinned from its own code panel and has no
// second author: point.hpp's bubble, Idle::PAWN_SPEED, CameraControls::
// MOVE_SPEED, Idle::CUBE_PLASTICITY_DEFAULT.
ORGAN_PARAM(CONFIG, GPUDesignConfig, point_bubble_radius, F32, 0.0f, 80.0f,  0.5f,   "Interaction · Point",  "bubble radius")
ORGAN_PARAM(CONFIG, GPUDesignConfig, pawn_speed,          F32, 0.0f, 60.0f,  0.5f,   "Interaction · Pawn",   "walk speed")
ORGAN_PARAM(CONFIG, GPUDesignConfig, point_fly_speed,     F32, 0.0f, 120.0f, 0.5f,   "Interaction · Camera", "fly speed")
ORGAN_PARAM(CONFIG, GPUDesignConfig, cube_plasticity,     F32, 0.0f, 1.0f,   0.005f, "Interaction · Cubes",  "plasticity λ")

// ─── Debug ──────────────────────────────────────────────────────────
// ORGAN_3 w1 (C1, D2). Config's uint32_t toggle words enroll as BOOL
// directly — four ABI bytes, no relayout, the rule D2 states.
// fpv_mode is KEY-SHARED: the keyboard writes it too, so its contest
// column will read EVENT the moment a key is used. That is the instrument
// seeing the operator, not a defect (the aura-intent precedent, 2a).
//
// mute_couplings is NOT here. It is a BITMASK over Coupling::* (ALL =
// 0x1FFFFF), and a slider from 0 to 2097151 is not a dial — D1(d), no
// defensible range, verdict DEFER-RANGE. What it wants is a checkbox per
// bit, which is a shell feature, not an enrollment line.
ORGAN_PARAM(CONFIG, GPUDesignConfig, mute_dynamics_0d, BOOL, 0.0f, 1.0f, 1.0f, "Debug", "mute dynamics (0D)")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mute_signal,      BOOL, 0.0f, 1.0f, 1.0f, "Debug", "mute signal")
ORGAN_PARAM(CONFIG, GPUDesignConfig, freeze_sphere,    BOOL, 0.0f, 1.0f, 1.0f, "Debug", "freeze sphere")
ORGAN_PARAM(CONFIG, GPUDesignConfig, fpv_mode,         BOOL, 0.0f, 1.0f, 1.0f, "Debug", "FPV mode (key-shared)")
