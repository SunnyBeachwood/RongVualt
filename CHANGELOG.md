# Changelog

## Unreleased

### File transfer progress

- Added an application-owned multi-file transfer layer for encrypted imports
  into writable FAT/exFAT volumes and decrypted exports from an unlocked
  volume. Transfers use a reused 256 KiB buffer, preserve the existing native
  encryption boundary, process one task at a time, and never create plaintext
  temporary files.

- Added real byte progress with known-total percentages, unknown-total
  indeterminate progress, recent-window speed and remaining-time estimates.
  The details page now exposes “加密导入” and “解密导出”, a live progress
  card, per-file failure summary, and cancellation. Direct lock is disabled
  while a transfer is active.

- Extended the existing foreground service notification with direction,
  current filename, file ordinal, progress and a cancel action. Android 13+
  notification permission is requested when a transfer starts; transfers still
  run when permission is declined.

- Added SAF source/target authority checks, automatic same-name suffixes,
  single-file failure continuation, fatal target/volume failure handling,
  cancellation cleanup, and a small cleanup journal for outputs left by a
  process kill. Directory recursion is intentionally not supported.

- Internal unlocked-volume targets now use a hidden partial filename while
  copying and are finalized with `DocumentsContract.renameDocument` only after
  the output stream closes successfully. External providers retain their
  final-name fallback and cleanup journal behavior.
- Fixed the details-page rendering so known percentages and remaining seconds
  display their numeric values instead of literal interpolation text.
- Focused unit tests, Lite Debug lint, Debug APK, and Debug AndroidTest APK
  all pass after the transfer implementation changes.
- Installed the resulting Debug APK on the connected OnePlus PLF110/API 36
  device and confirmed `ContainerCatalogActivity` launches successfully.
- Preserved coroutine cancellation exceptions across blocking source reads and
  target writes so cancellation performs whole-task cleanup instead of being
  misreported as an individual file failure. Re-ran the focused unit-test,
  lint, and Debug build gate successfully.
- Added and ran a real-device instrumentation round trip for encrypted import:
  a 512 KiB host file was copied into a newly formatted AES/exFAT `EDS-TEST-*`
  volume through the unlocked `DocumentsProvider`, reached `Completed`, and
  was read back byte-for-byte. The temporary volume and source were removed.
- Extended that device test through decrypt export as well. A Debug-only test
  `DocumentsProvider` supplies a controlled external SAF sink; the exported
  file reached 100% and matched the original 512 KiB payload byte-for-byte.
  The provider is absent from Release and the test output is deleted after the
  run.
- Hardened transfer completion: output streams are flushed and synced before
  a file is marked successful, metadata-query failures fall back to per-file
  handling, and cancellation during task startup now reaches `Cancelled`.
  Re-ran the full focused Gradle gate and the real-device import/export test
  successfully.
- Normalized `OpenDocumentTree` tree URIs to plain parent document URIs at the
  `createDocument` boundary while retaining tree URIs for child enumeration.
  The device test now copies two files through import and export with both
  payloads verified byte-for-byte; the export reached 100%.
- The same device test now includes a repeated source URI; the second output
  is automatically named with the stable `(1)` suffix and is also exported and
  verified byte-for-byte.
- Final focused gate passed: `testLiteDebugUnitTest`, `lintLiteDebug`,
  `assembleLiteDebug`, and `assembleLiteDebugAndroidTest`. Release assembly
  remains intentionally blocked without the externally supplied
  `EDS_RELEASE_*` signing properties.
- Added a controllable slow-source device regression. Cancelling during an
  active 8 MiB import reaches `Cancelled`, leaves no visible partial output in
  the unlocked volume, and keeps the foreground transfer state observable.
- Added an intentional source-open failure to the multi-file device case;
  successful files continue, the task reports `PartialSuccess` with one
  failure, and the final failure list is retained in progress state.
- Extended the cancellation regression to send the activity to the Home
  screen while the slow transfer is active. The current filename/byte progress
  remains observable through the foreground task, and cancellation still
  leaves no residual output.
- Fixed source-open exceptions so a revoked/unreadable individual source is
  cleaned up and reported in `PartialSuccess` while later files continue; the
  final progress snapshot now retains the failure list. The focused unit/Lint/
  Debug/AndroidTest gate passes after this change.
- Extracted the transfer-action UI policy and covered it by unit test: NTFS is
  export-only, while writable FAT and exFAT expose both import and export.
- Added a pending, explicitly ignored transfer-level hidden-protection test
  scaffold. Both FAT and exFAT protected outer fixtures currently fail to
  remount after hidden creation; the existing native raw-block
  hidden-protection regression remains active.
- Created an explicit temporary desktop VeraCrypt AES/NTFS container, mounted
  it on `D:`, wrote a SHA-256-pinned 512 KiB file, then verified Android opens
  it read-only and completes app-owned decrypt export with an identical hash.
  The desktop and device `EDS-TEST-*` fixtures and export were removed after
  the test.
- Re-ran the native invariant self-test on device and the final focused Gradle
  gate (`testLiteDebugUnitTest`, `lintLiteDebug`, `assembleLiteDebug`, and
  `assembleLiteDebugAndroidTest`) successfully.
- Extended the slow-transfer regression through a short screen-off/screen-on
  interval. The foreground transfer stayed active with live progress and then
  cancelled cleanly without a residual output.

- Added unit coverage for known/unknown totals, bounded percentages and
  long-integer overflow, stable duplicate-name allocation, and failure
  summaries. The focused build gate passed through
  `testLiteDebugUnitTest`, `lintLiteDebug`, and `assembleLiteDebug`.

### UI

- Replaced the catalog's selected-row and bottom-button interaction with a
  lifecycle-aware Material `RecyclerView` of scrollable container cards.
  Cards now show locked, unlocked read/write, read-only, or hidden-volume
  protection status and expose only the operations valid for that state:
  unlock, browse, details, lock, conditional hidden-volume creation, and a
  removal menu with the existing non-destructive confirmation.

- Added an unlocked-only container-details page. It reads the active native
  session and displays a safe summary of status, volume type, filesystem,
  access mode, logical size, sector size, cipher, KDF, and backup-header use.
  It deliberately excludes passwords, PIM, keyfiles, SAF URIs, paths and raw
  physical offsets. If the session closes, the page clears its content and
  shows a locked-state return action.

- Moved “Open system files” into the toolbar overflow beside Help and About,
  keeping catalog-wide utilities separate from actions for a specific
  container. Added state-mapping tests that cover locked, writable outer,
  read-only, protection-triggered, and safe-detail-model cases.

- Concentrated validation passed: `testLiteDebugUnitTest`, `lintLiteDebug`,
  `assembleLiteDebug`, and `assembleLiteDebugAndroidTest`. The rebuilt
  `app.rongvault` APK was installed on the connected Android 16 device;
  UIAutomator confirmed the new Chinese catalog title, global controls,
  `RecyclerView` card, locked Chip, primary unlock action and more-actions
  control without opening or changing a volume.

- Replaced the launcher artwork with the supplied RongVault shield/key image
  (`drawable-nodpi/rongvault_icon.png`) and wired it as both the standard and
  round application icon.

- Changed the default Android application ID from `org.eds.veracrypt` to
  `app.rongvault`. Vector staging, performance, and desktop interop scripts
  now target the new ID; Kotlin source namespaces remain unchanged to avoid a
  risky package-wide refactor.

- Rebuilt and installed the new `app.rongvault` package on the connected test
  device. The package manager reports the new launcher activity and version
  `0.1.0-dev`; the legacy package remains installed separately until its test
  data is intentionally migrated or removed.

