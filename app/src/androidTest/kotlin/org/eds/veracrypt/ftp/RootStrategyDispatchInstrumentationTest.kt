/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package org.eds.veracrypt.ftp

import androidx.test.ext.junit.runners.AndroidJUnit4
import me.zhanghai.android.files.provider.remote.RemoteFileSystemException
import me.zhanghai.android.files.provider.root.LibSuFileServiceLauncher
import me.zhanghai.android.files.provider.root.RootFileService
import me.zhanghai.android.files.provider.root.RootStrategy
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Assume.assumeFalse
import org.junit.Test
import org.junit.runner.RunWith

/** Root strategy/failure assertions for the later privileged-device phase. */
@RunWith(AndroidJUnit4::class)
class RootStrategyDispatchInstrumentationTest {
    @Test
    fun rootStrategyOrdinalsRemainCompatible() {
        assertEquals(0, RootStrategy.NEVER.ordinal)
        assertEquals(1, RootStrategy.AUTOMATIC.ordinal)
        assertEquals(2, RootStrategy.ALWAYS.ordinal)
        assertEquals(15_000L, RootFileService.TIMEOUT_MILLIS)
    }

    @Test
    fun unavailableRootIsReportedInsteadOfFallingBack() {
        assumeFalse(LibSuFileServiceLauncher.isSuAvailable())
        try {
            LibSuFileServiceLauncher.launchService()
            throw AssertionError("Root launch unexpectedly succeeded")
        } catch (error: RemoteFileSystemException) {
            assertTrue(error.message.orEmpty().contains("Root", ignoreCase = true))
        }
    }
}
