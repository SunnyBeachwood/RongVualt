/*
 * Copyright (c) 2026 EDS contributors.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

package me.zhanghai.android.files.security

import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Base64
import java.nio.charset.StandardCharsets
import java.security.KeyStore
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec

/**
 * Small process-safe credential store for network passwords and key passphrases.
 * Only an opaque reference is persisted in the network configuration; the
 * encrypted value and its IV live in a private preference file.
 */
object CredentialVault {
    private const val KEYSTORE = "AndroidKeyStore"
    private const val KEY_ALIAS = "eds.materialfiles.credentials.v1"
    private const val PREFS = "credential_vault"
    private const val IV_BYTES = 12
    private const val TAG_BITS = 128

    fun put(reference: String, value: CharSequence) {
        require(reference.isNotBlank()) { "Credential reference must not be blank" }
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.ENCRYPT_MODE, key())
        val ciphertext = cipher.doFinal(value.toString().toByteArray(StandardCharsets.UTF_8))
        prefs().edit()
            .putString(reference, Base64.encodeToString(cipher.iv + ciphertext, Base64.NO_WRAP))
            .apply()
    }

    fun get(reference: String): String? {
        val encoded = prefs().getString(reference, null) ?: return null
        return runCatching {
            val packed = Base64.decode(encoded, Base64.NO_WRAP)
            require(packed.size > IV_BYTES) { "Invalid credential record" }
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            cipher.init(
                Cipher.DECRYPT_MODE,
                key(),
                GCMParameterSpec(TAG_BITS, packed.copyOf(IV_BYTES))
            )
            String(cipher.doFinal(packed.copyOfRange(IV_BYTES, packed.size)), StandardCharsets.UTF_8)
        }.getOrNull()
    }

    fun remove(reference: String) {
        prefs().edit().remove(reference).apply()
    }

    private fun prefs() =
        me.zhanghai.android.files.app.application.getSharedPreferences(PREFS, 0)

    private fun key(): SecretKey {
        val store = KeyStore.getInstance(KEYSTORE).apply { load(null) }
        (store.getKey(KEY_ALIAS, null) as? SecretKey)?.let { return it }
        return KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, KEYSTORE).run {
            init(
                KeyGenParameterSpec.Builder(
                    KEY_ALIAS,
                    KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT
                ).setBlockModes(KeyProperties.BLOCK_MODE_GCM)
                    .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                    .setRandomizedEncryptionRequired(true)
                    .build()
            )
            generateKey()
        }
    }
}
