# WEB_METER_0 — THE FIRST AUDIENCE-FLOOR READING

The first `[METER]` windows off a real phone. Captured 2026-08-12 by Jean
on a **Pixel 8**, Chrome, over USB (`chrome://inspect`), against the
deployed preview build of the `the-board-web-meter` preset (`ce6d53d`).

This is a **baseline record**, not a campaign. Nothing was changed to
produce it. Its job is to stop the next handoff reasoning about product
performance from the 920M's numbers.

**Two captures, same device, same build, same day.** Sections 1–6 are
capture 1 (two outdoor windows, hands-off). **Section 7 is capture 2**
(five windows, with a portal walk into `indoor_vault` at t≈93) and it
closes the gap §6 named as this document's principal omission.

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

> **Amended by §7e.** That last sentence is too generous to SHADOW_1.
> Capture 2 shows this row does not scale with light count on this device,
> which is the specific redundancy SHADOW_1 exists to remove. Read §7b–7e
> before quoting this paragraph.

---

## 5. Two anomalies worth recording

**`stream_patches gpu max 1323.04 ms`** in window 1 (window 2: 23.46).
The meter discards any single timestamp PAIR over 100 ms
(`if (ms > 100.0) continue;`), but records the max over the per-frame SUM
of pairs. A boot-adjacent frame that arms many patch-upload pairs can
therefore post a max no individual pair could produce. Most likely that,
not a defect — but the number is misleading on its face and the asymmetry
between the per-pair discard and the per-frame max is real.

> **Capture 2 reproduces it**: `stream_patches gpu 1.72/1198.06` in its
> window 1, and `0.31/255.52` in the transition window — the two frames in
> the whole session that arm the most patch pairs. Twice, in the two places
> the explanation predicts, is enough to call the explanation confirmed and
> the row not a defect. It is still a misleading number to publish.

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
- ~~**Anything about indoor.**~~ **CLOSED by capture 2 — see §7.** Both
  of capture 1's windows are outdoor (`open_sunset`) and no mood
  transition occurred, so when this section was written the 14 → 31 ms
  indoor multiplication SHADOW_0 derived had no web counterpart. Capture 2
  walked a portal into `indoor_vault` and got one. **The prediction stated
  here — "a tiler pays differently for N passes than an immediate-mode GPU
  does" — held, and more strongly than expected.**
- **Sustained thermal behaviour.** Two 30 s windows is not a session; a
  phone throttles and this capture is too short to show it. Capture 2's
  five windows (150 s) do not show throttling either, but they do show an
  unexplained upward drift in two GPU rows — §7d.

---

## 7. CAPTURE 2 — THE INDOOR WINDOWS

Same device, same build, same day. Five windows instead of two, and at
t≈93 Jean walked a portal:

```
[Portal] GPU trigger: arch 1 -> seed=1909815867 finite=1
[Lighting] Quartet (4 lights, E/W walls)
[Mood] Indoor palette: slate blue (idx=1)
[Shell] Generated GROIN VAULT: 1105 verts, 6168 indices
[Mood] Applied: indoor_vault (mood=2 INDOOR)
```

The trigger lands between window 3 and window 4, so windows 1–3 are
`open_sunset` and windows 4–5 are `indoor_vault`. **Quartet = 4 lights =
4 spot atlas tiles**, against outdoor's single sun pass.

This is the exact case SHADOW_0 §Q4 flagged as unmeasured: it derived the
native indoor number from `indoor_flat` / **Cathedral / 3 lights**, and
closed by predicting *"`indoor_vault` (Quartet, 4 lights) should read ~41 ms
on the same reasoning… a prediction the next meter boot can settle for
free."* It is settled here on the wrong platform to confirm it, and that
is the whole finding.

### 7a. The rows, all five windows

GPU means in ms (the caveats of §4 all still apply — quantized to 100 µs,
non-additive).

| row | w1 30.1 out | w2 60.1 out | w3 90.1 out | **w4 120.1 IN** | **w5 150.1 IN** |
|---|---|---|---|---|---|
| **fps** | **50.8** | **58.8** | **60.0** | **60.0** | **60.0** |
| `frame_total` cpu | 1.21 | 1.24 | 1.20 | 1.28 | 1.26 |
| `shadow_pass` gpu | 8.79 | 8.67 | 8.13 | **8.17** | **8.05** |
| `shadow_pass` **cpu** | 0.12 | 0.15 | 0.14 | **0.29** | **0.28** |
| `main_pass` gpu | 10.61 | 10.40 | 9.81 | 8.63 | 7.51 |
| `orb_sky` gpu | 7.09 | 8.39 | 9.15 | 0.83 | **0.00** |
| `gol_zone_compute` gpu | 7.02 | 8.31 | 9.07 | 5.91 | 5.04 |
| `frustum_cull` gpu | 3.63 | 4.27 | 4.65 | 3.02 | 2.58 |
| `dispatch_compute` gpu | 3.45 | 4.09 | 4.47 | 2.89 | 2.46 |
| `live_card_write` gpu | 1.02 | 1.12 | 1.21 | 0.86 | 0.62 |
| `stream_patches` gpu | 1.72 | 0.34 | 0.31 | 0.31 | — |

### 7b. THE FINDING — the tiler does not pay the per-pass multiple

**`shadow_pass` GPU is flat across the transition: 8.13 outdoor → 8.17 →
8.05 indoor.** On the 920M the same row went **14.2 → 30.84 ms at three
lights**, a 2.17× jump, with ~41 ms predicted at four.

