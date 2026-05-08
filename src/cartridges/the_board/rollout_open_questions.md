# Rollout open-questions report

A running list of items surfaced during the agents/orbs/per-module rollout
that don't fit cleanly into a single module's audit. Each entry names what
was found, where, and what kind of resolution it needs.

The report exists because the rollout is the first time the active
references (pattern glossary, organization guide) are being applied
against source at scale. Drift between docs and source, or gaps in either,
will show up most readily in this work — and many of those findings will
be most useful when *aggregated*, not addressed per-file.

When this list reaches a useful size we'll do a sweep: which items recur
(suggesting a pattern), which are isolated (suggesting one-off cleanup),
which are confusion (no action needed), and which warrant doc updates
(glossary v1.4, organization guide revisions, etc.).

---

## Format

Each entry has:

- **ID.** Sequential, for back-reference (Q1, Q2, …).
- **Surfaced.** Which file/audit raised it.
- **What.** One-line summary.
- **Detail.** Full context, evidence, and the question it raises.
- **Tentative resolution kind.** code-edit / glossary-correction /
  organization-guide-correction / seam-map-question / no-action /
  unclear-needs-review.

Items are not actioned from this report directly. They wait for the
end-of-rollout sweep where the patterns can be seen against the whole
codebase.

---

## Open items

### Q1 — `OrbPlayerState` named in glossary but absent from source

**Surfaced.** agents/orbs audit.

**What.** The glossary v1.3's P3 canonical-instance text references
`OrbPlayerState` as a named struct in `orbs.inl`. That struct does not
exist in source.

**Detail.** P3 ("Player state vs mood state, explicit split") is honored
in `orbs.inl`, but via labeled member variables under
`// ═══ RUNTIME CPU STATE ═══`, sub-grouped with comments naming which
fields are player-mutated vs. mood-authored. The mood side has a real
struct (`OrbMoodConfig`); the player side does not. The glossary's
canonical-instance text describes a struct that isn't there.

Possible explanations:

1. The struct existed at some prior point and was dissolved into loose
   members during a refactor.
2. The struct was always conceptual — the glossary author imagined how
   the split *should* be done rather than describing how it actually is.
3. The struct exists in another file I haven't checked.

**Tentative resolution kind.** glossary-correction (v1.4 candidate). The
correct text would acknowledge that P3's player-side instance is a set
of labeled members rather than a named struct, and note that this is a
weaker form of the principle than `OrbMoodConfig` instantiates on the
mood side. *Whether the loose-members form is acceptable P3, or whether
P3 properly requires a named struct on both sides, is the deeper
question* — answering it might change how P3 should be applied in other
modules during the rollout.

---

### Q2 — `Dn` locator family in source but not in glossary

**Surfaced.** agents/orbs audit.

**What.** `orbs.inl` contains a `DONE[orbs:D2]` tag. The glossary
defines `Pn` (pattern), `Kn` (keystone), `Ln` (local refinement) as
numbered locator families but does not mention `Dn`.

**Detail.** Three possibilities:

1. `Dn` is a real locator family (perhaps "Decision" or "Design"?) that
   exists in the seam map's tag inventory but didn't make it into the
   glossary.
2. `Dn` was an ad-hoc numbering by the original tag author that should
   be reworked into either an `Ln` locator or a bespoke descriptive one.
3. There may be other `Dn` tags scattered in modules I haven't audited
   yet.

**Tentative resolution kind.** seam-map-question (priority: low until we
see whether `Dn` recurs in other modules). If recurrence is found, this
is a glossary-correction. If `orbs:D2` is the only instance, it's a
code-edit (rework the locator).

**Watch for.** Other `Dn` tags as the rollout proceeds.

---

### Q3 — `mood:L1` referenced but unverified — RESOLVED

(See closed-items section. mood.inl audit confirmed mood:L1 was a
genuinely dangling reference; resolved by adding `SEAM[mood:L1]` to
the has_anchor_ribbon check site during this pass.)

---

### Q4 — Naming consistency: `*_CEILING` constants are actually gain coefficients

**Surfaced.** pawn.inl audit; recurrence confirmed in musical.inl audit.

**What.** The pattern `1.0f + intensity * MULT` produces an output in
`[1, 1+MULT]`. Naming `MULT` "ceiling" is misleading because the actual
ceiling is `1+MULT`, not `MULT`. `MULT` is a gain coefficient.

**Status — confirmed as a recurring family.** Five known instances:

- `pawn.inl::AURA_EXPAND_GAIN` (renamed in pawn.inl pass)
- `musical.inl::GOL_TICK_GAIN` (renamed in musical.inl pass)
- `musical.inl::GOL_HEIGHT_GAIN` (renamed in musical.inl pass)
- `orbs.inl::ORB_SPEED_CEILING` (still uses legacy name)
- *Possibly more* — every coupling that scales an output by polyphony
  is a candidate.

**Detail.** In musical.inl, the `GOL_TICK_GAIN = 3.0f` produces
`1 + 1.0 × 3.0 = 4×` slower zones at full music; the older comment said
"up to 3× slower" which was incorrect. Same drift pattern as
orbs.inl — the name hides the math, the comment trusts the name. The
gain framing is honest about what the math does and avoids the
silent off-by-one in commentary.

**Tentative resolution kind.** code-edit + organization-guide-correction.

- code-edit: rename `ORB_SPEED_CEILING` → `ORB_SPEED_GAIN` in orbs.inl,
  update its comment to "4× baseline at full music." Small mechanical
  change. Fits naturally into a future orbs.inl pass.
- organization-guide-correction: add "gain-vs-ceiling" naming
  convention to the comments-as-policy section, with the formula
  `1 + intensity * GAIN` as a worked example. The convention is now
  documented enough across modules to deserve a single source of
  truth.

**Watch for.** Other `_CEILING` constants in subsequent modules. Also
watch for ad-hoc inline gain expressions (`* 2.0f` and `* 3.0f` in
formulas) that should be promoted to named constants.

---

### Q5 — Input-vocabulary fluidity: temporary bindings, durable functions

**Surfaced.** Jean's note during pawn.inl audit. Applied in pawn.inl
and musical.inl.

**What.** Many of the program's current input bindings — numpad keys,
function keys (F1/F2/F3), Caps Lock, and other diagnostic toggles —
are temporary scaffolding. The *functions* those keys control will
persist (and may be driven by music or other inputs in the future);
the *keys* are placeholder bindings during development.

**Framing.** The program is a synth for visuals — a tunable, automatable
system serving an artistic vision. The temporary nature of input
bindings is part of a deliberate fluidity in the codebase, not an
oversight. This reinforces the importance of the rollout work: good
organization and surfaced tunables make it possible for the artistic
vision to drive the system through whatever input vocabulary is current.

**Status — convention now applied in two modules.** pawn.inl and
musical.inl both carry the temporary-binding marker now. Remaining
modules with input bindings: agents.inl (Caps Lock, F1/F2/F3),
orbs.inl (KP_0/KP_8/KP_DECIMAL/KP_9), input.inl (the central
dispatch — needs the framing in its header). All should pick up the
convention as they're audited.

**Implications for module audits.**

- Comments on input bindings should explicitly name them as temporary.
- The Public surface blocks should name the **function**, not the
  binding, as the durable surface. The current agents.inl block reads
  `try_possess_nearest(queue) — Caps Lock`; that's fine because the
  binding is parenthetical context, not the surface itself.
- The "player-vs-mood" P3 split is grounded in *input cadence*, which
  changes meaning slightly under this framing: the durable distinction
  is between "things the player or upstream automation can mutate
  between mood transitions" and "things mood entry sets." Player-as-
  human-on-numpad and player-as-music-driven-automation are the same
  *cadence*, even though one is current and one is future.

**Tentative resolution kind.** organization-guide-correction
(small) — add a brief "input fluidity" note to the comments-as-policy
section, naming the convention. Plus a tag-harmonization sweep item:
scan all binding-mention comments and add the temporary marker where
missing.

**Higher-order observation.** This framing — "synth for visuals,
tunable and automatable" — is the *why* behind the organizational
work itself. The rollout is making the parameters legible and accessible
because the program will eventually be *played*, not just configured.
That's worth surfacing in the organization guide's preamble (or the
seam map's introduction) so future readers understand the work's
purpose, not just its mechanics.

---

### Q6 — `auraHeightEnabled_` provenance — RESOLVED

(See closed-items section. Now player state, regrouped in pawn.inl.)

---

### Q13 — A–Z keys are claimed by the analysis layer above the cartridge

**Surfaced.** input.inl audit. Promoted from incidental observation to
canonical framing.

**What.** The boot banner reads "A-Z=piano keys" — the analysis layer
above the cartridge consumes letter keys as MIDI piano notes. This
is why the diagnostic surface uses function keys (F1–F7); letter
keys cannot double as cartridge diagnostics without playing notes.

**Detail.** This is more substantial than a comment about why F-keys
exist. It's a *framing fact* about the program: the cartridge sits
under an analysis layer, and the analysis layer plays notes when you
press letters. The implication is that the program is *literally* a
synth — and pressing a letter is *literally* playing a note. The
"synth for visuals" framing in Q5 isn't metaphor; it's structural.

**Implications.**

- The Q5 framing in the organization guide should be sharpened: not
  just "tunable, automatable system serving an artistic vision," but
  *"the cartridge sits below an MIDI piano-note layer; pressing
  letters plays notes; the visualizer is being driven by music in
  the strict sense."*
- Diagnostic dispatch on function keys is a *structural choice*, not
  an aesthetic one. Future diagnostic surfaces should respect the
  same constraint.
- This framing strengthens the case for input.inl as the *home* of
  the binding-fluidity convention. The convention isn't just "keys
  are temporary"; it's "this is a synth and the keys are stand-ins
  for MIDI input."

**Tentative resolution kind.** organization-guide-correction. Add the
framing fact to the input-fluidity note.

---

### Q14 — `cycle_orb_palette` was bound to GLFW_KEY_0, not numpad

**Surfaced.** input.inl audit (positional observation, not a problem).

**What.** GLFW_KEY_0 (top-row 0) cycles the orb palette, while the
other orb cycle commands (motion rule, gesture, anchor) live on
KP_8/KP_DECIMAL/KP_9. The grouping in the original switch placed
GLFW_KEY_0 well after the numpad keys, mixed in with the F-keys.

**Detail.** Probably an artifact of when the binding was added —
GLFW_KEY_0 isn't claimed by the piano-note layer (numbers aren't
notes), so it was a free key when palette cycling needed a binding.
The numpad-orb cluster (KP_8/KP_9/KP_DECIMAL) was already full of
other orb commands, so GLFW_KEY_0 picked up the overflow.

