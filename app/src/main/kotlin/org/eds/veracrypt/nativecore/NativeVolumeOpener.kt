package org.eds.veracrypt.nativecore

import android.content.Context
import android.net.Uri
import java.security.MessageDigest
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.eds.veracrypt.domain.VolumeError
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenTarget
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.domain.VolumeUnlockProgress
import org.eds.veracrypt.domain.VolumeUnlockStage
import org.eds.veracrypt.domain.VolumeSession
import org.eds.veracrypt.keyfiles.KeyfileSourceExpander
import org.eds.veracrypt.session.ManagedVolumeSession
import org.eds.veracrypt.session.VolumeSessionRegistry
import org.eds.veracrypt.storage.SeekableContainer

/** SAF-to-JNI open path; it deliberately exposes neither paths nor keys. */
internal class NativeVolumeOpener(
    private val context: Context,
    private val scope: CoroutineScope,
    private val sessions: VolumeSessionRegistry,
) {
    private val resolver = context.contentResolver
    suspend fun createNormal(
        containerUri: Uri,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
        sessionKey: String = containerKey(containerUri),
        progress: NativeCreateProgress = NativeCreateProgress.inert(),
    ): VolumeSession = withContext(Dispatchers.IO) {
        require(options.volumeKind == VolumeKind.NORMAL) { "Normal creation requires normal-volume options" }
        var container: SeekableContainer? = null
        var lease: VolumeSessionRegistry.Lease? = null
        var nativeHandle = 0L
        var ownershipTransferred = false
        val keyfiles = mutableListOf<SeekableContainer>()
        try {
            container = SeekableContainer.open(resolver, containerUri, org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE)
            lease = sessions.acquire(sessionKey, org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE)
            val expander = KeyfileSourceExpander(context)
            val expanded = credentials.keyfiles.takeIf { it.isNotEmpty() }?.let(expander::expand).orEmpty()
            expanded.forEach { source ->
                keyfiles += SeekableContainer.open(resolver, Uri.parse(source.uri), org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY)
            }
            nativeHandle = NativeRequestCodec.encodeCreate(options, credentials, expanded.size).use { request ->
                request.useForJni { bytes ->
                    try {
                        VcCore.nativeCreateNormal(checkNotNull(container).fd, bytes, keyfiles.map { it.fd }.toIntArray(), progress)
                    } catch (failure: VcCoreFailure) {
                        throw failure.asVolumeError()
                    }
                }
            }
            check(nativeHandle > 0L) { "Native volume creation returned no session handle" }
            val activeContainer = checkNotNull(container)
            val activeLease = checkNotNull(lease)
            val session = ManagedVolumeSession(
                nativeHandle,
                org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE,
                VolumeKind.NORMAL,
                scope,
                onClose = {
                var flushFailure: Throwable? = null
                try {
                    VcCore.nativeFlush(nativeHandle)
                } catch (error: Throwable) {
                    flushFailure = error
                } finally {
                    try {
                        VcCore.nativeClose(nativeHandle)
                    } finally {
                        activeContainer.close()
                        activeLease.close()
                    }
                }
                flushFailure?.let { throw it }
                },
            )
            session.mountFileSystem()
            session.enableAutoLock(DEFAULT_AUTO_LOCK_MILLIS)
            ownershipTransferred = true
            session
        } catch (error: Throwable) {
            if (nativeHandle != 0L) VcCore.nativeClose(nativeHandle)
            throw when (error) {
                is VolumeError -> error
                is CancellationException -> error
                else -> VolumeError.IoInterrupted(error)
            }
        } finally {
            if (!ownershipTransferred) {
                container?.close()
                lease?.close()
            }
            keyfiles.forEach(SeekableContainer::close)
            credentials.close()
        }
    }

    suspend fun open(
        containerUri: Uri,
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
        sessionKey: String = containerKey(containerUri),
        progress: VolumeUnlockProgress = VolumeUnlockProgress.NONE,
    ): VolumeSession = withContext(Dispatchers.IO) {
        var container: SeekableContainer? = null
        var lease: VolumeSessionRegistry.Lease? = null
        var nativeHandle = 0L
        var ownershipTransferred = false
        var sourceAccessMode = org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY
        val keyfiles = mutableListOf<SeekableContainer>()
        try {
            progress.onStage(VolumeUnlockStage.PREPARING_INPUTS)
            // A manual open no longer exposes a dangerous read-only toggle.
            // Prefer a writable descriptor, but retain a usable read-only
            // session when the SAF grant or storage medium rejects writes.
            sourceAccessMode = when (options.accessMode) {
                org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE -> org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE
                org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY -> org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY
                org.eds.veracrypt.domain.VolumeAccessMode.AUTOMATIC -> try {
                    container = SeekableContainer.open(resolver, containerUri, org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE)
                    org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE
                } catch (_: VolumeError.ReadOnlySource) {
                    org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY
                }
            }
            if (container == null) container = SeekableContainer.open(resolver, containerUri, sourceAccessMode)
            lease = sessions.acquire(sessionKey, sourceAccessMode)
            val expander = KeyfileSourceExpander(context)
            val primaryKeyfiles = credentials.keyfiles.takeIf { it.isNotEmpty() }?.let(expander::expand).orEmpty()
            val protectionKeyfiles = options.hiddenVolumeProtection?.keyfiles?.takeIf { it.isNotEmpty() }?.let(expander::expand).orEmpty()
            primaryKeyfiles.forEach { source ->
                keyfiles += SeekableContainer.open(resolver, Uri.parse(source.uri), org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY)
            }
            protectionKeyfiles.forEach { source ->
                keyfiles += SeekableContainer.open(resolver, Uri.parse(source.uri), org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY)
            }
            progress.onStage(VolumeUnlockStage.DERIVING_AND_VERIFYING)
            val nativeOptions = options.copy(accessMode = sourceAccessMode)
            val nativeProgress = NativeUnlockProgress(onUpdate = { completed, total ->
                progress.onProbe(completed, total)
                !progress.isCancellationRequested()
            })
            nativeHandle = NativeRequestCodec.encodeOpen(nativeOptions, credentials, primaryKeyfiles.size, protectionKeyfiles.size).use { request ->
                request.useForJni { bytes ->
                    try {
                        VcCore.nativeOpenWithProgress(checkNotNull(container).fd, sourceAccessMode == org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE, bytes, keyfiles.map { it.fd }.toIntArray(), nativeProgress)
                    } catch (failure: VcCoreFailure) {
                        throw failure.asVolumeError()
                    }
                }
            }
            check(nativeHandle > 0L) { "Native volume opening returned no session handle" }
            val activeContainer = checkNotNull(container)
            val activeLease = checkNotNull(lease)
            progress.onStage(VolumeUnlockStage.MOUNTING_FILESYSTEM)
            val mountedFileSystem = try {
                when (VcCore.nativeMountFileSystem(nativeHandle)) {
                    1 -> org.eds.veracrypt.domain.VolumeFileSystem.FAT
                    2 -> org.eds.veracrypt.domain.VolumeFileSystem.EXFAT
                    3 -> org.eds.veracrypt.domain.VolumeFileSystem.NTFS
                    else -> throw IllegalStateException("Native mount returned an unknown filesystem type")
                }
            } catch (failure: VcCoreFailure) {
                throw failure.asVolumeError()
            }
            val effectiveAccessMode = if (
                sourceAccessMode == org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE &&
                mountedFileSystem != org.eds.veracrypt.domain.VolumeFileSystem.NTFS
            ) org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE else org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY
            val session = ManagedVolumeSession(
                nativeHandle,
                effectiveAccessMode,
                if (VcCore.volumeInfo(nativeHandle).isHiddenVolume) VolumeKind.HIDDEN else VolumeKind.NORMAL,
                scope,
                mountedFileSystem,
                canModifyContainer = sourceAccessMode == org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE,
                onClose = {
                var flushFailure: Throwable? = null
                try {
                    VcCore.nativeFlush(nativeHandle)
                } catch (error: Throwable) {
                    flushFailure = error
                } finally {
                    try {
                        VcCore.nativeClose(nativeHandle)
                    } finally {
                        activeContainer.close()
                        activeLease.close()
                    }
                }
                flushFailure?.let { throw it }
                },
            )
            session.enableAutoLock(DEFAULT_AUTO_LOCK_MILLIS)
            ownershipTransferred = true
            session
        } catch (error: Throwable) {
            if (nativeHandle != 0L) VcCore.nativeClose(nativeHandle)
            throw when (error) {
                is VolumeError -> error
                // Cancellation is control flow, not an unlock failure. Let
                // the parent scope observe it after releasing native inputs.
                is CancellationException -> error
                else -> VolumeError.IoInterrupted(error)
            }
        } finally {
            if (!ownershipTransferred) {
                container?.close()
                lease?.close()
            }
            keyfiles.forEach(SeekableContainer::close)
            // Opening consumes credentials exactly once. Closing is idempotent,
            // so callers such as the ViewModel may also clear defensively.
            credentials.close()
            options.hiddenVolumeProtection?.close()
        }
    }

    private fun containerKey(uri: Uri): String {
        val digest = MessageDigest.getInstance("SHA-256").digest(uri.toString().toByteArray(Charsets.UTF_8))
        return digest.joinToString(separator = "") { byte -> "%02x".format(byte.toInt() and 0xff) }
    }

    private companion object {
        const val DEFAULT_AUTO_LOCK_MILLIS = 5 * 60 * 1000L
    }
}
