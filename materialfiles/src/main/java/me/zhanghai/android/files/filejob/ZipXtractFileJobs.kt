/*
 * Copyright (C) 2026 RongVualt contributors
 *
 * The jobs in this file are deliberately thin: FileJobService owns the
 * foreground lifecycle while zipxtract-core owns archive format handling.
 */

package me.zhanghai.android.files.filejob

import java.io.IOException
import java8.nio.file.FileVisitResult
import java8.nio.file.Files
import java8.nio.file.LinkOption
import java8.nio.file.Path
import java8.nio.file.SimpleFileVisitor
import java8.nio.file.StandardCopyOption
import java8.nio.file.attribute.BasicFileAttributes
import me.zhanghai.android.files.provider.archive.createArchiveRootPath
import me.zhanghai.android.files.provider.archive.zipxtract.PathArchiveSource
import me.zhanghai.android.files.provider.archive.zipxtract.PathArchiveTarget
import me.zhanghai.android.files.provider.common.deleteIfExists
import me.zhanghai.android.files.provider.common.exists
import me.zhanghai.android.files.provider.common.isDirectory
import me.zhanghai.android.files.provider.common.moveTo
import me.zhanghai.android.files.provider.common.newDirectoryStream
import me.zhanghai.android.files.provider.common.newInputStream
import me.zhanghai.android.files.provider.common.readAttributes
import me.zhanghai.android.files.provider.linux.isLinuxPath
import me.zhanghai.android.files.provider.document.isDocumentPath
import java.util.UUID
import org.eds.zipxtract.core.ArchiveCreateOptions
import org.eds.zipxtract.core.ArchiveEditPolicy
import org.eds.zipxtract.core.ArchiveNames
import org.eds.zipxtract.core.ArchivePathPolicy
import org.eds.zipxtract.core.ArchivePasswordException
import org.eds.zipxtract.core.ArchiveFormat
import org.eds.zipxtract.core.ArchiveInputEntry
import org.eds.zipxtract.core.ArchiveProgressListener
import org.eds.zipxtract.core.CreateArchiveRequest
import org.eds.zipxtract.core.ExtractRequest
import org.eds.zipxtract.core.PrivateArchiveTempStore
import org.eds.zipxtract.core.Update7zRequest
import org.eds.zipxtract.core.ZipXtractArchiveEngine
import org.eds.zipxtract.core.supportsFormat
import org.eds.zipxtract.core.wipe

