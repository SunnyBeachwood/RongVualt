/*
 * Copyright (c) 2026 RongVualt contributors
 *
 * The list continuation behavior is adapted from Markor's AutoTextFormatter:
 * Copyright 2018-2025 Gregor Santner <gsantner AT mailbox DOT org>
 * License: Apache License 2.0
 * https://www.apache.org/licenses/LICENSE-2.0
 * Upstream commit: 8d657fd20fff71719d782a3bc375c6f982d09b19
 */

package me.zhanghai.android.files.viewer.text

import android.text.InputFilter
import android.text.Spanned
import java.util.regex.Pattern

internal class MarkdownAutoFormatFilter : InputFilter {
    override fun filter(
        source: CharSequence,
        start: Int,
        end: Int,
        dest: Spanned,
        dstart: Int,
        dend: Int
    ): CharSequence? {
        if (start >= end || dstart <= 0 || !source.subSequence(start, end).contains("\n")) {
            return source
        }
        val lineStart = dest.toString().lastIndexOf('\n', dstart - 1).let { if (it < 0) 0 else it + 1 }
        val line = dest.subSequence(lineStart, dstart).toString()
        val match = LIST_PREFIX.matcher(line)
        if (!match.find() || line.substring(match.end()).trim().isEmpty()) return source

        val prefix = match.group(1).orEmpty()
        val marker = match.group(2).orEmpty()
        val normalizedMarker = marker.trimEnd()
        val continuation = if (normalizedMarker.firstOrNull()?.isDigit() == true) {
            val number = normalizedMarker.dropLastWhile { it == '.' || it == ')' }.toIntOrNull() ?: return source
            val delimiter = normalizedMarker.last()
            "$prefix${number + 1}$delimiter "
        } else {
            "$prefix$marker"
        }
        return source.toString() + continuation
    }

    companion object {
        private val LIST_PREFIX = Pattern.compile(
            "^(\\s*)([-*+] \\[[ xX]\\] |\\d+[.)] |[-*+] )"
        )
    }
}
