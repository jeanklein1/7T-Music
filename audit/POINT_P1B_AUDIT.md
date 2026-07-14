# PANEL-0 p1b — THE PAWN/POINT DISENTANGLEMENT AUDIT (read-only; ONE STOP)

Read-only campaign product. Nothing moved, nothing cut. Jean's question:
what must the PAWN own by definition (the body), and what should the POINT
own instead (the viewpoint/awareness primitive that lives in whichever
host)? The trigger: patch generation centers on the pawn, so in camera-host
the world stops streaming — one of a whole class of pawn-position couplings
that need classifying. This audit enumerates EVERY consumer of the pawn's
position (CPU + GPU), classifies each VIEWPOINT / BODY / AMBIGUOUS, names the
mechanism, and sizes p1b. FULL STOP for the stamp.

METHOD. Two parallel censuses read every pawn-position consumer: (a) the
GPU — every `compute_pawn_pos()` / `render_pawn_pos()` / `config.lod_pawn_*`
read across world.wgsl (~11.5k lines); (b) the CPU — every `readback_x/z`,
`lod_pawn`, `slots[possessed_slot].pos` read across cartridge.hpp, the
surface, the bodies. The spine's own reads grounded the streaming center,
the witness harvest, and the readback infrastructure. Every claim carries a
file:line. v3 §9 (the driver law / anchor) and §11 (the witness / bubble)
are the lens.

---

## 0 — THE DEFINITIONAL ANSWER (what the audit reveals)

**The pawn's position does DOUBLE DUTY today** — it is simultaneously the
BODY's location and the VIEWPOINT's location — because the pawn was the only
host the point ever had. The whole entanglement is that one conflation. The
audit sorts it cleanly:

**THE PAWN owns, by definition (the body) — position = the possessed agent
slot:**
- its own placement + terrain-snap (the walk), its draw, its
  orientation/tilt;
- its ENTITY-EMANATING fields — the aura (a terrain deformation dome
  centered on the body), the sphere/cube forcefield (radius grows with body
  speed);
- its role as the AI REFERENCE FRAME — NPCs pursue/flee/cluster the body,
  floaters and cubes are leashed to it;
- its role as the POSSESSION TARGET and the body that steps through a
  portal; the photographer's photographic SUBJECT.

**THE POINT owns (the viewpoint/awareness) — position = host-sourced:**
- the world position the CAMERA renders from;
- the STREAMING / generation center, the LOD/cull center, the visibility
  window, the recenter cursor;
- the SHADOW-VP center;
- the BUBBLE — the bounded awareness region (proximity / portal-sensing /
  the coming event source, v3 §11).

The luck of the existing design: the CPU **already** keeps these two
positions in two different places — `player_.readback_x/z` (the de-facto
VIEWPOINT position: what streaming centers on) versus
`slots[possessed_slot].pos` (the BODY reference frame: what possession /
corral / respawn read). Today they hold the same value (both the pawn); the
disentanglement makes `readback_x/z` follow the POINT and leaves
`slots[].pos` the BODY. The split is half-built already.

---

## 1 — THE SOURCE: ONE lever re-tracks the whole CPU side

`cartridge.hpp:835-837` — the witness HARVEST, the SOLE AUTHOR of
`readback_x/z`:
```cpp
const auto& p = agent_state_.slots[player_.possessed_slot];
player_.readback_x = p.pos_x;
player_.readback_z = p.pos_z;
```
It reads the possessed slot — the pawn body — and does NOT consult
`config.point_host`. In camera-host the pawn idles, so `readback_x/z`
freezes, and **every CPU viewpoint consumer downstream freezes with it.**

**Fixing this ONE site — host-routing the harvest — re-tracks the entire CPU
viewpoint set** (streaming center, recenter, LOD banding, `lod_pawn` stage,
entity draw-cull, orb anchor). They all read `readback_x/z`; none needs its
own edit. That is the audit's central simplification.

The catch the witness law predicts: in camera-host the point = the CAMERA,
whose position is **GPU-resident and never read back to CPU** (confirmed —
only `agent_state` + `floating_entity` are read back, `cartridge.hpp:1064-
1074`; there is no camera staging). So host-routing the harvest needs a
**camera-position readback** — the one genuinely new piece of machinery.

---

## 2 — CLASSIFICATION: CPU consumers

### VIEWPOINT (must follow the point — all break in free-fly today)
All read `readback_x/z` (or the `lod_pawn` staged from it):

