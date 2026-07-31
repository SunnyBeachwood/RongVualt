# Android VeraCrypt migration baseline

This repository is the imported EDS Lite 2.0.0.237 source tree.  The target is
an Android 10+ container-only application interoperable with VeraCrypt
1.26.29.  This document fixes the migration boundary; it is not a claim that
the legacy implementation is VeraCrypt compatible.

## Baseline captured 2026-07-27

| Area | Current source | Target |
| --- | --- | --- |
| Build | AGP 3.3.2, compile/target SDK 28, Java 8, NDK r13b | AGP 9.3.1, Gradle 9.5, JDK 17, SDK 37, NDK 28.2 |
| UI | Java, Support Library, RxJava | New Kotlin Fragment/ViewModel/XML screens, AndroidX, coroutines/Flow |
| Crypto | Java plus small JNI cipher libraries | isolated C++ `vc_core`; Botan-backed, opaque native session handles |
| Filesystems | Java FAT implementation | C++ FatFs FAT/exFAT read/write and libfsntfs/libbfio NTFS read-only adapters |
| Containers | EDS, TrueCrypt, LUKS, legacy VeraCrypt-related code | VeraCrypt 1.26.29 non-system file containers only |
| Storage | path-oriented and legacy providers | SAF descriptor-backed seekable containers and a privacy-safe DocumentsProvider |

The original `misc/keys.keystore` was a public debug key guarded by the
password `android`.  It is deliberately removed from the source tree. Debug
builds must use the Android Gradle Plugin's per-developer default debug key;
release builds will be enabled only by injected signing configuration.

## Non-negotiable compatibility boundary

Supported inputs are seekable local, SD-card, or USB **file containers** made
for VeraCrypt 1.26.29. Unsupported inputs must fail before a volume is opened:
system-encrypted devices, raw partitions, cloud/network sources, TrueCrypt,
LUKS, EncFS, PKCS#11, and EMV. Legacy source code is retained only as a
reference during the staged replacement; it must not expose those formats in
the new product UI.

## Sequenced implementation

1. Modernize the build in a dedicated, compiling commit: wrapper, repositories,
   AndroidX conversion, namespace/application ID, SDK/NDK, and release signing
   gate. Do not mix it with native crypto changes.
2. Add Kotlin domain contracts and SAF validation. Implement `SeekableContainer`
   around a duplicated `ParcelFileDescriptor`; validate seekability, known
   length, and requested write capability before JNI is called.
3. Introduce a single `vc_core` shared library. Its JNI ABI accepts descriptors
   and request structs, returns an opaque `jlong` session, and maps every native
   failure to the domain error taxonomy. It never returns a password or key.
4. Implement and test volume headers, KDFs/keyfiles, XTS, hidden-volume
   protection, and FAT/exFAT before connecting the Java file-manager adapter.
   Add NTFS only as read-only.
5. Replace container UI flows with Kotlin Fragment/ViewModel screens, then
   rebuild the DocumentsProvider around random volume UUIDs and path-free IDs.
6. Complete interoperability, fault-injection, sanitizer/fuzz, licensing, and
   independent cryptography/memory-safety reviews before release.

## Implemented foundation

- The build declaration now requests AGP 9.3.1, JDK 17 bytecode, SDK 37,
  minSdk 29, targetSdk 37 and NDK 28.2.13676358; it has no JCenter or Support
  Library dependency. AndroidX source imports and XML names were migrated.
- The new install identity defaults to `app.rongvault` and is overridable
  only through `EDS_APPLICATION_ID`; the legacy Java namespace is intentionally
  retained while Java file-management code is still in place.
- Release tasks require all four `EDS_RELEASE_*` properties and cannot fall
  back to a checked-in key. The application disables backup and removes broad
  external-storage write permission; storage grants are SAF based.
- Kotlin domain contracts define repository operations, request validation,
  session state, error taxonomy and a seekable SAF descriptor boundary. The
  `vc_core` JNI surface exposes only opaque handles and descriptor/block/header
  operations. Current source includes header/KDF/XTS code and a FatFs bridge;
  it remains explicitly unverified and is not a VeraCrypt-compatible release.

## Required evidence before each promotion

- An immutable upstream EDS source reference and VeraCrypt 1.26.29 vector
  bundle are recorded by commit ID and SHA-256 in `docs/COMPATIBILITY_MATRIX.md`.
- Android recognizes every header combination in that matrix; Android-created
  or modified containers reopen in desktop VeraCrypt.
- APK inspection shows no saved plaintext secrets, bundled signing keys,
  external native-library loading, sensitive logging, backups, or over-exported
  providers.
- Release signing is supplied outside source control. A missing configuration
  must make the release task fail.

## Deliberately deferred changes

There is no safe mechanical upgrade from Support Library/AGP 3 to AndroidX/AGP
9. The legacy project has thousands of `android.support.*` references and old
variant APIs, so changing only version numbers would produce a non-buildable
tree. The first modernization change must therefore include an automated
AndroidX migration and a compile-fix pass, with the legacy build retained as a
separately tagged baseline.
