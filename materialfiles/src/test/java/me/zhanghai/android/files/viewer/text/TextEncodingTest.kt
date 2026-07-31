package me.zhanghai.android.files.viewer.text

import java.nio.charset.Charset
import java.nio.charset.StandardCharsets
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Test

class TextEncodingTest {
    @Test
    fun detectsUtf8AndWindowsChinese() {
        val utf8 = "Windows 中文文本".toByteArray(StandardCharsets.UTF_8)
        assertEquals(StandardCharsets.UTF_8, detectTextEncoding(utf8).charset)

        val gb18030 = Charset.forName("GB18030")
        val ansi = "Windows 中文文本".toByteArray(gb18030)
        assertEquals(gb18030, detectTextEncoding(ansi).charset)
        assertEquals("Windows 中文文本", decodeText(ansi, gb18030, detectTextEncoding(ansi)))
    }

    @Test
    fun stripsAndPreservesWindowsUnicodeBom() {
        val charset = StandardCharsets.UTF_16LE
        val bom = byteArrayOf(0xFF.toByte(), 0xFE.toByte())
        val bytes = bom + "中文".toByteArray(charset)
        val detected = detectTextEncoding(bytes)

        assertEquals(charset, detected.charset)
        assertEquals("中文", decodeText(bytes, charset, detected))
        assertArrayEquals(bytes, encodeText("中文", charset, detected))
    }
}
