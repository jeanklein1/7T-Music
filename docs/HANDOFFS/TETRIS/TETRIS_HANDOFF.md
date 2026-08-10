# TETRIS — GPU resource reallocation, round one

Campaign arc: **T-OPEN → WALLET_0 → WALLET_1 → ORB_V → ARENA_0 → T-CLOSE.**
Authored by Claude against BINDING_LEDGER.md at source commit `1fee4c7`
(BUDGET_1-8b). Jean has ruled the doctrine; CC executes. Music-era and
painting-era surfaces are OUT OF SCOPE — nothing in this arc touches
`painting_slots`, `photo_painting_slots`, `painting_array`, or any
signal/coupling mechanism.

Purpose, in one line each:
- WALLET_0 converts the only full budget row (room family, storage 8/8)
  to 6/8 by demoting the two occupier windows to uniform.
- WALLET_1 merges the three fragment-stage light buffers into one
  uniform `Lighting` block: entity-family F storage 7/8 → 4/8. This
  also creates the one home the future fog-anchor work will extend —
  it adds NO fog fields itself.
- ORB_V moves `render_orb_state` to an instance-step vertex buffer:
  render-family V storage 7/8 → 6/8, and retires a registry site.
- ARENA_0 is a READ-ONLY recon of the five mesh-gen families for a
  future shared-arena ruling. Zero source edits.

## Standing orders (all campaigns)

1. **Start from the newest master.** `git checkout master && git pull
   --ff-only origin master`. Record HEAD sha at the top of every
   report. All census output and all edits are against that HEAD.
2. **Stop-on-mismatch.** Any census predicate that fails, any grep
   count that differs from the expectation stated here, any symbol
   named here that does not exist at HEAD: STOP, publish what was
   found, make no edit. The named authority-bearing surprises are
   listed per campaign; on those, STOP even if a "fix" looks obvious.
3. **One commit per numbered unit**, pushed directly to master (git
   law). Commit messages carry the campaign tag exactly as given.
4. **naga gate.** After every commit that touches `world.wgsl`, run
   naga validation on it before pushing. A naga pass is not an FXC
   pass (world.wgsl banner law): every shader-touching commit's report
   ends with "awaiting glaw1 witness". Jean runs glaw1 + visual gate
   on his side; the campaign is not DONE until he says so.
5. **L1 encoding.** Every touched file: LF-only, no BOM, exactly one
   trailing newline. Verify by byte read-back after write.
6. **Comments describe present behavior only.** Any comment in a
   touched file that asserts a count falsified by the edit is updated
   in the same commit. Cite symbols and the ledger, never line
   numbers.
7. **Defended sites (ledger Table H).** Before editing inside or
   adjacent to `occupier_contact`, `calc_spot_light`,
   `sample_spot_shadow_pcf`, the `world.wgsl` file banner, or the
   Render Entity Layout caption region in `state.hpp`: read the
   attached prose first. Preserve its content; update only numbers and
   symbol spellings the edit falsifies.
8. **L6.** Binding numbers change only in `binding_registry.hpp`;
   `world.wgsl` `@binding` literals mirror it.
9. **Census before blocks.** Each campaign's Phase C publishes the
   verbatim current text of every edit site. Derive your FIND text
   from that census output mechanically; the transformations here are
   exact and complete. Do not improvise beyond them.

## Ledger-derived expectations used below

From BINDING_LEDGER.md at `1fee4c7` (re-verified at T-OPEN):

| fact | value |
|---|---|
| roomLayout_ entries | 6 (bindings g2: 0,1,2,3,4,5) |
| room family (4 kernels) per-stage | storage 8/12→ 8 of 8, uniform 8 of 12 |
| renderEntityBindGroupLayout_ entries | 17 |
| render family V / F storage | 7 of 8 / 7 of 8 |
| render family F uniform | 3 of 12 |
| scalar access sites: `occupier_cmg` / `occupier_amg` | 2 / 2 |
| access sites: `render_light` / `render_point_lights` / `render_spot_lights` | 5 / 2 / 4 |
| `render_orb_state` read sites (orb_vs only) | 1 |
| bytes: `occupier_cmg` 4096, `occupier_amg` 1280, light trio 48+272+528 | all ≤ 65536, uniform-legal per Table C |
| program vertex buffers / attributes | 1 of 8 / 4 of 16 |

