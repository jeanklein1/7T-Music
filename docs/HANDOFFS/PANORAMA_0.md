# PANORAMA_0 — where the frame goes, and how to give it back

> **STATE (CC, at the round's close).** This work order is OPEN: five units
> landed, seven campaigns are held. It lives here for that reason and dies
> when the last of them closes (L31). Per-item status is in `docs/OPEN.md`
> under `## PANORAMA_0` — the one home for open state (L32) — and is not
> duplicated into the body below, which stands as written.
>
> **Landed:** B/RIDE_1 (F10, the queue bound) · A/RIDE_0's compute half
> (F1 `update_cube`, F2 `update_other_agents`) · E-1/LIGHT_0's PCF
> early-outs (F5) · F14's absence sentence · §5.5's mesh-gen firing
> instrument.
>
> **Held:** A's mesh half (F3) · C/RIDE_2 (the bake by rows) · D/RIDE_3
> (the photograph) · E-2 (the static/dynamic shadow) · F/CELL_0 · G/FIELD_0
> · H/PACE_0 · §4.1/4.2 (the boot campaign) · §4.4 (the bundle) · §4.6
> (the preset) · I/CARD_0 · §5's remaining instruments.
>
> **One finding did not survive recon:** F14's first half. The READY
> floor's clock is armed from the first frame, not from boot — see the
> OPEN.md line. The rest of §2 was verified against the tree before use.

---

Read from: `CONSOLE_CHROME.md` (laptop, NVIDIA Kepler, canvas 689×607, dpr 1), `CONSOLE_pixel.md` + `CONSOLE_pixel_ribbon.md` (Pixel 8 Pro, Mali Valhall, canvas 1495×672, dpr 2.25 capped 1.5), the tree at OVERTURE_0's close, and `world.wgsl` (698 KB, 14,799 lines). Every number below is a `[METER]` window mean or max with its window named; per L36 the reading is the ecology of many windows on two devices, not a seed.

The captures are of the **meter build** served from `everexpandingboard.com` — see §4.6.

---

## 0. The reading

Steady-state means over the `[METER]` windows while walking (the ribbon windows are called out separately). GPU rows are pass spans in ms; CPU is `frame_total`.

| | laptop (Kepler) | Pixel 8 Pro (Mali) |
|---|---|---|
| GPU envelope, mean | **20.3 – 23.4** (every window) | **13.6 – 16.1** |
| frames over 16.6 ms | 100% of sampled | 7 – 30% |
| `main_pass` | 10.3 – 12.8 | 10.7 – 11.4 |
| `shadow_pass` | 5.9 – 8.0 | 5.1 – 9.2 |
| `dispatch_compute` | 2.1 – 2.4 | 2.4 – 4.7, max 8 – 12 |
| `live_card_write` | 0.9 – 1.3 | 0.4 – 1.4, max 5 – 8 |
| `entity_mesh_gen` | 0.1 – 0.5, max 3 | 0.02 – 0.8, max 4 – 7.7 |
| `stream_patches` gpu (the bake) | 0.1 – 0.7, max 6 – 10 | 0.05 – 2.0, max 3.5 – 17; 206 at a portal; 1,604 at the first bake |
| `snapshot_pass` | mean 0.1, **max 16.7 / 21.2 / 55.4** | mean 0.1 – 0.2, **max 19 – 25** |
| CPU `frame_total` | 1.9 – 3.0 mean; max 40 – 180 | 1.2 – 1.8 mean; max 8 – 30 |

Two facts the table states:

1. **The CPU is asleep.** 2 ms of a 16.6 ms frame, both devices. Every optimization campaign the tree has run on the CPU side (queues, budgets, dirty flags) has already won; the remaining CPU cost is spikes, not load — the transition frame (`transition_machine` 57 – 67 ms + the fullRegen's `stream_patches` 55 – 93 ms, hidden by the fade), and the cell-crossing frame (5 – 8 ms on the laptop's CPU).
2. **Both devices are GPU-bound, and differently.** The laptop's baseline is 20+ ms — it cannot make 60 at any window size. The Pixel's baseline is 15.5 ms — it makes 60 with **0.5 – 1.4 ms of purse**, and every event that costs more than the purse drops a vblank.

On the Pixel the pass spans **sum to ~24 ms while the envelope is 15**: a tile-based GPU overlaps one pass's fragment work with the next pass's vertex and compute work. The envelope is the truth; per-pass gains are upper bounds there. On Kepler the sum equals the envelope (20.2 vs 20.4) — passes run serially, and every pass millisecond is a frame millisecond.

### The ride, in the `[PRESENT]` histogram

Frames per refresh multiple, per ~60-frame window:

| | 1× | 2× | 3× | 4×+ |
|---|---|---|---|---|
| laptop, walking | 51 | 8 | 1 | 0 |
| **laptop, riding** | **37 – 45** | **5 – 12** | **4 – 10** | **4 – 9** |
| Pixel, walking | 60 | 0 | 0 | 0 |
| **Pixel, riding** | **53 – 59** | **1 – 6** | **0 – 2** | 0 |

While riding, the laptop spends a third of its frames at three or four refresh intervals; the Pixel drops one frame in ten to fifteen. The iPad, with two to three times the Pixel's GPU, has purse enough that no event crosses it.

**One sentence:** the world is drawn at a cost that fills the frame on the audience floor, and the ribbon is where the events that were spread out while walking arrive together.

---

## 1. Why the ribbon stutters, from first principles

A vsynced display shows a frame at 16.6, 33.3 or 50 ms — never 20. A GPU frame of 20 ms presents at 33; a frame that is 15 on most frames and 19 on some presents at 16 mostly and 33 sometimes. The eye reads the second case as stutter and the first as slowness; the ribbon's fast camera turns every 33 into a visible hitch, where the walking pawn's turns it into nothing.

Riding lines the events up. At ~40 wu/s the window recenters every ~1.2 s, and each crossing demands: 15 patches allocated (CPU 5 – 8 ms on the laptop, in one frame), 15 spawns (`SPAWN=14` in `[STREAM]`), 15 bakes at 2 – 4 ms of GPU each, one per frame — fifteen consecutive frames each 2 – 4 ms over the baseline; every arch, palm, cactus, blade, column that spawns or evicts regenerates its **entire family's** mesh (4 – 8 ms on Mali per firing); every spawn raises `placement_dirty`; the photographer fires every ~30 wu (a 16 – 55 ms pass). On a device with a 1 ms purse, each of these is a dropped vblank; on the laptop, which has no purse at all, each is a second or third one.

So the cure has three parts, and they are separable:

- **Lower the baseline** — the main and shadow passes and the compute chain, where the frame's 15 – 20 ms actually go (§3, campaigns A, D, E, F).
- **Flatten every event under the purse** — bake by rows, regenerate one mesh, photograph at LOD1 over two frames (§3, B, C).
- **Pace by measurement** — a device that cannot make 60 after that should present at a steady 30, not a juddering 45 (§3, H).

---

## 2. What the tree computes that it need not

Read from `world.wgsl` and the dispatch sites. Each is a finding, with the fix in one line.

**F1 — `update_cube` is one thread.** `@workgroup_size(1)`, dispatched `(1,1,1)`, looping `for slot in 0..256`: two `manifold_position` queries (each a heightfield fetch through the patch grid plus the pyramid/zone/pulse overlay loops), `cube_behavior_force`, `influence_response`, the drift integrator — per cube, on a single GPU lane. No iteration reads another slot; the loop is embarrassingly parallel and runs serially. This is the `dispatch_compute` mean (2 – 4.7 ms) and its 8 – 12 ms spikes. **Fix:** `@workgroup_size(64)`, 4 workgroups, `slot = CUBE_SLOT_OFFSET + gid.x`; identical arithmetic, identical results.

**F2 — `update_other_agents` computes 296 field lanes on 32 threads.** Each thread runs `field_sum` for lanes `slot, slot+32, …` — nine or ten sums of ~450 pair evaluations each (32 agents + 8 spheres + 256 cubes + ribbon segments + 32 columns + 32 arch legs), serial per thread, with a dependent storage load per pair. **Fix:** dispatch `FIELD_SUBSCRIBERS` (296) threads in 5 workgroups of 64; thread `i` writes `field_forces[i]` and, if `i < 32`, runs the agent's behavior reading its own lane — no barrier needed because it is the same thread. The agent-agent read race is the one that exists today.

**F3 — mesh generation is one thread per mesh, and every mesh regenerates when one changes.** `arch_mesh_gen` (16×4 workgroups of one thread), `column` (32), `palm` (24), `cactus` (20), `blade` (32): each lane emits a whole mesh — thousands of vertices with trig — in a loop, and `prepare_*_mesh_gen` re-dispatches **all slots** on any `*_mesh_gen_pending`. During a ride, spawns and evictions keep it firing: 4 – 8 ms on Mali per firing. **Fix, two halves:** (a) per-slot regeneration — the pending set becomes a bitmask, the dispatch covers pending slots only (the kernels already index by `gid.x`; a slot list in the params ring, or one dispatch per pending slot); (b) a workgroup per mesh, `for (vi = lid; vi < count; vi += 64)`. Either alone is an order of magnitude; both are two.

**F4 — a per-cell colour is evaluated per pixel.** `patch_terrain_fs` calls `animated_cell_color(cell_center, addr_used)` whenever `has_mode_bias` (any of `mode_color_shift`, `mode_checker_scatter`, `mode_palette_intensity`, `checker_music_amount` is non-zero — the music's dials). That function runs `evaluate_cell_fields` + `discrete_cell_color_at_tier` (a 500-line lattice stack) and, when palette drift is on, runs them twice — for a value that depends only on the cell. A LOD0 cell is ~1,000 pixels: the same fact a thousand times per cell per frame, a million times a frame, and `generate_patch_cells` already knows how to bake it. In these captures the music was not playing (`12 sources unbound`) so the branch was cold; **during the recording it will not be**, and the main pass will grow by the size of that stack. **Fix:** `recolor_patch_cells` — one compute dispatch over the ring's patches (147 × 256 = 38k threads) rewriting `patch_cell_color_array` at the boundary cadence; the fragment shader reads the texture and only the texture. One fact, one writer. The three `DEBUG_VIEW` arms that ride the branch fold with it.

**F5 — sixteen shadow taps, unconditionally.** `sample_shadow_pcf` issues all 16 `textureSampleCompareLevel` taps and only then tests `out_of_bounds`; and it never tests `ndotl`. Terrain facing away from a 17° sun is in shadow by geometry. **Fix:** early return `1.0` on out-of-bounds and return the ambient path when `ndotl <= 0` before the taps (`textureSampleCompareLevel` is legal in non-uniform control flow). Expect 1 – 2 ms of the main pass on both devices.

**F6 — the snapshot draws the whole window at full mesh into a 512² photo.** `draw_patch_terrain_direct` over `render_patch_count` (147 patches) through `patch_index_buffer_lod0_live` — ~2.4 M triangles, no frustum cull, plus every entity and both painting draws — for a 262k-pixel image. That is the 16 – 55 ms `snapshot_pass` maximum, and the photographer fires every ~30 wu walked, in bursts of up to four. **Fix:** LOD1 index buffer for the photo; a photographer-frustum cull (the cull kernel with the photographer VP writing a second `fc_visible` window); and split the photo over two frames — terrain into the offscreen colour+depth in frame N, entities in frame N+1 with `LoadOp::Load` — the pose is frozen in the photographer VP, so nothing moves between the halves.

**F7 — the shadow pass redraws a texel-stable world every frame.** `coupling_pawn_to_sun_vp` snaps the light VP to whole shadow texels, so between crossings the map is bit-stable and the tree's own comment prices a reuse it never built. The map (2048², ±420 wu) covers a square of which 48% lies beyond the veil ring — ground nothing draws — and the pass draws all 147 ring patches at LOD1 plus every entity into 4 M depth texels: 6 ms on Kepler, 8 – 9 on Mali. **Fix (campaign E):** the static/dynamic split — a static map rebuilt over N frames (terrain and grounded entities), copied into the working map each frame by `copyTextureToTexture` with the texel offset the snap guarantees, dynamics (agents, ribbon, spheres, cubes, pawn) drawn on top. Flat ~3 ms instead of 8 with no spike, and the sun's orbit lags the static map by at most N frames.

**F8 — the heightfield is 4× denser than anything that reads it.** `PATCH_HEIGHTFIELD_N` 256 against `PATCH_MESH_N` 64: the vertex shader samples once per vertex, the fragment shader reads the interpolated gradient varying and never touches the texture, and the walkers/placement sample bilinearly. 128² is visually identical (two texels per LOD0 vertex), the bake is 4× cheaper (2 – 4 ms → 0.5 – 1 per patch; 206 ms → ~50 at a portal on the Pixel), and the array drops from **112.5 MiB to 28 MiB** — the largest allocation on the floor device. One `Dim` constant, one WGSL mirror, one gate: a pinned-seed side-by-side, which is the one case where a pinned seed is the right witness.

**F9 — the live card rewrites 410k texels every frame** while any zone is active anywhere (nearly always). Its content moves at music tempo, at 30 wu/s, and at GoL period. **Fix:** write half the rows per frame (each texel at 30 Hz, constant cost, no alternation) — 1.2 → 0.6 ms; or shrink `LIVE_CARD_EXTENT` to what the readers reach (the allocation window is 400 wu, the card reaches 500). Lowest priority of the list; the gain is real but small.

**F10 — the spawn queue's proven bound is wrong at every birth.** `SPAWN_QUEUE_MAX = SPAWN_BUDGET_PER_FRAME (2) × PopFamily::COUNT (12) = 24`, but the fullRegen arm hands `spawn_selected_patches` all 49 priority-window patches in one call. `[SPAWN] entityQueue_ OVERFLOW at 24` fires **at boot and at every portal on both devices** (11 drops in one laptop world; 6 – 12 per Pixel world) and what it drops is `gall`, `arch`, `cube`, `col`, `gol` — the tail families, galleries first. OVERTURE's ring is being truncated by this. **Fix:** drain per patch — `select → place → commit` inside the loop over candidates. Placement order is unchanged (select already queues patch-by-patch; footprints register at place, so patch N+1's place sees the same ground either way), and the bound becomes true (≤ COUNT entries ever).

**F11 — pipelines compile synchronously, all sixty, before the first frame.** `createComputePipelines` then `createRenderPipelines`, serial, on the main thread: 57 s on the dev laptop's first visit (204 s in one capture), 31 ms when cached. **Fix (campaign G):** `CreateRenderPipelineAsync` / `CreateComputePipelineAsync` — Dawn compiles on its worker pool, the storm divides by cores, and the page stays responsive — plus a first-frame set (terrain, agents, compute chain, shadow terrain, the entity draws) so the world can start while the indoor and photographer pipelines finish behind it.

**F12 — first use compiles again on mobile.** The Pixel's first bake cost 1,604 ms of GPU (`stream_patches` max, first window) against 206 ms later: Mali's driver compiles at first dispatch, not at creation. The first portal pays the same for the room pipelines. **Fix:** a warm-up pass at boot, behind the veil — one minimal dispatch/draw per pipeline into a 4×4 target.

**F13 — the audience is served the meter build.** `[METER]`, `[PRESENT]`, `[STREAM]` print from `everexpandingboard.com` on the Pixel. Timestamp writes at every pass boundary serialize passes a tiler would overlap; the census prints cost 31 – 98 ms of CPU per firing (with DevTools open — partly the capture's own cost). The instruments are the lab's; the audience gets `the-board-web`.

**F14 — the READY floor fired on the timeout on the laptop.** `Controls:` printed after `Staged 0/57`, before `PAINTING_1` landed (CHROME line 259 vs 264); on the Pixel it printed after `PAINTING_6` (line 314 vs 280), as ruled. The 5 s clock is armed from boot, not from the first frame — the device request and download spend it before a frame exists. One-line fix at U9's site. Cosmetic beside it: the conductor's first fill prints `No paintings folder found` one frame before the manifest lands; the sentence should be the manifest's ("not yet here").

**Not twice, checked and cleared:** the aura is sampled per vertex (height) and per pixel (colour) for different facts; the card is written once and read by vertex and placement; the bake's band stack and the card's are the same functions at different times by design; the frustum cull, the cull plan and the LOD band read one live point; `witness_harvest` reads back the point one frame stale by law and does not stall.

---

## 3. The plan

Ordered by gain per unit of risk. Gains are stated per device against §0's means, as upper bounds on the Pixel (overlap).

### A. RIDE_0 — the compute lanes (F1, F2, F3b)
Three kernels get real thread mappings; no equation changes. Pixel: `dispatch_compute` 3.7 → ~0.7 ms mean and the 8 – 12 ms spikes vanish; `entity_mesh_gen` firings 4 – 8 → <0.5 ms. Kepler: ~1.5 ms. **Risk:** nil to the picture; the agent-agent read race is today's. **Witness:** the same three meter rows, one window each side, and the `[PRESENT]` 2×/3× columns on a ride. **This is the campaign that gives the Pixel its purse — on the order of 4 – 6 ms of frame.**

### B. RIDE_1 — the queue bound (F10)
Per-patch draining in `spawn_selected_patches`; delete the `SPAWN_QUEUE_MAX` derivation's false clause. **Witness:** zero `OVERFLOW` lines across a boot and three portals; the `born` census's `gall` row on the same seeds.

### C. RIDE_2 — the runway (the bake by rows, F8's neighbour)
The bake's unit becomes rows: `BAKE_ROWS_PER_FRAME` (64 = a quarter patch) with the in-flight patch's row cursor in the params ring; heights over `[cursor, cursor+rows)`, then gradients + cells on the last slice, then `GENERATED`. The scratch is one patch's and persists across the slices. The pace scales with the backlog — `rows = 64 × clamp(backlog / 4, 1, 4)` — so a world that has fallen behind still catches up, evenly. The young world keeps its whole-patch burst behind the fade. Pixel: a 2 – 4 ms event becomes four 0.5 – 1 ms ones under the purse. **Witness:** `stream_patches` gpu max across a ride; the `[STREAM]` `GEN` count keeping pace with the crossing.

### D. RIDE_3 — the photograph (F6)
LOD1 terrain, the photographer's own cull window, two-frame composite. Removes a 16 – 55 ms hitch every ~2 s of walking on both devices. **Risk:** none to the photograph's content (the pose is frozen); the snapshot's terrain detail drops from LOD0 to LOD1 in a 512² image — a taste gate Jean passes by looking at two photos. **Witness:** `snapshot_pass` max.

### E. LIGHT_0 — the shadow's two halves (F5, F7)
Two commits: the PCF early-outs (an afternoon; 1 – 2 ms of main pass on both devices), then the static/dynamic split. **Design of the split:** two depth16 targets of today's size; `shadow_static` is rebuilt over N=4 frames (147/4 patches + grounded entities per frame, world-anchored at the rebuild's start, sun direction sampled at its start); each frame `copyTextureToTexture(static → working, origin = the snap's texel offset)` then the dynamic casters draw on top; the fragment shader is unchanged. The uncovered strip a moving pawn opens at the map's far edge is 3 texels walking and ~32 riding, all beyond the ring (420 − 342 = 78 wu of margin), so it never enters view while δ per rebuild stays under 78 wu. Pixel: 8 → ~3.5 ms flat; Kepler 6 → ~2.5. **Risk:** the static shadow lags the sun's orbit by ≤4 frames (invisible) and the dynamics by none. **Witness:** `shadow_pass` mean and max; a sun rotated by the organ's dial while standing still — the shadows must follow within a beat.

### F. CELL_0 — the cell colour has one writer (F4)
`recolor_patch_cells` at the frame boundary when any mode dial is non-zero; the FS branch dies. **Witness:** `main_pass` with music driving `checker_music_amount`, before and after — the before number does not exist in these captures and must be taken first.

### G. FIELD_0 — the heightfield at 128² (F8)
One constant, two rooms, one visual gate on a pinned seed (the kernels' stencil spacing derives from `resolution`; CC confirms). Gains: bake ÷4 (the portal's 206 ms → ~50 on the Pixel; the ride's runway four times cheaper), 84 MiB of texture returned to the floor device, four times less bandwidth for every sampler. **Risk:** sub-vertex detail in placement and the walkers' ground — centimetres on a 0.78 wu mesh.

### H. PACE_0 — the metronome
After A – G a Kepler laptop at full width still cannot make 16.6. The honest answer is a steady 30: when the measured envelope exceeds the budget for three windows, present on every second vblank (skip the alternate `requestAnimationFrame`) and serve the steady clock the 33.3 ms interval; return with hysteresis. The steady clock exists; the governor is a switch on it, witnessed by one `[PACE]` line. A film at 24 is smooth; a game at 45 is not. **Risk:** none to the picture, half the temporal resolution on devices that could not keep it anyway. An optional second dial for the ribbon alone — render scale keyed to the point's speed (1.0× at rest, 0.8× at full throttle; the motion hides it) — is a taste gate.

### I. CARD_0 — the live card by halves (F9)
Last, and only if the purse still wants it.

Landing order: **B → A → E-1 (PCF) → D → C → G → E-2 (split) → F → H.** B is a bug; A is the largest gain at no risk; E-1 and D are afternoons; C and G are one campaign's shape; E-2 is the one that needs a design round; F waits on its own measurement; H waits on all of them.

---

## 4. Boot: the first visit

4.1 **Async pipelines with a first-frame set** (F11). The single largest first-visit lever: 60 serial compiles become a pool, and the world starts on the subset the first frame needs. The veil's `Compiling shaders (n)` wording already exists; it will simply count faster.

4.2 **A warm-up pass** (F12) behind the veil, so the first bake and the first room pay nothing at their first use on Mali/Adreno.

4.3 **The exhibition** — four lanes and the deferred hang landed; the READY clock (F14) is one line.

4.4 **The bundle.** `world.wgsl` ships inside `the_board.data` as octet-stream; Cloudflare does not compress that type by default, so 698 KB cross the wire uncompressed on every first visit (`.wasm` is compressed). Either a `_headers` content-type rule for the `.data` file, or `web_dist.py` strips the shader's comments into `dist/` and computes the serve digest from the stripped bytes — the `[Dist] world.wgsl sha … MATCH` witness holds because both halves read the shipped file. Roughly 700 KB → 150 KB.

4.5 **The first `fullRegen`** — 98 ms of GPU on Kepler, 206 ms on the Pixel after warm-up — runs behind the veil at boot and behind the fade at a portal; G halves it twice over; C keeps the young burst.

4.6 **The deploy** (F13). `the-board-web` for the audience; `the-board-web-meter` for captures. The meter's timestamp writes are not free on a tiler, and the census prints are not free anywhere.

---

## 5. Measure first

Instruments before mechanisms; each is one dial or one line, and each names the campaign it gates:

1. **A release-vs-meter window** on the Pixel: the same walk, both presets, envelope mean. Gates §4.6 and calibrates every number above.
2. **Shadow terrain OFF** as an organ dial (the shadow pass draws entities only): the terrain's share of `shadow_pass`. Gates E-2's design.
3. **PCF 4 vs 16** as a dial: the tap cost on both GPUs. Gates E-1's second half.
4. **`main_pass` with music on**: the size of F4 today. Gates F.
5. **A per-family firing count for mesh gen across one ride** (one counter, printed with the meter window). Gates A's F3 half and proves it after.
6. **`BAKE_ROWS_PER_FRAME` as a dial** from the first commit of C, so the pace is tuned by eye and meter rather than argued.

---

## 6. Rulings for Jean

1. **A (RIDE_0)** — the compute lanes: no aesthetic change; stamp.
2. **B (RIDE_1)** — the queue bound: a bug; stamp.
3. **D (RIDE_3)** — the photograph at LOD1 over two frames: a taste gate on two photos.
4. **E** — PCF early-outs (stamp) and the static/dynamic shadow (a design round; stamp the round).
5. **G (FIELD_0)** — 128²: a pinned-seed visual gate, your eye.
6. **H (PACE_0)** — a steady 30 on devices that cannot make 60; and whether the ribbon may drop render scale at speed.
7. **§4.6** — which preset the site serves.
8. **§4.1** — async pipelines and a first-frame set: a boot campaign of its own; stamp the campaign.
