package org.eds.veracrypt.domain

import java.io.Closeable
import java.nio.CharBuffer
import java.nio.charset.CodingErrorAction
import java.util.UUID
import kotlinx.coroutines.flow.StateFlow

/** VeraCrypt's non-boot PIM upper bound (15000 + PIM * 1000 fits signed 32-bit). */
const val MAX_PIM_VALUE: Int = 2_147_468

/** Credentials are request-scoped and must be cleared by the caller after use. */
class SecretPassword(chars: CharArray) : Closeable {
    private var value: CharArray? = chars.copyOf()

    fun copyForNativeUse(): ByteArray {
        val current = checkNotNull(value) { "Password has been cleared" }
        val encoded = Charsets.UTF_8.newEncoder()
            .onMalformedInput(CodingErrorAction.REPORT)
            .onUnmappableCharacter(CodingErrorAction.REPORT)
            .encode(CharBuffer.wrap(current))
        require(encoded.remaining() <= MAX_UTF8_BYTES) { "Passwords are limited to $MAX_UTF8_BYTES UTF-8 bytes" }
        return ByteArray(encoded.remaining()).also(encoded::get)
    }

    override fun close() {
        value?.fill('\u0000')
        value = null
    }

    private companion object {
        const val MAX_UTF8_BYTES = 128
    }
}

data class KeyfileSource(val uri: String)

enum class VolumeKind { NORMAL, HIDDEN }
/** Target requested while opening. AUTO never escapes an open request. */
enum class VolumeOpenTarget { AUTO, NORMAL, HIDDEN }
/** AUTOMATIC is valid only on an open request; live sessions are always concrete. */
enum class VolumeAccessMode { AUTOMATIC, READ_ONLY, READ_WRITE }
/**
 * VeraCrypt 1.26.29 non-system XTS suites. Enum names and values are part of
 * the persisted/native protocol and therefore retain the on-disk cipher order.
 * Use [displayName] for the user-facing VeraCrypt order.
 */
enum class CipherHint(val isCreatable: Boolean) {
    AUTO(false),
    AES(true),
    SERPENT(true),
    TWOFISH(true),
    CAMELLIA(true),
    KUZNYECHIK(false),
    TWOFISH_AES(true),
    SERPENT_TWOFISH_AES(true),
    AES_SERPENT(true),
    AES_TWOFISH_SERPENT(true),
    SERPENT_TWOFISH(true),
    KUZNYECHIK_CAMELLIA(false),
    TWOFISH_KUZNYECHIK(false),
    SERPENT_CAMELLIA(false),
    AES_KUZNYECHIK(false),
    CAMELLIA_SERPENT_KUZNYECHIK(false);

    /** All suites are recognized by the native reader. */
    val isOpenable: Boolean
        get() = this != AUTO

    /** VeraCrypt displays cascade names from the outermost cipher inward. */
    val displayName: String
        get() = when (this) {
            AUTO -> "Automatic"
            AES -> "AES"
            SERPENT -> "Serpent"
            TWOFISH -> "Twofish"
            CAMELLIA -> "Camellia"
            KUZNYECHIK -> "Kuznyechik"
            TWOFISH_AES -> "AES-Twofish"
            SERPENT_TWOFISH_AES -> "AES-Twofish-Serpent"
            AES_SERPENT -> "Serpent-AES"
            AES_TWOFISH_SERPENT -> "Serpent-Twofish-AES"
            SERPENT_TWOFISH -> "Twofish-Serpent"
            KUZNYECHIK_CAMELLIA -> "Camellia-Kuznyechik"
            TWOFISH_KUZNYECHIK -> "Kuznyechik-Twofish"
            SERPENT_CAMELLIA -> "Camellia-Serpent"
            AES_KUZNYECHIK -> "Kuznyechik-AES"
            CAMELLIA_SERPENT_KUZNYECHIK -> "Kuznyechik-Serpent-Camellia"
        }
}
enum class KdfHint {
    AUTO,
    PBKDF2_HMAC_SHA512,
    PBKDF2_HMAC_SHA256,
    PBKDF2_HMAC_BLAKE2S,
    PBKDF2_HMAC_WHIRLPOOL,
    PBKDF2_HMAC_STREEBOG,
    ARGON2ID;

