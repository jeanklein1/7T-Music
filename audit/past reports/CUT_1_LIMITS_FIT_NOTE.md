# CUT_1 / C6 — THE 8-FIT NOTE (held branch; awaiting Jean's design)

**Status:** C6 activated. After CUT_1d (field CPU dialect) and CUT_1e (camera
host), the room-family compute pipelines still carry **9 storage-buffer layout
entries visible to the compute stage** — one over the WebGPU core default
`maxStorageBuffersPerShaderStage = 8`. Per the land-gate: no layout edit is
designed or executed here. This note is the C5 report, placed for the design
chat.

## The count, exactly (post-CUT_1e tree)

Room-family pipelines (`update_player_agent`, `update_other_agents`,
`update_sphere`, `update_cube` — renderer.hpp, roomComputeLayout stack):

| Group | Binding | Buffer | Type | Cuttable? |
|---|---|---|---|---|
| g0 | vp_data | vpBuffer_ | Storage | no — viewpoint set |
| g0 | agent_state | agentStateBuffer_ | Storage | no — the agents |
| g0 | camera_state | cameraBuffer_ | Storage (read_write in WGSL) | **no** — GPU camera integrator; azimuth feeds pawn velocity in every host mode (world.wgsl 6947/6976/6988), VP build reads it (8688-8690). Camera HOST died in CUT_1e but the integrator is host-agnostic. Deleting = L7 re-section + WGSL surgery. |
| g0 | floating_entities | floatingEntityBuffer_ | Storage | no — floaters |
| g0 | patch_grid | patchGridBuffer_ | ReadOnlyStorage | no — terrain adoption |
| g2 | occupier_cmg | (window) | ReadOnlyStorage | demotion candidate |
| g2 | occupier_amg | (window) | ReadOnlyStorage | demotion candidate |
| g2 | field_head_poses | headPosesBuffer_ window | ReadOnlyStorage | demotion candidate |
| g2 | field_forces | fieldForcesBuffer_ | Storage (read_write) | no — the field's one output; agents/spheres/cubes consume (world.wgsl 8035/8251/8586) |

The GPU field dialect is fully live post-CUT_1d (R2): `field_head_poses` /
`field_forces` WGSL decls at world.wgsl:2506-2507, writer `write_field_forces`
:7934, three consumer bands.

## Candidate single minimal edits (for the design chat, not executed)

1. **Demote one g2 read-only window to Uniform** on the tree's own precedent
   (state.hpp:4386-4398 — tier-gains + figure-profiles were demoted to duck
   the render-stage cap). Constraints to check in design: WGSL address-space
   rewrite for the chosen binding (`var<storage, read>` → `var<uniform>`),
   16-byte array-stride law (already vec4-shaped for field_head_poses:
   `array<vec4<f32>, 400>` = 6,400 B — under the 64 KiB uniform cap),
   occupier windows' sizes and dynamic-ness, and L6 (registry is the one
   home of the numbers; L7 binding closure re-proved at boot).
2. **Merge occupier_cmg + occupier_amg into one buffer** with two windows →
   drops one entry; heavier surgery (upload paths + registry + WGSL).

Uniform-count headroom exists either way: room family sees 7 uniforms of 12.

## Also standing (unchanged by CUT_1, assigned elsewhere)

- Exceedance 2: `maxTextureArrayLayers` 289 > 256 (patch arrays,
  state.hpp:4110/4129; `MAX_ACTIVE_PATCHES = 17²`) — assigned to OPT_1 under
  Jean's visual gate (16² = 256 sits exactly at the cap).
- Near-limits (ledger H5): VS and FS storage 7/8 in the main render family;
  three (16,16) workgroups exactly at 256; FloatingEntityArray uniform binding
  54,912 B = 83.8% of 64 KiB.