- Upgraded the catalog shell to a Material 3 day/night theme with RongVault
  branding, deep-blue/teal color tokens, edge-to-edge inset handling, a
  Material toolbar, and local Help/About pages available from the toolbar
  menu. Chinese and English names now use “容匣” and “RongVault”.

- Refreshed the catalog layout with status/list cards, Material buttons,
  improved empty-state presentation, and an explicit removal confirmation that
  explains the source container file is not deleted. The unlock screen now
  uses Material cards and password-toggle text fields while preserving the
  existing binding IDs and crypto/session behavior.

- Applied the same Material password-toggle fields and spacing to normal and
  hidden-volume creation forms; existing ViewBinding IDs and creation progress
  callbacks are unchanged.

- Concentrated UI build gate passed after the modernization batch:
  `testLiteDebugUnitTest`, `lintLiteDebug`, `assembleLiteDebug`, and
  `assembleLiteDebugAndroidTest`.

- Installed the rebuilt APK on the connected OnePlus PLF110 (Android 16/API
  36) and confirmed `ContainerCatalogActivity` starts. UIAutomator observed
  the Chinese RongVault toolbar, overflow menu, and local “使用说明” content;
  no volume session was opened during this smoke check.

- Added an “打开系统文件”/“Open system Files” button to the container
  catalog. It launches Android’s `ACTION_OPEN_DOCUMENT` picker directly and
  does not persist a selection or request any additional SAF permission.

- Installed the updated APK on the connected device and verified the button is
  visible in the Chinese catalog; tapping it opened the Android DocumentsUI
  picker (`com.android.documentsui/.picker.PickActivity`).

### Security

- Added opt-in saved-unlock credentials for the container catalog. A successful
  manual password-only open can now be explicitly saved behind Android
  biometric or device-credential authorization; the persisted record contains
  only AES-GCM ciphertext protected by an Android Keystore key that requires
  user authentication and is invalidated by biometric enrollment. Password,
  PIM, resolved cipher/KDF, volume kind and access mode are decoded only into
  a short-lived buffer for the immediate JNI open request and are wiped on all
  normal and error exits. Keyfile-based opens and hidden-volume-protection
  credentials are deliberately refused for saving, and no additional SAF
  permissions are requested.

- The open screen now exposes an unchecked "save with biometric" choice and,
  when an opt-in record exists, a biometric unlock action. Authentication
  cancellation, enrollment/key invalidation, malformed records and Keystore
  failures leave manual password unlock available; unusable records are
  removed. Removing a catalog container also removes its associated encrypted
  credential record. Added codec contract tests for round-trip data, opaque
  per-container record IDs, and rejection of keyfile/hidden-protection
  captures. `testLiteDebugUnitTest` and `lintLiteDebug` passed.

- Unified manual and biometric opens onto one lifecycle-scoped suspend path so
  the biometric action cannot race a still-running native open. The path now
  disables both unlock actions until completion and closes decoded credential
  objects on decode, cancellation, failure and successful hand-off. Rebuilt
  `assembleLiteDebug`; unit tests and Lint remain green.

- The saved-credential Keystore key now explicitly uses per-use authentication
  with `BIOMETRIC_STRONG | DEVICE_CREDENTIAL` on API 30+, while retaining
  biometric-enrollment invalidation. The concentrated gate
  (`testLiteDebugUnitTest`, `lintLiteDebug`, `assembleLiteDebug`, and
  `assembleLiteDebugAndroidTest`) passed after this change.

- Installed the resulting `liteDebug` APK on the connected OnePlus PLF110
  (`192.168.1.101:44315`) and launched `ContainerCatalogActivity` successfully.
  The remaining verification is the user-mediated biometric prompt on a real
  saved container; it is intentionally not automated because it requires the
  device owner’s biometric or lock-screen credential.

- Opened the installed APK to the real container unlock screen on the same
  device. The Chinese UI renders the opt-in save checkbox and leaves it
  unchecked by default; no password or authentication choice was injected by
  automation.

- Localized the system authentication prompt title so Chinese users see a
  clear saved-volume authentication message instead of a hard-coded English
  label.

- Rebuilt and reinstalled the localized `liteDebug` APK on the connected
  device; the focused unit-test, Lint and APK build gate passed again.

- Fixed the real-device biometric-unlock failure diagnosis: fingerprint and
  Keystore authorization had succeeded, but the same container was already
  mounted read-write, so the session-arbitration guard rejected a second open
  and its generic I/O error reached the UI. Catalog Open is now disabled for
  an unlocked entry, and any stale open screen reports that the volume is
  already unlocked and must be browsed or locked first.
  `testLiteDebugUnitTest`, `lintLiteDebug`, `assembleLiteDebug`, and
  `assembleLiteDebugAndroidTest` passed for the change.

- Reinstalled the fix after the user safely locked the prior session, then ran
  the real saved-credential regression on OnePlus PLF110. Fingerprint
  authentication opened the saved `container.hc` successfully: the catalog
  displayed it as unlocked, the non-exported foreground service was active,
  and Open was disabled while Browse and Lock were enabled. The encrypted
  credential record remained present; no password, keyfile or host path was
  written to the catalog.

- Added an app-level foreground privacy lock. After the catalog activity has
  gone to the background, it covers the UI and requires a new per-use Android
  Keystore biometric/device-credential proof before showing app content again.
  The app-lock key is independent of saved volume credentials and contains no
  volume secret. Cancellation or failure leaves an opaque lock screen with an
  explicit retry action; prompt lifecycle transitions do not relock the app.
  The lock overlay is screenshot-protected before the app enters background.
  `testLiteDebugUnitTest`, `lintLiteDebug`, `assembleLiteDebug`, and
  `assembleLiteDebugAndroidTest` passed.

- Installed the foreground-lock build on OnePlus PLF110 and verified the real
  lifecycle: launched the app, sent it to Home, reopened it, observed the
  system prompt titled "验证身份以继续使用", then completed fingerprint
  authentication. The opaque lock path released back to the container catalog
  only after authentication succeeded.

### Performance

- Added the stage-1 Benchmark APK protocol without changing crypto, cache, or
  filesystem execution. `PerformanceBenchmarkActivity` is merged only into
  `liteBenchmark`, holds a bounded WakeLock, runs Fast/Full case lists on a
  worker, atomically persists status after every case, records monotonic wall
  and CPU time, PSS, thermal state, correctness-checked throughput and device
  metadata, and finishes headlessly. Added
  `tools/run-performance-baseline.ps1` with `-Mode Fast|Full`, `-Case`,
  `-SkipBuild`, and `-Serial`; it installs once, polls every two seconds with
  a 20-minute cap, exports JSON/Markdown under `app/build/reports/benchmark/`,
  and returns non-zero on failed or timed-out cases. SAF and NTFS are reported
  as explicit skipped cases when no fixture/URI is configured; generated
  reports remain outside version control. Exported reports now also include
  the current Git commit (or `uncommitted`) alongside device and APK metadata.
  The runner now performs two warmups followed by three Fast or five Full
  measured trials per applicable case and exports per-trial data plus median,
  P95, and relative standard deviation; skipped external-fixture cases are not
  repeated. It now emits an atomic heartbeat after every warmup or measured
  sub-run; the PowerShell controller treats 60 seconds without a new heartbeat
  as invalid, force-stops the benchmark process, and removes only its
  `cache/EDS-TEST-protocol-*.hc` containers.

- Completed the Stage-1 Fast and Full device baselines on OnePlus PLF110
  (Android 16/API 36, arm64-v8a). All implemented native, AES/Serpent/Twofish,
  encrypted I/O, FAT/exFAT format, SHA-512 KDF and AUTO wrong-password cases
  passed with thermal status 0. The formal Full medians are AES 110.15 MB/s,
  Serpent 41.13 MB/s, Twofish 57.95 MB/s, and AES end-to-end I/O 100.09 MB/s;
  exact P95/PSS/CPU data are in `docs/PERFORMANCE_BASELINE.md`. SAF copy and
  NTFS read correctly remain `SKIPPED` without a configured URI/fixture. FAT
  and exFAT formatting variance exceeded 10% and is recorded as hot-state
  follow-up, not treated as a core implementation regression.

