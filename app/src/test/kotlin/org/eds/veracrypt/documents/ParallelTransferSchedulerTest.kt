package org.eds.veracrypt.documents

import java.util.concurrent.atomic.AtomicInteger
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.async
import kotlinx.coroutines.awaitAll
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Test

class ParallelTransferSchedulerTest {
    @Test
    fun severeThermalStatusSerializesConcurrentFiles() = runBlocking {
        val scheduler = ParallelTransferScheduler(null, { 4 }, { 3 })
        val active = AtomicInteger(0)
        val peak = AtomicInteger(0)
        (0 until 4).map {
            async(Dispatchers.Default) {
                scheduler.withPermit {
                    val now = active.incrementAndGet()
                    peak.updateAndGet { previous -> maxOf(previous, now) }
                    delay(10)
                    active.decrementAndGet()
                }
            }
        }.awaitAll()
        assertEquals(1, scheduler.workerCount)
        assertEquals(1, peak.get())
    }

    @Test
    fun normalThermalStatusUsesDetectedWorkerBudget() = runBlocking {
        val scheduler = ParallelTransferScheduler(null, { 3 }, { 0 })
        val active = AtomicInteger(0)
        val peak = AtomicInteger(0)
        (0 until 3).map {
            async(Dispatchers.Default) {
                scheduler.withPermit {
                    val now = active.incrementAndGet()
                    peak.updateAndGet { previous -> maxOf(previous, now) }
                    delay(10)
                    active.decrementAndGet()
                }
            }
        }.awaitAll()
        assertEquals(3, scheduler.workerCount)
        assertEquals(3, peak.get())
    }
}
