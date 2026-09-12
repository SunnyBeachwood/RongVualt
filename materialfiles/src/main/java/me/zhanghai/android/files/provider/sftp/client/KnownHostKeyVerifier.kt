package me.zhanghai.android.files.provider.sftp.client

import android.util.Base64
import java.security.MessageDigest
import java.security.PublicKey
import java.util.Locale
import me.zhanghai.android.files.app.application
import net.schmizz.sshj.transport.verification.HostKeyVerifier

/** Pins the first SSH host key and rejects unexpected later key changes. */
internal class KnownHostKeyVerifier : HostKeyVerifier {
    override fun verify(hostname: String, port: Int, key: PublicKey): Boolean {
        val id = "${hostname.lowercase(Locale.ROOT)}:$port"
        val fingerprint = Base64.encodeToString(
            MessageDigest.getInstance("SHA-256").digest(key.encoded), Base64.NO_WRAP,
        )
        val preferences = application.getSharedPreferences(PREFERENCES, 0)
        val known = preferences.getString(id, null)
        if (known != null) return MessageDigest.isEqual(known.toByteArray(), fingerprint.toByteArray())
        return preferences.edit().putString(id, fingerprint).commit()
    }

    override fun findExistingAlgorithms(hostname: String, port: Int): List<String> = emptyList()

    private companion object {
        const val PREFERENCES = "rongvault_sftp_host_keys"
    }
}
