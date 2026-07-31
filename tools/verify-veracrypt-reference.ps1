[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateScript({ Test-Path -LiteralPath $_ -PathType Leaf })]
    [string]$ArchivePath
)

$expectedSha256 = '60826731e2982b4bd231e3930e85a44391169638671a1b200c518f8c8b46cb2a'
$expectedName = 'VeraCrypt_1.26.29_Source.tar.bz2'
$expectedCommit = 'd26216c294fdfb090ee856f6195c294176defa1d'

if ((Split-Path -Leaf $ArchivePath) -ne $expectedName) {
    throw "Expected $expectedName, not $(Split-Path -Leaf $ArchivePath)."
}

$actualSha256 = (Get-FileHash -LiteralPath $ArchivePath -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actualSha256 -ne $expectedSha256) {
    throw "Unexpected VeraCrypt source digest: $actualSha256"
}

Write-Output "Verified VeraCrypt 1.26.29 source archive SHA-256."
Write-Output "Expected signed tag commit: $expectedCommit"
Write-Output 'Verify the release detached signature with the upstream signing key before use.'
