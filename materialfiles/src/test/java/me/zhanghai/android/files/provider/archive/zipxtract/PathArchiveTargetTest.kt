/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 */

package me.zhanghai.android.files.provider.archive.zipxtract

import java8.nio.file.Paths
import me.zhanghai.android.files.provider.TestFileSystemProvider
import org.junit.Assert.assertEquals
import org.junit.BeforeClass
import org.junit.Test

class PathArchiveTargetTest {
    companion object {
        @JvmStatic
        @BeforeClass
        fun installFileSystemProvider() {
            TestFileSystemProvider.install()
        }
    }

    @Test
    fun parentTraversalStopsAtStorageBoundary() {
        val destination = PathArchiveTarget(Paths.get("/storage/emulated/0/测试"))
        var target = destination.resolve("folder/file.txt")
        val parents = mutableListOf<String>()

        while (true) {
            target = target.parent() ?: break
            parents += target.displayName
        }

        assertEquals(listOf("folder", "测试", "0"), parents)
    }
}
