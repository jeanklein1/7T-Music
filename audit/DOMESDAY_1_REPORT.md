# DOMESDAY_1 — the batch report

Rulings enacted, the estate spent into. Stream A landed on `master`;
Stream B waits on `claude/domesday-1` for Jean's gates (glaw1 +
visual + merge). Expected visual delta of the whole branch: none,
except B7's accepted ≤100 ms scale-softness during resize animation.
`binding_gen.py --check`, `binding_ledger.py --check` and
`command_census.py --check` are fully green at both tips. No compile
ran in this environment; glaw1 is the batch's compile gate, and B6 is
the one unit that may bounce a spelling off it (its error text comes
back as the correction — do not guess twice).

**Before any unit ran:** the handoff premised "the branch is merged";
the repository had `claude/domesday-0` fully green but unmerged. The
merge was performed as the enacting step (`2f23916`, a no-conflict
merge commit — history preserved), and all three global gates passed
on the merged tree before the first edit.

## §1 — unit table

| unit | verdict | where |
|---|---|---|
| A7 — stale-prose sweep | **LANDED** `efa0b38` | `master` — the three _0-report sites, plus the copy-kernel comment's own retired 413/414 numbers (same species, disclosed) |
| A8 — feature names (R6) | **LANDED** `4d3af67` | `master` — `feature_name()` switch over 20 `wgpu::FeatureName` enumerator identifiers; paired `[Device] features named:` line; unknown ids print as numbers. glaw1-gated: any enumerator it rejects gets removed in a follow-up. |
| A9-fix — label law, subject | **LANDED** `83bcfbb` | `master` — the two unlabeled `CreateCommandEncoder` sites named `"frame"` and `"flush_zone_derive_requests"`; the 20 pass descriptors were already labeled |
| A9-wit — label law, instrument | **LANDED** `2ea07c7` | `master` — witness C-7 (2 encoders + 20 passes, zero unlabeled) + the encoder table in COMMAND_LEDGER §2 |
| B5 — plan-group collapse (R2) | **LANDED** `0c8b923` | `claude/domesday-1` — the byte-compare answered the open half: the Photographer group is identical too, so all three redundants collapsed (GROUPS 37→34) |
| B7 — reconfigure debounce (R4) | **LANDED** `ffdd140` | `claude/domesday-1` — six stable frames; boot untouched; FRAME_1 kept as the acceptance witness |
| B8 — snapshot → Exhibition (R5 first half) | **FLAGGED** (zero commits) | expected a pass-through hop ("slot bookkeeping now marks at capture; behavior: none"); found a **speculative portfolio with hang-time curation** — `capture_snapshot` overwrites a rolling 32-layer ring as the pawn walks ("cursor wraps freely, unconditional overwrite", `gallery.hpp`), and galleries select at commit among all unconsumed candidates (mono-tier filter to Panoramic/Portrait/Cinematic + chronological sort, `gallery.hpp` commit path). The pixels must persist between capture and selection, and that persistence IS the staging pool: rendering into the Exhibition at capture would park up to 32 candidates in the shared 40-layer array (starving hanging, saving nothing) or change which shots hang. The retirement needs a photographer-model ruling — capture-on-demand, or portfolio-in-exhibition with eviction — which is Jean's, not mechanical. The −32 MiB does not land in this batch. |
| B9 — parameter surface | **LANDED** `051c37e` | `claude/domesday-1` — `src/core/boot_params.hpp`; LANTERN §L2 read in full, no constraint conflicts (URL params had zero readers, P11) |
| B6 — shadow_slot → immediate (R3) | **LANDED** `f8defbd` | `claude/domesday-1` — the API-frontier unit, last by design; spellings per the statute and the handoff's Dawn convention (`var<immediate>`, `PipelineLayoutDescriptor::immediateSize`, `SetImmediates(offset, data, size)`), glaw1 arbitrates |
| B-close — regenerate | **LANDED** `3439eb2` | `claude/domesday-1` — content current from B6's own gates; the pass restamps the source-commit handles |

## §2 — the ruling record (verbatim from the handoff header)

*Delegated by Jean, ruled by Claude, enacted here.*

> - **R1 — orb ping-pong→bind-group-swap: DECLINED.** L23′'s letter
>   (LAWS.md:577) bars mixed-writability faces of one buffer sharing a
>   layout; its pessimism clause prices any relaxation at a witnessed
>   Dawn behavior test; the recoverable purse (two seats in a lane with
>   six free, one two-workgroup dispatch) does not pay for the test.
>   The copy pass is the law's price.
> - **R2 — plan-group collapse: ADOPTED** (unit B5).
> - **R3 — shadow_slot → immediate: ADOPTED, single path, browser floor
>   raised** (unit B6). Jean's stated principle governs: *impress all
>   who can open the site rather than mildly impress everybody.* No
>   dual path; `fallback()` catches the ancient.
> - **R4 — reconfigure debounce: ADOPTED, K = 6 stable frames**
>   (unit B7).
> - **R5 — staging retirement: snapshot half ADOPTED now** (unit B8);
>   authored half is DOMESDAY_2, alone.
> - **R6 — feature names by enumerator switch: ADOPTED** (unit A8).

