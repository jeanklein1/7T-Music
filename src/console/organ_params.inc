// ═══════════════════════════════════════════════════════════════════════
// ORGAN — THE ENROLLMENT LIST
//
// ADDING A DIAL IS ONE LINE IN THIS FILE. Nothing else, anywhere.
//
//   ORGAN_PARAM(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL)
//   ORGAN_PARAM_GEN(... the same columns ...)
//                                       — a GENERATIONAL dial: the edit
//                                         lands at the author's next
//                                         natural event, not now. The row
//                                         wears an `on respawn` chip, so
//                                         the panel says WHEN a stop
//                                         sounds where the hand is.
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
// EVERY FORM HAS AN _NS TWIN (ORGAN_3b) taking the NAMESPACE first:
//
//   ORGAN_PARAM_NS(NS, BLOCK, STRUCT, FIELD, ...)   — and _GEN_NS, _DEF_NS,
//                                                     _DEFONLY_NS, _RO_NS
//
// The plain names are one-line forwards supplying `the_board`, so a line
// that does not care never sees the parameter and a line that does writes
// its own namespace: `ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, ...)`.
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
//          BEHAVIOR — the_board::AgentBehaviorBank (BEHAVIOR_LIVE). One
//                  bank, target ignored. It raises TIER's flag, because a
//                  kind shares a flag when it shares an AUTHOR and both
//                  are re-spoken by upload_agent_registries_to_gpu.
//          ORB_MOOD — the_board::OrbMoodConfig (ORB_MOOD_LIVE). One row per
//                  mood, so like MOOD and unlike TIER/BEHAVIOR the write's
//                  target SELECTS the row. Its author is configure_orbs, so
//                  it carries its own flag.
//
//          ORGAN_PARAM_DEFONLY enrolls a definition with NO INSTANCE at
//          all: the fact has a definition the panel may write and no home
//          the panel may address. It takes no BLOCK/STRUCT/FIELD, preview
//          on it is refused, and its block is a SENTINEL DERIVED FROM ITS
//          KIND (ORGAN_DEFONLY_BLOCK_##DEFKIND) — 255 MoodProfile, 254
//          OrbMoodConfig, descending. Two families on one sentinel would
//          collide the moment their offsets met, and once did.
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
// TWO CARRY A DEFINITION, TWO ARE INSTANCE DIALS (ATMOS_1). The mood apply
// DRAWS this home on every mood change — apply_mood_lighting draws an
// AtmosphereInstance from MoodProfile.atmos and the world's seed,
// upload_lights composes GPULighting from it, and the home records what
// shipped. So a write here is an INSTANCE that the next apply takes back.
// Direction and colour have a definition beside them (the centre; the
// colour). Intensity and ambient do not: their definition is the LIGHT
// TIERS (Sky & Light · Light tier 1-3) — weighted regimes the seed picks
// between — so their dials here are previews, honestly labelled.
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
ORGAN_PARAM_DEFONLY(VEC3, -1.0f, 1.0f, 0.01f, "Sky & Light · Sun", "direction (centre)", MOOD, MoodProfile, atmos.sun_direction)
ORGAN_PARAM_DEFONLY(F32, 0.0f, 180.0f, 1.0f, "Sky & Light · Sun", "azimuth spread (±deg)",   MOOD, MoodProfile, atmos.sun_az_spread_deg)
ORGAN_PARAM_DEFONLY(F32, 0.0f, 45.0f,  0.5f, "Sky & Light · Sun", "elevation spread (±deg)", MOOD, MoodProfile, atmos.sun_el_spread_deg)
ORGAN_PARAM_RO(LIGHTING, GPULighting, sun.direction, VEC3, "Sky & Light · Sun", "direction (instance)")
ORGAN_PARAM(LIGHTING, GPULighting, sun.color,     VEC3,  0.0f, 1.0f, 0.01f,  "Sky & Light · Sun", "colour (instance)")
ORGAN_PARAM(LIGHTING, GPULighting, sun.intensity, F32,   0.0f, 4.0f, 0.01f,  "Sky & Light · Sun", "intensity (instance)")
ORGAN_PARAM(LIGHTING, GPULighting, sun.ambient,   F32,   0.0f, 1.0f, 0.005f, "Sky & Light · Sun", "ambient (instance)")

// ═══ THE RULE CONSOLE (ORGAN_5 P3c) ══════════════════════════════
//
// A SECTION IS ORGANIZED FOR THE HAND THAT PLAYS IT, NOT THE STRUCT THAT
// STORES IT. Until ORGAN_5 the orb rows were grouped by HOME — three
// under "Dome" because they live in OrbConsole, nineteen under "Orb mood"
// and "Orb flock" because they live in OrbMoodConfig — and the operator
// paid for it twice. Brownian's strength dial sat in the Dome group under
// the name "noise floor", nowhere near the rule it drives; and fifteen
// rule-scoped rows sat in one undifferentiated block with nothing saying
// which rule each acted in.
//
// The five groups below are the hand's taxonomy: what the sky IS (Dome),
// what populates it (Orbs), what moves ALL of it (Motion), and one group
// per rule that has dials of its own. A group whose name ends "<rule>
// rule" grows a live-rule line under its header (ORGAN_5 P2b), derived
// from the name — so this layout and that readout stay in step with no
// table between them.
//
// D4 — REGROUPING MOVES STRINGS AND ORDER, NOTHING ELSE. Every block,
// offset, type, range, step and mask bit below is what it was; only the
// group strings and the rows' order within the section changed, plus the
// two edits P3a and P3b authorise by name (one new row, one widened max).
// Order matters only because the shell opens a header when the group
// token changes, so a group must be CONTIGUOUS — the harness proves it.

// ─── Sky & Light · Dome ─────────────────────────────────────────────
// ORGAN_3 w2 (C2). ORB_CONSOLE_LIVE (contracts/orb_surface.hpp). The dome
// radius's own comment named it "Jean's dial" while it was a constexpr;
// this line is that sentence made true.
//
// WHAT THE SKY IS: the shell it is painted on, and how big a mote is.
// Both are init-baked — dome radius is also a per-frame read (the
// dynamics kernel re-projects onto the shell), so it lands smooth under
// the finger; base size is baked into orb_state at seed time and takes
// the re-speak. `noise_floor` LEFT this group at ORGAN_5 P3c: it is not
// geometry, it is brownian's strength.
ORGAN_PARAM(ORBS, OrbConsole, dome_radius, F32, 0.0f, 2000.0f, 5.0f,   "Sky & Light · Dome", "dome radius")
ORGAN_PARAM(ORBS, OrbConsole, base_size,   F32, 0.0f, 12.0f,   0.05f,  "Sky & Light · Dome", "base size")

