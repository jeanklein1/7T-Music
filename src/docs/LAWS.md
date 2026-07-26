# THE LAWS

Repo home: `src/docs/`. Sibling of `7t_program_theory_v3.md` — that file is
THE LENS (how to think about the program); this one is the LAWS OF PRACTICE
(what breaks if you don't). Created by PRUNING_1 P4 because there was no
live one: `src/docs/old docs/cartridge_constitution.md` and
`src/docs/old docs/terrain_program_charter.md` are archived by their folder,
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
is gone; see `audit/WEB_PORT_LEDGER.md`. The pin stands on its own merit.)

## L2 — THE FXC LAW

The Windows D3D12 backend compiles through FXC, which has hard limits the
Vulkan/Metal backends do not. The shader honors them **by structure**, so
nothing in it looks like a workaround and everything is one:

1. Instance structs in hot loops stay lean and byte-pinned — `GPUPierInstance`
   is 48 B with a `static_assert` in `state.hpp` (successor of the retired
   32-byte `SolidInstance` rule).
2. The collision/ground chain admits **no new runtime branching**.
   `evaluate_pier`'s caller bounds its loop by a uniform (`config.pier_count`)
   and dispatch is by uniform function choice, never by branch.
3. Texture-array stamps in the collision chain **hang FXC**. Do not add one.
4. One `DrawIndexedIndirect` per render pass, maximum.
5. Storage buffers per stage = 10. Uniform buffers per stage = 12.

A violation does not fail on the developer's machine. It fails on Windows, at
pipeline creation, in someone else's hands.

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
2. Numbers are **group-scoped**, not global: `22` is `terrain_mesh_indices` in
   group 0 and `bilinear_sampler` in group 1. `g0::` and `g1::` are separate
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
time — `audit/WEB_PORT_LEDGER.md`. `audit/cc4_wgsl_static_usage.py` computes
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
