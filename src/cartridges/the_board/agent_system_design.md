# Agent System Design Document

_A unified entity layer for the board: every pawn-like body is an
agent, the player is the body they currently inhabit, and possession
is a transferable relationship. Companion to `ground_hierarchy_design.md`
(already landed) and `floater_backbone_design.md` (deferred)._

---

## 1. Motivation

The existing pawn is not really a character. Read across the codebase,
it's a **reference frame** — the camera's target, the shadow VP's
center, the aura field's origin, the orb dome's optional anchor, the
radial pulse origin, the terrain clearance reference, and the source
of a dozen `COUPLING_PAWN_TO_*` bits. A lot of systems sample from it;
the fact that one of them is player input is almost incidental to its
structural role.

Two earlier moves made this more visible rather than less:

- The ground architecture refactor gave us `POLICY_WALKER_AGENT` — a
  ready-made policy for any pawn-like body that isn't the player.
  The policy exists before any consumer does, explicitly anticipating
  a multi-agent future.
- The floater backbone proposal made anchors live, mobile things. But
  today the only moving anchor the board can offer is the pawn. A
  population of pawn-like bodies with distinct motion gives floaters
  something to actually bind to.

The narrow move — adding "agents" as a new kind of thing alongside the
pawn — solves the floater-anchor problem but punts on the deeper
observation. The deeper observation is:

> **There is only one kind of pawn-like body on the board, and the
> player's relationship to it is a soft, transferable property.**

The player isn't a special entity. The player is a *cluster of state*
that attaches to an entity: a coupling bitmask, an aura, a camera
configuration, a set of active musical modes. The body the player
inhabits at any given moment is one slot in a shared array; the
behavior driving that slot is `PlayerControlled` rather than
`RandomWalk`. When the player transfers to another body, they take
their state with them; the body they leave behind stays in place and
reverts to a simple algorithmic behavior.

This framing — possession over inheritance, relationship over identity
— produces an architecture that is strictly simpler than the current
one, not more complex. It collapses two parallel structures (pawn vs.
agents) into one.

---

## 2. Player and Agents

### 2.1 Two structures, cleanly separated

**`AgentState`** is the physical state of a body on the board:

```
  pos:           vec3<f32>     world position
  vel:           vec3<f32>     integrated velocity
  heading:       f32           facing direction
  home:          vec3<f32>     spawn location, tether target
  t:             f32           personal clock
  behavior_id:   u32           which algorithm drives motion
  tier_idx:      u32           per-tier parameter variation
  seed:          u32           stable noise source
  is_active:     u32
  _pad:          u32
```

Padded to 64 bytes, 16-byte aligned. Identical shape for every body:
the one the player inhabits, the RandomWalker you just walked past,
the Patrol agent tracing its circuit. What differs is the
`behavior_id`.

**`PlayerState`** is the player's relationship to the world:

```
  possessed_slot:     u32             which agent slot the player inhabits
  active_couplings:   u32             COUPLING_* bitmask
  aura_presence:      f32             [0..1] ramp value
  mmode_intensities:  array<f32, N>   per-mmode [0..1] trajectory values
  camera_prefs:       CameraPreferences
```

`PlayerState` survives mood transitions. `AgentState` arrays are
cleared and re-populated by each mood (except slot 0, which belongs to
the player). See §6 (lifecycle).

### 2.2 Why this is simpler than the pawn-plus-agents version

Today the pawn has: its own state struct, its own compute kernel, its
own render pipeline, its own shadow pass, its own bind group, its own
place in every coupling lookup, its own special ground policy. It's
also the authoritative reference for camera, aura, eviction, and
dozens of other systems.

Under the unified model, there is one `agent_state[]` array, one
compute kernel with behavior dispatch, one render pipeline that
handles everyone, one bind group. The player-vs-not distinction is a
single field lookup: `agent_state[player.possessed_slot]` is the body
the player inhabits. Camera tracks that slot. Aura centers on that
slot. Eviction is measured from that slot. No special cases.

