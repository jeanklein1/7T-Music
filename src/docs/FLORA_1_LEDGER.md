# FLORA_1 — F1 RECON LEDGER

Read-only reconnaissance for the FLORA_1 campaign. Branch `claude/flora-1`,
cut from master `d785fd9`. **No code was changed by this unit.**

Every entry is a verbatim quote from the live tree with file and line, per the
campaign register: report, do not interpret. Where the handoff offered a claim
"to be confirmed or refuted", the arithmetic was redone from the code and the
verdict is stated with its numbers.

Recon ran against HEAD `b7e4d92` (post-F0), so palm figures are the reduced
proportions, not the pre-F0 ones.

---

## Items 1–3 — palm clamps, slot constants, bounds guards, the zero-fill gate

**Scope as run:** ITEMS 1-3: palm_mesh_gen clamps, slot capacity constants, bounds guards, and trailing index zero-fill in /home/user/7T-Pawns/src/cartridges/the_board/realization/world.wgsl (branch claude/flora-1, HEAD b7e4d92)

### Answer

ITEM 1 — n_fronds. world.wgsl:11361, verbatim: `    let n_fronds = min(u32(max(3.0, p.frond_count)), 18u);`. Literal clamp value: 18u (upper), with a floor of max(3.0, ...).

ITEM 2 — capacity constants and clamps. world.wgsl:11213 `const PALMG_MAX_VERTS_PER_SLOT: u32 = 1200u;` and world.wgsl:11214 `const PALMG_MAX_INDICES_PER_SLOT: u32 = 6000u;`. Literals: 1200 verts / 6000 indices per slot. Every min(...) clamp on the three named quantities (these are the complete set — each is declared exactly once in the function):
- world.wgsl:11265 `    let trunk_rings = min(u32(max(8.0, p.bark_rings)), 40u);`
- world.wgsl:11266 `    let trunk_segs = min(p.trunk_segs, 24u);`
- world.wgsl:11362 `    let frond_segs = min(p.frond_segs, 14u);`

ITEM 3 (GATE) — PALM ZERO-FILL PRESENT.

Part A, bounds guard on vi or ii before a write: none. Searched the entire body of palm_mesh_gen (lines 11239-11477) for `if (`, for any `vi`/`ii` comparison operator, and for `return`/`break`/`clamp(`. The only guard-shaped constructs are world.wgsl:11241 `    if (slot >= PALMG_MAX_SLOTS) { return; }` (guards slot, not vi/ii), world.wgsl:11247 (the is_active branch), and two geometry branches at 11386 and 11391. Neither `vi` nor `ii` is ever compared against PALMG_MAX_VERTS_PER_SLOT / PALMG_MAX_INDICES_PER_SLOT before a write. Every write via palmg_write_vertex and every `palmg_indices[ib_base + ii] = ...` is unguarded.

Part B, trailing index zero-fill on the ACTIVE path: PRESENT, and it matches the asserted form exactly. world.wgsl:11473-11477, verbatim:
```
    // Zero remaining indices (degenerate padding)
    for (var i = ii; i < PALMG_MAX_INDICES_PER_SLOT; i++) {
        palmg_indices[ib_base + i] = vb_base;
    }
}
```
It is the final statement of the active path, immediately after the frond loop closes at line 11471. (Separately, the inactive path has its own fill at world.wgsl:11247-11252.)

The two reference forms, on record:
- cactus_mesh_gen, world.wgsl:11810-11814 — `    // Zero remaining indices` / `    for (var i = ii; i < CACTUSG_MAX_INDICES_PER_SLOT; i++) {` / `        cactusg_indices[ib_base + i] = vb_base;` / `    }` / `}`. Structurally identical to palm's.
- column_mesh_gen, world.wgsl:11178-11183 — `    // ── Zero remaining indices ─────────────────────────────────` / `    let used = ii - slot_ib;` / `    for (var i = used; i < CMG_MAX_INDICES_PER_SLOT; i++) {` / `        cmg_indices[slot_ib + i] = slot_vb;` / `    }` / `}`. NOT byte-identical: column's `ii` is an absolute buffer index, so it derives `used = ii - slot_ib` first and loops from `used`; variable names are slot_ib/slot_vb instead of ib_base/vb_base. Semantically the same degenerate fill.

PALM_TIERS verification (grounded.hpp:1131-1153) — CONFIRM, all 18 asserted values match verbatim. SAPLING (line 1133-1137): height 25.2 / bark_rings 12.0 / frond_count 20.0 / frond_len 6.8 / trunk_segs 12 / frond_segs 8. COASTAL (1140-1144): 28.8 / 20.0 / 24.0 / 6.8 / 16 / 10. ROYAL (1147-1151): 37.8 / 30.0 / 30.0 / 8.8 / 20 / 12.

Arithmetic I performed myself, given Part A found no vi/ii guard (counts derived from the loop structure at world.wgsl:11268-11296, 11332-11349, 11365-11454, and index loops at 11300-11318, 11351-11356, 11457-11470):
verts = (trunk_rings+1)*trunk_segs + 1 + trunk_segs + n_fronds*(frond_segs+1)*2
indices = trunk_rings*trunk_segs*6 + trunk_segs*3 + n_fronds*frond_segs*6
- At the clamp ceilings (40/24/18/14): verts = 41*24 + 1 + 24 + 18*15*2 = 984+25+540 = 1549 vs cap 1200 → overflow by 349. indices = 40*24*6 + 24*3 + 18*14*6 = 5760+72+1512 = 7344 vs cap 6000 → overflow by 1344.
- At ROYAL mu exactly (bark_rings 30, trunk_segs 20, frond_count 30 clamped to 18, frond_segs 12): verts = 31*20 + 1 + 20 + 18*13*2 = 620+21+468 = 1109 ≤ 1200 OK. indices = 30*20*6 + 60 + 18*12*6 = 3600+60+1296 = 4956 ≤ 6000 OK.
- At ROYAL with bark_rings sampled to the 40u clamp (mu 30, sigma 5.0, i.e. +2 sigma; no CPU upper bound per grounded.hpp:1081): verts = 41*20 + 1 + 20 + 468 = 1309 > 1200 → overflow by 109. indices = 40*20*6 + 60 + 1296 = 6156 > 6000 → overflow by 156.
So the slot budget is exceedable at reachable sampled values, and there is no vi/ii guard to stop it — but that is a separate defect from the zero-fill, which is present.

### Quotes

**`src/cartridges/the_board/realization/world.wgsl:11361`**

```
    let n_fronds = min(u32(max(3.0, p.frond_count)), 18u);
```

ITEM 1. Literal clamp value = 18u (upper). Lower floor = max(3.0, ...). Inside palm_mesh_gen (fn spans lines 11239-11477).

**`src/cartridges/the_board/realization/world.wgsl:11213`**

```
const PALMG_MAX_VERTS_PER_SLOT: u32 = 1200u;
```

ITEM 2. Literal value 1200u. Declared at file scope just above palm_mesh_gen.

**`src/cartridges/the_board/realization/world.wgsl:11214`**

```
const PALMG_MAX_INDICES_PER_SLOT: u32 = 6000u;
```

ITEM 2. Literal value 6000u.

**`src/cartridges/the_board/realization/world.wgsl:11265`**

```
    let trunk_rings = min(u32(max(8.0, p.bark_rings)), 40u);
```

ITEM 2. The ONLY min() clamp on trunk_rings. Upper 40u, floor max(8.0,...).

**`src/cartridges/the_board/realization/world.wgsl:11266`**

```
    let trunk_segs = min(p.trunk_segs, 24u);
```

ITEM 2. The ONLY min() clamp on trunk_segs. Upper 24u, no floor.

**`src/cartridges/the_board/realization/world.wgsl:11362`**

```
    let frond_segs = min(p.frond_segs, 14u);
```

ITEM 2. The ONLY min() clamp on frond_segs. Upper 14u, no floor.

**`src/cartridges/the_board/realization/world.wgsl:11241`**

```
    if (slot >= PALMG_MAX_SLOTS) { return; }
```

ITEM 3. This is the only bounds-style guard in palm_mesh_gen and it guards SLOT, not vi/ii. Bounds guard on vi or ii before a write: none. Searched the full function body lines 11239-11477 for `if (` (only 4 hits: 11241 slot guard, 11247 is_active, 11386 sin_el > 0.95, 11391 rl > 0.001) and for any vi/ii comparison, return, break, or clamp() — no vi/ii guard exists.

**`src/cartridges/the_board/realization/world.wgsl:11473`**

```
    // Zero remaining indices (degenerate padding)
    for (var i = ii; i < PALMG_MAX_INDICES_PER_SLOT; i++) {
        palmg_indices[ib_base + i] = vb_base;
    }
}
```

ITEM 3 GATE — PALM ZERO-FILL PRESENT. Verbatim lines 11473-11477. It is the last statement of palm_mesh_gen's ACTIVE path (immediately after the frond loop closes at 11471), and matches the asserted form character-for-character: `for (var i = ii; i < PALMG_MAX_INDICES_PER_SLOT; i++) { palmg_indices[ib_base + i] = vb_base; }`.

**`src/cartridges/the_board/realization/world.wgsl:11810`**

```
    // Zero remaining indices
    for (var i = ii; i < CACTUSG_MAX_INDICES_PER_SLOT; i++) {
        cactusg_indices[ib_base + i] = vb_base;
    }
}
```

ITEM 3 model form — cactus_mesh_gen (fn starts 11571). Verbatim lines 11810-11814. Identical shape to palm's: loop from ii, fill with vb_base.

**`src/cartridges/the_board/realization/world.wgsl:11178`**

```
    // ── Zero remaining indices ─────────────────────────────────
    let used = ii - slot_ib;
    for (var i = used; i < CMG_MAX_INDICES_PER_SLOT; i++) {
        cmg_indices[slot_ib + i] = slot_vb;
    }
}
```

ITEM 3 model form — column_mesh_gen (fn starts 10741). Verbatim lines 11178-11183. NOT byte-identical to palm/cactus: column's `ii` is an ABSOLUTE index, so it first computes `let used = ii - slot_ib;` and loops from `used`; names are slot_ib/slot_vb rather than ib_base/vb_base. Semantically the same fill.

**`src/cartridges/the_board/realization/world.wgsl:11247`**

```
    if (p.is_active == 0u) {
        for (var i = 0u; i < PALMG_MAX_INDICES_PER_SLOT; i++) {
            palmg_indices[ib_base + i] = vb_base;
        }
        return;
    }
```

Context for ITEM 3: the INACTIVE path zero-fill (lines 11247-11252), distinct from the active-path fill at 11473. Both exist.

**`src/cartridges/the_board/bodies/grounded.hpp:1133`**

```
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {6.8f, 0.85f},  {0.80f, 0.15f},
                   {1.20f, 0.10f}, {0.45f, 0.08f}, {0.65f, 0.10f}, {0.20f, 0.08f}, {0.20f, 0.05f},
                   {0.30f, 0.05f} }},
        0.15f, 0.15f, 0.06f, 0.06f, 12, 8
```

SAPLING row, lines 1133-1137. CONFIRM: height mu 25.2, bark_rings mu 12.0, frond_count mu 20.0, frond_len mu 6.8, trunk_segs 12, frond_segs 8 — all match the asserted post-F0 values.

**`src/cartridges/the_board/bodies/grounded.hpp:1140`**

```
        { 0.35f, 0.0f, { {28.8f, 3.6f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {6.8f, 1.28f},  {1.00f, 0.20f},
                   {0.65f, 0.12f}, {0.45f, 0.20f}, {0.95f, 0.12f}, {0.60f, 0.10f}, {0.30f, 0.08f},
                   {0.40f, 0.08f} }},
        0.20f, 0.15f, 0.06f, 0.06f, 16, 10
```

COASTAL row, lines 1140-1144. CONFIRM: height 28.8, bark_rings 20.0, frond_count 24.0, frond_len 6.8, trunk_segs 16, frond_segs 10 — all match.

**`src/cartridges/the_board/bodies/grounded.hpp:1147`**

```
        { 0.20f, 0.0f, { {37.8f, 7.2f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {8.8f, 2.0f},   {1.40f, 0.35f},
                   {0.75f, 0.22f}, {0.40f, 0.12f}, {0.85f, 0.10f}, {0.50f, 0.12f}, {0.50f, 0.10f},
                   {0.50f, 0.10f} }},
        0.25f, 0.20f, 0.08f, 0.08f, 20, 12
```

ROYAL row, lines 1147-1151. CONFIRM: height 37.8, bark_rings 30.0, frond_count 30.0, frond_len 8.8, trunk_segs 20, frond_segs 12 — all match.

**`src/cartridges/the_board/bodies/grounded.hpp:1081`**

```
    { PalmProp::BARK_RINGS,   3.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
```

Relevant to ITEM 3 arithmetic: BARK_RINGS has NO effective upper bound on the CPU side (1e30f), so the GPU min(...,40u) at world.wgsl:11265 is the only cap. Same for FROND_COUNT at line 1083: `{ PalmProp::FROND_COUNT,  3.0f, 1e30f,  true,  ParamDist::GAUSSIAN },`.

---

## Item 4 — palm vertex budget against PALMG_MAX_VERTS_PER_SLOT

**Scope as run:** ITEM 4 — palm_mesh_gen vertex budget vs PALMG_MAX_VERTS_PER_SLOT (post-F0 PALM_TIERS mu)

### Answer

VERDICT ON THE CLAIM: **CONFIRM** — all four numbers in the claim reproduce exactly from the code.

=== THE FOUR CLAMPS AS READ (world.wgsl) ===
L11265  trunk_rings = min(u32(max(8.0, p.bark_rings)), 40u)   -> floor 8, ceiling 40
L11266  trunk_segs  = min(p.trunk_segs, 24u)                  -> ceiling 24 (NEVER binds: authored max 20)
L11361  n_fronds    = min(u32(max(3.0, p.frond_count)), 18u)  -> ceiling 18 (BINDS on all three tiers)
L11362  frond_segs  = min(p.frond_segs, 14u)                  -> ceiling 14 (NEVER binds: authored max 12)
Budget: PALMG_MAX_VERTS_PER_SLOT = 1200 (world.wgsl:11213, state.hpp:287).

Formula (verified against the emit sites): verts = (trunk_rings+1)*trunk_segs  [L11268 inclusive <= x L11286]  + 1  [cap tip, L11332-11337]  + trunk_segs  [cap ring, L11340]  + n_fronds * (frond_segs+1)*2  [L11401 inclusive <= , two verts per s at L11436/L11446].

=== SAPLING (bark_rings mu 12, frond_count 20, trunk_segs 12, frond_segs 8) ===
Clamped: trunk_rings=min(max(8,12),40)=12; trunk_segs=min(12,24)=12; frond_segs=min(8,14)=8; n_fronds=min(max(3,20),18)=18 (CLAMP BINDS, 20->18).
Fixed base = (12+1)*12 + 1 + 12 = 156 + 13 = 169.  Per-frond = (8+1)*2 = 18.
(a) at mu rings, AUTHORED 20 fronds: 169 + 20*18 = 169 + 360 = **529**  (671 under budget)
(b) with the literal 18-frond clamp:  169 + 18*18 = 169 + 324 = **493**  (707 under budget)
(c) crossing 1200: verts(R) = 12R + 12 + 13 + F*18.
    authored 20 fronds -> 12R + 385 > 1200 needs R > 67.9; at the R=40 hard clamp verts = 480 + 385 = 865. **NEVER CROSSES** — unreachable inside the clamp.
    clamped 18 fronds  -> 12R + 349 > 1200 needs R > 70.9; at R=40 verts = 829. **NEVER CROSSES.**
(d) max affordable fronds: at mu rings=12, budget left = 1200 - 169 = 1031; 1031/18 = 57.28 -> **57 fronds** (169+57*18 = 1195 OK; 58 -> 1213 OVER).
    at the R=40 ring clamp, base = 41*12 + 13 = 505; left = 695; 695/18 = 38.6 -> **38 fronds** (505+38*18 = 1189 OK; 39 -> 1207 OVER).

=== COASTAL (bark_rings mu 20, frond_count 24, trunk_segs 16, frond_segs 10) ===
Clamped: trunk_rings=20; trunk_segs=min(16,24)=16; frond_segs=min(10,14)=10; n_fronds=min(max(3,24),18)=18 (CLAMP BINDS, 24->18).
Fixed base = (20+1)*16 + 1 + 16 = 336 + 17 = 353.  Per-frond = (10+1)*2 = 22.
(a) at mu rings, AUTHORED 24 fronds: 353 + 24*22 = 353 + 528 = **881**  (319 under budget)
(b) with the literal 18-frond clamp:  353 + 18*22 = 353 + 396 = **749**  (451 under budget)
(c) crossing 1200: verts(R) = 16R + 16 + 17 + F*22.
    authored 24 fronds -> 16R + 561 > 1200 needs 16R > 639, R > 39.94. R=39 -> 624+561 = 1185 (OK). R=40 -> 640+561 = **1201, OVER BY 1**. So it is safe at 24 fronds through bark_rings 39 and crosses at exactly **bark_rings = 40**, which is also the hard clamp — i.e. the very last legal ring value is the only failing one.
    clamped 18 fronds  -> 16R + 429; at R=40 verts = 1069. **NEVER CROSSES.**
(d) max affordable fronds: at mu rings=20, left = 1200-353 = 847; 847/22 = 38.5 -> **38 fronds** (1189 OK; 39 -> 1211 OVER).
    at the R=40 ring clamp, base = 41*16 + 17 = 673; left = 527; 527/22 = 23.95 -> **23 fronds** (1179 OK; 24 -> 1201 OVER).

