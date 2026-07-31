package org.eds.veracrypt.nativecore

import android.content.Intent
import android.content.Context
import android.os.Debug
import android.os.ParcelFileDescriptor
import android.os.PowerManager
import android.os.SystemClock
import android.util.Log
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.util.Random
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicLong
import java.util.concurrent.atomic.AtomicReference
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.ui.ContainerCatalogActivity
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Assume.assumeTrue
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Opt-in device benchmark. It intentionally creates only app-cache
 * EDS-TEST-* files and never runs as part of the ordinary instrumentation
 * suite. Invoke with `-e vc.benchmark true` against the benchmark APK.
 */
@RunWith(AndroidJUnit4::class)
class VcCoreBenchmarkInstrumentationTest {
    @Test
    fun cipher256MiBSequentialAndRandomIo() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        assumeTrue(
            "Opt in with instrumentation argument vc.benchmark=true",
            InstrumentationRegistry.getArguments().getString("vc.benchmark") == "true",
        )
        val context = instrumentation.targetContext
        val cipher = benchmarkCipher()
        val volumeBytes = benchmarkVolumeBytes()
        val logicalBytes = volumeBytes - 256L * 1024
        val warmups = benchmarkInt("vc.benchmark.warmups", WARMUP_RUNS, 0, 2)
        val measuredRuns = benchmarkInt("vc.benchmark.runs", MEASURED_RUNS, 1, 5)
        val randomReads = benchmarkInt("vc.benchmark.randomReads", RANDOM_READ_OPERATIONS, 1, RANDOM_READ_OPERATIONS)
        val resultFile = File(context.cacheDir, "EDS-TEST-benchmark-last.txt")
        resultFile.writeText("VC_BENCHMARK device run started cipher=$cipher size_mib=${volumeBytes / (1024 * 1024)} topology=${VcCore.nativeCpuTopologySummary()}\n")
        context.startActivity(Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK))
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            repeat(warmups) { index -> record(resultFile, runTrial(context, resultFile, "warmup-${index + 1}", cipher, volumeBytes, logicalBytes, randomReads)) }
            val measurements = List(measuredRuns) { index -> record(resultFile, runTrial(context, resultFile, "run-${index + 1}", cipher, volumeBytes, logicalBytes, randomReads)) }
            val median = measurements.sortedBy { it.sequentialWriteWallNs }[measurements.size / 2]
            Log.i(TAG, "VC_BENCHMARK_MEDIAN ${median.toLogLine()}")
            resultFile.appendText("VC_BENCHMARK_MEDIAN ${median.toLogLine()}\n")
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    /**
     * Opt-in five-minute durability and thermal soak.  It intentionally uses
     * one encrypted 256 MiB container rather than creating a sequence of
     * volumes, so it exercises the long-lived session/XTS worker lifecycle.
     */
    @Test
    fun fiveMinuteSustainedReadWrite() {
        assumeTrue(
            "Opt in with instrumentation argument vc.sustained=true",
            InstrumentationRegistry.getArguments().getString("vc.sustained") == "true",
        )
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        val cipher = benchmarkCipher()
        val resultFile = File(context.cacheDir, "EDS-TEST-benchmark-sustained-last.txt")
        resultFile.writeText("VC_SUSTAINED device run started cipher=$cipher duration_s=300 topology=${VcCore.nativeCpuTopologySummary()}\n")
        context.startActivity(Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK))
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            runSustainedTrial(context, resultFile, cipher)
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    private fun runTrial(
        context: Context,
        resultFile: File,
        label: String,
        cipher: CipherHint,
        volumeBytes: Long,
        logicalBytes: Long,
        randomReads: Int,
    ): Trial {
        val container = File(context.cacheDir, "EDS-TEST-benchmark-$label.hc")
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        val watchdog = StageWatchdog(resultFile, label)
        try {
            watchdog.begin("format")
            val formatStart = sample()
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            VolumeCredentials(SecretPassword(PASSWORD.toCharArray()), pim = 1, kdfHint = KdfHint.PBKDF2_HMAC_SHA512).use { credentials ->
                val options = VolumeCreateOptions(
                    sizeBytes = volumeBytes,
                    volumeKind = VolumeKind.NORMAL,
                    cipher = cipher,
                    kdf = KdfHint.PBKDF2_HMAC_SHA512,
                    pim = 1,
                    fileSystem = VolumeFileSystem.EXFAT,
                )
                NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                    val progress = NativeCreateProgress(onUpdate = { _, _, _ ->
                        watchdog.pulse()
                    })
                    session = request.useForJni { bytes ->
                        VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), progress)
                    }
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(session))
            val formatEnd = sample()
            watchdog.finish()
            val afterFormat = VcCore.nativeGetPerformanceCounters(session)

            val write = measure(watchdog, "sequential-write") {
                val chunk = ByteArray(IO_CHUNK_BYTES) { PATTERN }
                var offset = 0L
                while (offset < logicalBytes) {
                    assertEquals(chunk.size, VcCore.nativeWrite(session, offset, chunk, 0, chunk.size))
                    offset += chunk.size
                    watchdog.pulse()
                }
                VcCore.nativeFlush(session)
                watchdog.pulse()
            }
            val afterWrite = VcCore.nativeGetPerformanceCounters(session)
            val read = measure(watchdog, "sequential-read") {
                val chunk = ByteArray(IO_CHUNK_BYTES)
                var offset = 0L
                while (offset < logicalBytes) {
                    assertEquals(chunk.size, VcCore.nativeRead(session, offset, chunk, 0, chunk.size))
                    assertTrue(chunk.all { it == PATTERN })
                    offset += chunk.size
                    watchdog.pulse()
                }
            }
            val afterRead = VcCore.nativeGetPerformanceCounters(session)
            val randomRead = measure(watchdog, "random-4k-read") {
                val random = Random(0x5643)
                val buffer = ByteArray(RANDOM_IO_BYTES)
                repeat(randomReads) {
                    val offset = random.nextLong(logicalBytes / RANDOM_IO_BYTES) * RANDOM_IO_BYTES
                    assertEquals(buffer.size, VcCore.nativeRead(session, offset, buffer, 0, buffer.size))
                    assertTrue(buffer.all { it == PATTERN })
                    watchdog.pulse()
                }
            }
            val afterRandom = VcCore.nativeGetPerformanceCounters(session)
            return Trial(
                label = label, cipher = cipher,
                formatWallNs = formatEnd.wallNs - formatStart.wallNs,
                formatCpuNs = formatEnd.cpuNs - formatStart.cpuNs,
                sequentialWriteWallNs = write.wallNs,
                sequentialWriteCpuNs = write.cpuNs,
                sequentialReadWallNs = read.wallNs,
                sequentialReadCpuNs = read.cpuNs,
                randomReadWallNs = randomRead.wallNs,
                randomReadCpuNs = randomRead.cpuNs,
                pssKiB = maxOf(formatStart.pssKiB, formatEnd.pssKiB, write.pssKiB, read.pssKiB, randomRead.pssKiB),
                thermal = randomRead.thermal,
                writeCounters = delta(afterFormat, afterWrite),
                readCounters = delta(afterWrite, afterRead),
                randomCounters = delta(afterRead, afterRandom),
            ).also { Log.i(TAG, "VC_BENCHMARK ${it.toLogLine()}") }
        } catch (error: Throwable) {
            watchdog.fail(error)
            throw error
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            assertTrue("Benchmark container was not removed", container.delete() || !container.exists())
            watchdog.close()
        }
    }

    private fun runSustainedTrial(context: Context, resultFile: File, cipher: CipherHint) {
        val container = File(context.cacheDir, "EDS-TEST-sustained-${cipher.name.lowercase()}.hc")
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        val watchdog = StageWatchdog(resultFile, "sustained-$cipher")
        try {
            watchdog.begin("format")
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            VolumeCredentials(SecretPassword(PASSWORD.toCharArray()), pim = 1, kdfHint = KdfHint.PBKDF2_HMAC_SHA512).use { credentials ->
                val options = VolumeCreateOptions(VOLUME_BYTES, VolumeKind.NORMAL, cipher, KdfHint.PBKDF2_HMAC_SHA512, 1, VolumeFileSystem.EXFAT)
                NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                    session = request.useForJni { bytes ->
                        VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress { _, _, _ -> watchdog.pulse() })
                    }
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(session))
            watchdog.finish()

            val startNs = SystemClock.elapsedRealtimeNanos()
            val deadlineNs = startNs + SUSTAINED_DURATION_NS
            var cycles = 0
            var bytesWritten = 0L
            var bytesRead = 0L
            var peakPss = Debug.getPss()
            var peakThermal = contextPowerManager.currentThermalStatus
            var nextReportNs = startNs
            val chunk = ByteArray(IO_CHUNK_BYTES)
            while (SystemClock.elapsedRealtimeNanos() < deadlineNs) {
                val pattern = (cycles and 0x7f).toByte()
                chunk.fill(pattern)
                measure(watchdog, "sustained-write") {
                    var offset = 0L
                    while (offset < LOGICAL_BYTES) {
                        assertEquals(chunk.size, VcCore.nativeWrite(session, offset, chunk, 0, chunk.size))
                        offset += chunk.size
                        bytesWritten += chunk.size
                        watchdog.pulse()
                    }
                    VcCore.nativeFlush(session)
                }
                measure(watchdog, "sustained-read") {
                    var offset = 0L
                    while (offset < LOGICAL_BYTES) {
                        assertEquals(chunk.size, VcCore.nativeRead(session, offset, chunk, 0, chunk.size))
                        assertTrue("sustained data mismatch cycle=$cycles offset=$offset", chunk.all { it == pattern })
                        offset += chunk.size
                        bytesRead += chunk.size
                        watchdog.pulse()
                    }
                }
                cycles++
                peakPss = maxOf(peakPss, Debug.getPss())
                peakThermal = maxOf(peakThermal, contextPowerManager.currentThermalStatus)
                val nowNs = SystemClock.elapsedRealtimeNanos()
                if (nowNs >= nextReportNs) {
                    resultFile.appendText("VC_SUSTAINED_PROGRESS cipher=$cipher cycles=$cycles elapsed_s=${(nowNs - startNs) / 1_000_000_000L} pss_kib=$peakPss thermal=$peakThermal\n")
                    nextReportNs = nowNs + SUSTAINED_REPORT_INTERVAL_NS
                }
            }
            val elapsedNs = SystemClock.elapsedRealtimeNanos() - startNs
            resultFile.appendText("VC_SUSTAINED_RESULT cipher=$cipher duration_ns=$elapsedNs cycles=$cycles bytes_written=$bytesWritten bytes_read=$bytesRead peak_pss_kib=$peakPss peak_thermal=$peakThermal\n")
            Log.i(TAG, "VC_SUSTAINED_RESULT cipher=$cipher duration_ns=$elapsedNs cycles=$cycles bytes_written=$bytesWritten bytes_read=$bytesRead peak_pss_kib=$peakPss peak_thermal=$peakThermal")
        } catch (error: Throwable) {
            watchdog.fail(error)
            throw error
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            assertTrue("Sustained container was not removed", container.delete() || !container.exists())
            watchdog.close()
        }
    }

    private fun measure(watchdog: StageWatchdog, stage: String, block: () -> Unit): Measurement {
        watchdog.begin(stage)
        val before = sample()
        block()
        val after = sample()
        watchdog.finish()
        return Measurement(after.wallNs - before.wallNs, after.cpuNs - before.cpuNs, maxOf(before.pssKiB, after.pssKiB), after.thermal)
    }

    private fun record(resultFile: File, trial: Trial): Trial = trial.also {
        resultFile.appendText("VC_BENCHMARK ${it.toLogLine()}\n")
    }

    private fun sample(): Sample = Sample(
        wallNs = SystemClock.elapsedRealtimeNanos(),
        cpuNs = Debug.threadCpuTimeNanos(),
        pssKiB = Debug.getPss(),
        thermal = contextPowerManager.currentThermalStatus,
    )

    private val contextPowerManager: PowerManager
        get() = InstrumentationRegistry.getInstrumentation().targetContext.getSystemService(PowerManager::class.java)

    private fun delta(before: LongArray, after: LongArray): List<Long> = after.indices.map { after[it] - before[it] }

    private fun benchmarkCipher(): CipherHint = when (
        InstrumentationRegistry.getArguments().getString("vc.benchmark.cipher", "AES").uppercase()
    ) {
        "AES" -> CipherHint.AES
        "SERPENT" -> CipherHint.SERPENT
        "TWOFISH" -> CipherHint.TWOFISH
        else -> throw IllegalArgumentException("vc.benchmark.cipher must be AES, SERPENT, or TWOFISH")
    }

    private fun benchmarkVolumeBytes(): Long {
        val mib = InstrumentationRegistry.getArguments().getString("vc.benchmark.sizeMiB", "256")!!.toLongOrNull()
            ?: throw IllegalArgumentException("vc.benchmark.sizeMiB must be an integer")
        require(mib in 8L..256L) { "vc.benchmark.sizeMiB must be between 8 and 256" }
        return mib * 1024L * 1024L
    }

    private fun benchmarkInt(name: String, default: Int, min: Int, max: Int): Int {
        val value = InstrumentationRegistry.getArguments().getString(name, default.toString())!!.toIntOrNull()
            ?: throw IllegalArgumentException("$name must be an integer")
        require(value in min..max) { "$name must be between $min and $max" }
        return value
    }

    private data class Sample(val wallNs: Long, val cpuNs: Long, val pssKiB: Long, val thermal: Int)
    private data class Measurement(val wallNs: Long, val cpuNs: Long, val pssKiB: Long, val thermal: Int)
    private data class Trial(
        val label: String, val cipher: CipherHint, val formatWallNs: Long, val formatCpuNs: Long,
        val sequentialWriteWallNs: Long, val sequentialWriteCpuNs: Long,
        val sequentialReadWallNs: Long, val sequentialReadCpuNs: Long,
        val randomReadWallNs: Long, val randomReadCpuNs: Long,
        val pssKiB: Long, val thermal: Int,
        val writeCounters: List<Long>, val readCounters: List<Long>, val randomCounters: List<Long>,
    ) {
        fun toLogLine() = "label=$label cipher=$cipher format_wall_ns=$formatWallNs format_cpu_ns=$formatCpuNs " +
            "seq_write_wall_ns=$sequentialWriteWallNs seq_write_cpu_ns=$sequentialWriteCpuNs " +
            "seq_read_wall_ns=$sequentialReadWallNs seq_read_cpu_ns=$sequentialReadCpuNs " +
            "random4k_wall_ns=$randomReadWallNs random4k_cpu_ns=$randomReadCpuNs " +
            "pss_kib=$pssKiB thermal=$thermal write=$writeCounters read=$readCounters random=$randomCounters"
    }

    /** Records non-sensitive stage heartbeats and requests native cancellation after 60s of silence. */
    private class StageWatchdog(private val resultFile: File, private val label: String) : AutoCloseable {
        private val executor = Executors.newSingleThreadScheduledExecutor()
        private val lastProgressNs = AtomicLong(SystemClock.elapsedRealtimeNanos())
        private val activeStage = AtomicReference("")
        private val timedOut = AtomicBoolean(false)
        private val lock = Any()

        init {
            executor.scheduleAtFixedRate({
                val elapsedNs = SystemClock.elapsedRealtimeNanos() - lastProgressNs.get()
                val stage = activeStage.get()
                if (stage.isNotEmpty() && elapsedNs >= STAGE_TIMEOUT_NS && timedOut.compareAndSet(false, true)) {
                    append("VC_BENCHMARK_STAGE label=$label stage=$stage outcome=timeout idle_ns=$elapsedNs")
                }
            }, 1, 1, TimeUnit.SECONDS)
        }

        fun begin(stage: String) {
            synchronized(lock) {
                activeStage.set(stage)
                timedOut.set(false)
                lastProgressNs.set(SystemClock.elapsedRealtimeNanos())
                append("VC_BENCHMARK_STAGE label=$label stage=$stage outcome=start")
            }
        }

        fun pulse(): Boolean {
            lastProgressNs.set(SystemClock.elapsedRealtimeNanos())
            return !timedOut()
        }

        fun finish() {
            synchronized(lock) {
                activeStage.get().takeIf { it.isNotEmpty() }?.let { append("VC_BENCHMARK_STAGE label=$label stage=$it outcome=finish") }
                activeStage.set("")
            }
        }

        fun timedOut(): Boolean = timedOut.get()

        fun fail(error: Throwable) = synchronized(lock) {
            append("VC_BENCHMARK_STAGE label=$label stage=${activeStage.get()} outcome=failure type=${error.javaClass.simpleName}")
            activeStage.set("")
        }

        override fun close() {
            executor.shutdownNow()
            executor.awaitTermination(5, TimeUnit.SECONDS)
        }

        private fun append(line: String) = synchronized(resultFile) { resultFile.appendText("$line\n") }
    }

    private companion object {
        const val TAG = "VcCoreBenchmark"
        // Keep the default 256 MiB opt-in benchmark below common device-lab
        // instrumentation watchdog limits. Longer statistical runs remain
        // available through vc.benchmark.warmups and vc.benchmark.runs.
        const val WARMUP_RUNS = 0
        const val MEASURED_RUNS = 1
        const val VOLUME_BYTES = 256L * 1024 * 1024
        const val LOGICAL_BYTES = VOLUME_BYTES - 256L * 1024
        const val IO_CHUNK_BYTES = 256 * 1024
        const val RANDOM_IO_BYTES = 4 * 1024
        const val RANDOM_READ_OPERATIONS = 4096
        const val PASSWORD = "benchmark-only"
        const val PATTERN: Byte = 0x5A
        const val STAGE_TIMEOUT_NS = 60L * 1_000_000_000L
        const val SUSTAINED_DURATION_NS = 5L * 60L * 1_000_000_000L
        const val SUSTAINED_REPORT_INTERVAL_NS = 5L * 1_000_000_000L
    }
}
