# Claude Code — Agent System Pass 1 (Unification)

Implement the unified entity layer: the pawn and the agents become
one array of `AgentState`, the player becomes a `PlayerState` struct
pointing at a slot in that array, possession transfers via Caps Lock.
The existing pawn's behavior, couplings, aura, shadow, and camera all
keep working — they just now operate through one extra layer of
indirection (`agent_state[player.possessed_slot]` wherever
`pawn_state` used to be).

**Design reference:** `/mnt/user-data/outputs/agent_system_design.md`.
Read §1–§9 in full before starting.

This brief is **larger** than a typical Pass 1. It touches every
pawn reference in the codebase — roughly 50 sites in `world.wgsl`, ~27
in C++. The branch-first protocol is essential; see §0.

---

## 0. Branch Before You Start

Before touching anything: create a new git branch from the current
main (which has the compile-time-optimized ground refactor).
Something like `agents/pass-1-unification`. Tag the pre-unification
state as `pre-unification-archive` so it's trivially reachable if we
need to roll back. Do not start coding on main.

This is a real rollback risk. If the refactor gets messy, we want to
be able to return to a known-good ground-refactored baseline with
one git command.

---

## 1. The Shape of the Refactor

### 1.1 What changes

- **`pawn_state` struct disappears.** Its fields repartition:
  position/velocity/orientation move into `agent_state[0]`; player-
  owned state (couplings, aura presence, mmode intensities) moves
  into a new `PlayerState` struct.
- **`update_pawn` kernel disappears.** Its body becomes the
  `PlayerControlled` case of a new `update_agents` kernel's behavior
  switch.
