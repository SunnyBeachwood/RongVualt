/*
 * Copyright (c) 2026 RongVault contributors.
 * All Rights Reserved.
 */

package me.zhanghai.android.files.ftpserver

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import java8.nio.file.Path
import me.zhanghai.android.files.navigation.RuntimeNavigationRoots
import me.zhanghai.android.files.provider.document.documentTreeUri
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.util.valueCompat

/**
 * A configured FTP home. Unlocked-volume paths contain process-local document
 * IDs and therefore must never be written to SharedPreferences.
 */
sealed interface FtpShareRoot {
    val path: Path
    val runtimeRootId: String?
    val isReadOnly: Boolean

    data class Persistent(override val path: Path) : FtpShareRoot {
        override val runtimeRootId: String? = null
        override val isReadOnly: Boolean = false
    }

    data class UnlockedVolume(
        override val path: Path,
        override val runtimeRootId: String,
        override val isReadOnly: Boolean,
    ) : FtpShareRoot
}

data class FtpShareRootSnapshot(
    val path: Path,
    val runtimeRootId: String?,
    val isReadOnly: Boolean,
)

/** Process bridge between the embedded picker and the FTP service. */
object FtpShareRootStore {
    private val lock = Any()
    private var unlockedSelection: FtpShareRoot.UnlockedVolume? = null
    private var pendingStart: FtpShareRootSnapshot? = null
    private var cancelledPendingStart = false
    private val mutableSelection = MutableLiveData<FtpShareRootSnapshot>()
    val selection: LiveData<FtpShareRootSnapshot> = mutableSelection

    fun current(): FtpShareRootSnapshot = synchronized(lock) {
        val unlocked = unlockedSelection
        if (unlocked != null) {
            val liveRoot = RuntimeNavigationRoots.findContaining(unlocked.path)
            if (liveRoot?.id == unlocked.runtimeRootId) {
                return@synchronized unlocked.copy(
                    isReadOnly = unlocked.isReadOnly || liveRoot.isReadOnly,
                ).snapshot()
            }
            unlockedSelection = null
        }
        FtpShareRoot.Persistent(Settings.FTP_SERVER_HOME_DIRECTORY.valueCompat).snapshot()
    }.also { mutableSelection.postValue(it) }

    /** Selects a live unlocked path without persisting its opaque document ID. */
    fun select(path: Path) {
        val normalizedPath = runCatching { path.normalize() }.getOrElse { path }
        val liveRoot = RuntimeNavigationRoots.findContaining(normalizedPath)
        val isUnlockedDocumentPath = runCatching {
            normalizedPath.documentTreeUri.authority?.endsWith(".unlocked") == true
        }.getOrDefault(false)
        val snapshot = synchronized(lock) {
            if (liveRoot != null) {
                unlockedSelection = FtpShareRoot.UnlockedVolume(
                    normalizedPath, liveRoot.id, liveRoot.isReadOnly
                )
            } else if (isUnlockedDocumentPath) {
                // A picker result can race volume teardown. Never write the
                // opaque unlocked document URI into the persisted setting.
                unlockedSelection = null
            } else {
                unlockedSelection = null
                Settings.FTP_SERVER_HOME_DIRECTORY.putValue(normalizedPath)
            }
            currentLocked()
        }
        mutableSelection.postValue(snapshot)
    }

    /** Captures the root used by a service start, so later picker changes do not affect it. */
    fun requestStart(): FtpShareRootSnapshot = synchronized(lock) {
        cancelledPendingStart = false
        currentLocked().also { pendingStart = it }
    }

    fun takePendingStart(): FtpShareRootSnapshot? = synchronized(lock) {
        pendingStart.also {
            if (it != null) cancelledPendingStart = false
            pendingStart = null
        }
    }

    fun cancelPendingStart() = synchronized(lock) {
        if (pendingStart != null) {
            pendingStart = null
            cancelledPendingStart = true
        }
    }

    /** Prevents an already queued Android service start from resurrecting an invalid root. */
    fun consumeCancelledPendingStart(): Boolean = synchronized(lock) {
        cancelledPendingStart.also { cancelledPendingStart = false }
    }

    /** Runtime roots must still be present when the service consumes a snapshot. */
    fun isLive(snapshot: FtpShareRootSnapshot): Boolean =
        snapshot.runtimeRootId == null || RuntimeNavigationRoots.findContaining(snapshot.path)
            ?.id == snapshot.runtimeRootId

    /** Invalidates a runtime selection before the native volume is closed. */
    fun invalidateRuntimeRoot(rootId: String): Boolean {
        val changed = synchronized(lock) {
            if (pendingStart?.runtimeRootId == rootId) {
                pendingStart = null
                cancelledPendingStart = true
            }
            if (unlockedSelection?.runtimeRootId != rootId) false
            else {
                unlockedSelection = null
                true
            }
        }
        if (changed) mutableSelection.postValue(current())
        return changed
    }

    private fun currentLocked(): FtpShareRootSnapshot {
        val unlocked = unlockedSelection
        if (unlocked != null) {
            val liveRoot = RuntimeNavigationRoots.findContaining(unlocked.path)
            if (liveRoot?.id == unlocked.runtimeRootId) {
                return unlocked.copy(
                    isReadOnly = unlocked.isReadOnly || liveRoot.isReadOnly,
                ).snapshot()
            }
            unlockedSelection = null
        }
        return FtpShareRoot.Persistent(Settings.FTP_SERVER_HOME_DIRECTORY.valueCompat).snapshot()
    }

    private fun FtpShareRoot.snapshot() =
        FtpShareRootSnapshot(path, runtimeRootId, isReadOnly)
}
