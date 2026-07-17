/* ═══════════════════════════════════════════════════════════════════════
   7T TOOL REGISTRY
   
   Add a new tool:
     1. Drop  7t_my_tool.jsx  into  src/tools/
     2. Add one entry below
     3. Done — it appears in the launcher at #my_tool
   ═══════════════════════════════════════════════════════════════════════ */

const TOOLS = [
  {
    id:    "population_designer",
    name:  "Population Designer",
    desc:  "The spawn-probability stack as a living instrument — per-factor sliders, live composition trace, spatial-density map, C++ export (composition recon R2b)",
    color: "#e8d8b0",
    load:  () => import("./7t_population_designer.jsx"),
  },
  {
    id:    "theme_tool",
    name:  "Theme Tool",
    desc:  "Entity distribution designer — 4-pass procedural placement synced with the C++ engine",
    color: "#d4713b",
    load:  () => import("./7t_theme_tool.jsx"),
  },
  {
    id:    "stamp",
    name:  "Stamp Generator",
    desc:  "Procedural terrain patterns extracted for product design — Voronoi, color engine, SVG export",
    color: "#4a90c4",
    load:  () => import("./7t_stamp.jsx"),
  },
  {
    id:    "pawn_designer",
    name:  "Pawn Designer",
    desc:  "Color gradient & profile geometry workbench for the 7T pawn",
    color: "#6aaa5c",
    load:  () => import("./7t_pawn_designer.jsx"),
  },
  {
    id:    "column_designer",
    name:  "Column Designer",
    desc:  "Surface-of-revolution geometry — 6 tiers from Pillar to Antenna Colossal",
    color: "#b07acc",
    load:  () => import("./7t_column_designer.jsx"),
  },
  {
    id:    "arch_designer",
    name:  "Arch Designer",
    desc:  "Catenary barrel vault — 3 tiers (Doorway / Standard / Monumental)",
    color: "#c4786a",
    load:  () => import("./7t_arch_designer.jsx"),
  },
  {
    id:    "pyramid_designer",
    name:  "Pyramid Designer",
    desc:  "Truncated quad solid — 3 tiers (Obelisk / Temple / Colossus)",
    color: "#c4a040",
    load:  () => import("./7t_pyramid_designer.jsx"),
  },
  {
    id:    "ribbon_designer",
    name:  "Ribbon Designer",
    desc:  "Animated cube-chain spine — 3 compound waves, harmonic ratio palettes",
    color: "#5ab0c4",
    load:  () => import("./7t_ribbon_designer.jsx"),
  },
  {
    id:    "antenna_designer",
    name:  "Antenna Designer",
    desc:  "Tall post with stacked drum elements — Antenna / Squat / Colossal tiers",
    color: "#8a8aaa",
    load:  () => import("./7t_antenna_designer.jsx"),
  },
  {
    id:    "palm_designer",
    name:  "Palm Designer",
    desc:  "Tapered trunk with lean + radial frond crown — 3 tiers (Sapling / Coastal / Royal)",
    color: "#3a8a3a",
    load:  () => import("./7t_palm_designer.jsx"),
  },
  {
    id:    "blade_cluster_designer",
    name:  "Blade Cluster Designer",
    desc:  "Ground-level leaf clusters — sparse pointed blades from a single point, 3 tiers",
    color: "#4a7a3a",
    load:  () => import("./7t_blade_cluster_designer.jsx"),
  },
  {
    id:    "cactus_designer",
    name:  "Cactus Designer",
    desc:  "Ribbed columnar cacti with optional forking arms — 3 tiers (Finger / Saguaro / Candelabra)",
    color: "#2a6a4a",
    load:  () => import("./7t_cactus_designer.jsx"),
  },
];

export default TOOLS;