/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filelist

enum class PaneId {
    LEFT,
    RIGHT;

    fun other(): PaneId = if (this == LEFT) RIGHT else LEFT
}

