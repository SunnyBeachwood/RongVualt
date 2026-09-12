package me.zhanghai.android.files.filejob

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class FileJobProgressTest {
    @Test
    fun percentPrefersBytesAndClamps() {
        val progress = FileJobProgress(1, FileJobOperation.COPY, "Copy", 150, 100, 1, 2)
        assertEquals(100, progress.percent)
    }

    @Test
    fun percentFallsBackToEntries() {
        val progress = FileJobProgress(1, FileJobOperation.MOVE, "Move", 0, 0, 2, 4)
        assertEquals(50, progress.percent)
    }

    @Test
    fun unknownTotalsAreIndeterminate() {
        val progress = FileJobProgress(1, FileJobOperation.ARCHIVE, "Archive", 0, 0, 0, 0)
        assertNull(progress.percent)
        assertEquals(true, progress.indeterminate)
    }
}
