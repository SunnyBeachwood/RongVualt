# Native third-party lock

This file records source identities before native libraries enter a release
build. A staged checkout is not an imported dependency and must not be added
to CMake until its entire transitive closure is present and reviewed.

## Botan arm64 amalgamation

| Library | Locked archive | SHA-256 | Required arm64 modules | Status |
| --- | --- | --- | --- | --- |
| Botan | `Botan-3.12.0.tar.xz` | `5370f98dc15f8c222ee1ce52cd61c8756a53be0dc57cc4c1b0714d5a09ad74fb` | CPUID, AES-ARMv8, SHA-256-ARMv8, SHA-512-ARMv8, AES, Serpent, Twofish, XTS, and the shipped KDF modules | locked source and generated arm64 amalgamation; CMake marker/intrinsic gates enabled |

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

The current staging checkout is intentionally not linked. Its configured
dependency closure requires at least `libcerror`, `libcthreads`, `libcdata`,
`libclocale`, `libcnotify`, `libcsplit`, `libuna`, `libcfile`, `libcpath`,
`libbfio`, `libfcache`, `libfdata`, `libfdatetime`, `libfguid`, and `libfwnt`.
`libfusn` and `libhmac` are tool-oriented optional dependencies and must not
be pulled into the Android reader unless a concrete API path requires them.

Before import, record every library's immutable revision, archive SHA-256,
license text, SPDX identifier, local modifications, and CMake source list.
`tools/ntfs-dependencies.json` is the machine-readable source of the pinned
revision set. The complete source closure is checked out in
`third_party/libyal`, but it is not yet connected to CMake.
`tools/verify-ntfs-dependencies.ps1` must pass before CMake is allowed to
compile an NTFS reader.
The review must verify that Android builds use only the read APIs: no journal
replay, repair, volume mutation, EFS decryption, reparse-point traversal, or
external file access. Every mutation request must be rejected in Kotlin,
DocumentsProvider, Java adapter, JNI, and native backend.
