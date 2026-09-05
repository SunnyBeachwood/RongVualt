/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.settings

import android.content.Context
import android.os.Environment
import android.util.AttributeSet
import androidx.annotation.AttrRes
import androidx.annotation.StyleRes
import java8.nio.file.Path
import java8.nio.file.Paths
import me.zhanghai.android.files.util.valueCompat

/** Directory preference used as the initial location of the right file pane. */
class SecondaryDirectoryPreference : PathPreference {
    constructor(context: Context) : super(context)

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs)

    constructor(context: Context, attrs: AttributeSet?, @AttrRes defStyleAttr: Int) : super(
        context, attrs, defStyleAttr
    )

    constructor(
        context: Context,
        attrs: AttributeSet?,
        @AttrRes defStyleAttr: Int,
        @StyleRes defStyleRes: Int
    ) : super(context, attrs, defStyleAttr, defStyleRes)

    override var persistedPath: Path
        get() = Settings.FILE_LIST_SECONDARY_START_DIRECTORY.valueCompat
        set(value) {
            Settings.FILE_LIST_SECONDARY_START_DIRECTORY.putValue(value)
        }
}