// ─── Sky & Light · Orbs ─────────────────────────────────────────────
// ORGAN_3b P3 (C3, kind ORB_MOOD). ORB_MOOD_LIVE
// (contracts/orb_surface.hpp), block sentinel 254 — DEFINITION-ONLY,
// because configure_orbs consumes the config into GPU state and there is
// no instance the panel may address.
//
// MOOD-SELECTED, like MoodProfile and unlike TIER/BEHAVIOR: there is a
// row per mood, the write's target picks it, and the panel shows the LIVE
// mood's. The boundary re-speaks configure_orbs for the live mood only —
// applying another mood's sky now would populate this world from one we
// are not in, the same rule apply_mood_lighting already follows.
//
// A SECOND SENTINEL, NOT A SHARED ONE. This family's `offset` is an
// offset into OrbMoodConfig; MoodProfile's family uses 255 and offsets
// into MoodProfile. Sharing one sentinel would let two different structs'
// offsets collide at the same number and resolve to each other.
//
// THESE FIVE ARE THE POPULATION, and they are exactly the four that
// RE-SEED plus the one that rides with them. `enabled`, `count`,
// `palette id` and `drag` are baked into orb_state by the init kernel, so
// touching any of them re-draws the sky (ORGAN_5 P1's ORB_RESEED_BITS);
// `brightness` is a per-frame read but belongs here by SUBJECT — it is
// what a mote looks like, not how it moves. Turning one of the four and
// watching the sky redraw is the dial working: they answer "what is up
// there", and that question cannot be re-answered without re-drawing.
//
// `count`'s max is 256 — Dim::MAX_ORBS, which the config's own comment
// names as its clamp. `tierset_id` is NOT here: its "none" value is
// 0xFFFFFFFF and a 0…1 slider cannot express a sentinel without lying
// (D1(d)), which the ledger recorded at ORGAN_3 and P3 does not overturn.
//
// FIVE ROWS DIED AT ORGAN_4 P1b, each for a reason the READER gave:
//   base hue / hue variance — dead BY CONSTRUCTION. pack_palette_ always
//     packs a palette, so palette_count is never 0 and the GPU's legacy
//     single-hue branch cannot fire. The fields are docketed on L26.
//   hue converge target — the FIELD stays (it is mood-scoped and a gen-2
//     coupling revives color_converge); the ROW returns with that
//     coupling, because until then nothing moves when it is turned.
//   rule (0B 1O 2F 3K) — the applier writes os.current_motion_rule and
//     never reads cfg.motion_rule. That hardcode is a RULING, not a bug:
//     the rule is PLAYER-OWNED, seeded once to Brownian. A config the
//     applier ignores is a dead dial, so the row dies and its reachable
//     form is the door (ORGAN_DOOR_ORB_RULE).
//   flock gesture seed — the field stays (a first-run seed is its honest
//     job) but a BOOT-ONLY fact wearing a boundary chip misreports: the
//     panel would promise a landing that apply_mood_first_run_defaults_
//     refuses on every run after the first. Its reachable form is the
//     other door (ORGAN_DOOR_ORB_GESTURE).
ORGAN_PARAM_DEFONLY(BOOL, 0.0f,   1.0f,     1.0f,    "Sky & Light · Orbs", "enabled", ORB_MOOD, OrbMoodConfig, enabled)
ORGAN_PARAM_DEFONLY(U32,  0.0f,   256.0f,   1.0f,    "Sky & Light · Orbs", "count", ORB_MOOD, OrbMoodConfig, count)
ORGAN_PARAM_DEFONLY(U32,  0.0f,   3.0f,     1.0f,    "Sky & Light · Orbs", "palette id", ORB_MOOD, OrbMoodConfig, palette_id)
ORGAN_PARAM_DEFONLY(F32,  0.01f,  2.0f,     0.01f,   "Sky & Light · Orbs", "drag", ORB_MOOD, OrbMoodConfig, drag)
ORGAN_PARAM_DEFONLY(F32,  0.0f,   1.0f,     0.005f,  "Sky & Light · Orbs", "brightness", ORB_MOOD, OrbMoodConfig, brightness)

