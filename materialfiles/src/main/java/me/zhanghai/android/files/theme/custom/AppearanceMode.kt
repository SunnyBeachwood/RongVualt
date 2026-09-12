package me.zhanghai.android.files.theme.custom

/** Mutually-exclusive visual styles exposed by the embedded file manager. */
enum class AppearanceMode {
    STANDARD,
    MATERIAL3,
    DYNAMIC,
    BLACK,
}

internal fun migrateAppearanceMode(
    blackNightMode: Boolean,
    dynamicColor: Boolean,
    material3: Boolean,
): AppearanceMode = when {
    blackNightMode -> AppearanceMode.BLACK
    dynamicColor -> AppearanceMode.DYNAMIC
    material3 -> AppearanceMode.MATERIAL3
    else -> AppearanceMode.STANDARD
}
