package org.eds.veracrypt.benchmark

import android.app.Activity
import android.content.Context
import android.os.Bundle
import android.os.Debug
import android.os.ParcelFileDescriptor
import android.os.PowerManager
import android.os.SystemClock
import java.io.File
import java.util.concurrent.Executors
import kotlin.math.max
import kotlin.math.sqrt
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.nativecore.NativeCreateProgress
import org.eds.veracrypt.nativecore.NativeRequestCodec
import org.eds.veracrypt.nativecore.VcCore

/**
 * Headless, benchmark-build-only protocol runner. It writes an atomic status
 * document after every case so an interrupted ADB transport never loses prior
 * measurements. Production execution paths are not changed by this class.
 */
class PerformanceBenchmarkActivity : Activity() {
    private val executor = Executors.newSingleThreadExecutor()
    private var wakeLock: PowerManager.WakeLock? = null

    override fun onCreate(state: Bundle?) {
        super.onCreate(state)
        val power = getSystemService(PowerManager::class.java)
        wakeLock = power.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "EDS:benchmark").apply {
            setReferenceCounted(false)
            acquire(20 * 60 * 1000L)
        }
        executor.execute { runProtocol() }
    }

    override fun onDestroy() {
        executor.shutdownNow()
        wakeLock?.takeIf { it.isHeld }?.release()
        super.onDestroy()
    }

    private fun runProtocol() {
        val mode = intent.getStringExtra("mode")?.uppercase() ?: "FAST"
        val requested = intent.getStringExtra("case")?.trim()?.lowercase()
        val cases = when {
            requested != null && requested.isNotEmpty() -> listOf(requested)
            mode == "FULL" -> FULL_CASES
            else -> FAST_CASES
        }
        val started = SystemClock.elapsedRealtimeNanos()
        val status = JSONObjectStatus(mode, deviceMetadata())
        writeStatus(status.state("RUNNING", started, null))
        try {
            cases.forEachIndexed { index, name ->
                val result = try {
                    status.runningCase = name
                    status.caseStep = 0
                    status.caseSteps = if (mode == "FULL") 7 else 5
                    writeStatus(status.state("RUNNING", started, index))
                    runCase(name, mode) { step, total ->
                        status.caseStep = step
                        status.caseSteps = total
                        writeStatus(status.state("RUNNING", started, index))
                    }
                } catch (error: Throwable) {
                    mapOf("name" to name, "status" to "FAILED", "error" to (error.message ?: error.javaClass.simpleName))
                }
                status.results.add(result)
                writeStatus(status.state("RUNNING", started, index + 1))
                if (result["status"] == "FAILED") throw IllegalStateException("Benchmark case failed: $name")
            }
            writeStatus(status.state("SUCCESS", started, cases.size))
        } catch (error: Throwable) {
            status.failure = error.message ?: error.javaClass.simpleName
            writeStatus(status.state("FAILED", started, status.results.size))
        } finally {
            wakeLock?.takeIf { it.isHeld }?.release()
            runOnUiThread { finish() }
        }
    }

    private fun runCase(name: String, mode: String, heartbeat: (Int, Int) -> Unit): Map<String, Any?> {
        val measurementCount = if (mode == "FULL") 5 else 3
        val warmups = 2
        var step = 0
        val first = runRawCase(name, mode, heartbeat)
        heartbeat(++step, warmups + measurementCount)
        if (first["status"] == "SKIPPED") return first
        repeat(warmups - 1) {
            runRawCase(name, mode, heartbeat)
            heartbeat(++step, warmups + measurementCount)
        }
        val samples = MutableList(measurementCount) {
            val result = runRawCase(name, mode, heartbeat)
            heartbeat(++step, warmups + measurementCount)
            result
        }
        return aggregateMeasurements(first, samples, warmups)
    }

    private fun runRawCase(name: String, mode: String, heartbeat: ((Int, Int) -> Unit)? = null): Map<String, Any?> = when (name) {
        "self-test" -> timedCase(name) { VcCore.nativeRunSelfTests() }
        "xts-aes" -> runVolumeCase(name, CipherHint.AES, if (mode == "FULL") 256 else 64, 1_000)
        "xts-serpent" -> runVolumeCase(name, CipherHint.SERPENT, if (mode == "FULL") 256 else 16, 1_000)
        "xts-twofish" -> runVolumeCase(name, CipherHint.TWOFISH, if (mode == "FULL") 256 else 16, 1_000)
        "io-aes-256m" -> runVolumeCase(name, CipherHint.AES, 256, if (mode == "FULL") 10_000 else 1_000)
        "format-fat-128m" -> runVolumeCase(name, CipherHint.AES, 128, 0, VolumeFileSystem.FAT)
        "format-exfat-128m" -> runVolumeCase(name, CipherHint.AES, 128, 0, VolumeFileSystem.EXFAT)
        "stage2-compare-aes" -> runStage2StrategyComparison(name, mode, heartbeat)
        "kdf-sha512-open" -> runKdfCase(name)
        // Stage 3 explicit cases keep the provider-enabled AES/KDF measurements
        // separate from the historical Stage 1/2 case names.
        "stage3-aes-hardware" -> runVolumeCase(
            name,
            CipherHint.AES,
            if (mode == "FULL") 256 else 64,
            if (mode == "FULL") 10_000 else 1_000,
        )
        "stage3-sha512-kdf" -> runKdfCase(name)
        "stage4-cache-random" -> runStage4CacheCase(name)
        "auto-wrong-password" -> runAutoFailureCase(name)
        "botan-provider" -> mapOf(
            "name" to name,
            "status" to "PASS",
            "providers" to VcCore.nativeGetBotanHardwareProviders(),
        )
        "saf-provider-copy-256m", "ntfs-sequential-read" -> mapOf(
            "name" to name,
            "status" to "SKIPPED",
            "reason" to "No external SAF URI or reusable NTFS fixture was configured for this baseline run",
        )
        else -> throw IllegalArgumentException("Unknown benchmark case: $name")
    }

    private fun aggregateMeasurements(
        first: Map<String, Any?>,
        samples: List<Map<String, Any?>>,
        warmups: Int,
    ): Map<String, Any?> {
        fun values(key: String) = samples.mapNotNull { (it[key] as? Number)?.toDouble() }.sorted()
        fun median(values: List<Double>): Double = when {
            values.isEmpty() -> 0.0
            values.size % 2 == 1 -> values[values.size / 2]
            else -> (values[values.size / 2 - 1] + values[values.size / 2]) / 2.0
        }
        fun p95(values: List<Double>): Double = if (values.isEmpty()) 0.0 else values[(kotlin.math.ceil(values.size * 0.95).toInt() - 1)]
        fun relativeStdDev(values: List<Double>): Double {
            if (values.size < 2) return 0.0
            val average = values.average()
            if (average == 0.0) return 0.0
            return kotlin.math.sqrt(values.sumOf { (it - average) * (it - average) } / values.size) / average
        }

        val elapsed = values("elapsedNs")
        val throughput = values("throughputBytesPerSecond")
        val cpu = values("cpuNs")
        val pss = values("pssKiB")
        val thermal = values("thermalStatus")
        return first + mapOf(
            "elapsedNs" to median(elapsed).toLong(),
            "throughputBytesPerSecond" to median(throughput),
            "cpuNs" to median(cpu).toLong(),
            "pssKiB" to (pss.maxOrNull()?.toLong() ?: 0L),
            "thermalStatus" to (thermal.maxOrNull()?.toInt() ?: 0),
            "warmupIterations" to warmups,
            "iterations" to samples.size,
            "medianElapsedNs" to median(elapsed).toLong(),
            "p95ElapsedNs" to p95(elapsed).toLong(),
            "relativeStdDev" to relativeStdDev(elapsed),
            "samples" to samples,
        )
    }

    private fun timedCase(name: String, block: () -> Unit): Map<String, Any?> {
        val before = sample()
        block()
        val after = sample()
        return measurement(name, before, after, mapOf("iterations" to 1))
    }

    private fun runVolumeCase(
        name: String,
        cipher: CipherHint,
        sizeMiB: Int,
        randomOperations: Int,
        fileSystem: VolumeFileSystem = VolumeFileSystem.EXFAT,
    ): Map<String, Any?> {
        val file = File(cacheDir, "EDS-TEST-protocol-$name.hc")
        val bytes = sizeMiB.toLong() * 1024 * 1024
        val logical = bytes - 256 * 1024
        val passwordText = "benchmark-protocol"
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        val before = sample()
        try {
            descriptor = ParcelFileDescriptor.open(
                file,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            VolumeCredentials(SecretPassword(passwordText.toCharArray()), pim = 1, kdfHint = KdfHint.PBKDF2_HMAC_SHA512).use { credentials ->
                val options = VolumeCreateOptions(bytes, VolumeKind.NORMAL, cipher, KdfHint.PBKDF2_HMAC_SHA512, 1, fileSystem)
                NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                    session = request.useForJni { encoded ->
                        VcCore.nativeCreateNormal(descriptor!!.fd, encoded, intArrayOf(), NativeCreateProgress.inert())
                    }
                }
            }
            val expectedFileSystem = when (fileSystem) {
                VolumeFileSystem.FAT -> 1
                VolumeFileSystem.EXFAT -> 2
                VolumeFileSystem.NTFS -> error("Benchmark creation does not support NTFS")
            }
            check(VcCore.nativeMountFileSystem(session) == expectedFileSystem) {
                "filesystem mount returned an unexpected type"
            }
            val chunk = ByteArray(256 * 1024) { 0x5A }
            var offset = 0L
            while (offset < logical) {
                val count = minOf(chunk.size.toLong(), logical - offset).toInt()
                check(VcCore.nativeWrite(session, offset, chunk, 0, count) == count)
                offset += count
            }
            VcCore.nativeFlush(session)
            val read = ByteArray(chunk.size)
            offset = 0
            while (offset < logical) {
                val count = minOf(read.size.toLong(), logical - offset).toInt()
                check(VcCore.nativeRead(session, offset, read, 0, count) == count)
                check(read.copyOf(count).all { it == 0x5A.toByte() }) { "sequential readback mismatch" }
                offset += count
            }
            val random = java.util.Random(0x56435251L)
            repeat(randomOperations) {
                val randomOffset = random.nextInt((logical / 4096).toInt()).toLong() * 4096
                check(VcCore.nativeRead(session, randomOffset, read, 0, 4096) == 4096)
            }
            val after = sample()
            return measurement(name, before, after, mapOf(
                "cipher" to cipher.name,
                "fileSystem" to fileSystem.name,
                "dataBytes" to logical,
                "randomOperations" to randomOperations,
                "throughputBytes" to logical * 2,
                "iops" to randomOperations,
            ))
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            file.delete()
        }
    }

    /** Same APK comparison of the benchmark-only legacy sector path and production batching. */
    private fun runStage2StrategyComparison(name: String, mode: String, heartbeat: ((Int, Int) -> Unit)?): Map<String, Any?> {
        val logicalBytes = if (mode == "FULL") 256 * 1024 * 1024 else 64 * 1024 * 1024
        val results = linkedMapOf<String, Map<String, Any?>>()
        listOf("LEGACY_SECTOR", "SERIAL_BATCHED").forEachIndexed { strategyIndex, strategy ->
            val file = File(cacheDir, "EDS-TEST-stage2-$strategy.hc")
            var descriptor: ParcelFileDescriptor? = null
            var session = 0L
            try {
                descriptor = ParcelFileDescriptor.open(
                    file,
                    ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
                )
                VolumeCredentials(SecretPassword("stage2-comparison".toCharArray()), pim = 1,
                    kdfHint = KdfHint.PBKDF2_HMAC_SHA512).use { credentials ->
                    val options = VolumeCreateOptions(
                        maxOf(16L * 1024 * 1024, logicalBytes.toLong() + 2L * 1024 * 1024),
                        VolumeKind.NORMAL, CipherHint.AES,
                        KdfHint.PBKDF2_HMAC_SHA512, 1, VolumeFileSystem.EXFAT)
                    NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                        session = request.useForJni { encoded ->
                            VcCore.nativeCreateNormal(descriptor!!.fd, encoded, intArrayOf(), NativeCreateProgress.inert())
                        }
                    }
                }
                val payload = ByteArray(logicalBytes) { (it * 13).toByte() }
                val before = sample()
                val beforeCounters = VcCore.nativeGetPerformanceCounters(session)
                val chunkBytes = 4 * 1024 * 1024
                var offset = 0
                while (offset < payload.size) {
                    val count = minOf(chunkBytes, payload.size - offset)
                    val written = if (strategy == "LEGACY_SECTOR") {
                        VcCore.nativeWriteLegacy(session, offset.toLong(), payload, offset, count)
                    } else {
                        VcCore.nativeWrite(session, offset.toLong(), payload, offset, count)
                    }
                    check(written == count)
                    heartbeat?.invoke(strategyIndex + 1, 2)
                    offset += count
                }
                VcCore.nativeFlush(session)
                val actual = ByteArray(payload.size)
                offset = 0
                while (offset < actual.size) {
                    val count = minOf(chunkBytes, actual.size - offset)
                    val read = if (strategy == "LEGACY_SECTOR") {
                        VcCore.nativeReadLegacy(session, offset.toLong(), actual, offset, count)
                    } else {
                        VcCore.nativeRead(session, offset.toLong(), actual, offset, count)
                    }
                    check(read == count)
                    heartbeat?.invoke(strategyIndex + 1, 2)
                    offset += count
                }
                check(actual.contentEquals(payload))
                val counters = VcCore.nativeGetPerformanceCounters(session)
                results[strategy] = measurement(strategy, before, sample(), mapOf(
                    "dataBytes" to logicalBytes,
                    "readSyscalls" to counters[0] - beforeCounters[0],
                    "writeSyscalls" to counters[1] - beforeCounters[1],
                    "throughputBytes" to logicalBytes * 2,
                ))
                heartbeat?.invoke(strategyIndex + 1, 2)
            } finally {
                if (session != 0L) VcCore.nativeClose(session)
                descriptor?.close()
                file.delete()
            }
        }
        val legacy = results.getValue("LEGACY_SECTOR")
        val serial = results.getValue("SERIAL_BATCHED")
        val legacySeconds = (legacy["elapsedNs"] as Number).toDouble() / 1e9
        val serialSeconds = (serial["elapsedNs"] as Number).toDouble() / 1e9
        return serial + mapOf(
            "name" to name,
            "legacySeconds" to legacySeconds,
            "serialSeconds" to serialSeconds,
            "speedup" to if (serialSeconds > 0) legacySeconds / serialSeconds else 0.0,
            "legacyReadSyscalls" to legacy["readSyscalls"],
            "legacyWriteSyscalls" to legacy["writeSyscalls"],
            "serialReadSyscalls" to serial["readSyscalls"],
            "serialWriteSyscalls" to serial["writeSyscalls"],
        )
    }

    private fun runKdfCase(name: String): Map<String, Any?> {
        val file = File(cacheDir, "EDS-TEST-protocol-kdf.hc")
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        try {
            descriptor = ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE)
            session = createFixture(descriptor!!)
            VcCore.nativeClose(session)
            session = 0
            val before = sample()
            VolumeCredentials(SecretPassword(PASSWORD.toCharArray()), pim = 1, kdfHint = KdfHint.PBKDF2_HMAC_SHA512).use { credentials ->
                NativeRequestCodec.encodeOpen(VolumeOpenOptions(accessMode = VolumeAccessMode.READ_ONLY), credentials).use { request ->
                    session = request.useForJni { encoded -> VcCore.nativeOpen(descriptor!!.fd, false, encoded, intArrayOf()) }
                }
            }
            check(VcCore.nativeMountFileSystem(session) == 2)
            return measurement(name, before, sample(), mapOf("kdf" to "PBKDF2_HMAC_SHA512", "cipher" to "AES"))
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            file.delete()
        }
    }

    /** Repeated 4 KiB reads exercise the session-local 16 KiB page cache. */
    private fun runStage4CacheCase(name: String): Map<String, Any?> {
        val file = File(cacheDir, "EDS-TEST-stage4-cache.hc")
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        try {
            descriptor = ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE)
            session = createFixture(descriptor!!)
            VcCore.nativeClose(session)
            session = 0
            val beforeSession = sample()
            VolumeCredentials(SecretPassword(PASSWORD.toCharArray()), pim = 1, kdfHint = KdfHint.PBKDF2_HMAC_SHA512).use { credentials ->
                NativeRequestCodec.encodeOpen(VolumeOpenOptions(accessMode = VolumeAccessMode.READ_ONLY), credentials).use { request ->
                    session = request.useForJni { encoded -> VcCore.nativeOpen(descriptor!!.fd, false, encoded, intArrayOf()) }
                }
            }
            check(VcCore.nativeMountFileSystem(session) == 2)
            val afterSessionCreate = sample()
            val offset = 4L * 1024 * 1024
            val first = ByteArray(4096)
            val uncachedBefore = VcCore.nativeGetPerformanceCounters(session)
            val uncachedStart = SystemClock.elapsedRealtimeNanos()
            repeat(10_001) {
                check(VcCore.nativeReadUncached(session, offset, first, 0, first.size) == first.size)
            }
            val uncachedElapsed = SystemClock.elapsedRealtimeNanos() - uncachedStart
            val uncachedAfter = VcCore.nativeGetPerformanceCounters(session)
            val before = sample()
            val beforeCounters = VcCore.nativeGetPerformanceCounters(session)
            val cachedStart = SystemClock.elapsedRealtimeNanos()
            check(VcCore.nativeRead(session, offset, first, 0, first.size) == first.size)
            repeat(10_000) {
                check(VcCore.nativeRead(session, offset, first, 0, first.size) == first.size)
            }
            val cachedElapsed = SystemClock.elapsedRealtimeNanos() - cachedStart
            val counters = VcCore.nativeGetPerformanceCounters(session)
            val result = measurement(name, before, sample(), mapOf(
                "readOperations" to 10_001,
                "readSyscalls" to counters[0] - beforeCounters[0],
                "cacheHits" to counters[8] - beforeCounters[8],
                "cacheMisses" to counters[9] - beforeCounters[9],
                "throughputBytes" to 10_001L * first.size,
            ))
            return result + mapOf(
                "cachedElapsedNs" to cachedElapsed,
                "uncachedElapsedNs" to uncachedElapsed,
                "uncachedReadSyscalls" to uncachedAfter[0] - uncachedBefore[0],
                "cacheSpeedup" to if (cachedElapsed > 0) uncachedElapsed.toDouble() / cachedElapsed else 0.0,
                "sessionPssDeltaKiB" to max(0, afterSessionCreate.pssKiB - beforeSession.pssKiB),
            )
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            file.delete()
        }
    }

    private fun runAutoFailureCase(name: String): Map<String, Any?> {
        val file = File(cacheDir, "EDS-TEST-protocol-auto.hc")
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        try {
            descriptor = ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE)
            session = createFixture(descriptor!!)
            VcCore.nativeClose(session)
            session = 0
            val before = sample()
            var rejected = false
            VolumeCredentials(SecretPassword("wrong-password".toCharArray()), pim = 1, kdfHint = KdfHint.AUTO).use { credentials ->
                NativeRequestCodec.encodeOpen(VolumeOpenOptions(cipherHint = CipherHint.AUTO), credentials).use { request ->
                    try { request.useForJni { encoded -> VcCore.nativeOpen(descriptor!!.fd, false, encoded, intArrayOf()) } }
                    catch (_: Throwable) { rejected = true }
                }
            }
            check(rejected) { "AUTO wrong password unexpectedly opened" }
            return measurement(name, before, sample(), mapOf("cipherHint" to "AUTO", "rejected" to true))
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            file.delete()
        }
    }

    private fun createFixture(descriptor: ParcelFileDescriptor): Long {
        VolumeCredentials(SecretPassword(PASSWORD.toCharArray()), pim = 1, kdfHint = KdfHint.PBKDF2_HMAC_SHA512).use { credentials ->
            val options = VolumeCreateOptions(16L * 1024 * 1024, VolumeKind.NORMAL, CipherHint.AES, KdfHint.PBKDF2_HMAC_SHA512, 1, VolumeFileSystem.EXFAT)
            NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                return request.useForJni { encoded -> VcCore.nativeCreateNormal(descriptor.fd, encoded, intArrayOf(), NativeCreateProgress.inert()) }
            }
        }
    }

    private fun measurement(name: String, before: Sample, after: Sample, extra: Map<String, Any?>): Map<String, Any?> {
        val elapsed = after.wallNs - before.wallNs
        val seconds = elapsed / 1_000_000_000.0
        val data = (extra["throughputBytes"] as? Number)?.toDouble() ?: 0.0
        return mapOf(
            "name" to name,
            "status" to "PASS",
            "elapsedNs" to elapsed,
            "throughputBytesPerSecond" to if (seconds > 0) data / seconds else 0.0,
            "cpuNs" to (after.cpuNs - before.cpuNs),
            "pssKiB" to max(before.pssKiB, after.pssKiB),
            "thermalStatus" to max(before.thermal, after.thermal),
        ) + extra
    }

    private fun sample() = Sample(
        SystemClock.elapsedRealtimeNanos(),
        Debug.threadCpuTimeNanos(),
        Debug.getPss(),
        getSystemService(PowerManager::class.java).currentThermalStatus,
    )

    private fun deviceMetadata() = mapOf(
        "model" to android.os.Build.MODEL,
        "manufacturer" to android.os.Build.MANUFACTURER,
        "androidApi" to android.os.Build.VERSION.SDK_INT,
        "abi" to android.os.Build.SUPPORTED_ABIS.firstOrNull(),
        "apkVersion" to packageManager.getPackageInfo(packageName, 0).versionName,
    )

    private data class Sample(val wallNs: Long, val cpuNs: Long, val pssKiB: Long, val thermal: Int)

    private class JSONObjectStatus(private val mode: String, private val metadata: Map<String, Any?>) {
        val results = mutableListOf<Map<String, Any?>>()
        var failure: String? = null
        var runningCase: String? = null
        var caseStep: Int = 0
        var caseSteps: Int = 0

        fun state(state: String, startedNs: Long, completed: Int?): String {
            val root = org.json.JSONObject()
            root.put("protocol", 1)
            root.put("state", state)
            root.put("mode", mode)
            root.put("startedWallNs", startedNs)
            root.put("updatedWallNs", SystemClock.elapsedRealtimeNanos())
            root.put("completed", completed ?: 0)
            runningCase?.let { root.put("runningCase", it) }
            root.put("caseStep", caseStep)
            root.put("caseSteps", caseSteps)
            root.put("metadata", org.json.JSONObject(metadata))
            root.put("results", org.json.JSONArray(results.map { org.json.JSONObject(it) }))
            failure?.let { root.put("failure", it) }
            return root.toString()
        }
    }

    private fun writeStatus(value: String) {
        val directory = File(cacheDir, "benchmark").apply { mkdirs() }
        val temporary = File(directory, "status.json.tmp")
        temporary.writeText(value)
        check(temporary.renameTo(File(directory, "status.json"))) { "status rename failed" }
    }

    private companion object {
        const val PASSWORD = "benchmark-protocol"
        val FAST_CASES = listOf("self-test", "xts-aes", "xts-serpent", "xts-twofish", "io-aes-256m", "format-fat-128m", "format-exfat-128m")
        val FULL_CASES = FAST_CASES + listOf("kdf-sha512-open", "auto-wrong-password", "saf-provider-copy-256m", "ntfs-sequential-read")
    }
}
