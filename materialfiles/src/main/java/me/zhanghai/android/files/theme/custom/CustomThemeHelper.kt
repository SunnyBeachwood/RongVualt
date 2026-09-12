/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.theme.custom

import android.app.Activity
import android.app.Application
import android.content.Context
import android.os.Bundle
import androidx.annotation.StyleRes
import androidx.appcompat.app.AppCompatActivity
import me.zhanghai.android.files.R
import me.zhanghai.android.files.compat.recreateCompat
import me.zhanghai.android.files.compat.setThemeCompat
import me.zhanghai.android.files.compat.themeResIdCompat
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.theme.night.NightModeHelper
import me.zhanghai.android.files.util.SimpleActivityLifecycleCallbacks
import me.zhanghai.android.files.util.valueCompat

object CustomThemeHelper {
    private val activityBaseThemes = mutableMapOf<Activity, Int>()

    fun initialize(application: Application) {
        application.registerActivityLifecycleCallbacks(object : SimpleActivityLifecycleCallbacks {
            override fun onActivityCreated(activity: Activity, savedInstanceState: Bundle?) {
                // Material Files is embedded in RongVault, so this process also contains host
                // activities that intentionally do not use Material Files' AppActivity base
                // class. Only activities registered by apply() participate in theme syncing.
                if (!activityBaseThemes.containsKey(activity)) return
            }

            override fun onActivityDestroyed(activity: Activity) {
                activityBaseThemes.remove(activity)
            }
        })
    }

    fun apply(activity: Activity) {
        val baseThemeRes = activity.themeResIdCompat
        activityBaseThemes[activity] = baseThemeRes
        val customThemeRes = resolveTheme(baseThemeRes, activity)
        activity.setThemeCompat(customThemeRes)
    }

    /**
     * Resolves the same theme resource used by embedded file-manager activities without
     * registering [context] for the file manager's lifecycle callbacks. Host activities use
     * this to share the user's palette while retaining their own lifecycle and access controls.
     */
    @StyleRes
    fun resolveTheme(@StyleRes baseThemeRes: Int, context: Context): Int =
        getCustomThemeRes(baseThemeRes, context)

    fun sync() {
        for ((activity, baseThemeRes) in activityBaseThemes) {
            val currentThemeRes = activity.themeResIdCompat
            val customThemeRes = resolveTheme(baseThemeRes, activity)
            if (currentThemeRes != customThemeRes) {
                // Ignore ".Black" theme changes when not in night mode.
                if (!NightModeHelper.isInNightMode(activity as AppCompatActivity)
                    && isBlackThemeChange(currentThemeRes, customThemeRes, activity)) {
                    continue
                }
                if (activity is OnThemeChangedListener) {
                    (activity as OnThemeChangedListener).onThemeChanged(customThemeRes)
                } else {
                    activity.recreateCompat()
                }
            }
        }
    }

    private fun getCustomThemeRes(@StyleRes baseThemeRes: Int, context: Context): Int {
        val resources = context.resources
        val baseThemeName = resources.getResourceName(baseThemeRes)
        val defaultThemeName = resources.getResourceEntryName(R.style.Theme_MaterialFiles)
        val material3ThemeName =
            resources.getResourceEntryName(R.style.Theme_MaterialFiles_Material3)
        // FileListActivity is declared with the Material 3 base theme. Avoid
        // producing `...Material3.Material3` when the setting is enabled and
        // map that base back to the legacy family when an existing user has
        // explicitly disabled Material 3.
        val legacyBaseName = if (baseThemeName.contains(material3ThemeName)) {
            baseThemeName.replaceFirst(material3ThemeName, defaultThemeName)
        } else baseThemeName
        val material3BaseName = if (baseThemeName.contains(material3ThemeName)) baseThemeName
            else baseThemeName.replaceFirst(defaultThemeName, material3ThemeName)
        val themeColorName =
            resources.getResourceEntryName(Settings.THEME_COLOR.valueCompat.resourceId)
        val customThemeName = when (Settings.APPEARANCE_MODE.valueCompat) {
            AppearanceMode.STANDARD -> "$legacyBaseName.$themeColorName"
            AppearanceMode.MATERIAL3 -> "$material3BaseName.$themeColorName"
            AppearanceMode.DYNAMIC -> "$legacyBaseName.Dynamic"
            AppearanceMode.BLACK -> "$legacyBaseName.$themeColorName.Black"
        }
        return resources.getIdentifier(customThemeName, null, null)
    }

    private fun isBlackThemeChange(
        @StyleRes themeRes1: Int,
        @StyleRes themeRes2: Int,
        context: Context
    ): Boolean {
        val resources = context.resources
        val themeName1 = resources.getResourceName(themeRes1)
        val themeName2 = resources.getResourceName(themeRes2)
        return themeName1 == "$themeName2.Black" || themeName2 == "$themeName1.Black"
    }

    interface OnThemeChangedListener {
        fun onThemeChanged(@StyleRes theme: Int)
    }
}