=== ROYAL (bark_rings mu 30, frond_count 30, trunk_segs 20, frond_segs 12) ===
Clamped: trunk_rings=30; trunk_segs=min(20,24)=20; frond_segs=min(12,14)=12; n_fronds=min(max(3,30),18)=18 (CLAMP BINDS HARD, 30->18).
Fixed base = (30+1)*20 + 1 + 20 = 620 + 21 = 641.  Per-frond = (12+1)*2 = 26.
(a) at mu rings, AUTHORED 30 fronds: 641 + 30*26 = 641 + 780 = **1421 — OVER BUDGET by 221** (118.4% of 1200)
(b) with the literal 18-frond clamp:  641 + 18*26 = 641 + 468 = **1109** (91 under budget — only the 18-frond literal is keeping Royal inside the slot at mu)
(c) crossing 1200: verts(R) = 20R + 20 + 21 + F*26.
    authored 30 fronds -> 20R + 821 > 1200 needs 20R > 379, R > 18.95. R=18 -> 1181 OK; **R=19 -> 1201, CROSSES AT bark_rings = 19** — i.e. 11 rings BELOW mu (mu 30 is ~2.2 sigma past the crossing at sigma 5).
    clamped 18 fronds  -> 20R + 509 > 1200 needs 20R > 691, R > 34.55. R=34 -> 1189 OK; **R=35 -> 1209, CROSSES AT bark_rings = 35**. At the R=40 hard clamp: 800 + 509 = **1309, OVER BY 109 EVEN WITH THE 18-FROND CLAMP APPLIED.** bark_rings mu 30 sigma 5 puts R=35 at exactly +1 sigma, so this is a routinely-reachable sample, not a tail case.
(d) max affordable fronds: at mu rings=30, left = 1200-641 = 559; 559/26 = 21.5 -> **21 fronds** (641+21*26 = 1187 OK; 22 -> 1213 OVER).
    at the R=40 ring clamp, base = 41*20 + 21 = 841; left = 359; 359/26 = 13.8 -> **13 fronds** (841+13*26 = 1179 OK; 14 -> 1205 OVER).

=== CLAIM CHECK, TERM BY TERM ===
"Sapling ~529"                          -> 529 exactly. CONFIRM.
"with headroom for ~57 fronds"          -> 57 at mu bark_rings. CONFIRM.
"Coastal ~881"                          -> 881 exactly. CONFIRM.
"safe at 24 until bark_rings ~39"       -> safe through 39 (1185), crosses at 40 (1201). CONFIRM.
"Royal ~1421"                           -> 1421 exactly. CONFIRM.
"OVER BUDGET at mu"                     -> 1421 > 1200 at authored frond count. CONFIRM.
"affording ~21 at mu"                   -> 21 at bark_rings 30. CONFIRM.
"and ~13 at the ring clamp"             -> 13 at bark_rings 40. CONFIRM.

=== TWO THINGS THE CLAIM DOES NOT STATE (arithmetic mine, from the same lines) ===
1. Royal's authored 30 fronds are never actually emitted: the L11361 literal 18u truncates 30 -> 18, so the real at-mu cost is 1109, not 1421. The 1421 is the authored-intent cost the shader silently refuses to pay. Same truncation hits Coastal (24->18) and Sapling (20->18).
2. Even WITH that truncation, Royal overflows the slot at bark_rings >= 35 (1209 at 35, 1309 at the 40 clamp), and there is no bounds guard on the write path — see the finding at world.wgsl:11222.

=== WHERE I LOOKED / ABSENCES ===
Searched /home/user/7T-Pawns/src for "palm_mesh_gen" and "PALMG_MAX_VERTS_PER_SLOT". Any guard comparing the running vertex counter vi against PALMG_MAX_VERTS_PER_SLOT inside palm_mesh_gen (world.wgsl:11239-11477) or palmg_write_vertex (world.wgsl:11222-11236): none. Any CPU-side clamp of frond_count/bark_rings before upload in palm_write_gpu (grounded.hpp:1238-1266): none — params[] go across raw, only trunk_segs/frond_segs come from the tier row (lines 1261-1262). Any static_assert tying PALM_TIERS tessellation to PALMG_MAX_VERTS_PER_SLOT: none (grounded.hpp has only the F-4/F-5 row-count asserts at lines 1094 and 1154). Files read: /home/user/7T-Pawns/src/cartridges/the_board/realization/world.wgsl, /home/user/7T-Pawns/src/cartridges/the_board/bodies/grounded.hpp, /home/user/7T-Pawns/src/cartridges/the_board/realization/state.hpp. No edits made; working tree clean at b7e4d92 on claude/flora-1.

### Quotes

**`src/cartridges/the_board/realization/world.wgsl:11213`**

```
const PALMG_MAX_VERTS_PER_SLOT: u32 = 1200u;
```

GPU-side budget declaration

**`src/cartridges/the_board/realization/state.hpp:287`**

```
            constexpr uint32_t PALMG_MAX_VERTS_PER_SLOT = 1200;
```

CPU-side mirror of the same budget

**`src/cartridges/the_board/realization/world.wgsl:11265`**

```
    let trunk_rings = min(u32(max(8.0, p.bark_rings)), 40u);
```

CLAMP: bark_rings floor 8, hard ceiling 40

**`src/cartridges/the_board/realization/world.wgsl:11266`**

```
    let trunk_segs = min(p.trunk_segs, 24u);
```

CLAMP: trunk_segs ceiling 24 (never binds; authored max is 20)

**`src/cartridges/the_board/realization/world.wgsl:11361`**

```
    let n_fronds = min(u32(max(3.0, p.frond_count)), 18u);
```

CLAMP: frond_count floor 3, LITERAL ceiling 18 — binds on all three tiers (authored 20/24/30)

**`src/cartridges/the_board/realization/world.wgsl:11362`**

```
    let frond_segs = min(p.frond_segs, 14u);
```

CLAMP: frond_segs ceiling 14 (never binds; authored max is 12)

**`src/cartridges/the_board/realization/world.wgsl:11268`**

```
    for (var ring = 0u; ring <= trunk_rings; ring++) {
```

inclusive <= : yields (trunk_rings + 1) rings

**`src/cartridges/the_board/realization/world.wgsl:11286`**

```
        for (var seg = 0u; seg < trunk_segs; seg++) {
```

trunk_segs vertices per ring -> (trunk_rings+1)*trunk_segs trunk verts

**`src/cartridges/the_board/realization/world.wgsl:11332`**

```
    let cap_tip_vi = vi;
```

the +1 in the formula: single crown-cap tip vertex written at 11333

**`src/cartridges/the_board/realization/world.wgsl:11340`**

```
    for (var seg = 0u; seg < trunk_segs; seg++) {
```

crown cap ring: + trunk_segs verts

**`src/cartridges/the_board/realization/world.wgsl:11401`**

```
        for (var s = 0u; s <= frond_segs; s++) {
```

inclusive <= , two verts written per s (11436 left, 11446 right) -> (frond_segs+1)*2 per frond

**`src/cartridges/the_board/realization/world.wgsl:11222`**

```
fn palmg_write_vertex(abs_idx: u32, px: f32, py: f32, pz: f32,
```

no bounds check against PALMG_MAX_VERTS_PER_SLOT anywhere in palmg_write_vertex (11222-11236) or palm_mesh_gen (11239-11477); overflow silently writes into the next slot's vertex range

**`src/cartridges/the_board/bodies/grounded.hpp:1133`**

```
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
```

SAPLING HEIGHT mu 25.2 — verifies post-F0 value

**`src/cartridges/the_board/bodies/grounded.hpp:1134`**

```
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {6.8f, 0.85f},  {0.80f, 0.15f},
```

SAPLING BARK_RINGS mu 12 sigma 3, FROND_COUNT mu 20 sigma 3, FROND_LEN mu 6.8

**`src/cartridges/the_board/bodies/grounded.hpp:1137`**

```
        0.15f, 0.15f, 0.06f, 0.06f, 12, 8
```

SAPLING trunk_segs 12, frond_segs 8

**`src/cartridges/the_board/bodies/grounded.hpp:1140`**

```
        { 0.35f, 0.0f, { {28.8f, 3.6f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
```

COASTAL HEIGHT mu 28.8

**`src/cartridges/the_board/bodies/grounded.hpp:1141`**

```
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {6.8f, 1.28f},  {1.00f, 0.20f},
```

COASTAL BARK_RINGS mu 20 sigma 4, FROND_COUNT mu 24 sigma 5, FROND_LEN mu 6.8

**`src/cartridges/the_board/bodies/grounded.hpp:1144`**

```
        0.20f, 0.15f, 0.06f, 0.06f, 16, 10
```

COASTAL trunk_segs 16, frond_segs 10

**`src/cartridges/the_board/bodies/grounded.hpp:1147`**

```
        { 0.20f, 0.0f, { {37.8f, 7.2f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
```

ROYAL HEIGHT mu 37.8

**`src/cartridges/the_board/bodies/grounded.hpp:1148`**

```
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {8.8f, 2.0f},   {1.40f, 0.35f},
```

ROYAL BARK_RINGS mu 30 sigma 5, FROND_COUNT mu 30 sigma 7, FROND_LEN mu 8.8

**`src/cartridges/the_board/bodies/grounded.hpp:1151`**

```
        0.25f, 0.20f, 0.08f, 0.08f, 20, 12
```

ROYAL trunk_segs 20, frond_segs 12

**`src/cartridges/the_board/bodies/grounded.hpp:1261`**

```
    mp.trunk_segs = tp.trunk_segs;
```

tier segs reach the shader verbatim (frond_segs on line 1262) — no CPU-side pre-clamp

**`src/cartridges/the_board/realization/world.wgsl:11214`**

```
const PALMG_MAX_INDICES_PER_SLOT: u32 = 6000u;
```

index budget, for reference only — not part of this item's arithmetic

---

## Items 5–7 — cactus arm ring basis, fork lean, arm cap block

**Scope as run:** ITEMS 5-7 — cactus_mesh_gen ARM loop recon in world.wgsl (plus PALM_TIERS mu verification in grounded.hpp)

### Answer

READ-ONLY recon complete on /home/user/7T-Pawns, branch claude/flora-1, HEAD 8f6b092. No edits, no commits. All quotes are verbatim from the live working-tree files.

=== PALM_TIERS PRE-CHECK (grounded.hpp) — CONFIRM, all 18 values ===
Verified against PalmIdx (grounded.hpp:1055-1073) and PalmTierRow field order (grounded.hpp:1105-1113). SAPLING (1132-1138) height 25.2 / bark_rings 12 / frond_count 20 / frond_len 6.8 / trunk_segs 12 / frond_segs 8 — CONFIRM. COASTAL (1139-1145) 28.8 / 20 / 24 / 6.8 / 16 / 10 — CONFIRM. ROYAL (1146-1152) 37.8 / 30 / 30 / 8.8 / 20 / 12 — CONFIRM. Zero discrepancies.

=== ITEM 5 ===
(a) The 'if (abs(ndy) > 0.95)' block and its else branch: NONE.
Where I looked: /home/user/7T-Pawns/src/cartridges/the_board/realization/world.wgsl, function cactus_mesh_gen, lines 11571-11824 (read in full); plus a whole-file grep for 'abs(ndy)' (zero hits anywhere in the file) and for '0.95' (10 hits file-wide, only ONE inside cactus_mesh_gen — line 11725, and it is a comment, not code). HEAD commit 8f6b092 "FLORA_1 F2: the cactus arm ring basis loses its runtime branch" is what removed it. There is nothing to quote and no else branch exists.
What stands in its place (world.wgsl:11721-11733) is an unconditional basis: rx = out_z; rz = -out_x; fx = 0.0 - rz * ndy; fy = rz * ndx - rx * ndz; fz = rx * ndy.

(b) Arm RING loop cactusg_write_vertex — world.wgsl:11762-11765. LITERAL NORMAL ARGUMENTS:
  nx = rx * ca + fx * sa
  ny = fy * sa
  nz = rz * ca + fz * sa
This is a TRANSFORMED WORLD NORMAL: it is exactly the position expression at 11746-11748 with the ring centre (apx/apy/apz) and the radius factor '* r' stripped, i.e. (ca, sa) pushed through the (r, f) basis.

(c) TRUNK cactusg_write_vertex — world.wgsl:11628-11630. LITERAL NORMAL ARGUMENTS: 'ca, 0.0, sa'.
TRUNK CONVENTION: UNTRANSFORMED LOCAL RING PARAMETER. ca/sa are declared at 11620-11621 as cos(angle)/sin(angle) of the ring parameter (11615) and passed straight through — no basis transform, no rib_mod slope correction, no taper/cap_scale term, no lean term. It happens to equal the world normal only because trunk rings are horizontal circles about a vertical axis, which is precisely the claim the arm-side comment makes at 11756-11758. So the two sites use DIFFERENT conventions: trunk = raw local ring parameter, arm = basis-transformed world normal.

=== ITEM 6 ===
fork_x — world.wgsl:11695: 'let fork_x = cx + cos(arm_az) * p.radius * p.taper * 0.9 + lean_at_fork * lean_cos * 0.3;'
fork_z — world.wgsl:11696: 'let fork_z = cz + sin(arm_az) * p.radius * p.taper * 0.9 + lean_at_fork * lean_sin * 0.3;'
Supporting: world.wgsl:11694 'let lean_at_fork = p.lean * p.height * fork_frac * fork_frac;'
Trunk — world.wgsl:11608-11610: 'let lean_mag = p.lean * p.height * t * t;' / 'let lx = lean_mag * lean_cos;' / 'let lz = lean_mag * lean_sin;'
BOTH LEAN COEFFICIENTS, LITERALLY:
  TRUNK = 1.0 — no literal multiplier is written at all; lx/lz are lean_mag times lean_cos/lean_sin and nothing else.
  ARM FORK = 0.3 — an explicit trailing '* 0.3' on the lean term of both fork_x and fork_z.
Both share the identical p.lean * p.height * (fraction)^2 magnitude form (t*t vs fork_frac*fork_frac), so the difference is purely the 0.3 vs 1.0 scalar: at the same height fraction the fork point is placed at 30% of the trunk surface's lean displacement. (Separately, the radial term carries its own literal 0.9 in both fork lines.)

=== ITEM 7 ===
Arm cap block quoted in full at world.wgsl:11793-11817 (the line above, 11792, is the comment '// Arm cap'). See the finding entry for the verbatim text.
LAST BODY RING'S FIRST VERTEX, as an expression: arm_vi_start + arm_segs_u * arm_around.
  Derivation from code: arm_vi_start is captured at 11706 immediately before the body loop; the body loop at 11708 is 'for (var s = 0u; s <= arm_segs_u; s++)' — INCLUSIVE upper bound, so arm_segs_u + 1 rings — and the inner loop at 11737 writes arm_around vertices per ring. Independently corroborated by 11781, 'let row1 = arm_vi_start + (s + 1u) * arm_around;', whose maximum (the index loop at 11777 runs s < arm_segs_u) is exactly arm_vi_start + arm_segs_u * arm_around. This is a relative index; vb_base is added at each use site.
Declarations:
  arm_cap_segs — world.wgsl:11802: 'let arm_cap_segs = min(arm_around, 8u);'
  arm_around  — world.wgsl:11685: 'let arm_around = min(max(arm_ribs * 2u, 8u), 12u);'
  arm_segs_u  — world.wgsl:11683: 'let arm_segs_u = min(u32(p.arm_segs), 12u);'
Two facts readable directly off the quoted text, stated without interpretation: (1) the cap block writes its own new ring at arm_cap_ring (11801-11810) and its index loop (11812-11817) references only arm_cap_tip and arm_cap_ring — the expression arm_vi_start + arm_segs_u * arm_around appears nowhere in the cap block, so the arm cap does not stitch to the body's last ring; contrast the trunk cap, whose comment at 11671 reads '// Fan from tip to trunk's existing top ring — no separate cap ring' and which indexes top_ring_vi (declared 11653). (2) arm_around ranges 8..12 by its own min/max, so arm_cap_segs = min(arm_around, 8u) equals arm_around only in the arm_around == 8 case.

### Quotes

**`src/cartridges/the_board/realization/world.wgsl:11725`**

```
            // whenever |ndy| passed 0.95 — which arm_curve mu 1.00 now
```

ITEM 5 / NONE. The 'if (abs(ndy) > 0.95)' block does NOT exist. Searched: grep 'abs(ndy)' over the whole file = zero hits; grep '0.95' over the whole file = 10 hits, of which exactly one falls inside cactus_mesh_gen (lines 11571-11824) and it is this comment line. The branch was removed by HEAD commit 8f6b092 'FLORA_1 F2: the cactus arm ring basis loses its runtime branch'. There is no else branch to quote.

**`src/cartridges/the_board/realization/world.wgsl:11721`**

```
            // THE ARM'S PLANE IS FIXED. The path lies entirely in the
            // vertical plane spanned by out = (cos az, 0, sin az) and world
            // up, so ONE horizontal perpendicular serves every ring. The
            // runtime branch it replaces flipped the reference axis mid-arm
            // whenever |ndy| passed 0.95 — which arm_curve mu 1.00 now
            // reaches around t ~ 0.75 (the old designer defaults peaked at
            // 0.949, one hundredth below it). out is already unit, so this
            // is too: no re-normalisation.
            let rx = out_z;
            let rz = -out_x;
            let fx = 0.0 - rz * ndy;
            let fy = rz * ndx - rx * ndz;
            let fz = rx * ndy;
```

ITEM 5 — what replaced the branch: the unconditional (rx, rz) / (fx, fy, fz) basis, lines 11721-11733.

**`src/cartridges/the_board/realization/world.wgsl:11755`**

```
                // The normal takes the SAME basis as the position above.
                // (ca, 0, sa) is the ring's local parameter, not a world
                // direction — correct for the trunk, whose rings are
                // horizontal circles about a vertical axis, and wrong here,
                // where the ring lives in (r, f). r and f are orthonormal by
                // construction (f = r x nd, both unit), so this is unit and
                // needs no normalisation.
```

