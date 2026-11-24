param(
    [ValidateSet('Editor', 'Packaged')]
    [string]$Target = 'Editor',
    [ValidateRange(30, 1800)]
    [int]$TimeoutSeconds = 600,
    [switch]$Visible
)

$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ((Get-Location).Path -ne $projectRoot) { throw "Run this script from $projectRoot." }

$testArguments = @()
if ($Target -eq 'Editor') {
    $executable = 'D:\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
    $testArguments += '"' + "$projectRoot\Delusion.uproject" + '"'
    $testArguments += '-game'
} else {
    $candidates = @(Get-ChildItem -LiteralPath "$projectRoot\Artifacts\Windows" -Filter 'Delusion.exe' -File -Recurse |
        Where-Object { $_.FullName -like '*\Binaries\Win64\Delusion.exe' })
    if ($candidates.Count -ne 1) { throw 'Expected exactly one archived Windows game binary. Package the game first.' }
    $executable = $candidates[0].FullName
}
if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) { throw "Missing executable: $executable" }

# A unique report directory prevents an old success report from masking a failed launch.
$runId = [Guid]::NewGuid().ToString('N')
$reportPath = "$projectRoot\Saved\Automation\$Target-$runId"
$logPath = "$reportPath\Game.log"
[IO.Directory]::CreateDirectory($reportPath) | Out-Null
$testArguments += @(
    '/Game/Maps/Prototype', '-dx11', '-windowed', '-ResX=1280', '-ResY=720',
    '-NoSplash', '-NoLiveCoding', '-NoSound',
    ('-UserDir="' + "$projectRoot\Saved\TestUser" + '"'),
    ('-AbsLog="' + $logPath + '"'),
    '-ExecCmds="Automation RunTests Delusion.Sprint0.Playable"',
    '-TestExit="Automation Test Queue Empty"',
    ('-ReportExportPath="' + $reportPath + '"')
)
if (-not $Visible) { $testArguments += '-unattended' }

$testEnvironment = @{
    TEMP = "$projectRoot\Saved\Temp"
    TMP = "$projectRoot\Saved\Temp"
    DOTNET_CLI_HOME = "$projectRoot\Saved\DotNet"
    'UE-LocalDataCachePath' = "$projectRoot\DerivedDataCache"
    UBA_ROOT = "$projectRoot\Saved\UnrealBuildTool\UBA"
    uebp_LogFolder = "$projectRoot\Saved\Logs\AutomationTool"
}
$previousEnvironment = @{}
try {
    foreach ($name in $testEnvironment.Keys) {
        $previousEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [IO.Directory]::CreateDirectory($testEnvironment[$name]) | Out-Null
        [Environment]::SetEnvironmentVariable($name, $testEnvironment[$name], 'Process')
    }
    $windowStyle = if ($Visible) { 'Normal' } else { 'Hidden' }
    $testProcess = Start-Process -FilePath $executable -ArgumentList $testArguments -WindowStyle $windowStyle -PassThru
    if (-not $testProcess.WaitForExit($TimeoutSeconds * 1000)) {
        Stop-Process -Id $testProcess.Id -ErrorAction SilentlyContinue
        throw "The owned test process timed out. Inspect $logPath."
    }
    # TestExit can return zero even when assertions fail. Require the actual report.
    $report = Get-Content -LiteralPath "$reportPath\index.json" -Raw | ConvertFrom-Json
    $result = @($report.tests | Where-Object fullTestPath -EQ 'Delusion.Sprint0.Playable')
    if ($testProcess.ExitCode -ne 0 -or $report.failed -ne 0 -or $result.Count -ne 1 -or $result[0].state -ne 'Success') {
        throw "Gameplay smoke test failed. Inspect $reportPath."
    }
    Write-Host "PASS: Delusion.Sprint0.Playable ($Target)"
    Write-Host "Report: $reportPath\index.json"
    Write-Host "Log: $logPath"
} finally {
    foreach ($name in $previousEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $previousEnvironment[$name], 'Process')
    }
}