- Stage-2 device comparison passed on OnePlus PLF110: the Benchmark-only
  `LEGACY_SECTOR` path measured 0.962 s versus 0.359 s for `SERIAL_BATCHED`
  over a verified 64 MiB AES write/read, a 2.68x median speedup. Syscalls fell
  from 262,144 reads/131,072 writes to 256 reads/256 writes. AES/Serpent/Twofish
  boundary round trips, aligned-write no-read assertions, hidden-volume
  protection, SHA-512 normal/hidden vector opens and native self-tests passed.
  The production path has no session thread pool or plaintext cache; legacy
  JNI is compiled only into Benchmark. The 3x speedup acceptance target is not
  yet met at 64 MiB; the 4 MiB smoke result (8.30x) is retained only as a
  diagnostic and is not used for acceptance. The comparison case accepts 64
  MiB in Fast and 256 MiB in Full.

- Hardened the serial batched write cleanup path: plaintext staging buffers are
  now wiped on both successful and exceptional transform/I/O exits before the
  exception propagates.

- Started Stage 3 hardware-acceleration gating: CMake now refuses an arm64
  build when the Botan amalgamation lacks CPUID, AES-ARMv8, SHA-256-ARMv8,
  SHA-512-ARMv8 markers or the corresponding NEON intrinsics. Benchmark-only
  JNI now reports the runtime AES and SHA-512 Botan providers so a real device
  result can distinguish hardware dispatch from a software-only build.
  The regeneration script also normalizes configure.py's temporary absolute
  build-path comment so repeated generation from the same archive is byte-stable.
  Added a `botan-provider` Benchmark case that reports the runtime AES and
  SHA-512 provider names on the connected arm64 device.

- Confirmed Stage-3 hardware dispatch on the connected OnePlus PLF110 through
  the Benchmark APK: Botan reported `aes=armv8aes` and
  `sha512=armv8sha2_512`, with no `base` provider. The result is recorded in
  `docs/PERFORMANCE_BASELINE.md`; AES/SHA-512 before/after performance deltas
  remain a separate pending measurement.

- Began Stage 2 by adding the focused `Stage2CoreIoInstrumentationTest` for
  serial aligned/unaligned boundaries, AES/Serpent/Twofish round trips and
  aligned-write syscall assertions. The production session no longer creates
  the four-lane executor or plaintext page cache; it uses one session mutex,
  one 256 KiB serial batch buffer, session-lifetime Botan XTS modes and
  expanded serial-batching counters. A benchmark-only Legacy JNI path is
  available for the upcoming same-APK comparison; Debug/Release production
  paths remain serial-batched.
  The Stage 2 test now also acquires the same foreground-operation lease used by
  existing device regressions so OEM process management cannot invalidate a
  long cipher boundary run.

### Compatibility

- Fixed the Android-created VeraCrypt header's required-program version from
  invalid `0x0630` to VeraCrypt 1.26.29's `VERSION_NUM` `0x0126`. The former
  value made desktop VeraCrypt reject the Android-created container as needing
  a future application before it assigned the requested `D:` drive. The
  rebuilt APK passed the complete desktop VeraCrypt 1.26.29 AES/exFAT round
  trip in both directions on `D:`: desktop-created -> Android writable open ->
  desktop marker read, and Android-created -> desktop mount/read/write ->
  Android marker read. The Android creation and both marker-validation tests
  passed under the foreground-operation lease.

### Performance

- Added opt-in, device-only cipher-matrix and soak-test coverage. The 256 MiB
  sequential/random benchmark now takes `vc.benchmark.cipher=AES|SERPENT|TWOFISH`
  (two warmups plus five measured trials) without changing the production JNI
  surface. A separate `vc.sustained=true` test holds one normal encrypted
  session for five minutes of full logical write/read cycles, checks every byte,
  keeps the foreground lease, records five-second PSS/thermal progress, and
  retains only non-sensitive app-private diagnostics. The first Serpent matrix
  completed on OnePlus PLF110: five valid measurements, no watchdog timeout or
  data mismatch, thermal status 0 throughout; the median write/read/random
  results are recorded in `docs/PERFORMANCE_BASELINE.md`.

- Completed the first AES sustained-device regression on OnePlus PLF110. The
  test held one 256 MiB encrypted session for 331.781 seconds (the five-minute
  window plus a final complete integrity cycle), completed eight full
  write/read byte-verification cycles, and transferred 2,145,386,496 bytes in
  each direction. It recorded no watchdog timeout or mismatch; peak PSS was
  84,650 KiB and Android thermal status remained 0. The temporary container
  was closed and removed after completion. Detailed scope and counters are in
  `docs/PERFORMANCE_BASELINE.md`.

- Added the Android side of a deliberately opt-in desktop VeraCrypt round-trip
  regression plus `tools/run-desktop-veracrypt-interop.ps1`. It only accepts
  explicit `EDS-TEST-*` containers, refuses overwrites, and validates marker
  bytes in both directions using AES/exFAT, PBKDF2-HMAC-SHA-512 and PIM 1.
  `assembleLiteDebugAndroidTest` passed. Desktop container creation succeeds,
  and, after explicitly closing stale VeraCrypt GUI processes, the desktop
  helper mounts it on the confirmed unused `D:` drive. The first direction
  passed end to end: desktop-created AES/exFAT -> Android writable open ->
  desktop re-open/read of the Android marker. After correcting the required
  program version, the reverse Android-created AES/exFAT container also passed
  desktop mount/read/write and Android marker readback on `D:`. The Android
  interop tests now hold the normal activity/foreground-operation lease so OEM
  process management cannot invalidate a creation result.

- Stage-1 Fast protocol execution on OnePlus PLF110 passed self-test, AES /
  Serpent / Twofish XTS correctness and AES 256 MiB I/O, with thermal status 0;
  its initial `format-fat-128m` stop was a Benchmark-only assertion error: FAT
  correctly reports mount type `1`, while the runner incorrectly required the
  exFAT value `2`. Corrected the type-specific assertion without changing FAT
  formatting or mounting implementation; Fast will be rerun before Full.
  JSON/Markdown output is kept in the ignored
  `app/build/reports/benchmark/` directory.

- Replaced the arm64 Botan amalgamation with one generated from the official
  Botan 3.12.0 archive after verifying its locked SHA-256
  `5370f98dc15f8c222ee1ce52cd61c8756a53be0dc57cc4c1b0714d5a09ad74fb`.
  The generated minimal module set retains every supported cipher and cascade
  and now contains CPUID, AES-ARMv8, SHA-256-ARMv8, and SHA-512-ARMv8.
  `assembleLiteBenchmark` passed, and arm64 disassembly contains `aese`
  instructions. The generator now uses verified Python extraction on Windows,
  an explicit Android NDK compiler, an absolute build/prefix path, and
  normalized amalgamation include naming; it rejects a replacement if any
  required hardware marker is absent. The regenerated Debug APK passed the
  native invariant, AES/Serpent/Twofish parallel-XTS, and hidden-volume
  create/reopen/protection regressions on OnePlus PLF110 (three tests, zero
  failures).

