/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.ftpserver

import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.IBinder
import androidx.annotation.WorkerThread
import androidx.core.content.ContextCompat
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import me.zhanghai.android.files.compat.mainExecutorCompat
import me.zhanghai.android.files.provider.archive.isArchivePath
import me.zhanghai.android.files.provider.common.checkAccess
import me.zhanghai.android.files.provider.common.readAttributes
import me.zhanghai.android.files.provider.document.documentTreeUri
import me.zhanghai.android.files.filelist.isLocalPath
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.util.WakeWifiLock
import me.zhanghai.android.files.util.showToast
import me.zhanghai.android.files.util.valueCompat
import java8.nio.file.AccessMode
import java8.nio.file.LinkOption
import java8.nio.file.attribute.BasicFileAttributes
import java.util.concurrent.Executors
import java.util.concurrent.Future

/** User-controlled FTP listener backed by the embedded provider filesystem. */
class FtpServerService : Service() {
    @Volatile
    private var state = State.STOPPED
        set(value) {
            field = value
            _stateLiveData.postValue(value)
        }

    private lateinit var wakeWifiLock: WakeWifiLock
    private lateinit var notification: FtpServerNotification
    private val executorService = Executors.newSingleThreadExecutor()
    private val serverLock = Any()
    @Volatile
    private var server: FtpServer? = null
    /** Startup task, retained so volume teardown can wait for validation/startup to finish. */
    private var startTask: Future<*>? = null
    @Volatile
    private var runningRoot: FtpShareRootSnapshot? = null
    @Volatile
    private var runningConfig: FtpServerConfigSnapshot? = null

    override fun onCreate() {
        super.onCreate()
        instance = this
        wakeWifiLock = WakeWifiLock(FtpServerService::class.java.simpleName)
        notification = FtpServerNotification(this)
    }

