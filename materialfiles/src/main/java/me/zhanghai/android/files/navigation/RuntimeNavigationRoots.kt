package me.zhanghai.android.files.navigation

import android.net.Uri
import androidx.annotation.DrawableRes
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import java8.nio.file.Path

/** Process-only roots contributed by the embedding app. They are deliberately
 * not persisted as user bookmarks. */
object RuntimeNavigationRoots {
    private val mutableRoots = MutableLiveData<List<RuntimeNavigationRoot>>(emptyList())
    @Volatile private var snapshot: List<RuntimeNavigationRoot> = emptyList()
    val roots: LiveData<List<RuntimeNavigationRoot>> = mutableRoots

    @Synchronized
    fun replace(roots: List<RuntimeNavigationRoot>) {
        snapshot = roots.sortedBy { it.title.lowercase() }
        mutableRoots.postValue(snapshot)
    }

    fun isReadOnly(treeUri: Uri): Boolean? = snapshot
        .firstOrNull { it.treeUri == treeUri }
        ?.isReadOnly

    /** Whether a tree belongs to a currently unlocked, process-only container root. */
    fun contains(treeUri: Uri): Boolean = snapshot.any { it.treeUri == treeUri }

    /** Returns the live unlocked root that contains [path], if any. */
    fun findContaining(path: Path): RuntimeNavigationRoot? {
        val normalizedPath = runCatching { path.normalize() }.getOrNull() ?: return null
        return snapshot.firstOrNull { root ->
            runCatching {
                val normalizedRoot = root.path.normalize()
                normalizedPath == normalizedRoot || normalizedPath.startsWith(normalizedRoot)
            }.getOrDefault(false)
        }
    }

    /**
     * Records an operation on a process-only root. The callback is supplied by
     * the owner of the root (RongVault's volume session) and is deliberately
     * not persisted with navigation settings.
     */
    fun touch(path: Path): Boolean = findContaining(path)?.let { root ->
        root.onAccess?.invoke()
        true
    } ?: false
}

data class RuntimeNavigationRoot(
    val id: String,
    val treeUri: Uri,
    val path: Path,
    val title: String,
    val subtitle: String?,
    @DrawableRes val iconRes: Int,
    val isReadOnly: Boolean,
    /** Optional activity hook for process-only roots, never serialized. */
    val onAccess: (() -> Unit)? = null,
)
