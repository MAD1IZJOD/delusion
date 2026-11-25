# DELUSION

An original single-player, third-person Unreal Engine 5.8 C++ game. Sprint 0 now has a locally verified, playable Windows greybox prototype: a controllable character, compact neighborhood, interaction terminal, HUD, and pause menu.

The packaged Development build, source gameplay test and packaged gameplay test passed on 2026-11-25. The full Unreal Editor opened the map successfully. See [SPRINT0_REPORT.md](SPRINT0_REPORT.md) for evidence, individual implementation commits, storage measurements and test limitations.

## Play

Launch the existing local package:

```powershell
Set-Location "D:\personal projects\delusion"
& ".\Artifacts\Windows\Delusion.exe"
```

Keep the entire package folder together. Packaged binaries are ignored local outputs; a Git clone contains the source and LFS assets, not this executable.

| Input | Action |
| --- | --- |
| W / A / S / D | Move |
| Mouse | Rotate camera |
| Left Shift | Sprint |
| Space | Jump |
| E | Interact when the terminal prompt appears |
| Escape | Pause / resume |
| Resume / Quit | Pause-menu buttons |

The player spawns on the marked road area. Approach the terminal and press E to toggle its visible state.

## Build and test

Use the existing project at `D:\personal projects\delusion\Delusion.uproject`. Verified dependencies are Unreal Engine 5.8.4 at `D:\Epic Games\UE_5.8`, MSVC 14.50.35740 and Windows SDK 10.0.26100.0. Preserve installed tools; there is no outstanding installation blocker.

Run commands from the permanent root:

```powershell
Set-Location "D:\personal projects\delusion"
.\Scripts\Build.ps1 -Target Editor
.\Scripts\Build.ps1 -Target Game
.\Scripts\Test.ps1 -Target Editor -Visible
.\Scripts\Build.ps1 -Target Package
.\Scripts\Test.ps1 -Target Packaged -Visible
```

Each command serves a separate purpose; packaging builds both C++ targets itself. `-Target ProjectFiles` generates ignored IDE project files when needed. Tests use the already built binaries, so rebuild after changing C++ and repackage before testing updated packaged gameplay.

The test runner executes `Delusion.Sprint0.Playable`, requires a fresh successful JSON report and exit status, and prints the report location under `Saved\Automation`. Its Editor target runs the game through Unreal Editor; the Packaged target runs the archived Windows game. Test windows close after completion. Omit `-Visible` for unattended checks.

The first cook and shader compilation may take substantial time. The scripts check disk space, pin the verified compiler, limit parallel C++ actions, and direct supported temporary files, DDC, UBA data and logs to ignored D: project locations using process-scoped settings.

## Open the full editor

After building the Editor target, use a fresh PowerShell session for these process-local cache settings. They do not change Windows user or machine environment settings.

```powershell
Set-Location "D:\personal projects\delusion"
$projectRoot = (Get-Location).Path
New-Item -ItemType Directory -Force -Path "Saved\Temp","Saved\Logs","DerivedDataCache" | Out-Null
$env:TEMP = "$projectRoot\Saved\Temp"
$env:TMP = $env:TEMP
${env:UE-LocalDataCachePath} = "$projectRoot\DerivedDataCache"
& "D:\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "$projectRoot\Delusion.uproject" -dx11 "-UserDir=$projectRoot\Saved\EditorUser" "-AbsLog=$projectRoot\Saved\Logs\Editor.log"
```

The startup map is `/Game/Maps/Prototype`. Press Play for the player camera. Fab and Android File Server are disabled in this project. Do not modify Windows security policies or signed engine files.

## Project layout

- `Source/Delusion`: native character, neighborhood, interaction interface/component, terminal, player controller, HUD, game mode and gameplay smoke test.
- `Config`: input, startup map, packaging and conservative graphics defaults.
- `Content/Maps/Prototype.umap`: saved playable map.
- `Content/Materials/M_PrototypeSurface.uasset`: original greybox material.
- `Scripts`: build/package runner, gameplay test runner and one-asset-at-a-time generation utility. The assets already exist; do not rerun generation to resume work.
- `Artifacts/Windows`: local packaged Windows build.
- `Saved`, `Intermediate`, `Binaries`, `DerivedDataCache`: ignored generated data.

The map and material use Git LFS. Their uploads were verified. Git LFS must be available when checking out assets on another machine.

## Development rules and scope

Read [AGENTS.md](AGENTS.md) before changing files. Keep all project work in `D:\personal projects\delusion`, use local Git on `main` with `origin`, and follow **one changed file, one verified commit, one successful push before the next file**. Preserve the exact required author/committer identity and all history restrictions.

The prototype uses DX11, low scalability, simple collision and lightweight geometry for the Radeon 860M. Character art is an unanimated primitive placeholder. Sustained performance, long-session stability, Shipping builds and other PCs still need validation. The recommended next step is owner playtesting, animation and movement/camera/performance refinement. Vehicles, combat and larger world systems have not started.
