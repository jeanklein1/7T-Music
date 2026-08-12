# SKY_0 — THE SKY-ERA RECON (read-only)

Read-only census against `master` at **`89ab1b8`**, zero source edits.
Answers the seven SKY_0 questions and the two riders, then closes with a
SKY_1 precondition matrix for Jean to rule on. No design here.

**One provenance note.** The handoff cites "the regenerated ledger at
`885aecd`". `885aecd` is the WALLET_1revA *subject* commit; the ledger
file was last written by `59b6d76` (`WALLET_1revA-close-ii: ledger regen`)
and `audit/BINDING_LEDGER.md` has not been touched since. Same content,
different sha — not a stop-on-mismatch, recorded so nobody re-derives it.

---

## 0. THE FACT THAT DECIDES THE ERA — there is no sky pass

Before any of the seven questions: the design's load-bearing claim is
**"chosen CPU-side at mood roll for zero per-frame GPU cost."** That is
not an aspiration to engineer toward. It is a description of what the
program already does.

The sky, as rendered today, is **two CPU-owned scalars and nothing else**:

| what you see | where it comes from | per-frame GPU cost |
|---|---|---|
| the background | `clearColor_` → `colorAttachment.clearValue` in `render_main_pass` (`render_passes.hpp`) | the clear, which happens regardless |
| the horizon / distance haze | `config.fog_color` + `config.fog_density`, read by `shade_lit` and three other WGSL sites | a `mix()` already in the shader |
| the dome of orbs | `orb_sky` — the orb compute dispatches (`orbs.hpp`, `meter_row::OrbSky`) | already paid, unrelated to sky variants |

There is **no sky shader, no sky geometry, no sky pass**. Grepping
`world.wgsl` for a sky entry point or struct returns nothing — the only
`sky` hits are a comment about the ribbon and the `orb_sky` meter row
name, which is the orb dome, not a sky.

**Consequence.** A sky variant is a different pair of RGB triples chosen
at mood roll. Its marginal per-frame cost is **exactly zero**, not
approximately zero. Three variants cost the same as one. This is the
strongest GO in the matrix and it is structural, not measured.

---

## Q1 — THE SALT. Is `78u` unclaimed?

**GO. `78u` is unclaimed, and so is the entire band 78–99 on every seed
domain.**

Census: **35 `cpu_hash` call sites** across the tree, plus every
`cpu_sample_gaussian` call (which claims `salt` *and* `salt + 1000` — see
`GAUSSIAN_PAIR_OFFSET` in `cpu_sample_gaussian`, `seed_utils.hpp`; a salt
census that ignores the +1000 shadow is incomplete).

Salts only collide **within one seed domain**. Grouped that way:

| seed domain | claimed salts |
|---|---|
| `world_state_.active_seed` | `999` (`request_mood_transition`), `5500` (`WallArtProp::SITE_SEED_OFFSET`), `5800` (indoor palette pick), `6600+i`, `7700+i`, `7710+i`, `7800+i`, `7950` (the finite forward-mood coin), `8800`, `0xA6E00000+mood_id`, `cur ^ 0xC11C` |
| destination seed (`cpu_hash(active_seed, 999u)` and the `7800+i` family) | `77` — `derive_finite_radius` **only** |
| `inst.seed` (entity instances) | `ArchProp` 600–652, `ColumnProp` 700–751, `AntennaProp` 900–…, `850/851/852`, `0xF10A7E70` |
| `site_seed` (wall art) | `WallArtProp::WALL_SHUFFLE_BASE` 2–5, `PER_WALL_BASE` 10/30/50/70, `PER_PAINTING_BASE` 100+p·10 |
| `w_seed`, `agent_seed`, `position_hash`, misc | `1`, `7`, `GalleryProp` 510+p·7 / 521, `IndoorLightProp` 1100–1102 / 1110+slot·10+field, `GoLZoneProp`, `RibbonProp`, `0xBEEF11A0` |

**Nothing in the tree uses `78u` on any domain.** The nearest claimant is
`77u` in `derive_finite_radius` — adjacent in value, and that is fine:
`cpu_hash` multiplies the property by `2891336453u` before mixing, so 77
and 78 are uncorrelated. It is worth knowing they are neighbours only
because a future reader will see them together.

