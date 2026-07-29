> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# THE FRAME CONDUCTOR — RECON (read-only; the per-frame sentence mapped)

Campaign: render/update organizational cleanup. This is the LAST organ — the
frame's own sentence: `update()` + `render()` + `render_passes` + the two
submits. Builds on **RENDER_UPDATE_API_RECON.md** (layer-3: 3 call sites,
O-5b/c staging→upload, the hidden 2nd submit, 3 hand-synced draw lists) and the
accreted **LADDER** edge-notes (pipeline node-types, the four upload cadences,
the hot-field list). Line numbers are HEAD at recon time (post-`cb4ddaf`, i.e.
after the C1–C4 + C2 cuts); a future cut re-greps.

DRIVER LAW (v3 constitution §9, `direction/input.hpp:102` — "a driver writes
intents through bodies it does not own"). The four clocks each movement runs on:
- **input** — player intents (move/look/zoom/pan deltas, harvested by on_input).
- **music** — the `AnalysisSignal` (t_beats, t_seconds, `stats[]` audio features).
- **algo** — autonomous logic independent of clock/music: streaming cadence,
  GPU-reported events (portal trigger, slot eviction), dirty-set draining.
- **wall-clock** — real-time `dt`: presence ramps, transition timer, smoothing.

---

## §0 THE ONE-LINE DIAGNOSIS

**The frame is a phase DAG written as a linear comment-annotated call sequence.**
The conductor already *names* its phases — the code is dense with `MOVEMENT:`
banners (CLOCK, S2 SURFACE, S4 MOTION, WITNESS HARVEST/CAPTURE, REALIZATION) and
`O-1..O-7` ordering constraints — but every one of those names lives in a comment
and every one of those orderings is held by hand call-order, not by structure.
The frame's *truth* (what clock we're on, what's dirty, which phase reads which
face) is smeared across two functions, four files, ~7 ad-hoc dirty booleans, four
readback state-machines, a hidden second submit, and three separately-enumerated
draw lists. Reorder two movements and it still compiles; drop a draw from one of
three lists and the entity silently vanishes from shadows or snapshots. The
conductor is straining toward a declared phase sequence and getting there in
prose.

---

## §1 THE TIMELINE — the per-frame sequence, in execution order

One frame = **`update()` → `render()` → host `Finish()`+`Submit()`**
(`incubator.cpp:254-255`, per RU-recon [L3-a]), with a **hidden second submit**
firing mid-`render()` on GoL-derive frames. `update()` carries CPU state + queue
writes and holds **no encoder**; `render()` encodes GPU work into the host's one
encoder *and* issues more queue writes. A maintainer reading only `render()`
misses the clock, the drivers, the transition machine, and both uploads.

### 1a — `update()` (`cartridge.hpp:568-797`) — CPU + queue writes, no encoder

| # | movement | line | driver | reads → writes |
|---|---|---|---|---|
| U1 | signal fill (analysis + input deltas; `dt_beats`) | 576-594 | music+input+wall-clock | `signal`, `inputState_`, `time_state_.prev_beats` → `gpuSignal` (CPU) |
| U2 | sky words → **neutral zeros** (SNAP-1) | 606-615 | — | → `gpuSignal.sky_*` (placeholders; real author is R7) |
| U3 | clock advance (`beats/seconds/dt/beat_rate`, bump `prev_beats`) | 617-625 | music+wall-clock | `signal`, `prev_beats` → `time_state_` |
| U4 | `visual_canvas_.tick` + fog stage | 631-638 | **music** | `signal`, canvas params → `visual_canvas_`, config(fog) |
| U5 | `tick_pawn_couplings` (presence ramp + aura height) | 643-644 | wall-clock | `pawn_state_`, `dt` → `pawn_state_`, aura buf (queue) |
| U6 | world seed + finite bounds stage (pre-machine, RC policy) | 652-660 | algo | `world_state_` → config(seed/bounds) |
| U7 | **the transition machine** (FADE_OUT / TEARDOWN / FADE_IN) | 662-775 | input-triggered + wall-clock | `transitionPhase_`, `dt`, `pendingDestination_` → `world_state_`, `mood_state_`, **every organ** (teardown verbs), agents, GPU (queue) |
| U8 | fade stage → **`upload_signal`** → **`upload_config`** (O-5b/c) | 780-784 | — | `fade_alpha`, `gpuSignal`, config → **GPU signal buf, GPU config buf** |
| U9 | `update_photographer` (gallery only) | 791-792 | algo | `gallery_state_` → photographer VP (queue) |
| U10 | `clear_input_deltas` (O-5e, dead-last) | 796 | — | → `input_deps_` (zeroes deltas) |