Restate it with the structure in view, because the flatness is stronger
than the bare numbers look. `meter_arm_render` allocates a fresh timestamp
pair per call and the callback does `frame_ms[row] += ms`, so:

| | outdoor | indoor (`indoor_vault`) |
|---|---|---|
| render passes on this row | **1** | **4** |
| what the number is | one bracket | the **sum of four** brackets |
| target per pass | 2048 × 2048 | 1024 × 2048 |
| total raster area | 4.19 Mpx | **8.39 Mpx** (2×) |
| terrain casts | yes | no (UMBRA_4) |
| **Pixel 8 reads** | **8.13** | **8.05–8.17** |
| **920M reads (3 lights)** | 14.2 | 30.84 |

Four summed brackets over twice the raster area come out equal to one
bracket. Whatever the phone is charging for a shadow pass, it is not
charging per pass.

**The CPU column proves the four passes are really there.** `shadow_pass`
cpu goes 0.14 → 0.28/0.29 — it roughly doubles, which is the encode cost
of four pass records instead of one, and it is the only row in the table
that moves *up* going indoors. So this is not a case of the indoor branch
failing to run. It runs, the CPU pays for it, and the GPU column does not
move.

### 7c. What this does NOT establish

Three confounds, all real, and none of them explain the platform contrast
away:

1. **It is not the same scene.** Indoor is a finite 9×9 world under a
   groin vault with 4 spawned agents; outdoor is a streaming world with
   11. Every other GPU row falls going indoors (`main_pass` 9.81 → 7.51,
   `orb_sky` 9.15 → 0.00 — there is no sky under a vault). Some of
   `shadow_pass`'s flatness is a lighter caster set, not tiler magic.
2. **The spot tiles skip terrain** and each rasterizes half the map.
3. **§4b bites hardest exactly here.** On a tiler a pass bracket is not
   exclusive occupancy, and a *sum of four overlapping brackets* is less
   trustworthy than a single one, not more. "4 passes cost what 1 costs"
   is also consistent with "these brackets do not measure what you want."

**But confounds 1 and 2 applied identically on the 920M, where the number
still tripled.** SHADOW_0 did that arithmetic explicitly: it found the
spot tile costs 72% of the full sun pass *despite* half the pixels and no
terrain, and named the residue as per-caster fixed cost repeated per
light. That residue is what the Pixel is not paying. The contrast between
platforms is the datum; the absolute claim "N passes are free on a tiler"
is not established and should not be quoted.

### 7d. Two more things the five windows show

**Mood transitions do not cost a frame.** Window 4 contains the whole
portal walk — teardown, seed change, vault mesh generation, 25 wall
paintings placed, agent respawn — and its `frame_total` **max is 9.60 ms**,
the *lowest* max of the five windows (w1 20.00, w2 16.20, w3 15.60,
w5 9.60). The most expensive single CPU tick in the capture is
`transition_machine max 5.80 ms`, comfortably inside a 16.6 ms budget.
The transition is invisible on the audience floor.

**An upward drift across the three outdoor windows, unexplained.**
`orb_sky` 7.09 → 8.39 → 9.15, `gol_zone_compute` 7.02 → 8.31 → 9.07,
`frustum_cull` 3.63 → 4.27 → 4.65, `dispatch_compute` 3.45 → 4.09 → 4.47
— four rows climbing ~30% over 90 s while fps *improves* 50.8 → 60.0.
Rising cost with rising frame rate is not throttling. Two candidate
readings, neither tested: the exhibition is still streaming in, so these
rows have genuinely more to do each window; or the GPU clocks up as the
frame stops being CPU-starved and the brackets widen. Capture 1's two
windows were flat (`orb_sky` 6.61 → 6.67), so this is not a fixed property
of the build. **Recorded as an open question, not a finding.**

**Reproducibility, incidentally, is excellent.** Capture 1's cold window
read 50.6 fps / `frame_total` 1.18 ms; capture 2's read **50.8 fps /
1.21 ms**. The settled windows agree to within 0.1 fps. Boot varies more
(total init 33 ms vs 59 ms) but both are three orders of magnitude below
the native 8,314 ms and the variance does not matter.

### 7e. Consequence for SHADOW_0

SHADOW_0's precondition matrix **row 4 — "the measurement justifies the
work" — is graded GO on native numbers**: *"14.2 ms outdoor, 30.8 ms
indoor at 3 lights, ~41 ms predicted at 4… the second-largest GPU line and
the largest that is structurally redundant."*

That grade is **platform-specific and should be read as such.** On the
audience floor the row it is grading does not scale with light count, sits
at 8 ms in a frame that finishes its CPU work in 1.3 ms, and lives on a
device that is hitting 60 fps in every window measured — indoors included.

This does not retire SHADOW_1. The static/dynamic caster split is still
structurally redundant work, it still costs real time on the immediate-mode
twin, and row 4's *native* reading stands. But the sentence "shadow is the
thing to fix" is not true of the phone, and any handoff that opens SHADOW_1
should say which machine it is optimizing before it says how much it saves.

**Row 7 is still open and still the binding constraint** — the
static/dynamic split inside the pass. §4b rules this device out for
answering it, and 7b's flatness makes the phone *less* able to answer it,
not more: a mechanism that cannot distinguish 1 pass from 4 will not
distinguish 9 static caster families from 4 dynamic ones.
