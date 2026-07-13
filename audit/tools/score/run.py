#!/usr/bin/env python3
# ─── THE SCORE CENSUS (REBUILD-0 m2; the charter's standing tool) ──
#
# Machine-checked bijection roster <-> score, both directions, over
# the composition root (cartridge.hpp) plus the delegated gate sites.
#
# DIRECTION A (roster -> score): every roster piece has its conductor
#   call(s) present AND gated by its bit — each manifest entry is a
#   gate+call PATTERN matched structurally (comments may intervene;
#   block-form entries allow a brace-free block body). A missing or
#   ungated call fails loud.
# DIRECTION B (score -> roster): every free-function conductor call in
#   the score regions (initialize / init_renderer / update / render)
#   must be attributable — a manifest call or a FOUNDATIONAL/spine
#   whitelist entry with a one-line justification. An unknown call
#   fails loud: an ungated module tick is UNWRITABLE (this is how P1
#   and P2 stay dead).
#
# JURISDICTION: this tool certifies score structure (presence, gating,
# attribution). Behavior is the rig's; syntax is glaw1's. The tool
# parses the score text directly — it shares no extraction with any
# generator (the mirror law does not apply).

import re, sys, os

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..', '..'))
TB   = os.path.join(ROOT, 'src', 'cartridges', 'the_board')

def read(rel):
    with open(os.path.join(TB, rel), 'rb') as f:
        return f.read().decode('utf-8')

CART = read('cartridge.hpp')

# comments (line or block) may sit between a gate and its call
C = r'(?:\s|//[^\n]*\n|/\*.*?\*/)*'

def imm(bit_expr, call):
    """gate applied to the immediately-following statement"""
    return rf'if constexpr \({bit_expr}\){C}{call}\('

def blk(bit_expr, *calls):
    """gate applied to a brace-free block body containing the calls in order"""
    inner = r'[^}}]*'.join(re.escape(c) + r'\(' for c in calls)
    return rf'if constexpr \({bit_expr}\)\s*(?:\{{|if \([^\n]*\{{)[^}}]*{inner}'

