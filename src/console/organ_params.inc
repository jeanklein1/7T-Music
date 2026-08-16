// ═══════════════════════════════════════════════════════════════════════
// ORGAN — THE ENROLLMENT LIST
//
// ADDING A DIAL IS ONE LINE IN THIS FILE. Nothing else, anywhere.
//
//   ORGAN_PARAM(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL)
//   ORGAN_PARAM_DEF(... , DEFFIELD)     — the same, plus a DEFINITION
//
// BLOCK  — one of ORGAN_BLOCK_* (organ_registry.hpp). Only the three CPU
//          homes GPUState exposes to the panel exist; there is no way to
//          name anything else, which is the sovereignty boundary.
// STRUCT — the C++ home type the offset is taken in.
// FIELD  — the member, spelled as offsetof takes it. Array elements and
//          nested members are legal: `tier_gains[2].color_r`.
// TYPE   — ORGAN_F32 / U32 / BOOL / VEC3 / VEC4. VEC3/VEC4 read as many
//          contiguous floats; a colour is a VEC3 over 0..1 and the panel
//          renders it as a colour input.
//
// DEFFIELD (O1b) — the member of the_board::MoodProfile this dial DEFINES,
//          when it has a definition. ORGAN_PARAM enrolls a dial whose home
//          is the only truth there is; ORGAN_PARAM_DEF enrolls one whose
//          home is an INSTANCE the mood apply re-authors, and names the
//          fact the apply reads. A definition must be a run of floats with
//          the same lane count as TYPE, and the field must have the mood
//          apply as its only runtime reader — the eligibility rule lives
//          beside MOOD_LIVE in contracts/spine_state.hpp.
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
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.direction, VEC3, -1.0f, 1.0f, 0.01f, "Sun", "direction", sun_direction)
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.color,     VEC3,  0.0f, 1.0f, 0.01f, "Sun", "colour",    sun_color)
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.intensity, F32,   0.0f, 4.0f, 0.01f, "Sun", "intensity", sun_intensity)
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.ambient,   F32,   0.0f, 1.0f, 0.005f, "Sun", "ambient",  sun_ambient)

// ─── ATMOSPHERE ──────────────────────────────────────────────────────
// DesignConfig's own "Lighting & atmosphere" panel, minus the transition
// overlay (fade_alpha / fade_color are driven by the transition system;
// a dial there would fight an author and lose confusingly).
//
// No definitions here: MoodProfile has no fog and no aura, so these four
// have no second author to outlive and their homes ARE the truth. If
// O1a's census shows one of them contested anyway, the finding is that a
// fact is authored somewhere the moods cannot see — an ORGAN_2 question,
// not something to paper over with a target invented here.
ORGAN_PARAM(CONFIG, GPUDesignConfig, fog_density,      F32,  0.0f, 0.05f, 0.0002f, "Atmosphere", "fog density")
ORGAN_PARAM(CONFIG, GPUDesignConfig, fog_color,        VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere", "fog colour")
ORGAN_PARAM(CONFIG, GPUDesignConfig, aura_enabled,     F32,  0.0f, 1.0f,  1.0f,    "Atmosphere", "aura enabled")
ORGAN_PARAM(CONFIG, GPUDesignConfig, pawn_aura_height, F32,  0.0f, 8.0f,  0.05f,   "Atmosphere", "aura height (wu)")

// ─── THE TIERS ───────────────────────────────────────────────────────
// AgentTierParams::color_r/g/b are three contiguous floats, so one VEC3
// addresses the triple; speed_gain is the tier's motion scale. The home is
// agentRoomStage_.tier_gains — ONE home, and organ_flush writes BOTH of the
// windows CHORD opened onto it (agent_room's and scene_constants').
//
// No definitions here either, and for a different reason than atmosphere:
// the tiers have an author (the agents' room is staged at init from the
// tier table) but that author is not a MOOD, so MoodProfile is the wrong
// place to reach for. Whether they need a definition of their own is what
// O1a's readings are for.
ORGAN_PARAM(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Tier 0", "colour")
ORGAN_PARAM(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Tier 0", "speed gain")
ORGAN_PARAM(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Tier 1", "colour")
ORGAN_PARAM(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Tier 1", "speed gain")
ORGAN_PARAM(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Tier 2", "colour")
ORGAN_PARAM(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Tier 2", "speed gain")
ORGAN_PARAM(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].color_r,    VEC3, 0.0f, 1.0f, 0.01f, "Tier 3", "colour")
ORGAN_PARAM(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].speed_gain, F32,  0.0f, 4.0f, 0.01f, "Tier 3", "speed gain")