**Free salts adjacent, if SKY_1 wants a band rather than a point.**
**78–99 is clear on every domain**, which matters: if `derive_sky` ever
becomes `78u + mood` or `78u + variant`, a point reservation is not
enough. Reserve the band and say so at the declaration site. Below 77 the
low integers are dirty (`1`, `2–5`, `7`, `10/30/50/70` are all live on
some domain) — do not reach down there.

**One caution.** If `derive_sky(seed, mood)` is called with
`world_state_.active_seed`, `78u` is free there. If it is called with a
*destination* seed — which the portal-color redesign needs, since a door
must know its destination's sky before that world exists — then it shares
a domain with `derive_finite_radius`'s `77u`, and both are fine. Either
call site works; SKY_1 should pick one and state it, because the two give
different answers and the portal redesign only works with the second.

---

## Q2 — PORTAL COLOR READERS

**GO. The palette has exactly one derivation and two call sites, and the
contract comment that says so is accurate.**

`PORTAL_COLORS[MOOD_COUNT][3]` and `PORTAL_COLOR_BACK[3]` live in
`contracts/mood_constants.hpp`. The single derivation is:

```cpp
inline const float* portal_color_for(const PortalDestination& dest, bool is_back) {
    return is_back ? PORTAL_COLOR_BACK : PORTAL_COLORS[dest.mood % MOOD_COUNT];
}
```

**Every reader, complete:**

| site | symbol | what it colors | design class today |
|---|---|---|---|
| `machine/entity_pipeline.hpp` | `arch_write_active` — `portal_color_for(aa.destination, aa.is_back_portal)` | the generic commit path: an arch becoming a live portal | all four |
| `bodies/grounded.hpp` | `force_spawn_portal_at` — `portal_color_for(dest, is_back_portal)` | the force-spawn channel (finite triad + back portal + door guarantee) | all four |

No third reader. `build_arch_mesh_params` reads `ActiveArch::col_*` like
every other arch and calls nothing — the comment in `mood_constants.hpp`
already states this and it verifies.

**Against the four design classes.** The current palette is indexed by
`dest.mood`, and `MOOD_COUNT == 4`, so today the classes map one-to-one
onto mood ids:

| design class | today's row | today's color |
|---|---|---|
| outdoor-infinite | `MOOD_OPEN_SUNSET` (0) | lilac `0.72/0.45/0.85` |
| indoor | `MOOD_INDOOR_FLAT` (1) | orange `0.95/0.55/0.15` |
| indoor | `MOOD_INDOOR_VAULT` (2) | yellow `0.95/0.80/0.20` |
| outdoor-finite | `MOOD_FINITE_OUTDOOR` (3) | red `0.85/0.20/0.15` |
| back | `PORTAL_COLOR_BACK` | blue `0.35/0.55/0.90` |

**What the redesign actually costs.** Three of the four classes stay
constant-indexed and need no new machinery. Only **outdoor-infinite**
changes shape: it must wear its *destination sky's* horizon color, which
means `portal_color_for` needs the destination **seed**, not just the
mood. `PortalDestination` already carries `seed` (`mood_constants.hpp`),
so the signature does not change — the body does:
`derive_sky(dest.seed, dest.mood).horizon` for the open class, table
lookup for the rest. Both call sites pass a full `PortalDestination`
already. **The redesign is one function body and one table, with no new
plumbing at either reader.**

---

## Q3 — THE FIVE ATMOSPHERE COLUMNS

**GO on all five, and the answer is cleaner than the question assumes:
all five columns have exactly ONE reader each, and it is the same
function.**

The five: `sun_direction[3]`, `sun_color[3]`, `sun_intensity`,
`sun_ambient`, `clear_color[3]` — declared in `struct MoodProfile`,
`contracts/spine_state.hpp`, authored in `MOOD_TABLE`.

| column | readers of `MoodProfile::<column>` | verdict |
|---|---|---|
| `sun_direction` | `apply_mood_lighting` only (twice — the member copy and the normalized `set_sun_direction` push) | **cuttable** |
| `sun_color` | `apply_mood_lighting` only | **cuttable** |
| `sun_intensity` | `apply_mood_lighting` only | **cuttable** |
| `sun_ambient` | `apply_mood_lighting` only | **cuttable** |
| `clear_color` | `apply_mood_lighting` only | **cuttable** |

