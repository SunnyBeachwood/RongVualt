/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filelist

import android.app.Dialog
import android.animation.ValueAnimator
import android.content.ClipData
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Color
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.SystemClock
import android.util.Log
import android.text.TextUtils
import android.view.Gravity
import android.view.KeyCharacterMap
import android.view.KeyEvent
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import android.widget.LinearLayout
import android.widget.ImageView
import android.widget.EditText
import android.widget.ProgressBar
import android.widget.TextView
import androidx.activity.OnBackPressedCallback
import androidx.activity.result.contract.ActivityResultContract
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.SearchView
import androidx.core.content.pm.ShortcutInfoCompat
import androidx.core.content.pm.ShortcutManagerCompat
import androidx.core.animation.doOnEnd
import androidx.core.graphics.drawable.IconCompat
import androidx.core.view.GravityCompat
import androidx.core.view.isVisible
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.updatePadding
import androidx.drawerlayout.widget.DrawerLayout
import androidx.fragment.app.Fragment
import androidx.fragment.app.commit
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import com.google.android.material.button.MaterialButton
import com.google.android.material.card.MaterialCardView
import com.google.android.material.color.MaterialColors
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import java8.nio.file.Path
import java8.nio.file.Paths
import java.net.URI
import java.util.Locale
import java.util.EnumMap
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import me.zhanghai.android.files.R
import me.zhanghai.android.files.app.application
import me.zhanghai.android.files.app.clipboardManager
import me.zhanghai.android.files.compat.checkSelfPermissionCompat
import me.zhanghai.android.files.file.FileItem
import me.zhanghai.android.files.file.JavaFile
import me.zhanghai.android.files.file.asFileSize
import me.zhanghai.android.files.file.MimeType
import me.zhanghai.android.files.file.asMimeTypeOrNull
import me.zhanghai.android.files.file.extension
import me.zhanghai.android.files.file.fileProviderUri
import me.zhanghai.android.files.file.isApk
import me.zhanghai.android.files.file.isImage
import me.zhanghai.android.files.file.loadFileItem
import me.zhanghai.android.files.filejob.FileJobService
import me.zhanghai.android.files.filejob.FileJobResult
import me.zhanghai.android.files.filejob.FileJobOperation
import me.zhanghai.android.files.filejob.FileJobProgress
import me.zhanghai.android.files.filejob.FileJobProgressPhase
import me.zhanghai.android.files.filejob.FileJobProgressRegistry
import me.zhanghai.android.files.databinding.ArchivePasswordDialogBinding
import me.zhanghai.android.files.filelist.FileSortOptions.By
import me.zhanghai.android.files.filelist.FileSortOptions.Order
import me.zhanghai.android.files.fileproperties.FilePropertiesDialogFragment
import me.zhanghai.android.files.navigation.BookmarkDirectories
import me.zhanghai.android.files.navigation.BookmarkDirectory
import me.zhanghai.android.files.navigation.NavigationFragment
import me.zhanghai.android.files.provider.archive.archiveFile
import me.zhanghai.android.files.provider.archive.createArchiveRootPath
import me.zhanghai.android.files.provider.archive.isArchivePath
import me.zhanghai.android.files.provider.document.isDocumentPath
import me.zhanghai.android.files.provider.linux.isLinuxPath
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.terminal.Terminal
import me.zhanghai.android.files.ui.CoordinatorScrollingFrameLayout
import me.zhanghai.android.files.ui.AdaptiveToolbar
import me.zhanghai.android.files.ui.DrawerLayoutOnBackPressedCallback
import me.zhanghai.android.files.ui.FixQueryChangeSearchView
import me.zhanghai.android.files.ui.ExpandableActionDockLayout
import me.zhanghai.android.files.util.DebouncedRunnable
import me.zhanghai.android.files.util.Failure
import me.zhanghai.android.files.util.Loading
import me.zhanghai.android.files.util.Success
import me.zhanghai.android.files.util.addOnBackPressedCallback
import me.zhanghai.android.files.util.args
import me.zhanghai.android.files.util.copyText
import me.zhanghai.android.files.util.createIntent
import me.zhanghai.android.files.util.createManageAppAllFilesAccessPermissionIntent
import me.zhanghai.android.files.util.createSendStreamIntent
import me.zhanghai.android.files.util.createViewIntent
import me.zhanghai.android.files.util.extraPath
import me.zhanghai.android.files.util.fadeToVisibilityUnsafe
import me.zhanghai.android.files.util.getQuantityString
import me.zhanghai.android.files.util.putArgs
import me.zhanghai.android.files.util.showToast
import me.zhanghai.android.files.util.startActivitySafe
import me.zhanghai.android.files.util.checkSelfPermission
import me.zhanghai.android.files.util.supportsExternalStorageManager
import me.zhanghai.android.files.util.valueCompat
import me.zhanghai.android.files.util.viewModels
import me.zhanghai.android.files.util.withChooser
import me.zhanghai.android.files.viewer.image.ImageViewerActivity
import me.zhanghai.android.files.viewer.text.TextEditorActivity
import org.eds.zipxtract.core.ArchiveCreateOptions
import org.eds.zipxtract.core.ArchiveEditPolicy
import org.eds.zipxtract.core.ArchiveFormat
import org.eds.zipxtract.core.PrivateArchiveTempStore
import org.eds.zipxtract.core.ZipXtractArchiveEngine
import me.zhanghai.android.files.provider.archive.zipxtract.PathArchiveSource
import me.zhanghai.android.files.provider.common.isDirectory
import me.zhanghai.android.files.provider.common.isEncrypted
import me.zhanghai.android.files.util.configureSecurePasswordInput
import me.zhanghai.android.files.util.hideTextInputLayoutErrorOnTextChange
import me.zhanghai.android.files.util.layoutInflater
import kotlin.math.roundToInt

/**
 * Native RongVualt file browser shell.  The legacy FileListFragment remains
 * available for external pickers, while normal browsing gets two independent
 * Material panes sharing one toolbar, drawer and operation dock.
 */
