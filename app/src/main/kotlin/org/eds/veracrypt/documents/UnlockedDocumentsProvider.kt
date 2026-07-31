package org.eds.veracrypt.documents

import android.content.res.AssetFileDescriptor
import android.database.Cursor
import android.database.MatrixCursor
import android.graphics.Point
import android.os.CancellationSignal
import android.os.Handler
import android.os.HandlerThread
import android.os.ParcelFileDescriptor
import android.os.ProxyFileDescriptorCallback
import android.os.storage.StorageManager
import android.provider.DocumentsContract
import android.provider.DocumentsContract.Document
import android.provider.DocumentsContract.Root
import android.provider.DocumentsProvider
import android.system.ErrnoException
import android.system.OsConstants
import android.webkit.MimeTypeMap
import java.io.Closeable
import java.io.FileNotFoundException
import java.time.LocalDateTime
import java.time.ZoneId
import org.eds.veracrypt.nativecore.NativeFileEntry
import org.eds.veracrypt.nativecore.NativeFileSystemAccess
import org.eds.veracrypt.nativecore.NativeOpenFile
import org.eds.veracrypt.domain.VolumeError

/**
 * Android DocumentsProvider boundary for application-owned unlocked volumes.
 * It deliberately does not fall back to the legacy URI/path provider or make
 * plaintext temporary files. File operations are enabled only after the
 * native FAT/exFAT/NTFS bridge supplies directory and proxy-FD support.
 */
class UnlockedDocumentsProvider : DocumentsProvider() {
    override fun onCreate(): Boolean {
        UnlockedVolumeService.bind(checkNotNull(context))
        return true
    }

    override fun queryRoots(projection: Array<String>?): Cursor {
        val cursor = MatrixCursor(projection ?: DEFAULT_ROOT_PROJECTION)
        UnlockedVolumeService.volumes.volumes.value.forEach { volume ->
            val documentId = register(UnlockedVolumeService.node(volume, ""))
            cursor.newRow()
                .add(Root.COLUMN_ROOT_ID, volume.id.toString())
                .add(Root.COLUMN_TITLE, volume.displayName)
                .add(Root.COLUMN_DOCUMENT_ID, documentId)
                .add(Root.COLUMN_ICON, com.sovworks.eds.android.R.drawable.ic_unlocked_container)
                .add(
                    Root.COLUMN_FLAGS,
                    Root.FLAG_LOCAL_ONLY or Root.FLAG_SUPPORTS_IS_CHILD or
                        (if (volume.session.isReadOnly) 0 else Root.FLAG_SUPPORTS_CREATE),
                )
        }
        return cursor
    }

    override fun queryDocument(documentId: String, projection: Array<String>?): Cursor {
        val node = resolve(documentId)
        return MatrixCursor(projection ?: DEFAULT_DOCUMENT_PROJECTION).apply {
            addDocumentRow(this, documentId, node)
        }
    }

    override fun queryChildDocuments(
        parentDocumentId: String,
        projection: Array<String>?,
        sortOrder: String?,
    ): Cursor {
        val parent = resolve(parentDocumentId)
        requireDirectory(parent)
        val access = access(parent)
        val cursor = MatrixCursor(projection ?: DEFAULT_DOCUMENT_PROJECTION).apply {
            access.list(parent.relativePath).forEach { entry ->
                val child = UnlockedVolumeService.node(parent.volume, childPath(parent.relativePath, entry.name), entry)
                addDocumentRow(this, register(child), child)
            }
        }
        val providerContext = context
        if (providerContext != null) {
            cursor.setNotificationUris(
                providerContext.contentResolver,
                childNotificationUris(parent),
            )
        }
        return cursor
    }

    override fun getDocumentType(documentId: String): String {
        val node = resolve(documentId)
        return mimeType(node)
    }

