package org.eds.veracrypt.ui

import java.util.UUID
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.eds.veracrypt.catalog.ContainerCatalogEntry
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeSessionState

class ContainerUiModelsTest {
    private val entry = ContainerCatalogEntry(UUID.randomUUID(), "content://private", "vault.hc")

    @Test fun lockedContainerOnlyOffersUnlock() {
        val card = entry.toCardUi(null)
        assertEquals(ContainerCardState.LOCKED, card.state)
        assertFalse(card.canBrowse)
        assertFalse(card.canShowDetails)
        assertFalse(card.canLock)
    }

    @Test fun writableFatOuterVolumeOffersHiddenCreation() {
        val card = entry.toCardUi(ContainerRuntimeSnapshot(
            VolumeKind.NORMAL, VolumeFileSystem.EXFAT, false, VolumeSessionState.Open,
        ))
        assertEquals(ContainerCardState.UNLOCKED_READ_WRITE, card.state)
        assertTrue(card.canBrowse)
        assertTrue(card.canShowDetails)
        assertTrue(card.canCreateHiddenVolume)
    }

    @Test fun readOnlyAndProtectionStatesNeverOfferHiddenCreation() {
        val readOnly = entry.toCardUi(ContainerRuntimeSnapshot(
            VolumeKind.NORMAL, VolumeFileSystem.NTFS, true, VolumeSessionState.Open,
        ))
        val protected = entry.toCardUi(ContainerRuntimeSnapshot(
            VolumeKind.NORMAL, VolumeFileSystem.EXFAT, true, VolumeSessionState.ProtectionTriggered,
        ))
        assertEquals(ContainerCardState.UNLOCKED_READ_ONLY, readOnly.state)
        assertEquals(ContainerCardState.PROTECTION_TRIGGERED, protected.state)
        assertFalse(readOnly.canCreateHiddenVolume)
        assertFalse(protected.canCreateHiddenVolume)
    }

    @Test fun ntfsOnlyOffersDecryptExportWhileWritableFatAndExfatOfferBothTransfers() {
        val ntfs = ContainerRuntimeSnapshot(
            VolumeKind.NORMAL, VolumeFileSystem.NTFS, true, VolumeSessionState.Open,
        ).transferUiPolicy()
        val fat = ContainerRuntimeSnapshot(
            VolumeKind.NORMAL, VolumeFileSystem.FAT, false, VolumeSessionState.Open,
        ).transferUiPolicy()
        val exfat = ContainerRuntimeSnapshot(
            VolumeKind.NORMAL, VolumeFileSystem.EXFAT, false, VolumeSessionState.Open,
        ).transferUiPolicy()

        assertFalse(ntfs.canImport)
        assertTrue(ntfs.canExport)
        assertTrue(fat.canImport)
        assertTrue(fat.canExport)
        assertTrue(exfat.canImport)
        assertTrue(exfat.canExport)
    }

    @Test fun writableNtfsCanChangeHeadersButNeverOffersFilesystemWrites() {
        val ntfs = entry.toCardUi(ContainerRuntimeSnapshot(
            VolumeKind.NORMAL, VolumeFileSystem.NTFS, true, VolumeSessionState.Open, canModifyContainer = true,
        ))
        assertFalse(ntfs.canCreateHiddenVolume)
        assertTrue(ntfs.canChangeCredentials)
    }

    @Test fun safeDetailModelHasNoSourceOrCredentialFields() {
        val names = VolumeDetailsUiModel::class.java.declaredFields.map { it.name }
        assertFalse(names.any { it.contains("uri", true) || it.contains("password", true) || it.contains("pim", true) || it.contains("keyfile", true) || it.contains("offset", true) })
    }
}
