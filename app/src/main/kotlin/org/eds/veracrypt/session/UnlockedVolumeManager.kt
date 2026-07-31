package org.eds.veracrypt.session

import java.io.Closeable
import java.util.UUID
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import org.eds.veracrypt.domain.VolumeSession

/**
 * Application-scoped owner for unlocked volumes. Provider and filesystem code
 * address a random [UnlockedVolume.id], never a SAF URI or an internal path.
 * ViewModels observe this manager but do not own its sessions.
 */
class UnlockedVolumeManager(
    private val onVolumeClosed: (UnlockedVolume) -> Unit = {},
) : Closeable {
    private val lock = Any()
    private val entries = mutableMapOf<UUID, UnlockedVolume>()
    private val mutableVolumes = MutableStateFlow<List<UnlockedVolume>>(emptyList())
    val volumes: StateFlow<List<UnlockedVolume>> = mutableVolumes.asStateFlow()

    fun add(containerId: UUID, displayName: String, session: VolumeSession): UnlockedVolume {
        require(displayName.isNotBlank()) { "Unlocked volume display name is required" }
        synchronized(lock) {
            check(entries.values.none { it.containerId == containerId }) {
                "A container can expose only one unlocked volume at a time"
            }
            val volume = UnlockedVolume(UUID.randomUUID(), containerId, displayName, session)
            entries[volume.id] = volume
            publishLocked()
            return volume
        }
    }

    fun find(id: UUID): UnlockedVolume? = synchronized(lock) { entries[id] }

    fun findForContainer(containerId: UUID): UnlockedVolume? = synchronized(lock) {
        entries.values.firstOrNull { it.containerId == containerId }
    }

    internal fun find(session: VolumeSession): UnlockedVolume? = synchronized(lock) {
        entries.values.firstOrNull { it.session === session }
    }

    /**
     * Removes a root and closes its provider proxy files without closing the
     * native session. The caller must either [restore] it or [replace] it.
     */
    internal fun detach(session: VolumeSession): UnlockedVolume? {
        val removed = synchronized(lock) {
            val match = entries.values.firstOrNull { it.session === session } ?: return null
            entries.remove(match.id)
            publishLocked()
            match
        }
        onVolumeClosed(removed)
        return removed
    }

    /** Restores a temporarily detached root after a failed sensitive operation. */
    internal fun restore(volume: UnlockedVolume) = synchronized(lock) {
        check(entries[volume.id] == null) { "A root already occupies the detached volume ID" }
        entries[volume.id] = volume
        publishLocked()
    }

    /** Atomically puts a dependent hidden session behind the previous root ID. */
    internal fun replace(detached: UnlockedVolume, hiddenSession: VolumeSession): UnlockedVolume = synchronized(lock) {
        check(entries[detached.id] == null) { "A root already occupies the detached volume ID" }
        val replacement = UnlockedVolume(detached.id, detached.containerId, detached.displayName, hiddenSession)
        entries[replacement.id] = replacement
        publishLocked()
        replacement
    }

    fun touch(id: UUID) = synchronized(lock) { entries[id] }?.session?.touch()

    fun close(id: UUID) {
        val removed = synchronized(lock) {
            val volume = entries.remove(id)
            publishLocked()
            volume
        }
        removed?.let { volume ->
            onVolumeClosed(volume)
            volume.session.close()
        }
    }

    /** Closes one application-owned session without exposing its volume ID. */
    fun close(session: VolumeSession): Boolean {
        val removed = synchronized(lock) {
            val match = entries.values.firstOrNull { it.session === session } ?: return false
            entries.remove(match.id)
            publishLocked()
            match
        }
        onVolumeClosed(removed)
        removed.session.close()
        return true
    }

    fun closeContainer(containerId: UUID) {
        val removed = synchronized(lock) {
            val matches = entries.values.filter { it.containerId == containerId }
            matches.forEach { entries.remove(it.id) }
            publishLocked()
            matches
        }
        removed.forEach { volume ->
            onVolumeClosed(volume)
            volume.session.close()
        }
    }

    override fun close() {
        val removed = synchronized(lock) {
            val all = entries.values.toList()
            entries.clear()
            publishLocked()
            all
        }
        removed.forEach { volume ->
            onVolumeClosed(volume)
            volume.session.close()
        }
    }

    private fun publishLocked() {
        mutableVolumes.value = entries.values.sortedBy { it.displayName.lowercase() }
    }
}

data class UnlockedVolume internal constructor(
    /** Random session-local public identifier; not derived from a URI or path. */
    val id: UUID,
    internal val containerId: UUID,
    val displayName: String,
    val session: VolumeSession,
)
