# DELUSION engineering instructions

Read this file before changing code in every session. The user's current instructions take precedence. Preserve these absolute Git rules in future sessions; development convenience, batching, generators, and automation never justify an exception.

## Project and permanent location

- Project: Delusion, an original single-player, third-person, open-world PC game.
- Runtime: Unreal Engine 5.8 stable on Windows 11; C++ for reusable gameplay and Blueprints for asset wiring and designer configuration.
- Permanent root: `D:\personal projects\delusion`.
- Project descriptor when implemented: `D:\personal projects\delusion\Delusion.uproject`.
- Repository: `https://github.com/MAD1IZJOD/delusion`.
- Required remote: `origin`. Required branch: `main`.
- Run every Git command from the permanent root. Do not change repositories or directories without explicit user authorization.
- Inspect this directory first. Use the correct existing repository. Clone directly into this directory only if it is empty and no repository exists. Stop if there are unrelated files or an unexpected repository; never overwrite them.
- Never create another nested `delusion` directory, a second project copy, or a temporary development clone.
- All project source, Blueprints, maps, assets, configuration, documentation, compilation outputs, and packaged builds must remain inside the permanent root. Configure Unreal and Visual Studio to use this location.
- Temporary generator output outside the repository is allowed only when necessary for safe one-file-at-a-time importing. Inspect that output before importing individual files.
- Never commit the Unreal Engine installation or engine source.

## Absolute Git rules

### Identity and authentication

Every new commit must have exactly these identities:

```text
Author: Madhavan <175943252+MAD1IZJOD@users.noreply.github.com>
Committer: Madhavan <175943252+MAD1IZJOD@users.noreply.github.com>
```

- Use the user's existing local Git configuration and authenticated Git installation.
- Before the first commit in each session, run `git var GIT_AUTHOR_IDENT` and `git var GIT_COMMITTER_IDENT`. Verify both names and email addresses exactly.
- Inspect relevant commit templates, hooks, configuration includes, and environment overrides for silent identity changes or attribution trailers.
- If either identity is incorrect, stop before committing and report it. Never silently override it.
- The user authorized the repository-local `user.name` correction to `Madhavan` on 2026-11-20. This does not authorize unrelated identity changes.
- Do not change the existing email, global Git configuration, credentials, GitHub identity, or historical commits.
- Never introduce Codex as author or committer, Codex-Session metadata, Co-Authored-By trailers, AI-generated attribution trailers, OpenAI or other AI contributor accounts, alternate author or committer identities, forged identities, or impersonation.
- Use local Git CLI for every commit and push. Do not use GitHub API-generated commits, web-based commit creation, a GitHub bot, Codex cloud Git integration, or alternate credentials.

### Prohibited operations

Never perform any of the following:

- `git push --force` or `git push --force-with-lease`.
- `git reset --hard`.
- Published-history rebases or published-commit amendments.
- History rewriting, historical author or committer changes, or commit-date rewriting.
- Squashing existing published commits.
- Deleting or recreating the remote repository.
- Replacing, overwriting, discarding, or resetting existing user work.
- Stashing user changes without permission.
- GitHub API commits or web-based commit creation.

Do not automatically amend or rewrite a commit that fails verification. Stop before pushing and report the failure.

### Mandatory initial checks

Before modifying any product code, execute these six commands exactly, in this order, from the permanent root. Show their actual results:

```text
git status
git branch --show-current
git log -1 --format=fuller
git remote -v
git diff
git ls-files --others --exclude-standard
```

The branch must be `main`, origin must be the specified GitHub repository, and there must be no unexpected working-tree changes.

The repository was initially empty. For a genuinely empty repository with unborn HEAD, the log command may fail because there are no commits. That alone is expected: use read-only Git checks to verify the absence of commits, the `main` branch, and the correct remote. Investigate every other unexpected failure.

If uncommitted user changes or unexpected untracked files exist, stop. Do not discard, overwrite, reset, stage, commit, or stash them. Run:

```text
git status
git diff
git ls-files --others --exclude-standard
```

Explain the issue and wait for user instructions.

### One file, one commit, one push