U7 is the whale: on a transition frame it re-seeds the world, runs a teardown
verb per owner (`teardown_surface/entities/gol/ribbon/…`, 703-721), resets the
player agent, `apply_mood`, and repopulates — all inside `update()`, all before
the two uploads at U8.

### 1b — `render()` (`cartridge.hpp:805-1196`) — GPU encode + queue writes

| # | movement | line | driver | reads → writes |
|---|---|---|---|---|
| R1 | **WITNESS HARVEST** — pawn/floater/camera readback maps (async) | 817-920 | algo (GPU→CPU) | *last frame's* staging → `player_.readback_x/z`, `portal_trigger`, sphere/cube mirrors |
| R2 | portal trigger → arm transition | 926-940 | algo (GPU event) | `readback_portal_trigger`, arches → `transitionPhase_`=FADE_OUT, `pendingDestination_` |
| R3 | `stream_patches` (S2 conductor; carries S3-trigger seam) | 946 | algo | `readback_x/z`, tile world → patch bufs + **encoder** (patch-gen compute) |
| R4 | `respawn_evicted_agents` (S3; RC-1: after stream) | 956-957 | algo | `agent_state_` → agent slots (queue) |
| R5 | `tick_cube_corral_animations` (S4; RC-2: after stream) | 965-966 | wall-clock | `cube_behaviors_` → cube fields (queue) |
| R6 | census dumps + GoL residue check | 968-1012 | wall-clock (interval) | `time_state_` → stdout |
| R7 | `ribbon_frame_tick` (+ **tail sky resync**, `resync_sky_head`) | 1021-1022 | music+wall-clock | `ribbon_state_`, `signal` → ribbon bufs, **sky words** (queue, overwrites U2) |
| R8 | entity mesh gen: **12 `prepare_mesh` → `dirty[]` → one compute pass** | 1026-1096 | algo (dirty) | entities → mesh params (queue) + mesh VB/IB (compute) |
| R9 | `upload_portal_array` + `upload_lights` | 1097-1098 | algo | `mood_deps_` → portal/light bufs (queue) |
| R10 | **`dispatch_compute`** (7 dispatches; §1c) | 1103 | music+input+algo | signal, entity/camera state, aura → agent/camera/sphere/cube/VP (compute) |
| R11 | **WITNESS CAPTURE** — agent/floater/camera → staging (O-2) | 1108-1134 | — | GPU bufs → staging (encoder copy; feeds R1 next frame) |
| R12 | GoL zone compute (if `zone_count>0`): **`flush_zone_derive` [HIDDEN SUBMIT]** + config upload + sync/evolve/mesh (3 passes = O-6a) | 1143-1151 | algo (GoL) | `gol_state_` → zone bufs, **2nd `queue.Submit`** |
| R13 | `dispatch_pawn_aura` | 1158-1159 | wall-clock | `pawn_state_` → aura texture (compute) |
| R14 | orb sky: init / recolor / copy_prev / dynamics | 1165-1170 | algo+music | `orbs_state_`, signal → orb bufs (compute) |
| R15 | `ground_entries_dirty` → **`upload_ground_entries`** → set `placement_dirty` | 1172-1176 | algo (dirty) | entities `cached_ground_y` → ground-origin bufs (queue) |
| R16 | `placement_dirty` → `dispatch_placement_correction` | 1177-1180 | algo (dirty) | ground origins → placement Y (compute) |
| R17 | `dispatch_frustum_cull` (O-7 tail) | 1185 | algo | camera/patch → frustum indirect buf (compute + copy) |
| R18 | `render_shadow_pass` → `draw_shadow_all` (§1d) | 1187 | — | shadow VP, entity bufs → shadow map (render pass) |
| R19 | `render_main_pass` (§1d) | 1188 | — | render VP, entity bufs → backbuffer (render pass) |
| R20 | `render_snapshot_pass` (**3rd draw list**, `gallery.hpp:1123`) | 1189 | algo (gallery cadence) | `gallery_state_` → snapshot texture (render pass) |
| R21 | `drain_gallery_promotions` | 1194-1195 | algo | gallery pending → exhibition (encoder) |

