package org.eds.veracrypt.session

import java.util.UUID
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeSession
import org.eds.veracrypt.domain.VolumeSessionState
import org.junit.Test

class UnlockedVolumeManagerTest {
    @Test
    fun rejectsAnySecondVolumeForTheSameContainer() {
        val manager = UnlockedVolumeManager()
        val containerId = UUID.randomUUID()
        val firstReader = FakeSession(VolumeAccessMode.READ_ONLY)
        val secondReader = FakeSession(VolumeAccessMode.READ_ONLY)
        manager.add(containerId, "Reader 1", firstReader)

        try {
            manager.add(containerId, "Reader 2", secondReader)
            throw AssertionError("A container must expose only one live volume")
        } catch (_: IllegalStateException) {
            // Expected.
        }

        manager.closeContainer(containerId)
        check(firstReader.closeCount == 1)
        check(secondReader.closeCount == 0)

        val writer = FakeSession(VolumeAccessMode.READ_WRITE)
        manager.add(containerId, "Writer", writer)
        check(manager.volumes.value.single().id.toString().isNotEmpty())
        manager.close()
        check(writer.closeCount == 1)
    }

    @Test
    fun closeBySessionRemovesThePublicVolumeBeforeClosingIt() {
        var callbackSawOpenEntry = false
        lateinit var manager: UnlockedVolumeManager
        manager = UnlockedVolumeManager { callbackSawOpenEntry = manager.volumes.value.isEmpty() }
        val session = FakeSession(VolumeAccessMode.READ_ONLY)
        manager.add(UUID.randomUUID(), "Reader", session)

        check(manager.close(session))
        check(callbackSawOpenEntry)
        check(session.closeCount == 1)
        check(!manager.close(session))
    }

    private class FakeSession(override val accessMode: VolumeAccessMode) : VolumeSession {
        override val id: UUID = UUID.randomUUID()
        override val volumeKind: VolumeKind = VolumeKind.NORMAL
        override val fileSystem: VolumeFileSystem? = VolumeFileSystem.EXFAT
        override val isReadOnly: Boolean get() = accessMode == VolumeAccessMode.READ_ONLY
        override val canModifyContainer: Boolean get() = accessMode == VolumeAccessMode.READ_WRITE
        override val state: StateFlow<VolumeSessionState> = MutableStateFlow(VolumeSessionState.Open)
        var closeCount = 0

        override fun touch() = Unit
        override fun flush() = Unit
        override fun mountFileSystem(): VolumeFileSystem = checkNotNull(fileSystem)
        override fun close() { closeCount++ }
    }
}
