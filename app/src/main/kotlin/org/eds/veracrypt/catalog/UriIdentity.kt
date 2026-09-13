package org.eds.veracrypt.catalog

import android.net.Uri
import android.provider.DocumentsContract

/** Compares SAF aliases by document identity, not only by their spelling. */
internal object UriIdentity {
    fun same(first: Uri, second: Uri): Boolean = keys(first).intersect(keys(second)).isNotEmpty()

    private fun keys(uri: Uri): Set<String> = buildSet {
        add(uri.normalizeScheme().buildUpon().clearQuery().fragment(null).build().toString())
        val authority = uri.authority ?: return@buildSet
        val documentId = runCatching { DocumentsContract.getDocumentId(uri) }.getOrNull()
            ?: runCatching { DocumentsContract.getTreeDocumentId(uri) }.getOrNull()
        documentId?.let { add("document:$authority:$it") }
    }
}
