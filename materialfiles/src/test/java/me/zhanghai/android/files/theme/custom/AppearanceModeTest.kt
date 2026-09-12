package me.zhanghai.android.files.theme.custom

import org.junit.Assert.assertEquals
import org.junit.Test

class AppearanceModeTest {
    @Test
    fun migrationUsesRequiredConflictPriority() {
        assertEquals(AppearanceMode.BLACK, migrateAppearanceMode(true, true, true))
        assertEquals(AppearanceMode.DYNAMIC, migrateAppearanceMode(false, true, true))
        assertEquals(AppearanceMode.MATERIAL3, migrateAppearanceMode(false, false, true))
        assertEquals(AppearanceMode.STANDARD, migrateAppearanceMode(false, false, false))
    }
}
