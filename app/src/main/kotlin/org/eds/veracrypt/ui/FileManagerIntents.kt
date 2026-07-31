package org.eds.veracrypt.ui

import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Environment
import java8.nio.file.Paths
import me.zhanghai.android.files.filelist.FileListActivity
import me.zhanghai.android.files.provider.document.createDocumentTreeRootPath
import org.eds.veracrypt.documents.UnlockedVolumeService

/** Internal, provider-backed entry points for the embedded Material Files UI. */
object FileManagerIntents {
    fun picker(context: Context, initialTreeUri: Uri): Intent =
        MaterialFilesContainerPickerActivity.intent(context, initialTreeUri)

    /** Opens the regular browser at the app-accessible internal shared-storage root. */
    fun browse(context: Context): Intent {
        // A synthetic ExternalStorage DocumentsProvider tree is not an authorization grant.
        // Start at the ordinary shared-storage path instead, which is accessible when the
        // user enabled all-files access and produces a useful fallback otherwise.
        val sharedStorage = Paths.get(Environment.getExternalStorageDirectory().absolutePath)
        return FileListActivity.createViewIntent(sharedStorage)
            .putExtra(EXTRA_LAUNCH_MODE, FileManagerLaunchMode.BROWSE.name)
    }

    fun unlockedVolume(context: Context, treeUri: Uri): Intent {
        require(UnlockedVolumeService.isTreeUriOwnedByUnlockedVolume(context, treeUri)) {
            "Unlocked-volume browser requires a live EDS tree URI"
        }
        return FileListActivity.createViewIntent(treeUri.createDocumentTreeRootPath())
            .putExtra(EXTRA_LAUNCH_MODE, FileManagerLaunchMode.OPEN_UNLOCKED_VOLUME.name)
    }

    private const val EXTRA_LAUNCH_MODE = "org.eds.veracrypt.ui.launch_mode"
}
