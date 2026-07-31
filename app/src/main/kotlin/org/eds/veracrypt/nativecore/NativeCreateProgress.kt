package org.eds.veracrypt.nativecore

import java.util.concurrent.atomic.AtomicBoolean

/** JNI callback for native full-format loops. Returning false requests cancellation. */
internal class NativeCreateProgress(
    private val cancelled: AtomicBoolean = AtomicBoolean(false),
    private val onUpdate: (stage: Int, completedBytes: Long, totalBytes: Long) -> Boolean = { _, _, _ -> true },
) {
    fun cancel() {
        cancelled.set(true)
    }

    fun onProgress(stage: Int, completedBytes: Long, totalBytes: Long): Boolean {
        return !cancelled.get() && onUpdate(stage, completedBytes, totalBytes)
    }

    companion object {
        fun inert() = NativeCreateProgress()
    }
}
