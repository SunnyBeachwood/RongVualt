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
- VeraCrypt non-system containers using all 15 VeraCrypt 1.26.29 XTS suites for
  opening and automatic detection; the nine Windows creation suites are exposed
  for new normal and hidden volumes. The 1.1.0 additions remain pending build
  and compatibility verification.
- FAT and exFAT read/write operations: create, rename, delete, copy, move, and
  import/export. NTFS is read-only.
- An Android `DocumentsProvider` for unlocked volumes and a foreground service
  while volumes are unlocked or long file operations are running.
- Built-in file manager with browsing, search, sorting, hidden-file display,
  creation, rename, deletion, and copy/move operations inside a volume.
- Markdown preview for `.md`, `.markdown`, and `.mkd` files, as well as standard
  Markdown MIME types. It is enabled by default and can be disabled in settings;
  the source remains available for editing.

## Cryptography and VeraCrypt compatibility scope

RongVault uses the VeraCrypt non-system-container design: XTS mode for encrypted
data, a password/PIM/keyfile-protected volume header, and a selected password
derivation function (KDF). RongVault 1.1.0 adds the complete reader registry,
nine Windows creation suites, five PBKDF2 variants, and Argon2id; these additions
are source-complete but remain pending compilation and verification:

The `CipherHint` names and numeric values, native request v2 framing, and saved
credential record field order remain unchanged for existing data. Only the
user-facing algorithm labels and supported capability lists are expanded.

| Area | RongVault 1.1.0 code scope (pending build/verification) | Desktop VeraCrypt comparison |
| --- | --- | --- |
| Data-cipher suites | All 15 VeraCrypt non-system XTS suites are registered for open/auto-detection. New normal and hidden volumes expose the nine Windows creation suites: AES, Serpent, Twofish, Camellia, AES-Twofish, AES-Twofish-Serpent, Serpent-AES, Serpent-Twofish-AES, and Twofish-Serpent. | VeraCrypt also supports system encryption and additional platform-specific layouts. Every RongVault suite remains unverified until the matrix is updated. |
| KDF choices | Open, create, hidden-volume protection, and credential-change code paths carry the six KDF hints: PBKDF2-HMAC-SHA-512 (default), SHA-256, BLAKE2s-256, Whirlpool, Streebog, and Argon2id. Hidden-volume protection keeps KDF auto-detection to preserve the native v2 layout; Argon2id shows its PIM-derived memory and iteration values. | VeraCrypt has broader, desktop-tested configuration coverage. Each RongVault KDF/cipher combination must be considered compatibility-dependent until it is marked verified in the matrix. |
| Volume type | Non-system file containers; standard and hidden volumes. | VeraCrypt additionally supports system encryption and can work with partitions and whole disks. |
| Filesystems | FAT and exFAT read/write; NTFS read-only. | VeraCrypt mounts volumes through desktop operating-system drivers and has a different host-filesystem integration model. |
| Platform | Android 15+ on `arm64-v8a`, using SAF and a `DocumentsProvider`. | VeraCrypt has official desktop releases for Windows, macOS, and Linux. |

The selectable cipher and KDF lists are not a blanket interoperability guarantee,
especially for Argon2id and less-common combinations. The 1.1.0 source changes
are intentionally marked **待编译/待验证 (pending build/verification)**. See the
[compatibility matrix](docs/COMPATIBILITY_MATRIX.md) for the recorded test
status. RongVault is not a replacement for desktop VeraCrypt when system
encryption, partitions, or desktop-level testing is required.

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

The built-in file manager vendors and adapts code, file-operation flows, and
Material Design patterns from [Material Files](https://github.com/zhanghai/MaterialFiles)
1.7.4. The following changes define the RongVault integration:

- File access is routed through RongVault's `UnlockedDocumentsProvider`, so the
  file manager sees only the active encrypted-volume session rather than host
  storage paths.
- Create, copy, move, delete, and rename operations are bound to the volume
  session and its foreground-operation protection; normal volume operations do
  not use a host plaintext temporary directory.
- Root/Shizuku access, network storage, the FTP server, APK installer, and
  Material Files credential storage are not initialized or exposed by RongVault.
- The app integrates the Material Files Markdown viewer. It detects `.md`,
  `.markdown`, and `.mkd` filenames or Markdown MIME types, renders formatted
  previews with Markwon (including tables, strikethrough, task lists, and local
  relative images), resolves relative in-volume links, and lets the user return
  to the source editor. Markdown rendering is a user setting and is on by
  default.

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
