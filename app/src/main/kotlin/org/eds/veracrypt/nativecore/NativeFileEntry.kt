package org.eds.veracrypt.nativecore

/** Metadata returned from a mounted native filesystem without host paths. */
data class NativeFileEntry(
    val name: String,
    val isDirectory: Boolean,
    val sizeBytes: Long,
    val modifiedDate: Int,
    val modifiedTime: Int,
)
