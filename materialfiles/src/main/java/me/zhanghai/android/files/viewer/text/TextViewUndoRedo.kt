/*
 * Copyright (c) 2026 RongVualt contributors
 *
 * This small, bounded edit history follows the public-domain TextView undo
 * approach used by Markor's upstream TextViewUndoRedo. The upstream class is
 * provided to the public domain without restrictions or warranty; this Kotlin
 * implementation is independently adapted for RongVualt.
 * Upstream commit: 8d657fd20fff71719d782a3bc375c6f982d09b19
 */

package me.zhanghai.android.files.viewer.text

import android.text.Editable
import android.text.Selection
import android.text.TextWatcher
import android.widget.EditText

internal class TextViewUndoRedo(
    private val editText: EditText,
    private val maxHistorySize: Int = 100,
    private val onStateChanged: () -> Unit = {}
) {
    private data class Edit(
        val start: Int,
        val before: String,
        val after: String,
        val selectionBefore: Int,
        val selectionAfter: Int
    )

    private data class Pending(
        val start: Int,
        val before: String,
        val selectionBefore: Int,
        var after: String = ""
    )

    private val history = ArrayList<Edit>()
    private var position = 0
    private var pending: Pending? = null
    private var applying = false

    private val watcher = object : TextWatcher {
        override fun beforeTextChanged(s: CharSequence, start: Int, count: Int, after: Int) {
            if (applying) return
            pending = Pending(
                start = start,
                before = s.subSequence(start, start + count).toString(),
                selectionBefore = editText.selectionStart.coerceAtLeast(0)
            )
        }

        override fun onTextChanged(s: CharSequence, start: Int, before: Int, count: Int) {
            pending?.after = s.subSequence(start, start + count).toString()
        }

        override fun afterTextChanged(s: Editable) {
            val change = pending ?: return
            pending = null
            if (applying) return
            val after = change.after
            if (change.before == after && change.before.isEmpty()) return
            record(
                Edit(
                    change.start,
                    change.before,
                    after,
                    change.selectionBefore,
                    editText.selectionStart.coerceAtLeast(0)
                )
            )
        }
    }

    init {
        editText.addTextChangedListener(watcher)
    }

    val canUndo: Boolean
        get() = position > 0

    val canRedo: Boolean
        get() = position < history.size

    fun setTextWithoutHistory(text: CharSequence?) {
        applying = true
        try {
            editText.setText(text)
            editText.setSelection(editText.length())
            clear()
        } finally {
            applying = false
        }
    }

    fun clear() {
        history.clear()
        position = 0
        onStateChanged()
    }

    fun undo() {
        if (!canUndo) return
        val edit = history[--position]
        apply(edit.start, edit.after.length, edit.before, edit.selectionBefore)
    }

    fun redo() {
        if (!canRedo) return
        val edit = history[position++]
        apply(edit.start, edit.before.length, edit.after, edit.selectionAfter)
    }

    private fun apply(start: Int, replacedLength: Int, replacement: String, selection: Int) {
        applying = true
        try {
            val end = (start + replacedLength).coerceIn(start, editText.length())
            editText.text.replace(start.coerceAtMost(editText.length()), end, replacement)
            Selection.setSelection(editText.text, selection.coerceIn(0, editText.length()))
        } finally {
            applying = false
            onStateChanged()
        }
    }

    private fun record(edit: Edit) {
        if (position < history.size) {
            history.subList(position, history.size).clear()
        }
        history += edit
        if (history.size > maxHistorySize) {
            history.removeAt(0)
        }
        position = history.size
        onStateChanged()
    }
}
