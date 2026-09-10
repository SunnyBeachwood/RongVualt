/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filejob

import android.os.Handler
import android.os.Looper
import java.util.concurrent.CopyOnWriteArraySet

/** Process-local progress snapshot for archive create/extract jobs. */
data class ArchiveJobProgress(
    val id: Int,
    val title: CharSequence,
    val completedBytes: Long,
    val totalBytes: Long,
    val completedEntries: Long,
    val totalEntries: Long,
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
 * this registry is only for the optional in-app progress dialog.
 */
object ArchiveJobProgressRegistry {
    private val mainHandler = Handler(Looper.getMainLooper())
    private val values = LinkedHashMap<Int, ArchiveJobProgress>()
    private val listeners = CopyOnWriteArraySet<(List<ArchiveJobProgress>) -> Unit>()

    fun update(progress: ArchiveJobProgress) {
        mainHandler.post {
            values[progress.id] = progress
            dispatch()
        }
    }

    fun finish(id: Int) {
        mainHandler.post {
            values.remove(id)
            dispatch()
        }
    }

    fun addListener(listener: (List<ArchiveJobProgress>) -> Unit) {
        listeners += listener
        mainHandler.post { listener(values.values.toList()) }
    }

    fun removeListener(listener: (List<ArchiveJobProgress>) -> Unit) {
        listeners -= listener
    }

    private fun dispatch() {
        val snapshot = values.values.toList()
        listeners.forEach { listener -> runCatching { listener(snapshot) } }
    }
}