    /**
     * DocumentsContract tree URIs are capability boundaries.  The framework
     * calls this method before allowing a client to query or open descendants
     * whose opaque document ID differs from the root ID.  Without it every
     * Material Files operation below the unlocked root is rejected by
     * DocumentsProvider.enforceTree(), even though direct document URIs work.
     */
    override fun isChildDocument(parentDocumentId: String, documentId: String): Boolean = try {
        val parent = resolve(parentDocumentId)
        val child = resolve(documentId)
        if (parent.volume.id != child.volume.id) {
            false
        } else {
            val parentPath = parent.relativePath.trimEnd('/')
            val childPath = child.relativePath.trimEnd('/')
            parentPath.isEmpty() && childPath.isNotEmpty() ||
                parentPath.isNotEmpty() && childPath.startsWith("$parentPath/")
        }
    } catch (_: FileNotFoundException) {
        false
    }

    override fun createDocument(parentDocumentId: String, mimeType: String, displayName: String): String {
        val parent = resolve(parentDocumentId)
        requireDirectory(parent)
        requireWritable(parent)
        val path = childPath(parent.relativePath, displayName)
        val access = access(parent)
        try {
            if (mimeType == Document.MIME_TYPE_DIR) {
                access.createDirectory(path)
            } else {
                access.openFile(path, writable = true, create = true).close()
            }
            val child = UnlockedVolumeService.node(parent.volume, path, access.stat(path))
            notifyChildrenChanged(parent)
            return register(child)
        } catch (error: Throwable) {
            throw FileNotFoundException("Unable to create unlocked document").apply { initCause(error) }
        }
    }

    override fun deleteDocument(documentId: String) {
        val node = resolve(documentId)
        rejectRoot(node)
        requireWritable(node)
        try {
            access(node).delete(node.relativePath)
            UnlockedVolumeService.revokePathAndDescendants(node.volume.id, node.relativePath)
            notifyChildrenChanged(node.volume, parentPath(node.relativePath))
        } catch (error: Throwable) {
            throw FileNotFoundException("Unable to delete unlocked document").apply { initCause(error) }
        }
    }

    override fun renameDocument(documentId: String, displayName: String): String {
        val node = resolve(documentId)
        rejectRoot(node)
        requireWritable(node)
        val parentPath = node.relativePath.substringBeforeLast('/', "")
        val destination = childPath(parentPath, displayName)
        try {
            val access = access(node)
            access.rename(node.relativePath, destination)
            UnlockedVolumeService.revokePathAndDescendants(node.volume.id, node.relativePath)
            val renamed = UnlockedVolumeService.node(node.volume, destination, access.stat(destination))
            notifyChildrenChanged(node.volume, parentPath(node.relativePath))
            return register(renamed)
        } catch (error: Throwable) {
            throw FileNotFoundException("Unable to rename unlocked document").apply { initCause(error) }
        }
    }

    /**
     * Keep moves inside one unlocked filesystem atomic. Cross-volume moves are
     * intentionally left to the caller's copy-and-delete workflow, so a lock
     * or failure cannot silently discard the only source copy.
     */
    override fun moveDocument(
        documentId: String,
        sourceParentDocumentId: String,
        targetParentDocumentId: String,
    ): String {
        val node = resolve(documentId)
        val sourceParent = resolve(sourceParentDocumentId)
        val targetParent = resolve(targetParentDocumentId)
        rejectRoot(node)
        requireDirectory(sourceParent)
        requireDirectory(targetParent)
        requireWritable(node)
        requireWritable(targetParent)
        if (node.volume.id != sourceParent.volume.id || node.volume.id != targetParent.volume.id) {
            throw FileNotFoundException("Moves between unlocked volumes are not supported")
        }
        if (parentPath(node.relativePath) != sourceParent.relativePath) {
            throw FileNotFoundException("Source parent does not own document")
        }
        val destination = childPath(targetParent.relativePath, node.entry?.name ?: node.relativePath.substringAfterLast('/'))
        try {
            val access = access(node)
            access.rename(node.relativePath, destination)
            UnlockedVolumeService.revokePathAndDescendants(node.volume.id, node.relativePath)
            val moved = UnlockedVolumeService.node(node.volume, destination, access.stat(destination))
            notifyChildrenChanged(sourceParent)
            if (sourceParent.relativePath != targetParent.relativePath) notifyChildrenChanged(targetParent)
            return register(moved)
        } catch (error: Throwable) {
            throw FileNotFoundException("Unable to move unlocked document").apply { initCause(error) }
        }
    }

