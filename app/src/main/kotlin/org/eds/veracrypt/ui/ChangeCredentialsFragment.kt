package org.eds.veracrypt.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.core.os.bundleOf
import androidx.core.view.isVisible
import androidx.core.widget.doAfterTextChanged
import androidx.lifecycle.lifecycleScope
import com.sovworks.eds.android.R
import com.sovworks.eds.android.databinding.FragmentChangeCredentialsBinding
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.launch
import org.eds.veracrypt.VeraCryptApplication
import org.eds.veracrypt.credentials.BiometricCredentialAuthorizer
import org.eds.veracrypt.credentials.SavedUnlockCredential
import org.eds.veracrypt.documents.FileTransferManager
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.nativecore.VcCore
import org.eds.veracrypt.nativecore.volumeInfo
import org.eds.veracrypt.nativecore.resolvedCipherHint
import org.eds.veracrypt.nativecore.resolvedKdfHint
import org.eds.veracrypt.session.ManagedVolumeSession

/** Updates headers only after Android device authentication; old saved unlock data is removed on success. */
class ChangeCredentialsFragment : SensitiveFragment() {
    private var binding: FragmentChangeCredentialsBinding? = null
    private val keyfiles = KeyfileSelections(this)
    private val authorizer = BiometricCredentialAuthorizer()

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, state: Bundle?): View =
        FragmentChangeCredentialsBinding.inflate(inflater, container, false).also { binding = it }.root

    override fun onViewCreated(view: View, state: Bundle?) {
        val entryId = requireArguments().getString(ARG_ENTRY_ID)?.let(java.util.UUID::fromString) ?: return navigateBack()
        val entry = app.catalog.find(entryId) ?: return navigateBack()
        val session = UnlockedVolumeService.volumes.findForContainer(entryId)?.session
        if (session == null || !session.canModifyContainer || FileTransferManager.hasActiveForVolume(session.id)) return navigateBack()
        binding!!.containerName.text = entry.displayName
        binding!!.kdf.adapter = ArrayAdapter(
            requireContext(), android.R.layout.simple_spinner_dropdown_item,
            creatableKdfHints.map { it.label(requireContext()) },
        )
        val currentKdf = (session as? ManagedVolumeSession)?.let { nativeSession ->
            runCatching { VcCore.volumeInfo(nativeSession.nativeHandle).resolvedKdfHint() }.getOrNull()
        }
        val currentKdfPosition = currentKdf?.let { creatableKdfHints.indexOf(it) }?.takeIf { it >= 0 } ?: 0
        binding!!.kdf.setSelection(currentKdfPosition, false)
        binding!!.let { screen ->
            screen.kdf.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
                override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
                    renderKdfParameters(screen)
                }

                override fun onNothingSelected(parent: AdapterView<*>?) = Unit
            }
            screen.pim.doAfterTextChanged {
                if (parsePim(it?.toString()) != null) screen.pim.error = null
                renderKdfParameters(screen)
            }
            renderKdfParameters(screen)
        }
        keyfiles.bind(binding!!.keyfileList, binding!!.selectKeyfiles, binding!!.selectKeyfileDirectory, binding!!.generateKeyfile) {
            binding?.status?.text = it
        }
        if (session.volumeKind == VolumeKind.HIDDEN) {
            binding!!.saveWithBiometric.isChecked = false
            binding!!.saveWithBiometric.isVisible = false
        }
        binding!!.changeCredentials.setOnClickListener { submit(entryId, session as? ManagedVolumeSession ?: return@setOnClickListener) }
    }

    private fun submit(entryId: java.util.UUID, session: ManagedVolumeSession) {
        val screen = binding ?: return
        val pim = parsePim(screen.pim.text?.toString())
        if (pim == null) {
            screen.pim.error = getString(R.string.vc_invalid_pim)
            screen.pim.requestFocus()
            return
        }
        val password = screen.password.text?.toString()?.toCharArray() ?: CharArray(0)
        val confirmation = screen.passwordConfirm.text?.toString()?.toCharArray() ?: CharArray(0)
        screen.password.text?.clear(); screen.passwordConfirm.text?.clear()
        if (!password.contentEquals(confirmation)) {
            password.fill('\u0000'); confirmation.fill('\u0000')
            screen.status.setText(R.string.vc_passwords_do_not_match)
            return
        }
        confirmation.fill('\u0000')
        val credentials = VolumeCredentials(
            SecretPassword(password), pim, keyfiles.snapshot(), creatableKdfHints[screen.kdf.selectedItemPosition],
        )
        password.fill('\u0000')
        val saved = if (session.volumeKind != VolumeKind.HIDDEN && screen.saveWithBiometric.isChecked && credentials.keyfiles.isEmpty()) {
            SavedUnlockCredential.capture(credentials, VolumeOpenOptions(
                cipherHint = CipherHint.AUTO,
                target = if (session.volumeKind == VolumeKind.HIDDEN) org.eds.veracrypt.domain.VolumeOpenTarget.HIDDEN else org.eds.veracrypt.domain.VolumeOpenTarget.NORMAL,
                accessMode = VolumeAccessMode.AUTOMATIC,
            ))
        } else null
        screen.changeCredentials.isEnabled = false
        screen.progress.isVisible = true
        viewLifecycleOwner.lifecycleScope.launch {
            try {
                // The cipher is deliberately discarded: this prompt is an authorization gate for modifying headers.
                authorizer.authorize(requireActivity(), app.credentialVault.cipherForEncryption(), getString(R.string.vc_change_credentials))
                app.repository.changeCredentials(session, credentials)
                app.credentialVault.remove(SavedUnlockCredential.recordId(entryId))
                if (saved != null) {
                    VcCore.volumeInfo(session.nativeHandle).let { info -> saved.resolve(info.resolvedCipherHint(), info.resolvedKdfHint()) }
                    app.credentialVault.saveAuthorized(requireActivity(), authorizer, SavedUnlockCredential.recordId(entryId), saved.encode())
                }
                screen.status.setText(R.string.vc_credentials_changed)
            } catch (_: CancellationException) {
                credentials.close(); saved?.close()
            } catch (error: Throwable) {
                credentials.close(); saved?.close()
                screen.status.text = error.message
            } finally {
                screen.progress.isVisible = false
                screen.changeCredentials.isEnabled = true
            }
        }
    }

    private fun renderKdfParameters(screen: FragmentChangeCredentialsBinding) {
        val kdf = creatableKdfHints.getOrNull(screen.kdf.selectedItemPosition) ?: KdfHint.PBKDF2_HMAC_SHA512
        val pim = parsePim(screen.pim.text?.toString())
        screen.kdfParameters.text = pim?.let { kdf.parametersDescription(requireContext(), it) }.orEmpty()
        screen.kdfParameters.isVisible = kdf == KdfHint.ARGON2ID && pim != null
    }

    private fun navigateBack() { parentFragmentManager.popBackStack() }
    private val app: VeraCryptApplication get() = requireActivity().application as VeraCryptApplication
    companion object {
        private const val ARG_ENTRY_ID = "entry_id"
        fun newInstance(entryId: java.util.UUID) = ChangeCredentialsFragment().apply { arguments = bundleOf(ARG_ENTRY_ID to entryId.toString()) }
    }
}
