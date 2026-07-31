package org.eds.veracrypt.catalog

import android.content.Context
import android.net.Uri
import android.os.ParcelFileDescriptor
import android.content.ContentResolver
import java.io.File

/** Validates a picked container without copying or exposing its plaintext path. */
object ContainerSourceResolver {
    fun validate(context: Context, uri: Uri): String? {
        if (uri.scheme != "content" && uri.scheme != "file") return "Only local or SAF files are supported"
        if (uri.authority == "${context.packageName}.unlocked") return "A file inside an unlocked volume cannot be nested"
        if (uri.authority == "${context.packageName}.materialfiles.files" && !isPrivateLocalFileUri(uri)) {
            return "Only ordinary local files can be used as containers"
        }
        if (uri.scheme == "file") {
            val file = File(uri.path ?: return "Invalid local file")
            if (!file.isFile || !file.canRead()) return "File is not readable"
            return null
        }
        val descriptor = runCatching {
            context.contentResolver.openFileDescriptor(uri, "r")
        }.getOrNull() ?: return "File cannot be opened"
        descriptor.use { pfd ->
            if (!isSeekable(pfd)) return "Container must support random access"
        }
        return null
    }

    /** The private Material Files provider encodes its backing Path URI as its path. */
    private fun isPrivateLocalFileUri(uri: Uri): Boolean {
        val encodedPath = uri.path?.removePrefix("/") ?: return false
        val backingUri = runCatching { Uri.parse(Uri.decode(encodedPath)) }.getOrNull() ?: return false
        return backingUri.scheme == ContentResolver.SCHEME_FILE
    }

    private fun isSeekable(pfd: ParcelFileDescriptor): Boolean =
        runCatching {
            val position = android.system.Os.lseek(pfd.fileDescriptor, 0, android.system.OsConstants.SEEK_CUR)
            android.system.Os.lseek(pfd.fileDescriptor, position, android.system.OsConstants.SEEK_SET)
            true
        }.getOrDefault(false)
}