The `pawn_state` struct disappears. Its contents repartition: physical
fields (position, velocity, orientation) move into
`agent_state[player.possessed_slot]`; player-owned state (couplings,
aura presence, mmode intensities) moves into `PlayerState`.

### 2.3 Term of art: "pawn"

The word "pawn" remains in the codebase as a term of art. It refers
to the chess-piece-shaped body the player inhabits — visually, not
structurally. `pawn_aura_*`, `pawn_vs`, `COUPLING_PAWN_TO_CAMERA_TARGET`
all keep their names; they mean "the body the player is currently
inhabiting" now. The mesh generator, shader, and aura system don't
need to know the player can move between bodies — they just render/
compute for whichever slot is currently `player.possessed_slot`.

The one meaningful rename: `COUPLING_INPUT_MOVES_PAWN` becomes
`COUPLING_INPUT_MOVES_PLAYER` because it describes what the player's
input does, not a property of any specific body.

---

## 3. Possession

### 3.1 The transfer mechanic

Trigger: **Caps Lock**. Single key press, no hold behavior, no combo.

When the player presses Caps Lock:

1. Find the nearest active agent (by XZ distance) within a radius of
   15 world units, excluding the currently-possessed agent. If none,
   the press is a no-op.
2. Transfer: the target agent's `behavior_id` becomes
   `PlayerControlled`; the previously-possessed agent's `behavior_id`
   becomes `RandomWalk`. `player.possessed_slot` updates to the new
   target. Body and tier are preserved on both sides.
3. All other `PlayerState` fields — couplings, aura presence, mmode
   intensities — remain untouched. They belong to the player, not
   the body.
4. The camera, which tracks `agent_state[player.possessed_slot]`,
   begins gliding toward its new rest position (at the new body's
   default orbit distance and angle). The glide is faster than walk
   speed; camera catches up within a second or two. It's not
   instant and it's not a fade — it's the camera moving as if it
   were tracking a fast-walking target.

### 3.2 What transfers and what doesn't

**Transfers with the player (in `PlayerState`):**
- Active couplings — the player's relationship to music, terrain,
  orbs, waves, pulses.
- Aura presence ramp value — walk into a new body and the aura is
  already present or already fading, whichever you were carrying.
- Mmode intensities — same.
- Camera preferences (zoom level, elevation preference, orbit
  angle).

**Stays with the body (in `AgentState`):**
- Position, velocity, heading, personal clock.
- Home tether, spawn seed.
- Tier — a Scout you possessed stays a Scout when you leave it. A
  Scout is a property of the body, not the player.

### 3.3 The dropped body

When the player leaves a body, its `behavior_id` switches from
`PlayerControlled` to `RandomWalk`. Tier is preserved. Position and
velocity are inherited — the body is where the player last had it.
Music couplings on the dropped body remain set to their per-behavior
defaults (for `RandomWalk`, no music couplings are active).

This means a Leader you just abandoned reverts to RandomWalk but
still moves with Leader-tier speed and step size. Visually, it keeps
its color, scale, and silhouette. It's the same chess piece it was
before; it just stops being *you*.

### 3.4 No body to possess

The player always has a body. Slot 0 of `agent_state[]` is reserved
for the player and is spawned on first world entry (session start).
When a mood transitions and clears slots 1..31 for the new
population, slot 0 is exempt — the player's body persists across
mood transitions.

If a mood authors `count = 0` agents, slots 1..31 are empty after
transition, and Caps Lock simply has no target. The player is alone.
That's a valid configuration — a mood where nothing else inhabits
the world.

### 3.5 Eviction

Today the pawn is immune to eviction; it's the reference frame for
the traversable world. Under the unified model, the **currently
possessed agent** is immune to eviction. Other agents are evicted
based on their distance from the possessed agent, using the same
350-unit threshold the ribbon system uses.

Concretely: each frame, for each agent in slots 1..31, if `distance(
agent.pos.xz, agent_state[player.possessed_slot].pos.xz) > 350`, the
agent's `is_active` flips to 0. Slot 0 is exempt from this check.

When the player transfers to a different slot via Caps Lock, the
"reference frame" for eviction moves with them. The agent now driving
eviction is whichever one they inhabit.