    override fun onBind(intent: Intent): IBinder? = null

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        val requestedRoot = FtpShareRootStore.takePendingStart()
        if (requestedRoot == null && FtpShareRootStore.consumeCancelledPendingStart()) {
            // The start intent may already be queued when a selected unlocked
            // volume is locked. Do not fall back to the persisted directory.
            stopSelfResult(startId)
            return START_NOT_STICKY
        }
        val startRoot = requestedRoot ?: FtpShareRootStore.current()
        if (state == State.STOPPED) {
            // A sticky restart has no pending in-process selection. It can only
            // restore a persisted ordinary or Root path; unlocked selections
            // are deliberately non-sticky.
            executeStart(startRoot)
        }
        // If a second start intent arrives while the listener is already
        // running, retain the restart policy of the immutable root that is
        // actually serving rather than the newly selected (and ignored) root.
        val effectiveRoot = runningRoot ?: startRoot
        return if (effectiveRoot.runtimeRootId != null) START_NOT_STICKY else START_STICKY
    }

    override fun onDestroy() {
        stopSynchronously()
        executorService.shutdownNow()
        if (instance === this) instance = null
        super.onDestroy()
    }

    private fun executeStart(root: FtpShareRootSnapshot) {
        synchronized(serverLock) {
            if (state == State.STARTING || state == State.RUNNING) return
            // Publish STARTING before foreground setup so a concurrent volume
            // close can observe and cancel a startup that has not submitted
            // its worker yet.
            runningRoot = root
            state = State.STARTING
        }
        wakeWifiLock.isAcquired = true
        try {
            notification.startForeground()
        } catch (error: RuntimeException) {
            wakeWifiLock.isAcquired = false
            synchronized(serverLock) {
                if (state == State.STARTING) {
                    runningRoot = null
                    state = State.STOPPED
                }
            }
            throw error
        }
        synchronized(serverLock) {
            if (state != State.STARTING) {
                // stopSynchronously() won the race while the notification was
                // being built. It already cleared the service state, but it
                // may have run before startForeground(); clean up this late
                // foreground registration as well.
                notification.stopForeground()
                wakeWifiLock.isAcquired = false
                return
            }
            // Submit while holding the same lock used by doStart and
            // stopSynchronously. This makes the Future visible before a
            // worker can begin and lets teardown wait without racing native
            // volume closure.
            startTask = executorService.submit { doStart(root) }
        }
    }

    private fun onStartError(exception: Exception) {
        if (state == State.STOPPING || state == State.STOPPED) return
        state = State.STOPPED
        runningRoot = null
        runningConfig = null
        showToast(exception.message ?: exception.toString())
        notification.stopForeground()
        wakeWifiLock.isAcquired = false
        stopSelf()
    }

    private fun stopSynchronously(): Boolean {
        FtpShareRootStore.cancelPendingStart()
        val currentServer: FtpServer?
        val currentStartTask: Future<*>?
        synchronized(serverLock) {
            if (state == State.STOPPED && server == null && startTask == null) return false
            state = State.STOPPING
            currentServer = server
            currentStartTask = startTask
            server = null
            startTask = null
            runningRoot = null
            runningConfig = null
        }
        return try {
            // A startup worker may still be validating a Root or unlocked
            // provider path. Wait until it has observed STOPPING and exited
            // before the caller closes the backing volume's native session.
            try {
                currentStartTask?.get()
            } catch (error: InterruptedException) {
                Thread.currentThread().interrupt()
                error.printStackTrace()
            } catch (error: Exception) {
                error.printStackTrace()
            }
            runCatching { currentServer?.stop() }
                .onFailure { it.printStackTrace() }
            true
        } finally {
            notification.stopForeground()
            wakeWifiLock.isAcquired = false
            state = State.STOPPED
            stopSelf()
        }
    }

    @WorkerThread
    private fun doStart(root: FtpShareRootSnapshot) {
        synchronized(serverLock) {
            if (server != null || state != State.STARTING) return
        }
        var ftpServer: FtpServer? = null
        try {
            require(FtpShareRootStore.isLive(root)) {
                "The selected unlocked volume is no longer available"
            }
            // Reject archive and network providers before any provider call so
            // an invalid persisted URI cannot trigger a remote connection.
            require(isShareableHomeDirectory(root)) {
                "FTP cannot share remote storage or an archive"
            }
            val rootAttributes = root.path.readAttributes(
                BasicFileAttributes::class.java, LinkOption.NOFOLLOW_LINKS
            )
            require(rootAttributes.isDirectory) { "FTP shared root must be an existing directory" }
            // Use throwing provider calls instead of Files.isReadable/isWritable,
            // which swallow RootService failures and make a denied Root request
            // look like an ordinary missing directory.
            root.path.checkAccess(AccessMode.READ)
            require(FtpPathPolicy.isSafe(root.path, root.path)) {
                "FTP shared root cannot be a symbolic link"
            }
            val anonymous = Settings.FTP_SERVER_ANONYMOUS_LOGIN.valueCompat
            val username = if (anonymous) USERNAME_ANONYMOUS else Settings.FTP_SERVER_USERNAME.valueCompat
            val password = if (anonymous) null else Settings.FTP_SERVER_PASSWORD.valueCompat
            require(username.isNotBlank()) { "FTP username cannot be empty" }
            require(anonymous || !password.isNullOrBlank()) {
                "Configure a non-empty FTP password or enable anonymous login explicitly"
            }
            val port = Settings.FTP_SERVER_PORT.valueCompat.also(::validatePort)
            val requestedWritable = Settings.FTP_SERVER_WRITABLE.valueCompat
            if (requestedWritable && !root.isReadOnly) {
                root.path.checkAccess(AccessMode.WRITE)
            }
            val ftpWritable = requestedWritable && !root.isReadOnly
            runningConfig = FtpServerConfigSnapshot(
                port = port,
                username = username,
                password = password,
                anonymous = anonymous,
            )
            ftpServer = FtpServer(
                username,
                password,
                port,
                root.path,
                ftpWritable,
                anonymous,
            )
            synchronized(serverLock) {
                if (state != State.STARTING) {
                    runningConfig = null
                    return
                }
                server = checkNotNull(ftpServer)
                checkNotNull(ftpServer).start()
            }
        } catch (e: Exception) {
            e.printStackTrace()
            runCatching { ftpServer?.stop() }.onFailure { it.printStackTrace() }
            synchronized(serverLock) {
                server = null
                runningConfig = null
            }
            mainExecutorCompat.execute { onStartError(e) }
            return
        }
        postState(State.RUNNING)
    }

    private fun isShareableHomeDirectory(root: FtpShareRootSnapshot): Boolean {
        val path = root.path
        if (path.isArchivePath) return false
        if (root.runtimeRootId != null) {
            // Unlocked container DocumentsProvider paths are only valid for
            // this process and are accepted after the liveness check above.
            return path.toUri().scheme == "document"
                && runCatching { path.documentTreeUri.authority?.endsWith(".unlocked") == true }
                    .getOrDefault(false)
        }
        return when (path.toUri().scheme) {
            "file" -> true
            // A persisted SAF tree is allowed only when it is one of the
            // local providers. Remote document providers must not become an
            // FTP bridge through an otherwise innocuous `document:` URI.
            "document" -> path.isLocalPath
                && runCatching { !path.documentTreeUri.authority.orEmpty().endsWith(".unlocked") }
                    .getOrDefault(false)
            else -> false
        }
    }

    @WorkerThread
    private fun postState(nextState: State) {
        mainExecutorCompat.execute {
            // A user stop may win the race with the worker finishing
            // FtpServer.start(); never publish RUNNING after STOPPED.
            if (nextState != State.RUNNING || state == State.STARTING) {
                state = nextState
            }
        }
    }

    companion object {
        const val USERNAME_ANONYMOUS = "anonymous"
        const val MIN_PORT = 1
        const val MAX_PORT = 65535

        @Volatile
        private var instance: FtpServerService? = null
        private val _stateLiveData = MutableLiveData(State.STOPPED)
        val stateLiveData: LiveData<State>
            get() = _stateLiveData

        fun start(context: Context) {
            FtpShareRootStore.requestStart()
            try {
                ContextCompat.startForegroundService(
                    context, Intent(context, FtpServerService::class.java)
                )
            } catch (error: RuntimeException) {
                // Background-start restrictions can reject the request before
                // Service.onStartCommand consumes the snapshot. Do not leave
                // a stale unlocked-volume root queued for a later start.
                FtpShareRootStore.cancelPendingStart()
                throw error
            }
        }

        fun stop(context: Context) {
            FtpShareRootStore.cancelPendingStart()
            context.stopService(Intent(context, FtpServerService::class.java))
        }

        /** Stops a running listener before its backing volume is closed. */
        fun stopAndWait(context: Context): Boolean {
            val service = instance
            if (service != null) return service.stopSynchronously()
            FtpShareRootStore.cancelPendingStart()
            return context.stopService(Intent(context, FtpServerService::class.java))
        }

        /** Called synchronously while an unlocked-volume root is being revoked. */
        fun stopIfSharing(runtimeRootId: String): Boolean {
            val service = instance ?: return false
            val sharesRoot = synchronized(service.serverLock) {
                service.runningRoot?.runtimeRootId == runtimeRootId
            }
            return sharesRoot && service.stopSynchronously()
        }

        /** Immutable connection settings captured for the running listener. */
        internal fun connectionSnapshot(): FtpServerConfigSnapshot? = instance?.runningConfig

        fun isValidPort(port: Int): Boolean = port in MIN_PORT..MAX_PORT

        @Throws(IllegalArgumentException::class)
        fun validatePort(port: Int) {
            require(isValidPort(port)) {
                "FTP port must be between $MIN_PORT and $MAX_PORT"
            }
        }

        fun toggle(context: Context) {
            when (_stateLiveData.value ?: State.STOPPED) {
                State.STARTING, State.STOPPING -> {}
                State.RUNNING -> stop(context)
                State.STOPPED -> start(context)
            }
        }
    }

    enum class State {
        STARTING,
        RUNNING,
        STOPPING,
        STOPPED,
    }
}

/** Settings used by the currently running Apache listener, never persisted. */
internal data class FtpServerConfigSnapshot(
    val port: Int,
    val username: String,
    val password: String?,
    val anonymous: Boolean,
)
