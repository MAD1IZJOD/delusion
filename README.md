# DELUSION

An original single-player, third-person, open-world PC game built with Unreal Engine 5.8 stable, C++, and Blueprints. Sprint 0 targets a small, playable greybox prototype and a packaged Windows executable.

## Current status

Repository foundation only, audited on 2026-11-20. No Unreal project descriptor, gameplay implementation, map, or executable has been created. Sprint 0 is not complete.

- Permanent project root: `D:\personal projects\delusion`.
- Planned project descriptor: `D:\personal projects\delusion\Delusion.uproject`.
- Remote: `origin`, `https://github.com/MAD1IZJOD/delusion`.
- Branch: `main`.
- Read [AGENTS.md](AGENTS.md) before changing code. It contains the mandatory Git workflow and engineering instructions.

Each version-controlled file must be reviewed, verified, committed on its own, and pushed to `origin main` before another file is changed. Use the user's local Git CLI and the exact required author and committer identity. Do not batch files or rewrite history.

## Verified repository setup

- `.gitignore` excludes Unreal project/plugin caches, generated IDE data, local credentials, and designated build and package destinations. Source, maps, assets, source art, third-party licenses, and source packaging icons remain visible to Git.
- `.gitattributes` normalizes text to LF, with CRLF for Windows batch files. Unreal packages, bulk-data sidecars, and binary source media use Git LFS without text conversion.
- Git LFS was initialized with `git lfs install --local`. The local pre-push hook was inspected. No global Git settings or credentials were changed.
- Git ignore probes verified 39 excluded paths and 28 preserved paths without creating sample files.
- Attribute checks verified 25 binary extensions and 9 text paths. A local LFS clean/pointer/smudge round trip preserved binary bytes exactly. No project asset or remote LFS object upload was involved; remote asset transfer remains untested.

On another development machine, install Git LFS and initialize it locally from the project root before adding binary assets:

```powershell
Set-Location "D:\personal projects\delusion"
git lfs version
git lfs install --local
```

## Windows environment audit

These are observed local results, not proof that an Unreal project can compile.

| Component | Observed result |
| --- | --- |
| OS | Windows 11 Home Single Language, 64-bit, build 10.0.26200 |
| CPU | AMD Ryzen AI 7 350, 8 cores / 16 logical processors |
| Installed RAM | 24 GiB |
| GPU | AMD Radeon 860M Graphics; driver 32.0.22024.3004 |
| Visual Studio tooling | Visual Studio Build Tools 2026, installation 18.8.12009.203; full IDE not detected |
| MSVC | Toolset directory 14.51.36231; x64 compiler reports 19.51.36248 |
| MSBuild | Executable launches and reports 18.8.2.30814 |
| Windows SDK | 10.0.26100.0; Windows headers, x64 kernel32.lib, and rc.exe found |
| Git | 2.55.0.windows.2 |
| Git LFS | 3.7.1 |
| Free space at audit | C: 29.38 GiB; D: 180.88 GiB; E: 120.71 GiB |
| Unreal Engine 5.8 | No installation detected in audited locations; actual installation path unverified |

The compiler launched for `cl /Bv` and reported its version. It then returned `D8003: missing source filename`, as expected with no source file supplied. No C++ compilation was claimed or performed.

## Unreal installation blocker

Unreal Editor, UnrealBuildTool, and AutomationTool were not found in the inspected locations. The usual Epic launcher installation list, launcher manifests, and Unreal installation registry keys were absent.

The installation search covered `C:\Program Files`, `C:\Program Files (x86)`, `D:\applications`, and `D:\technologies`, plus common Epic/Unreal engine roots. The `C:\Program Files\WindowsApps` subtree returned `Access is denied` (OS error 5). This was a bounded search, not proof about every directory on the machine.

Engine-dependent work is stopped until an actual Unreal Engine 5.8 stable installation is available and its path and version are verified. Unreal compilation, editor loading, cooking, packaging, and gameplay have not been tested.

To unblock:

1. Install [Epic Games Launcher](https://www.unrealengine.com/download) if necessary and sign in with your own Epic account.
2. In the launcher, open Unreal Engine > Library, add an engine version, and select a stable 5.8 release. Install the editor and Windows build/packaging support. Keep the engine installation outside this Git repository; keep every DELUSION project file in the permanent project root.
3. Provide the actual installed engine root, or allow the next session to discover it. A proposed or example path is not installation evidence.
4. Verify `Engine\Build\Build.version` reports major version 5 and minor version 8, and inspect these installed files relative to that root:
   - `Engine\Binaries\Win64\UnrealEditor.exe`
   - `Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll`
   - `Engine\Build\BatchFiles\Build.bat`
   - `Engine\Build\BatchFiles\RunUAT.bat`
5. Use the installed engine's toolchain checks and a real project build to establish compiler compatibility. Resolve any actual missing component or compiler-version errors before proceeding.

Epic confirms that [Unreal Engine 5.8 is available through the launcher](https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available). Its [Visual Studio setup guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine) documents version compatibility and C++ components. No download or installation was performed during repository setup.

## Next engineering checkpoint

After the engine gate is satisfied, create the C++ project descriptor and module/target files individually under the mandatory one-file/commit/push workflow. Import any necessary generator output one file at a time. Do not run a generator that creates multiple version-controlled files directly in this repository.

Then implement and integrate:

- WASD movement, mouse camera, Space to jump, Shift to sprint, gravity, and collisions.
- A third-person follow camera with pitch constraints and camera collision handling.
- A compact greybox with ground, buildings, a short road, obstacles, lighting, and a recognizable spawn.
- A reusable E-key interaction with an approach prompt and one test object.
- A minimal DELUSION HUD.

Keep future local packaged outputs under the ignored `D:\personal projects\delusion\Artifacts\Windows` destination. The final acceptance test must launch the packaged executable, exercise movement/camera/sprint/jump/collision/interaction, and exit normally. Editor-only play does not complete Sprint 0. Vehicle and Sprint 1 work remain out of scope until those criteria are met.