class ZipXtractExtractJob(
    private val sources: List<Path>,
    private val targetDirectory: Path,
    private val createContainingDirectory: Boolean,
    private val entries: Set<String>? = null,
    private val password: CharArray? = null,
) : FileJob() {
    @Throws(IOException::class)
    override fun run() {
        try {
            withPrivateEngine { engine ->
                sources.forEach { source ->
                    val cancellation = CancellationBridge()
                    val sourceAdapter = PathArchiveSource(source)
                    val detected = engine.probe(sourceAdapter)
                    if (!engine.supportsFormat(detected.format) || source.isLzipArchive()) {
                        // libarchive remains the compatibility fallback for
                        // ISO/CAB/DEB/RPM/WIM and other generic read-only
                        // formats. It still uses the existing archive FS and
                        // therefore retains its password dialog and conflict
                        // behavior. Flattening an archive uses its root's
                        // immediate children, so “extract here” has the same
                        // semantics as the native engine.
                        if (!password.isNullOrEmpty()) {
                            throw IOException("This archive format does not support passwords")
                        }
                        copyLegacyArchive(source, targetDirectory, createContainingDirectory, entries)
                        return@forEach
                    }
                    // ZipXtract can inspect encrypted headers without asking for
                    // a password. Route those whole-archive jobs through the
                    // existing Material Files password action before any output
                    // directory is touched.
                    if (password == null && entries == null &&
                        runCatching { engine.list(sourceAdapter).any { it.encrypted } }
                            .getOrDefault(false)
                    ) {
                        CopyFileJob(listOf(source.createArchiveRootPath()), targetDirectory)
                            .runOn(service)
                        return@forEach
                    }
                    // Reuse Material Files' conflict dialog when a target
                    // already exists. The provider-neutral engine is kept for
                    // the common no-conflict path; a preflight avoids silently
                    // replacing a user's file and preserves rename/skip/all
                    // semantics implemented by CopyFileJob.
                    if (password == null && entries == null &&
                        hasExtractionConflict(
                            engine,
                            source,
                            sourceAdapter,
                            targetDirectory,
                            createContainingDirectory,
                        )
                    ) {
                        copyLegacyArchive(source, targetDirectory, createContainingDirectory, entries)
                        return@forEach
                    }
                    try {
                        engine.extract(
                            ExtractRequest(
                                source = sourceAdapter,
                                destination = PathArchiveTarget(targetDirectory),
                                entries = entries,
                                createContainingDirectory = createContainingDirectory,
                                // The core wipes the request-owned array. Keep
                                // the job's original only until every selected
                                // source has completed so multi-select jobs do
                                // not reuse a cleared password buffer.
                                password = password?.copyOf(),
                                // A preflight routes existing targets through
                                // CopyFileJob's replacement/skip/all dialog.
                                // Keep the core conservative if a race creates
                                // a target between that check and the write.
                                overwrite = false,
                                cancellation = cancellation,
                            ),
                            ArchiveProgressListener { progress ->
                                postArchiveEngineNotification(
                                    "Extracting ${source.fileName}",
                                    progress.completedBytes,
                                    progress.totalBytes,
                                    progress.completedEntries,
                                    progress.totalEntries,
                                )
                            },
                        )
                    } catch (error: ArchivePasswordException) {
                        // Keep the existing Material Files password action in the
                        // loop. It persists the entered passphrase only in the
                        // archive browser and retries through the normal conflict
                        // and notification UI; the ZipXtract path never logs or
                        // stores the password.
                        if (password == null && entries == null) {
                            CopyFileJob(listOf(source.createArchiveRootPath()), targetDirectory)
                                .runOn(service)
                        } else {
                            throw error
                        }
                    }
                }
            }
        } finally {
            password.wipe()
        }
    }
}

class ZipXtractCreateJob(
    private val sources: List<Path>,
    private val archiveFile: Path,
    private val format: ArchiveFormat,
    private val options: ArchiveCreateOptions,
    private val password: CharArray? = null,
) : FileJob() {
    @Throws(IOException::class)
    override fun run() {
        try {
            val inputEntries = collectArchiveInputs(sources)
            val archiveExisted = archiveFile.exists(LinkOption.NOFOLLOW_LINKS)
            var successful = false
            try {
                withPrivateEngine { engine ->
                    engine.create(
                        CreateArchiveRequest(
                            sources = inputEntries,
                            destination = PathArchiveTarget(archiveFile),
                            format = format,
                            options = options,
                            password = password?.copyOf(),
                            cancellation = CancellationBridge(),
                        ),
                        ArchiveProgressListener { progress ->
                            postArchiveEngineNotification(
                                "Creating ${archiveFile.fileName}",
                                progress.completedBytes,
                                progress.totalBytes,
                                progress.completedEntries,
                                progress.totalEntries,
                            )
                        },
                    )
                    successful = true
                }
            } finally {
                if (!successful && !archiveExisted) {
                    runCatching { archiveFile.deleteIfExists() }
                }
            }
        } finally {
            password.wipe()
        }
    }
}

/**
 * Safely replaces an unencrypted 7z archive after writing and re-opening a
 * temporary sibling. The original is kept as a backup until the replacement
 * has been committed, so a provider failure can be rolled back.
 */
