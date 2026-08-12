# WEB_METER_0 — THE FIRST AUDIENCE-FLOOR READING

The first `[METER]` windows off a real phone. Captured 2026-08-12 by Jean
on a **Pixel 8**, Chrome, over USB (`chrome://inspect`), against the
deployed preview build of the `the-board-web-meter` preset (`ce6d53d`).

This is a **baseline record**, not a campaign. Nothing was changed to
produce it. Its job is to stop the next handoff reasoning about product
performance from the 920M's numbers.

---

## 1. The device

```
[Device] adapter: arm | valhall | ? | ?
[Device] requesting CORE DEFAULTS; exceptions carried: none
[Device] granted vs floor: maxTextureDimension2D=8192/2048
                           maxStorageBuffersPerShaderStage=8/8
                           maxUniformBufferBindingSize=65536/65536
[Device] modest device accepted — NO DISCARD
```

Mali, Valhall architecture — the Tensor G3's GPU. **This matters more than
it looks and Section 4 is about it.**

**`maxStorageBuffersPerShaderStage` is granted at exactly 8.** The 920M
reports 10. So the audience floor really is the Core default, and the
whole TETRIS budget arc — WALLET_0's 8/8→6/8, ORB_V's 7/8→6/8,
WALLET_1revA's 7/8→4/8 — was buying headroom on the machine that has
none to spare, not on the one that already had two seats going free. The
native log never showed that, because the 920M's driver hands over 10 and
hides the constraint.

**The pixel cap is doing exactly what its comment claims.**

```
[FRAME_1] dpr=2.25  fbPreCap=1008x2243  fbPostCap=672x1495
```

2.26 Mpx before, 1.00 Mpx after — `MAX_DEVICE_PIXEL_RATIO = 1.5` against a
device ratio of 2.25 removes **2.25× the fragment work**, and 448 CSS px ×
1.5 = 672 reproduces the post-cap width exactly. PORT_3c's "largest mobile
lever this program has, and it is a scalar" is confirmed on the first
device to test it.

---

## 2. Boot, against the native twin

| stage | 920M / Vulkan | **Pixel 8 / Chrome** |
|---|---|---|
| GPUState init | 38–69 ms | **13 ms** |
| Shader compile (Tint front-end) | 324–342 ms | **8 ms** |
| **All 59 pipelines** | **6,606 ms** | **10 ms** |
| Renderer init | 7,075 ms | **28 ms** |
| Patch system | 1,238 ms | **5 ms** |
| **Total init** | **8,314 ms** | **33 ms** |

Every one of the 59 pipelines reports `0 ms` individually; the total is 10.

**Put that beside the campaign that produced it.** PIVOT existed because
`update_player_agent` took 19,745–28,010 ms under FXC and then
access-violated. The same shader, on the audience's compiler, is part of a
**ten-millisecond** total. The compiler problem was never the program's —
it was one retired compiler on one laptop, and the pivot's real value was
getting off it rather than any shader change.

---

## 3. The frame

Two windows, 30 s apart, hands-off.

| | window 1 (t=30.1) | window 2 (t=60.1) |
|---|---|---|
| frames | 1779 | 1798 |
| **fps** | **50.6** | **59.9** |
| gpu sampled | 841 f | 751 f |
| `frame_total` mean | **1.18 ms** | **1.20 ms** |
| `S present` mean | 0.00 ms | 0.00 ms |
| `U_SUM` / `R_SUM` | 0.10 / 0.63 | 0.10 / 0.67 |
| residue | 0.17 | 0.15 |

Against the 920M's **17.4–18.2 fps** and `frame_total` **53.6–58.0 ms**.

**Window 1 is the honest one to quote for a cold visit** — 50.6 fps while
the exhibition is still streaming in and the photographer is filling its
snapshot pool. Window 2 at 59.9 fps is the settled state, and it is
vsync-locked: the frame has ~1.2 ms of CPU in a 16.6 ms budget.

`S present` reading **0.00** is the structural difference from native, not
a measurement error. On the web twin the frame ends at rAF; there is no
blocking wait on a saturated GPU, which is precisely what `present` was
absorbing on the 920M (48.7–51.8 ms of it).

**CPU is a non-issue on this device.** The nine U rows total 0.10 ms and
the twenty-two R rows total 0.67 ms.

---

## 4. THE GPU COLUMNS — READ THIS BEFORE USING THEM

`timestamp-query` **was granted**. This was not expected — the WEB_METER_0
commit predicted Chrome-on-Android would refuse it, and the degrade path
(`[METER] timestamp-query unavailable …`) never fired. The GPU columns are
populated for ~42–47% of frames.

**But they do not mean what the native GPU columns mean, for two separate
reasons.**

### 4a. Quantized to 100 µs

Every GPU value in both windows is a multiple of 0.1 ms — `0.10`, `0.40`,
`1.10`, `12.32`, `20.64`. That is Chrome's security quantization of
timestamp-query. **Any row whose mean is under ~0.5 ms is noise**, and the
sub-0.1 ms rows that are meaningful natively (`gol_derive_flush`,
`pawn_aura`, `placement_correction`) carry no information here.

