package me.zhanghai.android.files.provider.root

/** Shizuku/Sui integration is intentionally unavailable in the embedded build. */
object SuiFileServiceLauncher {
    fun isSuiAvailable(): Boolean = false
    fun launchService(): Nothing = error("Privileged access is disabled in the embedded file manager")
}
