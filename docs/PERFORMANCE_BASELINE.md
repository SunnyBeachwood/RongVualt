# Native Performance Baseline

## 2026-07-29: arm64 four-lane XTS

Device: OnePlus PLF110, Android 16 / API 36, `arm64-v8a`. Build: Debug-signed
`liteBenchmark` with native `-O3` and ThinLTO. The benchmark uses an
app-private 256 MiB temporary AES-XTS / exFAT container, two warmups and five
measured runs. Each measured run formats the container, writes and reads
255.75 MiB of logical data in 256 KiB requests, then performs 4,096 random
4 KiB reads. No paths, volume names, credentials, keys, or plaintext are
stored in this document.

| Metric | Five measured runs (seconds) | Median |
| --- | --- | ---: |
| Full format | 5.777, 5.979, 5.970, 6.049, 4.276 | 5.970 s |
| Sequential write | 1.937, 2.827, 2.034, 2.786, 2.719 | 2.719 s / 94.06 MiB/s |
| Sequential read | 3.363, 4.100, 3.891, 2.459, 4.219 | 3.891 s / 65.73 MiB/s |
| 4 KiB random reads | 1.132, 1.137, 1.136, 1.156, 1.135 | 1.136 s / 3,607 ops/s |

All five runs finished every stage without a 60-second watchdog timeout.
Thermal status was `0` throughout. Peak process PSS was 84,971 KiB; this is
whole-process PSS, not the memory cost of one native session. Sequential I/O
made 1,023 encrypted-container operations per direction. The random test
made 4,076 encrypted-container reads, with 20 cache hits and 4,076 cache
misses.

This is an initial post-four-lane AES result, not a controlled before/after
claim: the earlier serial trial was interrupted by an abnormal format stall.
Serpent, Twofish, cascades, hidden-volume throughput, five-minute sustained
load, and ARMv8 Botan hardware-provider measurements remain separate work.

## 2026-07-29: arm64 four-lane XTS, Serpent

Device and method are the same as the AES baseline above: OnePlus PLF110,
Android 16 / API 36, Debug-signed `liteDebug`, an app-private 256 MiB
Serpent-XTS / exFAT container, two warmups and five measured trials. The
benchmark was selected explicitly with `vc.benchmark.cipher=SERPENT`; it is a
device result, not a cross-device cipher comparison.

| Metric | Five measured runs (seconds) | Median |
| --- | --- | ---: |
| Full format | 31.740, 31.137, 31.967, 31.970, 31.467 | 31.740 s |
| Sequential write | 34.189, 34.022, 33.658, 34.067, 34.188 | 34.067 s / 7.51 MiB/s |
| Sequential read | 18.650, 18.431, 18.515, 18.478, 18.419 | 18.478 s / 13.84 MiB/s |
| 4 KiB random reads | 17.757, 17.746, 17.789, 17.452, 17.460 | 17.746 s / 230.81 ops/s |

All stages of both warmups and all five measurements completed before the
60-second watchdog. Thermal status was `0` throughout. Per-run PSS ranged from
83,219 KiB to 87,370 KiB. Every sequential direction performed 1,023 encrypted
container operations; the random test performed 4,076 encrypted-container
reads with 20 cache hits and 4,076 cache misses. The displayed random median
is calculated from the five elapsed times, rather than the representative
trial selected by the benchmark's write-time median log record.

Twofish, supported cascades, and hidden-volume throughput remain pending.
Cascades currently have byte-level native
parallel-XTS regressions, but are not exposed through the first-release
creation UI and therefore do not yet have a comparable end-to-end device row.

## 2026-07-29: AES sustained read/write stability

One app-private 256 MiB AES-XTS / exFAT container was formatted, then retained
for a five-minute sustained window. Each cycle overwrote the complete 255.75
MiB logical range, flushed it, read it back in full, and verified every byte.
The implementation permits the final whole cycle to finish after the window,
so this valid run lasted 331.781 seconds rather than terminating in the middle
of a verification pass.

| Result | Value |
| --- | ---: |
| Complete write/read/verify cycles | 8 |
| Bytes written | 2,145,386,496 (2.00 GiB) |
| Bytes read and verified | 2,145,386,496 (2.00 GiB) |
| Peak process PSS | 84,650 KiB |
| Peak thermal status | 0 (normal) |
| Watchdog timeouts / data mismatches | 0 / 0 |

Progress observations were 41, 84, 126, 168, 210, 253, 294 and 331 seconds.
PSS rose from 83,498 KiB to 84,650 KiB without monotonic runaway; thermal
status remained 0 throughout. The container was closed and successfully
removed after the result record was written. This validates one AES normal
volume path only; hidden-volume protection, Serpent and Twofish soak runs are
separate pending work.

## 2026-07-29: Stage-1 Benchmark protocol baseline