After editing, creating, renaming, or deleting ONE version-controlled file, create exactly ONE commit for that file and push it before starting the next file. There are no convenience exceptions.

For every file, follow this exact sequence:

1. Inspect repository status.
2. Select the next file.
3. Modify only that file.
4. Review its diff or newly created contents.
5. Perform appropriate verification.
6. Stage ONLY that file using an explicit path: `git add -- <path>`.
7. Verify the staged changes.
8. Commit using the verified local identity.
9. Verify commit metadata and contents.
10. Push to `origin main`.
11. Confirm the push succeeded using actual command evidence.
12. Check the working-tree state and confirm local and remote synchronization.
13. Only then proceed to the next file.

- Never use `git add .` or `git add -A` to stage a batch.
- Never combine two separately modified files into one commit.
- Never edit file B while file A remains uncommitted or unpushed.
- Ignored generated caches, compilation outputs, and engine binaries do not count as project source files. Never accidentally commit them.
- If a project generator creates multiple source files, generate outside the repository only as permitted above, inspect the results, and import each required version-controlled file individually using this entire sequence.
- If verification tools modify additional tracked files, stop and inspect them. Never silently include them in the current commit.
- If a push fails, stop editing files. Investigate and report the exact failure safely.
- If the remote has advanced, stop and report divergence. Never force push or rewrite history.
- Verify that the local project and GitHub repository remain synchronized.

### Commit messages

Use short, natural, human-sounding, lowercase messages describing completed work on the one file. Do not describe planned work. Do not add automated footers or AI attribution.

Examples:

```text
set up the unreal project structure
add the third person character controller
configure basic camera movement
add collision to the test environment
set up the first playable map
fix camera rotation when moving
```

Never include `generated by codex`, `co-authored-by`, `automated agent commit`, or similar attribution.

### Verify every commit and push

Before pushing, inspect all three outputs:

```text
git show -s --format=fuller HEAD
git show -s --format=%B HEAD
git show --stat --oneline HEAD
```

Confirm the exact author, exact committer, absence of unwanted trailers or metadata, and that the commit contains only the intended file. Stop before pushing if any check fails. Do not automatically amend or rewrite that commit.

After pushing, verify that the remote received that exact commit and that the local branch remains synchronized. Never claim success without actual command evidence.

### Permanent instruction file

`AGENTS.md` must be the first repository file added, committed, and pushed before any other project file. Keep these Git rules and core engineering principles here so future sessions can follow them. Every subsequent session must read this file before changing code.

## Permanent storage policy

- Use D: as the primary drive for all DELUSION development. Keep the project at `D:\personal projects\delusion` and Unreal Engine at `D:\Epic Games\UE_5.8`.
- Prefer D: for downloads and installer packages, project assets, build and packaging outputs, shader and Derived Data Cache data, intermediate compilation data, temporary build files, debugging logs, and profiling output. Prefer D: for new large development tools when the official installer supports a custom location; do not relocate existing installations unnecessarily.
- Keep project artifacts inside the permanent root and outside version control. Existing ignored destinations include `DerivedDataCache`, `Intermediate`, `Saved`, `Temp`, `Downloads`, and `Artifacts`. Use `Artifacts\Windows` for packaged Windows builds, `Saved\Logs` for logs, and `Saved\Temp` for temporary project build files.
- Keep external development-software installers on D:, such as `D:\Epic Games\Installers`, outside the Git repository.
- Inspect actual cache and temporary-directory settings before changing them. Prefer documented project-specific configuration and reversible process-scoped overrides over global Windows environment changes. When relevant, inspect `UE-LocalDataCachePath`, `TEMP`, `TMP`, `DOTNET_CLI_HOME`, and `uebp_LogFolder` for each engine or build invocation; direct supported outputs to the ignored project locations on D:.
- The successful engine verification launch used a process-local `UE-LocalDataCachePath=D:\personal projects\delusion\DerivedDataCache`, with editor user data and logs under `Saved\EnvironmentVerification`. These process-local settings do not persist automatically into a fresh shell. Do not assume they do.
- Do not change machine-wide or user-wide Windows environment settings merely for convenience. Do not create symbolic links, alter disk partitions, or relocate Windows system directories.
- Inspect C: disk consumption if needed. Remove only known-safe, nonessential temporary files after confirming they are not in use. Never automatically delete personal documents, downloads, repositories, Windows system files, application installations, user profiles, or unknown folders.
- Do not empty the Recycle Bin or clear unrelated browser, application, or account data without approval. Ask before ambiguous or destructive cleanup.
- Report actual free space before and after significant installation or cleanup operations. Preserve existing compilers, SDKs, and development tools. Do not bypass UAC or other security prompts.

