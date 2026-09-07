/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package me.zhanghai.android.files.ftpserver

import java8.nio.file.LinkOption
import java8.nio.file.NoSuchFileException
import java8.nio.file.Path
import java8.nio.file.attribute.BasicFileAttributes
import me.zhanghai.android.files.provider.common.readAttributes

/**
 * Keeps every FTP path below its selected home, including when a provider
 * exposes POSIX symbolic links. This policy is intentionally independent from
 * Apache FtpServer so it can be checked before any provider operation.
 */
internal object FtpPathPolicy {
    /**
     * Reads lstat-style attributes without converting provider failures into a
     * false "missing" result. In particular, a RootService denial or binder
     * death must reach the FTP caller instead of silently looking like a
     * deleted path.
     */
    fun readAttributesIfExists(path: Path): BasicFileAttributes? = try {
        path.readAttributes(BasicFileAttributes::class.java, LinkOption.NOFOLLOW_LINKS)
    } catch (_: NoSuchFileException) {
        null
    }

    fun isSafe(root: Path, path: Path): Boolean = isSafe(root, path) {
        readAttributesIfExists(it)?.isSymbolicLink
    }

    internal fun isSafe(
        root: Path,
        path: Path,
        isSymbolicLink: (Path) -> Boolean?,
    ): Boolean {
        val normalizedRoot = runCatching { root.normalize() }.getOrNull() ?: return false
        val normalizedPath = runCatching { path.normalize() }.getOrNull() ?: return false
        if (!runCatching { normalizedPath.startsWith(normalizedRoot) }.getOrDefault(false)) return false
        if (isSymbolicLink(normalizedRoot) == true) return false
        var current: Path? = normalizedPath
        while (current != null && current != normalizedRoot) {
            val candidate = current
            if (isSymbolicLink(candidate) == true) return false
            current = candidate.parent
        }
        return current == normalizedRoot
    }
}
