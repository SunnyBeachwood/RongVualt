package org.eds.veracrypt.documents

import android.content.ContentResolver
import android.content.Context
import android.database.Cursor
import android.net.Uri
import android.os.ParcelFileDescriptor
import android.os.PowerManager
import android.provider.DocumentsContract
import android.provider.OpenableColumns
import android.system.Os
import android.system.OsConstants
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.util.UUID
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicLong
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.currentCoroutineContext
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlinx.coroutines.delay
import kotlinx.coroutines.withTimeoutOrNull
import kotlinx.coroutines.async
import kotlinx.coroutines.awaitAll
import kotlinx.coroutines.coroutineScope
import org.eds.veracrypt.nativecore.NativeFileSystemAccess
import org.eds.veracrypt.nativecore.VcCore

/** One active volume transfer with bounded file/chunk parallelism and real byte accounting. */
internal object FileTransferManager {
    private const val TAG = "RongVaultTransfer"
    private const val BUFFER_SIZE = 256 * 1024
    private const val PARALLEL_CHUNK_BYTES = 1024 * 1024
    private const val MAX_PARALLEL_CHUNKS = 8
    private const val UPDATE_INTERVAL_MILLIS = 200L
    private var appContext: Context? = null
    private val scope = CoroutineScope(Dispatchers.IO)
    private val mutableState = MutableStateFlow<TransferState>(TransferState.Idle)
    val state: StateFlow<TransferState> = mutableState.asStateFlow()
    private var job: Job? = null
    private var currentProgress: TransferProgress? = null
    private val cancelRequested = AtomicBoolean(false)
    private const val CLEANUP_PREFS = "rongvault_transfer_cleanup"
    private const val CLEANUP_URIS = "output_uris"
    private val cleanupLock = Any()

    fun bind(context: Context) {
        appContext = context.applicationContext
        if (job?.isActive != true) cleanupInterruptedOutputs(context.applicationContext)
    }

    @Synchronized
    fun start(request: TransferRequest): Boolean {
        if (job?.isActive == true) return false
        val context = appContext ?: return false
        cancelRequested.set(false)
        currentProgress = null
        publish(context, TransferState.Preparing(request))
        job = scope.launch {
            try {
                UnlockedVolumeService.withForegroundOperation {
                    runTransfer(context, request)
                }
            } catch (_: CancellationException) {
                val current = mutableState.value
                if (current is TransferState.Cancelling || current is TransferState.Preparing) {
                    publish(context, TransferState.Cancelled(request, currentProgress))
                }
            }
        }
        return true
    }

    @Synchronized
    fun cancel() {
        val active = job?.takeIf { it.isActive } ?: return
        cancelRequested.set(true)
        val progress = currentProgress
        val request = when (val current = mutableState.value) {
            is TransferState.Preparing -> current.request
            is TransferState.Running -> current.request
            is TransferState.Cancelling -> current.request
            is TransferState.Completed -> current.request
            is TransferState.PartialSuccess -> current.request
            is TransferState.Failed -> current.request
            is TransferState.Idle -> null
            is TransferState.Cancelled -> current.request
        } ?: return
        appContext?.let { publish(it, TransferState.Cancelling(request, progress)) }
        active.cancel(CancellationException("Transfer cancelled by user"))
    }

    suspend fun cancelAndWait(): Boolean {
        cancel()
        if (!hasActiveTransfer()) return true
        return withTimeoutOrNull(10_000L) {
            while (hasActiveTransfer()) delay(25L)
            true
        } ?: false
    }

    fun notificationProgress(): TransferProgress? = currentProgress.takeIf { hasActiveTransfer() }

    fun hasActiveTransfer(): Boolean = when (state.value) {
        is TransferState.Preparing, is TransferState.Running, is TransferState.Cancelling -> true
        else -> false
    }

    fun hasActiveForVolume(volumeId: UUID): Boolean = when (val current = state.value) {
        is TransferState.Preparing -> current.request.volumeId == volumeId
        is TransferState.Running -> current.request.volumeId == volumeId
        is TransferState.Cancelling -> current.request.volumeId == volumeId
        else -> false
    }

