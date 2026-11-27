# DELUSION Sprint 1 delivery

Verified on 2026-11-27 on the development PC. Sprint 1 adds one genuinely drivable Chaos vehicle to the existing Sprint 0 neighborhood and keeps the original Sprint 0 package intact. Latest verified commit: `e647fd2`. Sprint 0 package payload hashes (49 files) were rechecked and are unchanged.

## Run the game

Launch the Sprint 1 package:

```text
D:\personal projects\delusion\Artifacts\Sprint1\Windows\Delusion.exe
```

Keep the entire packaged folder together. The executable and cooked data are local ignored build outputs.

On foot:

| Input | Action |
| --- | --- |
| W / A / S / D | Walk |
| Mouse | Rotate the third-person camera |
| Left Shift | Sprint |
| Space | Jump |
| E | Enter the nearby vehicle when its prompt appears |
| Escape | Pause or resume |

Driving:

| Input | Action |
| --- | --- |
| W | Accelerate |
| S | Brake / reverse |
| A / D | Steer |
| Space | Handbrake |
| Mouse | Rotate the chase camera |
| E | Exit safely when stopped |
| Escape | Pause or resume |

The player spawns near the marked road. Walk to the greybox car beside the street and press E to enter.

## Implemented files and systems

- `Delusion.uproject`: standard Chaos Vehicles plugin enabled; Enhanced Input retained.
- `Source/Delusion/DelusionVehicleWheel.h/.cpp`: front and rear Chaos wheel classes with grip, suspension, braking, steering and handbrake settings.
- `Source/Delusion/DelusionVehicle.h/.cpp`: four-wheel Chaos vehicle pawn, imported sports-car rig and physics asset, greybox bodywork and wheels, chase camera with collision, and the `IDelusionInteractable` entry point.
- `Source/Delusion/DelusionVehicleInputComponent.h/.cpp`: driver-only Enhanced Input mapping for throttle, brake, steering, handbrake and camera; clears on exit.
- `Source/Delusion/DelusionVehicleSeatComponent.h/.cpp`: preserves the on-foot pawn, possession transfer, safe exit search, occupancy checks and cleanup on owner removal.
- `Source/Delusion/DelusionPlayerController.h/.cpp` and `DelusionHUDWidget.h/.cpp`: context-aware interaction, vehicle speed and control hints, pause/resume and safe exit messaging.
- `Source/Delusion/DelusionGameMode.h/.cpp`: spawns one prototype car beside the starting street without duplicating a designer-placed car.
- `Source/Delusion/Tests/DelusionVehicleSmoke.cpp`: editor and packaged gameplay coverage for physics, entry/exit, driving controls, camera, pause, collision and repeated cycles.
- `Source/Delusion/Tests/DelusionDriveBenchmark.cpp`: sustained driving benchmark that enters the car and drives a continuous circle.
- `Scripts/Build.ps1` and `Scripts/Test.ps1`: Sprint 1 package path and both gameplay suites.
- `Scripts/Profile.ps1`: Idle and Driving profiling scenarios with CSV parsing made tolerant of malformed trailing rows.
- `THIRD_PARTY_NOTICES.md`: Epic attribution for the installed vehicle template assets.
- `Content/Vehicles/SportsCar/*`: imported skeleton, physics asset and skeletal rig used by the Chaos vehicle.

## Verification results

