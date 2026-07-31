/*
 * Copyright (c) 2026
 */

package me.zhanghai.android.files.viewer.markdown

import android.os.Bundle
import android.view.View
import androidx.fragment.app.commit
import java8.nio.file.Path
import me.zhanghai.android.files.app.AppActivity
import me.zhanghai.android.files.util.createIntent
import me.zhanghai.android.files.util.extraPath

class MarkdownViewerActivity : AppActivity() {
    private lateinit var fragment: MarkdownViewerFragment

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (intent.extraPath == null) {
            finish()
            return
        }
        findViewById<View>(android.R.id.content)
        if (savedInstanceState == null) {
            fragment = MarkdownViewerFragment()
            supportFragmentManager.commit { add(android.R.id.content, fragment) }
        } else {
            fragment = supportFragmentManager.findFragmentById(android.R.id.content)
                as MarkdownViewerFragment
        }
    }

    override fun onSupportNavigateUp(): Boolean = fragment.onSupportNavigateUp() ||
        super.onSupportNavigateUp()

    companion object {
        fun createIntent(path: Path) = MarkdownViewerActivity::class.createIntent().apply {
            extraPath = path
        }
    }
}