ITEM 5 — comment immediately preceding the arm ring's cactusg_write_vertex, lines 11755-11761.

**`src/cartridges/the_board/realization/world.wgsl:11762`**

```
                cactusg_write_vertex(vb_base + vi,
                    vx, vy, vz,
                    rx * ca + fx * sa, fy * sa, rz * ca + fz * sa,
                    cr, cg, cb, slot);
```

ITEM 5 — the cactusg_write_vertex call inside the arm RING loop, lines 11762-11765. LITERAL NORMAL ARGUMENTS: nx = 'rx * ca + fx * sa', ny = 'fy * sa', nz = 'rz * ca + fz * sa'. This is the transformed world normal: the same (r, f) basis used for the position at lines 11746-11748, minus the ring centre (apx, apy, apz) and minus the radius factor r.

**`src/cartridges/the_board/realization/world.wgsl:11746`**

```
                let vx = apx + (rx * ca + fx * sa) * r;
                let vy = apy + fy * sa * r;
                let vz = apz + (rz * ca + fz * sa) * r;
```

ITEM 5 — arm ring POSITION, lines 11746-11748, shown for the basis comparison: the normal triple at 11764 is exactly these three expressions with 'ap*' and '* r' stripped.

**`src/cartridges/the_board/realization/world.wgsl:11628`**

```
            cactusg_write_vertex(vb_base + vi,
                cx + lx + ca * r, y, cz + lz + sa * r,
                ca, 0.0, sa, cr, cg, cb, slot);
```

ITEM 5 — the TRUNK's own cactusg_write_vertex, lines 11628-11630. LITERAL NORMAL ARGUMENTS: 'ca, 0.0, sa'. CONVENTION: UNTRANSFORMED LOCAL RING PARAMETER. ca and sa are declared at lines 11620-11621 as 'let ca = cos(angle);' / 'let sa = sin(angle);' where angle is the ring parameter from line 11615; they are passed straight through with no basis transform, no rib_mod correction, no taper/cap_scale slope term and no lean term. It coincides with the world normal only because the trunk's rings are horizontal circles about a vertical axis, which is what the arm-side comment at 11756-11758 states.

**`src/cartridges/the_board/realization/world.wgsl:11694`**

```
        let lean_at_fork = p.lean * p.height * fork_frac * fork_frac;
```

ITEM 6 — the arm's lean magnitude, quadratic in fork_frac, matching the trunk's t*t profile. Needed to read the arm lean coefficient off lines 11695-11696.

**`src/cartridges/the_board/realization/world.wgsl:11695`**

```
        let fork_x = cx + cos(arm_az) * p.radius * p.taper * 0.9 + lean_at_fork * lean_cos * 0.3;
```

ITEM 6 — fork_x. ARM LEAN COEFFICIENT = 0.3 (literal, trailing '* 0.3' on the lean_at_fork term). Radial offset coefficient is a separate literal 0.9.

**`src/cartridges/the_board/realization/world.wgsl:11696`**

```
        let fork_z = cz + sin(arm_az) * p.radius * p.taper * 0.9 + lean_at_fork * lean_sin * 0.3;
```

ITEM 6 — fork_z. Same literal 0.3 lean coefficient, same literal 0.9 radial coefficient.

**`src/cartridges/the_board/realization/world.wgsl:11608`**

```
        let lean_mag = p.lean * p.height * t * t;
        let lx = lean_mag * lean_cos;
        let lz = lean_mag * lean_sin;
```

ITEM 6 — the TRUNK's lean_mag / lx / lz, lines 11608-11610. TRUNK LEAN COEFFICIENT = none written, i.e. literally 1.0: lx and lz are lean_mag * lean_cos / lean_sin with no scalar multiplier. BOTH COEFFICIENTS: trunk 1.0 (implicit, no literal present), arm fork 0.3 (explicit literal). Both use the same p.lean * p.height * (fraction)^2 form, so at equal height fraction the arm fork is displaced to 30% of the trunk surface's lean displacement.

**`src/cartridges/the_board/realization/world.wgsl:11793`**

```
        let arm_cap_r = arm_r * 0.6;
        let arm_cap_tip = vi;
        cactusg_write_vertex(vb_base + vi,
            apx, apy + arm_cap_r * 0.6, apz,
            0.0, 1.0, 0.0,
            cap_col_r, cap_col_g, cap_col_b, slot);
        vi++;

        let arm_cap_ring = vi;
        let arm_cap_segs = min(arm_around, 8u);
        for (var seg = 0u; seg < arm_cap_segs; seg++) {
            let angle = f32(seg) / f32(arm_cap_segs) * 2.0 * PI;
            cactusg_write_vertex(vb_base + vi,
                apx + cos(angle) * arm_cap_r, apy, apz + sin(angle) * arm_cap_r,
                0.0, 1.0, 0.0,
                cap_col_r, cap_col_g, cap_col_b, slot);
            vi++;
        }

        for (var seg = 0u; seg < arm_cap_segs; seg++) {
            let next = (seg + 1u) % arm_cap_segs;
            cactusg_indices[ib_base + ii] = vb_base + arm_cap_tip; ii++;
            cactusg_indices[ib_base + ii] = vb_base + arm_cap_ring + seg; ii++;
            cactusg_indices[ib_base + ii] = vb_base + arm_cap_ring + next; ii++;
        }
```

ITEM 7 — the arm cap block in full, from 'let arm_cap_r' (11793) through the closing brace of the cap index loop (11817). The preceding line 11792 is the comment '        // Arm cap'. Note verbatim: the cap writes its OWN new ring (arm_cap_ring, lines 11801-11810) and the index loop references only arm_cap_tip and arm_cap_ring — the expression arm_vi_start + arm_segs_u * arm_around appears nowhere in the cap block.

**`src/cartridges/the_board/realization/world.wgsl:11706`**

```
        let arm_vi_start = vi;
```

ITEM 7 — arm_vi_start captured before the body ring loop at 11708 ('for (var s = 0u; s <= arm_segs_u; s++)', inclusive upper bound), which writes arm_around vertices per s (loop at 11737). LAST BODY RING'S FIRST VERTEX = arm_vi_start + arm_segs_u * arm_around.

**`src/cartridges/the_board/realization/world.wgsl:11780`**

```
                let row0 = arm_vi_start + s * arm_around;
                let row1 = arm_vi_start + (s + 1u) * arm_around;
```

ITEM 7 — corroboration of the index expression. The arm index loop at 11777 runs s < arm_segs_u, so its maximum row1 is arm_vi_start + arm_segs_u * arm_around, i.e. the last body ring's first vertex. Same relative-index convention (vb_base added at use site).

**`src/cartridges/the_board/realization/world.wgsl:11802`**

```
        let arm_cap_segs = min(arm_around, 8u);
```

ITEM 7 — declaration of arm_cap_segs.

**`src/cartridges/the_board/realization/world.wgsl:11685`**

```
    let arm_around = min(max(arm_ribs * 2u, 8u), 12u);
```

ITEM 7 — declaration of arm_around. Range 8..12 (arm_ribs declared line 11684: 'let arm_ribs = max(4u, ribs - 2u);'). So arm_cap_segs = min(arm_around, 8u) equals arm_around only when arm_around == 8; for arm_around in 9..12 the cap ring has strictly fewer segments than the body ring.

**`src/cartridges/the_board/realization/world.wgsl:11683`**

```
    let arm_segs_u = min(u32(p.arm_segs), 12u);
```

ITEM 7 — declaration of arm_segs_u.

**`src/cartridges/the_board/realization/world.wgsl:11653`**

```
    let top_ring_vi = trunk_steps * around;  // first vertex of trunk's last ring
```

ITEM 7 — the TRUNK's analogous expression, for contrast. The trunk cap comment at line 11671 reads '    // Fan from tip to trunk's existing top ring — no separate cap ring' and its fan (11672-11677) indexes 'vb_base + top_ring_vi + seg', i.e. the trunk cap stitches to the body's last ring; the arm cap does not.

**`src/cartridges/the_board/bodies/grounded.hpp:1132`**

```
    /* SAPLING */ {
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {6.8f, 0.85f},  {0.80f, 0.15f},
                   {1.20f, 0.10f}, {0.45f, 0.08f}, {0.65f, 0.10f}, {0.20f, 0.08f}, {0.20f, 0.05f},
                   {0.30f, 0.05f} }},
        0.15f, 0.15f, 0.06f, 0.06f, 12, 8
    },
```

CONFIRM SAPLING (lines 1132-1138). Read against PalmIdx (grounded.hpp:1055-1073) and PalmTierRow (1105-1113): HEIGHT[0] mu 25.2f = 25.2 OK; BARK_RINGS[5] mu 12.0f = 12 OK; FROND_COUNT[7] mu 20.0f = 20 OK; FROND_LEN[8] mu 6.8f = 6.8 OK; trailing trunk_segs 12 / frond_segs 8 OK (row tail order: color_over 0.15f, burial 0.15f, trunk_var 0.06f, frond_var 0.06f, trunk_segs, frond_segs).

**`src/cartridges/the_board/bodies/grounded.hpp:1139`**

```
    /* COASTAL */ {
        { 0.35f, 0.0f, { {28.8f, 3.6f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {6.8f, 1.28f},  {1.00f, 0.20f},
                   {0.65f, 0.12f}, {0.45f, 0.20f}, {0.95f, 0.12f}, {0.60f, 0.10f}, {0.30f, 0.08f},
                   {0.40f, 0.08f} }},
        0.20f, 0.15f, 0.06f, 0.06f, 16, 10
    },
```

CONFIRM COASTAL (lines 1139-1145). HEIGHT 28.8f OK; BARK_RINGS 20.0f OK; FROND_COUNT 24.0f OK; FROND_LEN 6.8f OK; trunk_segs 16 / frond_segs 10 OK.

**`src/cartridges/the_board/bodies/grounded.hpp:1146`**

```
    /* ROYAL   */ {
        { 0.20f, 0.0f, { {37.8f, 7.2f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {8.8f, 2.0f},   {1.40f, 0.35f},
                   {0.75f, 0.22f}, {0.40f, 0.12f}, {0.85f, 0.10f}, {0.50f, 0.12f}, {0.50f, 0.10f},
                   {0.50f, 0.10f} }},
        0.25f, 0.20f, 0.08f, 0.08f, 20, 12
    },
```

CONFIRM ROYAL (lines 1146-1152). HEIGHT 37.8f OK; BARK_RINGS 30.0f OK; FROND_COUNT 30.0f OK; FROND_LEN 8.8f OK; trunk_segs 20 / frond_segs 12 OK.

---

## Items 8–10 — PalmProp, CPU/GPU slot pairs, blade spawn chain

**Scope as run:** ITEM 8, ITEM 9, ITEM 10 (repo /home/user/7T-Pawns, branch claude/flora-1, HEAD 734e8c2, working tree clean)

### Answer

ITEM 8 — PalmProp, /home/user/7T-Pawns/src/cartridges/the_board/bodies/grounded.hpp lines 312-338. Full struct quoted verbatim in findings (25 members).\n- Does any member equal 931u? NO. 931u is not present anywhere in PalmProp. 931u exists in the repo at exactly three places: grounded.hpp:253 `static constexpr uint32_t SOLID_HEIGHT = 931u;` (AntennaProp, struct opens line 237), gol_zones.hpp:88 (GoL TICK_PERIOD), world.wgsl:6416 (ZONE_PROP_TICK_PERIOD). Searched: grep for "931" in grounded.hpp and "931u" across all of src/.\n- Lowest palm-band property index: 950u (PalmProp::SPAWN_ROLL, line 313). Highest: 993u (PalmProp::COLOR_VAR_B, line 337). Band is sparse — gaps at 955-959, 967-969, 977-979, 982-989.\n- Arithmetic side-note (numbers only): palm_compute_colors derives extra hash props by literal offset at grounded.hpp:1217-1219 (COLOR_VAR_R/G/B + 10u = 1001u/1002u/1003u) and 1221-1223 (+ 20u = 1011u/1012u/1013u). 1001u/1002u/1003u are byte-identical to CactusProp::POSITION_X/POSITION_Z/ROTATION (grounded.hpp:371/372/373). Declared band top is 993u; highest index actually reached by palm code is 1013u.\n\nPALM_TIERS mu verification (grounded.hpp:1131-1153) — CONFIRM, all 18 supplied values match exactly.\n- SAPLING (line 1133-1137): height 25.2f, bark_rings 12.0f, frond_count 20.0f, frond_len 6.8f, trunk_segs 12, frond_segs 8. CONFIRM.\n- COASTAL (line 1140-1144): height 28.8f, bark_rings 20.0f, frond_count 24.0f, frond_len 6.8f, trunk_segs 16, frond_segs 10. CONFIRM.\n- ROYAL (line 1147-1151): height 37.8f, bark_rings 30.0f, frond_count 30.0f, frond_len 8.8f, trunk_segs 20, frond_segs 12. CONFIRM.\n\nITEM 9 — CPU/GPU slot-count pairs. All three AGREE.\n- BLADE: Dim::MAX_BLADE_INSTANCES = 32 (realization/state.hpp:300, inside `namespace Dim {` opened at state.hpp:47) vs BLADEG_MAX_SLOTS = 32u (realization/world.wgsl:11900). AGREE.\n- PALM: Dim::MAX_PALM_INSTANCES = 24 (state.hpp:286) vs PALMG_MAX_SLOTS = 24u (world.wgsl:11216). AGREE. Binding array also 24 (world.wgsl:11218).\n- CACTUS: Dim::MAX_CACTUS_INSTANCES = 20 (state.hpp:293) vs CACTUSG_MAX_SLOTS = 20u (world.wgsl:11555). AGREE. Binding array also 20 (world.wgsl:11557).\nThe WGSL names are exactly PALMG_MAX_SLOTS / CACTUSG_MAX_SLOTS / BLADEG_MAX_SLOTS. Note: a raw `grep -n` early in this session reported stale offsets (11540/11880) for the cactus/blade constants; the correct 1-indexed lines, verified three ways (Grep tool, awk, and direct Read of the surrounding lines), are 11555 and 11900.\n\nITEM 10 — every site that multiplies blade spawn probability. There are SIX, all on one path: BLADE_TRAITS (grounded.hpp:953-964) -> blade_run_gate (grounded.hpp:970) -> gate_from_traits (spawn_engine.hpp:271) -> run_spawn_preamble (spawn_engine.hpp:203) -> compose_spawn_chance (spawn_engine.hpp:831-850). compose_spawn_chance is the ONLY composition site; grep for `adj_mod` and for `chance` multiplications across src/ returns no other multiplier.\n1. BASE — bodies/grounded.hpp:418, `static constexpr float SPAWN_CHANCE = 0.025f;` (BladeClusterConfig). BLADE literal = 0.025f.\n2. MOOD — machine/spawn_engine.hpp:834, `float adj_mod = mood_mult[c->mood_state_.active];`. BLADE column of MOOD_SPAWN_MULT (surface/population_themes.hpp:38, column index 6 per the legend at line 37). Literal = 1.0f in all four moods (lines 39, 40, 41, 42). Identity. The zero-mood veto at line 835 is dead on this path — run_spawn_preamble passes veto_on_zero_mood=false (line 229).\n3. GLOBAL DENSITY — machine/spawn_engine.hpp:836, `adj_mod *= GLOBAL_ENTITY_DENSITY;`. Literal = 1.0f (contracts/spawn_services.hpp:62). Identity.\n4. TILE/THEME SPATIAL DENSITY — surface/tile_world.hpp:492, `adj_mod *= it->second.pop.spatial_density[family];`, invoked from spawn_engine.hpp:837. For BLADE this is a bilinear blend (population_themes.hpp:298) of THEMES[t].spawn_weight[6]. Literals: TRANSITION 0.5f (line 129), MONUMENTAL 0.5f (line 144), COLONNADE 0.5f (line 159), ANTENNA 0.5f (line 174), BARREN 0.1f (line 189). Range [0.1, 0.5].\n5. ADJACENCY / PROXIMITY — machine/spawn_engine.hpp:841, `adj_mod *= proximity_affinity_boost(c, pcx, pcz, family);`, live for BLADE (use_proximity=true, line 229). Value returned at line 893, `return std::min(1.0f + weighted, PROXIMITY_MAX_BOOST[family]);`. BLADE literals: RADIUS 120.0f (line 103), MAX_BOOST 3.0f (line 104), THRESHOLD 1 (line 105), affinity row (line 118) = 0.3 per PALM, 0.3 per CACTUS, 0.5 per BLADE neighbor. Range [1.0, 3.0]. PROXIMITY_GAP_REDUCTION[6] = 0.6f (line 106) is placement spacing, not probability, so it is excluded.\n6. FINAL COMPOSE — machine/spawn_engine.hpp:843, `float chance = base_chance * adj_mod;`, then clamped by SpawnClamp::MIN1 at line 845 (a clamp, not a multiplier). Consumed at line 861 against BladeProp::SPAWN_ROLL = 1100u (grounded.hpp:424).\n\nArithmetic I did myself for BLADE, from the literals above: effective chance = 0.025 x 1.0 (mood) x 1.0 (global) x [0.1..0.5] (theme) x [1.0..3.0] (adjacency), then min(.,1.0). Nominal non-BARREN patch with no qualifying neighbors = 0.025 x 0.5 x 1.0 = 0.0125. Maximum = 0.025 x 0.5 x 3.0 = 0.0375. Minimum (pure BARREN, no neighbors) = 0.025 x 0.1 x 1.0 = 0.0025. The MIN1 clamp is unreachable for BLADE — the ceiling 0.0375 is far below 1.0.\n\nNo edits, no commits made. Working tree clean at HEAD 734e8c2.

### Quotes

**`src/cartridges/the_board/bodies/grounded.hpp:312`**

