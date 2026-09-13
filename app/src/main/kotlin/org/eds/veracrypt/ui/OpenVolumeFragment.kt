package org.eds.veracrypt.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.core.view.isVisible
import androidx.core.widget.doAfterTextChanged
import androidx.lifecycle.lifecycleScope
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import com.sovworks.eds.android.R
import com.sovworks.eds.android.databinding.FragmentOpenVolumeBinding
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import org.eds.veracrypt.VeraCryptApplication
import org.eds.veracrypt.credentials.BiometricCredentialAuthorizer
import org.eds.veracrypt.credentials.SavedUnlockCredential
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeError
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.domain.VolumeOpenTarget
import org.eds.veracrypt.domain.VolumeUnlockProgress
import org.eds.veracrypt.domain.VolumeUnlockStage
import org.eds.veracrypt.nativecore.VcCore
import org.eds.veracrypt.nativecore.resolvedCipherHint
import org.eds.veracrypt.nativecore.resolvedKdfHint
import org.eds.veracrypt.nativecore.volumeInfo
import org.eds.veracrypt.session.ManagedVolumeSession
import javax.crypto.Cipher
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicLong
import kotlin.coroutines.resume

/** Credential screen. Manual passwords always detect outer and hidden headers automatically. */
class OpenVolumeFragment : SensitiveFragment() {
    private var binding: FragmentOpenVolumeBinding? = null
    private val keyfiles = KeyfileSelections(this)
    private val protectionKeyfiles = KeyfileSelections(this)
    private val credentialAuthorizer = BiometricCredentialAuthorizer()
    private var unlockCancellation: AtomicBoolean? = null

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, state: Bundle?): View =
        FragmentOpenVolumeBinding.inflate(inflater, container, false).also { binding = it }.root

    override fun onViewCreated(view: View, state: Bundle?) {
        val entry = app.catalog.find(requireArguments().getString(ARG_ENTRY_ID)?.let(java.util.UUID::fromString)
            ?: error("Missing catalog entry")) ?: run {
            parentFragmentManager.popBackStack()
            return
        }
        binding!!.containerName.text = entry.displayName
        binding!!.cipher.adapter = ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item,
            openCipherHints.map { it.label(requireContext()) })
        binding!!.kdf.adapter = ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item,
            openKdfHints.map { it.label(requireContext()) })
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
            screen.hiddenPim.doAfterTextChanged {
                if (parsePim(it?.toString()) != null) screen.hiddenPim.error = null
            }
            renderKdfParameters(screen)
            keyfiles.bind(screen.keyfileList, screen.selectKeyfiles, screen.selectKeyfileDirectory, screen.generateKeyfile) {
                screen.openStatus.text = it
            }
            protectionKeyfiles.bind(screen.hiddenKeyfileList, screen.selectHiddenKeyfiles,
                screen.selectHiddenKeyfileDirectory, screen.generateHiddenKeyfile) { screen.openStatus.text = it }
            screen.protectHiddenVolume.setOnCheckedChangeListener { _, _ ->
                screen.hiddenProtectionCredentials.isVisible = screen.protectHiddenVolume.isChecked
            }
            screen.biometricUnlock.isVisible = app.credentialVault.has(SavedUnlockCredential.recordId(entry.id))
            screen.biometricUnlock.setOnClickListener { unlockWithBiometric(entry) }
            screen.unlock.setOnClickListener { beginManualUnlock(entry) }
            screen.cancelUnlock.setOnClickListener {
                unlockCancellation?.set(true)
                screen.cancelUnlock.isEnabled = false
                screen.openStatus.setText(R.string.vc_unlock_cancelling)
            }
        }
    }

    private fun beginManualUnlock(entry: org.eds.veracrypt.catalog.ContainerCatalogEntry) {
        val screen = binding ?: return
        screen.passwordLayout.error = null
        val pim = parsePim(screen.pim.text?.toString())
        if (pim == null) {
            screen.pim.error = getString(R.string.vc_invalid_pim)
            screen.pim.requestFocus()
            return
        }
        val protectionPim = if (screen.protectHiddenVolume.isChecked) {
            parsePim(screen.hiddenPim.text?.toString())
        } else {
            null
        }
        if (screen.protectHiddenVolume.isChecked && protectionPim == null) {
            screen.hiddenPim.error = getString(R.string.vc_invalid_pim)
            screen.hiddenPim.requestFocus()
            return
        }
        val passwordChars = screen.password.text?.toString()?.toCharArray() ?: CharArray(0)
        screen.password.text?.clear()
        val credentials = VolumeCredentials(SecretPassword(passwordChars),
            pim, keyfiles.snapshot(), openKdfHints[screen.kdf.selectedItemPosition])
        passwordChars.fill('\u0000')
        val protection = protectionPim?.let(::readProtectionCredentials)
        val options = VolumeOpenOptions(
            cipherHint = openCipherHints[screen.cipher.selectedItemPosition],
            target = VolumeOpenTarget.AUTO,
            accessMode = VolumeAccessMode.AUTOMATIC,
            hiddenVolumeProtection = protection,
        )
        // Capture only a short-lived copy. It is persisted only after the real
        // session proves this is an eligible outer volume and the user agrees.
        val pendingSave = if (credentials.keyfiles.isEmpty() && protection == null) {
            SavedUnlockCredential.capture(credentials, VolumeOpenOptions(
                cipherHint = options.cipherHint, target = VolumeOpenTarget.NORMAL, accessMode = options.accessMode))
        } else null
        unlock(entry, options, credentials, protection, pendingSave)
    }

    private fun unlock(
        entry: org.eds.veracrypt.catalog.ContainerCatalogEntry,
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
        protection: VolumeCredentials?,
        pendingSave: SavedUnlockCredential?,
    ) {
        val screen = binding ?: run { credentials.close(); protection?.close(); pendingSave?.close(); return }
        (activity as? ContainerCatalogActivity)?.requestForegroundNotificationPermission()
        screen.unlock.isEnabled = false
        screen.biometricUnlock.isEnabled = false
        unlockCancellation = AtomicBoolean(false)
        screen.cancelUnlock.isVisible = true
        screen.cancelUnlock.isEnabled = true
        screen.openProgress.isVisible = true
        screen.openStatus.setText(R.string.vc_opening_volume)
        viewLifecycleOwner.lifecycleScope.launch { unlockSuspending(entry, options, credentials, protection, pendingSave, screen) }
    }

    private suspend fun unlockSuspending(
        entry: org.eds.veracrypt.catalog.ContainerCatalogEntry,
        options: VolumeOpenOptions,
        credentials: VolumeCredentials,
        protection: VolumeCredentials?,
        pendingSave: SavedUnlockCredential?,
        screen: FragmentOpenVolumeBinding,
    ) {
        try {
            if (UnlockedVolumeService.volumes.findForContainer(entry.id) != null) {
                screen.openStatus.setText(R.string.vc_volume_already_unlocked)
                return
            }
            val cancellation = checkNotNull(unlockCancellation)
            val lastProgressAt = AtomicLong(0L)
            val session = app.repository.open(entry, options, credentials, object : VolumeUnlockProgress {
                override fun onStage(stage: VolumeUnlockStage) {
                    activity?.runOnUiThread { if (binding === screen) renderUnlockStage(screen, stage) }
                }
                override fun onProbe(completed: Int, total: Int) {
                    val now = android.os.SystemClock.uptimeMillis()
                    val previous = lastProgressAt.get()
                    if (now - previous >= 200L || (total > 0 && completed >= total)) {
                        if (lastProgressAt.compareAndSet(previous, now)) {
                            activity?.runOnUiThread { if (binding === screen) renderUnlockProgress(screen, completed, total) }
                        }
                    }
                }
                override fun isCancellationRequested() = cancellation.get()
            })
            screen.openStatus.setText(R.string.vc_volume_unlocked)
            if (pendingSave != null && session.volumeKind == VolumeKind.NORMAL && askToEnableBiometric()) {
                saveAfterSuccessfulUnlock(entry, session, pendingSave, screen)
            }
            if (!showProviderRoot(session)) screen.openStatus.setText(R.string.vc_open_browser_failed)
        } catch (error: CancellationException) {
            throw error
        } catch (error: Exception) {
            if (error is VolumeError.Cancelled) {
                screen.openStatus.setText(R.string.vc_unlock_cancelled)
                screen.password.requestFocus()
            } else if (error is VolumeError.InvalidCredentialsOrFormat) {
                screen.passwordLayout.error = getString(R.string.vc_invalid_credentials)
                screen.password.requestFocus()
                screen.openStatus.setText(R.string.vc_invalid_credentials)
            } else {
                screen.openStatus.text = error.message ?: getString(R.string.vc_open_failed)
            }
        } finally {
            screen.unlock.isEnabled = true
            screen.biometricUnlock.isEnabled = true
            screen.cancelUnlock.isVisible = false
            unlockCancellation = null
            screen.openProgress.isVisible = false
            credentials.close(); protection?.close(); pendingSave?.close()
            keyfiles.clear(); protectionKeyfiles.clear()
        }
    }

    private suspend fun askToEnableBiometric(): Boolean = suspendCancellableCoroutine { continuation ->
        if (!isAdded) { continuation.resume(false); return@suspendCancellableCoroutine }
        val dialog = MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.vc_enable_biometric_title)
            .setMessage(R.string.vc_enable_biometric_message)
            .setNegativeButton(android.R.string.cancel) { _, _ -> if (continuation.isActive) continuation.resume(false) }
            .setPositiveButton(R.string.vc_enable_biometric) { _, _ -> if (continuation.isActive) continuation.resume(true) }
            .setOnCancelListener { if (continuation.isActive) continuation.resume(false) }
            .show()
        continuation.invokeOnCancellation { dialog.dismiss() }
    }

    private suspend fun saveAfterSuccessfulUnlock(
        entry: org.eds.veracrypt.catalog.ContainerCatalogEntry,
        session: org.eds.veracrypt.domain.VolumeSession,
        saved: SavedUnlockCredential,
        screen: FragmentOpenVolumeBinding,
    ) {
        try {
            val cipher: Cipher = credentialAuthorizer.authorize(requireActivity(), app.credentialVault.cipherForEncryption())
            val native = session as? ManagedVolumeSession ?: return
            val info = VcCore.volumeInfo(native.nativeHandle)
            saved.resolve(info.resolvedCipherHint(), info.resolvedKdfHint())
            app.credentialVault.save(SavedUnlockCredential.recordId(entry.id), cipher, saved.encode())
            screen.biometricUnlock.isVisible = true
        } catch (_: CancellationException) {
            screen.openStatus.setText(R.string.vc_biometric_save_skipped)
        } catch (_: Exception) {
            screen.openStatus.setText(R.string.vc_biometric_save_skipped)
        }
    }

    private fun unlockWithBiometric(entry: org.eds.veracrypt.catalog.ContainerCatalogEntry) {
        val screen = binding ?: return
        screen.biometricUnlock.isEnabled = false
        viewLifecycleOwner.lifecycleScope.launch {
            try {
                val saved = SavedUnlockCredential.decode(app.credentialVault.loadAuthorized(
                    requireActivity(), credentialAuthorizer, SavedUnlockCredential.recordId(entry.id)))
                try { unlock(entry, saved.takeOptions(), saved.takeCredentials(), null, null) } finally { saved.close() }
            } catch (_: CancellationException) {
                screen.openStatus.setText(R.string.vc_biometric_cancelled)
                screen.biometricUnlock.isEnabled = true
            } catch (_: Exception) {
                screen.openStatus.setText(R.string.vc_biometric_unavailable)
                screen.biometricUnlock.isEnabled = true
            }
        }
    }

    private fun readProtectionCredentials(pim: Int): VolumeCredentials {
        val screen = binding ?: error("Open volume view is no longer available")
        val chars = screen.hiddenPassword.text?.toString()?.toCharArray() ?: CharArray(0)
        screen.hiddenPassword.text?.clear()
        // Native request v2 has one KDF byte for the primary credential only;
        // hidden-volume protection deliberately remains AUTO so all six
        // VeraCrypt KDFs are probed without changing the persisted layout.
        return try { VolumeCredentials(SecretPassword(chars), pim, protectionKeyfiles.snapshot()) } finally { chars.fill('\u0000') }
    }

    private fun renderKdfParameters(screen: FragmentOpenVolumeBinding) {
        val kdf = openKdfHints.getOrNull(screen.kdf.selectedItemPosition) ?: KdfHint.AUTO
        val pim = parsePim(screen.pim.text?.toString())
        screen.kdfParameters.text = pim?.let { kdf.parametersDescription(requireContext(), it) }.orEmpty()
        screen.kdfParameters.isVisible = kdf == KdfHint.ARGON2ID && pim != null
    }

    private fun renderUnlockStage(screen: FragmentOpenVolumeBinding, stage: VolumeUnlockStage) {
        screen.openProgress.isIndeterminate = false
        when (stage) {
            VolumeUnlockStage.PREPARING_INPUTS -> screen.openStatus.setText(R.string.vc_unlock_preparing)
            VolumeUnlockStage.DERIVING_AND_VERIFYING -> screen.openStatus.setText(R.string.vc_unlock_deriving)
            VolumeUnlockStage.MOUNTING_FILESYSTEM -> screen.openStatus.setText(R.string.vc_unlock_mounting)
            VolumeUnlockStage.REGISTERING_PROVIDER -> screen.openStatus.setText(R.string.vc_unlock_registering)
        }
    }

    private fun renderUnlockProgress(screen: FragmentOpenVolumeBinding, completed: Int, total: Int) {
        screen.openProgress.max = total.coerceAtLeast(1)
        screen.openProgress.progress = completed.coerceIn(0, screen.openProgress.max)
        screen.openStatus.text = getString(R.string.vc_unlock_deriving_progress, completed, total)
    }

    private fun showProviderRoot(session: org.eds.veracrypt.domain.VolumeSession): Boolean {
        if (!isAdded) return false
        val treeUri = UnlockedVolumeService.rootTreeUri(session) ?: return false
        (requireActivity() as ContainerCatalogActivity).startTrustedActivity(
            FileManagerIntents.unlockedVolume(requireContext(), treeUri),
        )
        parentFragmentManager.popBackStack()
        return true
    }

    override fun onDestroyView() { binding = null; super.onDestroyView() }
    private val app: VeraCryptApplication get() = requireActivity().application as VeraCryptApplication
    companion object { const val ARG_ENTRY_ID = "entry_id" }
}
