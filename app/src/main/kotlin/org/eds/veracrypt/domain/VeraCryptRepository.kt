package org.eds.veracrypt.domain

import android.net.Uri
import org.eds.veracrypt.catalog.ContainerCatalogEntry

interface VeraCryptRepository {
    suspend fun probe(container: Uri): VolumeProbe
    suspend fun open(
        container: Uri,
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
        progress: VolumeUnlockProgress = VolumeUnlockProgress.NONE,
    ): VolumeSession
    /** Catalog opens use the persistent random container ID for session arbitration. */
    suspend fun open(
        entry: ContainerCatalogEntry,
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
        progress: VolumeUnlockProgress = VolumeUnlockProgress.NONE,
    ): VolumeSession
    suspend fun createNormal(container: Uri, options: VolumeCreateOptions, credentials: VolumeCredentials): VolumeSession
    /** Creates and registers a normal volume under the catalog's random ID. */
    suspend fun createNormal(entry: ContainerCatalogEntry, options: VolumeCreateOptions, credentials: VolumeCredentials): VolumeSession
    /**
     * Creates a catalog-backed normal volume while reporting native full-format
     * progress. Implementations that do not support callbacks retain the
     * established creation behavior.
     */
    suspend fun createNormal(
        entry: ContainerCatalogEntry,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
        progress: VolumeCreateProgress,
    ): VolumeSession = createNormal(entry, options, credentials)
    /** Scans the formatted outer volume for tail-contiguous space safe for a hidden volume. */
    suspend fun analyzeHiddenCapacity(outerSession: VolumeSession): HiddenVolumeCapacity
    /** Creates a hidden volume only inside capacity returned for this live outer session. */
    suspend fun createHidden(outerSession: VolumeSession, options: VolumeCreateOptions, credentials: VolumeCredentials): VolumeSession
    /** Catalog-backed hidden creation replaces the outer DocumentsProvider root on success. */
    suspend fun createHidden(entry: ContainerCatalogEntry, outerSession: VolumeSession, options: VolumeCreateOptions, credentials: VolumeCredentials): VolumeSession
    /** Creates a catalog-backed hidden volume while reporting native full-format progress. */
    suspend fun createHidden(
        entry: ContainerCatalogEntry,
        outerSession: VolumeSession,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
        progress: VolumeCreateProgress,
    ): VolumeSession = createHidden(entry, outerSession, options, credentials)
    suspend fun changeCredentials(session: VolumeSession, newCredentials: VolumeCredentials)
    /** Re-encrypts a fresh selected-type header; credentials are never retained by the session. */
    suspend fun backupHeader(session: VolumeSession, destination: Uri, credentials: VolumeCredentials)
    suspend fun restoreHeader(container: Uri, source: Uri, options: VolumeOpenOptions, credentials: VolumeCredentials)
    suspend fun close(session: VolumeSession)
}

/** Request-scoped callback for creation work; it never receives secrets or paths. */
interface VolumeCreateProgress {
    fun onProgress(stage: Int, completedBytes: Long, totalBytes: Long)
    fun isCancellationRequested(): Boolean

    companion object {
        val NONE: VolumeCreateProgress = object : VolumeCreateProgress {
            override fun onProgress(stage: Int, completedBytes: Long, totalBytes: Long) = Unit
            override fun isCancellationRequested() = false
        }
    }
}

/** Coarse but truthful unlock phases; KDF work cannot expose a safe continuous percentage. */
enum class VolumeUnlockStage {
    PREPARING_INPUTS,
    DERIVING_AND_VERIFYING,
    MOUNTING_FILESYSTEM,
    REGISTERING_PROVIDER,
}

interface VolumeUnlockProgress {
    fun onStage(stage: VolumeUnlockStage)
    /** Candidate counts are exact for the selected KDF/cipher candidates. */
    fun onProbe(completed: Int, total: Int) = Unit
    fun isCancellationRequested(): Boolean = false

    companion object {
        val NONE: VolumeUnlockProgress = object : VolumeUnlockProgress {
            override fun onStage(stage: VolumeUnlockStage) = Unit
        }
    }
}

data class VolumeProbe(
    val isSeekableContainer: Boolean,
    val capacityBytes: Long?,
    val isWritable: Boolean,
)

/** Capacity is tied to one mounted outer session and invalid after it closes. */
data class HiddenVolumeCapacity(
    val maximumBytes: Long,
    val encryptedAreaOffset: Long,
)

sealed class VolumeError(message: String, cause: Throwable? = null) : Exception(message, cause) {
    class InvalidCredentialsOrFormat : VolumeError("Invalid credentials or unsupported container format")
    class CorruptHeader : VolumeError("The volume header is corrupt")
    class UnsupportedAlgorithm : VolumeError("The selected cipher or KDF is unsupported")
    class UnsupportedFileSystem : VolumeError("The encrypted filesystem is unsupported")
    class InsufficientMemory : VolumeError("Insufficient memory for the selected KDF")
    class SourceNotSeekable : VolumeError("The container source is not seekable or has unknown length")
    class ReadOnlySource : VolumeError("The container source is read-only")
    class HiddenVolumeRisk : VolumeError("The write could damage the hidden volume")
    class Cancelled : VolumeError("The operation was cancelled")
    class IoInterrupted(cause: Throwable? = null) : VolumeError("Container I/O was interrupted", cause)
    /** A filesystem entry does not exist; distinct from media I/O failure. */
    class NotFound : VolumeError("The requested filesystem entry does not exist")
}
