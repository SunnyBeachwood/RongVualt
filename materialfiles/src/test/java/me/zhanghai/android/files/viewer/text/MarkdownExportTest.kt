package me.zhanghai.android.files.viewer.text

import org.junit.Assert.assertTrue
import org.junit.Test

class MarkdownExportTest {
    @Test
    fun toHtmlRendersMarkdownAndUtf8Metadata() {
        val html = MarkdownExport.toHtml("# 标题\n\n**粗体**")
        assertTrue(html.contains("charset=\"utf-8\""))
        assertTrue(html.contains("<h1>标题</h1>"))
        assertTrue(html.contains("<strong>粗体</strong>"))
    }
}
