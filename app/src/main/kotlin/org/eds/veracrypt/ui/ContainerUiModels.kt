package org.eds.veracrypt.ui

import java.util.UUID
import org.eds.veracrypt.catalog.ContainerCatalogEntry
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeSessionState

/** UI-only, non-sensitive projection of an active unlocked session. */
internal data class ContainerRuntimeSnapshot(
    val volumeKind: VolumeKind,
    val fileSystem: VolumeFileSystem?,
    val isReadOnly: Boolean,
    val state: VolumeSessionState,
    val canModifyContainer: Boolean = !isReadOnly,
)

internal enum class ContainerCardState {
    LOCKED,
    UNLOCKED_READ_WRITE,
    UNLOCKED_READ_ONLY,
    PROTECTION_TRIGGERED,
}

internal data class ContainerCardUiModel(
    val entry: ContainerCatalogEntry,
    val state: ContainerCardState,
    val canBrowse: Boolean,
    val canShowDetails: Boolean,
    val canLock: Boolean,
    val canCreateHiddenVolume: Boolean,
    val canChangeCredentials: Boolean,
)

/** Small, safe aggregate for the catalog header. It never exposes a container source. */
internal data class ContainerCatalogSummary(
    val totalContainers: Int,
    val unlockedContainers: Int,
)

internal fun catalogSummary(
    entries: List<ContainerCatalogEntry>,
    activeContainerIds: Set<UUID>,
): ContainerCatalogSummary = ContainerCatalogSummary(
    totalContainers = entries.size,
    unlockedContainers = entries.count { it.id in activeContainerIds },
)

internal fun ContainerRuntimeSnapshot.cardState(): ContainerCardState = when {
    state == VolumeSessionState.ProtectionTriggered -> ContainerCardState.PROTECTION_TRIGGERED
    isReadOnly || fileSystem == VolumeFileSystem.NTFS -> ContainerCardState.UNLOCKED_READ_ONLY
    else -> ContainerCardState.UNLOCKED_READ_WRITE
}

internal fun ContainerCatalogEntry.toCardUi(runtime: ContainerRuntimeSnapshot?): ContainerCardUiModel {
    if (runtime == null) {
        return ContainerCardUiModel(this, ContainerCardState.LOCKED, false, false, false, false, false)
    }
    val state = runtime.cardState()
    val canCreateHidden = state == ContainerCardState.UNLOCKED_READ_WRITE &&
        runtime.volumeKind == VolumeKind.NORMAL &&
        runtime.fileSystem in setOf(VolumeFileSystem.FAT, VolumeFileSystem.EXFAT)
    return ContainerCardUiModel(
        entry = this,
        state = state,
        canBrowse = true,
        canShowDetails = true,
        canLock = true,
        canCreateHiddenVolume = canCreateHidden,
        canChangeCredentials = runtime.canModifyContainer,
    )
}

/** Safe detail content: no credentials, SAF URI, container path or raw offsets. */
internal data class VolumeDetailsUiModel(
    val displayName: String,
    val state: ContainerCardState,
    val volumeKind: VolumeKind,
    val fileSystem: VolumeFileSystem?,
    val logicalSizeBytes: Long,
    val sectorSizeBytes: Int,
    val cipherName: String,
    val kdfName: String,
    val usedBackupHeader: Boolean,
)

/** Availability of the app-owned file transfer actions for an unlocked volume. */
internal data class VolumeTransferUiPolicy(
    val canImport: Boolean,
    val canExport: Boolean,
)

internal fun ContainerRuntimeSnapshot.transferUiPolicy(): VolumeTransferUiPolicy = VolumeTransferUiPolicy(
    canImport = !isReadOnly && fileSystem in setOf(VolumeFileSystem.FAT, VolumeFileSystem.EXFAT),
    canExport = true,
)