In the restructured input.inl, GLFW_KEY_0 sits under "World / aura
toggles" because it's a top-row number key (consistent with that
group's other number keys). Conceptually it belongs with the orb
commands; positionally it doesn't fit there.

**Tentative resolution kind.** no-action (not a real problem).
Filed as positional observation. If a future binding sweep moves
all orb commands together, palette cycling should join them; until
then, the registry comment table makes the relationship visible
("0 — cycle_orb_palette") even when the dispatch organization
doesn't.

**Watch for.** When the binding remap eventually happens, this is
an opportunity to consolidate.

---

### Q17 — Lineage as a documented convention

**Surfaced.** entities.inl audit. The seven family vocabulary blocks
exhibit kinship structure that benefits from being named in source.

**What.** Some entity families are genealogically related — sibling
pairs from "design cell division" or shared design constraints.
Examples currently visible:

- **Column ↔ Antenna**: explicit sibling pair. Antenna is a design
  cell division from Column (shared ColumnTierRow shape, shared
  ActiveColumn struct, shared mesh-gen pipeline; distinct tier
  vocabulary, distinct GPU tier indices 3–5).
- **Palm / Cactus / Blade**: vegetation cluster — no piers, no
  collision solids, no heightfield contribution. Same paradigm
  (body-part bases with per-instance variance), distinct mesh
  shapes.

In the restructured entities.inl these relationships are now
captured under a "Lineage." paragraph in each family banner. The
pattern works.

**Tentative resolution kind.** organization-guide-correction. Add a
"Lineage notes" subsection to the comments-as-policy principle —
when a module hosts multiple instances of a pattern (P10 here, but
also could apply to musical modes, GoL/Pulse, etc.), explicit
genealogy comments make divergence legible. The pattern: name what
this thing shares with its siblings, name what makes it distinct.

**Watch for.** Other modules where lineage notes would help:
musical.inl's mode registry (some modes share polyphony curves,
others have their own); gol_zones.inl's Conway/Pulse split is
already framed this way.

---

### Q18 — `select_harmonic_ratio` and ratio palettes are P8 latent infrastructure

**Surfaced.** entities.inl audit; resolved in ribbon.inl pass via
explicit P8 tagging.

