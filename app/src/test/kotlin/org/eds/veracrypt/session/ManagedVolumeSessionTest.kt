package org.eds.veracrypt.session

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.TestScope
import kotlinx.coroutines.test.advanceTimeBy
import kotlinx.coroutines.test.runCurrent
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeSessionState
import org.eds.veracrypt.domain.VolumeKind
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class ManagedVolumeSessionTest {
    @Test
    fun ntfsSessionRejectsEveryMutationBeforeJniDispatch() {
        val session = ManagedVolumeSession(
            1L,
            VolumeAccessMode.READ_WRITE,
            VolumeKind.NORMAL,
            TestScope(StandardTestDispatcher()),
            initialFileSystem = VolumeFileSystem.NTFS,
            onClose = {},
        )

        assertReadOnlyRejected { session.openFile("entry", writable = true) }
        assertReadOnlyRejected { session.openFile("entry", writable = false, create = true) }
        assertReadOnlyRejected { session.openFile("entry", writable = false, truncate = true) }
        assertReadOnlyRejected { session.createDirectory("folder") }
        assertReadOnlyRejected { session.delete("entry") }
        assertReadOnlyRejected { session.rename("entry", "renamed") }
    }

    @Test
    fun autoLockClosesTheOpaqueSessionOnce() {
        val dispatcher = StandardTestDispatcher()
        val scope = TestScope(dispatcher)
        var closeCount = 0
        val session = ManagedVolumeSession(
            1L, VolumeAccessMode.READ_ONLY, VolumeKind.NORMAL, scope, onClose = { closeCount++ }
        )

        session.enableAutoLock(5_000)
        scope.advanceTimeBy(5_000)
        scope.runCurrent()

        check(closeCount == 1)
        check(session.state.value == VolumeSessionState.Closed)
        session.close()
        check(closeCount == 1)
    }

    @Test
    fun beforeCloseListenerRunsBeforeNativeCleanup() {
        val dispatcher = StandardTestDispatcher()
        val scope = TestScope(dispatcher)
        var nativeClosed = false
        var listenerSawOpenNativeState = false
        val session = ManagedVolumeSession(
            1L,
            VolumeAccessMode.READ_ONLY,
            VolumeKind.NORMAL,
            scope,
            onClose = { nativeClosed = true },
        )
        session.setBeforeCloseListener {
            listenerSawOpenNativeState = !nativeClosed
        }

        session.close()

        check(listenerSawOpenNativeState)
        check(nativeClosed)
    }

    @Test
    fun closingOuterSessionClosesDependentHiddenSession() {
        val dispatcher = StandardTestDispatcher()
        val scope = TestScope(dispatcher)
        var outerCloseCount = 0
        var hiddenCloseCount = 0
        val outer = ManagedVolumeSession(
            1L, VolumeAccessMode.READ_WRITE, VolumeKind.NORMAL, scope, onClose = { outerCloseCount++ }
        )
        val hidden = ManagedVolumeSession(
            2L, VolumeAccessMode.READ_WRITE, VolumeKind.HIDDEN, scope, onClose = { hiddenCloseCount++ }
        )

        outer.registerDependent(hidden)
        outer.close()

        check(outerCloseCount == 1)
        check(hiddenCloseCount == 1)
        check(outer.state.value == VolumeSessionState.Closed)
        check(hidden.state.value == VolumeSessionState.Closed)
    }

    private fun assertReadOnlyRejected(operation: () -> Unit) {
        try {
            operation()
            error("Expected a read-only operation to be rejected")
        } catch (_: IllegalArgumentException) {
            // The guard is deliberately before the JNI call.
        }
    }
}
