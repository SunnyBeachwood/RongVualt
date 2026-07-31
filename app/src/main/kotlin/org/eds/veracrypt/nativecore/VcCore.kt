package org.eds.veracrypt.nativecore

/**
 * Deliberately narrow JNI boundary. Native session handles are opaque and no
 * keying material crosses this API. The implementation is introduced before
 * volume support so future code cannot grow a Java-side crypto boundary.
 */
internal object VcCore {
    init {
        System.loadLibrary("vc_core")
    }

    external fun nativeOpenWithProgress(fd: Int, writable: Boolean, request: ByteArray, keyfileFds: IntArray, progress: NativeUnlockProgress): Long
    /** Compatibility entry point for tests and non-interactive native callers. */
    fun nativeOpen(fd: Int, writable: Boolean, request: ByteArray, keyfileFds: IntArray): Long =
        nativeOpenWithProgress(fd, writable, request, keyfileFds, NativeUnlockProgress())
    external fun nativeCreateNormal(fd: Int, request: ByteArray, keyfileFds: IntArray, progress: NativeCreateProgress): Long
    external fun nativeCreateHidden(outerSessionHandle: Long, request: ByteArray, keyfileFds: IntArray, progress: NativeCreateProgress): Long
    external fun nativeClose(sessionHandle: Long)
    external fun nativeFlush(sessionHandle: Long)
    external fun nativeMountFileSystem(sessionHandle: Long): Int
    external fun nativeListDirectory(sessionHandle: Long, relativePath: String): Array<NativeFileEntry>
    external fun nativeStat(sessionHandle: Long, relativePath: String): NativeFileEntry
    external fun nativeOpenFile(
        sessionHandle: Long,
        relativePath: String,
        writable: Boolean,
        create: Boolean,
        truncate: Boolean,
    ): Long
    external fun nativeReadFile(fileHandle: Long, offset: Long, target: ByteArray, targetOffset: Int, length: Int): Int
    external fun nativeWriteFile(fileHandle: Long, offset: Long, source: ByteArray, sourceOffset: Int, length: Int): Int
    external fun nativeTruncateFile(fileHandle: Long, length: Long)
    external fun nativePreallocateFile(fileHandle: Long, length: Long)
    external fun nativeFlushFile(fileHandle: Long)
    external fun nativeCloseFile(fileHandle: Long)
    external fun nativeCreateDirectory(sessionHandle: Long, relativePath: String)
    external fun nativeDelete(sessionHandle: Long, relativePath: String)
    external fun nativeRename(sessionHandle: Long, fromRelativePath: String, toRelativePath: String)
    external fun nativeGetVolumeInfo(sessionHandle: Long): LongArray
    external fun nativeAnalyzeHiddenCapacity(sessionHandle: Long): LongArray
    external fun nativeRead(sessionHandle: Long, offset: Long, target: ByteArray, targetOffset: Int, length: Int): Int
    external fun nativeWrite(sessionHandle: Long, offset: Long, source: ByteArray, sourceOffset: Int, length: Int): Int
    /** Benchmark-only direct serial read that bypasses the session LRU. */
    external fun nativeReadUncached(sessionHandle: Long, offset: Long, target: ByteArray, targetOffset: Int, length: Int): Int
    /** Benchmark-only legacy per-sector path; unavailable in production builds. */
    external fun nativeReadLegacy(sessionHandle: Long, offset: Long, target: ByteArray, targetOffset: Int, length: Int): Int
    external fun nativeWriteLegacy(sessionHandle: Long, offset: Long, source: ByteArray, sourceOffset: Int, length: Int): Int
    external fun nativeChangeCredentials(sessionHandle: Long, request: ByteArray, keyfileFds: IntArray)
    external fun nativeBackupHeader(sessionHandle: Long, outputFd: Int, request: ByteArray, keyfileFds: IntArray)
    external fun nativeRestoreHeader(fd: Int, inputFd: Int, request: ByteArray, keyfileFds: IntArray)
    external fun nativeRunSelfTests()
    /** Number of CPUs selected by the native affinity/topology probe. */
    external fun nativeRecommendedWorkerCount(): Int
    /** Non-sensitive CPU/affinity facts for diagnostics and benchmark reports. */
    external fun nativeCpuTopologySummary(): String
    /** Debug/benchmark-only: syscall/byte totals and serial-batching counters. */
    external fun nativeGetPerformanceCounters(sessionHandle: Long): LongArray
    /** Benchmark-only runtime provider/CPUID observation. */
    external fun nativeGetBotanHardwareProviders(): String
}
