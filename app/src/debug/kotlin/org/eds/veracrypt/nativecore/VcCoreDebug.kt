package org.eds.veracrypt.nativecore

/** Debug-only bridge for native invariant instrumentation tests. */
object VcCoreDebug {
    fun runSelfTests() = VcCore.nativeRunSelfTests()
}
