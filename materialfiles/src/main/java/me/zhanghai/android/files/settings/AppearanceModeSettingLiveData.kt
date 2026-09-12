package me.zhanghai.android.files.settings

import android.content.SharedPreferences
import androidx.core.content.edit
import me.zhanghai.android.files.R
import me.zhanghai.android.files.app.application
import me.zhanghai.android.files.theme.custom.AppearanceMode
import me.zhanghai.android.files.theme.custom.ThemeColorSource
import me.zhanghai.android.files.theme.custom.migrateAppearanceMode

/** Reads the new single-choice setting and migrates the three legacy controls once. */
class AppearanceModeSettingLiveData : SettingLiveData<AppearanceMode>(
    R.string.pref_key_appearance_mode,
    R.string.pref_default_value_appearance_mode,
) {
    init {
        init()
    }

    override fun getDefaultValue(defaultValueRes: Int): AppearanceMode =
        AppearanceMode.entries[application.getString(defaultValueRes).toInt()]

    override fun getValue(
        sharedPreferences: SharedPreferences,
        key: String,
        defaultValue: AppearanceMode,
    ): AppearanceMode {
        val stored = sharedPreferences.getString(key, null)?.toIntOrNull()
        if (stored != null) return AppearanceMode.entries.getOrElse(stored) { defaultValue }

        val black = sharedPreferences.getBoolean(
            application.getString(R.string.pref_key_black_night_mode), false
        )
        val material3 = sharedPreferences.getBoolean(
            application.getString(R.string.pref_key_material_design_3), true
        )
        val sourceOrdinal = sharedPreferences.getString(
            application.getString(R.string.pref_key_theme_color_source), null
        )?.toIntOrNull()
        val dynamic = sourceOrdinal == null ||
            ThemeColorSource.entries.getOrNull(sourceOrdinal) == ThemeColorSource.DYNAMIC
        return migrateAppearanceMode(black, dynamic, material3).also { migrated ->
            sharedPreferences.edit { putString(key, migrated.ordinal.toString()) }
        }
    }

    override fun putValue(
        sharedPreferences: SharedPreferences,
        key: String,
        value: AppearanceMode,
    ) {
        sharedPreferences.edit { putString(key, value.ordinal.toString()) }
    }
}
