package org.eds.veracrypt.credentials

import android.content.Context
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Base64
import androidx.fragment.app.FragmentActivity
import java.security.KeyStore
import java.security.InvalidKeyException
import android.security.keystore.KeyPermanentlyInvalidatedException
import javax.crypto.Cipher
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.AEADBadTagException

/**
 * Opt-in persistent credential storage.
 *
 * Call [saveAuthorized] and [loadAuthorized] for the prompt-bound workflow,
 * or present [cipherForEncryption] / [cipherForDecryption] yourself before
 * calling [save] / [load]. This class deliberately owns neither a password
 * nor a long-lived decrypted credential buffer.
 */
class CredentialVault(context: Context) {
    private val preferences = context.applicationContext.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE)

    fun cipherForEncryption(): Cipher = try {
        Cipher.getInstance(TRANSFORMATION).apply { init(Cipher.ENCRYPT_MODE, getOrCreateKey(V2_KEY_ALIAS, false)) }
    } catch (_: KeyPermanentlyInvalidatedException) {
        removeV2RecordsAndKey()
        Cipher.getInstance(TRANSFORMATION).apply { init(Cipher.ENCRYPT_MODE, getOrCreateKey(V2_KEY_ALIAS, false)) }
    }

    fun cipherForDecryption(recordId: String): Cipher {
        val record = readRecord(recordId) ?: throw CredentialNotFoundException(recordId)
        return Cipher.getInstance(TRANSFORMATION).apply {
            init(Cipher.DECRYPT_MODE, keyFor(record.version), GCMParameterSpec(TAG_LENGTH_BITS, record.iv))
        }
    }

    /**
     * Prompt-authenticated save convenience for UI callers. [plaintext] is
     * wiped even when authorization is cancelled or the Keystore rejects it.
     */
    suspend fun saveAuthorized(
        activity: FragmentActivity,
        authorizer: BiometricCredentialAuthorizer,
        recordId: String,
        plaintext: ByteArray,
    ) {
        try {
            save(recordId, authorizer.authorize(activity, cipherForEncryption()), plaintext)
        } catch (error: Throwable) {
            plaintext.fill(0)
            throw error
        }
    }

    /**
     * Prompt-authenticated load. The returned secret must be cleared by the
     * immediate caller after turning it into a native request.
     */
    suspend fun loadAuthorized(
        activity: FragmentActivity,
        authorizer: BiometricCredentialAuthorizer,
        recordId: String,
    ): ByteArray = try {
        load(recordId, authorizer.authorize(activity, cipherForDecryption(recordId)))
    } catch (error: Throwable) {
        if (error is KeyPermanentlyInvalidatedException || error is AEADBadTagException) remove(recordId)
        throw error
    }

    /**
     * Writes an authenticated ciphertext produced by the prompt-authorized
     * cipher. [plaintext] is wiped after encryption whether the operation
     * succeeds or fails.
     */
    fun save(recordId: String, cipher: Cipher, plaintext: ByteArray) {
        require(recordId.matches(RECORD_ID)) { "Invalid credential record ID" }
        try {
            val ciphertext = cipher.doFinal(plaintext)
            val iv = cipher.iv ?: throw IllegalStateException("AES-GCM did not generate an IV")
            preferences.edit()
                .putString(keyFor(recordId), "$RECORD_VERSION:${encode(iv, ciphertext)}")
                .commit()
                .also { committed -> check(committed) { "Could not persist encrypted credentials" } }
        } finally {
            plaintext.fill(0)
        }
    }

    /**
     * Returns a short-lived decrypted buffer. The caller must wipe it in a
     * finally block after constructing the request passed to native code.
     */
    fun load(recordId: String, cipher: Cipher): ByteArray {
        val record = readRecord(recordId) ?: throw CredentialNotFoundException(recordId)
        return cipher.doFinal(record.ciphertext)
    }

    fun remove(recordId: String) {
        preferences.edit().remove(keyFor(recordId)).commit()
    }

    fun has(recordId: String): Boolean = readRecord(recordId) != null

    fun clearAll() {
        check(preferences.edit().clear().commit()) { "Could not clear saved credentials" }
        runCatching {
            KeyStore.getInstance(ANDROID_KEY_STORE).apply {
                load(null)
                if (containsAlias(V2_KEY_ALIAS)) deleteEntry(V2_KEY_ALIAS)
                if (containsAlias(LEGACY_KEY_ALIAS)) deleteEntry(LEGACY_KEY_ALIAS)
            }
        }
    }

    private fun readRecord(recordId: String): StoredRecord? {
        val encoded = preferences.getString(keyFor(recordId), null) ?: return null
        val pieces = encoded.split(':')
        val version: Int
        val ivIndex: Int
        when {
            pieces.size == 3 && pieces[0] == RECORD_VERSION -> {
                version = 2
                ivIndex = 1
            }
            pieces.size == 2 -> {
                version = 1
                ivIndex = 0
            }
            else -> return null
        }
        return try {
            StoredRecord(version, Base64.decode(pieces[ivIndex], Base64.NO_WRAP), Base64.decode(pieces[ivIndex + 1], Base64.NO_WRAP))
        } catch (_: IllegalArgumentException) {
            null
        }
    }

    private fun keyFor(version: Int): SecretKey = when (version) {
        1 -> getExistingKey(LEGACY_KEY_ALIAS)
        2 -> getOrCreateKey(V2_KEY_ALIAS, false)
        else -> throw IllegalStateException("Unsupported saved credential version")
    }

    private fun getExistingKey(alias: String): SecretKey {
        val keyStore = KeyStore.getInstance(ANDROID_KEY_STORE).apply { load(null) }
        return keyStore.getKey(alias, null) as? SecretKey
            ?: throw KeyPermanentlyInvalidatedException()
    }

    private fun getOrCreateKey(alias: String, invalidatedByEnrollment: Boolean): SecretKey {
        val keyStore = KeyStore.getInstance(ANDROID_KEY_STORE).apply { load(null) }
        (keyStore.getKey(alias, null) as? SecretKey)?.let { return it }

        val generator = javax.crypto.KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, ANDROID_KEY_STORE)
        generator.init(
            KeyGenParameterSpec.Builder(alias, KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT)
                .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
                .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                .setKeySize(256)
                .setUserAuthenticationRequired(true)
                .setUserAuthenticationParameters(
                    0,
                    KeyProperties.AUTH_BIOMETRIC_STRONG or KeyProperties.AUTH_DEVICE_CREDENTIAL,
                )
                .setInvalidatedByBiometricEnrollment(invalidatedByEnrollment)
                .build(),
        )
        return generator.generateKey()
    }

    private fun encode(iv: ByteArray, ciphertext: ByteArray): String =
        Base64.encodeToString(iv, Base64.NO_WRAP) + ':' + Base64.encodeToString(ciphertext, Base64.NO_WRAP)

    private fun keyFor(recordId: String) = "$RECORD_PREFIX$recordId"

    private fun removeV2RecordsAndKey() {
        preferences.all.keys.filter { key ->
            key.startsWith(RECORD_PREFIX) && preferences.getString(key, null)?.startsWith("$RECORD_VERSION:") == true
        }.forEach { preferences.edit().remove(it).commit() }
        KeyStore.getInstance(ANDROID_KEY_STORE).apply {
            load(null)
            deleteEntry(V2_KEY_ALIAS)
        }
    }

    private data class StoredRecord(val version: Int, val iv: ByteArray, val ciphertext: ByteArray)

    class CredentialNotFoundException(recordId: String) : IllegalStateException("No saved credentials for $recordId")

    companion object {
        private const val PREFERENCES = "veracrypt_credentials"
        private const val RECORD_PREFIX = "credential."
        private const val ANDROID_KEY_STORE = "AndroidKeyStore"
        private const val LEGACY_KEY_ALIAS = "org.eds.veracrypt.saved-credentials.v1"
        private const val V2_KEY_ALIAS = "org.eds.veracrypt.saved-credentials.v2"
        private const val RECORD_VERSION = "2"
        private const val TRANSFORMATION = "AES/GCM/NoPadding"
        private const val TAG_LENGTH_BITS = 128
        private val RECORD_ID = Regex("[A-Za-z0-9_-]{1,128}")
    }
}
