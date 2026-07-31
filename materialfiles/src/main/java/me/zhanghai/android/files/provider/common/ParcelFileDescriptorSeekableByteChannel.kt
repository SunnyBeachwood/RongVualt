/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.provider.common

import android.os.ParcelFileDescriptor
import android.system.ErrnoException
import android.system.Os
import java8.nio.channels.SeekableByteChannel
import java.io.IOException
import java.nio.ByteBuffer
import java.nio.channels.ClosedChannelException
import java.nio.channels.NonReadableChannelException
import java.nio.channels.NonWritableChannelException

/**
 * Public-API seekable channel for proxy file descriptors.
 *
 * Android removed the hidden NioUtils.newFileChannel() bridge used by the
 * upstream document provider. Proxy descriptors already implement positional
 * I/O, truncation and fsync, so expose those operations directly.
 */
class ParcelFileDescriptorSeekableByteChannel(
    private val descriptor: ParcelFileDescriptor,
    private val readable: Boolean,
    private val writable: Boolean,
    private val syncWrites: Boolean = false,
) : SeekableByteChannel, ForceableChannel {
    private var open = true
    private var offset = 0L

    override fun read(destination: ByteBuffer): Int {
        ensureOpen()
        if (!readable) throw NonReadableChannelException()
        if (!destination.hasRemaining()) return 0
        val bytes = ByteArray(destination.remaining())
        val read = posix("pread") {
            Os.pread(descriptor.fileDescriptor, bytes, 0, bytes.size, offset)
        }
        if (read == 0) return -1
        destination.put(bytes, 0, read)
        offset += read
        return read
    }

    override fun write(source: ByteBuffer): Int {
        ensureOpen()
        if (!writable) throw NonWritableChannelException()
        if (!source.hasRemaining()) return 0
        val bytes = ByteArray(source.remaining())
        source.duplicate().get(bytes)
        val written = posix("pwrite") {
            Os.pwrite(descriptor.fileDescriptor, bytes, 0, bytes.size, offset)
        }
        source.position(source.position() + written)
        offset += written
        if (syncWrites) force(false)
        return written
    }

    override fun position(): Long {
        ensureOpen()
        return offset
    }

    override fun position(newPosition: Long): SeekableByteChannel {
        ensureOpen()
        require(newPosition >= 0L) { "Negative channel position" }
        offset = newPosition
        return this
    }

    override fun size(): Long {
        ensureOpen()
        return posix("fstat") { Os.fstat(descriptor.fileDescriptor).st_size }
    }

    override fun truncate(size: Long): SeekableByteChannel {
        ensureOpen()
        if (!writable) throw NonWritableChannelException()
        require(size >= 0L) { "Negative channel size" }
        posix("ftruncate") { Os.ftruncate(descriptor.fileDescriptor, size) }
        if (offset > size) offset = size
        return this
    }

    override fun force(metaData: Boolean) {
        ensureOpen()
        if (!writable) return
        posix("fsync") { Os.fsync(descriptor.fileDescriptor) }
    }

    override fun isOpen(): Boolean = open

    override fun close() {
        if (!open) return
        var failure: IOException? = null
        if (writable) {
            try {
                posix("fsync") { Os.fsync(descriptor.fileDescriptor) }
            } catch (error: IOException) {
                failure = error
            }
        }
        open = false
        try {
            descriptor.close()
        } catch (error: IOException) {
            if (failure == null) failure = error else failure.addSuppressed(error)
        }
        failure?.let { throw it }
    }

    private fun ensureOpen() {
        if (!open) throw ClosedChannelException()
    }

    private inline fun <T> posix(operation: String, block: () -> T): T =
        try {
            block()
        } catch (error: ErrnoException) {
            throw IOException("$operation failed", error)
        }
}
