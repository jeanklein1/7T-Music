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
// GROUP IS A PATH (ORGAN_3): "Section · Group". The shell splits on the
// FIRST separator and renders each section as a collapsible block — two
// levels, never three. Sections are the program's VOICES, the operator's
// taxonomy rather than the homes': a dial sits under its SUBJECT, never
// under its implementation home, which is why the fog's rests are under
// Atmosphere beside their witnesses and not under a "Drivers" heading.
// There is deliberately no Camera section: camera pose is GPU truth, and
// its absence here is the sovereignty boundary made visible.
//
// KEEP THIS FILE ORDERED BY SECTION PATH. A section must be CONTIGUOUS in
// registry order, because the shell opens a new block whenever the first
// token changes — a section that appears twice renders twice. The harness
// asserts it. This file's order IS the panel's table of contents.
//
// THE CLASS LETTERS (docs/ORGAN.md, "The disposition") tag a banner where
// they aid the reader: C1 a live home with no runtime author, C2 a
// graduated console, C3 an event-authored definition, C4 a driven seam.
// C5 is the class that never appears here — it is recorded in
// docs/HANDOFFS/DISPOSITION_LEDGER.md with its one line of reason.
//
// The proving set below was census-derived from the structs themselves;
// anything whose meaning was not obvious from its name and its comment was
// left out on purpose. A wrong range on a dial is worse than a missing
// dial: the missing one is silent, the wrong one lies. Every range here
// follows the evidence order in docs/ORGAN.md and carries its evidence
// class in the ledger: [tree] the tree's own word, [identity] a physical
// identity, [heuristic] the authored value's envelope — the tag Jean's eye
// visits first.
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

// ─── Sky & Light · Dome ─────────────────────────────────────────────
// ORGAN_3 w2 (C2). ORB_CONSOLE_LIVE (contracts/orb_surface.hpp). The dome
// radius's own comment named it "Jean's dial" while it was a constexpr;
// this line is that sentence made true.
ORGAN_PARAM(ORBS, OrbConsole, dome_radius, F32, 0.0f, 2000.0f, 5.0f,   "Sky & Light · Dome", "dome radius")
ORGAN_PARAM(ORBS, OrbConsole, base_size,   F32, 0.0f, 12.0f,   0.05f,  "Sky & Light · Dome", "base size")
ORGAN_PARAM(ORBS, OrbConsole, noise_floor, F32, 0.0f, 1.0f,    0.005f, "Sky & Light · Dome", "noise floor")

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

// ─── Atmosphere · Checker (the drivers' room + its witnesses) ────
// ORGAN_3 w4 (C4). The pitch-class colour field is DRIVEN —
// phase_motion_drivers flushes the visual canvas through
// set_checker_color_field every frame — so the dials sit on the driver's
// seam and the driven values convert to read-only witnesses below them.
// The 2a recipe verbatim: out = rest + gain·(driven − rest).
//
// THE RESTS ARE LAW, not taste. terrain_looks ROW 2: "RESTS are law:
// amount 0 (the GPU maps that to each cell's seed color) and variance 0 —
// a return to seed, not gray." Dial the gain to 0 and the terrain returns
// to its seed colours; that is the authored silence, not an absence.
ORGAN_PARAM(DRIVERS, DriverSurface, checker.rest_resultant, VEC3, 0.0f, 1.0f, 0.01f,  "Atmosphere · Checker", "rest colour")
ORGAN_PARAM(DRIVERS, DriverSurface, checker.rest_amount,    F32,  0.0f, 1.0f, 0.005f, "Atmosphere · Checker", "rest amount")
ORGAN_PARAM(DRIVERS, DriverSurface, checker.rest_variance,  F32,  0.0f, 8.0f, 0.04f,  "Atmosphere · Checker", "rest variance")
ORGAN_PARAM(DRIVERS, DriverSurface, checker.gain,           F32,  0.0f, 1.0f, 0.01f,  "Atmosphere · Checker", "drive gain")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, checker_resultant,      VEC3, "Atmosphere · Checker", "colour (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, checker_music_amount,   F32,  "Atmosphere · Checker", "amount (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, checker_music_variance, F32,  "Atmosphere · Checker", "variance (driven)")