---

## 4. What An Agent Is

### 4.1 The structure

```
  AgentState {
      pos:         vec3<f32>
      vel:         vec3<f32>
      heading:     f32
      home:        vec3<f32>
      t:           f32
      behavior_id: u32
      tier_idx:    u32
      seed:        u32
      is_active:   u32
  }
```

64 bytes. One storage buffer of 32 slots. Slot 0 is the player; slots
1..31 are mood-authored.

### 4.2 Visual identity

Every agent renders as a chess pawn — same mesh, same vertex
generation logic, instanced. The possessed agent receives a subtle
visual distinction (possibly a brighter emissive, a thin outline, or a
slight scale bump) so the player always knows which body they're in.
Exact treatment is a Pass 1 decision; the architecture just needs the
pipeline to know which slot is `player.possessed_slot`.

Per-tier colors are authored in `AGENT_TIER_GAINS`:
- Worker: slate gray
- Scout: bronze
- Sentinel: deep blue
- Leader: pale gold

These are properties of the body. A Scout is always bronze,
regardless of who's driving it.

### 4.3 Ground policy

Every agent (including the possessed one) resolves ground through
**`POLICY_WALKER`** when it's the possessed agent, and
**`POLICY_WALKER_AGENT`** otherwise. The distinction is the pawn aura
self-form and GoL self-suppression, which are relevant only to the
body the player inhabits (the aura is centered on the possessed
agent; self-suppression is relevant when *the player* is standing on
a GoL zone).

The policy choice is one conditional in `update_agents`:
```
if (slot == player.possessed_slot) {
    // POLICY_WALKER + step-climb
} else {
    // POLICY_WALKER_AGENT + simple snap
}
```

This is semantically cleaner than having a "pawn" kernel and an
"agent" kernel — it's one kernel with one branch that naturally
selects the right ground policy based on who's driving.

---

## 5. The Control Panel

Three registries in `modules/agents.inl`, following the established
`constexpr`-table aesthetic.

### 5.1 AGENT_BEHAVIORS

Indexed by `behavior_id`. Carries the parameters that govern each
behavior's motion. Pass 1 fully implements `PlayerControlled` and
`RandomWalk`; the rest are stubbed for Pass 2.

```
  PlayerControlled   input drives velocity; no algorithmic component
  RandomWalk         uniform random direction at each step interval
  CorrelatedWalk     direction rotates by small random angle per step
  Wanderer           CorrelatedWalk + home tether
  HomeSeeker         strong spring to home, weak perturbation
  Patrol             deterministic waypoint circuit
  Pursuit            steer toward target (player or agent)
  Flee               Pursuit with sign flipped
  Flock2D            Vicsek-style alignment with local neighbors
  LevyFlight         heavy-tailed step size distribution
```

Columns: `motion_kind`, `step_rate`, `step_size`, `persistence`,
`drag`, `home_pull`, `neighbor_radius`, `speed_cap`, plus per-
behavior gesture sub-tables (deferred to Pass 2).

### 5.2 AGENT_TIER_GAINS

Indexed by `tier_idx`. Per-tier multipliers on behavior parameters,
plus render color.

```
  Worker     step_size×1.0  persistence×1.0  speed×1.0  coupling×1.0   gray
  Scout      step_size×1.8  persistence×0.4  speed×1.4  coupling×1.0   bronze
  Sentinel   step_size×0.6  persistence×1.2  speed×0.5  coupling×1.0   deep blue
  Leader     step_size×1.2  persistence×0.9  speed×1.1  coupling×2.5   pale gold
```

Tier gains compound with behavior parameters — a Scout running
RandomWalk takes longer, less-persistent steps than a Worker running
RandomWalk.

### 5.3 AGENT_POPULATIONS

Indexed by mood. Authors what non-player agents (slots 1..31) look
like for each mood.

```
  per mood:
    count                0..31
    behavior_weights[]   per-behavior spawn probability
    tier_weights[]       per-tier spawn probability
    spawn_radius         distance from player at spawn
    home_seeding_radius  home offset variance per agent
```

