package org.eds.veracrypt.session

import java.util.UUID
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeError
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeSession
import org.eds.veracrypt.domain.VolumeSessionState
import org.eds.veracrypt.nativecore.VcCore
import org.eds.veracrypt.nativecore.VcCoreFailure
import org.eds.veracrypt.nativecore.NativeFileEntry
import org.eds.veracrypt.nativecore.NativeFileSystemAccess
import org.eds.veracrypt.nativecore.NativeOpenFile
import org.eds.veracrypt.nativecore.nativeFileOperation

/**
 * Internal hook used by [UnlockedVolumeManager] to revoke provider roots
 * before an automatic or external session close reaches native cleanup.
 */
internal interface BeforeCloseAwareSession {
    fun setBeforeCloseListener(listener: () -> Unit)
}

/**
 * Owns an opaque native handle and the resources that made it usable (the SAF
 * descriptor and the process-local writer lease).  No Android component owns
 * this directly; its repository owns a [CoroutineScope] and closes sessions
 * when its process is stopped.
 */
internal class ManagedVolumeSession(
    internal val nativeHandle: Long,
    override val accessMode: VolumeAccessMode,
    override val volumeKind: VolumeKind,
    private val scope: CoroutineScope,
    initialFileSystem: VolumeFileSystem? = null,
    override val canModifyContainer: Boolean = accessMode == VolumeAccessMode.READ_WRITE,
    private val onClose: () -> Unit,
) : VolumeSession, NativeFileSystemAccess, BeforeCloseAwareSession {
    override val id: UUID = UUID.randomUUID()

    private val mutableState = MutableStateFlow<VolumeSessionState>(VolumeSessionState.Open)
    override val state: StateFlow<VolumeSessionState> = mutableState
    override var fileSystem: VolumeFileSystem? = null
        private set
    private val lock = Any()
    private var closeJob: Job? = null
    private var autoLockMillis: Long? = null
    private var beforeCloseListener: (() -> Unit)? = null
    private var closed = false
    private var hiddenVolumeRiskTriggered = false
    private val dependents = mutableSetOf<ManagedVolumeSession>()

    override val isReadOnly: Boolean
        get() = synchronized(lock) {
            accessMode != VolumeAccessMode.READ_WRITE || fileSystem == VolumeFileSystem.NTFS || hiddenVolumeRiskTriggered
        }

    init {
        fileSystem = initialFileSystem
    }

    /**
     * A hidden child owns a separate native descriptor but depends on this
     * session's writer lease. Parent closure must therefore close it first.
     */
    internal fun registerDependent(dependent: ManagedVolumeSession) {
        val closeNow = synchronized(lock) {
            if (closed) true else {
                dependents += dependent
                false
            }
        }
        if (closeNow) dependent.close()
    }

    fun enableAutoLock(timeoutMillis: Long) {
        require(timeoutMillis > 0) { "Auto-lock timeout must be positive" }
        synchronized(lock) {
            check(!closed) { "Session is closed" }
            autoLockMillis = timeoutMillis
            scheduleAutoLockLocked()
        }
    }

    override fun touch() {
        synchronized(lock) {
            if (!closed && autoLockMillis != null) scheduleAutoLockLocked()
        }
    }

    override fun setBeforeCloseListener(listener: () -> Unit) {
        val invokeNow = synchronized(lock) {
            if (closed) true else {
                beforeCloseListener = listener
                false
            }
        }
        if (invokeNow) runCatching(listener)
    }

    override fun flush() {
        synchronized(lock) {
            check(!closed) { "Session is closed" }
        }
        try {
            VcCore.nativeFlush(nativeHandle)
        } catch (failure: VcCoreFailure) {
            throw failure.asVolumeError()
        }
    }

    override fun mountFileSystem(): VolumeFileSystem {
        synchronized(lock) {
            check(!closed) { "Session is closed" }
        }
        val mounted = try {
            when (VcCore.nativeMountFileSystem(nativeHandle)) {
                1 -> VolumeFileSystem.FAT
                2 -> VolumeFileSystem.EXFAT
                3 -> VolumeFileSystem.NTFS
                else -> throw IllegalStateException("Native mount returned an unknown filesystem type")
            }
        } catch (failure: VcCoreFailure) {
            throw failure.asVolumeError()
        }
        fileSystem = mounted
        return mounted
    }

    override fun list(relativePath: String): List<NativeFileEntry> = withMountedFileSystem {
        nativeFileOperation { VcCore.nativeListDirectory(nativeHandle, relativePath).asList() }
    }

    override fun stat(relativePath: String): NativeFileEntry = withMountedFileSystem {
        nativeFileOperation { VcCore.nativeStat(nativeHandle, relativePath) }
    }

    override fun openFile(
        relativePath: String,
        writable: Boolean,
        create: Boolean,
        truncate: Boolean,
    ): NativeOpenFile = withMountedFileSystem {
        require(!isReadOnly || (!writable && !create && !truncate)) {
            "Read-only volumes cannot create, truncate, or open writable files"
        }
        val handle = withRiskTracking {
            nativeFileOperation {
            VcCore.nativeOpenFile(nativeHandle, relativePath, writable, create, truncate)
            }
        }
        check(handle != 0L) { "Native open returned an invalid file handle" }
        NativeOpenFile(handle, ::markHiddenVolumeRisk)
    }

    override fun createDirectory(relativePath: String) = withMountedFileSystem {
        require(!isReadOnly) { "Read-only volumes cannot create directories" }
        withRiskTracking { nativeFileOperation { VcCore.nativeCreateDirectory(nativeHandle, relativePath) } }
    }

    override fun delete(relativePath: String) = withMountedFileSystem {
        require(!isReadOnly) { "Read-only volumes cannot delete entries" }
        withRiskTracking { nativeFileOperation { VcCore.nativeDelete(nativeHandle, relativePath) } }
    }

    override fun rename(fromRelativePath: String, toRelativePath: String) = withMountedFileSystem {
        require(!isReadOnly) { "Read-only volumes cannot rename entries" }
        withRiskTracking { nativeFileOperation { VcCore.nativeRename(nativeHandle, fromRelativePath, toRelativePath) } }
    }

    override fun close() {
        val beforeClose: (() -> Unit)?
        val closingDependents = synchronized(lock) {
            if (closed) return
            closed = true
            closeJob?.cancel()
            mutableState.value = VolumeSessionState.Closing
            beforeClose = beforeCloseListener
            beforeCloseListener = null
            val result = dependents.toList()
            dependents.clear()
            result
        }
        // Revoke DocumentsProvider and FTP roots before any native descriptor
        // or proxy file is closed. A listener failure must not strand native
        // resources, so continue with normal cleanup in all cases.
        runCatching { beforeClose?.invoke() }
        // Let a child invoke its normal native close path. Its attempt to
        // close this parent is a no-op because the parent is already closing.
        closingDependents.forEach(ManagedVolumeSession::close)
        try {
            onClose()
            mutableState.value = VolumeSessionState.Closed
        } catch (error: VolumeError) {
            mutableState.value = VolumeSessionState.Failed(error)
        } catch (error: Throwable) {
            mutableState.value = VolumeSessionState.Failed(VolumeError.IoInterrupted(error))
        }
    }

    private fun scheduleAutoLockLocked() {
        closeJob?.cancel()
        val timeout = checkNotNull(autoLockMillis)
        closeJob = scope.launch {
            delay(timeout)
            close()
        }
    }

    /** Native protection rejected a write; future provider mutations are read-only. */
    private fun markHiddenVolumeRisk() = synchronized(lock) {
        hiddenVolumeRiskTriggered = true
        mutableState.value = VolumeSessionState.ProtectionTriggered
    }

    private inline fun <T> withRiskTracking(block: () -> T): T = try {
        block()
    } catch (error: VolumeError.HiddenVolumeRisk) {
        markHiddenVolumeRisk()
        throw error
    }

    private inline fun <T> withMountedFileSystem(block: () -> T): T {
        synchronized(lock) {
            check(!closed) { "Session is closed" }
            check(fileSystem != null) { "Filesystem is not mounted" }
        }
        return block()
    }
}
