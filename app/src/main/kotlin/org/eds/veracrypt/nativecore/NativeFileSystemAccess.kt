package org.eds.veracrypt.nativecore

import java.io.Closeable
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeError

/**
 * Internal, path-relative filesystem contract over an opaque native volume.
 * Callers never receive a native session handle, a host path, or key material.
 */
interface NativeFileSystemAccess {
    val accessMode: VolumeAccessMode
    /** Filesystem-level read-only state, e.g. an NTFS reader over a writable container. */
    val isReadOnly: Boolean
    fun list(relativePath: String): List<NativeFileEntry>
    fun stat(relativePath: String): NativeFileEntry
    fun openFile(relativePath: String, writable: Boolean, create: Boolean = false, truncate: Boolean = false): NativeOpenFile
    fun createDirectory(relativePath: String)
    fun delete(relativePath: String)
    fun rename(fromRelativePath: String, toRelativePath: String)
}

/** A single native filesystem file handle, invalidated when its volume closes. */
class NativeOpenFile internal constructor(
    private var handle: Long,
    private val onHiddenVolumeRisk: () -> Unit = {},
) : Closeable {
    private val lock = Any()

    fun read(offset: Long, target: ByteArray, targetOffset: Int = 0, length: Int = target.size - targetOffset): Int =
        useHandle { VcCore.nativeReadFile(it, offset, target, targetOffset, length) }

    fun write(offset: Long, source: ByteArray, sourceOffset: Int = 0, length: Int = source.size - sourceOffset): Int =
        useHandle { VcCore.nativeWriteFile(it, offset, source, sourceOffset, length) }

    fun truncate(length: Long) = useHandle { VcCore.nativeTruncateFile(it, length) }

    fun preallocate(length: Long) = useHandle { VcCore.nativePreallocateFile(it, length) }

    fun flush() = useHandle { VcCore.nativeFlushFile(it) }

    override fun close() {
        val closing = synchronized(lock) {
            val current = handle
            handle = 0
            current
        }
        if (closing != 0L) VcCore.nativeCloseFile(closing)
    }

    private inline fun <T> useHandle(block: (Long) -> T): T {
        val current = synchronized(lock) { check(handle != 0L) { "File is closed" }; handle }
        return try {
            block(current)
        } catch (failure: VcCoreFailure) {
            val error = failure.asVolumeError()
            if (error is VolumeError.HiddenVolumeRisk) onHiddenVolumeRisk()
            throw error
        }
    }
}

internal inline fun <T> nativeFileOperation(block: () -> T): T = try {
    block()
} catch (failure: VcCoreFailure) {
    throw failure.asVolumeError()
} catch (failure: IllegalArgumentException) {
    throw VolumeError.IoInterrupted(failure)
}
