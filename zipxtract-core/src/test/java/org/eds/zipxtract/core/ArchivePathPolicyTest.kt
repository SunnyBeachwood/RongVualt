package org.eds.zipxtract.core

import org.junit.Assert.assertEquals
import org.junit.Test

class ArchivePathPolicyTest {
    @Test fun normalizesWindowsSeparatorsAndDots() {
        assertEquals("folder/file.txt", ArchivePathPolicy.normalizeEntryName("folder\\.\\file.txt"))
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsTraversal() {
        ArchivePathPolicy.normalizeEntryName("../../outside.txt")
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsNormalizedParentComponentsToo() {
        ArchivePathPolicy.normalizeEntryName("safe/../outside.txt")
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsAbsoluteAndNulNames() {
        ArchivePathPolicy.normalizeEntryName("/tmp/out\u0000")
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsDriveQualifiedNames() {
        ArchivePathPolicy.normalizeEntryName("C:relative.txt")
    }

    @Test fun childMatchingIsBoundaryAware() {
        assertEquals(true, ArchivePathPolicy.isChildOf("a", "a/b"))
        assertEquals(false, ArchivePathPolicy.isChildOf("a", "ab"))
    }
}