    private suspend fun runTransfer(context: Context, request: TransferRequest) {
        val resolver = context.contentResolver
        val failures = mutableListOf<String>()
        var completedBytes = 0L
        var successCount = 0
        var latest: TransferProgress? = null
        val progressLock = Any()
        val reservedTargetNames = mutableSetOf<String>()
        try {
            val sources = request.sourceUris.map { source -> querySource(resolver, source) }
            val totalBytes = sources.map { it.size }.takeIf { sizes -> sizes.none { it == null || it < 0L } }?.sumSaturated()
            val calculator = TransferProgressCalculator()
            val plans = sources.mapIndexedNotNull { index, source ->
                if (source.isDirectory) {
                    failures += "${source.name}: directories are not supported"
                    null
                } else {
                    currentCoroutineContext().ensureActive()
                    val targetName = nextTargetName(resolver, request.targetDirectoryUri, source.name, reservedTargetNames)
                    reservedTargetNames += targetName
                    PlannedFile(index, source, targetName)
                }
            }
            val scheduler = ParallelTransferScheduler(context)
            val results = coroutineScope {
                plans.map { plan ->
                    async(Dispatchers.IO) {
                        scheduler.withPermit {
                            transferOneFile(
                                context, request, resolver, plan, sources.size, totalBytes,
                                calculator, progressLock,
                                onProgress = { progress, bytes ->
                                    val adjusted = synchronized(progressLock) {
                                        completedBytes = (completedBytes + bytes).coerceAtMost(Long.MAX_VALUE)
                                        progress.copy(
                                            completedBytes = completedBytes,
                                            totalBytes = totalBytes,
                                            percent = totalBytes?.takeIf { it > 0L }?.let {
                                                val bounded = completedBytes.coerceIn(0L, it)
                                                (bounded / it * 100L + (bounded % it) * 100L / it).toInt().coerceIn(0, 100)
                                            },
                                            failures = failures.toList(),
                                        ).also {
                                            latest = it
                                            currentProgress = it
                                        }
                                    }
                                    publish(context, TransferState.Running(request, adjusted))
                                },
                            )
                        }
                    }
                }.awaitAll()
            }
            results.forEach { result ->
                if (result.success) successCount++ else result.failure?.let(failures::add)
            }
            val final = (latest ?: calculator.update(
                request.direction,
                "",
                sources.size,
                sources.size,
                0L,
                null,
                completedBytes,
                totalBytes,
                failures.toList(),
            )).copy(
                currentFileIndex = sources.size,
                totalFileCount = sources.size,
                completedBytes = completedBytes,
                totalBytes = totalBytes,
                failures = failures.toList(),
            )
            currentProgress = final
            publish(context, when {
                failures.isEmpty() && successCount == sources.size -> TransferState.Completed(request, final)
                successCount > 0 -> TransferState.PartialSuccess(request, final)
                else -> TransferState.Failed(request, final, "No files were transferred")
            })
        } catch (_: CancellationException) {
            publish(context, TransferState.Cancelled(request, currentProgress))
        } catch (error: Throwable) {
            publish(context, TransferState.Failed(request, currentProgress, error.message ?: "Transfer failed"))
        } finally {
            cancelRequested.set(false)
            synchronized(this) { job = null }
        }
    }

