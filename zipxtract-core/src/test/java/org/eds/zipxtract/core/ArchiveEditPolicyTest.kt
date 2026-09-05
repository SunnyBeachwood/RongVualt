package org.eds.zipxtract.core

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class ArchiveEditPolicyTest {
    @Test fun onlyUnencryptedSevenZipOnReplaceableProviderCanUpdate() {
        val probe = ArchiveProbe(ArchiveFormat.SEVEN_ZIP, "x.7z")
        assertTrue(ArchiveEditPolicy.canUpdate7z(probe, providerSupportsReplacement = true))
        assertFalse(ArchiveEditPolicy.canUpdate7z(probe, providerSupportsReplacement = false))
    }

    @Test fun encryptedAndOtherFormatsRemainReadOnly() {
        assertFalse(
            ArchiveEditPolicy.canUpdate7z(
                ArchiveProbe(ArchiveFormat.SEVEN_ZIP, "x.7z", ArchiveEncryption.HEADER),
                providerSupportsReplacement = true,
            ),
        )
        assertFalse(
            ArchiveEditPolicy.canUpdate7z(
                ArchiveProbe(ArchiveFormat.ZIP, "x.zip"),
                providerSupportsReplacement = true,
            ),
        )
    }
}