```
struct PalmProp {
    static constexpr uint32_t SPAWN_ROLL = 950u;
    static constexpr uint32_t POSITION_X = 951u;
    static constexpr uint32_t POSITION_Z = 952u;
    static constexpr uint32_t ROTATION = 953u;
    static constexpr uint32_t TIER = 954u;
    static constexpr uint32_t HEIGHT = 960u;
    static constexpr uint32_t BASE_R = 961u;
    static constexpr uint32_t TOP_R = 962u;
    static constexpr uint32_t LEAN = 963u;
    static constexpr uint32_t LEAN_DIR = 964u;
    static constexpr uint32_t BARK_RINGS = 965u;
    static constexpr uint32_t BARK_DEPTH = 966u;
    static constexpr uint32_t FROND_COUNT = 970u;
    static constexpr uint32_t FROND_LEN = 971u;
    static constexpr uint32_t FROND_WIDTH = 972u;
    static constexpr uint32_t FROND_DROOP = 973u;
    static constexpr uint32_t FROND_ARCH = 974u;
    static constexpr uint32_t CROWN_SPREAD = 975u;
    static constexpr uint32_t CROWN_SKIRT = 976u;
    static constexpr uint32_t SOLID_PADDING = 980u;
    static constexpr uint32_t EDGE_BLEND = 981u;
    static constexpr uint32_t COLOR_OVER = 990u;
    static constexpr uint32_t COLOR_VAR_R = 991u;
    static constexpr uint32_t COLOR_VAR_G = 992u;
    static constexpr uint32_t COLOR_VAR_B = 993u;
};
```

ITEM 8. FULL PalmProp struct, lines 312-338 verbatim. 25 members. No member equals 931u. Lowest palm-band index = 950u (SPAWN_ROLL); highest = 993u (COLOR_VAR_B). Band is not contiguous: gaps at 955-959, 967-969, 977-979, 982-989.

**`src/cartridges/the_board/bodies/grounded.hpp:253`**

```
    static constexpr uint32_t SOLID_HEIGHT = 931u;
```

ITEM 8 — where 931u actually lives. It is AntennaProp::SOLID_HEIGHT (struct AntennaProp opens at line 237), NOT a PalmProp member. Repo-wide 931u also appears at bodies/gol_zones.hpp:88 (`static constexpr uint32_t TICK_PERIOD = 931u;`) and realization/world.wgsl:6416 (`const ZONE_PROP_TICK_PERIOD: u32  = 931u;`). Those are the only three 931u sites in src/.

**`src/cartridges/the_board/bodies/grounded.hpp:1221`**

```
    inst.colors[6] = traits.color_parts[2].base[0] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_R + 20u) - 0.5f) * tp.trunk_var;
```

ITEM 8 side-note, arithmetic only, no interpretation: palm_compute_colors derives hash props by offset. Lines 1217-1219 use COLOR_VAR_R/G/B + 10u = 1001u/1002u/1003u; lines 1221-1223 use + 20u = 1011u/1012u/1013u. 1001u/1002u/1003u are literally CactusProp::POSITION_X/POSITION_Z/ROTATION (grounded.hpp lines 371/372/373). So the EFFECTIVE highest hash-property index reached by palm code is 1013u, above the declared band top of 993u. Reported as a number, not a verdict.

**`src/cartridges/the_board/bodies/grounded.hpp:1133`**

```
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {6.8f, 0.85f},  {0.80f, 0.15f},
                   {1.20f, 0.10f}, {0.45f, 0.08f}, {0.65f, 0.10f}, {0.20f, 0.08f}, {0.20f, 0.05f},
                   {0.30f, 0.05f} }},
        0.15f, 0.15f, 0.06f, 0.06f, 12, 8
```

PALM_TIERS SAPLING row (PALM_TIERS opens at line 1131). CONFIRM against the mu values supplied in the task: height 25.2 (25.2f), bark_rings 12 (12.0f), frond_count 20 (20.0f), frond_len 6.8 (6.8f), trunk_segs 12, frond_segs 8. All six match exactly.

**`src/cartridges/the_board/bodies/grounded.hpp:1140`**

```
        { 0.35f, 0.0f, { {28.8f, 3.6f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {6.8f, 1.28f},  {1.00f, 0.20f},
                   {0.65f, 0.12f}, {0.45f, 0.20f}, {0.95f, 0.12f}, {0.60f, 0.10f}, {0.30f, 0.08f},
                   {0.40f, 0.08f} }},
        0.20f, 0.15f, 0.06f, 0.06f, 16, 10
```

PALM_TIERS COASTAL row. CONFIRM: height 28.8 (28.8f), bark_rings 20 (20.0f), frond_count 24 (24.0f), frond_len 6.8 (6.8f), trunk_segs 16, frond_segs 10. All six match exactly.

**`src/cartridges/the_board/bodies/grounded.hpp:1147`**

```
        { 0.20f, 0.0f, { {37.8f, 7.2f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {8.8f, 2.0f},   {1.40f, 0.35f},
                   {0.75f, 0.22f}, {0.40f, 0.12f}, {0.85f, 0.10f}, {0.50f, 0.12f}, {0.50f, 0.10f},
                   {0.50f, 0.10f} }},
        0.25f, 0.20f, 0.08f, 0.08f, 20, 12
```

PALM_TIERS ROYAL row. CONFIRM: height 37.8 (37.8f), bark_rings 30 (30.0f), frond_count 30 (30.0f), frond_len 8.8 (8.8f), trunk_segs 20, frond_segs 12. All six match exactly.

**`src/cartridges/the_board/realization/state.hpp:286`**

```
            constexpr uint32_t MAX_PALM_INSTANCES = 24;
```

ITEM 9 CPU side. Inside `namespace Dim {` which opens at state.hpp:47, so this is Dim::MAX_PALM_INSTANCES = 24.

**`src/cartridges/the_board/realization/world.wgsl:11216`**

```
const PALMG_MAX_SLOTS: u32 = 24u;
```

ITEM 9 GPU side for palm. 24u vs Dim::MAX_PALM_INSTANCES 24 -> AGREE. Corroborating binding one line below at world.wgsl:11218: `@group(0) @binding(180) var<storage, read>       palmg_params: array<PalmMeshParams, 24>;` — also 24.

**`src/cartridges/the_board/realization/state.hpp:293`**

```
            constexpr uint32_t MAX_CACTUS_INSTANCES = 20;
```

ITEM 9 CPU side. Dim::MAX_CACTUS_INSTANCES = 20.

**`src/cartridges/the_board/realization/world.wgsl:11555`**

```
const CACTUSG_MAX_SLOTS: u32 = 20u;
```

ITEM 9 GPU side for cactus. 20u vs Dim::MAX_CACTUS_INSTANCES 20 -> AGREE. Corroborating binding at world.wgsl:11557: `@group(0) @binding(183) var<storage, read>       cactusg_params: array<CactusMeshParams, 20>;` — also 20.

**`src/cartridges/the_board/realization/state.hpp:300`**

```
            constexpr uint32_t MAX_BLADE_INSTANCES = 32;
```

ITEM 9 CPU side. Dim::MAX_BLADE_INSTANCES = 32.

**`src/cartridges/the_board/realization/world.wgsl:11900`**

```
const BLADEG_MAX_SLOTS: u32 = 32u;
```

ITEM 9 GPU side for blade. 32u vs Dim::MAX_BLADE_INSTANCES 32 -> AGREE. Names are exactly PALMG_MAX_SLOTS / CACTUSG_MAX_SLOTS / BLADEG_MAX_SLOTS as guessed in the task.

**`src/cartridges/the_board/bodies/grounded.hpp:418`**

```
    static constexpr float SPAWN_CHANCE = 0.025f;
```

ITEM 10, SITE 1 of 6 — the BASE. `struct BladeClusterConfig {` opens at grounded.hpp:417. Literal value for BLADE = 0.025f. Reaches the gate via BLADE_TRAITS (grounded.hpp:957: `BladeProp::SPAWN_ROLL, BladeClusterConfig::SPAWN_CHANCE,`) -> blade_run_gate (grounded.hpp:970) -> gate_from_traits (spawn_engine.hpp:271) -> run_spawn_preamble (spawn_engine.hpp:203) -> compose_spawn_chance (spawn_engine.hpp:831). Pinned by T7_GATE_PIN at machine/entity_pipeline.hpp:1150.

**`src/cartridges/the_board/machine/spawn_engine.hpp:834`**

```
    float adj_mod = mood_mult[c->mood_state_.active];
```

ITEM 10, SITE 2 of 6 — the MOOD multiplier (seed of adj_mod, not a multiply per se but the first factor). mood_mult for BLADE = mood_mult_for(PopFamily::BLADE) (grounded.hpp:958), which is the transposed column 6 of MOOD_SPAWN_MULT. Current literal for BLADE = 1.0f in ALL FOUR moods (population_themes.hpp lines 39-42, column 7 of 12 = PopFamily index 6). Table is at identity. Note the veto is inactive on this path: spawn_engine.hpp:835 `if (veto_on_zero_mood && adj_mod <= 0.0f) return { 0.0f, true };` and run_spawn_preamble passes /*veto_on_zero_mood=*/false (spawn_engine.hpp:229).

**`src/cartridges/the_board/surface/population_themes.hpp:39`**

```
    /* MOOD_OPEN_SUNSET     */ { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
```

ITEM 10, SITE 2 backing data. MOOD_SPAWN_MULT declared at population_themes.hpp:38; column legend at line 37 is `//                              pyr   arch  col   ant   palm  cact  blade sph   rib   cube  gol   gal`. BLADE is column index 6. Value 1.0f here and identically 1.0f on lines 40, 41, 42 (MOOD_INDOOR_FLAT, MOOD_INDOOR_VAULT, MOOD_FINITE_OUTDOOR). So the BLADE mood multiplier is 1.0f under every mood — identity, no effect today.

**`src/cartridges/the_board/machine/spawn_engine.hpp:836`**

```
    adj_mod *= GLOBAL_ENTITY_DENSITY;
```

ITEM 10, SITE 3 of 6 — the GLOBAL DENSITY multiply. Current literal at contracts/spawn_services.hpp:62: `inline constexpr float GLOBAL_ENTITY_DENSITY = 1.0f;`. Identity today. Family-agnostic — same 1.0f for BLADE as everyone.

**`src/cartridges/the_board/surface/tile_world.hpp:492`**

```
    adj_mod *= it->second.pop.spatial_density[family];
```

ITEM 10, SITE 4 of 6 — the TILE/THEME DENSITY multiply (the per-position spatial factor). Called from spawn_engine.hpp:837 `tile_apply_spawn_mult(c->tile_world_state_, gx, gz, family, adj_mod);  // F3: the S2 boundary face`. Function opens at tile_world.hpp:480. For BLADE, family index = 6. spatial_density is written by generate_tile_population (population_themes.hpp:298 `blended_spawn[f] += theme.spawn_weight[f] * w;`, assigned at line 303), a bilinear blend of THEMES[t].spawn_weight[6] over 4 lattice nodes. BLADE column literals across the 5 themes: TRANSITION 0.5f (line 129), MONUMENTAL 0.5f (line 144), COLONNADE 0.5f (line 159), ANTENNA 0.5f (line 174), BARREN 0.1f (line 189). So this factor ranges [0.1, 0.5] for BLADE; it is 0.5 everywhere except where BARREN blends in.

**`src/cartridges/the_board/surface/population_themes.hpp:189`**

```
    {   { 0.4f, 0.3f, 0.5f, 0.3f, 0.2f, 0.2f, 0.1f, 0.3f, 1.0f, 0.1f, 0.2f, 0.6f },
```

ITEM 10, SITE 4 backing data — the only theme where BLADE's spawn_weight is not 0.5f. This is THEMES row 4 BARREN (comment line 188: `// ── 4: BARREN — near-empty ───────────────────────────────────`). Column index 6 = 0.1f. Compare TRANSITION row at line 129 whose inline legend reads `// spawn_weight [pyr..sph, ribn, cube, gol, gall]` and whose column 6 = 0.5f.

**`src/cartridges/the_board/machine/spawn_engine.hpp:841`**

```
        adj_mod *= proximity_affinity_boost(c, pcx, pcz, family);
```

ITEM 10, SITE 5 of 6 — the ADJACENCY / PROXIMITY multiply. Gated by `if (use_proximity)` at spawn_engine.hpp:838; run_spawn_preamble passes /*use_proximity=*/true (spawn_engine.hpp:229), so it IS live for BLADE. Function body at spawn_engine.hpp:873-894; returns 1.0f early if the family's affinity row is all-zero (line 874) or radius <= 0 (line 876) or neighbor count < threshold (line 892).

**`src/cartridges/the_board/machine/spawn_engine.hpp:893`**

```
    return std::min(1.0f + weighted, PROXIMITY_MAX_BOOST[family]);
```

ITEM 10, SITE 5 — the actual adjacency value returned. For BLADE (family index 6) the governing literals are: PROXIMITY_RADIUS[6] = 120.0f (spawn_engine.hpp:103), PROXIMITY_MAX_BOOST[6] = 3.0f (line 104), PROXIMITY_THRESHOLD[6] = 1 (line 105), and the BLADE affinity row (line 118): `    /* Blad  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },` — i.e. 0.3 per nearby PALM, 0.3 per nearby CACTUS, 0.5 per nearby BLADE, within 120 wu, needing >= 1 qualifying neighbor. So this factor ranges [1.0, 3.0] for BLADE. (PROXIMITY_GAP_REDUCTION[6] = 0.6f at line 106 is placement spacing, NOT probability — excluded.)

**`src/cartridges/the_board/machine/spawn_engine.hpp:843`**

```
    float chance = base_chance * adj_mod;
```

ITEM 10, SITE 6 of 6 — the final composition multiply, base x accumulated adj_mod. Immediately followed by the clamp; for BLADE the clamp is SpawnClamp::MIN1 (passed at spawn_engine.hpp:230), i.e. spawn_engine.hpp:845 `        case SpawnClamp::MIN1:    chance = std::min(chance, 1.0f); break;`. The clamp is not a multiplier. Consumed at spawn_engine.hpp:861 `    result.passed = cpu_hash_f(result.seed, spawn_roll_prop) < chance;` with spawn_roll_prop = BladeProp::SPAWN_ROLL = 1100u (grounded.hpp:424).

---

## Item 11 — cactus and blade designer defaults vs the landed tables

**Scope as run:** ITEM 11

### Answer

ITEM 11 — designer defaults vs LANDED CACTUS_TIERS / BLADE_TIERS in /home/user/7T-Pawns/src/cartridges/the_board/bodies/grounded.hpp (branch claude/flora-1, HEAD 734e8c2). Read-only; no edits made. Mapped by NAME. Palm excluded from the diff report per instruction (PALM_TIERS mu values in the task were verified against grounded.hpp L1131-1153 and all CONFIRM).

=== DIFFERING PARAMETERS (family / tier / param / designer value / landed value) ===
Designer file for cactus rows: /home/user/7T-Pawns/src/tools/7t_cactus_designer.jsx (defaultTier, L60-118)
Designer file for blade rows: /home/user/7T-Pawns/src/tools/7t_blade_cluster_designer.jsx (defaultTier, L62-108)

cactus / FINGER / height / 4.0 / 9.0
cactus / FINGER / radius / 0.30 / 0.40
cactus / FINGER / taper / 0.70 / 0.90
cactus / FINGER / lean / 0.06 / 0.26
cactus / FINGER / arm_count / 0 / 1.0
cactus / FINGER / arm_count_s / 0 / 1.0
cactus / FINGER / arm_length / 1.5 / 2.0
cactus / FINGER / arm_radius / 0.18 / 0.29
cactus / FINGER / arm_curve / 0.60 / 1.0
cactus / FINGER / arm_curve_s / 0.15 / 0.30
cactus / SAGUARO / height / 10.0 / 13.0
cactus / SAGUARO / arm_count / 1 / 2.0
cactus / SAGUARO / arm_count_s / 0.5 / 1.0
cactus / SAGUARO / arm_length / 3.5 / 4.5
cactus / SAGUARO / arm_radius / 0.30 / 0.40
cactus / SAGUARO / arm_curve / 0.70 / 1.0
cactus / CANDELABRA / height / 18.0 / 20.0
cactus / CANDELABRA / arm_count / 3 / 4.0
cactus / CANDELABRA / arm_length / 5.0 / 7.0
cactus / CANDELABRA / arm_radius / 0.35 / 0.50
cactus / CANDELABRA / arm_curve / 0.75 / 1.0
blade / SPROUT / blade_count_s / 0.5 / 1.0
blade / SPROUT / blade_h / 1.8 / 3.0
blade / CLUMP / blade_count_s / 1 / 2.0
blade / CLUMP / blade_h / 3.2 / 3.80
blade / THICKET / blade_count / 6 / 7.0
blade / THICKET / blade_count_s / 1 / 3.0
blade / THICKET / blade_h / 5.5 / 5.20

Total: 21 cactus diffs, 7 blade diffs, 28 lines.

=== PARAMETERS CHECKED AND IDENTICAL (no diff) ===
cactus: height_s, radius_s, taper_s, ribs, ribs_s, rib_depth, rib_depth_s, lean_s, cap_round, cap_round_s, arm_height, arm_height_s, arm_length_s, arm_radius_s, color_over, color_var, trunk_segs, arm_segs, weight — all three tiers, except the sigma diffs listed above (FINGER arm_count_s, FINGER arm_curve_s, SAGUARO arm_count_s).
blade: blade_h_var, blade_h_var_s, blade_w, blade_w_s, splay, splay_s, curve, curve_s, twist, twist_s, taper, taper_s, blade_h_s, color_over, color_var, blade_segs, weight — all three tiers; blade_count mu matches on SPROUT and CLUMP.

