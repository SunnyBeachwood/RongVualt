/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filelist

import androidx.lifecycle.ViewModel
import java8.nio.file.Path
import java.util.ArrayDeque

/** Session state shared by the two file-pane controllers. */
class DualPaneFileListViewModel : ViewModel() {
    var activePane: PaneId = PaneId.LEFT
    var isRecentAccessExpanded: Boolean = false

    // Normal file-manager sessions start as two panes. The Fragment restores the
    // application preference immediately after construction.
    var layoutMode: FileListLayoutMode = FileListLayoutMode.DUAL

    private val backStacks = PaneId.entries.associateWith { ArrayDeque<Path>() }
    private val forwardStacks = PaneId.entries.associateWith { ArrayDeque<Path>() }

    fun recordNavigation(pane: PaneId, current: Path) {
        val back = backStacks.getValue(pane)
        if (back.peekLast() != current) {
            back.addLast(current)
        }
        forwardStacks.getValue(pane).clear()
    }

    fun canGoBack(pane: PaneId): Boolean = backStacks.getValue(pane).isNotEmpty()

    fun canGoForward(pane: PaneId): Boolean = forwardStacks.getValue(pane).isNotEmpty()

    fun goBack(pane: PaneId, current: Path): Path? {
        val target = backStacks.getValue(pane).pollLast() ?: return null
        forwardStacks.getValue(pane).addLast(current)
        return target
    }

    fun goForward(pane: PaneId, current: Path): Path? {
        val target = forwardStacks.getValue(pane).pollLast() ?: return null
        backStacks.getValue(pane).addLast(current)
        return target
    }

    fun clearHistory(pane: PaneId) {
        backStacks.getValue(pane).clear()
        forwardStacks.getValue(pane).clear()
    }
}
