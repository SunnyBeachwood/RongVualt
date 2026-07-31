package org.eds.veracrypt.documents.test

import android.database.Cursor
import android.database.MatrixCursor
import android.os.CancellationSignal
import android.os.ParcelFileDescriptor
import android.provider.DocumentsContract.Document
import android.provider.DocumentsContract.Root
import android.provider.DocumentsProvider
import java.io.File
import java.io.FileNotFoundException

/** Debug-only SAF sink for exercising the application-owned export path. */
class TestTransferDocumentsProvider : DocumentsProvider() {
    private val rootId = "root"
    private val slowSourceId = "slow-source"
    private val hiddenRiskSourceId = "hidden-risk-source"
    private val failingSourceId = "failing-source"
    private val slowSourceSize = 8L * 1024 * 1024
    private val hiddenRiskSourceSize = 20L * 1024 * 1024

    private fun outputDirectory(): File = File(checkNotNull(context).filesDir, "transfer-target").apply { mkdirs() }

    override fun onCreate(): Boolean {
        outputDirectory()
        return true
    }

    override fun queryRoots(projection: Array<String>?): Cursor = MatrixCursor(ROOT_PROJECTION).apply {
        newRow().add(Root.COLUMN_ROOT_ID, rootId).add(Root.COLUMN_DOCUMENT_ID, rootId)
            .add(Root.COLUMN_TITLE, "Transfer test target")
            .add(Root.COLUMN_FLAGS, Root.FLAG_LOCAL_ONLY or Root.FLAG_SUPPORTS_CREATE)
            .add(Root.COLUMN_MIME_TYPES, "*/*")
    }

    override fun queryDocument(documentId: String, projection: Array<String>?): Cursor = MatrixCursor(DOCUMENT_PROJECTION).apply {
        addRow(documentRow(documentId))
    }

    override fun queryChildDocuments(parentDocumentId: String, projection: Array<String>?, sortOrder: String?): Cursor {
        require(parentDocumentId == rootId)
        return MatrixCursor(DOCUMENT_PROJECTION).apply {
            addRow(documentRow(slowSourceId))
            addRow(documentRow(hiddenRiskSourceId))
            addRow(documentRow(failingSourceId))
            outputDirectory().listFiles()?.forEach { addRow(documentRow(it.name)) }
        }
    }

    override fun getDocumentType(documentId: String): String =
        if (documentId == rootId) Document.MIME_TYPE_DIR else "application/octet-stream"

    override fun createDocument(parentDocumentId: String, mimeType: String, displayName: String): String {
        if (parentDocumentId != rootId) throw FileNotFoundException("Only the debug root is writable")
        val file = File(outputDirectory(), displayName)
        if (file.exists()) file.delete()
        if (!file.createNewFile()) throw FileNotFoundException("Unable to create debug target")
        return displayName
    }

    override fun openDocument(documentId: String, mode: String, signal: CancellationSignal?): ParcelFileDescriptor {
        if (documentId == rootId) throw FileNotFoundException("Cannot open debug root")
        if (documentId == failingSourceId) {
            throw FileNotFoundException("Intentional test source failure")
        }
        if (documentId == slowSourceId || documentId == hiddenRiskSourceId) {
            if (mode.contains('w')) throw FileNotFoundException("Slow source is read-only")
            val sourceSize = if (documentId == hiddenRiskSourceId) hiddenRiskSourceSize else slowSourceSize
            val pipe = ParcelFileDescriptor.createPipe()
            Thread {
                runCatching {
                    ParcelFileDescriptor.AutoCloseOutputStream(pipe[1]).use { output ->
                        val chunk = ByteArray(64 * 1024) { index -> (index * 13 + 11).toByte() }
                        repeat((sourceSize / chunk.size).toInt()) {
                            output.write(chunk)
                            output.flush()
                            Thread.sleep(10L)
                        }
                    }
                }.onFailure { runCatching { pipe[1].close() } }
            }.apply { isDaemon = true; start() }
            return pipe[0]
        }
        return ParcelFileDescriptor.open(File(outputDirectory(), documentId), ParcelFileDescriptor.parseMode(mode))
    }

    private fun documentRow(documentId: String): Array<Any?> {
        val directory = documentId == rootId
        val syntheticSource = documentId == slowSourceId || documentId == hiddenRiskSourceId || documentId == failingSourceId
        val file = if (directory || syntheticSource) null else File(outputDirectory(), documentId)
        return arrayOf(documentId, documentId, if (directory) Document.MIME_TYPE_DIR else "application/octet-stream",
            if (directory || syntheticSource) 0 else Document.FLAG_SUPPORTS_WRITE,
            if (documentId == slowSourceId) slowSourceSize
            else if (documentId == hiddenRiskSourceId) hiddenRiskSourceSize
            else if (documentId == failingSourceId) 1024L
            else file?.length() ?: 0L)
    }

    companion object {
        private val ROOT_PROJECTION = arrayOf(Root.COLUMN_ROOT_ID, Root.COLUMN_DOCUMENT_ID, Root.COLUMN_TITLE, Root.COLUMN_FLAGS, Root.COLUMN_MIME_TYPES)
        private val DOCUMENT_PROJECTION = arrayOf(Document.COLUMN_DOCUMENT_ID, Document.COLUMN_DISPLAY_NAME, Document.COLUMN_MIME_TYPE, Document.COLUMN_FLAGS, Document.COLUMN_SIZE)
    }
}
