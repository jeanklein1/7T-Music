
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
[Console] Adapter 4: CPU / D3D12 | Microsoft Basic Render Driver (D3D12 driver version 10.0.19041.3636) vendor=microsoft
[Console] Adapter 5: CPU / D3D11 | Microsoft Basic Render Driver (D3D11 driver version 10.0.19041.3636) vendor=microsoft
[Console] Adapter 6: CPU / Null | Null backend () vendor=
[Console] Adapter selected: index=2
[Console] Adapter limits: storageBuffers/stage=10 uniformBuffers/stage=12 bindingsPerGroup=1000
[Console] Adapter features (32): 1 2 3 4 5 9 10 12 13 14 15 16 17 19 21 22 327680 327681 327682 327684 327692 327696 327700 327701 327704 327715 327722 327724 327727 327728 327729 327732
[Console] feature multi-draw-indirect=no timestamp-query=YES
[Incubator] BeatClock ready (bpm 100)
[GPUState] Design Config: 624 B (C++ side; WGSL DesignConfig mirror must match)
[GPUState] Monolith mesh: 24 verts, 36 indices
[GPUState] Arch buffers (GPU mesh gen): 32000 vert, 120000 index capacity
[GPUState] Column buffers (GPU mesh gen): 48000 vert, 192000 index capacity
[GPUState] Shell buffers: 2048 vert, 8192 index capacity
[GPUState] GoL zone buffers: 8 zones ├ù 32├ù32 grid
[Cartridge] GPUState init:    83 ms
[SPINE] validated: 9 update rows + 22 render rows + 12 dispatch rows name-checked; O-#/RC laws static-asserted
Loaded shader from: ../../../src/cartridges/the_board/realization/world.wgsl
[Renderer] Shader compile:    297 ms
  [Pipeline] update_player_agent: 2848 ms
  [Pipeline] update_other_agents: 3856 ms
  [Pipeline] update_camera: 397 ms
  [Pipeline] update_sphere: 661 ms
  [Pipeline] update_cube: 883 ms
  [Pipeline] compute_vp: 287 ms
  [Pipeline] gen_patch_heights: 777 ms
  [Pipeline] gen_patch_gradients: 388 ms
  [Pipeline] gen_patch_cells: 989 ms
  [Pipeline] compute_ribbon_rings: 312 ms
  [Pipeline] compute_photographer_vp: 311 ms
  [Pipeline] compute_entity_placement: 461 ms
  [Pipeline] frustum_cull_patches: 355 ms
  [Pipeline] compute_pawn_aura: 1021 ms
  [Pipeline] write_live_card_heights: 589 ms
  [Pipeline] write_live_card_resolve: 345 ms
  [Pipeline] orb_init: 340 ms
  [Pipeline] orb_dynamics: 486 ms
  [Pipeline] orb_recolor: 297 ms
  [Pipeline] orb_state_prev_copy: 249 ms
  [Pipeline] zone_gol_sync: 256 ms
  [Pipeline] zone_gol_evolve: 343 ms
  [Pipeline] zone_derive_params: 516 ms
  [Pipeline] zone_seed_mask: 476 ms
  [Pipeline] arch_mesh_gen: 1037 ms
  [Pipeline] column_mesh_gen: 1629 ms
  [Pipeline] palm_mesh_gen: 584 ms
  [Pipeline] cactus_mesh_gen: 669 ms
  [Pipeline] blade_cluster_mesh_gen: 420 ms
  [Pipeline] patch_terrain: 4689 ms
  [Pipeline] patch_terrain_indirect: 4590 ms
  [Pipeline] pawn: 3670 ms
  [Pipeline] sphere: 2667 ms
  [Pipeline] monolith: 3699 ms
  [Pipeline] arch: 938 ms
  [Pipeline] column: 923 ms
  [Pipeline] palm: 928 ms
  [Pipeline] cactus: 912 ms
  [Pipeline] blade: 946 ms
  [Pipeline] shell: 891 ms
  [Pipeline] ribbon: 737 ms
  [Pipeline] orb: 478 ms
  [Pipeline] gallery_frame: 534 ms
  [Pipeline] wall_painting_canvas: 593 ms
  [Pipeline] wall_painting_frame: 584 ms
  [Pipeline] shadow_patch_terrain: 300 ms
  [Pipeline] shadow_pawn: 806 ms
  [Pipeline] shadow_sphere: 1636 ms
  [Pipeline] shadow_monolith: 2213 ms
  [Pipeline] shadow_arch: 235 ms
  [Pipeline] shadow_column: 239 ms
  [Pipeline] shadow_palm: 244 ms
  [Pipeline] shadow_cactus: 238 ms
  [Pipeline] shadow_blade: 237 ms
  [Pipeline] shadow_shell: 227 ms
  [Pipeline] shadow_ribbon: 307 ms
  [Pipeline] shadow_gallery_frame: 285 ms
  [Pipeline] shadow_wall_painting: 329 ms
  [Pipeline] fade_overlay: 450 ms

