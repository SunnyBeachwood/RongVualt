package org.eds.veracrypt.session

import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeError
import org.junit.Test

class VolumeSessionRegistryTest {
    @Test
    fun everyLeaseIsExclusiveForOneContainer() {
        val registry = VolumeSessionRegistry()
        val writer = registry.acquire("container-1", VolumeAccessMode.READ_WRITE)
        try {
            registry.acquire("container-1", VolumeAccessMode.READ_ONLY)
            throw AssertionError("A reader must not coexist with a writer")
        } catch (_: VolumeError.IoInterrupted) {
            // Expected.
        }
        writer.close()

        val firstReader = registry.acquire("container-1", VolumeAccessMode.READ_ONLY)
        try {
            registry.acquire("container-1", VolumeAccessMode.READ_ONLY)
            throw AssertionError("An outer and hidden reader must not coexist")
        } catch (_: VolumeError.IoInterrupted) {
            // Expected.
        }
        firstReader.close()
        registry.acquire("container-1", VolumeAccessMode.READ_WRITE).close()
    }

    @Test
    fun closingALeaseIsIdempotent() {
        val lease = VolumeSessionRegistry().acquire("container-1", VolumeAccessMode.READ_ONLY)
        lease.close()
        lease.close()
    }
}
