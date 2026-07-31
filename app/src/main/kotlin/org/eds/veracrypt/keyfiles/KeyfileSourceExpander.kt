package org.eds.veracrypt.keyfiles

import android.content.Context
import android.net.Uri
import androidx.documentfile.provider.DocumentFile
import org.eds.veracrypt.domain.KeyfileSource
import org.eds.veracrypt.domain.VolumeError

/** Expands SAF selections with VeraCrypt's one-directory-level keyfile rule. */
class KeyfileSourceExpander(private val context: Context) {
    fun expand(sources: List<KeyfileSource>): List<KeyfileSource> {
        val result = mutableListOf<KeyfileSource>()
        val visited = mutableSetOf<String>()
        sources.forEach { append(Uri.parse(it.uri), result, visited) }
        if (result.isEmpty()) throw VolumeError.InvalidCredentialsOrFormat()
        return result
    }

    private fun append(uri: Uri, output: MutableList<KeyfileSource>, visited: MutableSet<String>) {
        if (!visited.add(uri.toString())) return
        val document = DocumentFile.fromSingleUri(context, uri) ?: DocumentFile.fromTreeUri(context, uri)
            ?: throw VolumeError.IoInterrupted(IllegalArgumentException("Invalid keyfile URI"))
        if (document.isFile) {
            require(output.size < MAX_KEYFILES) { "Too many keyfiles" }
            output += KeyfileSource(uri.toString())
            return
        }
        if (!document.isDirectory) throw VolumeError.IoInterrupted(IllegalArgumentException("Keyfile is not readable"))
        // Directory selections include only immediate, non-hidden regular
        // files. VeraCrypt does not recursively walk subdirectories here.
        document.listFiles()
            .asSequence()
            .filter { it.isFile }
            .filterNot { (it.name ?: "").startsWith('.') }
            .sortedWith(compareBy({ it.name ?: "" }, { it.uri.toString() }))
            .forEach { child ->
                if (visited.add(child.uri.toString())) {
                    require(output.size < MAX_KEYFILES) { "Too many keyfiles" }
                    output += KeyfileSource(child.uri.toString())
                }
            }
    }

    private companion object { const val MAX_KEYFILES = 1_024 }
}
