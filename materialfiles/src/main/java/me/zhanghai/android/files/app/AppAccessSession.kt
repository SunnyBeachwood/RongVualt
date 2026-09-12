package me.zhanghai.android.files.app

/** Process-local authorization boundary for RongVault-owned Activities. */
object AppAccessSession {
    @Volatile
    private var authorized = false

    fun authorize() { authorized = true }

    fun lock() { authorized = false }

    fun isAuthorized(): Boolean = authorized
}