| site | purpose |
|---|---|
| `patch_system.hpp:563-564` | **the streaming grid center** (Jean's terrain freeze) |
| `patch_system.hpp:573 / 578-579` | recenter trigger + persisted cursor (`last_center_x/z`) |
| `patch_system.hpp:630/640/655/756/769` | `collect_sorted_patches` spawn/gen/evict priority origins |
| `patch_system.hpp:679-683` | allocation ring center + distance-sort origin |
| `patch_system.hpp:790-811` | CPU LOD0/LOD1/pregen + visibility-cylinder banding |
| `patch_system.hpp:839-840` | `stage_lod_pawn` → SET of GPU `config.lod_pawn_x/z` |
| `spawn_engine.hpp:378-442` | entity draw-visibility hysteresis cull (arch/column/antenna) |
| `cartridge.hpp:835-837` | THE SOURCE (harvest) — re-route here |

`tile_world.hpp` has **no** direct read — it receives `last_center_x/z` as
parameters (`patch_system.hpp:517/584/886`), so it inherits the viewpoint
center for free.

### BODY (stays with the pawn — untouched)
- Pawn aura: `pawn.hpp` operates on the scalar `aura_presence`; its POSITION
  is GPU-resolved via `compute_pawn_pos()`, not the CPU readback. BODY.
- Gallery photographer: `gallery.hpp:583-618` accumulates the pawn's WALK
  distance to trigger snapshots — it documents the body's journey; in
  free-fly the body idles so the photographer correctly idles. BODY.
- Ribbon spawn orientation: `ribbon.hpp:1145-1146` orients "away" from the
  body at spawn. BODY (cosmetic).

### AMBIGUOUS — for your ruling (§4)
Orb dome anchor (`cartridge.hpp:786`, `orbs.hpp:782-786`); portal/bubble
trigger (`cartridge.hpp:882-895`); NPC respawn clustering
(`agents.hpp:470-472`); possession radius (`agents.hpp:510-524`); cube
corral/kite (`cube_behaviors.hpp:280-281/377-388`); ribbon ride selection
(`ribbon.hpp:934-935`).

---

## 3 — CLASSIFICATION: GPU consumers (world.wgsl)

The GPU side is even more contained. Only **two** reads must migrate; a
`point_pos()` accessor (`= point_camera_hosted() ? camera_state.pos :
compute_pawn_pos()`) serves them:

### VIEWPOINT — migrate
| site | purpose | note |
|---|---|---|
| `world.wgsl:6768` | **shadow-VP** (`coupling_pawn_to_sun_vp(compute_pawn_pos())`) | the 300-unit ortho shadow box must cover what the eye sees; off-pawn in free-fly it un-shadows the view |
| `world.wgsl:8216-8217` | **frustum-cull LOD0 center** (`config.lod_pawn_x/z`) | but this is CPU-staged (§2) — it follows the CPU harvest automatically; **no GPU edit needed if the CPU stage becomes the point** |

So on the GPU, the shadow-VP is the **only** hand-edit; the cull center
rides the CPU `lod_pawn` change. The camera aim (`world.wgsl:6359`) is
**already** point-correct — the camera-hosted branch returns before it
(`world.wgsl:6335`), so the pawn read runs only when the pawn hosts.

### BODY — do not move (confirmed, every site)
Aura (`sample_pawn_aura` — `world.wgsl:2581/3620/3732/7522/7572/7629`),
forcefield (`zone_pawn_ff` — `3710/7559`, radius grows with body speed), GoL
proximity suppression (`7603`), floater/cube leashing
(`6412/6457/6584/6669`), and every `possessed_slot` IDENTITY read (the six
accessors + the walker/NPC AI reference frames — `6234/6261/6288/5795/5964/
6007/6072`). All entity-emanating or AI-reference; all stay.

### AMBIGUOUS — for your ruling
`compute_photographer_vp` (`world.wgsl:7921/7967`) — it IS a camera, but its
subject is the pawn body ("Build VP looking at pawn"); lean BODY (photograph
the character), but it's your call.

---

## 4 — THE AMBIGUOUS SET (the rulings owed)

Each is a genuine "does this serve the viewpoint or the body?" that only you
can settle. My leanings, for the stamp:

| item | lean | reasoning |
|---|---|---|
| Portal / bubble trigger (`cartridge.hpp:882`) | **POINT** (but deferred within p1b) | v3 §11 makes the bubble the point's; but p1a ruled the bubble's sensors DORMANT in free-fly, so the trigger stays pawn-realized until the bubble machinery moves. The audit's bubble-realization sub-movement. |
| Orb dome anchor | **POINT** | the sky dome centers on where the player IS — the viewpoint in free-fly reads more natural than a dome stranded on the idle pawn |
| NPC respawn clustering | **BODY** | NPCs are the body's neighbors (they pursue/flee the body); clustering on a free-flying viewpoint would spawn crowds under an empty camera |
| Possession radius | **BODY** | possession re-anchors to a body near THE BODY; in free-fly there's arguably nothing to possess-from |
| Cube corral / kite | **BODY** | cubes are leashed to the body by design |
| Ribbon ride selection | **BODY** | riding is a host migration onto the body's neighborhood |
| Photographer VP | **BODY** | its subject is the character's journey |

Pattern in the leanings: **the viewpoint owns terrain existence and framing
(streaming, LOD, shadow, dome); the body owns the living world around it
(NPCs, floaters, possession, the photograph).** If that rule reads right,
the ambiguous set resolves as above and only two truly move to the point
(orb dome + the deferred bubble/portal).

---

## 5 — THE MECHANISM (sized)

**The point gains a WORLD POSITION**, host-sourced, available on both sides:

- **GPU:** `point_pos()` — `point_camera_hosted() ? camera_state.pos :
  compute_pawn_pos()`. Repoint the shadow-VP (one read). The cull follows
  the CPU `lod_pawn`. Trivial.
- **CPU:** a **point-position readback** — the new machinery. In pawn-host
  the point = the possessed slot, so the harvest reads it exactly as today
  (**pixel-identical**). In camera-host the point = the camera, so a small
  readback of `camera_state.pos.xz` (a 2-float staging buffer, the same
  pattern as the agent readback) fills `readback_x/z`. The harvest becomes:
  `readback_x/z = point_camera_hosted() ? camera_readback : slots[possessed].pos`.

**Two realization options for the point readback** (a stamp micro-choice):
- **(A) Camera-state readback** — read back `camera_state.pos` only in
  camera-host; pawn-host keeps the agent readback untouched. Most surgical;
  pawn-host provably pixel-identical (its path is byte-unchanged).
- **(B) A dedicated point-position buffer** the GPU fills with `point_pos()`
  each frame; the CPU reads it in both hosts. Unifies the two hosts through
  one path; slightly more faithful to "the point has a position," but it
  routes pawn-host through a new buffer (pixel-identity then rests on the
  value matching, not the path being byte-identical).

Recommendation: **(A)** — the pixel gate on pawn-host is binding and (A)
leaves the pawn-host CPU path byte-untouched.

**THE WITNESS-CONTRACT / CENSUS-W IMPACT.** `readback_x/z` becomes the
POINT's position (host-authored), while `readback_portal_trigger` stays the
BODY's (the pawn's portal crossing) until the bubble moves — the readback
trio SPLITS along the pawn/point line. `slots[possessed_slot].pos` stays the
BODY reference frame. The score census Direction W (readback trio
sole-author) must be updated to reflect the point as `readback_x/z`'s
author; the possession/aura guards are unchanged.

