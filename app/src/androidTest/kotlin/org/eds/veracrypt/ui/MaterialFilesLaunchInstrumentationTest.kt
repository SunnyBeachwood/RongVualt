package org.eds.veracrypt.ui

import androidx.test.core.app.ActivityScenario
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import java.io.File
import java8.nio.file.Paths
import me.zhanghai.android.files.file.fileProviderUri
import me.zhanghai.android.files.security.CredentialVault
import org.eds.veracrypt.catalog.ContainerSourceResolver
import me.zhanghai.android.files.filelist.FileListActivity
import org.junit.Test
import org.junit.runner.RunWith

/** Verifies the embedded, non-exported Material Files Activity can start in-process. */
@RunWith(AndroidJUnit4::class)
class MaterialFilesLaunchInstrumentationTest {
    @Test
    fun hostCatalogLaunchesAlongsideMaterialFilesLifecycleHelpers() {
        // Material Files registers process-wide ActivityLifecycleCallbacks. The host catalog
        // does not inherit its AppActivity base class, so this guards the embedding boundary.
        ActivityScenario.launch(ContainerCatalogActivity::class.java).use { scenario ->
            scenario.onActivity { activity -> check(!activity.isFinishing) }
        }
    }

    @Test
    fun fileListActivityLaunches() {
        ActivityScenario.launch(FileListActivity::class.java).use { scenario ->
            scenario.onActivity { activity -> check(!activity.isFinishing) }
        }
    }

    @Test
    fun regularBrowserLaunchesAtSharedStorage() {
        val context = ApplicationProvider.getApplicationContext<android.content.Context>()
        ActivityScenario.launch<me.zhanghai.android.files.filelist.FileListActivity>(
            FileManagerIntents.browse(context),
        ).use { scenario ->
            scenario.onActivity { activity -> check(!activity.isFinishing) }
        }
    }

    @Test
    fun materialFilesSettingsLaunchesWithItsOwnTheme() {
        ActivityScenario.launch(me.zhanghai.android.files.settings.SettingsActivity::class.java).use { scenario ->
            scenario.onActivity { activity -> check(!activity.isFinishing) }
        }
    }

    @Test
    fun privateFileProviderAllowsOnlyLocalContainerCandidate() {
        val context = ApplicationProvider.getApplicationContext<android.content.Context>()
        val file = File(context.filesDir, "picker-container-test.hc").apply { writeBytes(byteArrayOf(0)) }
        try {
            val uri = Paths.get(file.toURI()).fileProviderUri
            check(ContainerSourceResolver.validate(context, uri) == null)
        } finally {
            file.delete()
        }
    }

    @Test
    fun privateFileProviderRejectsRemoteContainerCandidate() {
        val context = ApplicationProvider.getApplicationContext<android.content.Context>()
        val remoteUri = android.net.Uri.Builder()
            .scheme("content")
            .authority("${context.packageName}.materialfiles.files")
            .path(android.net.Uri.encode("ftp://example.test/container.hc"))
            .build()
        check(ContainerSourceResolver.validate(context, remoteUri) != null)
    }

    @Test
    fun credentialVaultRoundTripsAndRemovesRecord() {
        val key = "instrumentation-${System.nanoTime()}"
        CredentialVault.put(key, "test-password")
        check(CredentialVault.get(key) == "test-password")
        CredentialVault.remove(key)
        check(CredentialVault.get(key) == null)
    }
}
