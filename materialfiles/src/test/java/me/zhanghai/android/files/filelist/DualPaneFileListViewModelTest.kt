package me.zhanghai.android.files.filelist

import java8.nio.file.Paths
import me.zhanghai.android.files.provider.TestFileSystemProvider
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

/** Pure state tests for the dual-pane shell; intentionally does not start Android UI. */
class DualPaneFileListViewModelTest {
    companion object {
        @JvmStatic
        @org.junit.BeforeClass
        fun installFileSystemProvider() {
            TestFileSystemProvider.install()
        }
    }

    @Test
    fun historyIsIndependentAndSupportsBackForward() {
        val state = DualPaneFileListViewModel()
        val leftRoot = Paths.get("/left")
        val leftChild = leftRoot.resolve("child")
        val rightRoot = Paths.get("/right")

        state.recordNavigation(PaneId.LEFT, leftRoot)
        state.recordNavigation(PaneId.LEFT, leftChild)
        state.recordNavigation(PaneId.RIGHT, rightRoot)

        assertTrue(state.canGoBack(PaneId.LEFT))
        assertTrue(state.canGoBack(PaneId.RIGHT))
        assertEquals(leftChild, state.goBack(PaneId.LEFT, Paths.get("/left/child/deep")))
        assertEquals(Paths.get("/left/child/deep"), state.goForward(PaneId.LEFT, leftChild))
        assertFalse(state.canGoForward(PaneId.RIGHT))
    }

    @Test
    fun navigatingClearsOnlyThatPanesForwardHistory() {
        val state = DualPaneFileListViewModel()
        val left = Paths.get("/left")
        val leftNext = left.resolve("next")
        val right = Paths.get("/right")
        val rightNext = right.resolve("next")

        state.recordNavigation(PaneId.LEFT, left)
        assertEquals(left, state.goBack(PaneId.LEFT, leftNext))
        state.recordNavigation(PaneId.RIGHT, right)
        state.goBack(PaneId.RIGHT, rightNext)

        assertTrue(state.canGoForward(PaneId.LEFT))
        assertTrue(state.canGoForward(PaneId.RIGHT))
        state.recordNavigation(PaneId.LEFT, leftNext)
        assertFalse(state.canGoForward(PaneId.LEFT))
        assertTrue(state.canGoForward(PaneId.RIGHT))
    }

    @Test
    fun layoutModeUsesDualForDefaultAndCanBeOverridden() {
        val state = DualPaneFileListViewModel()
        assertEquals(FileListLayoutMode.DUAL, state.layoutMode)
        assertTrue(state.layoutMode.isDualPane(360))
        assertFalse(FileListLayoutMode.AUTO.isDualPane(599))
        assertTrue(FileListLayoutMode.AUTO.isDualPane(600))
        state.layoutMode = FileListLayoutMode.SINGLE
        assertFalse(state.layoutMode.isDualPane(1200))
        assertEquals(FileListLayoutMode.SINGLE, state.layoutMode)
        state.layoutMode = FileListLayoutMode.DUAL
        assertTrue(state.layoutMode.isDualPane(360))
        assertEquals(FileListLayoutMode.DUAL, state.layoutMode)
    }
}