    override fun openDocument(
        documentId: String,
        mode: String,
        signal: CancellationSignal?,
    ): ParcelFileDescriptor {
        val node = resolve(documentId)
        if (isDirectory(node)) throw FileNotFoundException("Cannot open a directory")
        val writable = mode.contains('w')
        val truncate = mode == "w" || mode.contains('t')
        if (writable && node.volume.session.isReadOnly) throw FileNotFoundException("Volume is read-only")
        val file = try {
            access(node).openFile(
                node.relativePath,
                writable,
                truncate = truncate,
            )
        } catch (error: Throwable) {
            throw FileNotFoundException("Unable to open unlocked file").apply { initCause(error) }
        }
        val nativeAccess = access(node)
        val initialSizeBytes = try {
            if (truncate) 0L else nativeAccess.stat(node.relativePath).sizeBytes
        } catch (error: Throwable) {
            file.close()
            throw FileNotFoundException("Unable to determine unlocked file size").apply { initCause(error) }
        }
        val callback = VolumeProxyCallback(
            file,
            nativeAccess,
            node.relativePath,
            node.volume.id,
            node.volume.session::touch,
            initialSizeBytes,
            if (writable) {
                { notifyChildrenChanged(node.volume, parentPath(node.relativePath)) }
            } else {
                null
            },
        )
        val flags = try {
            ParcelFileDescriptor.parseMode(mode)
        } catch (error: IllegalArgumentException) {
            file.close()
            throw FileNotFoundException("Unsupported document access mode").apply { initCause(error) }
        }
        return try {
            if (!UnlockedVolumeService.registerProxyFile(node.volume.id, callback)) {
                callback.close()
                throw FileNotFoundException("Unlocked volume is closing")
            }
            context?.getSystemService(StorageManager::class.java)
                ?.openProxyFileDescriptor(flags, callback, proxyCallbackHandler)
                ?: throw FileNotFoundException("Storage service is unavailable")
        } catch (error: Throwable) {
            callback.close()
            if (error is FileNotFoundException) throw error
            throw FileNotFoundException("Unable to create unlocked file descriptor").apply { initCause(error) }
        }
    }

    override fun openDocumentThumbnail(
        documentId: String,
        sizeHint: Point,
        signal: CancellationSignal?,
    ): AssetFileDescriptor {
        signal?.throwIfCanceled()
        val node = resolve(documentId)
        if (isDirectory(node) || !mimeType(node).startsWith("image/")) {
            throw FileNotFoundException("Document has no image thumbnail")
        }
        // Return the encrypted-provider proxy itself. Android/Coil decodes it
        // with the requested bounds, so no plaintext thumbnail is written to
        // disk or public cache.
        val descriptor = openDocument(documentId, "r", signal)
        val length = node.entry?.sizeBytes ?: AssetFileDescriptor.UNKNOWN_LENGTH
        return AssetFileDescriptor(descriptor, 0L, length)
    }

    private fun resolve(documentId: String) = try {
        UnlockedVolumeService.documentIds.resolve(documentId).also { it.volume.session.touch() }
    } catch (error: IllegalArgumentException) {
        throw FileNotFoundException("Unknown or expired document ID").apply { initCause(error) }
    }

    private fun register(node: UnlockedDocumentNode): String =
        UnlockedVolumeService.documentIds.register(node.volume.id, node)

    private fun access(node: UnlockedDocumentNode): NativeFileSystemAccess =
        node.volume.session as? NativeFileSystemAccess
            ?: throw FileNotFoundException("Unlocked filesystem is unavailable")

    private fun requireDirectory(node: UnlockedDocumentNode) {
        if (!isDirectory(node)) throw FileNotFoundException("Document is not a directory")
    }