[Renderer] Pipelines by compile time (descending):
      4689 ms  patch_terrain
      4590 ms  patch_terrain_indirect
      3856 ms  update_other_agents
      3699 ms  monolith
      3670 ms  pawn
      2848 ms  update_player_agent
      2667 ms  sphere
      2213 ms  shadow_monolith
      1636 ms  shadow_sphere
      1629 ms  column_mesh_gen
      1037 ms  arch_mesh_gen
      1021 ms  compute_pawn_aura
       989 ms  gen_patch_cells
       946 ms  blade
       938 ms  arch
       928 ms  palm
       923 ms  column
       912 ms  cactus
       891 ms  shell
       883 ms  update_cube
       806 ms  shadow_pawn
       777 ms  gen_patch_heights
       737 ms  ribbon
       669 ms  cactus_mesh_gen
       661 ms  update_sphere
       593 ms  wall_painting_canvas
       589 ms  write_live_card_heights
       584 ms  palm_mesh_gen
       584 ms  wall_painting_frame
       534 ms  gallery_frame
       516 ms  zone_derive_params
       486 ms  orb_dynamics
       478 ms  orb
       476 ms  zone_seed_mask
       461 ms  compute_entity_placement
       450 ms  fade_overlay
       420 ms  blade_cluster_mesh_gen
       397 ms  update_camera
       388 ms  gen_patch_gradients
       355 ms  frustum_cull_patches
       345 ms  write_live_card_resolve
       343 ms  zone_gol_evolve
       340 ms  orb_init
       329 ms  shadow_wall_painting
       312 ms  compute_ribbon_rings
       311 ms  compute_photographer_vp
       307 ms  shadow_ribbon
       300 ms  shadow_patch_terrain
       297 ms  orb_recolor
       287 ms  compute_vp
       285 ms  shadow_gallery_frame
       256 ms  zone_gol_sync
       249 ms  orb_state_prev_copy
       244 ms  shadow_palm
       239 ms  shadow_column
       238 ms  shadow_cactus
       237 ms  shadow_blade
       235 ms  shadow_arch
       227 ms  shadow_shell

[Renderer] Compute pipelines: 21826 ms
[Renderer] Render pipelines:  35568 ms
[Renderer] Total pipelines:   57395 ms
[Orbs] Configured: count=128 palette=jwst_deep drag=0.4 noise=0.3 rule=brownian rot=0.012 orbital=0.15 tiers=jwst_stars
[Mood] Applied: open_sunset (mood=0 outdoor)
[Agents] Spawned 10 for mood 0 around (0,0)
[AGENTS t=0.0 trigger=boot] 11/32 active, possessed=0 tier:{worker=7 scout=4} drv:{player=1 biased_walk=10}
[CENSUS t=    0.0 trigger=boot]
  fam    active  claimed   delta     new
  pyr         0        0       0       0
  arch        0        0       0       0
  col         0        0       0       0
  ant         0        0       0       0
  palm        0        0       0       0
  cact        0        0       0       0
  blad        0        0       0       0
  sph         0        ΓÇö       ΓÇö       ΓÇö
  ribn        0        0       0       0
  cube        0        ΓÇö       ΓÇö       ΓÇö
  gol         0        0       0       0
  gall        0        0       0       0
  TOTAL       0        0       0       0    footprints 0/128
  fam      live   hi-wtr     cap  portal
  pyr         0        0       8       ΓÇö
  arch        0        0      16       0
  col         0        0      16       ΓÇö
  ant         0        0      16       ΓÇö
  palm        0        0      24       ΓÇö
  cact        0        0      20       ΓÇö
  blad        0        0      32       ΓÇö
  sph         0        0       8       ΓÇö
  ribn        0        0       1       ΓÇö
  cube        0        0     256       ΓÇö
  gol         0        0       8       ΓÇö
  gall        0        0      48       ΓÇö
