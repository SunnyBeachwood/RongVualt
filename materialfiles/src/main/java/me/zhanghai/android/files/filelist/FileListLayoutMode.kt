/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filelist

/** The layout policy used by the native dual-pane file browser. */
enum class FileListLayoutMode {
    AUTO,
    SINGLE,
    DUAL;

    fun isDualPane(widthDp: Int): Boolean = when (this) {
        AUTO -> widthDp >= AUTO_DUAL_PANE_WIDTH_DP
        SINGLE -> false
        DUAL -> true
    }

    companion object {
        const val AUTO_DUAL_PANE_WIDTH_DP = 600
    }
}