Every other `sun_direction` / `clear_color` hit in the tree is a *different
symbol*: `GPUDesignConfig::sun_direction` (`state.hpp`), the WGSL
`DesignConfig.sun_direction` mirror, `MachineCtx::sunDirection_` /
`clearColor_`, or `GPUDirectionalLight::direction`. Those are downstream
of `apply_mood_lighting` and are **not** touched by cutting the table
columns — they keep receiving values, from a deriver instead of a row.

**The downstream chain, so SKY_1 knows what must keep working:**

```
MOOD_TABLE row  →  apply_mood_lighting  →  { c->sunDirection_[3]        →  upload_lights → GPULighting::sun.direction
                                             c->sunColor_[3]            →  upload_lights → GPULighting::sun.color
                                             mood_state_.sun_intensity  →  upload_lights → GPULighting::sun.intensity
                                             mood_state_.sun_ambient    →  upload_lights → GPULighting::sun.ambient
                                             c->clearColor_[3]          →  render_main_pass clearValue
                                                                        →  render_gallery_snapshot clearValue (gallery.hpp)
                                             gpuState_.set_sun_direction → GPUDesignConfig (shadow VP)
                                           }
```

Two extra `clearColor_` consumers worth naming because they are easy to
miss: `render_gallery_snapshot` (`gallery.hpp`) clears the photographer's
target with the same color, and `cartridge.hpp` hands it out through an
accessor. Both read `clearColor_`, not `MoodProfile::clear_color`, so both
are unaffected by the cut and both automatically inherit the new sky.
`gallery.hpp` also reads `sunDirection_` for the snapshot VP — same story.

**Three columns NOT in the five, that a sky edit will want to leave
alone**: `terrain_amp_ceiling`, `wall_height`, `indoor`. They are set in
the same function and have readers all over the tree. The five are
separable; those three are not.

**The stale comment.** `apply_mood_lighting`'s header says it does
*"sun direction/color/intensity, **fog**, ambient, terrain amp ceiling."*
**The function body contains no fog.** See Q5 — fog has not been mood's
for some time and the comment never followed. This is a present-behavior
comment violation sitting exactly on the function SKY_1 will rewrite;
flagged here so the fix lands with the edit rather than as a separate nit.

---

## Q4 — `ORB_MOOD_TABLE[mood]` CALL SITES

**GO. One call site. `MOOD_COUNT`-sized, positional, no id column.**

```cpp
configure_orbs(orbs_state, &orbs_deps, ORB_MOOD_TABLE[mood], queue);
```
— `direction/mood.hpp`, inside the mood-apply chain. That is the **only**
indexed read of the table in the tree. `configure_orbs` itself
(`bodies/orbs.hpp`) is declared once and defined once, and is called from
nowhere else.

`ORB_MOOD_TABLE` is `inline constexpr OrbMoodConfig ORB_MOOD_TABLE[MOOD_COUNT]`
(`bodies/orbs.hpp`), rows positional in mood-id order with the
`MOOD_TABLE` pattern and **no id field**, so the rows move with the ids or
not at all. Its four rows today: open_sunset enabled with 128 orbs and
rule 3 (flocking); both indoor rows disabled (`count = 0`); finite_outdoor
enabled with 128 and rule 0.

**For the moon variant.** The table is per-**mood**, and a moon is a
per-**sky-variant** fact within one mood. There is no row to put it in
today. Two shapes are available and SKY_1 picks one:

1. `configure_orbs` gains a sky argument and the moon parameters ride
   beside `OrbMoodConfig` rather than inside it. One call site to change.
2. `OrbMoodConfig` gains moon columns and `derive_sky` overrides them
   post-lookup, the way `apply_mood_first_run_defaults_` already overrides
   player-owned fields.

Shape 1 leaves the positional-row law intact; shape 2 puts variant data in
a mood-indexed table, which is the thing the `MOOD_TABLE` cut in Q3 exists
to stop doing. **Recommendation, non-binding: shape 1.**

