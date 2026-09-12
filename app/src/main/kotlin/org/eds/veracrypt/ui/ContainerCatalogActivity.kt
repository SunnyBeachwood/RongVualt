package org.eds.veracrypt.ui

import android.os.Bundle
import android.content.Intent
import android.widget.Toast
import android.Manifest
import android.os.Build
import androidx.activity.result.contract.ActivityResultContracts
import android.view.WindowManager
import android.view.MenuItem
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.updatePadding
import androidx.core.view.WindowCompat
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.view.isVisible
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.os.LocaleListCompat
import androidx.lifecycle.lifecycleScope
import java.util.concurrent.CancellationException
import com.sovworks.eds.android.R
import com.sovworks.eds.android.databinding.ActivityContainerCatalogBinding
import kotlinx.coroutines.launch
import org.eds.veracrypt.VeraCryptApplication
import org.eds.veracrypt.credentials.AppAccessGate
import org.eds.veracrypt.credentials.BiometricCredentialAuthorizer
import org.eds.veracrypt.documents.FileTransferManager
import org.eds.veracrypt.documents.UnlockedVolumeService
import me.zhanghai.android.files.ftpserver.FtpServerService
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import me.zhanghai.android.files.compat.forceShowIconsCompat
import me.zhanghai.android.files.app.AppAccessSession

class ContainerCatalogActivity : AppCompatActivity() {
    private lateinit var binding: ActivityContainerCatalogBinding
    private val authorizer = BiometricCredentialAuthorizer()
    private val accessGate by lazy { AppAccessGate() }
    private var hasStarted = false
    private var shouldAuthenticate = false
    private var authenticating = false
    private var exiting = false
    private lateinit var homeThemeSignature: HomeThemeHelper.Signature
    /** One transition into a RongVault workflow must not lock its own return. */
    private var trustedNavigationPending = false
    private val notificationPermission = registerForActivityResult(ActivityResultContracts.RequestPermission()) { }

    override fun onCreate(savedInstanceState: Bundle?) {
        homeThemeSignature = HomeThemeHelper.apply(this)
        super.onCreate(savedInstanceState)
        WindowCompat.setDecorFitsSystemWindows(window, false)
        binding = ActivityContainerCatalogBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setSupportActionBar(binding.appToolbar)
        binding.appToolbar.setNavigationOnClickListener { onBackPressedDispatcher.onBackPressed() }
        supportFragmentManager.addOnBackStackChangedListener {
            if (supportFragmentManager.backStackEntryCount == 0) {
                binding.appToolbar.title = getString(R.string.app_name)
                binding.appToolbar.navigationIcon = null
            }
        }
        ViewCompat.setOnApplyWindowInsetsListener(binding.root) { view, insets ->
            val bars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            view.updatePadding(top = bars.top, bottom = bars.bottom)
            insets
        }
        ViewCompat.requestApplyInsets(binding.root)
        binding.appUnlock.setOnClickListener { authenticateForAppAccess() }
        // A task can be recreated while a process-owned unlocked session is
        // still alive. Always begin behind the biometric boundary.
        shouldAuthenticate = true
        binding.appLockOverlay.isVisible = true
        window.addFlags(WindowManager.LayoutParams.FLAG_SECURE)
        if (savedInstanceState == null) {
            supportFragmentManager.beginTransaction()
                .replace(R.id.container_catalog_host, ContainerCatalogFragment())
                .commit()
        }
    }

    override fun onResume() {
        super.onResume()
        // Settings are owned by the embedded file manager. It can recreate its own activity
        // while this task remains alive, so refresh the home task when the user returns here.
        if (homeThemeSignature != HomeThemeHelper.signature(this)) {
            recreate()
        }
    }

