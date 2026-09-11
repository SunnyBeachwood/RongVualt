/*
 * Copyright (c) 2026 RongVualt contributors
 *
 * Markdown highlighting rules are adapted from Markor's MarkdownSyntaxHighlighter:
 * Copyright 2018-2025 Gregor Santner <gsantner AT mailbox DOT org>
 * License: Apache License 2.0
 * https://www.apache.org/licenses/LICENSE-2.0
 * Upstream commit: 8d657fd20fff71719d782a3bc375c6f982d09b19
 */

package me.zhanghai.android.files.viewer.text

import android.graphics.Color
import android.graphics.Typeface
import android.text.Editable
import android.text.Spannable
import android.text.TextWatcher
import android.text.style.BackgroundColorSpan
import android.text.style.ForegroundColorSpan
import android.text.style.StrikethroughSpan
import android.text.style.StyleSpan
import android.text.style.TypefaceSpan
import android.widget.EditText
import java.util.regex.Pattern
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import me.zhanghai.android.files.util.getColorByAttr

internal class MarkdownSyntaxHighlighter(private val editText: EditText) {
    private enum class Kind { COLOR, BOLD, ITALIC, STRIKE, CODE, CODE_BACKGROUND }

    private data class SpanSpec(val start: Int, val end: Int, val kind: Kind)

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main.immediate)
    private var refreshJob: Job? = null
    private val appliedSpans = mutableListOf<Any>()
    private val primaryColor = editText.context.getColorByAttr(androidx.appcompat.R.attr.colorPrimary)
    private val codeBackground = ColorUtils.withAlpha(
        // The embedded file manager and host can resolve different Material
        // resource tables after R8. AppCompat's colorPrimary is present in
        // both themes and avoids a runtime NoSuchFieldError here.
        primaryColor,
        0x28
    )

    private val watcher = object : TextWatcher {
        override fun beforeTextChanged(s: CharSequence, start: Int, count: Int, after: Int) = Unit
        override fun onTextChanged(s: CharSequence, start: Int, before: Int, count: Int) = Unit
        override fun afterTextChanged(s: Editable) = refresh()
    }

    init {
        editText.addTextChangedListener(watcher)
    }

    fun refresh() {
        val snapshot = editText.text.toString()
        refreshJob?.cancel()
        refreshJob = scope.launch {
            delay(180)
            val specs = withContext(Dispatchers.Default) { compute(snapshot) }
            if (snapshot == editText.text.toString()) apply(specs)
        }
    }

    fun dispose() {
        editText.removeTextChangedListener(watcher)
        refreshJob?.cancel()
        scope.cancel()
        clearAppliedSpans()
    }

    private fun apply(specs: List<SpanSpec>) {
        val text = editText.text
        clearAppliedSpans()
        for (spec in specs) {
            if (spec.start < 0 || spec.end <= spec.start || spec.end > text.length) continue
            val span = when (spec.kind) {
                Kind.COLOR -> ForegroundColorSpan(primaryColor)
                Kind.BOLD -> StyleSpan(Typeface.BOLD)
                Kind.ITALIC -> StyleSpan(Typeface.ITALIC)
                Kind.STRIKE -> StrikethroughSpan()
                Kind.CODE -> TypefaceSpan("monospace")
                Kind.CODE_BACKGROUND -> BackgroundColorSpan(codeBackground)
            }
            text.setSpan(span, spec.start, spec.end, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE)
            appliedSpans += span
        }
    }

    private fun clearAppliedSpans() {
        val text = editText.text
        appliedSpans.forEach { text.removeSpan(it) }
        appliedSpans.clear()
    }

    private fun compute(source: String): List<SpanSpec> {
        if (source.isEmpty()) return emptyList()
        val result = ArrayList<SpanSpec>()
        addMatches(result, source, HEADING, Kind.COLOR)
        addMatches(result, source, LINK, Kind.COLOR)
        addMatches(result, source, LIST, Kind.COLOR)
        addMatches(result, source, QUOTE, Kind.COLOR)
        addMatches(result, source, BOLD, Kind.BOLD)
        addMatches(result, source, ITALIC, Kind.ITALIC)
        addMatches(result, source, STRIKE, Kind.STRIKE)
        addMatches(result, source, INLINE_CODE, Kind.CODE_BACKGROUND)
        addMatches(result, source, INLINE_CODE, Kind.CODE)
        addMatches(result, source, FENCED_CODE, Kind.CODE_BACKGROUND)
        addMatches(result, source, FENCED_CODE, Kind.CODE)
        return result
    }

    private fun addMatches(
        destination: MutableList<SpanSpec>,
        source: String,
        pattern: Pattern,
        kind: Kind
    ) {
        val matcher = pattern.matcher(source)
        while (matcher.find()) destination += SpanSpec(matcher.start(), matcher.end(), kind)
    }

    private object ColorUtils {
        fun withAlpha(color: Int, alpha: Int): Int = Color.argb(alpha, Color.red(color), Color.green(color), Color.blue(color))
    }

    companion object {
        private val HEADING = Pattern.compile("(?m)^#{1,6}\\s+.+$")
        private val LINK = Pattern.compile("!?\\[[^]]*]\\([^)]*\\)")
        private val LIST = Pattern.compile("(?m)^\\s*(?:[-*+] |\\d+[.)] |[-*+] \\[[ xX]\\] )")
        private val QUOTE = Pattern.compile("(?m)^\\s*>.*$")
        private val BOLD = Pattern.compile("(?:\\*\\*|__)(?=\\S)(.+?\\S)(?:\\*\\*|__)")
        private val ITALIC = Pattern.compile("(?<![*_])(?:\\*|_)(?=\\S)(.+?\\S)(?:\\*|_)(?![*_])")
        private val STRIKE = Pattern.compile("~~(?=\\S)(.+?\\S)~~")
        private val INLINE_CODE = Pattern.compile("(?<!`)`[^`]+`(?!`)")
        private val FENCED_CODE = Pattern.compile("(?ms)^```.*?^```\\s*$")
    }
}
