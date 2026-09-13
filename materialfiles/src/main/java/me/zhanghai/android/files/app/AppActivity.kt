/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.app

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import me.zhanghai.android.files.theme.custom.CustomThemeHelper
import me.zhanghai.android.files.theme.night.NightModeHelper

abstract class AppActivity : AppCompatActivity() {
    private var isDelegateCreated = false

    override fun getDelegate(): AppCompatDelegate {
        val delegate = super.getDelegate()

        if (!isDelegateCreated) {
            isDelegateCreated = true
            NightModeHelper.apply(this)
        }
        return delegate
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        // Resolve the app-local night policy before looking up the custom
        // theme, so a night-mode recreation reads values-night resources.
        delegate
        CustomThemeHelper.apply(this)

        super.onCreate(savedInstanceState)
    }

    override fun onStart() {
        super.onStart()
        // Embedded file-manager screens can expose live unlocked-volume
        // roots, so they cannot provide an alternate path around app unlock.
        if (!AppAccessSession.isAuthorized()) {
            AppAccessSession.requestAuthorization(this, intent)
            finish()
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        if (!super.onSupportNavigateUp()) {
            finish()
        }
        return true
    }
}