class ZipXtractUpdate7zJob(
    private val archive: Path,
    private val additions: List<Path>,
    private val removals: Set<String>,
) : FileJob() {
    @Throws(IOException::class)
    override fun run() {
        if (!(archive.isLinuxPath || archive.isDocumentPath) || archive.fileSystem.isReadOnly) {
            throw IOException("7z editing requires a local or unlocked writable provider")
        }
        val archiveName = archive.fileName?.toString()
            ?: throw IOException("7z editing requires a named archive")
        val recoveryPrefix = ".${archiveName}.rongvault-backup-"
        val tempPrefix = ".${archiveName}.rongvault-temp-"
        recoverInterrupted7zReplacement(archive, recoveryPrefix, tempPrefix)
        // Never reuse or delete a deterministic user-visible sibling. Every
        // operation gets unique private names; recovery scans only this
        // reserved prefix after a process kill.
        val operationId = UUID.randomUUID().toString()
        val temp = archive.resolveSibling("$tempPrefix$operationId")
        val backup = archive.resolveSibling("$recoveryPrefix$operationId")
        val archiveAttributes = archive.readAttributes(
            BasicFileAttributes::class.java,
            LinkOption.NOFOLLOW_LINKS,
        )
        if (archiveAttributes.isSymbolicLink || !archiveAttributes.isRegularFile) {
            throw IOException("7z editing requires a regular archive file")
        }
        val additionEntries = collectArchiveInputs(additions)
        try {
            withPrivateEngine { engine ->
                val probe = engine.probe(PathArchiveSource(archive))
                if (!ArchiveEditPolicy.canUpdate7z(
                        probe,
                        providerSupportsReplacement =
                            (archive.isLinuxPath || archive.isDocumentPath) &&
                                !archive.fileSystem.isReadOnly,
                    )) {
                    throw IOException("Encrypted or unsupported 7z archives are read-only")
                }
                val existingEntries = runCatching { engine.list(PathArchiveSource(archive)) }
                    .getOrElse { throw IOException("Unable to inspect 7z encryption", it) }
                if (existingEntries.any { it.encrypted }) {
                    throw IOException("Encrypted 7z archives are read-only")
                }
                val existingNames = existingEntries.mapTo(HashSet()) {
                    ArchivePathPolicy.canonicalKey(it.name)
                }
                val duplicateAdditions = additionEntries.mapNotNull { entry ->
                    val key = ArchivePathPolicy.canonicalKey(entry.name)
                    key.takeIf { candidate ->
                        existingNames.any { existing ->
                            candidate == existing ||
                                candidate.startsWith("$existing/") ||
                                existing.startsWith("$candidate/")
                        }
                    }
                }.distinct()
                if (duplicateAdditions.isNotEmpty()) {
                    throw IOException(
                        "7z entry already exists: ${duplicateAdditions.joinToString(", ")}",
                    )
                }
                engine.update7z(
                    Update7zRequest(
                        source = PathArchiveSource(archive),
                        destination = PathArchiveTarget(temp),
                        additions = additionEntries,
                        removals = removals,
                        cancellation = CancellationBridge(),
                    ),
                    ArchiveProgressListener { progress ->
                        postArchiveEngineNotification(
                            "Updating ${archive.fileName}",
                            progress.completedBytes,
                            progress.totalBytes,
                            progress.completedEntries,
                            progress.totalEntries,
                        )
                    },
                )
                // Re-open the generated file before touching the original.
                engine.list(PathArchiveSource(temp))
            }
            backup.deleteIfExists()
            archive.moveTo(backup, StandardCopyOption.REPLACE_EXISTING)
            try {
                temp.moveTo(archive, StandardCopyOption.REPLACE_EXISTING)
            } catch (error: IOException) {
                runCatching { backup.moveTo(archive, StandardCopyOption.REPLACE_EXISTING) }
                    .onFailure(error::addSuppressed)
                throw error
            }
            backup.deleteIfExists()
        } finally {
            temp.deleteIfExists()
        }
    }
}

