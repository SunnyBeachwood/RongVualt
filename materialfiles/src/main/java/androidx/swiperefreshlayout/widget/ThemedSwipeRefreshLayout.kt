/*
 * Copyright (c) 2018 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package androidx.swiperefreshlayout.widget

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.drawable.ShapeDrawable
import android.util.AttributeSet
import android.view.View
import androidx.core.graphics.ColorUtils
import com.google.android.material.R
import me.zhanghai.android.files.compat.getColorCompat
import me.zhanghai.android.files.util.getColorByAttr
import me.zhanghai.android.files.util.isMaterial3Theme

@Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
class ThemedSwipeRefreshLayout : SwipeRefreshLayout {
    constructor(context: Context) : super(context) {
        init()
    }

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs) {
        init()
    }

    private fun init() {
        val backgroundColor = if (context.isMaterial3Theme) {
            val surfaceColor = context.getColorByAttr(R.attr.colorSurface)
            @SuppressLint("PrivateResource")
            val overlayColor = context.getColorCompat(R.color.m3_popupmenu_overlay_color)
            ColorUtils.compositeColors(overlayColor, surfaceColor)
        } else {
            context.getColorByAttr(androidx.appcompat.R.attr.colorBackgroundFloating)
        }
        (mCircleView.background as ShapeDrawable).paint.color = backgroundColor
        setColorSchemeColors(context.getColorByAttr(androidx.appcompat.R.attr.colorAccent))
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)

        childView?.let { child ->
            measureChild(child, widthMeasureSpec, heightMeasureSpec)
            setMeasuredDimension(
                child.measuredWidth + paddingLeft + paddingRight,
                child.measuredHeight + paddingTop + paddingBottom
            )
        }
    }

    private val childView: View?
        get() = (0 until childCount)
            .asSequence()
            .map(::getChildAt)
            .firstOrNull { it != mCircleView }
}