Enactment notes: R1's declined alternative is now pinned in-tree at
the orb copy kernel (A7 item 3) so it cannot be innocently remade.
R5's first half FLAGGED on census — see §1; the ruling's premise
("same pixels, one hop instead of three") did not survive contact
with the photographer's portfolio model, and the authored half
(DOMESDAY_2) should be scoped with that same fact in hand: **both**
staging pools are inventories, not pipes — the authored pool holds
fetched paintings across world rebuilds the same way the snapshot
pool holds candidate shots across the walk.

## §3 — held, named, untouched

Caster instancing · stream_patches amortization · render bundles ·
wider indirect · **MSAA** — held with its coupling: main-pass
multisampling forces a pipeline `multisample.count` shared with the
snapshot pass, so it must be designed *with* the exhibition-direct
render path (the B8 redesign), not before it.

## §4 — wallet movement (branch tip, regenerated MANIFEST)

| lane | batch start | batch end |
|---|---|---|
| uniform | 11 / 12 — agents C | 11 / 12 — unchanged; **main render V 9→8 of 12** (`patchTerrainPipeline_` V row), gallery V 5→4, shadow V 8→7 |
| storage | 5 / 8 — agents C | 5 / 8 — unchanged; scene V stays 3, shadow V 4 |
| dyn_u / dyn_s | 1 dynamic-offset binding (the program's only one) | **0 / 8 and 0 / 4 program-wide** — witness `0d-1`: "dynamic-offset bindings: 0 of 8 uniform, 0 of 4 storage"; the whole machinery (window buffer, strided writes, offset arguments) left the program |
| immediates | 0 / 64 | first spend: 4 B (the shadow family's `u32` light index). **Not yet visible in MANIFEST's immediates column** — the schema relations carry no immediate-size fact, so the A1 emitter still prints 0/64; teaching PIPELINES the field is follow-up instrument work, recorded in §6. |
| estate | — | **−32 MiB did not land** (B8 flagged); B6's retired window buffer returns 1 KiB, below the budget print's noise floor |

## §5 — the soak arms (data, not code)

The parameter surface makes an arm addressable for the first time:
`?seed=&mood=&cap=` (web) / `--seed= --mood= --cap=` (native), read
once at boot, `[Params]` line as the witness. Canonical arms — the
authored world (seed 42, `DEMO_SEED`) across all four moods:

| arm | seed | mood | names the mood |
|---|---|---|---|
| S0 | 42 | 0 | OPEN_SUNSET |
| S1 | 42 | 1 | INDOOR_FLAT |
| S2 | 42 | 2 | INDOOR_VAULT (the four-light vault §L2 could not reach on demand) |
| S3 | 42 | 3 | FINITE_OUTDOOR |

Cap values worth pricing, per arm: **1.5** (the compile-time
baseline — the control) and **2.25** (the native-density candidate —
the purse question). Example: `?seed=42&mood=2&cap=2.25`. A second
seed (any fixed number) re-runs the walk on a fresh world when
generality is in question; the arm table above is the repeatable
core.

## §6 — anything unexpected, one line each

- **The merge premise**: the handoff said "the branch is merged"; the
  repo said otherwise — merged as the enacting step (`2f23916`),
  gates green before any edit.
- **B8's model mismatch** (§1): the snapshot staging is a curated
  portfolio, not a hop; the R5 second half should be re-scoped with
  this fact.
- **B5's open comparison closed the strong way**: the Photographer
  scene group was byte-identical too, so four groups became one, not
  three.
- **`binding_ledger.py` taught twice**: the `strataLayoutFor`
  call-site regex learned the optional `immediateSize` argument (B6);
  witness `0c-0c` caught the gap before it could ship — the
  instrument chain worked as designed.
- **MANIFEST's immediates column still reads 0/64** after the lane's
  first spend — the schema carries no immediate-size fact for the A1
  emitter to derive; adding it (PIPELINES field + tree capture + both
  emitters) is named follow-up work, not smuggled in here.
- **B6 and A8 are glaw1-gated on spellings**: `var<immediate>`,
  `immediateSize`, `SetImmediates`, and the 20 feature-name
  enumerators were authored from the statute and the handoff's stated
  Dawn conventions, uncompiled here; a rejection's error text is the
  correction.
- **B7 changed what COMMAND_LEDGER §3 quotes**: the census now
  captures the settle-gated inner branch as the reconfigure trigger —
  which is the truth; C-3 still anchors on the FRAME_1 call.

One line for the spirit: **five rulings enacted, one refused by the
tree itself — the lane took its first coin, the dynamic machinery is
gone from everything but memory, and the walk finally has an address
for every arm it needs to price.**
