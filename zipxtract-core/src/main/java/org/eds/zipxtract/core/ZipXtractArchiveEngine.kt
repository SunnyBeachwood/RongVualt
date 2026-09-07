/*
 * Copyright (C) 2023 WirelessAlien <https://github.com/WirelessAlien>
 * Copyright (C) 2026 RongVualt contributors
 *
 * Derived from ZipXtract under the GNU GPL v3. The Android UI and service
 * layer were intentionally not imported; this class exposes the upstream
 * archive engines through provider-neutral streams.
 */

package org.eds.zipxtract.core

import java.io.BufferedInputStream
import java.io.BufferedOutputStream
import java.io.Closeable
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.io.RandomAccessFile
import java.util.Date
import java.util.Locale
import net.lingala.zip4j.ZipFile
import net.lingala.zip4j.exception.ZipException
import net.lingala.zip4j.io.outputstream.SplitOutputStream
import net.lingala.zip4j.io.outputstream.ZipOutputStream
import net.lingala.zip4j.model.ZipParameters
import net.lingala.zip4j.model.enums.AesKeyStrength
import net.lingala.zip4j.model.enums.CompressionLevel
import net.lingala.zip4j.model.enums.CompressionMethod
import net.lingala.zip4j.model.enums.EncryptionMethod
import org.apache.commons.compress.archivers.tar.TarArchiveEntry
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream
import org.apache.commons.compress.compressors.CompressorInputStream
import org.apache.commons.compress.compressors.CompressorStreamFactory
import org.apache.commons.compress.compressors.bzip2.BZip2CompressorOutputStream
import org.apache.commons.compress.compressors.gzip.GzipCompressorOutputStream
import org.apache.commons.compress.compressors.lzma.LZMACompressorOutputStream
import org.apache.commons.compress.compressors.xz.XZCompressorOutputStream
import org.apache.commons.compress.compressors.zstandard.ZstdCompressorOutputStream
import net.sf.sevenzipjbinding.ExtractAskMode
import net.sf.sevenzipjbinding.ExtractOperationResult
import net.sf.sevenzipjbinding.IArchiveExtractCallback
import net.sf.sevenzipjbinding.IArchiveOpenCallback
import net.sf.sevenzipjbinding.IArchiveOpenVolumeCallback
import net.sf.sevenzipjbinding.ICryptoGetTextPassword
import net.sf.sevenzipjbinding.IInArchive
import net.sf.sevenzipjbinding.IInStream
import net.sf.sevenzipjbinding.IOutCreateCallback
import net.sf.sevenzipjbinding.IOutItem7z
import net.sf.sevenzipjbinding.ISequentialInStream
import net.sf.sevenzipjbinding.ISequentialOutStream
import net.sf.sevenzipjbinding.PropID
import net.sf.sevenzipjbinding.SevenZip
import net.sf.sevenzipjbinding.SevenZipException
import net.sf.sevenzipjbinding.impl.OutItemFactory
import net.sf.sevenzipjbinding.impl.RandomAccessFileInStream
import net.sf.sevenzipjbinding.impl.RandomAccessFileOutStream
import net.sf.sevenzipjbinding.impl.VolumedArchiveInStream

/**
 * ZipXtract's engine selection and safety policy without its application UI.
 * The engine writes extracted entries directly to [ArchiveTarget] streams;
 * only file-oriented archive containers are staged in [PrivateArchiveTempStore].
 */