=== LEAN_DIR PLACEHOLDER ===
Confirmed: grounded.hpp CactusIdx::LEAN_DIR = 6 (L1305) occupies a {0.0f, 0.0f} slot in every cactus row (L1362, L1368, L1374), sampled ParamDist::UNIFORM_TAU (L1322). The cactus designer has NO lean_dir field — grep for "lean_dir|LEAN_DIR" over /home/user/7T-Pawns/src/tools/7t_cactus_designer.jsx returns none. It stores/emits 12 mu-sigma pairs against CactusIdx::COUNT == 13. BLADE_TIERS has no such placeholder (BladeIdx::COUNT == 8, designer stores 8) — none.

=== genCpp ROW SHAPE VERDICTS ===
CACTUS DESIGNER (/home/user/7T-Pawns/src/tools/7t_cactus_designer.jsx, genCpp L424-444): does NOT emit the tree's row shape. NEEDS THE SAME FIX PALM IS GETTING. Four defects: (1) weight is emitted LAST — L439 `vals.push(String(T.trunk_segs), String(T.arm_segs), f(T.weight));` — while the landed row puts weight FIRST inside TierProfile (entity_types.hpp L111-115: weight, color_var, params[]); (2) flat value list, no nested `{ { weight, color_var, { pairs } }, color_over, trunk_segs, arm_segs }` bracing and no per-pair `{mu, sigma}` braces; (3) no LEAN_DIR `{0.0f, 0.0f}` slot, so the emitted row is one pair short of CactusIdx::COUNT; (4) wrong struct name — L428 emits `CactusTierParams` but the landed type is `CactusTierRow` (grounded.hpp L1338/L1359).

BLADE DESIGNER (/home/user/7T-Pawns/src/tools/7t_blade_cluster_designer.jsx, genCpp L403-423): does NOT emit the tree's row shape. NEEDS THE SAME FIX PALM IS GETTING. Three defects: (1) weight is emitted LAST — L418 `vals.push(String(T.blade_segs), f(T.weight));` — landed puts weight FIRST; (2) flat value list with color_over and color_var emitted adjacent mid-row (L416) instead of color_var inside TierProfile and color_over after it, and no nested/pair bracing; (3) wrong struct name — L409 emits `BladeClusterTierParams` but the landed type is `BladeTierRow` (grounded.hpp L899/L916). Blade needs no placeholder slot (param count already matches).

For reference, the palm designer has the identical defect at /home/user/7T-Pawns/src/tools/7t_palm_designer.jsx L523 (`PalmTierParams`) and L535 (`vals.push(String(T.trunk_segs), String(T.frond_segs), f(T.weight));`) — same weight-last, flat, wrong-struct-name pattern.

=== OUT-OF-SCOPE NOTE ===
Both designers also store per-tier color arrays (cactus body_color/rib_color L78-79, L96-97, L114-115; blade blade_color/blade_color_aged L76-77, L90-91, L104-105). These have NO name-mapped counterpart inside CACTUS_TIERS / BLADE_TIERS — the landed bases live in CACTUS_COLOR_PARTS (grounded.hpp L1386-1389) and BLADE_COLOR_PARTS (L942-949) as single non-per-tier values, so they are not reported as differing tier-table parameters.

### Quotes

**`src/tools/7t_cactus_designer.jsx:63`**

```
    { height: 4.0, height_s: 1.0,
```

cactus / FINGER / height (mu) / 4.0 / 9.0  — landed grounded.hpp:1361 "        { 0.45f, 0.06f, { {9.0f, 1.0f},   {0.40f, 0.06f}, {0.90f, 0.08f}, {8.0f, 1.0f},   {0.12f, 0.03f},". height_s 1.0 == 1.0f, no diff.

**`src/tools/7t_cactus_designer.jsx:64`**

```
      radius: 0.30, radius_s: 0.06,
```

cactus / FINGER / radius (mu) / 0.30 / 0.40  — landed grounded.hpp:1361 second pair {0.40f, 0.06f}. radius_s 0.06 == 0.06f, no diff.

**`src/tools/7t_cactus_designer.jsx:65`**

```
      taper: 0.70, taper_s: 0.08,
```

cactus / FINGER / taper (mu) / 0.70 / 0.90  — landed grounded.hpp:1361 third pair {0.90f, 0.08f}. taper_s 0.08 == 0.08f, no diff.

**`src/tools/7t_cactus_designer.jsx:68`**

```
      lean: 0.06, lean_s: 0.03,
```

cactus / FINGER / lean (mu) / 0.06 / 0.26  — landed grounded.hpp:1362 "                   {0.26f, 0.03f}, {0.0f, 0.0f},   {0.60f, 0.10f}, {1.0f, 1.0f},". lean_s 0.03 == 0.03f, no diff.

**`src/tools/7t_cactus_designer.jsx:70`**

```
      arm_count: 0, arm_count_s: 0,
```

cactus / FINGER / arm_count (mu) / 0 / 1.0  — landed grounded.hpp:1362 fourth pair {1.0f, 1.0f}.

**`src/tools/7t_cactus_designer.jsx:70`**

```
      arm_count: 0, arm_count_s: 0,
```

cactus / FINGER / arm_count_s (sigma) / 0 / 1.0  — landed grounded.hpp:1362 fourth pair {1.0f, 1.0f}.

**`src/tools/7t_cactus_designer.jsx:72`**

```
      arm_length: 1.5, arm_length_s: 0.3,
```

cactus / FINGER / arm_length (mu) / 1.5 / 2.0  — landed grounded.hpp:1363 "                   {0.40f, 0.10f}, {2.0f, 0.30f},  {0.29f, 0.03f}, {1.0f, 0.30f} }},". arm_length_s 0.3 == 0.30f, no diff.

**`src/tools/7t_cactus_designer.jsx:73`**

```
      arm_radius: 0.18, arm_radius_s: 0.03,
```

cactus / FINGER / arm_radius (mu) / 0.18 / 0.29  — landed grounded.hpp:1363 third pair {0.29f, 0.03f}. arm_radius_s 0.03 == 0.03f, no diff.

**`src/tools/7t_cactus_designer.jsx:74`**

```
      arm_curve: 0.60, arm_curve_s: 0.15,
```

cactus / FINGER / arm_curve (mu) / 0.60 / 1.0  — landed grounded.hpp:1363 fourth pair {1.0f, 0.30f}.

**`src/tools/7t_cactus_designer.jsx:74`**

```
      arm_curve: 0.60, arm_curve_s: 0.15,
```

cactus / FINGER / arm_curve_s (sigma) / 0.15 / 0.30  — landed grounded.hpp:1363 fourth pair {1.0f, 0.30f}.

**`src/tools/7t_cactus_designer.jsx:81`**

```
    { height: 10.0, height_s: 2.5,
```

cactus / SAGUARO / height (mu) / 10.0 / 13.0  — landed grounded.hpp:1367 "        { 0.35f, 0.06f, { {13.0f, 2.5f},  {0.55f, 0.10f}, {0.75f, 0.06f}, {12.0f, 2.0f},  {0.15f, 0.04f},". height_s 2.5 == 2.5f, no diff.

**`src/tools/7t_cactus_designer.jsx:88`**

```
      arm_count: 1, arm_count_s: 0.5,
```

cactus / SAGUARO / arm_count (mu) / 1 / 2.0  — landed grounded.hpp:1368 "                   {0.04f, 0.02f}, {0.0f, 0.0f},   {0.65f, 0.10f}, {2.0f, 1.0f},".

**`src/tools/7t_cactus_designer.jsx:88`**

```
      arm_count: 1, arm_count_s: 0.5,
```

cactus / SAGUARO / arm_count_s (sigma) / 0.5 / 1.0  — landed grounded.hpp:1368 fourth pair {2.0f, 1.0f}.

**`src/tools/7t_cactus_designer.jsx:90`**

```
      arm_length: 3.5, arm_length_s: 0.8,
```

cactus / SAGUARO / arm_length (mu) / 3.5 / 4.5  — landed grounded.hpp:1369 "                   {0.45f, 0.12f}, {4.5f, 0.80f},  {0.40f, 0.06f}, {1.0f, 0.15f} }},". arm_length_s 0.8 == 0.80f, no diff.

**`src/tools/7t_cactus_designer.jsx:91`**

```
      arm_radius: 0.30, arm_radius_s: 0.06,
```

cactus / SAGUARO / arm_radius (mu) / 0.30 / 0.40  — landed grounded.hpp:1369 third pair {0.40f, 0.06f}. arm_radius_s 0.06 == 0.06f, no diff.

**`src/tools/7t_cactus_designer.jsx:92`**

```
      arm_curve: 0.70, arm_curve_s: 0.15,
```

cactus / SAGUARO / arm_curve (mu) / 0.70 / 1.0  — landed grounded.hpp:1369 fourth pair {1.0f, 0.15f}. arm_curve_s 0.15 == 0.15f, no diff.

**`src/tools/7t_cactus_designer.jsx:99`**

```
    { height: 18.0, height_s: 4.0,
```

cactus / CANDELABRA / height (mu) / 18.0 / 20.0  — landed grounded.hpp:1373 "        { 0.20f, 0.08f, { {20.0f, 4.0f},  {0.75f, 0.12f}, {0.80f, 0.05f}, {14.0f, 2.0f},  {0.18f, 0.04f},". height_s 4.0 == 4.0f, no diff.

**`src/tools/7t_cactus_designer.jsx:106`**

```
      arm_count: 3, arm_count_s: 1,
```

cactus / CANDELABRA / arm_count (mu) / 3 / 4.0  — landed grounded.hpp:1374 "                   {0.03f, 0.02f}, {0.0f, 0.0f},   {0.70f, 0.10f}, {4.0f, 1.0f},". arm_count_s 1 == 1.0f, no diff.

**`src/tools/7t_cactus_designer.jsx:108`**

```
      arm_length: 5.0, arm_length_s: 1.2,
```

cactus / CANDELABRA / arm_length (mu) / 5.0 / 7.0  — landed grounded.hpp:1375 "                   {0.40f, 0.10f}, {7.0f, 1.20f},  {0.50f, 0.06f}, {1.0f, 0.12f} }},". arm_length_s 1.2 == 1.20f, no diff.

**`src/tools/7t_cactus_designer.jsx:109`**

```
      arm_radius: 0.35, arm_radius_s: 0.06,
```

cactus / CANDELABRA / arm_radius (mu) / 0.35 / 0.50  — landed grounded.hpp:1375 third pair {0.50f, 0.06f}. arm_radius_s 0.06 == 0.06f, no diff.

**`src/tools/7t_cactus_designer.jsx:110`**

```
      arm_curve: 0.75, arm_curve_s: 0.12,
```

cactus / CANDELABRA / arm_curve (mu) / 0.75 / 1.0  — landed grounded.hpp:1375 fourth pair {1.0f, 0.12f}. arm_curve_s 0.12 == 0.12f, no diff.

**`src/tools/7t_blade_cluster_designer.jsx:65`**

```
    { blade_count: 3, blade_count_s: 0.5,
```

blade / SPROUT / blade_count_s (sigma) / 0.5 / 1.0  — landed grounded.hpp:918 "        { 0.50f, 0.06f, { {3.0f, 1.0f}, {3.0f, 0.40f}, {0.35f, 0.08f}, {0.30f, 0.06f},". blade_count mu 3 == 3.0f, no diff.

**`src/tools/7t_blade_cluster_designer.jsx:66`**

```
      blade_h: 1.8, blade_h_s: 0.4,
```

blade / SPROUT / blade_h (mu) / 1.8 / 3.0  — landed grounded.hpp:918 second pair {3.0f, 0.40f}. blade_h_s 0.4 == 0.40f, no diff.

**`src/tools/7t_blade_cluster_designer.jsx:79`**

```
    { blade_count: 5, blade_count_s: 1,
```

blade / CLUMP / blade_count_s (sigma) / 1 / 2.0  — landed grounded.hpp:923 "        { 0.35f, 0.06f, { {5.0f, 2.0f}, {3.80f, 0.60f}, {0.40f, 0.10f}, {0.45f, 0.08f},". blade_count mu 5 == 5.0f, no diff.

**`src/tools/7t_blade_cluster_designer.jsx:80`**

```
      blade_h: 3.2, blade_h_s: 0.6,
```

blade / CLUMP / blade_h (mu) / 3.2 / 3.80  — landed grounded.hpp:923 second pair {3.80f, 0.60f}. blade_h_s 0.6 == 0.60f, no diff.

**`src/tools/7t_blade_cluster_designer.jsx:93`**

```
    { blade_count: 6, blade_count_s: 1,
```

blade / THICKET / blade_count (mu) / 6 / 7.0  — landed grounded.hpp:928 "        { 0.15f, 0.08f, { {7.0f, 3.0f}, {5.20f, 1.20f}, {0.45f, 0.10f}, {0.55f, 0.10f},".

**`src/tools/7t_blade_cluster_designer.jsx:93`**

```
    { blade_count: 6, blade_count_s: 1,
```

blade / THICKET / blade_count_s (sigma) / 1 / 3.0  — landed grounded.hpp:928 first pair {7.0f, 3.0f}.

**`src/tools/7t_blade_cluster_designer.jsx:94`**

```
      blade_h: 5.5, blade_h_s: 1.2,
```

blade / THICKET / blade_h (mu) / 5.5 / 5.20  — landed grounded.hpp:928 second pair {5.20f, 1.20f}. blade_h_s 1.2 == 1.20f, no diff.

**`src/tools/7t_cactus_designer.jsx:428`**

```
  lines.push("static constexpr CactusTierParams CACTUS_TIERS[] = {");
```

genCpp SHAPE: cactus designer emits struct name CactusTierParams; landed struct is CactusTierRow (grounded.hpp:1338 "struct CactusTierRow {", array at 1359 "inline constexpr CactusTierRow CACTUS_TIERS[] = {"). NEEDS THE SAME FIX AS PALM.

**`src/tools/7t_cactus_designer.jsx:439`**

```
    vals.push(String(T.trunk_segs), String(T.arm_segs), f(T.weight));
```

genCpp SHAPE: cactus designer emits weight LAST, flat, no nested TierProfile braces, no {mu,sigma} pair braces, and NO LEAN_DIR {0,0} placeholder slot (emits 12 mu/sigma pairs, landed CactusIdx::COUNT == 13). Landed order is weight FIRST: grounded.hpp:1360-1364 row = { { weight, color_var, { 13 pairs } }, color_over, trunk_segs, arm_segs }. VERDICT: does NOT emit the tree's row shape; needs the same fix palm is getting.

**`src/tools/7t_blade_cluster_designer.jsx:409`**

```
  lines.push("static constexpr BladeClusterTierParams BLADE_TIERS[] = {");
```

genCpp SHAPE: blade designer emits struct name BladeClusterTierParams; landed struct is BladeTierRow (grounded.hpp:899 "struct BladeTierRow {", array at 916 "inline constexpr BladeTierRow BLADE_TIERS[] = {"). NEEDS THE SAME FIX AS PALM.

**`src/tools/7t_blade_cluster_designer.jsx:418`**

```
    vals.push(String(T.blade_segs), f(T.weight));
```

genCpp SHAPE: blade designer emits weight LAST, flat, no nested TierProfile braces, no {mu,sigma} pair braces, and color_over/color_var emitted mid-row after taper_s instead of color_var-inside-profile / color_over-after-profile. Landed order is weight FIRST: grounded.hpp:917-921 row = { { weight, color_var, { 8 pairs } }, color_over, blade_segs }. VERDICT: does NOT emit the tree's row shape; needs the same fix palm is getting. (Blade has no LEAN_DIR placeholder — 8 params both sides.)

**`src/cartridges/the_board/bodies/grounded.hpp:1133`**

```
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
```

PALM_TIERS post-F0 mu values VERIFIED as stated in the task (out of scope for the diff report, verification only). SAPLING height 25.2 (L1133), bark_rings 12.0 / frond_count 20.0 / frond_len 6.8 (L1134), trunk_segs 12 frond_segs 8 (L1137). COASTAL height 28.8 (L1140), bark_rings 20.0 / frond_count 24.0 / frond_len 6.8 (L1141), trunk_segs 16 frond_segs 10 (L1144). ROYAL height 37.8 (L1147), bark_rings 30.0 / frond_count 30.0 / frond_len 8.8 (L1148), trunk_segs 20 frond_segs 12 (L1151). All match — CONFIRM.

---

## Item 12 — the bark groove term

**Scope as run:** ITEM 12 — world.wgsl palm_mesh_gen TRUNK loop: ring_phase / bark_rings degeneracy

### Answer

VERDICT: **CONFIRM, with one exact counterexample inside the claim's own escape clause.** The core reduction holds. The clause "except when a clamp bites" is very slightly too generous — bark_rings == 4 is a clamp-bite that is STILL identically zero.