- Added a session-owned, adaptive four-lane XTS executor for aligned native
  read/write blocks of 64 KiB or larger. The calling thread and three lazily
  created workers each own distinct Botan cipher-mode state; encrypted
  container I/O remains one bounded `pread`/`pwrite` operation, and only the
  in-memory 512-byte-unit transforms run concurrently. Small reads, cache
  fills, and unaligned read-modify-write edges remain serial. Each session
  wipes its reusable write scratch buffer and joins/wipes its XTS lanes when
  it closes. Debug native self-tests now compare serial and parallel output
  plus decrypt round trips across all 15 implemented cipher suites and
  cascades; the public Android creation regression covers AES, Serpent, and
  Twofish, the three cipher suites currently accepted by `VolumeCreateOptions`.
  Debug/benchmark counter snapshots now additionally expose aggregate
  parallel-XTS block and sector totals, without exposing data or keys.
  `assembleLiteDebug`, `assembleLiteBenchmark`, and
  `assembleLiteDebugAndroidTest` passed after the change; the Debug native
  invariant self-test passed on OnePlus PLF110. The activity-backed public
  AES/Serpent/Twofish 128 KiB round-trip regression also passed there in
  13.169 seconds, with temporary test-container cleanup verified.
- Hardened the opt-in 256 MiB device benchmark with parseable per-stage start,
  finish, failure, and timeout records. Native full-format progress and every
  benchmark I/O chunk now heartbeat a 60-second watchdog; timeout requests
  native creation cancellation and marks the trial invalid. Results remain in
  the app-private `EDS-TEST-benchmark-last.txt` diagnostic artifact until
  collected, while failed containers are closed and removed in `finally`.
- Completed the first stable five-run 256 MiB AES/exFAT benchmark on OnePlus
  PLF110 (Android 16/API 36) after four-lane XTS integration. Median full
  format was 5.970 s, sequential write 2.719 s (94.06 MiB/s), sequential read
  3.891 s (65.73 MiB/s), and 4,096 random 4 KiB reads 1.136 s (3,607 ops/s).
  All stages completed without a watchdog timeout and thermal status remained
  normal. Raw run values, PSS caveat, I/O counters, and scope limitations are
  recorded in `docs/PERFORMANCE_BASELINE.md`; this is not a controlled
  before/after improvement claim because the preceding serial trial was
  interrupted.
- Re-ran the hidden-volume create/reopen/protected-outer-write regression on
  OnePlus after the parallel-XTS change. It passed in 15.671 seconds: hidden
  data remained readable, a write intersecting its protected range was
  rejected, the protection error remained latched for later writes, and raw
  reads remained available.
- ARMv8 Botan acceleration remains deliberately blocked: the required locked
  `Botan-3.12.0.tar.xz` archive was not found locally, so the vendored
  amalgamation was not replaced and no hardware-provider macro was forced.
  The connected OnePlus PLF110 exposes `aes`, `sha2`, `sha512`, and `cpuid`,
  so the hardware validation will proceed once the SHA-256-verified archive
  is supplied.

- Added an opt-in, device-only 256 MiB native I/O benchmark instrumentation
  suite. Each of two warmups and five measured runs formats a temporary
  `EDS-TEST-*` exFAT container, then measures 256 MiB sequential write/read
  and 4 KiB random reads. It records wall/CPU time, process PSS, Android
  thermal state, cache hit/miss totals, and native container syscall/byte
  totals in parseable logcat records. The suite requires the explicit
  `vc.benchmark=true` instrumentation argument and holds the normal foreground
  operation lease throughout. `assembleLiteDebugAndroidTest` passes for the
  new opt-in suite.
- The opt-in benchmark now mirrors each non-sensitive, parseable result line
  into an app-private `EDS-TEST-benchmark-last.txt` artifact. This preserves
  complete five-run evidence on Android 16 devices whose wireless-ADB
  instrumentation transport ends before returning trailing logcat output;
  the artifact contains timings, aggregate counters, PSS, and thermal state
  only—never container paths, volume data, passwords, or keys.
- Began the five-run 256 MiB sequential/random I/O test on the connected
  OnePlus PLF110 (Android 16/API 36). The first warmup completed with a
  6.090 s full format, 5.662 s sequential write, 5.519 s sequential read,
  and 1.278 s for 4,096 random 4 KiB reads (PSS 79,245 KiB; thermal status
  0). The second warmup then remained in native formatting for over two
  minutes at roughly 3% CPU instead of its expected tens-of-seconds duration.
  The foreground process was deliberately stopped and its incomplete
  app-cache `EDS-TEST-*` container removed. This is recorded as an abnormal
  interrupted run, not a performance result; five-run medians remain pending.

- Added Debug/benchmark-only native performance counters. A live session can
  report aggregate encrypted-container `pread`/`pwrite` attempts and completed
  bytes plus plaintext-cache hit/miss totals; counter values contain no paths,
  plaintext, credentials, or key material. The JNI query is unavailable from
  release builds. `testLiteDebugUnitTest`, `assembleLiteDebug`, and
  `assembleLiteBenchmark` passed after the native integration. The native
  exFAT instrumentation regression now verifies a first 4 KiB read is a cache
  miss with encrypted container I/O and a second identical read is a cache hit
  without an additional container read. That direct-JNI regression now starts
  the app activity and holds the normal foreground-operation lease for its
  full-format duration, matching production long-running behavior on devices
  that aggressively stop background instrumentation. The cache assertion uses
  a middle-of-volume page rather than page zero, which mounting may already
  have populated. On OnePlus PLF110 (Android 16/API 36), the complete
  `exfatSessionSupportsCoreFileOperations` instrumentation regression passed
  in 7.441 seconds with the cache/counter assertions enabled.
- Tightened the performance-counter boundary: release builds now omit the
  counter JNI symbol entirely rather than exporting a method that only rejects
  calls at runtime.

- Expanded the foreground-operation lease to cover container probing as well as
  volume opening, formatting/hidden-volume creation, capacity scans,
  credential changes, and header backup/restore. Consequently, potentially
  slow SAF, SD-card, or USB provider probing now starts the same privacy-safe
  `dataSync` foreground service used by the other native operations. An
  unlocked session keeps that service active for `DocumentsProvider` I/O until
  it is explicitly closed. Added a nested-lease unit test so an inner operation
  cannot stop foreground execution while its outer operation is still active.
- Added an explicit KDF selector to the unlock screen. Per-normal/per-hidden
  catalog KDF hints now select its initial value, while users can override the
  hint or select `AUTO`; the selected value, rather than a hidden catalog
  default, is now passed to the open request. `testLiteDebugUnitTest`,
  `lintLiteDebug`, and `assembleLiteDebug` passed after this UI change.

- Replaced per-512-byte-unit Botan XTS mode construction, key scheduling, and
  secure-vector copying with a native session-owned `XtsTransformContext`.
  It preserves VeraCrypt's physical-offset data-unit numbering, restarts the
  tweak per 512-byte data unit, and wipes the Botan contexts on session
  destruction. Full aligned reads now issue up to 256 KiB `pread` blocks;
  full aligned writes encrypt and issue up to 256 KiB `pwrite` blocks, while
  only non-aligned edges use read-modify-write. Hidden-volume protection now
  checks the entire affected encrypted range before any write.
- Rebuilt and installed the arm64 Debug APK on OnePlus PLF110 (Android 16 /
  API 36). The existing activity-backed DocumentsProvider exFAT mutation
  regression passed in 6.308 seconds after the I/O change. The prior 13.769
  second result is not a controlled five-run baseline, so no performance
  target is claimed from this comparison.
- Added the installable `benchmark` build type. It uses the Debug signing key
  and disables shrink/minification, while native `vc_core` uses `-O3`,
  ThinLTO, hidden symbols, and section garbage collection. It is explicitly
  not a publishable release build; the existing release-signing gate remains
  unchanged. `assembleLiteBenchmark` succeeds and packages only arm64
  `libvc_core.so`.
