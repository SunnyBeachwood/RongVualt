# RongVault (Android VeraCrypt Container Tool)

[中文说明](README.zh-CN.md)

RongVault is an offline Android application for creating, opening, and managing
VeraCrypt-compatible **file containers**. After a container is unlocked, RongVault
exposes it through Android's document-provider framework so files can be accessed
from the built-in file manager or the system Files app without decrypting the
entire container onto device storage.

> RongVault is still under development. Keep independent backups of your
> containers and volume headers. Do not use a test build as your only means of
> protecting data: a forgotten password, PIM, or key file cannot be recovered.

## What it is for

- Access files in VeraCrypt containers offline on an Android phone or tablet.
- Create standard or hidden volumes for storing sensitive files.
- Import and export encrypted volumes on local storage, SD cards, and USB media.
- Browse and manage files without keeping the whole container as a plaintext
  directory on device storage.

## Implemented features

- Android Storage Access Framework (SAF) selection for local, SD-card, and USB
  random-access container files.
- Add, unlock, lock, and remove containers from the app catalog. Removing a
  catalog entry does not delete the original container file.
- Standard and hidden volumes, including hidden-volume protection when writing
  to the outer volume.
- Password, PIM, and key-file authentication, with optional Android Keystore and
  biometric protection for saved outer-volume credentials.
- VeraCrypt non-system containers using AES, Serpent, Twofish, selected cascades,
  and supported KDF combinations; both primary and backup volume headers are
  recognized.
- FAT and exFAT read/write operations: create, rename, delete, copy, move, and
  import/export. NTFS is read-only.
- An Android `DocumentsProvider` for unlocked volumes and a foreground service
  while volumes are unlocked or long file operations are running.
- Built-in file manager with browsing, search, sorting, hidden-file display,
  creation, rename, deletion, and copy/move operations inside a volume.

## Platform and build requirements

| Item | Current scope |
| --- | --- |
| Operating system | Android 15 (API 35) or newer |
| CPU architecture | `arm64-v8a` (64-bit ARM) |
| Container sources | Random-access files on local, SD-card, or USB storage |
| Supported file systems | FAT / exFAT read/write; NTFS read-only |
| Build tools | JDK 17, Android SDK Platform 37, NDK 28.2.13676358 |

After configuring the Android SDK path in `local.properties`, build a debug APK:

```powershell
.\gradlew.bat :app:assembleLiteDebug
```

A release build also requires all `EDS_RELEASE_*` signing properties outside the
source tree and an HTTPS `RONGVAULT_SOURCE_URL`. The repository's signing key is
not used to create release packages.

## Limitations

- Supports VeraCrypt 1.26.29-compatible **non-system file containers** only; it
  does not support system encryption, physical partitions, or whole disks.
- TrueCrypt, LUKS, EncFS, PKCS#11, EMV, cloud drives, network locations, and
  other non-random-access container sources are not supported.
- NTFS volumes cannot be written to.
- Some cipher, KDF, cascade, hidden-volume, SAF-provider, and desktop-VeraCrypt
  combinations are still being validated. See the
  [compatibility matrix](docs/COMPATIBILITY_MATRIX.md) for current coverage.
- RongVault is not an official VeraCrypt client, is not affiliated with or
  endorsed by VeraCrypt, and has not received an independent cryptographic or
  security audit.

## Relationship with EDS Lite and VeraCrypt

### EDS Lite

RongVault uses **EDS Lite 2.0.0.237** as a migration baseline and retains selected
historical code and engineering experience from that project. The user interface,
AndroidX/Gradle build, SAF storage boundary, Kotlin application layer, and
`vc_core` native container implementation have been progressively rebuilt for
this migration.

RongVault is not an official EDS Lite release and does not reproduce all of its
features. It deliberately focuses on VeraCrypt non-system file containers on
Android rather than the additional formats and legacy path-access modes in EDS
Lite.

The [EDS Lite README](https://github.com/sovworks/edslite#license) states that it
is GPL-2.0-or-later, and its [GPLv2 license text](https://github.com/sovworks/edslite/blob/master/LICENSE)
is retained with applicable imported files. RongVault distributes the combined
work under GPL-3.0-or-later. If an imported file has a GPLv2-only or other
license notice, that notice governs the file and its compatibility must be
reviewed before combining it with GPLv3-only code.

### VeraCrypt

RongVault implements interoperability with the public VeraCrypt container format
and targets non-system file containers from VeraCrypt 1.26.29. It does not contain
the VeraCrypt desktop application and is not an official mobile client,
affiliated project, or certified implementation.

## Built-in file manager and Material Files

The built-in file manager adapts code, file-operation flows, and Material Design
patterns from [Material Files](https://github.com/zhanghai/MaterialFiles) 1.7.4.
The adapted portion is scoped to unlocked encrypted volumes:

- It accesses files through RongVault's `UnlockedDocumentsProvider`.
- Material Files' Root/Shizuku, network storage, FTP server, APK installer, and
  credential-storage features are not enabled here.
- File operations remain constrained by the encrypted-volume session and the
  foreground-operation protection; the app does not use a host plaintext temp
  directory for normal volume operations.

Material Files is licensed under
[GPL-3.0-or-later](https://github.com/zhanghai/MaterialFiles/blob/master/LICENSE).
Imported copyright and license notices are preserved, including the complete
license text and upstream source notes in
[`materialfiles/MATERIAL_FILES_UPSTREAM.md`](materialfiles/MATERIAL_FILES_UPSTREAM.md).

GPL is advance permission to copy, modify, and redistribute when its terms are
followed, so separate author permission is normally not required. GPL does not
grant permission to use project names, trademarks, or claims of official
endorsement.

## License

RongVault is distributed under the **GNU General Public License v3.0 or later
(GPL-3.0-or-later)**. When distributing a modified version or an APK, provide the
corresponding complete source code, retain copyright, license, and warranty
disclaimers, and do not impose restrictions that conflict with the GPL.

Third-party components may have additional license requirements; review the
license notices in each component before redistribution. Versions containing EDS
Lite source code must also retain the EDS Lite GPLv2 license text with the source.

See [LICENSE](LICENSE) for the full GPLv3 terms.