# ── DIRECTION A: the manifest ─────────────────────────────────────
# piece -> list of (file, human site name, regex)
MANIFEST = {
    'pyramid':  [('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.pyramid',  r'[^;]*?\.prepare_mesh'))],
    'arch':     [('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.arch',     r'[^;]*?\.prepare_mesh'))],
    'column':   [('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.column',   r'[^;]*?\.prepare_mesh'))],
    'antenna':  [('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.antenna',  r'[^;]*?\.prepare_mesh'))],
    'palm':     [('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.palm',     r'[^;]*?\.prepare_mesh'))],
    'cactus':   [('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.cactus',   r'[^;]*?\.prepare_mesh'))],
    'blade':    [('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.blade',    r'[^;]*?\.prepare_mesh'))],
    'sphere':   [('cartridge.hpp', 'mirror',     imm(r'ROSTER\.sphere',   r'reconcile_sphere_mirror')),
                 ('cartridge.hpp', 'teardown',   imm(r'ROSTER\.sphere',   r'clear_spheres')),
                 ('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.sphere',   r'[^;]*?\.prepare_mesh'))],
    'ribbon':   [('cartridge.hpp', 'frame tick', imm(r'ROSTER\.ribbon',   r'ribbon_frame_tick')),
                 ('cartridge.hpp', 'teardown',   imm(r'ROSTER\.ribbon',   r'teardown_ribbon')),
                 ('cartridge.hpp', 'finite rel', imm(r'ROSTER\.ribbon',   r'release_finite_ribbons')),
                 ('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.ribbon',   r'[^;]*?\.prepare_mesh')),
                 ('direction/input.inl', 'F8 door (D9)', imm(r'ROSTER\.ribbon', r'toggle_sky_mode'))],
    'cube':     [('cartridge.hpp', 'corral',     imm(r'ROSTER\.cube',     r'tick_cube_corral_animations')),
                 ('cartridge.hpp', 'mirror',     imm(r'ROSTER\.cube',     r'reconcile_cube_mirror')),
                 ('cartridge.hpp', 'teardown',   imm(r'ROSTER\.cube',     r'clear_cubes')),
                 ('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.cube',     r'[^;]*?\.prepare_mesh'))],
    'gol':      [('cartridge.hpp', 'zone block (D7)', blk(r'ROSTER\.gol',
                     'flush_zone_derive_requests', 'upload_gol_zone_config',
                     'dispatch_zone_sync', 'dispatch_zone_evolve', 'dispatch_zone_mesh')),
                 ('cartridge.hpp', 'teardown',   imm(r'ROSTER\.gol',      r'teardown_gol')),
                 ('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.gol',      r'[^;]*?\.prepare_mesh'))],
    'gallery':  [('cartridge.hpp', 'boot textures (P2 dead)', blk(r'ROSTER\.gallery', 'load_authored_textures')),
                 ('cartridge.hpp', 'photographer (P1 dead)',  imm(r'ROSTER\.gallery', r'update_photographer')),
                 ('cartridge.hpp', 'teardown (shared organ)', imm(r'ROSTER\.gallery \|\| ROSTER\.indoor_shell', r'teardown_gallery')),
                 ('cartridge.hpp', 'promotion drain',         imm(r'ROSTER\.gallery \|\| ROSTER\.indoor_shell', r'drain_gallery_promotions')),
                 ('cartridge.hpp', 'mesh prep',  imm(r'ROSTER\.gallery',  r'[^;]*?\.prepare_mesh'))],
    'pawn_aura':[('cartridge.hpp', 'couplings tick', imm(r'ROSTER\.pawn_aura', r'tick_pawn_couplings')),
                 ('cartridge.hpp', 'aura compute',   imm(r'ROSTER\.pawn_aura', r'dispatch_pawn_aura')),
                 ('cartridge.hpp', 'teardown',       imm(r'ROSTER\.pawn_aura', r'teardown_pawn_aura'))],
    'orbs':     [('cartridge.hpp', 'boot config',  blk(r'ROSTER\.orbs', 'configure_orbs')),
                 ('cartridge.hpp', 'anchor',       imm(r'ROSTER\.orbs', r'update_orb_anchor')),
                 ('cartridge.hpp', 'compute chain', blk(r'ROSTER\.orbs',
                     'dispatch_orb_init', 'dispatch_orb_recolor',
                     'dispatch_orb_copy_prev', 'dispatch_orb_dynamics')),
                 ('cartridge.hpp', 'teardown',     imm(r'ROSTER\.orbs', r'teardown_orbs'))],
    # DELEGATED pieces: the gate lives at the module door (cited), not
    # in the score — the score has no per-frame site to gate.
    'spot_lights': [('direction/mood.inl', 'apply door', rf'if constexpr \(ROSTER\.spot_lights\)')],
    'indoor_shell':[('direction/mood.inl', 'apply door', rf'if constexpr \(ROSTER\.indoor_shell\)')],
    'portal':      [('bodies/entities.inl', 'force-spawn door', rf'if constexpr \(!ROSTER\.portal\)')],
    'transitions': [('cartridge.hpp', 'portal-trigger door #2', rf'if constexpr \(ROSTER\.transitions\){C}if \(player_\.readback_portal_trigger'),
                    ('direction/mood.inl', 'request door #1', rf'if constexpr \(!ROSTER\.transitions\)')],
    'wanderers':   [('cartridge.hpp', 'boot population', imm(r'ROSTER\.wanderers', r'spawn_population_for_mood')),
                    ('cartridge.hpp', 'respawn',         imm(r'ROSTER\.wanderers', r'respawn_evicted_agents'))],
}

