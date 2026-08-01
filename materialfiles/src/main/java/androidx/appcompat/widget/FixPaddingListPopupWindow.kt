/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package androidx.appcompat.widget

import android.content.Context
import android.util.AttributeSet
import androidx.annotation.AttrRes
import androidx.annotation.StyleRes

@Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
class FixPaddingListPopupWindow : ListPopupWindow {
    constructor(context: Context) : super(context)

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs)

    constructor(context: Context, attrs: AttributeSet?, @AttrRes defStyleAttr: Int) :
        super(context, attrs, defStyleAttr)

    constructor(
        context: Context,
        attrs: AttributeSet?,
        @AttrRes defStyleAttr: Int,
        @StyleRes defStyleRes: Int
    ) : super(context, attrs, defStyleAttr, defStyleRes)

    @Suppress("EXPOSED_FUNCTION_RETURN_TYPE")
    override fun createDropDownListView(context: Context, hijackFocus: Boolean): DropDownListView =
        FixPaddingDropDownListView(context, hijackFocus)

    private class FixPaddingDropDownListView(context: Context, hijackFocus: Boolean) :
        DropDownListView(context, hijackFocus) {

        // DropDownListView.measureHeightOfChildrenCompat() uses list padding instead of regular
        // padding, which isn't initialized before onMeasure() so returns no padding for the first
        // time. And ListPopupWindow.buildDropDown() adds the regular padding back every time, which
        // will double the padding after the first show.
        override fun measureHeightOfChildrenCompat(
            widthMeasureSpec: Int,
            startPosition: Int,
            endPosition: Int,
            maxHeight: Int,
            disallowPartialChildPosition: Int
        ): Int {
            return super.measureHeightOfChildrenCompat(
                widthMeasureSpec,
                startPosition,
                endPosition,
                maxHeight,
                disallowPartialChildPosition
            ) - listPaddingTop - listPaddingBottom
        }
    }
}
