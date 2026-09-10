/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.filelist

import android.app.Dialog
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.WindowManager
import android.widget.EditText
import android.widget.RadioGroup
import android.widget.ArrayAdapter
import android.widget.Spinner
import android.widget.AdapterView
import android.widget.TextView
import androidx.annotation.StringRes
import androidx.core.view.isGone
import androidx.core.view.isVisible
import androidx.fragment.app.Fragment
import com.google.android.material.textfield.TextInputEditText
import com.google.android.material.textfield.TextInputLayout
import kotlinx.parcelize.Parcelize
import me.zhanghai.android.files.R
import me.zhanghai.android.files.databinding.CreateArchiveDialogBinding
import me.zhanghai.android.files.databinding.NameDialogNameIncludeBinding
import me.zhanghai.android.files.util.ParcelableArgs
import me.zhanghai.android.files.util.args
import me.zhanghai.android.files.util.putArgs
import me.zhanghai.android.files.util.show
import me.zhanghai.android.files.util.takeIfNotEmpty
import me.zhanghai.android.libarchive.Archive
import org.eds.zipxtract.core.ArchiveCreateOptions
import org.eds.zipxtract.core.ArchiveEncryption
import org.eds.zipxtract.core.ArchiveFormat
import org.eds.zipxtract.core.TarCompression
import org.eds.zipxtract.core.ZipCompression

class CreateArchiveDialogFragment : FileNameDialogFragment() {
    private val args by args<Args>()

    override val binding: Binding
        get() = super.binding as Binding

    override val listener: Listener
        get() = super.listener as Listener

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        val dialog = super.onCreateDialog(savedInstanceState)
        // Keep the archive form at its full height when the keyboard opens; the
        // dialog is panned upward while its NestedScrollView remains usable.
        dialog.window?.setSoftInputMode(
            WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE or
                WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN
        )

        if (savedInstanceState == null) {
            val files = args.files
            var sourceName: String? = null
            if (files.size == 1) {
                sourceName = files.single().path.fileName.toString()
            } else {
                val parent = files.mapTo(mutableSetOf()) { it.path.parent }.singleOrNull()
                if (parent != null && parent.nameCount > 0) {
                    sourceName = parent.fileName.toString()
                }
            }
            initialArchiveSourceName = sourceName
        }
        val sourceParents = args.files.mapNotNull { it.path.parent }.distinct()
        val targetText = sourceParents.singleOrNull()?.toString()
            ?: getString(R.string.file_create_archive_target_multiple)
        binding.root.findViewById<TextView>(R.id.targetPathText).text =
            getString(R.string.file_create_archive_target, targetText)
        binding.typeGroup.setOnCheckedChangeListener { _, _ ->
            updatePasswordLayoutVisibility()
            updateAdvancedOptionVisibility()
            updateArchiveNameExtension()
        }
        binding.root.findViewById<com.google.android.material.button.MaterialButton>(R.id.advancedButton)
            .setOnClickListener {
                val advanced = binding.root.findViewById<View>(R.id.advancedLayout)
                advanced.isVisible = !advanced.isVisible
            }
        binding.root.findViewById<Spinner>(R.id.tarCompressionSpinner)
            .onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onItemSelected(
                parent: AdapterView<*>?,
                view: View?,
                position: Int,
                id: Long,
            ) {
                updateAdvancedOptionVisibility()
                updateArchiveNameExtension()
            }