    private suspend fun transferOneFile(
        context: Context,
        request: TransferRequest,
        resolver: ContentResolver,
        plan: PlannedFile,
        fileCount: Int,
        totalBytes: Long?,
        calculator: TransferProgressCalculator,
        progressLock: Any,
        onProgress: suspend (TransferProgress, Long) -> Unit,
    ): FileResult {
        val source = plan.source
        val internalTarget = request.targetDirectoryUri.authority == (context.packageName + ".unlocked")
        val outputName = if (internalTarget) "." + plan.targetName + ".rongvault-partial" else plan.targetName
        var output: Uri? = null
        var fileBytes = 0L
        return try {
            output = try {
                if (internalTarget) {
                    val directory = UnlockedVolumeService.resolveTreeUri(request.targetDirectoryUri, request.volumeId)
                    // Non-seekable sources (pipes, including hidden-risk
                    // fixtures) stay on the provider's streaming path. The
                    // direct backend requires a stable size and random reads
                    // before it preallocates a native file.
                    if (directory != null && isSeekableSource(resolver, source.uri)) {
                        return transferNativeTarget(
                            context, request, resolver, plan, directory, fileCount, totalBytes,
                            calculator, progressLock, onProgress,
                        )
                    }
                    if (directory == null) throw TransferFatalException("Unlocked target is no longer available")
                }
                DocumentsContract.createDocument(
                    resolver,
                    targetDocumentUri(request.targetDirectoryUri),
                    source.mimeType ?: "application/octet-stream",
                    outputName,
                ) ?: throw FileNotFoundException("Provider did not create target document")
            } catch (error: Throwable) {
                throw TransferFatalException("Unable to create target for ${source.name}: ${error.message ?: error.javaClass.simpleName}", error)
            }
            journalOutput(context, output)
            val reportBytes: suspend (Long) -> Unit = { delta ->
                val progress = synchronized(progressLock) {
                    fileBytes += delta
                    calculator.update(
                        request.direction, source.name, plan.index + 1, fileCount,
                        fileBytes, source.size, 0L, totalBytes, emptyList(),
                    )
                }
                onProgress(progress, delta)
            }
            val backend: TransferBackend = if (isSeekableTarget(resolver, output) && isSeekableSource(resolver, source.uri)) {
                TransferBackend.SeekablePfd
            } else {
                TransferBackend.Streaming
            }
            when (backend) {
                TransferBackend.SeekablePfd -> {
                    if (!copySeekable(context, resolver, source.uri, output, source.size, reportBytes)) {
                        copyStreaming(resolver, source, output, outputName, reportBytes)
                    }
                }
                TransferBackend.Streaming -> copyStreaming(resolver, source, output, outputName, reportBytes)
                TransferBackend.DirectNative -> error("Direct backend must return before provider target creation")
            }
            if (internalTarget) {
                val partial = output
                output = DocumentsContract.renameDocument(resolver, partial, plan.targetName)
                    ?: throw TransferFatalException("Provider could not finalize ${plan.targetName}")
                unjournalOutput(context, partial)
            }
            unjournalOutput(context, output)
            FileResult(true, null)
        } catch (error: CancellationException) {
            deleteQuietly(resolver, output)
            unjournalOutput(context, output)
            throw error
        } catch (error: Throwable) {
            deleteQuietly(resolver, output)
            unjournalOutput(context, output)
            FileResult(false, error.message ?: "${source.name}: transfer failed")
        }
    }

    /** Direct FAT/exFAT backend: metadata is created once, data uses a stable native handle. */
    private suspend fun transferNativeTarget(
        context: Context,
        request: TransferRequest,
        resolver: ContentResolver,
        plan: PlannedFile,
        directory: UnlockedDocumentNode,
        fileCount: Int,
        totalBytes: Long?,
        calculator: TransferProgressCalculator,
        progressLock: Any,
        onProgress: suspend (TransferProgress, Long) -> Unit,
    ): FileResult {
        val source = plan.source
        val base = directory.relativePath
        val partialPath = if (base.isEmpty()) ".${plan.targetName}.rongvault-partial" else "$base/.${plan.targetName}.rongvault-partial"
        val finalPath = if (base.isEmpty()) plan.targetName else "$base/${plan.targetName}"
        val access = directory.volume.session as? NativeFileSystemAccess
            ?: return FileResult(false, "${source.name}: native filesystem access unavailable")
        var fileBytes = 0L
        return try {
            val file = access.openFile(partialPath, writable = true, create = true, truncate = true)
            file.use { nativeFile ->
                val preallocated = source.size?.let { size ->
                    runCatching { nativeFile.preallocate(size) }.onFailure {
                        android.util.Log.w(TAG, "direct backend degraded=serial_preallocation_failed file=${source.name} reason=${it.message}")
                    }.isSuccess
                } ?: false
                val report: suspend (Long) -> Unit = { delta ->
                    val progress = synchronized(progressLock) {
                        fileBytes += delta
                        calculator.update(request.direction, source.name, plan.index + 1, fileCount, fileBytes, source.size, 0L, totalBytes, emptyList())
                    }
                    onProgress(progress, delta)
                }
                if (!copySourceToNative(context, resolver, source, nativeFile, report, parallel = preallocated)) {
                    throw SourceTransferException("Unable to read ${source.name}")
                }
                nativeFile.flush()
            }
            access.rename(partialPath, finalPath)
            UnlockedVolumeService.notifyVolumeChanged(directory.volume.id)
            FileResult(true, null)
        } catch (error: CancellationException) {
            runCatching { access.delete(partialPath) }
            throw error
        } catch (error: Throwable) {
            runCatching { access.delete(partialPath) }
            FileResult(false, error.message ?: "${source.name}: transfer failed")
        }
    }

