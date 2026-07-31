package org.eds.veracrypt.documents

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.content.pm.ServiceInfo
import android.os.IBinder
import android.app.PendingIntent
import androidx.core.content.ContextCompat
import com.sovworks.eds.android.R
import org.eds.veracrypt.domain.VolumeUnlockStage

/** Keeps unlocked volumes and user-initiated full formatting out of cached state. */
class VolumeForegroundService : Service() {
    override fun onCreate() {
        super.onCreate()
        // Android requires a started foreground service to promote itself
        // promptly. Do this before any volume/transfer bookkeeping so a slow
        // native open cannot trigger ForegroundServiceDidNotStartInTime.
        startForeground(NOTIFICATION_ID, notification(), ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC)
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (intent?.action == ACTION_CANCEL_TRANSFER) FileTransferManager.cancel()
        startForeground(NOTIFICATION_ID, notification(), ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC)
        if (!UnlockedVolumeService.requiresForegroundService()) stopSelf(startId)
        return START_NOT_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? = null

    private fun notification(): Notification {
        val snapshot = UnlockedVolumeService.foregroundSnapshot()
        val transfer = FileTransferManager.notificationProgress()
        val manager = getSystemService(NotificationManager::class.java)
        manager.createNotificationChannel(
            NotificationChannel(CHANNEL_ID, getString(R.string.vc_foreground_channel), NotificationManager.IMPORTANCE_LOW),
        )
        val text = when {
            snapshot.unlockCount == 1 -> getString(R.string.vc_foreground_unlocking_one, unlockStageLabel(snapshot.unlockStage))
            snapshot.unlockCount > 1 -> getString(R.string.vc_foreground_unlocking_many, snapshot.unlockCount)
            transfer != null -> getString(
                if (transfer.direction == TransferDirection.ENCRYPT_IMPORT) R.string.vc_transfer_encrypt_notification else R.string.vc_transfer_decrypt_notification,
                transfer.currentFileName,
                transfer.currentFileIndex,
                transfer.totalFileCount,
            )
            snapshot.operationCount > 0 -> getString(R.string.vc_foreground_processing)
            snapshot.volumeCount == 1 -> getString(R.string.vc_foreground_one_volume)
            else -> getString(R.string.vc_foreground_many_volumes, snapshot.volumeCount)
        }
        return Notification.Builder(this, CHANNEL_ID)
            .setSmallIcon(android.R.drawable.stat_sys_warning)
            .setContentTitle(getString(R.string.vc_foreground_title))
            .setContentText(text)
            .setCategory(Notification.CATEGORY_SERVICE)
            .setOngoing(true)
            .setVisibility(Notification.VISIBILITY_SECRET)
            .apply {
                if (snapshot.unlockCount > 0) {
                    setProgress(0, 0, true)
                } else if (transfer != null) {
                    setProgress(100, transfer.percent ?: 0, transfer.percent == null)
                }
                if (transfer != null) {
                    addAction(Notification.Action.Builder(
                        null,
                        getString(R.string.vc_transfer_cancel),
                        PendingIntent.getService(
                            this@VolumeForegroundService,
                            1,
                            Intent(this@VolumeForegroundService, VolumeForegroundService::class.java).setAction(ACTION_CANCEL_TRANSFER),
                            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE,
                        ),
                    ).build())
                }
            }
            .build()
    }

    private fun unlockStageLabel(stage: VolumeUnlockStage?): String = getString(when (stage) {
        VolumeUnlockStage.PREPARING_INPUTS, null -> R.string.vc_unlock_preparing
        VolumeUnlockStage.DERIVING_AND_VERIFYING -> R.string.vc_unlock_deriving
        VolumeUnlockStage.MOUNTING_FILESYSTEM -> R.string.vc_unlock_mounting
        VolumeUnlockStage.REGISTERING_PROVIDER -> R.string.vc_unlock_registering
    })

    companion object {
        private const val CHANNEL_ID = "unlocked_volumes"
        private const val NOTIFICATION_ID = 0x5643
        private const val ACTION_CANCEL_TRANSFER = "app.rongvault.action.CANCEL_TRANSFER"

        fun refresh(context: Context, required: Boolean) {
            val intent = Intent(context, VolumeForegroundService::class.java)
            if (required) ContextCompat.startForegroundService(context, intent) else context.stopService(intent)
        }
    }
}