## Verified environment checkpoint and compiler requirement

- Unreal Engine 5.8.4 is installed at `D:\Epic Games\UE_5.8`. Do not download or reinstall it. Repeat already completed engine checks only when a relevant condition changes.
- UnrealBuildTool and AutomationTool have executed successfully. UBT platform validation reported `Win64 VALID` for the installed Windows SDK. These checks are not proof of C++ compilation or packaging.
- The editor initialized with DirectX 11 and the supported runtime argument `-DisablePlugins=Fab`. Windows Application Control blocked the optional Fab DLL with error 4551 during the default launch. Keep Windows security settings intact; do not patch engine code or signed files to work around restrictions.
- Install and verify MSVC 14.50 before game implementation. The official Visual Studio component is `Microsoft.VisualStudio.Component.VC.14.50.18.0.x86.x64`; use the existing Visual Studio Installer if it is available in that installation. Preserve MSVC 14.51 and the existing Windows SDK.
- The installed engine's `Engine\Config\Windows\Windows_SDK.json` prefers 14.50 or 14.44 and rejects 14.50 versions below 14.50.35723. Do not assume 14.51 compatibility. Verify the actual compiler executables and UBT selection rather than treating an existing version directory as a working installation.
- On 2026-11-21, Visual Studio Installer showed an 18.12.1 setup paused. MSVC 14.50 was absent, and no `cl.exe` or `link.exe` was found under the registered MSVC toolset root. Finish safe toolchain setup and verify the compiler before changing game source.
- Sprint 0 also includes pause and normal exit functionality. Its acceptance target remains a genuinely playable packaged Windows executable.

## Toolchain audit and current engine gate

After the mandatory Git checks, inspect the actual Windows environment using commands, installed files, and tool outputs:

1. OS and architecture.
2. Installed Unreal version and installation path.
3. Availability of Unreal Engine 5.8 stable.
4. Visual Studio and required C++ toolchain.
5. Windows SDK.
6. Git and Git LFS.
7. RAM and GPU.
8. Whether a compatible Unreal C++ project can actually compile.
9. Available disk space.
10. Unreal build and packaging tool availability.

- Never fabricate installations, compilation, editor, packaging, or runtime results.
- Report exact missing components and specific installation instructions. Do not download or install unrelated software, substitute another engine, or create a browser game to bypass missing Unreal tools.
- On 2026-11-20, the audit found Git, Git LFS, Visual Studio Build Tools 2026, MSVC, and Windows SDK 10.0.26100.0. No Unreal installation was detected in the inspected software directories or registrations. A restricted `WindowsApps` directory could not be searched.
- The user explicitly authorized independently verifiable repository setup while Unreal is absent. Stop engine-dependent implementation until Unreal Engine 5.8 stable is installed and its actual installation path is verified.
- Repository setup is not a playable prototype. Do not claim Sprint 0 completion from configuration files or an editor-only simulation.

## Engineering principles

- DELUSION is original work. Do not copy proprietary Rockstar code, assets, maps, characters, or intellectual property.
- Unreal Engine is the chosen runtime. Do not implement the game in Next.js, React, Three.js, or another browser framework.
- Prefer supported native Unreal systems and minimal external dependencies.
- Use modular C++ gameplay components, Blueprints for configuration, Enhanced Input, CharacterMovementComponent, SpringArm and camera components, Chaos, Niagara, and existing Unreal animation and AI frameworks.
- Use Chaos Vehicles when vehicle development is authorized. Use World Partition when world size justifies streaming.
- Separate game rules, rendering, and input concerns. Prefer deterministic, testable gameplay logic where practical. Avoid an enormous character class or GameMode.
- Do not introduce multiplayer architecture yet.
- Use original or appropriately licensed assets and retain required license information. Never assume premium assets are available. Use locally available engine primitives and placeholders to reach a playable build.
- Inspect relevant code and actual engine APIs before editing. Make the smallest complete change, check Unreal version compatibility, verify syntax and integration, and preserve working behavior. Avoid speculative API use.
- Run full compilation at meaningful integration checkpoints. Load the project in Unreal Editor when possible. Distinguish automated checks from required manual editor or gameplay validation.
- Use practical test automation without inventing tests for behavior that needs actual editor or gameplay verification.

