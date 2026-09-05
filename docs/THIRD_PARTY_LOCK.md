# Third-party dependency lock

This file records source identities before native libraries enter a release
build. A staged checkout is not an imported dependency and must not be added
to CMake until its entire transitive closure is present and reviewed.

## Android runtime components

| Library | Locked version | License | Purpose | Status |
| --- | --- | --- | --- | --- |
| libsu service | `com.github.topjohnwu.libsu:service:5.2.2` | Apache-2.0 | RootService IPC launcher used by the Material Files RootablePath provider | source declaration and attribution added; Root behavior remains pending build/device verification |
| Apache FtpServer | `org.apache.ftpserver:ftpserver-core:1.2.1` | Apache-2.0 | User-enabled local FTP listener | source declaration and attribution added; protocol and lifecycle tests remain pending |
| Apache MINA | `org.apache.mina:mina-core:2.2.4` | Apache-2.0 | FtpServer transport dependency | version is pinned to the FtpServer 1.2.1 release line; build verification remains pending |

## ZipXtract archive engines (1.3.0)

| Component | Locked version | License | Scope | Status |
| --- | --- | --- | --- | --- |
| ZipXtract source mirror | `v7.1.1` / `c137bc1e07e8bea89f13dc26a938fd35df62918c` | GPL-3.0 | Read-only upstream reference; UI/Service/database code excluded from the port | source snapshot present; build and archive fixtures pending |
| Zip4j | `2.11.5` | Apache-2.0 | ZIP/JAR read, write, encryption and split output | source declaration present; verification pending |
| 7-Zip-JBinding-4Android | `Release-16.02-2.03` | LGPL-2.1+ / upstream notices | 7z, RAR/RAR5 and volume callbacks | arm64 packaging and runtime verification pending |
| Apache Commons Compress | `1.28.0` | Apache-2.0 | tar and compressor streams | source declaration present; verification pending |
| Brotli decoder | `0.1.2` | MIT | Brotli (`.br`) stream support used by Commons Compress | source declaration present; verification pending |
| XZ for Java | `1.10` | Public Domain / 0BSD | XZ/LZMA streams | source declaration present; verification pending |
| zstd-jni | `1.5.7-6` | BSD-2-Clause | Zstandard tar streams | arm64 packaging and runtime verification pending |
| libarchive Android | `1.1.6` | BSD-2-Clause | generic format fallback in the existing archive provider | existing provider dependency; integration verification pending |

## Botan arm64 amalgamation

| Library | Locked archive | SHA-256 | Required arm64 modules | Status |
| --- | --- | --- | --- | --- |
| Botan | `Botan-3.12.0.tar.xz` | `5370f98dc15f8c222ee1ce52cd61c8756a53be0dc57cc4c1b0714d5a09ad74fb` | CPUID, AES-ARMv8, SHA-256/SHA-512 base and ARMv8, AES, Serpent, Twofish, Camellia, Kuznyechik, XTS, BLAKE2s/BLAKE2b, PBKDF2, Argon2, Whirlpool, and Streebog | locked source and generated arm64 amalgamation; CMake marker/intrinsic gates enabled; 1.2.0 BLAKE2s regeneration remains pending build verification |

`tools/generate-botan-arm64.ps1` is the only supported regeneration entry
point. It must verify the archive digest, produce the arm64 amalgamation, and
prove the required implementation markers before the generated files replace
the vendored `vc_botan.*` pair.

## FAT/exFAT

FatFs is vendored in `third_party/fatfs`. The native bridge uses it only over
`EncryptedBlockDevice`; it must never receive a host path or a plaintext
temporary file.

## NTFS read-only candidate

| Library | Upstream repository | Locked staging revision | License | Purpose |
| --- | --- | --- | --- | --- |
| libfsntfs | `https://github.com/libyal/libfsntfs` | `2eccb7a4356e919d50bb7f8956a683131141f0d3` (`20260727`) | LGPL-3.0-or-later | NTFS metadata, directory and data-stream parsing |
| libbfio | `https://github.com/libyal/libbfio` | `9603beb63808f194447cf7529f2e8f558f99b7ae` (`20260623`) | LGPL-3.0-or-later | bounded I/O abstraction for the encrypted native block source |

The current staging checkout is linked to `vc_core` for the read-only NTFS
backend. Its configured dependency closure requires at least `libcerror`, `libcthreads`, `libcdata`,
`libclocale`, `libcnotify`, `libcsplit`, `libuna`, `libcfile`, `libcpath`,
`libbfio`, `libfcache`, `libfdata`, `libfdatetime`, `libfguid`, and `libfwnt`.
`libfusn` and `libhmac` are tool-oriented optional dependencies and must not
be pulled into the Android reader unless a concrete API path requires them.

Before import, record every library's immutable revision, archive SHA-256,
license text, SPDX identifier, local modifications, and CMake source list.
`tools/ntfs-dependencies.json` is the machine-readable source of the pinned
revision set. The complete source closure is checked out in
`third_party/libyal` and connected to CMake as `vc_libyal_ntfs`.
`tools/verify-ntfs-dependencies.ps1` must pass before CMake is allowed to
compile an NTFS reader.
The review must verify that Android builds use only the read APIs: no journal
replay, repair, volume mutation, EFS decryption, reparse-point traversal, or
external file access. Every mutation request must be rejected in Kotlin,
DocumentsProvider, Java adapter, JNI, and native backend.