    private suspend fun copySourceToNative(
        context: Context,
        resolver: ContentResolver,
        source: SourceInfo,
        target: org.eds.veracrypt.nativecore.NativeOpenFile,
        onBytes: suspend (Long) -> Unit,
        parallel: Boolean,
    ): Boolean = coroutineScope {
        if (!parallel) return@coroutineScope copyNativeStream(resolver, source, target, onBytes)
        if (source.size == null || source.size < 0L) {
            val input = resolver.openInputStream(source.uri) ?: return@coroutineScope false
            input.use { stream ->
                val buffer = ByteArray(BUFFER_SIZE)
                var offset = 0L
                try {
                    while (true) {
                        currentCoroutineContext().ensureActive()
                        val read = stream.read(buffer)
                        if (read < 0) break
                        if (read == 0) continue
                        if (target.write(offset, buffer, 0, read) != read) return@coroutineScope false
                        offset += read
                        onBytes(read.toLong())
                    }
                    true
                } finally {
                    buffer.fill(0)
                }
            }
        } else {
            val descriptor = resolver.openFileDescriptor(source.uri, "r") ?: return@coroutineScope copyNativeStream(resolver, source, target, onBytes)
            try {
                runCatching { Os.lseek(descriptor.fileDescriptor, 0L, OsConstants.SEEK_CUR) }.getOrElse {
                    descriptor.close()
                    return@coroutineScope copyNativeStream(resolver, source, target, onBytes)
                }
                val stream = ParcelFileDescriptor.AutoCloseInputStream(descriptor)
                val channel = stream.channel
                val workers = recommendedWorkerCount(context)
                val nextOffset = AtomicLong(0L)
                val jobs = (0 until workers).map {
                    async(Dispatchers.IO) {
                        while (true) {
                            val offset = nextOffset.getAndAdd(PARALLEL_CHUNK_BYTES.toLong())
                            if (offset >= source.size) break
                            currentCoroutineContext().ensureActive()
                            val length = minOf(PARALLEL_CHUNK_BYTES.toLong(), source.size - offset).toInt()
                            val buffer = ByteArray(length)
                            try {
                            var read = 0
                            var emptyReads = 0
                            while (read < length) {
                                val count = channel.read(ByteBuffer.wrap(buffer, read, length - read), offset + read)
                                if (count < 0) throw SourceTransferException("Source ended before its reported size")
                                if (count == 0) {
                                    if (++emptyReads > 16) throw SourceTransferException("Source made no progress")
                                    continue
                                }
                                emptyReads = 0
                                read += count
                                }
                                if (target.write(offset, buffer) != length) throw TransferFatalException("Unable to write native transfer chunk")
                                onBytes(length.toLong())
                            } finally {
                                buffer.fill(0)
                            }
                        }
                    }
                }
                jobs.awaitAll()
                stream.close()
                true
            } catch (error: Throwable) {
                runCatching { descriptor.close() }
                if (error is CancellationException || error is SourceTransferException || error is TransferFatalException) throw error
                false
            }
        }
    }

