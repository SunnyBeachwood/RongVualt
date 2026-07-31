package org.eds.veracrypt.credentials

import org.junit.Test

class CredentialVaultContractTest {
    @Test
    fun recordIdsAreRestrictedToOpaqueTokens() {
        val regex = Regex("[A-Za-z0-9_-]{1,128}")
        check(regex.matches("a0_-"))
        check(!regex.matches("content://provider/container"))
        check(!regex.matches("../container"))
    }
}
