package org.eds.veracrypt.catalog

import android.net.Uri
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ContainerCatalogInstrumentationTest {
    @Test
    fun persistsContainerIdentityWithoutOpenAlgorithmHints() {
        val catalog = ContainerCatalog(InstrumentationRegistry.getInstrumentation().targetContext)
        val entry = catalog.add(Uri.parse("content://org.eds.test/EDS-TEST-stage5.hc"), "EDS-TEST-stage5")
        try {
            val reloaded = ContainerCatalog(InstrumentationRegistry.getInstrumentation().targetContext).find(entry.id)
            assertEquals(entry.id, reloaded?.id)
            assertEquals(entry.uri, reloaded?.uri)
            assertEquals(entry.displayName, reloaded?.displayName)
            assertFalse("catalog entry must not persist credentials", reloaded.toString().contains("password", ignoreCase = true))
        } finally {
            catalog.remove(entry.id)
        }
    }

    @Test
    fun clearRemovesAllEntriesAndKeepsContainerUrisUntouched() {
        val catalog = ContainerCatalog(InstrumentationRegistry.getInstrumentation().targetContext)
        catalog.clear()
        val first = catalog.add(Uri.parse("content://org.eds.test/EDS-TEST-clear-one.hc"), "clear-one")
        val second = catalog.add(Uri.parse("content://org.eds.test/EDS-TEST-clear-two.hc"), "clear-two")
        try {
            catalog.setClearOnExitEnabled(true)
            assertTrue(catalog.clearOnExitEnabled())
            catalog.clear()
            assertTrue(catalog.list().isEmpty())
            assertFalse(catalog.find(first.id) != null)
            assertFalse(catalog.find(second.id) != null)
        } finally {
            catalog.setClearOnExitEnabled(false)
            catalog.clear()
        }
    }
}
