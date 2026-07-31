package org.eds.veracrypt.documents

import android.content.Context
import android.os.PowerManager
import kotlinx.coroutines.sync.Semaphore
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import org.eds.veracrypt.nativecore.VcCore

/**
 * Bounded scheduler shared by file-level transfer jobs. Native topology is
 * the primary budget; thermal pressure immediately reduces new work to one
 * permit, while queued work remains cancellable and ordered by its caller.
 */
internal class ParallelTransferScheduler(
    private val context: Context?,
    private val workerCountProvider: () -> Int = { VcCore.nativeRecommendedWorkerCount() },
    private val thermalStatusProvider: () -> Int? = {
        context?.getSystemService(PowerManager::class.java)?.currentThermalStatus
    },
) {
    val workerCount: Int
        get() = currentWorkerCount()

    private val permits = Semaphore(currentWorkerCount())
    private val thermalSerial = Mutex()

    suspend fun <T> withPermit(block: suspend () -> T): T {
        if (isThermallyConstrained()) {
            if (context != null) android.util.Log.w(TAG, "transfer scheduler degraded=thermal_serial")
            return thermalSerial.withLock { block() }
        }
        permits.acquire()
        return try {
            block()
        } finally {
            permits.release()
        }
    }

    private fun currentWorkerCount(): Int {
        if (isThermallyConstrained()) return 1
        return runCatching { workerCountProvider() }
            .onFailure { if (context != null) android.util.Log.w(TAG, "transfer scheduler degraded=native_worker_count reason=${it.message}") }
            .getOrElse { Runtime.getRuntime().availableProcessors().coerceAtLeast(1) }
            .coerceIn(1, 8)
    }

    private fun isThermallyConstrained(): Boolean {
        val thermal = thermalStatusProvider()
        return thermal != null && thermal >= PowerManager.THERMAL_STATUS_SEVERE
    }

    private companion object {
        const val TAG = "RongVaultTransfer"
    }
}
