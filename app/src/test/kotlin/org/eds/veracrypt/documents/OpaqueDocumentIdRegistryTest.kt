package org.eds.veracrypt.documents

import java.util.UUID
import org.junit.Test

class OpaqueDocumentIdRegistryTest {
    @Test
    fun idsAreStablePerNodeAndRevokedPerVolume() {
        val registry = OpaqueDocumentIdRegistry<Any>()
        val volume = UUID.randomUUID()
        val node = Any()
        val token = registry.register(volume, node)
        check(token == registry.register(volume, node))
        check(registry.resolve(token) === node)

        registry.revokeVolume(volume)
        try {
            registry.resolve(token)
            throw AssertionError("A revoked document token must not resolve")
        } catch (_: IllegalArgumentException) {
            // Expected.
        }
    }

    @Test
    fun revokingOneNodeKeepsOtherNodesInTheSameVolumeValid() {
        val registry = OpaqueDocumentIdRegistry<Any>()
        val volume = UUID.randomUUID()
        val renamedNode = Any()
        val unaffectedNode = Any()
        val renamedToken = registry.register(volume, renamedNode)
        val unaffectedToken = registry.register(volume, unaffectedNode)

        registry.revoke(renamedNode)

        try {
            registry.resolve(renamedToken)
            throw AssertionError("A renamed document token must not resolve")
        } catch (_: IllegalArgumentException) {
            // Expected.
        }
        check(registry.resolve(unaffectedToken) === unaffectedNode)
    }
}
