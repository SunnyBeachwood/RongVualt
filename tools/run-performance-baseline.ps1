[CmdletBinding()]
param(
    [ValidateSet('Fast', 'Full')]
    [string]$Mode = 'Fast',
    [string]$Case,
    [switch]$SkipBuild,
    [string]$Serial = '192.168.1.101:44315'
)

$ErrorActionPreference = 'Stop'
$workspace = Split-Path -Parent $PSScriptRoot
$javaHome = 'C:\Program Files\Android\Android Studio\jbr'
$adb = 'C:\Users\QinQin\AppData\Local\Android\Sdk\platform-tools\adb.exe'
$package = 'app.rongvault'
$statusRemote = "cache/benchmark/status.json"
$reportDir = Join-Path $workspace 'app/build/reports/benchmark'
$statusLocal = Join-Path $reportDir 'status.json'
$markdownLocal = Join-Path $reportDir 'summary.md'
$apk = Join-Path $workspace 'app/build/outputs/apk/lite/benchmark/app-lite-benchmark.apk'
$gitCommit = (& git -C $workspace rev-parse HEAD 2>$null | Select-Object -First 1)
if (!$gitCommit -or $gitCommit -notmatch '^[0-9a-fA-F]{40}$') { $gitCommit = 'uncommitted' }

if (!(Test-Path -LiteralPath $adb)) { throw "ADB not found: $adb" }
$env:JAVA_HOME = $javaHome
$env:ANDROID_SERIAL = $Serial
New-Item -ItemType Directory -Force -Path $reportDir | Out-Null

if (!$SkipBuild) {
    Push-Location $workspace
    try { & .\gradlew.bat assembleLiteBenchmark } finally { Pop-Location }
}
if (!(Test-Path -LiteralPath $apk)) { throw "Benchmark APK not found: $apk" }
& $adb -s $Serial install -r -t $apk | Out-Host
& $adb -s $Serial shell am force-stop $package | Out-Null
& $adb -s $Serial shell run-as $package rm -rf cache/benchmark
$arguments = @('start', '-n', "$package/org.eds.veracrypt.benchmark.PerformanceBenchmarkActivity", '--es', 'mode', $Mode)
if ($Case) { $arguments += @('--es', 'case', $Case) }
& $adb -s $Serial shell am @arguments | Out-Host

$deadline = (Get-Date).AddMinutes(20)
$staleDeadline = (Get-Date).AddSeconds(60)
$lastUpdate = $null
$last = $null
while ((Get-Date) -lt $deadline) {
    Start-Sleep -Seconds 2
    try {
        $text = (& $adb -s $Serial shell run-as $package cat $statusRemote 2>$null) -join "`n"
        if ($text) {
            $last = $text | ConvertFrom-Json
            if ($last.updatedWallNs -ne $lastUpdate) {
                $lastUpdate = $last.updatedWallNs
                $staleDeadline = (Get-Date).AddSeconds(60)
            }
            if ($last.state -in @('SUCCESS', 'FAILED')) { break }
        }
    } catch { }
    if ((Get-Date) -ge $staleDeadline) {
        & $adb -s $Serial shell am force-stop $package | Out-Null
        & $adb -s $Serial shell run-as $package rm -f cache/EDS-TEST-protocol-*.hc | Out-Null
        throw "Benchmark made no progress for 60 seconds; the app was stopped and EDS-TEST protocol containers were removed."
    }
}
if ($null -eq $last) { throw 'Benchmark timed out after 20 minutes or produced no status.' }
$last.metadata | Add-Member -NotePropertyName gitCommit -NotePropertyValue $gitCommit -Force
$last | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $statusLocal -Encoding UTF8
$rows = foreach ($item in $last.results) {
    "| $($item.name) | $($item.status) | $([math]::Round([double]$item.elapsedNs / 1e9, 3)) | $([math]::Round([double]$item.p95ElapsedNs / 1e9, 3)) | $([math]::Round([double]$item.relativeStdDev * 100, 2))% | $([math]::Round([double]$item.throughputBytesPerSecond, 2)) | $($item.pssKiB) | $($item.thermalStatus) |"
}
$comparison = foreach ($item in $last.results | Where-Object { $_.name -eq 'stage2-compare-aes' }) {
    "", "## Stage 2 path comparison", "", "Legacy median: $([math]::Round([double]$item.legacySeconds, 3)) s; Serial batched median: $([math]::Round([double]$item.serialSeconds, 3)) s; Speedup: $([math]::Round([double]$item.speedup, 2))x", "Legacy syscalls (read/write): $($item.legacyReadSyscalls)/$($item.legacyWriteSyscalls); Serial syscalls (read/write): $($item.serialReadSyscalls)/$($item.serialWriteSyscalls)"
}
@("# Performance baseline ($Mode)", "", "Device: $($last.metadata.manufacturer) $($last.metadata.model), Android API $($last.metadata.androidApi), ABI $($last.metadata.abi), Git $($last.metadata.gitCommit)", "", "| Case | Status | Median seconds | P95 seconds | Elapsed RSD | Bytes/s | Peak PSS KiB | Thermal |", "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |") + $rows + $comparison + @("", "Final state: $($last.state)") | Set-Content -LiteralPath $markdownLocal -Encoding UTF8
if ($last.state -ne 'SUCCESS') { exit 1 }
