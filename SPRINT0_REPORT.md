# DELUSION Sprint 0 delivery

Verified on 2026-11-25 on the development PC. The first playable Windows greybox prototype is built and locally runnable. This report covers gameplay source through commit `fd29b3b`; subsequent documentation commits do not change the tested binaries.

## Run the game

Launch:

```text
D:\personal projects\delusion\Artifacts\Windows\Delusion.exe
```

Keep the entire packaged folder together. The executable and its cooked data are local ignored build outputs, not files downloaded by cloning GitHub. The package is a Win64 Development build.

| Input | Action |
| --- | --- |
| W / A / S / D | Walk |
| Mouse | Rotate the third-person camera |
| Left Shift | Sprint |
| Space | Jump |
| E | Activate a nearby terminal when its prompt appears |
| Escape | Pause or resume |
| Resume / Quit | Pause-menu buttons |

## Implemented files and systems

- `Delusion.uproject`, `Source/Delusion.Target.cs`, `Source/DelusionEditor.Target.cs`, `Source/Delusion/Delusion.Build.cs`, and `Source/Delusion/Delusion.cpp`: native Unreal 5.8 C++ project and game/editor targets.
- `DelusionCharacter.h/.cpp`: Enhanced Input mappings, native CharacterMovement, walking/sprinting/jumping, gravity and capsule collision, constrained mouse camera, collision-aware SpringArm, and a primitive placeholder character.
- `DelusionNeighborhood.h/.cpp`: compact connected streets, intersection, crosswalks, sidewalks, building blocks, plaza, obstacles, spawn marker, boundaries, and basic lighting. Component collision provides solid ground and buildings.
- `DelusionInteractable.h`, `DelusionInteractionComponent.h/.cpp`, and `DelusionTestTerminal.h/.cpp`: reusable interaction interface and detection with 260 cm range, facing and line-of-sight checks; terminal changes its label and appearance on activation.
- `DelusionPlayerController.h/.cpp`, `DelusionHUDWidget.h/.cpp`, and `DelusionGameMode.h/.cpp`: player spawning, DELUSION branding, control hints, interaction prompt/feedback, pause/resume, and normal exit through native Unreal UI.
- `Config/DefaultEngine.ini`, `DefaultInput.ini`, `DefaultGame.ini`, and `DefaultGameUserSettings.ini`: startup map, input, Windows cooking, DX11 and conservative graphics defaults.
- `Content/Maps/Prototype.umap` and `Content/Materials/M_PrototypeSurface.uasset`: saved playable level and original parameterized greybox material; both stored with Git LFS.
- `Scripts/Build.ps1`, `Scripts/Test.ps1`, `Scripts/CreatePrototypeAssets.py`, and `Source/Delusion/Tests/DelusionPrototypeSmoke.cpp`: repeatable build/package commands, report-checked gameplay tests, and the one-asset-at-a-time generation utility. Both assets already exist; do not regenerate them to resume development.

The C++ gameplay files above are under `Source/Delusion/`. No vehicle, combat, NPC, mission, multiplayer, or streamed-world system is included.

## Verification results

| Check | Actual result and evidence |
| --- | --- |
| Editor C++ target | UBT succeeded using MSVC 14.50.35740 and Windows SDK 10.0.26100.0; [build log](Saved/Logs/Build-Editor.log). |
| Game C++ target | Win64 Development UBT build succeeded with the same compiler and SDK; [build log](Saved/Logs/Build-Game.log). |
| Full Unreal Editor | Opened the existing project and rendered Prototype. Map check reported 0 errors and 0 warnings. Editor closed normally; [startup log](Saved/Logs/Editor-project-open.log). |
| Source gameplay smoke test | `Delusion.Sprint0.Playable` passed after the mouse scaling fix; [report](Saved/Automation/Prototype-mouse-fix/index.json). |
| Windows packaging | AutomationTool BuildCookRun completed cooking, staging, packaging and archiving; BUILD SUCCESSFUL, exit 0; [console log](Saved/Logs/Package-console.log). |
| Packaged gameplay smoke test | `.\Scripts\Test.ps1 -Target Packaged -Visible` returned PASS and exit 0. JSON recorded 1 success, 0 failures, 0 warnings and 0 tests skipped, duration 8.74 seconds; [report](Saved/Automation/Packaged-f1fe140d505d4189aa7eef326d894f31/index.json), [runtime log](Saved/Automation/Packaged-f1fe140d505d4189aa7eef326d894f31/Game.log). |
| Normal packaged launch | Started the top-level launcher without supplying a map; the configured game map loaded and displayed the character, neighborhood and HUD. |
| Physical pause UI | Escape opened the menu, a mouse click on Resume returned to play, and a mouse click on Quit closed the game normally. The launcher exited 0; [runtime log](Saved/Logs/Packaged-manual.log). |

