# ROSTER-1a — PHASE R RECON (read-only, for ratification)

the_board only. Branch **MOD_1_ROSTER** off FINAL_TOUCH. Worktree pinned
to **`3aac3db506e49e95a688cab9d5593f8a048cb0ec`** (printed per the
standing rule). No code changed in this phase — this is the recon output
Phase I builds against. Method: verify-first, adversarial census, the
tree wins. Planning-side counts ("twelve families", "featured six") are
hypotheses; the census below rules.

Three gate kinds (from the handoff): **(a)** buffer creation
(state.hpp::init — ROSTER-1b, not cut here; R2 feeds it), **(b)**
dispatch/registration, **(c)** boot/teardown.

---

## R1 — PIECE ENUMERATION (the roster's key list — this phase's OUTPUT)

### Kind FAMILY — the 12 FAMILY_DISPATCH rows (dispatch-gated)

`PopFamily` (spawn_engine.inl:691-704) and `FAMILY_DISPATCH[COUNT]`
(cartridge.hpp:1422-1459). Every family flows through the same select →
place → commit → mesh → draw → evict chain, so its **(b)** gate has one
canonical site (the select loop) plus the per-frame mesh loop, and its
**(c)** teardown is the family's reset in `teardown_world` /
`init_patch_system`.

| # | family | key | (b) dispatch sites | (c) teardown |
|---|---|---|---|---|
| 0 | pyramid | `pyr` | select spawn_engine.inl:1071-1075; place :1093; commit :1110; mesh-prepare/dispatch cartridge.hpp:3266/3274; evict :680 | teardown_world reset of `entities_state_.pyramids`/count/`cpu_pyramids` |
| 1 | arch | `arch` | same select/place/commit/mesh/evict chain | arch array + `arch_count` + pier slots |
| 2 | column | `col` | same | column array + pier slots |
| 3 | antenna | `ant` | same — **shares column mesh** (`prepare_mesh_column`/`mesh_gen_column`, cartridge.hpp:1433-1434) | antenna array (shares column mesh buffer) |
| 4 | palm | `palm` | same | palm array |
| 5 | cactus | `cact` | same | cactus array — **G1 target** |
| 6 | blade | `blad` | same | blade array |
| 7 | sphere | `sph` | same — **shares `floatingEntityBuffer_` with cube** | floater array (shared) |
| 8 | ribbon | `ribn` | select/place/commit are the ribbon dual-entry (also `ribbon_frame_tick` per-frame render:3259; commit also via patch stream) | ribbon reset (transition finite-mode sweep cartridge.hpp:3024-3033) |
| 9 | cube | `cube` | same — shares floater buffer with sphere | floater array (shared) |
| 10 | gol | `gol` | select/place/commit + **bespoke compute** (render:3322-3355, gated `zone_count>0`) | zone reset — **G3 target (residue recipe)** |
| 11 | gallery | `gall` | select/place/commit + `render_snapshot_pass` (render:3431) + `update_photographer` (update:3070, see F-note) | gallery centers/paintings/exhibition reset (teardown 2316-2338) |

**(b) canonical family gate:** the select loop
`for f in COUNT { FAMILY_DISPATCH[f].try_select }` (spawn_engine.inl:1071).
A family gated off there is never selected → never placed → never
committed → count stays 0 → mesh-prepare returns not-dirty → 0 dispatch →
0 draws (draws in render_passes.inl are count-gated). One gate, the whole
chain. **HOT-PATH CAVEAT:** the per-frame mesh loop
(cartridge.hpp:3265-3274) calls `prepare_mesh(f)` for all 12 families
every frame; a disabled family's `prepare_mesh` is a cheap no-op (count
0), but the *call* still happens. Invariant "no runtime branching on
disabled pieces in hot paths" ⇒ Phase I should gate this loop by
constexpr fold (unroll + `if constexpr`), not a runtime `if (ROSTER[f])`.
That is the one genuinely-hot family site; select/place/commit run on the
budgeted stream path, not per-frame-per-entity.

### Kind FEATURE — per-frame ticks + mood-configured subsystems

