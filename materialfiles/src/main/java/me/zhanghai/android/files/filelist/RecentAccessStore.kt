package me.zhanghai.android.files.filelist

import android.content.Context
import androidx.core.content.edit
import java8.nio.file.Path
import org.json.JSONArray
import org.json.JSONObject

/** Small, ordered and private MRU list for the dual-pane browser. */
data class RecentAccessEntry(
    val uri: String,
    val title: String,
    val isDirectory: Boolean,
    val accessedAt: Long,
    val displayPath: String = uri,
)

class RecentAccessStore(context: Context) {
    private val preferences = context.getSharedPreferences(PREFERENCES_NAME, Context.MODE_PRIVATE)

    fun entries(): List<RecentAccessEntry> = runCatching {
        val array = JSONArray(preferences.getString(KEY_ENTRIES, "[]"))
        buildList {
            for (index in 0 until array.length()) {
                val item = array.getJSONObject(index)
                add(
                    RecentAccessEntry(
                        item.getString("uri"),
                        item.getString("title"),
                        item.getBoolean("directory"),
                        item.getLong("accessedAt"),
                        item.optString("path", item.getString("uri")),
                    )
                )
            }
        }
    }.getOrDefault(emptyList())

    fun record(path: Path, isDirectory: Boolean) {
        val entry = RecentAccessEntry(
            path.toUri().toString(), path.name, isDirectory, System.currentTimeMillis(),
            path.toUserFriendlyString(),
        )
        save(merge(entries(), entry))
    }

    fun clear() = preferences.edit { remove(KEY_ENTRIES) }

    private fun save(entries: List<RecentAccessEntry>) {
        val array = JSONArray()
        entries.forEach { entry ->
            array.put(
                JSONObject()
                    .put("uri", entry.uri)
                    .put("title", entry.title)
                    .put("directory", entry.isDirectory)
                    .put("accessedAt", entry.accessedAt)
                    .put("path", entry.displayPath)
            )
        }
        preferences.edit { putString(KEY_ENTRIES, array.toString()) }
    }

    companion object {
        private const val PREFERENCES_NAME = "rongvault_recent_access"
        private const val KEY_ENTRIES = "entries"
        const val MAX_ENTRIES = 20

        internal fun merge(
            existing: List<RecentAccessEntry>,
            entry: RecentAccessEntry,
        ): List<RecentAccessEntry> =
            (listOf(entry) + existing.filterNot { it.uri == entry.uri }).take(MAX_ENTRIES)
    }
}
