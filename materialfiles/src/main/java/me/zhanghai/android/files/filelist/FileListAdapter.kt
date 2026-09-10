/*
 * Copyright (c) 2018 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filelist

import android.animation.ValueAnimator
import android.os.Build
import android.text.TextUtils
import android.view.HapticFeedbackConstants
import android.view.MotionEvent
import android.view.View
import android.view.ViewConfiguration
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.appcompat.widget.PopupMenu
import androidx.core.view.isVisible
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.RecyclerView
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import coil.dispose
import coil.load
import java8.nio.file.Path
import me.zhanghai.android.fastscroll.PopupTextProvider
import me.zhanghai.android.files.R
import me.zhanghai.android.files.coil.AppIconPackageName
import me.zhanghai.android.files.compat.foregroundCompat
import me.zhanghai.android.files.compat.getDrawableCompat
import me.zhanghai.android.files.databinding.FileItemGridBinding
import me.zhanghai.android.files.databinding.FileItemListBinding
import me.zhanghai.android.files.file.FileItem
import me.zhanghai.android.files.file.fileSize
import me.zhanghai.android.files.file.formatShort
import me.zhanghai.android.files.file.iconRes
import me.zhanghai.android.files.file.isApk
import me.zhanghai.android.files.provider.archive.isArchivePath
import me.zhanghai.android.files.provider.common.isEncrypted
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.ui.AnimatedListAdapter
import me.zhanghai.android.files.ui.CheckableForegroundLinearLayout
import me.zhanghai.android.files.ui.CheckableItemBackground
import me.zhanghai.android.files.util.isMaterial3Theme
import me.zhanghai.android.files.util.layoutInflater
import me.zhanghai.android.files.util.dpToDimensionPixelSize
import me.zhanghai.android.files.util.valueCompat
import java.util.Locale

class FileListAdapter(
    private val listener: Listener
) : AnimatedListAdapter<FileItem, FileListAdapter.ViewHolder>(CALLBACK), PopupTextProvider {
    private var isSearching = false

    private lateinit var _viewType: FileViewType
    var viewType: FileViewType
        get() = _viewType
        set(value) {
            _viewType = value
            if (!isSearching) {
                super.replace(list, true)
            }
        }

    private lateinit var _sortOptions: FileSortOptions
    var sortOptions: FileSortOptions
        get() = _sortOptions
        set(value) {
            _sortOptions = value
            if (!isSearching) {
                val sortedList = list.sortedWith(value.createComparator())
                super.replace(sortedList, true)
                rebuildFilePositionMap()
            }
        }

    var pickOptions: PickOptions? = null
        set(value) {
            field = value
            notifyItemRangeChanged(0, itemCount, PAYLOAD_STATE_CHANGED)
        }

    private val selectedFiles = fileItemSetOf()

    private val filePositionMap = mutableMapOf<Path, Int>()

    private lateinit var _nameEllipsize: TextUtils.TruncateAt
    var nameEllipsize: TextUtils.TruncateAt
        get() = _nameEllipsize
        set(value) {
            _nameEllipsize = value
            notifyItemRangeChanged(0, itemCount, PAYLOAD_STATE_CHANGED)
        }

    private var _nameMaxLines: Int = 2
    var nameMaxLines: Int
        get() = _nameMaxLines
        set(value) {
            _nameMaxLines = value.coerceIn(1, 3)
            notifyItemRangeChanged(0, itemCount, PAYLOAD_STATE_CHANGED)
        }

    var fontSize: FileListFontSize = FileListFontSize.fromSp(FileListFontSize.DEFAULT_SP)
        set(value) {
            field = value
            notifyItemRangeChanged(0, itemCount, PAYLOAD_STATE_CHANGED)
        }

    fun replaceSelectedFiles(files: FileItemSet) {
        val changedFiles = fileItemSetOf()
        val iterator = selectedFiles.iterator()
        while (iterator.hasNext()) {
            val file = iterator.next()
            if (file !in files) {
                iterator.remove()
                changedFiles.add(file)
            }
        }
        for (file in files) {
            if (file !in selectedFiles) {
                selectedFiles.add(file)
                changedFiles.add(file)
            }
        }
        for (file in changedFiles) {
            val position = filePositionMap[file.path]
            position?.let { notifyItemChanged(it, PAYLOAD_STATE_CHANGED) }
        }
    }

    private fun selectFile(file: FileItem) {
        if (!isFileSelectable(file)) {
            return
        }
        val selected = file in selectedFiles
        val pickOptions = pickOptions
        if (!selected && pickOptions != null && !pickOptions.allowMultiple) {
            listener.clearSelectedFiles()
        }
        listener.selectFile(file, !selected)
    }

    fun selectAllFiles() {
        val files = fileItemSetOf()
        for (index in 0..<itemCount) {
            val file = getItem(index)
            if (isFileSelectable(file)) {
                files.add(file)
            }
        }
        listener.selectFiles(files, true)
    }

    fun invertSelectedFiles() {
        val files = (0..<itemCount).map { getItem(it) }
        val inverted = FileListSelection.inverted(files, selectedFiles, ::isFileSelectable)
        listener.replaceSelectedFiles(fileItemSetOf(*inverted.toTypedArray()))
    }

    fun hasFile(path: Path): Boolean = path in filePositionMap

    fun selectableFilesInRange(anchor: FileItem, target: FileItem): FileItemSet {
        val files = (0..<itemCount).map { getItem(it) }
        return fileItemSetOf(*FileListSelection.range(files, anchor, target, ::isFileSelectable)
            .toTypedArray())
    }

    fun attachSwipeSelection(recyclerView: RecyclerView) {
        val touchSlop = ViewConfiguration.get(recyclerView.context).scaledTouchSlop
        var downX = 0f
        var downY = 0f
        var downView: View? = null
        var nameView: TextView? = null
        var handled = false
        var moved = false
        var verticalGesture = false

        fun resetNameView(animate: Boolean) {
            val view = nameView ?: return
            view.animate().cancel()
            if (animate && shouldAnimateSelection()) {
                view.animate().translationX(0f).setDuration(160L)
                    .setInterpolator(FastOutSlowInInterpolator()).start()
            } else {
                view.translationX = 0f
            }
            nameView = null
        }

        recyclerView.addOnItemTouchListener(object : RecyclerView.SimpleOnItemTouchListener() {
            override fun onInterceptTouchEvent(recyclerView: RecyclerView, event: MotionEvent): Boolean {
                when (event.actionMasked) {
                    MotionEvent.ACTION_DOWN -> {
                        downX = event.x
                        downY = event.y
                        downView = recyclerView.findChildViewUnder(downX, downY)
                        handled = false
                        moved = false
                        verticalGesture = false
                    }
                    MotionEvent.ACTION_MOVE -> {
                        if (_viewType != FileViewType.LIST) return handled
                        val deltaX = event.x - downX
                        val deltaY = event.y - downY
                        if (kotlin.math.abs(deltaY) > touchSlop &&
                            kotlin.math.abs(deltaY) > kotlin.math.abs(deltaX)
                        ) {
                            verticalGesture = true
                            moved = true
                            resetNameView(animate = false)
                            return false
                        }
                        if (verticalGesture || kotlin.math.abs(deltaX) <= touchSlop) return handled
                        moved = true
                        val child = downView
                        if (child == null) return false
                        if (nameView == null) {
                            nameView = (recyclerView.getChildViewHolder(child) as? ViewHolder)?.nameText
                        }
                        nameView?.translationX = deltaX.coerceIn(
                            -recyclerView.context.dpToDimensionPixelSize(24).toFloat(),
                            recyclerView.context.dpToDimensionPixelSize(24).toFloat(),
                        )
                        val threshold = maxOf(touchSlop * 2, child.width / 4)
                        if (kotlin.math.abs(deltaX) >= threshold) {
                            val position = recyclerView.getChildAdapterPosition(child)
                            if (position != RecyclerView.NO_POSITION) {
                                listener.onFileSwipeSelect(getItem(position))
                                child.performHapticFeedback(HapticFeedbackConstants.KEYBOARD_TAP)
                                handled = true
                                resetNameView(animate = true)
                                return true
                            }
                        }
                    }
                    MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                        if (event.actionMasked == MotionEvent.ACTION_UP &&
                            downView == null && !moved
                        ) {
                            listener.onPaneBackgroundClick()
                        }
                        resetNameView(animate = true)
                        downView = null
                        handled = false
                        moved = false
                        verticalGesture = false
                    }
                }
                return handled
            }

            override fun onTouchEvent(recyclerView: RecyclerView, event: MotionEvent) {
                if (event.actionMasked == MotionEvent.ACTION_UP || event.actionMasked == MotionEvent.ACTION_CANCEL) {
                    downView = null
                    handled = false
                }
            }
        })
    }

    private fun isFileSelectable(file: FileItem): Boolean {
        val pickOptions = pickOptions ?: return true
        return when (pickOptions.mode) {
            PickOptions.Mode.OPEN_FILE, PickOptions.Mode.CREATE_FILE ->
                !file.attributes.isDirectory &&
                    pickOptions.mimeTypes.any { it.match(file.mimeType) }
            PickOptions.Mode.OPEN_DIRECTORY -> file.attributes.isDirectory
        }
    }

    override fun clear() {
        super.clear()

        rebuildFilePositionMap()
    }

    @Deprecated("", ReplaceWith("replaceListAndSearching(list, searching)"))
    override fun replace(list: List<FileItem>, clear: Boolean) {
        throw UnsupportedOperationException()
    }

    fun replaceListAndIsSearching(list: List<FileItem>, isSearching: Boolean) {
        val clear = this.isSearching != isSearching
        this.isSearching = isSearching
        val sortedList = if (!isSearching) list.sortedWith(sortOptions.createComparator()) else list
        super.replace(sortedList, clear)
        rebuildFilePositionMap()
    }

    private fun rebuildFilePositionMap() {
        filePositionMap.clear()
        for (index in 0..<itemCount) {
            val file = getItem(index)
            filePositionMap[file.path] = index
        }
    }

    override fun getItemViewType(position: Int): Int = viewType.ordinal

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val viewType = FileViewType.entries[viewType]
        val inflater = parent.context.layoutInflater
        val holder = when (viewType) {
            FileViewType.LIST -> ViewHolder(FileItemListBinding.inflate(inflater, parent, false))
            FileViewType.GRID -> ViewHolder(FileItemGridBinding.inflate(inflater, parent, false))
        }
        return holder.apply {
            itemLayout.apply {
                val context = context
                val isMaterial3Theme = context.isMaterial3Theme
                if (viewType == FileViewType.GRID && isMaterial3Theme) {
                    foregroundCompat =
                        context.getDrawableCompat(R.drawable.file_item_grid_foreground_material3)
                }
                background = if (viewType == FileViewType.GRID && isMaterial3Theme) {
                    CheckableItemBackground.create(4f, 12f, context)
                } else {
                    CheckableItemBackground.create(0f, 0f, context)
                }
            }
            thumbnailOutlineView?.apply {
                val context = context
                if (context.isMaterial3Theme) {
                    background = context.getDrawableCompat(
                        R.drawable.file_item_grid_thumbnail_outline_material3
                    )
                }
            }
            popupMenu = PopupMenu(itemLayout.context, itemLayout)
                .apply { inflate(R.menu.file_item) }
        }
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        throw UnsupportedOperationException()
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int, payloads: List<Any>) {
        val file = getItem(position)
        val isDirectory = file.attributes.isDirectory
        val isEnabled = isFileSelectable(file) || isDirectory
        holder.itemLayout.isEnabled = isEnabled
        val density = holder.itemLayout.resources.displayMetrics.density
        holder.itemLayout.layoutParams = holder.itemLayout.layoutParams.apply {
            height = (fontSize.rowHeightDp * density).toInt()
        }
        holder.iconLayout.layoutParams = holder.iconLayout.layoutParams.apply {
            width = (fontSize.iconSlotDp * density).toInt()
            height = (fontSize.iconSlotDp * density).toInt()
        }
        holder.nameText.textSize = fontSize.nameSp
        holder.descriptionText?.textSize = fontSize.metadataSp
        val menu = holder.popupMenu.menu
        val path = file.path
        val hasPickOptions = pickOptions != null
        val isReadOnly = path.fileSystem.isReadOnly
        menu.findItem(R.id.action_cut).isVisible = !hasPickOptions && !isReadOnly
        menu.findItem(R.id.action_copy).isVisible = !hasPickOptions
        val checked = file in selectedFiles
        val selectionChanged = payloads.any { it === PAYLOAD_STATE_CHANGED } &&
            holder.itemLayout.isChecked != checked
        holder.itemLayout.animate().cancel()
        holder.itemLayout.alpha = 1f
        holder.itemLayout.scaleX = 1f
        holder.itemLayout.scaleY = 1f
        holder.itemLayout.isChecked = checked
        if (selectionChanged && shouldAnimateSelection()) {
            holder.itemLayout.apply {
                alpha = 0.82f
                scaleX = 0.985f
                scaleY = 0.985f
                animate()
                    .alpha(1f)
                    .scaleX(1f)
                    .scaleY(1f)
                    .setDuration(160L)
                    .setInterpolator(FastOutSlowInInterpolator())
                    .start()
            }
        }
        holder.nameText.apply {
            val maxLines = nameMaxLines
            if (maxLines == 1) {
                isSingleLine = true
                ellipsize = nameEllipsize
                isSelected = nameEllipsize == TextUtils.TruncateAt.MARQUEE
            } else {
                isSingleLine = false
                this.maxLines = maxLines
                ellipsize = if (nameEllipsize == TextUtils.TruncateAt.MARQUEE) {
                    TextUtils.TruncateAt.END
                } else nameEllipsize
                isSelected = false
            }
        }
        holder.itemLayout.apply {
            setOnClickListener {
                if (selectedFiles.isEmpty()) {
                    listener.openFile(file)
                } else {
                    selectFile(file)
                }
            }
            setOnLongClickListener {
                if (!listener.onFileLongClick(file)) holder.popupMenu.show()
                true
            }
        }
        holder.iconLayout.setOnClickListener { selectFile(file) }
        if (payloads.isNotEmpty()) {
            return
        }
        bindViewHolderAnimation(holder)
        val iconRes = file.mimeType.iconRes
        holder.iconImage.apply {
            isVisible = true
            setImageResource(iconRes)
        }
        holder.directoryThumbnailImage?.isVisible = isDirectory
        holder.thumbnailOutlineView?.isVisible = !isDirectory
        val supportsThumbnail = file.supportsThumbnail
        val shouldLoadThumbnailIcon = supportsThumbnail && holder.thumbnailIconImage != null &&
            file.mimeType.isApk
        val attributes = file.attributes
        holder.thumbnailIconImage?.apply {
            dispose()
            isVisible = !isDirectory
            setImageResource(iconRes)
            if (shouldLoadThumbnailIcon) {
                load(path to attributes)
            }
        }
        holder.thumbnailImage.apply {
            dispose()
            setImageDrawable(null)
            val shouldLoadThumbnail = supportsThumbnail && !shouldLoadThumbnailIcon
            isVisible = shouldLoadThumbnail
            if (shouldLoadThumbnail) {
                load(path to attributes) {
                    // The ImageView begins as GONE while a list row is being
                    // bound. Give Coil a concrete preview size so it doesn't
                    // wait forever for a zero-sized target on some devices.
                    size(resources.getDimensionPixelSize(R.dimen.large_icon_size))
                    listener { _, _ ->
                        val iconImage = holder.thumbnailIconImage ?: holder.iconImage
                        iconImage.isVisible = false
                    }
                }
            }
        }
        holder.appIconBadgeImage.apply {
            dispose()
            setImageDrawable(null)
            val appDirectoryPackageName = file.appDirectoryPackageName
            val hasAppIconBadge = appDirectoryPackageName != null
            isVisible = hasAppIconBadge
            if (hasAppIconBadge) {
                load(AppIconPackageName(appDirectoryPackageName!!))
            }
        }
        holder.badgeImage.apply {
            val badgeIconRes = if (file.attributesNoFollowLinks.isSymbolicLink) {
                if (file.isSymbolicLinkBroken) {
                    R.drawable.error_badge_icon_18dp
                } else {
                    R.drawable.symbolic_link_badge_icon_18dp
                }
            } else if (file.attributesNoFollowLinks.isEncrypted()) {
                R.drawable.encrypted_badge_icon_18dp
            } else {
                null
            }
            val hasBadge = badgeIconRes != null
            isVisible = hasBadge
            if (hasBadge) {
                setImageResource(badgeIconRes!!)
            } else {
                setImageDrawable(null)
            }
        }
        holder.nameText.text = file.name
        holder.descriptionText?.text = runCatching {
            val context = holder.descriptionText!!.context
            val lastModificationTime = attributes.lastModifiedTime().toInstant()
                .formatShort(context)
            if (isDirectory) lastModificationTime else {
                val size = attributes.fileSize.formatHumanReadable(context)
                val descriptionSeparator = context.getString(R.string.file_item_description_separator)
                "$lastModificationTime$descriptionSeparator$size"
            }
        }.getOrNull()
        val isArchivePath = path.isArchivePath
        menu.findItem(R.id.action_copy)
            .setTitle(if (isArchivePath) R.string.file_item_action_extract else R.string.copy)
        menu.findItem(R.id.action_delete).isVisible = !isReadOnly
        menu.findItem(R.id.action_rename).isVisible = !isReadOnly
        menu.findItem(R.id.action_extract).isVisible = file.isArchiveFile
        menu.findItem(R.id.action_archive).isVisible = !isArchivePath
        menu.findItem(R.id.action_add_bookmark).isVisible = isDirectory
        holder.popupMenu.setOnMenuItemClickListener {
            when (it.itemId) {
                R.id.action_open_with -> {
                    listener.openFileWith(file)
                    true
                }
                R.id.action_cut -> {
                    listener.cutFile(file)
                    true
                }
                R.id.action_copy -> {
                    listener.copyFile(file)
                    true
                }
                R.id.action_delete -> {
                    listener.confirmDeleteFile(file)
                    true
                }
                R.id.action_rename -> {
                    listener.showRenameFileDialog(file)
                    true
                }
                R.id.action_extract -> {
                    listener.extractFile(file)
                    true
                }
                R.id.action_archive -> {
                    listener.showCreateArchiveDialog(file)
                    true
                }
                R.id.action_share -> {
                    listener.shareFile(file)
                    true
                }
                R.id.action_copy_path -> {
                    listener.copyPath(file)
                    true
                }
                R.id.action_add_bookmark -> {
                    listener.addBookmark(file)
                    true
                }
                R.id.action_create_shortcut -> {
                    listener.createShortcut(file)
                    true
                }
                R.id.action_properties -> {
                    listener.showPropertiesDialog(file)
                    true
                }
                else -> false
            }
        }
    }

    override fun getPopupText(view: View, position: Int): CharSequence {
        val file = getItem(position)
        return when (sortOptions.by) {
            FileSortOptions.By.NAME -> file.name.take(1).uppercase(Locale.getDefault())
            FileSortOptions.By.TYPE -> file.extension.uppercase(Locale.getDefault())
            FileSortOptions.By.SIZE -> file.attributes.fileSize.formatHumanReadable(view.context)
            FileSortOptions.By.LAST_MODIFIED ->
                file.attributes.lastModifiedTime().toInstant().formatShort(view.context)
        }
    }

    override val isAnimationEnabled: Boolean
        get() = Settings.FILE_LIST_ANIMATION.valueCompat

    override fun onViewRecycled(holder: ViewHolder) {
        holder.itemLayout.animate().cancel()
        holder.itemLayout.alpha = 1f
        holder.itemLayout.scaleX = 1f
        holder.itemLayout.scaleY = 1f
        super.onViewRecycled(holder)
    }

    private fun shouldAnimateSelection(): Boolean = Settings.FILE_LIST_ANIMATION.valueCompat &&
        (Build.VERSION.SDK_INT < Build.VERSION_CODES.O || ValueAnimator.areAnimatorsEnabled())

    companion object {
        private val PAYLOAD_STATE_CHANGED = Any()

        private val CALLBACK = object : DiffUtil.ItemCallback<FileItem>() {
            override fun areItemsTheSame(oldItem: FileItem, newItem: FileItem): Boolean =
                oldItem.path == newItem.path

            override fun areContentsTheSame(oldItem: FileItem, newItem: FileItem): Boolean =
                oldItem == newItem
        }
    }

    class ViewHolder private constructor(
        root: View,
        val itemLayout: CheckableForegroundLinearLayout,
        val iconLayout: View,
        val iconImage: ImageView,
        val directoryThumbnailImage: ImageView?,
        val thumbnailOutlineView: View?,
        val thumbnailIconImage: ImageView?,
        val thumbnailImage: ImageView,
        val appIconBadgeImage: ImageView,
        val badgeImage: ImageView,
        val nameText: TextView,
        val descriptionText: TextView?
    ) : RecyclerView.ViewHolder(root) {
        constructor(binding: FileItemListBinding) : this(
            binding.root,
            binding.itemLayout,
            binding.iconLayout,
            binding.iconImage,
            null,
            null,
            null,
            binding.thumbnailImage,
            binding.appIconBadgeImage,
            binding.badgeImage,
            binding.nameText,
            binding.descriptionText
        )

        constructor(binding: FileItemGridBinding) : this(
            binding.root,
            binding.itemLayout,
            binding.iconLayout,
            binding.iconImage,
            binding.directoryThumbnailImage,
            binding.thumbnailOutlineView,
            binding.thumbnailIconImage,
            binding.thumbnailImage,
            binding.appIconBadgeImage,
            binding.badgeImage,
            binding.nameText,
            null
        )

        lateinit var popupMenu: PopupMenu
    }

    interface Listener {
        fun clearSelectedFiles()
        fun selectFile(file: FileItem, selected: Boolean)
        fun selectFiles(files: FileItemSet, selected: Boolean)
        fun replaceSelectedFiles(files: FileItemSet) = Unit
        fun openFile(file: FileItem)
        fun openFileWith(file: FileItem)
        fun cutFile(file: FileItem)
        fun copyFile(file: FileItem)
        fun confirmDeleteFile(file: FileItem)
        fun showRenameFileDialog(file: FileItem)
        fun extractFile(file: FileItem)
        fun showCreateArchiveDialog(file: FileItem)
        fun shareFile(file: FileItem)
        fun copyPath(file: FileItem)
        fun addBookmark(file: FileItem)
        fun createShortcut(file: FileItem)
        fun showPropertiesDialog(file: FileItem)

        /** Return true when a host wants to replace the legacy popup menu. */
        fun onFileMenuRequested(file: FileItem): Boolean = false

        /** Return true when a host handled long-press interaction itself. */
        fun onFileLongClick(file: FileItem): Boolean = false

        fun onPaneBackgroundClick() = Unit

        fun onFileSwipeSelect(file: FileItem) = Unit
    }
}
