package org.eds.veracrypt.ui

import android.content.Intent
import android.os.Bundle
import android.provider.DocumentsContract
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.os.bundleOf
import androidx.core.view.isVisible
import androidx.fragment.app.Fragment
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import androidx.activity.result.contract.ActivityResultContracts
import com.sovworks.eds.android.R
import com.sovworks.eds.android.databinding.FragmentVolumeDetailsBinding
import kotlinx.coroutines.launch
import kotlinx.coroutines.flow.collectLatest
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.documents.FileTransferManager
import org.eds.veracrypt.documents.TransferDirection
import org.eds.veracrypt.documents.TransferRequest
import org.eds.veracrypt.documents.TransferState
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.nativecore.VcCore
import org.eds.veracrypt.nativecore.resolvedCipherHint
import org.eds.veracrypt.nativecore.resolvedKdfHint
import org.eds.veracrypt.nativecore.volumeInfo
import org.eds.veracrypt.session.ManagedVolumeSession

/** Displays only non-sensitive metadata from a currently active volume session. */
class VolumeDetailsFragment : Fragment() {
    private var binding: FragmentVolumeDetailsBinding? = null
    private val containerId: java.util.UUID by lazy {
        java.util.UUID.fromString(requireArguments().getString(ARG_CONTAINER_ID))
    }
    private var pendingDirection: TransferDirection? = null
    private var pendingSources: List<android.net.Uri> = emptyList()
    private val sourcePicker = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        val data = result.data
        val uris = buildList {
            data?.data?.let(::add)
            data?.clipData?.let { clip -> for (index in 0 until clip.itemCount) add(clip.getItemAt(index).uri) }
        }.distinct()
        if (result.resultCode == android.app.Activity.RESULT_OK && uris.isNotEmpty()) {
            val providerAuthority = requireContext().packageName + ".unlocked"
            if ((pendingDirection == TransferDirection.DECRYPT_EXPORT && uris.any { it.authority != providerAuthority }) ||
                (pendingDirection == TransferDirection.ENCRYPT_IMPORT && uris.any { it.authority == providerAuthority })) {
                pendingSources = emptyList()
                pendingDirection = null
                binding?.transferFailures?.text = getString(R.string.vc_transfer_invalid_source)
                return@registerForActivityResult
            }
            pendingSources = uris
            targetPicker.launch(null)
        }
    }
    private val targetPicker = registerForActivityResult(ActivityResultContracts.OpenDocumentTree()) { target ->
        val direction = pendingDirection
        pendingDirection = null
        if (target != null && direction != null) {
            val providerAuthority = requireContext().packageName + ".unlocked"
            val targetIsProvider = target.authority == providerAuthority
            if ((direction == TransferDirection.ENCRYPT_IMPORT && !targetIsProvider) ||
                (direction == TransferDirection.DECRYPT_EXPORT && targetIsProvider)) {
                binding?.transferFailures?.text = getString(R.string.vc_transfer_invalid_target)
                pendingSources = emptyList()
                return@registerForActivityResult
            }
            val flags = Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION
            runCatching { requireContext().contentResolver.takePersistableUriPermission(target, flags) }
            pendingSources.forEach { source ->
                runCatching { requireContext().contentResolver.takePersistableUriPermission(source, flags) }
            }
            startTransfer(direction, pendingSources, target)
        }
        pendingSources = emptyList()
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, state: Bundle?): View =
        FragmentVolumeDetailsBinding.inflate(inflater, container, false).also { binding = it }.root

    override fun onViewCreated(view: View, state: Bundle?) {
        FileTransferManager.bind(requireContext())
        (activity as? ContainerCatalogActivity)?.setPageTitle(getString(R.string.rv_container_details))
        binding!!.detailsBrowse.setOnClickListener { browse() }
        binding!!.detailsImport.setOnClickListener { beginImport() }
        binding!!.detailsExport.setOnClickListener { beginExport() }
        binding!!.detailsLock.setOnClickListener { lock() }
        binding!!.detailsReturn.setOnClickListener { parentFragmentManager.popBackStack() }
        viewLifecycleOwner.lifecycleScope.launch {
            viewLifecycleOwner.repeatOnLifecycle(Lifecycle.State.STARTED) {
                launch {
                    UnlockedVolumeService.volumes.volumes.collectLatest {
                        val session = UnlockedVolumeService.volumes.findForContainer(containerId)?.session
                        if (session == null) render() else session.state.collect { render() }
                    }
                }
                launch { FileTransferManager.state.collect { renderTransfer(it) } }
            }
        }
        render()
        renderTransfer(FileTransferManager.state.value)
    }

    override fun onDestroyView() {
        binding = null
        super.onDestroyView()
    }

    private fun render() {
        val screen = binding ?: return
        val volume = UnlockedVolumeService.volumes.findForContainer(containerId)
        val managed = volume?.session as? ManagedVolumeSession
        if (volume == null || managed == null) {
            screen.detailsContent.isVisible = false
            screen.detailsLocked.isVisible = true
            screen.detailsBrowse.isVisible = false
            screen.detailsLock.isVisible = false
            screen.detailsReturn.isVisible = true
            screen.detailsImport.isVisible = false
            screen.detailsExport.isVisible = false
            return
        }
        val info = runCatching { VcCore.volumeInfo(managed.nativeHandle) }.getOrNull()
        if (info == null) {
            screen.detailsContent.isVisible = false
            screen.detailsLocked.isVisible = true
            screen.detailsBrowse.isVisible = false
            screen.detailsLock.isVisible = false
            screen.detailsReturn.isVisible = true
            screen.detailsImport.isVisible = false
            screen.detailsExport.isVisible = false
            return
        }
        val snapshot = ContainerRuntimeSnapshot(
            managed.volumeKind, managed.fileSystem, managed.isReadOnly, managed.state.value, managed.canModifyContainer,
        )
        val details = VolumeDetailsUiModel(
            displayName = volume.displayName,
            state = snapshot.cardState(),
            volumeKind = managed.volumeKind,
            fileSystem = managed.fileSystem,
            logicalSizeBytes = info.logicalSize,
            sectorSizeBytes = info.sectorSize,
            cipherName = info.resolvedCipherHint().name,
            kdfName = info.resolvedKdfHint().name,
            usedBackupHeader = info.usedBackupHeader,
        )
        screen.detailsContent.isVisible = true
        screen.detailsLocked.isVisible = false
        screen.detailsReturn.isVisible = false
        screen.detailsBrowse.isVisible = true
        val transferPolicy = snapshot.transferUiPolicy()
        screen.detailsImport.isVisible = transferPolicy.canImport
        screen.detailsExport.isVisible = transferPolicy.canExport
        screen.detailsLock.isVisible = !FileTransferManager.hasActiveForVolume(volume.id)
        screen.detailsName.text = details.displayName
        screen.detailsStatus.text = label(R.string.rv_details_status, stateText(details.state))
        screen.detailsKind.text = label(R.string.rv_details_volume_kind, if (details.volumeKind == VolumeKind.HIDDEN) getString(R.string.rv_kind_hidden) else getString(R.string.rv_kind_normal))
        screen.detailsFilesystem.text = label(R.string.rv_details_filesystem, details.fileSystem?.name ?: getString(R.string.rv_unknown))
        screen.detailsAccess.text = label(R.string.rv_details_access, if (managed.isReadOnly) getString(R.string.rv_access_read_only) else getString(R.string.rv_access_read_write))
        screen.detailsSize.text = label(R.string.rv_details_size, formatBytes(details.logicalSizeBytes))
        screen.detailsSector.text = label(R.string.rv_details_sector, getString(R.string.rv_bytes, details.sectorSizeBytes))
        screen.detailsCipher.text = label(R.string.rv_details_cipher, details.cipherName)
        screen.detailsKdf.text = label(R.string.rv_details_kdf, details.kdfName)
        screen.detailsBackupHeader.text = label(R.string.rv_details_backup_header, getString(if (details.usedBackupHeader) R.string.rv_yes else R.string.rv_no))
    }

    private fun beginImport() {
        val volume = UnlockedVolumeService.volumes.findForContainer(containerId) ?: return
        if (volume.session.isReadOnly) return
        pendingDirection = TransferDirection.ENCRYPT_IMPORT
        sourcePicker.launch(Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "*/*"
            putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true)
        })
    }

    private fun beginExport() {
        val volume = UnlockedVolumeService.volumes.findForContainer(containerId) ?: return
        if (UnlockedVolumeService.rootUri(volume.session) == null) return
        pendingDirection = TransferDirection.DECRYPT_EXPORT
        sourcePicker.launch(Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "*/*"
            putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true)
            putExtra(DocumentsContract.EXTRA_INITIAL_URI, UnlockedVolumeService.rootUri(volume.session))
        })
    }

    private fun startTransfer(direction: TransferDirection, sources: List<android.net.Uri>, target: android.net.Uri) {
        val volume = UnlockedVolumeService.volumes.findForContainer(containerId) ?: return
        (activity as? ContainerCatalogActivity)?.requestForegroundNotificationPermission()
        val request = TransferRequest(direction = direction, sourceUris = sources, targetDirectoryUri = target, volumeId = volume.id)
        if (!FileTransferManager.start(request)) {
            binding?.transferFailures?.text = getString(R.string.vc_transfer_already_running)
        }
    }

    private fun renderTransfer(state: TransferState) {
        val screen = binding ?: return
        val activeVolumeId = UnlockedVolumeService.volumes.findForContainer(containerId)?.id
        val request = when (state) {
            is TransferState.Preparing -> state.request
            is TransferState.Running -> state.request
            is TransferState.Cancelling -> state.request
            is TransferState.Completed -> state.request
            is TransferState.PartialSuccess -> state.request
            is TransferState.Failed -> state.request
            is TransferState.Cancelled -> state.request
            TransferState.Idle -> null
        }
        if (request == null || request.volumeId != activeVolumeId) {
            screen.transferCard.isVisible = false
            return
        }
        val progress = when (state) {
            is TransferState.Running -> state.progress
            is TransferState.Completed -> state.progress
            is TransferState.PartialSuccess -> state.progress
            is TransferState.Cancelling -> state.progress
            is TransferState.Failed -> state.progress
            is TransferState.Cancelled -> state.progress
            else -> null
        }
        screen.transferCard.isVisible = true
        screen.transferCancel.isVisible = state is TransferState.Preparing || state is TransferState.Running || state is TransferState.Cancelling
        screen.transferCancel.setOnClickListener { FileTransferManager.cancel() }
        screen.transferTitle.text = when (state) {
            is TransferState.Preparing, is TransferState.Running, is TransferState.Cancelling -> getString(if (request.direction == TransferDirection.ENCRYPT_IMPORT) R.string.vc_transfer_encrypt_import else R.string.vc_transfer_decrypt_export)
            is TransferState.Completed -> getString(R.string.vc_transfer_completed, progress?.totalFileCount ?: 0, progress?.failures?.size ?: 0)
            is TransferState.PartialSuccess -> getString(R.string.vc_transfer_partial, (progress?.totalFileCount ?: 0) - (progress?.failures?.size ?: 0), progress?.failures?.size ?: 0)
            is TransferState.Failed -> getString(R.string.vc_transfer_failed, state.reason)
            is TransferState.Cancelled -> getString(R.string.vc_transfer_cancelled)
            TransferState.Idle -> ""
        }
        if (progress == null) {
            screen.transferFile.text = getString(R.string.vc_transfer_waiting)
            screen.transferProgress.isIndeterminate = true
            screen.transferPercent.text = ""
            return
        }
        screen.transferFile.text = getString(R.string.vc_transfer_current, progress.currentFileName) + "  " + getString(R.string.vc_transfer_files, progress.currentFileIndex, progress.totalFileCount)
        screen.transferProgress.isIndeterminate = progress.percent == null
        screen.transferProgress.progress = progress.percent ?: 0
        screen.transferPercent.text = progress.percent?.let { "${it}%" } ?: getString(R.string.vc_transfer_unknown_total)
        screen.transferBytes.text = if (progress.totalBytes != null) getString(R.string.vc_transfer_bytes, formatBytes(progress.completedBytes), formatBytes(progress.totalBytes)) else getString(R.string.vc_transfer_bytes, formatBytes(progress.completedBytes), "?")
        screen.transferSpeed.text = getString(R.string.vc_transfer_speed, formatBytes(progress.bytesPerSecond))
        screen.transferRemaining.text = progress.remainingSeconds?.let { getString(R.string.vc_transfer_remaining, "${it}s") } ?: ""
        screen.transferFailures.text = progress.failures.joinToString("\n")
    }

    private fun label(labelRes: Int, value: String): String = getString(labelRes, value)

    private fun stateText(state: ContainerCardState): String = getString(when (state) {
        ContainerCardState.LOCKED -> R.string.rv_state_locked
        ContainerCardState.UNLOCKED_READ_WRITE -> R.string.rv_state_unlocked
        ContainerCardState.UNLOCKED_READ_ONLY -> R.string.rv_state_read_only
        ContainerCardState.PROTECTION_TRIGGERED -> R.string.rv_state_protection_triggered
    })

    private fun formatBytes(bytes: Long): String = when {
        bytes >= 1024L * 1024L * 1024L -> String.format(java.util.Locale.getDefault(), "%.2f GiB", bytes / (1024.0 * 1024.0 * 1024.0))
        bytes >= 1024L * 1024L -> String.format(java.util.Locale.getDefault(), "%.2f MiB", bytes / (1024.0 * 1024.0))
        else -> getString(R.string.rv_bytes, bytes)
    }

    private fun browse() {
        val session = UnlockedVolumeService.volumes.findForContainer(containerId)?.session ?: return
        UnlockedVolumeService.rootUri(session)?.let { rootUri ->
            startActivity(Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
                type = "*/*"
                putExtra(DocumentsContract.EXTRA_INITIAL_URI, rootUri)
            })
        }
    }

    private fun lock() {
        val volume = UnlockedVolumeService.volumes.findForContainer(containerId)
        if (volume != null && FileTransferManager.hasActiveForVolume(volume.id)) return
        volume?.let { UnlockedVolumeService.volumes.close(it.id) }
        render()
    }

    companion object {
        private const val ARG_CONTAINER_ID = "container_id"
        fun newInstance(containerId: java.util.UUID) = VolumeDetailsFragment().apply {
            arguments = bundleOf(ARG_CONTAINER_ID to containerId.toString())
        }
    }
}
