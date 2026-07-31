param(
    [Parameter(Mandatory = $true)]
    [string] $Destination
)

$ErrorActionPreference = 'Stop'
$target = [IO.Path]::GetFullPath($Destination)
if (Test-Path -LiteralPath $target) {
    if (-not (Get-Item -LiteralPath $target).PSIsContainer) { throw 'Destination must be a directory' }
} else {
    New-Item -ItemType Directory -Path $target | Out-Null
}

$revision = 'd26216c294fdfb090ee856f6195c294176defa1d'
$fixtures = [ordered]@{
    'test.blake2s.hc' = '332aa03c41f891aaa4d93be41548d2df8f3a6780fe4dfea6fe02658fba52d201'
    'test.sha256.hc' = '031d5cd7604a151be66af01e341409e327183663d00b96da9839352fc521b52e'
    'test.sha512.hc' = '78531ef7658d8bce818df2a526c656f2b3c9a00c171a45d66638ef88f9c7eb05'
    'test.streebog.hc' = 'd64c33aa228ec5d404740373150bde56f1311275a07266890ceedf200d40da13'
    'test.whirlpool.hc' = '3286e8ddf682206151235d5089451ce61eabad14c2e4e896846159da68b62317'
}

foreach ($entry in $fixtures.GetEnumerator()) {
    $path = Join-Path $target $entry.Key
    $uri = "https://raw.githubusercontent.com/veracrypt/VeraCrypt/$revision/Tests/$($entry.Key)"
    Invoke-WebRequest -Uri $uri -OutFile $path
    $actual = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $entry.Value) { throw "Checksum mismatch for $($entry.Key)" }
}

Write-Output "Verified VeraCrypt 1.26.29 test containers in $target"