# ── DIRECTION B: the attribution sets ─────────────────────────────
MANIFEST_CALLS = {
    'reconcile_sphere_mirror', 'reconcile_cube_mirror', 'clear_spheres', 'clear_cubes',
    'ribbon_frame_tick', 'teardown_ribbon', 'release_finite_ribbons',
    'tick_cube_corral_animations', 'teardown_gol', 'flush_zone_derive_requests',
    'upload_gol_zone_config', 'dispatch_zone_sync', 'dispatch_zone_evolve', 'dispatch_zone_mesh',
    'load_authored_textures', 'update_photographer', 'teardown_gallery', 'drain_gallery_promotions',
    'tick_pawn_couplings', 'dispatch_pawn_aura', 'teardown_pawn_aura',
    'configure_orbs', 'update_orb_anchor', 'dispatch_orb_init', 'dispatch_orb_recolor',
    'dispatch_orb_copy_prev', 'dispatch_orb_dynamics', 'teardown_orbs',
    'spawn_population_for_mood', 'respawn_evicted_agents',
}

FOUNDATIONAL = {
    # call -> one-line justification (the whitelist is part of the census record)
    'init_patch_system':            'S2 boot — the surface is the stage (S1/S2 foundational)',
    'setup_test_rig_piers':         'S2 boot — test-rig piers, surface-owned',
    'upload_agent_registries_to_gpu': 'the agent buffer hosts the unconditional pawn (slot 0)',
    'seed_player_body':             'the player body is unconditional (boot twin)',
    'reseed_player_body':           'the player body is unconditional (transition twin)',
    'dump_agent_census':            'spine diagnostics (DIAG-unwrapped, constitution §5)',
    'dump_entity_census':           'spine diagnostics (#ifdef DIAG_ENTITY_CENSUS)',
    'teardown_surface':             'the surface core of the TEARDOWN movement (D4)',
    'teardown_entities':            'seven grounded families, one organ — per-family gating buys nothing (disclosed, D4)',
    'apply_mood':                   'mood owns no bit; the atmosphere is foundational (K4)',
    'stream_patches':               'S2 — the streaming conductor (carries SEAM[patch:spawn-trigger])',
    'upload_portal_array':          'flag-consumer at owner tick (portals_dirty); count-guarded',
    'upload_lights':                'light array upload; count=0 disables (mood-authored)',
    'dispatch_compute':             'REALIZATION — the frame compute dispatch',
    'upload_ground_entries':        'flag-driven realization (ground_entries_dirty cascade, O-4)',
    'dispatch_placement_correction': 'flag-driven realization (placement_dirty, O-4)',
    'dispatch_frustum_cull':        'REALIZATION pass (piece-gating lives at gate a-prime)',
    'render_shadow_pass':           'REALIZATION pass (piece-gating lives at gate a-prime)',
    'render_main_pass':             'REALIZATION pass (piece-gating lives at gate a-prime)',
    'render_snapshot_pass':         'REALIZATION pass; internally inert without a pending snapshot',
    'clear_input_deltas':           'driver bookkeeping, dead-last (O-5e)',
    'mark':                         'boot ROSTER report lambda',
}

KEYWORDS = {'if', 'for', 'while', 'switch', 'sizeof', 'return', 'defined', 'assert', 'constexpr',
            'static_cast', 'reinterpret_cast', 'const_cast', 'min', 'max', 'abs',
            'memset', 'memcpy', 'floor', 'sqrt', 'exp', 'atan2', 'cos', 'sin',
            'to_string', 'duration_cast', 'now', 'fprintf'}

def extract_score_regions(text):
    """bodies of initialize / init_renderer / update / render"""
    regions = []
    for sig in (r'void initialize\(wgpu::Device device\) override \{',
                r'bool init_renderer\(', r'void update\(const AnalysisSignal& signal',
                r'void render\(wgpu::CommandEncoder& encoder'):
        m = re.search(sig, text)
        assert m, f'score region not found: {sig}'
        i = text.index('{', m.start())
        depth, j = 0, i
        while True:
            if text[j] == '{': depth += 1
            elif text[j] == '}': depth -= 1
            if depth == 0: break
            j += 1
        regions.append(text[i:j+1])
    return regions

