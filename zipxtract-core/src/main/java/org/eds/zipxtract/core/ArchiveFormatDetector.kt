/*
 * Copyright (C) 2023 WirelessAlien <https://github.com/WirelessAlien>
 * Copyright (C) 2026 RongVualt contributors
 *
 * Derived from ZipXtract under the GNU GPL v3.
 */

package org.eds.zipxtract.core

import java.io.BufferedInputStream
import java.util.Locale

/** Small, deterministic format detector used before selecting a native backend. */
object ArchiveFormatDetector {
    private val tarCompressionExtensions = setOf(
        "gz", "tgz", "bz2", "tbz", "tbz2", "xz", "txz", "lzma", "zst", "zstd",
        "lz4", "br", "lzip", "z",
    )
    private val genericArchiveExtensions = setOf(
        "iso", "cab", "deb", "rpm", "dmg", "wim", "chm", "cpio",
        "ar", "jar",
    )

    fun detect(source: ArchiveSource): ArchiveProbe {
        val name = source.displayName
        val lower = name.lowercase(Locale.ROOT)
        val extensionProbe = detectByName(lower)
        val magicProbe = runCatching { detectByMagic(source) }.getOrNull()
        // A compressor magic number identifies the outer stream, not whether
        // the payload is a tar archive. Prefer an explicit `.tar.*` suffix in
        // that one ambiguous case; otherwise keep the stream format so a
        // single `.gz`/`.xz` file can be decompressed as well.
        val format = when {
            magicProbe?.format == ArchiveFormat.COMPRESSED_STREAM &&
                extensionProbe.format == ArchiveFormat.COMPRESSED_TAR ->
                ArchiveFormat.COMPRESSED_TAR
            magicProbe != null && magicProbe.format != ArchiveFormat.UNKNOWN -> magicProbe.format
            else -> extensionProbe.format
        }
        val encryption = magicProbe?.encryption ?: extensionProbe.encryption
        val capabilities = when (format) {
            ArchiveFormat.ZIP, ArchiveFormat.SEVEN_ZIP, ArchiveFormat.RAR,
            ArchiveFormat.TAR, ArchiveFormat.COMPRESSED_TAR, ArchiveFormat.COMPRESSED_STREAM,
            ArchiveFormat.GENERIC -> setOf(
                ArchiveCapability.LIST,
                ArchiveCapability.EXTRACT,
            )
            ArchiveFormat.UNKNOWN -> emptySet()
        }
        return ArchiveProbe(
            format,
            name,
            encryption,
            capabilities,
            volumeNames = extensionProbe.volumeNames,
        )
    }

    fun detectByName(name: String): ArchiveProbe {
        val lower = name.lowercase(Locale.ROOT)
        val base = lower.substringAfterLast('/').substringAfterLast('\\')
        return when {
            base.matches(Regex(".+\\.(zip|jar|apk)(\\.\\d{3}|\\.z\\d{2,})?")) ||
                base.matches(Regex(".+\\.z\\d{2,}")) ||
                base.matches(Regex(".+\\.zip\\.part\\d+")) ->
                ArchiveProbe(
                    ArchiveFormat.ZIP,
                    name,
                    volumeNames = if (base.matches(Regex(".+(\\.\\d{3}|\\.z\\d{2,})")) || base.contains(".part")) listOf(base) else emptyList(),
                )
            base.matches(Regex(".+\\.7z(\\.\\d{3})?")) ->
                ArchiveProbe(
                    ArchiveFormat.SEVEN_ZIP,
                    name,
                    volumeNames = if (base.matches(Regex(".+\\.7z\\.\\d{3}"))) listOf(base) else emptyList(),
                )
            base.matches(Regex(".+\\.part\\d+\\.rar")) ||
                base.matches(Regex(".+\\.(rar|r\\d{2})")) ->
                ArchiveProbe(
                    ArchiveFormat.RAR,
                    name,
                    volumeNames = if (base.contains(".part") || base.matches(Regex(".+\\.r\\d{2}"))) listOf(base) else emptyList(),
                )
            base.endsWith(".tar") -> ArchiveProbe(ArchiveFormat.TAR, name)
            base.substringAfterLast('.', "") in tarCompressionExtensions &&
                (base.contains(".tar.") || base.endsWith(".tgz") || base.endsWith(".tbz") || base.endsWith(".tbz2") || base.endsWith(".txz")) ->
                ArchiveProbe(ArchiveFormat.COMPRESSED_TAR, name)
            base.substringAfterLast('.', "") in tarCompressionExtensions ->
                ArchiveProbe(ArchiveFormat.COMPRESSED_STREAM, name)
            base.substringAfterLast('.', "") in genericArchiveExtensions ->
                ArchiveProbe(ArchiveFormat.GENERIC, name)
            else -> ArchiveProbe(ArchiveFormat.UNKNOWN, name)
        }
    }

