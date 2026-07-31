/*
 * Copyright (c) 2026
 */

package me.zhanghai.android.files.viewer.markdown

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import java8.nio.file.Path
import java8.nio.file.attribute.BasicFileAttributes
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import me.zhanghai.android.files.R
import me.zhanghai.android.files.databinding.MarkdownViewerFragmentBinding
import me.zhanghai.android.files.file.MimeType
import me.zhanghai.android.files.file.guessFromPath
import me.zhanghai.android.files.filelist.FileListActivity
import me.zhanghai.android.files.filelist.OpenFileActivity
import me.zhanghai.android.files.provider.common.readAttributes
import me.zhanghai.android.files.ui.ThemedFastScroller
import me.zhanghai.android.files.util.DataState
import me.zhanghai.android.files.util.extraPath
import me.zhanghai.android.files.util.fadeInUnsafe
import me.zhanghai.android.files.util.fadeOutUnsafe
import me.zhanghai.android.files.util.showToast
import me.zhanghai.android.files.util.startActivitySafe
import me.zhanghai.android.files.util.viewModels
import me.zhanghai.android.files.viewer.text.TextEditorActivity
import me.zhanghai.android.files.viewer.text.TextEditorViewModel

class MarkdownViewerFragment : Fragment() {
    private lateinit var file: Path
    private lateinit var binding: MarkdownViewerFragmentBinding
    private lateinit var renderer: io.noties.markwon.Markwon
    private val viewModel by viewModels { { TextEditorViewModel(file) } }

    private var renderJob: Job? = null
    private var editorLaunched = false
    private val rendererMutex = Mutex()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setHasOptionsMenu(true)
        file = requireActivity().intent.extraPath ?: run {
            requireActivity().finish()
            return
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View = MarkdownViewerFragmentBinding.inflate(inflater, container, false)
        .also { binding = it }
        .root

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        val activity = requireActivity() as AppCompatActivity
        activity.setSupportActionBar(binding.toolbar)
        activity.supportActionBar?.setDisplayHomeAsUpEnabled(true)
        activity.title = file.fileName?.toString() ?: getString(R.string.markdown_viewer_title)

        ThemedFastScroller.create(binding.scrollView)
        renderer = createMarkdownRenderer(requireContext(), file, this::onLinkClicked)
        viewLifecycleOwner.lifecycleScope.launch {
            viewModel.textState.collect(::onTextStateChanged)
        }
    }

    override fun onResume() {
        super.onResume()
        if (editorLaunched) {
            editorLaunched = false
            viewModel.reload()
        }
    }

    override fun onDestroyView() {
        renderJob?.cancel()
        super.onDestroyView()
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.markdown_viewer, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean = when (item.itemId) {
        R.id.action_edit_source -> {
            editorLaunched = true
            startActivity(TextEditorActivity.createIntent(file, forceTextEditor = true))
            true
        }
        R.id.action_reload -> {
            viewModel.reload()
            true
        }
        else -> super.onOptionsItemSelected(item)
    }

    fun onSupportNavigateUp(): Boolean = false

    private fun onTextStateChanged(state: DataState<String>) {
        when (state) {
            is DataState.Loading -> {
                renderJob?.cancel()
                binding.progress.fadeInUnsafe()
                binding.errorText.fadeOutUnsafe()
                binding.scrollView.fadeOutUnsafe()
            }
            is DataState.Success -> renderMarkdown(state.data)
            is DataState.Error -> {
                renderJob?.cancel()
                binding.progress.fadeOutUnsafe()
                binding.errorText.fadeInUnsafe()
                binding.errorText.text = state.throwable.toString()
                binding.scrollView.fadeOutUnsafe()
            }
        }
    }

    private fun renderMarkdown(markdown: String) {
        renderJob?.cancel()
        renderJob = viewLifecycleOwner.lifecycleScope.launch {
            try {
                val rendered = withContext(Dispatchers.Default) {
                    rendererMutex.withLock { renderer.toMarkdown(markdown) }
                }
                renderer.setParsedMarkdown(binding.markdownText, rendered)
                binding.progress.fadeOutUnsafe()
                binding.errorText.fadeOutUnsafe()
                binding.scrollView.fadeInUnsafe()
            } catch (e: Exception) {
                binding.progress.fadeOutUnsafe()
                binding.errorText.fadeInUnsafe()
                binding.errorText.text = e.toString()
                binding.scrollView.fadeOutUnsafe()
            }
        }
    }

    private fun onLinkClicked(link: String) {
        if (isMarkdownWebLink(link)) {
            requireContext().startActivitySafe(Intent(Intent.ACTION_VIEW, Uri.parse(link)))
            return
        }
        val target = resolveMarkdownRelativePath(file, link) ?: return
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
}
