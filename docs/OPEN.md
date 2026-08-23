# OPEN — the register of open state
One line per item: what · origin (sha or doc) · what unblocks it.
This file is the ONLY home of open/parked state. When an item closes, its line dies.

- PIPE_0: PARKED per Jean's 2026-08-07 directive. Origin sha of
  docs/HANDOFFS/WEB/PIPE_0_DECISION.md (path retired to git; content at its last sha).
- STREAM_0: PARKED per Jean's 2026-08-07 directive. Origin sha of
  docs/HANDOFFS/WEB/STREAM_0_DECISION.md (path retired to git; content at its last sha).
- DAWN_REFERENCE web-era rewrite: docs/reference/DAWN_REFERENCE.md is
  archival-with-named-drift (its own CANON stamp); the rewrite awaits Jean's
  reference round. Origin: the stamp itself.
- GUARD DEBT in realization/renderer.hpp: four __EMSCRIPTEN__ guards survive
  SUNSET_1 (census `git grep -n "__EMSCRIPTEN__" -- src`). Coverage is
  SPLIT, not blind: console_gate defines __EMSCRIPTEN__ (fidelity with emcc)
  and so type-checks the shipping arms; glaw1 does not, and so type-checks
  the dead ones. The sitting decides two things: collapse the guards
  (SUNSET_1's stated intent; may cost stub declarations), and whether
  glaw1's macro set should match the build's. Origin: RECENSION_1 FLAG-7,
  corrected by RECENSION_4 when the gates were first read whole. Unblocked
  by the gates sitting.
- L26 docket — fields marked dead, awaiting the sitting that next opens their
  struct: `OrbConfig`'s driverless gen-1 block, `Instruments.watcher_ticks` (driver
  went with the FileWatcher at SUNSET_1; `pawn.cpp:60` still names the dial),
  `OrbMoodConfig.base_hue` / `.hue_variance` (dead BY CONSTRUCTION — every
  ORB_PALETTES row carries count ≥ 1, so `pack_palette_` never leaves
  `palette_count` at 0 and the kernel's legacy single-hue arm is
  unreachable; ORGAN_4 P1b), and `OrbMoodConfig.motion_rule` (dead TREE-WIDE
  — `configure_orbs` writes the player's `os.current_motion_rule` and never
  reads the config field; ORGAN_4 C2). The three orb fields stay put rather
  than dying now because `ORB_MOOD_TABLE` is positionally brace-initialised
  (D3: the braces outrank the broom), and the twin rooms — table and struct
  — must move in one commit. Each dies in the commit that reopens its struct
  (L26, L3: twin rooms, one commit). Origin: L26, moved here by RECENSION_2;
  grown by ORGAN_4 P1b. Unblocked by any campaign that relayouts one of the
  three structs named.
- Dead boot write, awaiting the boot block's next sitting: state.hpp's
  `config_.aura_enabled = 1.0f` boot seed is overwritten within one frame by
  tick_pawn_couplings (the driven window's only runtime author since
  ORGAN_2a). Origin: ORGAN_2a D3. Dies in the commit that next opens the
  boot-config block.