Pass 1 authors small populations across 2–3 moods (open_default,
open_sunset); other moods author `count = 0` and remain unpopulated.
Pass 2 fills the rest with authored flavor.

---

## 6. Lifecycle

### 6.1 Session start

- Initialize `PlayerState` with defaults: `possessed_slot = 0`,
  couplings from the mood's default, aura presence 0.0, mmode
  intensities 0.0.
- Spawn `agent_state[0]` with `behavior_id = PlayerControlled`,
  tier `Worker`, position at the world's authored start point.
- Other slots start inactive.

### 6.2 Mood transition

On mood change:

1. `PlayerState` persists untouched. Couplings, aura, mmodes survive.
   The specific coupling values may be re-set by mood application
   (e.g. the new mood disables COUPLING_PAWN_AURA and the player's
   aura begins ramping down), but the player's internal state
   variables aren't reset.
2. `agent_state[0]` persists. The player remains in their body.
3. `agent_state[1..31]` are cleared (`is_active = 0`). Whatever
   population the previous mood had is gone.
4. If the new mood's `AGENT_POPULATIONS` row specifies `count > 0`,
   spawn that many agents into slots 1..(count), each rolled from
   the mood's behavior and tier weights, positioned within
   `spawn_radius` of the player's current position.

Because the player's position is the reference, agents spawn *around
the player*, not around a world origin. The world transitions feel
like the populations change around a persistent observer.

### 6.3 Eviction and respawn

Per §3.5, agents in slots 1..31 are evicted when they wander past 350
units from the possessed agent. Evicted slots do not respawn within
the same mood — the population is set once at mood entry. Pass 2 may
add respawn-on-demand if the emptiness feels wrong.

### 6.4 Transfer (Caps Lock)

Per §3.1. Atomic: one-frame slot swap, no animation state beyond
camera glide. The kernel handles the case naturally — on the next
frame, slot 0's `behavior_id` is `RandomWalk`, slot K's `behavior_id`
is `PlayerControlled`, the player's `possessed_slot` is K, and all
the per-slot consumers (camera, aura, eviction reference) look up the
new slot and work correctly.

---

## 7. The Compute Kernel

One kernel, `update_agents`, dispatches 32 threads. Each thread
handles one agent slot. The shape:

```
update_agents(gid):
    slot = gid.x
    if (slot >= 32): return
    a = agent_state[slot]
    if (a.is_active == 0): return

    // Eviction (slot 0 is exempt; the possessed slot is also exempt
    // in case the player's body is ever moved to a different slot)
    if (slot != player.possessed_slot):
        if (distance(a.pos.xz, possessed.pos.xz) > 350):
            a.is_active = 0
            write back; return

    // Personal clock advances
    a.t += dt

    // Behavior dispatch (uniform switch)
    switch (a.behavior_id):
        case PlayerControlled:  behavior_player(a, input_state)
        case RandomWalk:        behavior_random_walk(a, dt)
        ...

    // Integration (drag + speed cap + position update)
    // ... identical for all agents ...

    // Ground resolve
    if (slot == player.possessed_slot):
        a.pos.y = POLICY_WALKER  // with step-climb
    else:
        a.pos.y = POLICY_WALKER_AGENT  // simple snap

    agent_state[slot] = a
```

The `PlayerControlled` branch reads the input state that today
`update_pawn` reads — move_x, move_z, camera azimuth for FPV mode
heading lock. It's the pre-refactor pawn movement body, inlined into
a behavior switch case.

### 7.1 Compile-time risk

The ground refactor taught us that `update_pawn` is FXC's sensitivity
point: contributor chain inlining × multiple query invocations
produced 58-second compile times that we optimized back down to 27s.
The unified kernel re-introduces some of that risk because it now
handles multiple behaviors, each potentially growing into its own
contribution.

Mitigations:

- Keep behavior functions small and uniformly structured.
- `PlayerControlled` is the heaviest branch (it's the old
  `update_pawn` body, with step-climb and tilt and full-walker
  ground resolve). It's also the branch that runs for exactly one
  slot per dispatch. This is fine — FXC's uniform branching survives
  this pattern as long as the switch is on a const-style dispatch.
