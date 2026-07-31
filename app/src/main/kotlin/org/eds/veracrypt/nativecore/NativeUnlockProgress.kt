package org.eds.veracrypt.nativecore

import java.util.concurrent.atomic.AtomicBoolean

/** JNI callback checked between native header candidates; it carries no secrets. */
internal class NativeUnlockProgress(
    private val cancelled: AtomicBoolean = AtomicBoolean(false),
    private val onUpdate: (completed: Int, total: Int) -> Boolean = { _, _ -> true },
) {
    fun cancel() = cancelled.set(true)

    fun onProgress(completed: Int, total: Int): Boolean =
        !cancelled.get() && onUpdate(completed, total)
}
