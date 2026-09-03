package org.eds.veracrypt.credentials

import java.io.ByteArrayOutputStream
import java.io.DataOutputStream
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.KeyfileSource
import org.eds.veracrypt.domain.MAX_PIM_VALUE
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.junit.Test
import java.util.UUID

class SavedUnlockCredentialTest {
    @Test
    fun roundTripAlwaysRestoresAutomaticAccessPolicy() {
        val credentials = VolumeCredentials(
            password = SecretPassword("correct horse battery staple".toCharArray()),
            pim = 17,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        val options = VolumeOpenOptions(
            cipherHint = CipherHint.AES,
            volumeKind = VolumeKind.NORMAL,
            accessMode = VolumeAccessMode.READ_WRITE,
        )
        val restored = SavedUnlockCredential.decode(
            SavedUnlockCredential.capture(credentials, options).encode(),
        )
        credentials.close()

        try {
            check(restored.takeOptions() == options.copy(accessMode = VolumeAccessMode.AUTOMATIC))
            val restoredCredentials = restored.takeCredentials()
            try {
                check(restoredCredentials.pim == 17)
                check(restoredCredentials.kdfHint == KdfHint.PBKDF2_HMAC_SHA512)
                check(
                    restoredCredentials.password.copyForNativeUse()
                        .contentEquals("correct horse battery staple".encodeToByteArray()),
                )
            } finally {
                restoredCredentials.close()
            }
        } finally {
            restored.close()
        }
    }

    @Test
    fun keyfileAndHiddenProtectionCredentialsAreNeverCapturable() {
        val keyfileCredentials = VolumeCredentials(
            password = SecretPassword("password".toCharArray()),
            keyfiles = listOf(KeyfileSource("content://keyfile")),
        )
        try {
            check(runCatching {
                SavedUnlockCredential.capture(keyfileCredentials, VolumeOpenOptions())
            }.isFailure)
        } finally {
            keyfileCredentials.close()
        }

        val protection = VolumeCredentials(SecretPassword("hidden".toCharArray()))
        val outerCredentials = VolumeCredentials(SecretPassword("outer".toCharArray()))
        try {
            check(runCatching {
                SavedUnlockCredential.capture(
                    outerCredentials,
                    VolumeOpenOptions(
                        accessMode = VolumeAccessMode.READ_WRITE,
                        hiddenVolumeProtection = protection,
                    ),
                )
            }.isFailure)
        } finally {
            outerCredentials.close()
            protection.close()
        }

        val hiddenCredentials = VolumeCredentials(SecretPassword("hidden".toCharArray()))
        try {
            check(runCatching {
                SavedUnlockCredential.capture(
                    hiddenCredentials,
                    VolumeOpenOptions(volumeKind = VolumeKind.HIDDEN),
                )
            }.isFailure)
        } finally {
            hiddenCredentials.close()
        }
    }

    @Test
    fun recordIdIsOpaqueAndContainerScoped() {
        val id = UUID.fromString("ee9292a8-bd8b-4436-9099-982405eac03a")
        check(SavedUnlockCredential.recordId(id) == "unlock-ee9292a8-bd8b-4436-9099-982405eac03a")
    }

    @Test
    fun persistedCredentialsRejectPimOutsideTheVeraCryptRange() {
        listOf(-1, MAX_PIM_VALUE + 1).forEach { invalidPim ->
            check(runCatching { SavedUnlockCredential.decode(encodedCredential(invalidPim)) }.isFailure)
        }
    }

    private fun encodedCredential(pim: Int): ByteArray = ByteArrayOutputStream().use { output ->
        DataOutputStream(output).use { data ->
            data.writeInt(2)
            data.writeInt(4)
            data.write("pass".encodeToByteArray())
            data.writeInt(pim)
            data.writeUTF(CipherHint.AES.name)
            data.writeUTF(KdfHint.PBKDF2_HMAC_SHA512.name)
            data.writeUTF(VolumeKind.NORMAL.name)
            data.writeUTF(VolumeAccessMode.AUTOMATIC.name)
        }
        output.toByteArray()
    }
}