// ─── Sky & Light · Motion — all rules ───────────────────────────────
// WHAT MOVES THE SKY, WHATEVER RULE IS ON. Every row here acts under
// every rule — which is why the group's name says so, and why the
// live-rule line the rule groups carry is deliberately absent: there is
// no rule for these to be dormant under.
//
// `speed mult` is the master (ORGAN_5 P3a) — ORB_CONSOLE_LIVE's fourth
// field, a per-frame GPU read that takes a targeted partial and never a
// re-seed. Its floor of 0 is HONEST and not an `eff()` sentinel: zero is
// STILLNESS, an authored artistic state, and the kernel scales by it
// rather than testing it. The four rule drags each name their own rule in
// their label; they sit here rather than one per rule group because they
// are one four-lane fact — the shape of drag across the whole rule set —
// and splitting them would hide that shape from the hand comparing them.
//
// `noise floor` MOVED HERE from Dome at ORGAN_5 P3c. It is brownian's
// strength dial, and its old home under a geometry heading with a
// geometry-sounding name is exactly the by-the-struct grouping this
// campaign ends. Its max widens 1.0 → 3.0 `[heuristic]` (P3b): a dial
// must be able to overshoot salience before Jean's eye can tune it back,
// and the ceiling is his to rule later.
//
// FLOORS SIT ONE STEP OFF ZERO for the four rule drags: 0 is the mood
// table's "no opinion" sentinel (configure_orbs passthrough), and a
// slider cannot express a sentinel without lying (D1(d), the tierset
// precedent). Each floor is exactly one step of its own row, so the 1.0×
// pass-through stays reachable; a half-step floor would put every
// gridline off the integers.
ORGAN_PARAM(ORBS, OrbConsole, speed_mult,  F32, 0.0f, 4.0f, 0.01f,  "Sky & Light · Motion — all rules", "speed mult")
ORGAN_PARAM(ORBS, OrbConsole, noise_floor, F32, 0.0f, 3.0f, 0.005f, "Sky & Light · Motion — all rules", "noise floor")
ORGAN_PARAM_DEFONLY(F32,  -2.0f,  2.0f,     0.005f,  "Sky & Light · Motion — all rules", "rotation speed (rad/s)", ORB_MOOD, OrbMoodConfig, rotation_speed)
ORGAN_PARAM_DEFONLY(VEC3, -1.0f,  1.0f,     0.01f,   "Sky & Light · Motion — all rules", "rotation axis", ORB_MOOD, OrbMoodConfig, rotation_axis)
ORGAN_PARAM_DEFONLY(F32,  0.02f,  4.0f,     0.02f,   "Sky & Light · Motion — all rules", "drag × brownian rule", ORB_MOOD, OrbMoodConfig, rule_drag_brownian)
ORGAN_PARAM_DEFONLY(F32,  0.02f,  4.0f,     0.02f,   "Sky & Light · Motion — all rules", "drag × orbital rule", ORB_MOOD, OrbMoodConfig, rule_drag_orbital)
ORGAN_PARAM_DEFONLY(F32,  0.02f,  4.0f,     0.02f,   "Sky & Light · Motion — all rules", "drag × frozen rule", ORB_MOOD, OrbMoodConfig, rule_drag_frozen)
ORGAN_PARAM_DEFONLY(F32,  0.02f,  4.0f,     0.02f,   "Sky & Light · Motion — all rules", "drag × flocking rule", ORB_MOOD, OrbMoodConfig, rule_drag_flocking)

// ─── Sky & Light · Orbital rule ─────────────────────────────────────
// ONE ROW, AND IT EARNS ITS OWN GROUP. The orbital angular speed does
// nothing under any other rule, and the group's name is what tells the
// shell to print the live-rule line beneath it (ORGAN_5 P2b derives the
// rule from the name). A row that only sometimes acts, in a group that
// always says whether now is one of those times.
ORGAN_PARAM_DEFONLY(F32,  0.005f, 1.0f,     0.005f,  "Sky & Light · Orbital rule", "orbital speed (rad/s)", ORB_MOOD, OrbMoodConfig, orbital_base_speed)

// ─── Sky & Light · Flocking rule ────────────────────────────────────
// THE SEVEN BOIDS ROWS, and they act only under FLOCKING. The group's
// header carries the live-rule line, so an operator dragging a cohesion
// radius in brownian is told why the sky is not answering rather than
// left to conclude the dial is dead — which is the defect Jean's sweep
// named and this campaign's first law answers.
//
// FLOORS SIT ONE STEP OFF ZERO here too: 0 is the mood table's "no
// opinion" sentinel (configure_orbs eff()), so each floor is one step of
// its own row and every authored value stays on the grid.
ORGAN_PARAM_DEFONLY(F32,  1.0f,   200.0f,   1.0f,    "Sky & Light · Flocking rule", "separation radius", ORB_MOOD, OrbMoodConfig, flock_sep_radius)
ORGAN_PARAM_DEFONLY(F32,  2.0f,   480.0f,   2.0f,    "Sky & Light · Flocking rule", "alignment radius", ORB_MOOD, OrbMoodConfig, flock_align_radius)
ORGAN_PARAM_DEFONLY(F32,  4.0f,   800.0f,   4.0f,    "Sky & Light · Flocking rule", "cohesion radius", ORB_MOOD, OrbMoodConfig, flock_coh_radius)
ORGAN_PARAM_DEFONLY(F32,  0.5f,   120.0f,   0.5f,    "Sky & Light · Flocking rule", "separation weight", ORB_MOOD, OrbMoodConfig, flock_sep_weight)
ORGAN_PARAM_DEFONLY(F32,  0.2f,   32.0f,    0.2f,    "Sky & Light · Flocking rule", "alignment weight", ORB_MOOD, OrbMoodConfig, flock_align_weight)
ORGAN_PARAM_DEFONLY(F32,  0.25f,  60.0f,    0.25f,   "Sky & Light · Flocking rule", "cohesion weight", ORB_MOOD, OrbMoodConfig, flock_coh_weight)
ORGAN_PARAM_DEFONLY(F32,  1.0f,   240.0f,   1.0f,    "Sky & Light · Flocking rule", "max speed", ORB_MOOD, OrbMoodConfig, flock_max_speed)

// ─── Sky & Light · Schemes / Portals — EDITS THE NEXT DRAW ──────
// ORGAN_4 P3d (C3, DESTRUCTIVE). WORLD_DRAW_LIVE
// (contracts/mood_constants.hpp), block 10.
//
// THE WORLD-DRAW BANK'S TWO LIGHT-SIDE FAMILIES. The scheme weights roll
// once, in derive_indoor_lights, as a room is built; the portal palette
// is read once per portal, by portal_color_for, as an arch commits. Both
// are read WHILE A WORLD IS BEING DRAWN and never re-read, so the bank
// has no boundary wiring at all (D5) and every row wears the GEN chip:
// turning one and seeing nothing is the dial working — the edit lands on
// the next room and the next portal.
//
// THE WEIGHTS ARE PROBABILITIES, NOT A PARTITION. select_tier normalises
// by their sum, so four independent 0…1 lanes stay honest at every value
// — the same reading the terrain palette weights already carry.
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, scheme_weights[0], F32, 0.0f, 1.0f, 0.005f, "Sky & Light · Schemes", "cathedral")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, scheme_weights[1], F32, 0.0f, 1.0f, 0.005f, "Sky & Light · Schemes", "quartet")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, scheme_weights[2], F32, 0.0f, 1.0f, 0.005f, "Sky & Light · Schemes", "gallery")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, scheme_weights[3], F32, 0.0f, 1.0f, 0.005f, "Sky & Light · Schemes", "sanctum")
// A portal's colour is a fact about its DESTINATION (PORTAL_1), so the
// per-mood rows are indexed by destination mood and the last is the way back.
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, portal_colors[0][0], VEC3, 0.0f, 1.0f, 0.01f, "Sky & Light · Portals", "to open sunset")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, portal_colors[1][0], VEC3, 0.0f, 1.0f, 0.01f, "Sky & Light · Portals", "to indoor flat")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, portal_colors[2][0], VEC3, 0.0f, 1.0f, 0.01f, "Sky & Light · Portals", "to indoor vault")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, portal_colors[3][0], VEC3, 0.0f, 1.0f, 0.01f, "Sky & Light · Portals", "to finite outdoor")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, portal_colors[4][0], VEC3, 0.0f, 1.0f, 0.01f, "Sky & Light · Portals", "to open night")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, portal_colors[5][0], VEC3, 0.0f, 1.0f, 0.01f, "Sky & Light · Portals", "to open noon")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, portal_color_back[0], VEC3, 0.0f, 1.0f, 0.01f, "Sky & Light · Portals", "back portal")