            override fun onNothingSelected(parent: AdapterView<*>?) = Unit
        }
        configureSpinners()
        initialArchiveSourceName?.let { setArchiveName(it) }
        updatePasswordLayoutVisibility()
        updateAdvancedOptionVisibility()
        if (savedInstanceState?.getBoolean(KEY_ADVANCED) == true) {
            binding.root.findViewById<View>(R.id.advancedLayout).isVisible = true
        }
        return dialog
    }

    override fun onSaveInstanceState(outState: Bundle) {
        outState.putBoolean(
            KEY_ADVANCED,
            binding.root.findViewById<View>(R.id.advancedLayout).isVisible,
        )
        super.onSaveInstanceState(outState)
    }

    @StringRes
    override val titleRes: Int = R.string.file_create_archive_title

    override fun onInflateBinding(inflater: LayoutInflater): NameDialogFragment.Binding =
        Binding.inflate(inflater)

    override val name: String
        get() = normalizeArchiveName(super.name)

    private var initialArchiveSourceName: String? = null

    private fun selectedArchiveExtension(): String = when (val checkedId = binding.typeGroup.checkedRadioButtonId) {
        R.id.zipRadio -> "zip"
        R.id.tarXzRadio -> tarExtension()
        R.id.sevenZRadio -> "7z"
        else -> throw AssertionError(checkedId)
    }

    private fun archiveBaseName(value: String): String {
        val trimmed = value.trim()
        val lower = trimmed.lowercase()
        val known = ARCHIVE_EXTENSIONS.firstOrNull { lower.endsWith(".$it") }
        if (known != null) return trimmed.dropLast(known.length + 1)
        val dot = trimmed.lastIndexOf('.')
        return if (dot > 0) trimmed.substring(0, dot) else trimmed
    }

    private fun setArchiveName(sourceName: String) {
        val base = archiveBaseName(sourceName)
        val fullName = "$base.${selectedArchiveExtension()}"
        binding.nameEdit.setText(fullName)
        binding.nameEdit.setSelection(0, base.length.coerceAtMost(fullName.length))
    }

    private fun updateArchiveNameExtension() {
        val current = binding.nameEdit.text?.toString().orEmpty()
        if (current.isBlank()) return
        setArchiveName(current)
    }

    private fun normalizeArchiveName(value: String): String {
        val trimmed = value.trim()
        if (trimmed.isEmpty()) return trimmed
        val extension = selectedArchiveExtension()
        val suffix = ".${extension.lowercase()}"
        return if (trimmed.lowercase().endsWith(suffix)) {
            trimmed
        } else {
            "${archiveBaseName(trimmed)}.$extension"
        }
    }

    private val isPasswordSupported: Boolean
        get() = when (val checkedId = binding.typeGroup.checkedRadioButtonId) {
            R.id.zipRadio -> true
            R.id.tarXzRadio, R.id.sevenZRadio -> binding.typeGroup.checkedRadioButtonId == R.id.sevenZRadio
            else -> throw AssertionError(checkedId)
        }

    private fun updatePasswordLayoutVisibility() {
        binding.passwordLayout.isGone = !isPasswordSupported
    }

    private fun updateAdvancedOptionVisibility() {
        val type = binding.typeGroup.checkedRadioButtonId
        val zip = type == R.id.zipRadio
        val sevenZip = type == R.id.sevenZRadio
        val tar = type == R.id.tarXzRadio
        binding.root.findViewById<Spinner>(R.id.compressionSpinner).isVisible = zip
        binding.root.findViewById<View>(R.id.compressionLevelLabel).isVisible = zip
        binding.root.findViewById<Spinner>(R.id.compressionLevelSpinner).isVisible = zip
        binding.root.findViewById<View>(R.id.sevenZLevelLabel).isVisible = sevenZip
        binding.root.findViewById<Spinner>(R.id.sevenZLevelSpinner).isVisible = sevenZip
        binding.root.findViewById<Spinner>(R.id.encryptionSpinner).isVisible = zip
        binding.root.findViewById<Spinner>(R.id.tarCompressionSpinner).isVisible = tar
        binding.root.findViewById<View>(R.id.tarZstdLevelLabel).isVisible =
            tar && selectedSpinnerValue(R.id.tarCompressionSpinner) == "tar.zst"
        binding.root.findViewById<Spinner>(R.id.tarZstdLevelSpinner).isVisible =
            tar && selectedSpinnerValue(R.id.tarCompressionSpinner) == "tar.zst"
        binding.root.findViewById<View>(R.id.splitSizeLayout).isVisible = zip
        binding.root.findViewById<View>(R.id.sevenZThreadLayout).isVisible = sevenZip
        binding.root.findViewById<View>(R.id.sevenZSolidCheck).isVisible = sevenZip
    }

    private fun configureSpinners() {
        setSpinner(R.id.compressionSpinner, listOf("STORE", "DEFLATE"), 1)
        setSpinner(R.id.compressionLevelSpinner, (0..9).map(Int::toString), 5)
        setSpinner(R.id.sevenZLevelSpinner, listOf("0", "1", "3", "5", "7", "9"), 3)
        setSpinner(
            R.id.encryptionSpinner,
            listOf("None", "Zip Standard", "Zip Strong", "AES-128", "AES-256"),
            0,
        )
        // Selecting TAR starts with a plain tar archive; compression is an
        // explicit advanced choice, matching the documented defaults.
        setSpinner(R.id.tarCompressionSpinner, listOf("tar", "tar.gz", "tar.bz2", "tar.xz", "tar.lzma", "tar.zst"), 0)
        setSpinner(R.id.tarZstdLevelSpinner, (0..22).map(Int::toString), 3)
        binding.root.findViewById<EditText>(R.id.sevenZThreadEdit).setText("2")
    }

    private fun setSpinner(id: Int, values: List<String>, selected: Int) {
        val spinner = binding.root.findViewById<Spinner>(id)
        spinner.adapter = ArrayAdapter(
            requireContext(), android.R.layout.simple_spinner_item, values
        ).also { it.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item) }
        spinner.setSelection(selected)
    }

    private fun selectedSpinnerValue(id: Int): String =
        binding.root.findViewById<Spinner>(id).selectedItem?.toString().orEmpty()

    private fun tarExtension(): String = selectedSpinnerValue(R.id.tarCompressionSpinner).ifBlank { "tar" }

    private fun createOptions(): ArchiveCreateOptions {
        val encryption = when (binding.root.findViewById<Spinner>(R.id.encryptionSpinner).selectedItemPosition) {
            1 -> ArchiveEncryption.ZIP_STANDARD
            2 -> ArchiveEncryption.ZIP_STANDARD_STRONG
            3 -> ArchiveEncryption.AES
            4 -> ArchiveEncryption.AES
            else -> ArchiveEncryption.NONE
        }
        val splitKiB = binding.root.findViewById<EditText>(R.id.splitSizeEdit).text?.toString()
            ?.trim()?.toLongOrNull()
            ?.takeIf { it > 0L }
        val tarCompression = when (selectedSpinnerValue(R.id.tarCompressionSpinner)) {
            "tar.gz" -> TarCompression.GZIP
            "tar.bz2" -> TarCompression.BZIP2
            "tar.xz" -> TarCompression.XZ
            "tar.lzma" -> TarCompression.LZMA
            "tar.zst" -> TarCompression.ZSTD
            else -> TarCompression.NONE
        }
        return ArchiveCreateOptions(
            zipCompression = if (binding.root.findViewById<Spinner>(R.id.compressionSpinner).selectedItemPosition == 0) {
                ZipCompression.STORE
            } else ZipCompression.DEFLATE,
            zipCompressionLevel = binding.root.findViewById<Spinner>(R.id.compressionLevelSpinner)
                .selectedItemPosition.coerceIn(0, 9),
            zipEncryption = encryption,
            zipAesKeyBits = if (binding.root.findViewById<Spinner>(R.id.encryptionSpinner).selectedItemPosition == 3) 128 else 256,
            zipSplitSizeBytes = splitKiB?.coerceAtLeast(64L)?.times(1024L),
            sevenZipCompressionLevel = listOf(0, 1, 3, 5, 7, 9).getOrElse(
                binding.root.findViewById<Spinner>(R.id.sevenZLevelSpinner).selectedItemPosition,
            ) { 5 },
            sevenZipSolid = binding.root.findViewById<android.widget.CheckBox>(R.id.sevenZSolidCheck).isChecked,
            sevenZipThreadCount = binding.root.findViewById<EditText>(R.id.sevenZThreadEdit).text?.toString()
                ?.toIntOrNull()?.coerceIn(1, 32) ?: 2,
            tarCompression = tarCompression,
            tarZstdLevel = binding.root.findViewById<Spinner>(R.id.tarZstdLevelSpinner)
                .selectedItemPosition.coerceIn(0, 22),
        )
    }

    override fun onOk(name: String) {
        val format = when (val checkedId = binding.typeGroup.checkedRadioButtonId) {
            R.id.zipRadio -> ArchiveFormat.ZIP
            R.id.tarXzRadio -> ArchiveFormat.COMPRESSED_TAR
            R.id.sevenZRadio -> ArchiveFormat.SEVEN_ZIP
            else -> throw AssertionError(checkedId)
        }
        val password = if (isPasswordSupported) {
            binding.passwordEdit.text!!.toString().takeIfNotEmpty()
        } else {
            null
        }
        try {
            listener.archiveWithOptions(args.files, name, format, createOptions(), password)
        } finally {
            // The job receives a char[] copy; clear the editable buffer as
            // soon as the dialog hands it off so the UI does not retain the
            // passphrase after dismissal.
            binding.passwordEdit.text?.clear()
        }
    }

    companion object {
        private const val KEY_ADVANCED = "zipxtract_advanced_options"
        private val ARCHIVE_EXTENSIONS = listOf(
            "tar.lzma", "tar.zst", "tar.bz2", "tar.gz", "tar.xz", "7z", "zip", "tar",
        )

        fun show(files: FileItemSet, fragment: Fragment) {
            CreateArchiveDialogFragment().putArgs(Args(files)).show(fragment)
        }
    }

    @Parcelize
    class Args(val files: FileItemSet) : ParcelableArgs

    protected class Binding private constructor(
        root: View,
        nameLayout: TextInputLayout,
        nameEdit: EditText,
        val typeGroup: RadioGroup,
        val passwordLayout: TextInputLayout,
        val passwordEdit: TextInputEditText
    ) : NameDialogFragment.Binding(root, nameLayout, nameEdit) {
        companion object {
            fun inflate(inflater: LayoutInflater): Binding {
                val binding = CreateArchiveDialogBinding.inflate(inflater)
                val bindingRoot = binding.root
                val nameBinding = NameDialogNameIncludeBinding.bind(bindingRoot)
                return Binding(
                    bindingRoot, nameBinding.nameLayout, nameBinding.nameEdit, binding.typeGroup,
                    binding.passwordLayout, binding.passwordEdit
                )
            }
        }
    }

    interface Listener : FileNameDialogFragment.Listener {
        fun archive(files: FileItemSet, name: String, format: Int, filter: Int, password: String?)

        fun archiveWithOptions(
            files: FileItemSet,
            name: String,
            format: ArchiveFormat,
            options: ArchiveCreateOptions,
            password: String?,
        ) {
            // A source-compatible default keeps downstream Material Files
            // embedders working while RongVualt uses the new core path.
            archive(files, name, Archive.FORMAT_ZIP, Archive.FILTER_NONE, password)
        }
    }
}
