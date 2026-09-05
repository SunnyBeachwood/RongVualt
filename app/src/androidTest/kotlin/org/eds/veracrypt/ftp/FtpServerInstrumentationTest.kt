/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package org.eds.veracrypt.ftp

import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.io.File
import java.net.ServerSocket
import java8.nio.file.Paths
import me.zhanghai.android.files.ftpserver.FtpServer
import me.zhanghai.android.files.ftpserver.FtpServerService
import org.apache.commons.net.ftp.FTPClient
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Exercises the loopback protocol surface. This source is intentionally not
 * run in the 1.2.0 source-only pass; it is for the later device-test stage.
 */
@RunWith(AndroidJUnit4::class)
class FtpServerInstrumentationTest {
    @Test
    fun loopbackLoginListUploadResumeDownloadRenameAndDelete() {
        val context = ApplicationProvider.getApplicationContext<android.content.Context>()
        val directory = File(context.cacheDir, "ftp-test-${System.nanoTime()}").apply { mkdirs() }
        val port = ServerSocket(0).use { it.localPort }
        val server = FtpServer(
            username = "test-user",
            password = "test-password",
            port = port,
            homeDirectory = Paths.get(directory.toURI()),
            writable = true,
            anonymous = false,
        )
        val client = FTPClient()
        try {
            server.start()
            client.connect("127.0.0.1", port, 5_000)
            assertTrue(client.login("test-user", "test-password"))
            client.enterLocalPassiveMode()
            assertTrue(client.changeWorkingDirectory("/"))

            val initial = "RongVault FTP".toByteArray()
            assertTrue(client.storeFile("upload.txt", ByteArrayInputStream(initial)))
            assertTrue(client.listNames().any { it == "upload.txt" || it.endsWith("/upload.txt") })

            client.restartOffset = 6
            assertTrue(client.storeFile("upload.txt", ByteArrayInputStream(" resumed".toByteArray())))
            client.restartOffset = 0

            val downloaded = ByteArrayOutputStream()
            assertTrue(client.retrieveFile("upload.txt", downloaded))
            assertArrayEquals("RongVa resumed".toByteArray(), downloaded.toByteArray())

            assertTrue(client.rename("upload.txt", "renamed.txt"))
            assertTrue(client.deleteFile("renamed.txt"))
            assertTrue(client.listNames().none { it.endsWith("renamed.txt") })
            assertTrue(client.logout())
        } finally {
            if (client.isConnected) client.disconnect()
            server.stop()
            directory.deleteRecursively()
        }
    }

    @Test
    fun anonymousLoginCanWriteWhenTheUserExplicitlyEnablesIt() {
        val context = ApplicationProvider.getApplicationContext<android.content.Context>()
        val directory = File(context.cacheDir, "ftp-anonymous-${System.nanoTime()}").apply { mkdirs() }
        val port = ServerSocket(0).use { it.localPort }
        val server = FtpServer(
            username = FtpServerService.USERNAME_ANONYMOUS,
            password = null,
            port = port,
            homeDirectory = Paths.get(directory.toURI()),
            writable = true,
            anonymous = true,
        )
        val client = FTPClient()
        try {
            server.start()
            client.connect("127.0.0.1", port, 5_000)
            assertTrue(client.login(FtpServerService.USERNAME_ANONYMOUS, ""))
            client.enterLocalPassiveMode()
            assertTrue(client.storeFile("anonymous.txt", ByteArrayInputStream(byteArrayOf(1, 2, 3))))
            assertTrue(client.deleteFile("anonymous.txt"))
        } finally {
            if (client.isConnected) client.disconnect()
            server.stop()
            directory.deleteRecursively()
        }
    }
}
