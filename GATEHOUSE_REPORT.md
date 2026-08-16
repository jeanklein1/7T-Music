# GATEHOUSE_0 — no class twice, nothing silent

Warranty round. Five units, six commits, no halt. Two warnings die, the
gate that let them through is rebuilt to read testimony, the class is
made unwritable, the cache door closes, and Firefox's price is written
where its PENDING line lives.

---

## Header

| field | value |
|---|---|
| HEAD before | `6498b1a` — PROBATE_SEAL6b |
| HEAD after | this commit (`GATEHOUSE_G6`) |
| branch | `master`, per O-1 |
| baseline (§G0) | `--check` **GREEN**; `wgsl_gate.py` **GREEN**; tree clean. The cartridge-TU gate: **did not exist** (GF1) |

| # | commit | subject |
|---|---|---|
| 1 | `469164d` | GATEHOUSE_G1: the EM_ASM empty strings speak double-quoted — two pp-token warnings die; the dev serve names its skip |
| 2 | `f213d23` | GATEHOUSE_G2: the TU gate reads the testimony — any warning is red, negative-controlled |
| 3 | `4e297fe` | GATEHOUSE_G3: EM_ASM lint — the empty single quote is unwritable |
| 4 | `ebc452e` | GATEHOUSE_G4: the index is no-cache — no viewer can load yesterday's build over today's |
| 5 | `712b44b` | GATEHOUSE_G5: the Firefox fallback is priced and held — the card is the boundary until naga or an exhibition says otherwise |
| 6 | this one | GATEHOUSE_G6: close |

---

## The two warnings, reproduced and killed

Before, with the build's own flag posture (no `-w`):

```
renderer.hpp:1471:81: warning: empty character constant [-Winvalid-pp-token]
    var s = (typeof SHADER_SHA === 'string') ? SHADER_SHA : '';
renderer.hpp:1476:56: warning: empty character constant [-Winvalid-pp-token]
    if (s.indexOf('__') === 0) s = '';
2 warnings generated.
EXIT=0
```

**`EXIT=0` is the whole of §G2.** The compiler answered the question
correctly and the gate read the verdict instead of the testimony.

After §G1: **0 warnings**, both TUs, zero diagnostics of any kind.
`'string'` and `'__'` stand — multi-character constants are valid
pp-tokens, and the minimal diff is law.

---

## Flags

