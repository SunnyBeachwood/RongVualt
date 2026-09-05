# VeraCrypt 1.26.29 compatibility matrix

Status terms: **planned** means no compatibility claim; **verified** requires
both the listed VeraCrypt test vector and a desktop round trip.

The RongVault 1.2.0 algorithm/KDF expansion is source-complete but deliberately
remains **planned (待编译/待验证)** throughout this matrix until the independent
build and verification phase is run.

RongVault 1.3.0 adds the ZipXtract archive engine and native file-manager
actions. Archive rows are likewise **代码完成，未编译/未验证** until the
release-first verification pass is authorized.

The existing `CipherHint` names and numeric values, native request v2 framing,
and saved credential record field order are unchanged; this matrix tracks only
the newly exposed algorithms, KDFs, and capability coverage.

## ZipXtract archive matrix (1.3.0)

These rows describe the source-level routing; every row remains unverified until
the release-first build and fixture pass is authorized.

| Family | Read/extract route | Create/update | Status |
| --- | --- | --- | --- |
| ZIP/JAR/APK, standard and split | Zip4j; 7-Zip-JBinding for `.zip.001`/`.partN` | ZIP STORE/DEFLATE, Zip Standard/Strong/AES, optional split | code complete, not compiled/verified |
| 7z and RAR/RAR5, split | 7-Zip-JBinding volume callbacks | 7z create; unencrypted 7z update only | code complete, not compiled/verified |
| tar, tar.gz/bz2/xz/lzma/zst/lz4/br | Commons Compress + XZ/zstd/Brotli decoders | TAR family (Zstd level default 3) | code complete, not compiled/verified |
| ISO/CAB/DEB/RPM/DMG/WIM/CHM/CPIO and Lzip fallback | Existing libarchive reader through the read-only archive filesystem | never mutated by ZipXtract jobs | code complete, not compiled/verified |

Archive extraction rejects absolute/parent-traversing names, NULs, archive
symlinks and hard links. Password buffers are transient character arrays and
are wiped in the engine/job `finally` paths; archive entries are streamed rather
than cached as plaintext files.

## Test corpus provenance

| Item | Required value | Recorded value |
| --- | --- | --- |
| EDS Lite baseline | 2.0.0.237 upstream commit and archive SHA-256 | pending import provenance |
| VeraCrypt reference | 1.26.29 release tag/archive SHA-256 | Signed tag `VeraCrypt_1.26.29` → commit `d26216c294fdfb090ee856f6195c294176defa1d`; `VeraCrypt_1.26.29_Source.tar.bz2` SHA-256 `60826731e2982b4bd231e3930e85a44391169638671a1b200c518f8c8b46cb2a`. The pinned public SHA-512, BLAKE2s, Whirlpool, and SHA-256 vectors open on OnePlus PLF110 as both their normal and hidden headers. A serial Streebog check exceeded four minutes and a separate normal-header check exceeded five minutes without a result; Streebog remains pending. These are not desktop round trips. |
| Official test vectors | exact source URL/revision and SHA-256 | VeraCrypt `d26216c294fdfb090ee856f6195c294176defa1d` `Tests/test.{blake2s,sha256,sha512,streebog,whirlpool}.hc`; SHA-256 values fixed in `tools/fetch-veracrypt-test-containers.ps1` |
| Device matrix | Android version, ABI, SAF provider, filesystem | OnePlus PLF110, Android 16 / API 36, arm64-v8a. The current installed Debug APK SHA-256 is `AC66F19B6A344B7B8D7F29E9074AC15A73094DDF8D0F69DA58D1F053F5D50577`; it packages only `lib/arm64-v8a/libvc_core.so`. Debug native instrumentation passes invariant, exFAT file-operation, hidden-volume create/reopen with temporary-file cleanup, keyfile-required-reopen, Activity-backed hidden-volume-protection coverage, native creation cancellation (`CANCELLED` at the first progress callback), the official SHA-512 wrong-PIM rejection path, and DocumentsContract exFAT Provider mutation: directory/file creation, proxy-FD write, non-zero-offset read, sequential read, rename, truncate, delete, session close, and temporary-container cleanup. The current installation re-ran the Provider mutation in 13.769 seconds and cancellation in 0.025 seconds. Protected-range writes are rejected and the error remains latched. The foreground service was confirmed as active on device. SAF normal-volume creation, reopening, and DocumentsUI root enumeration were exercised. Desktop VeraCrypt 1.26.29 AES/exFAT round trips now pass both directions on `D:`: desktop-created -> Android writable open -> desktop marker read, and Android-created -> desktop mount/read/write -> Android marker read. Interactive DocumentsUI mutation, NTFS, and SAF hidden-volume UI remain pending. |

No test container or vector containing user data may be committed. Generated
fixtures are reproducible from scripts and stored outside version control.

The VeraCrypt tag signature was verified by GitHub when this baseline was
recorded. Download the source archive and its detached signature from the
release, verify both independently, then compare the archive digest above
before generating any compatibility fixture.

## Volume-header recognition