- Audited the currently vendored Botan 3.12.0 arm64 amalgamation before
  enabling hardware acceleration. It contains ARM target flags but omits the
  CPUID, AES-ARMv8, SHA-256-ARMv8, and SHA-512-ARMv8 implementation modules;
  attempting to force those macros fails compilation because `CPUID` is not
  present. The forcing configuration was removed. Hardware acceleration is
  therefore pending regeneration from the locked Botan source, rather than
  being falsely claimed by configuration macros.
- Added `tools/generate-botan-arm64.ps1`, which requires a local Botan 3.12.0
  archive matching the locked SHA-256, generates in a temporary directory,
  checks the four required hardware-module markers, and refuses to replace
  vendored files without explicit `-Replace`.
- Added a Proxy FD file-length cache. Each opened file obtains its current
  length once, `onGetSize()` then serves the cached value, and successful
  writes extend that value. Truncate opens begin at zero. A stale document
  node initially exposed a zero-length sequential read; the callback now
  refreshes length at open time. The complete OnePlus exFAT Provider mutation
  regression passed after the fix in 6.559 seconds.
- Installed the `benchmark` APK on OnePlus PLF110 and used the Debug-signed
  instrumentation APK against its matching application ID. After two warmup
  runs, five `DocumentsProvider` exFAT mutation runs were 1.331, 1.329,
  1.328, 1.331, and 1.274 seconds (median 1.329 seconds). This is a small
  Provider workflow microbenchmark only; it is recorded as an initial
  repeatability check, not as the required 256 MiB sequential/random I/O or
  full-format performance baseline. The rebuilt current benchmark APK
  SHA-256 is `28727D3F18783DEE459A3C85B85EAF399069EF74DDFBF7EA2E477979BD854672`.
- Added the native session-owned 1 MiB `SecureBlockCache` (64 wiped 16 KiB
  pages). Random reads below 64 KiB use LRU pages shared by FAT/exFAT and
  NTFS through `NativeVolumeSession`; larger reads bypass it. Writes invalidate
  overlapping plaintext pages before I/O, and hidden-volume protection or
  session destruction clears all pages. The current exFAT Provider regression
  passed on OnePlus in 7.595 seconds after the cache change.
- Upgraded the persisted container catalog to backward-compatible `entries.v2`.
  Entries can now retain separate successful normal/hidden cipher and KDF
  hints; v1 or missing fields remain AUTO. Only resolved native metadata is
  stored—never passwords, PIM values, keyfiles, headers, or plaintext paths.
  The open screen applies the matching hint for normal or hidden selection,
  and a hint-write failure cannot prevent an otherwise successful unlock.
  The combined catalog/cache build was installed on OnePlus PLF110; the
  activity-backed exFAT DocumentsProvider regression passed in 7.550 seconds.

### Added

- Added a Debug-only native self-test bridge and Android instrumentation test.
  The runner now links the header-layout, KDF-parameter, XTS data-unit, and
  NTFS libbfio callback invariants into `vc_core` without shipping them in
  release builds.
- Added a generic native filesystem type and file-handle boundary shared by
  FAT/exFAT and the new read-only NTFS adapter.
- Added pinned libyal dependency metadata and an Android CMake integration
  layer for `libfsntfs`, `libbfio`, and their required dependency closure.
- Added a native `NtfsVolume` adapter that reads from the decrypted volume
  session through libbfio callbacks without exposing a host path, SAF URI, or
  plaintext temporary file.
- Added NTFS directory enumeration, metadata lookup, and random file reads;
  EFS-marked files and reparse points are rejected explicitly.

### Changed

- Refactored NTFS libbfio seek and read-range validation into checked helpers.
  The adapter now rejects negative, overflowing, and out-of-volume callback
  requests before dispatching an encrypted-volume read.
- Tightened the Kotlin filesystem boundary so read-only and NTFS sessions
  reject writable opens, creation, truncation, directory creation, deletion,
  and renames before any JNI call.
- Unified native NTFS write, truncate, and writable/create/truncate open
  requests behind a tested `kReadOnlySource` rejection path.
- Configured the Gradle wrapper to use Tencent Cloud's Gradle mirror and the
  build's plugin and dependency repositories to use Aliyun Maven mirrors.
- Disabled Android cloud backup and device-to-device data extraction explicitly
  for API 31+, and declared RTL support in the new product manifest.
- Added the local Android SDK path required for this workstation's Gradle
  builds; the build now recognizes Android SDK Platform 37.0, NDK
  28.2.13676358, and CMake 3.22.1.
- Enabled Jetifier for the legacy RxLifecycle dependency and updated Kotlin
  SAF keyfile expansion to use `Context` for `DocumentFile` access.
- Updated proxy file-descriptor callbacks for the current Android API,
  preserved coroutine cancellation propagation, and corrected native-backed
  append positioning and the retired external-storage permission callback.
- Replaced legacy Java `switch (R.id...)` dispatches with resource-ID
  comparisons compatible with AGP 9's non-final resource IDs.
- Extended libyal CMake header generation to cover common and implementation
  configuration templates, resolve `@PACKAGE@`, and expose internal
  cross-library headers during Android native compilation.
- Replaced the FAT-specific native mount JNI entry point with
  `nativeMountFileSystem`; filesystem type values are stable across the JNI
  boundary.
- Routed mounted filesystem operations through a filesystem-neutral native
  file handle and directory-entry type.
- Integrated NTFS dispatch into native volume sessions and report NTFS mounts
  as read-only to Kotlin, DocumentsProvider, and the legacy Java filesystem
  facade.
- Explicitly reject NTFS writes, creation, truncation, deletion, and renames
  in Kotlin and native code. FatFs formatting also rejects an NTFS request.
- Corrected libyal CMake include ordering so each library resolves its own
  generated `common.h` and `types.h` before dependency public headers.
- Replaced permissive libyal Autoconf template substitution with an explicit
  Android API 29 capability map. CMake now fails if a newly introduced
  template token is not registered, instead of silently configuring it as
  unavailable.
- Configured the embedded libyal closure for Android libc, POSIX I/O and
  pthread support. Wide-character code paths are explicitly disabled because
  Android's 32-bit `wchar_t` is unsupported by this pinned libyal revision.
- Added original libyal public include directories as a fallback after the
  generated configuration headers, so non-template public headers remain
  available to `vc_core`.
- Removed legacy EDS cipher, hash, local-XTS and file-descriptor JNI targets
  from the lite native build. The lite APK now packages only `libvc_core.so`.
- Corrected the two non-arm64 Botan amalgamations to include their checked-in
  `vc_botan.h` configuration header.
- Updated the embedded NTFS adapter to release local-mode libyal errors
  through `libcerror_error_free`, matching the symbols exported by the static
  dependency closure.
- Replaced the lite launcher with a Kotlin AppCompat SAF container catalog.
  It stores only persisted `content://` grants and random catalog IDs, and
  opens selected containers through the native repository without retaining
  credentials.
- Added a sensitive Kotlin unlock screen for normal or hidden AES, Serpent and
  Twofish volumes with PIM and read-only/read-write selection. Successful
  opens register application-owned sessions with `UnlockedDocumentsProvider`.
- Added a session-to-provider-root bridge so the UI can send DocumentsUI to a
  live unlocked volume without accessing host paths, SAF container URIs or
  opaque document IDs.
- Moved imported EDS Java UI and resources to `app/src/reference/` and made
  the lite variant consume only the new Kotlin product surface and
  `src/veracrypt/res`. Legacy TrueCrypt, LUKS, EncFS, file-manager and old JNI
  entry points are no longer packaged or linted by lite builds.
- Added a SAF `ACTION_CREATE_DOCUMENT` normal-volume flow. It collects a
  capacity, password, PIM, AES/Serpent/Twofish selection and FAT/exFAT choice,
  requests native full formatting, removes failed catalog entries, and
  registers successful sessions under their random catalog IDs.

