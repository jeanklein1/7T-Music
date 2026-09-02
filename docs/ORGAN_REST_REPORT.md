# ORGAN_REST — REPORT

> **Name provisional (Jean's gate).** Executed on `master`, the standing
> instruction of this session. U1 and U3 are marked **DRAFT** in their commit
> subjects: they move charter prose into the enrollment file's voice, and
> Jean's amendment reserved that wording to his pen the way naming is
> reserved.

**THE HEADLINE, BEFORE THE UNITS.** The campaign's ruling — *the organ stays,
its default inverts* — is right and landed. Its **diagnosis** was mostly
wrong, and the tree said so on every one of the four taxes but one. The
handoff was authored from source alone, with `docs/`, `CLAUDE.md`,
`docs/LAWS.md` and `tools/` unreadable, and three of the four answers lived in
exactly those files. **This is not a failure of the handoff; it is the
recon-first shape working as designed.** What follows is the account.

| the handoff's tax | the tree | what executed |
|---|---|---|
| 1 · `organ_gap` charges a fee for non-enrollment | **false** — the tool's own banner refuses to, and L45 names it a standing witness | the demotion did NOT run; one sentence struck |
| 2 · `audit/ORGAN.md` costs a per-campaign regeneration | **false** — no stamp, pure function of the rows, and named in L33's ritual | **U4 quarantined** |
| 3 · `organ_params.inc` carries long epitaphs | **TRUE** — 779 lines, 324 rows, 411 comment lines | U5, narrower than named |
| 4 · "every range is evidence" reads as an obligation | **misquoted** — the sentence says a missing dial is *merely silent* | U1, rewritten |

---

## THE HEADLINE GATE — STATED FIRST, AS §5 REQUIRES

**PASS. Byte-identical at every checkpoint.**

```
count 324
65628 bytes of organ_manifest() JSON
```

Captured before the first edit and re-captured after U1, U2, U3, U5 and the
merge — **six captures, all identical**, by `cmp`.

**And §5's premise about its own cost was wrong in the campaign's favour.**
The handoff believed this needed the built program, which cannot run here (no
built Dawn, no display). It does not. `organ_manifest()` is `extern "C"
inline` at global scope and its whole dependency set is registry walking:
`block_base` returns `nullptr` before `bind_home` and `read_lane`
short-circuits on it, so the manifest builds deviceless. The witness is a
two-line TU compiled with the shell gate's own flags:

```
clang++ -std=gnu++20 -DGLFW_INCLUDE_NONE \
  -I src -I tools/gates/console_gate/stubs \
  -I third_party/dawn_native_headers/include -o organ_witness organ_witness.cpp
```

~2 seconds a run. **The `-I .../console_gate/stubs` arm and
`-DGLFW_INCLUDE_NONE` are load-bearing; do not drop either.**

### THE TRAP, and it is the most valuable single line in this report

**`organ_param_count()` — the C ABI export — returns `0` deviceless, by
design.** PURSE_0 R-D made the count the readiness gate:

```cpp
EMSCRIPTEN_KEEPALIVE inline int organ_param_count(void) {
    if (!t7::organ::g_home) return 0;
    return (int)t7::organ::kOrganParamCount;
```

A safety gate built on that export would have compared **0 to 0 and passed
over any damage this campaign did**. The whole byte-identity proof would have
been theatre. The witness reads `t7::organ::kOrganParamCount` — the `inline
constexpr` — instead. Anyone building §5's gate into a tool must use the
constant, never the export.

---

## U0 — RECON

Nine readers over every U0 target plus two scouts the handoff could not write.
The quotations that decided a unit:

**`tools/organ_gap.py`, its own banner:**
> `EXIT 0 ALWAYS UNLESS --gate. A member absent from the panel is usually`
> `absent ON PURPOSE and the reason lives in docs/ORGAN.md, so failing a`
> `build over one would assert a judgement this tool cannot make.`

It prints `"A map, not a gate."` on every run. Its one `violations` list is
appended at exactly one site, inside the reader witness. Run at the base
commit: **39 absences, `--gate: PASS`, `exit=0`.**

**`docs/LAWS.md`, closing L45:**
> `tools/organ_gap.py --gate` is the standing witness.

**`docs/LAWS.md`, L33:**
> The five that live there — `MANIFEST.md`, `BINDING_LEDGER.md`,
> `MIRROR_LEDGER.md`, `COMMAND_LEDGER.md`, `ORGAN.md` — each name their
> producer in their own header, and each is byte-reproducible from the tree.

**`docs/ORGAN.md`, opening "What has no dial, and why":**
> An enrollment states a belief and only the reader proves it; the converse
> also has a register. Four facts survived every wave of the disposition
> survey **without earning a row**, and each **names its own owner rather than
> waiting on a campaign**.

**`src/console/organ_params.inc`, the banner tax 4 accuses:**
> EVERY RANGE HERE IS EVIDENCE, NOT TASTE: a wrong range on a dial is worse
> than a missing dial, **because the missing one is silent** and the wrong one
> lies.

**`src/console/organ_registry.hpp`, four lines above where U3 wanted its text:**
> NO RESERVED COLUMNS: a column arrives with the campaign that fills it, which
> is also the only campaign that can say what shape it needs.

**`docs/OPEN.md`'s Ableton seam entry reserves nothing about registry shape**,
so U3 cites nothing.

**No enrollment obligation exists anywhere** — not in `docs/ORGAN.md`, not in
any law. The obligation was a **habit**: a practice of defending absences to a
tool that never asked.

---

## PER UNIT

| unit | status | commit |
|---|---|---|
| U0 | landed (read-only) | — |
| U1 | **landed, DRAFT** | `e4bcec70` |
| U2 | **landed, narrowed** — the demotion did not run | `c3aba8c8` |
| U3 | **landed, DRAFT** — extension only, nothing restated | `9928e996` |
| U4 | **QUARANTINED** | — |
| U5 | **landed, narrower than named** | `965e243b` |
| U6 | landed (report only, corrected after audit) | this file |
| U7 | this file | — |

### U1 — the posture, moved rather than inverted

The handoff's ruling text contains *"a wrong range lies where a missing dial
is merely silent"*. That is a **paraphrase of the banner line it would have
sat under**, and a rule restated in a second place is a rule with two homes
(L46) whose copy drifts. So the ruling text was **not installed**. What landed
is the belief itself — that a field *earns* a row and an absence is a CHOICE —
moved from `docs/ORGAN.md` line 400 into the file a hand actually opens, under
the ranges law, because that is the sentence the habit misread. It quotes
`organ_gap`'s banner rather than stating the posture a third time, and points
at the charter's register rather than copying it. **It states no count**,
because a count beside a generated one is the drift `docs/ORGAN.md`'s own
tally section was written to end.

**The charter was not amended. It already held the belief.**

### U2 — the demotion did not run, and the reason is a law

`organ_gap` is two tools in one file. The **census** half already exits zero
and already runs on request — the posture the demotion was for is already the
state of the tree. The **gate** half fails on exactly one thing, a graduated
design table keeping a surviving runtime reader, and **L45 names that
invocation as its standing witness**. Removing it from the battery would leave
a ratified law citing a witness that no longer stands.

What was actually taxing anything is one clause, and it is struck: the
`mode_threshold` hazard note ended by reporting to the tool and confirming the
tool's verdict was correct. The hazard explanation — a real L46 two-homes
finding — keeps every other line.

### U3 — the extension, not the restatement

The moratorium already exists for columns, four lines above where the handoff
wanted its copy. Only the **extension** landed: definition kinds, block ids,
macro forms and ABI exports, held by the reason the existing note already
gives. Nothing was added to `docs/ORGAN.md` — one paragraph, one place.

### U4 — QUARANTINED, and this is the one unit that did not run

Three findings, each sufficient alone:

1. **`audit/ORGAN.md` is named in L33's rebuild ritual** — one of the five
   deleted and regenerated to prove the room rebuilds byte-identically. A
   release-tag cadence makes it stale between tags, and a stale file fails
   that witness.
2. **The cadence anchor does not exist.** `git tag` returns three tags:
   `attic/full-board`, `native-sunset`, `web-sunset` — all attic markers. This
   repo has never released, and `CLAUDE.md` says it "has never deployed and
   now carries no route to a deploy." *"Regenerate at release tags only"*
   means **never regenerate again**.
3. **The cost being saved is zero, and THIS CAMPAIGN IS THE PROOF.** The book
   carries no provenance stamp; it is a pure function of the enrolled rows.
   Three comment-only commits to `organ_params.inc` left `audit/ORGAN.md`
   **byte-identical** — verified by regenerating after U1, U2 and U5. Across
   history it moved in 63 commits against `organ_params.inc`'s 71: it
   regenerates because rows moved.

**And U2 and U4 are not independent, though the map says all six are.**
`tools/organ_ledger.py` embeds `organ_gap.py`'s stdout tail *verbatim* into
`audit/ORGAN.md`. Had U2's rewording run, it would have moved U4's file.

### U5 — three blocks, and the handoff's own rule excluded three of the five

Moved verbatim to `docs/ORGAN_RETIRED.md`, each subject grep-verified to have
no surviving `ORGAN_PARAM*` line:

| block | lines | named by the handoff? |
|---|---|---|
| `Terrain · Mosaic`, the retired section | 10 | yes |
| `field_occupier_gain` | 8 | yes |
| `mute_signal` | 7 | **no — found by walking the file** |

Left in place and flagged, under *"if a block is ambiguous, leave it in
place"*: `veil_dither`/`veil_strength` is **exactly four lines**, not more than
four, and is not separable — it sits inside one 30-line run with the chain
constraint, the `draw_ring` rename note and the `grain_band` epitaph, three of
which the same handoff protects. `grain_band`'s last four lines justify a
**live** section's name. `lod0_radius` is **three lines**.

**And the file did not get much shorter.** Comment lines **411 → 402** against
324 rows, because U1's posture paragraph costs about what U5's three moves
saved. What changed is not the volume: the table no longer explains three
things that are not there.

---

## U6 — THE TRIM CENSUS

**Report only. Nothing was deleted, commented out, disabled or reordered.**
Row ids are stored-preset keys and block ids are the seam's wire contract;
this is an inventory Jean rules over, and **D is the default bucket**.

- **A** — frozen subject, settled taste rather than a live question
- **B** — boot-time-only reader: the row never answers under the hand in real time
- **C** — `_RO` witness rows: meters, not dials, and **not trim candidates**
- **D** — live and load-bearing, and **everything not clearly A, B or C**

| heading | rows |
|---|---|
| A | **5** |
| B | **46** |
| C | **14** |
| D | **259** |
| total | **324** |

### The census was audited adversarially, and it was corrected

A critic re-derived every A and every B against the tree. **B held 26 for 26
structurally. A did not**, and the failure was in the direction the rules
forbid — over-populating the non-default buckets. Two corrections were applied
before publication:

**1 · Fifteen agent rows moved A → D.** The test that produced them
("unchanged since 2026-04-25") is true of **all seventy** agent rows: the
whole `AGENT_BEHAVIORS` table is byte-identical from `88a94877` to HEAD, the
only intervening edit being ATRIUM_4's appended `aux` column at `0.0f`. A test
that does not separate its subjects from the default bucket does not earn a
heading. `behaviors[6].persistence` was the sharpest instance — `[1]`, `[3]`,
`[5]` and `[9]` are identical in every respect and all got D.

**2 · Twenty ribbon-spawn rows moved to B, and this one is a finding.** The
census gave the Colour rows **A** and the Spawn rows **D**, and one of its
notes read *"a patch spawns several times a second under a rider, so the GEN
edit lands continuously rather than at world birth."* **That is streaming-era
thinking, and the conductor died at ONE_SURFACE-I U2.** `select_ribbon_for_patch`
is reached only through `build_world`, whose two callers are boot and
`rebirth_world`, and `patch_system.hpp` says it outright: *"A finite world
draws from the pool once, at birth, and never returns to it."* All twenty are
**B**.

**The census reproduced, in fresh prose, the exact stale belief TENSE_0 was
chartered to remove.** That is worth more than the bucket it corrected.

The critic also found the four `PANEL.camera.*` notes cite the wrong campaign
— they name ORGAN_3 w2, which left the rows writing a bank nothing read, and
omit ORGAN_3b P0, which introduced the very readers the note cites as
evidence. The heading survives on value; the citation was corrected.

**Countervailing evidence that the census was not arguing for cuts:** it left
**ten rows with no runtime reader at all** in D rather than promoting them
out. That is the discipline the unit asked for.

### The table

| row id | group | form | heading | note |
|---|---|---|---|---|
| `PANEL.camera.look_sens_init` | Interaction · Camera | PARAM | **A** | The mouse-look clamp's anchor — input grammar, not pose — read only when KP_+/KP_- is pressed; its value 0.005f was carried verbatim into PANEL_TABLE at ORGAN_3 w2 and has not moved since. |
| `PANEL.camera.look_sens_range` | Interaction · Camera | PARAM | **A** | The clamp half-width (8.0f) bounding the keypress nudge to init/R … init·R, unchanged since ORGAN_3 w2. |
| `PANEL.camera.look_sens_step` | Interaction · Camera | PARAM | **A** | The multiplicative per-keypress step (1.25f), unchanged since ORGAN_3 w2 moved it into PANEL_TABLE; no frame reads it. |
| `PANEL.camera.scroll_zoom_scale` | Interaction · Camera | PARAM | **A** | Orbit distance per wheel notch (2.0f), read only inside the scroll event; unchanged since ORGAN_3 w2 carried it out of CameraControls. |
| `PANEL.possession.radius` | Interaction · Possession | PARAM | **A** | The Caps-Lock reach, squared at its one read site; POSSESSION_RADIUS = 20.0f predates its ORGAN_4 P3 graduation into PANEL_TABLE and has not moved since. |
| `RIBBON_SPAWN.color_weights[0]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.color_weights[1]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.color_weights[2]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.smooth_palette[0][0]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.smooth_palette[1][0]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.smooth_palette[2][0]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.smooth_palette[3][0]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.smooth_var_b_scale` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.smooth_var_bias` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.smooth_var_g_scale` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.smooth_var_range` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.tinted_base[0]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.tinted_range[0]` | Ribbon · Colour | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.position_jitter` | Ribbon · Spawn | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.spawn_chance` | Ribbon · Spawn | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.wander_chance` | Ribbon · Spawn | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.wander_cruise_base` | Ribbon · Spawn | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.wander_cruise_max` | Ribbon · Spawn | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.wander_cruise_min` | Ribbon · Spawn | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `RIBBON_SPAWN.wander_cruise_sigma` | Ribbon · Spawn | PARAM_GEN | **B** | B — select_ribbon_for_patch is reached only through build_world, whose two callers are boot and rebirth_world |
| `ORBS.base_size` | Sky & Light · Dome | PARAM | **B** | Its only two readers are one-shot kernels — the seed-to-dome init and the palette re-sample — so nothing reads it per frame and the boundary can only reach it by re-seeding the whole sky. |
| `AUTOMATON.alive_height` | Terrain · Automaton draw | PARAM_GEN | **B** | The dial the walk reaches for first, and it still lands only on the next world: the bank field is drawn into cfg at birth and the shader reads auto_config, not the bank. |
| `AUTOMATON.alive_height_spread` | Terrain · Automaton draw | PARAM_GEN | **B** | The ± around alive_height, consumed by the same one-shot draw. |
| `AUTOMATON.density` | Terrain · Automaton draw | PARAM_GEN | **B** | The seed-density centre the world draws from once; no reader runs inside the frame. |
| `AUTOMATON.density_spread` | Terrain · Automaton draw | PARAM_GEN | **B** | The ± around density, consumed by the same one-shot draw at world birth. |
| `AUTOMATON.target` | Terrain · Automaton draw | PARAM_GEN | **B** | The colour target's three lanes, each clamped into cfg by the birth draw. |
| `AUTOMATON.target_spread` | Terrain · Automaton draw | PARAM_GEN | **B** | One ± shared by all three colour lanes, consumed by the same one-shot draw. |
| `AUTOMATON.tick_period` | Terrain · Automaton draw | PARAM_GEN | **B** | The Ableton seam's own beat cue, but the bank field is drawn once into AutomatonState::cfg at birth; the per-frame WGSL reads the drawn cfg copy, not this field. |
| `AUTOMATON.tick_period_spread` | Terrain · Automaton draw | PARAM_GEN | **B** | The ± around tick_period, consumed by the same one-shot draw. |
| `AUTOMATON.transition_fraction` | Terrain · Automaton draw | PARAM_GEN | **B** | The spring's transition share of a tick, clamped into cfg at birth and never re-read from the bank. |
| `AUTOMATON.transition_fraction_spread` | Terrain · Automaton draw | PARAM_GEN | **B** | The ± around transition_fraction, consumed by the same one-shot draw. |
| `AUTOMATON.height_factor_hi` | Terrain · Automaton grain | PARAM_GEN | **B** | Birth-only copy through draw_automaton, the upper half of the same clamp pair. |
| `AUTOMATON.height_factor_lo` | Terrain · Automaton grain | PARAM_GEN | **B** | Birth-only copy through draw_automaton into the clamp the GPU applies per cell. |
| `AUTOMATON.height_factor_mean` | Terrain · Automaton grain | PARAM_GEN | **B** | draw_automaton copies it to as.cfg.height_factor_mean at birth; the Gaussian itself is drawn on the GPU from that copy. |
| `AUTOMATON.height_factor_sigma` | Terrain · Automaton grain | PARAM_GEN | **B** | Birth-only copy through draw_automaton; the shader reads auto_config.height_factor_sigma, not the bank. |
| `AUTOMATON.phase_randomness` | Terrain · Automaton grain | PARAM_GEN | **B** | Carried verbatim into as.cfg by draw_automaton at world birth; no runtime reader touches the bank field again. |
| `AUTOMATON.spring_variance` | Terrain · Automaton grain | PARAM_GEN | **B** | The bank field's one reader is draw_automaton, which copies it into as.cfg at a world's birth; the per-frame WGSL reads the uploaded copy, not this field. |
| `AUTOMATON.tempo_randomness` | Terrain · Automaton grain | PARAM_GEN | **B** | Same birth-only copy path through draw_automaton into the automaton config header. |
| `AUTOMATON.algorithm` | Terrain · Automaton rule | PARAM_GEN | **B** | AUTO_LIVE.algorithm is copied into AutomatonState::cfg once per world birth; the per-frame WGSL reads z.algorithm from that copy, never from the bank. |
| `AUTOMATON.boundary_mode` | Terrain · Automaton rule | PARAM_GEN | **B** | Copied into AutomatonState::cfg at world birth only; the REFLECT static_assert in contracts/automaton_surface.hpp pins AUTO_TABLE, not the AUTO_LIVE bank this row writes. |
| `AUTOMATON.color_mode` | Terrain · Automaton rule | PARAM_GEN | **B** | Copied into AutomatonState::cfg at world birth only; world.wgsl branches on auto_config.color_mode per frame off that copy, so a turn shows at the next world. |
| `AUTOMATON.field_fn` | Terrain · Automaton rule | PARAM_GEN | **B** | Copied into AutomatonState::cfg at world birth only, and the enrollment's own label says it is unread while algorithm is CONWAY. |
| `AUTOMATON.rule_mask` | Terrain · Automaton rule | PARAM_GEN | **B** | Copied into AutomatonState::cfg at world birth only; the CONWAY/0x1808 static_assert in contracts/automaton_surface.hpp pins AUTO_TABLE (the design table), not AUTO_LIVE (the bank this row writes). |
| `WORLD.radius_max` | World · Radius | PARAM_GEN | **B** | Same single reader as radius_min: derive_finite_radius at become_world, clamped to FINITE_RADIUS_MAX, with no per-frame consumer. |
| `WORLD.radius_min` | World · Radius | PARAM_GEN | **B** | Read only by derive_finite_radius, which cartridge.hpp calls once inside become_world — the L10 door both boot and rebirth walk. |
| `WORLD.next_seed` | World · Seed | PARAM_GEN | **B** | Its one runtime reader is rebirth_world, pressed by ORGAN_DOOR_REBIRTH in organ_flush; boot seats the field from world_state_.active_seed and nothing reads it per frame. |
| `CONFIG.checker_music_amount` | Atmosphere · Checker | PARAM_RO | **C** | _RO witness on the driven amount written by set_checker_color_field. |
| `CONFIG.checker_music_variance` | Atmosphere · Checker | PARAM_RO | **C** | _RO witness on the driven variance written by set_checker_color_field. |
| `CONFIG.checker_resultant` | Atmosphere · Checker | PARAM_RO | **C** | _RO witness: the blended colour as it crossed the CPU→GPU seam; organ_set refuses to write it. |
| `CONFIG.fog_color` | Atmosphere · Fog | PARAM_RO | **C** | _RO witness of the three colour lanes set_fog wrote at the same seam; organ_set refuses to write it. |
| `CONFIG.fog_density` | Atmosphere · Fog | PARAM_RO | **C** | _RO witness of what phase_motion_drivers wrote through set_fog; the terrain fragment shader reads config.fog_density per pixel. |
| `CONFIG.aura_enabled` | Pawn · Aura | PARAM_RO | **C** | _RO witness: written by set_aura_enabled from the presence ramp and refused by organ_set. |
| `CONFIG.pawn_aura_height` | Pawn · Aura | PARAM_RO | **C** | _RO witness: the driven extrusion height the terrain VS and the pawn both read, written by set_pawn_aura_height. |
| `CONFIG.fpv_eye_height` | Pawn · Figure (driven) | PARAM_RO | **C** | _RO witness: the possessed figure's eye height, authored CPU-side each frame and read by the FPV camera. |
| `CONFIG.pawn_body_radius` | Pawn · Figure (driven) | PARAM_RO | **C** | _RO witness: the possessed body's boundary inset, restated each frame through a dirty-guarded setter and refused by organ_set. |
| `CONFIG.pawn_tilt_tau` | Pawn · Figure (driven) | PARAM_RO | **C** | _RO witness: the possessed figure's tilt_tau, restated each frame through a dirty-guarded setter and refused by organ_set. |
| `LIGHTING.sun.ambient` | Sky & Light · Sun | PARAM_RO | **C** | Witness of the drawn ambient fill, metered not written. |
| `LIGHTING.sun.color` | Sky & Light · Sun | PARAM_RO | **C** | Witness of the drawn sun colour; the dial that authors it sits under Atmosphere · Sky. |
| `LIGHTING.sun.direction` | Sky & Light · Sun | PARAM_RO | **C** | Witness of the bearing stage_sky's draw produced; organ_set refuses it. |
| `LIGHTING.sun.intensity` | Sky & Light · Sun | PARAM_RO | **C** | Witness of the drawn diffuse strength (centre + jitter), metered not written. |
| `AGENT_ROOM.tier_gains[0].color_r` | Agents · Tier 0 | PARAM_DEF | **D** | Read per frame by the entity VS as the body-colour fallback for a regular figure carrying no per-agent colour, which today is exactly slot 0 — seed_player_body leaves the pawn's colour zero and its tier at WORKER, so this row paints the pawn. |
| `AGENT_ROOM.tier_gains[0].contact_mass` | Agents · Tier 0 | PARAM_DEF | **D** | Read per frame as relative yield authority in the contact resolve; the DEF twin writes TIER_LIVE.t[0].contact_mass. |
| `AGENT_ROOM.tier_gains[0].contact_radius` | Agents · Tier 0 | PARAM_DEF | **D** | Read per frame as the body radius in the contact passes; also read CPU-side by the doorway witness in direction/sky.hpp. |
| `AGENT_ROOM.tier_gains[0].flee_gain_player` | Agents · Tier 0 | PARAM_DEF | **D** | Read per frame as the flee response gain against the point source; the enrolled range 0…0.99 is the CATCHABILITY LAW written into the dial. |
| `AGENT_ROOM.tier_gains[0].persist_gain` | Agents · Tier 0 | PARAM_DEF | **D** | Read per frame as the multiplier on both home_pull and persistence; the DEF twin writes TIER_LIVE.t[0].persist_gain. |
| `AGENT_ROOM.tier_gains[0].personal_radius` | Agents · Tier 0 | PARAM_DEF | **D** | Read per frame as the social shell for flock sense and the flee trigger; all four tiers still carry the single value 30.0 seeded at CONTACT_2. |
| `AGENT_ROOM.tier_gains[0].speed_gain` | Agents · Tier 0 | PARAM_DEF | **D** | Read per frame by every agent arm as the tier's speed-cap multiplier; the DEF twin writes AgentTierBank TIER_LIVE.t[0].speed_gain. |
| `AGENT_ROOM.tier_gains[0].step_gain` | Agents · Tier 0 | PARAM_DEF | **D** | Read per frame wherever a step impulse fires; the DEF twin writes TIER_LIVE.t[0].step_gain. |
| `AGENT_ROOM.tier_gains[1].color_r` | Agents · Tier 1 | PARAM_DEF | **D** | Tier 1 (scout) is the most-drawn tier — weight 3 of 4 in AGENTS_LIVE.tier_weights — and its colour triple is read per frame in the pawn draw. |
| `AGENT_ROOM.tier_gains[1].contact_mass` | Agents · Tier 1 | PARAM_DEF | **D** | Read per frame as the relative yield authority in the same gathers; the design table marks it Jean-tunable. |
| `AGENT_ROOM.tier_gains[1].contact_radius` | Agents · Tier 1 | PARAM_DEF | **D** | Read per frame as the body radius in the bounded pair gathers and the mover clearance; the design table marks it Jean-tunable. |
| `AGENT_ROOM.tier_gains[1].flee_gain_player` | Agents · Tier 1 | PARAM_DEF | **D** | Read per frame as the flee response gain; the tree's CATCHABILITY LAW keeps the enrolled ceiling at 0.99 because a gain ≥ 1 means the agent can never be approached. |
| `AGENT_ROOM.tier_gains[1].persist_gain` | Agents · Tier 1 | PARAM_DEF | **D** | Read per frame as the multiplier on persistence (noise arc) and on home_pull in the tethered arms. |
| `AGENT_ROOM.tier_gains[1].personal_radius` | Agents · Tier 1 | PARAM_DEF | **D** | Read per frame as the social shell — it is the flock's own sense radius in behavior_flock2d and the flee shell in row_agent_flee. |
| `AGENT_ROOM.tier_gains[1].speed_gain` | Agents · Tier 1 | PARAM_DEF | **D** | Read per frame by every behaviour arm's agent_post_step as the tier's cap multiplier. |
| `AGENT_ROOM.tier_gains[1].step_gain` | Agents · Tier 1 | PARAM_DEF | **D** | Read per frame as the multiplier on each arm's step impulse (b.step_size × g.step_gain). |
| `AGENT_ROOM.tier_gains[2].color_r` | Agents · Tier 2 | PARAM_DEF | **D** | Read per vertex every frame by pawn_vs as the tier fallback body colour when a slot carries no per-agent pick; sentinel's authored triple is the deep blue (0.30, 0.40, 0.70). |
| `AGENT_ROOM.tier_gains[2].contact_mass` | Agents · Tier 2 | PARAM_DEF | **D** | Read every frame in field_sum as the scale on an agent emitter's contribution to the summed force. |
| `AGENT_ROOM.tier_gains[2].contact_radius` | Agents · Tier 2 | PARAM_DEF | **D** | Read every frame as the agent's body radius in the sky-push clearance test and as the emitter radius in the field pair sum. |
| `AGENT_ROOM.tier_gains[2].flee_gain_player` | Agents · Tier 2 | PARAM_DEF | **D** | Read every frame by row_agent_flee as the escape gain against the point source; the row's 0.99 ceiling is the tree's CATCHABILITY LAW, since a gain >= 1.0 makes the agent unapproachable. |
| `AGENT_ROOM.tier_gains[2].persist_gain` | Agents · Tier 2 | PARAM_DEF | **D** | Read every frame as the multiplier on behaviour persistence and on the home-tether pull in the arms that have one. |
| `AGENT_ROOM.tier_gains[2].personal_radius` | Agents · Tier 2 | PARAM_DEF | **D** | Read every frame as the social shell: row_agent_flee sums both parties' radii for the flee shell, and the flock arm uses it as its sense radius. |
| `AGENT_ROOM.tier_gains[2].speed_gain` | Agents · Tier 2 | PARAM_DEF | **D** | Read every frame by agent_post_step, which every behaviour arm calls, as the multiplier on the behaviour's speed cap. |
| `AGENT_ROOM.tier_gains[2].step_gain` | Agents · Tier 2 | PARAM_DEF | **D** | Read every frame in each behaviour arm's step-trigger branch as the multiplier on step_size. |
| `AGENT_ROOM.tier_gains[3].color_r` | Agents · Tier 3 | PARAM_DEF | **D** | Read per frame by the entity VS, but only as the fallback for a regular figure whose per-agent colour is all zero — spawned walkers always roll a non-zero AGENT_PALETTE swatch and the one zero-colour body (slot 0) is seeded at tier WORKER, so this LEADER fallback does not fire today. |
| `AGENT_ROOM.tier_gains[3].contact_mass` | Agents · Tier 3 | PARAM_DEF | **D** | Read per frame as the LEADER's relative yield authority in the contact resolve; the DEF twin writes TIER_LIVE.t[3].contact_mass. |
| `AGENT_ROOM.tier_gains[3].contact_radius` | Agents · Tier 3 | PARAM_DEF | **D** | Read per frame as the LEADER's body radius in the contact passes; also read CPU-side by the doorway witness in direction/sky.hpp. |
| `AGENT_ROOM.tier_gains[3].flee_gain_player` | Agents · Tier 3 | PARAM_DEF | **D** | Read per frame as the LEADER's flee response gain against the point source; the enrolled range 0…0.99 is the CATCHABILITY LAW written into the dial. |
| `AGENT_ROOM.tier_gains[3].persist_gain` | Agents · Tier 3 | PARAM_DEF | **D** | Read per frame as the LEADER's multiplier on both home_pull and persistence; the DEF twin writes TIER_LIVE.t[3].persist_gain. |
| `AGENT_ROOM.tier_gains[3].personal_radius` | Agents · Tier 3 | PARAM_DEF | **D** | Read per frame as the LEADER's social shell for flock sense and the flee trigger; still carrying the same 30.0 as every other tier since CONTACT_2. |
| `AGENT_ROOM.tier_gains[3].speed_gain` | Agents · Tier 3 | PARAM_DEF | **D** | Read per frame by every agent arm as the LEADER tier's speed-cap multiplier; the DEF twin writes TIER_LIVE.t[3].speed_gain. |
| `AGENT_ROOM.tier_gains[3].step_gain` | Agents · Tier 3 | PARAM_DEF | **D** | Read per frame wherever a LEADER's step impulse fires; the DEF twin writes TIER_LIVE.t[3].step_gain. |
| `AGENT_ROOM.behaviors[2].drag` | Agents · biased_walk | PARAM_DEF | **D** | Passed every frame from behavior_biased_walk into agent_post_step as the velocity decay exponent. |
| `AGENT_ROOM.behaviors[2].home_pull` | Agents · biased_walk | PARAM_DEF | **D** | behavior_biased_walk never reads b.home_pull — the field is read only on the tether arms (wanderer, home_seeker, slow_patrol, pursuit, flee, levy_flight) — and the row's authored value is 0.0f; see notes. |
| `AGENT_ROOM.behaviors[2].neighbor_radius` | Agents · biased_walk | PARAM_DEF | **D** | Read every frame by behavior_biased_walk's soft-cohesion branch, which is gated on it being greater than zero and samples two other slots for a centroid pull. |
| `AGENT_ROOM.behaviors[2].persistence` | Agents · biased_walk | PARAM_DEF | **D** | Read every frame by behavior_biased_walk as the noise arc around the seed-derived travel direction: 1 holds the heading exactly, 0 is a full random arc. |
| `AGENT_ROOM.behaviors[2].speed_cap` | Agents · biased_walk | PARAM_DEF | **D** | Passed every frame from behavior_biased_walk into agent_post_step, where it is multiplied by the tier's speed_gain to form the velocity clamp. |
| `AGENT_ROOM.behaviors[2].step_rate` | Agents · biased_walk | PARAM_DEF | **D** | Read every frame by behavior_biased_walk's step_trigger, and biased_walk is the only behaviour the standing world draws (AGENTS_TABLE.behavior_weights is 1.0 on lane 2 and 0.0 everywhere else). |
| `AGENT_ROOM.behaviors[2].step_size` | Agents · biased_walk | PARAM_DEF | **D** | Read every frame by behavior_biased_walk as the step impulse (times the tier's step_gain), and the cohesion pull is 20% of that same impulse. |
| `AGENT_ROOM.behaviors[7].drag` | Agents · flee | PARAM_DEF | **D** | Read per frame by the flee arm of the agent kernel, on the same behavior_id-7 branch no current spawn weight selects. |
| `AGENT_ROOM.behaviors[7].home_pull` | Agents · flee | PARAM_DEF | **D** | Read per frame by the flee arm of the agent kernel, on the same behavior_id-7 branch no current spawn weight selects. |
| `AGENT_ROOM.behaviors[7].neighbor_radius` | Agents · flee | PARAM_DEF | **D** | Read per frame by the flee arm of the agent kernel, on the same behavior_id-7 branch no current spawn weight selects. |
| `AGENT_ROOM.behaviors[7].persistence` | Agents · flee | PARAM_DEF | **D** | Read per frame by the flee arm of the agent kernel, on the same behavior_id-7 branch no current spawn weight selects. |
| `AGENT_ROOM.behaviors[7].speed_cap` | Agents · flee | PARAM_DEF | **D** | Read per frame by the flee arm of the agent kernel, on the same behavior_id-7 branch no current spawn weight selects. |
| `AGENT_ROOM.behaviors[7].step_rate` | Agents · flee | PARAM_DEF | **D** | Read per frame by the flee arm of the agent kernel, which only executes for a slot whose behavior_id is 7 — and AGENTS_LIVE.behavior_weights currently spawns only biased_walk. |
| `AGENT_ROOM.behaviors[7].step_size` | Agents · flee | PARAM_DEF | **D** | Read per frame by the flee arm of the agent kernel, on the same behavior_id-7 branch no current spawn weight selects. |
| `AGENT_ROOM.behaviors[8].drag` | Agents · flock2d | PARAM_DEF | **D** | Read per frame by the arm's closing agent_post_step. |
| `AGENT_ROOM.behaviors[8].home_pull` | Agents · flock2d | PARAM_DEF | **D** | behavior_flock2d contains no tether term, so this column is unread on the flock arm; the design table seats it at 0. |
| `AGENT_ROOM.behaviors[8].neighbor_radius` | Agents · flock2d | PARAM_DEF | **D** | The flock's sense radius is g.personal_radius since CONTACT_2 (whose seed 30 was transcribed from this column), and behavior_flock2d never reads b.neighbor_radius — the design table still carries 30.0 here. |
| `AGENT_ROOM.behaviors[8].persistence` | Agents · flock2d | PARAM_DEF | **D** | Read per frame as the noise-arc term (1 − persistence·persist_gain)·0.78 before alignment. |
| `AGENT_ROOM.behaviors[8].speed_cap` | Agents · flock2d | PARAM_DEF | **D** | Read per frame by the arm's closing agent_post_step, scaled by the tier's speed_gain. |
| `AGENT_ROOM.behaviors[8].step_rate` | Agents · flock2d | PARAM_DEF | **D** | Read per frame by behavior_flock2d's step_trigger — flock decisions are beat-gated — for any agent carrying behaviour 8. |
| `AGENT_ROOM.behaviors[8].step_size` | Agents · flock2d | PARAM_DEF | **D** | Read per frame twice in the arm: the noise impulse (×0.15) and the cohesion pull (×0.5). |
| `AGENT_ROOM.behaviors[4].drag` | Agents · home_seeker | PARAM_DEF | **D** | Read every frame through agent_post_step, which behavior_home_seeker returns through. |
| `AGENT_ROOM.behaviors[4].home_pull` | Agents · home_seeker | PARAM_DEF | **D** | The dominant spring that defines this behaviour, read every frame in behavior_home_seeker. |
| `AGENT_ROOM.behaviors[4].neighbor_radius` | Agents · home_seeker | PARAM_DEF | **D** | behavior_home_seeker does not read neighbor_radius (pursuit, flee, biased_walk and flock2d do); AGENT_BEHAVIORS authors row 4's neighbor_radius at 0.0. |
| `AGENT_ROOM.behaviors[4].persistence` | Agents · home_seeker | PARAM_DEF | **D** | behavior_home_seeker does not read persistence (only biased_walk and flock2d do); AGENT_BEHAVIORS authors row 4's persistence at 0.0. |
| `AGENT_ROOM.behaviors[4].speed_cap` | Agents · home_seeker | PARAM_DEF | **D** | Read every frame through agent_post_step, scaled by the tier's speed_gain. |
| `AGENT_ROOM.behaviors[4].step_rate` | Agents · home_seeker | PARAM_DEF | **D** | Read every frame by the agent kernel's step trigger for slot 4; an edit is re-spoken at the frame boundary via organ_boundary.inc → upload_agent_registries_to_gpu. |
| `AGENT_ROOM.behaviors[4].step_size` | Agents · home_seeker | PARAM_DEF | **D** | The noise impulse magnitude, read every frame in behavior_home_seeker when the step trigger fires. |
| `AGENT_ROOM.behaviors[9].drag` | Agents · levy_flight | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[9].home_pull` | Agents · levy_flight | PARAM_DEF | **D** | behavior_levy_flight does not read b.home_pull — there is no tether on this arm — and the enrolled value is 0.0. |
| `AGENT_ROOM.behaviors[9].neighbor_radius` | Agents · levy_flight | PARAM_DEF | **D** | behavior_levy_flight does not read b.neighbor_radius — the arm samples no neighbours — and the enrolled value is 0.0. |
| `AGENT_ROOM.behaviors[9].persistence` | Agents · levy_flight | PARAM_DEF | **D** | behavior_levy_flight does not read b.persistence — the arm touches only step_rate, step_size, drag and speed_cap — and the enrolled value is 0.0. |
| `AGENT_ROOM.behaviors[9].speed_cap` | Agents · levy_flight | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[9].step_rate` | Agents · levy_flight | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[9].step_size` | Agents · levy_flight | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[0].drag` | Agents · player_controlled | PARAM_DEF | **D** | Row 0 column with no shader consumer; the player arm owns its own integration and never calls agent_post_step with b.drag. |
| `AGENT_ROOM.behaviors[0].home_pull` | Agents · player_controlled | PARAM_DEF | **D** | Row 0 column with no shader consumer; there is no tether term on the possessed arm. |
| `AGENT_ROOM.behaviors[0].neighbor_radius` | Agents · player_controlled | PARAM_DEF | **D** | Row 0 column with no shader consumer; the possessed body's neighbour sensing is the contact gather's, keyed off tier_gains, not this field. |
| `AGENT_ROOM.behaviors[0].persistence` | Agents · player_controlled | PARAM_DEF | **D** | Row 0 column with no shader consumer; a possessed body's motion comes from the input path, not from the behaviour row. |
| `AGENT_ROOM.behaviors[0].speed_cap` | Agents · player_controlled | PARAM_DEF | **D** | Row 0 column with no shader consumer; the pawn's speed is PAWN_SPEED in the WGSL room, not this field. |
| `AGENT_ROOM.behaviors[0].step_rate` | Agents · player_controlled | PARAM_DEF | **D** | No kernel reads agent_room.behaviors[0]: the possessed slot dispatches to behavior_player_controlled, which takes input, and the general kernel's `case 0u` is a no-op; the row's value is authored to the GPU array and never consumed. |
| `AGENT_ROOM.behaviors[0].step_size` | Agents · player_controlled | PARAM_DEF | **D** | Same as the rest of row 0 — authored into the GPU behaviour array by upload_agent_registries_to_gpu and read by no kernel; the design row is all zeros. |
| `AGENT_ROOM.behaviors[6].drag` | Agents · pursuit | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[6].home_pull` | Agents · pursuit | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[6].neighbor_radius` | Agents · pursuit | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[6].persistence` | Agents · pursuit | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[6].speed_cap` | Agents · pursuit | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[6].step_rate` | Agents · pursuit | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[6].step_size` | Agents · pursuit | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[1].drag` | Agents · random_walk | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[1].home_pull` | Agents · random_walk | PARAM_DEF | **D** | behavior_random_walk does not read b.home_pull — there is no tether on this arm — and the enrolled value is 0.0. |
| `AGENT_ROOM.behaviors[1].neighbor_radius` | Agents · random_walk | PARAM_DEF | **D** | behavior_random_walk does not read b.neighbor_radius — the arm samples no neighbours — and the enrolled value is 0.0. |
| `AGENT_ROOM.behaviors[1].persistence` | Agents · random_walk | PARAM_DEF | **D** | behavior_random_walk does not read b.persistence — the arm touches only step_rate, step_size, drag and speed_cap — and the enrolled value is 0.0. |
| `AGENT_ROOM.behaviors[1].speed_cap` | Agents · random_walk | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[1].step_rate` | Agents · random_walk | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[1].step_size` | Agents · random_walk | PARAM_DEF | **D** | D — the frozen-since test that produced this A takes all 70 agent rows; D is the default and doubt resolves to D |
| `AGENT_ROOM.behaviors[5].drag` | Agents · slow_patrol | PARAM_DEF | **D** | Read per frame by behavior_slow_patrol, passed into agent_post_step. |
| `AGENT_ROOM.behaviors[5].home_pull` | Agents · slow_patrol | PARAM_DEF | **D** | Read per frame by behavior_slow_patrol as the spring constant steering toward the waypoint. |
| `AGENT_ROOM.behaviors[5].neighbor_radius` | Agents · slow_patrol | PARAM_DEF | **D** | behavior_slow_patrol does not read b.neighbor_radius — the field is consumed only on arms 2, 6, 7 and 8 — so this seat has no runtime reader. |
| `AGENT_ROOM.behaviors[5].persistence` | Agents · slow_patrol | PARAM_DEF | **D** | behavior_slow_patrol does not read b.persistence — the field is consumed only on arms 2, 6, 7 and 8 — so this seat has no runtime reader. |
| `AGENT_ROOM.behaviors[5].speed_cap` | Agents · slow_patrol | PARAM_DEF | **D** | Read per frame by behavior_slow_patrol, passed into agent_post_step. |
| `AGENT_ROOM.behaviors[5].step_rate` | Agents · slow_patrol | PARAM_DEF | **D** | Read per frame by behavior_slow_patrol to advance the waypoint index off t_beats. |
| `AGENT_ROOM.behaviors[5].step_size` | Agents · slow_patrol | PARAM_DEF | **D** | Read per frame by behavior_slow_patrol as the waypoint radius, scaled by tier step_gain. |
| `AGENT_ROOM.behaviors[3].drag` | Agents · wanderer | PARAM_DEF | **D** | Read per frame by the wanderer arm as the velocity decay handed to agent_post_step. |
| `AGENT_ROOM.behaviors[3].home_pull` | Agents · wanderer | PARAM_DEF | **D** | Read per frame by the wanderer arm as the soft home tether's spring constant, applied outside a 0.5-unit deadband. |
| `AGENT_ROOM.behaviors[3].neighbor_radius` | Agents · wanderer | PARAM_DEF | **D** | Transported to the GPU with the rest of the row, but no shader arm reads behaviors[3].neighbor_radius and its authored value is 0. |
| `AGENT_ROOM.behaviors[3].persistence` | Agents · wanderer | PARAM_DEF | **D** | Transported to the GPU by upload_agent_registries_to_gpu with the rest of the row, but no shader arm reads behaviors[3].persistence — the wanderer is documented as having no persistent direction and its authored value is 0. |
| `AGENT_ROOM.behaviors[3].speed_cap` | Agents · wanderer | PARAM_DEF | **D** | Read per frame by the wanderer arm as the velocity ceiling handed to agent_post_step. |
| `AGENT_ROOM.behaviors[3].step_rate` | Agents · wanderer | PARAM_DEF | **D** | Read per frame by the wanderer arm as the step trigger rate; the DEF twin writes AgentBehaviorBank BEHAVIOR_LIVE.b[3].step_rate. |
| `AGENT_ROOM.behaviors[3].step_size` | Agents · wanderer | PARAM_DEF | **D** | Read per frame by the wanderer arm as the graze step's magnitude in world units. |
| `DRIVERS.checker.gain` | Atmosphere · Checker | PARAM | **D** | The coupling's own throttle at the checker seam — 1 is the coupling verbatim, 0 is the rest — read every frame. |
| `DRIVERS.checker.rest_amount` | Atmosphere · Checker | PARAM | **D** | The seam's rest music-amount; terrain_looks calls amount 0 law — it returns each cell to its seed colour — and it is read every frame. |
| `DRIVERS.checker.rest_resultant` | Atmosphere · Checker | PARAM | **D** | The pc-colour seam's rest colour, blended per lane against the driven mean every frame at out = rest + gain·(driven − rest). |
| `DRIVERS.checker.rest_variance` | Atmosphere · Checker | PARAM | **D** | The seam's rest variance, read every frame on both arms of the seam (bound and headless). |
| `CANVAS.checker_attack` | Atmosphere · Checker cadence | PARAM_NS | **D** | Read per frame in VisualCanvas::tick as the LINEAR rise span for the resultant colour, amount and variance envelopes. |
| `CANVAS.checker_read_span` | Atmosphere · Checker cadence | PARAM_NS | **D** | VisualCanvas::tick runs per frame after analysis and reads this every tick to place the next sample-and-hold crossing on the absolute beat grid, including the backward-jump re-anchor. |
| `CANVAS.checker_release` | Atmosphere · Checker cadence | PARAM_NS | **D** | Read per frame in VisualCanvas::tick as the LINEAR fall span whenever the amount or variance goal is zero. |
| `DRIVERS.fog.gain` | Atmosphere · Fog | PARAM | **D** | Blended every frame at the driver seam — out = sky_state_ fog rest + gain × the canvas deviation — and handed straight to set_fog. |
| `CANVAS.fog_span` | Atmosphere · Fog envelope | PARAM_NS | **D** | Read twice on every coupling tick as the trajectory_release span for both fog pipes (density and colour), so the value is consumed in the running frame loop. |
| `CONFIG.draw_ring` | Atmosphere · Ring & grain | PARAM | **D** | THE DRAW AUTHORITY, read per frame by the terrain fragment cull, the pawn and floater vertex culls, and on the CPU by the spawn engine's draw-set gate. |
| `ATMOS.ambient` | Atmosphere · Sky | PARAM | **D** | draw_atmosphere composes it into sun_ambient; stage_sky writes SkyState::sun_ambient and raises lights_dirty. |
| `ATMOS.ambient_spread` | Atmosphere · Sky | PARAM | **D** | Same reader through atmos_jitter; ATMOS_TABLE pins it at 0 under the POINT-ROW static_assert. |
| `ATMOS.clear_color` | Atmosphere · Sky | PARAM | **D** | Drawn by draw_atmosphere and written straight to the clear-colour channel by stage_sky at birth and at every atmos-dirty boundary. |
| `ATMOS.clear_color_spread` | Atmosphere · Sky | PARAM | **D** | Same reader through atmos_draw_color; ATMOS_TABLE pins it at 0 under the POINT-ROW static_assert. |
| `ATMOS.fog_color` | Atmosphere · Sky | PARAM | **D** | Drawn by draw_atmosphere into SkyState::fog_rest_color, the rest the per-frame seam composes the canvas's tint deviation over. |
| `ATMOS.fog_color_spread` | Atmosphere · Sky | PARAM | **D** | Same reader through atmos_draw_color; ATMOS_TABLE pins it at 0 under the POINT-ROW static_assert. |
| `ATMOS.fog_density` | Atmosphere · Sky | PARAM | **D** | Drawn by draw_atmosphere into SkyState::fog_rest_density, which the per-frame driver seam in phase_motion_drivers composes the canvas deviation over. |
| `ATMOS.fog_density_spread` | Atmosphere · Sky | PARAM | **D** | Same reader through atmos_jitter; ATMOS_TABLE pins it at 0 under the POINT-ROW static_assert. |
| `ATMOS.intensity` | Atmosphere · Sky | PARAM | **D** | draw_atmosphere composes it into AtmosphereInstance::sun_intensity, which stage_sky writes to SkyState on every birth and every atmos-dirty boundary. |
| `ATMOS.intensity_spread` | Atmosphere · Sky | PARAM | **D** | Same reader through atmos_jitter; ATMOS_TABLE pins it at 0 under the POINT-ROW static_assert. |
| `ATMOS.sun_color` | Atmosphere · Sky | PARAM | **D** | Read by draw_atmosphere inside stage_sky, which re-runs at the frame boundary on every atmos-dirty write, so a drag re-draws this world's sun colour under the same seed. |
| `ATMOS.sun_color_spread` | Atmosphere · Sky | PARAM | **D** | Same reader; ATMOS_TABLE pins this spread at 0 under the POINT-ROW static_assert, so the shipped sky draws the centre exactly. |
| `CONFIG.camera_chase_ff` | Camera · Chase | PARAM | **D** | Live, read every frame by update_camera_vp on the RIBBON host to add back the aim ease's v·tau trail. |
| `CONFIG.camera_push_gain` | Camera · Presence | PARAM | **D** | The shove's acceleration at the shell centre, read every frame by the floater force path in world.wgsl (cubes only, off in free-fly). |
| `CONFIG.camera_push_radius` | Camera · Presence | PARAM | **D** | The shell radius, read every frame in the same force path; 0 shuts the term off. |
| `CONFIG.fpv_mode` | Debug · | PARAM | **D** | Read per frame through fpv_mode_active() in behavior_player_controlled and the camera arm, and key-shared — input.hpp's toggle writes it too. |
| `CONFIG.freeze_sphere` | Debug · | PARAM | **D** | Read per frame through sphere_frozen() inside update_sphere and update_cube. |
| `CONFIG.mute_dynamics_0d` | Debug · | PARAM | **D** | Read per frame through dynamics_0d_active() at four kernel entry points — update_player_agent, update_other_agents, update_sphere and update_cube — each of which early-returns when it is set. |
| `PANEL.beacon.lift` | Interaction · Beacon | PARAM | **D** | Read every frame and added to the point's ground height before the authored-field row is uploaded. |
| `PANEL.beacon.r` | Interaction · Beacon | PARAM | **D** | Read every frame from PANEL_LIVE into the authored-field row the driver phase uploads. |
| `PANEL.beacon.r0` | Interaction · Beacon | PARAM | **D** | Read every frame from PANEL_LIVE into the authored-field row the driver phase uploads. |
| `PANEL.beacon.s` | Interaction · Beacon | PARAM | **D** | Read every frame and clamped at the writer against the live config.field_k before it reaches the authored-field row. |
| `CONFIG.point_fly_speed` | Interaction · Camera | PARAM | **D** | Read every frame by the free-fly movement branch in world.wgsl (0 falls back to PAWN_SPEED); boot pins it once from CameraControls::MOVE_SPEED. |
| `CONFIG.cube_plasticity` | Interaction · Cubes | PARAM | **D** | Read per frame in update_cube as the global λ master multiplying each tier's own plasticity character. |
| `CONFIG.floater_coordination` | Interaction · Cubes | PARAM | **D** | Read per frame in two rooms — update_cube's floater step on the GPU and the CPU-side read in cartridge.hpp — and has one author since ORGAN_9 retired the cycler. |
| `CONFIG.field_authored_gain` | Interaction · Field | PARAM | **D** | Read every frame in field_sum's authored-emitter loop as the mute over the authored table, which the beacon publishes per frame through upload_field_authored. |
| `CONFIG.field_fmax` | Interaction · Field | PARAM | **D** | Read every frame in field_sum as the single magnitude clamp on the summed force, applied before the subscriber-class gain. |
| `CONFIG.field_gain_agent` | Interaction · Field | PARAM | **D** | Read every frame in field_sum as the default post-clamp gain, which is the agent class' lane. |
| `CONFIG.field_gain_cube` | Interaction · Field | PARAM | **D** | Read every frame in field_sum as the post-clamp gain for subscriber indices 40 and above (the cube class). |
| `CONFIG.field_gain_sphere` | Interaction · Field | PARAM | **D** | Read every frame in field_sum as the post-clamp gain for subscriber indices 32..39 (the sphere class). |
| `CONFIG.field_k` | Interaction · Field | PARAM | **D** | Read every frame as the quadratic shell-depth acceleration in field_pair_slack, and the beacon writer clamps its own strength against the live value rather than the constexpr. |
| `CONFIG.field_slack` | Interaction · Field | PARAM | **D** | Read every frame as the shell factor over summed radii in field_pair_slack, and again in orb_dynamics where the flock separation shell is divided by 2 * field_slack. |
| `CONFIG.pawn_speed` | Interaction · Pawn | PARAM | **D** | Read per frame in behavior_player_controlled as select(PAWN_SPEED, config.pawn_speed, config.pawn_speed > 0.0) — zero means no opinion. |
| `CONFIG.point_bubble_radius` | Interaction · Point | PARAM | **D** | Read per frame as the radius of the point-source flee profile and again as the shell ring the terrain shader draws. |
| `CONFIG.draw_mask` | Measure · | PARAM | **D** | Read at bundle recording in render_passes.hpp:encode_main_opaque; set_draw_mask raises bundlesDirty_ so the turn re-records, and the enrolment prose calls it a measurement instrument rather than a taste dial. |
| `CONFIG.shadow_mask` | Measure · | PARAM | **D** | Read at the shadow encoder in render_passes.hpp on the same re-record rule; rests open at 0x3 and a cleared bit is a measurement in progress. |
| `CONFIG.shadow_pcf_taps` | Measure · | PARAM | **D** | Read per fragment in world.wgsl's PCF branch; the enrolment prose calls it a measurement AND a taste gate — whether the narrower penumbra is the piece. |
| `DRIVERS.aura.attack` | Pawn · Aura | PARAM | **D** | Per-frame rate term in tick_pawn_couplings' exponential ramp when the target is above the present value. |
| `DRIVERS.aura.height_gain` | Pawn · Aura | PARAM | **D** | Multiplied into the effective aura height every frame in tick_pawn_couplings before set_pawn_aura_height. |
| `DRIVERS.aura.intent` | Pawn · Aura | PARAM | **D** | Read every frame by tick_pawn_couplings as the presence ramp's target, and toggled by the aura key in the same file. |
| `DRIVERS.aura.release` | Pawn · Aura | PARAM | **D** | Per-frame rate term in the same ramp on the falling side. |
| `PAWN.attack_damping` | Pawn · Aura profile | PARAM | **D** | Copied verbatim into GPUPawnAuraConfig as the spring's damping term; same aura_cfg_dirty gate on the upload. |
| `PAWN.attack_stiffness` | Pawn · Aura profile | PARAM | **D** | Copied verbatim into GPUPawnAuraConfig for the aura compute kernel's spring; same aura_cfg_dirty gate on the upload. |
| `PAWN.delta_magnitude` | Pawn · Aura profile | PARAM | **D** | Uploaded every dirty frame; the aura kernel reads it only on the RANDOM delta arm (delta_mode 1), and the authored mode is CONVERGENT. |
| `PAWN.delta_mode` | Pawn · Aura profile | PARAM | **D** | Selects the aura kernel's convergent (0) or random (1) delta arm; uploaded behind the same aura_cfg_dirty gate. |
| `PAWN.height_scale` | Pawn · Aura profile | PARAM | **D** | The one PAWN row with an ungated per-frame reader: tick_pawn_couplings multiplies it by the driver height gain and presence into set_pawn_aura_height, and dispatch_pawn_aura also gates the kernel's height write on it. |
| `PAWN.influence_radius` | Pawn · Aura profile | PARAM | **D** | Scaled by aura presence into GPUPawnAuraConfig; the upload sits behind ps.aura_cfg_dirty, which no organ write raises, so an edit lands at the next presence ramp, aura toggle or teardown. |
| `PAWN.release_rate` | Pawn · Aura profile | PARAM | **D** | Uploaded as-is while presence > 0.01 and replaced by 999.0 below that (the snap-clear); same aura_cfg_dirty gate. |
| `PAWN.tint_r` | Pawn · Aura profile | PARAM | **D** | The VEC3 row over tint_r/g/b, copied lane-for-lane into GPUPawnAuraConfig; its only consumer is color_blend in the aura kernel, gated by tint_strength. |
| `PAWN.tint_strength` | Pawn · Aura profile | PARAM | **D** | min(tint_strength × presence, 1) into GPUPawnAuraConfig; the tree records it resting at 0, which silences the terrain tint outright. |
| `AGENTS.count` | Population · Agents | PARAM_GEN | **D** | Enrolled GEN, but respawn_evicted_agents — spine row R4 — reads AGENTS_LIVE.count every frame as well as spawn_population at birth. |
| `AGENTS.home_seeding_radius` | Population · Agents | PARAM_GEN | **D** | Read on every respawn inside populate_agent_slot_ when the home offset disc is sampled. |
| `AGENTS.spawn_center_forward` | Population · Agents | PARAM_GEN | **D** | Read on every respawn inside populate_agent_slot_ to walk the annulus centre along the arrival gaze. |
| `AGENTS.spawn_inner_radius` | Population · Agents | PARAM_GEN | **D** | Reached per frame through populate_agent_slot_ from respawn_evicted_agents, so an edit lands on the next agent to die rather than only at rebirth. |
| `AGENTS.spawn_radius` | Population · Agents | PARAM_GEN | **D** | Same per-frame path: populate_agent_slot_ draws the annulus outer bound on every respawn. |
| `DRIVERS.ribbon.gain` | Ribbon · Drivers | PARAM | **D** | Read every frame as the one multiplier over all four ribbon pipes' deviation from rest. |
| `DRIVERS.ribbon.rest_amp_lat` | Ribbon · Drivers | PARAM | **D** | Read every frame as the lateral-amplitude seam's fallback in out = rest + gain·(driven − rest). |
| `DRIVERS.ribbon.rest_amp_vert` | Ribbon · Drivers | PARAM | **D** | Read every frame as the vertical-amplitude seam's fallback in the same composition. |
| `DRIVERS.ribbon.rest_tint_mix` | Ribbon · Drivers | PARAM | **D** | Read every frame as the tint-mix seam's fallback. |
| `DRIVERS.ribbon.rest_tint_stim` | Ribbon · Drivers | PARAM | **D** | Read every frame, per channel, as the tint-stimulus seam's fallback when no driver is bound. |
| `CONFIG.ribbon_alt_smooth_dist` | Ribbon · Head | PARAM | **D** | Live, the per-frame distance constant in ribbon_head's altitude-target smoothing alpha. |
| `CONFIG.ribbon_alt_stiff` | Ribbon · Head | PARAM | **D** | Live, the spring constant integrated every frame in ribbon_head, and the damping term is derived from it in the same expression. |
| `CONFIG.ribbon_climb_rate` | Ribbon · Head | PARAM | **D** | Live, clamps the head's vertical velocity every frame in ribbon_head. |
| `CONFIG.ribbon_floor_margin` | Ribbon · Head | PARAM | **D** | Live, added to the sampled ground every frame to form floor_y in ribbon_head. |
| `CONFIG.ribbon_hands_tau` | Ribbon · Head | PARAM | **D** | Live, the per-frame ease constant on the hands' command in ribbon_head. |
| `CONFIG.ribbon_max_speed` | Ribbon · Head | PARAM | **D** | Live, read per frame in ribbon_head (speed = throttle_eased × ribbon_max_speed) and again in the camera chase block. |
| `CONFIG.ribbon_mount_setback` | Ribbon · Head | PARAM | **D** | Live, placed per frame in the mount's motor transform in world.wgsl. |
| `CONFIG.ribbon_r_min` | Ribbon · Head | PARAM | **D** | Live, the divisor in ribbon_head's per-frame yaw_avail = min(yaw_rate, speed / r_min). |
| `CONFIG.ribbon_yaw_rate` | Ribbon · Head | PARAM | **D** | Live, read every frame by the ribbon_head kernel as one of the two terms of the steering law. |
| `RIBBON.reference_bpm` | Ribbon · Head | PARAM | **D** | Live and CPU-read: ribbon_frame_tick divides 60 by it every frame to scale the phase clock against the tempo follower. |
| `RIBBON.board_seconds` | Ribbon · Mount | PARAM | **D** | Read on the CPU in the frame spine's mount block, dividing dt to advance the boarding ease on every frame a mount is live. |
| `RIBBON.land_seconds` | Ribbon · Mount | PARAM | **D** | Read on the CPU in the frame spine's mount block, dividing dt to advance the landing ease on every frame a dismount is live. |
| `CONFIG.ribbon_clear_body` | Ribbon · Sky Rule | PARAM | **D** | Read per frame as the body pass's sky_push berth — deliberately the smaller of the two clearances. |
| `CONFIG.ribbon_clear_head` | Ribbon · Sky Rule | PARAM | **D** | Read per frame as the head's sky_push berth at the probe. |
| `CONFIG.ribbon_lookahead` | Ribbon · Sky Rule | PARAM | **D** | Read per frame by the head kernel to place the sky probe ahead of the nose. |
| `CANVAS.swell_attack` | Ribbon · Swell | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the rising half of the swell's two-sided envelope. |
| `CANVAS.swell_ceiling` | Ribbon · Swell | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the swell envelope's goal multiplier over idle. |
| `CANVAS.swell_ramp` | Ribbon · Swell | PARAM_NS | **D** | Read every frame in VisualCanvas::tick to scale hold_beats_ into the swell's ramp fraction. |
| `CANVAS.swell_release` | Ribbon · Swell | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the falling half of the swell's two-sided envelope. |
| `CANVAS.pitch_vec_origin` | Ribbon · Tint | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the seating angle the twelve hue vectors are stepped from. |
| `CANVAS.tint_chroma` | Ribbon · Tint | PARAM_NS | **D** | Read every frame in VisualCanvas::tick, scaling the TINT_D1/TINT_D2 chroma axes. |
| `CANVAS.tint_hue_span` | Ribbon · Tint | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the glide span of the hue's own trajectory segments. |
| `CANVAS.tint_luma` | Ribbon · Tint | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the tint's value term before the chroma axes are added. |
| `CANVAS.tint_mix_attack` | Ribbon · Tint | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the rising half of the tint-mix envelope. |
| `CANVAS.tint_mix_max` | Ribbon · Tint | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the mix envelope's goal while the room is sounding. |
| `CANVAS.tint_mix_release` | Ribbon · Tint | PARAM_NS | **D** | Read every frame in VisualCanvas::tick as the falling half of the tint-mix envelope. |
| `CONFIG.ribbon_roam_radius` | Ribbon · Wander | PARAM | **D** | Read every frame in ribbon_head as the radius of the anchor disc that new wander targets are drawn on. |
| `CONFIG.ribbon_wander_arrive` | Ribbon · Wander | PARAM | **D** | Read every frame in ribbon_head as the distance at which a wander target counts as reached and the next is drawn. |
| `CONFIG.ribbon_wander_soft` | Ribbon · Wander | PARAM | **D** | Read every frame in the ribbon head kernel as the heading-error scale that maps error to yaw command. |
| `CONFIG.ribbon_wander_yaw_max` | Ribbon · Wander | PARAM | **D** | Read every frame in ribbon_head as the brain's share of the hands' yaw cap. |
| `ORBS.dome_radius` | Sky & Light · Dome | PARAM | **D** | Read every frame by the sky's rule kernel and again at seed, and the boundary lands it as a targeted 4-byte partial with no re-seed. |
| `ORB_BANK.flock_align_radius` | Sky & Light · Flocking rule | PARAM | **D** | Squared into ali_r2 every frame as the alignment neighbourhood test under rule FLOCKING. |
| `ORB_BANK.flock_align_weight` | Sky & Light · Flocking rule | PARAM | **D** | Scales the alignment force into the per-frame velocity integration under rule FLOCKING. |
| `ORB_BANK.flock_coh_radius` | Sky & Light · Flocking rule | PARAM | **D** | Squared into coh_r2 every frame as the cohesion neighbourhood test under rule FLOCKING. |
| `ORB_BANK.flock_coh_weight` | Sky & Light · Flocking rule | PARAM | **D** | Scales the cohesion force into the per-frame velocity integration under rule FLOCKING. |
| `ORB_BANK.flock_max_speed` | Sky & Light · Flocking rule | PARAM | **D** | Multiplied by speed_mult into eff_max, the per-frame velocity clamp under rule FLOCKING. |
| `ORB_BANK.flock_sep_radius` | Sky & Light · Flocking rule | PARAM | **D** | Divided by 2·config.field_slack every frame to give the separation pair radius under rule FLOCKING. |
| `ORB_BANK.flock_sep_weight` | Sky & Light · Flocking rule | PARAM | **D** | Scales the separation force into the per-frame velocity integration under rule FLOCKING. |
| `ORBS.noise_floor` | Sky & Light · Motion — all rules | PARAM | **D** | Rides GPUOrbConfig.noise_amp and is read per frame by the brownian arm of orb_dynamics; the boundary lands it as a targeted partial (upload_orb_noise), not a re-seed. |
| `ORBS.speed_mult` | Sky & Light · Motion — all rules | PARAM | **D** | The master motion strength: orb_dynamics multiplies brownian's noise injection, orbital's angular speed and the flock speed ceiling by it every frame, and the boundary sends it as a targeted 4-byte partial with no re-seed. |
| `ORB_BANK.rotation_axis` | Sky & Light · Motion — all rules | PARAM | **D** | The three components are read per frame in the same orb_dynamics rotation branch, normalized on the CPU in configure_orbs before upload. |
| `ORB_BANK.rotation_speed` | Sky & Light · Motion — all rules | PARAM | **D** | Read per frame by orb_dynamics' dome-rotation branch (gated on abs(rotation_speed) > 0.0001); the bank authors 0.012 rad/s. |
| `ORB_BANK.rule_drag_brownian` | Sky & Light · Motion — all rules | PARAM | **D** | Read every frame as the brownian arm's drag exponent; the bank authors 0.0, which pack_flocking_'s passthrough sentinel turns into 1.0x, and the row's floor of 0.02 is one step above that sentinel so the dial cannot write the authored rest. |
| `ORB_BANK.rule_drag_flocking` | Sky & Light · Motion — all rules | PARAM | **D** | Read every frame as the flocking arm's drag exponent, and flocking is the rule the bank actually selects (ORB_TABLE motion_rule = 3). |
| `ORB_BANK.rule_drag_frozen` | Sky & Light · Motion — all rules | PARAM | **D** | Read every frame as the frozen arm's drag exponent — the one rule with no energy source for speed_mult to scale, so drag is the whole of its motion law. |
| `ORB_BANK.rule_drag_orbital` | Sky & Light · Motion — all rules | PARAM | **D** | Read every frame as the orbital arm's drag exponent; same 0.0 authored value and same passthrough sentinel the row's 0.02 floor sits one step above. |
| `ORB_BANK.orbital_base_speed` | Sky & Light · Orbital rule | PARAM | **D** | Read every frame under rule ORBITAL: multiplied by the per-orb speed variance and speed_mult, then spun through the Rodrigues rotation. |
| `ORB_BANK.brightness` | Sky & Light · Orbs | PARAM | **D** | Not in the reseed set: it rides the uniform upload and is a per-frame GPU read, so it lands under the finger without replacing an orb. |
| `ORB_BANK.count` | Sky & Light · Orbs | PARAM | **D** | configure_orbs clamps it to Dim::MAX_ORBS into OrbsState::count; in ORB_RESEED_BITS, so a write re-runs the init kernel at the boundary. |
| `ORB_BANK.drag` | Sky & Light · Orbs | PARAM | **D** | configure_orbs runs it through the zero-means-default eff() and into GPUOrbConfig::drag; in ORB_RESEED_BITS, so a write re-seeds. |
| `ORB_BANK.enabled` | Sky & Light · Orbs | PARAM | **D** | configure_orbs sets OrbsState::active from it and it sits in ORB_RESEED_BITS, so a write re-seeds the sky at the next boundary rather than riding the uniform. |
| `ORB_BANK.palette_id` | Sky & Light · Orbs | PARAM | **D** | configure_orbs hands it to pack_palette_; in ORB_RESEED_BITS, so a write re-seeds the sky at the boundary. |
| `ATMOS.sun_az_spread_deg` | Sky & Light · Sun | PARAM | **D** | Seeded 0 since the bank rose (ONE_WORLD-II U1) and the point-row static_assert pins the DESIGN table's spreads at 0; ATMOS_LIVE — the bank the panel edits — carries no such pin, and draw_atmosphere short-circuits both bearing axes while the spread is 0. |
| `ATMOS.sun_direction` | Sky & Light · Sun | PARAM | **D** | The light vector's centre; stage_sky re-draws from ATMOS_LIVE on every apply — world birth and panel edit alike — and pushes the normalized result into config for the shadow VP. |
| `ATMOS.sun_el_spread_deg` | Sky & Light · Sun | PARAM | **D** | Same point-row pin as the azimuth spread; nonzero takes the trig path, where draw_atmosphere clamps the drawn elevation to [5°, 88°] so the shadow VP cannot degenerate. |
| `CONFIG.mode_gol_height_scale` | Terrain · Automaton couplings | PARAM | **D** | Read per frame as the multiplier on the automaton's drawn alive_height contribution. |
| `CONFIG.mode_gol_tick_scale` | Terrain · Automaton couplings | PARAM | **D** | Read per frame as the divisor on the automaton's tick period in both the phase and frequency terms. |
| `CONFIG.band_phase_origin_0` | Terrain · Band phase | PARAM | **D** | Continental band's time origin, subtracted from config.terrain_time inside the per-frame terrain band accumulation. |
| `CONFIG.band_phase_origin_1` | Terrain · Band phase | PARAM | **D** | Regional band's time origin, read per frame through the same switch. |
| `CONFIG.band_phase_origin_2` | Terrain · Band phase | PARAM | **D** | Local band's time origin, read per frame through the same switch. |
| `CONFIG.band_phase_origin_3` | Terrain · Band phase | PARAM | **D** | Detail band's time origin, read per frame through the same switch. |
| `CONFIG.band_phase_origin_4` | Terrain · Band phase | PARAM | **D** | Fine band's time origin; the accumulation loop skips band 4 under the Nyquist ruling, so the switch case is reachable but that band is bake-only. |
| `CONFIG.band_phase_origin_5` | Terrain · Band phase | PARAM | **D** | Tectonic band's time origin, read per frame through the same switch. |
| `CONFIG.band_blend_0` | Terrain · Bands | PARAM | **D** | Read per frame in the live card's heights loop; boot pins it to the -1 inactive sentinel via set_band_motion and the whole band sum sits behind config.terrain_time > 0. |
| `CONFIG.band_blend_1` | Terrain · Bands | PARAM | **D** | Read per frame in the live card's heights loop; boot pins it to the -1 inactive sentinel via set_band_motion and the whole band sum sits behind config.terrain_time > 0. |
| `CONFIG.band_blend_2` | Terrain · Bands | PARAM | **D** | Read per frame in the live card's heights loop; boot pins it to the -1 inactive sentinel via set_band_motion and the whole band sum sits behind config.terrain_time > 0. |
| `CONFIG.band_blend_3` | Terrain · Bands | PARAM | **D** | Read per frame in the live card's heights loop; boot pins it to the -1 inactive sentinel via set_band_motion and the whole band sum sits behind config.terrain_time > 0. |
| `CONFIG.band_blend_4` | Terrain · Bands | PARAM | **D** | Band 4 is the one band the heights loop skips unconditionally (`if (b == 4u) { continue; }`, the Nyquist ruling), so the fine ripple's blend is fetched by no live iteration. |
| `CONFIG.band_blend_5` | Terrain · Bands | PARAM | **D** | Read per frame in the live card's heights loop; boot pins it to the -1 inactive sentinel via set_band_motion and the whole band sum sits behind config.terrain_time > 0. |
| `CONFIG.mode_checker_scatter` | Terrain · Modes | PARAM | **D** | GPU config read per frame in the terrain fragment shader's mode-bias test and in the cell colour draw. |
| `CONFIG.mode_color_shift` | Terrain · Modes | PARAM | **D** | GPU config read per frame in the terrain fragment shader's mode-bias test and in the cell colour draw. |
| `CONFIG.mode_discrete_tier` | Terrain · Modes | PARAM | **D** | Read per frame by the cell colour draw, rounded to the target discrete tier. |
| `CONFIG.mode_palette_intensity` | Terrain · Modes | PARAM | **D** | Read per frame as the drift strength that gates the whole palette override, in both the fragment shader and the cell colour draw. |
| `CONFIG.mode_palette_target` | Terrain · Modes | PARAM | **D** | Read per frame by the cell colour draw, which hands it to palette_target_color whenever the drift is non-zero. |
| `CONFIG.terrain_time` | Terrain · Motion | PARAM | **D** | Read per frame in the card heights pass as the uniform bands_awake gate and as the per-band t_eff; it rests at 0 (terrain_looks REST_*), which is the [MUSICAL] conjunct that gates the band sum off. |
| `CONFIG.palette_center[0][0]` | Terrain · Palette 0 sand | PARAM | **D** | Live cadence and read every frame by the terrain colour mix in world.wgsl (config.palette_center[i].rgb), so a turn is heard immediately. |
| `CONFIG.palette_light[0][0]` | Terrain · Palette 0 sand | PARAM | **D** | Live cadence, per-frame reader alongside palette_center in the same world.wgsl mix expression. |
| `CONFIG.palette_center[1][0]` | Terrain · Palette 1 salmon | PARAM | **D** | The salmon median; palette_color_smooth mixes it against the light variant by complexity on both the live fragment path and the patch bake, and config_ uploads whole every frame. |
| `CONFIG.palette_light[1][0]` | Terrain · Palette 1 salmon | PARAM | **D** | The salmon light variant, the other end of palette_color_smooth's complexity mix; also read by palette_target_color on the drift branch. |
| `CONFIG.palette_center[2][0]` | Terrain · Palette 2 green | PARAM | **D** | Read per fragment every frame by both terrain colour folds; the green lane's mix weight is authored rare (PALETTE_WEIGHT_REST[2] = 0.04) and the underlying rest triple has not moved since TERRAIN_LOOKS. |
| `CONFIG.palette_light[2][0]` | Terrain · Palette 2 green | PARAM | **D** | Read per fragment every frame as the low-complexity end of the same two colour folds; rest triple unmoved since TERRAIN_LOOKS. |
| `CONFIG.palette_center[3][0]` | Terrain · Palette 3 warm | PARAM | **D** | The warm palette's median RGB is read per frame by both terrain colour paths; boot writes it once from terrain_looks::PALETTE_CENTER_REST and after boot the organ is its only writer. |
| `CONFIG.palette_light[3][0]` | Terrain · Palette 3 warm | PARAM | **D** | The warm palette's light variant is read per frame beside the centre in the same two mixes; boot writes it once from terrain_looks::PALETTE_LIGHT_REST and after boot the organ is its only writer. |
| `CONFIG.palette_weight[0]` | Terrain · Palette mix | PARAM | **D** | Sand's share; palette_weights_at_node accumulates the four lanes into a cumulative per-node pick, run on the live fragment path and at bake. |
| `CONFIG.palette_weight[1]` | Terrain · Palette mix | PARAM | **D** | Salmon's share in the same cumulative per-node pick; the WGSL normalises the four lanes rather than clamping them. |
| `CONFIG.palette_weight[2]` | Terrain · Palette mix | PARAM | **D** | Green's share — the rest pin holds it at 0.04, the rare lane — in the same per-node cumulative pick. |
| `CONFIG.palette_weight[3]` | Terrain · Palette mix | PARAM | **D** | Warm's share in the same per-node cumulative pick. |

---

## FLAGS, COLLECTED

Every one raised under the continuation doctrine, with the ruled default
applied and why the handoff did not cover it.

1. **Branch.** Executed on `master`, per this session's standing instruction,
   not on a per-unit branch.
2. **U4 quarantined**, on three independent grounds (L33, no release tags,
   zero cost). Jean's run list dropped it **by omission rather than by
   statement**; it is stated here so it is not inferred.
3. **U2's demotion did not run.** The nearest ruled default was *"if
   `organ_gap` exits non-zero on a finding, change it to exit zero"* — a
   conditional whose condition is false. Executing the demotion anyway would
   have repealed L45's standing witness.
4. **U2 and U4 are coupled** through `organ_ledger.py`'s verbatim embed of
   `organ_gap`'s stdout. The dependency map says U1–U6 have **no dependencies
   on each other at all**.
5. **U1, U2 and U5 all edit `organ_params.inc`; U1 and U3 both target
   `docs/ORGAN.md`.** The same independence claim fails on file contention
   too. They were serialized.
6. **U1's ruling text was not installed**, because it paraphrases the line it
   would sit under (L46). The belief landed instead.
7. **U3 cites nothing**, because `docs/OPEN.md`'s seam entry reserves no shape
   questions — the handoff's condition for citing it is unmet.
8. **U5 moved 3 blocks, not 5.** Three of the handoff's named instances fail
   its own >4-line / separable-block rule. One instance it did not name was
   found by walking the file.
9. **The frozen set states a wrong fact.** Block enum holes are at **8 and
   10**, not 8/10/11 — id 11 is live (`ORGAN_BLOCK_RIBBON_SPAWN`), and the
   banner three lines above it warns against exactly that misreading.
10. **`organ_registry.hpp`'s own prose miscounts its macro forms** — "The five
    forwards below are the price", with four forwards beneath it. Found in
    passing, not fixed: it is not this campaign's subject.
11. **A live range is constrained against two dead terms.** The chain note U5
    *keeps* at its row reads `EXIST(350) > ring > lod0, and ring − band >
    lod0`, and both `lod0` and `band` are retired — their homes are now
    `_pad_lod0_radius_retired` and `_pad_grain_band_retired`. Flagged, not
    fixed: it is a ruling about a range, not a prose move.
12. **The census over-populated A** and was corrected before publication; the
    corrections are stated above rather than folded silently into the table.
13. **A sync test moved `master` mid-campaign** — three commits adding 63,206
    lines to `CLAUDE.md`, moving them to `CMakeLists.txt`, then deleting them.
    They net to **exactly zero**; both files verified byte-identical. Merged at
    `804fbee7`, which records it so the next reader meeting a 63,000-line diff
    does not have to re-derive that it round-tripped.

## THE ROUND'S REAL FINDING

We went in believing the organ was taxing the work. **The tree says it mostly
is not.** The one measurable cost was prose volume in a single file, and even
that fell only 411 → 402 because the campaign spent most of what it saved
saying what the posture is.

Everything else was a habit rather than a mechanism: a practice of defending
absences to a tool built specifically not to ask for a defense, and of reading
an obligation into a sentence whose own second clause says a missing dial is
merely silent. **The organ was never the drag. The belief about it was.**

That belief is now written down in the file people open, which is the only
place a habit of this kind can be ended.

## WHAT JEAN GATES

1. **The name.** `ORGAN_REST` is provisional.
2. **U1's and U3's wording** — both marked DRAFT in their commit subjects, per
   the amendment. They are drafts for his pen, not settled text.
3. **The U6 census** — which rows, if any, ever leave. This campaign removed
   nothing and recommends nothing.
4. **U4** — quarantined here with three grounds. Overrulable, but the L33
   collision would have to be answered first.
5. **The merge, the tag, and the probe.** No unit here touched GPU state or
   the shader's token stream, so the campaign carries no probe debt of its own
   — but `glaw1` and the build remain his, and were not run here.
