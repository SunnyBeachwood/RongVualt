package org.eds.veracrypt.documents

import android.content.Context
import android.provider.DocumentsContract
import me.zhanghai.android.files.coil.clearFilePreviewMemoryCache
import me.zhanghai.android.files.ftpserver.FtpServerService
import me.zhanghai.android.files.ftpserver.FtpShareRootStore
import me.zhanghai.android.files.navigation.RuntimeNavigationRoot
import me.zhanghai.android.files.navigation.RuntimeNavigationRoots
import me.zhanghai.android.files.provider.document.createDocumentTreeRootPath
import java.io.Closeable
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import kotlinx.coroutines.flow.collectLatest
import org.eds.veracrypt.session.UnlockedVolume
import org.eds.veracrypt.session.UnlockedVolumeManager
import org.eds.veracrypt.domain.VolumeUnlockStage

/**
 * Process-scoped bridge for DocumentsProvider. Process death drops both the
 * native sessions and IDs, which is equivalent to locking every volume.
 */
internal object UnlockedVolumeService {
    private val nodes = mutableMapOf<java.util.UUID, MutableMap<String, UnlockedDocumentNode>>()
    private val proxyFiles = mutableMapOf<java.util.UUID, MutableSet<Closeable>>()
    private val closingVolumes = mutableSetOf<java.util.UUID>()
    private var applicationContext: Context? = null
    private var foregroundObserverStarted = false
    private var foregroundOperationCount = 0
    private var nextForegroundOperationId = 1L
    private val unlockOperations = mutableMapOf<Long, VolumeUnlockStage>()
    private val foregroundScope = CoroutineScope(SupervisorJob() + Dispatchers.Default)
    val documentIds = OpaqueDocumentIdRegistry<UnlockedDocumentNode>()
    val volumes = UnlockedVolumeManager { volume ->
        // FTP providers may still hold native-backed file handles. Stop the
        // listener synchronously before the session is allowed to close, then
        // remove the process-only root so a stale picker selection cannot be
        // reused by a later service start.
        FtpServerService.stopIfSharing(volume.id.toString())
        FtpShareRootStore.invalidateRuntimeRoot(volume.id.toString())
        closeProxyFiles(volume.id)
        try {
            synchronized(nodes) { nodes.remove(volume.id) }
            documentIds.revokeVolume(volume.id)
            notifyRootsChanged()
        } finally {
            synchronized(proxyFiles) { closingVolumes.remove(volume.id) }
        }
    }

    fun bind(context: Context) {
        FileTransferManager.bind(context)
        val beginObservation = synchronized(this) {
            applicationContext = context.applicationContext
            if (foregroundObserverStarted) false else {
                foregroundObserverStarted = true
                true
            }
        }
        if (beginObservation) {
            foregroundScope.launch {
                volumes.volumes.collectLatest {
                    refreshForegroundService()
                    refreshNavigationRoots()
                }
            }
        }
        refreshForegroundService()
        refreshNavigationRoots()
        notifyRootsChanged()
    }

    /** Holds foreground importance for a user-initiated native volume operation. */
    fun beginLongRunningOperation() {
        synchronized(this) { foregroundOperationCount += 1 }
        refreshForegroundService()
    }

    fun endLongRunningOperation() {
        synchronized(this) {
            check(foregroundOperationCount > 0) { "No foreground operation is active" }
            foregroundOperationCount -= 1
        }
        refreshForegroundService()
    }

    /**
     * Keeps all heavyweight repository work foregrounded, including failures
     * and structured cancellation. Calls may be nested by related workflows.
     */
    suspend fun <T> withForegroundOperation(block: suspend () -> T): T {
        beginLongRunningOperation()
        return try {
            block()
        } finally {
            endLongRunningOperation()
        }
    }

