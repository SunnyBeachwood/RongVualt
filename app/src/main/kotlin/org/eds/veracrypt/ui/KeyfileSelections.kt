package org.eds.veracrypt.ui

import android.content.Intent
import android.net.Uri
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import org.eds.veracrypt.domain.KeyfileSource
import org.eds.veracrypt.keyfiles.KeyfileGenerator
import com.sovworks.eds.android.R

/** Request-scoped SAF keyfile selections for a sensitive volume operation. */
internal class KeyfileSelections(private val fragment: Fragment) {
    private val sources = mutableListOf<KeyfileSource>()
    private var list: LinearLayout? = null
    private var onError: ((String) -> Unit)? = null

    private val selectFiles = fragment.registerForActivityResult(ActivityResultContracts.OpenMultipleDocuments()) { uris ->
        uris.forEach(::addPersistedReadUri)
        render()
    }
    private val selectDirectory = fragment.registerForActivityResult(ActivityResultContracts.OpenDocumentTree()) { uri ->
        uri?.let(::addPersistedReadUri)
        render()
    }
    private val createKeyfile = fragment.registerForActivityResult(ActivityResultContracts.CreateDocument("application/octet-stream")) { uri ->
        uri ?: return@registerForActivityResult
        fragment.lifecycleScope.launch {
            try {
                if (!addPersistedReadUri(uri)) return@launch
                KeyfileGenerator(fragment.requireContext().contentResolver).generate(uri)
                render()
            } catch (error: Throwable) {
                sources.removeAll { it.uri == uri.toString() }
                render()
                onError?.invoke(error.message ?: fragment.getString(R.string.vc_keyfile_generate_failed))
            }
        }
    }

    fun bind(list: LinearLayout, selectFiles: Button, selectDirectory: Button, generate: Button, onError: (String) -> Unit) {
        this.list = list
        this.onError = onError
        selectFiles.setOnClickListener { this.selectFiles.launch(arrayOf("application/octet-stream", "*/*")) }
        selectDirectory.setOnClickListener { this.selectDirectory.launch(null) }
        generate.setOnClickListener { createKeyfile.launch("veracrypt-keyfile.bin") }
        render()
    }

    fun snapshot(): List<KeyfileSource> = sources.toList()

    fun clear() {
        sources.clear()
        render()
    }

    private fun addPersistedReadUri(uri: Uri): Boolean {
        val resolver = fragment.requireContext().contentResolver
        try {
            resolver.takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION)
        } catch (_: SecurityException) {
            onError?.invoke("Android did not grant persistent read access to this keyfile")
            return false
        }
        if (sources.none { it.uri == uri.toString() }) sources += KeyfileSource(uri.toString())
        return true
    }

    private fun render() {
        val target = list ?: return
        target.removeAllViews()
        sources.forEach { source ->
            val row = LinearLayout(fragment.requireContext()).apply { orientation = LinearLayout.HORIZONTAL }
            row.addView(TextView(fragment.requireContext()).apply {
                text = Uri.parse(source.uri).lastPathSegment ?: source.uri
                layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
            })
            row.addView(Button(fragment.requireContext()).apply {
                text = fragment.getString(R.string.vc_remove_keyfile)
                setOnClickListener { sources.remove(source); render() }
            })
            target.addView(row)
        }
    }
}