=====================================================================
1. THE FOUR REQUESTED LINES, QUOTED TOGETHER
=====================================================================
All from /home/user/7T-Pawns/src/cartridges/the_board/realization/world.wgsl,
inside `fn palm_mesh_gen` (declared at line 11239):

  11265:     let trunk_rings = min(u32(max(8.0, p.bark_rings)), 40u);
  11268:     for (var ring = 0u; ring <= trunk_rings; ring++) {
  11269:         let t = f32(ring) / f32(trunk_rings);
  11272:         let ring_phase = t * p.bark_rings * 2.0 * PI;
  11273:         r += sin(ring_phase) * p.bark_depth * (1.0 - t * 0.5);

PI is /home/user/7T-Pawns/src/cartridges/the_board/realization/world.wgsl:140
  `const PI: f32 = 3.14159265359;`

=====================================================================
2. HOW ring_phase IS ACTUALLY BUILT (the thing to be careful about)
=====================================================================
`ring_phase = t * p.bark_rings * 2.0 * PI` — WGSL `*` is left-associative,
so this is `((t * p.bark_rings) * 2.0) * PI`.

The load-bearing asymmetry: the MULTIPLIER on line 11272 is **p.bark_rings**
(the raw f32 param), while the DIVISOR on line 11269 is **trunk_rings** (the
clamped u32). They are two different quantities that happen to coincide over
most of the domain. This is exactly the detail the claim hinges on, and the
claim reads it correctly.

=====================================================================
3. ALGEBRAIC REDUCTION
=====================================================================
Let B = p.bark_rings, N = trunk_rings.

(a) B is integral and B >= 3.
    entity_pipeline.hpp:141 `if (pd.do_round) val = std::round(val);`
    entity_pipeline.hpp:142 `val = std::max(pd.floor, val);`
    with grounded.hpp:1081 do_round=true, floor=3.0f, ceiling=1e30f (none).
    Not touched by indoor rescale (grounded.hpp:1189). Copied verbatim to the
    GPU at grounded.hpp:1248. So B in {3,4,5,...} exactly.

(b) N = min(u32(max(8.0, B)), 40u). For an integral non-negative B the u32()
    truncation is a no-op, so N == clamp(B, 8, 40). The claim's identification
    of line 11265 as clamp(bark_rings, 8, 40) is CORRECT for the reachable
    domain. (Caveat: it is NOT clamp() in general — trunc + asymmetric
    max/min — but nothing reachable exercises the difference.)

(c) t = ring / N, so
        ring_phase = (ring / N) * B * 2*PI
                   = ring * (B / N) * 2*PI.
    Everything reduces to the RATIO B/N.

(d) If 8 <= B <= 40 then N = B, the ratio is exactly 1, and
        ring_phase = ring * 2*PI
        sin(ring_phase) = 0   for every integer ring.
    REDUCTION HOLDS. bark_depth is multiplied by zero at every single ring
    and contributes nothing to r.

(e) If B < 8 then N = 8 and the ratio is B/8; if B > 40 then N = 40 and the
    ratio is B/40. sin is generally nonzero — UNLESS the ratio happens to be
    a half-integer, which is the gap in the claim (see section 5).

=====================================================================
4. NUMERIC EVALUATION, sin(ring_phase) for ring = 0..trunk_rings
    (computed in f32 with PI = 3.14159265359f, mirroring the shader)
=====================================================================
Because PI_f32 = 3.14159274101257324 != pi (error +8.742278e-08), the residual
is NOT bitwise zero: sin(ring * 2*PI_f32) ~= ring * 1.7484555e-07.

--- bark_rings = 12  (SAPLING mu) -> trunk_rings = 12, bark_depth mu 0.04 ---
ring  0: t=0            ring_phase=0            sin= 0
ring  1: t=0.0833333358 ring_phase=6.28318548   sin= 1.74845553e-07
ring  2: t=0.166666672  ring_phase=12.566371    sin= 3.49691106e-07
ring  3: t=0.25         ring_phase=18.849556    sin= 4.76995226e-08
ring  4: t=0.333333343  ring_phase=25.1327419   sin= 6.99382213e-07
ring  5: t=0.416666657  ring_phase=31.4159279   sin= 1.35106495e-06
ring  6: t=0.5          ring_phase=37.6991119   sin= 9.53990451e-08
ring  7: t=0.583333313  ring_phase=43.9822998   sin= 2.65443032e-06
ring  8: t=0.666666687  ring_phase=50.2654839   sin= 1.39876443e-06
ring  9: t=0.75         ring_phase=56.5486679   sin= 1.43098561e-07
ring 10: t=0.833333313  ring_phase=62.8318558   sin= 2.70212990e-06
ring 11: t=0.916666687  ring_phase=69.1150436   sin= 5.26116128e-06
ring 12: t=1            ring_phase=75.3982239   sin= 1.90798090e-07
MAX|sin| = 5.26116128e-06  ->  max |r delta| = 5.26e-06 * 0.04 * <=1.0
                                             = 2.10446e-07 wu

--- bark_rings = 20  (COASTAL mu) -> trunk_rings = 20, bark_depth mu 0.06 ---
ring  0..20 sin values:
 0: 0
 1: 1.74845553e-07     11: 5.26116128e-06
 2: 3.49691106e-07     12: 1.90798090e-07
 3: 4.76995226e-08     13: 2.74982949e-06
 4: 6.99382213e-07     14: 5.30886064e-06
 5: 1.35106495e-06     15: 2.38497620e-07
 6: 9.53990451e-08     16: 2.79752885e-06
 7: 2.65443032e-06     17: 5.35656045e-06
 8: 1.39876443e-06     18: 2.86197121e-07
 9: 1.43098561e-07     19: 2.84522844e-06
10: 2.70212990e-06     20: 5.40425981e-06
MAX|sin| = 5.40425981e-06  ->  max |r delta| = 3.24256e-07 wu

--- bark_rings = 30  (ROYAL mu) -> trunk_rings = 30, bark_depth mu 0.08 ---
ring  0..30 sin values:
 0: 0                  11: 5.26116128e-06     21:  7.96329095e-06
 1: 1.74845553e-07     12: 1.90798090e-07     22:  1.05223226e-05
 2: 3.49691106e-07     13: 2.74982949e-06     23: -2.17743514e-06
 3: 4.76995226e-08     14: 5.30886064e-06     24:  3.81596180e-07
 4: 6.99382213e-07     15: 2.38497620e-07     25:  2.94062761e-06
 5: 1.35106495e-06     16: 2.79752885e-06     26:  5.49965898e-06
 6: 9.53990451e-08     17: 5.35656045e-06     27:  8.05869058e-06
 7: 2.65443032e-06     18: 2.86197121e-07     28:  1.06177213e-05
 8: 1.39876443e-06     19: 2.84522844e-06     29: -2.08203619e-06
 9: 1.43098561e-07     20: 5.40425981e-06     30:  4.76995240e-07
10: 2.70212990e-06
MAX|sin| = 1.06177213e-05  ->  max |r delta| = 8.49418e-07 wu
(the sign flips at ring 23/29 are libm argument-reduction artifacts on a
 large argument, not structure — magnitude is still ~1e-05.)

SUMMARY: at all three shipped mu, sin(ring_phase) is zero to within 1.1e-05,
and the resulting radial displacement is <= 8.5e-07 wu against base_r of
0.55 / 0.55 / 0.90. That is roughly 14 ulp of r. Visually and geometrically
it is exactly nothing. The bark rings do not exist at the tier means.

=====================================================================
5. THE COUNTEREXAMPLE INSIDE THE ESCAPE CLAUSE — say it loudly
=====================================================================
The claim says the degeneracy lifts "when a clamp bites (bark_rings < 8 or
> 40)". That is ALMOST right. What actually matters is whether B/N is an
INTEGER *or a half-integer* — a half-integer ratio gives sin(ring * k * pi)
= 0 as well.

**bark_rings == 4 is a clamp-bite that is STILL identically zero.**
  B=4 -> N=8 -> ratio = 1/2 -> ring_phase = ring * pi -> sin = 0 for all ring.
  Measured MAX|sin| over ring=0..8 = 1.32721516e-06. Dead.

Full enumeration over every reachable bark_rings value (z-clamped to +/-3 by
seed_utils.hpp:69, then rounded, then floored at 3):

  B :  3  4  5  6  7 | 8 ... 40 | 41 42 43 44 45
  N :  8  8  8  8  8 | B  ... B  | 40 40 40 40 40
  sin: NZ ZERO NZ NZ NZ | ZERO ... ZERO | NZ NZ NZ NZ NZ

  MAX|sin| for the NONZERO cases: 1.0 (B=3,5,6,7,41,42,43,45), 0.951061487
  (B=44). i.e. full-amplitude bark, r delta up to +/- bark_depth.

  Every B in [8,40] inclusive -- 33 of the 43 reachable values -- is ZERO
  (MAX|sin| between 2.65e-06 and 2.32e-05).

Example of a live clamp-bite, B=5, N=8, bark_depth=0.04:
  ring 0: t=0      rp=0           sin= 0             r delta= 0
  ring 1: t=0.125  rp=3.92699099  sin=-0.707106888   r delta=-0.0265165064
  ring 2: t=0.25   rp=7.85398197  sin= 1             r delta= 0.0350000001
  ring 3: t=0.375  rp=11.7809725  sin=-0.707106769   r delta=-0.0229809694
  ring 4: t=0.5    rp=15.7079639  sin=-6.75532476e-07 r delta=-2.02659738e-08
  ring 5: t=0.625  rp=19.6349545  sin= 0.707107067   r delta= 0.0194454435
  ring 6: t=0.75   rp=23.561945   sin=-1             r delta=-0.0249999985
  ring 8: t=1      rp=31.4159279  sin= 1.35106495e-06 r delta= 2.7021299e-08
  -> +/-0.04 wu on a base_r of 0.55: ~7% radial wobble, plainly visible.

Example of a high-side clamp-bite, B=45, N=40, bark_depth=0.08:
  ring 1: rp=7.06858349  sin= 0.707106769  r delta= 0.0558614358
  ring 2: rp=14.137167   sin= 1            r delta= 0.0780000016
  ring 6: rp=42.4115067  sin=-1            r delta=-0.0740000010
  -> +/-0.08 wu on a base_r of 0.90: ~9% radial wobble.

=====================================================================
6. PER-TIER CONSEQUENCE (from the z-clamp at seed_utils.hpp:69)
=====================================================================
Reachable bark_rings range per tier = round(mu +/- 3*sigma), floored at 3:

  SAPLING  mu 12 sigma 3  -> B in [3, 21].  Bark ALIVE only for
                             B in {3,5,6,7}.  B=4 is dead.
  COASTAL  mu 20 sigma 4  -> B in [8, 32].  **Bark is dead for EVERY
                             reachable value. 100% dead. No clamp can bite.**
  ROYAL    mu 30 sigma 5  -> B in [15, 45]. Bark ALIVE only for
                             B in {41,...,45}.

Approximate incidence, assuming the Box-Muller z (seed_utils.hpp:65-70) is
ideally normal before its +/-3 clamp — treat as indicative, not exact, since
cpu_hash_f's uniformity is unverified here:
  SAPLING: P(bark visible) ~ 6.3%   (P(B=3)~0.23%, B=5~0.89%, B=6~1.8%, B=7~3.3%)
  COASTAL: P(bark visible) = 0%     (exact, structural — not a probability)
  ROYAL:   P(bark visible) ~ 1.8%   (z >= 2.1)

So BARK_DEPTH ({0.04,0.01} / {0.06,0.01} / {0.08,0.02}) is a sampled,
stored, uploaded, per-tier-authored parameter that is multiplied by zero
for roughly 97% of all palms, and for 100% of COASTAL palms. When it does
fire, it fires at FULL amplitude with a phase that has nothing to do with
the authored ring count — it renders 3, 5, 6 or 7 "bark rings" spread across
8 geometry rows, or 41-45 spread across 40. The clamp is simultaneously the
only thing that makes the feature visible and the only thing that makes it
wrong.

=====================================================================
7. SECONDARY FINDING (unasked but adjacent)
=====================================================================
The reference designer /home/user/7T-Pawns/src/tools/7t_palm_designer.jsx
has the SAME bug class on a shifted boundary. It splits resolution from
count — line 177 `const steps = Math.max(rings * 2, 20);` and line 179
`const t = i / steps;` — so line 183 `const ringPhase = t * rings * Math.PI * 2;`
becomes i*pi whenever steps == rings*2, i.e. whenever rings >= 10, which is
again zero at every sample. The designer only shows bark when rings < 10
(steps pinned at 20). The shader did not inherit the `steps` split at all;
it reuses trunk_rings for both roles, which widens the dead band from
"rings >= 10" to "8 <= rings <= 40".

=====================================================================
8. WHERE I LOOKED / WHAT I DID NOT FIND
=====================================================================
- Any second writer of bark_rings between CPU sampling and the shader:
  none. grepped BARK_RINGS|bark_rings across /home/user/7T-Pawns/src; the
  only mutation sites are entity_pipeline.hpp:141-142 and the pass-through
  at grounded.hpp:1248.
- Any ceiling clamp on BARK_RINGS: none (1e30f at grounded.hpp:1081).
- Any existing test, assert, or comment in world.wgsl acknowledging the
  degeneracy: none. Lines 11263 and 11271-11274 carry only the descriptive
  comment `// -- TRUNK: surface of revolution with taper + bark rings + lean --`.
- READ-ONLY: no files were edited, no commits made. Branch claude/flora-1,
  working tree clean.

### Quotes

**`src/cartridges/the_board/realization/world.wgsl:11265`**

```
    let trunk_rings = min(u32(max(8.0, p.bark_rings)), 40u);
```

trunk_rings definition. Note it is max-in-f32 then u32() TRUNCATION then min-in-u32, not a symmetric clamp() call.

**`src/cartridges/the_board/realization/world.wgsl:11268`**

```
    for (var ring = 0u; ring <= trunk_rings; ring++) {
```

TRUNK ring loop header; inclusive upper bound, so ring = 0..trunk_rings, trunk_rings+1 rows.

**`src/cartridges/the_board/realization/world.wgsl:11269`**

```
        let t = f32(ring) / f32(trunk_rings);
```

t is normalized by trunk_rings (the CLAMPED value).

**`src/cartridges/the_board/realization/world.wgsl:11272`**

```
        let ring_phase = t * p.bark_rings * 2.0 * PI;
```

ring_phase is built from p.bark_rings (the RAW param), not trunk_rings. This is the whole load-bearing asymmetry: numerator raw, denominator clamped.

**`src/cartridges/the_board/realization/world.wgsl:11273`**

```
        r += sin(ring_phase) * p.bark_depth * (1.0 - t * 0.5);
```

The only consumer of ring_phase and the only consumer of p.bark_depth in palm_mesh_gen.

**`src/cartridges/the_board/realization/world.wgsl:11271`**

```
        var r = p.base_r + (p.top_r - p.base_r) * t;
```

Included for context: the base radius that 'r +=' modulates.

**`src/cartridges/the_board/realization/world.wgsl:140`**

```
const PI: f32 = 3.14159265359;
```

PI as f32 rounds to 3.14159274101257324; PI_f32 - pi = +8.742278e-08. This is the entire source of the residual sin values below.

**`src/cartridges/the_board/bodies/grounded.hpp:1081`**

```
    { PalmProp::BARK_RINGS,   3.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
```

TierParamDef{prop, floor=3.0f, ceiling=1e30f (none), do_round=true, dist}. do_round=true is what makes bark_rings integral.

**`src/cartridges/the_board/machine/entity_pipeline.hpp:141`**

```
        if (pd.do_round) val = std::round(val);
```

Rounding happens BEFORE the floor clamp.

**`src/cartridges/the_board/machine/entity_pipeline.hpp:142`**

```
        val = std::max(pd.floor, val);
```

Floor 3.0f applied after round, so bark_rings >= 3 and integral.

**`src/cartridges/the_board/primitives/seed_utils.hpp:69`**

```
    z = std::max(-3.0f, std::min(3.0f, z));
```

z-clamp to +/-3 sigma. This bounds the reachable bark_rings range per tier exactly: SAPLING [3,21], COASTAL [8,32], ROYAL [15,45].

**`src/cartridges/the_board/bodies/grounded.hpp:1248`**

```
    mp.bark_rings   = inst.params[PalmIdx::BARK_RINGS];
```

CPU->GPU handoff; p.bark_rings in the shader is exactly the rounded CPU value, no further transform.

**`src/cartridges/the_board/bodies/grounded.hpp:1189`**

```
    // LEAN/LEAN_DIR (angles), BARK_RINGS/FROND_COUNT (counts),
```

BARK_RINGS is excluded from PALM_INDOOR_RESCALE_PARAMS, so indoor rescale cannot make it non-integral.

**`src/cartridges/the_board/bodies/grounded.hpp:1133`**

```
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
```

SAPLING line 1: HEIGHT mu 25.2 CONFIRMED.

**`src/cartridges/the_board/bodies/grounded.hpp:1134`**

```
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {6.8f, 0.85f},  {0.80f, 0.15f},
```

SAPLING line 2: BARK_RINGS mu 12 sigma 3 CONFIRMED; BARK_DEPTH 0.04; FROND_COUNT mu 20 CONFIRMED; FROND_LEN mu 6.8 CONFIRMED.

**`src/cartridges/the_board/bodies/grounded.hpp:1137`**

```
        0.15f, 0.15f, 0.06f, 0.06f, 12, 8
```

SAPLING trunk_segs 12, frond_segs 8 CONFIRMED.

**`src/cartridges/the_board/bodies/grounded.hpp:1140`**

```
        { 0.35f, 0.0f, { {28.8f, 3.6f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
```

COASTAL line 1: HEIGHT mu 28.8 CONFIRMED.

**`src/cartridges/the_board/bodies/grounded.hpp:1141`**

```
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {6.8f, 1.28f},  {1.00f, 0.20f},
```

COASTAL line 2: BARK_RINGS mu 20 sigma 4 CONFIRMED; BARK_DEPTH 0.06; FROND_COUNT mu 24 CONFIRMED; FROND_LEN mu 6.8 CONFIRMED.

**`src/cartridges/the_board/bodies/grounded.hpp:1144`**

```
        0.20f, 0.15f, 0.06f, 0.06f, 16, 10
```

COASTAL trunk_segs 16, frond_segs 10 CONFIRMED.

**`src/cartridges/the_board/bodies/grounded.hpp:1147`**

```
        { 0.20f, 0.0f, { {37.8f, 7.2f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
```

ROYAL line 1: HEIGHT mu 37.8 CONFIRMED.

**`src/cartridges/the_board/bodies/grounded.hpp:1148`**

```
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {8.8f, 2.0f},   {1.40f, 0.35f},
```