    private fun detectByMagic(source: ArchiveSource): ArchiveProbe? {
        BufferedInputStream(source.openInputStream()).use { input ->
            // ISO-9660 stores the primary-volume signature at byte 32769;
            // reading that bounded prefix keeps magic detection useful even
            // when a provider does not offer a seekable channel.
            input.mark(ISO_MAGIC_END)
            val header = ByteArray(ISO_MAGIC_END)
            var count = 0
            while (count < header.size) {
                val read = input.read(header, count, header.size - count)
                if (read < 0) break
                if (read == 0) break
                count += read
            }
            if (count < 2) return null
            input.reset()
            return when {
                header.startsWith(byteArrayOf(0x50, 0x4b, 0x03, 0x04)) ||
                    header.startsWith(byteArrayOf(0x50, 0x4b, 0x05, 0x06)) ||
                    header.startsWith(byteArrayOf(0x50, 0x4b, 0x07, 0x08)) ->
                    ArchiveProbe(ArchiveFormat.ZIP, source.displayName)
                header.startsWith(byteArrayOf(0x37, 0x7a, 0xbc.toByte(), 0xaf.toByte(), 0x27, 0x1c)) ->
                    ArchiveProbe(ArchiveFormat.SEVEN_ZIP, source.displayName)
                header.startsWith(byteArrayOf(0x52, 0x61, 0x72, 0x21, 0x1a, 0x07)) ->
                    ArchiveProbe(ArchiveFormat.RAR, source.displayName)
                count >= 257 + 5 && String(header, 257, 5, Charsets.US_ASCII) == "ustar" ->
                    ArchiveProbe(ArchiveFormat.TAR, source.displayName)
                count >= 4 && (header.startsWithAscii("MSCF") ||
                    (count >= 7 && header.startsWithAscii("MSWIM\u0000\u0000")) ||
                    header.startsWithAscii("ITSF")) ->
                    ArchiveProbe(ArchiveFormat.GENERIC, source.displayName)
                count >= 4 && (header.startsWithAscii("!<arch>\n") ||
                    header.startsWith(byteArrayOf(0xed.toByte(), 0xab.toByte(), 0xee.toByte(), 0xdb.toByte())) ||
                    header.startsWithAscii("070701") || header.startsWithAscii("070702") ||
                    header.startsWithAscii("070707")) ->
                    ArchiveProbe(ArchiveFormat.GENERIC, source.displayName)
                count >= ISO_MAGIC_OFFSET + ISO_MAGIC_LENGTH &&
                    String(header, ISO_MAGIC_OFFSET, ISO_MAGIC_LENGTH, Charsets.US_ASCII) == "CD001" ->
                    ArchiveProbe(ArchiveFormat.GENERIC, source.displayName)
                header.startsWith(byteArrayOf(0x1f, 0x8b)) ||
                header.startsWith(byteArrayOf(0x42, 0x5a, 0x68)) ||
                    header.startsWith(byteArrayOf(0xfd.toByte(), 0x37, 0x7a, 0x58, 0x5a, 0x00)) ||
                    header.startsWith(byteArrayOf(0x28, 0xb5.toByte(), 0x2f, 0xfd.toByte())) ||
                    header.startsWith(byteArrayOf(0x04, 0x22, 0x4d, 0x18)) ||
                    header.startsWith(byteArrayOf(0x4c, 0x5a, 0x49, 0x50)) ->
                    ArchiveProbe(ArchiveFormat.COMPRESSED_STREAM, source.displayName)
                else -> null
            }
        }
    }

    private fun ByteArray.startsWith(prefix: ByteArray): Boolean =
        size >= prefix.size && prefix.indices.all { this[it] == prefix[it] }

    private fun ByteArray.startsWithAscii(prefix: String): Boolean =
        size >= prefix.length && prefix.indices.all { this[it].toInt().and(0xff) == prefix[it].code }

    private const val ISO_MAGIC_OFFSET = 0x8001
    private const val ISO_MAGIC_LENGTH = 5
    private const val ISO_MAGIC_END = ISO_MAGIC_OFFSET + ISO_MAGIC_LENGTH
}