class ZipXtractArchiveEngine(
    private val tempStore: PrivateArchiveTempStore,
) : ArchiveEngine, ArchiveEngineCapabilities {

    override fun supportsFormat(format: ArchiveFormat): Boolean = format in setOf(
        ArchiveFormat.ZIP,
        ArchiveFormat.SEVEN_ZIP,
        ArchiveFormat.RAR,
        ArchiveFormat.TAR,
        ArchiveFormat.COMPRESSED_TAR,
        ArchiveFormat.COMPRESSED_STREAM,
    )

    override fun probe(source: ArchiveSource): ArchiveProbe {
        val detected = ArchiveFormatDetector.detect(source)
        val capabilities = detected.capabilities.toMutableSet()
        if (supportsCreate(detected.format)) capabilities += ArchiveCapability.CREATE
        if (detected.format == ArchiveFormat.SEVEN_ZIP) capabilities += ArchiveCapability.UPDATE
        if (detected.volumeNames.isNotEmpty()) capabilities += ArchiveCapability.MULTI_VOLUME
        return detected.copy(capabilities = capabilities)
    }

    override fun supportsCreate(format: ArchiveFormat): Boolean = format in setOf(
        ArchiveFormat.ZIP,
        ArchiveFormat.SEVEN_ZIP,
        ArchiveFormat.TAR,
        ArchiveFormat.COMPRESSED_TAR,
    )

    override fun list(source: ArchiveSource, password: CharArray?): List<ArchiveEntry> {
        try {
            val probe = probe(source)
            return when (probe.format) {
                ArchiveFormat.ZIP -> withStagedVolumes(source, probe) { primary, volumes ->
                    if (isSevenZipZipVolume(source.displayName)) {
                        // 7-Zip's `.zip.001`/`.zip.partN` convention is not the
                        // Zip4j `.z01` convention; use the same native volume
                        // reader as the upstream app for that form.
                        listSevenZip(primary, volumes, password, ArchiveFormat.ZIP)
                    } else {
                        listZip(primary, password)
                    }
                }
                ArchiveFormat.SEVEN_ZIP, ArchiveFormat.RAR -> withStagedVolumes(source, probe) { primary, volumes ->
                    listSevenZip(primary, volumes, password, probe.format)
                }
                ArchiveFormat.TAR, ArchiveFormat.COMPRESSED_TAR -> listTar(source)
                ArchiveFormat.COMPRESSED_STREAM -> listCompressedStream(source)
                else -> throw UnsupportedArchiveException("Unsupported archive format: ${source.displayName}")
            }
        } catch (error: ZipException) {
            if (error.type == ZipException.Type.WRONG_PASSWORD) {
                throw ArchivePasswordException("Wrong archive password", error)
            }
            throw ArchiveException(error.message ?: "ZIP listing failed", error)
        } catch (error: SevenZipException) {
            if (error.message?.contains("WrongPassword", ignoreCase = true) == true) {
                throw ArchivePasswordException("Wrong archive password", error)
            }
            throw ArchiveException(error.message ?: "7-Zip listing failed", error)
        } finally {
            password.wipe()
        }
    }

    override fun extract(request: ExtractRequest, listener: ArchiveProgressListener?) {
        try {
            request.cancellation.throwIfCancelled()
            val probe = probe(request.source)
            val destination = if (request.createContainingDirectory) {
                request.destination.resolve(
                    ArchiveNames.containingDirectoryName(request.source.displayName, probe.format)
                ).also { if (!it.exists()) it.createDirectory() }
            } else {
                request.destination.also { if (!it.exists()) it.createDirectory() }
            }
            when (probe.format) {
                ArchiveFormat.ZIP -> withStagedVolumes(request.source, probe) { primary, volumes ->
                    if (isSevenZipZipVolume(request.source.displayName)) {
                        extractSevenZip(primary, volumes, destination, request, listener, ArchiveFormat.ZIP)
                    } else {
                        extractZip(primary, destination, request, listener)
                    }
                }
                ArchiveFormat.SEVEN_ZIP, ArchiveFormat.RAR -> withStagedVolumes(request.source, probe) { primary, volumes ->
                    extractSevenZip(primary, volumes, destination, request, listener, probe.format)
                }
                ArchiveFormat.TAR, ArchiveFormat.COMPRESSED_TAR ->
                    extractTar(request.source, destination, request, listener)
                ArchiveFormat.COMPRESSED_STREAM ->
                    extractCompressedStream(request.source, destination, request, listener)
                else -> throw UnsupportedArchiveException("Unsupported archive format: ${request.source.displayName}")
            }
        } catch (e: ZipException) {
            if (e.type == ZipException.Type.WRONG_PASSWORD) {
                throw ArchivePasswordException("Wrong archive password", e)
            }
            throw ArchiveException(e.message ?: "ZIP extraction failed", e)
        } catch (e: SevenZipException) {
            if (e.message?.contains("WrongPassword", ignoreCase = true) == true) {
                throw ArchivePasswordException("Wrong archive password", e)
            }
            throw ArchiveException(e.message ?: "7-Zip extraction failed", e)
        } finally {
            request.password.wipe()
        }
    }

    override fun create(request: CreateArchiveRequest, listener: ArchiveProgressListener?) {
        try {
            request.cancellation.throwIfCancelled()
            request.options.validate(
                request.format,
                request.password,
                request.sources.sumOf { it.size.coerceAtLeast(0L) },
            )
            validateInputEntries(request.sources)
            when (request.format) {
                ArchiveFormat.ZIP -> createZip(request, listener)
                ArchiveFormat.SEVEN_ZIP -> createSevenZip(request, listener)
                ArchiveFormat.TAR, ArchiveFormat.COMPRESSED_TAR -> createTar(request, listener)
                else -> throw UnsupportedArchiveException("Cannot create ${request.format}")
            }
        } finally {
            request.password.wipe()
        }
    }

    override fun update7z(request: Update7zRequest, listener: ArchiveProgressListener?) {
        try {
            if (request.password?.isNotEmpty() == true) {
                throw UnsupportedArchiveException("Encrypted 7z archives are read-only")
            }
            request.cancellation.throwIfCancelled()
            validateInputEntries(request.additions)
            withStagedVolumes(request.source, probe(request.source)) { primary, volumes ->
                updateSevenZip(primary, volumes, request, listener)
            }
        } finally {
            request.password.wipe()
        }
    }

    private fun listZip(file: File, password: CharArray?): List<ArchiveEntry> {
        return ZipFile(file).use { zip ->
            if (password?.isNotEmpty() == true) zip.setPassword(password)
            zip.fileHeaders.map { header ->
                val path = ArchivePathPolicy.normalizeEntryName(header.fileName)
                ArchiveEntry(
                    name = path,
                    isDirectory = header.isDirectory,
                    size = header.uncompressedSize,
                    lastModifiedEpochMillis = header.lastModifiedTimeEpoch,
                    encrypted = header.isEncrypted,
                    isSymbolicLink = isZipSymbolicLink(header),
                )
            }
        }
    }

    private fun extractZip(
        file: File,
        destination: ArchiveTarget,
        request: ExtractRequest,
        listener: ArchiveProgressListener?,
    ) {
        ZipFile(file).use { zip ->
            if (request.password?.isNotEmpty() == true) zip.setPassword(request.password)
            val headers = zip.fileHeaders
            headers.firstOrNull { isZipSymbolicLink(it) }?.let { header ->
                throw ArchiveException("Symbolic links are not extracted: ${header.fileName}")
            }
            val selected = request.entries
            val total = headers.sumOf { it.uncompressedSize.coerceAtLeast(0L) }
            var completed = 0L
            var completedEntries = 0L
            for (header in headers) {
                request.cancellation.throwIfCancelled()
                val path = ArchivePathPolicy.normalizeEntryName(header.fileName)
                if (path.isEmpty()) continue
                if (isZipSymbolicLink(header)) {
                    throw ArchiveException("Symbolic links are not extracted: $path")
                }
                if (selected != null && selected.none { it == path || ArchivePathPolicy.isChildOf(it, path) }) {
                    continue
                }
                val target = destination.resolve(path)
                if (header.isDirectory) {
                    if (target.exists() && !target.isDirectory()) {
                        when (request.resolveConflict(target, ArchiveEntry(path, true))) {
                            ArchiveConflictAction.SKIP -> continue
                            ArchiveConflictAction.ABORT -> throw ArchiveException("Target already exists: ${target.displayName}")
                            ArchiveConflictAction.REPLACE -> target.deleteIfExists()
                        }
                    }
                    if (!target.exists()) target.createDirectory()
                } else {
                    target.parent()?.ensureDirectory()
                    if (target.exists() && !request.overwrite) {
                        when (request.resolveConflict(target, ArchiveEntry(path, false, header.uncompressedSize, header.lastModifiedTimeEpoch, header.isEncrypted))) {
                            ArchiveConflictAction.SKIP -> continue
                            ArchiveConflictAction.ABORT -> throw ArchiveException("Target already exists: ${target.displayName}")
                            ArchiveConflictAction.REPLACE -> Unit
                        }
                    }
                    zip.getInputStream(header).use { input ->
                        target.openOutputStream(request.overwrite || target.exists()).use { output ->
                            val copied = copyWithProgress(
                                input,
                                output,
                                request.cancellation,
                                completed,
                                total,
                            ) { bytes ->
                                completed = bytes
                                listener?.onProgress(ArchiveProgress(completed, total, completedEntries, headers.size.toLong()))
                            }
                            completed = copied
                        }
                    }
                }
                completedEntries++
                listener?.onProgress(ArchiveProgress(completed, total, completedEntries, headers.size.toLong()))
            }
        }
    }

    private fun listTar(source: ArchiveSource): List<ArchiveEntry> {
        openTarInput(source).use { input ->
            val entries = mutableListOf<ArchiveEntry>()
            var entry = input.nextTarEntry
            while (entry != null) {
                entries += ArchiveEntry(
                    name = ArchivePathPolicy.normalizeEntryName(entry.name),
                    isDirectory = entry.isDirectory,
                    size = entry.size,
                    lastModifiedEpochMillis = entry.lastModifiedDate?.time,
                    isSymbolicLink = entry.isSymbolicLink || entry.isLink,
                )
                entry = input.nextTarEntry
            }
            return entries
        }
    }

    private fun listCompressedStream(source: ArchiveSource): List<ArchiveEntry> {
        // Opening the stream here makes an extension-only false positive fail
        // during probing/listing instead of producing an unusable empty row.
        openCompressorInput(source).use { }
        return listOf(
            ArchiveEntry(
                compressedOutputName(source.displayName),
                isDirectory = false,
                size = 0L,
            )
        )
    }

    private fun extractCompressedStream(
        source: ArchiveSource,
        destination: ArchiveTarget,
        request: ExtractRequest,
        listener: ArchiveProgressListener?,
    ) {
        val path = ArchivePathPolicy.normalizeEntryName(compressedOutputName(source.displayName))
        if (request.entries != null && request.entries.none {
                it == path || ArchivePathPolicy.isChildOf(it, path)
            }) {
            return
        }
        val target = destination.resolve(path)
        if (target.exists() && !request.overwrite) {
            when (request.resolveConflict(target, ArchiveEntry(path, false))) {
                ArchiveConflictAction.SKIP -> return
                ArchiveConflictAction.ABORT -> throw ArchiveException("Target already exists: ${target.displayName}")
                ArchiveConflictAction.REPLACE -> Unit
            }
        }
        target.parent()?.ensureDirectory()
        val total = source.size.coerceAtLeast(0L)
        var completed = 0L
        openCompressorInput(source).use { input ->
            target.openOutputStream(overwrite = request.overwrite || target.exists()).use { output ->
                val buffer = ByteArray(DEFAULT_BUFFER_SIZE)
                while (true) {
                    request.cancellation.throwIfCancelled()
                    val count = input.read(buffer)
                    if (count < 0) break
                    output.write(buffer, 0, count)
                    completed += count
                    listener?.onProgress(
                        ArchiveProgress(completed, total, 0, 1),
                    )
                }
            }
        }
        listener?.onProgress(ArchiveProgress(completed, total, 1, 1))
    }

    private fun openCompressorInput(source: ArchiveSource): InputStream {
        val buffered = BufferedInputStream(source.openInputStream())
        return try {
            val factory = CompressorStreamFactory()
            // Brotli has no identifying magic bytes and therefore is not part
            // of Commons Compress' auto-detection set. Extension-directed
            // construction also makes raw .zst/.xz/.lzma streams reliable
            // when a provider does not expose a seekable channel.
            val suffix = source.displayName.substringAfterLast('.', "")
                .lowercase(Locale.ROOT)
            val compressor = when (suffix) {
                "br" -> CompressorStreamFactory.BROTLI
                "gz" -> CompressorStreamFactory.GZIP
                "bz2" -> CompressorStreamFactory.BZIP2
                "xz" -> CompressorStreamFactory.XZ
                "lzma" -> CompressorStreamFactory.LZMA
                "zst", "zstd" -> CompressorStreamFactory.ZSTANDARD
                "lz4" -> CompressorStreamFactory.LZ4_FRAMED
                "z" -> CompressorStreamFactory.Z
                else -> null
            }
            if (compressor == null) {
                factory.createCompressorInputStream(buffered)
            } else {
                factory.createCompressorInputStream(compressor, buffered)
            }
        } catch (error: Throwable) {
            buffered.close()
            throw ArchiveException("Unsupported compression stream: ${source.displayName}", error)
        }
    }

    private fun compressedOutputName(name: String): String {
        val base = name.substringAfterLast('/').substringAfterLast('\\')
        val lower = base.lowercase(Locale.ROOT)
        val suffixes = listOf(
            ".tar.gz", ".tar.bz2", ".tar.xz", ".tar.lzma", ".tar.zst", ".tar.lz4",
            ".tar.br", ".tar.lzip", ".tgz", ".tbz", ".tbz2", ".txz", ".gz", ".bz2",
            ".xz", ".lzma", ".zst", ".zstd", ".lz4", ".br", ".lzip", ".z",
        )
        return suffixes.firstOrNull { lower.endsWith(it) }
            ?.let { base.dropLast(it.length) }
            ?.ifBlank { "decompressed" }
            ?: "$base.decompressed"
    }

    private fun extractTar(
        source: ArchiveSource,
        destination: ArchiveTarget,
        request: ExtractRequest,
        listener: ArchiveProgressListener?,
    ) {
        openTarInput(source).use { input ->
            val entries = listTar(source)
            entries.firstOrNull { it.isSymbolicLink }?.let { entry ->
                throw ArchiveException("Symbolic links are not extracted: ${entry.name}")
            }
            val total = entries.sumOf { it.size.coerceAtLeast(0L) }
            var completed = 0L
            var index = 0L
            var entry = input.nextTarEntry
            while (entry != null) {
                request.cancellation.throwIfCancelled()
                val path = ArchivePathPolicy.normalizeEntryName(entry.name)
                if (entry.isSymbolicLink || entry.isLink) {
                    throw ArchiveException("Symbolic links are not extracted: $path")
                }
                val selected = request.entries
                if (path.isNotEmpty() && (selected == null || selected.any { it == path || ArchivePathPolicy.isChildOf(it, path) })) {
                    val target = destination.resolve(path)
                    if (entry.isDirectory) {
                        if (target.exists() && !target.isDirectory()) {
                            when (request.resolveConflict(target, ArchiveEntry(path, true))) {
                                ArchiveConflictAction.SKIP -> {
                                    input.skip(entry.size)
                                    entry = input.nextTarEntry
                                    continue
                                }
                                ArchiveConflictAction.ABORT -> throw ArchiveException("Target already exists: ${target.displayName}")
                                ArchiveConflictAction.REPLACE -> target.deleteIfExists()
                            }
                        }
                        if (!target.exists()) target.createDirectory()
                    } else {
                        target.parent()?.ensureDirectory()
                        if (target.exists() && !request.overwrite) {
                            when (request.resolveConflict(target, ArchiveEntry(path, false, entry.size, entry.lastModifiedDate?.time))) {
                                ArchiveConflictAction.SKIP -> {
                                    input.skip(entry.size)
                                    entry = input.nextTarEntry
                                    continue
                                }
                                ArchiveConflictAction.ABORT -> throw ArchiveException("Target already exists: ${target.displayName}")
                                ArchiveConflictAction.REPLACE -> Unit
                            }
                        }
                        target.openOutputStream(request.overwrite || target.exists()).use { output ->
                            completed = copyWithProgress(input, output, request.cancellation, completed, total) { bytes ->
                                completed = bytes
                                listener?.onProgress(ArchiveProgress(completed, total, index, entries.size.toLong()))
                            }
                        }
                    }
                } else {
                    input.skip(entry.size)
                }
                index++
                listener?.onProgress(ArchiveProgress(completed, total, index, entries.size.toLong()))
                entry = input.nextTarEntry
            }
        }
    }

    private fun createZip(request: CreateArchiveRequest, listener: ArchiveProgressListener?) {
        val output = tempStore.newFile("zipxtract-create-", ".zip")
        val options = request.options
        val parameters = ZipParameters().apply {
            compressionMethod = if (options.zipCompression == ZipCompression.STORE) {
                CompressionMethod.STORE
            } else {
                CompressionMethod.DEFLATE
            }
            compressionLevel = compressionLevel(options.zipCompressionLevel)
            encryptionMethod = encryptionMethod(options.zipEncryption)
            isEncryptFiles = options.zipEncryption != ArchiveEncryption.NONE && request.password?.isNotEmpty() == true
            aesKeyStrength = when (options.zipAesKeyBits) {
                128 -> AesKeyStrength.KEY_STRENGTH_128
                else -> AesKeyStrength.KEY_STRENGTH_256
            }
            isIncludeRootFolder = false
        }
        val splitStream = SplitOutputStream(output, options.zipSplitSizeBytes ?: -1L)
        try {
            ZipOutputStream(splitStream, request.password).use { zip ->
                val total = request.sources.sumOf { it.size.coerceAtLeast(0L) }
                var completed = 0L
                for (entry in request.sources) {
                    request.cancellation.throwIfCancelled()
                    val normalizedName = ArchivePathPolicy.normalizeEntryName(entry.name)
                    parameters.fileNameInZip = if (entry.isDirectory && !normalizedName.endsWith('/')) {
                        "$normalizedName/"
                    } else {
                        normalizedName
                    }
                    parameters.entrySize = entry.size.coerceAtLeast(0L)
                    zip.putNextEntry(parameters)
                    if (!entry.isDirectory) {
                        val input = entry.openInputStream ?: throw ArchiveException("Missing source stream: ${entry.name}")
                        input().use { stream ->
                            completed = copyWithProgress(stream, zip, request.cancellation, completed, total) { bytes ->
                                completed = bytes
                                listener?.onProgress(ArchiveProgress(completed, total, 0, request.sources.size.toLong()))
                            }
                        }
                    }
                    zip.closeEntry()
                    listener?.onProgress(ArchiveProgress(completed, total, 0, request.sources.size.toLong()))
                }
            }
        } finally {
            runCatching { splitStream.close() }
        }
        copySplitZipToTarget(output, request.destination)
    }

    private fun copySplitZipToTarget(output: File, target: ArchiveTarget) {
        val allParts = output.parentFile?.listFiles()
            ?.filter { file ->
                file.name == output.name ||
                    file.name.matches(Regex("${Regex.escape(output.nameWithoutExtension)}\\.z\\d{2,}"))
            }
            ?.sortedWith(
                compareBy<File> { it.name == output.name }
                    .thenBy { file ->
                        if (file.name == output.name) Int.MAX_VALUE else {
                            file.name.substringAfterLast('.', "")
                                .removePrefix("z")
                                .toIntOrNull() ?: Int.MAX_VALUE - 1
                        }
                    }
                    .thenBy { it.name },
            )
            .orEmpty()
        if (allParts.isEmpty()) throw ArchiveException("ZIP creation produced no output")
        val copied = mutableListOf<ArchiveTarget>()
        try {
            for (part in allParts) {
                val destination = if (part.name == output.name) {
                    target
                } else {
                    val suffix = part.name.substringAfterLast('.', "")
                    target.resolveSibling("${target.displayName.substringBeforeLast('.', target.displayName)}.$suffix")
                }
                copyFileToTarget(part, destination, overwrite = false)
                copied += destination
            }
        } catch (error: Throwable) {
            copied.forEach { runCatching { it.deleteIfExists() } }
            throw error
        }
    }

    private fun createTar(request: CreateArchiveRequest, listener: ArchiveProgressListener?) {
        val output = tempStore.newFile("zipxtract-create-", ".tar")
        var completed = 0L
        val total = request.sources.sumOf { it.size.coerceAtLeast(0L) }
        openTarOutput(
            output,
            request.options.tarCompression,
            request.options.tarZstdLevel,
        ).use { tar ->
            tar.setLongFileMode(TarArchiveOutputStream.LONGFILE_POSIX)
            for (entry in request.sources) {
                request.cancellation.throwIfCancelled()
                val name = ArchivePathPolicy.normalizeEntryName(entry.name)
                val tarEntry = TarArchiveEntry(name, entry.isDirectory).apply {
                    this.size = if (entry.isDirectory) 0 else entry.size.coerceAtLeast(0L)
                    entry.lastModifiedEpochMillis?.let { setModTime(Date(it)) }
                }
                tar.putArchiveEntry(tarEntry)
                if (!entry.isDirectory) {
                    val input = entry.openInputStream ?: throw ArchiveException("Missing source stream: ${entry.name}")
                    input().use { stream ->
                        completed = copyWithProgress(stream, tar, request.cancellation, completed, total) { bytes ->
                            completed = bytes
                            listener?.onProgress(ArchiveProgress(completed, total, 0, request.sources.size.toLong()))
                        }
                    }
                }
                tar.closeArchiveEntry()
            }
        }
        copyFileToTarget(output, request.destination, overwrite = false)
    }

    private fun openTarInput(source: ArchiveSource): TarArchiveInputStream {
        // Do not optimistically probe a plain tar stream with
        // CompressorStreamFactory: a failed probe may consume bytes from a
        // provider stream that cannot be rewound. The detector has already
        // classified `.tar.*` by suffix/magic, so only wrap explicit
        // compressed-tar inputs. Use the extension-directed path here as well
        // as for single streams: Brotli and raw LZMA have no reliable magic
        // bytes, so Commons Compress cannot auto-detect them.
        return if (ArchiveFormatDetector.detect(source).format == ArchiveFormat.COMPRESSED_TAR) {
            TarArchiveInputStream(openCompressorInput(source))
        } else {
            TarArchiveInputStream(BufferedInputStream(source.openInputStream()))
        }
    }

    private fun openTarOutput(
        file: File,
        compression: TarCompression,
        zstdLevel: Int,
    ): TarArchiveOutputStream {
        val output = BufferedOutputStream(FileOutputStream(file))
        val compressed: OutputStream = when (compression) {
            TarCompression.NONE -> output
            TarCompression.GZIP -> GzipCompressorOutputStream(output)
            TarCompression.BZIP2 -> BZip2CompressorOutputStream(output)
            TarCompression.XZ -> XZCompressorOutputStream(output)
            TarCompression.LZMA -> LZMACompressorOutputStream(output)
            TarCompression.ZSTD -> ZstdCompressorOutputStream(output, zstdLevel.coerceIn(0, 22))
        }
        return TarArchiveOutputStream(compressed)
    }

    private fun compressionLevel(level: Int): CompressionLevel = when (level.coerceIn(0, 9)) {
        0 -> CompressionLevel.NO_COMPRESSION
        1 -> CompressionLevel.FASTEST
        2 -> CompressionLevel.FASTER
        3 -> CompressionLevel.FAST
        4 -> CompressionLevel.MEDIUM_FAST
        5 -> CompressionLevel.NORMAL
        6 -> CompressionLevel.HIGHER
        7 -> CompressionLevel.MAXIMUM
        8 -> CompressionLevel.PRE_ULTRA
        else -> CompressionLevel.ULTRA
    }

    private fun encryptionMethod(encryption: ArchiveEncryption): EncryptionMethod = when (encryption) {
        ArchiveEncryption.ZIP_STANDARD -> EncryptionMethod.ZIP_STANDARD
        ArchiveEncryption.ZIP_STANDARD_STRONG -> EncryptionMethod.ZIP_STANDARD_VARIANT_STRONG
        ArchiveEncryption.AES -> EncryptionMethod.AES
        else -> EncryptionMethod.NONE
    }

    private fun isZipSymbolicLink(header: net.lingala.zip4j.model.FileHeader): Boolean {
        // Zip external attributes store a Unix mode in the high two bytes.
        // Only honor that mode when the creator platform is Unix; DOS
        // attributes use the low bytes and must not be mistaken for a link.
        val attributes = header.externalFileAttributes ?: return false
        if (attributes.size < 4 || ((header.versionMadeBy ushr 8) and 0xff) != 3) {
            return false
        }
        val mode = ((attributes[3].toInt() and 0xff) shl 8) or
            (attributes[2].toInt() and 0xff)
        return mode and 0xf000 == 0xa000
    }

    private fun isSevenZipZipVolume(name: String): Boolean {
        val base = name.substringAfterLast('/').substringAfterLast('\\').lowercase(Locale.ROOT)
        return base.matches(Regex(".+\\.zip\\.\\d{3}")) ||
            base.matches(Regex(".+\\.zip\\.part\\d+"))
    }

    private fun copyFileToTarget(file: File, target: ArchiveTarget, overwrite: Boolean) {
        val existed = target.exists()
        if (existed && !overwrite) {
            throw ArchiveException("Target already exists: ${target.displayName}")
        }
        try {
            file.inputStream().use { input ->
                target.openOutputStream(overwrite).use { output -> input.copyTo(output) }
            }
        } catch (error: Throwable) {
            // A failed provider write can leave a truncated newly-created
            // archive behind. Never remove a pre-existing user file, but do
            // clean the partial target when this operation created it.
            if (!existed) runCatching { target.deleteIfExists() }
            throw error
        }
    }

    private fun validateInputEntries(entries: List<ArchiveInputEntry>) {
        val names = HashSet<String>()
        entries.forEach { entry ->
            val normalized = ArchivePathPolicy.normalizeEntryName(entry.name)
            require(normalized.isNotEmpty()) { "Archive entry name must not be empty" }
            require(names.add(ArchivePathPolicy.canonicalKey(normalized))) {
                "Duplicate archive entry: $normalized"
            }
            require(entry.size >= 0L) { "Archive entry size must not be negative" }
        }
    }

    private fun copyWithProgress(
        input: InputStream,
        output: OutputStream,
        cancellation: CancellationToken,
        initial: Long,
        total: Long,
        onProgress: (Long) -> Unit,
    ): Long {
        val buffer = ByteArray(DEFAULT_BUFFER_SIZE)
        var completed = initial
        while (true) {
            cancellation.throwIfCancelled()
            val count = input.read(buffer)
            if (count < 0) break
            output.write(buffer, 0, count)
            completed += count
            onProgress(completed.coerceAtMost(total.coerceAtLeast(completed)))
        }
        return completed
    }

    private fun ArchiveTarget.ensureDirectory() {
        parent()?.ensureDirectory()
        if (!exists()) createDirectory()
        if (!isDirectory()) throw ArchiveException("Target is not a directory: $displayName")
    }

    private fun ExtractRequest.resolveConflict(
        target: ArchiveTarget,
        entry: ArchiveEntry,
    ): ArchiveConflictAction = onConflict?.invoke(target, entry)
        ?: if (overwrite) ArchiveConflictAction.REPLACE else ArchiveConflictAction.ABORT

    private inline fun <T> withStagedVolumes(
        source: ArchiveSource,
        probe: ArchiveProbe,
        block: (primary: File, volumes: List<File>) -> T,
    ): T {
        val resolved = MultipartArchiveResolver.resolve(source, probe)
        if (resolved.missingVolumes.isNotEmpty()) {
            throw ArchiveMissingVolumeException(resolved.missingVolumes)
        }
        val staged = resolved.volumes.map { volume ->
            tempStore.stage(volume, volume.displayName.substringAfterLast('/').substringAfterLast('\\'))
        }
        val primaryName = resolved.primary.displayName.substringAfterLast('/').substringAfterLast('\\')
        val primary = staged.firstOrNull { it.name.equals(primaryName, ignoreCase = true) }
            ?: staged.firstOrNull()
            ?: throw ArchiveException("Archive has no readable volume")
        return block(primary, staged)
    }

    private fun listSevenZip(
        primary: File,
        volumes: List<File>,
        password: CharArray?,
        format: ArchiveFormat,
    ): List<ArchiveEntry> {
        val handles = SevenZipHandles.open(primary, volumes, password, format)
        return try {
            val archive = handles.archive
            val archiveEncrypted = archive.getArchiveProperty(PropID.ENCRYPTED) as? Boolean ?: false
            buildList {
                for (index in 0 until archive.numberOfItems) {
                    val path = ArchivePathPolicy.normalizeEntryName(archive.getStringProperty(index, PropID.PATH) ?: continue)
                    if (path.isNotEmpty()) {
                        add(
                            ArchiveEntry(
                                path,
                                archive.getProperty(index, PropID.IS_FOLDER) as? Boolean ?: false,
                                archive.getProperty(index, PropID.SIZE) as? Long ?: 0L,
                                (archive.getProperty(index, PropID.LAST_MODIFICATION_TIME) as? Date)?.time,
                                encrypted = archiveEncrypted ||
                                    (archive.getProperty(index, PropID.ENCRYPTED) as? Boolean ?: false),
                                isSymbolicLink = archive.isArchiveLink(index),
                            )
                        )
                    }
                }
            }
        } finally {
            handles.close()
        }
    }

    private fun extractSevenZip(
        primary: File,
        volumes: List<File>,
        destination: ArchiveTarget,
        request: ExtractRequest,
        listener: ArchiveProgressListener?,
        format: ArchiveFormat,
    ) {
        val handles = SevenZipHandles.open(primary, volumes, request.password, format)
        try {
            val archive = handles.archive
            val selected = request.entries
            val indices = if (selected == null) null else buildList {
                for (index in 0 until archive.numberOfItems) {
                    val path = archive.getStringProperty(index, PropID.PATH)?.replace('\\', '/') ?: continue
                    if (selected.any { it == path || ArchivePathPolicy.isChildOf(it, path) }) add(index)
                }
            }.toIntArray()
            val inspectedIndices = indices ?: IntArray(archive.numberOfItems) { it }
            inspectedIndices.firstOrNull { archive.isArchiveLink(it) }?.let { index ->
                val path = archive.getStringProperty(index, PropID.PATH) ?: "<unnamed>"
                throw ArchiveException("Archive links are not extracted: $path")
            }
            val callback = SevenZipExtractCallback(archive, destination, request, listener)
            archive.extract(indices, false, callback)
            callback.throwIfFailed()
        } finally {
            handles.close()
        }
    }

    private fun createSevenZip(request: CreateArchiveRequest, listener: ArchiveProgressListener?) {
        val output = tempStore.newFile("zipxtract-create-", ".7z")
        RandomAccessFile(output, "rw").use { randomAccess ->
            val archive = SevenZip.openOutArchive7z()
            try {
                archive.setLevel(request.options.sevenZipCompressionLevel.coerceIn(0, 9))
                archive.setSolid(request.options.sevenZipSolid)
                archive.setSolidSize(8192)
                archive.setThreadCount(request.options.sevenZipThreadCount.coerceAtLeast(1))
                if (request.password?.isNotEmpty() == true) archive.setHeaderEncryption(true)
                val callback = SevenZipCreateCallback(request, listener)
                try {
                    archive.createArchive(
                        RandomAccessFileOutStream(randomAccess),
                        request.sources.size,
                        callback,
                    )
                } finally {
                    callback.close()
                }
            } finally {
                archive.close()
            }
        }
        copyFileToTarget(output, request.destination, overwrite = false)
    }

    private fun updateSevenZip(
        primary: File,
        volumes: List<File>,
        request: Update7zRequest,
        listener: ArchiveProgressListener?,
    ) {
        val handles = SevenZipHandles.open(primary, volumes, null, ArchiveFormat.SEVEN_ZIP)
        val temporary = tempStore.newFile("zipxtract-update-", ".7z")
        try {
            val input = handles.archive
            val output = input.connectedOutArchive
                ?: throw UnsupportedArchiveException("This 7z archive cannot be updated")
            val removed = request.removals.map { ArchivePathPolicy.normalizeEntryName(it) }.toSet()
            val removeIndexes = buildList {
                for (index in 0 until input.numberOfItems) {
                    val path = input.getStringProperty(index, PropID.PATH)?.replace('\\', '/') ?: continue
                    if (removed.any { it == path || ArchivePathPolicy.isChildOf(it, path) }) add(index)
                }
            }
            val newCount = input.numberOfItems - removeIndexes.size + request.additions.size
            RandomAccessFile(temporary, "rw").use { randomAccess ->
                val callback = SevenZipUpdateCallback(input, removeIndexes, request, listener)
                try {
                    output.updateItems(
                        RandomAccessFileOutStream(randomAccess),
                        newCount,
                        callback,
                    )
                } finally {
                    callback.close()
                }
            }
        } finally {
            handles.close()
        }
        copyFileToTarget(temporary, request.destination, overwrite = true)
    }

    companion object {
        private const val DEFAULT_BUFFER_SIZE = 64 * 1024
    }
}

