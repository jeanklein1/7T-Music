# Cartridge annotation manifest — chunks 1, 2, 3

**Scope:** the full `cartridge.hpp` file end-to-end. Three annotation chunks
covering, in order: the cartridge spine (chunk 1), the five banner-only
modules `seed_utils` / `gol_zones` / `entities` / `spawn_engine` / `gallery`
(chunk 2), and the three specialized-family blocks Sphere / Cube / Ribbon
(chunk 3).

**Tags placed: 53 SEAM, 3 TODO, 5 NOTE.** The other `.inl` modules and the
foreign files (`state.hpp`, `renderer.hpp`, `world.wgsl`) remain to be
annotated in later chunks.

**File integrity verified:**

- Brace counts in actual code (non-comment): 1281 `{` / 1286 `}`,
  identical to the original.
- Include count: 28, identical to the original.
- All tag bodies are pure comments. No semantic changes.

---

## Tag families summary (all chunks)

| Family | Count | Meaning |
|--------|-------|---------|
| `SEAM[spine:owns]` | 3 | Genuinely spine-owned (chunk 1) |
| `SEAM[spine:K1]` | 1 | The big update() ramp-in-spine harvest |
| `SEAM[spine:K2-related]` | 1 | FAMILY_DISPATCH wrappers |
| `SEAM[spine:L1]` | 1 | Empty for-loop bug |
| `SEAM[spine:L2]` | 1 | audit half-coverage |
| `SEAM[spine:L4]` | 1 | Phase-table comment at update() top |
| `SEAM[spine:P5]` | 2 | Readback state machines + worldGen_ |
| `SEAM[spine:P8]` | 1 | PlayerState deferred fields |
| `SEAM[spine:per-mood-data]` | 1 | Indoor lighting / palettes / WALL_ART |
| `SEAM[seed_utils:P9]` | 1 | Library without state |
| `SEAM[seed_utils:contract]` | 1 | FXC mirror functions |
| `SEAM[gol_zones:complete-subsystem]` | 1 | Bespoke single-block subsystem |
| `SEAM[gol_zones:dual-algorithm]` | 1 | NEW FINDING: Conway + Pulse co-equal |
| `SEAM[gol_zones:L1]` | 1 | MODE_LATTICE_SPACING missing MUST match |
| `SEAM[gol_zones:P4]` | 1 | Hygiene rows MOOD_MULTIPLIER |
| `SEAM[entities:P10]` | 1 | Per-family vocabulary template (canonical) |
| `SEAM[entities:K1]` | 1 | Two-home tier representation (legacy + sampling) |
| `SEAM[entities:taxonomy]` | 1 | Sphere/Cube NOT here, in Ch. 13 block |
| `SEAM[entities:L1]` | 1 | RibbonProp stride convention undocumented |
| `SEAM[orbs:D2]` | 1 | ORB_MOOD_TABLE migrate to orbs.inl |
| `SEAM[mood:K4]` | 2 | Mood-5 reference clone (MOOD_TABLE + ORB_MOOD_TABLE rows) |
| `SEAM[spawn_engine:P11]` | 1 | Templated active-array helper |
| `SEAM[spawn_engine:structural]` | 2 | Mid-block #include not a leak |
| `SEAM[spawn_engine:L1]` | 1 | Latent diagnostic DIAG_ENTITY_LIFECYCLE |
| `SEAM[gallery:complete-subsystem]` | 1 | Bespoke single-block subsystem |
| `SEAM[gallery:dual-role]` | 1 | Outdoor-terrain / indoor-wall split |
| `SEAM[gallery:L1]` | 1 | ENVIRONMENTAL weight 0.01 unexplained |
| `SEAM[gallery:L2]` | 1 | P3 instance — clean concern separation |
| `SEAM[sphere:taxonomy]` | 1 | Vocabulary lives here, not in entities.inl |
| `SEAM[sphere:L1]` | 2 | FloatingEntityTierProfile naming, misleading "Reuses..." comment |
| `SEAM[sphere:P5]` | 1 | last_alloc_time race protection |
| `SEAM[cube:taxonomy]` | 1 | Vocabulary lives here, not in entities.inl |
| `SEAM[cube:cx-cz-mirror]` | 1 | CPU mirror fields, agents:D2 family |
| `SEAM[ribbon:taxonomy]` | 1 | Bespoke machinery here, vocabulary in entities.inl |
| `SEAM[ribbon:dual-entry]` | 1 | commit_ribbon called from two paths |
| `SEAM[ribbon:L1]` | 1 | Unconditional stdout — exhibition guard |
| `SEAM[mood:K1]` | 1 | Indoor/outdoor binary at MOOD_TABLE |
| `SEAM[mood:K3]` | 1 | TEARDOWN block parallels apply_mood |
| `SEAM[musical:K2]` | 3 | Musical mode intensities + band motion + include site |
| `SEAM[musical:K3]` | 2 | prevPolyphony_ pulse onset + include cross-ref |
| `SEAM[orbs:P1]` | 1 | update_orb_coupling counter-example |
| `SEAM[pawn:K1]` | 1 | Aura presence ramp at update() |
| `TODO[phase-1:spine:L1]` | 1 | Investigate empty for-loop intent |
| `TODO[phase-1:spine:L2]` | 1 | Document audit coverage rationale |
| `NOTE[seam-map]` | 5 | "Don't migrate / kept on purpose" markers |

