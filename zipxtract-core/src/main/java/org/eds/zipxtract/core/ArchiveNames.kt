/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 */

package org.eds.zipxtract.core

import java.util.Locale

/** User-facing names shared by extraction jobs and the core engine. */
object ArchiveNames {
    /**
     * Return the sibling directory used by “extract to same-name folder”.
     * Split and compound compression suffixes are removed as one unit.
     */
    fun containingDirectoryName(name: String, format: ArchiveFormat): String {
        val rawBase = name.substringAfterLast('/').substringAfterLast('\\')
        val primary = MultipartArchiveResolver.primaryName(rawBase, format)
        val lower = primary.lowercase(Locale.ROOT)
        val withoutSuffix = when {
            lower.matches(Regex(".+\\.zip\\.\\d{3}")) -> primary.dropLast(8)
            lower.matches(Regex(".+\\.zip\\.part\\d+")) ->
                primary.replace(Regex("(?i)\\.zip\\.part\\d+$"), "")
            lower.matches(Regex(".+\\.7z\\.\\d{3}")) -> primary.dropLast(7)
            lower.matches(Regex(".+\\.part\\d+\\.rar")) ->
                primary.replace(Regex("(?i)\\.part\\d+\\.rar$"), "")
            lower.matches(Regex(".+\\.r\\d{2}")) -> primary.dropLast(4)
            lower.matches(Regex(".+\\.z\\d{2,}")) ->
                primary.replace(Regex("(?i)\\.z\\d{2,}$"), "")
            else -> primary
        }
        val compoundSuffixes = listOf(
            ".tar.gz", ".tar.bz2", ".tar.xz", ".tar.lzma", ".tar.zst", ".tar.lz4",
            ".tar.br", ".tar.lzip", ".tgz", ".tbz", ".tbz2", ".txz",
        )
        val lowerWithoutSuffix = withoutSuffix.lowercase(Locale.ROOT)
        val base = compoundSuffixes.firstOrNull { lowerWithoutSuffix.endsWith(it) }
            ?.let { withoutSuffix.dropLast(it.length) }
            ?: withoutSuffix.substringBeforeLast('.', withoutSuffix)
        return base.ifBlank { "extracted" }
    }
}
