package org.eds.veracrypt.nativecore

import android.content.Intent
import android.os.ParcelFileDescriptor
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.MAX_PIM_VALUE
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.ui.ContainerCatalogActivity
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assume.assumeTrue
import org.junit.Test
import org.junit.runner.RunWith

/**
 * The Android half of the deliberately opt-in desktop round trip.  Containers
 * are retained only while the three documented phases execute; the desktop
 * PowerShell helper performs the matching mount/read/write checks.
 */
@RunWith(AndroidJUnit4::class)
class VcCoreDesktopInteropInstrumentationTest {
    private val interopCipher: CipherHint
        get() = when (InstrumentationRegistry.getArguments().getString("vc.cipher")?.uppercase() ?: "AES") {
            "AES" -> CipherHint.AES
            "SERPENT" -> CipherHint.SERPENT
            "TWOFISH" -> CipherHint.TWOFISH
            "CAMELLIA" -> CipherHint.CAMELLIA
            "AES-TWOFISH" -> CipherHint.TWOFISH_AES
            "AES-TWOFISH-SERPENT" -> CipherHint.SERPENT_TWOFISH_AES
            "SERPENT-AES" -> CipherHint.AES_SERPENT
            "SERPENT-TWOFISH-AES" -> CipherHint.AES_TWOFISH_SERPENT
            "TWOFISH-SERPENT" -> CipherHint.SERPENT_TWOFISH
            else -> error("vc.cipher must be one of the 9 VeraCrypt creation suites")
        }

    private val interopKdf: KdfHint
        get() = when (InstrumentationRegistry.getArguments().getString("vc.kdf")?.uppercase() ?: "SHA-512") {
            "SHA-512" -> KdfHint.PBKDF2_HMAC_SHA512
            "SHA-256" -> KdfHint.PBKDF2_HMAC_SHA256
            "BLAKE2S-256" -> KdfHint.PBKDF2_HMAC_BLAKE2S
            "WHIRLPOOL" -> KdfHint.PBKDF2_HMAC_WHIRLPOOL
            "STREEBOG" -> KdfHint.PBKDF2_HMAC_STREEBOG
            "ARGON2", "ARGON2ID" -> KdfHint.ARGON2ID
            else -> error("vc.kdf must be one of the 6 supported VeraCrypt KDFs")
        }

    private val interopPim: Int
        get() {
            val raw = InstrumentationRegistry.getArguments().getString("vc.pim") ?: return 1
            val value = raw.toLongOrNull() ?: error("vc.pim must be an integer")
            require(value in 0..MAX_PIM_VALUE.toLong()) {
                "vc.pim must be between 0 and $MAX_PIM_VALUE"
            }
            return value.toInt()
        }

    @Test
    fun desktopCreatedContainerOpensAndAcceptsAndroidMarker() = withForegroundOperation {
        withInteropFile(DESKTOP_TO_ANDROID) { container ->
            withOpenSession(container) { session ->
                assertEquals(2, VcCore.nativeMountFileSystem(session))
                writeFile(session, ANDROID_MARKER, ANDROID_PAYLOAD)
            }
        }
    }

    @Test
    fun desktopCreatedContainerReadsDesktopReturnMarker() = withForegroundOperation {
        withInteropFile(DESKTOP_TO_ANDROID) { container ->
            withOpenSession(container) { session ->
                assertEquals(2, VcCore.nativeMountFileSystem(session))
                assertArrayEquals(DESKTOP_PAYLOAD, readFile(session, DESKTOP_MARKER, DESKTOP_PAYLOAD.size))
            }
        }
    }