---

## Tag locations by chunk

### Chunk 1 — the cartridge spine

1. **File header** — overview tag explaining seam-map relationship.
2. **`#include "modules/musical.inl"`** — `musical:K2` and `musical:K3` cross-refs.
3. **`struct PlayerState`** — `spine:P8` for explicit "Future (deferred)" fields.
4. **`MOOD_TABLE`** — `mood:K1` (indoor/outdoor binary), `mood:K4` (mood-5 reference clone).
5. **"Indoor Wall Palette" header** — `spine:per-mood-data` (NEW FINDING, indoor lighting tables).
6. **`enum class PawnReadbackState`** — `spine:P5` for readback state machines + worldGen_.
7. **`evict_patch_entities` empty for-loop** — `spine:L1` + `TODO[phase-1:spine:L1]`. **REAL BUG.**
8. **`audit_entity_integrity`** — `spine:L2` + `TODO[phase-1:spine:L2]`.
9. **`struct FamilyDispatch`** — `spine:owns` + `spine:K2-related` + `NOTE[seam-map]`.
10. **`void update(...)` signature** — `spine:K1` (big harvest) + `spine:L4`.
11. **Aura presence trajectory block** — `pawn:K1` confirmation.
12. **`case TransitionPhase::TEARDOWN:`** — `mood:K3` + `spine:P5`.
13. **"Polyphony-driven band motion" block** — `musical:K2` (NEW FINDING).
14. **"Musical animation modes" block** — `musical:K2` main site.
15. **"Radial pulse onset detection" block** — `musical:K3`.
16. **`update_orb_coupling(...)` call** — `orbs:P1` counter-example.
17. **`void render(...)` signature** — `spine:owns`.
18. **`void stream_patches(...)` signature** — `spine:owns`.

### Chunk 2 — banner-only modules

