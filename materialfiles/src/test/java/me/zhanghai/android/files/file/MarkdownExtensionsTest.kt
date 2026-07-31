package me.zhanghai.android.files.file

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class MarkdownExtensionsTest {
    @Test
    fun recognizesMarkdownMimeTypeAndExtensionsCaseInsensitively() {
        assertTrue(isMarkdownFileName("notes.MD"))
        assertTrue(isMarkdownFileName("notes.markdown"))
        assertTrue(isMarkdownFileName("notes.MkD"))
        assertTrue(isMarkdownFileName("notes.txt", "text/markdown".asMimeType()))
        assertTrue(isMarkdownFileName("notes", "application/markdown".asMimeType()))
        assertTrue(isMarkdownFileName("notes", "application/x-markdown".asMimeType()))
    }

    @Test
    fun doesNotTreatOtherTextFilesAsMarkdown() {
        assertFalse(isMarkdownFileName("notes.txt", MimeType.TEXT_PLAIN))
        assertFalse(isMarkdownFileName("notes.md.txt", MimeType.TEXT_PLAIN))
    }
}
