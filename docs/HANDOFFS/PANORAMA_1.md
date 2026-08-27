# PANORAMA_1 — round two: the free events, the dials, the pace

> **STATE (CC, at the round's close).** OPEN: six units landed, one half held,
> one unit STOPped as the order provides. Per-item status is in `docs/OPEN.md`
> under `## PANORAMA_1` — open state's one home (L32) — and is not duplicated
> into the body below, which stands as written.
>
> **Landed:** U1 the ceiling gate · U2 the settle · U3 the subtraction dials ·
> U4 the PCF tap dial · U5 the photograph at LOD1 · U6a the forced metronome
> (`?pace=1|2`) · U8 this filing. Plus one instrument the round earned: the TU
> gate now compiles `src/the_board.cpp`, closing a blind spot OVERTURE_0
> opened and PANORAMA_0 carried.
>
> **Held:** U6b, the governor — a GPU completion callback in the frame loop,
> unwitnessable without a build.
>
> **STOPped, per the unit's own condition:** U7. The preset cannot be stamped
> without a CMake change, which this round's Touches exclude. The ruling
> stands as a ruling; two ways forward are filed.
>
> **Recon answered R0-R5 against the tree.** Two answers departed from the
> order and are named in `docs/OPEN.md` and in their commits: `world_young`
> replaces `door_fallback_pending` as U2's birth signal, and the settle stamps
> at first sight rather than at the raise.
>
> **The round's deliverable is a table nobody has taken yet** — eight
> main-pass masks and two shadow masks, one window each, on both devices.
> Round three is designed from it.

---

**Status:** OPEN work order (`docs/HANDOFFS/`, dies at close — L31). **Executor:** CC on master, one commit per unit; instrument and subject never share a commit. **Gates:** Jean — build (glaw1), the two taste stamps (U4, U5), the pace by eye (U6), deploy. **Mode:** FLAG-AND-FINISH. R0 failing STOPs U1 only; every other mismatch STOPs its unit and the round continues. **Base:** master at PANORAMA_0's close (`a1d22229` or later). Boot preflight first. **Touches:** `cartridge.hpp` (the re-raise; the draw sites), `bodies/grounded.hpp` (the preparers), `realization/render_passes.hpp`, `realization/renderer.hpp` (the snapshot's index buffer), `bodies/gallery.hpp` (`render_snapshot_pass`), `world.wgsl` (U4 only — one uniform branch), the config mirror (U3/U4 fields), `src/the_board.cpp` (U6), `src/console/organ_params.inc` (the dials), `tools/web_dist.py` (U7), `docs/OPEN.md`. **WGSL gate:** owed by U4 (the shader moves) and by nothing else.

## 0. Why this order

PANORAMA_0's round removed the compute chain's serial lanes and the queue bound, and its one instrument — the mesh-gen firing count — found the next event: `col 378` of `498` firings in 1,300 frames, on a re-raise that is inert outdoors. The meter cannot split a render pass, so the main pass (11–12 ms on Kepler, unexplained by its vertex and pixel counts) and the shadow pass get subtraction dials before any design is spent on them. And a 1.3 ms difference between two worlds flipped the laptop's pacing from 48 fps to a 12/24 ms alternation at 40: on a device between one and two vblanks, pace is the cure and load-shaving is chasing a threshold.

This round lands what needs no design and no measurement that does not yet exist; it produces the table round three is designed from. The floor device decides round three: a Pixel console after this round, walking and riding.

## 1. Recon (report only)

