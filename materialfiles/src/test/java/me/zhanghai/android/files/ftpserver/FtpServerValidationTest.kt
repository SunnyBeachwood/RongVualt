/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package me.zhanghai.android.files.ftpserver

import java8.nio.file.Paths
import me.zhanghai.android.files.provider.TestFileSystemProvider
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

/** Source-level coverage for the validation that is exercised before startup. */
class FtpServerValidationTest {
    companion object {
        @JvmStatic
        @org.junit.BeforeClass
        fun installFileSystemProvider() {
            TestFileSystemProvider.install()
        }
    }

    @Test
    fun acceptsOnlyTheFullTcpPortRange() {
        assertTrue(FtpServerService.isValidPort(1))
        assertTrue(FtpServerService.isValidPort(2121))
        assertTrue(FtpServerService.isValidPort(65535))
        assertFalse(FtpServerService.isValidPort(0))
        assertFalse(FtpServerService.isValidPort(65536))
    }

    @Test
    fun normalizesTraversalBeforeApplyingTheHomeBoundary() {
        val home = Paths.get("/srv/share")
        assertTrue(FtpPathPolicy.isSafe(home, home.resolve("documents/report.txt").normalize()) { false })
        assertFalse(FtpPathPolicy.isSafe(home, home.resolve("../etc/passwd").normalize()) { false })
        assertFalse(FtpPathPolicy.isSafe(home, Paths.get("/srv/share-old/file.txt")) { false })
    }

    @Test
    fun rejectsAPathBelowASymbolicLink() {
        val root = Paths.get("/srv/share")
        val link = root.resolve("escape")
        assertFalse(FtpPathPolicy.isSafe(root, link.resolve("secret.txt")) { candidate ->
            candidate == link
        })
    }
}
