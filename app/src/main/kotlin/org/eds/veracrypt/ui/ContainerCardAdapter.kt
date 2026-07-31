package org.eds.veracrypt.ui

import android.graphics.Color
import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.PopupMenu
import androidx.core.view.isVisible
import androidx.recyclerview.widget.RecyclerView
import com.sovworks.eds.android.R
import com.sovworks.eds.android.databinding.ItemContainerCardBinding

internal class ContainerCardAdapter(
    private val onUnlock: (ContainerCardUiModel) -> Unit,
    private val onBrowse: (ContainerCardUiModel) -> Unit,
    private val onDetails: (ContainerCardUiModel) -> Unit,
    private val onLock: (ContainerCardUiModel) -> Unit,
    private val onCreateHidden: (ContainerCardUiModel) -> Unit,
    private val onChangeCredentials: (ContainerCardUiModel) -> Unit,
    private val onRemove: (ContainerCardUiModel) -> Unit,
) : RecyclerView.Adapter<ContainerCardAdapter.Holder>() {
    private var items: List<ContainerCardUiModel> = emptyList()

    fun submitList(models: List<ContainerCardUiModel>) {
        items = models
        notifyDataSetChanged()
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): Holder = Holder(
        ItemContainerCardBinding.inflate(LayoutInflater.from(parent.context), parent, false),
    )

    override fun onBindViewHolder(holder: Holder, position: Int) = holder.bind(items[position])
    override fun getItemCount(): Int = items.size

    inner class Holder(private val binding: ItemContainerCardBinding) : RecyclerView.ViewHolder(binding.root) {
        fun bind(item: ContainerCardUiModel) = with(binding) {
            containerName.text = item.entry.displayName
            containerState.text = stateLabel(item)
            containerState.chipBackgroundColor = android.content.res.ColorStateList.valueOf(stateColor(item))
            containerState.setTextColor(Color.WHITE)
            containerSummary.text = summaryLabel(item)
            if (item.state == ContainerCardState.LOCKED) {
                primaryAction.setText(R.string.vc_unlock)
                primaryAction.setOnClickListener { onUnlock(item) }
            } else {
                primaryAction.setText(R.string.vc_browse_volume)
                primaryAction.setOnClickListener { onBrowse(item) }
            }
            detailsAction.isVisible = item.canShowDetails
            detailsAction.setOnClickListener { onDetails(item) }
            lockAction.isVisible = item.canLock
            lockAction.setOnClickListener { onLock(item) }
            moreActions.setOnClickListener { showMore(item) }
        }

        private fun showMore(item: ContainerCardUiModel) {
            PopupMenu(binding.root.context, binding.moreActions).apply {
                if (item.canCreateHiddenVolume) menu.add(0, MENU_CREATE_HIDDEN, 0, R.string.vc_create_hidden_volume)
                if (item.canChangeCredentials) menu.add(0, MENU_CHANGE_CREDENTIALS, 1, R.string.vc_change_credentials)
                menu.add(0, MENU_REMOVE, 2, R.string.vc_remove_container)
                setOnMenuItemClickListener { menu ->
                    when (menu.itemId) {
                        MENU_CREATE_HIDDEN -> onCreateHidden(item)
                        MENU_CHANGE_CREDENTIALS -> onChangeCredentials(item)
                        MENU_REMOVE -> onRemove(item)
                    }
                    true
                }
            }.show()
        }

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

        private fun stateColor(item: ContainerCardUiModel): Int = binding.root.context.getColor(when (item.state) {
            ContainerCardState.LOCKED -> R.color.rv_secondary
            ContainerCardState.UNLOCKED_READ_WRITE -> R.color.rv_success
            ContainerCardState.UNLOCKED_READ_ONLY -> R.color.rv_warning
            ContainerCardState.PROTECTION_TRIGGERED -> R.color.rv_error
        })
    }

    private companion object {
        const val MENU_CREATE_HIDDEN = 1
        const val MENU_REMOVE = 2
        const val MENU_CHANGE_CREDENTIALS = 3
    }
}