    val displayName: String
        get() = when (this) {
            AUTO -> "Automatic"
            PBKDF2_HMAC_SHA512 -> "PBKDF2-HMAC-SHA-512"
            PBKDF2_HMAC_SHA256 -> "PBKDF2-HMAC-SHA-256"
            PBKDF2_HMAC_BLAKE2S -> "PBKDF2-HMAC-BLAKE2s-256"
            PBKDF2_HMAC_WHIRLPOOL -> "PBKDF2-HMAC-Whirlpool"
            PBKDF2_HMAC_STREEBOG -> "PBKDF2-HMAC-Streebog"
            ARGON2ID -> "Argon2id"
        }
}
enum class VolumeFileSystem { FAT, EXFAT, NTFS }
enum class FormatStrategy { FULL }

data class VolumeCredentials(
    val password: SecretPassword,
    val pim: Int = 0,
    val keyfiles: List<KeyfileSource> = emptyList(),
    val kdfHint: KdfHint = KdfHint.AUTO,
) : Closeable {
    init { require(pim in 0..MAX_PIM_VALUE) { "PIM must be between 0 and $MAX_PIM_VALUE" } }
    override fun close() = password.close()
}

data class VolumeOpenOptions(
    val cipherHint: CipherHint = CipherHint.AUTO,
    val target: VolumeOpenTarget = VolumeOpenTarget.AUTO,
    val accessMode: VolumeAccessMode = VolumeAccessMode.AUTOMATIC,
    val hiddenVolumeProtection: VolumeCredentials? = null,
) {
    /** Source-compatible bridge for callers that explicitly select one volume type. */
    constructor(
        cipherHint: CipherHint = CipherHint.AUTO,
        volumeKind: VolumeKind,
        accessMode: VolumeAccessMode = VolumeAccessMode.AUTOMATIC,
        hiddenVolumeProtection: VolumeCredentials? = null,
    ) : this(
        cipherHint = cipherHint,
        target = if (volumeKind == VolumeKind.HIDDEN) VolumeOpenTarget.HIDDEN else VolumeOpenTarget.NORMAL,
        accessMode = accessMode,
        hiddenVolumeProtection = hiddenVolumeProtection,
    )

    init {
        require(cipherHint == CipherHint.AUTO || cipherHint.isOpenable) { "Unsupported VeraCrypt cipher" }
        require(hiddenVolumeProtection == null || accessMode != VolumeAccessMode.READ_ONLY) {
            "Hidden-volume protection is only meaningful for writable outer volumes"
        }
    }

}

data class VolumeCreateOptions(
    val sizeBytes: Long,
    val volumeKind: VolumeKind,
    val cipher: CipherHint = CipherHint.AES,
    val kdf: KdfHint = KdfHint.PBKDF2_HMAC_SHA512,
    val pim: Int = 0,
    val fileSystem: VolumeFileSystem = VolumeFileSystem.EXFAT,
    val sectorSizeBytes: Int = 512,
    val formatStrategy: FormatStrategy = FormatStrategy.FULL,
) {
    init {
        require(sizeBytes > 0) { "Volume size must be positive" }
        require(pim in 0..MAX_PIM_VALUE) { "PIM must be between 0 and $MAX_PIM_VALUE" }
        require(cipher.isCreatable && cipher.isOpenable) { "This VeraCrypt cipher cannot be used for new volumes" }
        require(kdf != KdfHint.AUTO) { "A concrete KDF is required when creating a volume" }
        require(sectorSizeBytes in setOf(512, 1024, 2048, 4096)) {
            "VeraCrypt logical sectors must be 512, 1024, 2048, or 4096 bytes"
        }
        require(fileSystem != VolumeFileSystem.NTFS) { "NTFS is read-only and cannot be created" }
        require(formatStrategy == FormatStrategy.FULL) { "Quick formatting is not supported" }
    }
}

sealed interface VolumeSessionState {
    data object Open : VolumeSessionState
    /** Hidden-volume protection rejected a write; session remains readable only. */
    data object ProtectionTriggered : VolumeSessionState
    data object Closing : VolumeSessionState
    data object Closed : VolumeSessionState
    data class Failed(val error: VolumeError) : VolumeSessionState
}

interface VolumeSession : Closeable {
    val id: UUID
    val state: StateFlow<VolumeSessionState>
    val accessMode: VolumeAccessMode
    val volumeKind: VolumeKind
    val fileSystem: VolumeFileSystem?
    val isReadOnly: Boolean
    /** True when the encrypted container descriptor itself was opened writable. */
    val canModifyContainer: Boolean
    /** Commits pending encrypted writes. Call from an I/O dispatcher. */
    fun flush()
    /** Mounts FAT/FAT32/exFAT or read-only NTFS and returns its detected type. */
    fun mountFileSystem(): VolumeFileSystem
    /** Records activity for automatic locking without exposing native state. */
    fun touch()
    override fun close()
}