// ─── Atmosphere · Regime 1-4 (ATMOS_2) ─────────────────────────────
// A REGIME IS A WHOLE SKY. Up to four weighted rows the world's seed
// picks between; each row carries every parameter the draw reads — the
// sun's colour, the light, the fog's rest, the clear colour — as a
// centre ± a spread (a colour's spread is ± brightness, hue kept). The
// light and the fog sit in ONE row, so "this light with this fog" is
// written, not coupled. Weight 0 is an absent regime. A write re-draws
// the live mood at the boundary with the same seed, so the instance
// moves WITH the dial: if this world is in regime 2, regime 2's rows
// move the sky now; if it is not, nothing visible changes until a world
// lands there (RESPEAK does not re-roll — the seed is the world's). The
// WEIGHT row is the exception: it moves where this world's roll lands.
// The shell grows a live line under each "Regime N" header.
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 1", "weight",                        MOOD, MoodProfile, atmos.regime[0].weight)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 1", "sun colour",                    MOOD, MoodProfile, atmos.regime[0].sun_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 1", "sun colour spread (±bright)",   MOOD, MoodProfile, atmos.regime[0].sun_color_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 4.0f,  0.01f,   "Atmosphere · Regime 1", "intensity",                     MOOD, MoodProfile, atmos.regime[0].intensity)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 2.0f,  0.01f,   "Atmosphere · Regime 1", "intensity spread",              MOOD, MoodProfile, atmos.regime[0].intensity_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.005f,  "Atmosphere · Regime 1", "ambient",                       MOOD, MoodProfile, atmos.regime[0].ambient)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.5f,  0.005f,  "Atmosphere · Regime 1", "ambient spread",                MOOD, MoodProfile, atmos.regime[0].ambient_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Regime 1", "fog density",                   MOOD, MoodProfile, atmos.regime[0].fog_density)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.02f, 0.0001f, "Atmosphere · Regime 1", "fog density spread",            MOOD, MoodProfile, atmos.regime[0].fog_density_spread)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 1", "fog colour",                    MOOD, MoodProfile, atmos.regime[0].fog_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 1", "fog colour spread (±bright)",   MOOD, MoodProfile, atmos.regime[0].fog_color_spread)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 1", "clear colour",                  MOOD, MoodProfile, atmos.regime[0].clear_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 1", "clear colour spread (±bright)", MOOD, MoodProfile, atmos.regime[0].clear_color_spread)

ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 2", "weight",                        MOOD, MoodProfile, atmos.regime[1].weight)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 2", "sun colour",                    MOOD, MoodProfile, atmos.regime[1].sun_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 2", "sun colour spread (±bright)",   MOOD, MoodProfile, atmos.regime[1].sun_color_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 4.0f,  0.01f,   "Atmosphere · Regime 2", "intensity",                     MOOD, MoodProfile, atmos.regime[1].intensity)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 2.0f,  0.01f,   "Atmosphere · Regime 2", "intensity spread",              MOOD, MoodProfile, atmos.regime[1].intensity_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.005f,  "Atmosphere · Regime 2", "ambient",                       MOOD, MoodProfile, atmos.regime[1].ambient)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.5f,  0.005f,  "Atmosphere · Regime 2", "ambient spread",                MOOD, MoodProfile, atmos.regime[1].ambient_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Regime 2", "fog density",                   MOOD, MoodProfile, atmos.regime[1].fog_density)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.02f, 0.0001f, "Atmosphere · Regime 2", "fog density spread",            MOOD, MoodProfile, atmos.regime[1].fog_density_spread)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 2", "fog colour",                    MOOD, MoodProfile, atmos.regime[1].fog_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 2", "fog colour spread (±bright)",   MOOD, MoodProfile, atmos.regime[1].fog_color_spread)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 2", "clear colour",                  MOOD, MoodProfile, atmos.regime[1].clear_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 2", "clear colour spread (±bright)", MOOD, MoodProfile, atmos.regime[1].clear_color_spread)

ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 3", "weight",                        MOOD, MoodProfile, atmos.regime[2].weight)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 3", "sun colour",                    MOOD, MoodProfile, atmos.regime[2].sun_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 3", "sun colour spread (±bright)",   MOOD, MoodProfile, atmos.regime[2].sun_color_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 4.0f,  0.01f,   "Atmosphere · Regime 3", "intensity",                     MOOD, MoodProfile, atmos.regime[2].intensity)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 2.0f,  0.01f,   "Atmosphere · Regime 3", "intensity spread",              MOOD, MoodProfile, atmos.regime[2].intensity_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.005f,  "Atmosphere · Regime 3", "ambient",                       MOOD, MoodProfile, atmos.regime[2].ambient)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.5f,  0.005f,  "Atmosphere · Regime 3", "ambient spread",                MOOD, MoodProfile, atmos.regime[2].ambient_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Regime 3", "fog density",                   MOOD, MoodProfile, atmos.regime[2].fog_density)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.02f, 0.0001f, "Atmosphere · Regime 3", "fog density spread",            MOOD, MoodProfile, atmos.regime[2].fog_density_spread)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 3", "fog colour",                    MOOD, MoodProfile, atmos.regime[2].fog_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 3", "fog colour spread (±bright)",   MOOD, MoodProfile, atmos.regime[2].fog_color_spread)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 3", "clear colour",                  MOOD, MoodProfile, atmos.regime[2].clear_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 3", "clear colour spread (±bright)", MOOD, MoodProfile, atmos.regime[2].clear_color_spread)

ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 4", "weight",                        MOOD, MoodProfile, atmos.regime[3].weight)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 4", "sun colour",                    MOOD, MoodProfile, atmos.regime[3].sun_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 4", "sun colour spread (±bright)",   MOOD, MoodProfile, atmos.regime[3].sun_color_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 4.0f,  0.01f,   "Atmosphere · Regime 4", "intensity",                     MOOD, MoodProfile, atmos.regime[3].intensity)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 2.0f,  0.01f,   "Atmosphere · Regime 4", "intensity spread",              MOOD, MoodProfile, atmos.regime[3].intensity_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.005f,  "Atmosphere · Regime 4", "ambient",                       MOOD, MoodProfile, atmos.regime[3].ambient)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.5f,  0.005f,  "Atmosphere · Regime 4", "ambient spread",                MOOD, MoodProfile, atmos.regime[3].ambient_spread)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Regime 4", "fog density",                   MOOD, MoodProfile, atmos.regime[3].fog_density)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.02f, 0.0001f, "Atmosphere · Regime 4", "fog density spread",            MOOD, MoodProfile, atmos.regime[3].fog_density_spread)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 4", "fog colour",                    MOOD, MoodProfile, atmos.regime[3].fog_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 4", "fog colour spread (±bright)",   MOOD, MoodProfile, atmos.regime[3].fog_color_spread)
ORGAN_PARAM_DEFONLY(VEC3, 0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 4", "clear colour",                  MOOD, MoodProfile, atmos.regime[3].clear_color)
ORGAN_PARAM_DEFONLY(F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Regime 4", "clear colour spread (±bright)", MOOD, MoodProfile, atmos.regime[3].clear_color_spread)

// ─── FOG (the drivers' room + its witnesses) ─────────────────────────
// ORGAN_2a. Fog is DRIVEN — phase_motion_drivers flushes the field
// coupling through set_fog every frame — so the dials sit on the
// driver's seam (contracts/driver_surface.hpp) and the driven values
// convert to read-only witnesses below them. ATMOS_1 — the fog's REST is
// the MOOD's; ATMOS_2 — it is one column of the REGIME (the groups
// above): the canvas emits a deviation from its anchor, and this room
// keeps the gain. out = regime rest + gain·deviation: gain 1 is the
// coupling verbatim, gain 0 is the regime's own fog.
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
// ORGAN_4 P3a (C4 witness). veil_strength has ONE author and it is not a
// dial: cartridge.hpp:1206 writes `finite_mode ? 0 : 1` — a WORLD-SHAPE
// fact, not a taste. A slider here would fight that author and lose on
// the next world draw, so the panel meters it instead. Where the three
// dials above are the veil's geometry, this is its authority: 1
// outdoors, 0 wherever walls define the edge. Its DEFER row named the
// witness "defensible and cheap"; it is both.
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, veil_strength, F32, "Atmosphere · Veil", "authority (driven)")

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


// ─── Atmosphere · Fog envelope ──────────────────────────────────
// ORGAN_3b P2 (C2). CANVAS_LIVE (coupling/canvas_surface.hpp), block 9 —
// the first block BELOW the cartridge tier, reached through the _NS macro
// forms this phase minted. An ENVELOPE AUTHORITY, not a rest: the fog's
// rests are in the drivers' room (the seam) and in PARAM_LAYOUT (the
// register map); this is how long the coupling takes to get there.
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, fog_span, F32, 0.0f, 8.0f, 0.04f, "Atmosphere · Fog envelope", "field glide (beats)")

