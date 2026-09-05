/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.ftpserver

import java8.nio.file.Path
import java8.nio.file.LinkOption
import java8.nio.file.AccessMode
import java8.nio.file.NoSuchFileException
import java8.nio.file.StandardOpenOption
import java8.nio.file.attribute.FileTime
import java8.nio.file.attribute.PosixFileAttributeView
import me.zhanghai.android.files.provider.common.createDirectory
import me.zhanghai.android.files.provider.common.checkAccess
import me.zhanghai.android.files.provider.common.delete
import me.zhanghai.android.files.provider.common.getFileAttributeView
import me.zhanghai.android.files.provider.common.getLastModifiedTime
import me.zhanghai.android.files.provider.common.getOwner
import me.zhanghai.android.files.provider.common.isWritable
import me.zhanghai.android.files.provider.common.moveTo
import me.zhanghai.android.files.provider.common.newByteChannel
import me.zhanghai.android.files.provider.common.newDirectoryStream
import me.zhanghai.android.files.provider.common.newInputStream
import me.zhanghai.android.files.provider.common.newOutputStream
import me.zhanghai.android.files.provider.common.setLastModifiedTime
import me.zhanghai.android.files.provider.common.size
import org.apache.ftpserver.ftplet.FtpFile
import org.apache.ftpserver.ftplet.User
import org.apache.ftpserver.usermanager.impl.WriteRequest
import java.io.IOException
import java.io.FilterInputStream
import java.io.FilterOutputStream
import java.io.InputStream
import java.io.OutputStream
import java.nio.ByteBuffer
import me.zhanghai.android.files.provider.remote.RemoteFileSystemException

