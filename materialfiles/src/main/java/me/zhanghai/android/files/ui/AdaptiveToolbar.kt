package me.zhanghai.android.files.ui

import android.content.Context
import android.util.AttributeSet
import android.util.TypedValue
import android.view.Gravity
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.widget.Toolbar
import androidx.core.widget.TextViewCompat
import me.zhanghai.android.files.filelist.BreadcrumbData
import me.zhanghai.android.files.filelist.BreadcrumbLayout
import me.zhanghai.android.files.util.dpToDimensionPixelSize
import me.zhanghai.android.files.util.getColorByAttr

/** A toolbar header that keeps the path scrollable and the storage summary fully visible. */
class AdaptiveToolbar @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = androidx.appcompat.R.attr.toolbarStyle,
) : Toolbar(context, attrs, defStyleAttr) {
    private val breadcrumbLayout = BreadcrumbLayout(context).apply {
        layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, context.dpToDimensionPixelSize(40)
        )
    }
    private val summaryText = TextView(context).apply {
        layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT
        )
        setTextColor(context.getColorByAttr(android.R.attr.textColorSecondary))
        setTextSize(TypedValue.COMPLEX_UNIT_SP, 10f)
        maxLines = 2
        setHorizontallyScrolling(false)
        ellipsize = null
        TextViewCompat.setAutoSizeTextTypeUniformWithConfiguration(this, 8, 12, 1, TypedValue.COMPLEX_UNIT_SP)
    }
    private val headerContent = LinearLayout(context).apply {
        orientation = LinearLayout.VERTICAL
        gravity = Gravity.CENTER_VERTICAL
        addView(breadcrumbLayout)
        addView(summaryText)
    }

    init {
        super.setTitle(null)
        super.setSubtitle(null)
        addView(
            headerContent,
            LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT).apply {
                gravity = Gravity.CENTER_VERTICAL
            }
        )
    }

    fun setBreadcrumbData(data: BreadcrumbData) = breadcrumbLayout.setData(data)

    fun setBreadcrumbListener(listener: BreadcrumbLayout.Listener) = breadcrumbLayout.setListener(listener)

    fun setStorageSummary(summary: CharSequence) {
        summaryText.text = summary
    }

    /** Keep AppCompat's ActionBar title from competing with the custom header. */
    fun clearBuiltInText() {
        super.setTitle(null)
        super.setSubtitle(null)
    }
}