private fun ArchiveTarget.ensureDirectoryRecursively() {
    parent()?.ensureDirectoryRecursively()
    if (!exists()) createDirectory()
    if (!isDirectory()) throw ArchiveException("Target is not a directory: $displayName")
}

private class SevenZipExtractCallback(
    private val archive: IInArchive,
    private val destination: ArchiveTarget,
    private val request: ExtractRequest,
    private val listener: ArchiveProgressListener?,
) : IArchiveExtractCallback, ICryptoGetTextPassword {
    private var output: OutputStream? = null
    private var currentTarget: ArchiveTarget? = null
    private var total = 0L
    private var completed = 0L
    private var failed: Throwable? = null
    private var currentIndex = -1

    override fun setTotal(total: Long) {
        this.total = total
    }

    override fun setCompleted(complete: Long) {
        request.cancellation.throwIfCancelled()
        completed = complete
        listener?.onProgress(ArchiveProgress(completed, total, 0, archive.numberOfItems.toLong()))
    }

    override fun prepareOperation(askExtractMode: ExtractAskMode?) = Unit

    override fun getStream(index: Int, askExtractMode: ExtractAskMode?): ISequentialOutStream {
        currentIndex = index
        val rawPath = archive.getStringProperty(index, PropID.PATH)
            ?: throw SevenZipException("Archive entry has no path")
        val path = ArchivePathPolicy.normalizeEntryName(rawPath)
        if (path.isEmpty()) {
            throw SevenZipException("Archive entry has no usable path")
        }
        if (archive.isArchiveLink(index)) {
            throw SevenZipException("Archive links are not extracted: $path")
        }
        val target = destination.resolve(path)
        currentTarget = target
        val isDirectory = archive.getProperty(index, PropID.IS_FOLDER) as? Boolean ?: false
        if (isDirectory) {
            if (target.exists() && !target.isDirectory()) target.deleteIfExists()
            if (!target.exists()) target.createDirectory()
            // 7-Zip still expects a sink for directory callbacks. Creating the
            // directory before returning means archives that emit no zero-byte
            // callback for folders are handled correctly as well.
            return ISequentialOutStream { data -> data.size }
        }
        if (target.exists() && !request.overwrite) {
            val entry = ArchiveEntry(path, isDirectory)
            when (request.onConflict?.invoke(target, entry)
                ?: if (request.overwrite) ArchiveConflictAction.REPLACE else ArchiveConflictAction.ABORT) {
                ArchiveConflictAction.SKIP -> return ISequentialOutStream { data -> data.size }
                ArchiveConflictAction.ABORT -> throw SevenZipException("Target already exists: ${target.displayName}")
                ArchiveConflictAction.REPLACE -> Unit
            }
        }
        return ISequentialOutStream { data ->
            try {
                target.parent()?.ensureDirectoryRecursively()
                if (output == null) output = target.openOutputStream(overwrite = true)
                output!!.write(data)
                data.size
            } catch (error: Throwable) {
                failed = error
                throw SevenZipException(error.message ?: "Unable to write extracted entry")
            }
        }
    }

    override fun setOperationResult(operationResult: ExtractOperationResult?) {
        output?.close()
        output = null
        when (operationResult) {
            ExtractOperationResult.OK -> Unit
            ExtractOperationResult.WRONG_PASSWORD -> failed = ArchivePasswordException("Wrong archive password")
            ExtractOperationResult.UNSUPPORTEDMETHOD -> failed = UnsupportedArchiveException("Unsupported compression method")
            else -> failed = ArchiveException(operationResult?.name ?: "Archive extraction failed")
        }
    }

    override fun cryptoGetTextPassword(): String = request.password?.concatToString() ?: ""

    fun throwIfFailed() {
        failed?.let { throw it }
    }
}

