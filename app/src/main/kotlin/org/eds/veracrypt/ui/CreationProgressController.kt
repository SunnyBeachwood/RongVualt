package org.eds.veracrypt.ui

import android.view.View
import java.util.concurrent.atomic.AtomicBoolean
import org.eds.veracrypt.domain.VolumeCreateProgress

/** Bridges native creation callbacks to the main thread without retaining secrets. */
internal class CreationProgressController(
    private val root: View,
    private val update: (stage: Int, completedBytes: Long, totalBytes: Long) -> Unit,
) {
    private val cancelled = AtomicBoolean(false)

    val reporter = object : VolumeCreateProgress {
        override fun onProgress(stage: Int, completedBytes: Long, totalBytes: Long) {
            root.post { update(stage, completedBytes, totalBytes) }
        }

        override fun isCancellationRequested(): Boolean = cancelled.get()
    }

    fun cancel() {
        cancelled.set(true)
    }
}
