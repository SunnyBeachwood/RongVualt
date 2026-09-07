/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.settings

import android.content.Context
import android.util.AttributeSet
import androidx.preference.DialogPreference
import me.zhanghai.android.files.R

/** Preference that stores the file-list name size directly as an sp integer. */
class FileListFontSizePreference : DialogPreference {
    constructor(context: Context) : super(context)
    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs)
    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(
        context, attrs, defStyleAttr
    )
    constructor(
        context: Context,
        attrs: AttributeSet?,
        defStyleAttr: Int,
        defStyleRes: Int
    ) : super(context, attrs, defStyleAttr, defStyleRes)

    var fontSizeSp: Int
        get() = getPersistedInt(FileListFontSizeDefaults.DEFAULT_SP)
        set(value) {
            val clamped = value.coerceIn(
                FileListFontSizeDefaults.MIN_SP, FileListFontSizeDefaults.MAX_SP
            )
            if (callChangeListener(clamped)) {
                persistInt(clamped)
                notifyChanged()
            }
        }

    init {
        dialogTitle = title
        summaryProvider = SummaryProvider<FileListFontSizePreference> {
            context.getString(R.string.settings_file_list_font_size_summary, fontSizeSp)
        }
    }

    override fun onGetDefaultValue(typedArray: android.content.res.TypedArray, index: Int): Any =
        typedArray.getInt(index, FileListFontSizeDefaults.DEFAULT_SP)

    override fun onSetInitialValue(defaultValue: Any?) {
        val defaultSp = (defaultValue as? Int) ?: FileListFontSizeDefaults.DEFAULT_SP
        if (shouldPersist()) {
            fontSizeSp = fontSizeSp
        } else {
            fontSizeSp = defaultSp
        }
    }

    object FileListFontSizeDefaults {
        const val MIN_SP = 12
        const val MAX_SP = 32
        const val DEFAULT_SP = 16
    }
}
