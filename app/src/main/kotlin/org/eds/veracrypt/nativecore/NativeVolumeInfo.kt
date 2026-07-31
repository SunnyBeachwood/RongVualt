package org.eds.veracrypt.nativecore

import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint

/**
 * Non-secret metadata from an active native session. Filesystem identification
 * is intentionally absent: it is discovered by the FAT/exFAT/NTFS adapter,
 * not stored in a VeraCrypt header.
 */
internal data class NativeVolumeInfo(
    val logicalSize: Long,
    val encryptedAreaOffset: Long,
    val encryptedAreaSize: Long,
    val sectorSize: Int,
    val cipherCode: Int,
    val kdfCode: Int,
    val isHiddenVolume: Boolean,
    val usedBackupHeader: Boolean,
)

internal fun VcCore.volumeInfo(sessionHandle: Long): NativeVolumeInfo {
    val values = nativeGetVolumeInfo(sessionHandle)
    require(values.size == 8) { "Native volume info has an invalid shape" }
    return NativeVolumeInfo(
        logicalSize = values[0],
        encryptedAreaOffset = values[1],
        encryptedAreaSize = values[2],
        sectorSize = values[3].toInt(),
        cipherCode = values[4].toInt(),
        kdfCode = values[5].toInt(),
        isHiddenVolume = values[6] != 0L,
        usedBackupHeader = values[7] != 0L,
    )
}

internal fun NativeVolumeInfo.resolvedCipherHint(): CipherHint = when (cipherCode) {
    1 -> CipherHint.AES
    2 -> CipherHint.SERPENT
    3 -> CipherHint.TWOFISH
    4 -> CipherHint.CAMELLIA
    5 -> CipherHint.KUZNYECHIK
    6 -> CipherHint.TWOFISH_AES
    7 -> CipherHint.SERPENT_TWOFISH_AES
    8 -> CipherHint.AES_SERPENT
    9 -> CipherHint.AES_TWOFISH_SERPENT
    10 -> CipherHint.SERPENT_TWOFISH
    11 -> CipherHint.KUZNYECHIK_CAMELLIA
    12 -> CipherHint.TWOFISH_KUZNYECHIK
    13 -> CipherHint.SERPENT_CAMELLIA
    14 -> CipherHint.AES_KUZNYECHIK
    15 -> CipherHint.CAMELLIA_SERPENT_KUZNYECHIK
    else -> error("Native volume returned an unresolved cipher code $cipherCode")
}

internal fun NativeVolumeInfo.resolvedKdfHint(): KdfHint = when (kdfCode) {
    1 -> KdfHint.PBKDF2_HMAC_SHA512
    2 -> KdfHint.PBKDF2_HMAC_SHA256
    3 -> KdfHint.PBKDF2_HMAC_BLAKE2S
    4 -> KdfHint.PBKDF2_HMAC_WHIRLPOOL
    5 -> KdfHint.PBKDF2_HMAC_STREEBOG
    6 -> KdfHint.ARGON2ID
    else -> error("Native volume returned an unresolved KDF code $kdfCode")
}