// ─── Atmosphere · Transition / Figure (witnesses only) ──────────
// ORGAN_3 w4 (C4). These four are DRIVEN and have no dial here on
// purpose. The transition fade is authored per frame by the transition
// system, and ORGAN_0's own enrollment banner already ruled that "a dial
// there would fight an author and lose confusingly". The three figure
// values are re-derived every frame from the POSSESSED FIGURE's row in
// PAWN_FIGURES — their dial is that table (a D5 composite, priced in the
// ledger), not a slider on the output. Metering them is the honest move:
// the panel shows what the author produced, and the contest column stays
// truthful about who is writing.
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fade_alpha,       F32,  "Atmosphere · Transition", "fade alpha (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fade_color,       VEC3, "Atmosphere · Transition", "fade colour (driven)")


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

// ─── Terrain · Indoor — EDITS THE NEXT SPAWN ────────────────────
// ORGAN_3 w3 (C3, DESTRUCTIVE). INDOOR_LIVE (contracts/indoor_module.hpp).
//
// THE TEMPERAMENT LAW MADE VISIBLE. These two have TEN readers between
// them and nine of those are spawn-time: cap_to_ceiling runs as each
// entity is placed, and the ribbon's indoor selection scales at spawn.
// Re-speaking a destructive author would tear the world down to apply a
// slider, so nothing here is wired to the frame boundary. The edit waits
// for the author's next natural event — the next spawn, the next mood
// change — exactly as a non-live mood's definition already waits.
//
// TURNING ONE AND SEEING NOTHING IS THE DIAL WORKING. That is why the
// group says so in its name: a dial that edits the future must say it
// edits the future, or the operator reads a working panel as a broken one.
ORGAN_PARAM(INDOOR, IndoorSurface, height_cap_fraction, F32, 0.0f, 1.0f, 0.005f, "Terrain · Indoor (edits the next spawn)", "height cap fraction")
ORGAN_PARAM(INDOOR, IndoorSurface, ribbon_scale,        F32, 0.0f, 0.6f, 0.005f, "Terrain · Indoor (edits the next spawn)", "ribbon scale")

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

// ─── Pawn · Aura profile ────────────────────────────────────────────
// ORGAN_3 w2 (C2). PAWN_AURA_LIVE (contracts/pawn_surface.hpp) — the
// module's OWN facts, which is the line ORGAN_2a's banner drew: the seam's
// dials (intent, attack, release, height gain) went to the drivers' room;
// the profile stayed with the pawn. PawnState's dead copy retired here.
//
// tint_strength rests at 0 — TUNE_1 A4 MUTED it, and the tree says "0
// silences the terrain tint outright". Enrolling it hands back a whole
// effect that is currently switched off in source.
//
// effect_mask is NOT enrolled: STATUS INTENT, uploaded and never read
// (the TUNE_1 A4 census). A dial on an unread field would lie.
ORGAN_PARAM(PAWN, PawnAuraProfile, influence_radius, F32, 0.0f, 80.0f, 0.4f,   "Pawn · Aura profile", "influence radius")
ORGAN_PARAM(PAWN, PawnAuraProfile, attack_stiffness, F32, 0.0f, 48.0f, 0.25f,  "Pawn · Aura profile", "attack stiffness")
ORGAN_PARAM(PAWN, PawnAuraProfile, attack_damping,   F32, 0.0f, 2.8f,  0.01f,  "Pawn · Aura profile", "attack damping")
ORGAN_PARAM(PAWN, PawnAuraProfile, release_rate,     F32, 0.05f, 8.0f, 0.05f,  "Pawn · Aura profile", "release rate")
ORGAN_PARAM(PAWN, PawnAuraProfile, tint_strength,    F32, 0.0f, 1.0f,  0.005f, "Pawn · Aura profile", "tint strength")
ORGAN_PARAM(PAWN, PawnAuraProfile, tint_r,           VEC3, 0.0f, 1.0f, 0.01f,  "Pawn · Aura profile", "tint colour")
ORGAN_PARAM(PAWN, PawnAuraProfile, delta_magnitude,  F32, 0.0f, 1.2f,  0.005f, "Pawn · Aura profile", "delta magnitude")
ORGAN_PARAM(PAWN, PawnAuraProfile, height_scale,     F32, 0.0f, 12.0f, 0.05f,  "Pawn · Aura profile", "height scale")
ORGAN_PARAM(PAWN, PawnAuraProfile, delta_mode,       U32, 0.0f, 1.0f,  1.0f,   "Pawn · Aura profile", "delta mode (0 conv, 1 rand)")