| Check | Actual result and evidence |
| --- | --- |
| Editor C++ target | UBT succeeded using MSVC 14.50 and Windows SDK 10.0.26100.0; [build log](Saved/Logs/Build-Editor.log). |
| Editor gameplay tests | `Delusion.Sprint0.Playable` and `Delusion.Sprint1.Vehicle` passed in the editor on 2026-11-27; report `Saved/Automation/Editor-cf4e92b304a8420894011bb21f534bba/index.json`. The vehicle test logged rest Z 25.87 cm, chassis mass 1250 kg, acceleration 3.95 m at 19.82 km/h, braking distance 2.50 m, reverse speed -17.14 km/h and a building stop at X -1302.4 cm. |
| Windows packaging | AutomationTool BuildCookRun completed cooking, staging, packaging and archiving for `Artifacts/Sprint1/Windows`; BUILD SUCCESSFUL, exit 0. Package contains 48 files. |
| Packaged gameplay tests | `.\Scripts\Test.ps1 -Target Packaged -Suite All` returned PASS and exit 0. `Delusion.Sprint0.Playable` (8.9 s) and `Delusion.Sprint1.Vehicle` (31.6 s) both recorded Success with 0 errors and 0 warnings; report `Saved/Automation/Packaged-fb65dbfde4034c6d91fc26b5d7bdb3f6/index.json`. |
| Sustained driving benchmark | `Perf.DelusionDrive` passed in the packaged build with 0 errors; report `Saved/Automation/PackagedDrive-*/index.json`. |
| Sprint 0 package integrity | All 49 recorded Sprint 0 payload hashes remain unchanged. |

The vehicle test exercises native input and Chaos physics: settle, distant-entry rejection, approach, entry, acceleration, fast-exit rejection, braking and reverse, handbrake, right/left steering, steering at speed, mouse camera, pause/resume, blocked exit rejection, exit, walking, sprinting and jumping after exit, three repeated enter/exit cycles, a sidewalk crossing and a building collision, plus chase-camera obstruction.

## Performance

Measured with `Scripts/Profile.ps1` at 1280x720, low scalability, DX11, VSync off, 60 FPS cap, 600 warm-up frames then 1800 measured frames. Sprint 0 numbers are from the earlier Sprint 0 baseline on the Sprint 0 package; Sprint 1 numbers are from the Sprint 1 package.

| Measurement | Sprint 0 baseline | Sprint 1 idle | Sprint 1 driving |
| --- | ---: | ---: | ---: |
| Average FPS | 59.96 | 49.70 | 49.85 |
| Frame mean (ms) | 16.68 | 20.12 | 20.06 |
| Frame P50 (ms) | 16.70 | 17.50 | 17.53 |
| Frame P95 (ms) | 18.12 | 30.42 | 30.34 |
| Frame P99 (ms) | 18.85 | 32.64 | 32.52 |
| Frame max (ms) | 22.42 | 50.24 | 48.65 |
| Frames over 33.3 ms | 0 | 14 | 11 |
| Frames over 50 ms | 0 | 1 | 0 |
| Game thread mean (ms) | 3.83 | 4.95 | 5.67 |
| Render thread mean (ms) | 5.48 | 6.82 | 7.00 |
| GPU mean (ms) | 16.38 | 19.36 | 19.65 |
| Peak working set (MiB) | 723 | 710 | 707 |
| Peak private (MiB) | not recorded | 588 | 584 |

The Sprint 1 package carries the vehicle, its greybox bodywork and the Chaos simulation. The GPU is now the bottleneck at roughly 19.4 ms, which holds the frame rate near 50 FPS instead of the 60 FPS cap. This is above the 30 FPS Sprint 1 target at 720p low settings. The idle and driving results are close, so the sustained driving workload does not collapse frame rate. One idle frame exceeded 50 ms; the driving run had none.

## Storage

| Location | Free space |
| --- | ---: |
| C: | 23.98 GiB |
| D: | 133.50 GiB |

No C: cleanup was performed. All generated outputs remain under `D:\personal projects\delusion` and are ignored by Git.

## Remaining limitations and next milestone

The vehicle is a greybox Chaos pawn with primitive bodywork and no animations, audio or damage model. Sprint 0 character art remains an unanimated primitive. Sustained long-session stability, Shipping builds, other PCs and higher resolutions are not validated. The Sprint 0 package remains unchanged and playable.

The next recommended milestone is Sprint 2: character and vehicle animation state machines, vehicle enter/exit animations, improved camera behavior, vehicle sound and presentation.

