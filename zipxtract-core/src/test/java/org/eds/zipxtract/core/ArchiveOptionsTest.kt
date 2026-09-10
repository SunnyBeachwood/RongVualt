package org.eds.zipxtract.core

import org.junit.Test

class ArchiveOptionsTest {
    @Test(expected = IllegalArgumentException::class)
    fun splitArchiveHasMinimumVolumeSize() {
        ArchiveCreateOptions(zipSplitSizeBytes = 63L * 1024L)
            .validate(ArchiveFormat.ZIP, null)
    }

    @Test(expected = ArchivePasswordException::class)
    fun encryptionRequiresEphemeralPassword() {
        ArchiveCreateOptions(zipEncryption = ArchiveEncryption.AES)
            .validate(ArchiveFormat.ZIP, null)
    }

    @Test(expected = IllegalArgumentException::class)
    fun tooManyVolumesNeedConfirmation() {
        ArchiveCreateOptions(zipSplitSizeBytes = 64L * 1024L)
            .validate(ArchiveFormat.ZIP, null, 101L * 64L * 1024L)
    }

    @Test fun explicitConfirmationAllowsLargeSplitSet() {
        ArchiveCreateOptions(
            zipSplitSizeBytes = 64L * 1024L,
            allowLargeZipSplit = true,
        ).validate(ArchiveFormat.ZIP, null, 101L * 64L * 1024L)
    }

    @Test(expected = UnsupportedArchiveException::class)
    fun nonZipFormatsRejectZipEncryption() {
        ArchiveCreateOptions(zipEncryption = ArchiveEncryption.AES)
            .validate(ArchiveFormat.SEVEN_ZIP, "secret".toCharArray())
    }

    @Test(expected = UnsupportedArchiveException::class)
    fun tarDoesNotAcceptAPassword() {
        ArchiveCreateOptions().validate(ArchiveFormat.TAR, "secret".toCharArray())
    }

    @Test(expected = IllegalArgumentException::class)
    fun aesKeySizeIsRestrictedToSupportedVariants() {
        ArchiveCreateOptions(zipAesKeyBits = 192).validate(ArchiveFormat.ZIP, null)
    }

    @Test(expected = IllegalArgumentException::class)
    fun sevenZipSplitHasMinimumVolumeSize() {
        ArchiveCreateOptions(sevenZipSplitSizeBytes = 63L * 1024L)
            .validate(ArchiveFormat.SEVEN_ZIP, null)
    }

    @Test(expected = IllegalArgumentException::class)
    fun tooManySevenZipVolumesNeedConfirmation() {
        ArchiveCreateOptions(sevenZipSplitSizeBytes = 64L * 1024L)
            .validate(ArchiveFormat.SEVEN_ZIP, null, 101L * 64L * 1024L)
    }

    @Test fun sevenZipHeaderEncryptionRequiresPassword() {
        try {
            ArchiveCreateOptions(sevenZipEncryptHeaders = true)
                .validate(ArchiveFormat.SEVEN_ZIP, null)
            throw AssertionError("Expected password validation failure")
        } catch (_: ArchivePasswordException) {
            // expected
        }
    }

    @Test(expected = UnsupportedArchiveException::class)
    fun sevenZipHeaderEncryptionIsRejectedForZip() {
        ArchiveCreateOptions(sevenZipEncryptHeaders = true)
            .validate(ArchiveFormat.ZIP, null)
    }
}
