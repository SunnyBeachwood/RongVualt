[CmdletBinding()]
param(
    [ValidateSet('desktop-create', 'desktop-verify', 'android-verify')]
    [string]$Phase,
    [string]$VeraCrypt = 'C:\Program Files\VeraCrypt\VeraCrypt.exe',
    [string]$Format = 'C:\Program Files\VeraCrypt\VeraCrypt Format.exe',
    [ValidatePattern('^[A-Za-z]$')]
    [string]$DriveLetter = 'D',
    [ValidateSet('AES', 'Serpent', 'Twofish', 'Camellia', 'AES-Twofish', 'AES-Twofish-Serpent', 'Serpent-AES', 'Serpent-Twofish-AES', 'Twofish-Serpent')]
    [string]$Cipher = 'AES',
    [ValidateSet('SHA-512', 'SHA-256', 'BLAKE2s-256', 'Whirlpool', 'Streebog', 'Argon2id')]
    [string]$Kdf = 'SHA-512',
    [ValidateRange(0, 2147468)]
    [int]$Pim = 1,
    [string]$Container,
    [string]$AndroidSerial = '192.168.1.101:44315'
)

$ErrorActionPreference = 'Stop'
$adb = Join-Path $env:LOCALAPPDATA 'Android\Sdk\platform-tools\adb.exe'
$password = 'EDS-TEST-desktop-interop'
$desktopPayload = [Text.Encoding]::UTF8.GetBytes('EDS Desktop interop marker v1')
$androidPayload = [Text.Encoding]::UTF8.GetBytes('EDS Android interop marker v1')

if (!(Test-Path -LiteralPath $VeraCrypt) -or !(Test-Path -LiteralPath $Format)) { throw 'VeraCrypt.exe and VeraCrypt Format.exe are required.' }
if (!(Test-Path -LiteralPath $adb)) { throw 'Android SDK platform-tools/adb.exe is required.' }
if ([string]::IsNullOrWhiteSpace($Container)) { throw 'Pass an explicit EDS-TEST-* container path; this helper will not choose or remove a path.' }
if ((Split-Path -Leaf $Container) -notlike 'EDS-TEST-*') { throw 'Only explicitly named EDS-TEST-* containers are allowed.' }

$driveRoot = "$DriveLetter`:\"
function Invoke-VeraCryptMount {
    $process = Start-Process -FilePath $VeraCrypt -ArgumentList @('/v', $Container, '/l', $DriveLetter, '/a', '/p', $password, '/pim', $Pim, '/hash', $Kdf, '/q', '/h', 'n', '/c', 'n') -Wait -PassThru
    if ($process.ExitCode -ne 0) { throw "VeraCrypt mount command failed (exit=$($process.ExitCode))." }
    Start-Sleep -Seconds 2
    if (!(Test-Path -LiteralPath $driveRoot)) { throw "VeraCrypt did not mount $Container as $driveRoot." }
}
function Dismount-VeraCrypt {
    $process = Start-Process -FilePath $VeraCrypt -ArgumentList @('/d', $DriveLetter, '/q') -Wait -PassThru
    if ($process.ExitCode -ne 0) { throw "VeraCrypt dismount command failed (exit=$($process.ExitCode))." }
    Start-Sleep -Seconds 1
    if (Test-Path -LiteralPath $driveRoot) { throw "VeraCrypt did not dismount $driveRoot." }
}
function Android-ExternalInteropDirectory {
    $package = 'app.rongvault'
    return "/sdcard/Android/data/$package/files/interop"
}

switch ($Phase) {
    'desktop-create' {
        if (Test-Path -LiteralPath $Container) { throw 'Refusing to overwrite an existing container.' }
        $parent = Split-Path -Parent $Container
        if (!(Test-Path -LiteralPath $parent)) { New-Item -ItemType Directory -Path $parent -Force | Out-Null }
        $process = Start-Process -FilePath $Format -ArgumentList @('/create', $Container, '/password', $password, '/pim', $Pim, '/hash', $Kdf, '/encryption', $Cipher, '/filesystem', 'ExFAT', '/size', '64M', '/force', '/silent') -Wait -PassThru
        if (!(Test-Path -LiteralPath $Container)) { throw "VeraCrypt Format did not create the requested container (exit=$($process.ExitCode))." }
        Invoke-VeraCryptMount
        try { [IO.File]::WriteAllBytes((Join-Path $driveRoot 'desktop-origin.bin'), $desktopPayload) } finally { Dismount-VeraCrypt }
        & $adb -s $AndroidSerial shell mkdir -p (Android-ExternalInteropDirectory)
        & $adb -s $AndroidSerial push $Container ((Android-ExternalInteropDirectory) + '/EDS-TEST-desktop-to-android.hc')
        Write-Output "Desktop-create complete for $Cipher / $Kdf / PIM $Pim. Next run Android desktopCreatedContainerOpensAndAcceptsAndroidMarker with -e vc.cipher=$Cipher -e vc.kdf=$Kdf -e vc.pim=$Pim."
    }
    'desktop-verify' {
        & $adb -s $AndroidSerial pull ((Android-ExternalInteropDirectory) + '/EDS-TEST-desktop-to-android.hc') $Container
        Invoke-VeraCryptMount
        try {
            $actual = [IO.File]::ReadAllBytes((Join-Path $driveRoot 'android-marker.bin'))
            if ([BitConverter]::ToString($actual) -ne [BitConverter]::ToString($androidPayload)) { throw 'Android marker mismatch.' }
            [IO.File]::WriteAllBytes((Join-Path $driveRoot 'desktop-return.bin'), $desktopPayload)
        } finally { Dismount-VeraCrypt }
        & $adb -s $AndroidSerial push $Container ((Android-ExternalInteropDirectory) + '/EDS-TEST-desktop-to-android.hc')
        Write-Output 'Desktop-created container round trip completed.'
    }
    'android-verify' {
        & $adb -s $AndroidSerial pull ((Android-ExternalInteropDirectory) + '/EDS-TEST-android-to-desktop.hc') $Container
        Invoke-VeraCryptMount
        try {
            $actual = [IO.File]::ReadAllBytes((Join-Path $driveRoot 'android-origin.bin'))
            if ([BitConverter]::ToString($actual) -ne [BitConverter]::ToString($androidPayload)) { throw 'Android-origin marker mismatch.' }
            [IO.File]::WriteAllBytes((Join-Path $driveRoot 'desktop-return.bin'), $desktopPayload)
        } finally { Dismount-VeraCrypt }
        & $adb -s $AndroidSerial push $Container ((Android-ExternalInteropDirectory) + '/EDS-TEST-android-to-desktop.hc')
        Write-Output 'Android-created container verified by desktop. Next run Android androidCreatedContainerReadsDesktopReturnMarker.'
    }
}
