package org.eds.veracrypt.domain

import org.junit.Test

class VolumeCreateOptionsTest {
    @Test
    fun persistedHintNamesAndCodesKeepTheV2Contract() {
        check(CipherHint.entries.map { it.name } == listOf(
            "AUTO", "AES", "SERPENT", "TWOFISH", "CAMELLIA", "KUZNYECHIK",
            "TWOFISH_AES", "SERPENT_TWOFISH_AES", "AES_SERPENT", "AES_TWOFISH_SERPENT",
            "SERPENT_TWOFISH", "KUZNYECHIK_CAMELLIA", "TWOFISH_KUZNYECHIK",
            "SERPENT_CAMELLIA", "AES_KUZNYECHIK", "CAMELLIA_SERPENT_KUZNYECHIK",
        ))
        check(KdfHint.entries.map { it.name } == listOf(
            "AUTO", "PBKDF2_HMAC_SHA512", "PBKDF2_HMAC_SHA256", "PBKDF2_HMAC_BLAKE2S",
            "PBKDF2_HMAC_WHIRLPOOL", "PBKDF2_HMAC_STREEBOG", "ARGON2ID",
        ))
        check(CipherHint.entries.map { it.displayName } == listOf(
            "Automatic", "AES", "Serpent", "Twofish", "Camellia", "Kuznyechik",
            "AES-Twofish", "AES-Twofish-Serpent", "Serpent-AES", "Serpent-Twofish-AES",
            "Twofish-Serpent", "Camellia-Kuznyechik", "Kuznyechik-Twofish",
            "Camellia-Serpent", "Kuznyechik-AES", "Kuznyechik-Serpent-Camellia",
        ))
        check(KdfHint.entries.map { it.displayName } == listOf(
            "Automatic", "PBKDF2-HMAC-SHA-512", "PBKDF2-HMAC-SHA-256",
            "PBKDF2-HMAC-BLAKE2s-256", "PBKDF2-HMAC-Whirlpool", "PBKDF2-HMAC-Streebog", "Argon2id",
        ))
        check(CipherHint.entries.mapIndexed { index, hint -> index to hint.ordinal }.all { it.first == it.second })
        check(KdfHint.entries.mapIndexed { index, hint -> index to hint.ordinal }.all { it.first == it.second })
    }

    @Test
    fun nineWindowsCreationSuitesAreEnabledAndSixKuznyechikSuitesRemainReadOnly() {
        val creatable = CipherHint.entries.filter { it.isCreatable }
        check(creatable == listOf(
            CipherHint.AES,
            CipherHint.SERPENT,
            CipherHint.TWOFISH,
            CipherHint.CAMELLIA,
            CipherHint.TWOFISH_AES,
            CipherHint.SERPENT_TWOFISH_AES,
            CipherHint.AES_SERPENT,
            CipherHint.AES_TWOFISH_SERPENT,
            CipherHint.SERPENT_TWOFISH,
        ))
        check(CipherHint.entries.count { it.isOpenable } == 15)
        check(CipherHint.entries.count { !it.isCreatable && it.isOpenable } == 6)
        creatable.forEach { cipher ->
            VolumeCreateOptions(
                sizeBytes = 1_048_576,
                volumeKind = VolumeKind.NORMAL,
                cipher = cipher,
            )
        }
    }

    @Test
    fun pimRangeMatchesVeraCryptMaximum() {
        VolumeCredentials(SecretPassword("pass".toCharArray()), pim = MAX_PIM_VALUE).close()
        assertInvalidCredentialsPim(-1)
        assertInvalidCredentialsPim(MAX_PIM_VALUE + 1)
        check(runCatching {
            VolumeCreateOptions(1_048_576, VolumeKind.NORMAL, pim = -1)
        }.isFailure)
        check(runCatching {
            VolumeCreateOptions(1_048_576, VolumeKind.NORMAL, pim = MAX_PIM_VALUE + 1)
        }.isFailure)
    }

    private fun assertInvalidCredentialsPim(pim: Int) {
        val password = SecretPassword("pass".toCharArray())
        try {
            VolumeCredentials(password, pim = pim)
            throw AssertionError("PIM $pim should be rejected")
        } catch (_: IllegalArgumentException) {
            // Expected.
        } finally {
            password.close()
        }
    }
}