### Verification

- Switched the Lite Debug product to the Android 15+ arm64-v8a target: its
  `minSdk` is now 35, `abiFilters` contains only `arm64-v8a`, and CMake
  rejects any other ABI before selecting Botan. Removed the Lite product's
  unused RxJava/RxLifecycle, DrawerLayout, Multidex, and metadata-extractor
  dependencies along with Jetifier and the legacy non-final-resource-ID flag.
- Added request-scoped SAF keyfile controls to normal-volume open and creation:
  multi-file and one-level directory selection, persisted read grants,
  selection removal, and user-selected 64 KiB random keyfile generation.
  The UI clears keyfile selections and password objects when an operation ends.
- Added catalog actions for browsing and locking an unlocked container, plus a
  hidden-volume creation screen. Capacity analysis temporarily revokes the
  outer DocumentsProvider root and its proxy FDs; successful creation replaces
  that root with the dependent hidden session, while a failed creation restores
  the still-valid outer root.
- Added optional hidden-volume protection credentials to writable outer-volume
  unlocks, including separate password, PIM and SAF keyfiles. A native
  `HiddenVolumeRisk` now changes the active session to read-only, refreshes
  DocumentsProvider roots, and presents a remount-required message when the
  user returns to the catalog.

- Confirmed the local Android build toolchain uses Android SDK Platform 37,
  NDK 28.2.13676358, CMake 3.22.1, and Android Studio JBR 21.0.10. Android
  API 37 is used only as the compile/target platform; the product minimum SDK
  is API 35.
- Ran `git diff --check` and verified all pinned NTFS dependencies with
  `tools/verify-ntfs-dependencies.ps1`.
- Ran `assembleLiteDebug` against Android SDK Platform 37.0 and NDK
  28.2.13676358. It now completes and produces the arm64-only Debug APK at
  `app/build/outputs/apk/lite/debug/app-lite-debug.apk`.
- Verified the merged native libraries contain only
  `lib/arm64-v8a/libvc_core.so`; no other ABI or legacy EDS native library is
  packaged.
- Ran `testLiteDebugUnitTest`, `tools/verify-ntfs-dependencies.ps1`, and
  `git diff --check` successfully.
- Ran `assembleLiteDebug`, `testLiteDebugUnitTest`, and `lintLiteDebug` under
  the installed Android Studio JBR 21.0.10 after source-set isolation. The
  merged lite manifest exposes `ContainerCatalogActivity` and
  `UnlockedDocumentsProvider`, not the legacy EDS launcher or provider.
- Re-ran `assembleLiteDebug`, all 14 `testLiteDebugUnitTest` tests, and
  `lintLiteDebug` after adding the SAF normal-volume creation screen.
- Ran `assembleLiteDebugAndroidTest` successfully after connecting native
  self-tests and again after adding NTFS callback-boundary checks. Both the
  app and instrumentation APK package all three supported ABIs.
- Ran `testLiteDebugUnitTest` after the read-only mutation guard. All 15 JVM
  tests passed, including the new NTFS mutation-rejection regression test.
- Re-ran `assembleLiteDebugAndroidTest` after the native NTFS mutation guard;
  the Debug instrumentation APK again linked and packaged all three ABIs.
- Installed the Debug app and instrumentation APK on an Android API 36
  `arm64-v8a` physical device. `VcCoreInstrumentationTest` passed (1/1),
  executing the native header-layout, KDF, XTS, NTFS callback, and mutation
  rejection self-tests in the packaged `libvc_core.so`.
- Ran `lintLiteDebug` with JBR 21 after the manifest update: it completed with
  zero errors. The API 31 data-extraction-rule warning is resolved; remaining
  output consists of non-blocking warnings that still need individual review.
- The current arm64-only Debug artifact is
  `app/build/outputs/apk/lite/debug/app-lite-debug.apk` (SHA-256:
  `E2E8D1CC2C59AB348D65EE4D212A9D55462ECBDFF682C55B825AB2A34A1822F0`). It
  is unsigned for release distribution and must not be treated as a completed
  production build.
- A OnePlus PLF110 physical device is connected at `192.168.1.101:44315`
  (Android API 36, `arm64-v8a`). The prior native instrumentation result of
  1/1 passing was run on this device. The contradictory earlier statement that
  no ADB device was attached is superseded by this record.
- Re-ran `testLiteDebugUnitTest`, `lintLiteDebug`, `assembleLiteDebug`,
  `assembleLiteDebugAndroidTest`, and `connectedLiteDebugAndroidTest` with
  `ANDROID_SERIAL=192.168.1.101:44315`; all passed. The final arm64 Debug APK
  was installed and the catalog activity rendered on the same device.
- DocumentsUI file-operation workflows, cancellation/interruption tests,
  sanitizers, fuzzing, and VeraCrypt desktop interoperability remain pending.
  The hidden-volume UI is reachable but has not yet received a real-container
  device or desktop-interoperability validation.
- Fixed the FatFs result adapter so `FR_OK` returns normally instead of being
  converted into an I/O interruption. This restores FAT/exFAT directory reads
  and all other successful FatFs operations after volume creation or unlock.
- Preserved non-secret FatFs failure context through JNI as its numeric
  `FRESULT`, allowing device diagnostics to distinguish a real filesystem
  failure from an adapter error without exposing credentials or paths.
- On the Android API 36 arm64 physical device, created and reopened a 16 MiB
  AES/exFAT `EDS-TEST-normal-20260728.hc` container through the SAF UI.
  DocumentsUI successfully listed its empty root after the fix. The test
  session was closed and the temporary container was removed afterwards.
- Added a Debug-only FatFs result self-test. It verifies that `FR_OK` returns
  normally and that a real disk error remains an `IoInterrupted` core error.
- Added an Android instrumentation test for a private 16 MiB exFAT test
  container. On OnePlus PLF110 (Android API 36, arm64-v8a), it completed
  creation, root enumeration, directory and file creation, random readback,
  truncate, rename, and deletion in 21 seconds, then removed its temporary
  `EDS-TEST-native-filesystem.hc` file.
- Re-ran the arm64 Debug matrix on `192.168.1.101:44315`: JVM tests, Lint,
  Debug APK, Debug instrumentation APK, and native instrumentation passed.
  The current Debug APK SHA-256 is
  `C4BC4EF32AA922ACB7E3047FC464FD9B4844C35CB061515858EE0754F99C9C9B`.
  This remains an unsigned-for-release Debug artifact.
- Added a device instrumentation hidden-volume workflow using the private
  `EDS-TEST-hidden-workflow.hc` temporary container. On OnePlus PLF110
  (Android API 36, arm64-v8a), it created a 64 MiB AES/exFAT outer volume,
  scanned its available hidden capacity, created a 16 MiB AES/exFAT hidden
  volume, mounted it, created a private directory, closed both sessions, and
  reopened the hidden volume by its distinct credentials. The original
  workflow passed in 134 seconds. The test now asserts that its temporary
  container is removed during cleanup; rerunning that assertion is pending
  because the device package manager became unresponsive while replacing the
  instrumentation APK. This is Android-native coverage only; SAF
  hidden-volume UI, protection, and desktop round-trip verification remain
  pending.
- Re-ran `hiddenVolumeCanBeCreatedAndReopened` on OnePlus PLF110 with its
  temporary-container cleanup assertion enabled. The 64 MiB outer / 16 MiB
  hidden AES/exFAT workflow passed in 91.91 seconds: capacity analysis,
  hidden creation, mount, private-directory creation, session closure, hidden
  credential reopening, and private-cache cleanup all completed successfully.