Note also `ORB_MOOD_TABLE` is sized by `MOOD_COUNT`. If SKY_1 changes
`MOOD_COUNT` (see Q7), this table needs rows in the same commit or the
compiler zero-fills silently — the same trap `mood_name`'s sized-array
comment already warns about.

---

## Q5 — THE FOG HOMES. **This is the finding of the recon.**

**NO-GO as scoped. Fog does not live in "more than one place" — it lives
in a place the design does not know about, and mood is not one of its
homes at all.**

### The complete census

| # | home | symbol | live? |
|---|---|---|---|
| 1 | `realization/state.hpp` | `Dim::FOG_COLOR_R/G/B = 0.88/0.74/0.58` | **DEAD — zero readers in the tree** |
| 2 | `realization/state.hpp` | boot defaults written into `config_`: `fog_density = 0.003f`, `fog_color = {0.85, 0.78, 0.72}` | live (boot only) |
| 3 | `coupling/visual_canvas.hpp` | `FOG_DENSITY_NONE = 0.0030f`, `FOG_COLOR_NONE[3] = {0.85, 0.78, 0.72}` — commented **"THE ANCHOR — one home"** | live |
| 4 | `coupling/visual_canvas.hpp` | `FOG_BY_FIELD[7]`, `FOG_COLOR_BY_FIELD[7][3]` — the per-field tables | **live and driving** |
| 5 | `realization/state.hpp` | `GPUDesignConfig::fog_density` / `fog_color` — the GPU home | live |
| 6 | `realization/world.wgsl` | `DesignConfig.fog_density` / `fog_color` — the L3 mirror | live |

### The driver, and why it is a collision

**Fog is currently owned by the music coupling, not by mood.** The chain:

```
signal "all.field"  →  VisualCanvas::update  →  trajectory_release over FOG_SPAN (2 beats)
                    →  params_["fog.density"], params_["fog.color"]
                    →  cartridge.hpp: gpuState_.set_fog(fp.get(...), ...)
                    →  GPUDesignConfig  →  world.wgsl config.fog_color
```

`set_fog` has **exactly one caller** — `cartridge.hpp`, inside the canvas
flush, guarded by `fog_density_dst_.valid && fog_color_dst_.valid`. The
canvas header states the ownership as law: **"Fog has one driver: the
field."**

`apply_mood_lighting` **never calls `set_fog`**. Its comment claims fog;
its body does not have it. So today: **a mood transition does not change
the fog.** Walking from a sunset field into a vault leaves the fog color
where the music put it.

### What this means for SKY_1

The handoff's item 5 says *"SKY_1 resolves this to ONE home, anchored in
`Lighting` beside the sun."* That is a clean instruction under the
assumption that the multiple homes are redundant copies. **They are not.**
Home #4 is a live driver with a temporal glide (`FOG_SPAN`, `Segment`,
`trajectory_release`) and an explicit ownership claim. Moving fog to
`Lighting` is not a consolidation — it is **taking fog away from the music
coupling**, which is a design decision, not a cleanup, and it touches the
music era that this handoff puts out of scope.

**Three resolutions, stated neutrally for Jean:**

1. **Mood wins.** `Lighting` carries fog; the canvas's fog pipes are
   retired. Costs the music era its first and only coupling. Contradicts
   "out of scope: music-era mechanisms."
2. **Mood is the base, the canvas modulates.** `Lighting` carries the
   sky's fog as the anchor value; `FOG_BY_FIELD` becomes a multiplier or
   an offset rather than an absolute. Preserves both. Costs one extra
   composition step and a re-authoring of the seven field rows, which are
   currently absolute densities.
3. **Fog stays where it is; SKY_1 lands without it.** The sky variant
   changes `clear_color` and the portal palette only. Fog follows in a
   later round when the music era is in scope. Smallest edit; leaves a
   visible seam — a midnight sky with a sunset-tinted horizon haze.

**This recon does not choose.** But it does record that the handoff's
one-line framing of item 5 understates it, and that **resolution 1 as
written violates the handoff's own scope line.**

### Two free wins regardless of which resolution wins