[Authored] Scanned assets/paintings ΓÇö found 57 paintings
[Authored] Loaded: assets/paintings\PAINTING_1.jpg (1505x1201) ΓåÆ staging 0
[Authored] Scaled ΓåÆ 1024x817 (aspect 1.3)
[Authored] Loaded: assets/paintings\PAINTING_2.jpeg (1280x1007) ΓåÆ staging 1
[Authored] Scaled ΓåÆ 1024x806 (aspect 1.3)
[Authored] Loaded: assets/paintings\PAINTING_3.jpeg (1280x843) ΓåÆ staging 2
[Authored] Scaled ΓåÆ 1024x674 (aspect 1.5)
[Authored] Loaded: assets/paintings\PAINTING_4.jpeg (1272x825) ΓåÆ staging 3
[Authored] Scaled ΓåÆ 1024x664 (aspect 1.5)
[Authored] Loaded: assets/paintings\PAINTING_5.jpeg (1283x1020) ΓåÆ staging 4
[Authored] Scaled ΓåÆ 1024x814 (aspect 1.3)
[Authored] Loaded: assets/paintings\PAINTING_6.jpeg (1450x1166) ΓåÆ staging 5
[Authored] Scaled ΓåÆ 1024x823 (aspect 1.2)
[Authored] Loaded: assets/paintings\PAINTING_7.jpeg (1600x985) ΓåÆ staging 6
[Authored] Scaled ΓåÆ 1024x630 (aspect 1.6)
[Authored] Loaded: assets/paintings\PAINTING_8.jpeg (1180x933) ΓåÆ staging 7
[Authored] Scaled ΓåÆ 1024x810 (aspect 1.3)
[Authored] Loaded: assets/paintings\PAINTING_9.jpeg (1080x1011) ΓåÆ staging 8
[Authored] Scaled ΓåÆ 1024x959 (aspect 1.1)
[Authored] Loaded: assets/paintings\PAINTING_10.jpeg (777x971) ΓåÆ staging 9
[Authored] Scaled ΓåÆ 777x971 (aspect 0.8)
[Authored] Loaded: assets/paintings\PAINTING_11.jpeg (1264x1572) ΓåÆ staging 10
[Authored] Scaled ΓåÆ 823x1024 (aspect 0.8)
[Authored] Loaded: assets/paintings\PAINTING_12.jpeg (1080x1304) ΓåÆ staging 11
[Authored] Scaled ΓåÆ 848x1024 (aspect 0.8)
[Authored] Loaded: assets/paintings\PAINTING_14.jpeg (859x696) ΓåÆ staging 12
[Authored] Scaled ΓåÆ 859x696 (aspect 1.2)
[Authored] Loaded: assets/paintings\PAINTING_32.jpeg (1280x1040) ΓåÆ staging 13
[Authored] Scaled ΓåÆ 1024x832 (aspect 1.2)
[Authored] Loaded: assets/paintings\PAINTING_50.jpeg (837x1280) ΓåÆ staging 14
[Authored] Scaled ΓåÆ 670x1024 (aspect 0.7)
[Authored] Loaded: assets/paintings\PAINTING_60.jpeg (920x926) ΓåÆ staging 15
[Authored] Scaled ΓåÆ 920x926 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_70.jpeg (1280x906) ΓåÆ staging 16
[Authored] Scaled ΓåÆ 1024x725 (aspect 1.4)
[Authored] Loaded: assets/paintings\PAINTING_71.jpeg (1280x1032) ΓåÆ staging 17
[Authored] Scaled ΓåÆ 1024x826 (aspect 1.2)
[Authored] Loaded: assets/paintings\PAINTING_72.jpeg (1268x1280) ΓåÆ staging 18
[Authored] Scaled ΓåÆ 1014x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_73.jpeg (1279x1280) ΓåÆ staging 19
[Authored] Scaled ΓåÆ 1023x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_90.jpeg (1280x506) ΓåÆ staging 20
[Authored] Scaled ΓåÆ 1024x405 (aspect 2.5)
[Authored] Loaded: assets/paintings\PAINTING_92.jpeg (1280x720) ΓåÆ staging 21
[Authored] Scaled ΓåÆ 1024x576 (aspect 1.8)
[Authored] Loaded: assets/paintings\PAINTING_100.jpeg (995x1028) ΓåÆ staging 22
[Authored] Scaled ΓåÆ 991x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_101.jpeg (1554x1600) ΓåÆ staging 23
[Authored] Scaled ΓåÆ 995x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_102.jpeg (1225x1280) ΓåÆ staging 24
[Authored] Scaled ΓåÆ 980x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_103.jpeg (1508x1600) ΓåÆ staging 25
[Authored] Scaled ΓåÆ 965x1024 (aspect 0.9)
[Authored] Loaded: assets/paintings\PAINTING_104.jpeg (1280x1169) ΓåÆ staging 26
[Authored] Scaled ΓåÆ 1024x935 (aspect 1.1)
[Authored] Loaded: assets/paintings\PAINTING_105.jpeg (1280x1219) ΓåÆ staging 27
[Authored] Scaled ΓåÆ 1024x975 (aspect 1.1)
[Authored] Loaded: assets/paintings\PAINTING_106.jpeg (1079x1280) ΓåÆ staging 28
[Authored] Scaled ΓåÆ 863x1024 (aspect 0.8)
[Authored] Loaded: assets/paintings\PAINTING_107.jpeg (1039x1280) ΓåÆ staging 29
[Authored] Scaled ΓåÆ 831x1024 (aspect 0.8)
[Authored] Loaded: assets/paintings\PAINTING_108.jpeg (1115x1132) ΓåÆ staging 30
[Authored] Scaled ΓåÆ 1009x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_109.jpeg (940x1280) ΓåÆ staging 31
[Authored] Scaled ΓåÆ 752x1024 (aspect 0.7)
[Authored] Staged 32/57 images
[Cartridge] Renderer init:    58443 ms
[Cartridge] Patch system:     1841 ms
[Cartridge] Total init:       60284 ms
[Ground] zone rects in core: 0 (boot)
[Incubator] the_board renderer ready
[SignalLayout] source 'all.field' not in layout (coupling disabled)
[SignalLayout] source 'ch1.present_count' not in layout (coupling disabled)
[SignalLayout] source 'all.window_length' not in layout (coupling disabled)
[SignalLayout] source 'all.present_count' not in layout (coupling disabled)
[SignalLayout] source 'ch1.window_length' not in layout (coupling disabled)
[SignalLayout] source 'ch0.onset' not in layout (coupling disabled)
[SignalLayout] source 'ch1.onset' not in layout (coupling disabled)
[SignalLayout] source 'ch2.onset' not in layout (coupling disabled)
[SignalLayout] source 'ch3.onset' not in layout (coupling disabled)
[SignalLayout] source 'ch4.onset' not in layout (coupling disabled)
[SignalLayout] source 'ch5.onset' not in layout (coupling disabled)
[SignalLayout] source 'ch6.onset' not in layout (coupling disabled)
[Zoetrope] ears bound: 0 of 7 (mask 0x7F)
[the_board] fog.density base=0 valid=1 | fog.color base=1 count=3 valid=1
[the_board] terrain.checker_mean base=10 count=3 valid=1 | terrain.checker_var base=13 valid=1
[Incubator] Hot reload enabled: ../../../src/cartridges/the_board/realization/world.wgsl

Controls: WASD=move, Mouse=camera, 5-8=moods, Esc=quit

