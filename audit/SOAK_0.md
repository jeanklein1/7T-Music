# SOAK_0 — THE PURSE TABLE

The reading HEADROOM_0's envelope row was built to produce. Fourteen
meter windows across one continuous session, both arms, four moods, six
portal transitions. No source was edited to produce this file.

## PROVENANCE — and how the build was pinned

| field | value |
|---|---|
| date | 2026-08-13 |
| device | NVIDIA GeForce 920M (discrete), driver 425.31 |
| backend | **Vulkan** (adapter 3 of 7) |
| Dawn | `f0bf8ab547a9a23b8b78ff67d8085d4a26600a7d` |
| build | `the-board-full-release-meter`, Release, `T7_INSTRUMENTS=meter` |
| budget | 16.6 ms (60 Hz) |
| windows | 14 |
| adapter limits | storageBuffers/stage **10**, uniformBuffers/stage 12, bindingsPerGroup 1000 |

**The build is at or after `dddb30d`** (TOGGLE_0 U2), established from
four independent marks in the log rather than from anyone's memory:

| mark in the log | what it pins |
|---|---|
| `Toggles used (9)`, **no `disable_symbol_renaming`** | at or after `dddb30d` — the control is retired. A pre-`30c9a7c` build also reads 9, but the three marks below rule that out. |
| `textures 237.9 MiB`, `TOTAL 251.1 MiB`, `5. 8.0 MiB Shadow Map`, no `UNDERCOUNT` | FORMAT_1 (`depth16unorm` + the `texel_bytes` case) |
| `envelope mean … max … -> purse …` in every window header | HEADROOM_0 U1 |
| every `[CENSUS]` carries `trigger=boot` or `trigger=mood-transition`; **no periodic census anywhere in 14 windows** | CLOSE_0 A5 / HEADROOM_0 U3 — `census_entity_dump` is false under `meter` |

Docs-only commits after `dddb30d` leave no mark in a boot log, so this
pins the floor, not the exact commit. Nothing in the binary differs
between `dddb30d` and `6a08357`.

## THE PURSE TABLE

`purse = 16.6 − envelope mean`. Negative throughout: **the GPU alone
overruns a 60 Hz budget in every window of the soak.**

| # | arm | frames | fps | env mean | env max | **purse** | shadow gpu | main gpu |
|---|---|---|---|---|---|---|---|---|
| 1 | outdoor | 559 | 17.7 | 50.00 | 83.17 | **−33.40** | 13.93 | 30.58 |
| 2 | outdoor + transition | 550 | 17.3 | 51.76 | 128.45 | **−35.16** | 15.52 | 30.96 |
| 3 | outdoor | 577 | 19.1 | 49.06 | 49.61 | **−32.46** | 14.02 | 31.42 |
| 4 | outdoor | 570 | 19.0 | 49.96 | 56.69 | **−33.36** | 14.04 | 32.06 |
| 5 | outdoor | 553 | 18.4 | 50.80 | 94.83 | **−34.20** | 14.24 | 31.48 |
| 6 | outdoor | 562 | 18.8 | 50.16 | 58.79 | **−33.56** | 14.17 | 30.69 |
| 7 | → indoor vault | 508 | 16.1 | 56.63 | 224.92 | **−40.03** | 19.57 | 31.79 |
| 8 | **indoor** (vault, 4) | 378 | 12.4 | 76.84 | 79.89 | **−60.24** | **42.10** | 33.00 |
| 9 | **indoor** + transition | 374 | 12.1 | 77.50 | 81.59 | **−60.90** | **42.10** | 33.64 |
| 10 | **indoor** (flat, 4) | 377 | 12.6 | 77.69 | 106.36 | **−61.09** | **42.35** | 33.34 |
| 11 | indoor → outdoor | 371 | 11.0 | 79.92 | 148.77 | **−63.32** | 40.87 | 33.73 |
| 12 | outdoor | 537 | 17.9 | 52.47 | 92.73 | **−35.87** | 14.35 | 32.01 |
| 13 | outdoor | 549 | 18.2 | 52.37 | 90.24 | **−35.77** | 14.24 | 31.06 |
| 14 | finite outdoor | 605 | 19.3 | 46.15 | 65.80 | **−29.55** | 13.12 | 29.59 |

GPU milliseconds, per-frame sum, mean over the window.

### The two steady states, transition windows excluded