- `Dim::FOG_COLOR_R/G/B` (`state.hpp`) have **zero readers**. Deletion
  candidate, P3 refuter trivially satisfied: three `constexpr float`s, no
  reference anywhere in `src/`.
- Homes #2 and #3 hold **the same three literals** (`0.85, 0.78, 0.72`)
  with no witness tying them. Either is free to drift from the other.

---

## Q6 — `GPUOrbConfig` / WGSL MIRROR — WHERE MOON PARAMETERS LAND

**GO, with one constraint that decides the shape of the edit.**

| fact | value |
|---|---|
| C++ struct | `struct alignas(16) GPUOrbConfig`, `realization/state.hpp` |
| WGSL twin | `struct OrbConfig`, `world.wgsl`, bound `@group(0) @binding(411) var<uniform> orb_config` |
| sizeof witness | `static_assert(sizeof(GPUOrbConfig) == 480, "GPUOrbConfig must be 480 bytes")`, immediately after the struct |
| current tail | `tier3_flock_sep_gain` (464), `tier3_flock_align_gain` (468), `tier3_flock_coh_gain` (472), `_tier3_flock_pad` (476) |
| binding usage | `wgpu::BufferUsage::Uniform`; `entries[1].size = sizeof(GPUOrbConfig)` at two bind-group sites, plus the layout site |

**The constraint: this struct has no free space and a hard ABI.** Every
offset is documented in-line and the file's history shows repeated
reclamation of pads (`_pad_tier0` → `brownian_radial_sign`,
`flock_weight_sign` → `flock_sep_sign`, `_tier1_flock_pad` →
`speed_mult`). Two pads remain: **`_tier2_flock_pad` (460)** and
**`_tier3_flock_pad` (476)**. There is also `_pad_anchor` (172) and the
three **dead-wire** `dome_center_x/y/z` (160/164/168), explicitly marked
`DEAD WIRE (orb VS eye-centers; ABI bytes)` — four more reclaimable
floats, but reclaiming *those* is a separate P3 deletion argument, not a
sky edit.

**Moon parameters (phase, tint, size) need at minimum 1 + 3 + 1 = 5
floats.** Two pads is not enough. So the choice is:

1. **Grow the struct** — 480 → 496 (one 16-byte row) or 512. The GROWTH
   LAW path applies: same commit, same order, both rooms, and the
   `static_assert` updated. Also update `entries[1].size` — it is
   `sizeof(GPUOrbConfig)` at both bind-group sites, so it follows
   automatically; the *layout* entry uses the same expression. **No
   binding-number change, no ledger movement.** `orb_config` is a uniform
   at 411 and stays one.
2. **Reclaim `_pad_tier2/3_flock_pad` + the three dead wires** — fits in
   the existing 480 with no growth, but scatters five moon fields across
   four non-adjacent offsets, which is exactly the readability cost the
   struct's comment history keeps paying down.

**Recommendation, non-binding: grow.** The uniform is 480 B against a
65,536 B `maxUniformBufferBindingSize` (granted on both twins — see
WEB_METER_0 §1), the ledger does not move, and the GROWTH LAW exists for
this.