    override fun onCreateOptionsMenu(menu: android.view.Menu): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        menu.forceShowIconsCompat()
        menu.findItem(R.id.menu_clear_catalog_on_exit)?.isChecked = app.catalog.clearOnExitEnabled()
        return true
    }

    override fun onPrepareOptionsMenu(menu: android.view.Menu): Boolean {
        // The toolbar sits above the full-screen lock overlay on some OEM
        // builds. Removing actions here makes the overlay an actual boundary.
        menu.setGroupVisible(0, !binding.appLockOverlay.isVisible)
        return super.onPrepareOptionsMenu(menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (binding.appLockOverlay.isVisible) return true
        return when (item.itemId) {
        R.id.menu_open_embedded_files -> {
            openEmbeddedFiles()
            true
        }
        R.id.menu_open_system_files -> {
            openSystemFiles()
            true
        }
        R.id.menu_clear_catalog_on_exit -> {
            configureClearCatalogOnExit(item)
            true
        }
        R.id.menu_exit_app -> {
            requestExit()
            true
        }
        R.id.menu_about -> {
            showInfo(true)
            true
        }
        R.id.menu_language -> {
            showLanguageChooser()
            true
        }
            else -> super.onOptionsItemSelected(item)
        }
    }

    private fun showLanguageChooser() {
        val values = arrayOf(
            getString(R.string.rv_language_system),
            getString(R.string.rv_language_chinese),
            getString(R.string.rv_language_english),
        )
        val current = AppCompatDelegate.getApplicationLocales().toLanguageTags()
        val selected = when {
            current.startsWith("zh", ignoreCase = true) -> 1
            current.startsWith("en", ignoreCase = true) -> 2
            else -> 0
        }
        MaterialAlertDialogBuilder(this)
            .setTitle(R.string.rv_language_title)
            .setSingleChoiceItems(values, selected) { dialog, which ->
                AppCompatDelegate.setApplicationLocales(when (which) {
                    1 -> LocaleListCompat.forLanguageTags("zh-CN")
                    2 -> LocaleListCompat.forLanguageTags("en")
                    else -> LocaleListCompat.getEmptyLocaleList()
                })
                dialog.dismiss()
            }
            .show()
    }

    private fun configureClearCatalogOnExit(item: MenuItem) {
        if (app.catalog.clearOnExitEnabled()) {
            app.catalog.setClearOnExitEnabled(false)
            item.isChecked = false
            return
        }
        MaterialAlertDialogBuilder(this)
            .setTitle(R.string.vc_clear_catalog_on_exit_title)
            .setMessage(R.string.vc_clear_catalog_on_exit_message)
            .setNegativeButton(android.R.string.cancel, null)
            .setPositiveButton(android.R.string.ok) { _, _ ->
                app.catalog.setClearOnExitEnabled(true)
                item.isChecked = true
            }
            .show()
    }

    private fun requestExit() {
        if (exiting) return
        val hasActiveState = FileTransferManager.hasActiveTransfer() || UnlockedVolumeService.volumes.volumes.value.isNotEmpty()
        if (!hasActiveState) {
            performExit()
            return
        }
        MaterialAlertDialogBuilder(this)
            .setTitle(R.string.vc_exit_app_title)
            .setMessage(R.string.vc_exit_app_active_message)
            .setNegativeButton(android.R.string.cancel, null)
            .setPositiveButton(android.R.string.ok) { _, _ -> performExit() }
            .show()
    }

    private fun performExit() {
        if (exiting) return
        exiting = true
        lifecycleScope.launch {
            if (!FileTransferManager.cancelAndWait()) {
                exiting = false
                Toast.makeText(this@ContainerCatalogActivity, R.string.vc_exit_app_transfer_timeout, Toast.LENGTH_LONG).show()
                return@launch
            }
            // A shared unlocked volume must release all FTP clients and
            // provider handles before the native sessions are closed.
            FtpServerService.stopAndWait(this@ContainerCatalogActivity)
            UnlockedVolumeService.volumes.close()
            // Closing each volume normally emits a roots notification. Emit one
            // final notification as well so DocumentsUI refreshes even when
            // the last session disappeared during cancellation.
            UnlockedVolumeService.notifyRootsChanged()
            if (app.catalog.clearOnExitEnabled()) app.catalog.clear()
            finishAndRemoveTask()
        }
    }

    internal fun setInfoTitle(titleRes: Int) {
        setPageTitle(getString(titleRes))
    }

    internal fun setPageTitle(title: String) {
        binding.appToolbar.title = title
        binding.appToolbar.navigationIcon = AppCompatResources.getDrawable(
            this,
            androidx.appcompat.R.drawable.abc_ic_ab_back_material,
        )
    }

    internal fun requestForegroundNotificationPermission() {
        if (Build.VERSION.SDK_INT >= 33 && checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) != android.content.pm.PackageManager.PERMISSION_GRANTED) {
            notificationPermission.launch(Manifest.permission.POST_NOTIFICATIONS)
        }
    }

    private fun showInfo(about: Boolean) {
        supportFragmentManager.beginTransaction()
            .replace(R.id.container_catalog_host, InfoFragment.newInstance(about))
            .addToBackStack("info")
            .commit()
    }

    private fun openSystemFiles() {
        try {
            startTrustedActivity(Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
                addCategory(Intent.CATEGORY_OPENABLE)
                type = "*/*"
            })
        } catch (_: Exception) {
            Toast.makeText(this, R.string.vc_system_files_unavailable, Toast.LENGTH_SHORT).show()
        }
    }

    private fun openEmbeddedFiles() {
        try {
            startTrustedActivity(FileManagerIntents.browse(this))
        } catch (_: Exception) {
            Toast.makeText(this, R.string.vc_system_files_unavailable, Toast.LENGTH_SHORT).show()
        }
    }

    private val app: VeraCryptApplication
        get() = application as VeraCryptApplication

    override fun onStart() {
        super.onStart()
        // A trusted transition into the embedded file manager normally keeps this
        // activity unlocked. If the whole app was sent to the background,
        // however, VeraCryptApplication locks the shared access session. Restore
        // the lock UI before exposing the catalog again, otherwise a subsequent
        // FileListActivity immediately finishes because it sees an unauthorized
        // session.
        if (hasStarted && (shouldAuthenticate || !AppAccessSession.isAuthorized())) {
            shouldAuthenticate = true
            showAppLock()
        }
        hasStarted = true
    }

    override fun onStop() {
        if (trustedNavigationPending) {
            trustedNavigationPending = false
        } else if (!isChangingConfigurations && !authenticating) {
            shouldAuthenticate = true
            binding.appLockOverlay.isVisible = true
            invalidateOptionsMenu()
            window.addFlags(WindowManager.LayoutParams.FLAG_SECURE)
        }
        super.onStop()
    }

    private fun showAppLock() {
        binding.appLockOverlay.isVisible = true
        invalidateOptionsMenu()
        binding.appLockStatus.setText(R.string.vc_app_locked_message)
        window.addFlags(WindowManager.LayoutParams.FLAG_SECURE)
        // The system-owned biometric surface may use an OEM-controlled black
        // background. Keep RongVault's own themed lock card visible until the
        // user explicitly requests authentication.
    }

    private fun authenticateForAppAccess() {
        if (authenticating || !binding.appLockOverlay.isVisible) return
        authenticating = true
        binding.appUnlock.isEnabled = false
        lifecycleScope.launch {
            try {
                authorizer.authorize(
                    this@ContainerCatalogActivity,
                    accessGate.cipherForAuthentication(),
                    getString(R.string.vc_app_auth_prompt_title),
                )
                AppAccessSession.authorize()
                shouldAuthenticate = false
                binding.appLockOverlay.isVisible = false
                invalidateOptionsMenu()
            } catch (_: CancellationException) {
                binding.appLockStatus.setText(R.string.vc_app_auth_failed)
            } catch (_: Throwable) {
                binding.appLockStatus.setText(R.string.vc_app_auth_failed)
            } finally {
                authenticating = false
                binding.appUnlock.isEnabled = true
            }
        }
    }

    /** Starts an app-owned browser or document workflow without relocking on return. */
    internal fun startTrustedActivity(intent: Intent) {
        trustedNavigationPending = true
        try {
            startActivity(intent)
        } catch (error: Throwable) {
            trustedNavigationPending = false
            throw error
        }
    }
}
