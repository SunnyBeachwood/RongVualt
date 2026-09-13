package org.eds.veracrypt.nativecore

import android.content.Context
import android.net.Uri
import android.system.Os
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.eds.veracrypt.domain.VeraCryptRepository
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCreateProgress
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeError
import org.eds.veracrypt.domain.HiddenVolumeCapacity
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.domain.VolumeProbe
import org.eds.veracrypt.domain.VolumeSession
import org.eds.veracrypt.domain.VolumeUnlockProgress
import org.eds.veracrypt.domain.VolumeUnlockStage
import org.eds.veracrypt.catalog.ContainerCatalogEntry
import org.eds.veracrypt.catalog.ContainerCatalog
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.documents.UnlockOperation
import org.eds.veracrypt.keyfiles.KeyfileSourceExpander
import org.eds.veracrypt.session.VolumeSessionRegistry
import org.eds.veracrypt.session.ManagedVolumeSession
import org.eds.veracrypt.storage.SeekableContainer

/** Repository implementation for the native open/read/write milestone. */
class NativeVeraCryptRepository(
    private val context: Context,
    private val scope: CoroutineScope,
    sessions: VolumeSessionRegistry = VolumeSessionRegistry(),
) : VeraCryptRepository {
    private val resolver = context.contentResolver
    private val catalog = ContainerCatalog(context)
    private val opener = NativeVolumeOpener(context, scope, sessions)

    override suspend fun probe(container: Uri): VolumeProbe =
        UnlockedVolumeService.withForegroundOperation {
            withContext(Dispatchers.IO) {
                try {
                    SeekableContainer.open(resolver, container, org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE).use {
                        VolumeProbe(isSeekableContainer = true, capacityBytes = it.sizeBytes, isWritable = true)
                    }
                } catch (_: VolumeError.ReadOnlySource) {
                    SeekableContainer.open(resolver, container, org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY).use {
                        VolumeProbe(isSeekableContainer = true, capacityBytes = it.sizeBytes, isWritable = false)
                    }
                }
            }
        }

    override suspend fun open(
        container: Uri,
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
        progress: VolumeUnlockProgress,
    ): VolumeSession =
        UnlockedVolumeService.withUnlockOperation { operation ->
            opener.open(container, options, credentials, progress = forwardingUnlockProgress(operation, progress))
        }

    /** Preferred catalog-backed entry point: its random catalog UUID is the arbitration key. */
    override suspend fun open(
        entry: ContainerCatalogEntry,
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
        progress: VolumeUnlockProgress,
    ): VolumeSession =
        UnlockedVolumeService.withUnlockOperation { operation ->
            val combinedProgress = forwardingUnlockProgress(operation, progress)
            val session = opener.open(Uri.parse(entry.uri), options, credentials, entry.id.toString(), combinedProgress)
            try {
                combinedProgress.onStage(VolumeUnlockStage.REGISTERING_PROVIDER)
                UnlockedVolumeService.volumes.add(
                    entry.id, entry.displayName, session, logicalSizeBytes(session)
                )
                UnlockedVolumeService.notifyRootsChanged()
                session
            } catch (error: Throwable) {
                session.close()
                throw error
            }
        }

    override suspend fun close(session: VolumeSession) {
        if (!UnlockedVolumeService.volumes.close(session)) session.close()
    }

    private fun forwardingUnlockProgress(
        operation: UnlockOperation,
        delegate: VolumeUnlockProgress,
    ): VolumeUnlockProgress = object : VolumeUnlockProgress {
        override fun onStage(stage: VolumeUnlockStage) {
            operation.update(stage)
            delegate.onStage(stage)
        }

        override fun onProbe(completed: Int, total: Int) = delegate.onProbe(completed, total)
        override fun isCancellationRequested() = delegate.isCancellationRequested()
    }

    private fun logicalSizeBytes(session: VolumeSession): Long? =
        (session as? ManagedVolumeSession)?.let { managed ->
            runCatching { VcCore.volumeInfo(managed.nativeHandle).logicalSize }
                .getOrNull()
                ?.takeIf { it > 0L }
        }

    override suspend fun createNormal(
        container: Uri,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
    ): VolumeSession {
        return UnlockedVolumeService.withForegroundOperation {
            opener.createNormal(container, options, credentials)
        }
    }

    override suspend fun createNormal(
        entry: ContainerCatalogEntry,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
        progress: VolumeCreateProgress,
    ): VolumeSession = UnlockedVolumeService.withForegroundOperation {
        val session = opener.createNormal(
            Uri.parse(entry.uri), options, credentials, entry.id.toString(), nativeProgress(progress),
        )
        try {
            UnlockedVolumeService.volumes.add(
                entry.id, entry.displayName, session, logicalSizeBytes(session)
            )
            UnlockedVolumeService.notifyRootsChanged()
            session
        } catch (error: Throwable) {
            session.close()
            throw error
        }
    }

    override suspend fun createNormal(
        entry: ContainerCatalogEntry,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
    ): VolumeSession = UnlockedVolumeService.withForegroundOperation {
        val session = opener.createNormal(Uri.parse(entry.uri), options, credentials, entry.id.toString())
        try {
            UnlockedVolumeService.volumes.add(
                entry.id, entry.displayName, session, logicalSizeBytes(session)
            )
            UnlockedVolumeService.notifyRootsChanged()
            session
        } catch (error: Throwable) {
            session.close()
            throw error
        }
    }

    override suspend fun analyzeHiddenCapacity(outerSession: VolumeSession): HiddenVolumeCapacity = UnlockedVolumeService.withForegroundOperation {
        withContext(Dispatchers.IO) {
        val nativeSession = outerSession as? ManagedVolumeSession
            ?: throw IllegalArgumentException("The session is not backed by vc_core")
        require(nativeSession.volumeKind == org.eds.veracrypt.domain.VolumeKind.NORMAL) {
            "Hidden capacity must be analyzed from an outer normal-volume session"
        }
        require(nativeSession.fileSystem != null) {
            "Mount the outer filesystem before analyzing hidden-volume capacity"
        }
        val detached = UnlockedVolumeService.volumes.detach(outerSession)
        try {
            val values = VcCore.nativeAnalyzeHiddenCapacity(nativeSession.nativeHandle)
            check(values.size == 2 && values.all { it >= 0L }) {
                "Native hidden-capacity result has an invalid shape"
            }
            HiddenVolumeCapacity(maximumBytes = values[0], encryptedAreaOffset = values[1])
        } catch (failure: VcCoreFailure) {
            throw failure.asVolumeError()
        } finally {
            detached?.let {
                UnlockedVolumeService.volumes.restore(it)
                UnlockedVolumeService.notifyRootsChanged()
            }
        }
        }
    }

    override suspend fun createHidden(
        outerSession: VolumeSession,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
    ): VolumeSession = UnlockedVolumeService.withForegroundOperation {
        createHiddenInternal(outerSession, options, credentials, NativeCreateProgress.inert())
    }

    private suspend fun createHiddenInternal(
        outerSession: VolumeSession,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
        progress: NativeCreateProgress,
    ): VolumeSession = withContext(Dispatchers.IO) {
        val outer = outerSession as? ManagedVolumeSession
            ?: throw IllegalArgumentException("The outer session is not backed by vc_core")
        require(outer.volumeKind == org.eds.veracrypt.domain.VolumeKind.NORMAL) {
            "A hidden volume must be created inside an outer normal volume"
        }
        require(outer.accessMode == org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE) {
            "Creating a hidden volume requires writable outer-volume access"
        }
        require(outer.fileSystem != null) {
            "Mount and analyze the outer filesystem before hidden-volume creation"
        }
        val keyfiles = mutableListOf<SeekableContainer>()
        var hiddenHandle = 0L
        try {
            val expander = KeyfileSourceExpander(context)
            val expanded = credentials.keyfiles.takeIf { it.isNotEmpty() }?.let(expander::expand).orEmpty()
            expanded.forEach { source ->
                keyfiles += SeekableContainer.open(
                    resolver,
                    Uri.parse(source.uri),
                    org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY,
                )
            }
            hiddenHandle = NativeRequestCodec.encodeCreate(options, credentials, expanded.size).use { request ->
                request.useForJni { bytes ->
                    try {
                        VcCore.nativeCreateHidden(outer.nativeHandle, bytes, keyfiles.map { it.fd }.toIntArray(), progress)
                    } catch (failure: VcCoreFailure) {
                        throw failure.asVolumeError()
                    }
                }
            }
            check(hiddenHandle > 0L) { "Native hidden-volume creation returned no session handle" }
            val result = ManagedVolumeSession(
                hiddenHandle,
                org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE,
                org.eds.veracrypt.domain.VolumeKind.HIDDEN,
                scope,
                onClose = {
                try {
                    VcCore.nativeClose(hiddenHandle)
                } finally {
                    // The child descriptor is independent, but its writer
                    // lease belongs to the outer session. Closing one closes
                    // both so the container cannot remain unlocked without a
                    // corresponding arbitration lease.
                    outerSession.close()
                }
                },
                sourceUri = outer.sourceUri,
            )
            result.mountFileSystem()
            outer.registerDependent(result)
            check(result.state.value == org.eds.veracrypt.domain.VolumeSessionState.Open) {
                "Outer session closed while the hidden session was being registered"
            }
            result.enableAutoLock(DEFAULT_AUTO_LOCK_MILLIS)
            result
        } catch (error: Throwable) {
            if (hiddenHandle != 0L) VcCore.nativeClose(hiddenHandle)
            throw when (error) {
                is VolumeError -> error
                is CancellationException -> error
                else -> VolumeError.IoInterrupted(error)
            }
        } finally {
            keyfiles.forEach(SeekableContainer::close)
            credentials.close()
        }
    }

    override suspend fun createHidden(
        entry: ContainerCatalogEntry,
        outerSession: VolumeSession,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
    ): VolumeSession = UnlockedVolumeService.withForegroundOperation {
        val detached = UnlockedVolumeService.volumes.detach(outerSession)
            ?: throw IllegalStateException("The outer volume is not an active catalog session")
        try {
            val hidden = createHiddenInternal(outerSession, options, credentials, NativeCreateProgress.inert())
            UnlockedVolumeService.volumes.replace(detached, hidden, logicalSizeBytes(hidden))
            UnlockedVolumeService.notifyRootsChanged()
            hidden
        } catch (error: Throwable) {
            UnlockedVolumeService.volumes.restore(detached)
            UnlockedVolumeService.notifyRootsChanged()
            throw error
        }
    }

    override suspend fun createHidden(
        entry: ContainerCatalogEntry,
        outerSession: VolumeSession,
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
        progress: VolumeCreateProgress,
    ): VolumeSession = UnlockedVolumeService.withForegroundOperation {
        val detached = UnlockedVolumeService.volumes.detach(outerSession)
            ?: throw IllegalStateException("The outer volume is not an active catalog session")
        try {
            val hidden = createHiddenInternal(outerSession, options, credentials, nativeProgress(progress))
            UnlockedVolumeService.volumes.replace(detached, hidden, logicalSizeBytes(hidden))
            UnlockedVolumeService.notifyRootsChanged()
            hidden
        } catch (error: Throwable) {
            UnlockedVolumeService.volumes.restore(detached)
            UnlockedVolumeService.notifyRootsChanged()
            throw error
        }
    }

    override suspend fun changeCredentials(
        session: VolumeSession,
        newCredentials: VolumeCredentials,
    ) = UnlockedVolumeService.withForegroundOperation {
        withContext(Dispatchers.IO) {
        val nativeSession = session as? ManagedVolumeSession
            ?: throw IllegalArgumentException("The session is not backed by vc_core")
        require(nativeSession.canModifyContainer) {
            "Changing credentials requires write access to the container source"
        }
        // The already-unlocked native session is the authorization proof.
        // Only replacement credentials and their keyfiles cross JNI.
        // Remove the provider root while headers are being rewritten. This
        // closes all proxy descriptors and prevents concurrent filesystem I/O.
        val expander = KeyfileSourceExpander(context)
        val keyfiles = mutableListOf<SeekableContainer>()
        var detached: org.eds.veracrypt.session.UnlockedVolume? = null
        try {
            detached = UnlockedVolumeService.volumes.detach(session)
            val expanded = newCredentials.keyfiles.takeIf { it.isNotEmpty() }?.let(expander::expand).orEmpty()
            expanded.forEach { keyfile ->
                keyfiles += SeekableContainer.open(
                    resolver,
                    Uri.parse(keyfile.uri),
                    org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY,
                )
            }
            val replacement = VolumeOpenOptions(
                target = if (nativeSession.volumeKind == org.eds.veracrypt.domain.VolumeKind.HIDDEN) org.eds.veracrypt.domain.VolumeOpenTarget.HIDDEN else org.eds.veracrypt.domain.VolumeOpenTarget.NORMAL,
                accessMode = org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE,
            )
            NativeRequestCodec.encodeOpen(replacement, newCredentials, expanded.size, 0).use { request ->
                request.useForJni { bytes ->
                    try {
                        VcCore.nativeChangeCredentials(nativeSession.nativeHandle, bytes, keyfiles.map { it.fd }.toIntArray())
                    } catch (failure: VcCoreFailure) {
                        throw failure.asVolumeError()
                    }
                }
            }
        } finally {
            keyfiles.forEach(SeekableContainer::close)
            newCredentials.close()
            detached?.let {
                UnlockedVolumeService.volumes.restore(it)
                UnlockedVolumeService.notifyRootsChanged()
            }
        }
        }
    }

    private fun nativeProgress(progress: VolumeCreateProgress) = NativeCreateProgress(onUpdate =
        { stage, completedBytes, totalBytes ->
            progress.onProgress(stage, completedBytes, totalBytes)
            !progress.isCancellationRequested()
        },
    )

    private companion object {
        const val DEFAULT_AUTO_LOCK_MILLIS = 5 * 60 * 1000L
    }

    override suspend fun backupHeader(
        session: VolumeSession,
        destination: Uri,
        credentials: VolumeCredentials,
    ) = UnlockedVolumeService.withForegroundOperation {
        withContext(Dispatchers.IO) {
        val nativeSession = session as? ManagedVolumeSession
            ?: throw IllegalArgumentException("The session is not backed by vc_core")
        val keyfiles = mutableListOf<SeekableContainer>()
        try {
            val expander = KeyfileSourceExpander(context)
            val expanded = credentials.keyfiles.takeIf { it.isNotEmpty() }?.let(expander::expand).orEmpty()
            expanded.forEach { keyfile ->
                keyfiles += SeekableContainer.open(
                    resolver,
                    Uri.parse(keyfile.uri),
                    org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY,
                )
            }
            val options = VolumeOpenOptions(
                target = if (nativeSession.volumeKind == org.eds.veracrypt.domain.VolumeKind.HIDDEN) org.eds.veracrypt.domain.VolumeOpenTarget.HIDDEN else org.eds.veracrypt.domain.VolumeOpenTarget.NORMAL,
                accessMode = org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE,
            )
            check(nativeSession.sourceUri == null || !sameContainer(nativeSession.sourceUri, destination)) {
                "Header backup destination must be different from the source container"
            }
            SeekableContainer.open(resolver, destination, org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE).use { target ->
                NativeRequestCodec.encodeOpen(options, credentials, expanded.size, 0).use { request ->
                    request.useForJni { bytes ->
                        try {
                            VcCore.nativeBackupHeader(nativeSession.nativeHandle, target.fd, bytes, keyfiles.map { it.fd }.toIntArray())
                        } catch (failure: VcCoreFailure) {
                            throw failure.asVolumeError()
                        }
                    }
                }
            }
        } finally {
            keyfiles.forEach(SeekableContainer::close)
            credentials.close()
        }
        }
    }

    override suspend fun restoreHeader(
        container: Uri,
        source: Uri,
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
    ) = UnlockedVolumeService.withForegroundOperation {
        withContext(Dispatchers.IO) {
        var target: SeekableContainer? = null
        var backup: SeekableContainer? = null
        val keyfiles = mutableListOf<SeekableContainer>()
        try {
            target = SeekableContainer.open(resolver, container, org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE)
            backup = SeekableContainer.open(resolver, source, org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY)
            check(!sameContainer(container, source)) {
                "Header restore source must be different from the target container"
            }
            val expander = KeyfileSourceExpander(context)
            val expanded = credentials.keyfiles.takeIf { it.isNotEmpty() }
                ?.let(expander::expand)
                .orEmpty()
            expanded.forEach { keyfile ->
                keyfiles += SeekableContainer.open(
                    resolver,
                    Uri.parse(keyfile.uri),
                    org.eds.veracrypt.domain.VolumeAccessMode.READ_ONLY,
                )
            }
            val restoreOptions = options.copy(
                accessMode = org.eds.veracrypt.domain.VolumeAccessMode.READ_WRITE,
                hiddenVolumeProtection = null,
            )
            NativeRequestCodec.encodeOpen(restoreOptions, credentials, expanded.size, 0).use { request ->
                request.useForJni { bytes ->
                    try {
                        VcCore.nativeRestoreHeader(checkNotNull(target).fd, checkNotNull(backup).fd, bytes, keyfiles.map { it.fd }.toIntArray())
                    } catch (failure: VcCoreFailure) {
                        throw failure.asVolumeError()
                    }
                }
            }
        } finally {
            keyfiles.forEach(SeekableContainer::close)
            backup?.close()
            target?.close()
            credentials.close()
            options.hiddenVolumeProtection?.close()
        }
        }
    }

    private fun sameContainer(first: Uri, second: Uri): Boolean {
        if (org.eds.veracrypt.catalog.UriIdentity.same(first, second)) return true
        val source = runCatching { resolver.openFileDescriptor(first, "r") }.getOrNull() ?: return false
        val target = runCatching { resolver.openFileDescriptor(second, "r") }.getOrNull()
            ?: return source.use { false }
        return source.use { sourcePfd -> target.use { targetPfd ->
            runCatching {
                val sourceStat = Os.fstat(sourcePfd.fileDescriptor)
                val targetStat = Os.fstat(targetPfd.fileDescriptor)
                sourceStat.st_dev == targetStat.st_dev && sourceStat.st_ino == targetStat.st_ino
            }.getOrDefault(false)
        } }
    }
}