### 1c — `dispatch_compute` (`render_passes.hpp:172-224`) — one pass, 7 dispatches

All share `compute_entity_group` (+ `compute_texture_group` for the aura/sampler).
Order: ribbon_rings (if `rendered_slot!=UINT32_MAX`) → **[update_terrain_config
removed by RAYMARCH excavation, note @185-186]** → update_player_agent
(POLICY_WALKER) → update_other_agents (POLICY_WALKER_AGENT) → update_camera
(POLICY_FLYER) → update_sphere → update_cube → compute_vp. `compute_vp` runs
**last** — every VP-consumer downstream reads a VP produced this frame.

### 1d — the three draw lists (the [L3-d] divergence, now precise)

| draw | shadow list (`rp:315-430`) | main list (`rp:458-617`) | snapshot (`gallery.hpp:1123`) |
|---|---|---|---|
| bind groups | `render_entity_group` + **`shadow_texture_group`** | `render_entity_group` + **`render_texture_group`** (gallery/orb/fade use their own) | `photographer_*` groups |
| terrain LOD0/1 | ✓ (direct) | ✓ (LOD0 indirect if outdoor) | ✓ |
| zone / pawn / sphere / monolith / arch / column / palm / cactus / blade / shell | ✓ | ✓ | (subset) |
| **ribbon** | position 6 (after monolith) | position 13 (after shell) | — |
| **wall_paintings, gallery_frames** | ✗ | ✓ (@584/601) | ✓ |
| **orbs, fade overlay** | ✗ | ✓ (@608/611) | ✗ |
| **pyramid** | ✗ (C2: never drawn) | ✗ (C2) | ✗ |

Three enumerations, three orders, three bind-group conventions, **zero shared
source**. A new drawable is a 3-site coordinated edit; miss one and it silently
drops from shadows or snapshots. Ribbon already sits in a *different ordinal
position* between the two lists — proof the lists drifted, not merely diverged.

---

## §2 THE EDGES — cross-stage read-after-write + dirty dependencies

**Silent, comment-guarded (each fails with no compile/runtime error):**

- **[E-1] O-5b/c staging→upload (`update:780-784`) — highest frequency.** Every
  config/signal setter must land before `upload_signal`/`upload_config`. A setter
  placed after either upload is silently dropped for the frame. The split into
  two uploads doubles the trap surface. (RU-recon [L3-b].)
- **[E-2] O-5a `dt_beats` reads `prev_beats` before U3 advances it**
  (`594` reads, `624` writes). Reorder = one-frame beat skew → step-trigger drift.
- **[E-3] The sky-word relay is a THREE-writer, cross-call-site chain.** U2
  (`update:606-615`) writes neutral zeros into `gpuSignal.sky_*`; U8 uploads the
  whole signal struct; R7 (`render:1021`, `resync_sky_head`) overwrites *just* the
  sky words via an offset write; R10 `dispatch_compute` reads them. Correctness
  rests on submission order across **two functions**: resync must come after the
  U8 whole-struct upload (it does — update precedes render) and before R10 (it
  does — 1021 < 1103). Neutral-in-`update` is deliberate (loss fails LOUD, pawn to
  origin, not one frame stale) — but the invariant is a paragraph, not a type.
