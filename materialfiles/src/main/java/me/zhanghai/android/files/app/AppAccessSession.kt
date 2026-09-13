package me.zhanghai.android.files.app

import android.app.Activity
import android.content.Intent

/** Process-local authorization boundary for RongVault-owned Activities. */
object AppAccessSession {
    const val EXTRA_RESUME_ACTIVITY_INTENT =
        "me.zhanghai.android.files.app.extra.RESUME_ACTIVITY_INTENT"

    @Volatile
    private var authorized = false

    fun authorize() { authorized = true }

    fun lock() { authorized = false }

    fun isAuthorized(): Boolean = authorized

    /**
     * Returns to RongVault's authentication boundary and preserves the protected
     * activity launch so it can be resumed after successful authentication.
     */
    fun requestAuthorization(activity: Activity, continuation: Intent) {
        val authorizationIntent = Intent()
            .setClassName(activity, "org.eds.veracrypt.ui.ContainerCatalogActivity")
            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP)
            .putExtra(EXTRA_RESUME_ACTIVITY_INTENT, Intent(continuation))
        activity.startActivity(authorizationIntent)
    }
}