ROYAL line 2: BARK_RINGS mu 30 sigma 5 CONFIRMED; BARK_DEPTH 0.08; FROND_COUNT mu 30 CONFIRMED; FROND_LEN mu 8.8 CONFIRMED.

**`src/cartridges/the_board/bodies/grounded.hpp:1151`**

```
        0.25f, 0.20f, 0.08f, 0.08f, 20, 12
```

ROYAL trunk_segs 20, frond_segs 12 CONFIRMED.

**`src/tools/7t_palm_designer.jsx:176`**

```
  const rings = Math.max(8, Math.round(T.bark_rings));
```

Reference designer, for contrast with the shader.

**`src/tools/7t_palm_designer.jsx:177`**

```
  const steps = Math.max(rings * 2, 20);
```

THE DIVERGENCE: the designer decouples sampling resolution (steps) from ring count (rings). The shader has no such split -- it uses trunk_rings for both.

**`src/tools/7t_palm_designer.jsx:179`**

```
    const t = i / steps;
```

Designer t is normalized by steps, not by rings.

**`src/tools/7t_palm_designer.jsx:183`**

```
    const ringPhase = t * rings * Math.PI * 2;
```

Designer ring_phase = (i/steps)*rings*2pi. With steps = 2*rings this is i*pi, so sin is ALSO identically zero for rings >= 10; for rings < 10 (steps pinned at 20) it is nonzero. The designer has the same class of bug on a different boundary.

**`src/tools/7t_palm_designer.jsx:184`**

```
    r += Math.sin(ringPhase) * T.bark_depth * (1.0 - t * 0.5); // less pronounced at top
```

Same modulation expression as world.wgsl:11273.

---

## Item 13 — per-arm and per-blade costs (inputs to F7)

**Scope as run:** ITEM 13 — world.wgsl cactus_mesh_gen / blade_cluster_mesh_gen exact vertex & index cost expressions (feeds F7)

### Answer

ITEM 13 — file: /home/user/7T-Pawns/src/cartridges/the_board/realization/world.wgsl (branch claude/flora-1, HEAD 734e8c2). READ-ONLY; no edits made.

=== (a) cactus_mesh_gen (lines 11586-11844) ===

Variables in scope that drive every trip count (verbatim decls in findings):
  L11609  ribs        = u32(max(4.0, p.ribs))                -> lower clamp only, NO upper bound
  L11610  around      = min(max(ribs * 2u, 12u), 20u)        -> in [12, 20]
  L11611  trunk_steps = min(u32(p.trunk_segs), 20u)          -> <= 20
  L11697  n_arms      = u32(max(0.0, p.arm_count))           -> NO upper bound
  L11698  arm_segs_u  = min(u32(p.arm_segs), 12u)            -> <= 12
  L11699  arm_ribs    = max(4u, ribs - 2u)                   -> lower clamp only
  L11700  arm_around  = min(max(arm_ribs * 2u, 8u), 12u)     -> in [8, 12]
  L11822  arm_cap_segs= min(arm_around, 8u)                  -> <= 8

PER-ARM cost (one iteration of `for (var a = 0u; a < n_arms; a++)` at L11702):
  vertices  = (arm_segs_u + 1) * arm_around + 1 + arm_cap_segs
              - arm body: L11728 `s <= arm_segs_u` (inclusive, arm_segs_u+1 rows) x L11757 `seg < arm_around` x 1 write (L11782, vi++ L11786)
              - arm cap tip: 1 write (L11815, vi++ L11819)
              - arm cap ring: L11823 `seg < arm_cap_segs` x 1 write (L11825, vi++ L11829)
  indices   = arm_segs_u * arm_around * 6 + arm_cap_segs * 3
              - arm body: L11797 `s < arm_segs_u` (exclusive) x L11798 `seg < arm_around` x 6 writes (L11803-11808)
              - arm cap: L11832 `seg < arm_cap_segs` x 3 writes (L11834-11836)
  Worst case (arm_segs_u=12, arm_around=12, arm_cap_segs=8): 13*12+1+8 = 165 verts; 12*12*6+8*3 = 888 indices.

TRUNK block cost:
  vertices  = (trunk_steps + 1) * around
              L11615 `ring <= trunk_steps` (INCLUSIVE) x L11629 `seg < around` x 1 write (L11643, vi++ L11646)
  indices   = trunk_steps * around * 6
              L11651 `ring < trunk_steps` (EXCLUSIVE) x L11652 `seg < around` x 6 writes (L11657-11662)
  Worst case (20, 20): 21*20 = 420 verts; 20*20*6 = 2400 indices.

CAP / TOP-RING blocks — there are exactly TWO cap blocks in the function, and only ONE of them has a ring:
  1. TRUNK CAP (L11666-11692): vertices = 1 (the single tip, L11680/vi++ L11684); indices = around * 3
     (L11687 x 3 writes L11689-11691). There is NO trunk cap ring — L11686 states verbatim
     "Fan from tip to trunk's existing top ring — no separate cap ring", and L11668
     `let top_ring_vi = trunk_steps * around;` reuses the trunk's last ring vertices.
     Worst case (around=20): 1 vert, 60 indices.
  2. ARM CAP (L11812-11837, inside the per-arm loop, already folded into the per-ARM numbers above):
     vertices = 1 + arm_cap_segs (tip L11815 + ring loop L11823); indices = arm_cap_segs * 3 (L11832-11836).
     Worst case: 9 verts, 24 indices.
  No other cap/top-ring vertex or index writes exist in cactus_mesh_gen — the only remaining write site is
  the tail degenerate fill at L11841-11843, which overwrites already-unused index slots with vb_base and
  emits no geometry.

TOTALS (as expressions):
  verts_total   = (trunk_steps + 1) * around + 1
                + n_arms * ((arm_segs_u + 1) * arm_around + 1 + arm_cap_segs)
  indices_total = trunk_steps * around * 6 + around * 3
                + n_arms * (arm_segs_u * arm_around * 6 + arm_cap_segs * 3)

Arithmetic against the slot caps (my own computation, worst-case geometry):
  vertices: 1500 - 421 = 1079; 1079 / 165 = 6.53 -> at most 6 arms fit (6*165=990, 7*165=1155 > 1079).
  indices:  7998 - 2460 = 5538; 5538 / 888 = 6.24 -> at most 6 arms fit (6*888=5328, 7*888=6216 > 5538).
  Both budgets bind at n_arms = 6 in the worst case; a 7th arm overruns BOTH the vertex and the index slot.

=== (b) blade_cluster_mesh_gen (lines 11930-12080) ===
Full function quoted verbatim in findings (the finding anchored at line 11930).

PER-BLADE cost (one iteration of `for (var b = 0u; b < n_blades; b++)` at L11955):
  vertices = (segs + 1) * 2
             L12000 `s <= segs` (INCLUSIVE, segs+1 steps) x TWO writes per step:
             L12047 "Left vertex" (vi++ L12050) and L12053 "Right vertex" (vi++ L12056).
             L12060 `let vps = 2u;  // verts per step (left + right)` corroborates the 2.
  indices  = segs * 6
             L12061 `s < segs` (EXCLUSIVE) x 6 writes (L12067-12072).

BASE / ROOT COST: none. There is no root, hub, socket, or shared base geometry — zero vertex writes and zero
index writes occur outside the per-blade loop. The only two write sites outside it are the inactive-slot fill
(L11940-11942) and the tail degenerate fill (L12077-12079), both of which write vb_base into already-unused
index slots and emit no geometry. Looked at the whole function body, lines 11930-12080.

TOTALS:
  verts_total   = n_blades * (segs + 1) * 2
  indices_total = n_blades * segs * 6

Arithmetic against the slot caps (my own computation): for any legal segs >= 3 the VERTEX budget always binds
first, since (segs+1)*2/500 > segs*6/1998 reduces to 3996*segs + 3996 > 3000*segs, true for all segs >= 0.
Max blades = floor(500 / ((segs+1)*2)). At segs=3: 8 verts/blade -> 62 blades (index budget would have allowed
111). At segs=8: 18 verts/blade -> 27 blades. At segs=12: 26 verts/blade -> 19 blades.

=== Slot capacity declarations (verbatim, in findings) ===
  L11552 CACTUSG_MAX_VERTS_PER_SLOT   = 1500u
  L11553 CACTUSG_MAX_INDICES_PER_SLOT = 7998u
  L11897 BLADEG_MAX_VERTS_PER_SLOT    = 500u
  L11898 BLADEG_MAX_INDICES_PER_SLOT  = 1998u

