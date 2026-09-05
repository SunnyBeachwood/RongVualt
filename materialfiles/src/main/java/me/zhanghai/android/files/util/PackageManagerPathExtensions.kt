/*
 * Copyright (c) 2022 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.util

import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import java8.nio.file.Path
import me.zhanghai.android.files.compat.getPackageArchiveInfoCompat
import me.zhanghai.android.files.app.application
import me.zhanghai.android.files.file.fileProviderUri
import me.zhanghai.android.files.provider.document.isDocumentPath
import me.zhanghai.android.files.provider.document.resolver.DocumentResolver
import me.zhanghai.android.files.provider.linux.isLinuxPath
import me.zhanghai.android.files.provider.root.shouldUseRoot
import java.io.Closeable
import java.io.IOException

val Path.isGetPackageArchiveInfoCompatible: Boolean
    get() = isLinuxPath || isDocumentPath

fun PackageManager.getPackageArchiveInfoCompat(
    path: Path,
    flags: Int
): Pair<PackageInfo?, Closeable?> {
    val archiveFilePath: String
    val closeable: Closeable?
    when {
        path.isLinuxPath -> {
            if (shouldUseRoot(path)) {
                val pfd = application.contentResolver.openFileDescriptor(path.fileProviderUri, "r")
                    ?: throw IOException("Unable to open Root APK descriptor")
                archiveFilePath = "/proc/self/fd/${pfd.fd}"
                closeable = pfd
            } else {
                archiveFilePath = path.toFile().path
                closeable = null
            }
        }
        path.isDocumentPath -> {
            val pfd = DocumentResolver.openParcelFileDescriptor(path as DocumentResolver.Path, "r")
            archiveFilePath = "/proc/self/fd/${pfd.fd}"
            closeable = pfd
        }
        else -> throw IllegalArgumentException(path.toString())
    }
    var successful = false
    val packageInfo: PackageInfo?
    try {
        packageInfo = getPackageArchiveInfoCompat(archiveFilePath, flags)?.apply {
            applicationInfo?.apply {
                sourceDir = archiveFilePath
                publicSourceDir = archiveFilePath
            }
        }
        successful = true
    } finally {
        if (!successful) {
            closeable?.close()
        }
    }
    return packageInfo to closeable
}
