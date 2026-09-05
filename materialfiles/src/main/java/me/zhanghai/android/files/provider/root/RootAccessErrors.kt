/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package me.zhanghai.android.files.provider.root

import me.zhanghai.android.files.provider.remote.RemoteFileSystemException

/** Stable, user-facing failures for the privileged provider boundary. */
internal object RootAccessErrors {
    fun unavailable(): RemoteFileSystemException =
        RemoteFileSystemException("Root access is unavailable")

    fun denied(cause: Throwable? = null): RemoteFileSystemException =
        RemoteFileSystemException("Root access was denied", cause)

    fun timedOut(cause: Throwable? = null): RemoteFileSystemException =
        RemoteFileSystemException("Root service timed out", cause)

    fun disconnected(cause: Throwable? = null): RemoteFileSystemException =
        RemoteFileSystemException("Root service was disconnected", cause)
}
