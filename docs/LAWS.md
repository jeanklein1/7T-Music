# THE LAWS

Repo home: `docs/`. Sibling of `7t_program_theory_v3.md` — that file is
THE LENS (how to think about the program); this one is the LAWS OF PRACTICE
(what breaks if you don't). Created by PRUNING_1 P4 because there was no
live one: `docs/past docs/cartridge_constitution.md` and
`docs/past docs/terrain_program_charter.md` are archived by their folder,
and the theory explicitly holds itself apart from law.

**What belongs here.** A rule that survives every line of code it currently
governs. If deleting the subject deletes the rule, the rule is a comment and
belongs next to its subject. If deleting the subject leaves the rule true and
the next author walking into the same trap, it belongs here.

Rules are NUMBERED and the numbers are permanent. A retired rule is struck,
not renumbered — a citation in a commit message has to keep resolving.

---

## L1 — THE ENCODING LAW

`world.wgsl` is **BOM-free, LF-terminated**, on every platform.

Enforced mechanically by `.gitattributes` (`*.wgsl text eol=lf`), which is the
authority; this rule exists so the reason survives the enforcement. A CRLF
checkout churns every line of a 12,000-line shader diff and buries the real
change. (The original reason — a sha256 sidecar on the deleted web mirror —
is gone; see `audit/past reports/WEB_PORT_LEDGER.md`. The pin stands on its own merit.)

## ~~L2 — THE FXC LAW~~ — **STRUCK 2026-08-12 (PIVOT_0)**

**Struck, not renumbered:** the number is permanent so citations keep
resolving. The full text, its three retired constraints, and what each one
still explains about the shape of `world.wgsl` are preserved verbatim in
**`audit/FXC_LAWS_RECORD.md`**.

**What replaced it.** The audience floor is WebGPU core through modern
compilers — Tint→DXC (SM6.0+), Tint→MSL, Tint→SPIR-V, naga. FXC is
**unsupported**. The native compiler is one constant, `kCompilerPlan` in
`src/console/console.hpp`; the shader's live statement of the floor is the
COMPILER FLOOR block in the `world.wgsl` banner, which is where L2's
operational home used to be.

**Why.** WALLET_0's occupier cbuffer arrays stalled `update_player_agent`
at 20,227 ms under FXC, then `D3DCompiler_47` access-violated on the next
room kernel. Jean ruled the floor up rather than the shader down.

**Do not honor L2's constraints as live.** Code already shaped by them is
not wrong — it is merely no longer required to be that shape, and undoing
any of it needs its own measurement. The agent-kernel split
(`update_player_agent` / `update_other_agents`) is the one with a price on
record: 48 s of FXC compile. Whether DXC prices it the same way is
**unmeasured** — re-witness before merging those kernels back.

**One clause of L2 was never an FXC law and survives it.** Item 4 — storage
buffers 8/stage, uniform buffers 12/stage — is WebGPU **core defaults**,
binding on every backend and every compiler. It lives in **L14**, in the
`world.wgsl` banner's budget line, and as the binding ledger's `gate`
witness. Nothing about it changed.

**The witness protocol survives too**, minus the compiler it named: a
shader-shape change is proven by witnesses, never by argument, and no
witness substitutes for another. naga is the per-commit gate; the web build
+ boot is the witness of record; each browser gates at its own.

**THE PER-COMMIT GATES, NAMED (GATE_1, 2026-08-16).** Two, and they answer
different questions:

| gate | invocation | subject |
|---|---|---|
| naga | `naga src/cartridges/the_board/realization/world.wgsl` | the WGSL module |
| console | `python3 tools/gates/console_gate/run.py` | `console.hpp` against the vendored WebGPU/GLFW surface |

The console gate is here because glaw1's translation unit is
`cartridge.hpp`, which does not include `console.hpp` — so for the life of
the tree glaw1 answered GREEN to console edits it had never read. An absent
witness wearing a witness's name is worse than no witness, because it is
counted. It compiles against the emdawnwebgpu payload pinned in
`third_party/emdawnwebgpu/PINNED.md` and never a system or emsdk copy;
its own banner states its boundary (syntax and types, no linking, no
semantics), and the web boot remains the witness of record past it.

*This strike is L15 collecting a debt on the largest referent in the tree —
see L15, and note that the three retired constraints are exactly the kind
of prose that goes on asserting itself long after its subject is gone.*

## L3 — THE MIRROR LAW

Three pairs of rooms hold the same facts in two languages, and nothing but
this rule and a witness keeps them equal:

1. `world.wgsl` §2.1 structs mirror `realization/state.hpp` **byte-for-byte**
   (`FrameSignal`, `AgentState`, `AgentBehaviorParams`, `AgentTierParams`,
   `DesignConfig`). Drift means the GPU reads different fields than the CPU
   wrote — no error, just wrong pixels.
2. `world.wgsl` §3.4 `POLICY_*_MASK` constants mirror `POLICIES[].contributors`
   in `contracts/ground_architecture.hpp`. The `POLICY_*` / `CONTRIB_*` numeric
   constants mirror the C++ enums, whose values double as table indices.
3. `world.wgsl` §1.5 deterministic-randomness helpers must produce
   **bit-identical** results to `primitives/seed_utils.hpp`. Same seed, same
   world, on both sides of the bus.

**Edit both rooms in the same commit.** The C++ room is held by `offsetof` /
`sizeof` static_asserts and `ASSERT_POLICY_DAG_CLOSED`; the WGSL room is held
by nothing the compiler can see, which is why the rule is written down.
`audit/tools/glaw2/run.py` checks the policy and contributor mirrors; the
struct mirror is checked by `tools/pruning_census.py` §3.

## L4 — THE ALIGNMENT LAW

**Every `float[3]` in `GPUDesignConfig` whose WGSL twin is `vec3<f32>` must sit
on a 16-byte boundary.**

WGSL rounds `vec3<f32>` up to align 16; C++ packs `float[3]` at align 4. A
field inserted *before* one of them shifts the C++ offset by 4, 8, or 12 and
the WGSL offset by a full 16. `sizeof` can stay equal while every following
field diverges — so the size witness does not fire, and neither does anything
else. Today the four are `sun_direction`, `fog_color`, `fade_color`,
`checker_resultant`, each already 16-aligned, held by a `static_assert` in
`state.hpp`.

Grow the struct at the **tail**, or insert a matching `_pad` so each `vec3`
keeps its boundary in both rooms. A declared pad is not waste; it is the
mirror.

## L5 — THE GROWTH LAW (how a config knob is born)

Field ORDER is the cross-room contract — `world.wgsl`'s `DesignConfig` mirrors
`GPUDesignConfig` field for field.

1. Prefer re-using a `_pad` slot inside the right `───` system group; else
   append within the group and let padding re-flow.
2. Edit **both rooms in the same commit** — same position, same type (L3).
3. Bump the `sizeof` witness. The number in the assert IS the handshake.
4. Targeted sub-writers carry `offsetof` witnesses; glaw1 re-proves them, so a
   silent shift is impossible.
5. The knob's REST value is authored at the boot block or its panel room,
   **never** in the struct.

New knobs join a cadence: dirty-config for slow dials, a bespoke hot writer
for per-frame voices. And see L4 before choosing the position.

## L6 — THE BINDING-NUMBER LAW

`realization/binding_registry.hpp` is the **single source of truth** for GPU
binding numbers, and it is the only record of them that is maintained.

1. Every bind-group layout entry and its matching group entry reference the
   **same named constant**. The "binding integer typed twice" hazard becomes an
   undefined symbol glaw1 catches, not a runtime crash.
2. Numbers are **group-scoped**, not global: `25` is `tile_grid` in
   group 0 and `shadow_map` in group 1. `g0::` and `g1::` are separate
   namespaces because a flat list would fuse distinct slots.
3. Numbers are **authored, not computed**. The `render = compute + 200` band is
   a `static_assert` witness at the foot of the registry: it CHECKS the
   authored literals and is never their source.
4. One constant per **site**, named for the WGSL variable it mirrors — not one
   per buffer. The same buffer wears several names because each name is one
   `(group, slot)`.
5. **A retired number is free.** Numbers are not reserved and comments do not
   reserve them; the registry shows what is taken, git shows what was.
   Precedent: 149 was retired and its neighbors 190/191 were reborn as the cmg
   pair. Reuse is normal.
6. The WGSL `@binding` literals stay a **mirror** — the shader cannot read a
   C++ constant. Lockstep is held by the crash-aware launch gate (bind-group
   and pipeline validation at boot), not by the compiler. The registry names
   deliberately equal the WGSL variable names so the mirror is greppable in
   both files. Closing that third copy (a generated block, or a
   token-substituted shader — feasible, since `world.wgsl` is loaded as runtime
   text) is a named follow-on, not a rule.

## L7 — THE BINDING-CLOSURE LAW

For any host that builds a pipeline against a hand-written bind-group layout,
the test that governs a resync is **binding closure**, not entry-point
existence.

The obvious check — "do the entry points the host names still exist upstream?"
— passes and means nothing. What matters is the set of `@group/@binding` slots
**reachable from each dispatched entry point**, because that is what the layout
must satisfy. An entry point that grows one new reachable binding breaks a host
whose layout is a transcription, and it breaks it at pipeline creation, at
boot.

Proven the expensive way by the deleted web port, which would have shipped a
module its layout could not satisfy while both entry points existed the whole
time — `audit/past reports/WEB_PORT_LEDGER.md`. `audit/past reports/cc4_wgsl_static_usage.py` computes
the closure. Any future port wants **generated** layouts, not transcribed ones.

## L8 — THE TOMBSTONE LAW (PRUNING_1)

**Git keeps every word.** A comment whose subject has been deleted carries no
information git does not, and it rots in a way git does not: it accretes,
it is grepped by future readers as if live, and it is indistinguishable from a
description of code that is still there.

The mechanical test, applied to any comment: **if it would still make sense
with the code beneath it deleted, it is prose** — and prose goes, unless it is
a law with no other home, in which case it comes here as a numbered rule and
leaves at most a one-line pointer behind.

What this does NOT license: deleting a comment that describes live code, a
number, an invariant, or a hazard adjacent to the thing it names. The failure
direction is under-reach. A deleted law is invisible until it bites.

## L9 — THE STATUS CONVENTION

A declaration that is not wired says so, in the declaration, in these words:

- `STATUS: REALIZED` — wired and live. **Cite the consumer.** A REALIZED tag
  with no named consumer is a claim, not a status.
- `STATUS: LATENT[name]` — a capability with a plausible future; kept and
  tagged; revive-or-rewire when this region is next worked. The bracketed name
  is the handle, so every site of one capability greps together.
- `STATUS: INTENT` — declared, zero realization yet, kept with the status
  stated. Honest futures, not lies.

Two riders, both learned the hard way:

1. **A tag is not a reprieve.** `STATUS: LATENT` and `STATUS: INTENT` are
   DELETE-AND-RECORD by default — the tag buys one reading, not permanent
   residence. `tools/pruning_census.py` §4 is the standing census of them.
2. **A tag dies with its subject.** When the declaration goes, the tag goes;
   a status describing something already deleted is a tombstone (L8), and the
   status word makes it read as live.

## L10 — BOOT IS A TRANSITION FROM NOTHING

**The world has exactly one way to come into being; the only difference
between boot and a mood change is what came before.**

Boot is not a special case that happens to resemble a transition. It IS the
transition whose prior state is empty. Wherever a transition path applies a
value, boot must reach that value **by calling the same door** — never by a
literal, an in-struct default, or a hand-copied table row that happens to
agree today.

The failure mode is silence. A hand copy and its source do not diverge with a
build error; they diverge the day someone edits one of them, and the symptom
is a boot frame that is subtly wrong in a way no test names. BOOT_ONE_VOICE
found the whole family at once: an amber sphere the CPU census did not know
existed, a test rig that outlived its own retirement condition, five
transcriptions of `MOOD_TABLE[0]` across three files, and a frustum-cull flag
whose only writer was `apply_mood` — so the world booted in `open_default`
wearing a cull setting that belonged to no mood at all.

The two doors this law currently names:

- `apply_mood` — the atmosphere, the feature gates, the orbs. Called at boot
  and at every transition, with `mood_state_.active`.
- `reset_surface` — patches, tiles, themes, queues, piers, footprints. Called
  at boot and by the transition machine. It was `teardown_surface`; the rename
  is the law made visible in the name.

**What this does NOT license.** Boot legitimately owns things a transition
never touches — buffer creation, pipeline construction, the one-shot index
generation, and the REST values of knobs no mood authors (fog is the live
example: `apply_mood_lighting` does not touch it, so the boot fog values are
correct, not residue). The test is not "did a transition write it" but
"**does a transition path author this value?**" If yes, boot calls that path.
If no, boot is its author and says so.

A corollary worth stating, because it was learned the expensive way: when boot
stops hand-copying a value, give the field a rest value that **fails loud**.
`MoodState::sun_intensity` rests at `0.0f`, not `0.8f` — if the door ever
fails to run, the sun goes out on frame 1 instead of hiding behind the value
the door would have written.

## L11 — THE PAINT ANCHOR LAW

**Evaluate a thing in the frame that owns it.**

Physics is owned by the world, so it reads the live position — the grounded
lift, the card, the terrain under the body this frame. Pigment is owned by the
body, so it reads the mesh frame: `paint_pos = vec3(world_pos.x, in.pos.y,
world_pos.z)` — mesh-authored XZ (the grounded mesh-gen lift is Y-only),
body-relative Y, immune to `ground_y` and the live card. `world_pos` remains
the coordinate of light, fog, and veil.

Either half alone reads as arbitrary — why should paint ignore the ground the
body stands on? — and together they are one principle. The failure the law
prevents is a pattern that swims: a body whose pigment is evaluated in world Y
repaints itself every time the ground under it moves, so its own surface
crawls while the body holds still. The mosaic found this first (MOSAIC_1), but
nothing about it is the mosaic's: any body-owned field — wear, weathering,
inscription — wants the same frame.

Two coordinates, two jobs; neither borrows the other's.

## L12 — DISTANCE TAKES THE GRAIN, NEVER THE MATERIAL

**A body's identity must not be a function of range.**

What fades with distance is the detail the eye can no longer resolve. What
must not fade is what the thing IS. A ceramic body seen from across the valley
is still ceramic — smoother, flatter, its tesserae gone — and if it instead
becomes the stone it would have been had it never been painted, then the
world's inventory changes as the camera moves, and no vantage point tells the
truth about it.

The law has a second half, and it is the half that pays for the first: **the
structure that produces only the faded detail should not be evaluated once it
has faded.** MOSAIC_1 fell into both halves at once — it dissolved a mosaic to
its palette color (identity as a function of range) and it justified that
dissolve as a cost cap it did not deliver (the walk still ran everywhere
inside the band, and a radius caps the mean, never the max). MOSAIC_2 split
them: the passage median is the material and is evaluated always, one hash; the
shard is the grain and its 27-cell walk runs only where the grain is fine
enough to matter. The saving is real precisely because it is structural rather
than a fade.

Where the grain's band comes from is a second question, and the answer is: the
band a body already has. MOSAIC_2 binds grain to `1 − veil_t`, the veil's own
icing smoothstep, so a body materializes at the ring already itself and gains
its detail across exactly the band where it materializes. Two bands measuring
"how far is this body" from two anchors can disagree — a body dithering out at
the ring while its grain insists it is near — and one band cannot.

The test for any distance-driven simplification: **name what the far form IS.**
If the answer is "the same material with a term at zero," the simplification is
lawful. If the answer names a different material, it is not a simplification —
it is a second body wearing the first one's geometry.

## L13 — A BOUNDARY IS A ZONE, NOT A LINE

**Where two regions meet, the meeting has width — and the far form of that
zone is the near form's average.**

A lattice gives you cells, and the naive reading of a cell is that everything
inside it takes the cell's value. That reading puts a hard edge at every face,
and worse, it cuts through whatever unit the content is made of: MOSAIC_2's
recon found passages sampled per fragment, so a single ceramic tile straddling
a passage face was **half one colour and half another** — a thing no tiler has
ever produced, and a defect no amount of tuning the edge would have fixed.

Two rules, and the second is what makes the first safe.

**Sample at the unit, not at the point.** If the content has a grain — tiles,
cells, grains, bodies — the region lookup belongs at the unit's own position,
so the whole unit belongs to one region. The fragment is where you are, not
what you are part of.

**Realize the zone in whatever the range can resolve.** Near, where units are
visible, the zone is *interleaved*: perturb the lookup per unit so units near a
face fall on either side, and the boundary becomes a band of intermixed
regions. Far, where units are not resolvable, the zone is *chromatic*: lerp
between the regions. At the boundary these two are the same function at
different scales — an unresolvable band of alternating blue and white tiles
**is** a blue-white lerp — so they agree in the limit by construction, and that
seam cannot pop or alias no matter how it is tuned.

**Do not extend that identity past where it holds.** MOSAIC_2's first draft of
this law claimed the far form is *always* the near form's average; adversarial
review showed it is not. Within a passage the far field draws one member where
the near field is a mixture of several — the ensemble means coincide, the
per-instance values differ by ~0.2 per channel. The law survives the correction
because the identity was only ever earned at the boundary, which is where it
was derived.

So the general form has two halves, and the second is the one that gets
forgotten: **when a simplification and the thing it simplifies are the same
function at different scales, no seam between them can exist** — and **when
they are not, the seam is real and must be placed where nothing can see it.**
MOSAIC_2 does both: the boundary zone is the same function twice, and the
within-passage seam is parked at `grain ≤ 0.001`, which the veil coupling
makes identical to "this fragment is ≥99.9% fog." A seam hidden by a
coincidence is a bug; a seam placed deliberately, where the placement is a
consequence of the design rather than of tuning, is engineering. The
distinction is whether you can say *why* nothing can see it.

## L14 — THE DEFAULT-LIMITS LAW

The lean build requests no WebGPU limit above core defaults and fits them:
storage buffers 8/stage (C6), texture array layers 225 of 256 (OPT_1b).
Exceeding a default requires Jean's stamp and a recorded reason; the
full-adapter passthrough at boot is a convenience, not a dependency.

The rationale is **compatibility**: a program that asks only for what it
uses runs on the widest set of devices, and the phone is the target that
decides. Requesting the adapter's maximum is not a harmless superset — it
narrows the device set for nothing — so restoring passthrough as a
"simplification" is a regression.

**No timing evidence backs this law, and none is claimed.** A previous
revision cited a single-run bisect on the development laptop (62,517 ms vs
5,609 ms, "an 11× slowdown") as proof that modest limits are also a
PERFORMANCE requirement. That claim is **withdrawn**: timing on that machine
varies by an order of magnitude between runs, so no single-run comparison
from it is evidence. Brackets on identical code:

| measurement | observed range |
|---|---|
| native pipeline creation | 70,459 → 205,527 ms |
| native patch system | 1,223 → 62,000 ms (era-dependent) |
| web total boot | 5.6 → 73.6 s |

The limits choice may or may not affect performance; this machine cannot
answer it, and the law does not need it to. Compatibility is sufficient
ground.

## L15 — A REFERENCE OUTLIVES ITS REFERENT

**A reference outlives its referent. Cite the symbol, not the line; name the
witness, not its value. And when a comment names a symbol, it has taken on a
debt the compiler will never collect.**

Ratified by HEM_1 from two misses in the same campaign — one in a handoff,
one standing in the tree:

| citation | claimed | actual | drift |
|---|---|---|---|
| the `sizeof(GPUDesignConfig)` witness, quoted as a value | 560 | 624 | two campaigns (MOSAIC_0, FIELD_2b) |
| `pawn_profile_normal_2d`, named by two `world.wgsl` comments | a function beside `pawn_profile_radius` | never defined, in any commit | the whole history |

These are one failure, not two. A line number, a quoted `sizeof`, and a symbol
in prose are all references to something that moves or was never there, and
nothing checks any of them. The symbol `GPUDesignConfig` survives every edit
that moves its size, while the number 560 was true once and then silently was
not. `pawn_profile_normal_2d` named a symmetry that was planned and not built,
and went on asserting it long after the code folded that branch into
`eval_profile_normal_2d` behind `is_regular`.

**The debt.** The compiler collects on a renamed function and never on a
renamed function *inside a comment*. Prose that names a symbol is load-bearing
with no test behind it: it must be re-read whenever the symbol moves, and
nothing will remind you. Name symbols anyway — vague prose is worse — but know
that naming one is a commitment, and that grepping the comment corpus before a
rename is the only collection mechanism there is.

**In practice.** In a handoff or a report, cite the symbol and let the reader
read its current value. Carry line numbers as hints, marked as hints, and
verify every one by symbol before editing.

---

### CANDIDATE (unnumbered) — WHERE A TIMER POINTS

*Filed by SHIP_0 U1. Unnumbered pending Jean's ruling — numbers here are
permanent, so this does not take one until it is adopted.*

**A timer names where the wait surfaced, not where the cost lives.**

Web per-pipeline times are **wire-enqueue latency**, not compile cost. The
backend compile executes in-order in the browser's GPU process and lands on
the first phase that waits. Witness, from one capture pair on the same
tree (`audit/THE BOARD FULL RELEASE CONSOLE.md`):

| phase | web twin | native twin |
|---|---|---|
| `Total pipelines` | **14 ms** | 205,527 ms (Renderer init) |
| `Patch system` | **56,887 ms** | 1,413 ms |
| `Total init` | 56,945 ms | 206,941 ms |

Neither twin is lying and neither is measuring what its label says. One
cost, two attributions: the web's near-zero pipeline times are enqueues
that returned immediately, and its 56.9 s "patch" phase is where the
deferred compile storm came due. The native twin shows the reverse, and its
patch phase — 1.4 s — is the honest cost of patch generation.

**Corollary.** Chromium disk-caches compiled pipelines per origin, so a
fast revisit (5.5 s observed) is expected and is not evidence that the
first visit was mismeasured.

The rule: before attributing a cost to the phase whose timer moved, ask
what that phase is the first thing to WAIT on.

---

## L16 — THE MECHANISM AUDIT

Every ruled mechanism in a handoff carries its own recon step verifying
the facts it stands on. **A design decision without a mechanism audit
does not land.**

Paid for three times in one session, each time by a mechanism that was
named confidently and was not what the design assumed:

- **`compute_vp`'s sun write.** ATLAS_1's D2 gave the sun and spot
  light 0 one slot, on the belief that the sun VP had a CPU writer at
  mood cadence. It has no CPU writer at all — `compute_vp` writes it on
  the GPU every frame, and the per-tile copy that D4 was retiring was
  also what protected light 0 from it.
- **The occupied instance channel.** D3 proposed `firstInstance` as a
  channel for the light index. `firstInstance` does not add a channel —
  it *biases* `instance_index`, five shadow VSes already index with it,
  six shadow draws are instanced, and the terrain's band 1 was already
  using it as an index base.
- **The gallery layout gap.** D2′ and D3″ were both sound and neither
  could *reach* two of the thirteen shadow VSes, which are drawn on a
  group-0 layout carrying neither binding.

None was catchable by argument; each was one grep from visible. The
audit is the grep, written down before the edit.

## L17 — DELEGATION CROSSES DESIGN FORKS, NEVER MEASUREMENT GATES

CC may rule a design fork under explicit delegation, with the ruling
annotated in the artifact so the choice has one home and a reader can
find who chose and why.

A gate requiring a **boot, a walk, or any physical reading is never
reasoned past.** A design fork has an answer in the tree; a measurement
gate has its answer only in the world. Confidence is not a substitute
for a reading, and an argument that a reading *would* come out a certain
way is the exact shape of the error the gate exists to catch.

## L18 — GATE RESULTS TRAVEL IN WRITING

Walk and boot verdicts are pasted into CC's next session opener or into
the merge commit. **A merge is never recorded "no walk reported" when a
walk occurred.**

The record is the only thing a later session has. A verdict held in a
person's memory is, to the tree, indistinguishable from a verdict that
never happened — and the merge commit is where a bisect looks first.

## L19 — COMPAT MODE, DECLINED AS A DECISION

WebGPU compatibility mode may zero vertex-stage storage bindings, and
the render room stands at **V storage 7 of 8** (`Shadow Gallery Frame` /
`Shadow Wall Painting`, post-ATLAS_1revB G2).

**Declined.** Revisit only if a *measured* audience device requires it.

Recorded as a decision rather than an omission: the constraint is known,
the cost of honouring it is known, and the choice is to spend the
headroom on the work instead. An unrecorded decline is indistinguishable
from an oversight, and the next reader of that 7-of-8 deserves to know
it was seen.

## L20 — OPTIONAL FEATURES

The baseline is **WebGPU core, one shader source.** A feature is adopted
only with all three of: runtime detection, an identical-semantics
fallback, and its own witness at every gate.

**The adopt-list starts empty.**

- `timestamp-query` stays meter-preset-only. It is an instrument, not a
  capability the artwork depends on.
- Texture compression, *if* the paintings are ever unfenced, is
  per-platform transcode — never baseline. The grants census already
  shows the split: ASTC and ETC2 on the Pixel's valhall row, BC absent.

Two shader sources is the failure this forbids. A feature adopted
without a fallback makes the second source inevitable, and the moment
there are two, every witness in the tree is witnessing half a program.

## L21 — A TOGGLE IS CHAINED AT THE STAGE THAT CONSUMES IT

Dawn toggles carry a `ToggleStage` — Instance, Adapter, or Device. A
toggle chained at a descriptor **downstream** of its stage is silently
inert: no error, no warning, no toggle. A toggle chained **upstream**
depends on inheritance carrying it every hop, which is a Dawn
implementation detail this program cannot read and must not assume.

**So chain each toggle at its own stage, and never at a neighbour's.**
`use_dxc` is Adapter-stage and belongs on the instance/adapter path;
`disable_symbol_renaming` is Device-stage and belongs on the device
descriptor. Two toggles at two stages is the correct shape, not
duplication to be tidied away.

Paid for by PIVOT_0a and debt 12. `use_dxc` was chained on the device
descriptor, one stage too late; the boot log said "Compiler plan: DXC"
and FXC compiled anyway. The reverse error — a Device-stage toggle
chained on the instance descriptor and hoping inheritance delivers it —
was never disproved, and under this law never needs to be: the question
only arises for a toggle sited somewhere other than its own stage.

**Debt 12 therefore closes as MOOT, not as branch (a) or (b).** Its
question — does the instance chain propagate two hops? — stops gating
anything once no toggle is ever sited off its own stage. Reopening it
costs one boot: re-arm one Device-stage toggle on `idesc.nextInChain`
alone and read the count. Present → inheritance carries; absent →
branch (b) confirmed at last. Nothing in the queue wants that boot.
`disable_symbol_renaming` at count 9 → 10 on the device chain (Dawn
`f0bf8ab`, 2026-08-13) is the positive half, and the only half proven.

**The corollary is the witness.** A toggle request is not a toggle.
`dawn::native::GetTogglesUsed(device_)` prints the set Dawn actually
enabled, and that line stays in the boot log for exactly this reason: a
switch that cannot be seen to have fired is indistinguishable from one
that never fired (P6).

## L22 — THE SCHEMA LAW

`tools/binding_schema.py` is the single authority for the binding
surface. `binding_registry.hpp` and `binding_surface.gen.inc` are
generated; edit the schema and run `binding_gen.py --write`.
`world.wgsl` declarations are checked mirrors: `binding_gen.py --check`
must pass at every campaign's recon gate and before any commit that
touches the surface.

## L23′ — THE SCOPE LAW (supersedes L23)

Within one synchronization scope, a buffer presents ONE writability.
A render pass is one scope, WHOLE. A compute dispatch is one scope
over its FULL bound groups. Neither is filtered by shader-stage
visibility or by static use — Dawn merges every entry of every bound
group, touched or not (Jean's boot log at the U4 gate is the
evidence; L23's "compute validates per dispatch over what it uses"
was the half of the truth that survived one gate).

So: **mixed-writability faces of one buffer never share a layout and
are never co-bound in one scope.** A stratum serving a scope carries
only the faces that scope may legally see — FRAME split by consumer
mode (FRAME_R render / FRAME_C compute, A8a); ORBS carries its face
partition in two layouts (A8b); GALLERY/PHOTO_K stand from A7.

Witness: `P-scope`, both arms — the render arm per pass span, the
compute arm per dispatch site over the full bound groups, plus the
group-local law (no bind group backs one buffer through entries of
mixed writability). **Pessimism is the law: no relaxation of the rule
may ever be committed on a citation — only on a witnessed Dawn
behavior test.**

Paid for twice: A7's gallery working set in the render passes, then
A8's FRAME ro windows and collapsed orb faces at the compute
dispatches — the same law, learned one scope at a time.

## SUNSET_0 (2026-08-16) — the web twin is the program
Native is archived at tag `native-sunset`. Resurrection is
archaeology from the tag, not maintenance. The witness chain is:
naga gates the WGSL module per commit (CC-runnable); the web build
+ boot witnesses pipeline-layout conformance and minBindingSize
(the classes naga cannot see — ATLAS_1revB). The audience floor
(WebGPU core defaults) and the compiler floor (PIVOT_0) are
unchanged by this sunset.

SUNSET_1 (2026-08-16): the native arms are deleted from shared files;
__EMSCRIPTEN__ guards collapsed; the tree compiles one program.
Resurrection remains archaeology from tag `native-sunset`.
