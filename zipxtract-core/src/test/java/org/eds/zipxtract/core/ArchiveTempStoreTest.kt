package org.eds.zipxtract.core

import java.io.File
import java.nio.file.Files
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class ArchiveTempStoreTest {
    @Test fun staleOperationDirectoriesAreRemoved() {
        val root = Files.createTempDirectory("zipxtract-test").toFile()
        try {
            val stale = File(root, "zipxtract-killed").apply {
                mkdirs()
                File(this, "container.zip").writeText("temporary")
            }
            File(root, "unrelated").mkdirs()
            cleanupStalePrivateArchiveTempStores(root)
            assertFalse(stale.exists())
            assertTrue(File(root, "unrelated").exists())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test fun stagingKeepsUnicodeVolumeBasename() {
        val root = Files.createTempDirectory("zipxtract-test").toFile()
        val store = PrivateArchiveTempStore(root)
        try {
            val source = object : ArchiveSource {
                override val displayName = "备份.7z.001"
                override val size = 1L
                override fun openInputStream() = "x".byteInputStream()
            }
            assertTrue(store.stage(source).name.contains("备份.7z.001"))
        } finally {
            store.close()
            root.deleteRecursively()
        }
    }

    @Test fun operationDirectoryIsRemovedOnCancellationPath() {
        val root = Files.createTempDirectory("zipxtract-test").toFile()
        var operation: File? = null
        try {
            runCatching {
                withPrivateArchiveTempStore(root) { store ->
                    operation = store.operationDirectory()
                    throw ArchiveCancelledException()
                }
            }
            assertTrue(operation != null)
            assertFalse(operation!!.exists())
        } finally {
            root.deleteRecursively()
        }
    }
}