[Ribbon] SPAWN slot=0 at (-22.7, -27.1) tier=0 len=562.0 near=(-1,-1) far=(1,-12)
[CENSUS t=    0.1 trigger=periodic]
  fam    active  claimed   delta     new
  pyr         0        0       0       0
  arch        1        1       0       1
  col         4        4       0       4
  ant         5        5       0       5
  palm        5        5       0       5
  cact        1        1       0       1
  blad        1        1       0       1
  sph         0        ΓÇö       ΓÇö       ΓÇö
  ribn        1        1       0       1
  cube       10        ΓÇö       ΓÇö       ΓÇö
  gol         0        0       0       0
  gall        0        0       0       0
  TOTAL      28       18       0      18    footprints 18/128
  fam      live   hi-wtr     cap  portal
  pyr         0        0       8       ΓÇö
  arch        1        2      16       1
  col         4        4      16       ΓÇö
  ant         5        5      16       ΓÇö
  palm        5        5      24       ΓÇö
  cact        1        1      20       ΓÇö
  blad        1        1      32       ΓÇö
  sph         0        0       8       ΓÇö
  ribn        1        1       1       ΓÇö
  cube       10       10     256       ΓÇö
  gol         0        0       8       ΓÇö
  gall        0        0      48       ΓÇö
  claimed ground ΓÇö arrivals (18):
  ribn t0 (   -22.7,   -27.1) p( -1, -1) age=0.0
  ant t5 (   -25.1,   -72.9) p( -1, -2) age=0.0
  palm t1 (   -71.7,    21.4) p( -2,  0) age=0.0
  blad t0 (    79.8,    22.4) p(  1,  0) age=0.0
  palm t1 (   -67.8,   -71.5) p( -2, -2) age=0.0
  palm t2 (    66.5,   -74.4) p(  1, -2) age=0.0
  col t2 (   122.4,   -33.7) p(  2, -1) age=0.0
  col t2 (  -128.4,    70.5) p( -3,  1) age=0.0
  col t2 (   123.2,    73.8) p(  2,  1) age=0.0
  ant t5 (  -128.3,  -119.7) p( -3, -3) age=0.0
  palm t1 (   132.2,   128.5) p(  2,  2) age=0.0
  col t2 (   171.6,   -29.5) p(  3, -1) age=0.0
    ... +6 more
