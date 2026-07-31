package org.eds.veracrypt.ui

import android.provider.DocumentsContract
import java.util.Locale

/** Pure list state for the provider-only file browser. */
internal enum class FileBrowserSortKey {
    NAME,
    MODIFIED,
    SIZE,
    TYPE,
}

internal data class FileBrowserSort(
    val key: FileBrowserSortKey = FileBrowserSortKey.NAME,
    val descending: Boolean = false,
)

internal data class EmbeddedFileEntry(
    val documentId: String,
    val name: String,
    val mimeType: String,
    val size: Long,
    val lastModified: Long,
    val flags: Int,
) {
    val isDirectory: Boolean get() = mimeType == DocumentsContract.Document.MIME_TYPE_DIR
    val isHidden: Boolean get() = name.startsWith('.')
}

internal object FileBrowserListing {
    fun display(
        entries: List<EmbeddedFileEntry>,
        query: String,
        showHidden: Boolean,
        sort: FileBrowserSort,
    ): List<EmbeddedFileEntry> {
        val normalizedQuery = query.trim().lowercase(Locale.ROOT)
        return entries.asSequence()
            .filter { showHidden || !it.isHidden }
            .filter { normalizedQuery.isEmpty() || it.name.lowercase(Locale.ROOT).contains(normalizedQuery) }
            .sortedWith(comparator(sort))
            .toList()
    }

    private fun comparator(sort: FileBrowserSort): Comparator<EmbeddedFileEntry> {
        val valueComparator = when (sort.key) {
            FileBrowserSortKey.NAME -> compareBy(String.CASE_INSENSITIVE_ORDER) { it.name }
            FileBrowserSortKey.MODIFIED -> compareBy<EmbeddedFileEntry> { it.lastModified }
            FileBrowserSortKey.SIZE -> compareBy<EmbeddedFileEntry> { it.size }
            FileBrowserSortKey.TYPE -> compareBy(String.CASE_INSENSITIVE_ORDER) { it.mimeType }
        }.let { if (sort.descending) it.reversed() else it }
        return compareBy<EmbeddedFileEntry> { if (it.isDirectory) 0 else 1 }
            .then(valueComparator)
            .thenBy(String.CASE_INSENSITIVE_ORDER) { it.name }
    }
}
