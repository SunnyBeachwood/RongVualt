/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 *
 * Temporary archive containers are kept private to the application and are
 * deleted by this class on every exit path. Decompressed entries and source
 * files are never staged here.
 */

package org.eds.zipxtract.core

import java.io.Closeable
import java.io.File
import java.io.IOException
import java.util.UUID

class PrivateArchiveTempStore(
    private val root: File,
) : ArchiveTempStore {
    private val operationDirectory: File = File(root, "zipxtract-${UUID.randomUUID()}")

    init {
        if (!operationDirectory.mkdirs() && !operationDirectory.isDirectory) {
            throw IOException("Unable to create private archive staging directory")
        }
    }

    override fun stage(source: ArchiveSource): File = stage(source, source.displayName)

    /** Stage a container while preserving the provider-visible volume name. */
    fun stage(source: ArchiveSource, requestedName: String): File {
        val name = requestedName.substringAfterLast('/').substringAfterLast('\\')
            // Keep Unicode volume names intact: 7-Zip asks the volume
            // callback for the original basename. Only path/control
            // separators are rewritten because this is a private directory.
            .replace(Regex("[\\u0000/\\\\]"), "_")
            .ifBlank { "archive.bin" }
        val file = uniqueFile(name)
        source.openInputStream().use { input -> file.outputStream().use { output -> input.copyTo(output) } }
        return file
    }

    fun newFile(prefix: String, suffix: String): File {
        val safePrefix = prefix.replace(Regex("[^A-Za-z0-9._-]"), "_").take(40).ifBlank { "archive" }
        val safeSuffix = if (suffix.startsWith('.')) suffix else ".${suffix}"
        return File.createTempFile(safePrefix, safeSuffix, operationDirectory)
    }

    fun operationDirectory(): File = operationDirectory

    override fun close() {
        operationDirectory.deleteRecursively()
    }

    private fun uniqueFile(name: String): File {
        val candidate = File(operationDirectory, name)
        if (!candidate.exists()) return candidate
        var index = 1
        while (true) {
            val numbered = File(operationDirectory, "$name.$index")
            if (!numbered.exists()) return numbered
            ++index
        }
    }
}

/** Remove operation directories left by a killed process on the next start. */
fun cleanupStalePrivateArchiveTempStores(root: File) {
    root.listFiles()?.forEach { child ->
        if (child.isDirectory && child.name.startsWith("zipxtract-")) {
            child.deleteRecursively()
        }
    }
}

inline fun <T> withPrivateArchiveTempStore(root: File, block: (PrivateArchiveTempStore) -> T): T {
    val store = PrivateArchiveTempStore(root)
    return try {
        block(store)
    } finally {
        store.close()
    }
}
