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
        val customThemeRes = getCustomThemeRes(baseThemeRes, activity)
        activity.setThemeCompat(customThemeRes)
    }

    fun sync() {
        for ((activity, baseThemeRes) in activityBaseThemes) {
            val currentThemeRes = activity.themeResIdCompat
            val customThemeRes = getCustomThemeRes(baseThemeRes, activity)
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
        val customThemeBaseName = if (Settings.MATERIAL_DESIGN_3.valueCompat) {
            val material3Base = if (baseThemeName.contains(material3ThemeName)) baseThemeName
            else baseThemeName.replaceFirst(defaultThemeName, material3ThemeName)
            // Material 3's DynamicColors parent otherwise ignores the app's
            // selected accent and leaves all surfaces tied to the system seed.
            // Keep the existing shared black overlay, while normal themes use
            // the matching low-saturation surface overlay below.
            if (Settings.BLACK_NIGHT_MODE.valueCompat ||
                Settings.THEME_COLOR_SOURCE.valueCompat == ThemeColorSource.DYNAMIC) {
                material3Base
            } else {
                val themeColorName =
                    resources.getResourceEntryName(Settings.THEME_COLOR.valueCompat.resourceId)
                "$material3Base.$themeColorName"
            }
        } else {
            val legacyBaseName = if (baseThemeName.contains(material3ThemeName)) {
                baseThemeName.replaceFirst(material3ThemeName, defaultThemeName)
            } else {
                baseThemeName
            }
            val themeColorName =
                resources.getResourceEntryName(Settings.THEME_COLOR.valueCompat.resourceId)
            "$legacyBaseName.$themeColorName"
        }
        val customThemeName = customThemeBaseName +
            if (Settings.BLACK_NIGHT_MODE.valueCompat) ".Black" else ""
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
