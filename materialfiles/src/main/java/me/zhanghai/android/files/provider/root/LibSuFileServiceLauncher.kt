package me.zhanghai.android.files.provider.root

/** Root integration is intentionally unavailable in the embedded build. */
object LibSuFileServiceLauncher {
    fun launchService(): Nothing = error("Root access is disabled in the embedded file manager")
}
