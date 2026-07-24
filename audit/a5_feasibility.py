#!/usr/bin/env python3
"""[A5-3] THE FEASIBILITY LEDGER, extended past radii to every profile column.

The GATE-FEASIBILITY RULE banner covered radii (a 3D gate at altitude H fires
only if R > H). This extends it to CAPS: for each PRESENCE row, the maximum
attainable response magnitude from the AUTHORED ranges vs that row's cap, and
the "cap-engagement dt" -- the frame time at which the cap first bites. If a
cap is unreachable at 60 Hz it is not a tuning knob, it is a FRAME-HITCH GUARD.

All numbers are read from the live tree (values pasted here with file:line;
verify with the greps in the report). Recipe only -- no edits.

Usage: python3 audit/a5_feasibility.py
"""
import json

DT60 = 1.0 / 60.0

# Authored, live @ 3cc67a8:
#   CONTACT_SPRING 40 (world.wgsl:2210), CONTACT_IMPULSE_CAP 6 (:2211)
#   SPHERE_PUSH_GAIN 40 (:2280), CUBE_PUSH_RADIUS 7 (:2268), CUBE_PUSH_GAIN 25 (:2270), CUBE_PART_CAP 12 (:2271)
#   contact_radius {1.6,1.4,2.0,1.8} (agents.hpp:182-185) -> max 2.0
#   contact_mass   {1.0,0.8,1.5,1.2}; yield = m_o/(m_s+m_o) in (0,1); worst case -> 1
#   sphere influence_radius mu {8.0 Sentinel, 6.0 Anomaly}, sigma {2.0,1.5}, floor 3 (spheres.hpp:136/141)
#   sphere body_radius mu {1.5,1.2}
#   point_bubble_radius 20; flee_gain_player {0.70,0.85,0.50,0.60}; NONPLAYER_FLEE_GAIN 0.8

# PRESENCE rows: max magnitude at d_gate -> 0 (overlap = radius), fall -> 1.
PRESENCE = [
    # name, max_overlap(=max radius), gain, max_yield, cap, note
    ("row1/4 agent contact",   2.0 + 2.0,  40.0, 1.0,  6.0,  "max contact_r pair 2.0+2.0; yield<=1"),
    ("row3/8 sphere push (mu)", 8.0,       40.0, 1.0,  6.0,  "influence_radius mu 8 (Sentinel)"),
    ("row3/8 sphere push (tail)", 12.0,    40.0, 1.0,  6.0,  "influence_radius mu+2sigma ~12"),
    ("row6 agent<->sphere",    2.0 + 1.5,  40.0, 1.0,  6.0,  "contact_r 2.0 + body_radius 1.5"),
    ("row9 cube push",         7.0,        25.0, 1.0, 12.0,  "CUBE_PUSH_RADIUS 7, fall=1 at centre"),
]

rows = []
for name, overlap, gain, yld, cap, note in PRESENCE:
    max_mag_60 = overlap * gain * DT60 * yld
    # dt at which max magnitude first reaches the cap:
    engage_dt = cap / (overlap * gain * yld)
    engage_fps = 1.0 / engage_dt
    rows.append({
        "row": name,
        "max_overlap_wu": round(overlap, 3),
        "gain": gain,
        "max_mag_at_60Hz": round(max_mag_60, 3),
        "cap": cap,
        "reachable_at_60Hz": max_mag_60 >= cap,
        "cap_engagement_dt_s": round(engage_dt, 4),
        "cap_engagement_fps": round(engage_fps, 1),
        "note": note,
    })

# Sphere: the influence_radius at which the cap engages AT 60 Hz.
sphere_engage_radius = 6.0 / (40.0 * DT60)   # cap / (gain*dt)
flee_note = ("rows 2/5/7 (flees) are APPROACH, cap = INFLUENCE_NO_CAP 1e9; "
             "deficit ~ v_ap*gain (<~ player speed 15) -- never within 8 orders of the cap.")

out = {
    "dt_reference": "60 Hz = 0.01667 s",
    "presence_rows": rows,
    "sphere_cap_engages_at_influence_radius_60Hz": round(sphere_engage_radius, 2),
    "flee_rows": flee_note,
    "verdict": (
        "Every PRESENCE cap is UNREACHABLE at 60 Hz EXCEPT the sphere push for "
        "influence_radius >= 9 wu (the sigma tail of Sentinel mu 8 / sigma 2). "
        "So the contact + cube caps are FRAME-HITCH GUARDS (engage only below "
        f"{min(r['cap_engagement_fps'] for r in rows if 'cube' in r['row'] or 'agent' in r['row']):.0f}-27 fps); "
        "the sphere cap is a REAL limiter for large spheres. A cap presented as "
        "a tuning knob that is really a hitch guard is the drift the charter forbids."
    ),
}
print(json.dumps(out, indent=2))
