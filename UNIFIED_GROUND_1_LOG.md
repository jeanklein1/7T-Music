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

---

## U1 — THE OPENING COMMITS

### [U1a] — e639e87
A2_P1_cleanup.patch verified hunk-by-hunk against this tree
(`git apply --check` clean — identical code base to the probe's), then
applied: 144 + the full trajectories closure (incl. the surviving
@binding(101) declaration, §1.2 primitives, and the zero-use
Idle::TRAJECTORY_FIELD_* pair). glaw1 GREEN.

### [U1b] — 838fe89
The stale six, exhaustive per A2-2c: dead alias notes (71/73) deleted;
census parenthetical RE-COUNTED post-[U1a] — recipe: cc7 census
`declaration_count` (100) + distinct (group,binding) pairs (97); three
fc_ aliases (fc_config/fc_vp/fc_patches) share slots. Shadow texture
banners → "22-23, 28, 34" (4 entries); render texture banners →
"22-23, 25-29, 31-34" (11 entries); render entity group header 19→18.
glaw1 GREEN.

---

## U2 — THE TOPOLOGY

### [U2a] — 3b4d580
Dim::UG_* appended after the patch-mesh cluster; the band static_assert
(UG_QUADS_PER_CELL==4, UG_CAP_BASE==4481, UG_BASE_BASE==10881,
UG_DECODE_VERTS==14977) compiles — glaw1 is the witness. WGSL mirrors
(UG_QUADS/UG_CAP_STRIDE/UG_CAP_BASE/UG_BASE_BASE) beside the skirt
cluster with the C++-room note. ADAPTATION: an additional
UG_CAP_STRIDE_C (=5) constant landed C++-side in [U2b] so the emission
never hardcodes the row stride.

### [U2b] — 226e29f
The LOD-0 emission block replaced per spec: cap quads (i00/i01/i10 ·
i10/i01/i11 — the house winding, verbatim from the legacy grid loop),
curtain quads ((a,b,sa),(b,sb,sa) — the skirt-quad shape, mirrored from
the ring loop), skirt ring kept VERBATIM except a/b re-aimed through
skirt_cap_index (the n=64 walk → cell clamp → cap outer vert; sa/sb
stay the legacy ring copies 4225+k). cell_perimeter reproduces
skirt_grid_index's CW bottom/right/top/left shape at n=4.
ARITHMETIC WITNESS: 256·(16+16)·6 + 4·64·6 = 50,688 emitted exactly;
max cap vert 10,880 and max base vert 14,976 both inside
UG_DECODE_VERTS=14,977. LOD1 block byte-untouched (git diff shows no
hunk there). PATCH_INDEX_COUNT: readers were {definition, the old
reserve} — reserve now Dim-derived; definition retained (documents the
legacy-grid arithmetic; zero runtime readers — noted, not a FINDING).
glaw1 GREEN.
