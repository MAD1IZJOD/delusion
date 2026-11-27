param(
    [string]$PackageDirectory = 'Artifacts\Sprint1\Windows',
    [ValidateSet('Idle', 'Driving')]
    [string]$Scenario = 'Idle',
    [ValidatePattern('^[A-Za-z0-9_-]+$')]
    [string]$Label = 'Sprint1Idle'
)

$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ((Get-Location).Path -ne $projectRoot) { throw "Run from $projectRoot." }
$packageRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot $PackageDirectory))
if (-not $packageRoot.StartsWith("$projectRoot\Artifacts\", [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Profile only a package inside the project Artifacts directory.'
}
$games = @(Get-ChildItem -LiteralPath $packageRoot -File -Filter Delusion.exe -Recurse |
    Where-Object FullName -Like '*\Binaries\Win64\Delusion.exe')
if ($games.Count -ne 1) { throw 'Expected exactly one packaged game runtime.' }
foreach ($drive in @(Get-CimInstance Win32_LogicalDisk -Filter "DeviceID='C:' OR DeviceID='D:'")) {
    Write-Host ('{0} free: {1:N2} GiB' -f $drive.DeviceID, ($drive.FreeSpace / 1GB))
    $minimum = if ($drive.DeviceID -eq 'D:') { 2GB } else { 10GB }
    if ($drive.FreeSpace -lt $minimum) { throw 'Insufficient profiling storage; inspect disk use first.' }
}
$runRoot = "$projectRoot\Saved\Profiling\$Label-$([Guid]::NewGuid().ToString('N'))"
[IO.Directory]::CreateDirectory($runRoot) | Out-Null
$profileEnvironment = @{
    TEMP = "$projectRoot\Saved\Temp"
    TMP = "$projectRoot\Saved\Temp"
    DOTNET_CLI_HOME = "$projectRoot\Saved\DotNet"
    'UE-LocalDataCachePath' = "$projectRoot\DerivedDataCache"
    UBA_ROOT = "$projectRoot\Saved\UnrealBuildTool\UBA"
    uebp_LogFolder = "$projectRoot\Saved\Logs\AutomationTool"
}
$previousEnvironment = @{}
try {
    foreach ($name in $profileEnvironment.Keys) {
        $previousEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [IO.Directory]::CreateDirectory($profileEnvironment[$name]) | Out-Null
        [Environment]::SetEnvironmentVariable($name, $profileEnvironment[$name], 'Process')
    }
    $execCmds = if ($Scenario -eq 'Driving') {
        'Automation RunTests Perf.DelusionDrive,t.MaxFPS 60,r.VSync 0,r.ScreenPercentage 80'
    } else {
        't.MaxFPS 60,r.VSync 0,r.ScreenPercentage 80'
    }
    $arguments = @(
        '-dx11', '-windowed', '-ResX=1280', '-ResY=720', '-NoSplash', '-NoSound', '-unattended',
        '-csvCaptureFrames=3600', '-ExitAfterCsvProfiling', '-csvGpuStats', '-csvCompression=0',
        ('-UserDir="' + $runRoot + '\User"'), ('-AbsLog="' + $runRoot + '\Game.log"'),
        ('-ExecCmds="' + $execCmds + '"')
    )
    $watch = [Diagnostics.Stopwatch]::StartNew()
    $captureProcess = Start-Process -FilePath $games[0].FullName -ArgumentList $arguments -WindowStyle Normal -PassThru
    $peakWorking = 0L
    $peakPrivate = 0L
    $firstWindowSeconds = $null
    while (-not $captureProcess.HasExited) {
        $captureProcess.Refresh()
        $peakWorking = [math]::Max($peakWorking, $captureProcess.WorkingSet64)
        $peakPrivate = [math]::Max($peakPrivate, $captureProcess.PrivateMemorySize64)
        if ($null -eq $firstWindowSeconds -and $captureProcess.MainWindowHandle -ne 0) {
            $firstWindowSeconds = $watch.Elapsed.TotalSeconds
        }
        if ($watch.Elapsed.TotalSeconds -gt 300) {
            Stop-Process -Id $captureProcess.Id -ErrorAction SilentlyContinue
            throw "Owned profiling process timed out. Inspect $runRoot."
        }
        Start-Sleep -Milliseconds 250
    }
    $captureProcess.WaitForExit()
    $csvFiles = @(Get-ChildItem -LiteralPath $runRoot -File -Filter '*.csv' -Recurse)
    if ($csvFiles.Count -ne 1) { throw "Missing unique completed CSV capture in $runRoot." }

    # UE CSV can contain duplicate nonmetric headings and a large events field.
    # Read the required columns by index instead of Import-Csv's unique-property conversion.
    Add-Type -AssemblyName Microsoft.VisualBasic.Core
    $reader = [Microsoft.VisualBasic.FileIO.TextFieldParser]::new($csvFiles[0].FullName)
    $frames = [Collections.Generic.List[double[]]]::new()
    $metricNames = @('FrameTime', 'GameThreadTime', 'RenderThreadTime', 'GPUTime')
    try {
        $reader.SetDelimiters(',')
        $reader.HasFieldsEnclosedInQuotes = $true
        $headers = $reader.ReadFields()
        $columns = @($metricNames | ForEach-Object {
            [Array]::IndexOf($headers, $_)
        })
        foreach ($metric in $metricNames) {
            if (@($headers | Where-Object { $_ -ceq $metric }).Count -ne 1) { throw "Missing or ambiguous CSV metric: $metric" }
        }
        while (-not $reader.EndOfData) {
            $fields = $reader.ReadFields()
            $values = [double[]]::new(4)
            $valid = $true
            for ($index = 0; $index -lt 4; ++$index) {
                $value = 0.0
                if ($columns[$index] -ge $fields.Length -or -not [double]::TryParse(
                    $fields[$columns[$index]], [Globalization.NumberStyles]::Float,
                    [Globalization.CultureInfo]::InvariantCulture, [ref]$value)) {
                    $valid = $false
                    break
                }
                $values[$index] = $value
            }
            if ($valid) { $frames.Add($values) }
        }
    } finally { $reader.Dispose() }
    if ($frames.Count -lt 2401 -or $null -eq $firstWindowSeconds) { throw 'Incomplete rendered profiling capture.' }
    # Discard the boot frame and 600 warm-up frames; measure the same next 1800 frames.
    $sample = $frames.GetRange(601, 1800)
    $frameTimes = @($sample | ForEach-Object { $_[0] })
    $sorted = @($frameTimes | Sort-Object)
    function Percentile([double]$Fraction) {
        $position = ($sorted.Count - 1) * $Fraction
        $low = [int][math]::Floor($position)
        $high = [int][math]::Ceiling($position)
        return $sorted[$low] + ($sorted[$high] - $sorted[$low]) * ($position - $low)
    }
    $durationMs = ($frameTimes | Measure-Object -Sum).Sum
    $scenarioDescription = if ($Scenario -eq 'Driving') {
        'Sustained driving benchmark; enters the prototype car and drives a continuous circle; 600 warm-up and 1800 measured runtime frames'
    } else {
        'Static default spawn; no intentional input; 600 warm-up and 1800 measured runtime frames'
    }
    $summary = [ordered]@{
        Scenario = $scenarioDescription
        Executable = $games[0].FullName
        Resolution = '1280x720 output; 80% render scale; low; DX11; VSync off; 60 FPS cap'
        Frames = $sample.Count
        DurationSeconds = $durationMs / 1000
        AverageFPS = $sample.Count * 1000 / $durationMs
        FrameMeanMs = $durationMs / $sample.Count
        FrameP50Ms = Percentile 0.5
        FrameP95Ms = Percentile 0.95
        FrameP99Ms = Percentile 0.99
        FrameMaxMs = $sorted[-1]
        FramesOver33Ms = @($frameTimes | Where-Object { $_ -gt 33.3333 }).Count
        FramesOver50Ms = @($frameTimes | Where-Object { $_ -gt 50 }).Count
        GameThreadMeanMs = ($sample | ForEach-Object { $_[1] } | Measure-Object -Average).Average
        RenderThreadMeanMs = ($sample | ForEach-Object { $_[2] } | Measure-Object -Average).Average
        GPUMeanMs = ($sample | ForEach-Object { $_[3] } | Measure-Object -Average).Average
        PeakWorkingSetMiB = $peakWorking / 1MB
        PeakPrivateMiB = $peakPrivate / 1MB
        FirstWindowSeconds = $firstWindowSeconds
        ProcessExitCode = $captureProcess.ExitCode
        Capture = $csvFiles[0].FullName
        Limitations = if ($Scenario -eq 'Driving') {
            'Driving benchmark, not a fixed idle view; frame cap active; other applications remain open; memory includes warm-up; first-window timing sampled at 250 ms.'
        } else {
            'Idle view, not driving; frame cap active; other applications remain open; memory includes warm-up; first-window timing sampled at 250 ms.'
        }
    }
    $summary | ConvertTo-Json | Set-Content -LiteralPath "$runRoot\Summary.json"
    $summary | ConvertTo-Json
    Write-Host "Profiling evidence: $runRoot"
    if ($captureProcess.ExitCode -ne 0) { throw "Profiler process exited $($captureProcess.ExitCode); retain CSV and inspect its log before accepting the run." }
} finally {
    foreach ($name in $previousEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $previousEnvironment[$name], 'Process')
    }
}
