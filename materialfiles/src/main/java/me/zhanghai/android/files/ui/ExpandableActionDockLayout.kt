package me.zhanghai.android.files.ui

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.ViewConfiguration
import android.widget.LinearLayout
import kotlin.math.abs

/** Intercepts deliberate vertical swipes without stealing ordinary dock button taps. */
class ExpandableActionDockLayout @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
) : LinearLayout(context, attrs) {
    var onExpandRequested: (() -> Unit)? = null
    var onCollapseRequested: (() -> Unit)? = null

    private val touchSlop = ViewConfiguration.get(context).scaledTouchSlop
    private var downX = 0f
    private var downY = 0f
    private var intercepting = false

    override fun onInterceptTouchEvent(event: MotionEvent): Boolean {
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                downX = event.x
                downY = event.y
                intercepting = false
            }
            MotionEvent.ACTION_MOVE -> {
                val deltaX = event.x - downX
                val deltaY = event.y - downY
                if (!intercepting && abs(deltaY) > touchSlop && abs(deltaY) > abs(deltaX)) {
                    intercepting = true
                    return true
                }
            }
        }
        return false
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        if (!intercepting) return super.onTouchEvent(event)
        if (event.actionMasked == MotionEvent.ACTION_UP) {
            val deltaY = event.y - downY
            if (deltaY < -touchSlop * 2) onExpandRequested?.invoke()
            else if (deltaY > touchSlop * 2) onCollapseRequested?.invoke()
            intercepting = false
        } else if (event.actionMasked == MotionEvent.ACTION_CANCEL) {
            intercepting = false
        }
        return true
    }
}
