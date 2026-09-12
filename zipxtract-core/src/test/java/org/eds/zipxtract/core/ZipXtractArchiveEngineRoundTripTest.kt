package org.eds.zipxtract.core

import java.io.ByteArrayInputStream
import java.io.File
import java.io.InputStream
import java.io.OutputStream
import java.nio.file.Files
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ZipXtractArchiveEngineRoundTripTest {
    @Test fun createsAndExtractsAnUnencryptedZip() {
        val root = Files.createTempDirectory("zipxtract-round-trip").toFile()
        try {
            val archive = File(root, "created.zip")
            val destination = File(root, "extracted")
            val contents = "RongVualt archive round trip".toByteArray()
            withPrivateArchiveTempStore(File(root, "staging")) { store ->
                val engine = ZipXtractArchiveEngine(store)
                engine.create(
                    CreateArchiveRequest(
                        sources = listOf(
                            ArchiveInputEntry(
                                name = "folder/source.txt",
                                isDirectory = false,
                                size = contents.size.toLong(),
                                openInputStream = { ByteArrayInputStream(contents) },
                            ),
                        ),
                        destination = FileTarget(archive),
                        format = ArchiveFormat.ZIP,
                    ),
                )
                assertTrue(archive.isFile)

                engine.extract(
                    ExtractRequest(
                        source = FileSource(archive),
                        destination = FileTarget(destination),
                    ),
                )
            }
            assertEquals("RongVualt archive round trip", File(destination, "folder/source.txt").readText())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test fun extractsSelectedEncryptedEntryWithPasswordIntoContainingDirectory() {
        val root = Files.createTempDirectory("zipxtract-encrypted-entry").toFile()
        try {
            val archive = File(root, "protected.zip")
            val destination = File(root, "extracted")
            val contents = "encrypted RongVualt entry".toByteArray()
            withPrivateArchiveTempStore(File(root, "staging")) { store ->
                val engine = ZipXtractArchiveEngine(store)
                engine.create(
                    CreateArchiveRequest(
                        sources = listOf(
                            ArchiveInputEntry(
                                name = "folder/secret.txt",
                                isDirectory = false,
                                size = contents.size.toLong(),
                                openInputStream = { ByteArrayInputStream(contents) },
                            ),
                        ),
                        destination = FileTarget(archive),
                        format = ArchiveFormat.ZIP,
                        options = ArchiveCreateOptions(zipEncryption = ArchiveEncryption.AES),
                        password = "RongVualt-test".toCharArray(),
                    ),
                )
                engine.extract(
                    ExtractRequest(
                        source = FileSource(archive),
                        destination = FileTarget(destination),
                        entries = setOf("folder/secret.txt"),
                        createContainingDirectory = true,
                        password = "RongVualt-test".toCharArray(),
                    ),
                )
            }
            assertEquals(
                "encrypted RongVualt entry",
                File(destination, "protected/folder/secret.txt").readText(),
            )
        } finally {
            root.deleteRecursively()
        }
    }

    private class FileSource(private val file: File) : ArchiveSource {
        override val displayName: String get() = file.name
        override val size: Long get() = file.length()
        override fun openInputStream(): InputStream = file.inputStream()
    }

    private class FileTarget(private val file: File) : ArchiveTarget {
        override val displayName: String get() = file.name
        override fun exists(): Boolean = file.exists()
        override fun isDirectory(): Boolean = file.isDirectory
        override fun createDirectory() {
            check(file.mkdirs() || file.isDirectory) { "Unable to create $file" }
        }
        override fun openOutputStream(overwrite: Boolean): OutputStream {
            file.parentFile?.mkdirs()
            if (file.exists() && !overwrite) throw IllegalStateException("Target already exists: $file")
            return file.outputStream()
        }
        override fun parent(): ArchiveTarget? = file.parentFile?.let(::FileTarget)
        override fun resolve(name: String): ArchiveTarget = FileTarget(File(file, name))
        override fun resolveSibling(name: String): ArchiveTarget = FileTarget(File(file.parentFile, name))
        override fun deleteIfExists() {
            file.delete()
        }
    }
}
