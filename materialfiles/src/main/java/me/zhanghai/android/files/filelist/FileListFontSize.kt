package me.zhanghai.android.files.filelist

/** Font density used by file names and their metadata in the native browser. */
class FileListFontSize private constructor(val nameSp: Float) {
    val metadataSp: Float
        get() = (nameSp * METADATA_SCALE).roundToInt().coerceAtLeast(MIN_METADATA_SP).toFloat()

    val rowHeightDp: Int
        get() = (nameSp * ROW_HEIGHT_SCALE).roundToInt().coerceAtLeast(MIN_ROW_HEIGHT_DP)

    val iconSlotDp: Int
        get() = (nameSp * ICON_SLOT_SCALE).roundToInt()
            .coerceIn(MIN_ICON_SLOT_DP, MAX_ICON_SLOT_DP)

    companion object {
        const val MIN_SP = 12
        const val MAX_SP = 32
        const val DEFAULT_SP = 16

        private const val METADATA_SCALE = 0.75f
        private const val ROW_HEIGHT_SCALE = 3.5f
        private const val ICON_SLOT_SCALE = 2.5f
        private const val MIN_METADATA_SP = 10
        private const val MIN_ROW_HEIGHT_DP = 48
        private const val MIN_ICON_SLOT_DP = 36
        private const val MAX_ICON_SLOT_DP = 64

        fun fromSp(value: Int): FileListFontSize =
            FileListFontSize(value.coerceIn(MIN_SP, MAX_SP).toFloat())
    }
}

private fun Float.roundToInt(): Int = kotlin.math.round(this).toInt()
