/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.viewer.text

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.text.InputType
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.SubMenu
import android.view.View
import android.view.ViewGroup
import android.widget.CheckBox
import android.widget.LinearLayout
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.AppCompatEditText
import androidx.core.view.children
import androidx.core.widget.doAfterTextChanged
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.google.android.material.button.MaterialButton
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import java8.nio.file.Path
import java8.nio.file.attribute.BasicFileAttributes
import java.nio.charset.Charset
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import kotlinx.parcelize.Parcelize
import me.zhanghai.android.files.R
import me.zhanghai.android.files.databinding.TextEditorFragmentBinding
import me.zhanghai.android.files.file.MimeType
import me.zhanghai.android.files.file.asMimeTypeOrNull
import me.zhanghai.android.files.file.guessFromPath
import me.zhanghai.android.files.file.isMarkdownFile
import me.zhanghai.android.files.filelist.FileListActivity
import me.zhanghai.android.files.filelist.OpenFileActivity
import me.zhanghai.android.files.provider.common.readAttributes
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.ui.ThemedFastScroller
import me.zhanghai.android.files.util.ActionState
import me.zhanghai.android.files.util.DataState
import me.zhanghai.android.files.util.ParcelableArgs
import me.zhanghai.android.files.util.addOnBackPressedCallback
import me.zhanghai.android.files.util.args
import me.zhanghai.android.files.util.dpToDimensionPixelSize
import me.zhanghai.android.files.util.extraPath
import me.zhanghai.android.files.util.fadeInUnsafe
import me.zhanghai.android.files.util.fadeOutUnsafe
import me.zhanghai.android.files.util.isReady
import me.zhanghai.android.files.util.showToast
import me.zhanghai.android.files.util.startActivitySafe
import me.zhanghai.android.files.util.valueCompat
import me.zhanghai.android.files.util.viewModels
import me.zhanghai.android.files.viewer.markdown.createMarkdownRenderer
import me.zhanghai.android.files.viewer.markdown.isMarkdownWebLink
import me.zhanghai.android.files.viewer.markdown.resolveMarkdownRelativePath

