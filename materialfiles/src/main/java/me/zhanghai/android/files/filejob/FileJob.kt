/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filejob

import me.zhanghai.android.files.util.showToast
import java.io.IOException
import java.io.InterruptedIOException
import java.util.Random

/** Outcome emitted after a foreground file job has really finished. */
data class FileJobResult(
    val id: Int,
    val error: Exception? = null,
    val cancelled: Boolean = false,
) {
    val isSuccess: Boolean get() = error == null && !cancelled
}

abstract class FileJob {
    val id = Random().nextInt()

    internal lateinit var service: FileJobService
        private set

    fun runOn(service: FileJobService): FileJobResult {
        this.service = service
        var result = FileJobResult(id)
        try {
            run()
            // TODO: Toast
        } catch (e: InterruptedIOException) {
            // TODO
            e.printStackTrace()
            result = FileJobResult(id, e, cancelled = true)
        } catch (e: Exception) {
            e.printStackTrace()
            service.showToast(e.toString())
            result = FileJobResult(id, e)
        } finally {
            FileJobProgressRegistry.finish(id)
            service.notificationManager.cancel(id)
        }
        return result
    }

    @Throws(IOException::class)
    protected abstract fun run()
}