    /** Registers an unlock separately so the foreground notification can show its safe aggregate state. */
    suspend fun <T> withUnlockOperation(block: suspend (UnlockOperation) -> T): T {
        val id = synchronized(this) {
            val next = nextForegroundOperationId++
            unlockOperations[next] = VolumeUnlockStage.PREPARING_INPUTS
            next
        }
        refreshForegroundService()
        val operation = UnlockOperation(id)
        return try {
            block(operation)
        } finally {
            synchronized(this) { unlockOperations.remove(id) }
            refreshForegroundService()
        }
    }

    internal fun requiresForegroundService(): Boolean = foregroundSnapshot().let {
        it.volumeCount > 0 || it.operationCount > 0
    }

    internal fun foregroundSnapshot(): ForegroundSnapshot = synchronized(this) {
        ForegroundSnapshot(
            volumeCount = volumes.volumes.value.size,
            operationCount = foregroundOperationCount + unlockOperations.size,
            unlockCount = unlockOperations.size,
            unlockStage = unlockOperations.values.singleOrNull(),
        )
    }

    internal fun updateUnlockOperation(id: Long, stage: VolumeUnlockStage) {
        val changed = synchronized(this) {
            if (unlockOperations[id] == null) false else {
                unlockOperations[id] = stage
                true
            }
        }
        if (changed) refreshForegroundService()
    }

    private fun refreshForegroundService() {
        val context = synchronized(this) { applicationContext } ?: return
        VolumeForegroundService.refresh(context, requiresForegroundService())
    }

    fun notifyRootsChanged() {
        val context = applicationContext ?: return
        context.contentResolver.notifyChange(
            DocumentsContract.buildRootsUri("${context.packageName}.unlocked"),
            null,
        )
    }

    /** Mirrors only live unlocked sessions into the embedded file manager.
     * The roots disappear with the in-memory session and are never bookmarks. */
    private fun refreshNavigationRoots() {
        val context = applicationContext ?: return
        val roots = volumes.volumes.value.mapNotNull { volume ->
            val treeUri = rootTreeUri(volume.session) ?: return@mapNotNull null
            RuntimeNavigationRoot(
                id = volume.id.toString(),
                treeUri = treeUri,
                path = treeUri.createDocumentTreeRootPath(),
                title = volume.displayName,
                subtitle = context.getString(
                    com.sovworks.eds.android.R.string.vc_navigation_volume_subtitle,
                    if (volume.session.volumeKind == org.eds.veracrypt.domain.VolumeKind.HIDDEN) {
                        context.getString(com.sovworks.eds.android.R.string.vc_volume_kind_hidden)
                    } else context.getString(com.sovworks.eds.android.R.string.vc_volume_kind_normal),
                    if (volume.session.isReadOnly) {
                        context.getString(com.sovworks.eds.android.R.string.rv_state_read_only)
                    } else context.getString(com.sovworks.eds.android.R.string.rv_state_unlocked),
                ),
                iconRes = com.sovworks.eds.android.R.drawable.ic_unlocked_container,
                isReadOnly = volume.session.isReadOnly,
                onAccess = volume.session::touch,
            )
        }
        RuntimeNavigationRoots.replace(roots)
    }

    /** Returns a DocumentsUI root URI for an application-owned live session. */
    fun rootUri(session: org.eds.veracrypt.domain.VolumeSession): android.net.Uri? {
        val context = applicationContext ?: return null
        val volume = volumes.find(session) ?: return null
        return DocumentsContract.buildRootUri("${context.packageName}.unlocked", volume.id.toString())
    }

    /** Returns a tree URI for the live root without exposing a plaintext path. */
    fun rootTreeUri(session: org.eds.veracrypt.domain.VolumeSession): android.net.Uri? {
        val context = applicationContext ?: return null
        val volume = volumes.find(session) ?: return null
        val documentId = documentIds.register(volume.id, node(volume, ""))
        return DocumentsContract.buildTreeDocumentUri("${context.packageName}.unlocked", documentId)
    }

