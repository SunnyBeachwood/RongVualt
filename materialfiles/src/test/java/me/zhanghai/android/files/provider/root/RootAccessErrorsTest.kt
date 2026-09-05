/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package me.zhanghai.android.files.provider.root

import org.junit.Assert.assertEquals
import org.junit.Assert.assertSame
import org.junit.Test

class RootAccessErrorsTest {
    @Test
    fun reportsUnavailableRootExplicitly() {
        assertEquals("Root access is unavailable", RootAccessErrors.unavailable().message)
    }

    @Test
    fun preservesDeniedCauseAndMessage() {
        val cause = SecurityException("permission denied")
        val error = RootAccessErrors.denied(cause)
        assertEquals("Root access was denied", error.message)
        assertSame(cause, error.cause)
    }

    @Test
    fun preservesTimeoutCauseAndMessage() {
        val cause = IllegalStateException("shell did not respond")
        val error = RootAccessErrors.timedOut(cause)
        assertEquals("Root service timed out", error.message)
        assertSame(cause, error.cause)
    }

    @Test
    fun reportsARevokedRootServiceExplicitly() {
        assertEquals("Root service was disconnected", RootAccessErrors.disconnected().message)
    }
}