    @Test
    fun androidCreatesContainerForDesktop() = withForegroundOperation {
        assumeInteropEnabled()
        val container = interopFile(ANDROID_TO_DESKTOP)
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            credentials().use { credentials ->
                val options = VolumeCreateOptions(
                    sizeBytes = CONTAINER_BYTES,
                    volumeKind = VolumeKind.NORMAL,
                    cipher = interopCipher,
                    kdf = interopKdf,
                    pim = interopPim,
                    fileSystem = VolumeFileSystem.EXFAT,
                )
                NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                    session = request.useForJni { bytes ->
                        VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                    }
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(session))
            writeFile(session, ANDROID_ORIGIN, ANDROID_PAYLOAD)
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
        }
    }

    @Test
    fun androidCreatedContainerReadsDesktopReturnMarker() = withForegroundOperation {
        withInteropFile(ANDROID_TO_DESKTOP) { container ->
            withOpenSession(container) { session ->
                assertEquals(2, VcCore.nativeMountFileSystem(session))
                assertArrayEquals(DESKTOP_PAYLOAD, readFile(session, DESKTOP_MARKER, DESKTOP_PAYLOAD.size))
            }
        }
    }

    private fun <T> withForegroundOperation(block: () -> T): T {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK))
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        return try {
            block()
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    private fun withInteropFile(name: String, block: (File) -> Unit) {
        assumeInteropEnabled()
        val container = interopFile(name)
        assumeTrue("Stage $name through the documented desktop helper first", container.isFile)
        block(container)
    }

    private fun withOpenSession(container: File, block: (Long) -> Unit) {
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        try {
            descriptor = ParcelFileDescriptor.open(container, ParcelFileDescriptor.MODE_READ_WRITE)
            credentials().use { credentials ->
                NativeRequestCodec.encodeOpen(
                    VolumeOpenOptions(cipherHint = interopCipher, accessMode = VolumeAccessMode.READ_WRITE),
                    credentials,
                ).use { request ->
                    session = request.useForJni { bytes -> VcCore.nativeOpen(descriptor!!.fd, true, bytes, intArrayOf()) }
                }
            }
            block(session)
            VcCore.nativeFlush(session)
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
        }
    }

    private fun writeFile(session: Long, name: String, payload: ByteArray) {
        val file = VcCore.nativeOpenFile(session, name, writable = true, create = true, truncate = true)
        try {
            assertEquals(payload.size, VcCore.nativeWriteFile(file, 0, payload, 0, payload.size))
            VcCore.nativeFlushFile(file)
        } finally {
            VcCore.nativeCloseFile(file)
        }
    }

    private fun readFile(session: Long, name: String, size: Int): ByteArray {
        val file = VcCore.nativeOpenFile(session, name, writable = false, create = false, truncate = false)
        try {
            return ByteArray(size).also { target ->
                assertEquals(target.size, VcCore.nativeReadFile(file, 0, target, 0, target.size))
            }
        } finally {
            VcCore.nativeCloseFile(file)
        }
    }

    private fun interopFile(name: String): File = File(
        checkNotNull(InstrumentationRegistry.getInstrumentation().targetContext.getExternalFilesDir("interop")),
        name,
    ).also { it.parentFile?.mkdirs() }

    private fun credentials() = VolumeCredentials(
        password = SecretPassword(PASSWORD.toCharArray()),
        pim = interopPim,
        kdfHint = interopKdf,
    )

    private fun assumeInteropEnabled() = assumeTrue(
        "Opt in with instrumentation argument vc.desktopInterop=true",
        InstrumentationRegistry.getArguments().getString("vc.desktopInterop") == "true",
    )

    private companion object {
        const val CONTAINER_BYTES = 64L * 1024 * 1024
        const val PASSWORD = "EDS-TEST-desktop-interop"
        const val DESKTOP_TO_ANDROID = "EDS-TEST-desktop-to-android.hc"
        const val ANDROID_TO_DESKTOP = "EDS-TEST-android-to-desktop.hc"
        const val ANDROID_MARKER = "android-marker.bin"
        const val ANDROID_ORIGIN = "android-origin.bin"
        const val DESKTOP_MARKER = "desktop-return.bin"
        val ANDROID_PAYLOAD = "EDS Android interop marker v1".encodeToByteArray()
        val DESKTOP_PAYLOAD = "EDS Desktop interop marker v1".encodeToByteArray()
    }
}
