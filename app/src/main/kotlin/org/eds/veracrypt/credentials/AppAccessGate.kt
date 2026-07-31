package org.eds.veracrypt.credentials

import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyPermanentlyInvalidatedException
import android.security.keystore.KeyProperties
import java.security.KeyStore
import javax.crypto.Cipher
import javax.crypto.SecretKey

/**
 * Per-use Android Keystore proof used only to unlock the app's foreground UI.
 * It stores no volume credential and can be recreated after biometric changes.
 */
internal class AppAccessGate {

    fun cipherForAuthentication(): Cipher = try {
        initializedCipher(getOrCreateKey())
    } catch (_: KeyPermanentlyInvalidatedException) {
        deleteKey()
        initializedCipher(getOrCreateKey())
    }

    private fun initializedCipher(key: SecretKey): Cipher = Cipher.getInstance(TRANSFORMATION).apply {
        init(Cipher.ENCRYPT_MODE, key)
    }

    private fun getOrCreateKey(): SecretKey {
        val keyStore = KeyStore.getInstance(ANDROID_KEY_STORE).apply { load(null) }
        (keyStore.getKey(KEY_ALIAS, null) as? SecretKey)?.let { return it }
        val generator = javax.crypto.KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, ANDROID_KEY_STORE)
        generator.init(
            KeyGenParameterSpec.Builder(KEY_ALIAS, KeyProperties.PURPOSE_ENCRYPT)
                .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
                .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                .setKeySize(256)
                .setUserAuthenticationRequired(true)
                .setUserAuthenticationParameters(
                    0,
                    KeyProperties.AUTH_BIOMETRIC_STRONG or KeyProperties.AUTH_DEVICE_CREDENTIAL,
                )
                // Adding a new biometric still requires device credentials on
                // Android; do not turn a non-secret UI lock into a permanent
                // failure after an enrollment change.
                .setInvalidatedByBiometricEnrollment(false)
                .build(),
        )
        return generator.generateKey()
    }

    private fun deleteKey() {
        KeyStore.getInstance(ANDROID_KEY_STORE).apply {
            load(null)
            deleteEntry(KEY_ALIAS)
        }
    }

    private companion object {
        const val ANDROID_KEY_STORE = "AndroidKeyStore"
        const val KEY_ALIAS = "org.eds.veracrypt.app-access.v1"
        const val TRANSFORMATION = "AES/GCM/NoPadding"
    }
}
