/*
 * Copyright (c) 2026
 */

package me.zhanghai.android.files.viewer.markdown

import android.os.Bundle
import java8.nio.file.Path
import me.zhanghai.android.files.app.AppActivity
import me.zhanghai.android.files.util.createIntent
import me.zhanghai.android.files.util.extraPath
import me.zhanghai.android.files.viewer.text.TextEditorActivity

class MarkdownViewerActivity : AppActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val path = intent.extraPath
        if (path == null) {
            finish()
            return
        }
        // Kept as a compatibility entry point for old internal intents. The
        // actual UI now lives in the unified text editor so source and preview
        // share one buffer and one unsaved-change state.
        startActivity(TextEditorActivity.createIntent(path))
        finish()
    }

    companion object {
        fun createIntent(path: Path) = MarkdownViewerActivity::class.createIntent().apply {
            extraPath = path
        }
    }
}
