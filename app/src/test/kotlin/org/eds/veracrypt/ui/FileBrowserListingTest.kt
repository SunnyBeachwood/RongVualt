package org.eds.veracrypt.ui

import android.provider.DocumentsContract
import org.junit.Assert.assertEquals
import org.junit.Test

class FileBrowserListingTest {
    private val entries = listOf(
        entry("folder-z", "Zoo", DocumentsContract.Document.MIME_TYPE_DIR, 0, 10),
        entry("hidden", ".private.txt", "text/plain", 1, 90),
        entry("alpha", "Alpha.txt", "text/plain", 50, 20),
        entry("movie", "movie.mp4", "video/mp4", 200, 30),
        entry("folder-a", "Archive", DocumentsContract.Document.MIME_TYPE_DIR, 0, 40),
    )

    @Test fun `hidden files are excluded by default and search is case insensitive`() {
        val result = FileBrowserListing.display(entries, "ALP", showHidden = false, FileBrowserSort())

        assertEquals(listOf("Alpha.txt"), result.map { it.name })
    }

    @Test fun `hidden files appear only when enabled`() {
        val result = FileBrowserListing.display(entries, "", showHidden = true, FileBrowserSort())

        assertEquals(listOf("Archive", "Zoo", ".private.txt", "Alpha.txt", "movie.mp4"), result.map { it.name })
    }

    @Test fun `directories remain first for descending size and modified sorts`() {
        val bySize = FileBrowserListing.display(entries, "", true, FileBrowserSort(FileBrowserSortKey.SIZE, true))
        val byModified = FileBrowserListing.display(entries, "", true, FileBrowserSort(FileBrowserSortKey.MODIFIED, true))

        assertEquals(listOf("Archive", "Zoo", "movie.mp4", "Alpha.txt", ".private.txt"), bySize.map { it.name })
        assertEquals(listOf("Archive", "Zoo", ".private.txt", "movie.mp4", "Alpha.txt"), byModified.map { it.name })
    }

    @Test fun `type sorting is deterministic`() {
        val result = FileBrowserListing.display(entries, "", true, FileBrowserSort(FileBrowserSortKey.TYPE, false))

        assertEquals(listOf("Archive", "Zoo", ".private.txt", "Alpha.txt", "movie.mp4"), result.map { it.name })
    }

    private fun entry(id: String, name: String, mime: String, size: Long, modified: Long) = EmbeddedFileEntry(
        documentId = id,
        name = name,
        mimeType = mime,
        size = size,
        lastModified = modified,
        flags = 0,
    )
}