    private fun requireWritable(node: UnlockedDocumentNode) {
        if (node.volume.session.isReadOnly) throw FileNotFoundException("Volume is read-only")
    }

    private fun rejectRoot(node: UnlockedDocumentNode) {
        if (node.relativePath.isEmpty()) throw FileNotFoundException("The unlocked-volume root cannot be changed")
    }

    private fun isDirectory(node: UnlockedDocumentNode): Boolean = node.relativePath.isEmpty() || node.entry?.isDirectory == true

    private fun childPath(parent: String, name: String): String {
        require(name.isNotEmpty() && name != "." && name != "..") { "Invalid document name" }
        require('/' !in name && '\\' !in name && '\u0000' !in name) { "Invalid document name" }
        return if (parent.isEmpty()) name else "$parent/$name"
    }

    private fun parentPath(relativePath: String): String = relativePath.substringBeforeLast('/', "")

    private fun notifyChildrenChanged(parent: UnlockedDocumentNode) {
        val providerContext = context ?: return
        childNotificationUris(parent).forEach { uri ->
            providerContext.contentResolver.notifyChange(uri, null)
        }
    }

    private fun notifyChildrenChanged(volume: org.eds.veracrypt.session.UnlockedVolume, relativePath: String) {
        notifyChildrenChanged(UnlockedVolumeService.node(volume, relativePath))
    }