// ─── Atmosphere · Checker cadence ───────────────────────────────
// ORGAN_3b P2 (C2). The pc-colour field's READ cadence and envelope —
// how often the voice is read and how fast the resultant chases it. The
// values it chases are the drivers' room's (w4); these are the clock.
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, checker_read_span, F32, 0.25f, 32.0f, 0.25f, "Atmosphere · Checker cadence", "read span (beats)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, checker_attack,    F32, 0.25f, 32.0f, 0.25f, "Atmosphere · Checker cadence", "attack (beats)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, checker_release,   F32, 0.25f, 32.0f, 0.25f, "Atmosphere · Checker cadence", "release (beats)")

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
// TURNING ONE AND SEEING NOTHING IS THE DIAL WORKING — and ORGAN_3b moved
// that warning from the GROUP'S NAME to THE ROW ITSELF. These two enroll
// through ORGAN_PARAM_GEN, so the manifest carries cadence GEN and the
// shell prints an "on respawn" chip beside the hand that is dragging.
// Jean's diagnosis was exactly this: the panel said what every stop WAS
// and never said WHEN it spoke, so a generational dial read as a dead one.
// One fact, one home — the chip is that home now, and the group's name is
// a group's name again.
ORGAN_PARAM_GEN(INDOOR, IndoorSurface, height_cap_fraction, F32, 0.0f, 1.0f, 0.005f, "Terrain · Indoor", "height cap fraction")
ORGAN_PARAM_GEN(INDOOR, IndoorSurface, ribbon_scale,        F32, 0.0f, 0.6f, 0.005f, "Terrain · Indoor", "ribbon scale")
// ORGAN_4 P3a (C4 witnesses) — THE THREE INDOOR STRUCTURALS, METERED.
// Their author is the mood applier (direction/mood.hpp): it composes
// each from MoodProfile's STRUCTURAL group, which is C5 by the standing
// eligibility rule — world generation reads it, so a dial would edit a
// world that is already built. All three land in GPUDesignConfig, the
// block the panel already addresses, so each has the FIXED home a
// witness needs (D2) and none needs a mirror minted for it.
//
// They sit here rather than in a section of their own because a dial
// sits under its SUBJECT: these three are what "indoor" MEANS to the
// terrain, the camera and the GoL lift. The GEN chip is per-ROW since
// ORGAN_3b, so a DRIVEN meter beside two generational dials reports its
// own cadence and not the group's.
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, terrain_amp_ceiling, F32, "Terrain · Indoor", "terrain amp ceiling (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, ceiling_height,      F32, "Terrain · Indoor", "ceiling height (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, indoor_height_cap,   F32, "Terrain · Indoor", "GoL lift cap (driven)")

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

