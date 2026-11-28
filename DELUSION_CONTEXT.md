# DELUSION context

Operational snapshot for a fresh OpenCode session. `AGENTS.md` remains authoritative for repository rules. This file is a living handover state and is updated under the same one-file/one-commit/one-push workflow.

## Project identity and vision

DELUSION is an original single-player, third-person open-world action game for Windows PC, built in Unreal Engine 5.8.4 with C++ gameplay and Blueprint asset wiring. Long-term ambition is a living city with vehicles, NPCs, traffic, missions, combat, police and weather, built incrementally with tested milestones and a continuously playable Windows build.

## Environment

- Project root: `D:\personal projects\delusion`
- Engine: `D:\Epic Games\UE_5.8` (5.8.4)
- Compiler: MSVC 14.50 (toolchain 14.50.35717, accepted by engine), Windows SDK 10.0.26100.0
- Hardware: Ryzen AI 7 350, Radeon 860M integrated GPU, 24 GB DDR5, Windows 11
- Rendering baseline: DX11, low scalability, ray tracing/Lumen/Nanite/virtual shadows disabled
- Storage: C: ~24 GiB free, D: ~133.5 GiB free (2026-11-27)

## Absolute rules

- Branch `main`, remote `origin` = `https://github.com/MAD1IZJOD/delusion`.
- Every commit author and committer must be exactly `Madhavan <175943252+MAD1IZJOD@users.noreply.github.com>`.
- One modified tracked file = one commit = one successful push before the next file.
- Stage only the exact file path. Never `git add .` or `git add -A`.
- Never force push, reset --hard, rebase/amend published history, or rewrite commits.
- Keep all project outputs under the permanent root; generated packages/binaries/logs are ignored.
- Prefer D: for caches, builds, profiling and temporary files; process-scoped env overrides only.

## Current repository state

- Branch: `main`, in sync with `origin/main`.
- Latest commit: `6b34d0e` record the sprint two milestone two vehicle animation report.
- Working tree: clean.

## Current milestone

Sprint 2 Milestone 2 — Vehicle animation. Objective: integrate the existing ABP_SportsCar and CtrlRig_SportsCar into the Chaos vehicle so wheels animate correctly while driving, steering, braking and reversing. Status: implemented, editor-tested, packaged and runtime-tested; Sprint 0 and Sprint 1 packages unchanged.

## Implemented and verified

- Sprint 0 playable prototype (movement, sprint, jump, camera, collision, interaction, HUD, pause): packaged and tested.
- Chaos Vehicles plugin enabled; standard (non-modular) Chaos vehicle.
- Four-wheel Chaos vehicle pawn with imported sports-car rig/physics asset and greybox bodywork.
- Wheel definitions (front/rear) with grip, suspension, brakes, steering, handbrake.
- Driver input component with Enhanced Input mapping for throttle, brake, steering, handbrake and camera.
- Vehicle seat component preserving the on-foot pawn, possession transfer, safe exit search and cleanup.
- Vehicle spawning beside the starting street.
- Reusable interaction detection includes vehicles (ECC_Vehicle).
- On-foot input mappings removed while driving; restored on exit.
- Context-aware HUD: speed in km/h, driving controls, exit hint, interaction prompt.
- Pause/resume/quit work while driving; vehicle controls reset on pause.
- Chase camera with collision handling.
- Editor build: succeeded.
- Editor tests: `Delusion.Sprint0.Playable` and `Delusion.Sprint1.Vehicle` passed (0 errors, 0 warnings).
- Packaged build: `Artifacts/Sprint1/Windows`, 48 files, BUILD SUCCESSFUL exit 0.
- Packaged tests: both suites passed (Sprint0 8.9 s, Sprint1 31.6 s, 0 errors/warnings).
- Sustained driving benchmark `Perf.DelusionDrive`: passed in packaged build.
- Profiling: idle ~49.70 FPS (P99 32.64 ms, peak 710 MiB) and driving ~49.85 FPS (P99 32.52 ms, peak 707 MiB) at 1280x720 low DX11.
- Sprint 0 package: all 49 payload hashes unchanged.
- Sprint 2 Milestone 1: imported 26 Mannequin assets (skeleton, mesh, physics asset, 9 animations, blend space, AnimBP, materials, textures).
- Sprint 2 Milestone 1: character now uses SK_Mannequin skeletal mesh with ThirdPerson_AnimBP (idle/walk/run/jump state machine).
- Sprint 2 Milestone 1: primitive parts retained as hidden fallback when mannequin loads.
- Sprint 2 Milestone 1: packaged build `Artifacts/Sprint2/Windows`, 48 files, BUILD SUCCESSFUL exit 0.
- Sprint 2 Milestone 1: packaged tests both suites passed (Sprint0 + Sprint1.Vehicle).
- Sprint 2 Milestone 1: profiling idle ~46.45 FPS (P99 32.71 ms, peak 725 MiB) at 1280x720 low DX11.
- Sprint 2 Milestone 2: ControlRig plugin enabled in Delusion.uproject.
- Sprint 2 Milestone 2: imported ABP_SportsCar and CtrlRig_SportsCar into Content/Vehicles/SportsCar/.
- Sprint 2 Milestone 2: vehicle mesh now uses ABP_SportsCar (WheelController + ControlRig nodes).
- Sprint 2 Milestone 2: packaged build `Artifacts/Sprint2/Windows`, 48 files, BUILD SUCCESSFUL exit 0.
- Sprint 2 Milestone 2: packaged tests both suites passed (Sprint0 + Sprint1.Vehicle).
- Sprint 2 Milestone 2: profiling idle ~58.03 FPS (P99 33.78 ms, peak 728 MiB) at 1280x720 low DX11.

