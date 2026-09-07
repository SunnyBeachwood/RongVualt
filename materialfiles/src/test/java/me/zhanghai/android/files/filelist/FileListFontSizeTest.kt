package me.zhanghai.android.files.filelist

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class FileListFontSizeTest {
    @Test
    fun clampsToSupportedRange() {
        assertEquals(12f, FileListFontSize.fromSp(1).nameSp)
        assertEquals(32f, FileListFontSize.fromSp(99).nameSp)
    }

    @Test
    fun defaultSizeKeepsComfortableDensity() {
        val size = FileListFontSize.fromSp(FileListFontSize.DEFAULT_SP)
        assertEquals(16f, size.nameSp)
        assertEquals(12f, size.metadataSp)
        assertEquals(56, size.rowHeightDp)
        assertEquals(40, size.iconSlotDp)
    }

    @Test
    fun minimumSizePreservesTouchTarget() {
        val size = FileListFontSize.fromSp(FileListFontSize.MIN_SP)
        assertTrue(size.rowHeightDp >= 48)
        assertTrue(size.iconSlotDp >= 36)
    }

    @Test
    fun maximumSizeScalesRowsAndIcons() {
        val size = FileListFontSize.fromSp(FileListFontSize.MAX_SP)
        assertEquals(32f, size.nameSp)
        assertTrue(size.metadataSp > 12f)
        assertTrue(size.rowHeightDp > 56)
        assertTrue(size.iconSlotDp > 40)
    }
}
