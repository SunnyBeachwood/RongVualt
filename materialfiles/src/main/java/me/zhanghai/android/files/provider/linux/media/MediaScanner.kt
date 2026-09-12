/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.provider.linux.media

import android.media.MediaScannerConnection
import android.mtp.MtpConstants
import android.os.Build
import android.os.Handler
import android.os.HandlerThread
import android.provider.MediaStore
import androidx.annotation.RequiresApi
import java8.nio.channels.FileChannel
import me.zhanghai.android.files.app.application
import me.zhanghai.android.files.app.contentResolver
import me.zhanghai.android.files.hiddenapi.RestrictedHiddenApi
import me.zhanghai.android.files.provider.common.DelegateFileChannel
import me.zhanghai.android.files.provider.root.isRunningAsRoot
import me.zhanghai.android.files.util.lazyReflectedMethod
import java.io.File
import java.io.IOException
import java.util.concurrent.atomic.AtomicInteger

/*
 * @see com.android.internal.content.FileSystemProvider
 * @see com.android.providers.media.scan.ModernMediaScanner.java
 */
object MediaScanner {
    private const val BATCH_SIZE = 128
    private val scanThread by lazy {
        HandlerThread("RongVualtMediaScanner").apply { start() }
    }
    private val scanHandler by lazy { Handler(scanThread.looper) }
    private val pendingScans = LinkedHashMap<String, Boolean>()
    private var scanInFlight = false

    fun scan(file: File, isDeleted: Boolean = false) {
        if (isRunningAsRoot) {
            return
        }
        scanHandler.post {
            pendingScans[file.path] = isDeleted
            if (!scanInFlight) drainScanBatch()
        }
    }

    /**
     * MediaScannerConnection allocates a connection and callback graph for every invocation.
     * Copying thousands of small files used to enqueue thousands of those graphs at once and
     * could exhaust Android's 256 MiB app heap. Keep one bounded batch in flight instead.
     */
    private fun drainScanBatch() {
        if (scanInFlight || pendingScans.isEmpty()) return
        val batch = LinkedHashMap<String, Boolean>()
        val iterator = pendingScans.iterator()
        repeat(minOf(BATCH_SIZE, pendingScans.size)) {
            val entry = iterator.next()
            batch[entry.key] = entry.value
            iterator.remove()
        }
        scanInFlight = true
        val remaining = AtomicInteger(batch.size)
        MediaScannerConnection.scanFile(application, batch.keys.toTypedArray(), null) { path, _ ->
            if (Build.VERSION.SDK_INT == Build.VERSION_CODES.Q && batch[path] == true) {
                deleteMediaStoreEntryAsync(File(path))
            }
            if (remaining.decrementAndGet() == 0) {
                scanHandler.post {
                    scanInFlight = false
                    drainScanBatch()
                }
            }
        }
    }

    @get:RequiresApi(Build.VERSION_CODES.Q)
    private val deleteMediaStoreEntryHandler by lazy {
        val thread = HandlerThread("DeleteMediaStoreEntry")
        thread.start()
        Handler(thread.looper)
    }

    @RequiresApi(Build.VERSION_CODES.Q)
    private fun deleteMediaStoreEntryAsync(file: File) {
        deleteMediaStoreEntryHandler.post {
            try {
                deleteMediaStoreEntrySync(file)
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    @RestrictedHiddenApi
    @get:RequiresApi(Build.VERSION_CODES.Q)
    private val mediaStoreGetVolumeName by lazyReflectedMethod(
        MediaStore::class.java, "getVolumeName", File::class.java
    )

    // @see com.android.providers.media.scan.ModernMediaScanner.reconcileAndClean
    // @see https://android.googlesource.com/platform/packages/providers/MediaProvider/+/android10-release/src/com/android/providers/media/scan/ModernMediaScanner.java
    // @see https://android.googlesource.com/platform/packages/providers/MediaProvider/+/android11-release/src/com/android/providers/media/scan/ModernMediaScanner.java
    @RequiresApi(Build.VERSION_CODES.Q)
    private fun deleteMediaStoreEntrySync(file: File) {
        val file = file.canonicalFile
        val volumeName = mediaStoreGetVolumeName.invoke(null, file) as String
        val uri = MediaStore.Files.getContentUri(volumeName)
            .buildUpon()
            .appendQueryParameter("includePending", "1")
            .appendQueryParameter("deletedata", "false")
            .build()
        @Suppress("DEPRECATION")
        val where = "ifnull(format, ${MtpConstants.FORMAT_UNDEFINED}) != ${
            MtpConstants.FORMAT_ABSTRACT_AV_PLAYLIST} AND ${MediaStore.Files.FileColumns.DATA} = ?"
        val selectionArgs = arrayOf(file.absolutePath)
        contentResolver.delete(uri, where, selectionArgs)
    }

    fun createScanOnCloseFileChannel(fileChannel: FileChannel, file: File): FileChannel =
        if (isRunningAsRoot) {
            fileChannel
        } else {
            object : DelegateFileChannel(fileChannel) {
                @Throws(IOException::class)
                override fun implCloseChannel() {
                    super.implCloseChannel()

                    scan(file)
                }
            }
        }
}