**What.** The `select_harmonic_ratio` function plus the
`VERTICAL_RATIOS` and `TWIST_RATIOS` palettes are authored data
and a working selector — but the selector is never called.
`fill_ribbon_selection_geometry` overrides `vertical_cycles` and
`twist_cycles` to lateral values directly (per the "overridden =
lateral" notes in RIBBON_TIERS). The harmonic-ratio system is
designed but not yet wired.

**Status.** Tagged with `SEAM[ribbon:P8]` in ribbon.inl, naming
this as latent infrastructure pending feature activation. When the
harmonic-ratio system is consumed at runtime, the per-axis ratio
palettes replace the override. Until then, the block is the
artist's note-to-self about what's coming.

**Tentative resolution kind.** no-action (currently). When the
system gets wired:

1. The `select_harmonic_ratio` function becomes a Q10 candidate
   (cumulative-weight selector — same pattern as the gol_zones
   tier picker, possibly replaceable with `seed_utils::select_tier`).
2. The "overridden = lateral" notes in RIBBON_TIERS get removed.
3. The P8 tag flips to a closed status.

**Watch for.** Activation of the harmonic-ratio feature.

---

### Q19 — Ribbon active state lives in spawn_engine.inl — RESOLVED

(See closed-items section. State and CPU mirrors migrated to ribbon.inl
during the entity_types/spawn_engine/entity_pipeline trio pass.
ribbon.inl is now a true `:complete-subsystem`.)

---

### Q20 — Ribbon "UNIQUE" framing is now stale

**Surfaced.** entities/ribbon pass; trivially closeable but worth
noting.

**What.** The original `SEAM[ribbon:taxonomy]` tag declared ribbon's
vocabulary/machinery split "UNIQUE among bespoke families." After
the entities/ribbon pass, ribbon's vocabulary lives with its
machinery in ribbon.inl, matching the gol_zones / gallery shape.
The "UNIQUE" framing is no longer accurate.

**Status.** New `SEAM[ribbon:complete-subsystem]` tag in ribbon.inl
correctly names ribbon as a peer of gol_zones and gallery. The old
"UNIQUE" wording is gone from source.

**Tentative resolution kind.** Implicitly closed — the tag was
rewritten, not strictly transferred. Filed for awareness only.

---

### Q7 — Snap-threshold constants `0.001f` / `0.999f` recur unparameterized

**Surfaced.** musical.inl audit; recurrence confirmed across pawn.inl
and musical.inl.

**What.** Both pawn.inl and musical.inl use `0.001f` as the lower-bound
"snap to zero" threshold and `0.999f` as the upper-bound "snap to one"
threshold for trajectory ramps. Comment in both says "Snap to endpoints
to avoid perpetual drift." The values are not parameterized.

**Detail.** Six instances in musical.inl alone (band motion, mode
intensities, palette drift), three in pawn.inl. The threshold is the
same across all of them because it represents the same thing:
floating-point drift below this is indistinguishable from rest.

**Tentative resolution kind.** unclear-needs-review. Two options:

1. Promote to a project-level constant (`TRAJECTORY_SNAP_EPSILON =
   0.001f`) in trajectory.inl alongside the release primitive. Pro:
   single source of truth. Con: trajectory.inl is the foundations
   layer and adding semantics ("snap to endpoints") may be the wrong
   level of abstraction.

2. Leave as-is. The pattern is consistent enough that drift is
   unlikely. Pro: simple. Con: if the threshold ever needs to change
   for one site, the others won't follow.

3. Add a `trajectory_snap_to_endpoints(value, target, epsilon)`
   helper in trajectory.inl that takes the epsilon as a parameter
   and returns the snapped value. Pro: encapsulates the policy.
   Con: changes the call shape at every site.

**Watch for.** Other modules with trajectory ramps (orbs.inl already
has them in `update_orb_coupling`). If the recurrence reaches
~10 instances, the case for option 1 or 3 strengthens.

---

### Q8 — `MMODE_NAMES[6]` is "UNUSED" — placeholder for a future mode

**Surfaced.** musical.inl audit.

**What.** The `MMODE_NAMES` array has 8 entries. Index 6 contains the
literal string `"UNUSED"` because `MMODE_RADIAL_PULSE` is at index 7
(separate from the intensity array). The slot at index 6 is reserved
for a future mode.

**Detail.** This is a textbook P8 (latent infrastructure) — slot kept
explicit so adding a new mode at index 6 doesn't require renumbering
the radial pulse mode. The DONE[musical:L2] tag mentions the off-by-one
("musical:L4 — semantic-kind off-by-one, folds into K1") but doesn't
explicitly tag the placeholder slot as P8.

**Tentative resolution kind.** code-edit (small). Add a `SEAM[musical:P8]`
tag to the `MMODE_NAMES` array explaining that index 6 is a reserved
placeholder for a future intensity-driven mode. This brings the source
into agreement with the glossary's P8 framing and makes the latent
infrastructure visible at the tag level.

**Suggested tag wording:**

```cpp
// SEAM[musical:P8] index 6 is a placeholder slot — reserved so adding
//   a new intensity-driven mode at MMODE_COUNT=6 doesn't require
//   renumbering MMODE_RADIAL_PULSE (currently 7). Latent infrastructure.
```

Could land in the next tag-harmonization sweep.

---

### Q9 — `std::cout` diagnostic logging is ad-hoc; cube_behaviors.inl shows the convergence pattern

**Surfaced.** musical.inl audit; recurrence in gol_zones.inl,
ribbon.inl. **Refined during the cube_behaviors.inl audit.**

**What.** Diagnostic stdout patterns vary across modules. Three
observed shapes:

1. *Free-form `<<` chains*: musical.inl (toggle_mmode, onset),
   ribbon.inl (commit_ribbon). One-off lines.
2. *Structured prefixed lines*: cube_behaviors.inl (every diagnostic
   path uses `[Floaters]` prefix and key=value-like format —
   `[Floaters] cube behavior: stationary`,
   `[Floaters] kite mode: ON (3 cube(s))`,
   `[Floaters] corral: 5 cube(s) gliding to ring radius 30 over 4s`).
3. *Named diagnostic functions*: agents.inl (dump_agent_census,
   structured tabular output with field names).

**Detail.** cube_behaviors.inl is the transitional good-practice
example. Every print uses a consistent module prefix (`[Floaters]`),
named fields, and human-readable units. It's not a structured-data
format like agents.inl's census, but it's far more disciplined than
the free-form chains in musical/ribbon. It also reads well in a
real session — you can grep `[Floaters]` and see the full session's
diagnostic narrative.

**Tentative resolution kind.** unclear-needs-review (low priority).
The *prefix + key:value* shape from cube_behaviors.inl could become
the project-wide convention for event-driven diagnostics, with
agents.inl-style census functions for on-demand state dumps.
Worth raising when the exhibition-guard discussion happens (DIAG_*
defines, etc.).

**Watch for.** Other modules with `std::cout` or other ad-hoc
logging during remaining audits.

---

### Q10 — Tier-picker logic duplicated across modules; seed_utils::select_tier is the consolidation target

**Surfaced.** gol_zones.inl audit. **Confirmed during seed_utils.inl audit.**

**What.** Multiple modules contain hand-rolled cumulative-weight
tier-picker loops with the same shape as `seed_utils::select_tier`:

```cpp
// seed_utils.inl
static uint32_t select_tier(uint32_t seed, uint32_t tier_prop,
    const float* weights, uint32_t count) {
    float roll = cpu_hash_f(seed, tier_prop);
    float cumul = 0.0f;
    for (uint32_t t = 0; t < count; t++) {
        cumul += weights[t];
        if (roll < cumul) return t;
    }
    return count - 1;
}
```

**Status — confirmed as the consolidation target.** Tagged in
seed_utils.inl with `SEAM[seed_utils:Q10-target]` so the function is
discoverable as the canonical replacement.

**Detail.** Known call sites of the hand-rolled loop:

- gol_zones.inl: `select_gol_for_patch` (Conway tier picker, plus
  Pulse tier picker). Two duplicate loops in one function.
- gallery.inl: `select_gallery_for_patch` (snapshot-only / mixed /
  authored-only weights via `site_roll`).
- Possibly entity_pipeline.inl per-family adapters (worth checking).

**Tentative resolution kind.** code-edit (mechanical sweep). Best
done as a single dedicated pass after the spine audit completes —
some call sites may have small variations (theme bias, post-roll
filtering) that need preserving.

**Watch for.** Other hand-rolled `cumul += weights[i]` loops
surfacing during the spine audit.

---

### Q11 — Gain constants reach across modules

**Surfaced.** gol_zones.inl audit (positive observation, not a problem).

**What.** `GOL_TICK_GAIN` is declared in musical.inl (where the
mode it drives lives) and consumed in gol_zones.inl (where the
parameter it scales lives). This is the first cross-module reach
of a gain constant in the rollout. It works cleanly because the
include order puts musical.inl ahead of gol_zones.inl.

**Detail.** The principle this exposes: gain constants belong with
the *coupling that drives them*, not with the *parameter they scale*.
That is, `GOL_TICK_GAIN` lives with `MMODE_GOL_TEMPO` because it's
the gain *for that coupling*, even though the value it scales is in
gol_zones.inl. This is consistent with the broader architecture
(modules hold what things *are*; the spine holds when things
*happen*) — the coupling layer holds the *intensity-to-gain*
mapping; the consumer just reads the result.

**Tentative resolution kind.** organization-guide-correction
(small) — extend the Q4 gain-vs-ceiling note with a residency rule:
"Gain constants live with the coupling that drives them, not with
the parameter they scale. The producer module declares; consumer
modules consume across the include boundary."

**Watch for.** When auditing entity_pipeline.inl and the other
remaining modules, watch for gain-shaped scalars that should be
relocated next to their driving coupling.

---

### Q12 — "NEW FINDING (Ch. 15 chunk N)" wording is an authoring artifact

**Surfaced.** gol_zones.inl audit.

**What.** The original `SEAM[gol_zones:dual-algorithm]` tag opened
with "NEW FINDING (Ch. 15 chunk 2): block houses TWO algorithms..."
That phrasing is an authoring artifact from when the seam map was
being written — it was a finding *as it was being added* to the
seam map, not an observation the source itself needs to carry
forever.

**Detail.** Stripped during the gol_zones.inl pass and rewritten as
a clean SEAM observation: "this module houses two algorithms, gated
by `PULSE_ALGORITHM_CHANCE`." The information is the same; the
"NEW FINDING" framing isn't.

There may be other tags in the codebase carrying similar authoring
artifacts — phrases like "NEW FINDING," "RECENTLY DISCOVERED,"
"Ch. N chunk M," or other in-progress meta-commentary. These should
be cleaned up when their host modules are audited, or as a
tag-harmonization sweep.

**Tentative resolution kind.** code-edit (sweep). Grep candidates:
`NEW FINDING`, `Ch\. [0-9]+ chunk`, `RECENTLY`, `chunk [0-9]+`. Each
hit is a candidate for the same kind of cleanup.

**Watch for.** During remaining module audits, similar phrases.

---

### Q21 — Ribbon CPU-mirror functions are P8 latent infrastructure

**Surfaced.** spawn_engine.inl audit during the trio pass.

**What.** Three functions in the (now-migrated) ribbon block —
`ribbon_spine_at_cpu`, `ribbon_tangent_cpu`, `ribbon_rotor_diag` —
plus the supporting `RotorDiag` struct are *defined but never
called anywhere in the codebase*. Same shape as `select_harmonic_ratio`
in the Q18 finding: authored, working, but unconsumed.

**Detail.** These are CPU mirrors of WGSL functions presumably
intended for future picking, queries, or diagnostics that need to
evaluate the ribbon spine on CPU. The ribbon system is GPU-driven
today; if a future feature needs to know "what's the world position
of ring N at time T?" outside a compute pass, these would be the
implementation.

**Status.** Tagged `SEAM[ribbon:P8]` in ribbon.inl during the
trio pass and grouped under their own `═══ CPU MIRRORS (P8 — latent) ═══`
section. Same family as the harmonic-ratio P8 (Q18) — both authored
infrastructure waiting for consumers.

**Tentative resolution kind.** no-action (currently). When wired,
the constraint becomes "keep aligned with the WGSL implementations."
The grouping under P8 makes that constraint discoverable.

**Watch for.** Activation by future picking/query/diagnostic features.

---

### Q22 — Authoring-artifact phrases recur across multiple modules

**Surfaced.** Recurring across Q12, Q15, the entities/ribbon pass,
and now the trio pass. Promoted from per-instance findings to a
documented pattern.

**What.** A class of comment text that describes the *act of having
done the work*, not the *current state of the code*. Examples seen
during the rollout:

- `// "NEW FINDING (Ch. 15 chunk 2): block houses TWO algorithms..."` (gol_zones, fixed)
- `// "Verified by Ch. 13 chunk-3 read"` (entities, fixed)
- `// "First family migrated to the generic pipeline"` (entity_pipeline, 8 instances, fixed)
- `// "Phase 2 extraction"` (spawn_engine, fixed)
- `// "What to delete after blade migration is validated"` (entity_pipeline, fixed — stale completion checklist)
- `// "Stripped Phase 2.8 references"` (orbs, fixed)
- The eight repeated `entities:K1 (Option B): per-family tier struct...` paragraphs in entity_pipeline (fixed)

**Detail.** Two subtypes worth distinguishing:

1. *Mid-authoring artifacts*: "NEW FINDING," "Phase N extraction,"
   "Verified by Ch. X" — text that records the moment of a discovery
   or the step in a refactor. Useful in an authoring journal; noise
   in production source.
2. *Stale completion checklists*: "What to delete after X is
   validated" — TODO-list-style content that becomes stale the
   moment the work is done. Worse than mid-authoring artifacts
   because it actively misleads readers about what's outstanding.

**Tentative resolution kind.** organization-guide-correction. Add an
"authoring artifacts" subsection to the comments-as-policy principle.
The rule: comments describe *present behavior*, not *the path that
got us here*. Paper trail belongs in tags (DONE[…]) or in commit
messages, not in line comments.

A grep-able sweep across the codebase for the patterns above is a
candidate batch fix. Search terms: `NEW FINDING`, `Ch\. [0-9]+`,
`Phase [0-9]+`, `chunk [0-9]+`, `migrated to`, `What to delete`,
`extraction (target|landed|complete)`.

**Watch for.** Each remaining module audit; the patterns are
distributed.

---

### Q23 — Indentation policy: un-indent if currently indented; leave if at column 0

**Surfaced.** entity_pipeline.inl audit (the first module already at
column 0).

**What.** Most modules audited so far were indented at 12 spaces
(as if still inside the class body where they're textually
included). The un-indent has been part of every Phase A so far.
entity_pipeline.inl was already at column 0 — no un-indent needed.

**Detail.** The rule is: column 0 is the target. Files that arrive
at the audit pre-aligned don't need the un-indent step. This is a
small organization-guide note worth making explicit so future
audits don't blindly run an un-indent script and corrupt files
that were already correct.

**Tentative resolution kind.** organization-guide-correction. Add
a one-line note: "Un-indent if the file is currently indented as
class-body context; leave alone if already at column 0. The target
is column 0 either way."

---

### Q24 — Per-family `compute_colors` shows tier coupling that violates the generic shape

**Surfaced.** entity_pipeline.inl audit (cross-cutting observation).

**What.** Several family adapters override `compute_colors` not to
do exotic color logic per se, but to read `color_var` from the
family's `*_TIERS` table directly (e.g. blade reads
`BLADE_TIERS[inst.tier_idx].color_var`). The comment in blade reads:
"color_var is a per-tier constant on the legacy struct. Read it
directly until BLADE_TIERS is retired."

