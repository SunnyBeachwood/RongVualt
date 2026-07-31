package org.eds.veracrypt.documents

import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Test
import org.eds.veracrypt.domain.VolumeUnlockStage

class UnlockedVolumeServiceTest {
    @Test
    fun foregroundOperationBalancesOnFailureAndCancellation() = runTest {
        val initial = UnlockedVolumeService.foregroundSnapshot().operationCount

        try {
            UnlockedVolumeService.withForegroundOperation {
                assertEquals(initial + 1, UnlockedVolumeService.foregroundSnapshot().operationCount)
                error("expected failure")
            }
        } catch (_: IllegalStateException) {
            // Expected.
        }
        assertEquals(initial, UnlockedVolumeService.foregroundSnapshot().operationCount)

        try {
            UnlockedVolumeService.withForegroundOperation {
                throw CancellationException("expected cancellation")
            }
        } catch (_: CancellationException) {
            // Expected.
        }
        assertEquals(initial, UnlockedVolumeService.foregroundSnapshot().operationCount)
    }

    @Test
    fun nestedForegroundOperationsKeepTheOuterLeaseUntilItFinishes() = runTest {
        val initial = UnlockedVolumeService.foregroundSnapshot().operationCount

        UnlockedVolumeService.withForegroundOperation {
            assertEquals(initial + 1, UnlockedVolumeService.foregroundSnapshot().operationCount)
            UnlockedVolumeService.withForegroundOperation {
                assertEquals(initial + 2, UnlockedVolumeService.foregroundSnapshot().operationCount)
            }
            assertEquals(initial + 1, UnlockedVolumeService.foregroundSnapshot().operationCount)
        }

        assertEquals(initial, UnlockedVolumeService.foregroundSnapshot().operationCount)
    }

    @Test
    fun unlockOperationsExposeSafeCountAndStageAndAlwaysRelease() = runTest {
        val before = UnlockedVolumeService.foregroundSnapshot()

        UnlockedVolumeService.withUnlockOperation { first ->
            assertEquals(before.unlockCount + 1, UnlockedVolumeService.foregroundSnapshot().unlockCount)
            first.update(VolumeUnlockStage.DERIVING_AND_VERIFYING)
            assertEquals(VolumeUnlockStage.DERIVING_AND_VERIFYING, UnlockedVolumeService.foregroundSnapshot().unlockStage)
            UnlockedVolumeService.withUnlockOperation {
                assertEquals(before.unlockCount + 2, UnlockedVolumeService.foregroundSnapshot().unlockCount)
                assertEquals(null, UnlockedVolumeService.foregroundSnapshot().unlockStage)
            }
        }

        assertEquals(before.unlockCount, UnlockedVolumeService.foregroundSnapshot().unlockCount)
        assertEquals(before.operationCount, UnlockedVolumeService.foregroundSnapshot().operationCount)
    }
}