Clean outdoor is windows 3, 4, 6, 12, 13. Clean indoor is 8 and 10 —
window 9 carries a vault→flat transition and window 11 an exit, so
neither is a steady state.

| | outdoor | indoor (4 lights) | ratio |
|---|---|---|---|
| envelope mean | **50.80 ms** | **77.27 ms** | 1.52× |
| purse | **−34.20 ms** | **−60.67 ms** | — |
| fps | 18.6 | 12.5 | 0.67× |
| shadow pass gpu | **14.16 ms** | **42.23 ms** | **2.98×** |
| main pass gpu | 31.45 ms | 33.17 ms | 1.05× |
| compute (dispatch + card + mesh) | ~4.1 ms | ~1.7 ms | 0.41× |

**The envelope reconciles.** Outdoor: 14.16 + 31.45 + 4.1 ≈ 48.7 against
a measured envelope of 50.80. Indoor: 42.23 + 33.17 + 1.7 ≈ 77.1 against
77.27. Two independent instruments — per-pass timestamp pairs and the
first-to-last envelope — agree to within 2 ms and 0.2 ms. That is the
envelope row's own witness, and it passes.

## THE FINDING — the shadow pass is no longer bandwidth-bound

This is the reading that changes what the next shadow campaign should
be, and it falls straight out of the two ratios above.

Post-ATLAS_1revB and post-FORMAT_1, shadow attachment traffic is 8 MiB
outdoor (one texture, `Clear`+`Store`) and 16 MiB indoor at four lights
(two textures). **A bandwidth-bound pass would show a 2.0× ratio. The
measured ratio is 2.98×.** Bandwidth alone cannot explain the indoor
cost any more — roughly a third of it now sits outside the attachment
term.

What remains is the per-light redraw: one pass per texture still draws
the whole caster set once per light it owns, under per-light viewports.
Four lights, four traversals. Indoors that set is already reduced —
`draw_shadow_all(cast_terrain=false)`, the UMBRA_4 cut — which is why
the ratio lands near 3× rather than at 4×.

**So the lever moved.** PASS_0 named the indoor shadow arm as the frame's
biggest bandwidth item and ATLAS + FORMAT took that from 96 MiB/frame to
16. What is left is geometry submitted four times, and no format or
store-op ruling touches it. The candidates are per-light caster culling
(a light's frustum sees a fraction of the room), instancing the caster
set across lights, or reducing the atlas resolution for the three
lights that are not the primary. None of those is priced here; this
census only establishes which term is now dominant.

Second reading, smaller: **the main pass is arm-indifferent** — 31.45
outdoor against 33.17 indoor, a 5% spread across two completely
different scenes. It is resolution-bound, not content-bound, which makes
the DPR knob (HEADROOM_0 U2, still unbuilt) the only lever with leverage
over the frame's single largest GPU row.

## WHAT THE SOAK CONFIRMS ABOUT THE INSTRUMENTS THEMSELVES

**A5 landed and is measurable.** No periodic `[CENSUS]` appears in 14
windows — every census in the log is `trigger=boot` or
`trigger=mood-transition`, both of which are doctrine and outside the
dial by design. The `census_dumps` row is correspondingly down: it read
`max 1178.39 / 957.51 / 713.54` before the split, and reads 45–230 ms
here with one 353 ms outlier, all of them on windows carrying a
transition census or the `[METER]` table's own ~40-line blocking print.

That residue is the instrument reporting on itself, and it is the
honest floor: the table cannot print without a blocking write. It
distorts `max` and leaves `mean` alone, so read the means.

**The frame is entirely GPU-bound on this device.** No CPU row exceeds
0.5 ms mean in any window. `present` runs 46–78 ms and tracks the
envelope almost exactly — that is the CPU blocking on a GPU that needs
50–77 ms per frame under `PresentMode::Fifo`. Every CPU-side cost in the
program is, on this hardware, free.

## THE HONEST GAP

There is **no pre-campaign baseline in this session to diff against.**
The earlier boot this session (`9489b8d`) already carried ATLAS_1revB
and FORMAT_1, and its one window read envelope mean 57.77 / max 137.89 /
purse −41.17 with a mixed arm. The 96 → 16 MiB figure is arithmetic
under PASS_0 Q3's stated model, confirmed structurally in the tree, and
**not** measured against a before. Producing one would cost a build at a
pre-ATLAS commit and a matching walk; nothing currently in the queue
needs it badly enough to be worth the boot.