The headless Benchmark APK protocol completed Fast and Full on OnePlus PLF110
(Android 16 / API 36, arm64-v8a). Each applicable item used two warmups plus
three Fast or five Full measurements. The report records per-trial data,
median, P95, relative standard deviation, CPU time, peak PSS and thermal state
under `app/build/reports/benchmark/`.

| Full case | Status | Median | P95 | RSD | Throughput | Peak PSS KiB | Thermal |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Native self-test | PASS | 0.063 s | 0.072 s | 5.96% | — | 74,204 | 0 |
| AES 256 MiB XTS | PASS | 4.869 s | 5.621 s | 6.00% | 110.15 MB/s | 81,952 | 0 |
| Serpent 256 MiB XTS | PASS | 13.040 s | 14.005 s | 5.44% | 41.13 MB/s | 75,751 | 0 |
| Twofish 256 MiB XTS | PASS | 9.255 s | 9.645 s | 2.39% | 57.95 MB/s | 79,494 | 0 |
| AES 256 MiB sequential/random I/O | PASS | 5.358 s | 5.555 s | 4.57% | 100.09 MB/s; 10,000 reads | 78,346 | 0 |
| FAT 128 MiB format/mount | PASS | 2.206 s | 3.146 s | 23.02% | 121.47 MB/s | 82,890 | 0 |
| exFAT 128 MiB format/mount | PASS | 2.594 s | 3.736 s | 39.26% | 103.28 MB/s | 82,036 | 0 |
| SHA-512 KDF open | PASS | 0.050 s | 0.056 s | 13.39% | — | 83,706 | 0 |
| AUTO wrong-password rejection | PASS | 0.074 s | 0.081 s | 4.15% | — | 66,800 | 0 |
| SAF/Provider copy | SKIPPED | — | — | — | no configured SAF URI | — | — |
| NTFS sequential read | SKIPPED | — | — | — | no reusable fixture | — | — |

The initial FAT failure was not native FAT failure: the Benchmark runner had
incorrectly required exFAT's mount-type value (`2`) for FAT, which correctly
returns `1`. After correcting only that benchmark assertion, Fast and Full
both passed. FAT/exFAT format variability exceeds the 10% target and is marked
as a hot-state follow-up; it is not a reason to alter crypto or filesystem
execution without a controlled rerun. One first Full attempt was invalidated
after 60 seconds without a status heartbeat; the final result used sub-run
heartbeats and completed without a watchdog timeout. SAF and NTFS remain
explicitly unmeasured rather than represented as pass results.

## 2026-07-29: Stage-2 serial I/O comparison

The production session now uses one mutex-protected serial path with a maximum
256 KiB encrypted batch, no plaintext cache and no session worker threads.
Botan XTS modes are created and keyed once per session; each 512-byte data unit
only updates its little-endian VeraCrypt tweak. The old path exists only in the
Benchmark APK for comparison.

The same Benchmark APK ran two warmups and three measured AES comparisons on
the connected OnePlus PLF110. The fast gate used 64 MiB of deterministic data
and verified every byte:

| Path | Median elapsed | Read syscalls | Write syscalls |
| --- | ---: | ---: | ---: |
| `LEGACY_SECTOR` | 0.962 s | 262,144 | 131,072 |
| `SERIAL_BATCHED` | 0.359 s | 256 | 256 |

The 64 MiB gate measured a 2.68x serial speedup, below the 3x acceptance target;
the earlier 4 MiB smoke comparison was 8.30x but is not used for the gate.
The aligned-write regression confirmed zero reads and one 256 KiB write; the boundary matrix passed for AES, Serpent
and Twofish. Hidden-volume creation/reopen/protection and SHA-512 normal-vector
opening also passed after the path change. The full official-vector aggregate
was started but its long final output was not retained by the ADB client; the
explicit SHA-512 normal and hidden methods are recorded as passing separately.

## 2026-07-29: Stage-3 arm64 provider observation

The Benchmark APK's provider probe on the connected OnePlus PLF110 reported:

`aes=armv8aes;sha512=armv8sha2_512`

This is runtime Botan dispatch selected by CPUID, not merely a compile-time
marker. The arm64 amalgamation contains CPUID detection, AES-ARMv8,
SHA-256-ARMv8 and SHA-512-ARMv8 markers and NEON intrinsic references; CMake
now stops configuration when any of those are missing. A Stage-3 AES/SHA-512
before/after throughput comparison is now recorded below; the provider gate
itself is directly verified on hardware.

The explicit Fast cases (two warmups plus three measurements) completed on the
same device after regeneration:

| Case | Status | Median | P95 | RSD | Throughput | Peak PSS KiB | Thermal |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| AES hardware XTS, 64 MiB | PASS | 0.667 s | 0.690 s | 17.14% | 200.40 MB/s | 264,077 | 0 |
| PBKDF2-HMAC-SHA-512 open | PASS | 0.0328 s | 0.0329 s | 5.46% | — | 72,553 | 0 |