- **[E-4] O-2 capture-after-compute (`render:1108` after `1103`).** The WITNESS
  CAPTURE copies land after `dispatch_compute` so next frame's HARVEST (R1) reads
  post-compute state. The readback is **1 frame stale by design** — player
  position, portal trigger, sphere/cube mirror all lag one frame. Downstream
  consumers (R3 stream center, R2 portal, R4 respawn) eat the harvest value; RC-1
  (`render:949-952`) asserts "no data edge" *because* the harvest refreshes before
  the stream reads — an invariant held by call-order.
- **[E-5] O-7 frustum cull → indirect draw (`render:1185` → `rp:461`).** The cull
  writes the indirect buffer; `draw_patch_terrain_lod0_indirect` consumes it a
  pass later. Cross-function data dep threaded through `frustum_indirect_lod0`,
  comment-guarded.
- **[E-6] the ground-entries → placement cascade (`render:1172-1180`).** A
  two-step *same-frame* dirty chain: `ground_entries_dirty` raises
  `placement_dirty` inside its own handler (`1174`), whose handler then runs the
  correction. Order within the frame is fixed by the two adjacent `if`s; a future
  setter of `placement_dirty` *after* line 1177 waits a frame silently.
- **[E-7] RC-1 / RC-2 disjointness by prose (`render:949-966`).** "respawn touches
  slots 1+ only" and "corral + patch-eviction touch disjoint cube fields" are
  correctness invariants guarded by comment; a future overlap corrupts silently.

**The hidden submit as an edge:**

- **[E-8] the 2nd submit (`gol_zones.hpp:577-587`, call site `render:1146`).**
  `flush_zone_derive_requests` builds its **own encoder and Submits** before the
  host submit. Its derived zone config must be visible to `dispatch_zone_sync`
  (`1148`) — which holds only because the separate submit completes before the
  main encoder's zone dispatches are themselves submitted. "One encoder, one
  submit" reasoning from `render()` is **wrong on any GoL-derive frame**. Invisible
  at the call site.

**Cross-FRAME edge (the subtle one):**

- **[E-9] portal → transition spans a frame boundary.** R2 (`render:934`) sets
  `transitionPhase_ = FADE_OUT` from a GPU-reported trigger; the transition
  machine that consumes it is U7 (`update:664`) — which runs **next frame**
  (update precedes render within a frame). So a portal step is: render frame N
  arms → update frame N+1 advances. The one-frame readback lag (E-4) stacks on top.

**Enforced-by-mechanism (good — the structural minority, for contrast):**

- O-6a zone `sync→evolve→mesh` barrier **is** the three pass boundaries
  (`render:1148-1150`).
- The prepare/dispatch split (R8) is a real two-phase: `prepare_mesh` sets
  `dirty[f]`; the dispatch loop branches on `dirty[]`, not the enable bit.
- P5 stale-callback guard is a real closure compare (`gen == world_state_.world_gen`,
  `render:830/868/903`) — a teardown mid-flight drops stale readbacks.
- The readback state-machines (`IDLE→COPIED→MAPPING`) are real enums, not comments.

---

## §3 FRAME-TRUTH — where "the state of the frame" lives vs is smeared

The scattered-truth table: for each axis of frame state, **where the truth
actually lives** and whether it is one place or many.

| axis | where it lives | one place? |
|---|---|---|
| **time / clocks** | `time_state_{beats,seconds,dt,prev_beats,beat_rate}` (CPU) + `gpuSignal` (GPU signal buf) + host `AnalysisSignal` | **3 mirrors.** `beat_rate` tempo-follower computed at `update:620-625`; the **silent-BPM axis** (default-BPM-when-music-off) is the parked inconsistency — a driver-law hole, no owner. |
| **dirty ledger** | `world_state_.ground_entries_dirty`, `.placement_dirty`; local `dirty[PopFamily::COUNT]` (R8, function-scoped); `pawn/floater/camera ReadbackState_` enums; `transitionPhase_`; assorted `mood_state_` flags | **~7 scattered booleans + 4 enums.** No registry; no way to ask "what is dirty this frame." |
| **pass order** | the linear call sequence of `render()` R3-R21 | **implicit.** No declared phase list; order = source order + comments. |
| **draw lists** | `draw_shadow_all` (rp:315) · `render_main_pass` (rp:458) · `render_snapshot_pass` (gallery:1123) | **3 separate enumerations** (§1d). |
| **camera / view** | GPU `camera_state_buffer` (authoritative in camera-host) + `player_.readback_x/z` (CPU mirror, 1-frame stale) + `point_.host` (who owns the point) + `vp_buffer` (matrices from `compute_vp`) | **4 places, host-dependent.** Which one is truth depends on `point_.host` at read time; the harvest re-checks host to avoid a stale overwrite (`render:911`). |
| **the encoder / submit** | host encoder (main) + the hidden derive encoder (E-8) | **2 submits**, one invisible. |

