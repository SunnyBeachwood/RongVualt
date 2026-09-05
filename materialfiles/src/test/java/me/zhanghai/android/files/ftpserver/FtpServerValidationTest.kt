/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package me.zhanghai.android.files.ftpserver

import java8.nio.file.Paths
import me.zhanghai.android.files.provider.common.createSymbolicLink
import me.zhanghai.android.files.provider.common.delete
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

/** Source-level coverage for the validation that is exercised before startup. */
class FtpServerValidationTest {
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
        assertTrue(FtpPathPolicy.isSafe(home, home.resolve("documents/report.txt").normalize()))
        assertFalse(FtpPathPolicy.isSafe(home, home.resolve("../etc/passwd").normalize()))
        assertFalse(FtpPathPolicy.isSafe(home, Paths.get("/srv/share-old/file.txt")))
    }

    @Test
    fun rejectsAPathBelowASymbolicLink() {
        val rootFile = java.io.File.createTempFile("rongvault-ftp-root", "").apply {
            delete()
            mkdirs()
        }
        val outsideFile = java.io.File.createTempFile("rongvault-ftp-outside", "")
        val root = Paths.get(rootFile.toURI())
        val outside = Paths.get(outsideFile.toURI())
        val link = root.resolve("escape")
        try {
            link.createSymbolicLink(outside)
            assertFalse(FtpPathPolicy.isSafe(root, link.resolve("secret.txt")))
        } finally {
            runCatching { link.delete() }
            rootFile.deleteRecursively()
            outsideFile.delete()
        }
    }
}