### 4b. NON-ADDITIVE — the GPU is a tiler

Window 2's GPU rows, summed:

```
shadow_pass 9.21 + main_pass 11.12 + gol_zone_compute 6.58 + orb_sky 6.67
+ dispatch_compute 3.22 + frustum_cull 3.41 + live_card_write 1.09
+ stream_patches 0.41 + entity_mesh_gen 0.26  ≈  41.97 ms
```

**41.97 ms of "GPU work" in a frame running at 59.9 fps.** That is
impossible as occupancy, and it is not a bug in the meter.

Valhall is a **tile-based deferred renderer**. A render pass's timestamps
bracket work the hardware defers and overlaps with other passes; the
bracket is real, but the intervals are not disjoint and do not partition
the frame. On the 920M — immediate-mode — the native rows DO roughly
partition, which is why `main_pass` 30 ms + `shadow_pass` 14 ms ≈ the
44 ms that explained an 18 fps frame.

**Consequences, stated plainly so nobody re-derives them:**

- **Do NOT sum web GPU rows.** The total is meaningless.
- **Do NOT compute a row's "share of the frame"** from web numbers.
- **DO compare a row against itself** across builds on the same device.
  That comparison is valid and is what the lab preset is for.
- **Any pass-decomposition question — SHADOW_0's row 7 among them — must
  be answered on an immediate-mode GPU**, i.e. the native twin, or by a
  mechanism finer than pass-level timestamps.

### 4c. The rows anyway, both windows

Means, in ms, for whatever within-device comparison they are worth:

| row | w1 gpu | w2 gpu | w1 cpu | w2 cpu |
|---|---|---|---|---|
| `main_pass` | 10.91 | 11.12 | 0.09 | 0.10 |
| `shadow_pass` | 8.71 | 9.21 | 0.12 | 0.14 |
| `orb_sky` | 6.61 | 6.67 | 0.04 | 0.03 |
| `gol_zone_compute` | 6.53 | 6.58 | 0.04 | 0.04 |
| `frustum_cull` | 3.37 | 3.41 | 0.05 | 0.05 |
| `dispatch_compute` | 3.18 | 3.22 | 0.03 | 0.03 |
| `stream_patches` | 1.90 | 0.41 | 0.07 | 0.07 |
| `live_card_write` | 1.02 | 1.09 | 0.06 | 0.06 |
| `placement_correction` | 0.23 | 0.34 | 0.00 | 0.00 |
| `entity_mesh_gen` | 0.16 | 0.26 | 0.01 | 0.01 |
| `snapshot_pass` | 0.03 | 0.04 | 0.00 | 0.01 |

**The one relation that survives the caveats:** `shadow_pass` sits at
roughly 0.8× `main_pass` here, and at roughly 0.47× natively
(14.2 vs 30.2). Shadow is a comparable-order cost on both platforms and is
relatively *heavier* on the phone. SHADOW_1's target is real on the
audience floor — but its *size* is not measurable from this data.

---

## 5. Two anomalies worth recording

**`stream_patches gpu max 1323.04 ms`** in window 1 (window 2: 23.46).
The meter discards any single timestamp PAIR over 100 ms
(`if (ms > 100.0) continue;`), but records the max over the per-frame SUM
of pairs. A boot-adjacent frame that arms many patch-upload pairs can
therefore post a max no individual pair could produce. Most likely that,
not a defect — but the number is misleading on its face and the asymmetry
between the per-pair discard and the per-frame max is real.

**`Draw with an index count of 0 is unusual`** — a Dawn warning from the
"Rasterized Scene" pass, emitted once. Consistent with what SHADOW_0
documented: species draw a high-water prefix rather than a live count, so
a family with nothing live still submits a zero-count draw. Harmless,
browser-visible, and it will appear in every audience console.

---

## 6. What this baseline settles, and what it does not

**Settles**

- The audience floor runs the program at **60 fps** with ~1.2 ms of CPU
  per frame. Product decisions should be made against this, not the 920M.
- **Pipeline compilation is a non-problem off FXC** — 10 ms for all 59.
- The web twin genuinely runs at **storage 8/stage**, so the binding
  budget work is load-bearing here even though the dev laptop hides it.
- `MAX_DEVICE_PIXEL_RATIO` removes 2.25× the fragment work on this device.
- `timestamp-query` IS available on Chrome/Android — GPU columns exist on
  the audience floor at all, which was an open question.

**Does not settle**

- **Any per-pass GPU budget on the phone.** Tiler, see §4b.
- **SHADOW_0's row 7** — the static/dynamic split inside `shadow_pass`.
  Still the measurement that bounds SHADOW_1, and this device cannot take
  it.
- **Anything about indoor.** Both windows are outdoor (`open_sunset`); no
  mood transition occurred. The 14 → 31 ms indoor multiplication SHADOW_0
  derived has **no web counterpart on record**, and the loop that causes
  it (one render pass per spot light) would be the single most interesting
  thing to capture on a phone next — a tiler pays differently for N passes
  than an immediate-mode GPU does.
- **Sustained thermal behaviour.** Two 30 s windows is not a session; a
  phone throttles and this capture is too short to show it.