**Detail.** This is a structural observation, not a bug. The generic
pipeline's `compute_colors` reads `variance` from each `ColorPartDef`,
but families that need per-tier variance need a per-tier value
instead. Their workaround: override `compute_colors`, read the tier
table directly. This works but it means "compute_colors is generic"
is partially false — at least three families (Blade, Cactus,
likely others) have their own.

There are two possible directions:

1. Add per-tier color variance to `TierProfile` or the per-family
   `<Family>TierRow` so the generic compute_colors can use it.
2. Accept the override as the convention — families that need tier-
   coupled colors implement their own compute_colors function.

**Tentative resolution kind.** unclear-needs-review. Touches the
type definitions (entity_types.inl) and could simplify several
adapters. Worth discussing as a design question rather than a
mechanical fix.

**Watch for.** When entity_types.inl is next revised; or when the
generic-pipeline shape is otherwise reconsidered.

---

### Q25 — `FloatingEntityProp` is a legacy name held for seed stability

**Surfaced.** floater_vocabulary.inl audit.

**What.** The sphere property index registry is named
`FloatingEntityProp` (range 100–126). The cube property registry is
named `CubeEntityProp` (range 130–156). The asymmetry comes from
history: the original design had both spheres and cubes share a
"FloatingEntity" profile struct. After they were split into separate
families, the cube-side struct was correctly renamed
`CubeEntityProp`, but the sphere-side struct kept the legacy name.

**Detail.** Renaming `FloatingEntityProp` to `SphereProp` for symmetry
with the entities.inl per-family naming convention (`ArchProp`,
`ColumnProp`, `PalmProp`, etc.) would touch every consumer site
(entity_pipeline.inl's SPHERE_PARAM_DEFS array references
`FloatingEntityProp::*` for every sphere parameter). The rename
itself is mechanical, but **changing the struct name does NOT
change the seed inputs** — the property index *values* (100, 101,
102…) are what hashes consume, not the struct or member names.
The rename would be safe.

The current floater_vocabulary.inl header notes the struct as
"legacy 'FloatingEntity' name preserved for seed stability — renaming
the struct would change hash inputs." That comment is **incorrect**
— hash inputs are the integer values, not the struct/member names.
The rename can proceed without affecting determinism.

**Tentative resolution kind.** code-edit (mechanical). Rename
`FloatingEntityProp` → `SphereProp` across all consumers. While doing
so, also fix the misleading "preserved for seed stability" comment
in floater_vocabulary.inl. Touch list:

- floater_vocabulary.inl: declaration site
- entity_pipeline.inl: SPHERE_PARAM_DEFS, sphere adapter functions
- Any other site grep would find

**Watch for.** Other files referencing `FloatingEntityProp` during
later audits or grep sweeps.

---

### Q26 — cube_behaviors.inl is an alignment exemplar

**Surfaced.** cube_behaviors.inl audit. Positive observation, not a
problem.

**What.** Of the modules audited so far, cube_behaviors.inl arrived
already aligned with the conventions the rollout has been
establishing — Public surface block (with the explicit "function
keys; chosen to avoid the A-Z piano range" rationale, anticipating
Q13 by months), three-registry framing, four-axis vocabulary box,
tuning console, sub-divider hierarchy, named WGSL kernel constants,
static_assert hygiene, structured diagnostic prints with consistent
prefix.

**Detail.** Two takeaways:

1. The conventions the rollout is documenting are *discoverable* —
   cube_behaviors.inl's author worked them out independently. They
   reflect natural good organization, not arbitrary house style.
2. cube_behaviors.inl is the cleanest reference for what a
   well-organized module looks like in this codebase. Future
   contributors (or future Claude sessions) looking for "what does
   the convention look like?" should read it.

**Tentative resolution kind.** organization-guide-correction (small).
Add a "Reference exemplars" subsection to the organization guide
listing modules that demonstrate specific conventions cleanly.
cube_behaviors.inl is the strongest candidate for *all* conventions
applied together; agents.inl exemplifies the matrix-style registries
(BEHAVIOR_TIER_GAINS); musical.inl exemplifies the cross-module gain
constants pattern.

**Watch for.** Other already-aligned modules. mood.inl hasn't been
audited yet — it may also be an exemplar. gallery.inl was a partial
exemplar (Q31 below).

---

### Q27 — `WALL_ART` config struct lives outside gallery.inl — RESOLVED

(See closed-items section. Migrated during Pass 1 of the
post-rollout structural cleanup. WallArtScaleBucket + WallArtConfig
+ WALL_ART now live in gallery.inl's TUNING CONSOLE section,
declared as `SEAM[gallery:wall-art]`.)

---

### Q28 — Paintings missing from the shadow pass

**Surfaced.** Jean during gallery.inl audit, before the audit pass
started.

**What.** `draw_shadow_all` in render_passes.inl draws terrain
(LOD0 + LOD1) and shells, but not paintings. Both painting forms
that are visible in `render_main_pass` — wall frames
(`draw_wall_paintings`) and gallery frames on terrain
(`draw_gallery_frames`) — render in the main pass but cast no
shadows.

**Detail.** This is a known missing feature, flagged for awareness.
The `:dual-role` framing of gallery (outdoor terrain quads + indoor
wall frames) means *both* painting types need shadow draws when this
is addressed — they're separate render paths.

**Tentative resolution kind.** no-action (this pass). Future shadow-
system work will pick up the gap. Filed here so it's visible during
the cartridge.hpp / spine audit and during any future shadow
performance work.

**Tagged in source.** A `NOTE[gallery:shadows-missing]` lives in
gallery.inl's header.

---

### Q29 — Gallery uses literal property indices without a `GalleryProp` registry — RESOLVED

(See closed-items section. Resolved during Pass 2' of the
post-rollout structural cleanup. Four named registries —
GalleryProp, GalleryPaintingProp, WallArtProp, WallPaintingProp —
now declared in gallery.inl's TUNING CONSOLE; all 40+ literal
property index references replaced with named members.)

---

### Q30 — `pick_authored_staging` has a misleading signature

**Surfaced.** gallery.inl audit.

**What.** The function signature is:
```cpp
uint32_t pick_authored_staging(uint32_t /*seed*/, uint32_t /*prop*/);
```

Both parameters are explicitly ignored. The implementation picks the
staging slot with the lowest `disk_index` (alphabetically lowest
manifest entry not yet consumed). The signature suggests seed-based
selection; the behavior is purely deterministic alphabetical order.

**Detail.** Two callers exist (commit_gallery and place_wall_paintings),
both passing a real `seed` value and a small `prop` integer. The
parameters are entirely unused.

There are two ways to resolve:

1. *Remove the parameters.* Cleanest — signature matches behavior.
   Touches both call sites (drop the seed/prop arguments).
2. *Implement seed-based selection.* If the intent is "stable but
   not strictly alphabetical," wire up the seed. Behavior change;
   check whether anything depends on the alphabetical determinism.

**Tentative resolution kind.** code-edit (small). Recommend option 1
(remove parameters) — alphabetical order is currently working and
is desirable for the rotation cursor's interaction with the disk
manifest. The parameters were probably leftover from a refactor.

**Watch for.** Other functions with `/*unused*/`-style parameter
comments — they signal abandoned signatures.

---

### Q31 — Gallery is a partial exemplar (positive)

**Surfaced.** gallery.inl audit. Positive observation alongside Q29.

**What.** Gallery exhibits the cube_behaviors.inl-style structured
diagnostic pattern across all four prefixes used in the module:
`[Photographer]`, `[Gallery]`, `[Authored]`, `[WallPainting]`. Each
diagnostic line is consistent, prefix-tagged, and reads well in a
session log.

The module also demonstrates the `:complete-subsystem` shape clearly
with two coherent halves (outdoor + indoor) sharing infrastructure —
a useful reference for what `:dual-role` looks like in practice.

**Detail.** Gallery is a partial exemplar because it has Q29 (no
property registry) and Q27 (misplaced WALL_ART) holding it back from
full alignment. But its diagnostic discipline and dual-role framing
are model behavior.

**Tentative resolution kind.** organization-guide-correction (folds
into the Q26 "Reference exemplars" entry). When the exemplar list
is written, gallery.inl is worth listing for: structured diagnostic
prefixes, `:dual-role` framing, `:complete-subsystem` with explicit
"Two halves" header.

---

### Q32 — Cross-file tag references without destination tags

**Surfaced.** mood.inl audit. The bigger pattern that subsumed Q3.

**What.** Multiple files reference mood-internal locations by tag
name (`mood:K1`, `mood:K3`, `mood:K4`, `mood:L1`), but mood.inl
itself didn't declare any of these tags. The convention has been
asymmetric: outbound references (file A says "see mood:K3") existed
without inbound declarations (mood.inl declaring its own K3 anchor).