- Added the `keyfileIsRequiredToReopenVolume` device regression. It creates a
  private 64 KiB keyfile and 16 MiB AES/exFAT volume, verifies a keyfile-backed
  reopen, rejects an otherwise identical request without the keyfile as
  `InvalidCredentialsOrFormat`, and removes both test inputs. It passed on the
  same device in 126.976 seconds.
- Fixed `NativeRequestCodec.encodeOpen` for writable outer-volume requests
  with hidden-volume protection. The encoded-request allocation had omitted
  the six bytes for protection PIM and protection-keyfile count, causing a
  `BufferOverflowException` before JNI. Added a JVM regression test that
  validates the complete protected-request layout and its 36-byte fixture.
- Added `VolumeForegroundService` for active unlocked sessions and full
  creation/formatting operations. It is declared as a non-exported
  `dataSync` foreground service, uses a privacy-safe `SECRET` notification,
  and reports only anonymous volume counts or formatting activity. The normal
  and hidden creation Fragments hold this state for their whole coroutines;
  session-list changes update it automatically.
- Installed the new arm64 Debug APK on OnePlus PLF110 and confirmed through
  `dumpsys activity services` that `VolumeForegroundService` had
  `isForeground=true`, type `dataSync`, an ongoing service notification, and
  `SECRET` visibility while the hidden-volume workflow ran.
- The OnePlus OPlus process manager still terminated the long no-Activity
  instrumentation process as `Cached(nirvana)[(instrumentation){fg-service}]`
  after several minutes, despite the verified foreground service. This is a
  device test-host limitation, not a native crash.
- Re-ran the full hidden-volume protection E2E case from an Activity-backed
  test host on OnePlus PLF110. It passed in 45.131 seconds: an outer session
  opened with hidden-volume protection rejected both the initial and repeated
  protected-range writes as `HIDDEN_VOLUME_RISK`, while a raw read remained
  available. The test closes every session and removes its `EDS-TEST-*`
  container. This verifies the Android native protection path only; desktop
  VeraCrypt round-trip compatibility remains unverified.
- Re-ran `testLiteDebugUnitTest`, `lintLiteDebug`, and `assembleLiteDebug` after
  the foreground-service and protected-request fixes. All succeeded; the
  current arm64-only Debug APK SHA-256 is
  `183FCA7C93F2DF22C04AB08915A9B18ADD24893A27693B37928DE217DDD90673` and
  packages only `lib/arm64-v8a/libvc_core.so`.
- Connected native creation progress and cancellation all the way through the
  `VeraCryptRepository` to the normal and hidden creation Fragments. Both
  pages now show initialization percentage, filesystem-formatting and header
  finalization stages, expose a Cancel action, and request cancellation when
  their view is destroyed. The request is checked by native full-format loops
  at each 128 KiB initialization block; no passwords, keyfiles, paths, or
  volume names enter the callback.
- Rebuilt and installed the updated arm64 Debug and instrumentation APKs on
  OnePlus PLF110 (Android 16 / API 36). `testLiteDebugUnitTest`,
  `lintLiteDebug`, `assembleLiteDebug`, and `assembleLiteDebugAndroidTest`
  all passed. The device regression
  `normalVolumeCreationCanBeCancelledAtNativeProgressCallback` passed in
  0.018 seconds and observed the native `CANCELLED` result. The current Debug
  APK SHA-256 is
  `0E879CB632871B861083CDF65E92976C7DE3376FC8DA8CC202977141972073BF` and
  contains only `lib/arm64-v8a/libvc_core.so`.
- Fixed `UnlockedDocumentsProvider` on Android 16: `StorageManager`
  `openProxyFileDescriptor` now receives a non-null main-thread `Handler`.
  Android 16 rejects the previous null handler with a
  `NullPointerException`, which made every Provider file open fail before its
  proxy callback. Added an Activity-backed exFAT `DocumentsContract`
  mutation regression covering directory/file creation, proxy-FD write/read,
  rename, truncation, and deletion. The complete mutation regression remains
  pending: its first execution exposed the handler defect, and a subsequent
  run was interrupted during native initialization by the OnePlus test-host
  lifecycle. It is deliberately not claimed as verified.
- After the Provider-handler fix, rebuilt `testLiteDebugUnitTest`,
  `lintLiteDebug`, `assembleLiteDebug`, and `assembleLiteDebugAndroidTest`;
  all succeeded. The current arm64-only Debug APK SHA-256 is
  `68CB249E215B0106DEDB3147642A9FB96FDBAB5DF75FE5FEFAAADF61BCE0CCD6`.
- Strengthened proxy-FD release semantics: `UnlockedDocumentsProvider` now
  flushes its native FAT/exFAT file handle before closing and unregistering a
  proxy callback. This commits file and directory metadata for clients that
  close without an explicit `fsync`, which is the normal behavior of several
  DocumentsUI paths. `testLiteDebugUnitTest`, `lintLiteDebug`,
  `assembleLiteDebug`, and `assembleLiteDebugAndroidTest` passed after this
  change. The current arm64-only Debug APK SHA-256 is
  `B46E8D332591A19EC78AB5BF144DE1B5CDA096E592AA4747E674C6206B2511A2`.
- Moved `StorageManager` proxy-FD callbacks from the Provider/Activity main
  thread to a process-local `VeraCryptProxyFd` `HandlerThread`. This prevents
  callback dispatch from competing with lifecycle and binder work. The
  Android 16 DocumentsProvider mutation regression remains unverified: the
  test reliably reaches native volume creation but did not complete within
  the bounded foreground test window after the dispatcher change, so the
  process was explicitly stopped. `testLiteDebugUnitTest`, `lintLiteDebug`,
  `assembleLiteDebug`, and `assembleLiteDebugAndroidTest` completed before
  installation. The current arm64-only Debug APK SHA-256 is
  `A3BF6030568DED9AF3F267C9C7F1A6239A8F59F850C7DC2C3BE2106AEF2989A9`.
- Re-ran the Activity-backed `documentsProviderSupportsWritableExfatMutations`
  regression after synchronizing proxy-FD writes before querying directory
  metadata. It passed on OnePlus PLF110 in 15.372 seconds. The test creates a
  directory and file through `DocumentsContract`, writes through a proxy FD,
  verifies a non-zero-offset random read and a sequential read, renames,
  truncates, rereads, deletes, closes the session, and removes its
  `EDS-TEST-*` container. This is device-level DocumentsProvider coverage;
  interactive DocumentsUI and desktop VeraCrypt round trips remain pending.
- Re-ran `testLiteDebugUnitTest`, `lintLiteDebug`, `assembleLiteDebug`, and
  `assembleLiteDebugAndroidTest`; all passed before the passing device
  regression above. The Debug APK remains arm64-only and has SHA-256
  `A3BF6030568DED9AF3F267C9C7F1A6239A8F59F850C7DC2C3BE2106AEF2989A9`.
- Added `tools/stage-veracrypt-vectors-on-device.ps1` and optional
  Activity-backed official-vector regressions. The script stages the pinned
  VeraCrypt 1.26.29 public test vectors only in app-private device storage;
  it never adds containers or credentials to the repository. On OnePlus
  PLF110, the official SHA-512 vector opened successfully as a normal header
  with password `test` in 38.839 seconds and as a hidden header with password
  `testhidden` in 39.721 seconds. The operation used read-only descriptors
  and a foreground service. The official Whirlpool normal header also opened
  in 65.174 seconds and the official SHA-256 normal header opened in 92.058
  seconds under the same conditions. Other KDF/hidden-header vectors and
  desktop round trips remain pending.
- Added and ran the negative official-vector PIM regression. The SHA-512
  normal header with the correct password but PIM 1 (instead of its default
  PIM 0) was rejected as `INVALID_CREDENTIALS_OR_FORMAT` in 2.53 seconds on
  OnePlus PLF110. This covers a real desktop-vector wrong-PIM path without
  retaining credentials beyond the test request.