**The GROWTH LAW path, confirmed by reading it** (`state.hpp`, "THE GROWTH
LAW, L5 in docs/LAWS.md"; `world.wgsl` "GROWTH LAW, same commit, same
order, same types"): **same commit, same order, same types, both rooms.**
The `sizeof` witness updates in that commit. Nothing else gates it.

---

## Q7 — `force_spawn_finite_portals` TRIAD MEMBERSHIP

**CONDITIONAL — and the condition is a question the handoff does not
answer.** The triad logic survives a pure sky-variant design untouched. It
does **not** survive if the "six-destination roster" means `MOOD_COUNT`
grows.

### What the code actually assumes

```cpp
constexpr uint32_t count = 2;
const uint32_t fwd_moods[2] = {
    (cpu_hash_f(c->world_state_.active_seed, 7950u) < 0.5f)
        ? MOOD_INDOOR_FLAT : MOOD_INDOOR_VAULT,
    MOOD_OPEN_SUNSET,
};
```

Four hard assumptions, all named in the PORTAL_2 comment
(*"A finite world offers exactly three doors: deeper in … out … and back"*):

1. **`count = 2` forwards**, `constexpr`, plus the standing back portal = three doors.
2. **Slot [0] is "deeper in"** — a coin between the two indoor moods, salt `7950u`.
3. **Slot [1] is "the way out"** — hardcoded `MOOD_OPEN_SUNSET`.
4. **Four wall candidates, one owned by the back portal** (`if (i == back_wall) continue;`), leaving exactly three usable and two taken.

### Reading A — sky variants are NOT moods (`MOOD_COUNT` stays 4)

**The triad survives completely untouched.** Slot [1]'s
`MOOD_OPEN_SUNSET` becomes `MOOD_OPEN` by the rename and means the same
thing; the destination's *sky* is then derived from `dest.seed` at the
door, which is precisely what the portal-color redesign needs. **Zero
logic change in this function.** This reading is also what makes
`derive_sky(seed, mood)` meaningful — if sky were a mood, the `seed`
argument would be redundant.

### Reading B — sky variants ARE moods (`MOOD_COUNT` → 6)

Then the triad breaks in four specific places, and the breakage is quiet
in three of them:

| site | what happens |
|---|---|
| `fwd_moods[1] = MOOD_OPEN_SUNSET` | now names one of three open skies. The "way out" door always leads to the same weather. **Silent.** |
| `pick_portal_mood` | splits the non-finite 90% into exactly `span = (1.0f - FINITE_OUTDOOR_CHANCE) / 3.0f` thirds and returns three named moods. **Hardcoded 3. Silent.** |
| `MOOD_TABLE`, `ORB_MOOD_TABLE`, `AGENT_POPULATIONS`, `CUBE_POPULATIONS`, `POPULATION_THEMES` | all `[MOOD_COUNT]`-sized and positional. Missing rows **zero-fill silently** — `mood_name`'s own comment says the compiler catches an extra entry but not a missing one. |
| `mood_name`'s `NAMES[MOOD_COUNT]` | same zero-fill; returns `nullptr`, not `"unknown"` |

Only `MOOD_TABLE` has row-drift `static_assert`s, and they pin
`MOOD_OPEN_SUNSET == 0` etc. — they catch *reordering*, not *appending
without filling*.

**The recon's read of the design intent, offered as a reading and not a
finding:** the phrase `derive_sky(seed, mood)` and the rename
`MOOD_OPEN_SUNSET → MOOD_OPEN` both point at **Reading A** — sky is
derived *within* a mood, and the mood loses "sunset" from its name
precisely because it no longer implies one. Under Reading A the
"six-destination roster" is the six distinguishable destination
*appearances* a door can advertise (3 open skies × the open mood, plus
finite_outdoor, indoor_flat, indoor_vault), not six mood ids.

**If that reading is right, Q7 is a clean GO.** If it is wrong, five
positional tables need rows in the same commit and three of them fail
silently. **Jean should rule on this explicitly** — it is the single
highest-leverage ambiguity in the handoff, and it changes SKY_1 from a
one-function edit into a table-roster campaign.

---

## RIDER 1 — THE RENAME BLAST RADIUS

**The claim "a single identifier change" is TRUE in kind and WRONG in
count. 23 occurrences across 9 files.**

| file | count | nature |
|---|---|---|
| `contracts/mood_constants.hpp` | 1 | **the definition** |
| `contracts/spine_state.hpp` | 4 | 1 row-label comment, 3 `static_assert`s |
| `direction/mood.hpp` | 2 | `fwd_moods[1]`, `pick_portal_mood` |
| `direction/input.hpp` | 1 | `GLFW_KEY_5` mood request |
| `bodies/agents.hpp` | 3 | row comment, `AGENT_POPULATIONS` row, `static_assert` |
| `bodies/cube_behaviors.hpp` | 3 | row comment, `CUBE_POPULATIONS` row, `static_assert` |
| `demos/matrix.hpp` | 5 | include comment, 2 `boot_mood` rows, 2 `static_assert`s |
| `surface/population_themes.hpp` | 1 | row-label comment |
| `realization/world.wgsl` | **2** | **comments only** (PENUMBRA_1 near-plane note) |

**Corrected claim:** it is a *pure* rename — no semantics, no ordering, no
ABI — but it is 23 sites, 8 of which are comments, and **2 of them are in
`world.wgsl`**. Under standing order 4 that means a comment-only shader
edit still takes a naga run. Worth knowing before the commit is planned as
"trivial."

**One thing the rename does NOT carry, and should be decided with it:**

```cpp
static const char* NAMES[MOOD_COUNT] = {
    "open_sunset", "indoor_flat", "indoor_vault", "finite_outdoor"
};
```

`mood_name`'s **string** is `"open_sunset"`, independent of the
identifier. It reaches the console in at least four places
(`[Mood] Applied: …`, `[World] Transition …`, `[Portal] Back-portal
spawned … mood=…`, `[Portal] Forward portal N … mood=…`) — all four appear
in the Pixel 8 logs, so this string is **audience-console visible**.
Renaming the identifier without the string leaves `MOOD_OPEN` printing
`open_sunset`. Renaming both changes log text that WEB_METER_0's captures
quote. Either is fine; silently doing one is not.

---

## RIDER 2 — THE `Lighting` EXTENSION POINT, PRE-SCOPED

`struct Lighting` / `GPULighting` is **848 B** and asserted three ways.
Here is every site a `FOG_SUN` / `FOG_AWAY` extension touches, so SKY_1's
fog commit is scoped before it is written.

**The current shape:**

```cpp
struct alignas(16) GPULighting {
    GPUDirectionalLight sun;      // offset   0, 48 B
    GPUPointLightArray  points;   // offset  48, 272 B
    GPUSpotLightArray   spots;    // offset 320, 528 B
};
static_assert(sizeof(GPULighting) == 848, ...);
static_assert(offsetof(GPULighting, sun)    ==   0, ...);
static_assert(offsetof(GPULighting, points) ==  48, ...);
static_assert(offsetof(GPULighting, spots)  == 320, ...);
```

**Where fog goes, and why the choice matters.** `GPUDirectionalLight` is
48 B with **three trailing pads** (`_pad1`, `_pad2`, `_pad3` after
`ambient`) in both rooms. So there are two shapes:

1. **Inside `sun`** — reclaim `_pad1/_pad2/_pad3` for a `vec3` fog color.
   `sizeof(GPULighting)` stays **848**; all four `static_assert`s stay
   green; `sizeof(GPUDirectionalLight) == 48` stays green. **The
   cheapest possible fog landing.** Fits exactly three floats — a fog
   *density* would not fit and would force shape 2.
2. **A new member after `spots`** — `sizeof` goes 848 → 864, the size
   assert updates, a fourth `offsetof` assert is added at 848, and both
   `entries[…].size = sizeof(GPULighting)` sites follow automatically.

**Every site either shape touches:**

| room | site |
|---|---|
| C++ struct | `GPULighting` (and `GPUDirectionalLight` for shape 1), `state.hpp` |
| C++ witnesses | `sizeof(GPULighting) == 848`; the three `offsetof`s; `sizeof(GPUDirectionalLight) == 48` |
| C++ writer | `upload_lights` (`mood.hpp`) — the one composer; already writes `sun.*` field by field |
| C++ upload | `upload_lighting` (`state.hpp`) — one `WriteBuffer`, size-agnostic |
| C++ buffer | `lightingBuffer_ = makeBuffer("Lighting", sizeof(GPULighting), …)` |
| C++ bind sites | **three**: the layout entry and **two** bind groups, each `entries[5].binding = bind::g0::render_lighting` with `entries[5].size = sizeof(GPULighting)` |
| WGSL struct | `struct Lighting` and `struct DirectionalLight`, `world.wgsl` |
| WGSL readers | `render_lighting.sun.*` — currently 3 sites; fog adds readers in `shade_lit` and the three other fog-mixing functions |
| ledger | **does not move.** `render_lighting` stays `@group(0) @binding(320)`, uniform, one slot. |

**The binding budget is not a constraint here.** The regenerated ledger's
tightest row is room-family **uniform 10 of 12**; no storage row leads the
program anywhere, and this edit adds no binding. 848 B (or 864) sits far
under the 65,536 B `maxUniformBufferBindingSize` granted on both twins.

**Recommendation, non-binding: shape 1 if fog is a color only, shape 2 if
fog carries density.** Q5's resolution decides which — resolution 2
(mood-base + canvas-modulation) needs color only in `Lighting`, since
density stays the canvas's.

---

## SKY_1 PRECONDITION MATRIX

Jean rules; Claude composes SKY_1 from the ruling.

| # | precondition | verdict | evidence |
|---|---|---|---|
| 1 | Sky variants cost nothing per frame | **GO** | §0: there is no sky pass. Sky = `clearColor_` + `config.fog_color`, both CPU-set at mood roll. Three variants cost what one costs, structurally |
| 2 | `78u` is a free salt | **GO** | Q1: 35 `cpu_hash` sites censused, plus the `+1000` gaussian shadow. `78u` unclaimed on every seed domain; band 78–99 clear |
| 3 | The portal palette has a knowable reader set | **GO** | Q2: one derivation (`portal_color_for`), two call sites (`arch_write_active`, `force_spawn_portal_at`), no third. `PortalDestination` already carries `seed`, so the outdoor class can derive its destination's horizon with no new plumbing |
| 4 | The five atmosphere columns are cuttable | **GO** | Q3: all five have exactly one reader, `apply_mood_lighting`. Downstream (`sunDirection_`, `clearColor_`, `GPUDesignConfig`, `GPULighting`) is unaffected and inherits the new sky automatically |
| 5 | The moon lands through `ORB_MOOD_TABLE` | **GO, shape open** | Q4: one call site. But the table is per-*mood* and a moon is per-*variant*; two shapes available, and putting variant data in a mood-indexed table undoes precondition 4's cut |
| 6 | Moon parameters fit `GPUOrbConfig` | **GO** | Q6: 480 B uniform, two free pads (insufficient alone), GROWTH LAW path confirmed, `entries[].size` follows `sizeof` automatically, ledger does not move |
| 7 | The `Lighting` fog extension is pre-scoped | **GO** | Rider 2: every site enumerated. Shape 1 (reclaim `DirectionalLight`'s three trailing pads) keeps `sizeof == 848` and all four witnesses green |
| 8 | The rename is a single identifier change | **CORRECTED** | Rider 1: true in kind, but 23 sites / 9 files, 2 in `world.wgsl` (naga run for a comment-only shader edit), and `mood_name`'s audience-visible `"open_sunset"` string does not follow the identifier |
| 9 | **Fog resolves to one home in `Lighting`** | **NO-GO as scoped** | Q5: fog's live driver is `VisualCanvas` (`FOG_BY_FIELD`, `FOG_SPAN`, one `set_fog` caller), which claims ownership in its header. `apply_mood_lighting` never touches fog despite its comment. Moving fog to `Lighting` **removes the music era's only coupling** — a design decision, and the handoff puts music-era mechanisms out of scope. Three resolutions offered; **this recon does not choose** |
| 10 | **The triad survives the six-destination roster** | **CONDITIONAL — Jean must rule** | Q7: survives untouched if sky variants are *not* moods (`MOOD_COUNT` stays 4), which is what `derive_sky(seed, mood)` and the `MOOD_OPEN` rename both imply. If `MOOD_COUNT` grows to 6, `count = 2`, `fwd_moods[1]`, `pick_portal_mood`'s hardcoded thirds, and **five positional `[MOOD_COUNT]` tables that zero-fill silently** all need the same commit |

### The two rulings SKY_1 is blocked on

Everything else is GO or corrected-and-GO. SKY_1 cannot be composed
without these two:

- **Row 10 — does `MOOD_COUNT` change?** Decides whether SKY_1 is a
  one-function edit or a table-roster campaign.
- **Row 9 — who owns fog?** Decides whether the fog commit exists in
  SKY_1 at all, and if so which of the three resolutions it implements.

### Free, uncontested, and available to any round

- `Dim::FOG_COLOR_R/G/B` (`state.hpp`) — **zero readers**, deletion
  candidate with a trivially satisfied P3 refuter.
- `apply_mood_lighting`'s header comment claims fog the body does not
  have — a present-behavior violation sitting on the exact function SKY_1
  rewrites.
- Homes #2 and #3 hold the same three fog literals with no witness tying
  them; either can drift from the other unnoticed.