---

## 6 — SIZING & RISK

- **CPU:** one re-routed harvest source (`cartridge.hpp:835`) + one small
  camera-position readback (buffer + staging + the MapAsync harvest, the
  agent-readback pattern). Everything downstream follows. SMALL–MEDIUM.
- **GPU:** one `point_pos()` accessor + one repointed read (shadow-VP). The
  cull rides the CPU change. SMALL.
- **The pixel gate (binding):** pawn-host must stay pixel-identical — both
  the streaming (reads the slot as today) and the shadow (point_pos = pawn
  slot when pawn-hosts). Option (A) keeps the pawn-host paths byte-untouched
  → identity by construction.
- **The anti-flicker dependency (the one real hazard):** `lod_pawn` is
  CPU-banded on purpose so the CPU banding and the GPU cull "partition with
  the same yardstick" (`state.hpp:379-389`, `world.wgsl:8209-8215`). The CPU
  banding anchor and the GPU cull center must become the point IN LOCK-STEP
  — since the cull reads the CPU-staged `lod_pawn`, routing the harvest to
  the point moves both together, so the yardstick stays shared. Changing
  only one side would reintroduce boundary flicker. Keep them coupled.
- **Free-fly latency:** the point readback is 1–2 frames latent (the agent
  readback pattern). Streaming already tolerates exactly this lag on the
  pawn today, so free-fly streaming inherits the same acceptable lag.

---

## STOP — THE STAMP REQUEST

The audit returns: the definitional split (§0 — pawn = body, point =
viewpoint, the doubled position); the CPU + GPU classification (§2/§3 — the
viewpoint set is one re-routed harvest source + one GPU shadow read, the
body set is untouched); the mechanism (§5 — the point-position readback,
option A recommended, pawn-host pixel-identical); the witness/census-W
impact; the sizing (§6). Open for the stamp:

1. **The ambiguous rulings (§4)** — is "viewpoint owns terrain
   existence/framing; body owns the living world around it" the rule? If so
   only the orb dome (and the deferred bubble/portal) move; confirm or
   re-rule each row.
2. **The readback realization** — option (A) camera-state readback
   (recommended, pixel-safe) or (B) a unified point-position buffer?
3. **The bubble/portal** — move the portal trigger to the point's bubble in
   p1b, or keep it a further-deferred sub-movement (per p1a's dormant-bubble
   ruling)?
4. **The photographer** — stays body-anchored (photograph the character), or
   follows the point?

Nothing cut. p1b cuts only after the stamp; the pawn-host pixel gate is
Jean's rig, held binding.
