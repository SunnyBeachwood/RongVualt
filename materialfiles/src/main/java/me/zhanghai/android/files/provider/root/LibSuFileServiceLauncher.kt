/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.provider.root

import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.os.IBinder
import com.topjohnwu.superuser.NoShellException
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ipc.RootService
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.TimeoutCancellationException
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withTimeout
import me.zhanghai.android.files.BuildConfig
import me.zhanghai.android.files.provider.remote.IRemoteFileService
import me.zhanghai.android.files.provider.remote.RemoteFileServiceInterface
import me.zhanghai.android.files.provider.remote.RemoteFileSystemException
import me.zhanghai.android.files.util.createIntent
import java.io.IOException
import java.util.concurrent.TimeUnit
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException

object LibSuFileServiceLauncher {
    private val lock = Any()

    init {
        Shell.enableVerboseLogging = BuildConfig.DEBUG
        Shell.setDefaultBuilder(
            Shell.Builder.create()
                .setInitializers(LibSuShellInitializer::class.java)
                .setFlags(Shell.FLAG_MOUNT_MASTER or Shell.FLAG_REDIRECT_STDERR)
                .setTimeout(TimeUnit.MILLISECONDS.toSeconds(RootFileService.TIMEOUT_MILLIS))
        )
    }

    fun isSuAvailable(): Boolean =
        try {
            val process = Runtime.getRuntime().exec(arrayOf("su", "--version"))
            process.inputStream.close()
            process.errorStream.close()
            if (!process.waitFor(2, TimeUnit.SECONDS)) {
                process.destroy()
                false
            } else {
                // Some su implementations do not implement --version (and
                // return a non-zero status) even though they can still grant
                // a root shell.  Treat a completed process as a candidate and
                // let Shell.getShell() report an actual denial below.
                true
            }
        } catch (e: InterruptedException) {
            Thread.currentThread().interrupt()
            false
        } catch (e: SecurityException) {
            false
        } catch (e: IOException) {
            false
        }

    @Throws(RemoteFileSystemException::class)
    fun launchService(): IRemoteFileService {
        synchronized(lock) {
            if (!isSuAvailable()) {
                throw RootAccessErrors.unavailable()
            }
            return try {
                runBlocking {
                    try {
                        withTimeout(RootFileService.TIMEOUT_MILLIS) {
                            suspendCancellableCoroutine<Unit> { continuation ->
                                Shell.EXECUTOR.submit {
                                    try {
                                        Shell.getShell()
                                        if (continuation.isActive) continuation.resume(Unit)
                                    } catch (e: NoShellException) {
                                        if (continuation.isActive) {
                                            continuation.resumeWithException(RootAccessErrors.denied(e))
                                        }
                                    }
                                }
                            }
                            suspendCancellableCoroutine { continuation ->
                                val intent = LibSuFileService::class.createIntent()
                                var bound = false
                                val connection = object : ServiceConnection {
                                    override fun onServiceConnected(
                                        name: ComponentName,
                                        service: IBinder
                                    ) {
                                        if (continuation.isActive) {
                                            continuation.resume(
                                                IRemoteFileService.Stub.asInterface(service)
                                            )
                                        }
                                    }

                                    override fun onServiceDisconnected(name: ComponentName) {
                                        if (continuation.isActive) {
                                            continuation.resumeWithException(RootAccessErrors.disconnected())
                                        }
                                    }

                                    override fun onBindingDied(name: ComponentName) {
                                        if (continuation.isActive) {
                                            continuation.resumeWithException(RootAccessErrors.disconnected())
                                        }
                                    }

                                    override fun onNullBinding(name: ComponentName) {
                                        if (continuation.isActive) {
                                            continuation.resumeWithException(RootAccessErrors.disconnected())
                                        }
                                    }
                                }
                                launch(Dispatchers.Main.immediate) {
                                    try {
                                        RootService.bind(intent, connection)
                                        bound = true
                                        if (!continuation.isActive) RootService.unbind(connection)
                                    } catch (e: Throwable) {
                                        if (continuation.isActive) {
                                            continuation.resumeWithException(RootAccessErrors.denied(e))
                                        }
                                    }
                                    continuation.invokeOnCancellation {
                                        if (bound) {
                                            launch(Dispatchers.Main.immediate) {
                                                runCatching { RootService.unbind(connection) }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } catch (e: TimeoutCancellationException) {
                        throw RootAccessErrors.timedOut(e)
                    }
                }
            } catch (e: InterruptedException) {
                Thread.currentThread().interrupt()
                throw RootAccessErrors.timedOut(e)
            }
        }
    }
}

private class LibSuShellInitializer : Shell.Initializer() {
    override fun onInit(context: Context, shell: Shell): Boolean = shell.isRoot
}

class LibSuFileService : RootService() {
    override fun onCreate() {
        super.onCreate()
        RootFileService.main()
    }

    override fun onBind(intent: Intent): IBinder = RemoteFileServiceInterface()
}
