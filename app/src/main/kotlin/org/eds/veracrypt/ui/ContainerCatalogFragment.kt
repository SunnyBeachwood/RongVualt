package org.eds.veracrypt.ui

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.OpenableColumns
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.os.bundleOf
import androidx.fragment.app.Fragment
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import androidx.recyclerview.widget.LinearLayoutManager
import com.sovworks.eds.android.R
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import kotlinx.coroutines.awaitCancellation
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch
import org.eds.veracrypt.VeraCryptApplication
import org.eds.veracrypt.catalog.ContainerCatalogEntry
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.documents.FileTransferManager
import org.eds.veracrypt.catalog.ContainerSourceResolver
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeSessionState
import com.sovworks.eds.android.databinding.FragmentContainerCatalogBinding

/** SAF-only catalog. Entries store an opaque random ID plus a persisted URI grant. */
class ContainerCatalogFragment : Fragment() {
    private var binding: FragmentContainerCatalogBinding? = null
    private var entries: List<ContainerCatalogEntry> = emptyList()
    private lateinit var adapter: ContainerCardAdapter

    private val selectContainer = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == android.app.Activity.RESULT_OK) {
            result.data?.data?.let(::addContainer)
        }
    }
    private val createContainer = registerForActivityResult(ActivityResultContracts.CreateDocument("application/octet-stream")) { uri ->
        uri?.let(::configureNewContainer)
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, state: Bundle?): View {
        return FragmentContainerCatalogBinding.inflate(inflater, container, false).also { binding = it }.root
    }

    override fun onViewCreated(view: View, state: Bundle?) {
        adapter = ContainerCardAdapter(
            onUnlock = { showOpen(it.entry) },
            onBrowse = { browse(it.entry) },
            onDetails = { showDetails(it.entry) },
            onLock = { lock(it.entry) },
            onCreateHidden = { showCreateHidden(it.entry) },
            onChangeCredentials = { showChangeCredentials(it.entry) },
            onRemove = { remove(it.entry) },
        )
        binding!!.containerList.layoutManager = LinearLayoutManager(requireContext())
        binding!!.containerList.adapter = adapter
        binding!!.addContainer.setOnClickListener { showAddContainerMenu() }
        viewLifecycleOwner.lifecycleScope.launch {
            viewLifecycleOwner.repeatOnLifecycle(Lifecycle.State.STARTED) {
                UnlockedVolumeService.volumes.volumes.collectLatest { volumes ->
                    refresh()
                    coroutineScope {
                        volumes.forEach { volume -> launch { volume.session.state.collect { refresh() } } }
                        awaitCancellation()
                    }
                }
            }
        }
        refresh()
    }

    override fun onResume() {
        super.onResume()
        refresh()
    }

    override fun onDestroyView() {
        binding = null
        super.onDestroyView()
    }

    private fun addContainer(uri: Uri) {
        ContainerSourceResolver.validate(requireContext(), uri)?.let {
            showMessage(it)
            return
        }
        val flags = Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION
        try {
            // The embedded FileProvider is app-private and remains readable
            // without a persisted external grant. SAF documents still need
            // their normal persisted permission for catalog restoration.
            if (uri.authority != "${requireContext().packageName}.materialfiles.files") {
                requireContext().contentResolver.takePersistableUriPermission(uri, flags)
            }
            val entry = app.catalog.add(uri, displayName(uri))
            refresh()
            showOpen(entry)
        } catch (error: SecurityException) {
            showMessage(getString(R.string.vc_catalog_permission_error))
        } catch (error: IllegalArgumentException) {
            showMessage(error.message ?: getString(R.string.vc_catalog_add_error))
        }
    }

    private fun showAddContainerMenu() {
        MaterialAlertDialogBuilder(requireContext())
            .setItems(arrayOf(getString(R.string.vc_add_existing_container), getString(R.string.vc_create_container))) { _, which ->
                if (which == 0) {
                    val tree = android.provider.DocumentsContract.buildTreeDocumentUri(
                        "com.android.externalstorage.documents", "primary:"
                    )
                    selectContainer.launch(FileManagerIntents.picker(requireContext(), tree))
                } else {
                    createContainer.launch("container.hc")
                }
            }
            .show()
    }

    private fun configureNewContainer(uri: Uri) {
        val flags = Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION
        try {
            requireContext().contentResolver.takePersistableUriPermission(uri, flags)
            val entry = app.catalog.add(uri, displayName(uri))
            parentFragmentManager.beginTransaction()
                .replace(R.id.container_catalog_host, CreateVolumeFragment().apply {
                    arguments = bundleOf(CreateVolumeFragment.ARG_ENTRY_ID to entry.id.toString())
                })
                .addToBackStack(null)
                .commit()
        } catch (error: SecurityException) {
            showMessage(getString(R.string.vc_catalog_permission_error))
        } catch (error: IllegalArgumentException) {
            showMessage(error.message ?: getString(R.string.vc_catalog_add_error))
        }
    }

    private fun displayName(uri: Uri): String {
        requireContext().contentResolver.query(uri, arrayOf(OpenableColumns.DISPLAY_NAME), null, null, null)?.use { cursor ->
            if (cursor.moveToFirst()) return cursor.getString(0) ?: getString(R.string.vc_unnamed_container)
        }
        return getString(R.string.vc_unnamed_container)
    }

    private fun remove(entry: ContainerCatalogEntry) {
        MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.vc_remove_container)
            .setMessage(R.string.vc_remove_container_warning)
            .setNegativeButton(android.R.string.cancel, null)
            .setPositiveButton(R.string.vc_remove_confirm) { _, _ ->
                app.catalog.remove(entry.id)
                refresh()
            }
            .show()
    }

    private fun showOpen(entry: ContainerCatalogEntry) {
        if (UnlockedVolumeService.volumes.findForContainer(entry.id) != null) {
            showMessage(getString(R.string.vc_volume_already_unlocked))
            return
        }
        parentFragmentManager.beginTransaction()
            .replace(R.id.container_catalog_host, OpenVolumeFragment().apply {
                arguments = bundleOf(OpenVolumeFragment.ARG_ENTRY_ID to entry.id.toString())
            })
            .addToBackStack(null)
            .commit()
    }

    private fun browse(entry: ContainerCatalogEntry) {
        val session = UnlockedVolumeService.volumes.findForContainer(entry.id)?.session ?: return
        UnlockedVolumeService.rootTreeUri(session)?.let { treeUri ->
            (requireActivity() as ContainerCatalogActivity).startTrustedActivity(
                FileManagerIntents.unlockedVolume(requireContext(), treeUri),
            )
        }
    }

    private fun showDetails(entry: ContainerCatalogEntry) {
        if (UnlockedVolumeService.volumes.findForContainer(entry.id) == null) return
        parentFragmentManager.beginTransaction()
            .replace(R.id.container_catalog_host, VolumeDetailsFragment.newInstance(entry.id))
            .addToBackStack("volume-details")
            .commit()
    }

    private fun lock(entry: ContainerCatalogEntry) {
        val volume = UnlockedVolumeService.volumes.findForContainer(entry.id)
        if (volume != null && FileTransferManager.hasActiveForVolume(volume.id)) {
            showMessage(getString(R.string.vc_transfer_cannot_lock))
            return
        }
        volume?.let { UnlockedVolumeService.volumes.close(it.id) }
        refresh()
    }

    private fun showCreateHidden(entry: ContainerCatalogEntry) {
        val session = UnlockedVolumeService.volumes.findForContainer(entry.id)?.session
        if (session == null || session.isReadOnly || session.volumeKind != org.eds.veracrypt.domain.VolumeKind.NORMAL ||
            session.fileSystem !in setOf(VolumeFileSystem.FAT, VolumeFileSystem.EXFAT)) {
            showMessage(getString(R.string.vc_hidden_requires_outer))
            return
        }
        parentFragmentManager.beginTransaction()
            .replace(R.id.container_catalog_host, CreateHiddenVolumeFragment().apply {
                arguments = bundleOf(CreateHiddenVolumeFragment.ARG_ENTRY_ID to entry.id.toString())
            })
            .addToBackStack(null)
            .commit()
    }

    private fun refresh() {
        if (!isAdded || !::adapter.isInitialized) return
        entries = app.catalog.list()
        val models = entries.map { entry ->
            UnlockedVolumeService.volumes.findForContainer(entry.id)?.session?.let { session ->
                ContainerRuntimeSnapshot(session.volumeKind, session.fileSystem, session.isReadOnly, session.state.value, session.canModifyContainer)
            }.let(entry::toCardUi)
        }
        adapter.submitList(models)
        binding?.emptyCatalog?.visibility = if (entries.isEmpty()) View.VISIBLE else View.GONE
    }

    private fun showChangeCredentials(entry: ContainerCatalogEntry) {
        val session = UnlockedVolumeService.volumes.findForContainer(entry.id)?.session ?: return
        if (!session.canModifyContainer || FileTransferManager.hasActiveForVolume(session.id)) {
            showMessage(getString(R.string.vc_change_credentials_unavailable))
            return
        }
        parentFragmentManager.beginTransaction()
            .replace(R.id.container_catalog_host, ChangeCredentialsFragment.newInstance(entry.id))
            .addToBackStack("change-credentials")
            .commit()
    }

    private fun showMessage(message: String) {
        binding?.catalogStatus?.text = message
        binding?.catalogStatusCard?.visibility = View.VISIBLE
    }

    private val app: VeraCryptApplication get() = requireActivity().application as VeraCryptApplication
}