    private suspend fun copyNativeStream(
        resolver: ContentResolver,
        source: SourceInfo,
        target: org.eds.veracrypt.nativecore.NativeOpenFile,
        onBytes: suspend (Long) -> Unit,
    ): Boolean {
        val input = resolver.openInputStream(source.uri) ?: return false
        input.use { stream ->
            val buffer = ByteArray(BUFFER_SIZE)
            var offset = 0L
            try {
                while (true) {
                    currentCoroutineContext().ensureActive()
                    val read = stream.read(buffer)
                    if (read < 0) break
                    if (read == 0) continue
                    if (target.write(offset, buffer, 0, read) != read) return false
                    offset += read
                    onBytes(read.toLong())
                }
                return true
            } finally {
                buffer.fill(0)
            }
        }
    }

    private fun publish(context: Context, value: TransferState) {
        mutableState.value = value
        VolumeForegroundService.refresh(context, true)
    }

    private fun isSeekableSource(resolver: ContentResolver, uri: Uri): Boolean {
        val descriptor = runCatching { resolver.openFileDescriptor(uri, "r") }.getOrNull() ?: return false
        return try {
            Os.lseek(descriptor.fileDescriptor, 0L, OsConstants.SEEK_CUR)
            true
        } catch (_: Throwable) {
            false
        } finally {
            descriptor.close()
        }
    }

    private fun isSeekableTarget(resolver: ContentResolver, uri: Uri): Boolean {
        val descriptor = runCatching { resolver.openFileDescriptor(uri, "rw") }.getOrNull() ?: return false
        return try {
            Os.lseek(descriptor.fileDescriptor, 0L, OsConstants.SEEK_CUR)
            true
        } catch (_: Throwable) {
            false
        } finally {
            descriptor.close()
        }
    }

    /**
     * Copies a seekable source and target with bounded positional I/O. A
     * provider exposing a pipe (or rejecting lseek) returns false so the
     * caller can safely use the streaming implementation instead.
     */
    private suspend fun copySeekable(
        context: Context,
        resolver: ContentResolver,
        sourceUri: Uri,
        targetUri: Uri,
        expectedSize: Long?,
        onBytes: suspend (Long) -> Unit,
    ): Boolean = coroutineScope {
        if (expectedSize == null || expectedSize < 0L) return@coroutineScope false
        val input = resolver.openFileDescriptor(sourceUri, "r") ?: return@coroutineScope false
        val output = try {
            resolver.openFileDescriptor(targetUri, "rw") ?: run {
                input.close()
                return@coroutineScope false
            }
        } catch (_: Throwable) {
            input.close()
            return@coroutineScope false
        }
        try {
            try {
                Os.lseek(input.fileDescriptor, 0L, OsConstants.SEEK_CUR)
                Os.lseek(output.fileDescriptor, 0L, OsConstants.SEEK_CUR)
            } catch (_: Throwable) {
                input.close()
                output.close()
                return@coroutineScope false
            }
            val inputStream = ParcelFileDescriptor.AutoCloseInputStream(input)
            val outputStream = ParcelFileDescriptor.AutoCloseOutputStream(output)
            val inputChannel = inputStream.channel
            val outputChannel = outputStream.channel
            val workers = recommendedWorkerCount(context)
            val nextOffset = AtomicLong(0L)
            val jobs = (0 until workers).map {
                async(Dispatchers.IO) {
                    while (true) {
                        val offset = nextOffset.getAndAdd(PARALLEL_CHUNK_BYTES.toLong())
                        if (offset >= expectedSize) break
                        currentCoroutineContext().ensureActive()
                        val length = minOf(PARALLEL_CHUNK_BYTES.toLong(), expectedSize - offset).toInt()
                        val bytes = ByteArray(length)
                        try {
                            var readTotal = 0
                            var emptyReads = 0
                            while (readTotal < length) {
                                val read = inputChannel.read(
                                    ByteBuffer.wrap(bytes, readTotal, length - readTotal),
                                    offset + readTotal,
                                )
                                if (read < 0) throw SourceTransferException("Source ended before its reported size")
                                if (read == 0) {
                                    if (++emptyReads > 16) throw SourceTransferException("Source made no progress")
                                    continue
                                }
                                emptyReads = 0
                                readTotal += read
                            }
                            var written = 0
                            while (written < length) {
                                val count = outputChannel.write(
                                    ByteBuffer.wrap(bytes, written, length - written),
                                    offset + written,
                                )
                                if (count <= 0) throw TransferFatalException("Unable to write transfer chunk")
                                written += count
                            }
                            onBytes(length.toLong())
                        } finally {
                            bytes.fill(0)
                        }
                    }
                }
            }
            jobs.awaitAll()
            outputStream.flush()
            outputStream.fd.sync()
            inputStream.close()
            outputStream.close()
            true
        } catch (error: Throwable) {
            runCatching { input.close() }
            runCatching { output.close() }
            if (error is CancellationException || error is SourceTransferException || error is TransferFatalException) throw error
            // A capability failure is reported before any bytes are written
            // and is the only case that may safely fall back to streaming.
            // Once a positional copy has started, do not reuse a possibly
            // partially written document: fail so the caller removes it.
            throw TransferFatalException("Seekable transfer failed", error)
        }
    }

