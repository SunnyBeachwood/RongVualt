/*
 * Copyright (c) 2026 RongVualt contributors
 *
 * Selection formatting semantics are adapted from Markor's MarkdownActionButtons:
 * Copyright 2017-2025 Gregor Santner <gsantner AT mailbox DOT org>
 * License: Apache License 2.0
 * https://www.apache.org/licenses/LICENSE-2.0
 * Upstream commit: 8d657fd20fff71719d782a3bc375c6f982d09b19
 */

package me.zhanghai.android.files.viewer.text

import android.text.Selection
import android.widget.EditText

internal enum class MarkdownFormatAction {
    HEADING,
    BOLD,
    ITALIC,
    STRIKE,
    QUOTE,
    INLINE_CODE,
    CODE_BLOCK,
    LINK,
    IMAGE,
    UNORDERED_LIST,
    ORDERED_LIST,
    TASK_LIST
}

internal object MarkdownFormatActions {
    fun apply(editText: EditText, action: MarkdownFormatAction) {
        val editable = editText.text
        when (action) {
            MarkdownFormatAction.HEADING -> prefixLines(editText, "# ")
            MarkdownFormatAction.QUOTE -> prefixLines(editText, "> ")
            MarkdownFormatAction.UNORDERED_LIST -> prefixLines(editText, "- ")
            MarkdownFormatAction.ORDERED_LIST -> prefixOrderedLines(editText)
            MarkdownFormatAction.TASK_LIST -> prefixLines(editText, "- [ ] ")
            MarkdownFormatAction.BOLD -> wrap(editText, "**", "**", "bold")
            MarkdownFormatAction.ITALIC -> wrap(editText, "*", "*", "italic")
            MarkdownFormatAction.STRIKE -> wrap(editText, "~~", "~~", "strikethrough")
            MarkdownFormatAction.INLINE_CODE -> wrap(editText, "`", "`", "code")
            MarkdownFormatAction.CODE_BLOCK -> wrap(editText, "```\n", "\n```", "code")
            MarkdownFormatAction.LINK -> wrap(editText, "[", "](url)", "text")
            MarkdownFormatAction.IMAGE -> wrap(editText, "![", "](path)", "alt")
        }
        if (editable.length == 0) Selection.setSelection(editable, 0)
    }

    private fun wrap(editText: EditText, prefix: String, suffix: String, placeholder: String) {
        val text = editText.text
        val start = editText.selectionStart.coerceAtLeast(0)
        val end = editText.selectionEnd.coerceAtLeast(start)
        val selected = text.subSequence(start, end).toString().ifEmpty { placeholder }
        val replacement = prefix + selected + suffix
        text.replace(start, end, replacement)
        val placeholderStart = start + prefix.length
        val placeholderEnd = placeholderStart + selected.length
        if (end == start) {
            Selection.setSelection(text, placeholderStart, placeholderEnd)
        } else {
            Selection.setSelection(text, start + replacement.length)
        }
    }

    private fun prefixLines(editText: EditText, prefix: String) {
        val text = editText.text
        val start = editText.selectionStart.coerceAtLeast(0)
        val end = editText.selectionEnd.coerceAtLeast(start)
        val lineStart = text.toString().lastIndexOf('\n', (start - 1).coerceAtLeast(0)).let {
            if (it < 0) 0 else it + 1
        }
        val selectionLineEnd = text.toString().indexOf('\n', end).let { if (it < 0) text.length else it }
        var cursor = lineStart
        var offset = 0
        while (cursor <= selectionLineEnd) {
            text.insert(cursor + offset, prefix)
            offset += prefix.length
            val next = text.toString().indexOf('\n', cursor + offset)
            if (next < 0 || next >= selectionLineEnd + offset) break
            cursor = next + 1 - offset
        }
        Selection.setSelection(text, (start + prefix.length).coerceAtMost(text.length))
    }

    private fun prefixOrderedLines(editText: EditText) {
        val text = editText.text
        val start = editText.selectionStart.coerceAtLeast(0)
        val end = editText.selectionEnd.coerceAtLeast(start)
        val lineStart = text.toString().lastIndexOf('\n', (start - 1).coerceAtLeast(0)).let {
            if (it < 0) 0 else it + 1
        }
        val selectionLineEnd = text.toString().indexOf('\n', end).let { if (it < 0) text.length else it }
        var line = 1
        var cursor = lineStart
        var offset = 0
        while (cursor <= selectionLineEnd) {
            val prefix = "$line. "
            text.insert(cursor + offset, prefix)
            offset += prefix.length
            line++
            val next = text.toString().indexOf('\n', cursor + offset)
            if (next < 0 || next >= selectionLineEnd + offset) break
            cursor = next + 1 - offset
        }
        Selection.setSelection(text, (start + 3).coerceAtMost(text.length))
    }
}
