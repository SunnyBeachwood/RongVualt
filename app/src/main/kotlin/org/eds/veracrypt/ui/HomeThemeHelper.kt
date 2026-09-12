package org.eds.veracrypt.ui

import androidx.annotation.StyleRes
import androidx.appcompat.app.AppCompatActivity
import me.zhanghai.android.files.compat.setThemeCompat
import me.zhanghai.android.files.R as MaterialFilesR
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.theme.custom.CustomThemeHelper
import me.zhanghai.android.files.util.valueCompat

/** Applies the embedded file manager's current theme policy to RongVault's home activity. */
internal object HomeThemeHelper {
    data class Signature(
        @StyleRes val themeRes: Int,
        val nightMode: Int,
    )

    fun apply(activity: AppCompatActivity): Signature {
        val signature = signature(activity)
        activity.delegate.localNightMode = signature.nightMode
        activity.setThemeCompat(signature.themeRes)
        return signature
    }

    fun signature(activity: AppCompatActivity): Signature = Signature(
        CustomThemeHelper.resolveTheme(
            MaterialFilesR.style.Theme_MaterialFiles_Material3, activity
        ),
        Settings.NIGHT_MODE.valueCompat.value,
    )
}
