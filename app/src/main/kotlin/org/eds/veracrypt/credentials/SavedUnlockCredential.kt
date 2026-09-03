package org.eds.veracrypt.credentials

import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.io.Closeable
import java.io.DataInputStream
import java.io.DataOutputStream
import java.nio.ByteBuffer
import java.nio.charset.CodingErrorAction
import java.util.UUID
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.MAX_PIM_VALUE
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenTarget
import org.eds.veracrypt.domain.VolumeOpenOptions

/**
 * Short-lived plaintext package used only between a successful Android
 * authentication and JNI request construction. Keyfiles, hidden volumes and
 * hidden-volume protection credentials are deliberately unsupported here.
 */
internal class SavedUnlockCredential private constructor(
    private var passwordUtf8: ByteArray,
    private val pim: Int,
    private var cipher: CipherHint,
    private var kdf: KdfHint,
    private val volumeKind: VolumeKind,
    private val accessMode: VolumeAccessMode,
) : Closeable {
    fun encode(): ByteArray {
        check(passwordUtf8.isNotEmpty()) { "Saved credential has already been cleared" }
        check(cipher != CipherHint.AUTO && kdf != KdfHint.AUTO) { "Saved credential hints were not resolved" }
        return ByteArrayOutputStream().use { output ->
            DataOutputStream(output).use { data ->
                data.writeInt(FORMAT_VERSION)
                data.writeInt(passwordUtf8.size)
                data.write(passwordUtf8)
                data.writeInt(pim)
                data.writeUTF(cipher.name)
                data.writeUTF(kdf.name)
                data.writeUTF(volumeKind.name)
                // Stored biometric records always reopen through the automatic
                // policy. Version 1's concrete mode is read for compatibility
                // but must never influence a new unlock.
                data.writeUTF(VolumeAccessMode.AUTOMATIC.name)
            }
            output.toByteArray()
        }
    }

    fun takeOptions(): VolumeOpenOptions = VolumeOpenOptions(
        cipherHint = cipher,
        target = when (volumeKind) {
            VolumeKind.NORMAL -> VolumeOpenTarget.NORMAL
            VolumeKind.HIDDEN -> VolumeOpenTarget.HIDDEN
        },
        accessMode = VolumeAccessMode.AUTOMATIC,
    )

    fun resolve(cipher: CipherHint, kdf: KdfHint) {
        require(cipher != CipherHint.AUTO && kdf != KdfHint.AUTO) { "Resolved credential hints must be concrete" }
        this.cipher = cipher
        this.kdf = kdf
    }

    fun takeCredentials(): VolumeCredentials {
        val bytes = passwordUtf8
        check(bytes.isNotEmpty()) { "Saved credential has already been cleared" }
        passwordUtf8 = ByteArray(0)
        try {
            val chars = Charsets.UTF_8.newDecoder()
                .onMalformedInput(CodingErrorAction.REPORT)
                .onUnmappableCharacter(CodingErrorAction.REPORT)
                .decode(ByteBuffer.wrap(bytes))
                .let { buffer -> CharArray(buffer.remaining()).also(buffer::get) }
            return VolumeCredentials(SecretPassword(chars), pim = pim, kdfHint = kdf)
        } finally {
            bytes.fill(0)
        }
    }

    override fun close() {
        passwordUtf8.fill(0)
        passwordUtf8 = ByteArray(0)
    }

    companion object {
        fun capture(credentials: VolumeCredentials, options: VolumeOpenOptions): SavedUnlockCredential {
            require(credentials.keyfiles.isEmpty()) { "Keyfile-based unlock credentials are not saved" }
            require(options.target != VolumeOpenTarget.HIDDEN) { "Hidden-volume credentials are not saved" }
            require(options.hiddenVolumeProtection == null) { "Hidden-volume protection credentials are not saved" }
            return SavedUnlockCredential(
                credentials.password.copyForNativeUse(),
                credentials.pim,
                options.cipherHint,
                credentials.kdfHint,
                VolumeKind.NORMAL,
                options.accessMode,
            )
        }

        fun decode(payload: ByteArray): SavedUnlockCredential {
            var password: ByteArray? = null
            try {
                DataInputStream(ByteArrayInputStream(payload)).use { data ->
                    require(data.readInt() in LEGACY_FORMAT_VERSION..FORMAT_VERSION) { "Unsupported saved credential format" }
                    val size = data.readInt()
                    require(size in 1..MAX_PASSWORD_BYTES) { "Invalid saved password length" }
                    val passwordBytes = ByteArray(size)
                    password = passwordBytes
                    data.readFully(passwordBytes)
                    val pim = data.readInt()
                    require(pim in 0..MAX_PIM_VALUE) { "Invalid saved PIM" }
                    val cipher = CipherHint.valueOf(data.readUTF()).also { require(it != CipherHint.AUTO) }
                    val kdf = KdfHint.valueOf(data.readUTF()).also { require(it != KdfHint.AUTO) }
                    val volumeKind = VolumeKind.valueOf(data.readUTF())
                    require(volumeKind != VolumeKind.HIDDEN) { "Hidden-volume credentials are not supported" }
                    val credential = SavedUnlockCredential(
                        passwordBytes,
                        pim,
                        cipher,
                        kdf,
                        volumeKind,
                        VolumeAccessMode.valueOf(data.readUTF()) // consumed; legacy values map to AUTO below.
                            .let { VolumeAccessMode.AUTOMATIC },
                    )
                    require(data.available() == 0) { "Unexpected trailing saved credential data" }
                    password = null
                    return credential
                }
            } finally {
                password?.fill(0)
                payload.fill(0)
            }
        }

        fun recordId(containerId: UUID): String = "unlock-${containerId}"

        private const val LEGACY_FORMAT_VERSION = 1
        private const val FORMAT_VERSION = 2
        private const val MAX_PASSWORD_BYTES = 128
    }
}