- CENSUS_1b, the exhaustive walk of the four realization giants (state.hpp,
  world.wgsl, renderer.hpp, cartridge.hpp) and the ten bodies/** files, line
  by line. CENSUS_1 covered them by sweep and declared that edge; the refuter
  breached it immediately (gol_zones.hpp's "Upload all 7 slots" against a
  5-slot stride). Origin: PROCESS_LAWS SCHEDULING RECORD, moved by
  RECENSION_3 when its dated owner (the control-panel campaign) ran without
  collecting it. Unblocked by any campaign that enumerates those files.
- pawn.cpp IS UNGATED: no gate compiles the TU that owns main(), the rAF
  driver and the boot sequence — glaw1 compiles tu.cpp, console_gate compiles
  cartridge.hpp and console.hpp. One missing declaration is the whole reason:
  tools/gates/console_gate/stubs/emscripten.h stubs
  emscripten_set_main_loop_arg but not emscripten_set_main_loop, the form
  pawn.cpp calls. CC verified everything else in the TU compiles clean under
  the existing gate flags. Cure is one stub line; adding to the pinned stub
  set is a gates decision. Origin: RECENSION_2 FLAG-U5c. Unblocked by the
  gates sitting.
- FIREFOX STAGING RATCHET (bounded; Firefox-side; the cheap cure landed, its
  witness owed): Firefox on Windows compiles the module with no diagnostics,
  boots, runs 30–56 fps on the Kepler, and loses the device with
  `[Device] LOST reason=1 : Out of memory` after several world transitions —
  rapid keyed transitions reach it in under a minute, a calm session in many
  minutes or never. about:memory, GPU process, across ONE keyed transition:
  gpu-committed 1003 → 1308 MiB; private write-combined commit 858 → 1089 MiB
  in 37 → 43 segments; a second transition +15 MiB, 46 segments; flat for
  minutes on either side. The program's own GPU budget is 251 MiB throughout
  and creates nothing after boot. Reading: Firefox's wgpu serves
  WriteBuffer/WriteTexture from upload-heap blocks it sub-allocates; a
  transition's upload burst — the paintings re-staged at teardown, 1 MiB
  WriteTextures landing several to a frame — opens blocks the per-frame trickle
  then keeps from emptying. Chrome's Dawn and WebKit recycle differently: an
  iPad (Safari 26) ran 10+ min with transitions. ORGAN_8 P3 paced the paintings
  to one lane (AUTHORED_FETCH_INFLIGHT_CAP = 1); its witness is a before/after
  about:memory pair across one keyed transition, Jean's. If the step survives:
  the structural cure is that the program owns its staging — a fixed ring of
  MAP_WRITE buffers made at boot, mapped, filled, unmapped and copied from, so
  the browser allocates no staging at all — priced at one round over
  GPUState's upload doors, not built. Origin: COMPAT_1's witness runs,
  2026-08-21.
- In-place same-shape atmosphere transition (sunset → night without a
  teardown, the sky re-drawn over a standing world): priced at ATMOS_1,
  not built — the spawns' population row is per mood, so a "same shape"
  transition is not yet a no-op below the sky. Origin: ATMOS_1 §7 — the
  campaign's handoff, held by Jean rather than filed in the tree; the
  campaign opens at 71619c0. Unblocked by a visual need (the recording's
  long takes) or the indoor atmospheres' rework.
- The regime's second subscriber owes the flag (REGIME_1): the four weight
  rows raise the MOOD definition flag only, so a weight edit re-rolls and
  re-speaks the sky and nothing else. The first family to carry a regime
  column (the orb mood bank is eligible today) must make the weight rows
  raise its definition flag too — a per-field mask or a "regime law"
  hook at the boundary — or a regime change leaves it behind. Origin:
  REGIME_1 §0.5.
- A per-parameter "mood-wide" flag for the regimes (ATMOS_2 §0.5): today a
  parameter wanted the same in every regime is set equal in every regime,
  four dials that agree. The flag would make it one dial and restore
  "one fact, one home" at the tuning level; it costs a selector row per
  parameter and a draw that reads regime 0 when the flag is set. Also
  priced, not built: a second independent roll (axes), should the
  combinatorics of independent light and fog be wanted back; and moving
  the sun's bearing into the regime row. LENS_1's ALL position writes
  every regime at a stroke, which is the gesture form of the same relief;
  the flag remains the one-home form. Unblocked by Jean's tuning asking
  for any of them. Origin: ATMOS_2 §0.5.
- naga is installed per session in CC's container with `cargo install
  naga-cli` (minutes), and `tools/wgsl_gate.py` then runs in-container on the
  raw module. Origin: ATMOS_1 report FLAG 12, answered at COMPAT_1.

## RIBBON_1 — the witnesses Jean owes the campaign

No gate can see any of these; they are the eye's, and the campaign is not
closed until they are looked at. Origin: RIBBON_1 (three commits on
`claude/ribbon-1`, base `fd53316`). Three were looked at, and RIBBON_2's
charter is what came back; the finding is noted under each. None is closed
here — the sign-off is Jean's, and RIBBON_2 has its own list below.

- THE FLOWN BODY FOLLOWS ITS TRACK. The spine became a SPACE law: one
  sample per chord of flight, so the body is drawn where the head has been.
  Tier 0 at full throttle and every parked ribbon are unchanged; tiers 1
  and 2 and every wanderer are visibly different from `fd53316` — where the
  body used to whip, laying rings at `cube_size` regardless of how far the
  head actually travelled, it now trails the path. This is the ruling to
  overturn if the old whip was the beauty.
  LOOKED AT (RIBBON_2 §0.3): the space law stands — the whip was not asked
  back. What was owed was the settling the time law's delay line used to
  give when the hands went still, which a space law has no version of. THE
  SWEEP is that version; witness (2) below.
- THE SKY RULE, both readers. The head should bank away from a shaft and
  climb a roofline before it reaches one (COLOSSAL antennas are the test:
  125 m of post with drums 20 m wide at the top). The body should bulge
  around what it meets and flow through the bulge — the bulge stays where
  the thing is — and lift over a pyramid rather than enter it.
  LOOKED AT (RIBBON_2 §0.1), the BODY's arm only: it still entered things,
  and the reason was structural, not a tuning — a critically damped string
  settles INSIDE a shell that only pushes it. The shell became advice and
  stands wider; THE WALL became law; witness (1) below. The HEAD's arm —
  banking away from a shaft and climbing a roofline before it reaches one
  — is still owed as written.
- THE SEAT. `R` boards and lands on an ease, never a teleport. The saddle
  sits on ring 0's top face through roll and pitch; the nose faces where it
  swims and the tube does not shear at the neck.
  LOOKED AT (RIBBON_2 §0.5): the ease and the saddle drew no finding. What
  came back was the camera, which kept the orbit the pawn had left it in
  instead of turning behind the rider; THE CHASE answers it, witness (4)
  below. The seat itself stays owed a verdict.
- THE DIALS MOVE THE FLIGHT LIVE. Sixteen `Ribbon · Head` / `Ribbon · Sky
  Rule` / `Ribbon · Wander` rows on the organ (twelve at RIBBON_1, four
  more at RIBBON_2) reach the kernels through `config.ribbon_*`; turning
  one mid-flight should change the flight without a respawn.
- FIREFOX. Three `WriteBuffer`s a frame per ribbon, where fifteen stood,
  and 6.4 KB of ring poses no longer uploaded twice per frame. The staging
  ratchet above reads against this.

### Priced at RIBBON_1, not built

- THE GESTURE RING. `[SEAM:ribbon-displacement]` (world.wgsl §6.5) now names
  its own shape: to let music drive the head's displacement, record the
  head's lateral/vertical into a GESTURE ring beside the spine —
  time-cadenced where the spine is chord-cadenced — and read the delayed
  samples in `ribbon_displacement_at`. Unblocked by the music-coupling
  campaign.
- THE SKY RULE'S COST. `sky_push` walks 344 emitters (32 shafts, 16 arches,
  32 walkers, 264 floaters) per reader, and `ribbon_body` runs one reader
  per ring — up to 400 — plus `ribbon_ground`'s analytic terrain per ring.
  No measurement exists; `the-board-web-meter`'s `ribbon_body` row is the
  first one to read. The cures if it bites, in order of cheapness: an
  EMIT_STRIDE-style stride on the body's rule reads, a broad-phase cull by
  the ribbon's own bounding box, or a shared per-frame shortlist the head
  builds once.
  RIBBON_2 RAISED IT, and the price is stated rather than measured, since
  no meter row exists to measure with: `sky_self` adds up to 200 capsule
  tests per reader on top of the 344 emitters, `sky_wall` adds a second
  walk of the standing things per ring, and the head — still ONE thread —
  now also sweeps the spine, ≤ 401 serial steps a frame. The sweep is the
  one of the three that is structurally parallelizable (each chord reads
  only its predecessor, so it is a scan), and it is the first thing the
  optimization sitting should look at.
  RIBBON_4 IS THE ONE THAT MOVED THE FRAME, and it moved it DOWN without a
  meter to prove it: the worst streaming frame used to be six whole bakes
  plus four spawns plus four evictions, and is now one band of one bake plus
  one spawn plus two evictions. The total work is unchanged — the same
  patches are baked, spawned and evicted — so what this buys is evenness,
  not throughput, and the cost is that a leisurely patch arrives
  `PATCH_BAKE_SLICES` frames later than it used to. If terrain is ever seen
  ARRIVING at the edge of the ring rather than being there, that is this
  trade showing, and `PATCH_URGENT_MARGIN` is the dial that buys it back.

  RIBBON_3 MOVED IT BOTH WAYS, still unmeasured. Up: an arch costs 8 rib
  capsules plus 2 piers per arch per reader where it cost one disc — 16
  arches, so up to 160 capsule tests added to every `sky_push` and every
  `sky_wall` call. Down, and by more: the head now hears 8 floating slots
  instead of 264, which removes up to 256 sphere tests from the head's
  reader every frame, and `sky_roof` lost its arch loop entirely. The head
  is still ONE thread. The body's reader is where the arch rib is actually
  paid, once per ring.

## RIBBON_2 — the witnesses Jean owes the campaign

Six, in Jean's own order from RIBBON_2 §0. No gate can see any of them.
Origin: RIBBON_2 (three commits on `claude/ribbon-1`, base `a76bbed`).

- THE WALL. No ring center stands inside a drum, a pyramid, a walker or a
  floater — nor under ground plus half a tube. (An arch is no longer a solid
  in this law: RIBBON_3 made it a doorway, so the wall keeps rings out of
  its rib and its piers and leaves the SPAN open.) The shell is
  advice now and stands wider (`clear_head` 25 → 40, `clear_body` 8 → 16);
  `sky_wall` runs AFTER the leash, because law outranks leash, and it kills
  only the velocity that points into the wall. The test case is a COLOSSAL
  antenna: 125 m of post with a drum 20 m wide at the top, which the old
  soft push let a ring settle 18 wu inside.
- THE SWEEP. Hands still → the body settles straight behind the head, and
  the settling FRONT should be seen travelling tailward at `propagation_
  speed`, not the whole body straightening at once. At full throttle the
  track is kept near the head instead. RULING TO OVERTURN WITH ONE WORD:
  the sweep runs at 1.0 × P idle and 0.15 × P at full throttle
  (`RIBBON_SPINE_RELAX_IDLE` / `_FLY`, world.wgsl §6.5).
- THE BODY IS A THING. A tight turn → the head goes OVER its own body and
  the body bulges off itself, both through the same Sky Rule that reads the
  world. The body it reads is last frame's emit half, minus a
  `RIBBON_SELF_NECK` of 24 rings so the neck does not fight itself. RULING
  TO OVERTURN WITH ONE WORD: over, not under, unless the body is clearly
  above.
- THE CHASE. `R` turns the camera behind the rider over the boarding ease,
  looking along the flight. OVERTURNED BY JEAN, and landed at RIBBON_3 P1:
  it does NOT re-center afterwards — the pose is taken once, at boarding,
  and the mouse owns the camera from then on (`RIBBON_CHASE_TAU` 0.0; > 0
  restores the idle-mouse settling). The elevation is his too:
  `RIBBON_CHASE_ELEVATION` 0.25 → 0.6, about 35° to the ribbon's surface.
  What is still owed is the look of that one pose — witness (4) below.
- THE WANDERER STEERS ITSELF. A wanderer should cross its anchor's disc
  target to target and come back, not drift away and never return. The
  brain is the head kernel's now — the target is drawn from the ribbon's
  own seed on a `roam_radius` disc, steered toward through the same yaw cap
  a rider's hands pass through, re-drawn inside `wander_arrive`. Four dials
  (`Ribbon · Wander`) move it live.
- IF THE MOTION STILL READS STEPPED. The polyline is gone (the spine is a
  C1 Hermite arc on each sample's own tangent), so if it still steps the
  cause is one of the three cadence facts, which are recorded here so the
  next sitting does not re-derive them:
  (a) `signal.dt` WAS a `std::chrono::high_resolution_clock` difference
  across `begin_frame()` (`console.hpp`), clamped to `[0, 0.1]` s, with no
  smoothing; `signal.t_seconds` is the running sum of that same clamped dt
  (`beat_clock.hpp`, `BeatClock::update`) — one clock, one integration.
  Under Emscripten that clock is `performance.now()`, which the browser
  coarsens. ANSWERED (RIBBON_3 P2): the measurement is made the same way and
  is still the only clock, but the value handed on is THE STEADY CLOCK's.
  Witness (5) below is what to check if judder survives it.
  (b) `dispatch_compute` is an unconditional row of `RENDER_SPINE`
  (`cartridge.hpp`), so it is recorded on EVERY rendered frame. But
  `update()` runs on frames that are never rendered — `pawn.cpp` returns
  before the encoder when `acquire_surface_texture()` fails — and
  `phase_fill_signal` has already written that frame's dt, which the next
  update overwrites. A dropped acquire therefore does not stretch the
  head's step; it DELETES one. LANDED (RIBBON_3 P2): the ruling went the
  carry's way. `phase_fill_signal` accumulates into `dtPending_` and the
  host clears it only once the frame's command buffer is submitted, so an
  updated-but-unrendered frame now stretches the next rendered step instead
  of deleting itself. The sum carries the same 100 ms ceiling the raw
  measurement does — a stretch is a stretch, a teleport is not.
  (c) The web main loop is `requestAnimationFrame`
  (`emscripten_set_main_loop(frame, 0, true)`, `pawn.cpp`).
  The gesture clock is NOT a suspect: `ribbon_frame_tick` advances
  `par.phase += beat_rate × (60 / reference_bpm) × dt` every frame — a
  smooth per-frame float, never a beat-quantized tick — which is why §3.5
  took its first branch and the clock did not come home.

## RIBBON_6 — ONE BAKE A FRAME, AND THE PRESENTATION LAW

Origin: RIBBON_6 (two commits, base `3493a05`). This round WITHDREW a premise
two earlier rounds were built on, so read this entry before theirs.

- **WHAT THE METER SAID ALL ALONG**, recorded so no future round re-derives
  the streaming hypothesis: across three steady windows of Jean's own
  recording, `stream_patches` never exceeded **2.00 ms of GPU** and its means
  were 0.01–0.02 ms; `frame_total` never exceeded **2.21 ms of CPU**; fps
  59.9 / 60.0 / 60.0. **Streaming never cost a frame.** RIBBON_4's charter
  held that the conductor's bursts were the blocks along the way; they were
  not, and everything built on that has been withdrawn. Origin: RIBBON_6 §0.
- ONE BAKE A FRAME. Fly straight at the top of the speed dial for a minute.
  The world should keep up without holes, and no frame should carry more than
  one bake. In a meter build `[STREAM]` shows `young=0` throughout — **if
  `young` ever flickers to 1 during ordinary flight, this round's diagnosis is
  wrong and that flicker is the whole finding.**
- THE PRESENTATION LAW. `[PRESENT]`, riding and walking, at the exhibition
  canvas. A near-pure `1x` column with the ride reading smooth closes the
  campaign. A fat `2x` column names the cost as GPU-side and hands the
  optimization sitting its target — and the meter's own first window already
  says where the budget goes: `main_pass` 7.4–11.7 ms and `shadow_pass`
  2.2–5.2 ms of GPU. The `[METER]` line now carries the canvas and an `over`
  count beside them, because a GPU budget read against an unknown resolution
  is not a reading.
- TWO RULINGS, each one word: `BAKE_BUDGET_PER_FRAME` (1 — the law; raising it
  buys catch-up and spends evenness) and the youth threshold (three quarters
  of the window, cleared once and never re-armed by anything the player does).

## Found by the RIBBON_5 audit, still open

- **THE POINT MIRROR CAN FREEZE, and nothing says so.** Every streaming
  consumer reads `c->point_` — the window centre, the eviction sort, the alloc
  box, the bake ordering, the draw band. TEARDOWN authors it and bumps
  `world_gen`, dropping every in-flight readback callback. If
  `pawnReadbackState_` ever wedges in MAPPING, `point_` never moves again:
  `gridChanged` stays false, the box raiser never fires, and `stream_patches`
  encodes nothing — a true 0.00 ms with a visibly moving player, which is the
  one mechanism that produces that reading. **The test is already in the tree
  and costs nothing to run:** in `[STREAM]`, `center=(cx,cz)` must equal
  `floor(point/50)`. If they ever disagree, the readback is the fault and no
  amount of conductor work will help. Unblocked by a meter-build session.
- The readback's own recovery has no witness either: nothing prints if a
  `MapAsync` never completes. Priced at one line beside the state machine,
  not built. Origin: the RIBBON_5 conservation audit, re-run adversarially.

## RIBBON_5 — THE WORLD COMES BACK

Origin: RIBBON_5 (two commits, base `b4fe1fb`). This one comes first: until
the world rebuilds, no other witness in this file can be read at all.

- THE WORLD COMES BACK. Walk through the door-fallback arch ON PURPOSE — it
  stands ~60 wu from spawn by design, so it is a short walk. The world fades,
  reseeds, and must **rebuild whole around the pawn within a few seconds**,
  then keep streaming as he walks. In a meter build the `[STREAM]` line (1 Hz)
  is the reading: `free` must return toward `MAX − active`, `young` must clear
  as the window fills, and `ALLOC`/`SPAWN`/`GEN` must move every second. The
  one-patch world is impossible while the conservation witness stays silent —
  and that witness runs in EVERY build, not just a meter one, so if it ever
  prints, that line is the whole diagnosis.
- FOR JEAN, two lines that are not defects:
  **The "mutation" moment was a door, not a bug.** Doors are arches; the
  fallback arch stands near spawn by design; crossing one is a world
  transition, and the patch underfoot changing IS the new seed's terrain
  arriving. That part was the program working. What was broken is only that
  the world never finished rebuilding afterwards.
  **RIBBON_4's circle-vs-straight witness is RETIRED**, not owed: RIBBON_6
  withdrew the premise it was written to test, and it was never reachable
  anyway.
- ONE RULING LEFT FROM THIS ROUND: whether the conservation witness stays
  always-on. It costs one O(225) walk a second and is the only thing standing
  between a layer leak and a silent one-patch world — the recommendation is
  that it never moves behind a dial. (The youth threshold and the young
  budgets moved to RIBBON_6's entry, which rewrote both.)
- WHAT THE AUDIT FOUND — and it found a real break, on the second pass. The
  first pass asked "does every site that clears `valid` return its layer?"
  The answer is yes, and the answer was useless, because the break runs the
  other way: **the continuous-allocation block checked pool capacity while
  COLLECTING candidates and never again while spending them.** The scan
  decrements nothing, so with `free == 2` and 15 vacant cells all 15 became
  candidates, `allocThisFrame = min(15, ALLOC_BUDGET_PER_FRAME) = 4`, and
  iterations 2 and 3 called `alloc_layer` against an empty pool — which
  silently returned **layer 0**, already owned by a live patch, and still
  wrote a valid record and incremented the count past the pool. Two records
  sharing one heightfield layer is one patch's terrain mutating under
  another, and `active_patch_count` past 225 writes `patches_[225]` — which
  is `freeLayerStack_[0]`, the member immediately after it. The bookkeeping
  error becomes memory corruption of the free list on the first frame it
  fires.
  **The trigger is ordinary and the regression is RIBBON_4's**: a built-out
  world, the player crosses one patch boundary, 15 cells go vacant.
  Before RIBBON_4, `EVICT` and `ALLOC` were both 4, so eviction always
  supplied what allocation could spend. RIBBON_4 lowered `EVICT` to 2 for
  the steady cadence and left `ALLOC` at 4 — and that gap is the hole. So the
  handoff's second defect was right in substance and wrong in mechanism: the
  pool did wedge and layer 0 did go to every comer, but by over-taking, not
  by leaking.
  Fixed at the site (`alloc_layer` refuses and says so; both loops honour the
  refusal; both write sites carry an independent array-bound guard), and the
  1 Hz conservation witness now proves it rather than assuming it. The
  zero-headroom equality remains (`MAX_ACTIVE_PATCHES == 225 == the 15x15
  window`): the pool has no slack by construction, which is exactly why a
  comment saying "this shouldn't happen" was never enough.

## RIBBON_3 — the witnesses Jean owes the campaign

Five. No gate can see any of them. Origin: RIBBON_3 (two commits on
`claude/ribbon-1`, base `92d58c6`), answering Jean's witness from the
saddle: *the flight accelerates almost smoothly, then breaks — like hitting
many blocks along the way, a few times a second, worse with speed.*

- NO BLOCKS. From the saddle, at speed, near things: the flight should read
  as ONE curve. Two causes were closed for it. THE STEADY CLOCK (P2) makes
  the frame's dt the display's cadence rather than the callback's arrival —
  the running mean while the measurement stays within ±20% of it, the
  measurement itself when it does not. ONE COMMAND, C2 (P1) low-passes the
  Sky Rule's lateral word (`RIBBON_RULE_TAU` 0.35 s) and slew-limits the
  total command (`RIBBON_YAW_SLEW` 1.5/s), so the heading's RATE is
  continuous where it used to step every time the probe crossed a shell.
  The head also stopped listening to the 256 cubes, each of which was a
  42-wu bubble at `clear_head` 40; it hears the standing things, the 32
  walkers and the 8 spheres, at `RIBBON_CLEAR_MOVER` 20. The cubes are the
  body's business.
- A DODGE IS A CURVE. An antenna is the test Jean already named as the one
  that looked RIGHT: it should now begin and end as a curve rather than
  snapping into and out of the avoidance. If it now reads too lazy instead,
  the two numbers are `RIBBON_RULE_TAU` (lower = quicker to hear a thing)
  and `RIBBON_YAW_SLEW` (higher = quicker to act on it).
- AN ARCH IS A DOORWAY. Jean's word was that the arch dodge looked STRANGE.
  It was: `sky_shell` gave every arch a disc of `half_span + max(thickness,
  depth)` and a top at the apex — for a MONUMENTAL a 70-wu drum 88 wu tall,
  which the head had to skirt or climb entire and the body was thrown out
  of. The tree's own law said the opposite all along (`occupier_contact`:
  *the SPAN stays open — walking through the doorway is the arch's whole
  meaning; only the legs push*). The ribbon should now pass UNDER, over or
  around, and the body should never be thrown out of a drum that is not
  there. The roofline is shafts-only for the same reason: a roof says the
  sky is closed at this xz, and an arch never closes it.
- THE CHASE POSE, ONCE. `R` → the camera takes its pose over the boarding
  ease, about 35° above the ribbon's surface, and then the mouse owns it —
  no drift back, ever. Both numbers are Jean's (`RIBBON_CHASE_ELEVATION`
  0.6, `RIBBON_CHASE_TAU` 0.0).
- IF JUDDER SURVIVES THE STEADY CLOCK, the next fact is not in this tree: it
  is the resolution of `performance.now()` on the machine in question, which
  is what `high_resolution_clock` becomes under Emscripten. Browsers coarsen
  it deliberately — commonly to 1 ms without cross-origin isolation, and
  Firefox with `privacy.resistFingerprinting` set coarsens it all the way to
  100 ms, which would quantize every frame's dt to a multiple far larger
  than a frame. That is a line for the optimization sitting, and it is
  measured in the browser, not read out of the source. THE IN-BAND SHARE, on
  the other hand, is readable here: at 60 Hz with ±2 ms of jitter the band
  is ±20% of a 16.67 ms mean, i.e. ±3.33 ms, so EVERY frame is in band and
  every frame is served the mean — the steady clock is fully engaged, not
  half of it.

## Harvested at WINNOW-2 W2 (from files that died this campaign)

- HARVEST: PREGEN-8 CONTINGENCY — rig-triggered storage weld (225→289 layers,
  TILE_GRID 17→19, MAX_ACTIVE_PATCHES) sleeps until the rig shows rim-pops under fast
  flight. (from audit/past reports/LADDER.md P-3, eafd9ec)
- HARVEST: TERRAIN-2 b3, the finite collapse — the radius-bounding choice (raise
  pregen / lower finite_radius_max / keep origin-pin degenerate) is undecided until b3
  lands; finite_radius_max is still live in gallery.hpp.
  (from audit/past reports/LADDER.md P-6, eafd9ec)
- HARVEST: the terrain word on the pinned seed — three rounds outstanding, and the last
  open item of PROBATE_I. Wrong looks like "the whole terrain is the same hill repeated"
  or "everything outside the first patch went flat". (from GATEHOUSE_REPORT.md, a5e84bf)
- HARVEST: GF6 — whether to pay for renaming tools/gates/console_gate/ to match what it
  now is; ruled HOLD, the directory rename stays unpaid.
  (from docs/audit/PROBATE_CLOSE.md + GATEHOUSE_REPORT.md, 606924f)
- HARVEST: SPAWN_3 parked — F6 newborns seat inside render radius on move.
  (from audit/past reports/FIELD_BRIDGE.md, 4c1a804)
- HARVEST: the mood-seam crack — neighbours disagree at a shared edge; remediation
  directions recorded but NOT executed, parked per ruling.
  (from audit/past reports/INVESTIGATION_mood_seam.md, eafd9ec)
- HARVEST: TEX_C0 — parked behind the feature wallet and Jean's eye on ASTC banding
  since PROBATE_F. As originally sketched it is **impossible**, not merely expensive;
  priced and shelved with its bill attached, not refused and not scheduled. The texture
  constraint belongs beside docs/FXC_LAWS_RECORD.md.
  (from docs/audit/TEX_C0_PRICE.md, de1d5db)

## Added at CANON C2

- VOICE bus (terrain live-modulation): alias-table design — VOICE_LAYOUT[] shaped
  like PARAM_LAYOUT[], one set_voice(channel, span) door, [VOICE] boot witness
  enumerating every channel with rest + address; sketch in terrain_program_charter
  §C5 (attic since CANON, last at a8f4580); unblocked by the music-coupling campaign.
- SALON Stage B (slot count): STILL OPEN, land-gated. PAINTING_MAX_SLOTS is still 288
  (state.hpp:271) and was last touched by LOOM_1 U3, not by a SALON re-spec; the
  re-spec §0.1/§0.2/§0.3 is unpaid. The witness the stage relies on DOES exist —
  tPipe (renderer.hpp:170). Origin: SALON_1 (attic, 3b931ba).
- SALON Stage D (weld): STILL OPEN, and sharper than the ledger recorded. The named
  slot-filter defect IS fixed — clear_wall_paintings now filters on the sentinel
  patch pair as well as form type (gallery.hpp:2481-2484). What remains: that
  function resets `gs.wall_frame_count = 0` unconditionally (gallery.hpp:2493),
  OUTSIDE the patch-filtered loop, while evict_paintings_for_patch decrements the
  same uint32_t unguarded (gallery.hpp:1423). An outdoor WALL_FRAME slot that
  survives a clear leaves the counter at 0, and the next evict on that patch
  underflows it; the value is consumed as a draw count (render_passes.hpp:490,614).
  Origin: SALON_1 (attic, 3b931ba).
- SALON Stage E (salon hang): STILL OPEN — visual gate, Jean's eye. Prerequisite
  findings §0.5 (vault-crown coupling) and §0.9 (live floor breach) stand.
  Origin: SALON_1 (attic, 3b931ba).
- SALON HELD (packing): STILL OPEN — never built: floor_margin and target_coverage
  both have 0 hits in src/. Note the ledger's cost figure assumed EXHIBITION_LAYERS
  32; the tree now reads 40 (state.hpp:294), so "32 -> 256 = +896 MiB" wants
  recomputing before it is spent. Origin: SALON_1 (attic, 3b931ba).