    private fun childNotificationUris(parent: UnlockedDocumentNode): List<android.net.Uri> {
        val providerContext = context ?: return emptyList()
        val authority = "${providerContext.packageName}.unlocked"
        val parentId = register(parent)
        val rootId = register(UnlockedVolumeService.node(parent.volume, ""))
        val treeUri = DocumentsContract.buildTreeDocumentUri(authority, rootId)
        return listOf(
            DocumentsContract.buildChildDocumentsUri(authority, parentId),
            DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, parentId),
        ).distinct()
    }

    private fun mimeType(node: UnlockedDocumentNode): String {
        if (isDirectory(node)) return Document.MIME_TYPE_DIR
        return MimeTypeMap.getSingleton().getMimeTypeFromExtension(node.entry?.name?.substringAfterLast('.', "")?.lowercase())
            ?: "application/octet-stream"
    }

    private fun addDocumentRow(cursor: MatrixCursor, documentId: String, node: UnlockedDocumentNode) {
        // Document IDs outlive individual writes, so their cached entry can
        // contain the creation-time size/date. Refresh metadata whenever a
        // concrete file is queried; this also prevents newly written files
        // from appearing with an epoch timestamp.
        val entry = if (node.relativePath.isEmpty()) {
            node.entry
        } else {
            try {
                access(node).stat(node.relativePath)
            } catch (_: Throwable) {
                node.entry
            }
        }
        val directory = isDirectory(node)
        val writable = !node.volume.session.isReadOnly
        var flags = 0
        if (directory && writable) flags = flags or Document.FLAG_DIR_SUPPORTS_CREATE
        if (!node.relativePath.isEmpty() && writable) {
            flags = flags or Document.FLAG_SUPPORTS_DELETE or Document.FLAG_SUPPORTS_RENAME or Document.FLAG_SUPPORTS_MOVE
        }
        if (!directory && writable) {
            flags = flags or Document.FLAG_SUPPORTS_WRITE
        }
        if (!directory && mimeType(node).startsWith("image/")) {
            flags = flags or Document.FLAG_SUPPORTS_THUMBNAIL
        }
        cursor.newRow()
            .add(Document.COLUMN_DOCUMENT_ID, documentId)
            .add(Document.COLUMN_DISPLAY_NAME, if (node.relativePath.isEmpty()) node.volume.displayName else entry?.name)
            .add(Document.COLUMN_MIME_TYPE, mimeType(node))
            .add(Document.COLUMN_FLAGS, flags)
            .add(Document.COLUMN_SIZE, entry?.sizeBytes ?: 0L)
            .add(Document.COLUMN_LAST_MODIFIED, entry?.let(::fatTimestampMillis) ?: 0L)
    }

    private fun fatTimestampMillis(entry: NativeFileEntry): Long = try {
        val year = 1980 + ((entry.modifiedDate ushr 9) and 0x7F)
        val month = (entry.modifiedDate ushr 5) and 0x0F
        val day = entry.modifiedDate and 0x1F
        val hour = (entry.modifiedTime ushr 11) and 0x1F
        val minute = (entry.modifiedTime ushr 5) and 0x3F
        val second = (entry.modifiedTime and 0x1F) * 2
        LocalDateTime.of(year, month, day, hour, minute, second).atZone(ZoneId.systemDefault()).toInstant().toEpochMilli()
    } catch (_: RuntimeException) {
        0L
    }

    private class VolumeProxyCallback(
        private val file: NativeOpenFile,
        private val access: NativeFileSystemAccess,
        private val relativePath: String,
        private val volumeId: java.util.UUID,
        private val touchSession: () -> Unit,
        initialSizeBytes: Long,
        private val onCommitted: (() -> Unit)?,
    ) : ProxyFileDescriptorCallback(), Closeable {
        private var cachedSizeBytes = initialSizeBytes
        private var closed = false

        override fun onGetSize(): Long = try {
            touchSession()
            cachedSizeBytes
        } catch (error: Throwable) {
            throw errno("stat", error)
        }

        override fun onRead(offset: Long, size: Int, data: ByteArray): Int = try {
            touchSession()
            file.read(offset, data, length = size)
        } catch (error: Throwable) {
            throw errno("read", error)
        }

        override fun onWrite(offset: Long, size: Int, data: ByteArray): Int = try {
            touchSession()
            file.write(offset, data, length = size).also { written ->
                if (offset < 0L || written < 0 || written.toLong() > Long.MAX_VALUE - offset) {
                    throw IllegalArgumentException("Proxy write range overflow")
                }
                // The native write has completed successfully before the
                // cached length is advanced; failed writes never alter it.
                cachedSizeBytes = maxOf(cachedSizeBytes, offset + written)
            }
        } catch (error: Throwable) {
            if (error is VolumeError.HiddenVolumeRisk) UnlockedVolumeService.notifyRootsChanged()
            throw errno("write", error)
        }

        override fun onFsync() {
            try {
                touchSession()
                file.flush()
            } catch (error: Throwable) {
                throw errno("fsync", error)
            }
        }

        override fun onRelease() = close()

        @Synchronized
        override fun close() {
            if (closed) return
            closed = true
            try {
                // A proxy-FD client can close immediately after its last
                // write without issuing an explicit fsync. Commit FAT/exFAT
                // metadata before retiring the native handle.
                file.flush()
            } catch (_: Throwable) {
                // Closure must still release the proxy registration. A prior
                // callback has already delivered any write/flush error.
            } finally {
                file.close()
                UnlockedVolumeService.unregisterProxyFile(volumeId, this)
                onCommitted?.let { runCatching(it) }
            }
        }

        private fun errno(operation: String, cause: Throwable): ErrnoException =
            ErrnoException(operation, OsConstants.EIO, cause)
    }

    private companion object {
        private val proxyCallbackHandler = Handler(
            HandlerThread("VeraCryptProxyFd").apply { start() }.looper,
        )

        val DEFAULT_ROOT_PROJECTION = arrayOf(
            Root.COLUMN_ROOT_ID,
            Root.COLUMN_TITLE,
            Root.COLUMN_DOCUMENT_ID,
            Root.COLUMN_ICON,
            Root.COLUMN_FLAGS,
        )
        val DEFAULT_DOCUMENT_PROJECTION = arrayOf(
            Document.COLUMN_DOCUMENT_ID,
            Document.COLUMN_DISPLAY_NAME,
            Document.COLUMN_MIME_TYPE,
            Document.COLUMN_FLAGS,
            Document.COLUMN_SIZE,
            Document.COLUMN_LAST_MODIFIED,
        )
    }
}
