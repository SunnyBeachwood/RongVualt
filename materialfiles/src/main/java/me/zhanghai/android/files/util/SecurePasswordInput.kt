/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 */

package me.zhanghai.android.files.util

import android.os.Build
import android.view.inputmethod.EditorInfo
import android.widget.EditText

/** Prevent password fields from being learned by the IME or exposed in extract-mode UI. */
fun EditText.configureSecurePasswordInput() {
    imeOptions = imeOptions or EditorInfo.IME_FLAG_NO_EXTRACT_UI or
        EditorInfo.IME_FLAG_NO_PERSONALIZED_LEARNING
    importantForAutofill = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
        android.view.View.IMPORTANT_FOR_AUTOFILL_NO_EXCLUDE_DESCENDANTS
    } else {
        android.view.View.IMPORTANT_FOR_AUTOFILL_NO
    }
    setSaveEnabled(false)
}
