param(
    [ValidateSet('Editor', 'Game', 'ProjectFiles', 'Package')]
    [string]$Target = 'Editor'
)

$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$engineRoot = 'D:\Epic Games\UE_5.8'
if ((Get-Location).Path -ne $projectRoot) {
    throw "Run this script from $projectRoot."
}

$dotnet = Join-Path $engineRoot 'Engine\Binaries\ThirdParty\DotNet\10.0\win-x64\dotnet.exe'
$buildTool = Join-Path $engineRoot 'Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll'
foreach ($requiredFile in @($dotnet, $buildTool, "$projectRoot\Delusion.uproject")) {
    if (-not (Test-Path -LiteralPath $requiredFile -PathType Leaf)) {
        throw "Required file is missing: $requiredFile"
    }
}

$drives = Get-CimInstance Win32_LogicalDisk -Filter "DeviceID='C:' OR DeviceID='D:'"
foreach ($drive in $drives) {
    Write-Host ('{0} free: {1:N2} GiB' -f $drive.DeviceID, ($drive.FreeSpace / 1GB))
    $minimumFree = if ($drive.DeviceID -eq 'D:') { 15GB } else { 10GB }
    if ($drive.FreeSpace -lt $minimumFree) {
        throw "Insufficient free space on $($drive.DeviceID); review storage before building."
    }
}

# These settings apply only to this build and its child processes.
$buildEnvironment = @{
    TEMP = "$projectRoot\Saved\Temp"
    TMP = "$projectRoot\Saved\Temp"
    DOTNET_CLI_HOME = "$projectRoot\Saved\DotNet"
    'UE-LocalDataCachePath' = "$projectRoot\DerivedDataCache"
    UBA_ROOT = "$projectRoot\Saved\UnrealBuildTool\UBA"
    uebp_LogFolder = "$projectRoot\Saved\Logs\AutomationTool"
}
$previousEnvironment = @{}
try {
    foreach ($name in $buildEnvironment.Keys) {
        $previousEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [IO.Directory]::CreateDirectory($buildEnvironment[$name]) | Out-Null
        [Environment]::SetEnvironmentVariable($name, $buildEnvironment[$name], 'Process')
    }
    [IO.Directory]::CreateDirectory("$projectRoot\Saved\Logs") | Out-Null
    if ($Target -eq 'Package') {
        if (-not (Test-Path -LiteralPath "$projectRoot\Content\Maps\Prototype.umap")) {
            throw 'The prototype map must exist before packaging.'
        }
        # Compile both targets explicitly with the pinned compiler before cooking.
        & $PSCommandPath -Target Editor
        & $PSCommandPath -Target Game
        $automationTool = Join-Path $engineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
        $packageArguments = @(
            '-NoCompile', '-NoCompileUAT', 'BuildCookRun', "-project=$projectRoot\Delusion.uproject",
            '-noP4', '-platform=Win64', '-clientconfig=Development', '-skipbuild', '-skipbuildeditor',
            '-cook', '-stage', '-pak', '-iostore', '-package', '-archive',
            "-archivedirectory=$projectRoot\Artifacts\Sprint2\Windows", '-map=/Game/Maps/Prototype',
            '-AdditionalCookerOptions=-CookProcessCount=1', '-unattended', '-utf8output'
        )
        & $automationTool @packageArguments
        if ($LASTEXITCODE -ne 0) {
            throw "Windows packaging failed with exit code $LASTEXITCODE. See Saved\Logs\AutomationTool."
        }
        Write-Host "Windows package archived under $projectRoot\Artifacts\Sprint1\Windows."
    } else {
        $buildArguments = @($buildTool)
        if ($Target -eq 'ProjectFiles') {
            $buildArguments += @('-ProjectFiles', "-Project=$projectRoot\Delusion.uproject", '-Game')
        } else {
            $targetName = if ($Target -eq 'Editor') { 'DelusionEditor' } else { 'Delusion' }
            $buildArguments += @(
                $targetName, 'Win64', 'Development', "-Project=$projectRoot\Delusion.uproject",
                '-CompilerVersion=14.50.35740', '-MaxParallelActions=1', '-NoUBA', '-NoXGE',
                '-NoHotReloadFromIDE'
            )
        }
        $buildArguments += "-Log=$projectRoot\Saved\Logs\Build-$Target.log"
        & $dotnet @buildArguments
        if ($LASTEXITCODE -ne 0) {
            throw "UnrealBuildTool failed with exit code $LASTEXITCODE. See Saved\Logs\Build-$Target.log."
        }
    }
} finally {
    foreach ($name in $previousEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $previousEnvironment[$name], 'Process')
    }
}
