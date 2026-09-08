package me.zhanghai.android.files.filelist

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Test

class RecentAccessStoreTest {
    @Test
    fun mergeMovesExistingUriToFrontWithoutDuplicatingIt() {
        val old = RecentAccessEntry("file:///old", "old", false, 1)
        val current = RecentAccessEntry("file:///current", "current", true, 2)
        val revisited = current.copy(accessedAt = 3)

        val entries = RecentAccessStore.merge(listOf(current, old), revisited)

        assertEquals(listOf(revisited, old), entries)
    }

    @Test
    fun mergeLimitsHistoryToTwentyEntries() {
        val existing = (0 until RecentAccessStore.MAX_ENTRIES).map { index ->
            RecentAccessEntry("file:///$index", "$index", false, index.toLong())
        }

        val entries = RecentAccessStore.merge(
            existing, RecentAccessEntry("file:///new", "new", false, 99)
        )

        assertEquals(RecentAccessStore.MAX_ENTRIES, entries.size)
        assertEquals("file:///new", entries.first().uri)
        assertFalse(entries.any { it.uri == "file:///19" })
    }
}
