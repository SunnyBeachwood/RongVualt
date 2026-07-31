package org.eds.veracrypt.nativecore

import org.eds.veracrypt.domain.VolumeError

/** Native failures carry no key, password, URI, path, or header bytes. */
internal class VcCoreFailure(val code: Int, message: String) : RuntimeException(message) {
    fun asVolumeError(cause: Throwable = this): VolumeError = when (code) {
        INVALID_CREDENTIALS_OR_FORMAT -> VolumeError.InvalidCredentialsOrFormat()
        CORRUPT_HEADER -> VolumeError.CorruptHeader()
        UNSUPPORTED_ALGORITHM -> VolumeError.UnsupportedAlgorithm()
        UNSUPPORTED_FILESYSTEM -> VolumeError.UnsupportedFileSystem()
        INSUFFICIENT_MEMORY -> VolumeError.InsufficientMemory()
        SOURCE_NOT_SEEKABLE -> VolumeError.SourceNotSeekable()
        READ_ONLY_SOURCE -> VolumeError.ReadOnlySource()
        HIDDEN_VOLUME_RISK -> VolumeError.HiddenVolumeRisk()
        CANCELLED -> VolumeError.Cancelled()
        NOT_FOUND -> VolumeError.NotFound()
        else -> VolumeError.IoInterrupted(cause)
    }

    companion object {
        const val INVALID_CREDENTIALS_OR_FORMAT = 1
        const val CORRUPT_HEADER = 2
        const val UNSUPPORTED_ALGORITHM = 3
        const val UNSUPPORTED_FILESYSTEM = 4
        const val INSUFFICIENT_MEMORY = 5
        const val SOURCE_NOT_SEEKABLE = 6
        const val READ_ONLY_SOURCE = 7
        const val HIDDEN_VOLUME_RISK = 8
        const val CANCELLED = 9
        const val IO_INTERRUPTED = 10
        const val NOT_FOUND = 11
    }
}