The AES result is a current hardware-provider measurement, not a claimed
improvement over Stage 2: the Stage 2 64 MiB serial comparison measured
0.359 s for the combined read/write gate, so the workloads are not identical
and no speedup percentage is asserted until a matched before/after pair is
rerun. The KDF result confirms the unchanged PBKDF2-SHA-512 parameter path.

The regenerated files currently have SHA-256 hashes
`0A28922586A4E8BC5FC20F8A818B0B0AAE96FD294255D26E70958FF3005BA60D`
(`vc_botan.cpp`) and
`CAC6E367A20BBE00DFD2D69618ED65B184FE94B76E0394F2BF8BDD4EEBC332AE`
(`vc_botan.h`).

## 2026-07-29: Stage-4 cache and bounded parallel prototype

The native session now owns a 1 MiB (64 × 16 KiB) secure LRU. Only reads
shorter than 64 KiB use it; aligned large I/O bypasses it. Writes invalidate
overlapping pages before their first I/O, and destruction wipes all page
buffers. The matched cache benchmark passed with 10,001 repeated 4 KiB reads.
The uncached path took 24.07 ms and issued 10,001 reads; the cached path took
6.55 ms, issued one read, and recorded 10,000 hits/one miss: a 3.67x (72.8%)
improvement on this device. Reopening a fresh session showed a 535 KiB PSS
delta after mount, below the 8 MiB session-growth limit.

The 128 KiB+ transform path now uses a bounded two-lane executor (at most four
lanes by design) with independent session XTS contexts; the existing serial
mutex and filesystem access boundaries remain unchanged. Native invariant,
cache invalidation, exFAT operations and AES/Serpent/Twofish 128 KiB+ round
trips passed on the connected device.

The first all-cipher executor measurement regressed AES, so the dispatch was
made adaptive: ARMv8 AES remains on the serial session path while software
bound ciphers retain the bounded lanes. The subsequent matched Full AES run
measured 2.462 s median versus the Stage-3 2.430 s baseline (1.3% difference,
within the 5% sequential gate), with thermal status 0. Serpent Fast 16 MiB
round-trip also passed through the bounded executor. The cache and parallel
correctness gates are therefore green: the native self-test now compares the
parallel and serial ciphertext byte-for-byte for every supported suite.
Broader Serpent/Twofish throughput measurement remains a follow-up.

The matched Stage-2 comparison was rerun after regeneration using the same
Benchmark APK protocol: current serial-batched median 0.343 s versus the
recorded Stage-2 0.359 s for the 64 MiB read/write gate (about 4.5% faster),
while the legacy path measured 0.964 s. This is a real improvement but does
not meet the former 10% optimization target; no larger claim is made.
The SHA-512 PBKDF2 open median is 32.8 ms versus the recorded Stage-1/2
50 ms result (about 34% lower), with the same PIM/KDF parameters.

For the same `xts-aes` Full protocol (256 MiB, two warmups, five samples and
1,000 random reads), the regenerated arm64 build measured a 2.430 s median
(220.76 MB/s), versus the Stage-1 recorded 4.869 s (110.15 MB/s), an
approximately 50% lower elapsed time. This is the matched AES XTS evidence
used for the Stage-3 improvement claim; the separate 10,000-random-operation
case measured 2.485 s.

## 2026-07-29: Stage-3 desktop cipher interoperability

The desktop helper now accepts `-Cipher AES|Serpent|Twofish`, and the Android
instrumentation receives the same choice through `vc.cipher`. On D: with
VeraCrypt's desktop build, both Serpent and Twofish completed the full marker
round trip in both directions:

1. desktop-created container mounted on D:, Android opened it and wrote a
   marker, desktop read it and wrote a return marker, Android read the return
   marker;
2. Android-created container opened on desktop, desktop read the Android
   marker and wrote a return marker, Android read the return marker.

All marker comparisons were byte-for-byte and all mount/dismount operations
returned success. The four explicitly named temporary containers were removed
after verification. AES remains covered by the earlier equivalent interop
path; no cascade result is inferred from these single-cipher tests.
## 2026-07-29: Stage-5 open-path metadata and Proxy FD

The catalog stores separate normal/hidden cipher and KDF hints as enum names
only. No password, PIM, keyfile bytes, or master key is serialized. A
successful catalog-backed open records the resolved native header values; new
normal and hidden creation paths also seed their concrete selections. The UI
applies the stored hint on the next open, while an explicit non-AUTO choice is
passed through unchanged and failure is surfaced without a second AUTO scan.

Proxy file descriptors now retain the initial length and advance it only after
a successful native write, with overflow rejection. The existing Provider
mutation regression passed after this change. Directory cursor behavior was
not changed; no large-directory evidence currently justifies pagination.
