/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filejob

import android.os.Handler
import android.os.Looper
import java.util.concurrent.CopyOnWriteArraySet

/** Process-local progress snapshot for foreground file jobs shown by the browser. */
enum class FileJobOperation { COPY, MOVE, DELETE, ARCHIVE, EXTRACT }

enum class FileJobProgressPhase { PREPARING, RUNNING, CANCELLING }

data class FileJobProgress(
    val id: Int,
    val operation: FileJobOperation,
    val title: CharSequence,
    val completedBytes: Long,
    val totalBytes: Long,
    val completedEntries: Long,
    val totalEntries: Long,
    val currentFile: CharSequence? = null,
    val phase: FileJobProgressPhase = FileJobProgressPhase.RUNNING,
) {
    val indeterminate: Boolean
        get() = totalBytes <= 0L && totalEntries <= 0L

    val percent: Int?
        get() = when {
            totalBytes > 0L -> (completedBytes * 100L / totalBytes)
                .coerceIn(0L, 100L).toInt()
            totalEntries > 0L -> (completedEntries * 100L / totalEntries)
                .coerceIn(0L, 100L).toInt()
            else -> null
        }
}

/**
 * Keeps foreground progress available while the service and the browser have
 * different lifecycles. Notifications remain the durable background surface;
 * this registry is only for the optional in-app progress dialog. Archive and
 * delete jobs share it so the UI has one ordered progress surface.
 */
object FileJobProgressRegistry {
    private val mainHandler = Handler(Looper.getMainLooper())
    private val values = LinkedHashMap<Int, FileJobProgress>()
    private val listeners = CopyOnWriteArraySet<(List<FileJobProgress>) -> Unit>()

    fun update(progress: FileJobProgress) {
        mainHandler.post {
            values[progress.id] = progress
            dispatch()
        }
    }

    fun markCancelling(id: Int) {
        mainHandler.post {
            val progress = values[id] ?: return@post
            values[id] = progress.copy(phase = FileJobProgressPhase.CANCELLING)
            dispatch()
        }
    }

    fun finish(id: Int) {
        mainHandler.post {
            values.remove(id)
            dispatch()
        }
    }

    fun addListener(listener: (List<FileJobProgress>) -> Unit) {
        listeners += listener
        mainHandler.post { listener(values.values.toList()) }
    }

    fun removeListener(listener: (List<FileJobProgress>) -> Unit) {
        listeners -= listener
    }

    private fun dispatch() {
        val snapshot = values.values.toList()
        listeners.forEach { listener -> runCatching { listener(snapshot) } }
    }
}

@Deprecated("Use FileJobProgress")
typealias ArchiveJobProgress = FileJobProgress

@Deprecated("Use FileJobProgressRegistry")
val ArchiveJobProgressRegistry: FileJobProgressRegistry
    get() = FileJobProgressRegistry
