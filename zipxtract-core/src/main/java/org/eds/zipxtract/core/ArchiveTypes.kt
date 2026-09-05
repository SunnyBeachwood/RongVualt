/*
 * Copyright (C) 2023 WirelessAlien <https://github.com/WirelessAlien>
 * Copyright (C) 2026 RongVualt contributors
 *
 * This file is derived from ZipXtract and is distributed under the GNU GPL v3.
 */

package org.eds.zipxtract.core

import java.io.Closeable
import java.io.InputStream
import java.io.OutputStream
import java.nio.channels.SeekableByteChannel
import java.util.concurrent.atomic.AtomicBoolean

/** The archive families understood by the ZipXtract compatibility layer. */
enum class ArchiveFormat {
    ZIP,
    SEVEN_ZIP,
    RAR,
    TAR,
    COMPRESSED_TAR,
    /** A single gzip/bzip2/xz/zstd/lzma stream rather than a tar container. */
    COMPRESSED_STREAM,
    GENERIC,
    UNKNOWN,
}

enum class ArchiveEncryption {
    NONE,
    ZIP_STANDARD,
    ZIP_STANDARD_STRONG,
    AES,
    HEADER,
    UNKNOWN,
}

enum class ArchiveCapability {
    LIST,
    EXTRACT,
    CREATE,
    UPDATE,
    MULTI_VOLUME,
    ENCRYPTION,
}

/** Stable capability value exposed to the file-manager adapter. */
data class ArchiveCapabilities(
    val format: ArchiveFormat,
    val operations: Set<ArchiveCapability>,
    val encryption: ArchiveEncryption = ArchiveEncryption.NONE,
) {
    fun supports(capability: ArchiveCapability): Boolean = capability in operations
}

data class ArchiveProbe(
    val format: ArchiveFormat,
    val displayName: String,
    val encryption: ArchiveEncryption = ArchiveEncryption.NONE,
    val capabilities: Set<ArchiveCapability> = setOf(
        ArchiveCapability.LIST,
        ArchiveCapability.EXTRACT,
    ),
    val volumeNames: List<String> = emptyList(),
) {
    val archiveCapabilities: ArchiveCapabilities
        get() = ArchiveCapabilities(format, capabilities, encryption)
}

/**
 * A provider-neutral input. Implementations must not expose a host path as the
 * identity of the source; this is what lets an unlocked volume and a
 * DocumentsProvider participate in the same archive job as ordinary storage.
 */
interface ArchiveSource {
    val displayName: String
    val size: Long

    fun openInputStream(): InputStream

    /**
     * Optional addressable access for engines that need random reads (for
     * example a central-directory parser). Stream-only providers return null;
     * file-oriented callers can still stage the container privately.
     */
    fun openSeekableByteChannel(): SeekableByteChannel? = null

    /** Returns the sibling volume with [name], or null when it is unavailable. */
    fun openSibling(name: String): ArchiveSource? = null
}

/** A provider-neutral destination used by extraction and archive creation. */
interface ArchiveTarget {
    val displayName: String

    fun exists(): Boolean
    fun isDirectory(): Boolean
    fun createDirectory()
    fun openOutputStream(overwrite: Boolean = false): OutputStream

    /** Optional random-access output used by engines that can write in place. */
    fun openSeekableByteChannel(overwrite: Boolean = false): SeekableByteChannel? = null
    /** The parent target, or null when this target is the provider root. */
    fun parent(): ArchiveTarget?
    fun resolve(name: String): ArchiveTarget
    fun resolveSibling(name: String): ArchiveTarget
    fun deleteIfExists()
}

data class ArchiveEntry(
    val name: String,
    val isDirectory: Boolean,
    val size: Long = 0,
    val lastModifiedEpochMillis: Long? = null,
    val encrypted: Boolean = false,
    /** Archive links are metadata, never extraction targets. */
    val isSymbolicLink: Boolean = false,
)

data class ExtractRequest(
    val source: ArchiveSource,
    val destination: ArchiveTarget,
    val entries: Set<String>? = null,
    val createContainingDirectory: Boolean = false,
    val password: CharArray? = null,
    val overwrite: Boolean = false,
    val onConflict: ((target: ArchiveTarget, entry: ArchiveEntry) -> ArchiveConflictAction)? = null,
    val cancellation: CancellationToken = CancellationToken(),
)

enum class ArchiveConflictAction {
    REPLACE,
    SKIP,
    ABORT,
}