// ─── Ribbon · Swell / Tint envelope ─────────────────────────────
// ORGAN_3b P2 (C2). CANVAS_LIVE again — under RIBBON because a dial sits
// under its SUBJECT and never under its implementation home. The swell
// ceiling and ramp are marked "(ruled)" in the module; the ranges keep
// the ruling reachable and let Jean re-rule it with his hand instead of
// a rebuild, which is the whole argument of this campaign.
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, swell_ceiling,    F32, 1.0f,  4.0f,  0.01f, "Ribbon · Swell", "ceiling (× idle)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, swell_ramp,       F32, 0.0f,  32.0f, 0.25f, "Ribbon · Swell", "ramp (beats)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, swell_attack,     F32, 0.01f, 8.0f,  0.04f, "Ribbon · Swell", "attack (beats)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, swell_release,    F32, 0.01f, 8.0f,  0.04f, "Ribbon · Swell", "release (beats)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, pitch_vec_origin, F32, 0.0f,  6.2832f, 0.01f, "Ribbon · Tint", "hue seating (rad)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, tint_luma,        F32, 0.0f,  1.0f,  0.005f, "Ribbon · Tint", "luma")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, tint_chroma,      F32, 0.0f,  1.0f,  0.005f, "Ribbon · Tint", "chroma")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, tint_mix_max,     F32, 0.0f,  1.0f,  0.005f, "Ribbon · Tint", "mix ceiling")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, tint_mix_attack,  F32, 0.01f, 12.0f, 0.05f,  "Ribbon · Tint", "mix attack (beats)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, tint_mix_release, F32, 0.01f, 12.0f, 0.05f,  "Ribbon · Tint", "mix release (beats)")
ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, tint_hue_span,    F32, 0.01f, 12.0f, 0.05f,  "Ribbon · Tint", "hue span (beats)")

// ─── Ribbon · Drivers (the seam's rests and its one gain) ───────
// ORGAN_4 P2 (C4). The four pipes ribbon_frame_tick reads every frame —
// amp_lateral_mult, amp_vertical_mult, color_stim, color_mix — seamed
// with the 2a recipe: out = rest + gain·(driven − rest). The rests ARE
// the hardcoded fallbacks the block carried before the room existed, so
// gain 1 at the shipped rests reproduces today's frames exactly.
//
// THE FOURTH PIPE'S BLOCKER, READ. Wave 4 held all four back because
// color_stim's fallback is a NULL POINTER and not a value. Downstream is
// `const float s = st ? st[c2] : 0.0f`, so the null branch's rest SHAPE
// is {0,0,0} — PARAM_LAYOUT's own rest column for ribbon.color_stim, and
// the same "return to seed" the checker's rests carry.
//
// NO WITNESS HERE, AND THAT IS THE HONEST ANSWER (D2). The driven quartet
// has no fixed CPU home: the last-uploaded values live at
// `rs.gpu[rs.rendered_slot]`, and the slot moves with eviction and reads
// UINT32_MAX when nothing is rendered. A witness needs a home; a varying
// slot is not one — and windows-not-homes forbids minting a mirror to
// give it one.
ORGAN_PARAM(DRIVERS, DriverSurface, ribbon.rest_amp_lat,   F32,  0.0f, 4.0f, 0.01f,  "Ribbon · Drivers", "amp lat rest")
ORGAN_PARAM(DRIVERS, DriverSurface, ribbon.rest_amp_vert,  F32,  0.0f, 4.0f, 0.01f,  "Ribbon · Drivers", "amp vert rest")
ORGAN_PARAM(DRIVERS, DriverSurface, ribbon.rest_tint_stim, VEC3, 0.0f, 1.0f, 0.01f,  "Ribbon · Drivers", "tint stim rest")
ORGAN_PARAM(DRIVERS, DriverSurface, ribbon.rest_tint_mix,  F32,  0.0f, 1.0f, 0.005f, "Ribbon · Drivers", "tint mix rest")
ORGAN_PARAM(DRIVERS, DriverSurface, ribbon.gain,           F32,  0.0f, 2.0f, 0.01f,  "Ribbon · Drivers", "drive gain")