- The "possessed vs. not" ground policy branch is the one architectural
  risk. If compile time rises to 60+ seconds, the fallback is to
  split the kernel: `update_player_agent` runs for just slot 0 (or
  whichever is possessed) with the walker policy and step-climb;
  `update_other_agents` runs for the rest with simple agent policy.
  Two kernels, two dispatches. Defer to observation.

---

## 8. Rendering

All agents render through one pipeline. The pawn mesh generator is
parameterized by instance index, which looks up the agent state
struct. Tier color comes from `AGENT_TIER_GAINS[a.tier_idx].color`.
The possessed agent is visually distinguished (a subtle outline or
brightness bump — detail deferred to Pass 1 execution).

Shadow pass: every active agent casts a shadow. Same instanced pawn
mesh, minimal vertex shader, into the shadow map.

Inactive slots (`is_active == 0`) produce degenerate triangles in the
vertex shader — collapsed behind the near plane. The draw call
always dispatches 32 instances; the VS handles the activity gate.

---

## 9. The Pass Plan

Three passes, as in the earlier agent doc, but the scope shifts
because unification moves earlier.

### Pass 1 — Unification

**Scope:**
- `PlayerState` struct on CPU side.
- `AgentState` struct (64 bytes) with storage buffer of 32 slots.
- `update_agents` compute kernel with `PlayerControlled` and
  `RandomWalk` behaviors implemented. Other behaviors stubbed.
- Delete `pawn_state` struct; migrate every reference to
  `agent_state[player.possessed_slot]`.
- Delete `update_pawn` kernel; its body is inlined into
  `update_agents` as the `PlayerControlled` case.
- Caps Lock handler for possession transfer (nearest-within-15).
- Camera target indirection through player.possessed_slot.
- Dropped-body reverts to RandomWalk on transfer.
- Mood transition preserves slot 0, re-populates slots 1..31.
- Player-centered eviction for non-player slots.
- Agent rendering: one pipeline, instanced, with tier colors and
  possessed-agent visual distinction.
- Shadow pass for all agents.
- `AGENT_BEHAVIORS`, `AGENT_TIER_GAINS`, `AGENT_POPULATIONS`
  registry tables in `modules/agents.inl`.

**Dependencies:**
- Ground architecture (landed). `POLICY_WALKER_AGENT` ready to use.
- Fresh git branch. Current ground-refactored main preserved.

**Success criterion:**
- Player-pawn behaves visually identically to pre-Pass-1. Camera,
  shadow, aura, pulses, couplings, portals all function.
- Caps Lock near another agent swaps the player into that body. The
  body the player left continues moving (RandomWalk). The camera
  glides to track the new body.
- Mood transitions preserve the player's body and state while
  clearing and respawning the rest of the population.
- No regressions in any pawn-coupled system.

**Risk:** Medium-high. This touches every pawn reference in the
codebase. Camera, shadow VP, aura, couplings, portals, radial pulse
origins. Each of these has working behavior today and needs to keep
working after the rewire. The branch-first protocol is essential.

### Pass 2 — Behavior library

**Scope:**
- Implement CorrelatedWalk, Wanderer, HomeSeeker, Patrol, Pursuit,
  Flee, Flock2D, LevyFlight.
- Populate gesture sub-tables.
- Populate tier gains so Worker/Scout/Sentinel/Leader produce
  visibly different motion on the same behavior.
- Fill out AGENT_POPULATIONS for all moods.

**Dependencies:** Pass 1.

**Success criterion:** each behavior produces its authored character
(Wanderer meanders in a region, Patrol traces its circuit, Flock2D
flocks). Mood transitions cleanly swap populations while the player
persists.

**Risk:** Low-medium. Mostly content work at this point; the
architecture carries it.

### Pass 3 — Floater binding