- **`pawn_vs` and `shadow_pawn_vs` disappear.** Replaced by
  `agent_vs` and `shadow_agent_vs` that render all agents (including
  the player's body) from one instanced pipeline.
- **Every `pawn_state.pos`, `pawn_state.heading`, `pawn_state.velocity`,
  `pawn_state.orientation` reference in shaders becomes
  `agent_state[player.possessed_slot].pos`** (or the `.heading`,
  `.vel`, `.orientation` equivalents). See §4 for the complete site
  audit.
- **Every C++ reference to `pawn_state` (in `state.hpp`,
  `cartridge.hpp`) becomes a reference to `agentStateBuffer_[0]`**
  or through a lookup helper. See §5 for the C++ migration.

### 1.2 What stays the same

- **The pawn mesh.** The chess piece generator in `pawn_vs` moves
  into `agent_vs` unchanged; it just reads its transform from
  `agent_state[instance_idx]` instead of from `pawn_state`.
- **The aura system.** `pawn_aura.inl` and `compute_pawn_aura`
  kernel keep running as-is. They center on "the player's body,"
  which now means `agent_state[player.possessed_slot]` instead of
  `pawn_state`. One line of indirection added; rest unchanged.
- **All couplings.** `COUPLING_PAWN_TO_*` names are preserved. Only
  `COUPLING_INPUT_MOVES_PAWN` renames to `COUPLING_INPUT_MOVES_PLAYER`
  because it describes player intent, not body relationship.
- **Camera, shadow VP, portal logic.** These look up "the pawn
  position" through one extra field dereference. No behavioral
  change.
- **Ground architecture.** `POLICY_WALKER` and `POLICY_WALKER_AGENT`
  are both used — the possessed agent queries through
  `POLICY_WALKER` (with self-centered aura/suppression and step-climb),
  non-possessed agents query through `POLICY_WALKER_AGENT` (simple
  snap, no self-centered contributors).

### 1.3 What's new

- `AgentState` struct (64 bytes, 32-slot storage buffer).
- `PlayerState` struct (CPU + GPU uniform version).
- `update_agents` compute kernel with behavior dispatch.
- `behavior_player_controlled`, `behavior_random_walk` functions.
- Caps Lock input handler for possession transfer.
- Nearest-agent-within-radius query on the CPU side for transfer
  targeting.
- `modules/agents.inl` with `AGENT_BEHAVIORS`, `AGENT_TIER_GAINS`,
  `AGENT_POPULATIONS` registries.
- Agent render pipeline (VS, FS, shadow VS) — one pipeline handles
  all agents including the possessed one.

---

## 2. Preflight

### 2.1 Read these files first

- `/mnt/user-data/outputs/agent_system_design.md` — the full
  design rationale.
- `modules/ground_architecture.inl` — the policy registry. Observe
  that `POLICY_WALKER` and `POLICY_WALKER_AGENT` exist and differ
  only in `CONTRIB_PAWN_AURA` (self vs external form) and
  `CONTRIB_GOL_SUPPRESSION` (included vs not). This distinction is
  what drives the possessed-vs-not branch in `update_agents`.
- `modules/pawn_aura.inl` — the aura system stays; understand that
  `compute_pawn_aura` will now center on whichever slot the player
  is in.
- `modules/orbs.inl` — the `constexpr`-table pattern to match for
  `AGENT_BEHAVIORS` / `AGENT_TIER_GAINS` / `AGENT_POPULATIONS`.
- The current `update_pawn` kernel body in `world.wgsl` (search for
  `@compute @workgroup_size(1)` near `fn update_pawn`). Its contents
  will move into `behavior_player_controlled` in the new unified
  kernel.
- The existing `pawn_vs` and `shadow_pawn_vs` in `world.wgsl`. Their
  mesh generation logic moves unchanged into `agent_vs` and
  `shadow_agent_vs`.

### 2.2 Project-specific tripwires

Standing rules from prior passes:

- **FXC uniform branching.** Behavior dispatch must be a
  const-style switch on `behavior_id`. No dynamic function pointers,
  no runtime dispatch tables.
- **`target` is reserved WGSL.** Use `tgt` or similar.
- **`bitcast<f32>(u32)` produces denormals.** Use `f32(x)`.
- **Struct alignment.** 16-byte-align all storage structs.
  `static_assert(sizeof(AgentState) == 64)`.
- **Storage buffer budget.** 10 per stage. `agent_state` replaces
  `pawn_state` in the vertex stage (net zero). Count carefully.
- **WGSL reserved keywords** — don't name fields `ray`, `texture`,
  `sampler`, `common`, `workgroup`, or `target`.
- **FXC compile-time sensitivity.** The `update_pawn` → 58s saga
  is still fresh. The new `update_agents` kernel may hit similar
  issues because it now contains multiple behaviors × the full
  walker contributor chain. See §6.4 for the mitigation plan.

### 2.3 What to NOT touch

- **Couplings bitmask.** Rename `COUPLING_INPUT_MOVES_PAWN` to
  `COUPLING_INPUT_MOVES_PLAYER` (because that's what it describes).
  **No other coupling renames.** `COUPLING_PAWN_TO_CAMERA_TARGET`,
  `COUPLING_PAWN_TO_AURA`, etc. keep their names. They describe
  the relationship "the body the player is in relates to X"; that
  wording is still correct.
- **The aura simulation.** `compute_pawn_aura` stays. Its one input
  change: instead of reading `pawn_state.pos` for the center, it
  reads `agent_state[player.possessed_slot].pos`. That's one line.
- **The ground architecture.** `POLICY_WALKER_AGENT` exists. Don't
  add new policies; just use it.
- **Portal, mood transition core.** Hook into them (see §7.2) but
  don't restructure them.
- **Any rendering pipeline other than pawn-related.** Floaters,
  orbs, terrain, zones — untouched.

---

## 3. New Structures

### 3.1 `AgentState` (GPU + CPU)

In `state.hpp`, near `GPUPawnState` (which you'll delete):

```cpp
// Unified entity state — replaces GPUPawnState, covers player-inhabited
// body and mood-authored agents in one array.
struct GPUAgentState {
    float pos_x;       float pos_y;       float pos_z;       float t;            // 16
    float vel_x;       float vel_y;       float vel_z;       float heading;      // 32
    float home_x;      float home_y;      float home_z;      uint32_t seed;      // 48
    uint32_t behavior_id;                                                        // 52
    uint32_t tier_idx;                                                           // 56
    uint32_t is_active;                                                          // 60
    uint32_t _pad;                                                               // 64
};
static_assert(sizeof(GPUAgentState) == 64, "GPUAgentState must be 64 bytes");
static_assert(sizeof(GPUAgentState) % 16 == 0, "GPUAgentState must be 16-byte aligned");
```

`orientation` quaternion — if the existing pawn struct carries one
for render transforms — becomes a derived quantity in the agent VS
(compute the rotation matrix from `heading` and any tilt from
terrain_normal). If the orientation quaternion is actually used
(not just derived), add it as four floats and re-pad to 80 or 96 bytes.
Grep for `pawn_state.orientation` to confirm.

### 3.2 `PlayerState`

In `cartridge.hpp`, as a class field:

```cpp
// Player-owned state — survives mood transitions and body swaps.
// Physical body state (position, velocity, etc) lives in
// agentStateBuffer_[possessed_slot]; this struct is the player's
// relationship to the world.
struct PlayerState {
    uint32_t possessed_slot          = 0;
    uint32_t active_couplings        = 0;  // COUPLING_* bitmask
    float    aura_presence           = 0.0f;
    float    mmode_intensities[MMODE_COUNT] = {};
    // Camera preferences live in CameraState already; not duplicated here.
    // Future: transfer cooldown timer, last-transfer time for UI feedback.
};
PlayerState player_;
```

For the GPU side, player state propagates through existing uniforms
where needed. The critical GPU-visible field is `possessed_slot`; add
it to whichever existing uniform is ambient on compute and render
stages (probably the main `config` uniform; check `state.hpp`).
Uploads: whenever `player_.possessed_slot` changes, update the
uniform. Camera, shadow VP, aura kernels read this field.

### 3.3 Agent buffer

In `state.hpp`:

```cpp
static constexpr uint32_t MAX_AGENTS = 32;

wgpu::Buffer agentStateBuffer_;        // storage, MAX_AGENTS * sizeof(GPUAgentState)
wgpu::Buffer agentStagingBuffer_;      // for reconciliation readback (is_active column)
```

Binding slot: **replace the `pawn_state` storage binding's slot
number**. Pawn state was almost certainly binding(N) on `@group(0)`;
the agent state takes that slot. This preserves the binding budget
exactly (no slots added or removed). Agent state is `read_write` in
the compute kernel and `read` in rendering stages.

### 3.4 `PlayerState` GPU uniform

The player's `possessed_slot` needs to be readable from every compute
kernel that currently reads `pawn_state.pos`. Options:

- **Add `possessed_slot` to the existing `DesignConfig` uniform.**
  Minimal footprint, probably one spare uint slot.
- **New tiny uniform buffer** for PlayerState.

Prefer option A — piggyback on `DesignConfig`. Add a `uint32_t
possessed_slot` field. This is one line of C++ and one line of WGSL.

---

## 4. WGSL Migration (the big sweep)

### 4.1 The pattern

Every site that reads `pawn_state.X` becomes:

```wgsl
let pawn = agent_state[config.possessed_slot];
// ... use pawn.X ...
```

Introduce a single helper function to avoid repetition:

```wgsl
fn possessed_agent() -> AgentState {
    return agent_state[config.possessed_slot];
}
```

Then `pawn_state.pos.xz` becomes `possessed_agent().pos.xz`, and so
on. This keeps migration mechanical and reversible.

Add `AgentState` struct definition to `world.wgsl` (mirror of C++):

```wgsl
struct AgentState {
    pos_x: f32,       pos_y: f32,       pos_z: f32,       t: f32,
    vel_x: f32,       vel_y: f32,       vel_z: f32,       heading: f32,
    home_x: f32,      home_y: f32,      home_z: f32,      seed: u32,
    behavior_id: u32, tier_idx: u32,    is_active: u32,   _pad: u32,
}
```

Agents naturally expose scalar fields; WGSL's lack of constructor
syntax means consumers may want a helper:

```wgsl
fn agent_pos(a: AgentState) -> vec3<f32> {
    return vec3(a.pos_x, a.pos_y, a.pos_z);
}
fn agent_vel(a: AgentState) -> vec3<f32> {
    return vec3(a.vel_x, a.vel_y, a.vel_z);
}
fn agent_home(a: AgentState) -> vec3<f32> {
    return vec3(a.home_x, a.home_y, a.home_z);
}
```

### 4.2 The sites to migrate

Run `grep -n "pawn_state\|pawn\.pos\|pawn\.heading\|pawn\.velocity" world.wgsl` to
find all 50-ish sites. Categorize:

**Camera sites.** `update_camera` reads `pawn_state.pos` for orbit
center. → `possessed_agent().pos` (via `agent_pos()`).

**Shadow VP.** The shadow-VP compute kernel reads the pawn's position
to build the light-space matrix. → `possessed_agent().pos`.

**Aura site.** The aura compute kernel reads `pawn_state.pos` as the
field's center. → `agent_state[config.possessed_slot].pos`.

**Pulse origin sites.** When the radial-pulse onset is triggered, the
origin is the pawn's current XZ. → `possessed_agent().pos.xz`.

**Portal trigger sites.** Portal detection reads `pawn_state.pos` to
check proximity. → `possessed_agent().pos`.

**GoL consumer_pos sites.** Every `QueryInputs(pawn_state.pos, ...)`
call becomes `QueryInputs(possessed_agent().pos, ...)`. This is
`update_pawn`, `update_sphere`, `update_cube`, and possibly others.

**Coupling computation sites.** Any place that reads `pawn_state.pos`
or `pawn_state.velocity` to couple to something else. → agent-indirect.

**Render sites.** The existing `pawn_vs` reads `pawn_state` for
instancing. This file is being replaced wholesale (§9); its contents
are subsumed by `agent_vs`.

For each site: read the site, understand which pawn field it wants,
write the indirect version, verify the surrounding code still
compiles.

### 4.3 Verification at the end of §4

After the migration but before anything runs: the codebase should be
in a state where the existing pawn system still works *exactly as it
used to*, but it's now routed through `agent_state[0]` with
`config.possessed_slot = 0`. Slot 0 is the player. No other slots are
active yet. No possession transfer yet. No other agents.

This is the key checkpoint: **the pawn rewire is complete, with zero
behavioral change, before any new functionality is added.** If the
pawn doesn't walk, tilt, see its aura, cast a shadow, or respond to
couplings exactly as it used to, stop and diagnose.

---

## 5. C++ Migration

### 5.1 Delete `pawn_state`

`GPUPawnState` struct in `state.hpp` — delete. Its buffer
`pawnStateBuffer_` — delete (replaced by `agentStateBuffer_`).

### 5.2 Migrate all `pawn_state` references

Grep for `pawn_state` in `cartridge.hpp` and `state.hpp`. Each site
becomes a reference to slot 0 of `agentStateBuffer_` (since the
player always starts in slot 0).

If a site needs to read the player's current position from the GPU,
the readback path changes: previously read `pawnStateBuffer_`, now
read `agentStateBuffer_` with offset = `player_.possessed_slot *
sizeof(GPUAgentState)`.

If a site needs to write the player's initial position on world
spawn, same: write to `agentStateBuffer_` at the possessed_slot
offset.

### 5.3 Upload the DesignConfig change

Whenever `player_.possessed_slot` changes (world entry, Caps Lock
transfer), re-upload the `DesignConfig` uniform with the updated
field. Hook into whatever existing path updates config on significant
events (probably in the mood-apply pipeline).

---

## 6. The `update_agents` Kernel

### 6.1 Replace `update_pawn`

Delete `update_pawn`. Delete its pipeline in `renderer.hpp`.

Add `update_agents`:

```wgsl
@compute @workgroup_size(32)
fn update_agents(@builtin(global_invocation_id) gid: vec3<u32>) {
    let slot = gid.x;
    if (slot >= 32u) { return; }

    var a = agent_state[slot];
    if (a.is_active == 0u) { return; }

    let dt = signal.dt;
    a.t += dt;

    // Eviction (possessed slot exempt)
    let is_player = (slot == config.possessed_slot);
    if (!is_player) {
        let player = agent_state[config.possessed_slot];
        let dx = a.pos_x - player.pos_x;
        let dz = a.pos_z - player.pos_z;
        if (dx*dx + dz*dz > 122500.0) {  // 350²
            a.is_active = 0u;
            agent_state[slot] = a;
            return;
        }
    }

    // Behavior dispatch
    switch a.behavior_id {
        case AGENT_BEHAVIOR_PLAYER_CONTROLLED: {
            behavior_player_controlled(&a, dt);
        }
        case AGENT_BEHAVIOR_RANDOM_WALK: {
            behavior_random_walk(&a, dt);
        }
        default: {
            // Unpopulated or stubbed behavior — drift.
        }
    }

    // Integration (shared by all agents)
    let drag = 3.0;  // Pass 1 — per-behavior drag deferred.
    let decay = exp(-drag * dt);
    a.vel_x *= decay;
    a.vel_z *= decay;

    // Speed cap
    let speed_cap = 3.0;  // Pass 1 — per-behavior cap deferred.
    let vel2 = a.vel_x * a.vel_x + a.vel_z * a.vel_z;
    if (vel2 > speed_cap * speed_cap) {
        let s = speed_cap / sqrt(vel2);
        a.vel_x *= s;
        a.vel_z *= s;
    }

    a.pos_x += a.vel_x * dt;
    a.pos_z += a.vel_z * dt;

    // Heading (tracks velocity for moving agents)
    if (vel2 > 0.01) {
        a.heading = atan2(a.vel_x, a.vel_z);
    }

    // Ground resolve (policy depends on who's driving)
    let qi = QueryInputs(
        vec3(a.pos_x, a.pos_y, a.pos_z),
        signal.t_seconds
    );
    if (is_player) {
        // Full walker policy with step-climb for smooth player movement.
        // Call pawn_ground_resolve (keep its name as a term of art) —
        // the function body is unchanged; it just takes qi now.
        let res = pawn_ground_resolve(
            vec2(a.pos_x, a.pos_z),
            vec2(/* prev_xz */ ...), /* prev_y */ ...,
            qi
        );
        a.pos_x = res.x;
        a.pos_y = res.y;
        a.pos_z = res.z;
    } else {
        // Simple snap for non-player agents.
        a.pos_y = query_ground_walker_agent(vec2(a.pos_x, a.pos_z), qi);
    }

    agent_state[slot] = a;
}
```

### 6.2 `behavior_player_controlled`

This is the body of the old `update_pawn` **minus the ground resolve
and shell** (which now live in the outer kernel):

```wgsl
fn behavior_player_controlled(a: ptr<function, AgentState>, dt: f32) {
    if (coupling_active(COUPLING_INPUT_MOVES_PLAYER)) {
        let input_dir = vec2(signal.move_x, signal.move_z);
        let world_vel = coupling_input_to_pawn_velocity(
            input_dir, camera_state.azimuth
        );
        let speed = select(PAWN_SPEED, config.pawn_speed,
                           config.pawn_speed > 0.0);

        (*a).vel_x = world_vel.x * speed;
        (*a).vel_z = world_vel.y * speed;

        if (fpv_mode_active()) {
            (*a).heading = camera_state.azimuth;
        } else {
            (*a).heading = coupling_velocity_to_pawn_heading(
                world_vel, (*a).heading, dt
            );
        }
    } else {
        (*a).vel_x = 0.0;
        (*a).vel_z = 0.0;
        if (fpv_mode_active()) {
            (*a).heading = camera_state.azimuth;
        }
    }
    // Finite world bound clamp — was in update_pawn, moves here.
    if (config.world_bound_max.x > 0.0) {
        (*a).pos_x = clamp((*a).pos_x,
                           config.world_bound_min.x,
                           config.world_bound_max.x);
        (*a).pos_z = clamp((*a).pos_z,
                           config.world_bound_min.y,
                           config.world_bound_max.y);
    }
}
```

Note the `COUPLING_INPUT_MOVES_PAWN` → `COUPLING_INPUT_MOVES_PLAYER`
rename. Apply it everywhere (including the CPU-side coupling bit
definitions).

### 6.3 `behavior_random_walk`

```wgsl
fn behavior_random_walk(a: ptr<function, AgentState>, dt: f32) {
    const STEP_RATE: f32 = 0.8;       // 1 step per 1.25s
    const STEP_SIZE: f32 = 1.5;
    let step_interval = 1.0 / STEP_RATE;

    let prev_t = (*a).t - dt;
    let crossed = floor((*a).t / step_interval)
                > floor(prev_t / step_interval);
    if (crossed) {
        let step_idx = u32(floor((*a).t / step_interval));
        let h = (*a).seed ^ (step_idx * 2654435761u);
        let angle = f32(h & 0xFFFFu) / 65535.0 * 6.2831853;

        // Apply impulse. Dividing by dt cancels the drag's per-frame
        // decay for this frame's impulse — the total delta lands at
        // STEP_SIZE irrespective of frame rate.
        let impulse = STEP_SIZE / dt;
        (*a).vel_x += cos(angle) * impulse * dt;
        (*a).vel_z += sin(angle) * impulse * dt;
    }
}
```

### 6.4 Compile-time budget

If the new `update_agents` compiles in under 35 seconds, accept and
move on. If it compiles in 35–50, consider splitting the kernel into
`update_player_agent` (1 thread, just the PlayerControlled branch) +
`update_other_agents` (32 threads, excluding the player slot). If it
compiles in 50+, split immediately.

The split is straightforward:
- Keep `behavior_player_controlled` + `pawn_ground_resolve` +
  `terrain_normal_at` + full walker policy in `update_player_agent`.
- Keep `behavior_random_walk` + simple ground snap in
  `update_other_agents`.
- Dispatch both from the CPU in sequence.

Don't split proactively; observe first.

### 6.5 Update the pipeline in `renderer.hpp`

Delete `update_pawn` pipeline creation. Add `update_agents` pipeline
with:
- Same bind group layout as the old `update_pawn` pipeline, but
  with `agent_state` replacing `pawn_state` at the same binding
  slot.
- Dispatch shape: `(1, 1, 1)` with workgroup_size 32 (one
  workgroup, 32 threads).

---

## 7. Possession Transfer

### 7.1 The input handler

The `input.inl` module (or wherever key events are processed) gets a
new handler for Caps Lock:

```cpp
void handle_caps_lock_pressed() {
    // Find nearest active non-player agent within 15 world units.
    const GPUAgentState& player_body = cpuAgents_[player_.possessed_slot];
    vec3 player_pos = { player_body.pos_x, player_body.pos_y, player_body.pos_z };

    int32_t nearest_slot = -1;
    float nearest_dist_sq = 15.0f * 15.0f;

    for (uint32_t i = 0; i < MAX_AGENTS; i++) {
        if (i == player_.possessed_slot) continue;
        if (!cpuAgents_[i].is_active) continue;
        float dx = cpuAgents_[i].pos_x - player_pos.x;
        float dz = cpuAgents_[i].pos_z - player_pos.z;
        float d2 = dx*dx + dz*dz;
        if (d2 < nearest_dist_sq) {
            nearest_dist_sq = d2;
            nearest_slot = (int32_t)i;
        }
    }

    if (nearest_slot < 0) return;  // no target, silent no-op

    // Swap behaviors
    cpuAgents_[player_.possessed_slot].behavior_id = AGENT_BEHAVIOR_RANDOM_WALK;
    cpuAgents_[nearest_slot].behavior_id            = AGENT_BEHAVIOR_PLAYER_CONTROLLED;

    // Update player state
    player_.possessed_slot = (uint32_t)nearest_slot;

    // Upload changes
    queue_.WriteBuffer(agentStateBuffer_, 0,
                      cpuAgents_, MAX_AGENTS * sizeof(GPUAgentState));
    queue_.WriteBuffer(configBuffer_, offsetof(DesignConfig, possessed_slot),
                      &player_.possessed_slot, sizeof(uint32_t));
}
```

Note the critical detail: the CPU shadow of agent state
(`cpuAgents_`) needs to be **read back periodically** so the nearest-
agent check uses current GPU positions. Reconciliation already runs
(§8); ensure it reads positions, not just `is_active`. Pass 1 may
want to simply read back positions on every Caps Lock press for
correctness — it's one readback per infrequent user action, cheap.

### 7.2 Camera glide

The camera already has a target position and a damping curve
(`update_camera` in WGSL). On possession transfer, the camera's
target changes (because `config.possessed_slot` changed and the
camera reads through it). The existing damping will cause the camera
to glide toward the new position.

**If the damping is too slow** — camera lags behind the player's new
body noticeably — bump the damping coefficient transiently for
~1 second after a transfer. A simple implementation: store
`last_transfer_time` in `PlayerState`, and in the camera kernel,
check `signal.t_seconds - config.last_transfer_time < 1.0` to use a
fast-glide coefficient.

Most likely the existing damping is already fast enough for the
transfer distances involved (under 15 units). Verify by testing.

### 7.3 Transferred state

Nothing else needs to move. `player_.active_couplings`,
`player_.aura_presence`, `player_.mmode_intensities` — all remain
untouched. The aura compute kernel continues running; on the next
frame it just centers on the new body. The couplings bitmask
continues to apply; the couplings now read state from the new slot.

---

## 8. Mood Transition

### 8.1 Player persistence

The existing mood-transition code calls something like
`teardown_world()` or `apply_mood()`. Find that path. On mood change,
the pattern becomes:

```cpp
void on_mood_change(MoodId new_mood) {
    // Preserve slot 0 (the player's body).
    GPUAgentState preserved_player = cpuAgents_[player_.possessed_slot];

    // If the player is not in slot 0 (they possessed another body
    // before this transition), move them to slot 0 and clear the
    // slot they were in. This is the "clean slate for slots 1..31"
    // rule.
    if (player_.possessed_slot != 0) {
        cpuAgents_[0] = preserved_player;
        cpuAgents_[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
        cpuAgents_[player_.possessed_slot] = {};  // inactive
        player_.possessed_slot = 0;
    }

    // Clear slots 1..31
    for (uint32_t i = 1; i < MAX_AGENTS; i++) {
        cpuAgents_[i] = {};
    }

    // PlayerState (couplings, aura, mmodes) — untouched. Couplings
    // may be re-set by apply_mood below; that's the mood's prerogative.

    // Apply mood (existing path)
    apply_mood(new_mood);

    // Spawn new population
    spawn_agents_for_mood(new_mood);

    // Upload everything
    queue_.WriteBuffer(agentStateBuffer_, 0,
                      cpuAgents_, MAX_AGENTS * sizeof(GPUAgentState));
}
```

The decision "always move the player back to slot 0 on mood change"
is a simplification — it means the player always controls slot 0, and
slots 1..31 are always the mood-authored population. The alternative
(preserve the player's current slot index) would work but introduces
edge cases when the new mood authors a count that covers the player's
slot.

### 8.2 `spawn_agents_for_mood`

Look up the mood's `AgentPopulationDef`. For each of the `count`
non-player slots (1..count), roll a behavior from behavior_weights,
roll a tier from tier_weights, place at a random position within
`spawn_radius` of the player, seed the home offset. Per the design
doc §5.3 and §6.1.

Use stable seeding (world seed + mood id + slot index) so a given
mood's agent layout is reproducible across session restarts.

### 8.3 Aura handling on transition

The design doc §11 flags "aura persistence across portals" as a
separate concern to revisit later. For Pass 1, the aura behaves
however it currently behaves (resets to on or off per the mood's
default) — the unification doesn't change this.

---

## 9. Rendering

### 9.1 Replace `pawn_vs` with `agent_vs`

`pawn_vs`'s mesh generation logic moves wholesale to `agent_vs`. The
only changes:
1. Read state from `agent_state[inst]` instead of from `pawn_state`.
2. Inactive slots collapse to degenerate triangles (position behind
   near plane).
3. Optionally, add a subtle visual distinction for the possessed
   slot (small brightness bump, thin outline). Pick one and
   document.

```wgsl
@vertex
fn agent_vs(@builtin(vertex_index) vid: u32,
            @builtin(instance_index) inst: u32) -> AgentVarying {
    let a = agent_state[inst];
    if (a.is_active == 0u) {
        var degen: AgentVarying;
        degen.clip_pos = vec4(0.0, 0.0, -1.0, 1.0);
        degen.color    = vec3(0.0);
        return degen;
    }

    // --- Chess piece mesh generation — copy from the existing pawn_vs ---
    // ... profile sampling, revolution around Y, etc. ...

    // --- Per-instance transform ---
    let c = cos(a.heading);
    let s = sin(a.heading);
    let world_x = a.pos_x + (local_x * c - local_z * s);
    let world_y = a.pos_y + local_y;
    let world_z = a.pos_z + (local_x * s + local_z * c);

    // Color: tier + possessed distinction
    var color = agent_tier_color(a.tier_idx);
    if (inst == config.possessed_slot) {
        color = color * 1.15;  // subtle brightness bump
        // Alternative: outline in fragment shader, or slight scale
    }

    // Output (mirror the existing pawn_vs's varying shape)
    var out: AgentVarying;
    out.clip_pos   = render_vp.vp * vec4(world_x, world_y, world_z, 1.0);
    out.color      = color;
    out.world_pos  = vec3(world_x, world_y, world_z);
    // ... plus any other varyings pawn_vs had (normal, uv, etc.) ...
    return out;
}

fn agent_tier_color(tier_idx: u32) -> vec3<f32> {
    switch tier_idx {
        case 0u: { return vec3(0.60, 0.62, 0.65); }  // Worker
        case 1u: { return vec3(0.85, 0.65, 0.40); }  // Scout
        case 2u: { return vec3(0.30, 0.40, 0.70); }  // Sentinel
        case 3u: { return vec3(0.95, 0.85, 0.55); }  // Leader
        default: { return vec3(1.0, 0.0, 1.0); }     // magenta on error
    }
}
```

### 9.2 `agent_fs`

Copy `pawn_fs`. If the pawn FS does lighting, shadow sampling, aura
color modulation — all of that applies the same way to any agent.
Tier color feeds through via the varying.

### 9.3 Shadow pass

Replace `shadow_pawn_vs` with `shadow_agent_vs`. Same shape as
`agent_vs` but minimal (position only, no color). Instanced, 32
instances, inactive slots collapse.

### 9.4 Render pipeline

One pipeline for all agents (forward pass) + one pipeline for shadows.
In `renderer.hpp`: replace the pawn render pipeline with the agent
render pipeline. Draw call: `DrawIndexed(pawn_mesh_indices, MAX_AGENTS, 0, 0, 0)`
— one draw, 32 instances, VS does the rest.

---

## 10. Module: `modules/agents.inl`

### 10.1 Skeleton

```cpp
// ─── agents.inl ─────────────────────────────────────────────
// Unified entity registry: behaviors, tier gains, and per-mood
// populations. The player inhabits slot 0; slots 1..31 are mood-
// authored populations.
//
// See: agent_system_design.md for rationale.

enum AgentBehaviorId : uint32_t {
    AGENT_BEHAVIOR_PLAYER_CONTROLLED = 0,
    AGENT_BEHAVIOR_RANDOM_WALK       = 1,
    AGENT_BEHAVIOR_CORRELATED_WALK   = 2,  // stub (Pass 2)
    AGENT_BEHAVIOR_WANDERER          = 3,  // stub
    AGENT_BEHAVIOR_HOME_SEEKER       = 4,  // stub
    AGENT_BEHAVIOR_PATROL            = 5,  // stub
    AGENT_BEHAVIOR_PURSUIT           = 6,  // stub
    AGENT_BEHAVIOR_FLEE              = 7,  // stub
    AGENT_BEHAVIOR_FLOCK2D           = 8,  // stub
    AGENT_BEHAVIOR_LEVY_FLIGHT       = 9,  // stub
    AGENT_BEHAVIOR_COUNT             = 10,
};

enum AgentTierId : uint32_t {
    AGENT_TIER_WORKER   = 0,
    AGENT_TIER_SCOUT    = 1,
    AGENT_TIER_SENTINEL = 2,
    AGENT_TIER_LEADER   = 3,
    AGENT_TIER_COUNT    = 4,
};

struct AgentBehaviorDef { /* ... */ };
struct AgentTierDef     { /* ... */ };
struct AgentPopulationDef { /* ... */ };

static constexpr AgentBehaviorDef AGENT_BEHAVIORS[AGENT_BEHAVIOR_COUNT] = {
    // PlayerControlled: all zero, handled by kernel switch case
    { AGENT_BEHAVIOR_PLAYER_CONTROLLED, "player_controlled",
      /* parameters unused — body reads input directly */ },
    // RandomWalk: real parameters
    { AGENT_BEHAVIOR_RANDOM_WALK, "random_walk",
      /*step_rate=*/ 0.8f, /*step_size=*/ 1.5f,
      /*persistence=*/ 0.0f, /*drag=*/ 3.0f,
      /*home_pull=*/ 0.0f, /*speed_cap=*/ 3.0f },
    // ... remaining behaviors stubbed with zeros (Pass 2)
};

static constexpr AgentTierDef AGENT_TIER_GAINS[AGENT_TIER_COUNT] = {
    // id, name, step_gain, persist_gain, speed_gain, coupling_gain,
    // home_gain, weight, color_r, color_g, color_b
    { AGENT_TIER_WORKER,   "worker",   1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 4.0f,
                                        0.60f, 0.62f, 0.65f },
    { AGENT_TIER_SCOUT,    "scout",    1.8f, 0.4f, 1.4f, 1.0f, 0.5f, 2.0f,
                                        0.85f, 0.65f, 0.40f },
    { AGENT_TIER_SENTINEL, "sentinel", 0.6f, 1.2f, 0.5f, 1.0f, 2.0f, 1.0f,
                                        0.30f, 0.40f, 0.70f },
    { AGENT_TIER_LEADER,   "leader",   1.2f, 0.9f, 1.1f, 2.5f, 0.8f, 0.3f,
                                        0.95f, 0.85f, 0.55f },
};

// Per-mood population authoring.
static constexpr AgentPopulationDef AGENT_POPULATIONS[] = {
    // open_default: 6 Workers/Scouts doing RandomWalk
    { MOOD_OPEN_DEFAULT, /*count=*/ 6,
      { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },  // all RandomWalk
      { 2, 2, 1, 0 },                     // Worker + Scout mix
      /*spawn_radius=*/ 50.0f,
      /*home_seeding_radius=*/ 5.0f },
    // open_sunset: 4 agents, Scout-heavy
    { MOOD_OPEN_SUNSET, /*count=*/ 4,
      { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 1, 3, 0, 0 },
      /*spawn_radius=*/ 60.0f,
      /*home_seeding_radius=*/ 8.0f },
    // other moods: count=0 (omit from table or add explicit zero rows)
};

static_assert(sizeof(AGENT_BEHAVIORS)/sizeof(AGENT_BEHAVIORS[0]) == AGENT_BEHAVIOR_COUNT);
static_assert(sizeof(AGENT_TIER_GAINS)/sizeof(AGENT_TIER_GAINS[0]) == AGENT_TIER_COUNT);
```

Match `orbs.inl`'s pattern for mood lookup — probably a helper like
`find_population_for_mood(MoodId)` that searches the table.

### 10.2 Include site

Include `modules/agents.inl` from `cartridge.hpp` in a sensible
place, alongside other module includes:

```cpp
#include "modules/orbs.inl"
#include "modules/agents.inl"
#include "modules/pawn_aura.inl"
```

---

## 11. Execution Steps

Commit after each step. Verify before proceeding.

### Step 1 — Branch + structs

- Git branch + archive tag.
- Create `AgentState` struct and agent buffer.
- Create `PlayerState` struct with `possessed_slot = 0` default.
- Add `possessed_slot` field to `DesignConfig` uniform.
- `static_assert`s on struct sizes.
- Scaffold `modules/agents.inl` with registries (empty behavior bodies
  are fine at this step).

**Verify:** build succeeds. No behavior change. `GPUPawnState` still
exists.

### Step 2 — Unified buffer, pawn as slot 0

- Replace `pawnStateBuffer_` with `agentStateBuffer_`.
- On world spawn, initialize `cpuAgents_[0]` with the data that used
  to go into `pawn_state`.
- Delete `GPUPawnState`.
- Migrate every `pawn_state.*` reference in WGSL to
  `agent_state[config.possessed_slot].*` (or through the
  `possessed_agent()` helper).
- Migrate every C++ reference to `pawn_state` to
  `cpuAgents_[player_.possessed_slot]`.
- Keep the old `update_pawn` kernel alive but have it operate on
  `agent_state[0]` instead of the deleted `pawn_state`. This is an
  interim state.

**Verify:** build succeeds. Pawn walks identically to pre-refactor.
Camera, aura, shadow all correct. This is the critical checkpoint —
the rewire must be transparent before any new functionality lands.

### Step 3 — Unified kernel

- Delete `update_pawn` kernel and its pipeline.
- Create `update_agents` kernel with `PlayerControlled` branch only
  (everything else stubbed).
- The `PlayerControlled` body is the old `update_pawn` body, moved
  into a switch case.
- Create the `update_agents` pipeline, dispatch 32 threads in one
  workgroup.
- Only slot 0 is active; slots 1..31 are inactive.

**Verify:** build succeeds. Pawn still walks identically. Compile
time for `update_agents` should be close to pre-unification
`update_pawn` (around 25-30s).

### Step 4 — Add RandomWalk + spawn

- Implement `behavior_random_walk` in WGSL.
- Implement `spawn_agents_for_mood` on CPU.
- Wire it into mood transition.
- Author `open_default` with count=6 RandomWalkers.

**Verify:** entering `open_default` spawns 6 agents that wander
randomly around the player. They ground-resolve correctly. The
player is unaffected.

### Step 5 — Rendering

- Replace `pawn_vs` with `agent_vs` (full 32-instance draw).
- Replace `pawn_fs` with `agent_fs` (or keep the same FS if shared
  across agents).
- Replace the pawn render pipeline with the agent render pipeline.
- Shadow pass: `shadow_agent_vs` replaces `shadow_pawn_vs`.

**Verify:** the player still renders (from slot 0). Agents render
(from slots 1..6 in open_default). They cast shadows. Tier colors
are visible. The possessed slot is visually distinguished.

### Step 6 — Eviction

- Add the `is_player` exemption to the eviction check in the kernel.
- Walk the player 350+ units in one direction; verify agents
  disappear behind.

**Verify:** eviction works from the player's reference, not from the
world origin.

### Step 7 — Possession transfer

- Caps Lock input handler on CPU.
- Nearest-within-15 agent targeting.
- Behavior swap + possessed_slot update + upload.
- Verify camera glides to the new body.

**Verify:** Caps Lock near an agent swaps the player into that body.
The previous body continues RandomWalking. Couplings and aura travel
with the player.

### Step 8 — Mood transition hardening

- Ensure slot 0 is preserved across mood changes.
- If the player is in a slot other than 0 at transition time, move
  them back to slot 0.
- Verify new moods spawn fresh populations into slots 1..31 while the
  player persists.

**Verify:** portal through a mood change, then another; the player
persists, populations change.

### Step 9 — Diagnostic logging

- `[AGENTS]` log on spawn, evict, transfer.

---

## 12. Out of Scope

- All behaviors other than `PlayerControlled` and `RandomWalk`.
  Pass 2.
- Floater-to-agent binding. Pass 3.
- Aura persistence across portals — design doc §11, revisit later.
- Agent-agent collision.
- Music coupling for non-player agents (per-behavior coupling gains).
  Pass 2.
- Multi-camera / camera-not-on-player.
- Disembodied observer state.
- Combo transfers (Caps Lock + direction).
- Transfer cooldowns or cost.

---

## 13. Questions to Raise

- **If `update_agents` compile time exceeds 50 seconds** at Step 3,
  stop and report. Fallback plan: split into `update_player_agent`
  (1 thread, PlayerControlled only) + `update_other_agents` (32
  threads, other behaviors). Details in §6.4.
- **If `pawn_state.orientation` is used in rendering** rather than
  just derived from heading+tilt, flag. The struct design may need a
  quaternion field (and struct padding adjusts to 80 or 96 bytes).
- **If the `possessed_agent()` helper triggers FXC inlining issues**,
  stop and consider passing the agent state into consumer functions
  rather than re-reading at each site. Unlikely; flagged
  defensively.
- **If the possessed-agent visual distinction doesn't "read" visually**
  (too subtle or too loud), mark it; we'll iterate after Pass 1 lands.
- **If any pawn-coupled system regresses** after Step 2 (the
  critical "transparent rewire" checkpoint) — camera jitter, aura
  misalignment, shadow glitch, coupling silence — stop immediately.
  Step 2 must be a complete no-op semantically before Step 3 starts.
- **If a mood change causes the player to lose their body or state**,
  stop. The mood transition is the most state-sensitive path in the
  refactor.

---

## 14. Commit Messages

- `agents: branch + AgentState/PlayerState structs`
- `agents: pawn migrates to slot 0 of agent buffer (transparent rewire)`
- `agents: update_pawn becomes update_agents with PlayerControlled branch`
- `agents: RandomWalk behavior + spawn_agents_for_mood`
- `agents: unified rendering (agent_vs + shadow_agent_vs)`
- `agents: player-centered eviction for non-player slots`
- `agents: possession transfer via Caps Lock`
- `agents: mood transition preserves player slot`
- `agents: diagnostic logging`

---

## 15. Success

After Pass 1, the session looks like this: the player walks around in
a chess-piece body (indistinguishable from pre-refactor in feel).
Entering a mood with agents authored (open_default or open_sunset),
they see other chess pieces — in different tier colors — wandering
nearby. Pressing Caps Lock near one of them, the player becomes that
piece. Their aura, couplings, mmode intensities, and camera all come
with them. The body they left continues to wander. The world has not
changed; only who they are in it has.