// ─── Ribbon · Spawn / Colour — EDITS THE NEXT RIBBON ────────────
// ORGAN_4 P3d (C3, DESTRUCTIVE). RIBBON_SPAWN_LIVE
// (contracts/ribbon_surface.hpp), block 11 — a SECOND bank beside
// RIBBON_LIVE because its temperament differs: the head law above is
// read every frame, and every row below is read ONCE, as a ribbon is
// drawn. select_ribbon_for_patch rolls the chance and the colour mode,
// fill_ribbon_selection_geometry draws the colour, commit_ribbon rolls
// the wander policy, and none of them looks again.
//
// NO BOUNDARY WIRING, BY LAW (D5). Re-speaking a destructive author
// means tearing down the ribbons that already fly to apply a slider, so
// every row wears the GEN chip instead and the edit lands on the next
// ribbon — the INDOOR_LIVE temperament, applied to the second bank that
// earns it.
//
// The cruise draw is a GAUSSIAN over a fraction of RIBBON_LIVE.max_speed,
// so base/sigma/min/max are all 0…1 and the pair asserts in the contract
// keep min ≤ max at the authored values; the panel's own clamp keeps each
// inside its box, and an operator who crosses them gets a pinned cruise
// rather than an inverted one.
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, spawn_chance,        F32, 0.0f,   1.0f,   0.005f, "Ribbon · Spawn", "spawn chance")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, position_jitter,     F32, 0.0f,   1.0f,   0.005f, "Ribbon · Spawn", "position jitter")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_chance,       F32, 0.0f,   1.0f,   0.005f, "Ribbon · Spawn", "wander chance")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_cruise_base,  F32, 0.0f,   1.0f,   0.005f, "Ribbon · Spawn", "cruise mean (× max speed)")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_cruise_sigma, F32, 0.0f,   1.0f,   0.005f, "Ribbon · Spawn", "cruise sigma")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_cruise_min,   F32, 0.0f,   1.0f,   0.005f, "Ribbon · Spawn", "cruise floor")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_cruise_max,   F32, 0.0f,   1.0f,   0.005f, "Ribbon · Spawn", "cruise ceiling")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_leg_min,      F32, 0.0f,   2000.0f, 5.0f,  "Ribbon · Spawn", "leg min (u)")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_leg_max,      F32, 0.0f,   2000.0f, 5.0f,  "Ribbon · Spawn", "leg max (u)")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_spread,       F32, 0.0f,   3.1416f, 0.01f, "Ribbon · Spawn", "bearing spread (rad)")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_retarget_min, F32, 0.0f,   60.0f,  0.25f,  "Ribbon · Spawn", "retarget min (s)")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_retarget_var, F32, 0.0f,   60.0f,  0.25f,  "Ribbon · Spawn", "retarget spread (s)")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, wander_hatch_leg,    F32, 0.0f,   1200.0f, 5.0f,  "Ribbon · Spawn", "hatch leg (u)")
// The colour vocabulary. The three weights are selection probabilities
// select_tier normalises, so three independent 0…1 lanes; the palette is
// four base colours the SMOOTH draw varies around; range/bias/scales are
// that variance's shape, and bias is signed on purpose (−0.05 authored).
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, color_weights[0],    F32, 0.0f, 1.0f, 0.005f, "Ribbon · Colour", "weight smooth")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, color_weights[1],    F32, 0.0f, 1.0f, 0.005f, "Ribbon · Colour", "weight tinted")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, color_weights[2],    F32, 0.0f, 1.0f, 0.005f, "Ribbon · Colour", "weight contrast")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, smooth_palette[0][0], VEC3, 0.0f, 1.0f, 0.01f, "Ribbon · Colour", "smooth 0 sandstone")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, smooth_palette[1][0], VEC3, 0.0f, 1.0f, 0.01f, "Ribbon · Colour", "smooth 1 sky")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, smooth_palette[2][0], VEC3, 0.0f, 1.0f, 0.01f, "Ribbon · Colour", "smooth 2 golden")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, smooth_palette[3][0], VEC3, 0.0f, 1.0f, 0.01f, "Ribbon · Colour", "smooth 3 sage")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, smooth_var_range,    F32,  0.0f, 0.4f,  0.005f, "Ribbon · Colour", "smooth var range")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, smooth_var_bias,     F32, -0.2f, 0.2f,  0.005f, "Ribbon · Colour", "smooth var bias")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, smooth_var_g_scale,  F32,  0.0f, 2.0f,  0.01f,  "Ribbon · Colour", "smooth var × green")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, smooth_var_b_scale,  F32,  0.0f, 2.0f,  0.01f,  "Ribbon · Colour", "smooth var × blue")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, tinted_range[0],     VEC3, 0.0f, 1.0f,  0.01f,  "Ribbon · Colour", "tinted range")
ORGAN_PARAM_GEN(RIBBON_SPAWN, RibbonSpawnSurface, tinted_base[0],      VEC3, 0.0f, 1.0f,  0.01f,  "Ribbon · Colour", "tinted base")

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

// ─── Agents · Portals — EDITS THE NEXT DRAW ─────────────────────
// ORGAN_4 P3d (C3, DESTRUCTIVE). WORLD_DRAW_LIVE again (block 10) — the
// population half of the same bank, under the section that owns what a
// world is populated WITH.
//
// portal density is `[tree]`: the constant's own comment calls it "the
// fraction of Doorway arches that become portals", and a fraction's box
// is 0…1. Both are read as the world draws and never again, so both wear
// the GEN chip and neither is wired.
//
// THE DESTINATION LAW (ATMOS_1) — one weight per mood, by id; the walk
// normalises, so these are proportions, not probabilities, and a 0 shuts
// a door without unmaking the mood. GEN: a portal drawn after the edit
// reads the new law; one already standing keeps its destination.
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, portal_density,        F32, 0.0f, 1.0f, 0.005f, "Agents · Portals", "portal density")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, mood_weights[0], F32, 0.0f, 1.0f, 0.005f, "Agents · Portals", "draw · open sunset")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, mood_weights[1], F32, 0.0f, 1.0f, 0.005f, "Agents · Portals", "draw · indoor flat")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, mood_weights[2], F32, 0.0f, 1.0f, 0.005f, "Agents · Portals", "draw · indoor vault")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, mood_weights[3], F32, 0.0f, 1.0f, 0.005f, "Agents · Portals", "draw · finite outdoor")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, mood_weights[4], F32, 0.0f, 1.0f, 0.005f, "Agents · Portals", "draw · open night")
ORGAN_PARAM_GEN(WORLD, WorldDrawSurface, mood_weights[5], F32, 0.0f, 1.0f, 0.005f, "Agents · Portals", "draw · open noon")

// ─── Interaction · Field ───────────────────────────────────────
// ORGAN_3 w1 (C1). THE PANEL (contracts/control_panel.hpp) authors these;
// GPUState::initializeState boot-pins them into config and nothing else
// writes them. Both dialects read them — the GPU field and the ribbon
// head's CPU sum — so one dial moves both rooms, which is the panel's
// whole argument.
//
// FIELD_K IS HALF OF A RULING. control_panel.hpp asserts
// FIELD_BEACON_S < FIELD_K so the beacon's pull loses to field repulsion
// at the ring. ORGAN_3 wrote here that this dial "only ever RAISES the
// winning side" — it does not: its min is 0, and lowering k under S breaks
// the ruling from the other end. ORGAN_3b P5's clamp is what makes both
// ends safe, because it reads THIS dial's live value rather than the
// constexpr, so the beacon's S loses to whatever k is actually in force.
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

// ─── Interaction · Cubes — the floater synchrony knob ───────
// ORGAN_5 CODA, on Jean's stamp. The last executable DEFER from ORGAN_4.
//
// TWO AUTHORS, ONE HOME: this dial and cube_behaviors.hpp's
// cycle_floater_coordination, which steps the same field through
// FLOATER_COORDINATION_STEPS { 0, 0.5, 1 }. Neither is an APPLIER — no
// re-speak passes over this field at any boundary — so coexistence is
// lawful and LAST WRITER WINS, with the contest column as the honest
// narrator of who spoke last. That is the palette situation's opposite.
//
// The range IS the command's own set, and the step reaches every one of
// its stops exactly (0.5 / 0.005 = 100), so the dial can say everything
// the command can say and 198 values besides. Boot rest is 0.0, from
// config_{}'s value-init — which is the command's own step 0.
//
// AND THE CENSUS FOUND THE SECOND AUTHOR UNREACHABLE. Tree-wide there are
// exactly two hits on cycle_floater_coordination, its declaration and its
// definition, both in cube_behaviors.hpp; nothing calls it, and the F5 the
// control_panel comment names binds nothing. The contest this ruling
// prices is therefore LATENT, not live: today the panel is this field's
// only reachable author, which is the whole reason the row is worth
// landing. When the command is bound, the contest column starts narrating
// and no line here has to change.
ORGAN_PARAM(CONFIG, GPUDesignConfig, floater_coordination, F32, 0.0f, 1.0f, 0.005f, "Interaction · Cubes", "floater coordination")