private fun IInArchive.isArchiveLink(index: Int): Boolean =
    getProperty(index, PropID.SYM_LINK) != null ||
        getProperty(index, PropID.HARD_LINK) != null

private class SevenZipCreateCallback(
    private val request: CreateArchiveRequest,
    private val listener: ArchiveProgressListener?,
) : IOutCreateCallback<IOutItem7z>, ICryptoGetTextPassword, Closeable {
    private val total = request.sources.sumOf { it.size.coerceAtLeast(0L) }
    private val streams = mutableListOf<InputStreamSequentialInStream>()

    override fun setOperationResult(operationResultOk: Boolean) {
        if (!operationResultOk) throw SevenZipException("7-Zip creation failed")
    }

    override fun setTotal(total: Long) = Unit

    override fun setCompleted(complete: Long) {
        request.cancellation.throwIfCancelled()
        listener?.onProgress(ArchiveProgress(complete, total, 0, request.sources.size.toLong()))
    }

    override fun getItemInformation(index: Int, outItemFactory: OutItemFactory<IOutItem7z>): IOutItem7z {
        val source = request.sources[index]
        return outItemFactory.createOutItem().apply {
            propertyPath = ArchivePathPolicy.normalizeEntryName(source.name)
            propertyIsDir = source.isDirectory
            dataSize = source.size.coerceAtLeast(0L)
            source.lastModifiedEpochMillis?.let { propertyLastModificationTime = Date(it) }
        }
    }

    override fun getStream(index: Int): ISequentialInStream? {
        val source = request.sources[index]
        if (source.isDirectory) return null
        val input = source.openInputStream ?: throw SevenZipException("Missing source stream")
        return InputStreamSequentialInStream(input()).also { streams += it }
    }

    override fun cryptoGetTextPassword(): String? = request.password?.concatToString()

    override fun close() {
        streams.forEach { runCatching { it.close() } }
        streams.clear()
    }
}

