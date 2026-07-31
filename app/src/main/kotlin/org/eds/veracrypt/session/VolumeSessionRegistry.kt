package org.eds.veracrypt.session

import java.io.Closeable
import java.util.UUID
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeError

/**
 * Process-local arbitration for an unlocked container. A container has exactly
 * one live lease: exposing an outer and hidden volume at the same time risks
 * writing the hidden area through the outer filesystem. The opaque
 * [containerKey] is never exposed through DocumentsProvider.
 */
class VolumeSessionRegistry {
    @Synchronized
    fun acquire(containerKey: String, accessMode: VolumeAccessMode): Lease {
        require(accessMode != VolumeAccessMode.AUTOMATIC) { "Session leases require a concrete access mode" }
        if (sessions.containsKey(containerKey)) {
            throw VolumeError.IoInterrupted(IllegalStateException("Container already has an active volume"))
        }

        val leaseId = UUID.randomUUID()
        sessions[containerKey] = leaseId
        return Lease(containerKey, leaseId, accessMode)
    }

    @Synchronized
    private fun release(lease: Lease) {
        if (sessions[lease.containerKey] == lease.id) sessions.remove(lease.containerKey)
    }

    inner class Lease internal constructor(
        internal val containerKey: String,
        internal val id: UUID,
        val accessMode: VolumeAccessMode,
    ) : Closeable {
        private var closed = false

        override fun close() {
            synchronized(this) {
                if (closed) return
                closed = true
            }
            release(this)
        }
    }

    private val sessions = mutableMapOf<String, UUID>()
}
