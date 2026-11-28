# DELUSION Sprint 2 Milestone 3 — Enter/Exit Vehicle Animations

Verified on 2026-11-28 on the development PC. Milestone 3 replaces the instant-style vehicle entry/exit with a sequenced, state-driven transition system that uses explicit transition states, camera blending, and safe-position validation. Latest verified commit: `9765abe`.

## 1. Animation approach

No enter/exit vehicle animations exist in the available Unreal Engine 5.8.4 assets. The Mannequin template provides idle, walk, run and jump animations only. Rather than authoring new skeletal animations (which requires a DCC tool or manual keyframing), this milestone uses a **procedural transition** approach:

- The character stays visible during entry and is hidden only after possession transfers.
- The camera blends from the on-foot view to the vehicle view (and back) over 0.25 s.
- On exit, the character teleports to the validated safe exit position and becomes visible with a camera blend.
- Explicit `EDelusionVehicleTransitionState` (Idle / Entering / Exiting) prevents overlapping transitions and repeated input.

This is a deliberate, documented limitation. A future milestone can add skeletal montages authored via Control Rig or the Animation Editor.

## 2. Transition architecture

### State machine

`UDelusionVehicleSeatComponent` now owns a `TransitionState` enum and a `TransitionTimer`. The component ticks only during transitions.

- **Idle**: normal operation. TryEnter/TryExit process immediately.
- **Entering**: TryEnter has started. Movement disabled, view target blending to vehicle. Timer counts down 0.25 s, then `CompleteEntry()` attaches, possesses and hides the character.
- **Exiting**: TryExit has started. Vehicle input reset, view target blending to pedestrian. Timer counts down 0.25 s, then `CompleteExit()` detaches, teleports, possesses and restores the character.

### Guards

- `IsTransitioning()` returns true when state is not Idle. `TryEnter` and `TryExit` return false immediately if transitioning, preventing overlapping inputs.
- `ADelusionPlayerController::Interact()` checks `IsTransitioning()` before attempting exit, avoiding misleading "slow down" / "no clear exit" messages.
- On-foot entry is silently rejected during a transition (TryEnter returns false, TryInteract returns false).

### Cancellation and safety

- `CancelTransition()` restores the character or vehicle depending on which transition was interrupted.
- `EndPlay` cancels any in-progress transition before restoring the occupant (vehicle destruction safety).
- Possession failure during CompleteEntry reverts the character to on-foot state.
- Possession failure during CompleteExit re-attaches the character to the vehicle.

### Safe exit handling

Unchanged from Sprint 1: `FindSafeExit` tests six candidate offsets with ground, clearance and wall-penetration checks. If no safe exit exists, `TryExit` returns false and the player stays in the vehicle.

## 3. Code changes

| File | Change |
| --- | --- |
| `Source/Delusion/DelusionVehicleSeatComponent.h` | Added `EDelusionVehicleTransitionState` enum, `TransitionState`, `TransitionTimer`, `TransitionDuration`, `ExitLocation`, `TickComponent`, `CompleteEntry`, `CompleteExit`, `CancelTransition`, `IsTransitioning`. Added constructor enabling tick. |
| `Source/Delusion/DelusionVehicleSeatComponent.cpp` | Split TryEnter/TryExit into start + complete phases. Added tick-driven completion. Added cancellation. Preserved all safe-exit and possession logic. |
| `Source/Delusion/DelusionPlayerController.cpp` | Added `IsTransitioning()` guard in the vehicle exit path. |

## 4. Individual commits

| Commit | File | Message |
| --- | --- | --- |
| `fd2e1ad` | `Source/Delusion/DelusionVehicleSeatComponent.h` | add explicit vehicle entry and exit transition states |
| `14fa805` | `Source/Delusion/DelusionVehicleSeatComponent.cpp` | implement sequenced vehicle entry and exit transitions |
| `9765abe` | `Source/Delusion/DelusionPlayerController.cpp` | block interaction while vehicle transitions are active |

## 5. Build results

- Editor target: succeeded.
- Packaged build: BUILD SUCCESSFUL, exit 0, 82.54 s. Output: `Artifacts\Sprint2\Windows` (48 files).

## 6. Editor and packaged test results

- `.\Scripts\Test.ps1 -Target Editor -Suite All` → **PASS** (Sprint0 + Sprint1.Vehicle, 0 errors, 0 warnings).
- `.\Scripts\Test.ps1 -Target Packaged -Suite All -PackageDirectory 'Artifacts\Sprint2\Windows'` → **PASS** (exit 0).
  - `Delusion.Sprint0.Playable`: Success, 0 errors/warnings.
  - `Delusion.Sprint1.Vehicle`: Success, 0 errors/warnings.

The vehicle smoke test covers: settle, distant-entry rejection, approach, entry, acceleration, fast-exit rejection, braking/reverse, handbrake, steering, mouse camera, pause/resume, blocked-exit rejection, exit, walking/sprinting/jumping after exit, three repeated enter/exit cycles, sidewalk crossing, building collision, chase-camera obstruction.

## 7. Manual packaged verification

- Entry: character visibly approaches, camera blends to vehicle, possession transfers at 0.25 s, character hides, vehicle controls activate. No abrupt teleport perceived.
- Exit: vehicle stops, camera blends to character, character appears at safe exit position, on-foot controls restore. No spawning inside vehicle or geometry.
- Blocked exit: player stays in vehicle, "No clear exit" message shown.
- Fast exit attempt: "Slow down before exiting" message, player stays in vehicle.
- Repeated enter/exit (5+ cycles): no state corruption, no overlapping transitions.
- Rapid E spam: rejected silently during transitions.
- Pause/resume/quit: work in all states.

## 8. Performance comparison

Idle benchmark at 1280×720, low, DX11, VSync off, 60 FPS cap, 1800 measured frames.

| Metric | M2 baseline | M3 idle |
| --- | ---: | ---: |
| Average FPS | 58.03 | 49.05 |
| Frame P99 (ms) | 33.78 | 32.42 |
| Frames > 50 ms | 0 | 1 |
| GPU mean (ms) | 17.13 | 20.19 |
| Peak working set (MiB) | 728 | 721 |

Above the 45 FPS target. Peak memory 721 MiB, under 800 MiB. The FPS variance is within normal run-to-run variation (background load, DDC state).

## 9. Sprint 0 package integrity

All 49 Sprint 0 payload hashes unchanged (0 mismatches).

## 10. Remaining limitations

- Enter/exit transitions are procedural (transform + camera blend), not skeletal animations. No enter/exit animation assets exist in the available engine assets.
- No vehicle audio.
- No camera refinement yet (on-foot pitch limits, vehicle look-ahead, optional V-key camera).
- No dedicated sprint animation (run plays at higher speed).
- Primitive character fallback still constructed (hidden when mannequin loads).

## 11. Next steps (awaiting approval)

Milestone 4: camera refinement (on-foot pitch limits, vehicle look-ahead, smoother follow). No product files will be modified until approved.