**Detail.** The audit found:

- `SEAM[mood:K4]` referenced in ribbon.inl (the dual-entry pattern)
- `DONE[mood:K3]` referenced in musical.inl (reset_musical_couplings extraction)
- `mood:L1` referenced in orbs.inl (has_anchor_ribbon flag)
- `mood:K1` referenced conceptually in the glossary

mood.inl carried only `DONE[mood:K2]` (the apply_mood split). All
other K/L tags were "documentation by external reference" — readers
following the trail from orbs.inl found nothing in mood.inl to land
on.

**Resolution.** Added during this pass:

- `SEAM[mood:K1]` — the single canonical mood entry point at
  apply_mood.
- `SEAM[mood:K3]` — anchor at the reset_musical_couplings call site.
- `SEAM[mood:K4]` — anchor at apply_mood_anchor_ribbon (the
  dual-entry second site).
- `SEAM[mood:L1]` — anchor at the MoodProfile.has_anchor_ribbon
  field reference.

Plus inline anchors at the K3 and K4 call sites in apply_mood's
orchestrator body so a reader scanning the function sees the
keystone references at the line they apply.

**Tentative resolution kind.** organization-guide-correction.
Document the convention: **when other files reference your tags by
name, you must declare them in your file.** The trail must end
somewhere. Sub-rule: a SEAM tag in file A pointing at "mood:K3"
implies a SEAM tag exists in mood.inl pointing at the actual code
site. If there's no destination tag, the reference is dangling.

This convention also subsumes Q22 (authoring-artifact strip) at a
deeper level — references must resolve to *current* code, not to
the historical moment when the cross-reference was authored.

**Watch for.** Other cross-file references during remaining audits.
Likely candidates: cartridge.hpp may carry many tagged anchors;
other modules' SEAM tags should resolve to those.

---

### Q33 — `apply_mood` is a K2-split exemplar

**Surfaced.** mood.inl audit. Positive observation alongside Q32.

**What.** `apply_mood` was originally a 217-line linear sequence
mixing 12 concerns (per DONE[mood:K2]). The split into 5 named
sub-functions where the orchestrator owns only ordering is now an
exemplar of the K2 split convention.

**Detail.** The structure:

```cpp
void apply_mood(uint32_t mood, wgpu::Queue& queue) {
    // ... activate-mood bookkeeping ...
    apply_mood_lighting(m, queue);
    apply_mood_spot_lights(m, queue);
    apply_mood_indoor_shell(m, queue);
    apply_mood_band_motion();
    reset_musical_couplings(queue);
    apply_mood_anchor_ribbon(mood, queue);
    configure_orbs(ORB_MOOD_TABLE[mood], queue);
}
```

The orchestrator reads as a recipe — each line is one named
concern. Order is preserved verbatim from the pre-split sequence,
so the K2 refactor is purely organizational.

**Tentative resolution kind.** organization-guide-correction (folds
into the Q26 "Reference exemplars" entry). When the exemplar list
is written, `apply_mood` is the canonical "long function → named
sub-function suite" example. The pattern: when a function exceeds
~150 lines mixing distinct concerns, split into named sub-functions
that the original now orchestrates. Names matter — "apply_mood"
stays; the new functions are `apply_mood_lighting`,
`apply_mood_spot_lights`, etc., readable as a sentence.

---

### Q34 — Duplicate copy-pasted comment line in mood.inl

**Surfaced.** mood.inl audit.

**What.** Lines 766-767 of the original mood.inl contained the same
comment twice:

```cpp
// --- Force-spawn the guaranteed back-portal ---
// --- Force-spawn the guaranteed back-portal ---
void force_spawn_back_portal(wgpu::Queue& queue) {
```

**Resolution.** Trivial fix during the pass — collapsed to a single
sub-divider with an expanded explanation.

**Tentative resolution kind.** no-action (closed during pass).
Filed for awareness — the kind of staleness only catchable by
careful reading. Worth a one-time grep across the codebase for
similar duplicates (e.g., adjacent identical comment lines).

---

### Q35 — mood.inl diagnostic prefixes (positive)

**Surfaced.** mood.inl audit. Positive observation alongside Q33.

**What.** mood.inl uses five distinct diagnostic prefixes —
`[Lighting]`, `[Mood]`, `[Shell]`, `[Portal]`, `[World]` — each
consistently applied at its emitting site. Same shape as
cube_behaviors.inl, gallery.inl, and the other Q31-family
exemplars.

**Detail.** Adds another data point to the Q9 / Q31 thread: the
prefix-tagged structured-line diagnostic shape is not just present
in cube_behaviors.inl, it's the *common* practice across the
codebase. The free-form chains in musical.inl (Q9's original
example) are the outlier, not the norm.

**Tentative resolution kind.** organization-guide-correction (folds
into Q26 reference-exemplar list). Updates Q9 toward closure: the
"prefix + structured line" pattern is already de facto convention.
musical.inl's free-form chains can be brought into line as a
simple drive-by edit during a future maintenance pass.

---

### Q36 — `select_tier_biased` residency unknown — RESOLVED

(See closed-items section. cartridge.hpp audit confirmed
select_tier_biased lives at line 2103 of cartridge.hpp. It depends
on private spine state — popBatch_, population_scale_tendency(),
tier_scale_character — and cannot migrate to seed_utils.inl. The
function correctly stays in the spine.)

---

### Q37 — `ground_architecture.inl` is a foundations exemplar

**Surfaced.** ground_architecture.inl audit. Positive observation,
not a problem.

**What.** Of the 19 modules audited so far, ground_architecture.inl
is the cleanest exemplar of the "foundations module with compile-time
relational integrity" pattern. Its distinctive contributions:

- Matrix-shaped registries (`CONTRIBUTOR_DAG[]`, `POLICIES[]`) with
  proper column-aligned comments
- The `ASSERT_POLICY_DAG_CLOSED` macro pattern — compile-time
  validation of relational invariants over a registry table. The
  inability to invoke a member constexpr from a class-body
  static_assert is documented inline; the macro is the principled
  workaround.
- Already at column 0 (no class-body indentation)
- No authoring artifacts

**Detail.** Adds another data point to the Q26 reference-exemplar
thread. The "compile-time relational integrity" pattern is rare —
it's the kind of thing other modules don't do. ground_architecture
is the exemplar specifically for *enforcing relationships between
registry entries at compile time*, where as cube_behaviors.inl is
the exemplar for *all the standard conventions applied together*,
mood.inl::apply_mood is the K2-split exemplar, gallery.inl is the
:dual-role exemplar.

**Tentative resolution kind.** organization-guide-correction (folds
into Q26 reference-exemplar list). When the exemplar list is
written, ground_architecture is worth listing for: matrix-shaped
registries with header comments, ASSERT_*_CLOSED macro pattern for
compile-time relational integrity, and as a textbook P9 instance.

---

### Q38 — Integration-glue pattern (P12) deserves naming

**Surfaced.** cartridge.hpp audit. Recurrence with entity_pipeline.inl's
per-family dispatch wrappers.

**What.** cartridge.hpp contains a ~400-line block of dispatch
wrappers — `dispatch_evict_<family>`, `dispatch_prepare_mesh_<family>`,
`dispatch_mesh_gen_<family>` — for all 12 entity families. These
wrappers exist purely to bind module functions to the function-
pointer slots of FAMILY_DISPATCH. Each wrapper is small (typically
3-12 lines), uniform in shape, and lives in cartridge.hpp because
that's where the dispatch table is.

The same pattern recurs in entity_pipeline.inl, where each of the
8 generic-pipeline families contributes 3 dispatch wrappers
(dispatch_select_<family>_generic, etc.). Same shape, same role
(integration glue), different file — because the entity_pipeline
families share enough machinery to live in one module.

**Detail.** This is a distinct pattern from P11 (templated active-
array helper) and P10 (per-family vocabulary block). The defining
characteristics:

- Per-family wrapper functions, all of the same uniform shape
- Pure binding glue — no domain logic
- Lives near the dispatch table that consumes them
- Adding a family means adding N wrappers (predictable cost)

Worth promoting to a documented pattern in the v1.3 glossary as
**P12 — "integration glue: per-family wrappers binding modules to
a dispatch table"**, with cartridge.hpp's FAMILY_DISPATCH wrappers
as the canonical instance and entity_pipeline.inl's per-family
dispatch wrappers as the sibling pattern at the module level.

This naming exposes what happens when a new family is added: known
work in three places (the family's owning module, the wrapper block,
the dispatch table row) rather than scattered ad-hoc work.

**Tentative resolution kind.** organization-guide-correction (small).
Add P12 entry to the v1.3 glossary; tag the cartridge.hpp wrapper
section with `SEAM[spine:P12]` (currently tagged `SEAM[spine:K2-related]`
and `SEAM[spine:family-dispatch]` — adding P12 makes the pattern
discoverable from the glossary).

**Watch for.** Other instances of "per-X wrapper binding to a
dispatch table" — this might recur elsewhere (input.inl's keypress
dispatch? render_passes.inl's per-family draw wrappers?).

---

### Q39 — Ch.N chunk M paper-trail subspecies

**Surfaced.** cartridge.hpp audit. Subspecies of Q22 (authoring
artifacts) deserving its own watch-for.