=== Trip-variable upper bounds — the direct answer ===
  n_arms   (L11697) = u32(max(0.0, p.arm_count))    -> NO upper bound. max() is a LOWER clamp to 0.0 only;
                                                       there is no min() and no MAX_ARMS constant anywhere in
                                                       the function. p.arm_count is an unbounded f32 field of
                                                       CactusMeshParams (declared L11541 `arm_count: f32,`).
  n_blades (L11951) = u32(max(2.0, p.blade_count))  -> NO upper bound. Same shape: lower clamp to 2.0 only,
                                                       no min(), no cap constant. p.blade_count is an unbounded
                                                       f32 field of BladeClusterMeshParams (declared L11886
                                                       `blade_count: f32,`).
  Neither has an upper bound. Note the asymmetry in the secondary trip vars: the cactus caps its per-arm
  detail (arm_segs_u min 12u L11698, arm_around min 12u L11700, around min 20u L11610, trunk_steps min 20u
  L11611, arm_cap_segs min 8u L11822), whereas the blade path caps NOTHING — `segs = max(3u, p.blade_segs)`
  (L11952) is also unbounded above, so blade cost is unbounded in BOTH factors of n_blades * (segs+1) * 2.
  Contrast the palm path, which per commit 734e8c2 ("F6: the palm frond count is capped by the slot, not a
  constant") does clamp by slot capacity; neither cactus_mesh_gen nor blade_cluster_mesh_gen has an equivalent
  slot-derived clamp in the shader.

=== PALM_TIERS mu verification (file: /home/user/7T-Pawns/src/cartridges/the_board/bodies/grounded.hpp) ===
CONFIRM — all 18 supplied values match the live table verbatim. Table at L1131-1153; pair ordering proved by
PalmIdx (L1055-1073: HEIGHT=0, BARK_RINGS=5, FROND_COUNT=7, FROND_LEN=8) and PalmTierRow (L1105-1113, whose
last two fields are trunk_segs then frond_segs).
  SAPLING (L1132-1138): height 25.2, bark_rings 12, frond_count 20, frond_len 6.8, trunk_segs 12, frond_segs 8
  COASTAL (L1139-1145): height 28.8, bark_rings 20, frond_count 24, frond_len 6.8, trunk_segs 16, frond_segs 10
  ROYAL   (L1146-1152): height 37.8, bark_rings 30, frond_count 30, frond_len 8.8, trunk_segs 20, frond_segs 12

### Quotes

**`src/cartridges/the_board/realization/world.wgsl:11552`**

```
const CACTUSG_MAX_VERTS_PER_SLOT: u32 = 1500u;
```

slot capacity, vertices

**`src/cartridges/the_board/realization/world.wgsl:11553`**

```
const CACTUSG_MAX_INDICES_PER_SLOT: u32 = 7998u;
```

slot capacity, indices

**`src/cartridges/the_board/realization/world.wgsl:11554`**

```
const CACTUSG_FLOATS_PER_VERTEX: u32 = 10u;
```

context

**`src/cartridges/the_board/realization/world.wgsl:11555`**

```
const CACTUSG_MAX_SLOTS: u32 = 20u;
```

context

**`src/cartridges/the_board/realization/world.wgsl:11897`**

```
const BLADEG_MAX_VERTS_PER_SLOT: u32 = 500u;
```

slot capacity, vertices

**`src/cartridges/the_board/realization/world.wgsl:11898`**

```
const BLADEG_MAX_INDICES_PER_SLOT: u32 = 1998u;
```

slot capacity, indices

**`src/cartridges/the_board/realization/world.wgsl:11899`**

```
const BLADEG_FLOATS_PER_VERTEX: u32 = 10u;
```

context

**`src/cartridges/the_board/realization/world.wgsl:11900`**

```
const BLADEG_MAX_SLOTS: u32 = 32u;
```

context

**`src/cartridges/the_board/realization/world.wgsl:11586`**

```
fn cactus_mesh_gen(@builtin(global_invocation_id) gid: vec3<u32>) {
```

(a) entry point

**`src/cartridges/the_board/realization/world.wgsl:11601`**

```
    var vi = 0u;
```

vertex cursor starts at 0

**`src/cartridges/the_board/realization/world.wgsl:11602`**

```
    var ii = 0u;
```

index cursor starts at 0

**`src/cartridges/the_board/realization/world.wgsl:11609`**

```
    let ribs = u32(max(4.0, p.ribs));
```

trip-count input; lower clamp only, NO upper bound

**`src/cartridges/the_board/realization/world.wgsl:11610`**

```
    let around = min(max(ribs * 2u, 12u), 20u);
```

trunk ring circumference; bounded [12,20]

**`src/cartridges/the_board/realization/world.wgsl:11611`**

```
    let trunk_steps = min(u32(p.trunk_segs), 20u);
```

bounded <= 20

**`src/cartridges/the_board/realization/world.wgsl:11615`**

```
    for (var ring = 0u; ring <= trunk_steps; ring++) {
```

TRUNK vertex outer loop — INCLUSIVE, runs trunk_steps+1 times

**`src/cartridges/the_board/realization/world.wgsl:11629`**

```
        for (var seg = 0u; seg < around; seg++) {
```

TRUNK vertex inner loop — runs `around` times

**`src/cartridges/the_board/realization/world.wgsl:11643`**

```
            cactusg_write_vertex(vb_base + vi,
```

TRUNK: the ONLY vertex write in the trunk loops (1 per iteration); vi++ at line 11646

**`src/cartridges/the_board/realization/world.wgsl:11646`**

```
            vi++;
```

TRUNK vertex increment

**`src/cartridges/the_board/realization/world.wgsl:11651`**

```
    for (var ring = 0u; ring < trunk_steps; ring++) {
```

TRUNK index outer loop — EXCLUSIVE, runs trunk_steps times

**`src/cartridges/the_board/realization/world.wgsl:11652`**

```
        for (var seg = 0u; seg < around; seg++) {
```

TRUNK index inner loop

**`src/cartridges/the_board/realization/world.wgsl:11657`**

```
            cactusg_indices[ib_base + ii] = vb_base + row0 + seg; ii++;
```

TRUNK index write 1 of 6

**`src/cartridges/the_board/realization/world.wgsl:11658`**

```
            cactusg_indices[ib_base + ii] = vb_base + row1 + seg; ii++;
```

TRUNK index write 2 of 6

**`src/cartridges/the_board/realization/world.wgsl:11659`**

```
            cactusg_indices[ib_base + ii] = vb_base + row1 + next_seg; ii++;
```

TRUNK index write 3 of 6

**`src/cartridges/the_board/realization/world.wgsl:11660`**

```
            cactusg_indices[ib_base + ii] = vb_base + row0 + seg; ii++;
```

TRUNK index write 4 of 6

**`src/cartridges/the_board/realization/world.wgsl:11661`**

```
            cactusg_indices[ib_base + ii] = vb_base + row1 + next_seg; ii++;
```

TRUNK index write 5 of 6

**`src/cartridges/the_board/realization/world.wgsl:11662`**

```
            cactusg_indices[ib_base + ii] = vb_base + row0 + next_seg; ii++;
```

TRUNK index write 6 of 6

**`src/cartridges/the_board/realization/world.wgsl:11666`**

```
    // ── TRUNK CAP (stitched to top ring) ──
```

TRUNK CAP block start

**`src/cartridges/the_board/realization/world.wgsl:11668`**

```
    let top_ring_vi = trunk_steps * around;  // first vertex of trunk's last ring
```

cap reuses the trunk's existing top ring — no new ring vertices

**`src/cartridges/the_board/realization/world.wgsl:11678`**

```
    // Single tip vertex above center
```

TRUNK CAP vertex cost is exactly 1

**`src/cartridges/the_board/realization/world.wgsl:11680`**

```
    cactusg_write_vertex(vb_base + vi,
```

TRUNK CAP: the single tip vertex write; vi++ at 11684

**`src/cartridges/the_board/realization/world.wgsl:11684`**

```
    vi++;
```

TRUNK CAP tip vertex increment

**`src/cartridges/the_board/realization/world.wgsl:11686`**

```
    // Fan from tip to trunk's existing top ring — no separate cap ring
```

explicit: there is NO separate top-ring vertex block for the trunk cap

**`src/cartridges/the_board/realization/world.wgsl:11687`**

```
    for (var seg = 0u; seg < around; seg++) {
```

TRUNK CAP index loop — runs `around` times

**`src/cartridges/the_board/realization/world.wgsl:11689`**

```
        cactusg_indices[ib_base + ii] = vb_base + cap_tip_vi; ii++;
```

TRUNK CAP index write 1 of 3

**`src/cartridges/the_board/realization/world.wgsl:11690`**

```
        cactusg_indices[ib_base + ii] = vb_base + top_ring_vi + seg; ii++;
```

TRUNK CAP index write 2 of 3

**`src/cartridges/the_board/realization/world.wgsl:11691`**

```
        cactusg_indices[ib_base + ii] = vb_base + top_ring_vi + next; ii++;
```

TRUNK CAP index write 3 of 3

**`src/cartridges/the_board/realization/world.wgsl:11697`**

```
    let n_arms = u32(max(0.0, p.arm_count));
```

ARM TRIP VARIABLE. Lower clamp to 0.0 only. NO upper bound / no min() cap.

**`src/cartridges/the_board/realization/world.wgsl:11698`**

```
    let arm_segs_u = min(u32(p.arm_segs), 12u);
```

bounded <= 12

**`src/cartridges/the_board/realization/world.wgsl:11699`**

```
    let arm_ribs = max(4u, ribs - 2u);
```

lower clamp only

**`src/cartridges/the_board/realization/world.wgsl:11700`**

```
    let arm_around = min(max(arm_ribs * 2u, 8u), 12u);
```

bounded [8,12]

**`src/cartridges/the_board/realization/world.wgsl:11702`**

```
    for (var a = 0u; a < n_arms; a++) {
```

PER-ARM loop header — everything below is per-ARM

**`src/cartridges/the_board/realization/world.wgsl:11726`**

```
        let arm_vi_start = vi;
```

per-arm vertex base

**`src/cartridges/the_board/realization/world.wgsl:11728`**

```
        for (var s = 0u; s <= arm_segs_u; s++) {
```

ARM BODY vertex outer loop — INCLUSIVE, runs arm_segs_u+1 times

**`src/cartridges/the_board/realization/world.wgsl:11757`**

```
            for (var seg = 0u; seg < arm_around; seg++) {
```

ARM BODY vertex inner loop — runs arm_around times

**`src/cartridges/the_board/realization/world.wgsl:11782`**

```
                cactusg_write_vertex(vb_base + vi,
```

ARM BODY: 1 vertex write per iteration; vi++ at 11786

**`src/cartridges/the_board/realization/world.wgsl:11786`**

```
                vi++;
```

ARM BODY vertex increment

**`src/cartridges/the_board/realization/world.wgsl:11797`**

```
        for (var s = 0u; s < arm_segs_u; s++) {
```

ARM BODY index outer loop — EXCLUSIVE, runs arm_segs_u times

**`src/cartridges/the_board/realization/world.wgsl:11798`**

```
            for (var seg = 0u; seg < arm_around; seg++) {
```

ARM BODY index inner loop

**`src/cartridges/the_board/realization/world.wgsl:11803`**

```
                cactusg_indices[ib_base + ii] = vb_base + row0 + seg; ii++;
```

ARM BODY index write 1 of 6

**`src/cartridges/the_board/realization/world.wgsl:11804`**

```
                cactusg_indices[ib_base + ii] = vb_base + row1 + seg; ii++;
```

ARM BODY index write 2 of 6

**`src/cartridges/the_board/realization/world.wgsl:11805`**

```
                cactusg_indices[ib_base + ii] = vb_base + row1 + next_seg; ii++;
```

ARM BODY index write 3 of 6

**`src/cartridges/the_board/realization/world.wgsl:11806`**

```
                cactusg_indices[ib_base + ii] = vb_base + row0 + seg; ii++;
```

ARM BODY index write 4 of 6

**`src/cartridges/the_board/realization/world.wgsl:11807`**

```
                cactusg_indices[ib_base + ii] = vb_base + row1 + next_seg; ii++;
```

ARM BODY index write 5 of 6

**`src/cartridges/the_board/realization/world.wgsl:11808`**

```
                cactusg_indices[ib_base + ii] = vb_base + row0 + next_seg; ii++;
```

ARM BODY index write 6 of 6

**`src/cartridges/the_board/realization/world.wgsl:11812`**

```
        // Arm cap
```

ARM CAP block start — this cap DOES have its own ring, unlike the trunk cap

**`src/cartridges/the_board/realization/world.wgsl:11815`**

```
        cactusg_write_vertex(vb_base + vi,
```

ARM CAP tip vertex — exactly 1; vi++ at 11819

**`src/cartridges/the_board/realization/world.wgsl:11819`**

```
        vi++;
```

ARM CAP tip vertex increment

**`src/cartridges/the_board/realization/world.wgsl:11821`**

```
        let arm_cap_ring = vi;
```

ARM CAP ring vertex base

**`src/cartridges/the_board/realization/world.wgsl:11822`**

```
        let arm_cap_segs = min(arm_around, 8u);
```

ARM CAP ring trip count; bounded <= 8

**`src/cartridges/the_board/realization/world.wgsl:11823`**

```
        for (var seg = 0u; seg < arm_cap_segs; seg++) {
```

ARM CAP RING vertex loop — arm_cap_segs vertices

**`src/cartridges/the_board/realization/world.wgsl:11825`**

```
            cactusg_write_vertex(vb_base + vi,
```

ARM CAP RING vertex write, 1 per iteration; vi++ at 11829

**`src/cartridges/the_board/realization/world.wgsl:11829`**

```
            vi++;
```

ARM CAP RING vertex increment

**`src/cartridges/the_board/realization/world.wgsl:11832`**

```
        for (var seg = 0u; seg < arm_cap_segs; seg++) {
```

ARM CAP index loop

**`src/cartridges/the_board/realization/world.wgsl:11834`**

```
            cactusg_indices[ib_base + ii] = vb_base + arm_cap_tip; ii++;
```

ARM CAP index write 1 of 3

**`src/cartridges/the_board/realization/world.wgsl:11835`**

```
            cactusg_indices[ib_base + ii] = vb_base + arm_cap_ring + seg; ii++;
```

ARM CAP index write 2 of 3

**`src/cartridges/the_board/realization/world.wgsl:11836`**

```
            cactusg_indices[ib_base + ii] = vb_base + arm_cap_ring + next; ii++;
```

ARM CAP index write 3 of 3

**`src/cartridges/the_board/realization/world.wgsl:11841`**

```
    for (var i = ii; i < CACTUSG_MAX_INDICES_PER_SLOT; i++) {
```

tail degenerate-fill; writes no vertices, consumes no new index budget

**`src/cartridges/the_board/realization/world.wgsl:11930`**

```
@compute @workgroup_size(1)
fn blade_cluster_mesh_gen(@builtin(global_invocation_id) gid: vec3<u32>) {
    let slot = gid.x;
    if (slot >= BLADEG_MAX_SLOTS) { return; }

    let p = bladeg_params[slot];
    let vb_base = slot * BLADEG_MAX_VERTS_PER_SLOT;
    let ib_base = slot * BLADEG_MAX_INDICES_PER_SLOT;

    if (p.is_active == 0u) {
        for (var i = 0u; i < BLADEG_MAX_INDICES_PER_SLOT; i++) {
            bladeg_indices[ib_base + i] = vb_base;  // NOT 0u!
        }
        return;
    }

    var vi = 0u;
    var ii = 0u;

    let cx = p.center_x;
    let cz = p.center_z;
    let n_blades = u32(max(2.0, p.blade_count));
    let segs = max(3u, p.blade_segs);
    let GA = PI * (3.0 - sqrt(5.0));

    for (var b = 0u; b < n_blades; b++) {
        let azimuth = f32(b) * GA;
        let ca = cos(azimuth);
        let sa = sin(azimuth);

        // Per-blade height variation
        let h_mult = 1.0 + (blade_hash(p.seed, 970u + b) - 0.5) * p.blade_h_var * 2.0;
        let blade_h = p.blade_h * max(0.4, h_mult);

        // Splay: outer blades splay more
        let rank = f32(b) / max(1.0, f32(n_blades - 1u));
        let splay_ang = p.splay * (0.6 + 0.4 * (1.0 - rank));
        let splay_j = (blade_hash(p.seed, 980u + b) - 0.5) * 0.15;
        let final_splay = splay_ang + splay_j;

        let cos_s = cos(final_splay);
        let sin_s = sin(final_splay);
        let fwd_x = ca * sin_s;
        let fwd_y = cos_s;
        let fwd_z = sa * sin_s;

        // Right vector (perpendicular for blade width)
        var rx: f32; var ry: f32; var rz: f32;
        if (cos_s > 0.95) {
            rx = -sa; ry = 0.0; rz = ca;
        } else {
            // cross(fwd, up)
            rx = fwd_z; ry = 0.0; rz = -fwd_x;
            let rl = sqrt(rx * rx + rz * rz);
            rx /= max(rl, 0.001);
            rz /= max(rl, 0.001);
        }

        let twist_dir = select(-1.0, 1.0, b % 2u == 0u);
        let twist_amt = p.twist * twist_dir;

        // Base color for this blade
        let age_blend = (1.0 - rank) * 0.5;
        let base_r = p.blade_r + (p.aged_r - p.blade_r) * age_blend;
        let base_g = p.blade_g + (p.aged_g - p.blade_g) * age_blend;
        let base_b = p.blade_b + (p.aged_b - p.blade_b) * age_blend;

        let blade_vi_start = vi;

        // Two vertices per segment step (left + right of midrib)
        for (var s = 0u; s <= segs; s++) {
            let t = f32(s) / f32(segs);
            let dist = t * blade_h;

            // Curve: quadratic outward arc
            let curve_off = p.curve * blade_h * t * t;

            // Position along forward + curve
            let px = fwd_x * dist + ca * curve_off;
            let py = fwd_y * dist;
            let pz = fwd_z * dist + sa * curve_off;

            // Width: ramp in at root, taper to point
            let base_frac = 0.3 + 0.7 * min(1.0, t * 4.0);
            let tip_frac = 1.0 - pow(t, p.taper * 2.5 + 0.5);
            let w = p.blade_w * base_frac * tip_frac;

            // Twist
            let tw_angle = twist_amt * t * PI;
            let ct = cos(tw_angle);
            let st_tw = sin(tw_angle);
            let trx = rx * ct + fwd_x * st_tw;
            let try_ = ry * ct + fwd_y * st_tw;
            let trz = rz * ct + fwd_z * st_tw;

            let half_w = w * 0.5;
            let perp_x = trx * half_w;
            let perp_y = try_ * half_w;
            let perp_z = trz * half_w;

            // Color: shade by height, age at tip
            let shade = 0.7 + 0.3 * sin(t * PI * 0.8);
            let tip_age = t * t * 0.3;
            let cr = min(1.0, (base_r + (p.aged_r - base_r) * tip_age) * shade);
            let cg = min(1.0, (base_g + (p.aged_g - base_g) * tip_age) * shade);
            let cb = min(1.0, (base_b + (p.aged_b - base_b) * tip_age) * shade);

            // Normal: blade face normal (cross of forward and right)
            let nx = fwd_y * trz - fwd_z * try_;
            let ny = fwd_z * trx - fwd_x * trz;
            let nz = fwd_x * try_ - fwd_y * trx;
            let nl = sqrt(nx * nx + ny * ny + nz * nz);
            let nnx = nx / max(nl, 0.001);
            let nny = ny / max(nl, 0.001);
            let nnz = nz / max(nl, 0.001);

            // Left vertex
            bladeg_write_vertex(vb_base + vi,
                cx + px + perp_x, py + perp_y, cz + pz + perp_z,
                nnx, nny, nnz, cr, cg, cb, slot);
            vi++;

            // Right vertex
            bladeg_write_vertex(vb_base + vi,
                cx + px - perp_x, py - perp_y, cz + pz - perp_z,
                -nnx, -nny, -nnz, cr, cg, cb, slot);
            vi++;
        }

        // Index the quad strip: 2 tris per segment
        let vps = 2u;  // verts per step (left + right)
        for (var s = 0u; s < segs; s++) {
            let i0 = blade_vi_start + s * vps;       // left  row s
            let i1 = blade_vi_start + s * vps + 1u;  // right row s
            let i2 = blade_vi_start + (s + 1u) * vps;      // left  row s+1
            let i3 = blade_vi_start + (s + 1u) * vps + 1u; // right row s+1

            bladeg_indices[ib_base + ii] = vb_base + i0; ii++;
            bladeg_indices[ib_base + ii] = vb_base + i2; ii++;
            bladeg_indices[ib_base + ii] = vb_base + i3; ii++;
            bladeg_indices[ib_base + ii] = vb_base + i0; ii++;
            bladeg_indices[ib_base + ii] = vb_base + i3; ii++;
            bladeg_indices[ib_base + ii] = vb_base + i1; ii++;
        }
    }

    // Fill remaining indices with vb_base (NOT 0u!)
    for (var i = ii; i < BLADEG_MAX_INDICES_PER_SLOT; i++) {
        bladeg_indices[ib_base + i] = vb_base;
    }
}
```

(b) FULL VERBATIM blade_cluster_mesh_gen, lines 11930-12080 inclusive (151 lines, spaces only, no tabs)

**`src/cartridges/the_board/realization/world.wgsl:11951`**

```
    let n_blades = u32(max(2.0, p.blade_count));
```

BLADE TRIP VARIABLE (the n_arms equivalent). Lower clamp to 2.0 only. NO upper bound / no min() cap.

**`src/cartridges/the_board/realization/world.wgsl:11952`**

```
    let segs = max(3u, p.blade_segs);
```

per-blade step count. Lower clamp only — NO upper bound either (contrast cactus trunk_segs/arm_segs which are min()-capped).

**`src/cartridges/the_board/realization/world.wgsl:11955`**

```
    for (var b = 0u; b < n_blades; b++) {
```

PER-BLADE loop header

**`src/cartridges/the_board/realization/world.wgsl:11997`**

```
        let blade_vi_start = vi;
```

per-blade vertex base

**`src/cartridges/the_board/realization/world.wgsl:12000`**

```
        for (var s = 0u; s <= segs; s++) {
```

BLADE vertex loop — INCLUSIVE, runs segs+1 times

**`src/cartridges/the_board/realization/world.wgsl:12047`**

```
            bladeg_write_vertex(vb_base + vi,
```

BLADE vertex write 1 of 2 per step (Left vertex); vi++ at 12050

**`src/cartridges/the_board/realization/world.wgsl:12053`**

```
            bladeg_write_vertex(vb_base + vi,
```

BLADE vertex write 2 of 2 per step (Right vertex); vi++ at 12056

**`src/cartridges/the_board/realization/world.wgsl:12060`**

```
        let vps = 2u;  // verts per step (left + right)
```

confirms 2 verts per step

**`src/cartridges/the_board/realization/world.wgsl:12061`**

```
        for (var s = 0u; s < segs; s++) {
```

BLADE index loop — EXCLUSIVE, runs segs times

**`src/cartridges/the_board/realization/world.wgsl:12067`**

```
            bladeg_indices[ib_base + ii] = vb_base + i0; ii++;
```

BLADE index write 1 of 6

**`src/cartridges/the_board/realization/world.wgsl:12068`**

```
            bladeg_indices[ib_base + ii] = vb_base + i2; ii++;
```

BLADE index write 2 of 6

**`src/cartridges/the_board/realization/world.wgsl:12069`**

```
            bladeg_indices[ib_base + ii] = vb_base + i3; ii++;
```

BLADE index write 3 of 6

**`src/cartridges/the_board/realization/world.wgsl:12070`**

```
            bladeg_indices[ib_base + ii] = vb_base + i0; ii++;
```

BLADE index write 4 of 6

**`src/cartridges/the_board/realization/world.wgsl:12071`**

```
            bladeg_indices[ib_base + ii] = vb_base + i3; ii++;
```

BLADE index write 5 of 6

**`src/cartridges/the_board/realization/world.wgsl:12072`**

```
            bladeg_indices[ib_base + ii] = vb_base + i1; ii++;
```

BLADE index write 6 of 6

**`src/cartridges/the_board/realization/world.wgsl:12077`**

```
    for (var i = ii; i < BLADEG_MAX_INDICES_PER_SLOT; i++) {
```

tail degenerate-fill only; no base/root geometry is emitted anywhere outside the per-blade loop

**`src/cartridges/the_board/bodies/grounded.hpp:1133`**

```
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
```

SAPLING: HEIGHT mu 25.2 CONFIRMED (PalmIdx::HEIGHT = 0, first pair)

**`src/cartridges/the_board/bodies/grounded.hpp:1134`**

```
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {6.8f, 0.85f},  {0.80f, 0.15f},
```

SAPLING: BARK_RINGS mu 12, FROND_COUNT mu 20, FROND_LEN mu 6.8 — all CONFIRMED (PalmIdx 5,7,8)

**`src/cartridges/the_board/bodies/grounded.hpp:1137`**

```
        0.15f, 0.15f, 0.06f, 0.06f, 12, 8
```

SAPLING: trunk_segs 12, frond_segs 8 — CONFIRMED (last two PalmTierRow fields)

**`src/cartridges/the_board/bodies/grounded.hpp:1140`**

```
        { 0.35f, 0.0f, { {28.8f, 3.6f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
```

COASTAL: HEIGHT mu 28.8 CONFIRMED

**`src/cartridges/the_board/bodies/grounded.hpp:1141`**

```
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {6.8f, 1.28f},  {1.00f, 0.20f},
```

COASTAL: BARK_RINGS 20, FROND_COUNT 24, FROND_LEN 6.8 — CONFIRMED

**`src/cartridges/the_board/bodies/grounded.hpp:1144`**

```
        0.20f, 0.15f, 0.06f, 0.06f, 16, 10
```

COASTAL: trunk_segs 16, frond_segs 10 — CONFIRMED

**`src/cartridges/the_board/bodies/grounded.hpp:1147`**

```
        { 0.20f, 0.0f, { {37.8f, 7.2f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
```

ROYAL: HEIGHT mu 37.8 CONFIRMED

**`src/cartridges/the_board/bodies/grounded.hpp:1148`**

```
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {8.8f, 2.0f},   {1.40f, 0.35f},
```

ROYAL: BARK_RINGS 30, FROND_COUNT 30, FROND_LEN 8.8 — CONFIRMED

**`src/cartridges/the_board/bodies/grounded.hpp:1151`**

```
        0.25f, 0.20f, 0.08f, 0.08f, 20, 12
```

ROYAL: trunk_segs 20, frond_segs 12 — CONFIRMED

**`src/cartridges/the_board/bodies/grounded.hpp:1105`**

```
struct PalmTierRow {
```

field order proving the trailing two ints are trunk_segs then frond_segs (lines 1111-1112)

**`src/cartridges/the_board/bodies/grounded.hpp:1055`**

```
struct PalmIdx {
```

index order proving pair positions: HEIGHT=0, BARK_RINGS=5, FROND_COUNT=7, FROND_LEN=8 (lines 1056-1072)

---