## Individual implementation commits and pushes

Each commit below changed exactly one file and was pushed successfully to `origin main` before the next tracked file changed. Author and committer were verified as `Madhavan <175943252+MAD1IZJOD@users.noreply.github.com>`.

| Commit | File | Message |
| --- | --- | --- |
| [44adff7](https://github.com/MAD1IZJOD/delusion/commit/44adff7) | Scripts/Build.ps1 | keep sprint one packages separate from the working build |
| [f1592da](https://github.com/MAD1IZJOD/delusion/commit/f1592da) | THIRD_PARTY_NOTICES.md | record the vehicle template asset license |
| [ddf47c6](https://github.com/MAD1IZJOD/delusion/commit/ddf47c6) | Content/Vehicles/SportsCar/SK_SportsCar.uasset | add the vehicle template skeleton |
| [0f19363](https://github.com/MAD1IZJOD/delusion/commit/0f19363) | Content/Vehicles/SportsCar/PA_SportsCar.uasset | add the vehicle chassis physics asset |
| [d6834db](https://github.com/MAD1IZJOD/delusion/commit/d6834db) | Content/Vehicles/SportsCar/SKM_SportsCar.uasset | add the four wheel skeletal vehicle rig |
| [b31542d](https://github.com/MAD1IZJOD/delusion/commit/b31542d) | Delusion.uproject | enable standard chaos vehicle simulation |
| [a2299d8](https://github.com/MAD1IZJOD/delusion/commit/a2299d8) | Source/Delusion/Delusion.Build.cs | link the chaos vehicles gameplay module |
| [65bd12d](https://github.com/MAD1IZJOD/delusion/commit/65bd12d) | Source/Delusion/DelusionVehicleWheel.h | define front and rear chaos wheels |
| [794b94b](https://github.com/MAD1IZJOD/delusion/commit/794b94b) | Source/Delusion/DelusionVehicleWheel.cpp | configure wheel grip suspension and brakes |
| [74f58fc](https://github.com/MAD1IZJOD/delusion/commit/74f58fc) | Source/Delusion/DelusionVehicleInputComponent.h | define driver input context management |
| [0ee03cb](https://github.com/MAD1IZJOD/delusion/commit/0ee03cb) | Source/Delusion/DelusionVehicleInputComponent.cpp | route driving controls through enhanced input |
| [95654f4](https://github.com/MAD1IZJOD/delusion/commit/95654f4) | Source/Delusion/DelusionVehicleSeatComponent.h | define vehicle entry and safe exit state |
| [1d17e42](https://github.com/MAD1IZJOD/delusion/commit/1d17e42) | Source/Delusion/DelusionVehicleSeatComponent.cpp | preserve the player and validate vehicle exits |
| [65456a3](https://github.com/MAD1IZJOD/delusion/commit/65456a3) | Source/Delusion/DelusionVehicle.h | define the interactable chaos vehicle pawn |
| [d2ff2fa](https://github.com/MAD1IZJOD/delusion/commit/d2ff2fa) | Source/Delusion/DelusionVehicle.cpp | add the four wheel chaos car and chase camera |
| [f6ac30d](https://github.com/MAD1IZJOD/delusion/commit/f6ac30d) | Source/Delusion/DelusionCharacter.h | declare on foot input cleanup on possession changes |
| [0073904](https://github.com/MAD1IZJOD/delusion/commit/0073904) | Source/Delusion/DelusionCharacter.cpp | remove on foot mappings while driving |
| [ee776a7](https://github.com/MAD1IZJOD/delusion/commit/ee776a7) | Source/Delusion/DelusionInteractionComponent.cpp | include vehicles in nearby interaction detection |
| [4aad9cb](https://github.com/MAD1IZJOD/delusion/commit/4aad9cb) | Source/Delusion/DelusionGameMode.h | declare prototype vehicle spawning |
| [85fabe8](https://github.com/MAD1IZJOD/delusion/commit/85fabe8) | Source/Delusion/DelusionGameMode.cpp | spawn one car beside the starting street |
| [80807c9](https://github.com/MAD1IZJOD/delusion/commit/80807c9) | Source/Delusion/DelusionHUDWidget.h | extend the hud for driving hints and feedback |
| [0a55a07](https://github.com/MAD1IZJOD/delusion/commit/0a55a07) | Source/Delusion/DelusionHUDWidget.cpp | show vehicle speed and context aware control hints |
| [b1dd453](https://github.com/MAD1IZJOD/delusion/commit/b1dd453) | Source/Delusion/DelusionPlayerController.h | connect vehicle exit and pause to the existing controller |
| [be0144a](https://github.com/MAD1IZJOD/delusion/commit/be0144a) | Scripts/Test.ps1 | test both gameplay suites in separate sprint packages |
| [e882213](https://github.com/MAD1IZJOD/delusion/commit/e882213) | Source/Delusion/DelusionVehicle.cpp | use the supported chaos input configuration api |
| [9c35554](https://github.com/MAD1IZJOD/delusion/commit/9c35554) | Source/Delusion/Tests/DelusionVehicleSmoke.cpp | exercise driving possession and safe exits at runtime |
| [28694f6](https://github.com/MAD1IZJOD/delusion/commit/28694f6) | Source/Delusion/Tests/DelusionVehicleSmoke.cpp | include wheel definitions in the gameplay test |
| [a5bd5df](https://github.com/MAD1IZJOD/delusion/commit/a5bd5df) | Scripts/Profile.ps1 | capture repeatable packaged performance measurements |
| [8f78605](https://github.com/MAD1IZJOD/delusion/commit/8f78605) | Source/Delusion/Tests/DelusionVehicleSmoke.cpp | verify chassis mass and chase camera obstruction |
| [39b7683](https://github.com/MAD1IZJOD/delusion/commit/39b7683) | Source/Delusion/Tests/DelusionVehicleSmoke.cpp | fix chassis body lookup in vehicle smoke test |
| [f84058d](https://github.com/MAD1IZJOD/delusion/commit/f84058d) | Source/Delusion/Tests/DelusionDriveBenchmark.cpp | add a sustained driving benchmark test |
| [9060c4d](https://github.com/MAD1IZJOD/delusion/commit/9060c4d) | Scripts/Profile.ps1 | support driving scenarios in the profiler |
| [e647fd2](https://github.com/MAD1IZJOD/delusion/commit/e647fd2) | Scripts/Profile.ps1 | make the profiler tolerant of ue exit codes |

Wait, I made an error in the last three rows of the commit table: the commit messages don't match the files. Let me check:
- 9060c4d: "support driving scenarios in the profiler" — file was Scripts/Profile.ps1, not DelusionPlayerController.cpp.
- e647fd2: "make the profiler tolerant of ue exit codes" — file was Scripts/Profile.ps1, not DelusionVehicleSmoke.cpp.
- f84058d: "add a sustained driving benchmark test" — file was Source/Delusion/Tests/DelusionDriveBenchmark.cpp. Correct.

Also, 9060c4d's message says "support driving scenarios in the profiler" and file is Scripts/Profile.ps1. I incorrectly attributed it to DelusionPlayerController.cpp. And e647fd2's message and file similarly.

Let me fix those two rows. Also, the commit `b1dd453` file is DelusionPlayerController.h (correct). And `0a55a07` is DelusionHUDWidget.cpp (correct). And `80807c9` is DelusionHUDWidget.h (correct).

Let me correct the table rows for 9060c4d and e647fd2.<|end|><|start|>start_of_turn<|channel|>assistant to=edit<|channel|>analysis <|constrain|>xml<|message|><rfl:parameter name="path">
D:\personal projects\delusion\SPRINT1_REPORT.md