The pattern: **no axis of frame-truth is single-sourced.** Time is triple-mirrored,
dirtiness is a handful of unrelated booleans, pass order is source order, draw
membership is triplicated, and the authoritative viewpoint is host-conditional
across four homes. Every §2 edge is a symptom of an un-single-sourced axis here.

---

## §5 THE MODEL QUESTION (posed, not answered)

**Is the conductor straining toward a declared phase sequence** — named phases,
declared read/write faces, ordering held by structure instead of comments?

**Evidence FOR (it is straining, hard):**
- The frame is *already* narrated as named phases: the `MOVEMENT:` banners are a
  phase list in prose (CLOCK → MOTION → REALIZATION STAGING → TRANSITION → WITNESS,
  and in render: HARVEST → S2 → S3 → S4 → REALIZATION → CAPTURE → passes).
- The orderings are *already* named and numbered: `O-1..O-7`, `RC-1/RC-2`,
  `SEAM[…]`, `P5`. The author has enumerated the exact constraints a phase model
  would encode — and written them as comments because there is no place to put
  them as structure.
- Two phase *shapes* already exist as real two-phase patterns: prepare→dispatch
  (R8, dirty-set drained by a loop) and harvest→(compute)→capture (R1/R10/R11,
  O-2). These are declared read-phase/write-phase pairs — in code, not comment.

**Evidence AGAINST (structure already holds some of it):**
- Pass boundaries enforce the barriers that matter for GPU correctness (O-6a; the
  compute/shadow/main split). The GPU's own submission order carries E-3/E-4.
- The `world_gen` guard, the readback enums, and the `dirty[]`/enable-bit
  separation are genuine mechanism — the parts most likely to corrupt *are* the
  parts already made structural.

**The gap = the distance to Jean's target (one hierarchical graph holding the
frame's truth):**
- What is NOT structural: the phase *sequence* itself (reorder two movements → it
  compiles), the read/write *face* of each phase (nothing declares that
  `dispatch_compute` reads the signal buffer and writes the VP buffer), and the
  *frame-truth axes* of §3 (no single home for time / dirtiness / pass order /
  draw membership / viewpoint).
- A model that closes the gap would: (1) name each phase as a first-class thing
  (not a comment), (2) declare each phase's read-set and write-set over the
  frame-truth axes, (3) let the ordering be *checked* — topological over the
  declared faces, or at minimum asserted — so a reorder that violates O-5b/c or
  O-2 fails at build, not at pixel; and (4) single-source the three draw lists so
  a drawable is one row (the draw-list half of **C5**).
- The frame is a DAG today expressed as a straight line across 2 functions + 4
  files + a hidden submit. The straining is real: the author has already done the
  *analysis* a phase model needs (every O-# is a named edge of that DAG) and has
  nowhere to *land* it but the margins.

**What waits on the conversation this report opens:** the ordering cuts **C7**
(staging→upload made a drained dirty-set, killing E-1's silent drop) and **C8**
(O-1..O-7 as types/asserts/structure over comments), plus the **draw-list half of
C5** (§1d — one family enumeration feeding shadow/main/snapshot). None are taken
here; the model conversation rules on shape first.

---

## §7 DISCIPLINE

Read-only. Nothing moved — no phase extracted, no draw list unified, no ordering
changed, no struct touched. `git status` clean but for this file. Full stop for
the model conversation before any frame-conductor change.
