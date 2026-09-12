package org.eds.veracrypt.ui

import android.content.res.ColorStateList
import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.PopupMenu
import androidx.core.view.isVisible
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.sovworks.eds.android.R
import com.sovworks.eds.android.databinding.ItemContainerCardBinding

/** Renders only the safe catalog projection; credentials and source URIs never reach a row. */
internal class ContainerCardAdapter(
    private val onUnlock: (ContainerCardUiModel) -> Unit,
    private val onBrowse: (ContainerCardUiModel) -> Unit,
    private val onDetails: (ContainerCardUiModel) -> Unit,
    private val onLock: (ContainerCardUiModel) -> Unit,
    private val onCreateHidden: (ContainerCardUiModel) -> Unit,
    private val onChangeCredentials: (ContainerCardUiModel) -> Unit,
    private val onRemove: (ContainerCardUiModel) -> Unit,
) : ListAdapter<ContainerCardUiModel, ContainerCardAdapter.Holder>(CONTAINER_CARD_DIFF_CALLBACK) {

    init {
        setHasStableIds(true)
    }

    override fun getItemId(position: Int): Long = getItem(position).entry.id.let {
        it.mostSignificantBits xor it.leastSignificantBits
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): Holder = Holder(
        ItemContainerCardBinding.inflate(LayoutInflater.from(parent.context), parent, false),
    )

    override fun onBindViewHolder(holder: Holder, position: Int) = holder.bind(getItem(position))

    inner class Holder(private val binding: ItemContainerCardBinding) : RecyclerView.ViewHolder(binding.root) {
        fun bind(item: ContainerCardUiModel) = with(binding) {
            containerName.text = item.entry.displayName
            containerState.text = stateLabel(item)
            containerState.setTextColor(binding.root.context.getColor(stateColor(item)))
            containerSummary.text = summaryLabel(item)
            containerStateIcon.setImageResource(stateIcon(item))
            containerStateIcon.imageTintList = ColorStateList.valueOf(binding.root.context.getColor(stateColor(item)))
            containerStateIcon.backgroundTintList = ColorStateList.valueOf(binding.root.context.getColor(stateContainerColor(item)))

            root.contentDescription = actionDescription(item)
            root.setOnClickListener {
                if (item.state == ContainerCardState.LOCKED) onUnlock(item) else onBrowse(item)
            }

            lockAction.isVisible = item.canLock
            lockAction.setOnClickListener { onLock(item) }
            moreActions.setOnClickListener { showMore(item) }
        }

        private fun showMore(item: ContainerCardUiModel) {
            PopupMenu(binding.root.context, binding.moreActions).apply {
                if (item.canShowDetails) menu.add(0, MENU_DETAILS, 0, R.string.rv_container_details)
                    .setIcon(R.drawable.ic_menu_info)
                if (item.canCreateHiddenVolume) menu.add(0, MENU_CREATE_HIDDEN, 1, R.string.vc_create_hidden_volume)
                    .setIcon(R.drawable.ic_create_container)
                if (item.canChangeCredentials) menu.add(0, MENU_CHANGE_CREDENTIALS, 2, R.string.vc_change_credentials)
                    .setIcon(R.drawable.ic_menu_clean)
                menu.add(0, MENU_REMOVE, 3, R.string.vc_remove_container)
                    .setIcon(R.drawable.ic_menu_delete)
                setForceShowIcon(true)
                setOnMenuItemClickListener { menu ->
                    when (menu.itemId) {
                        MENU_DETAILS -> onDetails(item)
                        MENU_CREATE_HIDDEN -> onCreateHidden(item)
                        MENU_CHANGE_CREDENTIALS -> onChangeCredentials(item)
                        MENU_REMOVE -> onRemove(item)
                    }
                    true
                }
            }.show()
        }

        private fun actionDescription(item: ContainerCardUiModel): String = binding.root.context.getString(
            if (item.state == ContainerCardState.LOCKED) R.string.rv_unlock_container else R.string.rv_browse_container,
            item.entry.displayName,
        )

        private fun stateLabel(item: ContainerCardUiModel): String = binding.root.context.getString(when (item.state) {
            ContainerCardState.LOCKED -> R.string.rv_state_locked
            ContainerCardState.UNLOCKED_READ_WRITE -> R.string.rv_state_unlocked
            ContainerCardState.UNLOCKED_READ_ONLY -> R.string.rv_state_read_only
            ContainerCardState.PROTECTION_TRIGGERED -> R.string.rv_state_protection_triggered
        })

        private fun summaryLabel(item: ContainerCardUiModel): String = binding.root.context.getString(when (item.state) {
            ContainerCardState.LOCKED -> R.string.rv_summary_locked
            ContainerCardState.UNLOCKED_READ_WRITE -> R.string.rv_summary_unlocked_read_write
            ContainerCardState.UNLOCKED_READ_ONLY -> R.string.rv_summary_unlocked_read_only
            ContainerCardState.PROTECTION_TRIGGERED -> R.string.rv_summary_protection_triggered
        })

        private fun stateIcon(item: ContainerCardUiModel): Int = when (item.state) {
            ContainerCardState.LOCKED -> R.drawable.ic_container_locked
            ContainerCardState.UNLOCKED_READ_WRITE -> R.drawable.ic_container_unlocked
            ContainerCardState.UNLOCKED_READ_ONLY -> R.drawable.ic_container_read_only
            ContainerCardState.PROTECTION_TRIGGERED -> R.drawable.ic_container_protection
        }

        private fun stateColor(item: ContainerCardUiModel): Int = when (item.state) {
            ContainerCardState.LOCKED -> R.color.rv_locked
            ContainerCardState.UNLOCKED_READ_WRITE -> R.color.rv_success
            ContainerCardState.UNLOCKED_READ_ONLY -> R.color.rv_warning
            ContainerCardState.PROTECTION_TRIGGERED -> R.color.rv_error
        }

        private fun stateContainerColor(item: ContainerCardUiModel): Int = when (item.state) {
            ContainerCardState.LOCKED -> R.color.rv_locked_container
            ContainerCardState.UNLOCKED_READ_WRITE -> R.color.rv_success_container
            ContainerCardState.UNLOCKED_READ_ONLY -> R.color.rv_warning_container
            ContainerCardState.PROTECTION_TRIGGERED -> R.color.rv_error_container
        }
    }

    private companion object {
        const val MENU_DETAILS = 0
        const val MENU_CREATE_HIDDEN = 1
        const val MENU_REMOVE = 2
        const val MENU_CHANGE_CREDENTIALS = 3

    }
}

internal val CONTAINER_CARD_DIFF_CALLBACK = object : DiffUtil.ItemCallback<ContainerCardUiModel>() {
    override fun areItemsTheSame(oldItem: ContainerCardUiModel, newItem: ContainerCardUiModel): Boolean =
        oldItem.entry.id == newItem.entry.id

    override fun areContentsTheSame(oldItem: ContainerCardUiModel, newItem: ContainerCardUiModel): Boolean =
        oldItem == newItem
}