The automated test injects native Unreal input events into the running game and observes actual movement, physics and UI state. It verifies spawn/grounding, forward movement, faster sprint movement, jump/landing/gravity, mouse yaw and pitch limits, building collision, camera-arm collision, out-of-range interaction rejection, prompt detection, E-key terminal activation, visible feedback, paused game time, and resume behavior. It uses prepared test positions for the collision and interaction checks.

The test runner requires a fresh successful JSON result as well as process exit 0. Unreal's Automation Test Queue Empty exit condition alone can return 0 even when an assertion fails.

These results do not claim a physical key-by-key manual playthrough or a performance benchmark. W, Shift, Space, mouse axes, E and Escape were covered by automation; individual A/S/D mappings were inspected rather than separately exercised by that test. Owner review of movement feel, sustained performance and extended exploration remains useful.

The evidence paths are ignored local files and will not exist in a fresh checkout until the corresponding tools are run.

## Resolved failures

- Corrected C++ local names that hid inherited Unreal members and a test include path exposed by compilation.
- Diagnosed actual mouse-test output of 0.728 degrees against the intended 10.4 degrees: inherited Unreal axis sensitivity was applying an extra 0.07 multiplier. Explicit input axis sensitivity of 1 restored the intended scaling; the complete test then passed.
- Removed only the approved automatically generated Android File Server configuration block. The config returned to its committed bytes and the existing material remained unchanged. Disabled the unused plugin before further asset generation.
- Kept Windows security policies intact. The optional Fab plugin is disabled in this project.

## Storage and build policy

All project files and generated outputs remain under `D:\personal projects\delusion`. Unreal remains at `D:\Epic Games\UE_5.8`.

The build/test scripts set process-scoped D: locations for TEMP, TMP, DOTNET_CLI_HOME, UE-LocalDataCachePath, UBA_ROOT and AutomationTool logs, restoring previous values afterward. Editor verification used D: user data, cache and logs. Shader compilation and C++ action parallelism are limited to reduce memory pressure. Rendering starts with low scalability, DX11, and expensive ray tracing, Lumen, Nanite and virtual shadows disabled.

Storage snapshots, rounded GiB:

| Location | Measured use / free space |
| --- | --- |
| C: free before packaging | 14.54 GiB |
| D: free before packaging | 137.72 GiB |
| C: free after packaging and verification | 16.43 GiB |
| D: free after packaging and verification | 132.81 GiB |
| Whole project, including ignored outputs | Approximately 7.65 GiB |
| Intermediate | Approximately 4.53 GiB |
| Saved, including cooked/staged data and logs | Approximately 1.22 GiB |
| Artifacts, including archived Windows package | Approximately 0.88 GiB |
| Binaries | Approximately 0.74 GiB |
| DerivedDataCache | Approximately 0.27 GiB |

The archive is included in project size. Disk free-space changes also reflect other machine activity; no C: cleanup was performed during this delivery work. No cache, generated executable or packaged output was added to Git.

## Remaining limitations and next milestone

No known acceptance-blocking failure remains in the tested slice. This is a small greybox prototype with an unanimated primitive character, simple materials and basic lighting. It is not a finished open-world game. Sustained frame rate, long-session stability, a Shipping configuration and another PC have not been benchmarked or validated.

The next recommended milestone is owner playtesting followed by character animation, movement/camera tuning and measured performance work on this same neighborhood. Vehicle development has not started.

## Individual implementation commits and pushes

Each commit below changed exactly one file and was pushed successfully to `origin main` before the next source file changed. Author and committer were verified as `Madhavan <175943252+MAD1IZJOD@users.noreply.github.com>`. Earlier repository foundation commits precede baseline `2dbea82`; documentation-only delivery commits follow this table and are available in Git history.

