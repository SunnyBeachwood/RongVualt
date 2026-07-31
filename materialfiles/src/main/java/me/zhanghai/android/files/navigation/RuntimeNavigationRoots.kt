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
}

data class RuntimeNavigationRoot(
    val id: String,
    val treeUri: Uri,
    val path: Path,
    val title: String,
    val subtitle: String?,
    @DrawableRes val iconRes: Int,
    val isReadOnly: Boolean,
)
