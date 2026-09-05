/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 */

package org.eds.zipxtract.core

import java.util.Locale

object ArchivePathPolicy {
    /**
     * Return a normalized relative path or reject it.  Both slash styles are
     * accepted because Windows-created archives commonly contain backslashes.
     */
    fun normalizeEntryName(rawName: String): String {
        require('\u0000' !in rawName) { "Archive entry contains NUL" }
        val slashName = rawName.replace('\\', '/')
        require(!slashName.startsWith('/')) { "Absolute archive entry: $rawName" }
        require(!Regex("^[A-Za-z]:.*").matches(slashName)) {
            "Drive-qualified archive entry: $rawName"
        }
        val components = slashName.split('/')
        val normalized = ArrayDeque<String>(components.size)
        for (component in components) {
            when (component) {
                "", "." -> Unit
                ".." -> throw IllegalArgumentException("Archive entry contains parent traversal: $rawName")
                else -> normalized.addLast(component)
            }
        }
        return normalized.joinToString("/")
    }

    fun isChildOf(parent: String, child: String): Boolean =
        parent.isEmpty() || child == parent || child.startsWith("$parent/")

    fun canonicalKey(path: String): String = path.lowercase(Locale.ROOT)
}