**What.** Q22 documented the broad pattern of authoring-artifact
phrases — "Phase N", "NEW FINDING", "First family migrated", etc.
The cartridge.hpp audit surfaced a distinct subspecies: comments
referring to **the seam-map authoring chapters and chunks**:

- `// SEAM[spine:per-mood-data] new finding (Ch. 15 chunk 1)`
- `// SEAM[spine:K2-related] ... New finding (Ch. 15 chunk 1)`
- `// Per Ch. 15 of the seam map`
- `// Three banner-only modules and three specialized-family blocks
   live inline inside this file (Ch. 12.A-E and Ch. 13)`

These reference the development chapters of the seam map document,
not the implementation. They're paper trail in a stronger sense
than "Phase N": **they record the moment the comment was authored
within the seam-map's development process**, which is meaningless
to anyone reading the code today.

**Detail.** The general rule (already documented as Q22) is:
*comments describe present behavior, not the path that got us
here*. The Ch.N chunk M form is a particularly common path-marker
in this codebase because the seam map's development was structured
into named chapters. Worth flagging as a specific watch-for during
remaining audits and during a future grep sweep.

**Tentative resolution kind.** organization-guide-correction (folds
into Q22). Augment Q22's grep-able sweep recipe to include
`Ch\. [0-9]+`, `chunk [0-9]+`, `(seam map)`, and `seam-map [Cc]hunk`
patterns.

**Watch for.** Other Ch.N chunk M references during remaining
audits (likely candidates: state.hpp, renderer.hpp, render_cartridge.hpp).
The pattern recurs because the seam-map document was a fertile
authoring source, but the references should resolve to actual code
locations or be stripped.

---

### Q40 — §-numbered section ordering: file-position vs topic

**Surfaced.** world.wgsl audit.

**What.** world.wgsl had `§7.5 GPU FRUSTUM CULLING` *numbered* in the
§7 (compute) family but *located* at line 7749 — between §8.0
(photographer) and §8.1 (gallery frame rendering). The misalignment
breaks file-position navigation: a reader scrolling past §8.0 hits
§7.5 unexpectedly, and a reader searching for §7.5 by number would
expect it before §8 starts.

**Detail.** The cause is reasonable — frustum culling lives near
photographer and gallery for bind-group locality (shared compute
groups, neighboring binding ranges). The fix in this pass: renumber
to §8.0.5 (matching file order) with a NOTE explaining why it lives
in §8 territory rather than moving the 110-line block. Moving WGSL
code is risky (forward-reference rules, bind-group layout
expectations).

The general principle this surfaces: **section numbers in WGSL
files reflect file-position, not topical-grouping**. Topical
grouping is captured in the SECTION MAP at the top of the file
(§7 = compute, §8 = gallery), but the granular numbering must
follow the file's actual order — otherwise navigation by number
breaks.

**Tentative resolution kind.** organization-guide-correction
(small). Document the section-numbering principle for WGSL files
specifically: §-numbers are positional. If a topical sibling needs
to live elsewhere for binding-locality or forward-reference
reasons, give it the destination's positional number with a NOTE
that names the topical home.

Closed during this pass; the §8.0.5 NOTE explains the placement.

---

### Q41 — Binding 144 cleanup requires CPU/GPU coordination

**Surfaced.** world.wgsl audit.

**What.** Binding 144 (`photo_patch_instances`, an `array<PatchInstance>`
storage buffer in the photographer compute) is currently unused —
sample_terrain_y_at moved to reading patch_grid at binding 152.
The binding is retained only so the photographer + placement
compute bind groups keep their layouts unchanged.

Removing it requires a coordinated edit across two files:
1. Remove the binding declaration in world.wgsl (this file)
2. Remove binding 144 from the photographer + placement layouts
   and bind groups in state.hpp (CPU side)

**Detail.** Tagged in source as `TODO[seam-map:cleanup]` with the
two-side coordination explained. Same general shape as Q27
(WALL_ART residency) and Q-closed-1 (moodAllowsMusicalModes_) —
work that can't be done in one file alone. Folds into the
deferred state.hpp/renderer.hpp audit.

**Tentative resolution kind.** code-edit (cross-file, deferred).
The cleanup is mechanical once both sides are touched together,
but tackling it in isolation here would create a binding-layout
mismatch. Pending the state.hpp work that's been deferred from
this rollout.

**Watch for.** Other VESTIGIAL-class bindings during the (deferred)
state.hpp audit — they typically live in pairs with the GPU
declaration here.

---

### Q42 — Paired CPU+GPU documentation as exemplar

**Surfaced.** world.wgsl audit. Positive observation.

**What.** §3.4 of world.wgsl contains a "Ground Architecture" block
(line ~2122) with Vocabulary / Contributors / Policies / Extension
patterns / Fused inline evaluations subsections. It documents the
GPU side of the ground-architecture system. Its CPU companion is
modules/ground_architecture.inl (Q37), which documents the same
concept-set with different emphasis:

- world.wgsl §3.4: contributor function bodies, policy dispatch
  functions, the `query_ground_<policy>` API
- ground_architecture.inl: ContributorId / PolicyId enums,
  CONTRIBUTOR_DAG edge list, POLICIES[] bitmasks, compile-time
  DAG closure validation

Together they form a **paired documentation pattern**: same
vocabulary, same extension steps, complementary content. Reading
one without the other would leave gaps; reading both gives the
full picture.

**Detail.** This is a sibling of the cube_behaviors.inl /
WGSL §6128 cube behavior registry pair (also paired, also
mirrored manually) and the agent registries pair (AgentBehaviorParams
in WGSL §2.1 ↔ GPUAgentBehaviorDef in state.hpp).

The pattern is rare and valuable — most CPU/GPU mirrors document
the *contract* (struct shapes, byte layouts) but not the *concept*.
Ground architecture documents the concept on both sides because
the concept itself spans both sides (DAG validation is C++, query
dispatch is WGSL, but they describe one composable system).

**Tentative resolution kind.** organization-guide-correction
(folds into Q26 reference-exemplar list). When the exemplar list
is written, the Ground Architecture pair is worth listing for:
paired CPU+GPU documentation of a system that genuinely spans
both. The pattern is a candidate for future replication where
similar systems exist (e.g., the population batch system has CPU
selection logic and GPU consumption — could it benefit from a
paired documentation block?).

Tagged in source via `SEAM[world.wgsl:ground-architecture-mirror]`
in both world.wgsl and ground_architecture.inl headers.

---

## Closed items

### Q-closed-1 — `moodAllowsMusicalModes_` misplacement (closed during gol_zones.inl pass)

**Original surfacing.** gol_zones.inl audit. The flag `moodAllowsMusicalModes_` was declared in gol_zones.inl but consumed only by musical.inl's `is_mmode_on`. A misplaced field — clearly out of place once the rollout-quality grouping by sub-divider made each module's true scope visible.

**Resolution.** Moved the field declaration from gol_zones.inl's runtime state block to musical.inl's runtime state block, under a new `── Mood gate ──` sub-divider. This put the declaration adjacent to its only consumer. Field default unchanged (`true`); semantics unchanged. The mood writer (mood.inl::apply_mood) sets it the same way regardless of where it lives.

---

### Q-closed-2 — `auraHeightEnabled_` provenance (closed during input.inl pass)

**Original surfacing.** pawn.inl audit. Couldn't tell from pawn.inl
alone whether the field was player-toggled, configuration, or a
debug switch.

**Resolution.** input.inl audit revealed it's bound to GLFW_KEY_2 —
player state. In pawn.inl, the field's declaration moved from the
"Profile" sub-divider to the "Player state" sub-divider, alongside
`auraEnabled_`. Both now carry the temporary-binding caveat per Q5.
No semantic change.

---

### Q-closed-3 — Ribbon vocabulary/machinery split (closed during entities/ribbon pass)

**Original surfacing.** entities.inl audit observed that Ribbon was
the sole entity family whose vocabulary lived in entities.inl while
its machinery lived in its own file (ribbon.inl) — flagged as
"UNIQUE" by `SEAM[ribbon:taxonomy]`. The split was suspect: it made
entities.inl's purpose mixed (vocabulary for seven generic-pipeline
families plus half of one bespoke family) and required readers to
open two files to understand Ribbon.

**Resolution.** Adopted Option C: moved Ribbon vocabulary out of
entities.inl into ribbon.inl. ribbon.inl is now a complete subsystem
(`SEAM[ribbon:complete-subsystem]`), peer to gol_zones.inl and
gallery.inl. entities.inl drops to seven clean vocabulary blocks for
the families that share entity_pipeline.inl as machinery.

Note: Ribbon's *active state* (ActiveRibbon, MAX_RIBBON_INSTANCES,
RIBBON_MAX_LENGTH, activeRibbons_, ribbonStates_) still lives in
spawn_engine.inl. A `NOTE[ribbon:state-residency]` tag in ribbon.inl
flags this as pending review when spawn_engine.inl is audited.

---

### Q15 — Ribbon orphan comment block at top of entities.inl (closed during entities/ribbon pass)

**Original surfacing.** entities.inl audit. The "Cube count: 100–400.
Cube size: pawn height (1.5) to 4× (6.0)..." block at lines 30-37
was floating between the file header and the first family banner,
describing Ribbon-specific values.

**Resolution.** Block deleted from entities.inl (Ribbon is no longer
in this file). Equivalent context absorbed into ribbon.inl's
RIBBON_TIERS section as comments next to the relevant fields
("Cube count: 100–400" near cube_count_mean, "Flying height: 50–80"
near height_mean).