| feature | (b) dispatch sites | (c) boot / teardown |
|---|---|---|
| **pawn aura** | `tick_pawn_couplings` update:2933 (presence ramp); aura compute render:3357-3406 (gated `aura_presence>0 \|\| aura_needs_clear`) | boot: none (presence starts 0); mood gate `pawn_state_.aura_enabled` mood.inl:669 — **G2 target** |
| **orbs (sky dome)** | `update_orb_anchor` update:3064; `dispatch_orb_init/recolor/copy_prev/dynamics` render:3411-3414; `configure_orbs` on mood mood.inl:675 | boot one-shot `configure_orbs` init_renderer:2779; teardown `teardown_orbs` (2350). **Distinct from SPHERE family** (family 7 = orbital spheres; orbs = the sky dome) |
| **spot lights** (indoor) | `apply_mood_spot_lights` mood.inl:672; `upload_lights` render:3280; shadow-pass topology reads `mood_state_.spot_light_active` (render_passes.inl) | mood-configured (indoor moods only) |
| **indoor shell** (indoor) | `apply_mood_indoor_shell` mood.inl:673 → `generate_indoor_shell` / `clear_indoor_shell` | mood-configured; also drives GALLERY wall paintings (`place_wall_paintings`) |
| **portal** | force-spawn ONLY — `force_spawn_portal_at` mood.inl:886 (choke point), via `force_spawn_back_portal` (cartridge.hpp:3566) + `force_spawn_finite_portals`. **Bypasses FAMILY_DISPATCH — see R3** | transition machine; the 2c edge (transitions REQUIRE portal) |

### Judgment calls (disclosed for ratification — the tree, not the plan)

- **F-note — photographer ⊂ gallery.** `update_photographer` (update:3070)
  and `render_snapshot_pass` (render:3431) are the per-frame ticks that
  fill GALLERY paintings. **Recommendation:** one `gallery` enable bit
  gates both the family dispatch and the photographer tick (they are one
  piece). If Jean wants an independent "photographer off, paintings
  static" mode, split into two bits — flag, not assumed.
- **AGENTS are foundational, not a gateable piece.** The NPC population
  (`spawn_population_for_mood`, `upload_agent_registries`, the agent
  kernel) shares `agentStateBuffer_` across **7 bind groups** (R2), and
  **the pawn IS agent slot 0** — disabling agents breaks possession and
  the pawn itself. **Recommendation:** agents are NOT a roster piece
  (always-on infrastructure); the pawn↔agent-slot-0 dependency is a
  hard edge like portal↔transitions. Listed here so the census is
  complete, not to gate it.
- **Test-rig piers** (`setup_test_rig_piers`, called from `init_renderer`) — a debug
  boot one-shot (3 fixed piers). Minor; a `test_rig` bit could gate it,
  but it is debug scaffolding. **STATUS: LATENT[roster]** candidate;
  ratify whether it earns a bit.
  RETIRED (BOOT_ONE_VOICE C, 4cc629d): setup_test_rig_piers deleted; pier
  slots 0-3 unassigned. The bit was never ratified; the subject died first.
- **NOT pieces (spine infra / shader capability — out of roster scope):**
  patch streaming, frustum cull, placement correction, ground-entry
  packing, the visual-canvas/fog coupling, terrain index gen, the
  boot-neutral/DRIVERLESS writes (initialize:2719-2730 — rider A exempts
  them explicitly). These are the *doors' frame*, not pieces passing
  through. The transition state machine is spine; its dependency on the
  portal piece is the 2c edge.

**Census ruling:** **12 families + 5 gateable features** (pawn aura,
orbs, spot lights, indoor shell, portal) + the 3 judgment-call items
above. The "featured six" hypothesis is close; my census lands 5 clean
features with photographer folded into gallery and agents ruled
foundational — ratify these two calls and the count is fixed.

---

## R2 — BIND-GROUP CENSUS (feeds ROSTER-1b; read-only, no cuts here)

24 bind groups parsed from state.hpp. Mesh **vertex/index** buffers ride
`SetVertexBuffer`/`SetIndexBuffer` (29 sites in renderer.hpp) — outside
bind groups, so each family's geometry is independently droppable. The
constraint for ROSTER-1b's gate (a) is the **shared storage bind groups**.

### Cleanly separable — piece owns its own bind group(s)

