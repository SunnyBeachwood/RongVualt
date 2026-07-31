package org.eds.veracrypt.storage

import android.content.ContentResolver
import android.net.Uri
import android.os.ParcelFileDescriptor
import android.system.Os
import android.system.OsConstants
import java.io.Closeable
import java.io.IOException
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeError

/**
 * A duplicated SAF descriptor accepted by the native layer. The native side
 * receives only [fd], not the source URI or a host filesystem path.
 */
class SeekableContainer private constructor(
    private val descriptor: ParcelFileDescriptor,
    val sizeBytes: Long,
    val accessMode: VolumeAccessMode,
) : Closeable {
    val fd: Int get() = descriptor.fd

    override fun close() = descriptor.close()

    companion object {
        @Throws(VolumeError::class)
        fun open(resolver: ContentResolver, uri: Uri, accessMode: VolumeAccessMode): SeekableContainer {
            require(accessMode != VolumeAccessMode.AUTOMATIC) { "Container descriptors require a concrete access mode" }
            val mode = if (accessMode == VolumeAccessMode.READ_WRITE) "rw" else "r"
            val opened = try {
                resolver.openFileDescriptor(uri, mode)
                    ?: throw VolumeError.SourceNotSeekable()
            } catch (error: SecurityException) {
                throw VolumeError.IoInterrupted(error)
            } catch (error: IOException) {
                if (accessMode == VolumeAccessMode.READ_WRITE) throw VolumeError.ReadOnlySource()
                throw VolumeError.IoInterrupted(error)
            }

            try {
                val duplicate = ParcelFileDescriptor.dup(opened.fileDescriptor)
                val size = Os.fstat(duplicate.fileDescriptor).st_size
                if (size < 0L) throw VolumeError.SourceNotSeekable()
                // Pipes and sockets cannot meet the native pread/pwrite contract.
                Os.lseek(duplicate.fileDescriptor, 0L, OsConstants.SEEK_CUR)
                return SeekableContainer(duplicate, size, accessMode)
            } catch (error: VolumeError) {
                throw error
            } catch (error: Exception) {
                throw VolumeError.SourceNotSeekable()
            } finally {
                opened.close()
            }
        }
    }
}