// ─── Pawn · Figure (driven) — witnesses only ─────────────
// ORGAN_3 w4 (C4). Re-derived every frame from the POSSESSED FIGURE's
// row in PAWN_FIGURES; the dial is that table (a D5 composite, priced in
// the ledger), not a slider on the output.
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, pawn_tilt_tau,    F32,  "Pawn · Figure (driven)", "tilt tau")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, pawn_body_radius, F32,  "Pawn · Figure (driven)", "body radius")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fpv_eye_height,   F32,  "Pawn · Figure (driven)", "eye height")

// ─── Ribbon · Head / Wander ─────────────────────────────────────────
// ORGAN_3 w2 (C2). RIBBON_LIVE (contracts/ribbon_surface.hpp). The module
// called these "All control-panel material" while they were constexpr;
// they are now a control panel.
//
// BOTH TERMS OF THE STEERING LAW GO LIVE, because the law is a min() and
// not an assert: available yaw = min(yaw_rate, speed / r_min) stays honest
// at every value either dial can reach. r_min's floor is 1, not 0 — it is
// the divisor, and zero would hand the head an unbounded yaw rate.
ORGAN_PARAM(RIBBON, RibbonSurface, yaw_rate,        F32, 0.0f, 4.0f,   0.02f, "Ribbon · Head", "yaw rate (rad/s)")
ORGAN_PARAM(RIBBON, RibbonSurface, max_speed,       F32, 0.0f, 160.0f, 0.5f,  "Ribbon · Head", "max speed")
ORGAN_PARAM(RIBBON, RibbonSurface, r_min,           F32, 1.0f, 160.0f, 0.5f,  "Ribbon · Head", "min turn radius")
ORGAN_PARAM(RIBBON, RibbonSurface, climb_rate,      F32, 0.0f, 60.0f,  0.25f, "Ribbon · Head", "climb rate")
ORGAN_PARAM(RIBBON, RibbonSurface, floor_margin,    F32, 0.0f, 100.0f, 0.5f,  "Ribbon · Head", "floor margin")
ORGAN_PARAM(RIBBON, RibbonSurface, alt_smooth_dist, F32, 0.0f, 720.0f, 2.5f,  "Ribbon · Head", "altitude smoothing")
ORGAN_PARAM(RIBBON, RibbonSurface, alt_stiff,       F32, 0.0f, 1.44f,  0.005f,"Ribbon · Head", "altitude stiffness")
ORGAN_PARAM(RIBBON, RibbonSurface, mount_setback,   F32, 0.0f, 6.0f,   0.02f, "Ribbon · Head", "mount setback")
ORGAN_PARAM(RIBBON, RibbonSurface, sky_yaw_tau,     F32, 0.0f, 2.4f,   0.01f, "Ribbon · Head", "sky yaw tau (s)")
ORGAN_PARAM(RIBBON, RibbonSurface, reference_bpm,   F32, 40.0f, 240.0f, 1.0f, "Ribbon · Head", "reference BPM")
ORGAN_PARAM(RIBBON, RibbonSurface, wander_steer_soft,    F32, 0.0f, 2.0f,   0.01f, "Ribbon · Wander", "steer softness")
ORGAN_PARAM(RIBBON, RibbonSurface, wander_yaw_max,       F32, 0.0f, 0.6f,   0.005f,"Ribbon · Wander", "yaw cap")
ORGAN_PARAM(RIBBON, RibbonSurface, wander_yaw_tau,       F32, 0.0f, 8.0f,   0.04f, "Ribbon · Wander", "yaw tau (s)")
ORGAN_PARAM(RIBBON, RibbonSurface, wander_arrive_radius, F32, 0.0f, 480.0f, 2.0f,  "Ribbon · Wander", "arrive radius")

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