/** Recover/clean only siblings created by the 7z replacement protocol. */
private fun recoverInterrupted7zReplacement(
    archive: Path,
    recoveryPrefix: String,
    tempPrefix: String,
) {
    val parent = archive.parent ?: return
    val (backups, temporaryFiles) = runCatching {
        parent.newDirectoryStream().use { stream ->
            val matching = stream.filter { candidate ->
                val name = candidate.fileName?.toString()
                name?.startsWith(recoveryPrefix) == true || name?.startsWith(tempPrefix) == true
            }.toList()
            matching.partition {
                it.fileName?.toString()?.startsWith(recoveryPrefix) == true
            }
        }
    }.getOrDefault(emptyList<Path>() to emptyList<Path>())
    if (!archive.exists(LinkOption.NOFOLLOW_LINKS)) {
        backups.firstOrNull()?.let { backup ->
            val safeBackup = runCatching {
                val attributes = backup.readAttributes(
                    BasicFileAttributes::class.java,
                    LinkOption.NOFOLLOW_LINKS,
                )
                attributes.isRegularFile && !attributes.isSymbolicLink
            }.getOrDefault(false)
            if (safeBackup) {
                backup.moveTo(archive, StandardCopyOption.REPLACE_EXISTING)
            } else {
                backup.deleteIfExists()
            }
        }
        backups.drop(1).forEach { it.deleteIfExists() }
    } else {
        backups.forEach { it.deleteIfExists() }
    }
    temporaryFiles.forEach { it.deleteIfExists() }
}

private class CancellationBridge : org.eds.zipxtract.core.CancellationToken() {
    override fun throwIfCancelled() {
        if (Thread.currentThread().isInterrupted) cancel()
        super.throwIfCancelled()
    }
}

private inline fun <T> FileJob.withPrivateEngine(block: (ZipXtractArchiveEngine) -> T): T {
    val root = service.cacheDir.resolve("zipxtract")
    if (!root.mkdirs() && !root.isDirectory) {
        throw IOException("Unable to create archive staging directory")
    }
    val store = PrivateArchiveTempStore(root)
    return try {
        block(ZipXtractArchiveEngine(store))
    } finally {
        store.close()
    }
}

private fun collectArchiveInputs(sources: List<Path>): List<ArchiveInputEntry> {
    val result = ArrayList<ArchiveInputEntry>()
    val names = HashSet<String>()
    for (source in sources) {
        val rootName = source.fileName?.toString()?.ifBlank { "entry" } ?: "entry"
        Files.walkFileTree(source, object : SimpleFileVisitor<Path>() {
            @Throws(IOException::class)
            override fun preVisitDirectory(
                directory: Path,
                attributes: BasicFileAttributes,
            ): FileVisitResult {
                addEntry(directory, source, rootName, true, attributes, result, names)
                return FileVisitResult.CONTINUE
            }

            @Throws(IOException::class)
            override fun visitFile(
                file: Path,
                attributes: BasicFileAttributes,
            ): FileVisitResult {
                if (attributes.isSymbolicLink) {
                    throw IOException("Symbolic links are not archived")
                }
                addEntry(file, source, rootName, false, attributes, result, names)
                return FileVisitResult.CONTINUE
            }
        })
    }
    return result
}

/** Commons Compress has no Lzip decoder; keep that uncommon stream on the
 * existing libarchive compatibility path instead of advertising a runtime
 * failure from the ZipXtract engine. */
private fun Path.isLzipArchive(): Boolean =
    fileName?.toString()?.lowercase(java.util.Locale.ROOT)?.endsWith(".lzip") == true

/**
 * Route formats outside the core engine through Material Files' read-only
 * archive filesystem while retaining its password/conflict dialogs. Scan the
 * archive tree first so a provider that exposes links cannot turn the generic
 * compatibility path into an extraction escape.
 */
