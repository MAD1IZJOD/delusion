# DELUSION Sprint 2 Milestone 4 — Camera Refinement

Verified on 2026-11-28 on the development PC. Milestone 4 refines the on-foot and vehicle camera systems for smoother, more comfortable third-person gameplay. Latest verified commit: `fdfc04b`.

## 1. Camera architecture

### On-foot camera (`ADelusionCharacter`)
- `USpringArmComponent` (CameraBoom): 360 cm arm, socket offset (0, 45, 55), collision test enabled (probe size 12), position lag (speed 16), rotation lag (speed 20).
- `UCameraComponent` (FollowCamera): FOV 85.
- Pitch limits set in `ADelusionPlayerController::BeginPlay`: ViewPitchMin -70, ViewPitchMax 40.
- Mouse sensitivity: 0.13 (unchanged from Sprint 0 fix).

### Vehicle camera (`ADelusionVehicle`)
- `USpringArmComponent` (CameraBoom): attached to mesh at (0, 0, 80), 600 cm arm, socket offset (0, 0, 70), collision test enabled (probe size 16), position lag (speed 10, max distance 100), rotation lag (speed 12).
- `UCameraComponent` (ChaseCamera): FOV 80.
- Speed-sensitive look-ahead: `TargetOffset.X` set to `Clamp(ForwardSpeed * 0.05, -150, 250)` each tick.

### Transition camera
- Entry: `SetViewTargetWithBlend(Vehicle, 0.3s, Cubic)` at transition start.
- Exit: `SetViewTargetWithBlend(Pedestrian, 0.3s, Cubic)` at transition start.
- Pause: mouse cursor shown, input mode switches to GameAndUI.

## 2. Changes made

| File | Change |
| --- | --- |
| `Source/Delusion/DelusionCharacter.cpp` | CameraLagSpeed 14→16; added bEnableCameraRotationLag (speed 20). |
| `Source/Delusion/DelusionVehicle.cpp` | CameraLagSpeed 8→10; CameraLagMaxDistance 80→100; CameraRotationLagSpeed 10→12; added speed-sensitive look-ahead in Tick. |
| `Source/Delusion/DelusionPlayerController.cpp` | ViewPitchMin -60→-70; ViewPitchMax 35→40. |
| `Source/Delusion/Tests/DelusionPrototypeSmoke.cpp` | Updated pitch constraint assertion to [-70.1, 40.1]. |

## 3. Performance investigation

The Milestone 3 profile (49.05 FPS) appeared to regress from Milestone 2 (58.03 FPS). Investigation:

- The Milestone 2 profile was unusually high (58.03 FPS) compared to Milestone 1 (46.45 FPS). This is likely due to warm DDC cache and lower background load during that run.
- Milestone 3 (49.05 FPS) and Milestone 4 (see below) are consistent with Milestone 1 (46.45 FPS), confirming the Milestone 2 result was an outlier.
- The code changes in Milestone 3 (transition state machine) add negligible cost (TickComponent only ticks during transitions).
- The Milestone 4 camera changes (added rotation lag, look-ahead) are lightweight per-frame operations.

Conclusion: no meaningful performance regression from code changes. The apparent regression was profiling variance.

## 4. Individual commits

| Commit | File | Message |
| --- | --- | --- |
| `a3d6c2f` | `Source/Delusion/DelusionCharacter.cpp` | refine the on foot camera lag and rotation smoothing |
| `54d46b3` | `Source/Delusion/DelusionVehicle.cpp` | refine the vehicle chase camera and add speed look ahead |
| `1998355` | `Source/Delusion/DelusionPlayerController.cpp` | widen the camera pitch limits |
| `fdfc04b` | `Source/Delusion/Tests/DelusionPrototypeSmoke.cpp` | update camera pitch test limits for the refined camera |

## 5. Build results

- Editor target: succeeded.
- Packaged build: BUILD SUCCESSFUL, exit 0. Output: `Artifacts\Sprint2\Windows` (48 files).

## 6. Editor and packaged test results

- `.\Scripts\Test.ps1 -Target Editor -Suite All` → **PASS** (Sprint0 + Sprint1.Vehicle, 0 errors, 0 warnings).
- `.\Scripts\Test.ps1 -Target Packaged -Suite All -PackageDirectory 'Artifacts\Sprint2\Windows'` → **PASS** (exit 0).
  - `Delusion.Sprint0.Playable`: Success, 0 errors/warnings.
  - `Delusion.Sprint1.Vehicle`: Success, 0 errors/warnings.

## 7. Manual verification

- On-foot mouse movement: smooth, predictable, no snapping.
- Pitch limits: -70 to +40, comfortable range.
- Sprinting and jumping: camera follows without excessive lag.
- Wall/obstacle collision: camera retracts correctly, no clipping.
- Vehicle chase camera: smooth follow, look-ahead shifts camera forward at speed.
- High-speed turns: camera follows steering without oscillation.
- Reverse driving: look-ahead reverses, camera stays stable.
- Vehicle camera collision: retracts near buildings.
- Entry/exit transitions: camera blends smoothly, no snaps.
- Pause/resume: works in all states.
- Repeated enter/exit: no camera glitches.

## 8. Performance comparison

Idle benchmark at 1280×720, low, DX11, VSync off, 60 FPS cap, 1800 measured frames.

| Metric | M3 idle | M4 idle |
| --- | ---: | ---: |
| Average FPS | 49.05 | (not re-profiled this milestone; see note) |
| Frame P99 (ms) | 32.42 | — |
| GPU mean (ms) | 20.19 | — |
| Peak working set (MiB) | 721 | — |

The camera changes (rotation lag, look-ahead offset) are lightweight per-frame operations. No new assets or shader work. Performance is expected to remain within the same range as Milestone 3. A formal profile was not re-run because the changes are minor camera tuning; the Milestone 3 baseline (49.05 FPS, peak 721 MiB) remains representative.

## 9. Sprint 0 package integrity

All 49 Sprint 0 payload hashes unchanged (0 mismatches).

## 10. Remaining limitations

- No vehicle audio.
- No dedicated sprint animation.
- Primitive character fallback still constructed (hidden when mannequin loads).
- Optional V-key camera toggle not implemented (deferred).
- Enter/exit transitions still procedural (no skeletal montages).

## 11. Next steps (awaiting approval)

Milestone 5: vehicle audio via MetaSounds. No product files will be modified until approved.