private class SevenZipUpdateCallback(
    private val input: IInArchive,
    private val removedIndexes: List<Int>,
    private val request: Update7zRequest,
    private val listener: ArchiveProgressListener?,
) : IOutCreateCallback<net.sf.sevenzipjbinding.IOutItemAllFormats>, Closeable {
    private val keptCount = input.numberOfItems - removedIndexes.size
    private val streams = mutableListOf<InputStreamSequentialInStream>()

    override fun setOperationResult(operationResultOk: Boolean) {
        if (!operationResultOk) throw SevenZipException("7-Zip update failed")
    }

    override fun setTotal(total: Long) = Unit

    override fun setCompleted(complete: Long) {
        request.cancellation.throwIfCancelled()
        listener?.onProgress(ArchiveProgress(complete, 0, 0, (keptCount + request.additions.size).toLong()))
    }

    override fun getItemInformation(
        index: Int,
        outItemFactory: OutItemFactory<net.sf.sevenzipjbinding.IOutItemAllFormats>,
    ): net.sf.sevenzipjbinding.IOutItemAllFormats {
        if (index >= keptCount) {
            val source = request.additions[index - keptCount]
            return outItemFactory.createOutItem().apply {
                propertyPath = ArchivePathPolicy.normalizeEntryName(source.name)
                propertyIsDir = source.isDirectory
                dataSize = source.size.coerceAtLeast(0L)
                source.lastModifiedEpochMillis?.let { propertyLastModificationTime = Date(it) }
            }
        }
        var oldIndex = index
        for (removed in removedIndexes) if (oldIndex >= removed) oldIndex++
        return outItemFactory.createOutItem(oldIndex)
    }

    override fun getStream(index: Int): ISequentialInStream? {
        if (index < keptCount) return null
        val source = request.additions[index - keptCount]
        if (source.isDirectory) return null
        val input = source.openInputStream ?: throw SevenZipException("Missing source stream")
        return InputStreamSequentialInStream(input()).also { streams += it }
    }

    override fun close() {
        streams.forEach { runCatching { it.close() } }
        streams.clear()
    }
}

