/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package androidx.appcompat.app

import android.annotation.SuppressLint
import android.content.Context

class AppCompatDelegateCompat private constructor() {
    companion object {
        @JvmStatic
        @SuppressLint("RestrictedApi")
        @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
        fun mapNightMode(delegate: AppCompatDelegate, context: Context, mode: Int): Int =
            (delegate as AppCompatDelegateImpl).mapNightMode(context, mode)
    }
}
