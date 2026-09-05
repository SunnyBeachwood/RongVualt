/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 */

package org.eds.zipxtract.core

import java.util.Locale

data class ResolvedArchiveVolumes(
    val primary: ArchiveSource,
    val volumes: List<ArchiveSource>,
    val missingVolumes: List<String> = emptyList(),
)

/** Resolves ZipXtract-style volume names without assuming a java.io.File path. */
object MultipartArchiveResolver {
    fun resolve(source: ArchiveSource, probe: ArchiveProbe = ArchiveFormatDetector.detect(source)): ResolvedArchiveVolumes {
        val primaryName = primaryName(source.displayName, probe.format)
        val primarySibling = if (source.displayName.equals(primaryName, ignoreCase = true)) {
            source
        } else {
            source.openSibling(primaryName)
        }
        val primary = primarySibling ?: source
        val missingPrimary = if (
            primarySibling == null &&
            !source.displayName.equals(primaryName, ignoreCase = true) &&
            isNumberedVolume(source.displayName, probe.format)
        ) listOf(primaryName) else emptyList()
        val (volumes, missing) = resolveSequential(primary, probe.format)
        return ResolvedArchiveVolumes(
            primary,
            volumes.ifEmpty { listOf(primary) },
            (missingPrimary + missing).distinct(),
        )
    }

    fun primaryName(name: String, format: ArchiveFormat): String {
        val base = name.substringAfterLast('/').substringAfterLast('\\')
        val lower = base.lowercase(Locale.ROOT)
        return when (format) {
            ArchiveFormat.ZIP -> when {
                lower.matches(Regex(".+\\.z\\d{2,}")) ->
                    base.replace(Regex("(?i)\\.z\\d{2,}$"), ".zip")
                lower.matches(Regex(".+\\.zip\\.\\d{3}")) ->
                    base.substringBeforeLast('.') + ".001"
                lower.matches(Regex(".+\\.zip\\.part\\d+")) ->
                    base.replace(Regex("(?i)\\.part\\d+$"), ".part1")
                else -> base
            }
            ArchiveFormat.SEVEN_ZIP -> if (lower.matches(Regex(".+\\.7z\\.\\d{3}"))) {
                base.substringBeforeLast('.') + ".001"
            } else base
            ArchiveFormat.RAR -> when {
                lower.matches(Regex(".+\\.part\\d+\\.rar")) -> base.replace(Regex("(?i)\\.part\\d+\\.rar$"), ".part1.rar")
                lower.matches(Regex(".+\\.r\\d{2}")) -> base.dropLast(3) + "rar"
                lower.matches(Regex(".+\\.\\d{3}")) -> base.dropLast(4) + ".rar"
                else -> base
            }
            else -> base
        }
    }

