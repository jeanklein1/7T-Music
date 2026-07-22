# UNIFIED_GROUND_1 — CAMPAIGN LOG

Campaign: UNIFIED_GROUND_1 (campaign v2 Stage 5, designed on AUDIT-2;
handoffs src/docs/HANDOFFS/U 0-6/ u0–u6). Branch: `UNIFIED_GROUND_1`.

---

## U0 — INDEX + PREFLIGHT

### Base

- Cut from `b180e2f9491d042f65800351846446e0ac8b51ea` — the
  GROUND_CARD_1_AUDIT2 tip, Jean's "trunk that suits you better"
  designation resolved to the audit branch: its src/cartridges tree is
  byte-identical to GROUND_CARD_1 HEAD 1240bece, and it carries the U0
  anchor source in-tree (AUDIT2_REPORT.md, A2_P1_cleanup.patch,
  A2_P2_stage5_retirement.patch, the U handoffs, the _post_gc1
  instruments). The exact analogue of the H0 verified-descendant call.
- Containment checks: `live_card_write` present in binding_registry
  (GROUND_CARD_1 [5c] in-tree ✓); AUDIT-2 instruments under audit/ ✓.

### Anchor table

| # | Anchor | Expect | Found | Verdict |
|---|--------|--------|-------|---------|
| a | world.wgsl `const PATCH_GRID_VERT_COUNT` | 1 | 1 | PASS |
| b | world.wgsl `fn patch_skirt_grid` | 1 | 1 | PASS |
| c | world.wgsl `fn zone_gol_mesh_gen` (dies at U4) | 1 | 1 | PASS |
| d | world.wgsl `ZONE_SUPPRESS_INNER` | 1 | definition `const ZONE_SUPPRESS_INNER` = 1 (bare string = 8: the def + the 7 suppression sites A2-4g enumerated — CLASS B: the anchor is the unique definition) | PASS |
| e | state.hpp `LOD-0: full 64×64 mesh (24576 indices)` | 1 | 1 | PASS |
| f | state.hpp `PATCH_MESH_N_LOD1 = 32` | 1 | 1 | PASS |
| g | gol_zones.hpp `HEIGHT_FACTOR_MEAN` | 1 | definition `float HEIGHT_FACTOR_MEAN` = 1 (bare = 2: def + the seed use — same CLASS B reading) | PASS |
| h | drawable_table.hpp `{ "zone",` | 1 | 1 | PASS |
| i | bodies/gallery.hpp `/*zone_active=*/false` (the P2-found fifth DrawBind site) | 1 | 1 | PASS |

All PASS → U1. Amendment register: U5 present in the batch — NOT
vetoed; runs as specified.

### Baseline gate

glaw1 at base b180e2f, before any edit: `G-LAW 1: GREEN`.