## Sprint 0 scope and order

The first milestone is a genuine, locally playable, packaged Windows game. Long-term AAA aspirations are not Sprint 0 requirements.

Implement in this order, adjusting only for genuine Unreal dependency requirements. The Git rules never change:

1. Repository and Git safety audit.
2. Toolchain inspection.
3. Root `AGENTS.md`.
4. Unreal-compatible `.gitignore`.
5. Git LFS configuration if installed and verified, including `.uasset` and `.umap` assets.
6. Unreal Engine 5.8 C++ project descriptor named `Delusion.uproject`.
7. Necessary module and build-target files using Unreal conventions.
8. Core gameplay framework.
9. Player character.
10. Enhanced Input configuration.
11. Third-person camera.
12. Compact greybox environment.
13. Reusable basic interaction system.
14. Minimal HUD.
15. Compilation and runtime verification.
16. Windows packaging and executable smoke test.
17. Honest Sprint 0 completion report.

Player controls must include WASD movement, mouse camera, Space to jump, Shift to sprint, gravity, ground collision, and smooth responsive rotation. Use frame-rate-independent movement and native character movement rather than unnecessary custom walking physics.

The follow camera needs sensible distance, constrained pitch, sensitivity, rotation, and camera collision handling.

The greybox needs walkable ground, several building blocks, a short road segment, colliding walls or obstacles, a recognizable spawn, and basic lighting.

Provide one nearby object with a visible prompt and E-key activation. Keep interaction reusable for future vehicles, NPCs, doors, and mission objects. The lightweight readable HUD must show DELUSION and applicable interaction hints; movement controls are optional.

Acceptance requires launching the packaged Windows executable, spawning into the environment, walking, turning the camera, running, jumping, colliding with obstacles, activating the test object, and exiting normally. Attempt real packaging, record actual errors, fix them incrementally, and run the resulting executable when possible. Report human inspection still needed. Never claim builds or runtime behavior without verification.

Do not begin Sprint 1, vehicles, combat, advanced procedural generation, or photorealistic environment work until Sprint 0 is stable and its basic playable-game acceptance criteria are met. Do not create empty placeholders merely to suggest a complete directory structure or deliver disconnected mechanics without playable integration.

Long-term targets include streamed urban districts, vehicles, pedestrians and traffic, interactions, dialogue and missions, combat, police pursuit, weather and day/night, lighting and VFX, save/load, and stable Windows releases. Keep current work focused on the approved milestone.

## Repository hygiene

- Exclude DerivedDataCache, Intermediate files, build caches, engine binaries, packaged executable outputs, local editor session data, temporary downloaded assets, and local credentials.
- Preserve intentional project assets and required third-party license information.
- Configure LFS only when installed and verified. Keep LFS and other repository-specific configuration local; never change global Git configuration.

## Reporting and stop conditions

After every successful individual file commit and push, report concisely:

```text
File:
What changed:
Verification:
Commit message:
Commit hash:
Author and committer verified:
Push result:
```

At meaningful milestones report implemented systems, verified functionality, outstanding issues, current executable status, and the next engineering milestone. Explain actual command failures without guessing.

Work autonomously through authorized safe changes; ordinary file edits do not require approval. Stop for unexpected repository changes, an incorrect branch or remote, unverifiable identity, a failed push, unintended commit files or attribution, credentials or an unexpected purchase required by a dependency, potentially destructive operations, or a technical blocker that prevents meaningful verification. Never conceal failures or claim actions without evidence.