data class CreateArchiveRequest(
    val sources: List<ArchiveInputEntry>,
    val destination: ArchiveTarget,
    val format: ArchiveFormat,
    val options: ArchiveCreateOptions = ArchiveCreateOptions(),
    val password: CharArray? = null,
    val cancellation: CancellationToken = CancellationToken(),
)

data class ArchiveInputEntry(
    val name: String,
    val isDirectory: Boolean,
    val size: Long,
    val lastModifiedEpochMillis: Long? = null,
    val openInputStream: (() -> InputStream)? = null,
)

data class Update7zRequest(
    val source: ArchiveSource,
    val destination: ArchiveTarget,
    val additions: List<ArchiveInputEntry> = emptyList(),
    val removals: Set<String> = emptySet(),
    val password: CharArray? = null,
    val cancellation: CancellationToken = CancellationToken(),
)

data class ArchiveCreateOptions(
    val zipCompression: ZipCompression = ZipCompression.DEFLATE,
    val zipCompressionLevel: Int = 5,
    val zipEncryption: ArchiveEncryption = ArchiveEncryption.NONE,
    val zipAesKeyBits: Int = 256,
    val zipSplitSizeBytes: Long? = null,
    /** Set only after the UI has shown its >100-volume confirmation. */
    val allowLargeZipSplit: Boolean = false,
    val sevenZipCompressionLevel: Int = 5,
    val sevenZipSolid: Boolean = false,
    val sevenZipThreadCount: Int = 2,
    val tarCompression: TarCompression = TarCompression.NONE,
    val tarZstdLevel: Int = 3,
)

fun ArchiveCreateOptions.validate(format: ArchiveFormat, password: CharArray?, inputBytes: Long = 0L) {
    require(zipCompressionLevel in 0..9) { "ZIP compression level must be 0..9" }
    require(zipAesKeyBits == 128 || zipAesKeyBits == 256) {
        "ZIP AES key size must be 128 or 256 bits"
    }
    require(sevenZipCompressionLevel in 0..9) { "7z compression level must be 0..9" }
    require(sevenZipThreadCount in 1..64) { "7z thread count must be 1..64" }
    require(tarZstdLevel in 0..22) { "Zstandard level must be 0..22" }
    if (format == ArchiveFormat.ZIP && zipSplitSizeBytes != null) {
        require(zipSplitSizeBytes >= 64L * 1024L) { "ZIP split size must be at least 64 KiB" }
        val estimatedVolumes = if (inputBytes <= 0L) 0L else
            ((inputBytes - 1L) / zipSplitSizeBytes) + 1L
        require(estimatedVolumes <= 100L || allowLargeZipSplit) {
            "More than 100 ZIP volumes requires explicit confirmation"
        }
    }
    if (format == ArchiveFormat.ZIP && zipEncryption !in setOf(
            ArchiveEncryption.NONE,
            ArchiveEncryption.ZIP_STANDARD,
            ArchiveEncryption.ZIP_STANDARD_STRONG,
            ArchiveEncryption.AES,
        )
    ) {
        throw UnsupportedArchiveException("Unsupported ZIP encryption mode")
    }
    if (format != ArchiveFormat.ZIP && zipEncryption != ArchiveEncryption.NONE) {
        throw UnsupportedArchiveException("ZIP encryption options are only valid for ZIP")
    }
    if (zipEncryption != ArchiveEncryption.NONE && password.isNullOrEmpty()) {
        throw ArchivePasswordException("An encryption password is required")
    }
    if (format != ArchiveFormat.ZIP && format != ArchiveFormat.SEVEN_ZIP &&
        !password.isNullOrEmpty()
    ) {
        throw UnsupportedArchiveException("Passwords are only supported for ZIP and 7z")
    }
    if (format == ArchiveFormat.ZIP && zipEncryption == ArchiveEncryption.NONE &&
        !password.isNullOrEmpty()
    ) {
        throw UnsupportedArchiveException("A ZIP password requires an encryption mode")
    }
}

enum class ZipCompression { STORE, DEFLATE }

enum class TarCompression { NONE, GZIP, BZIP2, XZ, LZMA, ZSTD }

