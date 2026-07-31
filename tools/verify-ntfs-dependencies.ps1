param(
    [string]$LockFile = (Join-Path $PSScriptRoot 'ntfs-dependencies.json'),
    [string]$SourceRoot = (Join-Path (Split-Path $PSScriptRoot -Parent) 'third_party\libyal')
)

$lock = Get-Content -LiteralPath $LockFile -Raw | ConvertFrom-Json
if ($lock.schema -ne 1) { throw "Unsupported NTFS dependency lock schema" }

foreach ($library in $lock.libraries) {
    if ([string]::IsNullOrWhiteSpace($library.revision)) {
        throw "NTFS dependency '$($library.name)' has no immutable revision"
    }
    $directory = Join-Path $SourceRoot $library.name
    if (-not (Test-Path -LiteralPath (Join-Path $directory '.git'))) {
        throw "NTFS dependency '$($library.name)' is not a pinned source checkout"
    }
    $actual = (& git -C $directory rev-parse HEAD).Trim()
    if ($actual -ne $library.revision) {
        throw "NTFS dependency '$($library.name)' revision mismatch: expected $($library.revision), got $actual"
    }
    if ((& git -C $directory status --porcelain)) {
        throw "NTFS dependency '$($library.name)' has uncommitted changes"
    }
}

Write-Output 'All NTFS dependencies are revision-locked and clean.'
