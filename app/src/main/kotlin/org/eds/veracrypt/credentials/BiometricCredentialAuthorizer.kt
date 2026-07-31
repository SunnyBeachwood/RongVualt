package org.eds.veracrypt.credentials

import androidx.biometric.BiometricManager
import androidx.biometric.BiometricPrompt
import androidx.core.content.ContextCompat
import androidx.fragment.app.FragmentActivity
import java.util.concurrent.CancellationException
import javax.crypto.Cipher
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlinx.coroutines.suspendCancellableCoroutine
import com.sovworks.eds.android.R

/**
 * Owns the Android prompt lifecycle for a [CredentialVault] cipher. It never
 * receives a password, keyfile path, or decrypted credential bytes.
 */
class BiometricCredentialAuthorizer {
    suspend fun authorize(
        activity: FragmentActivity,
        cipher: Cipher,
        promptTitle: String = activity.getString(R.string.vc_biometric_prompt_title),
    ): Cipher =
        suspendCancellableCoroutine { continuation ->
            val prompt = BiometricPrompt(
                activity,
                ContextCompat.getMainExecutor(activity),
                object : BiometricPrompt.AuthenticationCallback() {
                    override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult) {
                        val authorized = result.cryptoObject?.cipher
                        if (authorized == null) {
                            continuation.resumeWithException(BiometricAuthorizationException(BiometricPrompt.ERROR_UNABLE_TO_PROCESS, "No authorized cipher returned"))
                        } else if (continuation.isActive) {
                            continuation.resume(authorized)
                        }
                    }

                    override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
                        if (!continuation.isActive) return
                        when (errorCode) {
                            BiometricPrompt.ERROR_CANCELED,
                            BiometricPrompt.ERROR_NEGATIVE_BUTTON,
                            BiometricPrompt.ERROR_USER_CANCELED -> continuation.cancel(
                                CancellationException("Credential authorization was cancelled"),
                            )
                            else -> continuation.resumeWithException(BiometricAuthorizationException(errorCode, errString.toString()))
                        }
                    }
                },
            )
            continuation.invokeOnCancellation { prompt.cancelAuthentication() }
            prompt.authenticate(
                BiometricPrompt.PromptInfo.Builder()
                    .setAllowedAuthenticators(
                        BiometricManager.Authenticators.BIOMETRIC_STRONG or
                            BiometricManager.Authenticators.DEVICE_CREDENTIAL,
                    )
                    .setTitle(promptTitle)
                    .build(),
                BiometricPrompt.CryptoObject(cipher),
            )
        }
}

/** Authentication errors are deliberately preserved so callers do not delete a valid saved record. */
class BiometricAuthorizationException(val errorCode: Int, message: String) : IllegalStateException(message)
