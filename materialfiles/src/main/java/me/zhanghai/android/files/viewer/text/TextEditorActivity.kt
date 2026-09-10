/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.viewer.text

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.fragment.app.commit
import java.net.URI
import java8.nio.file.Paths
import me.zhanghai.android.files.app.AppActivity
import me.zhanghai.android.files.util.createIntent
import me.zhanghai.android.files.util.extraPath
import me.zhanghai.android.files.util.putArgs

class TextEditorActivity : AppActivity() {
    private var textEditorFragment: TextEditorFragment? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (intent.extraPath == null) {
            val uri = intent.data
            if (uri != null && (intent.action == Intent.ACTION_VIEW || intent.action == Intent.ACTION_EDIT)) {
                // ContentFileSystemProvider keeps the caller's temporary URI
                // grant and provides the same streaming editor path used by
                // files opened from inside Material Files.
                intent.extraPath = runCatching { Paths.get(URI.create(uri.toString())) }.getOrNull()
            }
        }
        // Calls ensureSubDecor().
        findViewById<android.view.View>(android.R.id.content)
        if (savedInstanceState == null) {
            textEditorFragment = TextEditorFragment().putArgs(TextEditorFragment.Args(intent))
            supportFragmentManager.commit { add(android.R.id.content, textEditorFragment!!) }
        } else {
            textEditorFragment = supportFragmentManager.findFragmentById(android.R.id.content)
                as TextEditorFragment
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        if (textEditorFragment?.onSupportNavigateUp() == true) {
            return true
        }
        return super.onSupportNavigateUp()
    }

    companion object {
        private const val EXTRA_FORCE_TEXT_EDITOR =
            "me.zhanghai.android.files.viewer.text.extra.FORCE_TEXT_EDITOR"

        fun createIntent(path: java8.nio.file.Path, forceTextEditor: Boolean = false): Intent =
            TextEditorActivity::class.createIntent().apply {
                extraPath = path
                val uri = Uri.parse(path.toUri().toString())
                if (uri.scheme == "content") {
                    data = uri
                    addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
                }
                putExtra(EXTRA_FORCE_TEXT_EDITOR, forceTextEditor)
            }
    }
}