| # | unit | site | expected | found | action |
|---|---|---|---|---|---|
| GF1 | §G0 / §G2 | "the cartridge-TU gate" | a gate in the tree to upgrade | **it did not exist.** PROBATE_SEAL's SF7 ran the cartridge TU *ad hoc* and deliberately did not pin it, reasoning that glaw1 already compiles that TU and a second ceremony would be the P12 mistake | `graduated` — that reasoning was half right and the expensive half was wrong. glaw1 does compile it, but glaw1 is Jean's and runs after the handoff; a check that exists only as a command I typed once is not a gate, and its result died with the shell that ran it. The TU gate is now in the tree with cartridge.hpp as a subject. **SF7 is hereby overruled by its own consequence:** the very next round found two defects in that TU. |
| GF2 | §G2.1 | the gate's blindness | one cause — exit status ignores warnings | **two causes.** Exit status is one. The other is that the ad-hoc invocation passed **`-w`**, and so does the committed console gate: it suppressed every diagnostic before the exit status ever mattered. Either alone was sufficient to hide both warnings | `graduated` — both closed. `-w` is gone and any `warning:` line is red. A gate that silences its instrument and then reports the silence is the purest form of the class this campaign kept collecting. |
| GF3 | §G2.1 | warning parity with glaw1 | recon the preset and align | the emcc line passes **no `-W` flags at all** — `T7_WEB_OPT` is `-O0 -g` or `-O2`, `T7_WEB_DIAG` is emscripten `-s` settings only. The build runs on **clang's defaults**, which is exactly where `-Winvalid-pp-token` lives, which is why Jean's build printed both on the first try | `graduated` — parity is therefore *no `-W` flags here either, and above all no `-w`*. Adding `-Wall -Wextra` would have been a different, stricter question than the build asks, and a gate that is stricter than the build produces findings nobody is obliged to fix. Chosen rather than assumed, and stated in the gate's header. |
| GF4 | §G1 | the dev-serve path | "recon what the digest check does with an empty expectation; add the behavior if absent" | **present but wrong-worded and over-broad.** SEAL's branch printed `expected=(none) UNWITNESSED — no baked digest (native, or a page web_dist did not generate)`, folding two different situations into one sentence | `graduated` — replaced with the required line for the web case, and the native case split out to its own. Under `__EMSCRIPTEN__`: `expected=none (dev serve — placeholder unsubstituted) SKIPPED`. Native: `expected=none (native — read off disk, no serve to witness) SKIPPED`. The floor does not stop on either, and neither is silent. |
| GF5 | §G2.3 | pre-existing warnings the gate now surfaces | none expected | **none found.** After §G1, both TUs compile with zero diagnostics | no action — the expectation held. Zero-warning is the standing state from `f213d23` onward. |
| GF6 | §G2 (naming) | the gate's directory | — | it is still `tools/gates/console_gate/`, though the gate now compiles two TUs and reports as `tu-gate` | `skipped-step` — the stubs and `PROVENANCE.md` live in that directory and are cited by path elsewhere; renaming would cost every one of those references to buy a tidier path. The header says the directory name is historical and the gate is the script. Jean's call whether to pay for the rename. |
| GF7 | §G6 (standing) | the shader digest | — | it moved **again** — §G5's banner sentence edited `world.wgsl`, so `637327 / b0c081ba` became `637679 / 3cf2e02f` | no action, and this is the witness working. Recorded because SEAL's SF9 caught this report's ancestor hard-carrying a stale digest into a checklist: **no digest is written down as a constant anywhere in this report either.** The checklist names the line `web_dist.py` prints. |

---

## Negative controls

Every arm added this round was planted against, per EF1's law. Control
results are rows here, not comments in a script.

| control | arm under test | planted | result |
|---|---|---|---|
| **A** | §G3 lint | `''` restored inside the §S2 EM_ASM body | **FAIL**, exit 1, naming `renderer.hpp:1479` and the offending line, **before clang ran** |
| **B** | §G2 warning-red | `#warning GATEHOUSE negative control` in renderer.hpp — a diagnostic the lint provably cannot see | **FAIL**, exit 1, `cartridge.hpp TU: 1 warning(s)`. Arm 2 proven independent of arm 1 |
| **B′** | §G2 warning-red, *lint arm absent* | `''` planted while the gate was at its §G2-only state | **FAIL**, exit 1, surfacing the real `-Winvalid-pp-token`. Proves §G2 alone would have caught the class §G3 now forbids — the two arms are belt and braces, not one mechanism counted twice |
| — | restore | every plant reverted | **PASS**, exit 0, both TUs, zero diagnostics |

**Why B′ matters.** Without it, §G3's lint would mask §G2's warning arm
forever after, and a later regression in the warning arm would be
invisible — the exact shape of a witness testifying for the wrong side.

---

## Numbers

| quantity | §G0 baseline | §G6 close |
|---|---|---|
| TUs compiled in the tree's own gates | 1 (`console.hpp`) | **2** (`+ cartridge.hpp`, glaw1's own) |
| warning flags passed by the gate | `-w` — all suppressed | **none** — clang defaults, parity with the emcc line |
| diagnostics on the cartridge TU | 2 warnings, exit 0 | **0** |
| gate arms | 1 (compile) | **2** (lint, then compile) |
| gates in the tree | 3 (`--check`, wgsl, console) + sha256 | **4**, one of them widened |
| `dist/` files | 66 | **67** (`+ _headers`) |
| index cache posture | default (a cached index pins every stale key behind it) | **`no-cache`** — revalidate, not refuse-to-keep |
| laws | L28 | L28 |
| Table H / Table D | 68 / 14 filled | **68 / 14 filled**, zero warnings |
| `E1-identity` | PASS | **PASS** |

### `dist/_headers`, byte-exact