    private suspend fun copyStreaming(
        resolver: ContentResolver,
        source: SourceInfo,
        targetUri: Uri,
        outputName: String,
        onBytes: suspend (Long) -> Unit,
    ) {
        val input = try {
            resolver.openInputStream(source.uri) ?: throw SourceTransferException("Unable to read ${source.name}")
        } catch (error: CancellationException) {
            throw error
        } catch (error: SourceTransferException) {
            throw error
        } catch (error: Throwable) {
            throw SourceTransferException("Unable to read ${source.name}", error)
        }
        val output = resolver.openOutputStream(targetUri, "w")
            ?: throw TransferFatalException("Unable to write $outputName")
        input.use { inputStream ->
            output.use { outputStream ->
                val buffer = ByteArray(BUFFER_SIZE)
                try {
                    while (true) {
                        currentCoroutineContext().ensureActive()
                        val read = inputStream.read(buffer)
                        if (read < 0) break
                        if (read == 0) continue
                        outputStream.write(buffer, 0, read)
                        onBytes(read.toLong())
                    }
                    outputStream.flush()
                    (outputStream as? FileOutputStream)?.fd?.sync()
                } finally {
                    buffer.fill(0)
                }
            }
        }
    }

    private fun querySource(resolver: ContentResolver, uri: Uri): SourceInfo {
        var name = uri.lastPathSegment ?: "unnamed"
        var size: Long? = null
        var mime: String? = null
        var directory = false
        runCatching {
            resolver.query(uri, arrayOf(OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE, DocumentsContract.Document.COLUMN_MIME_TYPE), null, null, null)
        }.getOrNull()?.use { cursor ->
            if (cursor.moveToFirst()) {
                name = cursor.string(OpenableColumns.DISPLAY_NAME) ?: name
                size = cursor.longOrNull(OpenableColumns.SIZE)
                mime = cursor.string(DocumentsContract.Document.COLUMN_MIME_TYPE)
                directory = mime == DocumentsContract.Document.MIME_TYPE_DIR
            }
        }
        return SourceInfo(uri, name.ifBlank { "unnamed" }, size, mime, directory)
    }

    private fun recommendedWorkerCount(context: Context?): Int = runCatching {
        val thermal = context?.getSystemService(PowerManager::class.java)?.currentThermalStatus
        if (thermal != null && thermal >= PowerManager.THERMAL_STATUS_SEVERE) return@runCatching 1
        VcCore.nativeRecommendedWorkerCount()
    }.getOrElse {
        Runtime.getRuntime().availableProcessors().coerceAtLeast(1)
    }.coerceIn(1, MAX_PARALLEL_CHUNKS)