// ─── Agents · Tier N — the bank's remaining columns ──────────────
// ORGAN_3 w3 (C3, kind TIER). These ride the EXISTING world re-speak:
// upload_agent_registries_to_gpu already reads TIER_LIVE and the frame
// boundary already calls it when the bank changes (ORGAN_2b U4). Six
// columns × four tiers, and not one line of new machinery — which is
// what "the cheapest class first" was supposed to buy.
//
// flee_gain's max is 0.99 and that is THE TREE'S OWN WORD, not a
// heuristic. THE CATCHABILITY LAW (CONTACT_4, quoted in
// contracts/agent_tiers.hpp): "a gain >= 1.0 means the agent matches or
// beats the player's radial speed and can NEVER be approached (nor
// possessed) ... keep every row < 1.0." A panel that let this reach 1.0
// would make the world silently uncatchable.
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].step_gain,       F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 0", "step gain", TIER, AgentTierBank, t[0].step_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].persist_gain,    F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 0", "persist gain", TIER, AgentTierBank, t[0].persist_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].contact_radius,  F32, 0.0f,   8.0f,    0.04f,  "Agents · Tier 0", "contact radius", TIER, AgentTierBank, t[0].contact_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].contact_mass,    F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 0", "contact mass", TIER, AgentTierBank, t[0].contact_mass)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].personal_radius, F32, 0.0f,   120.0f,  0.5f,   "Agents · Tier 0", "personal radius", TIER, AgentTierBank, t[0].personal_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[0].flee_gain_player, F32, 0.0f,   0.99f,   0.005f, "Agents · Tier 0", "flee gain", TIER, AgentTierBank, t[0].flee_gain_player)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].step_gain,       F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 1", "step gain", TIER, AgentTierBank, t[1].step_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].persist_gain,    F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 1", "persist gain", TIER, AgentTierBank, t[1].persist_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].contact_radius,  F32, 0.0f,   8.0f,    0.04f,  "Agents · Tier 1", "contact radius", TIER, AgentTierBank, t[1].contact_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].contact_mass,    F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 1", "contact mass", TIER, AgentTierBank, t[1].contact_mass)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].personal_radius, F32, 0.0f,   120.0f,  0.5f,   "Agents · Tier 1", "personal radius", TIER, AgentTierBank, t[1].personal_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[1].flee_gain_player, F32, 0.0f,   0.99f,   0.005f, "Agents · Tier 1", "flee gain", TIER, AgentTierBank, t[1].flee_gain_player)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].step_gain,       F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 2", "step gain", TIER, AgentTierBank, t[2].step_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].persist_gain,    F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 2", "persist gain", TIER, AgentTierBank, t[2].persist_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].contact_radius,  F32, 0.0f,   8.0f,    0.04f,  "Agents · Tier 2", "contact radius", TIER, AgentTierBank, t[2].contact_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].contact_mass,    F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 2", "contact mass", TIER, AgentTierBank, t[2].contact_mass)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].personal_radius, F32, 0.0f,   120.0f,  0.5f,   "Agents · Tier 2", "personal radius", TIER, AgentTierBank, t[2].personal_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[2].flee_gain_player, F32, 0.0f,   0.99f,   0.005f, "Agents · Tier 2", "flee gain", TIER, AgentTierBank, t[2].flee_gain_player)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].step_gain,       F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 3", "step gain", TIER, AgentTierBank, t[3].step_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].persist_gain,    F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 3", "persist gain", TIER, AgentTierBank, t[3].persist_gain)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].contact_radius,  F32, 0.0f,   8.0f,    0.04f,  "Agents · Tier 3", "contact radius", TIER, AgentTierBank, t[3].contact_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].contact_mass,    F32, 0.0f,   4.0f,    0.02f,  "Agents · Tier 3", "contact mass", TIER, AgentTierBank, t[3].contact_mass)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].personal_radius, F32, 0.0f,   120.0f,  0.5f,   "Agents · Tier 3", "personal radius", TIER, AgentTierBank, t[3].personal_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, tier_gains[3].flee_gain_player, F32, 0.0f,   0.99f,   0.005f, "Agents · Tier 3", "flee gain", TIER, AgentTierBank, t[3].flee_gain_player)

