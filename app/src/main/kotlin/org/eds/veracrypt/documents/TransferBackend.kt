package org.eds.veracrypt.documents

/** Internal transfer strategy selected per file after capability checks. */
internal sealed interface TransferBackend {
    data object DirectNative : TransferBackend
    data object SeekablePfd : TransferBackend
    data object Streaming : TransferBackend
}
