package org.eds.veracrypt.documents

import android.net.Uri
import java.util.UUID

internal enum class TransferDirection { ENCRYPT_IMPORT, DECRYPT_EXPORT }

internal data class TransferRequest(
    val id: UUID = UUID.randomUUID(),
    val direction: TransferDirection,
    val sourceUris: List<Uri>,
    val targetDirectoryUri: Uri,
    val volumeId: UUID,
)

internal data class TransferProgress(
    val direction: TransferDirection,
    val currentFileName: String,
    val currentFileIndex: Int,
    val totalFileCount: Int,
    val currentFileBytes: Long,
    val currentFileTotalBytes: Long?,
    val completedBytes: Long,
    val totalBytes: Long?,
    val percent: Int?,
    val bytesPerSecond: Long,
    val remainingSeconds: Long?,
    val failures: List<String> = emptyList(),
)

internal sealed interface TransferState {
    data object Idle : TransferState
    data class Preparing(val request: TransferRequest) : TransferState
    data class Running(val request: TransferRequest, val progress: TransferProgress) : TransferState
    data class Cancelling(val request: TransferRequest, val progress: TransferProgress?) : TransferState
    data class Cancelled(val request: TransferRequest, val progress: TransferProgress?) : TransferState
    data class Completed(val request: TransferRequest, val progress: TransferProgress) : TransferState
    data class PartialSuccess(val request: TransferRequest, val progress: TransferProgress) : TransferState
    data class Failed(val request: TransferRequest, val progress: TransferProgress?, val reason: String) : TransferState
}

internal data class TransferProgressSample(val timeMillis: Long, val bytes: Long)

internal fun allocateTransferName(original: String, existing: Set<String>): String {
    if (original !in existing) return original
    val dot = original.lastIndexOf('.')
    val stem = if (dot > 0) original.substring(0, dot) else original
    val extension = if (dot > 0) original.substring(dot) else ""
    var index = 1
    while ((stem + " (" + index + ")" + extension) in existing) index++
    return stem + " (" + index + ")" + extension
}

/** Pure progress math used by the worker and unit tests. */
internal class TransferProgressCalculator(
    private val windowMillis: Long = 4_000L,
    private val clockMillis: () -> Long = { System.currentTimeMillis() },
) {
    private val samples = ArrayDeque<TransferProgressSample>()

    fun update(
        direction: TransferDirection,
        fileName: String,
        fileIndex: Int,
        fileCount: Int,
        currentFileBytes: Long,
        currentFileTotalBytes: Long?,
        completedBytes: Long,
        totalBytes: Long?,
        failures: List<String>,
    ): TransferProgress {
        val now = clockMillis()
        samples.addLast(TransferProgressSample(now, completedBytes))
        while (samples.size > 1 && now - samples.first().timeMillis > windowMillis) samples.removeFirst()
        val first = samples.first()
        val elapsed = now - first.timeMillis
        val delta = (completedBytes - first.bytes).coerceAtLeast(0L)
        val speed = if (elapsed <= 0L) 0L else safeMultiplyDivide(delta, 1000L, elapsed)
        val percent = totalBytes?.takeIf { it > 0L }?.let {
            val bounded = completedBytes.coerceIn(0L, it)
            (bounded / it * 100L + (bounded % it) * 100L / it).toInt().coerceIn(0, 100)
        }
        val remaining = if (speed > 0L && totalBytes != null) ((totalBytes - completedBytes).coerceAtLeast(0L) / speed) else null
        return TransferProgress(direction, fileName, fileIndex, fileCount, currentFileBytes, currentFileTotalBytes, completedBytes, totalBytes, percent, speed, remaining, failures)
    }

    private fun safeMultiplyDivide(value: Long, multiplier: Long, divisor: Long): Long =
        if (value == 0L || divisor <= 0L) 0L
        else if (value <= Long.MAX_VALUE / multiplier) value * multiplier / divisor
        else {
            val quotient = value / divisor
            if (quotient > Long.MAX_VALUE / multiplier) Long.MAX_VALUE else quotient * multiplier
        }
}
