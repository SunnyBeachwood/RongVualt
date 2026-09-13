package org.eds.veracrypt.ui

import android.content.Intent
import android.os.Bundle
import android.provider.DocumentsContract
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.core.view.isVisible
import androidx.core.widget.doAfterTextChanged
import androidx.lifecycle.lifecycleScope
import com.sovworks.eds.android.R
import com.sovworks.eds.android.databinding.FragmentCreateHiddenVolumeBinding
import kotlinx.coroutines.launch
import kotlinx.coroutines.CancellationException
import org.eds.veracrypt.VeraCryptApplication
import org.eds.veracrypt.catalog.ContainerCatalogEntry
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.MAX_CREATED_VOLUME_SIZE_BYTES

/** Two-stage hidden-volume creation from a mounted writable outer volume. */
class CreateHiddenVolumeFragment : SensitiveFragment() {
    private var binding: FragmentCreateHiddenVolumeBinding? = null
    private val keyfiles = KeyfileSelections(this)
    private var capacityBytes = 0L
    private lateinit var entry: ContainerCatalogEntry
    private var creationProgress: CreationProgressController? = null

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, state: Bundle?): View =
        FragmentCreateHiddenVolumeBinding.inflate(inflater, container, false).also { binding = it }.root

    override fun onViewCreated(view: View, state: Bundle?) {
        entry = app.catalog.find(requireArguments().getString(ARG_ENTRY_ID)?.let(java.util.UUID::fromString)
            ?: error("Missing catalog entry")) ?: run {
            parentFragmentManager.popBackStack()
            return
        }
        binding!!.containerName.text = entry.displayName
        binding!!.cipher.adapter = ArrayAdapter(
            requireContext(), android.R.layout.simple_spinner_dropdown_item,
            creatableCipherHints.map { it.label(requireContext()) },
        )
        binding!!.kdf.adapter = ArrayAdapter(
            requireContext(), android.R.layout.simple_spinner_dropdown_item,
            creatableKdfHints.map { it.label(requireContext()) },
        )
        binding!!.fileSystem.adapter = ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item,
            listOf(VolumeFileSystem.EXFAT, VolumeFileSystem.FAT).map(VolumeFileSystem::name))
        binding!!.let { screen ->
            screen.cipher.onItemSelectedListener = descriptionListener { renderOptionDescriptions(screen) }
            screen.kdf.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
                override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
                    renderOptionDescriptions(screen)
                }

                override fun onNothingSelected(parent: AdapterView<*>?) = Unit
            }
            screen.fileSystem.onItemSelectedListener = descriptionListener { renderOptionDescriptions(screen) }
            screen.pim.doAfterTextChanged {
                if (parsePim(it?.toString()) != null) screen.pim.error = null
                renderOptionDescriptions(screen)
            }
            renderOptionDescriptions(screen)
            keyfiles.bind(screen.keyfileList, screen.selectKeyfiles, screen.selectKeyfileDirectory, screen.generateKeyfile) {
                message -> screen.createStatus.text = message
            }
            screen.createVolume.isEnabled = false
            screen.createVolume.setOnClickListener { createHidden() }
        }
        analyzeCapacity()
    }

    override fun onDestroyView() {
        creationProgress?.cancel()
        creationProgress = null
        binding = null
        super.onDestroyView()
    }

    private fun analyzeCapacity() {
        val screen = binding ?: return
        val outer = UnlockedVolumeService.volumes.findForContainer(entry.id)?.session ?: run {
            screen.createStatus.text = getString(R.string.vc_hidden_requires_outer)
            return
        }
        screen.createProgress.isVisible = true
        viewLifecycleOwner.lifecycleScope.launch {
            try {
                val capacity = app.repository.analyzeHiddenCapacity(outer)
                capacityBytes = capacity.maximumBytes
                screen.maximumCapacity.text = getString(R.string.vc_hidden_capacity, capacityBytes / MIB)
                screen.createVolume.isEnabled = capacityBytes > 0
            } catch (error: Throwable) {
                screen.createStatus.text = error.message ?: getString(R.string.vc_hidden_requires_outer)
            } finally {
                screen.createProgress.isVisible = false
            }
        }
    }

    private fun createHidden() {
        val screen = binding ?: return
        val sizeMiB = screen.sizeMib.text?.toString()?.toLongOrNull()
        if (sizeMiB == null || sizeMiB <= 0 || sizeMiB > minOf(capacityBytes, MAX_CREATED_VOLUME_SIZE_BYTES) / MIB) {
            screen.createStatus.text = getString(R.string.vc_invalid_size)
            return
        }
        val pim = parsePim(screen.pim.text?.toString())
        if (pim == null) {
            screen.pim.error = getString(R.string.vc_invalid_pim)
            screen.pim.requestFocus()
            return
        }
        val outer = UnlockedVolumeService.volumes.findForContainer(entry.id)?.session ?: run {
            screen.createStatus.text = getString(R.string.vc_hidden_requires_outer)
            return
        }
        screen.passwordConfirmLayout.error = null
        val passwordChars = screen.password.text?.toString()?.toCharArray() ?: CharArray(0)
        val confirmationChars = screen.passwordConfirm.text?.toString()?.toCharArray() ?: CharArray(0)
        screen.password.text?.clear()
        screen.passwordConfirm.text?.clear()
        if (!passwordChars.contentEquals(confirmationChars)) {
            passwordChars.fill('\u0000')
            confirmationChars.fill('\u0000')
            screen.passwordConfirmLayout.error = getString(R.string.vc_creation_passwords_do_not_match)
            screen.passwordConfirm.requestFocus()
            return
        }
        confirmationChars.fill('\u0000')
        val credentials = VolumeCredentials(
            SecretPassword(passwordChars),
            pim,
            keyfiles.snapshot(),
            creatableKdfHints[screen.kdf.selectedItemPosition],
        )
        passwordChars.fill('\u0000')
        val options = VolumeCreateOptions(
            sizeBytes = sizeMiB * MIB,
            volumeKind = VolumeKind.HIDDEN,
            cipher = creatableCipherHints[screen.cipher.selectedItemPosition],
            kdf = creatableKdfHints[screen.kdf.selectedItemPosition],
            pim = credentials.pim,
            fileSystem = VolumeFileSystem.valueOf(screen.fileSystem.selectedItem as String),
        )
        screen.createVolume.isEnabled = false
        screen.createProgress.isVisible = true
        screen.cancelCreate.isVisible = true
        val progress = CreationProgressController(screen.root) { stage, complete, total ->
            if (binding === screen) {
                renderProgress(screen.createProgress, screen.createStatus, stage, complete, total)
            }
        }
        creationProgress = progress
        screen.cancelCreate.setOnClickListener {
            progress.cancel()
            it.isEnabled = false
            screen.createStatus.text = getString(R.string.vc_cancelling_creation)
        }
        viewLifecycleOwner.lifecycleScope.launch {
            try {
                val hidden = app.repository.createHidden(entry, outer, options, credentials, progress.reporter)
                screen.createStatus.text = getString(R.string.vc_volume_unlocked)
                UnlockedVolumeService.rootUri(hidden)?.let { rootUri ->
                    (requireActivity() as ContainerCatalogActivity).startTrustedActivity(Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
                        type = "*/*"
                        putExtra(DocumentsContract.EXTRA_INITIAL_URI, rootUri)
                    })
                }
            } catch (error: CancellationException) {
                screen.createStatus.text = error.message ?: getString(R.string.vc_create_failed)
                screen.createVolume.isEnabled = true
            } catch (error: Exception) {
                screen.createStatus.text = error.message ?: getString(R.string.vc_create_failed)
                screen.createVolume.isEnabled = true
            } finally {
                credentials.close()
                keyfiles.clear()
                screen.createProgress.isVisible = false
                screen.cancelCreate.isVisible = false
                creationProgress = null
            }
        }
    }

    private val app: VeraCryptApplication get() = requireActivity().application as VeraCryptApplication

    private fun renderOptionDescriptions(screen: FragmentCreateHiddenVolumeBinding) {
        screen.cipherDescription.text = creatableCipherHints
            .getOrElse(screen.cipher.selectedItemPosition) { creatableCipherHints.first() }
            .description(requireContext())
        val kdf = creatableKdfHints.getOrNull(screen.kdf.selectedItemPosition) ?: KdfHint.PBKDF2_HMAC_SHA512
        val pim = parsePim(screen.pim.text?.toString())
        screen.kdfParameters.text = listOfNotNull(
            kdf.description(requireContext()),
            pim?.let { kdf.parametersDescription(requireContext(), it) },
        ).joinToString("\n")
        val fileSystem = listOf(VolumeFileSystem.EXFAT, VolumeFileSystem.FAT)
            .getOrElse(screen.fileSystem.selectedItemPosition) { VolumeFileSystem.EXFAT }
        screen.fileSystemDescription.text = fileSystem.description(requireContext())
    }

    private fun descriptionListener(render: () -> Unit) = object : AdapterView.OnItemSelectedListener {
        override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) = render()
        override fun onNothingSelected(parent: AdapterView<*>?) = Unit
    }

    private fun renderProgress(
        progress: android.widget.ProgressBar,
        status: android.widget.TextView,
        stage: Int,
        completedBytes: Long,
        totalBytes: Long,
    ) {
        when (stage) {
            1 -> {
                progress.isIndeterminate = false
                progress.max = 100
                progress.progress = if (totalBytes == 0L) 0 else ((completedBytes * 100) / totalBytes).toInt()
                status.text = getString(R.string.vc_initializing_volume, progress.progress)
            }
            2 -> {
                progress.isIndeterminate = true
                status.text = getString(R.string.vc_formatting_volume)
            }
            else -> {
                progress.isIndeterminate = true
                status.text = getString(R.string.vc_finalizing_volume)
            }
        }
    }

    companion object {
        const val ARG_ENTRY_ID = "entry_id"
        private const val MIB = 1024L * 1024L
    }
}
