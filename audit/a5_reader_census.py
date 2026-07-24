#!/usr/bin/env python3
"""[A5-2 / A5-8] Reader census — reproduces the possessed_slot, neighbor_radius,
and personal_radius read counts from the live tree (grep-driven; no edits).

Usage: python3 audit/a5_reader_census.py
"""
import re, json, os

ROOT = os.path.join(os.path.dirname(__file__), "..")
W = os.path.join(ROOT, "src/cartridges/the_board/realization/world.wgsl")
text = open(W, encoding="utf-8").read().splitlines()

def hits(pat):
    rx = re.compile(pat)
    return [(i + 1, ln.strip()) for i, ln in enumerate(text) if rx.search(ln)]

# possessed_slot reads inside behavior_* bodies (A5-2). We report all reads and
# tag those that read the POSITION (agent_state[possessed_slot]) vs an index skip.
poss = hits(r"config\.possessed_slot")
poss_pos = hits(r"agent_state\[config\.possessed_slot\]")

# neighbor_radius (behavior-table) vs personal_radius (tier) reads (A5-8).
nbr = hits(r"\bb\.neighbor_radius\b")
pers = hits(r"\.personal_radius\b")

out = {
    "possessed_slot_total_reads": len(poss),
    "possessed_slot_position_reads (agent_state[...])": [f"{n}: {t}" for n, t in poss_pos],
    "b.neighbor_radius_reads": [f"{n}: {t}" for n, t in nbr],
    "personal_radius_reads": [f"{n}: {t}" for n, t in pers],
    "notes": (
        "A5-2: of the behavior-layer possessed_slot reads, the POSITION reads in "
        "behavior_pursuit + behavior_flee are POINT-terms (should route point_pos()); "
        "the skip-self reads in biased_walk + flock2d are index-identity (non-convertible). "
        "A5-8: b.neighbor_radius is LIVE for pursuit/flee/biased_walk; the flock gates on "
        "g.personal_radius, so AGENT_BEHAVIORS[8].neighbor_radius (flock row) is the only DEAD one."
    ),
}
print(json.dumps(out, indent=2))
