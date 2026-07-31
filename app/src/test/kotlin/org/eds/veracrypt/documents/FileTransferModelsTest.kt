package org.eds.veracrypt.documents

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class FileTransferModelsTest {
    @Test fun knownTotalComputesBoundedPercent() {
        val calculator = TransferProgressCalculator { 1_000L }
        val result = calculator.update(TransferDirection.ENCRYPT_IMPORT, "a", 1, 2, 50, 100, 150, 200, emptyList())
        assertEquals(75, result.percent)
        assertEquals(150L, result.completedBytes)
    }

    @Test fun unknownTotalDoesNotInventPercentOrRemainingTime() {
        val calculator = TransferProgressCalculator { 1_000L }
        val result = calculator.update(TransferDirection.DECRYPT_EXPORT, "a", 1, 1, 50, null, 50, null, emptyList())
        assertNull(result.percent)
        assertNull(result.remainingSeconds)
    }

    @Test fun overflowAndNegativeInputsAreClamped() {
        val calculator = TransferProgressCalculator { 1_000L }
        val result = calculator.update(TransferDirection.ENCRYPT_IMPORT, "a", 1, 1, Long.MAX_VALUE, Long.MAX_VALUE, Long.MAX_VALUE, Long.MAX_VALUE, emptyList())
        assertEquals(100, result.percent)
        assertTrue(result.completedBytes >= 0L)
    }

    @Test fun duplicateNamesGetStableSuffixes() {
        assertEquals("photo (2).jpg", allocateTransferName("photo.jpg", setOf("photo.jpg", "photo (1).jpg")))
        assertEquals("notes (1)", allocateTransferName("notes", setOf("notes")))
    }

    @Test fun failuresRemainAttachedToProgress() {
        val calculator = TransferProgressCalculator { 1_000L }
        val result = calculator.update(TransferDirection.ENCRYPT_IMPORT, "a", 2, 3, 1, 1, 1, 3, listOf("b: source unavailable"))
        assertEquals(listOf("b: source unavailable"), result.failures)
    }

}
