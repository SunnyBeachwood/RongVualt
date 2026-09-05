package org.eds.veracrypt

import android.app.Application
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import org.eds.veracrypt.catalog.ContainerCatalog
import org.eds.veracrypt.credentials.CredentialVault
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.nativecore.NativeVeraCryptRepository
import me.zhanghai.android.files.ftpserver.FtpServerService

/** Process owner for catalog records and unlocked native sessions. */
class VeraCryptApplication : Application() {
    private val applicationJob = SupervisorJob()
    val applicationScope = CoroutineScope(applicationJob + Dispatchers.Default)
    val catalog by lazy { ContainerCatalog(this) }
    val credentialVault by lazy { CredentialVault(this) }
    val repository by lazy { NativeVeraCryptRepository(this, applicationScope) }

    override fun onCreate() {
        super.onCreate()
        UnlockedVolumeService.bind(this)
    }

    override fun onTerminate() {
        FtpServerService.stopAndWait(this)
        UnlockedVolumeService.volumes.close()
        applicationJob.cancel()
        super.onTerminate()
    }
}