    /** True only for a live tree issued by this process' unlocked provider. */
    fun isTreeUriOwnedByUnlockedVolume(context: Context, uri: android.net.Uri): Boolean {
        if (uri.authority != "${context.packageName}.unlocked") return false
        val token = runCatching { DocumentsContract.getTreeDocumentId(uri) }.getOrNull() ?: return false
        return runCatching { documentIds.resolve(token) }.getOrNull()?.let {
            volumes.find(it.volume.id) != null
        } == true
    }

    fun node(volume: UnlockedVolume, relativePath: String, entry: org.eds.veracrypt.nativecore.NativeFileEntry? = null): UnlockedDocumentNode =
        synchronized(nodes) {
            val byPath = nodes.getOrPut(volume.id) { mutableMapOf() }
            byPath.getOrPut(relativePath) { UnlockedDocumentNode(volume, relativePath, entry) }.also { it.entry = entry ?: it.entry }
        }

    /** Resolves a tree URI held by this process without exposing opaque IDs. */
    internal fun resolveTreeUri(uri: android.net.Uri, volumeId: java.util.UUID): UnlockedDocumentNode? {
        val token = runCatching { android.provider.DocumentsContract.getTreeDocumentId(uri) }.getOrNull()
            ?: return null
        return runCatching { documentIds.resolve(token) }.getOrNull()
            ?.takeIf { it.volume.id == volumeId }
    }

    /** Requests DocumentsUI to re-query a volume after a direct native write. */
    internal fun notifyVolumeChanged(volumeId: java.util.UUID) {
        if (volumes.find(volumeId) != null) notifyRootsChanged()
    }

    /**
     * A rename or deletion makes the old path-based node identity stale. Revoke
     * that node and all descendants rather than exposing a path-derived ID.
     */
    fun revokePathAndDescendants(volumeId: java.util.UUID, relativePath: String) {
        synchronized(nodes) {
            val byPath = nodes[volumeId] ?: return
            val prefix = if (relativePath.isEmpty()) "" else "$relativePath/"
            byPath.entries.removeIf { (path, node) ->
                val matches = path == relativePath || path.startsWith(prefix)
                if (matches) documentIds.revoke(node)
                matches
            }
            if (byPath.isEmpty()) nodes.remove(volumeId)
        }
    }

    /** Returns false once volume closure has begun, preventing a close race. */
    fun registerProxyFile(volumeId: java.util.UUID, file: Closeable): Boolean = synchronized(proxyFiles) {
        if (volumeId in closingVolumes || volumes.find(volumeId) == null) false
        else proxyFiles.getOrPut(volumeId) { mutableSetOf() }.add(file)
    }

    fun unregisterProxyFile(volumeId: java.util.UUID, file: Closeable) {
        synchronized(proxyFiles) {
            proxyFiles[volumeId]?.let { files ->
                files.remove(file)
                if (files.isEmpty()) proxyFiles.remove(volumeId)
            }
        }
    }

    private fun closeProxyFiles(volumeId: java.util.UUID) {
        val files = synchronized(proxyFiles) {
            closingVolumes.add(volumeId)
            proxyFiles.remove(volumeId)?.toList().orEmpty()
        }
        files.forEach { file -> runCatching { file.close() } }
        // The file manager uses memory-only image caching. A lock invalidates
        // every unlocked path, so also evict any preview retained in RAM.
        runCatching { clearFilePreviewMemoryCache() }
    }
}

internal class UnlockOperation internal constructor(private val id: Long) {
    fun update(stage: VolumeUnlockStage) {
        UnlockedVolumeService.updateUnlockOperation(id, stage)
    }
}

internal data class ForegroundSnapshot(
    val volumeCount: Int,
    val operationCount: Int,
    val unlockCount: Int = 0,
    val unlockStage: VolumeUnlockStage? = null,
)

/** Internal document identity; only [OpaqueDocumentIdRegistry] exposes its token. */
internal class UnlockedDocumentNode(
    val volume: UnlockedVolume,
    val relativePath: String,
    var entry: org.eds.veracrypt.nativecore.NativeFileEntry?,
)