```
/
  Cache-Control: no-cache
/index.html
  Cache-Control: no-cache
```

Nothing else is listed, and that is the design: the versioned assets keep
default caching **because** a fresh index always names fresh `?v=` keys.
Busting them too would discard the whole point of BUILDID_0.
`no-cache`, not `no-store` — a 304 on an unchanged index is free and
correct; what must never happen is the browser answering from its own
copy without asking.

---

## Per-unit status

| unit | status | one line |
|---|---|---|
| §G0 | **done** | Baseline green; the gate §G2 was to upgrade turned out not to exist (GF1). |
| §G1 | **done** | Two characters, two warnings gone; `'string'` and `'__'` untouched. The dev serve names its skip, and the native case got its own sentence (GF4). |
| §G2 | **done** | Gate widened to both TUs, `-w` removed, any `warning:` red. Parity chosen from the emcc line, not assumed (GF3). Controls B and B′. |
| §G3 | **done** | Lint arm before clang; the doctrine line verbatim in the gate's header. Control A. |
| §G4 | **done** | `dist/_headers` on the refusal-safe path; one deploy line added. |
| §G5 | **done** | The price written into the banner, never-adapt; old clause verbatim to the record. |
| §G6 | **done** | Ledger regenerated; all gates green; this report. |

---

## The gate ledger — one line per class that ever reached Jean

| class | how it reached him | what stands there now |
|---|---|---|
| FXC ghosts | prose defending shapes on a retired compiler | probated; lessons in one record |
| naga stillborn | a per-commit gate that could not parse the module, failing open for a campaign | `wgsl_gate.py`, pinned transform, negative-controlled |
| retired-name filter | `roomLayout_` renamed by LOOM_2; `max(…, default=0)` printed a confident `0 of 8` | identity keys + `E1-identity`, negative-controlled |
| hand-carried numbers | a preamble calling 74 "the ruling of record" while the index held 68 | computed at emission; history in the record |
| the serve | a shader cut 665 bytes from its end, every tree gate green | hashed at the door, both ends held together by a gate |
| the floor | narrated, then compiled the shader it said could not compile | stops before any GPU object; the audience meets a card |
| a TU never compiled | `renderer.hpp` edited for three rounds, read by nothing | the cartridge TU is a gate subject |
| warnings unread | the gate read the verdict, not the testimony — and passed `-w` | any `warning:` is red, parity with the build |
| EM_ASM pp-tokens | `''` lexed as an empty character constant | unwritable — lint fires before clang |
| stale index | a cached page pinning every stale key behind it | `no-cache` on the index alone |

What remains can only arrive once, loudly, wearing its name.

---

## Jean's checklist

1. **Pull, build.** The build output is the witness for §G1/§G2: **zero
   warnings where two stood.** If any warning appears, the TU gate
   disagrees with your emcc and GF3's parity claim is what to re-read.
2. **Deploy.** `web_dist.py` prints the shader sha and the exact `[Dist]`
   line to expect, then confirms `_headers`: *index is no-cache; a plain
   reload now fetches the current build*.
3. **Pixel — hard-refresh ONCE more.** This one last time, to break the
   old cached index; §G4 makes it unnecessary from then on. Read in
   order:
   - `[Dist] world.wgsl sha=… expected=… MATCH` — both halves equal to
     the sha `web_dist.py` printed at deploy. **Do not check it against
     any number written down here**; it moved twice during this round
     alone (GF7), by design.
   - `[Shader] world.wgsl compiled: 0 error(s)`
   - zero validation errors
   - **the terrain word on the pinned seed** — three rounds outstanding,
     and the last open item of PROBATE_I. If it is wrong the words are
     *"the whole terrain is the same hill repeated"* or *"everything
     outside the first patch went flat"*.
4. **Firefox needs nothing.** The card is witnessed and standing, and its
   price is now written in the banner beside the PENDING line.
5. **One thing wants your eye:** GF6 — whether to pay for renaming
   `console_gate/` to match what it now is.

---

*What remains after this is the art, which is yours and SOAK's, as it
should be.*
