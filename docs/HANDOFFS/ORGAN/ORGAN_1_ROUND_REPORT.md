# ORGAN_1 — round report

Reset hash `1d22d2b` (SUNSET_2's tip). Branch `claude/organ1`,
fast-forwarded into `master` per the standing instruction. Three units,
three commits, unit order as handed down: O1a → O1b → O1c.

| unit | commit | one line |
| --- | --- | --- |
| O1a | `546b6f4` | the contested-dial instrument |
| O1b | `f48ad7a` | instance and definition |
| O1c | `812a3f0` | the uniform floor's note cites the lane table |

Gates on the merged tip: glaw1 GREEN · console gate PASS ·
`binding_gen --check` all relations agree, all witnesses pass (S-6
included). `world.wgsl` untouched this round, so naga's verdict stands
from SUNSET_2.

---

## O1a — the contested-dial instrument

`organ_set` shadows the bytes that land in the home, read BACK from the
home so a clamp can never masquerade as a rival author. Once a frame, at
the flush boundary, the observer re-reads and asks whether the home still
says what the panel said. Never disagreed is FREE, stood a while and then
lost it is EVENT, lost it at once is PER-FRAME; the evidence is the
survival count.

**The per-frame threshold is one frame, not zero.** The observer sits at
the head of the frame, so a write arriving between frames is seen intact
once before that frame's authors run again. A per-frame author therefore
leaves survival == 1. Reading 0 as the only per-frame signature would have
classified every one of them as an EVENT. The threshold is a consequence
of where the observer stands and is stated beside it in the code.

`organ_contest_frames(i)` is defined as the frames the panel's last write
STOOD — the handoff named the function without fixing its meaning, and
survival is the number a table wants: it is the evidence behind the class,
and for an uncontested dial it keeps climbing, which is how a FREE reading
earns confidence.

Nothing acted on the finding. No write path changed, no dial was
withdrawn, no author was edited.

### The census O1b consumed — and the reading I could not take

The handoff's report requirement is a table taken at a running browser:
boot with `?organ=1`, drag all sixteen dials, change mood twice, tabulate.
**This environment cannot run the Emscripten build, so that table is
yours at the gate.** What I could do instead is read the authors
statically and commit to a prediction, so the gate becomes a comparison
rather than a discovery.

**Protocol, so the reading is reproducible.** Switch the panel to
`preview` before dragging. A DEFINITION write never touches the instance
and so asks the instrument nothing — a starred dial dragged in definition
mode stays `·` on purpose, and the legend says so.

| # | dial | author found by census | cadence of that author | predicted |
| --- | --- | --- | --- | --- |
| 0 | `LIGHTING.sun.direction` | `apply_mood_lighting` → deps → `upload_lights` | `lights_dirty`: mood change, world reset | EVENT |
| 1 | `LIGHTING.sun.color` | same | same | EVENT |
| 2 | `LIGHTING.sun.intensity` | same | same | EVENT |
| 3 | `LIGHTING.sun.ambient` | same | same | EVENT |
| 4 | `CONFIG.fog_density` | `phase_motion_drivers` → `set_fog` from the visual canvas | every frame, if the `fog.*` pipes resolve | PER-FRAME |
| 5 | `CONFIG.fog_color` | same | same | PER-FRAME |
| 6 | `CONFIG.aura_enabled` | `tick_pawn_couplings` → `set_aura_enabled` | every frame | PER-FRAME |
| 7 | `CONFIG.pawn_aura_height` | `tick_pawn_couplings` → `set_pawn_aura_height` | every frame | PER-FRAME |
| 8–15 | `AGENT_ROOM.tier_gains[0..3].*` | `upload_agent_registries`, once at world-init | world init only | FREE within a world, EVENT across a regen |

A disagreement between this column and the gate's is worth more than an
agreement: it means a fact has an author the census could not see, which
is an ORGAN_2 finding rather than an instrument fault.

---

## O1b — instance and definition

`[Mood] Applied` is `apply_mood`, and it fans ONE profile out to the
renderer, the GoL gate, the pawn, the sun, the clear colour, the amp
ceiling, the spot array, the shell and the sky. The full table is in
`docs/ORGAN.md`.

That profile was `MOOD_TABLE`: constexpr, asserted, and therefore
unreachable by any runtime edit. It is now `mood_def(mood)` — `MOOD_LIVE`,
seeded from `MOOD_TABLE` when the program loads. **Every runtime reader in
the tree moved with the apply** (mood.hpp ×8, entity_pipeline ×3,
spawn_engine, ribbon ×2, tile_world), so the definition in force has one
home and not two. `MOOD_TABLE` keeps exactly two jobs: seeding that array,
and standing under the F-3 row assert and the column-drift asserts. The
constexpr readers in `bodies/gallery.hpp` stay on the design table on
purpose — they are compile-time geometry budgets, and a wall's allowance
is not a live dial.

**The eligibility rule**, written in code beside `MOOD_LIVE`: a field may
carry a definition target only if the mood apply is its ONLY runtime
reader. The atmospheric group passes. The structural group does not —
world generation reads it, and rewriting `wall_height` without
regenerating the world means nothing at best and disagrees at worst. This
rule is why four dials took targets and twelve did not; it is a ruling
about readers, not a shortage of effort.

The registry gained a definition column and a second enrollment macro, so
the compiler swears to the definition offset exactly as it swears to the
instance offset. `organ_set` gained a target: a mood id writes that mood's
definition and raises one re-apply flag; `-1` writes the instance. A
definition write deliberately does NOT write the instance, does not set a
dirty bit, and does not call `note_write` — the instance is the mood
apply's to produce, and the instrument must keep measuring the instance
rather than a value the panel put there on the definition's behalf.

The re-apply runs at the frame boundary in the cartridge, the one layer
that owns both the mood deps and the queue, and **only for the live
mood** — re-applying another mood's sun would paint this world with
another world's light. An edit to a non-live mood is stored and takes
effect when the program next enters it.

### FLAGGED for ORGAN_2

1. **Per-frame authors with no definition.** If the gate confirms the
   prediction above, `fog_density`, `fog_color`, `aura_enabled` and
   `pawn_aura_height` are authored every frame and have nowhere durable
   to go: `MoodProfile` has no fog and no aura, so there is no definition
   to write. These four need a definition surface of their own. Inventing
   a `MoodProfile` field for them would be the second copy the charter
   forbids. `docs/ORGAN.md` carries the standing flag; the names are here
   because they are a prediction, and the doc should record what was
   measured.
2. **`clear_color` is the mirror case** — a definition with no instance
   dial, because `clearColor_` is not one of the three homes.
3. **The tiers have an author that is not a mood**, so `MoodProfile` is
   the wrong place to reach for even though they are contested across a
   world regen.
4. **`config.sun_direction` beside `lighting.sun.direction`** — two
   apparent homes for one fact, carried in from CHORD.
5. **`mood_def(mood)` wraps** (`% MOOD_COUNT`) rather than rejecting, so a
   hand-edited import file naming mood 9 silently writes mood 1. The
   panel checks only `mood >= 0`; it has no `MOOD_COUNT` to check against
   without another ABI call. Small, and worth closing when the ABI next
   moves.

---

## O1c — the uniform floor's note

`worst row 11 of 12 — the agents compute family` was true before CHORD
redistricted the uniform seats and has been wrong since: M-1 reads the
worst uniform row at **5 of 12**. A note that restates a number the
MANIFEST already computes is a second copy of that number, and it went
stale silently in a line the granted-vs-floor print quotes to the
operator.

The note now cites its subject instead of repeating it: `the wallet fits
the default; worst row per MANIFEST's lane table`. One line in the schema,
one line regenerated in `limits_floor.gen.inc`;
`binding_registry.hpp`, `binding_surface.gen.inc` and `audit/MANIFEST.md`
re-emitted byte-identically, which is the "only that line" requirement
discharged.

**FLAGGED, not fixed:** the storage row beside it still spells `worst row
5 of 8`. It is correct today and carries exactly the same hazard. This
unit's bound was one line.

---

## Round notes

- The ORGAN charter lives at `docs/ORGAN.md`, not `src/docs/` — the same
  flag CHORD and ORGAN_0 raised. `src/docs/` does not exist in this tree.
- `organ_get` still has no caller: the panel reads current values from the
  manifest's `v` array. Pre-existing since ORGAN_0c, not a regression.
- The panel is the only caller of `organ_set`, so the arity change reached
  exactly one site.
- `incubator_dual.cpp` is compiled by no gate. O1a's one line there calls a
  method glaw1 does compile, on the concrete type the line below it
  already calls — but the file itself is unchecked, and that is a standing
  gap rather than a fact about this round.
