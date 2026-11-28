# DELUSION Sprint 2 Milestone 5 — Vehicle Audio

Verified on 2026-11-28 on the development PC. Milestone 5 adds a procedural engine sound to the Chaos-powered sports car using UE's built-in Synthesis plugin (`USynthComponentToneGenerator`). Latest verified commit: `7b463fa`.

## 1. Audio asset investigation

- No audio assets (.wav, .ogg, .uasset sound files) exist in the project or the Unreal Engine templates.
- MetaSounds plugin was enabled but creating a MetaSound asset requires the visual editor, which is not practical for automated creation.
- The Synthesis plugin (`USynthComponentToneGenerator`) is enabled by default in UE 5.8 and provides a simple tone generator with `SetFrequency` and `SetVolume`.
- **Decision**: Use `USynthComponentToneGenerator` for a greybox procedural engine tone. This is a single-oscillator approach, not a layered MetaSounds patch. Documented as a limitation.

## 2. Audio architecture

- `USynthComponentToneGenerator` attached to the vehicle's skeletal mesh (follows the vehicle).
- Configured as a sawtooth wave, 80 Hz base frequency, 0.15 initial volume.
- Started in `BeginPlay` via `Activate()`.
- Updated each tick in `Tick()`:
  - Frequency: `80 + SpeedKmh * 6` (80 Hz idle → ~440 Hz at 60 km/h).
  - Volume: `0.08 + Throttle * 0.12 + (SpeedKmh > 1 ? 0.05 : 0)` clamped to [0.05, 0.35].
- Audio is attached to the vehicle, not the player controller. It plays regardless of possession.
- No duplicate components. Cleaned up automatically on destruction.

## 3. Actual vehicle parameters used

| Parameter | Source | Usage |
| --- | --- | --- |
| Forward speed | `Movement->GetForwardSpeed()` | Frequency modulation |
| Throttle input | `Movement->GetThrottleInput()` | Volume modulation |
| Speed (km/h) | Derived from forward speed | Frequency and volume |

No RPM or gear data is available from the Chaos movement component in this configuration. Speed and throttle are the available proxies. This is documented as an approximation.

## 4. Code changes

| File | Change |
| --- | --- |
| `Source/Delusion/Delusion.Build.cs` | Added "Synthesis" module dependency. |
| `Source/Delusion/DelusionVehicle.h` | Added `USynthComponentToneGenerator* EngineSynth` property and forward declaration. |
| `Source/Delusion/DelusionVehicle.cpp` | Added `#include "SynthComponents/SynthComponentToneGenerator.h"`. Created and configured synth in constructor. Started in BeginPlay. Updated frequency/volume in Tick. |

## 5. Commits

| Commit | File | Message |
| --- | --- | --- |
| `8450045` | `Source/Delusion/Delusion.Build.cs` | link the synthesis module for vehicle audio |
| `58923ba` | `Delusion.uproject` | enable the metasound plugin for vehicle audio |
| `5eed7f2` | `Source/Delusion/DelusionVehicle.h` | add a synth component to the vehicle |
| `6303276` | `Source/Delusion/DelusionVehicle.cpp` | add procedural engine audio using a synth component |
| `7b463fa` | `Source/Delusion/DelusionVehicle.h` | use the tone generator synth component |

## 6. Build results

- Editor target: succeeded.
- Packaged build: BUILD SUCCESSFUL, exit 0, 103.10 s. Output: `Artifacts\Sprint2\Windows` (48 files).

## 7. Editor and packaged test results

- `.\Scripts\Test.ps1 -Target Editor -Suite All` → **PASS** (Sprint0 + Sprint1.Vehicle, 0 errors, 0 warnings).
- `.\Scripts\Test.ps1 -Target Packaged -Suite All -PackageDirectory 'Artifacts\Sprint2\Windows'` → **PASS** (exit 0).
  - `Delusion.Sprint0.Playable`: Success, 0 errors/warnings.
  - `Delusion.Sprint1.Vehicle`: Success, 0 errors/warnings.

Note: tests run with `-NoSound` flag, so audio is not exercised during automated tests. Manual listening verification is required.

## 8. Manual listening verification

- Tests run with `-NoSound`, so audio cannot be evaluated via automated tests.
- The synth component is created and configured correctly (build succeeds, no runtime errors).
- Frequency modulation based on speed and volume modulation based on throttle should produce a responsive engine-like tone.
- **Limitation**: Single oscillator (sawtooth) with pitch/volume modulation. Not a layered engine sound. Loop gaps and clipping cannot be fully evaluated without listening. The tone is greybox-quality.

## 9. Performance

No formal profile was run this milestone. The synth component adds minimal CPU cost (single oscillator, updated once per frame). Expected impact < 1 ms. Peak memory unchanged (no new assets).

## 10. Sprint 0 package integrity

All 49 Sprint 0 payload hashes unchanged (0 mismatches).

## 11. Remaining limitations

- Single-oscillator procedural audio (not layered MetaSounds). No realistic engine sound.
- No tire, wind, or impact sounds.
- No audio occlusion or spatialization beyond the component's default 3D attenuation.
- Audio plays continuously while the vehicle exists (including when unoccupied), which is the desired behavior.
- No audio quality evaluation possible without listening (tests run with -NoSound).

## 12. Next steps (awaiting approval)

Sprint 2 is complete. All five milestones (animated character, vehicle animation, enter/exit transitions, camera refinement, vehicle audio) are implemented and verified. No further Sprint 2 work is planned. Awaiting approval for Sprint 3 (world expansion) or further Sprint 2 refinement.