// ─── Agents · <behaviour> ────────────────────────────────────────
// ORGAN_3 w3 (C3, kind BEHAVIOR). BEHAVIOR_LIVE (contracts/agent_tiers.hpp)
// rides the tier bank's author and the tier bank's boundary — so ten rows
// of seven columns cost one enum value and no new occasion.
//
// Ranges are the authored table's own spans with D1's +25% headroom, so
// every dial opens standing where the design put it with room on both
// sides. The player row is all zeros by construction (the possessed body
// takes its motion from input, not from a behaviour) — its dials move
// nothing until a behaviour is assigned to a live agent, which is worth
// knowing before wondering why row 0 is inert.
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[0].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · player_controlled", "step rate", BEHAVIOR, AgentBehaviorBank, b[0].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[0].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · player_controlled", "step size", BEHAVIOR, AgentBehaviorBank, b[0].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[0].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · player_controlled", "persistence", BEHAVIOR, AgentBehaviorBank, b[0].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[0].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · player_controlled", "drag", BEHAVIOR, AgentBehaviorBank, b[0].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[0].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · player_controlled", "home pull", BEHAVIOR, AgentBehaviorBank, b[0].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[0].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · player_controlled", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[0].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[0].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · player_controlled", "speed cap", BEHAVIOR, AgentBehaviorBank, b[0].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[1].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · random_walk", "step rate", BEHAVIOR, AgentBehaviorBank, b[1].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[1].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · random_walk", "step size", BEHAVIOR, AgentBehaviorBank, b[1].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[1].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · random_walk", "persistence", BEHAVIOR, AgentBehaviorBank, b[1].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[1].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · random_walk", "drag", BEHAVIOR, AgentBehaviorBank, b[1].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[1].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · random_walk", "home pull", BEHAVIOR, AgentBehaviorBank, b[1].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[1].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · random_walk", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[1].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[1].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · random_walk", "speed cap", BEHAVIOR, AgentBehaviorBank, b[1].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[2].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · biased_walk", "step rate", BEHAVIOR, AgentBehaviorBank, b[2].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[2].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · biased_walk", "step size", BEHAVIOR, AgentBehaviorBank, b[2].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[2].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · biased_walk", "persistence", BEHAVIOR, AgentBehaviorBank, b[2].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[2].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · biased_walk", "drag", BEHAVIOR, AgentBehaviorBank, b[2].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[2].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · biased_walk", "home pull", BEHAVIOR, AgentBehaviorBank, b[2].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[2].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · biased_walk", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[2].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[2].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · biased_walk", "speed cap", BEHAVIOR, AgentBehaviorBank, b[2].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[3].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · wanderer", "step rate", BEHAVIOR, AgentBehaviorBank, b[3].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[3].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · wanderer", "step size", BEHAVIOR, AgentBehaviorBank, b[3].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[3].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · wanderer", "persistence", BEHAVIOR, AgentBehaviorBank, b[3].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[3].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · wanderer", "drag", BEHAVIOR, AgentBehaviorBank, b[3].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[3].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · wanderer", "home pull", BEHAVIOR, AgentBehaviorBank, b[3].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[3].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · wanderer", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[3].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[3].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · wanderer", "speed cap", BEHAVIOR, AgentBehaviorBank, b[3].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[4].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · home_seeker", "step rate", BEHAVIOR, AgentBehaviorBank, b[4].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[4].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · home_seeker", "step size", BEHAVIOR, AgentBehaviorBank, b[4].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[4].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · home_seeker", "persistence", BEHAVIOR, AgentBehaviorBank, b[4].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[4].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · home_seeker", "drag", BEHAVIOR, AgentBehaviorBank, b[4].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[4].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · home_seeker", "home pull", BEHAVIOR, AgentBehaviorBank, b[4].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[4].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · home_seeker", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[4].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[4].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · home_seeker", "speed cap", BEHAVIOR, AgentBehaviorBank, b[4].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[5].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · slow_patrol", "step rate", BEHAVIOR, AgentBehaviorBank, b[5].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[5].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · slow_patrol", "step size", BEHAVIOR, AgentBehaviorBank, b[5].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[5].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · slow_patrol", "persistence", BEHAVIOR, AgentBehaviorBank, b[5].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[5].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · slow_patrol", "drag", BEHAVIOR, AgentBehaviorBank, b[5].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[5].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · slow_patrol", "home pull", BEHAVIOR, AgentBehaviorBank, b[5].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[5].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · slow_patrol", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[5].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[5].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · slow_patrol", "speed cap", BEHAVIOR, AgentBehaviorBank, b[5].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[6].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · pursuit", "step rate", BEHAVIOR, AgentBehaviorBank, b[6].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[6].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · pursuit", "step size", BEHAVIOR, AgentBehaviorBank, b[6].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[6].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · pursuit", "persistence", BEHAVIOR, AgentBehaviorBank, b[6].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[6].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · pursuit", "drag", BEHAVIOR, AgentBehaviorBank, b[6].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[6].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · pursuit", "home pull", BEHAVIOR, AgentBehaviorBank, b[6].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[6].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · pursuit", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[6].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[6].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · pursuit", "speed cap", BEHAVIOR, AgentBehaviorBank, b[6].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[7].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · flee", "step rate", BEHAVIOR, AgentBehaviorBank, b[7].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[7].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · flee", "step size", BEHAVIOR, AgentBehaviorBank, b[7].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[7].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · flee", "persistence", BEHAVIOR, AgentBehaviorBank, b[7].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[7].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · flee", "drag", BEHAVIOR, AgentBehaviorBank, b[7].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[7].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · flee", "home pull", BEHAVIOR, AgentBehaviorBank, b[7].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[7].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · flee", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[7].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[7].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · flee", "speed cap", BEHAVIOR, AgentBehaviorBank, b[7].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[8].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · flock2d", "step rate", BEHAVIOR, AgentBehaviorBank, b[8].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[8].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · flock2d", "step size", BEHAVIOR, AgentBehaviorBank, b[8].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[8].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · flock2d", "persistence", BEHAVIOR, AgentBehaviorBank, b[8].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[8].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · flock2d", "drag", BEHAVIOR, AgentBehaviorBank, b[8].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[8].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · flock2d", "home pull", BEHAVIOR, AgentBehaviorBank, b[8].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[8].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · flock2d", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[8].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[8].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · flock2d", "speed cap", BEHAVIOR, AgentBehaviorBank, b[8].speed_cap)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[9].step_rate,       F32, 0.0f,  1.0f,   0.005f,  "Agents · levy_flight", "step rate", BEHAVIOR, AgentBehaviorBank, b[9].step_rate)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[9].step_size,       F32, 0.0f,  10.0f,  0.05f,   "Agents · levy_flight", "step size", BEHAVIOR, AgentBehaviorBank, b[9].step_size)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[9].persistence,     F32, 0.0f,  1.0f,   0.005f,  "Agents · levy_flight", "persistence", BEHAVIOR, AgentBehaviorBank, b[9].persistence)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[9].drag,            F32, 0.0f,  3.75f,  0.02f,   "Agents · levy_flight", "drag", BEHAVIOR, AgentBehaviorBank, b[9].drag)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[9].home_pull,       F32, 0.0f,  10.0f,  0.05f,   "Agents · levy_flight", "home pull", BEHAVIOR, AgentBehaviorBank, b[9].home_pull)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[9].neighbor_radius, F32, 0.0f,  50.0f,  0.25f,   "Agents · levy_flight", "neighbour radius", BEHAVIOR, AgentBehaviorBank, b[9].neighbor_radius)
ORGAN_PARAM_DEF(AGENT_ROOM, GPUAgentRoomConstants, behaviors[9].speed_cap,       F32, 0.0f,  10.0f,  0.05f,   "Agents · levy_flight", "speed cap", BEHAVIOR, AgentBehaviorBank, b[9].speed_cap)

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