---

## T-OPEN — census anchor

**Commit 1 — `TETRIS T-OPEN: regenerate BINDING_LEDGER at campaign head`**

1. Pull master per standing order 1; record HEAD sha.
2. Locate the committed ledger: `git ls-files | grep -i binding_ledger`.
3. Run `tools/binding_ledger.py` the way its own header or `--help`
   documents. All witnesses it asserts must PASS. Its G2-eol read-back
   is part of the run.
4. Commit the regenerated artifact only. No source edits.

**Report — the GROVE question.** Publish the regenerated Room Layout
(Table A rows for `roomLayout_`) verbatim, and state which branch
holds:

- **Branch A** — roomLayout_ has 6 entries, bindings 0–5, matching
  `1fee4c7`. Proceed; WALLET_0 predictions: room storage 6/8, uniform
  10/12.
- **Branch B** — roomLayout_ has 8 entries: 0–5 as above PLUS palm and
  cactus mesh-param windows at bindings 6/7 in the UNIFORM address
  space (a GROVE_0 landing after the census). Proceed; WALLET_0
  predictions become room storage 6/8, uniform **12/12** — at cap,
  zero margin. Legal; the fourth-group charter absorbs future eras.
  State the 12/12 plainly in the report.
- **Any other shape** (extra storage entries, different bindings,
  missing occupier constants): STOP. Authority-bearing.

---

## WALLET_0 — occupier windows ride uniform

**Commit 2 — `WALLET_0: occupier windows ride uniform; room family storage 8->6`**

### Phase C — site census (publish verbatim, verify, then edit)

| # | file | site | predicate |
|---|---|---|---|
| C1 | world.wgsl | declaration at `@group(2) @binding(0)`, symbol `occupier_cmg` | `var<storage, read>`, type `array<ColumnMeshParams, 32>` |
| C2 | world.wgsl | declaration at `@group(2) @binding(1)`, symbol `occupier_amg` | `var<storage, read>`, type `array<ArchMeshParams, 16>` |
| C3 | world.wgsl | all access sites of both symbols | exactly 2 + 2 read sites, inside the closure of the four room kernels (`occupier_contact` region — defended, read its banner) |
| C4 | state.hpp | roomLayout_ entries whose initializers name `bind::g2::occupier_cmg` and `bind::g2::occupier_amg` | buffer type token `ReadOnlyStorage`, visibility Compute, no dynamic offset |
| C5 | state.hpp / renderer.hpp | the room BIND GROUP creation: which buffer objects are bound at g2 bindings 0 and 1, and at what offsets | offsets 0, or 256-multiples; publish the buffer member names |
| C6 | state.hpp | the CREATION descriptors of the buffer(s) found in C5 | publish current `wgpu::BufferUsage` flags |
| C7 | world.wgsl banner + state.hpp | every comment asserting "8/8" for the room family, and the comment containing the phrase "10-per-stage" (or "past the 10") | publish each sentence |

STOP conditions: C3 counts differ; C5 shows non-256-multiple offsets;
either binding constant missing.

### Phase E — edits

- E1 (world.wgsl): C1 and C2 declarations: `var<storage, read>` →
  `var<uniform>`. Types unchanged — Table C verdicts are CANDIDATE,
  element strides 128 and 80, both 16-multiples, "none needed".
  Access sites unchanged (W1-0: no `ptr<…>` anywhere in the module,
  so no pointer plumbing exists to break).
- E2 (state.hpp): the two layout entries from C4:
  `wgpu::BufferBindingType::ReadOnlyStorage` →
  `wgpu::BufferBindingType::Uniform`. One token per entry.
- E3 (state.hpp): the buffer creation(s) from C6: add
  `| wgpu::BufferUsage::Uniform` if not already present.
- E4 (comments, same commit): the banner's room-family line updates to
  the post-edit truth — the family sits at 6/8 storage, occupier
  windows ride uniform, demotion record: BINDING_LEDGER Table C. The
  "10-per-stage" number corrects to the Core default **8** (ledger
  finding 4); the decision the comment defends is untouched. Any C7
  sentence asserting 8/8 updates to 6/8.