private fun FileJob.copyLegacyArchive(
    source: Path,
    targetDirectory: Path,
    createContainingDirectory: Boolean,
    entries: Set<String>?,
) {
    val root = source.createArchiveRootPath()
    val sources = if (createContainingDirectory) {
        listOf(root)
    } else {
        root.newDirectoryStream().use { it.toList() }
    }
    if (sources.isEmpty()) return
    sources.forEach { archivePath ->
        Files.walkFileTree(archivePath, object : SimpleFileVisitor<Path>() {
            @Throws(IOException::class)
            override fun preVisitDirectory(
                directory: Path,
                attributes: BasicFileAttributes,
            ): FileVisitResult {
                if (attributes.isSymbolicLink) {
                    throw IOException("Symbolic links are not extracted")
                }
                return FileVisitResult.CONTINUE
            }

            @Throws(IOException::class)
            override fun visitFile(
                file: Path,
                attributes: BasicFileAttributes,
            ): FileVisitResult {
                if (attributes.isSymbolicLink) {
                    throw IOException("Symbolic links are not extracted")
                }
                return FileVisitResult.CONTINUE
            }
        })
    }
    // The core path accepts selected virtual names. Generic extraction is
    // currently whole-archive in the UI; if a caller supplies entries, keep
    // the safe default of copying only matching immediate roots.
    val filteredSources = entries?.takeIf { it.isNotEmpty() }?.let { selected ->
        sources.filter { candidate ->
            val name = candidate.fileName?.toString()?.replace('\\', '/') ?: return@filter false
            selected.any { it == name || it.startsWith("$name/") }
        }
    } ?: sources
    if (filteredSources.isNotEmpty()) {
        CopyFileJob(filteredSources, targetDirectory).runOn(service)
    }
}

/**
 * Check only destination names; archive contents are still streamed by the
 * selected backend. If a conflict is found, CopyFileJob supplies the existing
 * Material Files replacement/skip/rename/all dialog and merge behavior.
 */
private fun hasExtractionConflict(
    engine: ZipXtractArchiveEngine,
    source: Path,
    sourceAdapter: PathArchiveSource,
    targetDirectory: Path,
    createContainingDirectory: Boolean,
): Boolean {
    val probe = engine.probe(sourceAdapter)
    val target = PathArchiveTarget(targetDirectory)
    val extractionRoot = if (createContainingDirectory) {
        target.resolve(ArchiveNames.containingDirectoryName(source.fileName?.toString().orEmpty(), probe.format))
    } else {
        target
    }
    if (createContainingDirectory && extractionRoot.exists()) return true
    val entries = runCatching { engine.list(sourceAdapter) }.getOrNull() ?: return false
    return entries.any { entry ->
        entry.name.isNotEmpty() && runCatching {
            extractionRoot.resolve(entry.name).exists()
        }.getOrDefault(false)
    }
}

private fun addEntry(
    path: Path,
    source: Path,
    rootName: String,
    isDirectory: Boolean,
    attributes: BasicFileAttributes,
    result: MutableList<ArchiveInputEntry>,
    names: MutableSet<String>,
) {
    val relative = if (path == source) "" else source.relativize(path).toString()
    val name = listOf(rootName, relative)
        .filter { it.isNotEmpty() }
        .joinToString("/")
        .replace('\\', '/')
    val normalized = org.eds.zipxtract.core.ArchivePathPolicy.normalizeEntryName(name)
    // Android's shared-storage and most DocumentsProvider targets are
    // case-insensitive in practice. Treat names that differ only by case as
    // duplicates before they reach the archive writer, otherwise extraction
    // would silently overwrite one of them on those targets.
    if (!names.add(org.eds.zipxtract.core.ArchivePathPolicy.canonicalKey(normalized))) {
        throw IOException("Duplicate archive entry: $normalized")
    }
    result += ArchiveInputEntry(
        name = normalized,
        isDirectory = isDirectory,
        size = if (isDirectory) 0 else attributes.size(),
        lastModifiedEpochMillis = attributes.lastModifiedTime().toMillis(),
        openInputStream = if (isDirectory) null else ({ path.newInputStream(LinkOption.NOFOLLOW_LINKS) }),
    )
}
