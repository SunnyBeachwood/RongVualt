/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package org.eds.veracrypt.ftp

import androidx.test.ext.junit.runners.AndroidJUnit4
import java8.nio.file.Paths
import me.zhanghai.android.files.provider.common.delete
import me.zhanghai.android.files.provider.common.newInputStream
import me.zhanghai.android.files.provider.common.newOutputStream
import me.zhanghai.android.files.provider.root.LibSuFileServiceLauncher
import me.zhanghai.android.files.provider.root.RootStrategy
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.util.valueCompat
import org.junit.After
import org.junit.Assume.assumeTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

/** Root-device coverage; non-root devices skip this test at runtime. */
@RunWith(AndroidJUnit4::class)
class RootFileAccessInstrumentationTest {
    private var previousStrategy: RootStrategy? = null
    private val path = Paths.get("/data/local/tmp/rongvault-root-${System.nanoTime()}.txt")

    @Before
    fun requireRootAndUseTheExplicitStrategy() {
        assumeTrue(LibSuFileServiceLauncher.isSuAvailable())
        previousStrategy = Settings.ROOT_STRATEGY.valueCompat
        Settings.ROOT_STRATEGY.putValue(RootStrategy.ALWAYS)
    }

    @After
    fun restoreSettingsAndRemoveFile() {
        runCatching { path.delete() }
        previousStrategy?.let(Settings.ROOT_STRATEGY::putValue)
    }

    @Test
    fun rootCanReadAndWriteARestrictedSystemPath() {
        path.newOutputStream().use { it.write("root-ok".toByteArray()) }
        val result = path.newInputStream().use { it.readBytes().decodeToString() }
        check(result == "root-ok")
    }
}
