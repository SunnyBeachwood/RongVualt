package org.eds.zipxtract.core

import java.io.ByteArrayInputStream
import java.io.InputStream
import org.junit.Assert.assertEquals
import org.junit.Test

class MultipartArchiveResolverTest {
    @Test fun arbitrarySplitEntryMapsToFirstVolumeName() {
        assertEquals("backup.zip.001", MultipartArchiveResolver.primaryName("backup.zip.007", ArchiveFormat.ZIP))
        assertEquals("backup.zip", MultipartArchiveResolver.primaryName("backup.z01", ArchiveFormat.ZIP))
        assertEquals("backup.zip", MultipartArchiveResolver.primaryName("backup.z100", ArchiveFormat.ZIP))
        assertEquals("backup.7z.001", MultipartArchiveResolver.primaryName("backup.7z.002", ArchiveFormat.SEVEN_ZIP))
        assertEquals("backup.part1.rar", MultipartArchiveResolver.primaryName("backup.part4.rar", ArchiveFormat.RAR))
    }

    @Test fun resolvesAnySplitEntryToPrimaryAndFollowingVolumes() {
        val source = MapSource("backup.7z.001", setOf("backup.7z.001", "backup.7z.002"))
        val resolved = MultipartArchiveResolver.resolve(
            source,
            ArchiveProbe(ArchiveFormat.SEVEN_ZIP, source.displayName),
        )
        assertEquals("backup.7z.001", resolved.primary.displayName)
        assertEquals(listOf("backup.7z.001", "backup.7z.002"), resolved.volumes.map { it.displayName })
    }

    @Test fun missingTailIsReportedWithoutProviderStorm() {
        val source = MapSource("backup.zip.001", setOf("backup.zip.001"))
        val resolved = MultipartArchiveResolver.resolve(
            source,
            ArchiveProbe(ArchiveFormat.ZIP, source.displayName),
        )
        assertEquals(listOf("backup.zip.001"), resolved.volumes.map { it.displayName })
        assertEquals(listOf("backup.zip.002"), resolved.missingVolumes)
    }

    @Test fun arbitraryEntryReportsMissingPrimaryVolume() {
        val source = MapSource("backup.7z.002", setOf("backup.7z.002"))
        val resolved = MultipartArchiveResolver.resolve(
            source,
            ArchiveProbe(ArchiveFormat.SEVEN_ZIP, source.displayName),
        )
        assertEquals(listOf("backup.7z.001", "backup.7z.003"), resolved.missingVolumes)
    }

    @Test fun zipPartVolumesAreResolvedFromAnyPart() {
        val source = MapSource(
            "backup.zip.part2",
            setOf("backup.zip.part1", "backup.zip.part2", "backup.zip.part3"),
        )
        val resolved = MultipartArchiveResolver.resolve(
            source,
            ArchiveProbe(ArchiveFormat.ZIP, source.displayName),
        )
        assertEquals(
            listOf("backup.zip.part1", "backup.zip.part2", "backup.zip.part3"),
            resolved.volumes.map { it.displayName },
        )
    }

    @Test fun canonicalZipWithOnePrecedingVolumeDoesNotInventAnotherTail() {
        val source = MapSource("backup.zip", setOf("backup.zip", "backup.z01"))
        val resolved = MultipartArchiveResolver.resolve(
            source,
            ArchiveProbe(ArchiveFormat.ZIP, source.displayName),
        )
        assertEquals(listOf("backup.zip", "backup.z01"), resolved.volumes.map { it.displayName })
        assertEquals(emptyList<String>(), resolved.missingVolumes)
    }

    private class MapSource(
        override val displayName: String,
        private val names: Set<String>,
    ) : ArchiveSource {
        override val size: Long = 0
        override fun openInputStream(): InputStream = ByteArrayInputStream(ByteArray(0))
        override fun openSibling(name: String): ArchiveSource? =
            if (name in names) MapSource(name, names) else null
    }
}
