package me.zhanghai.android.files.filelist

import org.junit.Assert.assertEquals
import org.junit.Test

class FileListSelectionTest {
    @Test
    fun rangeIncludesBothEndpointsInDisplayedOrder() {
        val files = listOf("download", "pictures", "music", "documents")

        assertEquals(
            listOf("pictures", "music", "documents"),
            FileListSelection.range(files, "pictures", "documents") { true },
        )
        assertEquals(
            listOf("pictures", "music", "documents"),
            FileListSelection.range(files, "documents", "pictures") { true },
        )
    }

    @Test
    fun rangeSkipsItemsThatCannotBeSelected() {
        val files = listOf("a", "parent", "b")

        assertEquals(
            listOf("a", "b"),
            FileListSelection.range(files, "a", "b") { it != "parent" },
        )
    }

    @Test
    fun rangeIsEmptyWhenTheAnchorWasRemovedByARefresh() {
        assertEquals(
            emptyList<String>(),
            FileListSelection.range(listOf("a", "b"), "missing", "b") { true },
        )
    }

    @Test
    fun inversionUsesOnlyVisibleSelectableItems() {
        val files = listOf("a", "hidden", "b", "blocked")

        assertEquals(
            listOf("b"),
            FileListSelection.inverted(files, setOf("a")) { it != "blocked" && it != "hidden" },
        )
    }
}
