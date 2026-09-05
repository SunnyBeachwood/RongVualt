/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.ftpserver

import android.content.Context
import android.util.AttributeSet
import androidx.annotation.AttrRes
import androidx.annotation.StyleRes
import androidx.lifecycle.Observer
import java8.nio.file.Path
import me.zhanghai.android.files.settings.PathPreference

class FtpServerHomeDirectoryPreference : PathPreference {
    private val rootObserver = Observer<FtpShareRootSnapshot> { snapshot ->
        if (path != snapshot.path) path = snapshot.path
    }

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
        get() = FtpShareRootStore.current().path
        set(value) {
            FtpShareRootStore.select(value)
        }

    override fun onAttached() {
        super.onAttached()
        FtpShareRootStore.selection.observeForever(rootObserver)
    }

    override fun onDetached() {
        FtpShareRootStore.selection.removeObserver(rootObserver)
        super.onDetached()
    }
}