class DualPaneFileListFragment : Fragment(), NavigationFragment.Listener,
    BreadcrumbLayout.Listener,
    ConfirmReplaceFileDialogFragment.Listener, ConfirmDeleteFilesDialogFragment.Listener,
    CreateArchiveDialogFragment.Listener, RenameFileDialogFragment.Listener,
    CreateFileDialogFragment.Listener, CreateDirectoryDialogFragment.Listener,
    NavigateToPathDialogFragment.Listener,
    ShowRequestAllFilesAccessRationaleDialogFragment.Listener,
    ShowRequestNotificationPermissionRationaleDialogFragment.Listener,
    ShowRequestNotificationPermissionInSettingsRationaleDialogFragment.Listener,
    ShowRequestStoragePermissionRationaleDialogFragment.Listener,
    ShowRequestStoragePermissionInSettingsRationaleDialogFragment.Listener {
    private val args by args<FileListFragment.Args>()
    private val argsPath by lazy { args.intent.extraPath }
    private val shellViewModel by viewModels { { DualPaneFileListViewModel() } }

    private val requestAllFilesAccessLauncher = registerForActivityResult(
        RequestAllFilesAccessContract(), ::onRequestAllFilesAccessResult
    )
    private val requestStoragePermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission(), ::onRequestStoragePermissionResult
    )
    private val requestStoragePermissionInSettingsLauncher = registerForActivityResult(
        RequestPermissionInSettingsContract(android.Manifest.permission.WRITE_EXTERNAL_STORAGE),
        ::onRequestStoragePermissionInSettingsResult,
    )
    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    private val requestNotificationPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission(), ::onRequestNotificationPermissionResult
    )
    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    private val requestNotificationPermissionInSettingsLauncher = registerForActivityResult(
        RequestPermissionInSettingsContract(android.Manifest.permission.POST_NOTIFICATIONS),
        ::onRequestNotificationPermissionInSettingsResult,
    )

    private lateinit var leftViewModel: FileListViewModel
    private lateinit var rightViewModel: FileListViewModel
    private lateinit var root: View
    private lateinit var drawerLayout: DrawerLayout
    private lateinit var paneContainer: LinearLayout
    private lateinit var toolbar: AdaptiveToolbar
    private lateinit var dockContainer: ExpandableActionDockLayout
    private lateinit var actionDock: LinearLayout
    private lateinit var recentAccessPanel: View
    private lateinit var recentAccessList: RecyclerView
    private lateinit var recentAccessEmpty: TextView
    private lateinit var recentAccessStore: RecentAccessStore
    private lateinit var recentAccessAdapter: RecentAccessAdapter
    private lateinit var navigationFragment: NavigationFragment
    private lateinit var leftPane: PaneBinding
    private lateinit var rightPane: PaneBinding
    private lateinit var leftPaneSurface: MaterialCardView
    private lateinit var rightPaneSurface: MaterialCardView
    private lateinit var leftAdapter: FileListAdapter
    private lateinit var rightAdapter: FileListAdapter
    private lateinit var leftLayoutManager: GridLayoutManager
    private lateinit var rightLayoutManager: GridLayoutManager

    private var searchView: SearchView? = null
    private var externalArchiveDialogShown = false
    private var pendingExtractionSources: List<Path>? = null
    private var pendingExtractionEntries: Pair<Path, Set<String>>? = null
    private var pendingExtractionPane: PaneId? = null
    private var archiveAdditionPane: PaneId? = null
    private var lastWindowWidthDp = -1
    private var startupErrorShown = false
    private var contextMenuHidingDockOperations = false
    private var paneSwitchButton: MaterialButton? = null
    private var paneSwitchTapCount = 0
    private var paneSwitchLastTapAt = 0L
    private var archiveProgressDialog: Dialog? = null
    private var archiveProgressDialogId: Int? = null
    private var archiveProgressBar: ProgressBar? = null
    private var archiveProgressDetail: TextView? = null
    private val hiddenArchiveProgressIds = mutableSetOf<Int>()
    private val archiveEditProbePaths = EnumMap<PaneId, Path?>(PaneId::class.java)
    private val archiveEditCapabilities = EnumMap<PaneId, Boolean>(PaneId::class.java)
    private val paneDiagnostics = EnumMap<PaneId, String>(PaneId::class.java)
    // This is deliberately kept separate from FileListViewModel.selectedFiles
    // during cold start. Its LiveData can be observed before its initial value
    // is dispatched on some AndroidX versions.
    private val selectionCounts = EnumMap<PaneId, Int>(PaneId::class.java).apply {
        put(PaneId.LEFT, 0)
        put(PaneId.RIGHT, 0)
    }
    private val debouncedSearchRunnable = DebouncedRunnable(
        android.os.Handler(android.os.Looper.getMainLooper()), 500
    ) {
        val query = searchView?.query?.toString() ?: return@DebouncedRunnable
        if (query.isNotEmpty()) model(shellViewModel.activePane).search(query)
    }
    private val fileJobResultListener: (FileJobResult) -> Unit = { result ->
        if (this::root.isInitialized) {
            root.post { onFileJobResult(result) }
        }
    }
    private val archiveProgressListener: (List<FileJobProgress>) -> Unit = { progress ->
        if (this::root.isInitialized) root.post { renderArchiveProgress(progress) }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setHasOptionsMenu(true)
        recentAccessStore = RecentAccessStore(requireContext().applicationContext)
        val provider = ViewModelProvider(this)
        leftViewModel = provider.get("rongvault-left-pane", FileListViewModel::class.java)
        rightViewModel = provider.get("rongvault-right-pane", FileListViewModel::class.java)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        root = inflater.inflate(R.layout.dual_file_list_fragment, container, false)
        drawerLayout = root.findViewById(R.id.dualDrawerLayout)
        paneContainer = root.findViewById(R.id.dualPaneContainer)
        toolbar = root.findViewById(R.id.dualToolbar)
        dockContainer = root.findViewById(R.id.dualDockContainer)
        actionDock = root.findViewById(R.id.dualActionDock)
        recentAccessPanel = root.findViewById(R.id.dualRecentAccessPanel)
        recentAccessList = root.findViewById(R.id.dualRecentAccessList)
        recentAccessEmpty = root.findViewById(R.id.dualRecentAccessEmpty)
        leftPane = PaneBinding(root.findViewById(R.id.leftPane))
        rightPane = PaneBinding(root.findViewById(R.id.rightPane))
        leftPaneSurface = root.findViewById(R.id.leftPaneSurface)
        rightPaneSurface = root.findViewById(R.id.rightPaneSurface)
        root.addOnLayoutChangeListener { _, left, _, right, _, oldLeft, _, oldRight, _ ->
            val width = right - left
            val oldWidth = oldRight - oldLeft
            if (width != oldWidth) {
                val widthDp = (width / resources.displayMetrics.density).roundToInt()
                if (widthDp != lastWindowWidthDp) {
                    lastWindowWidthDp = widthDp
                    updateLayoutMode()
                }
            }
        }
        return root
    }

    override fun onActivityCreated(savedInstanceState: Bundle?) {
        super.onActivityCreated(savedInstanceState)
        try {
            initializeActivity(savedInstanceState)
        } catch (exception: Exception) {
            showRecoverableStartupError(exception)
        }
    }

    /** Keep a browser startup fault isolated from RongVualt's container UI. */
    private fun initializeActivity(savedInstanceState: Bundle?) {
        val activity = requireActivity() as AppCompatActivity
        activity.setTitle(R.string.file_list_title)
        activity.setSupportActionBar(toolbar)
        toolbar.logo = null
        toolbar.clearBuiltInText()
        toolbar.setBreadcrumbListener(ActivePaneBreadcrumbListener())
        if (savedInstanceState == null) {
            navigationFragment = NavigationFragment()
            childFragmentManager.commit {
                add(R.id.dualNavigationFragment, navigationFragment)
            }
        } else {
            navigationFragment = childFragmentManager.findFragmentById(R.id.dualNavigationFragment)
                as NavigationFragment
        }
        navigationFragment.listener = this
        toolbar.setNavigationOnClickListener { onToolbarNavigationClick() }
        ViewCompat.setOnApplyWindowInsetsListener(dockContainer) { view, insets ->
            val bottom = insets.getInsets(WindowInsetsCompat.Type.systemBars()).bottom
            view.updatePadding(bottom = 4.dp() + bottom)
            insets
        }
        if (!shellViewModelInitialized()) {
            initializePaths()
        }
        // FileListActivity only creates this fragment for the normal browser;
        // ACTION_OPEN_DOCUMENT and other picker contracts retain FileListFragment.
        // Restoring here keeps the choice across new browser Activities while the
        // ViewModel still preserves it across configuration changes.
        shellViewModel.layoutMode = FileListLayoutMode.DUAL
        setupPane(PaneId.LEFT, leftPane, leftViewModel, PaneAdapterListener(PaneId.LEFT))
        setupPane(PaneId.RIGHT, rightPane, rightViewModel, PaneAdapterListener(PaneId.RIGHT))
        setupRecentAccessDock()
        addOnBackPressedCallback(DrawerLayoutOnBackPressedCallback(drawerLayout))
        addOnBackPressedCallback(
            object : OnBackPressedCallback(true) {
                override fun handleOnBackPressed() {
                    val pane = shellViewModel.activePane
                    val current = model(pane).currentPath
                    if (shellViewModel.isRecentAccessExpanded) {
                        setRecentAccessExpanded(false)
                    } else if (hasSelection(pane)) {
                        model(pane).clearSelectedFiles()
                    } else if (!goBack(pane, current)) {
                        isEnabled = false
                        requireActivity().onBackPressedDispatcher.onBackPressed()
                        isEnabled = true
                    }
                }
            }
        )
        updateLayoutMode()
        renderActionDock()
        observeSharedSettings()
        requireActivity().invalidateOptionsMenu()
    }

    private fun showRecoverableStartupError(exception: Exception) {
        if (startupErrorShown || !isAdded) return
        startupErrorShown = true
        Log.e("RongVualtFileManager", "Unable to initialize file manager", exception)
        val detail = exception.localizedMessage?.takeIf { it.isNotBlank() }
            ?: exception.javaClass.simpleName
        MaterialAlertDialogBuilder(requireContext())
            .setTitle("文件管理器无法打开")
            .setMessage("发生错误：$detail\n\n已安全返回，不会影响容器和已解锁卷。")
            .setPositiveButton(android.R.string.ok) { _, _ -> requireActivity().finish() }
            .setOnDismissListener { requireActivity().finish() }
            .show()
    }

    override fun onResume() {
        super.onResume()
        if (!leftViewModel.isStorageAccessRequested) ensureStorageAccess()
        if (!leftViewModel.isNotificationPermissionRequested) ensureNotificationPermission()
    }

    override fun onStart() {
        super.onStart()
        FileJobService.addResultListener(fileJobResultListener)
        FileJobProgressRegistry.addListener(archiveProgressListener)
    }

    override fun onStop() {
        FileJobService.removeResultListener(fileJobResultListener)
        FileJobProgressRegistry.removeListener(archiveProgressListener)
        archiveProgressDialog?.dismiss()
        archiveProgressDialog = null
        archiveProgressDialogId = null
        super.onStop()
    }

    override fun onDestroyView() {
        debouncedSearchRunnable.cancel()
        searchView = null
        super.onDestroyView()
    }

    private fun shellViewModelInitialized(): Boolean =
        leftViewModel.hasTrail && rightViewModel.hasTrail

    private fun initializePaths() {
        var primary = argsPath
        val intent = args.intent
        if (primary == null) {
            @Suppress("DEPRECATION")
            primary = Paths.get(Environment.getExternalStorageDirectory().absolutePath)
        } else {
            val mimeType = intent.type?.asMimeTypeOrNull()
            if (mimeType != null && primary.isArchiveFile(mimeType)) {
                primary = primary.createArchiveRootPath()
            }
        }
        leftViewModel.resetTo(primary)
        val configured = Settings.FILE_LIST_SECONDARY_START_DIRECTORY.valueCompat
        val secondary = resolveSecondaryPath(configured)
        rightViewModel.resetTo(secondary)
    }

    private fun resolveSecondaryPath(configured: Path): Path {
        // Do not probe remote providers here: a startup preference must not
        // implicitly establish a network session. A document path is safe to
        // probe because it represents an already-granted provider tree; a
        // locked tree simply falls through to the local candidates.
        fun accessible(path: Path): Boolean {
            if (!path.isLinuxPath && !path.isDocumentPath) return false
            return runCatching { path.isDirectory() }.getOrDefault(false)
        }
        if (accessible(configured)) return configured
        @Suppress("DEPRECATION")
        val shared = Paths.get(Environment.getExternalStorageDirectory().absolutePath)
        val fallback = Settings.FILE_LIST_DEFAULT_DIRECTORY.valueCompat
        val firstRoot = runCatching {
            java.io.File.listRoots().asSequence()
                .map { Paths.get(it.absolutePath) }
                .firstOrNull(::accessible)
        }.getOrNull()
        val resolved = listOf(shared, fallback, firstRoot).filterNotNull().firstOrNull(::accessible)
            ?: shared
        if (configured != resolved && isAdded) {
            showToast(R.string.file_list_action_secondary_path_fallback)
        }
        return resolved
    }

    private fun setupPane(
        pane: PaneId,
        binding: PaneBinding,
        viewModel: FileListViewModel,
        listener: FileListAdapter.Listener
    ) {
        val context = requireContext()
        val layoutManager = GridLayoutManager(context, 1)
        val adapter = FileListAdapter(listener)
        adapter.fontSize = FileListFontSize.fromSp(Settings.FILE_LIST_FONT_SIZE.valueCompat)
        binding.root.setOnClickListener { activatePane(pane) }
        binding.parentRow.setOnClickListener {
            model(pane).currentPathLiveData.value?.let { parentPath(it) }?.let { navigateTo(pane, it) }
        }
        binding.header.contentDescription = getString(
            if (pane == PaneId.LEFT) R.string.file_list_pane_left else R.string.file_list_pane_right
        )
        binding.recyclerView.layoutManager = layoutManager
        binding.recyclerView.adapter = adapter
        // FileListAdapter already owns the row animations. RecyclerView's
        // predictive item animator can otherwise try to reattach a holder
        // that is still participating in the previous layout when a file job
        // and the directory observer refresh this pane at nearly the same
        // time (for example immediately after creating an archive).
        binding.recyclerView.itemAnimator = null
        adapter.attachSwipeSelection(binding.recyclerView)
        binding.recyclerView.setHasFixedSize(true)
        binding.recyclerView.isVerticalScrollBarEnabled = !isDualPaneVisible()
        binding.recyclerView.isScrollbarFadingEnabled = true
        binding.swipeRefreshLayout.setOnRefreshListener { model(pane).reload() }
        binding.errorText.setOnClickListener { model(pane).reload() }
        binding.errorText.setOnLongClickListener {
            paneDiagnostics[pane]?.let { diagnostic ->
                clipboardManager.copyText(diagnostic, requireContext())
                showToast("已复制诊断信息")
            }
            true
        }
        binding.breadcrumbLayout.setListener(PaneBreadcrumbListener(pane))
        if (pane == PaneId.LEFT) {
            leftAdapter = adapter
            leftLayoutManager = layoutManager
        } else {
            rightAdapter = adapter
            rightLayoutManager = layoutManager
        }
        viewModel.currentPathLiveData.observe(viewLifecycleOwner) { onPanePathChanged(pane, it) }
        viewModel.breadcrumbLiveData.observe(viewLifecycleOwner) { breadcrumb ->
            binding.breadcrumbLayout.setData(breadcrumb)
            if (shellViewModel.activePane == pane) toolbar.setBreadcrumbData(breadcrumb)
        }
        viewModel.viewTypeLiveData.observe(viewLifecycleOwner) { onPaneViewTypeChanged(pane, it) }
        viewModel.sortOptionsLiveData.observe(viewLifecycleOwner) { onPaneSortChanged(pane, it) }
        viewModel.viewSortPathSpecificLiveData.observe(viewLifecycleOwner) { invalidateOptionsMenu() }
        viewModel.selectedFilesLiveData.observe(viewLifecycleOwner) {
            selectionCounts[pane] = it.size
            adapter.replaceSelectedFiles(it)
            updatePaneHeader(pane)
            renderActionDock()
            invalidateOptionsMenu()
        }
        viewModel.searchStateLiveData.observe(viewLifecycleOwner) { updatePaneHeader(pane) }
        viewModel.fileListLiveData.observe(viewLifecycleOwner) { onPaneFileListChanged(pane, it) }
        Settings.FILE_LIST_FONT_SIZE.observe(viewLifecycleOwner) {
            adapter.fontSize = FileListFontSize.fromSp(it)
        }
        Settings.FILE_NAME_ELLIPSIZE.observe(viewLifecycleOwner) {
            adapter.nameEllipsize = it
        }
        Settings.FILE_NAME_MAX_LINES.observe(viewLifecycleOwner) {
            adapter.nameMaxLines = it
        }
        if (viewModel.hasTrail) updatePaneHeader(pane)
    }

    private fun observeSharedSettings() {
        Settings.FILE_LIST_SHOW_HIDDEN_FILES.observe(viewLifecycleOwner) {
            updateAdapterFileList(PaneId.LEFT)
            updateAdapterFileList(PaneId.RIGHT)
        }
    }

    private fun binding(pane: PaneId): PaneBinding = if (pane == PaneId.LEFT) leftPane else rightPane

    private fun model(pane: PaneId): FileListViewModel =
        if (pane == PaneId.LEFT) leftViewModel else rightViewModel

    private fun adapter(pane: PaneId): FileListAdapter =
        if (pane == PaneId.LEFT) leftAdapter else rightAdapter

    private fun layoutManager(pane: PaneId): GridLayoutManager =
        if (pane == PaneId.LEFT) leftLayoutManager else rightLayoutManager

    private fun onPanePathChanged(pane: PaneId, path: Path) {
        val binding = binding(pane)
        binding.path.text = path.toUserFriendlyString()
        updateParentRow(pane)
        updatePaneHeader(pane)
        updateArchiveEditCapability(pane)
        if (pane == shellViewModel.activePane) updateToolbarNavigation()
        invalidateOptionsMenu()
    }

    private fun updateToolbarNavigation() {
        if (!this::toolbar.isInitialized) return
        val inArchive = runCatching { model(shellViewModel.activePane).currentPath.isArchivePath }
            .getOrDefault(false)
        toolbar.setNavigationIcon(
            if (inArchive) R.drawable.dual_arrow_back_24dp
            else R.drawable.menu_icon_control_normal_24dp
        )
        toolbar.navigationContentDescription = getString(
            if (inArchive) R.string.file_list_action_back else R.string.open_navigation_drawer
        )
    }

    private fun onToolbarNavigationClick() {
        val pane = shellViewModel.activePane
        val current = model(pane).currentPath
        if (!current.isArchivePath) {
            drawerLayout.openDrawer(GravityCompat.START)
            return
        }
        val target = current.parent ?: runCatching { current.archiveFile.parent }.getOrNull()
        if (target != null) navigateTo(pane, target)
    }

    private fun updateParentRow(pane: PaneId) {
        val binding = binding(pane)
        val parent = model(pane).currentPathLiveData.value?.let(::parentPath)
        binding.parentRow.isVisible = parent != null
        binding.parentText.text = ".."
        binding.parentRow.contentDescription = getString(R.string.file_list_parent_directory)
    }

    private fun parentPath(path: Path): Path? = path.parent ?: if (path.isArchivePath) {
        runCatching { path.archiveFile.parent }.getOrNull()
    } else null

    private fun updatePaneHeader(pane: PaneId) {
        if (!this::leftPane.isInitialized) return
        val binding = binding(pane)
        val viewModel = model(pane)
        val active = shellViewModel.activePane == pane
        binding.header.setBackgroundColor(
            MaterialColors.getColor(
                binding.header,
                if (active) com.google.android.material.R.attr.colorSurfaceContainerHigh
                else com.google.android.material.R.attr.colorSurfaceContainerLow,
            )
        )
        binding.label.setText(if (pane == PaneId.LEFT) R.string.file_list_pane_left else R.string.file_list_pane_right)
        binding.currentBadge.isVisible = active
        val selected = selectionCount(pane)
        binding.selectedCount.text = if (selected == 0) "" else selected.toString()
        // FileListSwitchMapLiveData does not publish its first loading value
        // synchronously with resetTo().  The pane header is set up immediately
        // afterwards, so use the nullable LiveData value here rather than the
        // ViewModel's non-null convenience accessor.
        val fileListStateful = viewModel.fileListLiveData.value
        val files = fileListStateful?.value
        if (files != null) {
            binding.summary.text = getSubtitle(files)
        } else if (fileListStateful is Loading<*>) {
            binding.summary.setText(R.string.loading)
        } else {
            binding.summary.text = ""
        }
        if (shellViewModel.activePane == pane) updateToolbarState()
    }

    private fun updateToolbarState() {
        if (!this::toolbar.isInitialized || !this::leftViewModel.isInitialized) return
        val viewModel = model(shellViewModel.activePane)
        // The selected-files observer is allowed to run before the first path
        // value is dispatched.  The non-null ViewModel convenience accessor is
        // not valid during that cold-start interval.
        val currentPath = viewModel.currentPathLiveData.value ?: return
        toolbar.clearBuiltInText()
        updateToolbarNavigation()
        viewModel.breadcrumbLiveData.value?.let(toolbar::setBreadcrumbData)
        val files = viewModel.fileListLiveData.value?.value
        val directories = files?.count { it.attributes.isDirectory } ?: 0
        val regular = files?.count { !it.attributes.isDirectory } ?: 0
        val storageRoot = Environment.getExternalStorageDirectory().absolutePath
        val total = JavaFile.getTotalSpace(storageRoot)
        val free = JavaFile.getFreeSpace(storageRoot)
        val used = (total - free).coerceAtLeast(0)
        val usedText = if (total > 0) used.asFileSize().formatHumanReadable(requireContext()) else "-"
        val totalText = if (total > 0) total.asFileSize().formatHumanReadable(requireContext()) else "-"
        toolbar.setStorageSummary(getString(
            R.string.file_list_toolbar_summary_format,
            directories, regular, usedText, totalText,
        ))
    }

    private fun invalidateOptionsMenu() {
        requireActivity().invalidateOptionsMenu()
    }

    /**
     * The two FileListViewModels are configured before their observers are
     * attached.  On a cold launch LiveData can still be null for that small
     * interval, so UI-only selection state must not use valueCompat.
     */
    private fun selectionCount(pane: PaneId): Int = selectionCounts[pane] ?: 0

    private fun hasSelection(pane: PaneId): Boolean = selectionCount(pane) > 0

    private fun onPaneFileListChanged(
        pane: PaneId,
        stateful: me.zhanghai.android.files.util.Stateful<List<FileItem>>
    ) {
        val binding = binding(pane)
        val files = stateful.value
        val searching = model(pane).searchState.isSearching
        val hasFiles = !files.isNullOrEmpty()
        binding.swipeRefreshLayout.isRefreshing = stateful is Loading<*> && (hasFiles || searching)
        binding.progress.fadeToVisibilityUnsafe(stateful is Loading<*> && !(hasFiles || searching))
        binding.errorText.fadeToVisibilityUnsafe(stateful is Failure<*> && !hasFiles)
        binding.emptyView.fadeToVisibilityUnsafe(stateful is Success<*> && !hasFiles)
        val throwable = (stateful as? Failure)?.throwable
        if (throwable != null) {
            showPaneError(pane, throwable, hasFiles)
        } else if (files != null) {
            paneDiagnostics.remove(pane)
        }
        if (files != null) {
            model(pane).selectionRangeAnchorPath = null
            updateAdapterFileList(pane)
        } else adapter(pane).clear()
        updatePaneHeader(pane)
        if (stateful.value != null) {
            maybeRestorePaneScroll(pane)
        }
    }

    private fun onFileJobResult(result: FileJobResult) {
        if (!isAdded || result.cancelled) return
        if (result.isSuccess) {
            model(PaneId.LEFT).reload()
            model(PaneId.RIGHT).reload()
        } else {
            showPaneError(shellViewModel.activePane, result.error ?: return, hasFiles = true)
        }
    }

    private fun showPaneError(pane: PaneId, throwable: Throwable, hasFiles: Boolean) {
        val diagnostic = buildSafeDiagnostic(throwable)
        paneDiagnostics[pane] = diagnostic
        val message = "${userFacingError(throwable)}\n轻触重试，长按复制诊断信息。"
        if (hasFiles) {
            showToast(userFacingError(throwable))
        } else {
            binding(pane).errorText.text = message
        }
    }

    private fun userFacingError(throwable: Throwable): String = when (throwable) {
        is SecurityException -> "没有访问该位置的权限。"
        is java.io.FileNotFoundException -> "文件或位置已不可用。"
        is java.io.IOException -> "文件操作未完成：${throwable.message.orEmpty().take(120)}"
        else -> "操作未完成：${throwable.message.orEmpty().take(120).ifBlank { "发生未知错误" }}"
    }

    private fun buildSafeDiagnostic(throwable: Throwable): String {
        val raw = "${throwable.javaClass.simpleName}: ${throwable.message.orEmpty()}"
        return raw
            .replace(Regex("(?i)(password|passphrase|secret|key)\\s*[=:]\\s*[^,\\s]+"), "$1=<redacted>")
            .replace(Regex("(?:content|file)://\\S+|/(?:[^\\s/]+/)+[^\\s]*"), "<path>")
            .take(500)
    }

    private fun maybeRestorePaneScroll(pane: PaneId) {
        val state = model(pane).pendingState ?: return
        layoutManager(pane).onRestoreInstanceState(state)
    }

    private fun updateAdapterFileList(pane: PaneId) {
        val files = model(pane).fileListStateful.value ?: return
        val visible = if (Settings.FILE_LIST_SHOW_HIDDEN_FILES.valueCompat) files
        else files.filterNot { it.isHidden }
        adapter(pane).replaceListAndIsSearching(visible, model(pane).searchState.isSearching)
    }

    private fun getSubtitle(files: List<FileItem>): String {
        val directories = files.count { it.attributes.isDirectory }
        val regular = files.size - directories
        val directoryText = if (directories > 0) getQuantityString(
            R.plurals.file_list_subtitle_directory_count_format, directories, directories
        ) else null
        val fileText = if (regular > 0) getQuantityString(
            R.plurals.file_list_subtitle_file_count_format, regular, regular
        ) else null
        return listOfNotNull(directoryText, fileText).joinToString(getString(R.string.file_list_subtitle_separator))
            .ifEmpty { getString(R.string.empty) }
    }

    private fun onPaneViewTypeChanged(pane: PaneId, viewType: FileViewType) {
        if (viewType != FileViewType.LIST) model(pane).viewType = FileViewType.LIST
        adapter(pane).viewType = FileViewType.LIST
        updateSpanCount(pane)
        invalidateOptionsMenu()
    }

    private fun onPaneSortChanged(pane: PaneId, options: FileSortOptions) {
        adapter(pane).sortOptions = options
        invalidateOptionsMenu()
    }

    private fun updateSpanCount(pane: PaneId) {
        // The root view can receive its first layout pass before onActivityCreated()
        // has installed both pane RecyclerViews. Do not turn that harmless early
        // callback into a process-wide crash.
        if (pane == PaneId.LEFT && !this::leftLayoutManager.isInitialized) return
        if (pane == PaneId.RIGHT && !this::rightLayoutManager.isInitialized) return
        val viewType = model(pane).viewTypeLiveData.value ?: return
        val widthDp = effectiveWindowWidthDp() /
            if (isDualPaneVisible()) 2 else 1
        layoutManager(pane).spanCount = when (viewType) {
            FileViewType.LIST -> 1
            FileViewType.GRID -> (widthDp / 180).coerceAtLeast(2)
        }
    }

    private fun isDualPaneVisible(): Boolean =
        shellViewModel.layoutMode.isDualPane(effectiveWindowWidthDp())

    private fun effectiveWindowWidthDp(): Int = if (root.width > 0) {
        (root.width / resources.displayMetrics.density).roundToInt()
    } else {
        resources.configuration.screenWidthDp
    }

    private fun updateLayoutMode() {
        if (shellViewModel.layoutMode != FileListLayoutMode.DUAL) {
            shellViewModel.layoutMode = FileListLayoutMode.DUAL
        }
        val dual = true
        // In single-pane mode keep the inactive controller mounted but hide
        // its view. This preserves its path, history, selection and scroll
        // state while making the bottom “switch” action feel like a true
        // pane toggle.
        leftPaneSurface.isVisible = dual || shellViewModel.activePane == PaneId.LEFT
        rightPaneSurface.isVisible = dual || shellViewModel.activePane == PaneId.RIGHT
        root.findViewById<View>(R.id.dualPaneDivider).isVisible = dual
        (leftPaneSurface.layoutParams as? LinearLayout.LayoutParams)?.apply {
            width = 0
            weight = 1f
        }
        (rightPaneSurface.layoutParams as? LinearLayout.LayoutParams)?.apply {
            width = 0
            weight = 1f
        }
        leftPane.recyclerView.isVerticalScrollBarEnabled = !dual
        rightPane.recyclerView.isVerticalScrollBarEnabled = !dual
        paneContainer.requestLayout()
        updateActiveDivider()
        updateActivePaneVisual(animate = false)
        updateSpanCount(PaneId.LEFT)
        updateSpanCount(PaneId.RIGHT)
        renderActionDock()
    }

    private fun setLayoutMode(mode: FileListLayoutMode) {
        shellViewModel.layoutMode = FileListLayoutMode.DUAL
        Settings.FILE_LIST_LAYOUT_MODE.putValue(FileListLayoutMode.DUAL)
        updateLayoutMode()
    }

    private fun activatePane(pane: PaneId) {
        if (shellViewModel.activePane == pane) return
        shellViewModel.activePane = pane
        updateActiveDivider()
        updateActivePaneVisual(animate = true)
        if (!isDualPaneVisible()) updateLayoutMode()
        updatePaneHeader(PaneId.LEFT)
        updatePaneHeader(PaneId.RIGHT)
        updateToolbarState()
        if (searchView?.isShown == true) searchView?.setQuery(model(pane).searchViewQuery, false)
        if (this::navigationFragment.isInitialized) navigationFragment.refreshCheckedState()
        renderActionDock()
        invalidateOptionsMenu()
    }

    private fun updateActiveDivider() {
        if (!this::root.isInitialized) return
        val divider = root.findViewById<View>(R.id.dualPaneDivider)
        if (!isDualPaneVisible()) return
        divider.layoutParams = divider.layoutParams.apply { width = 1.dp() }
        divider.setBackgroundColor(
            MaterialColors.getColor(divider, com.google.android.material.R.attr.colorOutlineVariant)
        )
        ViewCompat.setElevation(divider, 0f)
    }

    /** A zero-radius surface gives the active pane an even top, bottom and side shadow. */
    private fun updateActivePaneVisual(animate: Boolean) {
        if (!this::leftPane.isInitialized || !this::rightPane.isInitialized) return
        updateDualSurfaceColors()
        val dual = isDualPaneVisible()
        val canAnimate = animate && Settings.FILE_LIST_ANIMATION.valueCompat &&
            (Build.VERSION.SDK_INT < Build.VERSION_CODES.O || ValueAnimator.areAnimatorsEnabled()) &&
            paneContainer.isLaidOut
        listOf(PaneId.LEFT to leftPaneSurface, PaneId.RIGHT to rightPaneSurface).forEach { (pane, view) ->
            val active = dual && shellViewModel.activePane == pane
            val lift = if (active) 4.dp().toFloat() else 0f
            view.animate().cancel()
            if (canAnimate) {
                view.animate()
                    .translationZ(lift)
                    .setDuration(180L)
                    .setInterpolator(FastOutSlowInInterpolator())
                    .start()
            } else {
                view.translationZ = lift
            }
        }
    }

    /** Keep every browser surface on the selected theme's same base surface. */
    private fun updateDualSurfaceColors() {
        val surface = MaterialColors.getColor(
            root, com.google.android.material.R.attr.colorSurface
        )
        root.findViewById<View>(R.id.dualContentRoot).setBackgroundColor(surface)
        dockContainer.setBackgroundColor(surface)
        leftPaneSurface.setCardBackgroundColor(surface)
        rightPaneSurface.setCardBackgroundColor(surface)
        leftPane.root.setBackgroundColor(surface)
        rightPane.root.setBackgroundColor(surface)
    }

    private fun navigateTo(pane: PaneId, path: Path, recordHistory: Boolean = true) {
        if (recordHistory) shellViewModel.recordNavigation(pane, model(pane).currentPath)
        model(pane).navigateTo(layoutManager(pane).onSaveInstanceState()!!, path)
        recordRecentAccess(path, isDirectory = true)
        activatePane(pane)
    }

    private fun goBack(pane: PaneId, current: Path): Boolean {
        val target = shellViewModel.goBack(pane, current) ?: return false
        navigateTo(pane, target, false)
        return true
    }

    private fun goForward(pane: PaneId, current: Path): Boolean {
        val target = shellViewModel.goForward(pane, current) ?: return false
        navigateTo(pane, target, false)
        return true
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        super.onCreateOptionsMenu(menu, inflater)
        inflater.inflate(R.menu.file_list, menu)
        // Keep the title/subtitle area visually close to the reference file
        // manager: search and sorting are available from the overflow menu.
        menu.findItem(R.id.action_search).setShowAsAction(
            MenuItem.SHOW_AS_ACTION_NEVER or MenuItem.SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW
        )
        menu.findItem(R.id.action_view_sort).setShowAsAction(MenuItem.SHOW_AS_ACTION_NEVER)
        setupSearchView(menu.findItem(R.id.action_search))
    }

    private fun setupSearchView(searchItem: MenuItem) {
        val view = searchItem.actionView as? FixQueryChangeSearchView ?: return
        searchView = view
        view.setOnSearchClickListener {
            val model = model(shellViewModel.activePane)
            model.isSearchViewExpanded = true
            view.setQuery(model.searchViewQuery, false)
        }
        searchItem.setOnActionExpandListener(object : MenuItem.OnActionExpandListener {
            override fun onMenuItemActionExpand(item: MenuItem): Boolean = true

            override fun onMenuItemActionCollapse(item: MenuItem): Boolean {
                model(shellViewModel.activePane).isSearchViewExpanded = false
                model(shellViewModel.activePane).stopSearching()
                return true
            }
        })
        view.setOnQueryTextListener(object : SearchView.OnQueryTextListener {
            override fun onQueryTextSubmit(query: String): Boolean {
                debouncedSearchRunnable.cancel()
                if (query.isNotEmpty()) model(shellViewModel.activePane).search(query)
                return true
            }

            override fun onQueryTextChange(query: String): Boolean {
                if (view is FixQueryChangeSearchView && view.shouldIgnoreQueryChange) return false
                model(shellViewModel.activePane).searchViewQuery = query
                if (query.isEmpty()) model(shellViewModel.activePane).stopSearching()
                else debouncedSearchRunnable()
                return true
            }
        })
    }

    override fun onPrepareOptionsMenu(menu: Menu) {
        super.onPrepareOptionsMenu(menu)
        val pane = shellViewModel.activePane
        val viewModel = model(pane)
        val searching = viewModel.isSearchViewExpanded
        menu.findItem(R.id.action_view_sort)?.isVisible = !searching
        listOf(
            R.id.action_create_shortcut,
            R.id.action_navigate_up,
            R.id.action_add_bookmark,
            R.id.action_copy_path,
            R.id.action_share,
            R.id.action_select_all,
        ).forEach { menu.findItem(it)?.isVisible = false }
        menu.findItem(R.id.action_layout_mode)?.isVisible = false
        menu.findItem(R.id.action_view_list)?.isVisible = false
        menu.findItem(R.id.action_view_grid)?.isVisible = false
        menu.findItem(R.id.action_layout_auto)?.isVisible = false
        menu.findItem(R.id.action_layout_single)?.isVisible = false
        menu.findItem(R.id.action_layout_dual)?.isVisible = false
        menu.findItem(R.id.action_set_secondary_start)?.isVisible =
            pane == PaneId.RIGHT && !viewModel.currentPath.isArchivePath
        menu.findItem(R.id.action_selection_more)?.isVisible = hasSelection(pane)
        menu.findItem(R.id.action_layout_auto)?.isChecked = shellViewModel.layoutMode == FileListLayoutMode.AUTO
        menu.findItem(R.id.action_layout_single)?.isChecked = shellViewModel.layoutMode == FileListLayoutMode.SINGLE
        menu.findItem(R.id.action_layout_dual)?.isChecked = shellViewModel.layoutMode == FileListLayoutMode.DUAL
        menu.findItem(R.id.action_view_list)?.isChecked = viewModel.viewType == FileViewType.LIST
        menu.findItem(R.id.action_view_grid)?.isChecked = viewModel.viewType == FileViewType.GRID
        val options = viewModel.sortOptions
        menu.findItem(R.id.action_sort_by_name)?.isChecked = options.by == By.NAME
        menu.findItem(R.id.action_sort_by_type)?.isChecked = options.by == By.TYPE
        menu.findItem(R.id.action_sort_by_size)?.isChecked = options.by == By.SIZE
        menu.findItem(R.id.action_sort_by_last_modified)?.isChecked = options.by == By.LAST_MODIFIED
        menu.findItem(R.id.action_sort_order_ascending)?.isChecked = options.order == Order.ASCENDING
        menu.findItem(R.id.action_sort_directories_first)?.isChecked = options.isDirectoriesFirst
        menu.findItem(R.id.action_view_sort_path_specific)?.isChecked = viewModel.isViewSortPathSpecific
        menu.findItem(R.id.action_show_hidden_files)?.isChecked = Settings.FILE_LIST_SHOW_HIDDEN_FILES.valueCompat
        val canAdd = currentSevenZArchiveRoot(pane) != null && archiveEditCapabilities[pane] == true
        menu.findItem(R.id.action_archive_add)?.isVisible = canAdd
        menu.findItem(R.id.action_archive_add_folder)?.isVisible = canAdd
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        val pane = shellViewModel.activePane
        val viewModel = model(pane)
        return when (item.itemId) {
            R.id.action_layout_auto -> {
                setLayoutMode(FileListLayoutMode.AUTO)
                invalidateOptionsMenu()
                true
            }
            R.id.action_layout_single -> {
                setLayoutMode(FileListLayoutMode.SINGLE)
                true
            }
            R.id.action_layout_dual -> {
                setLayoutMode(FileListLayoutMode.DUAL)
                true
            }
            R.id.action_set_secondary_start -> {
                Settings.FILE_LIST_SECONDARY_START_DIRECTORY.putValue(viewModel.currentPath)
                showToast(R.string.file_list_action_fix_secondary_success)
                true
            }
            R.id.action_selection_more -> {
                showSelectionActionPanel(pane, viewModel.selectedFiles)
                true
            }
            R.id.action_view_list -> {
                viewModel.viewType = FileViewType.LIST
                true
            }
            R.id.action_view_grid -> {
                viewModel.viewType = FileViewType.GRID
                true
            }
            R.id.action_sort_by_name -> {
                viewModel.setSortBy(By.NAME)
                true
            }
            R.id.action_sort_by_type -> {
                viewModel.setSortBy(By.TYPE)
                true
            }
            R.id.action_sort_by_size -> {
                viewModel.setSortBy(By.SIZE)
                true
            }
            R.id.action_sort_by_last_modified -> {
                viewModel.setSortBy(By.LAST_MODIFIED)
                true
            }
            R.id.action_sort_order_ascending -> {
                viewModel.setSortOrder(if (!item.isChecked) Order.ASCENDING else Order.DESCENDING)
                true
            }
            R.id.action_sort_directories_first -> {
                viewModel.setSortDirectoriesFirst(!item.isChecked)
                true
            }
            R.id.action_view_sort_path_specific -> {
                viewModel.isViewSortPathSpecific = !item.isChecked
                true
            }
            R.id.action_navigate_up -> {
                navigateUp(pane)
                true
            }
            R.id.action_navigate_to -> {
                NavigateToPathDialogFragment.show(viewModel.currentPath, this)
                true
            }
            R.id.action_refresh -> {
                viewModel.reload()
                true
            }
            R.id.action_select_all -> {
                adapter(pane).selectAllFiles()
                true
            }
            R.id.action_show_hidden_files -> {
                Settings.FILE_LIST_SHOW_HIDDEN_FILES.putValue(!item.isChecked)
                true
            }
            R.id.action_share -> {
                sharePath(viewModel.currentPath, MimeType.DIRECTORY)
                true
            }
            R.id.action_copy_path -> {
                copyPath(viewModel.currentPath)
                true
            }
            R.id.action_open_in_terminal -> {
                if (viewModel.currentPath.isLinuxPath) {
                    Terminal.open(viewModel.currentPath.toFile().path, requireContext())
                }
                true
            }
            R.id.action_add_bookmark -> {
                addBookmark(viewModel.currentPath)
                true
            }
            R.id.action_create_shortcut -> {
                createShortcut(viewModel.currentPath, MimeType.DIRECTORY)
                true
            }
            R.id.action_archive_add -> {
                chooseArchiveAddition(pane)
                true
            }
            R.id.action_archive_add_folder -> {
                chooseArchiveAdditionFolder(pane)
                true
            }
            android.R.id.home -> {
                drawerLayout.openDrawer(GravityCompat.START)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    private fun navigateUp(pane: PaneId) {
        val current = model(pane).currentPath
        parentPath(current)?.let { navigateTo(pane, it) }
    }

    private fun collapseSearchView() {
        val item = toolbar.menu.findItem(R.id.action_search)
        if (item?.isActionViewExpanded == true) item.collapseActionView()
    }

    fun onKeyShortcut(keyCode: Int, event: KeyEvent): Boolean {
        val menu = toolbar.menu
        menu.setQwertyMode(KeyCharacterMap.load(event.deviceId).keyboardType != KeyCharacterMap.NUMERIC)
        return menu.performShortcut(keyCode, event, 0)
    }

    override val currentPath: Path
        get() = model(shellViewModel.activePane).currentPath

    override fun navigateTo(path: Path) {
        navigateTo(shellViewModel.activePane, path)
    }

    override fun navigateToRoot(path: Path) {
        navigateTo(shellViewModel.activePane, path)
    }

    override fun navigateToDefaultRoot() {
        navigateTo(shellViewModel.activePane, Settings.FILE_LIST_DEFAULT_DIRECTORY.valueCompat)
    }

    override fun observeCurrentPath(owner: LifecycleOwner, observer: (Path) -> Unit) {
        leftViewModel.currentPathLiveData.observe(owner) { if (shellViewModel.activePane == PaneId.LEFT) observer(it) }
        rightViewModel.currentPathLiveData.observe(owner) { if (shellViewModel.activePane == PaneId.RIGHT) observer(it) }
    }

    override fun closeNavigationDrawer() {
        drawerLayout.closeDrawer(GravityCompat.START)
    }

    override fun copyPath(path: Path) {
        clipboardManager.copyText(path.toUserFriendlyString(), requireContext())
    }

    override fun openInNewTask(path: Path) {
        startActivitySafe(
            FileListActivity.createViewIntent(path)
                .addFlags(Intent.FLAG_ACTIVITY_NEW_DOCUMENT or Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
        )
    }

    private inner class PaneBreadcrumbListener(private val pane: PaneId) : BreadcrumbLayout.Listener {
        override fun navigateTo(path: Path) = this@DualPaneFileListFragment.navigateTo(pane, path)

        override fun copyPath(path: Path) = this@DualPaneFileListFragment.copyPath(path)

        override fun openInNewTask(path: Path) = this@DualPaneFileListFragment.openInNewTask(path)
    }

    /** The toolbar breadcrumb always represents the currently active pane. */
    private inner class ActivePaneBreadcrumbListener : BreadcrumbLayout.Listener {
        override fun navigateTo(path: Path) =
            this@DualPaneFileListFragment.navigateTo(shellViewModel.activePane, path)

        override fun copyPath(path: Path) = this@DualPaneFileListFragment.copyPath(path)

        override fun openInNewTask(path: Path) = this@DualPaneFileListFragment.openInNewTask(path)
    }

    private inner class PaneAdapterListener(private val pane: PaneId) : FileListAdapter.Listener {
        override fun clearSelectedFiles() = clearSelectedFiles(pane)

        override fun selectFile(file: FileItem, selected: Boolean) = selectFile(pane, file, selected)

        override fun selectFiles(files: FileItemSet, selected: Boolean) = selectFiles(pane, files, selected)

        override fun replaceSelectedFiles(files: FileItemSet) {
            model(pane).selectionRangeAnchorPath = null
            model(pane.other()).clearSelectedFiles()
            shellViewModel.activePane = pane
            model(pane).replaceSelectedFiles(files)
            updatePaneHeader(PaneId.LEFT)
            updatePaneHeader(PaneId.RIGHT)
            renderActionDock()
        }

        override fun openFile(file: FileItem) = openFile(pane, file)

        override fun openFileWith(file: FileItem) = this@DualPaneFileListFragment.openFileWith(file)

        override fun cutFile(file: FileItem) = transferSingle(pane, file, true)

        override fun copyFile(file: FileItem) = transferSingle(pane, file, false)

        override fun confirmDeleteFile(file: FileItem) = confirmDeleteFiles(pane, fileItemSetOf(file))

        override fun showRenameFileDialog(file: FileItem) = RenameFileDialogFragment.show(file, this@DualPaneFileListFragment)

        override fun extractFile(file: FileItem) = extractFiles(pane, fileItemSetOf(file), true)

        override fun showCreateArchiveDialog(file: FileItem) = showCreateArchiveDialog(pane, fileItemSetOf(file))

        override fun shareFile(file: FileItem) = sharePath(file.path, file.mimeType)

        override fun copyPath(file: FileItem) = this@DualPaneFileListFragment.copyPath(file.path)

        override fun addBookmark(file: FileItem) = addBookmark(file.path)

        override fun createShortcut(file: FileItem) = createShortcut(file.path, file.mimeType)

        override fun showPropertiesDialog(file: FileItem) = FilePropertiesDialogFragment.show(file, this@DualPaneFileListFragment)

        override fun onFileMenuRequested(file: FileItem): Boolean {
            showContextActionPanel(pane, file)
            return true
        }

        override fun onFileLongClick(file: FileItem): Boolean {
            showContextActionPanel(pane, file)
            return true
        }

        override fun onFileSwipeSelect(file: FileItem) = selectFileBySwipe(pane, file)

        override fun onPaneBackgroundClick() = activatePane(pane)
    }

    private fun clearSelectedFiles(pane: PaneId) {
        model(pane).clearSelectedFiles()
        renderActionDock()
    }

    private fun selectFile(pane: PaneId, file: FileItem, selected: Boolean) {
        if (!selected && model(pane).selectionRangeAnchorPath == file.path) {
            model(pane).selectionRangeAnchorPath = null
        }
        if (selected) {
            model(pane.other()).clearSelectedFiles()
            shellViewModel.activePane = pane
        }
        model(pane).selectFile(file, selected)
        updatePaneHeader(PaneId.LEFT)
        updatePaneHeader(PaneId.RIGHT)
        renderActionDock()
    }

    private fun selectFileBySwipe(pane: PaneId, file: FileItem) {
        val viewModel = model(pane)
        if (file in viewModel.selectedFiles) return
        val anchor = viewModel.selectionRangeAnchorPath?.takeIf { adapter(pane).hasFile(it) }
            ?.let { anchorPath ->
                (0 until adapter(pane).itemCount).asSequence()
                    .map { adapter(pane).getItem(it) }
                    .firstOrNull { it.path == anchorPath }
            }
        if (anchor == null) {
            selectFile(pane, file, true)
            viewModel.selectionRangeAnchorPath = file.path
            return
        }
        selectFiles(pane, adapter(pane).selectableFilesInRange(anchor, file), true)
    }

    private fun selectFiles(pane: PaneId, files: FileItemSet, selected: Boolean) {
        if (selected) {
            model(pane.other()).clearSelectedFiles()
            shellViewModel.activePane = pane
        }
        model(pane).selectFiles(files, selected)
        updatePaneHeader(PaneId.LEFT)
        updatePaneHeader(PaneId.RIGHT)
        renderActionDock()
    }

    private fun openFile(pane: PaneId, file: FileItem) {
        activatePane(pane)
        if (file.mimeType.isApk) {
            FilePropertiesDialogFragment.show(file, this)
            return
        }
        if (file.isListable) {
            navigateTo(pane, file.listablePath)
            return
        }
        recordRecentAccess(file.path, isDirectory = false)
        if (file.shouldOpenInTextEditor()) {
            val intent = TextEditorActivity.createIntent(file.path)
            startActivity(intent)
            return
        }
        openFileWithIntent(file, false)
    }

    private fun openFileWith(file: FileItem) {
        recordRecentAccess(file.path, isDirectory = false)
        openFileWithIntent(file, true)
    }

    private fun FileItem.shouldOpenInTextEditor(): Boolean {
        if (mimeType.value.startsWith("text/")) return true
        if (mimeType.value in setOf(
                "application/json", "application/xml", "application/yaml",
                "application/javascript", "application/ecmascript", "application/typescript",
                "application/x-sh", "application/x-shellscript"
            )
        ) return true
        return extension.substringAfterLast('.').lowercase(Locale.ROOT) in TEXT_FILE_EXTENSIONS
    }

    private fun openFileWithIntent(file: FileItem, withChooser: Boolean) {
        val path = file.path
        val mimeType = file.mimeType
        if (path.isArchivePath) {
            FileJobService.open(path, mimeType, withChooser, requireContext())
            return
        }
        val intent = path.fileProviderUri.createViewIntent(mimeType)
            .addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
            .apply {
                extraPath = path
                if (mimeType.isImage) addImageViewerExtras(this, path)
            }
            .let {
                if (withChooser) {
                    it.withChooser(
                        EditFileActivity::class.createIntent()
                            .putArgs(EditFileActivity.Args(path, mimeType)),
                        OpenFileAsDialogActivity::class.createIntent()
                            .putArgs(OpenFileAsDialogFragment.Args(path))
                    )
                } else it
            }
        startActivitySafe(intent)
    }

    private fun addImageViewerExtras(intent: Intent, path: Path) {
        val paths = adapter(shellViewModel.activePane).let { list ->
            (0 until list.itemCount).map { list.getItem(it) }
                .filter { it.path == path || it.mimeType.isImage }
                .mapTo(mutableListOf()) { it.path }
        }
        var position = paths.indexOf(path)
        if (position < 0) return
        if (paths.size > IMAGE_VIEWER_ACTIVITY_PATH_LIST_SIZE_MAX) {
            val start = (position - IMAGE_VIEWER_ACTIVITY_PATH_LIST_SIZE_MAX / 2)
                .coerceIn(0, paths.size - IMAGE_VIEWER_ACTIVITY_PATH_LIST_SIZE_MAX)
            position -= start
            paths.subList(start, start + IMAGE_VIEWER_ACTIVITY_PATH_LIST_SIZE_MAX)
        }
        ImageViewerActivity.putExtras(intent, paths, position)
    }

    private fun renderActionDock() {
        if (!this::actionDock.isInitialized) return
        actionDock.removeAllViews()
        val pane = shellViewModel.activePane
        val selected = if (hasSelection(pane)) model(pane).selectedFiles else null
        if (selected != null && selected.isNotEmpty()) {
            actionDock.orientation = LinearLayout.VERTICAL
            val selectionRow = LinearLayout(requireContext()).apply {
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT
                )
                orientation = LinearLayout.HORIZONTAL
            }
            actionDock.addView(selectionRow)
            addDockButton(selectionRow, R.drawable.check_icon_control_normal_24dp, R.string.select_all) {
                adapter(pane).selectAllFiles()
            }
            addDockButton(selectionRow, R.drawable.dual_swap_horiz_24dp, R.string.file_list_action_invert_selection) {
                adapter(pane).invertSelectedFiles()
            }
            addDockButton(selectionRow, R.drawable.close_icon_control_normal_24dp, R.string.file_list_action_cancel_selection) {
                clearSelectedFiles(pane)
            }
        } else {
            actionDock.orientation = LinearLayout.HORIZONTAL
            val currentPath = model(pane).currentPathLiveData.value
            val back = addDockButton(R.drawable.dual_arrow_back_24dp, R.string.file_list_action_back) {
                model(pane).currentPathLiveData.value?.let { goBack(pane, it) }
            }
            back.isEnabled = currentPath != null && shellViewModel.canGoBack(pane)
            val forward = addDockButton(R.drawable.dual_arrow_forward_24dp, R.string.file_list_action_forward) {
                model(pane).currentPathLiveData.value?.let { goForward(pane, it) }
            }
            forward.isEnabled = currentPath != null && shellViewModel.canGoForward(pane)
            val up = addDockButton(R.drawable.dual_arrow_up_24dp, R.string.file_list_action_navigate_up) {
                navigateUp(pane)
            }
            up.isEnabled = currentPath?.let(::parentPath) != null
            addDockButton(R.drawable.add_icon_white_24dp, R.string.file_list_action_new) {
                showNewActionPanel(pane)
            }
            paneSwitchButton = addDockButton(
                R.drawable.dual_swap_horiz_24dp, R.string.file_list_pane_switch
            ) { onPaneSwitchClicked(pane) }
        }
    }

    private fun renderArchiveProgress(progresses: List<FileJobProgress>) {
        if (!isAdded || !lifecycle.currentState.isAtLeast(Lifecycle.State.RESUMED)) return
        val host = activity ?: return
        if (host.isFinishing || host.isDestroyed) return
        val progress = progresses.lastOrNull { it.id !in hiddenArchiveProgressIds }
        if (progress == null) {
            archiveProgressDialog?.dismiss()
            archiveProgressDialog = null
            archiveProgressDialogId = null
            archiveProgressBar = null
            archiveProgressDetail = null
            hiddenArchiveProgressIds.retainAll(progresses.map { it.id }.toSet())
            return
        }
        val currentDialog = archiveProgressDialog
        if (currentDialog == null || archiveProgressDialogId != progress.id) {
            runCatching {
                currentDialog?.dismiss()
                val content = LinearLayout(requireContext()).apply {
                orientation = LinearLayout.VERTICAL
                setPadding(24.dp(), 0, 24.dp(), 8.dp())
                }
                archiveProgressDetail = TextView(requireContext()).apply {
                setTextAppearance(com.google.android.material.R.style.TextAppearance_Material3_BodyLarge)
                }.also { content.addView(it) }
                archiveProgressBar = ProgressBar(
                requireContext(), null, android.R.attr.progressBarStyleHorizontal
                ).apply {
                max = 100
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT, 24.dp()
                ).also { it.topMargin = 12.dp() }
                }.also { content.addView(it) }
                val id = progress.id
                archiveProgressDialogId = id
                archiveProgressDialog = MaterialAlertDialogBuilder(requireContext())
                .setTitle(progress.title)
                .setView(content)
                .setPositiveButton(R.string.hide) { dialog, _ ->
                    hiddenArchiveProgressIds += id
                    dialog.dismiss()
                    archiveProgressDialog = null
                    archiveProgressDialogId = null
                }
                .apply {
                    if (progress.operation != FileJobOperation.DELETE) {
                        setNegativeButton(android.R.string.cancel, null)
                    }
                }
                .setCancelable(false)
                .create()
                archiveProgressDialog!!.show()
                if (progress.operation != FileJobOperation.DELETE) {
                    (archiveProgressDialog as? androidx.appcompat.app.AlertDialog)
                        ?.getButton(android.content.DialogInterface.BUTTON_NEGATIVE)
                        ?.setOnClickListener {
                            if (FileJobService.cancelJob(id)) {
                                FileJobProgressRegistry.markCancelling(id)
                                it.isEnabled = false
                                (it as? android.widget.Button)?.setText(R.string.file_job_cancelling)
                            }
                        }
                }
            }.onFailure {
                // The notification remains available if the host is changing
                // windows; never let an invalid dialog token crash the app.
                archiveProgressDialog = null
                archiveProgressDialogId = null
                archiveProgressBar = null
                archiveProgressDetail = null
            }
        } else {
            currentDialog.setTitle(progress.title)
        }
        if (progress.phase == FileJobProgressPhase.CANCELLING) {
            (archiveProgressDialog as? androidx.appcompat.app.AlertDialog)
                ?.getButton(android.content.DialogInterface.BUTTON_NEGATIVE)
                ?.apply {
                    isEnabled = false
                    setText(R.string.file_job_cancelling)
                }
        }
        archiveProgressDetail?.text = if (progress.totalEntries > 0L) {
            "${progress.completedEntries} / ${progress.totalEntries}" +
                (progress.percent?.let { "  ($it%)" } ?: "")
        } else {
            progress.title
        }
        archiveProgressBar?.apply {
            isIndeterminate = progress.indeterminate
            progress.percent?.let { setProgress(it, true) }
        }
    }

    private fun onPaneSwitchClicked(pane: PaneId) {
        val now = SystemClock.uptimeMillis()
        paneSwitchTapCount = if (now - paneSwitchLastTapAt <= 1500L) paneSwitchTapCount + 1 else 1
        paneSwitchLastTapAt = now
        activatePane(pane.other())
        if (paneSwitchTapCount >= 7) {
            paneSwitchTapCount = 0
            paneSwitchLastTapAt = 0L
            paneSwitchButton?.animate()?.cancel()
            paneSwitchButton?.animate()?.rotationBy(720f)?.setDuration(500L)?.start()
        }
    }

    private fun addDockButton(icon: Int, text: Int, action: () -> Unit): MaterialButton =
        addDockButton(actionDock, icon, text, action)

    private fun addDockButton(
        parent: LinearLayout,
        icon: Int,
        text: Int,
        action: () -> Unit,
    ): MaterialButton {
        val button = MaterialButton(requireContext()).apply {
            layoutParams = LinearLayout.LayoutParams(0, 56.dp(), 1f)
            minWidth = 0
            minimumWidth = 0
            setText(text)
            textSize = 11f
            maxLines = 2
            ellipsize = TextUtils.TruncateAt.END
            setIconResource(icon)
            iconGravity = MaterialButton.ICON_GRAVITY_TEXT_TOP
            iconPadding = 2.dp()
            insetTop = 0
            insetBottom = 0
            setPadding(2.dp(), 2.dp(), 2.dp(), 2.dp())
            setOnClickListener { action() }
            setTextColor(MaterialColors.getColor(this, com.google.android.material.R.attr.colorOnSurfaceVariant))
            iconTint = android.content.res.ColorStateList.valueOf(
                MaterialColors.getColor(this, com.google.android.material.R.attr.colorOnSurfaceVariant)
            )
            backgroundTintList = android.content.res.ColorStateList.valueOf(Color.TRANSPARENT)
            rippleColor = android.content.res.ColorStateList.valueOf(
                // colorPrimary is owned by AppCompat in the host application's theme.  Looking
                // it up through Material's generated R class is not safe when the host resolves
                // a different Material artifact version: R8 can then leave no such field in the
                // final APK (and crash the file manager at startup).
                MaterialColors.getColor(this, androidx.appcompat.R.attr.colorPrimary, Color.GRAY)
            ).withAlpha(32)
            contentDescription = getString(text)
        }
        parent.addView(button)
        return button
    }

    private fun setupRecentAccessDock() {
        recentAccessAdapter = RecentAccessAdapter(::openRecentAccess)
        recentAccessList.layoutManager = GridLayoutManager(requireContext(), 1)
        recentAccessList.adapter = recentAccessAdapter
        root.findViewById<View>(R.id.dualRecentAccessClear).setOnClickListener {
            recentAccessStore.clear()
            refreshRecentAccess()
        }
        dockContainer.onExpandRequested = { setRecentAccessExpanded(true) }
        dockContainer.onCollapseRequested = { setRecentAccessExpanded(false) }
        root.post {
            refreshRecentAccess()
            if (shellViewModel.isRecentAccessExpanded) setRecentAccessExpanded(true, animate = false)
        }
    }

    private fun recordRecentAccess(path: Path, isDirectory: Boolean) {
        recentAccessStore.record(path, isDirectory)
        if (this::recentAccessAdapter.isInitialized) refreshRecentAccess()
    }

    private fun refreshRecentAccess() {
        if (!this::recentAccessAdapter.isInitialized) return
        val entries = recentAccessStore.entries()
        recentAccessAdapter.replace(entries)
        recentAccessEmpty.isVisible = entries.isEmpty()
        recentAccessList.isVisible = entries.isNotEmpty()
    }

    private fun setRecentAccessExpanded(expanded: Boolean, animate: Boolean = true) {
        if (!this::recentAccessPanel.isInitialized) return
        if (shellViewModel.isRecentAccessExpanded == expanded &&
            (expanded == recentAccessPanel.isVisible)
        ) return
        shellViewModel.isRecentAccessExpanded = expanded
        refreshRecentAccess()
        val targetHeight = if (expanded) {
            (root.height * 0.45f).roundToInt().coerceAtLeast(160.dp())
        } else 0
        val startHeight = recentAccessPanel.height
        recentAccessPanel.isVisible = true
        if (!animate || startHeight == targetHeight) {
            recentAccessPanel.layoutParams = recentAccessPanel.layoutParams.apply { height = targetHeight }
            recentAccessPanel.isVisible = expanded
            return
        }
        ValueAnimator.ofInt(startHeight, targetHeight).apply {
            duration = 180L
            interpolator = FastOutSlowInInterpolator()
            addUpdateListener {
                recentAccessPanel.layoutParams = recentAccessPanel.layoutParams.apply {
                    height = it.animatedValue as Int
                }
                recentAccessPanel.requestLayout()
            }
            doOnEnd { recentAccessPanel.isVisible = expanded }
            start()
        }
    }

    private fun openRecentAccess(entry: RecentAccessEntry) {
        val path = runCatching { Paths.get(URI.create(entry.uri)) }.getOrNull()
            ?: run { showToast(R.string.file_list_recent_access_unavailable); return }
        setRecentAccessExpanded(false)
        lifecycleScope.launch {
            val file = withContext(Dispatchers.IO) {
                runCatching {
                    if (entry.isDirectory) {
                        if (!path.isDirectory()) throw IllegalStateException("Recent directory is unavailable")
                        null
                    } else {
                        path.loadFileItem()
                    }
                }
            }.getOrElse {
                showToast(R.string.file_list_recent_access_unavailable)
                return@launch
            }
            if (entry.isDirectory) navigateTo(shellViewModel.activePane, path)
            else file?.let { openFile(shellViewModel.activePane, it) }
        }
    }

    private class RecentAccessAdapter(
        private val onClick: (RecentAccessEntry) -> Unit,
    ) : RecyclerView.Adapter<RecentAccessAdapter.ViewHolder>() {
        private var entries: List<RecentAccessEntry> = emptyList()

        fun replace(entries: List<RecentAccessEntry>) {
            this.entries = entries
            notifyDataSetChanged()
        }

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
            val context = parent.context
            val density = context.resources.displayMetrics.density
            val icon = ImageView(context).apply {
                layoutParams = LinearLayout.LayoutParams((40 * density).roundToInt(), (40 * density).roundToInt())
                setPadding((8 * density).roundToInt(), (8 * density).roundToInt(), (8 * density).roundToInt(), (8 * density).roundToInt())
                imageTintList = android.content.res.ColorStateList.valueOf(
                    MaterialColors.getColor(this, com.google.android.material.R.attr.colorOnSurfaceVariant)
                )
            }
            val title = TextView(context).apply {
                textSize = 16f
                maxLines = 1
                ellipsize = TextUtils.TruncateAt.MIDDLE
            }
            val path = TextView(context).apply {
                textSize = 12f
                maxLines = 1
                ellipsize = TextUtils.TruncateAt.MIDDLE
                setTextColor(MaterialColors.getColor(this, com.google.android.material.R.attr.colorOnSurfaceVariant))
            }
            val details = LinearLayout(context).apply {
                orientation = LinearLayout.VERTICAL
                layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
                addView(title)
                addView(path)
            }
            val view = LinearLayout(context).apply {
                layoutParams = RecyclerView.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT, (56 * density).roundToInt()
                )
                gravity = Gravity.CENTER_VERTICAL
                orientation = LinearLayout.HORIZONTAL
                setPadding((12 * density).roundToInt(), 0, (12 * density).roundToInt(), 0)
                val styled = context.obtainStyledAttributes(intArrayOf(android.R.attr.selectableItemBackground))
                foreground = styled.getDrawable(0)
                styled.recycle()
                addView(icon)
                addView(details)
            }
            return ViewHolder(view, icon, title, path)
        }

        override fun onBindViewHolder(holder: ViewHolder, position: Int) {
            val entry = entries[position]
            holder.icon.setImageResource(
                if (entry.isDirectory) R.drawable.directory_icon_white_24dp
                else R.drawable.file_icon_white_24dp
            )
            holder.title.text = entry.title
            holder.path.text = entry.displayPath
            holder.root.contentDescription = "${entry.title}, ${entry.displayPath}"
            holder.root.setOnClickListener { onClick(entry) }
        }

        override fun getItemCount(): Int = entries.size

        class ViewHolder(
            val root: View,
            val icon: ImageView,
            val title: TextView,
            val path: TextView,
        ) : RecyclerView.ViewHolder(root)
    }

    private fun showNewActionPanel(pane: PaneId) {
        activatePane(pane)
        MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.file_list_action_new)
            .setItems(
                arrayOf(
                    getString(R.string.file_list_action_create_directory),
                    getString(R.string.file_list_action_create_file),
                )
            ) { _, which ->
                if (which == 0) CreateDirectoryDialogFragment.show(this)
                else CreateFileDialogFragment.show(this)
            }
            .show()
    }

    private fun transferToOther(
        pane: PaneId,
        move: Boolean,
        selectedFiles: FileItemSet? = null,
    ) {
        val destinationPane = pane.other()
        val destination = model(destinationPane).currentPath
        val files = selectedFiles ?: model(pane).selectedFiles
        if (files.isEmpty()) return
        transferBlockReason(pane, move, files)?.let { showToast(it); return }
        val paths = makePathListForJob(files)
        val archiveSelection = archiveEntrySelection(files)
        if (!move && archiveSelection != null) {
            FileJobService.extractZipXtract(
                listOf(archiveSelection.first), destination, false, requireContext(),
                entries = archiveSelection.second,
            )
        } else if (!move && paths.all { it.isArchivePath }) {
            // A selection made in a normal directory contains archive files,
            // whereas archiveEntrySelection() above represents entries inside
            // an already opened archive.
            FileJobService.extractZipXtract(paths, destination, false, requireContext())
        } else if (move) {
            FileJobService.move(paths, destination, requireContext())
        } else {
            FileJobService.copy(paths, destination, requireContext())
        }
        model(pane).clearSelectedFiles()
        // FileJobService dispatches a result after the operation has actually
        // completed. Refreshing here would present a failed copy/move as if it
        // had already succeeded on slow or remote providers.
        activatePane(destinationPane)
    }

    /** Returns the physical archive and safe entry names for an archive-FS selection. */
    private fun archiveEntrySelection(files: FileItemSet): Pair<Path, Set<String>>? {
        if (files.isEmpty() || files.any { !it.path.isArchivePath }) return null
        val roots = files.mapNotNull { it.path.root }.distinctBy { it.toString() }
        if (roots.size != 1) return null
        val root = roots.single()
        val archive = runCatching { root.archiveFile }.getOrNull() ?: return null
        val names = files.mapNotNull { file ->
            runCatching { root.relativize(file.path) }.getOrNull()
                ?.takeUnless { it.isAbsolute }
                ?.toString()?.replace('\\', '/')?.trim('/')
                ?.takeIf { it.isNotEmpty() }
        }.toSet()
        if (names.isEmpty()) return null
        return archive to names
    }

    private fun transferBlockReason(pane: PaneId, move: Boolean): Int? {
        val files = model(pane).selectedFiles
        if (files.isEmpty()) return null
        return transferBlockReason(pane, move, files)
    }

    private fun transferBlockReason(pane: PaneId, move: Boolean, files: FileItemSet): Int? =
        transferBlockReason(pane, files.map { it.path }, move)

    private fun transferBlockReasonForFile(pane: PaneId, file: FileItem, move: Boolean): Int? =
        transferBlockReason(pane, listOf(file.path), move)

    private fun transferBlockReason(pane: PaneId, paths: List<Path>, move: Boolean): Int? {
        if (paths.isEmpty()) return null
        val destination = model(pane.other()).currentPath
        if (runCatching { destination.fileSystem.isReadOnly }.getOrDefault(true)) {
            return R.string.file_list_action_destination_read_only
        }
        val sortedPaths = paths.sortedBy { it.toUri() }
        if (sortedPaths.any { source ->
                runCatching {
                    source == destination ||
                        (source.isDirectoryPath() && destination.startsWith(source))
                }.getOrDefault(true)
            }) {
            return R.string.file_job_cannot_copy_move_into_itself_message
        }
        if (move && sortedPaths.any { it.isArchivePath }) {
            return R.string.file_list_select_action_extract
        }
        return null
    }

    private fun Path.isDirectoryPath(): Boolean = runCatching { isDirectory() }.getOrDefault(false)

    private fun makePathListForJob(files: FileItemSet): List<Path> = files.map { it.path }.sortedBy { it.toUri() }

    private fun showContextActionPanel(pane: PaneId, file: FileItem) {
        contextMenuHidingDockOperations = true
        renderActionDock()
        val restoreDock = {
            contextMenuHidingDockOperations = false
            renderActionDock()
        }
        val selected = model(pane).selectedFiles
        if (selected.isNotEmpty()) {
            if (file !in selected) selectFile(pane, file, true)
            val snapshot = fileItemSetOf(*model(pane).selectedFiles.toTypedArray())
            if (snapshot.size > 1) {
                showSelectionActionPanel(pane, snapshot, restoreDock)
                return
            }
        }
        showEntryActionPanel(pane, file, restoreDock)
    }

    private fun showEntryActionPanel(
        pane: PaneId,
        file: FileItem,
        onDismiss: (() -> Unit)? = null,
    ) {
        showActionPanel(file.name, entryActions(pane, file), onDismiss)
    }

    private fun showActionPanel(
        title: CharSequence,
        actions: List<EntryAction>,
        onDismiss: (() -> Unit)? = null,
    ) {
        val dialog = MaterialAlertDialogBuilder(requireContext()).setTitle(title).create()
        val content = LinearLayout(requireContext()).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(12.dp(), 12.dp(), 12.dp(), 12.dp())
        }
        val grid = android.widget.GridLayout(requireContext()).apply {
            columnCount = 2
            useDefaultMargins = false
        }
        actions.forEach { action ->
            val button = MaterialButton(requireContext()).apply {
                layoutParams = android.widget.GridLayout.LayoutParams().apply {
                    width = 0
                    height = 56.dp()
                    columnSpec = android.widget.GridLayout.spec(android.widget.GridLayout.UNDEFINED, 1f)
                }
                minWidth = 0
                minimumWidth = 0
                insetTop = 0
                insetBottom = 0
                cornerRadius = 0
                gravity = Gravity.START or Gravity.CENTER_VERTICAL
                setText(action.title)
                backgroundTintList = android.content.res.ColorStateList.valueOf(
                    MaterialColors.getColor(this, com.google.android.material.R.attr.colorSurface)
                )
                strokeWidth = 1.dp()
                strokeColor = android.content.res.ColorStateList.valueOf(
                    MaterialColors.getColor(this, com.google.android.material.R.attr.colorOutlineVariant)
                )
                setTextColor(MaterialColors.getColor(this, com.google.android.material.R.attr.colorOnSurface))
                contentDescription = action.disabledReason?.let {
                    getString(R.string.file_list_action_disabled_with_reason, action.title, getString(it))
                } ?: action.title
                setIconResource(action.icon)
                iconTint = android.content.res.ColorStateList.valueOf(
                    MaterialColors.getColor(this, com.google.android.material.R.attr.colorOnSurfaceVariant)
                )
                isEnabled = action.enabled
                setOnClickListener {
                    dialog.dismiss()
                    action.run()
                }
            }
            grid.addView(button)
        }
        content.addView(grid)
        // AlertDialog installs its own content when show() first creates the window. Using
        // Dialog.setContentView() here is therefore overwritten and leaves only a dimmed,
        // empty dialog on screen. Register the action grid with AlertController instead.
        dialog.setView(content)
        dialog.setCanceledOnTouchOutside(true)
        dialog.setOnDismissListener { onDismiss?.invoke() }
        dialog.show()
        dialog.window?.setLayout(
            (360.dp()).coerceAtMost(resources.displayMetrics.widthPixels - 32.dp()),
            android.view.WindowManager.LayoutParams.WRAP_CONTENT,
        )
    }

    private data class EntryAction(
        val title: String,
        val icon: Int,
        val enabled: Boolean = true,
        val disabledReason: Int? = null,
        val run: () -> Unit,
    )

    private fun entryActions(pane: PaneId, file: FileItem): List<EntryAction> {
        val readOnly = file.path.fileSystem.isReadOnly
        val archivePath = file.path.isArchivePath
        if (archivePath) {
            val files = fileItemSetOf(file)
            val archiveDirectory = archiveEntrySelection(files)?.first?.parent
            val otherDirectory = model(pane.other()).currentPath
            return listOf(
                EntryAction(
                    getString(R.string.file_list_select_action_extract_here),
                    R.drawable.extract_icon_control_normal_24dp,
                    enabled = archiveDirectory != null &&
                        !archiveDirectory.fileSystem.isReadOnly,
                ) {
                    archiveDirectory?.let {
                        chooseArchiveEntryExtractionMode(pane, files, it, false)
                    }
                },
                EntryAction(
                    getString(R.string.file_list_select_action_extract_other_pane),
                    R.drawable.extract_icon_control_normal_24dp,
                    enabled = !otherDirectory.fileSystem.isReadOnly &&
                        !otherDirectory.isArchivePath,
                    disabledReason = if (otherDirectory.fileSystem.isReadOnly ||
                        otherDirectory.isArchivePath
                    ) R.string.file_list_action_destination_read_only else null,
                ) {
                    chooseArchiveEntryExtractionMode(pane, files, otherDirectory, true)
                },
                EntryAction(
                    getString(R.string.file_item_action_properties),
                    R.drawable.information_icon_white_24dp,
                ) { FilePropertiesDialogFragment.show(file, this) },
                EntryAction(
                    getString(R.string.share),
                    R.drawable.dual_share_24dp,
                    enabled = !file.attributes.isDirectory,
                ) { sharePath(file.path, file.mimeType) },
            )
        }
        val transferBlockCopy = transferBlockReasonForFile(pane, file, false)
        val transferBlockMove = transferBlockReasonForFile(pane, file, true)
        val actions = mutableListOf<EntryAction>()
        if (file.isArchiveFile) {
            actions += EntryAction(
                getString(R.string.file_list_select_action_extract),
                R.drawable.extract_icon_control_normal_24dp,
                enabled = !model(pane).currentPath.fileSystem.isReadOnly,
            ) { extractFiles(pane, fileItemSetOf(file), false) }
        }
        actions += listOf(
            EntryAction(
                getString(R.string.file_list_action_copy_short),
                R.drawable.copy_icon_control_normal_24dp,
                enabled = transferBlockCopy == null,
                disabledReason = transferBlockCopy,
            ) { transferSingle(pane, file, false) },
            EntryAction(
                getString(R.string.file_list_action_move_short),
                R.drawable.dual_arrow_forward_24dp,
                enabled = transferBlockMove == null && !archivePath,
                disabledReason = transferBlockMove,
            ) { transferSingle(pane, file, true) },
            EntryAction(
                getString(R.string.file_list_action_link), R.drawable.link_icon_control_normal_24dp,
            ) { createShortcut(file.path, file.mimeType) },
            EntryAction(getString(R.string.rename), R.drawable.edit_icon, !readOnly) {
                RenameFileDialogFragment.show(file, this)
            },
            EntryAction(getString(R.string.delete), R.drawable.delete_icon_control_normal_24dp, !readOnly) {
                confirmDeleteFiles(pane, fileItemSetOf(file))
            },
            EntryAction(
                getString(R.string.file_list_action_compress), R.drawable.file_archive_icon,
                enabled = !archivePath && !model(pane).currentPath.fileSystem.isReadOnly,
            ) { showCreateArchiveDialog(pane, fileItemSetOf(file)) },
            EntryAction(getString(R.string.file_item_action_properties), R.drawable.information_icon_white_24dp) {
                FilePropertiesDialogFragment.show(file, this)
            },
            EntryAction(getString(R.string.share), R.drawable.dual_share_24dp) {
                sharePath(file.path, file.mimeType)
            },
            EntryAction(getString(R.string.file_item_action_open_with), R.drawable.open_as_icon) {
                openFileWith(file)
            },
            EntryAction(
                getString(R.string.file_list_action_add_bookmark), R.drawable.dual_bookmark_24dp,
                enabled = file.attributes.isDirectory,
            ) { addBookmark(file.path) },
        )
        return actions
    }

    private fun showSelectionActionPanel(
        pane: PaneId,
        files: FileItemSet,
        onDismiss: (() -> Unit)? = null,
    ) {
        activatePane(pane)
        val snapshot = fileItemSetOf(*files.toTypedArray())
        val archiveFiles = snapshot.all { it.isArchiveFile }
        val archiveEntries = snapshot.all { it.path.isArchivePath }
        if (archiveEntries) {
            val archiveDirectory = archiveEntrySelection(snapshot)?.first?.parent
            val otherDirectory = model(pane.other()).currentPath
            showActionPanel(
                getString(R.string.file_list_select_title_format, snapshot.size),
                listOf(
                    EntryAction(
                        getString(R.string.file_list_select_action_extract_here),
                        R.drawable.extract_icon_control_normal_24dp,
                        enabled = archiveDirectory != null &&
                            !archiveDirectory.fileSystem.isReadOnly,
                    ) {
                        archiveDirectory?.let {
                            chooseArchiveEntryExtractionMode(pane, snapshot, it, false)
                        }
                    },
                    EntryAction(
                        getString(R.string.file_list_select_action_extract_other_pane),
                        R.drawable.extract_icon_control_normal_24dp,
                        enabled = !otherDirectory.fileSystem.isReadOnly &&
                            !otherDirectory.isArchivePath,
                        disabledReason = if (otherDirectory.fileSystem.isReadOnly ||
                            otherDirectory.isArchivePath
                        ) R.string.file_list_action_destination_read_only else null,
                    ) {
                        chooseArchiveEntryExtractionMode(pane, snapshot, otherDirectory, true)
                    },
                    EntryAction(
                        getString(R.string.share),
                        R.drawable.dual_share_24dp,
                        enabled = snapshot.none { it.attributes.isDirectory },
                    ) { shareFiles(snapshot, pane) },
                ),
                onDismiss,
            )
            return
        }
        val copyReason = transferBlockReason(pane, false, snapshot)
        val moveReason = transferBlockReason(pane, true, snapshot)
        val actions = mutableListOf<EntryAction>()
        actions += EntryAction(
            getString(R.string.file_list_action_copy_short), R.drawable.copy_icon_control_normal_24dp,
            enabled = copyReason == null, disabledReason = copyReason,
        ) { transferToOther(pane, false, snapshot) }
        actions += EntryAction(
            getString(R.string.file_list_action_move_short), R.drawable.dual_arrow_forward_24dp,
            enabled = moveReason == null, disabledReason = moveReason,
        ) { transferToOther(pane, true, snapshot) }
        actions += EntryAction(getString(R.string.delete), R.drawable.delete_icon_control_normal_24dp,
            enabled = snapshot.all { !it.path.fileSystem.isReadOnly }) {
            confirmDeleteFiles(pane, snapshot)
        }
        if (!archiveEntries && !model(pane).currentPath.fileSystem.isReadOnly) {
            actions += EntryAction(getString(R.string.file_list_action_compress), R.drawable.file_archive_icon) {
                showCreateArchiveDialog(pane, snapshot)
            }
        }
        actions += EntryAction(getString(R.string.share), R.drawable.dual_share_24dp) {
            shareFiles(snapshot, pane)
        }
        if (archiveFiles) {
            actions += EntryAction(
                getString(R.string.file_list_select_action_extract_here), R.drawable.file_archive_icon
            ) { extractFiles(pane, snapshot, false) }
            actions += EntryAction(
                getString(R.string.file_list_select_action_extract_choose), R.drawable.file_archive_icon
            ) { chooseExtractionDirectory(pane, snapshot) }
        }
        showActionPanel(
            getString(R.string.file_list_select_title_format, snapshot.size), actions, onDismiss
        )
    }

    private fun showLegacySelectionActionPanel(pane: PaneId, files: FileItemSet) {
        val archiveFiles = files.all { it.isArchiveFile }
        val archiveEntries = files.all { it.path.isArchivePath }
        val items = mutableListOf<Pair<String, () -> Unit>>()
        if (archiveEntries) {
            items += getString(R.string.file_list_select_action_extract_choose) to {
                chooseExtractionDirectory(pane, files)
            }
            MaterialAlertDialogBuilder(requireContext())
                .setTitle(getString(R.string.file_list_select_title_format, files.size))
                .setItems(items.map { it.first }.toTypedArray()) { _, which -> items[which].second() }
                .show()
            return
        }
        if (archiveFiles) {
            items += getString(R.string.file_list_select_action_extract_here) to {
                extractFiles(pane, files, false)
            }
            items += getString(R.string.file_list_select_action_extract_choose) to {
                chooseExtractionDirectory(pane, files)
            }
        }
        if (archiveEntries && currentSevenZArchiveFile(pane) != null && archiveEditCapabilities[pane] == true) {
            items += getString(R.string.file_list_select_action_archive_delete_entries) to {
                deleteArchiveEntries(pane, files)
            }
        }
        if (!model(pane).currentPath.fileSystem.isReadOnly && !archiveEntries) {
            items += getString(R.string.file_list_select_action_archive) to {
                showCreateArchiveDialog(pane, files)
            }
        }
        items += getString(R.string.share) to { shareFiles(files) }
        items += getString(R.string.select_all) to { adapter(pane).selectAllFiles() }
        if (items.isEmpty()) return
        MaterialAlertDialogBuilder(requireContext())
            .setTitle(getString(R.string.file_list_select_title_format, files.size))
            .setItems(items.map { it.first }.toTypedArray()) { _, which -> items[which].second() }
            .show()
    }

    private fun shareFiles(files: FileItemSet, sourcePane: PaneId = shellViewModel.activePane) {
        shareFiles(files.map { it.path }, files.map { it.mimeType })
        model(sourcePane).selectFiles(files, false)
    }

    private fun shareFiles(paths: List<Path>, mimeTypes: List<MimeType>) {
        if (paths.isEmpty()) return
        val uris = paths.map { it.fileProviderUri }
        val intent = uris.createSendStreamIntent(mimeTypes).apply {
            addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
            clipData = ClipData.newUri(
                requireContext().contentResolver,
                paths.first().fileName?.toString() ?: "",
                uris.first(),
            ).also { clip -> uris.drop(1).forEach { clip.addItem(ClipData.Item(it)) } }
        }
        val chooser = intent.withChooser().apply {
            addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
            clipData = intent.clipData
        }
        startActivitySafe(chooser)
    }

    private fun deleteArchiveEntries(pane: PaneId, files: FileItemSet) {
        val archive = currentSevenZArchiveFile(pane) ?: return
        val root = model(pane).currentPath.root ?: return
        val names = files.mapNotNull { file ->
            runCatching { root.relativize(file.path) }.getOrNull()
                ?.takeUnless { it.isAbsolute }
                ?.toString()?.replace('\\', '/')?.trim('/')
                ?.takeIf { it.isNotEmpty() }
        }.toSet()
        if (names.isEmpty()) return
        FileJobService.update7z(archive, emptyList(), names, requireContext())
        model(pane).selectFiles(files, false)
    }

    private class PaneBinding(val root: View) {
        val header: View = root.findViewById(R.id.paneHeader)
        val label: TextView = root.findViewById(R.id.paneLabel)
        val currentBadge: TextView = root.findViewById(R.id.paneCurrentBadge)
        val selectedCount: TextView = root.findViewById(R.id.paneSelectedCount)
        val path: TextView = root.findViewById(R.id.panePath)
        val breadcrumbLayout: BreadcrumbLayout = root.findViewById(R.id.paneBreadcrumb)
        val summary: TextView = root.findViewById(R.id.paneSummary)
        val progress: ProgressBar = root.findViewById(R.id.paneProgress)
        val errorText: TextView = root.findViewById(R.id.paneErrorText)
        val emptyView: View = root.findViewById(R.id.paneEmptyView)
        val contentLayout: CoordinatorScrollingFrameLayout = root.findViewById(R.id.paneContentLayout)
        val swipeRefreshLayout: SwipeRefreshLayout = root.findViewById(R.id.paneSwipeRefreshLayout)
        val recyclerView: RecyclerView = root.findViewById(R.id.paneRecyclerView)
        val parentRow: View = root.findViewById(R.id.paneParentRow)
        val parentText: TextView = root.findViewById(R.id.paneParentText)
    }

    private fun Int.dp(): Int = (this * resources.displayMetrics.density).roundToInt()

    private fun android.content.res.ColorStateList.withAlpha(alpha: Int): android.content.res.ColorStateList =
        android.content.res.ColorStateList.valueOf(defaultColor and 0x00FFFFFF or (alpha.coerceIn(0, 255) shl 24))

    companion object {
        private const val IMAGE_VIEWER_ACTIVITY_PATH_LIST_SIZE_MAX = 1000

        private val TEXT_FILE_EXTENSIONS = setOf(
            "txt", "text", "log", "md", "markdown", "mkd", "json", "json5", "xml", "yaml", "yml",
            "csv", "tsv", "ini", "conf", "cfg", "properties", "toml", "gradle", "java", "kt", "kts",
            "c", "cc", "cpp", "h", "hpp", "cs", "go", "rs", "py", "rb", "php", "js", "mjs", "cjs",
            "ts", "tsx", "jsx", "css", "scss", "html", "htm", "xhtml", "sh", "bash", "zsh", "fish",
            "bat", "cmd", "ps1", "sql", "lua", "r", "swift", "dart", "vue", "svelte"
        )
    }
    private fun transferSingle(pane: PaneId, file: FileItem, move: Boolean) {
        model(pane).clearSelectedFiles()
        selectFile(pane, file, true)
        transferToOther(pane, move)
    }

    private fun confirmDeleteFiles(pane: PaneId, files: FileItemSet) {
        if (files.isEmpty()) return
        ConfirmDeleteFilesDialogFragment.show(files, this)
        pendingDeletePane = pane
    }

    private var pendingDeletePane: PaneId = PaneId.LEFT

    override fun deleteFiles(files: FileItemSet) {
        FileJobService.delete(makePathListForJob(files), requireContext())
        model(pendingDeletePane).selectFiles(files, false)
    }

    private fun extractFiles(pane: PaneId, files: FileItemSet, containingDirectory: Boolean) {
        FileJobService.extractZipXtract(
            makePathListForJob(files), model(pane).currentPath,
            createContainingDirectory = containingDirectory,
            context = requireContext(),
        )
        model(pane).selectFiles(files, false)
    }

    private fun extractArchiveEntries(
        pane: PaneId,
        files: FileItemSet,
        targetDirectory: Path,
        createContainingDirectory: Boolean,
        activateDestination: Boolean = false,
    ) {
        val selection = archiveEntrySelection(files) ?: return
        if (targetDirectory.fileSystem.isReadOnly || targetDirectory.isArchivePath) {
            showToast(R.string.file_list_action_destination_read_only)
            return
        }
        if (files.any { it.attributes.isEncrypted() }) {
            showArchiveExtractionPasswordDialog(selection.first) { password ->
                startArchiveEntryExtraction(
                    pane,
                    files,
                    selection,
                    targetDirectory,
                    createContainingDirectory,
                    activateDestination,
                    password,
                )
            }
            return
        }
        startArchiveEntryExtraction(
            pane,
            files,
            selection,
            targetDirectory,
            createContainingDirectory,
            activateDestination,
            null,
        )
    }

    private fun startArchiveEntryExtraction(
        pane: PaneId,
        files: FileItemSet,
        selection: Pair<Path, Set<String>>,
        targetDirectory: Path,
        createContainingDirectory: Boolean,
        activateDestination: Boolean,
        password: CharArray?,
    ) {
        FileJobService.extractZipXtract(
            listOf(selection.first),
            targetDirectory,
            createContainingDirectory = createContainingDirectory,
            context = requireContext(),
            entries = selection.second,
            password = password,
        )
        model(pane).selectFiles(files, false)
        if (activateDestination) activatePane(pane.other())
    }

    private fun showArchiveExtractionPasswordDialog(
        archive: Path,
        onPassword: (CharArray) -> Unit,
    ) {
        val passwordBinding = ArchivePasswordDialogBinding.inflate(requireContext().layoutInflater)
        passwordBinding.passwordEdit.configureSecurePasswordInput()
        passwordBinding.passwordEdit.hideTextInputLayoutErrorOnTextChange(passwordBinding.passwordLayout)
        val dialog = MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.file_action_archive_password_title)
            .setMessage(
                getString(
                    R.string.file_action_archive_password_message_format,
                    archive.fileName,
                )
            )
            .setView(passwordBinding.root)
            .setPositiveButton(android.R.string.ok, null)
            .setNegativeButton(android.R.string.cancel) { _, _ ->
                passwordBinding.passwordEdit.text?.clear()
            }
            .create()
        dialog.setOnShowListener {
            dialog.window?.addFlags(android.view.WindowManager.LayoutParams.FLAG_SECURE)
            dialog.getButton(androidx.appcompat.app.AlertDialog.BUTTON_POSITIVE)
                .setOnClickListener {
                    val password = passwordBinding.passwordEdit.text?.toString().orEmpty()
                    if (password.isEmpty()) {
                        passwordBinding.passwordLayout.error =
                            getString(R.string.file_action_archive_password_error_empty)
                        return@setOnClickListener
                    }
                    val characters = password.toCharArray()
                    passwordBinding.passwordEdit.text?.clear()
                    dialog.dismiss()
                    onPassword(characters)
                }
            passwordBinding.passwordEdit.requestFocus()
        }
        dialog.setCanceledOnTouchOutside(false)
        dialog.show()
    }

    private fun chooseArchiveEntryExtractionMode(
        pane: PaneId,
        files: FileItemSet,
        targetDirectory: Path,
        activateDestination: Boolean,
    ) {
        MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.file_list_extract_mode_title)
            .setItems(
                arrayOf(
                    getString(R.string.file_list_extract_mode_direct),
                    getString(R.string.file_list_extract_mode_containing_directory),
                )
            ) { _, which ->
                extractArchiveEntries(
                    pane,
                    files,
                    targetDirectory,
                    createContainingDirectory = which == 1,
                    activateDestination = activateDestination,
                )
            }
            .show()
    }

    private fun chooseExtractionDirectory(pane: PaneId, files: FileItemSet) {
        archiveEntrySelection(files)?.let {
            pendingExtractionEntries = it
            pendingExtractionSources = null
            pendingExtractionPane = null
            extractionDirectoryLauncher.launch(it.first.parent)
            return
        }
        pendingExtractionPane = pane
        pendingExtractionSources = makePathListForJob(files)
        extractionDirectoryLauncher.launch(model(pane).currentPath)
    }

    private val extractionDirectoryLauncher = registerForActivityResult(
        FileListActivity.OpenDirectoryContract(), ::onExtractionDirectoryResult
    )

    private fun onExtractionDirectoryResult(path: Path?) {
        val entries = pendingExtractionEntries
        pendingExtractionEntries = null
        if (entries != null) {
            if (path != null) {
                FileJobService.extractZipXtract(
                    listOf(entries.first), path, false, requireContext(),
                    entries = entries.second,
                )
            }
            return
        }
        val sources = pendingExtractionSources ?: return
        val pane = pendingExtractionPane
        pendingExtractionSources = null
        pendingExtractionPane = null
        if (path == null) return
        FileJobService.extractZipXtract(sources, path, true, requireContext())
        pane?.let { model(it).clearSelectedFiles() }
    }

    private fun sharePath(path: Path, mimeType: MimeType) {
        val uri = path.fileProviderUri
        val sendIntent = listOf(uri).createSendStreamIntent(listOf(mimeType)).apply {
            addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
            clipData = ClipData.newUri(requireContext().contentResolver, path.fileName?.toString() ?: "", uri)
        }
        startActivitySafe(sendIntent.withChooser().addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION))
    }

    private fun addBookmark(path: Path) {
        BookmarkDirectories.add(BookmarkDirectory(null, path))
        showToast(R.string.file_add_bookmark_success)
    }

    private fun createShortcut(path: Path, mimeType: MimeType) {
        val context = requireContext()
        val isDirectory = mimeType == MimeType.DIRECTORY
        val shortcut = ShortcutInfoCompat.Builder(context, path.toString())
            .setShortLabel(path.fileName?.toString() ?: path.toString())
            .setIntent(
                if (isDirectory) FileListActivity.createViewIntent(path)
                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
                else OpenFileActivity.createIntent(path, mimeType)
            )
            .setIcon(
                IconCompat.createWithResource(
                    context, if (isDirectory) R.mipmap.directory_shortcut_icon
                    else R.mipmap.file_shortcut_icon
                )
            )
            .build()
        ShortcutManagerCompat.requestPinShortcut(context, shortcut, null)
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) showToast(R.string.shortcut_created)
    }

    fun copyPath(file: FileItem) = copyPath(file.path)

    fun addBookmark(file: FileItem) = addBookmark(file.path)

    fun createShortcut(file: FileItem) = createShortcut(file.path, file.mimeType)

    override fun hasFileWithName(name: String): Boolean {
        val files = model(shellViewModel.activePane).fileListStateful
        return files.value?.any { it.name == name } == true
    }

    override fun renameFile(file: FileItem, newName: String) {
        FileJobService.rename(file.path, newName, requireContext())
        model(shellViewModel.activePane).selectFile(file, false)
    }

    override fun replaceFile(file: FileItem) {
        FileJobService.create(file.path, false, requireContext())
    }

    override fun createFile(name: String) {
        FileJobService.create(model(shellViewModel.activePane).currentPath.resolve(name), false, requireContext())
    }

    override fun createDirectory(name: String) {
        FileJobService.create(model(shellViewModel.activePane).currentPath.resolve(name), true, requireContext())
    }

    private fun showCreateArchiveDialog(pane: PaneId, files: FileItemSet) {
        shellViewModel.activePane = pane
        CreateArchiveDialogFragment.show(files, this)
    }

    override fun archive(
        files: FileItemSet,
        name: String,
        format: Int,
        filter: Int,
        password: String?
    ) {
        val archiveFile = model(shellViewModel.activePane).currentPath.resolve(name)
        FileJobService.archive(
            makePathListForJob(files), archiveFile, format, filter, password, requireContext()
        )
        model(shellViewModel.activePane).selectFiles(files, false)
    }

    override fun archiveWithOptions(
        files: FileItemSet,
        name: String,
        format: ArchiveFormat,
        options: ArchiveCreateOptions,
        password: String?
    ) {
        val splitSize = when (format) {
            ArchiveFormat.ZIP -> options.zipSplitSizeBytes
            ArchiveFormat.SEVEN_ZIP -> options.sevenZipSplitSizeBytes
            else -> null
        }
        val allowLargeSplit = when (format) {
            ArchiveFormat.ZIP -> options.allowLargeZipSplit
            ArchiveFormat.SEVEN_ZIP -> options.allowLargeSevenZipSplit
            else -> true
        }
        val estimatedBytes = files.sumOf { it.attributes.size().coerceAtLeast(0L) }
        val estimatedVolumes = if (splitSize == null || splitSize <= 0L || estimatedBytes <= 0L) {
            0L
        } else ((estimatedBytes - 1L) / splitSize) + 1L
        if (splitSize != null && estimatedVolumes > 100L && !allowLargeSplit
        ) {
            MaterialAlertDialogBuilder(requireContext())
                .setTitle(R.string.file_create_archive_large_split_title)
                .setMessage(R.string.file_create_archive_large_split_message)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(android.R.string.ok) { _, _ ->
                    archiveWithOptions(
                        files, name, format,
                        if (format == ArchiveFormat.ZIP) {
                            options.copy(allowLargeZipSplit = true)
                        } else {
                            options.copy(allowLargeSevenZipSplit = true)
                        },
                        password
                    )
                }
                .show()
            return
        }
        val archiveFile = model(shellViewModel.activePane).currentPath.resolve(name)
        FileJobService.archiveZipXtract(
            makePathListForJob(files), archiveFile, format, options,
            password?.toCharArray(), requireContext()
        )
        model(shellViewModel.activePane).selectFiles(files, false)
    }

    private fun currentSevenZArchiveRoot(pane: PaneId): Path? {
        val current = model(pane).currentPath
        if (!current.isArchivePath || current.nameCount != 0) return null
        return currentSevenZArchiveFile(pane)
    }

    private fun currentSevenZArchiveFile(pane: PaneId): Path? {
        val current = model(pane).currentPath
        if (!current.isArchivePath) return null
        val archive = runCatching { current.archiveFile }.getOrNull() ?: return null
        val name = archive.fileName?.toString()?.lowercase(Locale.ROOT) ?: return null
        return archive.takeIf { name.endsWith(".7z") }
    }

    private fun updateArchiveEditCapability(pane: PaneId) {
        val archive = currentSevenZArchiveFile(pane)
        if (archive == archiveEditProbePaths[pane]) return
        archiveEditProbePaths[pane] = archive
        archiveEditCapabilities[pane] = false
        if (archive == null) return
        val context = requireContext()
        lifecycleScope.launch {
            val editable = withContext(Dispatchers.IO) {
                if (!(archive.isLinuxPath || archive.isDocumentPath) || archive.fileSystem.isReadOnly) {
                    false
                } else {
                    val root = context.cacheDir.resolve("zipxtract-ui")
                    if (!root.mkdirs() && !root.isDirectory) false else {
                        val store = PrivateArchiveTempStore(root)
                        try {
                            val engine = ZipXtractArchiveEngine(store)
                            val probe = engine.probe(PathArchiveSource(archive))
                            ArchiveEditPolicy.canUpdate7z(probe, true) &&
                                engine.list(PathArchiveSource(archive)).none { it.encrypted }
                        } catch (_: Throwable) {
                            false
                        } finally {
                            store.close()
                        }
                    }
                }
            }
            if (isAdded && archive == archiveEditProbePaths[pane]) {
                archiveEditCapabilities[pane] = editable
                invalidateOptionsMenu()
            }
        }
    }

    private val archiveAdditionFileLauncher = registerForActivityResult(
        FileListActivity.OpenFileContract(), ::onArchiveAdditionResult
    )

    private val archiveAdditionFolderLauncher = registerForActivityResult(
        FileListActivity.OpenDirectoryContract(), ::onArchiveAdditionFolderResult
    )

    private fun chooseArchiveAddition(pane: PaneId) {
        if (currentSevenZArchiveRoot(pane) == null) return
        archiveAdditionPane = pane
        archiveAdditionFileLauncher.launch(listOf(MimeType.ANY))
    }

    private fun chooseArchiveAdditionFolder(pane: PaneId) {
        if (currentSevenZArchiveRoot(pane) == null) return
        archiveAdditionPane = pane
        archiveAdditionFolderLauncher.launch(null)
    }

    private fun onArchiveAdditionResult(path: Path?) {
        val pane = archiveAdditionPane ?: return
        archiveAdditionPane = null
        val archive = currentSevenZArchiveRoot(pane) ?: return
        if (path != null) FileJobService.update7z(archive, listOf(path), emptySet(), requireContext())
    }

    private fun onArchiveAdditionFolderResult(path: Path?) {
        val pane = archiveAdditionPane ?: return
        archiveAdditionPane = null
        val archive = currentSevenZArchiveRoot(pane) ?: return
        if (path != null) FileJobService.update7z(archive, listOf(path), emptySet(), requireContext())
    }

    private fun ensureStorageAccess() {
        if (leftViewModel.isStorageAccessRequested) return
        if (Environment::class.supportsExternalStorageManager()) {
            if (!Environment.isExternalStorageManager()) {
                ShowRequestAllFilesAccessRationaleDialogFragment.show(this)
                leftViewModel.isStorageAccessRequested = true
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M &&
            checkSelfPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE) !=
            PackageManager.PERMISSION_GRANTED
        ) {
            if (shouldShowRequestPermissionRationale(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                ShowRequestStoragePermissionRationaleDialogFragment.show(this)
            } else {
                requestStoragePermissionLauncher.launch(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
            }
            leftViewModel.isStorageAccessRequested = true
        }
    }

    override fun onShowRequestAllFilesAccessRationaleResult(shouldRequest: Boolean) {
        if (shouldRequest) {
            requestAllFilesAccessLauncher.launch(Unit)
        } else {
            leftViewModel.isStorageAccessRequested = false
        }
    }

    private fun onRequestAllFilesAccessResult(isGranted: Boolean) {
        leftViewModel.isStorageAccessRequested = false
        if (isGranted) {
            leftViewModel.reload()
            rightViewModel.reload()
        }
    }

    override fun onShowRequestStoragePermissionRationaleResult(shouldRequest: Boolean) {
        if (shouldRequest) {
            requestStoragePermissionLauncher.launch(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
        } else {
            leftViewModel.isStorageAccessRequested = false
        }
    }

    private fun onRequestStoragePermissionResult(isGranted: Boolean) {
        if (isGranted) {
            leftViewModel.isStorageAccessRequested = false
            leftViewModel.reload()
            rightViewModel.reload()
        } else if (shouldShowRequestPermissionRationale(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            ShowRequestStoragePermissionRationaleDialogFragment.show(this)
        } else {
            ShowRequestStoragePermissionInSettingsRationaleDialogFragment.show(this)
        }
    }

    override fun onShowRequestStoragePermissionInSettingsRationaleResult(shouldRequest: Boolean) {
        if (shouldRequest) {
            requestStoragePermissionInSettingsLauncher.launch(Unit)
        } else {
            leftViewModel.isStorageAccessRequested = false
        }
    }

    private fun onRequestStoragePermissionInSettingsResult(isGranted: Boolean) {
        leftViewModel.isStorageAccessRequested = false
        if (isGranted) {
            leftViewModel.reload()
            rightViewModel.reload()
        }
    }

    private fun ensureNotificationPermission() {
        if (leftViewModel.isNotificationPermissionRequested || Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU) return
        if (checkSelfPermission(android.Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            if (shouldShowRequestPermissionRationale(android.Manifest.permission.POST_NOTIFICATIONS)) {
                ShowRequestNotificationPermissionRationaleDialogFragment.show(this)
            } else {
                requestNotificationPermissionLauncher.launch(android.Manifest.permission.POST_NOTIFICATIONS)
            }
            leftViewModel.isNotificationPermissionRequested = true
        }
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    override fun onShowRequestNotificationPermissionRationaleResult(shouldRequest: Boolean) {
        if (shouldRequest) {
            requestNotificationPermissionLauncher.launch(android.Manifest.permission.POST_NOTIFICATIONS)
        } else {
            leftViewModel.isNotificationPermissionRequested = false
        }
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    private fun onRequestNotificationPermissionResult(isGranted: Boolean) {
        if (isGranted) {
            leftViewModel.isNotificationPermissionRequested = false
        } else if (shouldShowRequestPermissionRationale(android.Manifest.permission.POST_NOTIFICATIONS)) {
            ShowRequestNotificationPermissionRationaleDialogFragment.show(this)
        } else {
            ShowRequestNotificationPermissionInSettingsRationaleDialogFragment.show(this)
        }
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    override fun onShowRequestNotificationPermissionInSettingsRationaleResult(shouldRequest: Boolean) {
        if (shouldRequest) {
            requestNotificationPermissionInSettingsLauncher.launch(Unit)
        } else {
            leftViewModel.isNotificationPermissionRequested = false
        }
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    private fun onRequestNotificationPermissionInSettingsResult(isGranted: Boolean) {
        if (isGranted) leftViewModel.isNotificationPermissionRequested = false
    }

    private class RequestAllFilesAccessContract : ActivityResultContract<Unit, Boolean>() {
        @RequiresApi(Build.VERSION_CODES.R)
        override fun createIntent(context: Context, input: Unit): Intent =
            Environment::class.createManageAppAllFilesAccessPermissionIntent(context.packageName)

        @RequiresApi(Build.VERSION_CODES.R)
        override fun parseResult(resultCode: Int, intent: Intent?): Boolean =
            Environment.isExternalStorageManager()
    }

    private class RequestPermissionInSettingsContract(private val permissionName: String) :
        ActivityResultContract<Unit, Boolean>() {
        override fun createIntent(context: Context, input: Unit): Intent = Intent(
            android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS,
            Uri.fromParts("package", context.packageName, null)
        )

        override fun parseResult(resultCode: Int, intent: Intent?): Boolean =
            application.checkSelfPermissionCompat(permissionName) == PackageManager.PERMISSION_GRANTED
    }
}
