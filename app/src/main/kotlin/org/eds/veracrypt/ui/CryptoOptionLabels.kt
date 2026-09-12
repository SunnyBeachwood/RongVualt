package org.eds.veracrypt.ui

import android.content.Context
import com.sovworks.eds.android.R
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.MAX_PIM_VALUE
import org.eds.veracrypt.domain.VolumeFileSystem

/** Central capability lists used by every crypto-related screen. */
internal val openCipherHints: List<CipherHint> =
    listOf(CipherHint.AUTO) + CipherHint.entries.filter(CipherHint::isOpenable)

internal val creatableCipherHints: List<CipherHint> =
    CipherHint.entries.filter(CipherHint::isCreatable)

internal val openKdfHints: List<KdfHint> = KdfHint.entries

internal val creatableKdfHints: List<KdfHint> =
    KdfHint.entries.filter { it != KdfHint.AUTO }

internal fun CipherHint.label(context: Context): String = context.getString(
    when (this) {
        CipherHint.AUTO -> R.string.vc_cipher_auto
        CipherHint.AES -> R.string.vc_cipher_aes
        CipherHint.SERPENT -> R.string.vc_cipher_serpent
        CipherHint.TWOFISH -> R.string.vc_cipher_twofish
        CipherHint.CAMELLIA -> R.string.vc_cipher_camellia
        CipherHint.KUZNYECHIK -> R.string.vc_cipher_kuznyechik
        CipherHint.TWOFISH_AES -> R.string.vc_cipher_aes_twofish
        CipherHint.SERPENT_TWOFISH_AES -> R.string.vc_cipher_aes_twofish_serpent
        CipherHint.AES_SERPENT -> R.string.vc_cipher_serpent_aes
        CipherHint.AES_TWOFISH_SERPENT -> R.string.vc_cipher_serpent_twofish_aes
        CipherHint.SERPENT_TWOFISH -> R.string.vc_cipher_twofish_serpent
        CipherHint.KUZNYECHIK_CAMELLIA -> R.string.vc_cipher_camellia_kuznyechik
        CipherHint.TWOFISH_KUZNYECHIK -> R.string.vc_cipher_kuznyechik_twofish
        CipherHint.SERPENT_CAMELLIA -> R.string.vc_cipher_camellia_serpent
        CipherHint.AES_KUZNYECHIK -> R.string.vc_cipher_kuznyechik_aes
        CipherHint.CAMELLIA_SERPENT_KUZNYECHIK -> R.string.vc_cipher_kuznyechik_serpent_camellia
    },
)

internal fun KdfHint.label(context: Context): String = context.getString(
    when (this) {
        KdfHint.AUTO -> R.string.vc_kdf_auto
        KdfHint.PBKDF2_HMAC_SHA512 -> R.string.vc_kdf_pbkdf2_sha512
        KdfHint.PBKDF2_HMAC_SHA256 -> R.string.vc_kdf_pbkdf2_sha256
        KdfHint.PBKDF2_HMAC_BLAKE2S -> R.string.vc_kdf_pbkdf2_blake2s
        KdfHint.PBKDF2_HMAC_WHIRLPOOL -> R.string.vc_kdf_pbkdf2_whirlpool
        KdfHint.PBKDF2_HMAC_STREEBOG -> R.string.vc_kdf_pbkdf2_streebog
        KdfHint.ARGON2ID -> R.string.vc_kdf_argon2id
    },
)

internal fun CipherHint.description(context: Context): String = when (this) {
    CipherHint.AES -> context.getString(R.string.vc_cipher_description_aes)
    CipherHint.SERPENT -> context.getString(R.string.vc_cipher_description_serpent)
    CipherHint.TWOFISH -> context.getString(R.string.vc_cipher_description_twofish)
    CipherHint.CAMELLIA -> context.getString(R.string.vc_cipher_description_camellia)
    else -> context.getString(R.string.vc_cipher_description_cascade, label(context))
}

internal fun KdfHint.description(context: Context): String = when (this) {
    KdfHint.ARGON2ID -> context.getString(R.string.vc_kdf_description_argon2id)
    KdfHint.AUTO -> context.getString(R.string.vc_kdf_description_auto)
    else -> context.getString(R.string.vc_kdf_description_pbkdf2, label(context))
}

internal fun VolumeFileSystem.description(context: Context): String = context.getString(
    when (this) {
        VolumeFileSystem.EXFAT -> R.string.vc_filesystem_description_exfat
        VolumeFileSystem.FAT -> R.string.vc_filesystem_description_fat
        VolumeFileSystem.NTFS -> R.string.vc_filesystem_description_ntfs
    }
)

internal fun parsePim(raw: String?): Int? {
    val text = raw?.trim()
    if (text.isNullOrEmpty()) return 0
    val value = text.toLongOrNull()
    return if (value != null && value in 0..MAX_PIM_VALUE.toLong()) {
        value.toInt()
    } else {
        null
    }
}

internal fun KdfHint.parametersDescription(context: Context, pim: Int): String? {
    if (this != KdfHint.ARGON2ID) return null
    require(pim in 0..MAX_PIM_VALUE) { "PIM must be between 0 and $MAX_PIM_VALUE" }
    val effectivePim = if (pim == 0) 12 else pim
    val memoryMiB = minOf(64L + (effectivePim.toLong() - 1L) * 32L, 1024L)
    val iterations = if (effectivePim <= 31) {
        3 + (effectivePim - 1) / 3
    } else {
        13 + (effectivePim - 31)
    }
    return context.getString(R.string.vc_argon2_parameters, memoryMiB, iterations)
}