| piece | buffer(s) | bind group(s) — no co-resident pieces |
|---|---|---|
| pawn aura | `pawnAuraCellsBuffer_`, `pawnAuraConfigBuffer_` | Pawn Aura Compute |
| orbs | `orbStateBuffer_`, `orbStatePrevBuffer_`, `orbConfigBuffer_` | Orb Compute, Orb Copy (+ read-only in Render Entity / Photographer) |
| per-family mesh gen | `{pyramid,arch,column,palm,cactus,blade}MeshParamsBuffer_` | one *Mesh Gen BindGroup* each (column shared by antenna) |
| gol mesh | `zoneMesh{Vertex,Index,Indirect}Buffer_`, `zoneDeriveRequestBuffer_` | GoL Zone Compute (self) |

### Shared / co-resident — buffer cannot be dropped without re-laying-out the group

| buffer | pieces it serves | # groups | co-resident groups |
|---|---|---|---|
| `agentStateBuffer_` | agents (+pawn) | **7** | Compute Entity, Render Entity, Photographer ×2, Entity Placement, Frustum Cull, Pawn Aura |
| `tileGridBuffer_` | terrain (all) | 6 | Compute Entity, Render Entity, Patch Gen, Ribbon Compute, Photographer, GoL Zone |
| `patchInstancesBuffer_` | terrain (all) | 6 | Render Entity, Photographer ×2, Entity Placement, Frustum Cull, GoL Zone |
| `pierBuffer_` | arch+column families (contributors) | 4 | Compute Entity, Patch Gen, Ribbon Compute, GoL Zone |
| `zoneConfigBuffer_` | gol | 4 | Compute Entity, **Render Texture**, Entity Placement, GoL Zone |
| `orbConfig/StateBuffer_` | orbs | 4 | Orb Compute, Orb Copy, Render Entity, Photographer |
| `floatingEntityBuffer_` | sphere **+** cube (shared!) | 3 | Compute Entity, Render Entity, Photographer |
| `ribbonBuffer_`/`ringTransformsBuffer_` | ribbon | 3 | Render Entity, Ribbon Compute, Photographer |
| `zoneLifeBuffer_` | gol | 3 | Compute Entity, Entity Placement, GoL Zone |
| `paintingSlotsBuffer_` | gallery | 2 | Compute Entity, Entity Placement |
| `pyramidInstancesBuffer_` | pyramid | 3 | Compute Entity, Patch Gen, GoL Zone |
| `spotLightArrayBuffer_` | spot lights | 2 | Render Entity, Photographer |
| `{pyramid,arch,column,plant}GroundBuffer_` | placement Y | 1 each | Entity Placement (co-resident with each other) |

**The two megabinds** — `Compute Entity BindGroup` (the update_world
kernel: agents, floaters, zoneLife, pyramidInstances, piers,
paintingSlots, tileGrid, agentBehaviors) and `Entity Placement Compute
BindGroup` (pyramid/arch/column/plant ground, zoneLife, paintingSlots,
agents, patchInstances) — co-reside nearly every piece. **ROSTER-1b
implication:** gate (a) buffer *non-creation* is clean for pawn-aura,
orbs, and per-family mesh params; but any piece feeding a megabind
(gol, sphere/cube, gallery, pyramid) can at most be left *pristine*
(rider A), not un-created, without re-sectioning the shared group.
`sphere` and `cube` share one floater buffer — they cannot be
independently *un-created*. This is precisely ROSTER-1b's design input.

---

## R3 — THE PORTAL DOOR (verdict + sites)

**VERDICT: NO — portals do NOT pass through gate (b)'s dispatch path.**

`force_spawn_portal_at` (mood.inl:886) writes the arch instance
**directly** — `entities_state_.arches[slot]` (mood.inl ~940), `arch_count++`,
`arch_mesh_gen_pending = true`, and `write_pier` ×2 for the portal feet —
with **no** `FAMILY_DISPATCH[ARCH].try_select/try_place/try_commit`. It is
a force-spawn that bypasses the family select loop entirely. Therefore a
disabled ARCH family (gated at the select loop) would still let portals
appear, and a disabled *portal* has no gate at all today.

**All portal spawns route through one choke point** — `force_spawn_portal_at`
— reached via:
- `force_spawn_back_portal` (cartridge.hpp:3566, inside stream_patches'
  first-frame regen, gated `mood_state_.back_portal_pending`) → calls
  `force_spawn_portal_at` (mood.inl:1102/1131) and `force_spawn_finite_portals`.
- `force_spawn_finite_portals` (mood.inl:1153) → `force_spawn_portal_at`
  (mood.inl:1237) per perimeter spot.

