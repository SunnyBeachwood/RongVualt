/*
 * Copyright (c) 2026 RongVualt contributors
 * All Rights Reserved.
 */

package me.zhanghai.android.files.settings

import android.os.Bundle
import android.text.InputType
import android.text.TextWatcher
import android.text.Editable
import android.view.View
import android.widget.EditText
import android.widget.SeekBar
import android.widget.TextView
import me.zhanghai.android.files.R
import me.zhanghai.android.files.ui.MaterialPreferenceDialogFragmentCompat
import me.zhanghai.android.files.util.layoutInflater

class FileListFontSizeDialogFragment : MaterialPreferenceDialogFragmentCompat() {
    private var pendingSp = FileListFontSizePreference.FileListFontSizeDefaults.DEFAULT_SP
    private var sizeEditText: EditText? = null
    private var sizeSeekBar: SeekBar? = null
    private var updatingViews = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (savedInstanceState == null) {
            pendingSp = (preference as FileListFontSizePreference).fontSizeSp
        } else {
            pendingSp = savedInstanceState.getInt(KEY_PENDING_SP, pendingSp)
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        outState.putInt(KEY_PENDING_SP, pendingSp)
        super.onSaveInstanceState(outState)
    }

    override fun onCreateDialogView(context: android.content.Context): View =
        context.layoutInflater.inflate(R.layout.file_list_font_size_dialog, null)

    override fun onBindDialogView(view: View) {
        super.onBindDialogView(view)
        sizeEditText = view.findViewById<EditText>(R.id.fileListFontSizeEdit)
        sizeSeekBar = view.findViewById<SeekBar>(R.id.fileListFontSizeSeek)
        view.findViewById<TextView>(R.id.fileListFontSizeRange).text = view.context.getString(
            R.string.settings_file_list_font_size_range,
            FileListFontSizePreference.FileListFontSizeDefaults.MIN_SP,
            FileListFontSizePreference.FileListFontSizeDefaults.MAX_SP,
        )
        sizeSeekBar!!.max = FileListFontSizePreference.FileListFontSizeDefaults.MAX_SP -
            FileListFontSizePreference.FileListFontSizeDefaults.MIN_SP
        sizeSeekBar!!.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                if (fromUser) updatePending(
                    FileListFontSizePreference.FileListFontSizeDefaults.MIN_SP + progress
                )
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) = Unit
            override fun onStopTrackingTouch(seekBar: SeekBar) = Unit
        })
        sizeEditText!!.inputType = InputType.TYPE_CLASS_NUMBER
        sizeEditText!!.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) = Unit
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) = Unit
            override fun afterTextChanged(s: Editable?) {
                if (!updatingViews) s?.toString()?.toIntOrNull()?.let { pendingSp = it }
            }
        })
        updatePending(pendingSp)
    }

    private fun updatePending(value: Int) {
        pendingSp = value.coerceIn(
            FileListFontSizePreference.FileListFontSizeDefaults.MIN_SP,
            FileListFontSizePreference.FileListFontSizeDefaults.MAX_SP,
        )
        updatingViews = true
        sizeSeekBar?.progress = pendingSp - FileListFontSizePreference.FileListFontSizeDefaults.MIN_SP
        sizeEditText?.setText(pendingSp.toString())
        sizeEditText?.setSelection(sizeEditText!!.length())
        updatingViews = false
    }

    override fun onDialogClosed(positiveResult: Boolean) {
        if (positiveResult) {
            (preference as FileListFontSizePreference).fontSizeSp = pendingSp
        }
        sizeEditText = null
        sizeSeekBar = null
    }

    companion object {
        private const val KEY_PENDING_SP = "pending_sp"
    }
}
