package org.eds.veracrypt.domain

import org.junit.Test

class VolumeCreateOptionsTest {
    @Test
    fun upstreamReadOnlySuiteCannotBeUsedForCreation() {
        try {
            VolumeCreateOptions(
                sizeBytes = 1_048_576,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.CAMELLIA,
            )
            throw AssertionError("Camellia creation should be rejected by the 1.26.29 matrix")
        } catch (_: IllegalArgumentException) {
            // Expected.
        }
    }

    @Test
    fun cascadeIsRejectedByTheFirstReleaseScope() {
        try {
            VolumeCreateOptions(
                sizeBytes = 1_048_576,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.SERPENT_TWOFISH_AES,
            )
            throw AssertionError("Cascaded cipher creation must be outside the first-release scope")
        } catch (_: IllegalArgumentException) {
            // Expected.
        }
    }
}