class TextEditorFragment : Fragment(), ConfirmReloadDialogFragment.Listener,
    ConfirmCloseDialogFragment.Listener {
    private val args by args<Args>()
    private lateinit var argsFile: Path
    private lateinit var binding: TextEditorFragmentBinding
    private lateinit var menuBinding: MenuBinding
    private val viewModel by viewModels { { TextEditorViewModel(argsFile) } }
    private lateinit var onBackPressedCallback: OnBackPressedCallback

    private var isSettingText = false
    private var isWritable = false
    private var isMarkdown = false
    private var isPreviewVisible = false
    private var editorScrollY = 0
    private var previewScrollY = 0
    private var renderJob: Job? = null
    private var renderer: io.noties.markwon.Markwon? = null
    private val rendererMutex = Mutex()
    private var highlighter: MarkdownSyntaxHighlighter? = null
    private var undoRedo: TextViewUndoRedo? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setHasOptionsMenu(true)
        isPreviewVisible = savedInstanceState?.getBoolean(STATE_PREVIEW, false) ?: false

        lifecycleScope.launchWhenStarted {
            onBackPressedCallback = object : OnBackPressedCallback(false) {
                override fun handleOnBackPressed() {
                    ConfirmCloseDialogFragment.show(this@TextEditorFragment)
                }
            }
            launch {
                viewModel.isTextChanged.collect {
                    onBackPressedCallback.isEnabled = viewModel.isTextChanged.value
                    updateTitle()
                }
            }
            addOnBackPressedCallback(onBackPressedCallback)
            launch { viewModel.encoding.collect { onEncodingChanged(it) } }
            launch { viewModel.textState.collect(::onTextStateChanged) }
            launch { viewModel.writeFileState.collect { onWriteFileStateChanged(it) } }
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View = TextEditorFragmentBinding.inflate(inflater, container, false)
        .also { binding = it }
        .root

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        val file = args.intent.extraPath
        if (file == null) {
            finish()
            return
        }
        argsFile = file
        isMarkdown = isMarkdownFile(file, args.intent.type?.asMimeTypeOrNull())
        if (isMarkdown && !args.intent.getBooleanExtra(EXTRA_FORCE_TEXT_EDITOR, false)) {
            isPreviewVisible = savedInstanceState?.getBoolean(STATE_PREVIEW)
                ?: Settings.MARKDOWN_RENDERING_ENABLED.valueCompat
        }

        val externalUri = args.intent.data
        isWritable = !file.fileSystem.isReadOnly && (externalUri == null ||
            args.intent.flags and Intent.FLAG_GRANT_WRITE_URI_PERMISSION != 0)

        val activity = requireActivity() as AppCompatActivity
        activity.lifecycleScope.launchWhenCreated {
            activity.setSupportActionBar(binding.toolbar)
            activity.supportActionBar!!.setDisplayHomeAsUpEnabled(true)
        }

        ThemedFastScroller.create(binding.scrollView)
        ThemedFastScroller.create(binding.previewScrollView)
        binding.textEdit.isSaveEnabled = false
        undoRedo = TextViewUndoRedo(binding.textEdit) { if (isAdded) requireActivity().invalidateOptionsMenu() }
        val textEditSavedState = viewModel.removeEditTextSavedState()
        if (textEditSavedState != null) binding.textEdit.onRestoreInstanceState(textEditSavedState)

        binding.textEdit.doAfterTextChanged {
            if (isSettingText || viewModel.textState.value !is DataState.Success) return@doAfterTextChanged
            viewModel.isTextChanged.value = true
            if (isPreviewVisible) renderMarkdown(binding.textEdit.text.toString())
        }
        binding.textEdit.isEnabled = isWritable

        if (isMarkdown) {
            renderer = createMarkdownRenderer(requireContext(), file, this::onLinkClicked)
            highlighter = MarkdownSyntaxHighlighter(binding.textEdit)
            if (isWritable) {
                binding.textEdit.filters = binding.textEdit.filters + MarkdownAutoFormatFilter()
                setupFormatActions()
            }
        }
        setPreviewVisible(isPreviewVisible, restoreScroll = false)
        updateTitle()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        if (this::binding.isInitialized) {
            viewModel.setEditTextSavedState(binding.textEdit.onSaveInstanceState())
            editorScrollY = binding.scrollView.scrollY
            previewScrollY = binding.previewScrollView.scrollY
        }
        outState.putBoolean(STATE_PREVIEW, isPreviewVisible)
        super.onSaveInstanceState(outState)
    }

    override fun onDestroyView() {
        renderJob?.cancel()
        highlighter?.dispose()
        highlighter = null
        undoRedo = null
        renderer = null
        super.onDestroyView()
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        super.onCreateOptionsMenu(menu, inflater)
        menuBinding = MenuBinding.inflate(menu, inflater)
    }

    override fun onPrepareOptionsMenu(menu: Menu) {
        super.onPrepareOptionsMenu(menu)
        updateMenuItems()
        updateEncodingMenuItems()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean = when (item.itemId) {
        R.id.action_save -> {
            save()
            true
        }
        R.id.action_undo -> {
            undoRedo?.undo()
            viewModel.isTextChanged.value = true
            true
        }
        R.id.action_redo -> {
            undoRedo?.redo()
            viewModel.isTextChanged.value = true
            true
        }
        R.id.action_preview -> {
            setPreviewVisible(true)
            true
        }
        R.id.action_edit -> {
            setPreviewVisible(false)
            true
        }
        R.id.action_search -> {
            showSearchReplaceDialog()
            true
        }
        R.id.action_reload -> {
            onReload()
            true
        }
        Menu.FIRST -> {
            viewModel.encoding.value = Charset.forName(item.titleCondensed!!.toString())
            true
        }
        else -> super.onOptionsItemSelected(item)
    }

    fun onSupportNavigateUp(): Boolean {
        if (onBackPressedCallback.isEnabled) {
            onBackPressedCallback.handleOnBackPressed()
            return true
        }
        return false
    }

    override fun finish() {
        requireActivity().finish()
    }

    private fun onEncodingChanged(encoding: Charset) = updateEncodingMenuItems()

    private fun updateEncodingMenuItems() {
        if (!this::menuBinding.isInitialized) return
        val charsetName = viewModel.encoding.value.name()
        menuBinding.encodingSubMenu.children
            .find { it.titleCondensed == charsetName }
            ?.isChecked = true
    }

    private fun onTextStateChanged(state: DataState<String>) {
        updateTitle()
        when (state) {
            is DataState.Loading -> {
                binding.progress.fadeInUnsafe()
                binding.errorText.fadeOutUnsafe()
                binding.scrollView.fadeOutUnsafe()
                binding.previewScrollView.fadeOutUnsafe()
            }
            is DataState.Success -> {
                binding.progress.fadeOutUnsafe()
                binding.errorText.fadeOutUnsafe()
                if (!viewModel.isTextChanged.value) setText(state.data)
                if (isPreviewVisible) renderMarkdown(binding.textEdit.text.toString())
                else binding.scrollView.fadeInUnsafe()
            }
            is DataState.Error -> {
                state.throwable.printStackTrace()
                renderJob?.cancel()
                binding.progress.fadeOutUnsafe()
                binding.errorText.fadeInUnsafe()
                binding.errorText.text = state.throwable.toString()
                binding.scrollView.fadeOutUnsafe()
                binding.previewScrollView.fadeOutUnsafe()
            }
        }
    }

    private fun setText(text: String?) {
        isSettingText = true
        undoRedo?.setTextWithoutHistory(text)
        if (undoRedo == null) binding.textEdit.setText(text)
        isSettingText = false
        viewModel.isTextChanged.value = false
        highlighter?.refresh()
    }

    private fun updateTitle() {
        if (!this::binding.isInitialized) return
        val fileName = viewModel.file.value.fileName.toString()
        val changed = viewModel.isTextChanged.value
        requireActivity().title = getString(
            if (changed) R.string.text_editor_title_changed_format else R.string.text_editor_title_format,
            fileName
        )
    }

    private fun onReload() {
        if (viewModel.isTextChanged.value) ConfirmReloadDialogFragment.show(this) else reload()
    }

    override fun reload() {
        viewModel.isTextChanged.value = false
        viewModel.reload()
    }

    private fun save() {
        if (!isWritable || !viewModel.writeFileState.value.isReady) return
        viewModel.writeFile(argsFile, binding.textEdit.text.toString(), requireContext())
    }

    private fun onWriteFileStateChanged(state: ActionState<Pair<Path, String>, Unit>) {
        when (state) {
            is ActionState.Ready, is ActionState.Running -> updateMenuItems()
            is ActionState.Success -> {
                showToast(R.string.text_editor_save_success)
                viewModel.finishWritingFile()
                viewModel.isTextChanged.value = false
            }
            is ActionState.Error -> viewModel.finishWritingFile()
        }
    }

    private fun updateMenuItems() {
        if (!this::menuBinding.isInitialized) return
        val canEdit = isWritable && !isPreviewVisible
        menuBinding.saveItem.isEnabled = isWritable && viewModel.writeFileState.value.isReady
        menuBinding.undoItem.isVisible = !isPreviewVisible
        menuBinding.redoItem.isVisible = !isPreviewVisible
        menuBinding.undoItem.isEnabled = canEdit && undoRedo?.canUndo == true
        menuBinding.redoItem.isEnabled = canEdit && undoRedo?.canRedo == true
        menuBinding.previewItem.isVisible = isMarkdown && !isPreviewVisible
        menuBinding.editItem.isVisible = isMarkdown && isPreviewVisible
        menuBinding.searchItem.isVisible = viewModel.textState.value is DataState.Success
    }

    private fun setupFormatActions() {
        binding.actionBar.removeAllViews()
        val actions = listOf(
            R.string.markdown_action_heading to MarkdownFormatAction.HEADING,
            R.string.markdown_action_bold to MarkdownFormatAction.BOLD,
            R.string.markdown_action_italic to MarkdownFormatAction.ITALIC,
            R.string.markdown_action_strike to MarkdownFormatAction.STRIKE,
            R.string.markdown_action_quote to MarkdownFormatAction.QUOTE,
            R.string.markdown_action_code to MarkdownFormatAction.INLINE_CODE,
            R.string.markdown_action_code_block to MarkdownFormatAction.CODE_BLOCK,
            R.string.markdown_action_link to MarkdownFormatAction.LINK,
            R.string.markdown_action_image to MarkdownFormatAction.IMAGE,
            R.string.markdown_action_unordered to MarkdownFormatAction.UNORDERED_LIST,
            R.string.markdown_action_ordered to MarkdownFormatAction.ORDERED_LIST,
            R.string.markdown_action_task to MarkdownFormatAction.TASK_LIST
        )
        actions.forEach { (label, action) ->
            val button = MaterialButton(requireContext()).apply {
                text = getString(label)
                isAllCaps = false
                minWidth = 0
                minimumHeight = 0
                setPadding(requireContext().dpToDimensionPixelSize(10), 0,
                    requireContext().dpToDimensionPixelSize(10), 0)
                setOnClickListener {
                    if (!isWritable || isPreviewVisible) return@setOnClickListener
                    MarkdownFormatActions.apply(binding.textEdit, action)
                    viewModel.isTextChanged.value = true
                    highlighter?.refresh()
                }
            }
            binding.actionBar.addView(button, LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                requireContext().dpToDimensionPixelSize(40)
            ))
        }
        binding.actionBarScrollView.visibility = if (isWritable && !isPreviewVisible) View.VISIBLE else View.GONE
    }

    private fun setPreviewVisible(visible: Boolean, restoreScroll: Boolean = true) {
        if (!isMarkdown || !this::binding.isInitialized) return
        if (visible == isPreviewVisible && restoreScroll) {
            if (visible) renderMarkdown(binding.textEdit.text.toString())
            return
        }
        if (visible) editorScrollY = binding.scrollView.scrollY else previewScrollY = binding.previewScrollView.scrollY
        isPreviewVisible = visible
        binding.scrollView.visibility = if (visible) View.GONE else View.VISIBLE
        binding.previewScrollView.visibility = if (visible) View.VISIBLE else View.GONE
        binding.actionBarScrollView.visibility = if (!visible && isWritable) View.VISIBLE else View.GONE
        if (visible) {
            renderMarkdown(binding.textEdit.text.toString())
            binding.previewScrollView.post { binding.previewScrollView.scrollTo(0, previewScrollY) }
        } else {
            binding.scrollView.post { binding.scrollView.scrollTo(0, editorScrollY) }
            binding.textEdit.requestFocus()
        }
        if (this::menuBinding.isInitialized) requireActivity().invalidateOptionsMenu()
    }

    private fun renderMarkdown(markdown: String) {
        val currentRenderer = renderer ?: return
        renderJob?.cancel()
        renderJob = viewLifecycleOwner.lifecycleScope.launch {
            try {
                val rendered = withContext(Dispatchers.Default) {
                    rendererMutex.withLock { currentRenderer.toMarkdown(markdown) }
                }
                if (!isPreviewVisible || markdown != binding.textEdit.text.toString()) return@launch
                currentRenderer.setParsedMarkdown(binding.markdownText, rendered)
                binding.progress.fadeOutUnsafe()
                binding.errorText.fadeOutUnsafe()
                binding.previewScrollView.fadeInUnsafe()
            } catch (e: Exception) {
                if (!isPreviewVisible) return@launch
                binding.progress.fadeOutUnsafe()
                binding.errorText.fadeInUnsafe()
                binding.errorText.text = e.toString()
                binding.previewScrollView.fadeOutUnsafe()
            }
        }
    }

    private fun onLinkClicked(link: String) {
        if (isMarkdownWebLink(link)) {
            requireContext().startActivitySafe(Intent(Intent.ACTION_VIEW, Uri.parse(link)))
            return
        }
        val target = resolveMarkdownRelativePath(argsFile, link) ?: return
        viewLifecycleOwner.lifecycleScope.launch {
            val attributes = withContext(Dispatchers.IO) {
                runCatching { target.readAttributes(BasicFileAttributes::class.java) }
            }.getOrElse {
                showToast(R.string.markdown_viewer_link_error)
                return@launch
            }
            val intent = if (attributes.isDirectory) {
                FileListActivity.createViewIntent(target)
            } else {
                OpenFileActivity.createIntent(target, MimeType.guessFromPath(target.toString()))
            }
            requireContext().startActivitySafe(intent)
        }
    }

    private fun showSearchReplaceDialog() {
        val container = LinearLayout(requireContext()).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(24, 0, 24, 0)
        }
        val findEdit = AppCompatEditText(requireContext()).apply {
            hint = getString(R.string.markdown_find)
            inputType = InputType.TYPE_CLASS_TEXT
        }
        val replaceEdit = AppCompatEditText(requireContext()).apply {
            hint = getString(R.string.markdown_replace_with)
            inputType = InputType.TYPE_CLASS_TEXT
        }
        val caseCheck = CheckBox(requireContext()).apply {
            text = getString(R.string.markdown_case_sensitive)
        }
        container.addView(findEdit)
        container.addView(replaceEdit)
        container.addView(caseCheck)
        val dialog = MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.markdown_search_replace)
            .setView(container)
            .setNegativeButton(R.string.close, null)
            .setNeutralButton(R.string.markdown_find_next, null)
            .setPositiveButton(R.string.markdown_replace_all, null)
            .create()
        dialog.setOnShowListener {
            dialog.getButton(android.content.DialogInterface.BUTTON_POSITIVE).isEnabled = isWritable
            dialog.getButton(android.content.DialogInterface.BUTTON_NEUTRAL).setOnClickListener {
                findNext(findEdit.text.toString(), caseCheck.isChecked)
            }
            dialog.getButton(android.content.DialogInterface.BUTTON_POSITIVE).setOnClickListener {
                if (!isWritable) return@setOnClickListener
                replaceAll(findEdit.text.toString(), replaceEdit.text.toString(), caseCheck.isChecked)
                dialog.dismiss()
            }
        }
        dialog.show()
        findEdit.requestFocus()
    }

    private fun findNext(query: String, caseSensitive: Boolean) {
        if (query.isEmpty()) return
        setPreviewVisible(false)
        val source = binding.textEdit.text.toString()
        val from = binding.textEdit.selectionEnd.coerceAtLeast(0)
        val index = source.indexOf(query, from, ignoreCase = !caseSensitive).let {
            if (it >= 0) it else source.indexOf(query, 0, ignoreCase = !caseSensitive)
        }
        if (index < 0) {
            showToast(R.string.markdown_search_not_found)
        } else {
            binding.textEdit.requestFocus()
            binding.textEdit.setSelection(index, index + query.length)
        }
    }

    private fun replaceAll(query: String, replacement: String, caseSensitive: Boolean) {
        if (!isWritable || query.isEmpty()) return
        val source = binding.textEdit.text.toString()
        val replaced = source.replace(query, replacement, ignoreCase = !caseSensitive)
        if (source != replaced) {
            binding.textEdit.text!!.replace(0, binding.textEdit.length(), replaced)
            viewModel.isTextChanged.value = true
            highlighter?.refresh()
        }
    }

    @Parcelize
    class Args(val intent: Intent) : ParcelableArgs

    private class MenuBinding private constructor(
        val menu: Menu,
        val saveItem: MenuItem,
        val undoItem: MenuItem,
        val redoItem: MenuItem,
        val previewItem: MenuItem,
        val editItem: MenuItem,
        val searchItem: MenuItem,
        val encodingSubMenu: SubMenu
    ) {
        companion object {
            fun inflate(menu: Menu, inflater: MenuInflater): MenuBinding {
                inflater.inflate(R.menu.text_editor, menu)
                val encodingSubMenu = menu.findItem(R.id.action_encoding).subMenu!!
                for ((charsetName, charset) in Charset.availableCharsets()) {
                    encodingSubMenu.add(Menu.NONE, Menu.FIRST, Menu.NONE, charset.displayName())
                        .titleCondensed = charsetName
                }
                encodingSubMenu.setGroupCheckable(Menu.NONE, true, true)
                return MenuBinding(
                    menu,
                    menu.findItem(R.id.action_save),
                    menu.findItem(R.id.action_undo),
                    menu.findItem(R.id.action_redo),
                    menu.findItem(R.id.action_preview),
                    menu.findItem(R.id.action_edit),
                    menu.findItem(R.id.action_search),
                    encodingSubMenu
                )
            }
        }
    }

    companion object {
        private const val STATE_PREVIEW = "text_editor_preview"
        private const val EXTRA_FORCE_TEXT_EDITOR =
            "me.zhanghai.android.files.viewer.text.extra.FORCE_TEXT_EDITOR"
    }
}