---

### Q16 — `select_harmonic_ratio` redundancy concern (withdrawn during entities/ribbon pass)

**Original surfacing.** entities.inl audit suggested
`select_harmonic_ratio` was the same cumulative-weight pattern as Q10
(the gol_zones tier picker) — a possible call site for
`seed_utils::select_tier`.

**Resolution.** Investigation showed the function is *declared but
never called*. It's P8 latent infrastructure for the harmonic-ratio
feature that isn't yet wired (see Q18). Q10 consolidation does not
apply until the function is actually consumed.

---

### Q-closed-4 — Ribbon active state and CPU mirrors (closed during trio pass)

**Original surfacing.** Q19, Q21. The ribbon system's active state
(ActiveRibbon, MAX_RIBBON_INSTANCES, RIBBON_MAX_LENGTH,
activeRibbons_[], ribbonStates_[], renderedRibbonSlot_,
moodRibbonOffset_) lived in spawn_engine.inl. The CPU-mirror
functions (ribbon_spine_at_cpu, ribbon_tangent_cpu,
ribbon_rotor_diag, RotorDiag) lived there too — and turned out to
be defined-but-never-called latent infrastructure.

**Resolution.** Both moved to ribbon.inl during the trio pass.

- The active state landed under a new `═══ RUNTIME STATE ═══`
  section in ribbon.inl, sub-divided into Capacity, Per-instance
  tracking, GPU-state mirror, and Mood-5 anchor offset.
- The CPU mirror functions landed under a new `═══ CPU MIRRORS
  (P8 — latent) ═══` section, tagged `SEAM[ribbon:P8]` to mark
  them as authored-but-not-yet-consumed.

ribbon.inl is now a true `:complete-subsystem` — vocabulary +
state + machinery + lifecycle in one module, peer to gol_zones.inl
and gallery.inl. Header and Public surface block updated; the
`NOTE[ribbon:state-residency]` flag from the previous audit is
gone, replaced with the complete-subsystem framing.

spawn_engine.inl drops the ribbon-specific block entirely (~108
lines net). The `// ─── Ribbon Lifecycle (patch-based dispatch
pipeline) ───` heading and its associated stray "Check a 3×3
neighborhood of ribbon cells around the pawn" comment fragment
both removed; the comment was a leftover from a previous version
that no longer applied.

The migration also closed Q21 (CPU-mirror P8 status — now tagged
in source).

---

### Q-closed-5 — `mood:L1` dangling reference + missing K-tags (closed during mood.inl pass)

**Original surfacing.** Q3 (orbs.inl referenced mood:L1 which couldn't
be verified). Expanded during the mood.inl audit into Q32 — multiple
external references to mood-internal tags (mood:K1, K3, K4, L1) with
no declarations in mood.inl.

**Resolution.** Four SEAM tags added to mood.inl during this pass:
SEAM[mood:K1] (canonical mood entry point at apply_mood),
SEAM[mood:K3] (reset_musical_couplings call site),
SEAM[mood:K4] (apply_mood_anchor_ribbon — dual-entry site),
SEAM[mood:L1] (has_anchor_ribbon flag check).

Plus inline anchors at the K3 and K4 sites in the apply_mood
orchestrator body so a reader following the call list sees the
keystone references at the line they apply.

The orbs.inl reference to mood:L1 now resolves to a real tag in
mood.inl. Q3 is closed by virtue of mood:L1's declaration being added.

The broader convention this surfaces is captured as Q32
(organization-guide-correction): when other files reference your
tags by name, you must declare them in your file.

---

### Q-closed-6 — `select_tier_biased` location resolved (closed during cartridge.hpp pass)

**Original surfacing.** Q36 — seed_utils.inl audit noted that
`select_tier_biased` was referenced from ribbon.inl and
entity_pipeline.inl but did not live in seed_utils.inl despite
being a clear sibling of `select_tier`. Two possibilities:
misplacement (should move to seed_utils) or intentional separation
(needs spine state).

**Resolution.** cartridge.hpp audit located the function at line
2103 of cartridge.hpp, inside the patch system area. Its
implementation reads three pieces of private spine state:

- `popBatch_` (PopBatchMode, scale_n)
- `population_scale_tendency()`
- `tier_scale_character(family, t)`

These are population-batch-system internals not visible to
seed_utils. The function correctly stays in cartridge.hpp.

The v1.3 glossary's hint about `select_tier_biased` being "in
the seed_utils family" was a sibling-*pattern* observation
(it's a cumulative-weight tier picker like `select_tier`), not
a residency claim. The pattern relationship is real; the
location relationship is not.

Q36 closes as no-action. The function is correctly placed. Q10
(seed_utils::select_tier as the consolidation target for hand-
rolled tier-picker loops) remains the actionable consolidation
opportunity — separate from where biased selection lives.

---

### Q-closed-7 — `WALL_ART` migration to gallery.inl (closed during structural Pass 1)

**Original surfacing.** Q27 — gallery.inl audit identified WALL_ART
as a misplaced config: declared in cartridge.hpp's class body,
consumed only by gallery.inl::place_wall_paintings. Same shape as
Q-closed-1 (moodAllowsMusicalModes_) and Q-closed-4 (ribbon state).

**Resolution.** Pass 1 of the post-rollout structural cleanup.
Three structures moved from cartridge.hpp's WALL ART CONFIGURATION
banner area to gallery.inl's TUNING CONSOLE section (under a new
`── Wall art configuration (indoor) ──` sub-divider):

- `WallArtScaleBucket` struct
- `WallArtConfig` struct
- `WALL_ART` constexpr instance

The y-position pipeline comment block moved with the structs.
The cartridge.hpp `═══ WALL ART CONFIGURATION ═══` banner removed
entirely. The `NOTE[seam-map:future-home]` removed from cartridge.hpp.
The `NOTE[gallery:wall-art-residency]` removed from gallery.inl's
header. New `SEAM[gallery:wall-art]` tag declared in gallery.inl's
TUNING CONSOLE banner naming this as the home.

The `SEAM[spine:per-mood-data]` tag in cartridge.hpp's file header
was trimmed: WALL_ART removed from the migration plan enumeration;
remaining items (INDOOR_PALETTES, LIGHT_SCHEMES, IndoorLightProp)
are still tagged for future migration to mood.inl.

**Lexical-only move.** No symbol renames, no signature changes, no
behavior changes. References inside `place_wall_paintings`
(WALL_ART.snapshot_only_share, WALL_ART.intimate, etc.) resolve the
same way before and after, because both source and destination are
inside the Cartridge class body and gallery.inl is included after
the spine declarations it references.

**Files changed.** cartridge.hpp (4391 → 4299, −92 lines),
gallery.inl (1692 → 1780, +88 lines). Net: −4 lines (the migration
also removed two redundant comment blocks — the residency NOTE in
gallery.inl's header and a "see NOTE" pointer in the WALL PAINTINGS
banner).

This pattern (data declared in spine, consumed by one module) has
now been resolved four times: Q-closed-1, Q-closed-4, Q-closed-7,
and ORB_MOOD_TABLE (closed earlier in the rollout). Two more
instances remain — INDOOR_PALETTES + LIGHT_SCHEMES → mood.inl —
tagged at source for Pass 2.

---

### Q-closed-8 — Gallery property index registries (closed during structural Pass 2')

**Original surfacing.** Q29 — gallery.inl audit identified that
gallery was the largest bespoke subsystem without named property
index registries. Other bespoke subsystems (gol_zones, ribbon)
already had `GoLZoneProp` / `RibbonProp` structs; gallery used
literal indices throughout.

**Resolution.** Pass 2' of the post-rollout structural cleanup.
Four registries declared in gallery.inl's TUNING CONSOLE section,
under a new `── Property index registries ──` sub-divider:

- **GalleryProp** (12 indices) — outdoor patch-level seed.
  SPAWN_ROLL, PAINTING_COUNT_R1/R2/R3, FACING_ANGLE,
  CENTER_OFFSET, CENTER_ANGLE, PER_PAINTING_BASE +
  PER_PAINTING_STRIDE, MONO_TIER_ROLL, FAVORITE_TIER_PICK,
  SIZE_JITTER, SITE_TYPE_ROLL.
- **GalleryPaintingProp** (8 indices) — outdoor per-painting,
  keyed off p_seed derived from the patch seed.
  LATERAL_JITTER, DEPTH_JITTER, SIZE_JITTER_A/B/C,
  GEOMETRY_SEED, MIX_AUTHOR_ROLL, AUTH_STG_PICK.
- **WallArtProp** (8 indices) — indoor seed structure.
  SITE_SEED_OFFSET (5500), SITE_TYPE_ROLL, WALL_COUNT_ROLL,
  WALL_SHUFFLE_BASE, PER_WALL_BASE + PER_WALL_STRIDE,
  WALL_PAINTING_COUNT, PER_PAINTING_BASE + PER_PAINTING_STRIDE.
- **WallPaintingProp** (6 indices) — indoor per-painting,
  keyed off p_seed derived from the wall seed.
  Y_OFFSET_JITTER, MIX_SNAPSHOT_ROLL, HEIGHT_JITTER,
  AUTH_STG_PICK, ASPECT_ESTIMATE, SCALE_ROLL.

