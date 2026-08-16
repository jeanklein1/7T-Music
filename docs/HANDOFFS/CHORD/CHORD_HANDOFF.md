# CHORD — HANDOFF (single round: S0, C0, C1, C2, C3, C4, C5)

Campaign: CHORD — uniform-seat redistricting by cadence, plus the native
sunset. Approved by Jean 2026-08-16, including the CHORD_5 demotion
reversal. Executor: CC (Opus 5). Author of record: Claude.

Provenance pin: this handoff was derived against `world.wgsl` at
`sha256:7a8f80af735177bd76b5afb8f5be5dd611562ae3d49f6eccf885be6f4144fdf6`
(see BINDING_LEDGER Provenance — trust the ledger's spelling of the hash,
not this line's). If `sha256sum` of the working-tree `world.wgsl` differs
from the ledger's recorded value, FLAG it in the report, re-derive the
verbatim FIND blocks by re-grepping the named symbols, and continue.

## ROUND POLICY — read before executing

1. Execute units in order: S0 → C0 → C1 → C2 → C3 → C4 → C5.
2. One unit = exactly one commit, or zero. Never split a unit across
   commits; never combine two units in one commit.
3. FLAG-AND-CONTINUE. If a witness fails or a FIND count mismatches:
   make ONE self-diagnosis pass (recount, re-census the symbol, check
   for drift from an earlier unit's edit). If resolved, proceed. If not,
   revert the unit's working-tree changes (`git checkout -- .` plus
   `git clean -fd` for files the unit created), record the flag, and
   CONTINUE to the next unit. Every unit below is safe to run after any
   earlier unit aborted — dependencies are noted inline where they are
   soft; there are no hard ones.
4. Do not stop to ask questions. The report at the end carries every
   flag, every mismatch, every judgment call made under a stated rule.
5. Push all commits to `master` once, after the final unit. No branch.
   Rollback story: `git tag native-sunset` (created in S0) plus one
   commit per unit.
6. Encoding law: every file written is UTF-8, LF-only, no BOM, exactly
   one trailing newline. Generated files (`MANIFEST.md`,
   `binding_surface.gen.inc`, the ledger) are never hand-edited —
   regenerate them.
7. CC does not run the Emscripten build. Jean builds and boots after the
   round; that boot is the witness of record for pipeline-layout
   conformance and minBindingSize (the ATLAS_1revB blind spot — naga
   cannot see those two classes).

## STANDING WITNESSES — run inside every unit that touches the surface

- W-naga: validate `world.wgsl` with the naga gate. Recon once in C1 how
  the repo invokes it (script, make target, or a bare `naga` call); use
  the repo's own invocation. Must pass before the unit commits.
- W-gen: `python tools/binding_gen.py --write` then
  `python tools/binding_gen.py --check`. Both must pass.
- W-ledger: regenerate the binding ledger with `tools/binding_ledger.py`
  (recon its invocation once). The `gate` witness must read PASS. Record
  the tightest row it names.
- W-zero: `grep -rn "\b<retired symbol>\b" src/ tools/` returns zero
  hits for every symbol the unit retires (audit/ and docs that are
  records of history are exempt — records are never rewritten).
- W-wallet: read the regenerated MANIFEST wallet rows named in the
  unit's "expected wallet" table. A mismatch is a FLAG, not an abort —
  MANIFEST is the truth; the table below is the prediction.

## SHARED FACTS (from the ledger — cite these, do not re-derive)

Byte sizes (WGSL layout, already witnessed by 0b-4's calculator):
`PortalArray` 1040 · `array<AgentBehaviorParams,10>` 320 ·
`array<AgentTierParams,4>` 192 · `array<ColumnMeshParams,32>` 4096 ·
`array<ArchMeshParams,16>` 1280 · `array<vec4<f32>,400>` 6400 ·
`RibbonState` 112 · `FieldAuthored` 144 · `Lighting` 848 ·
`VPMatrix` 128 · `CameraState` 48 · `array<PawnFigure,14>` 4032 ·
`FloatingEntityArray` 54912.

All merged-struct member offsets below are multiples of 16 and the
totals are multiples of 16 — legal uniform layout by construction. The
C++ mirrors must prove it with `static_assert` on `sizeof` and
`offsetof` (L3: byte-for-byte, both rooms, same commit).

---

# UNIT S0 — NATIVE SUNSET

Objective: the web twin becomes the program. Native is archived at a
tag, not maintained.

1. Tag first, before any edit:
   `git tag -a native-sunset -m "Native twin archived; the web twin is the program (SUNSET_0)."`

2. Census the native build surface. Criteria, in order:
   a. Grep the CMake tree for the native executable target — the one
      that links the native Dawn checkout (grep `f0bf8ab`, and grep
      `third_party` for a native dawn path distinct from
      `third_party/emdawnwebgpu`).
   b. List that target's exclusive sources: files referenced by the
      native target and by nothing else (windowing/init glue).
   c. Grep `allow_unsafe_apis` (console.hpp, F5-d) — the toggle exists
      only to open the immediate lane on the old native generation.

3. Delete: the native target block, its exclusive sources, the
   `allow_unsafe_apis` toggle and its F5-d comment, and any CMake
   option that exists only to select the native backend. KEEP anything
   referenced by the web build. If a file is referenced by both, keep
   it and FLAG it. Do not delete the native Dawn checkout directory if
   it is a git submodule or external path — remove the *reference*
   (CMake/config), flag the directory for Jean to remove by hand.

4. Re-true the living law. In `world.wgsl` (each FIND expects exactly 1):

   FIND:
   ```
   // This module is single-source for both twins. Supported
   // compilers: Tint→DXC (SM6.0+) on native and Windows Chrome,
   ```
   REPLACE:
   ```
   // The web twin is the program (SUNSET_0; native archived at tag
   // native-sunset). Supported compilers: Tint→DXC (SM6.0+) on
   // Windows Chrome,
   ```

   FIND: `// the per-commit gate (CC); glaw1 + boot is the witness of`
   REPLACE: `// the per-commit gate (CC); the web build + boot is the witness of`

   FIND: `so glaw1/boot`
   REPLACE: `so the web boot`

   FIND:
   ```
   //   Budget = WebGPU core defaults: storage 8 / uniforms 12 per stage; the room
   //   family sits at 6/8 storage — the two occupier windows ride uniform
   //   (TETRIS WALLET_0; demotion record: BINDING_LEDGER Table C).
   ```
   REPLACE:
   ```
   //   Budget = WebGPU core defaults: storage 8 / uniforms 12 per stage;
   //   per-row occupancy is MANIFEST.md's lane table — the banner names
   //   the witness, not its value (TETRIS WALLET_0; demotions: Table C).
   ```

   Then grep `glaw1` across `world.wgsl`: two further mentions exist in
   body comments (near lines 1102 and 1758). Re-point each to
   "the web boot" where it names a living gate; leave any mention that
   is quoting history. LAWS.md and other living docs: same rule.
   Records under `audit/` keep `glaw1` — history is not rewritten.

5. Append to `src/docs/LAWS.md`:
   ```
   ## SUNSET_0 (2026-08-16) — the web twin is the program
   Native is archived at tag `native-sunset`. Resurrection is
   archaeology from the tag, not maintenance. The witness chain is:
   naga gates the WGSL module per commit (CC-runnable); the web build
   + boot witnesses pipeline-layout conformance and minBindingSize
   (the classes naga cannot see — ATLAS_1revB). The audience floor
   (WebGPU core defaults) and the compiler floor (PIVOT_0) are
   unchanged by this sunset.
   ```

6. Check `third_party/emdawnwebgpu/PINNED.md` for language comparing
   the two generations; if it prescribes native-side behavior, trim the
   prescription to a historical note. If unsure, FLAG and leave.

Witnesses: W-naga (banner edits touched the file). Commit:
`SUNSET_0: the web twin is the program; native archived at tag native-sunset`.

---

# UNIT C0 — CHARTER

Create `src/docs/CHORD.md` with exactly this content (then commit —
instrument and subject never share a commit, and this is the record the
subjects will cite):

```
# CHORD — uniform-seat redistricting by cadence

LOOM stratified the GROUPS by cadence (world / frame / family state /
family textures). CHORD stratifies the BYTES: seats of one cadence and
one author merge into one block, one binding, one upload per beat of
their clock. Scarcity was a symptom of granularity — the program ran
out of seats because it bound objects, not categories.

## The taxonomy
Every datum classifies by three questions: WHEN it changes (cadence),
HOW the GPU touches it (access), WHO authors it (CPU intent / GPU
truth). The cell answers the layout question mechanically. This
document is the taxonomy's record until ORGAN gives it an instrument.

## The blocks (CHORD_1..4)
- agent_room  (g2:1, uniform, C)  = portals + behaviors + tier_gains
  + occupier_cmg + occupier_amg. 6928 B. Cadence: world/mood.
- field_bus   (g2:9, uniform, C)  = head_poses + ribbon + authored.
  6656 B. Cadence: frame (fastest member governs).
- frame_r     (g1:1, uniform, VF) = lighting + vp + camera. 1024 B.
  Two instances: main and photographer. vp/camera arrive by
  copyBufferToBuffer from the GPU-sovereign state each frame — the
  CPU never reads them (readback law).
- scene_constants (g2:200, uniform, V) = tier_gains + figure_profiles
  + ribbon. 4336 B. Cadence: world/mood. Bound by scene and shadow.

## Rulings of record
- WINDOWS, NOT HOMES: a fact's home is its one CPU-side struct and its
  one authoring site. GPU blocks are transport windows; tier_gains and
  ribbon-state appearing in two blocks is two windows on one home, and
  the authoring site writes every window it owns.
- CHORD_5 REVERSAL (Jean, 2026-08-16): render_floating returns from
  uniform to read-only storage. The DOMESDAY demotion bought storage
  seats when storage was the famine; post-LOOM the famine moved to
  uniforms, and the 54,912 B block sat at 83.8% of the uniform binding
  ceiling — a wall on entity growth. Storage rows can afford the seat.
  The demotion record in Table C stays; this entry is the reversal's.
- SINGLE PATH, RESTATED: the Pixel offers f16, subgroups, dual-source
  blending, ASTC. Declined — one program, one path, core defaults.
  The cost is named (f16 doubles Valhall ALU) so the choice stays a
  choice.
- DYNAMIC OFFSETS STAY AT 0/8 AND 0/4: offsets solve many-instances-
  of-one-shape; the disease here was many-shapes-of-one-cadence, and a
  struct cures it with no per-draw plumbing.

## Target wallet (prediction; MANIFEST is the truth)
uniform worst 5/12 (agents C, tied by patchgen C) — was 11/12.
storage worst 5/8. All other lanes unchanged.
```

Commit: `CHORD_0: charter — the taxonomy, the blocks, the rulings of record`.

---

# UNIT C1 — agent_room (g2:1)

Objective: five agents-room uniform seats become one. Agents-room
uniform row: 11 → 7.

## C1.a — WGSL

1. REPLACE (expect 1) the portal declaration:
   FIND: `@group(2) @binding(1)  var<uniform> portal_array: PortalArray;`
   REPLACE:
   ```
   // THE AGENTS' ROOM CONSTANTS (CHORD_1) — one cadence, one block.
   // Everything here is CPU-authored at world/mood cadence. Mirrors
   // GPUAgentRoomConstants in state.hpp BYTE-FOR-BYTE (6928 B; the
   // static_asserts are the handshake). Offsets: portals 0,
   // behaviors 1040, tier_gains 1360, occupier_cmg 1552,
   // occupier_amg 5648.
   struct AgentRoomConstants {
       portals: PortalArray,
       behaviors: array<AgentBehaviorParams, 10>,
       tier_gains: array<AgentTierParams, 4>,
       occupier_cmg: array<ColumnMeshParams, 32>,
       occupier_amg: array<ArchMeshParams, 16>,
   }
   @group(2) @binding(1) var<uniform> agent_room: AgentRoomConstants;
   ```

2. DELETE (expect 1) the line:
   `@group(2) @binding(3) var<uniform> agent_behaviors: array<AgentBehaviorParams, 10>;`
   Prune any comment line directly attached to it that names only the
   deleted seat (rule: a surviving comment must name a surviving
   symbol; delete rather than annotate).

3. REPLACE (expect 1) the occupier block:
   FIND:
   ```
   // SAME mesh-param buffers the mesh-gen kernels read — one authored
   // geometry, one home; the rows and the mesh can never disagree.
   @group(2) @binding(7) var<uniform> occupier_cmg: array<ColumnMeshParams, 32>;
   @group(2) @binding(8) var<uniform> occupier_amg: array<ArchMeshParams, 16>;
   ```
   REPLACE:
   ```
   // The occupier windows ride agent_room.occupier_cmg / .occupier_amg
   // (CHORD_1) — same mesh-param rows the mesh-gen kernels read: one
   // authored geometry, one home; the rows and the mesh can never disagree.
   ```

4. The tier-gains declaration at `@group(2) @binding(4)` SURVIVES this
   unit (the render room still reads it until C4). Its seat leaves only
   the agents layout.

5. Token renames, word-boundary (`\b`) anchored, whole file:
   - `portal_array` → `agent_room.portals` — expect 2.
   - `agent_behaviors` → `agent_room.behaviors` — expect 11.
   - `occupier_cmg` → `agent_room.occupier_cmg` — expect 2.
   - `occupier_amg` → `agent_room.occupier_amg` — expect 2.
   (Counts exclude the declaration lines already replaced/deleted.)

6. GUARDED rename for tier gains — the one shared symbol. Replace
   `\bagent_tier_gains\b` → `agent_room.tier_gains` on every line
   EXCEPT lines containing any of: `@binding(4)` (the surviving decl),
   `render entity group` (the decl's seat comment), `let tg =` (the
   single pawn_vs render site). Expect exactly 17 replacements.
   Python recipe:
   ```python
   import re, pathlib
   p = pathlib.Path("src/cartridges/the_board/realization/world.wgsl")
   out, n = [], 0
   for line in p.read_text(encoding="utf-8").splitlines(keepends=True):
       if ("@binding(4)" in line or "render entity group" in line
               or "let tg =" in line):
           out.append(line); continue
       new, k = re.subn(r"\bagent_tier_gains\b",
                        "agent_room.tier_gains", line)
       n += k; out.append(new)
   assert n == 17, n
   p.write_text("".join(out), encoding="utf-8", newline="")
   ```

## C1.b — schema, registry, generated surface

Recon `tools/binding_schema.py` once: identify how DECLS, SEATS,
LAYOUTS express a row (the MANIFEST header names the relations). Then:
- DECLS: remove `portal_array`, `agent_behaviors`, `occupier_cmg`,
  `occupier_amg`. Add `agent_room` — uniform, store type
  `AgentRoomConstants`, group 2, binding 1, visibility C. Keep
  `agent_tier_gains` (seated only in the scene layout after this unit).
- SEATS: `agentsStateLayout_` drops entries at bindings 1, 3, 4, 7, 8
  and gains (binding 1, `bind::g2::agent_room`, Uniform, C). The layout
  goes 13 → 9 entries.
- `binding_registry.hpp` (recon whether generated or hand-kept; L6
  names it the source): retire `g2::portal_array`, `g2::agent_behaviors`,
  `g2::occupier_cmg`, `g2::occupier_amg`; add `g2::agent_room = 1`.
  `g2::agent_tier_gains` survives until C4.
- Run W-gen. The regenerated surface re-emits the layout.

## C1.c — C++ mirror, buffer, uploads

- In `state.hpp`, beside the existing GPU mirror structs, add the
  mirror (match the repo's naming convention for GPU-mirror types; the
  member types are the existing mirrors of PortalArray,
  AgentBehaviorParams, AgentTierParams, ColumnMeshParams,
  ArchMeshParams):
  ```cpp
  struct GPUAgentRoomConstants {
      /*portals*/      <PortalArray mirror>              portals;
      /*behaviors*/    std::array<<BehaviorParams>,10>   behaviors;
      /*tier_gains*/   std::array<<TierParams>,4>        tier_gains;
      /*occupier_cmg*/ std::array<<ColumnMeshParams>,32> occupier_cmg;
      /*occupier_amg*/ std::array<<ArchMeshParams>,16>   occupier_amg;
  };
  static_assert(sizeof(GPUAgentRoomConstants) == 6928);
  static_assert(offsetof(GPUAgentRoomConstants, behaviors)    == 1040);
  static_assert(offsetof(GPUAgentRoomConstants, tier_gains)   == 1360);
  static_assert(offsetof(GPUAgentRoomConstants, occupier_cmg) == 1552);
  static_assert(offsetof(GPUAgentRoomConstants, occupier_amg) == 5648);
  ```
- One buffer replaces five: create `agentRoomBuffer_` (6928 B, usage
  Uniform | CopyDst). Census the five retired buffers by grepping the
  retired registry constants and their buffer member names; delete the
  four whose only binding was the agents layout. THE TIER-GAINS BUFFER
  SURVIVES (still bound by the scene layout until C4).
- Persistent CPU home: keep one `GPUAgentRoomConstants` member in the
  state; every site that today writes portals / behaviors / tier gains
  / occupiers updates the member in place and then issues ONE
  `queue.WriteBuffer(agentRoomBuffer_, 0, &member, sizeof(member))`.
  Tier-gain authoring writes BOTH windows while both exist: the block
  member and the surviving standalone buffer (windows, not homes — see
  the charter).
- Bind-group creation for the agents layout: entries follow the
  regenerated surface; the entry at binding 1 binds `agentRoomBuffer_`.

Witnesses: W-naga, W-gen, W-ledger, W-zero on the four retired symbols,
W-wallet. Expected wallet: `updatePlayerAgentPipeline_ C` uniform
**7 / 12** (likewise the other three agent-family compute rows);
storage unchanged 5/8.

Commit: `CHORD_1: agent_room — five agents-room uniform seats become one (11→7)`.

---

# UNIT C2 — field_bus (g2:9)

Objective: the three field uniform windows become one. Agents-room
uniform row: 7 → 5.

## C2.a — WGSL

1. DELETE (expect 1 each):
   `@group(2) @binding(9) var<uniform> field_head_poses : array<vec4<f32>, 400>;`
   `@group(2) @binding(11) var<uniform> field_ribbon : RibbonState;`
   The field comment block above them ("The field (FIELD_2)…") names
   the windows generically — it survives.

2. REPLACE (expect 1) the authored declaration line:
   FIND: `@group(2) @binding(12) var<uniform> field_authored : FieldAuthored;`
   REPLACE:
   ```
   // THE FIELD BUS (CHORD_2) — the field's three windows, one block,
   // one write per frame. Mirrors GPUFieldBus in state.hpp
   // BYTE-FOR-BYTE (6656 B). Offsets: head_poses 0, ribbon 6400,
   // authored 6512.
   struct FieldBus {
       head_poses: array<vec4<f32>, 400>,
       ribbon: RibbonState,
       authored: FieldAuthored,
   }
   @group(2) @binding(9) var<uniform> field_bus: FieldBus;
   ```
   (The `FieldAuthored` struct definition directly above survives — it
   is the member's type. Its "mirrors GPUFieldAuthored" comment
   survives with it.)

3. Token renames, `\b`-anchored, whole file:
   - `field_head_poses` → `field_bus.head_poses` — expect 1.
   - `field_ribbon` → `field_bus.ribbon` — expect 4.
   - `field_authored` → `field_bus.authored` — expect 3.
   (`config.field_authored_gain` is a distinct token; `\b` protects it.
   Verify it still greps 4+ after the rename.)

## C2.b — schema / registry / C++

- DECLS: remove the three; add `field_bus` (uniform, `FieldBus`, g2:9,
  C). SEATS: `agentsStateLayout_` drops bindings 9, 11, 12; gains
  (9, `bind::g2::field_bus`, Uniform, C). Layout 9 → 7 entries.
  `field_forces` (g2:10, storage) is untouched.
- Registry: retire the three constants; add `g2::field_bus = 9`.
- C++ mirror:
  ```cpp
  struct GPUFieldBus {
      std::array<std::array<float,4>,400> head_poses;  // match repo's vec4 mirror
      <RibbonState mirror>  ribbon;
      <FieldAuthored mirror> authored;
  };
  static_assert(sizeof(GPUFieldBus) == 6656);
  static_assert(offsetof(GPUFieldBus, ribbon)   == 6400);
  static_assert(offsetof(GPUFieldBus, authored) == 6512);
  ```
- One buffer `fieldBusBuffer_` (6656 B, Uniform | CopyDst) replaces
  three. Census today's three upload sites; the head-poses and
  ribbon-state authors already run per frame — fold all three into one
  persistent CPU `GPUFieldBus` member written in place, one
  `WriteBuffer` per frame at the site that today writes head poses.
  NOTE: the ribbon pipeline's OWN seats (`ribbon_state` g2:140,
  `head_poses` g2:142) are other windows on the same homes — their
  authoring sites keep writing them; only the agents-room windows
  merged.

Witnesses: standard set. W-zero on the three retired symbols. Expected
wallet: agents-family compute rows uniform **5 / 12**.

Commit: `CHORD_2: field_bus — the field's three windows become one (7→5)`.

---

# UNIT C3 — frame_r (g1:1), two instances

Objective: three per-frame render uniforms become one block; the
GPU-sovereign vp/camera reach it by copy, never by CPU readback.
Render V uniform 8 → 6; render F 6 → 4; shadow V 7 → 5.

## C3.a — WGSL

1. REPLACE (expect 1):
   FIND: `@group(1) @binding(1) var<uniform> render_lighting: Lighting;`
   REPLACE:
   ```
   // FRAME R (CHORD_3) — the render frame's block: lighting (CPU-
   // authored) + vp + camera (GPU-sovereign, arriving by encoder copy
   // from vp_data / camera_state each frame — the CPU never reads
   // them). Two instances back two bind groups over this layout: main
   // and photographer. Mirrors GPUFrameR in state.hpp BYTE-FOR-BYTE
   // (1024 B). Offsets: lighting 0, vp 848, camera 976.
   struct FrameR {
       lighting: Lighting,
       vp: VPMatrix,
       camera: CameraState,
   }
   @group(1) @binding(1) var<uniform> frame_r: FrameR;
   ```
2. DELETE (expect 1 each):
   `@group(1) @binding(3) var<uniform> render_vp: VPMatrix;`
   `@group(1) @binding(4) var<uniform> render_camera: CameraState;`
   (Prune attached single-seat comments by the surviving-symbol rule.)
3. Token renames, `\b`-anchored, whole file:
   - `render_lighting` → `frame_r.lighting` — expect 13.
   - `render_vp` → `frame_r.vp` — expect 18.
   - `render_camera` → `frame_r.camera` — expect 11.

## C3.b — schema / registry

- DECLS: remove the three; add `frame_r` (uniform, `FrameR`, g1:1,
  visibility VF). SEATS: `frameRLayout_` drops bindings 3 and 4; the
  binding-1 entry becomes `bind::g1::frame_r` (Uniform, VF). Layout
  5 → 3 entries (frame_r + the two samplers).
- Registry: retire `g1::render_vp`, `g1::render_camera`; rename/re-point
  `g1::render_lighting` → `g1::frame_r = 1`.

## C3.c — C++: two instances and the copies

- Mirror:
  ```cpp
  struct GPUFrameR {
      <Lighting mirror>    lighting;
      <VPMatrix mirror>    vp;
      <CameraState mirror> camera;
  };
  static_assert(sizeof(GPUFrameR) == 1024);
  static_assert(offsetof(GPUFrameR, vp)     == 848);
  static_assert(offsetof(GPUFrameR, camera) == 976);
  ```
- Buffers: `frameRMainBuffer_` and `frameRPhotoBuffer_` (1024 B each,
  Uniform | CopyDst).
- Census how `render_vp` / `render_camera` are fed today. Two known
  worlds:
  (a) the g1 seats bind the SAME buffers as `vp_data` / `camera_state`
      (dual Uniform|Storage usage) — then no copies exist today;
  (b) separate buffers filled by `CopyBufferToBuffer`.
  Either way the NEW law is (b): per frame, after the compute pass that
  runs `compute_vp` / `update_camera` closes (census anchor: the
  encoder section metered as `dispatch_compute`), encode
  `CopyBufferToBuffer(vpDataBuf, 0, frameRMainBuffer_, 848, 128)` and
  `CopyBufferToBuffer(cameraStateBuf, 0, frameRMainBuffer_, 976, 48)`.
  Add CopySrc usage to `vp_data` / `camera_state` buffers if absent.
- Photographer: the second bind group over `frameRLayout_` binds
  `frameRPhotoBuffer_`. At the site that prepares the snapshot pass
  (census anchors: meters `snapshot_pass` / `witness_capture`, after
  the `compute_photographer_vp` dispatch), encode
  `CopyBufferToBuffer(photographerVpBuf, 0, frameRPhotoBuffer_, 848, 128)`
  and `CopyBufferToBuffer(photographerCameraOutBuf, 0, frameRPhotoBuffer_, 976, 48)`.
- Lighting authoring: every site that today writes the lighting buffer
  writes `WriteBuffer(frameRMainBuffer_, 0, &lighting, 848)` AND the
  same into `frameRPhotoBuffer_` (two windows, one home).
- Usage cleanup: if world (a) held, remove Uniform usage from
  `vp_data` / `camera_state` / `photographer_vp` /
  `photographer_camera_out` buffers where no uniform binding remains.
- Retire the standalone lighting/vp/camera uniform buffers if world (b)
  held (census by registry constants; W-zero proves it).

Witnesses: standard set. Expected wallet: `patchTerrainPipeline_ V`
uniform **6 / 12**, F **4 / 12**; shadow rows V uniform **5 / 12**;
gallery rows drop by 2 likewise. Storage rows unchanged.

Commit: `CHORD_3: frame_r — the render frame's three uniforms become one block, GPU truth arrives by copy`.

---

# UNIT C4 — scene_constants (g2:200)

Objective: the render room's three mood-cadence uniforms become one;
the tier-gains window split completes. Render V uniform 6 → 4;
shadow V 5 → 4.

## C4.a — WGSL

1. REPLACE (expect 1):
   FIND: `@group(2) @binding(200) var<uniform> agent_figure_profiles: array<PawnFigure, 14>;`
   REPLACE:
   ```
   // SCENE CONSTANTS (CHORD_4) — the render room's mood-cadence block:
   // the tier-gains window, the figure profiles, the ribbon window.
   // Bound by the scene AND shadow layouts, VERTEX only. Mirrors
   // GPUSceneConstants in state.hpp BYTE-FOR-BYTE (4336 B). Offsets:
   // tier_gains 0, figure_profiles 192, ribbon 4224.
   struct SceneConstants {
       tier_gains: array<AgentTierParams, 4>,
       figure_profiles: array<PawnFigure, 14>,
       ribbon: RibbonState,
   }
   @group(2) @binding(200) var<uniform> scene_constants: SceneConstants;
   ```
2. DELETE (expect 1 each):
   `@group(2) @binding(4) var<uniform> agent_tier_gains: array<AgentTierParams, 4>;`
   `@group(2) @binding(201) var<uniform> render_ribbon: RibbonState;`
   Prune the tier-gains seat comment ("render entity group…", the
   paragraph attached to the deleted decl) by the surviving-symbol rule.
3. Token renames, `\b`-anchored:
   - remaining `agent_tier_gains` → `scene_constants.tier_gains` —
     expect exactly 1 (the pawn_vs site `let tg = …[tier];`). If more
     than 1 remains, C1's guarded rename drifted: FLAG, fix by hand to
     the room each site belongs to, continue.
   - `agent_figure_profiles` → `scene_constants.figure_profiles` —
     expect 3.
   - `render_ribbon` → `scene_constants.ribbon` — expect 2.

## C4.b — schema / registry / C++

- DECLS: remove `agent_tier_gains`, `agent_figure_profiles`,
  `render_ribbon`; add `scene_constants` (uniform, `SceneConstants`,
  g2:200, V). SEATS: `sceneStateLayout_` drops bindings 4, 200, 201 and
  gains (200, `bind::g2::scene_constants`, Uniform, V) — 8 → 6 entries.
  `shadowStateLayout_` drops bindings 200, 201 and gains the same seat
  — 7 → 6 entries.
- Registry: retire `g2::agent_tier_gains`, `g2::agent_figure_profiles`,
  `g2::render_ribbon`; add `g2::scene_constants = 200`.
- Mirror:
  ```cpp
  struct GPUSceneConstants {
      std::array<<TierParams>,4>  tier_gains;
      std::array<<PawnFigure>,14> figure_profiles;
      <RibbonState mirror>        ribbon;
  };
  static_assert(sizeof(GPUSceneConstants) == 4336);
  static_assert(offsetof(GPUSceneConstants, figure_profiles) == 192);
  static_assert(offsetof(GPUSceneConstants, ribbon)          == 4224);
  ```
- One buffer `sceneConstantsBuffer_` (4336 B, Uniform | CopyDst)
  replaces three. Persistent CPU member; every authoring site for tier
  gains / figures / render-side ribbon state updates the member and
  issues one WriteBuffer. Tier-gain authoring now writes agent_room's
  window and this one (two windows, one home) — the C1 interim write to
  the standalone buffer is retired with the buffer. Ribbon-state
  authoring keeps writing the ribbon pipeline's own g2:140 window and
  field_bus.ribbon as before, plus this window.
- Scene and shadow bind groups: the entry at 200 binds
  `sceneConstantsBuffer_`.

Witnesses: standard set. W-zero on the three retired symbols. Expected
wallet: render entity rows V uniform **4 / 12**; shadow rows V uniform
**4 / 12**.

Commit: `CHORD_4: scene_constants — the render room's mood-cadence seats become one (V 8→4 across CHORD)`.

---

# UNIT C5 — the floating promotion (g2:6, uniform → storage read)

Objective: remove the 65,536 B wall on entity growth. Approved
reversal of the DOMESDAY demotion (charter carries the record).

## C5.a — WGSL

REPLACE (expect 1):
FIND: `@group(2) @binding(6) var<uniform> render_floating: FloatingEntityArray;`
REPLACE:
```
// CHORD_5: promoted back to read-only storage — the uniform ceiling
// (54,912 of 65,536 B) was a wall on entity growth; post-LOOM the
// storage rows afford the seat. Demotion: Table C. Reversal: CHORD.md.
@group(2) @binding(6) var<storage, read> render_floating: FloatingEntityArray;
```
No access-site edits — the five reads keep their spelling. Do NOT
repack `FloatingEntityState` (behavior preservation; L3 mirror law).

## C5.b — schema / registry / C++

- DECLS: `render_floating` channel uniform → storage (read). SEATS:
  `sceneStateLayout_` entry at binding 6 and `shadowStateLayout_` entry
  at binding 6: buffer type Uniform → ReadOnlyStorage (visibility
  unchanged: VF in scene, V in shadow).
- Census which buffer backs g2:6:
  (a) if it is the SAME buffer as `floating_entities` (g2:2, already
      Storage): after promotion, remove Uniform from its usage flags if
      no uniform binding remains on it;
  (b) if it is a separate render copy: flip that buffer's Uniform usage
      to Storage; keep whatever copy feeds it.
- If Table C's demotion records live as schema-side annotations, mark
  this row reversed per the existing pattern; if Table C is
  ledger-generated prose only, the regenerated ledger plus CHORD.md is
  the record — do not hand-edit the ledger. FLAG which world held.

Witnesses: standard set (W-naga catches the address-space change; the
web boot remains the layout-conformance witness). Expected wallet:
render entity rows V uniform **3 / 12**, V storage **4 / 8**; F uniform
**3 / 12**, F storage **3 / 8**; shadow rows V uniform **3 / 12**,
V storage **5 / 8**. Program-wide after the round: uniform worst
**5 / 12** (agents C, tied by patchgen C), storage worst **5 / 8**.

Commit: `CHORD_5: render_floating promoted to read-only storage — the entity-growth wall falls`.

---

# FINAL REPORT (always lands, even if every unit aborted)

1. Wallet table: the six wallet-summary rows from the regenerated
   MANIFEST, before-column from this handoff's predictions,
   after-column from the file. Name the new tightest rows.
2. Witness log: per unit, each standing witness and its result.
3. Flags: every mismatch, every judgment call and the rule it was made
   under, every world-(a)/(b) census outcome (C3 feeding, C5 backing
   buffer, Table C's home), everything left for Jean's hand.
4. Commits: hash + message list, confirmation of the `native-sunset`
   tag, confirmation of the single push.
5. Jean's gate after the round: web build + boot on desktop Chrome and
   the Pixel — the console and the MANIFEST wallet are the receipts;
   the SOAK session remains the standing product gate.