[METER] window 1f  fps 5.3  gpu sampled 0f | budget 16.6 ms
[METER] U fill_signal             mean 0.01  max 0.01
[METER] U advance_clock           mean 0.00  max 0.00
[METER] U motion_drivers          mean 0.04  max 0.04
[METER] U motion_bodies           mean 0.00  max 0.00
[METER] U stage_world             mean 0.00  max 0.00
[METER] U transition_machine      mean 0.00  max 0.00
[METER] U stage_fade_and_upload   mean 0.01  max 0.01
[METER] U witness_photographer    mean 0.00  max 0.00
[METER] U clear_input_deltas      mean 0.00  max 0.00
[METER] R witness_harvest         mean 0.00  max 0.00
[METER] R portal_trigger          mean 0.00  max 0.00
[METER] R stream_patches          mean 5.28  max 5.28
[METER] R respawn_agents          mean 0.00  max 0.00
[METER] R census_dumps            mean 0.00  max 0.00
[METER] R ribbon_tick             mean 0.00  max 0.00
[METER] R entity_mesh_gen         mean 0.00  max 0.00
[METER] R upload_portal_lights    mean 0.00  max 0.00
[METER] R live_card_write         mean 0.00  max 0.00
[METER] R dispatch_compute        mean 0.00  max 0.00
[METER] R witness_capture         mean 0.00  max 0.00
[METER] R gol_derive_flush        mean 0.00  max 0.00
[METER] R gol_zone_compute        mean 0.00  max 0.00
[METER] R pawn_aura               mean 0.00  max 0.00
[METER] R orb_sky                 mean 0.00  max 0.00
[METER] R ground_entries          mean 0.00  max 0.00
[METER] R placement_correction    mean 0.00  max 0.00
[METER] R frustum_cull            mean 0.00  max 0.00
[METER] R shadow_pass             mean 0.00  max 0.00
[METER] R main_pass               mean 0.00  max 0.00
[METER] R snapshot_pass           mean 0.00  max 0.00
[METER] R promotion_drain         mean 0.00  max 0.00
[METER] U_SUM 0.06   R_SUM 5.28
[Orbs] Init dispatched: 128 orbs, 2 workgroups
[GoL] Pulse slot=0 node=(-3,-1) corner=(-350.0,-112.5) host=(-6,-2) HEIGHT period=1.4
[Ground] zones active anywhere: 1
[GoL] Conway slot=1 node=(0,2) corner=(34.4,275.0) host=(1,6) HEIGHT period=13.9
[Ground] zones active anywhere: 2
[Agents] Respawn 1 around (17.0,-12.5)
[Agents] Respawn 1 around (30.1,-21.8)
[Photographer] Capture -> layer 0 (Portrait) aspect=0.7 pool=1/32
[Photographer] Rendering snapshot -> layer 0
[Photographer] Capture -> layer 1 (Panoramic) aspect=1.9 pool=2/32
[Photographer] Rendering snapshot -> layer 1
[Agents] Respawn 1 around (52.4,-26.4)
[Agents] Respawn 1 around (70.4,-22.6)
[Agents] Respawn 1 around (94.3,-25.6)
[Agents] Respawn 1 around (95.2,-26.0)
[Ground] zones active anywhere: 1
[Agents] Respawn 1 around (102.8,-29.5)
[Photographer] Capture -> layer 2 (Portrait) aspect=0.6 pool=3/32
[Photographer] Rendering snapshot -> layer 2
[Photographer] Capture -> layer 3 (Medium) aspect=1.4 pool=4/32
[Photographer] Rendering snapshot -> layer 3
[Photographer] Capture -> layer 4 (Bird's Eye) aspect=1.0 pool=5/32
[Photographer] Rendering snapshot -> layer 4
[Agents] Respawn 1 around (142.5,-34.9)
[GoL] Pulse slot=0 node=(4,-2) corner=(512.5,-206.2) host=(10,-4) period=0.4
[Ground] zones active anywhere: 2
[Photographer] Capture -> layer 5 (Panoramic) aspect=2.3 pool=6/32
[Photographer] Rendering snapshot -> layer 5
[Photographer] Capture -> layer 6 (Low Angle) aspect=2.0 pool=7/32
[Photographer] Rendering snapshot -> layer 6
[Photographer] Capture -> layer 7 (Portrait) aspect=0.6 pool=8/32
[Photographer] Rendering snapshot -> layer 7
[Photographer] Capture -> layer 8 (Medium) aspect=1.6 pool=9/32
[Photographer] Rendering snapshot -> layer 8
[Portal] GPU trigger: arch 0 -> seed=1898512436 finite=1
[Agents] Respawn 1 around (226.1,-21.0)
[Lighting] Added vault uplight (slot 3)
[Lighting] Cathedral (4 lights, E/W walls)
[Mood] Indoor palette: terracotta (idx=2)
[WallPainting] Placed 19 painting(s) + 9 snapshot(s) across 4 walls (SNAPSHOT)
[Shell] Generated GROIN VAULT: 1105 verts, 6168 indices bounds=[-100.0,150.0] wall_h=25.0 crown=62.5 rise=37.5
[Mood] Applied: indoor_vault (mood=2 INDOOR)
[Agents] Spawned 4 for mood 2 around (0.0,0.0)
[AGENTS t=24.1 trigger=mood-transition] 5/32 active, possessed=0 tier:{worker=2 sentinel=3} drv:{player=1 slow_patrol=4}
[CENSUS t=   24.1 trigger=mood-transition]
  fam    active  claimed   delta     new
  pyr         0        0       0       0
  arch        0        0       0       0
  col         0        0       0       0
  ant         0        0       0       0
  palm        0        0       0       0
  cact        0        0       0       0
  blad        0        0       0       0
  sph         0        ΓÇö       ΓÇö       ΓÇö
  ribn        0        0       0       0
  cube        0        ΓÇö       ΓÇö       ΓÇö
  gol         0        0       0       0
  gall        0        0       0       0
  TOTAL       0        0       0       0    footprints 0/128
  fam      live   hi-wtr     cap  portal
  pyr         0        0       8       ΓÇö
  arch        0        0      16       0
  col         0        0      16       ΓÇö
  ant         0        0      16       ΓÇö
  palm        0        0      24       ΓÇö
  cact        0        0      20       ΓÇö
  blad        0        0      32       ΓÇö
  sph         0        0       8       ΓÇö
  ribn        0        0       1       ΓÇö
  cube        0        0     256       ΓÇö
  gol         0        0       8       ΓÇö
  gall        0        0      48       ΓÇö
[World] Teardown complete, seed=1898512436 mode=finite 5x5
[Portal] Back-portal spawned at (14.8,-71.6) rot=1.6 slot=0 -> return seed=42 mood=open_sunset
[Portal] Forward portal 1 at (121.6,32.8) -> seed=3437462692 mood=finite_outdoor FINITE
[Portal] Forward portal 2 at (13.4,121.6) -> seed=1100791468 mood=open_sunset open
[Portal] Finite world: 2 forward portals + 1 back-portal
[Ribbon] SPAWN slot=0 at (10.7, 10.7) tier=2 len=89.9 near=(0,0) far=(-2,-1)
[Ground] zones active anywhere: 0
[CENSUS t=   30.1 trigger=periodic]
  fam    active  claimed   delta     new
  pyr         0        0       0       0
  arch        5        5       0       5
  col         1        1       0       1
  ant         1        1       0       1
  palm        5        5       0       5
  cact        0        0       0       0
  blad        0        0       0       0
  sph         0        ΓÇö       ΓÇö       ΓÇö
  ribn        1        1       0       1
  cube        5        ΓÇö       ΓÇö       ΓÇö
  gol         0        0       0       0
  gall        0        0       0       0
  TOTAL      18       13       0      13    footprints 13/128
  fam      live   hi-wtr     cap  portal
  pyr         0        0       8       ΓÇö
  arch        5        5      16       5
  col         1        1      16       ΓÇö
  ant         1        1      16       ΓÇö
  palm        5        5      24       ΓÇö
  cact        0        0      20       ΓÇö
  blad        0        0      32       ΓÇö
  sph         0        0       8       ΓÇö
  ribn        1        1       1       ΓÇö
  cube        5        5     256       ΓÇö
  gol         0        0       8       ΓÇö
  gall        0        0      48       ΓÇö
  claimed ground ΓÇö arrivals (13):
  arch t0 (    14.8,   -71.6) p(  0, -2) age=6.0
  arch t0 (   121.6,    32.8) p(  2,  0) age=6.0
  arch t0 (    13.4,   121.6) p(  0,  2) age=6.0
  ribn t2 (    10.7,    10.7) p(  0,  0) age=6.0
  palm t0 (   -28.8,    31.5) p( -1,  0) age=6.0
  col t2 (    18.7,    31.5) p(  0,  0) age=6.0
  palm t0 (   -29.7,   -79.1) p( -1, -2) age=6.0
  ant t4 (   -78.7,   -28.2) p( -2, -1) age=6.0
  palm t1 (    78.7,   -34.0) p(  1, -1) age=6.0
  palm t1 (    77.9,    25.6) p(  1,  0) age=6.0
  palm t1 (   -21.3,    66.0) p( -1,  1) age=6.0
  arch t0 (   118.7,   -70.2) p(  2, -2) age=6.0
    ... +1 more
[METER] window 1383f  fps 43.7  gpu sampled 462f | budget 16.6 ms
[METER] U fill_signal             mean 0.00  max 0.02
[METER] U advance_clock           mean 0.00  max 0.01
[METER] U motion_drivers          mean 0.09  max 4.16
[METER] U motion_bodies           mean 0.00  max 0.13
[METER] U stage_world             mean 0.00  max 0.01
[METER] U transition_machine      mean 0.32  max 435.56
[METER] U stage_fade_and_upload   mean 0.02  max 0.21
[METER] U witness_photographer    mean 0.03  max 5.84
[METER] U clear_input_deltas      mean 0.00  max 0.00
[METER] R witness_harvest         mean 0.01  max 0.06
[METER] R portal_trigger          mean 0.00  max 5.89
[METER] R stream_patches          cpu 0.34/48.83  gpu 0.64/152.11
[METER] R respawn_agents          mean 0.03  max 5.92
[METER] R census_dumps            mean 0.13  max 172.83
[METER] R ribbon_tick             mean 0.07  max 0.39
[METER] R entity_mesh_gen         cpu 0.00/0.35  gpu 0.14/3.01
[METER] R upload_portal_lights    mean 0.00  max 0.04
[METER] R live_card_write         cpu 0.05/0.27  gpu 0.90/1.11
[METER] R dispatch_compute        cpu 0.14/0.58  gpu 1.25/1.44
[METER] R witness_capture         mean 0.00  max 0.39
[METER] R gol_derive_flush        mean 0.00  max 0.61
[METER] R gol_zone_compute        cpu 0.05/0.51  gpu 0.01/0.07
[METER] R pawn_aura               mean 0.00  max 0.03
[METER] R orb_sky                 cpu 0.05/3.54  gpu 0.04/0.07
[METER] R ground_entries          mean 0.00  max 0.10
[METER] R placement_correction    cpu 0.00/0.13  gpu 0.02/0.33
[METER] R frustum_cull            cpu 0.06/3.25  gpu 0.02/0.07
[METER] R shadow_pass             cpu 0.19/0.88  gpu 5.18/5.77
[METER] R main_pass               cpu 0.36/148.50  gpu 12.81/28.70
[METER] R snapshot_pass           mean 0.02  max 6.67
[METER] R promotion_drain         mean 0.00  max 0.08
[METER] U_SUM 0.46   R_SUM 1.51
[Photographer] Capture -> layer 9 (Panoramic) aspect=2.1 pool=10/32
[Photographer] Rendering snapshot -> layer 9
[Possess] 0 -> 3 (tier 0, dist 19.6)
[Photographer] Capture -> layer 10 (Medium) aspect=1.7 pool=11/32
[Photographer] Rendering snapshot -> layer 10
[Photographer] Capture -> layer 11 (Panoramic) aspect=2.2 pool=12/32
[Photographer] Rendering snapshot -> layer 11
[Photographer] Capture -> layer 12 (Panoramic) aspect=2.3 pool=13/32
[Photographer] Rendering snapshot -> layer 12
[Photographer] Capture -> layer 13 (Panoramic) aspect=2.1 pool=14/32
[Photographer] Rendering snapshot -> layer 13
[Photographer] Capture -> layer 14 (Medium) aspect=1.8 pool=15/32
[Photographer] Rendering snapshot -> layer 14
[Photographer] Capture -> layer 15 (Close-up) aspect=1.3 pool=16/32
[Photographer] Rendering snapshot -> layer 15
[Photographer] Capture -> layer 16 (Portrait) aspect=0.7 pool=17/32
[Photographer] Rendering snapshot -> layer 16
[Photographer] Capture -> layer 17 (Low Angle) aspect=1.9 pool=18/32
[Photographer] Rendering snapshot -> layer 17
[CENSUS t=   60.1 trigger=periodic]
  fam    active  claimed   delta     new
  pyr         0        0       0       0
  arch        5        5       0       0
  col         1        1       0       0
  ant         1        1       0       0
  palm        5        5       0       0
  cact        0        0       0       0
  blad        0        0       0       0
  sph         0        ΓÇö       ΓÇö       ΓÇö
  ribn        1        1       0       0
  cube        5        ΓÇö       ΓÇö       ΓÇö
  gol         0        0       0       0
  gall        0        0       0       0
  TOTAL      18       13       0       0    footprints 13/128
  fam      live   hi-wtr     cap  portal
  pyr         0        0       8       ΓÇö
  arch        5        5      16       5
  col         1        1      16       ΓÇö
  ant         1        1      16       ΓÇö
  palm        5        5      24       ΓÇö
  cact        0        0      20       ΓÇö
  blad        0        0      32       ΓÇö
  sph         0        0       8       ΓÇö
  ribn        1        1       1       ΓÇö
  cube        5        5     256       ΓÇö
  gol         0        0       8       ΓÇö
  gall        0        0      48       ΓÇö
[METER] window 990f  fps 32.5  gpu sampled 330f | budget 16.6 ms
[METER] U fill_signal             mean 0.00  max 0.16
[METER] U advance_clock           mean 0.00  max 0.00
[METER] U motion_drivers          mean 0.08  max 5.87
[METER] U motion_bodies           mean 0.00  max 0.02
[METER] U stage_world             mean 0.00  max 0.01
[METER] U transition_machine      mean 0.00  max 0.00
[METER] U stage_fade_and_upload   mean 0.02  max 0.29
[METER] U witness_photographer    mean 0.05  max 7.44
[METER] U clear_input_deltas      mean 0.00  max 0.14
[METER] R witness_harvest         mean 0.01  max 0.47
[METER] R portal_trigger          mean 0.00  max 0.00
[METER] R stream_patches          mean 0.14  max 0.77
[METER] R respawn_agents          mean 0.00  max 0.01
[METER] R census_dumps            mean 0.71  max 701.92
[METER] R ribbon_tick             mean 0.07  max 1.11
[METER] R entity_mesh_gen         mean 0.00  max 0.01
[METER] R upload_portal_lights    mean 0.00  max 0.00
[METER] R live_card_write         mean 0.00  max 0.01
[METER] R dispatch_compute        cpu 0.17/0.66  gpu 0.90/4.78
[METER] R witness_capture         mean 0.00  max 0.02
[METER] R gol_derive_flush        mean 0.00  max 0.01
[METER] R gol_zone_compute        mean 0.00  max 0.03
[METER] R pawn_aura               mean 0.00  max 0.00
[METER] R orb_sky                 mean 0.00  max 0.00
[METER] R ground_entries          mean 0.00  max 0.00
[METER] R placement_correction    mean 0.00  max 0.01
[METER] R frustum_cull            cpu 0.06/0.40  gpu 0.03/0.07
[METER] R shadow_pass             cpu 0.45/1.96  gpu 3.33/15.14
[METER] R main_pass               cpu 0.26/1.10  gpu 24.72/33.62
[METER] R snapshot_pass           mean 0.02  max 2.89
[METER] R promotion_drain         mean 0.00  max 0.00
[METER] U_SUM 0.16   R_SUM 1.92
[Portal] GPU trigger: arch 1 -> seed=3437462692 finite=1
[Authored] Loaded: assets/paintings\PAINTING_110.jpeg (1569x1148) ΓåÆ staging 0
[Authored] Scaled ΓåÆ 1024x749 (aspect 1.4)
[Authored] Loaded: assets/paintings\PAINTING_111.jpeg (1221x1280) ΓåÆ staging 1
[Authored] Scaled ΓåÆ 977x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_112.jpeg (1600x985) ΓåÆ staging 2
[Authored] Scaled ΓåÆ 1024x630 (aspect 1.6)
[Authored] Loaded: assets/paintings\PAINTING_113.jpeg (1555x1600) ΓåÆ staging 3
[Authored] Scaled ΓåÆ 995x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_114.jpeg (1028x1060) ΓåÆ staging 4
[Authored] Scaled ΓåÆ 993x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_115.jpeg (1266x1280) ΓåÆ staging 5
[Authored] Scaled ΓåÆ 1013x1024 (aspect 1.0)
[Authored] Loaded: assets/paintings\PAINTING_200.jpeg (752x1280) ΓåÆ staging 6
[Authored] Scaled ΓåÆ 602x1024 (aspect 0.6)
[Authored] Loaded: assets/paintings\PAINTING_201.jpeg (731x1280) ΓåÆ staging 7
[Authored] Scaled ΓåÆ 585x1024 (aspect 0.6)
[Authored] Loaded: assets/paintings\PAINTING_202.jpeg (736x1280) ΓåÆ staging 8
[Authored] Scaled ΓåÆ 589x1024 (aspect 0.6)
[Authored] Loaded: assets/paintings\PAINTING_203.jpeg (734x1280) ΓåÆ staging 9
[Authored] Scaled ΓåÆ 587x1024 (aspect 0.6)
[Authored] Loaded: assets/paintings\PAINTING_205.jpeg (1280x734) ΓåÆ staging 10
[Authored] Scaled ΓåÆ 1024x587 (aspect 1.7)
[Authored] Loaded: assets/paintings\PAINTING_206.jpeg (1280x701) ΓåÆ staging 11
[Authored] Scaled ΓåÆ 1024x561 (aspect 1.8)
[Authored] Loaded: assets/paintings\PAINTING_207.jpeg (1055x1600) ΓåÆ staging 12
[Authored] Scaled ΓåÆ 675x1024 (aspect 0.7)
[Authored] Loaded: assets/paintings\PAINTING_208.jpeg (769x1280) ΓåÆ staging 13
[Authored] Scaled ΓåÆ 615x1024 (aspect 0.6)
[Authored] Loaded: assets/paintings\PAINTING_209.jpeg (1008x654) ΓåÆ staging 14
[Authored] Scaled ΓåÆ 1008x654 (aspect 1.5)
[Authored] Loaded: assets/paintings\PAINTING_210.jpeg (795x1280) ΓåÆ staging 15
[Authored] Scaled ΓåÆ 636x1024 (aspect 0.6)
[Authored] Loaded: assets/paintings\PAINTING_211.jpeg (1035x1600) ΓåÆ staging 16
[Authored] Scaled ΓåÆ 662x1024 (aspect 0.6)
[Authored] Loaded: assets/paintings\PAINTING_212.jpeg (801x1280) ΓåÆ staging 17
[Authored] Scaled ΓåÆ 641x1024 (aspect 0.6)
[Authored] Loaded: assets/paintings\PAINTING_213.jpeg (912x676) ΓåÆ staging 18
[Authored] Scaled ΓåÆ 912x676 (aspect 1.3)
[Authored] Rotated 19 slot(s), 32 valid, disk cursor at 51/57
[Orbs] Configured: count=128 palette=jwst_deep drag=0.4 noise=0.3 rule=brownian rot=0.0 orbital=0.2 tiers=jwst_stars
[Mood] Applied: finite_outdoor (mood=3 outdoor)
[Agents] Spawned 0 for mood 3 around (0.0,0.0)
[AGENTS t=62.8 trigger=mood-transition] 1/32 active, possessed=0 tier:{worker=1} drv:{player=1}
[CENSUS t=   62.8 trigger=mood-transition]
  fam    active  claimed   delta     new
  pyr         0        0       0       0
  arch        0        0       0       0
  col         0        0       0       0
  ant         0        0       0       0
  palm        0        0       0       0
  cact        0        0       0       0
  blad        0        0       0       0
  sph         0        ΓÇö       ΓÇö       ΓÇö
  ribn        0        0       0       0
  cube        0        ΓÇö       ΓÇö       ΓÇö
  gol         0        0       0       0
  gall        0        0       0       0
  TOTAL       0        0       0       0    footprints 0/128
  fam      live   hi-wtr     cap  portal
  pyr         0        0       8       ΓÇö
  arch        0        0      16       0
  col         0        0      16       ΓÇö
  ant         0        0      16       ΓÇö
  palm        0        0      24       ΓÇö
  cact        0        0      20       ΓÇö
  blad        0        0      32       ΓÇö
  sph         0        0       8       ΓÇö
  ribn        0        0       1       ΓÇö
  cube        0        0     256       ΓÇö
  gol         0        0       8       ΓÇö
  gall        0        0      48       ΓÇö
[World] Teardown complete, seed=3437462692 mode=finite 7x7
[Portal] Back-portal spawned at (57.9,192.0) rot=-1.6 slot=0 -> return seed=1898512436 mood=indoor_vault
[Portal] Forward portal 1 at (26.3,-142.0) -> seed=1503216848 mood=open_sunset open
[Portal] Forward portal 2 at (9.8,192.0) -> seed=619369256 mood=open_sunset open
[Portal] Forward portal 3 at (-142.0,19.7) -> seed=1519938578 mood=finite_outdoor FINITE
[Portal] Finite world: 3 forward portals + 1 back-portal
[Ribbon] SPAWN slot=0 at (29.8, -23.7) tier=0 len=563.3 near=(0,-1) far=(3,-12)
[Gallery] slot=1 at (33.3,130.1) host=(0,2) arch=0 paintings=3/3 type=snap
[GoL] Conway slot=0 node=(1,0) corner=(140.6,21.9) host=(3,1) HEIGHT period=4.1
[Orbs] Init dispatched: 128 orbs, 2 workgroups
[Ground] zone rects in core: 1
[Ground] zones active anywhere: 1
[Photographer] Capture -> layer 18 (Panoramic) aspect=2.2 pool=19/32
[Photographer] Rendering snapshot -> layer 18
[Photographer] Capture -> layer 19 (Close-up) aspect=1.4 pool=20/32
[Photographer] Rendering snapshot -> layer 19
[Photographer] Capture -> layer 20 (Portrait) aspect=0.7 pool=21/32
[Photographer] Rendering snapshot -> layer 20
[Photographer] Capture -> layer 21 (Panoramic) aspect=2.1 pool=22/32
[Photographer] Rendering snapshot -> layer 21
[Photographer] Capture -> layer 22 (Panoramic) aspect=2.2 pool=23/32
[Photographer] Rendering snapshot -> layer 22
[Photographer] Capture -> layer 23 (Close-up) aspect=1.5 pool=24/32
[Photographer] Rendering snapshot -> layer 23
[Photographer] Capture -> layer 24 (Panoramic) aspect=2.3 pool=25/32
[Photographer] Rendering snapshot -> layer 24
[Photographer] Capture -> layer 25 (Panoramic) aspect=2.0 pool=26/32
[Photographer] Rendering snapshot -> layer 25
[Photographer] Capture -> layer 26 (Cinematic) aspect=2.1 pool=27/32
[Photographer] Rendering snapshot -> layer 26
[Photographer] Capture -> layer 27 (Low Angle) aspect=1.5 pool=28/32
[Photographer] Rendering snapshot -> layer 27
[Photographer] Capture -> layer 28 (Cinematic) aspect=2.4 pool=29/32
[Photographer] Rendering snapshot -> layer 28
[Photographer] Capture -> layer 29 (Panoramic) aspect=1.8 pool=30/32
[Photographer] Rendering snapshot -> layer 29
[Photographer] Capture -> layer 30 (Bird's Eye) aspect=1.2 pool=31/32
[Photographer] Rendering snapshot -> layer 30
[Photographer] Capture -> layer 31 (Panoramic) aspect=2.1 pool=32/32
[Photographer] Rendering snapshot -> layer 31
[Photographer] Capture -> layer 0 (Panoramic) aspect=2.3 pool=32/32
[Photographer] Rendering snapshot -> layer 0
[Photographer] Capture -> layer 1 (Panoramic) aspect=1.8 pool=32/32
[Photographer] Rendering snapshot -> layer 1
[Photographer] Capture -> layer 2 (Close-up) aspect=1.4 pool=32/32
[Photographer] Rendering snapshot -> layer 2
[Incubator] Shutdown

C:\dev\7t\out\build\the-board-full-release-meter\incubator_dual.exe (process 2628) exited with code 0 (0x0).
To automatically close the console when debugging stops, enable Tools->Options->Debugging->Automatically close the console when debugging stops.
Press any key to close this window . . .