- **R0 — the re-raise.** In `cartridge.hpp` `phase_placement_correction`, read the block headed THE RE-RAISE (COLUMN CEILING FIT). Name the CPU fact that is the mirror of `cmg_config.ceiling_height` (the value `column_mesh_gen` selects on: `select(p.height, max(ceiling − ground_y, MIN), cmg_config.ceiling_height > 0.0 && tier < ANTENNA)`). Confirm that outdoors that value is 0, so the rebake's every output byte equals the last bake's. If the CPU home cannot be named, STOP U1 — a gate on an inference is not a gate.
- **R1 — the preparers.** `prepare_{arch,column,palm,cactus,blade}_mesh_gen` (`bodies/grounded.hpp`) each consume `es.*_mesh_gen_pending` and set the family's index count. List every raiser of each `pending` (spawn commit, evict, the portal channel `force_spawn_portal_arch`, the re-raise, teardown). Confirm the entity_mesh_gen pass runs the five preparers through `FAMILY_DISPATCH` mesh hooks in one pass, and name the frame counter the settle can stamp with (`gallery_state_.frame_counter` exists; a world-level one may too).
- **R2 — the loop and the clock.** `src/the_board.cpp`: the `emscripten_set_main_loop*` call and its timing mode; the steady clock (RIBBON_6/7 — the "served" dt and its in-band rule); where a frame's `dt` is served to `signal.dt`. Confirm `emscripten_set_main_loop_timing(EM_TIMING_RAF, n)` is callable mid-run on this Emscripten (it is in upstream; confirm the pinned version). Confirm `wgpuQueueOnSubmittedWorkDone` exists on the pinned emdawnwebgpu surface (`console_gate` covers it).
- **R3 — the draw sites.** `render_passes.hpp` `render_main_pass`: the three terrain plan slots (A/B/C), `draw_table(…, DRAW_MAIN)`, the ribbon, the two painting draws, `render_orbs`, the fade. `render_shadow_pass`: the terrain draw and the entity table. Name each site the mask bits gate. Read `organ_params.inc` for the precedent of a U32 or enum dial in PREVIEW mode (`fpv_mode` is a u32 in config — its enrollment is the shape).
- **R4 — the photograph.** `bodies/gallery.hpp` `render_snapshot_pass`: the terrain draw is `draw_patch_terrain_direct(... patch_index_buffer_lod0_live(), patch_index_count_lod0_live(), render_patch_count)`; name the LOD1 pair (`patch_index_buffer_lod1()` and its count — the shadow pass already draws both bands through it).
- **R5 — the PCF site.** `sample_shadow_pcf` in `world.wgsl`: the sixteen taps at (±0.5, ±1.5)²; the inner four are the (±0.5, ±0.5) taps. `TEXEL_UV`, `PCF_RADIUS_TEXELS` (drives the normal offset — it stays 2.5 for both arms in this round; the taps change, the offset does not). Confirm `DesignConfig`'s mirror rule (L3) for adding one u32 field and its size assert on both rooms.

## 2. Units, in landing order

### U1 — the ceiling gate (mesh half a, free)
At the re-raise: `column_mesh_gen_pending = true` only when the ceiling fit is live — the CPU home R0 names is `> 0`. Rewrite the RE-RAISE comment to say so: "a corrected ground demands one rebake where a ceiling reads it; outdoors the kernel's select keeps `p.height` and a rebake would reproduce every byte". **Witness:** `[METER] mesh-gen firings … col N` in an open-world window: N falls from ~400 to the spawn-driven tens. Indoor: unchanged.

### U2 — the settle (mesh half b, free)

```cpp
// THE SETTLE (PANORAMA_1). A family regenerates at most once per
// MESH_GEN_SETTLE_FRAMES outside a world's birth. A crossing raises
// `pending` on several consecutive frames; without this each raise was
// a whole-family rebake (4–8 ms on the floor device). What arrives late
// arrives at the ring's edge, materializing through the icing, where a
// 130 ms delay is invisible; what leaves late leaves beyond the ring,
// where it is already veiled. The birth burst bypasses it — a young
// world and the birth frame after a portal regenerate at once, behind
// the veil or the fade.
inline constexpr uint32_t MESH_GEN_SETTLE_FRAMES = 8;
```

Per family: `pending_since` (frame stamp, written when `pending` goes false→true). The preparer consumes when `frame − pending_since ≥ MESH_GEN_SETTLE_FRAMES`, or `world_young`, or the birth frame (`door_fallback_pending` was consumed this frame — R1 names the cleanest birth signal). The portal channel's raise is a birth raise. **Witness:** total firings per window on a ride; `[STREAM] SPAWN` bursts no longer produce firing bursts. A visual check by Jean at the ring's edge while riding: nothing pops in view.

### U3 — the subtraction dials (instrument)
Two u32 bitmasks in the config, PREVIEW-mode dials, default all bits set:
`draw_mask`: bit0 terrain plan A (clean LOD0), bit1 plan B (zoned LOD0), bit2 plan C (LOD1), bit3 the entity table, bit4 the ribbon, bit5 the paintings (both draws), bit6 the orbs, bit7 the fade. `shadow_mask`: bit0 terrain, bit1 the entity table (including the ribbon and paintings).

Each draw site tests its bit. A masked draw is skipped at the encoder, not culled in the shader, so the meter's pass row measures its absence. This is the table: Jean records `main_pass` and `shadow_pass` means under each single-bit-off mask, walking, one window each, laptop and Pixel. Round three is designed from it. The dials stay in the tree (they are the instrument the meter lacks) under the instruments gate if the panel wants them hidden.

