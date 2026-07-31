param(
    [string] $Source = 'C:\Projects\VeraCrypt-master\Tests',
    [string] $Adb = 'C:\Users\QinQin\AppData\Local\Android\Sdk\platform-tools\adb.exe',
    [string] $Serial = '192.168.1.101:44315',
    [string] $Package = 'app.rongvault'
)

$ErrorActionPreference = 'Stop'
$names = 'test.sha512.hc', 'test.whirlpool.hc', 'test.sha256.hc', 'test.blake2s.hc', 'test.streebog.hc'

& $Adb -s $Serial shell "run-as $Package mkdir -p files/veracrypt-vectors"
if ($LASTEXITCODE -ne 0) { throw "Could not prepare the private test-vector directory for $Package" }

foreach ($name in $names) {
    $local = Join-Path $Source $name
    if (-not (Test-Path -LiteralPath $local -PathType Leaf)) { throw "Missing VeraCrypt test vector: $local" }
    & $Adb -s $Serial push $local "/data/local/tmp/$name"
    if ($LASTEXITCODE -ne 0) { throw "Could not stage $name on the device" }
    & $Adb -s $Serial shell "run-as $Package cp /data/local/tmp/$name files/veracrypt-vectors/$name"
    if ($LASTEXITCODE -ne 0) { throw "Could not move $name into the app-private test directory" }
    & $Adb -s $Serial shell "rm -f /data/local/tmp/$name"
}

Write-Output "Staged VeraCrypt 1.26.29 public test vectors in $Package private storage."