    private fun resolveSequential(primary: ArchiveSource, format: ArchiveFormat): Pair<List<ArchiveSource>, List<String>> {
        val base = primary.displayName.substringAfterLast('/').substringAfterLast('\\')
        val lower = base.lowercase(Locale.ROOT)
        val names = when (format) {
            ArchiveFormat.ZIP -> if (lower.endsWith(".zip")) {
                val stem = base.dropLast(4)
                sequenceNames("$stem.z", 2)
            } else if (lower.matches(Regex(".+\\.zip\\.\\d{3}"))) {
                val stem = base.substringBeforeLast('.')
                sequenceNamesFrom(base.substringAfterLast('.').toIntOrNull() ?: 1) { i ->
                    "$stem.${i.toString().padStart(3, '0')}"
                }
            } else if (lower.matches(Regex(".+\\.zip\\.part\\d+"))) {
                val stem = base.replace(Regex("(?i)\\.part\\d+$"), "")
                val start = Regex("(?i)\\.part(\\d+)$").find(base)
                    ?.groupValues?.get(1)?.toIntOrNull() ?: 1
                sequenceNamesFrom(start) { i -> "$stem.part$i" }
            } else emptyList()
            ArchiveFormat.SEVEN_ZIP -> if (lower.endsWith(".7z")) {
                val stem = base.dropLast(3)
                sequenceNames("$stem.7z.", 3)
            } else if (lower.matches(Regex(".+\\.7z\\.\\d{3}"))) {
                val stem = base.substringBeforeLast('.')
                sequenceNamesFrom(base.substringAfterLast('.').toIntOrNull() ?: 1) { i ->
                    "$stem.${i.toString().padStart(3, '0')}"
                }
            } else emptyList()
            ArchiveFormat.RAR -> when {
                lower.endsWith(".part1.rar") -> {
                    val stem = base.dropLast(".part1.rar".length)
                    generateSequence(1) { it + 1 }.map { "$stem.part$it.rar" }.take(999).toList()
                }
                lower.matches(Regex(".+\\.part\\d+\\.rar")) -> {
                    val stem = base.replace(Regex("(?i)\\.part\\d+\\.rar$"), "")
                    val start = Regex("(?i)\\.part(\\d+)\\.rar$").find(base)?.groupValues?.get(1)?.toIntOrNull() ?: 1
                    generateSequence(start) { it + 1 }.map { "$stem.part$it.rar" }.take(999).toList()
                }
                lower.endsWith(".rar") -> {
                    val stem = base.dropLast(4)
                    listOf("$stem.rar") + generateSequence(0) { it + 1 }
                        .map { "$stem.r${it.toString().padStart(2, '0')}" }.take(999).toList()
                }
                else -> emptyList()
            }
            else -> emptyList()
        }
        if (names.isEmpty()) return listOf(primary) to emptyList()
        val result = mutableListOf<ArchiveSource>(primary)
        val missing = mutableListOf<String>()
        // A canonical `.zip`/`.rar` entry does not carry the final volume
        // number.  Seeing `.z01` or `.r00` is therefore not enough to infer
        // that another part is missing: two-volume sets are perfectly valid.
        // Numbered entry points, on the other hand, identify a contiguous
        // sequence and let us report the first absent successor precisely.
        val expectsContinuation = lower.matches(Regex(".+\\.(zip|7z)\\.\\d{3}")) ||
            lower.matches(Regex(".+\\.zip\\.part\\d+")) ||
            lower.matches(Regex(".+\\.part\\d+\\.rar")) ||
            lower.matches(Regex(".+\\.r\\d{2}"))
        // A split set is contiguous by definition. Stop at the first absent volume;
        // the concrete engine will report a malformed set if its central directory
        // references a later volume. This avoids hundreds of provider lookups.
        for (name in names.dropWhile { it.equals(primary.displayName, ignoreCase = true) }) {
            val sibling = primary.openSibling(name)
            if (sibling == null) {
                if (expectsContinuation) missing += name
                break
            }
            result += sibling
        }
        return result to missing
    }

    private fun isNumberedVolume(name: String, format: ArchiveFormat): Boolean {
        val base = name.substringAfterLast('/').substringAfterLast('\\')
        val lower = base.lowercase(Locale.ROOT)
        return when (format) {
            ArchiveFormat.ZIP -> lower.matches(Regex(".+\\.z\\d{2,}")) ||
                lower.matches(Regex(".+\\.zip\\.(\\d{3}|part\\d+)"))
            ArchiveFormat.SEVEN_ZIP -> lower.matches(Regex(".+\\.7z\\.\\d{3}"))
            ArchiveFormat.RAR -> lower.matches(Regex(".+\\.part\\d+\\.rar")) ||
                lower.matches(Regex(".+\\.r\\d{2}"))
            else -> false
        }
    }

    private fun sequenceNames(prefix: String, width: Int): List<String> =
        generateSequence(1) { it + 1 }
            .map { "$prefix${it.toString().padStart(width, '0')}" }
            .take(999)
            .toList()

    private fun sequenceNamesFrom(start: Int, name: (Int) -> String): List<String> =
        generateSequence(start.coerceAtLeast(1)) { it + 1 }
            .map(name)
            .take(999)
            .toList()

    @Suppress("unused")
    private fun volumeNames(primaryName: String, format: ArchiveFormat): List<String> {
        val base = primaryName.substringAfterLast('/').substringAfterLast('\\')
        val lower = base.lowercase(Locale.ROOT)
        return when (format) {
            ArchiveFormat.ZIP -> if (lower.endsWith(".zip")) {
                val stem = base.dropLast(4)
                buildList {
                    add("$stem.zip")
                    for (i in 1..999) add("$stem.z${i.toString().padStart(2, '0')}")
                }
            } else listOf(base)
            ArchiveFormat.SEVEN_ZIP -> if (lower.endsWith(".7z")) {
                val stem = base.dropLast(3)
                buildList {
                    add("$stem.7z")
                    for (i in 1..999) add("$stem.7z.${i.toString().padStart(3, '0')}")
                }
            } else listOf(base)
            ArchiveFormat.RAR -> when {
                lower.endsWith(".part1.rar") -> {
                    val stem = base.dropLast(".part1.rar".length)
                    buildList { for (i in 1..999) add("$stem.part${i}.rar") }
                }
                lower.endsWith(".rar") -> {
                    val stem = base.dropLast(4)
                    buildList {
                        add("$stem.rar")
                        for (i in 0..999) add("$stem.r${i.toString().padStart(2, '0')}")
                    }
                }
                else -> listOf(base)
            }
            else -> listOf(base)
        }
    }
}
