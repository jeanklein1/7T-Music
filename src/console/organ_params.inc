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
// GPULighting::sun is what the shader actually reads (world.wgsl reads
// frame_r.lighting.sun for direction, colour, intensity and ambient), so
// these four move the picture immediately and unambiguously.
//
// AND THEY ARE THE FOUR WITH A DEFINITION. The mood apply authors this
// home on every mood change — apply_mood_lighting reads MoodProfile's
// sun_* into the deps, upload_lights composes GPULighting from them, and
// the home records what shipped. So a write here is an INSTANCE that the
// next mood change takes back, and the durable edit is the definition
// beside it. The panel writes the definition by default and lets the same
// apply produce the instance; nothing is duplicated to make that work.
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.direction, VEC3, -1.0f, 1.0f, 0.01f, "Sun", "direction", MOOD, MoodProfile, sun_direction)
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.color,     VEC3,  0.0f, 1.0f, 0.01f, "Sun", "colour",    MOOD, MoodProfile, sun_color)
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.intensity, F32,   0.0f, 4.0f, 0.01f, "Sun", "intensity", MOOD, MoodProfile, sun_intensity)
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.ambient,   F32,   0.0f, 1.0f, 0.005f, "Sun", "ambient",  MOOD, MoodProfile, sun_ambient)

// ORGAN_2b — CLEAR COLOUR, THE DEFINITION-ONLY DIAL. It passes the
// eligibility rule (apply_mood_lighting is clear_color's only runtime
// reader) but its instance, the cartridge's clearColor_, is not one of the
// exposed homes — so there is nothing to enroll a value dial on. Enrolled
// as a definition with no instance: the write is always the mood's
// meaning, the re-apply is the same apply_mood_lighting the sun dials
// already trigger, and preview is refused because there is nothing a
// preview could show.
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f, 0.01f, "Sun", "clear colour", MOOD, MoodProfile, clear_color)

// ─── FOG (the drivers' room + its witnesses) ─────────────────────────
// ORGAN_2a. Fog is DRIVEN — phase_motion_drivers flushes the field
// coupling through set_fog every frame — so the dials sit on the
// driver's seam (contracts/driver_surface.hpp) and the driven values
// convert to read-only witnesses below them. out = rest + gain·(driven
// − rest): gain 1 is the coupling verbatim, gain 0 is the curator's
// hand.
ORGAN_PARAM(DRIVERS, DriverSurface, fog.rest_density, F32,  0.0f, 0.05f, 0.0002f, "Fog", "rest density")
ORGAN_PARAM(DRIVERS, DriverSurface, fog.rest_color,   VEC3, 0.0f, 1.0f,  0.01f,   "Fog", "rest colour")
ORGAN_PARAM(DRIVERS, DriverSurface, fog.gain,         F32,  0.0f, 1.0f,  0.01f,   "Fog", "drive gain")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fog_density,  F32,  "Fog", "density (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fog_color,    VEC3, "Fog", "colour (driven)")

// ─── AURA (the drivers' room + its witnesses) ────────────────────────
// The intent is the presence ramp's rest target — moved home from
// PawnState.aura_enabled (U5); key 3 and the mood policy door write it
// here now. attack/release carry the authored AURA_PRESENCE_* values;
// height_gain multiplies profile.height_scale at the tick. The two
// driven outputs witness below.
ORGAN_PARAM(DRIVERS, DriverSurface, aura.intent,      BOOL, 0.0f, 1.0f,  1.0f,    "Aura", "intent")
ORGAN_PARAM(DRIVERS, DriverSurface, aura.attack,      F32,  0.05f, 8.0f, 0.05f,   "Aura", "attack (1/s)")
ORGAN_PARAM(DRIVERS, DriverSurface, aura.release,     F32,  0.05f, 8.0f, 0.05f,   "Aura", "release (1/s)")
ORGAN_PARAM(DRIVERS, DriverSurface, aura.height_gain, F32,  0.0f, 2.0f,  0.01f,   "Aura", "height gain")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, aura_enabled,     F32, "Aura", "enabled (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, pawn_aura_height, F32, "Aura", "height (driven)")

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
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Tier 0", "colour",     TIER, AgentTierBank, t[0].color_r)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Tier 0", "speed gain", TIER, AgentTierBank, t[0].speed_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Tier 1", "colour",     TIER, AgentTierBank, t[1].color_r)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Tier 1", "speed gain", TIER, AgentTierBank, t[1].speed_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Tier 2", "colour",     TIER, AgentTierBank, t[2].color_r)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Tier 2", "speed gain", TIER, AgentTierBank, t[2].speed_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Tier 3", "colour",     TIER, AgentTierBank, t[3].color_r)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Tier 3", "speed gain", TIER, AgentTierBank, t[3].speed_gain)