### U4 — the PCF dial (instrument + taste)
`shadow_pcf_taps` (u32, 16 or 4) in the config, one uniform branch in `sample_shadow_pcf`: the 4-tap arm sums the four (±0.5, ±0.5) taps and divides by four — the sampler's comparison-bilinear makes each a 2×2, so the support is 3×3 texels against today's 5×5. Nothing else moves (offset, fade, bounds). **Witness:** `main_pass` 16 vs 4 on both devices — the tap cost, which PANORAMA_0 estimated at 1–2 ms and could not read. **Taste gate:** a shadow edge under the sun at 17°, both settings, Jean's eye. If 4 passes, it becomes the default in a later dial round; this round only measures.

### U5 — the photograph at LOD1 (D-1, taste)
`render_snapshot_pass`: the terrain draw takes the LOD1 pair from R4. Everything else in the pass stays. **Witness:** `snapshot_pass` max — 16–55 ms today on both devices, expected under 8. **Taste gate:** two photographs, same pose, LOD0 vs LOD1, at the size they hang at. D-2 (the photographer's own cull window and the two-frame composite) is a later unit and waits behind this stamp.

### U6 — PACE_0, the metronome
Two halves:

**(a) The switch.** `pace_` ∈ {1, 2}: rAF callbacks per frame, applied with `emscripten_set_main_loop_timing(EM_TIMING_RAF, pace_)`. The steady clock serves `pace_ × base` — it already serves the in-band mean, and a halved rate is in band at 33; CC confirms nothing in the served-dt path assumes 16.7. Boot param `?pace=1|2` forces it (the `boot_params` surface, one field, announced on the `[Params]` line).

**(b) The governor**, release-build-safe: each frame records `submit_t`; `wgpuQueueOnSubmittedWorkDone`'s callback records `done_t`; `gpu_latency = done_t − submit_t` is the frame's GPU envelope proxy (it includes queueing behind the previous frame, which is exactly what a bound device shows). Trailing mean over 60 frames: `> 15.5 ms` for three consecutive windows → `pace_ = 2`; `< 12.5 ms` for three windows → `pace_ = 1`. At pace 2 the proxy still measures the GPU (a frame that fits reads ~half the interval), so the disengage rule is observable. Hysteresis is the two thresholds; the windows are the damping. Never engages under `?pace=`. **Witness:** one line per switch — `[PACE] 60 → 30 (gpu 17.2 ms over 3 windows)` — and the `[PRESENT]` histogram on the laptop's second world: the 12/24 alternation becomes a flat 2×. **Taste gate:** Jean rides the ribbon on the laptop at `?pace=2` and at `?pace=1` and says which is the piece.

### U7 — the deploy (ruling made executable)
`web_dist.py` reads a build stamp the CMake preset writes beside the artifacts (`web/build_preset.txt`: the preset name) and prints it in the inventory; if the preset is `the-board-web-meter`, the verdict line says so in capitals and `--lab` is required to write `dist/`. The audience gets `the-board-web`; the meter's timestamp writes serialize passes on a tiler and its prints are not free. If the preset cannot be stamped without a CMake change, STOP and file: the ruling stands as a ruling.

### U8 — OPEN.md
- KILL PANORAMA_0's F3 line if U1+U2 empty it; else rewrite: F3 proper (per-slot regeneration, workgroup-per-mesh) is priced, waits on the post-round Pixel firing count.
- ADD: the subtraction table's home (the round report) and what round three reads from it: E-2 (shadow split) vs a smaller map vs a near cascade, decided by `shadow_mask`; the main pass's whale, named by `draw_mask`.
- ADD: F (cell colour bake) is cold until the analyser socket binds (`12 sources unbound`); re-opened by the socket's campaign.
- ADD: BOOT_0's shape (PANORAMA_0 §4) as the next campaign after this round.
- ADD: round three's order is decided by a Pixel console after this round.

## 3. Jean's gates after the round

1. Build per commit; U4 owes the WGSL gate.
2. Walk one open world on each device: the firing count (U1/U2), and the subtraction table — eight main-pass masks and two shadow masks, one window each.
3. Two photographs (U5). One shadow edge at 16 and 4 taps (U4).
4. Ride the laptop at `?pace=2` and `?pace=1`, and let the governor pick once with no param (U6). Read the `[PACE]` lines.
5. A Pixel console — walk and ride — sent with the table. Round three is written from those two.