def strip_comments_strings(s):
    s = re.sub(r'/\*.*?\*/', ' ', s, flags=re.S)
    s = re.sub(r'//[^\n]*', ' ', s)
    s = re.sub(r'"(?:[^"\\]|\\.)*"', '""', s)
    return s

def main():
    failures = []

    # Direction A
    print('── DIRECTION A: roster -> score ──')
    for piece, entries in MANIFEST.items():
        for (rel, name, pat) in entries:
            text = CART if rel == 'cartridge.hpp' else read(rel)
            if re.search(pat, text, flags=re.S):
                print(f'  {piece:13s} {name:26s} [{rel}]  OK')
            else:
                failures.append(f'A: {piece} — {name} [{rel}] MISSING or UNGATED')
                print(f'  {piece:13s} {name:26s} [{rel}]  ** FAIL **')

    # Direction B
    print('── DIRECTION B: score -> roster (free-call attribution) ──')
    unknown = set()
    for region in extract_score_regions(CART):
        clean = strip_comments_strings(region)
        for m in re.finditer(r'(?<![\w.>:])([a-z_][a-zA-Z0-9_]*)\s*\(', clean):
            name = m.group(1)
            if name in KEYWORDS or name in MANIFEST_CALLS or name in FOUNDATIONAL:
                continue
            unknown.add(name)
    if unknown:
        for n in sorted(unknown):
            failures.append(f'B: unattributed conductor call in the score: {n}()')
            print(f'  ** UNATTRIBUTED ** {n}()')
    else:
        print(f'  every free call attributed ({len(MANIFEST_CALLS)} manifest, {len(FOUNDATIONAL)} foundational)')

    # Direction W (m5): the witness sole-author law — no module file
    # may write the witness record's guarded fields.
    print('── DIRECTION W: the witness sole-author law ──')
    GUARDS = [
        (r'readback_(?:x|z|portal_trigger)\s*=[^=]', {'cartridge.hpp', 'contracts/spine_state.hpp'},
         'readback trio: P5 harvest + teardown reset + portal consume (spine only; the record declares its own defaults)'),
        (r'player_\.possessed_slot\s*=[^=]', {'bodies/agents.inl'},
         'possession: the agents door only (re-anchoring, v3 §9 Act III)'),
        (r'player_\.aura_presence\s*[+]?=[^=]', {'bodies/pawn.inl'},
         'aura presence: P8, pawn-owned'),
    ]
    import glob as _glob
    module_files = [f for f in _glob.glob(os.path.join(TB, '**', '*.inl'), recursive=True)
                    + _glob.glob(os.path.join(TB, '**', '*.hpp'), recursive=True)]
    for pat, allowed, law in GUARDS:
        offenders = []
        for f in module_files:
            rel = os.path.relpath(f, TB).replace(os.sep, '/')
            if rel in allowed or rel == 'cartridge.hpp' and 'cartridge.hpp' in allowed:
                continue
            body = strip_comments_strings(open(f, encoding='utf-8').read())
            if re.search(pat, body):
                offenders.append(rel)
        if offenders:
            for o in offenders:
                failures.append(f'W: {o} writes a guarded witness field ({law})')
                print(f'  ** VIOLATION ** {o}: {law}')
        else:
            print(f'  {law}: HOLDS')

    print('──')
    if failures:
        for f in failures: print('FAIL:', f)
        print(f'SCORE CENSUS: RED ({len(failures)} violation(s))')
        return 1
    print('SCORE CENSUS: GREEN — the bijection holds, both directions')
    return 0

if __name__ == '__main__':
    sys.exit(main())
