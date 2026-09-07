package me.zhanghai.android.files.ui

import android.content.Context
import android.text.TextUtils
import android.util.AttributeSet
import android.util.TypedValue
import android.widget.TextView
import androidx.appcompat.widget.Toolbar

/** Toolbar whose built-in title and subtitle scale to the space left by navigation and actions. */
class AdaptiveToolbar @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = androidx.appcompat.R.attr.toolbarStyle,
) : Toolbar(context, attrs, defStyleAttr) {
    fun refreshTextSizing() {
        post {
            findTextView(title)?.let { titleView ->
                titleView.maxLines = 1
                titleView.ellipsize = TextUtils.TruncateAt.MIDDLE
                titleView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 22f)
            }
            findTextView(subtitle)?.let { subtitleView ->
                // Toolbar's own layout does not reliably remeasure framework text auto-size
                // after a subtitle update. A compact fixed size plus two lines leaves room for
                // the complete storage value on narrow dual-pane screens.
                subtitleView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 10f)
                subtitleView.maxLines = 2
                subtitleView.ellipsize = null
                subtitleView.setHorizontallyScrolling(false)
            }
        }
    }

    private fun findTextView(text: CharSequence?): TextView? {
        if (text.isNullOrEmpty()) return null
        return (0 until childCount)
            .asSequence()
            .map { getChildAt(it) }
            .filterIsInstance<TextView>()
            .firstOrNull { it.text.toString() == text.toString() }
    }
}
