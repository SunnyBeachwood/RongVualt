package org.eds.zipxtract.core

import org.junit.Assert.assertEquals
import org.junit.Test

class ArchiveNamesTest {
    @Test fun stripsSplitAndCompoundSuffixes() {
        assertEquals("backup", ArchiveNames.containingDirectoryName("backup.zip.007", ArchiveFormat.ZIP))
        assertEquals("backup", ArchiveNames.containingDirectoryName("backup.z100", ArchiveFormat.ZIP))
        assertEquals("backup", ArchiveNames.containingDirectoryName("backup.7z.002", ArchiveFormat.SEVEN_ZIP))
        assertEquals("backup", ArchiveNames.containingDirectoryName("backup.part4.rar", ArchiveFormat.RAR))
        assertEquals("photos", ArchiveNames.containingDirectoryName("photos.tar.zst", ArchiveFormat.COMPRESSED_TAR))
    }
}