class ProviderFtpFile(
    private val path: Path,
    private val relativePath: Path,
    private val user: User,
    private val isSafePath: (Path) -> Boolean,
    private val onAccess: (Path) -> Unit,
) : Comparable<ProviderFtpFile>, FtpFile {
    override fun getAbsolutePath(): String {
        val path = relativePath.toString()
        return "/$path"
    }

    override fun getName(): String {
        val name = relativePath.fileName?.toString().orEmpty()
        return if (name.isNotEmpty()) name else "/"
    }

    override fun isHidden(): Boolean = false

    override fun isDirectory(): Boolean = touchIfSafe()
        && FtpPathPolicy.readAttributesIfExists(path)?.isDirectory == true

    override fun isFile(): Boolean = touchIfSafe()
        && FtpPathPolicy.readAttributesIfExists(path)?.isRegularFile == true

    override fun doesExist(): Boolean = touchIfSafe()
        && FtpPathPolicy.readAttributesIfExists(path) != null

    override fun isReadable(): Boolean = touchIfSafe() && hasAccess(AccessMode.READ)

    override fun isWritable(): Boolean {
        if (!touchIfSafe()) return false
        if (user.authorize(WriteRequest(absolutePath)) == null) {
            return false
        }
        return FtpPathPolicy.readAttributesIfExists(path) == null || hasAccess(AccessMode.WRITE)
    }

    override fun isRemovable(): Boolean {
        if (!touchIfSafe()) return false
        if (relativePath.isEmpty) {
            return false
        }
        if (user.authorize(WriteRequest(absolutePath)) == null) {
            return false
        }
        val parent = path.parent ?: return false
        return isSafePath(parent) && hasAccess(parent, AccessMode.WRITE)
    }

    override fun getOwnerName(): String =
        try {
            if (!touchIfSafe()) return "user"
            path.getOwner().name
        } catch (ignored: UnsupportedOperationException) {
            null
        } catch (e: RemoteFileSystemException) {
            throw e
        } catch (e: IOException) {
            e.printStackTrace()
            null
        } ?: "user"

    override fun getGroupName(): String {
        if (!touchIfSafe()) return "group"
        val attributeView = path.getFileAttributeView(PosixFileAttributeView::class.java)
        return if (attributeView != null) {
            try {
                attributeView.readAttributes().group().name
            } catch (e: IOException) {
                if (e is RemoteFileSystemException) throw e
                e.printStackTrace()
                null
            }
        } else {
            null
        } ?: "group"
    }

    override fun getLinkCount(): Int = if (isDirectory) 3 else 1

    override fun getLastModified(): Long {
        if (!touchIfSafe()) return 0
        return try {
            path.getLastModifiedTime().toMillis()
        } catch (e: RemoteFileSystemException) {
            throw e
        } catch (e: IOException) {
            e.printStackTrace()
            0
        }
    }

    override fun setLastModified(time: Long): Boolean =
        if (!isWritable) {
            false
        } else {
            try {
                path.setLastModifiedTime(FileTime.fromMillis(time))
                true
            } catch (e: RemoteFileSystemException) {
                throw e
            } catch (e: IOException) {
                e.printStackTrace()
                false
            }
        }

    override fun getSize(): Long {
        if (!touchIfSafe()) return 0
        return try {
            path.size()
        } catch (e: RemoteFileSystemException) {
            throw e
        } catch (e: IOException) {
            e.printStackTrace()
            0
        }
    }

    override fun getPhysicalFile(): Path = path

    override fun mkdir(): Boolean =
        if (!isWritable) {
            false
        } else {
            try {
                path.createDirectory()
                true
            } catch (e: RemoteFileSystemException) {
                throw e
            } catch (e: IOException) {
                e.printStackTrace()
                false
            }
        }

    override fun delete(): Boolean =
        if (!isRemovable) {
            false
        } else {
            try {
                path.delete()
                true
            } catch (e: RemoteFileSystemException) {
                throw e
            } catch (e: IOException) {
                e.printStackTrace()
                false
            }
        }

    override fun move(destination: FtpFile): Boolean {
        if (destination !is ProviderFtpFile || !isRemovable || !destination.isWritable) {
            return false
        }
        val targetPath = destination.path
        if (!isSafePath(targetPath)) return false
        return try {
            onAccess(path)
            onAccess(targetPath)
            path.moveTo(targetPath)
            true
        } catch (e: RemoteFileSystemException) {
            throw e
        } catch (e: IOException) {
            e.printStackTrace()
            false
        }
    }

    override fun listFiles(): List<ProviderFtpFile>? {
        val directoryStream = try {
            if (!touchIfSafe()) return null
            path.newDirectoryStream()
        } catch (e: IOException) {
            if (e is RemoteFileSystemException) throw e
            e.printStackTrace()
            return null
        }
        return try {
            directoryStream.mapNotNull {
                // Providers normally return absolute children. Use only the
                // final name when constructing the FTP-relative path so a
                // root listing never turns `/child` into `//child`.
                val childName = it.fileName ?: return@mapNotNull null
                val childPath = path.resolve(childName).normalize()
                if (!isSafePath(childPath)) {
                    null
                } else {
                    ProviderFtpFile(
                        childPath,
                        relativePath.resolve(childName).normalize(),
                        user,
                        isSafePath,
                        onAccess,
                    )
                }
            }.sorted()
        } finally {
            runCatching { directoryStream.close() }
        }
    }

    @Throws(IOException::class)
    override fun createOutputStream(offset: Long): OutputStream {
        if (offset < 0 || !isWritable || !isWritableTarget()) {
            throw IOException("Not writable: $absolutePath")
        }
        return if (offset == 0L) {
            path.newOutputStream().withAccess()
        } else {
            val channel = path.newByteChannel(StandardOpenOption.WRITE)
            var successful = false
            try {
                val size = channel.size()
                if (offset <= size) {
                    if (offset < size) {
                        channel.truncate(offset)
                    }
                    channel.position(offset)
                } else {
                    channel.position(offset - 1)
                    channel.write(ByteBuffer.allocate(1))
                }
                val outputStream = channel.newOutputStream()
                successful = true
                outputStream.withAccess()
            } finally {
                if (!successful) {
                    channel.close()
                }
            }
        }
    }

    @Throws(IOException::class)
    override fun createInputStream(offset: Long): InputStream {
        if (offset < 0 || !touchIfSafe()) {
            throw IOException("Not readable: $absolutePath")
        }
        return if (offset == 0L) {
            path.newInputStream().withAccess()
        } else {
            val channel = path.newByteChannel()
            var successful = false
            try {
                channel.position(offset)
                val inputStream = channel.newInputStream()
                successful = true
                inputStream.withAccess()
            } finally {
                if (!successful) {
                    channel.close()
                }
            }
        }
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (javaClass != other?.javaClass) {
            return false
        }
        other as ProviderFtpFile
        return path == other.path
    }

    override fun hashCode(): Int = path.hashCode()

    override fun compareTo(other: ProviderFtpFile): Int = path.compareTo(other.path)

    private fun touchIfSafe(): Boolean {
        if (!isSafePath(path)) return false
        onAccess(path)
        return true
    }

    /** FTP must never turn a special node (for example /dev/block/*) into a
     * writable data sink, even when the selected Root strategy grants access. */
    private fun isWritableTarget(): Boolean =
        FtpPathPolicy.readAttributesIfExists(path)?.isRegularFile != false

    private fun hasAccess(mode: AccessMode): Boolean = hasAccess(path, mode)

    private fun hasAccess(target: Path, mode: AccessMode): Boolean = try {
        target.checkAccess(mode)
        true
    } catch (_: NoSuchFileException) {
        false
    } catch (e: RemoteFileSystemException) {
        throw e
    } catch (_: IOException) {
        false
    }

    /** Keeps an unlocked-volume session alive throughout a long transfer. */
    private fun InputStream.withAccess(): InputStream = object : FilterInputStream(this) {
        override fun read(): Int {
            onAccess(path)
            return super.read()
        }

        override fun read(buffer: ByteArray, offset: Int, length: Int): Int {
            onAccess(path)
            return super.read(buffer, offset, length)
        }

        override fun skip(count: Long): Long {
            onAccess(path)
            return super.skip(count)
        }
    }

    private fun OutputStream.withAccess(): OutputStream = object : FilterOutputStream(this) {
        override fun write(value: Int) {
            onAccess(path)
            super.write(value)
        }

        override fun write(buffer: ByteArray, offset: Int, length: Int) {
            onAccess(path)
            super.write(buffer, offset, length)
        }

        override fun flush() {
            onAccess(path)
            super.flush()
        }
    }
}
