/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package org.eds.veracrypt.ftp

import android.net.Uri
import androidx.test.ext.junit.runners.AndroidJUnit4
import java8.nio.file.Paths
import me.zhanghai.android.files.navigation.RuntimeNavigationRoot
import me.zhanghai.android.files.navigation.RuntimeNavigationRoots
import me.zhanghai.android.files.ftpserver.FtpShareRootStore
import me.zhanghai.android.files.provider.document.createDocumentTreeRootPath
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.util.valueCompat
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.After
import org.junit.Test
import org.junit.runner.RunWith

/** Verifies that an unlocked share is process-only and invalidates atomically. */
@RunWith(AndroidJUnit4::class)
class FtpShareRootInstrumentationTest {
    private val persistentPath = Settings.FTP_SERVER_HOME_DIRECTORY.valueCompat

    @After
    fun clearRuntimeRoot() {
        RuntimeNavigationRoots.replace(emptyList())
        FtpShareRootStore.cancelPendingStart()
        FtpShareRootStore.select(persistentPath)
    }

    @Test
    fun runtimeRootIsNotPersistedAndStopsBeingLiveWhenVolumeDisappears() {
        val rootPath = Paths.get("/tmp/rongvault-unlocked-${System.nanoTime()}")
        val rootId = "test-volume-${System.nanoTime()}"
        RuntimeNavigationRoots.replace(
            listOf(
                RuntimeNavigationRoot(
                    id = rootId,
                    treeUri = Uri.parse("content://app.rongvault.unlocked/$rootId"),
                    path = rootPath,
                    title = "Test volume",
                    subtitle = null,
                    iconRes = 0,
                    isReadOnly = false,
                ),
            ),
        )

        FtpShareRootStore.select(rootPath.resolve("shared"))
        val selected = FtpShareRootStore.requestStart()
        assertEquals(rootId, selected.runtimeRootId)
        assertEquals(persistentPath, Settings.FTP_SERVER_HOME_DIRECTORY.valueCompat)
        assertTrue(FtpShareRootStore.isLive(selected))

        RuntimeNavigationRoots.replace(emptyList())
        assertFalse(FtpShareRootStore.isLive(selected))
        FtpShareRootStore.select(
            Uri.parse("content://app.rongvault.unlocked/stale").createDocumentTreeRootPath()
        )
        assertEquals(persistentPath, Settings.FTP_SERVER_HOME_DIRECTORY.valueCompat)
        // The invalidation path clears a queued runtime start and falls back to
        // the persisted ordinary directory before any native session closes.
        FtpShareRootStore.invalidateRuntimeRoot(rootId)
        assertNull(FtpShareRootStore.takePendingStart())
        assertTrue(FtpShareRootStore.consumeCancelledPendingStart())
        assertFalse(FtpShareRootStore.consumeCancelledPendingStart())
        assertNull(FtpShareRootStore.current().runtimeRootId)
    }
}
