/*
 * Copyright (c) 2026
 */

package me.zhanghai.android.files.viewer.markdown

import android.net.Uri
import java8.nio.file.Path

/** Resolves only relative Markdown destinations against the displayed file. */
internal fun resolveMarkdownRelativePath(markdownFile: Path, destination: String): Path? {
    val trimmedDestination = destination.trim()
    if (trimmedDestination.isEmpty() || trimmedDestination.startsWith('#') ||
        trimmedDestination.startsWith("//")) {
        return null
    }
    val uri = Uri.parse(trimmedDestination)
    if (uri.scheme != null) {
        return null
    }
    val pathPart = trimmedDestination.substringBefore('#').substringBefore('?')
    if (pathPart.isEmpty() || pathPart.startsWith('/')) {
        return null
    }
    val decodedPath = Uri.decode(pathPart)
    if (decodedPath.indexOf('\u0000') >= 0) {
        return null
    }
    return markdownFile.parent?.resolve(decodedPath)?.normalize()
}

internal fun isMarkdownWebLink(destination: String): Boolean {
    val scheme = Uri.parse(destination.trim()).scheme?.lowercase()
    return scheme == "http" || scheme == "https"
}
