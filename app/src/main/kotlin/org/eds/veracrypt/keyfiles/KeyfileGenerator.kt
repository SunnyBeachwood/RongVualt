package org.eds.veracrypt.keyfiles

import android.content.ContentResolver
import android.net.Uri
import android.os.ParcelFileDescriptor
import java.io.IOException
import java.security.SecureRandom
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.eds.veracrypt.domain.KeyfileSource
import org.eds.veracrypt.domain.VolumeError

/** Generates a user-selected SAF keyfile without retaining its random bytes. */
class KeyfileGenerator(
    private val contentResolver: ContentResolver,
    private val secureRandom: SecureRandom = SecureRandom(),
) {
    suspend fun generate(destination: Uri, sizeBytes: Int = DEFAULT_SIZE_BYTES): KeyfileSource = withContext(Dispatchers.IO) {
        require(destination.scheme == "content") { "Keyfiles must be created through Storage Access Framework" }
        require(sizeBytes in 1..MAX_SIZE_BYTES) { "Invalid keyfile size" }
        val descriptor = try {
            contentResolver.openFileDescriptor(destination, "wt")
                ?: throw VolumeError.IoInterrupted(IOException("Could not open keyfile destination"))
        } catch (error: SecurityException) {
            throw VolumeError.IoInterrupted(error)
        } catch (error: IOException) {
            throw VolumeError.IoInterrupted(error)
        }

        val buffer = ByteArray(minOf(BUFFER_SIZE_BYTES, sizeBytes))
        try {
            ParcelFileDescriptor.AutoCloseOutputStream(descriptor).use { output ->
                var remaining = sizeBytes
                while (remaining > 0) {
                    val chunk = minOf(buffer.size, remaining)
                    secureRandom.nextBytes(buffer)
                    output.write(buffer, 0, chunk)
                    remaining -= chunk
                }
                output.flush()
            }
        } catch (error: IOException) {
            throw VolumeError.IoInterrupted(error)
        } finally {
            buffer.fill(0)
        }
        KeyfileSource(destination.toString())
    }

    private companion object {
        const val DEFAULT_SIZE_BYTES = 64 * 1024
        const val MAX_SIZE_BYTES = 16 * 1024 * 1024
        const val BUFFER_SIZE_BYTES = 32 * 1024
    }
}
