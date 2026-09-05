package org.eds.zipxtract.core

import java.io.ByteArrayInputStream
import java.io.InputStream
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ArchiveFormatDetectorTest {
    @Test fun detectsMagicBeforeExtension() {
        val source = BytesSource("payload.rar", byteArrayOf(0x50, 0x4b, 0x03, 0x04))
        assertEquals(ArchiveFormat.ZIP, ArchiveFormatDetector.detect(source).format)
    }

    @Test fun recognizesSplitNamesAndGenericFormats() {
        val splitZip = ArchiveFormatDetector.detectByName("a.zip.001")
        assertEquals(ArchiveFormat.ZIP, splitZip.format)
        assertTrue(splitZip.volumeNames.isNotEmpty())
        assertEquals(ArchiveFormat.ZIP, ArchiveFormatDetector.detectByName("a.z01").format)
        assertEquals(ArchiveFormat.ZIP, ArchiveFormatDetector.detectByName("a.z100").format)
        assertEquals(ArchiveFormat.SEVEN_ZIP, ArchiveFormatDetector.detectByName("a.7z.001").format)
        assertEquals(ArchiveFormat.RAR, ArchiveFormatDetector.detectByName("a.part2.rar").format)
        assertEquals(ArchiveFormat.GENERIC, ArchiveFormatDetector.detectByName("disk.iso").format)
        assertEquals(ArchiveFormat.COMPRESSED_STREAM, ArchiveFormatDetector.detectByName("payload.gz").format)
    }

    @Test fun unknownDoesNotAdvertiseCapabilities() {
        val probe = ArchiveFormatDetector.detectByName("notes.txt")
        assertEquals(ArchiveFormat.UNKNOWN, probe.format)
        assertTrue(probe.capabilities.isEmpty())
    }

    @Test fun detectsPosixTarMagicWithoutAnExtension() {
        val header = ByteArray(512)
        "ustar".toByteArray().copyInto(header, 257)
        assertEquals(ArchiveFormat.TAR, ArchiveFormatDetector.detect(BytesSource("payload", header)).format)
    }

    @Test fun detectsGenericContainerMagicWithoutAnExtension() {
        val header = ByteArray(0x8001 + 5)
        "CD001".toByteArray().copyInto(header, 0x8001)
        assertEquals(
            ArchiveFormat.GENERIC,
            ArchiveFormatDetector.detect(BytesSource("payload", header)).format,
        )
    }

    private data class BytesSource(
        override val displayName: String,
        private val bytes: ByteArray,
    ) : ArchiveSource {
        override val size: Long get() = bytes.size.toLong()
        override fun openInputStream(): InputStream = ByteArrayInputStream(bytes)
    }
}