- E5: naga; L1 read-back; commit; push.

### Gate row

| fixes | could break | artifact the reader would see | witness | revert |
|---|---|---|---|---|
| Reopens 2 storage seats in the only full row; future occupier subscribers land without demolition; retires the banner's standing debt ("no new storage binding without a demotion plan") | FXC codegen for the four room kernels shifts (cbuffer vs structured-buffer paths through `occupier_contact`) | Boot fails at pipeline creation on D3D12, or possessed-pawn soft collision against columns/arches misbehaves | glaw1 + boot; then visual gate on a collision-active scene | this one commit |

---

## WALLET_1 — the lighting block

**Commit 3 — `WALLET_1: lighting block; entity F storage 7->4`**

GROWTH LAW applies: the C++ and WGSL structs land in this one commit,
members in the same order with the same types, with a sizeof witness.

### Phase C — site census

| # | file | site | predicate |
|---|---|---|---|
| C1 | world.wgsl | struct definitions `DirectionalLight`, `PointLightArray`, `SpotLightArray` | publish full field lists; sizes 48 / 272 / 528, aligns 16 (Appendix 1) |
| C2 | world.wgsl | declarations at `@group(0) @binding(320/321/322)`: `render_light`, `render_point_lights`, `render_spot_lights` | all `var<storage, read>`, F-only reach |
| C3 | world.wgsl | all access sites of the three symbols | exactly 5 + 2 + 4, all within the `entity_fs` closure; the spot path crosses `calc_spot_light` and `sample_spot_shadow_pcf` — both defended, read their banners before touching anything nearby |
| C4 | binding_registry.hpp | constants for 320/321/322 | publish names and neighborhood comments |
| C5 | state.hpp | renderEntityBindGroupLayout_ entries naming those three constants; the layout's `std::array<…, 17>` declaration and any entry-count caption | 17 entries; the caption region is defended (Table H) — read it |
| C6 | state.hpp | the render entity BIND GROUP entries for the three bindings, and the three C++ buffer members + their creation descriptors | publish member names, sizes, usages |
| C7 | tree-wide | every CPU write site of those three buffers (grep the member names from C6 against `WriteBuffer`/`writeBuffer` and staging paths) | publish each site; note whether all three writes sit in one function |
| C8 | state.hpp | the C++ mirror structs of C1 (`GPUDirectionalLight` etc. — publish actual names) and their static_asserts | sizes 48 / 272 / 528 |

STOP conditions: C3 counts differ; any of the three bindings reached
outside `entity_fs`'s closure; C1 sizes differ from 48/272/528.

### Phase E — edits

- E1 (world.wgsl): immediately after the three struct definitions,
  add:

      struct Lighting {
          sun    : DirectionalLight,   // offset   0
          points : PointLightArray,    // offset  48
          spots  : SpotLightArray,     // offset 320
      };                               // size 848, uniform-legal

  Offsets 0/48/320 are 16-multiples and each member is followed by a
  16-multiple of space; total 848 ≤ 65536. The three member structs
  remain defined where they are — one fact, one home; only their
  standalone bindings die.
- E2 (world.wgsl): delete the three declarations from C2; add one:
  `@group(0) @binding(320) var<uniform> render_lighting : Lighting;`
- E3 (world.wgsl): rewrite the 11 access sites:
  `render_light.` → `render_lighting.sun.`;
  `render_point_lights.` → `render_lighting.points.`;
  `render_spot_lights.` → `render_lighting.spots.`.
  Post-edit grep of the three old symbols in code must be 0; comments
  naming them update spellings, content preserved (standing order 7).
- E4 (binding_registry.hpp): retire the three constants; add one
  constant `render_lighting = 320`. Update the registry banner's
  declaration/slot counts if it states them (0b-1 reproduced 100 over
  97 at census — this commit makes it 98 over 95; T-CLOSE verifies).
- E5 (state.hpp): C++ struct, adjacent to the C8 mirrors, using the
  actual mirror-struct names from C8:

      struct GPULighting {
          <C8 sun type>    sun;
          <C8 points type> points;
          <C8 spots type>  spots;
      };
      static_assert(sizeof(GPULighting) == 848);

