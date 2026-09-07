package me.zhanghai.android.files.filelist

/** Order-preserving selection calculations kept independent from RecyclerView gesture handling. */
internal object FileListSelection {
    fun <T> range(
        items: List<T>,
        anchor: T,
        target: T,
        isSelectable: (T) -> Boolean,
    ): List<T> {
        val anchorIndex = items.indexOf(anchor)
        val targetIndex = items.indexOf(target)
        if (anchorIndex < 0 || targetIndex < 0) return emptyList()
        return items.subList(
            minOf(anchorIndex, targetIndex), maxOf(anchorIndex, targetIndex) + 1
        ).filter(isSelectable)
    }

    fun <T> inverted(
        items: List<T>,
        selected: Set<T>,
        isSelectable: (T) -> Boolean,
    ): List<T> = items.filter { isSelectable(it) && it !in selected }
}
