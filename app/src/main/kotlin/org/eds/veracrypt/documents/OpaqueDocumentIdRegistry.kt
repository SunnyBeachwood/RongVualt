package org.eds.veracrypt.documents

import java.util.UUID
import java.util.IdentityHashMap

/**
 * Process-local backing store for DocumentsProvider identifiers. Android
 * clients receive only an unpredictable token; source URIs, volume labels and
 * plaintext paths remain inside the unlocked-session process.
 */
class OpaqueDocumentIdRegistry<T : Any> {
    @Synchronized
    fun register(volumeId: UUID, node: T): String {
        val existing = tokensByNode[node]
        if (existing != null) {
            check(entries[existing]?.volumeId == volumeId) { "Document node belongs to another volume" }
            return existing
        }
        val token = UUID.randomUUID().toString()
        entries[token] = Entry(volumeId, node)
        tokensByNode[node] = token
        tokensByVolume.getOrPut(volumeId) { mutableSetOf() }.add(token)
        return token
    }

    @Synchronized
    fun resolve(token: String): T = entries[token]?.node
        ?: throw IllegalArgumentException("Unknown or expired document ID")

    @Synchronized
    fun volumeId(token: String): UUID = entries[token]?.volumeId
        ?: throw IllegalArgumentException("Unknown or expired document ID")

    /** Invalidates one node token after its filesystem identity changes. */
    @Synchronized
    fun revoke(node: T) {
        val token = tokensByNode.remove(node) ?: return
        val entry = entries.remove(token) ?: return
        tokensByVolume[entry.volumeId]?.let { tokens ->
            tokens.remove(token)
            if (tokens.isEmpty()) tokensByVolume.remove(entry.volumeId)
        }
    }

    /** Revokes every DocumentProvider handle when an unlocked volume closes. */
    @Synchronized
    fun revokeVolume(volumeId: UUID) {
        tokensByVolume.remove(volumeId)?.forEach { token ->
            entries.remove(token)?.let { tokensByNode.remove(it.node) }
        }
    }

    @Synchronized
    fun clear() {
        entries.clear()
        tokensByNode.clear()
        tokensByVolume.clear()
    }

    private data class Entry<T : Any>(val volumeId: UUID, val node: T)

    private val entries = mutableMapOf<String, Entry<T>>()
    private val tokensByNode = IdentityHashMap<T, String>()
    private val tokensByVolume = mutableMapOf<UUID, MutableSet<String>>()
}
