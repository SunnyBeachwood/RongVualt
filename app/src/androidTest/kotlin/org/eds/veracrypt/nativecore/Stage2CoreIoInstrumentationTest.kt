package org.eds.veracrypt.nativecore

import android.os.ParcelFileDescriptor
import android.content.Intent
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.ui.ContainerCatalogActivity
import org.junit.After
import org.junit.Before
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

/** Focused Stage-2 regression tests for serial batching and RMW boundaries. */
@RunWith(AndroidJUnit4::class)
class Stage2CoreIoInstrumentationTest {
    @Before
    fun acquireForegroundLease() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        context.startActivity(Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK))
        InstrumentationRegistry.getInstrumentation().waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
    }

    @After
    fun releaseForegroundLease() {
        UnlockedVolumeService.endLongRunningOperation()
    }

    @Test
    fun serialBatchAlignedAndUnalignedRoundTripsAcrossCiphers() {
        listOf(CipherHint.AES, CipherHint.SERPENT, CipherHint.TWOFISH).forEach { cipher ->
            withNormalSession(cipher) { session ->
                listOf(0L, 1L, 511L, 512L, 513L).forEach { offset ->
                    listOf(0, 1, 511, 512, 513, 4096, 256 * 1024 - 1, 256 * 1024, 256 * 1024 + 1).forEach { length ->
                        val expected = ByteArray(length) { index -> (index * 17 + offset.toInt()).toByte() }
                        assertEquals(length, VcCore.nativeWrite(session, offset, expected, 0, length))
                        val actual = ByteArray(length)
                        assertEquals(length, VcCore.nativeRead(session, offset, actual, 0, length))
                        assertArrayEquals("serial boundary mismatch: $cipher/$offset/$length", expected, actual)
                    }
                }
            }
        }
    }

    @Test
    fun alignedBatchUsesOneWriteAndNoRead() {
        withNormalSession(CipherHint.AES) { session ->
            val before = VcCore.nativeGetPerformanceCounters(session)
            val data = ByteArray(256 * 1024) { 0x5A }
            assertEquals(data.size, VcCore.nativeWrite(session, 0, data, 0, data.size))
            val after = VcCore.nativeGetPerformanceCounters(session)
            assertEquals("aligned batch must not RMW-read", before[0], after[0])
            assertEquals("aligned batch must issue one write", 1L, after[1] - before[1])
            assertEquals("aligned batch counter", 1L, after[5] - before[5])
        }
    }

    private fun withNormalSession(cipher: CipherHint, block: (Long) -> Unit) {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val container = File(context.cacheDir, "EDS-TEST-stage2-${cipher.name}.hc")
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        val credentials = VolumeCredentials(
            password = SecretPassword("stage2-${cipher.name}".toCharArray()),
            pim = 1,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            val options = VolumeCreateOptions(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = cipher,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.EXFAT,
            )
            NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                session = request.useForJni { bytes ->
                    VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            block(session)
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            credentials.close()
            assertTrue(container.delete() || !container.exists())
        }
    }
}