The source-pinned 1.26.29 non-system XTS suite registry is: AES, Serpent,
Twofish, Camellia, Kuznyechik, AES-Twofish, AES-Twofish-Serpent,
Serpent-AES, Serpent-Twofish-AES, Twofish-Serpent, Camellia-Kuznyechik,
Kuznyechik-Twofish, Camellia-Serpent, Kuznyechik-AES, and
Kuznyechik-Serpent-Camellia. The native request values retain the existing
on-disk enum order; only the user-facing labels follow VeraCrypt's official
outer-to-inner names. Upstream creation is enabled only for the first four
single-cipher choices and the five cascades AES-Twofish, AES-Twofish-Serpent,
Serpent-AES, Serpent-Twofish-AES, and Twofish-Serpent. The six Kuznyechik
combinations remain open-only choices.

The KDF recognition matrix includes PBKDF2-HMAC-SHA-512, PBKDF2-HMAC-SHA-256,
PBKDF2-HMAC-BLAKE2s-256, PBKDF2-HMAC-Whirlpool, PBKDF2-HMAC-Streebog, and
Argon2id. Creation defaults to PBKDF2-HMAC-SHA-512 but exposes every listed
KDF; Argon2id must fail with an explicit memory error rather than silently
changing its parameters.

| Cipher / cascade | PBKDF2-HMAC-SHA-512 | PBKDF2-HMAC-SHA-256 | PBKDF2-HMAC-BLAKE2s-256 | PBKDF2-HMAC-Whirlpool | PBKDF2-HMAC-Streebog | Argon2id | PIM | Keyfiles | Primary / backup header | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| AES | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| Serpent | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| Twofish | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| Camellia | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| Kuznyechik | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned (open-only) |
| AES-Twofish | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| AES-Twofish-Serpent | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| Serpent-AES | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| Serpent-Twofish-AES | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| Twofish-Serpent | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned |
| Camellia-Kuznyechik | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned (open-only) |
| Kuznyechik-Twofish | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned (open-only) |
| Camellia-Serpent | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned (open-only) |
| Kuznyechik-AES | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned (open-only) |
| Kuznyechik-Serpent-Camellia | planned | planned | planned | planned | planned | planned | planned | planned | planned | planned (open-only) |

The implementation must derive and validate VeraCrypt's Argon2 192-byte
output exactly. It must not silently substitute a weaker KDF or reduced memory
setting on constrained devices.

## Filesystem and storage behavior

| Capability | Required behavior | Status |
| --- | --- | --- |
| FAT | random read/write, truncate, rename/move, >4 GiB regression coverage | planned |
| exFAT | creation defaults; full formatting; random read/write | planned (Android native create/reopen, directory enumeration, random read/write, truncate, rename, and delete are device-tested; desktop round trip remains required) |
| NTFS | read-only; every mutation is rejected before native write | Android opened a desktop VeraCrypt AES/NTFS container read-only and app-owned decrypt export matched the desktop 512 KiB SHA-256 payload. Unit coverage rejects every mutation before JNI dispatch; writable import remains unavailable in the UI. |
| SAF source | only a seekable, known-length descriptor; `pread`/`pwrite`/`fsync`/`ftruncate` | planned |
| Concurrent opens | one writer per container; multiple readers allowed | planned |
| DocumentsProvider IDs | random volume UUID, never host path or container name | planned |

## Negative and interruption cases

- Wrong password, PIM, keyfile, KDF/cipher hint, malformed/corrupt header, and
  unsupported filesystem each map to a distinct user-facing domain error.
- Test primary-header damage with backup-header recovery, header-update power
  interruption, format cancellation, hidden-volume protection, medium removal,
  full storage, process death, lock screen, and automatic session close.
- Run native unit tests under ASan and UBSan, and fuzz header parsing and
  seekable-container boundary handling.

## Root and FTP coverage (1.2.0)

The following rows are deliberately source-only. They describe the required
checks for the next build/device phase and do not claim that a Root device or a
network client has been exercised yet.

| Area | Required behavior | Status |
| --- | --- | --- |
| Root strategy | Preserve `NEVER`/`AUTOMATIC`/`ALWAYS` enum order, setting key, and default `AUTOMATIC`; dispatch denied mounted paths through libsu service 5.2.2 | planned (pending build/device verification) |
| Root failure handling | Root unavailable, denied, timed out, or revoked returns an explicit error; no silent local fallback | planned (pending build/device verification) |
| Root file operations | Browse, read, create, edit, copy, move, rename, delete, permissions, and properties on mounted paths; read-only/AVB/kernel restrictions remain effective; no block-device writes/remount | planned (pending build/device verification) |
| FTP share roots | Persist ordinary/Root path in existing home-directory setting; keep one unlocked-volume path, runtime ID, and read-only state in process memory only | planned (pending build/device verification) |
| FTP listener | Apache FtpServer 1.2.1/MINA 2.2.4, all interfaces, port 1..65535 validation, account login/2121/read-only defaults, explicit anonymous login/write opt-ins, specialUse foreground service | planned (pending build/protocol verification) |
| FTP safety | Reject archives and FTP/SFTP/SMB/WebDAV roots; normalize paths, enforce `startsWith(homeDirectory)` boundary, reject `..` and symlink escapes | planned (pending unit/device verification) |
| FTP volume lifecycle | Capture immutable root at start; touch active volume session during operations; stop clients/server before volume close; unlocked-root service start is non-sticky | planned (pending device verification) |
