/*
 * Copyright (c) 2026
 */

package me.zhanghai.android.files.file

import java.util.Locale
import java8.nio.file.Path

private val markdownExtensions = setOf("md", "markdown", "mkd")

val MimeType.isMarkdown: Boolean
    get() = value.equals("text/markdown", ignoreCase = true) ||
        value.equals("application/markdown", ignoreCase = true) ||
        value.equals("application/x-markdown", ignoreCase = true)

fun isMarkdownFile(path: Path, mimeType: MimeType? = null): Boolean {
    return isMarkdownFileName(path.fileName?.toString().orEmpty(), mimeType)
}

fun isMarkdownFileName(fileName: String, mimeType: MimeType? = null): Boolean {
    if (mimeType?.isMarkdown == true) {
        return true
    }
    val extension = fileName.substringAfterLast('.', "").lowercase(Locale.ROOT)
    return extension in markdownExtensions
}