| Commit | File | Message |
| --- | --- | --- |
| [30b5fc9](https://github.com/MAD1IZJOD/delusion/commit/30b5fc9986635a08f7ad63ac4bd3454faa2e13d7) | Delusion.uproject | add the unreal project descriptor |
| [d777d37](https://github.com/MAD1IZJOD/delusion/commit/d777d370299ae930542b4363b2a938f2bd04aae4) | Source/Delusion.Target.cs | add the windows game target |
| [a030c6b](https://github.com/MAD1IZJOD/delusion/commit/a030c6bc977c3b6fa4f705c3e2852b02d1e586f3) | Source/DelusionEditor.Target.cs | add the unreal editor target |
| [7968859](https://github.com/MAD1IZJOD/delusion/commit/7968859c53fd7885219cb47845431e05605828c5) | Source/Delusion/Delusion.Build.cs | configure the gameplay module dependencies |
| [e068431](https://github.com/MAD1IZJOD/delusion/commit/e0684317e7c46d94c4bd7502c4dcd5da8c13ae45) | Source/Delusion/Delusion.cpp | register the delusion gameplay module |
| [0250853](https://github.com/MAD1IZJOD/delusion/commit/0250853d9329de2c6f2a411b926151e459dc2059) | Scripts/Build.ps1 | keep build caches and temporary files on d |
| [66e8ce3](https://github.com/MAD1IZJOD/delusion/commit/66e8ce3d23c31be226b3cd8c5a53fde9b0afcfbf) | Config/DefaultEngine.ini | configure the prototype map and lightweight rendering |
| [6688b20](https://github.com/MAD1IZJOD/delusion/commit/6688b2096563a5b70398a6f8da5bb3cefcf6de6e) | Config/DefaultInput.ini | enable enhanced input and direct mouse controls |
| [d40c565](https://github.com/MAD1IZJOD/delusion/commit/d40c565a922da734e1a7285bad7e5e74f5277945) | Source/Delusion/DelusionCharacter.h | define the third person character and camera settings |
| [c31a69c](https://github.com/MAD1IZJOD/delusion/commit/c31a69c729cb194119855e7f70235232be4160e8) | Source/Delusion/DelusionCharacter.cpp | implement third person movement sprinting and camera controls |
| [4200869](https://github.com/MAD1IZJOD/delusion/commit/4200869c93e0521c44fbf9b5d1a5f2e6cf3d2772) | Source/Delusion/DelusionCharacter.cpp | avoid hiding the character mesh member |
| [86287ce](https://github.com/MAD1IZJOD/delusion/commit/86287ce0ba724bf25d321f1f38b96e9eb438c5a7) | Source/Delusion/DelusionNeighborhood.h | define the greybox neighborhood actor |
| [2014b78](https://github.com/MAD1IZJOD/delusion/commit/2014b789fde6b39c4d44782eb1f740bb35fe96c2) | Source/Delusion/DelusionNeighborhood.cpp | build the colliding greybox city block |
| [8a06e29](https://github.com/MAD1IZJOD/delusion/commit/8a06e29bcadd3b9757c356fe12ca35e5ee06a72c) | Source/Delusion/DelusionInteractable.h | add the reusable interaction interface |
| [78334e6](https://github.com/MAD1IZJOD/delusion/commit/78334e6e82220e1c5cc9650ddcdd3aaa8e2cd7f3) | Source/Delusion/DelusionInteractionComponent.h | define reusable interaction detection |
| [f41a1e8](https://github.com/MAD1IZJOD/delusion/commit/f41a1e8d66f6853515a447fa7ad83909dab8df50) | Source/Delusion/DelusionInteractionComponent.cpp | detect nearby interactions with range and visibility checks |
| [ea21263](https://github.com/MAD1IZJOD/delusion/commit/ea21263263e0c91fadf0910ea525772b867e5564) | Source/Delusion/DelusionTestTerminal.h | define the interactable test terminal |
| [8537484](https://github.com/MAD1IZJOD/delusion/commit/8537484e2d9031850d453790d187988c53f72fc1) | Source/Delusion/DelusionTestTerminal.cpp | add terminal activation and visible feedback |
| [b2affe5](https://github.com/MAD1IZJOD/delusion/commit/b2affe5377f70c5dd274174c04cb06563b7f9edd) | Source/Delusion/DelusionPlayerController.h | define the player controller and pause actions |
| [20920ed](https://github.com/MAD1IZJOD/delusion/commit/20920ed8f3128c6bab30679699c122a251819551) | Source/Delusion/DelusionHUDWidget.h | define the native hud and pause menu |
| [b654a58](https://github.com/MAD1IZJOD/delusion/commit/b654a5851f6e5b1425dc6885e21941b6843b0759) | Source/Delusion/DelusionHUDWidget.cpp | add interaction prompts and the resume quit menu |
| [09bf0af](https://github.com/MAD1IZJOD/delusion/commit/09bf0afb135b1792c72583dce910bc587e8192c0) | Source/Delusion/DelusionPlayerController.cpp | connect interaction controls pause and normal exit |
| [d2fdfa7](https://github.com/MAD1IZJOD/delusion/commit/d2fdfa7753c880955b5c3f213b4ae37ab667a8cb) | Source/Delusion/DelusionGameMode.h | define the single player game mode |
| [3ff009f](https://github.com/MAD1IZJOD/delusion/commit/3ff009f3f00a5c12ba84dc17a550e89eaad00ae8) | Source/Delusion/DelusionGameMode.cpp | connect the playable character and controller |
| [82dee8f](https://github.com/MAD1IZJOD/delusion/commit/82dee8f4c0c36cd9772c1746b406118808c195d5) | Config/DefaultGameUserSettings.ini | set conservative prototype graphics defaults |
| [4cb714a](https://github.com/MAD1IZJOD/delusion/commit/4cb714ad190d18d2dd83b99bdbbbb634d68e437d) | Source/Delusion/DelusionPlayerController.cpp | avoid hiding the controlled character member |
| [51ae0b6](https://github.com/MAD1IZJOD/delusion/commit/51ae0b673e130ffc5a043cc5d9ddffce78bcba2a) | Config/DefaultGame.ini | configure the prototype windows package |
| [52deece](https://github.com/MAD1IZJOD/delusion/commit/52deecefde9d388e82a6b564bfbdc48ebaa62511) | Scripts/CreatePrototypeAssets.py | create prototype assets one file at a time |
| [4d177dc](https://github.com/MAD1IZJOD/delusion/commit/4d177dc86e7207b4bbcee26486a6ec197e7c0676) | Content/Materials/M_PrototypeSurface.uasset | add the prototype surface material |
| [40bffb0](https://github.com/MAD1IZJOD/delusion/commit/40bffb08b9bf7fe6e36e511954ff142797bf87f4) | Delusion.uproject | disable the unused android file server plugin |
| [3282ef5](https://github.com/MAD1IZJOD/delusion/commit/3282ef58dfb1dcbc631e202992ec60906ff42f87) | Content/Maps/Prototype.umap | add the first playable neighborhood map |
| [2f0c191](https://github.com/MAD1IZJOD/delusion/commit/2f0c1910f9edcdf2daee9bac71540facc2193f9d) | Config/DefaultEngine.ini | limit shader compilation memory for the prototype |
| [648832e](https://github.com/MAD1IZJOD/delusion/commit/648832ef55e648d3cd736cb03a67f984c52749ea) | Source/Delusion/Tests/DelusionPrototypeSmoke.cpp | add a live gameplay smoke test |
| [1fedb6b](https://github.com/MAD1IZJOD/delusion/commit/1fedb6b68e0b608acde91ca7f78db2a962c4c8a0) | Source/Delusion/Tests/DelusionPrototypeSmoke.cpp | report measured input values in the smoke test |
| [33be434](https://github.com/MAD1IZJOD/delusion/commit/33be434dd8140f2896dbff06c01294780bc28534) | Config/DefaultInput.ini | fix mouse sensitivity scaling |
| [02c24f7](https://github.com/MAD1IZJOD/delusion/commit/02c24f7300744f2082e8f92f24352b357f8e4c54) | Scripts/Build.ps1 | add the windows packaging command |
| [fd29b3b](https://github.com/MAD1IZJOD/delusion/commit/fd29b3b16d9ae2b29460f09c19921da2e2a95cfb) | Scripts/Test.ps1 | add a repeatable gameplay test runner |
