package org.eds.veracrypt.nativecore

import java.nio.ByteBuffer
import java.nio.ByteOrder
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.domain.VolumeOpenTarget
import org.junit.Test

class NativeRequestCodecTest {
    @Test
    fun automaticOpenRequestSetsTheAutoVolumeFlag() {
        val credentials = VolumeCredentials(password = SecretPassword("pass".toCharArray()))
        NativeRequestCodec.encodeOpen(VolumeOpenOptions(target = VolumeOpenTarget.AUTO), credentials).use { request ->
            request.useForJni { bytes ->
                val flags = ByteBuffer.wrap(bytes).order(ByteOrder.BIG_ENDIAN).apply { position(7) }.get().toInt()
                check(flags and 0x08 == 0x08)
                check(flags and 0x01 == 0)
            }
        }
        credentials.close()
    }

    @Test
    fun openRequestContainsNoKeyfileUriAndUsesStableHeader() {
        val options = VolumeOpenOptions(
            cipherHint = CipherHint.AES,
            accessMode = VolumeAccessMode.READ_WRITE,
        )
        val credentials = VolumeCredentials(
            password = SecretPassword("pass".toCharArray()),
            pim = 11,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        val request = NativeRequestCodec.encodeOpen(options, credentials)
        try {
            request.useForJni { bytes ->
                val buffer = ByteBuffer.wrap(bytes).order(ByteOrder.BIG_ENDIAN)
                check(buffer.int == 0x56435251)
                check(buffer.short.toInt() == 2)
                check(buffer.get().toInt() == 1)
                check(buffer.get().toInt() and 0x02 == 0x02)
                check(buffer.int == 11)
                check(buffer.get().toInt() == 1)
                check(buffer.get().toInt() == 1)
            }
        } finally {
            request.close()
            credentials.close()
        }
    }

    @Test
    fun createRequestHasAnIndependentStableLayout() {
        val options = VolumeCreateOptions(
            sizeBytes = 8L * 1024 * 1024,
            volumeKind = VolumeKind.NORMAL,
            cipher = CipherHint.SERPENT,
            kdf = KdfHint.ARGON2ID,
            pim = 7,
            fileSystem = VolumeFileSystem.EXFAT,
            sectorSizeBytes = 4096,
        )
        val credentials = VolumeCredentials(password = SecretPassword("create".toCharArray()))
        val request = NativeRequestCodec.encodeCreate(options, credentials)
        try {
            request.useForJni { bytes ->
                val buffer = ByteBuffer.wrap(bytes).order(ByteOrder.BIG_ENDIAN)
                check(buffer.int == 0x56435251)
                check(buffer.short.toInt() == 2)
                check(buffer.get().toInt() == 2)
                check(buffer.get().toInt() == 0)
                check(buffer.long == options.sizeBytes)
                check(buffer.int == 7)
                check(buffer.get().toInt() == 2)
                check(buffer.get().toInt() == 6)
                check(buffer.get().toInt() == 2)
                check(buffer.int == 4096)
                check(buffer.short.toInt() == 0)
                check(buffer.short.toInt() == 6)
            }
        } finally {
            request.close()
            credentials.close()
        }
    }

    @Test
    fun hiddenCreateRequestSetsOnlyTheHiddenFlag() {
        val options = VolumeCreateOptions(
            sizeBytes = 16L * 1024 * 1024,
            volumeKind = VolumeKind.HIDDEN,
            cipher = CipherHint.TWOFISH,
            kdf = KdfHint.PBKDF2_HMAC_SHA256,
            pim = 3,
            fileSystem = VolumeFileSystem.FAT,
            sectorSizeBytes = 512,
        )
        val credentials = VolumeCredentials(password = SecretPassword("hidden".toCharArray()))
        val request = NativeRequestCodec.encodeCreate(options, credentials)
        try {
            request.useForJni { bytes ->
                val buffer = ByteBuffer.wrap(bytes).order(ByteOrder.BIG_ENDIAN)
                check(buffer.int == 0x56435251)
                check(buffer.short.toInt() == 2)
                check(buffer.get().toInt() == 2)
                check(buffer.get().toInt() == 0x01)
                check(buffer.long == options.sizeBytes)
                check(buffer.int == options.pim)
                check(buffer.get().toInt() == 3)
                check(buffer.get().toInt() == 2)
                check(buffer.get().toInt() == 1)
                check(buffer.int == options.sectorSizeBytes)
            }
        } finally {
            request.close()
            credentials.close()
        }
    }
    @Test
    fun protectedOuterOpenRequestIncludesProtectionCredentialMetadata() {
        val protection = VolumeCredentials(
            password = SecretPassword("hidden".toCharArray()),
            pim = 17,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        val options = VolumeOpenOptions(
            cipherHint = CipherHint.AES,
            accessMode = VolumeAccessMode.READ_WRITE,
            hiddenVolumeProtection = protection,
        )
        val credentials = VolumeCredentials(
            password = SecretPassword("pass".toCharArray()),
            pim = 11,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        val request = NativeRequestCodec.encodeOpen(options, credentials)
        try {
            request.useForJni { bytes ->
                check(bytes.size == 36)
                val buffer = ByteBuffer.wrap(bytes).order(ByteOrder.BIG_ENDIAN)
                check(buffer.int == 0x56435251)
                check(buffer.short.toInt() == 2)
                check(buffer.get().toInt() == 1)
                check(buffer.get().toInt() == 0x0E)
                check(buffer.int == 11)
                check(buffer.get().toInt() == 1)
                check(buffer.get().toInt() == 1)
                check(buffer.short.toInt() == 0)
                check(buffer.short.toInt() == 4)
                buffer.position(buffer.position() + 4)
                check(buffer.int == 17)
                check(buffer.short.toInt() == 0)
                check(buffer.short.toInt() == 6)
                buffer.position(buffer.position() + 6)
                check(!buffer.hasRemaining())
            }
        } finally {
            request.close()
            credentials.close()
            protection.close()
        }
    }
}