19. **seed_utils block opener** — `seed_utils:P9` + `seed_utils:contract`.
20. **gol_zones block opener** — `gol_zones:complete-subsystem` + `gol_zones:dual-algorithm` (NEW FINDING).
21. **`MODE_LATTICE_SPACING = 120.0f`** — `gol_zones:L1` (missing MUST match).
22. **`MOOD_MULTIPLIER` for GoL** — `gol_zones:P4`.
23. **entities block opener** — `entities:P10` + `entities:K1` + `entities:taxonomy`.
24. **`RibbonProp` registry** — `entities:L1` (stride convention undocumented).
25. **`ORB_MOOD_TABLE`** — `orbs:D2` + `mood:K4`.
26. **spawn_engine block opener** — `spawn_engine:P11` + `spawn_engine:structural` + `NOTE[seam-map]` + `spawn_engine:L1`.
27. **`#include "modules/entity_types.inl"` mid-block** — `spawn_engine:structural` + `NOTE[seam-map]`.
28. **gallery block opener** — `gallery:complete-subsystem` + `gallery:dual-role`.
29. **`ENVIRONMENTAL` row in SHOT_PARAMS** — `gallery:L1`.
30. **`PhotographerCaptureConfig`/`GalleryConfig` split** — `gallery:L2` (P3 instance).

### Chunk 3 — specialized-family blocks

31. **Sphere Entity System opener** — `sphere:taxonomy` + `sphere:L1` (naming claim).
32. **`FloatingEntityTierProfile` and "(Reuses...)" comment** — `sphere:L1` (misleading comment).
33. **`struct ActiveFloater`** — `sphere:P5`.
34. **Cube Entity System opener** — `cube:taxonomy` + `cube:cx-cz-mirror`.
35. **Ribbon Dispatch Pipeline opener** — `ribbon:taxonomy` + `ribbon:dual-entry`.
36. **Ribbon spawn `std::cout`** — `ribbon:L1` (exhibition guard candidate).

---

## New findings beyond the seam map (across all chunks)

The annotation work surfaced **6 things not previously in the seam map**:

1. **`spine:L1` — Empty for-loop bug** in `evict_patch_entities`. Two-line
   surgical fix once intent is clarified. **Phase 1 immediate-fix candidate.**

2. **`spine:per-mood-data` — Indoor lighting tables in spine.**
   `INDOOR_PALETTES`, `WALL_ART`, `LIGHT_SCHEMES`, `IndoorLightProp` are
   per-mood authoring data declared in the spine, parallel to the
   already-flagged `ORB_MOOD_TABLE`. Same family as `orbs:D2` but never
   named. **Migration target with mood.inl decomposition.**

3. **`spine:K2-related` — Family-dispatch wrappers** (~400 lines of
   `dispatch_evict_*`, `dispatch_prepare_mesh_*`, `dispatch_mesh_gen_*`).
   Integration glue, correctly placed in spine; under-credited.

4. **`spine:L2` — `audit_entity_integrity` half coverage.** 4 of 12
   families covered; the rest may be intentionally excluded (bespoke
   families have their own state machines) but the rationale isn't
   documented. **Phase 1 immediate-fix candidate (documentation).**

5. **`gol_zones:dual-algorithm` — Conway + Pulse co-equal algorithms.**
   The seam map's Ch. 12.B treated GoL as a monolithic block; in fact
   it houses two algorithms (Conway with `GOL_TIERS[]`, Pulse with
   `PULSE_TIERS[]`, governed by `PULSE_ALGORITHM_CHANCE = 0.35`). Inventory
   correction, not a leak.

6. **`sphere:L1` (refined) — The "(Reuses FloatingEntityTierProfile...)"
   comment is misleading.** The seam map noted the naming nit but didn't
   catch that the comment claims cubes "reuse" the struct when in fact
   they have their own `CubeTierProfile`. Phase 3 cleanup batch.

---

## Phase 1 candidates for the first Claude Code session

For your decision (per your "I tag, you decide" rule):

**Currently in `cartridge.hpp` and ready to act on:**

- `TODO[phase-1:spine:L1]` — empty for-loop investigation. Read git
  blame; either delete the loop wrapper + `gx`/`gz` declarations, or
  restore the meaningful body if git shows one was removed.
