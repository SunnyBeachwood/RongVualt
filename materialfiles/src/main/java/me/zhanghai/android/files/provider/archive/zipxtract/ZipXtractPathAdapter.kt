/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 *
 * Provider adapter for the ZipXtract compatibility core.  The adapter keeps
 * archive engines independent of Material Files' Path implementation.
 */

package me.zhanghai.android.files.provider.archive.zipxtract

import java.io.InputStream
import java.io.OutputStream
import java.nio.ByteBuffer
import java.nio.channels.SeekableByteChannel
import java8.nio.file.LinkOption
import java8.nio.file.OpenOption
import java8.nio.file.Path
import me.zhanghai.android.files.provider.common.deleteIfExists
import me.zhanghai.android.files.provider.common.createDirectories
import me.zhanghai.android.files.provider.common.exists
import me.zhanghai.android.files.provider.common.isDirectory
import me.zhanghai.android.files.provider.common.newInputStream
import me.zhanghai.android.files.provider.common.newByteChannel
import me.zhanghai.android.files.provider.common.newOutputStream
import me.zhanghai.android.files.provider.common.readAttributes
import me.zhanghai.android.files.provider.common.size
import org.eds.zipxtract.core.ArchiveSource
import org.eds.zipxtract.core.ArchiveTarget
import org.eds.zipxtract.core.ArchivePathPolicy

class PathArchiveSource(
    val path: Path,
) : ArchiveSource {
    override val displayName: String
        get() = path.fileName?.toString() ?: path.toString()

    override val size: Long
        get() = runCatching { path.size(LinkOption.NOFOLLOW_LINKS) }.getOrDefault(0L)

    override fun openInputStream(): InputStream = path.newInputStream(LinkOption.NOFOLLOW_LINKS)

    override fun openSeekableByteChannel(): SeekableByteChannel? = runCatching {
        JavaSeekableByteChannel(
            path.newByteChannel(
                java8.nio.file.StandardOpenOption.READ,
                LinkOption.NOFOLLOW_LINKS,
            ),
        )
    }.getOrNull()

    override fun openSibling(name: String): ArchiveSource? {
        val sibling = path.resolveSibling(name)
        return sibling.takeIf { runCatching { it.exists(LinkOption.NOFOLLOW_LINKS) }.getOrDefault(false) }
            ?.let(::PathArchiveSource)
    }
}

class PathArchiveTarget(
    val path: Path,
) : ArchiveTarget {
    override val displayName: String
        get() = path.fileName?.toString() ?: path.toString()

    override fun exists(): Boolean = path.exists(LinkOption.NOFOLLOW_LINKS)

    override fun isDirectory(): Boolean = path.isDirectory(LinkOption.NOFOLLOW_LINKS)

    override fun createDirectory() {
        path.parent?.normalize()?.let { parent ->
            ensureNoSymbolicLink(parent, path.normalize())
        }
        path.createDirectories()
    }

    override fun openOutputStream(overwrite: Boolean): OutputStream {
        path.parent?.normalize()?.let { parent ->
            ensureNoSymbolicLink(parent, path.normalize())
        }
        val options = if (overwrite) {
            arrayOf<OpenOption>(
                java8.nio.file.StandardOpenOption.CREATE,
                java8.nio.file.StandardOpenOption.TRUNCATE_EXISTING,
                java8.nio.file.StandardOpenOption.WRITE,
                LinkOption.NOFOLLOW_LINKS,
            )
        } else {
            arrayOf<OpenOption>(
                java8.nio.file.StandardOpenOption.CREATE_NEW,
                java8.nio.file.StandardOpenOption.WRITE,
                LinkOption.NOFOLLOW_LINKS,
            )
        }
        return path.newOutputStream(*options)
    }

    override fun openSeekableByteChannel(overwrite: Boolean): SeekableByteChannel? = runCatching {
        path.parent?.normalize()?.let { parent ->
            ensureNoSymbolicLink(parent, path.normalize())
        }
        val options = if (overwrite) {
            arrayOf<OpenOption>(
                java8.nio.file.StandardOpenOption.CREATE,
                java8.nio.file.StandardOpenOption.TRUNCATE_EXISTING,
                java8.nio.file.StandardOpenOption.WRITE,
                LinkOption.NOFOLLOW_LINKS,
            )
        } else {
            arrayOf<OpenOption>(
                java8.nio.file.StandardOpenOption.CREATE_NEW,
                java8.nio.file.StandardOpenOption.WRITE,
                LinkOption.NOFOLLOW_LINKS,
            )
        }
        JavaSeekableByteChannel(path.newByteChannel(*options))
    }.getOrNull()

    override fun parent(): ArchiveTarget? = path.parent?.let(::PathArchiveTarget)

    override fun resolve(name: String): ArchiveTarget = PathArchiveTarget(safeResolve(name))

    override fun resolveSibling(name: String): ArchiveTarget =
        PathArchiveTarget(path.resolveSibling(ArchivePathPolicy.normalizeEntryName(name)).normalize())
            .also { sibling ->
                path.parent?.normalize()?.let { parent ->
                    sibling.ensureNoSymbolicLink(parent, sibling.path)
                }
            }

    override fun deleteIfExists() {
        path.deleteIfExists()
    }

    private fun safeResolve(name: String): Path {
        val normalizedName = ArchivePathPolicy.normalizeEntryName(name)
        val base = path.normalize()
        val candidate = path.resolve(normalizedName).normalize()
        if (!candidate.startsWith(base)) {
            throw IllegalArgumentException("Archive entry escapes destination: $name")
        }
        ensureNoSymbolicLink(base, candidate)
        return candidate
    }

    internal fun ensureNoSymbolicLink(base: Path, candidate: Path) {
        // Walk all ancestors, not only the newly-created suffix. A symlink
        // already present in the selected destination's parent would
        // otherwise redirect every extracted entry outside the user's chosen
        // tree before the base-path check ever sees it.
        var current: Path? = candidate
        while (current != null) {
            if (runCatching {
                    current.readAttributes(
                        java8.nio.file.attribute.BasicFileAttributes::class.java,
                        LinkOption.NOFOLLOW_LINKS,
                    ).isSymbolicLink
                }.getOrDefault(false)
            ) {
                throw IllegalArgumentException("Destination contains a symbolic link")
            }
            current = current.parent
        }
    }
}

/** Bridges Material Files' desugared NIO channel to the core's JDK-facing API. */
private class JavaSeekableByteChannel(
    private val delegate: java8.nio.channels.SeekableByteChannel,
) : SeekableByteChannel {
    override fun read(dst: ByteBuffer): Int = delegate.read(dst)

    override fun write(src: ByteBuffer): Int = delegate.write(src)

    override fun position(): Long = delegate.position()

    override fun position(newPosition: Long): SeekableByteChannel {
        delegate.position(newPosition)
        return this
    }

    override fun size(): Long = delegate.size()

    override fun truncate(size: Long): SeekableByteChannel {
        delegate.truncate(size)
        return this
    }

    override fun isOpen(): Boolean = delegate.isOpen

    override fun close() = delegate.close()
}

fun Path.asArchiveSource(): PathArchiveSource = PathArchiveSource(this)

fun Path.asArchiveTarget(): PathArchiveTarget = PathArchiveTarget(this)
