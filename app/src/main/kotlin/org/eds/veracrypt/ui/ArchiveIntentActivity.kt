/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 */

package org.eds.veracrypt.ui

import android.app.Activity
import android.content.ClipData
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import java.net.URI
import java.util.Locale
import java8.nio.file.Path
import java8.nio.file.Paths
import me.zhanghai.android.files.file.MimeType
import me.zhanghai.android.files.file.asMimeTypeOrNull
import me.zhanghai.android.files.file.guessFromPath
import me.zhanghai.android.files.file.isSupportedArchive
import me.zhanghai.android.files.filelist.FileListActivity
import me.zhanghai.android.files.provider.archive.createArchiveRootPath
import me.zhanghai.android.files.util.createIntent
import me.zhanghai.android.files.util.extraPath
import me.zhanghai.android.files.util.extraPathList
import me.zhanghai.android.files.app.AppAccessSession

/**
 * Narrow, exported bridge for archive viewers and share sheets. FileListActivity
 * itself remains non-exported; all URI validation and grants happen here.
 */
class ArchiveIntentActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (!AppAccessSession.isAuthorized()) {
            finishWithError()
            return
        }
        if (savedInstanceState != null) return
        when (intent.action) {
            Intent.ACTION_VIEW -> openArchive()
            Intent.ACTION_SEND, Intent.ACTION_SEND_MULTIPLE -> openCreateArchive()
            else -> finishWithError()
        }
    }

    private fun openArchive() {
        val uri = intent.data
        if (uri == null || !intent.isArchiveView(uri)) return finishWithError()
        val path = uri?.toArchivePath() ?: return finishWithError()
        if (!isValidClipData(intent.clipData)) return finishWithError()
        // ACTION_VIEW must enter the read-only archive filesystem, just like a
        // tap on an archive in the embedded file list, rather than showing the
        // container file as an ordinary directory row.
        val archiveRoot = runCatching { path.createArchiveRootPath() }.getOrNull()
            ?: return finishWithError()
        val forward = FileListActivity.createViewIntent(archiveRoot).apply {
            addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
            // A VIEW sender is allowed to grant only through data or
            // ClipData.  The internal Path extra is deliberately not treated
            // as a grant-bearing URI, so keep the original object attached to
            // the forwarded intent as well.
            data = uri
            clipData = intent.clipData ?: ClipData.newRawUri(
                archiveRoot.fileName?.toString() ?: uri.toString(), uri,
            )
        }
        startActivity(forward)
        finish()
    }

    private fun openCreateArchive() {
        val uris = collectUris() ?: return finishWithError()
        val paths = uris.mapNotNull { it.toArchivePath() }.distinctBy { it.toString() }
        if (paths.isEmpty()) return finishWithError()
        val forward = FileListActivity::class.createIntent()
            .setAction(Intent.ACTION_SEND)
            .apply {
                extraPathList = paths
                val parent = paths.mapNotNull { it.parent }.distinct().singleOrNull()
                parent?.let { extraPath = it }
                putExtra(FileListActivity.EXTRA_EXTERNAL_CREATE_ARCHIVE, true)
                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                // EXTRA_STREAM alone does not reliably carry grants across a
                // second explicit startActivity() hop. Rebuild ClipData from
                // every validated URI so content-provider permissions survive
                // both SEND and SEND_MULTIPLE forwarding.
                clipData = uris.toClipData()
            }
        startActivity(forward)
        finish()
    }

    private fun collectUris(): List<Uri>? {
        val result = LinkedHashSet<Uri>()
        intent.data?.let(result::add)
        runCatching { intent.getParcelableExtra<Uri>(Intent.EXTRA_STREAM) }
            .getOrNull()?.let(result::add)
        runCatching { intent.getParcelableArrayListExtra<Uri>(Intent.EXTRA_STREAM) }
            .getOrNull()?.forEach(result::add)
        intent.clipData?.let { clip ->
            for (index in 0 until clip.itemCount) {
                val uri = clip.getItemAt(index).uri ?: return null
                result += uri
            }
        }
        return result.toList().takeIf { uris -> uris.isNotEmpty() && uris.all(::isSupportedUri) }
    }

    private fun Uri.toArchivePath(): Path? {
        if (!isSupportedUri(this)) return null
        // FileSystemProvider schemes are case-sensitive even though Android's
        // Uri scheme matching is not. Normalize before Paths.get() so a sender
        // using CONTENT:// or FILE:// cannot be rejected after validation.
        val normalizedScheme = scheme!!.lowercase(Locale.ROOT)
        val raw = toString()
        val normalizedRaw = raw.replaceFirst(
            Regex("^[^:]+:"),
            "$normalizedScheme:",
        )
        val parsed = runCatching { URI.create(normalizedRaw) }.getOrElse {
            // A few senders construct Uri values with unescaped spaces. Keep
            // the same component-wise recovery used by Material Files' own
            // intent path adapter before rejecting the input.
            runCatching { URI(normalizedScheme, userInfo, host, port, path, query, fragment) }
                .getOrNull()
        } ?: return null
        return runCatching { Paths.get(parsed) }.getOrNull()
    }

    private fun isValidClipData(clipData: ClipData?): Boolean {
        if (clipData == null) return true
        for (index in 0 until clipData.itemCount) {
            val uri = clipData.getItemAt(index).uri ?: return false
            if (!isSupportedUri(uri)) return false
        }
        return true
    }

    private fun isSupportedUri(uri: Uri): Boolean =
        uri.scheme?.equals("content", ignoreCase = true) == true ||
            uri.scheme?.equals("file", ignoreCase = true) == true

    private fun Intent.isArchiveView(uri: Uri): Boolean {
        val declaredMime = type?.asMimeTypeOrNull()
        if (declaredMime?.isSupportedArchive == true) return true
        val name = uri.lastPathSegment ?: return false
        return MimeType.guessFromPath(name).isSupportedArchive
    }

    private fun List<Uri>.toClipData(): ClipData {
        val first = first()
        return ClipData.newRawUri(first.lastPathSegment ?: first.toString(), first).also { clip ->
            drop(1).forEach { clip.addItem(ClipData.Item(it)) }
        }
    }

    private fun finishWithError() {
        setResult(RESULT_CANCELED)
        finish()
    }
}
