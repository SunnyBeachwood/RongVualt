package org.eds.veracrypt.nativecore

import java.io.Closeable
import java.nio.ByteBuffer
import java.nio.ByteOrder
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenTarget
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeOpenOptions

/**
 * Versioned request framing for vc_core. Container and keyfile locations are
 * intentionally absent: JNI receives their already-opened descriptors.
 */
internal object NativeRequestCodec {
    fun encodeOpen(
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
        keyfileCount: Int = credentials.keyfiles.size,
        protectionKeyfileCount: Int = options.hiddenVolumeProtection?.keyfiles?.size ?: 0,
    ): EncodedNativeRequest {
        val password = credentials.password.copyForNativeUse()
        val hiddenPassword = options.hiddenVolumeProtection?.password?.copyForNativeUse()
        try {
            require(keyfileCount in 0..MAX_KEYFILES) { "Too many keyfiles" }
            val size = HEADER_BYTES + credentialSize(password) +
                (hiddenPassword?.let { HIDDEN_PROTECTION_HEADER_BYTES + credentialSize(it) } ?: 0)
            val buffer = ByteBuffer.allocate(size).order(ByteOrder.BIG_ENDIAN)
            buffer.putInt(MAGIC)
            buffer.putShort(VERSION.toShort())
            buffer.put(OP_OPEN)
            buffer.put(flags(options))
            buffer.putInt(credentials.pim)
            buffer.put(cipherCode(options.cipherHint))
            buffer.put(kdfCode(credentials.kdfHint))
            buffer.putShort(keyfileCount.toShort())
            putCredential(buffer, password)
            options.hiddenVolumeProtection?.let { protection ->
                putHiddenProtection(buffer, protection, protectionKeyfileCount, checkNotNull(hiddenPassword))
            }
            check(!buffer.hasRemaining()) { "Native request size calculation is wrong" }
            return EncodedNativeRequest(buffer.array())
        } finally {
            password.fill(0)
            hiddenPassword?.fill(0)
        }
    }

    fun encodeCreate(
        options: VolumeCreateOptions,
        credentials: VolumeCredentials,
        keyfileCount: Int = credentials.keyfiles.size,
    ): EncodedNativeRequest {
        val password = credentials.password.copyForNativeUse()
        try {
            require(keyfileCount in 0..MAX_KEYFILES) { "Too many keyfiles" }
            val buffer = ByteBuffer.allocate(CREATE_FIXED_BYTES + credentialSize(password)).order(ByteOrder.BIG_ENDIAN)
            buffer.putInt(MAGIC)
            buffer.putShort(VERSION.toShort())
            buffer.put(OP_CREATE)
            buffer.put(if (options.volumeKind == org.eds.veracrypt.domain.VolumeKind.HIDDEN) 0x01 else 0x00)
            buffer.putLong(options.sizeBytes)
            buffer.putInt(options.pim)
            buffer.put(cipherCode(options.cipher))
            buffer.put(kdfCode(options.kdf))
            buffer.put(
                when (options.fileSystem) {
                    VolumeFileSystem.FAT -> 1
                    VolumeFileSystem.EXFAT -> 2
                    VolumeFileSystem.NTFS -> throw IllegalArgumentException("NTFS cannot be created")
                },
            )
            buffer.putInt(options.sectorSizeBytes)
            buffer.putShort(keyfileCount.toShort())
            putCredential(buffer, password)
            check(!buffer.hasRemaining()) { "Native create request size calculation is wrong" }
            return EncodedNativeRequest(buffer.array())
        } finally {
            password.fill(0)
        }
    }

    private fun flags(options: VolumeOpenOptions): Byte {
        var value = 0
        if (options.target == VolumeOpenTarget.HIDDEN) value = value or 0x01
        if (options.target == VolumeOpenTarget.AUTO) value = value or 0x08
        if (options.accessMode == VolumeAccessMode.READ_WRITE) value = value or 0x02
        if (options.hiddenVolumeProtection != null) value = value or 0x04
        return value.toByte()
    }

    private fun credentialSize(password: ByteArray) = 2 + password.size

    private fun putCredential(buffer: ByteBuffer, password: ByteArray) {
        require(password.size <= MAX_PASSWORD_BYTES) { "Password is too long" }
        buffer.putShort(password.size.toShort())
        buffer.put(password)
    }

    private fun putHiddenProtection(buffer: ByteBuffer, protection: VolumeCredentials, keyfileCount: Int, password: ByteArray) {
        require(keyfileCount in 0..MAX_KEYFILES) { "Too many hidden-volume protection keyfiles" }
        buffer.putInt(protection.pim)
        buffer.putShort(keyfileCount.toShort())
        putCredential(buffer, password)
    }

    private fun cipherCode(value: CipherHint): Byte = when (value) {
        CipherHint.AUTO -> 0
        CipherHint.AES -> 1
        CipherHint.SERPENT -> 2
        CipherHint.TWOFISH -> 3
        CipherHint.CAMELLIA -> 4
        CipherHint.KUZNYECHIK -> 5
        CipherHint.TWOFISH_AES -> 6
        CipherHint.SERPENT_TWOFISH_AES -> 7
        CipherHint.AES_SERPENT -> 8
        CipherHint.AES_TWOFISH_SERPENT -> 9
        CipherHint.SERPENT_TWOFISH -> 10
        CipherHint.KUZNYECHIK_CAMELLIA -> 11
        CipherHint.TWOFISH_KUZNYECHIK -> 12
        CipherHint.SERPENT_CAMELLIA -> 13
        CipherHint.AES_KUZNYECHIK -> 14
        CipherHint.CAMELLIA_SERPENT_KUZNYECHIK -> 15
    }

    private fun kdfCode(value: KdfHint): Byte = when (value) {
        KdfHint.AUTO -> 0
        KdfHint.PBKDF2_HMAC_SHA512 -> 1
        KdfHint.PBKDF2_HMAC_SHA256 -> 2
        KdfHint.PBKDF2_HMAC_BLAKE2S -> 3
        KdfHint.PBKDF2_HMAC_WHIRLPOOL -> 4
        KdfHint.PBKDF2_HMAC_STREEBOG -> 5
        KdfHint.ARGON2ID -> 6
    }

    internal class EncodedNativeRequest internal constructor(private val value: ByteArray) : Closeable {
        /** Limits the second JVM-side request copy to the native call scope. */
        fun <T> useForJni(block: (ByteArray) -> T): T {
            val copy = value.copyOf()
            return try {
                block(copy)
            } finally {
                copy.fill(0)
            }
        }

        override fun close() {
            value.fill(0)
        }
    }

    private const val MAGIC = 0x56435251 // VCRQ
    private const val VERSION = 2
    private const val OP_OPEN: Byte = 1
    private const val OP_CREATE: Byte = 2
    private const val HEADER_BYTES = 16
    private const val HIDDEN_PROTECTION_HEADER_BYTES = 6 // PIM plus keyfile count.
    private const val CREATE_FIXED_BYTES = 29
    private const val MAX_PASSWORD_BYTES = 128
    private const val MAX_KEYFILES = 1_024
}