    private fun nextTargetName(
        resolver: ContentResolver,
        directory: Uri,
        original: String,
        reserved: Set<String>,
    ): String {
        val existing = reserved.toMutableSet()
        runCatching {
            val children = DocumentsContract.buildChildDocumentsUriUsingTree(directory, DocumentsContract.getDocumentId(directory))
            resolver.query(children, arrayOf(DocumentsContract.Document.COLUMN_DISPLAY_NAME), null, null, null)?.use { cursor ->
                while (cursor.moveToNext()) cursor.string(DocumentsContract.Document.COLUMN_DISPLAY_NAME)?.let(existing::add)
            }
        }
        return allocateTransferName(original, existing)
    }

    /** DocumentsContract.createDocument expects a document URI, while the
     * picker returns a tree URI. Keep the tree URI for child enumeration but
     * normalize it at the mutation boundary. */
    private fun targetDocumentUri(uri: Uri): Uri = runCatching {
        val treeId = DocumentsContract.getTreeDocumentId(uri)
        DocumentsContract.buildDocumentUri(uri.authority!!, treeId)
    }.getOrDefault(uri)

    private fun deleteQuietly(resolver: ContentResolver, uri: Uri?) {
        if (uri != null) runCatching { DocumentsContract.deleteDocument(resolver, uri) }
    }

    private fun journalOutput(context: Context, uri: Uri?) {
        if (uri == null) return
        synchronized(cleanupLock) {
            val prefs = context.getSharedPreferences(CLEANUP_PREFS, Context.MODE_PRIVATE)
            val values = prefs.getStringSet(CLEANUP_URIS, emptySet()).orEmpty().toMutableSet()
            values += uri.toString()
            prefs.edit().putStringSet(CLEANUP_URIS, values).apply()
        }
    }

    private fun unjournalOutput(context: Context, uri: Uri?) {
        if (uri == null) return
        synchronized(cleanupLock) {
            val prefs = context.getSharedPreferences(CLEANUP_PREFS, Context.MODE_PRIVATE)
            val values = prefs.getStringSet(CLEANUP_URIS, emptySet()).orEmpty().toMutableSet()
            if (values.remove(uri.toString())) prefs.edit().putStringSet(CLEANUP_URIS, values).apply()
        }
    }

    private fun cleanupInterruptedOutputs(context: Context) {
        val prefs = context.getSharedPreferences(CLEANUP_PREFS, Context.MODE_PRIVATE)
        val values = prefs.getStringSet(CLEANUP_URIS, emptySet()).orEmpty()
        values.forEach { value -> deleteQuietly(context.contentResolver, Uri.parse(value)) }
        if (values.isNotEmpty()) prefs.edit().remove(CLEANUP_URIS).apply()
    }

    private data class SourceInfo(val uri: Uri, val name: String, val size: Long?, val mimeType: String?, val isDirectory: Boolean)
    private data class PlannedFile(val index: Int, val source: SourceInfo, val targetName: String)
    private data class FileResult(val success: Boolean, val failure: String?)
    private class SourceTransferException(message: String, cause: Throwable? = null) : Exception(message, cause)
    private class TransferFatalException(message: String, cause: Throwable? = null) : Exception(message, cause)

    private fun Cursor.string(column: String): String? = getColumnIndex(column).takeIf { it >= 0 }?.let { getString(it) }
    private fun Cursor.longOrNull(column: String): Long? = getColumnIndex(column).takeIf { it >= 0 && !isNull(it) }?.let { getLong(it) }
    private fun List<Long?>.sumSaturated(): Long = fold(0L) { total, value ->
        val amount = value ?: return@fold Long.MAX_VALUE
        if (total > Long.MAX_VALUE - amount) Long.MAX_VALUE else total + amount
    }
}

internal data class TransferNotificationSnapshot(
    val direction: TransferDirection,
    val progress: TransferProgress,
)
