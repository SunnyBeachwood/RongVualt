package me.zhanghai.android.files.viewer.text

import android.content.Context
import android.graphics.Canvas
import android.util.AttributeSet
import android.widget.TextView
import androidx.appcompat.widget.AppCompatTextView

/** Draws only the visible editor gutter, avoiding clipping of a document-sized TextView. */
class LineNumberView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
) : AppCompatTextView(context, attrs) {
    var editor: TextView? = null
        set(value) {
            field = value
            invalidate()
        }

    private var editorScrollY = 0

    fun setEditorScrollY(value: Int) {
        if (editorScrollY == value) return
        editorScrollY = value
        invalidate()
    }

    fun refresh() = invalidate()

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        val editor = editor ?: return
        val layout = editor.layout ?: return
        if (layout.lineCount == 0) return

        val firstVisualLine = layout.getLineForVertical(
            (editorScrollY - editor.totalPaddingTop).coerceAtLeast(0)
        )
        val lastVisualLine = layout.getLineForVertical(
            (editorScrollY + height - editor.totalPaddingTop).coerceAtLeast(0)
        ).coerceAtMost(layout.lineCount - 1)
        val content = editor.text
        var sourceLine = 1
        val firstStart = layout.getLineStart(firstVisualLine).coerceAtMost(content.length)
        for (index in 0 until firstStart) if (content[index] == '\n') sourceLine++

        paint.color = currentTextColor
        paint.textAlign = android.graphics.Paint.Align.RIGHT
        val x = width - paddingEnd.toFloat()
        for (visualLine in firstVisualLine..lastVisualLine) {
            if (visualLine > firstVisualLine) {
                val start = layout.getLineStart(visualLine)
                if (start > 0 && content.getOrNull(start - 1) == '\n') sourceLine++
            }
            val baseline = editor.totalPaddingTop + layout.getLineBaseline(visualLine) - editorScrollY
            canvas.drawText(sourceLine.toString(), x, baseline.toFloat(), paint)
        }
    }
}