- E6 (state.hpp): layout — the three entries from C5 become ONE entry:
  binding `bind::g0::render_lighting`, Fragment visibility,
  `wgpu::BufferBindingType::Uniform`. Array size 17 → 15; every
  entry-count caption updates in the same commit (BUDGET_1-8b
  precedent: falsified captions are defects).
- E7 (state.hpp): bind group — three entries become one, binding the
  new buffer. Three buffer members become one `GPULighting`-sized
  buffer, usage `Uniform | CopyDst`.
- E8 (write sites from C7): if all three writes sit in one function,
  compose one CPU-side `GPULighting` value and write it whole;
  otherwise write the three members at offsets 0/48/320 into the one
  buffer. This choice is proceed-and-report: take it, state which, do
  not wait.
- E9: naga; L1; commit; push.

### Gate row

| fixes | could break | artifact the reader would see | witness | revert |
|---|---|---|---|---|
| 3 F-stage storage seats freed across the entire entity family; the fog-anchor work gets its one home; two finding-4 stale-comment neighborhoods die | Every `entity_fs` pipeline's lighting; FXC recompiles the whole render family — `sample_spot_shadow_pcf` carries measured FXC cost, and cbuffer-indexed light loops are new codegen for it | Wrong or black lighting on all entities; broken spot shadows; or boot fails at pipeline creation; or glaw1 compile time regresses pathologically | glaw1 + boot; visual gate on a scene with point AND spot lights active | this one commit — and if glaw1 shows a pathological FXC time, revert and hold WALLET_1 for the DXC-flip era; Jean rules |

---

## ORB_V — render_orb_state to instance-step vertex buffer

**Commit 4 — `ORB_V: render_orb_state to instance-step vertex buffer; render V storage 7->6`**

Table F facts of record: eligible instance-step, stride 80 ≤ 2048;
exactly one pipeline reaches it (no Shadow Orb exists); its slot is
one of the three names on one buffer (`orb_state` /
`render_orb_state` / `orb_state_ro`), so retiring it retires a
registry site rather than relocating one.

### Phase C — site census

| # | file | site | predicate |
|---|---|---|---|
| C1 | world.wgsl | `struct OrbState` | publish full field list with offsets; size 80 |
| C2 | world.wgsl | `orb_vs` entire function | exactly 1 read of `render_orb_state`, indexed by the instance builtin (Table F: `builtin_sequential(instance)`); publish the existing vertex-input signature and its `@location` numbers |
| C3 | world.wgsl | declaration at `@group(0) @binding(400)` | `var<storage, read>`, runtime `array<OrbState>` |
| C4 | renderer.hpp | orb render pipeline vertex state | currently 1 vertex buffer, 1 attribute (Table B); publish stride, format, location, step mode |
| C5 | render pass encoding + `bodies/orbs.hpp` | the `draw_orbs` path: encoder block, SetVertexBuffer calls, `DrawIndexed(… os.count …)` | publish; instanceCount is `os.count` (Table G) |
| C6 | state.hpp | orb state buffer creation (the buffer behind the trio) | publish usage flags |
| C7 | binding_registry.hpp + state.hpp | `bind::g0::render_orb_state` constant; the renderEntityBindGroupLayout_ entry and bind-group entry naming it | present; layout is 15 entries after WALLET_1 |
| C8 | state.hpp | the comment containing "VS storage-buffer cap is full" (or nearest variant — ledger finding 4) | publish the sentence |

STOP conditions: C2 shows more than 1 read site or a non-instance
index; C1 size ≠ 80; any OrbState field type with no matching WebGPU
vertex format.

### Phase E — edits

- E1 (renderer.hpp): add a SECOND vertex buffer to the orb pipeline:
  `arrayStride = 80`, `stepMode = Instance`, one attribute PER
  OrbState field, offsets equal to the C1 struct offsets, formats
  mapped 1:1 (`f32`→float32, `vec2f`→float32x2, `vec3f`→float32x3,
  `vec4f`→float32x4, `u32`→uint32, `vec4u`→uint32x4), locations
  numbered consecutively after the existing attribute's location from
  C4.