data class ArchiveProgress(
    val completedBytes: Long,
    val totalBytes: Long,
    val completedEntries: Long,
    val totalEntries: Long,
) {
    val percent: Int
        get() = if (totalBytes > 0) {
            ((completedBytes.coerceIn(0, totalBytes) * 100) / totalBytes).toInt()
        } else if (totalEntries > 0) {
            ((completedEntries.coerceIn(0, totalEntries) * 100) / totalEntries).toInt()
        } else {
            0
        }
}

fun interface ArchiveProgressListener {
    fun onProgress(progress: ArchiveProgress)
}

open class CancellationToken {
    private val cancelled = AtomicBoolean(false)

    val isCancelled: Boolean
        get() = cancelled.get()

    open fun cancel() {
        cancelled.set(true)
    }

    open fun throwIfCancelled() {
        if (isCancelled) throw ArchiveCancelledException()
    }
}

class ArchiveCancelledException : java.io.InterruptedIOException("Archive operation cancelled")

open class ArchiveException(message: String, cause: Throwable? = null) : Exception(message, cause)

class ArchivePasswordException(
    message: String = "Archive password is required",
    cause: Throwable? = null,
) : ArchiveException(message, cause)

class ArchiveMissingVolumeException(
    val missingVolumes: List<String>,
) : ArchiveException("Missing archive volume(s): ${missingVolumes.joinToString(", ")}")

interface ArchiveEngine {
    fun probe(source: ArchiveSource): ArchiveProbe
    fun list(source: ArchiveSource, password: CharArray? = null): List<ArchiveEntry>
    fun extract(request: ExtractRequest, listener: ArchiveProgressListener? = null)
    fun create(request: CreateArchiveRequest, listener: ArchiveProgressListener? = null)
    fun update7z(request: Update7zRequest, listener: ArchiveProgressListener? = null)
}

/**
 * The host owns the concrete engines so that this module stays independent of
 * Material Files' Path implementation. Backends are selected in order; a
 * backend that cannot handle a format must throw [UnsupportedArchiveException].
 */
class ArchiveEngineRouter(
    private val engines: List<ArchiveEngine>,
) : ArchiveEngine {
    private fun engineFor(source: ArchiveSource): ArchiveEngine =
        engines.firstOrNull {
            runCatching {
                val format = it.probe(source).format
                format != ArchiveFormat.UNKNOWN && it.supportsFormat(format)
            }.getOrDefault(false)
        }
            ?: throw UnsupportedArchiveException("No archive engine can read ${source.displayName}")

    override fun probe(source: ArchiveSource): ArchiveProbe =
        engines.asSequence().mapNotNull { runCatching { it.probe(source) }.getOrNull() }
            .firstOrNull { it.format != ArchiveFormat.UNKNOWN }
            ?: ArchiveProbe(ArchiveFormat.UNKNOWN, source.displayName)

    override fun list(source: ArchiveSource, password: CharArray?): List<ArchiveEntry> =
        engineFor(source).list(source, password)

    override fun extract(request: ExtractRequest, listener: ArchiveProgressListener?) =
        engineFor(request.source).extract(request, listener)

    override fun create(request: CreateArchiveRequest, listener: ArchiveProgressListener?) {
        engines.firstOrNull { it.supportsCreate(request.format) }
            ?.create(request, listener)
            ?: throw UnsupportedArchiveException("No archive engine can create ${request.format}")
    }

    override fun update7z(request: Update7zRequest, listener: ArchiveProgressListener?) {
        engineFor(request.source).update7z(request, listener)
    }

}

interface ArchiveEngineCapabilities {
    fun supportsFormat(format: ArchiveFormat): Boolean = true
    fun supportsCreate(format: ArchiveFormat): Boolean = false
}

class UnsupportedArchiveException(message: String) : ArchiveException(message)

fun ArchiveEngine.supportsCreate(format: ArchiveFormat): Boolean =
    (this as? ArchiveEngineCapabilities)?.supportsCreate(format) == true

fun ArchiveEngine.supportsFormat(format: ArchiveFormat): Boolean =
    (this as? ArchiveEngineCapabilities)?.supportsFormat(format) ?: true

/** Best-effort cleanup helper used by host adapters in finally blocks. */
fun CharArray?.wipe() {
    this?.fill('\u0000')
}

/** Close a source stream and wipe a password without masking the main failure. */
inline fun <T> withArchivePassword(password: CharArray?, block: () -> T): T =
    try {
        block()
    } finally {
        password.wipe()
    }

interface ArchiveTempStore : Closeable {
    fun stage(source: ArchiveSource): java.io.File
}
