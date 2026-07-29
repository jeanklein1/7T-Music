#!/usr/bin/env python3
"""[CC-3] Dead-binding cross: layout entries vs static shader usage.

Joins the CC-6 layout table (cc6_output.json) with the CC-4 static-usage table
(cc4_output.json) under the pipeline->layout mapping read from renderer.hpp
(createComputePipelines / createRenderPipelines, verified by hand at HEAD).

A layout entry is DEAD iff no entry point that runs against that layout
statically reaches the entry's binding number (group 0 for compute layouts).
Dawn permits dead (superset) entries — they cost budget, not validity — so
"dead" here means "safe to remove", which is exactly what probe A claims.

Usage: python3 audit/cc3_dead_entry_cross.py  (expects the two json files beside it)
"""
import json
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
cc6 = json.load(open(os.path.join(here, "cc6_output.json")))
cc4 = json.load(open(os.path.join(here, "cc4_output.json")))

# Pipeline -> layout mapping at CLOSURE_GPU HEAD (renderer.hpp
# createComputePipelines, read 2026-07-21; zone_mesh_gen_layout() aliases
# zoneGolComputeLayout_ — state.hpp:2630). The five live-contributor agents
# run on [Compute Entity (g0), Compute Texture (g1)]; compute_vp on g0 alone;
# update_terrain_config was REMOVED (raymarch/SDF excavation).
LIVE_CONTRIB = ["update_player_agent", "update_other_agents", "update_camera",
                "update_sphere", "update_cube"]
LAYOUT_EPS = {
    "Compute Entity Layout": LIVE_CONTRIB + ["compute_vp"],
    "Compute Texture Layout": LIVE_CONTRIB,          # group 1 of the same pipelines
    "Terrain Index Gen Layout": ["generate_terrain_indices"],
    "Patch Gen Layout": [
        "generate_patch_heights", "generate_patch_gradients", "generate_patch_cells",
    ],
    "Ribbon Compute Layout": ["compute_ribbon_rings"],
    "Photographer Compute Layout": ["compute_photographer_vp"],
    "Entity Placement Compute Layout": ["compute_entity_placement"],
    "Frustum Cull Compute Layout": ["frustum_cull_patches"],
    "Pawn Aura Compute Layout": ["compute_pawn_aura"],
    "Orb Compute Layout": ["orb_init", "orb_dynamics", "orb_recolor"],
    "Orb Copy Layout": ["orb_state_prev_copy"],
    "GoL Zone Compute Layout": [
        "zone_gol_sync", "zone_gol_evolve", "zone_gol_mesh_reset",
        "zone_gol_mesh_gen", "zone_derive_params",
    ],
    "Arch Mesh Gen Layout": ["arch_mesh_gen"],
    "Column Mesh Gen Layout": ["column_mesh_gen"],
    "Palm Mesh Gen Layout": ["palm_mesh_gen"],
    "Cactus Mesh Gen Layout": ["cactus_mesh_gen"],
    "Blade Mesh Gen Layout": ["blade_cluster_mesh_gen"],
}
# Group attribution: Compute Texture Layout binds at group 1; every other
# layout in this map binds at group 0.
LAYOUT_GROUP = {label: (1 if label == "Compute Texture Layout" else 0)
                for label in LAYOUT_EPS}

usage = cc4["usage"]
layouts = {l["label"]: l for l in cc6["layouts"]}

report = {}
for label, eps in LAYOUT_EPS.items():
    lay = layouts.get(label)
    if lay is None:
        report[label] = {"error": "layout not found in cc6_output.json"}
        continue
    grp = LAYOUT_GROUP[label]
    missing_eps = [ep for ep in eps if ep not in usage]
    used = set()
    per_ep = {}
    for ep in eps:
        if ep not in usage:
            continue
        b = {x["binding"] for x in usage[ep]["bindings"] if x["group"] == grp}
        per_ep[ep] = sorted(b)
        used |= b
    entries = {e["binding"] for e in lay["entries"]}
    dead = sorted(entries - used)
    unlisted = sorted(used - entries)  # shader uses a binding the layout lacks -> would FAIL pipeline creation
    report[label] = {
        "entry_points": eps,
        "missing_entry_points": missing_eps,
        "layout_bindings": sorted(entries),
        "union_of_static_usage": sorted(used),
        "per_entry_point_usage": per_ep,
        "DEAD_entries": dead,
        "MISSING_from_layout": unlisted,
    }

print(json.dumps(report, indent=2))

fails = {k: v for k, v in report.items() if v.get("MISSING_from_layout")}
if fails:
    print("\n!! Some layouts are missing bindings their shaders use — pipeline "
          "creation would fail: " + ", ".join(fails), file=sys.stderr)
