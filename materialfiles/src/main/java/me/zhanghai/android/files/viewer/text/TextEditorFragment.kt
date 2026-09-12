/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.viewer.text

import android.content.Intent
import android.content.Context
import android.net.Uri
import android.os.Bundle
import android.provider.CalendarContract
import android.text.InputType
import android.util.TypedValue
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuInflater
import me.zhanghai.android.files.compat.forceShowIconsCompat
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
import androidx.core.view.ViewCompat
import androidx.core.widget.doAfterTextChanged
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.repeatOnLifecycle
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
import me.zhanghai.android.files.file.fileProviderUri
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
import me.zhanghai.android.files.util.createSendStreamIntent
import me.zhanghai.android.files.util.createSendTextIntent
import me.zhanghai.android.files.util.withChooser
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
    private var wrapWords = true
    private var lineNumbersEnabled = false
    private var syntaxHighlightingEnabled = true
    private var autoFormatEnabled = true
    private var editorFontSizeSp = 16
    private var pendingAfterSave: (() -> Unit)? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setHasOptionsMenu(true)
        isPreviewVisible = savedInstanceState?.getBoolean(STATE_PREVIEW, false) ?: false
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
        onBackPressedCallback = object : OnBackPressedCallback(false) {
            override fun handleOnBackPressed() {
                ConfirmCloseDialogFragment.show(this@TextEditorFragment)
            }
        }
        addOnBackPressedCallback(onBackPressedCallback)
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
            updateLineNumbers()
            // TextView.lineCount is only final after the editor has been laid out. A large
            // document can therefore initially produce just the visible/partially measured
            // prefix; recalculate once the new text has a real layout.
            binding.textEdit.post { updateLineNumbers() }
            if (isPreviewVisible) renderMarkdown(binding.textEdit.text.toString())
        }
        binding.textEdit.addOnLayoutChangeListener { _, _, _, _, _, _, _, _, _ ->
            updateLineNumbers()
        }
        binding.textEdit.isEnabled = isWritable

        if (isMarkdown) {
            renderer = createMarkdownRenderer(requireContext(), file, this::onLinkClicked)
            loadEditorPreferences()
            if (syntaxHighlightingEnabled) highlighter = MarkdownSyntaxHighlighter(binding.textEdit)
            if (isWritable) {
                updateAutoFormatFilter()
                setupFormatActions()
            }
            applyEditorPreferences()
        }
        binding.scrollView.setOnScrollChangeListener { _, _, scrollY, _, _ ->
            binding.lineNumbers.setEditorScrollY(scrollY)
        }
        binding.lineNumbers.editor = binding.textEdit
        setPreviewVisible(isPreviewVisible, restoreScroll = false)
        updateTitle()

        viewLifecycleOwner.lifecycleScope.launch {
            viewLifecycleOwner.repeatOnLifecycle(Lifecycle.State.STARTED) {
                launch {
                    viewModel.isTextChanged.collect {
                        onBackPressedCallback.isEnabled = it
                        updateTitle()
                    }
                }
                launch { viewModel.encoding.collect { onEncodingChanged(it) } }
                launch { viewModel.textState.collect(::onTextStateChanged) }
                launch { viewModel.writeFileState.collect(::onWriteFileStateChanged) }
            }
        }
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
        menu.forceShowIconsCompat()
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
        R.id.action_wrap_words -> { wrapWords = !wrapWords; applyEditorPreferences(); saveEditorPreferences(); true }
        R.id.action_line_numbers -> { lineNumbersEnabled = !lineNumbersEnabled; applyEditorPreferences(); saveEditorPreferences(); true }
        R.id.action_syntax_highlighting -> { toggleSyntaxHighlighting(); true }
        R.id.action_auto_format -> { autoFormatEnabled = !autoFormatEnabled; updateAutoFormatFilter(); saveEditorPreferences(); requireActivity().invalidateOptionsMenu(); true }
        R.id.action_font_size -> { showFontSizeDialog(); true }
        R.id.action_share_path -> { shareText(argsFile.toString(), "text/plain"); true }
        R.id.action_share_text -> { shareText(binding.textEdit.text.toString(), "text/plain"); true }
        R.id.action_share_file -> { withSavedDiskVersion(::shareSourceFile); true }
        R.id.action_share_html -> { shareText(MarkdownExport.toHtml(binding.textEdit.text.toString()), "text/html"); true }
        R.id.action_share_html_source -> { shareText(MarkdownExport.toHtml(binding.textEdit.text.toString()), "text/plain"); true }
        R.id.action_share_pdf -> { exportAndShare(ExportKind.PDF); true }
        R.id.action_share_image -> { exportAndShare(ExportKind.IMAGE); true }
        R.id.action_share_screenshot -> { exportAndShare(ExportKind.SCREENSHOT); true }
        R.id.action_share_calendar -> { shareCalendarEvent(); true }
        R.id.action_editor_info -> { withSavedDiskVersion(::showDocumentInfo); true }
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
        listOf(binding.progress, binding.errorText, binding.scrollView, binding.previewScrollView)
            .forEach { it.animate().cancel() }
        when (state) {
            is DataState.Loading -> {
                binding.progress.visibility = View.VISIBLE
                binding.errorText.visibility = View.INVISIBLE
                binding.scrollView.visibility = View.INVISIBLE
                binding.previewScrollView.visibility = View.INVISIBLE
            }
            is DataState.Success -> {
                binding.progress.visibility = View.INVISIBLE
                binding.errorText.visibility = View.INVISIBLE
                if (!viewModel.isTextChanged.value) setText(state.data)
                if (isPreviewVisible) renderMarkdown(binding.textEdit.text.toString())
                else {
                    binding.previewScrollView.visibility = View.GONE
                    binding.scrollView.alpha = 1f
                    binding.scrollView.visibility = View.VISIBLE
                }
            }
            is DataState.Error -> {
                state.throwable.printStackTrace()
                renderJob?.cancel()
                binding.progress.visibility = View.INVISIBLE
                binding.errorText.alpha = 1f
                binding.errorText.visibility = View.VISIBLE
                binding.errorText.text = state.throwable.toString()
                binding.scrollView.visibility = View.INVISIBLE
                binding.previewScrollView.visibility = View.INVISIBLE
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
        updateLineNumbers()
        binding.textEdit.post { updateLineNumbers() }
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
                pendingAfterSave?.also { pendingAfterSave = null }?.invoke()
            }
            is ActionState.Error -> {
                pendingAfterSave = null
                viewModel.finishWritingFile()
            }
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
        menuBinding.menu.findItem(R.id.action_wrap_words)?.isChecked = wrapWords
        menuBinding.menu.findItem(R.id.action_line_numbers)?.isChecked = lineNumbersEnabled
        menuBinding.menu.findItem(R.id.action_syntax_highlighting)?.isChecked = syntaxHighlightingEnabled
        menuBinding.menu.findItem(R.id.action_auto_format)?.isChecked = autoFormatEnabled
    }

    private fun setupFormatActions() {
        binding.actionBar.removeAllViews()
        val actions = listOf(
            Triple(R.string.markdown_action_heading, R.drawable.text_heading_24dp, MarkdownFormatAction.HEADING),
            Triple(R.string.markdown_action_bold, R.drawable.text_bold_24dp, MarkdownFormatAction.BOLD),
            Triple(R.string.markdown_action_italic, R.drawable.text_italic_24dp, MarkdownFormatAction.ITALIC),
            Triple(R.string.markdown_action_strike, R.drawable.text_strike_24dp, MarkdownFormatAction.STRIKE),
            Triple(R.string.markdown_action_quote, R.drawable.text_quote_24dp, MarkdownFormatAction.QUOTE),
            Triple(R.string.markdown_action_code, R.drawable.text_code_24dp, MarkdownFormatAction.INLINE_CODE),
            Triple(R.string.markdown_action_code_block, R.drawable.file_code_icon, MarkdownFormatAction.CODE_BLOCK),
            Triple(R.string.markdown_action_link, R.drawable.text_link_24dp, MarkdownFormatAction.LINK),
            Triple(R.string.markdown_action_image, R.drawable.text_image_24dp, MarkdownFormatAction.IMAGE),
            Triple(R.string.markdown_action_unordered, R.drawable.text_list_24dp, MarkdownFormatAction.UNORDERED_LIST),
            Triple(R.string.markdown_action_ordered, R.drawable.text_ordered_list_24dp, MarkdownFormatAction.ORDERED_LIST),
            Triple(R.string.markdown_action_task, R.drawable.text_task_24dp, MarkdownFormatAction.TASK_LIST)
        )
        actions.forEach { (label, iconRes, action) ->
            val button = MaterialButton(requireContext()).apply {
                text = null
                icon = androidx.appcompat.content.res.AppCompatResources.getDrawable(context, iconRes)
                contentDescription = getString(label)
                ViewCompat.setTooltipText(this, getString(label))
                isAllCaps = false
                setOnLongClickListener {
                    showToast(getString(label))
                    true
                }
                minWidth = requireContext().dpToDimensionPixelSize(48)
                minimumHeight = 0
                iconGravity = MaterialButton.ICON_GRAVITY_TEXT_START
                setPadding(requireContext().dpToDimensionPixelSize(12), 0,
                    requireContext().dpToDimensionPixelSize(12), 0)
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
        binding.lineNumbers.visibility =
            if (!visible && lineNumbersEnabled) View.VISIBLE else View.GONE
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
                binding.progress.visibility = View.INVISIBLE
                binding.errorText.visibility = View.INVISIBLE
                binding.previewScrollView.alpha = 1f
                binding.previewScrollView.visibility = View.VISIBLE
            } catch (e: Exception) {
                if (!isPreviewVisible) return@launch
                binding.progress.visibility = View.INVISIBLE
                binding.errorText.alpha = 1f
                binding.errorText.visibility = View.VISIBLE
                binding.errorText.text = e.toString()
                binding.previewScrollView.visibility = View.INVISIBLE
            }
        }
    }

    private fun preferenceKey(name: String): String =
        "${argsFile.toAbsolutePath().normalize()}::$name"

    private fun loadEditorPreferences() {
        val preferences = requireContext().getSharedPreferences("text_editor_files", Context.MODE_PRIVATE)
        wrapWords = preferences.getBoolean(preferenceKey("wrap"), true)
        lineNumbersEnabled = preferences.getBoolean(preferenceKey("lines"), false)
        syntaxHighlightingEnabled = preferences.getBoolean(preferenceKey("highlight"), true)
        autoFormatEnabled = preferences.getBoolean(preferenceKey("auto"), true)
        editorFontSizeSp = preferences.getInt(preferenceKey("size"), 16).coerceIn(10, 32)
    }

    private fun saveEditorPreferences() {
        requireContext().getSharedPreferences("text_editor_files", Context.MODE_PRIVATE).edit()
            .putBoolean(preferenceKey("wrap"), wrapWords)
            .putBoolean(preferenceKey("lines"), lineNumbersEnabled)
            .putBoolean(preferenceKey("highlight"), syntaxHighlightingEnabled)
            .putBoolean(preferenceKey("auto"), autoFormatEnabled)
            .putInt(preferenceKey("size"), editorFontSizeSp)
            .apply()
        requireActivity().invalidateOptionsMenu()
    }

    private fun applyEditorPreferences() {
        binding.textEdit.setHorizontallyScrolling(!wrapWords)
        binding.textEdit.setTextSize(TypedValue.COMPLEX_UNIT_SP, editorFontSizeSp.toFloat())
        binding.markdownText.setTextSize(TypedValue.COMPLEX_UNIT_SP, editorFontSizeSp.toFloat())
        binding.lineNumbers.setTextSize(TypedValue.COMPLEX_UNIT_SP, editorFontSizeSp.toFloat())
        binding.lineNumbers.typeface = binding.textEdit.typeface
        binding.lineNumbers.includeFontPadding = binding.textEdit.includeFontPadding
        binding.lineNumbers.setLineSpacing(
            binding.textEdit.lineSpacingExtra,
            binding.textEdit.lineSpacingMultiplier,
        )
        binding.lineNumbers.visibility = if (lineNumbersEnabled && !isPreviewVisible) View.VISIBLE else View.GONE
        val horizontalPadding = requireContext().dpToDimensionPixelSize(16)
        binding.textEdit.setPadding(
            if (lineNumbersEnabled) requireContext().dpToDimensionPixelSize(56) else horizontalPadding,
            horizontalPadding,
            horizontalPadding,
            horizontalPadding,
        )
        updateLineNumbers()
        if (this::menuBinding.isInitialized) requireActivity().invalidateOptionsMenu()
    }

    private fun updateLineNumbers() {
        if (!lineNumbersEnabled || !this::binding.isInitialized) return
        binding.lineNumbers.refresh()
    }

    private fun updateAutoFormatFilter() {
        binding.textEdit.filters = binding.textEdit.filters.filterNot { it is MarkdownAutoFormatFilter }
            .let { filters ->
                if (autoFormatEnabled && isWritable) filters + MarkdownAutoFormatFilter() else filters
            }.toTypedArray()
    }

    private fun toggleSyntaxHighlighting() {
        syntaxHighlightingEnabled = !syntaxHighlightingEnabled
        highlighter?.dispose()
        highlighter = if (syntaxHighlightingEnabled) MarkdownSyntaxHighlighter(binding.textEdit) else null
        highlighter?.refresh()
        saveEditorPreferences()
    }

    private fun showFontSizeDialog() {
        val sizes = (10..32 step 2).toList()
        MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.text_editor_font_size)
            .setSingleChoiceItems(sizes.map { "$it sp" }.toTypedArray(), sizes.indexOf(editorFontSizeSp)) { dialog, which ->
                editorFontSizeSp = sizes[which]
                applyEditorPreferences()
                saveEditorPreferences()
                dialog.dismiss()
            }
            .setNegativeButton(android.R.string.cancel, null)
            .show()
    }

    private fun shareText(text: String, mimeType: String) {
        val intent = if (mimeType == "text/plain") text.createSendTextIntent() else {
            Intent(Intent.ACTION_SEND).setType(mimeType).putExtra(Intent.EXTRA_TEXT, text)
        }
        requireContext().startActivitySafe(intent.withChooser(getString(R.string.share)))
    }

    private fun shareSourceFile() {
        requireContext().startActivitySafe(
            argsFile.fileProviderUri.createSendStreamIntent(MimeType.guessFromPath(argsFile.toString()))
                .withChooser(getString(R.string.share))
        )
    }

    private fun withSavedDiskVersion(action: () -> Unit) {
        if (!viewModel.isTextChanged.value) {
            action()
            return
        }
        MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.text_editor_unsaved_title)
            .setMessage(R.string.text_editor_unsaved_share_message)
            .setPositiveButton(R.string.save) { _, _ -> pendingAfterSave = action; save() }
            .setNeutralButton(R.string.text_editor_use_disk_version) { _, _ -> action() }
            .setNegativeButton(android.R.string.cancel, null)
            .show()
    }

    private enum class ExportKind { PDF, IMAGE, SCREENSHOT }

    private fun exportAndShare(kind: ExportKind) {
        val text = binding.textEdit.text.toString()
        val width = binding.scrollView.width
        val viewportHeight = binding.scrollView.height
        val name = argsFile.fileName.toString()
        viewLifecycleOwner.lifecycleScope.launch {
            runCatching {
                withContext(Dispatchers.IO) {
                    when (kind) {
                        ExportKind.PDF -> listOf(MarkdownExport.writePdf(requireContext(), name, text))
                        ExportKind.IMAGE -> MarkdownExport.writeImages(requireContext(), name, text, width)
                        ExportKind.SCREENSHOT -> MarkdownExport.writeImages(requireContext(), name, text, width, viewportHeight)
                    }
                }
            }.onSuccess { uris ->
                val mime = if (kind == ExportKind.PDF) MimeType.PDF else MimeType.IMAGE_ANY
                requireContext().startActivitySafe(
                    uris.createSendStreamIntent(List(uris.size) { mime }).withChooser(getString(R.string.share))
                )
            }.onFailure { showToast(it.toString()) }
        }
    }

    private fun shareCalendarEvent() {
        val intent = Intent(Intent.ACTION_INSERT)
            .setData(CalendarContract.Events.CONTENT_URI)
            .putExtra(CalendarContract.Events.TITLE, argsFile.fileName.toString())
            .putExtra(CalendarContract.Events.DESCRIPTION, binding.textEdit.text.toString())
        requireContext().startActivitySafe(intent)
    }

    private fun showDocumentInfo() {
        viewLifecycleOwner.lifecycleScope.launch {
            val info = withContext(Dispatchers.IO) {
                val attributes = runCatching { argsFile.readAttributes(BasicFileAttributes::class.java) }.getOrNull()
                val text = binding.textEdit.text.toString()
                listOf(
                    getString(R.string.text_editor_info_path, argsFile.toString()),
                    getString(R.string.text_editor_info_encoding, viewModel.encoding.value.displayName()),
                    getString(R.string.text_editor_info_size, attributes?.size() ?: 0L),
                    getString(R.string.text_editor_info_modified, attributes?.lastModifiedTime()?.toString() ?: "—"),
                    getString(R.string.text_editor_info_lines, text.lineSequence().count()),
                    getString(R.string.text_editor_info_words, Regex("\\S+").findAll(text).count()),
                    getString(R.string.text_editor_info_characters, text.length),
                ).joinToString("\n")
            }
            MaterialAlertDialogBuilder(requireContext())
                .setTitle(argsFile.fileName.toString())
                .setMessage(info)
                .setPositiveButton(android.R.string.ok, null)
                .show()
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