Three seeds are at play in gallery's hash chain — patch seed,
site_seed (indoor), p_seed (per-painting, derived from one of
two parent seeds). Outdoor and indoor per-painting contexts use
different offsets off p_seed, so they get separate registries
even though the seed type is the same. The TUNING CONSOLE comment
documents this explicitly.

**Lexical-only change.** Property index *values* unchanged — only
names that reference them changed. Hash outputs identical
bit-for-bit; world generation deterministic across the migration.
40 literal occurrences replaced with their named equivalents in
`select_gallery_for_patch`, `commit_gallery`, and
`place_wall_paintings`.

**Files changed.** gallery.inl only (1780 → 1859, +79 lines for
the four registries). cartridge.hpp untouched.

This closes the structural pattern "bespoke subsystems use named
property registries" — all three (`gol_zones`, `ribbon`, `gallery`)
now follow it.

---

### Q-closed-9 — INDOOR_PALETTES + LIGHT_SCHEMES migration to mood.inl (closed during structural Pass 3')

**Original surfacing.** SEAM[spine:per-mood-data] in cartridge.hpp's
file header. The spine declared per-mood authoring data — INDOOR_PALETTES
(8 wall+ceiling palettes), LightAnchor enum, IndoorLightProp,
LightSlotDef, AnchorRole enum, LightSchemeSlot, LightScheme,
LIGHT_SCHEMES table (4 schemes), SCHEME_WEIGHTS, SCHEME_COUNT,
SCHEME_NAMES, ANCHOR_NAMES — all consumed by mood.inl::apply_mood and
mood.inl::derive_indoor_lights. Same family as ORB_MOOD_TABLE
(migrated to orbs.inl) and WALL_ART (migrated to gallery.inl,
Q-closed-7).

**Resolution.** Pass 3' of the post-rollout structural cleanup. Both
data blocks moved into mood.inl under a new `═══ TUNING DATA ═══`
section, wrapped in a `private: ... public:` access toggle.

The toggle is necessary because mood.inl is included inside the
public: section of the Cartridge class body (alongside its functions,
which need to be callable from update()). The data declarations
should remain private — they're internal authoring tables, not part
of the cartridge's public surface — so the toggle preserves the
original access semantics. When mood.inl's include placement is
reconsidered (a separate question), the toggle can be removed.

**Files changed.** cartridge.hpp (4299 → 4116, −184 lines),
mood.inl (1188 → 1375, +187 lines). Net: +3 lines for the toggle
and the new TUNING DATA section banner.

**Lexical-only move.** No symbol renames, no signature changes, no
behavior changes. References inside derive_indoor_lights and
apply_mood resolve the same way.

**The `SEAM[spine:per-mood-data]` tag is now removed entirely from
cartridge.hpp's file header.** The pattern "data declared in spine,
consumed by one module" has been resolved five times across the
rollout: ORB_MOOD_TABLE → orbs.inl, moodAllowsMusicalModes_ → musical
(Q-closed-1), ribbon active state (Q-closed-4), WALL_ART → gallery
(Q-closed-7), and now INDOOR_PALETTES + LIGHT_SCHEMES → mood
(Q-closed-9). The pattern no longer has known instances.

---

### Q-closed-10 — Cross-file tag sweep (closed during structural Pass 4)

**Original surfacing.** Q32 — mood.inl audit identified that mood.inl
was being referenced by tag name from multiple files but didn't
declare those tags itself. The convention was implicit; Q32 made it
explicit.

**Resolution.** Pass 4 of the post-rollout structural cleanup. Python
sweep script walked all 21 audited files, parsing every
`SEAM[mod:Y]` / `DONE[mod:Y]` / `NOTE[mod:Y]` tag, separating
declarations (where mod matches the file's own module) from
references (where mod points at a different file). For each
reference, verified the destination file declares a matching tag.

**Sweep result.** 14 cross-file references resolved. 2 dangling at
first sweep:

1. `cartridge.hpp:161 DONE[musical:K2 / mood:K3]` — compound tag
   form (one bracket, two locators). The actual declarations exist
   (`DONE[musical:K2]` in musical.inl, `DONE[mood:K3]` in mood.inl)
   but the compound form is unmatchable by tooling. Fixed by splitting
   into two clean references in the cartridge.hpp comment.
2. `cartridge.hpp:3179 SEAM[orbs:P1]` — referenced as the architectural
   exemplar for per-frame coupling decomposition, but orbs.inl had
   never declared its own P1 anchor. Fixed by adding `SEAM[orbs:P1]`
   to the `update_orb_coupling` function header in orbs.inl, naming
   it as the canonical instance and pointing at the closed
   K-migrations that targeted this shape.

**Final sweep state.** 14 resolved, 0 dangling. Every cross-file tag
reference now resolves to a real declaration at its destination.

The convention is now documented explicitly in glossary §9
("Cross-file tag declarations") with the compound-tag rule and the
load-bearing-anchor placement rule.

---

### Q-closed-11 — v1.4 glossary update (closed during structural Pass 5)

**Original surfacing.** Seventeen organization-guide-correction items
across the rollout — Q4, Q5, Q11, Q13, Q17, Q22, Q23, Q26, Q31, Q32,
Q33, Q35, Q37, Q38, Q39, Q40, Q42. Each was a small documentation
amendment to the v1.3 glossary; together they would land a meaningful
v1.4.

**Resolution.** Pass 5 of the post-rollout structural cleanup.
`pattern_glossary_v1.4.md` produced as a new file in /mnt/user-data/
outputs/, extending v1.3 (936 → 1329 lines) with:

- **New §4 entry: P12** — Integration glue: per-family wrappers
  binding modules to a dispatch table. Earned by recurrence:
  cartridge.hpp's FAMILY_DISPATCH wrappers (canonical) and
  entity_pipeline.inl's per-family dispatch adapters (sibling). The
  bespoke `:K2-related` locator is retained as historical record;
  P12 is the durable name. (Closes Q38.)

- **New §11 Reference exemplars** — Curated list of files demonstrating
  conventions cleanly: cube_behaviors.inl (all conventions in one
  place), apply_mood K2-style decomposition, gallery.inl (`:dual-role`
  + `:complete-subsystem`), ground_architecture.inl + world.wgsl §3.4
  (paired CPU+GPU documentation), diagnostic prefix discipline as
  cross-module convention, and smaller-scope exemplars for P9, P1, P3,
  P10, P12. (Closes Q26, Q31, Q33, Q35, Q37, Q42.)

- **§9 Naming conventions for tunable constants** — `*_GAIN` vs
  `*_CEILING` distinction with the residency rule (gain constants
  live with the coupling that drives them). (Closes Q4, Q11.)

- **§9 Stripping authoring artifacts** — Sweep recipe with grep
  patterns for Phase N, Step N, Ch.N chunk M, (was X), (legacy X
  removed), NEW FINDING, First X migrated, Renamed from. Subspecies
  named during the rollout collapsed into one cleanup discipline.
  (Closes Q22, Q39.)

- **§9 Cross-file tag declarations** — Convention rule: declarations
  belong at the load-bearing site, compound tags must be split.
  (Closes Q32.)

- **§9 Indentation policy for source comments** — Un-indent if
  class-body-indented; leave at column 0 if already there.
  (Closes Q23.)

- **§9 Section numbering in WGSL files** — Section numbers reflect
  file position, not topical grouping. NOTE-explained placements when
  a topical sibling lives elsewhere. (Closes Q40.)

- **§9 Lineage notes within a P10 host** — When a module hosts
  multiple P10 instances and some are genealogically related
  (sibling pairs, design cell divisions), capture the relationship
  in a "Lineage" paragraph in each family banner. (Closes Q17.)

- **§9 Input-vocabulary fluidity** — Framing: input bindings are
  fluid scaffolding; the cartridge sits below an analysis layer
  that consumes A–Z as MIDI piano notes; diagnostic toggles live
  on function keys/numpad/Caps Lock. (Closes Q5, Q13.)

P12 added to §10 quick reference table.

**Pattern definitions, locator meanings, and existing conventions
from v1.3 are unchanged.** v1.4 *adds* — it doesn't revise.

---

## Summary by resolution kind

| Kind                           | Count |
|--------------------------------|-------|
| code-edit                      | 7 (Q4, Q8, Q10, Q12, Q25, Q30, Q41) |
| glossary-correction            | 1 (Q1) |
| organization-guide-correction  | 0 (all 17 closed via Q-closed-11) |
| seam-map-question              | 1 (Q2) |
| no-action                      | 4 (Q14, Q21, Q28, Q34) |
| unclear-needs-review           | 4 (Q7, Q9, Q18, Q24) |

**Closed:** 14 (Q-closed-1 through Q-closed-11 + Q15 Q16 Q20).

**Note on Q4.** Q4 appears in both the code-edit and (closed)
organization-guide-correction lines because it had two facets: the
naming convention (closed via v1.4 glossary §9) and the actual
rename of `ORB_SPEED_CEILING` to `ORB_SPEED_GAIN` in orbs.inl
(remaining as code-edit). The convention is documented; the
remaining work is a single rename.

**Open total:** 17 items remaining. The structural backlog is now
small and mostly polish:

- 7 code-edit items, mostly individual mechanical fixes
- 4 unclear-needs-review (Q7, Q9, Q18, Q24 — Q24 being the
  compute_colors design conversation)
- 4 no-action items filed for awareness
- 1 glossary-correction (Q1, OrbPlayerState mismatch)
- 1 seam-map-question (Q2)
