package org.eds.veracrypt.ui

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.DocumentsContract
import androidx.appcompat.app.AppCompatActivity
import me.zhanghai.android.files.file.MimeType
import me.zhanghai.android.files.filelist.FileListActivity
import me.zhanghai.android.files.provider.document.createDocumentTreeRootPath
import me.zhanghai.android.files.provider.document.documentUri
import me.zhanghai.android.files.file.fileProviderUri
import me.zhanghai.android.files.util.createIntent
import me.zhanghai.android.files.util.extraPath

/** Private bridge between Material Files' Path result and EDS' URI catalog. */
class MaterialFilesContainerPickerActivity : AppCompatActivity() {
    override fun onCreate(state: Bundle?) {
        super.onCreate(state)
        if (state != null) return
        val tree = intent.getStringExtra(EXTRA_TREE_URI)?.let(Uri::parse)
            ?: return finishCancelled()
        val root = runCatching {
            DocumentsContract.getTreeDocumentId(tree)
            tree.createDocumentTreeRootPath()
        }.getOrElse { return finishCancelled() }
        val picker = FileListActivity::class.createIntent()
            .setAction(Intent.ACTION_OPEN_DOCUMENT)
            .setType(MimeType.ANY.value)
            .addCategory(Intent.CATEGORY_OPENABLE)
            .apply { extraPath = root }
        startActivityForResult(picker, REQUEST_PICK)
    }

    @Deprecated("Compatibility bridge for the embedded Material Files picker")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != REQUEST_PICK || resultCode != RESULT_OK) return finishCancelled()
        // Document paths retain their provider URI. A regular local path has
        // no DocumentsProvider URI, so pass it through Material Files' private
        // non-exported FileProvider instead; the resolver below admits only
        // file:// origins from that provider.
        val uri = runCatching {
            data?.extraPath?.let { path ->
                runCatching { path.documentUri }.getOrElse { path.fileProviderUri }
            }
        }.getOrNull()
            ?: return finishCancelled()
        setResult(Activity.RESULT_OK, Intent().setData(uri).addFlags(
            Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION
        ))
        finish()
    }

    private fun finishCancelled() {
        setResult(Activity.RESULT_CANCELED)
        finish()
    }

    companion object {
        private const val REQUEST_PICK = 41
        private const val EXTRA_TREE_URI = "org.eds.veracrypt.ui.picker_tree_uri"

        fun intent(context: Context, treeUri: Uri): Intent =
            Intent(context, MaterialFilesContainerPickerActivity::class.java)
                .putExtra(EXTRA_TREE_URI, treeUri.toString())
    }
}
