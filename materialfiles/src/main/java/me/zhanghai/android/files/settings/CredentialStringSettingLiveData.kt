package me.zhanghai.android.files.settings

import android.content.SharedPreferences
import androidx.annotation.StringRes
import me.zhanghai.android.files.app.application
import me.zhanghai.android.files.security.CredentialVault

/** A String setting whose value is encrypted before it reaches preferences. */
class CredentialStringSettingLiveData(
    @StringRes keyRes: Int,
    @StringRes defaultValueRes: Int,
) : SettingLiveData<String>(keyRes, defaultValueRes) {
    override fun getDefaultValue(defaultValueRes: Int): String = application.getString(defaultValueRes)

    override fun getValue(sharedPreferences: SharedPreferences, key: String, defaultValue: String): String {
        val reference = "setting:$key"
        CredentialVault.get(reference)?.let { return it }
        val legacy = sharedPreferences.getString(key, null) ?: return defaultValue
        if (legacy.isNotEmpty()) {
            CredentialVault.put(reference, legacy)
            sharedPreferences.edit().remove(key).apply()
        }
        return legacy
    }

    override fun putValue(sharedPreferences: SharedPreferences, key: String, value: String) {
        CredentialVault.put("setting:$key", value)
        sharedPreferences.edit().remove(key).apply()
    }
}
