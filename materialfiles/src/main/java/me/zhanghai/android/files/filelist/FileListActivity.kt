/*
 * Copyright (c) 2018 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filelist

import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.os.Environment
import android.view.KeyEvent
import android.view.View
import androidx.activity.result.contract.ActivityResultContract
import androidx.fragment.app.commit
import androidx.fragment.app.Fragment
import java8.nio.file.Path
import java8.nio.file.Paths
import me.zhanghai.android.files.app.AppActivity
import me.zhanghai.android.files.file.MimeType
import me.zhanghai.android.files.util.createIntent
import me.zhanghai.android.files.util.extraPath
import me.zhanghai.android.files.util.extraPathList
import me.zhanghai.android.files.util.putArgs

class FileListActivity : AppActivity() {
    private lateinit var fragment: Fragment

    override fun onCreate(savedInstanceState: Bundle?) {
        if (savedInstanceState == null &&
            (intent.action == Intent.ACTION_SEND || intent.action == Intent.ACTION_SEND_MULTIPLE)
        ) {
            val paths = intent.extraPathList
            if (paths.isNotEmpty()) {
                val parent = paths.mapNotNull { it.parent }.distinct().singleOrNull()
                parent?.let { intent.extraPath = it }
                intent.putExtra(EXTRA_EXTERNAL_CREATE_ARCHIVE, true)
                intent.extraPathList = paths
            }
        }
        if (intent.extraPath == null &&
            (intent.action == null || intent.action == Intent.ACTION_VIEW)
        ) {
            @Suppress("DEPRECATION")
            val sharedStorage = Paths.get(Environment.getExternalStorageDirectory().absolutePath)
            intent.extraPath = sharedStorage
        }
        super.onCreate(savedInstanceState)

        // Calls ensureSubDecor().
        findViewById<View>(android.R.id.content)
        if (savedInstanceState == null) {
            fragment = if (shouldUseDualPane(intent)) {
                DualPaneFileListFragment().putArgs(FileListFragment.Args(intent))
            } else {
                FileListFragment().putArgs(FileListFragment.Args(intent))
            }
            supportFragmentManager.commit { add(android.R.id.content, fragment) }
        } else {
            fragment = supportFragmentManager.findFragmentById(android.R.id.content)!!
        }
    }

    override fun onKeyShortcut(keyCode: Int, event: KeyEvent): Boolean {
        if (fragment is DualPaneFileListFragment && fragment.onKeyShortcut(keyCode, event)) {
            return true
        }
        if (fragment is FileListFragment && fragment.onKeyShortcut(keyCode, event)) {
            return true
        }
        return super.onKeyUp(keyCode, event)
    }

    companion object {
        const val EXTRA_EXTERNAL_CREATE_ARCHIVE =
            "me.zhanghai.android.files.filelist.extra.EXTERNAL_CREATE_ARCHIVE"

        fun createViewIntent(path: Path): Intent =
            FileListActivity::class.createIntent()
                .setAction(Intent.ACTION_VIEW)
                .apply { extraPath = path }

        private fun shouldUseDualPane(intent: Intent): Boolean = when (intent.action) {
            Intent.ACTION_GET_CONTENT,
            Intent.ACTION_OPEN_DOCUMENT,
            Intent.ACTION_CREATE_DOCUMENT,
            Intent.ACTION_OPEN_DOCUMENT_TREE,
            Intent.ACTION_SEND,
            Intent.ACTION_SEND_MULTIPLE -> false
            else -> true
        }
    }

    class OpenFileContract : ActivityResultContract<List<MimeType>, Path?>() {
        override fun createIntent(context: Context, input: List<MimeType>): Intent =
            FileListActivity::class.createIntent()
                .setAction(Intent.ACTION_OPEN_DOCUMENT)
                .setType(MimeType.ANY.value)
                .addCategory(Intent.CATEGORY_OPENABLE)
                .putExtra(Intent.EXTRA_MIME_TYPES, input.map { it.value }.toTypedArray())

        override fun parseResult(resultCode: Int, intent: Intent?): Path? =
            if (resultCode == RESULT_OK) intent?.extraPath else null
    }

    class CreateFileContract : ActivityResultContract<Triple<MimeType, String?, Path?>, Path?>() {
        override fun createIntent(
            context: Context,
            input: Triple<MimeType, String?, Path?>
        ): Intent =
            FileListActivity::class.createIntent()
                .setAction(Intent.ACTION_CREATE_DOCUMENT)
                .setType(input.first.value)
                .addCategory(Intent.CATEGORY_OPENABLE)
                .apply {
                    input.second?.let { putExtra(Intent.EXTRA_TITLE, it) }
                    input.third?.let { extraPath = it }
                }

        override fun parseResult(resultCode: Int, intent: Intent?): Path? =
            if (resultCode == RESULT_OK) intent?.extraPath else null
    }

    class OpenDirectoryContract : ActivityResultContract<Path?, Path?>() {
        override fun createIntent(context: Context, input: Path?): Intent =
            FileListActivity::class.createIntent()
                .setAction(Intent.ACTION_OPEN_DOCUMENT_TREE)
                .apply { input?.let { extraPath = it } }

        override fun parseResult(resultCode: Int, intent: Intent?): Path? =
            if (resultCode == RESULT_OK) intent?.extraPath else null
    }
}