// ─── Interaction · Beacon / Camera ──────────────────────────────────
// ORGAN_3 w2 (C2). PANEL_LIVE (contracts/control_panel.hpp) — the two
// families whose rests had no transport before this campaign.
//
// FIELD_BEACON_S IS PRESENT (ORGAN_3b P5). ORGAN_3 held it out because a
// compile-time proof cannot guard a runtime dial; Jean stamped the clamp
// at the writer, which guards every author rather than only this one. Its
// max is the tree's own word — FIELD_BEACON_S_MAX = FIELD_K − 1 — so the
// range and the clamp agree by construction at the authored k, and the
// writer's clamp against the LIVE k covers the rest.
//
// The camera's four are CONTROLS, not pose. Pose is GPU truth and has no
// section by ORGAN_3's rule; what is dialled here is the input grammar.
// look sensitivity's range is the tree's own clamp: init/R … init*R.
ORGAN_PARAM(PANEL, PanelSurface, beacon.r0,   F32, 0.0f, 100.0f, 0.5f,  "Interaction · Beacon", "inner radius")
ORGAN_PARAM(PANEL, PanelSurface, beacon.r,    F32, 0.0f, 480.0f, 2.0f,  "Interaction · Beacon", "outer radius")
ORGAN_PARAM(PANEL, PanelSurface, beacon.s,    F32, 0.0f, the_board::FIELD_BEACON_S_MAX, 1.0f, "Interaction · Beacon", "pull strength")
ORGAN_PARAM(PANEL, PanelSurface, beacon.lift, F32, 0.0f, 80.0f,  0.4f,  "Interaction · Beacon", "lift")
ORGAN_PARAM(PANEL, PanelSurface, camera.look_sens_init,    F32, 0.000625f, 0.04f, 0.0002f, "Interaction · Camera", "look sens (anchor)")
ORGAN_PARAM(PANEL, PanelSurface, camera.look_sens_step,    F32, 1.01f, 2.0f,  0.005f, "Interaction · Camera", "look sens step")
ORGAN_PARAM(PANEL, PanelSurface, camera.look_sens_range,   F32, 1.0f,  32.0f, 0.25f,  "Interaction · Camera", "look sens range")
ORGAN_PARAM(PANEL, PanelSurface, camera.scroll_zoom_scale, F32, 0.0f,  8.0f,  0.05f,  "Interaction · Camera", "scroll zoom")
// ORGAN_4 P3b (C2). PANEL_LIVE again — the possession reach, graduated
// out of bodies/agents.hpp's TUNING CONSOLE, which the ORGAN may not
// include. Its DEFER row wanted the derived `_SQ` twin read first: the
// square had exactly one reader, so it is derived AT that read from the
// live value now and the constant is retired. Range 0…80 is 4× the
// authored 20 `[heuristic]`; the floor is one step, because a zero reach
// makes the search unwinnable rather than merely small — the same
// argument that floors the ribbon's r_min at 1.
ORGAN_PARAM(PANEL, PanelSurface, possession.radius, F32, 0.5f, 80.0f, 0.5f, "Interaction · Possession", "reach")

// ─── Debug ──────────────────────────────────────────────────────────
// ORGAN_3 w1 (C1, D2). Config's uint32_t toggle words enroll as BOOL
// directly — four ABI bytes, no relayout, the rule D2 states.
// fpv_mode is KEY-SHARED: the keyboard writes it too, so its contest
// column will read EVENT the moment a key is used. That is the instrument
// seeing the operator, not a defect (the aura-intent precedent, 2a).
//
// mute_couplings is NOT here, and ORGAN_3c found the second reason.
// It is a BITMASK over Coupling::* (ALL = 0x1FFFFF), and a slider from 0
// to 2097151 is not a dial — D1(d), no defensible range, verdict
// DEFER-RANGE. What it wants is a checkbox per bit.
//
// ORGAN_3c censused the enum to build those checkboxes and STOPPED at
// D1's third branch. Two findings, either of which is enough:
//
//   1. `Coupling::` (realization/state.hpp) opens with its own banner —
//      "Mirrors world.wgsl's COUPLING_* bit-flag block. MUST match those
//      bit values 1:1" — so the names are an entangled WGSL mirror, not a
//      bare enum. Minting a name list beside one half of a hand-kept
//      mirror is how the two halves start to drift.
//   2. ALL is 21 bits wide and only EIGHT are named, sparsely: bits
//      1..6, 14, 16. A checkbox per bit of ALL would be thirteen toggles
//      over bits nothing reads — the dead-dial defect ORGAN_3b P0 spent a
//      commit repairing, rebuilt on purpose.
//
// The honest shape, priced and not built: a roster over the eight NAMED
// bits, asserted one-to-one against their constants. That is a ruling
// about the mirror, and rulings are not CC's to make.
ORGAN_PARAM(CONFIG, GPUDesignConfig, mute_dynamics_0d, BOOL, 0.0f, 1.0f, 1.0f, "Debug", "mute dynamics (0D)")
ORGAN_PARAM(CONFIG, GPUDesignConfig, mute_signal,      BOOL, 0.0f, 1.0f, 1.0f, "Debug", "mute signal")
ORGAN_PARAM(CONFIG, GPUDesignConfig, freeze_sphere,    BOOL, 0.0f, 1.0f, 1.0f, "Debug", "freeze sphere")
ORGAN_PARAM(CONFIG, GPUDesignConfig, fpv_mode,         BOOL, 0.0f, 1.0f, 1.0f, "Debug", "FPV mode (key-shared)")