**THE SECOND DOOR (pre-authorized deviation — implemented in Phase I).**
Because the verdict is NO, ROSTER-1a implements a roster consult on the
portal piece. **Recommended single site:** the choke point
`force_spawn_portal_at` (mood.inl:886) — a disabled portal early-returns
(spawns nothing: no arch, no piers, no mesh-pending), which catches back,
finite, and any future portal spawner from one place. Tagged `ROSTER-GATE`
(piece: portal, kind: b). (It cannot land now — it needs the manifest bit,
which is Phase I 2a.)

**THE FIRST EDGE (2c): transitions REQUIRE portal.** Portals are *both*
the transition trigger (stepping through an `is_portal` arch fires the
FADE_OUT, render:3204-3216) *and* the guaranteed return path in finite
worlds (`force_spawn_back_portal`). Disable portal with transitions live
⇒ no trigger in and no way back ⇒ soft-lock. This is a **forbidden
configuration**, never a silent degrade. v0 mechanism: since the
transition machine is unconditionally enabled today (no `transitions`
bit), the edge is `static_assert(ROSTER.portal, "portal disabled while
transitions enabled")` in the manifest, adjacent-commented as the
resolver's first dependency edge (M-j embryo). If Jean prefers a
`transitions` bit, the assert becomes conditional — flag for ratification.

---

## PHASE I PREVIEW (what lands after ratification — not done here)

Per the handoff's commit shape: manifest (2a, spine-owned constexpr table
+ banner list + maturity-dial comment) → gates (2b, select-loop + hot mesh
loop by constexpr fold; boot/teardown one-shots) → the portal second door
+ the 2c edge (2c) → the residue recipe for gol (2e). Rider A: DISABLED =
zero GPU writes, not merely zero draws (buffers may exist but stay
pristine; non-creation is ROSTER-1b). All consults carry the literal
`ROSTER-GATE` sentinel (piece + gate kind) for the grep census.

**Awaiting ratification of:** (1) the piece key list above — especially
the two judgment calls (photographer⊂gallery; agents foundational); (2)
the portal second-door site (`force_spawn_portal_at`); (3) the 2c edge
mechanism (unconditional `static_assert(ROSTER.portal)` vs a `transitions`
bit). Nothing is cut until these land.

---

## LEDGER APPENDIX (dispositions — recorded per the method: silence is the
one deviation form the method forbids)

- **terrain tokens** — judged **substrate** (generation composition), not a
  piece; gating deferred to **M-m**. (`tick_terrain_tokens` /
  `terrainTokens_` compose per-tile amplitude momentum during generation;
  they shape the ground the doors gate, they are not a door.) Disclosed
  here rather than left as a silent exclusion.
- **test-rig piers** — **TESTING** class, NO roster bit (roster rows are
  design pieces, not scaffolds). Mortal retirement: dies at ship
  (checklist). Tagged in-code `TESTING[test-rig-piers]` at
  `setup_test_rig_piers`; added to Constitution §5 TESTING (count 1→2);
  joins the future exhibition-guard discussion with
  `SEAM[spawn_engine:L1]` DIAG_ENTITY_LIFECYCLE.
  RETIRED (BOOT_ONE_VOICE C, 4cc629d): setup_test_rig_piers deleted; pier
  slots 0-3 unassigned. The mortal retirement was honoured — it died at
  ship, as the checklist said (count 2→1 at C).

## PHASE I — LANDED (ratified go-order)

Manifest `Roster ROSTER` (v0, all-enabled) added to the spine banner's
owned list. 12 family bits + 7 feature bits (pawn_aura, orbs, spot_lights,
indoor_shell, portal, transitions, wanderers) + a FOUNDATIONAL note
(agents machinery: pawn is slot 0, buffer co-resides 7 bind groups).
Gates carry the `ROSTER-GATE <piece> (<b|c>)` sentinel; the gol residue
check carries `ROSTER-RESIDUE gol`. The 2c edge is
`static_assert(!ROSTER.transitions || ROSTER.portal, …)` (conditional, so
the lean build stays legal) plus the maturity-proof early-return doors in
`request_mood_transition` and `force_spawn_portal_at`. spot_lights ×
indoor_shell verified INDEPENDENT (§5). See the Phase-I close-out in the
commit body / chat report.