**Scope:**
- `anchor_agent_id` field on floater instances.
- Floater kernels look up their anchor agent each frame.
- `FLOATER_BINDING_PATTERNS` registry.
- Mood-authored binding patterns on 2–3 moods.
- Orphaned binding fallback: if anchor agent evicts, floater falls
  back to its seeded home anchor.

**Dependencies:** Pass 2 plus floater backbone refactor (which has
been deferred separately; Pass 3 requires it).

**Success criterion:** a mood authored with "4 Wanderer agents, each
with 2 bound cubes" produces cubes visibly following their authored
anchors.

**Risk:** Low. Agent system and floater system are both proven at
this point; binding is a small integration.

---

## 10. The Product-Line Horizon

The earlier agent design named three product shapes — visualizer,
gallery, proto-game — that the system could inflect toward based on
which mood rows were authored. Unification adds a fourth, and arguably
the most interesting:

**An inhabited world where identity is a choice.**

- The music visualizer becomes participatory differently. You can
  *be* any part of the responsive environment by transferring into
  it. Inhabiting a Flock2D agent briefly puts you inside the
  flocking rule — you feel the alignment pressure of your neighbors
  as part of your own motion.
- The gallery becomes narrative. You can stage scenes — leave a body
  in a contemplative posture, transfer to another, walk around to
  see the composition you just made.
- The proto-game loop gains texture. A Pursuit agent chasing you can
  be escaped by *becoming* one of the other agents. The rules of
  pursuit don't disappear; your relationship to them shifts.
- A quieter reading: possession as contemplation. You walk, you
  transfer, you walk again. The transfer is a meditation tool —
  perspective as something you pick up and put down. No game
  objective; just presence migrating through the world.

All four are reachable from the same system. Which one a given
session is depends on the mood rows loaded — which means, in practice,
on which set of registry rows the mood author checks.

The unification doesn't force the interpretation. It just keeps it
reachable from the quietest to the most game-like direction without
building different engines.

---

## 11. Open Questions

**Visual distinction of the possessed agent.** Subtle outline? Small
scale bump? Brightness lift? Tier-specific aura color? Some combination?
Resolvable in Pass 1 execution; low stakes either way as long as
it's clear which body is currently you.

**Aura persistence across portals.** Flagged in conversation: today,
aura resets to on after portal transitions. The *correct* behavior is
"carry the previous aura state across." Deferred — not urgent, not
blocking, revisit post-Pass-1.

**Possession transfer when no target exists.** Caps Lock with no agent
in range is a no-op today. Should it have feedback — a small sound,
a failed-gesture camera wiggle? Probably just silent for now.

**Self-transfer / deselection.** What if the player wanted to *leave
all bodies* and become a disembodied observer? Not reachable under
the current design (the player always has a body). Out of scope;
noted if it ever matters.

**Multi-button / combo transfers.** Caps Lock + directional key to
choose *which* nearby agent to possess (rather than always the
nearest)? Defer. Nearest is fine for now.

**Possession-triggered transitions.** Should transferring into a body
with a specific tier change the mood? Author a mood that "becomes a
Leader" when the player possesses a Leader? Architecturally clean,
aesthetically unexplored. Out of scope.

**What happens when the possessed body enters a portal.** Current
behavior: the pawn enters a portal and the world transitions. Under
the unified model, the possessed agent enters the portal and the
world transitions. Should non-player agents entering a portal do
*something* interesting? Probably just disappear (evict). Confirm
during Pass 1 execution.

**Agent-agent collision.** Unclaimed. Bodies can pass through each
other. Pass 2+ concern.

**Multi-camera choice.** Could the camera follow a *non*-possessed
agent for a cinematic moment while the player controls another?
Architecturally trivial; aesthetic value unclear. Out of scope.

---

## 12. Summary in One Sentence

The board is populated by a single kind of pawn-like body, and the
player is whichever body they currently inhabit — a transferable
relationship, carried in `PlayerState`, triggered by Caps Lock,
expressed by a `behavior_id` swap that the compute kernel handles as
one case among many — collapsing the pawn/agent distinction into a
simpler architecture while opening the product-line door to an
inhabited world where identity is a choice the player makes by
walking over to another body and pressing a key.