- `TODO[spine:L2]` — document audit coverage rationale, OR file a
  follow-up note (don't extend audit in this session).

**Other Phase 1 items from the seam map, not yet tagged in source:**

- `floaters:L1` — two-line bug in `floaters.inl` (`cpuAgents_[0]` →
  `cpuAgents_[player_.possessed_slot]`). Annotated in chunk 4 (when we
  do the `.inl` modules), but actionable now if we want to bundle.
- `floaters:L4` — add `CUBE_BEHAVIOR_COUNT_WGSL` to `world.wgsl` plus
  static_assert in `floaters.inl`. Two-file fix.

**My inclination for the first CC session, conservative reading:** start
with **only the two `TODO[phase-1:*]` tags inside `cartridge.hpp`**.
That's a single-file calibration session. The floaters items can be
session 2 once we've seen how session 1 goes.

---

## Instruction packet for Claude Code (when you're ready to send chunk 1)

> **First session: Phase 1 immediate fixes in cartridge.hpp.**
>
> Scope: act on the two `TODO[phase-1:*]` tags in `cartridge.hpp`. Do not
> touch any other file. Do not act on `SEAM[*]` tags (those are
> observations, not actions). Do not act on `NOTE[seam-map]` tags
> (those are "don't migrate" markers).
>
> The two TODOs in scope:
>
> 1. `TODO[phase-1:spine:L1]` (in `evict_patch_entities`) — investigate
>    the empty for-loop. The tag body explains: read git blame on the
>    loop region. If git history shows a meaningful loop body that was
>    removed, restore it. If the loop was always vestigial, delete the
>    loop wrapper, the `gx`/`gz` declarations, and leave only the
>    `patch.entity_ref_count = 0` assignment. After deciding, replace
>    the SEAM/TODO comment block with a brief one-line comment
>    explaining what was done and why.
>
> 2. `TODO[phase-1:spine:L2]` (in `audit_entity_integrity`) — determine
>    whether the half-coverage of entity families is intentional. If
>    intentional, document the rationale alongside the function's banner
>    comment. If accidental, file a follow-up note (a comment naming
>    what would need to change) but **do not** extend the audit in this
>    session — its design needs review before adding families.
>
> **Working approach:**
> - Read each TODO's body in full before acting; the body is the spec.
> - For ambiguity, the file `docs/the_board_seam_map.md` Chapter 15 has
>   full context. The file `docs/cartridge_annotation_manifest.md`
>   summarizes what these chunks found.
> - **Do not run a build until both TODOs are resolved.** Then run the
>   project's normal build command and report any errors.
> - **Do not move code between files** in this session.
> - **Do not delete any `SEAM[*]` or `NOTE[seam-map]` tags** even if
>   surrounding code changes. Tags are part of the navigation layer.
> - When the TODO is resolved, replace the `TODO[phase-1:*]` line with
>   a brief done-note: `// DONE[phase-1:<id>] <what was done>`. Keep
>   the surrounding `SEAM[*]` tag intact unless the SEAM observation no
>   longer applies.
>
> **Output:**
> - The modified `cartridge.hpp`.
> - A summary report listing each TODO, what was decided, what was
>   done, and the build outcome.
> - Any questions or ambiguities you couldn't resolve — flag them
>   rather than guess.

---

## Next chunks (after you commit and review this one)

- **Chunk 4** — annotate the eight `.inl` modules (`musical.inl`,
  `pawn_aura.inl`, `ground_architecture.inl`, `agents.inl`, `mood.inl`,
  `orbs.inl`, `floaters.inl`, `entity_pipeline.inl`, `entity_types.inl`,
  `render_passes.inl`, `input.inl`). Most-actionable findings: floaters:L1
  (the `cpuAgents_[0]` bug), floaters:L4 (the `CUBE_BEHAVIOR_COUNT_WGSL`
  count), agents:L1 (PLAYER_SLOT comment), input:L1 (`request_mood_transition`).
- **Chunk 5** — `state.hpp`, `renderer.hpp`, `world.wgsl`. Findings:
  state:L1 (Coupling reserved annotations), state:L3 (typo), wgsl:L1 (=
  floaters:L4 cross-file partner), wgsl:L4 (mesh-gen binding isolation
  recognition).
