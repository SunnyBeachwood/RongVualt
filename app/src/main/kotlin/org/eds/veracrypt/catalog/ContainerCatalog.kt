package org.eds.veracrypt.catalog

import android.content.Context
import android.net.Uri
import java.util.UUID
import org.eds.veracrypt.credentials.CredentialVault
import org.eds.veracrypt.credentials.SavedUnlockCredential
import org.json.JSONArray
import org.json.JSONObject

/**
 * Stores user-selected container locations, never credentials or unlocked
 * filesystem paths. Persisted SAF grants are owned by Android; callers must
 * acquire them before adding an entry here.
 */
class ContainerCatalog(context: Context) {
    private val appContext = context.applicationContext
    private val preferences = appContext.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE)
    private val credentialVault = CredentialVault(appContext)

    init {
        // Recognition hints must not survive in the catalog: every manual
        // open starts from AUTO for both cipher and KDF.
        if (!preferences.getBoolean(OPEN_HINTS_MIGRATED, false)) {
            writeEntries(readEntries())
            preferences.edit().putBoolean(OPEN_HINTS_MIGRATED, true).apply()
        }
    }

    @Synchronized
    fun list(): List<ContainerCatalogEntry> = readEntries().sortedBy { it.displayName.lowercase() }

    @Synchronized
    fun add(uri: Uri, displayName: String): ContainerCatalogEntry {
        require(uri.scheme == "content") { "Containers must be selected through Storage Access Framework" }
        val normalizedName = displayName.trim()
        require(normalizedName.isNotEmpty()) { "Container display name is required" }
        require(normalizedName.length <= MAX_DISPLAY_NAME_LENGTH) { "Container display name is too long" }
        // URI identity is stable for both persisted SAF documents and our
        // private local-file bridge. Do not create two unlock records for the
        // same container file.
        readEntries().firstOrNull { it.uri == uri.toString() }?.let { return it }
        val entry = ContainerCatalogEntry(UUID.randomUUID(), uri.toString(), normalizedName)
        writeEntries(readEntries() + entry)
        return entry
    }

    @Synchronized
    fun remove(id: UUID) {
        writeEntries(readEntries().filterNot { it.id == id })
        credentialVault.remove(SavedUnlockCredential.recordId(id))
    }

    /** Removes every saved catalog record and its associated saved credential. */
    @Synchronized
    fun clear() {
        val ids = readEntries().map { it.id }
        writeEntries(emptyList())
        ids.forEach { credentialVault.remove(SavedUnlockCredential.recordId(it)) }
    }

    @Synchronized
    fun clearOnExitEnabled(): Boolean = preferences.getBoolean(CLEAR_ON_EXIT, false)

    @Synchronized
    fun setClearOnExitEnabled(enabled: Boolean) {
        check(preferences.edit().putBoolean(CLEAR_ON_EXIT, enabled).commit()) {
            "Could not save container catalog privacy setting"
        }
    }

    @Synchronized
    fun find(id: UUID): ContainerCatalogEntry? = readEntries().firstOrNull { it.id == id }

    @Synchronized
    fun findByUri(uri: Uri): ContainerCatalogEntry? = readEntries().firstOrNull { it.uri == uri.toString() }

    private fun readEntries(): List<ContainerCatalogEntry> {
        val serialized = preferences.getString(ENTRIES_V2, null)
            ?: preferences.getString(ENTRIES_V1, null)
            ?: return emptyList()
        return try {
            val array = JSONArray(serialized)
            buildList {
                for (index in 0 until array.length()) {
                    val item = array.getJSONObject(index)
                    val id = UUID.fromString(item.getString("id"))
                    val uri = item.getString("uri")
                    val name = item.getString("name")
                    if (Uri.parse(uri).scheme == "content" && name.isNotBlank() && name.length <= MAX_DISPLAY_NAME_LENGTH) {
                        add(ContainerCatalogEntry(id = id, uri = uri, displayName = name))
                    }
                }
            }
        } catch (_: Exception) {
            emptyList()
        }
    }

    private fun writeEntries(entries: List<ContainerCatalogEntry>) {
        val array = JSONArray()
        entries.forEach { entry ->
            array.put(JSONObject().apply {
                put("id", entry.id.toString())
                put("uri", entry.uri)
                put("name", entry.displayName)
            })
        }
        check(preferences.edit().putString(ENTRIES_V2, array.toString()).remove(ENTRIES_V1).commit()) {
            "Could not save container catalog"
        }
    }

    companion object {
        private const val PREFERENCES = "veracrypt_container_catalog"
        private const val ENTRIES_V1 = "entries.v1"
        private const val ENTRIES_V2 = "entries.v2"
        private const val CLEAR_ON_EXIT = "clear_on_exit"
        private const val OPEN_HINTS_MIGRATED = "open_hints_migrated"
        private const val MAX_DISPLAY_NAME_LENGTH = 256
    }
}

data class ContainerCatalogEntry(
    /** Random stable identifier used by unlocked-session and provider code. */
    val id: UUID,
    /** Private persisted SAF URI; never use this string as a document ID. */
    val uri: String,
    val displayName: String,
)