## Not implemented / not verified

- Vehicle enter/exit animations (still procedural transform blend).
- Vehicle audio.
- Camera refinement (on-foot and chase) — not yet tuned.
- Character sprint animation (uses run animation; no dedicated sprint asset).
- Traffic, NPCs, missions, combat, police.
- Shipping configuration, other PCs, long-session stability, higher resolutions.
- Driving camera second mode (V) is optional and not implemented.

## Architecture and decisions

- Vehicle simulation owned by `UChaosWheeledVehicleMovementComponent`; input and seat are separate components.
- On-foot pawn is preserved and hidden during driving; possession transfers to the vehicle.
- Context-aware Enhanced Input: controller-level mapping (priority 1) handles E and Escape; character/vehicle mappings at priority 0.
- Safe exit searches six candidate offsets with ground and clearance checks plus a wall-penetration test.
- Standard Chaos Vehicles chosen over Chaos Modular Vehicles.

## Important paths and assets

- Source: `Source/Delusion/` (character, vehicle, wheels, input, seat, controller, HUD, interaction, neighborhood, game mode, tests).
- Map: `Content/Maps/Prototype.umap`.
- Vehicle assets: `Content/Vehicles/SportsCar/{SK,PA,SKM}_SportsCar.uasset`.
- Scripts: `Scripts/Build.ps1`, `Scripts/Test.ps1`, `Scripts/Profile.ps1`.
- Reports: `SPRINT0_REPORT.md`, `SPRINT1_REPORT.md`.
- Packages: `Artifacts/Windows` (Sprint 0), `Artifacts/Sprint1/Windows` (Sprint 1, ignored), `Artifacts/Sprint2/Windows` (Sprint 2, ignored).
- Hash manifest: `Saved/Sprint0PackagePayloadHashes.csv` (ignored).

## Build/test commands and results

- `.\Scripts\Build.ps1 -Target Editor` — succeeded.
- `.\Scripts\Build.ps1 -Target Package` — succeeded, exit 0, `Artifacts\Sprint1\Windows`.
- `.\Scripts\Test.ps1 -Target Editor -Suite Sprint1` — PASS.
- `.\Scripts\Test.ps1 -Target Packaged -Suite All` — PASS (Sprint0 + Sprint1.Vehicle).
- `.\Scripts\Profile.ps1 -Scenario Idle -Label Sprint1Idle` — completed, 1800 frames.
- `.\Scripts\Profile.ps1 -Scenario Driving -Label Sprint1Drive` — completed, 1800 frames.

## Performance and storage

See `SPRINT1_REPORT.md` for the full comparison table. Summary: idle ~49.70 FPS (P99 32.64 ms, peak 710 MiB); driving ~49.85 FPS (P99 32.52 ms, peak 707 MiB) at 1280x720 low DX11. Above the 30 FPS target.

## Known bugs and blockers

- None blocking. Profiler process exit code is 777003 (non-zero) despite clean `LogExit: Exiting.`; CSV and summary are valid. Profiler now warns instead of throwing.
- `Microsoft.VisualBasic.Core` assembly is unavailable under Windows PowerShell; profiler uses `Microsoft.VisualBasic`.
- UE CSV trailing metadata row can throw `MalformedLineException`; profiler now skips malformed lines.
- The optional Fab plugin remains disabled (Windows security policy).

## Next actions

1. Sprint 2 Milestone 3: implement enter/exit animations (procedural blend first, montages if authored).
2. Sprint 2 Milestone 4: camera refinement (on-foot pitch, vehicle look-ahead).
3. Sprint 2 Milestone 5: vehicle audio via MetaSounds.
4. Re-run packaged tests after any gameplay change.

## Recent change log

- `e647fd2` — make the profiler tolerant of ue exit codes — pushed.
- `9060c4d` — support driving scenarios in the profiler — pushed.
- `f84058d` — add a sustained driving benchmark test — pushed.
- `39b7683` — fix chassis body lookup in vehicle smoke test — pushed.
- `8b66684` — record the sprint one delivery report — pushed.
- `86c52e6` — add the delusion project context file — pushed.
- `75d589d` — update the delusion context file for sprint one completion — pushed.
- `0638f36` through `1fe9805` — import 26 Mannequin assets — pushed.
- `920d5f3` — use the mannequin skeletal mesh and animation blueprint — pushed.
- `b13a2f9` — package sprint two separately from sprint one — pushed.
- `484bce0` — update the delusion context file for sprint two milestone one — pushed.
- `3dfd0b4` — enable the control rig plugin for vehicle animation — pushed.
- `e8629ac` — import the sports car animation blueprint — pushed.
- `cf192ed` — import the sports car control rig — pushed.
- `1af6c4b` — use the sports car animation blueprint — pushed.
- `6b34d0e` — record the sprint two milestone two vehicle animation report — pushed.