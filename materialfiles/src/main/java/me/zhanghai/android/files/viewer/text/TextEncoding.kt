package me.zhanghai.android.files.viewer.text

import java.nio.ByteBuffer
import java.nio.charset.Charset
import java.nio.charset.CodingErrorAction
import java.nio.charset.StandardCharsets

/** Encoding selected from a BOM or from strict text validation. */
internal data class DetectedTextEncoding(
    val charset: Charset,
    val byteOrderMark: ByteArray = ByteArray(0),
)

internal fun detectTextEncoding(bytes: ByteArray): DetectedTextEncoding {
    if (bytes.startsWith(UTF_8_BOM)) {
        return DetectedTextEncoding(StandardCharsets.UTF_8, UTF_8_BOM)
    }
    if (bytes.startsWith(UTF_32_LE_BOM)) {
        return DetectedTextEncoding(Charset.forName("UTF-32LE"), UTF_32_LE_BOM)
    }
    if (bytes.startsWith(UTF_32_BE_BOM)) {
        return DetectedTextEncoding(Charset.forName("UTF-32BE"), UTF_32_BE_BOM)
    }
    if (bytes.startsWith(UTF_16_LE_BOM)) {
        return DetectedTextEncoding(StandardCharsets.UTF_16LE, UTF_16_LE_BOM)
    }
    if (bytes.startsWith(UTF_16_BE_BOM)) {
        return DetectedTextEncoding(StandardCharsets.UTF_16BE, UTF_16_BE_BOM)
    }
    if (bytes.isValid(StandardCharsets.UTF_8)) {
        return DetectedTextEncoding(StandardCharsets.UTF_8)
    }

    // Legacy Simplified-Chinese Windows programs commonly write "ANSI"
    // files in the system code page. GB18030 is a strict superset of GBK and
    // preserves those files when the editor saves them again.
    val windowsChinese = Charset.forName("GB18030")
    return if (bytes.isValid(windowsChinese)) {
        DetectedTextEncoding(windowsChinese)
    } else {
        DetectedTextEncoding(StandardCharsets.UTF_8)
    }
}

internal fun decodeText(
    bytes: ByteArray,
    charset: Charset,
    detected: DetectedTextEncoding?,
): String {
    val bomLength = detected?.takeIf { it.charset == charset && bytes.startsWith(it.byteOrderMark) }
        ?.byteOrderMark?.size ?: 0
    return String(bytes, bomLength, bytes.size - bomLength, charset)
}

internal fun encodeText(
    text: String,
    charset: Charset,
    detected: DetectedTextEncoding?,
): ByteArray {
    val body = text.toByteArray(charset)
    val bom = detected?.takeIf { it.charset == charset }?.byteOrderMark ?: return body
    if (bom.isEmpty()) return body
    return ByteArray(bom.size + body.size).also { output ->
        bom.copyInto(output)
        body.copyInto(output, bom.size)
    }
}

private fun ByteArray.isValid(charset: Charset): Boolean = runCatching {
    charset.newDecoder()
        .onMalformedInput(CodingErrorAction.REPORT)
        .onUnmappableCharacter(CodingErrorAction.REPORT)
        .decode(ByteBuffer.wrap(this))
}.isSuccess

private fun ByteArray.startsWith(prefix: ByteArray): Boolean =
    size >= prefix.size && prefix.indices.all { index -> this[index] == prefix[index] }

private val UTF_8_BOM = byteArrayOf(0xEF.toByte(), 0xBB.toByte(), 0xBF.toByte())
private val UTF_16_LE_BOM = byteArrayOf(0xFF.toByte(), 0xFE.toByte())
private val UTF_16_BE_BOM = byteArrayOf(0xFE.toByte(), 0xFF.toByte())
private val UTF_32_LE_BOM = byteArrayOf(0xFF.toByte(), 0xFE.toByte(), 0x00, 0x00)
private val UTF_32_BE_BOM = byteArrayOf(0x00, 0x00, 0xFE.toByte(), 0xFF.toByte())
