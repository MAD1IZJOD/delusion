param(
    [ValidateSet('Editor', 'Packaged')]
    [string]$Target = 'Editor',
    [ValidateSet('Sprint0', 'Sprint1', 'All')]
    [string]$Suite = 'All',
    [string]$PackageDirectory = 'Artifacts\Sprint1\Windows',
    [ValidateRange(30, 1800)]
    [int]$TimeoutSeconds = 600,
    [switch]$Visible
)

$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ((Get-Location).Path -ne $projectRoot) { throw "Run this script from $projectRoot." }
$expectedTests = @(switch ($Suite) {
    'Sprint0' { @('Delusion.Sprint0.Playable') }
    'Sprint1' { @('Delusion.Sprint1.Vehicle') }
    'All' { @('Delusion.Sprint0.Playable', 'Delusion.Sprint1.Vehicle') }
})
$testFilter = if ($Suite -eq 'All') { 'Delusion.' } else { $expectedTests[0] }

$testArguments = @()
if ($Target -eq 'Editor') {
    $executable = 'D:\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
    $testArguments += '"' + "$projectRoot\Delusion.uproject" + '"'
    $testArguments += '-game'
} else {
    $packageRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot $PackageDirectory))
    if (-not $packageRoot.StartsWith("$projectRoot\Artifacts\", [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Test packages must remain inside the project Artifacts directory.'
    }
    $candidates = @(Get-ChildItem -LiteralPath $packageRoot -Filter 'Delusion.exe' -File -Recurse |
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
    ('-ExecCmds="Automation RunTests ' + $testFilter + '"'),
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
    $results = @($report.tests | Where-Object { $_.fullTestPath -in $expectedTests })
    $passedNames = @($results | Where-Object state -EQ 'Success' | Select-Object -ExpandProperty fullTestPath -Unique)
    if ($testProcess.ExitCode -ne 0 -or $report.failed -ne 0 -or $passedNames.Count -ne $expectedTests.Count) {
        throw "Gameplay smoke test failed. Inspect $reportPath."
    }
    Write-Host "PASS: $($expectedTests -join ', ') ($Target)"
    Write-Host "Report: $reportPath\index.json"
    Write-Host "Log: $logPath"
} finally {
    foreach ($name in $previousEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $previousEnvironment[$name], 'Process')
    }
}