- Fixed a Kotlin compile regression in `NativeVolumeOpener` and
  `NativeVeraCryptRepository`: `ManagedVolumeSession` calls now name the
  trailing `onClose` argument explicitly, so its preceding optional
  `initialFileSystem` parameter cannot absorb the lifecycle closure. Re-ran
  `testLiteDebugUnitTest` and `assembleLiteDebug`; both passed.
- Centralized foreground-service ownership in `NativeVeraCryptRepository`.
  Opening a volume, normal or hidden creation, hidden-capacity analysis,
  credential changes, and header backup or restore now hold a nested,
  cancellation-safe foreground-operation lease for their full repository
  call. Active unlocked sessions continue to keep DocumentsProvider I/O in
  the same service, so proxy-file work does not create notification churn.
  Creation Fragments no longer duplicate lifecycle accounting; the private
  notification text now says only that an encrypted volume is being processed.
  `:app:testLiteDebugUnitTest --rerun-tasks` passed, including the new
  failure-and-cancellation balance regression; `lintLiteDebug` and
  `assembleLiteDebug` also passed.
- Rebuilt and installed the foreground-operation update on OnePlus PLF110
  (Android 16 / API 36, `arm64-v8a`) at ADB serial
  `192.168.1.101:44315`. `assembleLiteDebugAndroidTest` passed, and the
  installed instrumentation regressions passed: writable exFAT
  `DocumentsProvider` mutations in 13.769 seconds and native creation
  cancellation in 0.025 seconds. The Debug APK SHA-256 is
  `AC66F19B6A344B7B8D7F29E9074AC15A73094DDF8D0F69DA58D1F053F5D50577`;
  its only packaged native library is `lib/arm64-v8a/libvc_core.so`.
- Added independently runnable official-vector regressions for BLAKE2s and
  Streebog normal/hidden headers, plus the previously aggregate-only
  Whirlpool and SHA-256 hidden headers. On OnePlus PLF110, the BLAKE2s
  normal and hidden headers both opened successfully in 96.795 and 96.801
  seconds, respectively. The serial Streebog run produced no result within
  the four-minute command deadline; a separate normal-header run also
  produced no result within five minutes, so it remains pending. During that run,
  `dumpsys activity services` confirmed `VolumeForegroundService` as
  `isForeground=true`, `dataSync`, ongoing, and `SECRET`; after the
  test-only `am force-stop`, no service record remained.
- Reinstalled the expanded instrumentation APK and verified the remaining
  individually exposed official-vector hidden headers on the same device:
  Whirlpool passed in 65.212 seconds and SHA-256 passed in 71.394 seconds.
  This completes Android vector-opening evidence for normal and hidden
  headers under SHA-512, BLAKE2s, Whirlpool, and SHA-256; Streebog and all
  desktop round trips remain pending.
- Confirmed the newly installed Debug APK can launch its public
  `ContainerCatalogActivity` on OnePlus PLF110. No encrypted volume was
  unlocked for this check and no foreground-service record remained. No
  encrypted NTFS fixture is currently available in the worktree or device
  test corpus, so NTFS remains explicitly unverified rather than inferred
  from the bundled `libfsntfs` sources.
- Stage 3 arm64 acceleration: regenerated the Botan 3.12.0 arm64 amalgamation
  from the locked archive with normalized generation metadata; CMake now fails
  when CPUID, AES-ARMv8, SHA-256-ARMv8, SHA-512-ARMv8 markers or required
  ARMv8 intrinsics are absent. Release and Benchmark retain `-O3`, ThinLTO and
  section garbage collection.
- Added Benchmark-only `stage3-aes-hardware` and `stage3-sha512-kdf` cases.
  On OnePlus PLF110/API 36 arm64, Botan reported
  `aes=armv8aes;sha512=armv8sha2_512`; Fast AES 64 MiB median was 0.667 s
  (200.40 MB/s) and PBKDF2-HMAC-SHA-512 open median was 32.8 ms. These are
  measured results, not an unverified improvement claim over the non-matched
  Stage 2 gate.
- Rebuilt `assembleLiteDebug`, `assembleLiteDebugAndroidTest` and
  `assembleLiteBenchmark` successfully. Re-ran native invariant and the
  AES/Serpent/Twofish large XTS round-trip instrumentation; both tests passed
  on the connected device.
- Extended `tools/run-desktop-veracrypt-interop.ps1` with a `-Cipher` parameter
  and passed the same cipher through Android instrumentation. Serpent and
  Twofish each completed desktop-created and Android-created bidirectional
  marker round trips on D:, with byte-for-byte checks and successful mount /
  dismount. Explicit temporary containers were deleted afterward. Matched
  Stage-2 versus Stage-3 performance deltas remain pending; no cascade result
  is inferred from the single-cipher evidence.
- Reran the matched Stage-2 AES comparison: current serial-batched median
  0.343 s versus the recorded 0.359 s (about 4.5% faster), with 256/256
  read/write syscalls; this is below the 10% target and is reported as such.
  PBKDF2-HMAC-SHA-512 open measured 32.8 ms versus the recorded 50 ms,
  approximately 34% lower with unchanged parameters.
- Completed the shortened Full AES XTS confirmation: the same `xts-aes`
  256 MiB protocol measured 2.430 s median (220.76 MB/s) versus the recorded
  Stage-1 4.869 s (110.15 MB/s), approximately 50% lower elapsed time. The
  separate 10,000-random-operation hardware case measured 2.485 s; both runs
  passed correctness, remained thermal status 0, and stayed below the recorded
  PSS peak.
- Stage 4 prototype: added a per-session 1 MiB secure 16 KiB-page LRU for
  sub-64 KiB reads, write-through overlap invalidation, cache hit/miss counters,
  and wipe-on-eviction/close behavior. Added bounded in-request XTS lanes for
  128 KiB+ aligned blocks while retaining the session mutex and filesystem
  boundary. Targeted device correctness tests passed, including cache reload
  after a write and AES/Serpent/Twofish large-block round trips.
- Added the native `VC_CORE_DISABLE_PARALLEL_XTS` build switch for debug
  A/B correctness runs; it changes only the transform scheduler, not the
  volume format, keys, tweaks or filesystem I/O.
- The shortened cache benchmark recorded 10,001 4 KiB reads with one native
  read, 10,000 hits and one miss. However, the first Full AES run with the
  executor measured 4.759 s versus the Stage-3 serial 2.430 s baseline. The
  dispatch was then made adaptive: ARMv8 AES remains serial while software
  ciphers use bounded lanes. The matched Full AES rerun measured 2.462 s
  (1.3% from baseline, within the 5% gate), and Serpent Fast round-trip passed.
- The matched uncached comparator measured 24.07 ms and 10,001 underlying
  reads versus 6.55 ms and one underlying read with the cache (3.67x, 72.8%
  faster); cache counters recorded 10,000 hits and one miss. A fresh-session
  PSS delta was 535 KiB after mount, below the 8 MiB limit.
- Extended the native invariant self-test to compare serial and bounded-
  parallel ciphertext for every supported cipher suite byte-for-byte; the
  targeted device self-test and large-block instrumentation both passed.
- Stage 5 open-path optimization: catalog-backed normal and hidden volumes now
  persist only non-sensitive cipher/KDF enum hints, including successful AUTO
  resolution and concrete creation selections. The next UI open applies those
  hints; explicit hint failures are surfaced without an automatic full scan.
  Added an instrumentation test proving separate normal/hidden hints survive
  reload and no credential fields are stored.
- Proxy FD callbacks now cache the initial file length and update it only after
  a successful write, with offset/length overflow rejection. The targeted
  writable DocumentsProvider and exFAT regressions passed. Directory cursors
  remain unchanged pending evidence from a large-directory benchmark.