// ─── Interaction · Beacon / Camera ──────────────────────────────────
// ORGAN_3 w2 (C2). PANEL_LIVE (contracts/control_panel.hpp) — the two
// families whose rests had no transport before this campaign.
//
// FIELD_BEACON_S IS ABSENT AND STAYS ABSENT until its assert has a
// runtime answer: control_panel.hpp proves FIELD_BEACON_S < FIELD_K at
// compile time, and a dial on S could cross field_k with the assert still
// reading true and no witness. The ledger prices the two ways out.
//
// The camera's four are CONTROLS, not pose. Pose is GPU truth and has no
// section by ORGAN_3's rule; what is dialled here is the input grammar.
// look sensitivity's range is the tree's own clamp: init/R … init*R.
ORGAN_PARAM(PANEL, PanelSurface, beacon.r0,   F32, 0.0f, 100.0f, 0.5f,  "Interaction · Beacon", "inner radius")
ORGAN_PARAM(PANEL, PanelSurface, beacon.r,    F32, 0.0f, 480.0f, 2.0f,  "Interaction · Beacon", "outer radius")
ORGAN_PARAM(PANEL, PanelSurface, beacon.lift, F32, 0.0f, 80.0f,  0.4f,  "Interaction · Beacon", "lift")
ORGAN_PARAM(PANEL, PanelSurface, camera.look_sens_init,    F32, 0.000625f, 0.04f, 0.0002f, "Interaction · Camera", "look sens (anchor)")
ORGAN_PARAM(PANEL, PanelSurface, camera.look_sens_step,    F32, 1.01f, 2.0f,  0.005f, "Interaction · Camera", "look sens step")
ORGAN_PARAM(PANEL, PanelSurface, camera.look_sens_range,   F32, 1.0f,  32.0f, 0.25f,  "Interaction · Camera", "look sens range")
ORGAN_PARAM(PANEL, PanelSurface, camera.scroll_zoom_scale, F32, 0.0f,  8.0f,  0.05f,  "Interaction · Camera", "scroll zoom")

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
