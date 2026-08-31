# LIGATURE_0 — recon report

Read-only census of the musical coupling ligature, taken at
`79adfa4d26c9e17e0074692928f1d2875d7edde1` on branch
`claude/ligature-0-recon-hcrix0`.

This report proposes nothing. Under the campaign's standing rulings it records
what exists, with the command that produced each number (R2), cited by symbol
rather than line (R3), quoting verbatim where the unit asked for a slice (R5),
and anchored to blob SHAs so a later reader can tell a stale quotation from a
live one (R6). Gaps are recorded as gaps (R4). No source file was modified; the
only write is this file (R1).

## The one-paragraph finding

The two-sided named-resolution system is **intact on the target side and empty on
the source side**, and the tree says so itself. `VisualCanvas::bind` resolves eight
pipes against the static `PARAM_LAYOUT` — all eight succeed, all eight are flushed
every frame, their setters run and their GPU fields are read by the shader — and
twelve musical stats against whatever `StatLayoutView` the console hands it. What
the console hands it is `BeatClock::stat_layout()`, which returns
`StatLayoutView{ nullptr, 0 }` by construction. So all twelve source resolves miss,
every coupling disables itself through `SignalLayout::resolve`'s graceful path, and
every pipe holds its `rest` forever while the flush machinery downstream runs
correctly every frame on those rest values. **The ligature is one hop: the empty
layout handed to `bind_signal_layout` in `the_board.cpp`.**

## The second finding, and the one that sets the plan's scope

Nothing needs resurrecting. The analysis arm that would fill that socket —
`analysis/canvas_1`, `MidiPort`, `stream_data`, `field`, `wagon`, `playhead`,
`RtMidi` — was deleted by `CUT_1c` (`1a52f2db`, 2026-08-05) and restored at
`0c951b11` / `e0e22e46` (2026-08-30) **byte-identically**: of the 32 files under
`src/musical/`, `src/sources/`, `src/analysis/` and the two `RtMidi` files at HEAD,
30 carry a blob equal to their blob at `1a52f2db^`, one (`beat_clock.hpp`) did not
exist before `CUT_1c` because `CUT_1c` created it, and one (`signal_layout.hpp`)
diverged — and that one was never deleted, so it is not part of the restored set
(PORT_4c changed it in place). **Zero restored files diverged** (§1, boundary C).
The publisher that once filled the layout, `Canvas::publish_reading`, is
present and unchanged. What it lacks is a build row and a caller: `canvas_1`
appears nowhere in `CMakeLists.txt`, and `the_board.cpp` includes `beat_clock.hpp`
in its place. This is a wiring question, not an archaeology one.

## How to read the section numbers

Sections follow the handoff's skeleton. §0 anchors every quotation. §1 is the
history. §2–§4 are the censuses. §5 and §6 are the two verbatim slices, and they
are the sections the plan is written from. §7 and §8 are the two side questions.
§9 lists everything that was skipped, could not be determined, or was recorded
against the handoff's own premise.

Each section was censused, then adversarially re-verified against the tree by a
second reader that re-ran every published recipe, and amended where the verifier
reproduced a discrepancy. Corrections the verifier could not reproduce were left
standing with the challenge recorded beside them.


## 0. Anchors

Every claim in this report was taken at one commit. If the tree has moved, check the
blob SHAs below before trusting a quotation: a changed blob means the quotation is
stale, and the row it supports must be re-taken.

### The commit

| what | value |
| --- | --- |
| repository | `7T-Music` (`jeanklein1/7T-Music`) |
| HEAD | `79adfa4d26c9e17e0074692928f1d2875d7edde1` |
| HEAD date | 2026-08-30 15:33:58 -0300 |
| HEAD subject | `Systems operational` |
| branch of record | `claude/ligature-0-recon-hcrix0` (see §9, FLAG-1) |
| working tree at recon time | clean (`git status --porcelain` empty) |

**Recipe.** `git rev-parse HEAD`; `git log -1 --format='%H%n%ad%n%s' --date=iso HEAD`;
`git status --porcelain`.

### Preflight

The clone arrived **shallow**. Per `CLAUDE.md` ("Boot preflight"), it was un-shallowed
before any claim about history was made:

```
git rev-parse --is-shallow-repository   # -> true
git fetch --unshallow origin            # -> also fetched tags: attic/full-board, web-sunset, native-sunset
git rev-parse --is-shallow-repository   # -> false
git log --oneline | wc -l               # -> 2228
```

Every history claim in §1 rests on the un-shallowed clone. Had the preflight been
skipped, the graft boundary would have reported false divergence and a false oldest
commit, and §1 would have been fiction.

### Blob anchors (R6)

Complete blob table for `src/`, `docs/`, `CMakeLists.txt` and `CMakePresets.json` at
HEAD. **Recipe:** `git ls-tree -r HEAD --format='%(objectname) %(path)' -- src docs CMakeLists.txt CMakePresets.json`

| blob | path |
| --- | --- |
| `2dddc9202f4d74650e28f95b3aa536ddb81cda9a` | `CMakeLists.txt` |
| `8f2298c00e9ac326af50cb621cd755947ebe2432` | `CMakePresets.json` |
| `bf36d995c9503fff4c3d42ae89874a60d23c8c9e` | `docs/7t_program_theory_v3.md` |
| `83b62a173e92280791693490bb81b7c100e0e107` | `docs/CHORD.md` |
| `2923b82cf04bbbe2827c865213141ec2805d3816` | `docs/FXC_LAWS_RECORD.md` |
| `595082461c203ea23124a2222694d0747e7c9946` | `docs/LAWS.md` |
| `008b477e61be15dc41f24042aef4d0b896600fc3` | `docs/OPEN.md` |
| `7639b022b788e1c694c95ad84ece494c2719af38` | `docs/ORGAN.md` |
| `dd1c241e0275035e935422da4a07c629126600f5` | `docs/PROCESS_LAWS.md` |
| `444b7c4a64019f3139f145e5543ba67f88ba12ef` | `docs/reference/ATTIC.md` |
| `e42a1935a53d411b464f39598245414e42057d56` | `docs/reference/DAWN_REFERENCE.md` |
| `b12e77adafdc97501106c4bec55d13aee9da11aa` | `docs/reference/RELEASE_CONSOLE.md` |
| `ad4307453c3ec8f8d7fed9fa38fa118fc0f21a3f` | `docs/reference/WEBGPU_SPEC.pdf` |
| `c41020be482a81065254341eab98fa48d5e118e4` | `docs/reference/WGSL_SPEC.pdf` |
| `f07e11a992d2381413b303f6741ddf32fe8205ab` | `src/analysis/analysis_cartridge.hpp` |
| `d088796d0ece785b9e34ee071273d6c5df7ce4a4` | `src/analysis/analysis_signal.hpp` |
| `b10038ff5069783c6be15e1e8d885d36238f7354` | `src/analysis/beat_clock.hpp` |
| `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5` | `src/analysis/canvas_1/canvas.hpp` |
| `071ba66d5d5ec87c9672a629842052ba6efc4909` | `src/analysis/canvas_1/check_canvas_compound.cpp` |
| `e8be1173416c8e56081ddabba5b8e4ed2cce288f` | `src/analysis/canvas_1/check_canvas_union.cpp` |
| `5cbe0a21bf8d94e9643ae280fc2e9c7c344706b3` | `src/analysis/canvas_1/check_field_union.cpp` |
| `f35a0927631c63df4685201f8cd9049315c5957d` | `src/analysis/canvas_1/check_pc_dft.cpp` |
| `7a341e9c00a0b8bd1965714df87c533d30a6cced` | `src/analysis/canvas_1/probe_canvas.cpp` |
| `2f024177980b55002008ae1f68b5ca045d698f3f` | `src/cartridges/the_board/bodies/agents.hpp` |
| `22647961b62e7446a91c802cd165ff079305b7ce` | `src/cartridges/the_board/bodies/cube_behaviors.hpp` |
| `0975ab7bfcb29129bd0ceae69474fbcdb73a9b22` | `src/cartridges/the_board/bodies/gol_zones.hpp` |
| `1467fab0aa48c00b31e7b8df58b223a29f4e537e` | `src/cartridges/the_board/bodies/grounded.hpp` |
| `1c695a09f1ceae4cd9bd773078dfead4d1f38c15` | `src/cartridges/the_board/bodies/orbs.hpp` |
| `f5952bfed05f02c18ccb3eda09e2e03dbe3902ae` | `src/cartridges/the_board/bodies/pawn.hpp` |
| `645dda2768c77f5acd2e63b22fcd0b801719a785` | `src/cartridges/the_board/bodies/pawn_figures.hpp` |
| `0c662c8e1454144d85f0074c70aaa97beee2087f` | `src/cartridges/the_board/bodies/ribbon.hpp` |
| `480082d864a0554e0abbaf21fedc2876d601e4b5` | `src/cartridges/the_board/bodies/spheres.hpp` |
| `3651bcabaa0b02a2925ad6868ce541ea9ab1b202` | `src/cartridges/the_board/cartridge.hpp` |
| `b54604d633b6662a640dd0c071dbcb255031e8ef` | `src/cartridges/the_board/contracts/agent_tiers.hpp` |
| `c40a926f5a6d2d23a58ac434e892aac2aa0ecea9` | `src/cartridges/the_board/contracts/control_panel.hpp` |
| `47f5526e229095bf8ed65727ae010c37ff5bdcbf` | `src/cartridges/the_board/contracts/demo_config.hpp` |
| `b0e478dba4d458b4d96c7fee0be04cd393bc58fb` | `src/cartridges/the_board/contracts/driver_surface.hpp` |
| `f1300cc6c3158c4fcdc01d65fb152c93b0bdfd20` | `src/cartridges/the_board/contracts/entity_types.hpp` |
| `794487bea3b27a29878eb3e83c9f51c5e657b4ec` | `src/cartridges/the_board/contracts/floaters.hpp` |
| `0488bc642c5993d6e32310d2fe1bd3fe6d41a3d2` | `src/cartridges/the_board/contracts/ground_architecture.hpp` |
| `e9d1c27a053ae716dffa67556dd11198fa7601ad` | `src/cartridges/the_board/contracts/indoor_module.hpp` |
| `93ea6a559dd6657a304ea47dd2a1ab6fd6676926` | `src/cartridges/the_board/contracts/mood_constants.hpp` |
| `db4962dd5efdcab2abed9ba8536c073772f7154b` | `src/cartridges/the_board/contracts/orb_surface.hpp` |
| `ea0518375a13ab1f966c8201ea6091215a413940` | `src/cartridges/the_board/contracts/pawn_surface.hpp` |
| `e42d7a6aa22b36b94402ea089e8a7188669f4d70` | `src/cartridges/the_board/contracts/point.hpp` |
| `9c27c0d48b4673b6573bdc4f8fa38fa0394978ba` | `src/cartridges/the_board/contracts/ribbon_surface.hpp` |
| `9a5345a95b5e69513ea8530d6b21a313bf3f87f9` | `src/cartridges/the_board/contracts/roster.hpp` |
| `b5494646c4e27f34521b40c9359a8203c4d0bcd8` | `src/cartridges/the_board/contracts/spawn_services.hpp` |
| `2bfd8c6ae424f3d051045fb72d3b95af231e9e12` | `src/cartridges/the_board/contracts/spine_state.hpp` |
| `2ea068e5442029125b5a6a83cea1ca66234e1568` | `src/cartridges/the_board/contracts/surface_services.hpp` |
| `32c3c5bd5df50c413aef4719b874edfa233eb44c` | `src/cartridges/the_board/contracts/wgpu_fwd.hpp` |
| `a339ba4b88a4f733621887f6eaf558ed38288516` | `src/cartridges/the_board/demos/demo.hpp` |
| `efb6f20f288f27e2e79155b70f427aaf3aff0d9a` | `src/cartridges/the_board/demos/matrix.hpp` |
| `718b7b6ef1b3ba98237262e7c383eead8c6d0f82` | `src/cartridges/the_board/direction/input.hpp` |
| `7b33643c0dc5d7d3467f3aa121b3e030dac5b7c8` | `src/cartridges/the_board/direction/mood.hpp` |
| `9d0fa2eabc3115fe654f5e512bcf69c113dec046` | `src/cartridges/the_board/machine/entity_pipeline.hpp` |
| `9824e094537c2138b272e08c027b8beb4e215f2e` | `src/cartridges/the_board/machine/spawn_engine.hpp` |
| `d747a67b5931dcbdbd713c00ba47d525e98d5b53` | `src/cartridges/the_board/organ_boundary.inc` |
| `1ce9b1a98de31721970065d5409536108e416b54` | `src/cartridges/the_board/primitives/seed_utils.hpp` |
| `7ec1457e886c5469724012ffb4c03ac7686a044c` | `src/cartridges/the_board/realization/binding_registry.hpp` |
| `c332a378a36f9372ebacd1c9852aa1e77d486bd7` | `src/cartridges/the_board/realization/binding_surface.gen.inc` |
| `c534d05260173fbbc9e15840bec1f49117a0773e` | `src/cartridges/the_board/realization/drawable_table.hpp` |
| `2c2508099eb04a6ac5dac5889448b2985ea6fc68` | `src/cartridges/the_board/realization/render_passes.hpp` |
| `fec8449cb0d5a94faaf1292c556cfed01731ca5e` | `src/cartridges/the_board/realization/renderer.hpp` |
| `fe4bce836b4665588ed43a0729d312e89cd05a20` | `src/cartridges/the_board/realization/state.hpp` |
| `5b36243dc6b45e27271d0d73eca8a01eb5dc2078` | `src/cartridges/the_board/realization/world.wgsl` |
| `ced6378eb92577ccb273a039d561f1b7cf221a94` | `src/cartridges/the_board/surface/patch_system.hpp` |
| `aea43238db01b9d3cced3b3609409de761d55b35` | `src/cartridges/the_board/surface/population_themes.hpp` |
| `0c7cc03b73dc4994bf21416817c8891849d7bdce` | `src/cartridges/the_board/surface/terrain_looks.hpp` |
| `08f82de05a02dda29072b5bcaf119aea9858a26a` | `src/cartridges/the_board/surface/tile_world.hpp` |
| `577c486049956d7c977580351b764967e3cb3d6c` | `src/console/console.hpp` |
| `6aff4fc75962679a06142b0834dc39a4365df1ab` | `src/console/features_wallet.gen.inc` |
| `d9c7d576a96c9f7ac03c2d0c03780cf061b60e6f` | `src/console/limits_floor.gen.inc` |
| `b426ac4f2b88f89b02f9a9d2236d14b992d93c7f` | `src/console/organ_params.inc` |
| `70d09e9602eb0f763a616da5303e14c34e7f44da` | `src/console/organ_registry.hpp` |
| `9b63e77bc7f32582e05b85a7bb1f00783ccdd1f5` | `src/core/boot_params.hpp` |
| `46bce8bba1ab1ffc6eeffe9837c75be648d372c1` | `src/core/cartridge_ids.hpp` |
| `1de6479a13627e4385741c3f16f4a0ce4aef5685` | `src/core/cartridge_manager.hpp` |
| `b3b4fc9e529a765de6dbd211ed052b6964fc63a4` | `src/core/input_event.hpp` |
| `3fd3b1285ab504d1bc3b9e264f440a77c26bc81d` | `src/core/instruments.hpp` |
| `6cd6c5a1d3bf3661fcbeb710e392ba0ec1c774f2` | `src/core/types.hpp` |
| `336d5d320f3a0337a78568f5762eae5885022109` | `src/coupling/canvas_surface.hpp` |
| `3047070e199df57c2a7cd6d8f75cf028ec48b817` | `src/coupling/organ_registry.hpp` |
| `a156425a5cc235d7a591fdd4c1150477406f86ce` | `src/coupling/trajectory.hpp` |
| `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35` | `src/coupling/visual_canvas.hpp` |
| `c196529d08b9b815a4b4282e7dc2695661febbc5` | `src/coupling/visual_params.hpp` |
| `2303bb1322738d84bac9f2919f62cb48477a63b4` | `src/external/RtMidi.cpp` |
| `2801037f628d3158c4660679e71ff833f1d26ead` | `src/external/RtMidi.h` |
| `a4c3977e08b4bf8638b6072077995a095667ad5f` | `src/musical/context.hpp` |
| `4285730324237ed01ed9fbd9e624d9bd4896417e` | `src/musical/context_realize.hpp` |
| `563962e7d0f05355d953d498ad6137606a627d7a` | `src/musical/context_spec.hpp` |
| `5e1f38d74dedcf8b32f5cd69a03b46808fd8115f` | `src/musical/field.hpp` |
| `f74d0b039cb6d630fe0f5e25b6c7067e2d9b18bf` | `src/musical/midi_stream.hpp` |
| `033b3a2f1a9f06f5c5dcf371408dea8fc68aa986` | `src/musical/musical_ops.hpp` |
| `acccba766e2a9b05b3d03f99ce3a238b6376dd9a` | `src/musical/pc_count.hpp` |
| `8429dd9ba8a679268d5e30abe1d7b25ba4a68765` | `src/musical/pc_dft.hpp` |
| `3172fe6cb587d3deed968db71611d64d39eb44af` | `src/musical/playhead.hpp` |
| `b97550ce0566bb351c36f2dfa8eeead803bde808` | `src/musical/previous_event.hpp` |
| `8e2e84312483e31e429276d91c23f7d63dc2643c` | `src/musical/signal_layout.hpp` |
| `8e9c144193e49019195d3526ed08f3c27486fb53` | `src/musical/spine.hpp` |
| `c33c885fb9fbd6b76d348f768a358d6066cc59b7` | `src/musical/spine_ops.hpp` |
| `d84086d83e06113e5284caa2aa6dc24b304cbd5f` | `src/musical/stream_data.hpp` |
| `b220ae3e019470beb3cfb3105c80492a1c37df1a` | `src/musical/vector_dressing.hpp` |
| `f7b091df7971971d53c2796ea61f4554e8952205` | `src/musical/wagon.hpp` |
| `ee8065299b2fb55fc0c97d6ec21fb465f36f5452` | `src/render/render_cartridge.hpp` |
| `3416fa83a9f300fd14375bf62977cebe01915f06` | `src/sources/keyboard_midi.hpp` |
| `b7c41853c298a1ff111445b508d58f9940615830` | `src/sources/midi_event.hpp` |
| `65c95697c244a0d4f4a9b6d0da2a11282ff46004` | `src/sources/midi_file.hpp` |
| `293ce7c46f669185235e2e6121d48f0a9863563e` | `src/sources/midi_port.hpp` |
| `d21388c5daf29c1dfd170d9c7a5c46870d0867af` | `src/sources/transport.hpp` |
| `588174ecddb0d68388e39a9025d6eda2f2afd000` | `src/the_board.cpp` |

Any file quoted at length anywhere in this report appears in that table; a section
that quotes a file also restates its blob inline, so a row can be checked without
scrolling back here.

## 1. Sunset boundary

Repo `/home/user/7T-Music`, branch `claude/ligature-0-recon-hcrix0`,
anchor commit `79adfa4d26c9e17e0074692928f1d2875d7edde1`. Working tree clean at the
time of every command below (`git status --porcelain` → empty, checked at the start
and again at the end of the amendment pass). All commands were run from the repo
root and are read-only.

**FLAG — the branch tip moved after this section was first written; every
HEAD-anchored recipe here is pinned to `79adfa4d`.** During verification the branch
`claude/ligature-0-recon-hcrix0` advanced from `79adfa4d` to
`6d53388e83f4a5cd7ad3b154484c885f567a02da` (*"LIGATURE_0 — the recon report: the
ligature is one hop, and the socket is empty"*, author `Claude`,
2026-08-30T19:50:05+00:00). That commit is one added file:
`git show --stat --format='' 6d53388e` → ` docs/LIGATURE_0_RECON.md | 8837 +`.
It changes nothing this section measures:
`git diff --name-only 79adfa4d 6d53388e -- src/` is **empty**, and
`git rev-parse '79adfa4d^{tree}:src'` = `git rev-parse '6d53388e^{tree}:src'` =
`c8b334db120a0c58d783e38e6052525d368689fa`. Every blob SHA, both file lists
(A = 42, D = 78) and every `--stat -- src/` total below were re-run pinned to
`79adfa4d` and are unchanged. Anyone re-running a recipe written here as `HEAD`
must substitute `79adfa4d` to reproduce these exact anchors.

**Two boundaries are recorded here, and they are not the same boundary.**

* **BOUNDARY A** is the handoff-literal one: `PRE..native-sunset`. The handoff named
  `native-sunset` as POST. The tag `native-sunset` marks the archival of the
  **native** twin (its own tag message says so), which is the opposite direction
  from the arm this campaign is about. Section (c) records it because the handoff
  asked for it; the evidence in (c) is that it is nearly empty over `src/coupling/`.
* **BOUNDARY B** is the severing the campaign is actually about: `1a52f2db^..1a52f2db`
  (CUT_1c, 2026-08-05). This is where the musical/MIDI arm was deleted.
* **BOUNDARY C** is the restoration: `web-sunset..HEAD`.

The orchestrator's established context is confirmed by my own reading on every
point I could test; two refinements and one new fact are flagged inline (§b note
on the PRUNE_1 range, §e note on `organ_registry.hpp`, §g).

---

### (a) TAG LEDGER

Recipes:

```
git tag -l
git cat-file -t <tag>                       # tag object type
git rev-parse <tag>                         # tag-object SHA
git rev-parse <tag>^{commit}                # commit SHA
git cat-file -p <tag>                       # tag message (annotated tags only)
git log -1 --format='%cI%n%s' <tag>^{commit}
git merge-base --is-ancestor <tag>^{commit} HEAD && echo YES || echo NO
```

| tag | object type | tag-object SHA | commit SHA | commit date (committer, ISO) | subject | tag message | ancestor of HEAD |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `attic/full-board` | commit (**lightweight**) | `de4b8b6fa0443d5ca53884bee539e7c6f4e3c3b9` | `de4b8b6fa0443d5ca53884bee539e7c6f4e3c3b9` | 2026-08-28T14:31:57+00:00 | `audit — ledger provenance converged (PURSE_0)` | *(none — lightweight tag carries no message)* | YES |
| `native-sunset` | tag (annotated) | `ed40fd943802cd5adb6705f83de64612a4437f99` | `29cec46b204f6b34845692f462f76c6df7051f3d` | 2026-08-16T05:07:41-03:00 | `7t/docs/HANDOFFS/CHORD` | `Native twin archived; the web twin is the program (SUNSET_0).` — tagger Jean Klein \<jeanfelipesouza@gmail.com\>, 1786874724 -0300 | YES |
| `web-sunset` | tag (annotated) | `34b971484718279deb26a1126455928806868708` | `60a3e935ce459326559a7a9ca377192944b36d8d` | 2026-08-29T04:22:47-03:00 | `SUNRISE_0 N10: retarget the glfw include row; install the include sentry (the vcpkg phantom named)` | `Web twin archived; the native twin is the program (WEB_SUNSET).]` *(trailing `]` is in the tag object)* — tagger jeanklein1 \<jeankleinmusic@gmail.com\>, 1788046310 -0300 | YES |

Three facts the ledger establishes that bear on the boundary choice:

1. `native-sunset` does **not** point at the SUNSET_0 deletion commit. It points at
   `29cec46b`, whose own diff is a single added file
   (`docs/HANDOFFS/CHORD/CHORD_HANDOFF.md`, +668). The SUNSET_0 deletion is its
   **child**, `e476addd` (`git log -1 --format='%H %P' e476addd` → parent
   `29cec46b`; `git log --format='%h %s' 29cec46b..e476addd` → exactly one commit).
   So "PRE..native-sunset" ends one commit *before* the sunset it names.
2. The SUNSET_0 "sequence" is one commit. `git log --all --grep='SUNSET_0'` returns
   four commits, only one of which has a `SUNSET_0`-prefixed subject: `e476addd`.
   The other three merely mention it in their bodies.
3. `attic/full-board` is a lightweight tag, so it has no tag message and no tagger —
   its "message" is the commit's own.
4. The two annotated tags were made under **different tagger identities**. Full tag
   objects, verbatim:

   ```
   $ git cat-file -p native-sunset
   object 29cec46b204f6b34845692f462f76c6df7051f3d
   type commit
   tag native-sunset
   tagger Jean Klein <jeanfelipesouza@gmail.com> 1786874724 -0300

   Native twin archived; the web twin is the program (SUNSET_0).
   ```

   ```
   $ git cat-file -p web-sunset
   object 60a3e935ce459326559a7a9ca377192944b36d8d
   type commit
   tag web-sunset
   tagger jeanklein1 <jeankleinmusic@gmail.com> 1788046310 -0300

   Web twin archived; the native twin is the program (WEB_SUNSET).]
   ```

   Recorded as a fact about the tag objects only; the unit asked for the tag
   message, and the tagger row is supplied here for completeness.

---

### (b) TIMELINE (oldest first)

Recipes used to build this table:

```
git log -1 --format='%H %P %cI %s' <sha>
git show --stat --format='' <sha>            # per-commit stat
git log --format='%h %p %cI %s' de4b8b6f..HEAD
git log --format='%h %cI %s' --grep='SUNRISE_0'
git log --format='%h %cI %s' --grep='WEB_SUNSET'
git log --format='%h %cI %s' --grep='PRUNE_1'
git log --format='%h %cI %s' --grep='music' -i
git ls-tree -r --name-only <sha> src/musical/ src/sources/
```

| sha | date (committer ISO) | subject | what it did to the musical / coupling arm |
| --- | --- | --- | --- |
| `1a52f2db` | 2026-08-05T07:23:11+00:00 | `CUT_1c: MIDI intake retired; BeatClock (variable BPM, default 100) feeds the signal spine` | **THE SEVERING.** 49 files deleted, 1 added, 3 modified; 53 files changed, +90 / −102426. Deleted 15 of 16 `src/musical/` headers, all 5 `src/sources/` headers, `src/analysis/analysis_cartridge.hpp` and all of `src/analysis/canvas_1/`, `src/external/RtMidi.{cpp,h}`, all of `src/external/imgui/` + `implot/`, and `src/the_lab.cpp`. Added `src/analysis/beat_clock.hpp`. Left `src/musical/signal_layout.hpp` alone. Touched **no** file under `src/coupling/`. Full detail in (d). |
| `2bedb4e2` | 2026-08-09T01:48:36+00:00 | `oil: U5 — the hue unit vectors seat once (ledger: U4 hue loop, C4)` | Last **behaviour-bearing** change to `src/coupling/` before the SUNSET_0 sequence (`visual_canvas.hpp`, +21/−6). Chosen as BOUNDARY A's PRE; see (c). |
| `939877c2` | 2026-08-09T03:38:00+00:00 | `oil: V2 — the invariants the batch created, stated where they live` | `src/coupling/visual_canvas.hpp` +1: one `#include <cstddef>` line, self-declared "Comments and one include; no behavior". Rejected as PRE (see (c)). |
| `7e76bec5` | 2026-08-12T04:36:39+00:00 | `TIDY_0b: EOL_1 — trailing newline on the unterminated text files` | Last commit of any kind touching `src/coupling/` before `native-sunset`. Appends exactly one `\n` to `trajectory.hpp`, `visual_canvas.hpp`, `visual_params.hpp`. Rejected as PRE — pure trailing-newline. |
| `29cec46b` | 2026-08-16T05:07:41-03:00 | `7t/docs/HANDOFFS/CHORD` | **TAG `native-sunset`.** Diff is one added docs file (+668). Zero effect on the arm. |
| `e476addd` | 2026-08-16T08:25:36+00:00 | `SUNSET_0: the web twin is the program; native archived at tag native-sunset` | The actual SUNSET_0 deletion, and it is a **build/console** deletion: `CMakeLists.txt` −558-ish, `docs/LAWS.md`, `world.wgsl`, `console.hpp`, `third_party/emdawnwebgpu/PINNED.md`. 5 files, +102 / −671. Touches no file under `src/musical/`, `src/sources/`, `src/analysis/` or `src/coupling/`. |
| `b33b40d3` | 2026-08-16T14:22:34+00:00 | `ORGAN_0b: the compiled registry — enrollment is one line, the compiler swears to the offsets, the manifest is the whitelist` | Birth of `src/console/organ_registry.hpp` (`git log --diff-filter=A -- src/console/organ_registry.hpp`). Relevant to §e's `organ_registry` finding. |
| `da0ae12d` | 2026-08-19T09:12:12+00:00 | `ORGAN_3b P2 — the canvas tier: a namespace parameter nobody has to see` | Birth of `src/coupling/canvas_surface.hpp` (`git log --diff-filter=A -- src/coupling/canvas_surface.hpp`). Post-dates CUT_1c, so it is "was-absent" in §e. |
| `de4b8b6f` | 2026-08-28T14:31:57+00:00 | `audit — ledger provenance converged (PURSE_0)` | **TAG `attic/full-board` — the fork point.** Diff is `audit/MIRROR_LEDGER.md` alone, +2/−2. Zero effect on the arm. |
| `b94da37c` .. `60a3e935` | 2026-08-28T22:26:28-03:00 .. 2026-08-29T04:22:47-03:00 | **SUNRISE_0 restoration sequence** — N0 `fork identity + deploy interdiction`, N1 `restore native driver arm — the_board.cpp + boot_params.hpp (source 315d4bc1^)`, N2 `restore native console arm — console.hpp`, N3 `restore native gallery arm — gallery.hpp`, N4 `restore native build arm — CMakeLists + presets (Dawn pin 56f332d7)`, N5 mirror-ledger regen, N6 dawn_lib manifest, N7 `/Zc:preprocessor`, N8 recorded finding, N9 (+3 ledger regen rounds), N10 glfw include sentry | 15 commits (`git log --format='%h' de4b8b6f..60a3e935 \| wc -l` → 15). Restores the **native** arm (driver, console, gallery, build). Restores **nothing** under `src/musical/`, `src/sources/`, `src/analysis/canvas_1/`. |
| `60a3e935` | 2026-08-29T04:22:47-03:00 | `SUNRISE_0 N10: retarget the glfw include row; install the include sentry (the vcpkg phantom named)` | **TAG `web-sunset`.** Diff is `CMakeLists.txt` alone, +46/−1. It is the last commit *before* the W-sequence, not part of it. |
| `1bc4c33d` .. `cd36a385` | 2026-08-29T18:29:44+00:00 .. 2026-08-29T20:17:27+00:00 | **WEB_SUNSET W-sequence** — W0 four zero-byte root files (`0`, `0\``, `16)`, `17)`); W1a/W1b gate flips native + vendored `third_party/dawn_native_headers` @56f332d7; W2 one build language; W3a·1/·2 `boot_params.hpp` + `the_board.cpp`; W3b·1/·2 `console.hpp`; W3c·1/·2 `gallery.hpp`; W3d·1/·2 `renderer.hpp` + serve witness; W3e organ prose; W4a presets move; W4b `web/`, `web_dist`, shell gate burn; W5a sha256 cascade; W5b emscripten stubs; W5c `assets/music`; W5d `assets/entrance`; W6a/W6b constitution, README, LAWS, OPEN, banners; W7a/W7 ledgers; + one ledger-regen round | 24 commits (`git log --format='%h' --first-parent 60a3e935..cd36a385 \| wc -l` → 24). Touches **no** file under `src/musical/`, `src/sources/`, `src/analysis/`. W3e is the one commit that moves `src/console/organ_registry.hpp` (blob `70d09e96` → `3047070e`) — see §e note. |
| `03005fcd` .. `a30a240e` | 2026-08-29T21:23:27+00:00 .. 2026-08-29T21:26:19+00:00 | KEEL_0 K1..K5 (build constitution, config axis, preset surface) | Build/prose only. No effect on the arm. |
| `c3719f0d` .. `bc91cf3a` | 2026-08-29T23:21:08+00:00 .. 2026-08-29T23:23:38+00:00 | HELM_0 H1..H3 (preset surface prunes to five) | Build/prose only. No effect on the arm. |
| `f008579b` .. `743dc9d0` | 2026-08-30T01:38:11+00:00 .. 2026-08-30T02:12:29+00:00 | **PRUNE_1** — U1 `the spine + the ready offer`, U2 `the pipelines + the draw verbs`, U5+U6 `the module, the assets, the family, the DTOs, the prose`, U3+U4 `the resources, the binding surface, the shader`, U7 `the bit, the gates, the ledgers, the record`, U7a `the mirror ledger's provenance stamp` | 6 commits. **Gallery** prune: `gallery.hpp` −327 then deleted entirely in the range, `stb_image.{h,cpp}` deleted, `state.hpp` −499-ish, `world.wgsl` −705, `binding_surface.gen.inc` −325. Whole range over `src/`: 35 files, +338 / −13140 (`git diff --stat f008579b^ 743dc9d0 -- src/`). Touches **no** file under `src/musical/`, `src/sources/`, `src/analysis/`, `src/coupling/`. |
| `0c951b11` | 2026-08-30T13:34:46-03:00 | `bringing back the music` | **THE RESTORATION, source half.** 29 files, **+11037, −0**. Re-adds all 15 `src/musical/` headers, all 5 `src/sources/` headers, `src/analysis/analysis_cartridge.hpp`, all 5 files of `src/analysis/canvas_1/`, and `src/external/RtMidi.{cpp,h}`. Parent is `743dc9d0`. |
| `e0e22e46` | 2026-08-30T13:41:16-03:00 | `bring back the music` | **THE RESTORATION, build half.** `CMakeLists.txt` +20/−2 (re-adds `__WINDOWS_MM__` to `MSVC_COMPILE_DEFS`, adds `src/external/RtMidi.cpp` to the `the_board` target, pins `VS_DEBUGGER_WORKING_DIRECTORY`), plus a stray zero-byte file named `git`. |
| `e9c2ace3` | 2026-08-30T16:36:35+00:00 | `Merge claude/prune-gallery-organ-exmmzu — the gallery organ leaves the instrument, vocabulary and all` | Merge of `bc91cf3a` (p1) and `743dc9d0` (p2). `git diff --stat 743dc9d0 e9c2ace3` is **empty** — the merge result is the tree of its second parent. Against p1: 106 files, +1187 / −14163. |
| `3ac091f8` | 2026-08-30T16:43:28+00:00 | `Merge claude/prune-gallery-organ-exmmzu — the musical half comes back` | Merge of `e9c2ace3` (p1) and `e0e22e46` (p2). `git diff --stat e0e22e46 3ac091f8` is **empty** — again the tree of its second parent. Against p1: 31 files, +11057 / −2. |
| `e9e1a442` | 2026-08-30T14:03:28-03:00 | `tidy: drop the stray git file` | Removes the zero-byte `git` file added by `e0e22e46`. |
| `ebfac622` | 2026-08-30T14:04:25-03:00 | `tidy: restore CMakeLists' trailing newline` | `CMakeLists.txt` +1/−1. |
| `72df32df` | 2026-08-30T14:29:22-03:00 | `Systems operating` | Adds `src/coupling/organ_registry.hpp` (+999). Its blob is `3047070e199d`, which is **byte-identical to the then-current `src/console/organ_registry.hpp`**. See §e note. |
| `79adfa4d` | 2026-08-30T15:33:58-03:00 | `Systems operational` | **HEAD.** `CMakeLists.txt` +1/−1 (re-removes the trailing newline `ebfac622` restored — `HEAD:CMakeLists.txt` = `2dddc920…` = `e0e22e46:CMakeLists.txt`), `src/console/console.hpp` **+3/−1** (`git show --numstat --format='' 79adfa4d` → `3\t1\tsrc/console/console.hpp`; the hunk deletes the one line `inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan;` and adds three — the declaration without its semicolon, a whitespace-only line, and a line holding `;` — see the verbatim hunk below the table), `src/console/organ_registry.hpp` reverted to blob `70d09e9602eb` — the pre-W3e content. |

**The `79adfa4d` numstat and its `console.hpp` hunk, verbatim.**

```
$ git show --numstat --format='' 79adfa4d
1	1	CMakeLists.txt
3	1	src/console/console.hpp
68	93	src/console/organ_registry.hpp
```

```diff
$ git show 79adfa4d -- src/console/console.hpp
diff --git a/src/console/console.hpp b/src/console/console.hpp
index a7106e3c..577c4860 100644
--- a/src/console/console.hpp
+++ b/src/console/console.hpp
@@ -104,7 +104,9 @@ namespace t7 {
     // Plan B is one line: if DXC fails on a given driver, set this to
     // Vulkan, rebuild, boot. That IS the fallback, not a failure.
     enum class CompilerPlan { D3D12_Dxc, Vulkan, D3D12_Fxc };
-    inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan;
+    inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan
+        
+        ;
 
     inline constexpr const char* compiler_plan_name(CompilerPlan p) {
         switch (p) {
```

The declaration's value is unchanged (`CompilerPlan::Vulkan` both sides); the three
added lines split the initialiser across a blank line and a bare `;`. The second
added line carries eight trailing spaces and is otherwise empty.

**FLAG — PRUNE_1 range notation.** The handoff writes the PRUNE_1 range as
`f008579b..743dc9d0`. In git's two-dot notation that range *excludes* `f008579b`,
i.e. it names 5 of the 6 PRUNE_1 commits. The six-commit sequence is
`bc91cf3a..743dc9d0` (equivalently `f008579b^..743dc9d0`), verified with
`git log --format='%h %s' bc91cf3a..743dc9d0 | wc -l` → 6 and
`git log --format='%h %cI %s' --grep='PRUNE_1'` → the same six subjects plus the two
later merge subjects that mention the branch name. I treat the handoff's pair as an
inclusive landmark pair, not a git range.

---

### (c) BOUNDARY A — the handoff-literal boundary

**PRE = `2bedb4e2` · POST = `29cec46b` (`native-sunset`).**

#### Choice of PRE, and the two commits rejected

`git log --format='%h %cI %s' -20 29cec46b -- src/coupling/` returns, most recent
first: `7e76bec5`, `939877c2`, `2bedb4e2`, `13669042`, `bfc96772`, `f4d656e5`, …

* **`7e76bec5` REJECTED — pure trailing-newline commit.** `git show 7e76bec5 -- src/coupling/`
  is three hunks, each `-} // namespace t7 \ No newline at end of file` /
  `+} // namespace t7`. The commit message states it and proves it: *"WHITESPACE
  ONLY, and proven so: every one of the 18 files differs from its parent by EXACTLY
  one appended \n, verified byte-for-byte (new == old + b"\n")"*.
* **`939877c2` REJECTED — comment + one include, self-declared no-behaviour.**
  `git show 939877c2 -- src/coupling/` is a single added line in `visual_canvas.hpp`:
  `+#include <cstddef>   // size_t — the table's index casts (OIL_1 U5)`. The commit
  message opens *"Comments and one include; no behavior."* It does change compiled
  content, so I record it as a judgement call rather than a certainty; §c's diff below
  contains its one line so nothing is hidden by the choice.
* **`2bedb4e2` CHOSEN.** `visual_canvas.hpp` +21/−6, seating the hue unit-vector
  table once — the last change under `src/coupling/` that alters what the code does.

Independent check that the choice barely matters here:
`git diff --stat 7e76bec5..29cec46b -- src/coupling/` is **empty**. Nothing touched
`src/coupling/` between the newline commit and the tag.

#### `git diff --stat 2bedb4e2..29cec46b -- src/`

```
 src/New chat first handoff.txt                     |    2 +-
 src/analysis/beat_clock.hpp                        |    9 +
 src/cartridges/the_board/bodies/agents.hpp         |   34 +-
 src/cartridges/the_board/bodies/gallery.hpp        |   55 +-
 src/cartridges/the_board/bodies/gol_zones.hpp      |   23 +-
 src/cartridges/the_board/bodies/orbs.hpp           |   23 +-
 src/cartridges/the_board/bodies/pawn.hpp           |    5 +-
 src/cartridges/the_board/bodies/ribbon.hpp         |   29 +-
 src/cartridges/the_board/bodies/spheres.hpp        |    2 +-
 src/cartridges/the_board/cartridge.hpp             |  373 ++-
 .../the_board/contracts/surface_services.hpp       |   24 +
 src/cartridges/the_board/direction/input.hpp       |    4 +-
 src/cartridges/the_board/direction/mood.hpp        |   21 +-
 src/cartridges/the_board/primitives/seed_utils.hpp |    2 +-
 .../the_board/realization/binding_registry.hpp     |  328 +--
 .../the_board/realization/binding_surface.gen.inc  | 1956 +++++++++++++++
 .../the_board/realization/drawable_table.hpp       |   53 +-
 .../the_board/realization/render_passes.hpp        |  289 ++-
 src/cartridges/the_board/realization/renderer.hpp  | 1006 ++++----
 src/cartridges/the_board/realization/state.hpp     | 2501 +++-----------------
 src/cartridges/the_board/realization/world.wgsl    |  645 +++--
 src/cartridges/the_board/surface/patch_system.hpp  |   78 +-
 src/cartridges/the_board/surface/tile_world.hpp    |    2 +-
 src/console/console.hpp                            |  822 ++++++-
 src/console/limits_floor.gen.inc                   |   14 +
 src/core/boot_params.hpp                           |  155 ++
 src/core/cartridge_manager.hpp                     |    2 +-
 src/core/instruments.hpp                           |   14 +-
 src/coupling/trajectory.hpp                        |    2 +-
 src/coupling/visual_canvas.hpp                     |    3 +-
 src/coupling/visual_params.hpp                     |    2 +-
 src/external/stb_image.cpp                         |    2 +-
 src/incubator_dual.cpp                             |   12 +-
 src/render/render_cartridge.hpp                    |    6 +-
 34 files changed, 5234 insertions(+), 3264 deletions(-)
```

#### `git diff --diff-filter=D --name-only 2bedb4e2..29cec46b`

```
(empty — zero files deleted, whole tree)
```

`git diff --diff-filter=D --name-only 2bedb4e2..29cec46b -- src/` is likewise empty.
**BOUNDARY A deletes nothing at all.**

#### `git diff --diff-filter=M --stat 2bedb4e2..29cec46b -- src/coupling/ src/analysis/ src/musical/`

```
 src/analysis/beat_clock.hpp    | 9 +++++++++
 src/coupling/trajectory.hpp    | 2 +-
 src/coupling/visual_canvas.hpp | 3 ++-
 src/coupling/visual_params.hpp | 2 +-
 4 files changed, 13 insertions(+), 3 deletions(-)
```

`src/sources/` does not exist at either endpoint. Both commands return the same
eight path-prefixed names, one per line — verbatim:

```
$ git ls-tree -d --name-only 2bedb4e2 src/
src/analysis
src/cartridges
src/console
src/core
src/coupling
src/external
src/musical
src/render
```

```
$ git ls-tree -d --name-only 29cec46b src/
src/analysis
src/cartridges
src/console
src/core
src/coupling
src/external
src/musical
src/render
```

No `src/sources` row appears at either endpoint.
`src/musical/` exists at both ends but holds only `signal_layout.hpp` — CUT_1c had already
emptied it three months of commits earlier.

#### FULL diff of `src/coupling/` across BOUNDARY A

`git diff 2bedb4e2..29cec46b -- src/coupling/`

Anchors: `2bedb4e2:src/coupling/trajectory.hpp` = `6d65eabe4f54…`,
`2bedb4e2:src/coupling/visual_canvas.hpp` = `e41ca84a2a4f…`,
`2bedb4e2:src/coupling/visual_params.hpp` = `084585af49c4…`;
`29cec46b:` → `a156425a5cc2…`, `97aa3dfc61a7…`, `c196529d08b9…`.

```diff
diff --git a/src/coupling/trajectory.hpp b/src/coupling/trajectory.hpp
index 6d65eabe..a156425a 100644
--- a/src/coupling/trajectory.hpp
+++ b/src/coupling/trajectory.hpp
@@ -77,4 +77,4 @@ namespace t7 {
         return sample_segment(seg, beat);
     }
 
-} // namespace t7
\ No newline at end of file
+} // namespace t7
diff --git a/src/coupling/visual_canvas.hpp b/src/coupling/visual_canvas.hpp
index e41ca84a..97aa3dfc 100644
--- a/src/coupling/visual_canvas.hpp
+++ b/src/coupling/visual_canvas.hpp
@@ -67,6 +67,7 @@
 #include "analysis/analysis_signal.hpp"
 #include <string>    // casting-sheet name composition ("<voice>.present_count")
 #include <array>     // the hue unit-vector table (OIL_1 U5)
+#include <cstddef>   // size_t — the table's index casts (OIL_1 U5)
 #include <cmath>     // std::floor / cos / sin / sqrt / atan2 — decode math
 #include <algorithm> // std::min/std::max — decode clamps
 #include <cstdio>    // std::fprintf — the [CHECKER] witness line
@@ -629,4 +630,4 @@ namespace t7 {
         float         zoetrope_rows_[7] = {}; // row impulses, overwritten each tick
     };
 
-} // namespace t7
\ No newline at end of file
+} // namespace t7
diff --git a/src/coupling/visual_params.hpp b/src/coupling/visual_params.hpp
index 084585af..c196529d 100644
--- a/src/coupling/visual_params.hpp
+++ b/src/coupling/visual_params.hpp
@@ -141,4 +141,4 @@ namespace t7 {
         ParamLayoutView view_{ nullptr, 0 };
     };
 
-} // namespace t7
\ No newline at end of file
+} // namespace t7
```

That is the entirety of BOUNDARY A over `src/coupling/`: one `#include` line and
three trailing newlines.

---

### (d) BOUNDARY B — CUT_1c, the severing the campaign is actually about

`git log -1 --format='%H %P %cI' 1a52f2db` →
`1a52f2db0991d2612e29588298215281272a70ed 7bb32d23a90621ed50633be34eb9fb64c628483b 2026-08-05T07:23:11+00:00`
Subject: `CUT_1c: MIDI intake retired; BeatClock (variable BPM, default 100) feeds the signal spine`

#### `git diff --stat 1a52f2db^..1a52f2db`

```
 CMakeLists.txt                                  |   300 +-
 src/analysis/analysis_cartridge.hpp             |   103 -
 src/analysis/beat_clock.hpp                     |    53 +
 src/analysis/canvas_1/canvas.hpp                |   686 -
 src/analysis/canvas_1/check_canvas_compound.cpp |    96 -
 src/analysis/canvas_1/check_canvas_union.cpp    |   138 -
 src/analysis/canvas_1/check_field_union.cpp     |   108 -
 src/analysis/canvas_1/check_pc_dft.cpp          |    91 -
 src/analysis/canvas_1/probe_canvas.cpp          |   196 -
 src/cartridges/the_board/realization/state.hpp  |     1 -
 src/external/RtMidi.cpp                         |  5275 -----
 src/external/RtMidi.h                           |   686 -
 src/external/imgui/backends/imgui_impl_glfw.cpp |  1727 --
 src/external/imgui/backends/imgui_impl_glfw.h   |    73 -
 src/external/imgui/backends/imgui_impl_wgpu.cpp |  1162 --
 src/external/imgui/backends/imgui_impl_wgpu.h   |   121 -
 src/external/imgui/imconfig.h                   |   147 -
 src/external/imgui/imgui.cpp                    | 24360 ----------------------
 src/external/imgui/imgui.h                      |  4523 ----
 src/external/imgui/imgui_demo.cpp               | 11540 ----------
 src/external/imgui/imgui_draw.cpp               |  6791 ------
 src/external/imgui/imgui_internal.h             |  4321 ----
 src/external/imgui/imgui_tables.cpp             |  4665 -----
 src/external/imgui/imgui_widgets.cpp            | 11119 ----------
 src/external/imgui/imstb_rectpack.h             |   627 -
 src/external/imgui/imstb_textedit.h             |  1527 --
 src/external/imgui/imstb_truetype.h             |  5085 -----
 src/external/implot/implot.cpp                  |  5935 ------
 src/external/implot/implot.h                    |  1406 --
 src/external/implot/implot_internal.h           |  1713 --
 src/external/implot/implot_items.cpp            |  3515 ----
 src/incubator_dual.cpp                          |   100 +-
 src/musical/context.hpp                         |   187 -
 src/musical/context_realize.hpp                 |    75 -
 src/musical/context_spec.hpp                    |   191 -
 src/musical/field.hpp                           |   127 -
 src/musical/midi_stream.hpp                     |   179 -
 src/musical/musical_ops.hpp                     |   192 -
 src/musical/pc_count.hpp                        |   132 -
 src/musical/pc_dft.hpp                          |    64 -
 src/musical/playhead.hpp                        |   179 -
 src/musical/previous_event.hpp                  |   194 -
 src/musical/spine.hpp                           |   271 -
 src/musical/spine_ops.hpp                       |    52 -
 src/musical/stream_data.hpp                     |   489 -
 src/musical/vector_dressing.hpp                 |    54 -
 src/musical/wagon.hpp                           |   164 -
 src/sources/keyboard_midi.hpp                   |   257 -
 src/sources/midi_event.hpp                      |    59 -
 src/sources/midi_file.hpp                       |   469 -
 src/sources/midi_port.hpp                       |   213 -
 src/sources/transport.hpp                       |   110 -
 src/the_lab.cpp                                 |   668 -
 53 files changed, 90 insertions(+), 102426 deletions(-)
```

#### Deleted-file list — `git diff --diff-filter=D --name-only 1a52f2db^..1a52f2db` (49 files)

```
src/analysis/analysis_cartridge.hpp
src/analysis/canvas_1/canvas.hpp
src/analysis/canvas_1/check_canvas_compound.cpp
src/analysis/canvas_1/check_canvas_union.cpp
src/analysis/canvas_1/check_field_union.cpp
src/analysis/canvas_1/check_pc_dft.cpp
src/analysis/canvas_1/probe_canvas.cpp
src/external/RtMidi.cpp
src/external/RtMidi.h
src/external/imgui/backends/imgui_impl_glfw.cpp
src/external/imgui/backends/imgui_impl_glfw.h
src/external/imgui/backends/imgui_impl_wgpu.cpp
src/external/imgui/backends/imgui_impl_wgpu.h
src/external/imgui/imconfig.h
src/external/imgui/imgui.cpp
src/external/imgui/imgui.h
src/external/imgui/imgui_demo.cpp
src/external/imgui/imgui_draw.cpp
src/external/imgui/imgui_internal.h
src/external/imgui/imgui_tables.cpp
src/external/imgui/imgui_widgets.cpp
src/external/imgui/imstb_rectpack.h
src/external/imgui/imstb_textedit.h
src/external/imgui/imstb_truetype.h
src/external/implot/implot.cpp
src/external/implot/implot.h
src/external/implot/implot_internal.h
src/external/implot/implot_items.cpp
src/musical/context.hpp
src/musical/context_realize.hpp
src/musical/context_spec.hpp
src/musical/field.hpp
src/musical/midi_stream.hpp
src/musical/musical_ops.hpp
src/musical/pc_count.hpp
src/musical/pc_dft.hpp
src/musical/playhead.hpp
src/musical/previous_event.hpp
src/musical/spine.hpp
src/musical/spine_ops.hpp
src/musical/stream_data.hpp
src/musical/vector_dressing.hpp
src/musical/wagon.hpp
src/sources/keyboard_midi.hpp
src/sources/midi_event.hpp
src/sources/midi_file.hpp
src/sources/midi_port.hpp
src/sources/transport.hpp
src/the_lab.cpp
```

`git diff --diff-filter=A --name-only 1a52f2db^..1a52f2db` → `src/analysis/beat_clock.hpp` (one file).
`git diff --diff-filter=M --name-only 1a52f2db^..1a52f2db` → `CMakeLists.txt`,
`src/cartridges/the_board/realization/state.hpp`, `src/incubator_dual.cpp`.

#### What CUT_1c deleted, and what it left behind — by directory

`git ls-tree -r --name-only 1a52f2db^ src/musical/` vs `git ls-tree -r --name-only 1a52f2db src/musical/`:

| directory | at `1a52f2db^` | at `1a52f2db` | verdict |
| --- | --- | --- | --- |
| `src/musical/` | 16 headers: `context`, `context_realize`, `context_spec`, `field`, `midi_stream`, `musical_ops`, `pc_count`, `pc_dft`, `playhead`, `previous_event`, **`signal_layout`**, `spine`, `spine_ops`, `stream_data`, `vector_dressing`, `wagon` | **`signal_layout.hpp` only** | 15 of 16 deleted; `signal_layout.hpp` **left behind** |
| `src/sources/` | `keyboard_midi.hpp`, `midi_event.hpp`, `midi_file.hpp`, `midi_port.hpp`, `transport.hpp` | *(directory gone — `git ls-tree -r --name-only 1a52f2db src/sources/` is empty)* | 5 of 5 deleted |
| `src/analysis/` | `analysis_cartridge.hpp`, `analysis_signal.hpp`, `canvas_1/{canvas.hpp, probe_canvas.cpp, check_canvas_compound.cpp, check_canvas_union.cpp, check_field_union.cpp, check_pc_dft.cpp}` | `analysis_signal.hpp`, **`beat_clock.hpp` (new)** | `analysis_cartridge.hpp` + all of `canvas_1/` deleted; `analysis_signal.hpp` **left behind**; `beat_clock.hpp` added |
| `src/coupling/` | `canvas_surface.hpp` did not yet exist; `trajectory.hpp`, `visual_canvas.hpp`, `visual_params.hpp` present | unchanged | **CUT_1c touched no file under `src/coupling/` at all** |
| `src/external/` | `RtMidi.{cpp,h}`, `imgui/` (**15** files), `implot/` (4 files), `stb_image.{h,cpp}` | `stb_image.{h,cpp}` only | **21** files deleted (RtMidi 2 + ImGui 15 + ImPlot 4 — the lab's GUI stack); `stb_image.{h,cpp}` left behind |
| repo root | `src/the_lab.cpp` | *(gone)* | deleted |

Its own commit message names the two lists (`git log -1 --format='%b' 1a52f2db`):

```
$ git log -1 --format='%b' 1a52f2db
DEATH LIST (handoff): external/RtMidi.{cpp,h}; sources/{midi_event,
midi_port,transport}.hpp; analysis/analysis_cartridge.hpp;
analysis/canvas_1/ whole (canvas.hpp, probe_canvas.cpp, 4 check_*.cpp);
15 of 16 musical headers: context, context_realize, context_spec, field,
midi_stream, musical_ops, pc_count, pc_dft, playhead, previous_event,
spine, spine_ops, stream_data, vector_dressing, wagon.
signal_layout.hpp survives (sole musical include: visual_canvas.hpp:66).

STAMPED EXTENSION (Jean, in chat — death-list isolation failed outside
the closure; gate 2): src/the_lab.cpp; sources/{midi_file,
keyboard_midi}.hpp (dangling includers of midi_event, zero consumers);
src/external/{imgui,implot}/ (lab-only, verified zero consumers outside
the_lab.cpp + its CMake block); CMake targets the_lab, probe_canvas,
check_canvas_union, check_field_union, check_pc_dft,
check_canvas_compound (all compiled dying sources unconditionally —
configure would fail tree-wide); the INCUBATOR_ONLY-gated main-target
block (referenced dead headers AND nonexistent src/main.cpp);
INCUBATOR_ANALYSIS_CARTRIDGE / INCUBATOR_DUAL_ANALYSIS_CARTRIDGE cache
vars + canvas_1 self-heal block + INCUBATOR_ONLY option.

CMAKE (handoff items): RtMidi.cpp out of the incubator_dual TU list;
__WINDOWS_MM__ out of MSVC_COMPILE_DEFS; INCUBATE_ANALYSIS define
retired (gate 3: reached only harness + CMakeLists:562).

BEATCLOCK BINDING DECISIONS (gate 4): true AnalysisSignal fields are
t_seconds / t_beats / dt / _pad0 / stats[1024] / _pad1[4]. No bpm and
no transport/'playing' flag exist in the struct — nothing to default
always-on; bpm lives only in BeatClock (one home, panel-eligible).
Mapping: t_seconds<-seconds accumulator; t_beats<-beats accumulator
(beats += dt * bpm/60); dt<-last update dt; stats and pads
value-initialized zero. stat_layout() = StatLayoutView{nullptr,0}:
all render-side resolves (12 sources named in beat_clock.hpp + 8 param
targets in bind_signal_layout) miss once, warn on stderr, and disable
via signal_layout.hpp's graceful path.

HARNESS REWIRE: INCUBATE_ANALYSIS fallback + ANALYSIS_HEADER macro +
analysis_ns/AnalysisCartridge/ANALYSIS_NAME + is_music_key + the
analysis input route deleted (route was ledger-verified dead: always
false). analysis.initialize("assets") DELETED, not replaced —
BeatClock needs no init (the old arg was discarded by the receiver
anyway). The misstating 'STAT_LAYOUT is constexpr' comment replaced
with present behavior (empty layout, graceful misses). Banner now
prints the clock + bpm.

ALSO: state.hpp:30 vestigial analysis_signal.hpp include deleted
(ledger-verified zero tokens used; cartridge gets the type via
render_cartridge.hpp:29).

RESIDUE (reported, left): prose mentions of the_lab in
signal_layout.hpp:12 and of canvas_1 in visual_canvas.hpp:126 — comments
in surviving files, outside this unit's edit list.

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_012VTvA9PPHhzh6EbEuHtjf8
```

**R5 note.** The section previously rendered the first two paragraphs of this body
as a markdown blockquote, which reflowed the commit message's hard line breaks and
introduced a bold span (`**signal_layout.hpp survives …**`) that is not in the
commit object. The fenced block above is the complete, unreflowed output of
`git log -1 --format='%b' 1a52f2db`, trailers included; no bold, no elision.

Two rows of that body bear directly on later sections and are quoted again here as
they appear in it, unmodified: `signal_layout.hpp survives (sole musical include:
visual_canvas.hpp:66).` and `stat_layout() = StatLayoutView{nullptr,0}: all
render-side resolves (12 sources named in beat_clock.hpp + 8 param targets in
bind_signal_layout) miss once, warn on stderr, and disable via signal_layout.hpp's
graceful path.` (The `visual_canvas.hpp:66` line reference is the commit author's
own text inside a verbatim quotation, not an anchor authored by this report; R3
applies to this report's prose, and every claim it makes is anchored to an
enclosing symbol.)

#### The named hops severed — verbatim

**Hop 1 — the stat publisher is replaced by an empty layout.**
`git show 1a52f2db -- src/incubator_dual.cpp`
(pre-image blob `0bceaa691bb1…`, post-image `77cb1ed93e19…`):

```diff
-    // Both cartridges are initialized; publish the slot map once so the render
-    // side can resolve coupling sources by name (the_board's fog coupling reads
-    // "all.field"). STAT_LAYOUT is constexpr, valid the moment analysis exists.
-    render.bind_signal_layout(analysis.stat_layout());
+    // Publish the slot map once. The BeatClock's layout is EMPTY by design
+    // (CUT_1c): every render-side resolve misses, warns once on stderr, and
+    // leaves its coupling disabled — the graceful path in signal_layout.hpp.
+    render.bind_signal_layout(clock.stat_layout());
```

**Hop 2 — the per-frame signal source is replaced.**

```diff
         // --- Update ---------------------------------------------------------
-        analysis.update(dt);
-
-        // (canvas_1 publishes the union field; the render side reads it through
-        // the bound stat layout — no per-cartridge debug stat to print here.)
-
-        render.update(analysis.output(), console.aspect_ratio(), queue);
+        clock.update(dt);
+        render.update(clock.output(), console.aspect_ratio(), queue);
```

**Hop 3 — the MIDI intake / analysis cartridge instantiation is removed.**

```diff
-    // --- Initialize Analysis Cartridge --------------------------------------
-    AnalysisCartridge analysis;
-    analysis.initialize("assets");
-
-    // canvas_1's source is the DAW transport (loopMIDI), not a MIDI file, so
-    // there is no command-line MIDI to load.
+    // --- The Clock -----------------------------------------------------------
+    // BeatClock needs no initialization: it starts at zero and advances
+    // from dt alone. No command-line input either.
+    t7::BeatClock clock;
     (void)argc; (void)argv;
```

```diff
-namespace analysis_ns = t7::INCUBATE_ANALYSIS;
 namespace render_ns = t7::INCUBATE_RENDER;
 
-using AnalysisCartridge = analysis_ns::Canvas;
 using RenderCartridge = render_ns::Cartridge;
```

**Hop 4 — the music-key input route is removed.**

```diff
-// =========================================================================
-// INPUT ROUTING -- the keyboard is the WORLD'S by default (PANEL-0 p1a)
-// =========================================================================
-//
-// is_music_key routed the QWERTY letters to the analysis cartridge back
-// when the computer keyboard WAS the note source. Ableton is the source
-// now, so every key falls to render — the world owns W/A/S/D and the rest.
-// (The INCUBATE_MUSIC_KEYS escape hatch was never defined anywhere, so it
-//  had never compiled and would not have worked if switched on; deleted
-//  PRUNING_1 P1 Step 4. git has the routing if it is ever wanted back.)
-static bool is_music_key(int key) {
-    (void)key;   // the keyboard is the world's; the note source is Ableton
-    return false;
-}
```

```diff
-        // --- Input Routing --------------------------------------------------
+        // --- Input (all of it is the world's) --------------------------------
         for (const auto& event : console.input_events()) {
-            if (event.type == t7::InputEvent::Type::KeyDown ||
-                event.type == t7::InputEvent::Type::KeyUp) {
-
-                if (is_music_key(event.key)) {
-                    analysis.on_input(event);
-                }
-                else {
-                    render.on_input(event);
-                }
-            }
-            else {
-                render.on_input(event);
-            }
+            render.on_input(event);
         }
```

**Hop 5 — the stats that stopped being published.** The deleted publisher is
`t7::canvas_1::Canvas` in `src/analysis/canvas_1/canvas.hpp` (pre-image blob
`250b6e56310c…`, 686 lines). The enclosing symbol of all thirteen publish call
sites is **`Canvas::initialize`** (an `override`), which first runs
`Canvas::configure` once per voice and then declares the readings through
`Canvas::publish_reading`; `configure` and `publish_reading` are the members it
calls, not the members the calls sit in.

**Correction of a prior R3 anchor in this section.** An earlier draft of this
paragraph named the publisher `t7::canvas_1::Canvas::configure`/`publish_reading`.
Re-reading `git cat-file blob '1a52f2db^:src/analysis/canvas_1/canvas.hpp'` around
the call sites shows they are inside `void initialize(const char* asset_path) override`,
whose body opens `constexpr int VOICES = 7;` and closes on
`port_.open_by_name("loopMIDI");   // the DAW's virtual port`. The enclosing symbol
is corrected to `Canvas::initialize` throughout.
`git cat-file blob '1a52f2db^:src/analysis/canvas_1/canvas.hpp' | grep -n 'publish_reading('`
returns **16** lines. Complete and unedited, line-number prefixes intact:

```
58:// same frame directly through configure(), publish_reading(), route() and
122:            publish_reading(Reading::CurrentPC,    Source::channel(v), NAME_CURRENT_PC[v]);
123:            publish_reading(Reading::PresentCount, Source::channel(v), NAME_PRESENT_COUNT[v]);
124:            publish_reading(Reading::WindowLength, Source::channel(v), NAME_WINDOW_LENGTH[v]);
125:            publish_reading(Reading::Distance,     Source::channel(v), NAME_DISTANCE[v]);
128:            publish_reading(Reading::DftMag,       Source::channel(v), NAME_DFT_MAG[v]);
129:            publish_reading(Reading::DftPhase,     Source::channel(v), NAME_DFT_PHASE[v]);
130:            publish_reading(Reading::Onset,        Source::channel(v), NAME_ONSET[v]);
138:        publish_reading(Reading::Field,        all, "all.field");
139:        publish_reading(Reading::CurrentPC,    all, "all.current_pc");
140:        publish_reading(Reading::PresentCount, all, "all.present_count");
141:        publish_reading(Reading::WindowLength, all, "all.window_length");
142:        publish_reading(Reading::DftMag,       all, "all.dft_mag");
143:        publish_reading(Reading::DftPhase,     all, "all.dft_phase");
168:    // configure() composes one analysis slot; publish_reading() declares a
227:    bool publish_reading(Reading r, const Source& src, const char* name, int want_band = -1) {
```

Of those 16 lines, three are not call sites and were previously dropped from this
block without a marker: two prose lines (at the file's banner and above the
declaration) and the declaration of the member function itself,
`bool publish_reading(Reading r, const Source& src, const char* name, int want_band = -1)`,
which is `Canvas::publish_reading`. The remaining 13 are the call sites — seven
per-channel publishes inside the per-voice loop and six `all.*` publishes after it.
The two line numbers the listing skips inside the loop (126, 127) hold lines that do
not match the pattern; the grep output is contiguous as printed. Line numbers here
are reproduced only because they are part of `grep -n`'s literal output; the claim
is anchored to `t7::canvas_1::Canvas::initialize` (the enclosing symbol of the
thirteen call sites) and `t7::canvas_1::Canvas::publish_reading` (the member
declared at the sixteenth matching line).

The replacement, `t7::BeatClock::stat_layout` in the added
`src/analysis/beat_clock.hpp` (blob at `1a52f2db` = `715b0c5c8f80…`), publishes
nothing, and its banner names the exact hops it breaks:

```cpp
// The empty layout is the audio socket. The render side resolves 12
// live source names against it — all.field, ch1.present_count,
// all.window_length, all.present_count, ch1.window_length,
// ch0.onset .. ch6.onset — and every resolve misses and disables its
// coupling via the graceful path (musical/signal_layout.hpp
// resolve(): one stderr warn, valid=false). A future browser-side
// audio source plugs into this socket by publishing exactly those
// names through a real StatLayoutView.
…
    StatLayoutView stat_layout() const { return StatLayoutView{ nullptr, 0 }; }
```

**Hop 6 — the vestigial include drop in the render state.**
`git show 1a52f2db -- src/cartridges/the_board/realization/state.hpp`
(pre-image `429b0c73783a…`, post-image `7467e2b4240d…`), one line:

```diff
-#include "analysis/analysis_signal.hpp"
 #include "core/instruments.hpp"                  // THE INSTRUMENTS DIAL: …
```

**Hop 7 — the MIDI backend leaves the build.**
`git diff 1a52f2db^..1a52f2db -- CMakeLists.txt` (pre-image `2fee93578fb8…`,
post-image `6860f3ffde3b…`), the two load-bearing hunks:

```diff
-set(MSVC_COMPILE_DEFS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS __WINDOWS_MM__)
+set(MSVC_COMPILE_DEFS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
```

```diff
 add_executable(incubator_dual
     src/incubator_dual.cpp
     src/external/stb_image.cpp
-    src/external/RtMidi.cpp
     ${DUAL_RENDER_HEADERS}
-    ${DUAL_ANALYSIS_HEADERS}
 )
```

plus the deletion of `INCUBATOR_ANALYSIS_CARTRIDGE`,
`INCUBATOR_DUAL_ANALYSIS_CARTRIDGE`, the `canvas_1` self-heal `foreach`, the
`INCUBATE_ANALYSIS` compile definition, and the whole `the_lab` / `probe_canvas` /
`check_*` target block (`CMakeLists.txt` 824 → 546 lines;
`git cat-file blob '1a52f2db^:CMakeLists.txt' | wc -l` → 824,
`git cat-file blob 1a52f2db:CMakeLists.txt | wc -l` → 546).

#### Full diffs of the touched arm files — **TRUNCATED, and why**

**CORRECTION — the file count in this paragraph was wrong, and the ~400-line
threshold is per-file, not aggregate.** An earlier draft said "21 deleted files
totalling 5076 deleted lines" and justified truncating all of them by aggregating
them against the threshold. Both halves are now restated from the tree.

The exact census. Files CUT_1c touched under the four directories the unit names:

```
$ git diff --name-only '1a52f2db^..1a52f2db' -- src/musical/ src/sources/ src/analysis/ src/coupling/ | wc -l
28
$ git diff --diff-filter=D --name-only '1a52f2db^..1a52f2db' -- src/musical/ src/sources/ src/analysis/ | wc -l
27
```

* `src/coupling/` — **zero** files touched, so nothing to quote.
* `src/analysis/` — 7 deleted (`analysis_cartridge.hpp` and the 6 `canvas_1/`
  files), 1 added (`beat_clock.hpp`, quoted in Hop 5 above). 8 touched.
* `src/musical/` — 15 deleted. `signal_layout.hpp` untouched.
* `src/sources/` — 5 deleted.

**27 deleted files**, not 21. The per-file deleted-line counts, taken with
`git diff --numstat '1a52f2db^..1a52f2db' -- <path>` (field 2), sum to **5076** —
that total was correct; only the file count attached to it was not.

Applying the threshold **per file**, as the unit's (f) wording sets it: exactly
**three** of the 27 exceed ~400 deleted lines —
`src/analysis/canvas_1/canvas.hpp` (686), `src/musical/stream_data.hpp` (489) and
`src/sources/midi_file.hpp` (469). The other **24 files, totalling 3432 deleted
lines**, are under the threshold and their full diffs are therefore owed. They are
supplied in **Appendix D-1** at the end of this section, quoted from
`git diff '1a52f2db^..1a52f2db' -- <path>` one file at a time, unabridged.

For the three over-threshold files the truncation stands and is declared here:
`--stat` above, top-level symbol inventory below, no raw hunks.

**Recoverability of the three truncated files, verified rather than asserted.** All
27 deleted files are byte-identical at `79adfa4d` to their `1a52f2db^` pre-image, so
`git show 1a52f2db^:<path>` reproduces the deleted text exactly. Checked by
comparing blob SHAs pairwise across all 27 paths:

```
$ for p in $(git diff --diff-filter=D --name-only '1a52f2db^..1a52f2db' \
      -- src/musical/ src/sources/ src/analysis/); do
    A=$(git rev-parse "1a52f2db^:$p")
    if git cat-file -e "79adfa4d:$p" 2>/dev/null; then B=$(git rev-parse "79adfa4d:$p"); else B=ABSENT; fi
    [ "$A" = "$B" ] || echo "MISMATCH $p $A $B"
  done
(no output — all 27 match)
```

identical = 27, diverged = 0, absent-at-HEAD = 0. The per-path blob SHAs are in
§(e)'s IDENTICAL/DIVERGED table.

**A units note on `src/musical/spine.hpp`.** The `--stat` above reports `271` for it
while §(f)'s table reports `pre=270`. Both are right and neither is a typo: the file
has no trailing newline, so `git diff --numstat` counts 271 deleted lines while
`git cat-file blob '1a52f2db^:src/musical/spine.hpp' | wc -l` → `270`, `wc -l`
counting newline bytes. Every other file in the set agrees between the two
measures.

Symbol inventories were produced with:

```
git show '1a52f2db^:<path>' | grep -nE '^[[:space:]]{0,4}(namespace|struct|class|enum|template|(inline |static |constexpr )*(void|bool|int|float|double|auto|std::|[A-Z][A-Za-z0-9_]*)[[:space:]]+[a-zA-Z_][A-Za-z0-9_]*\()'
```

| deleted path | blob @ `1a52f2db^` | lines | top-level symbols deleted |
| --- | --- | --- | --- |
| `src/analysis/analysis_cartridge.hpp` | `f07e11a992d2` | 103 | `namespace t7`; `class AnalysisCartridge` |
| `src/analysis/canvas_1/canvas.hpp` | `250b6e56310c` | 686 | `namespace t7::canvas_1`; `class Canvas : public AnalysisCartridge` with `initialize`, `update`, `on_input`, `enum class Reading`, `struct Source`, `configure`, `publish_reading`, `route`, `advance`, `stat_layout`, `active`, `is_open`, `struct ReadingSpec`, `reading_spec`, `reading_needs_window`, `reading_needs_spine`, `writer_wired`, `struct Published`, `available`, `publish`, `write_reading`, `write_vector`, `reading_vector`, `per_channel_reading`, `first_source`, `step_fields`, `union_present_set`, `field_index` |
| `src/analysis/canvas_1/check_canvas_compound.cpp` | `071ba66d5d5e` | 96 | `bin`, `near`, `setup`, `main` |
| `src/analysis/canvas_1/check_canvas_union.cpp` | `e8be1173416c` | 138 | `setup`, `field_of`, `main` |
| `src/analysis/canvas_1/check_field_union.cpp` | `5cbe0a21bf8d` | 108 | `pcs`, `print_set`, `expect_set`, `main` |
| `src/analysis/canvas_1/check_pc_dft.cpp` | `f35a0927631c` | 91 | `near`, `main` |
| `src/analysis/canvas_1/probe_canvas.cpp` | `7a341e9c00a0` | 196 | `print_binding`, `print_layout`, `print_published`, `note_changed`, `main` |
| `src/musical/context.hpp` | `a4c3977e08b4` | 187 | `namespace t7`; `class Context` — `set_channel`, `channel`, `set_retention_beats`, `add_wagon`, `wagon_count`, `enable_previous`, `enable_spine`, `receive`, `update`, `has_previous`, `has_spine`, `clear` |
| `src/musical/context_realize.hpp` | `428573032423` | 75 | `namespace t7`; `inline void realize(const ContextSpec&, Context&)` |
| `src/musical/context_spec.hpp` | `563962e7d0f0` | 191 | `struct WindowSpec`; `enum class OracleBinding`; `struct EventMemorySpec`; `struct CrossingMemorySpec`; `struct ContextSpec` (`add_window`, `has_present`, `has_event`, `has_crossings`, `window_count`, `has_window`, `required_retention_beats`); `default_spec` |
| `src/musical/field.hpp` | `5e1f38d74ded` | 127 | `present_set`; `struct Field`; `field_mask`; `field_overlap`; `struct FieldChoice` (`ambiguous`); `elect_field`; `struct HeldField` (`step`, `settled`) |
| `src/musical/midi_stream.hpp` | `f74d0b039cb6` | 179 | `class MidiStream` — `set_channel`, `channel`, `set_retention_beats`, `retention_beats`, `receive`, `update`, `snapshot`, `current_beat`, `had_time_discontinuity`, `active_count`, `is_silent`, `has_active`, `completed_count`, `has_completed`, `clear` |
| `src/musical/musical_ops.hpp` | `033b3a2f1a9f` | 192 | `pc_of`, `pc_relative_to`, `distance`; `struct PitchClassBits` (`set`, `clear`, `test`, `clear_all`, `count`, `empty`); `struct PitchClassVector` (`clear`, `sum`, `max`, `normalized`, `unit`); `to_degrees`; `present_union` |
| `src/musical/pc_count.hpp` | `acccba766e2a` | 132 | `pc_count` ×3 overloads, `pc_length` ×3 overloads, `pc_onset`, `pc_set` |
| `src/musical/pc_dft.hpp` | `8429dd9ba8a6` | 64 | `struct PcDft`; `pc_dft` |
| `src/musical/playhead.hpp` | `3172fe6cb587` | 179 | `struct CurrentNote`; `struct PlayheadReadout` (`gate`, `silent`, `has_overflow`, `current_duration`, `clear`); `class Playhead` (`update`, `clear`, `rebuild_current`, `detect_transitions`, `update_temporal`) |
| `src/musical/previous_event.hpp` | `b97550ce0566` | 194 | `struct PrevNote`; `class PreviousEvent` — `set_tolerance`, `tolerance`, `on_onset`, `on_offset`, `has_previous`, `previous_count`, `previous_overflow`, `previous_onset`, `previous_last_offset_index`, `previous_last_offset_pitch`, `has_open`, `open_count`, `open_overflow`, `open_onset`, `temporal_distance`, `closest_pitch`, `clear`, `iabs`, `push_open`, `latch` |
| `src/musical/spine.hpp` | `8e9c144193e4` | 270 | `enum class Provenance`; `class Spine` |
| `src/musical/spine_ops.hpp` | `c33c885fb9fb` | 52 | `current_note(const Spine&)`; `line_distance(const Spine&)` |
| `src/musical/stream_data.hpp` | `d84086d83e06` | 489 | `namespace bits` (`popcount64`, `ctz64`); `struct PitchBitmask`; `struct ActiveNote`; `struct CompletedNote`; `struct ActiveSet`; `struct CompletedRing` (`push`, `clear`, `prune_before`, `empty`, `size`, `most_recent`, `for_each`, `for_each_reverse`, `for_each_inside_window`, `for_each_overlapping_window`, `count_inside_window`, `count_overlapping_window`, `for_each_recent`, `most_recent_set`, `previous_before`); `struct StreamSnapshot` |
| `src/musical/vector_dressing.hpp` | `b220ae3e0194` | 54 | `struct VectorDressing`; `enum class Scale`; `dress` |
| `src/musical/wagon.hpp` | `f7b091df7971` | 164 | `struct WindowNote`; `struct WagonReadout`; `class Wagon` (`set_span`, `span`, `set_offset`, `offset`, `update`, `clear`, `rebuild`, `add_note`) |
| `src/sources/keyboard_midi.hpp` | `3416fa83a9f3` | 257 | `class KeyboardMidi` — `set_channel`, `channel`, `set_velocity`, `velocity`, `set_octave_shift`, `octave_shift`, `on_key_press`, `on_key_release`, `release_all`, `poll`, `pending_count`, `held_count`, `queue_event`, `init_piano_layout` |
| `src/sources/midi_event.hpp` | `b7c41853c298` | 59 | `struct MidiEvent`; `enum Type`; `note_on`; `note_off` |
| `src/sources/midi_file.hpp` | `65c95697c244` | 469 | `namespace midi_internal`; `struct RawEvent`; `parse_file`; `struct MidiTrackInfo`; `class MidiFile` — `load`, `clear`, `set_channel`, `channel`, `set_loop`, `loop`, `poll`, `is_loaded`, `duration_beats`, `reset`, `struct Message`, `clear_open_notes`, `reset_playback`, `poll_range`, `flush_open_notes_to_buffer` |
| `src/sources/midi_port.hpp` | `293ce7c46f66` | 213 | `class MidiPort` — `open`, `open_by_name`, `close`, `is_open`, `playing`, `beats`, `bpm`, `ever_synced`, `poll`, `pending_count`, `on_rtmidi_callback`, `handle_message`, `push`, `icontains` |
| `src/sources/transport.hpp` | `d21388c5daf2` | 110 | `class MidiTransport` — `feed`, `playing`, `beats`, `bpm`, `ever_synced`, `reset`, `on_clock` |

---

### (e) BOUNDARY C — the restoration, `web-sunset..HEAD`

`60a3e935ce459326559a7a9ca377192944b36d8d` .. `79adfa4d26c9e17e0074692928f1d2875d7edde1`

#### `git diff --stat 60a3e935..HEAD -- src/`

```
 src/analysis/analysis_cartridge.hpp                |  103 +
 src/analysis/canvas_1/canvas.hpp                   |  686 ++
 src/analysis/canvas_1/check_canvas_compound.cpp    |   96 +
 src/analysis/canvas_1/check_canvas_union.cpp       |  138 +
 src/analysis/canvas_1/check_field_union.cpp        |  108 +
 src/analysis/canvas_1/check_pc_dft.cpp             |   91 +
 src/analysis/canvas_1/probe_canvas.cpp             |  196 +
 src/cartridges/the_board/bodies/gallery.hpp        | 3291 --------
 src/cartridges/the_board/bodies/gol_zones.hpp      |    4 +-
 src/cartridges/the_board/bodies/grounded.hpp       |    4 +-
 src/cartridges/the_board/bodies/ribbon.hpp         |    2 +-
 src/cartridges/the_board/cartridge.hpp             |  160 +-
 src/cartridges/the_board/contracts/agent_tiers.hpp |    5 +-
 src/cartridges/the_board/contracts/demo_config.hpp |    4 +-
 .../the_board/contracts/entity_types.hpp           |   41 +-
 .../the_board/contracts/ground_architecture.hpp    |    2 +-
 .../the_board/contracts/indoor_module.hpp          |   19 +-
 src/cartridges/the_board/contracts/point.hpp       |    2 +-
 src/cartridges/the_board/contracts/roster.hpp      |   37 +-
 .../the_board/contracts/spawn_services.hpp         |   47 +-
 src/cartridges/the_board/contracts/spine_state.hpp |    2 +-
 .../the_board/contracts/surface_services.hpp       |   24 +-
 src/cartridges/the_board/demos/matrix.hpp          |   11 +-
 src/cartridges/the_board/direction/mood.hpp        |   56 +-
 src/cartridges/the_board/machine/spawn_engine.hpp  |   72 +-
 src/cartridges/the_board/primitives/seed_utils.hpp |    4 +-
 .../the_board/realization/binding_registry.hpp     |   15 +-
 .../the_board/realization/binding_surface.gen.inc  |  325 +-
 .../the_board/realization/drawable_table.hpp       |   59 +-
 .../the_board/realization/render_passes.hpp        |   92 +-
 src/cartridges/the_board/realization/renderer.hpp  |  558 +-
 src/cartridges/the_board/realization/state.hpp     |  499 +-
 src/cartridges/the_board/realization/world.wgsl    |  728 +-
 src/cartridges/the_board/surface/patch_system.hpp  |    8 +-
 .../the_board/surface/population_themes.hpp        |   32 +-
 src/console/console.hpp                            | 1520 +---
 src/console/features_wallet.gen.inc                |    2 +-
 src/console/organ_params.inc                       |   17 +-
 src/core/boot_params.hpp                           |   95 +-
 src/core/instruments.hpp                           |   47 +-
 src/core/sha256.hpp                                |  143 -
 src/coupling/organ_registry.hpp                    |  999 +++
 src/external/RtMidi.cpp                            | 5275 +++++++++++++
 src/external/RtMidi.h                              |  686 ++
 src/external/stb_image.cpp                         |    2 -
 src/external/stb_image.h                           | 7988 --------------------
 src/musical/context.hpp                            |  187 +
 src/musical/context_realize.hpp                    |   75 +
 src/musical/context_spec.hpp                       |  191 +
 src/musical/field.hpp                              |  127 +
 src/musical/midi_stream.hpp                        |  179 +
 src/musical/musical_ops.hpp                        |  192 +
 src/musical/pc_count.hpp                           |  132 +
 src/musical/pc_dft.hpp                             |   64 +
 src/musical/playhead.hpp                           |  179 +
 src/musical/previous_event.hpp                     |  194 +
 src/musical/spine.hpp                              |  271 +
 src/musical/spine_ops.hpp                          |   52 +
 src/musical/stream_data.hpp                        |  489 ++
 src/musical/vector_dressing.hpp                    |   54 +
 src/musical/wagon.hpp                              |  164 +
 src/sources/keyboard_midi.hpp                      |  257 +
 src/sources/midi_event.hpp                         |   59 +
 src/sources/midi_file.hpp                          |  469 ++
 src/sources/midi_port.hpp                          |  213 +
 src/sources/transport.hpp                          |  110 +
 src/the_board.cpp                                  |  169 +-
 67 files changed, 12521 insertions(+), 15601 deletions(-)
```

Note the four `src/coupling/` files that are **absent** from that stat:
`canvas_surface.hpp`, `trajectory.hpp`, `visual_canvas.hpp`, `visual_params.hpp`
are unchanged across `60a3e935..HEAD` — confirming the orchestrator's established
context. `organ_registry.hpp` is the only `src/coupling/` entry, and it is an add.

#### Added-file list — `git diff --diff-filter=A --name-only 60a3e935..HEAD` (42 files, whole tree)

```
src/analysis/analysis_cartridge.hpp
src/analysis/canvas_1/canvas.hpp
src/analysis/canvas_1/check_canvas_compound.cpp
src/analysis/canvas_1/check_canvas_union.cpp
src/analysis/canvas_1/check_field_union.cpp
src/analysis/canvas_1/check_pc_dft.cpp
src/analysis/canvas_1/probe_canvas.cpp
src/coupling/organ_registry.hpp
src/external/RtMidi.cpp
src/external/RtMidi.h
src/musical/context.hpp
src/musical/context_realize.hpp
src/musical/context_spec.hpp
src/musical/field.hpp
src/musical/midi_stream.hpp
src/musical/musical_ops.hpp
src/musical/pc_count.hpp
src/musical/pc_dft.hpp
src/musical/playhead.hpp
src/musical/previous_event.hpp
src/musical/spine.hpp
src/musical/spine_ops.hpp
src/musical/stream_data.hpp
src/musical/vector_dressing.hpp
src/musical/wagon.hpp
src/sources/keyboard_midi.hpp
src/sources/midi_event.hpp
src/sources/midi_file.hpp
src/sources/midi_port.hpp
src/sources/transport.hpp
third_party/dawn_native_headers/PINNED.md
third_party/dawn_native_headers/include/dawn/dawn_proc.h
third_party/dawn_native_headers/include/dawn/dawn_proc_table.h
third_party/dawn_native_headers/include/dawn/native/DawnNative.h
third_party/dawn_native_headers/include/dawn/native/dawn_native_export.h
third_party/dawn_native_headers/include/dawn/webgpu.h
third_party/dawn_native_headers/include/dawn/webgpu_cpp.h
third_party/dawn_native_headers/include/webgpu/webgpu.h
third_party/dawn_native_headers/include/webgpu/webgpu_cpp.h
third_party/dawn_native_headers/include/webgpu/webgpu_cpp_chained_struct.h
third_party/dawn_native_headers/include/webgpu/webgpu_enum_class_bitmasks.h
tools/gates/console_gate/stubs/GLFW/glfw3native.h
```

#### Deleted-file list — `git diff --diff-filter=D --name-only 60a3e935..79adfa4d` (78 paths, whole tree)

Recipe and totals, re-run pinned to `79adfa4d`:

```
$ git diff --diff-filter=D --name-only 60a3e935..79adfa4d | wc -l
78
$ git diff --diff-filter=D --name-only 60a3e935..79adfa4d | grep -c '^assets/paintings/'
57
$ git diff --diff-filter=D --name-only 60a3e935..79adfa4d | grep -vc '^assets/paintings/'
21
```

57 paintings + 21 non-painting paths = 78. An earlier draft printed **76** here and
**58** for the paintings; both numbers were wrong against the recipe printed beside
them. The enumerated painting list below was and remains correct — it holds exactly
57 entries and matches the tree item for item.

```
0
0`
16)
17)
assets/entrance/ENTRANCE_CONTROLS.jpg
assets/entrance/ENTRANCE_FIELD.jpeg
assets/music/samsara.mp3
assets/paintings/PAINTING_*.{jpg,jpeg}          (57 files — the full list is
    PAINTING_1, 10, 100, 1001, 1002, 101, 102, 103, 104, 105, 106, 107, 108,
    109, 11, 110, 111, 112, 113, 114, 115, 12, 14, 2, 200, 201, 202, 203, 205,
    206, 207, 208, 209, 210, 211, 212, 213, 214, 3, 32, 4, 5, 50, 500, 501, 6,
    60, 7, 70, 71, 72, 73, 8, 9, 90, 900, 92)
src/cartridges/the_board/bodies/gallery.hpp
src/core/sha256.hpp
src/external/stb_image.cpp
src/external/stb_image.h
tools/gates/console_gate/stubs/GLFW/emscripten_glfw3.h
tools/gates/console_gate/stubs/emscripten.h
tools/gates/console_gate/stubs/emscripten/em_types.h
tools/gates/console_gate/stubs/emscripten/fetch.h
tools/gates/console_gate/stubs/emscripten/html5.h
tools/gates/sha256_gate/run.py
tools/gates/shell_gate/run.py
tools/web_dist.py
web/index.html
web/organ_panel.js
```

The four odd names `0`, `0\``, `16)`, `17)` are real: `git ls-tree 60a3e935` shows
four zero-byte blobs (`e69de29b…`) at the repo root, swept by WEB_SUNSET W0.

#### IDENTICAL vs DIVERGED — restored content against its pre-CUT_1c pre-image

Recipe, run per path:

```
git rev-parse HEAD:<path>
git cat-file -e '1a52f2db^:<path>' && git rev-parse '1a52f2db^:<path>' || echo absent
```

(Every `src/musical/`, `src/sources/`, `src/analysis/` and `src/coupling/` path in
`git ls-tree -r --name-only HEAD` is covered; `src/external/RtMidi.*` is appended
because it is part of the same MIDI arm even though it lives outside the three
directories.)

| path | blob @ HEAD | blob @ `1a52f2db^` | verdict |
| --- | --- | --- | --- |
| `src/analysis/analysis_cartridge.hpp` | `f07e11a992d2` | `f07e11a992d2` | **IDENTICAL** |
| `src/analysis/analysis_signal.hpp` | `d088796d0ece` | `d088796d0ece` | **IDENTICAL** (never deleted) |
| `src/analysis/beat_clock.hpp` | `b10038ff5069` | *(absent)* | **was-absent** — created *by* CUT_1c; diverged since, see below |
| `src/analysis/canvas_1/canvas.hpp` | `250b6e56310c` | `250b6e56310c` | **IDENTICAL** |
| `src/analysis/canvas_1/check_canvas_compound.cpp` | `071ba66d5d5e` | `071ba66d5d5e` | **IDENTICAL** |
| `src/analysis/canvas_1/check_canvas_union.cpp` | `e8be1173416c` | `e8be1173416c` | **IDENTICAL** |
| `src/analysis/canvas_1/check_field_union.cpp` | `5cbe0a21bf8d` | `5cbe0a21bf8d` | **IDENTICAL** |
| `src/analysis/canvas_1/check_pc_dft.cpp` | `f35a0927631c` | `f35a0927631c` | **IDENTICAL** |
| `src/analysis/canvas_1/probe_canvas.cpp` | `7a341e9c00a0` | `7a341e9c00a0` | **IDENTICAL** |
| `src/musical/context.hpp` | `a4c3977e08b4` | `a4c3977e08b4` | **IDENTICAL** |
| `src/musical/context_realize.hpp` | `428573032423` | `428573032423` | **IDENTICAL** |
| `src/musical/context_spec.hpp` | `563962e7d0f0` | `563962e7d0f0` | **IDENTICAL** |
| `src/musical/field.hpp` | `5e1f38d74ded` | `5e1f38d74ded` | **IDENTICAL** |
| `src/musical/midi_stream.hpp` | `f74d0b039cb6` | `f74d0b039cb6` | **IDENTICAL** |
| `src/musical/musical_ops.hpp` | `033b3a2f1a9f` | `033b3a2f1a9f` | **IDENTICAL** |
| `src/musical/pc_count.hpp` | `acccba766e2a` | `acccba766e2a` | **IDENTICAL** |
| `src/musical/pc_dft.hpp` | `8429dd9ba8a6` | `8429dd9ba8a6` | **IDENTICAL** |
| `src/musical/playhead.hpp` | `3172fe6cb587` | `3172fe6cb587` | **IDENTICAL** |
| `src/musical/previous_event.hpp` | `b97550ce0566` | `b97550ce0566` | **IDENTICAL** |
| `src/musical/signal_layout.hpp` | `8e2e84312483` | `9468d32cb667` | **DIVERGED** (never deleted; changed by PORT_4c) |
| `src/musical/spine.hpp` | `8e9c144193e4` | `8e9c144193e4` | **IDENTICAL** |
| `src/musical/spine_ops.hpp` | `c33c885fb9fb` | `c33c885fb9fb` | **IDENTICAL** |
| `src/musical/stream_data.hpp` | `d84086d83e06` | `d84086d83e06` | **IDENTICAL** |
| `src/musical/vector_dressing.hpp` | `b220ae3e0194` | `b220ae3e0194` | **IDENTICAL** |
| `src/musical/wagon.hpp` | `f7b091df7971` | `f7b091df7971` | **IDENTICAL** |
| `src/sources/keyboard_midi.hpp` | `3416fa83a9f3` | `3416fa83a9f3` | **IDENTICAL** |
| `src/sources/midi_event.hpp` | `b7c41853c298` | `b7c41853c298` | **IDENTICAL** |
| `src/sources/midi_file.hpp` | `65c95697c244` | `65c95697c244` | **IDENTICAL** |
| `src/sources/midi_port.hpp` | `293ce7c46f66` | `293ce7c46f66` | **IDENTICAL** |
| `src/sources/transport.hpp` | `d21388c5daf2` | `d21388c5daf2` | **IDENTICAL** |
| *(supplementary)* `src/external/RtMidi.cpp` | `2303bb132273` | `2303bb132273` | **IDENTICAL** |
| *(supplementary)* `src/external/RtMidi.h` | `2801037f628d` | `2801037f628d` | **IDENTICAL** |
| *(supplementary)* `src/coupling/canvas_surface.hpp` | `336d5d320f3a` | *(absent)* | **was-absent** — born `da0ae12d` (ORGAN_3b P2, 2026-08-19) |
| *(supplementary)* `src/coupling/organ_registry.hpp` | `3047070e199d` | *(absent)* | **NEW** — born `72df32df` (2026-08-30) |
| *(supplementary)* `src/coupling/trajectory.hpp` | `a156425a5cc2` | `6d65eabe4f54` | **DIVERGED** — trailing newline only (`7e76bec5`) |
| *(supplementary)* `src/coupling/visual_canvas.hpp` | `ab5a21993a98` | `3f3a3c8c7874` | **DIVERGED** |
| *(supplementary)* `src/coupling/visual_params.hpp` | `c196529d08b9` | `084585af49c4` | **DIVERGED** — trailing newline only (`7e76bec5`) |

**Headline of (e): every single restored file — all 15 `src/musical/` headers, all 5
`src/sources/` headers, `analysis_cartridge.hpp`, all 5 `canvas_1/` files, and both
`RtMidi` files — is byte-identical to its pre-CUT_1c pre-image. Zero divergence in
the restored set.** The only DIVERGED files in the arm are files CUT_1c never
deleted (`signal_layout.hpp`, the three `src/coupling/` headers).

#### The DIVERGED files, by symbol

**`src/musical/signal_layout.hpp`** — `git diff --stat 1a52f2db^ HEAD -- src/musical/signal_layout.hpp`
→ `1 file changed, 17 insertions(+), 1 deletion(-)`. Pre `9468d32cb667…`, post `8e2e84312483…`.
Changed symbols, all in `class t7::SignalLayout`:

* `SignalLayout::bind` — now resets a counter: `void bind(StatLayoutView v) { view_ = v; misses_ = 0; }`
* `SignalLayout::misses` — **new** accessor, `uint32_t misses() const`.
* `SignalLayout::resolve` — increments `misses_` on a miss, and the per-source
  `std::fprintf(stderr, "[SignalLayout] source '%.*s' not in layout (coupling disabled)\n", …)`
  is now wrapped in `#ifndef NDEBUG`.
* `SignalLayout::misses_` — **new** member, `mutable uint32_t misses_ = 0`.

Attributed in-file to `PORT_4c`. **Complete, unabridged diff** — this is the entire
output of `git diff '1a52f2db^' 79adfa4d -- src/musical/signal_layout.hpp`, hunk
headers and inter-hunk context included. An earlier draft labelled an abridged
rendering of this as the full diff: it kept every `+`/`-` line byte-verbatim but
replaced each `@@ … @@` hunk header and the context lines between hunks with a bare
`@@`. The real output follows.

```diff
diff --git a/src/musical/signal_layout.hpp b/src/musical/signal_layout.hpp
index 9468d32c..8e2e8431 100644
--- a/src/musical/signal_layout.hpp
+++ b/src/musical/signal_layout.hpp
@@ -40,7 +40,15 @@ struct SourceBinding {
 
 class SignalLayout {
 public:
-    void bind(StatLayoutView v) { view_ = v; }
+    void bind(StatLayoutView v) { view_ = v; misses_ = 0; }
+
+    // PORT_4c — how many resolves missed since the last bind(). The
+    // release twin prints ONE summary line from this instead of one
+    // line per source: with no audio source every resolve misses, and
+    // twenty stderr lines read like twenty faults to a visitor who
+    // opened DevTools out of curiosity. It is one fact — the socket is
+    // empty — so it gets one line. The debug twin still names each.
+    uint32_t misses() const { return misses_; }
 
     // Look up a source by name. Returns {valid=false} and warns on stderr
     // if the name is absent — callers leave the coupling disabled rather
@@ -52,9 +60,16 @@ public:
                 return SourceBinding{ g.channel, g.slot_base, g.count, true };
             }
         }
+        ++misses_;
+#ifndef NDEBUG
+        // Debug twin only (the-board-web-debug): the full list, one line
+        // per source, unchanged. NDEBUG is the gate because CMake
+        // defines it for Release and not for Debug, which is exactly the
+        // two-preset split PORT_2c installed.
         std::fprintf(stderr,
             "[SignalLayout] source '%.*s' not in layout (coupling disabled)\n",
             (int)name.size(), name.data());
+#endif
         return SourceBinding{};              // valid = false
     }
 
@@ -63,6 +78,7 @@ public:
 
 private:
     StatLayoutView view_{ nullptr, 0 };
+    mutable uint32_t misses_ = 0;   // resolve() is const; the tally is not state the caller owns
 };
 
 } // namespace t7
```

Three hunk headers, `@@ -40,7 +40,15 @@ struct SourceBinding {`,
`@@ -52,9 +60,16 @@ public:` and `@@ -63,6 +78,7 @@ public:`, locate the change in
`class SignalLayout`; the context lines carried between them are the `resolve`
doc-comment, the `return SourceBinding{ g.channel, g.slot_base, g.count, true };`
success path, `return SourceBinding{};              // valid = false`, and the
closing `};` of the class. No substantive claim above changes: the four symbol-level
statements about `SignalLayout::bind`, `SignalLayout::misses`, `SignalLayout::resolve`
and `SignalLayout::misses_` and the stat
`1 file changed, 17 insertions(+), 1 deletion(-)` all reproduce exactly.

**`src/analysis/beat_clock.hpp`** — absent at `1a52f2db^`, so compared instead
against the version CUT_1c created:
`git diff --stat 1a52f2db HEAD -- src/analysis/beat_clock.hpp`
→ `1 file changed, 21 insertions(+), 15 deletions(-)`. Blob at `1a52f2db` = `715b0c5c8f80…`,
at HEAD = `b10038ff5069…`. Changed symbols in `struct t7::BeatClock`:

* `BeatClock::update` — writes into a persistent member instead of three scalars:
  `signal_.dt`, `signal_.t_seconds`, `signal_.t_beats`.
* `BeatClock::output` — signature changed from `AnalysisSignal output() const`
  (returning a per-frame copy) to `const AnalysisSignal& output() const`
  (returning a reference into the member). Attributed to `OIL_1 U2`.
* members `seconds_`, `beats_`, `dt_` **removed**; member `AnalysisSignal signal_{}`
  **added**.
* `BeatClock::bpm` and `BeatClock::stat_layout` unchanged — `stat_layout()` still
  returns `StatLayoutView{ nullptr, 0 }`.

**`src/coupling/trajectory.hpp`** and **`src/coupling/visual_params.hpp`** — the
whole divergence is the one trailing `\n` appended by `7e76bec5`; diffs quoted in
full in §(c).

**`src/coupling/visual_canvas.hpp`** — `git diff --stat '1a52f2db^' 79adfa4d -- src/coupling/visual_canvas.hpp`
→ verbatim:

```
 src/coupling/visual_canvas.hpp | 173 ++++++++++++++++++++++++-----------------
 1 file changed, 101 insertions(+), 72 deletions(-)
```

i.e. **101 insertions, 72 deletions** (an earlier draft printed 106/67, which the
recipe does not yield). The companion figure is correct:
`git diff '1a52f2db^' 79adfa4d -- src/coupling/visual_canvas.hpp | wc -l` → **385**
diff lines. Every symbol-level statement below was re-checked against the tree and
reproduces.
Pre `3f3a3c8c7874…`, post `ab5a21993a98…`. Changed by symbol:

* File banner + `#include` block — adds `<array>`, `<cstddef>`, and
  `#include "coupling/canvas_surface.hpp"` (`ORGAN_3b P2 — CANVAS_LIVE`).
* **Deleted compile-time constants**, all migrated to `canvas::CANVAS_LIVE`:
  `FOG_SPAN`, `RIBBON_SWELL_CEILING`, `RIBBON_SWELL_RAMP`, `RIBBON_SWELL_ATTACK`,
  `RIBBON_SWELL_RELEASE`, `PITCH_VEC_ORIGIN`, `TINT_LUMA`, `TINT_CHROMA`,
  `TINT_MIX_MAX`, `TINT_MIX_ATTACK`, `TINT_MIX_RELEASE`, `TINT_HUE_SPAN`,
  `CHECKER_READ_SPAN`, `CHECKER_ATTACK`, `CHECKER_RELEASE`.
* `PARAM_LAYOUT` — `fog.density` and `fog.color` rests changed from
  `FOG_DENSITY_NONE` / `0.80f` to `0.0f` / `0.0f` (ATMOS_1: the canvas now emits a
  deviation from the anchor row, not an absolute).
* `VisualCanvas::bind` — the fog `Segment`s seed at `0.0f` instead of
  `FOG_DENSITY_NONE` / `FOG_COLOR_BY_FIELD[0][c]`; a new
  `if (signal_layout_.misses() > 0) std::fprintf(stderr, "[SignalLayout] %u sources unbound (no audio source)\n", …)`
  block is appended (PORT_4c).
* `VisualCanvas::tick` — fog now sets `FOG_BY_FIELD[idx] - FOG_BY_FIELD[0]` and
  `FOG_COLOR_BY_FIELD[idx][c] - FOG_COLOR_BY_FIELD[0][c]`; the hue loop now reads a
  function-local `static const std::array<std::array<float,2>,12> PITCH_VECS`
  (`OIL_1 U5`) instead of calling `std::cos`/`std::sin` per frame; every span/gain
  constant is read from `canvas::CANVAS_LIVE.*`.
* **Unchanged across the whole arc**: every `signal_layout_.resolve(...)` call site
  and its name. The grep yields **16 lines on each side**, not the six an earlier
  draft listed; the six are the `signal_layout_.` subset of those 16. Both outputs
  in full:

  ```
  $ git cat-file blob '1a52f2db^:src/coupling/visual_canvas.hpp' | grep -n 'resolve('
  55://   auto d = visual_canvas_.layout().resolve("fog.density");   // entity, once
  56://   auto k = visual_canvas_.layout().resolve("fog.color");     // base..base+2
  284:            fog_field_ = signal_layout_.resolve("all.field");
  285:            fog_density_ = param_layout_.resolve("fog.density");
  286:            fog_color_ = param_layout_.resolve("fog.color");
  298:                voice_playhead_ = signal_layout_.resolve((v + ".present_count").c_str());
  300:            room_wagon_ = signal_layout_.resolve("all.window_length");
  301:            room_playhead_ = signal_layout_.resolve("all.present_count");
  302:            amp_lat_ = param_layout_.resolve("ribbon.amp_lateral_mult");
  303:            amp_vert_ = param_layout_.resolve("ribbon.amp_vertical_mult");
  306:            tint_stim_ = param_layout_.resolve("ribbon.color_stim");
  307:            tint_mix_ = param_layout_.resolve("ribbon.color_mix");
  317:                checker_win_ = signal_layout_.resolve((v + ".window_length").c_str());
  319:            checker_mean_ = param_layout_.resolve("terrain.checker_mean");   // 3: resultant rgb
  320:            checker_var_  = param_layout_.resolve("terrain.checker_var");    // 2: amount, variance
  339:                    signal_layout_.resolve((v + ".onset").c_str());
  ```

  ```
  $ git cat-file blob '79adfa4d:src/coupling/visual_canvas.hpp' | grep -n 'resolve('
  58://   auto d = visual_canvas_.layout().resolve("fog.density");   // entity, once
  59://   auto k = visual_canvas_.layout().resolve("fog.color");     // base..base+2
  284:            fog_field_ = signal_layout_.resolve("all.field");
  285:            fog_density_ = param_layout_.resolve("fog.density");
  286:            fog_color_ = param_layout_.resolve("fog.color");
  296:                voice_playhead_ = signal_layout_.resolve((v + ".present_count").c_str());
  298:            room_wagon_ = signal_layout_.resolve("all.window_length");
  299:            room_playhead_ = signal_layout_.resolve("all.present_count");
  300:            amp_lat_ = param_layout_.resolve("ribbon.amp_lateral_mult");
  301:            amp_vert_ = param_layout_.resolve("ribbon.amp_vertical_mult");
  304:            tint_stim_ = param_layout_.resolve("ribbon.color_stim");
  305:            tint_mix_ = param_layout_.resolve("ribbon.color_mix");
  315:                checker_win_ = signal_layout_.resolve((v + ".window_length").c_str());
  317:            checker_mean_ = param_layout_.resolve("terrain.checker_mean");   // 3: resultant rgb
  318:            checker_var_  = param_layout_.resolve("terrain.checker_var");    // 2: amount, variance
  337:                    signal_layout_.resolve((v + ".onset").c_str());
  ```

  The 16 decompose identically on both sides: **2** comment lines in the
  `VisualCanvas` doc banner, **8** `param_layout_.resolve(...)` call sites
  (`fog.density`, `fog.color`, `ribbon.amp_lateral_mult`, `ribbon.amp_vertical_mult`,
  `ribbon.color_stim`, `ribbon.color_mix`, `terrain.checker_mean`,
  `terrain.checker_var`), and **6** `signal_layout_.resolve(...)` call sites
  (`"all.field"`, `(v + ".present_count")`, `"all.window_length"`,
  `"all.present_count"`, `(v + ".window_length")`, `(v + ".onset")`). All 16 sit
  inside `VisualCanvas::bind` except the two banner comments. The load-bearing claim
  is unchanged and reproduces: the six `signal_layout_` names, and the two
  `(v + …)` templates among them, are byte-identical and in the same order on both
  sides of the whole arc. The three `(v + …)` forms expand per voice, which is how
  six call sites cover the twelve source names `beat_clock.hpp` enumerates.

  **FLAG — the verifier's count for this grep does not hold against the tree.** The
  adversarial verifier reported "15 lines, not 6" and decomposed them as two comment
  lines plus seven `param_layout_` call sites plus six `signal_layout_` call sites.
  My own re-run gives **16** lines on each side —
  `git cat-file blob '1a52f2db^:src/coupling/visual_canvas.hpp' | grep -c 'resolve('`
  → `16` and the same over `79adfa4d:` → `16` — and the `param_layout_` subset is
  **8**, not seven; the verifier's own enumeration in fact names eight targets while
  stating seven. 2 + 8 + 6 = 16. The verifier's substantive point stands (the earlier
  draft listed only the `signal_layout_` subset and did not say so); its arithmetic
  does not, and the tree's figures are recorded here.

#### FLAG — a finding the established context does not cover: TWO `organ_registry.hpp` at HEAD

`git ls-tree -r --name-only HEAD src/console/ src/coupling/` shows both
`src/console/organ_registry.hpp` and `src/coupling/organ_registry.hpp` exist at HEAD,
with **different** blobs:

```
HEAD:src/console/organ_registry.hpp   70d09e9602eb0f763a616da5303e14c34e7f44da
HEAD:src/coupling/organ_registry.hpp  3047070e199df57c2a7cd6d8f75cf028ec48b817
```

Blob trace (`for c in $(git rev-list --reverse 60a3e935..HEAD); do git rev-parse $c:src/console/organ_registry.hpp; done`):

* `60a3e935` .. `488d9f79` — `src/console/organ_registry.hpp` = `70d09e96…`
* `4cfc899b` (WEB_SUNSET W3e, *"the organ keeps its ABI and loses its browser — prose only"*)
  changes it to `3047070e…`, and it stays there through PRUNE_1 and both merges.
* `72df32df` (*"Systems operating"*) **adds** `src/coupling/organ_registry.hpp` carrying
  blob `3047070e…` — i.e. a byte-for-byte copy of the then-current console file.
* `79adfa4d` (HEAD, *"Systems operational"*) reverts `src/console/organ_registry.hpp`
  to `70d09e96…`, the pre-W3e blob — the same blob `60a3e935` carries
  (`git rev-parse 60a3e935:src/console/organ_registry.hpp` → `70d09e96…`).

Consequence recorded as fact, not as a proposal: at HEAD the tree holds two files
named `organ_registry.hpp`, one under `src/console/` at the pre-W3e content and one
under `src/coupling/` at the post-W3e content, and `git diff 60a3e935..HEAD` shows
the console one as unchanged because its endpoints coincide.

#### The build half of the restoration, verbatim

`git show e0e22e46 -- CMakeLists.txt` (blob `2dddc9202f4d…`, identical to `HEAD:CMakeLists.txt`):

```diff
-set(MSVC_COMPILE_DEFS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
+# __WINDOWS_MM__ selects RtMidi's Windows Multimedia backend. RtMidi
+# compiles to an empty TU without a backend define, and the link then
+# fails on RtMidiIn's constructor rather than on anything that names it.
+# winmm.lib is already carried at LEVEL 9 of the link list above.
+set(MSVC_COMPILE_DEFS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS
+    __WINDOWS_MM__)
@@
 add_executable(the_board
     src/the_board.cpp
+    # The one other translation unit: RtMidi's Windows MM backend, the
+    # canvas's route to the DAW's virtual port. Vendored, not header-only,
+    # so it compiles separately; __WINDOWS_MM__ below selects its backend.
+    src/external/RtMidi.cpp
     ${T7_RENDER_HEADERS}
 )
```

Recorded as fact, without inference: `src/the_board.cpp` at HEAD (blob `588174ecddb0…`)
still contains `#include "analysis/beat_clock.hpp"`, `t7::BeatClock clock;`,
`app->render.bind_signal_layout(app->clock.stat_layout());` and
`app->render.update(app->clock.output(), …)` — `git cat-file blob HEAD:src/the_board.cpp | grep -nE 'BeatClock|bind_signal_layout|\.output\(\)'`.
`git grep -n -E '#include *"(musical|sources|analysis)/' HEAD -- 'src/*'` returns
no include of `analysis/canvas_1/canvas.hpp` from any file outside `src/analysis/`.

---

### (f) Files outside `src/coupling/` that lost more than half their lines

Census method (exact, reproducible). Saved as a shell script and run once per
boundary; the test is `post_lines * 2 < pre_lines`, i.e. strictly more than half the
lines gone. Deleted files score `post = 0`.

```sh
#!/bin/sh
# usage: halfloss.sh PRE POST
PRE=$1; POST=$2
git diff --name-only "$PRE".."$POST" | while IFS= read -r p; do
  case "$p" in src/coupling/*) continue;; esac
  if git cat-file -e "$PRE:$p" 2>/dev/null; then a=$(git cat-file blob "$PRE:$p" | wc -l); else a=0; fi
  if git cat-file -e "$POST:$p" 2>/dev/null; then b=$(git cat-file blob "$POST:$p" | wc -l); else b=0; fi
  if [ "$a" -gt 0 ] && [ $((b * 2)) -lt "$a" ]; then printf '%s\tpre=%s\tpost=%s\n' "$p" "$a" "$b"; fi
done
```

Invocations: `halfloss.sh 2bedb4e2 29cec46b`, `halfloss.sh '1a52f2db^' 1a52f2db`,
`halfloss.sh 60a3e935 HEAD`.

**BOUNDARY A — zero qualifying files.** The script returns nothing. The largest
single change over the boundary, `state.hpp`, went 6527 → 4713 lines
(`git cat-file blob 2bedb4e2:src/cartridges/the_board/realization/state.hpp | wc -l`
→ 6527; `… 29cec46b: …` → 4713), a ratio of 0.72 — well above half.

**BOUNDARY B — 49 qualifying files, all deletions to zero.** An earlier draft
printed 50 in this sentence while the next clause said 49; the script yields **49**
and the two are now reconciled to the tree:

```
$ sh halfloss.sh '1a52f2db^' 1a52f2db | wc -l
49
$ diff <(sh halfloss.sh '1a52f2db^' 1a52f2db | cut -f1 | sort) \
       <(git diff --diff-filter=D --name-only '1a52f2db^..1a52f2db' | sort)
(no output — the two sets are identical)
```

They are exactly the 49 files in the `--diff-filter=D` list of §(d) and nothing
else: no *modified* file crossed the threshold (`CMakeLists.txt` 824 → 546 = 0.66;
`src/incubator_dual.cpp` 267 → 219 = 0.82; `state.hpp` −1 line).

Per-file `pre`/`post` and disposition:

| file | pre | post | full diff given? |
| --- | --- | --- | --- |
| `src/analysis/analysis_cartridge.hpp` | 103 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/analysis/canvas_1/canvas.hpp` | 686 | 0 | **truncated, declared** → symbol table in §(d); over the ~400-line threshold, so no raw hunks |
| `src/analysis/canvas_1/check_canvas_compound.cpp` | 96 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/analysis/canvas_1/check_canvas_union.cpp` | 138 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/analysis/canvas_1/check_field_union.cpp` | 108 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/analysis/canvas_1/check_pc_dft.cpp` | 91 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/analysis/canvas_1/probe_canvas.cpp` | 196 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/context.hpp` | 187 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/context_realize.hpp` | 75 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/context_spec.hpp` | 191 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/field.hpp` | 127 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/midi_stream.hpp` | 179 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/musical_ops.hpp` | 192 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/pc_count.hpp` | 132 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/pc_dft.hpp` | 64 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/playhead.hpp` | 179 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/previous_event.hpp` | 194 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/spine.hpp` | 270 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/spine_ops.hpp` | 52 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/stream_data.hpp` | 489 | 0 | **truncated, declared** → symbol table in §(d); over the ~400-line threshold, so no raw hunks |
| `src/musical/vector_dressing.hpp` | 54 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/musical/wagon.hpp` | 164 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/sources/keyboard_midi.hpp` | 257 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/sources/midi_event.hpp` | 59 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/sources/midi_file.hpp` | 469 | 0 | **truncated, declared** → symbol table in §(d); over the ~400-line threshold, so no raw hunks |
| `src/sources/midi_port.hpp` | 213 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/sources/transport.hpp` | 110 | 0 | **FULL DIFF GIVEN** — Appendix D-1 (under the ~400-line threshold); symbols also in §(d) |
| `src/external/RtMidi.cpp` | 5275 | 0 | **truncated** — vendored third party (RtMidi), restored byte-identical at HEAD (blob `2303bb132273`) |
| `src/external/RtMidi.h` | 686 | 0 | **truncated** — vendored third party, restored byte-identical at HEAD (blob `2801037f628d`) |
| `src/external/imgui/**` (15 files: `imgui.cpp` 24360, `imgui_widgets.cpp` 11119, `imgui_demo.cpp` 11540, `imgui_draw.cpp` 6791, `imstb_truetype.h` 5085, `imgui_tables.cpp` 4665, `imgui.h` 4523, `imgui_internal.h` 4321, `backends/imgui_impl_glfw.cpp` 1727, `imstb_textedit.h` 1527, `backends/imgui_impl_wgpu.cpp` 1162, `imstb_rectpack.h` 627, `imconfig.h` 147, `backends/imgui_impl_wgpu.h` 121, `backends/imgui_impl_glfw.h` 73) | 77788 | 0 | **truncated** — vendored third party (Dear ImGui), **not restored**; a symbol inventory of ImGui is not evidence about this repo's arm |
| `src/external/implot/**` (4 files: `implot.cpp` 5935, `implot_items.cpp` 3515, `implot_internal.h` 1713, `implot.h` 1406) | 12569 | 0 | **truncated** — vendored third party (ImPlot), **not restored** |
| `src/the_lab.cpp` | 668 | 0 | **truncated** (exceeds ~400 lines); symbols below |

`src/the_lab.cpp` deleted top-level symbols (blob `3cd1a13fe66e` at `1a52f2db^`):
`#define LAB_ANALYSIS canvas_1`, `STRINGIFY`, `STRINGIFY2`, `ANALYSIS_HEADER`,
`namespace analysis_ns = t7::LAB_ANALYSIS`, `layout_count`, `struct ScrollingBuffer`
(`push`), `struct VectorHistory` (`push`, `unroll`), `struct TestCoupling` (`tick`),
`draw_stats_grid`, `draw_coupling_controls`, `draw_input_scope`,
`draw_trajectory_scope`, `draw_scalar`, `draw_vector`, `draw_vector_history`,
`struct StatScopes` (`struct ChannelGroup`, `tick`, `draw`), `draw_coupling_window`,
`main`. **This file has NOT been restored** — `git cat-file -e HEAD:src/the_lab.cpp`
fails; it is absent from the §(e) added-file list.

**BOUNDARY C — 74 qualifying paths.** An earlier draft printed 76; the script
yields **74**:

```
$ sh halfloss.sh 60a3e935 79adfa4d | wc -l
74
$ sh halfloss.sh 60a3e935 79adfa4d | grep -c '^assets/'
60
```

60 of the 74 are binary assets, for which `wc -l` counts newline bytes and is not a
line count; they are recorded as a class, not individually. With the corrected
painting count the class arithmetic closes exactly: 57 paintings + 2 entrance
posters + 1 mp3 = **60**. (The earlier draft's 58 paintings would have made 61,
inconsistent with its own — correct — "60 of them are binary assets".)

* `assets/paintings/PAINTING_*.{jpg,jpeg}` — **57** files, all deleted by
  **`89a4f929` (PRUNE_1 U5+U6 — the module, the assets, the family, the DTOs, the
  prose)**, which reaches mainline through the merge `e9c2ace3`. An earlier draft
  attributed the shelf to an unidentified WEB_SUNSET `W?` commit; the tree names one
  deleting commit and it is not a W-commit:

  ```
  $ git log --diff-filter=D --format='%h %s' 60a3e935..79adfa4d -- assets/paintings/
  89a4f929 PRUNE_1 U5+U6 — the module, the assets, the family, the DTOs, the prose
  ```

  `89a4f929` is inside the six-commit PRUNE_1 sequence `bc91cf3a..743dc9d0`
  established in §(b) (`git log --format='%h %s' bc91cf3a..743dc9d0` lists it), its
  parent is `d84a825401da…`, and `git merge-base --is-ancestor 89a4f929 79adfa4d`
  → true. So the paintings shelf leaves with the gallery prune, in the same
  sequence that deleted `gallery.hpp`, not with the web sunset.
* `assets/entrance/ENTRANCE_CONTROLS.jpg`, `assets/entrance/ENTRANCE_FIELD.jpeg` —
  2 files, deleted by `0f163d4c`. Confirmed:
  `git log --diff-filter=D --format='%h %s' 60a3e935..79adfa4d -- assets/entrance/`
  → `0f163d4c WEB_SUNSET W5d: assets/entrance — the page's two posters`.
* `assets/music/samsara.mp3` — 1 file, deleted by `1507b472`. Confirmed:
  `git log --diff-filter=D --format='%h %s' 60a3e935..79adfa4d -- assets/music/`
  → `1507b472 WEB_SUNSET W5c: assets/music — the soundtrack the shell played`.

These two attributions were correct as first written and are re-stated here with
their recipes. Both are genuine WEB_SUNSET W-commits; the paintings row above is
not.

**FLAG — binary line counts, now supplemented with a byte census.** For these 60
files "lines lost" is not a meaningful figure; `wc -l` on a JPEG/MP3 counts `0x0A`
bytes, and the census script's `pre`/`post` columns for them are therefore not line
counts. The ambiguity is recorded, not resolved by redefinition. To leave no
quantity missing, the same 60 paths are additionally measured in **bytes** with
`git cat-file -s '60a3e935:<path>'`, which is exact for binary blobs:

```
$ for p in $(sh halfloss.sh 60a3e935 79adfa4d | grep '^assets/' | cut -f1); do
    git cat-file -s "60a3e935:$p"; done | paste -sd+ | bc
13909864
```

| class | files | bytes at `60a3e935` | bytes at `79adfa4d` |
| --- | --- | --- | --- |
| `assets/paintings/PAINTING_*.{jpg,jpeg}` | 57 | 9,072,698 | 0 (all deleted) |
| `assets/entrance/ENTRANCE_CONTROLS.jpg` | 1 | 513,770 | 0 (deleted) |
| `assets/entrance/ENTRANCE_FIELD.jpeg` | 1 | 122,745 | 0 (deleted) |
| `assets/music/samsara.mp3` | 1 | 4,200,651 | 0 (deleted) |
| **total** | **60** | **13,909,864** | **0** |

9,072,698 + 513,770 + 122,745 + 4,200,651 = 13,909,864, and the file count closes at
60. All 60 are absent at `79adfa4d`, checked with `git cat-file -e '79adfa4d:<path>'`
over each path (no path survives). The byte figures are supplied as a supplement to
the class record, not as a substitute for the line-loss criterion the unit set.

The **14** text files that qualify at BOUNDARY C (an earlier draft said 16; the
script yields 74 − 60 = 14, and the table below has always held exactly 14
qualifying rows plus one non-qualifying row kept for completeness):

```
$ sh halfloss.sh 60a3e935 79adfa4d | grep -v '^assets/'
src/cartridges/the_board/bodies/gallery.hpp	pre=3291	post=0
src/core/sha256.hpp	pre=143	post=0
src/external/stb_image.cpp	pre=2	post=0
src/external/stb_image.h	pre=7988	post=0
tools/gates/console_gate/stubs/GLFW/emscripten_glfw3.h	pre=314	post=0
tools/gates/console_gate/stubs/emscripten.h	pre=32	post=0
tools/gates/console_gate/stubs/emscripten/em_types.h	pre=4	post=0
tools/gates/console_gate/stubs/emscripten/fetch.h	pre=57	post=0
tools/gates/console_gate/stubs/emscripten/html5.h	pre=38	post=0
tools/gates/sha256_gate/run.py	pre=152	post=0
tools/gates/shell_gate/run.py	pre=322	post=0
tools/web_dist.py	pre=907	post=0
web/index.html	pre=1215	post=0
web/organ_panel.js	pre=1414	post=0
```

The final table row — `0`, `0\``, `16)`, `17)` — is **not** among the 14: the
script's `a > 0` guard excludes zero-byte files, as the row itself says. It is
listed so the plan does not think those four paths went uncensused.

| file | pre @ `60a3e935` | post @ HEAD | blob @ `60a3e935` | disposition |
| --- | --- | --- | --- | --- |
| `src/cartridges/the_board/bodies/gallery.hpp` | 3291 | 0 (deleted) | `02d27583452b` | **truncated** (>400); symbols below |
| `src/external/stb_image.h` | 7988 | 0 (deleted) | `9eedabedc45b` | **truncated** — vendored third party (stb) |
| `src/external/stb_image.cpp` | 2 | 0 (deleted) | `9d6c8d58c4ad` | full content below (2 lines) |
| `src/core/sha256.hpp` | 143 | 0 (deleted) | `340c90569b62` | **truncated**; symbols below |
| `web/organ_panel.js` | 1414 | 0 (deleted) | `d26f594387ed` | **truncated** (>400) |
| `web/index.html` | 1215 | 0 (deleted) | `6b1725779aaa` | **truncated** (>400) |
| `tools/web_dist.py` | 907 | 0 (deleted) | `0d61ff86e17e` | **truncated** (>400); symbols below |
| `tools/gates/shell_gate/run.py` | 322 | 0 (deleted) | `080b81ae5cbe` | **truncated**; symbols below |
| `tools/gates/console_gate/stubs/GLFW/emscripten_glfw3.h` | 314 | 0 (deleted) | `12a17bc57f7c` | **truncated**; symbols below |
| `tools/gates/sha256_gate/run.py` | 152 | 0 (deleted) | `7626f095ee6b` | **truncated**; symbols below |
| `tools/gates/console_gate/stubs/emscripten/fetch.h` | 57 | 0 (deleted) | `36385378fdb3` | symbols below |
| `tools/gates/console_gate/stubs/emscripten/html5.h` | 38 | 0 (deleted) | `d96e47abacee` | symbols below |
| `tools/gates/console_gate/stubs/emscripten.h` | 32 | 0 (deleted) | `96158c60b545` | symbols below |
| `tools/gates/console_gate/stubs/emscripten/em_types.h` | 4 | 0 (deleted) | `68208ee5e6dd` | `typedef bool EM_BOOL;` |
| `0`, `0\``, `16)`, `17)` | 0 | 0 | `e69de29bb2d1` (empty blob) | zero-byte; excluded by the script's `a > 0` guard, listed here for completeness |

`src/external/stb_image.cpp` in full (blob `9d6c8d58c4ad`, 2 lines):

```c
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"
```

**FLAG — this last line was reconstructed from the file's own 2-line content via
`git cat-file blob 60a3e935:src/external/stb_image.cpp`; the grep-based symbol pass
surfaced only the `#define`. I re-read the blob directly to quote it whole.**

Deleted top-level symbols, remaining truncated files:

* **`src/cartridges/the_board/bodies/gallery.hpp`** (`02d27583452b`, 3291 lines) —
  `namespace t7::the_board`; `class Renderer` (fwd); `struct GalleryDeps`;
  `enum class ShotType`; `struct ShotTypeParams`; `struct PhotographerCaptureConfig`;
  `struct GalleryConfig`; `struct GallerySiteType`; `enum class GalleryPlaceResult`;
  `struct WallArtScaleBucket`; `struct WallArtConfig`; `struct GalleryProp`;
  `struct GalleryPaintingProp`; `struct WallArtProp`; `struct WallPaintingProp`;
  `struct PhotographerState` (`uniform`, `gaussian`, `sample_shot_count`,
  `sample_shot_type`); `struct SnapshotStagingRecord`; `struct AuthoredStagingRecord`;
  `struct PendingPromotion`; `struct GalleryCenter`; `struct PendingSnapshot`;
  `struct GalleryState` (`struct AuthoredFetchRequest`); `enum class IndoorSiteType`;
  free functions `update_photographer`, `render_snapshot_pass`,
  `select_gallery_for_patch`, `place_gallery_from_selection`, `commit_gallery`,
  `evict_paintings_for_patch`, `evict_gallery`, `dispatch_select_gallery`,
  `dispatch_place_gallery`, `dispatch_commit_gallery`, `place_wall_paintings`,
  `clear_wall_paintings`, `load_authored_textures`, `rotate_authored_staging`,
  `tick_gallery_deferred_hang`, `teardown_gallery`, `drain_gallery_promotions`,
  `find_free_exhibition_layer`, `queue_promotion`, `find_free_painting_slot`,
  `recompute_slot_high_water`, `capture_snapshot`, `authored_hangable`,
  `authored_release_layer`, `count_unused_authored`, `pick_authored_staging`,
  `struct FrameStyle`, `fill_slot_wall_frame`, `gallery_fan_radius`,
  `gallery_available_staging`, `authored_extract_number`.
  Note: this file was already reduced by `d84a8254` (PRUNE_1 U2, −327) before being
  deleted outright inside the same range.
* **`src/core/sha256.hpp`** (`340c90569b62`, 143 lines) — `namespace t7`;
  `namespace t7::sha256_detail` (`constexpr uint32_t K[64]`, `rotr`, `compress`);
  `inline std::string sha256_hex(std::string_view)`.
* **`tools/web_dist.py`** (`0d61ff86e17e`, 907 lines) — module constants `HERE`,
  `ROOT`, `WEB`, `DIST`, `SRC_PAINTINGS`, `SRC_ENTRANCE`, `SRC_MUSIC`,
  `DIST_PAINTINGS`, `DIST_MUSIC`, `SRC_PRESETS`, `DIST_PRESETS`, `ARTIFACTS`,
  `BUILD_ID_PLACEHOLDER`, `BUILD_ID_LEN`, `SHADER_SHA_PLACEHOLDER`, `SHADER_SHA_LEN`,
  `SHADER_SRC`, `PAINTING_CAP`, `PAINTING_QUALITY`, `MANIFEST_DEDUPE_CAP`,
  `PAINTING_EXTS`, `POSTERS`, `POSTER_WIDTH`, `POSTER_QUALITY`, `EXHIBITION_JSON`,
  `CF_LIMIT`, `GH_LIMIT`; functions `preset_files`, `mib`, `extract_number`, …
* **`tools/gates/shell_gate/run.py`** (`080b81ae5cbe`, 322 lines) — `HERE`, `ROOT`,
  `HEADER` (= `src/console/organ_registry.hpp`), `SHELL` (= `web/organ_panel.js`),
  `ORBS`; functions `read`, `strip_js`, `enum_values`, `keepalive_arity`,
  `js_array_len`, `main`.
* **`tools/gates/sha256_gate/run.py`** (`7626f095ee6b`, 152 lines) — `HERE`, `REPO`,
  `HEADER` (= `src/core/sha256.hpp`), `WORLD`, `DRIVER`, `VECTORS`; functions
  `shallow_note`, `main`.
* **`tools/gates/console_gate/stubs/GLFW/emscripten_glfw3.h`** (`12a17bc57f7c`, 314
  lines) — `#define EMSCRIPTEN_GLFW_EMSCRIPTEN_GLFW3_H`, `extern "C"` block,
  `#define GLFW_PLATFORM_EMSCRIPTEN 0x00060006`.
* **`tools/gates/console_gate/stubs/emscripten.h`** (`96158c60b545`, 32 lines) —
  `EM_ASM`, `EM_ASM_INT`, `EM_ASM_DOUBLE`, `EM_JS`, `EMSCRIPTEN_KEEPALIVE`,
  `EM_TIMING_SETTIMEOUT`, `EM_TIMING_RAF`, `EM_TIMING_SETIMMEDIATE`, `extern "C"` block.
* **`tools/gates/console_gate/stubs/emscripten/fetch.h`** (`36385378fdb3`, 57 lines) —
  `EMSCRIPTEN_FETCH_LOAD_TO_MEMORY`, `…_STREAM_DATA`, `…_PERSIST_FILE`, `…_APPEND`,
  `…_REPLACE`, `…_NO_DOWNLOAD`, `…_SYNCHRONOUS`, `…_WAITABLE`, `extern "C"` block.
* **`tools/gates/console_gate/stubs/emscripten/html5.h`** (`d96e47abacee`, 38 lines) —
  `EMSCRIPTEN_RESULT_SUCCESS`, `EMSCRIPTEN_EVENT_TARGET_DOCUMENT`,
  `EMSCRIPTEN_EVENT_TOUCHSTART/TOUCHEND/TOUCHMOVE/TOUCHCANCEL`, `extern "C"` block.
* **`web/organ_panel.js`** (`d26f594387ed`, 1414 lines) — **GAP NOW FILLED.** The
  earlier column-0 grep returned nothing because the whole file is a single IIFE:
  its first statement is `(function () {` at column 0 and everything else is indented
  inside it. `git cat-file blob 60a3e935:web/organ_panel.js | grep -cE '^(export |)(function|const|let|var|class|async function) '`
  → `0`, which is the fact, not an absence of structure. Re-censused with a
  name-extracting pass that does not depend on indentation:

  ```
  git cat-file blob 60a3e935:web/organ_panel.js \
    | grep -oE '\b(function [A-Za-z_$][A-Za-z0-9_$]*|(const|let|var) [A-Za-z_$][A-Za-z0-9_$]* *= *(async *)?(\(|function))' \
    | sed 's/ *= *.*//' | sort -u
  ```

  Module shape: one IIFE opening `(function () {` / `'use strict';`, gated on
  `var Q = new URLSearchParams(location.search); var WANT_PANEL = Q.get('organ') === '1';
  var WANT_PRESET = Q.get('preset'); if (!WANT_PANEL && !WANT_PRESET) return;`, with
  the kind constants `var F32 = 0, U32 = 1, BOOL = 2, VEC3 = 3, VEC4 = 4;`.
  Declared functions (33 `function` names): `applyFile`, `applyFilter`, `build`,
  `buildRow`, `clamp`, `collect`, `download`, `fanRegimes`, `finish`, `g4`, `hex`,
  `inHand`, `kinKey`, `lensAdmits`, `loadPreset`, `push`, `pushDef`, `readDef`,
  `refreshHost`, `refreshRegime`, `refreshRule`, `regimeOfGroup`, `ruleName`,
  `ruleOfGroup`, `same`, `setMinimized`, `setWidth`, `shelf`, `slug`, `stampTag`,
  `unhex`, `vis`, `wMax`. Function-valued `var` bindings: `BUILD_ID`, `add`, `addOpt`,
  `b`, `closeHead`, `commit`, `endDrag`, `isCol`, `lanes`, `line`, `lit`, `mood`,
  `on`, `px`, `q`, `s`, `setMode`, `sync`, `target`.
* **`web/index.html`** (`6b1725779aaa`, 1215 lines) — **GAP NOW FILLED.** The
  column-0 grep found no `<script`/`<style` because both are nested; censused with
  `git cat-file blob 60a3e935:web/index.html | grep -noE '<(script|style|canvas|body|head|title|div id="[^"]*"|meta name="[^"]*")[^>]*>'`.
  Document shape, in file order: `<head>`, three `<meta name=…>` rows (`viewport`,
  `color-scheme`, `theme-color`), `<title>`, one `<style>` block, `<body>`,
  `<div id="frame">` containing `<canvas id="canvas">`, `<div id="veil" class="layer">`,
  `<div id="status">`, `<div id="card" class="layer" hidden>`, then one `<script>`
  block. The full set of element ids (`grep -oE 'id="[A-Za-z0-9_-]+"' | sort -u`):
  `canvas`, `card`, `cardPoster`, `cardSay`, `cardSub`, `frame`, `log`, `log2`,
  `logToggle`, `logToggle2`, `music`, `reload`, `status`, `veil`. Functions declared
  in its inline script: `armResumeOnGesture`, `carriesActivation`, `classify`,
  `disarm`, `exists`, `fallback`, `floor`, `goFullscreen`, `lost`, `note`,
  `onEntryGesture`, `onLine`, `present`, `record`, `requestWakeLock`, `retry`, `say`,
  `showCard`, `showReady`, `sizeToViewport`, `startMusic`, `wireToggle`.

  Both files remain truncated as to raw content (1414 and 1215 lines, each over the
  ~400-line threshold); the blob SHAs above recover them exactly with
  `git show 60a3e935:web/organ_panel.js` and `git show 60a3e935:web/index.html`.

**Near-misses, recorded so the plan does not think they were skipped.** Two files
lost close to half their lines at BOUNDARY C and fall on the *keeping* side of the
`> half` test:

* `src/console/console.hpp` — 2826 → 1452 lines, ratio 0.514.
* `src/core/boot_params.hpp` — 171 → 94 lines, ratio 0.550.

Others checked and comfortably above half: `renderer.hpp` 2962 → 2484 (0.839),
`state.hpp` 5572 → 5145 (0.923), `world.wgsl` 15382 → 14698 (0.956),
`the_board.cpp` 517 → 404 (0.781), `instruments.hpp` 255 → 222 (0.871),
`cartridge.hpp` 3249 → 3169 (0.975).

---

### What the boundary evidence establishes

1. The tag `native-sunset` (`29cec46b`) marks the archival of the **native** twin —
   its own tag message reads *"Native twin archived; the web twin is the program
   (SUNSET_0)."* — and it points at the commit **before** the SUNSET_0 deletion
   commit `e476addd`, whose diff is five build/console/shader/docs files and touches
   nothing under `src/musical/`, `src/sources/`, `src/analysis/` or `src/coupling/`.
2. BOUNDARY A (`2bedb4e2..29cec46b`) deletes **zero** files anywhere in the tree, and
   its entire effect on `src/coupling/` is one added `#include <cstddef>` line and
   three appended trailing newlines.
3. The musical/MIDI arm was severed at `1a52f2db` (CUT_1c, 2026-08-05), three
   months of commits before either sunset tag: 49 files deleted, 1 added, 3 modified,
   +90 / −102426.
4. CUT_1c deleted 15 of 16 `src/musical/` headers, all 5 `src/sources/` headers,
   `src/analysis/analysis_cartridge.hpp`, all 5 `src/analysis/canvas_1/` files,
   `src/external/RtMidi.{cpp,h}`, **19** vendored ImGui/ImPlot files (15 ImGui +
   4 ImPlot), and `src/the_lab.cpp`. The 49 deleted paths partition exactly as
   27 arm files + 21 under `src/external/` + `src/the_lab.cpp`. It left behind `src/musical/signal_layout.hpp` and
   `src/analysis/analysis_signal.hpp`, and it added `src/analysis/beat_clock.hpp`.
5. CUT_1c touched **no** file under `src/coupling/`.
6. The severed hops are named in the diff: `render.bind_signal_layout(analysis.stat_layout())`
   became `render.bind_signal_layout(clock.stat_layout())`;
   `render.update(analysis.output(), …)` became `render.update(clock.output(), …)`;
   `AnalysisCartridge analysis; analysis.initialize("assets");` and `is_music_key`
   were deleted; `__WINDOWS_MM__` and `src/external/RtMidi.cpp` left the build.
7. `BeatClock::stat_layout()` returns `StatLayoutView{ nullptr, 0 }`, and
   `beat_clock.hpp`'s own banner enumerates the twelve source names that thereby stop
   resolving: `all.field`, `ch1.present_count`, `all.window_length`,
   `all.present_count`, `ch1.window_length`, `ch0.onset` .. `ch6.onset`.
8. Neither the SUNRISE_0 sequence (15 commits, `de4b8b6f..60a3e935`) nor the
   WEB_SUNSET W-sequence (24 commits, `60a3e935..cd36a385`) nor PRUNE_1 (6 commits,
   `bc91cf3a..743dc9d0`) touches any file under `src/musical/`, `src/sources/` or
   `src/analysis/`.
9. The restoration is two commits: `0c951b11` (29 files, +11037, −0 — the sources)
   and `e0e22e46` (`CMakeLists.txt` +20/−2 — `__WINDOWS_MM__` and `RtMidi.cpp` back
   in the build). Both merges that carry them (`e9c2ace3`, `3ac091f8`) produce the
   tree of their second parent exactly.
10. Every restored file — 15 `src/musical/` headers, 5 `src/sources/` headers,
    `analysis_cartridge.hpp`, 5 `canvas_1/` files, `RtMidi.cpp`, `RtMidi.h` — is
    **byte-identical** to its pre-CUT_1c blob. There are **no** DIVERGED files in the
    restored set.
11. The only DIVERGED files in the arm are ones CUT_1c never deleted:
    `src/musical/signal_layout.hpp` (PORT_4c added `misses()` / `misses_` and put the
    per-source stderr warning behind `#ifndef NDEBUG`), and the three `src/coupling/`
    headers, of which two diverge only by a trailing newline.
12. `src/coupling/visual_canvas.hpp` diverged substantially (101 insertions, 72
    deletions since `1a52f2db^`, 385 diff lines) — fifteen compile-time constants migrated to
    `canvas::CANVAS_LIVE`, the fog pipes turned into deviations from the anchor row
    (ATMOS_1), the hue table seated once (OIL_1 U5), and a PORT_4c miss-summary added —
    while **every `signal_layout_.resolve(...)` name in it is unchanged** across the
    whole arc.
13. `src/the_lab.cpp` (668 lines) and the 19 vendored ImGui/ImPlot files deleted by
    CUT_1c have **not** been restored; `src/external/stb_image.{h,cpp}`,
    `src/cartridges/the_board/bodies/gallery.hpp`, `src/core/sha256.hpp`, the two
    `web/` files, `tools/web_dist.py` and the two web gates were deleted across
    BOUNDARY C and are also absent at HEAD.
14. At HEAD the tree carries two distinct files named `organ_registry.hpp`:
    `src/console/organ_registry.hpp` (blob `70d09e9602eb`, the pre-W3e content,
    restored to that blob by HEAD itself) and `src/coupling/organ_registry.hpp`
    (blob `3047070e199d`, the post-W3e content, added by `72df32df`).
15. `HEAD:CMakeLists.txt` is blob `2dddc9202f4d…`, byte-identical to
    `e0e22e46:CMakeLists.txt`: the trailing newline `ebfac622` restored was removed
    again by `79adfa4d`.
16. `src/the_board.cpp` at HEAD still instantiates `t7::BeatClock` and binds
    `clock.stat_layout()`; no file outside `src/analysis/` includes
    `analysis/canvas_1/canvas.hpp`.

---

### Appendix D-1 — full diffs of the 24 under-threshold arm files deleted by CUT_1c

This appendix discharges the §(d) obligation the earlier draft truncated. The unit
asks for the full diff of every file under `src/coupling/`, `src/musical/`,
`src/sources/` and `src/analysis/` that CUT_1c touched, with the ~400-line
truncation licence applied **per file**. `src/coupling/`: none touched.
`src/analysis/beat_clock.hpp` is an *add*, not a delete, and is quoted in §(d)
Hop 5. That leaves 27 deleted files, of which 24 are under the threshold; all 24
appear below, unabridged.

Recipe, run once per path, with no filtering, no elision and no reflow:

```
git diff '1a52f2db^..1a52f2db' -- <path>
```

Every hunk header and every context line is as git printed it. The three files held
back — `src/analysis/canvas_1/canvas.hpp` (686 deleted lines),
`src/musical/stream_data.hpp` (489) and `src/sources/midi_file.hpp` (469) — are over
the threshold; their `--stat` and symbol inventories are in §(d), and their exact
text is `git show 1a52f2db^:<path>`.

Order below is the order of
`git diff --diff-filter=D --name-only '1a52f2db^..1a52f2db' -- src/analysis/ src/musical/ src/sources/`,
with the three over-threshold paths skipped. Each block is preceded by its path,
its blob SHA at `1a52f2db^` (R6), and its deleted-line count from
`git diff --numstat`.


#### D-1.1 — `src/analysis/analysis_cartridge.hpp`

Blob at `1a52f2db^`: `f07e11a992d2381413b303f6741ddf32fe8205ab` · deleted lines: 103 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/analysis/analysis_cartridge.hpp b/src/analysis/analysis_cartridge.hpp
deleted file mode 100644
index f07e11a9..00000000
--- a/src/analysis/analysis_cartridge.hpp
+++ /dev/null
@@ -1,103 +0,0 @@
-#pragma once
-
-/**
- * ANALYSIS CARTRIDGE - Interface for Musical Analysis Modules
- * ===========================================================
- * 
- * An analysis cartridge transforms MIDI events into musical statistics.
- * It owns the musical interpretation: which channels to listen to,
- * what analyzers to run, what stats to compute.
- * 
- * THE CATEGORICAL BOUNDARY
- * ------------------------
- * 
- * The analysis cartridge knows about music. It knows what polyphony means,
- * what a note onset is, how to compute pitch centroid.
- * 
- * It does NOT know about visualization. It has no concept of terrain,
- * cameras, or vertices. It just produces numbers in slots.
- * 
- * LIFECYCLE
- * ---------
- * 
- *   1. Console creates cartridge
- *   2. Console calls initialize()
- *   3. Per frame:
- *      a. Console routes input events via on_input()
- *      b. Console calls update(dt)
- *      c. Console reads output() and passes to render cartridges
- * 
- * WHAT THE CARTRIDGE OWNS
- * -----------------------
- * 
- *   - Clock (converts dt to beats)
- *   - MidiStream(s)
- *   - Sources (MidiFile, KeyboardMidi) that feed streams
- *   - Analyzers (Playhead, Wagon)
- *   - Train (stat computation)
- *   - The mapping of stats to slots
- */
-
-#include "analysis/analysis_signal.hpp"
-#include "core/input_event.hpp"
-
-namespace t7 {
-
-// =============================================================================
-// ANALYSIS CARTRIDGE INTERFACE
-// =============================================================================
-
-class AnalysisCartridge {
-public:
-    virtual ~AnalysisCartridge() = default;
-    
-    // ─── LIFECYCLE ──────────────────────────────────────────────────────────
-    
-    /**
-     * Initialize the cartridge.
-     * Called once after construction.
-     * 
-     * @param asset_path  Path to assets folder (for loading MIDI files, etc.)
-     */
-    virtual void initialize(const char* asset_path) = 0;
-    
-    /**
-     * Update the cartridge.
-     * Called once per frame.
-     * 
-     * @param dt  Delta time in seconds (from console's wall clock)
-     */
-    virtual void update(float dt) = 0;
-    
-    // ─── INPUT ──────────────────────────────────────────────────────────────
-    
-    /**
-     * Handle an input event.
-     * The cartridge decides which events it cares about (e.g., musical keys).
-     * 
-     * @param event  The input event
-     */
-    virtual void on_input(const InputEvent& event) = 0;
-    
-    // ─── OUTPUT ─────────────────────────────────────────────────────────────
-    
-    /**
-     * Get the current analysis output.
-     * Valid after update() has been called.
-     *
-     * @return  Reference to the analysis signal (time + stats)
-     */
-    virtual const AnalysisSignal& output() const = 0;
-
-    /**
-     * Publish this cartridge's stat layout — the slot map that names which
-     * (channel, slot_base, count) each stat group occupies. The render side
-     * receives this once and resolves coupling sources by name, without ever
-     * including the analysis cartridge's own headers.
-     *
-     * @return  Non-owning view over the cartridge's static STAT_LAYOUT.
-     */
-    virtual StatLayoutView stat_layout() const = 0;
-};
-
-} // namespace t7
```

#### D-1.2 — `src/analysis/canvas_1/check_canvas_compound.cpp`

Blob at `1a52f2db^`: `071ba66d5d5ec87c9672a629842052ba6efc4909` · deleted lines: 96 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/analysis/canvas_1/check_canvas_compound.cpp b/src/analysis/canvas_1/check_canvas_compound.cpp
deleted file mode 100644
index 071ba66d..00000000
--- a/src/analysis/canvas_1/check_canvas_compound.cpp
+++ /dev/null
@@ -1,96 +0,0 @@
-// check_canvas_compound.cpp — verify the per-voice readings and their compounds.
-// Three voices, spine on. Each voice publishes current_pc / window_length; the
-// union publishes compound current_pc and window_length as VECTOR SUMS (compound
-// current_pc = a per-pc voice count; compound window_length = summed length), and
-// the compound field. The test drives notes on the three voices and asserts the
-// compound equals the element-wise sum of the per-voice vectors, and that the
-// compounds land in the group band at their canonical slots. Frame driven
-// directly (configure / publish_reading / route / advance), the port dormant.
-
-#include "canvas.hpp"
-#include "sources/midi_event.hpp"
-
-#include <cstdio>
-#include <cassert>
-#include <cmath>
-#include <string_view>
-
-using namespace t7;
-using namespace t7::canvas_1;
-
-static const StatGroup* find(const Canvas& cv, const char* name) {
-    StatLayoutView lay = cv.stat_layout();
-    for (uint32_t g = 0; g < lay.count; ++g)
-        if (std::string_view(lay.groups[g].name) == name) return &lay.groups[g];
-    return nullptr;
-}
-static float bin(const Canvas& cv, const char* name, int i) {
-    const StatGroup* g = find(cv, name);
-    return g ? cv.output().stat(g->channel, g->slot_base + i) : -1.0f;
-}
-static bool near(float a, float b) { return std::fabs(a - b) < 1e-4f; }
-
-// Three voices (0,1,2), spine on; per-voice current_pc + window_length, and the
-// compound current_pc / window_length / field over their union.
-static void setup(Canvas& cv) {
-    for (int v = 0; v < 3; ++v) {
-        ContextSpec s = default_spec(/*midi*/ v, /*window*/ 4.0f);
-        s.crossings.active = true;          // spine on — current_pc needs it
-        cv.configure(v, s);
-    }
-    cv.publish_reading(Canvas::Reading::CurrentPC,    Canvas::Source::channel(0), "ch0.current_pc");
-    cv.publish_reading(Canvas::Reading::WindowLength, Canvas::Source::channel(0), "ch0.window_length");
-    cv.publish_reading(Canvas::Reading::CurrentPC,    Canvas::Source::channel(2), "ch2.current_pc");
-
-    const Canvas::Source all = Canvas::Source::group({0, 1, 2});
-    cv.publish_reading(Canvas::Reading::Field,        all, "all.field");
-    cv.publish_reading(Canvas::Reading::CurrentPC,    all, "all.current_pc");
-    cv.publish_reading(Canvas::Reading::WindowLength, all, "all.window_length");
-}
-
-int main() {
-    Canvas cv; setup(cv);
-
-    // Voices 0 and 1 both sound D; voice 2 sounds F#. Held one beat (onset @1,
-    // read @2). Degrees from D: D -> 0, F# -> 4.
-    cv.route(MidiEvent::note_on(0, 62, 0.8f, 1.0f));   // ch0 D
-    cv.route(MidiEvent::note_on(1, 62, 0.8f, 1.0f));   // ch1 D
-    cv.route(MidiEvent::note_on(2, 66, 0.8f, 1.0f));   // ch2 F#
-    cv.advance(2.0f);
-
-    // ── Per-voice current_pc: a one-hot at each voice's line note ───────────
-    std::printf("per-voice current_pc: ch0[D]=%g  ch2[F#]=%g\n",
-                bin(cv, "ch0.current_pc", 0), bin(cv, "ch2.current_pc", 4));
-    assert(bin(cv, "ch0.current_pc", 0) == 1.0f);   // ch0 on D
-    assert(bin(cv, "ch2.current_pc", 4) == 1.0f);   // ch2 on F#
-
-    // ── Compound current_pc = vector sum (a per-pc voice count) ─────────────
-    std::printf("compound current_pc: [D]=%g (expect 2)  [F#]=%g (expect 1)\n",
-                bin(cv, "all.current_pc", 0), bin(cv, "all.current_pc", 4));
-    assert(bin(cv, "all.current_pc", 0) == 2.0f);   // D held by voices 0 and 1
-    assert(bin(cv, "all.current_pc", 4) == 1.0f);   // F# by voice 2
-
-    // ── window_length: per-voice ~1 beat; compound = summed length ──────────
-    assert(near(bin(cv, "ch0.window_length", 0), 1.0f));        // ch0 D: one beat
-    std::printf("compound window_length: [D]=%g (expect 2)  [F#]=%g (expect 1)\n",
-                bin(cv, "all.window_length", 0), bin(cv, "all.window_length", 4));
-    assert(near(bin(cv, "all.window_length", 0), 2.0f));        // D: 1 beat x 2 voices
-    assert(near(bin(cv, "all.window_length", 4), 1.0f));        // F#: 1 beat
-
-    // ── Placement: compounds in the group band; voices in their own ─────────
-    const StatGroup* gc = find(cv, "all.current_pc");
-    assert(gc && gc->channel == MAX_CHANNELS - 1 && gc->slot_base == 48);
-    const StatGroup* gf = find(cv, "all.field");
-    assert(gf && gf->channel == MAX_CHANNELS - 1 && gf->slot_base == 61);
-    const StatGroup* g0 = find(cv, "ch0.current_pc");
-    assert(g0 && g0->channel == 0 && g0->slot_base == 48);
-
-    // ── Availability still bites: a line reading needs the spine ────────────
-    Canvas bare;
-    bare.configure(0, default_spec(/*midi*/ 0, /*window*/ 4.0f));   // spine OFF
-    assert(!bare.publish_reading(Canvas::Reading::CurrentPC, Canvas::Source::channel(0), "ch0.current_pc"));
-
-    std::printf("\nOK -- per-voice readings publish; the additive compounds are the element-wise\n");
-    std::printf("      sum of the voices; compounds sit in the group band; spine gates the line.\n");
-    return 0;
-}
```

#### D-1.3 — `src/analysis/canvas_1/check_canvas_union.cpp`

Blob at `1a52f2db^`: `e8be1173416c8e56081ddabba5b8e4ed2cce288f` · deleted lines: 138 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/analysis/canvas_1/check_canvas_union.cpp b/src/analysis/canvas_1/check_canvas_union.cpp
deleted file mode 100644
index e8be1173..00000000
--- a/src/analysis/canvas_1/check_canvas_union.cpp
+++ /dev/null
@@ -1,138 +0,0 @@
-// check_canvas_union.cpp — verify the union field: the one published reading of
-// the first instance. Two voices are subscribed (slot 0 ← MIDI 0, slot 1 ←
-// MIDI 1); the field is published once, source the union {0, 1}, in the
-// reserved group band under the bare name `field`. The test drives notes on
-// both channels and asserts the field is elected from the COMBINED present-set
-// — a note on one voice moving the field the other established — that it lands
-// in the group band at the field's canonical slot, and that no per-channel
-// reading is published. The frame is driven directly (configure /
-// publish_reading / route / advance), the port dormant.
-
-#include "canvas.hpp"
-#include "sources/midi_event.hpp"
-
-#include <cstdio>
-#include <cassert>
-#include <string_view>
-
-using namespace t7;
-using namespace t7::canvas_1;   // the Canvas now lives in the cartridge's namespace
-
-// Subscribe two voices and publish the union field — the first instance,
-// composed without opening the port (the test's standing-in for initialize()).
-static void setup(Canvas& cv) {
-    cv.configure(0, default_spec(/*midi*/ 0, /*window*/ 4.0f));
-    cv.configure(1, default_spec(/*midi*/ 1, /*window*/ 4.0f));
-    const bool ok = cv.publish_reading(Canvas::Reading::Field,
-                                       Canvas::Source::group({0, 1}), "field");
-    assert(ok);
-    (void)ok;
-}
-
-static const StatGroup* find(const Canvas& cv, const char* name) {
-    StatLayoutView lay = cv.stat_layout();
-    for (uint32_t g = 0; g < lay.count; ++g)
-        if (std::string_view(lay.groups[g].name) == name) return &lay.groups[g];
-    return nullptr;
-}
-
-static int field_of(const Canvas& cv) {   // the published group `field`, as an int
-    const StatGroup* g = find(cv, "field");
-    return g ? static_cast<int>(cv.output().stat(g->channel, g->slot_base)) : -999;
-}
-
-int main() {
-    // 1. Silence: no field has scored, so the index is the top of the hierarchy.
-    {
-        Canvas cv; setup(cv);
-        cv.advance(0.5f);
-        std::printf("silence                       -> field %d   (expect 1, top of hierarchy)\n", field_of(cv));
-        assert(field_of(cv) == 1);
-    }
-
-    // 2. The union elects across both voices, and a note on one voice moves the
-    //    field the other established.
-    //
-    //    Voice 0 sounds D E F#  (degrees {0,2,4} from D): Mixolydian and Major
-    //    tie at the top, the hierarchy keeps Mixolydian (rank 2). Then voice 1
-    //    adds C# (degree 11): the COMBINED set {0,2,4,11} makes Major (rank 3)
-    //    strictly out-score Mixolydian, so the held field moves to Major. The
-    //    move is driven by a note on a channel the field did not start on —
-    //    proof the field reads the union, not either voice alone.
-    {
-        Canvas cv; setup(cv);
-
-        cv.route(MidiEvent::note_on(0, 62, 0.8f, 1.0f));   // D  on voice 0
-        cv.route(MidiEvent::note_on(0, 64, 0.8f, 1.0f));   // E  on voice 0
-        cv.route(MidiEvent::note_on(0, 66, 0.8f, 1.0f));   // F# on voice 0
-        cv.advance(1.5f);
-        std::printf("voice0: D E F#                -> field %d   (expect 2, Mixolydian)\n", field_of(cv));
-        assert(field_of(cv) == 2);
-
-        cv.route(MidiEvent::note_on(1, 61, 0.8f, 2.0f));   // C# on voice 1
-        cv.advance(2.5f);
-        std::printf("voice1 adds C#  (union)       -> field %d   (expect 3, Major — moved by voice 1)\n", field_of(cv));
-        assert(field_of(cv) == 3);
-
-        // Persistence: release both voices and age past the window; the held
-        // field holds through the silence that follows.
-        cv.route(MidiEvent::note_off(0, 62, 3.0f));
-        cv.route(MidiEvent::note_off(0, 64, 3.0f));
-        cv.route(MidiEvent::note_off(0, 66, 3.0f));
-        cv.route(MidiEvent::note_off(1, 61, 3.0f));
-        cv.advance(12.0f);
-        std::printf("then silence (aged)           -> field %d   (expect 3, held)\n", field_of(cv));
-        assert(field_of(cv) == 3);
-    }
-
-    // 3. Placement and opt-in: the field is published once, in the reserved
-    //    group band (the last index) at the field's canonical slot, under the
-    //    bare name `field` — and nothing per-channel is published.
-    {
-        Canvas cv; setup(cv);
-        cv.advance(0.5f);
-
-        const StatGroup* g = find(cv, "field");
-        assert(g != nullptr);
-        assert(g->channel   == MAX_CHANNELS - 1);   // the reserved group band
-        assert(g->slot_base == 61);                 // the field's canonical slot
-        assert(g->count     == 1);
-        assert(g->shape     == StatShape::Scalar);
-
-        assert(find(cv, "ch0.field")         == nullptr);
-        assert(find(cv, "ch1.field")         == nullptr);
-        assert(find(cv, "ch0.present_count") == nullptr);
-        assert(cv.stat_layout().count == 1);        // the union field is the sole tenant
-        std::printf("placement: `field` at band %d slot %d, sole published reading\n",
-                    g->channel, g->slot_base);
-    }
-
-    // 4. Availability-binding: a line reading needs the spine, which the spec
-    //    leaves off, so the declaration is refused and the contract is unchanged.
-    {
-        Canvas cv; setup(cv);
-        const bool refused = !cv.publish_reading(Canvas::Reading::CurrentPC,
-                                                 Canvas::Source::channel(0), "ch0.current_pc");
-        assert(refused);
-        (void)refused;
-        assert(cv.stat_layout().count == 1);        // nothing was added
-        std::printf("availability: line reading refused (spine off), contract unchanged\n");
-    }
-
-    // 5. Write-gating: a reading whose value-writer is not yet wired is refused
-    //    even when the analysis could feed it — so the layout never advertises a
-    //    slot that would stay zero. present_count is available (the present is
-    //    always there) but unwired this round, so its declaration is refused.
-    {
-        Canvas cv; setup(cv);
-        const bool refused = !cv.publish_reading(Canvas::Reading::PresentCount,
-                                                 Canvas::Source::channel(0), "ch0.present_count");
-        assert(refused);
-        (void)refused;
-        assert(cv.stat_layout().count == 1);        // nothing was added
-        std::printf("write-gating: available-but-unwired reading refused, contract unchanged\n");
-    }
-
-    std::printf("\nOK -- the union field publishes: one reading across both voices, in the group band, opt-in and availability-bound.\n");
-    return 0;
-}
```

#### D-1.4 — `src/analysis/canvas_1/check_field_union.cpp`

Blob at `1a52f2db^`: `5cbe0a21bf8d94e9643ae280fc2e9c7c344706b3` · deleted lines: 108 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/analysis/canvas_1/check_field_union.cpp b/src/analysis/canvas_1/check_field_union.cpp
deleted file mode 100644
index 5cbe0a21..00000000
--- a/src/analysis/canvas_1/check_field_union.cpp
+++ /dev/null
@@ -1,108 +0,0 @@
-// check_field_union.cpp — prove the cross-channel presence union in isolation,
-// before any canvas wiring leans on it. Two claims:
-//
-//   1. The op is a presence join. The union of several per-channel present-sets
-//      marks a class iff it sounds in ANY of them; binary in, binary out; the
-//      empty set is the identity, and a set joined with itself is unchanged.
-//
-//   2. It feeds the field unchanged, and that is the point. A bare D-major triad
-//      on one channel is ambiguous; two foreign tones on another are ambiguous;
-//      their union resolves to ONE field, clear and untied -- read by the same
-//      machinery that reads a single voice, with nothing in it altered.
-
-#include "musical/field.hpp"
-
-#include <cstdio>
-#include <cassert>
-#include <initializer_list>
-
-using namespace t7;
-
-static constexpr int D = 2;   // PROJECT_PC_ORIGIN: the home note
-
-// The canvas's bank, inline, so the election here matches the live one.
-static const Field BANK[6] = {
-    { "phrygian_dominant", field_mask({0, 1, 4, 5, 7, 8, 10}) },
-    { "mixolydian",        field_mask({0, 2, 4, 5, 7, 9, 10}) },
-    { "major",             field_mask({0, 2, 4, 5, 7, 9, 11}) },
-    { "dorian",            field_mask({0, 2, 3, 5, 7, 9, 10}) },
-    { "harmonic_minor",    field_mask({0, 2, 3, 5, 7, 8, 11}) },
-    { "lydian_sharp2",     field_mask({0, 3, 4, 6, 7, 9, 11}) },
-};
-
-static PitchClassVector pcs(std::initializer_list<int> classes) {
-    PitchClassVector s;
-    for (int c : classes) s[((c % 12) + 12) % 12] = 1.0f;
-    return s;
-}
-
-static void print_set(const char* label, const PitchClassVector& s) {
-    std::printf("  %-12s { ", label);
-    for (int i = 0; i < 12; ++i) if (s.v[i] > 0.0f) std::printf("%d ", i);
-    std::printf("}\n");
-}
-
-static void expect_set(const PitchClassVector& s, std::initializer_list<int> classes) {
-    PitchClassVector want = pcs(classes);
-    for (int i = 0; i < 12; ++i) assert(s.v[i] == want.v[i]);
-}
-
-int main() {
-    // 1. The op is a presence join. -------------------------------------------
-    {
-        PitchClassVector a = pcs({2, 6});            // ch0: D, F#
-        PitchClassVector b = pcs({9, 0});            // ch1: A, C
-        PitchClassVector sets[2] = {a, b};
-        PitchClassVector u = present_union(sets, 2);
-        print_set("ch0", a);
-        print_set("ch1", b);
-        print_set("union", u);
-        expect_set(u, {0, 2, 6, 9});                 // sounding in either
-        std::printf("  -> union is the presence-OR of the two\n\n");
-    }
-
-    // identity and idempotence: the join laws.
-    {
-        PitchClassVector a = pcs({2, 6, 9});
-        PitchClassVector empty;
-        PitchClassVector with_empty[2] = {a, empty};
-        PitchClassVector with_self[2]  = {a, a};
-        expect_set(present_union(with_empty, 2), {2, 6, 9});   // a + 0 = a
-        expect_set(present_union(with_self, 2),  {2, 6, 9});   // a + a = a
-        std::printf("  identity (a + empty = a) and idempotence (a + a = a) hold\n\n");
-    }
-
-    // 2. The union feeds the field, and decides what neither voice does. ------
-    {
-        PitchClassVector ch0 = pcs({2, 6, 9});       // D major triad: D F# A
-        PitchClassVector ch1 = pcs({0, 4});          // C, E -- the b7 and the 9
-
-        FieldChoice e0 = elect_field(to_degrees(ch0, D), BANK, 6);
-        FieldChoice e1 = elect_field(to_degrees(ch1, D), BANK, 6);
-        std::printf("  ch0 (D F# A)   -> %-16s overlap %.0f, tie %d   ambiguous\n",
-                    BANK[e0.index].name, e0.overlap, e0.tie);
-        std::printf("  ch1 (C E)      -> %-16s overlap %.0f, tie %d   ambiguous\n",
-                    BANK[e1.index].name, e1.overlap, e1.tie);
-        assert(e0.tie > 1);
-        assert(e1.tie > 1);
-
-        PitchClassVector sets[2] = {ch0, ch1};
-        PitchClassVector u = present_union(sets, 2);
-        FieldChoice eu = elect_field(to_degrees(u, D), BANK, 6);
-        print_set("union", u);
-        std::printf("  union          -> %-16s overlap %.0f, tie %d   clear\n",
-                    BANK[eu.index].name, eu.overlap, eu.tie);
-        expect_set(u, {0, 2, 4, 6, 9});              // D E F# A C
-        assert(eu.index == 1);                       // mixolydian
-        assert(eu.tie == 1);                         // untied
-
-        HeldField hf;                                // and the held field takes it
-        int idx = hf.step(to_degrees(u, D), BANK, 6);
-        assert(idx == 1);
-        std::printf("  -> held field bootstraps to %s\n", BANK[idx].name);
-    }
-
-    std::printf("\nOK -- the union is a presence join, and feeds the field exactly as one voice does;\n");
-    std::printf("      the combination resolves to one field what each voice alone leaves open.\n");
-    return 0;
-}
```

#### D-1.5 — `src/analysis/canvas_1/check_pc_dft.cpp`

Blob at `1a52f2db^`: `f35a0927631c63df4685201f8cd9049315c5957d` · deleted lines: 91 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/analysis/canvas_1/check_pc_dft.cpp b/src/analysis/canvas_1/check_pc_dft.cpp
deleted file mode 100644
index f35a0927..00000000
--- a/src/analysis/canvas_1/check_pc_dft.cpp
+++ /dev/null
@@ -1,91 +0,0 @@
-// ─── check_pc_dft — the pure-fn gate for the pc-DFT capability ────
-// Known vectors → known families (CORRECTED against the math at the
-// gate's first run: the major triad peaks f3 — |X3| = √5, the
-// triadicity coefficient — NOT f5 as the handoff sketched; f5's
-// signature set is the DIATONIC SCALE, |X5| ≈ 3.73 with every other
-// family ≤ 1): a whole-tone cluster puts all energy in f6; a single
-// pc → all mags equal (1.0); the zero vector rests at mags 0 /
-// phases 0; mags are transposition-invariant; phases stay in [−π,π].
-// No canvas, no port, no RtMidi — pc_dft.hpp + musical_ops.hpp alone.
-
-#include <cassert>
-#include <cmath>
-#include <cstdio>
-#include "musical/pc_dft.hpp"
-
-using t7::PitchClassVector;
-using t7::PcDft;
-using t7::pc_dft;
-
-static bool near(float a, float b, float eps = 1e-5f) { return std::fabs(a - b) < eps; }
-
-int main() {
-    // 1. Single pc → every family's mag is exactly 1 (|X_k| = L1 = 1).
-    {
-        PitchClassVector v; v[0] = 1.0f;
-        const PcDft d = pc_dft(v);
-        for (int k = 0; k < 6; ++k) assert(near(d.mag[k], 1.0f));
-    }
-
-    // 2. Whole-tone cluster {0,2,4,6,8,10} → all energy in f6, none elsewhere.
-    {
-        PitchClassVector v;
-        for (int pc = 0; pc < 12; pc += 2) v[pc] = 1.0f;
-        const PcDft d = pc_dft(v);
-        assert(near(d.mag[5], 1.0f));
-        for (int k = 0; k < 5; ++k) assert(near(d.mag[k], 0.0f));
-    }
-
-    // 3a. C major triad {0,4,7} → argmax is f3 (index 2): |X3| = √5 ≈ 2.236,
-    //     the triadicity coefficient (f5 runs second at ≈ 1.932).
-    {
-        PitchClassVector v; v[0] = 1.0f; v[4] = 1.0f; v[7] = 1.0f;
-        const PcDft d = pc_dft(v);
-        int argmax = 0;
-        for (int k = 1; k < 6; ++k) if (d.mag[k] > d.mag[argmax]) argmax = k;
-        assert(argmax == 2);
-        assert(near(d.mag[2], std::sqrt(5.0f) / 3.0f, 1e-4f));
-    }
-
-    // 3b. The DIATONIC SCALE {0,2,4,5,7,9,11} → argmax is f5 (index 4):
-    //     |X5| ≈ 3.732 while every other family sits at ≤ 1 — the fifths
-    //     family's signature set.
-    {
-        PitchClassVector v;
-        for (int pc : {0, 2, 4, 5, 7, 9, 11}) v[pc] = 1.0f;
-        const PcDft d = pc_dft(v);
-        int argmax = 0;
-        for (int k = 1; k < 6; ++k) if (d.mag[k] > d.mag[argmax]) argmax = k;
-        assert(argmax == 4);
-        for (int k = 0; k < 6; ++k) if (k != 4) assert(d.mag[k] < d.mag[4] * 0.5f);
-    }
-
-    // 4. Zero vector → the declared REST of the pure half: mags 0, phases 0.
-    {
-        PitchClassVector v;
-        const PcDft d = pc_dft(v);
-        for (int k = 0; k < 6; ++k) { assert(near(d.mag[k], 0.0f)); assert(near(d.phase[k], 0.0f)); }
-    }
-
-    // 5. Transposition invariance of mags (rotation only moves phase).
-    {
-        PitchClassVector a; a[0] = 1.0f; a[4] = 1.0f; a[7] = 1.0f;
-        PitchClassVector b; b[3] = 1.0f; b[7] = 1.0f; b[10] = 1.0f;   // +3 semitones
-        const PcDft da = pc_dft(a), db = pc_dft(b);
-        for (int k = 0; k < 6; ++k) assert(near(da.mag[k], db.mag[k]));
-    }
-
-    // 6. Phase range [−π,π] over a spread of vectors.
-    {
-        for (int seed = 0; seed < 12; ++seed) {
-            PitchClassVector v; v[seed] = 1.0f; v[(seed * 5 + 3) % 12] = 2.0f;
-            const PcDft d = pc_dft(v);
-            for (int k = 0; k < 6; ++k) assert(d.phase[k] >= -3.14159266f && d.phase[k] <= 3.14159266f);
-        }
-    }
-
-    std::printf("check_pc_dft: GREEN — triad peaks f3, diatonic scale peaks f5, "
-                "whole-tone peaks f6, single pc uniform, zero rests, "
-                "mags transposition-invariant\n");
-    return 0;
-}
```

#### D-1.6 — `src/analysis/canvas_1/probe_canvas.cpp`

Blob at `1a52f2db^`: `7a341e9c00a0b8bd1965714df87c533d30a6cced` · deleted lines: 196 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/analysis/canvas_1/probe_canvas.cpp b/src/analysis/canvas_1/probe_canvas.cpp
deleted file mode 100644
index 7a341e9c..00000000
--- a/src/analysis/canvas_1/probe_canvas.cpp
+++ /dev/null
@@ -1,196 +0,0 @@
-// probe_canvas.cpp ──────────────────────────────────────────────────
-//
-// DAW-synced lab for canvas_1 — run as the cartridge it now is. It constructs
-// the canvas, calls initialize() (which composes its two voices and opens
-// loopMIDI), drives it the way the_lab does (update() each frame), and reads the
-// published signal back through stat_layout().
-//
-// canvas_1's composition: two voices — slot 0 <- MIDI 0, slot 1 <- MIDI 1 —
-// each a present and a four-beat window, the spine off. It publishes ONE
-// reading: the field, taken across the UNION of both voices, in the group band.
-//
-// At startup it prints the binding of every active slot and the published
-// layout. Then, on each note transition (seen from any voice's playhead) it
-// prints three things in step: what the canvas speaks (the published field);
-// the present notes of BOTH voices as ground truth; and the combined present-
-// and-window pitch-class set the field elects from — so the printed field rank
-// is explained by the notes that produced it. Whatever the layout advertises is
-// printed, so a reading added later appears here without changing this probe.
-//
-// The canvas owns its port and reads the beat from it, so this harness passes
-// wall-clock dt to update() (the signal's t_seconds telemetry) and reads the
-// musical beat back from the published signal; it routes no events of its own.
-//
-// Needs RtMidi and the transport-aware MidiPort. In Ableton, enable loopMIDI's
-// Clock/Sync output, then play.  (Ctrl-C to stop)
-//   ./probe_canvas
-
-#include "canvas.hpp"
-#include "sources/midi_event.hpp"
-
-#include <chrono>
-#include <cstdio>
-#include <iostream>
-#include <string>
-#include <thread>
-
-using namespace t7;
-using namespace t7::canvas_1;   // the Canvas now lives in the cartridge's namespace
-
-static std::string note_name(int midi) {
-    static const char* n[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
-    return std::string(n[((midi % 12) + 12) % 12]) + std::to_string(midi / 12 - 1);
-}
-
-static const char* yn(bool b)    { return b ? "yes" : "no"; }
-static const char* onoff(bool b) { return b ? "on"  : "off"; }
-static const char* oracle_name(OracleBinding o) {
-    switch (o) { case OracleBinding::Lowest: return "lowest"; }
-    return "lowest";
-}
-
-// The static binding for one slot, printed once before the flow starts.
-static void print_binding(const Canvas& canvas, int slot) {
-    if (!canvas.active(slot)) return;
-    const ContextSpec& s = canvas.spec(slot);
-    const Context&     c = canvas.context(slot);
-    std::printf("  slot %d  <-  midi ch %d   present %s   windows %d   event %s   spine %s",
-                slot, s.channel, yn(s.has_present()), s.window_count(),
-                onoff(s.has_event()), onoff(s.has_crossings()));
-    if (s.has_crossings())
-        std::printf(" (tol %.3f oracle %s)", s.crossings.tolerance_beats, oracle_name(s.crossings.oracle));
-    std::printf("   [ctx ch %d, wagons %d]\n", c.channel(), c.wagon_count());
-}
-
-// The published layout: the named slot map the canvas advertises. A render
-// side receives exactly this and resolves its sources against it.
-static void print_layout(const Canvas& canvas) {
-    StatLayoutView lay = canvas.stat_layout();
-    std::printf("canvas output -- the published layout (%u group%s)\n",
-                lay.count, lay.count == 1 ? "" : "s");
-    for (uint32_t g = 0; g < lay.count; ++g) {
-        const StatGroup& grp = lay.groups[g];
-        std::printf("  %-18s  ch %d   slot %d..%d   %s\n",
-                    grp.name, grp.channel,
-                    grp.slot_base, grp.slot_base + grp.count - 1,
-                    grp.shape == StatShape::Scalar ? "scalar" : "vector");
-    }
-}
-
-// Read the published signal back the way the_lab does: walk the layout, pull
-// each group's slots out of the signal. Scalars print as one value, vectors as
-// a bracketed row of their nonzero bins, each as degree:value above D.
-static void print_published(const Canvas& canvas) {
-    const AnalysisSignal& sig = canvas.output();
-    StatLayoutView lay = canvas.stat_layout();
-    for (uint32_t g = 0; g < lay.count; ++g) {
-        const StatGroup& grp = lay.groups[g];
-        if (g) std::printf("   ");
-        if (grp.shape == StatShape::Scalar) {
-            std::printf("%s=%g", grp.name, sig.stat(grp.channel, grp.slot_base));
-        } else {
-            std::printf("%s=[", grp.name);
-            bool first = true;
-            for (int i = 0; i < grp.count; ++i) {
-                const float x = sig.stat(grp.channel, grp.slot_base + i);
-                if (x == 0.0f) continue;
-                if (!first) std::printf(" ");
-                std::printf("%d:%g", i, x);
-                first = false;
-            }
-            std::printf("]");
-        }
-    }
-}
-
-// A note transition this frame, read from the playhead rather than from events:
-// any voice that started or ended on any configured channel.
-static bool note_changed(const Canvas& canvas) {
-    for (int slot = 0; slot < MAX_CHANNELS; ++slot) {
-        if (!canvas.active(slot)) continue;
-        const PlayheadReadout& ph = canvas.context(slot).playhead();
-        if (ph.onset_count > 0 || ph.release_count > 0) return true;
-    }
-    return false;
-}
-
-// The present notes of one voice — ground truth for what that channel holds now.
-static std::string present_notes(const Canvas& canvas, int slot) {
-    const PlayheadReadout& ph = canvas.context(slot).playhead();
-    std::string s;
-    for (int k = 0; k < ph.current_count; ++k) {
-        if (k) s += " ";
-        s += note_name(ph.current[k].pitch);
-    }
-    return s;
-}
-
-// The field's actual input: the cross-voice union of each active channel's
-// present-and-window pitch-class set, built from the same present_set /
-// present_union the canvas feeds the field. Printed as pitch-class letters so
-// the published field rank is explained by the classes that produced it. (For
-// canvas_1 the active voices are exactly the field's source, {0,1}.)
-static std::string field_input(const Canvas& canvas) {
-    PitchClassVector sets[MAX_CHANNELS];
-    int n = 0;
-    for (int slot = 0; slot < MAX_CHANNELS; ++slot) {
-        if (!canvas.active(slot)) continue;
-        const Context& c = canvas.context(slot);
-        sets[n++] = present_set(c.playhead(), c.wagon(0));
-    }
-    const PitchClassVector u = present_union(sets, n);
-    static const char* PC[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
-    std::string s;
-    for (int i = 0; i < 12; ++i)
-        if (u.v[i] > 0.0f) { if (!s.empty()) s += " "; s += PC[i]; }
-    return s;
-}
-
-int main() {
-    std::cout << std::unitbuf;
-
-    // Construct and bring up the cartridge: initialize() composes two voices
-    // (slot 0 <- MIDI 0, slot 1 <- MIDI 1), each present + a four-beat window,
-    // the spine off; publishes the union field; and opens loopMIDI.
-    Canvas canvas;
-    canvas.initialize(/*asset_path*/ nullptr);
-
-    std::cout << "canvas wiring -- the bindings\n";
-    for (int slot = 0; slot < MAX_CHANNELS; ++slot) print_binding(canvas, slot);
-    std::cout << "\n";
-    print_layout(canvas);
-    std::cout << "\n";
-
-    if (!canvas.is_open()) {
-        std::cout << "No port open -- is loopMIDI running and Ableton routed to it?\n";
-        return 1;
-    }
-    std::cout << "Reading " << canvas.port_name()
-              << " as a cartridge. Enable its Sync in Ableton and play.\n\n";
-
-    // Drive it: pass the real wall-clock dt (the signal's t_seconds telemetry),
-    // let update() drain the port and advance on the DAW's beat, then read the
-    // beat back from the published signal.
-    auto prev = std::chrono::steady_clock::now();
-    while (true) {
-        const auto now = std::chrono::steady_clock::now();
-        const float dt = std::chrono::duration<float>(now - prev).count();
-        prev = now;
-
-        canvas.update(dt);   // drains the owned port, advances, publishes
-
-        if (note_changed(canvas)) {
-            const float beat = canvas.output().t_beats;
-            std::printf("@%-7.2f ", beat);
-            print_published(canvas);                 // what the canvas speaks (the field)
-            std::printf("   ");
-            for (int slot = 0; slot < MAX_CHANNELS; ++slot) {
-                if (!canvas.active(slot)) continue;
-                std::printf("ch%d(%s) ", slot, present_notes(canvas, slot).c_str());
-            }
-            std::printf("  field-input{ %s }\n", field_input(canvas).c_str());
-        }
-
-        std::this_thread::sleep_for(std::chrono::milliseconds(2));
-    }
-}
```

#### D-1.7 — `src/musical/context.hpp`

Blob at `1a52f2db^`: `a4c3977e08b4bf8638b6072077995a095667ad5f` · deleted lines: 187 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/context.hpp b/src/musical/context.hpp
deleted file mode 100644
index a4c3977e..00000000
--- a/src/musical/context.hpp
+++ /dev/null
@@ -1,187 +0,0 @@
-#pragma once
-
-// ─── context.hpp ─────────────────────────────────────────────────
-//
-// Per-channel context: the stateful home for one channel's reading. It
-// owns the channel's MidiStream and reads through it in two layers. The
-// view layer is the stream as a slice — the Playhead (the present) and a
-// small bank of Wagons (trailing windows of completed history) — rebuilt
-// each frame from the stream. The memory layer is the stream as a
-// succession — the PreviousEvent (the latched prior onset-group) and the
-// Spine (the elected previous voice of a line) — fed the event flow and
-// holding its value across silence. Both memories are opt-in; a bare
-// context is views only.
-//
-// The two layers update on different occasions, because they are
-// different kinds. A view is stateless: it is rebuilt from the stream in
-// update(), each frame, whole. A memory accumulates: it is fed in
-// receive(), per event, and update() leaves it untouched. So the present
-// and the windows refresh every frame while the memories simply persist
-// between the events that move them.
-//
-// Routing feeds the whole flow. Every event reaches the stream; onsets
-// and offsets both reach whichever memories are enabled — onsets open and
-// group, offsets record the releases the elections read. The previous
-// event needs its offsets to know which voice lingered to the seam; the
-// spine needs both onsets and offsets to see its crossings.
-//
-// This is where the Train's ownership role lands now that the Train is
-// removed: the Context owns the windows and the memories. Composition
-// (the operations DAG) and shipping (the serializer) live elsewhere —
-// operation attachment is deliberately NOT here yet, and the spec-to-
-// context realizer is its own concern (context_realize.hpp).
-//
-// Per-frame cycle (driven by the canvas):
-//     for each event this frame:  ctx.receive(event)
-//     ctx.update(beat)
-//     ... read ctx.playhead(), ctx.previous(), ctx.spine(), ctx.wagon(i) ...
-//
-// One clock: every time the context handles is the beat clock the canvas
-// runs on. The windows clip in beats, and the two memories group in beats.
-//
-// Depends on: sources/midi_event.hpp, musical/midi_stream.hpp,
-//             musical/playhead.hpp, musical/previous_event.hpp,
-//             musical/wagon.hpp, musical/spine.hpp, <array>.
-
-#include "sources/midi_event.hpp"
-#include "musical/midi_stream.hpp"
-#include "musical/playhead.hpp"
-#include "musical/previous_event.hpp"
-#include "musical/wagon.hpp"
-#include "musical/spine.hpp"
-
-#include <array>
-
-namespace t7 {
-
-constexpr int CONTEXT_MAX_WAGONS = 4;
-
-class Context {
-public:
-    Context() = default;
-
-    // Owns a MidiStream (non-copyable); the Context is non-copyable, movable.
-    Context(const Context&) = delete;
-    Context& operator=(const Context&) = delete;
-    Context(Context&&) = default;
-    Context& operator=(Context&&) = default;
-
-    // ── Configuration (setup time) ───────────────────────────────
-
-    void set_channel(int channel) { stream_.set_channel(channel); }
-    int  channel() const { return stream_.channel(); }
-
-    void set_retention_beats(float beats) { stream_.set_retention_beats(beats); }
-
-    // Add a Wagon of the given span/offset. Returns its index, or -1 if full.
-    int add_wagon(float span_beats, float offset_beats = 0.0f) {
-        if (wagon_count_ >= CONTEXT_MAX_WAGONS) return -1;
-        const int slot = wagon_count_++;
-        wagons_[slot].set_span(span_beats);
-        wagons_[slot].set_offset(offset_beats);
-        return slot;
-    }
-
-    int wagon_count() const { return wagon_count_; }
-
-    // Turn on the previous-event memory (off by default) and set its
-    // onset-grouping tolerance, in beats.
-    void enable_previous(float tolerance_beats) {
-        previous_active_ = true;
-        previous_.set_tolerance(tolerance_beats);
-    }
-
-    // Turn on the spine memory (off by default), set its grouping
-    // tolerance in beats, and bind its oracle (lowest by default).
-    void enable_spine(float tolerance_beats,
-                      Spine::Oracle oracle = &Spine::oracle_lowest) {
-        spine_active_ = true;
-        spine_.set_tolerance(tolerance_beats);
-        spine_.set_oracle(oracle);
-    }
-
-    // ── Per-frame: receive events, then update ───────────────────
-
-    // Route one event for this channel. The stream takes every event; the
-    // enabled memories take onsets and offsets both — onsets to open and
-    // group, offsets to record the releases their elections read.
-    void receive(const MidiEvent& ev) {
-        stream_.receive(ev);
-
-        const bool is_on = (ev.type == MidiEvent::NOTE_ON);
-
-        if (previous_active_) {
-            if (is_on) previous_.on_onset(ev.pitch, ev.velocity, ev.beat);
-            else       previous_.on_offset(ev.pitch, ev.beat);
-        }
-        if (spine_active_) {
-            if (is_on) spine_.on_onset(ev.pitch, ev.beat);
-            else       spine_.on_offset(ev.pitch, ev.beat);
-        }
-    }
-
-    // Advance the stream, then rebuild the views. Call once per frame
-    // after routing this frame's events. The memories are not rebuilt
-    // here — they moved in receive() and hold otherwise.
-    void update(float beat) {
-        stream_.update(beat);
-
-        // A backward time jump clears the stream; the views and memories
-        // must follow so they do not carry stale state across the
-        // discontinuity.
-        if (stream_.had_time_discontinuity()) {
-            playhead_.clear();
-            previous_.clear();
-            spine_.clear();
-        }
-
-        const StreamSnapshot snap = stream_.snapshot();
-        playhead_.update(snap);
-
-        for (int i = 0; i < wagon_count_; ++i) {
-            wagons_[i].update(stream_.history(), beat);
-        }
-    }
-
-    // ── Readouts (read side) ─────────────────────────────────────
-
-    const PlayheadReadout& playhead() const { return playhead_.readout(); }
-    const PreviousEvent&   previous() const { return previous_; }
-    const Spine&           spine() const { return spine_; }
-
-    const WagonReadout& wagon(int i) const {
-        static const WagonReadout empty{};
-        return (i >= 0 && i < CONTEXT_MAX_WAGONS) ? wagons_[i].readout() : empty;
-    }
-
-    const MidiStream& stream() const { return stream_; }
-
-    // Which memories this context keeps — the read side checks these the
-    // way the canvas checks a spec, before reading a memory's readout.
-    bool has_previous() const { return previous_active_; }
-    bool has_spine() const { return spine_active_; }
-
-    // ── Reset ────────────────────────────────────────────────────
-
-    void clear() {
-        stream_.clear();
-        playhead_.clear();
-        previous_.clear();
-        spine_.clear();
-        for (int i = 0; i < wagon_count_; ++i) wagons_[i].clear();
-    }
-
-private:
-    MidiStream    stream_;
-    Playhead      playhead_;
-    PreviousEvent previous_;
-    Spine         spine_;
-
-    std::array<Wagon, CONTEXT_MAX_WAGONS> wagons_{};
-    int  wagon_count_ = 0;
-
-    bool previous_active_ = false;
-    bool spine_active_    = false;
-};
-
-} // namespace t7
```

#### D-1.8 — `src/musical/context_realize.hpp`

Blob at `1a52f2db^`: `4285730324237ed01ed9fbd9e624d9bd4896417e` · deleted lines: 75 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/context_realize.hpp b/src/musical/context_realize.hpp
deleted file mode 100644
index 42857303..00000000
--- a/src/musical/context_realize.hpp
+++ /dev/null
@@ -1,75 +0,0 @@
-#pragma once
-
-// ─── context_realize.hpp ─────────────────────────────────────────
-//
-// The realizer: it reads a ContextSpec and configures a Context to match.
-// The spec is data and the Context is mechanism; this is the one place
-// that knows both, turning a description into the live windows and
-// memories it names. Keeping it apart is what lets the spec be included
-// anywhere — it pulls in nothing — and the Context be used without a spec.
-//
-// It configures, it does not attach. Channel, retention, the windows, and
-// the two memories are set here; what reads a configured context — the
-// operations — is a later layer. Realize once, into a fresh context: the
-// windows accumulate, so a second pass over a used context would double
-// them.
-//
-// The present is not toggled here. The Playhead is the view layer's floor
-// and the cheapest of the views — a stateless snapshot, costing nothing
-// to keep fresh for a channel that does not read it — so the Context
-// maintains it always. A spec's `present` is a read-side declaration: the
-// canvas consults has_present() when it decides which operations a channel
-// may run, and that is where a dropped present takes effect. The memories
-// are gated instead, both because their feeding has cost and because an
-// un-fed memory is correctly empty, which is what "off" must mean.
-//
-// Retention is set to cover the windows whatever the spec names, so a
-// window can never out-reach the history behind it.
-//
-// Depends on: musical/context.hpp, musical/context_spec.hpp,
-//             musical/spine.hpp, <algorithm>.
-
-#include "musical/context.hpp"
-#include "musical/context_spec.hpp"
-#include "musical/spine.hpp"
-
-#include <algorithm>
-
-namespace t7 {
-
-// Map a spec's oracle binding to one of the spine's injectable oracles.
-// Lowest is the only binding bound so far; an unbound selector falls back
-// to lowest rather than leaving the oracle null.
-inline Spine::Oracle oracle_for(OracleBinding binding) {
-    switch (binding) {
-        case OracleBinding::Lowest: return &Spine::oracle_lowest;
-    }
-    return &Spine::oracle_lowest;
-}
-
-// Configure `ctx` to match `spec`. Realize into a fresh context.
-inline void realize(const ContextSpec& spec, Context& ctx) {
-    ctx.set_channel(spec.channel);
-
-    // Retention covers the deepest window, never less.
-    const float retention =
-        std::max(spec.stream_retention_beats, spec.required_retention_beats());
-    ctx.set_retention_beats(retention);
-
-    // View layer: the windows. The present is the always-maintained floor
-    // (see the file header for why it is not toggled here).
-    for (const auto& w : spec.windows) {
-        if (w.active) ctx.add_wagon(w.span_beats, w.offset_beats);
-    }
-
-    // Memory layer: each enabled only if the spec asks for it.
-    if (spec.event.active) {
-        ctx.enable_previous(spec.event.tolerance_beats);
-    }
-    if (spec.crossings.active) {
-        ctx.enable_spine(spec.crossings.tolerance_beats,
-                         oracle_for(spec.crossings.oracle));
-    }
-}
-
-} // namespace t7
```

#### D-1.9 — `src/musical/context_spec.hpp`

Blob at `1a52f2db^`: `563962e7d0f05355d953d498ad6137606a627d7a` · deleted lines: 191 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/context_spec.hpp b/src/musical/context_spec.hpp
deleted file mode 100644
index 563962e7..00000000
--- a/src/musical/context_spec.hpp
+++ /dev/null
@@ -1,191 +0,0 @@
-#pragma once
-
-// ─── context_spec.hpp ────────────────────────────────────────────
-//
-// The composition of one channel's context, written as data. A Context
-// owns the windows and memories a channel reads through; this spec says
-// which ones, and with what parameters, in a form the canvas can read
-// before it builds anything. The canvas holds one spec per channel, and
-// before it runs an operation it asks the spec whether the channel
-// supplies what the operation needs — so the description has to stand on
-// its own, ahead of the Context it gives rise to.
-//
-// A composition divides in two, because a channel's material does. The
-// view layer is the stream read as a slice: the present, the notes
-// sounding now, and a small bank of windows, each a trailing depth of the
-// completed past. A view keeps nothing of its own — snapshot it from the
-// stream at any instant and it is whole — so it is taken fresh each frame.
-// The memory layer is the stream read as a succession that outlasts the
-// slice: the previous-event, which latches the last onset-group, and the
-// spine, which latches the elected previous voice of a line. A memory
-// cannot be snapshotted into being; it is fed the event flow and
-// accumulates, and it holds its value across the silence a slice would
-// simply find empty.
-//
-// The default floor is a present and one window, the memory layer empty:
-// enough for the measures and selections that read only the present and
-// its window, and silent about succession until a channel asks for it.
-// The window's depth is a per-channel quantity — there is no shared span
-// constant; one channel's bar is not another's.
-//
-// One clock. Every quantity here is in beats, the clock the Context
-// already runs on: the windows clip in beats, the stream retains in beats,
-// and the two memories group in beats — the width within which onsets
-// fuse into one event, or offsets into one cohort. The spine was first
-// built and probed on a separate clock of arrival seconds; owned by the
-// Context it is fed the beat clock with the rest, and its grouping becomes
-// musical rather than perceptual. That this makes simultaneity scale with
-// tempo is deliberate, of a piece with windows measured in beats; a
-// perceptual seconds-grouping is the alternative the slot leaves open.
-//
-// This file is data only — it includes none of the views or memories it
-// describes. The realizer that turns a spec into a live Context, and the
-// table that says which operation needs which layer, are separate.
-//
-// Usage:
-//   ContextSpec s = default_spec(/*channel*/ 2, /*window beats*/ 4.0f);
-//   s.add_window(8.0f, 4.0f);          // a second, deeper, offset window
-//   s.crossings.active = true;         // turn the spine on (beats default)
-//   if (s.has_crossings()) { /* the canvas may run a step here */ }
-//
-// Depends on: <array>, <cstdint>.
-
-#include <array>
-#include <cstdint>
-
-namespace t7 {
-
-// ═══ LIMITS ══════════════════════════════════════════════════════
-
-// A channel carries at most this many windows. Matches CONTEXT_MAX_WAGONS;
-// the realizer maps each active window onto one Wagon.
-constexpr int SPEC_MAX_WINDOWS = 4;
-
-// ═══ VIEW LAYER ══════════════════════════════════════════════════
-
-// A window the channel reads: a trailing slice of the completed stream,
-// clipped to [anchor - offset - span, anchor - offset]. The span is the
-// window's depth and is a per-channel quantity, never a shared constant;
-// the offset slides the whole window back from the present. Both in beats,
-// to match Context::add_wagon(span_beats, offset_beats).
-struct WindowSpec {
-    float span_beats   = 0.0f;   // the window's depth
-    float offset_beats = 0.0f;   // how far back the window's near edge sits
-    bool  active       = false;  // whether this slot is in use
-};
-
-// ═══ MEMORY LAYER ════════════════════════════════════════════════
-
-// The binding the spine consults when the music alone cannot decide the
-// previous — the last key of the cascade, selecting one note from a tie.
-// Lowest is the only binding drawn so far; the slot is open by design, and
-// the alternatives are named here as they are bound, never left implicit.
-// The realizer maps a binding to one of the spine's injectable oracles.
-enum class OracleBinding : uint8_t {
-    Lowest = 0,   // the lowest pitch of the tie (Spine::oracle_lowest)
-    // Highest, Loudest, Newest, ... — open, not yet bound.
-};
-
-// The previous-event memory: a latch over onset-groups. Onsets falling
-// within `tolerance_beats` of one another fuse into a single event, and
-// the latch holds the last such group across the silence that follows.
-struct EventMemorySpec {
-    bool  active         = false;
-    float tolerance_beats = 0.1f;   // the onset-grouping (simultaneity) width
-};
-
-// The spine: a latch over the line. It elects the previous voice at the
-// crossings through cardinality one and holds the resolved election across
-// silence. `tolerance_beats` is the grouping width the final-offset cohort
-// and the synchronous-onset test share; `oracle` is the last-resort
-// binding consulted when neither survival nor a recorded mint decides.
-struct CrossingMemorySpec {
-    bool          active         = false;
-    float         tolerance_beats = 0.05f;                // cohort / synchronous width
-    OracleBinding oracle         = OracleBinding::Lowest;
-};
-
-// ═══ THE COMPOSITION ═════════════════════════════════════════════
-
-// The composition of one channel: a view layer (a present, a bank of
-// windows) and a memory layer (the previous-event, the spine). All of it
-// is data; the queries near the end are what the canvas asks to learn
-// whether a channel provides what an operation requires, before the
-// operation is allowed to run.
-struct ContextSpec {
-    // The MIDI channel this composition reads.
-    int channel = 0;
-
-    // How much completed history the stream retains, in beats. It must
-    // cover the deepest window; required_retention_beats() reports that
-    // floor, and default_spec() seeds it from the windows declared.
-    float stream_retention_beats = 0.0f;
-
-    // ── View layer ───────────────────────────────────────────────
-    // The present has no parameters — it simply reads the notes sounding
-    // now — so it is a flag, not a struct. It is the floor, and droppable:
-    // a window-only channel is coherent, purely retrospective, but it
-    // forfeits the present half of every combine.
-    bool present = true;
-    std::array<WindowSpec, SPEC_MAX_WINDOWS> windows{};
-
-    // ── Memory layer ─────────────────────────────────────────────
-    // Both off by default. Either may be turned on; each is then fed the
-    // complete event flow — onsets and offsets, in arrival order, on the
-    // beat clock — and holds across silence.
-    EventMemorySpec    event;       // the previous-event latch
-    CrossingMemorySpec crossings;   // the spine
-
-    // ── Construction helper ──────────────────────────────────────
-
-    // Fill the next free window slot. Returns false if the bank is full.
-    bool add_window(float span_beats, float offset_beats = 0.0f) {
-        for (auto& w : windows) {
-            if (!w.active) {
-                w = WindowSpec{span_beats, offset_beats, true};
-                return true;
-            }
-        }
-        return false;
-    }
-
-    // ── Queries the canvas checks an operation against ───────────
-
-    bool has_present()   const { return present; }
-    bool has_event()     const { return event.active; }
-    bool has_crossings() const { return crossings.active; }
-
-    int window_count() const {
-        int n = 0;
-        for (const auto& w : windows) if (w.active) ++n;
-        return n;
-    }
-    bool has_window() const { return window_count() > 0; }
-
-    // The retention the windows demand: the farthest near-edge plus depth.
-    float required_retention_beats() const {
-        float deepest = 0.0f;
-        for (const auto& w : windows) {
-            if (w.active) {
-                const float reach = w.offset_beats + w.span_beats;
-                if (reach > deepest) deepest = reach;
-            }
-        }
-        return deepest;
-    }
-};
-
-// ═══ DEFAULT FLOOR ═══════════════════════════════════════════════
-
-// The default composition: a present and a single window, no memories.
-// The window's depth is supplied per channel — there is no built-in span.
-inline ContextSpec default_spec(int channel, float window_span_beats) {
-    ContextSpec s;
-    s.channel = channel;
-    s.present = true;
-    s.add_window(window_span_beats);
-    s.stream_retention_beats = s.required_retention_beats();
-    return s;
-}
-
-} // namespace t7
```

#### D-1.10 — `src/musical/field.hpp`

Blob at `1a52f2db^`: `5e1f38d74dedcf8b32f5cd69a03b46808fd8115f` · deleted lines: 127 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/field.hpp b/src/musical/field.hpp
deleted file mode 100644
index 5e1f38d7..00000000
--- a/src/musical/field.hpp
+++ /dev/null
@@ -1,127 +0,0 @@
-#pragma once
-
-// field.hpp ───────────────────────────────────────────────────────────────
-//
-// Locate a pitch-class set among declared harmonic fields. The barest harmonic
-// reading there is.
-//
-// Take which pitch classes are sounding -- present (Playhead) OR completed in
-// the recent window (Wagon) -- as a BINARY set: one per present class, no
-// weight, no time. Re-origin it onto the home D, so each entry is a degree above
-// the root. Dot it against each declared field, whose degrees are themselves a
-// binary vector. The dot of two binary vectors is the count of shared degrees --
-// how many of the sounding notes belong to that field. One number per field is
-// the set's coordinates in field-space.
-//
-// What is deliberately ABSENT, to be added later one at a time, each leaving
-// this map untouched: weighting (length / count / recency enrich the input
-// vector's values), the in-minus-out penalty (refines the field masks), and the
-// moment-to-moment held frame (acts on the stream of these vectors, downstream).
-//
-// Depends on: musical/playhead.hpp, musical/wagon.hpp, musical/musical_ops.hpp.
-
-#include "musical/playhead.hpp"
-#include "musical/wagon.hpp"
-#include "musical/musical_ops.hpp"
-
-namespace t7 {
-
-// Binary presence over absolute pitch classes (C = 0): 1 if any note of that
-// class is sounding now or completed in the window, else 0. Present and window
-// are disjoint; OR-ing them cannot double-mark.
-inline PitchClassVector present_set(const PlayheadReadout& ph,
-                                    const WagonReadout& wg) {
-    PitchClassVector s;
-    for (int i = 0; i < wg.note_count; ++i)   s.v[wg.notes[i].pitch % 12] = 1.0f;
-    for (int i = 0; i < ph.current_count; ++i) s.v[ph.current[i].pitch % 12] = 1.0f;
-    return s;
-}
-
-// to_degrees — the re-origin onto the root — now lives in musical_ops, beside
-// pc_relative_to, since the dressing and the spine reach for it too. field.hpp
-// includes musical_ops, so the rotation is in scope here unchanged.
-
-// A declared field: a name and its degrees as a root-position binary vector
-// (bin 0 = root). The bank of fields is the axes of the musical space; it is
-// composition policy, declared by the caller.
-struct Field {
-    const char*      name;
-    PitchClassVector mask;
-};
-
-// Build a field mask from a root-position degree list (semitones above root).
-inline PitchClassVector field_mask(std::initializer_list<int> degrees) {
-    PitchClassVector m;
-    for (int d : degrees) m.v[((d % 12) + 12) % 12] = 1.0f;
-    return m;
-}
-
-// Overlap of a degree-set with a field: shared degrees. For binary inputs this
-// is the count of the set's degrees that lie in the field.
-inline float field_overlap(const PitchClassVector& degrees,
-                           const PitchClassVector& mask) {
-    float s = 0.0f;
-    for (int i = 0; i < 12; ++i) s += degrees.v[i] * mask.v[i];
-    return s;
-}
-
-// The result of electing one field from the bank by overlap and hierarchy.
-struct FieldChoice {
-    int   index = 0;     // chosen field's position in the bank
-    float overlap = 0.0f; // its overlap with the set
-    int   tie = 0;        // how many fields share the top overlap (1 = clear)
-
-    bool ambiguous() const { return tie > 1; }
-};
-
-// Elect a field: the MAXIMUM overlap, ties broken by HIERARCHY -- and the bank's
-// order is the hierarchy, earlier = higher. The first pass keeps the earliest
-// field at the top overlap (a later equal never displaces it), so a lower-ranked
-// field wins only by strictly exceeding every field above it -- which it can do
-// only by holding a degree they lack, its signature. `tie` counts how many share
-// the top, so ambiguity stays visible rather than hidden behind the pick.
-inline FieldChoice elect_field(const PitchClassVector& degrees,
-                               const Field* bank, int n) {
-    FieldChoice c;
-    c.overlap = -1.0f;
-    for (int i = 0; i < n; ++i) {
-        const float ov = field_overlap(degrees, bank[i].mask);
-        if (ov > c.overlap) { c.overlap = ov; c.index = i; }   // strict: earlier wins ties
-    }
-    for (int i = 0; i < n; ++i)
-        if (field_overlap(degrees, bank[i].mask) == c.overlap) ++c.tie;
-    return c;
-}
-
-// The held field: the standing harmonic reading. This is the one stateful piece
-// of the field card. It holds the incumbent through ambiguity and moves only
-// when another field strictly out-scores it -- the cascade as persistence:
-//
-//   - no incumbent yet            -> take the elected leader        (bootstrap)
-//   - some field strictly beats it -> move to the elected leader     (the music)
-//   - otherwise                    -> hold                           (persistence)
-//
-// "Otherwise" covers two cases at once. A draw that still includes the incumbent
-// (its overlap equals the maximum) holds, because the elected leader's overlap
-// does not exceed it. And silence holds for the same reason without a special
-// case: an empty set scores zero on every field, so the incumbent is tied for
-// the top at zero and is not beaten. The threshold that would one day let a
-// challenger need MORE than a bare strict win, or let silence eventually release
-// the field, attaches to these two edges later; here a single strict beat moves
-// it and silence holds it forever.
-struct HeldField {
-    int incumbent = -1;   // -1 = none yet
-
-    int step(const PitchClassVector& degrees, const Field* bank, int n) {
-        const FieldChoice pick = elect_field(degrees, bank, n);
-        if (pick.overlap <= 0.0f) return incumbent;             // empty / silence: hold
-        if (incumbent < 0) { incumbent = pick.index; return incumbent; }   // bootstrap
-        const float held = field_overlap(degrees, bank[incumbent].mask);
-        if (pick.overlap > held) incumbent = pick.index;        // strictly beaten: move
-        return incumbent;                                        // else hold
-    }
-
-    bool settled() const { return incumbent >= 0; }
-};
-
-} // namespace t7
```

#### D-1.11 — `src/musical/midi_stream.hpp`

Blob at `1a52f2db^`: `f74d0b039cb6d630fe0f5e25b6c7067e2d9b18bf` · deleted lines: 179 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/midi_stream.hpp b/src/musical/midi_stream.hpp
deleted file mode 100644
index f74d0b03..00000000
--- a/src/musical/midi_stream.hpp
+++ /dev/null
@@ -1,179 +0,0 @@
-#pragma once
-
-// ─── midi_stream.hpp ─────────────────────────────────────────────
-//
-// One Stream per MIDI channel — the single source of truth for that
-// channel. Constructed inert (no external coupling): it receives events
-// explicitly via receive(), which keeps storage fixed-array (no heap),
-// makes it testable (inject events directly), and keeps control flow
-// visible (composition routes events).
-//
-// Responsibilities: receive note events; maintain the ActiveSet
-// (currently sounding) and CompletedRing (finished sounding); prune
-// history beyond the retention window; produce snapshots for analyzers.
-//
-// Time invariant: beat is assumed monotonically increasing. If time
-// jumps backward (loop, seek), the stream clears all state to stay
-// consistent.
-//
-// Usage:
-//   MidiStream streams[4];
-//   for (int i = 0; i < 4; ++i) streams[i].set_channel(i);
-//   MidiEvent event = MidiEvent::note_on(2, 60, 0.8f, beat);
-//   streams[event.channel].receive(event);
-//   for (auto& s : streams) s.update(beat);
-//   auto snap = streams[2].snapshot();
-//
-// Depends on: musical/stream_data.hpp (ActiveSet, CompletedRing,
-//             StreamSnapshot), sources/midi_event.hpp (MidiEvent).
-
-#include "musical/stream_data.hpp"
-#include "sources/midi_event.hpp"
-
-namespace t7 {
-
-// ═══ MIDI STREAM ═════════════════════════════════════════════════
-
-class MidiStream {
-public:
-    /**
-     * Default constructor - inert, no external coupling.
-     */
-    MidiStream() = default;
-    
-    /**
-     * Construct with channel assignment.
-     */
-    explicit MidiStream(int channel, float retention_beats = 64.0f)
-        : channel_(channel)
-        , retention_beats_(retention_beats)
-    {}
-    
-    // Non-copyable (owns significant state)
-    MidiStream(const MidiStream&) = delete;
-    MidiStream& operator=(const MidiStream&) = delete;
-    
-    // Movable
-    MidiStream(MidiStream&&) = default;
-    MidiStream& operator=(MidiStream&&) = default;
-    
-    // ── Configuration ────────────────────────────────────────────
-
-    void set_channel(int channel) { channel_ = channel; }
-    int channel() const { return channel_; }
-    
-    void set_retention_beats(float beats) { retention_beats_ = beats; }
-    float retention_beats() const { return retention_beats_; }
-    
-    // ── Event Reception ──────────────────────────────────────────
-
-    /**
-     * Receive a MIDI event.
-     * Called by composition to route events to this stream.
-     */
-    void receive(const MidiEvent& event) {
-        if (event.type == MidiEvent::NOTE_ON) {
-            active_.note_on(event.pitch, event.velocity, event.beat);
-        } else {
-            ActiveNote was_active = active_.note_off(event.pitch);
-            if (was_active.is_active()) {
-                completed_.push(event.pitch, was_active.velocity, 
-                               was_active.onset_beat, event.beat);
-            }
-        }
-    }
-    
-    // ── Frame Update ─────────────────────────────────────────────
-
-    /**
-     * Advance stream time and prune old history.
-     * Call once per frame after routing all events.
-     * 
-     * IMPORTANT: If beat jumps backward (loop/seek), the stream automatically
-     * clears all state. This maintains the monotonic time invariant.
-     */
-    void update(float current_beat) {
-        // Detect backward time jump (loop/seek)
-        constexpr float BACKWARD_THRESHOLD = 0.001f;
-        had_discontinuity_ = false;
-        
-        if (current_beat < current_beat_ - BACKWARD_THRESHOLD) {
-            clear();
-            had_discontinuity_ = true;
-        }
-        
-        current_beat_ = current_beat;
-        
-        // Prune completed notes older than retention window
-        float cutoff = current_beat - retention_beats_;
-        completed_.prune_before(cutoff);
-    }
-    
-    // ── Snapshot ─────────────────────────────────────────────────
-
-    /**
-     * Produce a snapshot of current ephemeral state.
-     * 
-     * This is a value type - safe to copy, store, pass to parallel analysis.
-     */
-    StreamSnapshot snapshot() const {
-        StreamSnapshot snap;
-        snap.beat = current_beat_;
-        snap.channel = channel_;
-        snap.active = active_;
-        return snap;
-    }
-    
-    /**
-     * Access to history (read-only).
-     * CompletedRing is append-only facts, safe for concurrent reads.
-     */
-    const CompletedRing& history() const { return completed_; }
-    
-    // ── Queries ──────────────────────────────────────────────────
-
-    float current_beat() const { return current_beat_; }
-    bool had_time_discontinuity() const { return had_discontinuity_; }
-    
-    int active_count() const { return active_.count(); }
-    bool is_silent() const { return active_.empty(); }
-    bool has_active() const { return !active_.empty(); }
-    
-    int completed_count() const { return completed_.size(); }
-    bool has_completed() const { return !completed_.empty(); }
-    
-    // ── Direct Access ────────────────────────────────────────────
-
-    const ActiveSet& active_set() const { return active_; }
-    const CompletedRing& completed_ring() const { return completed_; }
-    
-    // ── State Management ─────────────────────────────────────────
-
-    /**
-     * Clear all state. Use when seeking or resetting playback.
-     * Called automatically if update() detects backward time jump.
-     */
-    void clear() {
-        active_.clear();
-        completed_.clear();
-    }
-    
-private:
-    int channel_ = 0;
-    float retention_beats_ = 64.0f;
-    float current_beat_ = 0.0f;
-    bool had_discontinuity_ = false;
-    
-    ActiveSet active_;
-    CompletedRing completed_;
-};
-
-// ── Size Verification ────────────────────────────────────────────
-
-// MidiStream memory:
-//   ActiveSet     ~  1 KB
-//   CompletedRing ~ 32 KB
-//   Scalars       ~ 16 bytes
-//   Total         ~ 33 KB per channel
-
-} // namespace t7
```

#### D-1.12 — `src/musical/musical_ops.hpp`

Blob at `1a52f2db^`: `033b3a2f1a9f06f5c5dcf371408dea8fc68aa986` · deleted lines: 192 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/musical_ops.hpp b/src/musical/musical_ops.hpp
deleted file mode 100644
index 033b3a2f..00000000
--- a/src/musical/musical_ops.hpp
+++ /dev/null
@@ -1,192 +0,0 @@
-#pragma once
-
-// ─── musical_ops.hpp ─────────────────────────────────────────────
-//
-// Vocabulary only: the typed representations that are the currency of
-// the analysis, plus the two pitch-class primitives every operation is
-// built from. No extractors, no reductions, no relations — those are
-// built when a concrete statistic makes them self-evident, against the
-// readout they actually read (Playhead / Wagon / PreviousEvent).
-//
-// Representations : PitchClassBits (12-bit membership mask, a set with
-//                   union/intersection/complement), PitchClassVector
-//                   (12-float weighted distribution, the linear-algebra
-//                   currency).
-// Primitives      : pc_of (MIDI → pitch class), pc_relative_to (re-origin a
-//                   pitch class — the reign's transpose), distance (signed
-//                   registral interval between two pitches), to_degrees
-//                   (re-origin a whole vector — pc_relative_to's rotation
-//                   carried to the distribution).
-//
-// Depends on: <array>, <cmath>, <cstdint>. Nothing upstream.
-
-#include <array>
-#include <cmath>
-#include <cstdint>
-
-namespace t7 {
-
-// ═══ PITCH-CLASS PRIMITIVES ══════════════════════════════════════
-
-/**
- * Pitch class of a MIDI note in the default C-origin convention (C = 0).
- * Input: MIDI pitch (0-127). Output: pitch class 0-11.
- */
-inline int pc_of(int midi_pitch) {
-    return ((midi_pitch % 12) + 12) % 12;
-}
-
-/**
- * Re-express a pitch class with `origin` as zero (origin transpose).
- * `origin` is the C-origin index of the desired zero (e.g. 2 = D).
- * Input: pc 0-11, origin 0-11. Output: transposed pc 0-11.
- */
-inline int pc_relative_to(int pc, int origin) {
-    return ((pc - origin) % 12 + 12) % 12;
-}
-
-/**
- * Signed registral interval between two pitches, in semitones: the distance
- * travelled from `from` up to `to`, positive upward. The registral cousin of
- * pc_relative_to — that folds onto one class, this keeps the octave.
- */
-inline int distance(int from, int to) {
-    return to - from;
-}
-
-// ═══ REPRESENTATIONS ═════════════════════════════════════════════
-
-// ── Pitch Class Bits ─────────────────────────────────────────────
-
-/**
- * 12-bit mask representing pitch classes present.
- * Bit N = pitch class N is present (C=0, C#=1, ... B=11).
- *
- * The set representation: union (|), intersection (&), symmetric
- * difference (^), complement (~, masked to 12 bits).
- */
-struct PitchClassBits {
-    uint16_t bits = 0;
-
-    void set(int pc) { bits = uint16_t(bits | (1u << (pc % 12))); }
-    void clear(int pc) { bits = uint16_t(bits & ~(1u << (pc % 12))); }
-    bool test(int pc) const { return (bits & (1u << (pc % 12))) != 0; }
-    void clear_all() { bits = 0; }
-
-    int count() const {
-        uint16_t b = bits;
-        b = uint16_t(b - ((b >> 1) & 0x5555));
-        b = uint16_t((b & 0x3333) + ((b >> 2) & 0x3333));
-        b = uint16_t((b + (b >> 4)) & 0x0F0F);
-        return (b + (b >> 8)) & 0x1F;
-    }
-
-    bool empty() const { return bits == 0; }
-
-    PitchClassBits operator&(PitchClassBits o) const { return {uint16_t(bits & o.bits)}; }
-    PitchClassBits operator|(PitchClassBits o) const { return {uint16_t(bits | o.bits)}; }
-    PitchClassBits operator^(PitchClassBits o) const { return {uint16_t(bits ^ o.bits)}; }
-    PitchClassBits operator~() const { return {uint16_t(~bits & 0x0FFF)}; }
-
-    bool operator==(PitchClassBits o) const { return bits == o.bits; }
-    bool operator!=(PitchClassBits o) const { return bits != o.bits; }
-};
-
-// ── Pitch Class Vector ───────────────────────────────────────────
-
-/**
- * 12-float vector: weight of each pitch class. The currency that edges
- * in the operation DAG carry. Weights are whatever the extractor lifts
- * in — velocity, length, length*velocity, or binary 0/1.
- */
-struct PitchClassVector {
-    std::array<float, 12> v = {};
-
-    float& operator[](int pc) { return v[pc % 12]; }
-    float operator[](int pc) const { return v[pc % 12]; }
-
-    void clear() { v.fill(0.0f); }
-
-    // Element-wise sum — vector addition. The additive combine for compound
-    // readings: a group's count/length is the sum of its channels' vectors, and
-    // re-origin commutes with it (to_degrees is a rotation), so the compound
-    // equals the sum of the per-channel published vectors. The set/field combine
-    // is present_union (OR), below; addition is for weights.
-    PitchClassVector operator+(const PitchClassVector& o) const {
-        PitchClassVector r;
-        for (int i = 0; i < 12; ++i) r.v[i] = v[i] + o.v[i];
-        return r;
-    }
-    PitchClassVector& operator+=(const PitchClassVector& o) {
-        for (int i = 0; i < 12; ++i) v[i] += o.v[i];
-        return *this;
-    }
-
-    float sum() const {
-        float s = 0.0f;
-        for (float x : v) s += x;
-        return s;
-    }
-
-    float max() const {
-        float m = 0.0f;
-        for (float x : v) if (x > m) m = x;
-        return m;
-    }
-
-    /** Normalize to unit sum (a distribution). Identity if empty. */
-    PitchClassVector normalized() const {
-        float s = sum();
-        if (s <= 0.0f) return *this;
-        PitchClassVector r;
-        for (int i = 0; i < 12; ++i) r.v[i] = v[i] / s;
-        return r;
-    }
-
-    /** Normalize to unit L2 length. Identity if empty. */
-    PitchClassVector unit() const {
-        float sq = 0.0f;
-        for (float x : v) sq += x * x;
-        if (sq <= 0.0f) return *this;
-        float norm = std::sqrt(sq);
-        PitchClassVector r;
-        for (int i = 0; i < 12; ++i) r.v[i] = v[i] / norm;
-        return r;
-    }
-};
-
-// ═══ RE-ORIGIN (VECTOR) ══════════════════════════════════════════
-
-/**
- * Rotate a vector so `root_pc` lands on bin 0; bin i is then the degree i
- * semitones above the root. The vector form of pc_relative_to — one class
- * re-expressed becomes the whole distribution re-expressed. Root 0 (C) is
- * the identity.
- */
-inline PitchClassVector to_degrees(const PitchClassVector& abs, int root_pc) {
-    PitchClassVector p;
-    int r = ((root_pc % 12) + 12) % 12;
-    for (int i = 0; i < 12; ++i) p.v[i] = abs.v[(r + i) % 12];
-    return p;
-}
-
-// ═══ UNION (VECTOR) ══════════════════════════════════════════════
-
-/**
- * Element-wise maximum of several pitch-class vectors. For the binary
- * present-sets the field reads, that maximum is set union: a class is marked
- * iff it is marked in any input, and the max of ones and zeros is again one or
- * zero, so binary in gives binary out. (Weighted vectors would combine by their
- * strongest per-class weight; the field uses the binary case alone.) This is how
- * a reading rises from one voice to a group — widen the source here, and nothing
- * downstream changes.
- */
-inline PitchClassVector present_union(const PitchClassVector* sets, int n) {
-    PitchClassVector u;
-    for (int c = 0; c < n; ++c)
-        for (int i = 0; i < 12; ++i)
-            if (sets[c].v[i] > u.v[i]) u.v[i] = sets[c].v[i];
-    return u;
-}
-
-} // namespace t7
```

#### D-1.13 — `src/musical/pc_count.hpp`

Blob at `1a52f2db^`: `acccba766e2a9b05b3d03f99ce3a238b6376dd9a` · deleted lines: 132 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/pc_count.hpp b/src/musical/pc_count.hpp
deleted file mode 100644
index acccba76..00000000
--- a/src/musical/pc_count.hpp
+++ /dev/null
@@ -1,132 +0,0 @@
-#pragma once
-
-// ─── pc_count.hpp ────────────────────────────────────────────────
-//
-// Canvas operations over a view's notes, in the 12-slot pitch-class space
-// (C = 0, octaves folded by pc_of). The count is the base; the others
-// derive from it — the same loop with a different weight, a fold of the
-// count, or the disjoint sum of two views.
-//
-//   pc_count  — weight 1 per occurrence (the base). Over the present, the
-//               window, or their union.
-//   pc_length — weight = the note's length. The present carries a
-//               provisional length (anchor − onset, still growing while
-//               held); the window a completed one; the union sums the two,
-//               which are disjoint because a note is sounding or completed,
-//               never both.
-//   pc_onset  — weight = the note-on's velocity, filtered to onsets since a
-//               caller-held beat (the per-frame aperture). Present + window,
-//               disjoint as the length is.
-//   pc_set    — support of a count: the pitch-class SET, a class in iff it
-//               occurred at all. Derives FROM the count (count dominates;
-//               the set is recoverable from the count, not the reverse).
-//
-// The union operations take both readouts and sum the single-view results.
-// The disjointness of present and window is what lets the sum stand without
-// double-counting: the same note instance never appears in both.
-//
-// Depends on: musical/musical_ops.hpp (PitchClassVector, PitchClassBits,
-//             pc_of), musical/playhead.hpp, musical/wagon.hpp.
-
-#include "musical/musical_ops.hpp"
-#include "musical/playhead.hpp"
-#include "musical/wagon.hpp"
-
-namespace t7 {
-
-// ═══ COUNT ═══════════════════════════════════════════════════════
-
-// Present notes → pitch-class counts. One sounding note adds 1 at its pc;
-// two sounding notes of the same class (different octaves) add 2.
-inline PitchClassVector pc_count(const PlayheadReadout& ph) {
-    PitchClassVector v;
-    for (int i = 0; i < ph.current_count; ++i)
-        v[pc_of(ph.current[i].pitch)] += 1.0f;
-    return v;
-}
-
-// Windowed completed occurrences → pitch-class counts. Each WindowNote is
-// one occurrence; repeats of a class within the window accumulate.
-inline PitchClassVector pc_count(const WagonReadout& wg) {
-    PitchClassVector v;
-    for (int i = 0; i < wg.note_count; ++i)
-        v[pc_of(wg.notes[i].pitch)] += 1.0f;
-    return v;
-}
-
-// Present + window counts, the disjoint union. A note is sounding now or
-// completed in the window, never both, so the sum does not double-count.
-inline PitchClassVector pc_count(const PlayheadReadout& ph, const WagonReadout& wg) {
-    PitchClassVector v = pc_count(ph);
-    const PitchClassVector w = pc_count(wg);
-    for (int i = 0; i < 12; ++i) v.v[i] += w.v[i];
-    return v;
-}
-
-// ═══ LENGTH ══════════════════════════════════════════════════════
-
-// Present notes → cumulative provisional length per pitch class. The same
-// reduction as the present count, with the weight switched from 1 to the
-// note's provisional span at the anchor (anchor − onset, still growing).
-inline PitchClassVector pc_length(const PlayheadReadout& ph) {
-    PitchClassVector v;
-    for (int i = 0; i < ph.current_count; ++i)
-        v[pc_of(ph.current[i].pitch)] += ph.current_duration(i);
-    return v;
-}
-
-// Windowed cumulative in-window length per pitch class. The same reduction
-// as the window count, with the weight switched from 1 to the note's clipped
-// span (window_duration). Summed across occurrences of the class.
-inline PitchClassVector pc_length(const WagonReadout& wg) {
-    PitchClassVector v;
-    for (int i = 0; i < wg.note_count; ++i)
-        v[pc_of(wg.notes[i].pitch)] += wg.notes[i].window_duration();
-    return v;
-}
-
-// Present + window length, the disjoint union: the present's provisional
-// span summed with the window's completed span. Disjoint as the counts are,
-// so the two halves add without overlap.
-inline PitchClassVector pc_length(const PlayheadReadout& ph, const WagonReadout& wg) {
-    PitchClassVector v = pc_length(ph);
-    const PitchClassVector w = pc_length(wg);
-    for (int i = 0; i < 12; ++i) v.v[i] += w.v[i];
-    return v;
-}
-
-// ═══ ONSET ═══════════════════════════════════════════════════════
-
-// Note-on impulses since `since_beat` (exclusive) → velocity-weighted
-// pitch-class sums. The same reduction as the counts, with the weight
-// switched from 1 to the note-on's velocity (already [0,1] at the stream
-// layer) and the population filtered to onsets inside (since_beat, anchor].
-// The present carries the still-sounding onsets; the window carries the
-// ones already completed (an on-and-off within a single frame) — disjoint
-// as ever, so the sum stands without double-counting. Each onset lands in
-// exactly one aperture: the caller advances since_beat to its anchor after
-// every read. Same-class retriggers inside one aperture sum.
-inline PitchClassVector pc_onset(const PlayheadReadout& ph, const WagonReadout& wg, float since_beat) {
-    PitchClassVector v;
-    for (int i = 0; i < ph.current_count; ++i)
-        if (ph.current[i].onset_beat > since_beat)
-            v[pc_of(ph.current[i].pitch)] += ph.current[i].velocity;
-    for (int i = 0; i < wg.note_count; ++i)
-        if (wg.notes[i].onset_beat > since_beat)
-            v[pc_of(wg.notes[i].pitch)] += wg.notes[i].velocity;
-    return v;
-}
-
-// ═══ SET ═════════════════════════════════════════════════════════
-
-// Support of a count vector: the pitch-class set. A class is in the set iff
-// its count is greater than zero. The set type carries set algebra (union,
-// intersection, complement) for the compound layer; here it is simply the
-// thresholded count.
-inline PitchClassBits pc_set(const PitchClassVector& count) {
-    PitchClassBits s;
-    for (int i = 0; i < 12; ++i) if (count.v[i] > 0.0f) s.set(i);
-    return s;
-}
-
-} // namespace t7
```

#### D-1.14 — `src/musical/pc_dft.hpp`

Blob at `1a52f2db^`: `8429dd9ba8a679268d5e30abe1d7b25ba4a68765` · deleted lines: 64 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/pc_dft.hpp b/src/musical/pc_dft.hpp
deleted file mode 100644
index 8429dd9b..00000000
--- a/src/musical/pc_dft.hpp
+++ /dev/null
@@ -1,64 +0,0 @@
-#pragma once
-
-// ─── pc_dft.hpp ──────────────────────────────────────────────────
-//
-// THE PC-DFT CAPABILITY (the compound stratum): the discrete Fourier
-// transform of a PUBLISHED pitch-class vector — the six interval
-// families f1..f6 as magnitude + phase. One stratum above pc_count:
-// where pc_count reads the VIEWS (playhead/wagon), pc_dft reads a
-// published pc-vector — the same 12 floats the PRESENT_COUNT slots
-// carry, re-origined as shipped. The DFT reads what ships, not raw
-// state.
-//
-// The families (Fourier-phase literature): f1 chromatic cluster axis ·
-// f2 tritone-pair axis · f3 minor-third/octatonic · f4 major-third/
-// hexatonic · f5 the FIFTHS family (diatonic/triadic sets peak here —
-// a major triad's argmax) · f6 the whole-tone family (a whole-tone
-// cluster puts ALL its energy here).
-//
-// UNITS (the slot-map contract):
-//   mag[k−1]   normalized [0,1] — |X_k| ÷ L1(v); L1 = 0 → 0 (safe;
-//              for non-negative v the triangle inequality caps it at 1).
-//   phase[k−1] radians [−π,π] on the pc circle (atan2 convention);
-//              origin = the PUBLISHED origin (D after the canvas dress).
-//
-// REST (declared here, held by the publisher): zero vector → mags 0;
-// phases HOLD-LAST per channel AT THE PUBLISH SITE — a consumer fading
-// on mag never sees phase snap. The pure fn itself returns phase 0 for
-// the zero vector; holding is the caller's stateful half (the same
-// entry-owned pattern as the held field).
-//
-// Depends on: musical/musical_ops.hpp (PitchClassVector) + <cmath>.
-
-#include <cmath>
-#include "musical/musical_ops.hpp"
-
-namespace t7 {
-
-struct PcDft {
-    float mag[6];     // families f1..f6, L1-normalized [0,1]
-    float phase[6];   // radians [−π,π]; 0 for the zero vector
-};
-
-// The 12-point real DFT, families k = 1..6 (k=0 is the L1 itself; k>6
-// mirrors by conjugate symmetry). X_k = Σ_n v[n]·e^(−i·2πkn/12).
-inline PcDft pc_dft(const PitchClassVector& v) {
-    PcDft out{};
-    float l1 = 0.0f;
-    for (int n = 0; n < 12; ++n)
-        l1 += (v.v[n] < 0.0f ? -v.v[n] : v.v[n]);
-    constexpr float TAU = 6.28318530717958647692f;
-    for (int k = 1; k <= 6; ++k) {
-        float re = 0.0f, im = 0.0f;
-        for (int n = 0; n < 12; ++n) {
-            const float a = TAU * (float)(k * n) / 12.0f;
-            re += v.v[n] * std::cos(a);
-            im -= v.v[n] * std::sin(a);
-        }
-        out.mag[k - 1]   = (l1 > 0.0f) ? std::sqrt(re * re + im * im) / l1 : 0.0f;
-        out.phase[k - 1] = (l1 > 0.0f) ? std::atan2(im, re) : 0.0f;
-    }
-    return out;
-}
-
-} // namespace t7
```

#### D-1.15 — `src/musical/playhead.hpp`

Blob at `1a52f2db^`: `3172fe6cb587d3deed968db71611d64d39eb44af` · deleted lines: 179 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/playhead.hpp b/src/musical/playhead.hpp
deleted file mode 100644
index 3172fe6c..00000000
--- a/src/musical/playhead.hpp
+++ /dev/null
@@ -1,179 +0,0 @@
-#pragma once
-
-// ─── playhead.hpp ────────────────────────────────────────────────
-//
-// Present-only context. The Playhead reads the active set at the anchor
-// and reports the present: which notes sound now, whether there is signal
-// or silence, what changed this frame, and how long the present state has
-// held. It holds NO memory of the past — the previous note lives in its own
-// object (previous_event.hpp), on a different clock.
-//
-// What it reads: the stream snapshot (active notes) only. No history. The
-// anchor is the snapshot beat — the live present.
-//
-// Reworked against target spec §2. Removed from the prior version: the
-// PREVIOUS set, gap_duration, the offset / chord-tolerance machinery, and
-// the history dependency. Those concerns moved out (previous_event.hpp) or
-// retired with the offset-playhead generalization.
-//
-// Depends on: musical/stream_data.hpp (StreamSnapshot, ActiveNote,
-//             PitchBitmask), <array>, <cstdint>.
-
-#include "musical/stream_data.hpp"
-#include <array>
-#include <cstdint>
-
-namespace t7 {
-
-constexpr int PLAYHEAD_MAX_POLYPHONY = 16;
-
-// ═══ CURRENT NOTE ════════════════════════════════════════════════
-//
-// A note sounding at the anchor. onset_beat is kept so an operation can
-// take the note's provisional in-window length (anchor − onset) — the
-// active contribution the combine sums against the Wagon's completed part.
-
-struct CurrentNote {
-    uint8_t pitch = 0;
-    uint8_t _pad[3] = {0, 0, 0};
-    float   velocity = 0.0f;
-    float   onset_beat = 0.0f;
-};
-
-static_assert(sizeof(CurrentNote) == 12, "CurrentNote should be 12 bytes");
-
-// ═══ PLAYHEAD READOUT ════════════════════════════════════════════
-//
-// Raw present data. No held memory, no derived folds.
-
-struct PlayheadReadout {
-    float anchor_beat = 0.0f;          // the present moment
-
-    // --- the present ---
-    std::array<CurrentNote, PLAYHEAD_MAX_POLYPHONY> current{};
-    int          current_count = 0;
-    PitchBitmask current_mask;
-    bool         current_overflow = false;   // more than MAX_POLYPHONY notes
-
-    // --- transitions this frame (the synchronicity edges) ---
-    PitchBitmask onset_mask;
-    PitchBitmask release_mask;
-    int          onset_count = 0;
-    int          release_count = 0;
-
-    // --- gate state ---
-    bool  is_onset = false;            // became non-silent this frame
-    bool  is_release = false;          // became silent this frame
-    float state_duration = 0.0f;       // how long the present state has held
-
-    bool gate()        const { return current_count > 0; }
-    bool silent()      const { return current_count == 0; }
-    bool has_overflow() const { return current_overflow; }
-
-    // Provisional length of an active note at the anchor.
-    float current_duration(int i) const {
-        return (i >= 0 && i < current_count)
-                   ? anchor_beat - current[i].onset_beat
-                   : 0.0f;
-    }
-
-    void clear() {
-        anchor_beat = 0.0f;
-        current_count = 0;
-        current_mask.clear_all();
-        current_overflow = false;
-        onset_mask.clear_all();
-        release_mask.clear_all();
-        onset_count = release_count = 0;
-        is_onset = is_release = false;
-        state_duration = 0.0f;
-    }
-};
-
-// ═══ PLAYHEAD ════════════════════════════════════════════════════
-
-class Playhead {
-public:
-    Playhead() {
-        readout_.clear();
-        prev_frame_mask_.clear_all();
-    }
-
-    // Update from the present snapshot. Reads active notes only.
-    void update(const StreamSnapshot& snap) {
-        const PitchBitmask prev_mask = prev_frame_mask_;
-        const bool was_gate = readout_.gate();
-
-        rebuild_current(snap);
-        detect_transitions(prev_mask);
-        update_temporal(was_gate);
-
-        prev_frame_mask_ = readout_.current_mask;
-    }
-
-    const PlayheadReadout& readout() const { return readout_; }
-
-    // Reset on a stream discontinuity (seek / state clear).
-    void clear() {
-        readout_.clear();
-        prev_frame_mask_.clear_all();
-        state_onset_beat_ = 0.0f;
-    }
-
-private:
-    PlayheadReadout readout_;
-    PitchBitmask    prev_frame_mask_;
-    float           state_onset_beat_ = 0.0f;
-
-    void rebuild_current(const StreamSnapshot& snap) {
-        readout_.anchor_beat = snap.beat;
-        readout_.current_count = 0;
-        readout_.current_mask.clear_all();
-        readout_.current_overflow = false;
-
-        snap.for_each_active([&](int pitch, const ActiveNote& note) {
-            if (readout_.current_count >= PLAYHEAD_MAX_POLYPHONY) {
-                readout_.current_overflow = true;
-                return;
-            }
-            CurrentNote& cn = readout_.current[readout_.current_count];
-            cn.pitch      = static_cast<uint8_t>(pitch);
-            cn.velocity   = note.velocity;
-            cn.onset_beat = note.onset_beat;
-            readout_.current_mask.set(pitch);
-            ++readout_.current_count;
-        });
-    }
-
-    void detect_transitions(const PitchBitmask& prev_mask) {
-        readout_.onset_mask.clear_all();
-        readout_.release_mask.clear_all();
-        readout_.onset_count = 0;
-        readout_.release_count = 0;
-
-        readout_.current_mask.for_each([&](int pitch) {
-            if (!prev_mask.test(pitch)) {
-                readout_.onset_mask.set(pitch);
-                ++readout_.onset_count;
-            }
-        });
-        prev_mask.for_each([&](int pitch) {
-            if (!readout_.current_mask.test(pitch)) {
-                readout_.release_mask.set(pitch);
-                ++readout_.release_count;
-            }
-        });
-    }
-
-    void update_temporal(bool was_gate) {
-        const bool now_gate = readout_.gate();
-        readout_.is_onset   = !was_gate &&  now_gate;
-        readout_.is_release =  was_gate && !now_gate;
-        if (was_gate != now_gate) {
-            state_onset_beat_ = readout_.anchor_beat;
-        }
-        readout_.state_duration = readout_.anchor_beat - state_onset_beat_;
-    }
-};
-
-} // namespace t7
```

#### D-1.16 — `src/musical/previous_event.hpp`

Blob at `1a52f2db^`: `b97550ce0566bb351c36f2dfa8eeead803bde808` · deleted lines: 194 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/previous_event.hpp b/src/musical/previous_event.hpp
deleted file mode 100644
index b97550ce..00000000
--- a/src/musical/previous_event.hpp
+++ /dev/null
@@ -1,194 +0,0 @@
-#pragma once
-
-// ─── previous_event.hpp ──────────────────────────────────────────
-//
-// Holds the two most recent onset-groups: the open one being struck now —
-// the CURRENT event — and the prior one — the PREVIOUS event — latched. A
-// latch, not a window: the previous survives any silence. Fed onset events;
-// notes struck within a simultaneity tolerance join the open group; when a
-// new group opens beyond tolerance, the open group becomes previous and is
-// held until the next opens.
-//
-// An event is defined by ONSET: notes struck together are one group, and the
-// grouping never consults release. But the previous event's REPRESENTATIVE —
-// the single note that speaks for the group in a comparison — is the voice
-// that goes offset LAST, the one that lingered to the seam. So onset groups;
-// offset elects. Releases are recorded (on_offset) for that election; a
-// still-sounding voice counts as the latest possible offset, so a held note
-// outranks any that has already let go. The minimal-motion closest_pitch
-// remains as an alternate criterion.
-//
-// This is the fast instance of the held-value-surviving-silence primitive;
-// the reign is the slow instance.
-//
-// Times are assumed monotonic; on a discontinuity the caller calls clear().
-//
-// Depends on: <array>, <cstdint>, <limits>.
-
-#include <array>
-#include <cstdint>
-#include <limits>
-
-namespace t7 {
-
-constexpr int   PREVIOUS_GROUP_MAX = 16;
-
-// A still-sounding note ranks as the latest possible offset, so the
-// last-to-offset election prefers a voice that is still being held.
-constexpr float PREV_SOUNDING = std::numeric_limits<float>::infinity();
-
-// ═══ PREVIOUS NOTE ═══════════════════════════════════════════════
-//
-// A note as latched at its onset, with its release recorded when it comes:
-// offset stays PREV_SOUNDING until the note goes silent. The pair (onset,
-// offset) is what the last-to-offset election reads.
-
-struct PrevNote {
-    uint8_t pitch = 0;
-    uint8_t _pad[3] = {0, 0, 0};
-    float   velocity   = 0.0f;
-    float   onset_beat = 0.0f;
-    float   offset     = PREV_SOUNDING;   // release time; PREV_SOUNDING while still on
-};
-
-static_assert(sizeof(PrevNote) == 16, "PrevNote should be 16 bytes");
-
-// ═══ PREVIOUS EVENT ══════════════════════════════════════════════
-
-class PreviousEvent {
-public:
-    explicit PreviousEvent(float tolerance = 0.1f) : tolerance_(tolerance) {}
-
-    void  set_tolerance(float beats) { tolerance_ = beats; }
-    float tolerance() const { return tolerance_; }
-
-    // Feed a note onset. Runs the grouping state machine; may latch the open
-    // group as "previous".
-    void on_onset(int pitch, float velocity, float beat) {
-        if (open_count_ == 0) {
-            open_onset_ = beat;
-            push_open(pitch, velocity, beat);
-        } else if (beat - open_onset_ <= tolerance_) {
-            push_open(pitch, velocity, beat);          // same group
-        } else {
-            latch();                                   // open group -> previous
-            open_onset_ = beat;
-            push_open(pitch, velocity, beat);          // start a new group
-        }
-    }
-
-    // Feed a note release. Records the offset on the matching still-sounding
-    // note — current group first, then previous — so the previous event can
-    // elect the voice that goes offset last.
-    void on_offset(int pitch, float time) {
-        for (int i = 0; i < open_count_; ++i)
-            if (open_[i].pitch == pitch && open_[i].offset == PREV_SOUNDING) { open_[i].offset = time; return; }
-        for (int i = 0; i < previous_count_; ++i)
-            if (previous_[i].pitch == pitch && previous_[i].offset == PREV_SOUNDING) { previous_[i].offset = time; return; }
-    }
-
-    // --- the previous group (held) ---
-
-    bool            has_previous() const { return previous_count_ > 0; }
-    int             previous_count() const { return previous_count_; }
-    bool            previous_overflow() const { return previous_overflow_; }
-    const PrevNote& previous_note(int i) const { return previous_[i]; }
-    float           previous_onset() const { return previous_onset_; }
-
-    // The previous member that goes offset last — the voice that lingered to
-    // the seam (a still-sounding member counts as latest). Tie -> higher pitch.
-    // Returns -1 if there is no previous. This is the previous event's chosen
-    // representative; the signed interval to a current note is a musical_op
-    // built on its pitch.
-    int previous_last_offset_index() const {
-        if (previous_count_ == 0) return -1;
-        int best = 0;
-        for (int i = 1; i < previous_count_; ++i) {
-            const float oi = previous_[i].offset, ob = previous_[best].offset;
-            if (oi > ob || (oi == ob && previous_[i].pitch > previous_[best].pitch)) best = i;
-        }
-        return best;
-    }
-    int previous_last_offset_pitch() const {
-        const int i = previous_last_offset_index();
-        return i < 0 ? -1 : previous_[i].pitch;
-    }
-
-    // --- the current group (open): the cluster being struck now ---
-
-    bool            has_open() const { return open_count_ > 0; }
-    int             open_count() const { return open_count_; }
-    bool            open_overflow() const { return open_overflow_; }
-    const PrevNote& open_note(int i) const { return open_[i]; }
-    float           open_onset() const { return open_onset_; }
-
-    // Time since the previous group was struck. Caller checks has_previous().
-    float temporal_distance(float anchor) const {
-        return has_previous() ? anchor - previous_onset_ : 0.0f;
-    }
-
-    // The previous member nearest `target` (minimal motion). Tie -> higher.
-    // Returns -1 if there is no previous. An alternate criterion to the
-    // last-to-offset election above.
-    int closest_pitch(int target) const {
-        if (previous_count_ == 0) return -1;
-        int best      = previous_[0].pitch;
-        int best_dist = iabs(int(previous_[0].pitch) - target);
-        for (int i = 1; i < previous_count_; ++i) {
-            const int p = previous_[i].pitch;
-            const int d = iabs(p - target);
-            if (d < best_dist || (d == best_dist && p > best)) {
-                best      = p;
-                best_dist = d;
-            }
-        }
-        return best;
-    }
-
-    // Reset on a stream discontinuity (seek / state clear).
-    void clear() {
-        open_count_        = 0;
-        open_overflow_     = false;
-        previous_count_    = 0;
-        previous_overflow_ = false;
-        previous_onset_    = 0.0f;
-    }
-
-private:
-    static int iabs(int x) { return x < 0 ? -x : x; }
-
-    void push_open(int pitch, float velocity, float beat) {
-        if (open_count_ >= PREVIOUS_GROUP_MAX) { open_overflow_ = true; return; }
-        PrevNote& n  = open_[open_count_];
-        n.pitch      = static_cast<uint8_t>(pitch);
-        n.velocity   = velocity;
-        n.onset_beat = beat;
-        n.offset     = PREV_SOUNDING;   // still sounding until released
-        ++open_count_;
-    }
-
-    void latch() {
-        previous_          = open_;
-        previous_count_    = open_count_;
-        previous_overflow_ = open_overflow_;
-        previous_onset_    = open_onset_;
-        open_count_        = 0;
-        open_overflow_     = false;
-    }
-
-    float tolerance_;
-
-    // open (in-progress) onset group
-    std::array<PrevNote, PREVIOUS_GROUP_MAX> open_{};
-    int   open_count_    = 0;
-    float open_onset_    = 0.0f;
-    bool  open_overflow_ = false;
-
-    // previous (held) onset group
-    std::array<PrevNote, PREVIOUS_GROUP_MAX> previous_{};
-    int   previous_count_    = 0;
-    float previous_onset_    = 0.0f;
-    bool  previous_overflow_ = false;
-};
-
-} // namespace t7
```

#### D-1.17 — `src/musical/spine.hpp`

Blob at `1a52f2db^`: `8e9c144193e49019195d3526ed08f3c27486fb53` · deleted lines: 271 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/spine.hpp b/src/musical/spine.hpp
deleted file mode 100644
index 8e9c1441..00000000
--- a/src/musical/spine.hpp
+++ /dev/null
@@ -1,271 +0,0 @@
-#pragma once
-
-// ─── spine.hpp ───────────────────────────────────────────────────
-//
-// The previous-note line and its election. A comparison needs two
-// terms; the previous term must be a single note, elected from the
-// previous event by a total order. Time gives a partial order; this
-// machine completes it, by the cascade law, in order of authority:
-//
-//   1  survived — the last note to go offset; the music performed it.
-//   2  minted   — the current term of a recorded succession; the
-//                 music stated it.
-//   3  chosen   — the oracle, an injected criterion; we declared it.
-//
-// Facts about "previous" are facts about the spine, and the spine
-// records them only at crossings through cardinality one:
-//
-//   1 → 2  mints succession: a note entering over exactly one sounding
-//          note ticks the line — the ground becomes its previous, the
-//          entrant becomes the holder. Entering over a chord mints
-//          nothing; synchronous onsets mint nothing (a chord entered,
-//          not a note) and void the mint they interrupt.
-//   2 → 1  is the handover: the survivor is handed the line. If the
-//          line already rests on the survivor, nothing moves.
-//
-// The vertical mints nothing; it only eliminates. At poly → 0 the
-// event ends and the election runs over the final offset cohort
-// (offsets within tolerance of the last): a sole survivor is elected
-// SURVIVED; a tied cohort elects the holder, MINTED, unless the
-// holder was installed by the dying tie itself; otherwise the oracle,
-// CHOSEN. The resolved value seeds the next event: a note entering
-// silence takes it as its previous, carrying its provenance.
-//
-// The first event has no previous: resolved is invalid until the
-// first election. Parked by decision, not oversight.
-//
-// Clock contract: the mechanism is agnostic — it asks only for a
-// monotonic clock, with tolerance in the same units. The interval probe
-// fed it arrival seconds; owned by a Context it is fed the beat clock
-// with everything else, and its grouping is musical. Same contract as
-// PreviousEvent.
-//
-// Depends on: <array>, <cstdint>.
-
-#include <array>
-#include <cstdint>
-
-namespace t7 {
-
-    // ═══ PROVENANCE ══════════════════════════════════════════════════
-
-    enum class Provenance : uint8_t {
-        None,       // no election yet
-        Survived,   // the music performed it (sole last offset)
-        Minted,     // the music stated it (recorded succession)
-        Chosen      // we declared it (the oracle)
-    };
-
-    inline const char* provenance_name(Provenance p) {
-        switch (p) {
-        case Provenance::Survived: return "survived";
-        case Provenance::Minted:   return "minted";
-        case Provenance::Chosen:   return "chosen";
-        default:                   return "none";
-        }
-    }
-
-    // ═══ SPINE ═══════════════════════════════════════════════════════
-
-    class Spine {
-    public:
-        static constexpr int MAX_SOUNDING = 16;
-        static constexpr int MAX_COHORT = 16;
-
-        // The oracle: completes the order when time is silent. Receives
-        // the tied pitches; returns the elected one. Lowest by default —
-        // one binding of the slot, not the slot.
-        using Oracle = int (*)(const int* pitches, int count);
-
-        static int oracle_lowest(const int* pitches, int count) {
-            int best = pitches[0];
-            for (int i = 1; i < count; ++i)
-                if (pitches[i] < best) best = pitches[i];
-            return best;
-        }
-
-        explicit Spine(float tolerance = 0.05f)
-            : tolerance_(tolerance), oracle_(&oracle_lowest) {
-        }
-
-        void  set_tolerance(float seconds) { tolerance_ = seconds; }
-        float tolerance() const { return tolerance_; }
-        void  set_oracle(Oracle o) { if (o) oracle_ = o; }
-
-        // ── Feed ─────────────────────────────────────────────────────
-
-        /**
-         * A note begins sounding at time t (grouping clock, seconds).
-         */
-        void on_onset(int pitch, float t) {
-            // Retrigger of a sounding pitch: refresh its onset, no crossing.
-            for (int i = 0; i < n_sounding_; ++i) {
-                if (sounding_[i].pitch == pitch) {
-                    sounding_[i].onset = t;
-                    return;
-                }
-            }
-
-            const int n = n_sounding_;
-
-            if (n == 0) {
-                // Entering silence: the resolved election is the previous.
-                if (resolved_.valid) {
-                    prev_ = LineNote{ resolved_.pitch, resolved_.prov, true };
-                }
-                else {
-                    prev_.valid = false;   // first event — no previous (parked)
-                }
-                holder_ = Holder{ pitch, t, true };
-            }
-            else if (n == 1) {
-                if (holder_.valid && (t - holder_.set_t) <= tolerance_) {
-                    // Synchronous with a just-installed holder: a chord is
-                    // being born, not a note arriving. The line is illegible.
-                    holder_.valid = false;
-                    prev_.valid = false;
-                }
-                else if (holder_.valid) {
-                    // MINT at 1 → 2: ground becomes previous, entrant holds.
-                    stash_holder_ = holder_;
-                    stash_prev_ = prev_;
-                    prev_ = LineNote{ holder_.pitch, Provenance::Minted, true };
-                    holder_ = Holder{ pitch, t, true };
-                }
-                // holder invalid at n==1 cannot persist (handover re-arms it),
-                // but if it occurs, the vertical rule applies: mint nothing.
-            }
-            else {
-                // n >= 2: the vertical mints nothing. One correction only:
-                // a partner arriving within tolerance of a fresh mint voids
-                // it — the "entrant" was a chord. Revert to the pre-mint line.
-                if (holder_.valid && (t - holder_.set_t) <= tolerance_) {
-                    holder_ = stash_holder_;
-                    prev_ = stash_prev_;
-                }
-            }
-
-            if (n_sounding_ < MAX_SOUNDING) {
-                sounding_[n_sounding_++] = SoundingNote{ pitch, t };
-            }
-        }
-
-        /**
-         * A note stops sounding at time t (grouping clock, seconds).
-         */
-        void on_offset(int pitch, float t) {
-            // Remove from the sounding set (swap with last).
-            int idx = -1;
-            for (int i = 0; i < n_sounding_; ++i) {
-                if (sounding_[i].pitch == pitch) { idx = i; break; }
-            }
-            if (idx < 0) return;   // offset of a note we never saw — ignore
-            sounding_[idx] = sounding_[--n_sounding_];
-
-            // The final-offset cohort: pitches whose offsets tie within
-            // tolerance of the latest. An offset beyond tolerance resets it.
-            if (n_cohort_ > 0 && (t - last_offset_t_) <= tolerance_) {
-                if (n_cohort_ < MAX_COHORT) cohort_[n_cohort_++] = pitch;
-            }
-            else {
-                cohort_[0] = pitch;
-                n_cohort_ = 1;
-            }
-            last_offset_t_ = t;
-
-            if (n_sounding_ == 1) {
-                // HANDOVER at 2 → 1: the survivor is handed the line.
-                // If the line already rests on it, nothing moves — the
-                // holder keeps its original installation time.
-                const int survivor = sounding_[0].pitch;
-                if (!(holder_.valid && holder_.pitch == survivor)) {
-                    holder_ = Holder{ survivor, t, true };
-                    prev_.valid = false;
-                }
-            }
-            else if (n_sounding_ == 0) {
-                elect();
-            }
-        }
-
-        /**
-         * Forget everything, including the resolved election.
-         */
-        void clear() {
-            n_sounding_ = 0;
-            n_cohort_ = 0;
-            holder_.valid = prev_.valid = resolved_.valid = false;
-            stash_holder_.valid = stash_prev_.valid = false;
-        }
-
-        // ── The line (live) ──────────────────────────────────────────
-
-        bool line_legible() const { return holder_.valid && n_sounding_ > 0; }
-        int  line_holder() const { return holder_.pitch; }
-        bool has_line_previous() const { return prev_.valid; }
-        int  line_previous() const { return prev_.pitch; }
-        Provenance line_previous_provenance() const { return prev_.prov; }
-
-        // ── The election (resolved) ──────────────────────────────────
-
-        bool       has_resolved() const { return resolved_.valid; }
-        int        resolved_pitch() const { return resolved_.pitch; }
-        Provenance resolved_provenance() const { return resolved_.prov; }
-
-        // ── The index ────────────────────────────────────────────────
-
-        int sounding_count() const { return n_sounding_; }
-
-    private:
-        struct SoundingNote { int pitch; float onset; };
-        struct Holder { int pitch = 0; float set_t = 0.0f; bool valid = false; };
-        struct LineNote { int pitch = 0; Provenance prov = Provenance::None; bool valid = false; };
-
-        float  tolerance_;
-        Oracle oracle_;
-
-        std::array<SoundingNote, MAX_SOUNDING> sounding_{};
-        int n_sounding_ = 0;
-
-        std::array<int, MAX_COHORT> cohort_{};
-        int   n_cohort_ = 0;
-        float last_offset_t_ = 0.0f;
-
-        Holder   holder_;
-        LineNote prev_;
-        Holder   stash_holder_;     // pre-mint line, for voided mints
-        LineNote stash_prev_;
-        LineNote resolved_;         // the elected previous event
-
-        // ── The cascade, run at poly → 0 ─────────────────────────────
-
-        void elect() {
-            if (n_cohort_ == 1) {
-                // Key 1 — survived: a sole last offset; the music selected.
-                resolved_ = LineNote{ cohort_[0], Provenance::Survived, true };
-            }
-            else {
-                // Key 2 — minted: the holder, if it is among the tied
-                // survivors and was not installed by the dying tie itself.
-                bool holder_in_tie = false;
-                if (holder_.valid && (last_offset_t_ - holder_.set_t) > tolerance_) {
-                    for (int i = 0; i < n_cohort_; ++i)
-                        if (cohort_[i] == holder_.pitch) { holder_in_tie = true; break; }
-                }
-                if (holder_in_tie) {
-                    resolved_ = LineNote{ holder_.pitch, Provenance::Minted, true };
-                }
-                else {
-                    // Key 3 — chosen: time said nothing; the oracle completes.
-                    resolved_ = LineNote{ oracle_(cohort_.data(), n_cohort_),
-                                         Provenance::Chosen, true };
-                }
-            }
-
-            holder_.valid = prev_.valid = false;
-            stash_holder_.valid = stash_prev_.valid = false;
-            n_cohort_ = 0;
-        }
-    };
-
-} // namespace t7
\ No newline at end of file
```

#### D-1.18 — `src/musical/spine_ops.hpp`

Blob at `1a52f2db^`: `c33c885fb9fbd6b76d348f768a358d6066cc59b7` · deleted lines: 52 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/spine_ops.hpp b/src/musical/spine_ops.hpp
deleted file mode 100644
index c33c885f..00000000
--- a/src/musical/spine_ops.hpp
+++ /dev/null
@@ -1,52 +0,0 @@
-#pragma once
-
-// ─── spine_ops.hpp ───────────────────────────────────────────────
-//
-// Canvas operations over the spine's line — the two readings the packet
-// takes from it. The spine elects the line (a current note and the previous
-// it descended from); these turn that line into the forms the canvas ships.
-//
-//   current_note  — the holder as a one-hot pitch-class vector. Which note
-//                   currently holds the line, octave-agnostic. Raw, like the
-//                   counts: the per-vector dressing (re-origin, scale) is
-//                   applied at the sink, the same for every vector.
-//   line_distance — the signed registral semitones from the previous note to
-//                   the current one (holder − previous): how far the line
-//                   just moved, with direction.
-//
-// Both fall quiet when the line is illegible — an empty vector, a zero
-// distance — so a vertical with no single line, or silence, reads as nothing
-// rather than as a stale value.
-//
-// distance(from, to), the general two-note interval line_distance is built
-// from, now lives in musical_ops beside pc_relative_to — the spine is one
-// supplier of the pair, not the only one, so the interval belongs on the
-// floor rather than here.
-//
-// Depends on: musical/musical_ops.hpp (PitchClassVector, pc_of, distance),
-//             musical/spine.hpp (the line).
-
-#include "musical/musical_ops.hpp"
-#include "musical/spine.hpp"
-
-namespace t7 {
-
-// The current note as a one-hot at the holder's pitch class. Empty when the
-// line is illegible. Raw — dressing is applied at the sink, with the rest.
-inline PitchClassVector current_note(const Spine& spine) {
-    PitchClassVector v;
-    if (spine.line_legible())
-        v[pc_of(spine.line_holder())] = 1.0f;
-    return v;
-}
-
-// How far the line just moved: holder minus previous, signed and registral.
-// Zero when the line is illegible or has no previous (a unison is zero of its
-// own accord, since the two pitches are equal).
-inline int line_distance(const Spine& spine) {
-    if (spine.line_legible() && spine.has_line_previous())
-        return distance(spine.line_previous(), spine.line_holder());
-    return 0;
-}
-
-} // namespace t7
```

#### D-1.19 — `src/musical/vector_dressing.hpp`

Blob at `1a52f2db^`: `b220ae3e019470beb3cfb3105c80492a1c37df1a` · deleted lines: 54 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/vector_dressing.hpp b/src/musical/vector_dressing.hpp
deleted file mode 100644
index b220ae3e..00000000
--- a/src/musical/vector_dressing.hpp
+++ /dev/null
@@ -1,54 +0,0 @@
-#pragma once
-
-// ─── vector_dressing.hpp ─────────────────────────────────────────
-//
-// The dressing a published pitch-class vector carries: an optional
-// re-origin and an optional scale, applied as the last step before the
-// vector reaches a sink. It is terminal — it shapes a vector for output,
-// it never produces one. A measure makes the vector; this dresses it.
-//
-// Both parts are per-vector and opt-in, because the choice belongs to a
-// representation, not to the music. The re-origin rotates the vector so a
-// chosen root lands on bin zero, the degrees rising above it; off, or at
-// root C, it leaves the vector absolute — the two are the same, since C is
-// the identity rotation. The scale either ships the raw weights, rescales
-// them to a distribution summing to one, or rescales them to unit length.
-//
-// One re-origin. The rotation here is `to_degrees`, the vector form of
-// `pc_relative_to`; both now live in musical_ops, so dressing a vector
-// depends on the floor alone and no longer reaches into the field header.
-//
-// Depends on: musical/musical_ops.hpp (PitchClassVector, to_degrees).
-
-#include "musical/musical_ops.hpp"
-
-namespace t7 {
-
-// The dressing carried by one published vector.
-struct VectorDressing {
-    // Re-origin. Off (or root C) leaves the vector absolute; on rotates so
-    // `root` lands on bin 0. Root is a pitch class, C = 0 the default.
-    bool reorigin = false;
-    int  root     = 0;
-
-    // Scale. None ships the raw weights; Normalized rescales to unit sum
-    // (a distribution); Unit rescales to unit L2 length.
-    enum class Scale { None, Normalized, Unit };
-    Scale scale = Scale::None;
-};
-
-// Apply a vector's dressing: re-origin, then scale. The scalers are
-// rotation-invariant — sum and length are unchanged by a permutation of
-// bins — so the order does not change the result; re-origin then scale just
-// reads as representation before magnitude.
-inline PitchClassVector dress(const PitchClassVector& v, const VectorDressing& d) {
-    PitchClassVector out = d.reorigin ? to_degrees(v, d.root) : v;
-    switch (d.scale) {
-        case VectorDressing::Scale::Normalized: return out.normalized();
-        case VectorDressing::Scale::Unit:       return out.unit();
-        case VectorDressing::Scale::None:       return out;
-    }
-    return out;  // unreachable; quiets the compiler
-}
-
-} // namespace t7
```

#### D-1.20 — `src/musical/wagon.hpp`

Blob at `1a52f2db^`: `f7b091df7971971d53c2796ea61f4554e8952205` · deleted lines: 164 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/musical/wagon.hpp b/src/musical/wagon.hpp
deleted file mode 100644
index f7b091df..00000000
--- a/src/musical/wagon.hpp
+++ /dev/null
@@ -1,164 +0,0 @@
-#pragma once
-
-// ─── wagon.hpp ───────────────────────────────────────────────────
-//
-// Completed-pairs-within-a-span context. The Wagon registers a temporary
-// history of finished notes over a window [anchor − span, anchor], each
-// clipped to the window so its in-window span is exact. Completed-only —
-// a note is either active or completed, never both, so the Wagon (past)
-// and the Playhead (present) are disjoint and their contributions add
-// without double-counting.
-//
-// The active contribution to any windowed read comes from the Playhead,
-// summed at the combine — NOT from the Wagon. There is no include_active
-// path here: reaching into active notes would double-count against the
-// Playhead and break the additive separation.
-//
-// Straddling completed notes (started before the window, ended inside) are
-// the Wagon's job: they are included and clipped, which is what lets held
-// notes that have since ended register their in-window length. A note still
-// sounding is active, and belongs to the Playhead, not here.
-//
-// Reworked against target spec §6. Removed from the prior version:
-// include_active (the trap), include_straddling (overlap + clip is now the
-// single behaviour; inside-only, if ever wanted, is a downstream filter),
-// and update_period (rebuild every frame — cheap, and no stale readout to
-// complicate the sync point).
-//
-// Depends on: musical/stream_data.hpp (CompletedRing, CompletedNote),
-//             <array>, <cstdint>.
-
-#include "musical/stream_data.hpp"
-#include <array>
-#include <cstdint>
-
-namespace t7 {
-
-constexpr int WAGON_MAX_NOTES = 128;
-
-// ═══ WINDOW NOTE ═════════════════════════════════════════════════
-//
-// A completed note seen through the window. window_onset / window_offset
-// are the original times clipped to the window bounds; window_duration is
-// the exact in-window span that length-weighted reads use.
-
-struct WindowNote {
-    uint8_t pitch = 0;
-    uint8_t _pad[3] = {0, 0, 0};
-    float   velocity = 0.0f;
-    float   onset_beat = 0.0f;
-    float   offset_beat = 0.0f;
-    float   window_onset = 0.0f;     // max(onset_beat, window_start)
-    float   window_offset = 0.0f;    // min(offset_beat, window_end)
-
-    float duration() const { return offset_beat - onset_beat; }
-    float window_duration() const { return window_offset - window_onset; }
-    int   pitch_class() const { return pitch % 12; }
-    int   octave() const { return pitch / 12; }
-
-    bool straddles_start() const { return window_onset  > onset_beat; }
-    bool straddles_end()   const { return window_offset < offset_beat; }
-    bool entirely_inside() const { return !straddles_start() && !straddles_end(); }
-};
-
-static_assert(sizeof(WindowNote) == 24, "WindowNote should be 24 bytes");
-
-// ═══ WAGON READOUT ═══════════════════════════════════════════════
-
-struct WagonReadout {
-    float anchor_beat = 0.0f;    // window end  (current_beat − offset)
-    float window_start = 0.0f;   // anchor − span
-    float window_end = 0.0f;     // anchor
-    float span = 0.0f;
-    float offset = 0.0f;
-
-    std::array<WindowNote, WAGON_MAX_NOTES> notes{};
-    int  note_count = 0;
-    bool overflow = false;       // more than WAGON_MAX_NOTES in the window
-    int  entirely_inside_count = 0;
-    int  straddling_count = 0;
-
-    bool empty() const { return note_count == 0; }
-    bool has_notes() const { return note_count > 0; }
-    bool has_overflow() const { return overflow; }
-
-    void clear() {
-        anchor_beat = window_start = window_end = span = offset = 0.0f;
-        note_count = 0;
-        overflow = false;
-        entirely_inside_count = straddling_count = 0;
-    }
-};
-
-// ═══ WAGON ═══════════════════════════════════════════════════════
-
-class Wagon {
-public:
-    Wagon() { readout_.clear(); }
-
-    explicit Wagon(float span, float offset = 0.0f)
-        : span_(span), offset_(offset) {
-        readout_.clear();
-    }
-
-    void  set_span(float beats) { span_ = beats; }
-    float span() const { return span_; }
-
-    void  set_offset(float beats) { offset_ = beats; }
-    float offset() const { return offset_; }
-
-    // Rebuild the window from history. Reads completed notes only.
-    void update(const CompletedRing& history, float current_beat) {
-        const float anchor = current_beat - offset_;
-        rebuild(history, anchor);
-    }
-
-    const WagonReadout& readout() const { return readout_; }
-
-    void clear() { readout_.clear(); }
-
-private:
-    float span_ = 0.0f;
-    float offset_ = 0.0f;
-    WagonReadout readout_;
-
-    void rebuild(const CompletedRing& history, float anchor) {
-        const float ws = anchor - span_;
-        const float we = anchor;
-
-        readout_.anchor_beat = anchor;
-        readout_.window_start = ws;
-        readout_.window_end = we;
-        readout_.span = span_;
-        readout_.offset = offset_;
-        readout_.note_count = 0;
-        readout_.overflow = false;
-        readout_.entirely_inside_count = 0;
-        readout_.straddling_count = 0;
-
-        history.for_each_overlapping_window(ws, we, [&](const CompletedNote& cn) {
-            add_note(cn, ws, we);
-        });
-    }
-
-    void add_note(const CompletedNote& cn, float ws, float we) {
-        if (readout_.note_count >= WAGON_MAX_NOTES) {
-            readout_.overflow = true;
-            return;
-        }
-        WindowNote& wn   = readout_.notes[readout_.note_count];
-        wn.pitch         = cn.pitch;
-        wn.velocity      = cn.velocity;
-        wn.onset_beat    = cn.onset_beat;
-        wn.offset_beat   = cn.offset_beat;
-        wn.window_onset  = cn.onset_beat  < ws ? ws : cn.onset_beat;
-        wn.window_offset = cn.offset_beat > we ? we : cn.offset_beat;
-
-        if (wn.straddles_start() || wn.straddles_end()) ++readout_.straddling_count;
-        else                                            ++readout_.entirely_inside_count;
-
-        ++readout_.note_count;
-    }
-};
-
-} // namespace t7
```

#### D-1.21 — `src/sources/keyboard_midi.hpp`

Blob at `1a52f2db^`: `3416fa83a9f300fd14375bf62977cebe01915f06` · deleted lines: 257 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/sources/keyboard_midi.hpp b/src/sources/keyboard_midi.hpp
deleted file mode 100644
index 3416fa83..00000000
--- a/src/sources/keyboard_midi.hpp
+++ /dev/null
@@ -1,257 +0,0 @@
-#pragma once
-
-// ─── keyboard_midi.hpp ───────────────────────────────────────────
-//
-// Computer keyboard to MIDI events: converts computer keyboard input to
-// MIDI-style note events. Events are queued internally and retrieved via
-// poll().
-//
-// Inert construction: no router coupling — events are queued and
-// retrieved by the caller, which enables fixed storage and visible
-// control flow. Data structures are fixed-size arrays indexed by ASCII
-// character (256 entries); zero allocation after initialization.
-//
-// Layout (US QWERTY):
-//     Black: W  E     T  Y  U     O  P
-//           C# D#    F# G# A#    C# D#
-//     White: A  S  D  F  G  H  J  K  L  ;
-//            C4 D4 E4 F4 G4 A4 B4 C5 D5 E5
-//     Lower: Z  X  C  V  B  N  M
-//            C3 D3 E3 F3 G3 A3 B3
-//     Controls: [ ] = Octave shift down/up
-//
-// Usage:
-//   KeyboardMidi keyboard(2, 100);  // channel 2, velocity 100
-//   keyboard.on_key_press('A', current_beat);   // in key callback
-//   keyboard.on_key_release('A', current_beat);
-//   MidiEvent events[32];
-//   int count = keyboard.poll(events, 32);       // each frame
-//   for (int i = 0; i < count; ++i)
-//       streams[events[i].channel].receive(events[i]);
-//
-// Depends on: sources/midi_event.hpp.
-
-#include "sources/midi_event.hpp"
-#include <array>
-#include <string>
-#include <cctype>
-#include <algorithm>
-
-namespace t7 {
-
-// ═══ KEYBOARD MIDI ═══════════════════════════════════════════════
-
-class KeyboardMidi {
-public:
-    /**
-     * Construct keyboard MIDI controller.
-     * 
-     * @param channel  MIDI channel (0-15)
-     * @param velocity Default velocity (1-127)
-     */
-    KeyboardMidi(int channel = 0, int velocity = 100)
-        : channel_(channel)
-        , velocity_(std::clamp(velocity, 1, 127))
-    {
-        key_to_note_.fill(-1);   // -1 = not mapped
-        held_note_.fill(-1);     // -1 = not held
-        pending_count_ = 0;
-        
-        init_piano_layout();
-    }
-    
-    // ── Configuration ────────────────────────────────────────────
-    
-    void set_channel(int channel) { channel_ = channel; }
-    int channel() const { return channel_; }
-    
-    void set_velocity(int v) { velocity_ = std::clamp(v, 1, 127); }
-    int velocity() const { return velocity_; }
-    
-    void set_octave_shift(int s) { octave_shift_ = std::clamp(s, -3, 3); }
-    int octave_shift() const { return octave_shift_; }
-    
-    // ── Key Events ───────────────────────────────────────────────
-    
-    /**
-     * Handle key press. Returns true if key was handled.
-     * Queues event internally - retrieve via poll().
-     */
-    bool on_key_press(char key, float current_beat) {
-        key = normalize_key(key);
-        uint8_t key_idx = static_cast<uint8_t>(key);
-        
-        // Octave shift controls
-        if (key == '[') {
-            octave_shift_ = std::max(-3, octave_shift_ - 1);
-            return true;
-        }
-        if (key == ']') {
-            octave_shift_ = std::min(3, octave_shift_ + 1);
-            return true;
-        }
-        
-        // Check if key is mapped
-        int base_note = key_to_note_[key_idx];
-        if (base_note < 0) return false;
-        
-        // Already held?
-        if (held_note_[key_idx] >= 0) return true;
-        
-        // Compute actual note with octave shift
-        int midi_note = std::clamp(base_note + octave_shift_ * 12, 0, 127);
-        
-        // Store held note and queue event
-        held_note_[key_idx] = static_cast<int8_t>(midi_note);
-        queue_event(MidiEvent::note_on(channel_, midi_note, velocity_ / 127.0f, current_beat));
-        
-        return true;
-    }
-    
-    /**
-     * Handle key release. Returns true if key was handled.
-     * Queues event internally - retrieve via poll().
-     */
-    bool on_key_release(char key, float current_beat) {
-        key = normalize_key(key);
-        uint8_t key_idx = static_cast<uint8_t>(key);
-        
-        // Get held note
-        int midi_note = held_note_[key_idx];
-        if (midi_note < 0) return false;
-        
-        // Clear held state and queue note off
-        held_note_[key_idx] = -1;
-        queue_event(MidiEvent::note_off(channel_, midi_note, current_beat));
-        
-        return true;
-    }
-    
-    /**
-     * Release all currently held notes.
-     */
-    void release_all(float current_beat) {
-        for (int i = 0; i < 256; ++i) {
-            if (held_note_[i] >= 0) {
-                queue_event(MidiEvent::note_off(channel_, held_note_[i], current_beat));
-                held_note_[i] = -1;
-            }
-        }
-    }
-    
-    // ── POLL - Retrieve pending events ───────────────────────────
-    
-    /**
-     * Retrieve pending events and clear the queue.
-     * 
-     * @param out     Output buffer for events
-     * @param max_out Maximum events to write
-     * @return Number of events written
-     */
-    int poll(MidiEvent* out, int max_out) {
-        int count = std::min(pending_count_, max_out);
-        
-        for (int i = 0; i < count; ++i) {
-            out[i] = pending_[i];
-        }
-        
-        // If we couldn't return all events, shift remaining to front
-        if (count < pending_count_) {
-            for (int i = count; i < pending_count_; ++i) {
-                pending_[i - count] = pending_[i];
-            }
-            pending_count_ -= count;
-        } else {
-            pending_count_ = 0;
-        }
-        
-        return count;
-    }
-    
-    /**
-     * Check how many events are pending.
-     */
-    int pending_count() const { return pending_count_; }
-    
-    // ── Accessors ────────────────────────────────────────────────
-    
-    int held_count() const {
-        int count = 0;
-        for (int i = 0; i < 256; ++i) {
-            if (held_note_[i] >= 0) count++;
-        }
-        return count;
-    }
-    
-private:
-    static constexpr int MAX_PENDING = 64;
-    
-    int channel_;
-    int velocity_;
-    int octave_shift_ = 0;
-    
-    // Fixed-size lookup tables (indexed by ASCII value)
-    std::array<int8_t, 256> key_to_note_;   // ASCII -> base MIDI note (-1 = unmapped)
-    std::array<int8_t, 256> held_note_;     // ASCII -> currently held note (-1 = not held)
-    
-    // Pending events queue
-    std::array<MidiEvent, MAX_PENDING> pending_;
-    int pending_count_ = 0;
-    
-    void queue_event(const MidiEvent& event) {
-        if (pending_count_ < MAX_PENDING) {
-            pending_[pending_count_++] = event;
-        }
-        // If queue is full, drop oldest events (shouldn't happen in normal use)
-    }
-    
-    static char normalize_key(char c) {
-        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
-    }
-    
-    void init_piano_layout() {
-        // White keys (home row): C4 - E5
-        key_to_note_[static_cast<uint8_t>('A')] = 60;  // C4
-        key_to_note_[static_cast<uint8_t>('S')] = 62;  // D4
-        key_to_note_[static_cast<uint8_t>('D')] = 64;  // E4
-        key_to_note_[static_cast<uint8_t>('F')] = 65;  // F4
-        key_to_note_[static_cast<uint8_t>('G')] = 67;  // G4
-        key_to_note_[static_cast<uint8_t>('H')] = 69;  // A4
-        key_to_note_[static_cast<uint8_t>('J')] = 71;  // B4
-        key_to_note_[static_cast<uint8_t>('K')] = 72;  // C5
-        key_to_note_[static_cast<uint8_t>('L')] = 74;  // D5
-        key_to_note_[static_cast<uint8_t>(';')] = 76;  // E5
-        
-        // Black keys (top row)
-        key_to_note_[static_cast<uint8_t>('W')] = 61;  // C#4
-        key_to_note_[static_cast<uint8_t>('E')] = 63;  // D#4
-        key_to_note_[static_cast<uint8_t>('T')] = 66;  // F#4
-        key_to_note_[static_cast<uint8_t>('Y')] = 68;  // G#4
-        key_to_note_[static_cast<uint8_t>('U')] = 70;  // A#4
-        key_to_note_[static_cast<uint8_t>('O')] = 73;  // C#5
-        key_to_note_[static_cast<uint8_t>('P')] = 75;  // D#5
-        
-        // Lower octave (Z row): C3 - B3
-        key_to_note_[static_cast<uint8_t>('Z')] = 48;  // C3
-        key_to_note_[static_cast<uint8_t>('X')] = 50;  // D3
-        key_to_note_[static_cast<uint8_t>('C')] = 52;  // E3
-        key_to_note_[static_cast<uint8_t>('V')] = 53;  // F3
-        key_to_note_[static_cast<uint8_t>('B')] = 55;  // G3
-        key_to_note_[static_cast<uint8_t>('N')] = 57;  // A3
-        key_to_note_[static_cast<uint8_t>('M')] = 59;  // B3
-    }
-};
-
-// ═══ UTILITY ═════════════════════════════════════════════════════
-
-inline std::string midi_note_name(int midi_note) {
-    static const char* names[] = {
-        "C", "C#", "D", "D#", "E", "F", 
-        "F#", "G", "G#", "A", "A#", "B"
-    };
-    int octave = (midi_note / 12) - 1;
-    int pc = midi_note % 12;
-    return std::string(names[pc]) + std::to_string(octave);
-}
-
-} // namespace t7
```

#### D-1.22 — `src/sources/midi_event.hpp`

Blob at `1a52f2db^`: `b7c41853c298a1ff111445b508d58f9940615830` · deleted lines: 59 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/sources/midi_event.hpp b/src/sources/midi_event.hpp
deleted file mode 100644
index b7c41853..00000000
--- a/src/sources/midi_event.hpp
+++ /dev/null
@@ -1,59 +0,0 @@
-#pragma once
-
-// ─── midi_event.hpp ──────────────────────────────────────────────
-//
-// Plain value type for event routing. A MidiEvent is a simple value
-// representing a note on or off. Events are produced by sources
-// (MidiFile, KeyboardMidi) and consumed by streams (MidiStream); this
-// decouples the concept of a note from the mechanism of delivery, so
-// events can be buffered, sorted, logged, or routed freely.
-//
-// Size: 12 bytes (fits in a register on 64-bit systems).
-//
-// Depends on: <cstdint>.
-
-#include <cstdint>
-
-namespace t7 {
-
-struct MidiEvent {
-    enum Type : uint8_t { 
-        NOTE_ON, 
-        NOTE_OFF 
-    };
-    
-    Type type;
-    uint8_t channel;
-    uint8_t pitch;
-    uint8_t _pad;
-    float velocity;  // 0-1 for NOTE_ON, ignored for NOTE_OFF
-    float beat;      // When this event occurred
-    
-    // ── Factory Methods ──────────────────────────────────────────
-    
-    static MidiEvent note_on(int channel, int pitch, float velocity, float beat) {
-        MidiEvent e;
-        e.type = NOTE_ON;
-        e.channel = static_cast<uint8_t>(channel);
-        e.pitch = static_cast<uint8_t>(pitch);
-        e._pad = 0;
-        e.velocity = velocity;
-        e.beat = beat;
-        return e;
-    }
-    
-    static MidiEvent note_off(int channel, int pitch, float beat) {
-        MidiEvent e;
-        e.type = NOTE_OFF;
-        e.channel = static_cast<uint8_t>(channel);
-        e.pitch = static_cast<uint8_t>(pitch);
-        e._pad = 0;
-        e.velocity = 0.0f;
-        e.beat = beat;
-        return e;
-    }
-};
-
-static_assert(sizeof(MidiEvent) == 12, "MidiEvent should be 12 bytes");
-
-} // namespace t7
```

#### D-1.23 — `src/sources/midi_port.hpp`

Blob at `1a52f2db^`: `293ce7c46f669185235e2e6121d48f0a9863563e` · deleted lines: 213 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/sources/midi_port.hpp b/src/sources/midi_port.hpp
deleted file mode 100644
index 293ce7c4..00000000
--- a/src/sources/midi_port.hpp
+++ /dev/null
@@ -1,213 +0,0 @@
-#pragma once
-
-// ─── midi_port.hpp  (dev: transport-aware) ───────────────────────
-//
-// External MIDI input via a system port (loopMIDI / DAW), now also reading
-// the DAW's transport. Same note path as the original — incoming note
-// events go into a lock-free ring drained by poll() — with one addition:
-// the callback also feeds MIDI clock and transport messages to a
-// MidiTransport, so the port surfaces the DAW's musical position, play
-// state, and tempo.
-//
-// Change from the original: ignoreTypes no longer drops timing (clock now
-// flows), and handle_message offers each message to the transport first;
-// only non-transport messages fall through to the note decode. Note
-// consumers are unaffected — they still just poll() events.
-//
-// In Ableton: enable this port's Clock/Sync output. The port then counts
-// the 24-per-quarter pulses into beats() — phase-locked, and frozen when
-// the DAW is stopped.
-//
-// Depends on: sources/midi_event.hpp, sources/transport.hpp,
-//             external/RtMidi.h, and the standard headers below.
-
-#include "sources/midi_event.hpp"
-#include "sources/transport.hpp"
-#include "external/RtMidi.h"
-
-#include <array>
-#include <atomic>
-#include <cctype>
-#include <cstdint>
-#include <memory>
-#include <string>
-#include <vector>
-
-namespace t7 {
-
-// ═══ MIDI PORT ═══════════════════════════════════════════════════
-
-class MidiPort {
-public:
-    MidiPort() {
-        try {
-            midi_in_ = std::make_unique<RtMidiIn>();
-        } catch (RtMidiError&) {
-            midi_in_.reset();
-        }
-    }
-
-    ~MidiPort() { close(); }
-
-    MidiPort(const MidiPort&) = delete;
-    MidiPort& operator=(const MidiPort&) = delete;
-
-    // ── Connection ───────────────────────────────────────────────
-
-    std::vector<std::string> enumerate() const {
-        std::vector<std::string> result;
-        if (!midi_in_) return result;
-        try {
-            unsigned int n = midi_in_->getPortCount();
-            for (unsigned int i = 0; i < n; ++i)
-                result.push_back(midi_in_->getPortName(i));
-        } catch (RtMidiError&) {}
-        return result;
-    }
-
-    bool open(unsigned int port_index) {
-        if (!midi_in_) return false;
-        if (open_) close();
-        try {
-            unsigned int n = midi_in_->getPortCount();
-            if (port_index >= n) return false;
-
-            midi_in_->openPort(port_index);
-            midi_in_->setCallback(&MidiPort::on_rtmidi_callback, this);
-            // Keep timing clock (middle = false); drop sysex and active sense.
-            midi_in_->ignoreTypes(true, false, true);
-
-            port_name_ = midi_in_->getPortName(port_index);
-            open_ = true;
-            return true;
-        } catch (RtMidiError&) {
-            return false;
-        }
-    }
-
-    bool open_by_name(const std::string& name_substring) {
-        if (!midi_in_) return false;
-        std::vector<std::string> ports = enumerate();
-        for (size_t i = 0; i < ports.size(); ++i)
-            if (icontains(ports[i], name_substring))
-                return open(static_cast<unsigned int>(i));
-        return false;
-    }
-
-    void close() {
-        if (!midi_in_ || !open_) return;
-        try {
-            midi_in_->cancelCallback();
-            midi_in_->closePort();
-        } catch (RtMidiError&) {}
-        open_ = false;
-        port_name_.clear();
-        transport_.reset();
-    }
-
-    bool is_open() const { return open_; }
-    const std::string& port_name() const { return port_name_; }
-
-    // ── DAW transport (read side) ────────────────────────────────
-
-    bool     playing()     const { return transport_.playing(); }
-    double   beats()       const { return transport_.beats(); }
-    float    bpm()         const { return transport_.bpm(); }
-    bool     ever_synced() const { return transport_.ever_synced(); }
-    uint32_t pulses()      const { return transport_.pulses(); }
-
-    // ── POLL — drain note events, stamp with current_beat ─────────
-
-    int poll(float current_beat, MidiEvent* out, int max_out) {
-        int count = 0;
-        const uint32_t write = write_idx_.load(std::memory_order_acquire);
-        uint32_t read = read_idx_.load(std::memory_order_relaxed);
-        while (read != write && count < max_out) {
-            out[count] = ring_[read & RING_MASK];
-            out[count].beat = current_beat;
-            ++read;
-            ++count;
-        }
-        read_idx_.store(read, std::memory_order_release);
-        return count;
-    }
-
-    int pending_count() const {
-        const uint32_t write = write_idx_.load(std::memory_order_acquire);
-        const uint32_t read = read_idx_.load(std::memory_order_acquire);
-        return static_cast<int>(write - read);
-    }
-
-private:
-    static constexpr uint32_t RING_SIZE = 256;
-    static constexpr uint32_t RING_MASK = RING_SIZE - 1;
-    static_assert((RING_SIZE & RING_MASK) == 0, "RING_SIZE must be a power of two");
-
-    std::unique_ptr<RtMidiIn> midi_in_;
-    bool open_ = false;
-    std::string port_name_;
-
-    MidiTransport transport_;
-
-    std::array<MidiEvent, RING_SIZE> ring_{};
-    std::atomic<uint32_t> write_idx_{0};
-    std::atomic<uint32_t> read_idx_{0};
-
-    // ── CALLBACK (runs on RtMidi's thread) ───────────────────────
-
-    static void on_rtmidi_callback(double deltatime,
-                                   std::vector<unsigned char>* msg,
-                                   void* user) {
-        if (!msg || msg->empty() || !user) return;
-        static_cast<MidiPort*>(user)->handle_message(deltatime, *msg);
-    }
-
-    void handle_message(double deltatime, const std::vector<unsigned char>& m) {
-        // Clock / start / stop / continue / song-position go to the transport.
-        if (transport_.feed(deltatime, m)) return;
-
-        // Everything else: the note path, unchanged.
-        if (m.size() < 3) return;
-
-        const uint8_t status   = m[0];
-        const uint8_t type     = status & 0xF0;
-        const uint8_t channel  = status & 0x0F;
-        const uint8_t pitch    = m[1];
-        const uint8_t velocity = m[2];
-
-        MidiEvent ev;
-        if (type == 0x90 && velocity > 0) {
-            ev = MidiEvent::note_on(channel, pitch, velocity / 127.0f, 0.0f);
-        } else if (type == 0x80 || (type == 0x90 && velocity == 0)) {
-            ev = MidiEvent::note_off(channel, pitch, 0.0f);
-        } else {
-            return;  // CC, pitch bend, aftertouch — ignore
-        }
-        push(ev);
-    }
-
-    void push(const MidiEvent& ev) {
-        const uint32_t write = write_idx_.load(std::memory_order_relaxed);
-        const uint32_t read  = read_idx_.load(std::memory_order_acquire);
-        if ((write - read) >= RING_SIZE) return;   // full — drop
-        ring_[write & RING_MASK] = ev;
-        write_idx_.store(write + 1, std::memory_order_release);
-    }
-
-    // ── Helpers ──────────────────────────────────────────────────
-
-    static bool icontains(const std::string& haystack, const std::string& needle) {
-        if (needle.empty()) return true;
-        if (haystack.size() < needle.size()) return false;
-        auto lower = [](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); };
-        for (size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
-            bool match = true;
-            for (size_t j = 0; j < needle.size(); ++j)
-                if (lower(haystack[i+j]) != lower(needle[j])) { match = false; break; }
-            if (match) return true;
-        }
-        return false;
-    }
-};
-
-} // namespace t7
```

#### D-1.24 — `src/sources/transport.hpp`

Blob at `1a52f2db^`: `d21388c5daf29c1dfd170d9c7a5c46870d0867af` · deleted lines: 110 · restored byte-identical at `79adfa4d`.

```diff
diff --git a/src/sources/transport.hpp b/src/sources/transport.hpp
deleted file mode 100644
index d21388c5..00000000
--- a/src/sources/transport.hpp
+++ /dev/null
@@ -1,110 +0,0 @@
-#pragma once
-
-// ─── transport.hpp ───────────────────────────────────────────────
-//
-// DAW musical time from MIDI clock + transport. Pure (no RtMidi): fed the
-// raw MIDI messages a port receives, it maintains the play state and the
-// musical position by counting 0xF8 timing-clock pulses — 24 per quarter
-// note. Position is therefore exact and phase-locked to the DAW; tempo is
-// estimated only for display and is never used to advance the beat.
-//
-// Why counting, not timing: the window we analyse is defined in beats, so
-// its edges must land on the DAW's beats. A pulse count does that exactly.
-// And because a DAW sends clock only while playing, a stop freezes the
-// count — the windows stop sliding on their own, no pause logic needed.
-//
-// Threading: feed() runs on the MIDI input thread; the accessors are read
-// on the main thread. The shared state is atomic; the tempo-estimate
-// scratch is touched only by feed().
-//
-// Depends on: <atomic>, <cstdint>, <vector>.
-
-#include <atomic>
-#include <cstdint>
-#include <vector>
-
-namespace t7 {
-
-constexpr int MIDI_CLOCK_PPQN = 24;   // timing-clock pulses per quarter note
-
-class MidiTransport {
-public:
-    // Offer one raw MIDI message (with RtMidi's delta-time, seconds since the
-    // previous message). Returns true if it was a clock/transport/position
-    // message — consumed here, and NOT to be treated as a note by the caller.
-    bool feed(double delta_seconds, const std::vector<unsigned char>& m) {
-        acc_ += delta_seconds;            // every message spaces the next pulse
-        if (m.empty()) return false;
-
-        switch (m[0]) {
-            case 0xF8:                    // timing clock
-                on_clock();
-                return true;
-            case 0xFA:                    // start — locate to 0 and run
-                pulses_.store(0, std::memory_order_release);
-                playing_.store(true, std::memory_order_release);
-                return true;
-            case 0xFB:                    // continue — run from here
-                playing_.store(true, std::memory_order_release);
-                return true;
-            case 0xFC:                    // stop
-                playing_.store(false, std::memory_order_release);
-                return true;
-            case 0xF2:                    // song position pointer (1/16 notes)
-                if (m.size() >= 3) {
-                    const uint32_t spp = (uint32_t(m[2]) << 7) | uint32_t(m[1]);
-                    pulses_.store(spp * 6, std::memory_order_release);  // 6 pulses / 1/16
-                    synced_.store(true, std::memory_order_release);
-                }
-                return true;
-            default:
-                return false;             // a note or other channel message
-        }
-    }
-
-    // ── Read side (main thread) ──────────────────────────────────
-
-    bool     playing() const { return playing_.load(std::memory_order_acquire); }
-    uint32_t pulses()  const { return pulses_.load(std::memory_order_acquire); }
-    double   beats()   const { return double(pulses()) / MIDI_CLOCK_PPQN; }
-    float    bpm()     const { return bpm_.load(std::memory_order_relaxed); }
-
-    // True once any clock or position message has arrived — lets a caller
-    // tell "DAW not sending clock yet" from "DAW parked at beat 0".
-    bool ever_synced() const { return synced_.load(std::memory_order_acquire); }
-
-    void reset() {
-        pulses_.store(0);
-        playing_.store(false);
-        bpm_.store(0.0f);
-        synced_.store(false);
-        acc_ = 0.0;
-        ema_ = 0.0;
-    }
-
-private:
-    std::atomic<uint32_t> pulses_{0};
-    std::atomic<bool>     playing_{false};
-    std::atomic<float>    bpm_{0.0f};
-    std::atomic<bool>     synced_{false};
-
-    // Tempo estimate — feed()-thread scratch only, never shared.
-    double acc_ = 0.0;    // seconds accumulated since the last pulse
-    double ema_ = 0.0;    // smoothed seconds-per-pulse
-
-    void on_clock() {
-        pulses_.fetch_add(1, std::memory_order_acq_rel);
-        synced_.store(true, std::memory_order_release);
-
-        const double interval = acc_;     // seconds since the previous pulse
-        acc_ = 0.0;
-        if (interval > 1e-6) {
-            ema_ = (ema_ <= 0.0) ? interval : (ema_ * 0.8 + interval * 0.2);
-            const double seconds_per_beat = ema_ * MIDI_CLOCK_PPQN;
-            if (seconds_per_beat > 1e-6)
-                bpm_.store(float(60.0 / seconds_per_beat), std::memory_order_relaxed);
-        }
-    }
-};
-
-} // namespace t7
```

---

### Appendix D-2 — what Appendix D-1 covers, and what it does not

Completeness check on D-1, so the plan can see the closure rather than assume it:

```
$ git diff --diff-filter=D --name-only '1a52f2db^..1a52f2db' -- src/analysis/ src/musical/ src/sources/ | wc -l
27
```

| set | files | deleted lines | disposition in this section |
| --- | --- | --- | --- |
| deleted under `src/analysis/` | 7 | 1418 | 6 full diffs in D-1; `canvas_1/canvas.hpp` truncated (686) |
| deleted under `src/musical/` | 15 | 2550 | 14 full diffs in D-1; `stream_data.hpp` truncated (489) |
| deleted under `src/sources/` | 5 | 1108 | 4 full diffs in D-1; `midi_file.hpp` truncated (469) |
| **all deleted arm files** | **27** | **5076** | **24 quoted in full · 3 truncated and declared** |
| added under `src/analysis/` | 1 | (+53) | `beat_clock.hpp`, quoted by symbol in §(d) Hop 5 |
| touched under `src/coupling/` | 0 | — | nothing to quote |

1418 + 2550 + 1108 = **5076**, closing exactly against the total, and 7 + 15 + 5 = 27
files. Per-directory figures taken with:

```
$ for d in src/analysis/ src/musical/ src/sources/; do
    git diff --numstat '1a52f2db^..1a52f2db' -- "$d" | awk -F'\t' '{d+=$2;n++} END{print n, d}'
  done
7 1418
15 2550
5 1108
$ git diff --numstat '1a52f2db^..1a52f2db' -- src/analysis/ src/musical/ src/sources/ \
    | awk -F'\t' '{a+=$1; d+=$2} END{print a, d}'
53 5076
```

(The `53` added lines are `src/analysis/beat_clock.hpp`, the one added file, which is
why the deleted column alone is the 5076 figure.) All rows here use `--numstat`,
which is also what §(d)'s `--stat` reports; §(f)'s table uses `wc -l`, and the two
disagree by one line on `src/musical/spine.hpp` alone — 271 by `--numstat`, 270 by
`wc -l` — because that file carries no trailing newline. Recorded rather than
smoothed over; it does not affect any total above, all of which are `--numstat`.

**Still truncated after this amendment, and declared as such:**

| path | deleted lines | why held back | exact recovery |
| --- | --- | --- | --- |
| `src/analysis/canvas_1/canvas.hpp` | 686 | over the ~400-line threshold | `git show 1a52f2db^:src/analysis/canvas_1/canvas.hpp` (blob `250b6e56310c`) |
| `src/musical/stream_data.hpp` | 489 | over the ~400-line threshold | `git show 1a52f2db^:src/musical/stream_data.hpp` (blob `d84086d83e06`) |
| `src/sources/midi_file.hpp` | 469 | over the ~400-line threshold | `git show 1a52f2db^:src/sources/midi_file.hpp` (blob `65c95697c244`) |
| `src/external/RtMidi.{cpp,h}` | 5961 | outside the four directories the unit names; vendored third party | `git show 1a52f2db^:src/external/RtMidi.cpp` (blob `2303bb132273`), `…RtMidi.h` (blob `2801037f628d`) |
| `src/external/imgui/**` (15 files) | 77788 | outside the four directories; vendored third party; **not restored** | `git show 1a52f2db^:<path>` |
| `src/external/implot/**` (4 files) | 12569 | outside the four directories; vendored third party; **not restored** | `git show 1a52f2db^:<path>` |
| `src/the_lab.cpp` | 668 | outside the four directories; over the threshold; **not restored** | `git show 1a52f2db^:src/the_lab.cpp` (blob `3cd1a13fe66e`) |

All 24 files quoted in D-1, and the three arm files truncated above, are present at
`79adfa4d` byte-identical to their `1a52f2db^` pre-image (blob-SHA check in §(d), 27
of 27 matching), so D-1's diffs can be re-derived from the tree at any time. The
`src/external/imgui/**`, `src/external/implot/**` and `src/the_lab.cpp` rows are the
only deleted files in this whole census that are **absent** at `79adfa4d`; they are
recoverable from history alone.

---

### Amendment ledger

Changes this pass made to the section, each re-verified against the tree at
`79adfa4d` before being written. Recorded so the plan can distinguish a corrected
number from an original one.

| § | claim as first written | claim now | recipe that settled it |
| --- | --- | --- | --- |
| header | HEAD = `79adfa4d`, no note | anchor pinned to `79adfa4d`; branch tip advanced to `6d53388e` mid-verification, `src` tree unchanged | `git rev-parse '79adfa4d^{tree}:src'` = `'6d53388e^{tree}:src'` = `c8b334db…` |
| (a) | no tagger for `web-sunset` | tagger `jeanklein1 <jeankleinmusic@gmail.com>`, distinct from `native-sunset`'s | `git cat-file -p web-sunset` |
| (b) | `console.hpp` +2/−1 | **+3/−1**, hunk quoted | `git show --numstat --format='' 79adfa4d` → `3\t1\tsrc/console/console.hpp` |
| (c) | ls-tree output quoted unprefixed and space-joined | quoted verbatim, `src/`-prefixed, one path per line | `git ls-tree -d --name-only 2bedb4e2 src/` |
| (d) | DEATH LIST as a reflowed blockquote with added bold | complete `%b` body in a fenced block, unreflowed, no bold | `git log -1 --format='%b' 1a52f2db` |
| (d) | Hop 4 diff elided 3 deleted comment lines behind `-…` inside the fence | the three lines quoted | `git show 1a52f2db -- src/incubator_dual.cpp` |
| (d) | Hop 5 block presented as `grep -n` output with prefixes stripped and 3 of 16 lines dropped | all 16 lines, prefixes intact | `git cat-file blob '1a52f2db^:src/analysis/canvas_1/canvas.hpp' \| grep -n 'publish_reading('` |
| (d) | publisher anchored to `Canvas::configure` | anchored to **`Canvas::initialize`** (the enclosing symbol of the 13 call sites) | read of the pre-image around the call sites |
| (d) | "21 deleted files totalling 5076 deleted lines" | **27** deleted files totalling 5076 lines; threshold applied per file; 24 full diffs supplied | `git diff --diff-filter=D --name-only '1a52f2db^..1a52f2db' -- src/musical/ src/sources/ src/analysis/ \| wc -l` → 27 |
| (e) | D-list "76 paths", "58" paintings | **78** paths, **57** paintings (57 + 21 = 78) | `git diff --diff-filter=D --name-only 60a3e935..79adfa4d \| wc -l` |
| (e) | `signal_layout.hpp` block labelled "Full diff" but abridged | the real full diff, 3 hunk headers and all context | `git diff '1a52f2db^' 79adfa4d -- src/musical/signal_layout.hpp` |
| (e) | `visual_canvas.hpp` 106 insertions / 67 deletions | **101 insertions / 72 deletions** (385 diff lines unchanged) | `git diff --stat '1a52f2db^' 79adfa4d -- src/coupling/visual_canvas.hpp` |
| (e) | `resolve(` grep "both yield" 6 names | grep yields **16** lines each side; the 6 are the `signal_layout_.` subset | `git cat-file blob '…:src/coupling/visual_canvas.hpp' \| grep -c 'resolve('` → 16 |
| (f) | "50 qualifying files" at BOUNDARY B | **49**, set-identical to the D-list | `sh halfloss.sh '1a52f2db^' 1a52f2db \| wc -l` → 49 |
| (f) | "76 qualifying paths", "16 text files" at BOUNDARY C | **74** paths, **14** text files (74 − 60 assets) | `sh halfloss.sh 60a3e935 79adfa4d \| wc -l` → 74 |
| (f) | paintings deleted by "WEB_SUNSET W?" | deleted by **`89a4f929` (PRUNE_1 U5+U6)**, reaching mainline via merge `e9c2ace3` | `git log --diff-filter=D --format='%h %s' 60a3e935..79adfa4d -- assets/paintings/` |
| (f) | 60 binary assets, no quantity | byte census added: 13,909,864 bytes at `60a3e935`, 0 at `79adfa4d` | `git cat-file -s '60a3e935:<path>'` per path |
| (f) | `web/index.html`, `web/organ_panel.js` symbol inventory FLAGged as unresolved | inventories supplied (IIFE shape, 33 JS functions; document shape, 14 ids, 22 script functions) | indentation-independent name greps over both blobs |
| (f) | 24 arm rows read "truncated → symbol table in §(d)" | 24 rows read "FULL DIFF GIVEN — Appendix D-1"; 3 rows declared over-threshold | Appendix D-1 |
| establishes | item 12 "106 insertions, 67 deletions" | "101 insertions, 72 deletions, 385 diff lines" | as (e) above |

**Additional corrections found by this amendment pass, not raised by the verifier.**
Each was caught while re-deriving a number the verifier had passed over, and each is
recorded with the recipe that settled it.

| § | claim as first written | claim now | recipe that settled it |
| --- | --- | --- | --- |
| (d) directory table | `src/external/` — `imgui/` "(14 files)", "20 files deleted" | `imgui/` **15** files; **21** files deleted under `src/external/` (RtMidi 2 + ImGui 15 + ImPlot 4) | `git diff --diff-filter=D --name-only '1a52f2db^..1a52f2db' -- src/external/ \| wc -l` → 21; `… -- src/external/imgui/ \| wc -l` → 15 |
| (f) truncation table | `src/external/imgui/**` "(14 files: …)" while the parenthetical itself enumerates 15 | label corrected to **15 files**; the enumeration was always right | same recipe; the 15 enumerated names match the D-list one for one |
| establishes 4, 13 | "18 vendored ImGui/ImPlot files" | **19** (15 ImGui + 4 ImPlot) | 15 + 4 = 19; the 49-path D-list partitions as 27 arm + 21 external + 1 `the_lab.cpp` |
| (d) Hop 5 | publisher anchored to `Canvas::configure` | anchored to `Canvas::initialize` | the 13 call sites sit in `void initialize(const char* asset_path) override`, between `constexpr int VOICES = 7;` and `port_.open_by_name("loopMIDI");` |
| D-2 (written this pass) | per-directory sums 1322 / 2647 | **1418** / **2550**; 1418 + 2550 + 1108 = 5076 exactly | `git diff --numstat '1a52f2db^..1a52f2db' -- <dir> \| awk -F'\t' '{d+=$2} END{print d}'` |

The 49-path deletion list of §(d) is unchanged and remains correct; only the prose
counts that summarised its sub-groups were wrong. The partition now closes:
27 + 21 + 1 = 49.

**Verifier claims this amendment did NOT adopt, with the reason:**

* The adversarial verifier reported the `resolve(` grep as yielding **15** lines with
  a seven-member `param_layout_` subset. My re-run gives **16** lines on each side
  and an eight-member `param_layout_` subset (2 comments + 8 `param_layout_` +
  6 `signal_layout_` = 16); the verifier's own enumeration lists eight target names
  while stating seven. The tree's figures are recorded in §(e) and the verifier's
  count is flagged there. Its substantive point — that the earlier draft published
  only the `signal_layout_` subset without saying so — was correct and is fixed.
* The verifier repeated the earlier draft's "21" arm files while noting "18 of the 21
  are under 400 lines". The tree gives **27** arm files, **24** of them under 400
  lines totalling 3432 deleted lines. This is a larger obligation than either the
  draft or the verifier stated, and Appendix D-1 discharges it in full.

**Unchanged by this pass, having survived re-checking:** the §(a) tag ledger rows and
the three facts under them; the §(b) timeline apart from the `console.hpp` numstat
cell, including the PRUNE_1 range FLAG (`bc91cf3a..743dc9d0`, 6 commits) and the
`CMakeLists.txt`/`organ_registry.hpp` cells of the `79adfa4d` row; the §(c) choice of
PRE and the two rejected commits, its `--stat`, its empty deletion list and its full
`src/coupling/` diff; §(d)'s `--stat`, its 49-path deletion list, its directory
table, and Hops 1, 2, 3, 6, 7 and the `namespace analysis_ns` block, all of which are
byte-verbatim with no elision; §(e)'s `--stat -- src/`, its 42-path added-file list,
the whole IDENTICAL/DIVERGED table (37 rows, every blob SHA re-checked), the
`beat_clock.hpp` symbol summary, the two-`organ_registry.hpp` FLAG and its blob
trace, and the build-half diff; §(f)'s census script, its BOUNDARY A result, its
near-miss list, and every symbol inventory other than the two `web/` files.

## 2. Reachability census

**Boundary for every count in this section:** working tree at
`HEAD = 79adfa4d26c9e17e0074692928f1d2875d7edde1`, branch
`claude/ligature-0-recon-hcrix0`, census root `src/` (the directory, recursed),
ripgrep 14.1.0 at `/usr/bin/rg`, run from `/home/user/7T-Music`.
`git status --porcelain` was empty before and after this unit.

**Boundary note added at amendment time.** After the first pass of this section
was written, `HEAD` on the same branch advanced from
`79adfa4d26c9e17e0074692928f1d2875d7edde1` to
`6d53388e83f4a5cd7ad3b154484c885f567a02da` ("LIGATURE_0 — the recon report").
`git diff --name-status 79adfa4d 6d53388e` shows exactly one change,
`A docs/LIGATURE_0_RECON.md`, and `git diff --stat 79adfa4d 6d53388e -- src/ CMakeLists.txt`
is empty. Every count, blob SHA and quote below was re-run at `6d53388e` and is
unchanged, because the census root is `src/`. The one consequence, called out at
its own row in §B.2, is that any *repo-wide* recipe (rooted at `.`) now also
matches this recon report itself; every such recipe below has been restated with
`src/` as its root.

**Two count conventions are used and both are reported:**
- `lines` = `rg -F -n "<needle>" src/ | wc -l` (matching lines)
- `matches` = `rg -F -o "<needle>" src/ | wc -l` (occurrences)

They differ for exactly one needle (`AnalysisSignal`, 20 lines / 22 occurrences).

**Fixed-string flag.** Every sweep below uses `rg -F` (`--fixed-strings`).
It is *required* for `.resolve(` (the `.` and `(` are regex metacharacters)
and is used for the other eleven needles too so that one recipe covers the
whole census. A consequence that is called out per row: `-F` matches
substrings, so `ParamLayout` also hits inside `ParamLayoutView`, and
`SignalLayout` also hits inside the bracketed log tag `[SignalLayout]`.
Each table names the identifier actually present.

**Blob SHAs (R6)** — `git rev-parse HEAD:<path>`:

| path | blob SHA |
| --- | --- |
| `src/cartridges/the_board/cartridge.hpp` | `3651bcabaa0b02a2925ad6868ce541ea9ab1b202` |
| `src/coupling/visual_canvas.hpp` | `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35` |
| `src/coupling/visual_params.hpp` | `c196529d08b9b815a4b4282e7dc2695661febbc5` |
| `src/coupling/trajectory.hpp` | `a156425a5cc235d7a591fdd4c1150477406f86ce` |
| `src/coupling/canvas_surface.hpp` | `336d5d320f3a0337a78568f5762eae5885022109` |
| `src/coupling/organ_registry.hpp` | `3047070e199df57c2a7cd6d8f75cf028ec48b817` |
| `src/musical/signal_layout.hpp` | `8e2e84312483e31e429276d91c23f7d63dc2643c` |
| `src/analysis/analysis_signal.hpp` | `d088796d0ece785b9e34ee071273d6c5df7ce4a4` |
| `src/analysis/beat_clock.hpp` | `b10038ff5069783c6be15e1e8d885d36238f7354` |
| `src/analysis/analysis_cartridge.hpp` | `f07e11a992d2381413b303f6741ddf32fe8205ab` |
| `src/analysis/canvas_1/canvas.hpp` | `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5` |
| `src/analysis/canvas_1/probe_canvas.cpp` | `7a341e9c00a0b8bd1965714df87c533d30a6cced` |
| `src/the_board.cpp` | `588174ecddb0d68388e39a9025d6eda2f2afd000` |
| `src/core/cartridge_manager.hpp` | `1de6479a13627e4385741c3f16f4a0ce4aef5685` |
| `src/sources/midi_port.hpp` | `293ce7c46f669185235e2e6121d48f0a9863563e` |
| `src/cartridges/the_board/bodies/ribbon.hpp` | `0c662c8e1454144d85f0074c70aaa97beee2087f` |
| `src/render/render_cartridge.hpp` | `ee8065299b2fb55fc0c97d6ec21fb465f36f5452` |

---

### 2.0 Count summary (all twelve needles, one recipe)

Recipe (run once per needle, from `/home/user/7T-Music`):

```sh
rg -F -n "<needle>" src/ | wc -l     # lines
rg -F -o "<needle>" src/ | wc -l     # matches
```

| needle | lines | matches | files touched |
| --- | ---: | ---: | ---: |
| `visual_canvas_` | 22 | 22 | 3 |
| `VisualCanvas` | 6 | 6 | 3 |
| `VisualParams` | 11 | 11 | 4 |
| `ParamLayout` | 11 | 11 | 2 |
| `TargetBinding` | 29 | 29 | 4 |
| `ParamSlot` | 10 | 10 | 2 |
| `SignalLayout` | 8 | 8 | 3 |
| `SourceBinding` | 15 | 15 | 3 |
| `AnalysisSignal` | 20 | 22 | 9 |
| `signal_layout_` | 11 | 11 | 2 |
| `.resolve(` | 26 | 26 | 4 |
| `Trajectory` | **0** | **0** | **0** |

---

### 2.1 `visual_canvas_` — 22 hits

Command: `rg -F -n "visual_canvas_" src/`

| file :: enclosing symbol | what the hit is |
| --- | --- |
| `src/coupling/visual_canvas.hpp` :: file header comment, `USAGE` block (pre-`namespace t7`) | 5 hits — `visual_canvas_.bind(...)`, `.tick(signal)`, two `.layout().resolve(...)`, `.params().get(...)` written as usage prose |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: `struct t7::the_board::RibbonDeps` | member `const VisualCanvas& visual_canvas_;` |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: `t7::the_board::ribbon_frame_tick(RibbonState&, RibbonDeps*, wgpu::Queue&)` | `const VisualParams& vp = c->visual_canvas_.params();` |
| `src/cartridges/the_board/cartridge.hpp` :: `class t7::the_board::Cartridge` (data member) | `VisualCanvas  visual_canvas_;` |
| `src/cartridges/the_board/cartridge.hpp` :: `t7::the_board::Cartridge::Cartridge()` (ctor member-init list) | `visual_canvas_` passed into `ribbon_deps_{...}` |
| `src/cartridges/the_board/cartridge.hpp` :: `t7::the_board::Cartridge::bind_signal_layout(StatLayoutView)` | 9 hits — `visual_canvas_.bind(v);` plus 8 `visual_canvas_.layout().resolve("...")` |
| `src/cartridges/the_board/cartridge.hpp` :: `t7::the_board::Cartridge::phase_motion_drivers(UpdateCtx&)` | 4 hits — `visual_canvas_.tick(signal);`, two `visual_canvas_.params()` reads, `visual_canvas_.zoetrope_rows()` |

Split: `visual_canvas.hpp` 5, `ribbon.hpp` 2, `cartridge.hpp` 15. Total 22.

---

### 2.2 `VisualCanvas` — 6 hits

Command: `rg -F -n "VisualCanvas" src/`

| file :: enclosing symbol | what the hit is |
| --- | --- |
| `src/coupling/visual_canvas.hpp` :: file header comment, `WIRING (live)` paragraph | prose "The cartridge owns a VisualCanvas" |
| `src/coupling/visual_canvas.hpp` :: `namespace t7` (class definition) | `class VisualCanvas {` — the sole definition |
| `src/cartridges/the_board/cartridge.hpp` :: `class t7::the_board::Cartridge` (data member) | `VisualCanvas  visual_canvas_;` |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: file scope, re-opened `namespace t7` | `namespace t7 { class VisualCanvas; struct TargetBinding; }` — forward declaration |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: `struct t7::the_board::RibbonDeps` | member `const VisualCanvas& visual_canvas_;` |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: file-scope `IMPL:` banner comment | cohort-order note "(VisualCanvas + TargetBinding complete)" |

---

### 2.3 `VisualParams` — 11 hits

Command: `rg -F -n "VisualParams" src/`

| file :: enclosing symbol | what the hit is |
| --- | --- |
| `src/coupling/visual_canvas.hpp` :: file header comment | prose "drives VisualParams" |
| `src/coupling/visual_canvas.hpp` :: `t7::VisualCanvas::params() const` | return type `const VisualParams&` |
| `src/coupling/visual_canvas.hpp` :: `class t7::VisualCanvas` (private data member) | `VisualParams params_;` |
| `src/coupling/visual_params.hpp` :: file header comment | 3 hits — the mirror table (`AnalysisSignal.stats[] ↔ VisualParams.v[]`), the `CPU-SIDE` paragraph, and the intro line |
| `src/coupling/visual_params.hpp` :: `namespace t7` (struct definition) | `struct VisualParams {` — the sole definition |
| `src/coupling/visual_params.hpp` :: `t7::ParamLayout::reset(VisualParams&) const` | parameter type |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: `t7::the_board::ribbon_frame_tick(...)` | `const VisualParams& vp = c->visual_canvas_.params();` |
| `src/cartridges/the_board/cartridge.hpp` :: `t7::the_board::Cartridge::phase_motion_drivers(UpdateCtx&)` | 2 hits — `const VisualParams& fp = ...` (fog flush) and `const VisualParams& cp = ...` (checker flush) |

---

### 2.4 `ParamLayout` — 11 hits

Command: `rg -F -n "ParamLayout" src/`

`-F` substring caveat: 5 of the 11 are inside the *different* identifier
`ParamLayoutView`. The "identifier present" column disambiguates.
Cross-check recipe: `rg -F -c "ParamLayoutView" src/` → `visual_canvas.hpp:1`,
`visual_params.hpp:4` (5 total), leaving 6 bare `ParamLayout`.

| file :: enclosing symbol | identifier present |
| --- | --- |
| `src/coupling/visual_canvas.hpp` :: `t7::VisualCanvas::bind(StatLayoutView)` | `ParamLayoutView{ PARAM_LAYOUT, PARAM_LAYOUT_COUNT }` |
| `src/coupling/visual_canvas.hpp` :: `t7::VisualCanvas::layout() const` | `const ParamLayout&` (return type) |
| `src/coupling/visual_canvas.hpp` :: `class t7::VisualCanvas` (private data member) | `ParamLayout  param_layout_;` |
| `src/coupling/visual_params.hpp` :: file header comment | `ParamLayout` (intro line) |
| `src/coupling/visual_params.hpp` :: file header comment (mirror table) | `ParamLayoutView` |
| `src/coupling/visual_params.hpp` :: file header comment (mirror table) | `ParamLayout` (in `SignalLayout/SourceBinding ↔ ParamLayout/TargetBinding`) |
| `src/coupling/visual_params.hpp` :: `namespace t7` (struct definition) | `struct ParamLayoutView {` |
| `src/coupling/visual_params.hpp` :: `namespace t7` (class definition) | `class ParamLayout {` — the sole definition |
| `src/coupling/visual_params.hpp` :: `t7::ParamLayout::bind(ParamLayoutView)` | `ParamLayoutView` (parameter) |
| `src/coupling/visual_params.hpp` :: `t7::ParamLayout::resolve(std::string_view) const` | `"[ParamLayout] pipe '%.*s' not in layout (coupling unbound)\n"` (stderr literal) |
| `src/coupling/visual_params.hpp` :: `class t7::ParamLayout` (private data member) | `ParamLayoutView view_{ nullptr, 0 };` |

**No `ParamLayout` hit exists outside `src/coupling/`.** The cartridge reaches
the layout only through `VisualCanvas::layout()`, never by naming the type.

---

### 2.5 `TargetBinding` — 29 hits

Command: `rg -F -n "TargetBinding" src/`

| file :: enclosing symbol | count | what the hits are |
| --- | ---: | --- |
| `src/coupling/visual_params.hpp` :: file header comment | 3 | mirror table line; `USAGE` line `TargetBinding t = param_layout_.resolve("orb.speed");`; the "Resolve once, store the TargetBinding" note above the struct |
| `src/coupling/visual_params.hpp` :: `namespace t7` (struct definition) | 1 | `struct TargetBinding {` — the sole definition |
| `src/coupling/visual_params.hpp` :: `t7::ParamLayout::resolve(std::string_view) const` | 3 | return type; `return TargetBinding{ s.base, s.count, true };`; `return TargetBinding{};` (the miss path) |
| `src/coupling/visual_canvas.hpp` :: `class t7::VisualCanvas` (private data members) | 8 | `fog_density_`, `fog_color_`, `amp_lat_`, `amp_vert_`, `tint_stim_`, `tint_mix_`, `checker_mean_`, `checker_var_` |
| `src/cartridges/the_board/cartridge.hpp` :: `class t7::the_board::Cartridge` (data members) | 8 | `fog_density_dst_`, `ribbon_amp_lat_dst_`, `ribbon_amp_vert_dst_`, `ribbon_tint_stim_dst_`, `ribbon_tint_mix_dst_`, `fog_color_dst_`, `checker_mean_dst_`, `checker_var_dst_` |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: file scope, re-opened `namespace t7` | 1 | `struct TargetBinding;` forward declaration |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: `struct t7::the_board::RibbonDeps` | 4 | `ribbon_amp_lat_dst_`, `ribbon_amp_vert_dst_`, `ribbon_tint_stim_dst_`, `ribbon_tint_mix_dst_` — all `const TargetBinding&` |
| `src/cartridges/the_board/bodies/ribbon.hpp` :: file-scope `IMPL:` banner comment | 1 | cohort-order note |

Split: `visual_params.hpp` 7, `visual_canvas.hpp` 8, `cartridge.hpp` 8,
`ribbon.hpp` 6. Total 29.

---

### 2.6 `ParamSlot` — 10 hits

Command: `rg -F -n "ParamSlot" src/`

| file :: enclosing symbol | count | what the hits are |
| --- | ---: | --- |
| `src/coupling/visual_params.hpp` :: file header comment | 2 | mirror-table row `StatGroup / StatShape ↔ ParamSlot (width = count)`; the `REST` paragraph |
| `src/coupling/visual_params.hpp` :: `namespace t7` (struct definition) | 1 | `struct ParamSlot {` — the sole definition |
| `src/coupling/visual_params.hpp` :: `struct t7::ParamLayoutView` (data member) | 1 | `const ParamSlot* slots;` |
| `src/coupling/visual_params.hpp` :: `t7::ParamLayout::reset(VisualParams&) const` | 1 | `const ParamSlot& s = view_.slots[i];` |
| `src/coupling/visual_params.hpp` :: `t7::ParamLayout::resolve(std::string_view) const` | 1 | `const ParamSlot& s = view_.slots[i];` |
| `src/coupling/visual_params.hpp` :: `t7::ParamLayout::slots() const` | 1 | return type `const ParamSlot*` |
| `src/coupling/visual_canvas.hpp` :: `namespace t7` (the master control panel table) | 1 | `inline constexpr ParamSlot PARAM_LAYOUT[] = { ... }` |
| `src/coupling/visual_canvas.hpp` :: the namespace-scope `static_assert` immediately after `PARAM_LAYOUT_COUNT` (the overlap/bounds witness lambda) | 2 | `const ParamSlot& a = PARAM_LAYOUT[i];` and `const ParamSlot& b = PARAM_LAYOUT[j];` |

**No `ParamSlot` hit exists outside `src/coupling/`.**

Related fact (separate recipe, `rg -F -c "PARAM_LAYOUT" src/`): the table's
*name* is referenced in 4 files — `visual_canvas.hpp` (10), `visual_params.hpp`
(2), `coupling/canvas_surface.hpp` (1), `bodies/ribbon.hpp` (1).

---

### 2.7 `SignalLayout` — 8 hits

Command: `rg -F -n "SignalLayout" src/`

`-F` substring caveat: 2 of the 8 are inside the bracketed stderr tag
`[SignalLayout]`, not a type use.

| file :: enclosing symbol | what the hit is |
| --- | --- |
| `src/musical/signal_layout.hpp` :: `namespace t7` (class definition) | `class SignalLayout {` — the sole definition |
| `src/musical/signal_layout.hpp` :: `t7::SignalLayout::resolve(std::string_view) const` | `"[SignalLayout] source '%.*s' not in layout (coupling disabled)\n"` — inside `#ifndef NDEBUG` |
| `src/coupling/visual_canvas.hpp` :: `t7::VisualCanvas::bind(StatLayoutView)` | `"[SignalLayout] %u sources unbound (no audio source)\n"` — the PORT_4c summary line |
| `src/coupling/visual_canvas.hpp` :: `class t7::VisualCanvas` (private data member) | `SignalLayout signal_layout_;` |
| `src/coupling/visual_params.hpp` :: file header comment | 4 hits — intro line, mirror-table row, `USAGE (mirrors SignalLayout …)` heading, and the "Dual of SignalLayout / SourceBinding" note |

**The only type-level use of `SignalLayout` outside its own header is the one
private member of `VisualCanvas`.** No cartridge, no `the_board.cpp`, no
`analysis/` file names the type.

---

### 2.8 `SourceBinding` — 15 hits

Command: `rg -F -n "SourceBinding" src/`

| file :: enclosing symbol | count | what the hits are |
| --- | ---: | --- |
| `src/musical/signal_layout.hpp` :: file header comment (`USAGE (B2)`) | 2 | `resolve ONCE, store the SourceBinding`; `SourceBinding src = signal_layout_.resolve("abbott.lowest_pc");` |
| `src/musical/signal_layout.hpp` :: `namespace t7` (struct definition) | 1 | `struct SourceBinding {` — the sole definition |
| `src/musical/signal_layout.hpp` :: `t7::SignalLayout::resolve(std::string_view) const` | 3 | return type; `return SourceBinding{ g.channel, g.slot_base, g.count, true };`; `return SourceBinding{};  // valid = false` |
| `src/coupling/visual_params.hpp` :: file header comment | 2 | mirror-table row; "Dual of SignalLayout / SourceBinding" |
| `src/coupling/visual_canvas.hpp` :: `t7::VisualCanvas::tick(const AnalysisSignal&)` | 1 | `const SourceBinding& ear = zoetrope_ears_[e];` (the zoetrope ear fold) |
| `src/coupling/visual_canvas.hpp` :: `class t7::VisualCanvas` (private data members) | 6 | `fog_field_`, `voice_playhead_`, `room_wagon_`, `room_playhead_`, `checker_win_`, `zoetrope_ears_[8]` |

**`SourceBinding` never appears in `src/cartridges/`, `src/analysis/`,
`src/sources/` or `src/the_board.cpp`.** Every source-side binding in the tree
is private state of `VisualCanvas`.

---

### 2.9 `AnalysisSignal` — 20 lines / 22 occurrences

Command: `rg -F -n "AnalysisSignal" src/` (20 lines);
`rg -F -o "AnalysisSignal" src/ | wc -l` (22 occurrences — two `static_assert`
lines in `analysis_signal.hpp` each carry the token twice, once in the
expression and once in the message string).

| file :: enclosing symbol | count | what the hits are |
| --- | ---: | --- |
| `src/analysis/analysis_signal.hpp` :: `namespace t7` (struct definition) | 1 | `struct alignas(16) AnalysisSignal {` — the sole definition |
| `src/analysis/analysis_signal.hpp` :: `namespace t7` (two namespace-scope `static_assert`s directly under the struct) | 2 lines / 4 occ. | `sizeof(AnalysisSignal) == 4128`; `alignof(AnalysisSignal) == 16` |
| `src/analysis/analysis_signal.hpp` :: `struct t7::StatGroup` (member comment) | 1 | `int channel;  // AnalysisSignal channel` |
| `src/analysis/beat_clock.hpp` :: file header comment | 1 | "an AnalysisSignal each frame and a StatLayoutView once at bind" |
| `src/analysis/beat_clock.hpp` :: `t7::BeatClock::output() const` (its doc comment + signature) | 2 | comment "AnalysisSignal carries no transport flag"; `const AnalysisSignal& output() const` |
| `src/analysis/beat_clock.hpp` :: `struct t7::BeatClock` (private data member) | 1 | `AnalysisSignal signal_{};` |
| `src/analysis/analysis_cartridge.hpp` :: `t7::AnalysisCartridge::output()` (pure virtual) | 1 | `virtual const AnalysisSignal& output() const = 0;` |
| `src/analysis/canvas_1/canvas.hpp` :: `t7::canvas_1::Canvas::output() const override` | 1 | `const AnalysisSignal& output() const override { return output_; }` |
| `src/analysis/canvas_1/canvas.hpp` :: `class t7::canvas_1::Canvas` (private data member) | 1 | `AnalysisSignal output_{};` |
| `src/analysis/canvas_1/probe_canvas.cpp` :: `print_published(const Canvas&)` (static, file scope) | 1 | `const AnalysisSignal& sig = canvas.output();` |
| `src/render/render_cartridge.hpp` :: `t7::RenderCartridge::update(...)` (pure virtual) | 1 | `virtual void update(const AnalysisSignal& signal, float, wgpu::Queue&) = 0;` |
| `src/cartridges/the_board/cartridge.hpp` :: `struct t7::the_board::Cartridge::UpdateCtx` (data member) | 1 | `const AnalysisSignal& signal;` |
| `src/cartridges/the_board/cartridge.hpp` :: `t7::the_board::Cartridge::update(...) override` | 1 | signature |
| `src/coupling/visual_canvas.hpp` :: file header comment | 1 | "publishes AnalysisSignal, this consumes that signal" |
| `src/coupling/visual_canvas.hpp` :: `t7::VisualCanvas::tick(const AnalysisSignal&)` | 1 | signature |
| `src/coupling/visual_params.hpp` :: file header comment | 3 | intro line; mirror-table row; the `CPU-SIDE` paragraph |

Lines by file: `analysis_signal.hpp` 4, `beat_clock.hpp` 4, `analysis_cartridge.hpp` 1,
`canvas_1/canvas.hpp` 2, `canvas_1/probe_canvas.cpp` 1, `render_cartridge.hpp` 1,
`cartridge.hpp` 2, `visual_canvas.hpp` 2, `visual_params.hpp` 3 = 20.

---

### 2.10 `signal_layout_` — 11 hits

Command: `rg -F -n "signal_layout_" src/`

| file :: enclosing symbol | count | what the hits are |
| --- | ---: | --- |
| `src/musical/signal_layout.hpp` :: file header comment (`USAGE (B2)`) | 1 | `SourceBinding src = signal_layout_.resolve("abbott.lowest_pc");` (prose) |
| `src/coupling/visual_canvas.hpp` :: `t7::VisualCanvas::bind(StatLayoutView)` | 9 | `signal_layout_.bind(analysis_layout);`; six `signal_layout_.resolve(...)` calls (`"all.field"`, `"<RIBBON_VOICE>.present_count"`, `"all.window_length"`, `"all.present_count"`, `"<CHECKER_VOICE>.window_length"`, `"chN.onset"`); two `signal_layout_.misses()` reads in the PORT_4c summary |
| `src/coupling/visual_canvas.hpp` :: `class t7::VisualCanvas` (private data member) | 1 | `SignalLayout signal_layout_;` |

**Every runtime use of `signal_layout_` in the tree sits inside
`VisualCanvas::bind`.** There is no per-frame use, and no use outside
`src/coupling/visual_canvas.hpp`.

---

### 2.11 `.resolve(` — 26 hits (fixed-string flag REQUIRED)

Command: `rg -F -n ".resolve(" src/`
The `-F` flag is load-bearing here: without it, `.` matches any character and
`(` opens a capture group, so the pattern would not be the literal call syntax.

| file :: enclosing symbol | count | what the hits are |
| --- | ---: | --- |
| `src/coupling/visual_canvas.hpp` :: file header comment (`USAGE`) | 2 | `visual_canvas_.layout().resolve("fog.density")`; `...resolve("fog.color")` |
| `src/coupling/visual_canvas.hpp` :: `t7::VisualCanvas::bind(StatLayoutView)` | 14 | 6 source-side `signal_layout_.resolve(...)` + 8 target-side `param_layout_.resolve(...)` (`"fog.density"`, `"fog.color"`, `"ribbon.amp_lateral_mult"`, `"ribbon.amp_vertical_mult"`, `"ribbon.color_stim"`, `"ribbon.color_mix"`, `"terrain.checker_mean"`, `"terrain.checker_var"`) |
| `src/coupling/visual_params.hpp` :: file header comment (`USAGE`) | 1 | `TargetBinding t = param_layout_.resolve("orb.speed");` |
| `src/musical/signal_layout.hpp` :: file header comment (`USAGE (B2)`) | 1 | `SourceBinding src = signal_layout_.resolve("abbott.lowest_pc");` |
| `src/cartridges/the_board/cartridge.hpp` :: `t7::the_board::Cartridge::bind_signal_layout(StatLayoutView)` | 8 | `visual_canvas_.layout().resolve("fog.density" / "fog.color" / "ribbon.amp_lateral_mult" / "ribbon.amp_vertical_mult" / "ribbon.color_stim" / "ribbon.color_mix" / "terrain.checker_mean" / "terrain.checker_var")` |

**Comment/code split of the 26 lines** — the first pass reported this split
wrongly (as 24 non-comment, 16 of them in `VisualCanvas::bind`) and contradicted
its own table above; the corrected census follows, with its recipes.

Per-file recipe: `rg -F -c ".resolve(" src/` →

```
src/musical/signal_layout.hpp:1
src/cartridges/the_board/cartridge.hpp:8
src/coupling/visual_canvas.hpp:16
src/coupling/visual_params.hpp:1
```

Each hit was then read to classify it as comment or code
(`rg -F -n ".resolve(" <file>`):

| file | total lines | comment lines | code lines |
| --- | ---: | ---: | ---: |
| `src/coupling/visual_canvas.hpp` | 16 | 2 | 14 |
| `src/cartridges/the_board/cartridge.hpp` | 8 | 0 | 8 |
| `src/coupling/visual_params.hpp` | 1 | 1 | 0 |
| `src/musical/signal_layout.hpp` | 1 | 1 | 0 |
| **total** | **26** | **4** | **22** |

The four comment lines. This block is a **collation of four individually
verbatim lines drawn from three different files**, not a contiguous region of any
one file; each is attributed immediately below it:

```cpp
//   auto d = visual_canvas_.layout().resolve("fog.density");   // entity, once
//   auto k = visual_canvas_.layout().resolve("fog.color");     // base..base+2
//   TargetBinding t = param_layout_.resolve("orb.speed");
//   SourceBinding src = signal_layout_.resolve("abbott.lowest_pc");
```

(the first pair from the `USAGE` block in `src/coupling/visual_canvas.hpp`'s file
header, the third from `src/coupling/visual_params.hpp`'s `USAGE` block, the
fourth from `src/musical/signal_layout.hpp`'s `USAGE (B2)` block.)

**All 22 non-comment `.resolve(` calls sit in exactly two functions:
`VisualCanvas::bind` (14 — six `signal_layout_.resolve` call sites plus eight
`param_layout_.resolve` call sites) and `Cartridge::bind_signal_layout` (8).**

Both function boundaries were checked, not assumed. In
`src/coupling/visual_canvas.hpp`, `rg -n "void bind\(|void tick\(" src/coupling/visual_canvas.hpp`
gives `void bind(StatLayoutView analysis_layout) {` and, as the next member
declaration, `void tick(const AnalysisSignal& signal) {`; all 14 code hits fall
between them, so every one is inside `t7::VisualCanvas::bind`. In
`src/cartridges/the_board/cartridge.hpp` all 8 sit between
`void bind_signal_layout(StatLayoutView v) {` and that function's closing brace —
the whole body is quoted in §A.2 below and contains all eight.

Zero `.resolve(` calls appear inside any per-frame function in the tree.

---

### 2.12 `Trajectory` — **ZERO HITS**

Command: `rg -F -n "Trajectory" src/` → exit status 1, no output.
Cross-check: `rg -F -o "Trajectory" src/ | wc -l` → `0`.

The capitalized identifier `Trajectory` does not exist anywhere under `src/`.
This is a zero on the *identifier*, not on the concept. `src/coupling/trajectory.hpp`
(blob `a156425a5cc235d7a591fdd4c1150477406f86ce`, 80 lines) exists and is
reached, but exports no type by that name. What it does export, in
`namespace t7` at file scope:

```cpp
    struct Segment {
        float from = 0.0f;
        float to = 0.0f;
        float duration_beats = 0.0f;   // span; arrival is exact at start_beat + this
        float start_beat = 0.0f;
    };

    inline Segment plan_segment(float from, float to, float duration_beats, float start_beat) { … }

    inline float sample_segment(const Segment& seg, float beat) { … }

    inline constexpr float DEFAULT_RELEASE_BEATS = 8.0f;

    inline float trajectory_release(Segment& seg, float goal, float beat,
        float span_beats = DEFAULT_RELEASE_BEATS) { … }
```

**Case-insensitive control census.** The first pass reported "26 lines across 9
files" and "12 call sites"; both figures are corrected below, each with the
recipe that produces it.

Recipe: `rg -i -n "trajectory" src/ | wc -l` → **28**.
Recipe: `rg -i -l "trajectory" src/ | wc -l` → **9** (the file count was correct).
Per-file recipe: `rg -i -c "trajectory" src/` →

| file | case-insensitive hits | what they are |
| --- | ---: | --- |
| `src/coupling/visual_canvas.hpp` | 12 | 10 `trajectory_release(...)` call sites (all inside `t7::VisualCanvas::tick`), plus the file header's `Depends on:` line and the `#include "coupling/trajectory.hpp"` line |
| `src/coupling/trajectory.hpp` | 4 | the file's own banner heading (`─── coupling/trajectory.hpp ───`), the `FOLLOW (trajectory_release(Segment&, …))` banner line, the `SEAM[trajectory:foundations]` banner line, and the definition `inline float trajectory_release(Segment& seg, float goal, float beat,` |
| `src/cartridges/the_board/realization/state.hpp` | 3 | prose in the `THE MOUNT BLOCK (RIBBON_1)` comment, the `mount_phase` member comment (`0→1 over the trajectory`), and a comment on the ribbon row intensities (`driven by polyphony through trajectory ramp`) |
| `src/cartridges/the_board/contracts/point.hpp` | 2 | prose — the host-change easing comment, and the `A HOST CHANGE IS A TRAJECTORY, NOT A TELEPORT.` banner |
| `src/cartridges/the_board/cartridge.hpp` | 2 | prose — two `possess()`/host-change comments inside member functions of `t7::the_board::Cartridge` |
| `src/cartridges/the_board/realization/world.wgsl` | 2 | prose — the `THE MOUNT BLOCK (RIBBON_1)` comment and the boarding comment |
| `src/cartridges/the_board/bodies/pawn.hpp` | 1 | prose — file-header line `// trajectory, per-frame coupling tick.` |
| `src/cartridges/the_board/direction/input.hpp` | 1 | prose — the `A HOST CHANGE IS A TRAJECTORY.` banner |
| `src/cartridges/the_board/primitives/seed_utils.hpp` | 1 | prose — the `Depends on:` note naming `coupling/trajectory.hpp` |
| **total** | **28** | |

`src/cartridges/the_board/bodies/pawn.hpp` was dropped from the first pass's
prose-hit enumeration; it is restored above. Nine files, 28 lines, no row omitted.

**The *lowercase* function `trajectory_release` has 10 static call sites, not 12,
and all 10 are inside `t7::VisualCanvas::tick(const AnalysisSignal&)` in
`src/coupling/visual_canvas.hpp`.**

Recipe: `rg -F -n "trajectory_release(" src/coupling/visual_canvas.hpp | wc -l`
→ `10`; `rg -F -o "trajectory_release(" src/coupling/visual_canvas.hpp | wc -l`
→ `10` as well, so no line carries the token twice. The "all inside `tick`" half
of the first pass's claim is CONFIRMED: `tick` opens at
`void tick(const AnalysisSignal& signal) {` and the next member declaration after
the last call site is `const VisualParams& params() const { return params_; }`;
all ten sites fall between them.

The ten sites, by the `Segment` each drives and by whether it is written inside a
loop (the loop-count column is the static loop bound in the source, not a
measured run):

| # | segment written | enclosing loop in the source | pipe it publishes into |
| ---: | --- | --- | --- |
| 1 | `fog_seg_` | none | `fog_density_.base` |
| 2 | `fog_color_seg_[c]` | `for (int c = 0; c < 3; ++c)` | `fog_color_.base + c` |
| 3 | `amp_lat_seg_` | none | `amp_lat_.base` |
| 4 | `amp_vert_seg_` | none | `amp_vert_.base` |
| 5 | `tint_stim_seg_[c2]` (live-hue branch, `len > 1e-4f`) | `for (int c2 = 0; c2 < 3; ++c2)` | `tint_stim_.base + c2` |
| 6 | `tint_stim_seg_[c2]` (window-drained `else` branch) | `for (int c2 = 0; c2 < 3; ++c2)` | `tint_stim_.base + c2` |
| 7 | `tint_mix_seg_` | none | `tint_mix_.base` |
| 8 | `checker_res_seg_[c2]` | `for (int c2 = 0; c2 < 3; ++c2)` | `checker_mean_.base + c2` |
| 9 | `checker_amount_seg_` | none | `checker_var_.base` |
| 10 | `checker_var_seg_` | none | `checker_var_.base + 1` |

**FLAG — the verifier's repo-wide sub-count is itself off by one, recorded rather
than adopted.** The verifier stated that repo-wide
`rg -F -n "trajectory_release(" src/` returns 11 lines, "the eleventh being the
FOLLOW banner comment in `src/coupling/trajectory.hpp`". Re-run here it returns
**12** lines: the 10 call sites above, plus *two* lines in
`src/coupling/trajectory.hpp` — the `FOLLOW (trajectory_release(Segment&, …))`
banner comment **and** the definition line
`inline float trajectory_release(Segment& seg, float goal, float beat,`. The
verifier's headline correction (10 call sites, not 12) is CONFIRMED and adopted;
only its repo-wide arithmetic is amended. What it would have cost to leave this
unresolved: nothing measurable — both figures were re-derivable from the recipes
already published — but the 12-line repo-wide sweep is the most likely origin of
the first pass's erroneous "12 call sites", so it is recorded here explicitly.

---

### (A) `src/cartridges/the_board/cartridge.hpp` — the `VisualCanvas` boundary

Blob SHA: `git rev-parse HEAD:src/cartridges/the_board/cartridge.hpp` →
**`3651bcabaa0b02a2925ad6868ce541ea9ab1b202`** (3169 lines).
The enclosing class throughout is `class Cartridge : public RenderCartridge`
inside `namespace t7 { namespace the_board { … } }`.

#### A.1 Does it hold a `VisualCanvas` member? — **YES**

In `class t7::the_board::Cartridge`, data-member block, immediately after
`TimeState time_state_;`:

```cpp
            VisualCanvas  visual_canvas_;
            TargetBinding fog_density_dst_{};   // resolved "fog.density" pipe
            // Ribbon amp pipes (pitch compass) — resolved once at bind.
            TargetBinding ribbon_amp_lat_dst_{};
            TargetBinding ribbon_amp_vert_dst_{};
            TargetBinding ribbon_tint_stim_dst_{};
            TargetBinding ribbon_tint_mix_dst_{};
            TargetBinding fog_color_dst_{};      // resolved "fog.color" pipe (3 wide)
            // Checker pipes (CHECKER-1) — resolved once at bind.
            TargetBinding checker_mean_dst_{};
            TargetBinding checker_var_dst_{};
```

The member is also handed by reference into the ribbon module's deps, in
`Cartridge::Cartridge()`'s member-init list:

```cpp
                , ribbon_deps_{ gpuState_, time_state_, tile_world_state_, player_, point_, inputState_, world_state_, mood_state_, visual_canvas_, ribbon_amp_lat_dst_, ribbon_amp_vert_dst_, ribbon_tint_stim_dst_, ribbon_tint_mix_dst_ }
```

#### A.2 Is `bind()` reached? — **YES**

Whole body of `t7::the_board::Cartridge::bind_signal_layout(StatLayoutView)`:

```cpp
            void bind_signal_layout(StatLayoutView v) {
                visual_canvas_.bind(v);
                fog_density_dst_ = visual_canvas_.layout().resolve("fog.density");
                fog_color_dst_ = visual_canvas_.layout().resolve("fog.color");
                ribbon_amp_lat_dst_ = visual_canvas_.layout().resolve("ribbon.amp_lateral_mult");
                ribbon_amp_vert_dst_ = visual_canvas_.layout().resolve("ribbon.amp_vertical_mult");
                ribbon_tint_stim_dst_ = visual_canvas_.layout().resolve("ribbon.color_stim");
                ribbon_tint_mix_dst_ = visual_canvas_.layout().resolve("ribbon.color_mix");
                checker_mean_dst_ = visual_canvas_.layout().resolve("terrain.checker_mean");
                checker_var_dst_ = visual_canvas_.layout().resolve("terrain.checker_var");
                std::fprintf(stderr,
                    "[the_board] fog.density base=%d valid=%d | fog.color base=%d count=%d valid=%d\n",
                    fog_density_dst_.base, (int)fog_density_dst_.valid,
                    fog_color_dst_.base, fog_color_dst_.count, (int)fog_color_dst_.valid);
                std::fprintf(stderr,
                    "[the_board] terrain.checker_mean base=%d count=%d valid=%d | terrain.checker_var base=%d valid=%d\n",
                    checker_mean_dst_.base, checker_mean_dst_.count, (int)checker_mean_dst_.valid,
                    checker_var_dst_.base, (int)checker_var_dst_.valid);
            }
```

Reach chain. Census recipe, restated with the `src/` root (the repo-wide form
now also matches `docs/LIGATURE_0_RECON.md` at the current HEAD — see the
boundary note at the top of this section): `rg -n "bind_signal_layout" src/` →
exactly 3 hits —

```
src/the_board.cpp:192:    app->render.bind_signal_layout(app->clock.stat_layout());
src/cartridges/the_board/cartridge.hpp:826:            void bind_signal_layout(StatLayoutView v) {
src/coupling/visual_canvas.hpp:31:// bind_signal_layout with the analysis layout, ticks it each frame in
```

— one comment in `visual_canvas.hpp`'s file header, the single call site, and the
definition. The call site sits inside `static bool init_world()`.

`static bool init_world()` has **two** callers, not one; both are named here
because the first pass presented the chain as though `main()` were the only one.
Recipe: `grep -n 'init_world' src/the_board.cpp` → 4 lines — the `world_ready`
member comment in `struct App`, the definition `static bool init_world() {`, and
two call sites. Their enclosing symbols were resolved by scanning backwards for
the nearest column-0 function opener:

| caller | the call, verbatim | guard |
| --- | --- | --- |
| `int main(int argc, char* argv[])` | `if (!init_world()) {` (followed by `delete app; return 1;`) | none — the native boot is synchronous, so the device exists by this point |
| `static void frame()` | `if (!init_world()) {` (followed by `return;`) | wrapped in `if (!app->world_ready) {` — the PORT_1c post-device-init path |

The `frame()` caller, verbatim:

```cpp
    if (!app->world_ready) {
        if (!init_world()) {
            return;
        }
    }
```

and the `main()` caller, verbatim:

```cpp
    // --- World init (device exists — native boot is synchronous) -------------
    if (!init_world()) {
        delete app;
        return 1;
    }
```

So the reach chain has two entrances into the same body:

`main()` (`src/the_board.cpp`) **or** `static void frame()` (`src/the_board.cpp`,
guarded by `!app->world_ready`) → `static bool init_world()` (`src/the_board.cpp`) →
`app->render.bind_signal_layout(app->clock.stat_layout());` →
`Cartridge::bind_signal_layout` → `visual_canvas_.bind(v)` →
`t7::VisualCanvas::bind(StatLayoutView)`.

Recorded without inference about which entrance fires: this unit did not run the
program, and `app->world_ready`'s value at the first `frame()` turn is a runtime
fact. Either entrance reaches the same single call site, so the conclusion —
`bind()` IS reached — does not depend on which.

Verbatim call site, inside `static bool init_world()`:

```cpp
    // Publish the slot map once. The BeatClock's layout is EMPTY by design
    // (CUT_1c): every render-side resolve misses, warns once on stderr, and
    // leaves its coupling disabled — the graceful path in signal_layout.hpp.
    app->render.bind_signal_layout(app->clock.stat_layout());
```

Two facts that come with that chain, recorded as they stand:
- `bind_signal_layout` is **not** a virtual of `t7::RenderCartridge`.
  `rg -n "virtual" src/render/render_cartridge.hpp` lists 11 virtuals
  (`~RenderCartridge`, `initialize`, `update`, `render`, `on_input`,
  `get_pending_transition`, `reset_pawn`, `supports_backspace`,
  `get_clear_color`, `depth_format`, `reload_shaders`) and
  `bind_signal_layout` is not among them. The call resolves because
  `src/the_board.cpp` aliases the **concrete** type:
  `namespace render_ns = t7::INCUBATE_RENDER;` then
  `using RenderCartridge = render_ns::Cartridge;`.
- The `StatLayoutView` it is handed is empty by construction —
  `t7::BeatClock::stat_layout()` in `src/analysis/beat_clock.hpp` is
  `return StatLayoutView{ nullptr, 0 };`. `VisualCanvas::bind` holds six
  `signal_layout_.resolve(...)` *call sites*, one of which is inside the
  `ZOETROPE_EARS` loop and runs seven times (mask `0b0111'1111u`), so the run
  is **12 source-side resolves** and every one returns `valid = false` —
  matching the name list in `beat_clock.hpp`'s own banner (`all.field`,
  `ch1.present_count`, `all.window_length`, `all.present_count`,
  `ch1.window_length`, `ch0.onset` .. `ch6.onset`). The eight
  `param_layout_.resolve(...)` target-side calls resolve against the
  in-header `PARAM_LAYOUT` table and succeed.

#### A.3 Is there a per-frame tick? — **YES**

In `t7::the_board::Cartridge::phase_motion_drivers(UpdateCtx& c)`:

```cpp
            void phase_motion_drivers(UpdateCtx& c) {
                auto& signal = c.signal;
                visual_canvas_.tick(signal);
```

That function is a row of the update spine — `static constexpr URow UPDATE_SPINE[]`,
member of `Cartridge`:

```cpp
                { UPhase::MotionDrivers,       "motion_drivers",        &Cartridge::phase_motion_drivers,        Driver::Music,     true,             F_CONFIG },
```

and the row's `enabled` column is the literal `true` (not a `ROSTER` bit), so it
is never skipped. The conductor is `Cartridge::update(...) override`:

```cpp
            void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {
                GPUFrameSignal gpuSignal{};   // the mount block is filled below, with the rest of the signal — one author, one write
                UpdateCtx ctx{ signal, aspect_ratio, queue, gpuSignal };
                for (const URow& row : UPDATE_SPINE) {
                    if (!row.enabled) continue;   // gated-off rows are never timed
```

and `update` is reached per frame from `static void frame()` in `src/the_board.cpp`:

```cpp
    // --- Update ---------------------------------------------------------
    app->clock.update(dt);
    app->render.update(app->clock.output(), app->console.aspect_ratio(), app->queue);
```

with `frame()` driven by `main()`'s loop:

```cpp
    // --- Main Loop ----------------------------------------------------------
    while (app->console.running()) {
        frame();
    }
```

#### A.4 Is there a per-frame flush? — **YES, in two places**

**(i) Same phase, immediately after the tick** — `Cartridge::phase_motion_drivers`,
the fog seam and the checker seam:

```cpp
                {
                    const auto& drv = DRIVER_LIVE.fog;
                    const auto& ms  = mood_state_;
                    if (fog_density_dst_.valid && fog_color_dst_.valid) {
                        const VisualParams& fp = visual_canvas_.params();
                        gpuState_.set_fog(
                            std::max(0.0f, ms.fog_rest_density + drv.gain * fp.get(fog_density_dst_.base)),
                            std::clamp(ms.fog_rest_color[0] + drv.gain * fp.get(fog_color_dst_.base + 0), 0.0f, 1.0f),
                            std::clamp(ms.fog_rest_color[1] + drv.gain * fp.get(fog_color_dst_.base + 1), 0.0f, 1.0f),
                            std::clamp(ms.fog_rest_color[2] + drv.gain * fp.get(fog_color_dst_.base + 2), 0.0f, 1.0f));
                    } else {
                        gpuState_.set_fog(ms.fog_rest_density, ms.fog_rest_color[0],
                                          ms.fog_rest_color[1], ms.fog_rest_color[2]);
                    }
                }
```

```cpp
                if (checker_mean_dst_.valid && checker_var_dst_.valid) {
                    const VisualParams& cp = visual_canvas_.params();
                    const auto& ck = DRIVER_LIVE.checker;
                    const float* mean = cp.run(checker_mean_dst_.base);
                    const float blended[3] = {
                        ck.rest_resultant[0] + ck.gain * (mean[0] - ck.rest_resultant[0]),
                        ck.rest_resultant[1] + ck.gain * (mean[1] - ck.rest_resultant[1]),
                        ck.rest_resultant[2] + ck.gain * (mean[2] - ck.rest_resultant[2]),
                    };
                    gpuState_.set_checker_color_field(blended,
                        ck.rest_amount   + ck.gain * (cp.get(checker_var_dst_.base)     - ck.rest_amount),
                        ck.rest_variance + ck.gain * (cp.get(checker_var_dst_.base + 1) - ck.rest_variance));
```

plus the zoetrope hand-off at the tail of the same function:

```cpp
                zoetrope_strike(cube_behaviors_state_, gpuState_, c.queue,
                    world_state_.active_seed, visual_canvas_.zoetrope_rows(), signal.t_beats);
```

**(ii) Ribbon module, on the render spine** — `t7::the_board::ribbon_frame_tick`
in `src/cartridges/the_board/bodies/ribbon.hpp` (blob
`0c662c8e1454144d85f0074c70aaa97beee2087f`) reads the bank through the deps:

```cpp
            const VisualParams& vp = c->visual_canvas_.params();
```

reached per frame via `Cartridge::phase_ribbon_tick(RenderCtx& c)`:

```cpp
            void phase_ribbon_tick(RenderCtx& c) {
                auto& queue = c.queue;
                // The sky-exit death first — it releases the ground, so it
                // takes the machine face the tick below does not carry.
                ribbon_on_dismount(&machine_ctx_, queue);
                ribbon_frame_tick(ribbon_state_, &ribbon_deps_, queue);
            }
```

which is a row of `static constexpr RRow RENDER_SPINE[]`:

```cpp
                { RPhase::RibbonTick,          "ribbon_tick",           &Cartridge::phase_ribbon_tick,           Driver::Mixed,     ROSTER.ribbon,                          F_SIGNAL },
```

looped inside `Cartridge::render(wgpu::CommandEncoder&, …) override`, itself
called each frame from `frame()` in `src/the_board.cpp`:

```cpp
    app->render.render(encoder, app->console.backbuffer(),
        app->console.msaa_color_view(),   // B10: null at msaa=1
        app->console.depth_view());
```

Recorded as a gap, not a repair: this ribbon row's `enabled` column is
`ROSTER.ribbon`, unlike the `true` on `MotionDrivers`, so its per-frame status
is roster-gated. `ROSTER` is resolved in
`src/cartridges/the_board/contracts/roster.hpp` from the compile-time demo
column, and this unit did not evaluate it (build forbidden).

#### A.5 Which headers `cartridge.hpp` includes from `src/coupling/`, `src/musical/`, `src/analysis/`

Recipe: `rg -n '^\s*#include\s*"(coupling|musical|analysis)/' src/cartridges/the_board/cartridge.hpp`
→ **exactly one hit.**

| directory | direct includes in `cartridge.hpp` |
| --- | ---: |
| `src/coupling/` | 1 — `coupling/visual_canvas.hpp` |
| `src/musical/` | **0** |
| `src/analysis/` | **0** |

The quoted-include block verbatim. `grep -c '^\s*#include' src/cartridges/the_board/cartridge.hpp`
→ `51`; the **50** lines below are the whole top-of-file block, unedited, and
diff byte-for-byte against `sed -n '49,98p' src/cartridges/the_board/cartridge.hpp`.
The 51st include is not in this block and is named immediately after it:

```cpp
#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "core/boot_params.hpp"                                    // DOMESDAY_1 B9 — ?seed= / ?mood= boot overrides (ctor, the one authoring site)
#include "core/instruments.hpp"                                    // THE INSTRUMENTS DIAL: INSTRUMENTS.frame_meter / .periodic_census gate the recurring self-measurement (compile-time, T7_INSTRUMENTS; default off)
#include "cartridges/the_board/contracts/roster.hpp"
#include "cartridges/the_board/demos/demo.hpp"             // THE SELECTED SENTENCE: DEMO + ROSTER (compile-time, INCUBATE_DEMO; default full)
#include "cartridges/the_board/primitives/seed_utils.hpp"           // hash/gaussian/tier-select helpers (pure-math leaf)
#include "cartridges/the_board/contracts/ground_architecture.hpp"  // ground contributor/policy tables + compile-time DAG checks
#include "cartridges/the_board/contracts/entity_types.hpp"         // THE CONTRACT HOME: pipeline contracts + boundary DTOs + queue unions + dispatch row/table decl
#include "cartridges/the_board/contracts/indoor_module.hpp"        // THE INDOOR MODULE: mood's insert on the spawn chain — one policy table + three dials; consumers ride the cohort (grounded/floaters/ribbon/the machine)
#include "cartridges/the_board/contracts/spawn_services.hpp"      // THE MACHINE'S DECL TIER: spawn/pipeline service decls + boundary DTOs + arch vocabulary + MIN_SEPARATION (bodies ride the merged machine headers at the cohort tail)
#include "cartridges/the_board/contracts/mood_constants.hpp"       // MOOD_COUNT + the Mood IDs + PortalDestination
#include "cartridges/the_board/contracts/spine_state.hpp"          // TimeState + PlayerState + TransitionPhase + InputState + MoodState/MoodProfile/MOOD_TABLE + the request door decl (spine organ TYPES; instances stay at the root)
#include "cartridges/the_board/contracts/point.hpp"                // THE POINT: the parent of the player system — host enum + the bubble decl; instance at the root
#include "cartridges/the_board/contracts/control_panel.hpp"        // THE PANEL: the field's dials + the beacon rests — one home, every room
#include "cartridges/the_board/contracts/driver_surface.hpp"       // THE DRIVERS' ROOM: rests and gains at the seams; phase_motion_drivers reads DRIVER_LIVE.fog
#include "cartridges/the_board/contracts/floaters.hpp"   // floater TYPES (ActiveSphere/ActiveCube), file scope
#include "cartridges/the_board/realization/state.hpp"
#include "console/organ_registry.hpp"   // the compiled dial registry + its C ABI (needs the home types above)
#include "cartridges/the_board/surface/population_themes.hpp"  // S2: THEMES + ThemeEnvelope + ThemesState — MERGED single file
#include "cartridges/the_board/contracts/surface_services.hpp"  // THE SURFACE'S DECL TIER: WorldState + the patch registry + budgets/visibility + PatchSystemState + the surface service decls (bodies ride surface/patch_system.hpp at the cohort tail)
#include "cartridges/the_board/surface/tile_world.hpp"          // S2: archetypes + tokens + TileState/cache + TileWorldDeps + impl — MERGED single file; after patch_system for WorldState/Dim::PATCH_EXTENT
#include "cartridges/the_board/bodies/grounded.hpp"             // grounded-family vocabulary + EntitiesState + impl — MERGED; after entity_pipeline for generic_*
#include "cartridges/the_board/bodies/agents.hpp"               // AgentState + AgentsDeps + impl — MERGED; after entities for COLUMN_PALETTE
#include "cartridges/the_board/bodies/cube_behaviors.hpp"       // CubeBehaviorsState + CubeDeps + impl — MERGED; after agents for AgentState
#include "cartridges/the_board/bodies/spheres.hpp"              // SphereState + SphereDeps + impl — MERGED single file; after entity_pipeline for the generic funnels
#include "cartridges/the_board/realization/renderer.hpp"
#include "cartridges/the_board/realization/drawable_table.hpp"  // The drawable table (one row per drawable; the two passes iterate it filtered) — after renderer/state, before render_passes
#include "cartridges/the_board/bodies/pawn.hpp"                 // PawnState + PawnDeps + impl — MERGED single file; after renderer for Renderer/GPUState complete
#include "cartridges/the_board/bodies/orbs.hpp"                 // OrbsState + OrbsDeps + impl — MERGED; after renderer for Renderer
#include "cartridges/the_board/bodies/gol_zones.hpp"            // GoLState + GolDeps (S5 device) + impl — MERGED; after renderer/machine/tile
#include "coupling/visual_canvas.hpp"
#include "cartridges/the_board/bodies/ribbon.hpp"               // RibbonState + RibbonDeps + impl — MERGED; after visual_canvas for the coupling face; after agents/cubes/spheres for the FIELD_2 mirror deps
#include "cartridges/the_board/direction/input.hpp"             // KeyState/MouseState + InputDeps + impl — MERGED; after ribbon for RibbonState (the sky fixture); InputState graduated to spine_state
#include "cartridges/the_board/realization/render_passes.hpp"   // the pass/dispatch bodies on THE MACHINE FACE + light-VP helpers — MERGED; before mood (compute_spot_light_vp)
#include "cartridges/the_board/direction/mood.hpp"              // MoodDeps + portal/palette vocabulary + impl — MERGED; after ribbon/input (fan targets), before the machine natives (they call its derivers); MoodState/MoodProfile/MOOD_TABLE graduated to spine_state
#include "cartridges/the_board/machine/spawn_engine.hpp"        // S3: proximity tables + footprints + SpawnEngineState + the preamble template + impl — MERGED; after entities/renderer for complete organs; decl tier in contracts/spawn_services.hpp
#include "cartridges/the_board/machine/entity_pipeline.hpp"     // S3: the three-phase verbs + the welded four — MERGED; after spawn_engine (services) + entities (vocab)
#include "cartridges/the_board/surface/patch_system.hpp"        // S2: the active-patch machine's bodies on THE MACHINE FACE — MERGED; decl tier in contracts/surface_services.hpp
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <string>
#include <vector>
```

**The 51st `#include`, named so the 51-vs-50 arithmetic closes.**
Recipe: `rg -n 'organ_boundary' src/cartridges/the_board/cartridge.hpp` → one hit,
`#include "cartridges/the_board/organ_boundary.inc"`. It is indented and sits at
**class scope inside `class t7::the_board::Cartridge`**, between the member
function `Cartridge::phase_clear_input_deltas(UpdateCtx&)` and the member function
`Cartridge::update(const AnalysisSignal&, float, wgpu::Queue&) override`. Its
banner and the include, verbatim:

```cpp
            // ORGAN — the frame boundary: doors, definition re-speaks, the masks,
            // the rule window, the flush. Member functions, in their own file.
            #include "cartridges/the_board/organ_boundary.inc"
```

and the two member functions that bracket it, verbatim, so the scope is evidenced
rather than asserted:

```cpp
            void phase_clear_input_deltas(UpdateCtx&) {
                clear_input_deltas(&input_deps_);
            }
```

```cpp
            // ── THE CONDUCTOR (update) — a LOOP over UPDATE_SPINE (§1a) ─────
            void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {
```

It names none of `src/coupling/`, `src/musical/` or `src/analysis/`, so the
directory table above is unchanged by it. The target file exists —
`ls -la src/cartridges/the_board/organ_boundary.inc` → a regular file of 8163
bytes — and it **is** in the 64-file include closure computed in §(C) (entry 37
of the sorted closure list, `src/cartridges/the_board/organ_boundary.inc`), so
this include changes no count in the reachability table either.

**FLAG — verifier characterisation not supported by the tree.** The verifier
recorded this include as sitting "inside a `Cartridge` member function body". The
tree does not support that: the quoted region shows `phase_clear_input_deltas`
closing with its own `}` before the banner, and the `.inc` splices *member
function declarations* into the class body, not statements into a function body.
Recorded here as class scope. The substantive half of the verifier's correction —
that a 51st include exists, sits outside the quoted top-of-file block, and was
left unnamed by the first pass — is CONFIRMED and is repaired above. What it
would have cost to resolve further: nothing; the enclosing scope is settled by
the brace structure quoted above.

`analysis/analysis_signal.hpp` still arrives at `cartridge.hpp` **transitively**,
by two routes: `render/render_cartridge.hpp` (its own line
`#include "analysis/analysis_signal.hpp"`) and `coupling/visual_canvas.hpp`.
`musical/signal_layout.hpp` arrives by one route only: `coupling/visual_canvas.hpp`.
`coupling/visual_canvas.hpp`'s own include block, verbatim:

```cpp
#include "coupling/visual_params.hpp"
#include "coupling/trajectory.hpp"
#include "core/instruments.hpp"   // THE INSTRUMENTS DIAL: INSTRUMENTS.checker_witness gates the [CHECKER] line
#include "musical/signal_layout.hpp"
#include "analysis/analysis_signal.hpp"
#include <string>    // casting-sheet name composition ("<voice>.present_count")
#include <array>     // the hue unit-vector table (OIL_1 U5)
#include <cstddef>   // size_t — the table's index casts (OIL_1 U5)
#include <cmath>     // std::floor / cos / sin / sqrt / atan2 — decode math
#include <algorithm> // std::min/std::max — decode clamps
#include <cstdio>    // std::fprintf — the [CHECKER] witness line
#include "coupling/canvas_surface.hpp"   // ORGAN_3b P2 — CANVAS_LIVE: the envelope authorities' live surface
```

---

### (B) Where is MIDI polled?

#### B.0 Verdict

**REACHED-OFF-FRAME-PATH.**

This is the verdict the unit's own boundary forces. The unit's constraint is
*"Static reachability only — do not build."* Under static reachability over the
tree, `t7::MidiPort::poll` is **defined and called**: exactly one call site
exists, `const int n = port_.poll(beat, ev, 256);` inside
`t7::canvas_1::Canvas::update(float dt) override` in
`src/analysis/canvas_1/canvas.hpp`, on a `MidiPort port_;` private member of
`class t7::canvas_1::Canvas` that is opened in `t7::canvas_1::Canvas::initialize(const char* asset_path) override`
via `port_.open_by_name("loopMIDI");`. `Canvas::update` is not the frame function
of `src/the_board.cpp` (`static void frame()`) and is not reachable from it —
hence OFF-frame-path rather than ON-frame-path, and not DEFINED-UNCALLED, because
a call site exists.

**The second reading, recorded because the two boundaries genuinely disagree and
because the first pass of this section led with it.** Measured against *the
program that is built and linked* — the sole executable target `the_board`, whose
translation units are `src/the_board.cpp` and `src/external/RtMidi.cpp` — the same
evidence reads **DEFINED-UNCALLED**: `Canvas::update`'s translation units
(`probe_canvas.cpp`, `check_canvas_compound.cpp`, `check_canvas_union.cpp`) are in
no CMake target, and no header in the 64-file built closure includes
`analysis/canvas_1/canvas.hpp`. That reading imports CMake target membership,
which is a build fact this unit's boundary excludes; it is recorded here, not
ranked first.

Neither reading is a guess: every fact underlying both is enumerated in §B.2
below and each ships its own recipe. The ranking, not the evidence, is what
changed at amendment time.

#### B.1 The handoff's trace, hop by hop

**Hop 1 — `src/the_board.cpp` → `src/core/cartridge_manager.hpp`: THE CHAIN IS BROKEN HERE.**

`src/the_board.cpp` (blob `588174ecddb0d68388e39a9025d6eda2f2afd000`) does not
include `core/cartridge_manager.hpp`.

The first pass presented a single fenced block here that was neither the labelled
grep's output nor a contiguous file region: it silently elided a five-line
`FILE WATCHER` banner comment sitting between `#include <chrono>` and
`#include <filesystem>`, while also carrying the `#if defined(__INTELLISENSE__)` /
`#else` / `#endif` lines and their three-line comment, which a `#include`-only
grep cannot emit. Both artefacts are given below, each under its own label, and
neither is edited.

**Artefact 1 — the census output, exactly as the recipe prints it.**
Recipe: `rg -n '^\s*#include' src/the_board.cpp` → **10 lines**. The line-number
prefixes are stripped here only because R3 forbids anchoring to them; nothing else
is changed, and no conditional-compilation line appears because the recipe cannot
produce one:

```cpp
#include "console/console.hpp"
#include "analysis/beat_clock.hpp"
#include "cartridges/the_board/cartridge.hpp"
#include RENDER_HEADER(INCUBATE_RENDER)
#include "core/instruments.hpp"   // THE INSTRUMENTS DIAL: INSTRUMENTS.watcher_ticks gates the hot-reload progress dot
#include "core/boot_params.hpp"   // DOMESDAY_1 B9 — parse_boot_params at the top of main
#include <iostream>
#include <chrono>
#include <filesystem>     // restored with the watcher (SUNRISE_0 N1)
#include <system_error>   // std::error_code — the watcher's non-throwing stat
```

**Artefact 2 — the contiguous file region, verbatim and unedited.** This is
`src/the_board.cpp` from its first `#include` down to the last `#include` before
`class FileWatcher {` (the region `sed -n '48,71p' src/the_board.cpp` prints),
reproduced whole so that the `__INTELLISENSE__` conditional and the `FILE WATCHER`
banner are both visible in place:

```cpp
#include "console/console.hpp"
#include "analysis/beat_clock.hpp"

// IntelliSense cannot resolve macro-expanded #include paths.
// This literal include gives VS navigation (Peek Definition, Go To, etc.).
// The compiler ignores it -- the macro include below pulls in the same file.
#if defined(__INTELLISENSE__)
#include "cartridges/the_board/cartridge.hpp"
#else
#include RENDER_HEADER(INCUBATE_RENDER)
#endif

#include "core/instruments.hpp"   // THE INSTRUMENTS DIAL: INSTRUMENTS.watcher_ticks gates the hot-reload progress dot
#include "core/boot_params.hpp"   // DOMESDAY_1 B9 — parse_boot_params at the top of main
#include <iostream>
#include <chrono>

// =========================================================================
// FILE WATCHER -- Detects shader file changes for hot reload
// (R6: a native instrument — the browser has no mtime to watch)
// =========================================================================

#include <filesystem>     // restored with the watcher (SUNRISE_0 N1)
#include <system_error>   // std::error_code — the watcher's non-throwing stat
```

Reading the two together: the quoted-include set of `src/the_board.cpp` is
`console/console.hpp`, `analysis/beat_clock.hpp`,
`cartridges/the_board/cartridge.hpp` (the `__INTELLISENSE__` arm),
`RENDER_HEADER(INCUBATE_RENDER)` (the `#else` arm, which expands to the same
file), `core/instruments.hpp`, `core/boot_params.hpp`, and the four system
headers `<iostream>`, `<chrono>`, `<filesystem>`, `<system_error>`.
`core/cartridge_manager.hpp` is in neither artefact.

`RENDER_HEADER(INCUBATE_RENDER)` expands through
`#define RENDER_HEADER(name)   STRINGIFY(cartridges/name/cartridge.hpp)`, and
`INCUBATE_RENDER` is supplied by CMake as
`INCUBATE_RENDER=${T7_RENDER_CARTRIDGE}` with
`set(T7_RENDER_CARTRIDGE "the_board" …)` — the same file the
`__INTELLISENSE__` branch names literally.

Census for the manager, with its root restated (the first pass ran this repo-wide
at `.`; at the current HEAD that form also matches this recon report,
`docs/LIGATURE_0_RECON.md`, on 12 lines — so the source-tree root is the stable
one). `rg -n 'cartridge_manager' src/` → **exactly one line**, the file's own
header banner:

```
src/core/cartridge_manager.hpp:3:// ─── cartridge_manager.hpp ───────────────────────────────────────
```

Outside `src/`, `rg -n 'cartridge_manager' --glob '!.git' . -c` reports
`./docs/LIGATURE_0_RECON.md:12` (this report), `./src/core/cartridge_manager.hpp:1`
(the banner above) and `./full_list.txt:1` (one line of a Windows path listing,
`C:\dev\7t\src\core\cartridge_manager.hpp`). **No source file in the repository
includes `core/cartridge_manager.hpp`** — the name appears in no `#include`
anywhere.

Frame loop quoted, so the absence is evidenced rather than asserted —
`static void frame()` in `src/the_board.cpp`, the update/render body with the
meter branches elided only where marked:

```cpp
    float dt = app->console.begin_frame();
    // ORGAN — THE DIRTY FLUSH, at the frame boundary and nowhere else.
    // begin_frame has polled events and reconciled, so every writer for this
    // frame has spoken and the panel's edits are bits: this turns them into
    // at most one WriteBuffer per block (docs/ORGAN.md).
    app->render.organ_flush(app->queue);
```

```cpp
    // --- Input (all of it is the world's) --------------------------------
    for (const auto& event : app->console.input_events()) {
        app->render.on_input(event);
    }
    app->console.clear_input_events();

    // --- Update ---------------------------------------------------------
    app->clock.update(dt);
    app->render.update(app->clock.output(), app->console.aspect_ratio(), app->queue);
```

and the loop-carried state it drives, `struct App`:

```cpp
struct App {
    t7::Console console;
    t7::BeatClock clock;
    RenderCartridge render;
    FileWatcher watcher;
    int reload_frame_counter = 0;
    wgpu::Queue queue;
    bool world_ready = false;   // PORT_1c: init_world() ran (post-device init)
    // U9's two: when the frame loop went live, and whether the offer is spent.
    // One-shot by construction — once printed, the test is never evaluated again.
    std::chrono::steady_clock::time_point world_live{};
    bool controls_offered = false;
};
```

There is no `MidiPort`, no `MidiEvent`, no `poll` on this path. The clock member
is `t7::BeatClock`, whose whole update is:

```cpp
    void update(float dt) {
        signal_.dt = dt;
        signal_.t_seconds += dt;
        signal_.t_beats += dt * (bpm / 60.0f);
    }
```

**Hop 2 — what `src/core/cartridge_manager.hpp` exposes.**

Blob `1de6479a13627e4385741c3f16f4a0ce4aef5685`, 337 lines. It defines
`class t7::CartridgeManager` with public members `init(wgpu::Device,
wgpu::TextureFormat, wgpu::TextureFormat)`, `active()`, `active_id()`,
`transition_from(uint32_t, uint32_t, bool)`, `transition_to(uint32_t)`,
`return_to_hub(wgpu::Queue)`, `return_to_previous()`, and a private
`static const char* id_name(uint32_t)`. **It exposes no MIDI surface at all** —
`rg -i -n '\bmidi\b' src/core/cartridge_manager.hpp` → zero hits.

Its own include block, verbatim:

```cpp
#include "core/cartridge_ids.hpp"
#include "render/render_cartridge.hpp"

#include "cartridges/gallery_raymarch/cartridge.hpp"
#include "cartridges/gallery/cartridge.hpp"
#include "cartridges/n_dimensional_2/cartridge.hpp"
#include "cartridges/the_board/cartridge.hpp"
#include "cartridges/world_compute/cartridge.hpp"
#include "cartridges/terrain_pawn/cartridge.hpp"
#include "cartridges/playground_hybrid/cartridge.hpp"
#include "cartridges/species_studio/cartridge.hpp"
#include "cartridges/pawn_rasterize/cartridge.hpp"

#include <memory>
#include <iostream>
```

Recorded as a gap: `ls src/cartridges/` returns exactly one entry, `the_board`.
Eight of those nine includes name directories that do not exist in the tree, so
the header cannot be compiled as written. Its private data members
(`std::unique_ptr<gallery_raymarch::Cartridge> galleryRaymarch_;` and eight
siblings) and its `switch (target)` arms name the same absent types.

**Hop 3 — `src/sources/midi_port.hpp`: the poll entry point.**

Blob `293ce7c46f669185235e2e6121d48f0a9863563e`, 213 lines. The entry point is
`t7::MidiPort::poll`, verbatim:

```cpp
    // ── POLL — drain note events, stamp with current_beat ─────────

    int poll(float current_beat, MidiEvent* out, int max_out) {
        int count = 0;
        const uint32_t write = write_idx_.load(std::memory_order_acquire);
        uint32_t read = read_idx_.load(std::memory_order_relaxed);
        while (read != write && count < max_out) {
            out[count] = ring_[read & RING_MASK];
            out[count].beat = current_beat;
            ++read;
            ++count;
        }
        read_idx_.store(read, std::memory_order_release);
        return count;
    }
```

The producer side is `t7::MidiPort::handle_message(double, const std::vector<unsigned char>&)`,
driven by the RtMidi callback `t7::MidiPort::on_rtmidi_callback` installed in
`t7::MidiPort::open(unsigned int)` via
`midi_in_->setCallback(&MidiPort::on_rtmidi_callback, this);`.

#### B.2 The evidence chain that forces the verdict

1. **`MidiPort::poll` has exactly one call site in the tree.**
   Recipe: `rg -F -n ".poll(" src/` → 3 lines, of which two are usage prose
   (`src/sources/keyboard_midi.hpp` header comment,
   `src/sources/midi_file.hpp` header comment). The single call is
   `src/analysis/canvas_1/canvas.hpp` :: `t7::canvas_1::Canvas::update(float dt) override`:

```cpp
    void update(float dt) override {
        dt_         = dt;          // wall-clock delta, telemetry only
        t_seconds_ += dt;
        const float beat = static_cast<float>(port_.beats());   // the DAW's clock
        MidiEvent ev[256];
        const int n = port_.poll(beat, ev, 256);

        for (int i = 0; i < n; ++i) route(ev[i]);
        advance(beat);
    }
```

   with the port owned as a private member of `class t7::canvas_1::Canvas`:

```cpp
    MidiPort port_;             // the DAW's MIDI port, owned and drained each frame
```

   and opened in `t7::canvas_1::Canvas::initialize(const char* asset_path) override`:

```cpp
        port_.open_by_name("loopMIDI");   // the DAW's virtual port

        std::fprintf(stderr, "[canvas] loopMIDI open=%d\n", (int)port_.is_open());
```

2. **`MidiPort` is named in exactly three files under `src/`, on nine lines.**
   Recipes: `rg -l 'MidiPort' src/` → three files;
   `rg -c 'MidiPort' src/sources/midi_port.hpp src/analysis/canvas_1/canvas.hpp src/analysis/canvas_1/probe_canvas.cpp`
   → `7`, `1`, `1`.

   | file | lines | the lines, by enclosing symbol |
   | --- | ---: | --- |
   | `src/sources/midi_port.hpp` | 7 | `class MidiPort {` (the definition, in `namespace t7`); `MidiPort() {` (default ctor); `~MidiPort() { close(); }`; `MidiPort(const MidiPort&) = delete;`; `MidiPort& operator=(const MidiPort&) = delete;`; `midi_in_->setCallback(&MidiPort::on_rtmidi_callback, this);` inside `t7::MidiPort::open(unsigned int)`; `static_cast<MidiPort*>(user)->handle_message(deltatime, *msg);` inside `t7::MidiPort::on_rtmidi_callback` |
   | `src/analysis/canvas_1/canvas.hpp` | 1 | `MidiPort port_;` — private data member of `class t7::canvas_1::Canvas` |
   | `src/analysis/canvas_1/probe_canvas.cpp` | 1 | file-header comment, "Needs RtMidi and the transport-aware MidiPort." |

   The first pass reported "6 lines" for `midi_port.hpp`; **7** is the corrected
   figure and the per-line enumeration above accounts for each.

   **Census-boundary repair (R2).** The first pass's recipe was rooted at the
   repo (`rg -n 'MidiPort' --glob '!.git' --glob '!full_list.txt' .`). That form no
   longer returns three files: at the current HEAD it also matches
   `docs/LIGATURE_0_RECON.md` — this recon report itself, added by commit
   `6d53388e` — on 23 lines (`rg -c 'MidiPort' docs/LIGATURE_0_RECON.md` → `23`).
   The root is therefore restated as `src/`, matching the root declared at the top
   of this section, and under that root the three-file / nine-line result is exact
   and stable.

3. **`sources/midi_port.hpp` is included by exactly one file.**
   Recipe (root restated as `src/`, for the reason given in item 2):
   `rg -n 'midi_port' src/` → 3 lines in 2 files —
   `src/sources/midi_port.hpp` :: file-header banner
   (`// ─── midi_port.hpp  (dev: transport-aware) ───`), and
   `src/analysis/canvas_1/canvas.hpp` :: file header, twice — its `Depends on:`
   note and the line `#include "sources/midi_port.hpp"`. No other file in `src/`
   names the path.

4. **`analysis/canvas_1/canvas.hpp` is included only by files with no build target.**
   Recipe: `rg -n '^\s*#include' src/analysis/canvas_1/*.cpp` →
   `probe_canvas.cpp`, `check_canvas_compound.cpp` and `check_canvas_union.cpp`
   each open with `#include "canvas.hpp"` (relative form — a
   `rg -n 'canvas_1/canvas.hpp'` sweep alone would miss these and report a false
   zero; noted so the next reader does not repeat it). `check_field_union.cpp`
   and `check_pc_dft.cpp` include only `musical/field.hpp` and
   `musical/pc_dft.hpp` respectively. No `.hpp` in the tree includes
   `canvas_1/canvas.hpp`.

5. **None of those `.cpp` files is in any build target.**
   Recipe: `rg -n 'add_executable|add_library|add_custom_target' CMakeLists.txt`
   → `add_executable(the_board` and `add_custom_target(t7_build_stamp`;
   `find . -name CMakeLists.txt -not -path './.git/*' -not -path './third_party/*'`
   → one file. The executable's sources, verbatim:

```cmake
add_executable(the_board
    src/the_board.cpp
    # The one other translation unit: RtMidi's Windows MM backend, the
    # canvas's route to the DAW's virtual port. Vendored, not header-only,
    # so it compiles separately; __WINDOWS_MM__ below selects its backend.
    src/external/RtMidi.cpp
    ${T7_RENDER_HEADERS}
)
```

   `T7_RENDER_HEADERS` is `file(GLOB_RECURSE … "src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp")`
   — headers listed for IDE indexing, not compiled TUs.
   `src/external/RtMidi.cpp` is therefore compiled and linked into a program in
   which no caller of `RtMidiIn` survives; that is a fact of the target list, not
   an inference.

6. **The word MIDI does not appear as code anywhere in the built closure — four
   prose hits, no code hits.**

   **Census boundary, stated exactly (R2 repair).** The first pass wrote the
   boundary as "the closure's key files", which names no file set and is therefore
   not reproducible; it also under-counted. The boundary is the **64-file include
   closure of `src/the_board.cpp`** computed by the walker in §(C) below — the
   same 64 files, no subset. Recipe, in two steps from `/home/user/7T-Music`:

```sh
# step 1 — write the closure, one path per line (the walker of §(C), sorted)
python3 - <<'EOF' > /tmp/closure.txt
import os,re
inc=re.compile(r'^\s*#\s*include\s*"([^"]+)"')
def resolve(cur,t):
    d=os.path.dirname(cur)
    for c in (os.path.normpath(os.path.join(d,t)), os.path.normpath(os.path.join('src',t))):
        if os.path.isfile(c): return c
    return None
seen=set(); order=[]
def walk(f):
    if f in seen: return
    seen.add(f); order.append(f)
    for ln in open(f,encoding='utf-8',errors='replace').read().splitlines():
        m=inc.match(ln)
        if m:
            r=resolve(f,m.group(1))
            if r: walk(r)
walk('src/the_board.cpp')
for f in sorted(order): print(f)
EOF

# step 2 — the census, over exactly those 64 files
rg -i -n '\bmidi\b' $(cat /tmp/closure.txt)
```

   Result — **4 lines, in 4 files, every one a comment**:

   | file :: enclosing symbol | the line |
   | --- | --- |
   | `src/coupling/visual_params.hpp` :: file header comment | `//   The analysis signal is a channel × slot grid because MIDI channels are` |
   | `src/coupling/visual_canvas.hpp` :: file header comment | `// parameter bank; it touches no GPU. Where the musical Canvas reads MIDI,` |
   | `src/analysis/beat_clock.hpp` :: file header comment | `// CUT_1c: the analysis intake's successor (ruling R7). MIDI/DAW` |
   | `src/cartridges/the_board/bodies/cube_behaviors.hpp` :: comment above the seed-independence note | `// world-seed-independent (same MIDI, same screen).` |

   The first pass named only the middle two; `visual_params.hpp` and
   `cube_behaviors.hpp` are restored above. The substantive finding is unchanged
   and is CONFIRMED: **all four hits are prose; not one is a declaration, a type
   use, a call, or a macro.** The `beat_clock.hpp` banner states the same fact this
   section measures:

```cpp
// CUT_1c: the analysis intake's successor (ruling R7). MIDI/DAW
// analysis left the build; the render side keeps its two contracts —
// an AnalysisSignal each frame and a StatLayoutView once at bind.
```

7. **No caller of RtMidi survives inside the built closure — ZERO HITS.**
   Recipe, over the same 64-file closure as item 6:
   `rg -n 'RtMidiIn' $(cat /tmp/closure.txt) | wc -l` → **`0`**. This is the
   counterpart fact to item 5: `src/external/RtMidi.cpp` is one of the two
   translation units of the `the_board` target, yet the type it exists to provide
   is named nowhere in the closure that target's other translation unit pulls in.
   A zero is a finding, and it is recorded as one; the census root is stated so the
   zero is reproducible rather than asserted.

8. **The single `.poll(` census, restated with its root.**
   Recipe: `rg -F -n ".poll(" src/` (fixed-string flag required — `.` and `(` are
   regex metacharacters) → **3 lines**:

```
src/analysis/canvas_1/canvas.hpp:155:        const int n = port_.poll(beat, ev, 256);
src/sources/keyboard_midi.hpp:28://   int count = keyboard.poll(events, 32);       // each frame
src/sources/midi_file.hpp:24://   int count = midi.poll(prev_beat, current_beat, events, 64);
```

   One call (`t7::canvas_1::Canvas::update(float dt) override`) and two usage-prose
   comments (`src/sources/keyboard_midi.hpp` and `src/sources/midi_file.hpp` file
   headers, neither of which is included by anything — see item 4 and §(C)). This
   is the census that forces the REACHED half of the §B.0 verdict.

---

### (C) Directory reachability from `src/the_board.cpp`'s include graph

Method (static, no build): a Python walker over quoted `#include "…"` lines
starting at `src/the_board.cpp`, resolving each target first relative to the
including file's directory and then against `src/`, recursing depth-first over
the transitive closure. It reached **64 files**. Two quoted includes were
unresolvable inside `src/` and are named as such, not silently dropped:
`src/core/instruments.hpp -> build_stamp.gen.inc` (CMake writes it into the
build directory, `${CMAKE_CURRENT_BINARY_DIR}/gen/build_stamp.gen.inc`) and
`src/console/console.hpp -> dawn/common/Version_autogen.h` (Dawn, out of tree).

Stated limitation of the method: the walker does **not** evaluate preprocessor
conditionals, so it follows both arms of `#if defined(__INTELLISENSE__)` in
`src/the_board.cpp`. Here both arms name the same file
(`cartridges/the_board/cartridge.hpp`, the second via
`RENDER_HEADER(INCUBATE_RENDER)` with `INCUBATE_RENDER=the_board` from CMake),
so no reached file in the table below depends on that limitation.

| directory | any file reached? | reached / total | the chain, or the break point |
| --- | --- | ---: | --- |
| `src/musical/` | **YES — one file only** | 1 / 16 | `src/the_board.cpp` → `cartridges/the_board/cartridge.hpp` → `coupling/visual_canvas.hpp` → `musical/signal_layout.hpp`. **Break point:** `signal_layout.hpp` includes only `analysis/analysis_signal.hpp`, `<string_view>`, `<cstdio>`, so the chain stops there. The other 15 (`context.hpp`, `context_realize.hpp`, `context_spec.hpp`, `field.hpp`, `midi_stream.hpp`, `musical_ops.hpp`, `pc_count.hpp`, `pc_dft.hpp`, `playhead.hpp`, `previous_event.hpp`, `spine.hpp`, `spine_ops.hpp`, `stream_data.hpp`, `vector_dressing.hpp`, `wagon.hpp`) hang off `analysis/canvas_1/canvas.hpp` and `analysis/canvas_1/check_*.cpp`, none of which is reached. |
| `src/analysis/` | **YES — two files** | 2 / 9 | `src/the_board.cpp` → `analysis/beat_clock.hpp` → `analysis/analysis_signal.hpp` (and `analysis_signal.hpp` again via `render/render_cartridge.hpp` and via `coupling/visual_canvas.hpp`). **Break point:** `analysis/analysis_cartridge.hpp` and `analysis/canvas_1/*` are reached by nothing — the only includer of `analysis_cartridge.hpp` is `canvas_1/canvas.hpp`, whose only includers are the three unbuilt `canvas_1/*.cpp` files. |
| `src/sources/` | **NO** | 0 / 5 | **Break point:** the sole includer of `sources/midi_port.hpp`, `sources/midi_event.hpp` and (transitively) `sources/transport.hpp` / `external/RtMidi.h` is `analysis/canvas_1/canvas.hpp`, which is not in the closure. `sources/keyboard_midi.hpp` and `sources/midi_file.hpp` have no includer anywhere in `src/`. |
| `src/coupling/` | **YES — four of five** | 4 / 5 | `src/the_board.cpp` → `cartridges/the_board/cartridge.hpp` → `coupling/visual_canvas.hpp`, which pulls `coupling/visual_params.hpp`, `coupling/trajectory.hpp` and `coupling/canvas_surface.hpp`. **Break point:** `src/coupling/organ_registry.hpp` (999 lines, blob `3047070e199df57c2a7cd6d8f75cf028ec48b817`) is included by nothing; `cartridge.hpp` instead includes `console/organ_registry.hpp` (974 lines), a near-twin — `diff` between the two prints 249 lines. |

For completeness, the two other `src/` directories in the closure:
`src/core/` reached 4 / 6 (`boot_params.hpp`, `cartridge_ids.hpp`,
`input_event.hpp`, `instruments.hpp`; **not** reached: `core/cartridge_manager.hpp`,
`core/types.hpp`), and `src/console/` reached 5 / 5.

Exact recipe for this table (written to the scratchpad only, never to the repo):

```sh
python3 - <<'EOF'
import os,re
inc=re.compile(r'^\s*#\s*include\s*"([^"]+)"')
def resolve(cur,t):
    d=os.path.dirname(cur)
    for c in (os.path.normpath(os.path.join(d,t)), os.path.normpath(os.path.join('src',t))):
        if os.path.isfile(c): return c
    return None
seen=set(); order=[]
def walk(f):
    if f in seen: return
    seen.add(f); order.append(f)
    for ln in open(f,encoding='utf-8',errors='replace').read().splitlines():
        m=inc.match(ln)
        if m:
            r=resolve(f,m.group(1))
            if r: walk(r)
walk('src/the_board.cpp')
EOF
```

---

### Gaps recorded (no repairs proposed, R4)

- `Trajectory` (capitalized) exists nowhere under `src/`; `src/coupling/trajectory.hpp`
  exports `Segment`, `plan_segment`, `sample_segment`, `trajectory_release`.
- `src/core/cartridge_manager.hpp` has zero includers repo-wide and includes
  eight cartridge headers whose directories are absent from `src/cartridges/`
  (only `the_board` exists).
- `src/sources/` (5 files, including `midi_port.hpp`, `midi_file.hpp`,
  `keyboard_midi.hpp`, `transport.hpp`, `midi_event.hpp`) is entirely outside
  the built program's include closure.
- `src/analysis/canvas_1/` (1 header + 5 `.cpp`) is outside the closure and
  outside every CMake target; the three `.cpp` files that name `Canvas` are
  compiled by nothing.
- 15 of 16 files in `src/musical/` are outside the closure.
- `src/coupling/organ_registry.hpp` has zero includers; a 974-line near-twin at
  `src/console/organ_registry.hpp` is the one `cartridge.hpp` includes.
- `src/core/types.hpp` has no includer in the closure.
- `src/external/RtMidi.cpp` is a compiled TU of the `the_board` target while no
  caller of `RtMidiIn` exists anywhere in that target's include closure.
- `Cartridge::bind_signal_layout` is not a virtual of `t7::RenderCartridge`; it
  is reached only because `src/the_board.cpp` aliases the concrete cartridge type.
- The `StatLayoutView` handed to `bind()` is `{ nullptr, 0 }` by construction
  (`BeatClock::stat_layout()`), so all 12 source-side resolves inside
  `VisualCanvas::bind` (six call sites, one of them a seven-iteration ear loop)
  return `valid = false`.
- `t7::MidiPort::poll` is defined and has exactly one static call site in the
  tree — `t7::canvas_1::Canvas::update(float dt) override` — and that caller sits
  outside the 64-file include closure of `src/the_board.cpp` and outside every
  CMake target. Under the unit's static-reachability boundary that is
  REACHED-OFF-FRAME-PATH; under a build-target boundary it is DEFINED-UNCALLED.
  Both are recorded in §B.0; the tree supports both and the boundary decides.
- `src/cartridges/the_board/cartridge.hpp` carries a 51st `#include` at class
  scope inside `class t7::the_board::Cartridge`
  (`#include "cartridges/the_board/organ_boundary.inc"`), splicing member
  functions in from a `.inc` file rather than a `.hpp`. It resolves, and it is
  inside the built closure.
- `src/coupling/trajectory.hpp` names `trajectory_release` on 2 of its 4
  case-insensitive `trajectory` lines (the `FOLLOW` banner and the definition),
  which is why a repo-wide fixed-string sweep for `trajectory_release(` returns
  12 lines while the call-site count is 10.
- Three census recipes published in the first pass were rooted at the repository
  (`.`) rather than at `src/`. At the current HEAD those forms also match
  `docs/LIGATURE_0_RECON.md`, the recon report added by commit `6d53388e`:
  `MidiPort` 23 lines, `bind_signal_layout` 42 lines, `cartridge_manager` 12
  lines. Every affected recipe in this section has been restated with the `src/`
  root; the underlying source-tree facts did not change.

---

### Amendment log (what changed at verification, and why)

This section was re-censused against the tree after an adversarial verification
pass. Nothing below was accepted on the verifier's word; each row was re-run here
before it was written.

| where | first pass said | the tree says | status |
| --- | --- | --- | --- |
| §2.11 summary | 24 non-comment `.resolve(`, 16 in `VisualCanvas::bind` | 22 non-comment, 14 in `VisualCanvas::bind` (26 total − 4 comment lines) | corrected; the section's own table had 14 already, so the summary had contradicted it |
| §2.12 line count | `rg -i -n "trajectory" src/` → 26 lines | **28** lines (file count 9 was correct) | corrected, with a per-file table |
| §2.12 prose-hit files | eight files enumerated | nine files; `src/cartridges/the_board/bodies/pawn.hpp` was dropped | corrected, row restored |
| §2.12 call sites | `trajectory_release` has 12 call sites | **10** call sites, all inside `t7::VisualCanvas::tick` | corrected; the "all inside `tick`" half CONFIRMED |
| §2.12 (verifier's own figure) | — | verifier said repo-wide sweep = 11 lines; it is **12** (banner *and* definition) | verifier amended, flagged in place |
| §A.2 | chain entered only from `main()` | `static bool init_world()` has **two** callers: `main()` and `static void frame()` | corrected, both quoted |
| §A.5 | "51 `#include` lines … this is the whole block" | the block is **50**; the 51st is `organ_boundary.inc` at class scope | corrected and named |
| §A.5 (verifier's own claim) | — | verifier placed the 51st "inside a member function body"; the braces show **class scope** | verifier amended, flagged in place |
| §B.0 | led with DEFINED-UNCALLED | under the unit's static-reachability boundary the forced verdict is **REACHED-OFF-FRAME-PATH** | prominence swapped; both readings kept, evidence unchanged |
| §B.1 hop 1 | one fenced block labelled as the `#include` grep | the block was a composite (elided a 5-line banner, added `#if`/`#else`/`#endif`) | R5 repair: replaced with two separately labelled artefacts, both unedited |
| §B.2 item 2 | `midi_port.hpp` names `MidiPort` on 6 lines | **7** lines; three files total CONFIRMED | corrected, enumerated line by line |
| §B.2 item 6 | "two hits over the closure's key files" | **four** hits over the stated **64-file** closure, all four prose | R2 repair (boundary named) + count corrected; substantive finding unchanged |
| §B.2 | no explicit RtMidi census | `rg -n 'RtMidiIn'` over the 64-file closure → **0** | added as item 7 (a zero is a finding) |
| §B.1 / §A.2 / §B.2 | three recipes rooted at `.` | restated at `src/`; the `.` forms now also match `docs/LIGATURE_0_RECON.md` | R2 repair |

Confirmed unchanged by the re-census, and re-derived here rather than carried
over: all twelve needle counts in §2.0 (`visual_canvas_` 22, `VisualCanvas` 6,
`VisualParams` 11, `ParamLayout` 11, `TargetBinding` 29, `ParamSlot` 10,
`SignalLayout` 8, `SourceBinding` 15, `AnalysisSignal` 20 lines / 22 occurrences,
`signal_layout_` 11, `.resolve(` 26 lines, `Trajectory` **0**); the 17 blob SHAs;
the §(C) walker result (64 files reached; `musical` 1/16, `analysis` 2/9,
`sources` 0/5, `coupling` 4/5, `core` 4/6, `console` 5/5) and its two
unresolvable includes (`build_stamp.gen.inc`, `dawn/common/Version_autogen.h`);
and the single-CMakeLists / two-translation-unit target facts.

## 3. Pipe table + dormant surface

Repo `/home/user/7T-Music`, branch `claude/ligature-0-recon-hcrix0`.

**HEAD, restated after re-verification.** The census below was first run at
`79adfa4d26c9e17e0074692928f1d2875d7edde1` ("Systems operational"). At
re-verification `git rev-parse HEAD` returns
`6d53388e83f4a5cd7ad3b154484c885f567a02da` ("LIGATURE_0 — the recon report: the
ligature is one hop, and the socket is empty"), which has `79adfa4d` as its
parent (`git log --oneline -3` → `6d53388e`, `79adfa4d`, `72df32df`). The whole
difference between the two commits is one added documentation file:

```
git diff --stat 79adfa4d 6d53388e
#  docs/LIGATURE_0_RECON.md | 8837 ++++++++++++++++++++++++++++++++++++++++++++++
#  1 file changed, 8837 insertions(+)
```

No source blob moved between them, and every blob SHA in §3.0 was re-checked
against `6d53388e` with `git rev-parse HEAD:<path>` and is unchanged. Every count
in this section was re-run at `6d53388e` and reproduces, with the two recipe
defects and one false claim named in §3.12.

Working tree clean at entry, at exit, and at re-verification
(`git status --porcelain` → 0 lines each time; no modified, staged, or untracked
file anywhere in the repo). Read-only throughout.

### 3.0 Anchors (R6)

All sixteen verify at both `79adfa4d` and the current `6d53388e`; the SHAs below
are the current ones, printed by `git rev-parse HEAD:<path>`.

| path | blob SHA (`git rev-parse HEAD:<path>`) |
| --- | --- |
| `src/coupling/visual_params.hpp` | `c196529d08b9b815a4b4282e7dc2695661febbc5` |
| `src/coupling/visual_canvas.hpp` | `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35` |
| `src/cartridges/the_board/realization/state.hpp` | `fe4bce836b4665588ed43a0729d312e89cd05a20` |
| `src/cartridges/the_board/realization/world.wgsl` | `5b36243dc6b45e27271d0d73eca8a01eb5dc2078` |
| `src/cartridges/the_board/cartridge.hpp` | `3651bcabaa0b02a2925ad6868ce541ea9ab1b202` |
| `src/cartridges/the_board/bodies/ribbon.hpp` | `0c662c8e1454144d85f0074c70aaa97beee2087f` |
| `src/cartridges/the_board/contracts/driver_surface.hpp` | `b0e478dba4d458b4d96c7fee0be04cd393bc58fb` |
| `src/cartridges/the_board/contracts/control_panel.hpp` | `c40a926f5a6d2d23a58ac434e892aac2aa0ecea9` |
| `src/coupling/canvas_surface.hpp` | `336d5d320f3a0337a78568f5762eae5885022109` |
| `src/console/organ_params.inc` | `b426ac4f2b88f89b02f9a9d2236d14b992d93c7f` |
| `src/console/organ_registry.hpp` | `70d09e9602eb0f763a616da5303e14c34e7f44da` |
| `src/coupling/organ_registry.hpp` | `3047070e199df57c2a7cd6d8f75cf028ec48b817` |
| `src/musical/signal_layout.hpp` | `8e2e84312483e31e429276d91c23f7d63dc2643c` |
| `src/analysis/beat_clock.hpp` | `b10038ff5069783c6be15e1e8d885d36238f7354` |
| `src/analysis/analysis_signal.hpp` | `d088796d0ece785b9e34ee071273d6c5df7ce4a4` |
| `src/the_board.cpp` | `588174ecddb0d68388e39a9025d6eda2f2afd000` |

### 3.1 Where the ParamLayout actually lives (correction to the unit's premise)

`src/coupling/visual_params.hpp` declares **no pipes**. It is infrastructure only:
`VisualParams` (the flat 256-float bank), `ParamSlot`, `ParamLayoutView`,
`TargetBinding`, and `class ParamLayout` (`bind` / `reset` / `resolve` / `count` /
`slots`). Its own header comment says so:

> The static PARAM_LAYOUT array (the master control panel) declares every
> pipe's name, slot, width, and rest value; this header is only the
> infrastructure that addresses it.

The pipe declarations are the `inline constexpr ParamSlot PARAM_LAYOUT[]` array in
`src/coupling/visual_canvas.hpp`. Census recipe — **scoped to the program's own
sources, `src/ tools/`, deliberately and not to `docs/`**:

```
git grep -n "PARAM_LAYOUT\|ParamSlot\|ParamLayoutView\|param_layout" -- src/ tools/
```
→ 37 hits, of which exactly one is an array definition site. Isolating the
definition:

```
git grep -n "ParamSlot [A-Z_]*\[\]" -- src/ tools/
#   src/coupling/visual_canvas.hpp:232:    inline constexpr ParamSlot PARAM_LAYOUT[] = {
```
→ one line, `src/coupling/visual_canvas.hpp`, symbol `t7::PARAM_LAYOUT`; there is
no second `ParamSlot[]` anywhere in the tree.

**Recipe correction, recorded.** An earlier form of this recipe added `-- docs/`
to the pathspec. At the current HEAD that widens the census to 126 hits, because
this report's own committed twin, `docs/LIGATURE_0_RECON.md`, quotes the same
identifiers back:

```
git grep -n "PARAM_LAYOUT\|ParamSlot\|ParamLayoutView\|param_layout" -- src/ tools/ docs/ | wc -l   # 126
git grep -n "PARAM_LAYOUT\|ParamSlot\|ParamLayoutView\|param_layout" -- src/ tools/      | wc -l   #  37
git grep -n "PARAM_LAYOUT\|ParamSlot\|ParamLayoutView\|param_layout" -- docs/            | wc -l   #  89
```
The 89 `docs/` hits are self-reference introduced by commit `6d53388e`; none of
them is a declaration. The claim "exactly one array definition site" holds on the
`src/ tools/` scope, which is the scope every count in this section uses.

### 3.2 Pipe count (R2)

```
sed -n '/inline constexpr ParamSlot PARAM_LAYOUT\[\] = {/,/^    };/p' \
  src/coupling/visual_canvas.hpp | grep -c '^        { "'
```
→ **8**

The same command without `-c` prints the eight rows and nothing else. The table
below therefore has exactly eight rows.

Bank occupancy from the same rows: bases 0..14 of `VISUAL_PARAM_SLOTS = 256`
(`src/coupling/visual_params.hpp`, `t7::VISUAL_PARAM_SLOTS`) are claimed; slots
15..255 are unclaimed by any pipe.

### 3.3 The ParamLayout declaration block, verbatim (R5)

From `src/coupling/visual_canvas.hpp`, symbol `t7::PARAM_LAYOUT` and its witness:

```cpp
    // ═══ MASTER CONTROL PANEL ════════════════════════════════════════════════════
    // The one place every exposed pipe is declared — name, slot, width, and the
    // value it rests at. Slots are assigned here, by hand, in this single table, so
    // there are no collisions across entities. Read it as a register map; every
    // coupling and every entity flush resolves against it by name. (A vector's rest
    // is one value across its channels. Both fog pipes rest at 0 since ATMOS_1:
    // the canvas emits DEVIATIONS from its anchor row, and the mood's own rest is
    // composed in at the U4 seam — the same shape the ribbon pipes below wear.)
    //
    //                          name           base count   rest
    inline constexpr ParamSlot PARAM_LAYOUT[] = {
        { "fog.density",          0,    1,    0.0f },   // deviation from the anchor (ATMOS_1)
        { "fog.color",            1,    3,    0.0f },   // per channel, same law
        // ── ribbon (pitch compass) ── deviations composed over the seed
        // draws at the entity flush; rest = identity (1 = the seed's dance).
        { "ribbon.amp_lateral_mult",  4, 1, 1.0f },
        { "ribbon.amp_vertical_mult", 5, 1, 1.0f },
        { "ribbon.color_stim", 6, 3, 0.0f },
        { "ribbon.color_mix",  9, 1, 0.0f },
        // ── terrain (CHECKER-REBUILD, the pc-color field) ── checker_mean
        // now carries the resultant COLOR (rgb); checker_var widens to TWO —
        // [0] = music_amount (presence), [1] = music_variance (distinct-pc
        // count). All rest at 0 (amount 0 → the GPU shows each cell's seed
        // color; the_board's authored rests: terrain_looks ROW 2 REST_CHECKER_*).
        { "terrain.checker_mean", 10, 3, 0.0f },
        { "terrain.checker_var",  13, 2, 0.0f },
    };
    inline constexpr uint32_t PARAM_LAYOUT_COUNT =
        sizeof(PARAM_LAYOUT) / sizeof(PARAM_LAYOUT[0]);

    // WITNESS — the register map's teeth: every pipe within the bank,
    // no two pipes overlapping. Hand-laying stays; a collision is now a
    // build error, not a silent cross-write.
    static_assert([] {
        for (uint32_t i = 0; i < PARAM_LAYOUT_COUNT; ++i) {
            const ParamSlot& a = PARAM_LAYOUT[i];
            if (a.count < 1) return false;
            if (a.base < 0 || a.base + a.count > VISUAL_PARAM_SLOTS) return false;
            for (uint32_t j = i + 1; j < PARAM_LAYOUT_COUNT; ++j) {
                const ParamSlot& b = PARAM_LAYOUT[j];
                if (a.base < b.base + b.count && b.base < a.base + a.count) return false;
            }
        }
        return true;
        }(), "PARAM_LAYOUT: a pipe leaves the bank or two pipes overlap");
```

### 3.4 THE PIPE TABLE

Column definitions used below:
* **rest** — the `ParamSlot.rest` column of `PARAM_LAYOUT`, verbatim.
* **coupling declared?** — a decode block in `t7::VisualCanvas::tick` writes this
  pipe via `params_.set(<binding>.base…)`.
* **source stat** — the `SignalLayout::resolve(...)` name whose `SourceBinding`
  gates that decode block, and whether it binds at runtime.
* **idle** — whether the pipe returns to a value when its source falls quiet, per
  the coupling's own code and doctrine comment.
* **resolved in cartridge?** — `visual_canvas_.layout().resolve("<pipe>")` in
  `t7::the_board::Cartridge::bind_signal_layout`.
* **flushed?** — a per-frame spine phase reads the bank slot and pushes it at the
  GPU seam.
* **setter symbol** — the member function or member write that crosses CPU→GPU.
* **field in realization/state.hpp** / **field in world.wgsl** — the terminal fields.

**READ THE VERDICT COLUMN WITH ITS FLAG, ALWAYS.** Every row below reads
`DECLARED-UNRESOLVED (source half)`. The unit's definition of that token is "pipe
declared, never resolved at bind", and by that literal definition the assignment
is inverted: all eight pipes **are** resolved at bind, twice, and both resolves
succeed. What fails is the **source** resolve — a different resolver
(`t7::SignalLayout::resolve`) against a different view
(`t7::BeatClock::stat_layout()` = `StatLayoutView{ nullptr, 0 }`). The full FLAG
below the table gives the mapping and the token this state would otherwise want
(`SOURCE-UNBOUND`). This paragraph is repeated at the head of the table so the
verdict column is never quoted downstream without it; §3.10 and §3.11 carry the
same caveat beside the same numbers.

| pipe | rest | coupling declared? | source stat | idle | resolved in cartridge? | flushed? | setter symbol | field in realization/state.hpp | field in world.wgsl | verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `fog.density` (base 0, w1) | `0.0f` (deviation from anchor `FOG_DENSITY_NONE = 0.0030f`) | YES — fog arm of `VisualCanvas::tick`, table decode over `FOG_BY_FIELD`, carried on `fog_seg_` with span `CANVAS_LIVE.fog_span` | `all.field` → `fog_field_` — **UNBOUND at runtime** (`SignalLayout::resolve` against `BeatClock::stat_layout()` = `{nullptr,0}`) | **none** — held source; doctrine in `visual_canvas.hpp`: "once a scale is established it persists through silence, so fog never returns to a rest" | YES — `fog_density_dst_` in `Cartridge::bind_signal_layout` | YES — `Cartridge::phase_motion_drivers`, `UPhase::MotionDrivers` row of `UPDATE_SPINE` | `GPUState::set_fog` (`realization/state.hpp`) | `GPUDesignConfig::fog_density` | `DesignConfig.fog_density` (read in the fog fragment path, `1.0 - exp(-dist * config.fog_density)`) | **DECLARED-UNRESOLVED** (source half) |
| `fog.color` (base 1, w3) | `0.0f` per channel (deviation from anchor `FOG_COLOR_NONE = {0.85,0.78,0.72}`) | YES — same fog arm, 3-channel loop over `FOG_COLOR_BY_FIELD`, `fog_color_seg_[3]`, same span | `all.field` → `fog_field_` — **UNBOUND** | **none** — same held source | YES — `fog_color_dst_` | YES — same phase, same call | `GPUState::set_fog` | `GPUDesignConfig::fog_color[3]` | `DesignConfig.fog_color` (`mix(lit, config.fog_color, fog)` and the veil `mix`) | **DECLARED-UNRESOLVED** (source half) |
| `ribbon.amp_lateral_mult` (base 4, w1) | `1.0f` (identity — the seed's dance) | YES — sustain-swell arm of `VisualCanvas::tick`; `hold_mask_`/`hold_beats_` clock, goal `1 + (CANVAS_LIVE.swell_ceiling-1)·t`, `amp_lat_seg_` | `ch1.present_count` (`RIBBON_VOICE` + `".present_count"`) → `voice_playhead_` — **UNBOUND** | **1.0** — "Silence ⇒ 1 ⇒ the seed dance"; release span `CANVAS_LIVE.swell_release` | YES — `ribbon_amp_lat_dst_` | YES — `ribbon_frame_tick` (`bodies/ribbon.hpp`) via `RibbonDeps::ribbon_amp_lat_dst_`, driven by `Cartridge::phase_ribbon_tick`, `RPhase::RibbonTick` row of `RENDER_SPINE` | no `set_*`; direct member write `rs.gpu[i].lateral_amp = rs.active[i].spawn_lateral_amp * ml`, shipped by `GPUState::upload_ribbon` | `GPURibbonState::lateral_amp` (offset 44) | `RibbonState.lateral_amp` (`sin(ribbon.lateral_freq * phase_age) * ribbon.lateral_amp`) | **DECLARED-UNRESOLVED** (source half) |
| `ribbon.amp_vertical_mult` (base 5, w1) | `1.0f` | YES — same swell arm, same goal, `amp_vert_seg_` | `ch1.present_count` → `voice_playhead_` — **UNBOUND** | **1.0** — same law, same release span | YES — `ribbon_amp_vert_dst_` | YES — same `ribbon_frame_tick` block | direct member write `rs.gpu[i].vertical_amp = rs.active[i].spawn_vertical_amp * mv`, shipped by `GPUState::upload_ribbon` | `GPURibbonState::vertical_amp` (offset 52) | `RibbonState.vertical_amp` (`sin(ribbon.vertical_freq * phase_age) * ribbon.vertical_amp`) | **DECLARED-UNRESOLVED** (source half) |
| `ribbon.color_stim` (base 6, w3) | `0.0f` per channel | YES — room-tint arm of `VisualCanvas::tick`; `PITCH_VECS` hue seating from `CANVAS_LIVE.pitch_vec_origin`, Rodrigues basis `TINT_D1`/`TINT_D2`, `CANVAS_LIVE.tint_luma`/`tint_chroma`, `tint_stim_seg_[3]` | `all.window_length` → `room_wagon_` — **UNBOUND** (the arm's outer guard) | **none** — "window drained: stim segments hold their last hue; the MIX below is what releases — fade, not gray-out" | YES — `ribbon_tint_stim_dst_` | YES — same `ribbon_frame_tick` seam block | no `set_*`; `rs.gpu[i].color[c2] = par.spawn_color[c2] + (s - par.spawn_color[c2]) * mix`, shipped by `GPUState::upload_ribbon` | `GPURibbonState::color[3]` (offset 32) — shared with `ribbon.color_mix` | `RibbonState.color` (`mix(ribbon.color, ribbon.color_b, cell_parity)`; `select(ribbon.color, checker, …)`) | **DECLARED-UNRESOLVED** (source half) |
| `ribbon.color_mix` (base 9, w1) | `0.0f` (mix 0 = the seed-drawn color exactly) | YES — same room-tint arm; goal is `CANVAS_LIVE.tint_mix_max` when the room sounds else 0, `tint_mix_seg_` | `all.present_count` → `room_playhead_` (summed over 12 pcs), **plus** the arm's outer guard on `all.window_length` → `room_wagon_` — **both UNBOUND** | **0.0** — "mix rises while the line sounds, releases to 0 in silence — rest = the seed-drawn ribbon exactly"; release span `CANVAS_LIVE.tint_mix_release` | YES — `ribbon_tint_mix_dst_` | YES — same `ribbon_frame_tick` seam block | no `set_*` and **no GPU field of its own** — it is the lerp factor into `rs.gpu[i].color[3]` | none of its own; consumed into `GPURibbonState::color[3]` | none of its own; consumed into `RibbonState.color` | **DECLARED-UNRESOLVED** (source half) |
| `terrain.checker_mean` (base 10, w3) | `0.0f` per channel | YES — CHECKER-REBUILD arm of `VisualCanvas::tick`; beat-grid sample-and-hold at `CANVAS_LIVE.checker_read_span`, length-weighted average over `PC_COLOR[12][3]`, `checker_res_seg_[3]` on `CANVAS_LIVE.checker_attack` | `ch1.window_length` (`CHECKER_VOICE` + `".window_length"`) → `checker_win_` — **UNBOUND** | **none** — "On silence we hold the last resultant (amount fades it to seed anyway)" | YES — `checker_mean_dst_` | YES — `Cartridge::phase_motion_drivers`, checker arm, `UPhase::MotionDrivers` | `GPUState::set_checker_color_field` (`realization/state.hpp`) | `GPUDesignConfig::checker_resultant[3]` | `DesignConfig.checker_resultant` (read by `discrete_cell_color_at_tier` via `animated_cell_color`; also the `DEBUG_VIEW == 1u` wheel meter) | **DECLARED-UNRESOLVED** (source half) |
| `terrain.checker_var` (base 13, w2) | `0.0f` both lanes | YES — same CHECKER-REBUILD arm; lane 0 = presence (`checker_amount_seg_`), lane 1 = distinct-pc surplus `max(0, n-1)` (`checker_var_seg_`), attack `CANVAS_LIVE.checker_attack` / release `CANVAS_LIVE.checker_release` | `ch1.window_length` → `checker_win_` — **UNBOUND** | **0.0** both lanes — "release LINEAR over 8 beats to rest — which the GPU maps to each cell's seed color" | YES — `checker_var_dst_` | YES — same phase, same `set_checker_color_field` call | `GPUState::set_checker_color_field` | lane 0 → `GPUDesignConfig::checker_music_amount`; lane 1 → `GPUDesignConfig::checker_music_variance` | `DesignConfig.checker_music_amount` (gates `has_mode_bias`) and `DesignConfig.checker_music_variance` (S3 within-patch spread) | **DECLARED-UNRESOLVED** (source half) |

**FLAG — verdict-vocabulary mismatch, resolved by explicit mapping.** The six
supplied verdict tokens carry no term for the state this tree is actually in:
*every target-side hop present and flushing, source socket empty*. I mapped it to
`DECLARED-UNRESOLVED` because that is the only token naming a bind-time resolve
failure, and a bind-time resolve failure is precisely what happens — but on the
**source** half of `VisualCanvas::bind`, never on the target half. Reading the row
as "the pipe is missing from `PARAM_LAYOUT`" would be wrong. The three columns
`resolved in cartridge?`, `flushed?` and `field in world.wgsl` are `YES` on all
eight rows for exactly this reason. Cost to have avoided the ambiguity: a seventh
verdict token (e.g. `SOURCE-UNBOUND`). No `LIVE`, `DECLARED-UNFLUSHED`,
`ORPHAN-SINK`, `ORPHAN-SOURCE` or `GONE` row exists in this table.

The three facts the mapping rests on, each with the command that produced it:

```
grep -n "stat_layout" src/analysis/beat_clock.hpp
#   44:    StatLayoutView stat_layout() const { return StatLayoutView{ nullptr, 0 }; }

grep -n -A2 "SourceBinding resolve" src/musical/signal_layout.hpp
#   56:    SourceBinding resolve(std::string_view name) const {
#   57-        for (uint32_t i = 0; i < view_.count; ++i) {
#   58-            const StatGroup& g = view_.groups[i];

grep -n "ZOETROPE_EARS" src/coupling/visual_canvas.hpp | head -1
#   143:    inline constexpr uint32_t ZOETROPE_EARS = 0b0111'1111u;
```
`view_.count == 0` makes the loop body unreachable, so `++misses_;` /
`return SourceBinding{};` is the only path through `t7::SignalLayout::resolve` for
every source name in this program. The two target-side resolvers are a different
function entirely (`t7::ParamLayout::resolve`, §3.6) over a different view
(`ParamLayoutView{ PARAM_LAYOUT, PARAM_LAYOUT_COUNT }`, count 8), and they hit.

### 3.5 Verdict evidence

Only one verdict class occurs, so this subsection proves that class on three
exemplar pipes and then proves the *absence* of each other class.

#### DECLARED-UNRESOLVED — exemplar 1: `fog.density`

The target resolves twice and succeeds twice; the source resolves once and fails.

```
git grep -n -- '"fog.density"' -- src/
```
→ the command's output, **pasted verbatim and unfiltered** (R5; an earlier
form of this block condensed it, stripped the `path:lineno:` prefixes, replaced
one hit with a placeholder and collapsed four more — corrected here):
```
src/cartridges/the_board/cartridge.hpp:250:            TargetBinding fog_density_dst_{};   // resolved "fog.density" pipe
src/cartridges/the_board/cartridge.hpp:828:                fog_density_dst_ = visual_canvas_.layout().resolve("fog.density");
src/console/organ_params.inc:157:ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Regime 1", "fog density",                   MOOD, MoodProfile, atmos.regime[0].fog_density)
src/console/organ_params.inc:170:ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Regime 2", "fog density",                   MOOD, MoodProfile, atmos.regime[1].fog_density)
src/console/organ_params.inc:183:ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Regime 3", "fog density",                   MOOD, MoodProfile, atmos.regime[2].fog_density)
src/console/organ_params.inc:196:ORGAN_PARAM_DEFONLY(F32,  0.0f, 0.05f, 0.0002f, "Atmosphere · Regime 4", "fog density",                   MOOD, MoodProfile, atmos.regime[3].fog_density)
src/coupling/visual_canvas.hpp:58://   auto d = visual_canvas_.layout().resolve("fog.density");   // entity, once
src/coupling/visual_canvas.hpp:233:        { "fog.density",          0,    1,    0.0f },   // deviation from the anchor (ATMOS_1)
src/coupling/visual_canvas.hpp:285:            fog_density_ = param_layout_.resolve("fog.density");
```

Nine matching lines. Independent recount, so the number does not rest on the
paste:
```
git grep -c -- '"fog.density"' -- src/
#   src/cartridges/the_board/cartridge.hpp:2
#   src/console/organ_params.inc:4
#   src/coupling/visual_canvas.hpp:3
```
2 + 4 + 3 = 9. Reading them by enclosing symbol:

* `t7::the_board::Cartridge` — the `TargetBinding fog_density_dst_` member's own
  comment, and the resolve inside `Cartridge::bind_signal_layout`.
* `src/console/organ_params.inc` — four `ORGAN_PARAM_DEFONLY` rows that match only
  because `.` is a regex wildcard: their label is `"fog density"` with a **space**,
  and they enroll `MOOD MoodProfile::atmos.regime[N].fog_density`, the mood-side
  rest that `Cartridge::phase_motion_drivers` composes the pipe's deviation onto.
  They are not pipe sites.
* `src/coupling/visual_canvas.hpp` — the file header's usage comment, the
  `t7::PARAM_LAYOUT` declaration row, and the resolve inside `VisualCanvas::bind`.

**FLAG — the re-census disagrees with the review that prompted it, on the count
only.** The review that flagged this block as condensed described the command's
output as EIGHT lines. Re-run at HEAD it is **nine**:
`git grep -n -- '"fog.density"' -- src/ | wc -l` → 9, corroborated by the
per-file `git grep -c` tallies 2 + 4 + 3 above. The paste and the reading below it
carry nine. Cost to have resolved the discrepancy from the review's side: none —
the two commands agree with each other and disagree with the reviewer's tally, so
the tree is taken as the authority. The review's substantive point (the block was
a condensation presented as output) was correct and is fixed.

Both `resolve` calls are `t7::ParamLayout::resolve` against a view bound one line
earlier from the same `PARAM_LAYOUT`, so both return `valid = true` by
construction (`ParamLayout::bind` then `ParamLayout::reset` in
`VisualCanvas::bind`; `visual_canvas_.layout()` in `Cartridge::bind_signal_layout`).

The source is a different resolver. In `VisualCanvas::bind`:
`fog_field_ = signal_layout_.resolve("all.field");` — that is
`t7::SignalLayout::resolve` (`src/musical/signal_layout.hpp`), whose loop is
`for (uint32_t i = 0; i < view_.count; ++i)`. The view it iterates comes from the
one call site:

```
git grep -n "bind_signal_layout" -- src/
```
→ `src/the_board.cpp: app->render.bind_signal_layout(app->clock.stat_layout());`
and `src/cartridges/the_board/cartridge.hpp: void bind_signal_layout(StatLayoutView v)`.

`app->clock` is declared `t7::BeatClock clock;` in `src/the_board.cpp`, and
`src/analysis/beat_clock.hpp`, symbol `t7::BeatClock::stat_layout`, is:

```cpp
    StatLayoutView stat_layout() const { return StatLayoutView{ nullptr, 0 }; }
```

`view_.count == 0` ⇒ the loop body never executes ⇒ `++misses_` ⇒
`return SourceBinding{};` with `valid = false`. `fog_field_.valid` is therefore
false for the life of the process, and the fog arm of `tick()` — whose first line
is `if (fog_field_.valid) {` — never runs. `fog.density` holds `PARAM_LAYOUT`'s
rest `0.0f` forever.

#### DECLARED-UNRESOLVED — exemplar 2: `terrain.checker_var`

```
git grep -n -- '"terrain.checker_var"' -- src/
```
→ declaration in `PARAM_LAYOUT`; `checker_var_ = param_layout_.resolve(...)` in
`VisualCanvas::bind`; `checker_var_dst_ = visual_canvas_.layout().resolve(...)` in
`Cartridge::bind_signal_layout`; the member comment on `TargetBinding checker_var_`.
No fourth site. The CHECKER arm's guard is
`if (checker_win_.valid && checker_mean_.valid && checker_var_.valid)`; the two
target halves are true, `checker_win_` is `signal_layout_.resolve("ch1.window_length")`
against the same empty view, so the conjunction is false. The downstream half is
nevertheless complete and unconditional every frame:
`Cartridge::phase_motion_drivers` calls `set_checker_color_field` in **both** arms
of its `if (checker_mean_dst_.valid && checker_var_dst_.valid)` — the `else` arm
ships `DRIVER_LIVE.checker.rest_*`. So the GPU field is written every frame; it is
written with a rest, not with music.

#### DECLARED-UNRESOLVED — exemplar 3: `ribbon.color_mix`

Its own source is `all.present_count` (`room_playhead_`), but the write is nested
inside the room-tint arm's outer guard
`if (room_wagon_.valid && tint_stim_.valid && tint_mix_.valid)`, so
`all.window_length` gates it as well. Both resolve against the empty view. Recipe:

```
git grep -n -- '"ribbon.color_mix"' -- src/
```
→ three sites only (declaration, `VisualCanvas::bind` resolve, cartridge resolve).
The consumer exists and runs — `ribbon_frame_tick` in `bodies/ribbon.hpp` reads
`c->ribbon_tint_mix_dst_` every frame — so this is not `ORPHAN-SOURCE`.

#### The socket, counted

`src/analysis/beat_clock.hpp` states the fact in its own header comment. Quoted
**to the end of the paragraph**, verbatim, with the leading `// ` of each comment
line stripped and nothing else changed (an earlier form of this blockquote stopped
at "…via the graceful path." and dropped the trailing parenthetical and the two
sentences after it — corrected here; recipe:
`grep -n -B14 -A4 "graceful path" src/analysis/beat_clock.hpp`):

> CUT_1c: the analysis intake's successor (ruling R7). MIDI/DAW
> analysis left the build; the render side keeps its two contracts —
> an AnalysisSignal each frame and a StatLayoutView once at bind.
> The BeatClock serves both from nothing but dt: advancing clocks at
> a variable BPM (default 100; this struct is the value's ONE home,
> panel-eligible), and an EMPTY layout.
>
> The empty layout is the audio socket. The render side resolves 12
> live source names against it — all.field, ch1.present_count,
> all.window_length, all.present_count, ch1.window_length,
> ch0.onset .. ch6.onset — and every resolve misses and disables its
> coupling via the graceful path (musical/signal_layout.hpp
> resolve(): one stderr warn, valid=false). A future browser-side
> audio source plugs into this socket by publishing exactly those
> names through a real StatLayoutView.

The dropped tail is load-bearing twice over: it names the resolver whose miss
path §3.4's FLAG turns on (`musical/signal_layout.hpp` `resolve()`, "one stderr
warn, valid=false"), and it records that the socket's stated future filler is a
**browser-side** audio source publishing those same twelve names.

Independently counted from `VisualCanvas::bind`: 5 named resolves
(`all.field`, `ch1.present_count`, `all.window_length`, `all.present_count`,
`ch1.window_length`) plus one per set bit of
`inline constexpr uint32_t ZOETROPE_EARS = 0b0111'1111u` (7 bits ⇒ `ch0.onset` …
`ch6.onset`) = **12**. Matches. `VisualCanvas::bind` ends by printing
`"[SignalLayout] %u sources unbound (no audio source)\n"` when
`signal_layout_.misses() > 0`.

`src/analysis/canvas_1/canvas.hpp` (symbol `t7::canvas_1`, with a real
`StatLayoutView stat_layout() const override`) exists in the tree but is included
by **nothing**:

```
git grep -n '#include "analysis/canvas_1/canvas.hpp"' -- src/     # no matches
grep -n include src/analysis/canvas_1/probe_canvas.cpp            # '#include "canvas.hpp"'
```
Only the five `src/analysis/canvas_1/{check_*,probe_*}.cpp` standalone binaries
reach it, via a relative `"canvas.hpp"`. It is not on the program's include graph.

#### Why no `DECLARED-UNFLUSHED` row

Every one of the eight is flushed by a spine phase that is a real row in a spine
table:
```
grep -n "phase_motion_drivers\|phase_ribbon_tick" src/cartridges/the_board/cartridge.hpp
```
→ `{ UPhase::MotionDrivers, "motion_drivers", &Cartridge::phase_motion_drivers, Driver::Music, true, F_CONFIG }`
in `UPDATE_SPINE`, and
`{ RPhase::RibbonTick, "ribbon_tick", &Cartridge::phase_ribbon_tick, Driver::Mixed, ROSTER.ribbon, F_SIGNAL }`
in `RENDER_SPINE` (guarded further by `static_assert((uint32_t)RPhase::RibbonTick < (uint32_t)RPhase::DispatchCompute, "O-1: …")`).

Caveat recorded, not repaired: the four ribbon pipes are computed into
`rs.gpu[i]` for **every** active instance, but `ribbon_frame_tick` ships only
`rs.gpu[rs.rendered_slot]` — "THE ONE WRITE (RIBBON_1). The whole 112-byte state,
three windows, once a frame." So the ribbon pipes' GPU reach is the rendered slot
only. That is a scope fact about the flush, not an absent flush.

#### Why no `GONE` row

Every one of the eight declared names has a surviving `params_.set(...)` writer in
`VisualCanvas::tick` and a surviving reader downstream. The eight-name grep in
§3.5 exemplars, repeated for all eight, returns a declaration site, a
`VisualCanvas::bind` resolve, a `Cartridge::bind_signal_layout` resolve, and (for
the two checker pipes) a member comment — never a declaration alone.

#### Why no `ORPHAN-SOURCE` row

Consumer sites, one command:
```
grep -n "fog_density_dst_\|fog_color_dst_\|ribbon_amp_lat_dst_\|ribbon_amp_vert_dst_\|ribbon_tint_stim_dst_\|ribbon_tint_mix_dst_\|checker_mean_dst_\|checker_var_dst_" \
  src/cartridges/the_board/cartridge.hpp
grep -n "amp_lat\|amp_vert\|tint_stim\|tint_mix" src/cartridges/the_board/bodies/ribbon.hpp
```
Both return live per-frame reads for all eight bindings. No pipe is consumerless.

#### `ORPHAN-SINK` — candidates found, none inside the pipe table

The table is one row per pipe, so an orphan sink cannot appear as a row. Two
GPU-side facts recorded for completeness:

1. `src/cartridges/the_board/realization/world.wgsl`, symbol
   `gol_composite_cell_color`, passes literal zeros where the checker fan would
   go, with the reason in-place: *"When the zones' own coupling pass convenes
   (Jean's, STATUS: INTENT), it revives by passing config.checker_resultant /
   _music_amount / _music_variance in place of those zeros."* The fields are not
   orphaned (`animated_cell_color` does pass them), but this second call site
   is deliberately unwired.
2. `GPURibbonState::color_b` and `GPURibbonState::checker_scatter` are read by the
   shader (`mix(ribbon.color, ribbon.color_b, cell_parity)`) and named by no pipe.
   Recipe: `grep -n "color_b\|checker_scatter" src/cartridges/the_board/realization/world.wgsl`
   against the eight-name grep, which never mentions them.

### 3.6 The resolve / flush machinery, verbatim (R5)

#### `t7::ParamLayout` — `bind`, `reset`, `resolve` (`src/coupling/visual_params.hpp`)

```cpp
    class ParamLayout {
    public:
        void bind(ParamLayoutView v) { view_ = v; }

        // Lay the bank to each pipe's rest value. Called once at boot, and on any
        // reload of the control panel. Vector pipes fill uniformly.
        void reset(VisualParams& p) const {
            for (uint32_t i = 0; i < view_.count; ++i) {
                const ParamSlot& s = view_.slots[i];
                for (int k = 0; k < s.count; ++k) p.v[s.base + k] = s.rest;
            }
        }

        // Look up a pipe by name. Returns {valid=false} and warns on stderr if
        // the name is absent — callers leave the coupling unbound rather than
        // writing a wrong slot.
        TargetBinding resolve(std::string_view name) const {
            for (uint32_t i = 0; i < view_.count; ++i) {
                const ParamSlot& s = view_.slots[i];
                if (name == s.name) {
                    return TargetBinding{ s.base, s.count, true };
                }
            }
            std::fprintf(stderr,
                "[ParamLayout] pipe '%.*s' not in layout (coupling unbound)\n",
                (int)name.size(), name.data());
            return TargetBinding{};
        }

        uint32_t         count() const { return view_.count; }
        const ParamSlot* slots() const { return view_.slots; }

    private:
        ParamLayoutView view_{ nullptr, 0 };
    };
```

#### `t7::VisualCanvas::bind` — the whole resolve pass (`src/coupling/visual_canvas.hpp`)

```cpp
        // Startup wiring: publish the control panel, lay the bank to its rests,
        // adopt the analysis layout, and resolve every coupling's source and target
        // once. tick() then never resolves.
        void bind(StatLayoutView analysis_layout) {
            param_layout_.bind(ParamLayoutView{ PARAM_LAYOUT, PARAM_LAYOUT_COUNT });
            param_layout_.reset(params_);

            signal_layout_.bind(analysis_layout);

            // fog: the held field → a density and a tint, each a DEVIATION from
            // the anchor row (ATMOS_1); index 0 is the anchor, so the Segments
            // start at 0 — no deviation yet.
            fog_field_ = signal_layout_.resolve("all.field");
            fog_density_ = param_layout_.resolve("fog.density");
            fog_color_ = param_layout_.resolve("fog.color");
            fog_seg_ = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            for (int c = 0; c < 3; ++c)
                fog_color_seg_[c] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };

            // ribbon sources (the casting sheet): the voice's Playhead drives
            // the sustain swell; the room's Wagon aims the tint's hue; the
            // room's Playhead gates the tint's mix.
            {
                std::string v(RIBBON_VOICE);
                voice_playhead_ = signal_layout_.resolve((v + ".present_count").c_str());
            }
            room_wagon_ = signal_layout_.resolve("all.window_length");
            room_playhead_ = signal_layout_.resolve("all.present_count");
            amp_lat_ = param_layout_.resolve("ribbon.amp_lateral_mult");
            amp_vert_ = param_layout_.resolve("ribbon.amp_vertical_mult");
            amp_lat_seg_ = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };
            amp_vert_seg_ = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };
            tint_stim_ = param_layout_.resolve("ribbon.color_stim");
            tint_mix_ = param_layout_.resolve("ribbon.color_mix");
            for (int c2 = 0; c2 < 3; ++c2)
                tint_stim_seg_[c2] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            tint_mix_seg_ = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };

            // CHECKER-REBUILD source + targets (the terrain's checker voice):
            // the voice's WINDOW pc-length vector becomes the resultant color;
            // presence + distinct-pc count envelope the pull and the spread.
            {
                std::string v(CHECKER_VOICE);
                checker_win_ = signal_layout_.resolve((v + ".window_length").c_str());
            }
            checker_mean_ = param_layout_.resolve("terrain.checker_mean");   // 3: resultant rgb
            checker_var_  = param_layout_.resolve("terrain.checker_var");    // 2: amount, variance
            for (int c2 = 0; c2 < 3; ++c2) {
                checker_res_goal_[c2] = 0.0f;                     // resultant color, held between reads
                checker_res_seg_[c2] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            }
            checker_amount_goal_ = 0.0f;                          // presence (rest 0 → seed)
            checker_amount_seg_  = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            checker_var_goal_ = 0.0f;                             // distinct-pc spread (rest 0)
            checker_var_seg_  = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            checker_next_read_ = 0.0f;   // first frame reads, then grid-locks

            // zoetrope ears (the listener set): one "chN.onset" resolve per
            // set bit of ZOETROPE_EARS. A miss warns and disables that ear —
            // the resolver's own semantics; the deaf ear simply never sums.
            zoetrope_ear_count_ = 0;
            for (int ch = 0; ch < 8; ++ch) {
                if (!(ZOETROPE_EARS & (1u << ch))) continue;
                std::string v("ch" + std::to_string(ch));
                zoetrope_ears_[zoetrope_ear_count_++] =
                    signal_layout_.resolve((v + ".onset").c_str());
            }
            for (int r = 0; r < 7; ++r) zoetrope_rows_[r] = 0.0f;
            // Boot witness — doctrine, not measurement (P6): one line,
            // always, so a deaf zoetrope names its fault at the seam.
            {
                int bound = 0;
                for (int e = 0; e < zoetrope_ear_count_; ++e)
                    if (zoetrope_ears_[e].valid) ++bound;
                std::fprintf(stderr, "[Zoetrope] ears bound: %d of %d (mask 0x%02X)\n",
                    bound, zoetrope_ear_count_, ZOETROPE_EARS);
            }

            // PORT_4c — THE SOCKET, in one line. Every signal-side
            // resolve above happens here, and with the BeatClock's empty
            // layout (CUT_1c) every one of them misses. The release twin
            // prints this summary; the debug twin has already printed
            // each source by name. Placed last, after the resolves it
            // counts, beside the Zoetrope witness it deliberately does
            // not replace — that line reports a different fact.
            if (signal_layout_.misses() > 0) {
                std::fprintf(stderr,
                    "[SignalLayout] %u sources unbound (no audio source)\n",
                    signal_layout_.misses());
            }
        }
```

#### `t7::VisualCanvas::layout` / `params` — the publish accessors

```cpp
        // Consumers read the bank (and resolve their pipe once through layout()).
        const VisualParams& params() const { return params_; }
        const ParamLayout& layout() const { return param_layout_; }

        // The zoetrope's row impulses — seven floats, bottom row = tonic,
        // overwritten each tick. The lattice (the_board) strikes from these.
        const float* zoetrope_rows() const { return zoetrope_rows_; }
```

There is **no per-frame publish/flush member on `VisualCanvas`**. `tick()` writes
the bank; the flush is the cartridge's and the ribbon's, quoted next.

#### `t7::the_board::Cartridge::bind_signal_layout` — the second resolve pass (`cartridge.hpp`)

```cpp
            void bind_signal_layout(StatLayoutView v) {
                visual_canvas_.bind(v);
                fog_density_dst_ = visual_canvas_.layout().resolve("fog.density");
                fog_color_dst_ = visual_canvas_.layout().resolve("fog.color");
                ribbon_amp_lat_dst_ = visual_canvas_.layout().resolve("ribbon.amp_lateral_mult");
                ribbon_amp_vert_dst_ = visual_canvas_.layout().resolve("ribbon.amp_vertical_mult");
                ribbon_tint_stim_dst_ = visual_canvas_.layout().resolve("ribbon.color_stim");
                ribbon_tint_mix_dst_ = visual_canvas_.layout().resolve("ribbon.color_mix");
                checker_mean_dst_ = visual_canvas_.layout().resolve("terrain.checker_mean");
                checker_var_dst_ = visual_canvas_.layout().resolve("terrain.checker_var");
                std::fprintf(stderr,
                    "[the_board] fog.density base=%d valid=%d | fog.color base=%d count=%d valid=%d\n",
                    fog_density_dst_.base, (int)fog_density_dst_.valid,
                    fog_color_dst_.base, fog_color_dst_.count, (int)fog_color_dst_.valid);
                std::fprintf(stderr,
                    "[the_board] terrain.checker_mean base=%d count=%d valid=%d | terrain.checker_var base=%d valid=%d\n",
                    checker_mean_dst_.base, checker_mean_dst_.count, (int)checker_mean_dst_.valid,
                    checker_var_dst_.base, (int)checker_var_dst_.valid);
            }
```

#### `t7::the_board::Cartridge::phase_motion_drivers` — the per-frame flush for fog and checker (`cartridge.hpp`)

The whole function, **not elided** (an earlier form of this quote stopped at the checker `else` arm's closing brace and dropped the remaining 55 lines and the function's own closing brace without a marker — corrected here). Boundaries: `grep -n "void phase_motion_drivers" src/cartridges/the_board/cartridge.hpp` opens it and the next member, `void phase_motion_bodies(UpdateCtx& c)`, closes it; the fog+checker prefix is the first ~78 lines and the zoetrope + FIELD_4 beacon tail is the remaining 55. Only the first two blocks are pipe flushes; the tail is quoted because it shares the phase and (in the zoetrope's case) the socket — see §3.7.

```cpp
            void phase_motion_drivers(UpdateCtx& c) {
                auto& signal = c.signal;
                visual_canvas_.tick(signal);
                // ORGAN — the drivers' room sits at this seam:
                // out = rest + gain·deviation. The REST is the mood's,
                // drawn per world into mood_state_.fog_rest_* by
                // apply_mood_lighting; the DEVIATION is the canvas's,
                // measured from its anchor row. Gain 1 is the coupling
                // verbatim, gain 0 is the mood's own fog, and with no
                // bindings the rest alone speaks — so the dial works
                // headless too.
                //
                // set_fog GUARDS — it compares all four lanes and dirties
                // only on a change — so both arms call it unconditionally
                // and the silent case costs no dirty.
                {
                    const auto& drv = DRIVER_LIVE.fog;
                    const auto& ms  = mood_state_;
                    if (fog_density_dst_.valid && fog_color_dst_.valid) {
                        const VisualParams& fp = visual_canvas_.params();
                        gpuState_.set_fog(
                            std::max(0.0f, ms.fog_rest_density + drv.gain * fp.get(fog_density_dst_.base)),
                            std::clamp(ms.fog_rest_color[0] + drv.gain * fp.get(fog_color_dst_.base + 0), 0.0f, 1.0f),
                            std::clamp(ms.fog_rest_color[1] + drv.gain * fp.get(fog_color_dst_.base + 1), 0.0f, 1.0f),
                            std::clamp(ms.fog_rest_color[2] + drv.gain * fp.get(fog_color_dst_.base + 2), 0.0f, 1.0f));
                    } else {
                        gpuState_.set_fog(ms.fog_rest_density, ms.fog_rest_color[0],
                                          ms.fog_rest_color[1], ms.fog_rest_color[2]);
                    }
                }
                // CHECKER-REBUILD: the pc-color field's flush — one setter,
                // the fan (resultant rgb + music amount + music variance).
                // ORGAN — the drivers' room sits at this seam too:
                // out = rest + gain·(driven − rest), the fog recipe verbatim.
                // Gain 1 is the coupling byte-for-byte; gain 0 is the rest,
                // which terrain_looks calls law — amount 0 returns each cell
                // to its SEED colour, not to gray. With no bindings the rest
                // alone speaks, so the dial works headless.
                //
                // The resultant is a run of three, so it blends per lane
                // against rest_resultant[] rather than against one scalar.
                if (checker_mean_dst_.valid && checker_var_dst_.valid) {
                    const VisualParams& cp = visual_canvas_.params();
                    const auto& ck = DRIVER_LIVE.checker;
                    const float* mean = cp.run(checker_mean_dst_.base);
                    const float blended[3] = {
                        ck.rest_resultant[0] + ck.gain * (mean[0] - ck.rest_resultant[0]),
                        ck.rest_resultant[1] + ck.gain * (mean[1] - ck.rest_resultant[1]),
                        ck.rest_resultant[2] + ck.gain * (mean[2] - ck.rest_resultant[2]),
                    };
                    gpuState_.set_checker_color_field(blended,
                        ck.rest_amount   + ck.gain * (cp.get(checker_var_dst_.base)     - ck.rest_amount),
                        ck.rest_variance + ck.gain * (cp.get(checker_var_dst_.base + 1) - ck.rest_variance));
                    // [FLUSH] one-shot: fires the first time a live resultant
                    // crosses the CPU->GPU seam. If [CHECKER] is singing in the
                    // console and this line never prints, the bindings above are
                    // invalid or the two params_ objects disagree — name it.
                    static bool checker_flush_seen = false;
                    if (!checker_flush_seen
                        && cp.get(checker_var_dst_.base) > 0.05f) {   // music_amount up
                        std::fprintf(stderr,
                            "[FLUSH] checker -> config: resultant=(%.2f %.2f %.2f) amount=%.2f var=%.2f\n",
                            cp.get(checker_mean_dst_.base),
                            cp.get(checker_mean_dst_.base + 1),
                            cp.get(checker_mean_dst_.base + 2),
                            cp.get(checker_var_dst_.base),
                            cp.get(checker_var_dst_.base + 1));
                        checker_flush_seen = true;
                    }
                } else {
                    // No bindings: the rest alone speaks, so the dials still
                    // reach the picture with the music silent — the fog
                    // seam's headless arm again.
                    // set_checker_color_field guards, so this costs no dirty.
                    const auto& ck = DRIVER_LIVE.checker;
                    gpuState_.set_checker_color_field(ck.rest_resultant,
                                                      ck.rest_amount, ck.rest_variance);
                }

                // ZOETROPE (C4/C5): the lattice hears the canvas's row
                // impulses, ticks on the musical clock, and projects cells
                // to cube color through the partial-write door. Same member
                // plumbing as the flushes above; the queue is the phase's.
                zoetrope_strike(cube_behaviors_state_, gpuState_, c.queue,
                    world_state_.active_seed, visual_canvas_.zoetrope_rows(), signal.t_beats);
                zoetrope_service(cube_behaviors_state_, gpuState_, c.queue,
                    world_state_.active_seed, signal.t_beats, signal.dt,
                    point_.x, point_.z);   // the point mirror — the reseat watch (G4)

                // ── THE BEACON (FIELD_4): row 0, rewritten hot each
                // frame — the point moved. point y is DERIVED (the
                // point's house carries no y): host-routed — pawn
                // mirror y (PAWN and RIBBON alike — the possessed body IS
                // where the point is in both, riding the seat in one and
                // walking in the other) / ground under the point in
                // camera-host (the camera has no CPU y mirror; the harvest
                // discards cam pos[1]).
                {
                    GPUFieldAuthored fa{};
                    const float coord = gpuState_.config().floater_coordination;
                    float py;
                    if (point_.host != PointHost::CAMERA) {
                        py = agent_state_.slots[player_.possessed_slot].pos_y;
                    } else {
                        py = estimate_terrain_height(tile_world_state_, point_.x, point_.z);
                    }
                    // ORGAN — the beacon reads its BANK, PANEL_LIVE, and
                    // not the design table: a bank nothing reads is a dial
                    // that writes nothing.
                    const auto& bcn = PANEL_LIVE.beacon;
                    // THE RING SELF-SPACES, AT RUNTIME TOO.
                    // control_panel.hpp's static_assert proves the AUTHORED
                    // pair; this clamp guards the DIALED one, reading
                    // config's LIVE field_k rather than the constexpr
                    // because field_k is a dial too and lowering it breaks
                    // the same ruling from the other end. It sits at the
                    // writer, not at the panel, so every author is
                    // guarded by it.
                    const float ceiling = gpuState_.config().field_k - 1.0f;
                    float s = bcn.s;
                    if (s > ceiling) s = ceiling;
                    if (s < 0.0f)    s = 0.0f;
                    fa.count = 1u;
                    fa.rows[0][0] = point_.x;
                    fa.rows[0][1] = py + bcn.lift;
                    fa.rows[0][2] = point_.z;
                    fa.rows[0][3] = s * coord;
                    fa.rows[1][0] = bcn.r0;
                    fa.rows[1][1] = bcn.r;
                    fa.rows[1][2] = (coord > 0.0f) ? 1.0f : 0.0f;
                    gpuState_.upload_field_authored(c.queue, fa);
                }
            }
```

Read off the newly-disclosed tail, three facts that belong to this section's other
halves:

* The zoetrope block calls `zoetrope_strike` / `zoetrope_service` on
  `visual_canvas_.zoetrope_rows()` — the seventh, non-pipe coupling recorded in
  §3.7. It shares the phase and the empty socket, and it touches no `ParamSlot`.
* The FIELD_4 beacon block reads `PANEL_LIVE.beacon` (`const auto& bcn =
  PANEL_LIVE.beacon;`) for all four of `r0`, `r`, `s`, `lift`. Those are four of
  Group B's 25 dormant rows, and this is their runtime read site — dormant with
  respect to the pipe table, not unread by the program.
* The same block reads `gpuState_.config().field_k` and
  `gpuState_.config().floater_coordination`, two more Group C dormant rows,
  through the live config rather than through `control_panel.hpp`'s constexpr
  twin — the "reads its BANK, not the design table" comment in place.

None of the three touches `visual_canvas_.params()`, so no pipe row above changes.

#### `t7::ribbon_frame_tick` — the per-frame flush for the four ribbon pipes (`bodies/ribbon.hpp`)

```cpp
        {
            const VisualParams& vp = c->visual_canvas_.params();
            // ORGAN_4 P2 — THE SEAM, the 2a recipe verbatim:
            // out = rest + gain·(driven − rest). The rests below ARE the
            // fallbacks this block hardcoded before the room existed, so
            // at the shipped seeds (gain 1, rests 1/1/{0,0,0}/0) the
            // arithmetic is byte-stable: rest + 1·(d − rest) folds back
            // to d for every float, and with the pipe unbound the driven
            // value IS the rest and the blend is the rest exactly.
            const auto& R = DRIVER_LIVE.ribbon;
            const float ml_raw = c->ribbon_amp_lat_dst_.valid
                ? vp.get(c->ribbon_amp_lat_dst_.base)  : R.rest_amp_lat;
            const float ml = R.rest_amp_lat + R.gain * (ml_raw - R.rest_amp_lat);
            const float mv_raw = c->ribbon_amp_vert_dst_.valid
                ? vp.get(c->ribbon_amp_vert_dst_.base) : R.rest_amp_vert;
            const float mv = R.rest_amp_vert + R.gain * (mv_raw - R.rest_amp_vert);
            rs.gpu[i].lateral_amp  = rs.active[i].spawn_lateral_amp  * ml;
            rs.gpu[i].vertical_amp = rs.active[i].spawn_vertical_amp * mv;

            // Line tint (color gen-2): gpu.color = lerp(spawn, stim, mix).
            // Rest = mix 0 = the seed-drawn color exactly; the tick's one
            // upload_ribbon ships it unchanged.
            const float mix_raw = c->ribbon_tint_mix_dst_.valid
                ? vp.get(c->ribbon_tint_mix_dst_.base) : R.rest_tint_mix;
            const float mix = R.rest_tint_mix + R.gain * (mix_raw - R.rest_tint_mix);
            const float* st = c->ribbon_tint_stim_dst_.valid
                ? vp.run(c->ribbon_tint_stim_dst_.base) : nullptr;
            for (int c2 = 0; c2 < 3; ++c2) {
                // THE NULL BRANCH'S REST SHAPE, READ RATHER THAN GUESSED:
                // downstream was `st ? st[c2] : 0.0f`, so the shape the
                // code already assumed is {0,0,0} — PARAM_LAYOUT's rest
                // column for ribbon.color_stim, verbatim.
                const float s_raw = st ? st[c2] : R.rest_tint_stim[c2];
                const float s = R.rest_tint_stim[c2]
                              + R.gain * (s_raw - R.rest_tint_stim[c2]);
                rs.gpu[i].color[c2] =
                    par.spawn_color[c2]
                    + (s - par.spawn_color[c2]) * mix;
            }
        }
```

and, at the tail of the same function, the single upload:

```cpp
        // THE ONE WRITE (RIBBON_1). The whole 112-byte state, three windows,
        // once a frame. It carries the phase clock, the canvas-driven wave
        // amplitudes and the line tint the flush loop above computed, and the
        // brain's two numbers — everything the CPU has to say.
        c->gpuState_.upload_ribbon(queue, g);
```

#### The two setters (`realization/state.hpp`)

```cpp
            void set_fog(float density, float r, float g, float b) {
                if (config_.fog_density != density ||
                    config_.fog_color[0] != r || config_.fog_color[1] != g || config_.fog_color[2] != b) {
                    config_.fog_density = density;
                    config_.fog_color[0] = r; config_.fog_color[1] = g; config_.fog_color[2] = b;
                    configDirty_ = true;
                }
            }
```

```cpp
            // CHECKER-REBUILD: the pc-color field — one call carries the
            // fan (resultant rgb + music amount + music variance travel on
            // one span). Enveloping lives in the coupling decode, never here.
            void set_checker_color_field(const float resultant[3], float amount, float variance) {
                if (config_.checker_resultant[0] != resultant[0]
                    || config_.checker_resultant[1] != resultant[1]
                    || config_.checker_resultant[2] != resultant[2]
                    || config_.checker_music_amount != amount
                    || config_.checker_music_variance != variance) {
                    config_.checker_resultant[0] = resultant[0];
                    config_.checker_resultant[1] = resultant[1];
                    config_.checker_resultant[2] = resultant[2];
                    config_.checker_music_amount = amount;
                    config_.checker_music_variance = variance;
                    configDirty_ = true;
                }
            }
```

### 3.7 One non-pipe side channel, recorded because it shares the socket

`VisualCanvas` carries a seventh coupling — the zoetrope — that is **not** a pipe:
it never touches `PARAM_LAYOUT` or the bank. `tick()` sums the resolved
`chN.onset` ears through `ZOETROPE_ROW_OF_PC` into `float zoetrope_rows_[7]`,
published by `VisualCanvas::zoetrope_rows()` and consumed by `zoetrope_strike` /
`zoetrope_service` inside `Cartridge::phase_motion_drivers`. All seven of its ears
resolve against the same empty layout and miss, so the array is all-zero every
frame. It appears in no row above because the table is one row per `ParamSlot`.

---

### 3.8 THE DORMANT SURFACE

### Recipe for "not named by any pipe"

A parameter is **named by a pipe** iff its declared member appears as an
identifier inside one of the four expressions that constitute a pipe's decode or
flush — that is, inside `t7::VisualCanvas::tick`,
`t7::the_board::Cartridge::phase_motion_drivers` (fog arm or checker arm), or the
`ribbon_frame_tick` seam block quoted in §3.6 — **or** is one of the terminal GPU
fields those flushes write. Everything else in the three named files is dormant.

Concretely the named-by-a-pipe set was built with these three commands:

```
# (a) the terminal fields the eight pipes write
grep -n "set_fog\|set_checker_color_field" src/cartridges/the_board/realization/state.hpp
#     -> GPUDesignConfig::{fog_density, fog_color, checker_resultant,
#                          checker_music_amount, checker_music_variance}
#     plus GPURibbonState::{lateral_amp, vertical_amp, color} written directly

# (b) the DriverSurface terms read inside those same flush expressions
grep -n "DRIVER_LIVE\.\(fog\|checker\|ribbon\)" \
    src/cartridges/the_board/cartridge.hpp src/cartridges/the_board/bodies/ribbon.hpp

# (c) the CanvasSurface terms read inside VisualCanvas::tick
grep -o "CANVAS_LIVE\.[a-z_]*" src/coupling/visual_canvas.hpp | sort -u    # -> 15 distinct
```

Command (c) returns fifteen distinct names, and
`grep -nE "^\s+float [a-z_]+;" src/coupling/canvas_surface.hpp` returns exactly
fifteen members — so **every** `CanvasSurface` member is pipe-named, none dormant.
Both halves recount cleanly:

```
grep -o "CANVAS_LIVE\.[a-z_]*" src/coupling/visual_canvas.hpp | sort -u | wc -l   # 15
grep -nE "^\s+float [a-z_]+;" src/coupling/canvas_surface.hpp            | wc -l   # 15
```

Sole-reader check, with its output stated exactly (**correction:** an earlier form
of this paragraph said the command returns "the `organ_params.inc` enrollment
lines", plural and mis-described; it returns **one** line, and that line is a prose
comment, not an enrollment row):

```
git grep -o "CANVAS_LIVE\.[a-z_]*" -- src/ | grep -v visual_canvas.hpp
#   src/console/organ_params.inc:CANVAS_LIVE.
#   (1 line)

sed -n '246p' src/console/organ_params.inc
#   // CANVAS_LIVE. The pc-colour field's READ cadence and envelope — how often
```
The fifteen CANVAS enrollments themselves never spell `CANVAS_LIVE.` at all —
their macro form is `ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, <field>, …)`
and the live instance is reached through `organ_block_home`'s
`case ORGAN_BLOCK_CANVAS: return &canvas::CANVAS_LIVE;`
(`src/console/organ_registry.hpp`). The conclusion is unchanged and now rests on
the right evidence: `src/coupling/visual_canvas.hpp` is the sole reader of
`CANVAS_LIVE`'s fields.

### Group A — `src/cartridges/the_board/contracts/driver_surface.hpp`

Full member census recipe — **corrected**. The recipe first published here,
`grep -nE "^\s+(float|uint32_t) [a-z_0-9]+(\[3\])?;" …/driver_surface.hpp`, returns
only **10** lines, not the 14 stated: it requires exactly one space after the type
keyword, and four declarations pad the type column
(`float    gain;`, `uint32_t intent;`, `float    attack;`, `float    release;`,
`float    height_gain;`). Widening the separator to `\s+` fixes it:

```
grep -nE "^\s+(float|uint32_t)\s+[a-z_0-9]+(\[3\])?;" \
  src/cartridges/the_board/contracts/driver_surface.hpp
#   22:        float    gain;            // 0 manual … 1 coupling verbatim
#   25:        uint32_t intent;          // the ramp's rest target: 0 off, 1 on
#   26:        float    attack;          // 1/s — presence rise rate
#   27:        float    release;         // 1/s — presence fall rate
#   28:        float    height_gain;     // × profile.height_scale at the tick
#   36:        float rest_resultant[3];  // the music colour at gain 0
#   37:        float rest_amount;        // enveloped presence at gain 0
#   38:        float rest_variance;      // enveloped distinct-pc count at gain 0
#   39:        float gain;               // 0 manual … 1 coupling verbatim
#   48:        float rest_amp_lat;       // 1.0 — identity: the seed's dance
#   49:        float rest_amp_vert;      // 1.0
#   50:        float rest_tint_stim[3];  // {0,0,0} — the null branch's shape
#   51:        float rest_tint_mix;      // 0.0
#   52:        float gain;               // one gain, the seam's volume
```
→ **14 named members across four sub-structs**: `Fog{gain}` = 1,
`Aura{intent, attack, release, height_gain}` = 4,
`Checker{rest_resultant[3], rest_amount, rest_variance, gain}` = 4 members / 6
words, `Ribbon{rest_amp_lat, rest_amp_vert, rest_tint_stim[3], rest_tint_mix,
gain}` = 5 members / 7 words. 14 members, 18 words. The struct's own witness says
the same:

```cpp
static_assert(sizeof(DriverSurface) == 18 * sizeof(float),
    "DRIVER_LIVE is a whole-struct copy of the design row: a field added "
    "to one is added to the other by construction. 18 words — fog 1, "
```
The substantive count of 14 was right; only its recipe was wrong.

Named by a pipe (**10 of 14**, not listed as dormant): `fog.gain`;
`checker.rest_resultant`, `checker.rest_amount`, `checker.rest_variance`,
`checker.gain`; `ribbon.rest_amp_lat`, `ribbon.rest_amp_vert`,
`ribbon.rest_tint_stim`, `ribbon.rest_tint_mix`, `ribbon.gain`.

**Dormant: 4 rows.** The `Aura` sub-struct is a driver's room with no pipe on the
other side — nothing in `PARAM_LAYOUT` names an aura, and `VisualCanvas::tick`
never mentions it.

| parameter | declared in | type / default (`DRIVER_TABLE`) | current driver (what writes it today) |
| --- | --- | --- | --- |
| `DriverSurface::Aura::intent` | `driver_surface.hpp` | `uint32_t`, `0u` (off) | console dial `ORGAN_PARAM(DRIVERS, DriverSurface, aura.intent, BOOL, 0…1, 1)` on `DRIVER_LIVE`; also written by "key 3's door" and the mood policy force-off per the file's own banner. Read by `bodies/pawn.hpp` (`set_aura_enabled` / `set_pawn_aura_height` path). No pipe. |
| `DriverSurface::Aura::attack` | `driver_surface.hpp` | `float`, `1.0f` (1/s) | console dial `ORGAN_PARAM(DRIVERS, …, aura.attack, F32, 0.05…8, 0.05)`. No pipe. |
| `DriverSurface::Aura::release` | `driver_surface.hpp` | `float`, `1.5f` (1/s) | console dial `ORGAN_PARAM(DRIVERS, …, aura.release, F32, 0.05…8, 0.05)`. No pipe. |
| `DriverSurface::Aura::height_gain` | `driver_surface.hpp` | `float`, `1.0f` | console dial `ORGAN_PARAM(DRIVERS, …, aura.height_gain, F32, 0…2, 0.01)`. No pipe. |

### Group B — `src/cartridges/the_board/contracts/control_panel.hpp`

Census recipes — **corrected on the aggregate count**:

```
grep -nE "^inline constexpr" src/cartridges/the_board/contracts/control_panel.hpp | wc -l   # 17
grep -nE "^\s+float [a-z_0-9]+;" src/cartridges/the_board/contracts/control_panel.hpp | wc -l  #  9
```
The first recipe returns **17** lines: 16 scalar `inline constexpr float`
declarations (`FIELD_SLACK`, `FIELD_K`, `FIELD_FMAX`, `FIELD_OCCUPIER_GAIN`,
`FIELD_AUTHORED_GAIN`, `FIELD_ARCH_SLACK`, `ATRIUM_DOOR_CHANNEL_MIN`,
`FIELD_GAIN_CUBE`, `FIELD_GAIN_SPHERE`, `FIELD_GAIN_AGENT`, `FIELD_BEACON_R0`,
`FIELD_BEACON_R`, `FIELD_BEACON_S`, `FIELD_BEACON_LIFT`, `FIELD_BEACON_S_MAX`,
`POSSESSION_RADIUS`) plus **one** aggregate,
`inline constexpr PanelSurface PANEL_TABLE = {`. An earlier form of this paragraph
said "the two aggregates `PANEL_TABLE` / `PANEL_LIVE`"; `PANEL_LIVE` is not
`constexpr` and the recipe never matches it. Its own declaration, found separately:

```
grep -n "PANEL_LIVE" src/cartridges/the_board/contracts/control_panel.hpp
#    38:// that had NO transport get one below: PANEL_LIVE, the live surface
#   156:// The rests above are the DESIGN; PANEL_LIVE is what the writers
#   221:inline PanelSurface PANEL_LIVE = PANEL_TABLE;
```
So the file holds 16 free scalars, one constexpr design table (`PANEL_TABLE`), one
mutable live surface seeded from it (`PANEL_LIVE`), and 9 `PanelSurface` members.
The dormant total of 25 (16 + 9) is unchanged by the correction; both halves were
recounted independently above.

**Named by a pipe: 0.** No identifier from this file appears in any of the four
pipe decode/flush expressions (verified by the absence of `FIELD_`, `PANEL_LIVE`
and `POSSESSION_` from `src/coupling/visual_canvas.hpp` and from the flush blocks
quoted in §3.6).

**Dormant: 25 rows** (16 free scalars + 9 `PanelSurface` members).

| parameter | declared in | type / default | current driver (what writes it today) |
| --- | --- | --- | --- |
| `FIELD_SLACK` | `control_panel.hpp` | `float`, `3.0f` | authored constant; boot-pinned into `GPUDesignConfig::field_slack`, then the console dial `ORGAN_PARAM(CONFIG, GPUDesignConfig, field_slack, F32, 0…12, 0.05)`. No pipe. |
| `FIELD_K` | `control_panel.hpp` | `float`, `300.0f` | boot-pinned into `config.field_k`; console dial `Interaction · Field / k (accel)`, 0…1200. No pipe. |
| `FIELD_FMAX` | `control_panel.hpp` | `float`, `600.0f` | boot-pinned into `config.field_fmax`; console dial `Interaction · Field / f max`, 0…2400. No pipe. |
| `FIELD_OCCUPIER_GAIN` | `control_panel.hpp` | `float`, `1.0f` | boot-pinned into `config.field_occupier_gain`; console dial 0…4. No pipe. |
| `FIELD_AUTHORED_GAIN` | `control_panel.hpp` | `float`, `1.0f` | boot-pinned into `config.field_authored_gain`; console dial 0…4. No pipe. |
| `FIELD_ARCH_SLACK` | `control_panel.hpp` | `float`, `1.25f` | boot-pinned into `config.field_arch_slack`; console dial 0.5…4. No pipe. |
| `ATRIUM_DOOR_CHANNEL_MIN` | `control_panel.hpp` | `float`, `2.0f` (wu) | **nothing at runtime** — a gate-read threshold, no organ enrollment (`grep -n ATRIUM_DOOR_CHANNEL_MIN src/console/organ_params.inc` → no match) and no `config` twin. No pipe. |
| `FIELD_GAIN_CUBE` | `control_panel.hpp` | `float`, `4.0f` | boot-pinned into `config.field_gain_cube`; console dial 0…16. No pipe. |
| `FIELD_GAIN_SPHERE` | `control_panel.hpp` | `float`, `1.0f` | boot-pinned into `config.field_gain_sphere`; console dial 0…4. No pipe. |
| `FIELD_GAIN_AGENT` | `control_panel.hpp` | `float`, `4.0f` | boot-pinned into `config.field_gain_agent`; console dial 0…16. No pipe. |
| `FIELD_BEACON_R0` | `control_panel.hpp` | `float`, `25.0f` | seeds `PANEL_TABLE.beacon.r0`; live value is `PANEL_LIVE.beacon.r0`, console dial `Interaction · Beacon / inner radius`. No pipe. |
| `FIELD_BEACON_R` | `control_panel.hpp` | `float`, `120.0f` | seeds `PANEL_TABLE.beacon.r`; console dial `outer radius`, 0…480. No pipe. |
| `FIELD_BEACON_S` | `control_panel.hpp` | `float`, `200.0f` | seeds `PANEL_TABLE.beacon.s`; console dial `pull strength`, 0…`FIELD_BEACON_S_MAX`. No pipe. |
| `FIELD_BEACON_LIFT` | `control_panel.hpp` | `float`, `20.0f` | seeds `PANEL_TABLE.beacon.lift`; console dial `lift`, 0…80. No pipe. |
| `FIELD_BEACON_S_MAX` | `control_panel.hpp` | `float`, `FIELD_K - 1.0f` = `299.0f` | derived constant; used as the console dial's own max in `organ_params.inc` (`the_board::FIELD_BEACON_S_MAX`) and by the writer's clamp against live `config.field_k`. Never written. No pipe. |
| `POSSESSION_RADIUS` | `control_panel.hpp` | `float`, `20.0f` | seeds `PANEL_TABLE.possession.radius`; console dial `Interaction · Possession / reach`, 0.5…80. No pipe. |
| `PanelSurface::Beacon::r0` | `control_panel.hpp` | `float`, `FIELD_BEACON_R0` | console dial `ORGAN_PARAM(PANEL, PanelSurface, beacon.r0, F32, 0…100, 0.5)` on `PANEL_LIVE`. No pipe. |
| `PanelSurface::Beacon::r` | `control_panel.hpp` | `float`, `FIELD_BEACON_R` | console dial `beacon.r`, 0…480, step 2. No pipe. |
| `PanelSurface::Beacon::s` | `control_panel.hpp` | `float`, `FIELD_BEACON_S` | console dial `beacon.s`, 0…`FIELD_BEACON_S_MAX`, step 1; also clamped at the writer against live `config.field_k`. No pipe. |
| `PanelSurface::Beacon::lift` | `control_panel.hpp` | `float`, `FIELD_BEACON_LIFT` | console dial `beacon.lift`, 0…80, step 0.4. No pipe. |
| `PanelSurface::Camera::look_sens_init` | `control_panel.hpp` | `float`, `0.005f` | console dial `Interaction · Camera / look sens (anchor)`, 0.000625…0.04. Carried verbatim from the retired `CameraControls` (`direction/input.hpp`). No pipe. |
| `PanelSurface::Camera::look_sens_step` | `control_panel.hpp` | `float`, `1.25f` | console dial `look sens step`, 1.01…2.0. No pipe. |
| `PanelSurface::Camera::look_sens_range` | `control_panel.hpp` | `float`, `8.0f` | console dial `look sens range`, 1…32. No pipe. |
| `PanelSurface::Camera::scroll_zoom_scale` | `control_panel.hpp` | `float`, `2.0f` | console dial `scroll zoom`, 0…8. No pipe. |
| `PanelSurface::Possession::radius` | `control_panel.hpp` | `float`, `POSSESSION_RADIUS` | console dial `Interaction · Possession / reach`; squared at the one read site from the live value. No pipe. |

### Group C — `src/console/organ_params.inc`

Enrollment census recipe (the grammar is `tools/organ_parse.py`'s `MACRO` regex,
`^(ORGAN_PARAM(?:_GEN|_DEF|_DEFONLY|_RO)?)(_NS)?\s*\((.*)\)\s*$`):

```
grep -c "^ORGAN_PARAM" src/console/organ_params.inc                       # -> 381
grep -o "^ORGAN_PARAM[A-Z_]*" src/console/organ_params.inc | sort | uniq -c
#   120 ORGAN_PARAM      110 ORGAN_PARAM_DEF   74 ORGAN_PARAM_DEFONLY
#    42 ORGAN_PARAM_GEN   20 ORGAN_PARAM_RO    15 ORGAN_PARAM_NS
```
120+110+74+42+20+15 = **381 enrolled rows.**

By block (`ORGAN_BLOCK_*` → live instance, from `src/console/organ_registry.hpp`
`organ_block_home`): AGENT_ROOM 110, CONFIG 97, definition-only 74, WORLD 20,
RIBBON_SPAWN 20, CANVAS 15, DRIVERS 14, PAWN 9, PANEL 9, LIGHTING 4, ORBS 4,
RIBBON 3, INDOOR 2.

**Block census recipe (R2), added so the distribution above is re-runnable.** The
BLOCK is the macro's first argument, except on the `_NS` twins where it is the
second (the first is the namespace) and on the `DEFONLY` form, which has no block
at all — its first argument is the TYPE, and its block is a sentinel derived from
its DEFKIND (`ORGAN_DEFONLY_BLOCK_MOOD → ORGAN_BLOCK_NONE`,
`ORGAN_DEFONLY_BLOCK_ORB_MOOD → ORGAN_BLOCK_NONE_ORB`, both in
`src/console/organ_registry.hpp`):

```
grep -n "^ORGAN_PARAM" src/console/organ_params.inc \
  | sed -E 's/^[0-9]+:ORGAN_PARAM(_GEN|_DEF|_DEFONLY|_RO)?(_NS)?\(//' \
  | awk -F, '{ if ($0 ~ /^(F32|U32|BOOL|VEC3|VEC4)[ ,]/) print "DEFONLY";
               else if (NF>0) { b=$1; if (b ~ /^(canvas|the_board)$/) b=$2;
                                gsub(/^[ \t]+|[ \t]+$/,"",b); print b } }' \
  | sort | uniq -c | sort -rn
#   110 AGENT_ROOM     97 CONFIG      74 DEFONLY     20 WORLD
#    20 RIBBON_SPAWN   15 CANVAS      14 DRIVERS      9 PAWN
#     9 PANEL           4 ORBS         4 LIGHTING     3 RIBBON
#     2 INDOOR
```
Sum = 381, matching the form census above. Splitting the 74 DEFONLY rows by
DEFKIND, and the 110 `ORGAN_PARAM_DEF` rows likewise:

```
grep "^ORGAN_PARAM_DEFONLY" src/console/organ_params.inc \
  | awk -F, '{ for(i=1;i<=NF;i++) if ($i ~ /^ *(MOOD|ORB_MOOD) *$/) {gsub(/ /,"",$i); print $i; break} }' \
  | sort | uniq -c
#   55 MOOD      19 ORB_MOOD

grep "^ORGAN_PARAM_DEF(" src/console/organ_params.inc \
  | awk -F, '{ for(i=1;i<=NF;i++) if ($i ~ /^ *(MOOD|TIER|BEHAVIOR|ORB_MOOD) *$/) {gsub(/ /,"",$i); print $i; break} }' \
  | sort | uniq -c
#   78 BEHAVIOR  32 TIER
```
All 110 `_DEF` rows are AGENT_ROOM rows (78 BEHAVIOR + 32 TIER), which is why
AGENT_ROOM's block count and the `_DEF` form count are the same number.

**Named by a pipe — 30 rows, excluded from the dormant list:**
* 5 `ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, …)` witnesses that ARE the pipes'
  terminal fields: `fog_density`, `fog_color`, `checker_resultant`,
  `checker_music_amount`, `checker_music_variance`.
* 10 `ORGAN_PARAM(DRIVERS, DriverSurface, …)` seam terms read inside a pipe flush:
  `fog.gain`, `checker.rest_resultant`, `checker.rest_amount`,
  `checker.rest_variance`, `checker.gain`, `ribbon.rest_amp_lat`,
  `ribbon.rest_amp_vert`, `ribbon.rest_tint_stim`, `ribbon.rest_tint_mix`,
  `ribbon.gain`.
* 15 `ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, …)` envelope terms read inside
  `VisualCanvas::tick`: `fog_span`, `checker_read_span`, `checker_attack`,
  `checker_release`, `swell_ceiling`, `swell_ramp`, `swell_attack`,
  `swell_release`, `pitch_vec_origin`, `tint_luma`, `tint_chroma`,
  `tint_mix_max`, `tint_mix_attack`, `tint_mix_release`, `tint_hue_span`.

**Dormant: 381 − 30 = 351 rows.**

Driver classification, read off the macro form (grammar quoted in the file's own
banner and compiled by `src/console/organ_registry.hpp`):
* `ORGAN_PARAM` → "console dial (live)": the dial writes the block's live instance
  directly; boot value comes from that block's authored table.
* `ORGAN_PARAM_GEN` → "console dial (generational)": the edit lands at the
  author's next natural event; the row wears "on respawn".
* `ORGAN_PARAM_DEF` → "console dial + definition": writes the live instance AND
  the named definition (`DEFKIND DEFSTRUCT::DEFFIELD`).
* `ORGAN_PARAM_DEFONLY` → "definition constant only (no instance)": the block is a
  sentinel (`ORGAN_BLOCK_NONE` for `MOOD`, `ORGAN_BLOCK_NONE_ORB` for `ORB_MOOD`);
  preview is refused; the value reaches the picture only when the definition is
  re-spoken.
* `ORGAN_PARAM_RO` → "WITNESS — console never writes": metered only, no range. Per-row
  runtime writers for the 15 dormant witnesses are named in §3.9 below.

**The recipe that produces the "current driver" column, row by row (R2).** The
classification above is a pure function of the macro form, so it re-runs as one
command. The GROUP and LABEL are always the last two quoted strings on the line,
in every one of the six forms, so a reader can join this output to the table below
by label:

```
grep -n "^ORGAN_PARAM" src/console/organ_params.inc \
  | awk -F'"' '{ split($1, h, ":"); form = h[2]; sub(/\(.*/, "", form);
                 print h[1] "\t" form "\t" $(NF-3) "\t" $(NF-1) }'
#   63	ORGAN_PARAM_DEFONLY	Sky & Light · Sun	direction (centre)
#   64	ORGAN_PARAM_DEFONLY	Sky & Light · Sun	azimuth spread (±deg)
#   65	ORGAN_PARAM_DEFONLY	Sky & Light · Sun	elevation spread (±deg)
#   66	ORGAN_PARAM_RO	Sky & Light · Sun	direction (drawn)
#   67	ORGAN_PARAM_RO	Sky & Light · Sun	colour (drawn)
#   68	ORGAN_PARAM_RO	Sky & Light · Sun	intensity (drawn)
#   69	ORGAN_PARAM_RO	Sky & Light · Sun	ambient (drawn)
#   81	ORGAN_PARAM	Sky & Light · Dome	dome radius
#   … (381 lines total)
```
`… | wc -l` → 381, and `… | cut -f2 | sort | uniq -c` reproduces the form census
(120 `ORGAN_PARAM`, 110 `_DEF`, 74 `_DEFONLY`, 42 `_GEN`, 20 `_RO`, 15 `_NS`).
Apply the five-line mapping above to column 2 and the "current driver" column of
the table below falls out for all 381 rows; the 15 `_NS` rows take the plain
`ORGAN_PARAM` reading ("console dial (live)"), since `_NS` is a namespace twin of
the same form and not a form of its own — the file's own banner states this
("EVERY FORM HAS AN _NS TWIN taking the enrolled struct's NAMESPACE first").

**GAP — the enrollment list carries no DEFAULT column, so the table below has
none.** The unit asked for "its type/default"; Groups A and B carry defaults
because their declaring files are authored tables. `src/console/organ_params.inc`
is not: its grammar, quoted verbatim from the file's own banner
(`sed -n '15,44p' src/console/organ_params.inc`), is

```
//   ORGAN_PARAM(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL)
//   ORGAN_PARAM_GEN(… the same …)      a GENERATIONAL dial: the edit lands
//                                      at the author's next natural event,
//                                      and the row wears `on respawn`
//   ORGAN_PARAM_DEF(… , DEFKIND, DEFSTRUCT, DEFFIELD)
//                                      the same, plus the DEFINITION it writes
//   ORGAN_PARAM_DEFONLY(TYPE, MIN, MAX, STEP, GROUP, LABEL,
//                       DEFKIND, DEFSTRUCT, DEFFIELD)
//                                      a definition with NO INSTANCE:
//                                      preview on it is refused
//   ORGAN_PARAM_RO(BLOCK, STRUCT, FIELD, TYPE, GROUP, LABEL)
//                                      a WITNESS: metered, never written,
//                                      and carrying no range
```
— MIN, MAX and STEP, and no DEFAULT in any of the six forms. The column is
**absent from the source**, not withheld from this report. A row's boot value is
whatever its block's live instance holds, and each block's live instance is seeded
from its own authored table. That mapping is itself a recipe:

```
grep -n "ORGAN_BLOCK_" src/console/organ_registry.hpp | sed -n '/case ORGAN_BLOCK/p'
git grep -n "_TABLE = \|_LIVE = " -- src/
```

| block | live instance (`organ_block_home`, `src/console/organ_registry.hpp`) | where its default value is authored | rows |
| --- | --- | --- | --- |
| `ORGAN_BLOCK_CONFIG` | `g_home->organ_config_home()` → `GPUState::config_` (`realization/state.hpp`, symbol `GPUState::organ_config_home`) | `GPUDesignConfig`'s own declaration plus the boot pins in `t7::the_board::Cartridge` (each dormant CONFIG row's pin is named in its Group B / §3.9 entry) | 97 |
| `ORGAN_BLOCK_LIGHTING` | `g_home->organ_lighting_home()` → `GPUState::lightingStage_` | never authored as a constant — drawn each mood entry by `apply_mood_lighting` (`direction/mood.hpp`) from the live regime | 4 |
| `ORGAN_BLOCK_AGENT_ROOM` | `g_home->organ_agent_room_home()` → `GPUAgentRoomConstants agentRoomStage_{}` (value-initialised) | `AGENT_TIER_GAINS` / `AGENT_BEHAVIORS` (`contracts/agent_tiers.hpp`), carried through `TIER_LIVE` / `BEHAVIOR_LIVE` and memcpy'd in by `GPUState::upload_agent_registries` | 110 |
| `ORGAN_BLOCK_DRIVERS` | `&the_board::DRIVER_LIVE` | `DRIVER_TABLE` (`contracts/driver_surface.hpp`), `inline DriverSurface DRIVER_LIVE = DRIVER_TABLE;` | 14 |
| `ORGAN_BLOCK_PAWN` | `&the_board::PAWN_AURA_LIVE` | `PAWN_AURA_DEFAULT` (`contracts/pawn_surface.hpp`), `inline PawnAuraProfile PAWN_AURA_LIVE = PAWN_AURA_DEFAULT;` | 9 |
| `ORGAN_BLOCK_ORBS` | `&the_board::ORB_CONSOLE_LIVE` | `ORB_CONSOLE` (`contracts/orb_surface.hpp`), `inline OrbConsole ORB_CONSOLE_LIVE = ORB_CONSOLE;` | 4 |
| `ORGAN_BLOCK_PANEL` | `&the_board::PANEL_LIVE` | `PANEL_TABLE` (`contracts/control_panel.hpp`), `inline PanelSurface PANEL_LIVE = PANEL_TABLE;` — Group B's own defaults column | 9 |
| `ORGAN_BLOCK_RIBBON` | `&the_board::RIBBON_LIVE` | `RIBBON_TABLE` (`contracts/ribbon_surface.hpp`) | 3 |
| `ORGAN_BLOCK_INDOOR` | `&the_board::INDOOR_LIVE` | `INDOOR_TABLE` (`contracts/indoor_module.hpp`) | 2 |
| `ORGAN_BLOCK_CANVAS` | `&canvas::CANVAS_LIVE` | `CANVAS_TABLE` (`src/coupling/canvas_surface.hpp`) — all 15 are pipe-named, so none is dormant | 15 |
| `ORGAN_BLOCK_WORLD` | `&the_board::WORLD_DRAW_LIVE` | `WORLD_DRAW_TABLE` (`contracts/mood_constants.hpp`) | 20 |
| `ORGAN_BLOCK_RIBBON_SPAWN` | `&the_board::RIBBON_SPAWN_LIVE` | `RIBBON_SPAWN_TABLE` (`contracts/ribbon_surface.hpp`) | 20 |
| `ORGAN_BLOCK_NONE` (sentinel, `MOOD` DEFONLY) | none — `is_defonly(block)` (`src/console/organ_registry.hpp`) is true | the `MoodProfile` definitions themselves; the row reaches the picture only when the definition is re-spoken | 55 |
| `ORGAN_BLOCK_NONE_ORB` (sentinel, `ORB_MOOD` DEFONLY) | none — same sentinel test | the `OrbMoodConfig` definitions (`contracts/orb_surface.hpp`, `bodies/orbs.hpp`) | 19 |

Rows sum to 381. Because a default lives in one of fourteen different places
depending on the row's block, and because no single command prints all 381 of
them, the table below carries `type` and `min…max /step` — which the enrollment
line does carry, verbatim — and points at this table for the default's home rather
than transcribing 351 authored values from fourteen files.

Columns: `| parameter (label) | group (section · group) | block | type | min…max /step | enrolled field (→ definition target) | current driver |`

| parameter | group | block | type | range | enrolled field → definition target | current driver |
| --- | --- | --- | --- | --- | --- | --- |
| `direction (centre)` | Sky & Light · Sun | - | VEC3 | -1.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.sun_direction` | definition constant only (no instance) |
| `azimuth spread (±deg)` | Sky & Light · Sun | - | F32 | 0.0f...180.0f /1.0f | `MOOD MoodProfile::atmos.sun_az_spread_deg` | definition constant only (no instance) |
| `elevation spread (±deg)` | Sky & Light · Sun | - | F32 | 0.0f...45.0f /0.5f | `MOOD MoodProfile::atmos.sun_el_spread_deg` | definition constant only (no instance) |
| `direction (drawn)` | Sky & Light · Sun | LIGHTING | VEC3 | - (no range) | `GPULighting::sun.direction` | WITNESS - console never writes |
| `colour (drawn)` | Sky & Light · Sun | LIGHTING | VEC3 | - (no range) | `GPULighting::sun.color` | WITNESS - console never writes |
| `intensity (drawn)` | Sky & Light · Sun | LIGHTING | F32 | - (no range) | `GPULighting::sun.intensity` | WITNESS - console never writes |
| `ambient (drawn)` | Sky & Light · Sun | LIGHTING | F32 | - (no range) | `GPULighting::sun.ambient` | WITNESS - console never writes |
| `dome radius` | Sky & Light · Dome | ORBS | F32 | 0.0f...2000.0f /5.0f | `OrbConsole::dome_radius` | console dial (live) |
| `base size` | Sky & Light · Dome | ORBS | F32 | 0.0f...12.0f /0.05f | `OrbConsole::base_size` | console dial (live) |
| `enabled` | Sky & Light · Orbs | - | BOOL | 0.0f...1.0f /1.0f | `ORB_MOOD OrbMoodConfig::enabled` | definition constant only (no instance) |
| `count` | Sky & Light · Orbs | - | U32 | 0.0f...256.0f /1.0f | `ORB_MOOD OrbMoodConfig::count` | definition constant only (no instance) |
| `palette id` | Sky & Light · Orbs | - | U32 | 0.0f...3.0f /1.0f | `ORB_MOOD OrbMoodConfig::palette_id` | definition constant only (no instance) |
| `drag` | Sky & Light · Orbs | - | F32 | 0.01f...2.0f /0.01f | `ORB_MOOD OrbMoodConfig::drag` | definition constant only (no instance) |
| `brightness` | Sky & Light · Orbs | - | F32 | 0.0f...1.0f /0.005f | `ORB_MOOD OrbMoodConfig::brightness` | definition constant only (no instance) |
| `speed mult` | Sky & Light · Motion — all rules | ORBS | F32 | 0.0f...4.0f /0.01f | `OrbConsole::speed_mult` | console dial (live) |
| `noise floor` | Sky & Light · Motion — all rules | ORBS | F32 | 0.0f...3.0f /0.005f | `OrbConsole::noise_floor` | console dial (live) |
| `rotation speed (rad/s)` | Sky & Light · Motion — all rules | - | F32 | -2.0f...2.0f /0.005f | `ORB_MOOD OrbMoodConfig::rotation_speed` | definition constant only (no instance) |
| `rotation axis` | Sky & Light · Motion — all rules | - | VEC3 | -1.0f...1.0f /0.01f | `ORB_MOOD OrbMoodConfig::rotation_axis` | definition constant only (no instance) |
| `drag × brownian rule` | Sky & Light · Motion — all rules | - | F32 | 0.02f...4.0f /0.02f | `ORB_MOOD OrbMoodConfig::rule_drag_brownian` | definition constant only (no instance) |
| `drag × orbital rule` | Sky & Light · Motion — all rules | - | F32 | 0.02f...4.0f /0.02f | `ORB_MOOD OrbMoodConfig::rule_drag_orbital` | definition constant only (no instance) |
| `drag × frozen rule` | Sky & Light · Motion — all rules | - | F32 | 0.02f...4.0f /0.02f | `ORB_MOOD OrbMoodConfig::rule_drag_frozen` | definition constant only (no instance) |
| `drag × flocking rule` | Sky & Light · Motion — all rules | - | F32 | 0.02f...4.0f /0.02f | `ORB_MOOD OrbMoodConfig::rule_drag_flocking` | definition constant only (no instance) |
| `orbital speed (rad/s)` | Sky & Light · Orbital rule | - | F32 | 0.005f...1.0f /0.005f | `ORB_MOOD OrbMoodConfig::orbital_base_speed` | definition constant only (no instance) |
| `separation radius` | Sky & Light · Flocking rule | - | F32 | 1.0f...200.0f /1.0f | `ORB_MOOD OrbMoodConfig::flock_sep_radius` | definition constant only (no instance) |
| `alignment radius` | Sky & Light · Flocking rule | - | F32 | 2.0f...480.0f /2.0f | `ORB_MOOD OrbMoodConfig::flock_align_radius` | definition constant only (no instance) |
| `cohesion radius` | Sky & Light · Flocking rule | - | F32 | 4.0f...800.0f /4.0f | `ORB_MOOD OrbMoodConfig::flock_coh_radius` | definition constant only (no instance) |
| `separation weight` | Sky & Light · Flocking rule | - | F32 | 0.5f...120.0f /0.5f | `ORB_MOOD OrbMoodConfig::flock_sep_weight` | definition constant only (no instance) |
| `alignment weight` | Sky & Light · Flocking rule | - | F32 | 0.2f...32.0f /0.2f | `ORB_MOOD OrbMoodConfig::flock_align_weight` | definition constant only (no instance) |
| `cohesion weight` | Sky & Light · Flocking rule | - | F32 | 0.25f...60.0f /0.25f | `ORB_MOOD OrbMoodConfig::flock_coh_weight` | definition constant only (no instance) |
| `max speed` | Sky & Light · Flocking rule | - | F32 | 1.0f...240.0f /1.0f | `ORB_MOOD OrbMoodConfig::flock_max_speed` | definition constant only (no instance) |
| `cathedral` | Sky & Light · Schemes | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::scheme_weights[0]` | console dial (generational) |
| `quartet` | Sky & Light · Schemes | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::scheme_weights[1]` | console dial (generational) |
| `gallery` | Sky & Light · Schemes | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::scheme_weights[2]` | console dial (generational) |
| `sanctum` | Sky & Light · Schemes | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::scheme_weights[3]` | console dial (generational) |
| `to open sunset` | Sky & Light · Portals | WORLD | VEC3 | 0.0f...1.0f /0.01f | `WorldDrawSurface::portal_colors[0][0]` | console dial (generational) |
| `to indoor flat` | Sky & Light · Portals | WORLD | VEC3 | 0.0f...1.0f /0.01f | `WorldDrawSurface::portal_colors[1][0]` | console dial (generational) |
| `to indoor vault` | Sky & Light · Portals | WORLD | VEC3 | 0.0f...1.0f /0.01f | `WorldDrawSurface::portal_colors[2][0]` | console dial (generational) |
| `to finite outdoor` | Sky & Light · Portals | WORLD | VEC3 | 0.0f...1.0f /0.01f | `WorldDrawSurface::portal_colors[3][0]` | console dial (generational) |
| `to open night` | Sky & Light · Portals | WORLD | VEC3 | 0.0f...1.0f /0.01f | `WorldDrawSurface::portal_colors[4][0]` | console dial (generational) |
| `to open noon` | Sky & Light · Portals | WORLD | VEC3 | 0.0f...1.0f /0.01f | `WorldDrawSurface::portal_colors[5][0]` | console dial (generational) |
| `to atrium` | Sky & Light · Portals | WORLD | VEC3 | 0.0f...1.0f /0.01f | `WorldDrawSurface::portal_colors[6][0]` | console dial (generational) |
| `back portal` | Sky & Light · Portals | WORLD | VEC3 | 0.0f...1.0f /0.01f | `WorldDrawSurface::portal_color_back[0]` | console dial (generational) |
| `draw · regime 1` | Atmosphere · Regimes | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::regime_weight[0]` | definition constant only (no instance) |
| `draw · regime 2` | Atmosphere · Regimes | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::regime_weight[1]` | definition constant only (no instance) |
| `draw · regime 3` | Atmosphere · Regimes | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::regime_weight[2]` | definition constant only (no instance) |
| `draw · regime 4` | Atmosphere · Regimes | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::regime_weight[3]` | definition constant only (no instance) |
| `sun colour` | Atmosphere · Regime 1 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[0].sun_color` | definition constant only (no instance) |
| `sun colour spread (±bright)` | Atmosphere · Regime 1 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[0].sun_color_spread` | definition constant only (no instance) |
| `intensity` | Atmosphere · Regime 1 | - | F32 | 0.0f...4.0f /0.01f | `MOOD MoodProfile::atmos.regime[0].intensity` | definition constant only (no instance) |
| `intensity spread` | Atmosphere · Regime 1 | - | F32 | 0.0f...2.0f /0.01f | `MOOD MoodProfile::atmos.regime[0].intensity_spread` | definition constant only (no instance) |
| `ambient` | Atmosphere · Regime 1 | - | F32 | 0.0f...1.0f /0.005f | `MOOD MoodProfile::atmos.regime[0].ambient` | definition constant only (no instance) |
| `ambient spread` | Atmosphere · Regime 1 | - | F32 | 0.0f...0.5f /0.005f | `MOOD MoodProfile::atmos.regime[0].ambient_spread` | definition constant only (no instance) |
| `fog density` | Atmosphere · Regime 1 | - | F32 | 0.0f...0.05f /0.0002f | `MOOD MoodProfile::atmos.regime[0].fog_density` | definition constant only (no instance) |
| `fog density spread` | Atmosphere · Regime 1 | - | F32 | 0.0f...0.02f /0.0001f | `MOOD MoodProfile::atmos.regime[0].fog_density_spread` | definition constant only (no instance) |
| `fog colour` | Atmosphere · Regime 1 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[0].fog_color` | definition constant only (no instance) |
| `fog colour spread (±bright)` | Atmosphere · Regime 1 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[0].fog_color_spread` | definition constant only (no instance) |
| `clear colour` | Atmosphere · Regime 1 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[0].clear_color` | definition constant only (no instance) |
| `clear colour spread (±bright)` | Atmosphere · Regime 1 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[0].clear_color_spread` | definition constant only (no instance) |
| `sun colour` | Atmosphere · Regime 2 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[1].sun_color` | definition constant only (no instance) |
| `sun colour spread (±bright)` | Atmosphere · Regime 2 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[1].sun_color_spread` | definition constant only (no instance) |
| `intensity` | Atmosphere · Regime 2 | - | F32 | 0.0f...4.0f /0.01f | `MOOD MoodProfile::atmos.regime[1].intensity` | definition constant only (no instance) |
| `intensity spread` | Atmosphere · Regime 2 | - | F32 | 0.0f...2.0f /0.01f | `MOOD MoodProfile::atmos.regime[1].intensity_spread` | definition constant only (no instance) |
| `ambient` | Atmosphere · Regime 2 | - | F32 | 0.0f...1.0f /0.005f | `MOOD MoodProfile::atmos.regime[1].ambient` | definition constant only (no instance) |
| `ambient spread` | Atmosphere · Regime 2 | - | F32 | 0.0f...0.5f /0.005f | `MOOD MoodProfile::atmos.regime[1].ambient_spread` | definition constant only (no instance) |
| `fog density` | Atmosphere · Regime 2 | - | F32 | 0.0f...0.05f /0.0002f | `MOOD MoodProfile::atmos.regime[1].fog_density` | definition constant only (no instance) |
| `fog density spread` | Atmosphere · Regime 2 | - | F32 | 0.0f...0.02f /0.0001f | `MOOD MoodProfile::atmos.regime[1].fog_density_spread` | definition constant only (no instance) |
| `fog colour` | Atmosphere · Regime 2 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[1].fog_color` | definition constant only (no instance) |
| `fog colour spread (±bright)` | Atmosphere · Regime 2 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[1].fog_color_spread` | definition constant only (no instance) |
| `clear colour` | Atmosphere · Regime 2 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[1].clear_color` | definition constant only (no instance) |
| `clear colour spread (±bright)` | Atmosphere · Regime 2 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[1].clear_color_spread` | definition constant only (no instance) |
| `sun colour` | Atmosphere · Regime 3 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[2].sun_color` | definition constant only (no instance) |
| `sun colour spread (±bright)` | Atmosphere · Regime 3 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[2].sun_color_spread` | definition constant only (no instance) |
| `intensity` | Atmosphere · Regime 3 | - | F32 | 0.0f...4.0f /0.01f | `MOOD MoodProfile::atmos.regime[2].intensity` | definition constant only (no instance) |
| `intensity spread` | Atmosphere · Regime 3 | - | F32 | 0.0f...2.0f /0.01f | `MOOD MoodProfile::atmos.regime[2].intensity_spread` | definition constant only (no instance) |
| `ambient` | Atmosphere · Regime 3 | - | F32 | 0.0f...1.0f /0.005f | `MOOD MoodProfile::atmos.regime[2].ambient` | definition constant only (no instance) |
| `ambient spread` | Atmosphere · Regime 3 | - | F32 | 0.0f...0.5f /0.005f | `MOOD MoodProfile::atmos.regime[2].ambient_spread` | definition constant only (no instance) |
| `fog density` | Atmosphere · Regime 3 | - | F32 | 0.0f...0.05f /0.0002f | `MOOD MoodProfile::atmos.regime[2].fog_density` | definition constant only (no instance) |
| `fog density spread` | Atmosphere · Regime 3 | - | F32 | 0.0f...0.02f /0.0001f | `MOOD MoodProfile::atmos.regime[2].fog_density_spread` | definition constant only (no instance) |
| `fog colour` | Atmosphere · Regime 3 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[2].fog_color` | definition constant only (no instance) |
| `fog colour spread (±bright)` | Atmosphere · Regime 3 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[2].fog_color_spread` | definition constant only (no instance) |
| `clear colour` | Atmosphere · Regime 3 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[2].clear_color` | definition constant only (no instance) |
| `clear colour spread (±bright)` | Atmosphere · Regime 3 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[2].clear_color_spread` | definition constant only (no instance) |
| `sun colour` | Atmosphere · Regime 4 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[3].sun_color` | definition constant only (no instance) |
| `sun colour spread (±bright)` | Atmosphere · Regime 4 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[3].sun_color_spread` | definition constant only (no instance) |
| `intensity` | Atmosphere · Regime 4 | - | F32 | 0.0f...4.0f /0.01f | `MOOD MoodProfile::atmos.regime[3].intensity` | definition constant only (no instance) |
| `intensity spread` | Atmosphere · Regime 4 | - | F32 | 0.0f...2.0f /0.01f | `MOOD MoodProfile::atmos.regime[3].intensity_spread` | definition constant only (no instance) |
| `ambient` | Atmosphere · Regime 4 | - | F32 | 0.0f...1.0f /0.005f | `MOOD MoodProfile::atmos.regime[3].ambient` | definition constant only (no instance) |
| `ambient spread` | Atmosphere · Regime 4 | - | F32 | 0.0f...0.5f /0.005f | `MOOD MoodProfile::atmos.regime[3].ambient_spread` | definition constant only (no instance) |
| `fog density` | Atmosphere · Regime 4 | - | F32 | 0.0f...0.05f /0.0002f | `MOOD MoodProfile::atmos.regime[3].fog_density` | definition constant only (no instance) |
| `fog density spread` | Atmosphere · Regime 4 | - | F32 | 0.0f...0.02f /0.0001f | `MOOD MoodProfile::atmos.regime[3].fog_density_spread` | definition constant only (no instance) |
| `fog colour` | Atmosphere · Regime 4 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[3].fog_color` | definition constant only (no instance) |
| `fog colour spread (±bright)` | Atmosphere · Regime 4 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[3].fog_color_spread` | definition constant only (no instance) |
| `clear colour` | Atmosphere · Regime 4 | - | VEC3 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[3].clear_color` | definition constant only (no instance) |
| `clear colour spread (±bright)` | Atmosphere · Regime 4 | - | F32 | 0.0f...1.0f /0.01f | `MOOD MoodProfile::atmos.regime[3].clear_color_spread` | definition constant only (no instance) |
| `ring (draw authority)` | Atmosphere · Veil | CONFIG | F32 | 265.0f...349.0f /0.5f | `GPUDesignConfig::veil_ring` | console dial (live) |
| `icing band` | Atmosphere · Veil | CONFIG | F32 | 0.0f...60.0f /0.25f | `GPUDesignConfig::veil_icing` | console dial (live) |
| `LOD0 core` | Atmosphere · Veil | CONFIG | F32 | 0.0f...175.0f /1.0f | `GPUDesignConfig::lod0_radius` | console dial (live) |
| `rim dither (>0.5)` | Atmosphere · Veil | CONFIG | F32 | 0.0f...1.0f /1.0f | `GPUDesignConfig::veil_dither` | console dial (live) |
| `authority (driven)` | Atmosphere · Veil | CONFIG | F32 | - (no range) | `GPUDesignConfig::veil_strength` | WITNESS - console never writes |
| `fade alpha (driven)` | Atmosphere · Transition | CONFIG | F32 | - (no range) | `GPUDesignConfig::fade_alpha` | WITNESS - console never writes |
| `fade colour (driven)` | Atmosphere · Transition | CONFIG | VEC3 | - (no range) | `GPUDesignConfig::fade_color` | WITNESS - console never writes |
| `centre` | Terrain · Palette 0 sand | CONFIG | VEC3 | 0.0f...1.0f /0.01f | `GPUDesignConfig::palette_center[0][0]` | console dial (live) |
| `light` | Terrain · Palette 0 sand | CONFIG | VEC3 | 0.0f...1.0f /0.01f | `GPUDesignConfig::palette_light[0][0]` | console dial (live) |
| `centre` | Terrain · Palette 1 salmon | CONFIG | VEC3 | 0.0f...1.0f /0.01f | `GPUDesignConfig::palette_center[1][0]` | console dial (live) |
| `light` | Terrain · Palette 1 salmon | CONFIG | VEC3 | 0.0f...1.0f /0.01f | `GPUDesignConfig::palette_light[1][0]` | console dial (live) |
| `centre` | Terrain · Palette 2 green | CONFIG | VEC3 | 0.0f...1.0f /0.01f | `GPUDesignConfig::palette_center[2][0]` | console dial (live) |
| `light` | Terrain · Palette 2 green | CONFIG | VEC3 | 0.0f...1.0f /0.01f | `GPUDesignConfig::palette_light[2][0]` | console dial (live) |
| `centre` | Terrain · Palette 3 warm | CONFIG | VEC3 | 0.0f...1.0f /0.01f | `GPUDesignConfig::palette_center[3][0]` | console dial (live) |
| `light` | Terrain · Palette 3 warm | CONFIG | VEC3 | 0.0f...1.0f /0.01f | `GPUDesignConfig::palette_light[3][0]` | console dial (live) |
| `sand` | Terrain · Palette mix | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::palette_weight[0]` | console dial (live) |
| `salmon` | Terrain · Palette mix | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::palette_weight[1]` | console dial (live) |
| `green` | Terrain · Palette mix | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::palette_weight[2]` | console dial (live) |
| `warm` | Terrain · Palette mix | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::palette_weight[3]` | console dial (live) |
| `clock (beats)` | Terrain · Motion | CONFIG | F32 | 0.0f...64.0f /0.25f | `GPUDesignConfig::terrain_time` | console dial (live) |
| `0 continental` | Terrain · Bands | CONFIG | F32 | -1.0f...1.0f /0.01f | `GPUDesignConfig::band_blend_0` | console dial (live) |
| `1 regional` | Terrain · Bands | CONFIG | F32 | -1.0f...1.0f /0.01f | `GPUDesignConfig::band_blend_1` | console dial (live) |
| `2 local` | Terrain · Bands | CONFIG | F32 | -1.0f...1.0f /0.01f | `GPUDesignConfig::band_blend_2` | console dial (live) |
| `3 detail` | Terrain · Bands | CONFIG | F32 | -1.0f...1.0f /0.01f | `GPUDesignConfig::band_blend_3` | console dial (live) |
| `4 fine` | Terrain · Bands | CONFIG | F32 | -1.0f...1.0f /0.01f | `GPUDesignConfig::band_blend_4` | console dial (live) |
| `5 tectonic` | Terrain · Bands | CONFIG | F32 | -1.0f...1.0f /0.01f | `GPUDesignConfig::band_blend_5` | console dial (live) |
| `0 continental` | Terrain · Band phase | CONFIG | F32 | 0.0f...64.0f /0.25f | `GPUDesignConfig::band_phase_origin_0` | console dial (live) |
| `1 regional` | Terrain · Band phase | CONFIG | F32 | 0.0f...64.0f /0.25f | `GPUDesignConfig::band_phase_origin_1` | console dial (live) |
| `2 local` | Terrain · Band phase | CONFIG | F32 | 0.0f...64.0f /0.25f | `GPUDesignConfig::band_phase_origin_2` | console dial (live) |
| `3 detail` | Terrain · Band phase | CONFIG | F32 | 0.0f...64.0f /0.25f | `GPUDesignConfig::band_phase_origin_3` | console dial (live) |
| `4 fine` | Terrain · Band phase | CONFIG | F32 | 0.0f...64.0f /0.25f | `GPUDesignConfig::band_phase_origin_4` | console dial (live) |
| `5 tectonic` | Terrain · Band phase | CONFIG | F32 | 0.0f...64.0f /0.25f | `GPUDesignConfig::band_phase_origin_5` | console dial (live) |
| `colour shift` | Terrain · Modes | CONFIG | F32 | -1.0f...1.0f /0.01f | `GPUDesignConfig::mode_color_shift` | console dial (live) |
| `checker scatter` | Terrain · Modes | CONFIG | F32 | -1.0f...1.0f /0.01f | `GPUDesignConfig::mode_checker_scatter` | console dial (live) |
| `palette target (idx)` | Terrain · Modes | CONFIG | F32 | 0.0f...3.0f /1.0f | `GPUDesignConfig::mode_palette_target` | console dial (live) |
| `palette intensity` | Terrain · Modes | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::mode_palette_intensity` | console dial (live) |
| `discrete tier (idx)` | Terrain · Modes | CONFIG | F32 | 0.0f...4.0f /1.0f | `GPUDesignConfig::mode_discrete_tier` | console dial (live) |
| `tick scale` | Terrain · GoL | CONFIG | F32 | 0.1f...4.0f /0.02f | `GPUDesignConfig::mode_gol_tick_scale` | console dial (live) |
| `height scale` | Terrain · GoL | CONFIG | F32 | 0.0f...4.0f /0.02f | `GPUDesignConfig::mode_gol_height_scale` | console dial (live) |
| `enable` | Terrain · Mosaic | CONFIG | F32 | 0.0f...1.0f /1.0f | `GPUDesignConfig::mosaic_enable` | console dial (live) |
| `shard size` | Terrain · Mosaic | CONFIG | F32 | 0.0f...1.2f /0.005f | `GPUDesignConfig::mosaic_shard_size` | console dial (live) |
| `passage scale` | Terrain · Mosaic | CONFIG | F32 | 0.0f...48.0f /0.25f | `GPUDesignConfig::mosaic_passage_scale` | console dial (live) |
| `boundary blend` | Terrain · Mosaic | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::mosaic_blend` | console dial (live) |
| `facet lean` | Terrain · Mosaic | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::mosaic_facet` | console dial (live) |
| `height cap fraction` | Terrain · Indoor | INDOOR | F32 | 0.0f...1.0f /0.005f | `IndoorSurface::height_cap_fraction` | console dial (generational) |
| `ribbon scale` | Terrain · Indoor | INDOOR | F32 | 0.0f...0.6f /0.005f | `IndoorSurface::ribbon_scale` | console dial (generational) |
| `terrain amp ceiling (driven)` | Terrain · Indoor | CONFIG | F32 | - (no range) | `GPUDesignConfig::terrain_amp_ceiling` | WITNESS - console never writes |
| `ceiling height (driven)` | Terrain · Indoor | CONFIG | F32 | - (no range) | `GPUDesignConfig::ceiling_height` | WITNESS - console never writes |
| `GoL lift cap (driven)` | Terrain · Indoor | CONFIG | F32 | - (no range) | `GPUDesignConfig::indoor_height_cap` | WITNESS - console never writes |
| `intent` | Pawn · Aura | DRIVERS | BOOL | 0.0f...1.0f /1.0f | `DriverSurface::aura.intent` | console dial (live) |
| `attack (1/s)` | Pawn · Aura | DRIVERS | F32 | 0.05f...8.0f /0.05f | `DriverSurface::aura.attack` | console dial (live) |
| `release (1/s)` | Pawn · Aura | DRIVERS | F32 | 0.05f...8.0f /0.05f | `DriverSurface::aura.release` | console dial (live) |
| `height gain` | Pawn · Aura | DRIVERS | F32 | 0.0f...2.0f /0.01f | `DriverSurface::aura.height_gain` | console dial (live) |
| `enabled (driven)` | Pawn · Aura | CONFIG | F32 | - (no range) | `GPUDesignConfig::aura_enabled` | WITNESS - console never writes |
| `height (driven)` | Pawn · Aura | CONFIG | F32 | - (no range) | `GPUDesignConfig::pawn_aura_height` | WITNESS - console never writes |
| `influence radius` | Pawn · Aura profile | PAWN | F32 | 0.0f...80.0f /0.4f | `PawnAuraProfile::influence_radius` | console dial (live) |
| `attack stiffness` | Pawn · Aura profile | PAWN | F32 | 0.0f...48.0f /0.25f | `PawnAuraProfile::attack_stiffness` | console dial (live) |
| `attack damping` | Pawn · Aura profile | PAWN | F32 | 0.0f...2.8f /0.01f | `PawnAuraProfile::attack_damping` | console dial (live) |
| `release rate` | Pawn · Aura profile | PAWN | F32 | 0.05f...8.0f /0.05f | `PawnAuraProfile::release_rate` | console dial (live) |
| `tint strength` | Pawn · Aura profile | PAWN | F32 | 0.0f...1.0f /0.005f | `PawnAuraProfile::tint_strength` | console dial (live) |
| `tint colour` | Pawn · Aura profile | PAWN | VEC3 | 0.0f...1.0f /0.01f | `PawnAuraProfile::tint_r` | console dial (live) |
| `delta magnitude` | Pawn · Aura profile | PAWN | F32 | 0.0f...1.2f /0.005f | `PawnAuraProfile::delta_magnitude` | console dial (live) |
| `height scale` | Pawn · Aura profile | PAWN | F32 | 0.0f...12.0f /0.05f | `PawnAuraProfile::height_scale` | console dial (live) |
| `delta mode (0 conv, 1 rand)` | Pawn · Aura profile | PAWN | U32 | 0.0f...1.0f /1.0f | `PawnAuraProfile::delta_mode` | console dial (live) |
| `tilt tau` | Pawn · Figure (driven) | CONFIG | F32 | - (no range) | `GPUDesignConfig::pawn_tilt_tau` | WITNESS - console never writes |
| `body radius` | Pawn · Figure (driven) | CONFIG | F32 | - (no range) | `GPUDesignConfig::pawn_body_radius` | WITNESS - console never writes |
| `eye height` | Pawn · Figure (driven) | CONFIG | F32 | - (no range) | `GPUDesignConfig::fpv_eye_height` | WITNESS - console never writes |
| `yaw rate (rad/s)` | Ribbon · Head | CONFIG | F32 | 0.0f...4.0f /0.02f | `GPUDesignConfig::ribbon_yaw_rate` | console dial (live) |
| `max speed` | Ribbon · Head | CONFIG | F32 | 0.0f...160.0f /0.5f | `GPUDesignConfig::ribbon_max_speed` | console dial (live) |
| `min turn radius` | Ribbon · Head | CONFIG | F32 | 1.0f...160.0f /0.5f | `GPUDesignConfig::ribbon_r_min` | console dial (live) |
| `climb rate` | Ribbon · Head | CONFIG | F32 | 0.0f...60.0f /0.25f | `GPUDesignConfig::ribbon_climb_rate` | console dial (live) |
| `floor margin` | Ribbon · Head | CONFIG | F32 | 0.0f...100.0f /0.5f | `GPUDesignConfig::ribbon_floor_margin` | console dial (live) |
| `altitude smoothing` | Ribbon · Head | CONFIG | F32 | 0.0f...720.0f /2.5f | `GPUDesignConfig::ribbon_alt_smooth_dist` | console dial (live) |
| `altitude stiffness` | Ribbon · Head | CONFIG | F32 | 0.0f...1.44f /0.005f | `GPUDesignConfig::ribbon_alt_stiff` | console dial (live) |
| `mount setback` | Ribbon · Head | CONFIG | F32 | 0.0f...6.0f /0.02f | `GPUDesignConfig::ribbon_mount_setback` | console dial (live) |
| `hands tau (s)` | Ribbon · Head | CONFIG | F32 | 0.0f...2.4f /0.01f | `GPUDesignConfig::ribbon_hands_tau` | console dial (live) |
| `lookahead` | Ribbon · Sky Rule | CONFIG | F32 | 0.0f...400.0f /2.0f | `GPUDesignConfig::ribbon_lookahead` | console dial (live) |
| `head clearance` | Ribbon · Sky Rule | CONFIG | F32 | 0.0f...100.0f /0.5f | `GPUDesignConfig::ribbon_clear_head` | console dial (live) |
| `body clearance` | Ribbon · Sky Rule | CONFIG | F32 | 0.0f...50.0f /0.25f | `GPUDesignConfig::ribbon_clear_body` | console dial (live) |
| `steer softness` | Ribbon · Wander | CONFIG | F32 | 0.05f...2.0f /0.01f | `GPUDesignConfig::ribbon_wander_soft` | console dial (live) |
| `yaw cap` | Ribbon · Wander | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::ribbon_wander_yaw_max` | console dial (live) |
| `arrive radius` | Ribbon · Wander | CONFIG | F32 | 10.0f...400.0f /2.0f | `GPUDesignConfig::ribbon_wander_arrive` | console dial (live) |
| `roam radius` | Ribbon · Wander | CONFIG | F32 | 50.0f...1500.0f /5.0f | `GPUDesignConfig::ribbon_roam_radius` | console dial (live) |
| `reference BPM` | Ribbon · Head | RIBBON | F32 | 40.0f...240.0f /1.0f | `RibbonSurface::reference_bpm` | console dial (live) |
| `board ease (s)` | Ribbon · Mount | RIBBON | F32 | 0.05f...6.0f /0.02f | `RibbonSurface::board_seconds` | console dial (live) |
| `land ease (s)` | Ribbon · Mount | RIBBON | F32 | 0.05f...6.0f /0.02f | `RibbonSurface::land_seconds` | console dial (live) |
| `spawn chance` | Ribbon · Spawn | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::spawn_chance` | console dial (generational) |
| `position jitter` | Ribbon · Spawn | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::position_jitter` | console dial (generational) |
| `wander chance` | Ribbon · Spawn | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::wander_chance` | console dial (generational) |
| `cruise mean (× max speed)` | Ribbon · Spawn | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::wander_cruise_base` | console dial (generational) |
| `cruise sigma` | Ribbon · Spawn | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::wander_cruise_sigma` | console dial (generational) |
| `cruise floor` | Ribbon · Spawn | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::wander_cruise_min` | console dial (generational) |
| `cruise ceiling` | Ribbon · Spawn | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::wander_cruise_max` | console dial (generational) |
| `weight smooth` | Ribbon · Colour | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::color_weights[0]` | console dial (generational) |
| `weight tinted` | Ribbon · Colour | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::color_weights[1]` | console dial (generational) |
| `weight contrast` | Ribbon · Colour | RIBBON_SPAWN | F32 | 0.0f...1.0f /0.005f | `RibbonSpawnSurface::color_weights[2]` | console dial (generational) |
| `smooth 0 sandstone` | Ribbon · Colour | RIBBON_SPAWN | VEC3 | 0.0f...1.0f /0.01f | `RibbonSpawnSurface::smooth_palette[0][0]` | console dial (generational) |
| `smooth 1 sky` | Ribbon · Colour | RIBBON_SPAWN | VEC3 | 0.0f...1.0f /0.01f | `RibbonSpawnSurface::smooth_palette[1][0]` | console dial (generational) |
| `smooth 2 golden` | Ribbon · Colour | RIBBON_SPAWN | VEC3 | 0.0f...1.0f /0.01f | `RibbonSpawnSurface::smooth_palette[2][0]` | console dial (generational) |
| `smooth 3 sage` | Ribbon · Colour | RIBBON_SPAWN | VEC3 | 0.0f...1.0f /0.01f | `RibbonSpawnSurface::smooth_palette[3][0]` | console dial (generational) |
| `smooth var range` | Ribbon · Colour | RIBBON_SPAWN | F32 | 0.0f...0.4f /0.005f | `RibbonSpawnSurface::smooth_var_range` | console dial (generational) |
| `smooth var bias` | Ribbon · Colour | RIBBON_SPAWN | F32 | -0.2f...0.2f /0.005f | `RibbonSpawnSurface::smooth_var_bias` | console dial (generational) |
| `smooth var × green` | Ribbon · Colour | RIBBON_SPAWN | F32 | 0.0f...2.0f /0.01f | `RibbonSpawnSurface::smooth_var_g_scale` | console dial (generational) |
| `smooth var × blue` | Ribbon · Colour | RIBBON_SPAWN | F32 | 0.0f...2.0f /0.01f | `RibbonSpawnSurface::smooth_var_b_scale` | console dial (generational) |
| `tinted range` | Ribbon · Colour | RIBBON_SPAWN | VEC3 | 0.0f...1.0f /0.01f | `RibbonSpawnSurface::tinted_range[0]` | console dial (generational) |
| `tinted base` | Ribbon · Colour | RIBBON_SPAWN | VEC3 | 0.0f...1.0f /0.01f | `RibbonSpawnSurface::tinted_base[0]` | console dial (generational) |
| `colour` | Agents · Tier 0 | AGENT_ROOM | VEC3 | 0.0f...1.0f /0.01f | `GPUAgentRoomConstants::tier_gains[0].color_r -> TIER AgentTierBank::t[0].color_r` | console dial + definition |
| `speed gain` | Agents · Tier 0 | AGENT_ROOM | F32 | 0.0f...4.0f /0.01f | `GPUAgentRoomConstants::tier_gains[0].speed_gain -> TIER AgentTierBank::t[0].speed_gain` | console dial + definition |
| `colour` | Agents · Tier 1 | AGENT_ROOM | VEC3 | 0.0f...1.0f /0.01f | `GPUAgentRoomConstants::tier_gains[1].color_r -> TIER AgentTierBank::t[1].color_r` | console dial + definition |
| `speed gain` | Agents · Tier 1 | AGENT_ROOM | F32 | 0.0f...4.0f /0.01f | `GPUAgentRoomConstants::tier_gains[1].speed_gain -> TIER AgentTierBank::t[1].speed_gain` | console dial + definition |
| `colour` | Agents · Tier 2 | AGENT_ROOM | VEC3 | 0.0f...1.0f /0.01f | `GPUAgentRoomConstants::tier_gains[2].color_r -> TIER AgentTierBank::t[2].color_r` | console dial + definition |
| `speed gain` | Agents · Tier 2 | AGENT_ROOM | F32 | 0.0f...4.0f /0.01f | `GPUAgentRoomConstants::tier_gains[2].speed_gain -> TIER AgentTierBank::t[2].speed_gain` | console dial + definition |
| `colour` | Agents · Tier 3 | AGENT_ROOM | VEC3 | 0.0f...1.0f /0.01f | `GPUAgentRoomConstants::tier_gains[3].color_r -> TIER AgentTierBank::t[3].color_r` | console dial + definition |
| `speed gain` | Agents · Tier 3 | AGENT_ROOM | F32 | 0.0f...4.0f /0.01f | `GPUAgentRoomConstants::tier_gains[3].speed_gain -> TIER AgentTierBank::t[3].speed_gain` | console dial + definition |
| `step gain` | Agents · Tier 0 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[0].step_gain -> TIER AgentTierBank::t[0].step_gain` | console dial + definition |
| `persist gain` | Agents · Tier 0 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[0].persist_gain -> TIER AgentTierBank::t[0].persist_gain` | console dial + definition |
| `contact radius` | Agents · Tier 0 | AGENT_ROOM | F32 | 0.0f...8.0f /0.04f | `GPUAgentRoomConstants::tier_gains[0].contact_radius -> TIER AgentTierBank::t[0].contact_radius` | console dial + definition |
| `contact mass` | Agents · Tier 0 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[0].contact_mass -> TIER AgentTierBank::t[0].contact_mass` | console dial + definition |
| `personal radius` | Agents · Tier 0 | AGENT_ROOM | F32 | 0.0f...120.0f /0.5f | `GPUAgentRoomConstants::tier_gains[0].personal_radius -> TIER AgentTierBank::t[0].personal_radius` | console dial + definition |
| `flee gain` | Agents · Tier 0 | AGENT_ROOM | F32 | 0.0f...0.99f /0.005f | `GPUAgentRoomConstants::tier_gains[0].flee_gain_player -> TIER AgentTierBank::t[0].flee_gain_player` | console dial + definition |
| `step gain` | Agents · Tier 1 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[1].step_gain -> TIER AgentTierBank::t[1].step_gain` | console dial + definition |
| `persist gain` | Agents · Tier 1 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[1].persist_gain -> TIER AgentTierBank::t[1].persist_gain` | console dial + definition |
| `contact radius` | Agents · Tier 1 | AGENT_ROOM | F32 | 0.0f...8.0f /0.04f | `GPUAgentRoomConstants::tier_gains[1].contact_radius -> TIER AgentTierBank::t[1].contact_radius` | console dial + definition |
| `contact mass` | Agents · Tier 1 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[1].contact_mass -> TIER AgentTierBank::t[1].contact_mass` | console dial + definition |
| `personal radius` | Agents · Tier 1 | AGENT_ROOM | F32 | 0.0f...120.0f /0.5f | `GPUAgentRoomConstants::tier_gains[1].personal_radius -> TIER AgentTierBank::t[1].personal_radius` | console dial + definition |
| `flee gain` | Agents · Tier 1 | AGENT_ROOM | F32 | 0.0f...0.99f /0.005f | `GPUAgentRoomConstants::tier_gains[1].flee_gain_player -> TIER AgentTierBank::t[1].flee_gain_player` | console dial + definition |
| `step gain` | Agents · Tier 2 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[2].step_gain -> TIER AgentTierBank::t[2].step_gain` | console dial + definition |
| `persist gain` | Agents · Tier 2 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[2].persist_gain -> TIER AgentTierBank::t[2].persist_gain` | console dial + definition |
| `contact radius` | Agents · Tier 2 | AGENT_ROOM | F32 | 0.0f...8.0f /0.04f | `GPUAgentRoomConstants::tier_gains[2].contact_radius -> TIER AgentTierBank::t[2].contact_radius` | console dial + definition |
| `contact mass` | Agents · Tier 2 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[2].contact_mass -> TIER AgentTierBank::t[2].contact_mass` | console dial + definition |
| `personal radius` | Agents · Tier 2 | AGENT_ROOM | F32 | 0.0f...120.0f /0.5f | `GPUAgentRoomConstants::tier_gains[2].personal_radius -> TIER AgentTierBank::t[2].personal_radius` | console dial + definition |
| `flee gain` | Agents · Tier 2 | AGENT_ROOM | F32 | 0.0f...0.99f /0.005f | `GPUAgentRoomConstants::tier_gains[2].flee_gain_player -> TIER AgentTierBank::t[2].flee_gain_player` | console dial + definition |
| `step gain` | Agents · Tier 3 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[3].step_gain -> TIER AgentTierBank::t[3].step_gain` | console dial + definition |
| `persist gain` | Agents · Tier 3 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[3].persist_gain -> TIER AgentTierBank::t[3].persist_gain` | console dial + definition |
| `contact radius` | Agents · Tier 3 | AGENT_ROOM | F32 | 0.0f...8.0f /0.04f | `GPUAgentRoomConstants::tier_gains[3].contact_radius -> TIER AgentTierBank::t[3].contact_radius` | console dial + definition |
| `contact mass` | Agents · Tier 3 | AGENT_ROOM | F32 | 0.0f...4.0f /0.02f | `GPUAgentRoomConstants::tier_gains[3].contact_mass -> TIER AgentTierBank::t[3].contact_mass` | console dial + definition |
| `personal radius` | Agents · Tier 3 | AGENT_ROOM | F32 | 0.0f...120.0f /0.5f | `GPUAgentRoomConstants::tier_gains[3].personal_radius -> TIER AgentTierBank::t[3].personal_radius` | console dial + definition |
| `flee gain` | Agents · Tier 3 | AGENT_ROOM | F32 | 0.0f...0.99f /0.005f | `GPUAgentRoomConstants::tier_gains[3].flee_gain_player -> TIER AgentTierBank::t[3].flee_gain_player` | console dial + definition |
| `step rate` | Agents · player_controlled | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[0].step_rate -> BEHAVIOR AgentBehaviorBank::b[0].step_rate` | console dial + definition |
| `step size` | Agents · player_controlled | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[0].step_size -> BEHAVIOR AgentBehaviorBank::b[0].step_size` | console dial + definition |
| `persistence` | Agents · player_controlled | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[0].persistence -> BEHAVIOR AgentBehaviorBank::b[0].persistence` | console dial + definition |
| `drag` | Agents · player_controlled | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[0].drag -> BEHAVIOR AgentBehaviorBank::b[0].drag` | console dial + definition |
| `home pull` | Agents · player_controlled | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[0].home_pull -> BEHAVIOR AgentBehaviorBank::b[0].home_pull` | console dial + definition |
| `neighbour radius` | Agents · player_controlled | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[0].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[0].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · player_controlled | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[0].speed_cap -> BEHAVIOR AgentBehaviorBank::b[0].speed_cap` | console dial + definition |
| `step rate` | Agents · random_walk | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[1].step_rate -> BEHAVIOR AgentBehaviorBank::b[1].step_rate` | console dial + definition |
| `step size` | Agents · random_walk | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[1].step_size -> BEHAVIOR AgentBehaviorBank::b[1].step_size` | console dial + definition |
| `persistence` | Agents · random_walk | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[1].persistence -> BEHAVIOR AgentBehaviorBank::b[1].persistence` | console dial + definition |
| `drag` | Agents · random_walk | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[1].drag -> BEHAVIOR AgentBehaviorBank::b[1].drag` | console dial + definition |
| `home pull` | Agents · random_walk | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[1].home_pull -> BEHAVIOR AgentBehaviorBank::b[1].home_pull` | console dial + definition |
| `neighbour radius` | Agents · random_walk | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[1].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[1].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · random_walk | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[1].speed_cap -> BEHAVIOR AgentBehaviorBank::b[1].speed_cap` | console dial + definition |
| `step rate` | Agents · biased_walk | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[2].step_rate -> BEHAVIOR AgentBehaviorBank::b[2].step_rate` | console dial + definition |
| `step size` | Agents · biased_walk | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[2].step_size -> BEHAVIOR AgentBehaviorBank::b[2].step_size` | console dial + definition |
| `persistence` | Agents · biased_walk | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[2].persistence -> BEHAVIOR AgentBehaviorBank::b[2].persistence` | console dial + definition |
| `drag` | Agents · biased_walk | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[2].drag -> BEHAVIOR AgentBehaviorBank::b[2].drag` | console dial + definition |
| `home pull` | Agents · biased_walk | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[2].home_pull -> BEHAVIOR AgentBehaviorBank::b[2].home_pull` | console dial + definition |
| `neighbour radius` | Agents · biased_walk | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[2].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[2].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · biased_walk | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[2].speed_cap -> BEHAVIOR AgentBehaviorBank::b[2].speed_cap` | console dial + definition |
| `step rate` | Agents · wanderer | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[3].step_rate -> BEHAVIOR AgentBehaviorBank::b[3].step_rate` | console dial + definition |
| `step size` | Agents · wanderer | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[3].step_size -> BEHAVIOR AgentBehaviorBank::b[3].step_size` | console dial + definition |
| `persistence` | Agents · wanderer | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[3].persistence -> BEHAVIOR AgentBehaviorBank::b[3].persistence` | console dial + definition |
| `drag` | Agents · wanderer | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[3].drag -> BEHAVIOR AgentBehaviorBank::b[3].drag` | console dial + definition |
| `home pull` | Agents · wanderer | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[3].home_pull -> BEHAVIOR AgentBehaviorBank::b[3].home_pull` | console dial + definition |
| `neighbour radius` | Agents · wanderer | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[3].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[3].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · wanderer | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[3].speed_cap -> BEHAVIOR AgentBehaviorBank::b[3].speed_cap` | console dial + definition |
| `step rate` | Agents · home_seeker | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[4].step_rate -> BEHAVIOR AgentBehaviorBank::b[4].step_rate` | console dial + definition |
| `step size` | Agents · home_seeker | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[4].step_size -> BEHAVIOR AgentBehaviorBank::b[4].step_size` | console dial + definition |
| `persistence` | Agents · home_seeker | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[4].persistence -> BEHAVIOR AgentBehaviorBank::b[4].persistence` | console dial + definition |
| `drag` | Agents · home_seeker | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[4].drag -> BEHAVIOR AgentBehaviorBank::b[4].drag` | console dial + definition |
| `home pull` | Agents · home_seeker | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[4].home_pull -> BEHAVIOR AgentBehaviorBank::b[4].home_pull` | console dial + definition |
| `neighbour radius` | Agents · home_seeker | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[4].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[4].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · home_seeker | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[4].speed_cap -> BEHAVIOR AgentBehaviorBank::b[4].speed_cap` | console dial + definition |
| `step rate` | Agents · slow_patrol | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[5].step_rate -> BEHAVIOR AgentBehaviorBank::b[5].step_rate` | console dial + definition |
| `step size` | Agents · slow_patrol | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[5].step_size -> BEHAVIOR AgentBehaviorBank::b[5].step_size` | console dial + definition |
| `persistence` | Agents · slow_patrol | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[5].persistence -> BEHAVIOR AgentBehaviorBank::b[5].persistence` | console dial + definition |
| `drag` | Agents · slow_patrol | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[5].drag -> BEHAVIOR AgentBehaviorBank::b[5].drag` | console dial + definition |
| `home pull` | Agents · slow_patrol | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[5].home_pull -> BEHAVIOR AgentBehaviorBank::b[5].home_pull` | console dial + definition |
| `neighbour radius` | Agents · slow_patrol | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[5].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[5].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · slow_patrol | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[5].speed_cap -> BEHAVIOR AgentBehaviorBank::b[5].speed_cap` | console dial + definition |
| `step rate` | Agents · pursuit | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[6].step_rate -> BEHAVIOR AgentBehaviorBank::b[6].step_rate` | console dial + definition |
| `step size` | Agents · pursuit | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[6].step_size -> BEHAVIOR AgentBehaviorBank::b[6].step_size` | console dial + definition |
| `persistence` | Agents · pursuit | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[6].persistence -> BEHAVIOR AgentBehaviorBank::b[6].persistence` | console dial + definition |
| `drag` | Agents · pursuit | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[6].drag -> BEHAVIOR AgentBehaviorBank::b[6].drag` | console dial + definition |
| `home pull` | Agents · pursuit | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[6].home_pull -> BEHAVIOR AgentBehaviorBank::b[6].home_pull` | console dial + definition |
| `neighbour radius` | Agents · pursuit | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[6].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[6].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · pursuit | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[6].speed_cap -> BEHAVIOR AgentBehaviorBank::b[6].speed_cap` | console dial + definition |
| `step rate` | Agents · flee | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[7].step_rate -> BEHAVIOR AgentBehaviorBank::b[7].step_rate` | console dial + definition |
| `step size` | Agents · flee | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[7].step_size -> BEHAVIOR AgentBehaviorBank::b[7].step_size` | console dial + definition |
| `persistence` | Agents · flee | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[7].persistence -> BEHAVIOR AgentBehaviorBank::b[7].persistence` | console dial + definition |
| `drag` | Agents · flee | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[7].drag -> BEHAVIOR AgentBehaviorBank::b[7].drag` | console dial + definition |
| `home pull` | Agents · flee | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[7].home_pull -> BEHAVIOR AgentBehaviorBank::b[7].home_pull` | console dial + definition |
| `neighbour radius` | Agents · flee | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[7].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[7].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · flee | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[7].speed_cap -> BEHAVIOR AgentBehaviorBank::b[7].speed_cap` | console dial + definition |
| `step rate` | Agents · flock2d | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[8].step_rate -> BEHAVIOR AgentBehaviorBank::b[8].step_rate` | console dial + definition |
| `step size` | Agents · flock2d | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[8].step_size -> BEHAVIOR AgentBehaviorBank::b[8].step_size` | console dial + definition |
| `persistence` | Agents · flock2d | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[8].persistence -> BEHAVIOR AgentBehaviorBank::b[8].persistence` | console dial + definition |
| `drag` | Agents · flock2d | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[8].drag -> BEHAVIOR AgentBehaviorBank::b[8].drag` | console dial + definition |
| `home pull` | Agents · flock2d | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[8].home_pull -> BEHAVIOR AgentBehaviorBank::b[8].home_pull` | console dial + definition |
| `neighbour radius` | Agents · flock2d | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[8].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[8].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · flock2d | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[8].speed_cap -> BEHAVIOR AgentBehaviorBank::b[8].speed_cap` | console dial + definition |
| `step rate` | Agents · levy_flight | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[9].step_rate -> BEHAVIOR AgentBehaviorBank::b[9].step_rate` | console dial + definition |
| `step size` | Agents · levy_flight | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[9].step_size -> BEHAVIOR AgentBehaviorBank::b[9].step_size` | console dial + definition |
| `persistence` | Agents · levy_flight | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[9].persistence -> BEHAVIOR AgentBehaviorBank::b[9].persistence` | console dial + definition |
| `drag` | Agents · levy_flight | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[9].drag -> BEHAVIOR AgentBehaviorBank::b[9].drag` | console dial + definition |
| `home pull` | Agents · levy_flight | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[9].home_pull -> BEHAVIOR AgentBehaviorBank::b[9].home_pull` | console dial + definition |
| `neighbour radius` | Agents · levy_flight | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[9].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[9].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · levy_flight | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[9].speed_cap -> BEHAVIOR AgentBehaviorBank::b[9].speed_cap` | console dial + definition |
| `step rate` | Agents · passer | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[10].step_rate -> BEHAVIOR AgentBehaviorBank::b[10].step_rate` | console dial + definition |
| `step size (waypoint radius)` | Agents · passer | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[10].step_size -> BEHAVIOR AgentBehaviorBank::b[10].step_size` | console dial + definition |
| `persistence` | Agents · passer | AGENT_ROOM | F32 | 0.0f...1.0f /0.005f | `GPUAgentRoomConstants::behaviors[10].persistence -> BEHAVIOR AgentBehaviorBank::b[10].persistence` | console dial + definition |
| `drag` | Agents · passer | AGENT_ROOM | F32 | 0.0f...3.75f /0.02f | `GPUAgentRoomConstants::behaviors[10].drag -> BEHAVIOR AgentBehaviorBank::b[10].drag` | console dial + definition |
| `home pull` | Agents · passer | AGENT_ROOM | F32 | 0.0f...10.0f /0.05f | `GPUAgentRoomConstants::behaviors[10].home_pull -> BEHAVIOR AgentBehaviorBank::b[10].home_pull` | console dial + definition |
| `neighbour radius` | Agents · passer | AGENT_ROOM | F32 | 0.0f...50.0f /0.25f | `GPUAgentRoomConstants::behaviors[10].neighbor_radius -> BEHAVIOR AgentBehaviorBank::b[10].neighbor_radius` | console dial + definition |
| `speed cap` | Agents · passer | AGENT_ROOM | F32 | 0.0f...30.0f /0.25f | `GPUAgentRoomConstants::behaviors[10].speed_cap -> BEHAVIOR AgentBehaviorBank::b[10].speed_cap` | console dial + definition |
| `band` | Agents · passer | AGENT_ROOM | F32 | 0.0f...20.0f /0.25f | `GPUAgentRoomConstants::behaviors[10].aux -> BEHAVIOR AgentBehaviorBank::b[10].aux` | console dial + definition |
| `portal density` | Agents · Portals | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::portal_density` | console dial (generational) |
| `draw · open sunset` | Agents · Portals | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::mood_weights[0]` | console dial (generational) |
| `draw · indoor flat` | Agents · Portals | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::mood_weights[1]` | console dial (generational) |
| `draw · indoor vault` | Agents · Portals | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::mood_weights[2]` | console dial (generational) |
| `draw · finite outdoor` | Agents · Portals | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::mood_weights[3]` | console dial (generational) |
| `draw · open night` | Agents · Portals | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::mood_weights[4]` | console dial (generational) |
| `draw · open noon` | Agents · Portals | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::mood_weights[5]` | console dial (generational) |
| `draw · atrium` | Agents · Portals | WORLD | F32 | 0.0f...1.0f /0.005f | `WorldDrawSurface::mood_weights[6]` | console dial (generational) |
| `slack` | Interaction · Field | CONFIG | F32 | 0.0f...12.0f /0.05f | `GPUDesignConfig::field_slack` | console dial (live) |
| `arch slack` | Interaction · Field | CONFIG | F32 | 0.5f...4.0f /0.05f | `GPUDesignConfig::field_arch_slack` | console dial (live) |
| `k (accel)` | Interaction · Field | CONFIG | F32 | 0.0f...1200.0f /5.0f | `GPUDesignConfig::field_k` | console dial (live) |
| `f max` | Interaction · Field | CONFIG | F32 | 0.0f...2400.0f /10.0f | `GPUDesignConfig::field_fmax` | console dial (live) |
| `occupier gain` | Interaction · Field | CONFIG | F32 | 0.0f...4.0f /0.02f | `GPUDesignConfig::field_occupier_gain` | console dial (live) |
| `authored gain` | Interaction · Field | CONFIG | F32 | 0.0f...4.0f /0.02f | `GPUDesignConfig::field_authored_gain` | console dial (live) |
| `cube gain` | Interaction · Field | CONFIG | F32 | 0.0f...16.0f /0.08f | `GPUDesignConfig::field_gain_cube` | console dial (live) |
| `sphere gain` | Interaction · Field | CONFIG | F32 | 0.0f...4.0f /0.02f | `GPUDesignConfig::field_gain_sphere` | console dial (live) |
| `agent gain` | Interaction · Field | CONFIG | F32 | 0.0f...16.0f /0.08f | `GPUDesignConfig::field_gain_agent` | console dial (live) |
| `bubble radius` | Interaction · Point | CONFIG | F32 | 0.0f...80.0f /0.5f | `GPUDesignConfig::point_bubble_radius` | console dial (live) |
| `walk speed` | Interaction · Pawn | CONFIG | F32 | 0.0f...60.0f /0.5f | `GPUDesignConfig::pawn_speed` | console dial (live) |
| `fly speed` | Interaction · Camera | CONFIG | F32 | 0.0f...120.0f /0.5f | `GPUDesignConfig::point_fly_speed` | console dial (live) |
| `plasticity λ` | Interaction · Cubes | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::cube_plasticity` | console dial (live) |
| `floater coordination` | Interaction · Cubes | CONFIG | F32 | 0.0f...1.0f /0.005f | `GPUDesignConfig::floater_coordination` | console dial (live) |
| `inner radius` | Interaction · Beacon | PANEL | F32 | 0.0f...100.0f /0.5f | `PanelSurface::beacon.r0` | console dial (live) |
| `outer radius` | Interaction · Beacon | PANEL | F32 | 0.0f...480.0f /2.0f | `PanelSurface::beacon.r` | console dial (live) |
| `pull strength` | Interaction · Beacon | PANEL | F32 | 0.0f...the_board::FIELD_BEACON_S_MAX /1.0f | `PanelSurface::beacon.s` | console dial (live) |
| `lift` | Interaction · Beacon | PANEL | F32 | 0.0f...80.0f /0.4f | `PanelSurface::beacon.lift` | console dial (live) |
| `look sens (anchor)` | Interaction · Camera | PANEL | F32 | 0.000625f...0.04f /0.0002f | `PanelSurface::camera.look_sens_init` | console dial (live) |
| `look sens step` | Interaction · Camera | PANEL | F32 | 1.01f...2.0f /0.005f | `PanelSurface::camera.look_sens_step` | console dial (live) |
| `look sens range` | Interaction · Camera | PANEL | F32 | 1.0f...32.0f /0.25f | `PanelSurface::camera.look_sens_range` | console dial (live) |
| `scroll zoom` | Interaction · Camera | PANEL | F32 | 0.0f...8.0f /0.05f | `PanelSurface::camera.scroll_zoom_scale` | console dial (live) |
| `reach` | Interaction · Possession | PANEL | F32 | 0.5f...80.0f /0.5f | `PanelSurface::possession.radius` | console dial (live) |
| `feed-forward` | Camera · Chase | CONFIG | F32 | 0.0f...1.0f /0.01f | `GPUDesignConfig::camera_chase_ff` | console dial (live) |
| `push gain` | Camera · Presence | CONFIG | F32 | 0.0f...40.0f /0.25f | `GPUDesignConfig::camera_push_gain` | console dial (live) |
| `push radius` | Camera · Presence | CONFIG | F32 | 0.0f...80.0f /0.5f | `GPUDesignConfig::camera_push_radius` | console dial (live) |
| `mute dynamics (0D)` | Debug | CONFIG | BOOL | 0.0f...1.0f /1.0f | `GPUDesignConfig::mute_dynamics_0d` | console dial (live) |
| `mute signal` | Debug | CONFIG | BOOL | 0.0f...1.0f /1.0f | `GPUDesignConfig::mute_signal` | console dial (live) |
| `freeze sphere` | Debug | CONFIG | BOOL | 0.0f...1.0f /1.0f | `GPUDesignConfig::freeze_sphere` | console dial (live) |
| `FPV mode (key-shared)` | Debug | CONFIG | BOOL | 0.0f...1.0f /1.0f | `GPUDesignConfig::fpv_mode` | console dial (live) |
| `main draw mask (bits: A/B/C terrain, table, ribbon, [5 unused], orbs, fade)` | Measure | CONFIG | U32 | 0.0f...255.0f /1.0f | `GPUDesignConfig::draw_mask` | console dial (live) |
| `shadow draw mask (bit0 terrain, bit1 entities+artworks)` | Measure | CONFIG | U32 | 0.0f...3.0f /1.0f | `GPUDesignConfig::shadow_mask` | console dial (live) |
| `sun PCF taps (4 or 16)` | Measure | CONFIG | U32 | 4.0f...16.0f /12.0f | `GPUDesignConfig::shadow_pcf_taps` | console dial (live) |

### 3.9 The 15 dormant `ORGAN_PARAM_RO` witnesses and their real writers

The `WITNESS` classification above says only that the console never writes these.
Each one's actual runtime writer, found with
`git grep -n "config_\.<field>\|set_.*<field>" -- src/`:

| witness row | writer today |
| --- | --- |
| `GPULighting::sun.direction` (drawn) | `apply_mood_lighting` (`direction/mood.hpp`) — draws from the live regime's row under the world seed, at every mood entry and every MOOD definition re-speak. Authored by the three `Sky & Light · Sun` DEFONLY rows. |
| `GPULighting::sun.color` (drawn) | `apply_mood_lighting`, from the regime (`Atmosphere · Regime N` DEFONLY rows). |
| `GPULighting::sun.intensity` (drawn) | `apply_mood_lighting`, from the regime. |
| `GPULighting::sun.ambient` (drawn) | `apply_mood_lighting`, from the regime. |
| `GPUDesignConfig::veil_strength` | `Cartridge` (`gpuState_.set_veil_strength(world_state_.finite_mode ? 0.0f : 1.0f)`) → `GPUState::set_veil_strength`. A world-mode boolean, not a coupling. |
| `GPUDesignConfig::fade_alpha` | `Cartridge` (`gpuState_.set_fade(mood_state_.transition_fade_alpha, 0,0,0)`) → `GPUState::set_fade`. Mood transition state. |
| `GPUDesignConfig::fade_color` | same `GPUState::set_fade` call; the three colour args are literal `0.0f` at the one call site. |
| `GPUDesignConfig::terrain_amp_ceiling` | `direction/mood.hpp` (`set_terrain_amp_ceiling(m.shape.terrain_amp_ceiling)`) — a mood constant. |
| `GPUDesignConfig::ceiling_height` | `direction/mood.hpp` (`set_ceiling_height(effective_ceiling)` indoors, `set_ceiling_height(0.0f)` outdoors). |
| `GPUDesignConfig::indoor_height_cap` | `direction/mood.hpp` (`set_indoor_height_cap(...)`), from `INDOOR_HEIGHT_CAP_FRACTION`. |
| `GPUDesignConfig::aura_enabled` | `bodies/pawn.hpp` (`set_aura_enabled(c->player_.aura_presence > 0.001f)`) — the presence ramp driven by `DRIVER_LIVE.aura` (Group A's four dormant rows). |
| `GPUDesignConfig::pawn_aura_height` | `bodies/pawn.hpp` (`set_pawn_aura_height(effective_aura_height)`), scaled by `DRIVER_LIVE.aura.height_gain`. |
| `GPUDesignConfig::pawn_tilt_tau` | `GPUState::set_pawn_tilt_tau`, called per frame from the cartridge on the possessed figure. |
| `GPUDesignConfig::pawn_body_radius` | `GPUState::set_pawn_body_radius` (`realization/state.hpp`), called from `t7::the_board::Cartridge` at its one call site (the statement containing `gpuState_.set_pawn_body_radius(`). |
| `GPUDesignConfig::fpv_eye_height` | `Cartridge` writes `gpuState_.config().fpv_eye_height = …` directly (two sites), bypassing any setter. |

### 3.10 Totals, with recipes

| quantity | value | recipe |
| --- | --- | --- |
| pipes declared | **8** | `sed -n '/inline constexpr ParamSlot PARAM_LAYOUT\[\] = {/,/^    };/p' src/coupling/visual_canvas.hpp \| grep -c '^        { "'` |
| bank slots claimed by pipes | **15** of 256 (bases 0..14) | sum of the `count` column of the same eight rows; `VISUAL_PARAM_SLOTS` from `src/coupling/visual_params.hpp` |
| pipes with a bound source at runtime | **0** | `t7::BeatClock::stat_layout()` returns `StatLayoutView{nullptr,0}`; `SignalLayout::resolve` loop bound is `view_.count`. This — and **not** a failure at `PARAM_LAYOUT` — is what every `DECLARED-UNRESOLVED` verdict in §3.4 records; read that column with the FLAG that sits above and below the table |
| pipes with a bound TARGET at runtime | **8** of 8 | both target resolves are `t7::ParamLayout::resolve` over `ParamLayoutView{PARAM_LAYOUT, PARAM_LAYOUT_COUNT}` (count 8), one in `VisualCanvas::bind` and one in `Cartridge::bind_signal_layout`; §3.5 exemplar 1 shows the nine-hit name census for `fog.density` |
| source names resolved and missed per boot | **12** | 5 named resolves in `VisualCanvas::bind` + `popcount(ZOETROPE_EARS = 0b0111'1111u) = 7` |
| dormant rows — `driver_surface.hpp` | **4** of 14 members | `grep -nE "^\s+(float\|uint32_t)\s+[a-z_0-9]+(\[3\])?;" …/driver_surface.hpp` → 14, minus the 10 pipe-named members of §3.8 recipe (b). **Recipe corrected**: the single-space form `(float\|uint32_t) [a-z_0-9]+` returns only 10, missing the padded-column declarations `float    gain;`, `uint32_t intent;`, `float    attack;`, `float    release;`, `float    height_gain;`. The count 14 is confirmed independently by `static_assert(sizeof(DriverSurface) == 18 * sizeof(float))` — 18 words over 14 members |
| dormant rows — `control_panel.hpp` | **25** (16 free `inline constexpr float` + 9 `PanelSurface` members) | `grep -nE "^inline constexpr" …/control_panel.hpp` → **17** lines = 16 scalars + the one aggregate `inline constexpr PanelSurface PANEL_TABLE = {`; **plus** `grep -nE "^\s+float [a-z_0-9]+;" …/control_panel.hpp` → 9. **Wording corrected**: `PANEL_LIVE` is declared `inline PanelSurface PANEL_LIVE = PANEL_TABLE;` — not `constexpr`, so the recipe never matched it and there is one aggregate in the output, not two |
| dormant rows — `organ_params.inc` | **351** | `grep -c "^ORGAN_PARAM" src/console/organ_params.inc` = 381, minus the 30-row named-by-a-pipe set enumerated in §3.8 Group C |
| **dormant rows, all three files** | **380** | 4 + 25 + 351 |

### 3.11 Gaps and flags

**FLAG — verdict vocabulary, carried forward with the column wherever it is
quoted.** Recorded in full under §3.4 and restated at the head of the table there.
All eight rows carry `DECLARED-UNRESOLVED` because the SOURCE half of
`VisualCanvas::bind` fails; no row's TARGET half fails. Taken at the token's
literal definition — "pipe declared, never resolved at bind" — the assignment
reads backwards, because every pipe IS resolved at bind, twice, successfully:
`param_layout_.resolve(...)` in `t7::VisualCanvas::bind` against
`ParamLayoutView{PARAM_LAYOUT, PARAM_LAYOUT_COUNT}`, then
`visual_canvas_.layout().resolve(...)` in
`t7::the_board::Cartridge::bind_signal_layout`. What fails is
`t7::SignalLayout::resolve` against `t7::BeatClock::stat_layout()` =
`StatLayoutView{ nullptr, 0 }`, whose loop bound `view_.count` is 0, so the body
never runs and `++misses_; return SourceBinding{};` is the only path. A reader
taking the verdict column at face value would conclude the pipes fail to resolve
at bind; that conclusion is false. The token this state would want, had the unit's
vocabulary carried one, is `SOURCE-UNBOUND`. The classes `LIVE`,
`DECLARED-UNFLUSHED`, `ORPHAN-SINK`, `ORPHAN-SOURCE` and `GONE` produce zero rows
in this table, and §3.5 shows the command that rules each out.

**GAP — `src/coupling/organ_registry.hpp` is on no include path.** Two registries
exist over the same enrollment list. The full, **unfiltered** output first (an earlier form of this block showed 2 of
the command's 14 returned lines without saying it was filtered — corrected here):

```
git grep -n "organ_params.inc" -- src/ tools/
#   src/cartridges/the_board/contracts/orb_surface.hpp:164:    // cannot dial a sentinel back — organ_params.inc floors these five one
#   src/console/organ_registry.hpp:253:#include "console/organ_params.inc"
#   src/console/organ_registry.hpp:886:// organ_params.inc and no JS edit — but that road carries F32/U32/BOOL
#   src/coupling/organ_registry.hpp:261:#include "console/organ_params.inc"
#   src/coupling/organ_registry.hpp:904:// organ_params.inc — but that road carries F32/U32/BOOL LANES, and a
#   tools/organ_gap.py:6:# list (src/console/organ_params.inc) for every home struct the macro
#   tools/organ_gap.py:56:INC  = os.path.join(ROOT, "src", "console", "organ_params.inc")
#   tools/organ_ledger.py:40:INC = os.path.join(ROOT, "src", "console", "organ_params.inc")
#   tools/organ_ledger.py:179:    A("<!-- GENERATED by tools/organ_ledger.py from src/console/organ_params.inc")
#   tools/organ_parse.py:2:# The enrollment grammar of src/console/organ_params.inc, parsed once.
#   tools/organ_readers.py:67:INC = os.path.join(ROOT, "src", "console", "organ_params.inc")
#   tools/organ_readers.py:303:            if rel.endswith("organ_registry.hpp") or rel.endswith("organ_params.inc"):
#   tools/organ_readers.py:448:                  % (fam, "organ_params.inc", r["line"], r["field"]))
#   tools/organ_readers.py:491:        skip = {"src/console/organ_params.inc"}
```
14 lines: **two** `#include` sites (the two registries), three further comment
hits inside `orb_surface.hpp` and the two registries, and nine `tools/` hits in
the four organ tools. Narrowed to the `#include` sites and to who includes a
registry at all:

```
git grep -n '#include "console/organ_params.inc"' -- src/
#   src/console/organ_registry.hpp:253:#include "console/organ_params.inc"
#   src/coupling/organ_registry.hpp:261:#include "console/organ_params.inc"
git grep -n 'include.*organ_registry.hpp' -- src/
#   src/cartridges/the_board/cartridge.hpp:67:#include "console/organ_registry.hpp"   // the compiled dial registry + its C ABI (needs the home types above)
```
→ exactly one line on the second command. Line counts of the two registries,
verified at HEAD:
```
git cat-file -p HEAD:src/console/organ_registry.hpp  | wc -l   # 974
git cat-file -p HEAD:src/coupling/organ_registry.hpp | wc -l   # 999
```
Only the `console/` copy (blob `70d09e96…`, 974 lines) is compiled. The
`coupling/` copy (blob `3047070e…`, 999 lines) is included by nothing; its diff
against the console copy is comment-level plus the extra WEB_SUNSET banner
("the browser panel that drove it is attic'd at tag web-sunset"). Recorded, not
judged.

**GAP — `src/analysis/canvas_1/canvas.hpp` is on no include path.** It carries a
real `StatLayoutView stat_layout() const override` and the `chN` name tables that
`RIBBON_VOICE`/`CHECKER_VOICE` were verified against, but
`git grep -n '#include "analysis/canvas_1/canvas.hpp"' -- src/` returns nothing;
only the five sibling `check_*.cpp` / `probe_canvas.cpp` binaries reach it by
relative include. `src/the_board.cpp` declares `t7::BeatClock clock;` and passes
`app->clock.stat_layout()` to `bind_signal_layout`.

**GAP — `ATRIUM_DOOR_CHANNEL_MIN` has no transport.** Alone among
`control_panel.hpp`'s sixteen free scalars it has neither an organ enrollment nor
a `GPUDesignConfig` twin (`grep -n ATRIUM_DOOR_CHANNEL_MIN src/console/organ_params.inc`
→ no match). It is a gate-read threshold only.

**GAP — `ribbon.color_mix` has no GPU field of its own.** It is consumed CPU-side
as the lerp factor into `GPURibbonState::color[3]`, which `ribbon.color_stim` also
feeds. Two pipes, one sink.

**GAP — `TIDE_SHIFT_MIN/MAX` and `RAIN_SCATTER_MIN/MAX`.** Declared in
`src/coupling/visual_canvas.hpp` under "DOOR AXES (Movement 1 harvest)" and read
by nothing: `git grep -n "TIDE_SHIFT_MIN\|TIDE_SHIFT_MAX\|RAIN_SCATTER_MIN\|RAIN_SCATTER_MAX" -- src/`
returns only the four declaration lines. They are not `ParamSlot` rows, so they
appear in no table above; recorded here because they sit inside the pipe file.

**GAP — ribbon pipe reach is one instance.** `ribbon_frame_tick` computes the four
ribbon pipes into `rs.gpu[i]` for every active `i` but calls
`upload_ribbon(queue, g)` with `g = rs.gpu[rs.rendered_slot]` only.

**RETRACTION — a claim this section previously made about
`GPUState::set_pawn_body_radius` is false and is withdrawn.** An earlier form of
§3.9 and of this subsection asserted that the setter "dirties the config without
assigning the new value", and presented
`if (config_.pawn_body_radius != r) { configDirty_ = true; }` as its guard body.
No such text exists in the tree. Re-read at
`src/cartridges/the_board/realization/state.hpp`, symbol
`t7::the_board::GPUState::set_pawn_body_radius`
(`grep -n -A6 "void set_pawn_body_radius" src/cartridges/the_board/realization/state.hpp`):

```cpp
            void set_pawn_body_radius(float r) {
                if (config_.pawn_body_radius != r) {
                    config_.pawn_body_radius = r;
                    configDirty_ = true;
                }
            }
```

The assignment is present, on the line immediately after the guard, and the setter
is shaped exactly like `GPUState::set_fog` and
`GPUState::set_checker_color_field` quoted in §3.6. The full name census confirms
there is one definition and one caller and no second setter:

```
git grep -n "pawn_body_radius" -- src/
#   src/cartridges/the_board/cartridge.hpp:1014:                // states. Recorded here because for pawn_body_radius a zero is
#   src/cartridges/the_board/cartridge.hpp:1034:                    gpuState_.set_pawn_body_radius(
#   src/cartridges/the_board/realization/state.hpp:788:            float pawn_body_radius;
#   src/cartridges/the_board/realization/state.hpp:3039:            void set_pawn_body_radius(float r) {
#   src/cartridges/the_board/realization/state.hpp:3040:                if (config_.pawn_body_radius != r) {
#   src/cartridges/the_board/realization/state.hpp:3041:                    config_.pawn_body_radius = r;
#   src/cartridges/the_board/realization/world.wgsl:1813 / 1817 / 8134 / 8139 / 8146 / 8151
#   src/console/organ_params.inc:340:ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, pawn_body_radius, F32,  "Pawn · Figure (driven)", "body radius")
```
`GPUDesignConfig::pawn_body_radius`'s §3.9 row stands on its writer claim alone;
there is no missing assignment and nothing here is a gap.

**No unit step failed.** Every count above ships its command; no row was dropped
and no value was guessed.

### 3.12 Amendment record — what an adversarial re-census changed

The whole section was re-run against the tree at
`6d53388e83f4a5cd7ad3b154484c885f567a02da` with a clean working tree
(`git status --porcelain` → 0 lines). One factual claim was **false** and is
retracted; two published recipes did not reproduce their own numbers and are
replaced; five quoted blocks were condensed or truncated without a marker and are
now verbatim; two omissions are filled. No substantive count changed.

| # | what was wrong | class | disposition |
| --- | --- | --- | --- |
| 1 | §3.9 row `GPUDesignConfig::pawn_body_radius` and the closing NOTE in §3.11 claimed `GPUState::set_pawn_body_radius` dirties the config without assigning, and quoted a guard body that does not exist in the tree | **false claim + fabricated quote (R5)** | Retracted in §3.11 with the real body quoted and the full `git grep -n "pawn_body_radius" -- src/` census. The §3.9 row now carries only its writer claim, which was correct |
| 2 | §3.8 Group A member recipe `(float\|uint32_t) [a-z_0-9]+` returns 10 lines, not the stated 14 | **R2** | Recipe replaced with the `\s+` form (returns 14, output pasted); the count 14 re-derived from the four sub-structs and from `static_assert(sizeof(DriverSurface) == 18 * sizeof(float))` |
| 3 | §3.8 Group B and §3.10 described `grep -nE "^inline constexpr" …/control_panel.hpp` as returning "the two aggregates `PANEL_TABLE` / `PANEL_LIVE`" | **R2** | Corrected to 17 lines = 16 scalars + one aggregate (`PANEL_TABLE`); `PANEL_LIVE` is `inline PanelSurface PANEL_LIVE = PANEL_TABLE;`, not constexpr, and is found by its own recipe. Dormant total 25 unchanged, both halves recounted |
| 4 | §3.8 recipe (c) said the cross-file `CANVAS_LIVE` grep "returns only the `organ_params.inc` enrollment lines" | **R2** | Corrected: it returns one line, a prose comment. The fifteen CANVAS enrollments never spell `CANVAS_LIVE.`; their form is `ORGAN_PARAM_NS(canvas, CANVAS, CanvasSurface, …)`. Sole-reader conclusion unchanged and re-verified |
| 5 | §3.5 exemplar 1 presented a 6-line condensation as the output of `git grep -n -- '"fog.density"' -- src/` | **R5/R2** | Replaced with the verbatim output, plus an independent `git grep -c` recount (2 + 4 + 3 = 9) and a per-symbol reading of every hit. **Partial disagreement, flagged in place**: the review called the real output 8 lines; at HEAD it is 9 (`… \| wc -l` → 9), and the tree is taken as the authority |
| 6 | §3.11 showed 2 of the 14 lines returned by `git grep -n "organ_params.inc" -- src/ tools/`, unlabelled | **R5/R2** | Full 14-line output pasted, then narrowed by two explicit follow-up commands. The GAP itself re-verified: one `#include` of a registry, 974 / 999 lines |
| 7 | §3.6 quoted `Cartridge::phase_motion_drivers` only to the checker `else` arm, dropping ~55 lines and the closing brace with no elision marker | **R5** | Whole function now quoted, with its boundaries named and the tail's three cross-links (zoetrope, `PANEL_LIVE.beacon`, live-config reads) recorded |
| 8 | §3.5's `beat_clock.hpp` blockquote ended "…via the graceful path." where the file continues | **R5** | Quoted to the end of the paragraph, with the dropped tail's two load-bearing facts named |
| 9 | §3.1's recipe carried `-- docs/`, which at this HEAD returns ~89 self-referential hits from the committed report | **R2 (contextual)** | Census re-scoped to `src/ tools/` (37 hits) with the definition-site isolated by its own command; the three counts published side by side |
| 10 | The header named HEAD `79adfa4d`, now the parent commit | **staleness** | Both commits named, their whole difference shown (`docs/LIGATURE_0_RECON.md`, one file), all sixteen blob SHAs re-verified at `6d53388e` |
| 11 | Group C gave type and min…max/step but no default, with no statement of why | **omission** | Filled: the enrollment grammar has no DEFAULT field in any of its six forms (banner quoted), and a 14-row block → live-instance → authored-table map now names where each block's defaults live, with its recipe |
| 12 | Group C's "current driver" column had no runnable command behind its macro-form → driver-class mapping | **omission** | Filled: a one-line `grep`/`awk` recipe emitting line, form, group and label for all 381 rows, plus a block census recipe reproducing the 13-way block distribution and the DEFKIND splits (55/19 DEFONLY, 78/32 DEF) |

Two verifier observations were checked and **not** acted on as corrections,
because the tree agrees with the text already in place: every published blob SHA
verifies (R6 clean), and no line-number anchor or proposal-shaped sentence
survives outside quoted tree text (R3, R4 clean). One observation — that the
verdict token is inverted relative to its own definition — was already flagged in
the original §3.4; the flag is now repeated at the head of the pipe table, in
§3.10's row for the source count, and in §3.11, so the column cannot be quoted
downstream without it.

## 4. Stat table + witnesses

**Commit of record:** `79adfa4d26c9e17e0074692928f1d2875d7edde1` (branch
`claude/ligature-0-recon-hcrix0`). Working tree unmodified; every command below is
read-only.

**Commit drift, recorded (amendment):** while this section was under verification the
branch tip advanced by one commit. `git rev-parse HEAD` now returns
`6d53388e83f4a5cd7ad3b154484c885f567a02da` ("LIGATURE_0 — the recon report…"), and
`git diff --stat 79adfa4d HEAD` reports exactly one changed path:

```
 docs/LIGATURE_0_RECON.md | 8837 ++++++++++++++++++++++++++++++++++++++++++++++
 1 file changed, 8837 insertions(+)
```

`git status --porcelain` is empty. Every blob this section cites is byte-identical at
both commits (proof in §4.0), so **no source-derived claim in this section is affected**.
One recipe *is* commit-sensitive, because it sweeps `docs/`: the `STAT_LAYOUT` census in
§4.1. It is republished there in a commit-pinned form.

### 4.0 Anchors (R6)

Recipe: `git rev-parse HEAD:<path>` and `git rev-parse 79adfa4d:<path>`, run for every
path below. The two columns are identical for all thirteen.

| path | blob SHA at `79adfa4d` | blob SHA at `HEAD` (`6d53388e`) |
| --- | --- | --- |
| `src/musical/signal_layout.hpp` | `8e2e84312483e31e429276d91c23f7d63dc2643c` | same |
| `src/analysis/analysis_signal.hpp` | `d088796d0ece785b9e34ee071273d6c5df7ce4a4` | same |
| `src/analysis/canvas_1/canvas.hpp` | `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5` | same |
| `src/analysis/canvas_1/check_canvas_compound.cpp` | `071ba66d5d5ec87c9672a629842052ba6efc4909` | same |
| `src/analysis/canvas_1/check_canvas_union.cpp` | `e8be1173416c8e56081ddabba5b8e4ed2cce288f` | same |
| `src/analysis/canvas_1/check_field_union.cpp` | `5cbe0a21bf8d94e9643ae280fc2e9c7c344706b3` | same |
| `src/analysis/canvas_1/check_pc_dft.cpp` | `f35a0927631c63df4685201f8cd9049315c5957d` | same |
| `src/analysis/canvas_1/probe_canvas.cpp` | `7a341e9c00a0b8bd1965714df87c533d30a6cced` | same |
| `CMakeLists.txt` (root) | `2dddc9202f4d74650e28f95b3aa536ddb81cda9a` | same |
| `src/coupling/visual_canvas.hpp` | `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35` | same |
| `src/musical/pc_count.hpp` | `acccba766e2a9b05b3d03f99ce3a238b6376dd9a` | same |
| `src/musical/pc_dft.hpp` | `8429dd9ba8a679268d5e30abe1d7b25ba4a68765` | same |
| `src/analysis/beat_clock.hpp` | `b10038ff5069783c6be15e1e8d885d36238f7354` | same |

Two further blobs are quoted in passing and carry their SHAs inline where they appear:
`src/the_board.cpp` = `588174ecddb0d68388e39a9025d6eda2f2afd000`.

---

### 4.1 Where the registration machinery actually lives

**Neither `signal_layout.hpp` nor `analysis_signal.hpp` registers a single stat name.**
Both are *contract* headers; the registration array is built at runtime by the
analysis cartridge. Recipe (commit-pinned — see the drift note above):

```
git grep -n "STAT_LAYOUT" 79adfa4d -- src/ tools/ docs/
    → 79adfa4d:src/analysis/analysis_cartridge.hpp:98:     * @return  Non-owning view over the cartridge's static STAT_LAYOUT.
    → 79adfa4d:src/analysis/analysis_signal.hpp:128:// Non-owning view over a cartridge's STAT_LAYOUT array. STAT_LAYOUT is
```

Exactly two hits, both prose comments. The enclosing symbols are, respectively, the
doc-block on `t7::AnalysisCartridge::stat_layout` (declared
`virtual StatLayoutView stat_layout() const = 0;` inside `class AnalysisCartridge`)
and the doc-block on `struct t7::StatLayoutView`. **There is no `STAT_LAYOUT` symbol
in the tree.** The name survives only in documentation of a shape that is now filled
imperatively.

The same recipe run without a revision at the current `HEAD` returns **15** hits, not
2, because thirteen of them live inside the newly committed `docs/LIGATURE_0_RECON.md`.
Scoping to `-- src/ tools/` (dropping `docs/`) also returns the two source hits at
`HEAD`:

```
git grep -n "STAT_LAYOUT" -- src/ tools/
    → src/analysis/analysis_cartridge.hpp:98
    → src/analysis/analysis_signal.hpp:128
```

#### `analysis_signal.hpp` — the passive descriptor

It declares the address space and the descriptor record; it names no stat.

```cpp
constexpr int MAX_CHANNELS = 8;
constexpr int STATS_PER_CHANNEL = 128;
constexpr int TOTAL_STATS = MAX_CHANNELS * STATS_PER_CHANNEL;  // 1024

/**
 * Compute array index for a (channel, stat) pair.
 */
constexpr int stat_index(int channel, int stat) {
    return channel * STATS_PER_CHANNEL + stat;
}
```

```cpp
enum class StatShape {
    Scalar,   // single value  → scrolling line
    Vector,   // run of `count` values → bar chart
};

struct StatGroup {
    const char* name;       // label, e.g. "abbott.pc_histogram"
    int         channel;    // AnalysisSignal channel
    int         slot_base;  // first slot
    int         count;      // number of slots (1 for scalar)
    StatShape   shape;
};

// Non-owning view over a cartridge's STAT_LAYOUT array. STAT_LAYOUT is
// static constexpr storage, so a pointer into it is valid for the whole
// program — this view is safe to copy and hold. It is the "key" the
// analysis cartridge publishes so the render side can resolve stat
// groups by name at runtime (see musical/signal_layout.hpp).
struct StatLayoutView {
    const StatGroup* groups;
    uint32_t         count;
};
```

The accessors the readers use, `t7::AnalysisSignal::stat` and
`t7::AnalysisSignal::set_stat`. **Whitespace note (R5):** the separator line between
the two members is four spaces, not empty — reproduced with
`grep -n "float stat(int channel" -A6 src/analysis/analysis_signal.hpp | cat -A`,
which renders it `    $`. It is written here as a line of four spaces; a markdown
renderer that trims trailing whitespace will show it empty.

```cpp
    float stat(int channel, int stat_type) const {
        return stats[stat_index(channel, stat_type)];
    }
    
    void set_stat(int channel, int stat_type, float value) {
        stats[stat_index(channel, stat_type)] = value;
    }
```

#### `signal_layout.hpp` — the resolve-by-name side

`t7::SignalLayout` is the *reader's* half. It registers nothing; it linearly
searches a bound `StatLayoutView`. The following is the **complete, un-elided** run
from `struct SourceBinding` through the closing brace of `SignalLayout::resolve`,
plus the two trailing accessors and the private state (blob
`8e2e84312483e31e429276d91c23f7d63dc2643c`):

```cpp
// One resolved stat group: where a named source lives in the signal.
// Held once after resolve(); width is irrelevant (never per-frame), so
// the fields mirror StatGroup's own int types — no narrowing on build.
struct SourceBinding {
    int  channel = 0;
    int  base    = 0;   // = StatGroup slot_base
    int  count   = 0;
    bool valid   = false;
};

class SignalLayout {
public:
    void bind(StatLayoutView v) { view_ = v; misses_ = 0; }

    // PORT_4c — how many resolves missed since the last bind(). The
    // release twin prints ONE summary line from this instead of one
    // line per source: with no audio source every resolve misses, and
    // twenty stderr lines read like twenty faults to a visitor who
    // opened DevTools out of curiosity. It is one fact — the socket is
    // empty — so it gets one line. The debug twin still names each.
    uint32_t misses() const { return misses_; }

    // Look up a source by name. Returns {valid=false} and warns on stderr
    // if the name is absent — callers leave the coupling disabled rather
    // than reading a wrong slot.
    SourceBinding resolve(std::string_view name) const {
        for (uint32_t i = 0; i < view_.count; ++i) {
            const StatGroup& g = view_.groups[i];
            if (name == g.name) {            // g.name is const char*
                return SourceBinding{ g.channel, g.slot_base, g.count, true };
            }
        }
        ++misses_;
#ifndef NDEBUG
        // Debug twin only (the-board-web-debug): the full list, one line
        // per source, unchanged. NDEBUG is the gate because CMake
        // defines it for Release and not for Debug, which is exactly the
        // two-preset split PORT_2c installed.
        std::fprintf(stderr,
            "[SignalLayout] source '%.*s' not in layout (coupling disabled)\n",
            (int)name.size(), name.data());
#endif
        return SourceBinding{};              // valid = false
    }

    uint32_t         count()  const { return view_.count; }
    const StatGroup* groups() const { return view_.groups; }

private:
    StatLayoutView view_{ nullptr, 0 };
    mutable uint32_t misses_ = 0;   // resolve() is const; the tally is not state the caller owns
};
```

Three facts this un-elided quote carries that an abridged one did not:

* The stderr warn inside `SignalLayout::resolve` is gated by `#ifndef NDEBUG`, and
  the comment names the gate's reason ("CMake defines it for Release and not for
  Debug"). The warn therefore does **not** fire in a Release build; only the
  `misses_` tally survives. This is load-bearing for FLAG-2 below.
* The doc-block on `SignalLayout::misses` records the PORT_4c intent — one summary
  line per bind, not one per source.
* `misses_` is `mutable`; `resolve` is `const` and still increments it.

The header's own USAGE doc-block, above `namespace t7`, is where the seventh hit of
the §4.5 R-a recipe comes from:

```cpp
// USAGE (B2): resolve ONCE, store the SourceBinding, never per-frame:
//   SourceBinding src = signal_layout_.resolve("abbott.lowest_pc");
//   if (src.valid) { ... read stats[src.channel][src.base + i], i < count ... }
```

#### `canvas.hpp` — the only registrar in the tree

`t7::canvas_1::Canvas::publish_reading` is the sole writer of `StatGroup` records.
It gates the layout entry and the slot-write in one act:

```cpp
    bool publish_reading(Reading r, const Source& src, const char* name, int want_band = -1) {
        if (src.count() == 0)                  return false;
        if (!available(r, src))                return false;
        if (!writer_wired(r))                  return false;
        if (published_count_ >= MAX_PUBLISHED) return false;

        const ReadingSpec rs = reading_spec(r);
        const int band = (want_band >= 0) ? want_band
                       : (src.is_group() ? GROUP_BAND : src.first());

        layout_[published_count_]    = StatGroup{ name, band, rs.slot, rs.width, rs.shape };
        published_[published_count_] = Published{ r, src.mask, band, HeldField{} };
        ++published_count_;
        return true;
    }
```

The canonical address map, `Canvas` private statics — **complete, including the six
continuation comment lines** (recipe:
`sed -n '/static constexpr int SLOT_PRESENT_COUNT/,/each onset lands in exactly one frame/p' src/analysis/canvas_1/canvas.hpp`):

```cpp
    static constexpr int SLOT_PRESENT_COUNT  = 0;    // 12  present pcs, by count
    static constexpr int SLOT_PRESENT_LENGTH = 12;   // 12  present pcs, by length (beats)
    static constexpr int SLOT_WINDOW_COUNT   = 24;   // 12  present+window, by count
    static constexpr int SLOT_WINDOW_LENGTH  = 36;   // 12  present+window, by length (beats)
    static constexpr int SLOT_CURRENT_PC     = 48;   // 12  current note, one-hot
    static constexpr int SLOT_DISTANCE       = 60;   // 1   signed registral interval
    static constexpr int SLOT_FIELD          = 61;   // 1   held field index 1..6
    static constexpr int SLOT_POLYPHONY      = 62;   // 1   present voice count
    static constexpr int SLOT_DFT_MAG        = 64;   // 6   pc-DFT |X1..X6| ÷ L1, [0,1]
                                                     //     (f3 triadicity · f5 fifths/diatonic · f6 whole-tone)
    static constexpr int SLOT_DFT_PHASE      = 70;   // 6   pc-DFT arg(X1..X6), radians [−π,π],
                                                     //     origin = the published D origin;
                                                     //     REST: zero vector → mags 0, phases HOLD-LAST
    static constexpr int SLOT_ONSET          = 76;   // 12  note-on impulses since the previous
                                                     //     published frame, velocity-weighted;
                                                     //     each onset lands in exactly one frame
```

Three source-side facts live only in those continuation lines:

* `SLOT_DFT_MAG`'s families are named — f3 triadicity, f5 fifths/diatonic, f6
  whole-tone. This is the header's own statement of what `check_pc_dft.cpp` asserts
  (§4.6).
* `SLOT_DFT_PHASE` states the REST behaviour **at the address map**: "REST: zero
  vector → mags 0, phases HOLD-LAST". This is the source-side statement of the
  hold-last behaviour that `Canvas::Published::held_phase` implements in
  `Canvas::write_reading` (§4.4).
* `SLOT_ONSET` states the aperture invariant — "each onset lands in exactly one
  frame" — matching the `onset_prev_beat_` bookkeeping in `Canvas::publish`.

There is a **numbering gap between `SLOT_POLYPHONY = 62` and `SLOT_DFT_MAG = 64`**:
slot 63 is named by no constant.

```cpp
    struct ReadingSpec { int slot; int width; StatShape shape; };

    static ReadingSpec reading_spec(Reading r) {
        switch (r) {
            case Reading::PresentCount:  return { SLOT_PRESENT_COUNT,  12, StatShape::Vector };
            case Reading::PresentLength: return { SLOT_PRESENT_LENGTH, 12, StatShape::Vector };
            case Reading::WindowCount:   return { SLOT_WINDOW_COUNT,   12, StatShape::Vector };
            case Reading::WindowLength:  return { SLOT_WINDOW_LENGTH,  12, StatShape::Vector };
            case Reading::CurrentPC:     return { SLOT_CURRENT_PC,     12, StatShape::Vector };
            case Reading::Distance:      return { SLOT_DISTANCE,        1, StatShape::Scalar };
            case Reading::Field:         return { SLOT_FIELD,           1, StatShape::Scalar };
            case Reading::Polyphony:     return { SLOT_POLYPHONY,       1, StatShape::Scalar };
            case Reading::DftMag:        return { SLOT_DFT_MAG,         6, StatShape::Vector };
            case Reading::DftPhase:      return { SLOT_DFT_PHASE,       6, StatShape::Vector };
            case Reading::Onset:         return { SLOT_ONSET,          12, StatShape::Vector };
        }
        return { 0, 1, StatShape::Scalar };   // unreachable; quiets the compiler
    }
```

The two gates that decide whether a declaration is admitted:

```cpp
    static bool reading_needs_window(Reading r) {
        return r == Reading::WindowCount || r == Reading::WindowLength
            || r == Reading::Field
            || r == Reading::DftMag || r == Reading::DftPhase
            || r == Reading::Onset;
    }
    static bool reading_needs_spine(Reading r) {
        return r == Reading::CurrentPC || r == Reading::Distance;
    }
```

```cpp
    static bool writer_wired(Reading r) {
        return r == Reading::Field || r == Reading::CurrentPC
            || r == Reading::PresentCount
            || r == Reading::WindowLength || r == Reading::Distance
            || r == Reading::DftMag || r == Reading::DftPhase
            || r == Reading::Onset;
    }
```

```cpp
    bool available(Reading r, const Source& src) const {
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (!(src.mask & (1u << i))) continue;
            if (!active_[i]) return false;
            if (reading_needs_window(r) && !specs_[i].has_window())    return false;
            if (reading_needs_spine(r)  && !specs_[i].has_crossings()) return false;
        }
        return true;
    }
```

The name tables (static storage, positional by channel index) — `Canvas::NAME_CURRENT_PC`,
`NAME_PRESENT_COUNT`, `NAME_WINDOW_LENGTH`, `NAME_DISTANCE`, `NAME_DFT_MAG`,
`NAME_DFT_PHASE`, `NAME_ONSET`, each `[MAX_CHANNELS]` and filled `"ch0.…" … "ch7.…"`.
Group names are string literals supplied at the `publish_reading` call site.

---

### 4.2 The count, with its recipe (R2)

The live registration is `t7::canvas_1::Canvas::initialize`. Census:

```
# C1 — publish_reading call sites inside Canvas::initialize
awk '/^    void initialize\(const char\* asset_path\) override \{/,/^    \}$/' \
    src/analysis/canvas_1/canvas.hpp | grep -c 'publish_reading('
    → 13

# C2 — the loop bound
grep -n "constexpr int VOICES" src/analysis/canvas_1/canvas.hpp
    → 110:        constexpr int VOICES = 7;

# C3 — the band constants
grep -n "GROUP_BAND = \|MAX_PUBLISHED = " src/analysis/canvas_1/canvas.hpp
    → 410:    static constexpr int GROUP_BAND = MAX_CHANNELS - 1;      (= 7)
    → 418:    static constexpr int MAX_PUBLISHED = MAX_CHANNELS * 8;   (= 64)
grep -n "MAX_CHANNELS = " src/analysis/analysis_signal.hpp
    → 57:constexpr int MAX_CHANNELS = 8;
```

The body of `Canvas::initialize`, verbatim and complete (same `awk` boundary as C1),
so the 7-in-loop / 6-in-group split is evidenced rather than asserted:

```cpp
    void initialize(const char* asset_path) override {
        (void)asset_path;   // no asset to load; the DAW is the source
        // The composition (a placeholder until the composer sets it): seven
        // voices, slot v <- MIDI v, each present + a four-beat window + the spine
        // on (so the line readings are available). The split of bands into voices
        // and a compound band is the composer's, not fixed: here voices 0..6 and
        // the cross-voice compounds in the group band (7).
        constexpr int VOICES = 7;
        for (int v = 0; v < VOICES; ++v) {
            ContextSpec s = default_spec(/*midi*/ v, /*window*/ 4.0f);
            s.crossings.active = true;   // the spine — current_pc and distance read it
            configure(v, s);
        }

        // Per-voice readings: the current note (one-hot), the present set (the
        // Playhead, by count — published on demand for the sustain law, its
        // first consumer), the present+window length (beats), and the line's
        // signed distance.
        for (int v = 0; v < VOICES; ++v) {
            publish_reading(Reading::CurrentPC,    Source::channel(v), NAME_CURRENT_PC[v]);
            publish_reading(Reading::PresentCount, Source::channel(v), NAME_PRESENT_COUNT[v]);
            publish_reading(Reading::WindowLength, Source::channel(v), NAME_WINDOW_LENGTH[v]);
            publish_reading(Reading::Distance,     Source::channel(v), NAME_DISTANCE[v]);
            // The pc-DFT capability (compound stratum) — the six interval
            // families over this voice's published present-count vector.
            publish_reading(Reading::DftMag,       Source::channel(v), NAME_DFT_MAG[v]);
            publish_reading(Reading::DftPhase,     Source::channel(v), NAME_DFT_PHASE[v]);
            publish_reading(Reading::Onset,        Source::channel(v), NAME_ONSET[v]);
        }

        // Compound readings over the union of all voices, in the group band: the
        // field (set-union then election), the current notes (vector sum — a
        // per-pc voice count), the present set (the room sounding), and the
        // present+window length (vector sum).
        const Source all = Source::group({0, 1, 2, 3, 4, 5, 6});
        publish_reading(Reading::Field,        all, "all.field");
        publish_reading(Reading::CurrentPC,    all, "all.current_pc");
        publish_reading(Reading::PresentCount, all, "all.present_count");
        publish_reading(Reading::WindowLength, all, "all.window_length");
        publish_reading(Reading::DftMag,       all, "all.dft_mag");
        publish_reading(Reading::DftPhase,     all, "all.dft_phase");

        port_.open_by_name("loopMIDI");   // the DAW's virtual port

        std::fprintf(stderr, "[canvas] loopMIDI open=%d\n", (int)port_.is_open());
    }
```

**Recorded discrepancy inside the quoted body:** the comment above the DFT pair reads
"the six interval families over this voice's published **present-count** vector",
while `Canvas::write_reading` computes the DFT over
`reading_vector(Reading::WindowLength, …)` and its own comment there says the
present-count feed "was starved by design and is retired" (§4.4). The two comments
in the same header disagree about the DFT's input; the code agrees with the
`write_reading` comment.

Of the 13 call sites, **7 sit inside the `for (int v = 0; v < VOICES; ++v)` loop**
and **6 are the group block**. Therefore:

> **7 × 7 + 6 = 55 registered stat names.**

Every one of the 55 is *admitted* (none is refused), because:

* `available()` — `Canvas::initialize` composes each voice with
  `ContextSpec s = default_spec(/*midi*/ v, /*window*/ 4.0f); s.crossings.active = true;`.
  Recipe (symbol-scoped, not line-anchored):
  `awk '/^inline ContextSpec default_spec/,/^}/' src/musical/context_spec.hpp`
  (blob `563962e7d0f05355d953d498ad6137606a627d7a`) returns

  ```cpp
  inline ContextSpec default_spec(int channel, float window_span_beats) {
      ContextSpec s;
      s.channel = channel;
      s.present = true;
      s.add_window(window_span_beats);
      s.stream_retention_beats = s.required_retention_beats();
      return s;
  }
  ```

  so `ContextSpec::has_window()` (`return window_count() > 0;`) is true; the explicit
  `crossings.active = true` makes `ContextSpec::has_crossings()`
  (`return crossings.active;`) true. Both window-gated and spine-gated readings
  therefore pass for every voice 0..6 and for the union `{0,1,2,3,4,5,6}`.
* `writer_wired()` — all eight declared kinds (`Field`, `CurrentPC`,
  `PresentCount`, `WindowLength`, `Distance`, `DftMag`, `DftPhase`, `Onset`)
  are in the predicate above.
* `published_count_ >= MAX_PUBLISHED` — 55 < 64.

Registration order is the table order below (loop-major: voice 0's seven, then
voice 1's seven, …, then the six group readings), indices 0..54 in
`Canvas::layout_`.

---

### 4.3 The 55-row stat table

`producer file` is where the producer symbol is *defined*; every one of them is
*invoked* from `t7::canvas_1::Canvas::write_reading` or
`t7::canvas_1::Canvas::per_channel_reading`.
`read by` is the render-side consumer resolved through `t7::SignalLayout::resolve`.
The complete reader census is §4.5; the only reader file in `src/` is
`src/coupling/visual_canvas.hpp`, whose reads all occur in
`t7::VisualCanvas::tick` after resolution in `t7::VisualCanvas::bind`.

**Producer-column convention (amendment).** Where a row names
`t7::pc_length(const PlayheadReadout&, const WagonReadout&)` or
`t7::pc_length(ph, wg)`, that two-argument overload is itself a *composite*: its body
in `src/musical/pc_count.hpp` (blob `acccba766e2a9b05b3d03f99ce3a238b6376dd9a`) reads

```cpp
inline PitchClassVector pc_length(const PlayheadReadout& ph, const WagonReadout& wg) {
    PitchClassVector v = pc_length(ph);
    const PitchClassVector w = pc_length(wg);
    for (int i = 0; i < 12; ++i) v.v[i] += w.v[i];
    return v;
}
```

so `t7::pc_length(const PlayheadReadout&)` and `t7::pc_length(const WagonReadout&)`
are both executed on every `window_length` row and — because the DFT reads the
`WindowLength` vector — on every DFT row as well. §4.4's analyzer table records them
as wired for this reason.

| stat name | channel | base | count | producer symbol | producer file | read by (file :: symbol) or NONE |
| --- | --- | --- | --- | --- | --- | --- |
| `ch0.current_pc` | 0 | 48 | 12 | `t7::current_note(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch0.present_count` | 0 | 0 | 12 | `t7::pc_count(const PlayheadReadout&)` | `src/musical/pc_count.hpp` | NONE |
| `ch0.window_length` | 0 | 36 | 12 | `t7::pc_length(const PlayheadReadout&, const WagonReadout&)` → `pc_length(ph)` + `pc_length(wg)` | `src/musical/pc_count.hpp` | NONE |
| `ch0.distance` | 0 | 60 | 1 | `t7::line_distance(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch0.dft_mag` | 0 | 64 | 6 | `t7::pc_dft(const PitchClassVector&)` ← `t7::pc_length(ph,wg)` | `src/musical/pc_dft.hpp` | NONE |
| `ch0.dft_phase` | 0 | 70 | 6 | `t7::pc_dft(const PitchClassVector&)` + `Canvas::Published::held_phase` | `src/musical/pc_dft.hpp` | NONE |
| `ch0.onset` | 0 | 76 | 12 | `t7::pc_onset(const PlayheadReadout&, const WagonReadout&, float)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `zoetrope_ears_[]`) |
| `ch1.current_pc` | 1 | 48 | 12 | `t7::current_note(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch1.present_count` | 1 | 0 | 12 | `t7::pc_count(const PlayheadReadout&)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `voice_playhead_`, `RIBBON_VOICE == "ch1"`) |
| `ch1.window_length` | 1 | 36 | 12 | `t7::pc_length(const PlayheadReadout&, const WagonReadout&)` → `pc_length(ph)` + `pc_length(wg)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `checker_win_`, `CHECKER_VOICE == "ch1"`) |
| `ch1.distance` | 1 | 60 | 1 | `t7::line_distance(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch1.dft_mag` | 1 | 64 | 6 | `t7::pc_dft(const PitchClassVector&)` | `src/musical/pc_dft.hpp` | NONE |
| `ch1.dft_phase` | 1 | 70 | 6 | `t7::pc_dft(...)` + `Canvas::Published::held_phase` | `src/musical/pc_dft.hpp` | NONE |
| `ch1.onset` | 1 | 76 | 12 | `t7::pc_onset(...)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `zoetrope_ears_[]`) |
| `ch2.current_pc` | 2 | 48 | 12 | `t7::current_note(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch2.present_count` | 2 | 0 | 12 | `t7::pc_count(const PlayheadReadout&)` | `src/musical/pc_count.hpp` | NONE |
| `ch2.window_length` | 2 | 36 | 12 | `t7::pc_length(ph, wg)` → `pc_length(ph)` + `pc_length(wg)` | `src/musical/pc_count.hpp` | NONE |
| `ch2.distance` | 2 | 60 | 1 | `t7::line_distance(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch2.dft_mag` | 2 | 64 | 6 | `t7::pc_dft(...)` | `src/musical/pc_dft.hpp` | NONE |
| `ch2.dft_phase` | 2 | 70 | 6 | `t7::pc_dft(...)` + `held_phase` | `src/musical/pc_dft.hpp` | NONE |
| `ch2.onset` | 2 | 76 | 12 | `t7::pc_onset(...)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `zoetrope_ears_[]`) |
| `ch3.current_pc` | 3 | 48 | 12 | `t7::current_note(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch3.present_count` | 3 | 0 | 12 | `t7::pc_count(const PlayheadReadout&)` | `src/musical/pc_count.hpp` | NONE |
| `ch3.window_length` | 3 | 36 | 12 | `t7::pc_length(ph, wg)` → `pc_length(ph)` + `pc_length(wg)` | `src/musical/pc_count.hpp` | NONE |
| `ch3.distance` | 3 | 60 | 1 | `t7::line_distance(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch3.dft_mag` | 3 | 64 | 6 | `t7::pc_dft(...)` | `src/musical/pc_dft.hpp` | NONE |
| `ch3.dft_phase` | 3 | 70 | 6 | `t7::pc_dft(...)` + `held_phase` | `src/musical/pc_dft.hpp` | NONE |
| `ch3.onset` | 3 | 76 | 12 | `t7::pc_onset(...)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `zoetrope_ears_[]`) |
| `ch4.current_pc` | 4 | 48 | 12 | `t7::current_note(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch4.present_count` | 4 | 0 | 12 | `t7::pc_count(const PlayheadReadout&)` | `src/musical/pc_count.hpp` | NONE |
| `ch4.window_length` | 4 | 36 | 12 | `t7::pc_length(ph, wg)` → `pc_length(ph)` + `pc_length(wg)` | `src/musical/pc_count.hpp` | NONE |
| `ch4.distance` | 4 | 60 | 1 | `t7::line_distance(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch4.dft_mag` | 4 | 64 | 6 | `t7::pc_dft(...)` | `src/musical/pc_dft.hpp` | NONE |
| `ch4.dft_phase` | 4 | 70 | 6 | `t7::pc_dft(...)` + `held_phase` | `src/musical/pc_dft.hpp` | NONE |
| `ch4.onset` | 4 | 76 | 12 | `t7::pc_onset(...)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `zoetrope_ears_[]`) |
| `ch5.current_pc` | 5 | 48 | 12 | `t7::current_note(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch5.present_count` | 5 | 0 | 12 | `t7::pc_count(const PlayheadReadout&)` | `src/musical/pc_count.hpp` | NONE |
| `ch5.window_length` | 5 | 36 | 12 | `t7::pc_length(ph, wg)` → `pc_length(ph)` + `pc_length(wg)` | `src/musical/pc_count.hpp` | NONE |
| `ch5.distance` | 5 | 60 | 1 | `t7::line_distance(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch5.dft_mag` | 5 | 64 | 6 | `t7::pc_dft(...)` | `src/musical/pc_dft.hpp` | NONE |
| `ch5.dft_phase` | 5 | 70 | 6 | `t7::pc_dft(...)` + `held_phase` | `src/musical/pc_dft.hpp` | NONE |
| `ch5.onset` | 5 | 76 | 12 | `t7::pc_onset(...)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `zoetrope_ears_[]`) |
| `ch6.current_pc` | 6 | 48 | 12 | `t7::current_note(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch6.present_count` | 6 | 0 | 12 | `t7::pc_count(const PlayheadReadout&)` | `src/musical/pc_count.hpp` | NONE |
| `ch6.window_length` | 6 | 36 | 12 | `t7::pc_length(ph, wg)` → `pc_length(ph)` + `pc_length(wg)` | `src/musical/pc_count.hpp` | NONE |
| `ch6.distance` | 6 | 60 | 1 | `t7::line_distance(const Spine&)` | `src/musical/spine_ops.hpp` | NONE |
| `ch6.dft_mag` | 6 | 64 | 6 | `t7::pc_dft(...)` | `src/musical/pc_dft.hpp` | NONE |
| `ch6.dft_phase` | 6 | 70 | 6 | `t7::pc_dft(...)` + `held_phase` | `src/musical/pc_dft.hpp` | NONE |
| `ch6.onset` | 6 | 76 | 12 | `t7::pc_onset(...)` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `zoetrope_ears_[]`) |
| `all.field` | 7 | 61 | 1 | `t7::HeldField::step` (via `Canvas::step_fields`), published through `Canvas::field_index` | `src/musical/field.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `fog_field_`) |
| `all.current_pc` | 7 | 48 | 12 | `t7::current_note(const Spine&)` summed by `Canvas::reading_vector` | `src/musical/spine_ops.hpp` | NONE |
| `all.present_count` | 7 | 0 | 12 | `t7::pc_count(const PlayheadReadout&)` summed by `Canvas::reading_vector` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `room_playhead_`) |
| `all.window_length` | 7 | 36 | 12 | `t7::pc_length(ph, wg)` summed by `Canvas::reading_vector` | `src/musical/pc_count.hpp` | `src/coupling/visual_canvas.hpp :: t7::VisualCanvas::tick` (via `room_wagon_`) |
| `all.dft_mag` | 7 | 64 | 6 | `t7::pc_dft(...)` over the union's `WindowLength` vector | `src/musical/pc_dft.hpp` | NONE |
| `all.dft_phase` | 7 | 70 | 6 | `t7::pc_dft(...)` + `Canvas::Published::held_phase` | `src/musical/pc_dft.hpp` | NONE |

Row count: 49 per-voice + 6 group = **55**, matching §4.2.

Two notes on the producer column, both verbatim from `Canvas::write_reading`:

* `dft_mag` / `dft_phase` do **not** read the present-count feed. Their input is
  `dress(reading_vector(Reading::WindowLength, p.source_mask), to_D)` — i.e.
  `pc_length(playhead, wagon(0))` re-origined to D — so `pc_length` is upstream of
  every DFT row.
* Every vector row is re-origined by `t7::dress` (`src/musical/vector_dressing.hpp`)
  with `VectorDressing{ true, PROJECT_PC_ORIGIN, VectorDressing::Scale::None }`,
  `PROJECT_PC_ORIGIN = 2` (D). Index 0 of every 12-wide row is therefore D, not C.

---

### 4.4 (a) WIRED ANALYZERS

#### The dispatch path, verbatim

`t7::canvas_1::Canvas::update` — the per-frame entry a render side calls:

```cpp
    void update(float dt) override {
        dt_         = dt;          // wall-clock delta, telemetry only
        t_seconds_ += dt;
        const float beat = static_cast<float>(port_.beats());   // the DAW's clock
        MidiEvent ev[256];
        const int n = port_.poll(beat, ev, 256);

        for (int i = 0; i < n; ++i) route(ev[i]);
        advance(beat);
    }
```

`t7::canvas_1::Canvas::advance` — the frame:

```cpp
    void advance(float beat) {
        for (int i = 0; i < MAX_CHANNELS; ++i)
            if (active_[i]) contexts_[i].update(beat);
        step_fields();
        publish(beat);
    }
```

`t7::canvas_1::Canvas::publish` — the closure over the published list. **Disclosed
elision:** one block is replaced by a marker — the `INSTRUMENTS.zoetrope_witness`
stderr block, which reads `output_.stat(v, SLOT_ONSET + i)` in two loops and writes
only to `stderr`. It is the source of the two `canvas.hpp` hits in the §4.5 R-b
recipe. Nothing else in `publish` is elided:

```cpp
    void publish(float beat) {
        output_.t_beats   = beat;
        output_.t_seconds = t_seconds_;
        output_.dt        = dt_;

        // The onset aperture: a backward beat jump (a DAW transport loop)
        // re-anchors the since-edge, so a looped clip keeps striking; the
        // edge then advances once per published frame, so each onset lands
        // in exactly one frame's (prev, beat] window.
        if (beat < onset_prev_beat_) onset_prev_beat_ = beat;

        for (int k = 0; k < published_count_; ++k)
            write_reading(published_[k]);

        /* … ELIDED: INSTRUMENTS.zoetrope_witness stderr block … */

        onset_prev_beat_ = beat;
    }
```

`t7::canvas_1::Canvas::write_reading` — **the dispatch**. This is the switch that
decides which analyzer runs. Quoted **complete and un-elided**, comments included
(recipe:
`awk '/^    void write_reading\(Published& p\) \{/,/^    \}$/' src/analysis/canvas_1/canvas.hpp`,
against blob `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5`):

```cpp
    void write_reading(Published& p) {
        const int slot = reading_spec(p.reading).slot;
        switch (p.reading) {
            case Reading::DftMag:
            case Reading::DftPhase: {
                // THE COMPOUND STRATUM: the pc-DFT of the PUBLISHED
                // window-length vector — pc_length(playhead, wagon(0)):
                // the whole 4-beat collection, present + completed,
                // duration-weighted, dressed to D first (the DFT reads
                // what ships, not raw state). THE APERTURE IS PART OF
                // THE INSTRUMENT'S MEANING: a spectrum is a property of
                // a body of material, and the instant is not a body —
                // the present-count feed (gen-1) was starved by design
                // and is retired. REST: zero vector → mags 0, phases
                // HOLD-LAST per channel (the hold lives with the entry,
                // like the held field — consumers fading on mag never
                // see phase snap).
                const VectorDressing to_D{ /*reorigin*/ true, PROJECT_PC_ORIGIN, VectorDressing::Scale::None };
                const PitchClassVector v = dress(reading_vector(Reading::WindowLength, p.source_mask), to_D);
                float l1 = 0.0f;
                for (int i = 0; i < 12; ++i) l1 += v.v[i];
                const PcDft d = pc_dft(v);
                if (p.reading == Reading::DftMag) {
                    for (int i = 0; i < 6; ++i)
                        output_.set_stat(p.band, slot + i, d.mag[i]);
                } else {
                    for (int i = 0; i < 6; ++i) {
                        if (l1 > 0.0f) p.held_phase[i] = d.phase[i];
                        output_.set_stat(p.band, slot + i, p.held_phase[i]);
                    }
                }
                break;
            }
            case Reading::Field:
                output_.set_stat(p.band, slot, static_cast<float>(field_index(p.field)));
                break;
            case Reading::CurrentPC:
            case Reading::PresentCount:
            case Reading::WindowLength:
            case Reading::Onset: {
                // Vector readings: the per-source sum dressed to D. One channel ->
                // that channel's reading; a union -> the cross-voice vector sum —
                // same code, since the additive compound is just the sum.
                const VectorDressing to_D{ /*reorigin*/ true, PROJECT_PC_ORIGIN, VectorDressing::Scale::None };
                write_vector(p.band, slot, dress(reading_vector(p.reading, p.source_mask), to_D));
                break;
            }
            case Reading::Distance: {
                // The line's signed interval — a per-voice scalar; no union form.
                const int c = first_source(p.source_mask);
                output_.set_stat(p.band, slot,
                    c >= 0 ? static_cast<float>(line_distance(contexts_[c].spine())) : 0.0f);
                break;
            }
            default:
                // Unreachable: publish_reading refuses any reading not wired here
                // (see writer_wired). The remaining readings stay as capability.
                break;
        }
    }
```

Facts carried only by the three previously-dropped comment blocks:

* The `DftMag`/`DftPhase` block states in the source itself that the DFT's input is
  the **window-length** vector and that "the present-count feed (gen-1) was starved
  by design and is retired". This is the header's own record of a retired feed.
* The same block states the REST rule — "zero vector → mags 0, phases HOLD-LAST per
  channel (the hold lives with the entry, like the held field)" — which is the
  narrative behind the `if (l1 > 0.0f) p.held_phase[i] = d.phase[i];` line and
  matches the `SLOT_DFT_PHASE` continuation comment quoted in §4.1.
* The vector-reading block states that the union case is a **vector sum** ("the
  additive compound is just the sum"), which is what `check_canvas_compound.cpp`
  asserts (§4.6).
* The `Distance` block states there is **no union form** for the line's interval —
  the source-side reason `all.distance` is not among the 55.

`t7::canvas_1::Canvas::per_channel_reading` — the second dispatch, reached from
`Canvas::reading_vector`:

```cpp
    PitchClassVector per_channel_reading(Reading r, int i) const {
        const Context& c = contexts_[i];
        switch (r) {
            case Reading::CurrentPC:    return current_note(c.spine());
            case Reading::PresentCount: return pc_count(c.playhead());
            case Reading::WindowLength: return pc_length(c.playhead(), c.wagon(0));
            case Reading::Onset:        return pc_onset(c.playhead(), c.wagon(0), onset_prev_beat_);
            default:                    return PitchClassVector{};
        }
    }
```

`t7::canvas_1::Canvas::step_fields` — the field's one mutation, called from `advance`:

```cpp
    void step_fields() {
        for (int k = 0; k < published_count_; ++k) {
            Published& p = published_[k];
            if (p.reading != Reading::Field) continue;
            const PitchClassVector degrees =
                to_degrees(union_present_set(p.source_mask), PROJECT_PC_ORIGIN);
            p.field.step(degrees, bank(), BANK_SIZE);
        }
    }
```

#### What "wired" means here

"Wired" means **invoked on the path `Canvas::update → advance → {step_fields,
publish → write_reading → reading_vector → per_channel_reading}` under the
composition `Canvas::initialize` declares**, *including transitively* — a function
whose only caller is itself on that path is on that path. Callers of the path in
`src/` are enumerated in §4.4.1 — read that before treating "yes" as runtime-live.

#### The full call-graph census for `pc_count` / `pc_length` / `pc_set` (amendment)

The earlier draft of this section marked four overloads defined-but-uncalled on the
strength of a summary. The three recipes, run in full, are:

```
grep -rn "pc_count(" --include='*.hpp' --include='*.cpp' src/
    → src/musical/pc_count.hpp:41:inline PitchClassVector pc_count(const PlayheadReadout& ph) {
    → src/musical/pc_count.hpp:50:inline PitchClassVector pc_count(const WagonReadout& wg) {
    → src/musical/pc_count.hpp:59:inline PitchClassVector pc_count(const PlayheadReadout& ph, const WagonReadout& wg) {
    → src/musical/pc_count.hpp:60:    PitchClassVector v = pc_count(ph);
    → src/musical/pc_count.hpp:61:    const PitchClassVector w = pc_count(wg);
    → src/analysis/canvas_1/canvas.hpp:594:            case Reading::PresentCount: return pc_count(c.playhead());
    (6 lines: 3 definitions, 2 inner calls inside the two-argument overload,
     1 external call inside Canvas::per_channel_reading)

grep -rn "pc_length(" --include='*.hpp' --include='*.cpp' src/
    → src/musical/pc_count.hpp:71:inline PitchClassVector pc_length(const PlayheadReadout& ph) {
    → src/musical/pc_count.hpp:81:inline PitchClassVector pc_length(const WagonReadout& wg) {
    → src/musical/pc_count.hpp:91:inline PitchClassVector pc_length(const PlayheadReadout& ph, const WagonReadout& wg) {
    → src/musical/pc_count.hpp:92:    PitchClassVector v = pc_length(ph);
    → src/musical/pc_count.hpp:93:    const PitchClassVector w = pc_length(wg);
    → src/analysis/canvas_1/canvas.hpp:513:                // window-length vector — pc_length(playhead, wagon(0)):
    → src/analysis/canvas_1/canvas.hpp:595:            case Reading::WindowLength: return pc_length(c.playhead(), c.wagon(0));
    → src/coupling/visual_canvas.hpp:37:// voice). The voice's WINDOW pc-LENGTH vector — pc_length(playhead,
    → src/coupling/visual_canvas.hpp:188:    // pc_length(playhead, wagon(0)). NO DFT, no interval math: each PITCH
    (9 lines: 3 definitions, 2 inner calls, 1 external call inside
     Canvas::per_channel_reading, 3 comments)

grep -rn "pc_set" --include='*.hpp' --include='*.cpp' src/
    → src/musical/pc_count.hpp:20://   pc_set    — support of a count: the pitch-class SET, a class in iff it
    → src/musical/pc_count.hpp:126:inline PitchClassBits pc_set(const PitchClassVector& count) {
    (2 lines: 1 doc-comment line in the header's own summary block,
     1 definition. Zero call sites.)
```

The resulting call graph, by symbol:

| symbol | direct callers in `src/` | on the publish path? |
| --- | --- | --- |
| `t7::pc_count(const PlayheadReadout&)` | `t7::canvas_1::Canvas::per_channel_reading` (`case Reading::PresentCount`); `t7::pc_count(const PlayheadReadout&, const WagonReadout&)` | **yes** |
| `t7::pc_count(const WagonReadout&)` | `t7::pc_count(const PlayheadReadout&, const WagonReadout&)` — exactly one | **no** (its one caller is itself uncalled) |
| `t7::pc_count(const PlayheadReadout&, const WagonReadout&)` | none | **no** (zero call sites) |
| `t7::pc_length(const PlayheadReadout&)` | `t7::pc_length(const PlayheadReadout&, const WagonReadout&)` — exactly one | **yes** (that caller is wired) |
| `t7::pc_length(const WagonReadout&)` | `t7::pc_length(const PlayheadReadout&, const WagonReadout&)` — exactly one | **yes** (that caller is wired) |
| `t7::pc_length(const PlayheadReadout&, const WagonReadout&)` | `t7::canvas_1::Canvas::per_channel_reading` (`case Reading::WindowLength`) | **yes** |
| `t7::pc_set(const PitchClassVector&)` | none | **no** (zero call sites) |
| `t7::pc_onset(const PlayheadReadout&, const WagonReadout&, float)` | `t7::canvas_1::Canvas::per_channel_reading` (`case Reading::Onset`) | **yes** |

**The zero-call-site set in `src/` is exactly two symbols:**
`t7::pc_count(const PlayheadReadout&, const WagonReadout&)` and
`t7::pc_set(const PitchClassVector&)`. A third,
`t7::pc_count(const WagonReadout&)`, has one caller and is unreachable only
because that caller is itself unreachable.

#### The analyzer table

| analyzer symbol | what it computes | stat(s) it publishes | wired? | evidence command |
| --- | --- | --- | --- | --- |
| `t7::pc_count(const PlayheadReadout&)` | present pitch classes, by note count | `chN.present_count` (N=0..6), `all.present_count` | **yes** | `grep -rn "pc_count(" --include='*.hpp' --include='*.cpp' src/` → 6 lines; the one external call is in `Canvas::per_channel_reading`, `case Reading::PresentCount` |
| `t7::pc_length(const PlayheadReadout&, const WagonReadout&)` | present+window pitch classes, weighted by beats | `chN.window_length`, `all.window_length`; also feeds all 16 DFT rows | **yes** | `grep -rn "pc_length(" --include='*.hpp' --include='*.cpp' src/` → 9 lines; the one external call is in `Canvas::per_channel_reading`, `case Reading::WindowLength` |
| `t7::pc_length(const PlayheadReadout&)` | present pitch classes, by beats | contributes the present half of every `*.window_length` row and therefore of every DFT row | **yes** (transitively) | same 9-line grep: `src/musical/pc_count.hpp:92` is `PitchClassVector v = pc_length(ph);`, inside the body of `t7::pc_length(const PlayheadReadout&, const WagonReadout&)`, which is wired |
| `t7::pc_length(const WagonReadout&)` | window pitch classes, by beats | contributes the window half of every `*.window_length` row and therefore of every DFT row | **yes** (transitively) | same 9-line grep: `src/musical/pc_count.hpp:93` is `const PitchClassVector w = pc_length(wg);`, same enclosing symbol |
| `t7::pc_onset(const PlayheadReadout&, const WagonReadout&, float)` | note-on impulses in `(onset_prev_beat_, beat]`, velocity-weighted | `chN.onset` (N=0..6) | **yes** | `grep -rn "pc_onset(" --include='*.hpp' --include='*.cpp' src/` → 2 lines: the definition in `pc_count.hpp` and the sole call in `Canvas::per_channel_reading`, `case Reading::Onset` |
| `t7::current_note(const Spine&)` | one-hot at the line holder's pitch class | `chN.current_pc`, `all.current_pc` | **yes** | `grep -rn "current_note(" --include='*.hpp' --include='*.cpp' src/` → 2 lines: definition in `spine_ops.hpp`, sole call in `Canvas::per_channel_reading`, `case Reading::CurrentPC` |
| `t7::line_distance(const Spine&)` | signed registral semitones, previous → holder | `chN.distance` (N=0..6) | **yes** | `grep -rn "line_distance(" --include='*.hpp' --include='*.cpp' src/` → 2 lines: definition in `spine_ops.hpp`, sole call in `Canvas::write_reading`, `case Reading::Distance` |
| `t7::pc_dft(const PitchClassVector&)` | 12-point real DFT, families f1..f6, L1-normalized mag + phase | `chN.dft_mag`, `chN.dft_phase`, `all.dft_mag`, `all.dft_phase` | **yes** | `grep -rn "pc_dft(" --include='*.hpp' --include='*.cpp' src/` → 9 lines: definition in `pc_dft.hpp`, one call in `Canvas::write_reading`, and 7 matched lines in the unbuilt `check_pc_dft.cpp` carrying **8** call sites (line 74 is `const PcDft da = pc_dft(a), db = pc_dft(b);` — two calls on one line) |
| `t7::present_set(const PlayheadReadout&, const WagonReadout&)` | binary presence over present+window | (none directly) — feeds `all.field` | **yes** | `grep -rn "present_set" --include='*.hpp' --include='*.cpp' src/` → 7 lines (the pattern is a substring, so it also matches `union_present_set`); the true call is `Canvas::union_present_set`; the remaining two hits are in the unbuilt `probe_canvas.cpp` |
| `t7::present_union(const PitchClassVector*, int)` | presence-OR across source channels | (none directly) — feeds `all.field` | **yes** | its **own** recipe — `grep -rn "present_union" --include='*.hpp' --include='*.cpp' src/` → 10 lines: definition in `musical_ops.hpp`, one call in `Canvas::union_present_set`, plus comments and 4 call sites in the unbuilt `check_field_union.cpp` / `probe_canvas.cpp` |
| `t7::to_degrees(const PitchClassVector&, int)` | re-origin onto the root (D) | all vector rows + the field's input | **yes** | `grep -rn "to_degrees" --include='*.hpp' --include='*.cpp' src/` → 12 lines: definition in `musical_ops.hpp`, one call in `t7::dress` (`vector_dressing.hpp`), one call in `Canvas::step_fields`, 4 in the unbuilt `check_field_union.cpp`, rest comments |
| `t7::dress(const PitchClassVector&, const VectorDressing&)` | the published dressing (re-origin to D, no scale) | all 12-wide and 6-wide vector rows | **yes** | `grep -rn "dress(" --include='*.hpp' src/analysis src/musical` → 3 lines: definition in `vector_dressing.hpp` and two calls, both inside `Canvas::write_reading` |
| `t7::HeldField::step` | held-field cascade (bootstrap / strict-beat move / hold) | `all.field` | **yes** | `grep -n "\.step(" src/analysis/canvas_1/canvas.hpp` → 1 line, `p.field.step(degrees, bank(), BANK_SIZE);`, inside `Canvas::step_fields` |
| `t7::elect_field(const PitchClassVector&, const Field*, int)` | max-overlap election with hierarchy tiebreak | `all.field` (transitively) | **yes** (via `HeldField::step`) | `grep -rn "elect_field" --include='*.hpp' --include='*.cpp' src/` → 5 lines: definition in `field.hpp`, one call inside `t7::HeldField::step` (same file), 3 direct call sites only in the unbuilt `check_field_union.cpp` |
| `t7::field_overlap(const PitchClassVector&, const PitchClassVector&)` | shared-degree count | `all.field` (transitively) | **yes** (via `elect_field` / `HeldField::step`) | its **own** recipe — `grep -rn "field_overlap" --include='*.hpp' --include='*.cpp' src/` → 4 lines, all in `src/musical/field.hpp`: the definition, two calls inside `t7::elect_field`, one call inside `t7::HeldField::step` |
| `t7::field_mask(std::initializer_list<int>)` | a field's root-relative degree mask | the bank behind `all.field` | **yes** | `grep -rn "field_mask" --include='*.hpp' src/musical/` → the definition in `field.hpp`; `grep -n "field_mask(" src/analysis/canvas_1/canvas.hpp` → 6 lines, all inside `Canvas::bank()` |
| `t7::canvas_1::Canvas::field_index(const HeldField&)` | 1-based rank; silence reads 1, never 0 | `all.field` | **yes** | present in `Canvas::write_reading`, `case Reading::Field` (quoted above) |
| `t7::canvas_1::Canvas::write_vector` | writes a 12-vector into a band's slot run | all 12-wide rows | **yes** | `grep -n "write_vector(" src/analysis/canvas_1/canvas.hpp` → 2 lines: one call inside `Canvas::write_reading`, one definition |
| `t7::canvas_1::Canvas::first_source` | first active channel of a mask (per-voice scalars) | `chN.distance` | **yes** | `grep -n "first_source(" src/analysis/canvas_1/canvas.hpp` → 2 lines: one call inside `Canvas::write_reading` (`case Reading::Distance`), one definition |
| `t7::pc_count(const WagonReadout&)` | window pitch classes, by count | would serve `Reading::WindowCount` — **unpublished** | **no** | `grep -rn "pc_count(" --include='*.hpp' --include='*.cpp' src/` → it **does** have one caller, `src/musical/pc_count.hpp:61` inside `t7::pc_count(const PlayheadReadout&, const WagonReadout&)`; the verdict is transitive unreachability, not absence of a caller |
| `t7::pc_count(const PlayheadReadout&, const WagonReadout&)` | present+window, by count | would serve `Reading::WindowCount` — **unpublished** | **no** | same grep; **zero** call sites in `src/`. This is the symbol that severs the branch above it |
| `t7::pc_set(const PitchClassVector&)` | support of a count vector, as a `PitchClassBits` set | none | **no** | `grep -rn "pc_set" --include='*.hpp' --include='*.cpp' src/` → 2 lines only: its definition and one doc-comment line in `pc_count.hpp`'s header block. **Zero call sites anywhere in `src/`** |
| `Reading::PresentLength` (slot 12, width 12) | — | none; refused by `Canvas::writer_wired` | **no** | `writer_wired` (quoted §4.1) omits it; `reading_spec` still reserves slot 12 |
| `Reading::WindowCount` (slot 24, width 12) | — | none; refused by `Canvas::writer_wired` | **no** | as above; slot 24 reserved, never written |
| `Reading::Polyphony` (slot 62, width 1) | present voice count | none; refused by `Canvas::writer_wired`, **and no producer function exists** | **no** | `grep -rn "Polyphony" --include='*.hpp' --include='*.cpp' src/` → **3** lines, and not `canvas.hpp`-only: `src/cartridges/the_board/realization/state.hpp:609` (a section-divider comment, "─── Polyphony-driven band motion ───", unrelated to the stat), `canvas.hpp:179` (the enumerator in the `Reading` enum), `canvas.hpp:357` (the `reading_spec` case). The grep is case-sensitive and therefore does **not** match `SLOT_POLYPHONY`; a separate `grep -rn "SLOT_POLYPHONY" --include='*.hpp' --include='*.cpp' src/` returns 2 lines, the constant and its use in `reading_spec`. No `per_channel_reading` case, no `write_reading` case, no symbol in `src/musical/` computes it |
| `t7::canvas_1::Canvas::on_input(const InputEvent&)` | — (empty override) | none | **n/a** (defined, body empty by design) | `grep -rn "on_input" --include='*.hpp' --include='*.cpp' src/` → 11 lines. `Canvas::on_input` (`canvas.hpp`, with the header's own note "on_input() is unused, the canvas's source being the DAW alone") has **no caller**; the only dispatch is `src/the_board.cpp:302` `app->render.on_input(event);`, which targets `t7::RenderCartridge::on_input` (`src/render/render_cartridge.hpp`), overridden in `src/cartridges/the_board/cartridge.hpp`. The pure virtual `t7::AnalysisCartridge::on_input` is declared but never dispatched |

**Summary of (a) — corrected count.** **19** analyzer symbols are wired on the publish
path, counting the two single-argument `pc_length` overloads that the section's own
definition of "wired" reaches transitively. **Two** symbols have zero call sites
anywhere in `src/`: `t7::pc_count(const PlayheadReadout&, const WagonReadout&)` and
`t7::pc_set(const PitchClassVector&)`. **One** further symbol,
`t7::pc_count(const WagonReadout&)`, is reachable only from that dead two-argument
form and is therefore also off the path. **Three** `Reading` enumerators
(`PresentLength`, `WindowCount`, `Polyphony`) hold reserved canonical slots but are
refused by `writer_wired` and therefore never enter a layout; `Polyphony` is the only
one of the three with **no producer function anywhere in `src/`**. `Canvas::on_input`
is defined with an empty body and dispatched by nothing.

The 19 wired symbols, enumerated so the count is checkable: `pc_count(ph)`,
`pc_length(ph,wg)`, `pc_length(ph)`, `pc_length(wg)`, `pc_onset`, `current_note`,
`line_distance`, `pc_dft`, `present_set`, `present_union`, `to_degrees`, `dress`,
`HeldField::step`, `elect_field`, `field_overlap`, `field_mask`,
`Canvas::field_index`, `Canvas::write_vector`, `Canvas::first_source`.

#### 4.4.1 What calls the publish path at all — a bounding fact

```
grep -rn "canvas_1/canvas.hpp" --include='*.cpp' --include='*.hpp' src/
    → (no output)

grep -rn '#include "canvas.hpp"' src/analysis/canvas_1/
    → src/analysis/canvas_1/check_canvas_union.cpp:11:#include "canvas.hpp"
    → src/analysis/canvas_1/check_canvas_compound.cpp:10:#include "canvas.hpp"
    → src/analysis/canvas_1/probe_canvas.cpp:28:#include "canvas.hpp"

grep -rn "canvas\.initialize(\|canvas\.update(" src/analysis/canvas_1/
    → probe_canvas.cpp:156:    canvas.initialize(/*asset_path*/ nullptr);
    → probe_canvas.cpp:180:        canvas.update(dt);
```

`t7::canvas_1::Canvas` is reachable **only** from those three `.cpp` files, all
inside `src/analysis/canvas_1/`, and none of them is in a build target (§4.6).
`Canvas::initialize` and `Canvas::update` — the composition that registers the 55
names and the frame that fills them — have exactly one caller each, both in
`probe_canvas.cpp`. `Canvas::advance` / `route` / `configure` / `publish_reading`
are additionally reached from the two `check_canvas_*` witnesses, which drive the
frame directly with their own (smaller, different) compositions.

`src/the_board.cpp` (blob `588174ecddb0d68388e39a9025d6eda2f2afd000`) binds a
different analysis side entirely. **Disclosed elision (R5):** the two fragments below
are *not* contiguous and are *not* in the same scope. The first is a data member of
`struct App`; the second is a statement inside `static bool init_world()`, which
begins after `struct App` closes. Between them lie the rest of `App`'s members, its
closing brace, the file-scope `static App* app = nullptr;`, and the opening of
`init_world` — 41 lines in all. They are quoted here as two separate scopes, not one
region:

```cpp
// — member of struct App —
    t7::BeatClock clock;
```

```cpp
// — statement inside static bool init_world() —
    app->render.bind_signal_layout(app->clock.stat_layout());
```

and `t7::BeatClock::stat_layout` (in `src/analysis/beat_clock.hpp`, blob
`b10038ff5069783c6be15e1e8d885d36238f7354`) is:

```cpp
    StatLayoutView stat_layout() const { return StatLayoutView{ nullptr, 0 }; }
```

**FLAG — the whole 55-name layout is unreachable from the shipping binary.**
The only `add_executable` in the root `CMakeLists.txt` is `the_board`, whose
sources are `src/the_board.cpp`, `src/external/RtMidi.cpp` and the header glob
`src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp`. Nothing in that TU set includes
`analysis/canvas_1/canvas.hpp`. Resolving this further — i.e. whether the linker
would drop it or whether a preset variant pulls it in — would require running
CMake, which BUILD forbids; the include-graph grep above is the static bound.

---

### 4.5 (b) READER-LESS STATS

#### The reader recipe

Each recipe below is transcribed with the **exact** number of lines the published
command produces, and every claim is anchored to an enclosing symbol rather than to a
line number.

```
# R-a — every SignalLayout::resolve call site in src/
grep -rn "signal_layout_\.resolve(" --include='*.hpp' --include='*.cpp' src/
    → src/musical/signal_layout.hpp:16://   SourceBinding src = signal_layout_.resolve("abbott.lowest_pc");
    → src/coupling/visual_canvas.hpp:284:            fog_field_ = signal_layout_.resolve("all.field");
    → src/coupling/visual_canvas.hpp:296:                voice_playhead_ = signal_layout_.resolve((v + ".present_count").c_str());
    → src/coupling/visual_canvas.hpp:298:            room_wagon_ = signal_layout_.resolve("all.window_length");
    → src/coupling/visual_canvas.hpp:299:            room_playhead_ = signal_layout_.resolve("all.present_count");
    → src/coupling/visual_canvas.hpp:315:                checker_win_ = signal_layout_.resolve((v + ".window_length").c_str());
    → src/coupling/visual_canvas.hpp:337:                    signal_layout_.resolve((v + ".onset").c_str());
    (7 lines. The first is a USAGE doc comment in signal_layout.hpp's header
     block — quoted in §4.1 — not a call site. The other 6 are the real call
     sites, and all 6 lie inside t7::VisualCanvas::bind: `void bind(` opens at
     visual_canvas.hpp:275 and `void tick(` opens at :366, so 284..337 is
     wholly within bind.)
```

**Amendment note:** the earlier draft of this recipe reported 6 output lines. The
command produces 7. The *conclusion* — 6 call sites, all in `t7::VisualCanvas::bind`
— is unchanged; only the transcript was wrong.

```
# R-b — every AnalysisSignal::stat( read in src/
grep -rn "\.stat(" --include='*.hpp' --include='*.cpp' src/
    → src/analysis/canvas_1/canvas.hpp:473
    → src/analysis/canvas_1/canvas.hpp:477
    → src/analysis/canvas_1/check_canvas_union.cpp:41
    → src/analysis/canvas_1/check_canvas_compound.cpp:29
    → src/analysis/canvas_1/probe_canvas.cpp:90
    → src/analysis/canvas_1/probe_canvas.cpp:95
    → src/coupling/visual_canvas.hpp:377
    → src/coupling/visual_canvas.hpp:403
    → src/coupling/visual_canvas.hpp:450
    → src/coupling/visual_canvas.hpp:477
    → src/coupling/visual_canvas.hpp:505
    → src/coupling/visual_canvas.hpp:570
    (12 lines, in that order. Grouped by enclosing symbol:
       · 2 in t7::canvas_1::Canvas::publish — the elided
         INSTRUMENTS.zoetrope_witness stderr block, `output_.stat(v, SLOT_ONSET + i)`
       · 1 in `field_of(const Canvas&)` in the unbuilt check_canvas_union.cpp
       · 1 in `bin(const Canvas&, const char*, int)` in the unbuilt
         check_canvas_compound.cpp
       · 2 in `print_layout(const Canvas&)` in the unbuilt probe_canvas.cpp
       · 6 in t7::VisualCanvas::tick — the whole render-side read surface)
```

```
# R-c — the three constants that expand the templated names
grep -rn "RIBBON_VOICE\|CHECKER_VOICE\|ZOETROPE_EARS" --include='*.hpp' src/
    → src/coupling/visual_canvas.hpp:137:    inline constexpr const char* RIBBON_VOICE = "ch1";   // live prefix verified: chN (canvas_1 NAME_* tables)
    → src/coupling/visual_canvas.hpp:143:    inline constexpr uint32_t ZOETROPE_EARS = 0b0111'1111u;
    → src/coupling/visual_canvas.hpp:201:    inline constexpr const char* CHECKER_VOICE = "ch1";   // the chordal piano; chN = wire = Ableton − 1
    → src/coupling/visual_canvas.hpp:295:                std::string v(RIBBON_VOICE);
    → src/coupling/visual_canvas.hpp:314:                std::string v(CHECKER_VOICE);
    → src/coupling/visual_canvas.hpp:330:            // set bit of ZOETROPE_EARS. A miss warns and disables that ear —
    → src/coupling/visual_canvas.hpp:334:                if (!(ZOETROPE_EARS & (1u << ch))) continue;
    → src/coupling/visual_canvas.hpp:347:                    bound, zoetrope_ear_count_, ZOETROPE_EARS);
    → src/coupling/visual_canvas.hpp:600:        SourceBinding voice_playhead_{};   // "<RIBBON_VOICE>.present_count" — the chord's sounding set
    → src/coupling/visual_canvas.hpp:616:        SourceBinding checker_win_{};         // "<CHECKER_VOICE>.window_length" — the 12-pc length vector
    → src/coupling/visual_canvas.hpp:628:        SourceBinding zoetrope_ears_[8]{};    // "chN.onset" per set bit of ZOETROPE_EARS
    (11 lines: 3 definitions, 4 uses inside t7::VisualCanvas::bind, 1 comment
     inside bind, 3 member-declaration comments in the private section.)
```

**Amendment note:** the earlier draft transcribed R-c as 3 lines and truncated the
three definition lines. The command produces 11 lines, and the two `const char*`
definitions carry trailing comments that were dropped. `ZOETROPE_EARS` has **no**
trailing comment on its own line; its explanation sits in the four-line block above
it, quoted here so the value's intent is on the record:

```cpp
    // ── The zoetrope's ears ── a listener SET, not a voice: bit N =
    // wire chN listens. DIAGNOSTIC WIDE: the screen hears the whole
    // composition. Narrow to a set once the pipe is proven — {ch6}
    // (0b0100'0000u) was the ruling; wire = Ableton − 1.
    inline constexpr uint32_t ZOETROPE_EARS = 0b0111'1111u;
```

The constants' values reproduce as `RIBBON_VOICE = "ch1"`, `CHECKER_VOICE = "ch1"`,
`ZOETROPE_EARS = 0b0111'1111u` (bits 0..6 set → `ch0`..`ch6`), and the derived count
of 12 distinct reader names below is unaffected.

`src/coupling/visual_canvas.hpp` is therefore the **sole** render-side reader file
in `src/`. The names resolved in `t7::VisualCanvas::bind` (quoted §4.5.1) expand to
exactly **12** distinct stat names:

| resolved name | binding member | consumed in |
| --- | --- | --- |
| `all.field` | `fog_field_` | `t7::VisualCanvas::tick` |
| `ch1.present_count` (`RIBBON_VOICE + ".present_count"`) | `voice_playhead_` | `t7::VisualCanvas::tick` |
| `all.window_length` | `room_wagon_` | `t7::VisualCanvas::tick` |
| `all.present_count` | `room_playhead_` | `t7::VisualCanvas::tick` |
| `ch1.window_length` (`CHECKER_VOICE + ".window_length"`) | `checker_win_` | `t7::VisualCanvas::tick` |
| `ch0.onset` … `ch6.onset` (7 names, one per set bit of `ZOETROPE_EARS`) | `zoetrope_ears_[0..6]` | `t7::VisualCanvas::tick` |

This 12 is independently corroborated by `src/analysis/beat_clock.hpp`'s own
header comment, quoted verbatim (blob `b10038ff5069783c6be15e1e8d885d36238f7354`):

```cpp
// The empty layout is the audio socket. The render side resolves 12
// live source names against it — all.field, ch1.present_count,
// all.window_length, all.present_count, ch1.window_length,
// ch0.onset .. ch6.onset — and every resolve misses and disables its
// coupling via the graceful path (musical/signal_layout.hpp
// resolve(): one stderr warn, valid=false). A future browser-side
// audio source plugs into this socket by publishing exactly those
// names through a real StatLayoutView.
```

Note that this comment says "one stderr warn"; §4.1's un-elided quote of
`SignalLayout::resolve` shows that warn is inside `#ifndef NDEBUG`, so it is a
debug-configuration behaviour.

#### The reader-less set: 55 − 12 = **43**

| family | reader-less members | n |
| --- | --- | --- |
| `chN.current_pc` | `ch0` `ch1` `ch2` `ch3` `ch4` `ch5` `ch6` | 7 |
| `chN.distance` | `ch0` `ch1` `ch2` `ch3` `ch4` `ch5` `ch6` | 7 |
| `chN.dft_mag` | `ch0` `ch1` `ch2` `ch3` `ch4` `ch5` `ch6` | 7 |
| `chN.dft_phase` | `ch0` `ch1` `ch2` `ch3` `ch4` `ch5` `ch6` | 7 |
| `chN.present_count` | `ch0` `ch2` `ch3` `ch4` `ch5` `ch6` (`ch1` **is** read) | 6 |
| `chN.window_length` | `ch0` `ch2` `ch3` `ch4` `ch5` `ch6` (`ch1` **is** read) | 6 |
| `chN.onset` | — (all seven are read) | 0 |
| group | `all.current_pc`, `all.dft_mag`, `all.dft_phase` | 3 |
| **total** | | **43** |

The three group readings with readers are `all.field`, `all.present_count`,
`all.window_length`. 43 + 12 = 55 ✓.

Notable shape of the gap: **the entire pc-DFT stratum (16 rows: 7×`dft_mag`,
7×`dft_phase`, `all.dft_mag`, `all.dft_phase`) has no reader anywhere in `src/`.**
So does the entire spine-derived stratum: no reader resolves any `*.current_pc`
name (8 published) or any `*.distance` name (7 published).

Direction of the answer to *"can the source side feed what the target side will
ask for"*: the source side (`Canvas`) publishes a strict **superset** — all 12
names the reader side asks for are present in the 55, at matching widths:

| reader asks | width the reader indexes | canvas publishes | count | match |
| --- | --- | --- | --- | --- |
| `all.field` | scalar, `base` only | band 7, slot 61 | 1 | ✓ |
| `ch1.present_count` | loops `i < 12` | band 1, slot 0 | 12 | ✓ |
| `all.window_length` | loops `i < 12` | band 7, slot 36 | 12 | ✓ |
| `all.present_count` | loops `i < 12` | band 7, slot 0 | 12 | ✓ |
| `ch1.window_length` | loops `i < 12` | band 1, slot 36 | 12 | ✓ |
| `ch0.onset`..`ch6.onset` | loops `i < 12` | bands 0..6, slot 76 | 12 | ✓ |

**Width evidence, anchored by symbol (R3).** All six reads live in
`t7::VisualCanvas::tick`. Five of them are `signal.stat(<binding>.channel,
<binding>.base + i)` inside a `for (int i = 0; i < 12; ++i)` loop; the sixth reads a
bare `base`. Recipe:
`grep -rn "\.stat(" --include='*.hpp' src/coupling/visual_canvas.hpp` for the reads,
then read the enclosing statement of each. Verbatim, one fragment per binding:

```cpp
// fog_field_ — scalar, bare base, no loop
                const int f = (int)signal.stat(fog_field_.channel, fog_field_.base);
```
```cpp
// voice_playhead_ — 12-wide
                for (int i = 0; i < 12; ++i)
                    if (signal.stat(voice_playhead_.channel,
                        voice_playhead_.base + i) > 0.0f)
                        mask |= (1u << i);
```
```cpp
// room_wagon_ — 12-wide
                for (int i = 0; i < 12; ++i) {
                    const float w = signal.stat(room_wagon_.channel, room_wagon_.base + i);
```
```cpp
// room_playhead_ — 12-wide
                if (room_playhead_.valid)
                    for (int i = 0; i < 12; ++i)
                        room_sounding += signal.stat(room_playhead_.channel,
                            room_playhead_.base + i);
```
```cpp
// checker_win_ — 12-wide
                    for (int i = 0; i < 12; ++i) {
                        const float w = signal.stat(checker_win_.channel,
                            checker_win_.base + i);
```
```cpp
// zoetrope_ears_[e] — 12-wide, per ear
            for (int e = 0; e < zoetrope_ear_count_; ++e) {
                const SourceBinding& ear = zoetrope_ears_[e];
                if (!ear.valid) continue;
                for (int i = 0; i < 12; ++i) {
                    const float w = signal.stat(ear.channel, ear.base + i);
```

**No name the reader side asks for is missing from the source side's 55.** The
gap is entirely in the other direction (43 published-but-unread) and in the
binding (§4.4.1: the layout actually bound at runtime is `BeatClock`'s
`{nullptr, 0}`).

#### 4.5.1 The resolve block, verbatim (`t7::VisualCanvas::bind`)

Four fenced fragments, each contiguous in the file; the material between them is
`param_layout_.resolve` calls, `Segment` initialisation, and comments — none of it a
`signal_layout_.resolve`.

```cpp
            // fog: the held field → a density and a tint, each a DEVIATION from
            // the anchor row (ATMOS_1); index 0 is the anchor, so the Segments
            // start at 0 — no deviation yet.
            fog_field_ = signal_layout_.resolve("all.field");
```
```cpp
            // ribbon sources (the casting sheet): the voice's Playhead drives
            // the sustain swell; the room's Wagon aims the tint's hue; the
            // room's Playhead gates the tint's mix.
            {
                std::string v(RIBBON_VOICE);
                voice_playhead_ = signal_layout_.resolve((v + ".present_count").c_str());
            }
            room_wagon_ = signal_layout_.resolve("all.window_length");
            room_playhead_ = signal_layout_.resolve("all.present_count");
```
```cpp
            // CHECKER-REBUILD source + targets (the terrain's checker voice):
            // the voice's WINDOW pc-length vector becomes the resultant color;
            // presence + distinct-pc count envelope the pull and the spread.
            {
                std::string v(CHECKER_VOICE);
                checker_win_ = signal_layout_.resolve((v + ".window_length").c_str());
            }
```
```cpp
            // zoetrope ears (the listener set): one "chN.onset" resolve per
            // set bit of ZOETROPE_EARS. A miss warns and disables that ear —
            // the resolver's own semantics; the deaf ear simply never sums.
            zoetrope_ear_count_ = 0;
            for (int ch = 0; ch < 8; ++ch) {
                if (!(ZOETROPE_EARS & (1u << ch))) continue;
                std::string v("ch" + std::to_string(ch));
                zoetrope_ears_[zoetrope_ear_count_++] =
                    signal_layout_.resolve((v + ".onset").c_str());
            }
```

---

### 4.6 (c) THE WITNESSES

#### Build decision, one command

```
grep -nE 'check_canvas_compound|check_canvas_union|check_field_union|check_pc_dft|probe_canvas' CMakeLists.txt
    → (no output; exit status 1)
```

Corroborating recipes, each transcribed as the command actually prints it:

```
grep -nE "add_executable|add_library|add_test|target_sources" CMakeLists.txt
    → 692:add_executable(the_board
    (1 line — the only target in the file)

grep -n "GLOB" CMakeLists.txt
    → 45:get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    → 688:file(GLOB_RECURSE T7_RENDER_HEADERS
    → 841:    file(GLOB T7_SDK_DXC_DIRS
    (3 lines. Line 688 prints WITHOUT its path argument — the `file(GLOB_RECURSE …)`
     call is split across three physical lines, so the glob pattern is on the next
     line and grep never shows it.)
```

**Amendment note:** the earlier draft presented line 688 as
`file(GLOB_RECURSE T7_RENDER_HEADERS "src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp")`
— a merge of three source lines into one, presented as grep output. It is not what
that command prints. The pattern is recovered instead with a range read:

```
sed -n '686,700p' CMakeLists.txt
```

```cmake
# Only the active cartridge's headers → scoped IntelliSense
file(GLOB_RECURSE T7_RENDER_HEADERS
    "src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp"
)

add_executable(the_board
    src/the_board.cpp
    # The one other translation unit: RtMidi's Windows MM backend, the
    # canvas's route to the DAW's virtual port. Vendored, not header-only,
    # so it compiles separately; __WINDOWS_MM__ below selects its backend.
    src/external/RtMidi.cpp
    ${T7_RENDER_HEADERS}
)
```

The substantive conclusion is unchanged: the glob is confined to
`src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp`, matches only `*.hpp`, and sweeps in
no `src/analysis` source of any kind.

```
grep -n "analysis\|canvas_1" CMakeLists.txt
    → 221:dawn_lib_optional(LIB_TINT_LANG_CORE_IR_ANALYSIS "src/tint/$<CONFIG>/tint_lang_core_ir_analysis.lib")
    → 262:dawn_lib(LIB_TINT_LANG_SPIRV_WRITER_ANALYSIS "src/tint/$<CONFIG>/tint_lang_spirv_writer_analysis.lib")   # N6 ADD
    → 612:# The analysis side is the BeatClock (src/analysis/beat_clock.hpp,
    → 613:# CUT_1c) — no analysis cartridge selection exists anymore.
    (4 lines: two Dawn/Tint .lib names, unrelated; and the two-line comment
     recording that no analysis cartridge selection exists.)
```

`add_executable(the_board …)` names exactly three source entries:
`src/the_board.cpp`, `src/external/RtMidi.cpp`, and `${T7_RENDER_HEADERS}` — a glob
confined to `src/cartridges/the_board/*.hpp`. No `.cpp` under `src/analysis/`
appears in any target, and no glob can sweep one in.

There is also **no CMakeLists.txt anywhere below the root**, and the root adds no
subdirectory:

```
find /home/user/7T-Music -name CMakeLists.txt -not -path "*/third_party/*" -not -path "*/.git/*"
    → /home/user/7T-Music/CMakeLists.txt          (the root file, and nothing else)

grep -n "add_subdirectory" CMakeLists.txt
    → (no output; exit status 1)
```

So the root file is the whole build description: **no target anywhere in this
repository compiles any of the five files.**

#### Include census across the five witnesses

Recipe:
`for f in check_canvas_compound check_canvas_union check_field_union check_pc_dft probe_canvas; do echo "--- $f ---"; grep -n '#include' src/analysis/canvas_1/$f.cpp; done`

| file | project headers included | standard headers |
| --- | --- | --- |
| `check_canvas_compound.cpp` | `"canvas.hpp"`, `"sources/midi_event.hpp"` | `<cstdio>` `<cassert>` `<cmath>` `<string_view>` |
| `check_canvas_union.cpp` | `"canvas.hpp"`, `"sources/midi_event.hpp"` | `<cstdio>` `<cassert>` `<string_view>` |
| `check_field_union.cpp` | `"musical/field.hpp"` only | `<cstdio>` `<cassert>` `<initializer_list>` |
| `check_pc_dft.cpp` | `"musical/pc_dft.hpp"` only | `<cassert>` `<cmath>` `<cstdio>` |
| `probe_canvas.cpp` | `"canvas.hpp"`, `"sources/midi_event.hpp"` | `<chrono>` `<cstdio>` `<iostream>` `<string>` `<thread>` |

**Two of the five witnesses do not include `canvas.hpp`** — `check_field_union.cpp`
and `check_pc_dft.cpp`. This is consistent with the §4.4.1 recipe
`grep -rn '#include "canvas.hpp"' src/analysis/canvas_1/`, which returns exactly three
hits.

#### The table

| file | asserts (headline) | in root CMakeLists.txt? (`grep -nE 'check_canvas_compound\|check_canvas_union\|check_field_union\|check_pc_dft\|probe_canvas' CMakeLists.txt`) | target name | verdict |
| --- | --- | --- | --- | --- |
| `src/analysis/canvas_1/check_canvas_compound.cpp` | per-voice readings and their additive compounds; group-band placement; spine gating | **no** (exit 1, no match) | — | **UNREACHABLE** |
| `src/analysis/canvas_1/check_canvas_union.cpp` | the union field elects across voices and holds; canonical placement; opt-in refusal on availability **and** on write-gating | **no** (exit 1, no match) | — | **UNREACHABLE** |
| `src/analysis/canvas_1/check_field_union.cpp` | `present_union` is a presence join with identity + idempotence, and feeds `elect_field`/`HeldField::step` unchanged | **no** (exit 1, no match) | — | **UNREACHABLE** |
| `src/analysis/canvas_1/check_pc_dft.cpp` | `pc_dft` family arithmetic: single-pc uniform, whole-tone→f6, triad→f3, diatonic→f5, zero rest, transposition invariance, phase range | **no** (exit 1, no match) | — | **UNREACHABLE** |
| `src/analysis/canvas_1/probe_canvas.cpp` | nothing (no `assert`; `grep -c "assert" src/analysis/canvas_1/probe_canvas.cpp` → 0); it is an interactive `main()` loop printing bindings, layout and published values against live loopMIDI | **no** (exit 1, no match) | — | **UNREACHABLE** |

All five: **present in the tree, absent from the build.** None is compiled by any
target; none can run; none gates anything.

#### Per-file prose

**`check_canvas_compound.cpp`** — a `main()` over a three-voice composition
(`setup(Canvas&)` configures slots 0,1,2 from `default_spec(v, 4.0f)` with
`s.crossings.active = true`). It publishes `ch0.current_pc`, `ch0.window_length`,
`ch2.current_pc` per-voice and `all.field`, `all.current_pc`, `all.window_length`
over `Canvas::Source::group({0,1,2})`. Its helpers are `find(const Canvas&, const char*)`
(a linear walk of `Canvas::stat_layout()`) and `bin(const Canvas&, const char*, int)`
(reads `Canvas::output().stat`). It asserts: per-voice `current_pc` is a one-hot at
each voice's line note (`bin(cv,"ch0.current_pc",0) == 1.0f`,
`bin(cv,"ch2.current_pc",4) == 1.0f`); the compound `all.current_pc` is the
element-wise **vector sum**, i.e. a per-pc voice count (`== 2.0f` at D, `== 1.0f`
at F#); `window_length` is ~1 beat per voice and sums to 2 beats at D across two
voices; placement — `all.current_pc` and `all.field` land in band `MAX_CHANNELS - 1`
at slots 48 and 61, `ch0.current_pc` in band 0 at slot 48; and that a bare
`Canvas` with the spine off refuses `Reading::CurrentPC`
(`assert(!bare.publish_reading(...))`). Its vector-sum assertion is the witness form
of the statement in `Canvas::write_reading`'s vector-reading comment ("the additive
compound is just the sum", §4.4). It includes `canvas.hpp`. It is **UNREACHABLE**:
the grep against `CMakeLists.txt` returns nothing, and `add_executable(the_board …)`
names no `src/analysis` source.

**`check_canvas_union.cpp`** — a `main()` over the two-voice union field. Its
`setup(Canvas&)` configures slots 0 and 1 from `default_spec(v, 4.0f)` — **spine
off** — and publishes exactly one reading, `Reading::Field` over
`Canvas::Source::group({0,1})`, under the bare name `"field"`. Helpers `find` and
`field_of(const Canvas&)`. Five blocks assert: (1) silence reads field index 1,
the top of the hierarchy (`Canvas::field_index` never returns 0); (2) the union
elects across both voices and a note on voice 1 moves a field voice 0 established
(Mixolydian rank 2 → Major rank 3), and the incumbent then **holds** through aged
silence; (3) placement — `g->channel == MAX_CHANNELS - 1`, `g->slot_base == 61`,
`g->count == 1`, `g->shape == StatShape::Scalar`, no `ch0.field` / `ch1.field` /
`ch0.present_count`, and `cv.stat_layout().count == 1`; (4) availability-binding —
`Reading::CurrentPC` is refused because the spec leaves the spine off, and the
contract is unchanged; (5) write-gating — `Reading::PresentCount` is refused.
**Recorded gap:** block (5)'s comment reads *"present_count is available (the
present is always there) but unwired this round"*, and its body is

```cpp
        Canvas cv; setup(cv);
        const bool refused = !cv.publish_reading(Canvas::Reading::PresentCount,
                                                 Canvas::Source::channel(0), "ch0.present_count");
        assert(refused);
```

but `Canvas::writer_wired` at this commit returns `true` for `Reading::PresentCount`
(quoted §4.1) and `Canvas::available` returns `true` for a present-only reading on an
active channel, so `publish_reading` would return `true` and `refused` would be
`false`. This is recorded as an observed contradiction between the witness's
assertion and the header it tests; it is not detected today because the file is not
compiled. It includes `canvas.hpp`. **UNREACHABLE**.

**`check_field_union.cpp`** — one of the **two** witnesses that do **not** include
`canvas.hpp` (the other is `check_pc_dft.cpp`); among project headers it includes
`musical/field.hpp` alone. It re-declares the canvas's six-field bank inline as
`static const Field BANK[6]` (`phrygian_dominant`, `mixolydian`, `major`, `dorian`,
`harmonic_minor`, `lydian_sharp2` — the same six masks `Canvas::bank()` builds) and
`static constexpr int D = 2`, a hand-copy of `Canvas::PROJECT_PC_ORIGIN`. Helpers
`pcs`, `print_set`, `expect_set`. It asserts two claims: (1) `present_union` is a
presence-OR — `{D,F#} ∪ {A,C} = {0,2,6,9}` — with identity (`a ∪ ∅ = a`) and
idempotence (`a ∪ a = a`); (2) the union feeds `elect_field` unchanged and resolves
what neither voice does — `FieldChoice::tie > 1` for each voice alone, `eu.index == 1`
(mixolydian) and `eu.tie == 1` for the union, and `HeldField::step` bootstraps to
that index. Because it names no `Canvas` symbol, it would still compile if
`canvas.hpp` were absent. It holds 4 of the 10 `present_union` grep hits and 3 of the
5 `elect_field` hits from §4.4's evidence commands. **UNREACHABLE**.

**`check_pc_dft.cpp`** — the pure-function gate for `t7::pc_dft`, and the **second**
witness that does not include `canvas.hpp`. Among project headers it includes
`musical/pc_dft.hpp` alone; no canvas, no port, no RtMidi (stated in its own header
comment). Helper `near(float,float,float)`. Six blocks assert: a single pitch
class gives every family magnitude exactly 1; the whole-tone cluster
`{0,2,4,6,8,10}` puts all energy in f6 and none in f1..f5; the C major triad
`{0,4,7}` peaks at **f3** with `mag[2] == √5 / 3` (the file's comment records
this as a correction against the handoff's f5 sketch); the diatonic scale
`{0,2,4,5,7,9,11}` peaks at **f5** with every other family below half; the zero
vector rests at mags 0 **and phases 0** (the pure half of the REST — the
hold-last half lives in `Canvas::Published::held_phase`, not here); magnitudes are
transposition-invariant; and phases stay within `[−π, π]` over twelve seeded
vectors. Its f3/f5/f6 findings are the witness form of the `SLOT_DFT_MAG`
continuation comment in §4.1 ("f3 triadicity · f5 fifths/diatonic · f6 whole-tone").
It carries **8** `pc_dft` call sites on 7 grep-matched lines — the transposition
block's line is `const PcDft da = pc_dft(a), db = pc_dft(b);`, two calls on one line.
**UNREACHABLE**.

**`probe_canvas.cpp`** — not an assertion witness at all: `grep -c "assert"` returns
0. It is an interactive DAW-synced lab. `main()` constructs a `Canvas`, calls
`Canvas::initialize(nullptr)` (which composes the voices, publishes the readings,
and opens `loopMIDI`), prints each active slot's binding via
`print_binding(const Canvas&, int)`, prints the published slot map via
`print_layout(const Canvas&)`, bails with exit 1 if `Canvas::is_open()` is false,
then loops forever: wall-clock `dt` → `Canvas::update(dt)` → on a transition
detected by `note_changed(const Canvas&)` (reading `PlayheadReadout::onset_count`
/ `release_count`) it prints `print_published`, `present_notes` per active slot,
and `field_input` (which rebuilds the field's input with the same
`present_set` / `present_union` pair the canvas uses). **Recorded gap:** its
header comment describes canvas_1 verbatim as

```cpp
// canvas_1's composition: two voices — slot 0 <- MIDI 0, slot 1 <- MIDI 1 —
// each a present and a four-beat window, the spine off. It publishes ONE
// reading: the field, taken across the UNION of both voices, in the group band.
```

while `Canvas::initialize` at this commit composes `VOICES = 7` with
`s.crossings.active = true` and publishes 55 readings (§4.2). The same header block
also says "Whatever the layout advertises is printed, so a reading added later
appears here without changing this probe" — and indeed `print_layout` /
`print_published` walk whatever the layout advertises, so the code itself does not
depend on the stale count; only the comment does. It includes `canvas.hpp` and is
the sole caller of `Canvas::initialize` and `Canvas::update` in the tree (§4.4.1).
**UNREACHABLE**.

---

### 4.7 FLAGS

**FLAG-1 — `STAT_LAYOUT` does not exist as a symbol.** The unit's premise that
`signal_layout.hpp` and `analysis_signal.hpp` enumerate registered stat names is
not met by the tree: both are contract-only headers. The 55 names are registered
imperatively by `t7::canvas_1::Canvas::publish_reading` from
`t7::canvas_1::Canvas::initialize`, in `src/analysis/canvas_1/canvas.hpp` (blob
`250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5`). The table in §4.3 is derived from
there. Resolving this differently would have cost nothing more; recording it so
the plan does not look for a constant array that isn't there.

**FLAG-2 — the 55 names are statically unreachable from the only build target.**
Nothing in the `the_board` TU set includes `analysis/canvas_1/canvas.hpp`
(§4.4.1). The layout actually bound at runtime is `t7::BeatClock::stat_layout()`,
which returns `StatLayoutView{ nullptr, 0 }`, so all 12 reader-side resolves miss
and `t7::SignalLayout::misses()` reports 12. The per-source stderr warn inside
`SignalLayout::resolve` is gated by `#ifndef NDEBUG` (see the un-elided quote in
§4.1), so in a Release configuration the misses are counted silently. Confirming
any of this dynamically would require running the binary; BUILD forbids it. The
static include-graph grep is the bound.

**FLAG-3 — RESOLVED, no residual uncertainty.** The build-absence claim was
initially scoped to the root file; the sweep has since been run:
`find /home/user/7T-Music -name CMakeLists.txt -not -path "*/third_party/*" -not -path "*/.git/*"`
returns the root file alone, and `grep -n "add_subdirectory" CMakeLists.txt`
returns nothing (exit 1). No unit step was left incomplete.

**FLAG-4 — RESOLVED.** `git rev-parse HEAD:src/coupling/visual_canvas.hpp` =
`ab5a21993a98c5dcc096f38b5db0c16cb7afbf35`, recorded in §4.0 along with twelve
other blobs. The working tree is clean (`git status --porcelain` empty), so the
§4.5.1 fragments equal that blob.

**FLAG-5 — commit drift under the section, recorded not resolved-away.** The branch
tip moved from `79adfa4d` to `6d53388e` while this section was being verified. The
one added path is `docs/LIGATURE_0_RECON.md`. Consequence: any recipe in this report
that sweeps `docs/` will now find the report quoting itself. The only such recipe in
this section is §4.1's `STAT_LAYOUT` census, republished there in commit-pinned form
(`git grep -n "STAT_LAYOUT" 79adfa4d -- src/ tools/ docs/` → 2). All thirteen blobs in
§4.0 are identical at both commits, so no source-derived claim moved. Resolving this
further would mean re-running every recipe in the whole report against `79adfa4d`,
which is outside this section's unit.

**FLAG-6 — slot 63 is unnamed.** The `Canvas` address map jumps from
`SLOT_POLYPHONY = 62` to `SLOT_DFT_MAG = 64`; no constant names slot 63, and no
`Reading` enumerator resolves to it. Recorded as an observed hole in the canonical
address map, not diagnosed.

---

### 4.8 Amendment log

Every change made to this section under adversarial verification, and the check that
justified it. Nothing was removed; the section is longer than before.

| # | what was wrong | check that settled it | outcome |
| --- | --- | --- | --- |
| 1 | `t7::pc_length(const PlayheadReadout&)` and `t7::pc_length(const WagonReadout&)` were marked wired? = **no** | `grep -rn "pc_length(" --include='*.hpp' --include='*.cpp' src/` + reading the body of `t7::pc_length(ph,wg)` in `src/musical/pc_count.hpp` | **Verifier correct.** Both are called from the two-argument form, which is itself on the publish path. Both rows flipped to **yes (transitively)** with the inner-call evidence quoted |
| 2 | `t7::pc_count(const WagonReadout&)` evidence read "this overload has no caller" | same `pc_count(` grep — `pc_count.hpp:61` is `const PitchClassVector w = pc_count(wg);` | **Verifier correct.** It has exactly one caller. The **no** verdict is kept but re-grounded on transitive unreachability |
| 3 | "17 analyzer symbols are wired; 4 overloads plus `pc_set` are defined-but-uncalled" | the three full greps, transcribed in §4.4 | **Verifier correct.** Wired count is **19**; the zero-call-site set is exactly **two** symbols. Summary rewritten and the 19 enumerated |
| 4 | `Canvas::write_reading` quoted with three comment blocks silently dropped | diff of the fenced block against `awk '/^    void write_reading\(Published& p\) \{/,/^    \}$/' src/analysis/canvas_1/canvas.hpp` | **Verifier correct.** Re-quoted complete and un-elided; the four facts the comments carry are itemised |
| 5 | `SourceBinding` / `SignalLayout` quoted with three comment blocks dropped | full read of `src/musical/signal_layout.hpp` | **Verifier correct.** Re-quoted complete, including the `#ifndef NDEBUG` block, which is now load-bearing in FLAG-2 |
| 6 | `SLOT_*` quote dropped six continuation comment lines, truncating two comments mid-sentence | `sed -n '297,313p' src/analysis/canvas_1/canvas.hpp` | **Verifier correct.** All six restored; the REST/HOLD-LAST and aperture statements are now on the record, and the slot-63 hole surfaced (FLAG-6) |
| 7 | §4.1 `stat` / `set_stat` quote normalised a four-space separator line to empty | `grep -n "float stat(int channel" -A6 src/analysis/analysis_signal.hpp \| cat -A` → `    $` | **Verifier correct.** Restored, with a note that renderers may trim it |
| 8 | R-a transcribed as 6 output lines | re-ran the published command | **Verifier correct.** 7 lines; the 7th is the USAGE doc comment in `signal_layout.hpp`. Conclusion (6 call sites, all in `t7::VisualCanvas::bind`) unchanged and now anchored to the `bind` / `tick` boundary |
| 9 | R-c transcribed as 3 lines, with the definition lines truncated | re-ran the published command | **Verifier correct.** 11 lines; trailing comments restored; `ZOETROPE_EARS` has no trailing comment, so its four-line explanatory block is quoted instead |
| 10 | `Polyphony` grep described as `canvas.hpp`-only, and credited with a `SLOT_POLYPHONY` hit | re-ran the published command | **Verifier correct.** 3 lines including `src/cartridges/the_board/realization/state.hpp`; the case-sensitive pattern never matches `SLOT_POLYPHONY`, so a second recipe is published for it. Substantive verdict unchanged |
| 11 | "`check_field_union.cpp` — the only witness that does not include `canvas.hpp`" | include census across all five witnesses | **Verifier correct.** Two witnesses lack it. A full include table is now published, and both per-file paragraphs say so |
| 12 | `grep -n "GLOB" CMakeLists.txt` transcribed with line 688's path argument inlined | re-ran the command, then `sed -n '686,700p' CMakeLists.txt` | **Verifier correct.** 3 lines; 688 alone is `file(GLOB_RECURSE T7_RENDER_HEADERS`. The range read is published separately. Conclusion unchanged |
| 13 | "7 call sites in the unbuilt `check_pc_dft.cpp`" | `grep -rn "pc_dft(" …` + reading the transposition block | **Verifier correct.** 8 call sites on 7 grep-matched lines |
| 14 | §4.5 width evidence anchored entirely to bare line numbers (R3) | located `void bind(` and `void tick(` and read each loop | **Verifier correct.** Rewritten as six verbatim fragments, all attributed to `t7::VisualCanvas::tick`; five loops are `for (int i = 0; i < 12; ++i)` and `fog_field_` is a bare `base` |
| 15 | §4.4.1 joined a member of `struct App` and a statement in `init_world()` across a bare `…` (R5) | `grep -n "BeatClock clock\|bind_signal_layout\|^struct App" src/the_board.cpp` — 151 and 192, in different scopes | **Verifier correct.** Split into two separately-labelled fragments with the 41-line span characterised |
| 16 | Section header pinned a commit that is no longer `HEAD` | `git rev-parse HEAD`, `git diff --stat 79adfa4d HEAD`, and a 13-blob SHA comparison at both commits | **Verifier correct.** Drift recorded at the top and in FLAG-5; §4.0 now carries both columns; the one commit-sensitive recipe is pinned |
| 17 | §4.4's `present_union` row cited "same grep" as `present_set`, and `field_overlap` cited "same grep" as `elect_field` — neither pattern produces the other's hits (R2) | ran each pattern separately | **Found during amendment, not by the verifier.** Both rows now carry their own recipe and line count (10 and 4) |
| 18 | §4.4's `on_input` row summarised an 11-line grep as two clauses (R2) | re-ran `grep -rn "on_input" --include='*.hpp' --include='*.cpp' src/` | **Found during amendment.** Line count published and the dispatch target named by symbol (`t7::RenderCartridge::on_input`) |

No CERTAIN or LIKELY finding from the verifier was rejected. No claim in this
section was found to contradict the tree on re-check.

## 5. Slice A — fog.density (continuous pipe)

Walked at HEAD `79adfa4d26c9e17e0074692928f1d2875d7edde1`, branch
`claude/ligature-0-recon-hcrix0`. Read-only; no file under `/home/user/7T-Music`
was written by this unit.

**FLAG — the repository HEAD moved after this section was first walked, by a
hand that is not this unit's.** At amendment time `git rev-parse HEAD` returns
`6d53388e83f4a5cd7ad3b154484c885f567a02da`, whose parent is the `79adfa4d…`
named above. `git diff --name-status 79adfa4d HEAD` → a single row,
`A	docs/LIGATURE_0_RECON.md`; `git diff --stat 79adfa4d HEAD -- src/` is empty;
`git status --porcelain` is empty. Every path this section cites carries the
same blob at both revisions — verified by running `git rev-parse 79adfa4d:<path>`
and `git rev-parse HEAD:<path>` over all 28 cited paths and comparing, all 28
identical. The new commit's added file embeds this section's own text, which is
why several of the code blocks below now also match a second file in the tree.
Every verdict, recipe and SHA below therefore holds unchanged at both `79adfa4d`
and at `6d53388e`. I cannot attribute the commit; I record its existence because
it is a write inside the repository that post-dates the stated walk point.

### 5.0 Method, names, and anchors

**The name.** The literal string `"fog.density"` exists and is the real pipe
name on the visual (target) side. Census recipe, with its literal output:

```
$ rg -n '"fog\.density"' src/
src/coupling/visual_canvas.hpp:58://   auto d = visual_canvas_.layout().resolve("fog.density");   // entity, once
src/coupling/visual_canvas.hpp:233:        { "fog.density",          0,    1,    0.0f },   // deviation from the anchor (ATMOS_1)
src/coupling/visual_canvas.hpp:285:            fog_density_ = param_layout_.resolve("fog.density");
src/cartridges/the_board/cartridge.hpp:250:            TargetBinding fog_density_dst_{};   // resolved "fog.density" pipe
src/cartridges/the_board/cartridge.hpp:828:                fog_density_dst_ = visual_canvas_.layout().resolve("fog.density");
```

5 occurrences, 2 files. By enclosing symbol (R3): the first is prose in the
header's USAGE block; the second is a row of `t7::PARAM_LAYOUT`; the third is
inside `t7::VisualCanvas::bind`; the fourth is the member declaration
`the_board::…::fog_density_dst_`; the fifth is inside
`the_board::bind_signal_layout`. No alternate spelling exists.

**The pipe has two halves and they are NOT the same name.** The slice as named
in the unit conflates them; the walk below keeps them apart:

| half | the name | who resolves it | against what |
| --- | --- | --- | --- |
| SOURCE (music → canvas) | `"all.field"` | `SignalLayout::resolve` in `VisualCanvas::bind` | the analysis cartridge's `StatLayoutView` |
| TARGET (canvas → GPU) | `"fog.density"` | `ParamLayout::resolve` in `VisualCanvas::bind` and again in `the_board::bind_signal_layout` | `PARAM_LAYOUT` (a static constexpr table in the same header) |

`"fog.density"` is a *bank slot name*, not a music name. It is resolved against
a compile-time table that ships in the same translation unit, so it can never
miss. The music name on this pipe is `"all.field"`. Census recipe, with its
literal output:

```
$ rg -n '"all\.field"' src/
src/coupling/visual_canvas.hpp:28:// "all.field", is already published, so the analysis side is untouched.
src/coupling/visual_canvas.hpp:284:            fog_field_ = signal_layout_.resolve("all.field");
src/analysis/canvas_1/check_canvas_compound.cpp:46:    cv.publish_reading(Canvas::Reading::Field,        all, "all.field");
src/analysis/canvas_1/check_canvas_compound.cpp:83:    const StatGroup* gf = find(cv, "all.field");
src/analysis/canvas_1/canvas.hpp:138:        publish_reading(Reading::Field,        all, "all.field");
```

5 occurrences, 3 files. By enclosing symbol: prose in the header banner of
`visual_canvas.hpp`; `t7::VisualCanvas::bind`; the test publish and the test
lookup in `check_canvas_compound.cpp` (`main`); and
`t7::canvas_1::Canvas::initialize`.

**Translation-unit boundary (the reachability rule used for every STALE
verdict below).** The whole program is one target:

```
$ rg -c 'add_executable|add_library' CMakeLists.txt
1
```
`CMakeLists.txt` (blob `2dddc9202f4d74650e28f95b3aa536ddb81cda9a`), symbol
`add_executable(the_board ...)`:

```cmake
add_executable(the_board
    src/the_board.cpp
    # The one other translation unit: RtMidi's Windows MM backend, the
    # canvas's route to the DAW's virtual port. Vendored, not header-only,
    # so it compiles separately; __WINDOWS_MM__ below selects its backend.
    src/external/RtMidi.cpp
    ${T7_RENDER_HEADERS}
)
```

`T7_RENDER_HEADERS` is `file(GLOB_RECURSE ... "src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp")`
— headers listed for IDE indexing, not compiled as TUs. So the compiled TU set
is exactly **two**: `src/the_board.cpp` and `src/external/RtMidi.cpp`.

Nothing in either TU's include closure names `analysis/canvas_1/canvas.hpp`:

```
$ rg -l 'canvas_1/canvas.hpp' src/
(no output)
```

and `sources/midi_port.hpp` has exactly one includer, which is that file:

```
$ rg -l 'include "sources/midi_port.hpp"' src/
src/analysis/canvas_1/canvas.hpp
```

Therefore hops 1–5 of the source half are **in the tree but in no translation
unit**. That is the basis for every STALE verdict below.

**Anchors (R6).** Every SHA below was produced by `git rev-parse HEAD:<path>`
and re-checked with `git rev-parse 79adfa4d:<path>`; all pairs match.

| path | blob SHA |
| --- | --- |
| `CMakeLists.txt` | `2dddc9202f4d74650e28f95b3aa536ddb81cda9a` |
| `src/the_board.cpp` | `588174ecddb0d68388e39a9025d6eda2f2afd000` |
| `src/sources/midi_event.hpp` | `b7c41853c298a1ff111445b508d58f9940615830` |
| `src/sources/midi_port.hpp` | `293ce7c46f669185235e2e6121d48f0a9863563e` |
| `src/musical/midi_stream.hpp` | `f74d0b039cb6d630fe0f5e25b6c7067e2d9b18bf` |
| `src/musical/stream_data.hpp` | `d84086d83e06113e5284caa2aa6dc24b304cbd5f` |
| `src/musical/field.hpp` | `5e1f38d74dedcf8b32f5cd69a03b46808fd8115f` |
| `src/musical/wagon.hpp` | `f7b091df7971971d53c2796ea61f4554e8952205` |
| `src/musical/playhead.hpp` | `3172fe6cb587d3deed968db71611d64d39eb44af` |
| `src/musical/context.hpp` | `a4c3977e08b4bf8638b6072077995a095667ad5f` |
| `src/musical/signal_layout.hpp` | `8e2e84312483e31e429276d91c23f7d63dc2643c` |
| `src/analysis/analysis_signal.hpp` | `d088796d0ece785b9e34ee071273d6c5df7ce4a4` |
| `src/analysis/analysis_cartridge.hpp` | `f07e11a992d2381413b303f6741ddf32fe8205ab` |
| `src/analysis/beat_clock.hpp` | `b10038ff5069783c6be15e1e8d885d36238f7354` |
| `src/analysis/canvas_1/canvas.hpp` | `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5` |
| `src/analysis/canvas_1/probe_canvas.cpp` | `7a341e9c00a0b8bd1965714df87c533d30a6cced` |
| `src/coupling/visual_canvas.hpp` | `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35` |
| `src/coupling/visual_params.hpp` | `c196529d08b9b815a4b4282e7dc2695661febbc5` |
| `src/coupling/canvas_surface.hpp` | `336d5d320f3a0337a78568f5762eae5885022109` |
| `src/coupling/trajectory.hpp` | `a156425a5cc235d7a591fdd4c1150477406f86ce` |
| `src/cartridges/the_board/cartridge.hpp` | `3651bcabaa0b02a2925ad6868ce541ea9ab1b202` |
| `src/cartridges/the_board/contracts/driver_surface.hpp` | `b0e478dba4d458b4d96c7fee0be04cd393bc58fb` |
| `src/cartridges/the_board/realization/state.hpp` | `fe4bce836b4665588ed43a0729d312e89cd05a20` |
| `src/cartridges/the_board/realization/world.wgsl` | `5b36243dc6b45e27271d0d73eca8a01eb5dc2078` |
| `src/console/organ_registry.hpp` | `70d09e9602eb0f763a616da5303e14c34e7f44da` |
| `src/console/organ_params.inc` | `b426ac4f2b88f89b02f9a9d2236d14b992d93c7f` |
| `tools/gates/score/run.py` | `4263f26af86fbb6b1fb2587a8b3cc1eec3a9bc1a` |
| `docs/reference/RELEASE_CONSOLE.md` | `b12e77adafdc97501106c4bec55d13aee9da11aa` |

**Hop 5 path correction.** The unit names `src/analysis/signal_layout.hpp`.
That path does not exist:

```
$ git rev-parse HEAD:src/analysis/signal_layout.hpp
fatal: path 'src/analysis/signal_layout.hpp' does not exist in 'HEAD'
$ git ls-tree -r HEAD --name-only | grep -i signal_layout
src/musical/signal_layout.hpp
```
The file lives at **`src/musical/signal_layout.hpp`** and is walked there.

**Elision convention used below (R5).** Where a fenced block is not
byte-identical to its source file, the omission is marked with a bare `...`
token inside the fence AND stated in prose immediately beneath it, naming the
recipe that prints the missing text. Every omission in this section removes
comment prose only; no code statement anywhere below is altered, reordered, or
abridged. After this amendment exactly **four** blocks carry an elision, each
marked in the fence and stated in prose with the recipe that prints the missing
text: Hop 5a (`StatLayoutView`'s comment), Hop 5c (`src/the_board.cpp`'s bind
comment), Hop 6 (the PORT_4c witness comment), and Hop 8 (the
`VisualCanvas::tick` fog comment). Three blocks that the first draft elided —
`SignalLayout::resolve` at 5b and the `FOG_BY_FIELD` block at Hop 8, both
**without any marker at all**, and `Canvas::initialize` in The break, which
carried two in-fence `...` tokens plus one unmarked omission — are quoted in
full below instead of re-elided.

**Verdict summary.**

| hop | name | verdict |
| --- | --- | --- |
| 1 | MIDI event ingress (`MidiEvent`, `MidiPort`) | **STALE** |
| 2 | `stream_data` (`ActiveSet`, `CompletedRing`, `MidiStream::receive`) | **STALE** |
| 3 | `field` (`HeldField::step`, `elect_field`) | **STALE** |
| 4 | wagon / playhead (`WagonReadout`, `PlayheadReadout`, `present_set`) | **STALE** |
| 5 | stat slot in `AnalysisSignal` (`"all.field"` at `SLOT_FIELD`) | **STALE** (compound; sub-verdicts 5a PRESENT, 5b PRESENT, 5c PRESENT-and-empty, 5d STALE) |
| 6 | `visual_canvas_.bind` | **PRESENT** |
| 7 | `layout().resolve("fog.density")` | **PRESENT** |
| 8 | per-frame flush (`phase_motion_drivers`) | **PRESENT** |
| 9 | setter symbol (`GPUState::set_fog`) | **PRESENT** |
| 10 | GPU config in `state.hpp` (`GPUDesignConfig::fog_density`) | **PRESENT** |
| 11 | field in `world.wgsl` (`DesignConfig.fog_density`) | **PRESENT** |
| 12 | use site in the shader (`shade_lit`) | **PRESENT** |

---

### Hop 1 — MIDI event ingress

**File:** `/home/user/7T-Music/src/sources/midi_event.hpp`
**Blob SHA:** `b7c41853c298a1ff111445b508d58f9940615830`
**Enclosing symbol:** `t7::MidiEvent`

```cpp
namespace t7 {

struct MidiEvent {
    enum Type : uint8_t { 
        NOTE_ON, 
        NOTE_OFF 
    };
    
    Type type;
    uint8_t channel;
    uint8_t pitch;
    uint8_t _pad;
    float velocity;  // 0-1 for NOTE_ON, ignored for NOTE_OFF
    float beat;      // When this event occurred
    
    // ── Factory Methods ──────────────────────────────────────────
    
    static MidiEvent note_on(int channel, int pitch, float velocity, float beat) {
        MidiEvent e;
        e.type = NOTE_ON;
        e.channel = static_cast<uint8_t>(channel);
        e.pitch = static_cast<uint8_t>(pitch);
        e._pad = 0;
        e.velocity = velocity;
        e.beat = beat;
        return e;
    }
    
    static MidiEvent note_off(int channel, int pitch, float beat) {
        MidiEvent e;
        e.type = NOTE_OFF;
        e.channel = static_cast<uint8_t>(channel);
        e.pitch = static_cast<uint8_t>(pitch);
        e._pad = 0;
        e.velocity = 0.0f;
        e.beat = beat;
        return e;
    }
};

static_assert(sizeof(MidiEvent) == 12, "MidiEvent should be 12 bytes");

} // namespace t7
```

**File:** `/home/user/7T-Music/src/sources/midi_port.hpp`
**Blob SHA:** `293ce7c46f669185235e2e6121d48f0a9863563e`
**Enclosing symbols:** `t7::MidiPort::handle_message`, `t7::MidiPort::push`,
`t7::MidiPort::poll`

```cpp
    void handle_message(double deltatime, const std::vector<unsigned char>& m) {
        // Clock / start / stop / continue / song-position go to the transport.
        if (transport_.feed(deltatime, m)) return;

        // Everything else: the note path, unchanged.
        if (m.size() < 3) return;

        const uint8_t status   = m[0];
        const uint8_t type     = status & 0xF0;
        const uint8_t channel  = status & 0x0F;
        const uint8_t pitch    = m[1];
        const uint8_t velocity = m[2];

        MidiEvent ev;
        if (type == 0x90 && velocity > 0) {
            ev = MidiEvent::note_on(channel, pitch, velocity / 127.0f, 0.0f);
        } else if (type == 0x80 || (type == 0x90 && velocity == 0)) {
            ev = MidiEvent::note_off(channel, pitch, 0.0f);
        } else {
            return;  // CC, pitch bend, aftertouch — ignore
        }
        push(ev);
    }
```

```cpp
    // ── POLL — drain note events, stamp with current_beat ─────────

    int poll(float current_beat, MidiEvent* out, int max_out) {
        int count = 0;
        const uint32_t write = write_idx_.load(std::memory_order_acquire);
        uint32_t read = read_idx_.load(std::memory_order_relaxed);
        while (read != write && count < max_out) {
            out[count] = ring_[read & RING_MASK];
            out[count].beat = current_beat;
            ++read;
            ++count;
        }
        read_idx_.store(read, std::memory_order_release);
        return count;
    }
```

**Status: STALE.** Both definitions exist and every name inside them resolves
(`MidiEvent`, `MidiTransport` from `sources/transport.hpp`, `RtMidiIn` from
`external/RtMidi.h`). What fails is reachability.

Two separate censuses are needed here, and the first draft of this section
crossed them. They are published separately below, each with the output it
actually emits.

*Census A — every mention of the type name `MidiPort` outside its own header.*
`rg` is case-sensitive, so this recipe does not and cannot match the include
directive `#include "sources/midi_port.hpp"` (lower case, underscore):

```
$ rg -n 'MidiPort' src/ | grep -v '^src/sources/midi_port.hpp'
src/analysis/canvas_1/probe_canvas.cpp:24:// Needs RtMidi and the transport-aware MidiPort. In Ableton, enable loopMIDI's
src/analysis/canvas_1/canvas.hpp:668:    MidiPort port_;             // the DAW's MIDI port, owned and drained each frame
```

Exactly two hits, in two files. By enclosing symbol: the first is a prose
comment in the file banner of `src/analysis/canvas_1/probe_canvas.cpp` (blob
`7a341e9c00a0b8bd1965714df87c533d30a6cced`) — a usage note, no declaration; the
second is the private member declaration `t7::canvas_1::Canvas::port_`. There is
therefore exactly **one** `MidiPort` object declared anywhere in the tree, and
it is a member of `Canvas`.

*Census B — every includer of the header.* This is the recipe that produces the
include edge:

```
$ rg -l 'include "sources/midi_port.hpp"' src/
src/analysis/canvas_1/canvas.hpp
```

One includer, and it is the same file that holds the sole declaration. In
`canvas.hpp` the directive sits in the file's include block, between
`#include "sources/midi_event.hpp"` and `#include "analysis/analysis_signal.hpp"`.

Neither file is in a translation unit: `rg -l 'canvas_1/canvas.hpp' src/`
returns nothing, and `probe_canvas.cpp` is in no build target (see the Gaps
section, `rg -n 'check_canvas|probe_canvas|check_field|check_pc_dft|canvas_1'
CMakeLists.txt tools/` → no output, exit 1). The target's TU list is the two
files quoted in §5.0. The built program never constructs a `MidiPort`, never
calls `open_by_name`, never calls `poll`.

**Second-order note (evidence, not proposal):** `src/external/RtMidi.cpp` *is*
compiled into `the_board`, and `e0e22e46` re-added `__WINDOWS_MM__` to
`MSVC_COMPILE_DEFS` so it compiles to a real backend rather than an empty TU.
The backend is therefore built and linked while having no caller in the program.

---

### Hop 2 — stream_data

**File:** `/home/user/7T-Music/src/musical/stream_data.hpp`
**Blob SHA:** `d84086d83e06113e5284caa2aa6dc24b304cbd5f`
**Enclosing symbol:** `t7::ActiveSet`

```cpp
struct ActiveSet {
    std::array<ActiveNote, MIDI_PITCH_COUNT> notes{};
    PitchBitmask mask;
    
    // --- Modification ---
    
    void note_on(int pitch, float velocity, float beat) {
        if (pitch < 0 || pitch >= MIDI_PITCH_COUNT) return;
        notes[pitch].velocity = velocity;
        notes[pitch].onset_beat = beat;
        mask.set(pitch);
    }
    
    /**
     * Deactivate a note. Returns the note that was active (for transfer to CompletedRing).
     * Returns an empty ActiveNote if pitch was not active.
     */
    ActiveNote note_off(int pitch) {
        if (pitch < 0 || pitch >= MIDI_PITCH_COUNT) return ActiveNote{};
        ActiveNote result = notes[pitch];
        notes[pitch].velocity = 0.0f;
        mask.clear(pitch);
        return result;
    }
```

The `MidiEvent` → `ActiveSet`/`CompletedRing` transfer is one hop above, in
`t7::MidiStream::receive` (`/home/user/7T-Music/src/musical/midi_stream.hpp`,
blob `f74d0b039cb6d630fe0f5e25b6c7067e2d9b18bf`):

```cpp
    void receive(const MidiEvent& event) {
        if (event.type == MidiEvent::NOTE_ON) {
            active_.note_on(event.pitch, event.velocity, event.beat);
        } else {
            ActiveNote was_active = active_.note_off(event.pitch);
            if (was_active.is_active()) {
                completed_.push(event.pitch, was_active.velocity, 
                               was_active.onset_beat, event.beat);
            }
        }
    }
```

and is entered from `t7::Context::receive`
(`/home/user/7T-Music/src/musical/context.hpp`, blob
`a4c3977e08b4bf8638b6072077995a095667ad5f`):

```cpp
    void receive(const MidiEvent& ev) {
        stream_.receive(ev);

        const bool is_on = (ev.type == MidiEvent::NOTE_ON);

        if (previous_active_) {
            if (is_on) previous_.on_onset(ev.pitch, ev.velocity, ev.beat);
            else       previous_.on_offset(ev.pitch, ev.beat);
        }
        if (spine_active_) {
            if (is_on) spine_.on_onset(ev.pitch, ev.beat);
```

**Status: STALE.** Every name resolves within the header set (`MidiEvent`,
`ActiveNote`, `CompletedRing`, `PitchBitmask` all present). Unreachable, and the
reachability census is two includers, not one:

```
$ rg -n 'include "musical/context.hpp"' src/
src/musical/context_realize.hpp:32:#include "musical/context.hpp"
src/analysis/canvas_1/canvas.hpp:73:#include "musical/context.hpp"
```

`musical/context.hpp` has exactly two includers: the include directive in
`src/musical/context_realize.hpp`'s own include block, and the one in
`src/analysis/canvas_1/canvas.hpp`'s include block (the directive immediately
above `#include "musical/context_spec.hpp"`). Both chains terminate at the same
place, because `context_realize.hpp` is itself reached only from `canvas.hpp`:

```
$ rg -n 'context_realize' src/
src/analysis/canvas_1/canvas.hpp:66://             musical/context_realize.hpp, musical/pc_count.hpp,
src/analysis/canvas_1/canvas.hpp:75:#include "musical/context_realize.hpp"
src/musical/context_realize.hpp:3:// ─── context_realize.hpp ─────────────────────────────────────────
src/musical/context_realize.hpp:32:#include "musical/context.hpp"
src/coupling/visual_canvas.hpp:69:#include "musical/signal_layout.hpp"
```

(the last row is that same command's match on the word `signal_layout` — see
the next paragraph; it is listed here because the pipeline emits it.) The only
non-`canvas.hpp` includer of `musical/context.hpp` is a header that is itself in
no TU, so the conclusion holds: `context.hpp` reaches a translation unit through
nothing. Census recipe for the whole musical include graph:

```
$ rg -n 'include "musical/' src/ | wc -l
32
$ rg -n 'include "musical/' src/ | grep -v '^src/musical/' | grep -v '^src/analysis/canvas_1/'
src/coupling/visual_canvas.hpp:69:#include "musical/signal_layout.hpp"
```

32 hits total; exactly one comes from outside `src/musical/` and
`src/analysis/canvas_1/`, and it is the `#include "musical/signal_layout.hpp"`
directive in `src/coupling/visual_canvas.hpp`'s include block (hop 5's socket,
which IS live).

---

### Hop 3 — field

**File:** `/home/user/7T-Music/src/musical/field.hpp`
**Blob SHA:** `5e1f38d74dedcf8b32f5cd69a03b46808fd8115f`
**Enclosing symbols:** `t7::present_set`, `t7::elect_field`, `t7::HeldField`

```cpp
// Binary presence over absolute pitch classes (C = 0): 1 if any note of that
// class is sounding now or completed in the window, else 0. Present and window
// are disjoint; OR-ing them cannot double-mark.
inline PitchClassVector present_set(const PlayheadReadout& ph,
                                    const WagonReadout& wg) {
    PitchClassVector s;
    for (int i = 0; i < wg.note_count; ++i)   s.v[wg.notes[i].pitch % 12] = 1.0f;
    for (int i = 0; i < ph.current_count; ++i) s.v[ph.current[i].pitch % 12] = 1.0f;
    return s;
}
```

```cpp
inline FieldChoice elect_field(const PitchClassVector& degrees,
                               const Field* bank, int n) {
    FieldChoice c;
    c.overlap = -1.0f;
    for (int i = 0; i < n; ++i) {
        const float ov = field_overlap(degrees, bank[i].mask);
        if (ov > c.overlap) { c.overlap = ov; c.index = i; }   // strict: earlier wins ties
    }
    for (int i = 0; i < n; ++i)
        if (field_overlap(degrees, bank[i].mask) == c.overlap) ++c.tie;
    return c;
}
```

```cpp
struct HeldField {
    int incumbent = -1;   // -1 = none yet

    int step(const PitchClassVector& degrees, const Field* bank, int n) {
        const FieldChoice pick = elect_field(degrees, bank, n);
        if (pick.overlap <= 0.0f) return incumbent;             // empty / silence: hold
        if (incumbent < 0) { incumbent = pick.index; return incumbent; }   // bootstrap
        const float held = field_overlap(degrees, bank[incumbent].mask);
        if (pick.overlap > held) incumbent = pick.index;        // strictly beaten: move
        return incumbent;                                        // else hold
    }

    bool settled() const { return incumbent >= 0; }
};
```

**Status: STALE.** `HeldField` is the value the whole fog pipe is built to
carry — `FOG_BY_FIELD` in `visual_canvas.hpp` is indexed by exactly this
one-based rank. Every name resolves (`PlayheadReadout`, `WagonReadout`,
`PitchClassVector`, `to_degrees` from `musical/musical_ops.hpp`). Unreachable:

```
$ rg -n 'include "musical/field.hpp"' src/
src/analysis/canvas_1/canvas.hpp:79:#include "musical/field.hpp"
src/analysis/canvas_1/check_field_union.cpp:13:#include "musical/field.hpp"
```

`field.hpp`'s only two includers are the directive in
`src/analysis/canvas_1/canvas.hpp`'s include block (between
`#include "musical/spine_ops.hpp"` and `#include "musical/vector_dressing.hpp"`)
and the single include directive at the head of
`src/analysis/canvas_1/check_field_union.cpp`, immediately above that file's
`<cstdio>` / `<cassert>` pair. Neither is in a TU: `canvas.hpp` has no includer
at all, and `check_field_union.cpp` lost its `add_executable(check_field_union
...)` row in `1a52f2db` and has not regained one.

---

### Hop 4 — wagon / playhead

**File:** `/home/user/7T-Music/src/musical/wagon.hpp`
**Blob SHA:** `f7b091df7971971d53c2796ea61f4554e8952205`
**Enclosing symbol:** `t7::WagonReadout`

```cpp
struct WagonReadout {
    float anchor_beat = 0.0f;    // window end  (current_beat − offset)
    float window_start = 0.0f;   // anchor − span
    float window_end = 0.0f;     // anchor
    float span = 0.0f;
    float offset = 0.0f;

    std::array<WindowNote, WAGON_MAX_NOTES> notes{};
    int  note_count = 0;
    bool overflow = false;       // more than WAGON_MAX_NOTES in the window
    int  entirely_inside_count = 0;
    int  straddling_count = 0;

    bool empty() const { return note_count == 0; }
    bool has_notes() const { return note_count > 0; }
    bool has_overflow() const { return overflow; }

    void clear() {
        anchor_beat = window_start = window_end = span = offset = 0.0f;
        note_count = 0;
        overflow = false;
        entirely_inside_count = straddling_count = 0;
    }
};
```

**File:** `/home/user/7T-Music/src/musical/playhead.hpp`
**Blob SHA:** `3172fe6cb587d3deed968db71611d64d39eb44af`
**Enclosing symbol:** `t7::PlayheadReadout`

```cpp
struct PlayheadReadout {
    float anchor_beat = 0.0f;          // the present moment

    // --- the present ---
    std::array<CurrentNote, PLAYHEAD_MAX_POLYPHONY> current{};
    int          current_count = 0;
    PitchBitmask current_mask;
    bool         current_overflow = false;   // more than MAX_POLYPHONY notes

    // --- transitions this frame (the synchronicity edges) ---
    PitchBitmask onset_mask;
    PitchBitmask release_mask;
    int          onset_count = 0;
    int          release_count = 0;

    // --- gate state ---
    bool  is_onset = false;            // became non-silent this frame
    bool  is_release = false;          // became silent this frame
    float state_duration = 0.0f;       // how long the present state has held

    bool gate()        const { return current_count > 0; }
    bool silent()      const { return current_count == 0; }
    bool has_overflow() const { return current_overflow; }
```

**Status: STALE.** Both readouts exist and are exactly the two arguments
`present_set` (hop 3) takes, so the hop-3↔hop-4 join resolves. Unreachable for
the same reason: both headers reach a TU only through
`musical/context.hpp` → `analysis/canvas_1/canvas.hpp`, and `canvas.hpp` has no
includer.

---

### Hop 5 — stat slot in AnalysisSignal

**Status: STALE (compound).** The hop is not a single artefact. Its container
and its socket are compiled and live; its *writer* — the only thing in the tree
that puts `"all.field"` into a `StatLayoutView` — is in no translation unit, and
what the live program hands the socket is an empty view. The sub-verdicts are
5a **PRESENT**, 5b **PRESENT**, 5c **PRESENT, and empty by construction**, 5d
**STALE**; the hop as a whole is therefore STALE, because the named counterpart
on the next hop (`"all.field"` resolving to a `StatGroup`) does not resolve at
runtime.

#### 5a — The container: `AnalysisSignal` — **PRESENT**

**File:** `/home/user/7T-Music/src/analysis/analysis_signal.hpp`
**Blob SHA:** `d088796d0ece785b9e34ee071273d6c5df7ce4a4`
**Enclosing symbols:** `t7::AnalysisSignal`, `t7::StatGroup`, `t7::StatLayoutView`

```cpp
struct alignas(16) AnalysisSignal {
    // ═══ TIME (16 bytes) ═══════════════════════════════════════════════════
    
    float t_seconds;        // Wall clock time when computed
    float t_beats;          // Musical time when computed
    float dt;               // Frame delta (seconds)
    float _pad0;            // Alignment padding
    
    // ═══ MUSICAL STATS (2048 bytes) ════════════════════════════════════════

    std::array<float, TOTAL_STATS> stats;
```

```cpp
    float stat(int channel, int stat_type) const {
        return stats[stat_index(channel, stat_type)];
    }
    
    void set_stat(int channel, int stat_type, float value) {
        stats[stat_index(channel, stat_type)] = value;
    }
```

```cpp
struct StatGroup {
    const char* name;       // label, e.g. "abbott.pc_histogram"
    int         channel;    // AnalysisSignal channel
    int         slot_base;  // first slot
    int         count;      // number of slots (1 for scalar)
    StatShape   shape;
};

// Non-owning view over a cartridge's STAT_LAYOUT array. ...
struct StatLayoutView {
    const StatGroup* groups;
    uint32_t         count;
};
```

**Elision marked (R5).** The `...` in the comment above `struct StatLayoutView`
stands for four further comment lines. `sed -n '128,132p'
src/analysis/analysis_signal.hpp` prints the whole comment; verbatim, the five
lines are:

```cpp
// Non-owning view over a cartridge's STAT_LAYOUT array. STAT_LAYOUT is
// static constexpr storage, so a pointer into it is valid for the whole
// program — this view is safe to copy and hold. It is the "key" the
// analysis cartridge publishes so the render side can resolve stat
// groups by name at runtime (see musical/signal_layout.hpp).
```

Live: the `#include "analysis/analysis_signal.hpp"` directive in
`src/coupling/visual_canvas.hpp`'s include block (immediately below its
`#include "musical/signal_layout.hpp"`) pulls it in, and `visual_canvas.hpp` is
itself pulled in by the `#include "coupling/visual_canvas.hpp"` directive in
`src/cartridges/the_board/cartridge.hpp`'s include block (the directive sitting
between `bodies/gol_zones.hpp` and `bodies/ribbon.hpp`), which is in
`src/the_board.cpp`'s closure.

#### 5b — The socket: `SignalLayout::resolve` — **PRESENT** (the file is at `src/musical/`, not `src/analysis/`)

**File:** `/home/user/7T-Music/src/musical/signal_layout.hpp`
**Blob SHA:** `8e2e84312483e31e429276d91c23f7d63dc2643c`
**Enclosing symbols:** `t7::SourceBinding`, `t7::SignalLayout::resolve`

```cpp
struct SourceBinding {
    int  channel = 0;
    int  base    = 0;   // = StatGroup slot_base
    int  count   = 0;
    bool valid   = false;
};

class SignalLayout {
public:
    void bind(StatLayoutView v) { view_ = v; misses_ = 0; }
```

The body of `resolve`, now quoted with no elision at all (the first draft
dropped the four `#ifndef NDEBUG` comment lines without a marker; they are
restored here). Recipe that reproduces it:
`grep -n 'SourceBinding resolve' -A 18 src/musical/signal_layout.hpp`.

```cpp
    SourceBinding resolve(std::string_view name) const {
        for (uint32_t i = 0; i < view_.count; ++i) {
            const StatGroup& g = view_.groups[i];
            if (name == g.name) {            // g.name is const char*
                return SourceBinding{ g.channel, g.slot_base, g.count, true };
            }
        }
        ++misses_;
#ifndef NDEBUG
        // Debug twin only (the-board-web-debug): the full list, one line
        // per source, unchanged. NDEBUG is the gate because CMake
        // defines it for Release and not for Debug, which is exactly the
        // two-preset split PORT_2c installed.
        std::fprintf(stderr,
            "[SignalLayout] source '%.*s' not in layout (coupling disabled)\n",
            (int)name.size(), name.data());
#endif
        return SourceBinding{};              // valid = false
    }
```

The restored comment carries one fact of its own that is evidence for the break:
the per-source stderr line is compiled only when `NDEBUG` is undefined, i.e. in
the Debug preset. In a Release build the individual misses print nothing; only
the aggregate line at the tail of `VisualCanvas::bind` (hop 6) survives.

#### 5c — What the live program puts in the socket: `BeatClock::stat_layout` — **PRESENT, and empty by construction**

**File:** `/home/user/7T-Music/src/analysis/beat_clock.hpp`
**Blob SHA:** `b10038ff5069783c6be15e1e8d885d36238f7354`
**Enclosing symbol:** `t7::BeatClock`

```cpp
struct BeatClock {
    float bpm = 100.0f;   // variable BPM — Jean's amendment; one home

    void update(float dt) {
        signal_.dt = dt;
        signal_.t_seconds += dt;
        signal_.t_beats += dt * (bpm / 60.0f);
    }
```

```cpp
    const AnalysisSignal& output() const { return signal_; }

    StatLayoutView stat_layout() const { return StatLayoutView{ nullptr, 0 }; }

private:
```

Its own header states the consequence in prose:

```cpp
// The empty layout is the audio socket. The render side resolves 12
// live source names against it — all.field, ch1.present_count,
// all.window_length, all.present_count, ch1.window_length,
// ch0.onset .. ch6.onset — and every resolve misses and disables its
// coupling via the graceful path (musical/signal_layout.hpp
// resolve(): one stderr warn, valid=false).
```

And this is what `src/the_board.cpp` (blob `588174ecddb0d68388e39a9025d6eda2f2afd000`,
enclosing symbol: the async-ready continuation that runs after
`render.init_renderer`) actually hands over:

```cpp
    // Publish the slot map once. The BeatClock's layout is EMPTY by design
    // ... leaves its coupling disabled — the graceful path in signal_layout.hpp.
    app->render.bind_signal_layout(app->clock.stat_layout());
```

**Elision marked (R5).** The `...` above stands for one comment line.
`sed -n '189,192p' src/the_board.cpp` prints the block unabridged; verbatim:

```cpp
    // Publish the slot map once. The BeatClock's layout is EMPTY by design
    // (CUT_1c): every render-side resolve misses, warns once on stderr, and
    // leaves its coupling disabled — the graceful path in signal_layout.hpp.
    app->render.bind_signal_layout(app->clock.stat_layout());
```

#### 5d — The writer of `"all.field"` at `SLOT_FIELD` — **STALE**

**File:** `/home/user/7T-Music/src/analysis/canvas_1/canvas.hpp`
**Blob SHA:** `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5`
**Enclosing symbol:** `t7::canvas_1::Canvas::initialize`

```cpp
        const Source all = Source::group({0, 1, 2, 3, 4, 5, 6});
        publish_reading(Reading::Field,        all, "all.field");
        publish_reading(Reading::CurrentPC,    all, "all.current_pc");
        publish_reading(Reading::PresentCount, all, "all.present_count");
        publish_reading(Reading::WindowLength, all, "all.window_length");
        publish_reading(Reading::DftMag,       all, "all.dft_mag");
        publish_reading(Reading::DftPhase,     all, "all.dft_phase");

        port_.open_by_name("loopMIDI");   // the DAW's virtual port
```

Enclosing symbol `t7::canvas_1::Canvas::reading_spec` — the slot address:

```cpp
    static constexpr int SLOT_FIELD          = 61;   // 1   held field index 1..6
```
```cpp
            case Reading::Field:         return { SLOT_FIELD,           1, StatShape::Scalar };
```

Enclosing symbol `t7::canvas_1::Canvas::step_fields` — the hop-3 → hop-5 join:

```cpp
    void step_fields() {
        for (int k = 0; k < published_count_; ++k) {
            Published& p = published_[k];
            if (p.reading != Reading::Field) continue;
            const PitchClassVector degrees =
                to_degrees(union_present_set(p.source_mask), PROJECT_PC_ORIGIN);
            p.field.step(degrees, bank(), BANK_SIZE);
        }
    }
```

Enclosing symbol `t7::canvas_1::Canvas::union_present_set` — the hop-4 → hop-3 join:

```cpp
    PitchClassVector union_present_set(uint32_t mask) const {
        PitchClassVector sets[MAX_CHANNELS];
        int n = 0;
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (!(mask & (1u << i)) || !active_[i]) continue;
            sets[n++] = present_set(contexts_[i].playhead(), contexts_[i].wagon(0));
        }
        return present_union(sets, n);
    }
```

Enclosing symbol `t7::canvas_1::Canvas::publish` — the actual `set_stat` write:

```cpp
            case Reading::Field:
                output_.set_stat(p.band, slot, static_cast<float>(field_index(p.field)));
                break;
```

Enclosing symbol `t7::canvas_1::Canvas::field_index` — the one-based rank the
fog table expects:

```cpp
    int field_index(const HeldField& hf) const {
        return (hf.incumbent < 0) ? 1 : hf.incumbent + 1;
    }
```

Enclosing symbol `t7::canvas_1::Canvas::stat_layout` — the non-empty layout that
would satisfy hop 5b:

```cpp
    StatLayoutView stat_layout() const override {
        return StatLayoutView{ layout_.data(), static_cast<uint32_t>(published_count_) };
    }
```

Enclosing symbol `t7::canvas_1::Canvas::update` — the hop-1 → hop-2 drain:

```cpp
    void update(float dt) override {
        dt_         = dt;          // wall-clock delta, telemetry only
        t_seconds_ += dt;
        const float beat = static_cast<float>(port_.beats());   // the DAW's clock
        MidiEvent ev[256];
        const int n = port_.poll(beat, ev, 256);

        for (int i = 0; i < n; ++i) route(ev[i]);
        advance(beat);
    }
```

**Status: STALE.** Every internal name resolves and the class correctly
`override`s `t7::AnalysisCartridge` (`/home/user/7T-Music/src/analysis/analysis_cartridge.hpp`,
blob `f07e11a992d2381413b303f6741ddf32fe8205ab`, which declares
`virtual StatLayoutView stat_layout() const = 0;`). It is unreachable: no
translation unit includes it, and no compiled code names `canvas_1`,
`Canvas`, or `AnalysisCartridge`:

```
$ rg -n 'canvas_1|MidiPort|AnalysisCartridge|ANALYSIS_HEADER|INCUBATE_ANALYSIS' \
     src/the_board.cpp src/cartridges/the_board/cartridge.hpp \
     src/console/console.hpp src/render/render_cartridge.hpp
(no output)
```

---

### Hop 6 — visual_canvas_.bind

**File:** `/home/user/7T-Music/src/coupling/visual_canvas.hpp`
**Blob SHA:** `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35`
**Enclosing symbol:** `t7::VisualCanvas::bind`

```cpp
        void bind(StatLayoutView analysis_layout) {
            param_layout_.bind(ParamLayoutView{ PARAM_LAYOUT, PARAM_LAYOUT_COUNT });
            param_layout_.reset(params_);

            signal_layout_.bind(analysis_layout);

            // fog: the held field → a density and a tint, each a DEVIATION from
            // the anchor row (ATMOS_1); index 0 is the anchor, so the Segments
            // start at 0 — no deviation yet.
            fog_field_ = signal_layout_.resolve("all.field");
            fog_density_ = param_layout_.resolve("fog.density");
            fog_color_ = param_layout_.resolve("fog.color");
            fog_seg_ = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            for (int c = 0; c < 3; ++c)
                fog_color_seg_[c] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
```

The state it fills (enclosing symbol: `t7::VisualCanvas`, private section
"fog coupling state"):

```cpp
        SourceBinding fog_field_{};
        TargetBinding fog_density_{};
        TargetBinding fog_color_{};
        Segment       fog_seg_{};
        Segment       fog_color_seg_[3]{};
```

The call site (enclosing symbol: `the_board::bind_signal_layout`, in
`/home/user/7T-Music/src/cartridges/the_board/cartridge.hpp`, blob
`3651bcabaa0b02a2925ad6868ce541ea9ab1b202`):

```cpp
            void bind_signal_layout(StatLayoutView v) {
                visual_canvas_.bind(v);
                fog_density_dst_ = visual_canvas_.layout().resolve("fog.density");
                fog_color_dst_ = visual_canvas_.layout().resolve("fog.color");
```

**Status: PRESENT.** `bind` is compiled — the `#include "coupling/visual_canvas.hpp"`
directive in `src/cartridges/the_board/cartridge.hpp`'s include block pulls it
in, and `src/the_board.cpp` pulls in `cartridge.hpp` through the macro include
described under the preprocessor FLAG below — and is called once at boot from
`src/the_board.cpp` via `app->render.bind_signal_layout(...)`. The
`param_layout_` half of the call resolves. The
`signal_layout_.resolve("all.field")` half returns `{valid=false}` at runtime —
see The break.

The header's own boot witness for the miss (enclosing symbol:
`t7::VisualCanvas::bind`, tail):

```cpp
            // PORT_4c — THE SOCKET, in one line. Every signal-side
            // resolve above happens here, and with the BeatClock's empty
            // layout (CUT_1c) every one of them misses. ...
            if (signal_layout_.misses() > 0) {
                std::fprintf(stderr,
                    "[SignalLayout] %u sources unbound (no audio source)\n",
                    signal_layout_.misses());
            }
```

**Elision marked (R5).** The `...` stands for four further comment lines.
`grep -n 'PORT_4c' -A 14 src/coupling/visual_canvas.hpp` prints the block
unabridged; verbatim, the comment is:

```cpp
            // PORT_4c — THE SOCKET, in one line. Every signal-side
            // resolve above happens here, and with the BeatClock's empty
            // layout (CUT_1c) every one of them misses. The release twin
            // prints this summary; the debug twin has already printed
            // each source by name. Placed last, after the resolves it
            // counts, beside the Zoetrope witness it deliberately does
            // not replace — that line reports a different fact.
```

The restored text names the two-twin split that hop 5b's `#ifndef NDEBUG` block
implements, and names the `[Zoetrope] ears bound:` line as a distinct witness —
both of which appear together in the recorded transcript quoted at hop 7.

---

### Hop 7 — layout().resolve("fog.density")

**File:** `/home/user/7T-Music/src/coupling/visual_canvas.hpp`
**Blob SHA:** `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35`
**Enclosing symbol:** `t7::PARAM_LAYOUT` (the master control panel table;
namespace-scope inside `namespace t7`, which is the file's only namespace —
`grep -n 'namespace' src/coupling/visual_canvas.hpp` → two lines, the opening
`namespace t7 {` and its closing brace)

```cpp
    //                          name           base count   rest
    inline constexpr ParamSlot PARAM_LAYOUT[] = {
        { "fog.density",          0,    1,    0.0f },   // deviation from the anchor (ATMOS_1)
        { "fog.color",            1,    3,    0.0f },   // per channel, same law
```

**File:** `/home/user/7T-Music/src/coupling/visual_params.hpp`
**Blob SHA:** `c196529d08b9b815a4b4282e7dc2695661febbc5`
**Enclosing symbols:** `t7::TargetBinding`, `t7::ParamLayout::resolve`

```cpp
    struct TargetBinding {
        int  base = 0;
        int  count = 0;
        bool valid = false;
    };
```

```cpp
        TargetBinding resolve(std::string_view name) const {
            for (uint32_t i = 0; i < view_.count; ++i) {
                const ParamSlot& s = view_.slots[i];
                if (name == s.name) {
                    return TargetBinding{ s.base, s.count, true };
                }
            }
            std::fprintf(stderr,
                "[ParamLayout] pipe '%.*s' not in layout (coupling unbound)\n",
                (int)name.size(), name.data());
            return TargetBinding{};
        }
```

Note the asymmetry with hop 5b: `ParamLayout::resolve`'s warn is **not** wrapped
in `#ifndef NDEBUG`, so a target-side miss would print in Release too. It never
prints, because `PARAM_LAYOUT` is in the same TU.

**File:** `/home/user/7T-Music/src/cartridges/the_board/cartridge.hpp`
**Blob SHA:** `3651bcabaa0b02a2925ad6868ce541ea9ab1b202`
**Enclosing symbol:** `the_board::bind_signal_layout` (the boot witness)

```cpp
                std::fprintf(stderr,
                    "[the_board] fog.density base=%d valid=%d | fog.color base=%d count=%d valid=%d\n",
                    fog_density_dst_.base, (int)fog_density_dst_.valid,
                    fog_color_dst_.base, fog_color_dst_.count, (int)fog_color_dst_.valid);
```

**Status: PRESENT.** `"fog.density"` is resolved twice — once inside
`VisualCanvas::bind` into `fog_density_` (the write end) and once in
`bind_signal_layout` into `fog_density_dst_` (the read end). Both resolve
against `PARAM_LAYOUT`, a `constexpr` table in the same TU, so both are
`valid=true` unconditionally.

**Recorded runtime witness — and it is NATIVE, not web-era.**
`/home/user/7T-Music/docs/reference/RELEASE_CONSOLE.md` (blob
`b12e77adafdc97501106c4bec55d13aee9da11aa`) holds **two** specimen transcripts,
not one. Its own header states the framing:

```
> Captured console transcripts, 2026-08 (P4 stamp). The web shell's status
> triggers were designed against these lines (web/index.html). Specimens,
> not current output.
```

Census of the line in question:

```
$ grep -n 'fog.density base' docs/reference/RELEASE_CONSOLE.md
272:(index):28 [the_board] fog.density base=0 valid=1 | fog.color base=1 count=3 valid=1
561:[the_board] fog.density base=0 valid=1 | fog.color base=1 count=3 valid=1
```

Two hits, one per transcript. Which transcript each belongs to, by recipe:

```
$ grep -n 'INCUBATOR DUAL' docs/reference/RELEASE_CONSOLE.md
8:(index):27   INCUBATOR DUAL (web twin — no hot reload)
286:  INCUBATOR DUAL (Hot Reload Enabled)
$ grep -c '(index):' docs/reference/RELEASE_CONSOLE.md
277
$ grep -n '(index):' docs/reference/RELEASE_CONSOLE.md | tail -1
282:printErr @ (index):28
```

Every Emscripten frame prefix in the file (`(index):27` / `(index):28`, and the
`printErr @ (index):28` continuation lines) lies at or before line 282. The
second transcript opens at line 286 and carries no Emscripten frame anywhere;
it is a native Windows/Dawn Release run, evidenced by its own banner block:

```
========================================
  INCUBATOR DUAL (Hot Reload Enabled)
  Clock:    BeatClock
  Render:   the_board
========================================

Warning: loader_get_json: Failed to open JSON file C:\Program Files (x86)\Epic Games\Epic Online Services\managedArtifacts\98bc04bc842e4906993fd6d6644ffb8d\EOSOverlayVkLayer-Win64.json
Warning: loader_get_json: Failed to open JSON file C:\Program Files (x86)\Epic Games\Launcher\Portal\Extras\Overlay\EOSOverlayVkLayer-Win32.json
[Console] Dawn revision: f0bf8ab547a9a23b8b78ff67d8085d4a26600a7d
[Console] Build: Release
[Console] Adapter 0: integrated / D3D12 | Intel(R) HD Graphics 5500 (D3D12 driver version 20.19.15.4703) vendor=intel
[Console] Adapter 1: integrated / D3D11 | Intel(R) HD Graphics 5500 (D3D11 driver version 20.19.15.4703) vendor=intel
[Console] Adapter 2: discrete / D3D12 | NVIDIA GeForce 920M (D3D12 driver version 25.21.14.2531) vendor=nvidia
[Console] Adapter 3: discrete / Vulkan | GeForce 920M (NVIDIA: 425.31 425.31.0.0) vendor=nvidia
```

The witness block for this slice, quoted verbatim from that **native**
transcript (bare lines, no `(index):` prefix, no interleaved `printErr @`):

```
[Zoetrope] ears bound: 0 of 7 (mask 0x7F)
[SignalLayout] 12 sources unbound (no audio source)
[the_board] fog.density base=0 valid=1 | fog.color base=1 count=3 valid=1
[the_board] terrain.checker_mean base=10 count=3 valid=1 | terrain.checker_var base=13 valid=1
```

The same four lines exist in the earlier web-twin transcript, where they carry
the Emscripten framing:

```
(index):28 [Zoetrope] ears bound: 0 of 7 (mask 0x7F)
printErr @ (index):28
(index):28 [SignalLayout] 12 sources unbound (no audio source)
printErr @ (index):28
(index):28 [the_board] fog.density base=0 valid=1 | fog.color base=1 count=3 valid=1
printErr @ (index):28
(index):28 [the_board] terrain.checker_mean base=10 count=3 valid=1 | terrain.checker_var base=13 valid=1
printErr @ (index):28
```

The lines together are the whole slice in one frame: the target half `valid=1`,
the source half among the 12 unbound. The reading is identical in both twins.

**Note (evidence, correcting the first draft of this section).** The first draft
recorded that the only witness was web-era and that no native transcript of
these lines existed in the tree. That is refuted by the file itself: a native
Windows/Dawn Release transcript of exactly these lines DOES exist in
`RELEASE_CONSOLE.md`, and it is the copy quoted above. Both transcripts are
labelled "Specimens, not current output" by the document's own header, so
neither is a capture at this HEAD; the surrounding native transcript also
reports `[GPUState] Design Config: 624 B`, whereas hop 10's `static_assert` at
this HEAD names 720 bytes, which dates that capture before the struct grew.

---

### Hop 8 — per-frame flush

**File:** `/home/user/7T-Music/src/coupling/visual_canvas.hpp`
**Blob SHA:** `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35`
**Enclosing symbol:** `t7::VisualCanvas::tick`

```cpp
        void tick(const AnalysisSignal& signal) {
            const float beat = signal.t_beats;

            // ── fog ──────────────────────────────────────────────────────────────
            // The held field selects a density and an atmospheric tint; the canvas
            // emits each as a DEVIATION from the anchor row (ATMOS_1), and the
            // cartridge's seam composes it over the mood's own rest. ...
            if (fog_field_.valid) {
                const int f = (int)signal.stat(fog_field_.channel, fog_field_.base);
                const int idx = (f >= 0 && f < FOG_FIELD_COUNT) ? f : 0;

                if (fog_density_.valid) {
                    params_.set(fog_density_.base,
                        trajectory_release(fog_seg_, FOG_BY_FIELD[idx] - FOG_BY_FIELD[0],
                                           beat, canvas::CANVAS_LIVE.fog_span));
                }
                if (fog_color_.valid) {
                    for (int c = 0; c < 3; ++c) {
                        params_.set(fog_color_.base + c,
                            trajectory_release(fog_color_seg_[c],
                                FOG_COLOR_BY_FIELD[idx][c] - FOG_COLOR_BY_FIELD[0][c],
                                beat, canvas::CANVAS_LIVE.fog_span));
                    }
                }
            }
```

**Elision marked (R5).** The `...` stands for three further comment lines.
`grep -n 'void tick(const AnalysisSignal' -A 14 src/coupling/visual_canvas.hpp`
prints the block unabridged; verbatim, the comment above `if (fog_field_.valid)`
is:

```cpp
            // ── fog ──────────────────────────────────────────────────────────────
            // The held field selects a density and an atmospheric tint; the canvas
            // emits each as a DEVIATION from the anchor row (ATMOS_1), and the
            // cartridge's seam composes it over the mood's own rest. Segments carry
            // both so they drift across a modulation rather than snapping. One
            // source, two pipes. Decode is a table index — inline, not a goal
            // object.
```

and the preceding function-level comment, two lines above `tick`, reads:

```cpp
        // One frame: run every coupling — read its source, decode inline, carry the
        // value on its Segment, write the bank. No GPU.
```

The table it indexes (enclosing symbol: namespace-scope `t7::FOG_BY_FIELD`),
now quoted with no elision — the first draft dropped a five-line comment here
without a marker, and it is restored. Recipe:
`grep -n 'FOG_FIELD_COUNT' -B 2 -A 16 src/coupling/visual_canvas.hpp`.

```cpp
    // Index 0 is "no field yet" — the value at boot, before any scale is held, not
    // an idle. Tunable.
    inline constexpr int   FOG_FIELD_COUNT = 7;          // index 0 = none, 1..6 fields
    // THE ANCHOR — one home for both rows that wear it. Twinned by the boot
    // config in realization/state.hpp (config_.fog_density / fog_color) and by
    // ATMOS_SUNSET's fog rest in contracts/spine_state.hpp: the mood's rest
    // and the canvas's zero point are the same number by construction, which
    // is what keeps gain 1 on the sunset the pre-ATMOS_1 picture exactly.
    inline constexpr float FOG_DENSITY_NONE  = 0.0030f;
    inline constexpr float FOG_COLOR_NONE[3] = { 0.85f, 0.78f, 0.72f };
    inline constexpr float FOG_BY_FIELD[FOG_FIELD_COUNT] = {
        FOG_DENSITY_NONE,   // 0  none   — no field yet (boot)
        FOG_DENSITY_NONE,   // 1  anchor — the open outdoor atmosphere
        0.0022f,            // 2  light
        0.0026f,            // 3  light
        0.0020f,            // 4  light
        0.0050f,            // 5  dense
        0.0058f,            // 6  dense
    };
```

The restored THE ANCHOR comment is itself evidence for hop 10: it names
`realization/state.hpp`'s `config_.fog_density` / `config_.fog_color` as the
twin of `FOG_DENSITY_NONE` / `FOG_COLOR_NONE`, and hop 10's quoted boot values
(`0.003f`, `{0.85f, 0.78f, 0.72f}`) match those constants exactly.

The colour twin of the same table, immediately below it (enclosing symbol:
namespace-scope `t7::FOG_COLOR_BY_FIELD`):

```cpp
    inline constexpr float FOG_COLOR_BY_FIELD[FOG_FIELD_COUNT][3] = {
        { FOG_COLOR_NONE[0], FOG_COLOR_NONE[1], FOG_COLOR_NONE[2] },   // 0  none   — no field yet
        { FOG_COLOR_NONE[0], FOG_COLOR_NONE[1], FOG_COLOR_NONE[2] },   // 1  anchor — the open outdoor atmosphere
        { 0.78f, 0.80f, 0.82f },   // 2  cool pale
        { 0.80f, 0.82f, 0.76f },   // 3  faint sage
        { 0.74f, 0.78f, 0.86f },   // 4  soft blue
        { 0.92f, 0.72f, 0.55f },   // 5  warm amber
        { 0.70f, 0.68f, 0.80f },   // 6  muted violet
    };
```

**File:** `/home/user/7T-Music/src/cartridges/the_board/cartridge.hpp`
**Blob SHA:** `3651bcabaa0b02a2925ad6868ce541ea9ab1b202`
**Enclosing symbol:** `the_board::phase_motion_drivers`

```cpp
            void phase_motion_drivers(UpdateCtx& c) {
                auto& signal = c.signal;
                visual_canvas_.tick(signal);
                // ORGAN — the drivers' room sits at this seam:
                // out = rest + gain·deviation. The REST is the mood's,
                // drawn per world into mood_state_.fog_rest_* by
                // apply_mood_lighting; the DEVIATION is the canvas's,
                // measured from its anchor row. Gain 1 is the coupling
                // verbatim, gain 0 is the mood's own fog, and with no
                // bindings the rest alone speaks — so the dial works
                // headless too.
                //
                // set_fog GUARDS — it compares all four lanes and dirties
                // only on a change — so both arms call it unconditionally
                // and the silent case costs no dirty.
                {
                    const auto& drv = DRIVER_LIVE.fog;
                    const auto& ms  = mood_state_;
                    if (fog_density_dst_.valid && fog_color_dst_.valid) {
                        const VisualParams& fp = visual_canvas_.params();
                        gpuState_.set_fog(
                            std::max(0.0f, ms.fog_rest_density + drv.gain * fp.get(fog_density_dst_.base)),
                            std::clamp(ms.fog_rest_color[0] + drv.gain * fp.get(fog_color_dst_.base + 0), 0.0f, 1.0f),
                            std::clamp(ms.fog_rest_color[1] + drv.gain * fp.get(fog_color_dst_.base + 1), 0.0f, 1.0f),
                            std::clamp(ms.fog_rest_color[2] + drv.gain * fp.get(fog_color_dst_.base + 2), 0.0f, 1.0f));
                    } else {
                        gpuState_.set_fog(ms.fog_rest_density, ms.fog_rest_color[0],
                                          ms.fog_rest_color[1], ms.fog_rest_color[2]);
                    }
                }
```

**Status: PRESENT.** Both halves compile and run every frame.
`phase_motion_drivers` is a frame-spine phase; the census that names it is the
score gate's phase dictionary in `/home/user/7T-Music/tools/gates/score/run.py`
(blob `4263f26af86fbb6b1fb2587a8b3cc1eec3a9bc1a`), whose entry keyed
`'phase_motion_drivers'` reads
`'the music driver + fog stage — atmosphere foundational (K4)'`. Recipe:
`grep -n 'phase_motion_drivers' tools/gates/score/run.py` → one hit.

The `if (fog_field_.valid)` guard inside `tick` is the gate that is closed at
runtime: `fog_field_` is the `SourceBinding` from `resolve("all.field")`, so
with the empty layout the entire fog decode block is skipped and
`params_.set(fog_density_.base, ...)` never executes. The bank slot therefore
holds its `PARAM_LAYOUT` rest, `0.0f`, forever
(`ParamLayout::reset` laid it there at bind).

The `fog_density_dst_.valid` arm in `phase_motion_drivers` is still taken
(target-side valid=1), so the expression evaluated each frame is
`max(0, ms.fog_rest_density + drv.gain * 0.0f)` = `ms.fog_rest_density`.

**The gain, and its correct enclosing symbol.**
`/home/user/7T-Music/src/cartridges/the_board/contracts/driver_surface.hpp`,
blob `b0e478dba4d458b4d96c7fee0be04cd393bc58fb`. The first draft of this
section named the enclosing symbol `t7::the_board::DRIVER_AUTHORED`. No such
symbol exists anywhere in the tree — `rg -n 'DRIVER_AUTHORED' src/` produces no
output and exits 1. The real enclosing symbol is
**`t7::the_board::DRIVER_TABLE`**, and the value `phase_motion_drivers` actually
reads is the mutable copy `t7::the_board::DRIVER_LIVE`. Recipe:
`rg -n 'DRIVER_TABLE|DRIVER_LIVE' src/`; namespaces confirmed with
`grep -n 'namespace' src/cartridges/the_board/contracts/driver_surface.hpp`
(`t7`, then `the_board`). Quoted verbatim in its surrounding context:

```cpp
// The authored design — the code panel. The fog row carries the gain
// alone: its rests live on the REGIME (Regime.fog_density / .fog_color,
// contracts/spine_state.hpp).
inline constexpr DriverSurface DRIVER_TABLE = {
    { 0.63f },                  // fog: the desk dialled the coupling back —
                                // 0.63 driven, the rest held at the regime's rest
    { 0u, 1.0f, 1.5f, 1.0f },   // aura: intent off, the authored rates.
                                // (It rested ON for one commit, d3b1f6d;
                                // the next desk export put it back off.)
    { { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 1.0f },   // checker: a return to seed
    { 1.0f, 1.0f, { 0.0f, 0.0f, 0.0f }, 0.0f, 1.0f },   // ribbon: the seam's own fallbacks
};

// The live surface — the panel's fourth block and the seams' read.
inline DriverSurface DRIVER_LIVE = DRIVER_TABLE;
```

`0.63f` is therefore multiplied against a constant zero every frame.

---

### Hop 9 — setter symbol

**File:** `/home/user/7T-Music/src/cartridges/the_board/realization/state.hpp`
**Blob SHA:** `fe4bce836b4665588ed43a0729d312e89cd05a20`
**Enclosing symbol:** `t7::the_board::GPUState::set_fog`

```cpp
            void set_fog(float density, float r, float g, float b) {
                if (config_.fog_density != density ||
                    config_.fog_color[0] != r || config_.fog_color[1] != g || config_.fog_color[2] != b) {
                    config_.fog_density = density;
                    config_.fog_color[0] = r; config_.fog_color[1] = g; config_.fog_color[2] = b;
                    configDirty_ = true;
                }
            }
```

**Status: PRESENT.** Called unconditionally each frame from
`phase_motion_drivers` (hop 8, both arms). Guards on change and raises
`configDirty_`, which is what schedules the uniform upload.

Also exposed read-only on the organ panel
(`/home/user/7T-Music/src/console/organ_params.inc`, blob
`b426ac4f2b88f89b02f9a9d2236d14b992d93c7f`). The reach is by two include
directives, cited by symbol: the `#include "console/organ_registry.hpp"`
directive in `src/cartridges/the_board/cartridge.hpp`'s include block (the
directive whose trailing comment reads "the compiled dial registry + its C ABI
(needs the home types above)"), and inside
`/home/user/7T-Music/src/console/organ_registry.hpp` (blob
`70d09e9602eb0f763a616da5303e14c34e7f44da`) the `#include "console/organ_params.inc"`
that forms the body of the array initializer `t7::…::kOrganParams`:

```cpp
inline const OrganParam kOrganParams[] = {
#include "console/organ_params.inc"
};
```

The three fog rows the `.inc` contributes:

```
ORGAN_PARAM(DRIVERS, DriverSurface, fog.gain,         F32,  0.0f, 1.0f,  0.01f,   "Atmosphere · Fog", "drive gain")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fog_density,  F32,  "Atmosphere · Fog", "density (driven)")
ORGAN_PARAM_RO(CONFIG, GPUDesignConfig, fog_color,    VEC3, "Atmosphere · Fog", "colour (driven)")
```

The `DRIVERS` block resolves to `DRIVER_LIVE` — `src/console/organ_registry.hpp`
carries `case ORGAN_BLOCK_DRIVERS:    return &the_board::DRIVER_LIVE;`, which is
the same object hop 8 reads.

---

### Hop 10 — GPU config in state.hpp

**File:** `/home/user/7T-Music/src/cartridges/the_board/realization/state.hpp`
**Blob SHA:** `fe4bce836b4665588ed43a0729d312e89cd05a20`
**Enclosing symbol:** `t7::the_board::GPUDesignConfig`

```cpp
            float sun_direction[3];
            float aura_enabled;               // 0.0 = off, 1.0 = on (guards all aura sampling)
            float pawn_aura_height;           // 0.0 = no aura extrusion, >0 = world units of rise
            float fog_density;                // exponential fog coefficient (default 0.003)
            uint32_t _pad_fog[2];             // ditto
            float fog_color[3];               // fog/sky color RGB
```

Its boot value (enclosing symbol: the `GPUState` config-initialization body
that also sets `mosaic_*` and `fade_*`):

```cpp
                config_.fog_density = 0.003f;
                config_.fog_color[0] = 0.85f;
                config_.fog_color[1] = 0.78f;
                config_.fog_color[2] = 0.72f;
```

These four values are byte-for-byte the `FOG_DENSITY_NONE` / `FOG_COLOR_NONE`
constants quoted at hop 8, which is the twinning the restored THE ANCHOR comment
asserts.

Its layout witnesses (enclosing symbols: file-scope `static_assert`s beside
`GPUDesignConfig`):

```cpp
        static_assert(sizeof(GPUDesignConfig) == 720,
```
```cpp
        static_assert(offsetof(GPUDesignConfig, sun_direction)     % 16 == 0
                   && offsetof(GPUDesignConfig, fog_color)         % 16 == 0
                   && offsetof(GPUDesignConfig, fade_color)        % 16 == 0
                   && offsetof(GPUDesignConfig, checker_resultant) % 16 == 0,
            "every float[3] whose WGSL twin is vec3<f32> must sit on a 16-byte "
            "boundary — WGSL rounds vec3 up to align 16 and C++ does not, so an "
```

**Status: PRESENT.** The field exists, is written by hop 9, and its mirror
contract with WGSL is enforced at compile time.

---

### Hop 11 — field in world.wgsl

**File:** `/home/user/7T-Music/src/cartridges/the_board/realization/world.wgsl`
**Blob SHA:** `5b36243dc6b45e27271d0d73eca8a01eb5dc2078`
**Enclosing symbol:** `struct DesignConfig`

```wgsl
    world_seed: u32,              // master seed for terrain/zone generation
    sun_direction: vec3<f32>,
    aura_enabled: f32,            // 0.0 = off, 1.0 = on (guards all aura sampling)
    pawn_aura_height: f32,
    fog_density: f32,             // exponential fog coefficient (default 0.003)
    fog_color: vec3<f32>,         // fog/sky color RGB
    fade_alpha: f32,              // 0.0 = no overlay, 1.0 = fully opaque
    fade_color: vec3<f32>,        // transition overlay RGB
```

**Status: PRESENT.** Field-for-field twin of hop 10's C++ struct, in the same
declaration order, with the `vec3` alignment that hop 10's `static_assert`
enforces from the other side. Census: `rg -c 'fog_density'
src/cartridges/the_board/realization/world.wgsl` → 2 (this declaration and hop
12's read).

---

### Hop 12 — use site in the shader

**File:** `/home/user/7T-Music/src/cartridges/the_board/realization/world.wgsl`
**Blob SHA:** `5b36243dc6b45e27271d0d73eca8a01eb5dc2078`
**Enclosing symbol:** `fn shade_lit(world_pos: vec3<f32>, normal: vec3<f32>, geo_normal: vec3<f32>, base_color: vec3<f32>, veil_scale: f32) -> vec3<f32>`

```wgsl
    let lit = ambient + sun + points + spot;

    // Fog — the EYE-anchored atmospheric term (a view effect; stays).
    let dist = distance(world_pos, frame_r.camera.pos);
    let fog = 1.0 - exp(-dist * config.fog_density);
    let fogged = mix(lit, config.fog_color, fog);
```

and the function's tail, where `fogged` leaves:

```wgsl
    if (config.veil_dither > 0.5) {
        if (veil_dither_noise(world_pos.xz) < veil) { discard; }
        return fogged;
    }
    return mix(fogged, config.fog_color, veil);
```

**Status: PRESENT.** The census, corrected — the first draft reported 7 hits and
accounted for 3 calls plus 4 comments, dropping the definition line itself. The
recipe returns **8**:

```
$ grep -n 'shade_lit' src/cartridges/the_board/realization/world.wgsl
1729:    //     shade_lit (cosmetic; joins materialize inside it).
4641:fn shade_lit(world_pos: vec3<f32>, normal: vec3<f32>, geo_normal: vec3<f32>, base_color: vec3<f32>, veil_scale: f32) -> vec3<f32> {
5030://   → 9 shade_lit (fog/veil last)
5042:    // Optional dither-dissolve inside the icing band handled in shade_lit.
5235:    var out_colour = shade_lit(in.world_pos, normal, geo_normal, base_color, 1.0);
5695:    return vec4(shade_lit(in.world_pos, n, geo_n, albedo, 1.0), 1.0);
5703:    return vec4(shade_lit(in.world_pos, normalize(in.normal), normalize(in.normal), in.entity_color, 0.0), 1.0);
```

8 hits: **1 definition** (the `fn shade_lit(...)` above), **3 calls**, and
**4 prose comments**. The three calls, with the fragment entry point each sits
in — established by walking back from each call to its nearest preceding
top-level `fn`:

| call | enclosing symbol |
| --- | --- |
| `var out_colour = shade_lit(in.world_pos, normal, geo_normal, base_color, 1.0);` | `fn patch_terrain_fs(in: PatchTerrainVarying) -> @location(0) vec4<f32>` |
| `return vec4(shade_lit(in.world_pos, n, geo_n, albedo, 1.0), 1.0);` | `fn entity_fs(in: EntityVarying) -> @location(0) vec4<f32>` |
| `return vec4(shade_lit(in.world_pos, normalize(in.normal), normalize(in.normal), in.entity_color, 0.0), 1.0);` | `fn ribbon_fs(in: EntityVarying) -> @location(0) vec4<f32>` |

quoted verbatim:

```wgsl
    var out_colour = shade_lit(in.world_pos, normal, geo_normal, base_color, 1.0);
```
```wgsl
    return vec4(shade_lit(in.world_pos, n, geo_n, albedo, 1.0), 1.0);
```
```wgsl
    return vec4(shade_lit(in.world_pos, normalize(in.normal), normalize(in.normal), in.entity_color, 0.0), 1.0);
```

`config.fog_density` reaches the picture on every shaded fragment of all three.

---

### The break

The pipe is severed once, at the analysis socket. Everything from
`VisualCanvas::bind` (hop 6) to the shader (hop 12) is live and carries a
value each frame; everything from the MIDI port (hop 1) to the `"all.field"`
publish (hop 5d) exists in the tree and in no translation unit. The two halves
meet at a `StatLayoutView` that the live program fills with `{nullptr, 0}`.

#### The last living hop

`t7::BeatClock::stat_layout`, in
`/home/user/7T-Music/src/analysis/beat_clock.hpp`
(blob `b10038ff5069783c6be15e1e8d885d36238f7354`) — compiled into
`src/the_board.cpp`, whose include block carries the file's only includer, the
`#include "analysis/beat_clock.hpp"` directive immediately below
`#include "console/console.hpp"`. Quoted in full (this block is the entire file
modulo its trailing newline):

```cpp
#pragma once

// ─── beat_clock.hpp ──────────────────────────────────────────────
//
// CUT_1c: the analysis intake's successor (ruling R7). MIDI/DAW
// analysis left the build; the render side keeps its two contracts —
// an AnalysisSignal each frame and a StatLayoutView once at bind.
// The BeatClock serves both from nothing but dt: advancing clocks at
// a variable BPM (default 100; this struct is the value's ONE home,
// panel-eligible), and an EMPTY layout.
//
// The empty layout is the audio socket. The render side resolves 12
// live source names against it — all.field, ch1.present_count,
// all.window_length, all.present_count, ch1.window_length,
// ch0.onset .. ch6.onset — and every resolve misses and disables its
// coupling via the graceful path (musical/signal_layout.hpp
// resolve(): one stderr warn, valid=false). A future browser-side
// audio source plugs into this socket by publishing exactly those
// names through a real StatLayoutView.

#include "analysis/analysis_signal.hpp"

namespace t7 {

struct BeatClock {
    float bpm = 100.0f;   // variable BPM — Jean's amendment; one home

    void update(float dt) {
        signal_.dt = dt;
        signal_.t_seconds += dt;
        signal_.t_beats += dt * (bpm / 60.0f);
    }

    // Every time-bearing field of the surviving contract, from the
    // clock; everything else (stats, pads) stays at the zero it was
    // value-initialized to ONCE, at construction (OIL_1 U2 — the
    // 4128-byte per-frame re-fill retired; ledger: X BeatClock, C3).
    // The stats plane has NO writer — that is the documented audio
    // socket, and a future soundtrack writer now has a persistent
    // home to fill instead of a per-frame temporary. AnalysisSignal
    // carries no transport flag — nothing to default.
    const AnalysisSignal& output() const { return signal_; }

    StatLayoutView stat_layout() const { return StatLayoutView{ nullptr, 0 }; }

private:
    // THE WRITE WINDOW (OIL_1 U2). signal_ is written BETWEEN frames —
    // update(dt), called once from frame() before render.update() — and
    // never during it. That matters because output() now hands out a
    // reference INTO this member rather than a per-frame copy: the whole
    // update spine reads it live (phase_advance_clock reads t_beats
    // twice), so a future soundtrack writer filling the stats plane must
    // publish on the same between-frames cadence, not from an audio
    // callback landing mid-spine. The empty layout is still the socket;
    // this is the contract that comes with the persistent home.
    AnalysisSignal signal_{};   // zero-filled once; update() writes the 3 live floats
};

} // namespace t7
```

Its two live call sites, in `/home/user/7T-Music/src/the_board.cpp`
(blob `588174ecddb0d68388e39a9025d6eda2f2afd000`) — enclosing symbols: the
async-ready continuation, and `frame`:

```cpp
    app->render.bind_signal_layout(app->clock.stat_layout());
```
```cpp
    app->clock.update(dt);
    app->render.update(app->clock.output(), app->console.aspect_ratio(), app->queue);
```

#### The first dead hop

`t7::canvas_1::Canvas`, in
`/home/user/7T-Music/src/analysis/canvas_1/canvas.hpp`
(blob `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5`) — the only definition in the
tree that publishes the name `"all.field"` through a non-empty
`StatLayoutView`. Enclosing symbols `Canvas::initialize`, `Canvas::update`,
`Canvas::route`, `Canvas::advance`, `Canvas::stat_layout`, and the private
member `port_`:

The unit asks for the first dead hop in full, and the first draft of this
section instead gave `Canvas::initialize` with two in-fence `...` tokens and a
third, unmarked omission (the five-line composition comment between
`(void)asset_path;` and `constexpr int VOICES = 7;`). It is quoted here with no
elision of any kind. Recipe: `sed -n '95,150p' src/analysis/canvas_1/canvas.hpp`.

```cpp
class Canvas : public AnalysisCartridge {
public:
    // ── The cartridge lifecycle ──────────────────────────────────
    //
    // What a render side calls. initialize() composes the canvas and opens its
    // port; update() drains the port and advances one frame on the DAW's beat;
    // on_input() is unused, the canvas's source being the DAW alone.

    void initialize(const char* asset_path) override {
        (void)asset_path;   // no asset to load; the DAW is the source
        // The composition (a placeholder until the composer sets it): seven
        // voices, slot v <- MIDI v, each present + a four-beat window + the spine
        // on (so the line readings are available). The split of bands into voices
        // and a compound band is the composer's, not fixed: here voices 0..6 and
        // the cross-voice compounds in the group band (7).
        constexpr int VOICES = 7;
        for (int v = 0; v < VOICES; ++v) {
            ContextSpec s = default_spec(/*midi*/ v, /*window*/ 4.0f);
            s.crossings.active = true;   // the spine — current_pc and distance read it
            configure(v, s);
        }

        // Per-voice readings: the current note (one-hot), the present set (the
        // Playhead, by count — published on demand for the sustain law, its
        // first consumer), the present+window length (beats), and the line's
        // signed distance.
        for (int v = 0; v < VOICES; ++v) {
            publish_reading(Reading::CurrentPC,    Source::channel(v), NAME_CURRENT_PC[v]);
            publish_reading(Reading::PresentCount, Source::channel(v), NAME_PRESENT_COUNT[v]);
            publish_reading(Reading::WindowLength, Source::channel(v), NAME_WINDOW_LENGTH[v]);
            publish_reading(Reading::Distance,     Source::channel(v), NAME_DISTANCE[v]);
            // The pc-DFT capability (compound stratum) — the six interval
            // families over this voice's published present-count vector.
            publish_reading(Reading::DftMag,       Source::channel(v), NAME_DFT_MAG[v]);
            publish_reading(Reading::DftPhase,     Source::channel(v), NAME_DFT_PHASE[v]);
            publish_reading(Reading::Onset,        Source::channel(v), NAME_ONSET[v]);
        }

        // Compound readings over the union of all voices, in the group band: the
        // field (set-union then election), the current notes (vector sum — a
        // per-pc voice count), the present set (the room sounding), and the
        // present+window length (vector sum).
        const Source all = Source::group({0, 1, 2, 3, 4, 5, 6});
        publish_reading(Reading::Field,        all, "all.field");
        publish_reading(Reading::CurrentPC,    all, "all.current_pc");
        publish_reading(Reading::PresentCount, all, "all.present_count");
        publish_reading(Reading::WindowLength, all, "all.window_length");
        publish_reading(Reading::DftMag,       all, "all.dft_mag");
        publish_reading(Reading::DftPhase,     all, "all.dft_phase");

        port_.open_by_name("loopMIDI");   // the DAW's virtual port

        std::fprintf(stderr, "[canvas] loopMIDI open=%d\n", (int)port_.is_open());
    }

    void update(float dt) override {
        dt_         = dt;          // wall-clock delta, telemetry only
        t_seconds_ += dt;
        const float beat = static_cast<float>(port_.beats());   // the DAW's clock
        MidiEvent ev[256];
        const int n = port_.poll(beat, ev, 256);

        for (int i = 0; i < n; ++i) route(ev[i]);
        advance(beat);
    }
```

The rest of the class, unabridged:

```cpp
    void route(const MidiEvent& ev) {
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (active_[i] && specs_[i].channel == ev.channel) {
                contexts_[i].receive(ev);
                return;
            }
        }
    }

    // Advance every configured channel's views to `beat`, step the held fields,
    // then publish. The memories already moved in route(), so the views rebuild
    // from the present and the windows, the fields step once on the rebuilt
    // views, and publish() reads everything into the signal.
    void advance(float beat) {
        for (int i = 0; i < MAX_CHANNELS; ++i)
            if (active_[i]) contexts_[i].update(beat);
        step_fields();
        publish(beat);
    }
```

```cpp
    StatLayoutView stat_layout() const override {
        return StatLayoutView{ layout_.data(), static_cast<uint32_t>(published_count_) };
    }
```

```cpp
    MidiPort port_;             // the DAW's MIDI port, owned and drained each frame
```

#### Exactly which name fails to resolve into what

The failing name is **`"all.field"`**, not `"fog.density"`. At boot,
`src/the_board.cpp` calls `render.bind_signal_layout(clock.stat_layout())`;
`BeatClock::stat_layout()` returns `StatLayoutView{ nullptr, 0 }`;
`the_board::bind_signal_layout` forwards that view into `VisualCanvas::bind`,
which stores it in `signal_layout_` and immediately asks
`signal_layout_.resolve("all.field")`. `SignalLayout::resolve` loops
`for (uint32_t i = 0; i < view_.count; ++i)` with `view_.count == 0`, so the
loop body never executes, `misses_` increments, and it returns
`SourceBinding{}` with `valid = false`. `VisualCanvas::tick` then guards the
whole fog decode behind `if (fog_field_.valid)`, so `params_.set(fog_density_.base, ...)`
is never reached and bank slot 0 keeps the `PARAM_LAYOUT` rest of `0.0f` for
the life of the process. The target half stays fully valid — `fog_density_dst_`
resolves to `{base=0, count=1, valid=true}` — so `phase_motion_drivers` takes
its driven arm every frame and computes
`max(0, mood_state_.fog_rest_density + 0.63f * 0.0f)`, i.e. the mood's rest
alone. `GPUState::set_fog` writes that to `GPUDesignConfig::fog_density`, the
uniform uploads, and `shade_lit` fogs the frame from the mood — a live pipe
carrying a constant.

The name `"all.field"` would resolve into `t7::canvas_1::Canvas`'s published
`StatGroup{ "all.field", band, SLOT_FIELD=61, 1, StatShape::Scalar }`, whose
value is `field_index(HeldField)` — a one-based rank in `1..6` that
`FOG_BY_FIELD[idx]` is dimensioned for (`FOG_FIELD_COUNT = 7`, index 0 = none).
That `StatGroup` is never constructed at runtime because
`Canvas::initialize` is never called, because `Canvas` is never instantiated,
because `src/analysis/canvas_1/canvas.hpp` is included by no translation unit
in the single `add_executable(the_board ...)` target.

**History of the severance — file counts corrected.** The first draft of this
section reported "29 files across `src/analysis/`, `src/musical/`,
`src/sources/`" deleted by `1a52f2db` and "all 29 files plus
`src/external/RtMidi.{cpp,h}`" re-added by `0c951b11`. Both numbers were wrong
and mutually inconsistent. Re-run:

```
$ git show --name-status --format='' 1a52f2db -- src/analysis src/musical src/sources | awk '{print $1}' | sort | uniq -c
      1 A
     27 D
$ git show --name-status --format='' 1a52f2db -- src/analysis src/musical src/sources | grep '^A'
A	src/analysis/beat_clock.hpp
$ git show --name-status --format='' 1a52f2db | awk '{print $1}' | sort | uniq -c
      1 A
     49 D
      3 M
$ git show --name-status --format='' 0c951b11 | awk '{print $1}' | sort | uniq -c
     29 A
$ git show --name-status --format='' 0c951b11 | grep external
A	src/external/RtMidi.cpp
A	src/external/RtMidi.h
```

The corrected record:

- **`1a52f2db`** (2026-08-05, "CUT_1c: MIDI intake retired; BeatClock (variable
  BPM, default 100) feeds the signal spine") deleted **27** files under
  `src/analysis/` + `src/musical/` + `src/sources/` and added exactly one there,
  `src/analysis/beat_clock.hpp`. Across the whole commit it deleted **49** files:
  the 27 above plus 22 outside those directories —
  `src/external/RtMidi.cpp`, `src/external/RtMidi.h`, sixteen files under
  `src/external/imgui/**`, four under `src/external/implot/**`, and
  `src/the_lab.cpp`. It modified three files: `CMakeLists.txt`,
  `src/cartridges/the_board/realization/state.hpp`, `src/incubator_dual.cpp`.
- The six CMake targets it removed verify from
  `git show 1a52f2db -- CMakeLists.txt | grep '^-.*add_executable'`, which
  prints seven `-add_executable` lines: `the_lab`, `probe_canvas`,
  `check_canvas_union`, `check_field_union`, `check_pc_dft`,
  `check_canvas_compound`, and `${PROJECT_NAME}` (the last being a rewrite of
  the surviving main target, not a removal).
- **`0c951b11`** (2026-08-30, "bringing back the music") added **exactly 29
  files and nothing else** — no deletion, no modification. Those 29 already
  *include* `src/external/RtMidi.cpp` and `src/external/RtMidi.h`; the split is
  27 + 2, not "29 plus 2". It touched **no** build file and **no** pre-existing
  `.cpp`.
- **`e0e22e46`** (2026-08-30, "bring back the music", author
  `jeanklein1 <jeankleinmusic@gmail.com>`) touched exactly two paths —
  `M	CMakeLists.txt` and `A	git`, the latter a stray file at the repository root
  named `git`. Its `CMakeLists.txt` edit does two things: appends
  `__WINDOWS_MM__` to `MSVC_COMPILE_DEFS`, and adds `src/external/RtMidi.cpp` to
  `add_executable(the_board ...)`. It added no other TU and left
  `src/the_board.cpp` untouched.

Recipes, in full:

```
$ git show --name-status --format='' 1a52f2db -- src/analysis src/musical src/sources
$ git show --name-status --format='' 1a52f2db
$ git show 1a52f2db -- CMakeLists.txt | grep '^-.*add_executable'
$ git show --name-status --format='' 0c951b11
$ git show --name-status --format='%h %ad %s' --date=short e0e22e46
$ git show e0e22e46 -- CMakeLists.txt
$ git log --oneline 1a52f2db..HEAD -- src/the_board.cpp CMakeLists.txt
```

The restore therefore returned the source code of hops 1–5 to the tree without
returning any of them to a translation unit, and `BeatClock` — the object that
answered for them after the cut — is still the one wired into
`src/the_board.cpp`.

---

### Gaps and flags recorded

- **`src/analysis/signal_layout.hpp` does not exist.** The unit's hop-5 path is
  wrong; the file is `src/musical/signal_layout.hpp` (blob
  `8e2e84312483e31e429276d91c23f7d63dc2643c`). Recipe:
  `git ls-tree -r HEAD --name-only | grep -i signal_layout` → one hit.
- **No `STAT_LAYOUT` symbol exists anywhere in `src/`; the census is 2 hits, not
  3.** The first draft of this section reported 3. Recipe and literal output:

  ```
  $ rg -n 'STAT_LAYOUT' src/
  src/analysis/analysis_signal.hpp:128:// Non-owning view over a cartridge's STAT_LAYOUT array. STAT_LAYOUT is
  src/analysis/analysis_cartridge.hpp:98:     * @return  Non-owning view over the cartridge's static STAT_LAYOUT.
  ```

  Confirmed at the stated walk point too: `git grep -c 'STAT_LAYOUT' 79adfa4d --
  src/` → `analysis_cartridge.hpp:1`, `analysis_signal.hpp:1`. Both hits are
  prose: the first is the comment immediately above `struct StatLayoutView`; the
  second is the `@return` doc line above
  `virtual StatLayoutView stat_layout() const = 0;`. The finding stands — no
  `STAT_LAYOUT` definition exists, only prose naming one. `Canvas` builds its
  equivalent at runtime into a member named `layout_`.
- **A native recorded witness of the fog binding DOES exist**, correcting the
  first draft, which recorded the opposite.
  `docs/reference/RELEASE_CONSOLE.md` holds two transcripts; the second is a
  native Windows/Dawn Release run (`[Console] Dawn revision: f0bf8ab5…`,
  `[Console] Build: Release`, six D3D12/D3D11/Vulkan adapter rows,
  Windows-backslash asset paths, CP437 mojibake, and no Emscripten frame), and
  it carries `[the_board] fog.density base=0 valid=1 | fog.color base=1 count=3
  valid=1`. Recipes: `grep -n 'fog.density base' docs/reference/RELEASE_CONSOLE.md`
  → lines 272 and 561; `grep -n 'INCUBATOR DUAL' …` → 8 and 286;
  `grep -n '(index):' … | tail -1` → 282. Both transcripts are labelled
  "Specimens, not current output" by the file's own header, and the native one
  reports `[GPUState] Design Config: 624 B` where this HEAD's `static_assert`
  says 720 — so it is not a capture at this commit.
- **`src/external/RtMidi.cpp` is compiled and linked with no caller.** It is
  the second of the target's two TUs and `__WINDOWS_MM__` selects a real
  backend, but the only class that would use it (`MidiPort`) is declared only as
  `t7::canvas_1::Canvas::port_` in `src/analysis/canvas_1/canvas.hpp`, which is
  in no TU.
- **The four `check_*.cpp` and `probe_canvas.cpp` under
  `src/analysis/canvas_1/` are in no build target.** Recipe:
  `rg -n 'check_canvas|probe_canvas|check_field|check_pc_dft|canvas_1
  CMakeLists.txt tools/` → no output, exit 1. `ls src/analysis/canvas_1/` lists
  six files: `canvas.hpp`, `check_canvas_compound.cpp`, `check_canvas_union.cpp`,
  `check_field_union.cpp`, `check_pc_dft.cpp`, `probe_canvas.cpp`. `1a52f2db`
  removed exactly the matching CMake targets; nothing since has re-added them.
- **`probe_canvas.cpp` names `MidiPort` in prose and was missing from the first
  draft's hop-1 census.** It is now reported: `src/analysis/canvas_1/probe_canvas.cpp:24`
  (blob `7a341e9c00a0b8bd1965714df87c533d30a6cced`), inside the file's banner
  comment, reading `// Needs RtMidi and the transport-aware MidiPort. In Ableton,
  enable loopMIDI's`. It is a comment, not a declaration, and the file is in no
  target, so the hop-1 verdict is unaffected.
- **`src/coupling/organ_registry.hpp` has no includer anywhere in `src/`.**
  Found while anchoring hop 9. Recipe: `rg -n 'coupling/organ_registry' src/` →
  no output, exit 1. The registry that IS compiled is
  `src/console/organ_registry.hpp` (blob `70d09e9602eb0f763a616da5303e14c34e7f44da`),
  named by the `#include "console/organ_registry.hpp"` directive in
  `src/cartridges/the_board/cartridge.hpp`'s include block. The two files carry
  near-identical `ORGAN_BLOCK_DRIVERS` / `DRIVER_LIVE` text (`rg -n
  'DRIVER_TABLE|DRIVER_LIVE' src/` shows both). Recorded as an observed
  duplicate with one live copy; no repair is proposed.
- **FLAG — I could not observe the program running.** Every verdict above is
  static reachability (include-graph closure over the two TUs named by
  `add_executable`) plus the two recorded console transcripts. A build was
  prohibited (BUILD ruling). Resolving the runtime behaviour first-hand would
  have cost a `cmake --build --preset the-board-full-release` and a native
  console capture, which only Jean's machine can produce (glaw1 gate,
  CLAUDE.md).
- **FLAG — preprocessor conditionals were not evaluated, and one edge on the
  critical path IS conditional.** The include-graph census used textual
  `rg -n 'include "..."'` matches, so an `#include` guarded by an `#if` that is
  false on the active preset would still count as an edge. The first draft of
  this section asserted that the edges that matter are "all unguarded". That is
  wrong for one of them, and is corrected here. In `src/the_board.cpp` the
  literal `#include "cartridges/the_board/cartridge.hpp"` sits **inside**
  `#if defined(__INTELLISENSE__)`; the compiled branch is the `#else` arm,
  `#include RENDER_HEADER(INCUBATE_RENDER)`. Verbatim:

  ```cpp
  // IntelliSense cannot resolve macro-expanded #include paths.
  // This literal include gives VS navigation (Peek Definition, Go To, etc.).
  // The compiler ignores it -- the macro include below pulls in the same file.
  #if defined(__INTELLISENSE__)
  #include "cartridges/the_board/cartridge.hpp"
  #else
  #include RENDER_HEADER(INCUBATE_RENDER)
  #endif
  ```

  The edge still lands on the same file: `#define RENDER_HEADER(name)
  STRINGIFY(cartridges/name/cartridge.hpp)` in `src/the_board.cpp`, and
  `INCUBATE_RENDER=${T7_RENDER_CARTRIDGE}` in `CMakeLists.txt`'s
  `target_compile_definitions(the_board PRIVATE ...)` block, with a `#ifndef
  INCUBATE_RENDER / #define INCUBATE_RENDER the_board` fallback in the same
  source file. So `cartridge.hpp` is in `the_board`'s closure on both arms, and
  hop 6's PRESENT verdict is unchanged. The other edges on the path — the
  `#include "console/console.hpp"` and `#include "analysis/beat_clock.hpp"`
  directives in `src/the_board.cpp`; `#include "coupling/visual_canvas.hpp"` and
  `#include "console/organ_registry.hpp"` in `cartridge.hpp`; and
  `#include "musical/signal_layout.hpp"` / `#include "analysis/analysis_signal.hpp"`
  in `visual_canvas.hpp` — were each read in their surrounding lines and are
  unguarded. A full preprocessor run was not performed; that would have cost a
  `clang++ -E` over `src/the_board.cpp` with the pinned Dawn include set, which
  the BUILD ruling forbids.
- **FLAG — the HEAD named at the top of this section is no longer the
  repository's HEAD.** See the note under the section title. The tree is clean
  (`git status --porcelain` empty) and every cited blob is identical at
  `79adfa4d` and at `6d53388e`, so no finding changes; the divergence is
  recorded because a commit landed inside the repository after the walk and
  because that commit's added file, `docs/LIGATURE_0_RECON.md`, contains copies
  of this section's own code blocks, which will make any future exact-substring
  search for those blocks match two files instead of one.

## 6. Slice B — radial pulses (event-driven ring)

Anchor: branch `claude/ligature-0-recon-hcrix0`, campaign anchor commit
`79adfa4d26c9e17e0074692928f1d2875d7edde1`. Every quotation below is from the
working tree at that commit; every blob SHA was obtained with
`git rev-parse <anchor>:<path>` (R6).

**Anchor note — the branch tip has advanced one commit past the campaign
anchor.** `git rev-parse HEAD` now returns
`6d53388e83f4a5cd7ad3b154484c885f567a02da` (*"LIGATURE_0 — the recon report: the
ligature is one hop, and the socket is empty"*); `79adfa4d` is `HEAD~1`.
`git diff --stat 79adfa4d HEAD` shows exactly one changed path:

```
 docs/LIGATURE_0_RECON.md | 8837 ++++++++++++++++++++++++++++++++++++++++++++++
 1 file changed, 8837 insertions(+)
```

Nothing under `src/` moved. Every blob SHA cited in this section was re-checked
at both revisions and is identical at each — the loop that checked it, and its
verdict:

```
$ for p in src/musical/pc_count.hpp src/analysis/canvas_1/canvas.hpp \
           src/coupling/visual_canvas.hpp src/cartridges/the_board/cartridge.hpp \
           src/cartridges/the_board/realization/state.hpp \
           src/cartridges/the_board/realization/world.wgsl \
           src/cartridges/the_board/surface/terrain_looks.hpp \
           src/cartridges/the_board/contracts/ground_architecture.hpp \
           src/musical/previous_event.hpp src/musical/spine.hpp \
           src/musical/context.hpp src/musical/stream_data.hpp ; do
      a=$(git rev-parse 79adfa4d:$p); b=$(git rev-parse HEAD:$p)
      [ "$a" = "$b" ] && echo "SAME $a  $p" || echo "DIFF $a vs $b  $p" ; done
SAME acccba766e2a9b05b3d03f99ce3a238b6376dd9a  src/musical/pc_count.hpp
SAME 250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5  src/analysis/canvas_1/canvas.hpp
SAME ab5a21993a98c5dcc096f38b5db0c16cb7afbf35  src/coupling/visual_canvas.hpp
SAME 3651bcabaa0b02a2925ad6868ce541ea9ab1b202  src/cartridges/the_board/cartridge.hpp
SAME fe4bce836b4665588ed43a0729d312e89cd05a20  src/cartridges/the_board/realization/state.hpp
SAME 5b36243dc6b45e27271d0d73eca8a01eb5dc2078  src/cartridges/the_board/realization/world.wgsl
SAME 0c7cc03b73dc4994bf21416817c8891849d7bdce  src/cartridges/the_board/surface/terrain_looks.hpp
SAME 0488bc642c5993d6e32310d2fe1bd3fe6d41a3d2  src/cartridges/the_board/contracts/ground_architecture.hpp
SAME b97550ce0566bb351c36f2dfa8eeead803bde808  src/musical/previous_event.hpp
SAME 8e9c144193e49019195d3526ed08f3c27486fb53  src/musical/spine.hpp
SAME a4c3977e08b4bf8638b6072077995a095667ad5f  src/musical/context.hpp
SAME d84086d83e06113e5284caa2aa6dc24b304cbd5f  src/musical/stream_data.hpp
```

The two revisions are therefore interchangeable for every claim in this section,
and every census recipe below reproduces identically at either.

### 6.0 Status vocabulary used in this section

Stated explicitly because the sibling Slice A section was not available to read
when this one was written (see FLAG at §6.9).

| marker | meaning as used here |
| --- | --- |
| **PRESENT** | The hop's code exists at HEAD, executes on a running frame, and the value it carries is data-dependent — it varies with what the program is fed. |
| **STALE** | The hop's code exists at HEAD and is structurally intact, but the value entering it is a boot/compile-time rest constant. The hop runs (or is skipped by a rest law) and can only produce the neutral result. |
| **GONE** | No code at HEAD performs the hop. The identifiers such a hop would need do not resolve anywhere under `src/`. |

### 6.1 The census before the walk (R2)

All greps run from the repo root at HEAD. Recipes and their exact outputs:

| # | command | result |
| --- | --- | --- |
| A | `rg -in 'pulse' src/ \| wc -l` | `280` matching lines |
| A′ | `rg -ic 'pulse' src/` | per-file line counts, table below |
| B | `rg -n 'set_pulse_data' src/ \| wc -l` | `2` |
| C | `rg -n 'pulse_count\|pulse_data' src/ \| wc -l` | `15` |
| D | `rg -n 'contrib_radial_pulses_at' src/ \| wc -l` | `5` |
| E | `rg -n 'CONTRIB_RADIAL_PULSES' src/ \| wc -l` | `14` |
| F | `rg -in 'onset' src/ \| wc -l` | `148` |
| G | `rg -n 'pulse_ring\|pulse_write_idx\|PULSE_RING_SIZE\|PULSE_ONSET_THRESHOLD\|PULSE_AMPLITUDE\|PULSE_INCREASE_CLAMP\|MMODE_RADIAL_PULSE' src/` | **no output, rg exit status 1** (no match) |
| H | `rg -n 'possessed\|pawn_pos\|player_\.\|readback' src/coupling/` | **no output, rg exit status 1** (no match) |
| I | `rg -in 'pulse' audit/` | exactly one line: `audit/BINDING_LEDGER.md` naming `contrib_radial_pulses_at` as a `law-ref` |
| J | `rg -in 'pulse' docs/LAWS.md docs/OPEN.md` | **no output** — neither the rule book nor the open-state file names a pulse |
| K | `rg -n 'on_onset\|on_offset' src/` | `9` lines across 3 files — the scoped onset-feeder census, output verbatim below |
| L | `rg -ic 'onset' src/` | 19 files, per-file line counts summing to 148 (= recipe F); table below |
| M | `rg -n 'onset' src/musical/stream_data.hpp` | `18` lines — the raw onset records; quoted at Hop 1d |

Recipe A′ output verbatim (`rg -ic 'pulse' src/`, lines-with-a-match per file):

```
src/cartridges/the_board/surface/terrain_looks.hpp:4
src/cartridges/the_board/bodies/ribbon.hpp:1
src/cartridges/the_board/bodies/orbs.hpp:1
src/cartridges/the_board/bodies/gol_zones.hpp:43
src/cartridges/the_board/contracts/entity_types.hpp:3
src/cartridges/the_board/contracts/ground_architecture.hpp:10
src/cartridges/the_board/contracts/roster.hpp:1
src/cartridges/the_board/contracts/spine_state.hpp:1
src/cartridges/the_board/realization/state.hpp:22
src/sources/transport.hpp:14
src/sources/midi_port.hpp:2
src/coupling/visual_canvas.hpp:5
src/analysis/canvas_1/canvas.hpp:1
src/musical/pc_count.hpp:1
src/cartridges/the_board/cartridge.hpp:6
src/cartridges/the_board/realization/world.wgsl:165
```

**Three distinct things wear the word "pulse" in this tree, and only one is
this slice.** Recorded so no count here is mistaken for another's:

1. **The radial pulse ring** (this slice): `pulse_count`, `pulse_data`,
   `set_pulse_data`, `contrib_radial_pulses_at`, `CONTRIB_RADIAL_PULSES`,
   `REST_PULSE_COUNT`, the `PULSE_SPEED..PULSE_AGE_DECAY` WGSL dial panel.
2. **The GoL "Pulse" automaton algorithm** (`src/cartridges/the_board/bodies/gol_zones.hpp`,
   43 lines; `world.wgsl` `GOL_PULSE_TIERS`, `PulseField`, `pulse_cell_target`) —
   a lattice rule family, unrelated to the ring. It owns most of the 280.
3. **MIDI timing-clock pulses** (`src/sources/transport.hpp`, 14 lines;
   `MIDI_CLOCK_PPQN = 24`, `pulses()`, `beats()`) — the transport's 0xF8 counter.
   Also unrelated to the ring.

Additionally `src/coupling/visual_canvas.hpp`'s 5 hits are all the substring
inside **"impulses"** (the zoetrope's row impulses), not the radial ring:
`rg -n 'pulse' src/coupling/visual_canvas.hpp` returns only the zoetrope ear
comments and the `zoetrope_rows_` member. **There is no radial-pulse identifier
anywhere in `src/coupling/`.**

**The scoped onset census (recipes K, L, M).** Recipe F is a broad line count and
conflates families; the walk of Hop 1 rests on the scoped ones. Recipe K output
verbatim (`rg -n 'on_onset|on_offset' src/`) — the complete set of onset-event
feeder sites in the tree:

```
src/musical/previous_event.hpp:16:// offset elects. Releases are recorded (on_offset) for that election; a
src/musical/previous_event.hpp:67:    void on_onset(int pitch, float velocity, float beat) {
src/musical/previous_event.hpp:83:    void on_offset(int pitch, float time) {
src/musical/context.hpp:114:            if (is_on) previous_.on_onset(ev.pitch, ev.velocity, ev.beat);
src/musical/context.hpp:115:            else       previous_.on_offset(ev.pitch, ev.beat);
src/musical/context.hpp:118:            if (is_on) spine_.on_onset(ev.pitch, ev.beat);
src/musical/context.hpp:119:            else       spine_.on_offset(ev.pitch, ev.beat);
src/musical/spine.hpp:100:        void on_onset(int pitch, float t) {
src/musical/spine.hpp:156:        void on_offset(int pitch, float t) {
```

Nine lines: **two `on_onset` definitions** (`PreviousEvent::on_onset`,
`Spine::on_onset`), **two `on_offset` definitions**, **four dispatch lines** in
the one router that feeds both (`Context::receive`), and **one prose line** in
`previous_event.hpp`'s file header. No fifth site exists; nothing under
`src/coupling/`, `src/cartridges/` or `src/analysis/` names `on_onset` at all.

Recipe L output verbatim (`rg -ic 'onset' src/`, lines-with-a-match per file) —
the per-file decomposition of recipe F's 148:

```
src/coupling/visual_canvas.hpp:4
src/analysis/beat_clock.hpp:1
src/analysis/analysis_cartridge.hpp:1
src/analysis/canvas_1/probe_canvas.cpp:1
src/analysis/canvas_1/check_canvas_compound.cpp:1
src/analysis/canvas_1/canvas.hpp:33
src/sources/midi_file.hpp:1
src/cartridges/the_board/bodies/pawn_figures.hpp:1
src/musical/previous_event.hpp:22
src/musical/stream_data.hpp:18
src/musical/context.hpp:8
src/musical/wagon.hpp:8
src/musical/context_spec.hpp:6
src/musical/pc_count.hpp:10
src/musical/midi_stream.hpp:1
src/musical/playhead.hpp:20
src/musical/spine.hpp:5
src/cartridges/the_board/realization/state.hpp:1
src/cartridges/the_board/realization/world.wgsl:6
```

(The nineteen counts sum to 148, matching recipe F. `rg`'s file order is not
stable between runs; the *set* of rows is.)

The width question the unit raised is answered here: **the ring is 8 slots wide,
carried CPU-side as a flat `float[32]`** (8 × 4 floats) and GPU-side as
`array<vec4<f32>, 8>`. Recipe C's rows are the witness; both declarations are
quoted at Hop 4 and Hop 5.

### Hop 1 — onset detection

**File:** `src/musical/pc_count.hpp`
**Blob SHA:** `acccba766e2a9b05b3d03f99ce3a238b6376dd9a`
**Enclosing symbol:** `inline PitchClassVector pc_onset(const PlayheadReadout&, const WagonReadout&, float)`

```cpp
// ═══ ONSET ═══════════════════════════════════════════════════════

// Note-on impulses since `since_beat` (exclusive) → velocity-weighted
// pitch-class sums. The same reduction as the counts, with the weight
// switched from 1 to the note-on's velocity (already [0,1] at the stream
// layer) and the population filtered to onsets inside (since_beat, anchor].
// The present carries the still-sounding onsets; the window carries the
// ones already completed (an on-and-off within a single frame) — disjoint
// as ever, so the sum stands without double-counting. Each onset lands in
// exactly one aperture: the caller advances since_beat to its anchor after
// every read. Same-class retriggers inside one aperture sum.
inline PitchClassVector pc_onset(const PlayheadReadout& ph, const WagonReadout& wg, float since_beat) {
    PitchClassVector v;
    for (int i = 0; i < ph.current_count; ++i)
        if (ph.current[i].onset_beat > since_beat)
            v[pc_of(ph.current[i].pitch)] += ph.current[i].velocity;
    for (int i = 0; i < wg.note_count; ++i)
        if (wg.notes[i].onset_beat > since_beat)
            v[pc_of(wg.notes[i].pitch)] += wg.notes[i].velocity;
    return v;
}
```

**Status: PRESENT.**

The reduction is reached, published and witnessed. Three further sites in
`src/analysis/canvas_1/canvas.hpp` (blob `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5`)
carry it into the signal:

Enclosing symbol: `class Canvas1` static member `SLOT_ONSET` / `NAME_ONSET`
(the address and the names):

```cpp
    static constexpr int SLOT_ONSET          = 76;   // 12  note-on impulses since the previous
                                                     //     published frame, velocity-weighted;
                                                     //     each onset lands in exactly one frame
```

```cpp
    static constexpr const char* NAME_ONSET[MAX_CHANNELS] = {
        "ch0.onset","ch1.onset","ch2.onset","ch3.onset",
        "ch4.onset","ch5.onset","ch6.onset","ch7.onset"
    };
```

Enclosing symbol: `Canvas1::configure`-side publication loop (the `for (int v = 0;
v < VOICES; ++v)` block that calls `publish_reading`):

```cpp
            publish_reading(Reading::Onset,        Source::channel(v), NAME_ONSET[v]);
```

Enclosing symbol: `Canvas1::per_channel_reading(Reading, int)` — the value source:

```cpp
            case Reading::Onset:        return pc_onset(c.playhead(), c.wagon(0), onset_prev_beat_);
```

Enclosing symbol: `Canvas1::write_reading(Published&)` — the write into the band:

```cpp
            case Reading::CurrentPC:
            case Reading::PresentCount:
            case Reading::WindowLength:
            case Reading::Onset: {
                // Vector readings: the per-source sum dressed to D. One channel ->
                // that channel's reading; a union -> the cross-voice vector sum —
                // same code, since the additive compound is just the sum.
                const VectorDressing to_D{ /*reorigin*/ true, PROJECT_PC_ORIGIN, VectorDressing::Scale::None };
                write_vector(p.band, slot, dress(reading_vector(p.reading, p.source_mask), to_D));
                break;
            }
```

Enclosing symbol: `Canvas1::writer_wired(Reading)` — the contract gate that lets
it be published at all:

```cpp
    static bool writer_wired(Reading r) {
        return r == Reading::Field || r == Reading::CurrentPC
            || r == Reading::PresentCount
            || r == Reading::WindowLength || r == Reading::Distance
            || r == Reading::DftMag || r == Reading::DftPhase
            || r == Reading::Onset;
    }
```

Enclosing symbol: `Canvas1::publish(float)` — the aperture and its witness
(quoted because it establishes that the onset value is live, per-frame, and
instrumented):

```cpp
        // The onset aperture: a backward beat jump (a DAW transport loop)
        // re-anchors the since-edge, so a looped clip keeps striking; the
        // edge then advances once per published frame, so each onset lands
        // in exactly one frame's (prev, beat] window.
        if (beat < onset_prev_beat_) onset_prev_beat_ = beat;
```

```cpp
        if constexpr (INSTRUMENTS.zoetrope_witness) {
            for (int v = 0; v < MAX_CHANNELS; ++v) {
                if (!active_[v]) continue;
                float sum = 0.0f;
                for (int i = 0; i < 12; ++i) sum += output_.stat(v, SLOT_ONSET + i);
                if (sum <= 0.0f) continue;
                std::fprintf(stderr, "[ONSET] ch%d sum=%.2f pcs=", v, sum);
```

**Note on the ONE consumer that exists.** The published `chN.onset` slots are
resolved by exactly one coupling site, and it is not the pulse ring.
`src/coupling/visual_canvas.hpp`, blob `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35`,
enclosing symbol `VisualCanvas` (the `reset`/bind block and the per-tick fold):

```cpp
            // zoetrope ears (the listener set): one "chN.onset" resolve per
            // set bit of ZOETROPE_EARS. A miss warns and disables that ear —
            // the resolver's own semantics; the deaf ear simply never sums.
            zoetrope_ear_count_ = 0;
            for (int ch = 0; ch < 8; ++ch) {
                if (!(ZOETROPE_EARS & (1u << ch))) continue;
                std::string v("ch" + std::to_string(ch));
                zoetrope_ears_[zoetrope_ear_count_++] =
                    signal_layout_.resolve((v + ".onset").c_str());
            }
```

```cpp
            // ── the zoetrope's ears (row impulses) ──────────────────────
            // Sum the resolved ears' onset vectors into pc impulses and fold
            // them through the mode table. Published vectors ship DRESSED to
            // D (index 0 = D — the canvas contract the checker's PC_COLOR
            // table already binds); ZOETROPE_ROW_OF_PC is authored by raw
            // pitch class (0 = C), so the fold un-dresses: pc = (i + 2) % 12.
            // Overwritten every tick — impulses, not an accumulator; the
            // lattice integrates, this side only hears.
            for (int r = 0; r < 7; ++r) zoetrope_rows_[r] = 0.0f;
            for (int e = 0; e < zoetrope_ear_count_; ++e) {
                const SourceBinding& ear = zoetrope_ears_[e];
                if (!ear.valid) continue;
                for (int i = 0; i < 12; ++i) {
                    const float w = signal.stat(ear.channel, ear.base + i);
                    if (w <= 0.0f) continue;
                    zoetrope_rows_[ZOETROPE_ROW_OF_PC[(i + 2) % 12]] += w;
                }
            }
```

Its destination is `zoetrope_rows_[7]`, declared in the same class:

```cpp
        // ── zoetrope coupling state (the ears + the fold) ────────────────
        SourceBinding zoetrope_ears_[8]{};    // "chN.onset" per set bit of ZOETROPE_EARS
        int           zoetrope_ear_count_ = 0;
        float         zoetrope_rows_[7] = {}; // row impulses, overwritten each tick
```

The onset signal therefore terminates in the zoetrope's seven rows. Nothing in
this file, or anywhere in `src/coupling/`, forwards it toward a pulse ring
(recipe G, recipe H).

#### Hop 1d — the raw onset records (`src/musical/stream_data.hpp`)

The unit named `src/musical/stream_data.hpp` as a Hop-1 candidate. It is not a
detector; it is the **store** whose fields `pc_onset` reads. Recorded here in
full so the chain from MIDI note-on to `pc_onset` has no unquoted link.

**File:** `src/musical/stream_data.hpp`
**Blob SHA:** `d84086d83e06113e5284caa2aa6dc24b304cbd5f`
**Enclosing symbols:** `struct ActiveNote`, `struct CompletedNote`,
`ActiveSet::note_on(int, float, float)`

The record of a still-sounding note — enclosing symbol `struct ActiveNote`:

```cpp
/**
 * An active note has known onset but undetermined offset.
 * Duration is provisional: current_beat - onset_beat.
 * 
 * This is stored in ActiveSet indexed by pitch.
 * velocity = 0 means the slot is empty.
 */
struct ActiveNote {
    float velocity = 0.0f;      // 0 = slot empty, >0 = note active
    float onset_beat = 0.0f;
    
    bool is_active() const { return velocity > 0.0f; }
    
    float duration_at(float current_beat) const {
        return is_active() ? current_beat - onset_beat : 0.0f;
    }
};
```

The record of a finished note — enclosing symbol `struct CompletedNote`:

```cpp
/**
 * A completed note has known onset and offset.
 * Duration is fixed and immutable.
 * 
 * This is stored in CompletedRing.
 */
struct CompletedNote {
    float onset_beat;
    float offset_beat;
    float velocity;
    uint8_t pitch;
    uint8_t _pad[3];
    
    float duration() const { return offset_beat - onset_beat; }
    int pitch_class() const { return pitch % 12; }
    int octave() const { return pitch / 12; }
};

static_assert(sizeof(CompletedNote) == 16, "CompletedNote should be 16 bytes");
```

The one write of an onset time — enclosing symbol `ActiveSet::note_on(int, float,
float)`, itself inside `struct ActiveSet`:

```cpp
    void note_on(int pitch, float velocity, float beat) {
        if (pitch < 0 || pitch >= MIDI_PITCH_COUNT) return;
        notes[pitch].velocity = velocity;
        notes[pitch].onset_beat = beat;
        mask.set(pitch);
    }
```

and the reader that hands the field back out — enclosing symbol
`ActiveSet::onset(int) const`:

```cpp
    float onset(int pitch) const {
        return (pitch >= 0 && pitch < MIDI_PITCH_COUNT) ? notes[pitch].onset_beat : 0.0f;
    }
```

**Status: PRESENT.** `ActiveNote::onset_beat` and `CompletedNote::onset_beat` are
exactly the two fields `pc_onset` tests (`ph.current[i].onset_beat > since_beat`
and `wg.notes[i].onset_beat > since_beat`, quoted at the top of Hop 1). Recipe M
(`rg -n 'onset' src/musical/stream_data.hpp`) returns 18 lines, published whole
so nothing in this file is left unaccounted for:

```
7://   ActiveSet     — notes currently sounding (onset known, offset
9://   CompletedRing — notes finished sounding (onset and offset both
120: * An active note has known onset but undetermined offset.
121: * Duration is provisional: current_beat - onset_beat.
128:    float onset_beat = 0.0f;
133:        return is_active() ? current_beat - onset_beat : 0.0f;
140: * A completed note has known onset and offset.
146:    float onset_beat;
152:    float duration() const { return offset_beat - onset_beat; }
179:        notes[pitch].onset_beat = beat;
213:    float onset(int pitch) const {
214:        return (pitch >= 0 && pitch < MIDI_PITCH_COUNT) ? notes[pitch].onset_beat : 0.0f;
257:    void push(int pitch, float velocity, float onset_beat, float offset_beat) {
261:        note.onset_beat = onset_beat;
328:     * A note is inside if: onset >= start AND offset <= end
339:            if (note.onset_beat >= start && note.offset_beat <= end) {
347:     * A note overlaps if: onset < end AND offset > start
358:            if (note.onset_beat < end && note.offset_beat > start) {
```

Grouped by enclosing symbol, the eighteen decompose exactly: **7 prose lines**
(the file header's two `ActiveSet` / `CompletedRing` lines, `struct ActiveNote`'s
two doc lines, `struct CompletedNote`'s one, and the two `CompletedRing`
window-query doc lines); **2 field declarations** (`ActiveNote::onset_beat`,
`CompletedNote::onset_beat`); **2 duration accessors**
(`ActiveNote::duration_at(float) const`, `CompletedNote::duration() const`); **1
write** (`ActiveSet::note_on`); **2 lines of the read accessor**
(`ActiveSet::onset(int) const`); **2 lines of the completed-note push**
(`CompletedRing::push`'s `onset_beat` parameter and its assignment); and **2
window predicates** in `CompletedRing`'s inside/overlap queries.
7 + 2 + 2 + 1 + 2 + 2 + 2 = 18. **No line
of this file names a pulse** — recipe A′ does not list `src/musical/stream_data.hpp`
among the sixteen files with a `pulse` match.

#### Hop 1e — the second live onset detector: `PreviousEvent`

Named by the unit as a Hop-1 candidate and present at HEAD as a running state
machine, distinct from `pc_onset`.

**File:** `src/musical/previous_event.hpp`
**Blob SHA:** `b97550ce0566bb351c36f2dfa8eeead803bde808`
**Enclosing symbol:** `PreviousEvent::on_onset(int pitch, float velocity, float beat)`
inside `class PreviousEvent`

The file's own statement of what it detects — enclosing symbol: the file header
prose block (before `namespace t7`):

```cpp
// ─── previous_event.hpp ──────────────────────────────────────────
//
// Holds the two most recent onset-groups: the open one being struck now —
// the CURRENT event — and the prior one — the PREVIOUS event — latched. A
// latch, not a window: the previous survives any silence. Fed onset events;
// notes struck within a simultaneity tolerance join the open group; when a
// new group opens beyond tolerance, the open group becomes previous and is
// held until the next opens.
//
// An event is defined by ONSET: notes struck together are one group, and the
// grouping never consults release. But the previous event's REPRESENTATIVE —
// the single note that speaks for the group in a comparison — is the voice
// that goes offset LAST, the one that lingered to the seam. So onset groups;
// offset elects. Releases are recorded (on_offset) for that election; a
// still-sounding voice counts as the latest possible offset, so a held note
// outranks any that has already let go. The minimal-motion closest_pitch
// remains as an alternate criterion.
```

The detector itself — enclosing symbol `PreviousEvent::on_onset(int, float, float)`:

```cpp
    // Feed a note onset. Runs the grouping state machine; may latch the open
    // group as "previous".
    void on_onset(int pitch, float velocity, float beat) {
        if (open_count_ == 0) {
            open_onset_ = beat;
            push_open(pitch, velocity, beat);
        } else if (beat - open_onset_ <= tolerance_) {
            push_open(pitch, velocity, beat);          // same group
        } else {
            latch();                                   // open group -> previous
            open_onset_ = beat;
            push_open(pitch, velocity, beat);          // start a new group
        }
    }
```

and the latched record it builds — enclosing symbol `struct PrevNote`:

```cpp
struct PrevNote {
    uint8_t pitch = 0;
    uint8_t _pad[3] = {0, 0, 0};
    float   velocity   = 0.0f;
    float   onset_beat = 0.0f;
    float   offset     = PREV_SOUNDING;   // release time; PREV_SOUNDING while still on
};

static_assert(sizeof(PrevNote) == 16, "PrevNote should be 16 bytes");
```

**Status: PRESENT** as a detector, **with no consumer outside `src/musical/`.**
The census that establishes the second half:

```
$ rg -n 'previous\(\)|PreviousEvent|previous_ops' src/analysis/ src/coupling/ src/cartridges/
$ echo $?
1
```

Zero output, exit status 1. Nothing in the analysis cartridge, the coupling layer
or either board cartridge reads `Context::previous()` or names `PreviousEvent`.
This onset detector therefore does not reach the signal spine at all, let alone a
pulse ring. It is enabled per-slot by `context_realize.hpp` (blob
`4285730324237ed01ed9fbd9e624d9bd4896417e`), enclosing symbol
`inline void realize(const ContextSpec& spec, Context& ctx)`:

```cpp
    // Memory layer: each enabled only if the spec asks for it.
    if (spec.event.active) {
        ctx.enable_previous(spec.event.tolerance_beats);
    }
    if (spec.crossings.active) {
        ctx.enable_spine(spec.crossings.tolerance_beats,
                         oracle_for(spec.crossings.oracle));
    }
```

#### Hop 1f — the third live onset detector: `Spine`

**File:** `src/musical/spine.hpp`
**Blob SHA:** `8e9c144193e49019195d3526ed08f3c27486fb53`
**Enclosing symbol:** `Spine::on_onset(int pitch, float t)` inside `class Spine`

```cpp
        /**
         * A note begins sounding at time t (grouping clock, seconds).
         */
        void on_onset(int pitch, float t) {
            // Retrigger of a sounding pitch: refresh its onset, no crossing.
            for (int i = 0; i < n_sounding_; ++i) {
                if (sounding_[i].pitch == pitch) {
                    sounding_[i].onset = t;
                    return;
                }
            }

            const int n = n_sounding_;

            if (n == 0) {
                // Entering silence: the resolved election is the previous.
                if (resolved_.valid) {
                    prev_ = LineNote{ resolved_.pitch, resolved_.prov, true };
                }
                else {
                    prev_.valid = false;   // first event — no previous (parked)
                }
                holder_ = Holder{ pitch, t, true };
            }
```

**Status: PRESENT.** Unlike `PreviousEvent`, `Spine` does have two readers outside
`src/musical/`, both in the analysis cartridge, and neither is a pulse. The
census:

```
$ rg -n '\.spine\(\)' src/analysis/ src/coupling/ src/cartridges/
src/analysis/canvas_1/canvas.hpp:558:                    c >= 0 ? static_cast<float>(line_distance(contexts_[c].spine())) : 0.0f);
src/analysis/canvas_1/canvas.hpp:593:            case Reading::CurrentPC:    return current_note(c.spine());
```

Enclosing symbol `Canvas1::write_reading(Published&)`, the `Reading::Distance` arm:

```cpp
            case Reading::Distance: {
                // The line's signed interval — a per-voice scalar; no union form.
                const int c = first_source(p.source_mask);
                output_.set_stat(p.band, slot,
                    c >= 0 ? static_cast<float>(line_distance(contexts_[c].spine())) : 0.0f);
                break;
            }
```

and enclosing symbol `Canvas1::per_channel_reading(Reading, int) const`, the
`Reading::CurrentPC` arm (quoted here with its neighbours so the `Reading::Onset`
arm's position in the same switch is visible):

```cpp
    PitchClassVector per_channel_reading(Reading r, int i) const {
        const Context& c = contexts_[i];
        switch (r) {
            case Reading::CurrentPC:    return current_note(c.spine());
            case Reading::PresentCount: return pc_count(c.playhead());
            case Reading::WindowLength: return pc_length(c.playhead(), c.wagon(0));
            case Reading::Onset:        return pc_onset(c.playhead(), c.wagon(0), onset_prev_beat_);
```

`Spine`'s onsets therefore surface as `Reading::Distance` and
`Reading::CurrentPC` — the line's signed interval and the current note — and not
as anything a ring would read.

#### Hop 1g — the one dispatcher that feeds 1e and 1f

**File:** `src/musical/context.hpp`
**Blob SHA:** `a4c3977e08b4bf8638b6072077995a095667ad5f`
**Enclosing symbol:** `Context::receive(const MidiEvent&)` inside `class Context`

```cpp
    // Route one event for this channel. The stream takes every event; the
    // enabled memories take onsets and offsets both — onsets to open and
    // group, offsets to record the releases their elections read.
    void receive(const MidiEvent& ev) {
        stream_.receive(ev);

        const bool is_on = (ev.type == MidiEvent::NOTE_ON);

        if (previous_active_) {
            if (is_on) previous_.on_onset(ev.pitch, ev.velocity, ev.beat);
            else       previous_.on_offset(ev.pitch, ev.beat);
        }
        if (spine_active_) {
            if (is_on) spine_.on_onset(ev.pitch, ev.beat);
            else       spine_.on_offset(ev.pitch, ev.beat);
        }
    }
```

Its one caller — `src/analysis/canvas_1/canvas.hpp`, blob
`250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5`, enclosing symbol
`Canvas1::route(const MidiEvent&)`:

```cpp
    // Route one event to the slot whose spec names its MIDI channel. The
    // dynamic half of the wiring; an event on a channel no slot listens to is
    // dropped. Both update()'s drain and a test's injection enter here.
    void route(const MidiEvent& ev) {
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (active_[i] && specs_[i].channel == ev.channel) {
                contexts_[i].receive(ev);
                return;
            }
        }
    }
```

**Status: PRESENT.** `rg -n '\.receive\(' src/` returns six lines: this call site,
the `stream_.receive(ev)` inside `Context::receive` itself, and four prose lines
in `context.hpp`, `midi_stream.hpp`, `keyboard_midi.hpp` and `midi_file.hpp`.
There is exactly one live route from a MIDI event into the two onset state
machines, and it terminates inside `src/musical/`.

**Summary of Hop 1 as a whole.** Three onset detectors exist at HEAD and all three
run: `pc_onset` (a per-frame reduction over `stream_data.hpp`'s records, published
as `chN.onset`), `PreviousEvent::on_onset` (a grouping latch, read by nothing
outside `src/musical/`), and `Spine::on_onset` (a line election, read as
`Reading::Distance` / `Reading::CurrentPC`). None of the three names a pulse:
`rg -in 'pulse' src/musical/` returns exactly one line, in `pc_count.hpp`
(recipe A′), and that line is the word "impulses" in `pc_onset`'s own header
comment quoted at the top of this hop.

### Hop 2 — ring buffer write

**Status: GONE.**

There is no ring, no circular store, and no write site anywhere under `src/`.
The evidence is an absence, so it is recorded twice — once as the failed census,
once as the structure that would have to contain it.

**The failed census (recipe G, R2):**

```
$ rg -n 'pulse_ring|pulse_write_idx|PULSE_RING_SIZE|PULSE_ONSET_THRESHOLD|PULSE_AMPLITUDE|PULSE_INCREASE_CLAMP|MMODE_RADIAL_PULSE' src/
$ echo $?
1
```

Zero output, exit status 1 (rg's "no match"). None of the six ring identifiers
resolves.

**The structure that shows the absence.** The only function that can put a pulse
into the config has exactly two occurrences in the tree (recipe B) — its own
definition, and one caller. The caller is a boot rest pin, not a per-frame write.

`src/cartridges/the_board/cartridge.hpp`, blob
`3651bcabaa0b02a2925ad6868ce541ea9ab1b202`, enclosing symbol `Cartridge::initialize`
(the boot rest-pin block, quoted whole so the surrounding pins are visible and
the absence of any other pulse write inside it is legible):

```cpp
                {
                    // The surface voice's terrain rows read THE
                    // TERRAIN_LOOKS PANEL ROW 2 (surface/
                    // terrain_looks.hpp) — the rest column lives where
                    // the parameters live. Values unchanged: blend -1
                    // = inactive, everything else 0.
                    gpuState_.set_band_motion(terrain_looks::REST_BAND_BLEND,
                        terrain_looks::REST_BAND_PHASE_ORIGIN);
                    gpuState_.set_terrain_time(terrain_looks::REST_TERRAIN_TIME);
                    gpuState_.set_mode_color_shift(terrain_looks::REST_MODE_COLOR_SHIFT);
                    gpuState_.set_mode_checker_scatter(terrain_looks::REST_MODE_CHECKER_SCATTER);
                    gpuState_.set_mode_palette_drift(terrain_looks::REST_MODE_PALETTE_DRIFT_TARGET,
                        terrain_looks::REST_MODE_PALETTE_DRIFT_INTENSITY,
                        terrain_looks::REST_MODE_PALETTE_DRIFT_TIER);
                    gpuState_.set_checker_color_field(terrain_looks::REST_CHECKER_RESULTANT,
                        terrain_looks::REST_CHECKER_AMOUNT,
                        terrain_looks::REST_CHECKER_VARIANCE);
                    gpuState_.set_mode_gol_scales(1.0f, 1.0f);   // GoL's jurisdiction — stays inline (ROW 9 pointer)
                    // Pulse ring rest — the count is a ROW 2 pin; the
                    // zeroed ring is the rest (Phase 1, C4-F1).
                    float zero_pulses[32] = {};
                    gpuState_.set_pulse_data(terrain_looks::REST_PULSE_COUNT, zero_pulses);
```

**The tree states the absence in its own words.** `src/cartridges/the_board/cartridge.hpp`,
same blob, enclosing symbol `Cartridge::phase_live_card_write(RenderCtx&)` — its
header comment names the ring's only writer. Quoted whole through the end of the
OPT_1a paragraph (`grep -n "OPT_1a — THE REST SKIP" -A 11
src/cartridges/the_board/cartridge.hpp`); the paragraph that follows it in the
source, headed `// SPINE_2 B — THE DECISION STAYS, THE PASS LEAVES.`, concerns
dispatch ordering and is not quoted:

```cpp
            // OPT_1a — THE REST SKIP: both dispatches (819,200 invocations)
            // are skipped while the card's field is at rest. The full
            // three-conjunct rest law, evaluated CPU-side and conservative
            // (any doubt => write): no GoL zone live, pulse ring empty,
            // terrain_time <= 0. Post-CUT_1 the last two are structurally
            // pinned at rest (O0-d: the ring's only writer is the boot
            // zero-pin; terrain_time's only writers pin 0.0) — checked
            // anyway so a future re-arming of either conjunct wakes the
            // writer without an edit here. On the transition into rest, ONE
            // final write runs so consumers never read stale non-zero
            // texels; the flag resets at world teardown so a fresh world's
            // rest field is written once too.
```

(The last four lines describe the single clearing write whose CPU implementation
— the `liveCardRestClean_` branch — is quoted separately at Hop 6.)

**And the shader states it too.** `src/cartridges/the_board/realization/world.wgsl`,
blob `5b36243dc6b45e27271d0d73eca8a01eb5dc2078`, in the contributor note directly
above `fn contrib_radial_pulses_at`:

```wgsl
// DRIVERLESS since gen-1 retirement — held at neutral by the boot
// block; revive via a gen-2 coupling or delete on the next pass here.
```

**The origin source is gone too.** The historical ring write took its ring
origin from a pawn position readback. Recipe H (`rg -n 'possessed|pawn_pos|player_\.|readback'
src/coupling/`) returns nothing, exit status 1: the coupling layer has no access
to any pawn position at HEAD. `rg -n 'readback_x|readback_z' src/` likewise
returns nothing.

**The polyphony driver is unwired.** The historical onset test was a polyphony
rise. `Reading::Polyphony` still exists as an enum member and has a slot spec —
`src/analysis/canvas_1/canvas.hpp`, enclosing symbol `Canvas1::reading_spec(Reading)`:

```cpp
            case Reading::Polyphony:     return { SLOT_POLYPHONY,       1, StatShape::Scalar };
```

— but `rg -n 'Reading::Polyphony' src/` returns that one line only. It appears
in no `publish_reading` call (the full list of `publish_reading` call sites is
`rg -n 'publish_reading' src/analysis/canvas_1/canvas.hpp`, and Polyphony is
absent from all of them), and `Canvas1::write_reading` has no `case` for it, so
it falls to:

```cpp
            default:
                // Unreachable: publish_reading refuses any reading not wired here
                // (see writer_wired). The remaining readings stay as capability.
                break;
```

`Canvas1::writer_wired` (quoted at Hop 1) does not list `Reading::Polyphony`, so
publication is refused by contract.

### Hop 3 — `pulse_count` upload

**File:** `src/cartridges/the_board/realization/state.hpp`
**Blob SHA:** `fe4bce836b4665588ed43a0729d312e89cd05a20`
**Enclosing symbol:** `struct GPUDesignConfig` (member `pulse_count`)

```cpp
            // ─── Radial pulse ring buffer ────────────────────────────────
            uint32_t pulse_count;             // active entries (0–8)
            // ─── Agent system ────────────────────────────────────────────
            // Slot index of the player's current body in agent_state[].
            // Piggybacks on the existing _pulse_pad triple (size witnessed by the sizeof static_assert below — 560).
            uint32_t possessed_slot;          // slot 0 at session start
```

**Status: STALE.**

The word exists in the uniform struct, is uploaded every dirty frame, and is
read by the shader — but the only value that ever enters it is a named
compile-time zero.

Enclosing symbol: `GPUState::set_pulse_data(uint32_t, const float[32])` — the
sole authoring wire (recipe B: two occurrences in `src/`, this and its one call):

```cpp
            void set_pulse_data(uint32_t count, const float data[32]) {
                config_.pulse_count = count;
                std::memcpy(config_.pulse_data, data, 32 * sizeof(float));
                configDirty_ = true;
            }
```

Enclosing symbol: `GPUState::upload_config(wgpu::Queue&)` — the transfer to the
GPU (whole-struct, dirty-driven, so `pulse_count` genuinely reaches the device
every time the config is dirty):

```cpp
            void upload_config(wgpu::Queue& queue) {
                if (!configDirty_ && !configDynamic_) return;
                configDirty_ = false;
                writeStruct(queue, configBuffer_, config_);   // Shape A, dirty-driven — the canonical cadence
            }
```

Its per-frame caller — `src/cartridges/the_board/cartridge.hpp`, enclosing symbol
`Cartridge::phase_stage_fade_and_upload(UpdateCtx&)`:

```cpp
            // U8 — STAGE FADE + THE TWO UPLOADS (O-5b/c). Fade after the machine
            // (alpha is current-frame); upload_signal then upload_config AFTER
            // all staging setters — the O-5b/c face law, enforced by
            // validate_spine at boot.
            void phase_stage_fade_and_upload(UpdateCtx& c) {
                auto& gpuSignal = c.gpuSignal;
                auto& queue = c.queue;
                gpuState_.set_fade(mood_state_.transition_fade_alpha, 0.0f, 0.0f, 0.0f);
                gpuState_.upload_signal(queue, gpuSignal);
                gpuState_.upload_config(queue);
            }
```

The value it carries is fixed at two sites, both zero.

Site 1 — `src/cartridges/the_board/surface/terrain_looks.hpp`, blob
`0c7cc03b73dc4994bf21416817c8891849d7bdce`, enclosing symbol `namespace
terrain_looks` (ROW 2 rest column):

```cpp
// Pulse ring rest: count 0 with a zeroed ring IS the rest (the boot
// pin sources it from here).
inline constexpr std::uint32_t REST_PULSE_COUNT = 0;
```

Site 2 — `src/cartridges/the_board/realization/state.hpp`, enclosing symbol
`GPUState::initializeState()` (declared `bool initializeState() {` inside
`class GPUState`, itself inside `namespace the_board` inside `namespace t7`).
These are **runtime assignments in that member function's body**, not default
member initializers on `struct alignas(16) GPUDesignConfig`; they run once at
world boot, alongside the rest of the config's opening values, and the
`Cartridge::initialize` rest pin quoted at Hop 2 re-states the same zero later:

```cpp
                config_.pulse_count = 0;
                for (int i = 0; i < 32; i++) config_.pulse_data[i] = 0.0f;
```

Their immediate neighbours in the same function body, quoted so the enclosing
scope is legible without a line number (`grep -n "config_.pulse_count = 0;" -B 2
-A 2 src/cartridges/the_board/realization/state.hpp`):

```cpp
                config_.mode_gol_tick_scale = 1.0f;
                config_.mode_gol_height_scale = 1.0f;
                config_.pulse_count = 0;
                for (int i = 0; i < 32; i++) config_.pulse_data[i] = 0.0f;
                config_.possessed_slot = 0;  // player starts in slot 0
```

**Correction of record (R3).** An earlier draft of this section cited Site 2's
enclosing symbol as "`GPUState`'s config-defaults initializer (the struct's own
boot value)". No such symbol exists; the census that settles it is
`grep -n "bool initializeState()" src/cartridges/the_board/realization/state.hpp`
(one hit) together with a line-number-free scan of the span between that opener
and the assignment for any intervening member-function opener:

```
$ sed -n '/bool initializeState() {/,/config_\.pulse_count = 0;/p' \
      src/cartridges/the_board/realization/state.hpp \
  | grep -nE '^            [A-Za-z_].*\) *\{ *$'
1:            bool initializeState() {
```

One hit, and it is `initializeState` itself — no other member function opens
between the declaration and the assignment, so the assignment is inside its body.
The STALE verdict is unaffected: both sites still put zero into `pulse_count`.

The one CPU-side *reader* of the uploaded count is a rest-law conjunct, not a
consumer of pulse content — `src/cartridges/the_board/cartridge.hpp`, enclosing
symbol `Cartridge::live_card_is_live() const`:

```cpp
            bool live_card_is_live() const {
                if (gpuState_.config().pulse_count > 0) return true;
                if (gpuState_.config().terrain_time > 0.0f) return true;
                for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES; i++)
                    if (gol_state_.zones[i].active) return true;
                return false;
            }
```

Because `pulse_count` is pinned at 0, this conjunct can never be the thing that
makes the card live.

### Hop 4 — `pulse_data[]` upload (actual width: 32 floats = 8 × vec4)

**File:** `src/cartridges/the_board/realization/state.hpp`
**Blob SHA:** `fe4bce836b4665588ed43a0729d312e89cd05a20`
**Enclosing symbol:** `struct GPUDesignConfig` (member `pulse_data`)

```cpp
            // THE RIM taste knob (config-gated): 0 = icing tints to fog
            // (default); >0.5 = icing DITHER-dissolves (geometry condenses
            // instead of tinting) at the two icing FS sites. Repurposes one
            // of the pulse pad floats — no struct-size delta.
            float veil_dither;
            // Indoor GoL height cap (0 = disabled): zone_derive_params bounds
            // each new zone's alive_height so the MAXIMUM realised cell lift
            // equals this exactly (it divides by the height_factor clamp
            // bound to get there). Staged by
            // apply_mood_lighting — INDOOR_HEIGHT_CAP_FRACTION ×
            // ceiling_height indoors, 0 elsewhere. Repurposes the last
            // pulse pad float — no struct-size delta (the sizeof witness
            // 560 stands).
            float indoor_height_cap;
            float pulse_data[32];             // 8 × {origin_x, origin_z, onset_seconds, amplitude}
```

**Status: STALE.**

**The width, stated with its recipe.** The unit asked whether the array is 8
wide. It is **8 slots**, expressed CPU-side as a flat **`float[32]`** (8 × 4
floats) and GPU-side as **`array<vec4<f32>, 8>`**. Both declarations are quoted
here and at Hop 5; the census that found them is recipe C
(`rg -n 'pulse_count|pulse_data' src/` → 15 lines).

The array reaches the GPU by exactly the same route as the count — the
`std::memcpy` inside `GPUState::set_pulse_data` (quoted at Hop 3), then
`upload_config`'s whole-struct `writeStruct`. Its only caller supplies a
stack-zeroed buffer, quoted whole at Hop 2:

```cpp
                    float zero_pulses[32] = {};
                    gpuState_.set_pulse_data(terrain_looks::REST_PULSE_COUNT, zero_pulses);
```

**The three pad floats are already spent.** The struct comments above record
that three of the ring's neighbouring pad words were repurposed —
`possessed_slot` ("Piggybacks on the existing _pulse_pad triple"), `veil_dither`
("Repurposes one of the pulse pad floats"), and `indoor_height_cap`
("Repurposes the last pulse pad float"). The 32-float array itself is untouched
by those repurposings; only the pad triple that sat beside it was taken.

### Hop 5 — shader read

**File:** `src/cartridges/the_board/realization/world.wgsl`
**Blob SHA:** `5b36243dc6b45e27271d0d73eca8a01eb5dc2078`
**Enclosing symbols:** `struct DesignConfig` (the WGSL mirror declaration) and
`fn contrib_radial_pulses_at(vec2<f32>, f32) -> f32` (the read)

**The mirror's name, with its recipe (R3).** The WGSL struct that declares
`pulse_count` and `pulse_data` is `DesignConfig`, not `Config`; `config` is the
name of the *uniform variable* of that type. There is no `struct Config` in the
file:

```
$ rg -n '^struct Config\b|struct Config ' src/cartridges/the_board/realization/world.wgsl
$ echo $?
1
$ rg -n 'struct DesignConfig|config: DesignConfig' src/cartridges/the_board/realization/world.wgsl
1649:struct DesignConfig {
7028:@group(0) @binding(0)   var<uniform>             config: DesignConfig;
11616:@group(0) @binding(0)   var<uniform>             fc_config: DesignConfig;
12251:@group(2) @binding(183) var<uniform>             cmg_config: DesignConfig;
```

The C++ side names the same pairing in its own words —
`src/cartridges/the_board/realization/state.hpp`, blob
`fe4bce836b4665588ed43a0729d312e89cd05a20`, the comment immediately above
`struct alignas(16) GPUDesignConfig`:

```cpp
        // DesignConfig mirrors this struct field-for-field. Adding a
        // knob: THE GROWTH LAW, L5 in docs/LAWS.md. Choosing where
        // to put it: THE ALIGNMENT LAW, L4.
        struct alignas(16) GPUDesignConfig {
```

So `config.pulse_count` in the read below resolves as: uniform `config` (binding
`@group(0) @binding(0)`) of type `struct DesignConfig`, member `pulse_count`.
Three uniforms of that type exist in the module — `config`, `fc_config`,
`cmg_config` — and `contrib_radial_pulses_at` reads the first.

The mirror declaration — enclosing symbol `struct DesignConfig`:

```wgsl
    // ─── Radial pulse ring buffer ────────────────────────────────
    pulse_count: u32,
    // Agent system: slot index of the player's current body in
    // agent_state[]. Piggybacks on the radial-pulse pad triple (no
    // struct size delta). Order matches GPUDesignConfig in state.hpp.
    possessed_slot: u32,
    veil_dither: f32,     // THE RIM taste knob: >0.5 → icing dither-dissolves (mirror of GPUDesignConfig)
    indoor_height_cap: f32,  // indoor cap on the GoL cell lift, 0 = disabled. READER: zone_derive_params, once per zone birth (mirror of GPUDesignConfig — last pulse pad repurposed)
    pulse_data: array<vec4<f32>, 8>,  // each: (origin_x, origin_z, onset_seconds, amplitude)
```

The read — enclosing symbol `fn contrib_radial_pulses_at`, quoted whole with its
dial panel and contributor note:

```wgsl
// ─── Radial pulses: expanding ring wavefronts from note onsets ──────────
//
// Each pulse is an expanding ring centered on the pawn's position at onset.
// The wavefront radius grows with time; a gaussian envelope makes the ring
// thin. Distance damping and age decay give natural falloff.
//
// Cost: 8 iterations per evaluation point (one per ring buffer slot).
// Dead entries (age > max or amplitude = 0) early-exit cheaply.

const PULSE_SPEED: f32 = 30.0;         // world units per second (ring expansion rate)
const PULSE_MAX_AGE: f32 = 8.0;        // seconds — pulses older than this are ignored
const PULSE_RING_SHARPNESS: f32 = 0.3; // gaussian falloff around wavefront (lower = wider ring)
const PULSE_DAMPING: f32 = 0.012;      // distance damping (attenuation per world unit)
const PULSE_AGE_DECAY: f32 = 0.4;      // age damping (1/seconds — half amplitude at ~1.7s)

// CONTRIB_RADIAL_PULSES — deformation_field, global.
// Contributes: sum of expanding gaussian ring wavefronts from note onsets.
// Dependencies (via DAG): none — orthogonal to the static stack.
// Notes: t_seconds is an explicit parameter so the contributor works in
//   both render stages and compute stages
//   (signal.t_seconds). 8-slot ring buffer; dead entries early-exit.
// DRIVERLESS since gen-1 retirement — held at neutral by the boot
// block; revive via a gen-2 coupling or delete on the next pass here.
fn contrib_radial_pulses_at(world_xz: vec2<f32>, t_seconds: f32) -> f32 {
    if (config.pulse_count == 0u) { return 0.0; }

    var h: f32 = 0.0;
    let count = min(config.pulse_count, 8u);

    for (var i: u32 = 0u; i < count; i++) {
        let p = config.pulse_data[i];  // (origin_x, origin_z, onset_seconds, amplitude)
        let age = t_seconds - p.z;
        if (age < 0.0 || age > PULSE_MAX_AGE || p.w < 0.001) { continue; }

        let dist = length(world_xz - p.xy);

        // Expanding ring: wavefront at radius = age × speed
        let wavefront_r = age * PULSE_SPEED;
        let ring_dist = dist - wavefront_r;

        // Gaussian ring envelope (sharp peak at wavefront)
        let ring = exp(-ring_dist * ring_dist * PULSE_RING_SHARPNESS);

        // Damping: distance from origin + age
        let spatial_damp = exp(-dist * PULSE_DAMPING);
        let age_damp = exp(-age * PULSE_AGE_DECAY);

        h += p.w * ring * spatial_damp * age_damp;
    }

    return h;
}
```

**Status: STALE.**

The function is structurally complete and compiles as part of the module (it is
carried in `audit/BINDING_LEDGER.md` as a `law-ref`, recipe I). Its first
statement, `if (config.pulse_count == 0u) { return 0.0; }`, is unconditionally
taken at HEAD, because the only value `pulse_count` can hold is
`terrain_looks::REST_PULSE_COUNT` (Hop 3). The loop body, the five dials, and
`pulse_data` are unreachable at runtime.

The dial panel is also the target of two navigation comments in the same file,
enclosing symbol: the module header index and the `TERRAIN_LOOKS` panel index —
recorded because they are the shader's own map of the slice:

```wgsl
// ── Radial Pulses (§3.5) ─────────────────────────────────────────
//   PULSE_SPEED / MAX_AGE / DAMPING  Ring dynamics
```

```wgsl
//   Radial pulses (music-onset rings): the PULSE_SPEED..PULSE_AGE_DECAY
//     dials directly above fn contrib_radial_pulses_at.
```

### Hop 6 — shader use

**File:** `src/cartridges/the_board/realization/world.wgsl`
**Blob SHA:** `5b36243dc6b45e27271d0d73eca8a01eb5dc2078`
**Enclosing symbol:** `@compute @workgroup_size(16, 16) fn write_live_card(...)`

Recipe D (`rg -n 'contrib_radial_pulses_at' src/`, 5 lines) resolves to exactly
**one call site** in the whole tree; the other four hits are the definition and
three prose comments. The call:

```wgsl
    // 400 tile texels over 256 threads — the resolve's own load loop,
    // evaluating where it used to fetch.
    for (var k = t; k < 400u; k += 256u) {
        let tx = tile_x0 + i32(k % 20u);
        let ty = tile_y0 + i32(k / 20u);
        // Beyond the window is still the field: no clamp, no edge case.
        let p = origin + (vec2<f32>(f32(tx), f32(ty)) + vec2(0.5)) * texel;
        var dh = 0.0;
        if (config.terrain_time > 0.0) {
            let af = terrain_activity_at(p, config.world_seed);
            for (var b = 0u; b < TERRAIN_BAND_COUNT; b++) {
                if (b == 4u) { continue; }   // the fine ripple stays bake-only —
                                             // the Nyquist ruling (campaign v2 §6)
                let blend = get_band_blend(b);
                if (blend <= 0.0) { continue; }   // −1 sentinel + 0
                let t_eff = config.terrain_time - get_band_phase_origin(b);
                dh += clamp(blend, 0.0, 1.0)
                    * true_band_delta_contribution(p, config.world_seed,
                          t_eff, b, af.x, af.y);
            }
        }
        dh += contrib_radial_pulses_at(p, signal.t_seconds);
        sh_card_h[k] = dh;
    }
```

and the store, same enclosing symbol, where `dh` becomes the card's `.x` and its
central difference becomes `.yz`. Quoted contiguously and without elision
(`grep -n "let height = sh_card_h\[cy \* 20u + cx\];" -A 14
src/cartridges/the_board/realization/world.wgsl`), through the closing brace of
`fn write_live_card`:

```wgsl
    let height = sh_card_h[cy * 20u + cx];

    // eps = texel-CENTER spacing (extent / res): the card maps texel
    // centers across the window, unlike the bake's corner-pinned
    // (res − 1) grid — the one mapping difference from the model.
    let eps = texel;
    let grad_x = (sh_card_h[cy * 20u + cx + 1u] - sh_card_h[cy * 20u + cx - 1u]) / (2.0 * eps);
    let grad_z = (sh_card_h[(cy + 1u) * 20u + cx] - sh_card_h[(cy - 1u) * 20u + cx]) / (2.0 * eps);

    // .a runs once per INTERIOR texel, as it did — pass 1 wrote it per
    // texel and pass 2 copied it across; the halo never needed it.
    let p_here = origin + (vec2<f32>(f32(ix), f32(iy)) + vec2(0.5)) * texel;
    textureStore(live_card_write, vec2<i32>(i32(ix), i32(iy)),
                 vec4(height, grad_x, grad_z, contrib_gol_zones_at(p_here)));
}
```

(`p_here` — the world position of the interior texel this invocation stores — is
bound by the `let` three lines above its use, and is a different quantity from
the halo-inclusive `p` of the load loop above. The intervening bindings, same
enclosing symbol, quoted so the two coordinate families are distinguishable:

```wgsl
    workgroupBarrier();                                   // 3: the tile

    let ix = wid.x * 16u + lid.x;
    let iy = wid.y * 16u + lid.y;
    if (ix >= LIVE_CARD_SIZE || iy >= LIVE_CARD_SIZE) { return; }   // after the barrier

    let cx = lid.x + 2u;
    let cy = lid.y + 2u;
```

`p` is built from `tx`/`ty`, which run over the 20×20 tile including the 2-texel
halo; `p_here` is built from `ix`/`iy`, which run over the 16×16 interior only.
Both are `origin + (texel-center offset) * texel`.)

**Status: STALE.**

The path from the call to a rendered value is intact and multiply realized:

Enclosing symbol `fn manifold_overlay_stack(vec2<f32>, QueryInputs, f32) -> f32`
— the shared dynamic-overlay fold every dynamic ground policy uses:

```wgsl
fn manifold_overlay_stack(xz: vec2<f32>, qi: QueryInputs, gol_term: f32) -> f32 {
    // (qi retained for signature stability; pulses now ride the card.)
    var h = sample_terrain_y_at(xz);   // base(p): static base + pyramids (baked)
    h += gol_term;                     // GoL before waves — historical operand order kept
    h += sample_live_card(xz).x;       // live(p).x: waves + pulses (the card)
    return h;
}
```

Enclosing symbol `@vertex fn patch_terrain_vs(...)` — the rendered terrain
vertex position and its shading normal:

```wgsl
    // The live card (GROUND_CARD_1): waves + pulses ride one field —
    // live.x = Δh (true-band waves + pulses), live.yz = the full-Δ
    // gradient (TRUEBAND_CONTACT_1: the resolve differentiates the
    // whole scratch — normals shade pulses AND bands)
    // (parity with the old fused overlay; pulse shading = Stage 6).
    let live = sample_live_card(world_pos.xz);
    world_pos.y += live.x;
```

The contributor is also a declared member of six of the seven ground policies.
`src/cartridges/the_board/contracts/ground_architecture.hpp`, blob
`0488bc642c5993d6e32310d2fe1bd3fe6d41a3d2`, enclosing symbol `enum ContributorId`:

```cpp
    CONTRIB_RADIAL_PULSES     = 6,   // deformation_field, global
```

and enclosing symbol `POLICIES[]` (the six rows that set the bit). Recipe E's 14
lines decompose exactly: **1** enum member + **6** mask terms, all in
`ground_architecture.hpp`, and **7** comment lines in `world.wgsl`
(`rg -n 'CONTRIB_RADIAL_PULSES' src/` — the WGSL seven are the contributor note
above `contrib_radial_pulses_at`, five policy-set recitals above the
`query_ground_*` functions, and one in the `patch_terrain_vs` header). The six
mask rows:

```cpp
    { POLICY_FLYER, "flyer",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA) },
```

```cpp
    { POLICY_WALKER, "walker",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA)
        | (1u << CONTRIB_GOL_SUPPRESSION) },
```

```cpp
    { POLICY_WALKER_TILT, "walker_tilt",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_GOL_SUPPRESSION) },  // pawn-centered; same suppression walker applies
```

```cpp
    { POLICY_WALKER_AGENT, "walker_agent",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA) },
```

```cpp
    { POLICY_TERRAIN_RENDER, "terrain_render",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_GOL_ZONES)   // realized as the card's .a, cell-nearest, suppressed under the pawn AND the eye — UNIFIED_GROUND_1 + KITE_1 (DAG: GoL has no ancestors)
        | (1u << CONTRIB_PAWN_AURA) },                  // realized in the fused VS (texture .yz + analytic wave gradient)
```

```cpp
    { POLICY_WALKER_WITNESS, "walker_witness",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA)          // external form — the witness is not the pawn
        | (1u << CONTRIB_GOL_SUPPRESSION) }, // eye-centered, height-faded
```

`POLICY_BAKED_HEIGHTFIELD` is the only policy of the seven that does not carry
the bit.

**Why STALE rather than PRESENT.** The value that reaches `dh` is
`contrib_radial_pulses_at`'s early return `0.0` (Hop 5), so every downstream
addition adds exactly zero. Beyond that, at HEAD the kernel containing the call
is normally **not dispatched at all**: `Cartridge::phase_live_card_write` returns
before the dispatch whenever the rest law holds, and two of its three conjuncts
are the pinned ones.

The shader states this in its own words, in a **file-scope prose block** whose
title line is `// THE CARD WRITER (TRUEBAND_CONTACT_1 T1b, fused at LATTICE_4).
ONE`. **That block is not adjacent to the declaration it describes (R3).** Its
nearest preceding declaration is `fn sample_live_card_gol(world_xz: vec2<f32>) ->
f32`, whose closing brace it immediately follows; between the end of the block
(`// frozen => a woken band grows out of the frozen shape.`) and the
`@compute @workgroup_size(16, 16)` / `fn write_live_card(` declaration stand the
whole `// ─── THE CARD'S NODE TABLE (LATTICE_4) ───` section (the `CARD_NODES_*`
/ `CARD_TABLE_OFF` / `CARD_WORKGROUP_BYTES` consts and their const_asserts) and
two function definitions, `fn card_band_of(k: u32) -> u32` and
`fn card_nodes_n(b: u32) -> u32`. A reader looking directly above
`fn write_live_card` finds `fn card_nodes_n`, not this comment. The recipe that
establishes the ordering:

```
$ grep -n "fn sample_live_card_gol\|THE CARD WRITER (TRUEBAND_CONTACT_1 T1b\|THE REST LAW IS A CONJUNCTION\|frozen => a woken band grows out of the frozen shape.\|THE CARD'S NODE TABLE\|fn card_band_of\|fn card_nodes_n\|fn write_live_card" \
      src/cartridges/the_board/realization/world.wgsl
10989:fn sample_live_card_gol(world_xz: vec2<f32>) -> f32 {
10994:// THE CARD WRITER (TRUEBAND_CONTACT_1 T1b, fused at LATTICE_4). ONE
11016:// THE REST LAW IS A CONJUNCTION, and only two of its three conjuncts
11037:// frozen => a woken band grows out of the frozen shape.
11039:// ─── THE CARD'S NODE TABLE (LATTICE_4) ──────────────────────────────
11098:fn card_band_of(k: u32) -> u32 {
11106:fn card_nodes_n(b: u32) -> u32 {
11117:fn write_live_card(
```

(Line numbers appear here only as the ordering evidence the recipe prints; no
claim in this section is anchored to one.)

The block's opening, quoted so its identity is fixed by its own text rather than
by position:

```wgsl
// THE CARD WRITER (TRUEBAND_CONTACT_1 T1b, fused at LATTICE_4). ONE
// kernel: each workgroup evaluates its own 20x20 tile (16x16 interior +
// a 2-texel halo) into `sh_card_h`, barriers, and stores
// vec4(h, gx, gz, gol) for the interior.
```

The rest-law paragraph, quoted verbatim from inside that same file-scope block:

```wgsl
// THE REST LAW IS A CONJUNCTION, and only two of its three conjuncts
// are MUSICAL. The card is zero — and every consumer therefore adds 0 —
// only when ALL of:
//   (1) config.terrain_time <= 0        the band sum is gated off  [MUSICAL]
//   (2) the pulse ring is empty         contrib_radial_pulses_at is
//       (pulse_count == 0, or every     added OUTSIDE that gate, on its
//       slot aged out / zero-amp)       own clock signal.t_seconds  [MUSICAL]
//   (3) no zone covers the texel        contrib_gol_zones_at feeds .a
//       (zone_config.count == 0, or     with no gate at all, and a zone
//       no covering zone has            runs on ITS OWN tick clock —
//       alive_height >= 0.01 and        beats, not the music's voice.
//       transition_fraction > 0)        [NOT MUSICAL]
// Conjunct (3) is why the one-way "terrain_time <= 0 => zeros" claim was
// tempting and wrong: silence the music and a living zone still lifts.
// Boot pins all three: REST_TERRAIN_TIME and REST_PULSE_COUNT
// (surface/terrain_looks.hpp ROW 2) and an empty zone table. The rest law
// is enforced by the CALLER — phase_live_card_write returns before the
// dispatch when the card is clean (liveCardRestClean_), so this kernel
// never runs at rest and inherits the law unchanged.
```

The CPU half of the same law, enclosing symbol
`Cartridge::phase_live_card_write(RenderCtx&)`:

```cpp
                if (card_live) {
                    liveCardRestClean_ = false;      // live: write every frame
                } else if (liveCardRestClean_) {
                    liveCardWritePending_ = false;   // at rest, card clean: skip
                    return;
                } else {
                    liveCardRestClean_ = true;       // entering rest: one clearing write
                }
```

A live GoL zone (conjunct 3, non-musical) can still make the card live and cause
`write_live_card` to run; in that case `contrib_radial_pulses_at` executes and
returns `0.0`.

### 6.7 Hop table

| hop | site | file | blob SHA | enclosing symbol | status |
| --- | --- | --- | --- | --- | --- |
| 1 | onset detection | `src/musical/pc_count.hpp` | `acccba766e2a9b05b3d03f99ce3a238b6376dd9a` | `pc_onset` | **PRESENT** |
| 1b | onset publication | `src/analysis/canvas_1/canvas.hpp` | `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5` | `Canvas1::write_reading` / `SLOT_ONSET` / `NAME_ONSET` / `publish_reading(Reading::Onset, …)` | **PRESENT** |
| 1c | `chN.onset`'s only consumer | `src/coupling/visual_canvas.hpp` | `ab5a21993a98c5dcc096f38b5db0c16cb7afbf35` | `VisualCanvas` zoetrope ears → `zoetrope_rows_[7]` | **PRESENT** (terminates in the zoetrope, not the ring) |
| 1d | the raw onset records `pc_onset` reads | `src/musical/stream_data.hpp` | `d84086d83e06113e5284caa2aa6dc24b304cbd5f` | `ActiveNote::onset_beat`, `CompletedNote::onset_beat`, `ActiveSet::note_on`, `ActiveSet::onset` | **PRESENT** (names no pulse) |
| 1e | second onset detector | `src/musical/previous_event.hpp` | `b97550ce0566bb351c36f2dfa8eeead803bde808` | `PreviousEvent::on_onset(int, float, float)`; `struct PrevNote` | **PRESENT** as a detector; **no consumer outside `src/musical/`** (census: `rg -n 'previous\(\)\|PreviousEvent\|previous_ops' src/analysis/ src/coupling/ src/cartridges/` → exit 1) |
| 1f | third onset detector | `src/musical/spine.hpp` | `8e9c144193e49019195d3526ed08f3c27486fb53` | `Spine::on_onset(int, float)` | **PRESENT**; consumed as `Reading::Distance` / `Reading::CurrentPC`, not as a pulse |
| 1g | the one dispatcher feeding 1e and 1f | `src/musical/context.hpp` | `a4c3977e08b4bf8638b6072077995a095667ad5f` | `Context::receive(const MidiEvent&)`; enabled by `realize()` in `src/musical/context_realize.hpp` (blob `4285730324237ed01ed9fbd9e624d9bd4896417e`) | **PRESENT** (sole caller `Canvas1::route(const MidiEvent&)`) |
| 2 | ring buffer write | — | — | — | **GONE** (recipe G, exit 1) |
| 3 | `pulse_count` upload | `src/cartridges/the_board/realization/state.hpp` | `fe4bce836b4665588ed43a0729d312e89cd05a20` | `GPUDesignConfig::pulse_count`, `GPUState::set_pulse_data`, `GPUState::upload_config`, `GPUState::initializeState` (the boot zero) | **STALE** (pinned `REST_PULSE_COUNT = 0`) |
| 4 | `pulse_data[32]` upload (8 × vec4) | `src/cartridges/the_board/realization/state.hpp` | `fe4bce836b4665588ed43a0729d312e89cd05a20` | `GPUDesignConfig::pulse_data`, `GPUState::set_pulse_data` | **STALE** (memcpy of `float zero_pulses[32] = {}`) |
| 4b | the sole caller | `src/cartridges/the_board/cartridge.hpp` | `3651bcabaa0b02a2925ad6868ce541ea9ab1b202` | `Cartridge::initialize` boot rest-pin block | **STALE** (boot-only, zero) |
| 5 | shader read | `src/cartridges/the_board/realization/world.wgsl` | `5b36243dc6b45e27271d0d73eca8a01eb5dc2078` | `struct DesignConfig` (the WGSL mirror; reached through the uniform `config`, `@group(0) @binding(0)`); `fn contrib_radial_pulses_at` | **STALE** (early `return 0.0`) |
| 6 | shader use | `src/cartridges/the_board/realization/world.wgsl` | `5b36243dc6b45e27271d0d73eca8a01eb5dc2078` | `fn write_live_card` → `sh_card_h`/`live_card_write`; consumed by `manifold_overlay_stack`, `patch_terrain_vs`. The rest-law prose is at WGSL **file scope** in the block titled `// THE CARD WRITER (TRUEBAND_CONTACT_1 T1b, fused at LATTICE_4)`, not adjacent to `fn write_live_card` | **STALE** (adds `0.0`; kernel skipped by the rest law) |
| 6b | contributor declaration | `src/cartridges/the_board/contracts/ground_architecture.hpp` | `0488bc642c5993d6e32310d2fe1bd3fe6d41a3d2` | `ContributorId::CONTRIB_RADIAL_PULSES`; six `POLICIES[]` rows | **PRESENT** (the declaration is live law; the value it declares is stale) |

### The break

**Last living hop — Hop 1, the published onset.** `src/analysis/canvas_1/canvas.hpp`,
blob `250b6e56310c3b7c7978eb12ade3ab9a07e7a1a5`, enclosing symbol
`Canvas1::per_channel_reading(Reading, int)`, with its publication and its names:

```cpp
            case Reading::Onset:        return pc_onset(c.playhead(), c.wagon(0), onset_prev_beat_);
```

```cpp
            publish_reading(Reading::Onset,        Source::channel(v), NAME_ONSET[v]);
```

```cpp
    static constexpr const char* NAME_ONSET[MAX_CHANNELS] = {
        "ch0.onset","ch1.onset","ch2.onset","ch3.onset",
        "ch4.onset","ch5.onset","ch6.onset","ch7.onset"
    };
```

**First dead hop — Hop 2, the ring buffer write.** There is no code to quote,
because there is none; the evidenced absence is the census that finds nothing
and the one structure that would have to hold it. The census:

```
$ rg -n 'pulse_ring|pulse_write_idx|PULSE_RING_SIZE|PULSE_ONSET_THRESHOLD|PULSE_AMPLITUDE|PULSE_INCREASE_CLAMP|MMODE_RADIAL_PULSE' src/
$ echo $?
1
```

and the structure — `src/cartridges/the_board/cartridge.hpp`, blob
`3651bcabaa0b02a2925ad6868ce541ea9ab1b202`, enclosing symbol
`Cartridge::initialize`, the only place in the tree from which a pulse can enter
the config, holding a stack-zeroed buffer and a named constant zero:

```cpp
                    // Pulse ring rest — the count is a ROW 2 pin; the
                    // zeroed ring is the rest (Phase 1, C4-F1).
                    float zero_pulses[32] = {};
                    gpuState_.set_pulse_data(terrain_looks::REST_PULSE_COUNT, zero_pulses);
```

**Which name fails to resolve into what.** `chN.onset` resolves: the analysis
cartridge computes it (`pc_onset`), publishes it under `NAME_ONSET[v]` at
`SLOT_ONSET`, and `VisualCanvas` resolves those eight names into
`zoetrope_ears_[8]` and folds them into `zoetrope_rows_[7]`. From there nothing
resolves onward: there is no name in the tree for a pulse ring store
(`pulse_ring`, `pulse_write_idx`, `PULSE_RING_SIZE` — none of them exist,
recipe G), no name in `src/coupling/` for a pulse target (recipe A′: the coupling
layer's only "pulse" hits are the substring inside "impulses"), and no
per-frame caller for `GPUState::set_pulse_data` (recipe B: two occurrences, the
definition and one boot-time call). The chain therefore does not break at a
mismatched name — it breaks at a *missing* one: the onset value is fully
computed and fully published, and the ring's authoring wire is fully built and
fully uploaded, but between the published `chN.onset` slots and
`set_pulse_data`'s parameters there is no code, no name, and no binding of any
kind. `contrib_radial_pulses_at` therefore reads a count that only ever holds
`terrain_looks::REST_PULSE_COUNT`, returns at its first statement, and the
`dh += contrib_radial_pulses_at(p, signal.t_seconds)` inside `fn write_live_card`
adds zero to every texel of the card — in the frames where the rest law lets the
kernel run at all. The two ends of the slice are both intact; the middle is
absent, and both ends say so in their own comments ("the ring's only writer is
the boot zero-pin" on the C++ side, "DRIVERLESS since gen-1 retirement" on the
WGSL side).

**The break holds for the other two onset detectors as well.** `chN.onset` is not
the only onset signal at HEAD — `PreviousEvent::on_onset` and `Spine::on_onset`
(Hops 1e, 1f) also run, fed by `Context::receive` (Hop 1g). Neither reaches the
ring either, and the failure mode is the same missing-name one, evidenced by two
censuses. `PreviousEvent`'s outputs have no reader at all outside `src/musical/`
(`rg -n 'previous\(\)|PreviousEvent|previous_ops' src/analysis/ src/coupling/
src/cartridges/` → no output, exit 1), so nothing can forward them. `Spine`'s
outputs have exactly two readers (`rg -n '\.spine\(\)' src/analysis/ src/coupling/
src/cartridges/` → two lines, both in `src/analysis/canvas_1/canvas.hpp`), and
both terminate in published readings — `Reading::Distance` and
`Reading::CurrentPC` — neither of which any pulse identifier names. And
`src/musical/` as a whole contains one line matching `pulse`
(`rg -in 'pulse' src/musical/` → one line, `src/musical/pc_count.hpp:100`), which
is the substring inside the word "impulses" in `pc_onset`'s header comment. There
is therefore no partial ligature anywhere in the onset family: all three
detectors are live, and none of the three has a name that resolves toward
`GPUState::set_pulse_data`.

### 6.9 Provenance of the absence (history, not tree)

Recorded because the unit's chain has a hole and the plan that reads this
section will otherwise have to re-derive where it came from. Nothing here is a
proposal (R4); it is where the missing hop was last present.

**The recipe, with its disambiguating step (R2).** The broad pickaxe search is
`git log --oneline -S 'set_pulse_data' -- src/`, and it returns **fourteen**
commits, not one — the whole history of that identifier's appearance count
changing:

```
$ git log --oneline -S 'set_pulse_data' -- src/
9f0109f8 HANDOFFS FOR WEB
181a52ad Backup_board moved to local backup folder. Cleaned branch.
d2a045cf TERRAIN PROGRAM Phase 0: the charter
3c30f3cc Sibling prune (ruled): the_chord testbed retired, purpose served; backup_board reframed as frozen reference text
c513e9e5 both cartridges: M1 — generation-1 coupling retirement (total sweep)
a1299601 Promote backup_board to a live, selectable second cartridge
5ccb4afc the_chord — clone of the_board with namespace/path rename (no logic changes)
6e0fd5ff getting to it
61f893e1 New docs for new task.
f958cc56 Phase 4.2: extract tick_musical_couplings + reset_musical_couplings
512fd309 C++ and JSX are on the same page
e82abc22 refactoring cartridge generative algorithm overhauling
1d954e78 Getting ready for refactor
d16d157f Initial project commit
$ git log --oneline -S 'set_pulse_data' -- src/ | wc -l
14
```

The commit that removed every data-carrying call is isolated by the deletion of
the file that held them, which is a single-row census:

```
$ git log --diff-filter=D --oneline -- src/cartridges/the_board/modules/musical.inl
c513e9e5 both cartridges: M1 — generation-1 coupling retirement (total sweep)
```

and confirmed by narrowing the pickaxe to that path, which halves the list to two
(the commit that deleted it and the commit that created the block):

```
$ git log --oneline -S 'set_pulse_data' -- src/cartridges/the_board/modules/musical.inl
c513e9e5 both cartridges: M1 — generation-1 coupling retirement (total sweep)
f958cc56 Phase 4.2: extract tick_musical_couplings + reset_musical_couplings
```

That commit is **`c513e9e5`** — *"both cartridges: M1 — generation-1 coupling
retirement (total sweep)"*, `Sun Jul 5 01:21:34 2026 +0000`
(`git show -s --format='%h %ad %s' c513e9e5`). Its message states the intent
verbatim (`git log -1 --format=%B c513e9e5`):

```
modules/musical.inl deleted (both cartridges) with every consumer
thread into it. One coupling layer remains (visual_canvas), one live
coupling (fog), zero hybrids. The GPU capabilities (band motion,
mode uniforms, pulse ring, WGSL machinery) and their set_* wires are
deliberately KEPT as future gen-2 coupling targets per
coupling_layer_migration_map.md.
```

The removed file was `src/cartridges/the_board/modules/musical.inl` (blob at the
parent commit: `513ac5ab55c6166492c5bc22dcb200245f87a7ae`, retrieved with
`git rev-parse c513e9e5^:src/cartridges/the_board/modules/musical.inl`; the path
does not exist at HEAD — `ls src/cartridges/the_board/modules/` reports no such
directory). It held Hop 2 whole. Its constants, its state, and its write block,
verbatim from `git show c513e9e5^:src/cartridges/the_board/modules/musical.inl`:

```cpp
// ── Radial pulse ─────────────────────────────────────────────────
// Pulses are event-driven: an onset (polyphony jump > threshold)
// writes a slot in the ring buffer, the kernel reads decaying
// pulses for terrain displacement.
static constexpr uint32_t PULSE_RING_SIZE        = 8;
static constexpr float    PULSE_AMPLITUDE        = 2.5f;   // world units of peak displacement
static constexpr float    PULSE_MAX_AGE          = 8.0f;   // seconds — must match WGSL
static constexpr float    PULSE_ONSET_THRESHOLD  = 0.5f;   // polyphony rise to count as onset
static constexpr float    PULSE_INCREASE_CLAMP   = 3.0f;   // max polyphony jump scaled into amplitude
```

```cpp
    // ── Radial pulse ─────────────────────────────────────────────
    // Ring buffer: 8 slots × 4 floats per slot = (origin_x, origin_z,
    // onset_seconds, amplitude). Circular write.
    float    pulse_ring[32]   = {};                // 8 × 4 floats
    uint32_t pulse_write_idx  = 0;                 // next slot to write (wraps at 8)
    float    prev_polyphony   = 0.0f;              // previous frame's polyphony (for onset detection)
```

```cpp
    // ─── 4. Radial pulse onset detection (musical:K3 site) ────────
    {
        const bool pulse_on = is_mmode_on(ms, MMODE_RADIAL_PULSE);

        if (pulse_on && polyphony > ms.prev_polyphony + PULSE_ONSET_THRESHOLD) {
            const float increase = polyphony - std::max(ms.prev_polyphony, 0.0f);
            const uint32_t slot = ms.pulse_write_idx % PULSE_RING_SIZE;
            const uint32_t base = slot * 4;
            ms.pulse_ring[base + 0] = c->player_.readback_x;
            ms.pulse_ring[base + 1] = c->player_.readback_z;
            ms.pulse_ring[base + 2] = c->time_state_.seconds;
            ms.pulse_ring[base + 3] = PULSE_AMPLITUDE * std::min(increase, PULSE_INCREASE_CLAMP);
            ms.pulse_write_idx++;
            std::cout << "[Pulse] ONSET slot=" << slot
                << " pos=(" << c->player_.readback_x << "," << c->player_.readback_z << ")"
                << " t=" << c->time_state_.seconds
                << " amp=" << ms.pulse_ring[base + 3]
                << " poly=" << polyphony << " prev=" << ms.prev_polyphony
                << "\n";
        }
        ms.prev_polyphony = polyphony;

        // Count active (non-expired) pulses and upload
        uint32_t active = 0;
        for (uint32_t i = 0; i < PULSE_RING_SIZE; i++) {
            const float onset = ms.pulse_ring[i * 4 + 2];
            const float amp   = ms.pulse_ring[i * 4 + 3];
            if (amp > 0.001f && (c->time_state_.seconds - onset) < PULSE_MAX_AGE) {
                active = std::max(active, i + 1);
            }
        }
        c->gpuState_.set_pulse_data(active, ms.pulse_ring);
    }
```

Four facts follow from comparing that block against HEAD, each of which is an
observed state of the tree, not a plan:

1. The historical onset test was **polyphony rise**, not `pc_onset`. `Reading::Polyphony`
   exists at HEAD as an enum member with a slot spec, is published by nothing,
   and is refused by `Canvas1::writer_wired` (quoted at Hop 1 / Hop 2).
2. The historical ring **origin** was `c->player_.readback_x/.readback_z`. Neither
   identifier exists anywhere in `src/` at HEAD (`rg -n 'readback_x|readback_z' src/`
   → no output, exit 1), and `src/coupling/` has no pawn-position access at all
   (recipe H).
3. The historical **clock** was `c->time_state_.seconds`. That does survive:
   `TimeState::seconds` is declared in
   `src/cartridges/the_board/contracts/spine_state.hpp` and written each frame in
   `cartridge.hpp` as `time_state_.seconds = signal.t_seconds`. The shader side
   uses `signal.t_seconds` directly at the Hop 6 call.
4. The **gate** was `MMODE_RADIAL_PULSE`, a member of the retired MMode registry.
   It does not exist at HEAD (recipe G).

The replacement boot pin introduced by the same commit is the direct ancestor of
what Hop 4b quotes today:

```cpp
+                    float zero_pulses[32] = {};
+                    gpuState_.set_pulse_data(0, zero_pulses);
```

**FLAG — Slice A's status definitions were not readable when this section was
written.** The unit says "definitions as in Slice A", but no `sec5_*.md` existed
in the section directory at the time of writing (`ls` of the lig0 scratchpad
showed `sec0_anchors.md`, `sec1_boundary.md`, `sec2_reach.md`, `sec3_pipes.md`,
`sec4_stats.md` and `blobs_all.txt` only), and a grep of those five for
`PRESENT|STALE|GONE` returned only §3's use of `GONE` alongside `ORPHAN-SINK` /
`ORPHAN-SOURCE`. This section therefore states its own three definitions at §6.0.
Resolving it would have cost one read of the finished Slice A section; if Slice A
draws the PRESENT/STALE line differently — in particular for a hop whose code
executes but whose input is a pinned constant (Hops 3, 4, 5, 6 here) — those four
markers are the ones to re-read against its definition. The underlying facts in
this section do not move: the code is quoted at every hop and the constants that
feed it are named.

**FLAG — some of §6.1's numbers are line counts, not symbol counts.**
Recipes A/A′/F count *lines containing a match*, which for `pulse` conflates the
three unrelated families named at §6.1 and for `onset` conflates note-onset
identifiers with the `onset_beat` fields of `stream_data.hpp`, `playhead.hpp`,
`wagon.hpp` and `spine.hpp`. Every claim in this section rests on the narrow
recipes (B, C, D, E, G, H) and on the scoped onset recipes (K, L, M) added for
Hop 1; A/A′/F are published only as the census that led to the narrow ones. The
`onset` side of the conflation is now decomposed: recipe K enumerates the nine
onset-feeder lines by file and symbol, recipe L decomposes F's 148 across
nineteen files, and recipe M's 18 lines are quoted or accounted for at Hop 1d.
What remains unsplit is the `pulse` side — A's 280 lines are attributed to three
families by name at §6.1 but not counted family by family. Resolving that would
have cost one scoped `rg -c` per family identifier set (the ring's seven names,
the GoL automaton's `GOL_PULSE_TIERS`/`PulseField`/`pulse_cell_target`, and the
transport's `MIDI_CLOCK_PPQN`/`pulses()`); no statement in this section depends
on it, because the ring's own census is recipes B/C/D/E/G, which are exact.

**Section-level record of what the amendment pass changed.** Five citations in
the first draft of this section did not survive re-checking against the tree and
have been corrected in place above, each with the recipe that settles it: (1) the
Hop 5 mirror's enclosing symbol was given as `struct Config`, which does not
exist — it is `struct DesignConfig`; (2) Hop 3's Site 2 was described as
"`GPUState`'s config-defaults initializer" — the enclosing symbol is
`GPUState::initializeState()` and the statements are runtime assignments, not
default member initializers; (3) Hop 6's `// THE REST LAW IS A CONJUNCTION` block
was attributed to `fn write_live_card`'s header comment — it is a WGSL file-scope
prose block titled `// THE CARD WRITER (TRUEBAND_CONTACT_1 T1b, fused at
LATTICE_4)`, separated from that declaration by two functions and the node-table
const section; (4) Hop 2's OPT_1a comment was truncated mid-line and is now
quoted through the end of its paragraph; (5) Hop 6's store block elided four
lines including the binding of `p_here` and is now quoted contiguously. None of
the five changed a status marker: Hops 1–1g remain PRESENT, Hop 2 remains GONE,
and Hops 3, 4, 5 and 6 remain STALE.

## 7. ORGAN duplication verdict

Recon boundary: `79adfa4d26c9e17e0074692928f1d2875d7edde1`, branch
`claude/ligature-0-recon-hcrix0`. **Every recipe in this section is pinned to
that rev explicitly** (`git grep … 79adfa4d`, `git rev-parse 79adfa4d:<path>`),
because the branch tip has since moved — see 7.0.1. Working tree verified clean
before and after this unit (`git status --porcelain` → empty output, both times;
re-verified at amendment time, still empty). No build was run; every claim below
is static.

### 7.0 Anchors (R6) — both blob SHAs verified independently

Recipe: `git rev-parse 79adfa4d:<path>` (and `git rev-parse HEAD:<path>` at the
moved tip — both forms return the same SHAs, see 7.0.1)

| path | blob SHA | lines (`wc -l`) | bytes (`wc -c`) | unit's claim |
| --- | --- | --- | --- | --- |
| `src/console/organ_registry.hpp` | `70d09e9602eb0f763a616da5303e14c34e7f44da` | 974 | 49377 | MATCHES |
| `src/coupling/organ_registry.hpp` | `3047070e199df57c2a7cd6d8f75cf028ec48b817` | 999 | 50109 | MATCHES |
| `src/console/organ_params.inc` | `b426ac4f2b88f89b02f9a9d2236d14b992d93c7f` | 665 | 75599 | (not claimed) |
| `src/cartridges/the_board/organ_boundary.inc` | `d747a67b5931dcbdbd713c00ba47d525e98d5b53` | 136 | 8163 | (not claimed) |

Both SHAs given in the unit are confirmed. Note the unit's "999-line diff" is a
description of the *coupling file's length*, not the diff's: the actual
`git diff --no-index` at default `-U3` is **413 lines**, and at `-U2` is 356.

#### 7.0.1 The branch tip moved after this unit's boundary was set — recorded, not attributed

Recipe: `git rev-parse HEAD` ; `git log -1 --format="%H %an %ad %s" HEAD` ;
`git show --stat --format="" 6d53388e` ; `git status --porcelain`

```
git rev-parse HEAD
→ 6d53388e83f4a5cd7ad3b154484c885f567a02da

git log -1 --format="%H %an %ad %s" 6d53388e
→ 6d53388e83f4a5cd7ad3b154484c885f567a02da Claude Sun Aug 30 19:50:05 2026 +0000 \
  LIGATURE_0 — the recon report: the ligature is one hop, and the socket is empty

git show --stat --format="" 6d53388e
→  docs/LIGATURE_0_RECON.md | 8837 ++++++++++++++++++++++++++++++++++++++++++++++
→  1 file changed, 8837 insertions(+)

git status --porcelain
→ (empty — clean tree, no modified files, no untracked files)
```

`HEAD` is now one commit ahead of this campaign's stated boundary `79adfa4d`.
The single commit adds one file, `docs/LIGATURE_0_RECON.md` — the recon report
this section is a part of. **No source file was touched by it.** All four blob
SHAs in the anchor table above are byte-identical at the moved tip (verified
`git rev-parse HEAD:src/console/organ_registry.hpp` → `70d09e96…`,
`git rev-parse HEAD:src/coupling/organ_registry.hpp` → `3047070e…`), so every
finding in this section stands unchanged.

**The consequence for reproducibility, recorded as fact:** two of this section's
original recipes were written unpinned (`git grep -n '#include.*organ_registry'`
and `grep -rn "coupling/organ_registry" --exclude-dir=.git .`). Now that the
report is itself a tracked file, an unpinned run of those recipes reads the
report's own quotations back as hits. Measured at the moved tip:

| recipe form | at `79adfa4d` (this unit's boundary) | at `6d53388e` (moved tip, unpinned) |
| --- | --- | --- |
| `git grep -n '#include.*organ_registry' <rev>` | **1 hit** (`src/cartridges/the_board/cartridge.hpp`) | **12 hits** — 11 of them inside `docs/LIGATURE_0_RECON.md`, 1 the real `cartridge.hpp` include |
| `git grep -n 'coupling/organ_registry' <rev>` | **no output, exit 1** | **42 hits**, exit 0 — all 42 in `docs/LIGATURE_0_RECON.md` (`grep -rln "coupling/organ_registry" --exclude-dir=.git .` → the single path `./docs/LIGATURE_0_RECON.md`) |

Every hit added by the move is the report quoting itself. No source, tool, gate,
or `CMakeLists.txt` hit was added. All recipes in 7.b below are therefore
published in their **pinned** `79adfa4d` form.

---

### 7.a Diff, header delta, and the substantively differing regions

#### 7.a.1 `--stat`

Recipe: `git diff --no-index --stat src/console/organ_registry.hpp src/coupling/organ_registry.hpp || true`

```
 src/{console => coupling}/organ_registry.hpp | 161 ++++++++++++++++-----------
 1 file changed, 93 insertions(+), 68 deletions(-)
```

Git itself treats the pair as a rename candidate (`src/{console => coupling}/`),
i.e. the similarity index is high enough that `--no-index` renders it as one path
moving rather than two unrelated files.

#### 7.a.2 The section-header delta

Recipe (the file's own section-banner idiom is `// ─── TITLE ───…`):

```sh
grep -n '^// ─── ' src/console/organ_registry.hpp
grep -n '^// ─── ' src/coupling/organ_registry.hpp
```

Recipe for the heavy `═══` banners:

```sh
grep -n '^// ═══ [A-Z]' <file>
grep -n '^// ORGAN — '  <file>
```

Both files carry **16** `─── ` section headings and **1** `═══ THE C ABI ═══`
banner and **1** top-of-file `// ORGAN — …` title line. Counts are equal; only
two headings differ in text.

| heading present ONLY in `src/console/organ_registry.hpp` | heading present ONLY in `src/coupling/organ_registry.hpp` |
| --- | --- |
| `// ORGAN — THE COMPILED REGISTRY AND THE PANEL'S C ABI` (file title) | `// ORGAN — THE COMPILED REGISTRY AND ITS C ABI` (file title) |
| `// ─── THE SHELL'S TWO QUESTIONS, DERIVED HERE ───` | `// ─── THE CONTROL SURFACE'S TWO QUESTIONS, DERIVED HERE ───` |

Headings COMMON to both (identical text, 15 of the 16 `───` headings plus the C
ABI banner):

| heading common to both files |
| --- |
| `// ─── Type tags ───` |
| `// ─── Block ids ───` |
| `// ─── Definition targets ───` |
| `// ─── Cadence ───` |
| `// ─── The entry ───` |
| `// ─── The enrollment macro ───` |
| `// ─── THE NAMESPACE PARAMETER, MADE INVISIBLE ───` |
| `// ─── The live home ───` |
| `// ─── THE DEFINITION WRITE PATH ───` |
| `// ─── THE TOUCHED MASK ───` |
| `// ─── THE CONSOLE MASK ───` |
| `// ─── DOORS ───` |
| `// ─── THE MOOD DOOR ───` |
| `// ─── THE HOST DOOR (RIBBON_1) ───` |
| `// ─── THE RULE WINDOW ───` |
| `// ═══ THE C ABI ═══` |

**No heading exists in one file and is absent from the other.** The section
skeleton is a bijection; the one asymmetric pair above is a RENAME of the same
section, not an addition or a removal.

#### 7.a.3 The differing regions, by symbol — comment-strip census

To separate prose drift from code drift I stripped C++ comments (string and char
literals preserved) with a scratchpad-only Python script and diffed the
remaining code lines.

Recipe (script at
`/tmp/claude-0/-home-user-7T-Music/e4d04ccd-accf-5afe-8420-1b448d1fd519/scratchpad/strip.py`;
tokenizer walks the file, passes `"…"` and `'…'` through verbatim, drops `//…EOL`
and `/*…*/`, then drops blank lines and diffs with `difflib.unified_diff`):

```
python3 .../strip.py src/console/organ_registry.hpp src/coupling/organ_registry.hpp
→ code lines A: 538   code lines B: 538
→ CODE DIFF LINES: 9
```

**538 code lines each. Exactly ONE code-level difference, and it is inside a
string literal.** Every other one of the 161 changed lines is a comment.

The single differing code region, by enclosing symbol — the `static_assert`
immediately following `t7::organ::kOrganDoors`:

`src/console/organ_registry.hpp` :: `t7::organ` :: `static_assert` on `kOrganDoors`

```cpp
static_assert(sizeof(kOrganDoors) / sizeof(kOrganDoors[0]) == ORGAN_DOOR_COUNT,
    "one row per door id — the manifest emits this table and the shell "
    "renders one button per row, so a missing row is a missing button");
```

`src/coupling/organ_registry.hpp` :: `t7::organ` :: `static_assert` on `kOrganDoors`

```cpp
static_assert(sizeof(kOrganDoors) / sizeof(kOrganDoors[0]) == ORGAN_DOOR_COUNT,
    "one row per door id — the manifest emits this table and a consumer "
    "renders one control per row, so a missing row is a missing door");
```

The asserted *predicate* is byte-identical; only the diagnostic message differs.

The substantively differing *prose* regions, by enclosing symbol:

| enclosing symbol | nature of the difference |
| --- | --- |
| file preamble (before `namespace t7`) | coupling adds a 6-line paragraph naming "the native control surface to come (docs/OPEN.md, THE ABLETON SEAM)" and stating "the browser panel that drove it is attic'd at tag web-sunset"; console has no such paragraph |
| `#include "core/instruments.hpp"` trailing comment | console: `the tree the panel names` / coupling: `the tree organ_build_stamp names` |
| the `// ─── Type tags ───` preamble comment in `t7::organ`, i.e. the comment block introducing the anonymous `enum : uint8_t { ORGAN_F32 … ORGAN_VEC4 }` | `the panel happens to render` → `a consumer happens to render` — **AMENDED**: this row previously named `t7::organ::lanes_of` as the enclosing symbol. Re-checked: `grep -n "happens to render" src/console/organ_registry.hpp src/coupling/organ_registry.hpp` → `console:40`, `coupling:46`; reading the surrounding region (`sed -n '36,58p' src/console/organ_registry.hpp`) shows the sentence is the closing line of the `// ─── Type tags ───` comment block, which is immediately followed by `enum : uint8_t {`. `inline int lanes_of(uint8_t type)` begins **after** that enum closes and carries **no preamble comment of its own**. The prose delta itself (`panel` → `consumer`) is confirmed verbatim; only the enclosing symbol was wrong. |
| block-id `enum` preamble (the `// ─── Block ids ───` comment block) | `GPUState hands the panel` → `GPUState hands the control surface` (verified verbatim: console `// One per CPU home GPUState hands the panel; the number is the bit` / coupling `// One per CPU home GPUState hands the control surface; the number is the bit`) |
| `t7::organ::OrganParam` / `ORGAN_PARAM_DEFONLY_NS` preamble | `a definition the panel may write` → `a definition the control surface may write` |
| `ORGAN_PARAM_RO_NS` preamble | `the panel meters it` → `the control surface meters it` |
| `t7::organ::g_home`, `g_rejected` | console: `the panel's JS may be present on a page whose program has not finished booting` and `shown in the panel`; coupling: `a consumer may be attached before the program has finished booting` and `reported by name` |
| `t7::organ::g_mood`, `g_point` | `the panel` → `the control surface` |
| `derived_has_instance` / `derived_def_kind` preambles | `THE SHELL MUST NOT KNOW A BLOCK NUMBER` → `THE CONTROL SURFACE MUST NOT KNOW A BLOCK NUMBER` |
| `organ_manifest`, `organ_doors`, `organ_set`, `organ_get`, `organ_rejected_count`, `organ_last_reject`, `organ_flush_count`, `organ_mood`, `organ_regime`, `organ_orb_rule`, `organ_door`, `organ_go_host`, `organ_mood_names`, `organ_def_get` preambles | `panel` / `shell` → `control surface` / `consumer` throughout |
| `organ_param_count` preamble (the PURSE_0 R-D block) | console names the concrete web caller (`organ_panel.js polls …`, `` `?preset=<name>` walks the same road at boot ``, `The panel's first poll is at 500.`); coupling replaces those with de-webbed prose (`A consumer polls this…`, `a preset walking the same road at boot would have…`, `The control surface that paid for this finding polled first at 500.`) |
| `organ_build_stamp` preamble (the PURSE_0 R2 block) | console names `window.T7_BUILD_ID`, `web_dist`, `no JS edit`, `a cwrap and a line of JS`, `The shell gate is the witness`; coupling replaces all of it with `The artifact digest it used to sit beside went with the web twin at tag web-sunset` + the ABLETON SEAM paragraph |
| `EMSCRIPTEN_KEEPALIVE` fallback `#define` preamble | console: `so ccall/cwrap can reach it by name`; coupling: `so a consumer can reach it BY NAME` + `which is why this ABI cost the sunset nothing and is exactly what survives it` |

Prose-vintage census (recipe: `grep -oic '<term>' <file>` — `-c` counts *lines
containing*, not occurrences):

| file | lines w/ "control surface" | lines w/ `\bpanel\b` | lines w/ `\bshell\b` | lines w/ `\bconsumer` |
| --- | --- | --- | --- | --- |
| `src/console/organ_registry.hpp` | 0 | 33 | 14 | 0 |
| `src/coupling/organ_registry.hpp` | 28 | 2 | 0 | 16 |
| `src/console/organ_params.inc` | 4 | 10 | 0 | 1 |
| `src/cartridges/the_board/organ_boundary.inc` | 0 | 1 | 0 | 0 |

The 2 residual `panel` lines in the coupling copy are both the same sentence
(`// ABLETON SEAM); the browser panel that drove it is attic'd at tag`), once in
the file preamble and once in `organ_build_stamp`'s preamble.

#### 7.a.4 PROVENANCE — how the pair came to exist (blob-identity evidence)

Recipes: `git log --oneline --follow -- <path>` ; `git rev-parse <rev>:<path>` ;
`git show --stat <rev>`

| rev | date | subject | `src/console/organ_registry.hpp` blob | `src/coupling/organ_registry.hpp` blob |
| --- | --- | --- | --- | --- |
| `5794624b` | — | PURSE_0 R-D — the count becomes the readiness gate it claimed to be | `70d09e96` | (absent) |
| `4cfc899b` | — | WEB_SUNSET W3e: the organ keeps its ABI and loses its browser — prose only | `3047070e` | (absent) |
| `72df32df` | 2026-08-30 14:29:22 -0300 | **Systems operating** | `3047070e` | `3047070e` |
| `79adfa4d` (unit boundary) | 2026-08-30 15:33:58 -0300 | **Systems operational** | `70d09e96` | `3047070e` |
| `6d53388e` (moved tip) | 2026-08-30 19:50:05 +0000 | LIGATURE_0 — the recon report … | `70d09e96` | `3047070e` |

Reading, stated as fact only:

1. Blob `3047070e` — the blob now sitting at `src/coupling/organ_registry.hpp` —
   is the blob `4cfc899b` ("WEB_SUNSET W3e … prose only") wrote to
   `src/console/organ_registry.hpp`.
2. `72df32df` is `1 file changed, 999 insertions(+)`, all of them
   `src/coupling/organ_registry.hpp`. It did **not** delete or modify
   `src/console/organ_registry.hpp`; git recorded no rename. At that commit both
   paths pointed at the *same blob* `3047070e`.
3. `79adfa4d` rewrote `src/console/organ_registry.hpp` to blob
   `70d09e96`, which is byte-identical to that path's **pre-W3e** blob at
   `5794624b` (verified: `git diff --stat 5794624b:src/console/organ_registry.hpp
   79adfa4d:src/console/organ_registry.hpp` prints nothing). The W3e prose change
   to the console path is therefore not present at the unit boundary, nor at the
   moved tip.
4. The same `79adfa4d` commit's two other hunks (`git show 79adfa4d -- CMakeLists.txt
   src/console/console.hpp`) are: `CMakeLists.txt` losing its trailing newline
   (`\ No newline at end of file`), and `src/console/console.hpp` ::
   `t7::kCompilerPlan` gaining a stray blank line and a line-broken `;`:

```cpp
-    inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan;
+    inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan
+        
+        ;
```

   Recorded as observed; no cause is asserted here.
5. `6d53388e` (the moved tip) touched neither registry path; see 7.0.1.

---

### 7.b The include graph

**All recipes in 7.b are pinned to the unit boundary `79adfa4d`.** See 7.0.1 for
why the unpinned forms no longer reproduce.

#### 7.b.1 The census

Recipe: `git grep -n "organ_registry.hpp" 79adfa4d -- src/`

```
79adfa4d:src/cartridges/the_board/cartridge.hpp:67:#include "console/organ_registry.hpp"   // the compiled dial registry + its C ABI (needs the home types above)
79adfa4d:src/cartridges/the_board/organ_boundary.inc:4:// organ_registry.hpp owns the flags and masks consumed here and knows neither
79adfa4d:src/console/organ_params.inc:4:// organ_registry.hpp compiles these lines into kOrganParams[] and the
79adfa4d:src/console/organ_params.inc:34:// BLOCK is an ORGAN_BLOCK_* (organ_registry.hpp) and there is no way to
```

(The same four hits are what the unit's literal recipe
`grep -rn "organ_registry.hpp" src/` returns when run against the working tree,
since no source file differs between `79adfa4d` and the moved tip.)

Three of the four hits are prose inside comments. Recipe narrowing to real
preprocessor directives, run repo-wide rather than `src/`-only, **pinned**:

```
git grep -n '#include.*organ_registry' 79adfa4d
→ 79adfa4d:src/cartridges/the_board/cartridge.hpp:67:#include "console/organ_registry.hpp"   // the compiled dial registry + its C ABI (needs the home types above)
   (exit 0, exactly one hit)
```

**Exactly one `#include` of an `organ_registry.hpp` exists in the entire tracked
tree at the unit boundary.** A second, untracked-inclusive sweep found no
reference at all to the coupling path:

```
git grep -n 'coupling/organ_registry' 79adfa4d
→ (no output, exit 1)

grep -rn "coupling/organ_registry" --exclude-dir=.git .   # run while the tree stood at 79adfa4d
→ (no output, exit 1)
```

**AMENDED / R2 —** both of those recipes were originally published in unpinned
form. Re-run unpinned at the moved tip they now return 12 and 42 hits
respectively; every added hit is inside `docs/LIGATURE_0_RECON.md`, the recon
report itself (`grep -rln "coupling/organ_registry" --exclude-dir=.git .` →
the single path `./docs/LIGATURE_0_RECON.md`). Pinned to `79adfa4d` both
censuses reproduce **exactly as published**: one hit and zero hits. The finding
is unchanged; only the recipe needed the explicit rev argument.

#### 7.b.2 The include directories actually configured

Recipe: `grep -n "target_include_directories\|include_directories" /home/user/7T-Music/CMakeLists.txt`,
then quoted from `CMakeLists.txt` (three rows, all `PRIVATE` on target `the_board`):

```cmake
add_executable(the_board
    src/the_board.cpp
    # The one other translation unit: RtMidi's Windows MM backend, the
    # canvas's route to the DAW's virtual port. Vendored, not header-only,
    # so it compiles separately; __WINDOWS_MM__ below selects its backend.
    src/external/RtMidi.cpp
    ${T7_RENDER_HEADERS}
)

target_include_directories(the_board PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
```

```cmake
target_include_directories(the_board PRIVATE ${DAWN_INCLUDES})
```

```cmake
target_include_directories(the_board PRIVATE "${T7_STAMP_DIR}")
```

`${T7_RENDER_HEADERS}` is a headers-only glob, quoted:

```cmake
# Only the active cartridge's headers → scoped IntelliSense
file(GLOB_RECURSE T7_RENDER_HEADERS
    "src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp"
)
```

with `set(T7_RENDER_CARTRIDGE "the_board" …)` — so the glob covers
`src/cartridges/the_board/**/*.hpp` only. It reaches neither `src/console/` nor
`src/coupling/`, and in any case adds header files to the target for
IDE/source-group purposes, not compiled TUs.

**The only `-I` root that can resolve a rooted `"console/…"` or `"coupling/…"`
spelling is `${CMAKE_SOURCE_DIR}/src`.** `${DAWN_INCLUDES}` is Dawn's tree and
`${T7_STAMP_DIR}` is the generated build-stamp directory; neither contains a
`console/` or `coupling/` subdirectory.

#### 7.b.3 The resolution table

| includer file :: enclosing context | include line verbatim | resolves to |
| --- | --- | --- |
| `src/cartridges/the_board/cartridge.hpp` :: **file scope**, in the top-of-file include block, *before* `namespace t7 {` opens (verified: `grep -n "^namespace" src/cartridges/the_board/cartridge.hpp` → first hit is `namespace t7 {` well after the include block; the include sits at column 0 among the other rooted includes) | `#include "console/organ_registry.hpp"   // the compiled dial registry + its C ABI (needs the home types above)` | **`src/console/organ_registry.hpp`** (blob `70d09e96`) |

Resolution reasoning, stated explicitly:

* The form is **quoted**, not angled, and the path is **rooted** (`console/…`),
  not relative-with-`../`.
* Quoted-form lookup tries the includer's own directory first:
  `src/cartridges/the_board/console/organ_registry.hpp` — this path does not
  exist (`ls src/cartridges/the_board/` has no `console/`), so that candidate
  falls through.
* Lookup then walks the `-I` list. `${CMAKE_SOURCE_DIR}/src` is the only entry
  that can complete `console/organ_registry.hpp`, and it completes to
  `src/console/organ_registry.hpp`, which exists.
* There is **no** `-I` entry rooted at `src/coupling/` and no `-I` entry under
  which the spelling `console/organ_registry.hpp` could name
  `src/coupling/organ_registry.hpp`. The resolution is unambiguous. NOT AMBIGUOUS.

Two further inclusion sites for completeness (both registries name the same
enrollment list, so the .inc is pulled from `src/console/` either way):

| includer file :: enclosing context | include line verbatim | resolves to |
| --- | --- | --- |
| `src/console/organ_registry.hpp` :: `namespace t7 { namespace organ {`, immediately after the five `ORGAN_PARAM*` forwarding macros | `#include "console/organ_params.inc"` | `src/console/organ_params.inc` (relative candidate `src/console/console/organ_params.inc` does not exist; `-I src` completes) |
| `src/coupling/organ_registry.hpp` :: `namespace t7 { namespace organ {`, same position | `#include "console/organ_params.inc"` | `src/console/organ_params.inc` (relative candidate `src/coupling/console/organ_params.inc` does not exist; `-I src` completes) — i.e. the coupling copy points **back into `src/console/`** for its rows |
| `src/cartridges/the_board/cartridge.hpp` :: inside `class Cartridge : public RenderCartridge` (declared at `class Cartridge`, itself inside `namespace t7 {`), among the member functions, immediately after `phase_clear_input_deltas` | `#include "cartridges/the_board/organ_boundary.inc"` | `src/cartridges/the_board/organ_boundary.inc` |

#### 7.b.4 Reachability from the compiled translation units

The target's real TUs are `src/the_board.cpp` and `src/external/RtMidi.cpp`.
`src/the_board.cpp`'s includes (recipe: `grep -n "#include" src/the_board.cpp`)
name `"console/console.hpp"`, `"analysis/beat_clock.hpp"`,
`"cartridges/the_board/cartridge.hpp"`, `RENDER_HEADER(INCUBATE_RENDER)`,
`"core/instruments.hpp"`, `"core/boot_params.hpp"`. Only `cartridge.hpp` reaches
an organ registry, and it reaches the `console/` one.

Gate coverage (recipe: `grep -rn "organ_registry\|cartridge.hpp\|console.hpp"
tools/gates/console_gate/run.py`): the TU gate's two tiers are
`#include "cartridges/the_board/cartridge.hpp"` (tier CARTRIDGE) and
`#include "console/console.hpp"` + `the_board.cpp` (tier CONSOLE). `glaw1`'s TU
is the same `cartridge.hpp`. Neither reaches `src/coupling/organ_registry.hpp`.

**Tool coverage — census A.** Recipe:
`git grep -n "organ_registry" 79adfa4d -- . | grep -v ':src/'`

| file | what it names | which registry |
| --- | --- | --- |
| `CMakeLists.txt` (the `/Zc:preprocessor` block) | `# src/console/organ_registry.hpp forwards five variadic macros:` | console, by explicit path |
| `tools/organ_ledger.py` | `REG = os.path.join(ROOT, "src", "console", "organ_registry.hpp")` | console, by explicit path |
| `audit/ORGAN.md` (generated) | `and src/console/organ_registry.hpp — do not hand-edit.` | console, by explicit path |
| `docs/ORGAN.md` | ``` `src/console/organ_params.inc` is the enrollment list. `src/console/organ_registry.hpp` ``` | console, by explicit path |
| `tools/organ_readers.py` | `if rel.endswith("organ_registry.hpp") or rel.endswith("organ_params.inc"): continue` | **path-suffix match — EXCLUDES BOTH copies from the reader scan.** The coupling copy is skipped by this tool as a registry, not scanned as a reader. |
| `full_list.txt` | `C:\dev\7t\src\console\organ_registry.hpp` | console, by explicit path (Windows-form inventory) |

**No row of census A names `src/coupling/organ_registry.hpp`.**

**Tool coverage — census B (directory-level text scraping, a SEPARATE census with
its own recipe).** The `src/coupling/` *directory* is walked as text by the
glaw1 stub generator. This does not appear in census A and cannot: the generator
never contains the string `organ` at all.

Recipe: `grep -c "organ" tools/gates/glaw1/gen_stubs.py` and
`grep -n "DIRS\|harvest(" tools/gates/glaw1/gen_stubs.py`

```
grep -c "organ" tools/gates/glaw1/gen_stubs.py
→ 0

grep -n "DIRS\|harvest(" tools/gates/glaw1/gen_stubs.py
→ 17:def harvest(pattern, dirs, exts=("hpp", "inl", "cpp", "h")):
→ 34:DIRS = ["cartridges/the_board", "render", "core", "coupling", "analysis", "musical"]
→ 35:types = harvest(r"wgpu::(\w+)", DIRS)
→ 36:pairs = harvest(r"wgpu::(\w+)::(\w+)", DIRS)
→ 158:macros = sorted(harvest(r"\b(GLFW_\w+)\b", ["cartridges/the_board", "render", "core", "coupling"]))
→ 159:fns = sorted(harvest(r"\b(glfw\w+)\s*\(", ["cartridges/the_board", "render", "core", "coupling"]))
```

| symbol in `tools/gates/glaw1/gen_stubs.py` | directory list | its consumers |
| --- | --- | --- |
| `DIRS` (module scope) | `["cartridges/the_board", "render", "core", "coupling", "analysis", "musical"]` — **6 entries**, includes `analysis` and `musical` | `types = harvest(r"wgpu::(\w+)", DIRS)` and `pairs = harvest(r"wgpu::(\w+)::(\w+)", DIRS)` — the **`wgpu::` harvests only** |
| inline literal at `macros = …` | `["cartridges/the_board", "render", "core", "coupling"]` — **4 entries**, omits `analysis` and `musical` | `harvest(r"\b(GLFW_\w+)\b", …)` |
| inline literal at `fns = …` | `["cartridges/the_board", "render", "core", "coupling"]` — **4 entries**, omits `analysis` and `musical` | `harvest(r"\b(glfw\w+)\s*\(", …)` |

**AMENDED / R2 + attribution —** this row was previously published under census
A's recipe (`git grep "organ_registry" … | grep -v src/`), which cannot produce
it, and it attributed `DIRS` to the two GLFW harvests. Both corrected above and
verified by the `grep -n "DIRS\|harvest("` output quoted verbatim. The row's
underlying fact is unchanged and stands: **`src/coupling/` is walked as a
directory of text by both the `wgpu::` harvests (via `DIRS`) and the GLFW
harvests (via the inline 4-entry list); neither harvest names `organ_registry`,
and neither compiles anything.** What is harvested are `wgpu::`, `GLFW_*` and
`glfw*()` symbols for stub generation. No organ symbol is harvested.

**Net across both censuses: no gate, ledger tool, generator, doc, or
`CMakeLists.txt` row names `src/coupling/organ_registry.hpp` by path.**

---

### 7.c Which registry are the two `.inc` files written against?

#### 7.c.1 `src/console/organ_params.inc` (blob `b426ac4f2b88f89b02f9a9d2236d14b992d93c7f`)

The deciding symbols are the five enrollment macro forms plus the `_NS` twin and
the `ORGAN_BLOCK_*` vocabulary. Census recipe:

```sh
grep -oE "^ *ORGAN_PARAM[A-Z_]*" src/console/organ_params.inc | tr -d ' ' | sort | uniq -c | sort -rn
```

| invocation form | rows in `organ_params.inc` |
| --- | --- |
| `ORGAN_PARAM` | 120 |
| `ORGAN_PARAM_DEF` | 110 |
| `ORGAN_PARAM_DEFONLY` | 74 |
| `ORGAN_PARAM_GEN` | 42 |
| `ORGAN_PARAM_RO` | 20 |
| `ORGAN_PARAM_NS` | 15 |
| **total** | **381** (the five plain forms alone = **366**, matching `CMakeLists.txt`'s `# and 366 rows of organ_params.inc ride them.`) |

The deciding macro, quoted **verbatim and unelided** from the `.inc`'s own
contract block. Recipe: `sed -n '15,38p' src/console/organ_params.inc` (the
`─── ADDING A DIAL IS ONE LINE IN THIS FILE ───` block, from its banner through
the end of the TYPE sentence; the block continues past this excerpt with
ELIGIBILITY / DEFKIND prose that is not deciding here):

```
// ─── ADDING A DIAL IS ONE LINE IN THIS FILE ────────────────────────────
// Nothing else, anywhere. Five forms, and EVERY FORM HAS AN _NS TWIN
// taking the enrolled struct's NAMESPACE first — the plain names forward
// `the_board`, so a line that does not care never sees the parameter:
//
//   ORGAN_PARAM(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL)
//   ORGAN_PARAM_GEN(… the same …)      a GENERATIONAL dial: the edit lands
//                                      at the author's next natural event,
//                                      and the row wears `on respawn`
//   ORGAN_PARAM_DEF(… , DEFKIND, DEFSTRUCT, DEFFIELD)
//                                      the same, plus the DEFINITION it writes
//   ORGAN_PARAM_DEFONLY(TYPE, MIN, MAX, STEP, GROUP, LABEL,
//                       DEFKIND, DEFSTRUCT, DEFFIELD)
//                                      a definition with NO INSTANCE:
//                                      preview on it is refused
//   ORGAN_PARAM_RO(BLOCK, STRUCT, FIELD, TYPE, GROUP, LABEL)
//                                      a WITNESS: metered, never written,
//                                      and carrying no range
//
// BLOCK is an ORGAN_BLOCK_* (organ_registry.hpp) and there is no way to
// name anything else, which is the sovereignty boundary. STRUCT is the
// home type the offset is taken in; FIELD is the member as offsetof takes
// it, so `tier_gains[2].color_r` is legal. TYPE is F32 / U32 / BOOL /
// VEC3 / VEC4, and a VEC3 over 0…1 renders as a colour.
```

**AMENDED / R5 —** the earlier printing of this block silently dropped seven
continuation comment lines (the `at the author's next natural event,` /
`and the row wears \`on respawn\`` pair after `ORGAN_PARAM_GEN`; the
`the same, plus the DEFINITION it writes` line after `ORGAN_PARAM_DEF`; the
`a definition with NO INSTANCE:` / `preview on it is refused` pair after
`ORGAN_PARAM_DEFONLY`; and the `a WITNESS: metered, never written,` /
`and carrying no range` pair after `ORGAN_PARAM_RO`), and truncated its last
line mid-sentence at `… sovereignty boundary.` where the file reads
`… sovereignty boundary. STRUCT is the`. Restored above in full. The deciding
content identified earlier — five `ORGAN_PARAM` forms, `BLOCK` constrained to
`ORGAN_BLOCK_*` — was correct; only the quote fidelity failed.

Their declarations. Recipe: `grep -n "^#define ORGAN_PARAM" <registry>`

`src/console/organ_registry.hpp` :: `namespace t7::organ`:
```cpp
#define ORGAN_PARAM(...)         ORGAN_PARAM_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_GEN(...)     ORGAN_PARAM_GEN_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_DEF(...)     ORGAN_PARAM_DEF_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_DEFONLY(...) ORGAN_PARAM_DEFONLY_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_RO(...)      ORGAN_PARAM_RO_NS(the_board, __VA_ARGS__)
```

`src/coupling/organ_registry.hpp` :: `namespace t7::organ`:
```cpp
#define ORGAN_PARAM(...)         ORGAN_PARAM_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_GEN(...)     ORGAN_PARAM_GEN_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_DEF(...)     ORGAN_PARAM_DEF_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_DEFONLY(...) ORGAN_PARAM_DEFONLY_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_RO(...)      ORGAN_PARAM_RO_NS(the_board, __VA_ARGS__)
```

**Byte-identical.** The five `_NS` primaries are likewise present in both with
identical parameter lists (`ORGAN_PARAM_NS(NS, BLOCK, STRUCT, FIELD, TYPE, MIN,
MAX, STEP, GROUP, LABEL)` etc.).

The `ORGAN_BLOCK_*` enum is identical in both. **Count recipe:**

```sh
grep -c "ORGAN_BLOCK_[A-Z_]* *=" src/console/organ_registry.hpp    → 15
grep -c "ORGAN_BLOCK_[A-Z_]* *=" src/coupling/organ_registry.hpp   → 15
```

**Identity recipe** (name+value pairs, order preserved, comments and whitespace
excluded):

```sh
diff <(grep -o "ORGAN_BLOCK_[A-Z_]* *= *[0-9]*" src/console/organ_registry.hpp) \
     <(grep -o "ORGAN_BLOCK_[A-Z_]* *= *[0-9]*" src/coupling/organ_registry.hpp)
→ (no output, exit 0 — identical)
```

**15 identical rows each**, enumerated: `ORGAN_BLOCK_CONFIG = 0`,
`ORGAN_BLOCK_LIGHTING = 1`, `ORGAN_BLOCK_AGENT_ROOM = 2`,
`ORGAN_BLOCK_DRIVERS = 3`, `ORGAN_BLOCK_PAWN = 4`, `ORGAN_BLOCK_ORBS = 5`,
`ORGAN_BLOCK_PANEL = 6`, `ORGAN_BLOCK_RIBBON = 7`, `ORGAN_BLOCK_INDOOR = 8`,
`ORGAN_BLOCK_CANVAS = 9`, `ORGAN_BLOCK_WORLD = 10`,
`ORGAN_BLOCK_RIBBON_SPAWN = 11` (**12 real block ids**), plus
`ORGAN_BLOCK_COUNT = 12`, `ORGAN_BLOCK_NONE = 255`,
`ORGAN_BLOCK_NONE_ORB = 254` (**3 sentinel/terminator rows**) = **15**.

**AMENDED / R2 —** the earlier text said "13 identical rows each" while
enumerating 15 in the same sentence; the published recipe yields 15 in each
file. The count is corrected to 15 and split into 12 block ids + 3 sentinels so
the arithmetic is legible. The identity claim was and remains correct.

For completeness, the door vocabulary is identical too. Recipe:

```sh
grep -c "ORGAN_DOOR_[A-Z_]* *=" src/console/organ_registry.hpp    → 4
grep -c "ORGAN_DOOR_[A-Z_]* *=" src/coupling/organ_registry.hpp   → 4
diff <(grep -o "ORGAN_DOOR_[A-Z_]* *= *[0-9]*" src/console/organ_registry.hpp) \
     <(grep -o "ORGAN_DOOR_[A-Z_]* *= *[0-9]*" src/coupling/organ_registry.hpp)
→ (no output, exit 0 — identical)
```

**FINDING: the symbols do not decide.** Every macro and every block id
`organ_params.inc` uses is declared identically in both registries. What *does*
decide is the inclusion direction: `organ_params.inc` is not a consumer that
picks a registry — it is the **payload**, `#include`d by whichever registry the
TU pulled in, and both registries name it with the identical rooted spelling
`#include "console/organ_params.inc"`. It is written against **both, indistinguishably**.

Prose vintage of the `.inc`, recorded as a separate fact: it carries the
**post-W3e** wording ("This file's order IS the control surface's table of
contents,", "a consumer splits on the FIRST separator", "because the control
surface opens a / new block"), complete with W3e's reflow artifacts — visible in
the verbatim excerpt above as the short orphan line `// contents,` following
`// enrollment line. This file's order IS the control surface's table of` —
while the registry that actually includes it at the unit boundary
(`src/console/organ_registry.hpp`, blob `70d09e96`) carries the **pre-W3e**
wording (`panel`/`shell`, 0 lines containing "control surface"). The two files
that compile together disagree in vocabulary; the two files that never compile
together (`organ_params.inc` and `src/coupling/organ_registry.hpp`) agree.

#### 7.c.2 `src/cartridges/the_board/organ_boundary.inc` (blob `d747a67b5931dcbdbd713c00ba47d525e98d5b53`)

The deciding symbols are the `t7::organ::` flags, masks and take-functions the
member function `Cartridge::organ_flush(wgpu::Queue&)` consumes. Quoted verbatim
from the `.inc`'s own preamble (recipe: `sed -n '1,9p' src/cartridges/the_board/organ_boundary.inc`):

```
// ORGAN — THE FRAME BOUNDARY. Included inside class Cartridge (cartridge.hpp):
// these are member functions and reach the cartridge's deps and queue directly.
// pawn.cpp's frame() calls them once per frame, before input and update.
// organ_registry.hpp owns the flags and masks consumed here and knows neither
// the deps nor the queue, which is why the consumption lives at this tier.

            // ORGAN — the frame boundary: doors, definition re-speaks and
            // the flush, taken once per frame for the LIVE mood only.
            void organ_flush(wgpu::Queue& queue) {
```

The deciding call, quoted **verbatim and unelided** — the first `t7::organ::`
reach inside `src/cartridges/the_board/organ_boundary.inc` ::
`Cartridge::organ_flush`. Recipe:
`grep -n -B4 -A12 "take_doors_pending" src/cartridges/the_board/organ_boundary.inc`

```cpp
                // DOOR 0, Re-speak definitions: it raises the flags the
                // lines below already consume, so the boundary then does
                // what it does every frame. A definition write is durable
                // but not immediate, and this is that round trip.
                const uint32_t doors = t7::organ::take_doors_pending();
                if (doors & (1u << t7::organ::ORGAN_DOOR_RESPEAK)) {
                    t7::organ::g_def_dirty      = true;
                    t7::organ::g_def_dirty_mood = mood_state_.active;
                    t7::organ::g_tier_def_dirty = true;
                    // The door promises the LIVE definitions, so it names
                    // the live mood.
                    t7::organ::raise_orb_definition(mood_state_.active);
                }
```

**AMENDED / R5 —** the earlier printing of this block silently dropped the two
comment lines `// The door promises the LIVE definitions, so it names` and
`// the live mood.` that sit between `g_tier_def_dirty = true;` and the
`raise_orb_definition` call, with no ellipsis and no note. Restored above, along
with the four-line `// DOOR 0, Re-speak definitions:` comment that introduces
the region, so the excerpt is now a contiguous verbatim run. The deciding
symbols identified earlier were correct; only the quote fidelity failed.

Its declaration — identical in both registries, quoted from
`src/console/organ_registry.hpp` :: `namespace t7::organ` (section `─── DOORS ───`)
and present verbatim in `src/coupling/organ_registry.hpp` :: `namespace t7::organ`:

```cpp
inline uint32_t take_doors_pending() {
```

Full symbol census across both registries. Recipe:

```sh
for s in take_doors_pending ORGAN_DOOR_RESPEAK … ; do
  grep -c "\b$s\b" src/console/organ_registry.hpp
  grep -c "\b$s\b" src/coupling/organ_registry.hpp
done
```

| symbol used by `organ_boundary.inc` | occurrences in `console/` | occurrences in `coupling/` |
| --- | --- | --- |
| `take_doors_pending` | 1 | 1 |
| `ORGAN_DOOR_RESPEAK` | 2 | 2 |
| `ORGAN_DOOR_ORB_RULE` | 2 | 2 |
| `ORGAN_DOOR_ORB_GESTURE` | 2 | 2 |
| `g_def_dirty` | 4 | 4 |
| `g_def_dirty_mood` | 3 | 3 |
| `g_tier_def_dirty` | 4 | 4 |
| `raise_orb_definition` | 2 | 2 |
| `take_go_mood` | 1 | 1 |
| `take_go_host` | 1 | 1 |
| `take_definition_dirty` | 1 | 1 |
| `take_tier_definition_dirty` | 1 | 1 |
| `take_orb_console_dirty` | 1 | 1 |
| `take_orb_definition_dirty` | 1 | 1 |
| `take_orb_def_touched` | 1 | 1 |
| `ORB_RESEED_BITS` | 2 | 2 |
| `set_orb_rule_view` | 2 | 2 |

**All 17 declared identically in both — that is itself the finding.**

What *does* decide for this `.inc` is textual position, not symbol vocabulary:
`organ_boundary.inc` is `#include`d inside `class Cartridge` in
`src/cartridges/the_board/cartridge.hpp`, at a point where the only
`organ_registry.hpp` in the TU is the one that same file pulled in at file scope
— `"console/organ_registry.hpp"`. So at the unit boundary it compiles against
**`src/console/organ_registry.hpp`**, and it could compile against the coupling
copy only if a different registry were the one in the TU.

Prose vintage: `organ_boundary.inc` carries the **pre-W3e** wording — one line
containing `panel` (`// sky's rule and gesture are player-owned and live in /
// OrbsState; the panel READS them.`), zero lines containing "control surface".
That matches the registry it actually compiles against.

#### 7.c.3 A structural fact about the pair, recorded not proposed

Recipe: `head -1 src/console/organ_registry.hpp` → `#pragma once` ;
`head -1 src/coupling/organ_registry.hpp` → `#pragma once` ;
`grep -c "pragma once" <each>` → 1 each.

**Each file's sole include guard is a `#pragma once` in its own file preamble,
before `namespace t7 {` opens** — there is no named `#ifndef`/`#define` macro
guard in either. (**AMENDED / R3:** the earlier text anchored this to "line 1 of
each", a bare line number. The claim is unchanged and re-verified by `head -1`;
only the anchor is now the enclosing region — the file preamble — rather than a
line number.)

Both files `#define ORGAN_PARAM`, `ORGAN_PARAM_GEN`, `ORGAN_PARAM_DEF`,
`ORGAN_PARAM_DEFONLY`, `ORGAN_PARAM_RO`, the five `_NS` primaries, and the full
`ORGAN_BLOCK_*` (15 rows) / `ORGAN_DOOR_*` (4 rows) enum vocabulary, all inside
`namespace t7::organ`. Because `#pragma once` is keyed to a **file identity**,
not to a macro name, the two paths do not suppress one another: a TU that
included both would see both preprocessed, and the second set of `#define`s
would be redefinitions of the first. No TU in the tree includes both — the
single `#include` census in 7.b.1 is the witness. Recorded as a structural
property; no consequence is proposed.

---

### VERDICT

**ONE LIVE, ONE DEAD — `src/console/organ_registry.hpp` is live;
`src/coupling/organ_registry.hpp` is dead.**

Forced by the include evidence, in three independent strands (all recipes pinned
to `79adfa4d`):

1. **Exactly one `#include` of any `organ_registry.hpp` exists in the tracked
   tree** (`git grep -n '#include.*organ_registry' 79adfa4d` → one hit), and it
   names the console path: `src/cartridges/the_board/cartridge.hpp` :: file
   scope :: `#include "console/organ_registry.hpp"`.
2. **That include is unambiguous under the configured search paths.** The only
   `-I` root that can complete a rooted `console/…` spelling is
   `${CMAKE_SOURCE_DIR}/src` (the sole `target_include_directories(the_board
   PRIVATE …)` row naming the tree); the includer-relative candidate
   `src/cartridges/the_board/console/organ_registry.hpp` does not exist; and no
   configured `-I` root can make that spelling name a file under `src/coupling/`.
3. **Nothing anywhere names the coupling path.** `git grep -n
   "coupling/organ_registry" 79adfa4d` returns no hits at all, and the
   untracked-inclusive `grep -rn "coupling/organ_registry" --exclude-dir=.git .`
   run while the tree stood at that rev returned no hits either. No `#include`,
   no gate, no ledger tool, no doc, no `CMakeLists.txt` row.
   `tools/organ_readers.py` even excludes it from its reader scan by the
   path-suffix test `rel.endswith("organ_registry.hpp")`, so it is invisible to
   that census rather than reported by it. The one tool that touches the
   `src/coupling/` **directory** at all — `tools/gates/glaw1/gen_stubs.py` —
   contains the string `organ` zero times (`grep -c "organ"
   tools/gates/glaw1/gen_stubs.py` → 0) and harvests only `wgpu::`, `GLFW_*` and
   `glfw*()` symbols from it as text.

It is not "two homes for one fact" in the drift sense either: the two blobs carry
**538 identical code lines each with exactly one differing string literal** (the
`kOrganDoors` `static_assert` message). Every other one of the 161 changed lines
is prose. The `ORGAN_BLOCK_*` (15 rows) and `ORGAN_DOOR_*` (4 rows) enums, the
five `ORGAN_PARAM*` forwarding macros, the five `_NS` primaries, and all 17
symbols `organ_boundary.inc` consumes are declared identically in both. The
coupling blob `3047070e` is a frozen copy of what the console path held between
`4cfc899b` (WEB_SUNSET W3e) and `79adfa4d`; the console path was returned at
`79adfa4d` to its pre-W3e blob `70d09e96`.

Per R4, no repair, merge, or deletion is proposed here.

---

### GAPS RECORDED

* **G7-1.** `src/coupling/organ_registry.hpp` has zero readers: no `#include`, no
  gate TU, no ledger tool, no doc reference, no `CMakeLists.txt` mention.
* **G7-2.** `tools/organ_readers.py` excludes the coupling copy from its reader
  scan by suffix match, so no existing census would report it as unreferenced.
  The absence is invisible to the machine's own room.
* **G7-3.** `src/console/organ_params.inc` (blob `b426ac4f`) carries post-W3e
  "control surface"/"consumer" prose while the registry that includes it at the
  unit boundary carries pre-W3e "panel"/"shell" prose. The two files that compile
  together disagree in vocabulary.
* **G7-4.** `79adfa4d` ("Systems operational") reverted
  `src/console/organ_registry.hpp` to its exact pre-W3e blob and in the same
  commit stripped `CMakeLists.txt`'s trailing newline and inserted stray
  whitespace into `src/console/console.hpp` :: `t7::kCompilerPlan`. The commit
  message states no reason and the tree carries no ruling covering it (nothing in
  `docs/OPEN.md` was consulted for this unit beyond the campaign brief — see
  FLAG-1).
* **G7-5.** No `docs/HANDOFFS/` entry, `docs/LAWS.md` law, or `docs/OPEN.md` row
  was found by this unit that authorises or records a second organ registry. Not
  exhaustively searched — see FLAG-1.
* **G7-6.** `src/coupling/` is reachable to `tools/gates/glaw1/gen_stubs.py` as a
  **directory of text** (both the 6-entry `DIRS` list and the two inline 4-entry
  GLFW lists name `"coupling"`), so the dead registry's `wgpu::` and `GLFW_*`
  spellings, if any, are in scope for stub harvesting even though nothing
  compiles the file. No organ symbol is harvested; whether the dead file
  contributes any harvested symbol at all was not measured by this unit — see
  FLAG-3.
* **G7-7.** The two registries carry no distinguishing include guard. Each uses a
  bare `#pragma once` keyed to file identity, so neither path's macro
  vocabulary would be suppressed by the other's prior inclusion. No TU includes
  both, so this is a latent structural property rather than an observed effect.

### FLAGS

**FLAG-1 — `docs/OPEN.md` / `docs/LAWS.md` / `docs/HANDOFFS/` were not swept for
a ruling covering the duplication.** The unit scoped me to the include graph, the
diff, and the two `.inc` files, and a full doc sweep was out of that scope. Cost
to resolve: a `git grep -n` over `docs/` for `organ_registry`, `coupling`, and
`Systems operating|operational`, plus a read of `docs/OPEN.md` end to end —
roughly one additional tool round. Without it I cannot say whether `72df32df`
was authorised by a standing ruling, and G7-4/G7-5 are stated as "not found",
not as "does not exist".

**FLAG-2 — the verdict is a static-reachability verdict, not a build witness.**
The BUILD directive forbade running cmake/ninja/any compiler, so "dead" here
means *no configured include path or file reference reaches it*, established from
the `CMakeLists.txt` rows quoted in 7.b.2. Cost to resolve to a build witness:
one configure + one `-H` include-trace build on Jean's lane, which is his gate,
not mine.

**FLAG-3 — the glaw1 stub-harvest contribution of the dead registry was not
measured.** I established which directory lists `tools/gates/glaw1/gen_stubs.py`
walks and which harvests consume each (7.b.4 census B), and that the file
contains the string `organ` zero times. I did **not** run the harvest to
determine whether `src/coupling/organ_registry.hpp` uniquely contributes any
`wgpu::` type, `wgpu::X::Y` pair, `GLFW_*` macro or `glfw*()` function to the
generated stub set — i.e. whether the dead file is load-bearing for glaw1's
inputs. Cost to resolve: one read of `tools/gates/glaw1/gen_stubs.py` :: `harvest`
end to end plus a diff of the harvested sets with and without that one path,
computed by grep rather than by running the tool — roughly two additional tool
rounds. Until then G7-6 is stated as "in scope for harvesting", not as
"contributes nothing".

**FLAG-4 — the campaign boundary and the repository tip are no longer the same
commit; recorded, not attributed.** This unit's stated boundary is `79adfa4d`;
`git rev-parse HEAD` now returns `6d53388e`, one commit ahead, authored
`Claude <noreply@anthropic.com>` 2026-08-30 19:50:05 +0000, subject
"LIGATURE_0 — the recon report: the ligature is one hop, and the socket is
empty", `1 file changed, 8837 insertions(+)`, the file being
`docs/LIGATURE_0_RECON.md`. `git status --porcelain` is empty: no modified and
no untracked files. No source file was touched and all four blob SHAs this
section anchors on are unchanged at that tip. I did not create that commit and I
did not write any file under `/home/user/7T-Music`; this is recorded because the
hard read-only check compares tips, and because it is the reason every recipe in
7.b is now published with an explicit `79adfa4d` rev argument (see 7.0.1).

---

### AMENDMENT RECORD

Six verifier findings were re-checked against the tree by this amender before
being folded in. All six reproduced; none was rejected. No original finding was
overturned — every correction was to a count, a recipe, a quote's fidelity, or an
enclosing-symbol attribution, and the verdict is unchanged.

| # | where | what was wrong | what the tree says | re-check recipe |
| --- | --- | --- | --- | --- |
| 1 | 7.c.1 | "13 identical rows each" for `ORGAN_BLOCK_*`, contradicting the same sentence's own enumeration | **15** in each file | `grep -c "ORGAN_BLOCK_[A-Z_]* *=" <each>` → 15, 15; `diff <(grep -o "ORGAN_BLOCK_[A-Z_]* *= *[0-9]*" …) <(…)` → empty |
| 2 | 7.b.4 | `gen_stubs.py` row published under the `organ_registry` grep recipe, which cannot produce it; `DIRS` attributed to the GLFW harvests | `grep -c "organ" tools/gates/glaw1/gen_stubs.py` → **0**; `DIRS` (6 entries) feeds only `harvest(r"wgpu::(\w+)", DIRS)` and `harvest(r"wgpu::(\w+)::(\w+)", DIRS)`; the two GLFW harvests take an inline **4-entry** list | `grep -n "DIRS\|harvest(" tools/gates/glaw1/gen_stubs.py` |
| 3 | 7.c.2 | `take_doors_pending` block dropped two comment lines with no ellipsis | the two lines `// The door promises the LIVE definitions, so it names` / `// the live mood.` sit between `g_tier_def_dirty = true;` and `raise_orb_definition(…)` | `grep -n -B4 -A12 "take_doors_pending" src/cartridges/the_board/organ_boundary.inc` |
| 4 | 7.c.1 | contract block dropped seven continuation lines and truncated its last line mid-sentence | full block restored; final line reads `… sovereignty boundary. STRUCT is the` | `sed -n '15,38p' src/console/organ_params.inc` |
| 5 | 7.a.3 | prose-delta row named `t7::organ::lanes_of` as enclosing symbol | the sentence closes the `// ─── Type tags ───` comment block above the anonymous `enum : uint8_t`; `lanes_of` begins after the enum and has no preamble comment | `grep -n "happens to render" <both>` → console:40, coupling:46; `sed -n '36,58p' src/console/organ_registry.hpp` |
| 6 | header, 7.b.1 | two recipes published unpinned; they no longer reproduce now that the report is a tracked file | pinned to `79adfa4d` both reproduce **exactly** (1 hit; no output, exit 1). Unpinned at `6d53388e` they return 12 and 42, every added hit inside `docs/LIGATURE_0_RECON.md` | `git grep -n '#include.*organ_registry' 79adfa4d` ; `git grep -n 'coupling/organ_registry' 79adfa4d` |

Rule violations fixed: **R5** ×2 (findings 3 and 4 — both quotes restored
verbatim and contiguous, with the elisions named in an AMENDED note rather than
left silent); **R3** ×1 (7.c.3's "line 1 of each" replaced with the enclosing
file-preamble region plus a `head -1` recipe); **R2** ×2 (findings 1 and 2 — the
count corrected to match its recipe, and the `gen_stubs.py` row moved into its
own census B with its own recipe).

Census work added by this amendment, not present in the original section:
the 7.0.1 tip-movement census with its pinned/unpinned recipe table; the
`ORGAN_BLOCK_*` identity `diff` recipe and the 12-ids + 3-sentinels split; the
`ORGAN_DOOR_*` count and identity recipe (4 rows, identical); the
`tools/gates/glaw1/gen_stubs.py` symbol table separating `DIRS` from the two
inline GLFW lists; `wc -c` for `src/cartridges/the_board/organ_boundary.inc`
(8163, previously unfilled); the verbatim `organ_boundary.inc` preamble through
`void organ_flush(wgpu::Queue& queue) {`; the `// ─── Block ids ───` verbatim
prose-delta pair; gaps **G7-6** and **G7-7**; and flags **FLAG-3** and **FLAG-4**.

R1 status at amendment close: `git status --porcelain` at `/home/user/7T-Music`
→ empty. The only file this amender wrote is this section file under the
scratchpad. No `git checkout`, `git stash`, `git add`, `git commit`, or branch
change was performed; every git call was `rev-parse`, `grep`, `log`, `show`, or
`status`.

## 8. Docs and build

Recon anchor: repo `/home/user/7T-Music`, branch `claude/ligature-0-recon-hcrix0`,
HEAD **as of authoring** `79adfa4d26c9e17e0074692928f1d2875d7edde1`
(`git rev-parse HEAD` at entry). Every claim below is scoped to that commit and
was re-verified against it during amendment with explicit `79adfa4d:` revisions
rather than the bare `HEAD` alias. Working tree clean throughout; nothing under
the repo was written (`git status --porcelain` → empty, verified at entry and
again at amendment).

**AMENDMENT NOTE — HEAD MOVED AFTER THIS SECTION WAS WRITTEN.** At amendment
time `git rev-parse HEAD` returns `6d53388e83f4a5cd7ad3b154484c885f567a02da`
("LIGATURE_0 — the recon report: the ligature is one hop, and the socket is
empty"). The delta is exactly one added file:

```
git diff --name-status 79adfa4d 6d53388e
  → A	docs/LIGATURE_0_RECON.md
git cat-file -s 6d53388e:docs/LIGATURE_0_RECON.md
  → 521224
```

`git status --porcelain` is still empty — nothing is modified or untracked, and
this section's author wrote nothing into the repo. The drift invalidates three
statements *if read against the new HEAD*, and all three are re-stated below
scoped to `79adfa4d`:

| statement | true at `79adfa4d` | at `6d53388e` |
| --- | --- | --- |
| `find docs -type f` / `git ls-tree -r --name-only <rev> -- docs/` file count | **12** | **13** |
| `git grep -ril 'ligature' <rev> -- .` | **1 file** (`full_list.txt`) | **2 files** (`docs/LIGATURE_0_RECON.md`, `full_list.txt`) |
| `git log --all --oneline --grep='ligature' -i` | **0 commits** | **1 commit** (`6d53388e`) |

The blob SHAs the unit supplied for `CMakeLists.txt`, `CMakePresets.json` and
`src/console/console.hpp` are unchanged across that drift (verified in 8b.1
against both revisions), so 8b is unaffected.

---

### 8a — DOCS

#### 8a.1 What `docs/` holds at `79adfa4d`

Recipes:

```
ls -la docs/
find docs -type f | sort
git ls-tree -r --name-only 79adfa4d -- docs/
for f in $(git ls-tree -r --name-only 79adfa4d -- docs/ | sort); do
  printf '%s | %s | %s | %s\n' "$f" \
    "$(git cat-file -s 79adfa4d:"$f")" \
    "$(git rev-parse 79adfa4d:"$f")" \
    "$(git log -1 --format='%h %ad %s' --date=short 79adfa4d -- "$f")"
done
```

`ls -la docs/` verbatim (run at authoring time, before the HEAD drift):

```
total 172
drwxr-xr-x  3 root root  4096 Aug 30 18:57 .
drwxr-xr-x 11 root root  4096 Aug 30 18:57 ..
-rw-r--r--  1 root root 12220 Aug 29 18:24 7t_program_theory_v3.md
-rw-r--r--  1 root root  2619 Aug 29 18:24 CHORD.md
-rw-r--r--  1 root root 24724 Aug 30 18:57 FXC_LAWS_RECORD.md
-rw-r--r--  1 root root 59629 Aug 30 18:57 LAWS.md
-rw-r--r--  1 root root 13179 Aug 30 18:57 OPEN.md
-rw-r--r--  1 root root 14021 Aug 30 18:57 ORGAN.md
-rw-r--r--  1 root root 20783 Aug 30 18:57 PROCESS_LAWS.md
drwxr-xr-x  2 root root  4096 Aug 29 18:24 reference
```

The same command re-run at amendment time, i.e. against the working tree at
`6d53388e`, showing the one added file and the correspondingly larger `total`:

```
total 684
drwxr-xr-x  3 root root   4096 Aug 30 19:49 .
drwxr-xr-x 11 root root   4096 Aug 30 18:57 ..
-rw-r--r--  1 root root  12220 Aug 29 18:24 7t_program_theory_v3.md
-rw-r--r--  1 root root   2619 Aug 29 18:24 CHORD.md
-rw-r--r--  1 root root  24724 Aug 30 18:57 FXC_LAWS_RECORD.md
-rw-r--r--  1 root root  59629 Aug 30 18:57 LAWS.md
-rw-r--r--  1 root root 521224 Aug 30 19:49 LIGATURE_0_RECON.md
-rw-r--r--  1 root root  13179 Aug 30 18:57 OPEN.md
-rw-r--r--  1 root root  14021 Aug 30 18:57 ORGAN.md
-rw-r--r--  1 root root  20783 Aug 30 18:57 PROCESS_LAWS.md
drwxr-xr-x  2 root root   4096 Aug 29 18:24 reference
```

`find docs -type f | sort` returned **12 files** and was set-identical to
`git ls-tree -r --name-only 79adfa4d -- docs/` (also 12) — there were **no
untracked and no missing files** under `docs/`. (At `6d53388e` both counts are
13; see the amendment note above.)

Every row below re-verified at amendment time against `79adfa4d` with the loop
recipe printed above; all twelve blob SHAs, sizes and last-touching commits
reproduce byte for byte.

| path | size (B) | blob SHA (`git rev-parse 79adfa4d:<path>`) | last-touching commit | date | subject |
| --- | ---: | --- | --- | --- | --- |
| `docs/7t_program_theory_v3.md` | 12220 | `bf36d995c9503fff4c3d42ae89874a60d23c8c9e` | `f9239ce7` | 2026-08-18 | RECENSION_1 [R2]: the panel ruling — ORGAN is the panel of record; the JSX sibling line dies |
| `docs/CHORD.md` | 2619 | `83b62a173e92280791693490bb81b7c100e0e107` | `20755a9b` | 2026-08-18 | RECENSION_1 [R3]: CHORD.md frame_r row — BEQ_A's passenger enters the taxonomy (1040 B) |
| `docs/FXC_LAWS_RECORD.md` | 24724 | `2923b82cf04bbbe2827c865213141ec2805d3816` | `810a2667` | 2026-08-29 | WEB_SUNSET W6b: LAWS, OPEN, banners |
| `docs/LAWS.md` | 59629 | `595082461c203ea23124a2222694d0747e7c9946` | `d7d65a3e` | 2026-08-30 | PRUNE_1 U7 — the bit, the gates, the ledgers, the record |
| `docs/OPEN.md` | 13179 | `008b477e61be15dc41f24042aef4d0b896600fc3` | `d7d65a3e` | 2026-08-30 | PRUNE_1 U7 — the bit, the gates, the ledgers, the record |
| `docs/ORGAN.md` | 14021 | `7639b022b788e1c694c95ad84ece494c2719af38` | `810a2667` | 2026-08-29 | WEB_SUNSET W6b: LAWS, OPEN, banners |
| `docs/PROCESS_LAWS.md` | 20783 | `dd1c241e0275035e935422da4a07c629126600f5` | `bc91cf3a` | 2026-08-29 | HELM_0 H3: the paper follows the preset surface |
| `docs/reference/ATTIC.md` | 17423 | `444b7c4a64019f3139f145e5543ba67f88ba12ef` | `eac338be` | 2026-08-26 | D7 — the docs of record, and one entry opened rather than closed (ATTIC_ATRIUM) |
| `docs/reference/DAWN_REFERENCE.md` | 7701 | `e42a1935a53d411b464f39598245414e42057d56` | `d2d42bee` | 2026-08-18 | docs: amend Dev Prompt claim — the shell appends; the law is L39 |
| `docs/reference/RELEASE_CONSOLE.md` | 29010 | `b12e77adafdc97501106c4bec55d13aee9da11aa` | `7a132d2a` | 2026-08-18 | CANON C4: the machine room purifies; the specimen shelves |
| `docs/reference/WEBGPU_SPEC.pdf` | 18275310 | `ad4307453c3ec8f8d7fed9fa38fa118fc0f21a3f` | `a8f4580d` | 2026-08-17 | WINNOW-2 W4: the law shelf assembles; paths lose their spaces |
| `docs/reference/WGSL_SPEC.pdf` | 16686506 | `c41020be482a81065254341eab98fa48d5e118e4` | `a8f4580d` | 2026-08-17 | WINNOW-2 W4: the law shelf assembles; paths lose their spaces |

Note on the `ls -la` "total 172": the two PDFs sit in `docs/reference/`, not
in `docs/` itself, so the directory listing above does not account for their
~35 MB. The `find` census does.

#### 8a.2 FINDING — the handoff's `docs/CHORD.md` recovery step is MOOT

**Both `docs/CHORD.md` and `docs/7t_program_theory_v3.md` are PRESENT AND
TRACKED at `79adfa4d`.** Verified two independent ways:

```
git ls-tree -r --name-only 79adfa4d -- docs/          # both appear
git cat-file -e 79adfa4d:docs/CHORD.md                # exit 0
git cat-file -e 79adfa4d:docs/7t_program_theory_v3.md # exit 0
```

The on-disk copies are byte-identical to those blobs
(`git show 79adfa4d:docs/CHORD.md | diff - docs/CHORD.md` → no output, exit 0).

The LIGATURE_0 handoff's recovery step — `git show <sha>:docs/CHORD.md`
redirected into a `.recovered` file in the source tree — is therefore
**moot and was not performed**. It would (i) write a new file into
`/home/user/7T-Music`, violating R1, and (ii) create a second, stale copy of
a document that is already live at its canonical path. **No recovery was
run. Nothing was written into the repo.** The correct read of either file is
a plain `cat docs/CHORD.md`, or `git show 79adfa4d:docs/CHORD.md`.

#### 8a.3 Was either doc ever deleted and restored?

Recipes:

```
git log --oneline --all -- docs/CHORD.md
git log --oneline --all -- docs/7t_program_theory_v3.md
git log --all --format='%h %ad %s' --date=short --name-status --follow -- docs/CHORD.md
git log --all --format='%h %ad %s' --date=short --name-status --follow -- docs/7t_program_theory_v3.md
```

`git log --oneline --all -- docs/CHORD.md`:

```
20755a9b RECENSION_1 [R3]: CHORD.md frame_r row — BEQ_A's passenger enters the taxonomy (1040 B)
6265eb40 CHORD_0: charter — the taxonomy, the blocks, the rulings of record
```

`git log --oneline --all -- docs/7t_program_theory_v3.md`:

```
f9239ce7 RECENSION_1 [R2]: the panel ruling — ORGAN is the panel of record; the JSX sibling line dies
a8f4580d WINNOW-2 W4: the law shelf assembles; paths lose their spaces
95d2fe4c PROBATE_X: the wider tree probated — remaining FXC sites ruled; DAWN_REFERENCE marked as archive
4f12479d src/docs -> docs: the reorg's tail, including one silent breakage
9f0109f8 HANDOFFS FOR WEB
```

Rename-following name-status, verbatim:

| doc | full status chain (oldest → newest) | ever `D`? |
| --- | --- | --- |
| `docs/CHORD.md` | `A` at `6265eb40` (2026-08-16, "CHORD_0: charter…") → `M` at `20755a9b` (2026-08-18) | **NO** |
| `docs/7t_program_theory_v3.md` | `A` at `46eb2d94` (2026-07-12, "Theory V3") as `src/docs/7t_program_theory_v3.md` → `M` at `6bc66d20` (2026-07-13) → `R100 src/docs/… → docs/…` at `9f0109f8` (2026-08-07, "HANDOFFS FOR WEB") → `M` at `4f12479d`, `95d2fe4c`, `a8f4580d`, `f9239ce7` | **NO** |

**Neither file was ever deleted.** `docs/CHORD.md` has never lived at any
other path. `docs/7t_program_theory_v3.md` was *renamed* once (`R100`,
100 % similarity, `src/docs/` → `docs/`) and never removed. Both predate the
fork point `de4b8b6f` and survived WEB_SUNSET untouched.

Corroborated against the deletion census of 8a.4: neither path appears in
`deleted_docs.txt` (recipe:
`grep -E '\|\|\| docs/(CHORD\.md|7t_program_theory_v3\.md)$' $SCRATCH/deleted_pairs.txt`
→ no output), and the pre-reorg path `src/docs/7t_program_theory_v3.md` appears
there only as the source side of the `R100` rename, never as a standalone `D`.

Note on `docs/CHORD.md`'s adding commit `6265eb40` and the tag
`native-sunset` = `29cec46b`, both dated 2026-08-16: CHORD_0 and the first
sunset are same-day siblings. No contradiction with the orchestrator's
established context; recorded for the plan's timeline.

#### 8a.4 Census — coupling-design docs that existed in history and are ABSENT at `79adfa4d`

Recipe (published in full; run from repo root, writes only into the
scratchpad `$SCRATCH` =
`/tmp/claude-0/-home-user-7T-Music/e4d04ccd-accf-5afe-8420-1b448d1fd519/scratchpad`):

```
# 1 — every deletion event on every ref, commit-tagged
git log --all --diff-filter=D --format='COMMIT %H %h %ad %s' --date=short \
    --name-only > $SCRATCH/deleted_all.txt          # 1046 lines

# 2 — re-associate each deleted path with its deleting commit
awk '/^COMMIT /{c=$0; next} /^$/{next} {print c" ||| "$0}' $SCRATCH/deleted_all.txt \
  > $SCRATCH/deleted_pairs.txt                       # 876 pairs

# 3a — the docs-scoped set the unit asks for
grep -E '\|\|\| (docs/|src/docs/)' $SCRATCH/deleted_pairs.txt \
  > $SCRATCH/deleted_docs.txt                        # 146 deletion events
awk -F'\\|\\|\\| ' '{print $2}' $SCRATCH/deleted_docs.txt | sort -u | wc -l   # 146 unique paths

# 3a-bis — are events and unique paths the same set? (no path deleted twice)
awk -F'\\|\\|\\| ' '{print $2}' $SCRATCH/deleted_docs.txt | sort | uniq -c | awk '$1>1'
  → (no output — every path has count 1)

# 3b — the topic filter
awk -F'\\|\\|\\| ' '{print $2}' $SCRATCH/deleted_docs.txt | sort -u \
  | grep -Ei 'coupling|chord|ligature|canvas|organ|signal'            # 6 hits

# 4 — absence proof, per hit
git cat-file -e "79adfa4d:<path>"   # non-zero exit ⇒ absent at 79adfa4d
```

**AMENDED COUNT.** An earlier draft of this section reported "143 unique
paths"; re-running the recipe above verbatim gives **146**, and step 3a-bis
shows every path in the docs-scoped stream has count 1, so the deletion-event
count (146) and the unique-path count (146) are *necessarily* equal — there is
nothing to collapse. **146 is the number.** The topic-filter result (6) is
unaffected and reproduced exactly.

Boundary: the census covers **all refs** (`--all`), the whole reachable
history (2228 commits, clone un-shallowed), and both the pre-reorg
`src/docs/` and post-reorg `docs/` roots. **146** unique doc paths have been
deleted at least once; **6** match the topic filter.

**The six topic-matching deleted docs (all confirmed ABSENT at `79adfa4d`):**

| # | path | deleting commit | date | deleting subject | size at `<sha>^` | recovery command (REPORTED, NOT RUN) |
| --- | --- | --- | --- | --- | ---: | --- |
| 1 | `docs/HANDOFFS/CHORD/CHORD_HANDOFF.md` | `0a6e2880` | 2026-08-17 | WINNOW-2 T-h: handoffs die at CLOSE | 30612 (668 lines) | `git show 0a6e2880^:docs/HANDOFFS/CHORD/CHORD_HANDOFF.md` |
| 2 | `docs/HANDOFFS/CHORD/CHORD_ROUND_REPORT.md` | `0a6e2880` | 2026-08-17 | WINNOW-2 T-h: handoffs die at CLOSE | 19802 (373 lines) | `git show 0a6e2880^:docs/HANDOFFS/CHORD/CHORD_ROUND_REPORT.md` |
| 3 | `docs/HANDOFFS/ORGAN/ORGAN_1_ROUND_REPORT.md` | `0a6e2880` | 2026-08-17 | WINNOW-2 T-h: handoffs die at CLOSE | 9057 (177 lines) | `git show 0a6e2880^:docs/HANDOFFS/ORGAN/ORGAN_1_ROUND_REPORT.md` |
| 4 | `docs/HANDOFFS/ORGAN_2c_RECON.md` | `37c2624e` | 2026-08-18 | ORGAN_2c U4 — the recon is consumed (docs/HANDOFFS empties) | 14665 (338 lines) | `git show 37c2624e^:docs/HANDOFFS/ORGAN_2c_RECON.md` |
| 5 | `docs/past docs/COUPLING_SAGA_FINISHER.txt` | `dff09eeb` | 2026-08-17 | WINNOW-2 T-d: past docs | 9090 (141 lines) | `git show 'dff09eeb^:docs/past docs/COUPLING_SAGA_FINISHER.txt'` |
| 6 | `docs/past docs/ribbon_color_coupling_datasheet.md` | `dff09eeb` | 2026-08-17 | WINNOW-2 T-d: past docs | 18213 (256 lines) | `git show 'dff09eeb^:docs/past docs/ribbon_color_coupling_datasheet.md'` |

Sizes and line counts re-verified at amendment with
`git show '<sha>^:<path>' | wc -c` and `| wc -l`.

Each was added exactly once and deleted exactly once (no restore cycles):
`0a6e2880` deletes #1–#3 (added `29cec46b` / `a4388fe2` / `a92b7687`,
all 2026-08-16); `37c2624e` deletes #4 (added `c80b74db`, 2026-08-18);
`dff09eeb` deletes #5–#6 (both added `9f0109f8`, 2026-08-07).
Note the paths at rows 5–6 contain a **space** (`docs/past docs/`) — the
recovery commands above quote the whole `<rev>:<path>` argument accordingly.

Term census inside the six (recipe, run once per file per term:
`git show '<sha>^:<path>' | grep -ciE '<term>'` — `-c` counts *matching lines*,
not occurrences):

| # | `ligature` | `\bpipes?\b` | `\bstats?\b` | `coupl` |
| --- | ---: | ---: | ---: | ---: |
| 1 CHORD_HANDOFF | 0 | 0 | 0 | 0 |
| 2 CHORD_ROUND_REPORT | 0 | 0 | 0 | 0 |
| 3 ORGAN_1_ROUND_REPORT | 0 | 1 | 0 | 2 |
| 4 ORGAN_2c_RECON | 0 | 0 | 0 | 4 |
| 5 COUPLING_SAGA_FINISHER | 0 | 0 | **1** | **3** |
| 6 ribbon_color_coupling_datasheet | 0 | 9 | 0 | 17 |

**AMENDED ROW 5.** An earlier draft recorded row 5 as `\bstats?\b` = 0 and
`coupl` = 0. Re-running the published recipe gives 1 and 3. The three `coupl`
lines are, verbatim (`git show 'dff09eeb^:docs/past docs/COUPLING_SAGA_FINISHER.txt' | grep -niE 'coupl'`):

```
7:COUPLING_SAGA — THE FINISHER (mop patch + floaters rename + T1 + flip)
8:Branch: continue on COUPLING_SAGA_SWEEP2
135:green. Jean-side leftover unchanged: the origin/COUPLING_SAGA deletion
```

and the single `\bstats?\b` line is
(`… | grep -niE '\bstats?\b'`):

```
15:`git show --stat` of the addendum commits to explain how 04 landed with
```

**FINDING — #5 is a false positive of the name filter.** `COUPLING_SAGA_FINISHER.txt`
carries "coupl" on exactly three lines: its own title line, the branch name it
rides on (`COUPLING_SAGA_SWEEP2`), and one reference to a deleted `origin/COUPLING_SAGA`
branch. No line of its body describes a coupling. Its single `stat` hit is the
git subcommand `--stat`, not a signal slot. It is a geometry/naming close-out
(mop patch, `floater_vocabulary.hpp` → `floaters.hpp`, `TileGrid` runtime
sizing, `PATCH_PREGEN_RADIUS 7 → 8`) and has **nothing to do with the
visual/musical coupling**. Verbatim header (whole lines; the `...` marks an
elision of lines 9–13, a companion-file/apply preamble):

```
═══════════════════════════════════════════════════════════════════════
COUPLING_SAGA — THE FINISHER (mop patch + floaters rename + T1 + flip)
Branch: continue on COUPLING_SAGA_SWEEP2
Companion file: mop.patch (apply from repo root, -p1)
═══════════════════════════════════════════════════════════════════════
...
COMMIT PLAN
  F1  MOP        apply mop.patch (comments + one free code rename)
  F2  FLOATERS   floater_vocabulary.hpp → floaters.hpp (mv + sweep)
  F3  T1         TileGrid runtime-sizing (completes the twin set)
  F4  FLIP       PATCH_PREGEN_RADIUS 7 → 8 (DISCLOSED — the weld)
```

**FINDING — #6 is the one deleted `docs/` file that IS the coupling's design
record.** `docs/past docs/ribbon_color_coupling_datasheet.md` (18213 B, 256
lines, deleted `dff09eeb` 2026-08-17) carries the pipe/stat two-sided naming
system in full. It is too long to quote whole; the load-bearing passages
follow, verbatim via
`git show 'dff09eeb^:docs/past docs/ribbon_color_coupling_datasheet.md'`.
Every fenced block below is whole source lines; `...` inside a fence marks an
elision, and no fence begins or ends mid-source-line.

The write side is called a **pipe**, and pipes are `PARAM_LAYOUT` rows in
`visual_canvas.hpp`. The document's title block and `## 0. ROW REGISTER`
section (the `...` elides the two-line "Standing rule" paragraph, quoted
separately below, and the blank lines around it):

```
# RIBBON COLOR & CELLS — COUPLING DATASHEET (post-SS-3)
The couplable surface of the ribbon's color system and cell skin, exposed in
contract-datasheet form. Classes per the standing key: L-global (body-wide,
Segment-safe), LH (through the head's history), D (discrete; selection +
state inheritance), C (identity/law; not a live target). Every pipe listed
with its idle, because rest = identity is the safety contract.
...
## 0. ROW REGISTER (the wiggle table — contract §6 schema)
Every pipe, one row: name · slot · width · shape · class · REST · what
the eye sees when it moves · guardrail. Slots are PARAM_LAYOUT rows
(visual_canvas.hpp); "—" = declared intent, no row yet (ledger-backed,
wires-on-demand per contract R9). Clamps are WRITE-SIDE (the canvas's
goal laws), declared per row per contract R13.

| name | slot | width | shape | class | REST | the eye sees | guardrail |
|------|------|-------|-------|-------|------|--------------|-----------|
| fog.density | 0 | 1 | Scalar | L-global (scene) | field-table value | fog thickens/thins with the room's field | held→table, glides over FOG_SPAN 2.0 beats |
| fog.color | 1–3 | 3 | Vector | L-global (scene) | field-table color | fog hue follows the field | held→table, same span |
| ribbon.amp_lateral_mult | 4 | 1 | Scalar | L-global | 1 | the dance widens; carve + bank deepen with it (BNK-1) | write-side: goal = 1 + (2−1)·t ∈ [1, 2], ceiling RULED |
| ribbon.amp_vertical_mult | 5 | 1 | Scalar | L-global | 1 | the bob deepens | write-side: same law, same clamp |
| ribbon.color_stim | 6–8 | 3 | Vector | L-global | 0,0,0 | tint hue re-aims toward the room's harmonic center | luma/chroma authored (TINT_LUMA 0.55 / TINT_CHROMA 0.35); holds last hue in silence |
| ribbon.color_mix | 9 | 1 | Scalar | L-global | 0 | tint fades in on sound, out on silence | write-side: mix_goal ∈ {0, TINT_MIX_MAX 0.85} |
| ribbon.color_b | — | 3 | Vector | L-global | spawn light median | contrast collapses/expands (chessboard ⇄ field) | commit-only today; wire on demand |
| ribbon.checker_scatter | — | 1 | Scalar | L-global | spawn draw | cell lightness texture energy | multiplier idiom, rest 1; commit-only today |
| ribbon.hue_spread | — | 1 | Scalar | L-global | spawn draw | the riot dial — per-cell hue fan | additive deviation, rest 0, range [0, π]; commit-only today |
| ribbon.seed | — | 1 | — | C | spawn seed | (identity; never moves) | never coupled |
| ribbon.color_mode | — | 1 | Discrete | D (birth) | seed roll × weights | species change (SMOOTH/TINTED/CONTRAST) | rebirth-class only |

Casting note: the ribbon's voice is RIBBON_VOICE = "ch1" (the casting
sheet's first row); the room rows read "all.*". Envelope constants:
swell attack 0.35 / release 2.0; tint mix attack 0.5 / release 3.0;
hue re-aim 2.0 (ENV-1).
```

The read side is a **dotted address**, `chN.<reading>` per voice and
`all.<reading>` room-wide, published out of `canvas_1/canvas.hpp` — this is
the "stat" half of the two-sided system, though the datasheet never uses the
word "stat" (`grep -ciE '\bstats?\b'` → 0). The bullet in full:

```
- CHANNEL NAMES (ledger, SS-2): the scaffolding names (abbott/costello/
  louise) are retired; channels are numbered chN. The currency caveat is
  CLOSED definitively — the live publish inventory, read from
  canvas_1/canvas.hpp: per voice (ch0..ch6 active of ch0..ch7 named):
  chN.current_pc, chN.present_count, chN.window_length, chN.distance;
  room-wide: all.field, all.current_pc, all.present_count,
  all.window_length. PresentCount was published on demand by SS-2 (the
  sustain law is its first consumer — readings-on-demand firing on
  schedule); its reserved layout was slot 0, the seat identified as
  gen-1's dead input during the demolition — the oldest empty chair in
  the contract, now filled by its intended occupant.
```

The two sides are named together in `## 6. RELATED SURFACES ALREADY LIVE OR
TAGGED`. **AMENDED — this block previously ended mid-source-line at "only ever
gives."** It is reproduced here to whole-line boundaries, which carries the
sentence to its end:

```
## 6. RELATED SURFACES ALREADY LIVE OR TAGGED
fog.density / fog.color — the played coupling (all.field, held→table).
ribbon.amp_lateral_mult / ribbon.amp_vertical_mult — PARAM_LAYOUT rows
4–5, LIVE (coupling #2, REDESIGNED by SS-2, made ADDITIVE by SS-3:
sustain swell — movement carries TIME): the dance swells with how long
the current chord has held, uninterrupted, on the ribbon's cast voice
("ch1.present_count", the Playhead's sounding set; the casting sheet's
first row). LAW: 1 + contribution (additive; idle inviolate) — music
only ever gives. Any change to the set re-articulates: breathe to
baseline (1.0), regrow. RULED: ceiling 2× idle, reached at 8 beats.
Silence ⇒ 1 from the formula itself (identity by construction, not by
branch). ENVELOPE (ENV-1): attack 0.35 / release 2.0 beats — fast
catch, slow let-go; release governs the re-articulation breath and the
after-silence let-go (span chosen at the call site: goal == idle ⇒
RELEASE). Multipliers composed over the spawn-drawn wave amps at the
conductor's per-frame flush; the pawn mount reads the same mirror —
the rider breathes with the coupled dance for free.
```

and the rest law is stated as the safety contract:

```
## IDLE MAP SUMMARY (what silence looks like, per pipe)
Every color/cell pipe's rest reproduces the seed-drawn, pair-raffled OR
free-raffled skin exactly. A stranger reading only this table can wiggle
each row on the future panel and predict the screen — that is the
datasheet's test.
```

Two further passages from the same file, quoted as whole source lines because
they fix vocabulary the plan will need. The document's own standing rule
(the paragraph elided from the title-block quote above):

```
Standing rule: this file updates in the SAME COMMIT as any change to the
surface it describes.
```

and the categorical boundary, stated per-cell at the close of `## 4`:

```
Every expression is fixed structure; couplings act ONLY through the §1
scalars — the cells never learn what drove them (the categorical boundary,
per-cell edition).
```

#### 8a.5 Safety net — topic-matching deletions OUTSIDE `docs/`

The unit scopes the census to `docs/`; the same pair file was re-filtered
with the docs prefixes **excluded**, because a coupling-design doc can live
outside `docs/` in this repo (`audit/` is the machine's room, L28):

```
awk -F'\\|\\|\\| ' '{print $2}' $SCRATCH/deleted_pairs.txt \
  | grep -Ei 'coupling|chord|ligature|canvas|organ|signal' \
  | grep -viE '^(docs/|src/docs/)' | sort -u | wc -l      # 46
```

**AMENDED COUNT.** An earlier draft said "47 hits". Re-running the recipe
verbatim gives **46**, with or without `sort -u` (the un-deduplicated stream is
also 46, i.e. no path outside `docs/` in this topic set was deleted twice).
**46 is the number.** The full 46, verbatim from the recipe above:

```
audit/past reports/SIGNAL_SOURCE_LEDGER.md
audit/past reports/TERRAIN_COUPLING_LEDGER.md
build.cmake43/check_canvas_compound.vcxproj
build.cmake43/check_canvas_compound.vcxproj.filters
build.cmake43/check_canvas_union.vcxproj
build.cmake43/check_canvas_union.vcxproj.filters
build.cmake43/probe_canvas.vcxproj
build.cmake43/probe_canvas.vcxproj.filters
src/analysis/canvas_1/canvas.hpp
src/analysis/canvas_1/check_canvas_compound.cpp
src/analysis/canvas_1/check_canvas_union.cpp
src/analysis/canvas_1/check_field_union.cpp
src/analysis/canvas_1/check_pc_dft.cpp
src/analysis/canvas_1/probe_canvas.cpp
src/analysis/polyphony_basic/canvas.hpp
src/cartridges/the_chord/cartridge.hpp
src/cartridges/the_chord/modules/agents.inl
src/cartridges/the_chord/modules/cube_behaviors.inl
src/cartridges/the_chord/modules/entities.inl
src/cartridges/the_chord/modules/entity_pipeline.inl
src/cartridges/the_chord/modules/entity_types.inl
src/cartridges/the_chord/modules/floater_vocabulary.inl
src/cartridges/the_chord/modules/gallery.inl
src/cartridges/the_chord/modules/gol_zones.inl
src/cartridges/the_chord/modules/ground_architecture.inl
src/cartridges/the_chord/modules/input.inl
src/cartridges/the_chord/modules/mood.inl
src/cartridges/the_chord/modules/musical.inl
src/cartridges/the_chord/modules/orbs.inl
src/cartridges/the_chord/modules/pawn.inl
src/cartridges/the_chord/modules/render_passes.inl
src/cartridges/the_chord/modules/ribbon.inl
src/cartridges/the_chord/modules/seed_utils.inl
src/cartridges/the_chord/modules/spawn_engine.inl
src/cartridges/the_chord/pattern_glossary_v1.6.md
src/cartridges/the_chord/renderer.hpp
src/cartridges/the_chord/rollout_open_questions.md
src/cartridges/the_chord/state.hpp
src/cartridges/the_chord/world.wgsl
src/dev/BACKUP 15 06/check_canvas_field.cpp
src/dev/BACKUP 15 06/check_canvas_present.cpp
src/dev/BACKUP 15 06/check_canvas_publish.cpp
src/dev/BACKUP 15 06/check_canvas_window.cpp
src/dev/compare this/canvas.hpp
src/dev/compare this/check_canvas_field_global.cpp
web/organ_panel.js
```

**AMENDED — FOUR prose documents, not two.** An earlier draft said "Two of the
hits are prose documents rather than code/project files". Filtering the same 46
to prose extensions gives four:

```
awk -F'\\|\\|\\| ' '{print $2}' $SCRATCH/deleted_pairs.txt \
  | grep -Ei 'coupling|chord|ligature|canvas|organ|signal' \
  | grep -viE '^(docs/|src/docs/)' | sort -u | grep -Ei '\.(md|txt)$'
  → audit/past reports/SIGNAL_SOURCE_LEDGER.md
  → audit/past reports/TERRAIN_COUPLING_LEDGER.md
  → src/cartridges/the_chord/pattern_glossary_v1.6.md
  → src/cartridges/the_chord/rollout_open_questions.md
```

All four are absent at `79adfa4d` (`git cat-file -e '79adfa4d:<path>'` →
non-zero for each). `audit/` at `79adfa4d` holds only the five generated
ledgers (`git ls-tree -r --name-only 79adfa4d -- audit/`):
`BINDING_LEDGER.md`, `COMMAND_LEDGER.md`, `MANIFEST.md`, `MIRROR_LEDGER.md`,
`ORGAN.md`.

| path | deleting commit | date | subject | size at `^` | `\bpipes?\b` / `\bstats?\b` hits | at `79adfa4d` | recovery command (REPORTED, NOT RUN) |
| --- | --- | --- | --- | ---: | --- | --- | --- |
| `audit/past reports/SIGNAL_SOURCE_LEDGER.md` | `585e99de` | 2026-08-17 | WINNOW-2 T-e: past reports | 7434 | 2 / 13 | ABSENT | `git show '585e99de^:audit/past reports/SIGNAL_SOURCE_LEDGER.md'` |
| `audit/past reports/TERRAIN_COUPLING_LEDGER.md` | `585e99de` | 2026-08-17 | WINNOW-2 T-e: past reports | 8541 | 0 / 0 | ABSENT | `git show '585e99de^:audit/past reports/TERRAIN_COUPLING_LEDGER.md'` |
| `src/cartridges/the_chord/pattern_glossary_v1.6.md` | `464f2ca0` | 2026-07-02 | simple changes | 75025 | 0 / 0 | ABSENT | `git show '464f2ca0^:src/cartridges/the_chord/pattern_glossary_v1.6.md'` |
| `src/cartridges/the_chord/rollout_open_questions.md` | `464f2ca0` | 2026-07-02 | simple changes | 75904 | 0 / 0 | ABSENT | `git show '464f2ca0^:src/cartridges/the_chord/rollout_open_questions.md'` |

The two `the_chord/` documents match the topic filter only through the
*directory* name `the_chord`, not through their own content vocabulary: each
has 0 hits for `\bpipes?\b` and 0 for `\bstats?\b` (`coupl` hits 26 and 16
lines respectively). They are the cloned `the_chord` cartridge's own paperwork
— `pattern_glossary_v1.6.md` opens "# Pattern Glossary / The source-side
reference for tag interpretation." and `rollout_open_questions.md` opens
"# Rollout open-questions report" — added by `5ccb4afc` (2026-05-20,
"the_chord — clone of the_board with namespace/path rename (no logic
changes)") and removed by `464f2ca0`. At 75 KB each they are not quoted here;
their recovery commands are in the table.

`SIGNAL_SOURCE_LEDGER.md` is short (7434 B) and is *the* surviving record of
the **stat** side, so it is quoted at length here rather than recovered. The
quotes below are consecutive whole source lines; each is introduced by the
section of the document it comes from, and the gaps between them are named
rather than silently jumped.

Its ARCHIVAL banner and title block:

```
> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# RECON A — THE SIGNAL SOURCE LEDGER (the music half's first survey)

The SOURCE wall of the patch bay: what the analyzer computes, what it ships,
what anything reads. Read-only, verified at HEAD `66f582c`. **STOP.**
```

**AMENDED — `## §0 HEADLINE` restored.** An earlier draft ran the title block
straight into `## §2` across a bare blank line, silently dropping a `---` rule,
the whole of `## §0 HEADLINE` and the whole of `## §1 THE PRODUCTION PATH`.
`§0` is materially relevant to this campaign — it names the *live* stats
consumers — so it is quoted here in full:

```
## §0 HEADLINE

The analyzer is a **MIDI pitch-class engine** — no audio path, no frequency
domain (DFT/FFT: absent everywhere, code AND design). The signal is 4128
bytes (time trio + 8 channels × 128 stat slots); the GPU relay windows
**channel 0, slots 0–63 (the 256-byte block)** — and at HEAD that window is
**read by NO shader**: every WGSL consumer was retired at M1-C (tombstones
below). The only live stats consumers are CPU couplings
(`visual_canvas.tick` — fog + voice/room playheads) and the_lab's
dashboards. Meanwhile the analysis half computes and DISCARDS a coupling
goldmine every frame: onset/release masks + counts, field-election strength
and ambiguity, tempo + play-state, note velocity, spine provenance. Chroma
exists as computation (12-bin pitch-class vectors throughout), absent only
as a label.
```

Between `§0` and `§2` sits `## §1 THE PRODUCTION PATH (canvas/port → harvest)`,
a single 12-line paragraph tracing
`RtMidi thread → MidiPort::on_rtmidi_callback → MidiTransport::feed / the
lock-free ring → Canvas::update → route → Context::receive → advance →
step_fields → publish → output() → the harness → the_board update(signal,…) →
U1 phase_fill_signal`. It is not quoted in full here; it is named so the jump
to `§2` is not silent.

`## §2 THE SIGNAL + THE SLOT MAP`, in full:

```
## §2 THE SIGNAL + THE SLOT MAP

`src/analysis/analysis_signal.hpp:72-105`: `{ t_seconds, t_beats, dt,
_pad0, stats[1024], _pad1[4] }` — 8 channels × 128 slots
(`stat_index = ch·128 + slot`, :64), `sizeof == 4128` static_asserted
(:107). DOC BUG: the :80 comment says "2048 bytes"; the block is 4096.
Channel assignment (canvas.hpp:118-133): **voices 0–6 → channels 0–6;
the group union → channel 7** (`all.*`). Pitch classes re-origined to D
(`PROJECT_PC_ORIGIN=2`) before write (:422-423).

The canonical per-channel slot map (canvas.hpp:283-290) + producers:

| slot | name | contents | units | producer | wired? |
|---|---|---|---|---|---|
| 0-11 | PRESENT_COUNT | sounding notes per pitch class | count | `pc_count(playhead)` pc_count.hpp:38 → write canvas.hpp:417,442 | YES |
| 12-23 | PRESENT_LENGTH | present pcs by provisional length | beats | capability `pc_length(ph)` pc_count.hpp:68 | **NO** (unwired) |
| 24-35 | WINDOW_COUNT | present+window occurrences | count | capability pc_count.hpp:56 | **NO** (unwired) |
| 36-47 | WINDOW_LENGTH | present+window length per pc | beats | pc_count.hpp:88 → write canvas.hpp:418 | YES |
| 48-59 | CURRENT_PC | current line note, one-hot | 0/1 | `current_note(spine)` spine_ops.hpp:36 → write canvas.hpp:416 | YES |
| 60 | DISTANCE | signed prev→current interval | semitones | `line_distance` spine_ops.hpp:46 → write canvas.hpp:429 | YES |
| 61 | FIELD | held harmonic-field rank | index 1..6 | `HeldField::step` field.hpp:115 → write canvas.hpp:414 | YES (group band) |
| 62 | POLYPHONY | present voice count | count | — | **NO WRITER EXISTS** |

Time trio: `t_beats` ← transport beats (exact); `t_seconds` ← wall clock
accumulation (canvas.hpp:140); `dt` ← wall frame delta.
```

(The `file:line` citations inside that block are the archived document's own
text, reproduced verbatim; they are not this section's citations.)

`## §4 THE READ WALLS`, in full. **AMENDED — the third bullet is restored.** An
earlier draft ended this quote after the second bullet while introducing it as
"the read-wall verdict", which suppressed the bullet naming the_lab as the only
FULL consumer. `§4` has three bullets and all three are here:

```
## §4 THE READ WALLS

- **GPU: ZERO live `signal.stats` reads in world.wgsl.** Three tombstones
  only: 1823 ("stats[0] terrain-amplitude coupling, retired M1-C"),
  6695 + 6980 ("DRIVERLESS (M1-C): raw signal.stats[0] substituted").
  **The 256-byte relay is a dead pipe at HEAD** — uploaded every frame,
  consumed by nothing.
- CPU (the_board): exactly cartridge.hpp U1/U3/U7 (time trio →
  gpuSignal/time_state_; the U7 transition timer reads dt) +
  visual_canvas.tick's named couplings. No signal reads anywhere in
  machine/, bodies/, direction/.
- the_lab: the only FULL consumer — StatShape-dispatched dashboards sweep
  channels × slots (the_lab.cpp:180-208, 294/315/332, 450-481).
```

So the read-wall verdict, stated whole: the GPU side of the pipe was dead at
that document's date; the CPU side had two readers, `visual_canvas.tick`'s
named couplings and the_lab's dashboards, the latter being the only consumer
that swept the whole channels × slots grid.

The `SHIPPED BUT NEVER READ:` block from `## §6 THE DELTAS`, in full.
**AMENDED — the middle bullet is restored.** An earlier draft silently dropped
`` - `_pad0`, `_pad1[4]`. `` from inside the fence:

```
SHIPPED BUT NEVER READ:
- `stats[64..1023]` (voices 1-6 + the group band) — never relayed to GPU;
  CPU-reachable only via the layout handshake (visual_canvas/the_lab).
- `_pad0`, `_pad1[4]`.
- **The entire GPU stats window** (channel 0, slots 0-63) — relayed, then
  read by no shader (§4). The pipe is pressurized from both ends and
  connected to nothing in the middle.
```

`TERRAIN_COUPLING_LEDGER.md` is the sibling survey of the terrain feed
points; it uses neither "pipe" nor "stat"
(`git show '585e99de^:audit/past reports/TERRAIN_COUPLING_LEDGER.md' | grep -ciE '\bpipes?\b'`
→ 0; same for `\bstats?\b` → 0). Its headline and `## §1`, quoted to whole-line
boundaries — **AMENDED**, an earlier draft ended this block mid-source-line at
"— this block.":

```
# TERRAIN COUPLING LEDGER (minimal-demo workbench; consolidation, deltas verified at HEAD `950607f`)

Method: the LADDER already holds the design history (the coupling strip, the
two-wave-systems trap, DEMO-2's disposition, P-5) — NOT re-derived. Two
delta-tracers confirmed the FEED POINTS only, file:line each; one tracer
conflation corrected by hand (§1-A note). **Read-only. STOP for rulings.**

---

## §1 THE FEED POINTS AT HEAD (confirmed)

THE BOOT-PIN BLOCK — one address holds every pinned voice input
(`cartridge.hpp:406-411`): `band_motion(inactive[-1]×6, zeros)` ·
`terrain_time(0.0f)` · `mode_color_shift(0.0f)` · `mode_checker_scatter(0.0f)`.
Every setter is live code with exactly one caller — this block. The
could-be authors all exist; nothing feeds them.
```

Both `audit/past reports/` files carry the identical standing ARCHIVAL banner
(verified by `git show '585e99de^:<path>' | head -5` on each):

```
> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.
```

and both therefore cite `docs/HANDOFFS/OPTIMIZATION 1/`, a directory that does
not exist at `79adfa4d` (see 8a.8).

#### 8a.6 What `docs/CHORD.md` and `docs/7t_program_theory_v3.md` say about the coupling

Recipes:

```
cat docs/CHORD.md
cat docs/7t_program_theory_v3.md
# substring form (what an earlier draft published):
grep -niE 'pipe|stat|stats|ligature|coupl' docs/CHORD.md
# word-boundary form (what the verdict below actually rests on):
for t in 'coupl' '\bpipes?\b' '\bstats?\b' 'ligature'; do
  printf '%s = %s\n' "$t" "$(grep -ciE "$t" docs/CHORD.md)"; done
for t in 'coupl' '\bpipes?\b' '\bstats?\b' 'ligature'; do
  printf '%s = %s\n' "$t" "$(grep -ciE "$t" docs/7t_program_theory_v3.md)"; done
grep -rniE 'ligature' docs/ --include=*.md
grep -rnoiE '\bpipes?\b' docs/ --include=*.md | awk -F: '{print $1}' | sort | uniq -c
grep -rnoiE '\bstats?\b' docs/ --include=*.md | awk -F: '{print $1}' | sort | uniq -c
```

**AMENDED RECIPE — the substring grep does NOT return nothing.** An earlier
draft published `grep -niE 'pipe|stat|stats|ligature|coupl' docs/CHORD.md` as
returning "NOTHING". It returns four lines (exit 0), every one of them a
substring `stat` inside an unrelated word:

```
3:LOOM stratified the GROUPS by cadence (world / frame / family state /
23:  copyBufferToBuffer from the GPU-sovereign state each frame — the
31:  ribbon-state appearing in two blocks is two windows on one home, and
39:- SINGLE PATH, RESTATED: the Pixel offers f16, subgroups, dual-source
```

— "stratified", "state", "ribbon-state", "RESTATED". None is the signal-slot
noun "stat". The verdict rests on the word-boundary form, which returns zero
for every term:

| term (regex) | `docs/CHORD.md` matching lines |
| --- | ---: |
| `coupl` | 0 |
| `\bpipes?\b` | 0 |
| `\bstats?\b` | 0 |
| `ligature` | 0 |

**`docs/CHORD.md` (blob `83b62a17`, 2619 B) — zero coupling content.** CHORD is
a GPU-uniform redistricting charter: it stratifies bytes by cadence into four
blocks and states four rulings of record. It is about buffer layout, not about
the musical→visual coupling. Its opening states its scope (whole lines):

```
# CHORD — uniform-seat redistricting by cadence

LOOM stratified the GROUPS by cadence (world / frame / family state /
family textures). CHORD stratifies the BYTES: seats of one cadence and
one author merge into one block, one binding, one upload per beat of
their clock. Scarcity was a symptom of granularity — the program ran
out of seats because it bound objects, not categories.

## The taxonomy
Every datum classifies by three questions: WHEN it changes (cadence),
HOW the GPU touches it (access), WHO authors it (CPU intent / GPU
truth). The cell answers the layout question mechanically. This
document is the taxonomy's record until ORGAN gives it an instrument.
```

The single ruling most likely to be mistaken for a coupling statement is the
first entry of `## Rulings of record`, which is about GPU block duplication,
not about musical channels (whole lines):

```
- WINDOWS, NOT HOMES: a fact's home is its one CPU-side struct and its
  one authoring site. GPU blocks are transport windows; tier_gains and
  ribbon-state appearing in two blocks is two windows on one home, and
  the authoring site writes every window it owns.
```

**`docs/7t_program_theory_v3.md` (blob `bf36d995`, 12220 B) — the coupling
appears as THEORY, never as the pipe/stat mechanism.** Word-boundary census
(`grep -ciE '<term>' docs/7t_program_theory_v3.md`): `coupl` hits **8** lines;
`\bpipes?\b` hits **1** line; `\bstats?\b` hits **0**; `ligature` hits **0**.
The eight `coupl` lines, verbatim from
`grep -niE 'coupl' docs/7t_program_theory_v3.md`:

```
38:L0 SUBSTRATE · L1 PRIMITIVES · L2 ENTITIES · L3 COUPLINGS · L4
40:COUPLED; no third kind. Coupled = a named channel, one author, a
85:BEHAVIOR (owns runtime state; anatomy per §10) + COUPLINGS. The
154:always, coupled to the anchor, part of the theatrical illusion: no
157:THE ANCHOR (§9 Act III) is what the camera couples to.
166:proximity READINGS for couplings. Proximity becomes musical
198:composition (coupling set, seed, mood profile). The flagship pair —
216:noun is a role or a cast, the census of couplings equals the census
```

The passages that bear on the coupling, each quoted as whole source lines:

The central law — §2 — defines what a coupling *is* and gives the rest law
that the datasheet in 8a.4 turns into the pipe's REST column:

```
L0 SUBSTRATE · L1 PRIMITIVES · L2 ENTITIES · L3 COUPLINGS · L4
ORCHESTRATION · L5 REALIZATION. Every behavior is AUTONOMOUS or
COUPLED; no third kind. Coupled = a named channel, one author, a
REST reproducing the autonomous self exactly, a composition law, the
categorical boundary (receivers get values, never sources).
COMPOSITION LAWS COME IN TWO KINDS: scalar deviations compose
ADDITIVELY over inviolate idleness; intent authorship composes by
ARBITRATION (possession is the precedent). The generalized silence
test holds in both. Entities share functions and vocabulary freely,
NEVER state.
```

The word **"pipes"** occurs once, in §14's demo grammar, and it means the
*visual* axis D2, not the musical channel:

```
## 14 — THE DEMO GRAMMAR (v3; the contract's summary)
Five axes a demo varies: D1 the surface cast (extent;
streamed-procedural or generated-once) · D2 the surface's voice
(geometry and color pipes; the wave rewired) · D3 the population
(roster + design tables + future profiles) · D4 the skins · D5 the
composition (coupling set, seed, mood profile). The flagship pair —
the gallery exhibit and the musician template — is the completion
test: one grammar, both sentences, a stranger composing a third
without editing a relation.
```

§11 declares proximity as a future *reading* source — the closest the theory
comes to naming the stat side, and it names it as unbuilt. **AMENDED** — an
earlier draft began this fence mid-source-line at "Declared underdeveloped";
the source line begins "the art of spending it. ". The block is reproduced here
from a whole-line boundary, which restores the paragraph that sets up the
declaration:

```
THE BUBBLE: around the anchor lives a bounded radius of awareness —
and the bound is the theory, not an obstacle: AWARENESS IS A SCARCE,
BUDGETED RESOURCE (N slots, hardware-shaped; the 64-solid resolve
loop and the aura grid are its two existing casts). COLLISION
ENTITLEMENT (T3) means buying slots in the budget; playability is
the art of spending it. Declared underdeveloped, direction M-q; its
destiny is doubled: the interaction system, AND an EVENT SOURCE —
entries, exits, counts, nearest-distance — a coming family of
proximity READINGS for couplings. Proximity becomes musical
material; the aura was the prototype all along.
```

§3 states that the two halves share one vocabulary by design — the sentence
that licenses the analyzer/render symmetry. **AMENDED** — an earlier draft
ended this fence mid-source-line at "bodies.)"; the source line continues
"K1 HAS MODES: …". Reproduced here to whole-line boundaries:

```
MAP, TERRITORY, REFERENCE FRAMES, CONVERGENCE, GROUNDING — that is
the terrain: the stage all interaction is measured against, the one
mandatory relation, the constrainer of spatial distribution and
movement. (The program's two halves share this vocabulary by design:
the analyzer's territory grounds pitch as the terrain grounds
bodies.) K1 HAS MODES: bound / referenced / anchored-elsewhere. K1 IS
BIDIRECTIONAL: entities may write the ground; the Surface carries the
authoring channel. THE LOCAL-FRAME LAW: entities speak frame_at,
never world axes; the geodesic price is exactly this discipline.
```

§9 makes the music a *driver*, peer to input and algorithm. **AMENDED —
attribution corrected and the re-wrap removed.** An earlier draft attributed
the driver triple to "§9 Act II" and rendered "What moves may move
individually or collectively; F6's gather and a held chord" as one 78-character
line that exists nowhere in the file. The driver triple is in **ACT I**; the
F6/held-chord sentence is the tail of **ACT II**; and the source breaks the
line after "What moves". Both acts are quoted here in full, at the source's own
wrapping:

```
## 9 — THE DRIVER LAW, THREE ACTS, AND THE FOUR STRATA
ACT I — A MOVING THING IS A BODY TIMES A DRIVER. The body owns K1
mode, constraints, the integrator, the realization; drivers (input /
algorithm / music) write intents through it; arbitration decides
authorship; the body never learns who drove it. The player is a
driver, not a protagonist.
ACT II — INTENTS HAVE ADDRESSES (v3): one body, a species, a spatial
set, or the trait-class MOVERS itself. Each addressed body TRANSLATES
the verb through its own gait under its own constraints. What moves
may move individually or collectively; F6's gather and a held chord
gathering the world are one sentence with different authors.
Direction currently smeared into species (corral, kite, hardwired
key routing) migrates to the direction layer (§13) as pulled.
```

Finally, the completion criterion makes the coupling↔channel census the
program's own done-test:

```
The engine is done when every influence is a declared channel, every
noun is a role or a cast, the census of couplings equals the census
of channels, the deletion test passes for every module — and the
flagship pair runs as two sentences of one template.
```

**Summary of the two documents, as asked by the unit.** `docs/CHORD.md` says
*nothing* about the visual/musical coupling: it is a byte-layout charter whose
subject is which GPU uniform block a datum sits in, chosen by cadence, access
and author, with four named blocks (`agent_room`, `field_bus`, `frame_r`,
`scene_constants`) and four rulings of record (WINDOWS-NOT-HOMES, the CHORD_5
storage reversal, SINGLE PATH RESTATED, DYNAMIC OFFSETS STAY AT 0/8 AND 0/4).
It carries neither half of the pipe/stat naming system.
`docs/7t_program_theory_v3.md` carries the coupling as *law*, not as
*mechanism*: a coupling is "a named channel, one author, a REST reproducing the
autonomous self exactly, a composition law, the categorical boundary (receivers
get values, never sources)", scalar deviations compose additively over
inviolate idleness, music is one of three drivers alongside input and
algorithm, proximity is declared as a *future* family of READINGS, and the
program is "done when … the census of couplings equals the census of channels".
It uses "pipes" once and in the visual sense, and never uses "stat" at all — so
the two-sided pipe/stat naming that the live code uses (8b.5, 8b.6) appears in
neither document.

#### 8a.7 GAP — the word "ligature" is nowhere in the documentation

Recipes and results, all scoped to `79adfa4d`:

```
grep -rniE 'ligature' docs/ --include=*.md            # 0 hits (at 79adfa4d)
git grep -ril 'ligature' 79adfa4d -- .                # 1 file: 79adfa4d:full_list.txt
grep -n -i 'ligature' full_list.txt
  → 7671:C:\dev\7t\node_modules\caniuse-lite\data\features\kerning-pairs-ligatures.js
git log --all --oneline --grep='ligature' -i          # 0 commits (at authoring time)
```

**At `79adfa4d`, "LIGATURE" appears in NO document, NO source file, and NO
commit subject in the entire reachable history.** The only textual hit anywhere
in that tree is an unrelated CSS-feature filename inside `full_list.txt` (a
635450-byte Windows directory listing of `C:\dev\7t`, last touched `6c38eff8`
2026-08-25 "atrium pics"). The campaign name LIGATURE_0 is therefore **new
vocabulary with no prior art in this repo at the commit this section anchors
to** — recorded as a gap, not a defect.

**AMENDMENT — the gap has since been filled by this campaign's own output.**
At `6d53388e` (the HEAD at amendment time) `git grep -ril 'ligature' HEAD -- .`
returns two files (`docs/LIGATURE_0_RECON.md`, `full_list.txt`) and
`git log --all --oneline --grep='ligature' -i` returns one commit (`6d53388e`
itself). The gap as recorded above is the state of the tree *before* the recon
report landed; it is not evidence of prior art.

Adjacent term census across `docs/`, at `79adfa4d` (recipe:
`for f in $(git ls-tree -r --name-only 79adfa4d -- docs/ | grep '\.md$'); do
git show "79adfa4d:$f" | grep -coiE '<term>'; done`, and cross-checked on disk
with `grep -rnoiE '<term>' docs/ --include=*.md | awk -F: '{print $1}' | sort | uniq -c`):

| term (regex) | files with hits | hits | detail |
| --- | ---: | ---: | --- |
| `ligature` | 0 | 0 | — |
| `\bpipes?\b` | 1 | 1 | `docs/7t_program_theory_v3.md`, §14: `(geometry and color pipes; the wave rewired) · D3 the population` |
| `\bstats?\b` | 1 | 3 | `docs/reference/ATTIC.md` ×3 |

**AMENDED ATTRIBUTION — the three ATTIC.md hits are in the attic entries'
DESCRIPTION text, not in the attic-tag names.** An earlier draft attributed
them to the tag names `attic/STAT_PANELS_S2B`, `attic/STAT_PANELS_S2A`,
`attic/TRAIN_MAX_STATS_64`. Those names cannot match `\bstats?\b` at all,
because `_` is a word character on both sides of `STAT` / `STATS`. The three
matched tokens, from `grep -oiE '[A-Za-z-]*\bstats?\b[A-Za-z-]*' docs/reference/ATTIC.md`:

```
vector-stat
stat-layout
stat
```

and the three whole rows they sit in, from
`grep -niE '\bstats?\b' docs/reference/ATTIC.md`:

```
110:| `attic/STAT_PANELS_S2B` | 2026-05-29 | y | STAT_PANELS_S2B: vector-stat history heatmap-strips + scope split |  |
111:| `attic/STAT_PANELS_S2A` | 2026-05-29 | y | STAT_PANELS_S2A: stat-layout descriptor + shape-aware lab panels |  |
113:| `attic/TRAIN_MAX_STATS_64` | 2026-05-28 | y | TRAIN_MAX_STATS_64: raise per-Train stat cap from 32 to 64 |  |
```

(The third row's description reads "raise per-Train stat cap from 32 to 64";
the matched token there is the bare word `stat`.) The tag names appear in the
same rows, but they are not what the regex matched.

**GAP.** The pipe/stat two-sided naming system that the live code uses (see
8b.5, 8b.6) is documented in **no file under `docs/` at `79adfa4d`**. Its only
prose records are the two deleted archives quoted in 8a.4 (#6,
`ribbon_color_coupling_datasheet.md`) and 8a.5 (`SIGNAL_SOURCE_LEDGER.md`),
both removed on 2026-08-17 by the WINNOW-2 sweep (`dff09eeb` and `585e99de`
respectively), and both carrying the ARCHIVAL banner that forbids citing them
as current.

#### 8a.8 `docs/HANDOFFS/` — ABSENT (health)

Recipes and results:

```
ls -la docs/HANDOFFS
  → ls: cannot access 'docs/HANDOFFS': No such file or directory
git ls-tree -r --name-only 79adfa4d -- docs/HANDOFFS | wc -l
  → 0
find docs -type f | sort            # 12 files at 79adfa4d, none under HANDOFFS/
```

**`docs/HANDOFFS/` DOES NOT EXIST at `79adfa4d` — neither on disk nor in the
tree.** Per `CLAUDE.md` ("The directory exists only while work is open —
absence is health"), this reads as **HEALTH: no open work order is filed in
the tree.** The directory's last inhabitant was
`docs/HANDOFFS/ORGAN_2c_RECON.md`, removed by `37c2624e` (2026-08-18,
"ORGAN_2c U4 — the recon is consumed (docs/HANDOFFS empties)"); the bulk
emptying happened one day earlier at `0a6e2880` (2026-08-17, "WINNOW-2 T-h:
handoffs die at CLOSE"). The related path `docs/HANDOFFS/OPTIMIZATION 1/`,
cited as live by both `audit/past reports/` archives quoted in 8a.5, is also
absent, and it never lived at exactly that path in this repo. The census
resolves it (recipe:
`awk -F'\\|\\|\\| ' '{print $2}' $SCRATCH/deleted_docs.txt | grep 'OPTIMIZATION 1/'`
→ **14** files, every one under `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/`,
all deleted by the single commit `6e726ea4` (2026-08-17, "WINNOW-2 T-c: past
campaigns")). Before the `src/docs/` → `docs/` reorg the same files sat at
`src/docs/HANDOFFS/OPTIMIZATION 1/`
(`git log --all --name-only --format='' | grep -i 'OPTIMIZATION' | sort -u`),
which is the path shape the two ARCHIVAL banners cite. Nothing matching
`OPTIMIZATION` exists at `79adfa4d`
(`git ls-tree -r --name-only 79adfa4d | grep -i 'OPTIMIZATION'` → no output).

---

### 8b — BUILD

#### 8b.1 Blob SHAs verified

Recipe:

```
git rev-parse HEAD:CMakeLists.txt HEAD:CMakePresets.json HEAD:src/console/console.hpp
git rev-parse 79adfa4d:CMakeLists.txt 79adfa4d:CMakePresets.json 79adfa4d:src/console/console.hpp
```

Result — identical at `79adfa4d` and at the amendment-time HEAD `6d53388e`
(the only delta between those commits is the added `docs/LIGATURE_0_RECON.md`):

```
2dddc9202f4d74650e28f95b3aa536ddb81cda9a   CMakeLists.txt
8f2298c00e9ac326af50cb621cd755947ebe2432   CMakePresets.json
577c486049956d7c977580351b764967e3cb3d6c   src/console/console.hpp
```

**All three match the values the unit supplied, exactly.** `CMakeLists.txt`
is 884 lines, `CMakePresets.json` is 76 lines (`wc -l`).

#### 8b.2 Presets

Recipe: `cat CMakePresets.json` (whole file, 76 lines — tabulated below rather
than re-pasted).

**configurePresets** (5 entries, `"version": 3`):

| preset name | hidden | generator | inherits | config / build type | what it selects (cacheVariables) |
| --- | --- | --- | --- | --- | --- |
| `native` | **yes** | `Ninja` | — | none set | `T7_RENDER_CARTRIDGE=the_board`, `THE_BOARD_DEMO=full`, `DAWN_DIR=C:/dev/dawn`, `DAWN_BUILD=C:/dev/dawn/out`; `binaryDir=${sourceDir}/out/build/${presetName}` |
| `the-board-full-release` | no | Ninja (inherited) | `native` | `CMAKE_BUILD_TYPE=Release` | the program: cartridge `the_board`, demo column `full` |
| `the-board-full-release-meter` | no | Ninja (inherited) | `the-board-full-release` | Release (inherited) | adds `T7_INSTRUMENTS=meter` — arms the frame meter |
| `the-board-minimal` | no | Ninja (inherited) | `the-board-full-release` | Release (inherited) | overrides `THE_BOARD_DEMO=minimal` |
| `the-board-vs` | no | **`Visual Studio 18 2026`** | `native` | multi-config (no `CMAKE_BUILD_TYPE`) | same cache as `native`; configuration chosen at build time via `--config` |

**buildPresets** (5 entries; every one targets `the_board` and nothing else):

| build preset | configurePreset | configuration | targets |
| --- | --- | --- | --- |
| `the-board-full-release` | `the-board-full-release` | (single-config) | `["the_board"]` |
| `the-board-full-release-meter` | `the-board-full-release-meter` | (single-config) | `["the_board"]` |
| `the-board-minimal` | `the-board-minimal` | (single-config) | `["the_board"]` |
| `the-board-vs-release` | `the-board-vs` | `Release` | `["the_board"]` |
| `the-board-vs-debug` | `the-board-vs` | `Debug` | `["the_board"]` |

**Cross-check against the four names `CLAUDE.md` mentions:**

| name in `CLAUDE.md` | exists in `CMakePresets.json`? | where | note |
| --- | --- | --- | --- |
| `the-board-full-release` | **YES** | configurePreset + buildPreset, same name | the invocation `CLAUDE.md` prints (`cmake --preset` / `cmake --build --preset`) resolves on both lists |
| `the-board-vs` | **YES**, configurePreset only | configurePresets | **There is NO buildPreset named `the-board-vs`.** The build side splits into `the-board-vs-release` and `the-board-vs-debug`. `CLAUDE.md`'s "`the-board-vs` is the same program on the Visual Studio multi-config lane, where `--config Debug` is the diagnostic build" describes the *configure* preset plus a raw `--config`, which is consistent with the file; but a literal `cmake --build --preset the-board-vs` names nothing. |
| `the-board-minimal` | **YES** | configurePreset + buildPreset, same name | selects `THE_BOARD_DEMO=minimal` |
| "`-meter` arms the frame meter" | **YES**, as the preset SUFFIX `-meter` | `the-board-full-release-meter` (both lists) | `CLAUDE.md` writes it as a flag-ish "`-meter`"; in the file it is a preset-name suffix, not a CMake option. `CMakeLists.txt` corroborates by name: "`the-board-full-release-meter` set T7_INSTRUMENTS=meter". |

**Presets in the file that `CLAUDE.md` does NOT name:** `native` (hidden
base — never invoked directly), `the-board-full-release-meter` (named only
obliquely as "`-meter`"), `the-board-vs-release`, `the-board-vs-debug`.

**Presets `CLAUDE.md` names that do not exist:** none, with the one
qualification above (`the-board-vs` exists as a *configure* preset only).

No `testPresets`, no `packagePresets`, no `workflowPresets` in the file.

#### 8b.3 Targets and their sources

Recipe:

```
grep -nE '^\s*(add_executable|add_library|target_sources)' CMakeLists.txt
find . -name CMakeLists.txt -not -path './third_party/*' -not -path './.git/*'
```

Results: **exactly one `add_executable`, zero `add_library`, zero
`target_sources`, and exactly one `CMakeLists.txt` in the whole repo**
(`./CMakeLists.txt` — no `add_subdirectory` tree). Re-verified at amendment:
`grep -nE '^\s*(add_executable|add_library|target_sources)' CMakeLists.txt`
returns exactly one row, `add_executable(the_board`.

The source rows, verbatim (enclosing symbol: the `add_executable(the_board …)`
call in the "THE BOARD — Render cartridge development" section):

```cmake
# Only the active cartridge's headers → scoped IntelliSense
file(GLOB_RECURSE T7_RENDER_HEADERS
    "src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp"
)

add_executable(the_board
    src/the_board.cpp
    # The one other translation unit: RtMidi's Windows MM backend, the
    # canvas's route to the DAW's virtual port. Vendored, not header-only,
    # so it compiles separately; __WINDOWS_MM__ below selects its backend.
    src/external/RtMidi.cpp
    ${T7_RENDER_HEADERS}
)
```

So the build has **exactly two compiled translation units**:
`src/the_board.cpp` and `src/external/RtMidi.cpp`. `${T7_RENDER_HEADERS}` is
a `GLOB_RECURSE` over `src/cartridges/the_board/*.hpp` — headers listed for
IDE indexing, not compiled.

#### 8b.4 ANSWER — are `src/analysis/canvas_1/*.cpp` still built?

**NO. Not under any target. There is no target that could build them.**

Recipe and evidence:

```
git ls-tree -r --name-only 79adfa4d -- src | grep -E '\.cpp$'
```

```
src/analysis/canvas_1/check_canvas_compound.cpp
src/analysis/canvas_1/check_canvas_union.cpp
src/analysis/canvas_1/check_field_union.cpp
src/analysis/canvas_1/check_pc_dft.cpp
src/analysis/canvas_1/probe_canvas.cpp
src/external/RtMidi.cpp
src/the_board.cpp
```

Seven `.cpp` files exist under `src/`; the single `add_executable` names two
of them. The five `canvas_1` files are named by **no** CMake row.

**AMENDED — there are TWO globs in `CMakeLists.txt`, not one.** An earlier
draft said "the only glob in the file is
`src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp`". `grep -n 'file(GLOB' CMakeLists.txt`
returns two rows:

```
688:file(GLOB_RECURSE T7_RENDER_HEADERS
841:    file(GLOB T7_SDK_DXC_DIRS
```

The second is inside the DXC-locator block and globs Windows SDK *directories*,
not sources. Verbatim, with its own explanatory comment:

```cmake
    # Then the Windows SDK, which ships both. Newest first; the version
    # segment is glob-expanded rather than pinned so a host with a
    # different SDK still resolves.
    file(GLOB T7_SDK_DXC_DIRS
        "C:/Program Files (x86)/Windows Kits/10/bin/*/x64")
```

**The conclusion survives the correction:** neither glob can reach
`src/analysis/canvas_1/*.cpp` — the first matches only
`src/cartridges/the_board/**/*.hpp` (wrong directory, wrong extension), and the
second matches only directories under `C:/Program Files (x86)/Windows Kits/10/bin/`.

Cross-check that nothing else names them:

```
git grep -n -e 'check_canvas_compound' -e 'check_canvas_union' \
   -e 'check_field_union' -e 'check_pc_dft' -e 'probe_canvas' 79adfa4d -- . \
 | grep -v '^79adfa4d:src/analysis/canvas_1/'
  → (no output)
```

**No file outside `src/analysis/canvas_1/` mentions any of the five by
name** — not `CMakeLists.txt`, not a preset, not a gate script, not a doc.
They are orphaned standalone `main()`-style probes.

Historical note (same deletion census as 8a.5). **AMENDED — four of the five,
not all five, were once carried by generated MSVC project files.** Recipe:

```
awk -F'\|\|\| ' '{print $2}' $SCRATCH/deleted_pairs.txt \
  | grep -E '^build\.cmake43/.*vcxproj' | sort -u
```

which returns, among the whole `build.cmake43/` project set
(`ALL_BUILD`, `ZERO_CHECK`, `incubator`, `incubator_backup`, `incubator_dual`,
`the_lab`, and two `CMakeFiles/` compiler-probe projects), exactly these
probe projects:

```
build.cmake43/check_canvas_compound.vcxproj
build.cmake43/check_canvas_compound.vcxproj.filters
build.cmake43/check_canvas_union.vcxproj
build.cmake43/check_canvas_union.vcxproj.filters
build.cmake43/check_field_union.vcxproj
build.cmake43/check_field_union.vcxproj.filters
build.cmake43/probe_canvas.vcxproj
build.cmake43/probe_canvas.vcxproj.filters
```

`check_field_union` — omitted by the earlier draft — has a project pair;
**`check_pc_dft` never had one anywhere in reachable history**:

```
git log --all --diff-filter=A --name-only --format='' \
  | grep -iE 'check_field_union|check_pc_dft' | sort -u
  → build.cmake43/check_field_union.vcxproj
  → build.cmake43/check_field_union.vcxproj.filters
  → src/analysis/canvas_1/check_field_union.cpp
  → src/analysis/canvas_1/check_pc_dft.cpp
  → src/dev/compare this/check_field_union.cpp
git log --all --name-only --format='' | grep -i 'check_pc_dft' | sort -u
  → src/analysis/canvas_1/check_pc_dft.cpp
```

(The `.vcxproj` files are CMake-generated Visual Studio projects that were
committed into the tree at the time, not hand-authored build rows; they are all
deleted at `79adfa4d` — `git ls-tree -r --name-only 79adfa4d -- build.cmake43`
→ no output.)

All five `.cpp` themselves appear in the `--diff-filter=D` census, i.e. they
were deleted at some point in history and later re-added (the orchestrator's
established context names `0c951b11` / `e0e22e46`, "bringing back the music",
2026-08-30, as the re-adding merge). **They came back as files; they did not
come back as build rows.**

#### 8b.5 Include paths — are `src/musical/`, `src/sources/`, `src/coupling/` reachable?

**YES, all three are on the main target's include path — by one blanket row,
not by three named rows.** All three `target_include_directories` rows attach
to `the_board`, `PRIVATE` (`grep -n 'target_include_directories' CMakeLists.txt`
→ three rows). Verbatim:

```cmake
target_include_directories(the_board PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
```

```cmake
# The native includes and link list, both from the Dawn section above
# (SUNRISE_0 N4).
target_include_directories(the_board PRIVATE ${DAWN_INCLUDES})
target_link_libraries(the_board PRIVATE ${DAWN_LIBS})
```

```cmake
target_include_directories(the_board PRIVATE "${T7_STAMP_DIR}")
```

Because `${CMAKE_SOURCE_DIR}/src` is on the path, every quoted include of
the form `"musical/…"`, `"sources/…"`, `"coupling/…"` resolves. That is the
idiom the tree actually uses:

```
git grep -n -E '#include "(coupling|musical|sources)/' 79adfa4d -- src
```

Cited by enclosing symbol rather than by line number (R3): the sole `coupling/`
row in the **top-level include preamble of `src/cartridges/the_board/cartridge.hpp`**
is `#include "coupling/visual_canvas.hpp"`; the sole `musical/` row in the
**include preamble of `src/coupling/visual_canvas.hpp`** is
`#include "musical/signal_layout.hpp"`; and the sole `sources/` row in the
**include preamble of `src/musical/context.hpp`** is
`#include "sources/midi_event.hpp"`. The per-file counts of such rows
(`git grep -c -E '#include "(coupling|musical|sources)/' 79adfa4d -- src`):

```
79adfa4d:src/analysis/canvas_1/canvas.hpp:10
79adfa4d:src/analysis/canvas_1/check_canvas_compound.cpp:1
79adfa4d:src/analysis/canvas_1/check_canvas_union.cpp:1
79adfa4d:src/analysis/canvas_1/check_field_union.cpp:1
79adfa4d:src/analysis/canvas_1/check_pc_dft.cpp:1
79adfa4d:src/analysis/canvas_1/probe_canvas.cpp:1
79adfa4d:src/cartridges/the_board/cartridge.hpp:1
79adfa4d:src/console/organ_registry.hpp:1
79adfa4d:src/coupling/organ_registry.hpp:1
79adfa4d:src/coupling/visual_canvas.hpp:4
79adfa4d:src/musical/context.hpp:6
79adfa4d:src/musical/context_realize.hpp:3
79adfa4d:src/musical/field.hpp:3
79adfa4d:src/musical/midi_stream.hpp:2
79adfa4d:src/musical/pc_count.hpp:3
79adfa4d:src/musical/pc_dft.hpp:1
79adfa4d:src/musical/playhead.hpp:1
79adfa4d:src/musical/spine_ops.hpp:2
79adfa4d:src/musical/vector_dressing.hpp:1
79adfa4d:src/musical/wagon.hpp:1
79adfa4d:src/sources/keyboard_midi.hpp:1
79adfa4d:src/sources/midi_file.hpp:1
79adfa4d:src/sources/midi_port.hpp:2
```

(Twenty-three files carry at least one such row. The trailing integer is the
count of matching lines in that file, not a line number.)

**Reachable-on-the-path is not the same as compiled.** Transitive
quoted-include closure from the one real TU (recipe: a Python walk over
`#include "…"` from `src/the_board.cpp`, resolving each header against
`src/` and against the includer's directory; script kept in the scratchpad at
`/tmp/claude-0/-home-user-7T-Music/e4d04ccd-accf-5afe-8420-1b448d1fd519/scratchpad/reach.py`,
re-run at amendment with identical output) gives, per subtree:

| subtree | headers at `79adfa4d` (`git ls-tree -r --name-only 79adfa4d -- src/<d>`, `.hpp`/`.inc`) | reached from `the_board.cpp` | which |
| --- | ---: | ---: | --- |
| `src/coupling/` | 5 | **4** | `canvas_surface.hpp`, `trajectory.hpp`, `visual_canvas.hpp`, `visual_params.hpp` (NOT `organ_registry.hpp`) |
| `src/musical/` | 16 | **1** | `signal_layout.hpp` only |
| `src/sources/` | 5 | **0** | none |
| `src/analysis/` | 4 | 2 | `analysis_signal.hpp`, `beat_clock.hpp` (NOT `analysis_cartridge.hpp`, NOT `canvas_1/canvas.hpp`) |
| `src/console/` | 5 | 5 | `console.hpp`, `features_wallet.gen.inc`, `limits_floor.gen.inc`, `organ_params.inc`, `organ_registry.hpp` |
| `src/cartridges/the_board/` | — | 46 | the cartridge tree |
| `src/core/` | 6 | 4 | `boot_params.hpp`, `cartridge_ids.hpp`, `input_event.hpp`, `instruments.hpp` |
| `src/render/` | 1 | 1 | `render_cartridge.hpp` |

**FINDING — `src/coupling/organ_registry.hpp` has no includer and is not
compiled.** Two files of that basename exist at `79adfa4d`:

```
git ls-tree -r --name-only 79adfa4d | grep organ_registry
  → src/console/organ_registry.hpp
  → src/coupling/organ_registry.hpp
```

`git grep 'organ_registry' 79adfa4d -- src tools` returns twelve rows, verbatim:

```
79adfa4d:src/cartridges/the_board/cartridge.hpp:#include "console/organ_registry.hpp"   // the compiled dial registry + its C ABI (needs the home types above)
79adfa4d:src/cartridges/the_board/organ_boundary.inc:// organ_registry.hpp owns the flags and masks consumed here and knows neither
79adfa4d:src/console/organ_params.inc:// organ_registry.hpp compiles these lines into kOrganParams[] and the
79adfa4d:src/console/organ_params.inc:// BLOCK is an ORGAN_BLOCK_* (organ_registry.hpp) and there is no way to
79adfa4d:tools/organ_ledger.py:# restated once, against the rules organ_registry.hpp states in C++, so
79adfa4d:tools/organ_ledger.py:# THE DOOR TABLE is parsed out of organ_registry.hpp's `kOrganDoors`
79adfa4d:tools/organ_ledger.py:REG = os.path.join(ROOT, "src", "console", "organ_registry.hpp")
79adfa4d:tools/organ_ledger.py:# ORGAN_DEFONLY_BLOCK_##DEFKIND in organ_registry.hpp; the convention
79adfa4d:tools/organ_ledger.py:    """organ_registry.hpp's `derived_cadence()`, restated once.
79adfa4d:tools/organ_ledger.py:    A("     and src/console/organ_registry.hpp — do not hand-edit.")
79adfa4d:tools/organ_ledger.py:    A("`organ_registry.hpp::derived_cadence()` and is restated once in this")
79adfa4d:tools/organ_readers.py:            if rel.endswith("organ_registry.hpp") or rel.endswith("organ_params.inc"):
```

Exactly one of the twelve is an `#include` directive, and it names the
`console/` copy. **No line anywhere in `src` or `tools` includes
`"coupling/organ_registry.hpp"`** (`git grep 'include "coupling/organ_registry.hpp"' 79adfa4d -- .`
→ no output, non-zero exit). The two `tools/` scripts that resolve the header
by path — `organ_ledger.py` via its `REG` module constant, and
`organ_readers.py` via a `rel.endswith(...)` test — both point at
`src/console/organ_registry.hpp`.

The cartridge includes the **`console/`** copy; the ledger tool reads the
**`console/`** copy; the **`coupling/`** copy is named by nothing. The two
files are **not identical** — `diff src/console/organ_registry.hpp
src/coupling/organ_registry.hpp` emits 249 lines of differences, and their
blobs and sizes differ:

| path | blob SHA at `79adfa4d` | size (B) |
| --- | --- | ---: |
| `src/console/organ_registry.hpp` | `70d09e9602eb0f763a616da5303e14c34e7f44da` | 49377 |
| `src/coupling/organ_registry.hpp` | `3047070e199df57c2a7cd6d8f75cf028ec48b817` | 50109 |

What they do share, cited by enclosing symbol: each file's **top-level
contracts-include group** — the run of `#include "cartridges/the_board/contracts/…"`
rows before the `<c…>` standard-library block — carries one identical
non-contracts row in the same position, between the `mood_constants.hpp` row
and the `driver_surface.hpp` row that closes the group
(`grep -n '^#include "' <file>` on each, verified at amendment):

```cpp
#include "coupling/canvas_surface.hpp"                        // CANVAS_LIVE (block 9, t7::canvas)
```

Recorded as an observed duplicate. No repair is proposed here.

**FINDING — `src/sources/` is on the include path and compiled into
nothing.** `git grep -n 'include "sources/<h>.hpp"' 79adfa4d -- .` per header
gives, exhaustively (the broader form
`git grep -nE '#include "[^"]*<h>\.hpp"' 79adfa4d -- .` returns the same rows,
so there is no relative-path variant hiding anywhere):

| header | includers | who |
| --- | ---: | --- |
| `sources/keyboard_midi.hpp` | **0** | **nothing in the tree includes it** |
| `sources/midi_event.hpp` | 9 | `src/analysis/canvas_1/canvas.hpp`, `src/analysis/canvas_1/check_canvas_compound.cpp`, `src/analysis/canvas_1/check_canvas_union.cpp`, `src/analysis/canvas_1/probe_canvas.cpp`, **`src/musical/context.hpp`**, **`src/musical/midi_stream.hpp`**, `src/sources/keyboard_midi.hpp`, `src/sources/midi_file.hpp`, `src/sources/midi_port.hpp` |
| `sources/midi_file.hpp` | **0** | **nothing in the tree includes it** |
| `sources/midi_port.hpp` | 1 | `src/analysis/canvas_1/canvas.hpp` |
| `sources/transport.hpp` | 1 | `src/sources/midi_port.hpp` |

**AMENDED — the earlier draft's shape claim was wrong for three of the five.**
It said all five "are reached only from `src/analysis/canvas_1/canvas.hpp` and
the five unbuilt `canvas_1/*.cpp` probes". In fact `keyboard_midi.hpp` and
`midi_file.hpp` have **no includer at all**, anywhere in the tree; and
`midi_event.hpp` is additionally pulled by two files outside both `canvas_1/`
and `sources/` — `src/musical/context.hpp` and `src/musical/midi_stream.hpp`.
Only `midi_port.hpp` (from `canvas_1/canvas.hpp`) and `transport.hpp` (from
`sources/midi_port.hpp`) fit the shape as originally stated.

**The subtree-level verdict is unchanged and independently reproduced: 0 of the
5 `sources/` headers is reached from `src/the_board.cpp`** (`reach.py` prints no
`[sources]` group at all). The two `src/musical/` files that pull
`midi_event.hpp` are themselves outside the compiled closure — of the 16
`src/musical/` headers, only `signal_layout.hpp` is reached from
`the_board.cpp`, and it is the only one of the 16 with any includer outside
`src/musical/` and `src/analysis/canvas_1/` (recipe: per-header
`git grep -l 'include "musical/<h>.hpp"' 79adfa4d -- src`, then
`grep -vE ':src/(musical|analysis/canvas_1)/'` — `signal_layout.hpp` returns
`src/coupling/visual_canvas.hpp`; the other fifteen return nothing).

#### 8b.6 `src/external/RtMidi.cpp` — compiled, and its guard

**YES, `src/external/RtMidi.cpp` IS compiled into the one target
`the_board`.** It is the second entry of `add_executable(the_board …)`,
quoted verbatim in 8b.3, with the comment that names its role:

```cmake
    # The one other translation unit: RtMidi's Windows MM backend, the
    # canvas's route to the DAW's virtual port. Vendored, not header-only,
    # so it compiles separately; __WINDOWS_MM__ below selects its backend.
    src/external/RtMidi.cpp
```

**The backend define is `__WINDOWS_MM__`, and it is the ONLY one set.**
Recipe:

```
grep -nE '__WINDOWS_MM__|__LINUX_ALSA__|__MACOSX_CORE__|__UNIX_JACK__|__RTMIDI_DUMMY__' CMakeLists.txt
```

Three hits, all `__WINDOWS_MM__` (two comments, one definition):

```
591:# __WINDOWS_MM__ selects RtMidi's Windows Multimedia backend. RtMidi
596:    __WINDOWS_MM__)
696:    # so it compiles separately; __WINDOWS_MM__ below selects its backend.
```

**`__LINUX_ALSA__`, `__MACOSX_CORE__`, `__UNIX_JACK__` and `__RTMIDI_DUMMY__`
appear ZERO times in `CMakeLists.txt`.** The rows, verbatim (enclosing
symbol: the `set(MSVC_COMPILE_DEFS …)` statement in the shared-MSVC-options
block):

```cmake
# __WINDOWS_MM__ selects RtMidi's Windows Multimedia backend. RtMidi
# compiles to an empty TU without a backend define, and the link then
# fails on RtMidiIn's constructor rather than on anything that names it.
# winmm.lib is already carried at LEVEL 9 of the link list above.
set(MSVC_COMPILE_DEFS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS
    __WINDOWS_MM__)
```

and the site that attaches it — **inside an `if(MSVC)` guard, so the define
reaches the compiler on the MSVC lane and on no other**:

```cmake
if(MSVC)
    target_compile_options(the_board PRIVATE ${MSVC_COMPILE_OPTS})
    target_link_options(the_board PRIVATE ${MSVC_LINK_OPTS})
    target_compile_definitions(the_board PRIVATE ${MSVC_COMPILE_DEFS})
endif()
```

The comment states the consequence of no backend define in the file's own
words: "RtMidi compiles to an empty TU without a backend define, and the link
then fails on RtMidiIn's constructor rather than on anything that names it."
Recorded as fact: on a non-MSVC compiler this build sets no RtMidi backend.

Who names RtMidi in the tree (`git grep -ln 'RtMidi' 79adfa4d -- src`):
`src/external/RtMidi.cpp`, `src/external/RtMidi.h`, `src/sources/midi_port.hpp`,
`src/sources/transport.hpp`, and two of the unbuilt probes
(`src/analysis/canvas_1/check_pc_dft.cpp`, `src/analysis/canvas_1/probe_canvas.cpp`).
None of these is in the compiled closure of `src/the_board.cpp` (8b.5), so
at `79adfa4d` `RtMidi.cpp` is compiled but its symbols are referenced by no
other compiled TU.

The other `target_compile_definitions` row on `the_board` (unconditional),
verbatim, for completeness of the define surface:

```cmake
# THE PROGRAM'S DIALS, RE-HOMED AT WEB_SUNSET. They were the program's on
# both twins and yet lived only in the web branch, so on native they reached
# nothing at all — `the-board-full-release-meter` set T7_INSTRUMENTS=meter and
# the compiler never heard it. Pre-existing defect, found by the W-recon,
# repaired here on the way out rather than deleted along with its only home.
target_compile_definitions(the_board PRIVATE
    INCUBATE_RENDER=${T7_RENDER_CARTRIDGE}
    INCUBATE_DEMO=${THE_BOARD_DEMO}
    T7_INSTRUMENTS=${T7_INSTRUMENTS}
    ${T7_WORLD_SEED_DEFS}   # empty ⇒ nothing pasted ⇒ the seed is drawn
)
```

#### 8b.7 `kCompilerPlan` — current value

Enclosing symbol: **namespace `t7`** (the only namespace opened before the
declaration — `grep -nE '^\s*namespace ' src/console/console.hpp` returns one
row above it, `namespace t7 {`), in the block headed
`═══ THE COMPILER PLAN (PIVOT_0, 2026-08-12) ═══`, in
`src/console/console.hpp` (blob `577c486049956d7c977580351b764967e3cb3d6c`).
Recipe: `grep -n 'kCompilerPlan' src/console/console.hpp` then read the
enclosing block.

**Current value: `CompilerPlan::Vulkan`.** Declaration verbatim, with the
adjacent comment that states its meaning, and the formatter that follows it:

```cpp
    // ═══ THE COMPILER PLAN (PIVOT_0, 2026-08-12) ═════════════════════
    //
    // PIVOT_0 — the shader-compiler plan. world.wgsl is single-source
    // across all values.
    //
    // Why it exists: WALLET_0's occupier cbuffer arrays stalled
    // update_player_agent at 20,227 ms under FXC and then
    // D3DCompiler_47 access-violated on the next room kernel. Jean
    // ruled the floor up rather than the shader down. The audience
    // floor is WebGPU core through modern compilers — Tint→DXC
    // (SM6.0+), Tint→MSL, Tint→SPIR-V, naga.
    //
    // D3D12_Fxc exists for ARCHAEOLOGY ONLY. It reproduces the retired
    // gate so a historical result can be re-run; it is not a supported
    // floor and nothing should be shaped to satisfy it. The laws it
    // used to impose are in docs/FXC_LAWS_RECORD.md.
    //
    // Plan B is one line: if DXC fails on a given driver, set this to
    // Vulkan, rebuild, boot. That IS the fallback, not a failure.
    enum class CompilerPlan { D3D12_Dxc, Vulkan, D3D12_Fxc };
    inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan
        
        ;

    inline constexpr const char* compiler_plan_name(CompilerPlan p) {
        switch (p) {
        case CompilerPlan::D3D12_Dxc: return "DXC";
        case CompilerPlan::Vulkan:    return "VULKAN";
        case CompilerPlan::D3D12_Fxc: return "FXC";
        }
        return "?";
    }
```

(The phrase "nothing should be shaped to satisfy it" above is the source
comment's own wording, not a recommendation by this section.)

**AMENDED RECIPE.** An earlier draft published `sed -n '105,110p'
src/console/console.hpp | cat -A` as the recipe for the byte-exact block below.
That range emits **six** lines — a leading
`    // Vulkan, rebuild, boot. That IS the fallback, not a failure.$` and a
trailing blank `$` around the four shown. The recipe that produces exactly the
four displayed lines is `sed -n '106,109p' src/console/console.hpp | cat -A`
(`$` rendered here as `[LF]`):

```
    enum class CompilerPlan { D3D12_Dxc, Vulkan, D3D12_Fxc };[LF]
    inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan[LF]
        [LF]
        ;[LF]
```

Recorded as observed: the initializer is followed by a line of **eight
spaces** and then the semicolon on its own line — the value sits on the
`= CompilerPlan::Vulkan` line, with the statement terminated two lines below.
LF endings, no CR. (The line numbers in this recipe are a shell argument, not a
citation; the claim is anchored to the declaration
`inline constexpr CompilerPlan kCompilerPlan` inside `namespace t7`.)

`git log -1 -S'kCompilerPlan = CompilerPlan::Vulkan' -- src/console/console.hpp`
names `f5d4180e` (2026-08-12, "PIVOT_0d-ii: toggles chain to instance
descriptor; device-level chain retired") as the commit that put `Vulkan` on
that line — i.e. the plan has stood at Vulkan since PIVOT_0d, which is
**before** both sunsets and before the fork point `de4b8b6f`.

**AMENDED — three consumers, enumerated as three.** An earlier draft announced
"three consumers" and then listed four items, counting
`compiler_plan_name(CompilerPlan)` among them.
`grep -n 'kCompilerPlan' src/console/console.hpp` returns five rows: the
declaration, three code references, and one comment mention.
`compiler_plan_name` takes the plan as a *parameter* and does not name
`kCompilerPlan` at all, so it is a formatter reached by the third consumer, not
a consumer itself. The three code consumers, cited by enclosing symbol — all
three sit inside the member function **`t7::Console::initWebGPU()`** (the
enclosing method of `class Console`; the next method declared after it is
`initSurface()`):

**Consumer 1** — arms the DXC toggle in the instance/toggles setup. Verbatim,
whole lines:

```cpp
            if constexpr (kCompilerPlan == CompilerPlan::D3D12_Dxc) {
                toggles.enabledToggleCount = 1;
                toggles.enabledToggles = kDxcToggle;
            }
```

**Consumer 2** — the backend preference in the adapter-request path. Verbatim,
whole lines (the continuation lines are column-aligned in the source and are
reproduced at their source indentation):

```cpp
            constexpr wgpu::BackendType kPreferredBackend =
                (kCompilerPlan == CompilerPlan::Vulkan) ? wgpu::BackendType::Vulkan
                                                        : wgpu::BackendType::D3D12;
```

**Consumer 3** — the boot log line, and the only site that calls
`compiler_plan_name` on the constant. Verbatim, whole lines:

```cpp
            std::cout << "[Console] Compiler plan (request): "
                << compiler_plan_name(kCompilerPlan) << "\n";
```

The fifth `grep` row is a comment, not a consumer, and it states the constant's
force in the adapter path. Verbatim, whole lines:

```cpp
            // adapters, and picks by the scorer below; kCompilerPlan's
            // backend preference is a tie-break, not a guarantee — the
            // effect witnesses after CreateDevice report what was actually
            // picked and enabled. Toggles ride the instance descriptor:
            // instance -> adapter -> device inheritance
            // (dawn/native Instance.cpp, Adapter.cpp).
```

**AMENDED — three non-C++ mentions, not one.** An earlier draft named only
`world.wgsl`. Recipe (no `-n`, so the output below is the command's own text
with no line numbers):
`git grep 'kCompilerPlan' 79adfa4d -- . | grep -v 'src/console/console.hpp'`
returns three rows:

```
79adfa4d:docs/LAWS.md:**unsupported**. The native compiler is one constant, `kCompilerPlan` in
79adfa4d:docs/OPEN.md:| lane | Vulkan (`kCompilerPlan = CompilerPlan::Vulkan`); D3D12 stays linked and reachable via the adapter scorer |
79adfa4d:src/cartridges/the_board/realization/world.wgsl:// The compiler of record is Dawn/Tint on the lane kCompilerPlan
```

`docs/LAWS.md` names the constant as the single point of truth for the native
compiler (whole lines, from the paragraph headed "**What replaced it.**"):

```
**What replaced it.** The audience floor is WebGPU core through modern
compilers — Tint→DXC (SM6.0+), Tint→MSL, Tint→SPIR-V, naga. FXC is
**unsupported**. The native compiler is one constant, `kCompilerPlan` in
`src/console/console.hpp`; the shader's live statement of the floor is the
COMPILER FLOOR block in the `world.wgsl` banner, which is where L2's
operational home used to be.
```

and `docs/OPEN.md` records the value in its Dawn-pin table, agreeing with the
header:

```
| lane | Vulkan (`kCompilerPlan = CompilerPlan::Vulkan`); D3D12 stays linked and reachable via the adapter scorer |
```

So the documentation of record and the source agree on the value at
`79adfa4d`: **Vulkan**.

---

### 8c — Flags

**FLAG — HEAD moved between authoring and amendment.** The section is anchored
at `79adfa4d`; `git rev-parse HEAD` now returns `6d53388e`, which adds
`docs/LIGATURE_0_RECON.md` (521224 B) and nothing else
(`git diff --name-status 79adfa4d 6d53388e` → one `A` row). Three HEAD-scoped
statements in 8a change value across that drift and are tabulated at the top of
this section; every one of them is re-stated here scoped to `79adfa4d` and
verified against that revision. The working tree is clean at both commits
(`git status --porcelain` → empty). Which of the two commits the plan reads
against is undetermined here: this section records both states rather than
choosing between them. Resolving it costs nothing further to read — the
`git diff --name-status` above is the whole delta.

**FLAG — `the-board-vs` has no build preset.** `CLAUDE.md` presents
`the-board-vs` as a lane one builds with `--config Debug`; `CMakePresets.json`
carries `the-board-vs` only under `configurePresets`, with the build side
split into `the-board-vs-release` and `the-board-vs-debug`. This is a
documentation/file mismatch of *shape*, not of capability — `cmake --build
--preset the-board-vs-debug` and `cmake --build <dir> --config Debug` both
reach the same thing. Resolving whether `CLAUDE.md`'s phrasing is intended
would need Jean's word; it costs nothing further to read.

**FLAG — could not verify any build behavior.** The unit forbids running
cmake/ninja/compilers, so every claim in 8b is **static reading of
`CMakeLists.txt` / `CMakePresets.json` / the include graph**. Specifically
unverified: whether `${DAWN_INCLUDES}` / `${DAWN_LIBS}` resolve on this
machine (they point at `C:/dev/dawn`, a Windows path, out of tree); whether
`the_board` links at all here; whether `T7_STAMP_DIR` is produced; whether the
`if(MSVC)` branch that carries `__WINDOWS_MM__` is ever taken on any lane
configured from this repo. Resolving would cost a configure+build against a
pinned Dawn checkout that this container does not have.

**FLAG — include-closure is a quoted-include walk, not a preprocessor run.**
The 8b.5 reachability table follows `#include "…"` only; it does not evaluate
`#if` / `#ifdef`, does not follow angle-bracket includes, and resolves the
macro-built include `RENDER_HEADER(INCUBATE_RENDER)` only because
`src/the_board.cpp` also carries the literal `#include
"cartridges/the_board/cartridge.hpp"` under `#if defined(__INTELLISENSE__)`.
A conditional include guarded by a macro this walk does not evaluate would be
missed. Resolving exactly would cost a `-E` preprocessor run, which the build
prohibition forbids. The script is at
`/tmp/claude-0/-home-user-7T-Music/e4d04ccd-accf-5afe-8420-1b448d1fd519/scratchpad/reach.py`
and was re-run at amendment with byte-identical output.

**FLAG — deleted-doc census is deletion-event based, not content based.** The
recipe in 8a.4 finds files whose *path* was deleted and whose *name* matches
the topic regex. A coupling-design document whose filename carries none of
`coupling|chord|ligature|canvas|organ|signal` (say, `PATCH_BAY.md` or
`SS-3_REPORT.md`) would not surface. The 8a.5 sweep outside `docs/` has the
same blind spot for the same reason. A content-based sweep would require
`git rev-list --all` × `git grep` over every historical blob — a full-history
content scan across 2228 commits — which was not run. The **146** unique
deleted doc paths are enumerated in
`/tmp/claude-0/-home-user-7T-Music/e4d04ccd-accf-5afe-8420-1b448d1fd519/scratchpad/deleted_docs.txt`
(regenerated at amendment) if a wider read is wanted.

**FLAG — the topic regex conflates directory names with document subjects.**
Two of the four prose hits in 8a.5
(`src/cartridges/the_chord/pattern_glossary_v1.6.md`,
`src/cartridges/the_chord/rollout_open_questions.md`) match only because the
string `chord` appears in their *parent directory*, `the_chord`. Neither
carries a `\bpipes?\b` or `\bstats?\b` hit. Distinguishing directory matches
from filename matches would need a second regex pass scoped to `basename`;
the four prose hits were sorted by hand instead, and the two cartridge
documents were read only far enough (first six lines each,
`git show '464f2ca0^:<path>' | head -6`) to classify them. Reading them whole
would cost 150 KB of transcript.

**FLAG — sizes of deleted files are taken at `<sha>^`, the deleting commit's
parent.** If a file was modified between refs on different branches, another
ref could hold a different final size. Every one of the six in 8a.4 had exactly
one `A` and one `D` across all refs (verified with `--name-status`), so no such
divergence exists for these six. The same check was **not** run for the four
prose hits in 8a.5; their sizes are `<sha>^` values only.

**FLAG — an earlier draft of this section shipped counts that did not
reproduce from its own recipes.** Recorded so the plan knows which numbers were
corrected rather than merely restated: `143` unique deleted doc paths → **146**;
`47` topic hits outside `docs/` → **46**; the `COUPLING_SAGA_FINISHER` term
census `0`/`0` → **1**/**3**; `two` prose hits outside `docs/` → **four**;
`all five` canvas_1 probes carried by generated `.vcxproj` files → **four of
five** (`check_pc_dft` never had one); `the only glob` in `CMakeLists.txt` →
**two globs**; `three consumers` of `kCompilerPlan` listed as four items →
**three**, with `compiler_plan_name` reclassified as a formatter reached by the
third; `one` non-C++ mention of `kCompilerPlan` → **three**
(`world.wgsl`, `docs/LAWS.md`, `docs/OPEN.md`); and the five `sources/`
headers' includer shape corrected (two have **no includer at all**;
`midi_event.hpp` has nine, two of them in `src/musical/`). Every corrected
number above was re-derived at amendment from the recipe printed beside it.

## 9. Flags

Every item the campaign asked for that was not delivered as asked, delivered
against a premise the tree contradicted, or delivered with a stated limit. A unit
that failed silently would be worse than one that failed loudly, so each entry
names the failure mode and what resolving it would have cost.

### F-1 — The deliverable is committed to the campaign branch, not `master`

**What the handoff asked.** `docs/LIGATURE_0_RECON.md`, committed on `master`.

**What happened.** This session's operating constitution designates
`claude/ligature-0-recon-hcrix0` as the development branch and forbids pushing to
any other branch without explicit permission. The two instructions conflict, and
the branch constraint is the one this session cannot set aside on its own
authority. The report is therefore committed to
`claude/ligature-0-recon-hcrix0`.

**Cost of the difference.** One merge or fast-forward onto `master`, Jean's gate.
Nothing in the report's content depends on which branch carries it.

### F-2 — The handoff's sunset boundary names the wrong sunset

**What the handoff asked.** `POST = native-sunset`, and `PRE` = the last
substantive commit touching `src/coupling/` before "the sunset sequence".

**What the tree says.** There are two sunsets, and `native-sunset` is the earlier
one, running in the opposite direction from this campaign. Its own annotation
reads *"Native twin archived; the web twin is the program (SUNSET_0)"* — it
archives the NATIVE arm. The sunset this repository's constitution describes is
`web-sunset`, *"Web twin archived; the native twin is the program (WEB_SUNSET)"*.
Across the handoff's literal `PRE..POST` the four `src/coupling/` headers change
by exactly one line each (a trailing-newline commit), and nothing under
`src/coupling/` is deleted at all.

**How it was handled.** §1 records three boundaries rather than one: **A** the
handoff-literal boundary, **B** the commit that actually severed the musical arm
(`1a52f2db`, *"CUT_1c: MIDI intake retired; BeatClock … feeds the signal spine"*,
2026-08-05), and **C** the restoration (`web-sunset..HEAD`, where
`src/musical/`, `src/sources/` and `src/analysis/canvas_1/` were **added** by
*"bringing back the music"*, 2026-08-30). Boundary A is reported in full as
asked, and reported as empty, because that is what it is.

**Cost of the correction.** None to the campaign; §1 is larger than the unit
specified, not smaller. The consequence for the plan is decisive: the ligature is
not a resurrection problem at all. Of the 32 files under `src/musical/`,
`src/sources/`, `src/analysis/` and the two `RtMidi` files at HEAD, **30 carry a
blob byte-identical to their blob at `1a52f2db^`**; one (`beat_clock.hpp`) has no
pre-`CUT_1c` counterpart because `CUT_1c` created it; and one
(`signal_layout.hpp`) diverged but was never deleted, so it is not part of the
restored set. **Zero restored files diverged.**

**Recipe.** For each path in
`git ls-tree -r --name-only HEAD -- src/musical src/sources src/analysis src/external/RtMidi.cpp src/external/RtMidi.h`,
test `git cat-file -e '1a52f2db^:<path>'` for existence, then compare
`git rev-parse '1a52f2db^:<path>'` against `git rev-parse HEAD:<path>`. Counts:
IDENTICAL=30, DIVERGED=1, NEW=1. Note that `git rev-parse` echoes its argument on
a missing path instead of failing, so the existence test must be `cat-file -e`;
using `rev-parse` alone misreports `beat_clock.hpp` as diverged.

### F-3 — U7's `docs/CHORD.md` recovery step is moot and was NOT performed

**What the handoff asked.** `git log --all -- docs/CHORD.md`; if it ever existed,
`git show <sha>:docs/CHORD.md` saved to `docs/CHORD.md.recovered`, and the same
for `docs/7t_program_theory_v3.md`.

**What the tree says.** Both files exist at HEAD (`docs/CHORD.md`, blob
`83b62a173e92280791693490bb81b7c100e0e107`; `docs/7t_program_theory_v3.md`, blob
`bf36d995c9503fff4c3d42ae89874a60d23c8c9e`). There is nothing to recover.

**How it was handled.** No `.recovered` file was written — writing one would have
violated R1, which permits exactly one write, this report. §8 lists both files,
their histories, and quotes the passages bearing on the coupling instead. §8 also
records the separate finding that `docs/CHORD.md` is **not** a coupling-design
document: it is the GPU uniform-seat redistricting record (blocks, cadences,
binding ceilings). The handoff's expectation that it would describe the pipe/stat
system is not met by its content.

### F-4 — The pipe table's verdict vocabulary has no cell for the observed state

**What the handoff asked.** One verdict per pipe from a fixed six-value set:
`LIVE` / `DECLARED-UNRESOLVED` / `DECLARED-UNFLUSHED` / `ORPHAN-SINK` /
`ORPHAN-SOURCE` / `GONE`.

**What the tree says.** All eight pipes are in one state the vocabulary does not
name: the pipe is declared, its coupling is declared, it **is** resolved in the
cartridge, it **is** flushed every frame, its setter runs, and its GPU field is
read by the shader — while its *source* stat resolves against an empty layout and
is therefore unbound. Every hop is present except the one that supplies the value.

**How it was handled.** §3.4 assigns `DECLARED-UNRESOLVED` to all eight rows and
marks each explicitly `(source half)`, with §3.5 giving the reproducing command
per verdict class and separate subsections stating why no `DECLARED-UNFLUSHED`,
`GONE` or `ORPHAN-SOURCE` row exists. The mapping is stated rather than assumed
so that a plan written from the table is not misled into thinking the target-side
resolve is missing. It is not; only the source is.

### F-5 — `STAT_LAYOUT` does not exist as a symbol

**What the handoff asked.** U4 enumerates registered stats from
`musical/signal_layout.hpp` and `analysis/analysis_signal.hpp`, and
`analysis_cartridge.hpp` documents a *"static `STAT_LAYOUT`"*.

**What the tree says.** No `STAT_LAYOUT` symbol exists anywhere in `src/`. The
layout is built at runtime by `Canvas1::publish_reading` into a member array and
published through `stat_layout()`. §4 records the actual mechanism and the 55
published names it produces.

**Cost.** None; §4 enumerates from the real producer. Recorded because a plan
that greps for `STAT_LAYOUT` will find nothing and should not conclude the layout
is absent.

### F-6 — No program was run, and no build was performed

Every reachability verdict in this report is **static**. The campaign forbids the
build (Jean holds that gate) and the unit specifies static reachability only.
Concretely, this limits three things:

* Preprocessor conditionals were read, not evaluated. §5 records this: an
  `#ifdef`-guarded call site is reported with its guard rather than resolved.
* `DEFINED-UNCALLED` in §2(B) is measured against the translation units CMake
  actually names. It is a statement about the build graph, not about a running
  process.
* No claim is made about runtime values. Where this report says a pipe holds its
  rest, that is derived from `SignalLayout::resolve`'s documented `valid=false`
  path and `BeatClock::stat_layout()`'s literal `{ nullptr, 0 }`, not observed.

**Cost of resolving.** One build and one run on Jean's machine, which would
convert the `[SignalLayout] N sources unbound` line that `VisualCanvas::bind`
already prints from a predicted 12 into a measured one.

### F-7 — Binary and generated files were counted by blob, not by line

§1(f) asks for the full diff of any file that lost more than half its lines
across a boundary. For the 60 deleted `assets/paintings/*.jpeg` and the vendored
`stb_image.h`, a line count is meaningless or enormous. §1 reports these by path,
deleting commit and blob, states the truncation explicitly, and gives the
`git show` command that would recover any of them. No file was silently dropped.

### F-8 — Report size

The assembled report is about 925 KB across roughly 17,100 lines and ten sections. It is one file, as
the handoff specified. §5 and §6 carry the bulk, because R5 required verbatim
quotation at every hop and the honest way to show a twelve-hop chain is to show
it. Recorded so the size is a known property rather than a surprise.

### F-9 — Two `organ_registry.hpp` at HEAD, and HEAD is what made them differ

Not a skipped item — a finding the handoff's framing did not anticipate, recorded
here because it bears on any plan that touches the organ. The blob lineage is
exact:

| commit | `src/console/organ_registry.hpp` | `src/coupling/organ_registry.hpp` |
| --- | --- | --- |
| `4cfc899b^` (pre-W3e) | `70d09e96` | absent |
| `4cfc899b` (WEB_SUNSET W3e, *"the organ keeps its ABI and loses its browser — prose only"*) | `3047070e` | absent |
| `72df32df` (*"Systems operating"*, 2026-08-30) | `3047070e` | `3047070e` — **created as a byte-identical copy** |
| `79adfa4d` (HEAD, *"Systems operational"*) | `70d09e96` — **reverted to the pre-W3e content** | `3047070e` |

**Recipe.** `git rev-parse <rev>:src/console/organ_registry.hpp` and
`git rev-parse <rev>:src/coupling/organ_registry.hpp` at each revision above;
`git cat-file -e` for the absence rows.

So the two files were identical when the second was created, and the divergence
visible at HEAD was introduced by HEAD itself, reverting the console copy across
W3e's prose change. The consequence: **the live file carries the older prose and
the dead file carries the newer.** `src/console/organ_registry.hpp` is the one
included (by `cartridge.hpp`, the tree's only `#include` of either) and the one
`tools/organ_ledger.py` hardcodes; `src/coupling/organ_registry.hpp` is named by
nothing at all — not the tree, not `CMakeLists.txt`, not the ledger tools, not
any untracked file. Full evidence and the forced verdict are in §7.

### F-10 — Not everything `CUT_1c` deleted came back

Recorded because §1's restoration finding could otherwise be over-read. `CUT_1c`
deleted 49 files; the restoration brought back the musical/analysis/sources arm
and `RtMidi`, but **not** `src/the_lab.cpp` (668 lines, blob `3cd1a13fe66e` at
`1a52f2db^`) and **not** the 18 vendored `src/external/imgui/` and
`src/external/implot/` files. `git cat-file -e HEAD:src/the_lab.cpp` fails.
Whether any of those belongs to this ligature is not a question this report
answers — it records only that they are gone and that the coupling arm proper is
not.

### F-11 — Items delivered in full, recorded here so their absence from this list is not read as an omission

Every other unit step ran to completion: U1's tag ledger, timeline and three
boundary diffs; U2's twelve-needle sweep and both boundary questions; U3's
eight-row pipe table, its verdict evidence and the dormant surface across all
three declaring files; U4's 55-name stat table, the wired-analyzer census, the
reader-less list and all five witnesses; U5's two slices at every hop; U6's
diff, header delta, include graph and forced verdict; U7's docs listing and full
build reading including `kCompilerPlan`. Each section additionally carries its
own local flags in its closing subsection, and those are not repeated here.