- E2 (world.wgsl): extend `orb_vs`'s input signature with the new
  `@location` parameters, one per field, named after the fields. Then
  replace the single C2 fetch line with a local reconstruction:
  `let orb = OrbState(<inputs in C1 field order>);` — the function
  body below it stays byte-identical. Delete the C3 declaration.
- E3 (binding_registry.hpp): retire `bind::g0::render_orb_state`. The
  registry's own prose noting the trio-of-names updates to a duo.
- E4 (state.hpp): remove the layout entry and bind-group entry from
  C7. Layout 15 → 14; captions update. Add
  `| wgpu::BufferUsage::Vertex` to the C6 creation flags.
- E5 (encoder, C5 site): `SetVertexBuffer(1, <orb state buffer>)`
  before the orb draw (slot number = the new buffer's index from E1).
- E6 (comment, C8): the sentence updates to the present truth — the
  vertex stage stands at 6 of 8 storage; cite the ledger.
- E7: naga; L1; commit; push.

### Gate row

| fixes | could break | artifact the reader would see | witness | revert |
|---|---|---|---|---|
| 1 V-stage storage seat freed family-wide; a registry alias site retired; vertex wallet opened deliberately (2/8, attrs ≤ 1+fields) | Orb sky layer only — its pipeline is the sole reacher; vertex-layout/struct-offset mismatch is the failure class | Orbs missing, misplaced, or wrongly colored in the sky layer; or D3D12 pipeline creation fails | glaw1 + boot; visual gate on an orb-active mood | this one commit |

---

## ARENA_0 — mesh-arena recon (READ-ONLY)

**Commit 5 — `ARENA_0: mesh-arena recon report (read-only)`**

Zero source edits. Output: `audit/ARENA_0_RECON.md` (or the audit
corpus's actual home — match the tree). Answer, with verbatim
citations by symbol:

1. The five families' buffer creation sites (arch, column, palm,
   cactus, blade): params / vertices / indices sizes and usage flags,
   and the C++ member names — this doubles as the buffer-identity
   census for these fifteen sites.
2. Render-side vertex layout per species: stride, attribute formats,
   locations. State whether all five are byte-identical.
3. Index format and today's `DrawIndexed` arguments per species:
   indexCount source, `baseVertex`, `firstIndex` (expected 0/0 with
   per-species buffers — confirm).
4. Kernel write addressing: Table F shows `abs_idx` and `ib_start`
   flowing as callee parameters in the mesh-gen kernels — confirm
   writes are already offset-parameterized (arena-favorable) and name
   where the base offsets originate.
5. The five params structs' field lists side by side (80/128/128/128/
   80 B): overlap and divergence.
6. Arithmetic: sum of current vertex bytes and index bytes across the
   five = the arena sizes a merge would need; against
   maxStorageBufferBindingSize this is nothing, state it anyway.
7. Dispatch shapes per species (Table G column) — the arena keeps five
   entry points and five dispatches; confirm nothing couples them.

Close the report with a GO / NO-GO matrix per precondition. No
recommendation beyond the matrix — Jean rules the arena.

---

## T-CLOSE — closing anchor

**Commit 6 — `TETRIS T-CLOSE: regenerate BINDING_LEDGER; Table B matches predictions`**

Re-run the ledger tool. All witnesses PASS. The regenerated Table B
must read:

| row | prediction |
|---|---|
| room family C storage | **6 of 8** |
| room family C uniform | 10 of 12 (Branch A) / 12 of 12 (Branch B) |
| render family V storage | **6 of 8** |
| render family F storage | **4 of 8** |
| render family F uniform | 4 of 12 |
| renderEntityBindGroupLayout_ | 14 entries |
| program vertex buffers | 2 of 8 |
| dynamic offsets | still 0 of 8 and 0 of 4 — untouched this round |
| Table C | occupier pair, light trio, and `render_orb_state` rows gone |

Any deviation from a predicted cell: do not "fix" the program to match
— publish the deviation and STOP. The prediction table is the
campaign's own witness; a miss means either an edit or this handoff is
wrong, and that ruling is Jean's.

Final report to Jean: HEAD shas open and close, branch A/B outcome,
the six commit shas, naga results, the three "awaiting glaw1" flags,
and the ARENA_0 GO/NO-GO matrix.