private class InputStreamSequentialInStream(
    private val input: InputStream,
) : ISequentialInStream, Closeable {
    override fun read(data: ByteArray): Int {
        val count = input.read(data)
        return if (count < 0) 0 else count
    }

    override fun close() {
        input.close()
    }
}

private class SevenZipOpenCallback(
    private val password: CharArray?,
) : IArchiveOpenCallback, ICryptoGetTextPassword {
    override fun setCompleted(files: Long?, bytes: Long?) = Unit
    override fun setTotal(files: Long?, bytes: Long?) = Unit
    override fun cryptoGetTextPassword(): String = password?.concatToString() ?: ""
}

private class SevenZipVolumeCallback(
    private val directory: File,
    private val password: CharArray?,
) : IArchiveOpenVolumeCallback, IArchiveOpenCallback, ICryptoGetTextPassword {
    private val openFiles = mutableMapOf<String, java.io.RandomAccessFile>()
    private var currentName: String? = null

    override fun getProperty(propID: PropID): Any? =
        if (propID == PropID.NAME) currentName else null

    override fun getStream(filename: String): IInStream? {
        val name = File(filename).name
        val file = File(directory, name)
        if (!file.isFile) return null
        val randomAccess = openFiles.getOrPut(name) { java.io.RandomAccessFile(file, "r") }
        randomAccess.seek(0)
        currentName = name
        return RandomAccessFileInStream(randomAccess)
    }

    override fun setCompleted(files: Long?, bytes: Long?) = Unit
    override fun setTotal(files: Long?, bytes: Long?) = Unit
    override fun cryptoGetTextPassword(): String = password?.concatToString() ?: ""

    fun close() {
        openFiles.values.forEach { runCatching { it.close() } }
        openFiles.clear()
    }
}

private class SevenZipHandles private constructor(
    val archive: IInArchive,
    private val input: IInStream,
    private val volumeCallback: SevenZipVolumeCallback?,
) : Closeable {
    override fun close() {
        runCatching { archive.close() }
        runCatching { input.close() }
        volumeCallback?.close()
    }

    companion object {
        fun open(
            primary: File,
            volumes: List<File>,
            password: CharArray?,
            format: ArchiveFormat,
        ): SevenZipHandles {
            val callback = if (volumes.size > 1) SevenZipVolumeCallback(primary.parentFile ?: primary, password) else null
            val input = if (callback != null && format != ArchiveFormat.RAR) {
                VolumedArchiveInStream(primary.name, callback)
            } else {
                RandomAccessFileInStream(java.io.RandomAccessFile(primary, "r"))
            }
            return try {
                val openCallback = if (callback != null && format == ArchiveFormat.RAR) {
                    callback
                } else {
                    SevenZipOpenCallback(password)
                }
                val archive = SevenZip.openInArchive(null, input, openCallback)
                SevenZipHandles(archive, input, callback)
            } catch (error: Throwable) {
                runCatching { input.close() }
                callback?.close()
                throw error
            }
        }
    }
}
