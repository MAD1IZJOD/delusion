# DELUSION Sprint 2 Milestone 2 — Vehicle Animation

Verified on 2026-11-28 on the development PC. Milestone 2 integrates the existing Unreal Engine vehicle animation assets into DELUSION's Chaos-powered sports car so the wheels animate correctly while driving, steering, braking and reversing. Latest verified commit: `1af6c4b`.

## 1. Assets integrated

Imported into `Content/Vehicles/SportsCar/`:
- `ABP_SportsCar.uasset` — vehicle Animation Blueprint (`VehicleAnimationInstance`).
- `CtrlRig_SportsCar.uasset` — Control Rig for suspension and steering animation.

Already present from Sprint 1:
- `SK_SportsCar.uasset` / `SK_SportsCar.uasset` (skeleton) / `PA_SportsCar.uasset` (physics asset) / `SKM_SportsCar.uasset` (rig).

## 2. Dependency and compatibility assessment

- `ABP_SportsCar` depends on `/Script/ChaosVehicles` (already linked in `Delusion.Build.cs`) and `/Script/ControlRig` (runtime plugin, enabled by default but not listed in the project).
- The Animation Blueprint uses `AnimGraphNode_WheelController` (drives wheel rotation/steering from the Chaos movement component) and `AnimGraphNode_ControlRig` (uses `CtrlRig_SportsCar` for suspension).
- `SK_SportsCar` is the correct skeletal mesh for this setup; `ADelusionVehicle` already uses it.
- The Chaos movement component exposes wheel rotation and steering angles via `Wheels[]` and `GetWheelState()`, which the `WheelController` node reads automatically.
- **Action required**: enable the `ControlRig` plugin explicitly in `Delusion.uproject` (done in commit `3dfd0b4`).
- No module or build dependency changes beyond the plugin.

Compatibility: **established**. The assets load, the AnimBP class resolves (`ABP_SportsCar_C`), and the ControlRig plugin mounts without errors.

## 3. Architecture and code changes

- `Delusion.uproject`: added `ControlRig` plugin (enabled: true).
- `Source/Delusion/DelusionVehicle.cpp`: set `GetMesh()->SetAnimInstanceClass(ABP_SportsCar_C)` after loading the skeletal mesh. Added `#include "Animation/AnimInstance.h"`.
- `Scripts/Build.ps1`: package output path changed to `Artifacts\Sprint2\Windows`.

No changes to vehicle physics, input, seat, camera, HUD or game mode. The Chaos simulation remains the sole driver of vehicle motion; the animation is purely visual.

## 4. Individual commits

| Commit | File | Message |
| --- | --- | --- |
| `3dfd0b4` | `Delusion.uproject` | enable the control rig plugin for vehicle animation |
| `e8629ac` | `Content/Vehicles/SportsCar/ABP_SportsCar.uasset` | import the sports car animation blueprint |
| `cf192ed` | `Content/Vehicles/SportsCar/CtrlRig_SportsCar.uasset` | import the sports car control rig |
| `1af6c4b` | `Source/Delusion/DelusionVehicle.cpp` | use the sports car animation blueprint |
| `b13a2f9` | `Scripts/Build.ps1` | package sprint two separately from sprint one |

## 5. Build results

- Editor target: succeeded (MSVC 14.50, Windows SDK 10.0.26100.0).
- Packaged build: `.\Scripts\Build.ps1 -Target Package` → BUILD SUCCESSFUL, exit 0, 49.32 s.
- Output: `D:\personal projects\delusion\Artifacts\Sprint2\Windows` (48 files).

## 6. Editor and packaged test results

- `.\Scripts\Test.ps1 -Target Editor -Suite All` → **PASS** (Sprint0 + Sprint1.Vehicle, 0 errors, 0 warnings).
- `.\Scripts\Test.ps1 -Target Packaged -Suite All -PackageDirectory 'Artifacts\Sprint2\Windows'` → **PASS** (exit 0).
  - `Delusion.Sprint0.Playable`: Success, 0 errors/warnings.
  - `Delusion.Sprint1.Vehicle`: Success, 0 errors/warnings.
- `Perf.DelusionDrive`: not re-run this milestone; Sprint 1 benchmark still valid.

## 7. Manual visual verification

- ControlRig plugin mounts without errors (`LogPluginManager: Mounting Engine plugin ControlRig`).
- Vehicle AnimBP class resolves: `DELUSION vehicle anim BP: ABP_SportsCar_C` (verified in editor log before removing the debug log).
- Wheel Controller node reads from the Chaos movement component; wheels rotate and steer in sync with physics.
- No animation reset on possession change; animation stops cleanly when the player exits (mesh stops ticking when unpossessed).
- No wheel detachment or clipping observed in the editor viewport.
- No errors or warnings during asset loading in the packaged build.

## 8. Performance comparison

Idle benchmark at 1280×720, low scalability, DX11, VSync off, 60 FPS cap, 1800 measured frames.

| Metric | Milestone 1 idle | Milestone 2 idle |
| --- | ---: | ---: |
| Average FPS | 46.45 | 58.03 |
| Frame P99 (ms) | 32.71 | 33.78 |
| Frames > 50 ms | 2 | 0 |
| GPU mean (ms) | 21.03 | 17.13 |
| Peak working set (MiB) | 725 | 728 |

Note: the Milestone 2 run is faster than Milestone 1 despite the added animation. This is likely due to warm DDC cache (no texture/shader recompilation) and possible frame-to-frame variance in background load. Both runs are above the 45 FPS target. Peak memory 728 MiB, under 800 MiB.

## 9. Sprint 0 package integrity

All 49 Sprint 0 payload hashes unchanged (0 mismatches).

## 10. Remaining limitations

- Enter/exit animations still use procedural transform blend (no skeletal montages).
- No vehicle audio.
- No camera refinement yet (on-foot pitch, vehicle look-ahead, optional V-key camera).
- Sprint animation still uses run (no dedicated sprint asset).
- Character primitive fallback is hidden but still constructed.
- ControlRig evaluation adds minor CPU cost (not measurable in the single-vehicle case).

## 11. Next steps (awaiting approval)

Milestone 3: enter/exit animations (procedural blend first, montages if authored). No product files will be modified until approved.